/*-----------------------------------------------------------------------
Source Name: 
System     : Budgetary Financial system.
Module     : Account Payable System.
Created  On: 17 Dec. 90  
Created  By: F. Tao

DESCRIPTION:
	This program converts floating point to double numeric from  the
	old aphist file to the new formated aphist file.
	references: convap.c (J. Prescott)

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

------------------------------------------------------------------------*/
#define  MAIN
#define  MAINFL		GLMAST   	/* main file used */

#include <stdio.h>
#include <isnames.h>
#include <cfomstrc.h>
#include <bfs_defs1.h>
#include <bfs_recs1.h>

#define	SYSTEM		"ACCOUNT PAYABLE"	/*Sub System Name */
#define	MOD_DATE	"18-DEC-90"		/* Program Last Modified */

typedef struct {
	short	funds ;			/* Funds PIC 999 */
	char	accno[19] ;		/* Account number */
	short	reccod ;		/* Record Code PIC 99 */
	short	sect ;			/* Section Code? PIC 9 */
	short	admis ;			/* Admissibility code PIC 9 */
	long	keys[12] ;		/* 12 keys PIC 9[5] */
	short	cdbud ;			/* Budget code */
	short	cdpro ;			/* Projection code */
	short	cdunit ;		/* Unit of measurement code */
	char	descp[49] ;		/* Description */
	float	grad ;			/* Gradiant ? PIC 999.99 */
	double	comdat ;		/* Committed to date */
	double	curdb ;			/* Current period Debits */
	double	curcr ;			/* Current period Credits */
	double	ytd ;			/* Year to Date */
	double	budcur;			/* Budget current Year */
	double	budpre ;		/* Budget previous Year */
	double	currel[13] ;	/* actual current yr ..13 periods */
	double	prerel[13] ;	/* actual previous yr ..13 periods */
	double	curbud[13] ;	/* Budget current yr ..13 periods */
	double	prebud[13] ;	/* Budget current yr ..13 periods */
	double	opbal ;			/* Opening balance */
	double	nextdb ;		/* Next period Debits(Accruals) */
	double	nextcr ;		/* Next period Credits(Accruals) */
	} Old_gl;

static	Old_gl ogl_rec;
static  Gl_rec  gl_rec;   

static	char filenm[50];
static	char outfile[50];
static	char tempfile[50];
static	char c_mesg[50];


main(argc,argv)
int argc;
char *argv[];
{
	int file_pt;
	int iostat, err, i;
	
	strncpy(SYS_NAME,SYSTEM,50);	/* Sub system name */
	strncpy(CHNG_DATE,MOD_DATE,10);	/* Modification Date */
	proc_switch(argc, argv, MAINFL) ; 	/* Process Switches */

	strcpy(filenm,"glmast");

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
	getchar();

	file_pt = isopen(tempfile,RWR);
	if(file_pt < 0) {
	  printf("Error opening old glmast file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(file_pt);
	  exit(-1);
	}
	iostat = isstart(file_pt,(char *)&ogl_rec,0,ISFIRST);
	if(iostat < 0) {
	  printf("Error starting old glmast file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(file_pt);
	  exit(-1);
	}
	for( ; ; ) {
		iostat = isreads(file_pt,(char *)&ogl_rec,0);
		if(iostat < 0) {
			if(iostat == EFL) break;
		  	printf("Error reading old glmast file. Iserror: %d\n"
					,iserror);
			break;
		}
		
		printf("KEY: %d-%s-%d\n",ogl_rec.funds,ogl_rec.accno,ogl_rec.reccod);
		gl_rec.funds = ogl_rec.funds ;
		strcpy(gl_rec.accno,	ogl_rec.accno);	
		gl_rec.reccod = ogl_rec.reccod ;
		gl_rec.sect = ogl_rec.sect ;
		gl_rec.admis = ogl_rec.admis ;
		for (i=0; i < 12; i++) 
			gl_rec.keys[i] = ogl_rec.keys[i] ;
		gl_rec.cdbud = ogl_rec.cdbud ;
		gl_rec.cdpro = ogl_rec.cdpro ;
		gl_rec.cdunit = ogl_rec.cdunit ;
		strcpy(gl_rec.desc,	ogl_rec.descp);	

		gl_rec.grad   = ogl_rec.grad;
		gl_rec.comdat   = ogl_rec.comdat;
		gl_rec.curdb   = ogl_rec.curdb;
		gl_rec.curcr   = ogl_rec.curcr;
		gl_rec.ytd   = ogl_rec.ytd;
		gl_rec.budcur   = ogl_rec.budcur;
		gl_rec.budpre   = ogl_rec.budpre;
		for( i=0 ; i < 13 ; i++ ) {
			gl_rec.currel[i]   = ogl_rec.currel[i];
			gl_rec.prerel[i]   = ogl_rec.prerel[i];
			gl_rec.curbud[i]   = ogl_rec.curbud[i];
			gl_rec.prebud[i]   = ogl_rec.prebud[i];
		}
		gl_rec.opbal   = ogl_rec.opbal;
		gl_rec.nextdb   = ogl_rec.nextdb;
		gl_rec.nextcr   = ogl_rec.nextcr;
		
		err = put_gl(&gl_rec,ADD,c_mesg);
		if(err != NOERROR){
			printf(c_mesg);
			printf("ERROR in Saving new GLMAST  Records\n"); 
			roll_back(c_mesg);
			exit(-1);
		}
	
		if((err = commit(c_mesg))<0) {
			printf(c_mesg);
			break;
		}
	}
	isclose(file_pt);
	close_dbh();			/* Close files */
	exit(0);
} /* END OF MAIN */
