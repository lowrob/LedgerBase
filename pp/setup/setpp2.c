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

	repcert()

HISTORY:
 Date           Programmer     Description of modification
____/__/__      __________     ___________________________
*******************************************************************************/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_pp.h>
#include <bfs_com.h>
#include <repdef.h>

#define SYSTEM		"PERSONNEL/PAYROLL"
#define MOD_DATE	"19-NOV-91"
#define	EXIT		12	/* also defined in reputils.c */

int	Inactive();
int	Classlist();
int	Regded();
int	Penplan();
int	Benlist();

char	e_mesg[80];

/*
	Initialize  menu items not exceeding 9, 
	and call the menu processing routine

	The routines AddMenuItem() and Process() are defined in the file
	reputils.c
*/ 
setpp2()
{
int	ret_option ;

	strncpy( SYS_NAME, SYSTEM, 20 ); 	/* Module name */
	strncpy( CHNG_DATE, MOD_DATE, 10 );	/* Last date of change */

	/* Add menu items in the following order */
#ifdef ENGLISH
	if( AddMenuItem("LIST OF CLASS CODES",Classlist,CLASSIFICATION)<0)
		exit(-1);
	if( AddMenuItem("LIST OF INACTIVATION CODES",Inactive,INACT_CODE)<0)
		exit(-1);
	if( AddMenuItem("LIST OF DEDUCTION CODES",Regded,DEDUCTION)<0)
		exit(-1);
	if( AddMenuItem("LIST OF REG. PENSION PLANS",Penplan,REG_PEN)<0)
		exit(-1);
	if( AddMenuItem("LIST OF BENEFITS",Benlist,BENEFIT)<0)
		exit(-1);
	if( AddMenuItem("OTHER MENU", RetSet(),CLASSIFICATION)<0)
		exit(-1);
#else
	if( AddMenuItem("LIST OF CLASS CODES",Classlist,CLASSIFICATION)<0)
		exit(-1);
	if( AddMenuItem("LIST OF INACTIVATION CODES",Inactive,INACT_CODE)<0)
		exit(-1);
	if( AddMenuItem("LIST OF DEDUCTION CODES",Regded,DEDUCTION)<0)
		exit(-1);
	if( AddMenuItem("LIST OF REG. PENSION PLANS",Penplan,REG_PEN)<0)
		exit(-1);
	if( AddMenuItem("LIST OF BENEFITS",Benlist,BENEFIT)<0)
		exit(-1);
	if( AddMenuItem("OTHER MENU", RetSet(),CLASSIFICATION)<0)
		exit(-1);
#endif
	for(;;) {
		/* process the user's options */
#ifdef ENGLISH
		ret_option = Process(terminal, " SETUP REPORTS (Screen 2) "); 
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
RetSet() 
{
	FillMenu();
	return(NOERROR);
}
