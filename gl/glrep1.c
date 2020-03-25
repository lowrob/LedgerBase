/******************************************************************************
		Sourcename    : glrep1.c
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

	registr();
	trial();
	bsheet();
	trans();
	journal();
	ChartOfAcc();
	gstbysupp();
	tranlist();
	tranaud();

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
#define	EXIT		12	/* also defined in reputils.c */

static int retval;	/* Global variable to store function values */
char e_mesg[80]; /* to store error messages */

int	registr();
int	trial();
int	bsheet();
int	trans();
int	journal();
int	ChartOfAcc() ;	/* Chart Of Account */
int	gstbysupp();
int	tranlist();
int	tranaud();

/*
	Initialize  menu items not exceeding 9, 
	and call the menu processing routine

	The routines AddMenuItem() and Process() are defined in the file
	reputils.c
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

	/* Add menu items in the following order */

#ifdef ENGLISH
	if( AddMenuItem( "CHART OF ACCOUNTS",ChartOfAcc,GLMAST)<0 )
		exit(-1);
	if( AddMenuItem( "G/L REGISTER", registr,GLMAST)<0 )
		exit(-1);
	if( AddMenuItem( "TRIAL BALANCE", trial,GLMAST)<0 )
		exit(-1);
	if( AddMenuItem( "BALANCE SHEET", NULL,GLMAST)<0 )
		exit(-1);
	if( AddMenuItem( "YEAR COMPARATIVE BALANCE SHEET", NULL,GLMAST)<0 )
		 exit(-1);
	if( AddMenuItem( "PERIOD COMPARATIVE BALANCE SHEET", NULL,GLMAST)<0 )
		 exit(-1);
	if( AddMenuItem( "JOURNAL LISTING", journal,GLTRHDR)<0 )
		 exit(-1);
	if( AddMenuItem( "TRANSACTION LISTING",tranlist,GLTRHDR)<0 )
		 exit(-1);
	if( AddMenuItem( "TRANSACTION/GL AUDIT",tranaud,GLTRHDR)<0 )
		 exit(-1);
	if( AddMenuItem( "GST REPORT BY SUPPLIER",gstbysupp,GLTRHDR)<0 )
		 exit(-1);
#else
	if( AddMenuItem( "PLAN COMPTABLE",ChartOfAcc,GLMAST)<0 )
		exit(-1);
	if( AddMenuItem( "REGISTRE G/L", registr,GLMAST)<0 )
		exit(-1);
	if( AddMenuItem( "BALANCE DE VERIFICATION", trial,GLMAST)<0 )
		exit(-1);
	if( AddMenuItem( "BILAN", NULL,GLMAST)<0 )
		exit(-1);
	if( AddMenuItem( "BILAN COMPARATIF D'ANNEES", NULL,GLMAST)<0 )
		 exit(-1);
	if( AddMenuItem( "BILAN COMPARATIF DE PERIODES", NULL,GLMAST)<0 )
		exit(-1);
	if( AddMenuItem( "LISTE DU JOURNAL", journal,GLTRHDR)<0 )
		exit(-1);
	if( AddMenuItem( "LISTE DES TRANSACTIONS", tranlist,GLTRHDR)<0 )
		exit(-1);
	if( AddMenuItem( "RAPPORT DE LA TPS PAR FOURNISSEUR",gstbysupp,GLTRHDR)<0 )
		 exit(-1);
#endif

	for(;;) {
		/* process the user's options */
#ifdef ENGLISH
		ret_option = Process(terminal, " FINANCIAL REPORTS ");
#else
		ret_option = Process(terminal, "RAPPORTS FINANCIERS");
#endif

		switch( ret_option) {
			case 0 :  exit(0) ;
			case 4 :
				bsheet(ret_option-3) ;
				break ;
			case 5 :
				bsheet(ret_option-3) ;
				break ;
			case 6 :
				bsheet(ret_option-3) ;
				break ;
			case NOACCESS:
				fomen(e_mesg);
				get();
				break;
			default : exit(0) ;
		}
	}	
}

