/*
*	isamutil.c
*
*	Utility to traverse index, check index and recreate index.
*
*	If SW1 is on, allows traversing & Checking Index also.
*/

#define	MAIN

#define SYSTEM		"UTILITIES"
#define MOD_DATE	"23-JAN-90"

#include <stdio.h>
#include <bfs_defs.h>
#include <filein.h>

main(argc, argv)
int	argc ;
char	*argv[] ;
{
	int	i, j, retval ;
	char	buffer[80];
	char	e_mesg[80];
	int	file_no, selection ;

	/*
	*	Initialize DBH
	*/
	strncpy(SYS_NAME, SYSTEM, 50);
	strncpy(CHNG_DATE, MOD_DATE, 10);
	proc_switch(argc, argv, -1) ;

	if ( dist_no[0] == '\0' )
		set_dist() ;/* Ask district# to position in data directory */

	if(init_dbh() < 0) {
		printf("DBH Initialization ERROR.. DBERROR: %d\n",dberror);
		close_dbh() ;
		exit(-1) ;
	}

	retval = GetUserClass(e_mesg);
	if(retval < 0 || (retval != ADMINISTRATOR && retval != SUPERUSER)) {
		if(retval == DBH_ERR) 
			printf("\n%s\n",e_mesg);
		else
			printf("\n\n\n\tACCESS DENIED\n");
		close_dbh() ;
		exit(-1);
	}	

	for( ; ; ) {
		if(SW1) {
		    do {
			printf("\n\n\t\t\t\tU-ISAM UTILITIES\n\n\n\n");
			printf("\t\t\t\t0. QUIT\n\n");
			printf("\t\t\t\t1. TRAVERSE INDEX\n\n");
			printf("\t\t\t\t2. CHECK INDEX\n\n");
			printf("\t\t\t\t3. REBUILD INDEX\n\n");
			printf("\n\t\t\t\tOption: ");
			scanf("%d",&selection) ;
		    } while (selection < 0 || selection > 3) ;

		    if(selection == 0) break;
		}
		else 
			selection = 3 ;
		
		for( ; ; ) {
			printf("\n\n\t\t\t\t");
			switch(selection) {
			case 1: printf("TRAVERSE INDEX");
				break;
			case 2: printf("CHECK INDEX");
				break;
			case 3: printf("REBUILD INDEX");
				break;
			}
			printf("\n\n");
			/*
			*	Display File Names
			*/

			if(TOTAL_FILES > 18)
				j = TOTAL_FILES / 2 + TOTAL_FILES % 2 ;
			else
				j = TOTAL_FILES ;

			printf("\t 0. QUIT\n");

			for(i = 0 ; i < j ; i++) {
			    getuserflnm(i, buffer) ;
			    printf("\t%2d.",(i+1) );

			    /* check if index file */
			    if(getfiletype(i) == ISAM)
				printf(" %-20.20s", buffer) ;
			    else
				printf(" %-20.20s", " ") ;
			    if(j != TOTAL_FILES && (j+i) < TOTAL_FILES) {
			        getuserflnm(i+j, buffer) ;
				printf("\t%2d.",(j+i+1));
				/* check if index file */
				if(getfiletype(j+i) == ISAM)
					printf(" %s\n", buffer) ;
				else
					printf("\n") ;
			    }
			    else
				printf("\n") ;
			}

			do {
				printf("\n\t\tSelect File#: ");
				scanf("%d",&file_no) ;
			} while (file_no < 0 || file_no > TOTAL_FILES ||
				(file_no && getfiletype(file_no-1) != ISAM)) ;
	
			if(file_no == 0) break ;	/* QUIT */
	
			file_no-- ;
	
			getflnm(file_no, buffer) ;
			printf("\t\t***  %s  ***  Data File Selected\n",buffer);
	
			switch(selection) {
			case	1 :
				ixdisplay(file_no);
				break ;
			case	2 :
				ixcheck(file_no);
				break ;
			case    3 :
				ixrecreat(file_no);
				break ;
			}

			printf("\nPress RETURN to continue  ");
			fflush(stdout);
			read(0,buffer,80);
			fflush(stdin);
		}
		if(!SW1) break ;
	}

	close_dbh() ;
	exit(1);
}
/*--------------------------------------------------------------------*/
