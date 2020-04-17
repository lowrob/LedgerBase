/* fomfrm.h : header for form file data strucures */

#define FMH_SZ		sizeof(struct frmhdr)	/* form file header size */
#define FMF_SZ		sizeof(struct frmfld)	/* form file record size */
#define FI_SZ		sizeof(struct fldinfo)	/* internal info rec size */

struct  frmhdr {		/* form file header record format */

char	version[6],		/* version number string */
	language[8],		/* language name - currently COBOL */
	scrnnam[12];		/* internal name of the screen */
int	noflds,			/* no of field in the form */
	vdsize,			/* size of string array block at the end */
	drsize;			/* data record size */
};

struct	frmfld {		/* format of field record */

int	fldnam,			/* offset to field name if any else 0 */
	fldno,			/* field number */
	prompt,			/* offset to prompt string if any else 0 */
	imask,			/* offset to input mask string */
	dmask,			/* offset to display mask string */
	drloc,			/* displacement within data record */
	picstrng,		/* offset to COBOL picture string */
	helpmes,		/* offset to help message if any */
	lbound,			/* offset to lower bound string if any */
	ubound,			/* offset to upper bound string if any */
	eddata,			/* offset to edited data area */
	dupval;			/* offset to dup value if any */

char fldtyp,		/* type of field */
	fldclas,		/* class of field */
	promx,			/* line no of promt */
	promy,			/* column no of prompt */
	fldx,			/* line no of field mask */
	fldy,			/* column no of field mask */
	promclr,		/* prompt colour */
	fldclr,			/* field mask colour */
	dupctrl,		/* duplication control */
	lattr1,			/* logical attribute set 1 */
	lattr2,			/* logical attribute set 2 */
	promva,			/* prompt video attributes */
	fldva,			/* field mask video attributes */
	maskchar,		/* mask character */
	dfsize;			/* size of the field in data record */
};

#define TYP_YN		1 	/* yes/no field */
#define TYP_DATE	2 	/* date field */
#define TYP_NUM		3 	/* numeric field */
#define TYP_STRING	4 	/* string field */
#define TYP_NONE	0	/* for prompt only field */

#define CL_PROM		1 	/* prompt only field */
#define CL_FLD		2 	/* field only field */
#define CL_PRMFLD  	3 	/* prompt and field */

#define DUP_MASTER	1 	/* master duplication */
#define DUP_COPY	2 	/* copy duplication */
#define DUP_NONE	0	/* no duplication */

/* the following masks should be applied on lattr1 */

#define LA_REQ		001 	/* required */
#define LA_VALID	002 	/* validity check */
#define LA_UESC		004 	/* user escape */
#define LA_NOECHO	010 	/* no echo */
#define LA_HRET		020 	/* help return */
#define LA_SUP		040 	/* suppress leading zeros */

/* the following masks should be applied on lattr2 */

#define LA_BOUNDS	001 	/* bounds check */
#define LA_FH		002 	/* field hold */
#define LA_LCASE        020
#define LA_UCASE	004 	/* upper to lower case */
#define LA_SHODUP	010	/* show dup data */

/* the following masks are for video attributes */

#define VA_ULINE	001 	/* under line */
#define VA_REVERSE	002 	/* reverse video */
#define VA_BOLD		004 	/* high intensity */
#define VA_DIM		010 	/* low intensity */
#define VA_BLINK	020 	/* blinking */

struct	fldinfo {	/* internal data items of fields */
char oscrnsz,	/* on-screen-size of the mask part */
	Signed,		/* whether field is signed */
	decimals,	/* no of posiotions after decimal point */
	type,		/* actual type of field */
	stortype;	/* most appropriate data type for this field */
int	aoff;		/* offset in actual user data record */
};

/* values for fldinfo.type */

#define FITYP_NUM	1	/* numeric fields */
#define FITYP_NON	0	/* non-numeric fields */

/* values for fldinfo.stortype */

#define STOR_CHAR	0	/* char arrays */
#define STOR_SHORT	1	/* short integer */
#define STOR_INT	2	/* standard integer */
#define STOR_LONG	3	/* long integer */
#define STOR_FLOAT	4	/* single precision real */
#define STOR_DOUBLE	5	/* doble precision real */

