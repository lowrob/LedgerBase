/******************************************************************************
		Sourcename    : popurge.c
		System        : Budgetary Financial system.
		Module        : Purchase Order
		Created on    : 89-09-30
		Created  By   : Cathy Burns 
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

int     PobyPoNbr();

PoPurge()
{
	int retval ;

#ifdef ENGLISH
	printf("\nPurging of Completed Purchase Orders");
#else
	printf("\nEfface les bons de commande completes");
#endif

	retval = PobyPoNbr(2);
	if (retval < 0) {
#ifdef ENGLISH
		printf("\nError in purging completed purchase orders") ;
#else
		printf("\nErreur en effacant les bons de commande conpletes") ;
#endif
		return(retval);
	}
	else {
#ifdef ENGLISH
		printf("\nSuccessfully purged completed purchase orders");
#else
		printf("\nEffacement des bons de commande completes est reussie");
#endif
		return(0);
	}
}

