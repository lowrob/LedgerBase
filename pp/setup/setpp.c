/******************************************************************************
		Sourcename    : setrep.c
		System        : Budgetary Financial system.
		Module        : Personnel/Payroll
		Created on    : 91-11-19
		Created  By   : Andre Cormier.
		Cobol sources : 
*******************************************************************************
About this program:
	This program prints the following reports, calling some utilities from
	reputils.c. reputils.c uses a profom based screen and provides a  user
	interface for the below report routines for accepting key information.

	Pptable()
	PosRep()
	PayRep()
	BargUnit()
	TaxTable()
	EarnCode()
	Repgl()
	CostCent()
	RepDept()
	RepArea()
	RepStat()
	RepCsbl()
	RepBank()

HISTORY:
 Date           Programmer     Description of modification
____/__/__      __________     ___________________________
*******************************************************************************/
#include <stdio.h>

#define MAIN
#define MAINFL		-1	/* no main file used */

#include <bfs_defs.h>

#define SYSTEM		"PERSONNEL/PAYROLL"
#define MOD_DATE	"19-NOV-91"
#define	EXIT		12	/* also defined in reputils.c */

int	Pptable();
int	PosRep();
int	PayRep();
int	BargUnit();		/* mkln */
int	TaxTable();
int	EarnCode();
int	Repgl();
int	CostCent();
int	RepDept();
int	RepArea();
int	RepStat();
int	RepCsbl();
int	RepBank();
int	RepAtt();
int	RepArea_spec();
int	Terminat();
int	Inactive();
int	Rep_senpar();
int	Classlist();
int	Regded();
int	Penplan();
int	Benlist();
int	Setpp2();

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
	if( AddMenuItem("LIST OF COST CENTERS",CostCent,SCHOOL)<0)
		exit(-1);
	if( AddMenuItem("LIST OF DEPARTMENTS",RepDept,DEPARTMENT)<0)
		exit(-1);
	if( AddMenuItem("LIST OF AREAS",RepArea,AREA)<0)
		exit(-1);
	if( AddMenuItem("LIST OF POSITIONS",PosRep,POSITION)<0)
		exit(-1);
	if( AddMenuItem("LIST OF BARGAINING UNITS ",BargUnit,BARG)<0)
		exit(-1);
	if( AddMenuItem("LIST OF BANK LOCATIONS",RepBank,BANK)<0)
		exit(-1);
	if( AddMenuItem("PAY PERIOD TABLE",Pptable,PAY_PERIOD)<0)
		exit(-1);
	if( AddMenuItem("UIC TABLE",PayRep,UIC)<0)
		exit(-1);
	if( AddMenuItem("TAX TABLE",TaxTable,TAX)<0)
		exit(-1);
	if( AddMenuItem("LIST OF EARNING CODES",EarnCode,EARN)<0)
		exit(-1);
	if( AddMenuItem("LIST OF CSB/LOAN CODES",RepCsbl,LOAN)<0)
		exit(-1);
	if( AddMenuItem("LIST OF STATUTORY HOLIDAYS",RepStat,STAT_HOL)<0)
		exit(-1);
	if( AddMenuItem("LIST OF PAYROLL G/L ACCOUNTS",Repgl,GLACCT)<0)
		exit(-1);
	if( AddMenuItem("LIST OF ATTENDANCE CODES",RepAtt,ATT)<0)
		exit(-1);
	if( AddMenuItem("LIST OF TERMINATION CODES",Terminat,TERM)<0)
		exit(-1);
	if(AddMenuItem("AREA OF SPECIALIZATION CODES",RepArea_spec,AREA_SPEC)<0)
		exit(-1);
	if( AddMenuItem("SENIORITY PARAMETER LISTING",Rep_senpar,SEN_PAR)<0)
		exit(-1);
	if( AddMenuItem("MORE REPORTS", Setpp2,ATT)<0)
		exit(-1);
#else
	if( AddMenuItem("LIST OF COST CENTERS",CostCent,SCHOOL)<0)
		exit(-1);
	if( AddMenuItem("LIST OF DEPARTMENTS",RepDept,DEPARTMENT)<0)
		exit(-1);
	if( AddMenuItem("LIST OF AREAS",RepArea,AREA)<0)
		exit(-1);
	if( AddMenuItem("LIST OF POSITIONS",PosRep,POSITION)<0)
		exit(-1);
	if( AddMenuItem("LIST OF BARGAINING UNITS ",BargUnit,BARG)<0)
		exit(-1);
	if( AddMenuItem("LIST OF BANK LOCATIONS",RepBank,BANK)<0)
		exit(-1);
	if( AddMenuItem("PAY PERIOD TABLE",Pptable,PAY_PERIOD)<0)
		exit(-1);
	if( AddMenuItem("UIC TABLE",PayRep,UIC)<0)
		exit(-1);
	if( AddMenuItem("TAX TABLE",TaxTable,TAX)<0)
		exit(-1);
	if( AddMenuItem("LIST OF EARNING CODES",EarnCode,EARN)<0)
		exit(-1);
	if( AddMenuItem("LIST OF CSB/LOAN CODES",RepCsbl,LOAN)<0)
		exit(-1);
	if( AddMenuItem("LIST OF STATUTORY HOLIDAYS",RepStat,STAT_HOL)<0)
		exit(-1);
	if( AddMenuItem("LIST PAYROLL G/L ACCOUNT",Repgl,GLACCT)<0)
		exit(-1);
	if( AddMenuItem("LIST OF ATTENDANCE CODES",RepAtt,ATT)<0)
		exit(-1);
	if( AddMenuItem("LIST OF TERMINATION CODES",Terminat,TERM)<0)
		exit(-1);
	if( AddMenuItem("AREA OF SPECIALIZATION CODES",RepArea_spec,AREA_SPEC)<0)
		exit(-1);
	if( AddMenuItem("SENIORITY PARAMETER LISTING",Rep_senpar,SEN_PAR)<0)
		exit(-1);
	if( AddMenuItem("MORE REPORTS", Setpp2,ATT)<0)
		exit(-1);
#endif
	for(;;) {
		/* process the user's options */
#ifdef ENGLISH
		ret_option = Process(terminal, "PERSONNEL/PAYROLL SETUP REPORTS "); 
#else
		ret_option = Process(terminal, "         TRANSALATION         ");
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

Terminat()
{
	return(terminat());
}

Rep_senpar()
{
	return(rep_senpar());
}

RepArea_spec()
{
	return(reparea_spec());
}

RepAtt()
{
	return(repatt());
}

Pptable()
{
	return(pptable());
}

PayRep()
{
	return(pay_rep());
}
BargUnit()
{
	return(bargunit());
}
TaxTable()
{
	return(taxtable());
}
EarnCode()
{
	return(earncode());
}
PosRep()
{
	return(posrep());
}
Repgl()
{
	return(repgl());
}
CostCent()
{
	return(costcent());
}
RepDept()
{
	return(repdept());
}
RepArea()
{
	return(reparea());
}
RepStat()
{
	return(repstat());
}
RepCsbl()
{
	return(repcsbl());
}
RepBank()
{
	return(repbank());
}

Setpp2()
{
	int	ret_option;
	InitOption();
	return(setpp2());
}
Benlist()
{
	return(benlist());
}

Penplan()
{
	return(pen_plan());
}

Regded()
{
	return(regded());
}

Inactive()
{
	return(inactive());
}

Classlist()
{
	return(classlist());
}

