/******************************************************************************
		Sourcename    : stockrep.c
		System        : Budgetary Financial system.
		Module        : Inventory reports
		Created on    : 89-09-15
		Created  By   : K HARISH.
		Cobol sources : 
*******************************************************************************
About this program:
	This program prints the following reports, calling some utilities from
	streputl.c. 
		
	PhyCntWS();		Physical Count WorkSheet
	PhyCntRep();		Physical Count Report
	StatusRep();		Stock Status Report
	BelowMin();		Below Minimum Report
	TrialBal();		Stock Trial Balance
	mastlist();		Stock Master Listing
	alloclst() ;		Stock Allocation Listing
	Tranlist() ;		Stock Transactions Report
	Stclear();		Clear Stock File

		streputl.c uses a profom based screen and provides a  user
	interface for the below report routines for 
		1.Report selection
		2.Accepting key information.

	The name of the option should not be longer than OPTIONLEN defined in
	streputl.c



HISTORY:
 Date           Programmer     Description of modification
____/__/__      __________     ___________________________
*******************************************************************************/
#include <stdio.h>

#define MAIN
#define MAINFL		-1	/* no main file used */

#include <bfs_defs.h>

#define SYSTEM		"INVENTORY"
#define MOD_DATE	"23-JAN-90"
#define	EXIT		12	/* also defined in streputl.c */

static int retval;	/* Global variable to store function values */
char e_mesg[80]; 	/* to store error messages */

int	PhyCntWS();		/* Physical Count WorkSheet */
int	PhyCntRep();		/* Physical Count Report */
int	StatusRep();		/* Stock Status Report */
int	BelowMin();		/* Below Minimum Report */
int	TrialBal();		/* Stock Trial Balance */
int	mastlist();		/* Stock Master Listing */
int	alloclst() ;		/* Stock Allocation Listing */
int	Tranlist() ;		/* Stock Transactions Report */
int 	Stclear();		/* Clear the stock file */

/*
	Initialize  menu items not exceeding 9, 
	and call the menu processing routine

	The routines AddMenuItem() and Process() are defined in the file
	streputl.c
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
	if( AddMenuItem("PHYSICAL COUNT WORKSHEET",PhyCntWS,STMAST)<0)
		exit(-1);
	if( AddMenuItem("PHYSICAL COUNT REPORT",PhyCntRep,STMAST)<0)
		exit(-1);
	if( AddMenuItem("STOCK STATUS REPORT",StatusRep,STMAST)<0)
		exit(-1);
	if( AddMenuItem("BELOW MINIMUM REPORT",BelowMin,STMAST)<0)
		exit(-1);
	if( AddMenuItem("STOCK TRIAL BALANCE",TrialBal,STMAST)<0)
		exit(-1);
	if( AddMenuItem("STOCK MASTER LISTING",mastlist,STMAST)<0)
		exit(-1);
	if( AddMenuItem("STOCK ALLOCATION LISTING",alloclst,ALLOCATION)<0)
		exit(-1);
	if( AddMenuItem("STOCK TRANSACTION LISTING",Tranlist,STTRAN)<0)
		exit(-1);
	if( AddMenuItem("CLEAR STOCK FILE",Stclear,STTRAN)<0)
		exit(-1);

#else
	if( AddMenuItem("FEUILLE DE TRAVAIL DU COMPTE MANUEL",PhyCntWS,STMAST)<0)
		exit(-1);
	if( AddMenuItem("RAPPORT DU COMPTE MANUEL",PhyCntRep,STMAST)<0)
		exit(-1);
	if( AddMenuItem("RAPPORT D'ETAT DES STOCKS",StatusRep,STMAST)<0)
		exit(-1);
	if( AddMenuItem("RAPPORT DES STOCKS SOUS-MINIMUM",BelowMin,STMAST)<0)
		exit(-1);
	if( AddMenuItem("BALANCE DE VERIFICATION DES STOCKS",TrialBal,STMAST)<0)
		exit(-1);
	if( AddMenuItem("LISTE DES STOCKS MAITRES",mastlist,STMAST)<0)
		exit(-1);
	if( AddMenuItem("LISTE DES ALLOCATIONS DE STOCKS",alloclst,ALLOCATION)<0)
		exit(-1);
	if( AddMenuItem("LISTE DES TRANSACTIONS DE STOCKS",Tranlist,STTRAN)<0)
		exit(-1);
#endif

	for(;;) {
		/* process the user's options */
#ifdef ENGLISH
		ret_option = Process(terminal, " STOCK REPORTS ");
#else
		ret_option = Process(terminal, "RAPPORTS STOCKS");
#endif

		switch( ret_option) {
			case REPORT_ERR:
			case NOACCESS:
				fomen(e_mesg);
				get();
				break;
			case DBH_ERR:
				fomen(e_mesg);
				get();
			case PROFOM_ERR:
			default :
				exit(0) ;
		}
	}	
}
PhyCntWS()
{
	return( phycnt(1) );	/* parameter: 1 if physical count worksheet */
}
PhyCntRep()		
{
	return( phycnt(2) );	/* parameter: 2 if physical count report */
}
StatusRep()			/* Stock status report */
{
	return( ststatus(1) );
}
BelowMin()			/* Below minimum report */
{
	return( ststatus(2) );
}
TrialBal()			/* Stock trial balance */
{
	return( ststatus(3) );
}
Tranlist()			/* Transaction listing: interactive */
{
	return( tranlist(1) );
}
Stclear()			/* Clear stock file */
{
	return( stclear() );
}
