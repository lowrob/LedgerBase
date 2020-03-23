
/*
*	requ.h
*/

/* Requisition Statuses	*/
#ifdef ENGLISH
#define	APPROVED	'A'
#define	OPEN		'O'
#define	PROCESSED	'P'
#define	STOCK		'S'
#define	DISAPPROVED	'D'
#else
#define	APPROVED	'A'
#define	OPEN		'O'
#define	PROCESSED	'T'
#define	STOCK		'S'
#define	DISAPPROVED	'N'
#endif

#define	TAXABLE		'T'
#define	EXEMPT		'E'

/* used in Individual Processing & Merge/Multi Processing */
#define MERGE_DIRECT	0   /* Direct stock item */
#define MERGE_NONBULK	1   /* Non-Bulk Stock Item */
#define MAX_PO		2   /* maximum number of po's allowed to be created */
#define PO_HDR_EMPTY	0    
#define PO_HDR_OCCUPIED	1

/* Macros for appropriate global variable declarations.	*/
#ifdef	MAIN
#define	GLOBAL	
#else
#define	GLOBAL	extern
#endif

/* Global variables for global tax calculation routine.	*/
GLOBAL	double	Gst_Tax;
GLOBAL	double	Pst_Tax;

long	calcreqcommits();
long	calctax();
long	settax();
double	calc_commit();
/*-------------------- E n d   O f   F i l e ---------------------*/
