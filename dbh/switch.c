/*-----------------------------------------------------------------------
	Source Name: switch.c
	System     : Budgetary Financial system.
	Created  On: 2nd May 89.
	Created  By: Kavindra Sharma.

	This function sets the environment of the system. Gets LBASE directory
	name and sets all the PATH names. Parses run time arguments and set
	PROG_NAME, terminal PROFOM name, district# and Switches. It also
	gets CTOOLS environment variable and sets CTOOLS_PATH.

	For ORACLE version to set the paths two environment variables are used.
	They are LBASE1 and LBASE2. LBASE1 is used to set DATA_PATH, EXE_PATH
	and WORK_DIR. LBASE2 is used to set NFM_PATH and FMT_PATH. (This was
	required, when the ORACLE version developed make files and executable
	files are kept in diffrent user account and .c, nfm and report formats
	are kept LBase ISAM version account).

	If all the switches are set to ON, then this function displays version
	details and exits.

MODIFICATIONS:

Programmer	YY/MM/DD	Description of modification
~~~~~~~~~~	~~~~~~~~	~~~~~~~~~~~~~~~~~~~~~~~~~~~
S Osborne	91/03/14	Added code to have cost center default to 99
				if it is not specified.
L Robichaud	97/01/03	changed the cost center to default to 900
------------------------------------------------------------------------*/

#include <stdio.h>
#include <bfs_defs.h>

#ifdef	MS_DOS

#define	DATA_DIR	"\\data\\"	/* Data files directory */
#define	NFM_DIR		"\\nfm\\"	/* PROFOM .nfm files directory */
#define	EXE_DIR		"\\bin\\"	/* Executable files directory */
#define	FMT_DIR		"\\formats\\"	/* Repgen Format files directory */

#define	WORK		"\\work"	/* Work Directory */

#else	/* UNIX compatible systems */

#define	DATA_DIR	"/data/"	/* Data files directory */

#define	NFM_DIR		"/nfm/"	     /* PROFOM .nfm files directory */
#define	EXE_DIR		"/bin/"	     /* Executable files directory */
#define	FMT_DIR		"/formats/"  /* Repgen Format files directory */

#define	WORK		"/work"		/* Work Directory */

char	*getlogin();

#endif

#ifdef	ORACLE
extern	char	owner_prefix[] ;
extern	char	tablespace[] ;
#endif

/*--------------------------------------------------------------------*/
#ifdef SECURITY

proc_switch(argc, argv, file_no)
int	argc ;
char	*argv[] ;
int	file_no;

#else

proc_switch(argc, argv)
int	argc ;
char	*argv[] ;

#endif
{
	int 	i, j ;
	char	*s_ptr ;
	char	*ctools ;
	char	*home_dir ;
	char	*getenv() ;
	char	e_mesg[80];
#ifdef	ORACLE
	FILE	*fp, *fopen();
	char	*getpass(), *home_dir2 ;
	char	tnum[5];
#endif

	/*
	*	Copy the program name in UpperCase to PROG_NAME form argv[0]
	*	upto '.'. Supress the path name if it exists.
	*/ 

	j = strlen(argv[0]) ;	/* Length of the program name */

	/* Find out the first slash, in backwards */
	for( j-- ; j > 0 ; j--)
		if(*(argv[0]+j) == '/' || *(argv[0]+j) == '\\'){
			j++ ;	/* Position to 1st char after '/' */
			break ;
		}

	s_ptr = argv[0]+j ;

	/* Copy the program name up to '.' */
	for(i = 0 ; *s_ptr && i < 10 && *s_ptr != '.' ; s_ptr++, i++) 
		*(PROG_NAME+i) = ( *s_ptr >= 'a' && *s_ptr <= 'z' ) 
					? *s_ptr +'A'-'a' : *s_ptr ; 

	PROG_NAME[i] = '\0'; 

	/* Set the terminal, district and switches from argumnets. Search
	   for following to set the above arguments.

		-t	- Terminal name
		-s	- Switches
		-d	- District#
		-c	- School/Cost Center
	*/

#ifndef MS_DOS
	s_ptr = getlogin();
	strncpy( User_Id, s_ptr, 10 ) ;
#else
	User_Id[0] = '\0';
#endif

	terminal[0] = '\0' ;
	dist_no[0] = '\0' ;
	CC_code[0] = '\0' ;
	SW1 = SW2 = SW3 = SW4 = SW5 = SW6 = SW7 = SW8 = SW9 = 0 ;
				/* Off All switches */

	for ( i = 1 ; i < argc ; i++) {
	    if ( argv[i][0] == '-') {
		switch(argv[i][1]) {	/* argument type */
		case 's' :
		case 'S' :
			for( j = 2 ; j < strlen(argv[i]) ; j++)
				switch( argv[i][j] ) {
				case '1' :
					SW1 = 1 ;
					break ;
				case '2' :
					SW2 = 1 ;
					break ;
				case '3' :
					SW3 = 1 ;
					break ;
				case '4' :
					SW4 = 1 ;
					break ;
				case '5' :
					SW5 = 1 ;
					break ;
				case '6' :
					SW6 = 1 ;
					break ;
				case '7' :
					SW7 = 1 ;
					break ;
				case '8' :
					SW8 = 1 ;
					break ;
				case '9' :
					SW9 = 1 ;
					break ;
				} /* switch */
			break ;
		case 't' :
		case 'T' :
			strcpy(terminal, argv[i]+2);
			break ;
		case 'd' :
		case 'D' :
			dist_no[0] = argv[i][2]   ;
			if ( (dist_no[1] = argv[i][3]) == '\0') {
				dist_no[0] = '0' ;
				dist_no[1] = argv[i][2]   ;
			}
			dist_no[2] = '\0' ;
			break ;
		case 'c' :
		case 'C' :
			CC_code[0] = argv[i][2]   ;
			if ( (CC_code[1] = argv[i][3]) == '\0') {
				CC_code[0] = '0' ;
				CC_code[1] = '0' ;
				CC_code[2] = argv[i][2];
			}
			else
			if ( (CC_code[2] = argv[i][4]) == '\0') {
				CC_code[0] = '0' ;
				CC_code[1] = argv[i][2];
				CC_code[2] = argv[i][3];
			}

			CC_code[3] = '\0' ;
			break ;
		}
	    }	/* If argv[][] == '-' */
	}	/* For */

	/* SBO	910314	Set cost center to 99 if it has not been set.	*/
	if (CC_code[0] == '\0')
		STRCPY(CC_code,"900");
	CC_no = atoi(CC_code);

	/* If all the switches are ON print the Version details
	   and exit */
	if(SW1 && SW2 && SW3 && SW4 && SW5 && SW6 && SW7 && SW8 && SW9) {
		printf("\n\n\n\n\n\n");
		printf("\n\t**   %s   **\n\n\n",SYS_NAME) ;
#ifdef ENGLISH
		printf("\n\tName of Program  : %s\n",PROG_NAME);
		printf("\n\tSource Version   : %s\n",VERSION_NO);
		printf("\n\tRelease Number   : %s\n",RELEASE_NO);
		printf("\n\tDate Last Changed: %s\n",CHNG_DATE);
#else
		printf("\n\tNom du programme          : %s\n",PROG_NAME);
		printf("\n\tVersion de source         : %s\n",VERSION_NO);
		printf("\n\tRelacher le numero        : %s\n",RELEASE_NO);
		printf("\n\tDate du dernier changement: %s\n",CHNG_DATE);
#endif

		exit(1);
	}

	/*
	*	Get Environment Variables
	*/

#ifndef	ORACLE
	home_dir = getenv("LBASE") ;
	ctools   = getenv("CTOOLS") ;
	if(home_dir == NULL)
#ifdef ENGLISH
		printf("\"LBASE\" environment variable is not exported\n");
	if(ctools == NULL)
		printf("\"CTOOLS\" environment variable is not exported\n");
	if(home_dir == NULL || ctools == NULL)
#else
		printf("\"LBASE\"Variable de l'environnement n'est pas exportee\n");
	if(ctools == NULL)
		printf("\"CTOOLS\"Variable de l'environnement n'est pas exportee\n");
	if(home_dir == NULL || ctools == NULL)
#endif
		exit(-1) ;

	/*
	*	Set the Path Varibales
	*/

#ifdef	MS_DOS
	/* If not a root directory */
	if( strcmp(home_dir, "\\") ) {
#else			/* UNIX compatible systems */
	/* If not a root directory */
	if( strcmp(home_dir, "/") ) {
#endif
		strcpy(DATA_PATH, home_dir) ;
		strcat(DATA_PATH, DATA_DIR) ;

		strcpy(NFM_PATH, home_dir) ;
		strcat(NFM_PATH, NFM_DIR) ;

		strcpy(EXE_PATH, home_dir) ;
		strcat(EXE_PATH, EXE_DIR) ;

		strcpy(FMT_PATH, home_dir) ;
		strcat(FMT_PATH, FMT_DIR) ;

		strcpy(WORK_DIR, home_dir) ;
		strcat(WORK_DIR, WORK) ;
	} else {
		strcpy(DATA_PATH, DATA_DIR) ;
		strcpy(NFM_PATH, NFM_DIR) ;
		strcpy(EXE_PATH, EXE_DIR) ;
		strcpy(FMT_PATH, FMT_DIR) ;

		strcpy(WORK_DIR, WORK) ;
	}
#else
	home_dir = getenv("LBASE1") ;
	home_dir2 = getenv("LBASE2") ;
	ctools   = getenv("CTOOLS") ;
#ifdef ENGLISH
	if(home_dir == NULL)
		printf("\"LBASE1\" environment variable is not exported\n");
	if(home_dir2 == NULL)
		printf("\"LBASE2\" environment variable is not exported\n");
	if(ctools == NULL)
		printf("\"CTOOLS\" environment variable is not exported\n");
	if(home_dir == NULL || home_dir2 == NULL || ctools == NULL)
		exit(-1) ;
#else
	if(home_dir == NULL)
		printf("\"LBASE1\"Variable de l'environnement n'est pas exporte\n");
	if(home_dir2 == NULL)
		printf("\"LBASE2\"Variable de l'environnement n'est pas exporte\n");
	if(ctools == NULL)
		printf("\"CTOOLS\"Variable de l'environnement n'est pas exporte\n");
	if(home_dir == NULL || home_dir2 == NULL || ctools == NULL)
		exit(-1) ;
#endif

	/*
	*	Set the Path Varibales
	*/

#ifdef	MS_DOS
	/* If not a root directory */
	if( strcmp(home_dir, "\\") ) {
#else			/* UNIX compatible systems */
	/* If not a root directory */
	if( strcmp(home_dir, "/") ) {
#endif
		strcpy(DATA_PATH, home_dir) ;
		strcat(DATA_PATH, DATA_DIR) ;

		strcpy(EXE_PATH, home_dir) ;
		strcat(EXE_PATH, EXE_DIR) ;

		strcpy(WORK_DIR, home_dir) ;
		strcat(WORK_DIR, WORK) ;
	} else {
		strcpy(DATA_PATH, DATA_DIR) ;
		strcpy(EXE_PATH, EXE_DIR) ;

		strcpy(WORK_DIR, WORK) ;
	}

#ifdef	MS_DOS
	/* If not a root directory */
	if( strcmp(home_dir2, "\\") ) {
#else			/* UNIX compatible systems */
	/* If not a root directory */
	if( strcmp(home_dir2, "/") ) {
#endif
		strcpy(NFM_PATH, home_dir2) ;
		strcat(NFM_PATH, NFM_DIR) ;

		strcpy(FMT_PATH, home_dir2) ;
		strcat(FMT_PATH, FMT_DIR) ;
	} else {
		strcpy(NFM_PATH, NFM_DIR) ;
		strcpy(FMT_PATH, FMT_DIR) ;
	}
#endif

	strcpy(CTOOLS_PATH, ctools) ;
	/* if ctools is not a root, append slashes to the path */
#ifdef	MS_DOS
	if(strcmp(ctools,"\\"))
		strcat(CTOOLS_PATH, "\\") ;
#else
	if(strcmp(ctools, "/"))
		strcat(CTOOLS_PATH, "/") ;
#endif

#ifdef	ORACLE
	s_ptr = getenv("OWNER_PREFIX") ;
	if(s_ptr == NULL)
	    printf("\"OWNER_PREFIX\" environment variable is not exported\n");
	if(s_ptr == NULL) exit(-1) ;

	strcpy(owner_prefix, s_ptr) ;

	s_ptr = getenv("TABLESPACE") ;
	if(s_ptr != NULL) strcpy(tablespace, s_ptr) ;

	strcpy( e_mesg, DATA_PATH );
	strcat(e_mesg, PASSWDPREFIX );
	get_tnum(tnum);
	strcat(e_mesg,tnum);
	if(  access(e_mesg,0)==0 && (fp=fopen(e_mesg,"r"))!=NULL ){
		fscanf( fp, "%s", UserPasswd ); 
		fclose( fp );
		unlink( e_mesg );/* delete oracle passwd file for security */
	}
	else{
#ifdef ENGLISH
		s_ptr = getpass("Password: ");
#else
		s_ptr = getpass("Mot de passe: ");
#endif
		if(s_ptr==NULL){
#ifdef ENGLISH
			printf("\nUnable to get password. Error: %d\n",errno);
#else
			printf("\nIncapable d'obtenir le mot de passe. Erreur: %d\n",errno);
#endif
			exit(-1);
		}
		else
			strcpy( UserPasswd, s_ptr );
	}
#endif

#ifdef SECURITY

	mainfileno = file_no;

	/* Check if the user has BROWSE access on the given file */
	/* Ignore access checking if the file number is invalid */
	if( file_no >= 0 && file_no < TOTAL_FILES ){
		i = CheckAccess( file_no, BROWSE, e_mesg );
		if(i < 0) {
			printf("%s\n",e_mesg);
			close_dbh() ;
			exit(1);
		}
	}
#endif

	return(NOERROR) ;
}
/*
*	Some times we like to set district number without passing arguments ..
*	This routine might be handy so has been put in the library ..
*/

set_dist()
{
	int	i;
	char	dist_number[10] ;

#ifdef ENGLISH
	printf("District Number: ");
#else
	printf("Numero du district: ");
#endif
	scanf("%s", dist_number) ;
	strncpy(dist_no, dist_number, 3) ;
	sscanf(dist_no, "%d", &i) ;
	if ( i == 0 )
		dist_no[0] = '\0' ;
	else if ( dist_no[1] == '\0' ) {
		dist_no[1] = dist_no[0] ;
		dist_no[0] = '0' ;
	}
}
#ifdef	ORACLE
#ifdef	MS_DOS
/*-------------------------------------------------------------*/
char *getpass(prmt)
char *prmt;
{
   register int i = 80;
   char *w;
   static char paswd[81];

   w = paswd;
   printf("%s" ,prmt);

   while(i-- > 0 && (*w = getch()) != EOF && *w > 30)w++;
   *w = '\0';

   return(paswd);
}
#endif
#endif
/*-----------------------------END OF FILE--------------------*/

