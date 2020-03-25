/*-----------------------------------------------------------------------
Source Name: jrnl_opt.c
Module     : DBH Utilities.

SYNOPSIS:
	Enable / Disable Journalling Options ..

	This programs reads DBH key descriptos file (KEY_DESC), displays
	current journalling status. If SW1 is ON journalling is enabled.
	If SW2 is ON journalling is disabled. If journalling is enabled,
	it initializes the size of journalling area to BUFSIZ.

Usage of SWITCHES when they are ON :

	SW1 - Enables Journalling.

	SW2 - Disables Journalling.

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

------------------------------------------------------------------------*/


#define	MAIN

#define SYSTEM		"UTILITIES"
#define MOD_DATE	"23-JAN-90"

#include <stdio.h>
#include <bfs_defs.h>


main(argc, argv)
int	argc ;
char	*argv[] ;			/* -dNN -s1 */
{
	int	k_fd ;			/* file fd for keys etc*/
	int	journaling ;
	char	file_name[50];
	long	lseek() ;

	/* Set The Environment */
	strncpy(SYS_NAME, SYSTEM, 50);
	strncpy(CHNG_DATE, MOD_DATE, 10);
	proc_switch(argc, argv, -1) ;

	/***
	if (dist_no[0] == '\0' ) 
		set_dist() ;
	form_f_name(KEY_DESC, file_name);
	****/
	strcpy(file_name, DATA_PATH);
	strcat(file_name, KEY_DESC) ;

	if ( (k_fd=open(file_name,RWMODE)) < 0  ) {
		printf("Key Descriptor File Could Not Be Accessed\n");
		exit(-1) ;
	}

	if(read(k_fd,(char*)&journaling,sizeof(int)) < sizeof(int)) {
		printf("ERROR while Reading\n");
		close(k_fd);
		exit(-1);
	}

	if ( journaling ) 
		printf("Currently Journalling is ENABLED\n");
	else
		printf("Currently Journalling is DISABLED\n");

	if ( SW1 )  {
		if ( journaling == 0) 
			printf("\n\nENABLING Journalling Now\n");
		journaling = 1 ;
	}
	else if ( SW2 ) {
		if ( journaling ) 
			printf("\n\nDISABLING Journalling Now\n");
		journaling = 0 ;
	}
	if( SW1 || SW2 ) {
		lseek(k_fd, 0L,0) ;
		if(write(k_fd,(char*)&journaling,sizeof(int)) < sizeof(int)) {
			printf("ERROR while Writing\n");
			close(k_fd);
			exit(-1);
		}

		if ( SW1 )  {
			journaling = BUFSIZ ;	/* Min Allocation */
			if(write(k_fd,(char*)&journaling,sizeof(int)) <
								sizeof(int)) {
				printf("ERROR while Writing\n");
				close(k_fd);
				exit(-1);
			}
		}
	}

	close(k_fd) ;
	exit(1) ;
}

