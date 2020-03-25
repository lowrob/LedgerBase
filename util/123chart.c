/*-----------------------------------------------------------------------
Source Name: 123chart.c 
System     : Ledger Base
Module     : Utility.
Created  On: 11-FEB-92
Created  By: J. Prescott 

DESCRIPTION:
	Convert Lotus Chart of accounts to lbase glmast file.

MODIFICATIONS:        

Programmer     	YY/MM/DD       	Description of modification
~~~~~~~~~~     	~~~~~~~~       	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
------------------------------------------------------------------------*/
#define  MAIN
#define  MAINFL		GLMAST

#include <stdio.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_recs.h>
#include <repdef.h>
#include <ctype.h>

#define	SYSTEM		"UTIL"  	/* Sub System Name */
#define	MOD_DATE	"11-FEB-92"		/* Program Last Modified */

#ifdef ENGLISH

#else

#endif

#define EXIT		12
#define BADKEY		-77
#define BADSECT		-88
#define NODESC		-99

static	Pa_rec	pa_rec;
static	Gl_rec	gl_rec;
static	Ctl_rec	ctl_rec;

static	char	e_mesg[80];
static	FILE *fd;
static	int err;
static	int log_fd;

main(argc,argv)
int argc;
char *argv[];
{
	int err;
	strncpy(SYS_NAME,SYSTEM,50);	/* Sub system name */
	strncpy(CHNG_DATE,MOD_DATE,10);	/* Modification Date */
	proc_switch(argc, argv, MAINFL) ; 	/* Process Switches */

	if ( UpdateGlmast() < 0) { 		/* Initiate Process */
		Close();
		exit(-1);
	}

	Close();			/* return to menu */
	exit(NOERROR);

} /* END OF MAIN */

/*-------------------------------------------------------------------*/
/* Reset information and close data files. */
Close()
{

	close_dbh();			/* Close files */
	return(NOERROR);
}
/*-----------------------------------------------------------------------*/
/* Update StudentBase Data Files 					 */
/*-----------------------------------------------------------------------*/
UpdateGlmast()
{
	int retval;	/* number of errors encountered */
	char txt_str[80];
	int i;
	int cnt = 0;

	fd = fopen("chart.txt","r");
	if(fd == NULL) {
		printf("Error Opening Chart File\n");
		return(ERROR);
	}

	if( (log_fd = creat("123audit",TXT_CRMODE)) < 0)  {
		printf("Open Error 123audit file... Contact Systems Manager\n");
		return(-1) ;
	}
	lseek(log_fd, (long)0, 2 );

	for( ; ; ) {
		ClearDBHRecord();

		retval = Read123Chart();	
		if(retval==EFL) {
			break;
		}
		printf("Key: %d-%s-%d\n",gl_rec.funds,gl_rec.accno,gl_rec.reccod);
		if(retval==BADKEY) { 
			sprintf(e_mesg,"Key: %d-%s-%d\n",gl_rec.funds,
					gl_rec.accno,gl_rec.reccod);
			write(log_fd, e_mesg, strlen(e_mesg) ) ;
			continue;
		}
		if(retval==BADSECT || retval == NODESC) {
			sprintf(e_mesg,"Key: %d-%s-%d\n",gl_rec.funds,
					gl_rec.accno,gl_rec.reccod);
			write(log_fd, e_mesg, strlen(e_mesg) ) ;
		}

		retval = get_gl(&gl_rec,BROWSE,0,e_mesg);
		if(retval != UNDEF) {
			sprintf(e_mesg,"Key Already Exists: %d-%s-%d\n",
				gl_rec.funds,gl_rec.accno,gl_rec.reccod);
			write(log_fd, e_mesg, strlen(e_mesg) ) ;
			continue;
		}

		retval = put_gl(&gl_rec,ADD,e_mesg);
		if(retval < 0) {
			printf("%s\n",e_mesg);	
			return(ERROR);
		}

		retval = commit(e_mesg);
		if (retval == LOCKED) return(LOCKED);
		if (retval < 0) {
			return(retval);
		}
		cnt++;
	}
	printf("Records Converted: %d\n",cnt);

	fclose(fd);
	return(NOERROR);
}
ClearDBHRecord()
{
	int i;

	gl_rec.funds = 0 ;		/* Funds PIC 999 */
	gl_rec.accno[0] = '\0' ;	/* Account number */
	gl_rec.reccod = 0 ;		/* Record Code PIC 99 */
	gl_rec.sect = 0 ;		/* Section Code? PIC 9 */
	gl_rec.admis = 0 ;		/* Admissibility code PIC 9 */
	for(i=0;i<NO_KEYS;i++) {
		gl_rec.keys[i] = 0 ;	/* 12 keys PIC 9[5] */
	}
	gl_rec.cdbud = 0 ;		/* Budget code */
	gl_rec.cdpro = 0 ;		/* Projection code */
	gl_rec.cdunit = 0 ;		/* Unit of measurement code */
	gl_rec.desc[0] = '\0';		/* Description */
	gl_rec.grad = 0.00 ;		/* Gradiant ? PIC 999.99 */
	gl_rec.comdat = 0.00 ;		/* Committed to date */
	gl_rec.curdb = 0.00 ;		/* Current period Debits */
	gl_rec.curcr = 0.00 ;		/* Current period Credits */
	gl_rec.ytd = 0.00 ;		/* Year to Date */
	gl_rec.budcur = 0.00;		/* Budget current Year */
	gl_rec.budpre = 0.00 ;		/* Budget previous Year */
	for(i=0;i<NO_PERIODS;i++) {
		gl_rec.currel[i] = 0.00;/* actual current yr ..13 periods */
		gl_rec.prerel[i] = 0.00;/* actual previous yr ..13 periods */
		gl_rec.curbud[i] = 0.00;/* Budget current yr ..13 periods */
		gl_rec.prebud[i] = 0.00;/* Budget current yr ..13 periods */
	}
	gl_rec.opbal = 0.00 ;		/* Opening balance */
	gl_rec.nextdb = 0.00 ;		/* Next period Debits(Accruals) */
	gl_rec.nextcr = 0.00 ;		/* Next period Credits(Accruals) */

	return(NOERROR);
}
Read123Chart()
{
	char	rec_str[200];
	char	txt_str[50];
	int	i;
	int	err;
	int	retval;
	int	desc_cnt;

	err = NOERROR; /* Assume no error until one found */

	if(fgets(rec_str,sizeof(rec_str),fd)==NULL) {
		return(EFL);
	}

	/* Make rec_str a fixed record length string with no newline */
	for(i=strlen(rec_str)-1;i < sizeof(rec_str)-1;i++) {
		rec_str[i] = ' ';
	}
	strcat(rec_str,"\0");

	/* Separate rec_str into gl_rec fields */
	/* Fund */
	strncpy(txt_str,rec_str,3);
	txt_str[3] = '\0';
	gl_rec.funds = atoi(txt_str);
	ctl_rec.fund = gl_rec.funds;
	retval = get_ctl(&ctl_rec,BROWSE,0,e_mesg);
	if(retval < 0) {
		sprintf(e_mesg,"Invalid Fund: %d\n",gl_rec.funds);
		write(log_fd, e_mesg, strlen(e_mesg) ) ;
		err = BADKEY;
	}

	/* Account */
	strncpy(txt_str,rec_str+3,13);
	txt_str[13] = '\0';
	strncpy(gl_rec.accno,txt_str,13);
	gl_rec.accno[13]='\0';
	if(acnt_chk(gl_rec.accno)==ERROR) {
		sprintf(e_mesg,"Invalid Account Number: %s\n",gl_rec.accno);	
		write(log_fd, e_mesg, strlen(e_mesg) ) ;
		err = BADKEY;
	}	

	/* Record Code */
	strncpy(txt_str,rec_str+20,2);
	txt_str[2] = '\0';
	gl_rec.reccod = atoi(txt_str);
	if(gl_rec.reccod == 0) {
		sprintf(e_mesg,"Invalid Record Code: %d\n",gl_rec.reccod);
		write(log_fd, e_mesg, strlen(e_mesg) ) ;
		err = BADKEY;
	}

	/* Section */
	strncpy(txt_str,rec_str+27,1);
	txt_str[1] = '\0';
	gl_rec.sect = atoi(txt_str);
	if(gl_rec.sect < 1 && gl_rec.sect > 4) {
		sprintf(e_mesg,"Invalid Section: %d\n",gl_rec.sect);
		write(log_fd, e_mesg, strlen(e_mesg) ) ;
		err = BADSECT;
	}

	/* Description */
	strncpy(txt_str,rec_str+29,30);
	txt_str[30] = '\0';
	strncpy(gl_rec.desc,txt_str,31);
	desc_cnt=0;
	for(i=0;i<strlen(gl_rec.desc);i++) {
		if(gl_rec.desc[i] == ' ' || gl_rec.desc[i] == '\0') {
			continue;
		}
		desc_cnt++;
	}
	if(desc_cnt==0) {
		sprintf(e_mesg,"No Description\n");
		write(log_fd, e_mesg, strlen(e_mesg) ) ;
		err = NODESC;
	}
	/* Setup Keys */
	for(i=0;i <  NO_KEYS;i++) {
		strncpy(txt_str,rec_str+59+(i*6),5);
		txt_str[5] = '\0';
		gl_rec.keys[i] = atoi(txt_str);
	}	

	if(strlen(rec_str) >= 132) {
		/* Budget Code */
		strncpy(txt_str,rec_str+132,5);
		txt_str[5] = '\0';
		gl_rec.cdbud = atoi(txt_str);
	}
	else {
		gl_rec.cdbud = 0;
	}
	return(err);
}
