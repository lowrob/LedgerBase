/*-----------------------------------------------------------------------
Source Name: fld_defs.h
System     : General Utility.
Created  On: 22nd Jun 89.

DESCRIPTION:
	Contains definitions for the sources which will be using
	'.def' files.

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

------------------------------------------------------------------------*/

#define	MAX_DIM		5

#define	T_CHAR		'c'
#define	T_SHORT		's'
#define	T_INT		'i'
#define	T_LONG		'l'
#define	T_FLOAT		'f'
#define	T_DOUBLE	'd'

typedef	struct	{
	char	filenm[11] ;	/* Def file name */
	short	no_fields ;	/* NO of fields */
	short	reclen ;	/* Record Length */
} Fld_hdr ;

typedef	struct	{
	char	name[16] ;	/* Name of the Field */
	char	format[10] ;	/* Print format to be used in Rec display prog*/
	char	type ;		/* Type of field */
	short	offset ;	/* Offset of the Field */
	short	len ;		/* Length of the Field for Character Fields */
} Field ;

typedef	struct 	c1 {
	char	a ;
	char	b ;
} c_algn ;	

typedef struct 	i1 {
	char	a ;
	int	b ;
} i_algn ;

typedef struct 	s1 {
	char	a ;
	short	b ;
} s_algn ;

typedef struct 	l1 {
	char	a ;
	long	b ;
} l_algn ;

typedef struct 	f1 {
	char	a ;
	double	b ;
} f_algn ;

typedef struct 	d1 {
	char	a ;
	double	b ;
} d_algn ;

typedef struct 	st1 {
	char	x ;
	d_algn	y ;
} st_algn ;

/** Alignment for NCR 3000 ***/
#define ALGN_CHAR	( (char*)&(((c_algn*)NULL)->b) - \
			(char*)&(((c_algn*)NULL)->a) )
#define ALGN_SHORT	( (char*)&(((s_algn*)NULL)->b) - \
			(char*)&(((s_algn*)NULL)->a) )
#define ALGN_INT	( (char*)&(((i_algn*)NULL)->b) - \
			(char*)&(((i_algn*)NULL)->a) )
#define ALGN_LONG	( (char*)&(((l_algn*)NULL)->b) - \
			(char*)&(((l_algn*)NULL)->a) )
#define ALGN_FLOAT	( (char*)&(((f_algn*)NULL)->b) - \
			(char*)&(((f_algn*)NULL)->a) )
#define ALGN_DOUBLE	( (char*)&(((d_algn*)NULL)->b) - \
			(char*)&(((d_algn*)NULL)->a) )
#define ALGN_STRUCT	( (char*)&(((st_algn*)NULL)->y) - \
			(char*)&(((st_algn*)NULL)->x) )

/** Alignment for TOWER 650 
#define ALGN_CHAR	1
#define ALGN_SHORT	2
#define ALGN_INT	2
#define ALGN_LONG	2
#define ALGN_FLOAT	2
#define ALGN_DOUBLE	2
#define ALGN_STRUCT	2
****/

#define	SET_OFFSET(OFFSET,TYP) OFFSET += \
				(int)( (OFFSET%TYP) ? (TYP - OFFSET%TYP) : 0 )


/*--------------------E-N-D---O-F---F-I-L-E----------------------------*/

