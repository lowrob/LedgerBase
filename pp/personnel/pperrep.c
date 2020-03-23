/******************************************************************************
		Sourcename    : pperrep.c
		System        : Employee Personnel Reports system.
		Module        : Personnel/Payroll
		Created on    : 92-02-25
		Created  By   : Robert Littlejohn.
		Cobol sources : 
*******************************************************************************
About this program:
	This program prints the following reports, calling some utilities from
	reputils.c. reputils.c uses a profom based screen and provides a  user
	interface for the below report routines for accepting key information.

	empsum ()
	teachresp ()
	subteach ()
	teachqual ()
	teachassign ()
	senlist ()
	empserv ()
	teachage ()
	empperc ()
	annivdate ()
	tchsal ()
	ageindex ()
	sickbank ()
	vaclia ()
	
HISTORY:
 Date           Programmer     Description of modification
____/__/__      __________     ___________________________
*******************************************************************************/
#include <stdio.h>

#define MAIN
#define MAINFL		-1	/* no main file used */

#include <bfs_defs.h>
#include <bobrpt.h>

#define SYSTEM		"PERSONNEL/PAYROLL"
#define MOD_DATE	"25-FEB-92"
#define	EXIT		12	

char	e_mesg[200];

int	Empsum ();
int	Subteach ();
int	Teachqual ();
int	Teachassign ();
int	Senlist ();
int	Empserv ();
int	Ageindx ();
int	Empinfo ();
int	Empperc ();
int	Annivdate ();
int	Empage ();
int	Sickbank ();
int	Vaclia ();

static	int	retval;

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
	if( AddMenuItem("EMPLOYEE SUMMARY",Empsum,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("LIST OF SUBSTITUTE TEACHERS",Subteach,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("EMPLOYEE QUALIFICATIONS",Teachqual,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("EMPLOYEE ASSIGNMENTS",Teachassign,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("SENIORITY LISTING",Senlist,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("EMPLOYEE YEARS OF SERVICE",Empserv,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("EMPLOYEE AGE INDEX",Ageindx,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("EMPLOYEE INFORMATION",Empinfo,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("EMPLOYEE PERCENTAGE LISTING",Empperc,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("EMPLOYEE ANNIVERSARY LISTING",Annivdate,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("EMPLOYEE AGE LISTING",Empage,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("SICK BANK FOR PRE 1985 SICK DAYS",Sickbank,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("VACATION PAY LIABILITY LISTING",Vaclia,EMPLOYEE)<0)
		exit(-1);
#else
	if( AddMenuItem("TRANSLATE               ",Empsum,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("TRANSLATE               ",Subteach,EMPLOYEE)<0) 
		exit(-1);
	if( AddMenuItem("TRANSLATE               ",Teachqual,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("TRANSLATE               ",Teachassign,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("TRANSLATE               ",Senlist,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("TRANSLATE               ",Empserv,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("TRANSLATE               ",Ageindx,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("TRANSLATE               ",Empinfo,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("TRANSLATE               ",Empperc,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("TRANSLATE               ",Annivdate,EMPLOYEE)<0)
		exit(-1);
#endif
	for(;;) {
		/* process the user's options */
#ifdef ENGLISH
		ret_option = Process(terminal, "     PERSONNEL REPORTS"); 
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
Empsum ()
{
	return(empsum());
}
Annivdate ()
{
	return(annivdate());
}
Subteach ()
{
	return(subteach());
}
Teachqual ()
{
	return(teachqual());
}
Teachassign ()
{
	return(teachassign());
}
Senlist ()
{
	return(senlist());
}
Empserv ()
{
	return(empserv());
}
Ageindx ()
{
	return(ageindex());
}
Empinfo ()
{
	return(empinfo());
}
Empperc ()
{
	return(empperc());
}
Empage ()
{
	return(empage());
}
Sickbank ()
{
	return(sickbank());
}
Vaclia ()
{
	return(vaclia());
}

