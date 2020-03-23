/******************************************************************************
		Sourcename    : perset.c
		System        : Budgetary Financial system.
		Module        : Personnel/Payroll
		Created on    : 91-11-25
		Created  By   : Andre Cormier.
		Cobol sources : 
*******************************************************************************
About this program:
	This program prints the following reports, calling some utilities from
	reputils.c. reputils.c uses a profom based screen and provides a  user
	interface for the below report routines for accepting key information.

	terminat();
	inactive();
	repatt();

HISTORY:
 Date           Programmer     Description of modification
____/__/__      __________     ___________________________
*******************************************************************************/
#include <stdio.h>

#define MAIN
#define MAINFL		-1	/* no main file used */

#include <bfs_defs.h>

#define SYSTEM		"PERSONNEL/PAYROLL"
#define MOD_DATE	"25-NOV-91"
#define	EXIT		12	/* also defined in reputils.c */

int	Terminat();
int	Inactive();
int	Repatt();


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
	if( AddMenuItem("LIST OF TERMINATION CODES",Terminat,TERM)<0)
		exit(-1);
	if( AddMenuItem("LIST OF INACTIVATION CODES",Inactive,INACT_CODE)<0)
		exit(-1);
	if( AddMenuItem("LIST OF ATTENDANCE CODES",Repatt,ATT)<0)
		exit(-1);
#else
	if( AddMenuItem("LIST OF TERMINATION CODES",Terminat,TERM)<0)
		exit(-1);
	if( AddMenuItem("LIST OF INACTIVATION CODES",Inactive,INACT_CODE)<0)
		exit(-1);
	if( AddMenuItem("LIST OF ATTENDANCE CODES",Repatt,ATT)<0)
		exit(-1);
#endif
	for(;;) {
		/* process the user's options */
#ifdef ENGLISH
		ret_option = Process(terminal, " PERSONNEL SETUP REPORTS "); 
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
Inactive()
{
	return(inactive());
}
Repatt()
{
	return(repatt());
}

