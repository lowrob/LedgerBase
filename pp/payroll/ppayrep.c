/******************************************************************************
		Sourcename    : ppayrep.c
		System        : Employee Payroll Reports system.
		Module        : Personnel/Payroll
		Created on    : 92-04-02
		Created  By   : Robert Littlejohn.
		Cobol sources : 
*******************************************************************************
About this program:
	This program prints the following reports, calling some utilities from
	reputils.c. reputils.c uses a profom based screen and provides a  user
	interface for the below report routines for accepting key information.

	ytdlist ()
	Earninfo ();
	Employinfo ();
	Cppexempt ();
	Uicexempt ();
	Taxexempt ();
	Salrpt ();
	Subsalrpt ();
	Empbank ();
	Empben ();
	Empded ();
 	Mthben ();
	Mthded ();
	Empgarn ();
	
HISTORY:
 Date           Programmer     Description of modification
____/__/__      __________     ___________________________
*******************************************************************************/
#include <stdio.h>

#define MAIN
#define MAINFL		-1	/* no main file used */

#include <bfs_defs.h>

#define SYSTEM		"PERSONNEL/PAYROLL"
#define MOD_DATE	"10-DEC-91"
#define	EXIT		12	/* also defined in repemputils.c */

int	Ytdlist ();
int	Earninfo ();
int	Employinfo ();
int	Cppexempt ();
int	Uicexempt ();
int	Taxexempt ();
int	Salrpt ();
int	Subsalrpt ();
int	Empbank ();
int	Empben ();
int	Empded ();
int 	Mthben ();
int	Mthded ();
int	Empgarn ();
int	Pensalcont ();
int	Time_sheet ();

char	e_mesg[200];

/*
	Initialize  menu items not exceeding 9, 
	and call the menu processing routine

	The routines AddMenuItem() and Process() are defined in the file
	repemputils.c
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
	if( AddMenuItem("MONTHLY DEDUCTION REPORT",Mthded,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("PENSIONABLE SALARY / CONTRIBUTION",Pensalcont,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("PRINT TIME SHEETS",Time_sheet,EMPLOYEE)<0)
		exit(-1);
#else
	if( AddMenuItem("MONTHLY DEDUCTION REPORT",Mthded,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("PENSIONABLE SALARY / CONTRIBUTION",Pensalcont,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("PRINT TIME SHEETS",Time_sheet,EMPLOYEE)<0)
		exit(-1);
#endif
	for(;;) {
		/* process the user's options */
#ifdef ENGLISH
		ret_option = Process(terminal, "DEMOGRAPTHIC REPORTS"); 
#else
		ret_option = Process(terminal, "    TRANSALATION      ");
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
	}/*end of loop*/	
}
Ytdlist()
{
	fomen("not available");
	fomen("             ");
	Confirm ();
	return (0);
}
Earninfo()
{
	fomen("not available");
	fomen("             ");
	Confirm ();
	return (0);
}
Employinfo()
{
	fomen("not available");
	fomen("             ");
	Confirm ();
	return (0);
}
Cppexempt()
{
	fomen("not available");
	fomen("             ");
	Confirm ();
	return (0);
}
Uicexempt()
{
	fomen("not available");
	fomen("             ");
	Confirm ();
	return (0);
}
Taxexempt()
{
	fomen("not available");
	fomen("             ");
	Confirm ();
	return (0);
}
Salrpt()
{
	fomen("not available");
	fomen("             ");
	Confirm ();
	return (0);
}
Subsalrpt()
{
	fomen("not available");
	fomen("             ");
	Confirm ();
	return (0);
}
Empbank()
{
	fomen("not available");
	fomen("             ");
	Confirm ();
	return (0);
}
Empben()
{
	fomen("not available");
	fomen("             ");
	Confirm ();
	return (0);
}
Empded()
{
/*	return (empded());*/
}
Mthben()
{
	fomen("not available");
	fomen("             ");
	Confirm ();
	return (0);
}
Mthded()
{
	int	retval;

	retval = mthded();

	return (retval);
}
Empgarn()
{
	fomen("not available");
	fomen("             ");
	Confirm ();
	return (0);
}
Pensalcont()
{
	return (pensalcont());
}
Time_sheet()
{
	return (time_sheet());
}
