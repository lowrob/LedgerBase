/*----------------------------------------------------------------
	Source Name : repdef.h
	Author      : T Amarendra
	Created Date: 25-Apr-87.

	Coomon variables between.c repgen.c & report print modules
----------------------------------------------------------------*/

#ifdef	REP

#ifndef	TXT_RDMODE

#ifdef   MS_DOS

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define	TXT_RDMODE	(O_TEXT | O_RDONLY)
#define TXT_WRMODE	(O_TEXT | O_WRONLY)
#define	TXT_RWMODE	(O_TEXT | O_RDWR) 
#define	TXT_CRMODE	(O_TEXT | S_IREAD | S_IWRITE) 

#else 	/* else MS_DOS */

#define TXT_RDMODE      0
#define TXT_WRMODE      1
#define TXT_RWMODE      2
#define TXT_CRMODE      0666

#endif

#endif   /* ifndef TXT_RDMODE */

int	prfd ;		/* Report file fd */

char	line[133];	/* Report line */
short	PGSIZE,		/* Report Page size */
	cur_pos,	/* current pos in line[] to start copy */
	linecnt;	/* report line no. in Current Page */
int	term;		/* if Display term=get_term() else if print term=99 */
short	LNSZ = 80;	/* Default LNSZ */

#else	/* else REP */

extern	int	prfd ;	/* Report file fd */

extern	char line[];	/* Report line */
extern	short PGSIZE,	/* Report Page size */
	cur_pos,	/* current pos in line[] to start copy */
	linecnt;	/* report line no. in Current Page */
extern	int term;	/* if Display term=get_term() else if print term=99 */
extern	short LNSZ;

#endif

/** Defines used in tedit.c routines **/

#define	R_SHORT		1 
#define R_INT		2
#define	R_LONG		3
/*
#define	R_FLOAT		4 
*/
#define	R_FLOAT		5  /* change float to double */
#define	R_DOUBLE	5	

#ifndef	MS_DOS		/* Not MS_DOS */

#define	SPOOLER

#endif

/*----------------------------END OF FILE----------------------------*/

