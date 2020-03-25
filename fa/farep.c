/******************************************************************************
		Sourcename    : farep.c
		System        : Budgetary Financial system.
		Module        : Inventory reports
		Created on    : 89-09-15
		Created  By   : J PRESCOTT.
		Cobol sources : 
*******************************************************************************
About this program:
	This program prints the following reports, calling some utilities from
	fareputl.c. 
		
	interface for the below report routines for 
		1.Report selection
		2.Accepting key information.

	The name of the option should not be longer than OPTIONLEN defined in
	fareputl.c



HISTORY:
 Date           Programmer     Description of modification
____/__/__      __________     ___________________________
1990/11/22      J. Cormier     added FA report by current cost centers
1990/11/26      J. Cormier     added Obsolete FA report by original cost centers
*******************************************************************************/
#include <stdio.h>

#define MAIN
#define MAINFL		-1	/* no main file used */

#include <bfs_defs.h>

#define SYSTEM		"FIXED ASSETS"
#define MOD_DATE	"23-JAN-90"
#define	EXIT		12	/* also defined in streputl.c */

int	FabyCost();		/* fixed asset report by original cost center */
int	FabyItem();		/* fixed asset report by item id */
int	FabyDept();		/* fixed asset report by dept. no */
int	FabyCuCost();		/* fixed asset report by current cost center */
int	FaObsolete();		/* obsolete fixed assets by orig. cost center */
int	FabyRoom();		/* report by current cost center and room */
int	Trfbyoriginal();	/* fa transfers by original cost center */
int 	Trfbycurrent();		/* fa transfers by current cost center */
int	Trfbydate();		/* fa transfers by date */

char	e_mesg[80];

/*
	Initialize  menu items not exceeding 9, 
	and call the menu processing routine

	The routines AddMenuItem() and Process() are defined in the file
	streputl.c
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
	if( AddMenuItem("REPORT BY DEPARTMENT NUMBER",FabyDept,FAMAST)<0)
		exit(-1);
	if( AddMenuItem("REPORT BY ASSET TYPE",FabyItem,FAMAST)<0)
		exit(-1);
	if( AddMenuItem("REPORT OF OBSOLETE FIXED ASSETS",FaObsolete,FAMAST)<0)
		exit(-1);
	if( AddMenuItem("REPORT BY ORIG COST CENTER",FabyCost,FAMAST)<0)
		exit(-1);
	if( AddMenuItem("REPORT BY CURR COST CENTER",FabyCuCost,FAMAST)<0)
		exit(-1);
	if(AddMenuItem("REPORT BY CURR COST CENTER AND ROOM",FabyRoom,FAMAST)<0)
		exit(-1);
	if( AddMenuItem("TRANSFERS BY ORIG COST CENTER",Trfbyoriginal,FATRAN)<0)
		exit(-1);
	if( AddMenuItem("TRANSFERS BY CURR COST CENTER",Trfbycurrent,FATRAN)<0)
		exit(-1);
	if( AddMenuItem("TRANSFERS BY DATE",Trfbydate,FATRAN)<0)
		exit(-1);
#else
	if( AddMenuItem("LISTE D'AI PAR NUMERO DE DEPT",FabyDept,FAMAST)<0)
		exit(-1);
	if( AddMenuItem("LISTE D'AI PAR GENRE D'ACTIF",FabyItem,FAMAST)<0)
		exit(-1);
	if( AddMenuItem("LISTE DES AI OBSOLETE",FaObsolete,FAMAST)<0)
		exit(-1);
	if( AddMenuItem("LISTE D'AI/ CENTRE (ORIG) DE COUTS",FabyCost,FAMAST)<0)
		exit(-1);
	if( AddMenuItem("LISTE D'AI/ CENTRE (COUR) DE COUTS",FabyCuCost,FAMAST)<0)
		exit(-1);
	/* XXXXXX */
	if( AddMenuItem("AI/ CENTRE COUR DE COUTS ET LOCAUX",FabyRoom,FAMAST)<0)
		exit(-1);
	if( AddMenuItem("TRANS PAR CENTRE DE COUTS ORIGINAL",Trfbyoriginal,FATRAN)<0)
		exit(-1);
	if( AddMenuItem("TRANS PAR CENTRE DE COUTS COURANT",Trfbycurrent,FATRAN)<0)
		exit(-1);
	if( AddMenuItem("TRANSFERTS PAR DATE",Trfbydate,FATRAN)<0)
		exit(-1);
#endif
	for(;;) {
		/* process the user's options */
#ifdef ENGLISH
		ret_option = Process(terminal, "  FA  REPORTS");
#else
		ret_option = Process(terminal, "RAPPORTS DES AI");
#endif

		switch( ret_option) {
			case 0 :  exit(0) ;
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
FabyCost()			/* parameter: key-0 of file */
{
	return(farep1(0));
}
FabyItem()			/* parameter: key-1 of file */
{
	return(farep1(1));
}
FabyDept()			/* parameter: key-2 of file */
{
	return(farep1(2));
}
FabyCuCost()			/* parameter: key-3 of file */
{
	return(farep1(3));
}
FabyRoom()			/* parameter: key-4 of file */
{
	return(fabyroom());
}
FaObsolete()
{
	return(farpt());
}
Trfbyoriginal()
{
	return(farep2(1));
}
Trfbycurrent()
{
	return(farep2(2));
}
Trfbydate()
{
	return(farep2(3));
}
