/******************************************************************************
		Sourcename    : aprep.c
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

#define SYSTEM		"ACCOUNTS PAYABLE"
#define MOD_DATE	"23-JAN-90" 
#define	EXIT		12	/* also defined in streputl.c */ 

#ifdef ENGLISH
#define SUMMARY		"Y"
#define NOSUMMARY	"N"
#define ALL		"A"
#define SP_ONLY		"S"
#else
#define SUMMARY		"O"
#define NOSUMMARY	"N"
#define ALL		"T"
#define SP_ONLY		"A"
#endif

int inv_labels();       /* invoice labels */


char e_mesg[80];
/*
	Initialize  menu items not exceeding 9, 
	and call the menu processing routine

	The routines AddMenuItem() and Process() are defined in the file
	apreputl.c
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
	if(AddMenuItem("INVOICE LABELS",inv_labels,APINVOICE)<0)
		exit(-1);
#else
	 if(AddMenuItem("ETIQUETTES DE FACTURE",inv_labels,APINVOICE)<0)
		exit(-1);
#endif


	for(;;) {
		/* process the user's options */
#ifdef ENGLISH
		ret_option = Process(terminal, "     INVOICE LABELS ");
#else
		ret_option = Process(terminal, "  ETIQUETTES DE FACTURE");
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
