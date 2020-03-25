/******************************************************************************
		Sourcename    : rcptlist.c
		System        : Budgetary Financial system.
		Module        : Accounts Receivable reports
		Created on    : 89-13-23
		Created  By   : Jonathan Prescott
		Cobol sources : 
*******************************************************************************
About this file:
	This routine generates one of the following :
		1. List of Accounts Receivable Transactions 
			using REPORT GENERATOR
	Calling file:	


Modifications:

Programmer	Date		Description
~~~~~~~~~~	YY/MM/DD	~~~~~~~~~~~
C.Leadbeater	90/11/29	Added ranges for customer code to specify
				what customers to include in report.

J McLean        92/07/27	Changed customer code from 6 to 10 characters.
******************************************************************************/
#define	EXIT	12		/* as defined in apreputl.c */

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	PROJNAME	"rcptrep"
#define	LOG_REC		1
#define EXIT		12

#ifdef ENGLISH
#define PRINTER 	'P'
#define DISPLAY		'D'
#define FILE_IO		'F'
#else
#define PRINTER		'I'
#define DISPLAY		'A'
#define FILE_IO		'D'
#endif

static Rcpt_hdr	rcpt_hdr;
static Rcpt_item rcpt_item;
static Pa_rec	pa_rec;

extern char e_mesg[80] ;

static long	transdt1;
static long	transdt2;
static short	fund1;
static short	fund2;
/**
static long	invcno1;
static long	invcno2;
**/
static short    nbrcopies;
static char	custcd1[11];   /* used to specify customer code range (CL) */
static char 	custcd2[11]; 	

Rcpt_list(mode) 
int mode;		/* 0 if interactive, 1 if day end process */
{
	char	chardate[11] ;
	char 	*arayptr[5] ; 	/** Declarations for Report writer usage **/
	char 	projname[50] ;
	char 	program[11] ;
	int 	outcntl ;
	char 	discfile[20] ;
	int	err;
	int	itemcnt;	/* number of items printed for receipt */

	if(mode == 0) {
#ifdef ENGLISH
		STRCPY(e_mesg,"P");
#else
		STRCPY(e_mesg,"I");
#endif
		err = GetOutputon(e_mesg);
		if(err < 0 || err == EXIT) return(err);

		switch( *e_mesg){
			case DISPLAY:	/* display on terminal */
				outcntl = 0;
				break;
			case FILE_IO:	/* print to a file */
				outcntl = 1;
				break;
			case PRINTER:	/* print on printer */
				outcntl = 2;
				break;
			default:
				outcntl = 2;
				break;
		}
		if(outcntl == 1) {
			STRCPY(e_mesg,"rcptlist.dat");
			if((err = GetFilename(e_mesg))< 0) return(err);
			STRCPY(discfile,e_mesg);
		}
		else	discfile[0] = '\0';

		nbrcopies = 1;
		if(outcntl == 2) {
			if((err = GetNbrCopies(&nbrcopies))< 0)
				return(err);
		}
	}
	else outcntl = 2;

	mkdate(get_date(),chardate);
	STRCPY(projname,FMT_PATH) ;
	strcat(projname,PROJNAME) ;
	STRCPY(program, PROG_NAME );

	/* set default ranges */
	transdt1 = transdt2 = get_date();
	fund1 = 1;
	fund2 = 999;
/****
	invcno1 = 0;
	invcno2 = 99999999;
****/
	if(mode == 0) {		/* interactive report */
		/* get transaction date range */
		err = GetTransDateRange(&transdt1, &transdt2);
		if(err < 0)	return(err);

		/* get fund number range */
		err = GetFundRange(&fund1, &fund2);
		if(err < 0)	return(err);
	
		/* get range for customer code (CL) */
		STRCPY(custcd1,"         1");
		STRCPY(custcd2,"ZZZZZZZZZZ");
		err = GetCNbrRange( custcd1, custcd2 );
		if(err < 0) return(err);
		
		if ( (err=Confirm())<= 0) 
			return(err);
	}

	err = get_param(&pa_rec, BROWSE, 1, e_mesg) ;
	if(err < 1) {
		close_dbh() ;
		return(DBH_ERR) ;
	}


	err = rpopen(projname,LOG_REC,1,outcntl,discfile,program,
			chardate);
							  	
	if ( err < 0 ){
		sprintf(e_mesg, "Rpopen code :%d\n", err ) ;
		close_dbh() ;
		return(REPORT_ERR);
	}

	if(outcntl == 2) {
		rpSetCopies( (int)nbrcopies );
	}
	/* Change first title line to Company/School district name */
	if((err = rpChangetitle(1, pa_rec.pa_co_name))<0) 
		return(REPORT_ERR);

	/* For Terminals set pagesize to 23 lines */
	if(outcntl == 0)
		rpPagesize(23);

	arayptr[0] = (char *)&rcpt_hdr ;
	arayptr[1] = (char *)&rcpt_item ;
	arayptr[2] = NULL ;

	rcpt_hdr.rhdr_rcptdate = transdt1;
	rcpt_hdr.rhdr_fund = fund1;
/**
	rcpt_hdr.rhdr_invnumb = invcno1;
**/
	rcpt_hdr.rhdr_refno = 0;	

	flg_reset(RCPTHDR);
	for(;;) {
		err = get_n_rcpthdr(&rcpt_hdr,BROWSE,1,FORWARD,e_mesg);
		if(err < 0) { 
			break;
		}

		if(rcpt_hdr.rhdr_rcptdate > transdt2)
			break;

		if(rcpt_hdr.rhdr_fund < fund1 || rcpt_hdr.rhdr_fund > fund2) {
			if(rcpt_hdr.rhdr_fund > fund2) {
				rcpt_hdr.rhdr_fund++;
			}
			rcpt_hdr.rhdr_fund = fund1;
/**
			rcpt_hdr.rhdr_invnumb = invcno1;
**/
			flg_reset(RCPTHDR);
			continue;
		}
		

/**
		if(rcpt_hdr.rhdr_invnumb < invcno1 ||
			rcpt_hdr.rhdr_invnumb > invcno2) {
			if(rcpt_hdr.rhdr_invnumb > invcno2) {
				rcpt_hdr.rhdr_fund++;
			}
			rcpt_hdr.rhdr_invnumb = invcno1;
			rcpt_hdr.rhdr_refno = 0;
			flg_reset(ARSHDR);
			continue;
		}
**/	
			/* check that customer code is within range (CL) */
	
		if (strcmp(rcpt_hdr.rhdr_cust, custcd1) < 0) continue;
		if (strcmp(custcd2, rcpt_hdr.rhdr_cust) < 0) continue;

		rcpt_item.ritm_refno = rcpt_hdr.rhdr_refno;
		rcpt_item.ritm_seqno = 0;
		flg_reset(RCPTITEM);

		itemcnt = 0; /* initialize items printed to zero */
		for ( ; ; ) {
			err=get_n_rcptitem(&rcpt_item,BROWSE,0,FORWARD,e_mesg);
			if(err < 0) {
				if(err == EFL) break;
				err = DBH_ERR;
				break;
			}
			if(rcpt_item.ritm_refno != rcpt_hdr.rhdr_refno) break;
			if(itemcnt != 0) {
				rcpt_hdr.rhdr_amount = 0.00;
			}
			if(rpline(arayptr) < 0) return(NOERROR) ;
			itemcnt++;	/* add 1 to items printed */
		}
		if(itemcnt == 0) {
			rcpt_item.ritm_invnumb = 0;
			rcpt_item.ritm_amount = 0.00;
			if(rpline(arayptr) < 0) return(NOERROR) ;
		}
	}
	rpclose();
	close_file(RCPTHDR);
	if(err < 0 && err != EFL) return(DBH_ERR);
	return(0);
}
