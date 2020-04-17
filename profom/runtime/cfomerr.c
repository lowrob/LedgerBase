/* cfomerr.c - CPROFOM error message file handler */

#include <stdio.h>
#include "cfomerr.h"
#include "cfomdef.h"
#include "cfomstrc.h"

extern struct stat_rec *sp;	/* defined in cfomfrm1.c */

static int	fd,	/* for reading error message file */
		einit = FALSE;	/* error init done? */

static struct erflhdr eh;	/* error message file header */

static char *mess;	/* error message strings */

char *malloc();

errinit(erflnm)		/* init for file *erflnm */
char *erflnm;{
	if (einit)
		fomintlerr(14)	/* init already done */
	if ((fd = open(erflnm,0)) == -1)
		reterr(41)
	if (read(fd,(char *)&eh,EHSZ) != EHSZ)
		reterr(42)	/* read error */
	if ((mess = malloc((unsigned)eh.messlen)) == NULL)
		reterr(6)	/* no space */
	if (read(fd,mess,eh.messlen) != eh.messlen)
		reterr(42)
	einit = TRUE;
	close(fd);
	return(0);
	}

char *err(errno)	/* return ptr to error message corr to errno */
int errno;{
	if (!einit)
		return(NULL);
	if (errno >= 1 && errno <= MAXERRORS)
		return(mess+eh.eroff[errno - 1]);
	else
		return(mess);
	}
