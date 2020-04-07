/*
	File Name : struct.h
	Written By : P. Jayadeep
	Modified By : V. Subramani
*/


typedef	struct 	{
	char	a ;
	char	b ;
} c_algn ;	

typedef struct 	{
	char	a ;
	int	b ;
} i_algn ;

typedef struct 	{
	char	a ;
	short	b ;
} s_algn ;

typedef struct 	{
	char	a ;
	long	b ;
} l_algn ;

typedef struct 	{
	char	a ;
	float	b ;
} f_algn ;

typedef struct 	{
	char	a ;
	double	b ;
} d_algn ;

typedef struct 	{
	char	a ;
	d_algn	b ;
} st_algn ;

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
#define ALGN_STRUCT	( (char*)&(((st_algn*)NULL)->b) - \
			(char*)&(((st_algn*)NULL)->a) )

#define	SET_OFFSET(OFFSET,TYP) OFFSET += \
				(int)( (OFFSET%TYP) ? (TYP - OFFSET%TYP) : 0 )

#define MAXFLD 100

#define LONG 'l'
#define INT 'i'
#define SHORT 's'
#define FLOAT 'f'
#define DOUBLE 'd'
#define CHAR 'c'

#define L_NAME	11	/* field and structure can have max. of 10 chars. */

typedef struct fldinfo {
	char fldname[L_NAME] ;	/* field name , given in comment string */
	char typf ;		/* field type, INT,SHORT,LONG.. */
	short posf ;		/* field offset from the begining of struct */
	short lenf ;		/* field length	*/
} FLDINFO ;


 typedef struct {
	char stname[L_NAME] ;	/* Record name. Supplied in comment string */ 
	short stsiz ;		/* size of the structure	*/
	short stflds ;		/* #of fields available in the structure */
} INHDR ;



