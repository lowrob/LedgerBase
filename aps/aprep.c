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

int PListbysuppns();	/* Liabilities list by supplier code (no summary)*/
int PListbysuppsu();	/* Liabilities list by supplier code (summary)   */
int PListbyfundns();	/* Liabilities list by fund number   (no summary)*/
int PListbyfundsu();	/* Liabilities list by fund number   (summary)   */
int SPListbysuppns();   /* Liabilities stop pmnt list by supplier (no summary)*/
int SPListbysuppsu();   /* Liabilities stop pmnt list by supplier (summary)   */
int SPListbyfundns();   /* Liabilities stop pmnt list by fund 	  (no summary)*/
int SPListbyfundsu();   /* Liabilities stop pmnt list by fund	  (summary)   */
int ChequePreList();	/* Cheque Pre-List report */
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
	if(AddMenuItem("LIABILITIES BY SUPPLIER CODE",PListbysuppns,
                                                                 APINVOICE)<0)
		exit(-1);
	if(AddMenuItem("LIABILITIES BY SUPPLIER CODE (SUMMARY)",PListbysuppsu,
								 APINVOICE)<0)
		exit(-1);
	if(AddMenuItem("LIABILITIES BY FUND",PListbyfundns,APINVOICE)<0)
		exit(-1);
	if(AddMenuItem("LIABILITIES BY FUND (SUMMARY)",PListbyfundsu,
								 APINVOICE)<0)
		exit(-1);
	if(AddMenuItem("STOP PMTS BY SUPPLIER CODE" ,SPListbysuppns,
                                                                 APINVOICE)<0)
		exit(-1);
	if(AddMenuItem("STOP PMTS BY SUPPLIER CODE (SUMMARY)",SPListbysuppsu,
								 APINVOICE)<0)
		exit(-1);
	if(AddMenuItem("STOP PMTS BY FUND ",SPListbyfundns, APINVOICE)<0)
		exit(-1);
	if(AddMenuItem("STOP PMTS BY FUND (SUMMARY)",SPListbyfundsu,
								APINVOICE)<0)
		exit(-1);
	if(AddMenuItem("CHEQUE PRE-LIST",ChequePreList,CHEQUE)<0)
		exit(-1);

#else
	if(AddMenuItem("PASSIF PAR CODE DE FOURN",PListbysuppns,
								APINVOICE)<0)
		exit(-1);
	if(AddMenuItem("PASSIF PAR CODE DE FOURN (RESUME)",PListbysuppsu,
								 APINVOICE)<0)
		exit(-1);
	if(AddMenuItem("PASSIF PAR FOND",PListbyfundns,APINVOICE)<0)
		exit(-1);
	if(AddMenuItem("PASSIF PAR FOND (RESUME)",PListbyfundsu,APINVOICE)<0)
		exit(-1);
	if(AddMenuItem("ARRETS PAIEMENTS PAR CODE DE FOURN",
						SPListbysuppns,APINVOICE)<0)
		exit(-1);
	if(AddMenuItem("ARRETS PAIEM PAR CD FOURN (RESUME)",
						SPListbysuppsu,APINVOICE)<0)
		exit(-1);
	if(AddMenuItem("ARRETS PAIEMENTS PAR FOND ",SPListbyfundns,
								 APINVOICE)<0)
		exit(-1);
	if(AddMenuItem("ARRETS PAIEMENTS PAR FOND (RESUME)",SPListbyfundsu,
								 APINVOICE)<0)
		exit(-1);
	if(AddMenuItem("LISTE PRELIMINAIRE DES CHEQUES",ChequePreList,CHEQUE)<0)
		exit(-1);
#endif


	for(;;) {
		/* process the user's options */
#ifdef ENGLISH
		ret_option = Process(terminal, " CHEQUE PRE-RUN REPORTS");
#else
		ret_option = Process(terminal, "RAPP CHEQUES AVANT IMPR");
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
PListbysuppns()		/* Payable list by supplier no summary */
{
	return(PayList(0,NOSUMMARY,ALL));
}
PListbysuppsu()		/* Payable summary list by supplier */
{
	return(PayList(0,SUMMARY,ALL)); 
}
PListbyfundns()		/* Payable list by fund no summary */
{
	return(PayList(1,NOSUMMARY,ALL)); 
}
PListbyfundsu()		/* Payable summary list by fund */
{
	return(PayList(1,SUMMARY,ALL)); 
}
SPListbysuppns()   /* Liabilities stop pmnt list by supplier   (no summary)*/
{
	return(PayList(0,NOSUMMARY,SP_ONLY)); 
}
SPListbysuppsu()   /* Liabilities stop pmnt list by supplier   (summary)   */
{
	return(PayList(0,SUMMARY,SP_ONLY)); 
}
SPListbyfundns()   /* Liabilities stop pmnt list by fund     (no summary)*/
{
	return(PayList(1,NOSUMMARY,SP_ONLY)); 
}
SPListbyfundsu()   /* Liabilities stop pmnt list by fund     (summary)   */
{
	return(PayList(1,SUMMARY,SP_ONLY)); 
}
