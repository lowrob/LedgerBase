/******************************************************************************
		Sourcename    : poyend.c
		System        : Budgetary Financial system.
		Module        : Purchase Order
		Created on    : 89-09-30
		Created  By   : Cathy Burns 
		Cobol sources : 
*******************************************************************************
About this program:

HISTORY:
 Date           Programmer     Description of modification
YYYY/MM/DD      __________     ___________________________
1990/12/18	C.Leadbeater   Added D_Roundoff() for rounding any calculated 
			       double values before writing to file.
*******************************************************************************/
#include <stdio.h>

#include <bfs_defs.h>
#include <bfs_recs.h>

#define PROJNAME	"porpt"
#define LOG_REC		1
#define	FORMNO		6

#define	EXIT		12	
#define DELTA_QTY 	0.00005
#define DELTA_AMT 	0.005
#define TAXABLE		'T'

static char e_mesg[80]; /* to store error messages */

static	Pa_rec		pa_rec ;
static	Ctl_rec		ctl_rec ;
static	Gl_rec		gl_rec ;
static  Po_hdr 		po_hdr ;
static 	Po_item		po_item ;
static 	St_mast		st_mast ;
static	Alloc_rec	alloc_rec ;
static	Supplier	supplier ;

double	D_Roundoff();
static 	char 	*arayptr[5] ; 	/** Declarations for Report writer usage **/


PoYearEnd( )
{
	int retval;

#ifdef ENGLISH
	printf("\n\n Starting PO Close") ;
#else
	printf("\n\n Commence la fermeture de BC") ;
#endif
	if ((retval = OpenReport()) < 0) {
		close_dbh() ;
		return(-1);
	}

	if ((retval = YearEndRtn()) < 0) {
#ifdef ENGLISH
		printf("\n\n Error in PO Close ") ;
#else
		printf("\n\n Erreur dans la fermeture de BC ") ;
#endif
		CloseRtn() ;
		return(-1);
 	}	
#ifdef ENGLISH
	printf("\n\n PO Close Sucessfully Completed") ;
#else
	printf("\n\n Fermeture de BC reussie");
#endif
	CloseRtn();
	return(1);
}

YearEndRtn()
{
	int	retval;

	po_hdr.ph_supp_cd[0] = '\0' ;
	po_hdr.ph_code = 0 ;
	flg_reset( POHDR );

	for( ;; ) {
		retval = get_n_pohdr(&po_hdr,UPDATE,1,FORWARD,e_mesg);
		if (retval < 0) {
			if (retval == EFL) break;
			DispMessage(e_mesg);
			break;
		}

		if (strcmp(supplier.s_supp_cd, po_hdr.ph_supp_cd) != 0) {
			STRCPY( supplier.s_supp_cd, po_hdr.ph_supp_cd);
			retval = get_supplier(&supplier,BROWSE,0,e_mesg);
			if( retval < 0) 
#ifdef ENGLISH
				STRCPY(supplier.s_name, "*** NOT DEFINED ****");
#else
				STRCPY(supplier.s_name, "*** PAS DEFINI ****");
#endif

			sprintf(supplier.s_add1,"%10s -%17s",supplier.s_supp_cd,
							  supplier.s_name);
		}
		if (po_hdr.ph_status[0] == COMPLETE) 
#ifdef ENGLISH
			sprintf(supplier.s_add2,"%ld - COMPLETE",
					po_hdr.ph_code);
		else
			sprintf(supplier.s_add2,"%ld - OPEN",
					po_hdr.ph_code);
#else	/* FRENCH */
			sprintf(supplier.s_add2,"%ld - COMPLET",
					po_hdr.ph_code);
		else
			sprintf(supplier.s_add2,"%ld - OUVERT",
					po_hdr.ph_code);
#endif

		/*  Bulk PO's are committed to the inventory general acct */
		if (po_hdr.ph_type[0] == BULK) {
			if (ctl_rec.fund != po_hdr.ph_funds) {
				ctl_rec.fund = po_hdr.ph_funds ;
				retval = get_ctl(&ctl_rec,BROWSE,0,e_mesg);
			}
			if ((retval = CheckGl( ctl_rec.inv_acnt )) < 0) 
				break ;
		}
		if ((retval = ProcItems()) < 0 ) break;

		/* Bulk PO's commit to one acct which is inventory general */
		if (po_hdr.ph_type[0] == BULK) {
			gl_rec.comdat = gl_rec.comdat -
				 (po_hdr.ph_comm - po_hdr.ph_lqamt);

			gl_rec.comdat=D_Roundoff(gl_rec.comdat);
			retval = put_gl(&gl_rec,UPDATE,e_mesg);
			if (retval < 0) {
				DispMessage(e_mesg);
				break;
			}
		}
		retval = put_pohdr(&po_hdr,P_DEL,e_mesg);
		if (retval < 0) {
			DispMessage(e_mesg);
			break;
		}
		if (commit(e_mesg) < 0) {
			DispMessage(e_mesg);
			break;
		}
		po_hdr.ph_code++;

	}
	seq_over( POHDR );
	if (retval < 0 && retval != EFL) return(-1);

	return(NOERROR);

}

/* Process all items on PO so that they can be backed out completely */
ProcItems()
{
	int retval;
	double os_amt,os_qty;
	short	err;
	short	gst_tax, pst_tax;
	double  gst_amt, pst_amt;
	static  Tax_cal	 tax_cal;

	po_item.pi_code = po_hdr.ph_code ;
	po_item.pi_item_no = 0 ;
	flg_reset( POITEM );
	for ( ; ; ) {
#ifndef ORACLE
		retval = get_n_poitem(&po_item,UPDATE,0,FORWARD,e_mesg);
#else
		retval = get_n_poitem(&po_item,UPDATE,0,EQUAL,e_mesg);
#endif
		if (retval < 0) {
			if (retval == EFL) break;
			DispMessage(e_mesg);
			break;
		}

#ifndef ORACLE
		if (po_item.pi_code != po_hdr.ph_code) break;
#endif

 		ctl_rec.fund = po_item.pi_fund ;
		err = get_ctl(&ctl_rec, BROWSE, 0, e_mesg) ;
		if(err < 0) {
			return(err) ;
		}

		gst_tax = ( (po_item.pi_tax1[0] == TAXABLE)
			? ctl_rec.gst_tax : 0 );
		pst_tax = ( (po_item.pi_tax2[0] == TAXABLE)
			? ctl_rec.pst_tax : 0 );

		/* Calculate O/S amts and qtys for each item */
		os_qty = po_item.pi_orig_qty - po_item.pi_pd_qty ;
		os_amt = po_item.pi_value - po_item.pi_paid ;
		Tax_Calculation(gst_tax,pst_tax,os_amt,&tax_cal);
		os_amt = tax_cal.gros_amt;

		if (po_hdr.ph_type[0] == NON_BULK)  {
			retval = CheckAlloc() ;
			if (retval != UNDEF && retval < 0) break; 
			if (retval == UNDEF) 
				os_qty = os_amt = 0;
			else {
			     /* Non bulk should decrease by either   
				outstanding or remaining in allocations
				whichever is less */
			 	if (alloc_rec.st_value < os_amt) {
					os_amt = alloc_rec.st_value ;
					alloc_rec.st_value = 0.00 ;
				}
				else
					alloc_rec.st_value -= os_amt ;
				if (alloc_rec.st_alloc < os_qty) {
					os_qty = alloc_rec.st_value ;
					alloc_rec.st_alloc = 0.00 ;
				}
				else
					alloc_rec.st_alloc -= os_qty ;
			}
		}
				
		if (po_hdr.ph_type[0] != DIRECT) 
			if ((retval = CheckStock()) < 0) break;

		if (po_hdr.ph_type[0] != BULK) 		/* Update Gl */
			if ((retval = CheckGl( po_item.pi_acct )) < 0) 
				break ;

		if (po_hdr.ph_type[0] != DIRECT) {
			if (po_hdr.ph_type[0] == NON_BULK) 
				st_mast.st_alloc -= os_qty ;
			st_mast.st_on_order -= os_qty ;
			if (st_mast.st_on_order < DELTA_QTY) 
				st_mast.st_on_order = 0 ;
		}

		if(rpline(arayptr) < 0) break ;

		if (po_hdr.ph_type[0] == NON_BULK)  {
		
			if (alloc_rec.st_alloc < DELTA_QTY) 
			     retval = put_alloc(&alloc_rec, P_DEL, e_mesg);
			else{
			     alloc_rec.st_alloc=D_Roundoff(alloc_rec.st_alloc);
			     retval = put_alloc(&alloc_rec, UPDATE, e_mesg);
			}
			if (retval < 0) break;
		}
		if (po_hdr.ph_type[0] != DIRECT) {
			st_mast.st_on_order = D_Roundoff(st_mast.st_on_order);
			st_mast.st_on_order = D_Roundoff(st_mast.st_alloc);
			if ((retval = put_stmast(&st_mast,UPDATE,e_mesg)) < 0) 
				break;
		}
		if (po_hdr.ph_type[0] != BULK) { 	/* Update Gl */
			gl_rec.comdat -= os_amt ;
			gl_rec.comdat=D_Roundoff(gl_rec.comdat);
			if ((retval = put_gl(&gl_rec,UPDATE,e_mesg)) < 0) break;
		}

		if ((retval = put_poitem(&po_item,P_DEL,e_mesg)) < 0) break;

		po_item.pi_item_no++;		/* To read next record after */
						/* updating */
	}
	seq_over( POITEM );
	if (retval < 0 && retval != EFL)  {
		DispMessage(e_mesg);
		return(-1);

	}
	return(NOERROR);
}


CheckGl( accno )
char	*accno;
{
	int	retval ;

	gl_rec.funds = po_hdr.ph_funds ;
	STRCPY(gl_rec.accno, accno);
	gl_rec.reccod = 99 ;
	retval = get_gl(&gl_rec, UPDATE, 0, e_mesg) ;
	if (retval < 0) {
		DispMessage(e_mesg);
		return(-1) ;
	}
	return(NOERROR);
}

CheckStock()
{
	int 	retval;

	st_mast.st_fund = po_hdr.ph_funds ;
	STRCPY( st_mast.st_code, po_item.pi_st_code);
	if ((retval = get_stmast(&st_mast,UPDATE,0,e_mesg)) < 0) return(retval);
	return(NOERROR);

}

CheckAlloc()
{
	int	retval;

	alloc_rec.st_fund = po_hdr.ph_funds ;
	STRCPY(alloc_rec.st_code, po_item.pi_st_code) ;
	alloc_rec.st_location = po_item.pi_school ;
	STRCPY(alloc_rec.st_expacc, po_item.pi_acct) ;

	if ((retval = get_alloc(&alloc_rec,UPDATE,0,e_mesg)) < 0) 
		return(retval);

	return(NOERROR) ;
}	/* CheckAllocation() */

DispMessage(s)
char	*s;
{
	printf("%s",s);
	roll_back(e_mesg);
}


OpenReport()
{

	char	chardate[11] ;
	char 	projname[50] ;
	char 	program[11] ;
	int 	outcntl ;
	char 	discfile[20] ;
	int	retval;

	outcntl = 2; 		/* Printer purge report to printer */
	discfile[0] = '\0';

	mkdate(get_date(),chardate);

	STRCPY(projname,FMT_PATH) ;
	strcat(projname,PROJNAME) ;
	STRCPY(program, PROG_NAME );

	retval = rpopen(projname,LOG_REC,FORMNO,outcntl,discfile,program,
			chardate);
							  	
	if ( retval < 0 ){
		printf("\n\nRpopen code :%d\n\n", retval ) ;
		return(-1);
	}

	rpChangetitle(1,pa_rec.pa_co_name);

#ifdef ENGLISH
	rpChangetitle(2,"PURCHASE ORDERS OUTSTANDING AT YEAREND");
#else
	rpChangetitle(2,"BONS DE COMMANDE NON-REGLES A LA FIN D'ANNEE");
#endif

	arayptr[0] = (char *)&supplier ;
	arayptr[1] = (char *)&po_hdr ;
        arayptr[2] = (char *)&po_item ;
        arayptr[3] = NULL ;

	return(0);
}

CloseRtn()
{
	close_file(POHDR);
	close_file(POITEM);
	close_file(GLMAST);
	close_file(SUPPLIER);
	close_file(CONTROL);
	close_file(ALLOCATION);
	rpclose();
}
