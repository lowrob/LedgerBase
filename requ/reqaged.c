/*
*	Source 	: reqaged.c 
*
*	Program to Print Requisition Aged Report.
*
*/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>
#include <requ.h>

#define	PROJNAME	"reqrpt1"
#define	LOG_REC		1
#define	FORMNO		4
#define EXIT		12
#define DELTA		0.005		/* To check float or double values*/

#ifdef ENGLISH
#define PRINTER 'P'
#define DISPLAY 'D'
#define FILE_IO	'F'
#else
#define PRINTER 'I'
#define DISPLAY 'A'
#define FILE_IO	'D'
#endif

extern	int	rperror;
extern char	e_mesg[80] ;

reqaged()
{
Pa_rec 	 pa_rec ;
Req_hdr  req_hdr;
Req_item req_item;
Supplier supp_rec;
Sch_rec	 sch_rec;

char	chardate[11] ;
int	code;
char 	*arayptr[5] ; 	/** Declarations for Report writer usage **/
char 	projname[50] ;
char 	program[11] ;
int 	outcntl ;
char 	discfile[20] ;
int	retval;
short	ccno1, ccno2;
char    suppno1[11], suppno2[11];
char    tempsupp[11];
long	parm_date ;
long	diff ;
short	copies ;

	code = get_param(&pa_rec, BROWSE, 1, e_mesg) ;
	if(code < 1) {
		close_dbh() ;
		return(DBH_ERR) ;
	}
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
			outcntl = 1;
			break;
		case PRINTER:       /* Print to printer */
			outcntl = 2;
			break;
		default:
			outcntl = 2;
			break;
	}
	if(outcntl == 1) {
		STRCPY(e_mesg,"reqaged.dat");
		if((retval = GetFilename(e_mesg)) < 0)
			return(retval);
		STRCPY(discfile, e_mesg);
	}
	else 	discfile[0] = '\0';

	copies = 1;
	if( outcntl == 2) {
		if((retval = GetNbrCopies( &copies )) < 0)
			return(retval);
	}

	STRCPY(suppno1,"         0");
	STRCPY(suppno2,"ZZZZZZZZZZ");
	retval = GetSuppRange( suppno1, suppno2 );
	if(retval < 0) return(retval);

	if(CC_no == pa_rec.pa_distccno) {
		ccno1 = 0;
		ccno2 = 99;
		retval = GetCCRange( &ccno1, &ccno2 );
		if(retval < 0) return(retval);
	}
	else {
		ccno1 = CC_no;
		ccno2 = CC_no;
	}

	parm_date = get_date();
	retval = GetDate( &parm_date );
	if(retval < 0) return(retval);

	if((retval = Confirm()) <= 0) return(retval);

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

	if( outcntl == 2) 
		rpSetCopies( (int)copies );   /* number of copies to print */

	/* Change first title line to Company/School district name */
	if((code = rpChangetitle(1, pa_rec.pa_co_name))<0) {
#ifdef ENGLISH
		sprintf(e_mesg,"Internal report error");
#else
		sprintf(e_mesg,"Erreur au rapport interne");
#endif
		rpclose();
		close_dbh();
		return(REPORT_ERR);
	}


	mkdate( parm_date, chardate );
#ifdef ENGLISH
	sprintf(e_mesg,"As of %s",chardate);
#else
	sprintf(e_mesg,"a partir de %s",chardate);
#endif
	if((code = rpChangetitle(3,e_mesg))<0) {
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

	/* Initialize Recs. to start printing req report on Supplier code. */
	tempsupp[0] = '\0';         
	req_hdr.funds = 1;
	STRCPY(req_hdr.supp_cd,suppno1);
	req_hdr.code = 0;

	flg_reset( REQHDR );

	for( ; ; ) {
		code = get_n_reqhdr(&req_hdr,BROWSE,3,FORWARD,e_mesg);
		if( code < 0) {
			if(code == EFL) break;
			code = DBH_ERR;
			break ;
		}
		if(strcmp(req_hdr.supp_cd,suppno2) > 0) break;

		if(req_hdr.status[0] == PROCESSED || 
		   req_hdr.status[0] == DISAPPROVED) {
			continue;
		}

		if(req_hdr.costcenter < ccno1 || req_hdr.costcenter > ccno2) {
			continue;
		}

		if(strcmp(req_hdr.supp_cd,tempsupp) != 0) {
		     STRCPY(supp_rec.s_supp_cd,req_hdr.supp_cd);
		     code = get_supplier(&supp_rec,BROWSE,0,e_mesg);
		     if( code != NOERROR) {
			 STRCPY(supp_rec.s_name,"???????????????");
		     }
		     STRCPY(tempsupp,req_hdr.supp_cd);
		}

		supp_rec.s_ytd_ord = supp_rec.s_ytd_ret = 0 ;
		supp_rec.s_ytd_recpt = supp_rec.s_ytd_disc = 0 ;
		
		diff = days(req_hdr.due_date) - days(parm_date);

		if(req_hdr.status[0] == APPROVED) {
			req_hdr.amount = req_hdr.appamt;
		}

		if(diff < 0) {		/* Past Due */ 
			supp_rec.s_ytd_ord = req_hdr.amount;
		}
		else if (diff <= 30) {	/* Currently Due */
			supp_rec.s_ytd_ret = req_hdr.amount;
		}
		else if (diff <= 60) {	/*Due Next Period */
			supp_rec.s_ytd_recpt = req_hdr.amount;
		}
		else {			/* Due in Future */ 
			supp_rec.s_ytd_disc = req_hdr.amount;
		}

	       	if((code = rpline(arayptr)) < 0) {
			if( rperror < 0 )  {
#ifdef ENGLISH
			sprintf(e_mesg,"Internal report error");
#else
			sprintf(e_mesg,"Erreur au rapport interne");
#endif
				code = REPORT_ERR;
			}
			else
				code = EXIT;
			 break ;
		}
	}

	close_dbh() ;
	rpclose() ;
	if(code == EFL ) return(0);
	return(code);
}
