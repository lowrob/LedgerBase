/******************************************************************************
		Sourcename    : porep1.c
		System        : Budgetary Financial system.
		Module        : PO reports
		Created on    : 89-07-31
		Created  By   : K HARISH.
		Cobol sources : 
*******************************************************************************
About this program:
	This program prints the following reports, calling some utilities from
	reputils.c. reputils.c uses a profom based screen and provides a  user
	interface for the below report routines for accepting key information.

	prntpo();
	PObyPO_No();
	PObySupp();
	PobyAcct();
	Supprpt();
	SLabels();
	SuppLst();
	PoOutstanding();

HISTORY:
 Date           Programmer     Description of modification
____/__/__      __________     ___________________________
*******************************************************************************/
#include <stdio.h>

#define MAIN
#define MAINFL		-1	/* no main file used */

#include <bfs_defs.h>

#define SYSTEM		"PURCHASE ORDER"
#define MOD_DATE	"23-JAN-90"
#define	EXIT		12	/* also defined in reputils.c */

int	SuppbyNo();
int	SuppbyName();
int	SuppLstNo();
int	SuppLstName();
int	Suppurge();
int	PobyNumber();
int	PobySupp();
int	PobyAcct();
int	PoLiquid();
int	PoOutstanding();
int  	PoOutstd();
int	PoPartial();
int	PoAged();
int	prntpo();
int	LabelsbyNo();
int	LabelsbyName();
int	inv_labels();
int     PoPurge();

char	e_mesg[80];

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

	strncpy( SYS_NAME, SYSTEM, 20 ); 	/* Module name */
	strncpy( CHNG_DATE, MOD_DATE, 10 );	/* Last date of change */
	proc_switch( argc,argv,MAINFL ); 	/* process the switches */
	if( argc<3 )
		exit(0);

	/* Add menu items in the following order */
#ifdef ENGLISH
	if( AddMenuItem("LIST OF SUPPLIERS BY CODE",SuppbyNo,SUPPLIER)<0)
#else
	if( AddMenuItem("LISTE DES FOURNISSEURS PAR CODE",SuppbyNo,SUPPLIER)<0)
#endif
		exit(-1);
#ifdef ENGLISH
	if( AddMenuItem("LIST OF SUPPLIERS BY NAME",SuppbyName,SUPPLIER)<0)
#else
	if( AddMenuItem("LISTE DES FOURNISSEURS PAR NOM",SuppbyName,SUPPLIER)<0)
#endif
		exit(-1); 
#ifdef ENGLISH
	if( AddMenuItem("SUPPLIER ADDRESS LIST BY CODE",SuppLstNo,SUPPLIER)<0)
#else
	if( AddMenuItem("LISTE D'ADRESSES FOURN PAR CODE",SuppLstNo,SUPPLIER)<0)
#endif
		exit(-1); 
#ifdef ENGLISH
	if( AddMenuItem("SUPPLIER ADDRESS LIST BY NAME",SuppLstName,SUPPLIER)<0)
#else
	if( AddMenuItem("LISTE D'ADRESSES FOURN PAR NOM",SuppLstName,SUPPLIER)<0)
#endif
		exit(-1); 
#ifdef ENGLISH
	if( AddMenuItem("SUPPLIER ACTIVITY REPORT",Suppurge,SUPPLIER) <0)
#else
	if( AddMenuItem("RAPPORT DES ACTIVITES DES FOURN",Suppurge,SUPPLIER) <0)
#endif
		exit(-1); 
#ifdef ENGLISH
	if( AddMenuItem("LIST OF PO'S BY PO NUMBER",PobyNumber,POHDR) <0)
#else
	if( AddMenuItem("LISTE DES BC PAR NUMERO DE BC",PobyNumber,POHDR) <0)
#endif
		exit(-1);
#ifdef ENGLISH
	if( AddMenuItem("LIST OF PO'S BY SUPPLIER",PobySupp,POHDR) <0) 
#else
	if( AddMenuItem("LISTE DES BC PAR FOURNISSEUR",PobySupp,POHDR) <0) 
#endif
		exit(-1);
#ifdef ENGLISH
	if( AddMenuItem("LIST OF PO'S BY G/L ACCOUNT",PobyAcct,POHDR) <0)
#else
	if( AddMenuItem("LISTE DES BC PAR COMPTE G/L",PobyAcct,POHDR) <0)
#endif
		exit(-1);
#ifdef ENGLISH
	if( AddMenuItem("LIST OF LIQUIDATED PO'S",PoLiquid,POHDR) <0)
#else
	if( AddMenuItem("LISTE DES BC LIQUIDES",PoLiquid,POHDR) <0)
#endif
		exit(-1);
#ifdef ENGLISH
	if( AddMenuItem("LIST OF OUTSTANDING PO'S",PoOutstd,POHDR) <0)
#else
	if( AddMenuItem("LISTE DES BC NON-REGLES",PoOutstd,POHDR) <0)
#endif
		exit(-1);
#ifdef ENGLISH
	if( AddMenuItem("LIST OF PARTIAL PO'S",PoPartial,POHDR) <0)
#else
	if( AddMenuItem("LISTE DES BC PARTIELS",PoPartial,POHDR) <0)
#endif
		exit(-1);
#ifdef ENGLISH
	if( AddMenuItem("PO AGED TRIAL BALANCE",PoAged,POHDR) <0)
#else
	if( AddMenuItem("BALANCE CHRONOLOGIQUE DES BC",PoAged,POHDR) <0)
#endif
		exit(-1); 
#ifdef ENGLISH
	if( AddMenuItem("PRINT PO'S",prntpo,POHDR)<0 )
#else
	if( AddMenuItem("IMPRIMER LES BC",prntpo,POHDR)<0 )
#endif
		exit(-1);
#ifdef ENGLISH
	if( AddMenuItem("PRINT SUPPLIER LABELS BY CODE",LabelsbyNo,SUPPLIER)<0)
#else
	if( AddMenuItem("IMPRIMER ETIQUETTES FOURN PAR CDE",LabelsbyNo,SUPPLIER)<0)
#endif
		exit(-1);
#ifdef ENGLISH
	if( AddMenuItem("PRINT SUPPLIER LABELS BY NAME",
						LabelsbyName,SUPPLIER)<0)
#else
	if( AddMenuItem("IMPRIMER ETIQUETTES FOURN PAR NOM",
						LabelsbyName,SUPPLIER)<0)
#endif
		exit(-1);
#ifdef ENGLISH
	if( AddMenuItem("PRINT INVOICE LABELS",
						inv_labels,POHDR)<0)
#else
	if( AddMenuItem("IMPRIMER ETIQUETTES POUR FACTURE",
						inv_labels,POHDR)<0)
#endif
		exit(-1);
#ifdef ENGLISH
	if( AddMenuItem("DELETE COMPLETED PO'S",
						PoPurge,POHDR)<0)
#else
	if( AddMenuItem("ELIMINATION DES BC COMPLETES",
						PoPurge,POHDR)<0)
#endif
		exit(-1);
	for(;;) {
		/* process the user's options */
#ifdef ENGLISH
		ret_option = Process(terminal, "  PO REPORTS   ");
#else
		ret_option = Process(terminal, "RAPPORTS DES BC");
#endif

		switch(ret_option) {
			case NOACCESS:
				fomen(e_mesg);
				get();
				break;
			case REPORT_ERR:
			case DBH_ERR: 
				fomen(e_mesg);
				get();
			case PROFOM_ERR:
			default:
				exit(0);
		}
	}	
}

SuppbyNo()
{
	return(Supprpt(1));
}

SuppbyName()
{
	return(Supprpt(2));
}
SuppLstNo()
{
	return(SuppLst(0));
}
SuppLstName()
{
	return(SuppLst(1));
}
PobyNumber()
{
	return(PobyPoNbr(0));
}

PoLiquid()
{
	return(PobyPoNbr(1));
}
PoPurge()
{
	return(PobyPoNbr(3));
}
LabelsbyNo()
{
	return(SLabels(0));
}

LabelsbyName()
{
	return(SLabels(1));
}
PoPartial()
{
	return(PoOutstanding(0));
}
PoOutstd()
{
	return(PoOutstanding(1));
}
