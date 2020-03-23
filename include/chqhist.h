/*       								*/
/*       								*/
/*       chqhist.h							*/
/*       								*/
/*       								*/
#include <stdio.h>
#include <reports.h>
#include <cfomstrc.h>
#define	HIGH		1
#define	LOW		-1
#define	CASHCHQFMT	3
#ifdef	ENGLISH
#define	ADDREC	'A'
#define	NEXT	'N'
#define	PREV	'P'
#define INQUIRE	'I'
#define EXITOPT	'E'
#define EDIT	'E'
#define CANCEL	'C'
#define	YES	'Y'
#define	NO	'N'
#else
#define	ADDREC	'R'
#define	NEXT	'S'
#define	PREV	'P'
#define INQUIRE	'I'
#define EXITOPT	'F'
#define EDIT	'M'
#define CANCEL	'A'
#define	YES	'O'
#define	NO	'N'
#endif

char	e_mesg[80];
Chq_hist	cheque, pre_rec,
		chqhist, oldchqhist;

char	*CurrentScr;
char	chardate[11];
char	projname[30];
char	*arayptr[5];

Pa_rec	pa_rec;
Ctl_rec	ctl_rec;
