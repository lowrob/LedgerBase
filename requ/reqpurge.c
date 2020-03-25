/******************************************************************************
		Sourcename    : reqpurge.c
		System        : Budgetary Financial system.
		Module        : Requisitioning System 
		Created on    : 91-04-29
		Created  By   : J. Prescott
		Cobol sources : 
*******************************************************************************
About this program:

HISTORY:
 Date           Programmer     Description of modification
____/__/__      __________     ___________________________
*******************************************************************************/
#include <stdio.h>

#include <bfs_defs.h>
#include <bfs_recs.h>

static int retval;	/* Global variable to store function values */
static char e_mesg[80]; /* to store error messages */

int     reqbyno();

ReqPurge()
{
	int retval ;

#ifdef ENGLISH
	printf("\nPurging of Process/Disapproved Requisitions");
#else
	printf("\nEfface les requisitions traitess/non-approuvees");
#endif

	retval = reqbyno(1);
	if (retval < 0) {
#ifdef ENGLISH
		printf("\nError in purging Processed/Disapproved Requisitions");
#else
		printf("\nErreur en effacant les req traitees/non-approuvees") ;
#endif
		return(retval);
	}
	else {
#ifdef ENGLISH
		printf("\nSuccessfully purged Processed/Disapproved Requisitions");
#else
		printf("\nEffacement des req traitees/non-approuvees est reussi");
#endif
		return(0);
	}
}

