/******************************************************************************
		Sourcename    : tendrep.c
		System        : Budgetary Financial system.
		Module        : Tendering reports
		Created on    : 92-04-20
		Created  By   : J Prescott.
		Cobol sources : 
*******************************************************************************
About this program:
	This program prints the following reports, calling some utilities from
	reputils.c. reputils.c uses a profom based screen and provides a  user
	interface for the below report routines for accepting key information.

	CategoryList();

HISTORY:
 Date           Programmer     Description of modification
____/__/__      __________     ___________________________
*******************************************************************************/
#include <stdio.h>

#define MAIN
#define MAINFL		-1	/* no main file used */

#include <bfs_defs.h>

#define SYSTEM		"TENDERING ORDER"
#define MOD_DATE	"20-APR-92"
#define	EXIT		12	/* also defined in reputils.c */

int     CategoryList();
int	ItemGroupList();
int	PrintTender();
int	TenderSpread();
int	SuppBidList();
int	SuppAwardList();
int	SupplyCatalogue();
int	CatbySupp();
int	UnawardedCat();
int	PriceVarList();
int	AwdUnawdSupp();
int	SupplierLabels();
int	PriceVarHist();
int	PurgeCatalogue();
int	HistReport();

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
	if( AddMenuItem("LIST OF SUPPLY CATEGORIES",CategoryList,CATEGORY)<0)
		exit(-1);
	if( AddMenuItem("LIST OF ITEM GROUPS BY CATEGORIES",ItemGroupList,ITEM_GROUP)<0)
		exit(-1);
	if( AddMenuItem("PRINT GENERAL SUPPLY TENDER",PrintTender,CATALOGUE)<0)
		exit(-1);
	if( AddMenuItem("PRINT SPREAD SHEET BY ITEM #",TenderSpread,CATALOGUE)<0)
		exit(-1);
	if( AddMenuItem("LIST OF SUPPLIER BIDS",SuppBidList,BID)<0)
		exit(-1);
	if( AddMenuItem("LIST OF SUPPLIER AWARDS",SuppAwardList,BID)<0)
		exit(-1);
	if( AddMenuItem("GENERAL SUPPLIES CATALOGUE",SupplyCatalogue,CATALOGUE)<0)
		exit(-1);
	if( AddMenuItem("GEN SUPPLY CATALOGUE BY SUPPLIER",CatbySupp,CATALOGUE)<0)
		exit(-1);
	if( AddMenuItem("UNAWARDED CATALOGUE ITEMS",UnawardedCat,CATALOGUE)<0)
		exit(-1);
	if( AddMenuItem("PRICE VARIATION REPORT",PriceVarList,CATALOGUE)<0)
		exit(-1);
	if( AddMenuItem("AWARDED/UNAWARDED SUPPLIERS",AwdUnawdSupp,BID)<0)
		exit(-1);
	if( AddMenuItem("PRINT SUPPLIER LABELS",SupplierLabels,POTBIDDER)<0)
		exit(-1);
	if( AddMenuItem("PRICE VARIATION HISTORY",PriceVarHist,CATALOGUE)<0)
		exit(-1);
	if( AddMenuItem("PURGE SUPPLIES CATALOGUE",PurgeCatalogue,CATALOGUE)<0)
		exit(-1);
	if( AddMenuItem("AWARD HISTORY",HistReport,TENDHIST)<0)
		exit(-1);
#else
	if( AddMenuItem("LISTE DES CATEGORIES DE MATERIEL",CategoryList,CATEGORY)<0)
		exit(-1);
	if( AddMenuItem("LISTE DES GROUPES D'ARTICLES PAR CATEGORIE",CategoryList,ITEM_GROUP)<0)
		exit(-1);
	if( AddMenuItem("IMPRIMER SOUMISSION DE MATERIEL GENERAL",PrintTender,CATALOGUE)<0)
		exit(-1);
	if( AddMenuItem("IMPRIMER TABLEAU PAR NUMERO D'ARTICLE",TenderSpread,CATALOGUE)<0)
		exit(-1);
	if( AddMenuItem("LISTE DES OFFRES D'ACHAT DES FOURNISSEURS",SuppBidList,BID)<0)
		exit(-1);
	if( AddMenuItem("LISTE DES ATTRIBUTIONS DES FOURNISSEURS",SuppAwardList,BID)<0)
		exit(-1);
	if( AddMenuItem("CATALOGUE DE MATERIEL GENERAL",SupplyCatalogue,CATALOGUE)<0)
		exit(-1);
	if( AddMenuItem("CATALOGUE DE MATERIEL GENERAL PAR FOURNISSEUR",CatbySupp,CATALOGUE)<0)
		exit(-1);
	if( AddMenuItem("ARTICLES NON-ATTRIBUES DU CATALOGUE",UnawardedCat,CATALOGUE)<0)
		exit(-1);
	if( AddMenuItem("FOURNISSEURS ATTRIBUES/NON-ATTRIBUES",AwdUnawdSupp,BID)<0)
		exit(-1);
	if( AddMenuItem("IMPRIMER ETIQUETTES DE FOURNISSEUR",SupplierLabels,POTBIDDER)<0)
		exit(-1);
	if( AddMenuItem("HISTORIQUE DE VARIATION DES PRIX",PriceVarHist,CATALOGUE)<0)
		exit(-1);
	if( AddMenuItem("CATALOGUE DE MATERIAL ELIMINE",PurgeCatalogue,CATALOGUE)<0)
		exit(-1);
	if( AddMenuItem("HISTORIQUE DES ATTRIBUTIONS",HistReport,TENDHIST)<0)
		exit(-1);
#endif

	for(;;) {
		/* process the user's options */
#ifdef ENGLISH
		ret_option = Process(terminal, "TENDERING REPORTS");
#else
		ret_option = Process(terminal, "RAPPORTS DE SOUMISSION");
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

