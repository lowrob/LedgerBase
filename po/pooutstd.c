/******************************************************************************
		Sourcename    : pooutstd.c
		System        : Budgetary Financial system.
		Module        : Purchase Order
		Created on    : 89-09-30
		Created  By   : Cathy Burns 
		Cobol sources : 
*******************************************************************************
About this program:

HISTORY:
 Date           Programmer     Description of modification
____/__/__      __________     ___________________________
1990/11/27	M. Cormier	Quit option wasn't working. Made a test if
				rperror is less than zero (< NOERROR).
1990/12/11	C.Leadbeater	Add GST & PST to pi_paid and pi_value fields
				('ORDER AMT' & 'PAID AMOUNT').
1990/12/27	F.Tao		Changed  Order amount calculation.
1991/02/07	J.Prescott	Added number of Po's printed.
*******************************************************************************/
#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>
#include <repdef.h>

#define	DELTA_AMOUNT	0.005

#define	PROJNAME	"porpt"
#define	LOG_REC		1
#define	FORMNO		6
#define EXIT		12
#define SYSTEM		"PURCHASE ORDER"
#define MOD_DATE	"28-SEPT-89"
#define PARTIALONLY	0
#define OUTSTDONLY	1
#define TAXABLE		'T'

#ifdef ENGLISH
#define PRINTER 'P'
#define DISPLAY 'D'
#define FILE_IO	'F'
#else
#define PRINTER 'I'
#define DISPLAY 'A'
#define FILE_IO	'D'
#endif

extern	int	rperror	;
extern char e_mesg[80]; /* to store error messages */

PoOutstanding(option)
int option;
{
Pa_rec 	 pa_rec ;
Supplier supp_rec;
Po_hdr   pohdr_rec;
Po_item  poitem_rec;
Ctl_rec  ctl_rec;

short	err;
short	gst_tax, pst_tax;
double  gst_amt, pst_amt;
static  Tax_cal	 tax_cal;

char    chardate[11];
char	chardate1[11] ;
char	chardate2[11] ;
int	code;
char 	*arayptr[5] ; 	/** Declarations for Report writer usage **/
char 	projname[50] ;
char 	program[11] ;
int 	outcntl ;
char 	discfile[20] ;
int	retval;
long    ponbr1, ponbr2;
char	outstanding[2];
long	date1,date2 ;
short	copies ;
long	nbr_printed_po;
long	prev_ponbr;



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
		case PRINTER:
			outcntl = 2;
			break;
		default:
			outcntl = 2;
			break;
	}
	if(outcntl == 1) {
		STRCPY(e_mesg,"pooutstd.dat");
		if((retval = GetFilename(e_mesg)) < 0) return(retval);
		STRCPY(discfile, e_mesg);
	}
	else 	discfile[0] = '\0';

	copies = 1;
	if(outcntl == 2) {
		if((retval = GetNbrCopies( &copies )) < 0)
			return(retval);
	}

	date1 = date2 = get_date();
	retval = GetDateRange( &date1, &date2 );
	if(retval < 0) return(retval);

	ponbr1 = 0;
	ponbr2 = 99999999;
	retval = GetPoRange( &ponbr1, &ponbr2 );
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

	if(outcntl == 2) 
		rpSetCopies( (int)copies );   /* number of copies to print */

	/* Change first title line to Company/School district name */
	retval = rpChangetitle(1, pa_rec.pa_co_name);
	if(option == OUTSTDONLY){
#ifdef ENGLISH
	retval = rpChangetitle(2,"LIST OF OUTSTANDING PURCHASE ORDERS");
#else
	retval = rpChangetitle(2,"LISTE DES BONS DE COMMANDE NON-REGLES");
#endif
	}else{
#ifdef ENGLISH
	retval = rpChangetitle(2,"LIST OF PARTIAL PURCHASE ORDERS");
#else
	retval = rpChangetitle(2,"LISTE DES BC PARTIELS");
#endif
	}

	if(retval < 0) {
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
        arayptr[3] = NULL ;

	/***	Read record and call report writer to output it ***/

	/* Initialize Recs. to start printing po report on po no. */
	pohdr_rec.ph_code = ponbr1;

	flg_reset( POHDR );

	nbr_printed_po = 0;

	for( ; ; ) {
		code = get_n_pohdr(&pohdr_rec,BROWSE,0,FORWARD,e_mesg);

		if( code < 0) {
			if(code == EFL) break ;
			code = DBH_ERR;
			break ;
		}

		if(pohdr_rec.ph_code > ponbr2) { /* PO Range */
			roll_back(e_mesg);
			break;
		}

		if (pohdr_rec.ph_date < date1 ||
	   	    pohdr_rec.ph_date > date2) {	/* Check date range */
			roll_back(e_mesg);
			continue;
		}

		if(pohdr_rec.ph_status[0] == COMPLETE) continue;

		if (strcmp(supp_rec.s_supp_cd, pohdr_rec.ph_supp_cd) != 0) {
			STRCPY( supp_rec.s_supp_cd, pohdr_rec.ph_supp_cd);
			retval = get_supplier(&supp_rec,BROWSE,0,e_mesg);
			if( retval < 0) 
#ifdef ENGLISH
				STRCPY(supp_rec.s_name, "*** NOT DEFINED ****");
#else
				STRCPY(supp_rec.s_name, "*** PAS DEFINI ****");
#endif

			sprintf(supp_rec.s_add1,"%10s -%17s",supp_rec.s_supp_cd,
							  supp_rec.s_name);
		}
			
		if (pohdr_rec.ph_status[0] == OPEN) 
		   if (option == OUTSTDONLY) 
#ifdef ENGLISH
		   	sprintf(supp_rec.s_add2,"%ld - OPEN",pohdr_rec.ph_code);
#else
		   	sprintf(supp_rec.s_add2,"%ld - OUVERT",pohdr_rec.ph_code);
#endif
	            else if (pohdr_rec.ph_lqamt > DELTA_AMOUNT)
#ifdef ENGLISH
		   	sprintf(supp_rec.s_add2,"%ld - PARTIAL",pohdr_rec.ph_code);
#else
		   	sprintf(supp_rec.s_add2,"%ld - PARTIEL",pohdr_rec.ph_code);
#endif
		    else continue;
		else continue;
	

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


		/* Calculate Tax on 'PAID AMT' value */	

		  Tax_Calculation(gst_tax,pst_tax,poitem_rec.pi_paid,&tax_cal);
		  poitem_rec.pi_paid = tax_cal.gros_amt; 
		
		/* print report line */

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
				prev_ponbr = pohdr_rec.ph_code;
				nbr_printed_po++;
			}
		}
		if((code < NOERROR || code == EXIT) && code != EFL)  break;
	}
	close_dbh() ;

	rpPutline();
	rpPutline();
	rpPutline();
#ifdef ENGLISH
	rpMkline(6,"No of Po's Printed:");
#else
	rpMkline(6,"No de BC imprimes:");
#endif
	tedit((char *)&nbr_printed_po,"___0_",e_mesg,R_LONG);
	rpMkline(7,e_mesg);
	rpPutline();

	rpclose() ;
	if(code == EFL) return(0);
	return(code);
}
