/******************************************************************************
		Sourcename    : roerep.c
		System        : Employee ROE Reports system.
		Module        : Personnel/Payroll
		Created on    : 08-FEB-93 
		Created  By   : m. galvin 
*******************************************************************************
About this program:
	This program prints the following reports, calling some utilities from
	reputils.c. reputils.c uses a profom based screen and provides a  user
	interface for the below report routines for accepting key information.

	roeaud()
	
HISTORY:
 Date           Programmer     Description of modification
____/__/__      __________     ___________________________
*******************************************************************************/
#include <stdio.h>

#define MAIN
#define MAINFL		-1	/* no main file used */

#include <bfs_defs.h>

#define SYSTEM		"PERSONNEL/PAYROLL"
#define MOD_DATE	"08-FEB-93"
#define	EXIT		12	/* also defined in repemputils.c */

int	roeaud ();

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
	if( AddMenuItem("ROE AUDIT LISTING",Roeaud,TIME)<0)
		exit(-1);
#else
	if( AddMenuItem("TRANSLATE               ",Roeaud,TIME)<0)
		exit(-1);
#endif
	for(;;) {
		/* process the user's options */
#ifdef ENGLISH
		ret_option = Process(terminal, "RECORD OF EMPLOYMENT REPORTS"); 
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

Roeaud()
{
	return (roeaud());
}
