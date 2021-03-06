/*------------------------------------------------------------------------
Source Name: pselect.c
System     : Accounts Payables.
Created  On: 29th November 89.
Created  By: J PRESCOTT.

DESCRIPTION:
	Program to enter payment selections.

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
peter ralph    90/11/23	      Right Justify Numeric supplier code.
F.Tao 	       90/12/18	      Round up amounts before writing to file.
F.Tao 	       90/12/21	      Right justify Supplier code.
L.Robichaud    93/07/08	      Call function to check anyone is doing invoice
			      entry. (invcheck()). Do not continue if their is
------------------------------------------------------------------------*/

#define	MAIN		/* Main program. This is to declare Switches */

#define	SYSTEM		"ACCOUNT PAYABLE"	/* Sub System Name */
#define	MOD_DATE	"19-DEC-90"		/* Program Last Modified */

#include <stdio.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	DELTA_AMT	0.005	/* To Check float and double values for zero */
#define	DELTA_QTY	0.00005	/* To Check float and double values for zero */
#define EXIT	   	12
#define CNCLCHEQ	66	/* Cancelled Cheque Found */

#define	FIRST_PASS	1
#define	SECOND_PASS	2

/* User Interface define constants */
#ifdef ENGLISH
#define SELECT		'S'
#define EXITOPT		'E'

#define	YES		'Y'
#define NO		'N'
#define	EDIT		'E'
#define	CANCEL		'C'
#else
#define SELECT		'C'
#define EXITOPT		'F'

#define	YES		'O'
#define NO		'N'
#define	EDIT		'M'
#define	CANCEL		'A'
#endif
/* in_transaction types */
#ifdef ENGLISH
#define INVOICE		"IN"
#define CRMEMO		"CM"
#define RETURN		"RT"
#define DBMEMO		"DM"
#else
#define INVOICE		"FC"
#define CRMEMO		"NC"
#define RETURN		"RV"
#define DBMEMO		"ND"
#endif

/* PROFOM Releted declarations */

#define	SCR_NAME	"pselect"	/* PROFOM screen Name */

/* Field PROFOM numbers */
#define START_FLD 	400	/* Start Field in range */
#define	END_FLD		1600	/* Last Field of the screen */

#define DUEDATE		400	/* due on or before date field number */
#define TRANSDATE	500	/* Trans. on or before date field number */
#define RELEASEHB	600	/* release holdbacks option field number */
#define SUPPCD1		700	/* starting supplier code field number */
#define SUPPCD2		800	/* ending supplier code field number */
#define TREFNO1		900	/* starting trans. ref. no. field number */
#define TREFNO2		1000	/* ending trans. ref. no. field number */
#define PASTDUEDT	1100	/* past due date field number */
#define FUND1		1200	/* starting fund # field number */
#define FUND2		1300	/* ending fund # field number */
#define OPTION		1400	/* option choice field number */
#define MESSAGE		1500	/* message field number */
#define RESPONSE	1600	/* response field number */

/* pselect.sth - header for C structure generated by PROFOM EDITOR */

typedef struct	{

	char	s_pgm[11];	/* 100 program name */
	long	s_rundate;	/* 300 run date */
	long	s_duedate;	/* 400 due date */
	long	s_transdate;	/* 500 transaction date */
	int	s_releasehb;	/* 600 release holdbacks option */
	char 	s_suppcd1[11];	/* 700 starting supplier code */
	char	s_suppcd2[11];	/* 800 ending supllier code */
	char	s_trefno1[16];	/* 900 starting trans ref. no. */
	char	s_trefno2[16];	/* 1000 ending trans ref. no. */
	long	s_pastduedt;	/* 1100 past due date */
	short	s_fund1;	/* 1200 starting fund # */
	short 	s_fund2;	/* 1300 ending fund # */
	char	s_option[2];	/* 1400 option choice */
	char	s_mesg[78];	/* 1500 message field */
	char	s_resp[2];	/* 1600 response field */
	} S_STRUCT;


S_STRUCT	s_sth;	/* PROFOM Screen Structure */
static	struct  stat_rec  sr;		/* PROFOM status rec */

/* File structures */
Pa_rec	pa_rec;
Invoice	in_rec;
Supplier su_rec;
Chq_rec	chq_rec;

static  short	manual_check, manual ;		/* program controlled flags */
static  short	eligible_flag;
static  short	retain_flag;

static  char    cur_suppcd[11]; 	/* Active suppliers code */
static	short	cur_fund;		/* Current Fund for supplier */
static	long	cur_cheque; 		/* Current Cheque No. */

static	char 	e_mesg[100];  		/* dbh will return err msg in this */

static  double fund_total;		/* fund total structure */
static  double disc_taken;
static  double disc_lost;
static  double gross_pay;

double 	D_Roundoff();
main(argc,argv)
int argc;
char *argv[];
{
	int 	retval, numprocess;

	retval = Initialize(argc,argv);	/* Initialization routine */

#ifndef DOS
	numprocess = invcheck();	/* Check for invoice entry processes */
	if(numprocess == ERROR){
		DispError("Error Occured Checking Active Processes");
		close_dbh();
		exit(0);
	}
	if(numprocess > 0){
		DispError("Can Not Proceed, Invoice Entry In Process");
		close_dbh();
		exit(0);
	}
#endif
	if (retval == NOERROR) retval = Process();

	CloseRtn();			/* return to menu */
	if (retval != NOERROR) exit(1);
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

	proc_switch(argc, argv, CHEQUE) ; 	/* Process Switches */

	/*
	*	Initialize PROFOM & Screen
	*/
	STRCPY(sr.termnm,terminal);	/* Copy Terminal Name */
	fomin(&sr);
	ret(err_chk(&sr)) ;		/* Check for PROFOM Error */
	fomcf(1,1);			/* Enable Snap screen option */

	err = InitScreen() ;		/* Initialize Screen */
	if(NOERROR != err) return(err) ;

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
		DispError("Parameters Are Not Setup..");
#else
		DispError("Parametres ne sont pas etablis..");
#endif
		return(ERROR) ;
	}

	return(NOERROR) ;
}	/* Initialize() */

/*--------------------------------------------------------------------------*/
/* Close nessary files and environment before exiting program               */

CloseRtn() 
{
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
	/* move screen name to Profom status structure */
	STRCPY(sr.scrnam,NFM_PATH);
	strcat(sr.scrnam,SCR_NAME) ;

	STRCPY(s_sth.s_pgm,PROG_NAME);
	s_sth.s_rundate = get_date();	/* get Today's Date in YYYYMMDD format*/
	s_sth.s_option[0] = HV_CHAR;
	s_sth.s_mesg[0] = HV_CHAR ;
	s_sth.s_resp[0] = HV_CHAR ;

	/* Move Low Values to data fields */
	InitFields() ;

	return(NOERROR) ;
}	/* InitScreen() */

/*-------------------------------------------------------------------*/
/* Get Option from user and call corresponding function */

Process()
{
	int	err;

	for( ; ; ){

		if((err = ReadOption())<0) 
			return(err);

		switch(s_sth.s_option[0]) {
		case  EXITOPT :
			return(NOERROR);
		case  SELECT :
			if(pa_rec.pa_cur_period == 0) {
#ifdef ENGLISH
				fomer("Not Allowed Before Yearly Closing...");
#else
				fomer("Pas permis avant la fermeture annuelle...");
#endif
				get();
				break;
			}
			CHKACC(err,ADD,e_mesg);
			err = ProcOption() ;
			break ;
		default :
			continue;
		}

		if(NOACCESS == err)	fomen(e_mesg);
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
}	/* Process() */
/*------------------------------------------------------------*/
ReadOption()
{

	s_sth.s_mesg[0] = HV_CHAR;
	ShowMesg();	
#ifdef ENGLISH
	fomer("S(elect), E(xit)");
#else
	fomer("C(hoisir), F(in)");
#endif
	sr.nextfld = OPTION;
	fomrf((char *)&s_sth);
	ret(err_chk(&sr));

}	/* ReadOption */
/*------------------------------------------------------------*/
ProcOption()
{
	int	i, err ;

	err = ChkCnclCheq();
	if(err == ERROR) return(ERROR);
	if(err == CNCLCHEQ) {
		return(NOERROR);
	}

	for(i = START_FLD ; i <= END_FLD - 300 ; i += 100)
		fomca1(i, 19, 0) ;    /* disable dup control */

	err = ReadRange(ADD) ;
	if(err != NOERROR) return(err) ;

	err = Confirm() ;
	if(err != YES) return(NOERROR) ;

	err = ProcRanges() ;

	if(err != NOERROR) return(err);

	return(NOERROR);
}	/* ProcSelection() */
/*------------------------------------------------------------*/
/* Check to see if any outstanding Cancelled Cheques before   */
/* A payment Selection Can be Done. 			      */
/*------------------------------------------------------------*/
ChkCnclCheq()
{
	int	err;

	/* check to see if check exists with manual cheque number */
	STRCPY(chq_rec.c_supp_cd,"\0");
	chq_rec.c_funds = 0;
	chq_rec.c_chq_no = 0 ;
	STRCPY(chq_rec.c_invc_no, "\0");
	STRCPY(chq_rec.c_tr_type, "\0");
	flg_reset(CHEQUE);

	err = get_n_chq(&chq_rec,BROWSE,0,FORWARD,e_mesg);
	if(err != NOERROR) {
	 	if(err == EFL) {
			return(NOERROR);
		}
		DispError(e_mesg);
		return(ERROR);
	}
	if(chq_rec.c_cancelled[0] == YES) {
#ifdef ENGLISH
		DispError("Cannot Do Payment Selection.  Cancelled Cheques Still to Process.");
#else
		DispError("Selection de paiement ne peut etre faite.  Il reste des cheques annules a traiter.");
#endif
		return(CNCLCHEQ);
	}

	return(NOERROR);
}
/*------------------------------------------------------------*/
/* Get the Header details from user */

ReadRange(mode)
int	mode ;
{
	int	 i, err ;

	if(mode == ADD) {
#ifdef ENGLISH
		STRCPY(s_sth.s_mesg,"Press ESC-F to Go to Option:");
#else
		STRCPY(s_sth.s_mesg,"Appuyer sur ESC-F pour retourner a Option:");
#endif
		ShowMesg();

		fomca1(DUEDATE, 19, 2) ;
		fomca1(TRANSDATE, 19, 2) ;
		fomca1(SUPPCD1, 19, 2);
		fomca1(SUPPCD2, 19, 2) ;
		fomca1(TREFNO2, 19, 2) ;
		fomca1(PASTDUEDT, 19, 2) ;
		fomca1(FUND1, 19, 2) ;
		fomca1(FUND2, 19, 2) ;
		s_sth.s_duedate = 99991231 ;
		s_sth.s_transdate = 99991231 ;
		STRCPY(s_sth.s_suppcd1, "0");
		STRCPY(s_sth.s_suppcd2, "ZZZZZZZZZZ");
		STRCPY(s_sth.s_trefno2, "ZZZZZZZZZZZZZZZ");
		s_sth.s_pastduedt = s_sth.s_rundate ;
		s_sth.s_fund1 = 1 ;
		s_sth.s_fund2 = 999 ;
		sr.nextfld = DUEDATE ;
		sr.endfld = END_FLD - 300 ;
		fomud((char*)&s_sth);
		ret(err_chk(&sr));
	}
	InitFields() ;

	i = ReadFields(START_FLD, END_FLD - 300, mode) ;
	if(PROFOM_ERR == i || DBH_ERR == i) return(i) ;
	if(RET_USER_ESC == i) {	/* ESC-F */
		return(RET_USER_ESC) ;
	}

	return(NOERROR) ;
}	/* ReadRange() */
/*------------------------------------------------------------*/
/* Read the PROFOM Screen for a given Range of fields */

ReadFields(st_fld, end_fld, mode)
int	st_fld ;
int	end_fld ;
int	mode ;
{
	int	err ;

	sr.nextfld = st_fld ;
	sr.endfld  = end_fld ;

	for( ; ;){
		fomrd( (char *)&s_sth );
		ret(err_chk(&sr));
		if(sr.retcode == RET_VAL_CHK){
			err = Validation() ;
			if(DBH_ERR == err || PROFOM_ERR == err) return(err);
			sr.nextfld = sr.curfld ;
			continue;
		}
		if(sr.retcode == RET_USER_ESC){
			if(mode == ADD &&
				(sr.escchar[0] == 'f' || sr.escchar[0] == 'F'))
					return(RET_USER_ESC) ;
			continue;
		}
		/* else RET_NO_ERROR */
		break;
	}

	return(NOERROR) ;
}	/* ReadFields() */
/*------------------------------------------------------------*/
/* Write fields on Screen for a given Range */

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
/* Validation function() for Key and Header fields when PROFOM returns
  RET_VAL_CHK */

Validation()
{
	switch(sr.curfld){
	case SUPPCD1:  /* ending supplier code */
		Right_Justify_Numeric(s_sth.s_suppcd1
					,(sizeof(s_sth.s_suppcd1)-1));
		break;
	case SUPPCD2:  /* ending supplier code */
		Right_Justify_Numeric(s_sth.s_suppcd2
					,(sizeof(s_sth.s_suppcd2)-1));
		if(strcmp(s_sth.s_suppcd2,s_sth.s_suppcd1) <0) {
#ifdef ENGLISH
			fomer("Ending code cannot precede starting code");
#else
			fomer("Code finissant ne peut pas preceder le code debutant");
#endif
			s_sth.s_suppcd2[0] = LV_CHAR;
		}
		break;
	case TREFNO2:  /* ending trans. ref. no. */
		if(strcmp(s_sth.s_trefno2,s_sth.s_trefno1) <0) {
#ifdef ENGLISH
			fomer("Ending number cannot precede starting number");
#else
			fomer("Numero finissant ne peut pas preceder le numero debutant");
#endif
			s_sth.s_trefno2[0] = LV_CHAR;
		}
		break;
	case FUND2:  /* ending fund number */
		if(s_sth.s_fund2 == 0) {
#ifdef ENGLISH
			fomer("Ending fund must be greater than Zero");
#else
			fomer("Fond finissant doit etre plus grand que zero");
#endif
			s_sth.s_fund2 = LV_SHORT;
		}
		if(s_sth.s_fund2 < s_sth.s_fund1) {
#ifdef ENGLISH
			fomer("Ending fund cannot precede starting fund");
#else
			fomer("Fond finissant ne peut pas preceder le fond debutant");
#endif
			s_sth.s_fund2 = LV_SHORT;
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
}	/* Validation() */
/*-----------------------------------------------------------------------*/
/* Take the confirmation from user for the header part */

Confirm()
{
	int	err ;

	for( ; ; ) {
#ifdef ENGLISH
		err = GetOption("Y(es), E(dit), C(ancel)", "YEC");
#else
		err = GetOption("O(ui), M(odifier), A(nnuler)", "OMA");
#endif
		if(err == PROFOM_ERR) return(err) ;

		switch(err) {
		case  YES :
			return(YES) ;
		case  EDIT  :
			err = FieldEdit();
			break ;
		case  CANCEL :
#ifdef ENGLISH
			err = GetOption("Confirm the Cancel (Y/N)?", "YN") ;
#else
			err = GetOption("Confirmer l'annulation (O/N)?", "ON") ;
#endif
			if(err == YES) { 
				roll_back(e_mesg) ;	/* Unlock  Records */
				return(CANCEL) ;
			}
			break ;
		}	/* switch err */

		if(err == PROFOM_ERR) return(err) ;
		if(err == DBH_ERR) return(err) ;
	}	/* for(; ; ) */
}	/* Confirm() */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Changing fields. Accept fld to be changed and read that fld 		 */

FieldEdit()
{
     	int	i,retval;

     	for ( i = START_FLD; i <= END_FLD - 300 ; i += 100 )
       		fomca1( i,19,2 );      		/*  enable Dup Control */

     	sr.nextfld = START_FLD;
     	sr.endfld = END_FLD - 300;
     	fomud( (char *) &s_sth );
     	ret(err_chk(&sr));

	retval = ReadRange(UPDATE);
	if(retval != NOERROR) return(retval) ;

     	return(NOERROR);
}	/* FieldEdit() */
/*-----------------------------------------------------------------------*/
/* Display message and get the option */

GetOption(msg,options)
char	*msg ;
char	*options ;
{
	int	i, j ;

	STRCPY(s_sth.s_mesg,msg);
	ShowMesg() ;

	sr.nextfld = RESPONSE ;
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
	
	ret( WriteFields(MESSAGE,RESPONSE) );

	return((int)(options[i])) ;
}	/* GetOption() */
/*-------------------------------------------------------------------------*/
/* Initialize Screen Header with either Low Values */

InitFields()
{
	s_sth.s_duedate = LV_LONG ;
	s_sth.s_transdate = LV_LONG ;
	s_sth.s_releasehb = LV_INT ;
	cur_suppcd[0] = LV_CHAR; 		/* Active suppliers code */
	s_sth.s_suppcd1[0] = LV_CHAR ;
	s_sth.s_suppcd2[0] = LV_CHAR ;
	s_sth.s_trefno1[0] = LV_CHAR ;
	s_sth.s_trefno2[0] = LV_CHAR ;
	s_sth.s_pastduedt = LV_LONG ;
	s_sth.s_fund1 = LV_SHORT ;
	s_sth.s_fund2 = LV_SHORT ;
	
	return(NOERROR) ;
}	/* InitFields() */
/*-------------------------------------------------------------------------*/

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
	roll_back(e_mesg);
	s_sth.s_mesg[0] = HV_CHAR;
	ShowMesg();
	return(NOERROR);
}
/*-------------------------------------------------------------------------*/
ShowMesg()  /* shows or clears message field */
{
	sr.nextfld = MESSAGE;
	fomwf( (char *)&s_sth ) ;
}
/*-----------------------------------------------------------------------*/
ProcRanges()
{
	int	i, err;
	int	totals ;

	cur_fund = s_sth.s_fund1;
	STRCPY(cur_suppcd,s_sth.s_suppcd1);
	cur_cheque = 0;
	for(;;) {
		in_rec.in_funds = cur_fund;
		STRCPY(in_rec.in_supp_cd,cur_suppcd);
		in_rec.in_chq_no = cur_cheque;
		STRCPY(in_rec.in_invc_no,s_sth.s_trefno1);
		in_rec.in_tr_type[0] = '\0';
	
		flg_reset(APINVOICE);
	
		err = get_n_invc(&in_rec,BROWSE,2,FORWARD,e_mesg);
		if(err < NOERROR && err != EFL){
			DispError(e_mesg) ;
			return(ERROR);
		}
		if(err == EFL || 
		   strcmp(in_rec.in_supp_cd,s_sth.s_suppcd2) > 0 ||
		   in_rec.in_funds > s_sth.s_fund2)
			break ;
	
		flg_reset(APINVOICE);

		STRCPY(cur_suppcd, in_rec.in_supp_cd) ;
		cur_fund = in_rec.in_funds;
		cur_cheque = in_rec.in_chq_no;

		err = FirstPass();
		if(err == ERROR) {
			roll_back(e_mesg);
			return(ERROR);
		}
		totals = 0 ;

		if(fund_total > DELTA_AMT) {
			totals = 1 ;
		}

		if(totals) { 
			err = SecondPass();
			if(err != NOERROR) {
				roll_back(e_mesg);
				return(ERROR);
			}
		}
		/* Release Stop Payments */
		if(retain_flag && s_sth.s_releasehb) {
			err = Release_StopPayment();
			if(err == ERROR) {
				roll_back(e_mesg);
				return(ERROR);
			}
		}

		cur_cheque++;
/*
		inc_str(cur_suppcd,sizeof(su_rec.s_supp_cd) -1, FORWARD);
*/
	}

	return(NOERROR);
}	/* ProcRanges() */
/*-----------------------------------------------------------------------*/
GetValidInv(pass_1or2)
int	pass_1or2;
{
	int err;

	eligible_flag = 0;
	manual = 0 ;

	for(;;) {
		err = get_n_invc(&in_rec,BROWSE,2,FORWARD,e_mesg);
		if(err < 0) {
			if(err == EFL) return(EFL); 
			DispError(e_mesg);
			return(ERROR);
		}

		if(strcmp(in_rec.in_invc_no,s_sth.s_trefno2) > 0 ||
			strcmp(in_rec.in_supp_cd,cur_suppcd) != 0 ||
			in_rec.in_funds != cur_fund || 
			in_rec.in_chq_no != cur_cheque)
			return(EFL);

		if(in_rec.in_funds > s_sth.s_fund2)
			return(EFL);

		if(in_rec.in_pmtcode[0] == PARTIAL && in_rec.in_chq_no != 0){
			manual_check = 1;
			manual = 1 ;
			eligible_flag = 1;
			break;
		}
		if(in_rec.in_due_dt > s_sth.s_duedate ||
		   in_rec.in_invc_dt > s_sth.s_transdate) {
			continue;
		}
		if(in_rec.in_pmtcode[0] == STOPPMT) {
			if(pass_1or2 == FIRST_PASS){
				retain_flag = 1;
			}
			continue;
		}
		if(in_rec.in_pmtcode[0] == COMPLETE || 
		   in_rec.in_pmtcode[0] == REL_HB) {
			continue;
		}	

		/* check to see if check already exists */
		STRCPY(chq_rec.c_supp_cd,in_rec.in_supp_cd);
		chq_rec.c_funds = in_rec.in_funds;
		chq_rec.c_chq_no = in_rec.in_chq_no ;
		STRCPY(chq_rec.c_invc_no, in_rec.in_invc_no);
		STRCPY(chq_rec.c_tr_type, in_rec.in_tr_type);

		err = get_chq(&chq_rec,BROWSE,0,e_mesg);
		if(err != NOERROR) {
		 	if(err == UNDEF) {
				eligible_flag = 1;
				break;
			}
			DispError(e_mesg);
			return(ERROR);
		}
	}
	return(NOERROR);
}	/* GetValidInv() */
/*-----------------------------------------------------------------------*/
FirstPass()	/* Check to see if supplier fund total > 0 */
{
	int	i, err;
	
	manual_check = 0 ;
	retain_flag = 0 ;
	fund_total = 0.0 ;

	for(;;) {
		err = GetValidInv(FIRST_PASS);
		if(err != NOERROR){
			if(err == EFL) break;
			roll_back(e_mesg);
			return(ERROR);
		}
		if(eligible_flag == 1) {
			calc_supplier_totals() ;
			fund_total += gross_pay - disc_taken ;
		}
	}
	return(NOERROR); 
}	/* FirstPass() */	
/*-----------------------------------------------------------------------*/
calc_supplier_totals()	/* calculate supplier totals */
{
	double	diff = 0;

	disc_taken = disc_lost = gross_pay = 0;

	switch(su_rec.s_type[0]) {
	case	CONTRACT:	/* Contract supplier */
	    diff = in_rec.in_amount - in_rec.in_disc_amt;
	    if(in_rec.in_pmtcode[0] == OPEN)
		gross_pay = diff ;
	    else {	/* Partial or Manual Payment */
		/*
		*	if partpmt > (transamt-disc)
		*		gross_pay = (tranamt-disc)
	   	*	else	
		*		gross_pay = partpmt
		*/

		if( strcmp(in_rec.in_tr_type,INVOICE) == 0 ||
				strcmp(in_rec.in_tr_type,CRMEMO) == 0 ) {
			if(in_rec.in_part_amt > diff)
				gross_pay = in_rec.in_part_amt ;
			else
				gross_pay = diff ;
		}
		else {
			if( in_rec.in_part_amt < diff)
				gross_pay = in_rec.in_part_amt ;
			else
				gross_pay = diff ;
		}
	    }
	    break ;
	default :	/* Ordinary supplier */
	    if(in_rec.in_due_dt < s_sth.s_pastduedt) {
		disc_taken = 0.0;
		disc_lost = in_rec.in_disc_amt ;
		if(in_rec.in_pmtcode[0] == OPEN)
			gross_pay = in_rec.in_amount ;
		else
			gross_pay = in_rec.in_part_amt ;
	    }
	    else {
		disc_lost = 0.0;
		if(in_rec.in_pmtcode[0] == OPEN) {
		    gross_pay = in_rec.in_amount ;
		    disc_taken = in_rec.in_disc_amt ;
		}
		else {	/* Partial or Manual Payment */
		    /*
		    *	if partpmt < (transamt - disc) {
		    *		disc_taken = (disc% * partpmt)/(100-disc%)
		    *		gross_pay = partpmt + disc_taken ;
		    *	}
		    *	else {
		    *		gross_pay = tranamt ;
		    *		disc_taken = disc_amt ;
		    *	}
		    */

		    diff = in_rec.in_amount - in_rec.in_disc_amt;

		    if( ((strcmp(in_rec.in_tr_type,INVOICE) == 0 ||
				strcmp(in_rec.in_tr_type,CRMEMO) == 0) &&
				in_rec.in_part_amt > diff) ||
			((strcmp(in_rec.in_tr_type,RETURN) == 0 ||
				strcmp(in_rec.in_tr_type,DBMEMO) == 0) && 
				in_rec.in_part_amt < diff) ) {

			disc_taken = (in_rec.in_disc_per * in_rec.in_part_amt) /
 				   (100 - in_rec.in_disc_per) ;
			gross_pay = in_rec.in_part_amt + disc_taken ;
		    }
		    else {
			gross_pay = in_rec.in_amount ;
			disc_taken = in_rec.in_disc_amt ;
		    }	
		}
	    }
	}	/* Switch() */

	/*
	*  NOTE: Here all the figures are multified by -1, because for INVOICE
	*  and CRMEMO figures are stored as -ve, for others +ve. So, when you
	*  are paying, payments will be in +ve amounts.
	*/

	gross_pay  *= -1 ;
	disc_taken *= -1 ;
	disc_lost  *= -1 ;

	return(NOERROR);
}
/*-----------------------------------------------------------------------*/
Release_StopPayment()
{
	int	err, i ;
	
	STRCPY(in_rec.in_supp_cd,cur_suppcd);
	STRCPY(in_rec.in_invc_no, s_sth.s_trefno1) ;
	in_rec.in_tr_type[0] = '\0';
	flg_reset(APINVOICE);
	i = 0 ;
	for(;;) {
		err = get_n_invc(&in_rec,BROWSE,0,FORWARD,e_mesg);
		if(err != NOERROR) {
			if(err == EFL) break;
			DispError(e_mesg);
			return(ERROR);
		}	
		if(strcmp(in_rec.in_supp_cd,cur_suppcd) != 0 ||
			strcmp(in_rec.in_invc_no,s_sth.s_trefno2) > 0 )
			break;

		if(in_rec.in_funds < s_sth.s_fund1 || 
					in_rec.in_funds > s_sth.s_fund2)
			continue;

		if(in_rec.in_pmtcode[0] == STOPPMT) {
			err = get_invc(&in_rec,UPDATE,0,e_mesg);
			if(err != NOERROR) {
				DispError(e_mesg);
				return(ERROR);
			}	
			in_rec.in_pmtcode[0] = OPEN ;
			err = put_invc(&in_rec,UPDATE,e_mesg);	
			if(err == ERROR) {
				DispError(e_mesg);
				return(ERROR);
			}
			i++;
			if(i % 10 == 0) {
				if(commit(e_mesg)<0) {
					DispError(e_mesg);
					return(ERROR);
				}
			}
			inc_str(in_rec.in_tr_type, sizeof(in_rec.in_tr_type)-1,
				FORWARD);
		}
	}
	seq_over(APINVOICE);
	if(i % 10 != 0) {
		if(commit(e_mesg)<0) {
			DispError(e_mesg);
			return(ERROR);
		}
	}

 	return(NOERROR);
}
/*-----------------------------------------------------------------------*/
SecondPass()
{
	int 	err, i ;

	STRCPY(su_rec.s_supp_cd,cur_suppcd);
	err = get_supplier(&su_rec,UPDATE,0,e_mesg);
	if(err == ERROR) {
		DispError(e_mesg);
		return(ERROR);
	}

	in_rec.in_funds = cur_fund;
	STRCPY(in_rec.in_supp_cd,cur_suppcd);
	in_rec.in_chq_no = cur_cheque;
	STRCPY(in_rec.in_invc_no,s_sth.s_trefno1);
	in_rec.in_tr_type[0] = '\0' ;
	flg_reset(APINVOICE);

	i = 0 ;

	for(;;) {
		err = GetValidInv(SECOND_PASS);
		if(err == ERROR){
			roll_back(e_mesg);
			return(ERROR);
		}
		if(err == EFL) break;
 
		if(fund_total <= DELTA_AMT)
				continue;

		err = get_invc(&in_rec,UPDATE,2,e_mesg);
		if(err != NOERROR) {
			DispError(e_mesg);
			return(ERROR);
		}	
		calc_supplier_totals() ;

		err = Write_Cheque();
		if(err != NOERROR) return(ERROR);

		su_rec.s_balance += gross_pay;
		su_rec.s_balance = D_Roundoff(su_rec.s_balance);
		su_rec.s_ytd_disc += disc_taken ;
		su_rec.s_ytd_disc = D_Roundoff(su_rec.s_ytd_disc);
		if(Update_Invoice()<0) return(ERROR);
		i++;
		if(i % 10 == 0) {
			if(Update_Supplier()<0) return(ERROR);
			if(commit(e_mesg)<0) {
				DispError(e_mesg);
				return(ERROR);
			}

			err = get_supplier(&su_rec,UPDATE,0,e_mesg);
			if(err == ERROR) {
				DispError(e_mesg);
				return(ERROR);
			}
		}
		/* Because a manual cheque sets the in_chq_no to zero it */ 
		/* must be set back to cur_cheque so the next record will */
		/* be read in */
		in_rec.in_chq_no = cur_cheque;
		inc_str(in_rec.in_tr_type,sizeof(in_rec.in_tr_type)-1, FORWARD);
		flg_reset(APINVOICE);
	}	
	if(i % 10 != 0) {
		if(Update_Supplier()<0) return(ERROR);
		if(commit(e_mesg)<0) {
			DispError(e_mesg);
			return(ERROR);
		}
	}
	return(NOERROR);
}	/* SecondPass() */

/*-----------------------------------------------------------------------*/
Write_Cheque()
{
	int err;

	STRCPY(chq_rec.c_supp_cd,in_rec.in_supp_cd);
	chq_rec.c_funds = in_rec.in_funds;
	if(in_rec.in_pmtcode[0] == OPEN){
		chq_rec.c_chq_no = 0;
	}
	else {
	 	chq_rec.c_chq_no = in_rec.in_chq_no;		
	}
	STRCPY(chq_rec.c_accno,in_rec.in_accno);
	STRCPY(chq_rec.c_invc_no,in_rec.in_invc_no);
	STRCPY(chq_rec.c_tr_type,in_rec.in_tr_type);
	if(manual == 1)
		chq_rec.c_chq_type[0] = MANUAL ;
	else
		chq_rec.c_chq_type[0] = REGULAR ;
	chq_rec.c_cancelled[0] = NO ;
	chq_rec.c_invc_dt = in_rec.in_invc_dt;
	chq_rec.c_due_dt = in_rec.in_due_dt;
	chq_rec.c_period = in_rec.in_period;
	chq_rec.c_disc_per = in_rec.in_disc_per;
	chq_rec.c_disc_per = D_Roundoff(chq_rec.c_disc_per);
	chq_rec.c_in_amt = in_rec.in_amount + gross_pay;
	chq_rec.c_in_amt = D_Roundoff( chq_rec.c_in_amt);
	chq_rec.c_disc_taken = disc_taken;
	chq_rec.c_disc_taken = D_Roundoff(chq_rec.c_disc_taken);
	chq_rec.c_gr_amt = gross_pay;
	chq_rec.c_gr_amt = D_Roundoff(chq_rec.c_gr_amt);
	chq_rec.c_cp_chq_no = 0;
	err = put_chq(&chq_rec,ADD,e_mesg);
	if(err != NOERROR) {
		DispError(e_mesg);
		return(ERROR);
	}
	return(NOERROR);
}	/* Write_Cheque() */

/*-----------------------------------------------------------------------*/
Update_Supplier()
{
	int err;

	err = put_supplier(&su_rec,UPDATE,e_mesg);
	if(err != NOERROR) {
		DispError(e_mesg);
		return(ERROR);
	}
	return(NOERROR);
}	/* Update_Supplier() */ 

/*-----------------------------------------------------------------------*/
Update_Invoice()
{
	int err;

	in_rec.in_amount += gross_pay;
	in_rec.in_amount = D_Roundoff(in_rec.in_amount );
	in_rec.in_disc_amt += (disc_taken + disc_lost);
	in_rec.in_disc_amt = D_Roundoff(in_rec.in_disc_amt);
	/***
	in_rec.in_part_amt = ((gross_pay - disc_taken) * -1);
	if(in_rec.in_pmtcode[0] == OPEN)
		in_rec.in_chq_no = 0;
	****/
	in_rec.in_pmtcode[0] = OPEN ;
	in_rec.in_part_amt   = 0.0 ;
	in_rec.in_chq_no     = 0 ;
	in_rec.in_accno[0]   = '\0' ;

	if(in_rec.in_amount > -0.005 && in_rec.in_amount < 0.005) { /* == 0.0 */
		in_rec.in_pmtcode[0] = COMPLETE;
	}
	else {
	 	if(su_rec.s_type[0] == CONTRACT) {
			if((strcmp(in_rec.in_tr_type,INVOICE)==0) ||
			   (strcmp(in_rec.in_tr_type,CRMEMO)==0)) {
				if(in_rec.in_amount >= in_rec.in_disc_amt) 
					in_rec.in_pmtcode[0] = REL_HB;
			}
			else {
				if(in_rec.in_amount <= in_rec.in_disc_amt)
					in_rec.in_pmtcode[0] = REL_HB;
			}
		}
	}
	err = put_invc(&in_rec,UPDATE,e_mesg);
	if(err != NOERROR) {
		DispError(e_mesg);
		return(ERROR);
	}
	return(NOERROR);
}	/* Update_Invoice() */

/*-------------------- E n d   O f   P r o g r a m ---------------------*/

