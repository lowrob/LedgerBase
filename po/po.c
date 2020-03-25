/*-----------------------------------------------------------------------
Source Name: po.c
System     : Purchase Order.
Created  On: 10th September 89.
Created  By: CATHY BURNS.

DESCRIPTION:
	Program to enter Purchase Orders.

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
J.PRESCOTT     90/05/15       Changed screen to contain description,
			      Bill To, tax1,tax2, Freight, contact person
p ralph	       90/11/20       Add pa_rec.pa_due_days to entry date to get
			      due date.
F Tao	       90/11/21       Add Header Edit Option
J.Prescott     90/11/22       Add Validatation for Inventory control account
			      When adding Bulk or Non-Bulk PO's
C.Leadbeater   90/11/27	      Changed to allow zero amounts for no charge
			      items and not to allow zero quantities. 
p ralph        90/12/03       Implemented gst. Functions Tax_Calculation 
			      & Commit_Calculation. 
C.Leadbeater   90/12/12	      Make Gst and Pst fields default to E and T 
			      when when adding a new PO item.	
C.Leadbeater   90/12/13	      Modified so that net amount is calculated from
			      (price * quantity) in change mode.
C.Leadbeater   90/12/18	      Added D_Roundoff().
C.Burns        91/01/01       Fixed locking problem.
F.Tao	       91/01/03	      Fixed Edit problem so that when editing a item
			      the cursor will be back to the CHANGE field 
			      position after end of that item.
C.Leadbeater   91/01/21	      Added two lines to set 'gst_tax' and 'pst_tax'
			      values to correspond with the current poitem
			      tax flags before Tax_Calculation().
C.Leadbeater   91/01/28	      Modified so that the stock code can be changed
			      for a po item, and allocation and stock files
			      are updated.
C.Leadbeater   91/01/29	      Changed NET AMT validation so that the budget
			      amount is checked with net amt + taxes.
J.Prescott     91/02/08       Changed Screen Design
C.Burns        91/06/11       Changed program to interface with Inventory System
C.Burns        91/07/11       When adding edit will take user to field#.
C.Burns	       91/12/18	      When editing a line item and Esc 'f' entered, it
			      reveats the line back to the information that
			      was there before editing the line.
------------------------------------------------------------------------*/
#define	MAIN		/* Main program. This is to declare Switches */
#define MAINFL		POHDR			/* main file used */

#define	SYSTEM		"PURCHASE ORDER"	/* Sub System Name */
#define	MOD_DATE	"02-FEB-91"		/* Progran Last Modified */

#include <stdio.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	SUPPMAINT   "supplier"	/* Supplier Maintenance executable file's
				   base name */

#define	DELTA_AMT  0.005	/* To Check double values for zero */
#define	DELTA_QTY  0.00005	/* To Check double values for zero */

#define TAXABLE		'T'
#define EXEMPT		'E'
#define FREIGHT		'F'


/* User Interface define constants */
#ifdef ENGLISH
#define ADDREC		'A'
#define CHANGE		'C'
#define INQUIRE		'I'
#define EXITOPT		'E'
#define HEADEREDIT	'H'
#define TOTAL		'T'
#define	NEXT		'N'
#define	PREV		'P'

#define	YES		'Y'
#define NO		'N'
#define	ADDITEMS	'A'
#define	DELITEM		'D'
#define	REVIVEITEM	'R'
#define	EDIT		'E'
#define	NEXTPAGE	'N'
#define	PREVPAGE	'P'
#define	CANCEL		'C'

#define	ENGLISH_MSG	'E'
#define FRENCH_MSG	'F'
#else
#define ADDREC		'R'
#define CHANGE		'C'
#define INQUIRE		'I'
#define EXITOPT		'F'
#define HEADEREDIT	'E'
#define TOTAL		'T'
#define	NEXT		'S'
#define	PREV		'P'

#define	YES		'O'
#define NO		'N'
#define	ADDITEMS	'R'
#define	DELITEM		'E'
#define	REVIVEITEM	'V'
#define	EDIT		'M'
#define	NEXTPAGE	'S'
#define	PREVPAGE	'P'
#define	CANCEL		'A'

#define	ENGLISH_MSG	'A'
#define FRENCH_MSG	'F'
#endif

/* PROFOM Releted declarations */

#define	SCR_NAME	"po"	/* PROFOM screen Name */

#define	PAGESIZE	2		/* No of Items */

/* PROFOM Screen STH file */

/* Field PROFOM numbers */

#define	HDR_ST_FLD	800	/* Header Start Field */
#define	HDR_END_FLD	2500	/* Header End Field */
#define	ITEM_ST_FLD	2600	/* Item 1 Start Field */
#define	END_FLD		9700	/* Last Field of the screen */
#define	STEP		1500	/* NO of fields diff. between 2 items */

#define TOTAL_ST_FLD	5600	/* Totals Start Field */
#define TOTAL_END_FLD	5800	/* Totals End Field */

/* HDR Fields */
#define	FN_FLD		400	/* Fn: */
#define	KEY_START	500	/* PO Number */
#define	KEY_END		500	/* PO Number */
#define	CHG_FLD		600	/* Field: */
#define	SUPPCD_FLD	800	/* Supplier Cd: */
#define	PAYEE_FLD	900	/* Payee: */
#define POAMT_FLD	1000 	/* PO Amount: */
#define	ENTRYDT_FLD	1100	/* Entry Dt: */
#define DUEDT_FLD	1200	/* Due Date */
#define	PERIOD_FLD	1400	/* Period: */
#define	FUND_FLD	1500	/* Fund: */
#define SHIPTO_FLD	1700	/* Ship To: */
#define	TYPE_FLD	1800	/* Type: */
#define REQ_FLD		2000	/* Req No: */
#define GSTMSG_FLD	2100	/* GST Msg:*/
#define ATT_FLD		2300    /* Attention: */
#define	PAGENO_FLD	2400	/* Page#: */
#define COLHDG_FLD	2500
	
/* Item Fields. These numbers are difference between that field
   and the 1st fld within the item */
#define	ACNO_FLD	100
#define	STCKCD_FLD	200
#define UOM_FLD		300
#define	SCHOOL_FLD	400
#define DESC_FLD	500
#define GST_FLD		700
#define	PST_FLD		800
#define	QTY_FLD		900
#define	PRICE_FLD	1000
#define	NET_FLD		1100
#define GROSS_FLD	1200
#define OSQTY_FLD	1400

/* po.sth - header for C structure generated by PROFOM EDITOR */

typedef	struct	{	/* Start Fld 2600, Endfld 5700 and Step 1500 */

	short	s_sno;		/* NUMERIC 999 field 2600 */
	char	s_accno[19];	/* STRING XXXXXXXXXXXXXXXXXX Field 2700 */
	char	s_stck_cd[11];	/* STRING XXXXXXXXXX Field 2800 */
	char	s_uom[7];	/* STRING XXXXXX Field 2900 */
	short	s_school;	/* NUMERIC 99 Field 3000 */
	char	s_desc[61];	/* STRING X(60) Field 3100 */
	char	s_act_del[4];	/* STRING XXX Field 3150 */
	char	s_subhdg[2];	/* STRING X Field 3200 */
	char	s_gst[2];	/* STRING XX Field 3300 */
	char	s_pst[2];	/* STRING XX Field 3400 */
	double	s_ord_qty;	/* NUMERIC 99999.9999 Field 3500 */
	double	s_unit_cost;	/* NUMERIC 99999.99 Field 3600 */
	double	s_net_amt;	/* NUMERIC 9999999.99 Field 3900 */
	double	s_value;	/* NUMERIC 9999999.99 Field 4000 */
	double	s_pd_amount;	/* NUMERIC 9999999.99 Field 4100 */
	double	s_os_qty;	/* NUMERIC 99999.9999 Field 4200 */
}	S_item ;

typedef struct	{

	char	s_pgm[11];	/* STRING XXXXXXXXXX Field 100 */
	char	s_scrname[15]; 	/* STRING X(14) Field 200 */
	long	s_rundate;	/* DATE YYYYFMMFDD Field 300 */
	char	s_fn[2];	/* STRING X Field 400 */
	long	s_po_no;	/* NUMERIC 99999999 Field 500 */
	short	s_field;	/* NUMERIC 999 Field 600 */

	char	s_supp_cd[11];	/* STRING XXXXXXXXXX Field 800 */
	char	s_payee_cd[11];	/* STRING XXXXXXXXXX Field 900 */
	double  s_po_amt;	/* NUMERIC 99999999.99 Field 1000 */
	long	s_entry_dt;	/* STRING XXXXXXXX Field 1100 */
	long	s_due_dt;	/* STRING XXXXXXXX Field 1200 */
	double  s_liq_amt;	/* NUMERIC 99999999.99 Field 1300 */
	short	s_period;	/* NUMERIC 99 Field 1400 */
	short	s_fund;		/* NUMERIC 999 Field 1500 */
	long	s_liq_dt;	/* STRING XXXXXXXX Field 1600 */
	short	s_shipto;	/* NUMERIC 99 Field 1700 */
	char	s_type[2];	/* STRING X Field 1800 */
	short	s_billto;	/* NUMERIC 99 Field 1900 */
	char	s_req_no[11];	/* STRING XXXXXXXXXX Field 2000 */
	char	s_gstmsg[2];	/* STRING X Field 2100 */
	char	s_status[2];	/* STRING X Field 2200 */
	char	s_attention[16];/* STRING X(15) Field 2300 */
	short	s_page_no;	/* NUMERIC 99 Field 2400 */
	char	s_dummy1[3];	/* STRING X Field 2500 */

	S_item	s_items[PAGESIZE] ;	/* Start Fld 2600, End Fld 5500 and
					   Step 1500 */
	double	s_total_po;	/* NUMERIC 9999999.99 Field 5600 */
	double	s_total_itm;	/* NUMERIC 9999999.99 Field 5700 */
	double	s_total_diff;	/* NUMERIC 99999999.99 Field 5800 */

	char	s_mesg[78];	/* STRING X[77] Field 9600 */
	char	s_resp[2];	/* STRING X Field 9700 */
} s_struct;


static  short gst_tax;
static  short pst_tax;
static  Tax_cal tax_cal;
static  double  net_amt;
double	D_Roundoff();

static	s_struct  s_sth,image;	/* PROFOM Screen Structure */
static	struct  stat_rec  sr;		/* PROFOM status rec */

static	Po_hdr		po_hdr ;	/* Purchase Order Header */
static  Po_item 	po_item ;	/* Purchase Order Item Record */
static	Po_hdr		pre_po ;	/* Purchase Order Header */
static  Po_item 	pre_item ;	/* Purchase Order Item Record */
static	Supplier	supplier;	/* Supplier */
static	Pa_rec		pa_rec ;	/* Parameters Record */
static	Ctl_rec		ctl_rec ;	/* Fund/Control Record */
static	Last_po		lastpo_rec;	/* Last P0 Number Record */
static	Gl_rec		gl_rec ;	/* Gl Master rec, for general purpose */
static	St_mast		stck_rec ;	/* Stock Master Rec */
static	Alloc_rec	aloc_rec ;	/* Stock Allocation Record */
static	Sch_rec		schl_rec ;	/* School Record */

static	char 	e_mesg[100];  		/* dbh will return err msg in this */

/*
*	Doubly linked list to maintain po items. Each node in this list
*	conatins one page full of po items. This list is freed only at the
*	time of exiting program. 
*/

typedef	struct Page {

	S_item	Items[PAGESIZE] ;	/* Items information */
	short	PoItemNo[PAGESIZE] ;	/* Corresponding PO Item# */
	char	PoStatus[PAGESIZE] ;	/* Status Add, Change */
	struct	Page	*PrevPage ;	/* Ptr to Previous Page */
	struct	Page	*NextPage ;	/* Ptr to Next Page */
	short	NoItems ;		/* No of Items in this page */
	short	Pageno ;		/* Page Number */

}	Page ;

static	Page	*FirstPage,		/* Address of the First Page */
		*CurPage,		/* Address of the Active Page */
		*PoLast,		/* Address of Cur. PO Last page */
		*LastPage ;		/* Address of the Last page Memory
					   allocated */
long	Entry_date;			/* Last Entry date used */
char	Prev_acct[19];

double  Commit_Calculation();

int	KeyAndHdrValidation(), ItemsValidation() ;
int	HdrAndKeyWindowHelp(), ItemsWindowHelp() ;
int	Argc;
char	**Argv;

void	free() ;
char	*malloc() ;

main(argc,argv)
int argc;
char *argv[];
{
	int 	retval;

	/* These two are passed to execute() when it is called */
	Argc = argc;
	Argv = argv;

	retval = Initialize(argc,argv);	/* Initialization routine */

	if (retval == NOERROR) retval = PoProcess();

	CloseRtn();			/* return to menu */
	if (retval != NOERROR) exit(-1);
	exit(0);
}

/*-------------------------------------------------------------------*/
/* Initialize PROFOM */

Initialize(argc, argv)
int	argc ;
char	*argv[] ;
{
	int	err ;

	/*
	*	Initialize DBH Environment
	*/
	strncpy(SYS_NAME,SYSTEM,50);	/* Sub system name */
	strncpy(CHNG_DATE,MOD_DATE,10);	/* Modification Date */

	proc_switch(argc, argv, MAINFL) ; 	/* Process Switches */

	/*
	*	Initialize PROFOM & Screen
	*/
	STRCPY(sr.termnm,terminal);	/* Copy Terminal Name */
	fomin(&sr);
	ret(err_chk(&sr)) ;		/* Check for PROFOM Error */
	fomcf(1,1);			/* Enable Snap screen option */

	err = InitScreen() ;		/* Initialize Screen */
	if(NOERROR != err) return(err) ;

/* Set up colors */
	fomsc(0,2,7,2);
	fomca1(100,17,6);
	fomca1(200,17,6);
	fomca1(300,17,6);

	/*
	*	Get The Parameter Record
	*/
	err = get_param(&pa_rec, BROWSE, 1, e_mesg) ;
	if(err == ERROR) {
		DispError(e_mesg);
		return(ERROR) ;
	}
	else if(err == UNDEF) {
#ifdef ENGLISH
		DispError("Parameters Are Not Set Up ...");
#else
		DispError("Parametres ne sont pas etablis... ");
#endif
		return(ERROR) ;
	}

	/* Initialize first Entry date to system date */
	Entry_date = get_date();

	/*
	*	Initialize Variables
	*/

	FirstPage = NULL ;
	LastPage  = NULL ;

	return(NOERROR) ;
}	/* Initialize() */

/*--------------------------------------------------------------------------*/
/* Close nessary files and environment before exiting program               */

CloseRtn() 
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

	/* Set terminal back to normal mode from PROFOM */
	fomcs();
	fomrt();

	close_dbh();	/* Close files */

	return(NOERROR);
}	/* CloseRtn() */
/*----------------------------------------------------------------*/
/* Initialize screen before going to process options */

InitScreen()
{
	int	err ;

	/* move screen name to Profom status structure */
	STRCPY(sr.scrnam,NFM_PATH);
	strcat(sr.scrnam,SCR_NAME) ;

	STRCPY(s_sth.s_pgm,PROG_NAME);
#ifdef ENGLISH
	STRCPY(s_sth.s_scrname, "PO MAINTENANCE");
#else
	STRCPY(s_sth.s_scrname, " ENTRETIEN BC");
#endif
	s_sth.s_rundate = get_date();	/* get Today's Date in YYYYMMDD format*/
	s_sth.s_field = HV_SHORT ;
	s_sth.s_mesg[0] = HV_CHAR ;
	s_sth.s_resp[0] = HV_CHAR ;

	/* Initialize Key Fields. So that, if user selectes 'N' option
	   immediately after invoking program, then he gets the first
	   record in the file */

	s_sth.s_po_no = 0 ;

	/* Move High Values to data fields and Display the screen */
	err = ClearScreen() ;
	if(NOERROR != err) return(err) ;

	return(NOERROR) ;
}	/* InitScreen() */

/*-------------------------------------------------------------------*/
/* Get Option from user and call corresponding function */

PoProcess()
{
	int err;

	for( ; ; ){

		/* Get the Fn: option from the user */
		if((err = ReadFunction()) != NOERROR) return(err) ;

		err = ProcFunction() ;	/* Process Function */

		if(QUIT == err)		return(NOERROR) ;    /* Exit */
		if(NOACCESS == err)	fomen(e_mesg);	     /* security */
		if(PROFOM_ERR == err)	return(PROFOM_ERR);  /* PROFOM ERROR */
		if(DBH_ERR == err) {
			DispError(e_mesg);
#ifdef ENGLISH
			sprintf(e_mesg,"%s %d Dberror: %d Errno: %d",
				"System Error... Iserror:",
				iserror, dberror, errno);
#else
			sprintf(e_mesg,"%s %d Dberror: %d Errno: %d",
				"Erreur du systeme... Iserror:",
				iserror, dberror, errno);
#endif
			DispError(e_mesg);
			return(DBH_ERR); /* DBH ERROR */
		}
	}      /*   end of the for( ; ; )       */
}	/* PoProcess() */
/*----------------------------------------------------------------*/
/* Display the Function (Fn:) options and get the option from the user */

ReadFunction()
{
	/* Display Fn: options */
#ifdef ENGLISH
	fomer("A(dd), C(hange), I(nquire), N(ext), P(rev), E(xit)");
#else
	fomer("R(ajouter), C(hanger), I(nterroger), S(uivant), P(recedent), F(in)");
#endif
	/* Read Fn: field to get the option */
	sr.nextfld = FN_FLD ;
	fomrf( (char *)&s_sth );
	ret(err_chk(&sr));	/* Check for PROFOM error */

	return(NOERROR) ;
}	/* ReadFunction() */
/*----------------------------------------------------------------*/
/* Process the user selected Fn: option */

ProcFunction()
{
	int retval;

	switch (s_sth.s_fn[0]) {
	case ADDREC  :			/* ADD */
		CHKACC(retval,ADD,e_mesg);
		return( AddPo() ) ;
	case CHANGE  :			/* CHANGE */
		CHKACC(retval,UPDATE,e_mesg);
		return( ChangePo() ) ;
	case INQUIRE  :			/* Inquire Po */
		CHKACC(retval,BROWSE,e_mesg);
		return( InquirePo() ) ;
	case NEXT  :			/* Next Po */
		CHKACC(retval,BROWSE,e_mesg);
		return( NextPo(FORWARD) ) ;
	case PREV  :			/* Previous Po */
		CHKACC(retval,BROWSE,e_mesg);
		return( NextPo(BACKWARD) ) ;
	case EXITOPT  :			/* Exit */
		return( QUIT ) ;
	}  /*   end of the switch statement */

	return(retval);
}	/* ProcFunction() */
/*----------------------------------------------------------------------*/
/* Adding Po. Get the unique Key, accept details and update the files */

AddPo()
{
	int	err ;

	err = GetUniqueKey();
	if(err != NOERROR) return(err) ;

	CurPage = NULL ;
	PoLast  = NULL ;

	/* Clear The Screen */
	err = ClearScreen();
	if(err != NOERROR) return(err) ;

	err = GetDetails() ;
	if(PROFOM_ERR == err || DBH_ERR == err) return(err) ;
	if(err < 0 || CANCEL == err) {
		roll_back(e_mesg) ;	/* Unlock the locked Records */
		return(ClearScreen()) ;	/* Clear the Screen */
	}

	return(NOERROR);
}	/* AddPo() */
/*-----------------------------------------------------------------------*/
/* Change the Po */

ChangePo()
{
	int	err ;

	err = SelectPo(UPDATE) ;
	if(NOERROR != err) return(err) ;

	if(po_hdr.ph_status[0] == COMPLETE) {
#ifdef ENGLISH
		DispError("PO is completed.. Changes not allowed");
#else
		DispError("BC est complete.. Changements ne sont pas permis");
#endif
		roll_back(e_mesg);
		return(NOERROR) ;
	}
	if(po_hdr.ph_lqamt > DELTA_QTY) {
#ifdef ENGLISH
		DispError("PO is Partially Paid.. Changes not allowed");
#else
		DispError("BC est partiellement paye.. Changements ne sont pas permis");
#endif
		roll_back(e_mesg);
		return(NOERROR) ;
	}
	for( ; ; ) {
		err = CheckFund(s_sth.s_fund) ;
		if(err < 0) return(err) ;

		err = ConfirmItems() ;
		if(err != YES) {
			roll_back(e_mesg);
			return(err) ;
		}

		err = WriteRecords(UPDATE) ;
		if(err==NOERROR) break;
		if(err==LOCKED) {
			roll_back(e_mesg) ;
			continue;
		}
		if (err < 0) {
			roll_back(e_mesg);
			return(err) ;
		}
	}


	return(NOERROR) ;
}	/* ChangePo() */
/*-----------------------------------------------------------------------*/
/* Show the user given keys Po */

InquirePo()
{
	int	err ;

	err = SelectPo(BROWSE) ;
	if(NOERROR != err) return(err) ;

	err = ConfirmItems() ;
	if(err < 0) return(err) ;

	return(NOERROR) ;
}	/* InquirePo() */
/*----------------------------------------------------------*/
/* Show the next or previous Po */

NextPo(direction)
int	direction ;
{
	int err;

	/* Check whether file is in seq. read mode */
	if(flg_start(POHDR) != direction) {
		po_hdr.ph_code = s_sth.s_po_no ;
		if (direction == FORWARD)
			po_hdr.ph_code++;
		else	po_hdr.ph_code--;
		flg_reset(POHDR);
	}

	err = get_n_pohdr(&po_hdr, BROWSE, 0, direction, e_mesg);
#ifndef ORACLE
	seq_over(POHDR);
#endif
	if(ERROR == err)return(DBH_ERR) ;
	if(EFL == err) {
#ifdef	ENGLISH
		fomen("No More Records....");
#else
		fomen("Plus de fiches....");
#endif
		get();
		flg_reset(POHDR);
		return(NOERROR) ;
	}

	return( ShowPo() ) ;
}	/* NextPo() */
/*----------------------------------------------------------------------*/
/* Read the Po Key. Check whether the po exists with this key */

GetUniqueKey()
{
	int retval,i;

	retval = CheckFund((short)1) ;
	if(retval < 0) return(retval) ;
	po_hdr.ph_code = lastpo_rec.last_po + 1 ;
	s_sth.s_po_no = po_hdr.ph_code;	
	SetDupBuffers(KEY_START,KEY_END,2);	/* set dup buffer on */
	s_sth.s_po_no = LV_LONG;

	i = ReadFields(KEY_START, KEY_END,
		KeyAndHdrValidation, HdrAndKeyWindowHelp,0) ;
	if(PROFOM_ERR == i || DBH_ERR == i) return(i) ;
	if(RET_USER_ESC == i){
		/* copy back key fields */
		s_sth.s_po_no = po_hdr.ph_code ;

		ret( WriteFields(KEY_START, KEY_END) ) ;

		s_sth.s_mesg[0] = HV_CHAR;
		ShowMesg();

		return(RET_USER_ESC) ;
	}

	if(WriteFields(KEY_START,KEY_END)<0) return(-1);

	return(NOERROR) ;
}	/* GetUniqueKey() */
/*------------------------------------------------------------*/
/* Read the Po Details from the User */

GetDetails()
{
	int	i ;

	i = ReadHdr() ;
	if(NOERROR != i) return(i) ;

	i = ConfirmHdr() ;
	if(i != YES) return(i);

	/* Initialize totals to zero */
	s_sth.s_total_po = 0.00;
	s_sth.s_total_itm = 0.00;
	s_sth.s_total_diff = 0.00;

	i = AddItems();
	if(NOERROR != i) return(i) ;

	for( ; ; ) {
		i = ConfirmItems() ;
		if(i != YES) break;

		i = WriteRecords(ADD) ;
		if(i < 0) {
			if(i == LOCKED) continue;
			if(i == DUPE) {
				s_sth.s_po_no = po_hdr.ph_code++;
				/* redisplay new po no */
				if((i = PoHdrtoScrHdr()) < 0) 
					return(i) ;	
				continue;
			}
			break;
		}
		break;
	}
	if(i != NOERROR) return(i);
	return(NOERROR) ;
}	/* GetDetails() */
/*----------------------------------------------------------*/
/* Get the Po key and show the po */

SelectPo(md)
int	md ;	/* UPDATE or BROWSE. UPDATE locks the po */
{
	int	err ;

	for(; ;){
		err = ReadKey();
		if(err != NOERROR) return(err) ;

		/* Get the record from database */
		err = GetPoHdr(md);
		if(ERROR == err) return(DBH_ERR) ;
		if(err < 0) {		/* UNDEF or LOCKED */
			fomer(e_mesg) ;
			get() ;
			continue ;
		}

		return( ShowPo() ) ;
	}
}	/* SelectPo() */
/*----------------------------------------------------------------------*/
/* Get the po key from user. In ADD mode disable dup buffers, other
   modes enable dup buffers and show the current key as a default key */

ReadKey()
{
	int	i;
	long	po_no;

	/* In Add mode turn off dup control for key fields.
	   Other modes reverse it */
	if(s_sth.s_fn[0] == ADDREC){	/* ADD */
		SetDupBuffers(KEY_START,KEY_END,0); /* Off Dup Control */
	}
	else {
		SetDupBuffers(KEY_START,KEY_END,2);
	}

	/* Store fields to copy back when user gives ESC-F */
	po_no = s_sth.s_po_no ;

	s_sth.s_po_no = LV_LONG ;

#ifdef ENGLISH
	STRCPY(s_sth.s_mesg,"Press ESC-F to Go Back to Fn:");
#else
	STRCPY(s_sth.s_mesg,"Appuyer sur ESC-F pour retourner a Fn:");
#endif
	ShowMesg();

	i = ReadFields(KEY_START, KEY_END,
		KeyAndHdrValidation, HdrAndKeyWindowHelp,0) ;
	if(PROFOM_ERR == i || DBH_ERR == i) return(i) ;
	if(RET_USER_ESC == i){
		/* copy back key fields */
		s_sth.s_po_no = po_no ;

		ret( WriteFields(KEY_START, KEY_END) ) ;

		s_sth.s_mesg[0] = HV_CHAR;
		ShowMesg();

		return(RET_USER_ESC) ;
	}

	return(NOERROR);
}	/*  ReadKey() */
/*-----------------------------------------------------------------------*/ 
/* Update the records and write to files */

WriteRecords(mode)
int mode;
{
	int	err;
	int	i ;
	Page	*temppage ;
	int	retval;

	/* Get Inventory General GL account, if not a direct disbursement */
	if(s_sth.s_type[0] == BULK) {
		err = CheckGlAcnt(ctl_rec.inv_acnt) ;
		if(err < 0) return(err) ;
	}
	STRCPY(supplier.s_supp_cd, s_sth.s_supp_cd);
	err = get_supplier(&supplier, UPDATE, 0, e_mesg);
	if(err < 0) {
		DispError(e_mesg);
		return(err) ;
	}
	if(mode == ADD && s_sth.s_po_no < 89999999) {  /* automatic no. po */
		ctl_rec.fund = 1 ;
		err = get_ctl(&ctl_rec,BROWSE, 0, e_mesg) ;
		if(err < 0) {
			DispError(e_mesg);
			return(err) ;
		}

		err = get_lastpo(&lastpo_rec,UPDATE, 1, e_mesg) ;
		if(err < 0) {
			DispError(e_mesg);
			return(err) ;
		}
		
	}
	else {
		ctl_rec.fund = 1 ;
		err = get_ctl(&ctl_rec,BROWSE, 0, e_mesg) ;
		if(err < 0) {
			DispError(e_mesg);
			return(err) ;
		}
	}

	if(mode == UPDATE) {
		po_hdr.ph_code = s_sth.s_po_no ;
		err = get_pohdr(&po_hdr, mode, 0, e_mesg);
		if(err < 0) {
			DispError(e_mesg) ;
			return(err) ;
		}
     		scpy((char*)&pre_po,(char*)&po_hdr,sizeof(po_hdr));
	}
	if (s_sth.s_fn[0] == CHANGE) {
		if (strcmp(s_sth.s_supp_cd, po_hdr.ph_supp_cd) != 0)
			err = SupplierChange() ;
	}

	ScrHdrtoPoHdr(mode) ;		/* Copy Po Header */

	if (s_sth.s_fn[0] == ADD ){
		retval = get_pohdr(&po_hdr, UPDATE,0, e_mesg);
 		if( retval==NOERROR ){
			s_sth.s_po_no ++  ;
			po_hdr.ph_code = s_sth.s_po_no ;
 			if( WriteFields(KEY_START,KEY_END)<0 )
 				return(-1);
 		}
	}

	err = put_pohdr(&po_hdr, mode,e_mesg) ;
	if(err < 0) {
		DispError(e_mesg) ;
		return(err) ;
	}

	if(mode != ADD) {
		err = rite_audit((char*)&s_sth, POHDR,mode,(char*)&po_hdr,
					(char*)&pre_po,e_mesg);
		if(err==LOCKED) {
			DispError(e_mesg);
			roll_back(e_mesg) ;
			return(LOCKED) ;
		}
		if(err != NOERROR ){
#ifdef	ENGLISH
			DispError("ERROR in Saving Records"); 
#else
			DispError("ERREUR en conservant les fiches");
#endif
			DispError(e_mesg);
			roll_back(e_mesg);
			return(err);
		}
	}

	
	/* Copy Items and write to the file */
	if(PoLast != NULL)
	    for(temppage = FirstPage ; temppage != NULL ;
						temppage = temppage->NextPage) {
		for(i = 0; i < temppage->NoItems ; i++) {
			if (temppage->PoStatus[i] == ' ') continue;
			err = ProcItemUpdates(temppage, i) ;
			if(err==LOCKED) {
				DispError(e_mesg);
				roll_back(e_mesg) ;
				return(LOCKED) ;
			}
			if(err != NOERROR) break ;
		}
		/* IF the process is done upto Current Po Last Page,
		   then break */
		if(temppage == PoLast) break ;
	    }

	if( s_sth.s_type[0] == BULK) { 
		
			/* round gl_rec.comdat after calculation */
		gl_rec.comdat = D_Roundoff(gl_rec.comdat);
		err = put_gl(&gl_rec,UPDATE,e_mesg);
		if (err < 0) return(err);
	}
		/* round ytd ord before writing to file */
	supplier.s_ytd_ord = D_Roundoff(supplier.s_ytd_ord); 

	err = put_supplier(&supplier, UPDATE, e_mesg) ;
	if (err < 0) return(err) ;

	if(mode == ADD && s_sth.s_po_no < 89999999) {
		lastpo_rec.last_po = s_sth.s_po_no ;
		err = put_lastpo(&lastpo_rec, UPDATE, 1, e_mesg) ;
	}

	if(err == NOERROR)
		err = commit(e_mesg) ;
	if(err < 0) {
#ifdef ENGLISH
		DispError("ERROR in Saving Records"); 
#else
		DispError("ERREUR en conservant les fiches");
#endif
		DispError(e_mesg);
		return(err);
	}

	return(NOERROR);
}	/* WriteRecords() */
/*-----------------------------------------------------------------------*/ 
/* Check whether total of items amount is equal to Po total */

CheckTotal()
{
	double	total, diff ;
	Page	*temppage ;
	int	i, items ;

	/* Total up all items amount */
	total = 0.0 ;
	items = 0 ;

	if(PoLast != NULL)
	    for(temppage = FirstPage ; temppage != NULL ;
						temppage = temppage->NextPage) {
		/* Take Each Po Item and Upadte the files */
		for(i = 0 ; i < temppage->NoItems ; i++) {
			total += temppage->Items[i].s_value ;
			items++ ;
		}

		/* IF the process is done upto Current Po Last Page,
		   then break */
		if(temppage == PoLast) break ;
	    }

	if(items == 0) {
#ifdef ENGLISH
		DispError("No Items In PO..  Cancel to Quit..");
#else
		DispError("Pas d'articles dans BC.. Annuler pour retourner..");
#endif
		return(ERROR);
	}

	diff = s_sth.s_po_amt - total ;	/* Difference */

	if(diff < -(DELTA_AMT) || diff > DELTA_AMT) {
#ifdef ENGLISH
		sprintf(e_mesg,
			"PO Total: %.2lf  Items Total: %.2lf  Diff: %.2lf",
			s_sth.s_po_amt, total, diff) ;
#else
		sprintf(e_mesg,
			"Total BC: %.2lf  Total Articles: %.2lf  Diff: %.2lf",
			s_sth.s_po_amt, total, diff) ;
#endif
		DispError(e_mesg) ;
		return(ERROR) ;
	}

	return(NOERROR) ;
}	/* CheckTotal() */
/*-----------------------------------------------------------------------*/ 
/* Process given item and Update the files */

ProcItemUpdates(temppage, item_no)
Page	*temppage ;
int	item_no ;
{
	int	stock_cd_chg = 0;
	int 	mode, err;
	double 	diff, diff_qty ;
	double	alloc_amt;

	if (s_sth.s_type[0] == DIRECT || s_sth.s_type[0] == NON_BULK ) {
		/* get the GL acnt to increase the commitments */
		err = CheckGlAcnt(temppage->Items[item_no].s_accno) ;
		if(err < 0) return(err) ;
	}

	if ((s_sth.s_type[0] != DIRECT && s_sth.s_fn[0] == CHANGE) &&
	    (temppage->PoStatus[item_no] == 'C'))  {
		err = AdjustPrevVals(temppage, item_no);
		if (err < 0) return (err);
	}
	
	po_item.pi_code = po_hdr.ph_code ;
	po_item.pi_item_no = temppage->PoItemNo[item_no] ;
	err = get_poitem(&po_item,UPDATE, 0, e_mesg);
	if(err == UNDEF) 
		mode = ADD ;
	else if(err < 0) {
		DispError(e_mesg) ;
		return(DBH_ERR) ;
	}

	if ((strcmp(po_item.pi_st_code,temppage->Items[item_no].s_stck_cd) != 0)
	 && (s_sth.s_fn[0] == CHANGE) && (temppage->PoStatus[item_no] == 'C'))  
		stock_cd_chg = 1;

	if (temppage->PoStatus[item_no] == COMPLETE) {
		mode = UPDATE ;
		if(po_item.pi_tax1[0] != TAXABLE) po_item.pi_tax1[0] = EXEMPT;
		if(po_item.pi_tax2[0] != TAXABLE) po_item.pi_tax2[0] = EXEMPT;
     		scpy((char*)&pre_item,(char*)&po_item,sizeof(po_item));
	}
	else
		mode = ADD ;

	if ((mode == ADD) || (stock_cd_chg == 1)) { 
		diff = temppage->Items[item_no].s_net_amt;
		diff_qty = temppage->Items[item_no].s_ord_qty ;
	}
	else 
	if (stock_cd_chg == 0) {
		gst_tax = ( (po_item.pi_tax1[0] == TAXABLE)
				? ctl_rec.gst_tax : 0.0 );
		pst_tax = ( (po_item.pi_tax2[0] == TAXABLE)
				? ctl_rec.pst_tax : 0.0 );
		gl_rec.comdat -= Commit_Calculation(gst_tax,pst_tax,
			 ctl_rec.rebate,po_item.pi_value, &tax_cal);
		diff = temppage->Items[item_no].s_net_amt - po_item.pi_value;
		diff_qty = temppage->Items[item_no].s_ord_qty -
				 po_item.pi_orig_qty ; 
		gst_tax = ( (po_item.pi_tax1[0] == TAXABLE) ? ctl_rec.gst_tax : 0.0 );
		pst_tax = ( (po_item.pi_tax2[0] == TAXABLE) ? ctl_rec.pst_tax : 0.0 );
		Tax_Calculation(gst_tax,pst_tax, po_item.pi_value,&tax_cal);
		supplier.s_ytd_ord -= tax_cal.gros_amt ;
	}

	/* If not Direct Disbursement */
	if(s_sth.s_type[0] == NON_BULK) {
	  if(diff_qty < -(DELTA_QTY) || diff_qty > DELTA_QTY ||
	     diff < -(DELTA_AMT) || diff > DELTA_AMT) {
		/* calculate New Allocation value with tax */
		
			/* 21/01/91 CL */
		gst_tax = ( (temppage->Items[item_no].s_gst[0] == TAXABLE)
			? ctl_rec.gst_tax : 0.0 );
		pst_tax = ( (temppage->Items[item_no].s_pst[0] == TAXABLE)
			? ctl_rec.pst_tax : 0.0 );

		Tax_Calculation(gst_tax,pst_tax,
			temppage->Items[item_no].s_net_amt,&tax_cal);
		alloc_amt = tax_cal.gros_amt;

		/* Create/Increase the Allocation */
		err = CheckAllocation(&(temppage->Items[item_no]),mode);
		if(err < 0 && err != UNDEF) return(err) ; 
	
		if(mode == UPDATE && err == UNDEF && stock_cd_chg == 0) {
			/* Increase YTD Ordered in supplier */
			if (diff_qty < -(DELTA_QTY) || diff_qty > DELTA_QTY) {
#ifdef ENGLISH
			    fomer("Quantity already issued - Cannot Update");
#else
			    fomer("Quantite deja emise - Ne peut pas mettre a jour");
#endif
			    get();
			    return(-1);
			}
		}
		else {
			if (mode == UPDATE) {
				if (-(alloc_amt) > aloc_rec.st_value) 
					alloc_amt = -(aloc_rec.st_value);
				if (-(diff_qty) > aloc_rec.st_alloc) {
#ifdef ENGLISH
			    		fomer("Quantity already issued - Cannot Update");
#else
			    		fomer("Quantite deja emise - Ne peut pas mettre a jour");
#endif
					get();
			    		return(-1);
				}
			}
		}

		if(mode==UPDATE && stock_cd_chg == 0) {
			/* calculate Old Allocation value with tax */
			/* and remove from allocation record to add */
			/* new allocation amount. */
			gst_tax = ( (po_item.pi_tax1[0] == TAXABLE)
				? ctl_rec.gst_tax : 0.0 );
			pst_tax = ( (po_item.pi_tax2[0] == TAXABLE)
				? ctl_rec.pst_tax : 0.0 );
			Tax_Calculation(gst_tax,pst_tax,
				po_item.pi_value,&tax_cal);
			aloc_rec.st_value -= tax_cal.gros_amt;
		}

		if(err == UNDEF){
			aloc_rec.st_alloc = 0.0;
			aloc_rec.st_value = 0.0;
		}
		
		aloc_rec.st_alloc += diff_qty ;
		aloc_rec.st_value += alloc_amt;

		if (aloc_rec.st_value < DELTA_AMT)
			aloc_rec.st_value = 0.00;
		
		/* round allocation values after mathmatical calculations */
		aloc_rec.st_alloc = D_Roundoff(aloc_rec.st_alloc);
		aloc_rec.st_value = D_Roundoff(aloc_rec.st_value); 

		if (err == UNDEF)
			err = put_alloc(&aloc_rec, ADD, e_mesg) ;
		else 
			if (aloc_rec.st_alloc < DELTA_QTY)
				err = put_alloc(&aloc_rec, P_DEL, e_mesg) ;
			else	
				err = put_alloc(&aloc_rec, UPDATE, e_mesg) ;
		if (err < 0) return(err) ;
	   }
	}

	err = UpdateWithPoItem(mode,temppage->PoItemNo[item_no],
		temppage->Items[item_no]);
	if (err < 0) return(err) ;

	/* Increase the Commitments */

	gst_tax = ( (temppage->Items[item_no].s_gst[0] == TAXABLE)
			? ctl_rec.gst_tax : 0.0 );
	pst_tax = ( (temppage->Items[item_no].s_pst[0] == TAXABLE)
			? ctl_rec.pst_tax : 0.0 );

	gl_rec.comdat += Commit_Calculation(gst_tax,pst_tax,
		ctl_rec.rebate, temppage->Items[item_no].s_net_amt, &tax_cal);

	if (s_sth.s_type[0] == DIRECT || s_sth.s_type[0] == NON_BULK ) {
			
			/* round gl_rec.comdat after calculation */
		gl_rec.comdat = D_Roundoff(gl_rec.comdat);
		err = put_gl(&gl_rec, UPDATE, e_mesg) ;
		if (err < 0) return(err);
	}

	gst_tax = ( (temppage->Items[item_no].s_gst[0] == TAXABLE)
		? ctl_rec.gst_tax : 0.0 );
	pst_tax = ( (temppage->Items[item_no].s_pst[0] == TAXABLE)
		? ctl_rec.pst_tax : 0.0 );

	Tax_Calculation(gst_tax,pst_tax,
			temppage->Items[item_no].s_net_amt,&tax_cal);
	supplier.s_ytd_ord += tax_cal.gros_amt;
	/* No change in value or quantity so no further processing */
	if ((diff > -(DELTA_AMT) && diff < DELTA_AMT) && 
  	   (diff_qty > -(DELTA_QTY) && diff_qty < DELTA_QTY)) 
		return(NOERROR);

	/* Increase On Order in stock */
	if (s_sth.s_type[0] == BULK || s_sth.s_type[0] == NON_BULK) {
		if ((diff_qty < -(DELTA_QTY) || diff_qty > DELTA_QTY) ||
		    (diff < -(DELTA_AMT) || diff > DELTA_AMT)) { 
			err = UpdateStock(temppage->Items[item_no].s_stck_cd,diff_qty, diff);
			if (err < 0) return(err) ;
		}
	}

	return(NOERROR) ;
}	/* ProcItemUpdates() */

/*-----------------------------------------------------------------------*/
/*  If editing po item in change mode, update both previous stock 
    allocation and stock master files if required.			 */

AdjustPrevVals(temppage, item_no)
Page	*temppage ;
int	item_no ;
{
	int	err;
	double	old_quant = 0.0;
	double	old_value = 0.0;

	po_item.pi_code = po_hdr.ph_code ;
	po_item.pi_item_no = temppage->PoItemNo[item_no] ;
	err = get_poitem(&po_item, UPDATE, 0, e_mesg);
	
	if(err < 0) {
		DispError(e_mesg) ;
		return(DBH_ERR) ;
	}

	if(strcmp(po_item.pi_st_code, temppage->Items[item_no].s_stck_cd) !=0) { 
		old_quant = po_item.pi_orig_qty;
		old_value = po_item.pi_value;

			/* Calculate tax on old_value */

		gst_tax = ( (po_item.pi_tax1[0] == TAXABLE)
			? ctl_rec.gst_tax : 0.0 );
		pst_tax = ( (po_item.pi_tax2[0] == TAXABLE)
			? ctl_rec.pst_tax : 0.0 );
		Tax_Calculation(gst_tax, pst_tax, old_value, &tax_cal);
		old_value = tax_cal.gros_amt;
	
			/*  Read stock master record for old poitem */

		stck_rec.st_fund = po_item.pi_fund ;
		STRCPY(stck_rec.st_code,po_item.pi_st_code) ;

		err = get_stmast(&stck_rec, UPDATE, 0, e_mesg) ;
	
		if(err == UNDEF) return (NOERROR); /* not found */
		if(ERROR == err) return(DBH_ERR);
		if(err < 0) {
			fomer(e_mesg) ;
			return(ERROR) ;
		}

			/* Adjust stock master on_order and st_alloc */

		stck_rec.st_on_order -= old_quant ;
		if (stck_rec.st_on_order < DELTA_QTY) 
			stck_rec.st_on_order = 0.00;
		if (s_sth.s_type[0] == NON_BULK) {
			stck_rec.st_alloc -= old_quant ;
			if (stck_rec.st_alloc < DELTA_QTY) 
				stck_rec.st_alloc = 0.00;
		}
	
			/* round stock master double variables calculated */

		stck_rec.st_on_order	= D_Roundoff(stck_rec.st_on_order); 
		stck_rec.st_alloc	= D_Roundoff(stck_rec.st_alloc); 
	
			/* Update old stock master record */
		stck_rec.st_fund = po_item.pi_fund ;
		STRCPY(stck_rec.st_code,po_item.pi_st_code) ;

		err = put_stmast(&stck_rec, UPDATE, e_mesg) ;
		if (err < 0) return(err) ;

		if (s_sth.s_type[0] == NON_BULK) {

			/* Read allocation record */

			aloc_rec.st_fund = po_item.pi_fund; 
			STRCPY(aloc_rec.st_code, po_item.pi_st_code);
			aloc_rec.st_location = po_item.pi_school;
			STRCPY(aloc_rec.st_expacc, po_item.pi_acct);

			err = get_alloc(&aloc_rec, UPDATE, 0, e_mesg) ;
			
			if(ERROR == err) return(DBH_ERR) ;
			if(err != UNDEF && err < 0) {
				fomer(e_mesg) ;
				get() ;
				return(ERROR) ;
			}

			aloc_rec.st_alloc -= old_quant;
			if (aloc_rec.st_alloc < DELTA_QTY) 
				aloc_rec.st_alloc = 0.00;

			aloc_rec.st_value -= old_value;
			if(aloc_rec.st_value < DELTA_AMT)
				aloc_rec.st_value = 0.00;

				/* round allocation values after mathmatical 
						calculations */

			aloc_rec.st_alloc = D_Roundoff(aloc_rec.st_alloc);
			aloc_rec.st_value = D_Roundoff(aloc_rec.st_value); 

			aloc_rec.st_fund = po_item.pi_fund; 
			STRCPY(aloc_rec.st_code, po_item.pi_st_code);
			aloc_rec.st_location = po_item.pi_school;
			STRCPY(aloc_rec.st_expacc, po_item.pi_acct);
			
			if (aloc_rec.st_alloc < DELTA_QTY)
				err = put_alloc(&aloc_rec, P_DEL, e_mesg) ;
			else	
				err = put_alloc(&aloc_rec, UPDATE, e_mesg) ;

			if (err < 0) return(err) ;
		}
	}

	return(NOERROR);
}  /* AdjustPrevVals */

/*-----------------------------------------------------------------------*/ 
/* Updating files when PoIsGiven. Get the PO item, increase the QTY and
  paid value and update */

UpdateWithPoItem(mode,po_item_no,s_item)
int	mode ;
int	po_item_no ;
S_item 	s_item ;
{
	int 	i, err ;

	if (mode == ADD) {
		po_item.pi_code = po_hdr.ph_code ;
		po_item.pi_item_no = po_item_no ;
		err = get_poitem(&po_item,BROWSE, 0, e_mesg);
		/* get new item no if exists in the file */
		for (i = po_item_no; err == 0; i++){
			po_item.pi_code = po_hdr.ph_code ;
			po_item.pi_item_no = i ;
			err = get_poitem(&po_item,BROWSE, 0, e_mesg);
		}
		po_item.pi_fund = po_hdr.ph_funds ;
		STRCPY(po_item.pi_acct, s_item.s_accno);
		po_item.pi_school = s_item.s_school ;
		po_item.pi_original = s_item.s_value ;
		po_item.pi_pd_qty = 0.0 ;
		po_item.pi_paid = 0.0 ;
	}
		/* for UPDATE mode, the original value remains the amount
		   originally entered not the current value. */
	STRCPY(po_item.pi_st_code, s_item.s_stck_cd);
	STRCPY(po_item.pi_desc, s_item.s_desc);
	po_item.pi_req_no = 0;
	STRCPY(po_item.pi_unit, s_item.s_uom);
	STRCPY(po_item.pi_tax1, s_item.s_gst);
	STRCPY(po_item.pi_tax2, s_item.s_pst);
	po_item.pi_unitprice = s_item.s_unit_cost ;
	po_item.pi_orig_qty = s_item.s_ord_qty ;
	po_item.pi_value = s_item.s_net_amt ;
	po_item.pi_fund = po_hdr.ph_funds ;

		/* round any calculated poitem double values */
	po_item.pi_original = D_Roundoff(po_item.pi_original); 
	po_item.pi_value = D_Roundoff(po_item.pi_value); 

	err = put_poitem(&po_item, mode,e_mesg) ;
	if(err < 0) {
		DispError(e_mesg) ;
		return(DBH_ERR) ;
	}

	if(s_sth.s_fn[0] == CHANGE && mode != ADD) {
		err = rite_audit((char*)&s_sth, POITEM,mode,(char*)&po_item,
				(char*)&pre_item,e_mesg);
		if(err==LOCKED) {
			DispError(e_mesg);
			roll_back(e_mesg) ;
			return(LOCKED) ;
		}
		if(err != NOERROR){
#ifdef	ENGLISH
			DispError("ERROR in Saving Records"); 
#else
			DispError("ERREUR en conservant les fiches");
#endif
			DispError(e_mesg);
			roll_back(e_mesg);
			return(DBH_ERR);
		}
	}

	return(NOERROR) ;
}	/* UpdateWithPoItem() */
/*----------------------------------------------------------------*/
/* get the allocation for given keys in UPDATE mode. If record not exists
   and mode = ADD then make the new record and return UNDEF */

CheckAllocation(s_item, mode)
S_item	*s_item ;
int	mode ;
{
	int	err ;

	aloc_rec.st_fund = s_sth.s_fund ;
	STRCPY(aloc_rec.st_code, s_item->s_stck_cd) ;
	aloc_rec.st_location = s_item->s_school ;
	STRCPY(aloc_rec.st_expacc, s_item->s_accno) ;

	err = get_alloc(&aloc_rec, UPDATE, 0, e_mesg) ;
	if(ERROR == err) return(DBH_ERR) ;
	if(err != UNDEF && err < 0) {
		fomer(e_mesg) ;
		get() ;
		return(ERROR) ;
	}
	if(err == UNDEF && mode == ADD) {	/* Make the Rec */
		aloc_rec.st_date = s_sth.s_entry_dt ;
		aloc_rec.st_time = get_time() ;
		aloc_rec.st_issued = 0.0 ;
		aloc_rec.st_alloc = 0.0 ;
		aloc_rec.st_value = 0.0 ;
	}
	return(err) ;
}	/* CheckAllocation() */
/*----------------------------------------------------------------*/
/* Read the given stock code record in UPDATE mode */

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
/*-----------------------------------------------------------------------*/
/* Update the stock allocation and on-order quantities.  If site wants   */
/* to have po/inventory interface which is set in the parameters then    */
/* the committed value and average price in inventory will be updated.   */

UpdateStock(stock_code, quantity, amount)
char	*stock_code;
double	quantity;
double	amount;
{
	int	err;

	err = CheckStock(stock_code) ;
	if (err < 0) return(err) ;
/* Cathy Burns June 20,1991 - Update Stock with committed quantities and values 			     when the parameter is set for po to inventory 
			     interface  				    */
	if(pa_rec.pa_poinv[0] == YES) {
		stck_rec.st_po_ordqty += quantity ;
		if (stck_rec.st_po_ordqty < DELTA_QTY) 
			stck_rec.st_po_ordqty = 0.00;
		gst_tax = ( (po_item.pi_tax1[0] == TAXABLE)
				? ctl_rec.gst_tax : 0.0 );
		pst_tax = ( (po_item.pi_tax2[0] == TAXABLE)
				? ctl_rec.pst_tax : 0.0 );
		stck_rec.st_committed += Commit_Calculation(gst_tax,pst_tax,
			 ctl_rec.rebate,amount, &tax_cal);
		if (stck_rec.st_committed < DELTA_QTY) 
			stck_rec.st_committed = 0.00;
	}
	stck_rec.st_on_order += quantity ;
	if (stck_rec.st_on_order < DELTA_QTY) 
		stck_rec.st_on_order = 0.00;
	if (s_sth.s_type[0] == NON_BULK) {
		stck_rec.st_alloc += quantity ;
		if (stck_rec.st_alloc < DELTA_QTY) 
			stck_rec.st_alloc = 0.00;
	}

		 /* round stmast double variables calculated */
	stck_rec.st_on_order= D_Roundoff(stck_rec.st_on_order); 
	stck_rec.st_po_ordqty= D_Roundoff(stck_rec.st_po_ordqty); 
	stck_rec.st_committed= D_Roundoff(stck_rec.st_committed); 
	stck_rec.st_alloc = D_Roundoff(stck_rec.st_alloc); 

	/*******************************************************/
	/* DO NOT Round stock rate because it is four decimals */
	if(pa_rec.pa_poinv[0] == YES) 
		stck_rec.st_rate = (stck_rec.st_value + stck_rec.st_committed) / (stck_rec.st_on_hand + stck_rec.st_paidfor + stck_rec.st_po_ordqty);
	else
		stck_rec.st_rate = stck_rec.st_value / (stck_rec.st_on_hand + stck_rec.st_paidfor);

	err = put_stmast(&stck_rec, UPDATE, e_mesg) ;
	if (err < 0) return(err) ;

	return(NOERROR);
}	/* UpdateStock() */
/*-----------------------------------------------------------------------*/ 
/* Copy the key fields from screen to po header record and get the
   record from data base */

GetPoHdr(md)
int	md; /* BROWSE or UPDATE */
{
	po_hdr.ph_code = s_sth.s_po_no ;

	return(get_pohdr(&po_hdr, md, 0, e_mesg));
}
/*------------------------------------------------------------*/
/* Get the Header details from user */

ReadHdr()
{
	int	i ;
	
	/* Change PROFOM logical attributes */
	for(i = HDR_ST_FLD ; i <= HDR_END_FLD ; i += 100) {
		fomca1(i,19,0); /* Disable Dup control */
		fomca1(i,10,1); /* Enable User Escape */
	}

#ifdef ENGLISH
	STRCPY(s_sth.s_mesg,"Press ESC-F to Go Back to Fn:");
#else
	STRCPY(s_sth.s_mesg,"Appuyer sur ESC-F pour retourner a Fn:");
#endif
	ShowMesg();

	/* Show last entered entry date as a default entry date */
	s_sth.s_entry_dt = Entry_date ;
	SetDupBuffers(ENTRYDT_FLD,ENTRYDT_FLD,1);

	/* Show current period as default */
	s_sth.s_period = pa_rec.pa_cur_period ;
	SetDupBuffers(PERIOD_FLD,PERIOD_FLD,1);
	
	/* Show fund. no as default 1 */
	s_sth.s_fund  = 1; 
	SetDupBuffers(FUND_FLD,FUND_FLD,1);
	
	/* Show Direct charge as default type po */
#ifdef ENGLISH
	STRCPY(s_sth.s_type, "D") ;
#else
	STRCPY(s_sth.s_type, "C") ;
#endif
	SetDupBuffers(TYPE_FLD,TYPE_FLD,1);

	/* Initialize Header fields with Low values */
	InitHdr(LV_CHAR, LV_SHORT, LV_LONG, LV_DOUBLE) ;
	s_sth.s_liq_dt = 0 ;
	s_sth.s_liq_amt = 0 ;
	s_sth.s_billto = pa_rec.pa_distccno ;

	i = ReadFields(HDR_ST_FLD, HDR_END_FLD,
		KeyAndHdrValidation, HdrAndKeyWindowHelp, 0) ;
	if(PROFOM_ERR == i || DBH_ERR == i) return(i) ;
	if(RET_USER_ESC == i) {	/* ESC-F */
		InitHdr(HV_CHAR,HV_SHORT,HV_LONG,HV_DOUBLE) ;

		ret( WriteFields(HDR_ST_FLD, HDR_END_FLD) ) ;

		s_sth.s_mesg[0] = HV_CHAR ;
		ShowMesg() ;

		return(ERROR) ;
	}

	/** set Entry date to the entry date shown on the screen **/
	Entry_date = s_sth.s_entry_dt;

	return(NOERROR) ;
}	/* ReadHdr() */
/*------------------------------------------------------------*/
/* Read Item Details from the User */

AddItems()
{
	int	i, err ;

	/* If the last node of po is Partial filled then Show Page */
	if(PoLast != NULL && PoLast->NoItems < PAGESIZE ) {
		ret( ShowItems(PoLast) ) ;
		i = PoLast->NoItems ;
		CurPage = PoLast ;
	}
	else {
		/* Calculate the page# */
		if(PoLast != NULL) {
			i = PAGESIZE ;
			CurPage = PoLast ;
		}
		else {
			s_sth.s_page_no = 1 ;
			strcpy(s_sth.s_dummy1, "  ");
			ret( WriteFields(PAGENO_FLD, COLHDG_FLD) ) ;
			i = 0 ;
		}
	}


	for( ; ; ) {
		if( PAGESIZE == i) {	/* Page Full */

			/* Move High Values to All items except first */
			for(i-- ; i > 0 ; i--)
				InitItem(i, HV_CHAR, HV_SHORT, HV_DOUBLE, ADD) ;

			/* Calculate the page# */
			s_sth.s_page_no = PoLast->Pageno + 1 ;

			ret( WriteFields(PAGENO_FLD, (END_FLD - 200)) ) ;

			i = 0 ;
		}

		err = ReadItem(i,ADD) ;		/* Read Each Item Line */
		if(PROFOM_ERR == err || DBH_ERR == err) return(err) ;
		if(NOERROR != err) break ;	/* ESC-F */

		if(0 == i)	/* First Item in the Page */
			if((err = MakeFreshPage()) < 0) return(err) ;
		
		/* Copy the Item to List */
		scpy((char*)&(CurPage->Items[i]), (char*)&(s_sth.s_items[i]),
			sizeof(S_item)) ;

		CurPage->PoItemNo[i] = ((PoLast->Pageno - 1) * 
					PAGESIZE) + (i + 1) ;
		CurPage->PoStatus[i] = 'A' ;
		i++ ;

		CurPage->NoItems = i;
	}
	if(i == 0) 
		if((err=ShowItems(CurPage))<0) return(err) ;

	return(NOERROR) ;
}	/* AddItems() */
/*------------------------------------------------------------*/
/*
*	Get the next node in linked list to add po items. If the
*	(Cur. Po last page) = (Last Page in linked List) or no
*	nodes in list, allocate node and add to linked list
*/

MakeFreshPage()
{
	Page	*tempptr ;

	/* If, no node is allocated yet or Current po used all the nodes,
	   then allocate new node */

	if( LastPage == NULL || PoLast == LastPage ){
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

	if(PoLast == NULL)
		PoLast = FirstPage ;
	else
		PoLast = PoLast->NextPage ;

	PoLast->NoItems = 0 ;
	CurPage = PoLast ;

	return(NOERROR);
}	/* MakeFreshPage() */
/*------------------------------------------------------------*/
/* Read details of given item# */

ReadItem(item_no,mode)
int	item_no ;
int	mode ;
{
	int	i, j, k ;
	int	st_fld ;
	int	end_fld ;

#ifdef ENGLISH
	STRCPY(s_sth.s_mesg,"Press ESC-F to Terminate");
#else
	STRCPY(s_sth.s_mesg,"Appuyer sur ESC-F pour terminer");
#endif
	ShowMesg();

	st_fld  = ITEM_ST_FLD + (STEP * item_no) ;
	end_fld = st_fld + STEP - 100 ;

	/* if Update mode subtract value of item entered then later
	   add value of new item */

	if(UPDATE == mode) 
		s_sth.s_total_itm -= s_sth.s_items[item_no].s_value;

	/* If mode == UPDATE turn ON dup control, else OFF */
	SetDupBuffers(st_fld,end_fld,(mode == UPDATE) ? 1 : 0); 

	if(ADD == mode) {
		/* Set the account number default to the last account number */
		/* entered if not the first item of the first page */

		if(item_no!=0 || s_sth.s_page_no != 1) {
		      STRCPY(s_sth.s_items[item_no].s_accno,Prev_acct);
		      SetDupBuffers(st_fld+ACNO_FLD,st_fld+ACNO_FLD,1);
		}
		s_sth.s_items[item_no].s_ord_qty = 1.0 ;
		SetDupBuffers(st_fld+QTY_FLD,st_fld+QTY_FLD,1);
	}

	/* Initialize Reading Item with Low values */
	InitItem(item_no, LV_CHAR, LV_SHORT, LV_DOUBLE, mode) ;

	if(ADD == mode) 
		s_sth.s_items[item_no].s_sno = item_no + 1;

	if (s_sth.s_type[0] == BULK) 	/* Get default inventory general acct */
		STRCPY(s_sth.s_items[item_no].s_accno, ctl_rec.inv_acnt); 

	i = ReadFields(st_fld,end_fld,ItemsValidation,ItemsWindowHelp,mode) ;
	if(PROFOM_ERR == i || DBH_ERR == i) return(i) ;
	if(RET_USER_ESC == i) {
		if(ADD == mode) {	/* Terminating the ADD */
			InitItem(item_no, HV_CHAR, HV_SHORT, HV_DOUBLE, mode) ;
			ret( WriteFields(st_fld, end_fld) );
			return(RET_USER_ESC) ;
		}
		/*
		* When user gives ESC-F while changing fields, assumption is
		* he completed his changes, and remaining fields are same as
		* old. But, at this point STH will be having low values in the 
		* remaining fields. So move the old values form the linked list.
		*/

/* CB Dec 18,1991 - When editing a line an ESC F entered, any changes to that
**  	  line will reveated back to line before changes.   
**		/ * Get Offset of the begining field of the Item in 'k' * /
**		fomfp(st_fld,&k,&j) ;
**		/ * Offset to the field where ESC-F Pressed in 'j' * /
**		fomfp(sr.curfld,&j,&i);
**		i =  j - k ;	/ * Offset within item * /
**
**		j = sizeof(S_item) - i ;  / * Length to copy * /
**    		scpy((char *)(&s_sth.s_items[item_no])+i,
**			(char*)(&CurPage->Items[item_no])+i, j);
******************************************************************************/
		scpy((char *)(&s_sth.s_items[item_no]), 
		     (char *)(&CurPage->Items[item_no]), sizeof(S_item));
		ret( WriteFields(st_fld, end_fld) );

		s_sth.s_total_itm += s_sth.s_items[item_no].s_value;
		return(RET_USER_ESC) ;
	}

	/** Set previous account number to last account number entered **/
	STRCPY(Prev_acct,s_sth.s_items[item_no].s_accno);

	/* Calculate and Display totals at Bottom of Screen */
	s_sth.s_total_po = s_sth.s_po_amt;
	s_sth.s_total_itm += s_sth.s_items[item_no].s_value;
	s_sth.s_total_diff = s_sth.s_total_po - s_sth.s_total_itm;

	ret(WriteFields(TOTAL_ST_FLD,TOTAL_END_FLD));

	return(NOERROR) ;
}	/* ReadItem() */
/*------------------------------------------------------------*/
/* Read the PROFOM Screen for a given Range of fields */

static
ReadFields(st_fld, end_fld, Validate, WindowHelp, mode)
int	st_fld ;
int	end_fld;
int	(*Validate)() ;
int	(*WindowHelp)() ;
int	mode ;	/* ADD or UPDATE. This is required only when reading item */
{
	int	err ;

	sr.nextfld = st_fld ;
	sr.endfld  = end_fld;
	for( ; ;){
		fomrd( (char *)&s_sth );
		ret(err_chk(&sr));

		if(sr.retcode == RET_VAL_CHK){
			err = (*Validate)(mode) ;
			if(DBH_ERR == err || PROFOM_ERR == err) return(err);
			sr.nextfld = sr.curfld ;
			continue;
		}
		if(sr.retcode == RET_USER_ESC){
			if(sr.escchar[0] == 'f' || sr.escchar[0] == 'F')
				return(RET_USER_ESC) ;
			if(sr.escchar[0] == 'h' || sr.escchar[0] == 'H'){
				err = (*WindowHelp)() ;
				if(DBH_ERR == err || PROFOM_ERR == err)
					return(err) ;
				sr.nextfld = sr.curfld ;
				continue;
			}
			continue;
		}
		/* else RET_NO_ERROR */
		break;
	}

	return(NOERROR) ;
}	/* ReadFields() */
/*------------------------------------------------------------*/
/* Write fields on Screen for a given Range */

static
WriteFields(st_fld, end_fld)
int	st_fld ;
int	end_fld ;
{
	sr.nextfld = st_fld ;
	sr.endfld  = end_fld ;

	fomwr( (char *)&s_sth );
	ret(err_chk(&sr));

	return(NOERROR) ;
}	/* WriteFields() */
/*----------------------------------------------------------------*/
/* Set Duplication buffers for fields 				  */
SetDupBuffers( firstfld, lastfld, value )
int	firstfld, lastfld;	/* field numbers range */
int	value;			/* ENABLE or DISABLE */
{
	int i ;

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
/*----------------------------------------------------------------*/
/* Copy the items form given node to screen and display */

ShowItems(pageptr)
Page	*pageptr ;
{
	int	i ;

	if(pageptr != NULL) {
		/* Copy the items to screen */
		scpy((char*)s_sth.s_items, (char*)pageptr->Items,
			(pageptr->NoItems * sizeof(S_item)) );

		s_sth.s_page_no   = pageptr->Pageno ;

		if(pageptr->NoItems==2) {
			s_sth.s_items[0].s_subhdg[0] = ' ';
			s_sth.s_items[1].s_subhdg[0] = ' ';
		}
		if(pageptr->NoItems==1) 
			s_sth.s_items[0].s_subhdg[0] = ' ';
		
		strcpy(s_sth.s_dummy1, "  ");
		i = pageptr->NoItems ;
	}
	else {
		s_sth.s_page_no   = HV_SHORT ;
		s_sth.s_dummy1[0] = HV_CHAR ;
		i = 0 ;
	}

	/* Move High Values to remaining Items */
	for( ; i < PAGESIZE ; i++ )
		InitItem(i, HV_CHAR, HV_SHORT, HV_DOUBLE, UPDATE) ;

	ret( WriteFields( PAGENO_FLD, (END_FLD - 200)) );

	return(NOERROR) ;
}	/* ShowItems() */
/*----------------------------------------------------------------*/
/* Validation function() for Key and Header fields when PROFOM returns
  RET_VAL_CHK */

KeyAndHdrValidation(mode)
int	mode;
{
	int	err, save_nextfld, save_endfld ;

	switch(sr.curfld){
	case	KEY_START	:	/* po number */
		if(s_sth.s_fn[0] == ADDREC) { /* only validate in add mode */
			if(s_sth.s_po_no == po_hdr.ph_code) {
				sr.curfld += 100;
			}
			if(s_sth.s_po_no != po_hdr.ph_code &&
				s_sth.s_po_no >= 10000000) {
#ifdef ENGLISH
			fomer("PO number cannot be greater than 9999999");
#else
			fomer("Numero de BC ne peut pas etre plus grand que 9999999");
#endif
				s_sth.s_po_no = LV_LONG;
				break;
			}
			if(s_sth.s_po_no != po_hdr.ph_code) 
				s_sth.s_po_no += 90000000;
				
			po_hdr.ph_code = s_sth.s_po_no ;
			err = get_pohdr(&po_hdr, BROWSE, 0, e_mesg);
			if(err == UNDEF) break;
			if(err < 0) {
				DispError(e_mesg) ;
				return(DBH_ERR) ;
			}
			else {
#ifdef ENGLISH
				fomer("PO already exists");
#else
				fomer("BC existe deja");
#endif
				s_sth.s_po_no = LV_LONG;
				break;
			}
		}
		break;
	case	SUPPCD_FLD	:	/* Supplier Cd: */
  		Right_Justify_Numeric(s_sth.s_supp_cd,
					sizeof(s_sth.s_supp_cd)-1);
		STRCPY(supplier.s_supp_cd, s_sth.s_supp_cd) ;
		err = get_supplier(&supplier, UPDATE, 0, e_mesg) ;
		if(ERROR == err) return(DBH_ERR) ;
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
		fomer(supplier.s_name);
		if (s_sth.s_fn[0] != ADDREC) 
			break;
		STRCPY(s_sth.s_payee_cd,supplier.s_payee);
		save_nextfld = sr.nextfld;
		save_endfld = sr.endfld;
		SetDupBuffers(PAYEE_FLD,PAYEE_FLD,2);
		s_sth.s_payee_cd[0] = LV_CHAR;
		sr.nextfld = save_nextfld;
		sr.endfld = save_endfld;
		break ;
	case	PAYEE_FLD	:	/* payee Cd: */
		if(s_sth.s_payee_cd[0] == '\0') {
			sr.curfld+=100; /* skip this field */
			break;
		}
  		Right_Justify_Numeric(s_sth.s_payee_cd,
				sizeof(s_sth.s_payee_cd)-1);
		STRCPY(supplier.s_supp_cd, s_sth.s_payee_cd) ;
		err = get_supplier(&supplier, BROWSE, 0, e_mesg) ;
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
		    s_sth.s_payee_cd[0] = LV_CHAR ;
		    return(ERROR) ;
		}
		fomer(supplier.s_name);
		break ;
	case	POAMT_FLD	:	/* Commitments: */

		if(s_sth.s_po_amt < 0) { 
#ifdef ENGLISH
			fomer("Commitments must be greater or equal to zero");
#else
			fomer("Engagements doivent etre plus grands ou egals a zero");
#endif
			s_sth.s_po_amt = LV_DOUBLE;
			return(ERROR) ;
		}
		break ;

	case	ENTRYDT_FLD	:	/* Entry Dt: */
		/* Entry Date shouldn't be future date */
		if(s_sth.s_entry_dt > get_date()) {
#ifdef ENGLISH
			fomer("Date Can't be Future Date");
#else
			fomer("Date ne peut pas etre ulterieure");
#endif
			s_sth.s_entry_dt = LV_LONG ;
			return(ERROR) ;
		}
		/* If the entry date is, show default due date as 90 days after
		   entry date */
		if(s_sth.s_fn[0] == ADDREC || s_sth.s_fn[0] == CHANGE) {
			s_sth.s_due_dt = date_plus(s_sth.s_entry_dt,
							pa_rec.pa_due_days_po);
			save_nextfld = sr.nextfld;
			save_endfld = sr.endfld;
			SetDupBuffers(DUEDT_FLD,DUEDT_FLD,1);
			if(mode != UPDATE) {
				s_sth.s_due_dt = LV_LONG ;
			}
			sr.nextfld = save_nextfld;
			sr.endfld = save_endfld;
		}
		break ;
	case	DUEDT_FLD	:	/* Due Date: */
		if(s_sth.s_due_dt < s_sth.s_entry_dt) {
#ifdef ENGLISH
			fomer("Due date can't be before Po Date");
#else
			fomer("Echeance ne peut pas etre avant la date du BC");
#endif
			s_sth.s_due_dt = LV_LONG;
			return(ERROR) ;
		}
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
		fomer(ctl_rec.desc);
		break ;
	case SHIPTO_FLD :	/* Ship to Fields */
		schl_rec.sc_numb = s_sth.s_shipto ;
		err = get_sch(&schl_rec, BROWSE, 0, e_mesg) ;
		if(ERROR == err) return(DBH_ERR) ;
		if(err < 0) {
			fomer(e_mesg) ;
			s_sth.s_shipto = LV_SHORT ;
			return(ERROR) ;
		}
		if( (s_sth.s_fn[0] == ADDREC) && (mode == UPDATE) ||
		    (s_sth.s_fn[0] == CHANGE) ) {
			STRCPY(s_sth.s_attention,schl_rec.sc_contact);

			save_nextfld = sr.nextfld;
			save_endfld = sr.endfld;
			ret(WriteFields(ATT_FLD,ATT_FLD));
			sr.nextfld = save_nextfld;
			sr.endfld = save_endfld;
		}
		fomer(schl_rec.sc_name);
		break ;
	case	TYPE_FLD	:	/* Type: */
		/* Type of Po should be Direct Disbursement, Bulk or
		   Non Bulk Order */
		if(s_sth.s_type[0] != DIRECT && s_sth.s_type[0] != BULK &&
				s_sth.s_type[0] != NON_BULK) {
#ifdef ENGLISH
			fomer("Valid types are D(irect Charge), B(ulk) and N(on Bulk)");
#else
			fomer("Genres valables sont C(harge Directe), E(n vrac) et S(tock reserve)");
#endif
			s_sth.s_type[0] = LV_CHAR ;
			return(ERROR) ;
		}
		break ;
	case	REQ_FLD :
		/* used to set duplication on attention for default */
		if(s_sth.s_fn[0] == ADDREC) { 
			STRCPY(s_sth.s_attention,schl_rec.sc_contact);

			save_nextfld = sr.nextfld;
			save_endfld = sr.endfld;
			SetDupBuffers(ATT_FLD,ATT_FLD,2);
			s_sth.s_attention[0] = LV_CHAR;
			sr.nextfld = save_nextfld;
			sr.endfld = save_endfld;

		}
		if(s_sth.s_req_no[0] == '\0') 
			sr.curfld += 100;

		break;
	case	GSTMSG_FLD	:	/* GST Msg: */
		/* Gst Message should be blank, English or French */
		if(s_sth.s_gstmsg[0] == LV_CHAR)  {
			sr.curfld += 100;
			break;
		}
		if(s_sth.s_gstmsg[0] != ENGLISH_MSG && s_sth.s_gstmsg[0] != FRENCH_MSG){
#ifdef ENGLISH
			fomer("Valid Types are blank, E(nglish), F(rench)");
#else
			fomer("Genres valables sont blanc, A(nglais), F(rancais)");
#endif
			s_sth.s_gstmsg[0] = LV_CHAR ;
			return(ERROR) ;
		}
		break;
	case	ATT_FLD	:
		if(s_sth.s_attention[0] == '\0') 
			sr.curfld += 100;
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

	/* Calculate item# and Fld# within item */
	item_no = (sr.curfld - ITEM_ST_FLD) / STEP ;
	fld_no  = (sr.curfld - ITEM_ST_FLD) % STEP ;

	switch(fld_no){
	case STCKCD_FLD :		/* Stock Code Field */
		/* If Direct Disbursement, No validation */
		if(s_sth.s_type[0] == DIRECT ) {
			STRCPY(s_sth.s_items[item_no].s_stck_cd, " ");
			sr.curfld += 100;
			break;
		}
		
		err = CheckStock(s_sth.s_items[item_no].s_stck_cd);
		if(err == DBH_ERR) return(err);
		if(err < 0) {
			s_sth.s_items[item_no].s_stck_cd[0] = LV_CHAR ;
			return(ERROR) ;
		}

		if( (s_sth.s_fn[0] == ADDREC) && (mode != UPDATE) ||
		    (s_sth.s_fn[0] == CHANGE) && (mode == ADD) ) {
			/** added for description ****/
			cur_fld = ITEM_ST_FLD + (STEP * item_no) ;
			cur_fld += DESC_FLD ;
			STRCPY( s_sth.s_items[item_no].s_desc, stck_rec.st_desc);
			SetDupBuffers(cur_fld,cur_fld,2);
	
			s_sth.s_items[item_no].s_desc[0] = LV_CHAR ;
			sr.curfld = (ITEM_ST_FLD + (STEP * item_no) ) + STCKCD_FLD ;
			sr.endfld = (ITEM_ST_FLD + (STEP * item_no) ) + (STEP - 100) ;

			/** added for unit/cost ****/
			cur_fld = ITEM_ST_FLD + (STEP * item_no) ;
			cur_fld += PRICE_FLD ;
			s_sth.s_items[item_no].s_unit_cost = stck_rec.st_rate;
			SetDupBuffers(cur_fld,cur_fld,2);
	
			s_sth.s_items[item_no].s_unit_cost = LV_DOUBLE ;
			sr.curfld = (ITEM_ST_FLD + (STEP * item_no) ) + STCKCD_FLD ;
			sr.endfld = (ITEM_ST_FLD + (STEP * item_no) ) + (STEP - 100) ;
	
			/* Show the PO UOM as a default UOM on stock master */
			cur_fld = ITEM_ST_FLD + (STEP * item_no) ;
			cur_fld += UOM_FLD ;
			STRCPY( s_sth.s_items[item_no].s_uom, stck_rec.st_unit);
			SetDupBuffers(cur_fld,cur_fld,2);

			s_sth.s_items[item_no].s_uom[0] = LV_CHAR ;
			sr.curfld = (ITEM_ST_FLD + (STEP * item_no) ) + STCKCD_FLD ;
			sr.endfld = (ITEM_ST_FLD + (STEP * item_no) ) + (STEP - 100) ;
		}
		fomer(stck_rec.st_desc);
		break ;
	case UOM_FLD :
		if( s_sth.s_items[item_no].s_uom[0] == '\0'  ) {
			STRCPY( s_sth.s_items[item_no].s_uom, "   "); 
			break;
		}
		break;
/*	REQUISITION FIELD MOVED to HEADER
	case REQ_FLD :
		if( s_sth.s_items[item_no].s_req_no[0] == '\0' ) {
			STRCPY( s_sth.s_items[item_no].s_req_no, "   "); 
			break;
		}
		break;
*/
	case DESC_FLD :
		/* If Not a Direct Disbursement, No validation */
		if(s_sth.s_type[0] == DIRECT) {

			if(s_sth.s_items[item_no].s_desc[0] == '\0') {
#ifdef ENGLISH
			   fomer("Description is required for Direct Charge");
#else
			   fomer("Description est requise pour charge directe");
#endif
				s_sth.s_items[item_no].s_desc[0] = LV_CHAR;
			}
		}
		/* Show the PO TAX type as TAXABLE */
		cur_fld = ITEM_ST_FLD + (STEP * item_no) ;
		cur_fld += GST_FLD ;

	/* default gst & pst flags to defaults set in parameter file
  	   if add ftn & not update mode or change ftn & add mode. (CL) */

		if( (s_sth.s_fn[0] == ADDREC) && (mode != UPDATE) ||
		    (s_sth.s_fn[0] == CHANGE) && (mode == ADD) ) {
			s_sth.s_items[item_no].s_gst[0] = pa_rec.pa_gst_tax[0];
			s_sth.s_items[item_no].s_pst[0] = pa_rec.pa_pst_tax[0];
  			SetDupBuffers(cur_fld,cur_fld+100,2);
		}

		s_sth.s_items[item_no].s_gst[0] = LV_CHAR ;
		s_sth.s_items[item_no].s_pst[0] = LV_CHAR ;

		sr.endfld = (ITEM_ST_FLD + (STEP * item_no) ) + (STEP - 100) ;
		break;
	case SCHOOL_FLD :	/* School Fields */
		schl_rec.sc_numb = s_sth.s_items[item_no].s_school ;
		err = get_sch(&schl_rec, BROWSE, 0, e_mesg) ;
		if(ERROR == err) return(DBH_ERR) ;
		if(err < 0) {
			fomer(e_mesg) ;
			s_sth.s_items[item_no].s_school = LV_SHORT ;
			return(ERROR) ;
		}
		fomer(schl_rec.sc_name);
		break ;
	case ACNO_FLD :		/* GL Account# Fields */
		if (acnt_chk(s_sth.s_items[item_no].s_accno) < 0)  {
			s_sth.s_items[item_no].s_accno[0] = LV_CHAR ;
#ifdef ENGLISH
			fomer("Invalid GL Account Number");
#else
			fomer("Numero de compte G/L invalide");
#endif
			return(ERROR) ;
		}
		err = CheckGlAcnt(s_sth.s_items[item_no].s_accno) ;
		if(DBH_ERR == err) return(err) ;
		if(err < 0) {
			s_sth.s_items[item_no].s_accno[0] = LV_CHAR ;
			return(ERROR) ;
		}
		if(s_sth.s_type[0] == DIRECT || s_sth.s_type[0] == NON_BULK) {
			if(strcmp(s_sth.s_items[item_no].s_accno,
						ctl_rec.inv_acnt)==0){
#ifdef	ENGLISH
				fomer("Cannot use Inventory General account for Direct or Non-Bulk PO's");
#else
				fomer("Ne peut pas utiliser compte d'invent. general pour BC's dir. ou stock reserve");
#endif
				s_sth.s_items[item_no].s_accno[0] = LV_CHAR ;
				return(ERROR) ;
			}
		}
		fomer(gl_rec.desc);
		break ;
	case GST_FLD:		/* GST Fields */
		if( s_sth.s_items[item_no].s_gst[0] != TAXABLE &&
			s_sth.s_items[item_no].s_gst[0] != EXEMPT )  {
#ifdef	ENGLISH
			fomer("GST Valid Tax codes are T(axable) or E(xempt)");
#else
			fomer("Codes valides de taxe sont T(axable) ou E(xempte)");
#endif
			s_sth.s_items[item_no].s_gst[0] = LV_CHAR;
		}
		break;
	case PST_FLD:		/* PST Fields */
		if( s_sth.s_items[item_no].s_pst[0] != TAXABLE &&
			s_sth.s_items[item_no].s_pst[0] != EXEMPT )  {
#ifdef	ENGLISH
			fomer("PST Valid Tax codes are T(axable) or E(xempt)");
#else
			fomer("Codes valides de taxe sont T(axable) ou E(xempte)");
#endif
			s_sth.s_items[item_no].s_pst[0] = LV_CHAR;
		}
		break;
	case QTY_FLD :		/* QTY Fields */
		if ( s_sth.s_items[item_no].s_ord_qty < DELTA_QTY) {
#ifdef ENGLISH
			fomer("Can't be zero");
#else
			fomer("Ne peut pas etre zero");
#endif
			s_sth.s_items[item_no].s_ord_qty = LV_DOUBLE ;
			return(ERROR) ;
		}
		if(mode == UPDATE && CurPage->PoStatus[item_no] != 'A' &&
		    (s_sth.s_type[0] == BULK || s_sth.s_type[0] == NON_BULK) &&
  		    (!strcmp(s_sth.s_items[item_no].s_stck_cd,
  							po_item.pi_st_code))){ 
			/* Changing QTY shouldn't make Stock On-order QTY -ve */
			temp = stck_rec.st_on_order +
				(s_sth.s_items[item_no].s_ord_qty -
					po_item.pi_orig_qty) ;
			if(temp < -(DELTA_QTY)) {
#ifdef ENGLISH
				fomer("On Order QTY of Stock becoming negative.. Invalid Change");
#else
				fomer("Quantite de stock sur commande devient negative... Changement invalide");
#endif
				s_sth.s_items[item_no].s_ord_qty = LV_DOUBLE ;
				return(ERROR) ;
			}
			/* Changing QTY shouldn't make Po Item QTY less than
			   it is already paid for */
			temp = s_sth.s_items[item_no].s_ord_qty -
							po_item.pi_pd_qty ;
			if(temp < -(DELTA_QTY)) {
#ifdef ENGLISH
				fomer("Order QTY Can't be less than QTY Invoiced");
#else
				fomer("Quantite commandee ne peut pas etre moins que la quantite facturee");
#endif
				s_sth.s_items[item_no].s_ord_qty = LV_DOUBLE ;
				return(ERROR) ;
			}
			/* Check allocations if they do not exist then 
			   quantity cannot be changed decause issued are
			   already complete */
			if (s_sth.s_type[0] == NON_BULK) {
			    temp = s_sth.s_items[item_no].s_ord_qty -
			  		po_item.pi_orig_qty ;
			    if (temp < -(DELTA_QTY) || temp > DELTA_QTY) {
				err = CheckAllocation(&(s_sth.s_items[item_no])
					,mode);
  				if(err == UNDEF && strcmp(po_item.pi_st_code,
  					s_sth.s_items[item_no].s_stck_cd)) {
#ifdef ENGLISH
					fomer("Quantity already issued - Cannot Update");
#else
					fomer("Quantite deja emise - Ne peut pas mettre a jour");
#endif
					s_sth.s_items[item_no].s_ord_qty = LV_DOUBLE ;
					return(ERROR) ;
				}
			    }
			}
		}
		if(mode == ADD)
			s_sth.s_items[item_no].s_os_qty =
				s_sth.s_items[item_no].s_ord_qty ;
		else {
			temp = s_sth.s_items[item_no].s_ord_qty -
				po_item.pi_pd_qty ;
			s_sth.s_items[item_no].s_os_qty = temp ;
			if (s_sth.s_items[item_no].s_os_qty < DELTA_QTY)
				s_sth.s_items[item_no].s_os_qty = 0.00;
		}

		if(s_sth.s_type[0] != DIRECT) {
			if(s_sth.s_items[item_no].s_ord_qty + 
				stck_rec.st_on_hand > stck_rec.st_max) {
				sprintf(e_mesg,"Ordering more than max. limit of %12.4lf by %12.4lf",
					stck_rec.st_max,
					(s_sth.s_items[item_no].s_ord_qty + 
					stck_rec.st_on_hand) -
					stck_rec.st_max); 
				fomer(e_mesg); get();
			}	
		}
		/* Show the PO OS QTY as a default */
		cur_fld = ITEM_ST_FLD + (STEP * item_no) ;
		cur_fld += OSQTY_FLD ;
		sr.nextfld = cur_fld;
		cur_fld = sr.curfld ;
		end_fld = sr.endfld ;
		sr.endfld  = sr.nextfld;
		fomwf((char*)&s_sth) ;
		ret(err_chk(&sr)) ;
		sr.curfld  = cur_fld ;
		sr.endfld  = end_fld ;

		break ;

	case PRICE_FLD :	/* Cost/Unit Fields */
/*		if(mode == ADD) {  			 (CL) */

		/* Show the PO Value as a default amount */
		cur_fld = ITEM_ST_FLD + (STEP * item_no) ;
		cur_fld += NET_FLD ;
		s_sth.s_items[item_no].s_net_amt = 
			s_sth.s_items[item_no].s_ord_qty *
				s_sth.s_items[item_no].s_unit_cost ;
		SetDupBuffers(cur_fld,cur_fld,1);

		s_sth.s_items[item_no].s_net_amt = LV_DOUBLE ;
		sr.curfld  = cur_fld ;
		sr.endfld  = end_fld ;
/*		}						*/
		break ;
	case NET_FLD :			/* Amount Fields */
				/* Must enter value for item */

		gst_tax = ( (s_sth.s_items[item_no].s_gst[0] == TAXABLE) ?
				ctl_rec.gst_tax : 0.0 );

		pst_tax = ( (s_sth.s_items[item_no].s_pst[0] == TAXABLE) ?
				ctl_rec.pst_tax : 0.0 );
		net_amt = s_sth.s_items[item_no].s_net_amt;
		
		Tax_Calculation(gst_tax,pst_tax,net_amt,&tax_cal);
		
		s_sth.s_items[item_no].s_value = tax_cal.gros_amt;

		/* When PO is given Program cotrol will never go to the top
		   ACNO_FLD case. So, get the GL acnt */
		err = CheckGlAcnt(s_sth.s_items[item_no].s_accno) ;
		if(DBH_ERR == err) return(err) ;
		if(err < 0) {
			s_sth.s_items[item_no].s_value = LV_DOUBLE ;
			return(ERROR) ;
		}

		/* Cumulate the uncommited value of current account */
		diff = CumulativeUncommited(s_sth.s_items[item_no].s_accno) ;

		/* Add the current item Value */
		if(mode == ADD)
			diff += s_sth.s_items[item_no].s_net_amt ;
		else
			diff += (s_sth.s_items[item_no].s_net_amt -
					CurPage->Items[item_no].s_net_amt) ;
		/* Find out remaining budget */
		temp = gl_rec.budcur - gl_rec.comdat - gl_rec.ytd ;
		if(s_sth.s_items[item_no].s_value > DELTA_AMT && temp < diff) {
#ifdef ENGLISH
			DispError("Not Enough Budget");
#else
			DispError("Pas assez de budget");
#endif
#ifdef ENGLISH
			STRCPY(s_sth.s_mesg,"Press ESC-F to Terminate");
#else
			STRCPY(s_sth.s_mesg,"Appuyer sur ESC-F pour terminer");
#endif
			ShowMesg();
		}
		sr.nextfld = sr.endfld;
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
	sr.nextfld = sr.curfld;
	return(NOERROR) ;
}	/* ItemsValidation() */
/*----------------------------------------------------------------*/
/* Check the given funds availability in file */

CheckFund(fund)
short	fund;
{
	int	err ;

	ctl_rec.fund = fund ;
	err = get_ctl(&ctl_rec, BROWSE, 0, e_mesg) ;
	if(ERROR == err) return(err);
	if(err < 0) {
		fomer(e_mesg) ;
		return(ERROR) ;
	}

	err = get_lastpo(&lastpo_rec, BROWSE, 1, e_mesg);
	if(ERROR == err) return(err);
	if(err < 0) {
		fomer(e_mesg) ;
		return(ERROR) ;
	}

	return(NOERROR) ;
}	/* CheckFund() */
/*----------------------------------------------------------------*/
/* Check the given GL account availability in file */

CheckGlAcnt(accno)
char	*accno ;
{
	int	err ;

	gl_rec.funds = s_sth.s_fund ;
	STRCPY( gl_rec.accno, accno);
	gl_rec.reccod = 99;
	err = get_gl(&gl_rec, UPDATE, 0, e_mesg) ;
	if(ERROR == err) return(err);
	if(err < 0) {
		fomer(e_mesg) ;
		return(ERROR) ;
	}
	return(NOERROR) ;
}	/* CheckGlAcnt() */
/*----------------------------------------------------------------*/
/* Cumulate the umcommitted po value of the the given account in items */

double
CumulativeUncommited(accno)
char	*accno ;
{
	double	diff ;
	Page	*temppage ;
	int	i ;

	diff = 0.0 ;

	if(PoLast != NULL)
	    for(temppage = FirstPage ; temppage != NULL ;
						temppage = temppage->NextPage) {

		for(i = 0 ; i < temppage->NoItems ; i++) {
			/* If not Same account, skip the item */
			if(strcmp(accno, temppage->Items[i].s_accno)) continue ;
			diff += temppage->Items[i].s_net_amt ;
		}

		/* IF the process is done upto Current Po Last Page,
		   then break */
		if(temppage == PoLast) break ;
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

		STRCPY(supplier.s_supp_cd, s_sth.s_supp_cd) ;
		err = get_supplier(&supplier, UPDATE, 0, e_mesg) ;
		if(ERROR == err) return(DBH_ERR) ;
		if(err < 0) {
			fomer(e_mesg) ;
			s_sth.s_supp_cd[0] = LV_CHAR ;
			return(ERROR) ;
		}
		break ;
	case	PAYEE_FLD	:	/* Supplier Cd: */
		err = supp_hlp(s_sth.s_payee_cd, 7, 13 );
		if(err == DBH_ERR) return(err) ;
		if(err >=0 ) redraw();
		if(err < 1) return(ERROR) ;

		STRCPY(supplier.s_supp_cd, s_sth.s_payee_cd) ;
		err = get_supplier(&supplier, BROWSE, 0, e_mesg) ;
		if(ERROR == err) return(DBH_ERR) ;
		if(err < 0) {
			fomer(e_mesg) ;
			s_sth.s_payee_cd[0] = LV_CHAR ;
			return(ERROR) ;
		}
		break ;
	case SHIPTO_FLD :		/* Ship To Fields */
		err = sch_hlp(&s_sth.s_shipto, 7, 13 );
		if(err == DBH_ERR) return(err) ;
		if(err >=0 ) redraw();
		if(err < 1) return(ERROR) ;	/* Not Selected */
		break ;
	default :
#ifdef ENGLISH
		fomer("No Help Window For This Field");
#else
		fomer("Pas de fenetre d'assistance pour ce champ");
#endif
	}	/* Switch sr.curfld */

	return(NOERROR) ;
}	/* HdrAndKeyWindowHelp() */
/*----------------------------------------------------------------*/
/* Show Help Windows, if applicable, when user gives ESC-H in Item fields */

ItemsWindowHelp()
{
	int	err ;
	int	fld_no, item_no ;
	int	cur_fld;
	short	reccod ;

	/* Calculate item# ans Fld# within item */
	item_no = (sr.curfld - ITEM_ST_FLD) / STEP ;
	fld_no  = (sr.curfld - ITEM_ST_FLD) % STEP ;

	switch(fld_no){
	case STCKCD_FLD :		/* Stock Code Fields */
		/* If Direct Disbursement, No Window */
		if(s_sth.s_type[0] == DIRECT){
			 break ; 
		}

		err = stock_hlp(s_sth.s_fund, s_sth.s_items[item_no].s_stck_cd,
								7, 13 );
		if(err == DBH_ERR) return(err) ;
		if(err >=0 ) redraw();
		if(err < 1) return(ERROR) ;	/* Not Selected */
		
		err = CheckStock(s_sth.s_items[item_no].s_stck_cd);
		if(err == DBH_ERR) return(err);
		if(err < 0) {
			s_sth.s_items[item_no].s_stck_cd[0] = LV_CHAR ;
			return(ERROR) ;
		}
		/** added for description ****/
		cur_fld = ITEM_ST_FLD + (STEP * item_no) ;
		cur_fld += DESC_FLD ;
		STRCPY( s_sth.s_items[item_no].s_desc, stck_rec.st_desc);
		SetDupBuffers(cur_fld,cur_fld,2);

		s_sth.s_items[item_no].s_desc[0] = LV_CHAR ;
		sr.curfld = (ITEM_ST_FLD + (STEP * item_no) ) + STCKCD_FLD ;
		sr.endfld = (ITEM_ST_FLD + (STEP * item_no) ) + (STEP - 100) ;

		/** added for unit/cost ****/
		cur_fld = ITEM_ST_FLD + (STEP * item_no) ;
		cur_fld += PRICE_FLD ;
		s_sth.s_items[item_no].s_unit_cost = stck_rec.st_rate;
		SetDupBuffers(cur_fld,cur_fld,2);

		s_sth.s_items[item_no].s_unit_cost = LV_DOUBLE ;
		sr.curfld = (ITEM_ST_FLD + (STEP * item_no) ) + STCKCD_FLD ;
		sr.endfld = (ITEM_ST_FLD + (STEP * item_no) ) + (STEP - 100) ;

		/* Show the PO UOM as a default UOM on stock master */
		cur_fld = ITEM_ST_FLD + (STEP * item_no) ;
		cur_fld += UOM_FLD ;
		STRCPY( s_sth.s_items[item_no].s_uom, stck_rec.st_unit);
		SetDupBuffers(cur_fld,cur_fld,2);

		s_sth.s_items[item_no].s_uom[0] = LV_CHAR ;
		sr.curfld = (ITEM_ST_FLD + (STEP * item_no) ) + STCKCD_FLD ;
		sr.endfld = (ITEM_ST_FLD + (STEP * item_no) ) + (STEP - 100) ;

		fomer(stck_rec.st_desc);
		break ;
	case SCHOOL_FLD :		/* School Fields */
		err = sch_hlp(&s_sth.s_items[item_no].s_school, 7, 13 );
		if(err == DBH_ERR) return(err) ;
		if(err >=0 ) redraw();
		if(err < 1) return(ERROR) ;	/* Not Selected */
		break ;
	case ACNO_FLD :		/* GL Account# Fields */
		err = gl_hlp(s_sth.s_fund, s_sth.s_items[item_no].s_accno,
			&reccod, 7, 13 );
		if(err == DBH_ERR) return(err) ;
		if(err >=0 ) redraw();
		if(err < 1) return(ERROR) ;	/* Not Selected */
		if(reccod != 99) {
#ifdef ENGLISH
			fomer("Select records with 99 as Record Code only");
#else
			fomer("Choisir les fiches avec 99 seulement comme code de fiche");
#endif
			s_sth.s_items[item_no].s_accno[0] = LV_CHAR ;
			return(ERROR) ;
		}
		err = CheckGlAcnt(s_sth.s_items[item_no].s_accno) ;
		if(DBH_ERR == err) return(err) ;
		if(err < 0) {
			s_sth.s_items[item_no].s_accno[0] = LV_CHAR ;
			return(ERROR) ;
		}
		break ;
	default :
#ifdef ENGLISH
		fomer("No Help Window For This Field");
#else
		fomer("Pas de fenetre d'assistance pour ce champ");
#endif
	}	/* Switch sr.curfld */

	return(NOERROR) ;
}	/* ItemsWindowHelp() */
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
		err = GetOption("A(dd Items), E(dit), C(ancel)", "AEC");
#else
		err = GetOption("R(ajouter articles), M(odifier), A(nnuler)", "RMA");
#endif
		if(err == PROFOM_ERR) return(err) ;

		switch(err) {
		case  ADDITEMS :
			return(YES) ;
		case  EDIT  :
			err = HeaderEdit();
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
/*-----------------------------------------------------------------------*/
/* Take the confirmation from user for the header part */

ConfirmItems()
{
	int	err ;
	double 	save_value = 0.0;
	char	save_supp[11];
	char	save_payee[11];

	/* Options:
	   Add      - YAEHTNPC
	   Change   - YAEHTNPC
	   Inquiry  - YNPH
	*/

	for( ; ; ) {
	    switch(s_sth.s_fn[0]) {
	    case  ADDREC :		/* Add Po */
#ifdef ENGLISH
		err = GetOption(
		"Y(es), A(dd item), E(dit item), H(dr edit), T(otal), N(ext), P(rev), C(ancel)"
		,"YAEHTNPC");
#else
		err = GetOption(
		"O(ui), R(aj art), M(odifier), T(otal), E(n-tete), S(uiv), P(rec), A(nnul)"
		,"ORMTESPA");
#endif
		break ;
	    case  CHANGE :		/* Change Po */
#ifdef ENGLISH
		err = GetOption(
		"Y(es), A(dd item), E(dit item), H(dr edit), T(otal), N(ext), P(rev), C(ancel)"
		,"YAEHTNPC");
#else
		err = GetOption(
		"O(ui), R(aj art), M(odifier), T(otal), E(n-tete), S(uiv), P(rec), A(nnul)"
		,"ORMTESPA");
#endif
		break ;
	    case  INQUIRE :		/* Inquire Po */
#ifdef ENGLISH
		err = GetOption("Y(es), N(ext Page), P(rev Page)","YNP");
#else
		err = GetOption("O(ui), S(uivant), P(rec)","OSP");
#endif
		break ;
	    }
	    if(err == PROFOM_ERR) return(err) ;

	    switch(err) {
	    case  YES  :
		if(s_sth.s_fn[0] == ADDREC || s_sth.s_fn[0] == CHANGE) {
			err = CheckTotal();
			if (err == ERROR) break;  
		}
		return(YES);
	    case  ADDITEMS :
		err = AddItems() ;
		break ;
	    case  EDIT  :
		err = ChangeItems();
		break ;
	    case  HEADEREDIT:   /* added on Nov. 21, 1990,  F.Tao */
		err = HeaderEdit();
		break;	  
	    case  TOTAL:
		SetDupBuffers(POAMT_FLD,POAMT_FLD,1);
		save_value = s_sth.s_po_amt;
		s_sth.s_po_amt = LV_FLOAT ;
		err = ReadFields(POAMT_FLD,POAMT_FLD, KeyAndHdrValidation, HdrAndKeyWindowHelp,0);
		if(PROFOM_ERR == err || DBH_ERR == err)
			return(err);

		if (RET_USER_ESC == err) {
			/* copy back key Fields */
			s_sth.s_po_amt = save_value;
			ret(WriteFields(POAMT_FLD, POAMT_FLD));
		}
		/* Calculate and Display totals at Bottom of Screen */
		s_sth.s_total_po = s_sth.s_po_amt;
		s_sth.s_total_diff = s_sth.s_total_po - s_sth.s_total_itm;

		ret(WriteFields(TOTAL_ST_FLD,TOTAL_END_FLD));
		break;
	    case  NEXTPAGE :
		if(CurPage == PoLast || PoLast == NULL) {
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
		if(PoLast == NULL || CurPage == FirstPage ) {
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
	    }	/* switch err */

	    if(err == PROFOM_ERR) return(err) ;
	    if(err == DBH_ERR) return(err) ;
	}	/* for(; ; ) */
}	/* ConfirmItems() */
/*----------------------------------------------------------------------*/
/* Changing Items. Accept Item to be changed, deleted or revived.
   For change option read the item and others mark the Item */

ChangeItems()
{
	int	i ;

	/* Get The Item to Be Modified */

	for( ; ; ) {
#ifdef ENGLISH
		STRCPY(s_sth.s_mesg,"Enter RETURN to terminate Edit");
#else
		STRCPY(s_sth.s_mesg,"Appuyer sur RETURN pour terminer l'ajustement");
#endif
		ShowMesg() ;
		sr.nextfld = CHG_FLD ;
		fomrf( (char *)&s_sth );
		ret(err_chk(&sr));

		if(s_sth.s_field == 0) break ;
		if( s_sth.s_field < 1 || CurPage == NULL ||
				s_sth.s_field > CurPage->NoItems)
			continue ;

		if(CurPage->PoStatus[s_sth.s_field-1] == ' ' ||
				CurPage->PoStatus[s_sth.s_field-1]==COMPLETE) {
			po_item.pi_code = po_hdr.ph_code ;
			po_item.pi_item_no = CurPage->PoItemNo[s_sth.s_field-1];
			i = get_poitem(&po_item,UPDATE, 0, e_mesg);
			if(i < 0) {
				DispError(e_mesg);
				if(i == ERROR) return(DBH_ERR);
				return(ERROR) ;
			}
			if (s_sth.s_type[0] != DIRECT) {
				i = CheckStock(CurPage->Items[s_sth.s_field-1].s_stck_cd) ;
				if (i < 0) return(i) ;
			}
		}

		i = ReadItem((int)(s_sth.s_field-1), UPDATE) ;
		if(i == PROFOM_ERR) return(i) ;
		if(i == DBH_ERR) return(i) ;
		/* Copy the Item to List */

		scpy((char*)&(CurPage->Items[s_sth.s_field-1]),
			(char*)&(s_sth.s_items[s_sth.s_field-1]),
			sizeof(S_item)) ;
		if(CurPage->PoStatus[s_sth.s_field-1] == ' ') 
			CurPage->PoStatus[s_sth.s_field-1] = COMPLETE;
	}	/* for( ; ; ) */

	s_sth.s_field = HV_SHORT ;
	fomwf( (char *)&s_sth );
	ret(err_chk(&sr));

	s_sth.s_mesg[0] = HV_CHAR;
	ShowMesg();

	return(NOERROR);
}	/* ChangeItems() */

/*-----------------------------------------------------------------------*/
/* Change supplier on Po so move po amount from old supplier to new      */
SupplierChange() 
{
	int 	err ;

				/* Decrease old supplier ytd purchased */
	STRCPY(supplier.s_supp_cd, po_hdr.ph_supp_cd);
	err = get_supplier(&supplier, UPDATE, 0, e_mesg);
	if(err < 0) {
		DispError(e_mesg);
		return(DBH_ERR) ;
	}
	supplier.s_ytd_ord -= po_hdr.ph_comm ;
		/* round ytd ord before writing to file */
	supplier.s_ytd_ord = D_Roundoff(supplier.s_ytd_ord); 

	err = put_supplier(&supplier, UPDATE, e_mesg) ;
	if (err < 0) return(err) ;

				/* Increase new supplier ytd purchased */
	STRCPY(supplier.s_supp_cd, s_sth.s_supp_cd);
	err = get_supplier(&supplier, UPDATE, 0, e_mesg);
	if(err < 0) {
		DispError(e_mesg);
		return(DBH_ERR) ;
	}
	supplier.s_ytd_ord += po_hdr.ph_comm ;
		/* round ytd ord before writing to file */
	supplier.s_ytd_ord = D_Roundoff(supplier.s_ytd_ord); 

	err = put_supplier(&supplier, UPDATE, e_mesg) ;
	if (err < 0) return(err) ;

	return(NOERROR);
}
/*-----------------------------------------------------------------------*/
/* Display message and get the option */

GetOption(msg,options)
char	*msg ;
char	*options ;
{
	int	i, j ;

	STRCPY(s_sth.s_mesg,msg);
	ShowMesg() ;

	sr.nextfld = END_FLD ;
	for( ; ; ) {
		fomrf( (char *)&s_sth ) ;
		ret(err_chk(&sr)) ;

		j = strlen(options) ;
		for( i = 0 ; i < j ; i++)
			if(s_sth.s_resp[0] == options[i]) break ;
		if(i != j) break ;	/* Valid Option Selected */
#ifdef ENGLISH
		fomer("Invalid Option..");
#else
		fomer("Option invalide..");
#endif
	}

	s_sth.s_mesg[0] = HV_CHAR ;
	s_sth.s_resp[0] = HV_CHAR ;
	
	ret( WriteFields(END_FLD -100, END_FLD) );

	return((int)(options[i])) ;
}	/* GetOption() */
/*-----------------------------------------------------------------------*/
/* Move po header details to Screen Header part and read po items
   into linked list and show the Po first page on screen */

ShowPo()
{
	int	err ;
	
	if( CheckFund((short)1) <0 ) 	return(-1);

	/* Initialize totals to zero */
	s_sth.s_total_po = 0.00;
	s_sth.s_total_itm = 0.00;
	s_sth.s_total_diff = 0.00;

	/* Make Items Linked List from po */
	if((err = MakePoItemsList()) < 0) return(err) ;

	if((err = PoHdrtoScrHdr()) < 0) return(err) ;	/* Move Header Fields */
	/* Get Po total and calculate Difference Items total calculated in
	   MakePoItemsList() */
	s_sth.s_total_po = s_sth.s_po_amt;
	s_sth.s_total_diff = s_sth.s_total_po - s_sth.s_total_itm;	

	if((err = ShowItems(CurPage)) < 0) return(err) ;	/* Show Items */

	return(NOERROR) ;
}	/* ShowPo() */
/*-----------------------------------------------------------*/
/* Move Po Header to Screen Hdr Fields */

PoHdrtoScrHdr()
{
	/* Copy Key */
	s_sth.s_po_no = po_hdr.ph_code ;

	/* Copy Header Fields */
	STRCPY( s_sth.s_supp_cd, po_hdr.ph_supp_cd);
	STRCPY( s_sth.s_payee_cd, po_hdr.ph_payee);
	s_sth.s_entry_dt = po_hdr.ph_date ;
	s_sth.s_fund = po_hdr.ph_funds ;
	s_sth.s_po_amt = po_hdr.ph_comm ;
	s_sth.s_billto = po_hdr.ph_billto ;
	s_sth.s_due_dt = po_hdr.ph_due_date ;
	s_sth.s_period = po_hdr.ph_period ;
	s_sth.s_liq_amt = po_hdr.ph_lqamt ;
	s_sth.s_shipto = po_hdr.ph_shipto ;
	s_sth.s_type[0] = po_hdr.ph_type[0] ;
	s_sth.s_type[1] = '\0';
	s_sth.s_liq_dt = po_hdr.ph_lqdate ;
	STRCPY( s_sth.s_req_no, po_hdr.ph_req_no);
	s_sth.s_gstmsg[0] = po_hdr.ph_gstmsg[0] ;
	s_sth.s_gstmsg[1] = '\0';
	STRCPY( s_sth.s_attention, po_hdr.ph_attention);
	s_sth.s_status[0] = po_hdr.ph_status[0] ;
	s_sth.s_status[1] = '\0' ;

	ret( WriteFields( KEY_START, HDR_END_FLD) ) ;
	return(NOERROR) ;
}	/* PoHdrtoScrHdr() */
/*-----------------------------------------------------------*/
/* Read PO Items and make Linked list */

MakePoItemsList()	/* Make the linked list from PO Items */
{
	int	err, i ;

	po_item.pi_code = po_hdr.ph_code ;
	po_item.pi_item_no = 0;
	flg_reset(POITEM);	/* Initialize to get first rec under givenkey */

	PoLast = CurPage = NULL ;
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

		if(PAGESIZE == i) i = 0 ;
		if(0 == i)		/* 1st Item in the Page */
			if((err = MakeFreshPage()) < 0) return(err) ;

		CurPage->Items[i].s_sno = i + 1 ;
		CurPage->Items[i].s_school = po_item.pi_school ;
		STRCPY(CurPage->Items[i].s_accno, po_item.pi_acct) ;
		CurPage->Items[i].s_ord_qty = po_item.pi_orig_qty ;
		CurPage->Items[i].s_unit_cost = po_item.pi_unitprice ;

		if (po_item.pi_tax1[0] == TAXABLE) 
			gst_tax = ctl_rec.gst_tax;
		else{
		    	po_item.pi_tax1[0] = EXEMPT;
			gst_tax = 0.0;
		}
		if (po_item.pi_tax2[0] == TAXABLE) 
			pst_tax = ctl_rec.pst_tax;
		else{
		    	po_item.pi_tax2[0] = EXEMPT;
			pst_tax = 0.0;
		}
						
		Tax_Calculation(gst_tax,pst_tax,po_item.pi_value,&tax_cal);
	
		CurPage->Items[i].s_net_amt = po_item.pi_value;
		CurPage->Items[i].s_value = tax_cal.gros_amt; 
		STRCPY(CurPage->Items[i].s_stck_cd, po_item.pi_st_code);
		STRCPY(CurPage->Items[i].s_desc, po_item.pi_desc);
		CurPage->Items[i].s_act_del[0] = HV_CHAR;
/** Requisitions taken out 
		if (po_item.pi_req_no[0] == LV_CHAR) 
			STRCPY(CurPage->Items[i].s_req_no, "  ");
		else
			STRCPY(CurPage->Items[i].s_req_no, po_item.pi_req_no);
*/
		if (po_item.pi_unit[0] == LV_CHAR) 
			STRCPY(CurPage->Items[i].s_uom, "    ");
		else
			STRCPY(CurPage->Items[i].s_uom, po_item.pi_unit);
		STRCPY(CurPage->Items[i].s_gst, po_item.pi_tax1);
		STRCPY(CurPage->Items[i].s_pst, po_item.pi_tax2);
		CurPage->Items[i].s_os_qty = po_item.pi_orig_qty -
						po_item.pi_pd_qty ;
		Tax_Calculation(gst_tax,pst_tax,po_item.pi_paid,&tax_cal);
		CurPage->Items[i].s_pd_amount = tax_cal.gros_amt ;
		CurPage->PoItemNo[i] = po_item.pi_item_no ;
		CurPage->PoStatus[i] = ' ';

		/* Calculate Items Total */
		s_sth.s_total_itm += CurPage->Items[i].s_value;

		CurPage->NoItems++ ;
		i++ ;
	} 
	seq_over(POITEM) ;

	if(PoLast != NULL)
		CurPage = FirstPage ;

	return(NOERROR) ;
}	/* MakePoItemsList() */
/*-----------------------------------------------------------*/
/* Move Screen Hdr Fields to Po Header */

ScrHdrtoPoHdr(mode)
int	mode ;
{

	po_hdr.ph_code = s_sth.s_po_no ;

	STRCPY(po_hdr.ph_supp_cd, s_sth.s_supp_cd) ;
	STRCPY(po_hdr.ph_payee, s_sth.s_payee_cd) ;
	po_hdr.ph_billto = s_sth.s_billto ;
	po_hdr.ph_shipto = s_sth.s_shipto ;
	po_hdr.ph_date = s_sth.s_entry_dt ;
	po_hdr.ph_funds = s_sth.s_fund ;
	po_hdr.ph_comm = s_sth.s_po_amt ;
	po_hdr.ph_due_date = s_sth.s_due_dt ;
	po_hdr.ph_period = s_sth.s_period ;
	po_hdr.ph_lqdate = s_sth.s_liq_dt ;
	po_hdr.ph_type[0] = s_sth.s_type[0] ;
	STRCPY(po_hdr.ph_req_no, s_sth.s_req_no) ;
	po_hdr.ph_gstmsg[0] = s_sth.s_gstmsg[0] ;
	STRCPY(po_hdr.ph_attention, s_sth.s_attention) ;
	if (mode == ADD) 
		po_hdr.ph_print[0] = NO ;
	po_hdr.ph_lqamt = s_sth.s_liq_amt ;
/* Freight taken out 
	po_hdr.ph_freight = s_sth.s_freight ;
*/
	po_hdr.ph_status[0] = s_sth.s_status[0] ;
	
		/* round all calculated double variables before writing CL*/

	po_hdr.ph_comm = D_Roundoff(po_hdr.ph_comm); 
	po_hdr.ph_lqamt = D_Roundoff(po_hdr.ph_lqamt);  

	return(NOERROR) ;
}	/* ScrHdrtoPoHdr() */
/*------------------------------------------------------------------------*/
/* Move High values to all data fields and clear the screen */

ClearScreen()
{
	int	i;

	/* Move High Values to Hedaer part */
	InitHdr(HV_CHAR, HV_SHORT, HV_LONG, HV_DOUBLE) ;

	s_sth.s_page_no    = HV_SHORT ;
	s_sth.s_dummy1[0] = HV_CHAR ;

	/* Move High Values to All items */
	for(i = 0 ; i < PAGESIZE ; i++)
		InitItem(i, HV_CHAR, HV_SHORT, HV_DOUBLE, UPDATE) ;

	InitTotals(HV_DOUBLE);

	ret( WriteFields(HDR_ST_FLD, (END_FLD - 200)) );

	return(NOERROR);
}	/* ClearScreen() */
/*-------------------------------------------------------------------------*/
/* Initialize Screen Header with either Low or High Values */

InitHdr( t_char, t_short, t_long, t_double )
char	t_char ;
short	t_short ;
long	t_long ;
double	t_double ;
{
	s_sth.s_supp_cd[0] = t_char ;
	s_sth.s_payee_cd[0] = t_char ;
	s_sth.s_po_amt = t_double ;
	s_sth.s_entry_dt = t_long ;
	s_sth.s_due_dt = t_long ;
	s_sth.s_liq_amt = t_double ;
	s_sth.s_period = t_short ;
	s_sth.s_fund = t_short ;
	s_sth.s_liq_dt = t_long ;
	s_sth.s_shipto = t_short ;
	if(t_short == LV_SHORT) {
		/* If Inventory system not present, always Direct
		   Disbursement */
		if(pa_rec.pa_stores[0] == YES)
			s_sth.s_type[0] = t_char ;
		else
			s_sth.s_type[0] = DIRECT ;
	}
	else
		s_sth.s_type[0] = t_char ;

	s_sth.s_billto = t_short ;

	s_sth.s_attention[0] = t_char ;
	s_sth.s_req_no[0] = t_char ;
	s_sth.s_gstmsg[0] = t_char ;

	if (s_sth.s_fn[0] == ADDREC && t_char == LV_CHAR) 
		s_sth.s_status[0] = OPEN ;
	else
		s_sth.s_status[0] = t_char ;


	return(NOERROR) ;
}	/* InitHdr() */
/*-------------------------------------------------------------------------*/
/* Initialize Given screen item with either Low values or High values */

InitItem(item_no, t_char, t_short, t_double, mode)
int	item_no ;
char	t_char ;
short	t_short ;
double	t_double ;
int	mode ;
{

	if (mode == ADD || s_sth.s_fn[0] == ADDREC ) {
		s_sth.s_items[item_no].s_accno[0] = t_char ;
	}
	else if (t_short == HV_SHORT) {
			s_sth.s_items[item_no].s_accno[0] = t_char ;
	}

	if (s_sth.s_type[0] == DIRECT && t_short == LV_SHORT)  
		s_sth.s_items[item_no].s_stck_cd[0] = HV_CHAR ;
	else
		s_sth.s_items[item_no].s_stck_cd[0] = t_char ;

	s_sth.s_items[item_no].s_uom[0] = t_char ;

	if ((s_sth.s_type[0] == BULK || s_sth.s_type[0] == DIRECT) &&
		t_short == LV_SHORT)
			s_sth.s_items[item_no].s_school = HV_SHORT ;
	else
			s_sth.s_items[item_no].s_school = t_short ;

	s_sth.s_items[item_no].s_desc[0] = t_char ;
	/* Not used in po maintenance only po delete */
	s_sth.s_items[item_no].s_act_del[0] = HV_CHAR ;

	if(t_short == LV_SHORT) 
		s_sth.s_items[item_no].s_subhdg[0] = ' ';
	else
		s_sth.s_items[item_no].s_subhdg[0] = t_char;

	s_sth.s_items[item_no].s_gst[0] = t_char;
	s_sth.s_items[item_no].s_pst[0] = t_char;
	s_sth.s_items[item_no].s_ord_qty = t_double ;
	s_sth.s_items[item_no].s_unit_cost = t_double ;
	s_sth.s_items[item_no].s_net_amt = t_double ;
	s_sth.s_items[item_no].s_value = t_double ;
	

	if (t_short == HV_SHORT) {
		s_sth.s_items[item_no].s_sno = t_short ;
		s_sth.s_items[item_no].s_os_qty = t_double ;
		s_sth.s_items[item_no].s_pd_amount = t_double ;
	}
	else 
		if (mode == ADD) {
			s_sth.s_items[item_no].s_os_qty = 0.0 ;
			s_sth.s_items[item_no].s_pd_amount = 0.0 ;
		}

	return(NOERROR) ;
}	/* Inititem() */
/*-------------------------------------------------------------------------*/
/* Initialize Given screen item with either Low values or High values */
InitTotals(t_double)
double t_double;
{
	
	s_sth.s_total_po = t_double;
	s_sth.s_total_itm = t_double;
	s_sth.s_total_diff = t_double;

	return(NOERROR);
}	/* InitTotals() */
/*-------------------------------------------------------------------------*/

static
DispError(s)    /* show ERROR and wait */
char	*s;
{
	strncpy(s_sth.s_mesg,s, (sizeof(s_sth.s_mesg) - 1));
	ShowMesg();
#ifdef ENGLISH
	fomen("Press any key to continue");
#else
	fomen("Appuyer sur une touche pour continuer");
#endif
	get();
	s_sth.s_mesg[0] = HV_CHAR;
	ShowMesg();
	return(NOERROR);
}
/*-------------------------------------------------------------------------*/

ShowMesg()  /* shows or clears message field */
{
	sr.nextfld = END_FLD - 100;
	fomwf( (char *)&s_sth ) ;
}

/*-------------------------------------------------------------------------*/
HeaderEdit()
{

	int retval;
	int firstfld,lastfld;

#ifdef ENGLISH
	STRCPY(s_sth.s_mesg,"Enter RETURN to terminate Edit");
#else
	STRCPY(s_sth.s_mesg,"Appuyer sur RETURN pour terminer l'ajustement");
#endif
	ShowMesg();


	for( ; ; ){
		/* Read number of field to be changed */
		s_sth.s_field = LV_SHORT;
		retval = ReadFields(CHG_FLD, CHG_FLD,
			KeyAndHdrValidation, HdrAndKeyWindowHelp, 0) ;
		if( retval < 0) return(retval);
		if( retval==RET_USER_ESC ){
			s_sth.s_field = HV_SHORT;
			ret(WriteFields(CHG_FLD,CHG_FLD));
			break;
		}
		if(s_sth.s_field == 0) {
			s_sth.s_field = HV_SHORT;
			ret(WriteFields(CHG_FLD,CHG_FLD));
			break;
		}
		if(s_sth.s_field ==  1) {
		   if(s_sth.s_status[0] != OPEN || s_sth.s_liq_amt != 0.00) {
#ifdef ENGLISH
			fomer("Invoice Already Entered Cannot Change Supplier");
#else
			fomer("Facture deja entree. Ne peut pas changer le fournisseur");
#endif
			break;
		   }
		}

		switch(s_sth.s_field){
			case 1: 	/* Supplier code */
				firstfld = SUPPCD_FLD;
				lastfld = PAYEE_FLD;
				break;
			case 2:		/* Payee code */
				firstfld = lastfld = PAYEE_FLD;
				break;
			case 3: 	/* PO amount */
				firstfld = lastfld = POAMT_FLD;
				break;
			case 4: 	/* Entry date */
				firstfld = lastfld = ENTRYDT_FLD;
				break;
			case 5: 	/* Due date */
				firstfld = lastfld = DUEDT_FLD;
				break;
			case 6: 	/* Period */
				firstfld = lastfld = PERIOD_FLD;
				break;
			case 7: 	/* Fund */
				firstfld = lastfld = FUND_FLD;
				break;
			case 8: 	/* Ship To */
				firstfld = lastfld = SHIPTO_FLD;
				break;
			case 10:	/* Req No */
				if(s_sth.s_fn[0] == ADDREC) {
					firstfld = lastfld = REQ_FLD;
					break;
				}
				continue;
			case 11: 
				firstfld = lastfld = GSTMSG_FLD;
				break;
			case 12: 
				firstfld = lastfld = ATT_FLD;
				break;
#ifdef ENGLISH
			default: fomer("Can't change specified field");
#else
			default: fomer("Ne peut pas changer le champ specifie");
#endif
				continue;
		}
		retval = ModifyField(firstfld, lastfld);
		if( retval<0 )	return(retval);
		if( retval==RET_USER_ESC ){
			s_sth.s_field = HV_SHORT;
			ret(WriteFields(CHG_FLD,CHG_FLD));
			break;
		}
	}
	return(0);
}
ModifyField( firstfld, lastfld )	/* Read & change the specified fields */
int firstfld,lastfld;
{
	int i, retval;

	for( i=firstfld; i<=lastfld; i+=100 ){
		fomca1( i,19,2);	/* enable dup buffers */
	}
	sr.nextfld = firstfld;
	sr.endfld = lastfld;
	fomud( (char *)&s_sth);		/* Update dup buffers */
	/* Reset fields because fomud initializes endfld to 0 */
	sr.nextfld = firstfld;
	sr.endfld = lastfld;
	switch(firstfld){
		case SUPPCD_FLD:	/* Supplier code */
			s_sth.s_supp_cd[0] = LV_CHAR;
			s_sth.s_payee_cd[0] = LV_CHAR;
			break;
		case PAYEE_FLD:		/* Payee code */
			s_sth.s_payee_cd[0] = LV_CHAR;
			break;
		case POAMT_FLD:		/* PO amount */
			s_sth.s_po_amt = LV_DOUBLE;
			break;
		case ENTRYDT_FLD:	/* Entry date */
			s_sth.s_entry_dt = LV_LONG;
			break;
		case DUEDT_FLD:		/* Due date */
			s_sth.s_due_dt = LV_LONG;
			break;
		case PERIOD_FLD:	/* Period */
			s_sth.s_period = LV_SHORT;
			break;
		case FUND_FLD:		/* Fund */
			s_sth.s_fund = LV_SHORT;
			break;
		case SHIPTO_FLD:	/* Ship to */
			s_sth.s_shipto = LV_SHORT;
			break;
		case REQ_FLD:		/* Req No to */
			s_sth.s_req_no[0] = LV_CHAR;
			break;
		case GSTMSG_FLD:		/* Req No to */
			s_sth.s_gstmsg[0] = LV_CHAR;
			break;
		case ATT_FLD:	/* Attention field */
			s_sth.s_attention[0] = LV_CHAR;
			break;
		default:
			break;
	}
	retval = ReadFields(firstfld, lastfld,
		KeyAndHdrValidation, HdrAndKeyWindowHelp, UPDATE) ;
	if( retval<0 || retval==RET_USER_ESC )	return(retval);
	for( i=firstfld; i<=lastfld; i+=100 ){
		fomca1( i,19,0);	/* disable dup buffers */
		fomca1( i,10,1);	/* enable escape flag */
	}

	/* if changing amount field update totals on botton of screen */
	if(firstfld == POAMT_FLD && s_sth.s_total_po != HV_DOUBLE) {
		/* Calculate and Display totals at Bottom of Screen */
		s_sth.s_total_po = s_sth.s_po_amt;
		s_sth.s_total_diff = s_sth.s_total_po - s_sth.s_total_itm;

		ret(WriteFields(TOTAL_ST_FLD,TOTAL_END_FLD));
	}

	return(0);
}	/* ModifyField() */
/*-------------------- E n d   O f   P r o g r a m ---------------------*/
