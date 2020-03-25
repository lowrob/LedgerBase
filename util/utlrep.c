/******************************************************************************
		Sourcename    : utlrep.c
		System        : Budgetary Financial system.
		Module        : Utilities
		Created on    : 89-07-31
		Created  By   : K HARISH.
		Cobol sources : 
*******************************************************************************
About this program:
	This program prints the following reports, calling some utilities from
	$G/reputils.c. which uses a profom based screen and provides a  user
	interface for the below report routines for accepting key information.

	auditrep();

HISTORY:
 Date           Programmer     Description of modification
____/__/__      __________     ___________________________
*******************************************************************************/
#include <stdio.h>

#define MAIN
#define MAINFL 		-1	/* no main file used */

#include <bfs_defs.h>
#include <bfs_recs.h>

#define SYSTEM		"UTILITIES"
#define MOD_DATE	"23-JAN-90"
#define	EXIT	12	/* also defined in reputils.c */

#ifdef ENGLISH
#define PRINTER		'P'
#define FILE_IO		'F'
#define DISPLAY		'D'

#define BYFILE		'F'
#define BYUSER		'U'
#define BYTIME		'T'
#else
#define PRINTER		'I'
#define FILE_IO		'D'
#define DISPLAY		'A'

#define BYFILE		'D'
#define BYUSER		'U'
#define BYTIME		'H'
#endif

static int retval;	/* Global variable to store function values */
char e_mesg[80]; /* to store error messages */
int	auditrep();
int	CCrpt();
/* 	add menu items  and process menu options when user selects them */
/*	
	Functions  AddMenuItem(), Process(), DisplayMessage(), GetResponse()
	GetOutputon(), GetFilename(), GetPrinter() and Confirm() are found 
	in the file  reputils.c
	Function AudRep() is found  in the file audrep.c 
*/
main( argc, argv )
int argc;
char *argv[];
{
int	ret_option ;

	strncpy( SYS_NAME, SYSTEM, 50 ); 	/* Module name */
	strncpy( CHNG_DATE, MOD_DATE, 10 );	/* Last date of change */
	proc_switch( argc,argv,MAINFL ); 	/* process the switches */
	if( argc<3 )
		exit(0);

	/* Add menu items to the list */
#ifdef ENGLISH
	if( AddMenuItem( "AUDIT REPORT", auditrep,AUDIT )<0 )
		exit(-1);
	if( AddMenuItem( "COST CENTER REPORT", CCrpt,SCHOOL )<0 )
		exit(-1);
/* 	read and process the user's options 	*/
	ret_option = Process(terminal, "GENERAL REPORTS");
#else
	if( AddMenuItem( "RAPPORT DE VERIFICATION", auditrep,AUDIT )<0 )
		exit(-1);
	if( AddMenuItem( "RAPPORT DE CENTRE DE COUTS", CCrpt,SCHOOL )<0 )
		exit(-1);
/* 	read and process the user's options 	*/
	ret_option = Process(terminal, "  RAPPORTS GENERAUX");
#endif
	
	if(ret_option == NOACCESS) {
		fomen(e_mesg);
		get();
	}
	close_dbh();

	exit( ret_option );
}

/* Ask the user on what key the audit file is to be sorted & printed */
auditrep()
{
	int	retval;
	char	key[2];
	char 	resp[2];
	char	filename[15];

#ifdef ENGLISH
	if( DisplayMessage("Output sorted on: F(ile), U(ser), T(ime)")<0 )
		return(-1);
#else
    if( DisplayMessage("Sortie en ordre de: D(ossier), U(sager), H(eure)")<0 )
		return(-1);
#endif

	for( ; ; ){
		if( GetResponse(key)<0 )
			return(-1);
		if( *key!=BYFILE && *key!=BYUSER && *key!=BYTIME )
			continue;
		else
			break;
	}

	/* Show printer for output as default, and accept output medium */
#ifdef ENGLISH
	if( DisplayMessage("Output medium: P(rinter), D(isplay), F(ile)")<0 )
		return(-1);
#else
 if( DisplayMessage("Support de sortie: I(mprimante), A(fficher), D(ossier)")<0 )
		return(-1);
#endif

#ifdef ENGLISH
	strcpy( resp, "P" );
#else
	strcpy( resp, "I" );
#endif
	for( ; ; ){
		if( GetOutputon(resp)<0 )
			return(-1);
		if( *resp!=PRINTER && *resp!=DISPLAY && *resp!=FILE_IO )
			continue;
		else
			break;
	}

	if( *resp == FILE_IO ){
		if( DisplayMessage("Entrer le nom de la filiere de sortie")<0 )
			return(-1);

		strcpy( filename, "auditrep.dat" );
		if( GetFilename( filename )<0 )
			return(-1);
	}
	else
		*filename = '\0';

	if( (retval = Confirm())<0 )
		return(-1);
	if( retval==0 )	/* not confirmed */
		return(0);

	if( AudRep( *key, resp, filename )<0 )
		exit(-1);

	return( 0 );
}
