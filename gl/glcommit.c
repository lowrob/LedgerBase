/******************************************************************************
		Sourcename    : glcommit.c
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

Pa_rec  pa_rec;
Tr_item	tr_item;

int retval;	/* Global variable to store function values */
char e_mesg[80]; /* to store error messages */
/* Initialize profom fields, call entry procedures */
main( argc, argv )
int argc;
char *argv[];
{
	strncpy( SYS_NAME, SYSTEM, 50 ); /* Module name */
	strncpy( CHNG_DATE, MOD_DATE, 10 );/* Last date of change */
	
	proc_switch( argc,argv,MAINFL );/* process the switches */

	tr_item.ti_fund = 0 ;
	tr_item.ti_reccod = 0 ;
	tr_item.ti_create[0] = ' ';
	tr_item.ti_seq_no = 0 ;
	tr_item.ti_item_no = 0 ;

	for(;;) {
	 	retval = get_n_tritem(&tr_item, UPDATE , 0, 0,e_mesg);

		if(retval == EFL) 
			break;

		if(retval < 0) {
			printf("%s", e_mesg);
			exit(-1);
		}

		if(tr_item.ti_fund > 1) break;

		printf(" - %d  - ", tr_item.ti_fund);
		retval = put_tritem( &tr_item,P_DEL, e_mesg);
		if( retval!=NOERROR ){
			printf("%s",e_mesg);
			roll_back(e_mesg);
			exit(-1);
		}
		if( commit(e_mesg)<0 ){
			printf("%s",e_mesg);
			exit(-1);
		}
	}
	close_dbh();
	exit(0);
}
