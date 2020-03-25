/******************************************************************************
		Sourcename    : glrep2.c
		System        : Budgetary Financial system.
		Module        : GL reports
		Created on    : 89-07-31
		Created  By   : K HARISH.
		Cobol sources : 
*******************************************************************************
About this program:
	This program prints the following reports, calling some utilities from
	reputils.c. reputils.c uses a profom based screen and provides a  user
	interface for the below report routines for accepting key information.

	consrep();
	rec_rep();
	bdtrrep();

HISTORY:
 Date           Programmer     Description of modification
____/__/__      __________     ___________________________
*******************************************************************************/
#include <stdio.h>

#define MAIN
#define MAINFL		-1	/* no main file used */

#include <bfs_defs.h>

#define SYSTEM		"GENERAL LEDGER"
#define MOD_DATE	"23-JAN-90"
#define	EXIT	12	/* also defined in reputils.c */

static int retval;	/* Global variable to store function values */
char e_mesg[80]; /* to store error messages */

int	consrep();
int	rec_rep();
int	bdtrrep();
int     bdtracct();

/* 	add menu items  and process menu options when user selects them */
/*	
	Functions  AddMenuItem(), Process(), DisplayMessage(), GetResponse()
	and Confirm() are found in the file  reputils.c
	Function AudRep() is found  in the file audrep.c 
*/
main( argc, argv )
int argc;
char *argv[];
{
int	ret_option ;

	strncpy( SYS_NAME, SYSTEM, 50 ); 	/* Module name */
	strncpy( CHNG_DATE, MOD_DATE, 10 );	/* Last date of change */
	proc_switch( argc,argv,MAINFL ); 	/* process the switches */
	if( argc<3 )
		exit(0);

	/* Add menu items to the list */
#ifdef ENGLISH
	if( AddMenuItem( "ENERGY CONSUMPTION",consrep,GLMAST)<0 )
		exit(-1);
	if( AddMenuItem( "RECURRING ENTRY LIST",rec_rep,RECHDR)<0 )
		exit(-1);
	if( AddMenuItem( "BUDGET TRANS LIST",bdtrrep,GLBDHDR)<0 )
		exit(-1);
  	if( AddMenuItem( "BUDGET TRANS BY ACCOUNT",bdtracct,GLBDITEM)<0 )
		exit(-1);
#else
	if( AddMenuItem( "CONSOMMATION D'ENERGIE",consrep,GLMAST)<0 )
		exit(-1);
	if( AddMenuItem( "LISTE DES ENTREES REPETITIVES",rec_rep,RECHDR)<0 )
		exit(-1);
	if( AddMenuItem( "LISTE DES TRANSACTIONS DE BUDGET",bdtrrep,GLBDHDR)<0 )
		exit(-1);
  	if( AddMenuItem( "TRANS DE BUDGET PAR COMPTE",bdtracct,GLBDITEM)<0 )
		exit(-1);
#endif

	/* read and process the user's options */
#ifdef ENGLISH
	ret_option = Process(terminal, "MISCELLANEOUS REPORTS");
#else
	ret_option = Process(terminal, "   RAPPORTS DIVERS   ");
#endif
	close_dbh();

	if(ret_option == NOACCESS) {
		fomen(e_mesg);
		get();
	}

	exit( ret_option );
}

