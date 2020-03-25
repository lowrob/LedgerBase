/******************************************************************************
		Sourcename    : apinvrep.c
		System        : Budgetary Financial system.
		Module        : Accounts Payable reports
		Created on    : 89-12-08
		Created  By   : Jonathan Prescott
		Cobol sources : cp050---04
*******************************************************************************
About this file:
	This routine generates one of the following :
		1. Accounts Payable Invoice Details Report 
	Calling file:	

*****************************************************************************

Modifications:

Programmer	YY/MM/DD	Description
~~~~~~~~~~	~~~~~~~~	~~~~~~~~~~~
C.Leadbeater	90/11/23	Changed 'BATCH' to 'SESSION' in report headings
				and moved fields as needed.
C.Leadbeater	90/11/26	Add 'TOTAL CONSUMPTION' line to report, added
				variables to store consumption totals.
C.Leadbeater	90/12/07	Add 'PO Complete?' line to report.
F.Tao		90/12/11	Add GST amount.
F.Tao		90/12/20	Change formats.
L.Robichaud	1993/10/13	Pass new parameter to close_rep function for 
				a banner to print with 3000 family.
******************************************************************************/

#define	EXIT	12		/* as defined in apreputl.c */

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>
#include <apinvc.h>
#include <repdef.h>

#define	DATE_FMT	"____/__/__"
#define PO_FMT		"______0_"
#define TAX_FMT		"__,_0_.__-"
#define DISCHB_FMT	"0_.__"
#define AMOUNT_FMT	"__,___,_0_.__-"
#define QTY_FMT		"___0_.____"
#define TOTAL_FMT	"___,___,_0_.__-"

#define HV_CHAR		'\377'

static int 	pgcnt;
static int	retval;

static char	prev_supp_cd[11];
static short	prev_funds;
static short	prev_period;

/* transaction total fields */

static short o_in_trans;
static short c_in_trans;
static short o_rt_trans;
static short c_rt_trans;
static short o_cm_trans;
static short c_cm_trans;
static short o_dm_trans;
static short c_dm_trans;
static short consmp_trans;	/* number of consumption (reccode=97) items */
static double o_in_trans_amt;
static double c_in_trans_amt;
static double o_rt_trans_amt;
static double c_rt_trans_amt;
static double o_cm_trans_amt;
static double c_cm_trans_amt;
static double o_dm_trans_amt;
static double c_dm_trans_amt;
static double o_in_disc_amt;
static double c_in_disc_amt;
static double o_rt_disc_amt;
static double c_rt_disc_amt;
static double o_cm_disc_amt;
static double c_cm_disc_amt;
static double o_dm_disc_amt;
static double c_dm_disc_amt;
static double consmp_trans_qty;		/* total consumption quantity */
static double total_gst_amt;
static double total_pst_amt;

static short go_in_trans;
static short gc_in_trans;
static short go_rt_trans;
static short gc_rt_trans;
static short go_cm_trans;
static short gc_cm_trans;
static short go_dm_trans;
static short gc_dm_trans;
static short gconsmp_trans;     /* grand tot. number consumption items */
static double go_in_trans_amt;
static double gc_in_trans_amt;
static double go_rt_trans_amt;
static double gc_rt_trans_amt;
static double go_cm_trans_amt;
static double gc_cm_trans_amt;
static double go_dm_trans_amt;
static double gc_dm_trans_amt;
static double go_in_disc_amt;
static double gc_in_disc_amt;
static double go_rt_disc_amt;
static double gc_rt_disc_amt;
static double go_cm_disc_amt;
static double gc_cm_disc_amt;
static double go_dm_disc_amt;
static double gc_dm_disc_amt;
static double gconsmp_trans_qty;   /* grand total consumption quantity */ 
static double gtotal_gst_amt;
static double gtotal_pst_amt;

/*---------------------------------------------------------------*/
ApInvcDetails()
{
	char	tnum[5];
	int 	first, i;

	/* always to printer */
	if(opn_prnt("P","\0",1,e_mesg,1) < 0) {
		DispError(e_mesg) ;
		return(ERROR) ;
	}

	/** set page size parameters **/
	pgcnt = 0;
	LNSZ = 133;
	linecnt = PGSIZE;

	o_in_trans = 0 ;
	c_in_trans = 0 ;
	o_rt_trans = 0 ;
	c_rt_trans = 0 ;
	o_cm_trans = 0 ;
	c_cm_trans = 0 ;
	o_dm_trans = 0 ;
	c_dm_trans = 0 ;
	consmp_trans = 0;
	o_in_trans_amt = 0 ;
	c_in_trans_amt = 0 ;
	o_rt_trans_amt = 0 ;
	c_rt_trans_amt = 0 ;
	o_cm_trans_amt = 0 ;
	c_cm_trans_amt = 0 ;
	o_dm_trans_amt = 0 ;
	c_dm_trans_amt = 0 ;
	o_in_disc_amt = 0 ;
	c_in_disc_amt = 0 ;
	o_rt_disc_amt = 0 ;
	c_rt_disc_amt = 0 ;
	o_cm_disc_amt = 0 ;
	c_cm_disc_amt = 0 ;
	o_dm_disc_amt = 0 ;
	c_dm_disc_amt = 0 ;
	consmp_trans_qty = 0;
	total_pst_amt = 0 ;
	total_gst_amt = 0 ;

	go_in_trans = 0 ;
	gc_in_trans = 0 ;
	go_rt_trans = 0 ;
	gc_rt_trans = 0 ;
	go_cm_trans = 0 ;
	gc_cm_trans = 0 ;
	go_dm_trans = 0 ;
	gc_dm_trans = 0 ;
	gconsmp_trans = 0;
	go_in_trans_amt = 0 ;
	gc_in_trans_amt = 0 ;
	go_rt_trans_amt = 0 ;
	gc_rt_trans_amt = 0 ;
	go_cm_trans_amt = 0 ;
	gc_cm_trans_amt = 0 ;
	go_dm_trans_amt = 0 ;
	gc_dm_trans_amt = 0 ;
	go_in_disc_amt = 0 ;
	gc_in_disc_amt = 0 ;
	go_rt_disc_amt = 0 ;
	gc_rt_disc_amt = 0 ;
	go_cm_disc_amt = 0 ;
	gc_cm_disc_amt = 0 ;
	go_dm_disc_amt = 0 ;
	gc_dm_disc_amt = 0 ;
	gconsmp_trans_qty = 0;
	gtotal_gst_amt = 0 ;
	gtotal_pst_amt = 0 ;

	get_tnum(tnum);
	STRCPY(in_hdr.h_term,tnum);
	in_hdr.h_batch = 0;
	in_hdr.h_sno = 0;

	prev_funds = 0;
	prev_period = 0;
	prev_supp_cd[0] = '\0';
	first = 0;

	flg_reset(APINHDR);
	for(;;) {
		retval = get_n_inhdr(&in_hdr,UPDATE,0,FORWARD,e_mesg);
		if(retval == ERROR) return(DBH_ERR) ;
		if(retval == EFL) break;
		if(retval < 0) {
			DispError(e_mesg);
			break;
		}
		if(strcmp(in_hdr.h_term,tnum) != 0 ) break;
		if(in_hdr.h_funds != prev_funds || 
				in_hdr.h_period != prev_period) {
			if(first != 0)  {
				if(linecnt+14 >= PGSIZE) {
					if(Print_Heading(0) < 0) break;
				}
				print_fund_totals();
			}
			else first = 1;	
			ctl_rec.fund = in_hdr.h_funds;
			retval = get_ctl(&ctl_rec,BROWSE,0,e_mesg);
			if(retval != NOERROR) {
				STRCPY(ctl_rec.desc,"??????????????");
			}	
			prev_funds = in_hdr.h_funds;   /* set current fund */
			prev_period = in_hdr.h_period; /* set current period */
			if(Print_Heading(0) < 0) break;
		}
		else if(linecnt >= PGSIZE ) {
			if(Print_Heading(0) < 0) break;
		}

		if(strcmp(in_hdr.h_supp_cd,prev_supp_cd) != 0) {
			STRCPY(supp_rec.s_supp_cd,in_hdr.h_supp_cd);	
			retval = get_supplier(&supp_rec,BROWSE,0,e_mesg);
			if(retval != NOERROR) {
				STRCPY(supp_rec.s_supp_cd,"??????????");
			}
			if(print_supplier_line() < 0) break;	
		}
		if(calc_invoice_totals() < 0){
			roll_back(e_mesg);
			break;
		}

		if(print_invoice_line() < 0) break;

		retval = put_inhdr(&in_hdr,P_DEL,e_mesg);
		if(retval < 0) {
			retval = DBH_ERR;
			break;
		}	

		STRCPY(in_item.i_supp_cd,in_hdr.h_supp_cd);
		STRCPY(in_item.i_invc_no,in_hdr.h_invc_no);
		STRCPY(in_item.i_tr_type,in_hdr.h_tr_type);
		in_item.i_item_no = 0;
		flg_reset(APINITEM);	

		for(i = 1 ; ; i++) {
#ifndef ORACLE
			retval = get_n_initem(&in_item,UPDATE,0,FORWARD,e_mesg);
#else
			retval = get_n_initem(&in_item,UPDATE,0,EQUAL,e_mesg);
#endif
			if(retval < 0) {
				if(retval == EFL)	break;
				DispError(e_mesg); 
				break;
			}

#ifndef ORACLE
			if(strcmp(in_item.i_tr_type,in_hdr.h_tr_type) != 0) 
				break;
			if(strcmp(in_item.i_invc_no,in_hdr.h_invc_no) != 0) 
				break;
			if(strcmp(in_item.i_supp_cd,in_hdr.h_supp_cd) != 0)
				break;
#endif
			if(linecnt >= PGSIZE) {
				if((retval = Print_Heading(0)) < 0) break;
				if((retval = print_supplier_line()) < 0) break;	
			}	
			if((retval = print_item_line(i)) < 0) break;

				/* Accumulate ordinary consumption totals */

			if(in_item.i_97accno[0] != '\0'){
				consmp_trans++;
				consmp_trans_qty += in_item.i_qty;
				}

			retval = put_initem(&in_item,P_DEL,e_mesg);
			if(retval < 0) {
				retval = DBH_ERR;
				break;
			}	
			/* Increment the last part of the key */
			in_item.i_item_no++ ;
			flg_reset(APINITEM);	

		}
		if(retval == REPORT_ERR || retval == DBH_ERR) 
			roll_back(e_mesg);
		else {
			if(commit(e_mesg) < 0) {
				retval = DBH_ERR;
				break;
			}
		}
		/* Increment the last part of the key */
		in_hdr.h_sno++ ;
	}
	roll_back(e_mesg);
	if(first) {
		if(linecnt+14 >= PGSIZE) {
			retval = Print_Heading(0);  
		}
		print_fund_totals();

		retval = Print_Heading(1);
		grand_fund_totals();
#ifndef	SPOOLER
	rite_top() ;
#endif
	}
	close_rep(BANNER) ;
	return(0);
}

Print_Heading(mode)
int mode;
{
	char	txt_line[80];
	long	longdate ;
	int	i ;

	linecnt = 0;
	
	if(pgcnt) rite_top();   /* if not first page advance */

	mkln(1,PROG_NAME,10);
	i = strlen(pa_rec.pa_co_name);
	mkln(((LNSZ - 1 - i) / 2)+1,pa_rec.pa_co_name, sizeof(pa_rec.pa_co_name));
	longdate = get_date();
#ifdef ENGLISH
        mkln(115,"DATE:",5);
#else
        mkln(115,"DATE:",5);
#endif
	tedit((char *)&longdate,"____/__/__",txt_line,R_LONG);
	mkln(121,txt_line,10);
	if(prnt_line() < 0) return(ERROR);
	
#ifdef ENGLISH
	mkln(49,"ACCOUNTS PAYABLE  -  INVOICE DETAILS",36);
#else
	mkln(47,"COMPTES PAYABLES  -  DETAILS DES FACTURES",41);
#endif
	pgcnt++;
#ifdef ENGLISH
	mkln(115,"PAGE:",5);
#else
	mkln(115,"PAGE:",5);
#endif
	tedit((char *)&pgcnt,"__0_",txt_line,R_INT);
	mkln(127,txt_line,4);
	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);

	if(mode == 0) {
#ifdef ENGLISH
		mkln(1,"PERIOD:",7);
#else
		mkln(1,"PERIODE:",8);
#endif
		tedit((char *)&in_hdr.h_period,"0_",txt_line,R_SHORT);
		mkln(10,txt_line,2);
#ifdef ENGLISH
		mkln(14,"FUND:",5);
#else
		mkln(14,"FOND:",5);
#endif
		tedit((char *)&in_hdr.h_funds,"_0_",txt_line,R_SHORT);
		mkln(21,txt_line,3);
		mkln(26,ctl_rec.desc,30);	
		if(prnt_line() < 0) return(ERROR);
		if(prnt_line() < 0) return(ERROR);
	}
	
#ifdef ENGLISH
	mkln(1,"TRAN REF#",9);
	mkln(12,"T_TYPE",6);
	mkln(20,"DESCRIPTION",11);
	mkln(35,"PO#",3);
	mkln(40,"SESSION",7);
	mkln(49,"TYPE",4);
 	mkln(54,"TRAN DATE",9);
	mkln(65,"DUE DATE",8);
	mkln(76,"HB/DISC%",8);
	mkln(85,"HB/DISC AMT",11);
	mkln(100,"  GST",7);
	mkln(111,"  PST",7);
	mkln(121,"GROSS AMT",9);
#else
	mkln(1,"#REF TRANS",10);
	mkln(13,"GENRE_T",7);
	mkln(22,"DESCRIPTN",9);
	mkln(33,"#BC",3);
	mkln(38,"SESSION",7);
	mkln(47,"GENRE",5);
 	mkln(54,"DATE TRANS",10);
	mkln(66,"ECHEANCE",8);
	mkln(76,"PR/%ESCPTE",10);
	mkln(88,"MONT.PR/ESCPTE",14);
	mkln(104,"   TPS",8);
	mkln(114,"   TVP",8);
	mkln(124,"MONT.BRUT",9);
#endif
	if(prnt_line() < 0) return(ERROR);
	
#ifdef ENGLISH
	mkln(4,"PO COMPLETED?",13);
	mkln(21,"CHEQUE#",7);
	mkln(40,"BANK ACCT",9);
	mkln(54,"STOCK CODE",10);
	mkln(74,"99 GL ACCT",10);
	mkln(95,"97 GL ACCT",10);
	mkln(108,"QUANTITY",8);
	mkln(123,"ITEM AMT",8);
#else
	mkln(4,"BC COMPLETE?",12);
	mkln(21,"#DE CHEQUE",10);
	mkln(40,"COMPTE BANQUE",13);
	mkln(54,"CD DE STOCK",11);
	mkln(74,"COMPTE GL 99",12);
	mkln(95,"COMPTE GL 97",12);
	mkln(108,"QUANTITE",8);
	mkln(121,"MONT D'ART.",12);
#endif
	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);

	return(NOERROR);
}

print_supplier_line()
{
	/* cur_pos is the position on the line where the next character */
	/* is to be printed.  cur_pos is internally incremented by the  */
	/* mkln function.						*/

#ifdef ENGLISH
	mkln(1,"SUPPLIER:",9);
#else
	mkln(1,"FOURNISSEUR:",12);
#endif
	mkln(11,supp_rec.s_supp_cd,strlen(supp_rec.s_supp_cd));
	mkln(cur_pos+3,supp_rec.s_name,strlen(supp_rec.s_name));
#ifdef ENGLISH
	mkln(cur_pos+4,"TYPE:",5);
#else
	mkln(cur_pos+4,"GENRE:",6);
#endif
	mkln(cur_pos+2,supp_rec.s_type,1);
	if(prnt_line() < 0) return(ERROR);

	STRCPY(prev_supp_cd,supp_rec.s_supp_cd);   /* setup current supplier */
	
	return(NOERROR);
}	

print_invoice_line()
{
	char txt_line[80];

	if(prnt_line() < 0) return(ERROR);
	mkln(1,in_hdr.h_invc_no,15);
	mkln(17,in_hdr.h_tr_type,2);	
	mkln(20,in_hdr.h_remarks,12);
	tedit((char *)&in_hdr.h_po_no,PO_FMT,txt_line,R_LONG);
	mkln(33,txt_line,8);
	tedit((char *)&in_hdr.h_batch,"_0_",txt_line,R_SHORT);
	mkln(43,txt_line,3);
	mkln(50,in_hdr.h_type,1);
	tedit((char *)&in_hdr.h_invc_dt,DATE_FMT,txt_line,R_LONG);
	mkln(54,txt_line,10);
	tedit((char *)&in_hdr.h_due_dt,DATE_FMT,txt_line,R_LONG);
	mkln(65,txt_line,10);
	tedit((char *)&in_hdr.h_disc_per,DISCHB_FMT,txt_line,R_FLOAT);	
	mkln(76,txt_line,5);
	tedit((char *)&in_hdr.h_disc_amt,AMOUNT_FMT,txt_line,R_DOUBLE);
	mkln(82,txt_line,14);
	tedit((char *)&in_hdr.h_gsttax,TAX_FMT,txt_line,R_DOUBLE);
	mkln(97,txt_line,10);
	tedit((char *)&in_hdr.h_psttax,TAX_FMT,txt_line,R_DOUBLE);
	mkln(108,txt_line,10);
	tedit((char *)&in_hdr.h_amount,AMOUNT_FMT,txt_line,R_DOUBLE);
	mkln(118,txt_line,14);
	if(prnt_line() < 0) return(ERROR);

	return(NOERROR);
}

calc_invoice_totals()
{
	if(supp_rec.s_type[0] == CONTRACT) {	/* Contract supplier */
		if(strcmp(in_hdr.h_tr_type,T_INVOICE) == 0){ /* invoice */
			c_in_trans++;
			c_in_trans_amt += in_hdr.h_amount;
			c_in_disc_amt += in_hdr.h_disc_amt;
		}
		else if(strcmp(in_hdr.h_tr_type,T_RETURN) == 0){ /* return */
				c_rt_trans++;
				c_rt_trans_amt += in_hdr.h_amount;
				c_rt_disc_amt += in_hdr.h_disc_amt;
		}
		else if(strcmp(in_hdr.h_tr_type,T_CRMEMO)==0){ /* credit memo */
				c_cm_trans++;
				c_cm_trans_amt += in_hdr.h_amount;
				c_cm_disc_amt += in_hdr.h_disc_amt;
		}
		else if(strcmp(in_hdr.h_tr_type,T_DBMEMO)==0){ /* debit memo */
				c_dm_trans++;
				c_dm_trans_amt += in_hdr.h_amount;
				c_dm_disc_amt += in_hdr.h_disc_amt;
		}
		else { 
			STRCPY(in_hdr.h_tr_type,"??");
		}
	}
	else {					/* ordinary suppier */
		if(strcmp(in_hdr.h_tr_type,T_INVOICE) == 0) {      /* invoice */
			o_in_trans++;
			o_in_trans_amt += in_hdr.h_amount;
			o_in_disc_amt += in_hdr.h_disc_amt;
		}
		else if(strcmp(in_hdr.h_tr_type,T_RETURN)==0) { /* return */
				o_rt_trans++;
				o_rt_trans_amt += in_hdr.h_amount;
				o_rt_disc_amt += in_hdr.h_disc_amt;
		}
		else if(strcmp(in_hdr.h_tr_type,T_CRMEMO)==0){ /* credit memo */
				o_cm_trans++;
				o_cm_trans_amt += in_hdr.h_amount;
				o_cm_disc_amt += in_hdr.h_disc_amt;
		}
		else if(strcmp(in_hdr.h_tr_type,T_DBMEMO)==0){ /* debit memo */
				o_dm_trans++;
				o_dm_trans_amt += in_hdr.h_amount;
				o_dm_disc_amt += in_hdr.h_disc_amt;
		}
		else { 
			STRCPY(in_hdr.h_tr_type,"??");
		}
	}

	total_pst_amt += in_hdr.h_psttax;
	total_gst_amt += in_hdr.h_gsttax;
	return(NOERROR);
}

print_item_line(item_no)
int	item_no ;
{
	char txt_line[80];

		/* print PO Complete flag only if PO# not 0 */
	
	if( in_hdr.h_po_no != 0 )
		mkln(10,in_hdr.h_po_cmp,1);

	if(item_no == 1 && in_hdr.h_chq_no) {		/* First Item */
		tedit((char *)&in_hdr.h_chq_no,"_______0",txt_line,R_LONG);
		mkln(20,txt_line,8);	
		mkln(31,in_hdr.h_accno,18);
	}
	if(in_item.i_stck_cd[0] != HV_CHAR) 
		mkln(54,in_item.i_stck_cd,10);
	mkln(65,in_item.i_99accno,18);
	mkln(84,in_item.i_97accno,18);
	tedit((char *)&in_item.i_qty,QTY_FMT,txt_line,R_DOUBLE);
	mkln(103,txt_line,10);
	tedit((char *)&in_item.i_value,AMOUNT_FMT,txt_line,R_DOUBLE);
	mkln(116,txt_line,14);
	if(prnt_line() < 0) return(ERROR);
	
	return(NOERROR);
}

print_fund_totals()
{
	char txt_line[80];
	double  total;
	
	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);
#ifdef ENGLISH
	mkln(2,"FUND",4);
#else
	mkln(2,"FOND",4);
#endif
	tedit((char *)&in_hdr.h_funds,"_0_",txt_line,R_SHORT);
	mkln(8,txt_line,3);
#ifdef ENGLISH
	mkln(20,"G/L ACCOUNT ORDINARY",20);
#else
	mkln(20,"COMPTE G/L ORDINAIRE",20);
#endif
	total = o_in_trans_amt + o_rt_trans_amt + o_cm_trans_amt + 
			o_dm_trans_amt;	
	mkln(45,ctl_rec.ap_gen_acnt,18);
	tedit((char *)&total,TOTAL_FMT,txt_line,R_DOUBLE);
	mkln(70,txt_line,15);
	if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
	mkln(20,"G/L ACCOUNT CONTRACT",20);
#else
	mkln(20,"COMPTE G/L CONTRACTUEL",22);
#endif
	mkln(45,ctl_rec.ap_cnt_acnt,18);
	total = c_in_trans_amt + c_rt_trans_amt + c_cm_trans_amt + 
			c_dm_trans_amt;	
	tedit((char *)&total,TOTAL_FMT,txt_line,R_DOUBLE);
	mkln(70,txt_line,15);
	if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
	mkln(20,"G/L ACCOUNT PST",19);
#else
	mkln(20,"TVP AU COMPTE G/L",22);
#endif
	mkln(45,ctl_rec.pst_tax_acnt,18);
	tedit((char *)&total_pst_amt,TOTAL_FMT,txt_line,R_DOUBLE);
	mkln(70,txt_line,15);
	if(prnt_line() < 0) return(ERROR);
#ifdef ENGLISH
	mkln(20,"G/L ACCOUNT GST ",19);
#else
	mkln(20,"TPS AU COMPTE G/L",22);
#endif
	mkln(45,ctl_rec.gst_tax_acnt,18);
	tedit((char *)&total_gst_amt,TOTAL_FMT,txt_line,R_DOUBLE);
	mkln(70,txt_line,15);
	if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
	mkln(20,"G/L ACCOUNT DISCOUNT",20);
#else
	mkln(20,"ESCOMPTE COMPTE DU G/L",22);
#endif
	mkln(45,ctl_rec.dis_acnt,18);
	total = -(c_in_disc_amt + c_rt_disc_amt + c_cm_disc_amt + 
		  c_dm_disc_amt + o_in_disc_amt + o_rt_disc_amt + 
		  o_cm_disc_amt + o_dm_disc_amt);	
	tedit((char *)&total,TOTAL_FMT,txt_line,R_DOUBLE);
	mkln(70,txt_line,15);
	if(prnt_line() < 0) return(ERROR);

	if(prnt_line() < 0) return(ERROR);
	
#ifdef ENGLISH
	mkln(10,"TRANSACTION TOTALS",18);
#else
	mkln(10,"TOTAUX DES TRANSACTIONS",23);
#endif
	if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
	mkln(40,"ORDINARY TRANSACTIONS",21);
#else
	mkln(40,"TRANSACTIONS ORDINAIRES",23);
#endif
#ifdef ENGLISH
	mkln(80,"CONTRACT TRANSACTIONS",21);
#else
	mkln(78,"TRANSACTIONS CONTRACTUELLES",27);
#endif
	if(prnt_line() < 0) return(ERROR);
	
#ifdef ENGLISH
	mkln(10,"TYPE OF RECORD",14);
#else
	mkln(10,"GENRE DE FICHE",14);
#endif
#ifdef ENGLISH
	mkln(40,"NUMBER",6);
#else
	mkln(40,"NOMBRE",6);
#endif
#ifdef ENGLISH
	mkln(55,"AMOUNT",6);
#else
	mkln(55,"MONTANT",7);
#endif
#ifdef ENGLISH
	mkln(80,"NUMBER",6);
#else
	mkln(80,"NOMBRE",6);
#endif
#ifdef ENGLISH
	mkln(95,"AMOUNT",6);
#else
	mkln(95,"MONTANT",7);
#endif
	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
	mkln(10,"1 INVOICE",9);
#else
	mkln(10,"1 FACTURE",9);
#endif
	tedit((char *)&o_in_trans,"_0_",txt_line,R_SHORT);
	mkln(41,txt_line,3);
	total = o_in_trans_amt - o_in_disc_amt ;
	tedit((char *)&total,TOTAL_FMT,txt_line,R_DOUBLE);
	mkln(47,txt_line,15);
	tedit((char *)&c_in_trans,"_0_",txt_line,R_SHORT);
	mkln(81,txt_line,3);
	total = c_in_trans_amt - c_in_disc_amt ;
	tedit((char *)&total,TOTAL_FMT,txt_line,R_DOUBLE);
	mkln(87,txt_line,15);
	if(prnt_line() < 0) return(ERROR);
	
#ifdef ENGLISH
	mkln(10,"2 RETURNS",9);
#else
	mkln(10,"2 RENVOIS",9);
#endif
	tedit((char *)&o_rt_trans,"_0_",txt_line,R_SHORT);
	mkln(41,txt_line,3);
	total = o_rt_trans_amt - o_rt_disc_amt ;
	tedit((char *)&total,TOTAL_FMT,txt_line,R_DOUBLE);
	mkln(47,txt_line,15);
	tedit((char *)&c_rt_trans,"_0_",txt_line,R_SHORT);
	mkln(81,txt_line,3);
	total = c_rt_trans_amt - c_rt_disc_amt ;
	tedit((char *)&total,TOTAL_FMT,txt_line,R_DOUBLE);
	mkln(87,txt_line,15);
	if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
	mkln(10,"3 CREDIT MEMOS",14);
#else
	mkln(10,"3 NOTES DE CREDIT",17);
#endif
	tedit((char *)&o_cm_trans,"_0_",txt_line,R_SHORT);
	mkln(41,txt_line,3);
	total = o_cm_trans_amt - o_cm_disc_amt ;
	tedit((char *)&total,TOTAL_FMT,txt_line,R_DOUBLE);
	mkln(47,txt_line,15);
	tedit((char *)&c_cm_trans,"_0_",txt_line,R_SHORT);
	mkln(81,txt_line,3);
	total = c_cm_trans_amt - c_cm_disc_amt ;
	tedit((char *)&total,TOTAL_FMT,txt_line,R_DOUBLE);
	mkln(87,txt_line,15);
	if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
	mkln(10,"4 DEBIT MEMOS",14);
#else
	mkln(10,"4 NOTES DE DEBIT",17);
#endif
	tedit((char *)&o_dm_trans,"_0_",txt_line,R_SHORT);
	mkln(41,txt_line,3);
	total = o_dm_trans_amt - o_dm_disc_amt ;
	tedit((char *)&total,TOTAL_FMT,txt_line,R_DOUBLE);
	mkln(47,txt_line,15);
	tedit((char *)&c_dm_trans,"_0_",txt_line,R_SHORT);
	mkln(81,txt_line,3);
	total = c_dm_trans_amt - c_dm_disc_amt ;
	tedit((char *)&total,TOTAL_FMT,txt_line,R_DOUBLE);
	mkln(87,txt_line,15);
	if(prnt_line() < 0) return(ERROR);
	

	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);
#ifdef ENGLISH
	mkln(10,"TOTAL CONSUMPTION",17);
	mkln(59,"NUMBER",6);
	mkln(72,"QUANTITY",8);
#else
	mkln(10,"CONSOMMATION TOTALE",19);
	mkln(59,"NOMBRE",6);
	mkln(72,"QUANTITE",8);
#endif
	if(prnt_line() < 0) return(ERROR);

	tedit((char *)&consmp_trans,"_0_",txt_line,R_SHORT);
	mkln(60,txt_line,3);
	tedit((char *)&consmp_trans_qty,TOTAL_FMT,txt_line,R_DOUBLE);
	mkln(70,txt_line,15);
	if(prnt_line() < 0) return(ERROR);


	/* setup run totals (i.e. grand totals )*/
	go_in_trans += o_in_trans;
	gc_in_trans += c_in_trans;
	go_rt_trans += o_rt_trans;
	gc_rt_trans += c_rt_trans;
	go_cm_trans += o_cm_trans;
	gc_cm_trans += c_cm_trans;
	go_dm_trans += o_dm_trans;
	gc_dm_trans += c_dm_trans;
	gconsmp_trans += consmp_trans;
	go_in_trans_amt += o_in_trans_amt;
	gc_in_trans_amt += c_in_trans_amt;
	go_rt_trans_amt += o_rt_trans_amt;
	gc_rt_trans_amt += c_rt_trans_amt;
	go_cm_trans_amt += o_cm_trans_amt;
	gc_cm_trans_amt += c_cm_trans_amt;
	go_dm_trans_amt += o_dm_trans_amt;
	gc_dm_trans_amt += c_dm_trans_amt;
	go_in_disc_amt += o_in_disc_amt;
	gc_in_disc_amt += c_in_disc_amt;
	go_rt_disc_amt += o_rt_disc_amt;
	gc_rt_disc_amt += c_rt_disc_amt;
	go_cm_disc_amt += o_cm_disc_amt;
	gc_cm_disc_amt += c_cm_disc_amt;
	go_dm_disc_amt += o_dm_disc_amt;
	gc_dm_disc_amt += c_dm_disc_amt;
	gconsmp_trans_qty += consmp_trans_qty;
	gtotal_gst_amt += total_gst_amt;
	gtotal_pst_amt += total_pst_amt;

	/* after fund totals printed initalize to 0 for next fund */
	o_in_trans = c_in_trans = o_rt_trans = c_rt_trans = 0;
	o_cm_trans = c_cm_trans = o_dm_trans = c_dm_trans = 0;
	o_in_trans_amt = c_in_trans_amt = o_rt_trans_amt = c_rt_trans_amt = 0;
	o_cm_trans_amt = c_cm_trans_amt = o_dm_trans_amt = c_dm_trans_amt = 0;
	o_in_disc_amt = c_in_disc_amt = o_rt_disc_amt = c_rt_disc_amt = 0;
	o_cm_disc_amt = c_cm_disc_amt = o_dm_disc_amt = c_dm_disc_amt = 0;
	total_pst_amt = 0;
	total_gst_amt = 0;

	return(NOERROR);
}

grand_fund_totals()
{
	char txt_line[80];
	double	total;

	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
	mkln(10,"RUN TRANSACTION TOTALS",21);
#else
	mkln(10,"TOTAUX FINAUX DES TRANCTIONS",29);
#endif
	if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
	mkln(40,"ORDINARY TRANSACTIONS",21);
	mkln(80,"CONTRACT TRANSACTIONS",21);
#else
	mkln(40,"TRANSACTIONS ORDINAIRES",23);
	mkln(80,"TRANSACTIONS CONTRACTUELLES",27);
#endif
	if(prnt_line() < 0) return(ERROR);
	
#ifdef ENGLISH
	mkln(10,"TYPE OF RECORD",14);
	mkln(40,"NUMBER",6);
	mkln(55,"AMOUNT",6);
	mkln(80,"NUMBER",6);
	mkln(95,"AMOUNT",6);
#else
	mkln(10,"GENRE DE FICHE",14);
	mkln(40,"NOMBRE",6);
	mkln(55,"MONTANT",7);
	mkln(80,"NOMBRE",6);
	mkln(95,"MONTANT",7);
#endif
	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
	mkln(10,"1 INVOICE",9);
#else
	mkln(10,"1 FACTURE",9);
#endif
	tedit((char *)&go_in_trans,"_0_",txt_line,R_SHORT);
	mkln(41,txt_line,3);
	total = go_in_trans_amt - go_in_disc_amt ;
	tedit((char *)&total,TOTAL_FMT,txt_line,R_DOUBLE);
	mkln(47,txt_line,15);
	tedit((char *)&gc_in_trans,"_0_",txt_line,R_SHORT);
	mkln(81,txt_line,3);
	total = gc_in_trans_amt - gc_in_disc_amt ;
	tedit((char *)&total,TOTAL_FMT,txt_line,R_DOUBLE);
	mkln(87,txt_line,15);
	if(prnt_line() < 0) return(ERROR);
	
#ifdef ENGLISH
	mkln(10,"2 RETURNS",9);
#else
	mkln(10,"2 RENVOIS",9);
#endif
	tedit((char *)&go_rt_trans,"_0_",txt_line,R_SHORT);
	mkln(41,txt_line,3);
	total = go_rt_trans_amt - go_rt_disc_amt ;
	tedit((char *)&total,TOTAL_FMT,txt_line,R_DOUBLE);
	mkln(47,txt_line,15);
	tedit((char *)&gc_rt_trans,"_0_",txt_line,R_SHORT);
	mkln(81,txt_line,3);
	total = gc_rt_trans_amt - gc_rt_disc_amt ;
	tedit((char *)&total,TOTAL_FMT,txt_line,R_DOUBLE);
	mkln(87,txt_line,15);
	if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
	mkln(10,"3 CREDIT MEMOS",14);
#else
	mkln(10,"3 NOTES DE CREDIT",17);
#endif
	tedit((char *)&go_cm_trans,"_0_",txt_line,R_SHORT);
	mkln(41,txt_line,3);
	total = go_cm_trans_amt - go_cm_disc_amt ;
	tedit((char *)&total,TOTAL_FMT,txt_line,R_DOUBLE);
	mkln(47,txt_line,15);
	tedit((char *)&gc_cm_trans,"_0_",txt_line,R_SHORT);
	mkln(81,txt_line,3);
	total = gc_cm_trans_amt - gc_cm_disc_amt ;
	tedit((char *)&total,TOTAL_FMT,txt_line,R_DOUBLE);
	mkln(87,txt_line,15);
	if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
	mkln(10,"4 DEBIT MEMOS",14);
#else
	mkln(10,"4 NOTES DE DEBIT",17);
#endif
	tedit((char *)&go_dm_trans,"_0_",txt_line,R_SHORT);
	mkln(41,txt_line,3);
	total = go_dm_trans_amt - go_dm_disc_amt ;
	tedit((char *)&total,TOTAL_FMT,txt_line,R_DOUBLE);
	mkln(47,txt_line,15);
	tedit((char *)&gc_dm_trans,"_0_",txt_line,R_SHORT);
	mkln(81,txt_line,3);
	total = gc_dm_trans_amt - gc_dm_disc_amt ;
	tedit((char *)&total,TOTAL_FMT,txt_line,R_DOUBLE);
	mkln(87,txt_line,15);
	if(prnt_line() < 0) return(ERROR);

	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
	mkln(10,"TOTAL CONSUMPTION",17);
	mkln(59,"NUMBER",6);
	mkln(72,"QUANTITY",8);
#else
	mkln(10,"CONSOMMATION TOTALE",19);
	mkln(59,"NOMBRE",6);
	mkln(72,"QUANTITE",8);
#endif
	if(prnt_line() < 0) return(ERROR);
	
	tedit((char *)&gconsmp_trans,"_0_",txt_line,R_SHORT);
	mkln(60,txt_line,3);
	tedit((char *)&gconsmp_trans_qty,TOTAL_FMT,txt_line,R_DOUBLE);
	mkln(70,txt_line,15);
	if(prnt_line() < 0) return(ERROR);

	return(NOERROR);
}
