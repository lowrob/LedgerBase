/*
*	add_dist.c
*
*	Programme to create necessary directories to create a new district ..
*/

#define	MAIN

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define SYSTEM		"UTILITIES"
#define MOD_DATE	"23-JAN-90"

#define	INIT_ID		"init_id"	/* DBH initialization program */
#define	SUPERUSR	"superusr"	/* Super User Creation Program */
#define	MAX_DIST	99		/* Maximum District# */

char	in_str[80] ;

main(argc,argv)
int	argc ;
char	*argv[] ;
{
	int	i, j, code ;
	char	dist[20] ;

	/* Set The environment */
	strncpy(SYS_NAME, SYSTEM, 50);
	strncpy(CHNG_DATE, MOD_DATE, 10);
	proc_switch(argc, argv, -1) ;

	strcpy(in_str, DATA_PATH) ;
	j = strlen(in_str) ;

	for(i=0; i < MAX_DIST ; i++) {
		sprintf(in_str+j, "%02d", i) ;
		if ( access(in_str, RDMODE) == 0 )
			printf("District: %02d Exists\n", i) ;
	}

	for(;;) {
		printf("Give District Number (1-%d, 0-To Quit): ", MAX_DIST);
		scanf("%s", dist) ; 
		if ( dist[0] < '0' || dist[0] > '9' ) continue ;
	
		if ( dist[1] != '\0' ) {
			if ( dist[1] < '0' || dist[1] > '9' )
				continue ;
			else if ( dist[2] != '\0' )
				continue ;
			else
				break ;
		}
		else  {
			dist[1] = dist[0] ;
			dist[0] = '0' ;
			dist[2] = '\0' ;
			break ;
		}
	}
	if ( dist[0] == '0' && dist[1] == '0' ) {
		printf( "Exiting Without District Creation !!!\n");
		exit(-1) ;
	}

	strcpy(in_str, "mkdir ");
	strcat(in_str, DATA_PATH) ;
	strcat(in_str, dist) ;

	code = system(in_str) ;

	if ( code != 0 ) { 
		printf("Command: %s Failed !!!\n", in_str);
		printf("Check if Directory already exists; Check Permissions\n");
		exit(-1) ;
	}

	strcpy(dist_no, dist) ;

#ifndef	ORACLE
	form_f_name(BACK_UP, in_str + strlen("mkdir ") ) ;

	system(in_str) ;

	form_f_name(PREV_YEAR, in_str + strlen("mkdir ") ) ;

	system(in_str) ;
#endif

	printf("Initialising DataBase ... \n\n");

	sprintf(in_str, "%s%s%s -d%s", EXE_PATH, INIT_ID, EXTN, dist_no);

	system( in_str ) ; 

#ifdef SECURITY
	printf("Creating Super User ... \n\n");

	sprintf(in_str, "%s%s%s -d%s", EXE_PATH, SUPERUSR, EXTN, dist_no);

	system( in_str ) ; 

	close_dbh() ;
#endif

	exit(1) ;
}
/*---------------------------------END OF FILE---------------------*/

