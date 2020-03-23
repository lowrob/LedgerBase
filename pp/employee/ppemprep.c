/******************************************************************************
		Sourcename    : ppemprep.c
		System        : Employee Payroll Reports system.
		Module        : Personnel/Payroll
		Created on    : 91-12-10
		Created  By   : Robert Littlejohn.
		Cobol sources : 
*******************************************************************************
About this program:
	This program prints the following reports, calling some utilities from
	reputils.c. reputils.c uses a profom based screen and provides a  user
	interface for the below report routines for accepting key information.

	emplist ()
	empaddlist ()
	stdate (1)
	stdate (2)
	stdate (3)
	stdate (4)
	inactemp ()
	termdate ()
	termcode ()
	printlabels ()
	emplistsum ()
	complbl ()
	strep ()
	
HISTORY:
 Date           Programmer     Description of modification
____/__/__      __________     ___________________________
1992/08/24	m. galvin	Added the empcntrct.c program
				to the menu.
1992/09/02	m. galvin	Added the emplist.c program to
				the menu.
*******************************************************************************/
#include <stdio.h>

#define MAIN
#define MAINFL		-1	/* no main file used */

#include <bfs_defs.h>

#define SYSTEM		"PERSONNEL/PAYROLL"
#define MOD_DATE	"92-SEP-02"
#define	EXIT		12	/* also defined in repemputils.c */

int	Emplist ();
int	Empaddlist ();	/*mkln*/
int	Ftempdate ();
int	Ptempdate ();
int	Caempdate ();
int	Suempdate ();
int	Inactemps ();
int	Termdate ();
int	Termcode ();
int	Printlabels ();
int	Emplistsum ();
int	Complbl ();
int	Strep ();

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
	if( AddMenuItem("LIST OF EMPLOYEES",Emplist,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("LIST OF ADDRESSES",Empaddlist,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("LIST OF EMP. BY START DATE FT",Ftempdate,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("LIST OF EMP. BY START DATE PT",Ptempdate,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("LIST OF EMP. BY START DATE CA",Caempdate,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("LIST OF EMP. BY START DATE SU",Suempdate,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("LIST OF INACTIVE EMPLOYEES",Inactemps,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("LIST OF EMP BY TERMINATION DATE",Termdate,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("LIST OF EMP BY TERMINATION CODE",Termcode,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("PRINT LABELS",Printlabels,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("EMPLOYEE SUMMARY LISTING",Emplistsum,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("PRINT EMPLOYEE COMPETITION LABELS",Complbl,EMP_COMP)<0)
		exit(-1);
	if( AddMenuItem("EMPLOYEE STATUS REPORT",Strep,EMPLOYEE)<0)
		exit(-1);
#else
	if( AddMenuItem("TRANSLATE               ",Emplist,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("TRANSLATE               ",Empaddlist,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("TRANSLATE               ",Ptempdate,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("TRANSLATE               ",Caempdate,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("TRANSLATE               ",Suempdate,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("TRANSLATE               ",Inactemps,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("TRANSLATE               ",Termdate,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("TRANSLATE               ",Termcode,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("TRANSLATE               ",Printlabels,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("TRANSLATE               ",Emplistsum,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("TRANSLATE               ",Complbl,EMP_COMP)<0)
		exit(-1);
#endif
	for(;;) {
		/* process the user's options */
#ifdef ENGLISH
		ret_option = Process(terminal, "DEMOGRAPHIC REPORTS"); 
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
	}	
}

Emplist()
{
	return(emplist());
}

Empaddlist()
{
	return(empaddlist());
}

Ftempdate()
{
	return(stdate(1));
}

Ptempdate()
{
	return(stdate(2));
}

Caempdate()
{
	return(stdate(3));
}

Suempdate()
{
	return(stdate(4));
}

Inactemps()
{
	return(inactlist());
}

Termdate()
{
	return(termdate());
}

Termcode()
{
	return(termcode());
}

Emplistsum()
{
	return(emplistsum());
}

Printlabels()
{
	return (prntlbl());
}
Complbl()
{
	return (complbl());
}
Strep()
{
	return (strep());
}
