/*-------------------------------------------------------------------
Source Name: apinvc12.c
System     : Accounts Payables.
Created  On: 1st November 89.
Created  By: T AMARENDRA.

COBOL Source(s): cp030---07

DESCRIPTION(SCR1):
	Program to show batch totals for entered Purchase Invoice, Return,
	Debit Memos and Credit Memo thru Invoice Entry Screen(SCR2).

DESCRIPTION(SCR2):
	Program to enter Purchase Invoice, Return, Debit Memos and Credit Memo.

	These transactions are categorised into DIRECT, BULK, NON-BULK. DIRECT
	means stock will not go to inventory. BULK means stock is not ordered
	for any particular cost center, so Inventory Allocation record need not
	be created. NON-BULK is stock ordered to sprecific cost centers.

	For BULK orders 99 type Accounts are not taken from user, Inventory
	General Account is taken as Item account.

	PO# is sought only for INVOICE. When the valid PO# is given, 
	outstanding items of PO are shown as default items for INVOICE. 
	Only QTY, AMOUNT and 97 type Accounts are accepted from User.

	When the "Inventory system Present Flag" of Parameter is set to NO,
	then the type will be always DIRECT Charge.

	STOCK Code is not Validated for DIRECT Charge Type.

	SCHOOL# is sought for NON-BULK type Invoice, to allocate stock.

	G/L UPDATE:
	-----------
	    Commitments
	    -----------
		Are updated only for INVOICEs. When PO is given commitments
		are decreased for DIRECT & BULK types. When PO is not given
		commitments are increased for NON-BULK items.
		(NOTE: Commitments are decreased in Stock Issues, when Stock
			allocation exists. Stock Allocation Records are created
			for NON-BULK type records)

	    GL Trans
	    --------

	    INVOICE:
	    CRMEMO:
		Credit the Gross Amount to A/P General
		Credit the Tax Amount to Tax General
		If( DIRECT Charge)
		    Debit each item amount to user given account in item
		else
		    Debit the Total of items(Gross+Tax) to Inventory General

		If INVOICE and 97-ACCOUNT is entered (for each item)
		    Debit item QTY to user given account in item
		
	    RETURN:
	    DBMEMO:
		Debit the Gross Amount to A/P General
		Debit the Tax Amount to Tax General
		If( DIRECT Charge)
		    Credit each item amount to user given account in item
		else
		    Credit the Total of items(Gross+Tax) to Inventory General

		If RETURN and 97-ACCOUNT is entered (for each item)
		    Credit item QTY to user given account in item

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
peter ralph    90/11/21	      Use pa_rec.pa_due_days as default due date
C.Leadbeater   90/11/22	      Default quantity to 1 if PO# equal to 0.
C.Leadbeater   90/11/22       If D(irect charge) invoice, allow return
			      to be entered at stock code field.
J.Prescott     90/11/22       Added Validation for Inventory Control account
			      When adding Direct or Non-Bulk Transactions.
C.Leadbeater   90/11/23       Change word 'batch' to 'session' in 
			      ReadFunction() .
J.Prescott     90/11/28       Added the use of payee field in supplier file.
p ralph	       90/12/05       Change transaction screen, add gst/pst,
			      description.
F.Tao 	       90/12/19	      Round up amounts before writing to file.
F.Tao 	       90/12/30	      Should not allow input account number and    
			      bank # when it is DM or RT.
C.Leadbeater   91/01/03	      Modified so that additional items could be 
				added to an invoice for a PO (Balance Items).
J.Prescott     91/05/27       Added GST rebate line by line.  Also made 
			      GST distribution work. and added default for
			      GST flag to exempt if supplier does not have
			      a gst registration number.
A.Cormier      92/10/27       Changed Cost Center number from an integer of 2
			      to a 4.
------------------------------------------------------------------------*/
#include <stdio.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_recs.h>
#include <apinvc.h>
 
#define	SUPPMAINT	"supplier"	/*Supplier Maintenace Executable
						 file's base name */

#define	TAXPROBLEM	-12		/* Problem with tax calculations */

/* PROFOM Releted declarations */
#define	SCR1_NAME	"apinvc1"	/* PROFOM screen Name */
#define	SCR2_NAME	"apinvc2"	/* PROFOM screen Name */

#define	TRUE		1
#define	FALSE		0 

/* User Interface define constants */
#ifdef ENGLISH
#define	ADDREC		'A'
#define	NEWBATCH	'S'
#define	EXITOPT		'E'
#define TAXABLE		'T'
#define EXEMPT          'E'

#define	YES		'Y'
#define	ADDITEMS	'A'
#define BALANCE		'B'
#define	DELITEM		'D'
#define	REVIVEITEM	'R'
#define	EDIT		'E'
#define	NEXTPAGE	'N'
#define	PREVPAGE	'P'
#define	TOTAL_CHNG	'T'
#define	CANCEL		'C'

#define	ACTIVE		"ACT"
#define	NOTACTIVE	"DEL"

#define POPRNT		'P'	/* set from L to P if po is not complete */
				/* so invoice label can be printed again */
#define PERCENT		'P'	/* percentage discount */
#define PARTIAL		'P'
#define FULL_AMT	'F'
#define AMOUNT		'A'
#else
#define	ADDREC		'R'
#define	NEWBATCH	'S'
#define	EXITOPT		'F'
#define TAXABLE		'T'
#define EXEMPT          'E'

#define	YES		'O'
#define	ADDITEMS	'R'
#define BALANCE		'B'
#define	DELITEM		'E'
#define	REVIVEITEM	'V'
#define	EDIT		'M'
#define	NEXTPAGE	'S'
#define	PREVPAGE	'P'
#define	TOTAL_CHNG	'T'
#define	CANCEL		'A'

#define	ACTIVE		"ACT"
#define	NOTACTIVE	"ELI"
#define POPRNT		'I'	/* set from E to I if po is not complete */
				/* so invoice label can be printed again */
#define PERCENT		'P'	/* percentage discount */
#define PARTIAL		'P'
#define FULL_AMT	'M'
#define AMOUNT		'M'
#endif
#define	SPACE		' '


/* PROFOM Screen-2 STH file */

/* Field PROFOM numbers */

#define	SCR1_END_FLD	3600	/* Last Field of the screen */
#define	FN_FLD		500	/* Fn: */
#define	BATCHNO_FLD	600	/* Batch#: */
#define	ST_FLD		1200	/* Batch Totals Start Fld */

/* apinvc1.sth - header for C structure generated by PROFOM EDITOR */

typedef	struct	{		/* Start Fld 1200, Endfld 2700 and Step 400 */

	short	b_trans ;	/* NUMERIC 9999 Field 1200 */
	double	b_gr_amt ;	/* NUMERIC S99F999F999.99 Field 1300 */
	double	b_disc_amt ;	/* NUMERIC S99F999F999.99 Field 1400 */
	double	b_hold_back ;	/* NUMERIC S99F999F999.99 Field 1500 */

}	B_item ;

struct	b_struct	{

	char	b_pgm[11];	/* STRING XXXXXXXXXX Field 100 */
	long	b_rundate;	/* DATE YYYYFMMFDD Field 300 */
	char	b_fn[2];	/* STRING X Field 500 */
	short	b_batch_no;	/* NUMERIC 999 Field 600 */

	B_item	b_items[4] ;	/* Start Fld 1200, Endfld 2700 step 400 */

	B_item	b_tot ;		/* Start Fld 2800, Endfld 3100 */

	char	b_mesg[78];	/* STRING X[77] Field 3200 */
	char	b_resp[2];	/* STRING X Field 3300 */
} ;


/* PROFOM Screen-2 STH file */

#define	PAGESIZE	2		/* No of Items in Screen 2 */

/* Field PROFOM numbers */

#define	HDR_ST_FLD	1100	/* Header Start Field */
#define	HDR_END_FLD	3500	/* Header End Field */
#define HDR_LINE_FLD	3700    /* Line divison between hdr and items */
#define	ITEM_ST_FLD	3800	/* Item 1 Start Field */
#define	SCR2_END_FLD	6900	/* Last Field of the screen */
#define	STEP		1200	/* NO of fields diff. between 2 items */

/* HDR Fields */
#define	KEY_START	600	/* Supplier Cd: */
#define	KEY_END		800	/* Type: */
#define	SUPPCD_FLD	600	/* Supplier Cd: */
#define	REFNO_FLD	700	/* Ref#: */
#define	TRTYPE_FLD	800	/* Trans. Type: */
#define	CHNG_FLD	900	/* Field: */
#define	PONO_FLD	1100	/* PO#: */
#define PAYEE_FLD	1200    /* Payee field */
#define	PERIOD_FLD	2300	/* Period: */
#define	FUND_FLD	1300	/* Fund: */
#define	INVCDT_FLD	1400	/* Ref date: */
#define	DUEDT_FLD	1500	/* Due Date: */
#define	TYPE_FLD	1600	/* Type: */
#define GST_PF		1800    /* Gst percent/amount */
#define GST_TAX_FLD	1900	/* Gst fld */
#define PST_PF		2000    /* Pst percent/amount */
#define PST_TAX_FLD	2100    /* pst fld */
#define	GRAMT_FLD	1700	/* Gross Amt: */
#define	DISCPER_FLD	2700	/* Disc% */
#define DISC_PF		2600	/* disccount full percentage or full amount */
#define NETAMT_FLD	2200    /* Net Amt Field */
#define PO_COMP_FLD	2400	/* Po complete field */
#define	CHQNO_FLD	2900	/* Cheque# : */
#define BANKACCT_FLD    3000	/* Bank Acct : */
#define	REMARKS_FLD	2800	/* Remarks  */
#define MISC_T_FLD	3100	/* Misc Expense Header */
#define MISC_GST_FLD	3200	/* Misc GST */
#define MISC_REB_FLD	3300	/* Misc REB */
#define MISC_PST_FLD	3400	/* Misc PST */
#define MISC_E_FLD	3500	/* Misc Expense Amount Field */
#define	PAGENO_FLD	3600	/* Page#: */
#define START_AMT_FLD	6200	/* Amount fields (gst,pst,disc,total) */
#define END_AMT_FLD     6700	/* end amount fields*/

/* Item Fields. These numbers are difference between that field
   and the 1st fld within the item */
#define	SCHOOL_FLD	600
#define	STCKCD_FLD	200
#define	ACNO99_FLD	400
#define	ACNO97_FLD	500
#define GST_FLD		700  
#define REBATE_FLD	800
#define PST_FLD		900
#define	QTY_FLD		1000 
#define	AMNT_FLD	1100 

/* Field Serial Numbers */
#define PO		1
#define PAYEE		2
#define	FUND		3
#define	INVC_DT		4
#define	DUE_DT		5
#define	TYPE		6
#define	GROSS_AMT	7
#define GST_TAX		8
#define PST_TAX         9
#define	NET_AMT		10
#define PERIOD		11	
#define	POCOMPLETE	12
#define DISC_AMT        13 
#define REMARKS		14
#define	CHEQUE_NO	15
#define	BANK_ACCT	16
#define MISC_EXP	17
#define MISC_REB	18
#define MISC_PST	19
#define MISC_AMT	20
#define	HDR_LAST_SNO	17

/* Accounting Posting Methods */
#define	Gl_1	1
#define Gl_2	2
#define Gl_3	3
#define Gl_4	4

typedef struct	{
	short	s_sno;		/* sequence # 3300 */
	char	s_desc[40];	/* description 3400 */
	char	s_stck_cd[11];	/* stock code 3500 */
	char	s_status[4];	/* status 3600 */
	char	s_99accno[19];	/* account 99 3700 */
	char	s_97accno[19];	/* account 97 3800 */
	short	s_school;	/* school code 3900*/
	char	s_gst[2];	/* item gst 4000 */
	short	s_rebate;	/* gst rebate 4100 */
	char	s_pst[2];	/* item pst 4200 */
	double	s_qty;		/* quantity 4300 */
	double	s_amount;	/* item amount 4400 */
}	S_item;

/* newapinvc.sth - header for C structure generated by PROFOM EDITOR */

struct	s_struct {
	char	s_pgm[11];	/* program name 100 */
	long	s_rundate;	/* date  YYYYFMMFDD 300 */
	char	s_fn[2];	/* function 500 */
	char	s_supp_cd[11];	/* supplier code 600 */
	char	s_invc_no[16];	/* reference number 700 */
	char	s_tr_type[3];	/* Tran Type 800 */
	short	s_field;	/* field 900 */
	long	s_po_no;	/* PO # 1100 */
	char	s_payee[11];	/* Payee 1200 */
	short	s_fund;		/* fund 1300 */
	long	s_invc_dt;	/* Tran date 1400*/
	long	s_due_dt;	/* Due Date  1500 */
	char	s_type[2];	/* type   1600 */
	double	s_gr_amt;	/* gross amount 1700 */
	char	s_gst_pf[2];	/* gst percent/full 1800 */
	double	s_gst_tax;	/* gst 1900 */
	char	s_pst_pf[2];	/* pst percent/full 2000 */
	double	s_pst_tax;	/* pst 2100 */
	double	s_net_amt;	/* net amount 2200 */
	short	s_period;	/* period 2300 */
	int	s_complete;	/* complete y/n 2400 */
	char	s_per_pr[11];	/* text holdback/discount 2500*/
	char	s_disc_pf[2];	/* discount percent/full 2600 */
	double	s_disc_per;	/* discount percent/amount 2700 */
	char	s_remarks[21];	/* remarks 2800 */
	long	s_chq_no;	/* cheque # 2900 */
	char	s_bank_acct[19];/* bank account 3000*/
	char	s_misc_hdr[2];
	char 	s_misc_gst[2];
	short	s_misc_reb;
	char	s_misc_pst[2];
	double	s_misc_amt;     /*  */
	short	s_page_no;	/* page # 3100 */
	char	s_hdr_line[2];	/* divsion line between header and items 3200 */
	S_item	s_items[PAGESIZE]; /* Start fld 3300 to fld 5400 */
	double	s_item_cost;	/* total item cost 5500 */
	char	s_amt_pr[11];	/* text holdback/discount 5600 */
	double	s_disc_amt;	/* discount amount 5700*/
	double	s_gst_amt;	/* gst amount 5800 */
	double	s_pst_amt;	/* pst amount 5900 */
	double	s_inv_total;	/* Invoice total 6000 */
	char	s_mesg[78];	/* message field 6100 */
	char	s_resp[2];	/* response field 6200 */
	};
static	struct	b_struct  b_sth;	/* PROFOM Batch Totals SCR Structure */
static	struct	s_struct  s_sth;	/* PROFOM Screen Structure */
static  char    save_amt_pr[11];	/* save label for disc amount */

static	Sch_rec		schl_rec ;	/* School Record */
static	Tr_hdr		gl99_hdr ;	/* Gl Trans. Header for 99 type Acnts */
static	Tr_item		gl99_itm ;	/* Gl Trans. Item for 99 type Acnts */
static	Tr_hdr		gl97_hdr ;	/* Gl Trans. Header for 97 type Acnts */
static	Tr_item		gl97_itm ;	/* Gl Trans. Item for 97 type Acnts */
static	Po_hdr		po_hdr ;	/* Purchase Order Header */
static	Po_item		po_item ;	/* Purchase Order Item */
static	St_mast		stck_rec ;	/* Stock Master Rec */
static	Alloc_rec	aloc_rec ;	/* Stock Allocation Record */

struct	stat_rec	sr ;
static  Tax_cal 	tax_cal;
static  double  	alloc_amt;
static  int 		Invoice_Crmemo_flag;
static  int 		global_mode = 0;
static	double  	ap_general_acnt,pst_payable,tax_clearance_acnt; 
static  double 		disc_pst,disc_gst;
static	long		default_date;
static 	double		remain_items;
static	double		remain_freight;
static 	double		remain_f_gst;

/*
*	Doubly linked list to maintain invoice items. Each node in this list
*	conatins one page full of invoice items. This list is freed only at the
*	time of exiting program. 
*/

typedef	struct Page {

	S_item	Items[PAGESIZE] ;	/* Items information */
	double	PoQty[PAGESIZE] ;	/* Quantity in PO */
	double	PoValue[PAGESIZE];	/* Value in PO */
	short   PoStatus[PAGESIZE];
	short	PoItemNo[PAGESIZE];	/* Corresponding PO Item# */
	double  DiscAmt[PAGESIZE];	/* discount amount */
	struct	Page	*PrevPage;	/* Ptr to Previous Page */
	struct	Page	*NextPage;	/* Ptr to Next Page */
	short	NoItems;		/* No of Items in this page */
	short	Pageno;			/* Page Number */

}	Page ;

static	Page	*FirstPage,		/* Address of the First Page */
		*CurPage,		/* Address of the Active Page */
		*InvcLast,		/* Address of Cur. Invoice Last page */
		*LastPage ;		/* Address of the Last page Memory
					   allocated */
/* Flags */
static	short	InvcExists ;		/* For CM, DM & RT, whether Invoice
					   exist with given Ref# */
static	short	Gl97Exists ;		/* 97 Type GL accounts are given */
static	short	TransType ;
static	char	Prev_99[18];		/* previous 99 GL account */
static	char	Prev_97[18];		/* previous 97 GL Account */

double	po_hdr_liq_tot = 0.0;		/* contains total of all items excluding
					   those added to a PO reference
					   invoice	*/
int	bal_items_added = 0;		/* flag to indicate that items have
					   been added to a PO referenced 
					   invoice */

/* The following two variables are required to pass to execute() */
extern	int	Argc;
extern	char	**Argv;

int	KeyAndHdrValidation(),  ItemsValidation() ;
int	HdrAndKeyWindowHelp(), ItemsWindowHelp() ;

double  Cal_Rebate(),Get_Commit_Amt(),Get_Commitf_Amt();
double  D_Roundoff();

void	free() ;
char	*malloc() ;

/*------------------------------------------------------------------------*/

TransEntry()
{
	int	err;
	err = Initialize() ;	/* Initialize Variables and PROFOM */

	if(err == NOERROR) err = Process();	/* Initiate Process */
	if(err == DBH_ERR || err == PROFOM_ERR) return(err) ;

	CloseProcess() ;

	return(NOERROR) ;
}	/* TransEntry() */
/*-------------------------------------------------------------------*/
/* Initialize Variables, PROFOM, DBH etc. */
static	int
Initialize()
{
	int	err ;
	static	short	first = 1;

	/*
	*	Initialize PROFOM & Screen
	*/

	if( first == 0 ) {/* Not First Time */
		Set1stScreen(0) ;	/* Display the batch total Screen */
		return(NOERROR) ;
	}

	first = 0 ;
	Set1stScreen(1) ;		/* Set SCR to batch total screen */

	STRCPY(b_sth.b_pgm,PROG_NAME);
	b_sth.b_rundate = get_date();	/* get Today's Date in YYYYMMDD format*/
	b_sth.b_fn[0]   = LV_CHAR ;
	b_sth.b_mesg[0] = HV_CHAR ;
	b_sth.b_resp[0] = HV_CHAR ;

	b_sth.b_batch_no = 0 ;

	err = InitBatchTotals() ;
	if(NOERROR != err) return(err) ;

	/* Initialize 2nd Screen Fields */

	STRCPY(s_sth.s_pgm,PROG_NAME);
	s_sth.s_rundate = get_date();	/* get Today's Date in YYYYMMDD format*/
	default_date = s_sth.s_rundate;
	s_sth.s_field   = HV_SHORT ;
	s_sth.s_mesg[0] = HV_CHAR ;
	s_sth.s_resp[0] = HV_CHAR ;

	s_sth.s_fn[0] = ADDREC ;	/* Always ADD */

	/*
	*	Initialize Variables
	*/

	FirstPage = NULL ;
	LastPage  = NULL ;

	return(NOERROR) ;
}	/* Initialize() */
/*-------------------------------------------------------------------*/
/* Get Fn: from user and call corresponding function */
static	int
Process()
{
	int err;

	if(b_sth.b_batch_no < 1) {	/* When no batch is active */
		err = NewBatch(1) ;
		if(err != NOERROR) return(err) ;
	}

	for( ; ; ){
		/* Get the Fn: option from the user */
		if((err = ReadFunction()) != NOERROR) return(err) ;

		err = ProcFunction() ;	/* Process Function */
		if(PROFOM_ERR == err || DBH_ERR == err)	return(err);
		if(NOACCESS == err) {
			fomen(e_mesg) ;
			get() ;
		}

		if(QUIT == err)		return(NOERROR) ;	/* Exit */

	}      /*   end of the for( ; ; )       */
}	/* Process() */
/*----------------------------------------------------------------*/
/* Free the Allocated memory before exiting the process */
static	int
CloseProcess()
{
	/* Free the linked list from the end */

	/* Free all the pages except the first page */
	for( ; LastPage != FirstPage ; ) {
		LastPage = LastPage->PrevPage ;
		free((char*)LastPage->NextPage) ;
		LastPage->NextPage = NULL ;
	}
	if(FirstPage != NULL)
		free((char*)FirstPage) ;
		
	FirstPage = LastPage = NULL ;

	return(NOERROR) ;
}	/* CloseProcess() */
/*----------------------------------------------------------------*/
/* Copy Current Scr to PROFOM status structure and set screen paramaters to
   Current Screen */
static	int
Set1stScreen(first)
int	first ;
{
	CurrentScr = (char*)&b_sth ;
	END_FLD    = SCR1_END_FLD ;

	/* move screen name to Profom status structure */
	STRCPY(sr.scrnam,NFM_PATH);
	strcat(sr.scrnam,SCR1_NAME) ;

	if( !first)
		ret( WriteFields(1,0) ) ;

	return(NOERROR) ;
}
/*----------------------------------------------------------------*/
/* Initialize Batch Totals() */
static	int
InitBatchTotals()
{
	int	i ;

	for(i = 0 ; i < 4 ; i++) {
		b_sth.b_items[i].b_trans     = 0 ;
		b_sth.b_items[i].b_gr_amt    = 0.0 ;
		b_sth.b_items[i].b_disc_amt  = 0.0 ;
		b_sth.b_items[i].b_hold_back = 0.0 ;
	}

	b_sth.b_tot.b_trans     = 0 ;
	b_sth.b_tot.b_gr_amt    = 0.0 ;
	b_sth.b_tot.b_disc_amt  = 0.0 ;
	b_sth.b_tot.b_hold_back = 0.0 ;

	return(WriteFields(ST_FLD, END_FLD-200)) ;
}	/* InitBatchTotals() */
/*----------------------------------------------------------------*/
/* Display the Function (Fn:) options and get the option from the user */
static	int
ReadFunction()
{
	/* Display Fn: options */
#ifdef ENGLISH
	fomer("A(dd), S(New Session), E(xit)");
#else
	fomer("R(ajouter), S(Nouvelle session), F(in)");
#endif
	/* Read Fn: field to get the option */
	sr.nextfld = FN_FLD ;
	fomrf( (char *)&b_sth );
	ret(err_chk(&sr));	/* Check for PROFOM error */

	return(NOERROR) ;
}	/* ReadFunction() */
/*----------------------------------------------------------------*/
/* Process the user selected Fn: option */
static	int
ProcFunction()
{
	int	err ;

	switch (b_sth.b_fn[0]) {
	case ADDREC  :			/* ADD */
		CHKACC(err, ADD, e_mesg) ;
		err = AddInvoice() ;
		if(err == PROFOM_ERR || err == DBH_ERR) return(err) ;
		ret( Set1stScreen(0) ) ;/* Set to batch Totals Screen */
		return(NOERROR) ;
	case NEWBATCH  :		/* Batch */
		CHKACC(err, ADD, e_mesg) ;
		return( NewBatch(0) ) ;
	case EXITOPT  :			/* Exit */
		return( QUIT ) ;
	}  /*   end of the switch statement */

	if(err < 0) return(err) ;	/* Security Error */

	return(NOERROR);
}	/* ProcFunction() */
/*------------------------------------------------------------*/
/* Get the New batch# from the user */
static	int
NewBatch(first)
int	first ;
{
	short	prev_batch ;

#ifdef ENGLISH
	STRCPY(b_sth.b_mesg,"Press ESC-F to Go Back to Fn:");
#else
	STRCPY(b_sth.b_mesg,"Appuyer sur ESC-F pour retourner a Fn:");
#endif
	ShowMesg();

	prev_batch = b_sth.b_batch_no;
	for( ; ; ) {
		sr.nextfld = BATCHNO_FLD;

		fomrf( (char*)&b_sth );
		ret(err_chk(&sr));

		if(sr.retcode == RET_USER_ESC) {
			if(sr.escchar[0] == 'f' || sr.escchar[0] == 'F') {
				b_sth.b_batch_no = prev_batch ;
				ret(WriteFields(BATCHNO_FLD, BATCHNO_FLD));

				b_sth.b_mesg[0] = HV_CHAR;
				ShowMesg();

				return(RET_USER_ESC);
			}
			continue ;
		}
		if(b_sth.b_batch_no < 1) {
#ifdef ENGLISH
			fomer("Invalid Batch#");
#else
			fomer("# de session invalide");
#endif
			continue ;
		}
		break ;
	}

	if( !first )	/* If not First Time */
		ret( InitBatchTotals() );

	b_sth.b_mesg[0] = HV_CHAR;
	ShowMesg();

	return(NOERROR);
}	/* NewBatch() */
/*----------------------------------------------------------------------*/
/* Adding Invoice. Get the unique Key, accept details and update the files */

AddInvoice()
{
	int	err;

	err = Set2ndScreen();	/* Set Invoice entry screen as Current SCR */
	if(err != NOERROR) return(err);

	/* If the Current Period == 0 (i.e before yearly closing), Add or
	   Change is not allowed */
	if(pa_rec.pa_cur_period == 0) {
#ifdef ENGLISH
		DispError("Not Allowed Before Yearly Closing...");
#else
		DispError("Pas permis avant la fermeture annuelle...");
#endif
		return(NOERROR) ;
	}

	for( ; ; ) {
		err = GetUniqueKey();
		if(err != NOERROR) return(err) ;

		CurPage   = NULL ;
		InvcLast  = NULL ;
		InvcExists = 0 ;

		/* Other than INVOICEs, check whether Invoice exists
		   with given Ref# */
		if( TransType != INVOICE) {
			err = GetInvoice(s_sth.s_supp_cd, s_sth.s_invc_no,
					T_INVOICE, BROWSE);
			if(ERROR == err) return(DBH_ERR) ;
			if(NOERROR == err) {
				InvcExists = 1 ;
				/* Read the Fund here itself, this case it 
				   won't be read thru KeyAndHdrValidation() */
				if((err = CheckFund(in_rec.in_funds)) < 0) 
					return(err) ;
			}
		}

		/* Clear the Screen */
		err = ClearScreen() ;
		if(err != NOERROR) return(err);

		err = GetDetails() ;		/* Read HDR & Items */
		if(PROFOM_ERR == err || DBH_ERR == err) return(err) ;
		if(err == DUPEREF) continue;
		if(err < 0 || CANCEL == err) {
			roll_back(e_mesg) ;	/* Unlock the locked Records */
			return(ClearScreen()) ;	/* Clear the Screen */
		}

		break;
	}
	ret( UpdateTotals() ) ;	/* Update Batch Totals */

	return(NOERROR);
}	/* AddInvoice() */
/*-------------------------------------------------------------------*/
/* Initialize 2nd Screen and set the current screen to that */
static	int
Set2ndScreen()
{
	int	err ;

	/*
	*	Initialize Screen & Screen Varibles
	*/

	CurrentScr = (char*)&s_sth ;
	END_FLD	   = SCR2_END_FLD ;

	/* move screen name to Profom status structure */
	STRCPY(sr.scrnam,NFM_PATH);
	strcat(sr.scrnam,SCR2_NAME) ;

	/* Move High Values to data fields and Display the screen */
	err = ClearScreen() ;
	if(NOERROR != err) return(err) ;

	return(NOERROR) ;
}	/* Set2ndScreen() */
/*----------------------------------------------------------------------*/
/* Read the Invoice Key. Check whether the invoice exists with this key */

GetUniqueKey()
{
	int	err ;

	for( ; ; ){
		err = ReadKey();
		if(ERROR == err) {		/* ESC-F */
			ClearScreen() ;
			return(ERROR) ;
		}
		if(NOERROR != err) return(err);

		/* check whether given key is already existing */
		err = GetInvoice(s_sth.s_supp_cd, s_sth.s_invc_no,
					s_sth.s_tr_type, BROWSE);
		if(ERROR == err) return(DBH_ERR) ;
		if(NOERROR == err){
#ifdef ENGLISH
			fomer("Given Key already in File - Please enter again");
#else
			fomer("Cle donnee deja dans le dossier - Reessayer");
#endif
			continue;
		}
		/* UNDEF */
		break;
	}

		return(NOERROR) ;
}	/* GetUniqueKey() */
/*------------------------------------------------------------*/
/* Read the Invoice Details from the User */

GetDetails()
{
	int	i ;

	i = ReadPoPayee();
	if(NOERROR != i) return(i) ;

	i = ReadHdr() ;
	if(NOERROR != i) return(i) ;

	i = ConfirmHdr() ;
	if(YES != i) return(i) ;

/**
	s_sth.s_hdr_line[0] = LV_CHAR;
**/
	s_sth.s_hdr_line[0] = ' ' ;
	WriteFields(HDR_LINE_FLD,HDR_LINE_FLD);

	/* Accept the items form user all cases except INVOICE with PO */
	if( TransType == INVOICE && s_sth.s_po_no) {
		i = MakePOItemsList() ;	/* Make linked list from PO Items */
		if(i < 0) return(i) ;
		Update_Pst_Gst_Cost();
		i = ShowItems(CurPage) ;	/* Show First Page of Items */
		if(i < 0) return(i) ;
	}
	else {
		i = Add_Balance_Items(ADD) ;
		if(NOERROR != i) return(i) ;
	}

	i = ConfirmItems() ;
	if(NOERROR != i) return(i) ;

	return(NOERROR) ;
}	/* GetDetails() */
/*----------------------------------------------------------------------*/
/* Update the Batch & Summary totals */
static	int
UpdateTotals()
{
	/*
	*	Update Batch Totals
	*/
	b_sth.b_items[TransType].b_trans++ ;
	b_sth.b_tot.b_trans++ ;
	b_sth.b_items[TransType].b_gr_amt  += (in_rec.in_amount + 
				in_rec.in_psttax  + in_rec.in_gsttax);
	b_sth.b_tot.b_gr_amt += (in_rec.in_amount + in_rec.in_psttax 
					+ in_rec.in_gsttax);
	if(supp_rec.s_type[0] == ORDINARY) {
		b_sth.b_items[TransType].b_disc_amt += in_rec.in_disc_amt ;
		b_sth.b_tot.b_disc_amt              += in_rec.in_disc_amt ;
	}else {
		b_sth.b_items[TransType].b_hold_back += in_rec.in_disc_amt ;
		b_sth.b_tot.b_hold_back              += in_rec.in_disc_amt ;
	}

	/*
	*	Update Summary Partition Totals
	*/
	if(in_rec.in_amount < -(DELTA_AMT)) {
	    if(supp_rec.s_type[0] == ORDINARY) {
		CR_totals[TransType].O_gross +=
		(in_rec.in_amount + in_rec.in_psttax  + in_rec.in_gsttax);
		CR_totals[TransType].O_disc += in_rec.in_disc_amt ;
	    }
	    else {
		CR_totals[TransType].C_gross +=
	            (in_rec.in_amount + in_rec.in_psttax  + in_rec.in_gsttax);
		CR_totals[TransType].C_disc += in_rec.in_disc_amt ;
	    }
	}
	else {
	    if(supp_rec.s_type[0] == ORDINARY) {
		DB_totals[TransType].O_gross +=
		 (in_rec.in_amount + in_rec.in_psttax  + in_rec.in_gsttax);
		DB_totals[TransType].O_disc += in_rec.in_disc_amt ;
	    }
		    else {
		DB_totals[TransType].C_gross +=
		    (in_rec.in_amount + in_rec.in_psttax  + in_rec.in_gsttax);
		DB_totals[TransType].C_disc += in_rec.in_disc_amt ;
	    }
	}

	/* If manual cheque is issued, then update the manual cheque totals */
	if(in_rec.in_chq_no) {
		ManCheques.no_trans++ ;
		ManCheques.tot_amt += in_rec.in_part_amt ;
	}

	TotalsUpdated = 1 ;	/* Print Summary Reports at the end */

	return(NOERROR) ;
}	/* UpdateTotals() */
/*----------------------------------------------------------------------*/
/* Get the invoice key from user. In ADD mode disable dup buffers, other
   modes enable dup buffers and show the current key as a default key */
static	int
ReadKey()
{
	int	i;
	char	supp_cd[sizeof(s_sth.s_supp_cd)];
	char	invc_no[sizeof(s_sth.s_invc_no)];
	char	type[sizeof(s_sth.s_tr_type)];

	/* Store fields to copy back when user gives ESC-F */
	STRCPY(supp_cd,s_sth.s_supp_cd) ;
	STRCPY(invc_no,s_sth.s_invc_no) ;
	STRCPY(type,s_sth.s_tr_type) ;

	s_sth.s_supp_cd[0] = LV_CHAR ;
	s_sth.s_invc_no[0] = LV_CHAR ;
	s_sth.s_tr_type[0] = LV_CHAR ;

#ifdef ENGLISH
	STRCPY(s_sth.s_mesg,"Press ESC-F to Go Back to Fn:");
#else
	STRCPY(s_sth.s_mesg,"Appuyer sur ESC-F pour retourner a Fn:");
#endif
	ShowMesg();

	i = ReadFields(KEY_START, KEY_END,
			KeyAndHdrValidation, HdrAndKeyWindowHelp,0,ESC_F) ;
	if(PROFOM_ERR == i || DBH_ERR == i) return(i) ;
	if(RET_USER_ESC == i){
		/* copy back key fields */
		STRCPY(s_sth.s_supp_cd,supp_cd) ;
		STRCPY(s_sth.s_invc_no,invc_no) ;
		STRCPY(s_sth.s_tr_type,type) ;

		ret( WriteFields(KEY_START, KEY_END) ) ;

		s_sth.s_mesg[0] = HV_CHAR;
		ShowMesg();

		roll_back(e_mesg) ;	/* Release Locked Records */
		return(ERROR) ;
	}

	return(NOERROR);
}	/*  ReadKey() */
/*------------------------------------------------------------*/
/* Get the Po and Payee Field details from user so that a dup.*/
/* Reference can be checked for payee */
ReadPoPayee()
{
	int	i;
	/* Change PROFOM logical attributes */
	for(i = HDR_ST_FLD ; i <= PAYEE_FLD ; i += 100)
		fomca1(i,19,0); /* Disable Dup control */

#ifdef ENGLISH
	STRCPY(s_sth.s_mesg,"Press ESC-F to Go Back to Fn:");
#else
	STRCPY(s_sth.s_mesg,"Appuyer sur ESC-F pour retourner a Fn:");
#endif
	ShowMesg();

	/* get default payee */
	fomca1(PAYEE_FLD,19,2);
	if(supp_rec.s_payee != '\0')
		STRCPY(s_sth.s_payee,supp_rec.s_payee);

	sr.nextfld = PAYEE_FLD ;
	sr.endfld  = PAYEE_FLD ;
	fomud((char*)&s_sth); 
	ret(err_chk(&sr)) ;
	/* Initialize Header fields with Low values */
	InitPoPayee(LV_CHAR, LV_LONG) ;
	i = ReadFields(HDR_ST_FLD,PAYEE_FLD,
			KeyAndHdrValidation, HdrAndKeyWindowHelp,ADD,ESC_F) ;
	if(PROFOM_ERR == i || DBH_ERR == i) return(i);
	if(DUPEREF == i) {
		InitPoPayee(HV_CHAR,HV_LONG) ;
		ret( WriteFields(HDR_ST_FLD, PAYEE_FLD) ) ;
		return(i) ;
	}
	if(RET_USER_ESC == i) {	/* ESC-F */
		InitPoPayee(HV_CHAR,HV_LONG) ;
		ret( WriteFields(HDR_ST_FLD, PAYEE_FLD) ) ;

		s_sth.s_mesg[0] = HV_CHAR ;
		ShowMesg() ;

		return(ERROR) ;
	}

	return(NOERROR);
} /* ReadPoPayee() */
/*------------------------------------------------------------*/
/* Get the Header details from user */
ReadHdr()
{
	int	i ;

	/* Change PROFOM logical attributes */
	for(i = FUND_FLD ; i <= HDR_END_FLD ; i += 100)
		fomca1(i,19,0); /* Disable Dup control */

#ifdef ENGLISH
	STRCPY(s_sth.s_mesg,"Press ESC-F to Go Back to Fn:");
#else
	STRCPY(s_sth.s_mesg,"Appuyer sur ESC-F pour retourner a Fn:");
#endif
	ShowMesg();

	/* Show Current Period as a default Period */
	fomca1(PERIOD_FLD, 19, 2) ;
	s_sth.s_period = pa_rec.pa_cur_period ;

	/* Show 1 as a default Fund */
	fomca1(FUND_FLD, 19, 2) ;
	s_sth.s_fund = 1 ;

	/* Show today as a default invoice date */
	fomca1(INVCDT_FLD, 19, 2) ;
	s_sth.s_invc_dt = default_date;

	/* Show Supplier Disc% as a default Disc% for INVOICE */
	if(TransType == INVOICE) {
		fomca1(DISCPER_FLD, 19, 2) ;    
		s_sth.s_disc_per = supp_rec.s_discount ;
		fomca1(REMARKS_FLD, 19, 2) ;
		STRCPY( s_sth.s_remarks, supp_rec.s_name) ;
	}
	sr.nextfld = FUND_FLD ;
	sr.endfld  = REMARKS_FLD ;
	fomud((char*)&s_sth); 
	ret(err_chk(&sr)) ;
	/* Initialize Header fields with Low values */
	InitHdr(LV_CHAR, LV_SHORT, LV_INT, LV_LONG, LV_DOUBLE) ;
	if(s_sth.s_po_no != 0) {
		s_sth.s_fund = po_hdr.ph_funds;
	}
	i = ReadFields(FUND_FLD, HDR_END_FLD,
			KeyAndHdrValidation, HdrAndKeyWindowHelp,ADD,ESC_F) ;
	if(PROFOM_ERR == i || DBH_ERR == i) return(i) ;
	if(RET_USER_ESC == i) {	/* ESC-F */
		InitHdr(HV_CHAR,HV_SHORT,HV_INT,HV_LONG,HV_DOUBLE) ;

		ret( WriteFields(FUND_FLD, HDR_LINE_FLD) ) ;

		s_sth.s_mesg[0] = HV_CHAR ;
		ShowMesg() ;

		return(ERROR) ;
	}

	return(NOERROR) ;
}	/* ReadHdr() */
/*-----------------------------------------------------------------------*/
/* Take the confirmation from user for the header part */

ConfirmHdr()
{
	int	err ;

	/* Options:
	   Add      - AEC
	*/

	for( ; ; ) {
#ifdef ENGLISH
		err = GetOption("A(dd Item), E(dit), C(ancel)", "AEC");
#else
		err = GetOption("R(ajouter article), M(odifier), A(nnuler)", "RMA");
#endif
		if(err == PROFOM_ERR) return(err) ;
		global_mode = err;
		switch(err) {
		case  ADDITEMS :
		    return(YES) ;
		case  EDIT  :
		    err = ChangeHdr();
		    break ;
		case  CANCEL :
#ifdef ENGLISH
		    err = GetOption("Confirm the Cancel (Y/N)?", "YN") ;
#else
		    err = GetOption("Confirmer l'annulation (O/N)?", "ON") ;
#endif
		    if(err == YES) return(CANCEL) ;
		    break ;
		}	/* switch err */

		if(err == PROFOM_ERR) return(err) ;
		if(err == DBH_ERR) return(err) ;
	}	/* for(; ; ) */
}	/* ConfirmHdr() */
/*----------------------------------------------------------------------*/
/* Changing Header fields. Accept fld to be changed and read that fld */

ChangeHdr()
	{
	int	i;

	/* Change PROFOM logical field attributes */
	for(i = HDR_ST_FLD ; i <= HDR_END_FLD ; i += 100)
		fomca1(i,19,2); /* enabling Dup control */

	/* Set Dup Buffers */
	sr.nextfld = HDR_ST_FLD;
	sr.endfld = HDR_END_FLD;
	fomud((char*)&s_sth);  /* Updating dup buffer */
	ret(err_chk(&sr));

	/* Get The Field to Be Modified */
#ifdef ENGLISH
	STRCPY(s_sth.s_mesg,"Enter RETURN to terminate Edit");
#else
	STRCPY(s_sth.s_mesg,"Appuyer sur RETURN pour terminer l'ajustement");
#endif
	ShowMesg() ;

	for( ; ; ) {
	    sr.nextfld = CHNG_FLD ;
	    fomrf((char*)&s_sth);
	    ret(err_chk(&sr));

	    if(s_sth.s_field == 0) break ;

		/* Validate Field Number */
		if(s_sth.s_field < 1 || s_sth.s_field > HDR_LAST_SNO)
			continue;

	    for( ; ; ) {
		i = ReadHdrFld(KeyAndHdrValidation, HdrAndKeyWindowHelp) ;
			/* Read Field */
		if(i == RET_USER_ESC) break;
		if(i == PROFOM_ERR) return(i) ;
		if(i == DBH_ERR) return(i) ;
		/* When Invoice date is changed make sure that due date is less
		   than that */
		if(s_sth.s_field == BANK_ACCT) {
			if(i == ERROR) {
				s_sth.s_field = CHEQUE_NO ;
				continue;
			}
		}
		if(i == ERROR) break ;
		if(s_sth.s_field == PO && s_sth.s_po_no != 0) {
			s_sth.s_complete = 1;
			ret(WriteFields(PO_COMP_FLD, PO_COMP_FLD));
		}
		if(s_sth.s_field == INVC_DT &&
				s_sth.s_due_dt < s_sth.s_invc_dt) {
			s_sth.s_field = DUE_DT ;
			continue ;
		}
		if(s_sth.s_field == CHEQUE_NO) {
			if(s_sth.s_chq_no != 0) {
				s_sth.s_field = BANK_ACCT ;
				continue ;
			}
			/* Validation Function Moves Blank to Acct# */
			sr.nextfld = BANKACCT_FLD ;
			fomwf((char*)&s_sth) ;
			break;
		}
		if(s_sth.s_field == BANK_ACCT) {
			/* Write Back Right Justified Account */
			sr.nextfld = BANKACCT_FLD ;
			fomwf((char*)&s_sth) ;
		}
		/* the way change Hdr works this edits the whole field 17 */
		if(s_sth.s_field == MISC_EXP){
			s_sth.s_field = MISC_REB;
			continue;
		}
		if(s_sth.s_field == MISC_REB){
			s_sth.s_field = MISC_PST;
			continue;
		}
		if(s_sth.s_field == MISC_PST){
			s_sth.s_field = MISC_AMT;
			continue;
		}
		break ;
	    }
	}	/* for( ; ; ) */

	s_sth.s_field = HV_SHORT ;
	fomwf((char*)&s_sth);
	ret(err_chk(&sr));

	s_sth.s_mesg[0] = HV_CHAR;
	ShowMesg();

	return(NOERROR);
}	/* ChangeHdr() */
/*-----------------------------------------------------------------------*/
/* Read the user selected Header field in change mode */

ReadHdrFld(Validate, WindowHelp)
int	(*Validate)() ;
int	(*WindowHelp)() ;
{
	int	err ;

/****** Validate  Field Number 
	if( s_sth.s_field < 1 || s_sth.s_field > HDR_LAST_SNO) return (ERROR) ;
******/

	/* When PO is given, fund & Type are copied from PO. Changes Not
	   allowed */
	if( (TransType == INVOICE && s_sth.s_po_no)
			&& (s_sth.s_field == FUND || s_sth.s_field == TYPE))
		return(ERROR) ;
	if(s_sth.s_chq_no == 0 && s_sth.s_field == BANK_ACCT) return(ERROR) ;

	/* When PO is not given, "PO Complete?" need not be asked */
	if(s_sth.s_po_no == PO && s_sth.s_field == POCOMPLETE) return(ERROR) ;
	/* When the Inventory system is not present, type is always DIRECT, i.e
	   no changes should be allowed to type, when inventory system is not
	   present */
	if(pa_rec.pa_stores[0] != YES && s_sth.s_field == TYPE)
		return(ERROR) ;

	/* If the Field# is between Gross Amt & Net Amt, always read upto
	   Disc Amt and update Net Amt */
	if(s_sth.s_field == NET_AMT) return(ERROR) ;
	if(s_sth.s_field >= GROSS_AMT && s_sth.s_field <= DISC_AMT) {
		return(ChangeTotal((int)s_sth.s_field)) ;
	}

	/* Set PROFOM nextfld */
	sr.nextfld = Convert_hdr_field((int)s_sth.s_field);

	for( ; ; ) {
		fomrf((char*)&s_sth);
		ret(err_chk(&sr));

		if(sr.retcode == RET_VAL_CHK) {
			err = (*Validate)(UPDATE) ;
			if(err == DBH_ERR) return(err) ;
			if(err == NOERROR) break;
			if(sr.curfld == CHQNO_FLD) return(ERROR);
			continue ;
		}
		if(sr.retcode == RET_USER_ESC){
			if(sr.escchar[0] == 'h' || sr.escchar[0] == 'H'){
				err = (*WindowHelp)() ;
				if(DBH_ERR == err || PROFOM_ERR == err)
					return(err) ;
				if(err == NOERROR) break;
			}else if(sr.escchar[0] == 'f' || sr.escchar[0] == 'F')
				return(RET_USER_ESC);
			continue;
		}
		if(sr.retcode != RET_NO_ERROR) continue;
		break ;
	}

	return(NOERROR) ;
}	/* ReadHdrFld() */
/*------------------------------------------------------------*/
/* Read Item Details from the User */
/* In BALANCE mode the the fields are filled with data from the first*/
/* item in the list. No taxes are applied and the user enters the*/   
/* amount. BALANCE MODE HAS BEEN REMOVED */

Add_Balance_Items(mode)
int mode;
{
	int	i, err,st_fld;
	if((mode == BALANCE) && (FirstPage == NULL)){
#ifdef ENGLISH
			fomer("No Items to Balance") ;
#else
			fomer("Pas d'articles a balancer") ;
#endif
	}

	bal_items_added = 1; 	/* CL 1991/01/03 */
	
/* If the last node of invoice is Partially filled then Show Page */
	if(InvcLast != NULL && InvcLast->NoItems < PAGESIZE ) {
		ret( ShowItems(InvcLast) ) ;
		i = InvcLast->NoItems ;
		CurPage = InvcLast ;
	}
	else {
		/* Calculate the page# */
		if(InvcLast != NULL) {
			i = PAGESIZE ;
			CurPage = InvcLast ;
		}
		else {
			s_sth.s_page_no = 1 ;
		ret( WriteFields(PAGENO_FLD,PAGENO_FLD) ) ;
		i = 0 ;
		}
	}
	err = CheckFund(s_sth.s_fund) ;
	if(err < 0) return(err);

	for( ; ; ) {
		if( PAGESIZE == i) {	/* Page Full */

			/* Move High Values to All items except first */
			for(i-- ; i > 0 ; i--)
				InitItem(mode,i, HV_CHAR, HV_SHORT, HV_DOUBLE) ;

		/* Calculate the page# */
			s_sth.s_page_no = InvcLast->Pageno + 1 ;

			ret( WriteFields(PAGENO_FLD, (END_FLD - 200)) ) ;

			i = 0 ;
		}

#ifdef ENGLISH
		STRCPY(s_sth.s_mesg,"Press ESC-F to Terminate");
#else
		STRCPY(s_sth.s_mesg,"Appuyer sur ESC-F pour terminer");
#endif
		ShowMesg();
			
		err = ReadItem(i,mode);		/* Read Each Item Line */
		if(PROFOM_ERR == err || DBH_ERR == err) return(err) ;
		if(NOERROR != err) break ;	/* ESC-F */

		if(0 == i)	/* First Item in the Page */
			if((err = MakeFreshPage()) < 0) return(err) ;

		/* Copy the Item to List */
		scpy((char*)&(CurPage->Items[i]), (char*)&(s_sth.s_items[i]),
			sizeof(S_item)) ;
		i++ ;
		CurPage->NoItems++;
		Update_Pst_Gst_Cost();
		/* Only one item can be added at a time in BALANCE mode */
		if(mode == BALANCE) break;
	}
	if(0 == i) ShowItems(CurPage) ;


	return(NOERROR) ;
}	/* Add_Balance_Items() */
/*----------------------------------------------------------------*/
/* Copy the items from given node to screen and display */

ShowItems(pageptr)
Page	*pageptr ;
{
	int	i ;

	if(pageptr != NULL) {
		/* Copy the all the items in given page to screen */
		scpy((char*)s_sth.s_items, (char*)pageptr->Items,
			(int)(pageptr->NoItems * sizeof(S_item)) );

		s_sth.s_page_no    = pageptr->Pageno ;
		i = pageptr->NoItems ;
	}
	else {
		s_sth.s_page_no    = HV_SHORT ;
		i = 0 ;
	}

	/* Move High Values to remaining Items */
	for( ; i < PAGESIZE ; i++ )
		InitItem(NOOP,i, HV_CHAR, HV_SHORT, HV_DOUBLE) ;

	ret( WriteFields( PAGENO_FLD, (END_FLD - 200)) );

	return(NOERROR) ;
}	/* ShowItems() */
/*-----------------------------------------------------------------------*/
/* Take the confirmation from user for the Items(i.e total screen) part */

ConfirmItems()
{
	int	err ;

	/* Options:
	   Add      - YADRENPTC
	*/

	for( ; ; ) {
	    /* if PO# is given, then items are copied from PO. Only changes
		allowed on item. Addition is not allowed */
/****************************
	    if(TransType == INVOICE && s_sth.s_po_no)
#ifdef ENGLISH
		err = GetOption(
			"Y(es), A(dd Item), D(el), E(dit), N(ext), P(rev), T(otal), C(ancel)",
			"YADENPTC");
#else
		err = GetOption(
         		"O(ui), R(aj.article), E(lim), M(odifier), S(uiv), P(rec), T(otal), A(nnuler)",
			"OREMSPTA");
#endif
	    else
********************************/
#ifdef ENGLISH
		err = GetOption(
"Y(es), A(dd), D(el), R(eactivate), E(dit), N(ext), P(rev), T(otal), C(ancel)",
"YADRENPTC");
#else
		err = GetOption(
"O(ui), R(aj art), E(lim), V(ivif), M(od), S(uiv), P(rec), T(otal), A(nnul)",
"OREVMSPTA");
#endif
	    if(err == PROFOM_ERR) return(err) ;
	    global_mode = err;

	    switch(err) {
	    case  YES  :
		err = WriteRecords() ;
		unlock_file(GLTRHDR);
		close_file(GLTRHDR);
		if(err == TAXPROBLEM) break; 
		if(err == ERROR) break ;
		if(err == LOCKED){
			err = CheckSupp(s_sth.s_supp_cd,UPDATE) ;
			if(err != NOERROR){ 
				if(err == LOCKED) break;
			}else if(s_sth.s_payee[0] != LV_CHAR) {
				STRCPY(payee_rec.s_supp_cd,s_sth.s_payee) ;
				err = get_supplier(&payee_rec, UPDATE, 0, e_mesg) ;
				if(err != NOERROR){ 
					if(err == LOCKED) break;
				}else
					break;
			}else 
				break;
		}
		return(err) ;
	    case  ADDITEMS :
		err = Add_Balance_Items(ADD) ;
		break ;
	    case  EDIT  :
	    case  DELITEM  :
	    case  REVIVEITEM  :
		err = ChangeItems(err);
		break ;
	    case  BALANCE :	/* CASE NOT POSSIBLE */
		err = Add_Balance_Items(BALANCE) ;
		break;
	    case  NEXTPAGE :
		if(InvcLast == NULL || CurPage == InvcLast) {
#ifdef ENGLISH
			fomer("No More Pages....") ;
#else
			fomer("Plus de pages....") ;
#endif
			continue ;
		}
		CurPage = CurPage->NextPage ;
		err = ShowItems(CurPage) ;
		break ;
	    case  PREVPAGE :
		if(InvcLast == NULL || CurPage == FirstPage) {
#ifdef ENGLISH
			fomer("No More Pages....") ;
#else
			fomer("Plus de pages....") ;
#endif
			continue ;
		}
		CurPage = CurPage->PrevPage ;
		err = ShowItems(CurPage) ;
		break ;
	    case  CANCEL :
#ifdef ENGLISH
		err = GetOption("Confirm the Cancel (Y/N)?", "YN") ;
#else
		err = GetOption("Confirmer l'annulation (O/N)?", "ON") ;
#endif
		if(err == YES) return(CANCEL) ;
		break ;
	    case  TOTAL_CHNG :
		err = ChangeTotal(GROSS_AMT) ;
		Update_Pst_Gst_Cost();
		break ;
	    }	/* switch err */

	    if(err == PROFOM_ERR) return(err) ;
	    if(err == DBH_ERR) return(err) ;
	    if(err == ERROR) return(err) ;
	}	/* for(; ; ) */
}	/* ConfirmItems() */
/*----------------------------------------------------------------------*/
/* Changing Items. Accept Item to be changed, deleted or revived.
   For change option read the item and others mark the Item */

ChangeItems(option)
int	option ;
{
	int	i ;

	for( ; ; ) {
#ifdef ENGLISH
		fomer("Enter RETURN to terminate Edit");
#else
		fomer("Appuyer sur RETURN pour terminer l'ajustement");
#endif

		/* Get The Item to Be Modified */
		sr.nextfld = CHNG_FLD ;
		fomrf( (char *)&s_sth );
		ret(err_chk(&sr));

		if(s_sth.s_field == 0) break ;	/* Changes completed */
		if( s_sth.s_field < 1 || CurPage == NULL ||
				s_sth.s_field > CurPage->NoItems)
			continue ;

		switch(option) {
		case	EDIT :
			if(strcmp(s_sth.s_items[s_sth.s_field-1].s_status,
							NOTACTIVE) == 0) {
#ifdef ENGLISH
				fomer("Item Is Deleted");
#else
				fomer("Article est elimine");
#endif
				continue ;
			}
			i = ReadItem((int)(s_sth.s_field-1), UPDATE) ;
			if(i == PROFOM_ERR) return(i) ;
			if(i == DBH_ERR) return(i) ;
			/* Copy the Item to List */
			scpy((char*)&(CurPage->Items[s_sth.s_field-1]),
				(char*)&(s_sth.s_items[s_sth.s_field-1]),
				sizeof(S_item)) ;
			break;
		case	DELITEM :
			if(strcmp(s_sth.s_items[s_sth.s_field-1].s_status,
							NOTACTIVE) == 0){
#ifdef ENGLISH
				fomer("Already Deleted");
#else
				fomer("Deja elimine");
#endif
				get();
			}else {
				STRCPY(s_sth.s_items[s_sth.s_field-1].s_status,
								NOTACTIVE) ;
				STRCPY(CurPage->Items[s_sth.s_field-1].s_status,
								NOTACTIVE) ;
				sr.nextfld = ITEM_ST_FLD +
					((s_sth.s_field-1)  * STEP) + 300 ;
				fomwf((char*)&s_sth) ;
				ret(err_chk(&sr)) ;
			}
			break;
		case	REVIVEITEM :
			if(strcmp(s_sth.s_items[s_sth.s_field-1].s_status,
							ACTIVE) == 0){
#ifdef ENGLISH
				fomer("Item is Not Deleted");
#else
				fomer("Article n'est pas elimine");
#endif
				get();
			}else {
				STRCPY(s_sth.s_items[s_sth.s_field-1].s_status,
								ACTIVE) ;
				STRCPY(CurPage->Items[s_sth.s_field-1].s_status,
								ACTIVE) ;
				sr.nextfld = ITEM_ST_FLD +
					((s_sth.s_field-1)  * STEP) + 300 ;
				fomwf((char*)&s_sth) ;
				ret(err_chk(&sr)) ;
			}
			break;

		}	/* Switch */
		Update_Pst_Gst_Cost();
		sr.nextfld =ITEM_ST_FLD + (s_sth.s_field * STEP) - 100;
		fomwf((char*)&s_sth) ;
		ret(err_chk(&sr)) ;
	}	/* for( ; ; ) */

	s_sth.s_field = HV_SHORT ;
	fomwf( (char *)&s_sth );
	ret(err_chk(&sr)) ;

	s_sth.s_mesg[0] = HV_CHAR;
	ShowMesg();

	return(NOERROR);
}	/* ChangeItems() */
/*-----------------------------------------------------------------------*/
/* This function allows user to change Total Fields in HDR. When user selects
   any fld between Gross Amt and Disc Amt, this function reads that fld to
   Net amt amount   
*/
static	int
ChangeTotal(field)
int	field;
{
	int	i ,start,end;

	start = sr.nextfld = Convert_hdr_field(field);
	end = sr.endfld  = Convert_hdr_field(DISCPER_FLD);

	/* Change PROFOM logical field attributes */
	for(i = sr.nextfld ; i <= sr.endfld ; i += 100)
		fomca1(i,19,2); /* enabling Dup control */

	fomud((char*)&s_sth);  /* Updating dup buffer */
	ret(err_chk(&sr));

	/* break is not there, to fall thru */
	switch(field) {
	case GROSS_AMT:
		s_sth.s_gr_amt  = LV_DOUBLE ;
	case GST_TAX:
		s_sth.s_gst_pf[0] = LV_CHAR;
		s_sth.s_gst_tax = LV_DOUBLE;
	case PST_TAX:
		s_sth.s_pst_pf[0] = LV_CHAR;
		s_sth.s_pst_tax = LV_DOUBLE ;
	/* Update net amount  on screen */
	case DISC_AMT:
		s_sth.s_disc_per = LV_DOUBLE;
		s_sth.s_disc_pf[0] = LV_CHAR;
		break;
	default:
		break;
	}

	i = ReadFields(start,end,KeyAndHdrValidation,HdrAndKeyWindowHelp,
								UPDATE,0) ;

	if(PROFOM_ERR == i || DBH_ERR == i) return(i) ;

	Update_Net_Amt();

	return(NOERROR) ;
}	/* ChangeTotal() */
/*------------------------------------------------------------*/
/* Read details of given item# */

ReadItem(item_no,mode)
int	item_no ;
int	mode ;	/* ADD or UPDATE */
{
	int	i, j, k ;
	int	st_fld ;
	int	end_fld ;

	st_fld  = ITEM_ST_FLD + (STEP * item_no) ;
	end_fld = st_fld + STEP - 100 ;

	/* If mode == UPDATE turn ON dup control, else OFF */
	for(i = st_fld ; i <= end_fld ; i += 100)
		fomca1(i, 19, (mode == UPDATE) ? 2 : 0) ;

	if(UPDATE == mode) {
		sr.nextfld = st_fld ;
  		sr.endfld  = end_fld ;   
		fomud( (char *)&s_sth );	/* Updating dup buffer */
		ret(err_chk(&sr));
	} 

	/* Initialize Reading Item with Low values */
	InitItem(mode,item_no,LV_CHAR,LV_SHORT,LV_DOUBLE);
	if(mode == BALANCE) Init_Balance_Item(&s_sth.s_items[item_no]); 

	if((ADD == mode) || (mode == BALANCE))
		 s_sth.s_items[item_no].s_sno = item_no + 1;

	if(s_sth.s_type[0] == BULK) {	/* INV general acct is item acct */
		STRCPY(s_sth.s_items[item_no].s_99accno, ctl_rec.inv_acnt); 
	}
	/* else * Copy default account #'s from first item */
/********** Do not want defaults, does not work on the pages after the 
first one. When in edit mode the account number gets switch to the first 
account. L.R.
		if(((mode == ADD) && (FirstPage != NULL) && (item_no > 0)) || 
		   s_sth.s_page_no != 1){
			fomca1(st_fld+ACNO99_FLD, 19, 2) ;
			fomca1(st_fld+ACNO97_FLD, 19, 2) ;
			STRCPY(s_sth.s_items[item_no].s_99accno,
					 FirstPage->Items[0].s_99accno);
			STRCPY(s_sth.s_items[item_no].s_97accno,
					FirstPage->Items[0].s_97accno);
*******These were previously commented out.
			strcpy(s_sth.s_items[item_no].s_99accno,Prev_99);
			strcpy(s_sth.s_items[item_no].s_97accno,Prev_97);
*******

			WriteFields(st_fld+ACNO99_FLD,st_fld+ACNO97_FLD);
			s_sth.s_items[item_no].s_99accno[0] = LV_CHAR;
			s_sth.s_items[item_no].s_97accno[0] = LV_CHAR;
		}
*******/

	/* The differences between ADD and BALANCE are completed. Balance */
	/* mode is now Add mode to complete write to file.                */  

	i = ReadFields(st_fld, end_fld, ItemsValidation, ItemsWindowHelp,
				mode, ESC_F);
	if(PROFOM_ERR == i || DBH_ERR == i) return(i) ;
	if(RET_USER_ESC == i) {
		if(ADD == mode || mode == BALANCE) {
			InitItem(mode,item_no, HV_CHAR, HV_SHORT, HV_DOUBLE) ;
			ret( WriteFields(st_fld, end_fld) );
			return(ERROR) ;
		}
		/*
		* When user gives ESC-F while changing fields, assumption is
		* he completed his changes, and reamaining fields are same as
		* old. But, at this point STH will be having low values in the 
		* remaining fields. So move the old values from the linked list.
		*/

		/* Get Offset of the begining field of the Item in 'k' */
		fomfp(st_fld,&k,&j) ;
		/* Offset to the field where ESC-F Pressed in 'j' */
		fomfp(sr.curfld,&j,&i);
		i =  j - k ;	/* Offset within item */

		j = sizeof(S_item) - i ;  /* Length to copy */
       		scpy((char *)(&s_sth.s_items[item_no])+i,
			(char*)(&CurPage->Items[item_no])+i, j);

		return(ERROR); 
	}
	/* Set previous account numbers */
	strcpy(Prev_99,s_sth.s_items[item_no].s_99accno);
	strcpy(Prev_97,s_sth.s_items[item_no].s_97accno);
	return(NOERROR);
}	/* ReadItem() */
/*----------------------------------------------------------------*/
/* Validation function() for Key and Header fields when PROFOM returns
  RET_VAL_CHK */

KeyAndHdrValidation(mode)
int	mode ;
{
	int	err, cur_fld, end_fld ;

	switch(sr.curfld){
	case	SUPPCD_FLD	:	/* Supplier Cd: */
		err = CheckSupp(s_sth.s_supp_cd,UPDATE) ;
		if(DBH_ERR == err) return(err) ;
		if(err < 0) {
		    if(UNDEF == err) {
#ifdef ENGLISH
			err=GetOption("Supplier not found. Want to add one (Y/N)?", "YN");
#else
			err=GetOption("Fournisseur pas retrouve. Desirez-vous en ajouter un (O/N)?", "ON");
#endif
			if(err < 0 ) return(err);
			if( err == YES ){
				err = execute( SUPPMAINT, Argc, Argv );
				if( err<0 )	return(err);
				redraw();
			}
		    }
		    else
			fomer(e_mesg) ;
		    s_sth.s_supp_cd[0] = LV_CHAR ;
		    return(ERROR) ;
		}
		fomer(supp_rec.s_name) ;
		break ;
	case	REFNO_FLD	:	/* Ref#: */
		if(s_sth.s_invc_no[0] == '\0') {
#ifdef ENGLISH
			fomer("Invalid Ref#");
#else
			fomer("# de reference invalide");
#endif
			s_sth.s_invc_no[0] = LV_CHAR ;
			return(ERROR) ;
		}
		break ;
	case	TRTYPE_FLD	:	/* Trans. Type: */
		/* Check for validity of type */
		if((TransType = CheckTransType(s_sth.s_tr_type)) == ERROR) {
			s_sth.s_tr_type[0] = LV_CHAR ;
			return(ERROR) ;
		}
		break ;
	case	PONO_FLD	:	/* PO#: */
		if(s_sth.s_po_no == 0) {
			s_sth.s_complete = HV_INT ;
			break ;
		}

		err = CheckPOValidity() ;
		if(PROFOM_ERR == err || DBH_ERR == err) return(err) ;
		if(err < 0){
			s_sth.s_po_no = LV_LONG ;
			return(ERROR) ;
		}
		err = GetPayee() ;
		if(err < 0) return(err);

		break ;

	case	PAYEE_FLD	:	/* payee Cd: */
		if(s_sth.s_payee[0] == '\0'){
			sr.curfld += 100;
			break;
		}

  		Right_Justify_Numeric(s_sth.s_payee,sizeof(s_sth.s_payee)-1);
		STRCPY(payee_rec.s_supp_cd,s_sth.s_payee) ;
		err = get_supplier(&payee_rec, UPDATE, 0, e_mesg) ;
		if(ERROR == err) return(DBH_ERR) ;
		if(err < 0) {
		    if(UNDEF == err) {
#ifdef ENGLISH
			err=GetOption("Payee not found. Want to add one (Y/N)?", "YN");
#else
			err=GetOption("Beneficiaire pas retrouve. Desirez-vous en ajouter un (O/N)?", "ON");
#endif
			if(err < 0 ) return(err);
			if( err == YES ){
				err = execute( SUPPMAINT, Argc, Argv );
				if( err<0 )	return(err);
				redraw();
			}
		    }
		    else
			fomer(e_mesg) ;
		    s_sth.s_payee[0] = LV_CHAR ;
		    return(ERROR) ;
		}
		/* check whether given key is already existing for payee */
		err = GetInvoice(s_sth.s_payee, s_sth.s_invc_no,
					s_sth.s_tr_type, BROWSE);
		if(ERROR == err) return(DBH_ERR) ;
		if(NOERROR == err){
#ifdef ENGLISH
			fomer("Given Key already in File - Please enter again");
#else
			fomer("Cle donnee deja dans le dossier - Reessayer");
#endif
			return(DUPEREF);
		}

		fomer(payee_rec.s_name);
		break ;


	case	PERIOD_FLD	:	/* Period: */
		/* Period should belong to Current Year, should be <= Current
		   period, and in case of previous period is should not < 
		   allowed open periods */
		if(s_sth.s_period < 1 || s_sth.s_period > pa_rec.pa_cur_period
			|| s_sth.s_period <
				(pa_rec.pa_cur_period - pa_rec.pa_open_per) ) {
#ifdef ENGLISH
			fomer("Invalid Period");
#else
			fomer("Periode invalide");
#endif
			s_sth.s_period = LV_SHORT ;
			return(ERROR) ;
		}
		break ;
	case	FUND_FLD	:	/* Fund: */
		err = CheckFund(s_sth.s_fund) ;
		if(DBH_ERR == err) return(err) ;
		if(ERROR == err) {
			s_sth.s_fund = LV_SHORT ;
			return(ERROR) ;
		}
		fomer(ctl_rec.desc) ;
		break ;
	case	INVCDT_FLD	:	/* Invoice Date: */
		/* Invoice Date shouldn't be future date */
		if(s_sth.s_invc_dt > s_sth.s_rundate) {
#ifdef ENGLISH
			fomer("Date Can't be Future Date");
#else
			fomer("Date ne peut pas etre ulterieure");
#endif
			s_sth.s_invc_dt = LV_LONG ;
			return(ERROR) ;
		}
		default_date = s_sth.s_invc_dt;
	/* show "pa_due_days" days after invoice date as default due date */
		if(mode == ADD) {
			end_fld = sr.endfld ;
			cur_fld = sr.curfld ;

			fomca1(DUEDT_FLD,19,2) ;    /* Turn ON Dup Control */
			s_sth.s_due_dt = date_plus(s_sth.s_invc_dt,
						pa_rec.pa_due_days_ap) ;

			sr.nextfld = DUEDT_FLD ;
			sr.endfld  = DUEDT_FLD ;
			fomud((char*)&s_sth) ;
			ret(err_chk(&sr)) ;
			s_sth.s_due_dt = LV_LONG ;
			sr.curfld  = cur_fld ;
			sr.endfld  = end_fld ;
		}
		break ;
	case	DUEDT_FLD	:	/* Due Date: */
		if(s_sth.s_due_dt < s_sth.s_invc_dt) {
#ifdef ENGLISH
			fomer("Due date can't be before Trans Date");
#else
			fomer("Echeance ne peut pas etre plus tot que la date de transaction");
#endif
			s_sth.s_due_dt = LV_LONG;
			return(ERROR) ;
		}
		break ;
	case	TYPE_FLD	:	/* Type: */
		if(s_sth.s_type[0] != DIRECT && s_sth.s_type[0] != BULK &&
			s_sth.s_type[0] != NON_BULK) {
#ifdef ENGLISH
			fomer(
"Valid types are D(irect Charge), B(ulk Order), N(on Bulk Order)");
#else
			fomer(
"Genres valables sont C(harge directe), E(n vrac), S(tock reserve)");
#endif
			s_sth.s_type[0] = LV_CHAR ;
			return(ERROR) ;
		}
		break ;
	case	GRAMT_FLD	:	/* Gross Amt */
		if(s_sth.s_gr_amt < DELTA_AMT) {
#ifdef ENGLISH
			fomer("Must be Greater than Zero");
#else
			fomer("Ne peut pas etre zero");
#endif
			s_sth.s_gr_amt = LV_DOUBLE ;
			return(ERROR) ;
		}
		break ;
	case	GST_PF:
		if((s_sth.s_gst_pf[0] != PARTIAL) &&
		   		(s_sth.s_gst_pf[0] != FULL_AMT) && 
		  				 (s_sth.s_gst_pf[0] != NULL)){
			s_sth.s_gst_pf[0] = LV_CHAR;
#ifdef ENGLISH
	fomer("Valid codes are P(artial), F(ull amount), or <RETURN>");
#else
	fomer("Codes valides sont P(artiel), M(ontant en entier), ou pesez <Return>");
#endif
			return(ERROR);
		}
		if(s_sth.s_gst_pf[0] != PARTIAL){
			s_sth.s_gst_tax = 0.0;
		}
		if(s_sth.s_gst_pf[0] == NULL) sr.curfld += 100;
		break;
	case	PST_PF:
		if((s_sth.s_pst_pf[0] != PARTIAL) &&
  		   		(s_sth.s_pst_pf[0] != FULL_AMT) &&
						 (s_sth.s_pst_pf[0] != NULL)){
			s_sth.s_pst_pf[0] = LV_CHAR;
#ifdef ENGLISH
	fomer("Valid codes are P(artial), F(ull amount), or <RETURN>");
#else
	fomer("Codes valides sont P(artiel), M(ontant en entier), ou pesez <Return>");
#endif
			return(ERROR);
		}
		if(s_sth.s_pst_pf[0] != PARTIAL){
			s_sth.s_pst_tax = 0.0;
		}
		Update_Net_Amt();
		if(s_sth.s_pst_pf[0] == NULL) sr.curfld += 100;
		break;
	case 	PST_TAX_FLD		:
		if((s_sth.s_pst_pf[0] == PARTIAL) &&
					 (s_sth.s_pst_tax < DELTA_AMT)){
#ifdef ENGLISH
	fomer("Partial code requires PST amount > 0.0");
#else
	fomer("Code partiel requiert un montant de TVP > 0"); 
			s_sth.s_pst_pf[0] = LV_CHAR;
			return(ERROR);
#endif
		}		
		/* Update net amount  on screen */
		Update_Net_Amt();
		break;
	case 	GST_TAX_FLD		:
		if((s_sth.s_gst_pf[0] == PARTIAL) &&
					 (s_sth.s_gst_tax < DELTA_AMT)){
#ifdef ENGLISH
	fomer("Partial code requires GST amount > 0.0");
#else
	fomer("Code partiel requiert un montant de TPS > 0"); 
#endif
			s_sth.s_gst_pf[0] = LV_CHAR;
			return(ERROR);
		}		
		break ;
	case	DISC_PF:
		if((s_sth.s_disc_pf[0] != PERCENT) &&
					(s_sth.s_disc_pf[0] != AMOUNT) &&
						(s_sth.s_disc_pf[0] != NULL)){
			s_sth.s_disc_pf[0] = LV_CHAR;
#ifdef ENGLISH
			fomer("Valid codes are P(ercent), A(mount)");
#else
			fomer("Codes valides sont P(ourcentage), M(ontant)");
#endif
			return(ERROR);
		}
		if (s_sth.s_disc_pf[0] == NULL){
			sr.curfld += 100;
			s_sth.s_disc_per = 0.0;
		}
		break;

	case	DISCPER_FLD	:	/* Disc% */
		if(s_sth.s_disc_pf[0] == PERCENT){
			if(s_sth.s_disc_per >= 100.0) {
#ifdef ENGLISH
				fomer("Percent must be less than 100");
#else
				fomer("Pourcentage doit etre moins de 100");
#endif
				s_sth.s_disc_pf[0] = LV_CHAR;
				s_sth.s_disc_per = LV_DOUBLE;
				sr.curfld = DISC_PF;
			}
		}else if(s_sth.s_disc_pf[0] == AMOUNT)
			if(s_sth.s_disc_per > s_sth.s_gr_amt){ 
#ifdef ENGLISH
				fomer("Discount must be less than gross amount");
#else
				fomer("Montant escompte doit etre moins que le montant brut");
#endif
				s_sth.s_disc_pf[0] = LV_CHAR;
				s_sth.s_disc_per = LV_DOUBLE;
				sr.curfld = DISC_PF;
			}
		break ;
	case    NETAMT_FLD	:	/* Net Amount */
		break;

	case	PO_COMP_FLD:
		break;

	case	CHQNO_FLD	:	/* Cheque# */
		if(mode==ADD){
			cur_fld = sr.curfld;
			end_fld = sr.endfld;
			/* Added by J.Prescott dec 29/92  */
			/* Show GST as default from param file */
			fomca1(MISC_GST_FLD, 19, 2);  
			sr.nextfld = MISC_GST_FLD;
			sr.endfld = MISC_GST_FLD;
			s_sth.s_misc_gst[0] = pa_rec.pa_gst_tax[0]; 
			fomud((char*)&s_sth);
			ret(err_chk(&sr)); 
			s_sth.s_misc_gst[0] = LV_CHAR;

			/* Show rebate as default from control */
			fomca1(MISC_REB_FLD, 19, 2); 
			sr.nextfld = MISC_REB_FLD;
			sr.endfld = MISC_REB_FLD;
			s_sth.s_misc_reb = ctl_rec.rebate;  
			fomud((char*)&s_sth);
			ret(err_chk(&sr)); 
			s_sth.s_misc_reb = LV_SHORT;

			/* Show PST as default from parameter file */
			fomca1(MISC_PST_FLD, 19, 2); 
			sr.nextfld = MISC_PST_FLD;
			sr.endfld = MISC_PST_FLD;
			s_sth.s_misc_pst[0] = pa_rec.pa_pst_tax[0];
			fomud((char*)&s_sth);
			ret(err_chk(&sr)); 
			s_sth.s_misc_pst[0] = LV_CHAR;
			
			sr.curfld = cur_fld;
			sr.endfld = end_fld;
		}
		if(s_sth.s_chq_no == 0) {
		    STRCPY(s_sth.s_bank_acct, " ") ;	/* Skip Reading Acct# */
		    break ;
		}
		/* Move Bank1 Account as a default account */
		cur_fld = sr.curfld ;
		end_fld = sr.endfld ;

		fomca1(BANKACCT_FLD, 19, 2) ;	/* Enable Dup Buffers */
		sr.nextfld = BANKACCT_FLD ;
		sr.endfld  = BANKACCT_FLD ;
		STRCPY(s_sth.s_bank_acct, ctl_rec.bank1_acnt) ;
		fomud((char*)&s_sth) ;
		ret(err_chk(&sr)) ;
		s_sth.s_bank_acct[0] = LV_CHAR ;
		sr.curfld  = cur_fld ;
		sr.endfld  = end_fld ;
		break ;
	case	BANKACCT_FLD	:	/* Bank Acct */
		if(acnt_chk(s_sth.s_bank_acct) == ERROR) {
#ifdef ENGLISH
			fomer("Invalid Account Number");
#else
			fomer("Numero de compte invalide");
#endif
			s_sth.s_bank_acct[0] = LV_CHAR ;
			return(ERROR) ;
		}
		/* Account# has to be One of the 2 Bank Accounts */
		if(strcmp(s_sth.s_bank_acct,ctl_rec.bank1_acnt) &&
				strcmp(s_sth.s_bank_acct,ctl_rec.bank2_acnt) ) {
#ifdef ENGLISH
			sprintf(e_mesg,
				"Valid Account Numbers.. BANK1: %s  BANK2: %s",
				ctl_rec.bank1_acnt, ctl_rec.bank2_acnt );
#else
			sprintf(e_mesg,
				"Numeros de comptes valables.. BANQUE1: %s  BANQUE2: %s",
				ctl_rec.bank1_acnt, ctl_rec.bank2_acnt );
#endif
			fomer(e_mesg) ;
			s_sth.s_bank_acct[0] = LV_CHAR ;
			return(ERROR) ;
		}
		err = CheckGlAcnt(s_sth.s_fund,s_sth.s_bank_acct,99,BROWSE) ;
		if(DBH_ERR == err) return(err) ;
		if(ERROR == err) {
			s_sth.s_bank_acct[0] = LV_CHAR ;
			return(ERROR) ;
		}
		chqhist.ch_funds = s_sth.s_fund;
		STRCPY( chqhist.ch_accno, s_sth.s_bank_acct );
		chqhist.ch_chq_no = s_sth.s_chq_no;
		err = get_chqhist(&chqhist,BROWSE,0,e_mesg);
		if( err==NOERROR ){
			fomen("Cheque Number Already Exists In Cheque History");
			get();
			s_sth.s_chq_no = LV_LONG ;
			s_sth.s_bank_acct[0] = LV_CHAR ;
			sr.curfld -= 100;
			return(ERROR);
		}
		break ;
	case MISC_GST_FLD:
		if(s_sth.s_misc_gst[0] != TAXABLE &&
		   s_sth.s_misc_gst[0] != EXEMPT) {
#ifdef ENGLISH
			fomer("Valid Tax Codes are T(axable), E(xempt) ");
#else
			fomer("Codes valables de taxe sont T(axable), E(xempt)");
#endif
			s_sth.s_misc_gst[0] = LV_CHAR;
			return(ERROR);
		}
		break;
	case MISC_PST_FLD:
		if(s_sth.s_misc_pst[0] != TAXABLE &&
		   s_sth.s_misc_pst[0] != EXEMPT) {
#ifdef ENGLISH
			fomer("Valid Tax Codes are T(axable), E(xempt)");
#else
			fomer("Codes valables de taxe sont T(axable), E(xempt)");
#endif
			s_sth.s_misc_pst[0] = LV_CHAR;
			return(ERROR);
		}
		break;
	default :
#ifdef ENGLISH
		sprintf(e_mesg,"No Validity Check For Field#  %d",sr.curfld);
#else
		sprintf(e_mesg,"Pas de controle de validite pour le Champ#  %d",sr.curfld);
#endif
		fomen(e_mesg);
		get();
		return(ERROR) ;
	}	/* Switch sr.curfld */

	return(NOERROR) ;
}	/* KeyAndHdrValidation() */
/*----------------------------------------------------------------*/
/* Validation function() for Item fields when PROFOM returns RET_VAL_CHK */

ItemsValidation(mode)
int	mode ;
{
	double	temp, diff, CumulativeUncommited() ;
	int	err, cur_fld, end_fld ;
	int	fld_no, item_no ;
	double	junk;

	/* Caluculate item# ans Fld# within item */
	item_no = (sr.curfld - ITEM_ST_FLD) / STEP ;
	fld_no  = (sr.curfld - ITEM_ST_FLD) % STEP ;

	switch(fld_no){
	case STCKCD_FLD :		/* Stock Code Field */
		/* If Direct Charge, No validation */
		if(s_sth.s_type[0] == DIRECT) {
			if( s_sth.s_items[item_no].s_stck_cd[0] == LV_CHAR)
				s_sth.s_items[item_no].s_stck_cd[0] = HV_CHAR; 
			break ;
		}
		
		err = CheckStock(s_sth.s_items[item_no].s_stck_cd) ;
		if(DBH_ERR == err) return(DBH_ERR) ;
		if(err < 0) {
			s_sth.s_items[item_no].s_stck_cd[0] = LV_CHAR ;
			return(ERROR) ;
		}
		/*
		else {
			fomen("Just before memset");
			get();
			memset(s_sth.s_items[item_no].s_stck_cd[0],' ',
				sizeof(s_sth.s_items[0].s_stck_cd)-1);
			fomen("Just after memset");
			get();
		}
		*/
		fomer(stck_rec.st_desc) ;
		break ;
	case SCHOOL_FLD :	/* School Fields */
		schl_rec.sc_numb = s_sth.s_items[item_no].s_school ;
		err = get_sch(&schl_rec, BROWSE, 0, e_mesg) ;
		if(ERROR == err) return(DBH_ERR) ;
		if(err < 0) {
			fomer(e_mesg) ;
			s_sth.s_items[item_no].s_school = LV_SHORT ;
			return(ERROR) ;
		}
		fomer(schl_rec.sc_name) ;
		break ;
	case ACNO99_FLD :		/* GL Account#(99) Fields */
		if(acnt_chk(s_sth.s_items[item_no].s_99accno) == ERROR) {
#ifdef ENGLISH
			fomer("Invalid Account Number");
#else
			fomer("Numero de compte invalide");
#endif
			s_sth.s_items[item_no].s_99accno[0] = LV_CHAR ;
			return(ERROR) ;
		}
		err = CheckGlAcnt(s_sth.s_fund,s_sth.s_items[item_no].s_99accno,
					99,UPDATE) ;
		if(DBH_ERR == err) return(DBH_ERR) ;
		if(err < 0) {
			s_sth.s_items[item_no].s_99accno[0] = LV_CHAR ;
			return(err) ;
		}
		if((pa_rec.pa_stores[0] == YES) && 
		   (s_sth.s_type[0] == DIRECT || s_sth.s_type[0] == NON_BULK)) {
			if(strcmp(s_sth.s_items[item_no].s_99accno,
						ctl_rec.inv_acnt)==0){
#ifdef ENGLISH
				fomer("Cannot use Inventory General account for Direct or Non-Bulk PO's");
#else
				fomer("Compte Inventaire Gen. ne peut etre utilise pour BC dir. ou de stock reserve");
#endif
				s_sth.s_items[item_no].s_99accno[0] = LV_CHAR ;
				return(ERROR) ;
			}
		}
		fomer(gl_rec.desc) ;
		break ;
	case ACNO97_FLD :		/* GL Account#(97) Fields */
		/* 97 Account can be ignored */
		if(s_sth.s_items[item_no].s_97accno[0] == '\0') {
			/* To skip reading again, Position to nextfld */
			sr.curfld += 100 ;
			break ;
		}
		if(acnt_chk(s_sth.s_items[item_no].s_97accno) == ERROR) {
#ifdef ENGLISH
			fomer("Invalid Account Number");
#else
			fomer("Numero de compte invalide");
#endif
			s_sth.s_items[item_no].s_97accno[0] = LV_CHAR ;
			return(ERROR) ;
		}
		err = CheckGlAcnt(s_sth.s_fund,s_sth.s_items[item_no].s_97accno,
					97,UPDATE) ;
		if(DBH_ERR == err) return(DBH_ERR) ;
		if(err < 0) {
			s_sth.s_items[item_no].s_97accno[0] = LV_CHAR ;
			return(err) ;
		}
		fomer(gl_rec.desc) ;
		break ;

	case GST_FLD :

		if ((s_sth.s_items[item_no].s_gst[0] != TAXABLE) &&
		    (s_sth.s_items[item_no].s_gst[0] != EXEMPT )){
#ifdef ENGLISH
			fomer("Valid Tax codes are T(axable), E(xempt)");
#else
			fomer("Codes valides de taxe sont T(axable), E(xempte)");
#endif
			s_sth.s_items[item_no].s_gst[0] = LV_CHAR;
			return(ERROR);
		}
		break;

	case PST_FLD :
		if ((s_sth.s_items[item_no].s_pst[0] != TAXABLE) &&
		    (s_sth.s_items[item_no].s_pst[0] != EXEMPT )){
#ifdef ENGLISH
			fomer("Valid Tax codes are T(axable), E(xempt)");
#else
			fomer("Codes valides de taxe sont T(axable), E(xempte)");
#endif
			s_sth.s_items[item_no].s_pst[0] = LV_CHAR;
			return(ERROR);
		}
		break;

	case QTY_FLD :		/* QTY Fields */
		/* If the invoice is for a specific PO, then quantity shouldn't
		   be more than PO quantities */

/*****
  Old condition replaced by below condition for adding items to PO referenced
  invoice. (CL 1991/01/03)	
		if((s_sth.s_po_no == 0)&&(s_sth.s_items[item_no].s_qty == 0)
							&& (mode != BALANCE)){
******/
		if((s_sth.s_items[item_no].s_qty == 0) && (mode != BALANCE) &&
			(mode != UPDATE)){
			fomca1(QTY_FLD,19,2) ;    /* Turn ON Dup Control */
			s_sth.s_items[item_no].s_qty = 1; 
		}

/* When adding or editing additional items added to a invoice referenced 
   by PO, don't validate the quantity according to the PO quantity. 
						  	CL 1991/01/03 	*/

		if((TransType==INVOICE)&&(s_sth.s_po_no)&&(!bal_items_added)) {

		    if(s_sth.s_items[item_no].s_qty > CurPage->PoQty[item_no]) {
#ifdef ENGLISH
			fomer("Can't be more than PO Quantity");
#else
			fomer("Ne peut pas etre plus que la quantite du BC");
#endif
			s_sth.s_items[item_no].s_qty = LV_DOUBLE ;
			return(ERROR) ;
		    }
 
		    if((s_sth.s_items[item_no].s_qty < DELTA_QTY) &&
						(mode != BALANCE)) 
			s_sth.s_items[item_no].s_amount = 0.0 ;
		    else {
			/* Show the PO Value as a defualt amount */
			cur_fld = ITEM_ST_FLD + (STEP * item_no) ;
			cur_fld += AMNT_FLD ;
			fomca1(cur_fld,19,2) ;
			temp = CurPage->PoValue[item_no]
					 /CurPage->PoQty[item_no];
			temp = D_Roundoff(temp);
			/* Separate to 2 lines because prob. NFLD */
			junk = s_sth.s_items[item_no].s_qty * temp;
			s_sth.s_items[item_no].s_amount = junk;

			sr.nextfld = cur_fld;
			cur_fld = sr.curfld ;
			end_fld = sr.endfld ;
			sr.endfld  = sr.nextfld;
			fomud((char*)&s_sth) ;
			ret(err_chk(&sr)) ;
			s_sth.s_items[item_no].s_amount = LV_DOUBLE ;
			sr.curfld  = cur_fld ;
			sr.endfld  = end_fld ;
		    }
		}
		else {
		    if( (TransType == INVOICE || TransType == RETURN) &&
			(s_sth.s_items[item_no].s_qty < DELTA_QTY)    &&
			(mode != BALANCE)){
#ifdef ENGLISH
			fomer("Can't be Zero");
#else
			fomer("Ne peut pas etre zero");
#endif
			s_sth.s_items[item_no].s_qty = LV_DOUBLE ;
			return(ERROR) ;
		    }
		    /* For BULK & NON-BULK type RETURNs & DBMEMOs Stock-paid-for
		       in Stock Master will be reduced by QTY. Check whether
		       this makes stock-paid-for -ve */
		    if(s_sth.s_type[0] != DIRECT &&
			(TransType == RETURN || TransType == DBMEMO) &&
			(stck_rec.st_paidfor - s_sth.s_items[item_no].s_qty) <
							-(DELTA_QTY) ) {
#ifdef ENGLISH
			fomer("Stock Master Paid for QTY is becoming negative");
#else
			fomer("Quantite payee du stock maitre devient negative");
#endif
			s_sth.s_items[item_no].s_qty = LV_DOUBLE ;
			return(ERROR) ;
		    }
		}
		break ;
	case AMNT_FLD :			/* Amount Fields */
		if((s_sth.s_items[item_no].s_amount < DELTA_AMT) &&
							(mode != UPDATE)) {
#ifdef ENGLISH
			fomer("Can't be Zero");
#else
			fomer("Ne peut pas etre zero");
#endif
			s_sth.s_items[item_no].s_amount = LV_DOUBLE ;
			return(ERROR) ;
		}
		if(TransType == INVOICE) {
		    err = CheckGlAcnt(s_sth.s_fund,
				s_sth.s_items[item_no].s_99accno,99,UPDATE) ;
		    if(DBH_ERR == err) return(err) ;
		    if(err < 0) {
			s_sth.s_items[item_no].s_amount = LV_DOUBLE ;
			return(err) ;
		    }
	
		    /* See Total for current account is exceeding the Allocated
		       budget. If So give warning */
		    /* Cumulate the uncommited value of INVC for curr. acct */
		    diff=
			CumulativeUncommited(s_sth.s_items[item_no].s_99accno);
	
		    /* Add the current Item value */
		    if(mode == ADD)
			diff += s_sth.s_items[item_no].s_amount ;
		    else
			diff += (s_sth.s_items[item_no].s_amount -
					CurPage->Items[item_no].s_amount) ;
	
		    /* Find out remaining budget */
		    temp = gl_rec.budcur - gl_rec.comdat - gl_rec.ytd ;
		    if(s_sth.s_items[item_no].s_amount > DELTA_AMT &&
								temp < diff)
#ifdef ENGLISH
			DispError("Not Enough Budget");
#else
			DispError("Pas assez de budget");
#endif
		}
		else if(TransType == RETURN || TransType == DBMEMO) {
		    /* For BULK and NON-BULK type transactions Stock value will
		       be reduced by amount. Check whether this makes stock
		       value -ve */
		    if(s_sth.s_type[0] != DIRECT &&
			(stck_rec.st_value - s_sth.s_items[item_no].s_amount) <
							-(DELTA_AMT) ) {
#ifdef ENGLISH
			fomer("Stock Master Value is becoming negative");
#else
			fomer("Valeur du stock maitre devient negative");
#endif
			s_sth.s_items[item_no].s_amount = LV_DOUBLE ;
			return(ERROR) ;
		    }
		}
		break ;
	default :
#ifdef ENGLISH
		sprintf(e_mesg,"No Validity Check For Field#  %d",sr.curfld);
#else
		sprintf(e_mesg,"Pas de controle de validite pour le Champ#  %d",sr.curfld);
#endif
		fomen(e_mesg);
		get();
		return(ERROR) ;
	}	/* Switch sr.curfld */

	return(NOERROR) ;
}	/* ItemsValidation() */
/*----------------------------------------------------------------*/
/* Check the Validity of Given PO#. It should exits, belongs to the
   same supplier and status of it should not be COMPLETE.
   If the PO is valid, then copy the PO Fund# and type as INVOICE fund & type */
static	int
CheckPOValidity()
{
	int	err ;

	po_hdr.ph_code = s_sth.s_po_no ;

	err = get_pohdr(&po_hdr, UPDATE, 0, e_mesg) ;
	if(ERROR == err) return(DBH_ERR);
	if(err < 0) {
		fomer(e_mesg) ;
		return(ERROR) ;
	}
	if(strcmp(s_sth.s_supp_cd, po_hdr.ph_supp_cd)) {
#ifdef ENGLISH
		fomer("Invoice Supplier and PO Supplier are not the same");
#else
		fomer("Fournisseur de la facture et fournisseur du BC ne sont pas les memes");
#endif
		return(ERROR) ;
	}
	if(po_hdr.ph_status[0] == COMPLETE) {
#ifdef ENGLISH
		fomer("PO is Complete");
#else
		fomer("BC est complet");
#endif
		return(ERROR) ;
	}
	/* Move PO fund# as a invoice Fund# */
	if((err = CheckFund(po_hdr.ph_funds)) < 0) return(err) ;

	s_sth.s_fund = po_hdr.ph_funds ;
	s_sth.s_type[0] = po_hdr.ph_type[0] ;
	s_sth.s_type[1] = '\0' ;

	return(NOERROR) ;
}	/* CheckPOValidity() */
/*----------------------------------------------------------------*/
/* check to see if payee in supplier file			  */
static  int
GetPayee()
{

	if(po_hdr.ph_payee[0] != '\0') {
		STRCPY(s_sth.s_payee,po_hdr.ph_payee);	
		WriteFields(PAYEE_FLD,PAYEE_FLD);
		s_sth.s_payee[0] = LV_CHAR;
	}
	return(NOERROR);
}
/*----------------------------------------------------------------*/
/* Read the given stock code record in UPDATE mode */
static	int
CheckStock(stcode)
char	*stcode ;
{
	int	err ;

	stck_rec.st_fund = s_sth.s_fund ;
	STRCPY(stck_rec.st_code, stcode) ;

	err = get_stmast(&stck_rec, UPDATE, 0, e_mesg) ;
	if(ERROR == err) return(DBH_ERR);
	if(err < 0) {
		fomer(e_mesg) ;
		return(ERROR) ;
	}

	return(NOERROR) ;
}	/* CheckStock() */
/*----------------------------------------------------------------*/
/* Cumulate the umcommitted invoice value of the the given account in items */

double
CumulativeUncommited(accno)
char	*accno ;
{
	double	diff ;
	Page	*temppage ;
	int	i ;

	diff = 0.0 ;
	if(InvcLast != NULL)
	    for(temppage = FirstPage ; temppage != NULL ;
						temppage = temppage->NextPage) {
		for(i = 0 ; i < temppage->NoItems ; i++) {
		    if(temppage->Items[i].s_amount < DELTA_AMT) continue ;
		    if(strcmp(temppage->Items[i].s_status, NOTACTIVE) != 0)
				continue ;
		    /* If not Same account, skip the item */
		    if(strcmp(accno, temppage->Items[i].s_99accno)) continue ;
		    if(TransType == INVOICE && s_sth.s_po_no) {
		        if(temppage->PoQty[i] != 0.0)
			    diff += temppage->Items[i].s_amount -
				(temppage->Items[i].s_qty *
				(temppage->PoValue[i]/temppage->PoQty[i])) ;
		    }
		    else
			diff += temppage->Items[i].s_amount ;
		}

		/* IF the Current Page is last page in Invoice, then break */
		if(temppage == InvcLast) break ;
	    }
	return(diff) ;
}	/* CumulativeUncommited() */
/*----------------------------------------------------------------*/
/* Show Help Windows, if applicable, when user gives ESC-H in key
   and header fields */

HdrAndKeyWindowHelp()
{
	int	err ;

	switch(sr.curfld){
	case	SUPPCD_FLD	:	/* Supplier Cd: */
		err = supp_hlp(s_sth.s_supp_cd, 7, 13 );
		if(err == DBH_ERR) return(err) ;
		if(err >=0 ) redraw();
		if(err < 1) return(ERROR) ;

		/* Lock the Supplier */
		err = CheckSupp(s_sth.s_supp_cd,UPDATE) ;
		if(DBH_ERR == err) return(err) ;
		if(err<0) {
			s_sth.s_supp_cd[0] = LV_CHAR ;
			return(ERROR) ;
		}
		break ;

	case	PAYEE_FLD:
		err = supp_hlp(s_sth.s_payee, 7, 13 );
		if(err == DBH_ERR) return(err) ;
		if(err >=0 ) redraw();
		if(err < 1) return(ERROR) ;

		/* check whether given key is already existing for payee */
		err = GetInvoice(s_sth.s_payee, s_sth.s_invc_no,
					s_sth.s_tr_type, BROWSE);
		if(ERROR == err) return(DBH_ERR) ;
		if(NOERROR == err){
#ifdef ENGLISH
			fomer("Given Key already in File - Please enter again");
#else
			fomer("Cle donnee deja dans le dossier - Reessayer");
#endif
			get();
			return(DUPEREF);
		}
		/* Lock the Supplier */
		err = CheckPayee(s_sth.s_payee,UPDATE) ;
		if(DBH_ERR == err) return(err) ;
		if(err<0) {
			s_sth.s_payee[0] = LV_CHAR ;
			return(ERROR) ;
		}
		break ;
	default :
#ifdef ENGLISH
		fomer("No Help Window For This Field");
#else
		fomer("Pas de fenetre d'assistance pour ce champ");
#endif
		return(ERROR) ;
	}	/* Switch sr.curfld */

	return(NOERROR) ;
}	/* HdrAndKeyWindowHelp() */
/*----------------------------------------------------------------*/
/* Show Help Windows, if applicable, when user gives ESC-H in Item fields */

ItemsWindowHelp()
{
	int	err ;
	int	fld_no, item_no ;
	short	reccod ;

	/* Caluculate item# ans Fld# within item */
	item_no = (sr.curfld - ITEM_ST_FLD) / STEP ;
	fld_no  = (sr.curfld - ITEM_ST_FLD) % STEP ;

	switch(fld_no){
	case STCKCD_FLD :		/* Stock Code Fields */
		/* If Direct Charge, No Window */

		err = stock_hlp(s_sth.s_fund, s_sth.s_items[item_no].s_stck_cd,
								7, 13 );
		if(err == DBH_ERR) return(err) ;
		if(err >=0 ) redraw();
		if(err < 1) return(ERROR) ;	/* Not Selected */
		err = CheckStock(s_sth.s_items[item_no].s_stck_cd) ;
		if(DBH_ERR == err) return(err) ;
		if(err < 0) {
			s_sth.s_items[item_no].s_stck_cd[0] = LV_CHAR ;
			return(ERROR) ;
		}
		break ;
	case SCHOOL_FLD :		/* School Fields */
		err = sch_hlp(&s_sth.s_items[item_no].s_school, 7, 13 );
		if(err == DBH_ERR) return(err) ;
		if(err >=0 ) redraw();
		if(err < 1) return(ERROR) ;	/* Not Selected */
		break ;
	case ACNO99_FLD :		/* GL Account#(99) Fields */
		err = gl_hlp(s_sth.s_fund, s_sth.s_items[item_no].s_99accno,
			&reccod, 7, 13 );
		if(err == DBH_ERR) return(err) ;
		if(err >=0 ) redraw();
		if(err < 1) return(ERROR) ;	/* Not Selected */
		if(reccod != 99) {
#ifdef ENGLISH
			fomer("Select records with 99 only as Record Code");
#else
			fomer("Choisir les fiches avec 99 seulement comme code de fiche");
#endif
			s_sth.s_items[item_no].s_99accno[0] = LV_CHAR ;
			return(ERROR) ;
		}
		err = CheckGlAcnt(s_sth.s_fund,s_sth.s_items[item_no].s_99accno,
				99,UPDATE) ;
		if(DBH_ERR == err) return(err) ;
		if(err < 0) {
			s_sth.s_items[item_no].s_99accno[0] = LV_CHAR ;
			return(err) ;
		}
		if((pa_rec.pa_stores[0] == YES) && 
		   (s_sth.s_type[0] == DIRECT || s_sth.s_type[0] == NON_BULK)) {
			if(strcmp(s_sth.s_items[item_no].s_99accno,
						ctl_rec.inv_acnt)==0){
#ifdef ENGLISH 
				fomer("Cannot use Inventory General account for Direct or Non-Bulk PO's");
#else
				fomer("Compte Inventaire Gen. ne peut etre utilise pour BC dir. ou de stock reserve");
#endif
				s_sth.s_items[item_no].s_99accno[0] = LV_CHAR ;
				return(ERROR) ;
			}
		}
		break ;
	case ACNO97_FLD :		/* GL Account#(97) Fields */
		err = gl_hlp(s_sth.s_fund, s_sth.s_items[item_no].s_97accno,
			&reccod, 7, 13 );
		if(err == DBH_ERR) return(err) ;
		if(err >=0 ) redraw();
		if(err < 1) return(ERROR) ;	/* Not Selected */
		if(reccod != 97) {
#ifdef ENGLISH
			fomer("Select records with 97 only as Record Code");
#else
			fomer("Choisir les fiches avec 97 seulement comme code de fiche");
#endif
			s_sth.s_items[item_no].s_97accno[0] = LV_CHAR ;
			return(ERROR) ;
		}
		err = CheckGlAcnt(s_sth.s_fund,s_sth.s_items[item_no].s_97accno,
				97,UPDATE) ;
		if(DBH_ERR == err) return(err) ;
		if(err < 0) {
			s_sth.s_items[item_no].s_97accno[0] = LV_CHAR ;
			return(err) ;
		}
		break ;
	default :
#ifdef ENGLISH
		fomer("No Help Window For This Field");
#else
		fomer("Pas de fenetre d'assistance pour ce champ");
#endif
		return(ERROR) ;
	}	/* Switch sr.curfld */

	return(NOERROR) ;
}	/* ItemsWindowHelp() */
/*-----------------------------------------------------------*/
/* Read PO Items and make Linked list */

MakePOItemsList()	/* Make the linked list from PO Items */
{
	int	err, i ;

	po_item.pi_code = po_hdr.ph_code ;
	po_item.pi_item_no = 0;
	flg_reset(POITEM);	/* Initialize to get first rec under givenkey */

	InvcLast = CurPage = NULL ;
	i = 0 ;

	for( ; ; ){
#ifndef ORACLE
		err = get_n_poitem( &po_item, BROWSE, 0, FORWARD, e_mesg);
#else
		err = get_n_poitem( &po_item, BROWSE, 0, EQUAL, e_mesg);
#endif
		if(ERROR == err) return(DBH_ERR) ;
		if(EFL == err) break ;

#ifndef ORACLE
		/* If PO changes break */
		if( po_item.pi_code != po_hdr.ph_code) break ;
#endif

		/* Skip Fully received Items */
/******
		if( (po_item.pi_orig_qty - po_item.pi_pd_qty) < DELTA_QTY)
			continue ;
******/
		if( (po_item.pi_orig_qty - po_item.pi_pd_qty) < DELTA_QTY &&
			po_item.pi_orig_qty != 0)
			continue ;

		if(PAGESIZE == i) i = 0 ;
		if(0 == i)		/* 1st Item in the Page */
			if((err = MakeFreshPage()) < 0) return(err) ;

		CurPage->Items[i].s_sno = i + 1 ;
		STRCPY(CurPage->Items[i].s_stck_cd, po_item.pi_st_code) ;

			/* CL 1991/01/03 */ 	
		if (po_item.pi_acct[0] != LV_CHAR) 
			STRCPY(CurPage->Items[i].s_99accno, po_item.pi_acct) ;
		else
			CurPage->Items[i].s_99accno[0] = LV_CHAR ;			
		CurPage->Items[i].s_97accno[0] = '\0' ;
		if(po_item.pi_school == 0)    /* Not an (INVOICE & NON-BULK) */
			CurPage->Items[i].s_school = HV_SHORT ;
		else
			CurPage->Items[i].s_school = po_item.pi_school ;
		STRCPY(CurPage->Items[i].s_desc,po_item.pi_desc);

		if(po_item.pi_tax1[0] != TAXABLE)
			CurPage->Items[i].s_gst[0] = EXEMPT;
		else
			CurPage->Items[i].s_gst[0] = po_item.pi_tax1[0];

		CurPage->Items[i].s_rebate = ctl_rec.rebate;

		if(po_item.pi_tax2[0] != TAXABLE)
			CurPage->Items[i].s_pst[0] = EXEMPT;
		else
			CurPage->Items[i].s_pst[0] = po_item.pi_tax2[0];
		CurPage->Items[i].s_qty = po_item.pi_orig_qty -
							po_item.pi_pd_qty ;
		CurPage->Items[i].s_amount = po_item.pi_value -
							po_item.pi_paid ;
		STRCPY(CurPage->Items[i].s_status, ACTIVE) ;
		
		CurPage->PoQty[i]    = CurPage->Items[i].s_qty ;
		CurPage->PoValue[i]  = CurPage->Items[i].s_amount ;
		CurPage->PoItemNo[i] = po_item.pi_item_no ;

		CurPage->NoItems++ ;
		i++ ;
	} 
	seq_over(POITEM) ;

	if(InvcLast != NULL) CurPage = FirstPage ;

	return(NOERROR) ;
}	/* MakePOItemsList() */
/*------------------------------------------------------------*/
/*
*	Get the next node in linked list to add invoice items. If the
*	(Cur. Invoice last page) = (Last Page in linked List) or no
*	nodes in list, allocate node and add to linked list
*
*	MakeFreshPage allocates memory when required. The memory is
*	nevered freed until the TransEntry() procedure is about to exit.
*	Memory is freed when CloseProcess is called.
*
*	The memory allocated is re-used for each invoice enterd.
*
*	'LastPage' points to the last item Page allocated for the program.
*
*	'InvcLast' points to the item Page used for the current invoice.
*/

MakeFreshPage()
{
	Page	*tempptr ;

	/* If, no node is allocted yet or Current invoice used all the nodes,
	   then allocate new node */

	if( LastPage == NULL || InvcLast == LastPage ){
		tempptr= (Page *)malloc((unsigned)sizeof(Page)) ;

		if( tempptr == NULL ){
#ifdef ENGLISH
			DispError("MEMORY ALLOCATION ERROR...");
#else
			DispError("ERREUR D'ALLOCATION A LA MEMOIRE...");
#endif
			return(ERROR);
		}
		tempptr->NextPage = NULL ;

		if( LastPage == NULL ){	/* No node is allocated Yet */
			tempptr->PrevPage = NULL ;
			tempptr->Pageno = 1 ;
			FirstPage = tempptr ;
		}
		else {				/* Not a first node in list */
			tempptr->Pageno = LastPage->Pageno + 1 ;
			LastPage->NextPage = tempptr ;
			tempptr->PrevPage = LastPage ;
		}
		LastPage = tempptr ;
	}

	if(InvcLast == NULL)
		InvcLast = FirstPage ;
	else
		InvcLast = InvcLast->NextPage ;

	InvcLast->NoItems = 0 ;
	CurPage = InvcLast ;

	return(NOERROR);
}	/* MakeFreshPage() */
/*-----------------------------------------------------------------------*/ 
/* Update the records and write to files */
static	int
WriteRecords()
{
	int	err;
	int	i,method;
	double  amount;
	Page	*temppage ;

	/* J. Prescott Sept. 29/92 Try to Lock GL Trans file until successful */
	for( ; ; ) {
		if((err = lock_file(GLTRHDR)) < 0) {
			if(err == LOCKED) { 
				continue;
			}
			DispError(e_mesg);
			roll_back(e_mesg);	/* Unlock the locked Records */
			return(err);
		}
		else break;
	}
	/***********************************************/
	if(TransType == INVOICE || TransType == CRMEMO )
		Invoice_Crmemo_flag = TRUE;
	else
		Invoice_Crmemo_flag = FALSE;

	Gl97Exists = 0 ;	/* Will be set to YES if 97 Acnts are given */

	err = LockGeneralAcnts() ;	/* Lock General GL Accounts */
	if(err < 0) return(err) ;

	err = InitGlTrans() ;	/* Initialize GL Trans. Header & Item */
	if(err < 0) return(err) ;

	err = CheckInvoiceValidity() ;
	if(err < 0) return(err) ;

	InitInvoice();

	err = InitTrans() ;		/* Initialize Invoice Header & Item */
	if(err < 0) return(err) ;

	/* Global Variables used in ProcGlobals; Init to Zero */
	pst_payable = ap_general_acnt = 0.0;
	/* If no items return; Should not happen */
	if(InvcLast == NULL) return(NOERROR);
	
	method = Get_Posting_Method();
	
	/* Count Total Items for invoice */
	remain_items=0;
	for(temppage=FirstPage; temppage != NULL; temppage=temppage->NextPage){
		for(i=0;i<temppage->NoItems;i++) {
			if(strcmp(temppage->Items[i].s_status,NOTACTIVE)==0){
				continue;
			}
			if(temppage->Items[i].s_amount > DELTA_AMT)
				remain_items+=temppage->Items[i].s_qty;
		}

		/* if current page is last page in invoice, the */
		if(temppage == InvcLast) break;
	}
	/* for freight distribution j.Prescott 04/01/93 */
	remain_freight = s_sth.s_misc_amt;
	if(remain_freight != 0.00){
		Get_Taxes(s_sth.s_misc_gst,s_sth.s_misc_pst,remain_freight,
			&tax_cal);
		remain_freight = D_Roundoff(tax_cal.gros_amt);
		remain_f_gst = D_Roundoff(tax_cal.gst_amt);
	}
	/* Process each item and update files */
	for(temppage=FirstPage; temppage != NULL; temppage=temppage->NextPage){
		/* Take Each Invoice Item and Update the files */
		for(i = 0 ; i < temppage->NoItems ; i++) {
		    /* Is Item amount < 0 or Not Active */
		    if(temppage->Items[i].s_amount < DELTA_AMT ||
			    strcmp(temppage->Items[i].s_status,NOTACTIVE) == 0){
			/*  CL 1991/01/03  */
			/* Is this an Invoice with a Po */
		    	if(TransType == INVOICE && s_sth.s_po_no) {
				/* Did the Item exist in the Po before */
				err = GetPOItem(temppage->PoItemNo[i]);	
				if(err == NOERROR){
					 /* Is the Po to be Completed */
					 if((s_sth.s_complete == BOOL_YES) &&
					   (!bal_items_added)){
					 /*Update Commitments for deleted Item*/
			    			err=UNCommit(temppage,i);
			    			if(err != NOERROR) return(err);
					}
				}else if (err != UNDEF) return(err);
			}
			continue ;
		    } /* end Non Active Item */

		    /* Subtract discount for tax calculations and post amount */
		    /* less discount to accounts			      */
  		    temppage->Items[i].s_amount -= temppage->DiscAmt[i];

		    /* Init global A/P general accounts */
		    err = ProcItemUpdates(temppage,i,method);

		    /* Add discount back to amount for screen display in   */
	   	    /* case we have an error and return to screen           */ 
		    temppage->Items[i].s_amount += temppage->DiscAmt[i]; 

		    if(err != NOERROR){
			DispError(e_mesg);
			roll_back(e_mesg);	/* Unlock the locked Records */
			return(err);
		    }
		/* for freight distribution J.Prescott 04 */
		remain_f_gst -= D_Roundoff((remain_f_gst*
				temppage->Items[i].s_qty)/remain_items);
		remain_freight -= D_Roundoff((remain_freight * 
				temppage->Items[i].s_qty)/remain_items);
		remain_items -= D_Roundoff(temppage->Items[i].s_qty);
			
		}	/* for( ; i < NoItems ; ) */

		/* If Current Page is last page in Invoice, then break */
		if(temppage == InvcLast) break;
	}	/* for( ; temppage != NULL ; ) */

	err = ProcGlobalUpdates();
	if(err == NOERROR) err = commit(e_mesg);
	if(err < 0) {
		DispError(e_mesg);
		roll_back(e_mesg) ;	/* Unlock the locked Records */
	}
	return(err);
}	/* WriteRecords() */
/*-----------------------------------------------------------------------*/ 
/* Check whether total of items amount is equal to Invoice total or not. And
   check whether this invoice has any items */
static	int
CheckInvoiceValidity()
{
	double	total1,total2,diff;
	Page	*temppage ;
	int	err ;

	/* Total up all items amount */
	tax_clearance_acnt = total1 = total2 = 0.0 ;

	if(InvcLast != NULL)
		if(FirstPage->NoItems <= 0){
#ifdef ENGLISH
			DispError("No Items in Invoice... Cancel to Quit..");
#else
			DispError("Pas d'articles dans la facture... Annuler pour retourner..");
#endif
			return(TAXPROBLEM) ;
		 }else
			if(Check_Pst_Gst()!= NOERROR) return(TAXPROBLEM);

	/* Difference */
	total1 = s_sth.s_pst_amt + s_sth.s_gst_amt + s_sth.s_item_cost
						 	 - s_sth.s_disc_amt;  
	total2 = s_sth.s_net_amt - s_sth.s_disc_amt;
	diff   = total2-total1;
			
	if(s_sth.s_gst_tax > s_sth.s_gst_amt){

#ifdef ENGLISH
		sprintf(e_mesg, "GST Adjustment: %.2f > Total Item GST %.2f " ,s_sth.s_gst_tax,s_sth.s_gst_amt);
#else
		sprintf(e_mesg, "GST Adjustment: %.2f > Total Item GST %.2f " ,s_sth.s_gst_tax,s_sth.s_gst_amt);
#endif
		DispError(e_mesg);
		return(TAXPROBLEM);
	}

	if(diff < -(DELTA_AMT) || diff > DELTA_AMT) {
#ifdef ENGLISH
		sprintf(e_mesg,
			"Invoice Total: %.2lf  Items Total: %.2lf  Diff: %.2lf",
			total2,total1,diff);
#else
		sprintf(e_mesg,
			"Total facture: %.2lf  Total articles: %.2lf  Diff: %.2lf",
			total2,total1,diff);
#endif
		DispError(e_mesg);

#ifdef ENGLISH
    err = GetOption("Post Difference to Tax Clearing Account (Y/N)?", "YN") ;
#else
    err = GetOption("Reporter la difference au compte provisoire de taxe (O/N)?", "ON") ;
#endif
		if(err == YES){
			err=Update_Tax_Clearance(diff);
			if(err < 0) return(ERROR);
		}else
			return(TAXPROBLEM);
	}
	return(NOERROR) ;
}	/* CheckInvoiceValidity() */
/*-----------------------------------------------------------------------*/ 
/* Lock Necessary General GL Accounts before file updations */

LockGeneralAcnts()
{
	int	err ;

	/* Lock AP General or Contract GL account before updation */
	if(supp_rec.s_type[0] == ORDINARY)	/* Ordinary Supplier */
		err = CheckGlAcnt(ctl_rec.fund,ctl_rec.ap_gen_acnt,99,UPDATE) ;
	else				/* Contract Supplier */
		err = CheckGlAcnt(ctl_rec.fund,ctl_rec.ap_cnt_acnt,99,UPDATE) ;
	if(err < 0) return(err) ;
	
	/* Lock Tax General Account, if the Tax amount > 0.0 */
	if(s_sth.s_pst_tax > DELTA_AMT) {
		err = CheckGlAcnt(ctl_rec.fund,ctl_rec.pst_tax_acnt,99,UPDATE) ;
		if(err < 0) return(err) ;
	}
	if(s_sth.s_gst_tax > DELTA_AMT) {
		err = CheckGlAcnt(ctl_rec.fund,ctl_rec.gst_tax_acnt,99,UPDATE) ;
		if(err < 0) return(err) ;
	}

	if(s_sth.s_type[0] != DIRECT) {	/* Not Direct Charge */
		/* Lock Inventory General GL account before updation */
		err = CheckGlAcnt(ctl_rec.fund,ctl_rec.inv_acnt,99,UPDATE) ;
		if(err < 0) return(err) ;
	}

	return(NOERROR) ;
}	/* LockGeneralAcnts() */
/*-----------------------------------------------------------------------*/ 
/* Process Invoice(Global) Level Updates */

ProcGlobalUpdates()
{
	int	err ;
	double	amount,rebate;
	Invoice	pre_rec ;
	double	total;

	/* When cheque# is given, that is cheque for this transaction is issued
	   manually. Actually this is suppose to be done thru other function,
	   but it is being accepeted here for the user's convenience. When the
	   manual is cheque is entered, create audit as if it is entered thru
	   regular function */
	if(s_sth.s_chq_no != 0)
		scpy((char*)&pre_rec, (char*)&in_rec, sizeof(Invoice)) ;

	/* Credit/Debit the Invoice amount - discount amt to Accounts Payable */
	if(supp_rec.s_type[0] == ORDINARY)	
		err = CheckGlAcnt(ctl_rec.fund,ctl_rec.ap_gen_acnt,99,UPDATE) ;
	else				
		err = CheckGlAcnt(ctl_rec.fund,ctl_rec.ap_cnt_acnt,99,UPDATE) ;
	if(err < 0) return(err) ;


/**
	total = s_sth.s_inv_total + s_sth.s_disc_amt - disc_pst - disc_gst;
**/
	total = s_sth.s_inv_total + s_sth.s_disc_amt;
	if(Invoice_Crmemo_flag)
		err = CrtGlTrans(&gl99_hdr,&gl99_itm,total,CREDIT);
	else
		err = CrtGlTrans(&gl99_hdr, &gl99_itm,total,DEBIT);
	if(err < 0) return(err) ;

	if(s_sth.s_disc_amt > DELTA_AMT) {	
		/* Credit/Debit the Discount Amount to Discount Account */
		err = CheckGlAcnt(ctl_rec.fund,ctl_rec.dis_acnt,99,UPDATE) ;
		if(err < 0) return(err) ;

		if(Invoice_Crmemo_flag)
			err = CrtGlTrans(&gl99_hdr,&gl99_itm,s_sth.s_disc_amt,DEBIT);
		else
			err = CrtGlTrans(&gl99_hdr, &gl99_itm,s_sth.s_disc_amt,CREDIT);
		if(err < 0) return(err) ;
	}

	if((s_sth.s_disc_pf[0] == AMOUNT) && (s_sth.s_disc_amt > DELTA_AMT)){
		/* Credit discount account */
		amount = s_sth.s_disc_amt;
		if((err=CheckGlAcnt(s_sth.s_fund,ctl_rec.expdis_acnt,
								99,UPDATE))<0)
			return(err);
		if(Invoice_Crmemo_flag)
			err=CrtGlTrans(&gl99_hdr,&gl99_itm,amount, CREDIT);
		else
			err=CrtGlTrans(&gl99_hdr,&gl99_itm,amount, DEBIT);
		if (err < 0) return(err) ;
	}

	if(s_sth.s_pst_pf[0] == FULL_AMT)
		pst_payable = D_Roundoff((s_sth.s_pst_tax - disc_pst));
	else if(s_sth.s_pst_pf[0] == PARTIAL)
		pst_payable = s_sth.s_pst_tax;
	else
		pst_payable = 0.0;

	if(pst_payable > DELTA_AMT ){
		/* Credit Pst Tax account */
		if((err=CheckGlAcnt(s_sth.s_fund,ctl_rec.pst_tax_acnt,
								99,UPDATE))<0)
			return(err);

		if(Invoice_Crmemo_flag)
			err=CrtGlTrans(&gl99_hdr,&gl99_itm,pst_payable,CREDIT);

		else
			err=CrtGlTrans(&gl99_hdr,&gl99_itm,pst_payable,DEBIT);
		if (err < 0) return(err) ;
	}

	if(s_sth.s_gst_pf[0] == PARTIAL){
		/* Calculate Gst rebate */
		rebate = Cal_Rebate(ctl_rec.rebate,s_sth.s_gst_tax);
		amount = D_Roundoff(s_sth.s_gst_tax - rebate);
		if((err=CheckGlAcnt(s_sth.s_fund,ctl_rec.gst_tax_acnt,
								99,UPDATE))<0)
			return(err);
		if(TransType == INVOICE || TransType == CRMEMO )
			err=CrtGlTrans(&gl99_hdr,&gl99_itm, amount, CREDIT);
		else
			err=CrtGlTrans(&gl99_hdr,&gl99_itm, amount, DEBIT);
		if (err < 0) return(err) ;

		if((err=CheckGlAcnt(s_sth.s_fund,ctl_rec.ar_gst_acnt,
								99,UPDATE))<0)
			return(err);

		if(Invoice_Crmemo_flag)
			err=CrtGlTrans(&gl99_hdr,&gl99_itm,rebate,CREDIT);
		else
			err=CrtGlTrans(&gl99_hdr,&gl99_itm,rebate,DEBIT);
		if (err < 0) return(err) ;
	}

	amount = gl99_hdr.th_debits - gl99_hdr.th_credits ;
	if(amount < -(DELTA_AMT) || amount > DELTA_AMT) {
		sprintf(e_mesg,"debits: %lf  credits: %lf  diff: %lf",gl99_hdr.th_debits,gl99_hdr.th_credits,amount);
		fomer(e_mesg); get();

#ifdef ENGLISH
		DispError("GL Trans. Credits are != Debits. LOGIC ERROR.. Contact Software Support Group");
#else
		DispError("Cr trans G/L ne sont pas =Db. ERREUR logique.. Contacter le groupe de soutient.");
#endif
		return(ERROR) ;
	}


	err = put_trhdr(&gl99_hdr, UPDATE, e_mesg) ;
	if(err < 0) return(err) ;

	/* If any GL trans item is written for 97 Account, then write Hdr */
	if(Gl97Exists) {
		err = put_trhdr(&gl97_hdr, UPDATE, e_mesg) ;
		if(err < 0) return(err) ;
	}
	else {
		/* Remove header reserved from before */
		err = put_trhdr(&gl97_hdr, P_DEL, e_mesg) ;
		if(err < 0) return(err) ;
	}

	if( (TransType == INVOICE) && (s_sth.s_po_no) ){
		/* Update the PO Hdr */
		po_hdr.ph_lqdate = s_sth.s_rundate ;

/* if additional items were added to the invoice referenced by PO recalculate
   the po_hdr.lqamt with paid amounts from the poitem's paid amounts. 
							CL 1991/01/03 */


		if(bal_items_added){  
			po_hdr.ph_lqamt += po_hdr_liq_tot;	
			po_hdr_liq_tot = 0.0;	
		}
		else {
			/* Jan 11/92 CB- Subtracted disc off of lq amt */
			po_hdr.ph_lqamt += s_sth.s_gr_amt + s_sth.s_pst_tax
				 		+ s_sth.s_gst_tax  
                                                - s_sth.s_disc_amt;
		}

		po_hdr.ph_lqamt = D_Roundoff(po_hdr.ph_lqamt);

		/* If the PO is fully received then set it to COMPLETE
		   automatically */
		if(s_sth.s_complete == BOOL_YES) 
			po_hdr.ph_status[0] = COMPLETE ;
		else
			po_hdr.ph_print[0] = POPRNT;

		err = put_pohdr(&po_hdr, UPDATE, e_mesg) ;
		if(err < 0) return(err) ;
	}

	/* For contract supplier add HB amount to disc_amt. Later on
	   it will be deducted whenever HB is released */
	if(Invoice_Crmemo_flag){
		supp_rec.s_ytd_recpt += s_sth.s_gr_amt;
		if(supp_rec.s_type[0] == CONTRACT)
			supp_rec.s_ytd_disc += s_sth.s_disc_amt ;

		if(TransType == INVOICE) {
		    supp_rec.s_last_actv = s_sth.s_rundate ;
		    if(s_sth.s_po_no == 0)	/* Invoice Without PO */
			supp_rec.s_ytd_ord += s_sth.s_gr_amt - s_sth.s_pst_amt -
						s_sth.s_gst_amt ;
		}
	}
	else {
		if(TransType == RETURN){
			supp_rec.s_ytd_ret += (s_sth.s_gr_amt - s_sth.s_pst_amt
						 - s_sth.s_gst_amt) ;
			supp_rec.s_ytd_ret = D_Roundoff(supp_rec.s_ytd_ret);
		}
		else{
			supp_rec.s_ytd_recpt -=(s_sth.s_gr_amt - s_sth.s_pst_amt
						- s_sth.s_gst_amt) ;
			supp_rec.s_ytd_recpt = D_Roundoff(supp_rec.s_ytd_recpt);
		}
		if(supp_rec.s_type[0] == CONTRACT){
			supp_rec.s_ytd_disc -= s_sth.s_disc_amt ;
			supp_rec.s_ytd_disc = D_Roundoff(supp_rec.s_ytd_disc);
		}
	}

	if(s_sth.s_payee[0] == '\0') {
		scpy((char *)&payee_rec,(char *)&supp_rec,sizeof(payee_rec));
/**
		memcpy(&payee_rec,&supp_rec,sizeof(Supplier));
**/
		if(Invoice_Crmemo_flag)
			payee_rec.s_balance   -= s_sth.s_gr_amt ;
		else
			payee_rec.s_balance  += s_sth.s_gr_amt ;
		payee_rec.s_balance  = D_Roundoff(payee_rec.s_balance);

		err = put_supplier(&payee_rec, UPDATE, e_mesg) ;
		if(err < 0) return(err) ;
	}
	else{
		err = put_supplier(&supp_rec, UPDATE, e_mesg) ;
		if(err < 0) return(err) ;
	}

			
		
	/* If the cheque is issued manually, set the transaction status to
	   manual payment, and write audit records for that */
	if(s_sth.s_chq_no != 0) {
		in_rec.in_pmtcode[0] = PARTIAL ;
		in_rec.in_chq_no   = s_sth.s_chq_no ;
		STRCPY(in_rec.in_accno, s_sth.s_bank_acct) ;
		in_rec.in_part_amt = in_rec.in_amount ;
		in_rec.in_part_amt = D_Roundoff(in_rec.in_part_amt);

		in_hdr.h_pmtcode[0] = PARTIAL ;
		in_hdr.h_chq_no   = s_sth.s_chq_no ;
		STRCPY(in_hdr.h_accno, s_sth.s_bank_acct) ;
		in_hdr.h_part_amt = in_rec.in_amount ;
		in_hdr.h_part_amt = D_Roundoff(in_hdr.h_part_amt);

		err = rite_audit((char*)&s_sth,APINVOICE,UPDATE,(char*)&in_rec,
			(char*)&pre_rec, e_mesg);

		if(err != NOERROR){
			roll_back(e_mesg);
			return(err) ;
		}
	}

	if(TransType == INVOICE && s_sth.s_po_no)
		if(s_sth.s_complete == BOOL_YES)
			/* pass complete flag to Ap details report */
			in_hdr.h_po_cmp[0] = 'Y';

	err = put_inhdr(&in_hdr, ADD, e_mesg) ;
	if(err < 0) return(err) ;

	strcpy(in_rec.in_orgsupp_cd, s_sth.s_supp_cd);   /* louis */

	err = put_invc(&in_rec, ADD, e_mesg) ;
	if(err < 0) return(err) ;
	return(NOERROR) ;
}	/* ProcGlobalUpdates() */
/*-----------------------------------------------------------------------*/ 
/* Process given item and Update the files */
ProcItemUpdates(page,item_indx,method)
Page *page;
int item_indx,method;
{
int	err, mode ;
double	diff,temp;
S_item  *s_item;
double	tmp_disc_pst, tmp_disc_gst ;

	s_item = &page->Items[item_indx];
	alloc_amt = 0.00;
	tmp_disc_pst = tmp_disc_gst = 0.00;

	/*  C Burns March4/1990 - Discount on tax not taken of expense acnt 
	if(s_sth.s_disc_pf[0] == PERCENT) {
		temp = s_item->s_amount + page->DiscAmt[item_indx] ;
        	Get_Applied_Taxes(s_item->s_gst,s_item->s_pst,temp,&tax_cal); 
		tmp_disc_pst = tax_cal.pst_amt ;
		tmp_disc_gst = tax_cal.gst_amt ;
        	Get_Applied_Taxes(s_item->s_gst,s_item->s_pst,
						s_item->s_amount,&tax_cal);
		tmp_disc_pst = D_Roundoff(tmp_disc_pst - tax_cal.pst_amt) ;
		tmp_disc_gst = D_Roundoff(tmp_disc_gst - tax_cal.gst_amt) ;
	}
	**************/
	/* If not Direct Charge Get the stock Record */
	if(s_sth.s_type[0] == NON_BULK || s_sth.s_type[0] == BULK) {
		err = CheckStock(s_item->s_stck_cd) ;
		if(err < 0) return(err) ;
	}

	if(method == Gl_1){	
		if((err=Gl_1_Update_Item(s_item,tmp_disc_pst,tmp_disc_gst))<0) 
			return(err);
	}else
	if(method == Gl_2){	
		if((err=Gl_2_Update_Item(s_item,tmp_disc_pst,tmp_disc_gst))<0) 
			return(err);
	}else
	if(method == Gl_3){	
		if((err=Gl_3_Update_Item(s_item,tmp_disc_pst,tmp_disc_gst))<0) 
			return(err);
	}else
		if((err=Gl_4_Update_Item(s_item,tmp_disc_pst,tmp_disc_gst))<0) 
			return(err);

	/* Get the Item account. Here account will be user given account for
	   direct Charge or Non bulk order cases and Inventory gen.
	   account for Bulk order */

	err = CheckGlAcnt(s_sth.s_fund,s_item->s_99accno,99,UPDATE) ;
	if(err < 0) return(err) ;

/*
	if(TransType == INVOICE) { */
	if(Invoice_Crmemo_flag)  {
	    if(s_sth.s_po_no) {	/* If we have a po  number */
		err = GetPOItem(page->PoItemNo[item_indx]);
		if( err != UNDEF && err != NOERROR) return(err);

		/* don't update gl commited (comdat) if additional items added
		   to a invoice referenced by PO.	(CL 1991/01/03) */
		/* if PO item exists update gl commit amts & po amts */
		if (err != UNDEF)
			if((err=Update_Commitments(s_item))<0)return(err);

				/* NO BACKORDERS */
		if(s_sth.s_complete == BOOL_YES){
			if(pa_rec.pa_poinv[0] == YES) {
				stck_rec.st_po_ordqty -= (po_item.pi_orig_qty - 
							po_item.pi_pd_qty);
				if(stck_rec.st_po_ordqty < DELTA_QTY) 
					stck_rec.st_po_ordqty = 0.00;
				stck_rec.st_po_ordqty = D_Roundoff(stck_rec.st_po_ordqty); 
				temp = po_item.pi_value - po_item.pi_paid;	
				stck_rec.st_committed -= Get_Commit_Amt(po_item.pi_tax1,po_item.pi_tax2,temp);	
				if(stck_rec.st_committed <= DELTA_AMT)
					stck_rec.st_committed = 0.00;

				stck_rec.st_committed = D_Roundoff(stck_rec.st_committed); 
			}
			/* Decrease on_order when releasing backorder */
			stck_rec.st_on_order -= (po_item.pi_orig_qty - 
					po_item.pi_pd_qty);
			stck_rec.st_on_order = D_Roundoff(stck_rec.st_on_order); 
			/* If non-bulk update allocations */
	    		if(s_sth.s_type[0] == NON_BULK) {
				stck_rec.st_alloc -= (po_item.pi_orig_qty - 
						po_item.pi_pd_qty);
				stck_rec.st_alloc = D_Roundoff(stck_rec.st_alloc); 
				if((err=Update_po_alloc(s_item))>0)return(err);
			}

		}
	    }else { /* Invoice with NO PO number */
		/* Non bulk orders */
		if(s_sth.s_type[0] == NON_BULK) {
			/* Increase the Commitments */
			gl_rec.comdat += Get_Commit_Amt(s_item->s_gst,
					s_item->s_pst,s_item->s_amount);	

			/* Create/Increase the Allocation */
			err = CheckAllocation(s_item, ADD);
			if(err < 0 && err != UNDEF) return(err);

			/* make new alloc. rec */
			if(err == UNDEF) mode = ADD;
			else mode = UPDATE;

			aloc_rec.st_alloc += s_item->s_qty ;

/******************************************************************************/
/* Change CB - May 14, 1992 The inventory should be the price without the 
   rebate
		        Get_Applied_Taxes(s_item->s_gst,s_item->s_pst,
						s_item->s_amount,&tax_cal);
			aloc_rec.st_value += tax_cal.gros_amt;
************************/
			aloc_rec.st_value += Get_Commit_Amt(s_item->s_gst,
				s_item->s_pst,s_item->s_amount);	
			stck_rec.st_alloc += s_item->s_qty ;
			aloc_rec.st_alloc =  D_Roundoff(aloc_rec.st_alloc);
			aloc_rec.st_value =  D_Roundoff(aloc_rec.st_value);

			err = put_alloc(&aloc_rec, mode, e_mesg) ;
			if(err < 0) return(err) ;
		}
	    }
	}	/* If INVOICE */
	else {
		if(s_sth.s_type[0] == NON_BULK) {
			/* Increase the Commitments */
			gl_rec.comdat -= Get_Commit_Amt(s_item->s_gst,
					s_item->s_pst,s_item->s_amount);	

			/* Create/Increase the Allocation */
			err = CheckAllocation(s_item, ADD);
			if(err < 0 && err != UNDEF) return(err);

			/* make new alloc. rec */
			if(err == UNDEF) mode = ADD;
			else mode = UPDATE;

			aloc_rec.st_alloc -= s_item->s_qty ;

/******************************************************************************/
/* Change CB - May 14, 1992 The inventory should be the price without the 
   rebate
		        Get_Applied_Taxes(s_item->s_gst,s_item->s_pst,
						s_item->s_amount,&tax_cal);
			aloc_rec.st_value -= tax_cal.gros_amt;
************************/
			aloc_rec.st_value -= Get_Commit_Amt(s_item->s_gst,
				s_item->s_pst,s_item->s_amount);	
			stck_rec.st_alloc -= s_item->s_qty ;
			aloc_rec.st_alloc =  D_Roundoff(aloc_rec.st_alloc);
			aloc_rec.st_value =  D_Roundoff(aloc_rec.st_value);

			err = put_alloc(&aloc_rec, mode, e_mesg) ;
			if(err < 0) return(err) ;
			err = put_gl(&gl_rec,UPDATE,e_mesg) ;
			if(err < 0) return(err) ;
		}
	}

	/*
	*	Stock Master Updates
	*/

	/* If not Direct Charge Update stock record */
	if(s_sth.s_type[0] == NON_BULK || s_sth.s_type[0] == BULK) {
		if((err=Update_Stock(s_item)) <0) return(err);
	}

	/* If Direct Charge Debit item amount to Item account */
	if(TransType == INVOICE) {
		/* Update the GL for current item Commitments. GL trans won't
		   be created, because the total of items will be posted to
		   Inventory General account in ProcInvoiceUpdates() */

		err = put_gl(&gl_rec,UPDATE,e_mesg) ;
		if(err < 0) return(err) ;
	}

	/* When QTY > 0.0, if 97 Account is given create 97 GL trans */
	if(s_item->s_qty > DELTA_QTY && s_item->s_97accno[0] != '\0'){

		err = CheckGlAcnt(s_sth.s_fund,s_item->s_97accno,97,UPDATE) ;
		if(err < 0) return(err) ;

		if(Invoice_Crmemo_flag)
			err = CrtGlTrans(&gl97_hdr,&gl97_itm,
					 (double)(s_item->s_qty),DEBIT) ;
		else
			err = CrtGlTrans(&gl97_hdr,&gl97_itm,
					(double)(s_item->s_qty),CREDIT) ;
		if(err < 0) return(err) ;
		Gl97Exists = 1 ;	/* 97 Acnt trans is written */
	}

	err = CrtInItem(s_item) ;	/* Create Invoice Item */
	if(err < 0) return(err) ;

	return(NOERROR) ;
}	/* ProcItemUpdates() */
/*-----------------------------------------------------------------------*/ 
/* Update files or records when PO exist and the item value changed. This
   function will be called only for INVOICE */

ValueChangeUpdates(s_item, diff)
S_item	*s_item ;
double	diff ;
{
	int		err ;
	double		temp,Commit_amt;

	/* Propagate the value change for all the items, which are in pending */

	diff = (po_item.pi_orig_qty - po_item.pi_pd_qty) *
			(diff / s_item->s_qty) ;

	/* Add diff to YTD Ordered in supplier */
	supp_rec.s_ytd_ord += diff ;


	/* If Direct Charge or Bulk Order add diff to YTD commitments */
	/* Here account will be user given account for Direct Charge and
	   inventory gen. acount for bulk order */
	if(s_sth.s_type[0] == NON_BULK){
		/* If Allocation Record availble add diff to Alloc. Record and
		   GL account Commitments */
		err = CheckAllocation(s_item, UPDATE) ;
		if(err < 0 && err != UNDEF) return(err) ;

		if(err == UNDEF) return(NOERROR) ;

		/* Allocation Record Exists */
		if(diff > DELTA_AMT) {	/* +ve diff, i.e increase in value */
/******************************************************************************/
/* Change CB - May 14, 1992 The inventory should be the price without the 
   rebate
		        Get_Applied_Taxes(s_item->s_gst,s_item->s_pst,diff,
								&tax_cal);
			aloc_rec.st_value += tax_cal.gros_amt;
************************/
			aloc_rec.st_value += Get_Commit_Amt(s_item->s_gst,
				s_item->s_pst,diff);	
		}
		else {		/* -ve diff, i.e decrease in value */
			/* If the addition of difference, making value less
			   than zero then add only the current value */
			diff = -diff;
/******************************************************************************/
/* Change CB - May 14, 1992 The inventory should be the price without the 
   rebate
		        Get_Applied_Taxes(s_item->s_gst,s_item->s_pst,diff,
								&tax_cal);
			diff = - tax_cal.gros_amt;
************************/
			diff -= Get_Commit_Amt(s_item->s_gst,
				s_item->s_pst,diff);	
			temp = aloc_rec.st_value + diff ;
			if(temp < -(DELTA_AMT)) {
				alloc_amt  = -(aloc_rec.st_value) ;
				aloc_rec.st_value += alloc_amt;
			}else
				aloc_rec.st_value += diff ;
		}
		aloc_rec.st_value = D_Roundoff(aloc_rec.st_value);
		alloc_amt = D_Roundoff(alloc_amt);
		if(aloc_rec.st_value >= -(DELTA_AMT) &&
						aloc_rec.st_value <= DELTA_AMT)
			err = put_alloc(&aloc_rec,P_DEL,e_mesg) ;
		else
			err = put_alloc(&aloc_rec,UPDATE,e_mesg) ;
		if(err < 0) return(err) ;
	}
	/* Add diff to committed in PO Hdr, and value in PO item */
	po_item.pi_value += diff;
	po_item.pi_value  = D_Roundoff(po_item.pi_value);

	return(NOERROR) ;
}	/* ValueChangeUpdates() */
/*----------------------------------------------------------------*/
/* get the allocation for given keys in UPDATE mode. If record not exists
   and mode = ADD then make the new record and return UNDEF */
static	int
CheckAllocation(s_item, mode)
S_item	*s_item ;
int	mode ;
{
	int	err ;

	aloc_rec.st_fund = s_sth.s_fund ;
	STRCPY(aloc_rec.st_code, s_item->s_stck_cd) ;
	aloc_rec.st_location = s_item->s_school ;
	STRCPY(aloc_rec.st_expacc, s_item->s_99accno) ;

	err = get_alloc(&aloc_rec, UPDATE, 0, e_mesg) ;
	if(ERROR == err) return(DBH_ERR) ;
	if(err != UNDEF && err < 0) {
		fomer(e_mesg) ;
		get() ;
		return(ERROR) ;
	}
	if(err == UNDEF && mode == ADD) {	/* Make the Rec */
		aloc_rec.st_date = s_sth.s_invc_dt ;
		aloc_rec.st_time = get_time() ;
		aloc_rec.st_issued = 0.0 ;
		aloc_rec.st_alloc = 0.0 ;
		aloc_rec.st_value = 0.0 ;
	}
	return(err) ;
}	/* CheckAllocation() */
/*-----------------------------------------------------------------------*/ 
/* Clear the Pending Commitments for the given item. This will be called when
   user wants to close PO, and this item is not part of current Invoice */

UNCommit(page,item_no)
Page	*page; 
int     item_no;
{
	int	err ;

	err = CheckGlAcnt(s_sth.s_fund,page->Items[item_no].s_99accno,
							99,UPDATE) ;
	if(err < 0) return(err) ;
	gl_rec.comdat -= Get_Commit_Amt(page->Items[item_no].s_gst,
			page->Items[item_no].s_pst,page->PoValue[item_no]);
	gl_rec.comdat = D_Roundoff(gl_rec.comdat);
	if(gl_rec.comdat < DELTA_AMT)	/* Shouldn't happen */
		gl_rec.comdat = 0.0 ;

	err = put_gl(&gl_rec, UPDATE, e_mesg);
	if(err < 0) return(err) ;

	return(NOERROR);
}	/* UNCommit */
/*-----------------------------------------------------------------------*/ 
/* Get the PO item in UPDATE mode for a given item# */
static	int
GetPOItem(item_no)
short	item_no ;
{
	int	err ;


	po_item.pi_code = po_hdr.ph_code ;
	po_item.pi_item_no = item_no ;

	err = get_poitem(&po_item, UPDATE, 0, e_mesg) ;
	
	if(UNDEF == err) return(UNDEF);
	if(ERROR == err) return(DBH_ERR) ;
	if(err < 0) {
		fomer(e_mesg) ;
		get() ;
		return(ERROR) ;
	}
	return(NOERROR) ;
}	/* GetPOItem() */
/*-----------------------------------------------------------*/
/* Move Screen Hdr Fields to Invoice */
static	int
InitInvoice()
{
	/* Copy Key */
	if(s_sth.s_payee[0] == '\0')
		STRCPY(in_rec.in_supp_cd, s_sth.s_supp_cd);
	else
		STRCPY(in_rec.in_supp_cd, s_sth.s_payee);

	STRCPY(in_rec.in_invc_no, s_sth.s_invc_no) ;
	STRCPY(in_rec.in_tr_type, s_sth.s_tr_type) ;

	/* Copy Header Fields */
	in_rec.in_type[0]  = s_sth.s_type[0] ;
/*  	in_rec.in_type[1]  = '\0'; */
	in_rec.in_pmtcode[0] = OPEN ;
	STRCPY(in_rec.in_remarks, s_sth.s_remarks) ;
	in_rec.in_funds    = s_sth.s_fund ;
	in_rec.in_period   = s_sth.s_period ;
	in_rec.in_po_no    = s_sth.s_po_no ;
	in_rec.in_invc_dt  = s_sth.s_invc_dt ;
	in_rec.in_due_dt   = s_sth.s_due_dt ;
	if(s_sth.s_disc_pf[0] == AMOUNT) 
		in_rec.in_disc_per = 0.00;
	else {
		in_rec.in_disc_per = s_sth.s_disc_per ;
		in_rec.in_disc_per = D_Roundoff(in_rec.in_disc_per );
	}

	/* For Invoice & Crmemo store amounts as -ve */
	if(TransType == INVOICE || TransType == CRMEMO) {
		in_rec.in_disc_amt = -(s_sth.s_disc_amt);
		in_rec.in_amount   = -(s_sth.s_gr_amt);
		if(s_sth.s_pst_pf[0] == FULL_AMT) {
			in_rec.in_psttax = D_Roundoff(-(s_sth.s_pst_tax - disc_pst));
		}
		else  { if(s_sth.s_pst_pf[0] == PARTIAL) {
				in_rec.in_psttax = -(s_sth.s_pst_tax);
			}
			else {
				in_rec.in_psttax = 0.0;
			}
		}
		if(s_sth.s_gst_pf[0] == FULL_AMT) {
			in_rec.in_gsttax   = D_Roundoff(-(s_sth.s_gst_tax - disc_gst));
		}
		else { if(s_sth.s_gst_pf[0] == PARTIAL) {
				in_rec.in_gsttax = -(s_sth.s_gst_tax);
			}
			else {
				in_rec.in_gsttax = 0.0;
			}
		}
	}
	else {
		in_rec.in_disc_amt = (s_sth.s_disc_amt) ;
		in_rec.in_amount   = s_sth.s_gr_amt;
		if(s_sth.s_pst_pf[0] == FULL_AMT) {
			in_rec.in_psttax = D_Roundoff(s_sth.s_pst_tax - disc_pst);
		}
		else  { if(s_sth.s_pst_pf[0] == PARTIAL) {
				in_rec.in_psttax = (s_sth.s_pst_tax);
			}
			else {
				in_rec.in_psttax = 0.0;
			}
		}
		if(s_sth.s_gst_pf[0] == FULL_AMT) {
			in_rec.in_gsttax   = D_Roundoff(s_sth.s_gst_tax - disc_gst);
		}
		else { if(s_sth.s_gst_pf[0] == PARTIAL) {
				in_rec.in_gsttax = (s_sth.s_gst_tax);
			}
			else {
				in_rec.in_gsttax = 0.0;
			}
		}
	}
	in_rec.in_disc_amt = D_Roundoff(in_rec.in_disc_amt); 
	in_rec.in_amount   = D_Roundoff(in_rec.in_amount);  
	in_rec.in_psttax   = D_Roundoff(in_rec.in_psttax);
	in_rec.in_gsttax   = D_Roundoff(in_rec.in_gsttax);

	/* Transaction is always initialized as open transaction. If it is
	   manual cheque issued transaction, corresponding updation is done
	   ProcGlobalUpdates() function (to write audit records) */
	in_rec.in_chq_no   = 0 ;
	in_rec.in_part_amt = 0.0 ;
	in_rec.in_accno[0] = '\0' ;

	return(NOERROR) ;
}	/* InitInvoice() */
/*-----------------------------------------------------------*/
/* Move Invoice Fields to Invoice Header and set the invoice item no to 0 */
static	int
InitTrans()
{
	int	err ;

	err = UniqueHdrKey(b_sth.b_batch_no) ;
	if(err != NOERROR) return(err) ;

	InitInvcHdr() ;

	/* Intialize Item */
	STRCPY(in_item.i_supp_cd, in_hdr.h_supp_cd) ;
	STRCPY(in_item.i_invc_no, in_hdr.h_invc_no) ;
	STRCPY(in_item.i_tr_type, in_hdr.h_tr_type) ;
	in_item.i_item_no = 0 ;

	return(NOERROR) ;
}	/* InitTrans() */
/*-----------------------------------------------------------------------*/ 
/* Fillup Invoive Item from Screen item and write to file */

CrtInItem(s_item)
S_item	*s_item ;
{
	int	err ;
	double	dtemp;
	double 	ftemp;      

	in_item.i_item_no++ ;
	STRCPY(in_item.i_stck_cd, s_item->s_stck_cd) ;
	if(s_sth.s_type[0] == NON_BULK)
		in_item.i_school = s_item->s_school ;
	else
		in_item.i_school = 0 ;
	STRCPY(in_item.i_99accno, s_item->s_99accno) ;

	if(s_item->s_qty > DELTA_QTY && s_item->s_97accno[0] != '\0')
		STRCPY(in_item.i_97accno, s_item->s_97accno) ;
	else
		in_item.i_97accno[0] = '\0' ;
	/* Invoice amounts should include taxes   */
	dtemp = s_item->s_amount;
	Get_Applied_Taxes(s_item->s_gst,s_item->s_pst,dtemp,&tax_cal);

	dtemp = tax_cal.gros_amt;

	/* Invoice amounts should include freight */
	ftemp=D_Roundoff((remain_freight * s_item->s_qty)/remain_items);

	/* For RETURN & Drmemo store amount and qty as -ve */
	if(TransType == RETURN || TransType == DBMEMO) {
		in_item.i_qty   = -s_item->s_qty ;
		in_item.i_value = -(dtemp + ftemp); /* tax_cal.gros_amt; */
	}
	else {
		in_item.i_qty   = s_item->s_qty ;
		in_item.i_value = (dtemp + ftemp); /* tax_cal.gros_amt; */
	}
	in_item.i_qty   = D_Roundoff(in_item.i_qty );
	in_item.i_value = D_Roundoff(in_item.i_value);

	err = put_initem(&in_item, ADD, e_mesg) ;
	if(err < 0) return(err) ;

	return(NOERROR) ;
}	/* CrtInItem() */
/*-----------------------------------------------------------------------*/ 
/* Initialize Gl Trans. Header and item with necessary information */

InitGlTrans()
{
	int	err ;
#ifdef	ORACLE
	long	get_maxsno(), sno ;
#endif

	/* Setup 99 type Hdr & Trans */

	gl99_hdr.th_fund = s_sth.s_fund ;
	gl99_hdr.th_reccod = 99 ;
	gl99_hdr.th_create[0] = 'G';
#ifndef	ORACLE
	gl99_hdr.th_seq_no = HV_LONG ;

	flg_reset( GLTRHDR ) ;

	err = get_n_trhdr(&gl99_hdr, BROWSE, 0, BACKWARD, e_mesg) ;
	seq_over( GLTRHDR );
	if(err < 0 && err != EFL) {
		fomer(e_mesg); get();
		return(DBH_ERR) ;
	}
	/* If EFL or Key Changed */
	if(err == EFL || gl99_hdr.th_fund != s_sth.s_fund ||
		   gl99_hdr.th_reccod != 99 || gl99_hdr.th_create[0] != 'G') {
		gl99_hdr.th_fund = s_sth.s_fund ;
		gl99_hdr.th_reccod = 99 ;
		gl99_hdr.th_create[0] = 'G';
		gl99_hdr.th_seq_no = 1 ;
	}
	else 
		gl99_hdr.th_seq_no++ ;
	
#else
	sno = get_maxsno(GLTRHDR, (char*)&gl99_hdr, 0, -1, e_mesg);
	if(sno < 0) return(DBH_ERR) ;
	gl99_hdr.th_seq_no = sno + 1 ;
#endif

	STRCPY(gl99_hdr.th_userid,User_Id);
	gl99_hdr.th_sys_dt = s_sth.s_rundate ;
	gl99_hdr.th_period = s_sth.s_period ;
	gl99_hdr.th_date = s_sth.s_rundate ;
	gl99_hdr.th_debits = gl99_hdr.th_credits = 0.0 ;

	STRCPY(gl99_hdr.th_descr, s_sth.s_remarks) ;
	if(TransType == INVOICE) 
		gl99_hdr.th_type[0] = '1' ;	/* Invoice */
	else if(TransType == RETURN) 
		gl99_hdr.th_type[0] = '2' ;	/* Return */
	else if(TransType == CRMEMO) 
		gl99_hdr.th_type[0] = '3' ;	/* Credit Memo */
	else 	/* DBMEMO */
		gl99_hdr.th_type[0] = '4' ;	/* Debit Memo */

	STRCPY(gl99_hdr.th_supp_cd, s_sth.s_supp_cd) ;
	STRCPY(gl99_hdr.th_reference, s_sth.s_invc_no) ;

	/* Write Gl Trans Header to Reserve It so no other Transaction Will
	 * Will get it */
	for( ; ; ) {
		/* Read Record Back in update mode for later Write */
		err = get_trhdr(&gl99_hdr, UPDATE, 0, e_mesg) ;
		if(err < 0) {
			if(err != UNDEF) {
				DispError(e_mesg);
				roll_back(e_mesg) ;	
				return(err);
			}
		} 
		else 
			gl99_hdr.th_seq_no++;
			
		err = put_trhdr(&gl99_hdr, ADD, e_mesg) ;
		if(err < 0) {
			if(err==DUPE) {
				gl99_hdr.th_seq_no++;
				continue;
			}
			DispError(e_mesg);
			roll_back(e_mesg) ;	/* Unlock the locked Records */
			return(err);
		}
		if(err== NOERROR) break;
	}
	/* Read Record Back in update mode for later Write */
	err = get_trhdr(&gl99_hdr, UPDATE, 0, e_mesg) ;
	if(err < 0) {
		DispError(e_mesg);
		roll_back(e_mesg) ;	/* Unlock the locked Records */
		return(err);
	}
	
	/* Initialize Item */
	gl99_itm.ti_fund = gl99_hdr.th_fund ;
	gl99_itm.ti_reccod = gl99_hdr.th_reccod ;
	gl99_itm.ti_create[0] = gl99_hdr.th_create[0];
	gl99_itm.ti_seq_no = gl99_hdr.th_seq_no ;
	gl99_itm.ti_item_no = 0 ;

	gl99_itm.ti_sys_dt = gl99_hdr.th_sys_dt ;
	gl99_itm.ti_period = gl99_hdr.th_period ;
	gl99_itm.ti_status = 0 ;

	/* Setup 97 type Hdr & Trans */

	gl97_hdr.th_fund = s_sth.s_fund ;
	gl97_hdr.th_reccod = 97 ;
	gl97_hdr.th_create[0] = 'G';
#ifndef	ORACLE
	gl97_hdr.th_seq_no = HV_LONG ;

	flg_reset( GLTRHDR ) ;

	err = get_n_trhdr(&gl97_hdr, BROWSE, 0, BACKWARD, e_mesg) ;
	seq_over( GLTRHDR );
	if(err < 0 && err != EFL) return(DBH_ERR) ;
	/* If EFL or Key Changed */
	if(err == EFL || gl97_hdr.th_fund != s_sth.s_fund ||
		   gl97_hdr.th_reccod != 97 || gl97_hdr.th_create[0] != 'G') {
		gl97_hdr.th_fund = s_sth.s_fund ;
		gl97_hdr.th_reccod = 97 ;
		gl97_hdr.th_create[0] = 'G';
		gl97_hdr.th_seq_no = 1 ;
	}
	else
		gl97_hdr.th_seq_no++ ;
#else
	sno = get_maxsno(GLTRHDR, (char*)&gl97_hdr, 0, -1, e_mesg);
	if(sno < 0) return(DBH_ERR) ;
	gl97_hdr.th_seq_no = sno + 1 ;
#endif

	STRCPY(gl97_hdr.th_userid,User_Id);
	gl97_hdr.th_sys_dt = s_sth.s_rundate ;
	gl97_hdr.th_period = s_sth.s_period ;
	gl97_hdr.th_date = s_sth.s_rundate ;
	gl97_hdr.th_debits = gl97_hdr.th_credits = 0.0 ;

	STRCPY(gl97_hdr.th_descr, s_sth.s_remarks) ;
	if(TransType == INVOICE) 
		gl97_hdr.th_type[0] = '1' ;	/* Invoice */
	else if(TransType == RETURN) 
		gl97_hdr.th_type[0] = '2' ;	/* Return */
	else if(TransType == CRMEMO) 
		gl97_hdr.th_type[0] = '3' ;	/* Credit Memo */
	else 	/* DBMEMO */
		gl97_hdr.th_type[0] = '4' ;	/* Debit Memo */

	STRCPY(gl97_hdr.th_supp_cd, s_sth.s_supp_cd) ;
	STRCPY(gl97_hdr.th_reference, s_sth.s_invc_no) ;

	/* Write Gl Trans Header to Reserve It so no other Transaction Will
	 * Will get it */
	for( ; ; ) {
		/* Read Record Back in update mode for later Write */
		err = get_trhdr(&gl97_hdr, UPDATE, 0, e_mesg) ;
		if(err < 0) {
			if(err != UNDEF) {
				DispError(e_mesg);
				roll_back(e_mesg) ;	
				return(err);
			}
		} 
		else 
			gl97_hdr.th_seq_no++;
			
		err = put_trhdr(&gl97_hdr, ADD, e_mesg) ;
		if(err < 0) {
			if(err==DUPE) {
				gl97_hdr.th_seq_no++;
				continue;
			}
			DispError(e_mesg);
			roll_back(e_mesg) ;	/* Unlock the locked Records */
			return(err);
		}
		if(err== NOERROR) break;
	}

	/* Read Record Back in update mode for later Write */
	err = get_trhdr(&gl97_hdr, UPDATE, 0, e_mesg) ;
	if(err < 0) {
		DispError(e_mesg);
		roll_back(e_mesg) ;	/* Unlock the locked Records */
		return(err);
	}

	/* Initialize Item */
	gl97_itm.ti_fund = gl97_hdr.th_fund ;
	gl97_itm.ti_reccod = gl97_hdr.th_reccod ;
	gl97_itm.ti_create[0] = gl97_hdr.th_create[0];
	gl97_itm.ti_seq_no = gl97_hdr.th_seq_no ;
	gl97_itm.ti_item_no = 0 ;

	gl97_itm.ti_sys_dt = gl97_hdr.th_sys_dt ;
	gl97_itm.ti_period = gl97_hdr.th_period ;
	gl97_itm.ti_status = 0 ;

	return(NOERROR) ;
}	/* InitGlTrans() */
/*-----------------------------------------------------------------------*/ 
/* Create the Gl Trans. Item for given amount and type */

CrtGlTrans(gltr_hdr, gltr_itm, amount, type)
Tr_hdr	*gltr_hdr ;
Tr_item	*gltr_itm ;
double	amount ;
int	type ;
{
	int	err ;

	gltr_itm->ti_item_no++ ;
	STRCPY(gltr_itm->ti_accno, gl_rec.accno) ;
	gltr_itm->ti_amount = amount ;
	if(type == CREDIT) gltr_itm->ti_amount *= -1 ;	/* make it -ve */
	gltr_itm->ti_amount = D_Roundoff(gltr_itm->ti_amount);
	gltr_itm->ti_section = gl_rec.sect ;

	/* Update the GL Master record */
	gl_rec.currel[s_sth.s_period - 1] += gltr_itm->ti_amount ;
	gl_rec.currel[s_sth.s_period - 1] = 
				D_Roundoff(gl_rec.currel[s_sth.s_period - 1]);
	gl_rec.ytd += gltr_itm->ti_amount ;
	gl_rec.ytd = D_Roundoff(gl_rec.ytd);
	if(s_sth.s_period == pa_rec.pa_cur_period) {
		if(type == DEBIT)
			gl_rec.curdb += amount ;
		else
			gl_rec.curcr += amount * -1 ;
	}
	gl_rec.curcr = D_Roundoff(gl_rec.curcr); 

	if(type == DEBIT){
/*
		printf("\nDebit %s %lf",gl_rec.accno,amount); 
*/
		gltr_hdr->th_debits += amount ;
		gltr_hdr->th_debits  = D_Roundoff(gltr_hdr->th_debits);  
	}else{
/*
		printf("\nCredit %s %lf",gl_rec.accno,amount); 
*/
		gltr_hdr->th_credits += amount ;
		gltr_hdr->th_credits = D_Roundoff(gltr_hdr->th_credits);  
	}

/*
	get(); 
*/

	err = put_tritem(gltr_itm, ADD, e_mesg) ;
	if(err < 0) return(err) ;

	/* Update The GL Account */
	err = put_gl(&gl_rec,UPDATE,e_mesg) ;
	if(err < 0) return(err) ;

	return(NOERROR) ;
}	/* CrtGlTrans() */
/*------------------------------------------------------------------------*/
/* Move High values to all data fields and clear the screen */

ClearScreen()
{
	int	i;

	/* Move High Values to Hedaer part */
	InitPoPayee(HV_CHAR, HV_LONG); 
	InitHdr(HV_CHAR, HV_SHORT, HV_INT, HV_LONG, HV_DOUBLE) ;
	s_sth.s_page_no    = HV_SHORT ;

	/* Move High Values to All items */
	for(i = 0 ; i < PAGESIZE ; i++)
		InitItem(NOOP,i, HV_CHAR, HV_SHORT, HV_DOUBLE) ;
	Init_Amt_Totals(HV_DOUBLE,HV_CHAR);
	ret(WriteFields(HDR_ST_FLD, (END_FLD - 200))); 
	return(NOERROR);
}	/* ClearScreen() */
/*-------------------------------------------------------------------------*/
/* Initialize Screen Header with either Low or High Values */
InitPoPayee( t_char, t_long )
char	t_char ;
long	t_long ;
{
	if(t_long == LV_LONG) {
		/* Accept PO# only for INVOICE */
		if(TransType == INVOICE)
			s_sth.s_po_no = t_long ;
		else
			s_sth.s_po_no = 0 ;
	}
	else {
		s_sth.s_po_no     = t_long ;
	}
	s_sth.s_payee[0]   = t_char;

	return(NOERROR);
}
/*-------------------------------------------------------------------------*/
/* Initialize Screen Header with either Low or High Values */

InitHdr( t_char, t_short, t_int, t_long,t_double )
char	t_char ;
short	t_short ;
int	t_int ;
long	t_long ;
double	t_double ;
{
	s_sth.s_period     = t_short ;
	s_sth.s_invc_dt    = t_long ;
	s_sth.s_due_dt     = t_long ;
	s_sth.s_gr_amt     = t_double ;
	s_sth.s_pst_pf[0]  = t_char;
	s_sth.s_pst_tax    = t_double ;
	s_sth.s_gst_pf[0]  = t_char;
	s_sth.s_gst_tax    = t_double ;
	s_sth.s_net_amt    = t_double ;
	s_sth.s_disc_pf[0] = t_char;
	s_sth.s_chq_no     = t_long ;
	s_sth.s_bank_acct[0]= t_char ;

	if(t_char == HV_CHAR){
		s_sth.s_misc_hdr[0] = t_char;
	}
	else{
		s_sth.s_misc_hdr[0] = ' ';
	}
	s_sth.s_misc_gst[0] = t_char;
	s_sth.s_misc_reb = t_short;
	s_sth.s_misc_pst[0] = t_char;
	s_sth.s_misc_amt = t_double;
	
	if(t_short == LV_SHORT) {
		s_sth.s_remarks[0]  = t_char ;
		s_sth.s_disc_per   = t_double;

		/* If Inventory system not present, always Direct
		   Charge */
		if(TransType == INVOICE) {
			if(pa_rec.pa_stores[0] == YES) {
				if(s_sth.s_po_no == 0) {
					s_sth.s_type[0] = t_char ;
				}
			}
			else {
				s_sth.s_type[0] = DIRECT ;
				s_sth.s_type[1] = '\0' ;
			}
			s_sth.s_fund = LV_SHORT;
		}
		else {
			/* For CM, DM & RT types copy type from invoice
			   if it exists */
			if(pa_rec.pa_stores[0] != YES) {
				s_sth.s_type[0] = DIRECT ;
				s_sth.s_type[1] = '\0' ;
			}
			else if(InvcExists) {
				s_sth.s_type[0] = in_rec.in_type[0] ;
				s_sth.s_type[1] = '\0';
				s_sth.s_fund    = in_rec.in_funds ;
			}
			else {
				s_sth.s_type[0] = t_char ;
				s_sth.s_fund = LV_SHORT;
			}
		}

		/* Following field applicable only for Invoice */
		if(TransType == INVOICE)
			if(s_sth.s_po_no != 0)
				s_sth.s_complete = t_int ;
			else 
				s_sth.s_complete = HV_INT ;

		/* If the supplier is ordinary type supplier show as "Discount"
		   else show as "Hold Back" */
		if(supp_rec.s_type[0] == ORDINARY) {
#ifdef ENGLISH
			STRCPY(s_sth.s_per_pr, "Disc(P/A):");
			STRCPY(save_amt_pr, "Disc Amt :");
#else
			STRCPY(s_sth.s_per_pr, "Esc (P/M):");
			STRCPY(save_amt_pr, "Mont. esc:");
#endif
			s_sth.s_disc_pf[0] = t_char;
		}
		else {
#ifdef ENGLISH
			STRCPY(s_sth.s_per_pr, "HB (P/A) :");
			STRCPY(save_amt_pr, "HB Amt   :");
#else
                        STRCPY(s_sth.s_per_pr, "PR (P/M) :");
			STRCPY(save_amt_pr, "Mont. PR :");
#endif
		}
	}
	else {
		s_sth.s_po_no     = t_long ;
		s_sth.s_type[0]   = t_char ;
		s_sth.s_fund 	  = t_short;
		s_sth.s_due_dt    = t_long ;
		s_sth.s_complete  = t_int ;
		s_sth.s_disc_pf[0] = t_char;
		s_sth.s_per_pr[0] = t_char ;
		s_sth.s_disc_per   = t_double;
		s_sth.s_hdr_line[0] = t_char;
		s_sth.s_remarks[0]  = t_char ;
	}
/* added by F.Tao 12/30/1990 */
	if(TransType == RETURN || TransType == DBMEMO){
		s_sth.s_chq_no 		= 0;
		STRCPY(s_sth.s_bank_acct, " ") ;
	}

	return(NOERROR) ;
}	/* InitHdr() */
/*-------------------------------------------------------------------------*/
/* Initialize Given screen item with either Low values or High values */

InitItem(mode,item_no, t_char, t_short, t_double)
int     mode;
int	item_no ;
char	t_char ;
short	t_short ;
double  t_double;
{
int st_fld, err;
char	gst_registration[11];

	s_sth.s_items[item_no].s_97accno[0] = t_char ;
	s_sth.s_items[item_no].s_qty 	    = t_double ;
	s_sth.s_items[item_no].s_amount     = t_double ;
	s_sth.s_items[item_no].s_desc[0]    = t_char;

	if(t_short == LV_SHORT) {
	    /* Move LVs to Stock code all the cases except INVOICE with PO */
		if( !( (TransType == INVOICE) && (s_sth.s_po_no) && 
							(mode != ADD) ) )
			if(s_sth.s_type[0] != DIRECT)
				s_sth.s_items[item_no].s_stck_cd[0] = t_char ;
/****************************/
/*  Commented this out so CC# would be prompted for any transaction for
	nonbulk order without a PO# 
	    	if( TransType == INVOICE && s_sth.s_po_no == 0 &&
				s_sth.s_type[0] == NON_BULK)
****************************/
	    /* Move LVs to School only when Non Bulk without PO */
	    	if( s_sth.s_po_no == 0 && s_sth.s_type[0] == NON_BULK)
			s_sth.s_items[item_no].s_school = t_short ;

	    /* Move Low Values to 99 Account# all cases, except INVOICE
	       with PO. For BULK INV gen. acct will be moved to this in */

	    	if(!(TransType == INVOICE && s_sth.s_po_no))
			s_sth.s_items[item_no].s_99accno[0]= t_char;

	/* Allow added items 99accno to be entered if invoice references PO. 
								CL 1991/01/03 */

	    	if ((s_sth.s_po_no != 0) && (bal_items_added))
			s_sth.s_items[item_no].s_99accno[0]= t_char;

		if( global_mode == ADDITEMS){ 
			s_sth.s_items[item_no].s_pst[0] = pa_rec.pa_pst_tax[0];
			/* set gst defaults */
			if(pa_rec.pa_dist_gst[0] == YES) {
				if(s_sth.s_payee[0] != '\0') {
					STRCPY(gst_registration,
							payee_rec.s_gst_reg);
				}
				else {
					STRCPY(gst_registration,
							supp_rec.s_gst_reg);
				}
				if(gst_registration[0] != '\0') {
					/* if GST registration number */
					/* default to parameter settings */
					s_sth.s_items[item_no].s_gst[0] = 
						pa_rec.pa_gst_tax[0];
				}
				else {
					/* if no GST registration number */
					/* default to tax exempt */
					s_sth.s_items[item_no].s_gst[0] = 
						EXEMPT;
				}
			}
			else {
				s_sth.s_items[item_no].s_gst[0] = 
					pa_rec.pa_gst_tax[0];
			}	
			s_sth.s_items[item_no].s_rebate = ctl_rec.rebate;
			st_fld  = ITEM_ST_FLD + (STEP * item_no) + GST_FLD;
  			SetDupBuffers(st_fld,st_fld+200,2);
		}
	  	s_sth.s_items[item_no].s_gst[0]     = t_char;
	  	s_sth.s_items[item_no].s_rebate     = t_short;
	    	s_sth.s_items[item_no].s_pst[0]     = t_char;

	    	STRCPY(s_sth.s_items[item_no].s_status, ACTIVE) ;
	}
	else {
		s_sth.s_items[item_no].s_sno        = t_short;
	    	s_sth.s_items[item_no].s_stck_cd[0] = t_char ;
	    	s_sth.s_items[item_no].s_school     = t_short ;
	    	s_sth.s_items[item_no].s_99accno[0] = t_char ;
	    	s_sth.s_items[item_no].s_status[0]  = t_char ;
	    	s_sth.s_items[item_no].s_pst[0]     = t_char;
	    	s_sth.s_items[item_no].s_gst[0]     = t_char;
	    	s_sth.s_items[item_no].s_rebate     = t_short;
	}

	return(NOERROR) ;
}	/* Inititem() */

Init_Amt_Totals(t_double,t_char)
double	t_double;
char    t_char;
{
    s_sth.s_disc_amt  = t_double;
    s_sth.s_gst_amt   = t_double;
    s_sth.s_pst_amt   = t_double;
    s_sth.s_item_cost = t_double;
    s_sth.s_amt_pr[0] = t_char;
    s_sth.s_inv_total = t_double;
}
Init_Balance_Item(item)
S_item *item;
{
int i;
	scpy((char*)(item),(char*)&(FirstPage->Items[0]),sizeof(S_item));
	for(i=0;i<sizeof(item->s_desc);i++) item->s_desc[i] = ' ';	
	STRCPY(item->s_desc,"Account Balance");
	item->s_97accno[0] = HV_CHAR;
	item->s_gst[0] = EXEMPT;
	item->s_pst[0] = TAXABLE;
	item->s_qty = 0;
	item->s_amount = LV_DOUBLE;
}
/*--------------------------------------------------------------------------
	Sum total cost and discount for all active items in list. 
	Calculate and sum Gst and Pst for each item.
	Display calculations at bottom of screen.
----------------------------------------------------------------------------*/
Update_Pst_Gst_Cost()
{
Page *cur_page;
short pst_tax,gst_tax;
double dtemp,item_gst,item_pst;
int i;
   /* Init variables */
	s_sth.s_disc_amt = s_sth.s_item_cost = s_sth.s_pst_amt = 
						s_sth.s_gst_amt = 0.0;
	disc_gst = disc_pst = 0.0;

	s_sth.s_amt_pr[0] = HV_CHAR;

	/* sum Item costs */ 
	s_sth.s_item_cost += s_sth.s_misc_amt;

	/* calculate tax on item before taxes */
	dtemp = s_sth.s_misc_amt;
	Get_Applied_Taxes(s_sth.s_misc_gst,s_sth.s_misc_pst,dtemp,&tax_cal);
	s_sth.s_pst_amt += tax_cal.pst_amt;
	s_sth.s_gst_amt += tax_cal.gst_amt;
	if(InvcLast == NULL) return(NOERROR);
	STRCPY(s_sth.s_amt_pr,save_amt_pr);

	cur_page = FirstPage;
	for(cur_page=FirstPage;cur_page != NULL;cur_page = cur_page->NextPage){

		for(i=0;i<cur_page->NoItems;i++){

			if(strcmp(cur_page->Items[i].s_status,ACTIVE) != 0)
				continue;
			/* Sum Item Costs */
	     		s_sth.s_item_cost+= cur_page->Items[i].s_amount;

			/* If qty zero must be Balance account entry */
	           	if(cur_page->Items[i].s_qty == 0.0) continue;

			/* calculate tax on item before taxes */
			dtemp = cur_page->Items[i].s_amount;
			Get_Applied_Taxes(cur_page->Items[i].s_gst,
				       cur_page->Items[i].s_pst,dtemp,&tax_cal);
			s_sth.s_pst_amt += tax_cal.pst_amt;
			s_sth.s_gst_amt += tax_cal.gst_amt;
			item_gst = tax_cal.gst_amt;
			item_pst = tax_cal.pst_amt;

			if(s_sth.s_disc_pf[0] == PERCENT){
				/* Calculate discount on item */
				dtemp = s_sth.s_disc_per;
				dtemp = cur_page->Items[i].s_amount * 
								 (dtemp/100.0);
				/* Sum discount */ 
				dtemp = D_Roundoff(dtemp);
				s_sth.s_disc_amt += dtemp;
				/* discount per item */
				cur_page->DiscAmt[i] = dtemp;

				/* get taxes on item after discount */
				dtemp = cur_page->Items[i].s_amount
							 - cur_page->DiscAmt[i];

				Get_Applied_Taxes(cur_page->Items[i].s_gst,
				       cur_page->Items[i].s_pst,dtemp,&tax_cal);

				/* Calculate tax savings after discount */
				dtemp =D_Roundoff((item_gst - tax_cal.gst_amt));
				disc_gst += dtemp;
				s_sth.s_disc_amt += dtemp; 

				dtemp =D_Roundoff((item_pst - tax_cal.pst_amt));
				disc_pst += dtemp;
				s_sth.s_disc_amt += dtemp; 
			}else
				cur_page->DiscAmt[i] = 0;
				
		}/* end for */
		if(InvcLast == cur_page) break;
	}/* end for */

	if(s_sth.s_disc_pf[0] == AMOUNT)
		s_sth.s_disc_amt = D_Roundoff(s_sth.s_disc_per);

	s_sth.s_inv_total =  s_sth.s_item_cost - s_sth.s_disc_amt;

	if(s_sth.s_gst_pf[0] == FULL_AMT){
		s_sth.s_gst_tax=s_sth.s_gst_amt;
		s_sth.s_inv_total += disc_gst;
		WriteFields(GST_TAX_FLD,GST_TAX_FLD);
		Update_Net_Amt();
	}else if(s_sth.s_gst_pf[0] == PARTIAL){
		s_sth.s_inv_total += s_sth.s_gst_amt - s_sth.s_gst_tax;
	}else 
		s_sth.s_inv_total += s_sth.s_gst_amt;

	if(s_sth.s_pst_pf[0] == FULL_AMT){
		s_sth.s_pst_tax=s_sth.s_pst_amt;
		s_sth.s_inv_total += disc_pst;
		WriteFields(PST_TAX_FLD,PST_TAX_FLD);
		Update_Net_Amt();
	}else if(s_sth.s_pst_pf[0] == PARTIAL){
		s_sth.s_inv_total += s_sth.s_pst_amt - s_sth.s_pst_tax;
	}else 
		s_sth.s_inv_total += s_sth.s_pst_amt;

	WriteFields(START_AMT_FLD,END_AMT_FLD);

	return(NOERROR);
}
/*--------------------------------------------------------------------------
	Calculate the rebate amount given the Gst amount
---------------------------------------------------------------------------*/
static double Cal_Rebate(gst_rebate,gst_amt)
short	gst_rebate;
double gst_amt; /* This is a local variable */
{
double temp;

	temp  = gst_amt * (((double)gst_rebate)/100);
	temp = D_Roundoff(temp);
	return(temp);
}
/*----------------------------------------------------------------------------
General Ledger Item Update

	Updates Gl accounts when gst and pst tax are included on Supplier
	Invoice. Supplier collects Gst and Pst.
_____________________________________________________________________________*/
static Gl_1_Update_Item(item,tmp_disc_pst,tmp_disc_gst)
S_item *item;
double	tmp_disc_pst;
double	tmp_disc_gst;
{
int err;
double item_rebate;
double item_amt;
double freight_rebate;
double freight_gst;
double freight_amt;
double rebate;
double temp;

	/* Calculate Gst Pst taxes from gross amount */
	Get_Applied_Taxes(item->s_gst,item->s_pst,(double)item->s_amount,
								&tax_cal);
	item_amt = tax_cal.gros_amt;
	/* Calculate Gst rebate for item amount */
	item_rebate = Cal_Rebate(item->s_rebate,tax_cal.gst_amt);
	/* Calculate Gst Pst taxes for freight amount */
	freight_amt = D_Roundoff((remain_freight * item->s_qty)/remain_items);
	freight_gst = D_Roundoff((remain_f_gst * item->s_qty)/remain_items);

	/* Calculate Gst rebate for freight amount */
	freight_rebate = Cal_Rebate(s_sth.s_misc_reb, freight_gst);


	if((err=CheckGlAcnt(s_sth.s_fund,item->s_99accno,99,UPDATE))<0)
		return(err);

	/* Debit Item Account99 with  item amount*/
	temp = D_Roundoff((item_amt+freight_amt-tmp_disc_pst-tmp_disc_gst));
	if(Invoice_Crmemo_flag) 
	    	err=CrtGlTrans(&gl99_hdr,&gl99_itm,temp,DEBIT);
	else
	    	err=CrtGlTrans(&gl99_hdr,&gl99_itm,temp,CREDIT);
	if(err < 0) return(err) ;

	rebate=D_Roundoff(item_rebate + freight_rebate);
	if (rebate > DELTA_AMT){
		/* Credit Item Account99 with gst rebate */
		if(Invoice_Crmemo_flag)
			err=CrtGlTrans(&gl99_hdr,&gl99_itm,rebate,CREDIT);
		else
			err=CrtGlTrans(&gl99_hdr,&gl99_itm,rebate,DEBIT);
		if (err < 0) return(err) ;
	}

	/* Running Total A/P account for current transaction */
/******
	ap_general_acnt += tax_cal.gros_amt;
********/
	if(rebate > DELTA_AMT){
		if(pa_rec.pa_dist_gst[0] == YES) {
			err = CrtGSTTrans(item->s_99accno,rebate);
			if(err < 0) {
				return(err);
			}
		}
		else {
			/* Debit/Credit GST receivable account with rebate */
			if((err=CheckGlAcnt(s_sth.s_fund,ctl_rec.ar_gst_acnt,
								99,UPDATE))<0)
				return(err);
			if(Invoice_Crmemo_flag)
			    err=CrtGlTrans(&gl99_hdr,&gl99_itm,rebate,DEBIT);
			else
			    err=CrtGlTrans(&gl99_hdr,&gl99_itm,rebate,CREDIT);
			if (err < 0) return(err) ;
		}
	}
	return(NOERROR);
}
/*----------------------------------------------------------------------------
General Ledger Item Update

	Updates Gl accounts when gst and pst tax are NOT included on Supplier
	Invoice. Supplier does not collect gst or pst.
_____________________________________________________________________________*/

static Gl_2_Update_Item(item,tmp_disc_pst,tmp_disc_gst)
S_item *item;
double	tmp_disc_pst;
double 	tmp_disc_gst;
{
int err;
double item_reb,item_amt;
double freight_reb,freight_amt;
double item_gst, freight_gst;
double	temp;

	/* Calculate Gst Pst taxes from gross amount */
	Get_Applied_Taxes(item->s_gst,item->s_pst,(double)item->s_amount,
								&tax_cal);
	item_amt = D_Roundoff(tax_cal.gros_amt);
	item_reb = Cal_Rebate(item->s_rebate,tax_cal.gst_amt);
	item_gst = D_Roundoff(tax_cal.gst_amt);

	/* Calculate Gst Pst taxes from gross amount */
	freight_amt = D_Roundoff((remain_freight * item->s_qty)/remain_items);
	freight_gst = D_Roundoff((remain_f_gst * item->s_qty)/remain_items);

	/* Calculate Gst rebate for freight amount */
	freight_reb = D_Roundoff(Cal_Rebate(s_sth.s_misc_reb, freight_gst));

	if((err=CheckGlAcnt(s_sth.s_fund,item->s_99accno,99,UPDATE))>0)
		return(err);

	item_amt = (item_amt - item_reb) + (freight_amt - freight_reb);

	/* Debit Item Account99 with  item amount less rebate*/
	temp = D_Roundoff((item_amt - tmp_disc_pst - tmp_disc_gst)); 
	if(TransType == INVOICE || TransType == CRMEMO )
		err=CrtGlTrans(&gl99_hdr,&gl99_itm,temp,DEBIT);
	else
		err=CrtGlTrans(&gl99_hdr,&gl99_itm,temp,CREDIT);
	if (err < 0) return(err) ;
/********
	ap_general_acnt += tax_cal.gros_amt - tax_cal.gst_amt - tax_cal.pst_amt;
	pst_payable += tax_cal.pst_amt;
*******/

	/* Credit Gst Tax Account with unrefundable part of GST */
	item_amt = (item_gst -item_reb) + (freight_gst - freight_reb);
	if(item_amt > DELTA_AMT){
		if(pa_rec.pa_dist_gst[0] == YES) {
			err = CrtGSTTrans(item->s_99accno,item_amt);
			if(err < 0) {
				return(err);
			}
		}
		else {
			if((err=CheckGlAcnt(s_sth.s_fund,ctl_rec.gst_tax_acnt,
								99,UPDATE))<0)
				return(err);
			if(TransType == INVOICE || TransType == CRMEMO )
	   		    err=CrtGlTrans(&gl99_hdr,&gl99_itm,item_amt,CREDIT);
			else
			    err=CrtGlTrans(&gl99_hdr,&gl99_itm,item_amt,DEBIT);
			if (err < 0) return(err) ;
		}
	}
	return(NOERROR);
}

static Gl_4_Update_Item(item,tmp_disc_pst,tmp_disc_gst)
S_item *item;
double	tmp_disc_pst ;
double	tmp_disc_gst ;
{
int err;
double item_reb,item_amt;
double freight_reb, freight_amt;
double item_gst, freight_gst;
double	temp;

	/* Calculate Gst Pst taxes from gross amount */
	Get_Applied_Taxes(item->s_gst,item->s_pst,(double)item->s_amount,
								&tax_cal);
	item_amt = tax_cal.gros_amt;
	item_reb = Cal_Rebate(item->s_rebate,tax_cal.gst_amt);
	item_gst = tax_cal.gst_amt;

	/* Calculate Gst Pst taxes from gross amount */
	freight_amt = D_Roundoff((remain_freight * item->s_qty)/remain_items);
	freight_gst = D_Roundoff((remain_f_gst * item->s_qty)/remain_items);

	/* calculate gst rebate for freight amount */
	freight_reb = Cal_Rebate(s_sth.s_misc_reb, freight_gst);

	if((err=CheckGlAcnt(s_sth.s_fund,item->s_99accno,99,UPDATE))>0)
		return(err);

	item_amt = (item_amt - item_reb) + (freight_amt - freight_reb);

	/* Debit Item Account99 with  item amount less rebate*/
	temp = D_Roundoff((item_amt - tmp_disc_pst - tmp_disc_gst)); 
	if(TransType == INVOICE || TransType == CRMEMO )
		err=CrtGlTrans(&gl99_hdr,&gl99_itm,temp,DEBIT);
	else
		err=CrtGlTrans(&gl99_hdr,&gl99_itm,temp,CREDIT);
	if (err < 0) return(err) ;

	/* Running Total A/P account for current invoice */
/******
	ap_general_acnt += tax_cal.gros_amt - tax_cal.gst_amt;
*******/

	/* Credit Gst Tax Account with unrefundable part of GST */
	item_amt = (item_gst - item_reb) + (freight_gst - freight_reb);
	if(item_amt > DELTA_AMT){
		if(pa_rec.pa_dist_gst[0] == YES) {
			err = CrtGSTTrans(item->s_99accno,item_amt);
			if(err < 0) {
				return(err);
			}
		}
		else {
			if((err=CheckGlAcnt(s_sth.s_fund,ctl_rec.gst_tax_acnt,
								99,UPDATE))<0)
				return(err);
			if(TransType == INVOICE || TransType == CRMEMO )
			    err=CrtGlTrans(&gl99_hdr,&gl99_itm,item_amt,CREDIT);
			else
			    err=CrtGlTrans(&gl99_hdr,&gl99_itm,item_amt, DEBIT);
			if (err < 0) return(err) ;
		}
	}
	return(NOERROR);
}
/*----------------------------------------------------------------------------
General Ledger Item Update

	Updates Gl accounts when Pst is NOT included on Supplier
	Invoice. The Supplier collects Gst. 
___________________________________________________________________________*/

static Gl_3_Update_Item(item,tmp_disc_pst,tmp_disc_gst)
S_item *item;
double	tmp_disc_pst ;
double 	tmp_disc_gst ;
{
int err;
double item_rebate;
double item_amt;
double freight_rebate;
double freight_gst;
double freight_amt;
double rebate;
double temp;

	/* Calculate Gst Pst taxes for item amount */

	Get_Applied_Taxes(item->s_gst,item->s_pst,(double)item->s_amount,
								&tax_cal);
	item_amt = tax_cal.gros_amt;
	/* Calculate Gst rebate for item amount */
	item_rebate = Cal_Rebate(item->s_rebate,tax_cal.gst_amt);
	/* Calculate Gst Pst taxes for freight amount */
	freight_amt = D_Roundoff((remain_freight * item->s_qty)/remain_items);
	freight_gst = D_Roundoff((remain_f_gst * item->s_qty)/remain_items);

	/* Calculate Gst rebate for freight amount */
	freight_rebate = Cal_Rebate(s_sth.s_misc_reb,freight_gst);

	/* Running Total A/P account for current invoice */
/********
	ap_general_acnt += item->s_amount + tax_cal.gst_amt;
	pst_payable += tax_cal.pst_amt;
********/

	/* Update item account */
	if((err=CheckGlAcnt(s_sth.s_fund,item->s_99accno,99,UPDATE))>0)
		return(err);
	temp = D_Roundoff((item_amt+freight_amt-tmp_disc_pst - tmp_disc_gst));
	if(TransType == INVOICE || TransType == CRMEMO)
		err=CrtGlTrans(&gl99_hdr,&gl99_itm,temp,DEBIT);
	else
		err=CrtGlTrans(&gl99_hdr,&gl99_itm,temp,CREDIT);
	if(err<0) return(err);

	rebate = D_Roundoff(item_rebate + freight_rebate);

	/* if rebate apply to item account */
	if (rebate > DELTA_AMT){
		/* Update Item Account with gst rebate */
		if(Invoice_Crmemo_flag)
			err=CrtGlTrans(&gl99_hdr,&gl99_itm,rebate,CREDIT);
		else
			err=CrtGlTrans(&gl99_hdr,&gl99_itm,rebate,DEBIT);
		if (err < 0) return(err) ;

		if(pa_rec.pa_dist_gst[0] == YES) {
			err = CrtGSTTrans(item->s_99accno,rebate);
			if(err < 0) {
				return(err);
			}
		}
		else {
			/* Update Gst Receivable account */
			if((err=CheckGlAcnt(s_sth.s_fund,ctl_rec.gst_tax_acnt,
								99,UPDATE))<0)
				return(err);
			if(TransType == INVOICE || TransType == CRMEMO )
			    err=CrtGlTrans(&gl99_hdr,&gl99_itm, rebate, DEBIT);
			else
    			    err=CrtGlTrans(&gl99_hdr,&gl99_itm, rebate, CREDIT);
			if (err < 0) return(err) ;
		}
	}

	return(NOERROR);
}
/*-------------------------------------------------------------------------*/
CrtGSTTrans(exp_accno,rebate)
char	*exp_accno;		/* expense account */
double	rebate;
{
	GST_dist	gst_rec;
	int	err;

	gst_rec.fund = s_sth.s_fund;
	STRCPY(gst_rec.accno,exp_accno);
	err = get_gstacct(&gst_rec,BROWSE,1,e_mesg);
	if(err < 0) {
		return(err);
	}

	/* Update Gst link account */
	if((err=CheckGlAcnt(gst_rec.fund,gst_rec.gst_accno,99,UPDATE))<0)
		return(err);

	if(TransType == INVOICE || TransType == CRMEMO )
		err=CrtGlTrans(&gl99_hdr,&gl99_itm, rebate, DEBIT);
	else
		err=CrtGlTrans(&gl99_hdr,&gl99_itm, rebate, CREDIT);

	if (err < 0) return(err) ;
	
	return(NOERROR);
}
/*-------------------------------------------------------------------------
Get Applied Taxes 
/*-------------------------------------------------------------------------
Get Applied Taxes 
	gst_flg		- E/T (EXEMPT/TAXABLE)
	pst_flg 	- E/T (EXEMPT/TAXABLE)
	gros_amt	- gross amount includes no taxes
	tax_cal		- structure in which gst,pst and net amt are returned

---------------------------------------------------------------------------*/
static Get_Applied_Taxes(gst_flag,pst_flag,gros_amt,tax_cal)
char gst_flag[],pst_flag[];
double gros_amt;
Tax_cal *tax_cal;
{
short gst_tax,pst_tax;
	/* get applied gst pst tax rates */
	gst_tax = (gst_flag[0] == TAXABLE) ? ctl_rec.gst_tax : 0;
	pst_tax = (pst_flag[0] == TAXABLE) ? ctl_rec.pst_tax : 0;
	/* calculate gst pst amounts */
	Tax_Calculation(gst_tax,pst_tax,gros_amt,tax_cal);
}
/*----------------------------------------------------------------------------*/
/*same as above but uses different tax calculation program */
static Get_Taxes(gst_flag,pst_flag,gros_amt,tax_cal)
char gst_flag[],pst_flag[];
double gros_amt;
Tax_cal *tax_cal;
{
short gst_tax,pst_tax;
	/* get applied gst pst tax rates */
	gst_tax = (gst_flag[0] == TAXABLE) ? ctl_rec.gst_tax : 0;
	pst_tax = (pst_flag[0] == TAXABLE) ? ctl_rec.pst_tax : 0;
	/* calculate gst pst amounts */
	Tax_Calc(gst_tax,pst_tax,gros_amt,tax_cal);
}
Tax_Calc(gst_tax,pst_tax,net_amt,tax_ret)
short gst_tax,pst_tax;
double net_amt;
Tax_cal *tax_ret;
{
	if(gst_tax == 0)
		tax_ret->gst_amt = 0.0;
	else{ 
		tax_ret->gst_amt = net_amt *((double)gst_tax/100.0);
		/******D_Roundup(&tax_ret->gst_amt);
		tax_ret->gst_amt = D_Roundoff(tax_ret->gst_amt
		*********************/
	}

	if(pst_tax == 0)
		tax_ret->pst_amt = 0.0;
	else{
		tax_ret->pst_amt=(net_amt + tax_ret->gst_amt)
				* ((double)pst_tax/100.0);
	}
	tax_ret->gros_amt = net_amt + tax_ret->gst_amt + tax_ret->pst_amt;
	/* add nest 3 lines and commented out rounding above */
	tax_ret->gst_amt = D_Roundoff(tax_ret->gst_amt);
	tax_ret->pst_amt = D_Roundoff(tax_ret->pst_amt);
	tax_ret->gros_amt = D_Roundoff(tax_ret->gros_amt);
}
/*-------------------------------------------------------------------------
Get Commited Amount
	gst_flag - E/T (EXEMPT/TAXABLE)
	pst_flag - E/T (EXEMPT/TAXABLE)
	gros_amt - Gross amount ;No taxes included

	commit   - returns committed amount.(gst tax less rebate)
--------------------------------------------------------------------------*/	
static double Get_Commit_Amt(gst_flag,pst_flag,gros_amt)
char gst_flag[],pst_flag[];
double gros_amt;
{
double rebate;

		Get_Applied_Taxes(gst_flag,pst_flag,gros_amt,&tax_cal);
		rebate =Cal_Rebate(ctl_rec.rebate,tax_cal.gst_amt);
		rebate =gros_amt + tax_cal.pst_amt + (tax_cal.gst_amt - rebate);
		return(rebate);
}
/* same as above only calculates all taxes befor rounding */
static double Get_Commitf_Amt(freight,gst_amt)
double freight;
double gst_amt;
{
double rebate;
		rebate = Cal_Rebate(s_sth.s_misc_reb,gst_amt);
		rebate = (freight - rebate);
		return(rebate);
}
/*--------------------------------------------------------------------------
Get Gst/Pst form net amount
	net_amount 	- Includes taxes depending on flags
	gst_flg		- '1' gst is included in net amount
	pst_flg		- '1' pst is included in net amount

	tax_cal		- structure in which gst pst tax is returned
--------------------------------------------------------------------------*/

static Get_Gst_Pst_Frm_Net(net_amt,gst_flg,pst_flg,tax_cal)
double net_amt;
int gst_flg,pst_flg;
Tax_cal *tax_cal;
{
double temp;

	tax_cal->gst_amt = tax_cal->pst_amt  = 0.0;
	if (pst_flg == 1){
   		tax_cal->pst_amt = net_amt - (net_amt / 
				(1.0 + (((double)ctl_rec.pst_tax)/100.0)));  
		tax_cal->pst_amt = D_Roundoff(tax_cal->pst_amt);
	}
	temp  = net_amt - tax_cal->pst_amt;   
	if (gst_flg == 1){
		tax_cal->gst_amt = temp  - 
			((temp)/(1.0 + (((double)ctl_rec.gst_tax)/100.0)));
		tax_cal->gst_amt = D_Roundoff(tax_cal->gst_amt);
	}
}

static Balance_Account(amount,s)
double amount;
char *s;
{
int err;

	if((err=CheckGlAcnt(s_sth.s_fund,FirstPage->Items[0].s_99accno,99,
								UPDATE))<0)
		return(err);
	/* Debit Item Account99 with  item amount*/
	if (amount > 0){
		if(TransType == INVOICE || TransType == CRMEMO )
			err=CrtGlTrans(&gl99_hdr,&gl99_itm,amount,CREDIT);
		else
			err=CrtGlTrans(&gl99_hdr,&gl99_itm,amount,DEBIT);
		if (err < 0) return(err) ;
	}
	else {
		if(TransType == INVOICE || TransType == CRMEMO )
			err=CrtGlTrans(&gl99_hdr,&gl99_itm,amount,DEBIT);
		else
			err=CrtGlTrans(&gl99_hdr,&gl99_itm,amount,CREDIT);
		if (err < 0) return(err) ;
	}
}
/*-------------------------------------------------------------------------
Update Net Amount

	- Calculate and Display header Net amount  on screen
--------------------------------------------------------------------------*/

Update_Net_Amt()
{ 
	s_sth.s_net_amt = s_sth.s_gr_amt + s_sth.s_pst_tax 
					+ s_sth.s_gst_tax;
	WriteFields(NETAMT_FLD,NETAMT_FLD);
}

Update_Tax_Clearance(amount)
double amount;
{
int err;
	amount = D_Roundoff(amount);
	tax_clearance_acnt = amount;
	if((err=CheckGlAcnt(ctl_rec.fund,ctl_rec.sus_acnt,99,UPDATE))<0) 
		return(err);
	if(amount>0){
		if(Invoice_Crmemo_flag)
			err=CrtGlTrans(&gl99_hdr,&gl99_itm,amount,DEBIT);
		else
			err=CrtGlTrans(&gl99_hdr,&gl99_itm,amount,CREDIT);
		if (err < 0) return(err) ;
	}
	else{
		if(Invoice_Crmemo_flag)
			err=CrtGlTrans(&gl99_hdr,&gl99_itm,amount,DEBIT);
		else
			err=CrtGlTrans(&gl99_hdr,&gl99_itm,amount,CREDIT);
		if (err < 0) return(err) ;
	}
	s_sth.s_inv_total += amount ;
	s_sth.s_inv_total = D_Roundoff(s_sth.s_inv_total);

	return(NOERROR);
}
/*----------------------------------------------------------------*/
/* Set Duplication buffers for fields 				  */
SetDupBuffers( firstfld, lastfld, value )
int	firstfld, lastfld;	/* field numbers range */
int	value;			/* ENABLE or DISABLE */
{
	int i;

	for( i=firstfld; i<=lastfld; i+=100 )
		fomca1( i, 19, value);
	if( value==0 )
		return(0);
	sr.nextfld = firstfld;
	sr.endfld = lastfld;
	fomud( (char *)&s_sth );
	ret( err_chk(&sr) );
	return( 0 );
}



Update_Commitments(s_item)
S_item *s_item;
{
int err;
double temp,diff;

	/* Find if there is any differents in price */
	diff = s_item->s_amount -
	(s_item->s_qty * ((po_item.pi_value - po_item.pi_paid) /
				(po_item.pi_orig_qty - po_item.pi_pd_qty))) ;
	diff=D_Roundoff(diff);

	if(s_sth.s_type[0] != NON_BULK) 
		temp = po_item.pi_value - po_item.pi_paid;	
	else
		temp = po_item.pi_value;	
	gl_rec.comdat -= Get_Commit_Amt(po_item.pi_tax1,po_item.pi_tax2,temp);	
	gl_rec.comdat =  D_Roundoff(gl_rec.comdat); 
	temp = po_item.pi_value - po_item.pi_paid;	
	if(pa_rec.pa_poinv[0] == YES) {
		stck_rec.st_committed -= Get_Commit_Amt(po_item.pi_tax1,po_item.pi_tax2,temp);	
		if(stck_rec.st_committed <= DELTA_AMT)
			stck_rec.st_committed = 0.00;

		stck_rec.st_committed =  D_Roundoff(stck_rec.st_committed); 
	}

	/* uncommit which was previously committed */
	Get_Applied_Taxes(po_item.pi_tax1,po_item.pi_tax2, po_item.pi_value,
								&tax_cal);
	po_hdr.ph_comm -= tax_cal.gros_amt;
	po_hdr.ph_comm = D_Roundoff(po_hdr.ph_comm);  
					
	/* CL 1991/01/03 */
	Get_Applied_Taxes(s_item->s_gst,s_item->s_pst,s_item->s_amount,
								&tax_cal);
	po_hdr_liq_tot += tax_cal.gros_amt;
	po_hdr_liq_tot = D_Roundoff(po_hdr_liq_tot);  
	
	/* If The Value Changed */
	/* If price changed (diff !=0) or if tax applied has changed*/
	if((diff < -(DELTA_AMT)) || (diff > (DELTA_AMT)) ||
			(po_item.pi_tax1[0] != s_item->s_gst[0])  || 
	     		(po_item.pi_tax2[0] != s_item->s_pst[0])) {
		err = ValueChangeUpdates(s_item, diff) ;
		if(err < 0) return(err) ;
	}

	if (pa_rec.pa_poinv[0] == YES) {
		stck_rec.st_po_ordqty -= s_item->s_qty;
		if(stck_rec.st_po_ordqty < DELTA_QTY) 
			stck_rec.st_po_ordqty = 0.00;
		stck_rec.st_po_ordqty = D_Roundoff(stck_rec.st_po_ordqty); 
	}
	/* Now Update the PO Item with Invoice item details.
	   Increase the QTY paid for and increase Paid Value */
	po_item.pi_pd_qty  += s_item->s_qty ;
	po_item.pi_paid += s_item->s_amount ;
	po_item.pi_paid =  D_Roundoff(po_item.pi_paid);
	/* put gst/pst screen flags to file in case they have changed */
	po_item.pi_tax1[0] = s_item->s_gst[0];
	po_item.pi_tax2[0] = s_item->s_pst[0];

	/* Increase the Commitments when DIRECT or BULK.
	   For NON-BULK Inventory system updates */

	if(s_sth.s_type[0] != NON_BULK) 
		temp = po_item.pi_value - po_item.pi_paid;	
	else
		temp = po_item.pi_value;	
	gl_rec.comdat += Get_Commit_Amt(s_item->s_gst,s_item->s_pst,temp);	
	gl_rec.comdat = D_Roundoff(gl_rec.comdat);
	temp = po_item.pi_value - po_item.pi_paid;	
	if(pa_rec.pa_poinv[0] == YES) {
		stck_rec.st_committed += Get_Commit_Amt(po_item.pi_tax1,po_item.pi_tax2,temp);	
		if(stck_rec.st_committed <= DELTA_AMT)
			stck_rec.st_committed = 0.00;

		stck_rec.st_committed =  D_Roundoff(stck_rec.st_committed); 
	}
	Get_Applied_Taxes(s_item->s_gst,s_item->s_pst,po_item.pi_value,
								 &tax_cal);	
	po_hdr.ph_comm += tax_cal.gros_amt;
	po_hdr.ph_comm = D_Roundoff(po_hdr.ph_comm); 
			
	/* Is po to be completed */
	if(s_sth.s_complete == BOOL_YES){
		/* Uncommit all items including backordered items */
		temp = po_item.pi_value - po_item.pi_paid;	
	    	gl_rec.comdat -= Get_Commit_Amt(s_item->s_gst,s_item->s_pst,temp);
		gl_rec.comdat = D_Roundoff(gl_rec.comdat);
	}

	err = put_poitem(&po_item, UPDATE, e_mesg) ;
	if(err < 0) return(err) ;
}
/*---------------------------------------------------------------------------
	Update Stock Record for BULK and NON-BULK Items
----------------------------------------------------------------------------*/
Update_Stock(s_item)
S_item *s_item;
{
	double	qty;
	double	ftemp;
	double	ftemp_gst;

	/* Update Stock Record. Increase the "QTY Paid For" & "Value"
	   for INVOICE and CRMEMO, decrease for RETURN & DBMEMO.
	   Recalculate the avg. price */
/*
	if(TransType == INVOICE) { 
		if(s_sth.s_po_no == 0){
			stck_rec.st_on_order += s_item->s_qty ;
			stck_rec.st_on_order = D_Roundoff(stck_rec.st_on_order);
		}
	}
*/
	/* Calculate Freight amount for item */
	ftemp = D_Roundoff((remain_freight * s_item->s_qty)/remain_items);
	ftemp_gst = D_Roundoff((remain_f_gst * s_item->s_qty)/remain_items);

	if(Invoice_Crmemo_flag){
		/* Increase on-order in stock */
		if(s_sth.s_po_no == 0){
			stck_rec.st_on_order += s_item->s_qty ;
			stck_rec.st_on_order = D_Roundoff(stck_rec.st_on_order);
		}
		stck_rec.st_paidfor += s_item->s_qty ;
		stck_rec.st_paidfor = D_Roundoff(stck_rec.st_paidfor);
/******************************************************************************/
/* Change CB - May 14, 1992 The inventory should be the price without the 
   rebate
		Get_Applied_Taxes(s_item->s_gst,s_item->s_pst,s_item->s_amount,
								&tax_cal);	
		stck_rec.st_value += tax_cal.gros_amt;
************************/
		stck_rec.st_value += Get_Commit_Amt(s_item->s_gst,
					s_item->s_pst,s_item->s_amount);	
		/* J.Prescott and freight stuff */
		stck_rec.st_value += Get_Commitf_Amt(ftemp,ftemp_gst);
		stck_rec.st_value = D_Roundoff(stck_rec.st_value);
	}
	else {
		/* Decrease on-order in stock */
		if(s_sth.s_po_no == 0){
			stck_rec.st_on_order -= s_item->s_qty ;
			stck_rec.st_on_order = D_Roundoff(stck_rec.st_on_order);
		}
		stck_rec.st_paidfor -= s_item->s_qty ;
		stck_rec.st_paidfor = D_Roundoff(stck_rec.st_paidfor);
/******************************************************************************/
/* Change CB - May 14, 1992 The inventory should be the price without the 
   rebate
		Get_Applied_Taxes(s_item->s_gst,s_item->s_pst,
						s_item->s_amount,&tax_cal);	
		stck_rec.st_value -= tax_cal.gros_amt;
************************/
		stck_rec.st_value -= Get_Commit_Amt(s_item->s_gst,
					s_item->s_pst,s_item->s_amount);	
		/* J.Prescott and freight stuff */
		stck_rec.st_value -= Get_Commitf_Amt(ftemp,ftemp_gst);
		stck_rec.st_value = D_Roundoff(stck_rec.st_value);
	}

	if(pa_rec.pa_poinv[0] == YES) {
		qty = (stck_rec.st_on_hand + stck_rec.st_paidfor +
			stck_rec.st_po_ordqty) ;
		if(qty > DELTA_QTY) {
			stck_rec.st_rate  = 
			(stck_rec.st_value + stck_rec.st_committed) / qty;
		}
	}
	else {
		qty = (stck_rec.st_on_hand + stck_rec.st_paidfor);
		if(qty > DELTA_QTY) {
			stck_rec.st_rate  = stck_rec.st_value / qty;
		}
	}

	/******************************************************/
	/* Don't Round st_rate because it is an average price */
	/* stck_rec.st_rate  =  D_Roundoff(stck_rec.st_rate); */
	/******************************************************/
	return(put_stmast(&stck_rec,UPDATE,e_mesg));
}
/*------------------------------------------------------------------------
	Update allocation for Non Bulk po item which has been completed.
---------------------------------------------------------------------------*/
Update_po_alloc(s_item)
S_item *s_item;
{
int err;
double temp;

	/* Update allocated value and qty for item */
	err = CheckAllocation(s_item, UPDATE) ;
	if(err < 0 && err != UNDEF) return(err) ;
	/* remove original item amount allocated */
/******************************************************************************/
/* Change CB - May 14, 1992 The inventory should be the price without the 
   rebate
  	Get_Applied_Taxes(s_item->s_gst,s_item->s_pst,po_item.pi_value,&tax_cal);
  	aloc_rec.st_value -= tax_cal.gros_amt;		
************************/
	aloc_rec.st_value -= Get_Commit_Amt(s_item->s_gst,
				s_item->s_pst,po_item.pi_value);	
   	aloc_rec.st_value=D_Roundoff(aloc_rec.st_value);
	/* Add back remaining allocated(Only what is being paid for) */
/******************************************************************************/
/* Change CB - May 14, 1992 The inventory should be the price without the 
   rebate
	Get_Applied_Taxes(s_item->s_gst,s_item->s_pst,po_item.pi_paid,&tax_cal);
   	aloc_rec.st_value += tax_cal.gros_amt; 
************************/
	aloc_rec.st_value += Get_Commit_Amt(s_item->s_gst,
				s_item->s_pst,po_item.pi_paid);	
   	aloc_rec.st_value=D_Roundoff(aloc_rec.st_value);
	/* decrease qty by amount of po item */
/*  Cathy Burns.  Should not be decreasing allocation by quantity being paid 
    for, it should only be decreasing allocated quantity when a backorder is
    released.
	aloc_rec.st_alloc -= s_item->s_qty;
	aloc_rec.st_alloc=D_Roundoff(aloc_rec.st_alloc);
*/
	/* Remove back order quantities from the allocations */
	aloc_rec.st_alloc -= po_item.pi_orig_qty - po_item.pi_pd_qty ;
	aloc_rec.st_alloc=D_Roundoff(aloc_rec.st_alloc);
	err = put_alloc(&aloc_rec, UPDATE, e_mesg) ;
	return(err) ;
}

Check_Pst_Gst()
{
int i,pst_flag,gst_flag;
Page *page;

	if(s_sth.s_pst_tax < DELTA_AMT) pst_flag = TRUE;
	else pst_flag = FALSE;
	if(s_sth.s_gst_tax < DELTA_AMT) gst_flag = TRUE;
	else gst_flag = FALSE;

	
	/* Process each item and update files */
	for(page=FirstPage;(page != NULL) && !(pst_flag & gst_flag);
							 page=page->NextPage){
		/* Take Each Invoice Item and Update the files */
		for(i = 0 ;(i < page->NoItems) && !(pst_flag & gst_flag); i++) {
		    /* Is Item amount < 0 or Not Active */
		    if(page->Items[i].s_amount < DELTA_AMT ||
			    strcmp(page->Items[i].s_status,ACTIVE) != 0){
			continue ;
		    } 
		    if(!pst_flag)
		    	if(page->Items[i].s_pst[0] == TAXABLE) 
				pst_flag = TRUE;
		    if(!gst_flag)	
		    	if(page->Items[i].s_gst[0] == TAXABLE) 
				gst_flag = TRUE;

			
		}	/* for( ; i < NoItems ; ) */
		/* If Current Page is last page in Invoice, then break */
		if(page == InvcLast) break;
	}	/* for( ; page != NULL ; ) */

	if(!pst_flag){
#ifdef ENGLISH
		DispError("Item PST is not consistent with Header");
#else 
		DispError("TVP pour items pas consistante avec info d'entete");
#endif 
		return(ERROR);
	}
	if(!gst_flag){
#ifdef ENGLISH
		DispError("Item GST is not consistent with Header");
#else 
		DispError("TPS pour items pas consistante avec info d'entete");
#endif 
		return(ERROR);
	}
	return(NOERROR);
}

static  Get_Posting_Method()
{
double diff;

	s_sth.s_gst_tax = D_Roundoff(s_sth.s_gst_tax);
	s_sth.s_pst_tax = D_Roundoff(s_sth.s_pst_tax);

	if(s_sth.s_gst_pf[0] != FULL_AMT)
		if(s_sth.s_pst_pf[0] != FULL_AMT)
			/* supplier collects GST and PST */
			return(Gl_1);
		else
			/* supplier collects GST ONLY */
			return(Gl_3);
	else
	if(s_sth.s_pst_pf[0] != FULL_AMT)
		/* supplier collects PST ONLY */
		return(Gl_4);
	else
		/* supplier does not collect any taxes*/
	        return(Gl_2);


}

Convert_hdr_field(field)
int field;
{
int i;
	switch(field){

	case	PO:		i = PONO_FLD;
				break;
	case	PAYEE:		i = PAYEE_FLD;
				break;
	case	FUND:		i = FUND_FLD;
				break;
	case	INVC_DT:	i = INVCDT_FLD;
				break;
	case	DUE_DT:		i = DUEDT_FLD;
				break;
	case	TYPE:		i = TYPE_FLD;
				break;
	case	GROSS_AMT:	i = GRAMT_FLD;
				break;
	case	GST_TAX:	i = GST_PF;
				break;
	case	PST_TAX:	i = PST_PF;
				break;
	case	DISC_AMT:	i = DISC_PF;
				break;
	case	POCOMPLETE:	i = PO_COMP_FLD;
				break;
	case	REMARKS:	i = REMARKS_FLD;
				break;
	case	CHEQUE_NO:	i = CHQNO_FLD;
				break;
	case	BANK_ACCT:	i = BANKACCT_FLD;
				break;
	case	MISC_EXP:	i = MISC_GST_FLD;
				break;
	case	MISC_REB:	i = MISC_REB_FLD;
				break;
	case	MISC_PST:	i = MISC_PST_FLD;
				break;
	case 	MISC_AMT:	i = MISC_E_FLD;
				break;
	default:		i = MISC_GST_FLD;

	}
	return(i);
}

/*------------------- E n d   O f   P r o g r a m ---------------------*/

