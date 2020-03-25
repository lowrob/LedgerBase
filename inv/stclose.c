/******************************************************************************
		Sourcename    : stclose.c
		System        : Budgetary Financial System.
		Subsystem     : Inventory System 
		Module        : Stock Closing 
		Created on    : 89-09-12
		Created  By   : K HARISH.
		Cobol sources : 

HISTORY:
 Date           Programmer     Description of modification
____/__/__      __________     ___________________________

******************************************************************************/
#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

static St_mast	stmast;	/* stock master record */
static int retval;	/* Global variable to store function values */
extern char e_mesg[80]; /* to store error messages */

StockClose(type)
int	type;
{
	/* Initialize the stock master key */
	stmast.st_fund = 0;
	stmast.st_code[0] = '\0';
	flg_reset(STMAST);
	for( ; ; ){

		/* Read the next stock master record in Update mode */
		retval = get_n_stmast( &stmast, UPDATE, 0, FORWARD, e_mesg );
		if( retval==ERROR || retval==LOCKED ){
			printf(e_mesg);
			printf("\nDberror: %d Iserror: %d Errno: %d",
				dberror, iserror, errno );
			return(-1);
		}
		if( retval==EFL )/* if end of file is reached */
			break;

		/* close for the month */
		stmast.st_m_opb = stmast.st_on_hand;
		stmast.st_m_iss = 0.0;
		stmast.st_m_rec = 0.0;
		stmast.st_m_adj = 0.0;

		if( type==YEAR ){	/* close for the year also */
			stmast.st_y_opb = stmast.st_on_hand;
			stmast.st_y_iss = 0.0;
			stmast.st_y_rec = 0.0;
			stmast.st_y_adj = 0.0;
		}

		/* write the updated record */
		retval = put_stmast( &stmast, UPDATE, e_mesg );
		if( retval<0 ){
			printf(e_mesg);
			printf("\nDberror: %d Iserror: %d Errno: %d",
				dberror, iserror, errno );
#ifdef ENGLISH
			printf("Aborting stock closing");
#else
			printf("Annule la fermeture de stock");
#endif
			return(-1);
		}

		if( commit(e_mesg)<0 ){	/* and commit it */
			roll_back(e_mesg);
#ifdef ENGLISH
			printf("Unable to commit from the record key: %d %s",
				stmast.st_fund, stmast.st_code );
#else
			printf("Incapable d'engager de la cle de fiche: %d %s",
				stmast.st_fund, stmast.st_code );
#endif
			return(-1);
		}
		/* increment the key to read the next record */
		inc_str( stmast.st_code, sizeof(stmast.st_code)-1,FORWARD);
	}
	/* All the record in stock master have been written successfully */
#ifdef ENGLISH
	if(type==MONTH)
		printf("\n\tStock Closing for the period successful.");
	else
		printf("\n\nStock Closing for the year successful.");
#else
	if(type==MONTH)
		printf("\n\tFermeture de stocks pour la periode reussie.");
	else
		printf("\n\nFermeture de stocks pour l'annee reussie.");
#endif

	close_file( STMAST );
	return(0);
}
