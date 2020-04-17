/* fomdef.h - contains some useful general definitions */

#define TRUE	1
#define FALSE	0
#define LOW_VAL        (unsigned) '\000' /* COBOL LOW_VALUE */
#define HIGH_VAL       (unsigned) '\377' /* COBOL HIGH_VALUE */
#define COB_ZEROE	(unsigned) '0'	/* COBOL ZERO */
#define COB_SPACE	(unsigned) ' '	/* COBOL SPACE */
#define CXL		132	/* COBOL X arguement length */
#define COB_LINES	24	/* no of lines on screen as in FOMSS call */
#define COB_COLUMNS	132	/* no of cols on screen as in FOMSS call */
#define DEF_WIDTH	80	/* we assume this is the width */
#define EOS_CHAR	(unsigned) '\0'	/* end of string char */
#define CMASK		0177	/* mask to ensure ASCII chars */
#define FORM_FEED	(unsigned) '\f'
#define LINE_FEED	(unsigned) '\n'
#define PROMPT		1
#define MASK		2
#define DATA		3

#define reterr(X) {sp->retcode = RET_ERROR; sp->errno = X; return(1);}
#define retnoerr {sp->retcode = RET_NO_ERROR; sp->errno = 0; return(0);}
#define elm(i,j)	((fap+i)->j)	/* element j of ith field */
#define velm(i,j)	(fvp+(fap+i)->j)/* address of string corr to elm(i,j)*/
#define cvelm(j)	(fvp+cf->j)	/* addr of string corr to cf(j) */
#define ielm(i,j)	((fiap+i)->j)	/* internal element j of ith fld */
#define max(X,Y)	((X)>(Y) ? (X) : (Y))
#define fomintlerr(X) {sp->retcode = RET_ERROR; sp->errno = 40; sp->endfld = X; return(1);}
#define chkstat	if (sp == NULL) return(1);


/**********
Old Machine Definitions Replaced on 25-3-89 by New definitions.. kavi

#define SMAXDIGITS	4	max no of places in a short int 
#define IMAXDIGITS	4	max no of places in a regular int 
#define FMAXDIGITS	9	max no of digits in a float 


#define ALGN_CHAR	1	 alignment required for char 
#define ALGN_SHORT	2	 alignment required for short int 
#define ALGN_INT	2	 alignment required for regular int
#define ALGN_LONG	2	 alignment required for long int 
#define ALGN_FLOAT	2	 alignment required for float 
#define ALGN_DOUBLE	2	 alignment required for double 

********/

/** New Machine Independent Definitions of Machine Dependent Constsnts **/

#define SMAXDIGITS	(sizeof(short)*8)/3 -1 
#define IMAXDIGITS	(sizeof(int)*8)/3 -1
/* SBO 901217: Start
*** Changed definition so that floats will never be used. 
#define FMAXDIGITS	(sizeof(float)*8)/3 -1  */
/* SBO 901217: End */
#define FMAXDIGITS	0

typedef	struct 	{	char	a ;	char	b ;	} c_algn ;	
typedef struct 	{	char	a ;	int	b ;	} i_algn ;
typedef struct 	{	char	a ;	short	b ;	} s_algn ;
typedef struct 	{	char	a ;	long	b ;	} l_algn ;
typedef struct 	{	char	a ;	float	b ;	} f_algn ;
typedef struct 	{	char	a ;	double	b ;	} d_algn ;

#define ALGN_CHAR	((char*)&(((c_algn*)NULL)->b) - \
				(char*)&(((c_algn*)NULL)->a))
#define ALGN_SHORT	((char*)&(((s_algn*)NULL)->b) - \
				(char*)&(((s_algn*)NULL)->a))
#define ALGN_INT	((char*)&(((i_algn*)NULL)->b) - \
				(char*)&(((i_algn*)NULL)->a))
#define ALGN_LONG	((char*)&(((l_algn*)NULL)->b) - \
				(char*)&(((l_algn*)NULL)->a))
#define ALGN_FLOAT	((char*)&(((f_algn*)NULL)->b) - \
				(char*)&(((f_algn*)NULL)->a))
#define ALGN_DOUBLE	((char*)&(((d_algn*)NULL)->b) - \
				(char*)&(((d_algn*)NULL)->a))



