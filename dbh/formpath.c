/*-----------------------------------------------------------------------
Source Name: formpath.c
System     : Budgetary Financial system.
Module     : General Ledger system.
Created  On: 11th May 89.

DESCRIPTION:
	Function to form the file name with complete PATH.

Format:
	int	form_f_name(in_file, out_file)
	char	*in_file ;	* Input file Name *
	char	*out_file ;	* New file name will be returned in this *


MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

------------------------------------------------------------------------*/

#include <bfs_defs.h>

form_f_name(in_file, out_file)
char	*in_file ;	/* Input File Name */
char	*out_file ;	/* Physical File name will be returned in this */
{
	strcpy(out_file, DATA_PATH) ;

	if(dist_no[0] != '\0') {
		strcat(out_file, dist_no) ;	/* dist is '00' - '99' */
#ifndef	MS_DOS
		strcat(out_file, "/" ) ;
#else
		strcat(out_file, "\\" ) ;
#endif
		}

#ifndef	ORACLE
	if ( SW8 ) 
		strcat(out_file, BACK_UP) ;
	else if ( SW9 ) 
		strcat(out_file, PREV_YEAR) ;

	if ( SW8 || SW9 ) 
#ifndef	MS_DOS
		strcat(out_file, "/" ) ;
#else
		strcat(out_file, "\\" ) ;
#endif
#endif
	
	strcat(out_file, in_file) ;
}

/*--------------------------END OF FILE----------------------------------*/

