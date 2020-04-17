/*****************************************************************************
File Name 	: rp.c
Written By	: P.JAYADEEP
Modified By	: V.SUBRAMANI
Compilation	: cc rp.c appl.c -o appl, where appl.c is application program.

Usage/Description :
		This is library to be linked to application programs, providing
		following three calls :
	1. rpopen(projname,logrec,formno,outcntrl,discfile,pgm_name)
			where,
		int logrec : 	Logical record number , available in the table
				given by repname module.

	  	int formno : 	Fomat index number returned by Format defn. 
				screen module.
		char *projname :Project name. That is project C-SOURCE header
				file name without any(.h normally) extention.
		int outcntrl :	Range is between 0-6. Output option.
				Output on the Terminal(0)/Printer(1) etc..
		char *discfile:	If opted for the discfile output, then output
				file name.
		char *pgm_name: If programme name to be printed at top of each
				page . NULL to ignore .

		This call should be used once to read the format record and
		structure def. record. 
 
	2. rpline(ptrarray)
		char **ptrarray :Addresses of the records relevant to the 
				current report. 
		This will print single output line. If Key value is changed
		then subtotalling will be also done.

	3. rpclose()
		To Produce Grand total line, if any.

MODIFICATIONS
-------------
AMAR on 04-aug-89......
NOTE:	For COMP_FLDs 'fld_type' will be changed form NUME_FLD to either LONG
	or DOUBLE in rpopen()(call comp_typ()), to pass to edit(), to use in
	totalling etc..
	calc_rp()  also returns the same type.

AMAR on 28-SEP-89...
	"initiate_keys()", "report_embeded()", "check_keys()" etc. are changed.
	"report_embeded()" is changed to print values from previous key values
	instead of from current record. Because it is printing new key values,
	if it needs to skip page while printing subtotals for previous page
	break keys.

	get_curkyey() is deleted. p_int[], p_char[] and t_int[] varibales are
	deleted. Now prevkey(currkey) is initialized in initiate_key().

D. Richardson  20-OCT-92
	In order to break out of a report generation once it has begun, I
	added code to capture the SIGINT, interrupt process, signal and
	call a signal handler which I have created. The signal handler will
	remove all new and temporary print files created during this report 
	generation and kill the process. The signal handler is defined in 
	the file 'brkrpt.c' found in the DBH directory.

L. Robichaud  10-APR-2020
	Fixing compiling errors and warning.
*****************************************************************************/


#include <stdio.h>
#include <string.h>
//#include <stdlib.h>
/* Added by D. Richardson  10/20/92  - to handler break interrupts */
#include <signal.h>

#include "rep.h"
#include "repname.h"
#include "struct.h"

/* #define DEBUG */

/* #define ENGLISH */

#ifndef	MS_DOS		/* Not MS_DOS */

#define	SPOOLER

#endif

#define MXFLDSZ		100
#define NULL_ARG	-1 	/* used in get_ftype , to pass null argument */
#define NOCHANGE	50
#define CHKERROR(X)	if(X == ERROR) return(ERROR)
#define	reterr(X)	{ rperror = X ;\
			printf("Internal data error occurred.. Rperror: %d\n",\
			rperror) ;\
			getchar();\
			return(ERROR) ; }


#define fldhd	f_attri
#define fhd	f_global
#define fld_hd	fldattri
#define f_hd	fldglob
#define O_APPEND	010
#define	TXT_WRAPMODE	(TXT_WRMODE | O_APPEND)

/* Added by D. Richardson   10/20/92
   for report termination and clean-up */
extern void BreakReport(int);
extern char bk_filename[40];
extern short bk_rpt_opn_flg;

static	char	Stru_buf[MAXSTRUCT][STRBUFSIZE] ;
				 /* addresses array for structure defns  */
/**
extern 	char	*Stru_buf[MAXSTRUCT] ; 
**/

static	short	nfltits ;

static	short	pageno, maxlines, maxcols;
static	short	colspace ;

/* preserve the last key value to be used in subtotal string */

typedef union {
	short	vshort ;
	long	vlong ;
	float	vfloat ;
	double	vdouble ;
	int	vint ;
	char	cvalue[MXFLDSZ] ;
} UNIVAL ;

static	UNIVAL	prevkey[MAX_KEYFLDS];
static	char	keytype[MAX_KEYFLDS] ;

static	char 	prtout,		/* 1/0 - output on the printer */
		termout,	/* 1/0 - terminal display	*/
		discout ;	/* 1/0 - disc file output	*/

static	int	fdp ;	/* printer file fd	*/
static	int	fdd ;	/* disc output file fd	*/

static	char	*hline[5], *rline, *uline, *tit_line[5] ;
static	char	*top_line = NULL ;

static	char	*subline[MAX_KEYFLDS] ;

static	char	*cline ;

static	char	*gline ;
static	NUMRET	s_tot[MAX_KEYFLDS][MAX_REPFLD],g_tot[MAX_REPFLD];
static	short	size[MAX_REPFLD];
static	UNIVAL	prev_val[MAX_REPFLD] ;

static	short	pageoff;

static	short	linecount ;
static	char	*hdrptr = NULL ;	/* buffer used to compute the report
					   string */
static	char	**userec ;  /* Array of pointers passed by appl. program */

int	rperror ;

static	int	BoundsExists ;
static	int	summary ;
static	char	*sub_title = NULL ;

/* Report spooling variables */

#ifdef	SPOOLER
#define	RPTEMP	"RWXXXXXX"
char	*mktemp() ;
static	char	*printer ;
static	char	tempname[15] ;
static	int	numberofcopies;
#endif

static char	*malloc() ;
long	lseek() ;

/*** Define finctions and prototypes ***/
static char get_fld(FIELDNO, NUMRET *);
static char get_ftype(FIELDNO, short);
static int disp_msgtext(short);
static int printgline();
static void right_jstfy(char, char, int);
static void left_jstfy(char[], char[], int);
static int cpystr(char[], int, char[]);
static int main_header(char*, char*);
static int report_embedded(void);
static int print_header(void);
static int fieldtitles(void);
static int col_tit_line(short);
static int initiate_keys(int);
static int initzero_totals(NUMRET *);
static void rpline(char **);
#ifdef	DEBUG
static int dump_val(short);
#endif	
store_val();
static int check_repeat(short);
static int put_linespace(void);
static int check_bound(short);
int check_keys(void);
static int star(short, char[], short, short);
int max_title_size(void);
static int centre_jstfy(char[], char[], short);
static int cp(char[], int, char[]);
static int i_space(char[]);
static int underline(int);
int putline(char[]);
static int field(short, char*);
static int get_fieldval(UNIVAL *, short);
static int subtotals(short);
static int getyp(FIELDNO);
int rpopen(char*, int, int, int, char *, char *, char *);
int rpclose(void);
static int free_space(void);
static int acumulate(short, char, NUMRET *);
static int intizero_totals(NUMRET *);
#ifdef UNUSED
static int iszero(NUMRET *, short);
#endif
static int Rd_strrecs(void);
static int Rd_formrec(char *, short, short);
static int Open_strfile(char *, int, char *);
static int decide_dimension(void);
static int ptr_get(FIELDNO, char *);
static int allocate_lines(short);
static int maxltitle(char *);
static int findntitles(void);
static int calc_rp(char*, NUMRET *);
static int lpush(NUMRET*, char);
static char lpop(NUMRET *);
static int lgetop(char *, char *);
static int comp_typ(char *);
static int edit(NUMRET *, char *, char *, char);
void suppress(int);
void asterisk(int);
void flot(int);
void last(void);
int form_format(char *, char *, int);
void rm_dot(char *);
int rpAddSubtotal(int, int, char *, char *, char *, int);
int srch_fldnm(char *, int *, int *);
int rpChangetitle(int, char *);
int rpSummaryOn(void);
int rpSummaryOff(void);
int rpGetColPos(int);
int rpMkline(int, char*);
int rpPutline(void);
void rpAddSubtittle(char *);
int rpPagesize(int);
int rpSetCopies(int);
int rpclose_mesg(char *);
int putmesg(char[]);
/*** Beiging of functions ***/
static	char 
get_fld(fieldnr,retval)	/* gets fieldnr field value */
FIELDNO	fieldnr;
NUMRET	*retval ;
{
	char	*reqpos ;
	FLDINFO	*ptr ;

	ptr = (FLDINFO *)(Stru_buf[fieldnr.inrecno] + sizeof(INHDR)) ;
	ptr += fieldnr.infieldno ;
	reqpos = userec[fieldnr.inrecno] + ptr->posf ;

#ifdef DEBUG
	fprintf(stderr,"GET_FLD :\n") ;
	fprintf(stderr,"fldname : %s,typf :%c, posf : %d, lenf : %d\n",
	ptr->fldname,ptr->typf,ptr->posf,ptr->lenf) ;
#endif

	switch(ptr->typf) {
	case SHORT :
		retval->vshort= *((short *)reqpos) ;
		break ;
	case INT :
		retval->vint= *((int *)(reqpos)) ;
		break ;
	case LONG :
		retval->vlong= *((long *)reqpos) ;
		break ;
	case FLOAT :
		retval->vfloat= *((float *)reqpos) ;
		break ;
	case DOUBLE :
		retval->vdouble= *((double *)reqpos) ;
		break ;
	}

	return(ptr->typf) ;
}


static char 
get_ftype(fieldnr,n)	/* gets the type of the field */
FIELDNO	fieldnr;
short	n ;
{
	FLDINFO	*ptr ;

	if(n != NULL_ARG)  /* second argument is not used */
		if((fld_hd+n)->fld_class == COMP_FLD)
			return((fld_hd+n)->fld_type) ;

	ptr = (FLDINFO *)(Stru_buf[fieldnr.inrecno] + sizeof(INHDR)) ;
	ptr += fieldnr.infieldno ;

#ifdef DEBUG
	fprintf(stderr,"GET_FLD :\n") ;
	fprintf(stderr,"fldname : %s,typf :%c, posf : %d, lenf : %d\n",
	ptr->fldname,ptr->typf,ptr->posf,ptr->lenf) ;
#endif
	return(ptr->typf) ;
}


/* This is a special function to cater printing of message text assigned */
/* to a subtotal control key ; This line is printed when a subtotal key is */
/* changed.  */

 
static	int
disp_msgtext(curkeyno)
short	curkeyno ;
{
	char	*mesgbuf;
	int	i,j ;
	char	keyvalue[MXFLDSZ] ;

	if((mesgbuf = malloc(maxcols)) == NULL) {
		fprintf(stderr,"Memory allocation error\n") ;
		return(-1) ;
	}

	if(curkeyno > MAX_KEYFLDS -1 )
		return(0) ; 

	if ( strlen( VARADDR + f_hd->msg_textoff[curkeyno]) == 0 )
		strcpy (mesgbuf,"  *** Subtotal ***  "); 
	else
		sprintf(mesgbuf,"  %s  ",VARADDR + f_hd->msg_textoff[curkeyno]);

	if(keytype[curkeyno] != CHAR) {
		edit(&prevkey[curkeyno], VARADDR + f_hd->keyeditoff[curkeyno],
		 	keyvalue, keytype[curkeyno]) ;
		strcat(mesgbuf,keyvalue) ;
	}
	else
		strcat(mesgbuf,prevkey[curkeyno].cvalue) ;
	
	for(i = 0; i < maxcols ; i++)  /* pad blanks	*/
		if(mesgbuf[i] == '\0') {
			for(j = i; j < maxcols -1 ; j++)
				mesgbuf[j] = ' ' ;
			break ;
		}

	mesgbuf[maxcols-1] = '\n' ;
	putline(mesgbuf) ;
	free(mesgbuf) ;
	return(0) ;
}


 
static	int
printgline()
{
	char	*mesgbuf;
	int	i,j ;

	if((mesgbuf = malloc(maxcols)) == NULL) {
		fprintf(stderr,"Memory allocation error\n") ;
		return(-1) ;
	}

	strcpy(mesgbuf,"  ***  Grand Total ***") ;
	for(i = 0; i < maxcols ; i++)  /* pad blanks	*/
		if(mesgbuf[i] == '\0') {
			for(j = i; j < maxcols -1 ; j++)
				mesgbuf[j] = ' ' ;
			break ;
		}

	mesgbuf[maxcols-1] = '\n' ;
	putline(mesgbuf) ;
	free(mesgbuf) ;
	return(0) ;
}


/****************************************************************************/
/* This routine is equivalent to its earlier counterpart but differs	    */
/*	only in that right justification is done here	     		    */
/****************************************************************************/


static 
right_jstfy(st,sc,len)				/* right justification */
char	st[],sc[];
int	len;
{
	register int i,j,k,l;

	l = strlen(sc) ;
	if(l > len)
		l = len ;		/* restricted to field width	*/
	j = len - l;
	for(i=0;i<j;i++)
		st[i] = ' ';		/* left blanks */
	for(k=0;k<l;k++)
		st[i++] = sc[k];	/* sc copied */
	st[i] = '\0';
	return;
}


/****************************************************************************/
/*	This ,of course, varies in that it performs left justification
	on the given strings						*/
/****************************************************************************/


static void
left_jstfy(st,sc,len)				/* left justification */
char	st[],sc[];
int	len;
{
	register	int	i,j;

	j = strlen(sc);
	if(j > len)
		j = len ;		/* restricted to field width	*/
	for(i=0;i<j;i++)
		st[i] = sc[i];		/* sc copied */
	for( ;i<len; )
		st[i++] = ' ';		/* right blanks */
	st[i]='\0';
	return;
}


/*****************************************************************************/
/*	This routine serves the purpose of copying a part string from
	a buffer into a given string form the given start position until
	it faces a null character					*/
/*****************************************************************************/


static int
cpystr(st,start,sc)			/* copies part string of st to sc */
char	st[],sc[];
int	start;
{
	short i=0;

	while((sc[i++] = st[start++]) != '\0')
		;
}

/******************************************************************************/
/*   function of the main_header routine is to create main header lines       */
/*   mline[mhdnos] ( if main headers are to be printed )                     */
/******************************************************************************/

static int
main_header(pgm_name , date)
char	*pgm_name ;
char	*date ;
{
	int i ;

	/**
	*	Copy programme name and date on first and second line  
	*	if program name not given then date comes on first line
	*	otherwise it comes on second line ..
	**/

	i_space(top_line) ;

	if ( pgm_name == NULL || strlen(pgm_name) == 0 ) 
							;
	else 
		cp(top_line, 1, pgm_name) ;

	if ( date == NULL || strlen(date) == 0 ) 
							;
	else {
		i = cp(top_line, (maxcols-30), "Date: ") ;
		cp(top_line, i, date ) ;
	     }

	pageoff = cp(top_line,(maxcols-11),"Page:");
	*(top_line+maxcols-1)= '\n' ;

	for(i = 0 ; i < f_hd->mhdnos ; i++) {
		i_space(hline[i]);
		centre_jstfy(hline[i],VARADDR+f_hd->mhdoff[i],maxcols-1);
		*(hline[i]+(maxcols-1)) = '\n' ;
	}

	return;
}


/****************************************************************************/
/*   function of the report embedded is to create report header line rline  */
/*   with an embedded report fld (if report line is to be printed )         */
/****************************************************************************/

static int
report_embedded()
{
	/*****
	char	typ ;
	NUMRET	val ;
	FLDINFO	*fptr ;
	****/
	short	roff=0;
	char	rfld[MXFLDSZ];		/* values of  embedded fields */
	int	I ;

	if ( f_hd->tot_pghdr == 0 ) return(0) ;

	i_space(rline);

	for(I=0, roff = 0 ;I < f_hd->tot_pghdr; I++, roff += 4) {

		/****** By Amar on 28-Sep-89
		fptr = (FLDINFO *)(Stru_buf[f_hd->keyno[I].inrecno] +
						sizeof(INHDR)) ;
		fptr += f_hd->keyno[I].infieldno ;

		if(fptr->typf !=CHAR) {
			typ= get_fld(f_hd->keyno[I],&val) ; 
			edit(&val,VARADDR + f_hd->keyeditoff[I],rfld,typ) ;
		}
		else 
			ptr_get(f_hd->keyno[I],rfld) ;
		*****/
		/**** Added on 28-SEP-89 by AMAR ****/

		if(keytype[I] != CHAR)
			edit(&prevkey[I], (VARADDR+f_hd->keyeditoff[I]), rfld,
							keytype[I]) ;
		else
			strcpy(rfld,prevkey[I].cvalue) ;

		/******/

		roff = cp(rline, roff, (VARADDR+f_hd->msg_textoff[I]));
		roff = cp(rline, roff, "  " );	/* Space between Prompt &
						   Field Val */
		roff = cp(rline, roff, rfld ) ;
	} /* for */

	*(rline+maxcols-1) = '\n';

	return(1) ;
}



/*************************************************/
/*      This routine takes care of printing      */
/*       main_headers and report headers         */
/*       (title line in case of column wise )    */
/*       whenever they are to be printed         */
/*************************************************/

static int
print_header()
{
	short	i;
	char	tmp[10];
	NUMRET	val ;
	char	tempchar[5] ;
	
	linecount= 0 ;

	if(termout == 1) {	/* Display */
		if(pageno) {	/* If not first page haeding */
#ifdef ENGLISH
			write(1,
			"Press Q<RETURN> to Quit ELSE 'RETURN' to Continue  ",
			50); 
#else
			write(1,
			"Appuyer sur Q<RETURN> pour retourner sinon 'RETURN' pour continuer",
			65); 
#endif
			read(0,tempchar,2);
			if(tempchar[0] == 'Q' || tempchar[0] == 'q')
			        return(-1);
		}
		write(0,MMCLR,7) ;
	}
	/* Don't print form feed for the first page */
	tempchar[0] = PAGESKIP ;
	if(discout > 0  && pageno)
		write(fdd,tempchar,1) ;
	if(prtout == 1 && pageno)
		write(fdp,tempchar,1) ;

	pageno++;
	val.vshort= pageno ;
	edit(&val,"___0",tmp,SHORT);
	
	cp(top_line,pageoff,tmp);	/* Copy The cur. Page# */
	putline( top_line ) ;

	for(i=0;i<f_hd->mhdnos;i++)
		putline(hline[i]);

	underline(2) ;

	if ( report_embedded() ) {
		putline(rline);
		underline(2);
	}

	if ( sub_title ) 
		putline(sub_title) ;

	fieldtitles() ;
	underline(2);
	return(0);
}

static int
fieldtitles()
{
	short	i ;

	for(i = 0 ; i < nfltits ; i++)
		putline(tit_line[i]) ;
}



/******************************************/
/*     This routine creates title line    */     
/*      in case of column_wise report     */
/******************************************/

static int
col_tit_line(nt)
short	nt ;
{
	short	i, j, k ;
	char	temp[MXFLDSZ], *str, *ptr ;
	short	pos[5] ; /*** max 5 field title lines****/

	for(i= 0 ; i<nt ; i++) {
		i_space(tit_line[i]);
		pos[i]= -colspace ;	/* To start at 1st column */
	}

	for(i=0;i<f_hd->tot_flds;i++) {
		str= VARADDR +(fld_hd+i)->title_off ;
		ptr= str ; j= 0 ;
		for(;*str!= '\0';str++)
			if (*str== '\\') {
				*str= '\0' ;
				centre_jstfy(temp,ptr,size[i]);
				pos[j] = cp(tit_line[j],pos[j]+colspace,temp);
				strcpy(temp,"");
				ptr= str+1 ;
				j++ ;
			}
		centre_jstfy(temp,ptr,size[i]);
		pos[j] = cp(tit_line[j],pos[j]+colspace,temp);
		strcpy(temp,"");
		for (k= j+1 ; k<nt;k++) pos[k]= pos[j] ;
	}

	for(i= 0 ; i<nt ; i++)
		*(tit_line[i]+maxcols-1)= '\n' ;
}

/*
*	Copy key values to prev keys form the given key to least key.
*
*	Example:
*		Page Break Fld1
*		    Page Break Fld2
*			Sub Total Fld1
*			    Sub total Fld2
*				Sub total Fld3
*
*	If the key_no is (Page Break Fld2), initialize prev keys for this key
*	and all sub total keys.
*/

static int
initiate_keys(k)
int	k ;
{
	for( ; k < (f_hd->tot_keys + f_hd->tot_pghdr) ;  k++) {
		if((keytype[k] = get_ftype(f_hd->keyno[k],NULL_ARG)) != CHAR)
			get_fld(f_hd->keyno[k],&prevkey[k]) ;
 		else
 			ptr_get(f_hd->keyno[k],prevkey[k].cvalue) ;
	}
}

/************************************************/
/*  This is a major routine which takes care of */
/*   printing of   the line				*/
/***********************************************/

static	short	repflag ;

rpline(record)
char	**record ;	/* Array of pointers to the input records	*/
{
 	short	n ;
  	short	key_no ;
  	char	strg[MXFLDSZ] ;
  	short	noff;

	userec = record ;

	/* added by D. Richardson 10/20/92 for breaking reports */
	signal(SIGINT,BreakReport);

	if(BoundsExists)
		for(n = 0; n<f_hd->tot_flds ; n++)
			if(check_bound(n) == INVALID_REC)
				return(NOERROR) ;


	if (pageno == 0) { 	/* Report start */

		/* Next 2 functions calling sequence is interchanged on
		   28-SEP-89 by AMAR */
		initiate_keys(0) ;
		print_header() ; 

		/* First time copy values to prev_values. It may be used
		   in repeat suppression check */
		CHKERROR(store_val()) ;

		repflag = 1 ;	/* Force Printing of Repeat supress fields */
	}
	else if((key_no = check_keys()) != NOCHANGE) {

		if(subtotals(key_no) < 0) return(-1) ;
		underline(2);

		/* Inserted on 28-sep-89 by amar */
		initiate_keys(key_no) ;	/* Initialize Cur Key */

		if (key_no < f_hd->tot_pghdr){	/* key_no betwn 0 &1 */
			if(print_header() < 0) return(-1) ;
		}
		repflag = 1 ;	/* Force Printing of Repeat supress fields */
	}

	noff = -colspace ;	/* Initialize noff to -column spaces to strart
				   first al 1st column */

	if(linecount >= maxlines) {
		if(print_header() < 0) return(-1) ;
		repflag = 1 ;	/* Force Printing of Repeat supress fields */
	}

	for(n = 0 ; n < f_hd->tot_flds ; n++) {
		CHKERROR(field(n,strg)) ;

		if ( summary ) continue ;

#ifdef DEBUG
		dump_val(n) ;
#endif
		if(((fld_hd+n)->repeatsuppress)) {
			if(check_repeat(n) != VALUE_MATCH) {
				CHKERROR(get_fieldval(prev_val+n,n)) ;	
				repflag = 1 ;
			}
			if(repflag == 0) {
				noff += size[n] + colspace ;
				continue ;
			}
		}
		noff = cp(cline,(noff+colspace),strg);
		strcpy(strg,"");
	}

	repflag = 0 ;

	if ( summary ) return(NOERROR) ;

	cline[maxcols-1] ='\n';
	putline(cline);
	i_space(cline);
	put_linespace() ;
	return(NOERROR) ;
}

#ifdef	DEBUG

static int
dump_val(n) 
short n ;
{
	UNIVAL val ;
	char f_type ;

	f_type = get_fieldval(&val,n) ;
	fprintf(stderr,"DUMB_VAL :\nfieldno : %d type : %c\n",n,f_type) ;

	switch(f_type) {
	case SHORT :
		fprintf(stderr,"short :%d\n",val.vshort) ;
		break ;
	case INT :
		fprintf(stderr,"Int : %d\n",val.vint) ;
		break ;
	case LONG :
		fprintf(stderr,"Long : %d\n",val.vlong) ;
		break ;
	case FLOAT :
		fprintf(stderr,"Float : %f\n",val.vfloat) ;
		break ;
	case DOUBLE :
		fprintf(stderr,"DOUBLE :%f\n",val.vdouble) ;
		break ;
	case CHAR :
		fprintf(stderr,"CHAR : %s\n",val.cvalue) ;
		break ;
	default : fprintf(stderr,"dump_val : wrong type\n") ;
		return(ERROR) ;
	}
	return(NOERROR) ;
}

#endif

/* 
	Store initial values of the report fields in an array, 
	for repeat fields suppress
*/

store_val()
{
	short i ;

	for(i = 0; i < f_hd->tot_flds ; i++) 
		CHKERROR(get_fieldval(prev_val + i,i)) ;

	return(NOERROR) ;
}

/*
	Check if value stored in prev_val[] and the current value are the same.
*/

static int
check_repeat(n)
short n ;
{
	UNIVAL val ;
	char f_type ;

	f_type = get_fieldval(&val,n) ;
	switch(f_type) {
	case SHORT : if(val.vshort != prev_val[n].vshort)
			return(VALUE_NOMATCH) ;	
		break ;

	case INT : if(val.vint != prev_val[n].vint)
			return(VALUE_NOMATCH) ;	
		break ;

	case LONG : if(val.vlong != prev_val[n].vlong)
			return(VALUE_NOMATCH) ;	
		break ;

	case FLOAT : if(val.vfloat != prev_val[n].vfloat)
			return(VALUE_NOMATCH) ;	
		break ;

	case DOUBLE : if(val.vdouble != prev_val[n].vdouble)
			return(VALUE_NOMATCH) ;	
		break ;

	case CHAR : if(strcmp(val.cvalue,prev_val[n].cvalue) != 0)
			return(VALUE_NOMATCH) ;
	}
	return(VALUE_MATCH) ;
}


/*
	Between two output lines leave specified no. of blank lines
*/

static int
put_linespace()
{
	int i ;

	for(i = 1; i < f_hd->linespace ; i++)
		underline(2) ;
	return(NOERROR) ;
}

/*
	Get the Minimum and maximum boundary values from the input, and
	check for the valid record. If record value is out of bound , return
	INVALID_REC, else VALID_REC
*/

static int
check_bound(n)
short n ;	/* Current report field	*/
{
	char f_typ ;
	UNIVAL	cur_value, bound_val ;
	
	if(strlen(VARADDR + (fld_hd+n)->minoff) == 0 &&
			strlen(VARADDR + (fld_hd+n)->maxoff) == 0)
		return(VALID_REC) ;

	f_typ = get_fieldval(&cur_value,n) ;
	if(strlen(VARADDR + (fld_hd+n)->minoff) > 0)
	switch(f_typ) {
	case SHORT : sscanf(VARADDR + (fld_hd+n)->minoff,"%hd",&bound_val.vshort ) ;
			if(cur_value.vshort < bound_val.vshort)
				return(INVALID_REC) ;
		break ;

	case INT : sscanf(VARADDR + (fld_hd+n)->minoff,"%d",&bound_val.vint) ;
			if(cur_value.vint < bound_val.vint)
				return(INVALID_REC) ;
		break ;

	
	case LONG : sscanf(VARADDR + (fld_hd+n)->minoff,"%ld",&bound_val.vlong ) ;
			if(cur_value.vlong < bound_val.vlong)
				return(INVALID_REC) ;
		break ;

	case FLOAT : sscanf(VARADDR + (fld_hd+n)->minoff,"%f",&bound_val.vfloat ) ;
			if(cur_value.vfloat < bound_val.vfloat)
				return(INVALID_REC) ;
		break ;

	case DOUBLE : sscanf(VARADDR + (fld_hd+n)->minoff,"%lf",&bound_val.vdouble ) ;
			if(cur_value.vdouble < bound_val.vdouble)
				return(INVALID_REC) ;
		break ;

	case CHAR : strcpy(bound_val.cvalue,VARADDR + (fld_hd+n)->minoff) ;
			if(strcmp(cur_value.cvalue ,bound_val.cvalue) < 0)
				return(INVALID_REC) ;
		break ;
	}
	
	if(strlen(VARADDR + (fld_hd+n)->maxoff) > 0) 
	switch(f_typ) {
	case SHORT : sscanf(VARADDR + (fld_hd+n)->maxoff,"%hd",&bound_val.vshort ) ;
			if(cur_value.vshort > bound_val.vshort)
				return(INVALID_REC) ;
		break ;

	case INT : sscanf(VARADDR + (fld_hd+n)->maxoff,"%d",&bound_val.vint) ;
			if(cur_value.vint > bound_val.vint)
				return(INVALID_REC) ;
		break ;

	
	case LONG : sscanf(VARADDR + (fld_hd+n)->maxoff,"%ld",&bound_val.vlong ) ;
			if(cur_value.vlong > bound_val.vlong)
				return(INVALID_REC) ;
		break ;

	case FLOAT : sscanf(VARADDR + (fld_hd+n)->maxoff,"%f",&bound_val.vfloat ) ;
			if(cur_value.vfloat > bound_val.vfloat)
				return(INVALID_REC) ;
		break ;

	case DOUBLE : sscanf(VARADDR + (fld_hd+n)->maxoff,"%lf",&bound_val.vdouble ) ;
			if(cur_value.vdouble > bound_val.vdouble)
				return(INVALID_REC) ;
		break ;


	case CHAR : strcpy(bound_val.cvalue,VARADDR + (fld_hd+n)->maxoff) ;
			if(strcmp(cur_value.cvalue ,bound_val.cvalue) > 0)
				return(INVALID_REC) ;
		break ;
	
	default : fprintf(stderr,"Incosistency error occurred\n") ;
		fprintf(stderr,"rp.c , check_bound()\n") ;

	}

	return(VALID_REC) ;
}


/*
*	Check whether any of page break or sub total field is changed. If so
*	return the key number changed.
*/

int
check_keys()
{
	short	kn ;
	char	ctemp[MXFLDSZ], typ ;
	FLDINFO	*ptr ;
	NUMRET	t_int ;

	if(f_hd->tot_keys == 0 && f_hd->tot_pghdr == 0)
		return(NOCHANGE) ;

	/* Compare from highest level and return the key_no where it is
	   changed */
	for( kn = 0 ; kn < (f_hd->tot_keys + f_hd->tot_pghdr) ; kn++) {
	    if(keytype[kn] != CHAR) {
		typ = get_fld(f_hd->keyno[kn],&t_int) ;
		switch (typ) {
		case SHORT :
			if ( prevkey[kn].vshort != t_int.vshort)
				return(kn) ;
			break ;
		case INT :
			if (prevkey[kn].vint != t_int.vint)
				return(kn) ;
			break ;
		case LONG :
			if (prevkey[kn].vlong != t_int.vlong)
				return(kn) ;
			break ;
		case FLOAT :
			if (prevkey[kn].vfloat != t_int.vfloat)
				return(kn) ;
			break ;
		case DOUBLE :
			if (prevkey[kn].vdouble != t_int.vdouble)
				return(kn) ;
			break ;
		}	/* Switch */
	    }	/* if() */
	    else {
		ptr = (FLDINFO *)(Stru_buf[f_hd->keyno[kn].inrecno] +
				sizeof(INHDR)) ;
		ptr += f_hd->keyno[kn].infieldno ;

		ptr_get(f_hd->keyno[kn], ctemp) ;

		if(strncmp(prevkey[kn].cvalue,ctemp,ptr->lenf ) )
			return(kn) ;
	    }	/* else */

	}	/* for() */

 	return(NOCHANGE);
}


#ifdef	UNUSED

static int
star(fldno,dest,opt,formattype)
short formattype;
short fldno,opt;
char dest[];
{
	int k;
	char splchar;
	if(opt==1) {
		splchar = ' ';
	}
	else
		splchar = '*' ;
/*****
	if(formattype==1) {
		if((fld_hd+fldno)->fld_type!=CHAR_FLD)
			size[fldno] = strlen(VARADDR+(fld_hd+fldno)->edit_off);
		else
			size[fldno] = (fld_hd+fldno)->fld_size;
	}
	else {

		size[fldno] = f_hd.max_width_fld;

	}
*******/
	for(k=0;k<size[fldno];k++) {
		dest[k] = splchar;
	}
	dest[k++] = '\0' ;
}

#endif


/******
int
max_title_size()
{
	int i,k;
	k=0;
	for(i=0;i<f_hd->tot_flds;i++) {
		if (k<strlen((fld_hd+i)->fld_title))
			k= strlen((fld_hd+i)->fld_title) ;
	}
	return(k);
}
*****/

static int
centre_jstfy(dest,srce,len)
char dest[];
char srce[];
short  len ;
{
	short i,j,r,k;

	i = strlen(srce);
	if(i > len)
		i = len ;	/* restricted to field width	*/

	j = (len - i) / 2;

	for(r = 0 ; r < j ; r++)
		dest[r] = ' ' ;

	for(k=0; (srce[k]!='\0') && (k < len);k++)
		dest[r++] = srce[k];

	for( ; r<len ; )
		dest[r++] = ' ' ;

	dest[r] = '\0' ;
}



static int
cp(dest,strt,srce)
char dest[];
char srce[];
int  strt;
{
	short i ;
	for(i=0; srce[i]!='\0'; i++) {
		dest[strt++] = srce[i] ;
	}
	return(strt);
}






#define SPACE ' '



int
i_space(lne)
char lne[];
{
	register int i;
	for(i=0;i<maxcols;i++) {
		lne[i] = SPACE ;
	}
	lne[maxcols-1] = '\n' ;
}

static int
underline(opt)
int opt;
{
	short i;
	char ULINE;
	if(opt==1)
		ULINE = '_';
	else
		ULINE = SPACE ;
	for(i=0; i<maxcols; i++) {
		*(uline+i) = ULINE ;
	}
	*(uline+maxcols-1) = '\n' ;
	putline(uline);
}


int
putline(line)
char line[];
{
	short count;

	if(line==NULL)
		return(0);

	if(termout)
		write(1,line,maxcols);
	if(prtout)
		write(fdp,line,maxcols);
	if(discout) {
		count = write(fdd,line,maxcols);

		if(count!=maxcols) {
			fprintf(stderr,"error in writing a line");
			return(-1);
		}
	}
	linecount++ ;
	return(NOERROR) ;
}



static int
field(n,strg)
short n ;
char *strg ;
{
	char	spol[80] ;
	NUMRET	val ;
	int	typ ;
	char	ptr[MXFLDSZ], temp[MXFLDSZ] ;

#ifdef	DEBUG
	fprintf(stderr,"FIELD : type :%c class:%c fld_size :%d\n",
	    (fld_hd+n)->fld_type,(fld_hd+n)->fld_class,(fld_hd+n)->fld_size) ;
#endif

	if((fld_hd+n)->fld_type != CHAR_FLD) {	/* NUME_FLD or (LONG or DOUBLE
						   for COMP_FLD) */
		if ((fld_hd+n)->fld_class == INP_FLD)
			typ = get_fld((fld_hd+n)->in_takeno.fieldref,&val) ;
		else if ((fld_hd+n)->fld_class == COMP_FLD) {
			cpystr(VARADDR,(fld_hd+n)->in_takeno.comp_off[1],spol);
			if((typ = calc_rp(spol,&val)) == ERROR) {
				fprintf(stderr,"error in calc-rp routine\n");
				reterr(INTERR) ;
			}
		}
		else {
			fprintf(stderr,"Invalid NUMERIC Field.. in field()\n");
			reterr(INTERR) ;
		}

		if((fld_hd+n)->fld_totflag)
			acumulate(n,(char)typ,&val) ;

		CHKERROR(edit(&val,VARADDR+(fld_hd+n)->edit_off,
				temp,(char)typ));
		
		/* centre justify the numeric fields to allign with col hdg.
		   This takes care alignment problem, if Edit mask size is
		   less than the size allocated to filed */

		centre_jstfy(strg,temp,size[n]);
	}
	else {
		ptr_get((fld_hd+n)->in_takeno.fieldref,ptr) ;
		if((fld_hd+n)->fld_justify == CENTREJUST) /* centre justn */
			centre_jstfy(temp,ptr,size[n]);
		if((fld_hd+n)->fld_justify == RIGHTJUST) /* right justn */
			right_jstfy(temp,ptr,size[n]);
		if((fld_hd+n)->fld_justify == LEFTJUST)   /* left justn */
			left_jstfy(temp,ptr,size[n]);
		strcpy(strg,temp);
	}

	return(NOERROR) ;
}


static int
get_fieldval(val,n)
UNIVAL *val ;
short n ;
{
	int	f_type ;
	char	spol[80] ;

	if ((fld_hd+n)->fld_class== INP_FLD) {
		f_type = get_ftype((fld_hd+n)->in_takeno.fieldref,n) ;
		if(f_type == CHAR)
			ptr_get((fld_hd+n)->in_takeno.fieldref,val->cvalue) ;
		else
			get_fld((fld_hd+n)->in_takeno.fieldref,val) ;
	}
	else if ((fld_hd+n)->fld_class== COMP_FLD) {
		/***
		if(((f_type = calc_rp(VARADDR+(fld_hd+n)->in_takeno.comp_off[1],
							val))) == ERROR) {
		***/
		cpystr(VARADDR,(fld_hd+n)->in_takeno.comp_off[1],spol);
		if(((f_type = calc_rp(spol, val))) == ERROR) {
			fprintf(stderr,"error in calc-rp routine\n");
			reterr(ERROR) ;
		}
	}
	else {
		fprintf(stderr,"Wrong field type : get_fieldval\n") ;
		return(ERROR) ;
	}
	
	return(f_type) ;
}

	
static int
subtotals(key_no)
short key_no ;
{
	short	noff, i, n;
	char	strg[MXFLDSZ], temp[MXFLDSZ], typ ;

	for(i=(f_hd->tot_keys + f_hd->tot_pghdr -1);i>=key_no;i--) {
		i_space(subline[i]) ;

		noff = -colspace ;	/* Initialize noff to -column spaces
					   to strart first al 1st column */
		for(n=0;n<f_hd->tot_flds;n++) {

			if(!(fld_hd+n)->fld_totflag) {
				noff += size[n] + colspace ;
				continue ;
			}

			if((fld_hd+n)->fld_class == COMP_FLD)
				typ = (fld_hd+n)->fld_type;
			else
				typ= getyp((fld_hd+n)->in_takeno.fieldref) ;

			CHKERROR(edit(&s_tot[i][n],VARADDR+(fld_hd+n)->edit_off,
				temp,(char)typ));

			/* centre justify the numeric fields to allign with col
			   hdg. This takes care alignment problem, if Edit
			   mask size is less than the size allocated to filed */

			centre_jstfy(strg,temp,size[n]);

			noff = cp(subline[i],noff+colspace,strg);
			strcpy(strg,"");
		}
	}

	for(i=(f_hd->tot_keys + f_hd->tot_pghdr -1);i>=key_no;i--) { 

		if(linecount >= maxlines) {
			if(print_header() < 0) return(-1) ;
		}
		disp_msgtext(i) ;
		*(subline[i]+(maxcols-1)) = '\n';
		putline(subline[i]);
		initzero_totals(s_tot[i]) ;
	 }

	return(0) ;
}


static int
getyp(n)
FIELDNO n ;
{
	FLDINFO *ptr ;

	ptr= (FLDINFO *)(Stru_buf[n.inrecno]+sizeof(INHDR)) ;
	return((ptr+n.infieldno)->typf) ;
}


int
rpopen(name,logrec,formrec,outopt,discfile, pgm_name, date)
char	*name ;
int	logrec ;
int	formrec ;
int	outopt ;	/* Output media option, TERM,DISC,PRTR */
char	*discfile ;	/* In case of disc file output, file name	*/
char 	*pgm_name ;	/* programme name */
char	*date ;
{
	short	i, n, lenth1, lenth2, ntitle ;

	numberofcopies = 1;

	f_hd = (struct fhd *)(frmbuf) ;
	fld_hd = (struct fldhd *)(f_hd + 1) ; 
	/***
	fld_hd = (struct fldhd *)(frmbuf+sizeof(struct fhd)) ; 
	***/

	CHKERROR(Open_strfile(name,outopt,discfile)) ;
	CHKERROR(Rd_formrec(name,logrec,formrec)) ; 
	CHKERROR(Rd_strrecs()) ;

#ifdef 	DEBUG
	fprintf(stderr,"Linespace : %d\n",f_hd->linespace) ;
#endif

	BoundsExists = 0 ;

	for(n = 0 ; n < f_hd->tot_flds ; n++) {
#ifdef	DEBUG
		fprintf(stderr,"title :%s\n",VARADDR + (fld_hd+n)->title_off) ;
		fprintf(stderr,"minbound : %s Maxbound : %s\n",
		VARADDR + (fld_hd+n)->minoff,VARADDR + (fld_hd+n)->maxoff) ;

#endif
		lenth1 = maxltitle(VARADDR + (fld_hd+n)->title_off);
		lenth2 = (fld_hd+n)->fld_size > strlen(VARADDR+(fld_hd+n)->edit_off) ?
			(fld_hd+n)->fld_size : strlen(VARADDR + (fld_hd+n)->edit_off) ;
		size[n] = (lenth1 < lenth2) ? lenth2 : lenth1 ;
		size[n] = (fld_hd+n)->fld_size == 0 ? 0 : size[n] ;

		if(BoundsExists == 0)
			if(strlen(VARADDR + (fld_hd+n)->minoff) ||
				strlen(VARADDR + (fld_hd+n)->maxoff))
					BoundsExists = 1 ;

		/* Find COMP_FLDs type... Added by AMAR on 04-aug-89 */
		if((fld_hd+n)->fld_class == COMP_FLD)
		    (fld_hd+n)->fld_type =
			comp_typ((VARADDR+(fld_hd+n)->in_takeno.comp_off[1]));
	}

	ntitle= findntitles() ;
	if (ntitle>5) reterr(100) ;
#ifdef DEBUG	
printf("#of titles :%d\n",ntitle) ;
#endif
	CHKERROR(decide_dimension()) ;
	CHKERROR(allocate_lines(ntitle)) ;
	
	if( (f_hd->tot_keys + f_hd->tot_pghdr) !=0) {
		for(i=0;i< (f_hd->tot_keys + f_hd->tot_pghdr); i++) {
			/****
			fprintf(stderr,"rec2 :%d fld2 :%d\n",
				(fld_hd+n)->in_takeno.fieldref.inrecno,
				(fld_hd+n)->in_takeno.fieldref.infieldno) ;
			*****/
			initzero_totals(s_tot[i]) ;
			i_space(subline[i]);
		}
	}

	initzero_totals(g_tot) ;
	col_tit_line(ntitle) ;
	pageno = 0;	
	main_header(pgm_name, date) ;
	i_space(cline);
	nfltits = ntitle ;

	summary = 0;
	sub_title = NULL ;

	/* Added by D. Richardson  10/20/92
	   to handle the termination and clean-up of reports. */
	bk_rpt_opn_flg = 1;
	if ( prtout )
		strcpy(bk_filename,printer);
	else
		strcpy(bk_filename,discfile);

	return(0) ;
}
	


rpclose()
{
	char	strg[MXFLDSZ], tmp[MXFLDSZ], typ ;
	short	i, noff ;
	char	glinedisplay ;	/* to display grand total line ? */
	char	tempchar[5] ;

	if(pageno) {

		glinedisplay = 0 ;

		/* If atleast one line printed and sub total fields exists
		   then print sub totals */
		if((f_hd->tot_keys + f_hd->tot_pghdr) > 0) {
			subtotals(0) ;
		}

		i_space(gline);
		noff = -colspace ;	/* To strat with 1st column */
	
		for(i=0; i<f_hd->tot_flds; i++) {
			if(!(fld_hd+i)->fld_totflag) { /* not to be totalled */
				noff += size[i] + colspace ;
				continue ;
			}
	
			glinedisplay = 1;
			if((fld_hd+i)->fld_class == COMP_FLD)
				typ =  (fld_hd+i)->fld_type;
			else
				typ= getyp((fld_hd+i)->in_takeno.fieldref) ;
			CHKERROR(edit(&g_tot[i],VARADDR+(fld_hd+i)->edit_off,tmp,typ));
	
			/* centre justify the numeric fields to allign with col
			   hdg. This takes care alignment problem, if Edit
			   mask size is less than the size allocated to filed */
	
			centre_jstfy(strg,tmp,size[i]);
	
			noff = cp(gline,noff+colspace,strg);
			strcpy(tmp,"");
			strcpy(strg,"");
		}
	
		/* Print headings, if the page is full */
		if(linecount > (maxlines-1))
			print_header();
	
		*(gline+maxcols-1) = '\n';
		
		if(glinedisplay)
			printgline() ;
		
		putline(gline);
		underline(1);
		initzero_totals(g_tot) ;
	
		/* If not Spooler, Generate Page skip at the end of the report
		   for disk and printer outputs */
#ifndef	SPOOLER
		tempchar[0] = PAGESKIP ;
		if(discout > 0 )
			write(fdd,tempchar,1) ;
		if(prtout == 1)
			write(fdp,tempchar,1) ;
#endif
		
	}	/* If pageno */

	if(discout == 2){
		tempchar[0] = PAGESKIP;
		write(fdd,tempchar,1) ;
	}
	/* close output file	*/
	if(prtout) {
		close(fdp) ;
#ifdef	SPOOLER
		/* If any report is genetrated, Spool it */
		if(pageno) {
#ifdef	XENIX
			sprintf(tmp, "lp -s -c -n%d %s",numberofcopies,printer);
#else
#ifdef i386
			sprintf(tmp, "lp -s -o nobanner -c -n%d %s",numberofcopies,printer);
#else
			sprintf(tmp, "lpr -cp %d %s",numberofcopies,printer);
#endif
#endif
			system(tmp) ;
		}
		unlink(printer) ;
#endif
	}
	if(discout)
		close(fdd) ;

	if(termout == 1) {
#ifdef ENGLISH
		write(1, "Press RETURN to Continue  ", 26); 
#else
		write(1, "Appuyer sur RETURN pour continuer ", 34); 
#endif
		read(0,tmp,2);
	}

	free_space() ; /* free the dynamically allocated space */

	return(0) ;
}

/**
	Free some global buffer space, allocated dynamically at run time
*/


static
free_space()
{
	if(top_line != NULL)
		free(top_line) ;
	if(hdrptr != NULL)
		free(hdrptr) ;	
	top_line = NULL ;
	hdrptr   = NULL ;
	return(NOERROR) ;
}


static int
acumulate(n,typ,val)
short	n ;
char	typ ;
NUMRET	*val ;
{
	short	i = 0 ;

	switch (typ) {
	case SHORT :
		for(i = 0 ; i < f_hd->tot_keys+f_hd->tot_pghdr ; i++)
			s_tot[i][n].vshort += val->vshort ;

		g_tot[n].vshort += val->vshort ;
		break ;
	case INT :
		for(i = 0 ; i < f_hd->tot_keys+f_hd->tot_pghdr ; i++)
			s_tot[i][n].vint += val->vint ;

		g_tot[n].vint += val->vint ;
		break ;
	case LONG :
		for(i = 0 ; i < f_hd->tot_keys+f_hd->tot_pghdr ; i++)
			s_tot[i][n].vlong += val->vlong ;

		g_tot[n].vlong += val->vlong ;
		break ;
	case FLOAT :
		for(i = 0 ; i < f_hd->tot_keys+f_hd->tot_pghdr ; i++)
			s_tot[i][n].vfloat += val->vfloat ;

		g_tot[n].vfloat += val->vfloat ;
		break ;
	case DOUBLE :
		for(i = 0 ; i < f_hd->tot_keys+f_hd->tot_pghdr ; i++)
			s_tot[i][n].vdouble += val->vdouble ;

		g_tot[n].vdouble += val->vdouble ;
		break ;
	}
}
static int
initzero_totals(ptr)
NUMRET *ptr ;
{
	short i;
	char typ ;

	for (i = 0 ; i<f_hd->tot_flds ; i++) {
		if((fld_hd+i)->fld_class == COMP_FLD)
			typ = (fld_hd+i)->fld_type;
		else 
			typ= getyp((fld_hd+i)->in_takeno.fieldref) ;

		switch(typ) {
		case SHORT  :
			(ptr+i)->vshort= (short)0 ;
			break ;
		case INT :
			(ptr+i)->vint= (int)0 ;
			break ;
		case LONG :
			(ptr+i)->vlong= (long)0 ;
			break ;
		case FLOAT : 
			(ptr+i)->vfloat= (float)0 ;
			break ;
		case DOUBLE :
			(ptr+i)->vdouble= (double)0 ;
			break ;
		default :
			break ;
		}
	}
}

#ifdef UNUSED

static int
iszero(ptr,n)
NUMRET *ptr ;
short n ;
{
	short fl= 0 ;
	char typ ;
	
	if((fld_hd+i)->fld_class == COMP_FLD)
		typ = (fld_hd+n)->fld_type ;
	else
		typ= getyp((fld_hd+n)->in_takeno.fieldref) ;
	switch(typ) {
		case SHORT  : if (ptr->vshort== (short)0) fl= 1; break ;
		case INT : if (ptr->vint== (int)0) fl= 1; break ;
		case LONG : if (ptr->vlong== (long)0) fl=1; break ;
		case FLOAT : if (ptr->vfloat== (float)0) fl=1; break ;
		case DOUBLE : if (ptr->vdouble== (double)0) fl=1; break ;
		default : break ;
	}
	return(fl) ;
}

#endif

static int fds;	/* Structure def. file fd	*/
/** read all the structure records used for the current report	*/
static int 
Rd_strrecs()
{
	int i ;

	/* Max. 'MAXSTRUCT' structures per report allowed */

	if(f_hd->Numrecs < 1 || f_hd->Numrecs > MAXSTRUCT){
		fprintf(stderr,"Internal consistency error : Numstruct: %d\n",
			f_hd->Numrecs) ;
		fprintf(stderr,"File : rp.c Func : Rd_strrecs\n") ;
		reterr(INTERR) ;
	}

	for(i = 0 ; i < f_hd->Numrecs ; i++) {
		/**
		if((Stru_buf[i] = malloc(STRBUFSIZE)) == NULL){ 
			fprintf(stderr,"Memory allocation error\n") ;
			fprintf(stderr,"File rp.c. Func : Rd_strrecs\n") ;
			reterr(MEMERR) ;}
		static allocation	******/

		if(lseek(fds,(long)((long)(f_hd->Inrecno[i])) * STRBUFSIZE,0) <
									0) {
			fprintf(stderr,
			"Seek error on structure def. file : Inrecno: %d\n",
			f_hd->Inrecno[i]) ;
			fprintf(stderr,"File : rp.c. Func : Rd_strrec");
			reterr(SEEKERR) ;
		}

		if(read(fds,Stru_buf[i],STRBUFSIZE) < STRBUFSIZE) {
			fprintf(stderr,
			"Stucture record read error,Inrecno: %d\n",
			f_hd->Inrecno[i]) ;
			fprintf(stderr,"File rp.c. Func : Rd_strrec\n") ;
			reterr(READERR) ;
		}
	}

	close(fds) ;

	return(NOERROR) ;
}




/* read the specified format record 	*/
static int 
Rd_formrec(proj,logrec,formrec)
char *proj ;
short logrec ;
short formrec ;
{
	char	filename[FILE_NAME_LEN] ;
	struct	rp_name repnamerec ;
	int	fdfr ;

	strcpy(filename,proj) ;
	strcat(filename,REPNAME) ;	/* structure def. file	*/
	if((fdfr = open(filename,RDMODE)) == -1){
		fprintf(stderr,"%s file open-read error\n",filename) ;
		fprintf(stderr,"Module : rp.c. Func : Rd_formrec\n") ;
		reterr(OPENERR) ;
	}

#ifdef DEBUG
	fprintf(stderr,"logrec : %d formrec# : %d\n",logrec,formrec) ;
#endif
	if(logrec < 1 ) {
		fprintf(stderr,"Wrong logical record number\n");
			fprintf(stderr,"Module : rp.c . Func : Rp_formrec\n");
			reterr(INTERR) ;
	}
	if(lseek(fdfr,(long)((logrec - 1) * sizeof(struct rp_name)), 0) < 0){
		fprintf(stderr,"Lseek error in repname file\n");
		fprintf(stderr,"Module : rp.c . Func : Rd_formrec\n");
		reterr(SEEKERR) ;
	}
	if(read(fdfr,(char *)&repnamerec ,sizeof(struct rp_name)) !=
						sizeof(struct rp_name )) {
		fprintf(stderr,"%s file read error\n",filename ) ;
		fprintf(stderr,"Module : rp.c . Func :Rd_formrec\n") ;
		reterr(READERR) ;
	}

	/* validate the given format number */

	if(formrec > repnamerec.formrecs || formrec < 1){
		fprintf(stderr,
		"Format# out of bound ! format#: %d, max form: %d\n",
		formrec,repnamerec.formrecs) ;
		fprintf(stderr,"Repname : %s\n", repnamerec.rep_name) ;
		fprintf(stderr,"Module : rp.c . Func : Rd_formrecs()\n");
		reterr(INTERR) ;
	}
 
	strcpy(filename + strlen(proj),FRMFILE) ;
	if((fdfr = open(filename,RDMODE)) == -1){
		fprintf(stderr,"%s file open-read error\n",filename) ;
		fprintf(stderr,"Module : rp.c. Func : Rd_formrec\n") ;
		reterr(OPENERR) ;
	}
	if (lseek(fdfr,
	   (long)(((long)(repnamerec.formoff[formrec - 1]))*FRMBUFSIZE),0) < 0){
		fprintf(stderr,"Format file seek error\n") ;
		fprintf(stderr,"Module : rp.c. Func : Rd_formrec\n") ;
		reterr(SEEKERR) ;
	}
	if (read(fdfr,frmbuf,FRMBUFSIZE) < FRMBUFSIZE){
		fprintf(stderr,"Format file read error\n") ;
		fprintf(stderr,"Module : rp.c. Func : Rd_formrec\n") ;
		reterr(READERR) ;
	}
	close(fdfr) ;

	return(NOERROR) ;
}


/** Open structure and format definition file	*/
static int
Open_strfile(proj,outopt,discfile)		
char *proj ;	/* project code	*/
int outopt ;	/* output media option - TERM/PRTR/DISC	*/
char *discfile ;/* In case of discfile output, filename	*/
{
#ifndef	SPOOLER		/* Not SPOOLER */
	FILE	*fopen(), *fppr ;	/* file ptr, to printer device name
					   file */
	char	printer[FILE_NAME_LEN] ;
#endif
	char	filename[FILE_NAME_LEN] ;
	
	strcpy(filename,proj) ;
	strcat(filename,STRFILE) ;	/* structure def. file	*/
	if((fds = open(filename,RDMODE)) == -1){
		fprintf(stderr,"%s file open-read error\n",filename) ;
		fprintf(stderr,"Module : rp.c. Func : inoutopen\n") ;
		reterr(OPENERR) ;
	}

	prtout = termout = discout = 0 ;
	switch(outopt) {
		case 0 : termout = 1 ; break ;

		case 1 : discout = 1 ; break ;

		case 2 : prtout = 1 ; break ;

		case 3 : termout = discout = 1 ; break ;

		case 4 : termout = prtout = 1 ; break ;

		case 5 : discout = prtout = 1 ; break ;

		case 6 : termout = discout = prtout = 1 ; break ;

		case 7 : discout = 2 ; break ;

		default : termout = 1 ; break ;
	}

	if(prtout) {		/* printer output opted	*/
#ifdef	SPOOLER
		strcpy(tempname, RPTEMP) ;
		printer = mktemp(tempname) ;	/* Get The Temporay File name */

		if((fdp = creat(printer,TXT_CRMODE)) < 0){
#else
		if((fppr  = fopen(PRINTER,"r")) == NULL){
			fprintf(stderr,"%s file open read error\n",PRINTER) ;
			fprintf(stderr,"Module : rp.c . Func : Open_str()\n") ;
			reterr(OPENERR) ;
		}
		fscanf(fppr,"%s",printer) ;	/* printer device name */
		fclose(fppr) ;

		if((fdp = open(printer,TXT_WRMODE)) < 0){
#endif
	
			fprintf(stderr,"%s file open-read error\n",PRINTER) ;
			fprintf(stderr,"Module : rp.c. Func : inoutopen\n") ;
		 	reterr(OPENERR) ;
		 }
	}

	if(discout) {
		if(strlen(discfile) == 0){
			fprintf(stderr,"Disc file name not specified\n") ;
			fprintf(stderr,"Module : rp.c. Func : inoutopen\n") ;
			reterr(INTERR) ;
		}
		if(discout == 2)
			if((fdd = open(discfile,TXT_WRAPMODE )) < 0)
				discout = 1;

		if(discout == 1){
			if((fdd = creat(discfile,TXT_CRMODE)) < 0){
				fprintf(stderr,"%s file creat  error\n",discfile) ;
				fprintf(stderr,"Module : rp.c. Func : inoutopen\n") ;
				reterr(CREATERR) ;
			}
			close(fdd) ;

			if((fdd = open(discfile,TXT_WRMODE)) < 0){
				fprintf(stderr,"%s file open-write error\n",discfile) ;
				fprintf(stderr,"Module : rp.c. Func : inoutopen\n") ;
				reterr(OPENERR) ;
			}
		}
	}

	return(NOERROR) ;
}



static int
decide_dimension()
{
	short totsiz, gap ;
	short i ;

	maxlines = f_hd->page_lines ;
	maxcols = f_hd->page_columns ;

	for(i= 0, totsiz= 0 ; i<f_hd->tot_flds ; i++)
		totsiz+= size[i] ;

	gap = (f_hd->page_columns-totsiz)-1 ; /*** -1 for \n***/

	if (gap < (f_hd->tot_flds-1)) {	/* 1 Blank between Columns */
		colspace = 0; 
/**
		fprintf(stderr,
		    "Line size(%d) exceeds the Max. #of(%d) columns allowed\n",
			(totsiz+gap+1), f_hd->page_columns ) ;
**/
		printf(
		    "Line size(%d) exceeds the Max. #of(%d) columns allowed\n",
			(totsiz+gap+1), f_hd->page_columns ) ;
		printf("Error in decide_dimension\n");
		getchar();
		reterr(ERROR) ;
	}	

	if(f_hd->tot_flds > 1)
		colspace = gap / (f_hd->tot_flds - 1);
	else
		colspace = 0; 

	return(NOERROR) ;
}

static int
ptr_get(fld,dest)
FIELDNO fld;
char *dest ;
{
	short	i ;
	FLDINFO	*ptr ;
	char	*srce ;

	ptr = (FLDINFO *)(Stru_buf[fld.inrecno]+sizeof(INHDR)) ;
	ptr += fld.infieldno ;
	srce = userec[fld.inrecno]+ptr->posf ;

	for(i=0 ; i<(ptr->lenf) && *srce!= '\0' ; srce++, dest++, i++)
		*dest= *srce ;
	*dest= '\0' ;

	/*****
	fprintf(stderr,"PTR_GET :\n") ;
	fprintf(stderr,"fldname : %s,typf :%c, posf : %d, lenf : %d\n",
	ptr->fldname,ptr->typf,ptr->posf,ptr->lenf) ;
	fprintf(stderr,"VALUE : %s\n",dest) ;
	******/
}


static int
allocate_lines(nt)
short	nt ;
{
	short lines ;
	char *ptr ;
	int   i ;

	top_line = malloc( maxcols ) ;
	lines= f_hd->mhdnos+f_hd->tot_keys + f_hd->tot_pghdr +nt+5 ;
	if ((hdrptr = ptr= malloc((unsigned)(lines*maxcols)))== NULL){
		fprintf(stderr,"Error in allocate_lines. Error no. = 107");
		reterr(107) ;
	} 
	for(i= 0 ; i<f_hd->mhdnos ; i++, ptr+=maxcols)
		hline[i]= ptr ;
	gline= ptr ; ptr+=maxcols ;
	uline= ptr ; ptr+=maxcols ;
	for(i=0 ; i<nt ; i++, ptr+=maxcols)
		tit_line[i]= ptr ;
	rline= ptr ; ptr+=maxcols ;
	cline= ptr ; ptr+=maxcols ;
	for(i= 0 ; i<f_hd->tot_keys + f_hd->tot_pghdr ; i++, ptr+=maxcols)
		subline[i]= ptr ;

	return(NOERROR) ;
}
#ifdef UNUSED
static int
slip_wise(rec)
char **rec ; {
	short i, noff ;
	char sst[MXFLDSZ], typ;
	NUMRET val ;
	userec = rec ;
	for(i=0;i<f_hd->tot_flds;i++) {
		i_space(cline);
		noff = 0;
		if((fld_hd+i)->fld_type != CHAR_FLD) {
			typ= get_fld((fld_hd+i)->in_takeno.fieldref,&val) ;
			CHKERROR(edit(&val,VARADDR + (fld_hd+i)->edit_off,sst,typ));
		}
		else
			ptr_get((fld_hd+i)->in_takeno.fieldref,sst);
		noff = cp(cline,(noff+2),VARADDR+(fld_hd+i)->title_off);
		noff = cp(cline,(noff+2)," :");
		noff = cp(cline,(noff+2),sst);
		strcpy(sst,"");
		noff+=2;
		putline(cline);
	}
	*(cline+maxcols-1) = '\n' ;	
	underline(2);
} 
#endif

/****
rmslash(str)
char *str ; {
	for(i;*str!= '\0';str++)
		if (*str== '\') *str= '\0' ;
}
*****/



static int
maxltitle(str)
char *str ;
{
	short	ml= 0, n= 0 ;

	for(; *str!='\0'; str++) {
		if (*str== '\\') {
			if (n > ml) ml= n ;
			n= 0 ;
		}
		else n++ ;
	}
	if (n > ml) ml = n ;
	return(ml) ;
}


static int
findntitles()
{
	short	i, nt, maxnt = 0 ;
	char	*ptr ;	

	for (i= 0 ; i<f_hd->tot_flds ; i++) {
		nt= 0 ;
		ptr = VARADDR + (fld_hd+i)->title_off ;
		for(;*ptr!= '\0' ; ptr++)
			if(*ptr== '\\')
				nt++ ;
		nt++ ;
		if (nt>maxnt) maxnt= nt ;
	}

	return(maxnt) ;
}



static int spp;		/* a pointer to the comptl string */



#define MAXOP 80
#define NUMBER  '1'
#define INFIELD '2'
#define OPERATOR '3'
#define STEND '0'


static int
calc_rp(spol,val)
char *spol ;
NUMRET *val ;
{
	char	typ, t1, t2 ;
	NUMRET	op1, op2 ;
	int	type;
	char	s[MAXOP] ;
	long	atol() ;
	char	lpop() ;
	FIELDNO	cfld ;

	spp= 0 ;
#ifdef	DEBUG
	fprintf(stderr,"compstring : %s\n",spol) ;
#endif
	while((type = lgetop(s,spol)) != STEND)
				/* if completed, getop returns null value */
	{
		if(type == -1) return(ERROR) ;
		switch (type) {
		case NUMBER :	/* for long numerals */
			/**
			val->vlong= atol(s) ;
			lpush(val,LONG);
			**/
			sscanf(s, "%lf", &val->vdouble );
			lpush(val,DOUBLE);
#ifdef DEBUG
			fprintf(stderr,"number %ld %s\n",val->vlong,s) ;
#endif
			break;
		case INFIELD:	/* for input field nos */
			sscanf(s,"%hd %hd",&cfld.inrecno,&cfld.infieldno) ;

#ifdef DEBUG
			fprintf(stderr,"inrecno: %d, infield: %d\n",cfld.inrecno,cfld.infieldno) ;
#endif
			typ= get_fld(cfld,val); /* the value that the no refers to is fetched from user record*/
#ifdef DEBUG
			fprintf(stderr,"infield %d %c\n",cfld.infieldno,typ) ;
#endif
			lpush(val,typ) ;
			break ;
		case OPERATOR :
			t1= lpop(&op1) ; t2= lpop(&op2) ;
			if (t1!= t2) {
				if (t1== LONG)
					op1.vdouble= (double)op1.vlong ;
				else op2.vdouble= (double)op2.vlong ;
				t1= t2= DOUBLE ;
			}
#ifdef DEBUG
			fprintf(stderr,"operator %c\n",s[0]) ;
#endif
			switch (s[0]) {
			case '+':
				if (t1== LONG)
					val->vlong= op1.vlong+op2.vlong ;
				else val->vdouble= op1.vdouble+op2.vdouble ;
				break ;
			case '-':
				if (t1==LONG)
					val->vlong= op2.vlong-op1.vlong ;
				else val->vdouble= op2.vdouble-op1.vdouble ;
				break ;
			case '*':
				if (t1== LONG)
					val->vlong= op1.vlong*op2.vlong ;
				else val->vdouble= op1.vdouble*op2.vdouble ;
				break ;
			case '/':
				if (t1== LONG) {
/** Changed By J.Prescott Mar 13/92 so that a divide by zero will not give
    an error.
					if (op1.vlong== (long)0) return(ERROR) ;
**/
					if (op1.vlong== (long)0) {
						val->vlong = 0; 
						break;
					}
					val->vlong= op2.vlong/op1.vlong ;
				}
				else {
/** Changed By J.Prescott Mar 13/92 so that a divide by zero will not give
    an error.
					if (op1.vdouble== (double)0.0) return(ERROR) ;
**/
					if (op1.vdouble== (double)0) {
						val->vdouble = 0; 
						break;
					}
					val->vdouble= op2.vdouble/op1.vdouble ;
				}
			}
			lpush(val,t1) ;
			break ;
		}
	}
	return(lpop(val)) ;
}

static 	short sp ;
#define MAXVAL 15
static struct xxx {
	char styp ;
	NUMRET selm ;
} stack[MAXVAL] ;
static int
lpush(val,typ) /*** makes all values long or double ****/
NUMRET *val ;
char typ ;
{
	switch(typ) {
		case SHORT : stack[sp].selm.vlong= (long)val->vshort ; break ;
		case INT : stack[sp].selm.vlong= (long)val->vint ; break ;
		case LONG : stack[sp].selm.vlong= val->vlong ; break ;
		case FLOAT : stack[sp].selm.vdouble= (double)val->vfloat ; break ;
		case DOUBLE : stack[sp].selm.vdouble= val->vdouble ; break ;
	}
	if ( sp >= MAXVAL ) 
		{
		fprintf(stderr, " Stack Overflow Error in lpush() ");
		reterr(INTERR) ;
		}
	if (typ== SHORT || typ== INT || typ== LONG)
		stack[sp++].styp= LONG ;
	else stack[sp++].styp= DOUBLE ;

	return(0) ;
}

static char lpop(val)
NUMRET *val ; {
	
	if ( sp <= 0 ) 
		{
		fprintf(stderr, "Stack Underflow in lpop() ");
		reterr(INTERR) ;
		}
	sp-- ;
	switch(stack[sp].styp) {
		case LONG : val->vlong= stack[sp].selm.vlong ; break ;
		case DOUBLE : val->vdouble= stack[sp].selm.vdouble ; break ;
	}
	return(stack[sp].styp) ;
}


static int
lgetop(s,spol)
char *s ;
char *spol ;
{
	short i ;
	char c ;

	if((c= spol[spp++]) == '\0')	/* string is completely scanned */
		return(STEND);
	if(c == 'i')
	{
		spp++;				/* skip '[' */
		for(i=0;(c=spol[spp++]) >= '0' && c <= '9' && c != ']' || c == ' ';i++)
			s[i]=c;
						/* ignore ']' */
		s[i]='\0'; spp++ ;
		return(INFIELD);
	}
	s[0]= c ;
	if(c < '0' || c > '9') {
		spp++ ;
		return(OPERATOR);			/* operator */
	}
	for(i=1;(c=spol[spp++]) >= '0' && c <= '9'; i++)
		s[i]=c;

	s[i]='\0';
	return(NUMBER );

}

/*
*	Return type of COMP_FLD for a given Riverse Polish formula
*
*	Type will be either LONG or DOUBLE based on Input Field types.
*	A numeric constant in formula is always considered as a DOUBLE.
*/

static int
comp_typ(spol)
char	*spol ;
{
	short	i, rec_no, fld_no ;
	char	fld[20] ;
	FLDINFO	*ptr ;

	for( ; *spol ; spol++ ) {
		if( *spol == 'i') {		/* Field */
			spol++;			/* skip 'i' */
			spol++;			/* skip '[' */
			for(i = 0; *spol != ']' ; spol++, i++)
				fld[i] = *spol ;

			fld[i] = '\0';

			/* Get the Field information */
			sscanf(fld, "%hd %hd", &rec_no, &fld_no) ;
			ptr = (FLDINFO *)(Stru_buf[rec_no] + sizeof(INHDR)) ;
			ptr += fld_no ;

			/* If its double no more checking is required */
			if(ptr->typf == DOUBLE) return(DOUBLE) ;

			continue ;	/* Current char ']' */
		}

		if( *spol >= '0' && *spol <= '9') 	/* NUMBER */
			/* When number is given in expression return DOUBLE */
			return(DOUBLE) ;
	}

	return(LONG) ;	/* Default LONG */
}



#define NEG -1
#define ZERO -2
#define SUP 1
#define FLT 2
#define STR 3
#define SUPPRESS { k=j; flag=SUP;   }
#define STAR     { k=j; flag=STR;   }
#define FLOOT    { k=j; flag=FLT;   }
#define edited   { strcpy(dest,ma); return(0);  }
#define mchar    ma[j]=='_'||ma[j]=='*'||ma[j]=='0'||ma[j]=='$'
#define punct    ma[j]==','||ma[j]=='.'||ma[j]==' '||ma[j]=='-'||ma[j]=='/'
static	short	i,j,k,flag,nflag,err;
static	char	ma[25] ;

static	int
edit(val,mask,dest,typ)
NUMRET *val ;
char *mask, *dest, typ;
{
	char	format[10], no[25] ;

	i=j=k=flag=nflag=err=0;

	switch (typ) {
	case SHORT :
		if(val->vshort== 0) nflag= ZERO ;
		if(val->vshort<0) {
			nflag= NEG ; val->vshort *= -1 ; }
		sprintf(no,"%d",val->vshort) ;
		break ;
	case INT :
		if(val->vint== 0) nflag= ZERO ;
		if(val->vint<0){ nflag= NEG ; val->vint *= -1 ; }
		sprintf(no,"%d",val->vint) ;
		break ;
	case LONG :
		if(val->vlong== 0) nflag= ZERO ;
		if(val->vlong<0){nflag= NEG ; val->vlong  *= -1 ; }
		sprintf(no,"%ld",val->vlong) ;
		break ;
	case FLOAT :
		if(val->vfloat== 0) nflag= ZERO ;
		if(val->vfloat<0){nflag= NEG ; val->vfloat *= -1 ; }
		form_format(mask,format, typ);
		sprintf(no,format,val->vfloat) ;
		rm_dot( no ) ;
		break ;
	case DOUBLE :
		if(val->vdouble== 0) nflag= ZERO ;
		if(val->vdouble<0){nflag= NEG ; val->vdouble *= -1 ; }
		form_format(mask,format, typ);
		sprintf(no,format,val->vdouble) ;
		rm_dot( no ) ;
		break ;
	default :
		fprintf(stderr, "Invalid Type in edit() ");
		reterr(INTERR) ;

	}
	i= strlen(no) ;         /* i=length of digits string */
	strcpy(ma,mask);

	for(j=0;ma[j]!='\0';j++){
		if(j!=0 && ma[j]=='$')  FLOOT
		if(ma[j]=='0') SUPPRESS
		if(ma[j]=='*') STAR
	}       /* sets the appropriate flag */

	if(ma[j-1]=='R'){      /* CR Case */
		j--;
		if(nflag!=NEG)
			ma[j]=ma[j-1]=' ';
		j--;
	}
	else
	if(ma[j-1]=='-'){     /* Case of -Sign */
		j--;
		if(nflag!=NEG)
			ma[j]=' ';
	}

	if(nflag!=ZERO){
		err=0;
		for(j--;j>=0;j--){
			if (punct) ;
			else
				if (mchar) {
					ma[j]=no[--i];
					if(i<=0) goto final;
					else ;
				}

		}		   /* for(j--;j>=0;j--) */
	}	   /* if(nflag!=ZERO) */

	err=0;
	for(j--;j>=0; j--){
		if(ma[j]=='_') { ma[j]='0';  continue;  }
		if(ma[j]=='0')  {
			ma[j]=' ';
			suppress(k);
			edited
		}
		if(ma[j]=='*')  {
			asterisk(k);
			edited
		}
		if(ma[j]=='$')  {
			last();
			edited
		}
	}
final:
	if(flag==SUP)  {
		suppress(k);
		edited
	}
	if(flag==STR)  {
		asterisk(k);
		edited
	}
	if(flag==FLT)  {
		flot(k);
		edited
	}
	err=0;
	for(j--;j>=0;j--){
		if(ma[j]=='$')  edited
		if(punct) continue;
		if(ma[j]=='_')  {
			ma[j]='0';
			continue;
		}
		else err=1;
	}
	edited
}


suppress(t)
int t;
{
	for(j--;j>=0;j--){
		if(ma[j]=='$') return;
		if(ma[j]=='.' || ma[j]==',') {
			if(j>t) continue;
			else
				if(j<t) {
					ma[j]=' ';
					continue;
				}
		}
		if(ma[j]=='_' && j>t)  {
			ma[j]='0';
			continue;
		}
		ma[j]=' ';
	}
}

asterisk(t)
int t;
{
	for(j--;j>=0;j--){
		if(j==0)   {   ma[j] = '*';  return;  }
		if(ma[j]=='$') return;
		if(ma[j]=='.'||ma[j]==','){
			if(j>t) continue;
			else
				if(j<t) {
					ma[j]='*';
					continue;
				}
		}
		else
			if(ma[j]=='_' && j>t)  {
				ma[j]='0';
				continue;
			}
		ma[j]='*';
	}
}

flot(t)
int t;
{
	for(j--;j>=0;j--){
		if(j==0)  {
			ma[j] = '$';
			return;
		}
		if((ma[j]=='.'||ma[j]==',') && j>t) continue;
		if(ma[j]=='_' && j>t)  {
			ma[j]='0';
			continue;
		}
		ma[j] = '$';
		last();
	}
}

last()
{
	for(j--;j>=0;j--)
		ma[j]=' ';
}


form_format( msk,form, type )
char	*msk,*form;
int	type ;
{
int	i, j,k ;

strcpy(form , "%.2");
if ( type == DOUBLE ) strcat(form, "lf") ;
else strcat(form, "f") ;

for(i=0, j = 0, k = 0; i < strlen(msk) ; i++ ) {
	if ( *(msk+i) == '.' ){
		k = 1;
		continue ;
	}
	if(k) j++ ;
	}
if ( j < 0 || j > 9 ) return(0) ;
/* if last char is sign */
if(msk[i-1] == '-' && j > 0) j--;
form[2] = j + '0' ;
return(0);
}

rm_dot( no )
char	*no ;
{
int	i, j ;

for(i=0, j = 0; i < strlen(no) ; i++ ) {
	if ( *(no+i) == '.' ) continue ;
	*(no+j) = *(no+i) ;
	j++ ;
	}
*(no+j) = '\0' ;
}


 int
rpAddSubtotal (recno, fldno, fld_name, fld_text, ed_mask, break_type)

int	recno ,		/* record sequence number in logical record (1-5) */
	fldno ;		/* field number in physical record */
char	*fld_name ,	/* Name of field .. if recno is not 0 then ignored */
	*fld_text ,	/* text to be prited before subtotal */
	*ed_mask ;	/* edit mask for subtotal field */
int	break_type ;	/* 1 - Page Break , 2 - Subtotal without page break */

{

static short	last_offset ;
int	i ;
FLDINFO	*ptr ;


/***	Check if enough room for new subtotal field ***/

	if ( f_hd->tot_keys+f_hd->tot_pghdr  >= MAX_KEYFLDS ) {
			fprintf(stderr, " Too Many Subtotal fields\n");
			reterr(INTERR) ;
			}
 
/***	Check validity of input parameters ****/

	if (recno == 0 ){
		if ( !fld_name || strlen(fld_name) == 0 ){
			fprintf(stderr, " recno and fld_name absent \n");
			reterr(INTERR) ;
			}
		else 
			if ( srch_fldnm(fld_name, &recno, &fldno) == 0 ) 
						reterr(INTERR);
		}

			
	if ( break_type < 1 || break_type > 2 ) reterr(INTERR) ;
/*** 	Update tot_hd fields  ***/

	if ( break_type == 1 ) 
		{
		/*** Make space for new header ***/
		
		for ( i = f_hd->tot_keys+f_hd->tot_pghdr ; 
					i > f_hd->tot_pghdr ; i-- ) 
			{
			f_hd->keyno[i].inrecno = f_hd->keyno[i-1].inrecno ;
			f_hd->keyno[i].infieldno = f_hd->keyno[i-1].infieldno;
			f_hd->keyoff[i] = f_hd->keyoff[i-1] ;
			f_hd->subserno[i] = f_hd->subserno[i-1] ;
			f_hd->msg_textoff[i] = f_hd->msg_textoff[i-1] ;
			f_hd->keyeditoff[i] = f_hd->keyeditoff[i-1] ;
			}
		}
	else 
		i = f_hd->tot_keys+f_hd->tot_pghdr ;
	
	/*** Copy the new information in the created slot ***/

	if ( last_offset == 0 )			/* First time */
		last_offset= (fld_hd+f_hd->tot_flds - 1)->title_off  
		+ strlen( VARADDR + (fld_hd+f_hd->tot_flds-1)->title_off) ;

#ifdef	DEBUG
	fprintf(stderr, "last offset :%d\n", last_offset) ;
#endif

	f_hd->keyno[i].inrecno = recno - 1 ;
	f_hd->keyno[i].infieldno = fldno ;
	f_hd->msg_textoff[i] = last_offset ;

	strcpy(VARADDR+last_offset, fld_text) ;
	last_offset += strlen(fld_text) + 1 ;

	f_hd->keyeditoff[i] = last_offset ;

	strcpy(VARADDR+last_offset, ed_mask ) ; 
	last_offset += strlen(ed_mask) + 1 ;

	if ( break_type == 1 ) 
				f_hd->tot_pghdr++ ;
	else
				f_hd->tot_keys++ ;

	return(0) ;
}
		

 int
srch_fldnm(fldnm, recno, fldno)
char	*fldnm ;
int	*recno ,
	*fldno ;
{

int ok ;
int	i ;
INHDR	*hd_ptr ;
FLDINFO	*ptr ;


	for(i=0 /* , *recno= f_hd->Inrecno[i] */ ;i <f_hd->Numrecs; i++ ) {
		hd_ptr = (INHDR *)(Stru_buf[f_hd->Inrecno[i]]) ;
		ptr = (FLDINFO *)(hd_ptr + 1 ) ;
		*recno = i+1 ;

		for(*fldno=0; *fldno < hd_ptr->stflds; (*fldno)++, ptr++) 
			if ( strncmp(fldnm, ptr->fldname, L_NAME) == 0 ) 
						return(1) ;
	}
	
	return(0) ;	/* Return Not found */
 }

 int
rpChangetitle(line_no, new_title) 
int	line_no ;
char	*new_title ;
{
	if ( line_no >  f_hd->mhdnos || line_no < 1 ) {
		fprintf(stderr, "Line number Improper \n");
		reterr(INTERR) ;
		}

	i_space(hline[line_no-1]);
	centre_jstfy(hline[line_no-1],new_title, maxcols-1);
	*(hline[line_no-1]+(maxcols-1)) = '\n' ;

	return(0);
}
 
/****  Set summary Option on .. rpline will not report detail lines ****/

 int
rpSummaryOn()
{
summary = 1 ;
}

/****  Set summary Option off .. rpline will report all lines ****/

 int
rpSummaryOff()
{
summary = 0 ;
}

/****	Get The column position for a particular column 	****/

 int
rpGetColPos(n)
int	n  ;
{
int	i ;
int	pos ;

if ( n > f_hd->tot_flds ) return(-1) ;

for(i=1, pos=0; i<n; i++) 
	pos += (size[i-1] + colspace) ;

return(pos) ;
}
	
int
rpMkline(n, strg)
int	n ;		/* Cloumn# 1 -n , as per format */
char	*strg ;		/* Value to be placed in column */
{
int	noff ;
char	ptr[MXFLDSZ], temp[MXFLDSZ] ;
int	i, j ;
	
	if ( (noff = rpGetColPos(n)) < 0 ) reterr(INTERR) ;

	n-- ;

	if ( (fld_hd+n)->fld_type != CHAR_FLD ) {
		i = 0 ;
		if ( *strg == '-' ) i++ ;
		
		for(j=0; *(strg+i) ; i++,j++)
			*(ptr+j) = *(strg+i) ;

		if ( *strg == '-' ) *(ptr+j++) = '-' ;
		else	*(ptr+j++) = ' ' ;
		
		*(ptr+j) = '\0' ;
		right_jstfy(temp, ptr, size[n] );
		}
	else 
		left_jstfy(temp,strg,size[n]);

	cp(cline,noff,temp);

	return(0) ;
}

 int
rpPutline()
{
	if(linecount >= maxlines) {
		if(print_header() < 0) return(-1) ;
		repflag = 1 ;	/* Force Printing of Repeat supress fields */
	}

	cline[maxcols-1] ='\n';
	putline(cline);
	i_space(cline);
	put_linespace() ;
	return(NOERROR) ;
}


 void
rpAddSubtitle(line)	/* The line is suppose to be allocated by user */
char	*line ;
{
sub_title = line ;
}
 
 int
rpPagesize(size)	/* Sets the page size to new size .. returns new size */
int	size ;
{
maxlines = size ;
return(size) ;
}

int
rpSetCopies( no_of_copies )	/* Set the number of copies. Used by lpr */
int	no_of_copies;
{
	if(no_of_copies < 1) return( -1 ) ;

	numberofcopies = no_of_copies;

	return(0);
}



rpclose_mesg(mesg)
char *mesg;
{
	char	strg[MXFLDSZ], tmp[MXFLDSZ], typ ;
	short	i, noff ;
	char	glinedisplay ;	/* to display grand total line ? */
	char	tempchar[5] ;

	if(pageno) {

		glinedisplay = 0 ;

		/* If atleast one line printed and sub total fields exists
		   then print sub totals */
		if((f_hd->tot_keys + f_hd->tot_pghdr) > 0) {
			subtotals(0) ;
		}

		i_space(gline);
		noff = -colspace ;	/* To strat with 1st column */
	
		for(i=0; i<f_hd->tot_flds; i++) {
			if(!(fld_hd+i)->fld_totflag) { /* not to be totalled */
				noff += size[i] + colspace ;
				continue ;
			}
	
			glinedisplay = 1;
			if((fld_hd+i)->fld_class == COMP_FLD)
				typ =  (fld_hd+i)->fld_type;
			else
				typ= getyp((fld_hd+i)->in_takeno.fieldref) ;
			CHKERROR(edit(&g_tot[i],VARADDR+(fld_hd+i)->edit_off,tmp,typ));
	
			/* centre justify the numeric fields to allign with col
			   hdg. This takes care alignment problem, if Edit
			   mask size is less than the size allocated to filed */
	
			centre_jstfy(strg,tmp,size[i]);
	
			noff = cp(gline,noff+colspace,strg);
			strcpy(tmp,"");
			strcpy(strg,"");
		}
	
		/* Print headings, if the page is full */
		if(linecount > (maxlines-1))
			print_header();
	
		*(gline+maxcols-1) = '\n';
		
		if(glinedisplay)
			printgline() ;
		
		putline(gline);
		putmesg(mesg);
		underline(1);
		initzero_totals(g_tot) ;
	
		/* If not Spooler, Generate Page skip at the end of the report
		   for disk and printer outputs */
#ifndef	SPOOLER
		tempchar[0] = PAGESKIP ;
		if(discout >0)
			write(fdd,tempchar,1) ;
		if(prtout == 1)
			write(fdp,tempchar,1) ;
#endif
	}	/* If pageno */

	if(discout == 2){
		tempchar[0] = PAGESKIP;
		write(fdd,tempchar,1) ;
	}
	/* close output file	*/
	if(prtout) {
		close(fdp) ;
#ifdef	SPOOLER
		/* If any report is genetrated, Spool it */
		if(pageno) {
#ifdef	XENIX
			sprintf(tmp, "lp -s -c -n%d %s",numberofcopies,printer);
#else
#ifdef i386
			sprintf(tmp, "lp -s -o nobanner -c -n%d %s",numberofcopies,printer);
#else
			sprintf(tmp, "lpr -cp %d %s",numberofcopies,printer);
#endif
#endif
			system(tmp) ;
		}
		unlink(printer) ;
#endif
	}
	if(discout)
		close(fdd) ;

	if(termout == 1) {
#ifdef ENGLISH
		write(1, "Press RETURN to Continue  ", 26); 
#else
		write(1, "Appuyer sur RETURN pour continuer ", 34); 
#endif
		read(0,tmp,2);
	}

	free_space() ; /* free the dynamically allocated space */
	return(0) ;
}



int
putmesg(mesg)
char mesg[];
{
	short count,c;
	count=strlen(mesg);
	if(mesg==NULL)
		return(0);

	if(termout)
		write(1,mesg,count);
	if(prtout)
		write(fdp,mesg,count);
	if(discout) {
		c = write(fdd,mesg,count);

		if(c!=count) {
			fprintf(stderr,"error in writing a line");
			return(-1);
		}
	}
	linecount++ ;
	return(NOERROR) ;
}


