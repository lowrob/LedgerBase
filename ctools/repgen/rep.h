/*
	File name : rep.h
	Written By : V.Subramani
*/


/* define XENIX, UNIX, OMEGA or MS_DOS for IBM-AT XENIX version,
   68000 Family Unix, Omega 58000 and MS-DOS respectively */

#define	UNIX		/* 68000 Family Unix (NCR Tower) */

#ifdef	UNUSED
#define	XENIX		/* XENIX Version */
#define	MS_DOS		/* MS_DOS Version */
#define	OMEGA		/* Omega 58000 */
#endif


#define	PRINTER	"/etc/PRINTER"	/* Printer File Name */

/* TERMINAL CLEAR CHARACTER	*/
#define MMCLR  "\033[2J\033[H"
#define W50CLR "\033*"
#define OENCLR	
#define PERQCLR
#define TVICLR	


/*
		Page skip character for printer 
*/

#define PAGESKIP 12


/* Field class	*/
#define	COMP_FLD	'C'  /* Coputational field, so formula is stored */
#define INP_FLD		'I'  /* Input field, so field # and rec# are given*/

/* Field types	*/
#define NUME_FLD	'N'	/* Numeric field, non accumulator	*/
#define ACCU_FLD	'A'	/* Numeric , accumulator field - unused	*/
#define CHAR_FLD	'C'	/* Character field			*/


/*** Default edit masks, displayed on the screen	*/
#define SHORTEDIT	"___0_-"
#define INTEDIT		((ALGN_INT > 2) ? "_______0_-" : "___0_-")
#define LONGEDIT	"_______0_-"
#define FLOATEDIT	"______0_.__-"		/* 8+2 decimal included */
#define DOUBLEDIT	"_________0_.__-"	/* 11+ 2 decimal included */


/** code for Justifications	*/

#define RIGHTJUST	'R'
#define LEFTJUST	'L'
#define CENTREJUST	'C'

/*
	Check already stored value and the current value return appropriate
	value.
*/

#define VALUE_MATCH	1	/* old and new values are same */
#define VALUE_NOMATCH	0	/* not same */


/*
	Minimum and maximum boundary values can be set for a valid record
*/

#define	VALID_REC	0	/* Boundary codition satisfied */
#define INVALID_REC	1	/* Not satisfied and so skipped */

/*
	If the field value is repeated then show blanks
*/

#define REPEAT_YES 	1
#define REPEAT_NO	0		

/* 
	OUTPUT PAGE DEFAULT CONSTRAINTS
*/

#define O_LINESPACE	1	/* Default line spacing in the report */
#define O_COLUMNS	80	/* Page size , columns */
#define O_LINES		60	/* Page size, lines	*/

#define FILE_NAME_LEN	50 	/* File name length	*/
#define NAME_LEN  	21	/* Max. length of the name	*/
#define MAXSTRUCT 	 5	/* Max. of structures can be referred	*/
#define MAXFORMAT	9 /* Max. no. of formats allowed for one report*/

#define MAX_REPFLD	30	/* Maxi. #of reporting fields allowed	*/
#define MAX_KEYFLDS	5	/* Max first 2 for page totals and break,
				  Rest Max 3 for Subtotals at footer */

/* I/O buffers	*/
#define FRMBUFSIZE	4096	/* format record size	*/
#define STRBUFSIZE	4096 	/* Field def structure block size	*/


#ifdef	MS_DOS

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define	RDMODE		(O_BINARY | O_RDONLY)
#define	WRMODE		(O_BINARY | O_WRONLY)
#define	RWMODE		(O_BINARY | O_RDWR)
#define	CRMODE		(O_BINARY | S_IREAD | S_IWRITE)

#define	TXT_RDMODE	(O_TEXT | O_RDONLY)
#define	TXT_WRMODE	(O_TEXT | O_WRONLY)
#define	TXT_RWMODE	(O_TEXT | O_RDWR)
#define	TXT_CRMODE	(O_TEXT | S_IREAD | S_IWRITE)

#else

#define	RDMODE		0
#define	WRMODE		1
#define	RWMODE		2
#define	CRMODE		0666

#define	TXT_RDMODE	0
#define	TXT_WRMODE	1
#define	TXT_RWMODE	2
#define	TXT_CRMODE	0666

#endif


/*
	Format file structure. Max. of 9 formats per report is possible	
*/

/* field reference for multirecord input	*/

typedef struct ref_tofield {
	short	inrecno ; /* Which record in the array this field belongs */
	short	infieldno ; /* Field no. in the above record	*/
} FIELDNO ;



struct f_attri {
	char	fld_class;		/* COMP_FLD / INP_FLD */

	union	inposition {

		FIELDNO	fieldref ;	/* rec. no. and field no. ,
					   for INP_FLD only */
		short	comp_off[2];	/* Computational formula -
					   for COMP_FLD only */
					/* Reverse polish form	*/
	} in_takeno;

	char	fld_type;	/* NUMERIC / ACCUMULATOR / CHARACTER	*/
	short	edit_off;	/* justification for CHARACTER type 15 - 21*/
	short	title_off;	/* Field title	*/
	char	fld_justify ;	/* justification - new	*/
	char	fld_totflag ;	/* to be totalled or not 0/1	*/
	short	fld_size ;	/* Field size	*/
	short	fld_serno ;	/* serial number w. r.to screen window - new*/
	short	fname_off ;	/* field name supplied in commets - new*/
	char	repeatsuppress;	/* if value is repeated, display blank Y/N*/
	short	minoff ;	/* Minimum value string offset */
	short	maxoff ;	/* Maximum value string offset */
} ;

struct f_global {

	short	Numrecs ;		/* No of recs used for this report */
	short	Inrecno[MAXSTRUCT] ;	/* Record nos in the proj.in file */
	char	formname[21] ;		/* format name	*/
	short	tot_flds;		/* #of fields selected for report */
	short	tot_keys;		/* #of subtotal, key fields selected */
	short	tot_pghdr ;		/* No of sub-hdr keys. Max 2 */
	FIELDNO	keyno[MAX_KEYFLDS];	/* key fields - rec# and field# */
	short	keyoff[MAX_KEYFLDS] ;	/* sub total field names - new	*/
	short	subserno[MAX_KEYFLDS] ;	/* Ser. # corr. to window - new	*/
	short	msg_textoff[MAX_KEYFLDS] ;	/* Offset to message text
						   string */	
	short	keyeditoff[MAX_KEYFLDS] ;	/* Edit mask for the key value
						   display */
#ifdef	OLD 

	FIELDNO	s_fldno[2] ;		/* Sub header field numbers */
	short	s_serlno[2] ;		/* Sub Header field serial numbers */
	short	s_off[2] ;		/* Offset to Prompt string to sub hdr
					   fields */
	short	s_edit_off[2] ;		/* offset to edit mask string for
					   sub header fields */
#endif
   
	short	mhdnos;			/* #of header lines availbale	*/
	short	mhdoff[3];		/* Offset to header line string	*/
	short	page_columns ;		/* # of columns per page */
	short	page_lines ;		/* #of lines per page	*/
	char	linespace ;		/* Between 2 ouput lines, blank lines */
} ;



typedef union {
	short	vshort ;
	long	vlong ;
	float	vfloat ;
	double	vdouble ;
	int	vint ;
} NUMRET ;


/**
		Format record buffer length
**/

/** 
		Define the buffer for format record 
**/

static char frmbuf[FRMBUFSIZE] ;

static struct f_attri *fldattri ;
static struct f_global *fldglob ;

 
#define L_FLDATTR 	sizeof(struct f_attri)
#define L_FLDGLOB 	sizeof(struct f_global)
/*
		Start address of the variable length string fields
*/

#define VARADDR	frmbuf + (L_FLDGLOB + fldglob->tot_flds * L_FLDATTR)


