/******************************************************************************
		Sourcename    : arrep.c
		System        : Budgetary Financial system.
		Module        : Accounts Receivable reports
		Created on    : 89-11-22
		Created  By   : J PRESCOTT.
		Cobol sources : 
*******************************************************************************
About this program:
	This program prints the following reports, calling some utilities from
	arreputl.c. 
		
	interface for the below report routines for 
		1.Report selection
		2.Accepting key information.

	The name of the option should not be longer than OPTIONLEN defined in
	arreputl.c


HISTORY:
 Date           Programmer     Description of modification
____/__/__      __________     ___________________________
1993/02/01	L.Robichaud	Add a customer list by Name or Code.
*******************************************************************************/
#include <stdio.h>

#define MAIN
#define MAINFL		-1		/* main file used (don't check) */

#include <bfs_defs.h>

#define SYSTEM		"ACCOUNTS RECEIVABLE"
#define MOD_DATE	"23-JAN-90"
#define	EXIT		12	/* also defined in streputl.c */

int CListbyno();		/* list of customers by code */
int CListbyna();		/* list of customers by name */
int CLabbyno();			/* customer labels by code */
int CLabbyna();			/* customer labels by name */
int CAddbyno();			/* customer address by code */
int CAddbyna();			/* customer address by name */
int agecust();			/* customer aged outstanding invoices */
int cu_state();			/* customer statement */
int Trans_list1();		/* Sales transaction listing */
int Rcpt_list1();		/* receipts transaction listing */
int CustPurge();		/* Delete customer report */
int non_receipts();		/* Non Receipts Listing */
int PrntInv();

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
	if( AddMenuItem("LIST OF CUSTOMERS BY CODE",CListbyno,CUSTOMER)<0 )
		 exit(-1);
	if( AddMenuItem("LIST OF CUSTOMERS BY NAME",CListbyna,CUSTOMER)<0)
		exit(-1);
	if( AddMenuItem("PRINT CUSTOMER LABELS BY CODE",CLabbyno,CUSTOMER)<0 )
		 exit(-1);
	if( AddMenuItem("PRINT CUSTOMER LABELS BY NAME",CLabbyna,CUSTOMER)<0)
		exit(-1);
	if( AddMenuItem("PRINT CUSTOMER ADDRESS BY CODE",CAddbyno,CUSTOMER)<0 )
		 exit(-1);
	if( AddMenuItem("PRINT CUSTOMER ADDRESS BY NAME",CAddbyna,CUSTOMER)<0)
		exit(-1);
	if( AddMenuItem("AR AGED TRIAL BALANCE",agecust,ARSHDR)<0)
		exit(-1);
	if( AddMenuItem("CUSTOMER STATEMENT",cu_state,ARSHDR)<0)
		exit(-1);
	if( AddMenuItem("SALES TRANSACTION LISTING",Trans_list1,ARSHDR)<0)
		exit(-1);
	if( AddMenuItem("RECEIPTS LISTING",Rcpt_list1,RCPTHDR)<0)
		exit(-1);
	if( AddMenuItem("CUSTOMER DELETION REPORT",CustPurge,CUSTOMER)<0)
		exit(-1);
	if( AddMenuItem("NON RECEIPTS LISTING",non_receipts,GLTRHDR)< 0)
		exit(-1);
	if( AddMenuItem("INVOICE PRINTING",PrntInv,ARSHDR)< 0)
		exit(-1);
#else
	if( AddMenuItem("LISTE DES CLIENTS PAR CODE",CListbyno,CUSTOMER)<0 )
		 exit(-1);
	if( AddMenuItem("LISTE DES CLIENTS PAR NOM",CListbyna,CUSTOMER)<0)
		exit(-1);
	if( AddMenuItem("IMPR ETIQUETTES CLIENTS PAR CODE",CLabbyno,CUSTOMER)<0 )
		 exit(-1);
	if( AddMenuItem("IMPR ETIQUETTES CLIENTS PAR NOM",CLabbyna,CUSTOMER)<0)
		exit(-1);
	if( AddMenuItem("IMPR ADRESS DES CLIENTS PAR CODE",CAddbyno,CUSTOMER)<0 )
		 exit(-1);
	if( AddMenuItem("IMPR ADRESS DES CLIENTS PAR NOM",CAddbyna,CUSTOMER)<0)
		exit(-1);
	if( AddMenuItem("BALANCE CHRONO DES C/R",agecust,ARSHDR)<0)
		exit(-1);
	if( AddMenuItem("RELEVE DE CLIENT",cu_state,ARSHDR)<0)
		exit(-1);
	if( AddMenuItem("LISTE DES TRANSACTIONS DE VENTE",Trans_list1,ARSHDR)<0)
		exit(-1);
	if( AddMenuItem("LISTE DES RECUS",Rcpt_list1,RCPTHDR)<0)
		exit(-1);
	if( AddMenuItem("RAPPORT D'ELIMINATION DE CLIENTS",CustPurge,CUSTOMER)<0)
		exit(-1);
	if( AddMenuItem("LISTE DES NON-RECUS",non_receipts,GLTRHDR)< 0)
		exit(-1);
	if( AddMenuItem("IMPRIMER LES FACTURES",PrntInv,ARSHDR)< 0)
		exit(-1);
#endif
	for(;;) {
		/* process the user's options */
#ifdef ENGLISH
		ret_option = Process(terminal, "  ARS REPORTS  ");
#else
		ret_option = Process(terminal, " RAPPORTS C/R");
#endif

		switch( ret_option) {
			case 0 :
				exit(0) ;

			case NOACCESS:
				fomen(e_mesg);
				get();
				break;
			case REPORT_ERR:
			case DBH_ERR:
				fomen(e_mesg);
				get();
			case PROFOM_ERR:
			default :
				exit(0) ;
		}
	}	
}
CListbyno()
{
	return( Custrpt(0) );
}
CListbyna()
{
	return( Custrpt(1) );
}

CLabbyno()
{
	return( CLabels(0) );
}

CLabbyna()
{
	return( CLabels(1) );
}
CAddbyno()
{
	return( CustLst(0) );
}

CAddbyna()
{
	return( CustLst(1) );
}
Trans_list1()		/* call interactive transaction listing */
{
	return( Trans_list(0) );
}
Rcpt_list1()		/* call interactive receipts listing */
{
	return( Rcpt_list(0) );
}
cu_state()		/* call interactive customer statement listing */
{
	return( custstat(0) );
}
PrntInv()
{	
	return(prntinv());
}
