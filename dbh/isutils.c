/******************************************************************************
		Sourcename    : isutils.c
		System        : Budgetary Financial system.
		Created on    : 89-12-28
		Created  By   : K HARISH.
*******************************************************************************

HISTORY:
 Date           Programmer     Description of modification
____/__/__      __________     ___________________________

Calls for the user:	( Check for the value returned: -1 on error )

About the file:
	This file provides utilities for verification and manipulation of isam 
	files, calling some programs from the directory
		/usr/ctools/uisam/bin

	The routines provided below can be linked to the bfs library(dbh)
	and the above mentioned utilities can be called by the bfs processes
	through these routines.

******************************************************************************/
#include <stdio.h>
#include <bfs_defs.h>
#include <filein.h>
#include <dberror.h>

#define	CONV_FILE	"isconv"
#define	CHECK_FILE	"ischeck"
#define	RECREAT_FILE	"isrecrt"

#define PATH_FILE_SIZE	50

static int retval;	/* Global variable to store function values */
static char e_mesg[80]; /* to store error messages */

static	char	path[PATH_FILE_SIZE];

extern	int	DT_TYPE;

ixdisplay( file_no )	/* display the indices of the given file */
int	file_no;		/* number of the isam file */
{
	/* ensure that the given file is indexed file */
	if( getfiletype( file_no )!= ISAM )
		dbexit( NONISMMOD, ERROR )	/* ; not needed */

	/* extract file basename from file# */
	retval = getflnm( file_no, e_mesg );
	if( retval<0 )
		return(retval);

	form_f_name( e_mesg, path );	/* form complete datapath */
	sprintf( e_mesg, "%s%s%s %s",CTOOLS_PATH,CONV_FILE,EXTN,path );

	return( system(e_mesg) );	/* execute the command line string */
}
ixcheck( file_no )	/* Check the index of the given isam file */
int	file_no;		/* number of the isam file */
{
	/* ensure that the given file is indexed file */
	if( getfiletype( file_no )!= ISAM )
		dbexit( NONISMMOD, ERROR )	/* ; not needed */

	/* extract file basename from file# */
	retval = getflnm( file_no, e_mesg );
	if( retval<0 )
		return(retval);

	form_f_name( e_mesg, path );	/* form complete datapath */
	sprintf( e_mesg, "%s%s%s %s",CTOOLS_PATH,CHECK_FILE,EXTN,path );

	return( system(e_mesg) );	/* execute the command line string */
}
ixrecreat( file_no )	/* Recreat ( compress,if possible ) given isam file */
int	file_no;		/* number of the isam file */
{
	/* ensure that the given file is indexed file */
	if( getfiletype( file_no )!= ISAM )
		dbexit( NONISMMOD, ERROR )	/* ; not needed */

	/* extract file basename from file# */
	retval = getflnm( file_no, e_mesg );
	if( retval<0 )
		return(retval);

	form_f_name( e_mesg, path );	/* form complete datapath */
	sprintf( e_mesg, "%s%s%s -d%d %s",
			CTOOLS_PATH,RECREAT_FILE,EXTN,DT_TYPE,path );

	return( system(e_mesg) );	/* execute the command line string */
}
