/*
*	Source 	: suppyend.c 
*
*	Program to Reset ytd values in supplier before starting new year. 
*
*/
#include <stdio.h>

#include <bfs_defs.h>
#include <bfs_recs.h>

#define EXIT	12

static	Pa_rec	pa_rec ;
static  Supplier supp_rec;

static	char	e_mesg[100] ;


SupplierYearEnd( )
{
	int	retval;
	
#ifdef ENGLISH
	printf("\n\n Starting Supplier Close");
#else
	printf("\n\n Commence la fermeture du fournisseur");
#endif
	/* Initialize Rec. Entry Header Record to start reading form the
	   beginging */
	supp_rec.s_supp_cd[0] = '\0' ;
	flg_reset( SUPPLIER );

	for( ; ; ) {
		retval = get_n_supplier(&supp_rec,UPDATE,0,FORWARD,e_mesg);
		if( retval < 0) {
			if(retval == EFL) break ;
			printf("\n %s",e_mesg);
			break ;
		}

		/*  Initialize YTD values */

		supp_rec.s_ytd_ord = 0.00 ;
		supp_rec.s_ytd_ret = 0.00 ;
		supp_rec.s_ytd_recpt = 0.00 ;

		/* Do not initialize HB amount */
		if(supp_rec.s_type[0] != CONTRACT)
			supp_rec.s_ytd_disc = 0.0 ;

		retval = put_supplier(&supp_rec,UPDATE,e_mesg);
		if (retval < 0) {
			printf("\n %s",e_mesg);
			roll_back(e_mesg) ;
			break;
		}	
		if (commit(e_mesg) < 0) {
			printf("\n %s",e_mesg);
			break;
		}
		inc_str(supp_rec.s_supp_cd, 
			sizeof(supp_rec.s_supp_cd) -1, FORWARD);
	}		
	if( retval < 0) {
		if(retval == EFL) 
#ifdef ENGLISH
			printf("\n\n Supplier Close Successfully completed");
#else
			printf("\n\n Fermeture du fournisseur reussie");
#endif
		else
#ifdef ENGLISH
			printf("\n\n Error in Supplier Close") ;
#else
			printf("\n\n Erreur dans la fermeture du fournisseur") ;
#endif
	}

	close_file(SUPPLIER) ;
	return(0);
}
