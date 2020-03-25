/*
*	Source 	: pobysupp.c 
*
*	Program to Print purchase orders by supplier no. using REPORT GENERATOR.
*
Modifications:

	YY/MM/DD	Programmer	Description
	~~~~~~~~	~~~~~~~~~~	~~~~~~~~~~~
	90/12/11	C.Leadbeater	Added GST & PST to 'AMOUNT' value 
					(poitem_rec.pi_value).

	91/02/07	J.Prescott	Added number of po's printed.
*/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>
#include <repdef.h>

#define	PROJNAME	"porpt"
#define	LOG_REC		1
#define	FORMNO		2
#define EXIT		12
#define TAXABLE		'T'

#ifdef ENGLISH
#define PRINTER 'P'
#define DISPLAY 'D'
#define FILE_IO	'F'
#define YES	'Y'
#else
#define PRINTER 'I'
#define DISPLAY 'A'
#define FILE_IO	'D'
#define YES	'O'
#endif

extern	int	rperror ;
extern char	e_mesg[80] ;

PobySupp()
{
Pa_rec 	 pa_rec ;
Supplier supp_rec;
Po_hdr   pohdr_rec;
Po_item  poitem_rec;
Ctl_rec  ctl_rec;
Gl_rec	 gl_rec;

char	chardate[11] ;
int	code;
char 	*arayptr[5] ; 	/** Declarations for Report writer usage **/
char 	projname[50] ;
char 	program[11] ;
int 	outcntl ;
char 	discfile[20] ;
int     keyno;
int     formno;
int	retval;
char    suppno1[11], suppno2[11];
char    tempsupp[11];
char	outstanding[2];
short	copies ;
long	nbr_printed_po;
long	prev_ponbr;

short	err;
short	gst_tax, pst_tax;
double  gst_amt, pst_amt;
static	Tax_cal	tax_cal;

	formno = 2;
	keyno = 1;
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
		case PRINTER:
			outcntl = 2;
			break;
		default:
			outcntl = 2;
			break;
	}
	if(outcntl == 1) {
		STRCPY(e_mesg,"pobysupp.dat");
		if((retval = GetFilename(e_mesg)) < 0)
			 return(retval);
		STRCPY(discfile, e_mesg);
	}
	else 	discfile[0] = '\0';

	copies = 1;
	if(outcntl == 2) {
		if((retval = GetNbrCopies( &copies )) < 0)
			return(retval);
	}

	STRCPY(suppno1,"         0");
	STRCPY(suppno2,"ZZZZZZZZZZ");
	retval = GetSuppRange( suppno1, suppno2 );
	if(retval < 0) return(retval);

	if((retval = 
#ifdef ENGLISH
		DisplayMessage("Do you want only outstanding PO's (Y/N)?"))<0)
#else
		DisplayMessage("Desirez-vous seulement les BC non-regles(O/N)?""))<0)
#endif
			return(retval);
	if((retval = GetResponse( outstanding ))<0)
		return(retval);
	if((retval = Confirm()) <= 0)
		return(retval);

	code = get_param(&pa_rec, BROWSE, 1, e_mesg) ;
	if(code < 1) {
		close_dbh() ;
		return(DBH_ERR) ;
	}

	mkdate(get_date(),chardate);

	STRCPY(projname,FMT_PATH) ;
	strcat(projname,PROJNAME) ;
	STRCPY(program, PROG_NAME );

	code = rpopen(projname,LOG_REC,formno,outcntl,discfile,program,
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
	/* For Terminals set pagesize to 23 lines */
	if(outcntl == 0)
		rpPagesize(23);

	arayptr[0] = (char *)&supp_rec ;
	arayptr[1] = (char *)&pohdr_rec ;
        arayptr[2] = (char *)&poitem_rec ;
	arayptr[3] = (char *)&gl_rec ;
        arayptr[4] = NULL ;

	/***	Read record and call report writer to output it ***/

	/* Initialize Recs. to start printing po report on po no. */
	STRCPY(tempsupp,"          ");         
	STRCPY(pohdr_rec.ph_supp_cd,suppno1);
	pohdr_rec.ph_code = 0;

	flg_reset( POHDR );

	nbr_printed_po=0;

	for( ; ; ) {
		code = get_n_pohdr(&pohdr_rec,BROWSE,keyno,FORWARD,e_mesg);
		if( code < 0) {
			if(code == EFL) break ;
			code = DBH_ERR;
			break ;
		}
		if(strcmp(pohdr_rec.ph_supp_cd,suppno2) > 0) break;
		if(outstanding[0] == YES) {
			if(pohdr_rec.ph_status[0] != OPEN) continue;
		}
		if(strcmp(pohdr_rec.ph_supp_cd,tempsupp) != 0) {
		     STRCPY(supp_rec.s_supp_cd,pohdr_rec.ph_supp_cd);
		     code = get_supplier(&supp_rec,BROWSE,0,e_mesg);
		     if( code != NOERROR) {
			  STRCPY(supp_rec.s_name,"???????????????");
		     }
		     STRCPY(tempsupp,pohdr_rec.ph_supp_cd);
		}
		poitem_rec.pi_code = pohdr_rec.ph_code;
 		poitem_rec.pi_item_no = 0;
        	flg_reset( POITEM );
		for( ; ; ) {
#ifndef ORACLE
		  code = get_n_poitem(&poitem_rec,BROWSE,0,FORWARD,e_mesg);
#else
		  code = get_n_poitem(&poitem_rec,BROWSE,0,EQUAL,e_mesg);
#endif
		  if( code < 0) {
			if(code == EFL) break ;
			code = DBH_ERR;
			break ;
		       }
#ifndef ORACLE
		  if(pohdr_rec.ph_code != poitem_rec.pi_code) break;
#endif

		if(outstanding[0] == YES) {
			if(poitem_rec.pi_orig_qty == poitem_rec.pi_pd_qty)
				continue;
		}

	/* 	calculate tax on each po item (CL) 	*/

 		  ctl_rec.fund = poitem_rec.pi_fund ;
		  err = get_ctl(&ctl_rec, BROWSE, 0, e_mesg) ;
		  if(err < 0) {
			return(err) ;
		  }

		  gst_tax = ( (poitem_rec.pi_tax1[0] == TAXABLE)
			? ctl_rec.gst_tax : 0 );
		  pst_tax = ( (poitem_rec.pi_tax2[0] == TAXABLE)
			? ctl_rec.pst_tax : 0 );
	
		  Tax_Calculation(gst_tax,pst_tax,poitem_rec.pi_value,&tax_cal);
		  poitem_rec.pi_value = tax_cal.gros_amt;

		  if((code = rpline(arayptr)) < 0) {
			if(rperror < 0) {
#ifdef ENGLISH
			sprintf(e_mesg,"Internal report error");
#else
			sprintf(e_mesg,"Erreur au rapport interne");
#endif
				code = REPORT_ERR;
			}
			else
				code = EXIT ;
			break;
		  }
		  if(prev_ponbr != pohdr_rec.ph_code) {
			nbr_printed_po++;
			prev_ponbr = pohdr_rec.ph_code;
		  }
                }
		if((code < NOERROR || code == EXIT) && code != EFL) break;
	}

	close_dbh() ;

	rpPutline();
	rpPutline();
	rpPutline();
#ifdef ENGLISH
	rpMkline(2,"No of Po's Printed:");
#else
	rpMkline(2,"No de BC imprimes:");
#endif
	tedit((char *)&nbr_printed_po,"___0_",e_mesg,R_LONG);
	rpMkline(3,e_mesg);
	rpPutline();
	rpclose() ;
	if(code == EFL) return(0);
	return(code);
}
