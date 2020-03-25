/******************************************************************************
		Sourcename    : receipts.c
		System        : Budgetary Financial System.
		Subsystem     : Accounts Receivable System 
		Module        : Accounts Receivable 
		Created on    : 89-11-7
		Created  By   : J Prescott.
		Cobol sources : 

HISTORY:
 Date           Programmer     Description of modification
1990/11/23      J.Prescott     Added Applied/Unapplied receipts.
1990/12/18	F.Tao 	       Round up amounts before writing to file.
1991/01/24	F.Tao	       When updating "curcr" in GL, should post
			       negative amount.
1991/02/08	p.ralph	       Right justify numeric customer codes
1991/02/26	J.Prescott     Changed receipts file to have headers and items.
1992/07/27	J McLean       Changed customer code from 6 to 10 characters.
******************************************************************************/
#include <stdio.h>

#define MAIN
#define MAINFL		RCPTHDR		/* main file used */

#include <bfs_defs.h>
#include <bfs_recs.h>
#include <cfomstrc.h>

#define SYSTEM		"ACCOUNTS RECEIVABLE"
#define MOD_DATE	"23-NOV-90"
#define SCREEN_NAME	"receipts"
#define ESCAPE		12	/* flag indicates discontinuation of entries */
#define RANDOM  	18
#define SEQUENTIAL	19
#define	DELTA_DIFF	0.001
#define LOW 		-1
#define HIGH 		 1
#define HL_CHAR(VAL)	(VAL==HIGH) ? HV_CHAR : LV_CHAR
#define ESC_F		(sr.retcode==RET_USER_ESC && \
			(sr.escchar[0]=='F'||sr.escchar[0]=='f'))
#define ESC_H		(sr.retcode==RET_USER_ESC && \
			(sr.escchar[0]=='H'||sr.escchar[0]=='h'))
#define NO_HLP_WIN	(sr.curfld!=1600 && sr.curfld!=1100)

#ifdef ENGLISH
#define ADDREC		'A'
#define CHANGE 		'C'
#define DELETE		'D'
#define NEXT		'N'
#define PREV		'P'
#define INQUIRE		'I'
#define EXITOPT		'E'

#define EDIT		'E'
#define CANCEL		'C'
#define YES		'Y'
#define NO		'N'
#else	/* FRENCH */
#define ADDREC 		'R'
#define CHANGE 		'C'
#define DELETE		'E'
#define NEXT		'S'
#define PREV		'P'
#define INQUIRE		'I'
#define EXITOPT		'F'

#define EDIT		'M'
#define CANCEL		'A'
#define YES		'O'
#define NO		'N'
#endif

#define FUNC_FLD	400
#define REFNBR_FLD	500
#define FLDNO_FLD	600
#define FUND_FLD	800
#define APP_FLD		900
#define INVNBR_FLD	1000
#define CUST_FLD	1100
#define TRANDT_FLD	1200
#define CHEQNO_FLD	1300
#define AMT_FLD		1400
#define PERIOD_FLD	1500
#define ACCT_FLD	1600
#define REMARKS_FLD	1700
#define MESG_FLD	1800
#define RESP_FLD	1900

/* receipts.sth - header for C structure generated by PROFOM EDITOR */

struct	rcpt_struct	{
	char	s_progname[11];	/* 100 program name */
	long	s_rundt;	/* 300 system date */
	char	s_fn[2];	/* 400 function */
	long	s_refnumb;	/* 500 reference number */
	short	s_fld_no;	/* 600 field# field for editing */
	short	s_fund;		/* 800 fund number */
	char	s_applied[2];	/* 900 Applied / Unapplied */
	long	s_invnbr;	/* 1000 invoice number */
	char	s_customer[11]; /* 1100 customer number */
	long	s_transdt;	/* 1200 transaction date */
	char	s_cheque[16];	/* 1300 cheque number */
	double	s_amount;	/* 1400 amount */
	short	s_glperiod;	/* 1500 G/L period */
	char	s_acctno[19];	/* 1600 G/L account number */
	char	s_remarks[20];	/* 1700 remarks */
	char	s_mesg[78];	/* 1800 message field */
	char	s_resp[2];	/* 1900 response field */
};
struct rcpt_struct	s_rec;		/* screen record */
struct stat_rec 	sr;		/* profom status record */

static Rcpt_hdr    rcpt_hdr;		/* receipts file structure */
static Rcpt_item   rcpt_item;		/* receipts file structure */
static Ctl_rec	   ctl_rec;		/* fund/control file structure */
static Gl_rec	   gl_rec;		/* G/L file structure */
static Pa_rec      param_rec;		/* parameter file structure */
static Ar_hdr	   ar_hdr;		/* ARS header file structure */
static Tr_hdr	   tr_hdr;		/* gl transaction header */
static Tr_item     tr_item;		/* gl transaction item */
static Cu_rec	   cu_rec;		/* customer master file */

double	   D_Roundoff();

short reccod;
int retval;	/* Global variable to store function values */
char e_mesg[80]; /* to store error messages */
char Dbacct_no[19], Cracct_no[19];
short Dbacct_sect, Cracct_sect;
int  first_rec;	/* flag to see if first record added for date default */
long prev_date; /* the previous date entered by the user */
int	Argc;
char	**Argv;
/*char	tnum[5];	 Variable to hold the terminal number */

/* Initialize profom fields, call entry procedures */
main( argc, argv )
int argc;
char *argv[];
{
	strncpy( SYS_NAME, SYSTEM, 50 ); /* Module name */
	strncpy( CHNG_DATE, MOD_DATE, 10 );/* Last date of change */
	
	proc_switch( argc,argv,MAINFL );/* process the switches */

	Argc = argc;
	Argv = argv;

	retval = get_param(&param_rec, BROWSE, 1, e_mesg) ;
	if(retval < 1) {
		fomen(e_mesg);  get();
		close_dbh() ;
		exit(-1) ;
	}
	if( Initialize()<0 )	/* Initialize profom enviroment */
		exit(-1);
	
	retval = Process();	/* Interact with the user */
	CleanExit();
}

CleanExit()
{
	/* clear and exit the screen , close files & exit program */
	free_audit();	/* free memory allocated for writing audit rec */
	fomcs();
	fomrt();
	close_dbh();
	exit(retval);
}

Initialize()
{
	/* initialize the profom status variables */
	STRCPY( sr.scrnam, NFM_PATH );
	strcat( sr.scrnam, SCREEN_NAME );
	STRCPY( sr.termnm, terminal );

	/* initialize the fields and the profom screen */
	if( FillScrHdg()<0 ) 			return(-1);
	if( FillKeyFields( LOW )<0 ) 		return(-1);
	if( FillField( HIGH )<0 ) 		return(-1);
	if( FillNonKeyFlds( HIGH )<0 ) 		return(-1);
	if( FillMesgRespFlds( HIGH )<0 ) 	return(-1);
	if( InitProfom()<0 ){
		fomcs();
		fomrt();
		return(-1);
	}
	return(0);
}
/* initialize the profom and the screen */
InitProfom()
{
	fomin( &sr );			/* initialize profom */
	ret( err_chk(&sr) );		/* if profom error return */
	if( WriteFields(1,0)<0 )	/* Write all fields once */
		return(-1);
	fomcf(1,1);			/* Enable snap-screen option */
	return(0);
}
/* Fill the screen heading fields: the program name and the date */
FillScrHdg()
{
	STRCPY( s_rec.s_progname, PROG_NAME );
	s_rec.s_rundt = get_date();
	return(0);
}
/* Fill the keyfields with high or low values */
FillKeyFields( value )
short value;
{
	s_rec.s_refnumb = value * HV_LONG;

	return(0);
}
/* Fill the field# field with high/low values */
FillField( value )
short value;
{
	s_rec.s_fld_no = value * HV_SHORT;
	return(0);
}
/* Fill the non key fields with high/low values */
FillNonKeyFlds( value )
short value;
{
	s_rec.s_fund		= value * HV_SHORT;
	s_rec.s_applied[0] 	= HL_CHAR(value);
	s_rec.s_invnbr		= value * HV_LONG;
	s_rec.s_customer[0] 	= HL_CHAR(value);
	s_rec.s_transdt		= value * HV_LONG;
	s_rec.s_cheque[0] 	= HL_CHAR(value);
	s_rec.s_amount 		= value * HV_DOUBLE;
	s_rec.s_glperiod	= value * HV_SHORT;
	s_rec.s_acctno[0] 	= HL_CHAR(value);
	s_rec.s_remarks[0] 	= HL_CHAR(value);

	return(0);
}
/* Fill the message and response fields with high or low values */
FillMesgRespFlds( value )
short value;
{
	s_rec.s_mesg[0] = HL_CHAR(value);
	s_rec.s_resp[0] = HL_CHAR(value);
	return(0);
}
/* Accept user's option and call the corresponding routine in a loop */
Process()
{
	int retval;

	/* set first record flag to zero to indicate first record */
	/* This is used for the date default */
	first_rec = 0;

	/* Initialize the key fields to zeros. Used if seq. search is made */
	s_rec.s_refnumb = 0;

	for( ; ; ){
		if( ReadFunction()<0 ) return(-1);
		switch( s_rec.s_fn[0] ){
			case ADDREC:	/* add a record */
				if(param_rec.pa_cur_period == 0) {
#ifdef ENGLISH
				  fomer("Not Allowed Before Yearly Closing...");
#else
				  fomer("Pas permis avant la fermeture annuelle...");
#endif
				     get();
				     break;
				}
				CHKACC(retval,ADD,e_mesg);
				retval = AddRecord();
				roll_back(e_mesg);
				if( retval<0 )
					return(-1);
				break;
/****
			case CHANGE:	* change to applied *
				CHKACC(retval,UPDATE,e_mesg);
				if(ChangeApplied()<0) return(-1);
				break;
****/
			case NEXT:	/* show next record in sequence */
				CHKACC(retval,BROWSE,e_mesg);
				if( Inquiry(SEQUENTIAL,FORWARD)<0 ) return(-1);
				break;
			case PREV:	/* show prev record in sequence */
				CHKACC(retval,BROWSE,e_mesg);
				if( Inquiry(SEQUENTIAL,BACKWARD)<0 ) return(-1);
				break;
			case INQUIRE:	/* show selected record */
				CHKACC(retval,BROWSE,e_mesg);
				if( Inquiry(RANDOM,0)<0 ) return(-1);
				break;
			case EXITOPT:	/* exit */
				return(0);
			default:
				break;
		}
		if( retval<0 ){
			fomen(e_mesg);
			get();
		}
		if( retval==DBH_ERR )
			return(retval);
	}
}
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
	fomud( (char *)&s_rec );
	ret( err_chk(&sr) );

	return( 0 );
}
ReadFunction()	/* Display options at the bottom, and read entry */
{
/******
#ifdef ENGLISH
	fomer("A(dd), C(hange to applied), N(ext), P(rev), I(nquire), E(xit)");
#else
	fomer("R(ajouter), C(hanger a applique), S(uivant), P(recedent), I(nterroger), F(in)");
#endif
******/
#ifdef ENGLISH
	fomer("A(dd), N(ext), P(rev), I(nquire), E(xit)");
#else
	fomer("R(ajouter), S(uivant), P(recedent), I(nterroger), F(in)");
#endif
	sr.nextfld = FUNC_FLD;	/* Fn field number */
	fomrf( (char *)&s_rec );
	ret( err_chk(&sr) );
	return(0);
}
/* Add a stock master record */
AddRecord()
{
	if( SetDupBuffers(REFNBR_FLD,REMARKS_FLD,0)<0 )
		return(-1);
	if( ClearScreen()<0 )	return(-1);
	if( (retval=RdKeyFlds())<0 )	/* Read key fields */
		return(retval);
	if(retval==ESCAPE){
		if(ClearScreen()<0)	
			return(-1);
		return(retval);
	}
	
	if(first_rec == 0) {
		s_rec.s_transdt = s_rec.s_rundt;
		prev_date = s_rec.s_transdt ;
		first_rec = 1; /* not first record any more */
	}
	else 
		s_rec.s_transdt = prev_date;
	WriteFields(TRANDT_FLD,TRANDT_FLD);
/* 	save_nextfld = sr.nextfld;
	save_endfld = sr.endfld;
*/
	SetDupBuffers(TRANDT_FLD,TRANDT_FLD,2);
/*
	s_rec.s_transdt = LV_LONG;
	sr.nextfld = save_nextfld;
	sr.endfld = save_endfld;     */

	if( FillNonKeyFlds(LOW)<0 ) 	/* Prepare to read other fields */
		return(-1);

	/* Read the non key fields now */
	fund_default();
	if( (retval=ReadFields(FUND_FLD,REMARKS_FLD))<0 )
		return(retval);
	if(retval==ESCAPE){
		if(ClearScreen()<0)	return(-1);
		return(retval);
	}

	/* Allow the user to edit entries, if required, before saving */
	if( (retval=EditFlds())<0 )	return(retval);
	if( retval==ESCAPE ){
		if(ClearScreen()<0)	return(-1);
		return(retval);
	}

	/* Write the record, in ADD mode, to receipts master file */
	retval = PostReceipts();
	unlock_file(GLTRHDR);
	close_file(GLTRHDR);	

	return(retval);
}
/* Read stock master key fields, the fund and stock item code */
RdKeyFlds()
{
	if( FillKeyFields(LOW)<0 ) return(-1);
	return ( ReadFields(REFNBR_FLD,REFNBR_FLD) );
}
static
ReadFields( start,end )	/* Read the given range of fields */
{
	sr.nextfld = start;
	sr.endfld = end;
	for( ; ; ){	/* Do in a loop */
		fomrd( (char *)&s_rec );	/* Profom call */
		ret(err_chk(&sr));		/* Check for profom error */
		if( sr.retcode==RET_USER_ESC || sr.retcode==RET_VAL_CHK ){
		    if( sr.retcode==RET_USER_ESC ){
			if(( sr.escchar[0]=='F' || sr.escchar[0]=='f') && s_rec.s_resp[0] != EDIT)
				return( ESCAPE );
			else if( sr.escchar[0]=='H' || sr.escchar[0]=='h' ){
				if( NO_HLP_WIN )
					continue;
			}
			else
				continue;
		    }
		    retval=Validate();
		    if(retval<0 || retval==ESCAPE)
			return(retval);
		    else
			continue;
		}
		break;
	}
	return(0);
}
static
WriteFields( start,end )	/* write the given range of fields */
int start, end;			/* start & end profom field numbers */
{
	sr.nextfld = start;
	sr.endfld = end;
	fomwr( (char *)&s_rec );
	ret( err_chk(&sr) );
	return(0);
}
ClearScreen()	/* clear the screen except fn field key flds & screen heading */
{
	if(FillField(HIGH)<0 ) return(-1);
	if(FillNonKeyFlds(HIGH)<0 ) return(-1);
	if(FillMesgRespFlds(HIGH)<0 ) return(-1);

	if( WriteFields(FLDNO_FLD,MESG_FLD)<0 ) return(-1);
	
	return(0);
}
Validate()	/* Validate the values entered by the user */
{
	int index;
	int save_nextfld, save_endfld;

	switch( sr.curfld ){
		case REFNBR_FLD:	/* reference number  */
			if( s_rec.s_fn[0]==ADDREC ){
				if( s_rec.s_refnumb == 0 ){
#ifdef ENGLISH
					fomer("Invalid Reference number");
#else
					fomer("Numero de fiche invalide");
#endif
					s_rec.s_refnumb = LV_LONG;
					break;
				}
			}
			rcpt_hdr.rhdr_refno = s_rec.s_refnumb;
			index = get_rcpthdr( &rcpt_hdr, BROWSE, 0, e_mesg );
			if( index==ERROR ){
				fomen(e_mesg); get();
				return(-1);
			}
			if( s_rec.s_fn[0]==ADDREC ){
				if( index!=UNDEF ){
#ifdef ENGLISH
					fomer("Record already exists");
#else
					fomer("Fiche existe deja");
#endif
					s_rec.s_refnumb = LV_LONG;
				}
			}
			else{
			    if ( index!=NOERROR ){
				fomer(e_mesg);
				s_rec.s_refnumb = LV_LONG;
			    }
			}
			break;
		case FLDNO_FLD:	/* field # for editing */
			if( sr.fillcode==FIL_OMITTED )	/* Nothing entered */
				return(ESCAPE);
			break;
		case FUND_FLD:	/* fund # */
			ctl_rec.fund = s_rec.s_fund ;
			retval = get_ctl(&ctl_rec, BROWSE, 0, e_mesg) ;
			if(retval < 0) {
				fomer(e_mesg) ;
				get();
				s_rec.s_fund = LV_SHORT ;
			}
			STRCPY(Cracct_no,ctl_rec.ar_acnt);
			break; 
		case APP_FLD:
			if(s_rec.s_applied[0] != APPLIED &&
				s_rec.s_applied[0] != UNAPPLIED) {
#ifdef ENGLISH
				fomer("Valid options are A(pplied) or U(napplied)");
#else
				fomer("Options valables sont A(pplique) ou N(on-applique)");
#endif
				s_rec.s_applied[0] = LV_CHAR;
				break;
			}
			/* if unapplied don't accept invoice number */
			if(s_rec.s_applied[0] == UNAPPLIED) {
				s_rec.s_invnbr = 0;

			}
			break;
		case INVNBR_FLD:	/* invoice number */
			ar_hdr.ah_fund = s_rec.s_fund;
			ar_hdr.ah_inv_no = s_rec.s_invnbr ;
			ar_hdr.ah_sno = 1;
			retval = get_arhdr(&ar_hdr, BROWSE, 0, e_mesg) ;
			if(retval < 0) {
				fomer(e_mesg) ; get();
				s_rec.s_invnbr = LV_LONG ;
			}
			if(ar_hdr.ah_balance == 0.00 || 
					ar_hdr.ah_status[0] == COMPLETE) {
#ifdef ENGLISH
				fomer("Invoice is already complete");
#else
				fomer("Facture est deja complete");
#endif
				s_rec.s_invnbr = LV_LONG;
			}
			else {
				save_nextfld = sr.nextfld;
				save_endfld = sr.endfld;
				STRCPY(s_rec.s_customer,ar_hdr.ah_cu_code);
				SetDupBuffers(CUST_FLD,CUST_FLD,2);
				sr.nextfld = save_nextfld;
				sr.endfld = save_endfld;

			strcpy( cu_rec.cu_code, s_rec.s_customer );
			retval = get_cust( &cu_rec,UPDATE,0,e_mesg );
			if( retval==ERROR ){
				fomen(e_mesg);
				get();
				return( retval );
			}
			else
				fomer(cu_rec.cu_name);
			}
			break;
		case CUST_FLD:		/* customer number */
			Right_Justify_Numeric(s_rec.s_customer,
					sizeof(s_rec.s_customer)-1);
			if( ESC_H ){
			   retval = cust_hlp(s_rec.s_customer, 7, 15 );
			   if( retval<0 )	/* error */
					return( retval );
			   if( retval>=0 )
					redraw(); /* remove hlp window*/
			   if( retval==0 ) 	/* nothing selected */
				break;
			}
			strcpy( cu_rec.cu_code, s_rec.s_customer );
			retval = get_cust( &cu_rec,UPDATE,0,e_mesg );
			if( retval==ERROR ){
				fomen(e_mesg);
				get();
				return( retval );
			}	
			else if( retval==UNDEF ){
			 retval = GetOpt(
#ifdef ENGLISH
			 "Customer not found. Want to add one (Y/N)?","YN");
#else
			 "Client pas retrouve. Desirez-vous en ajouter un (O/N)?","ON");
A
#endif
				if( retval<0 )	return(retval);
				if( retval==YES ){
				   retval=execute( "customer", Argc, Argv );
				   if( retval<0 )
					return( retval );
				   redraw();
				}
				s_rec.s_customer[0] = LV_CHAR;
			}
			else if( retval!=NOERROR ){
				fomer( e_mesg );
				s_rec.s_customer[0] = LV_CHAR;
			}
			else
				fomer(cu_rec.cu_name);
			break;

		case TRANDT_FLD:	/* receipt date */
			if( s_rec.s_transdt> s_rec.s_rundt ){
#ifdef ENGLISH
				fomer("Date can't exceed current date");
#else
				fomer("Date ne peut pas etre plus tard que la date courante");
#endif
				s_rec.s_transdt = LV_LONG;
			}
			if( s_rec.s_transdt == 0 ){
#ifdef ENGLISH
				fomer("Date can't be zero");
#else
				fomer("Date ne peut pas etre zero");
#endif
				s_rec.s_transdt = LV_LONG;
			}
		/*	if(s_rec.s_transdt < ar_hdr.ah_trandt) {
#ifdef ENGLISH
			   fomer("Receipt date cannot precede Invoice date");
#else
			   fomer("Date du recu ne peut pas preceder la date de la facture");
#endif
				s_rec.s_transdt = LV_LONG;
			}
		*/
			prev_date = s_rec.s_transdt;
			break;
		case AMT_FLD:	/* receipt amount */
/*****			if(s_rec.s_applied[0] == APPLIED) {
				if(s_rec.s_amount > ar_hdr.ah_balance) {

#ifdef ENGLISH
					sprintf(e_mesg,
			"Receipt: %.2lf, Outstanding: %.2lf, Difference: %.2lf",
				s_rec.s_amount,ar_hdr.ah_balance,
				s_rec.s_amount-ar_hdr.ah_balance);
#else
				sprintf(e_mesg,
			"Recu: %.2lf, Non-Regle: %.2lf, Difference: %.2lf",
				s_rec.s_amount,ar_hdr.ah_balance,
				s_rec.s_amount-ar_hdr.ah_balance);
#endif
					fomer(e_mesg);
				 	get();
					s_rec.s_amount = LV_DOUBLE;
				}
			} *****/
			if(s_rec.s_amount == 0 ) {
#ifdef ENGLISH
				fomer("Receipt amount cannot be zero");
#else
				fomer("Montant du recu ne peut pas etre zero");
#endif
				get();
				s_rec.s_amount = LV_DOUBLE;
			}
			/* set default period */
			s_rec.s_glperiod = param_rec.pa_cur_period;
			SetDupBuffers(PERIOD_FLD,PERIOD_FLD,1);
			s_rec.s_glperiod = LV_SHORT;
			sr.nextfld = AMT_FLD;
			sr.endfld = REMARKS_FLD;
			break;
		case PERIOD_FLD:	/* G/L period */
			if( s_rec.s_glperiod < 1 || s_rec.s_glperiod < 
			   param_rec.pa_cur_period-param_rec.pa_open_per ||
				s_rec.s_glperiod > param_rec.pa_cur_period) {
#ifdef ENGLISH
					fomer("Period Invalid or not open");
#else
					fomer("Periode invalide ou pas ouverte");
#endif
					s_rec.s_glperiod = LV_SHORT;
			}
			else {
				if(s_rec.s_resp[0] != EDIT) {
				     STRCPY(s_rec.s_acctno,ctl_rec.bank1_acnt);
				     SetDupBuffers(ACCT_FLD,ACCT_FLD,1);
				     s_rec.s_acctno[0] = LV_CHAR;
				     sr.nextfld = PERIOD_FLD;
				     sr.endfld = REMARKS_FLD;
				}
			}
			break;
		case ACCT_FLD:	/* G/L account */
			if( ESC_H ){
			   retval = gl_hlp(s_rec.s_fund, s_rec.s_acctno, &reccod, 7, 13 );
			   if(retval == DBH_ERR) return(retval) ;
			   if(retval >=0 ) redraw();
			   if(retval < 0) return(ERROR) ;	/* Not Selected */
			   if(reccod != 99 && retval != 0) {
#ifdef ENGLISH
				fomer("Select records with 99 as Record Code only");
#else
				fomer("Choisir les fiches avec 99 comme code de fiche seulement");
#endif
				s_rec.s_acctno[0] = LV_CHAR ;
				break ;
			   }
			   if(retval == 0) break;
			}
			/* check if account # is numeric */
			if( acnt_chk(s_rec.s_acctno)==ERROR){
#ifdef ENGLISH
				fomer("Invalid Account number");
#else
				fomer("Numero de compte invalide");
#endif
				s_rec.s_acctno[0]=LV_CHAR;
				break;
			}
			ctl_rec.fund = s_rec.s_fund ;
			retval = get_ctl(&ctl_rec, BROWSE, 0, e_mesg) ;
			if(retval < 0) {
				fomer(e_mesg) ;
				get();
				return( retval );
			}
			if( strcmp(s_rec.s_acctno,ctl_rec.bank1_acnt) &&
			    strcmp(s_rec.s_acctno,ctl_rec.bank2_acnt)  ){
#ifdef ENGLISH
				fomer("Not a Bank Account");
#else
				fomer("N'est pas un compte de banque");
#endif
				s_rec.s_acctno[0]=LV_CHAR;
				break;
			}
			/* Check if Gl master record exists */
			gl_rec.funds = s_rec.s_fund;
			STRCPY( gl_rec.accno,s_rec.s_acctno );
			gl_rec.reccod = 99;
			retval = get_gl( &gl_rec,BROWSE,0,e_mesg );
			if(retval!=NOERROR ){
				fomen(e_mesg);
				get();
				s_rec.s_acctno[0]=LV_CHAR;	
			}
			else	fomer(gl_rec.desc);
			SetDupBuffers(PERIOD_FLD,PERIOD_FLD,0);
			sr.nextfld = PERIOD_FLD;
			sr.endfld = REMARKS_FLD;
			STRCPY(Dbacct_no,s_rec.s_acctno);
			break;
		default:
			break;
	}
	sr.nextfld = sr.curfld;
	return(0);
}
EditFlds()	/* Ask if user wants to edit fields before saving */
{
	for( ; ; ){
#ifdef ENGLISH
		if(DisplayMessage( "E(dit), C(ancel), Y(es) ")<0 ) 
#else
		if(DisplayMessage( "M(odifier), A(nnuler), O(ui) ")<0 ) 
#endif
			return(-1);
		sr.nextfld = RESP_FLD;
		fomrf( (char *)&s_rec );
		ret( err_chk(&sr) );
		switch( s_rec.s_resp[0] ){
			case EDIT:	/* Edit the fields */
				if( (retval=FldEdit())<0 )
					return(retval);
				break;
			case CANCEL:	/* Cancel the session */
#ifdef ENGLISH
				if( DisplayMessage("Confirm the cancel (Y/N)?")<0 )
#else
				if( DisplayMessage("Confirmer l'annulation (O/N)?")<0 )
#endif
					return(-1);
				for(;;){
					sr.nextfld = RESP_FLD;	/* response field */
					fomrf( (char *)&s_rec );
					ret( err_chk(&sr) );
					if(s_rec.s_resp[0] == YES ||
					   s_rec.s_resp[0] == NO)   break;
#ifdef ENGLISH
					fomer("Invalid Option..");
#else
					fomer("Option invalide..");
#endif
				}
					if( s_rec.s_resp[0]==YES ){
						if(HideMessage()<0) return(-1);
						return(ESCAPE);
					}
				break;
			case YES:
				if( HideMessage()<0 ) return(-1);
				return(0);
		}
	}
}
DisplayMessage(mesg)	/* Display the given message in the message field */
char *mesg;
{
	STRCPY( s_rec.s_mesg, mesg );
	if( WriteFields(MESG_FLD,MESG_FLD)<0 )	return(-1);
	return(0);
}
HideMessage()	/* Hide the message & response fields */
{
	if( FillMesgRespFlds(HIGH)<0 )	return(-1);
	if( WriteFields(MESG_FLD,RESP_FLD)<0 ) return(-1);
	return(0);
}
HideFldNo()	/* Hide the 'Field#' field */
{
	if( FillField(HIGH)<0 )	return(-1);
	if( (WriteFields(FLDNO_FLD,FLDNO_FLD))<0 )
		return(-1);
	return(0);
}
FldEdit()	/* Read the field number and read corresponding field */
{
	int firstfld,lastfld;

	for( ; ; ){
		/* Read number of field to be changed */
		if( FillField(LOW)<0 )	return(-1);
		if( (retval = ReadFields(FLDNO_FLD,FLDNO_FLD))<0 ) return(-1);
		if( retval==ESCAPE ){
			if( HideFldNo()<0 ) return(-1);
			break;
		}
		switch(s_rec.s_fld_no){
			case 1: 	/* fund */
				firstfld = lastfld = FUND_FLD;
				break;
			case 2: 	/* applied */
				firstfld = APP_FLD;
				lastfld = REMARKS_FLD;
				break;
			case 3: 	/* Invoice number */
			/*	firstfld = INVNBR_FLD;
				lastfld = REMARKS_FLD;*/
				if ( s_rec.s_applied[0] == UNAPPLIED){
#ifdef ENGLISH
					fomer("Cannot edit this field for unapplied receipt");
#else
					fomer("Ne peut pas changer ce champ pour un recu non-applique");
#endif	
					continue;
				}
				firstfld = lastfld=INVNBR_FLD;
				break;
			case 4: 	/* customer */
				if ( s_rec.s_applied[0] == APPLIED){
#ifdef ENGLISH
					fomer("Cannot edit this field for applied receipt");
#else
					fomer("Ne peut pas changer ce champ pour un recu applique");
#endif	
					continue;
				}
				firstfld = lastfld=CUST_FLD;
				break;
			case 5: 	/* transaction date */
				firstfld = lastfld = TRANDT_FLD;
				break;
			case 6: 	/* cheque number */
				firstfld = lastfld = CHEQNO_FLD;
				break;
			case 7: 	/* amount */
				firstfld = lastfld = AMT_FLD;
				break;
			case 8: 	/* G/L period */
				firstfld = lastfld = PERIOD_FLD;
				break;
			case 9: 	/* G/L acct # */
				firstfld = lastfld = ACCT_FLD;
				break;
			case 10: 	/* remarks */
				firstfld = lastfld = REMARKS_FLD;
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
		if( retval==ESCAPE ){
			if( HideFldNo()<0 ) return(-1);
			break;
		}
	}
	return(0);
}
ModifyField( firstfld, lastfld )	/* Read & change the specified fields */
int firstfld,lastfld;
{
	int i;

	for( i=firstfld; i<=lastfld; i+=100 ){
		fomca1( i,19,2);	/* enable dup buffers */
	}
	sr.nextfld = firstfld;
	sr.endfld = lastfld;
	fomud( (char *)&s_rec);		/* Update dup buffers */
	/* Reset fields because fomud initializes endfld to 0 */
	sr.nextfld = firstfld;
	sr.endfld = lastfld;
	switch(firstfld){
		case FUND_FLD:	/* fund */
			s_rec.s_fund = LV_SHORT;
			break;
		case APP_FLD:	/* applied/unapplied */
			s_rec.s_applied[0] = LV_CHAR;
			s_rec.s_invnbr = LV_LONG;
			s_rec.s_customer[0] = LV_CHAR;
			s_rec.s_transdt = LV_LONG;
			s_rec.s_cheque[0] = LV_CHAR;
			s_rec.s_amount = LV_DOUBLE;
			s_rec.s_glperiod = LV_SHORT;
			s_rec.s_acctno[0] = LV_CHAR;
			s_rec.s_remarks[0] = LV_CHAR;
			break;
		case INVNBR_FLD:	/* invoice number */
			s_rec.s_invnbr = LV_LONG;
			s_rec.s_customer[0] = LV_CHAR;
			s_rec.s_transdt = LV_LONG;
			s_rec.s_cheque[0] = LV_CHAR;
			s_rec.s_amount = LV_DOUBLE;
			s_rec.s_glperiod = LV_SHORT;
			s_rec.s_acctno[0] = LV_CHAR;
			s_rec.s_remarks[0] = LV_CHAR;
			break;
		case CUST_FLD:	/* customer */
			s_rec.s_customer[0] = LV_CHAR;
			break;
		case TRANDT_FLD:	/* Transaction date */
			s_rec.s_transdt = LV_LONG;
			break;
		case CHEQNO_FLD:	/* cheque number */
			s_rec.s_cheque[0] = LV_CHAR;
			break;
		case AMT_FLD:	/* cheque number */
			s_rec.s_amount = LV_DOUBLE;
			break;
		case PERIOD_FLD:	/* period */
			s_rec.s_glperiod = LV_SHORT;
			break;
		case ACCT_FLD:	/* account number */
			s_rec.s_acctno[0] = LV_CHAR;
			break;
		case REMARKS_FLD:	/* remarks */
			s_rec.s_remarks[0] = LV_CHAR;
			break;
		default:
			break;
	}
	retval = ReadFields( firstfld, lastfld );
	if( retval<0 || retval==ESCAPE )	return(retval);
	for( i=firstfld; i<=lastfld; i+=100 ){
		fomca1( i,19,0);	/* disable dup buffers */
		fomca1( i,10,1);	/* enable escape flag */
	}
	return(0);
}
PostReceipts()  /* post journal entries for receipts */
{
	int	err;
	/* J. Prescott Sept. 29/92 Try to Lock GL Trans file until successful */
	for( ; ; ) {
		if((err = lock_file(GLTRHDR)) < 0) {
			if(err == LOCKED) { 
				continue;
			}
			DispError();
			roll_back(e_mesg);	/* Unlock the locked Records */
			return(err);
		}
		else break;
	}
	/***********************************************/

	if(WriteARSrcpt(ADD)<0) return(-1);
	if(WriteGlmast()<0) return(-1);
	/*if(get_tnum(tnum) < 0) return(-1);*/
	if(WriteTrhdr()<0) return(-1);
	if(WriteTritems()<0) return(-1);
	if(WriteARShdr()<0) return(-1);
	if(WriteCustomer()<0) return(-1);
	if( commit(e_mesg)<0 ){
		fomen(e_mesg);
		get();
		return(-1);
	}
}

WriteARSrcpt(mode)	/* Write the ars record */
int	mode;
{
	rcpt_hdr.rhdr_refno = s_rec.s_refnumb;
	rcpt_hdr.rhdr_fund = s_rec.s_fund;
	STRCPY(rcpt_hdr.rhdr_applied,s_rec.s_applied); 
	STRCPY( rcpt_hdr.rhdr_cust, s_rec.s_customer );
	rcpt_hdr.rhdr_rcptdate =  s_rec.s_transdt;
	STRCPY( rcpt_hdr.rhdr_chequeno, s_rec.s_cheque );
	rcpt_hdr.rhdr_amount = s_rec.s_amount;
	rcpt_hdr.rhdr_period = s_rec.s_glperiod ;
	STRCPY( rcpt_hdr.rhdr_acctno, s_rec.s_acctno );
	STRCPY( rcpt_hdr.rhdr_remarks, s_rec.s_remarks );
	
	retval = put_rcpthdr( &rcpt_hdr, mode, e_mesg );
	if( retval!=NOERROR ){
		fomen(e_mesg);get();
		roll_back(e_mesg);
		return(retval);
	}

	if(s_rec.s_applied[0] == APPLIED) {
		rcpt_item.ritm_refno = s_rec.s_refnumb;
		rcpt_item.ritm_invnumb = s_rec.s_invnbr;
		rcpt_item.ritm_seqno = 1;
		STRCPY( rcpt_item.ritm_cust, s_rec.s_customer );
		rcpt_item.ritm_amount = s_rec.s_amount;

		retval = put_rcptitem( &rcpt_item, mode, e_mesg );
		if( retval!=NOERROR ){
			fomen(e_mesg);get();
			roll_back(e_mesg);
			return(retval);
		}
	}
	return(0);
}

WriteTrhdr()
{
#ifdef	ORACLE
	long	get_maxsno(), sno ;
#endif
	long	sysdt ;

	tr_hdr.th_fund = s_rec.s_fund;
	tr_hdr.th_reccod = 99;
	tr_hdr.th_create[0] = 'G';
#ifndef	ORACLE
	tr_hdr.th_seq_no = HV_SHORT;
	retval = get_n_trhdr( &tr_hdr, BROWSE, 0, BACKWARD, e_mesg );
	seq_over( GLTRHDR );
	if( retval==ERROR ){
		DispError();
		roll_back(e_mesg);
		return(-1);
	}
	if( retval==EFL || 
	    tr_hdr.th_fund != s_rec.s_fund ||
	    tr_hdr.th_reccod != 99 ||
	    tr_hdr.th_create[0] != 'G' ){
		tr_hdr.th_fund = s_rec.s_fund;
		tr_hdr.th_reccod = 99;
		tr_hdr.th_create[0] = 'G';
		tr_hdr.th_seq_no = 1;
	}
	else
		tr_hdr.th_seq_no++;
#else
	sno = get_maxsno(GLTRHDR, (char*)&tr_hdr, 0, -1, e_mesg) ;
	if(sno < 0) {
		DispError();
		roll_back(e_mesg) ;
		return(-1);
	}
	tr_hdr.th_seq_no = sno + 1;
#endif
	/*STRCPY(tr_hdr.th_term, tnum);*/
	STRCPY( tr_hdr.th_userid, User_Id );
	tr_hdr.th_sys_dt = sysdt = get_date() ;
	tr_hdr.th_period = s_rec.s_glperiod;
	tr_hdr.th_date = s_rec.s_transdt;
	tr_hdr.th_debits = tr_hdr.th_credits = s_rec.s_amount;

	strcpy(tr_hdr.th_descr, s_rec.s_remarks);
	strcpy(tr_hdr.th_supp_cd, s_rec.s_customer);

	sprintf(tr_hdr.th_reference, "%d-%ld", s_rec.s_fund, s_rec.s_refnumb) ;
	tr_hdr.th_type[0] = '5';

	if( put_trhdr( &tr_hdr, ADD, e_mesg )<0 ){
		DispError();
		roll_back(e_mesg);
		return(-1);
	}
	return(0);
}
WriteTritems()
{
	tr_item.ti_fund = tr_hdr.th_fund;
	tr_item.ti_reccod = tr_hdr.th_reccod;
	tr_item.ti_create[0] = tr_hdr.th_create[0];
	tr_item.ti_seq_no = tr_hdr.th_seq_no;
	tr_item.ti_item_no = 1;
	/*STRCPY(tr_item.ti_term, tnum);*/
	tr_item.ti_sys_dt = tr_hdr.th_sys_dt;
	tr_item.ti_period = tr_hdr.th_period;
	STRCPY(tr_item.ti_accno,Dbacct_no);
	tr_item.ti_amount = s_rec.s_amount;
	tr_item.ti_status = 0;
	tr_item.ti_section = Dbacct_sect;
	if( put_tritem(&tr_item, ADD, e_mesg)<0 ){
		DispError();
		roll_back(e_mesg);
		return(-1);
	}

	tr_item.ti_item_no = 2;
	/*STRCPY(tr_item.ti_term, tnum);*/
	STRCPY(tr_item.ti_accno,Cracct_no);
	tr_item.ti_amount = -s_rec.s_amount;
	tr_item.ti_section = Cracct_sect;
	if( put_tritem(&tr_item, ADD, e_mesg)<0 ){
		DispError();
		roll_back(e_mesg);
		return(-1);
	}
}
WriteGlmast()
{
	double	tempamount;

	gl_rec.funds = s_rec.s_fund;
	STRCPY( gl_rec.accno, Dbacct_no );
	gl_rec.reccod = 99;
	retval = get_gl( &gl_rec, UPDATE, 0, e_mesg );
	if( retval!=NOERROR ){
		DispError();
		roll_back(e_mesg);
		return(-1);
	}
	Dbacct_sect = gl_rec.sect;  /* section of db acct : used in GL Xaction */
	if(s_rec.s_glperiod == param_rec.pa_cur_period)
		 gl_rec.curdb += s_rec.s_amount;
	gl_rec.curdb = D_Roundoff(gl_rec.curdb);
	gl_rec.ytd += s_rec.s_amount;
	gl_rec.ytd   = D_Roundoff(gl_rec.ytd);
	gl_rec.currel[s_rec.s_glperiod-1] += s_rec.s_amount;
	gl_rec.currel[s_rec.s_glperiod-1] =
				 D_Roundoff(gl_rec.currel[s_rec.s_glperiod-1]);
	retval = put_gl( &gl_rec, UPDATE, e_mesg );
	if( retval!=NOERROR ){
		DispError();
		roll_back(e_mesg);
		return(-1);
	}

	gl_rec.funds = s_rec.s_fund;
	STRCPY( gl_rec.accno, Cracct_no );
	gl_rec.reccod = 99;
	retval = get_gl( &gl_rec, UPDATE, 0, e_mesg );
	if( retval!=NOERROR ){
		DispError();
		roll_back(e_mesg);
		return(-1);
	}
	Cracct_sect = gl_rec.sect;  /* section of cr acct : used in GL Xaction */
	if(s_rec.s_glperiod == param_rec.pa_cur_period)
		gl_rec.curcr -= s_rec.s_amount;
	gl_rec.curcr = D_Roundoff(gl_rec.curcr);
	gl_rec.ytd -= s_rec.s_amount;
	gl_rec.ytd = D_Roundoff( gl_rec.ytd);
	gl_rec.currel[s_rec.s_glperiod-1] -= s_rec.s_amount;
	gl_rec.currel[s_rec.s_glperiod-1] =  
		D_Roundoff(gl_rec.currel[s_rec.s_glperiod-1]); 
	retval = put_gl( &gl_rec, UPDATE, e_mesg );
	if( retval!=NOERROR ){
		DispError();
		roll_back(e_mesg);
		return(-1);
	}
	return(0);
}
WriteARShdr()
{
	if(s_rec.s_applied[0] == UNAPPLIED) 
		return(0);
	ar_hdr.ah_fund = s_rec.s_fund;
	ar_hdr.ah_inv_no = s_rec.s_invnbr ;
	ar_hdr.ah_sno = 1;
	retval = get_arhdr(&ar_hdr, UPDATE, 0, e_mesg) ;
	if( retval!=NOERROR ){
		DispError();
		roll_back(e_mesg);
		return(-1);
	}
	ar_hdr.ah_balance -= s_rec.s_amount;
	ar_hdr.ah_balance = D_Roundoff(ar_hdr.ah_balance);
	if( ar_hdr.ah_balance<=0 )
		ar_hdr.ah_status[0] = COMPLETE;
	retval = put_arhdr( &ar_hdr, UPDATE, e_mesg );
	if( retval!=NOERROR ){
		DispError();
		roll_back(e_mesg);
		return(-1);
	}
	return(0);
}


WriteCustomer()
{
	STRCPY(cu_rec.cu_code,s_rec.s_customer);
	retval = get_cust(&cu_rec,UPDATE,0,e_mesg); 	
	if( retval!=NOERROR ){
		DispError();
		roll_back(e_mesg);
		return(-1);
	}
	if(cu_rec.cu_rcpt_dt < s_rec.s_transdt) {
		cu_rec.cu_rcpt_dt = s_rec.s_transdt;
	}
	cu_rec.cu_ytd_rcpts += s_rec.s_amount;
	cu_rec.cu_ytd_rcpts = D_Roundoff(cu_rec.cu_ytd_rcpts);
	cu_rec.cu_cur_bal -= s_rec.s_amount;
	cu_rec.cu_cur_bal = D_Roundoff(cu_rec.cu_cur_bal);
	retval = put_cust( &cu_rec, UPDATE, e_mesg );
	if( retval!=NOERROR ){
		DispError();
		roll_back(e_mesg);
		return(-1);
	}
	return(0);
}

static
DispError()
{
	if(DisplayMessage(e_mesg)<0)
		return(-1);
#ifdef ENGLISH
	fomen("Press any key");
#else
	fomen("Appuyer sur une touche");
#endif
	get();
	sprintf(e_mesg,"iserror: %d, dberror: %d, errno: %d",
			iserror, dberror, errno );
	if(DisplayMessage(e_mesg)<0)
		return(-1);
#ifdef ENGLISH
	fomen("Press any key");
#else
	fomen("Appuyer sur une touche");
#endif
	get();
	return(HideMessage());
}

/**********
ChangeApplied()
{
	if( SetDupBuffers(REFNBR_FLD,REFNBR_FLD,2)<0 )
		return(-1);

	retval = GetRecord();	* Read the key values *
	if( retval==UNDEF || retval==ESCAPE || retval==LOCKED )
		return( ESCAPE );

	if(rcpt_hdr.rhdr_applied[0] == APPLIED) {
#ifdef ENGLISH
		fomer("Receipt Already Applied to an Invoice");
#else
		fomer("Recu deja applique a une facture");
#endif	
		get();
		return(0);
	}
	retval = DisplayRecord();	* Display the header & item records *
	if( retval<0 ) return(retval);

	s_rec.s_applied[0] = APPLIED;
	if(WriteFields(APP_FLD,APP_FLD) < 0)
		return(-1);

	s_rec.s_invnbr = LV_LONG;

	* Read the non key fields now *
	if( (retval=ReadFields(INVNBR_FLD,INVNBR_FLD))<0 )
		return(retval);
	if(retval==ESCAPE){
		if(ClearScreen()<0)	return(-1);
		return(retval);
	}

	if(ar_hdr.ah_balance < rcpt_hdr.rhdr_amount) {
		sprintf(e_mesg,"Receipt more than Invoice balance by $ %.02lf",rcpt_hdr.rhdr_amount-ar_hdr.ah_balance); 
		fomer(e_mesg);
		roll_back(e_mesg);
		get();
		return(0);	
	}
	* Allow the user to edit entries, if required, before saving *
	if( (retval=EditFlds())<0 )	return(retval);
	if( retval==ESCAPE ){
		if(ClearScreen()<0)	return(-1);
		return(retval);
	}

	WriteARShdr();
	WriteARSrcpt(UPDATE);
	if( commit(e_mesg)<0 ){
		fomen(e_mesg);
		get();
		return(-1);
	}
	return(0);
}
**********/

Inquiry( access, direction )	/* Screen inquiry, random/sequential */
int access, direction;		/* RANDOM, SEQUENTIAL accesses */
{
	if( access==RANDOM ){
		if( SetDupBuffers(REFNBR_FLD,REFNBR_FLD,2)<0 )
			return(-1);
		retval = GetRecord();	/* Read the key values */
		if( retval==UNDEF || retval==ESCAPE || retval==LOCKED )
			return( ESCAPE );
	}
	else{	/* access is sequential, so get next record from file */
		retval = GetNextRec( direction );	/* Get next rec */
		if(retval==EFL)
			return(0);
	}
	if( retval<0 ){	/* errors in reading */
		fomen(e_mesg);get();
		return(-1);
	}
	
	retval = DisplayRecord();	/* Display the header & item records */
	if( retval<0 ) return(retval);

	return(0);
}
DisplayRecord()	/* Get and display the header and item records */
{
	s_rec.s_refnumb = rcpt_hdr.rhdr_refno;
	s_rec.s_fund = rcpt_hdr.rhdr_fund;
	s_rec.s_applied[0] = rcpt_hdr.rhdr_applied[0];
	s_rec.s_invnbr = 0;
/***
	s_rec.s_invnbr = rcpt_hdr.rhdr_invnumb;
***/
	STRCPY( s_rec.s_customer, rcpt_hdr.rhdr_cust );
	s_rec.s_transdt = rcpt_hdr.rhdr_rcptdate;
	STRCPY( s_rec.s_cheque, rcpt_hdr.rhdr_chequeno);
	s_rec.s_amount = rcpt_hdr.rhdr_amount ;
	s_rec.s_glperiod = rcpt_hdr.rhdr_period;
	STRCPY(s_rec.s_acctno, rcpt_hdr.rhdr_acctno );
	STRCPY( s_rec.s_remarks, rcpt_hdr.rhdr_remarks );

	if( WriteFields(FUND_FLD,REMARKS_FLD) < 0 )
		return(-1);
	return(0);
}
GetNextRec(direction)	/* Read the next record in the specified direction */
int	direction;
{
	if( flg_start(RCPTHDR)!=direction ){ 	/* file access mode changed */
		rcpt_hdr.rhdr_refno = s_rec.s_refnumb;
		if( direction==FORWARD )
			rcpt_hdr.rhdr_refno++;
		else
			rcpt_hdr.rhdr_refno--;
		flg_reset(RCPTHDR);
	}
	/* Read the next record from stmast file */
	retval = get_n_rcpthdr( &rcpt_hdr, BROWSE, 0, direction, e_mesg );
#ifndef ORCALE
	seq_over( RCPTHDR );
#endif
	if( retval==EFL ){
#ifdef ENGLISH
		fomen("No More Records....");
#else
		fomen("Plus de fiches....");
#endif
		get();
		flg_reset(RCPTHDR);
		return(EFL);
	}
	if( retval!=NOERROR ){
		fomen(e_mesg);	get();
		return(retval);
	}
	/* Write the key fields on to the screen */
	s_rec.s_refnumb = rcpt_hdr.rhdr_refno;
	if( WriteFields(REFNBR_FLD,REFNBR_FLD)<0 )
		return(-1);
	return(0);
}
GetRecord()	/* Read the header key values, get terminal info ,read rec */
{
	int	mode;

	if( s_rec.s_fn[0]==CHANGE||s_rec.s_fn[0]==DELETE )	
		mode = UPDATE;
	else
		mode = BROWSE;
	s_rec.s_refnumb = LV_LONG;

	if((retval=ReadFields(REFNBR_FLD,REFNBR_FLD))<0 || retval==ESCAPE ){ 
		return( retval );
	}
	rcpt_hdr.rhdr_refno = s_rec.s_refnumb;
	retval = get_rcpthdr( &rcpt_hdr, mode, 0, e_mesg );
	if( retval!=NOERROR ){
		fomer(e_mesg);
		get();
		return(retval);
	}

	return(0);
}

/* Display message and get the option */
GetOpt(msg,options)
char	*msg ;
char	*options ;
{
	int	i, j ;

	if( DisplayMessage(msg)<0 )
		return(-1); ;

	for( ; ; ) {
		s_rec.s_resp[0] = LV_CHAR;
		if( (retval=ReadFields(RESP_FLD,RESP_FLD))<0 )
			return(retval);
		j = strlen(options) ;
		for( i = 0 ; i < j ; i++)
			if(s_rec.s_resp[0] == options[i]) break ;
		if(i != j) break ;	/* Valid Option Selected */
#ifdef ENGLISH
		fomer("Invalid Option..");
#else
		fomer("Option invalide..");
#endif
	}
	s_rec.s_mesg[0] = HV_CHAR ;
	s_rec.s_resp[0] = HV_CHAR ;
	if( (retval=WriteFields(MESG_FLD,RESP_FLD))<0 )
		return(retval);

	return((int)(options[i])) ;
}	/* GetOpt() */

fund_default()
{
	fomca1(FUND_FLD,19,2);
	s_rec.s_fund = 1;
	WriteFields(FUND_FLD,FUND_FLD);
	s_rec.s_fund = LV_SHORT;
}