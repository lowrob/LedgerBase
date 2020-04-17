/* cfomstrc.h : header for Cprofom status record */

/* value returned by profom calls in retcode field of status record */

#define RET_ERROR  	-1 	/* return code for error */
#define RET_NO_ERROR	0 	/* return code for no error */
#define RET_VAL_CHK 	1 	/* return code for validity check */
#define RET_USER_ESC	2 	/* return code for user escape */
#define RET_HELP	6	/* return code for help */
#define RET_PVAL_CHK	7	/* added by Dale */

/* values returned profom read calls in termcode field of status record */

#define FT_CR   	0 	/* field terminated by carriage return */
#define FT_TAB		1 	/* field terminated by tab */
#define FT_FULL 	2 	/* field terminated by all full */

/* values returned profom read calls in fillcode field of status record */

#define FIL_PARTLY	0 	/* field is partly filled */
#define FIL_FULLY	1 	/* field is fully  filled */
#define FIL_OMITTED	2 	/* field is omitted */
#define FIL_DUP		3 	/* field is filled by duplication */

/* sizes of char arrays in status record */

#define	SR_TNML		9 	/* terminal name size */
#define SR_SCRNML	81	/* screen name size */
#define SR_DFNML	31	/* data field name length */
#define SR_ESNML	81	/* error set name length */
#define SR_ESCL		5	/* length of escape char array */

/* LOW and HIGH values for different types of fields */

#define LV_CHAR		'\000'		/* LOW-VALUE for char field */
#define HV_CHAR		'\377'		/* HIGH-VALUE for char field */

#define LV_SHORT	(-32767)	/* LOW-VALUE for short int field */
#define HV_SHORT	(32767)		/* HIGH-VALUE for short int field */
#define LV_LONG		(-2147483647L)	/* LOW-VALUE for long int field */
#define HV_LONG		(2147483647L)	/* HIGH-VALUE for long int field */

#define LV_INT		((sizeof(int) == sizeof(long)) ? LV_LONG : LV_SHORT)
#define HV_INT		((sizeof(int) == sizeof(long)) ? HV_LONG : HV_SHORT)

/*
#define LV_FLOAT	(-1.0E10)	* LOW-VALUE for float field *
#define HV_FLOAT	(1.0E10)	* HIGH-VALUE for float field *
*/
#define LV_FLOAT	(-1.0E30)	/* changed float to double */
#define HV_FLOAT	(1.0E30)

#define LV_DOUBLE	(-1.0E30)
#define HV_DOUBLE	(1.0E30)

/* values for BOOLEAN (YES/NO) fields */

#define BOOL_YES	1
#define BOOL_NO		0

/* profom status record structure */

struct stat_rec {
char	termnm[SR_TNML],	/* terminal name */
	scrnam[SR_SCRNML];	/* screen name */
int	retcode,		/* return code */
	retcode1,		
	errno,			/* error number */
	strtfld,		/* start field number */
	nextfld,		/* next field number */
	endfld,			/* last field number */
	curfld,			/* current field number */
	termcode,		/* field termination code */
	fillcode,		/* field fill code */
	fillcount;		/* field fill count */
char	escchar[SR_ESCL],	/* escape character */
	errset[SR_ESNML],	/* alternate error message file */
	nextnam[SR_DFNML],	/* next field name */
	endnam[SR_DFNML];	/* last field name */
};


