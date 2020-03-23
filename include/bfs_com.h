/*
*    Source Name : bfs_com.h
*    System      : Budgetary Financial System.
*
*    Created On  : 2nd May 1989.
*
*    Contains Structure/Record Definitions used in this system.
*/


#define	DESC_KEY	12	/* Desc key size for ISAM */
#define	NO_KEYS		12	/* No of Keys */
#define	NO_PERIODS	13	/* No Of Periods */


/*
*	Structure/record definition PARAMETER file ..
*
*	File Type: SEQUENTIAL.
*
*	File contains only one record.
*/

typedef	struct {
	char	pa_co_name[51] ;	/* Company Name */
	short	pa_distccno;		/* District Cost Center Number */
	short	pa_wareccno;		/* Warehouse Cost Center Number */

	char	pa_co_or_dist[1] ;	/* Company or School district. C or D */
	char	pa_aps[1] ;		/* A/P system Present? Y or N */
	char	pa_requisition[1] ;	/* Requisition system Present? Y or N */
	char	pa_tendering[1] ;	/* Requisition system Present? Y or N */
	char	pa_rotational[1] ;	/* Requisition system Present? Y or N */
	char	pa_pos[1] ;		/* Cancel pending POs at year end.
					   Y or N. This is applicable only
					   when A/P present */
	char	pa_poinv[1] ;		/* Po and Inventory Interface */
	char	pa_stores[1] ;		/* Inventory system Present? Y or N */
	char	pa_fa[1] ;		/* Fixed Assests system Present?
					   Y or N */
	char	pa_ars[1] ;		/* A/R system Present? Y or N */
	char	pa_funds[1] ;		/* More than 1 Fund(Control Record).
					   Y or N */
	char	pa_glmast[1] ;		/* G/L Master File Present. Y or N */
	char	pa_cheque[1] ;		/* Consolidated Cheques. Y or N. This
					   is applicable only when A/P present*/
	short	pa_cur_period;		/* Current Period 1 to 13 */
	short	pa_open_per ;		/* No of open periods to post journal
					   entries */
	long	pa_date ;		/* Today's System Date */
	short	pa_no_periods;		/* Number of periods is 12 or 13 */
	short	pa_bdgt_key1;		/* Budget Report key 1 */
	short	pa_bdgt_key2;		/* Budget Report key 2 */
	short	pa_bdgt_key3;		/* Budget Report key 3 */
	short	pa_bdgt_key4;		/* Budget Report key 4 */
	short	pa_bdgt_key5;		/* Budget Report key 5 */
	short	pa_bdgt_key6;		/* Budget Report key 6 */
	short	pa_bdgt_key7;		/* Budget Report key 7 */
	short	pa_cc_key;		/* Cost Center key */
	short	pa_due_days_po;		/* Number of days before due date (PO)*/
	short	pa_due_days_ap;		/* Number of days before due date (AP)*/
	short	pa_due_days_ar;		/* Number of days before due date (AR)*/
	char	pa_budget[1];		/* Transfer budget to new year YorN */
	char	pa_dist_gst[1];		/* Distribute GST over accts. YorN */
	char	pa_regist[11];		/* GST Registration number */
	char	pa_gst_tax[2];		/* GST Tax Field Default */
	char	pa_pst_tax[2];		/* PST Tax Field Default */
	double	pa_purlimit;		/* Purchasing limit */
	short	pa_cons_key;		/* Consumption Key  */
	short	pa_tb_key;		/* Trial Balance Key */
} Pa_rec ;



/*
*	Structure/record definition of Control file ..
*
*	File Type: ISAM. No of Isam keys are : 1.
*
*	Main key : fund
*/

typedef struct {
	short	fund ;			/* fund number */
	char	desc[31];		/* description of fund */

	/* Common G/L Accounts numbers */
	char	ap_gen_acnt[19] ;	/* A/P General Account */
	char	ap_cnt_acnt[19] ;	/* A/P Contract Acccount */
	char	ar_gst_acnt[19] ;	/* A/P GST Acccount */
	char	dis_acnt[19] ;		/* Discount General Account */
	char	expdis_acnt[19] ;	/* Expense Discount Account */
	char	sus_acnt[19] ;		/* Suspence general Account */
	char	bank1_acnt[19];		/* Bank-1 Account */
	char	bank2_acnt[19];		/* Bank-1 Account */
	char	pst_tax_acnt[19] ;	/* Provincial tax Account */
	char	gst_tax_acnt[19] ;	/* Provincial tax Account */
	char	ar_acnt[19] ;		/* A/R General Account */
	char	ap_gst_acnt[19] ;	/* A/R General Account */
	char	inv_acnt[19] ;		/* Inventory General Account */
	char	p_and_l_acnt[19] ;	/* Profit & Loss Account */
	char	s_d_accm_acnt[19] ;	/* Surplus/Deficit Accumulated Acct */
	long	bank1_chq ;		/* Last cheque# printed for bank 1
					   in A/P System */
	long	bank2_chq ;		/* Last cheque# printed for bank 2
					   in A/P System */
	long	last_po ;		/* Last Po number generated */
	long	last_req;		/* Last Requisition number generated */
	short	pst_tax ;		/* Percentage of PST */
	short	gst_tax ;		/* Percentage of GST */
	short	rebate ;		/* Percentage of REBATE */
	char	duetofrom_acct[19];	/* Due to From Account Numer */
} Ctl_rec ;


/*
*	Structure/record definition of School file
*
*	File Type: ISAM. No of Isam keys are : 1.
*
*	Main key : sc_numb
*/

typedef struct {
	short	sc_numb ;		/* School number PIC 99 */
	char	sc_name[29] ;		/* School name PIC X(28)  */
	char	sc_add1[31];		/* Address Line 1 */
	char	sc_add2[31];		/* Address Line 2 */
	char	sc_add3[31];		/* Address Line 3 */
	char	sc_pc[8];		/* postal code */
	char	sc_contact[26];		/* Contact person */
	char	sc_phone[11];		/* telephone number */
	char	sc_fax[11];		/* fax number */
	long	sc_size ;		/* School size, or area, PIC 9(6) */
	char	sc_r_t[2];		/* School Rural or Town */
} Sch_rec ;


/*
*	Structure/record definition AUDIT file ..
*
*	File Type: SEQUENTIAL.
*/

typedef	struct {
	char	terminal[5] ;		/* Terminal Name */
	char	user_id[11] ;		/* User Id */
	char	program_id[11] ;	/* Program Id */
	long	run_date ;		/* Date */
	short	run_time ;		/* Time */
	short	file_no ;		/* DBH File# */
	short	field_no ;		/* Field# in Record */
	short	mode ;			/* Audit Mode. ADD - Rec Added,
					   UPDATE - Rec Changed,
					   P_DEL - Rec Deleted */
	char	rec_key[26] ;		/* record Key */
	char	old_value[51] ;		/* Old Value */
	char	new_value[51] ;		/* New Value */

} Aud_rec ;


/*
*	Structure/record definition of G/L MASTER file ..
*
*	File Type: ISAM. No of Isam keys are 4.
*
*	Main key : funds + accno + reccod.
*	Alt1 key : funds + desc(DESC_KEY).
*	Alt2 key : reccod + funds + sect + accno.
*	Alt3 key : funds + sect + reccod + accno.
*/

typedef	struct {
	short	funds ;			/* Funds PIC 999 */
	char	accno[19] ;		/* Account number */
	short	reccod ;		/* Record Code PIC 99 */
	short	sect ;			/* Section Code? PIC 9 */
	short	admis ;			/* Admissibility code PIC 9 */
	long	keys[NO_KEYS] ;		/* 12 keys PIC 9[5] */
	short	cdbud ;			/* Budget code */
	short	cdpro ;			/* Projection code */
	short	cdunit ;		/* Unit of measurement code */
	char	desc[49] ;		/* Description */
	double	grad ;			/* Gradiant ? PIC 999.99 */
	double	comdat ;		/* Committed to date */
	double	curdb ;			/* Current period Debits */
	double	curcr ;			/* Current period Credits */
	double	ytd ;			/* Year to Date */
	double	budcur;			/* Budget current Year */
	double	budpre ;		/* Budget previous Year */
	double	currel[NO_PERIODS] ;	/* actual current yr ..13 periods */
	double	prerel[NO_PERIODS] ;	/* actual previous yr ..13 periods */
	double	curbud[NO_PERIODS] ;	/* Budget current yr ..13 periods */
	double	prebud[NO_PERIODS] ;	/* Budget previous yr .13 periods */
	/***
	double	budrep[NO_PERIODS] ;	* actual current yr ..13 periods *
	***/
	double	opbal ;			/* Opening balance */

	double	nextdb ;		/* Next period Debits(Accruals) */
	double	nextcr ;		/* Next period Credits(Accruals) */
} Gl_rec ;


/*
*		Transaction file .. Normalisation
*
*	The old transaction file will be broken in two files for
*	normalisation . Common information will be kept in a header
*	file and actual line entries(items) will be kept
*	in an item file. Main key of header file will be appended by
*	Item number ( running sequence number ) to derive item file
*	key .
*/

/*
*	Structure/record definition of G/L Trans Header file
*
*	The header record contains common information for all transaction
*	records entered in a single journal entry session . Two extra
*	fields are added for optionally keeping total debits and total
*	credits of all items under a complete transaction .
*	The main key is ( Terminal_number + Reference Code ) .
*	Alternate keys could be Date for some reports .
*
*	File Type: ISAM. No of Isam keys are : 2.
*
*	Main key : th_fund + th_reccod + th_create[2] + th_seq_no.
*	Alt Key-1: th_sys_dt + th_fund + th_reccod + th_create[2] + th_seq_no 
*			 + th_user_id[11] 
*/

typedef struct {
	short	th_fund ;		/* Fund */
	short	th_reccod ;		/* Record code */
	char	th_create[2] ;		/* Created E- entered G- generated */
	long	th_seq_no ;		/* Running sno under tr_userid[11]
					   tr_sys_dt */
	long	th_sys_dt ;		/* Trans Entered date */
	char	th_userid[11] ;		/* UserId(login name) */
	short	th_period ;		/* Period */
	long	th_date ;		/* Date of Transaction */
	double  th_debits ;		/* Optional .. Sum of all -ve amnts */
	double  th_credits ;		/* Optional .. Sum of all +ve amnts */
	char	th_descr[25] ;		/* Description */
	char	th_supp_cd[11] ;	/* Supplier number */
	char	th_reference[16] ;	/* Reference number */
	char	th_type[1] ;		/* Transaction type */
	char	th_print[2];		/* Print Flag for Audit Report */
} Tr_hdr ;


/*
*	Structure/record definition of G/L Trans Item file
*
*	The item file record has all repeating fields of original file,
*	i.e. GL key, Period and amount for each line entry on screen .
*	The main key is same as header appended by line sequence number
*	to ensure uniqueness of all line items .
*	Key required by Update : (period+fund+recod+accno)
*	Key Required by Listing: (period+fund+reccod+ref_no)
*
*	File Type: ISAM. No of Isam keys are : 3.
*
*	Main key : ti_fund + ti_reccod + ti_create[2] + ti_seq_no + ti_item_no
*
*		NOTE: tr_sys_dt & tr_userid are interchanged their positions
*		      in main_key, when it is compared with Tr_hdr main key. 
*
*	Alt Key-1: ti_fund + ti_reccod + ti_accno[18] + ti_period ;
*/

typedef struct {
	short	ti_fund ;		/* Fund Code        Key-1, Part-1 */
	short	ti_reccod ;		/* Record code      Key-1, Part-2 */
	char	ti_create[2] ;		/* create E- entered G- generated */
	long	ti_seq_no ;		/* Running sno under tr_userid[11]
					   tr_sys_dt */
	short	ti_item_no ;		/* Item No. 	                  */
	long	ti_sys_dt ;		/* Trans Entered date */
	short	ti_period ;		/* Period           Key-1, Part-4 */
	char	ti_accno[19] ;		/* Account code     Key-1, part-3 */
	double	ti_amount ;		/* Transaction Amount 	  	  */
	short	ti_status ;		/* Transaction Update status 	  */
	short	ti_section ;		/* Section code     */

} Tr_item ;

/*
*	Cheque History File.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: ch_funds + ch_accno + ch_chq_no
*/

typedef struct	{

	short	ch_funds ;		/* Fund#, where the cheque belongs to */
	long	ch_chq_no ;		/* Cheque Seq. number */
	char	ch_accno[19] ;		/* Cheque Bank Acct# */
	char	ch_name[49] ;		/* Name on Cheque */
	char	ch_sys[1];		/* system flag A - A/P     */
					/*	       P - Payroll */
	char	ch_status[1] ;		/* cheque status  C - Cashed */
					/*		  X - Cancelled */
					/*		  O - Outstanding */
	long	ch_date ;		/* Cheque date */
	double	ch_net_amt ;		/* Cheque Amount */

}	Chq_hist ;

/*
*	User Profile File.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: u_id
*/

typedef struct {
	char	u_id[11];	/* user's login name */
	char	u_name[31];	/* user's name for identification */
	char	u_passwd[15];	/* user's password */
	char	u_trml[4];	/* user's login teminal */
	char	u_class[2];	/* user's class: Administrator or User */
	char	u_access[TOTAL_FILES+1];	/* All Bit flags in one byte */
	/* access rights by file & operation( DELETE, UPDATE, ADD, BROWSE) */

}	UP_rec;


typedef	struct{
	double gst_amt;
	double pst_amt;
	double gros_amt;
} Tax_cal;

/*--------------------------------END OF FILE----------------------------*/
