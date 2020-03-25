/******************************************************************************
		Sourcename    : glclear.c
		System        : Budgetary Financial System.
		Subsystem     : General Ledger System 
		Module        : General Ledger 
		Created on    : Nov 7, 90.
		Created  By   : Cathy Burns.
		Cobol sources : 

HISTORY:
 Date           Programmer     Description of modification
____/__/__      __________     ___________________________

******************************************************************************/
#include <stdio.h>

#define MAIN
#define MAINFL		GLMAST	/* main file used */

#include <bfs_defs.h>
#include <bfs_recs.h>
#include <cfomstrc.h>

#define SYSTEM		"General Ledger"
#define MOD_DATE	"07-NOV-90"
#define SCREEN_NAME	"glchg"
#define ESCAPE		12	/* flag indicates discontinuation of entries */
#define LOW 		-1
#define HIGH 		 1
#define HL_CHAR(VAL)	(VAL==HIGH) ? HV_CHAR : LV_CHAR
#define NO_HLP_WIN	(sr.curfld!=800)
#define ESC_H		(sr.retcode==RET_USER_ESC && \
			(sr.escchar[0]=='H'||sr.escchar[0]=='h'))

#ifdef ENGLISH
#define CHANGE 		'C'
#define EXITOPT		'E'

#define EDIT		'E'
#define CANCEL		'C'
#define YES		'Y'
#define NO		'N'
#else	/* FRENCH */
#define CHANGE 		'C'
#define EXITOPT		'F'

#define EDIT		'M'
#define CANCEL		'A'
#define YES		'O'
#define NO		'N'
#endif

Gl_rec	gl_rec, pre_rec;	/* glmast reccord and previod record */
Pa_rec  pa_rec;
Ctl_rec ctl_rec;

int retval;	/* Global variable to store function values */
char e_mesg[80]; /* to store error messages */
/* Initialize profom fields, call entry procedures */
main( argc, argv )
int argc;
char *argv[];
{
	int	 i;

	strncpy( SYS_NAME, SYSTEM, 50 ); /* Module name */
	strncpy( CHNG_DATE, MOD_DATE, 10 );/* Last date of change */
	
	proc_switch( argc,argv,MAINFL );/* process the switches */

	gl_rec.funds = 0 ;
	gl_rec.accno[0] = '0';
	gl_rec.reccod = 0 ;

	for(;;) {
	 	retval = get_n_gl(&gl_rec, UPDATE , 0, 0,e_mesg);

		if(retval == EFL) 
			break;

		if(retval < 0) {
			printf("%s", e_mesg);
			exit(-1);
		}

		printf("Key %d-%s-%d \n",gl_rec.funds,gl_rec.accno,gl_rec.reccod);
		gl_rec.nextdb = 0.00 ;
		gl_rec.nextcr = 0.00 ;
		gl_rec.comdat = 0.00 ;
		gl_rec.opbal = 0.00 ;
		gl_rec.ytd = 0.00 ;
		gl_rec.budpre = 0.00 ;
		gl_rec.budcur = 0.00 ;
		gl_rec.curdb = 0.00 ;
		gl_rec.curcr = 0.00 ;
		gl_rec.cdbud = 0 ;
	
		for(i=0 ; i<13 ; i++) {
			gl_rec.currel[i] = 0.00 ;
			gl_rec.prerel[i] = 0.00 ;
			gl_rec.curbud[i] = 0.00 ;
			gl_rec.prebud[i] = 0.00 ;
		}

		retval = put_gl( &gl_rec, UPDATE, e_mesg);
		if( retval!=NOERROR ){
			printf("%s",e_mesg);
			roll_back(e_mesg);
			exit(-1);
		}
		if( commit(e_mesg)<0 ){
			printf("%s",e_mesg);
			exit(-1);
		}
		gl_rec.reccod++;
	}
	close_dbh();
	exit(0);
}
