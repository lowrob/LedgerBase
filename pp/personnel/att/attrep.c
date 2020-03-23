/******************************************************************************
		Sourcename    : attrep.c
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

	empabsent ()
	negsick ()
	attabsent ()
	trendsatt ()
	attsum ()
	balsick ()
	unionrep()
	substute ()
	
HISTORY:
 Date           Programmer     Description of modification
____/__/__      __________     ___________________________
*******************************************************************************/
#include <stdio.h>

#define MAIN
#define MAINFL		-1	/* no main file used */

#include <bfs_defs.h>

#define SYSTEM		"PERSONNEL/PAYROLL"
#define MOD_DATE	"25-FEB-92"
#define	EXIT		12	

char	e_mesg[200];

int	Empabsent ();
int	Negsick ();
int	Attabsent ();
int	Trendsatt ();
int	Attsum ();
int	Balsick ();
int	Unionrep ();
int	Substute ();

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
	if( AddMenuItem("EMPLOYEE DETAILED ABSENT LIST",Empabsent,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("NEGATIVE SICK LEAVE LIST",Negsick,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("ATTENDANCE CODE TOTAL REPORT",Attabsent,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("ATTENDANCE TRENDS LISTING",Trendsatt,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("ATTENDANCE SUMMARY LISTING",Attsum,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("BALANCE SICK LEAVE LIST",Balsick,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("UNION SICK REPORT      ",Unionrep,EMPLOYEE)<0)
		exit(-1);
	if(AddMenuItem("SUBSTITUTE/REPLACEMENT TIME CLAIM",Substute,EMPLOYEE)<0)
		exit(-1);
#else
	if( AddMenuItem("EMPLOYEE DETAILED ABSENT LIST",Empabsent,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("NEGATIVE SICK LEAVE LIST",Negsick,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("ATTENDANCE CODE TOTAL REPORT",Attabsent,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("ATTENDANCE TRENDS LISTING",Trendsatt,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("ATTENDANCE SUMMARY LISTING",Attsum,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("BALANCE SICK LEAVE LIST",Balsick,EMPLOYEE)<0)
		exit(-1);
	if( AddMenuItem("UNION SICK REPORT      ",Unionrep ,EMPLOYEE)<0)
		exit(-1);
	if(AddMenuItem("SUBSTITUTE/REPLACEMENT TIME CLAIM",Substute,EMPLOYEE)<0)
		exit(-1);
#endif
	for(;;) {
		/* process the user's options */
#ifdef ENGLISH
		ret_option = Process(terminal, "     ATTENDANCE REPORTS"); 
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


Empabsent ()
{
	return(empabsent());
}

Negsick ()
{
	return(negsick());
}

Balsick ()
{
	return(balsick());
}

Unionrep()
{
	return(unionrep());
}

Attabsent ()
{
	return(attabsent());
}

Attsum ()
{
	return(attsum());
}

Trendsatt ()
{
	return(trendsatt());
}

Substute ()
{
	return(substute());
}
