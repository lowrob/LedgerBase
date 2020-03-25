/*-----------------------------------------------------------------------
Source Name: suppytd.c
System     : Budgetary Financial system.
Module     : General Ledger system.
Created  On: 15th Sept 93.
Created  By: Sherry Whittaker.

DESCRIPTION:

MODIFICATIONS:        

------------------------------------------------------------------------*/
#define  MAIN
#define  MAINFL		SUPPLIER		/* main file used */

#include <stdio.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	SYSTEM		"PURCHASE ORDER"	/* Sub System Name */
#define	MOD_DATE	"15-SEP-93"		/* Program Last Modified */

#define	RETURN		"RT"
#define	INVOICE		"IN"
#define	PER1START	19930721

static  Supplier supplier, pre_rec, payee_rec;	/* Supplier Master Record */
static 	Pa_rec	pa_rec;
static	Invoice	in_rec;
static 	Po_hdr	po_hdr;
static	Ap_hist	ap_hist;

static	char 	e_mesg[80];  		/* dbh will return err msg in this */

/*-------------------------------------------------------------------------*/
main(argc,argv)
int argc;
char *argv[];
{
	strncpy(SYS_NAME,SYSTEM,50);	/* Sub system name */
	strncpy(CHNG_DATE,MOD_DATE,10);	/* Modification Date */
	proc_switch(argc, argv, MAINFL) ; 	/* Process Switches */

	if ( Process() < 0) { 		/* Initiate Process */
		Close();
		exit(-1);
	}

	Close();			/* return to menu */
	exit(NOERROR);

} /* END OF MAIN */

/*-------------------------------------------------------------------*/
/* Reset information */
Close()
{
	/* Set terminal back to normal mode from PROFOM */
	fomcs();
	fomrt();
	close_dbh();			/* Close files */
	free_audit();
}
/*-------------------------------------------------------------------*/
/* Process the information to update supplier YEAR TO DATE totals    */
Process()
{
	int	retval;
	short	counter=0;
	double	ytd_order=0;
	double	ytd_received=0;
	double	ytd_returned=0;
	double	ytd_discount=0;
	double	cur_balance=0;

	retval = get_param(&pa_rec, BROWSE, 1, e_mesg) ;
	if(retval < 1) {
		close_dbh() ;
		return(DBH_ERR) ;
	}

	supplier.s_supp_cd[0] = '\0' ;
	flg_reset( SUPPLIER );
	for( ; ;inc_str(supplier.s_supp_cd,sizeof(supplier.s_supp_cd)-1,FORWARD)){
		ytd_order=0;
		ytd_received=0;
		ytd_returned=0;
		ytd_discount=0;
		cur_balance=0;
		counter++;	
		printf("\nUpdate supplier %d",counter);
		retval = get_n_supplier(&supplier,BROWSE,0,FORWARD,e_mesg);
		if( retval < 0) {
			if(retval == EFL) break ;
			retval = DBH_ERR;
			break ;
		}
		STRCPY(in_rec.in_supp_cd, supplier.s_supp_cd);
		in_rec.in_invc_no[0] = '\0' ;
		in_rec.in_tr_type[0] = '\0';
		flg_reset(APINVOICE) ;
		for( ; ; ){
			retval=get_n_invc(&in_rec,BROWSE,0,FORWARD,e_mesg);
			if (retval < 0){     		/*  */
				if(retval == EFL) break ;
				retval = DBH_ERR;
				return(ERROR);
			}
			if(strcmp(in_rec.in_supp_cd, supplier.s_supp_cd)!=0)
				break;
			if(in_rec.in_invc_dt >= PER1START)
				cur_balance += in_rec.in_amount;
		}
		STRCPY(po_hdr.ph_supp_cd, supplier.s_supp_cd);
		po_hdr.ph_code = 0;
		flg_reset(POHDR) ;
		for( ; ; ){
			retval=get_n_pohdr(&po_hdr,BROWSE,1,FORWARD,e_mesg);
			if (retval < 0){ 		/* */
				if(retval == EFL) break ;
				retval = DBH_ERR;
				return(ERROR);
			}
			if(strcmp(po_hdr.ph_supp_cd, supplier.s_supp_cd) !=0)
				break;
			if(po_hdr.ph_date >= PER1START){
				ytd_order+=po_hdr.ph_comm;
			}
		}
		STRCPY(ap_hist.a_supp_cd, supplier.s_supp_cd) ;
		ap_hist.a_invc_no[0] = '\0';
		ap_hist.a_tr_type[0] = '\0';
		ap_hist.a_sno = 0 ;
		flg_reset(APHIST) ;
		for ( ; ; ) {
			retval=get_n_aphist(&ap_hist,BROWSE,0,FORWARD,e_mesg) ;
			if (retval < 0){ 		/* */
				if(retval == EFL) break ;
				retval = DBH_ERR;
				return(ERROR);
			}
			if(strcmp(ap_hist.a_supp_cd, supplier.s_supp_cd)!=0)
				break;
			if(ap_hist.a_tr_date >= PER1START){
				if(strncmp(ap_hist.a_tr_type, RETURN,2) == 0)
					ytd_returned += ap_hist.a_gr_amt;
				if(strncmp(ap_hist.a_tr_type, INVOICE,2) == 0)
					ytd_received += ap_hist.a_gr_amt;
				ytd_discount += ap_hist.a_disc_taken;
			}
	
		}
		retval = get_supplier(&supplier,UPDATE,0,e_mesg);
		if( retval < 0) {
			printf("\n%s",e_mesg);
			if(retval == EFL) break ;
			retval = DBH_ERR;
			break ;
		}
		supplier.s_ytd_ret = ytd_returned;
		supplier.s_ytd_recpt = ytd_received;
		supplier.s_ytd_ord = ytd_order;
		supplier.s_balance = cur_balance;
		supplier.s_ytd_disc = ytd_discount;
		
		retval = put_supplier(&supplier,UPDATE,0,e_mesg);
		if( retval < 0) {
			printf("\n%d",retval);
			printf("\n%s",e_mesg);
			if(retval == EFL) break ;
			retval = DBH_ERR;
			break ;
		}
		retval = commit();
		if(retval <0){
			printf("\n Supplier file %s",e_mesg);
			if(retval == EFL) break ;
			retval = DBH_ERR;
			break ;
		}
	}
	return(NOERROR);
}
/*---------------------------END OF PROGRAM--------------------------------*/
