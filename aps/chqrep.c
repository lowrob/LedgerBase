/******************************************************************************
		Sourcename    : chqrep.c
		System        : Budgetary Financial system.
		Module        : Accounts Payable reports
		Created on    : 89-11-22
		Created  By   : J PRESCOTT.
		Cobol sources : 
*******************************************************************************
About this program:
	This program prints the following reports, calling some utilities from
	apreputl.c. 
		
	interface for the below report routines for 
		1.Report selection
		2.Accepting key information.

	The name of the option should not be longer than OPTIONLEN defined in
	apreputl.c



HISTORY:
 Date           Programmer     Description of modification
____/__/__      __________     ___________________________
*******************************************************************************/
#include <stdio.h>

#define MAIN
#define MAINFL		-1	/* no main file used */

#include <bfs_defs.h>
#include <bfs_recs.h>

#define SYSTEM		"ACCOUNTS PAYABLE"
#define MOD_DATE	"23-JAN-90" 
#define	EXIT		12	/* also defined in streputl.c */ 

int	ChequeCircAll();
int	ChequeCircOut();

char	e_mesg[80];

/*
	Initialize  menu items not exceeding 9, 
	and call the menu processing routine

	The routines AddMenuItem() and Process() are defined in the file
	arreputl.c
*/
main( argc, argv )
int argc; char *argv[];
{
int	ret_option ;

	strncpy( SYS_NAME, SYSTEM, 50 ); 	/* Module name */
	strncpy( CHNG_DATE, MOD_DATE, 10 );	/* Last date of change */
	proc_switch( argc,argv,MAINFL ); 	/* process the switches */
	if( argc<3 )
		exit(0);

	/* Add menu items in the following order */
#ifdef ENGLISH
	if(AddMenuItem("LIST OF CHEQUES ",ChequeCircAll,CHQHIST)<0 )
		exit(-1);
	if(AddMenuItem("LIST OF OUTSTANDING CHEQUES",ChequeCircOut,CHQHIST)<0 )
		exit(-1);
#else
	if(AddMenuItem("LISTE DES CHEQUES",ChequeCircAll,CHQHIST)<0 )
		exit(-1);
	if(AddMenuItem("LISTE DES CHEQUES NON-REGLES",ChequeCircOut,CHQHIST)<0 )
		exit(-1);
#endif
	for(;;) {
		/* process the user's options */
#ifdef ENGLISH
		ret_option = Process(terminal, " CHEQUE HISTORY REPORTS");
#else
		ret_option = Process(terminal, "RAPPORTS DE L'HISTORIQUE");
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
ChequeCircAll()			/* List of all cheques in circulation */
{			

	return(ChequeCirc(0)); 
}
ChequeCircOut()			/* List of outstanding cheques in circulation */
{			

	return(ChequeCirc(1)); 
}
