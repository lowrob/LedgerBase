/* ---------------------------------------------------------------------------
*	Source 	: reqbyccno.c 
*
*	Program to Print Requisitions by cost center no. using REPORT GENERATOR.
*

Modifications:

	YY/MM/DD	Programmer	Description
	~~~~~~~~	~~~~~~~~~~	~~~~~~~~~~~
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>
#include <repdef.h>
#include <requ.h>

#define	PROJNAME	"reqrpt1"
#define	LOG_REC		1
#define	FORMNO		3
#define EXIT		12

#ifdef ENGLISH
#define PRINTER 'P'
#define DISPLAY 'D'
#define FILE_IO	'F'
#define SPOOL	'S'
#define YES	'Y'
#else
#define PRINTER 'I'
#define DISPLAY 'A'
#define FILE_IO	'D'
#define SPOOL	'B'
#define YES	'O'
#endif

extern	int	rperror	;
extern char	e_mesg[80] ;

static Tax_cal	tax_cal;

reqbyccno()
{
Pa_rec 	 pa_rec ;
Req_hdr  req_hdr;
Req_item req_item;
Supplier supp_rec;
Sch_rec   sch_rec;

char	desc[14];
char	total_str[132];

char    chardate[11];
char	chardate1[11] ;
char	chardate2[11] ;
int	code;
char 	*arayptr[5] ; 	/** Declarations for Report writer usage **/
char 	projname[50] ;
char 	program[11] ;
int 	outcntl ;
char 	discfile[20] ;
int     keyno;
int	retval;
long	date1; 
long	date2;
short	fund1;
short	fund2;
short	costcenter1;
short	costcenter2;
char	reqstat1[2];
char	reqstat2[2];
char	itmstat1[2];
char	itmstat2[2];

short	copies ;
long	nbr_printed_req;
long	prev_reqnbr;
short	prev_ccno;

short	err;
short	gst_tax, pst_tax;
double  gst_amt, pst_amt;

	code = get_param(&pa_rec, BROWSE, 1, e_mesg) ;
	if(code < 1) {
		return(DBH_ERR) ;
	}

	keyno = 4;
#ifdef ENGLISH
	STRCPY(e_mesg, "P");
#else
	STRCPY(e_mesg, "I");
#endif
	retval = GetOutputon(e_mesg);
	if(retval < 0) return(retval);

	switch( *e_mesg){
		case DISPLAY:	/* display on Terminal */
			outcntl = 0;
			break;
		case FILE_IO:	/* print to a file */
		case SPOOL:	/* spool report */
			outcntl = 1;
			break;
		case PRINTER:
			outcntl = 2;
			break;
		default:
			outcntl = 2;
			break;
	}
	if(outcntl == 1) {
		if(e_mesg[0]==FILE_IO) {
			STRCPY(e_mesg,"reqbysupp.dat");
			if((retval = GetFilename(e_mesg)) < 0)
				return(retval);
			STRCPY(discfile, e_mesg);
		}
		else {
			sprintf(discfile,"spool%d",CC_no);
		}
	}
	else 	discfile[0] = '\0';

	copies = 1;
	if(outcntl == 2) {
		if((retval = GetNbrCopies( &copies )) < 0)
			return(retval);
	}

	fund1 = 1;
	fund2 = 999;
	retval = GetFundRange( &fund1, &fund2 );
	if(retval < 0) return(retval);
	
	if(CC_no == pa_rec.pa_distccno) {
		costcenter1 = 0;
		costcenter2 = 9999;
		retval = GetCCRange( &costcenter1, &costcenter2 );
		if(retval < 0) return(retval);
	}
	else {
		costcenter1 = CC_no;
		costcenter2 = CC_no;
	}

	reqstat1[0] = 'A';
	reqstat2[0] = 'Z';
	retval = GetReqStatRange( reqstat1, reqstat2 );
	if(retval < 0) return(retval);

	itmstat1[0] = ' ';
	itmstat2[0] = 'Z';
	retval = GetItmStatRange( itmstat1, itmstat2 );
	if(retval < 0) return(retval);
	
	date1 = date2 = get_date();
	retval = GetDateRange( &date1, &date2 );
	if(retval < 0) return(retval);

	if((retval = Confirm()) <= 0) 
		return(retval);

	mkdate(get_date(),chardate);

	STRCPY(projname,FMT_PATH) ;
	strcat(projname,PROJNAME) ;
	STRCPY(program, PROG_NAME );

	code = rpopen(projname,LOG_REC,FORMNO,outcntl,discfile,program,
			chardate);
							  	
	if ( code < 0 ){
		sprintf(e_mesg,"Rpopen code :%d\n", code ) ;
		close_dbh() ;
		return(REPORT_ERR);
	}

	if(outcntl == 2) 
		rpSetCopies( (int)copies );   /* number of copies to print */

	/* Change first title line to Company/School district name */
	if((retval = rpChangetitle(1, pa_rec.pa_co_name))<0) {
#ifdef ENGLISH
		sprintf(e_mesg,"Internal report error");
#else
		sprintf(e_mesg,"Erreur au rapport interne");
#endif
		rpclose();
		close_dbh();
		return(REPORT_ERR);
	}
	mkdate( date1, chardate1);
	mkdate( date2, chardate2);
#ifdef ENGLISH
	sprintf(e_mesg,"From %s To %s",chardate1, chardate2);
#else
	sprintf(e_mesg,"de %s a %s",chardate1, chardate2);
#endif
	if((retval = rpChangetitle(3, e_mesg))<0) {
#ifdef ENGLISH
		sprintf(e_mesg,"Internal report error");
#else
		sprintf(e_mesg,"Erreur au rapport interne");
#endif
		rpclose();
		close_dbh();
		return(REPORT_ERR);
	}	
 
	/* For Terminals set pagesize to 23 lines */ 
	if(outcntl == 0)
		rpPagesize(23);

	arayptr[0] = (char *)&req_hdr ;
        arayptr[1] = (char *)&req_item ;
	arayptr[2] = (char *)&supp_rec ;
	arayptr[3] = (char *)&sch_rec ;
        arayptr[4] = NULL ;

	/***	Read record and call report writer to output it ***/

	/* Initialize Recs. to start printing req report on req no. */
	req_hdr.funds = fund1;
	req_hdr.costcenter = costcenter1;
	req_hdr.code = 0;

	flg_reset( REQHDR );

	nbr_printed_req = 0;
	for( ; ; ) {
		code = get_n_reqhdr(&req_hdr,BROWSE,keyno,FORWARD,e_mesg);
		if( code < 0) {
			if(code == EFL)  break ;
			code = DBH_ERR;
			break ;
		}

		if(req_hdr.funds > fund1) {  /* fund range */
			break;
		}

		if(req_hdr.costcenter < costcenter1 || 
		   req_hdr.costcenter > costcenter2) {
			continue;
		}

		if (req_hdr.date < date1 ||
		    	req_hdr.date > date2) {	/* Check date range */
				continue;
		}

		if(strcmp(req_hdr.status,reqstat1) < 0 || 
		   strcmp(req_hdr.status,reqstat2) > 0) {
			continue;
		}

		if(prev_ccno != req_hdr.costcenter) {
			prev_ccno = req_hdr.costcenter;
		
			sch_rec.sc_numb = req_hdr.costcenter;
			code = get_sch(&sch_rec,BROWSE,0,e_mesg);
			if(code < 0) {
				code = DBH_ERR;
				break;
			}
		}

		req_item.code = req_hdr.code;
 		req_item.item_no = 0;
        	flg_reset( REQITEM );
		for( ; ; ) {
#ifndef ORACLE
		  code=get_n_reqitem(&req_item,BROWSE,0,FORWARD,e_mesg);
#else
		  code=get_n_reqitem(&req_item,BROWSE,0,EQUAL,e_mesg);
#endif
		  if( code < 0) {
			if(code == EFL)  break ;
			code = DBH_ERR;
			break ;
		       }
#ifndef ORACLE
		  if(req_hdr.code != req_item.code) break;
#endif

		  if(strcmp(req_item.appstat,itmstat1) < 0 || 
		     strcmp(req_item.appstat,itmstat2) > 0) {
		 	continue;
		  }
		
		  calctax(req_item.tax1,req_item.tax2,
				req_item.value,&tax_cal);

		  req_item.value = tax_cal.gros_amt;

		/* truncate description to make lenght allowed on report */
		  STRCPY(desc,req_item.desc); 
		  STRCPY(req_item.desc,desc);		  
		  if((code = rpline(arayptr)) <0) {
			if(rperror < 0)  {
#ifdef ENGLISH
				sprintf(e_mesg,"Internal report error");
#else
				sprintf(e_mesg,"Erreur au rapport interne");
#endif
				code = REPORT_ERR;
			}
			else {
				code = EXIT ;
				break;
			}
		  }

		  if(prev_reqnbr != req_hdr.code) {
			nbr_printed_req++;
			prev_reqnbr=req_hdr.code;
		  }
		  if((code < NOERROR || code == EXIT) && code != EFL)
			break;

		}
		if (code == EXIT) break;
	}
	close_file(REQHDR) ;
	close_file(REQITEM) ;

	memset(total_str,' ',sizeof(total_str));

	tedit((char *)&nbr_printed_req,"___0_",e_mesg,R_LONG);
#ifdef ENGLISH
	sprintf(&total_str[70],"No of Req's Printed: %s\n",e_mesg);
#else
	sprintf(&total_str[70],"No de Req imprimes: %s\n",e_mesg);
#endif

	rpclose_mesg(total_str) ;
	nbr_printed_req = 0;
	if(code == EFL) return(0);
	return(code);
}


