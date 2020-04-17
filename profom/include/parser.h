
#include <stdio.h>
#include <ctype.h>
#define	isoctal(c)  (( c >= '0' && c < '8' ) ? 1 : 0 )
#define cctype(i)   capstg_buf[i].captype
#define ccunion(i)  capstg_buf[i].capunion
#define ccstgcap(i) capstg_buf[i].capunion.strngcap
#define	FALSE       0
#define TRUE        1
#define MAXRECS     30		/* maximum capstring records */
#define CAPBUFR_SZ  150		/* size of buffer to store capstring names */
#define MAXSTRG_SZ  1024 	/* size of buffer to store TERMCAP string */
#define CAPSREC     struct capsrec
CAPSREC {
	short captype;	/* strngcap(0), boolcap(1) ,notfound(-1) */
	union {
		struct {
			char name[3];  /* name of the capability */
			char *ptr;     /* pointer to the parsed capstring */
			short delay;   /* amount of delay */
		       } strngcap;
		char name[3];  /* name of the capability */
	      } capunion;
	}; /* record layout to store the parsed capability string */
CAPSREC capstg_buf[MAXRECS];  /* records to store the parsed capstrings */
/*
char caps[CAPBUFR_SZ];	      /* buffer containing all caps to be parsed *
*/
char caps[] = "cm\0co\0cl\0li\0so\0se\0us\0ue\0ce\0";	      
char tmpbuf[100];	      /* temp buffer */
char tcap_strng[MAXSTRG_SZ];  /* buffer to store the initial TERMCAP string */
char pars_strng[MAXSTRG_SZ];  /* buffer to store the parsed TERMCAP string */
char *capsptr = caps;
char *tcapptr = tcap_strng;
char *parsptr = pars_strng;
/******	  set of flags used while parsing the 'termcap' string	******/
short   firsttime = TRUE,   /* to indicate firsttime parsing of 'cm' string */
	delay     = FALSE,  /* to indicate the presence of delay in a string */
	addition  = FALSE,  /* to indicate '%+' string in the 'cm' string */
	greater   = FALSE,  /* to indicate '%>' string in the 'cm' string */
	reverse   = FALSE,  /* to indicate '%r' string in the 'cm' string */
	increment = FALSE,  /* to indicate '%i' string in the 'cm' string */
	ex_or     = FALSE,  /* to indicate '%n' string in the 'cm' string */
	boolcap   = FALSE,  /* given cap is present with default meaning */
	addarg1   = FALSE,  /* presence of '%+' for firsttime in 'cm'string */
	addarg2   = FALSE,  /* presence of '%+' for secndtime in 'cm'string */
	grtarg1   = FALSE,  /* presence of '%>' for firsttime in 'cm'string */
	grtarg2   = FALSE,  /* presence of '%>' for secndtime in 'cm'string */
	incarg1   = FALSE,  /* presence of '%i' for firsttime in 'cm'string */ 
	incarg2   = FALSE;  /* presence of '%i' for secndtime in 'cm'string */
short 	arg1,	    /* firsttime argument of '%+' string of 'cm' string */
	arg2,       /* secndtime argument of '%+' string of 'cm' string */
	addval1,    /* firsttime 'y'argument of '%>xy' string of 'cm' string */
	addval2,    /* secndtime 'y'argument of '%>xy' string of 'cm' string */
	chckval1,   /* firsttime 'x'argument of '%>xy' string of 'cm' string */
	chckval2,   /* secndtime 'x'argument of '%>xy' string of 'cm' string */
	delay_amt,  /* amount of delay present w.r.t a given capability */
	ncaps;      /* total number of cpabilities to be parsed */

