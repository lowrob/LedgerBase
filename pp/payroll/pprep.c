/******************************************************************************
		Sourcename    : pprep.c
		System        : Budgetary Financial system.
		Module        : Personnel/Payroll
		Created on    : 91-10-26
		Created  By   : Cathy Burns.
		Cobol sources : 
*******************************************************************************
About this program:
	This program prints the following reports, calling some utilities from
	reputils.c. reputils.c uses a profom based screen and provides a  user
	interface for the below report routines for accepting key information.

	empppben();
	rpposition();
	adjust();
	pp_reg();
	neg_pay();
	earn_per();
	earn_cod();
	jourlist();
	bal_pay();

HISTORY:
 Date           Programmer     Description of modification
____/__/__      __________     ___________________________
*******************************************************************************/
#include <stdio.h>

#define MAIN
#define MAINFL		-1	/* no main file used */

#include <bfs_defs.h>

#define SYSTEM		"PERSONNEL/PAYROLL"
#define MOD_DATE	"26-OCT-91"
#define	EXIT		12	/* also defined in reputils.c */

int	Empppben();
int	Rpposition();
int	Adjust();
int	Pp_reg();		/* mkln() */
int	Neg_pay();		/* mkln() */
int	Earn_per();		/* mkln() */
int	Earn_cod();		/* mkln() */
int	Jourlist();
int	Cheqreg();
int	Bal_pay();


char	e_mesg[200];

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
	if( AddMenuItem("EMPLOYEE PAY PERIOD BENEFIT LIST",Empppben,PP_BEN)<0)
		exit(-1);
	if( AddMenuItem("GROSS EARNINGS BY POSITION",Rpposition,PP_EARN)<0)
		exit(-1);
	if( AddMenuItem("PAYROLL ADJUSTMENT AUDIT TRAIL",Adjust,AUD_PAY)<0)
		exit(-1);
	if( AddMenuItem("PAY PERIOD REGISTER",Pp_reg,PP_EARN)<0)
		exit(-1);
	if( AddMenuItem("NEGATIVE PAY LIST",Neg_pay,PP_EARN)<0)
		exit(-1);
	if( AddMenuItem("GROSS EARNINGS FOR PERIOD",Earn_per,EMP_SCHED1)<0)
		exit(-1);
	if( AddMenuItem("GROSS EARNINGS BY EARNING CODE",
		Earn_cod,EMP_SCHED1)<0)
		exit(-1);
	if( AddMenuItem("JOURNAL LISTING",Jourlist,JR_ENT)<0)
		exit(-1);
	if( AddMenuItem("CHEQUE REGISTER",Cheqreg,JR_ENT)<0)
		exit(-1);
	if( AddMenuItem("BALANCE REGISTER TO JOURNAL",Bal_pay,JR_ENT)<0)
		exit(-1);
#else
	if( AddMenuItem("TRANSLATE               ",Empppben,PP_BEN)<0)
		exit(-1);
	if( AddMenuItem("TRANSLATE               ",Rpposition,PP_EARN)<0)
		exit(-1);
	if( AddMenuItem("TRANSLATE               ",Adjust,AUD_PAY)<0)
		exit(-1);
	if( AddMenuItem("TRANSLATE               ",Pp_reg,PP_EARN)<0)
		exit(-1);
	if( AddMenuItem("TRANSLATE               ",Neg_pay,PP_EARN)<0)
		exit(-1);
	if( AddMenuItem("TRANSLATE               ",Earn_cod,EMP_SCHED1)<0)
		exit(-1);
	if( AddMenuItem("JOURNAL LISTING",Jourlist,JR_ENT)<0)
		exit(-1);
	if( AddMenuItem("CHEQUE REGISTER",Cheqreg,JR_ENT)<0)
		exit(-1);
	if( AddMenuItem("BALANCE REGISTER TO JOURNAL",Bal_pay,JR_ENT)<0)
		exit(-1);
#endif
	for(;;) {
		/* process the user's options */
#ifdef ENGLISH
		ret_option = Process(terminal, " CHEQUE PREPROCESSING REPORTS "); 
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

Cheqreg()
{
	return(cheqreg(1));
}

Empppben()
{
	return(empppben());
}


Rpposition()
{
	return(rpposition());
}

Adjust()
{
	return(adjust());
}
Pp_reg()
{
	return(pp_reg());
}
Neg_pay()
{
	return(neg_pay());
}
Earn_per()
{
	return(earn_per());
}
Earn_cod()
{
	return(earn_cod());
}
Jourlist()
{
	return(jourlist(0));
}
Bal_pay()
{
	return(bal_pay());
}
