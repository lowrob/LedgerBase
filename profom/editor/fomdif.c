/* dispfor.c - display form module for Editor */

#include "cfomdef.h"
#include "fomlink.h"
#include "cfomstrc.h"
#include "cfomfrm.h"
#include <stdio.h>

extern	struct	linkstr	*first;

extern	struct	stat_rec	*sp;

extern	int	formup;

dispform(){	/* release current screen and display screen in *first */
	register struct linkstr *lkptr,*lkptr2;

	chkstat
	if (formup)
		if (clrall())
			return(1);
		else
			;
	else
		if (clrscrn())
			return(1);
	lkptr = lkptr2 = first;
	do	{
		if (lkptr->fldcla == CL_PROM  || lkptr->fldcla == CL_PRMFLD)
			if (display(lkptr->prox,lkptr->proy,lkptr->promv,
					lkptr->promp))
				return(1);
		if (lkptr->fldcla == CL_FLD || lkptr->fldcla == CL_PRMFLD)
			if (display(lkptr->flx,lkptr->fly,lkptr->fldv,
					lkptr->dmas))
				return(1);
		lkptr = lkptr->nexp;
		} while (lkptr != lkptr2);
	if (ncmerr("Press Any Key To Continue "))
		return(1);
	get();
	if (clrscrn())
		return(1);
	retnoerr
	}
