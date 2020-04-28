/*---------------------------------------------------------------------
	Source Name: err_chk.c
	Created By : T Amarendra
	Created On : 13-SEP-88.

	Function checks for PROFOM error.
-----------------------------------------------------------------------*/
#include <bfs_defs.h>
#include <cfomstrc.h>

err_chk(sr)
struct stat_rec *sr;
{
	int err,endfld,curfld;

	if(sr->retcode == RET_ERROR)
	{
		err = sr->errno_lou;
		endfld = sr->endfld;
		curfld = sr->curfld;
		if(err == 88)           /* free error */
			return(NOERROR);
		fomcs();
#ifdef ENGLISH
		printf("PROFOM Error\n\tScreen Name: %s\n",sr->scrnam) ;
#else
		printf("Erreur PROFOM\n\tNom d'ecran: %s\n",sr->scrnam) ;
#endif
		if (err == 40 )
#ifdef ENGLISH
			printf("\tInternal Error: %d Error: %d\n", err,
				endfld);
#else
			printf("\tErreur interne: %d Erreur: %d\n", err,
				endfld);
#endif
		else
#ifdef ENGLISH
			printf("\tError: %d at Field: %d\n",err, curfld);
		get();
#else
			printf("\tErreur: %d au Champ: %d\n",err, curfld);
		get();
#endif
		return(PROFOM_ERR);
	}
	return(NOERROR);
}

