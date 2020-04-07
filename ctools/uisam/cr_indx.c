#ifdef	O_ISAM
#include "iserror.h"
#define	currkeyno	currkey
#endif
/************************************************************************/
/*									*/
/* ISBUILD() :								*/
/*									*/
/*	This routine creates INDEXED files for the file name		*/
/*	The data file should exist while index should not !!!		*/
/*									*/
/************************************************************************/

isbuild(flnam, iomode, keysarray, ix_name, reclen)
char	*flnam;		/* Existing data file name */
int	iomode,		/* R,W, RWR */
	*keysarray;	/* Key descriptor array */
char	*ix_name;	/* Index file name to be created */
int	reclen ;	/* Record length of Data file */
{
	int	fd ;
#ifdef	MS_DOS
	char	buff[200] ;
#endif

	if(access(flnam,0) <  0) 
		reterr(FLACCSERR)

	if ((fd=iscreat(ix_name,iomode, 1,keysarray, reclen))< 0)return(ERROR);

	isclose(fd) ;

	unlink(ix_name) ;		/* Remove Data file for linking */

#ifdef	MS_DOS
	sprintf(buff, "copy %s %s", flnam, ix_name) ;
	system(buff) ;
#else
	if ( link(flnam, ix_name) < 0 ) reterr(DFLCRTERR) 
#endif

	return( isopen(ix_name, iomode) );
	
}


/****************************************************************************/
/*** 
*	New functions ....
****/

#ifdef	FIXED_RECLEN
ixbuild(fp, buffer, saveposn)
#else
ixbuild(fp, buffer, saveposn, length)
int	length ;
#endif

int	fp;
char	*buffer; /* record te written */
long	saveposn ;
{
int	retcode ;

	slotnum = fp ;
	currkeyno = 0 ;

	/***
 	if ( buildtables(0) == ERROR ) return(ERROR) ;
	***/

#ifdef	FIXED_RECLEN
	currslot.nxtposn = saveposn + currslot.reclength + STATUS_LEN ;
#else
	currslot.nxtposn = saveposn + length + STATUS_LEN ;
#endif

	currindex.pnumkeys++ ;
		
	/*
	if(writeheader() == ERROR) return (ERROR) ;
	*/

	assemblekey(buffer, keybuffer) ;

	if((retcode = writesearch((keybuffer))) == ERROR) 
		return (retcode) ;	
	
	/*
	if ( retcode == DUPE ) return(retcode) ;
	*/

#ifdef	FIXED_RECLEN
	if((retcode = insert(currslot.reclength+STATUS_LEN,saveposn)) 
					 == ERROR) return (retcode) ;
	return(currslot.reclength) ;
#else
	if((retcode = insert(length+STATUS_LEN,saveposn))
					 == ERROR) return (retcode) ;
	return(length) ;
#endif
}
/*------------------------END OF FILE----------------------------------*/

