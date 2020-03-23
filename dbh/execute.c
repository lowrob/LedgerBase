/*
*	execute.c
*
*	Runs given program thru' system() call.
*	This routine is useful in  calling a child process from 
*	a process running currently.
*
*	For ORACLE version communicate user's password thru temporary file.
*
*/

#include <stdio.h>
#include <bfs_defs.h>

/*--------------------------------------------------------------*/
execute(program, argc, argv)
char	*program ;		/* program name without extension */
int	argc ;			/* no. of arguments including program name */
char	**argv ;		/* pointer to actual arguments of program */
{
	char	command[256] ;
	int	i ;
#ifdef	ORACLE
	char	opfilename[50];
	FILE	*fopen(), *fp ;
	char	tnum[5];

	/* Create oracle user's password temporary file with path */
	strcpy( opfilename, DATA_PATH );
	strcat(opfilename, PASSWDPREFIX );
	get_tnum(tnum);
	strcat(opfilename,tnum);

	/* Store oracle user's passwd in a temp file */
	fp = fopen( opfilename, "w" );
	if( fp!=NULL ){
		fprintf( fp, "%s", UserPasswd );
		fclose( fp );
	}
#endif

	sprintf(command,"%s%s%s", EXE_PATH, program, EXTN );

	/* argv[0] is suppressed here as it is name of calling program */
	/* remaining arguments should be same as those for calling program */
	/* calling program should make sure that it is passing same arguments */

	for( i = 1 ; i < argc ; i++) {
		strcat(command, " ") ;
		strcat(command, argv[i] ) ;
	}

	system(command) ;

#ifdef	ORACLE
	/* To be safe, (try to) delete oracle user's password temp file */
	unlink( opfilename );
#endif

	return(0) ;
}
/*----------------------------- END OF FILE ---------------------------*/
