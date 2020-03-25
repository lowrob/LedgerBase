/******************************************************************************
		Sourcename    : reqrep.c
		System        : Budgetary Financial system.
		Module        : Requisition reports
		Created on    : 91-04-08
		Created  By   : J Prescott.
		Cobol sources : 
*******************************************************************************
About this program:
	This program prints the following reports, calling some utilities from
	reputils.c. reputils.c uses a profom based screen and provides a  user
	interface for the below report routines for accepting key information.


HISTORY:
 Date           Programmer     Description of modification
____/__/__      __________     ___________________________
*******************************************************************************/
#include <stdio.h>

#define MAIN
#define MAINFL		-1	/* no main file used */

#include <bfs_defs.h>
#include <bfs_recs.h>
#include <requ.h>

#define SYSTEM		"REQUISITIONING"
#define MOD_DATE	"23-JAN-90"
#define	EXIT		12	/* also defined in reputils.c */

int	SuppbyNo();
int	SuppbyName();
int	SuppLstNo();
int	SuppLstName();
int	ReqbyNo();
int	ReqbySupp();
int	ReqbyCCno();
int	ReqbyAcct();
int	ReqbyStck();
int	ReqAged();
int	ProcReq();
int	DisReq();
int	PrntReq();
int	StckRep();
int	ReqAudit();

char	e_mesg[80];

#ifdef ENGLISH
#define YES	'Y'
#else
#define YES	'O'
#endif
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
Pa_rec	param;
int	ret_option ;

	strncpy( SYS_NAME, SYSTEM, 20 ); 	/* Module name */
	strncpy( CHNG_DATE, MOD_DATE, 10 );	/* Last date of change */
	proc_switch( argc,argv,MAINFL ); 	/* process the switches */
	if( argc<3 )
		exit(0);

	ret_option = get_param(&param, BROWSE, 1, e_mesg) ;
	if(ret_option < 1) {
		exit(1) ;
	}

	ret_option = settax();
	if(ret_option < 0) {
		exit(1);
	}

	/* Add menu items in the following order */
#ifdef ENGLISH
	if( AddMenuItem("LIST OF SUPPLIERS BY CODE",SuppbyNo,SUPPLIER)<0)
		exit(-1);
	if( AddMenuItem("LIST OF SUPPLIERS BY NAME",SuppbyName,SUPPLIER)<0)
		exit(-1);
	if( AddMenuItem("SUPPLIER ADDRESS LIST BY CODE",SuppLstNo,SUPPLIER)<0)
		exit(-1);
	if( AddMenuItem("SUPPLIER ADDRESS LIST BY NAME",SuppLstName,SUPPLIER)<0)
		exit(-1);
	if( AddMenuItem("LIST OF REQ BY NUMBER",ReqbyNo,REQHDR) <0)
		exit(-1);
	if( AddMenuItem("LIST OF REQ BY SUPPLIER",ReqbySupp,REQHDR) <0)
		exit(-1);
	if( AddMenuItem("LIST OF REQ BY COST CENTER",ReqbyCCno,REQHDR) <0)
		exit(-1);
	if( AddMenuItem("LIST OF REQ BY G/L ACCOUNT",ReqbyAcct,REQHDR) <0)
		exit(-1);
	if( AddMenuItem("REQ AGED TRIAL BALANCE",ReqAged,REQHDR) <0)
		exit(-1);
	if( AddMenuItem("LIST OF PROCESSED REQ",ProcReq,REQHDR) <0)
		exit(-1);
	if( AddMenuItem("LIST OF DISAPPROVED REQ",DisReq,REQHDR) <0)
		exit(-1);
	if( AddMenuItem("PRINT REQUISITIONS",PrntReq,REQHDR) <0)
		exit(-1);
	if( AddMenuItem("REQUISITION AUDIT",ReqAudit,REQHDR) <0)
		exit(-1);
	if(param.pa_stores[0] == YES) {
		if(AddMenuItem("LIST OF REQ BY STOCK CODE",ReqbyStck,REQHDR) <0)
			exit(-1);
	}
	if(param.pa_stores[0] == YES) {
		if(AddMenuItem("LIST OF INVENTORY STOCK CODES",StckRep,STMAST) <0)
			exit(-1);
	}
#else
	if( AddMenuItem("LISTE DES FOURNISSEURS PAR CODE",SuppbyNo,SUPPLIER)<0)
		exit(-1); 
	if( AddMenuItem("LISTE DES FOURNISSEURS PAR NOM",SuppbyName,SUPPLIER)<0)
		exit(-1); 
	if( AddMenuItem("LISTE D'ADRESSES FOURN PAR CODE",SuppLstNo,SUPPLIER)<0)
		exit(-1); 
	if( AddMenuItem("LISTE D'ADRESSES FOURN PAR NOM",SuppLstName,SUPPLIER)<0)
		exit(-1); 
	if( AddMenuItem("LISTE DE REQ PAR NUMERO DE REQU",ReqbyNo,REQHDR) <0)
		exit(-1);
	if( AddMenuItem("LISTE DE REQ PAR FOURNISSEUR",ReqbySupp,REQHDR) <0)
		exit(-1);
	if( AddMenuItem("LISTE DE REQ PAR CENTRE DE COUTS",ReqbyCCno,REQHDR) <0)
		exit(-1);
	if( AddMenuItem("LISTE DE REQ PAR COMPTE G/L",ReqbyAcct,REQHDR) <0)
		exit(-1);
	if( AddMenuItem("BALANCE CHRONOLOGIQUE DES REQ",ReqAged,REQHDR) <0)
		exit(-1);
	if( AddMenuItem("LISTE DES REQ TRAITEES",ProcReq,REQHDR) <0)
		exit(-1);
	if( AddMenuItem("LISTE DES REQ NON-APPROUVEES",DisReq,REQHDR) <0)
		exit(-1);
	if(param.pa_stores[0] == YES) {
		if( AddMenuItem("LISTE DE REQ PAR CODE DE STOCK",ReqbyStck,REQHDR) <0)
			exit(-1);
	}
	if(param.pa_stores[0] == YES) {
		if( AddMenuItem("LISTE DE CODES DE STOCK",StckRep,REQHDR) <0)
			exit(-1);
	}
#endif

	for(;;) {
		/* process the user's options */
#ifdef ENGLISH
		ret_option = Process(terminal, "  REQUISITION  REPORTS  ");
#else
		ret_option = Process(terminal, "RAPPORTS DE REQUISITIONS");
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
ReqbyNo()
{
	return(reqbyno(0));
}	
ReqbySupp()
{
	return(reqbysupp());
}	
ReqbyCCno()
{
	return(reqbyccno());
}	
ReqbyAcct()
{
	return(reqbyacct());
}	
ReqAged()
{
	return(reqaged());
}	
ProcReq()
{
	return(processedreq());
}	
DisReq()
{
	return(disapprovedreq());
}	
PrntReq()
{
	return(prntreq());
}	
ReqbyStck()
{
	return(reqbystck());
}	
StckRep()
{
	return(stockrep());
}	
ReqAudit()
{
	return(reqaudit());
}	
