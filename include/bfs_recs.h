/*
*    Source Name : bfs_recs.h
*    System      : Budgetary Financial System.
*
*    Created On  : 2nd May 1989.
*
*    Contains Structure/Record Definitions used in this system.
*/


#include <bfs_com.h>

/*
*	section codes for accounts
*/

#define	ASSETS		1
#define	LIABILITIES	2
#define	EXPENDITURE	3
#define	INCOME		4

/*
*	PO, AP Invoice etc. type codes
*/

#ifdef ENGLISH
#define	OPEN		'O'	/* PO, Invoice status */
#define COMPLETE	'C'
#define PARTIAL		'P'

#define REL_HB		'R'	/* Invoice Payment Codes */
#define STOPPMT		'S'

#define	DIRECT		'D'	/* PO, Invoice Types */
#define	BULK		'B'
#define NON_BULK	'N'

#define REGULAR		'R'	/* Cheque types and status */
#define MANUAL		'M'
#define CANCELLED	'X'
#define OUTSTANDING	'O'
#define CASHED		'C'

#define CONTRACT	'C'	/* supplier type */
#define ORDINARY	'O'

#define APPLIED		'A'	/* applied or unapplied receipts */
#define UNAPPLIED	'U'
#else
#define	OPEN		'O'	/* PO, Invoice status */
#define COMPLETE	'C'
#define PARTIAL		'P'

#define REL_HB		'R'	/* Invoice Payment Codes */
#define STOPPMT		'A'

#define	DIRECT		'C'	/* PO, Invoice Types */
#define	BULK		'E'
#define NON_BULK	'S'

#define REGULAR		'R'	/* Cheque types and status */
#define MANUAL		'M'
#define CANCELLED	'X'
#define OUTSTANDING	'N'
#define CASHED		'E'

#define CONTRACT	'C'	/* supplier type */
#define ORDINARY	'O'

#define APPLIED		'A'	/* applied or unapplied receipts */
#define UNAPPLIED	'N'
#endif


/*
*	Structure/record definition of GST Distribution file ..
*
*	File Type: ISAM. No of Isam keys are 4.
*
*	Main key : fund + gst_accno + accno
*	Alt1 key : fund + accno 
*/

typedef	struct {
	short	fund ;			/* Funds PIC 999 */
	char	gst_accno[19] ;		/* GST Account number */
	char	accno[19] ;		/* Distribution Account number */
} GST_dist ;

/*
*	Structure/record definition of Recurring Entry Header file
*
*	The header record contains header information for a recurring
*	entry.
*
*	File Type: ISAM. No of Isam keys are : 1.
*
*	Main key : re_fund + re_sno.
*/

typedef struct {
	short	rh_fund ;		/* Fund Code of Entry */
	short	rh_sno ;		/* Running Sno under Fund */

	char	rh_descr[25] ;		/* Description */
	short	rh_reccod ;		/* Record code. 97,98 or 99 */
	char	rh_type[1] ;		/* Recurring type. Daily, Weekly,
					   Biweekly, Monthly, Quarterly,
					   Half Yearly or Yearly */
	double  rh_debits ;		/* Optional .. Sum of all -ve amnts */
	double  rh_credits ;		/* Optional .. Sum of all +ve amnts */
	long	rh_date ;		/* Last Posted Date */
	short	rh_speriod;		/* Starting Period for Posting */
	short	rh_lperiod;		/* Last Posted Period */

} Re_hdr ;


/*
*	Structure/record definition of Recurring Entry Item file
*
*	The item file contains item details of a recurring entry.
*	Header record and item records make one recurring entry.
*
*	The main key is same as header appended by line sequence
*	number to ensure uniqueness of all line items .
*
*	File Type: ISAM. No of Isam keys are : 1.
*
*	Main key : ri_fund + ri_sno + ri_item_no.
*/

typedef struct {
	short	ri_fund ;		/* Fund Code of Entry */
	short	ri_sno ;		/* Running Sno under Fund */
	short	ri_item_no ;		/* Item No. */
	double	ri_amount ;		/* Transaction Amount */
	char	ri_accno[19] ;		/* Account code */
} Re_item ;

/*
*		Budget transaction file .. Normalisation
*		Similar to GL transactions file
*/

/*
*	Similar to gl_trans header file
*	The main key is ( Terminal_name +systemdate +sequence# ) .
*	Alternate keys could be Date for some reports .
*
*	File Type: ISAM. No of Isam keys are : 1.
*
*	Main key : tr_term[5] + tr_sys_dt + tr_seq_no.
*/

typedef struct {
	char	tr_term[5] ;		/* Terminal name */
	long	tr_sys_dt ;		/* Trans Entered date */
	short	tr_seq_no ;		/* Running sno under tr_term +
					   tr_sys_dt */
	char	tr_descr[25] ;		/* Description */

} Bd_hdr ;


/*
*	Structure/record definition of G/L Trans Item file
*
*	Similar to tr_hdr file, fields are different.
*	File Type: ISAM. No of Isam keys are : 2.
*
*	Main key : tr_term[5] + tr_sys_dt + tr_seq_no + tr_item_no.
*	Alt Key-1: tr_period + tr_fund + tr_reccod + tr_accno[18].
*/

typedef struct {
	char	tr_term[5] ;		/* Terminal name */
	long	tr_sys_dt ;		/* Trans Entered date */
	short	tr_seq_no ;		/* Running sno under tr_term +
					   tr_sys_dt */
	short	tr_item_no ;		/* Item No. 	                  */
	short	tr_fund ;		/* Fund Code        Key-2, Part-2 */
	short	tr_reccod ;		/* Record code      Key-2, Part-3 */
	char	tr_accno[19] ;		/* Account code     Key-2, part-4 */
	short	tr_period ;		/* Period           Key-2, Part-1 */
	double	tr_amount ;		/* Transaction Amount 	  	  */
	short	tr_status ;		/* Transaction Update status 	  */

} Bd_item ;

/*
*	Stock Master file.
*
*	File Type: ISAM.
*	No of Isam keys: 2.
*
*	Main key  : st_fund + st_code
*	Alt key 1 : st_section + st_fund + st_code
*	Alt key 2 : st_fund + st_desc
*/

typedef	struct {

	short	st_fund;		/* Fund to which stock belongs */
	char	st_code[11];		/* Stock item code */
	short	st_section;		/* Stock section number */
	char	st_desc[31];		/* Description/Name of stock */
	char	st_unit[7];		/* Unit of measurement */
	double	st_on_hand;		/* Stock on hand */
	double	st_on_order;		/* Stock on order */
	double	st_po_ordqty;		/* Quantity committed from PO */
	double	st_alloc;		/* Stock allocated */
	double	st_paidfor;		/* Stock paid for */
	double	st_rate;		/* Average price of a unit */
	short	st_leaddays;		/* No. of leaddays for order */
	double	st_committed;		/* Committed Amount from PO */
	double	st_value;		/* Total value of the stock */
	double	st_min;			/* Minimum Limit - Stock on hand */
	double	st_max;			/* Maximum Limit - Stock on hand */
	char	st_accno[19];		/* Default account associated */
	long	st_lastdate;		/* Date of last receipt of stock */
	double	st_y_opb;		/* This year opening balance */
	double	st_y_iss;		/* This year issues */
	double	st_y_rec;		/* This year receipts */
	double	st_y_adj;		/* This year adjustments */
	double	st_m_opb;		/* This month opening balance */
	double	st_m_iss;		/* This month issues */
	double	st_m_rec;		/* This month receipts */
	double	st_m_adj;		/* This month adjustments */
	double	st_bef_cnt;		/* Stock before physical count */
	double	st_aft_cnt;		/* Stock after physical count */

}	St_mast;

/*
*	Stock Transaction file
*
*	File Type: ISAM.
*	No of Isam keys: 2.
*
*	Main key  : st_date + st_type + st_seq_no
*	Alt Key 1 : st_fund + st_code + st_date + st_type
*/

typedef struct {

	long	st_date;		/* Transaction date */
	char	st_type[3];		/* Type of transaction */
	short	st_seq_no;		/* Transaction number */
	short	st_fund;		/* Fund to which stock belongs */
	char	st_code[11];		/* Stock item code */
	char	st_suppl_cd[11];	/* supplier code */
	char	st_ref[13];		/* Reference: PO# or Inv# */
	short	st_location;		/* Location/Cost centre # */
	short	st_period;		/* Period to which GL posting is made */
	char	st_db_acc[19];		/* Debit account number */
	char	st_cr_acc[19];		/* Credit account number */
	double	st_qty;			/* Quantity of transaction */
	double	st_amount;		/* Value of transaction */
	char	st_remarks[31];		/* Remarks on the transaction */
	short	st_fund2;		/* Fund to which stock belongs */
	long	st_po_no;		/* Po Number for Receipts */
}	St_tran;

/*
*	Allocation File 
*
*	File Type: ISAM.
*	No of Isam keys: 3.
*
*	Main Key  :	st_fund + st_code + st_location + st_expacc
*	Alt Key 1 :	st_fund + st_code + st_date + st_time
*	Alt Key 2 :	st_location + st_fund + st_code
*/

typedef struct {
	short	st_fund;		/* Fund to which stock belongs */
	char	st_code[11];		/* Stock item code */
	short	st_location;		/* Location/Cost centre # */
	long	st_date;		/* Date of allocation */
	short	st_time;		/* Time of allocation */
	char	st_expacc[19];		/* Expense acnt for this allocation */
	double	st_issued;		/* Total quantity issued so far */
	double	st_alloc;		/* Balance of Stock allocated */
	double	st_value;		/* Value of balance */

}	Alloc_rec;

/*
*	Stock Section File.
*
*	File Type: SEQUENTIAL.
*
*	File contains only one record.
*/

typedef struct {

	short	no_of_sections;	/* number of sections active */
	char	name[15][36];	/* Array of Section names */

}	St_sect;


/*
*	Supplier file
*
*	File Type: ISAM.
*	No of Index Keys: 3.
*
*	Main key: s_supp_cd (Supplier Code)
*	Alt Key1: s_name (Supplier Name)
*	Alt Key2: s_abb (Supplier Name)
*/

typedef struct {

	char	s_supp_cd[11];	/* Supplier number */
	char	s_name[49];	/* Supplier Name */
	char	s_category[21];	/* Supplier Category Name */
	char	s_fax[11];	/* Supplier fax number */
	char    s_abb[25];	/* Abbreviation for supplier name */
	char    s_add1[31];	/* Address line 1 */
	char    s_add2[31];	/* Address line 2 */
	char    s_add3[31];	/* Address line 3 */
	char    s_pc[11];	/* postal code */
	char	s_contact[26];	/* contact person at supplier */
	char	s_payee[11];	/* Payee code number */
	char	s_phone[11];	/* Supplier phone number */
	char   	s_tmp_flg[1];	/* Temporary supplier (Y,N) */
	char	s_type[1];	/* Contract or Ordinary */
	double  s_discount;	/* Supplier discount on goods */
	double  s_ytd_ord;	/* Total goods in $ ordered */ 
	double  s_ytd_ret;	/* Total goods in $ returned */ 
	double 	s_ytd_recpt;	/* Amount year to date received */
	double 	s_ytd_disc;	/* Amount year to date received */
	double 	s_balance;	/* balance */
	long	s_last_actv;	/* Date of last Activity */
	char	s_gst_reg[11];	/* GST registration number */
	double	s_min_dollar;	/* Min $ Required used thru tendering */

}	Supplier;


/*
*	Purchase Order Header file
*
*	File Type: ISAM.
*	No of Index Keys: 2.
*
*	Main key  : po_code;
*	Alt Key1  : supplier code + PO code
*/

typedef struct {

	long	ph_code;		/* Purchase order number */
	char	ph_supp_cd[11];		/* Supplier number */
	char	ph_payee[11];		/* Payee number */
	short	ph_billto;		/* Bill To Number */
	short	ph_shipto;		/* Ship To Number */
	char	ph_attention[16];	/* Contact for shipment */
	char	ph_req_no[11];		/* Requisition number */
	char	ph_status[1];		/* Status indicating deletion of PO or
					   fully liquidated */
	char	ph_type[1] ;		/* Type of Purchase Order */
	char	ph_print[1] ;		/* PO Print Status */
	long	ph_date;		/* Purchase order date */
	long	ph_due_date;		/* Purchase order due date */
	short   ph_funds;		/* Purchase order fund no */
	short	ph_period ;		/* Accounting Period */
	double	ph_comm;		/* PO total committed amount. */	
	long	ph_lqdate;		/* Date of liquidation of PO */
	double	ph_lqamt;		/* Liquidated amount */
	char	ph_gstmsg[1];		/* Gst Message print on po form */

}	Po_hdr;


/*
*	Purchase Order Detail file
*
*	File Type: ISAM.
*	No of Index Keys: 2.
*
*	Main key  : pi_code + pi_item_no
*	Alt Key1  : pi_fund + pi_acct + pi_code + pi_item_no
*/

typedef struct {

	long	pi_code;		/* Purchase order number */
	short	pi_item_no;		/* Purchase order item number */
	short	pi_school;		/* school Number */
	short	pi_fund;		/* Fund Number */
	char	pi_acct[19]; 		/* Account number */
	char	pi_st_code[11];		/* Stock code in inventory */
	char	pi_desc[61];		/* Description of items */
	long	pi_req_no;		/* Requisition number */
	char	pi_unit[7];		/* Unit of measurement */
	char	pi_tax1[2];		/* Field for tax */
	char	pi_tax2[2];		/* Field for 2nd tax */
	double	pi_pd_qty;		/* Quantity ordered and paid for */
	double	pi_unitprice;		/* Unit Price of the item */
	double  pi_orig_qty;		/* Quantity originally ordered */
	double	pi_original;		/* Originial price of item */
	double	pi_paid;		/* Price paid for the item */
	double  pi_value;		/* Change in the originial price */

}	Po_item;
	
/*
*	Requisition Header file
*
*	File Type: ISAM.
*	No of Index Keys: 2.
*
*	Main key  : code;
*	Alt Key1  : fund + requisition code 
*	Alt Key2  : supplier code + requisition code 
*	Alt Key3  : fund + supplier code + requisition code 
*	Alt Key4  : fund + shipto + requisition code 
*/

typedef struct {

	long	code;		/* requisition number */
	char	supp_cd[11];	/* Supplier number */
	double	amount;		/* requisition amount. */	
	long	date;		/* requisition date */
	long	due_date;	/* requisition due date */
	double	appamt;		/* approved amount */
	short	billto;		/* Bill To Number */
	short   funds;		/* requisition fund no */
	long	appdate;	/* approval date */
	short	shipto;		/* Ship To Number */
	short	costcenter;	/* Cost Center Requesting goods */
	short	period ;	/* Accounting Period */
	char	attention[16];	/* Contact for shipment */
	char	status[2];	/* Approval status */
	char	print_form[2] ;	/* Requistion Form Print Status */
	char	print_stock[2];	/* Requistion Stock Print Status */
	char	print_rpt[2] ;	/* Requistion Po detail Print Status */
	short	stock_fund;	/* Stock Fund for interfund requisitioning*/
}	Req_hdr;


/*
*	Requisition Detail file
*
*	File Type: ISAM.
*	No of Index Keys: 2.
*
*	Main key  : code + item_no
*	Alt Key1  : fund + acct + code + item_no
*	Alt Key2  : fund + st_code + code + item_no
*	Alt Key3  : code + appstat + st_code + item_no
*/

typedef struct {

	long	code;		/* requisition number */
	short	item_no;	/* requisition item number */
	char	acct[19]; 	/* Account number */
	short	fund; 		/* Fund number */
	char	st_code[11];	/* Stock code in inventory */
	char	appstat[2];	/* approval status */
	short	school;		/* school Number */
	char	desc[61];	/* Description of items */
	char	tax1[2];	/* Field for tax */
	char	tax2[2];	/* Field for 2nd tax */
	double  orig_qty;	/* Quantity originally requisted */
	char	unit[7];	/* Unit of measurement */
	double	unitprice;	/* Unit Price of the item */
	double	value;		/* Originial price of items */
	long	pocode;		/* purchase order number */
	char	bdgt_flag[2];	/* Over Budget Flag */
	short	stock_fund	/* Used for Interfund Requisitioning */

}	Req_item;

/*
*	Requisition Reason file
*
*	File Type: ISAM.
*	No of Index Keys: 2.
*
*	Main key  : code + item_no
*/

typedef struct {

	long	reqr_code;		/* requisition number */
	short	reqr_item_no;		/* requisition item number */
	char	reqr_reason[71]; 	/* Requision Disapproval reason */

}	Req_reason;




/* Tendering Data Structures */
#include <bfs_tend.h>



/*
*	Fixed Asset Item Master Record
*
*	File Type: ISAM.
*	No of Index Keys: 4.
*
*	Main Key: fa_costcen + fa_itemid
*	Alt Key 1: fa_type + fa_costcen + fa_itemid
*	Alt Key 2: fa_deptno + fa_type + fa_costcen + fa_itemid
*	Alt Key 3: fa_curcostcen + fa_costcen + fa_itemid
*/

typedef struct	{

	short	fa_costcen;	/* cost centre# */
	long	fa_itemid;	/* item id */
	char	fa_desc[36];	/* description of the item */
	char	fa_type[5];	/* fixed asset item type code */
	char	fa_sno[13];	/* model or s# under the type */
	char	fa_dept[5];	/* department number */
	char	fa_roomno[6];	/* room number */
	char	fa_cond[2];	/* condition code */
	char	fa_suppname[21];/* supplier name */
	char	fa_invc[11];	/* invoice number */
	short	fa_curcostcen;	/* current location of fixed asset */
	long	fa_rectdate;	/* date of receipt */
	long	fa_qty;		/* quantity */
	double	fa_rate;	/* rate */
	double	fa_value;	/* value of the stock */

}	Fa_rec ;

/*
*	Fixed Asset Item type File
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main key: code
*/

typedef struct {

	char	code[5];	/* FA Item type code */
	char	desc[31];	/* FA Item type description */

}	Fa_type;

/*
*	Fixed Asset Dept code File
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main key: code
*/

typedef struct {

	char	code[5];	/* FA Dept Code code */
	char	desc[31];	/* FA Dept Code description */

}	Fa_dept;

/*
*	Fixed Asset Tranfer File
*
*	File Type: ISAM.
*	No of Index Keys: 4.
*
*	Main key: fatr_numb
*	Alt key1: fatr_costcen + fatr_itemid + fatr_numb
*	Alt key2: fatr_tocostcen + fatr_numb
*	Alt key3: fatr_date + fatr_costcen + fatr_numb
*/

typedef struct {

	short	fatr_numb;	/* FA Transfer number */
	short	fatr_costcen;	/* FA Cost center */
	long	fatr_itemid;	/* FA item id number */
	short	fatr_frcostcen;	/* FA current location */
	short	fatr_tocostcen;	/* FA transfer location */
	char	fatr_todept[5];	/* FA transfer department no */ 
	char	fatr_toroomno[6];	/* FA transfer location room no */
	long	fatr_date;	/* FA transfer date */
	char	fatr_cond[2];	/* FA condition code on transfer */
	char	fatr_remarks[25]; /* FA remarks */

}	Fa_transfer;

/*
*	Structure/record definition of Customer Master file ..
*
*	File Type: ISAM.
*	No of Index keys: 2.
*
*	Main key : cu_code
*	Alt Key1 : cu_name
*/

typedef struct {

	char	cu_code[11] ;		/* Customer unique code */
	char	cu_name[31] ;		/* customer's name */
	char	cu_abrev[31];		/* Abreviated name */
	char	cu_adr1[31] ;		/* customer's address line 1 */
	char	cu_adr2[31] ;		/* address line 2 */
	char	cu_adr3[31] ;		/* address line 3 */
	char	cu_pc[11] ;		/* Postal Code */
	char	cu_phone[11] ;		/* Phone Number */
	char	cu_fax[11] ;		/* Fax Number */
	char	cu_contper[26] ;	/* Contact person */
	char	cu_prnt_cd[1] ;		/* Statement Print Code..
					   A - Always Print
					   B - Don't print if Balance = 0
					   C - Don't print if Balance < 0
					   D - Don't print if Balance <= 0
					*/
	long	cu_open_dt ;		/* Either date of account opened or
					   business started */
	long	cu_sale_dt ;		/* Last Sales date */
	long	cu_rcpt_dt ;		/* Last Receipt date */
	double	cu_mon_op ;		/* Monthly Opening Balance in Value */
	double	cu_cur_bal ;		/* Current Outstanding balance */

	double	cu_ytd_sales ;		/* YTD Sales */
	double	cu_ytd_rcpts ;		/* YTD Receipts */

} Cu_rec ;


/*
*	Structure/record definition of Sales Invoice Header file ..
*
*	File Type: ISAM.
*	No of Index keys: 3.
*
*	Main key : ah_fund + ah_inv_no + ah_sno
*	Alt Key 1 : ah_cu_code + ah_fund + ah_inv_no + ah_sno
*	Alt Key 2 : ah_trandt + ah_fund + ah_inv_no + ah_sno
*/

typedef struct {

	short	ah_fund;	/* fund# for which invoice is raised */
	long	ah_inv_no;	/* invoice number under fund */
	short	ah_sno;		/* header serial number under invoice */
	char	ah_type[3];	/* Invoice/DM/CM type code */
	char	ah_status[1];	/* Invoice status O/C : Open/Close */
	char	ah_cu_code[11];	/* customer code */
	long	ah_trandt;	/* transaction date */
	long	ah_duedt;	/* due date for the invoice */
	double	ah_oriamt;	/* original amt for which invoice is raised */
	double	ah_gramt;	/* gross amount inclusive of DM and CM */
	double	ah_txpercent;	/* tax % on net amount */
	double	ah_txamt;	/* tax amount on net amount */
	double	ah_netamt;	/* net amount */
	double	ah_balance;	/* payment outstanding against the invoice */
	short	ah_period;	/* accounting period */
	char	ah_remarks[21];	/* remarks on the invoice */
	double  ah_gstpercent;	/* GST percentage	*/
	double  ah_gstamt;	/* GST amount		*/ 

}	Ar_hdr;

/*
*	Structure/record definition of Sales Invoice Item file ..
*
*	File Type: ISAM.
*	No of Index keys: 1.
*
*	Main key : ai_fund + ai_inv_no + ai_hno + ai_sno
*/

typedef struct {

	short	ai_fund;	/* fund of the header file record */
	long	ai_inv_no;	/* Invoice number in the header file record */
	short	ai_hno;		/* Header serial number under above invoice */
	short	ai_sno;		/* Item number under above key */
	char	ai_accno[19];	/* Account number to be debited */
	double	ai_amount;	/* Amount debited to above account */
	char	ai_desc[61];	/* Invoice description */

}	Ar_item;

/*
*	Structure/record definition of Receipts Header file ..
*
*	File Type: ISAM.
*	No of Index keys: 3.
*
*	Main key : rhdr_refno
*	Alt-key1 : rhdr_rcptdate + rhdr_fund + rhdr_refno
*	Alt-key2 : rhdr_cust + rhdr_fund + rhdr_refno
*/

typedef struct {

	long	rhdr_refno;		/* reference number */
	short	rhdr_fund;		/* Invoice fund number */
	char	rhdr_applied[2];	/* applied or unapplied */
	char	rhdr_cust[11] ;		/* Customer unique code */
	long	rhdr_rcptdate;		/* receipt date */
	char	rhdr_chequeno[16];	/* Cheque number */
	double	rhdr_amount;		/* Receipt amount */
	short	rhdr_period;		/* G/L period */
	char	rhdr_acctno[19];	/* G/L account number */
	char	rhdr_remarks[21];	/* remarks */

}	Rcpt_hdr;


/*
*	Structure/record definition of Receipts Item file ..
*
*	File Type: ISAM.
*	No of Index keys: 2.
*
*	Main key : ritm_refno + ritm_seqno
*	Alt-key1 : ritm_cust + ritm_invnumb + ritm_refno + ritm_seqno
*/

typedef struct {

	long	ritm_refno;		/* reference number */
	long	ritm_seqno ;		/* sequence number */
	long	ritm_invnumb;		/* Invoice number */
	char	ritm_cust[11] ;		/* Customer unique code */
	double	ritm_amount;		/* Receipt amount */

}	Rcpt_item;


/*
*	Purchase Invoices File.
*
*	File Type: ISAM.
*	No of Index Keys: 2.
*
*	Main Key: in_supp_cd + in_invc_no + in_tr_type
*	Alt Key1: in_funds + in_supp_cd + in_invc_no + in_tr_type
*/

typedef struct {

	char	in_supp_cd[11] ; 	/* Supplier number */
	char	in_invc_no[16] ;	/* Purchase invoice number */
	char	in_tr_type[3] ;		/* IN(voice),RT(Return),CM(Cr Memo),
					   DM(Dr Memo) */
	char	in_type[1] ;		/* D(irect Charge),B(ulk), N(on Bulk) */
	char	in_pmtcode[1] ;		/* O(pen),S(top),C(omplete),
					   R(elease HB),P(artial/Manual) */
	char	in_accno[19] ;		/* Manual Cheques Bank Account# */
	char	in_remarks[21] ;	/* Invoice Remarks */
	short   in_funds ;		/* Fund no */
	short	in_period ;		/* Period */
	long	in_po_no ;		/* Purchase Order Number */
	long	in_invc_dt ;		/* Invoice date */
	long	in_due_dt ;		/* Payment Due date */
	long	in_chq_no ;		/* Latest Paid cheque# */
	double	in_disc_per ;		/* Discount % */
	double	in_disc_amt ;		/* Discount on Gross Amount */
	double	in_amount ;		/* Amount (With Discount on taxes) */
	double	in_gsttax ;		/* GST Tax amount */
	double	in_psttax ;		/* PST Tax amount */
	double	in_part_amt ;		/* Partial pmnt amt/Manual Chq Paid */
	char	in_orgsupp_cd[11];	/* Used for the AP history */
}	Invoice;


/*
*	Purchase Invoice Temporary Hdr File.
*
*	This file is a temporary file. This is used in Invoice Entry.
*	Records written to this file are temporary, they will be DELETED
*	immediately the process is over.
*
*	File Type: ISAM.
*	No of Index Keys: 2.
*
*	Main Key: h_term + h_batch + h_sno
*	Alt Key1: h_term + h_period + h_funds + h_supp_cd + h_invc_no +
*							h_tr_type + h_sno
*/

typedef struct {

	char	h_term[5] ;		/* Terminal */
	short	h_batch ;		/* Batch# */
	short	h_sno ;			/* Running Sno */
	char	h_supp_cd[11] ; 	/* Supplier number */
	char	h_invc_no[16] ;		/* Purchase invoice number */
	char	h_tr_type[3] ;		/* IN(voice),RT(Return),CM(Cr Memo),
					   DM(Dr Memo) */
	char	h_type[1] ;		/* D(irect Charge),B(ulk), N(on Bulk) */
	char	h_pmtcode[1] ;		/* O(pen),S(top),C(omplete),
					   R(elease HB),P(artial/Manual) */
	char	h_accno[19] ;		/* Manual Cheques Bank Account# */
	char	h_remarks[21] ;		/* Invoice Remarks */
	short   h_funds ;		/* Fund no */
	short	h_period ;		/* Period */
	long	h_po_no ;		/* Purchase Order Number */
	long	h_invc_dt ;		/* Invoice date */
	long	h_due_dt ;		/* Payment Due date */
	long	h_chq_no ;		/* Latest Paid cheque# */
	double	h_disc_per ;		/* Discount % */
	double	h_disc_amt ;		/* Discount on Gross Amount */
	double	h_amount ;		/* Amount (With Discount on taxes) */
	double	h_gsttax ;		/* GST Tax amount */
	double	h_psttax ;		/* PST Tax amount */
	double	h_part_amt ;		/* Partial pmnt amt/Manual Chq Paid */
	char	h_po_cmp[2];		/* Po complete flag (Y/N) */

}	In_hdr;


/*
*	Purchase Invoice Items File.
*
*	This file contains item details(distribution) of a Purhase Invoice.
*	Header record and item records make one purchase invoice.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: i_supp_cd + i_invc_no + i_tr_type + i_item_no
*/

typedef struct {

	char	i_supp_cd[11] ; 	/* Supplier number */
	char	i_invc_no[16] ;		/* Purchase invoice number */
	char	i_tr_type[3] ;		/* IN(voice),RT(Return),CM(Cr Memo),
					   DM(Dr Memo) */
	short	i_item_no ;		/* Invoice item number */
	char	i_stck_cd[11];		/* Stock code in inventory */
	char	i_99accno[19]; 		/* Record Cd 99 G/L Account number */
	char	i_97accno[19]; 		/* Record Cd 97 G/L Account number */
	short	i_school ;		/* School# */
	short	i_fund ;		/* Fund# */
	double	i_qty;			/* Quantity Received */
	double  i_value;		/* Total Value of the item */

}	In_item;


/*
*	Cheque File.
*
*	File Type: ISAM.
*	No of Index Keys: 2.
*
*	Main Key: c_supp_cd + c_funds + c_chq_no + c_invc_no + c_tr_type
*	Alt Key1: c_funds+c_accno + c_chq_no + c_supp_cd + c_invc_no + c_tr_type
*
*	NOTE:	This file has two cheque numbers. 'c_chq_no' is either 0 or
*		manual cheque#( c_chq_no will be put as 0 for the regular chqs
*		at the time of Payment selection). This number is required in
*		the both the index keys to get all the trans of one manual chq
*		in one palce. 'c_cp_chq_no' is required to have the real chq#,
*		which is generated in cheque process run. This number you can't
*		put in 'c_chq_no', because this is part of main key, which can't
*		be changed.
*/

typedef struct	{

	short	c_funds ;		/* Fund#, where the cheque belongs to */
	long	c_chq_no ;		/* Cheque Seq. number */
	char	c_supp_cd[11] ;		/* Supplr#, to whom cheque is issued */
	char	c_invc_no[16] ;		/* Purchase invoice number */
	char	c_tr_type[3] ;		/* IN(voice),RT(Return),CM(Cr Memo),
					   DM(Dr Memo) */
	char	c_chq_type[1] ;		/* 'M' - Manual, 'R' - Regular Cheque */
	char	c_cancelled[1] ;	/* 'Y'es or 'N'o */
	char	c_accno[19] ;		/* Manual/Cancelled Cheuqe Acct# */
	long	c_cp_chq_no ;		/* Chq# generated by chq processing */
	long	c_invc_dt ;		/* Invoice Date */
	long	c_due_dt ;		/* Payment Due Date */
	short	c_period ;		/* Invoice Period */
	double	c_disc_per ;		/* Discount Percentage */
	double	c_in_amt ;		/* Invoice Balance */
	double	c_disc_taken ;		/* Discount Taken */
	double	c_gr_amt ;		/* Cheque Gross Amount (Including
					   Discount Taken) */
}	Chq_rec ;


/*
*	Cheque Register File.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: cr_funds + cr_chq_no
*/

typedef struct	{

	short	cr_funds ;		/* Fund#, where the cheque belongs to */
	long	cr_chq_no ;		/* Cheque Seq. number */
	long	cr_date ;		/* Cheque date */
	char	cr_supp_cd[11] ;	/* Supplr#, to whom cheque is issued */
	char	cr_chq_type[1] ;	/* 'M' - Manual, 'R' - Regular Cheque */
	char	cr_cancelled[1] ;	/* 'Y'es or 'N'o */
	char	cr_sys[1];		/* system flag A - A/P     */
					/*	       P - Payroll */
	double	cr_disc_taken ;		/* Discount Taken */
	double	cr_gr_amt ;		/* Cheque Gross Amount (Including
					   Discount Taken) */

}	Reg_rec ;

/*
*	APHIST File.  Maintains History of Invoice etc. paid details.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: a_supp_cd + a_invc_no + a_tr_type + a_sno
*/

typedef struct	{

	char	a_supp_cd[11] ; 	/* Supplier number */
	char	a_invc_no[16] ;		/* Purchase invoice number */
	char	a_tr_type[3] ;		/* IN(voice),RT(Return),CM(Cr Memo),
					   DM(Dr Memo) */
	short	a_sno ;			/* Running Sno under a_supp_cd +
					   a_invc_no + a_tr_type */
	char	a_paid[1] ;		/* Invioce Complete - 'Y'es or 'N'o */
	char	a_accno[19] ;		/* Cheque Bant Acct# */
	short	a_fund ;		/* Transaction Fund */
	long	a_po_no ;		/* Purchase Order number */
	short	a_period ;		/* Period number */
	long	a_chq_no ;		/* Cheque number */
	long	a_tr_date ;		/* Invoice date */
	double	a_disc_taken ;		/* Discount Taken */
	double	a_gr_amt ;		/* Cheque Gross Amount (Including
					   Discount Taken) */
}	Ap_hist ;

/*
*	Structure/record definition LASTPO file ..
*
*	File Type: SEQUENTIAL.
*
*	File contains only one record.
*/

typedef	struct {
	long	last_po;
}	Last_po;

/*
*	Structure/record definition LASTREQ file ..
*
*	File Type: SEQUENTIAL.
*
*	File contains only one record.
*/

typedef	struct {
	long	last_req;
}	Last_req;

/*--------------------------------END OF FILE----------------------------*/

