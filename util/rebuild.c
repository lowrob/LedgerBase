/*
*	dispfile.c
*
*	Program to display field contents of given files using .DEF file.
*/

#define	MAIN
#define SYSTEM		"UTILITIES"
#define MOD_DATE	"17-JLY-91"

#include <stdio.h>
#include <bfs_defs.h>
#include <isnames.h>
#include <fld_defs.h>
#include <filein.h>

static	char	c_mesg[100];

static	int	code;

main(argc, argv)
int	argc ;
char	*argv[] ;
{
	int	i ;

	/*
	*	Initialize DBH
	*/

	strncpy(SYS_NAME, SYSTEM, 50);
	strncpy(CHNG_DATE, MOD_DATE, 10);
	proc_switch(argc, argv, -1) ;

	if ( dist_no[0] == '\0' )
		set_dist() ;/* Ask district# to position in data directory */

#ifdef	SECURITY
	i = GetUserClass(c_mesg);
	if(i < 0 || (i != SUPERUSER && i != ADMINISTRATOR)) {
		if(i == DBH_ERR) 
			printf("\n%s\n",c_mesg);
		else
			printf("\n\n\n\tACCESS DENIED\n");
		close_dbh() ;
		exit(-1);
	}
#endif

	/* Find out Max Reclen */
	for(i = 0 ; i < TOTAL_FILES ; i++)
		ixrecreat(i) ;

		
	close_dbh() ;
	exit(1);
}
/*--------------------------------------------------------------------*/

