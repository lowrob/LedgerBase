/*-----------------------------------------------------------------------
Source Name: convsch.c   
System     : Budgetary Financial system.
Module     : 
Created  On: 12 Feb. 91  
Created  By: J. Prescott

DESCRIPTION:
	Convert Old school file to contain contact.

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

------------------------------------------------------------------------*/
#define  MAIN
#define  MAINFL		SCHOOL  	/* main file used */

#include <stdio.h>
#include <isnames.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	SYSTEM		"SETUP"	/*Sub System Name */
#define	MOD_DATE	"19-DEC-90"		/* Program Last Modified */

typedef struct {
	short	sc_numb ;		/* School number PIC 99 */
	char	sc_name[29] ;		/* School name PIC X(28)  */
	char	sc_add1[31] ;		/* address line 1 */
	char	sc_add2[31] ;		/* address line 2 */
	char	sc_add3[31] ;		/* address line 3 */
	char	sc_pc[8] ;		/* postal code */
	char 	sc_contact[26] ;	/* Contact Person*/
	char 	sc_phone[11] ;		/* phone number */
	char	sc_fax[11] ;		/* fax number */
	long	sc_size ;		/* School size, or area, PIC 9(6) */
	} Old_school;

static	Old_school old_sch; 
static  Sch_rec	sch_rec;   

static	char filenm[50];
static	char outfile[50];
static	char tempfile[50];
static	char c_mesg[50];

int 	err;

main(argc,argv)
int argc;
char *argv[];
{
	int school_fd;
	int iostat, err, i;
	
	strncpy(SYS_NAME,SYSTEM,50);	/* Sub system name */
	strncpy(CHNG_DATE,MOD_DATE,10);	/* Modification Date */
	proc_switch(argc, argv, MAINFL) ; 	/* Process Switches */

	strcpy(filenm,"school");

        form_f_name(filenm,outfile);
	strcpy(tempfile,"CFXXXXXX");
	mktemp(tempfile);
#ifdef  MS_DOS
        rename(outfile,tempfile);
#else
        link(outfile,tempfile);
        unlink(outfile);        
#endif

        strcat(outfile,".IX");
	strcpy(c_mesg, tempfile) ;
        strcat(c_mesg,".IX");
#ifdef	MS_DOS
        rename(outfile,c_mesg);
#else
        link(outfile,c_mesg);
        unlink(outfile);
#endif

	printf("outfile: %s  tempfile: %s\n", outfile,tempfile);

	school_fd = isopen(tempfile,RWR);
	if(school_fd < 0) {
	  printf("Error opening old school file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(school_fd);
	  exit(-1);
	}
	iostat = isstart(school_fd,(char *)&old_sch,0,ISFIRST);
	if(iostat < 0) {
	  printf("Error starting old school file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(school_fd);
	  exit(-1);
	}
	for( ; ; ) {
		iostat = isreads(school_fd,(char *)&old_sch,0);
		if(iostat < 0) {
			if(iostat == EFL) break;
		  	printf("Error reading old school file. Iserror: %d\n"
					,iserror);
			break;
		}

		sch_rec.sc_numb = old_sch.sc_numb ;
		strcpy(sch_rec.sc_name , old_sch.sc_name) ;
		strcpy(sch_rec.sc_add1 , old_sch.sc_add1) ;
		strcpy(sch_rec.sc_add2 , old_sch.sc_add2) ;
		strcpy(sch_rec.sc_add3 , old_sch.sc_add3) ;
		strcpy(sch_rec.sc_pc , old_sch.sc_pc) ;
		strcpy(sch_rec.sc_phone , old_sch.sc_phone) ;
		strcpy(sch_rec.sc_fax , old_sch.sc_fax) ;
		sch_rec.sc_size = old_sch.sc_size ;
		strcpy(sch_rec.sc_contact, old_sch.sc_size) ;
		sch_rec.sc_r_t[0] = '\0';

		err = put_sch(&sch_rec,ADD,c_mesg);
		if(err != NOERROR){
			printf(c_mesg);
			printf("ERROR in Saving new STMAST  Records\n"); 
			roll_back(c_mesg);
			exit(-1);
		}
	
		i++;
		if(i % 10 == 0)
			if((err = commit(c_mesg))<0) {
				printf(c_mesg);
				break;
		}
	}
	if(i % 10 != 0)
		if((err = commit(c_mesg))<0) {
			printf(c_mesg);
	}
	isclose(school_fd);
	close_dbh();			/* Close files */
	exit(0);
} /* END OF MAIN */

