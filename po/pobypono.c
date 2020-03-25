/* ---------------------------------------------------------------------------
*	Source 	: pobypono.c 
*
*	Program to Print purchase orders by po no. using REPORT GENERATOR.
*

Modifications:

	YY/MM/DD	Programmer	Description
	~~~~~~~~	~~~~~~~~~~	~~~~~~~~~~~
	90/12/11	C.Leadbeater	Include GST & PST amounts on the 
					'AMOUNT' column value. (poitem_rec.
					pi_value).
	91/02/07	J.Prescott	Add Total number of po's printed.
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>
#include <repdef.h>

#define	TAXABLE		'T'
#define	PROJNAME	"porpt"
#define	LOG_REC		1
#define	FORMNO		1
#define EXIT		12
#define LIQUID	 	1
#define PURGE		2
#define R_PURGE		3

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

extern	int	rperror	;
extern char	e_mesg[80] ;

static Tax_cal	tax_cal;

PobyPoNbr(mode)
int mode;
{
Pa_rec 	 pa_rec ;
Supplier supp_rec;
Po_hdr   pohdr_rec;
Ctl_rec  ctl_rec ;		/* Fund/Control Record */
Po_item  poitem_rec;

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
int     formno;
int	retval;
long    ponbr1, ponbr2;
int	rd_md ;
char	outstanding[2];
long	date1,date2 ;
short	copies ;
int	column_nbr;
long	nbr_printed_po;
long	prev_ponbr;

short	err;
short	gst_tax, pst_tax;
double  gst_amt, pst_amt;

	formno = 1;
	keyno = 0;
#ifdef ENGLISH
	STRCPY(e_mesg, "P");
#else
	STRCPY(e_mesg, "I");
#endif
	if (mode != PURGE && mode != R_PURGE) {
		retval = GetOutputon(e_mesg);
		if(retval < 0) return(retval);
	}

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
		if(mode == LIQUID)
			STRCPY(e_mesg,"poliquid.dat");
		else STRCPY(e_mesg,"pobypono.dat");
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

	ponbr1 = 0;
	ponbr2 = 99999999;
	if (mode != PURGE && mode != R_PURGE) {
		date1 = date2 = get_date();
		retval = GetDateRange( &date1, &date2 );
		if(retval < 0) return(retval);

		retval = GetPoRange( &ponbr1, &ponbr2 );
		if(retval < 0) return(retval);
		if (mode != LIQUID) {
#ifdef ENGLISH
			DisplayMessage("Do you want only outstanding PO's (Y/N)?");
#else
			DisplayMessage("Desirez-vous seulement les BC non-regles (O/N)?");
#endif
			GetResponse(outstanding);
		}

	}
	if (mode != PURGE) {
		if((retval = Confirm()) <= 0) 
			return(retval);
	}

	code = get_param(&pa_rec, BROWSE, 1, e_mesg) ;
	if(code < 1) {
		return(DBH_ERR) ;
	}

	mkdate(get_date(),chardate);

	STRCPY(projname,FMT_PATH) ;
	strcat(projname,PROJNAME) ;
	STRCPY(program, PROG_NAME );

	if(mode == LIQUID || mode == PURGE || mode == R_PURGE) {
		formno = 3;
	}
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
	if (mode == PURGE || mode == R_PURGE) { 
#ifdef ENGLISH
		if((retval = rpChangetitle(2,
				"DELETION OF COMPLETED PURCHASE ORDERS"))<0) {
#else
		if((retval = rpChangetitle(2,
				"ELIMINATION DES BONS DE COMMANDE COMPLETES"))<0) {
#endif
#ifdef ENGLISH
			sprintf(e_mesg,"Internal report error");
#else
			sprintf(e_mesg,"Erreur au rapport interne");
#endif
			rpclose();
			close_dbh();
			return(REPORT_ERR);
		}	
	
#ifdef ENGLISH
		sprintf(e_mesg,"Period: %d",pa_rec.pa_cur_period);
#else
		sprintf(e_mesg,"Periode: %d",pa_rec.pa_cur_period);
#endif
		if((retval = rpChangetitle(3,e_mesg))<0){
#ifdef ENGLISH
			sprintf(e_mesg,"Internal report error");
#else
			sprintf(e_mesg,"Erreur au rapport interne");
#endif
			rpclose();
			close_dbh();
			return(REPORT_ERR);
		}	
	}
	else  {
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
	if (mode == PURGE || mode == R_PURGE) 
		rd_md = UPDATE ; 
	else
		rd_md = BROWSE ;

	nbr_printed_po = 0;
	for( ; ; ) {
		code = get_n_pohdr(&pohdr_rec,rd_md,keyno,FORWARD,e_mesg);

		if( code < 0) {
			if(code == EFL)  break ;
			code = DBH_ERR;
			break ;
		}

		if(mode != PURGE && mode != R_PURGE) {
			if(pohdr_rec.ph_code > ponbr2) { /* PO Range */
				roll_back(e_mesg);
				break;
			}

			if (pohdr_rec.ph_date < date1 ||
		    	pohdr_rec.ph_date > date2) {	/* Check date range */
#ifndef	ORACLE
				roll_back(e_mesg);
#endif
				continue;
			}
		}

		if((mode != LIQUID && mode != PURGE && mode != R_PURGE) && outstanding[0] == YES) {
			if(pohdr_rec.ph_status[0] != OPEN) continue;
		}
		poitem_rec.pi_code = pohdr_rec.ph_code;
 		poitem_rec.pi_item_no = 0;
        	flg_reset( POITEM );
		for( ; ; ) {
#ifndef ORACLE
		  code = get_n_poitem(&poitem_rec,rd_md,keyno,FORWARD,e_mesg);
#else
		  code = get_n_poitem(&poitem_rec,rd_md,keyno,EQUAL,e_mesg);
#endif
		  if( code < 0) {
			if(code == EFL)  break ;
			code = DBH_ERR;
			break ;
		       }
#ifndef ORACLE
		  if(pohdr_rec.ph_code != poitem_rec.pi_code) break;
#endif

		if(poitem_rec.pi_orig_qty == poitem_rec.pi_pd_qty && 
		   mode != PURGE && mode != R_PURGE)
			continue;

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
		  Tax_Calculation(gst_tax,pst_tax,poitem_rec.pi_paid,&tax_cal);
		  poitem_rec.pi_paid = tax_cal.gros_amt;

		  if(mode == LIQUID || mode == PURGE || mode == R_PURGE) {
			if(pohdr_rec.ph_status[0]==COMPLETE) {
			   if(mode == LIQUID || 
			      date_plus(pohdr_rec.ph_lqdate,30) < get_date()) {
		  		if((code = rpline(arayptr)) < 0) {
					if(rperror < 0)  {
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
				if(prev_ponbr != pohdr_rec.ph_code) {
					nbr_printed_po++;
					prev_ponbr=pohdr_rec.ph_code;
				}
				if(mode == PURGE || mode == R_PURGE) {
				   code = put_poitem(&poitem_rec,P_DEL,e_mesg);
				   if (code < 0) {
					break;
				   }
				   poitem_rec.pi_item_no++;
				}
			   } 
			}
			else {
#ifndef	ORACLE
				roll_back(e_mesg);
#endif
				break;
			}
		  }
		  else { 
			if((code = rpline(arayptr)) <0) {
				if(rperror < 0)  {
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
				prev_ponbr=pohdr_rec.ph_code;
			}
		  }
                }
		if((code < NOERROR || code == EXIT) && code != EFL)
			break;


		if(mode == PURGE || mode == R_PURGE) {
			if(pohdr_rec.ph_status[0] == COMPLETE && 
			   date_plus(pohdr_rec.ph_lqdate,30) < get_date()) {
				code = put_pohdr(&pohdr_rec, P_DEL, e_mesg) ;
				if (code < 0) {
					code = DBH_ERR;
					break;
				}
				if (commit(e_mesg) < 0) {
					code = DBH_ERR;
					break ;
				}
			}
#ifndef	ORACLE
			else roll_back(e_mesg);
#endif
		}
	}

	close_file(POHDR) ;
	close_file(POITEM) ;

	if(mode == LIQUID || mode == PURGE || mode == R_PURGE) 
		column_nbr = 7;
	else
		column_nbr = 9;

	rpPutline();
	rpPutline();
	rpPutline();
#ifdef ENGLISH
	rpMkline(column_nbr,"No of Po's Printed:");
#else
	rpMkline(column_nbr,"No de BC imprimes:");
#endif
	tedit((char *)&nbr_printed_po,"___0_",e_mesg,R_LONG);
	rpMkline(column_nbr+1,e_mesg);
	rpPutline();

	rpclose() ;
	nbr_printed_po = 0;
	if(code == EFL) return(0);
	return(code);
}


