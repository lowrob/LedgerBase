/*-----------------------------------------------------------------------
Source Name: mancheq .c
System     : Personnel/Payroll.
Created  On: 9th Nov 91.
Created  By: Andre Cormier

DESCRIPTION: This program does the maintnance of the manual cheques (add and
		delete manual or advance)

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
------------------------------------------------------------------------*/
#define	MAIN		/* Main program. This is to declare Switches */
#define MAINFL		MAN_CHQ			/* main file used */

#define	SYSTEM		"Setup"			/* Sub System Name */
#define	MOD_DATE	"8-NOV-91"		/* Progran Last Modified */

#include <stdio.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_pp.h>
#include <bfs_com.h>

/* User Interface define constants */
#ifdef ENGLISH
#define MANUAL		'M'
#define DELETE		'D'
#define EXITOPT		'E'

#define EDIT		'E'
#define YES		'Y'
#define CANCEL		'C'

#else
#define MANUAL		'M'
#define DELETE		'D'
#define EXITOPT		'E'

#define EDIT		'E'
#define YES		'Y'
#define CANCEL		'C'

#endif

/* PROFOM Releted declarations */

#define	SCR_NAME	"mancheq"	/* PROFOM screen Name */

/* PROFOM Screen STH file */

/* Field PROFOM numbers */

#define FN_FLD		400	/*  Function : */

#define EMPLOYEE_FLD	500	/*  Description: */
#define FUND_FLD 	600	/*  Income */
#define BANK_ACCT_FLD	700	/*  T4 field */
#define CHEQ_NUM_FLD 	800	/*  T4 field */
#define	CHEQ_DATE_FLD	900	/*  Amt Flag P or F */
#define DEDUCTION_FLD	1000	/*  T4 field */
#define AMOUNT_FLD	1100	/*  Fund: */
#define EMP_FIRST_FLD	1200	/*  UIC Account: */
#define EMP_LAST_FLD	1300	/*  Pay Period 1 */
#define MAIL_ADRES_FLD	1400	/*  Pay Period 2 */
#define STREET_FLD	1500	/*  Pay Period 3 */
#define CITY_FLD	1600	/*  Pay Period 4 */
#define PROV_FLD	1700	/*  Pay Period 5 */
#define POSTAL_FLD	1800	/*  Pay Period 5 */


#define START_FLD	500	/* First Field on screen */
#define	END_FLD		2000	/* Last Field of the screen */
/* bargain.sth - header for C structure generated by PROFOM EDITOR */

typedef struct	{

	char	s_pgname[11];		/* 100 STRING X(10) */
	long	s_rundate;		/* 300 DATE YYYYFMMFDD */
	char	s_fn[2];		/* 400 STRING X */

	char	s_emp_num[13];		/* 500 STRING X(12) */
	short	s_fund_num;		/* 600 NUMERIC 999 */
	char	s_acct_num[19];		/* 700 STRING X(18) */
	long	s_cheq_num;		/* 800 NUMERIC 9(8) */
	long	s_cheq_date;		/* 900 DATE YYYYFMMFDD X */
	char	s_deduction[7];		/* 1000 STRING X(6) */
	double	s_amount;		/* 1100 NUMERIC 9,999,999.99 */
	char	s_name[49];		/* 1200 STRING X(15) */
	char	s_mail_adres[31];	/* 1400 STRING X(30) */
	char	s_street[31];		/* 1500 STRING X(30) */
	char	s_city[31];		/* 1600 STRING X(30) */
	char	s_prov[31];		/* 1700 STRING X(30) */
	char	s_postal[11];		/* 1800 STRING X(10) */

	char	s_mesg[78];		/* 1900 STRING X(76) */
	char	s_resp[2];		/* 2000 STRING X */
} s_struct;

static	s_struct  s_sth,image;	/* PROFOM Screen Structure */
static	struct  stat_rec  sr;		/* PROFOM status rec */

static	char 	e_mesg[100];  		/* dbh will return err msg in this */

static	Man_chq		man_chq,	pre_man_chq;
static	Jr_ent		jr_ent,		pre_jr_ent;
static	Emp		emp;
static	Deduction	deduction;
static	Ctl_rec		ctl_rec;
static	Pa_rec		pa_rec;
static	Chq_hist	chq_hist;
static	Barg_unit	barg_unit;

int	Validation() ;
int	WindowHelp();
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

	if (retval == NOERROR) retval = Process();

	CloseRtn();			/* return to menu */
	if (retval != NOERROR) exit(-1);
	exit(0);

}	/* main */

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
	strcpy(sr.termnm,terminal);	/* Copy Terminal Name */
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
		DispError((char *)&s_sth,e_mesg);
		return(ERROR) ;
	}
	else if(err == UNDEF) {
#ifdef ENGLISH
		DispError((char *)&s_sth,"Parameters Are Not Set Up ...");
#else
		DispError((char *)&s_sth,"Parametres ne sont pas etablis... ");
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
	int	err ;

	/* move screen name to Profom status structure */
	strcpy(sr.scrnam,NFM_PATH);
	strcat(sr.scrnam,SCR_NAME) ;

	strcpy(s_sth.s_pgname,PROG_NAME);

	s_sth.s_rundate = get_date();	/* get Today's Date in YYYYMMDD format*/
	s_sth.s_mesg[0] = HV_CHAR ;
	s_sth.s_resp[0] = HV_CHAR ;

	/* Initialize Key Fields. So that, if user selectes 'N' option
	   immediately after invoking program, then he gets the first
	   record in the file */

	/* Move High Values to data fields and Display the screen */
	err = ClearScreen() ;
	if(NOERROR != err) return(err) ;

	return(NOERROR) ;

}	/* InitScreen() */

/*-------------------------------------------------------------------*/
/* Get Option from user and call corresponding function */

Process()
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
			DispError((char *)&s_sth,e_mesg);
#ifdef ENGLISH
			sprintf(e_mesg,"%s %d Dberror: %d Errno: %d",
				"System Error... Iserror:",
				iserror, dberror, errno);
#else
			sprintf(e_mesg,"%s %d Dberror: %d Errno: %d",
				"Erreur du systeme... Iserror:",
				iserror, dberror, errno);
#endif
			DispError((char *)&s_sth,e_mesg);
			return(DBH_ERR); /* DBH ERROR */
		}
	}      /*   end of the for( ; ; )       */

}	/* Process() */

/*----------------------------------------------------------------*/
/* Display the Function (Fn:) options and get the option from the user */

ReadFunction()
{
	/* Display Fn: options */
#ifdef ENGLISH
	fomer("M(anual), D(elete Manual), E(xit)");
#else
	fomer("M(anual), D(elete Manual), E(xit)");
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
	case MANUAL  :			/* ADD */
		CHKACC(retval,ADD,e_mesg);
		return( Manual() ) ;
	case DELETE  :			/* CHANGE */
		CHKACC(retval,UPDATE,e_mesg);
		return( Delete() ) ;
	case EXITOPT  :			/* Exit */
		return( QUIT ) ;
	}  /*   end of the switch statement */

	return(retval);

}	/* ProcFunction() */

/*----------------------------------------------------------------------*/
/* Adding manual cheques.   */
Manual()
{
	int	err ;

	err = ReadKey();
	if(err != NOERROR) return(err) ;

	err = GetDetails() ;
	if(PROFOM_ERR == err || DBH_ERR == err) return(err) ;
	if(err < 0 || CANCEL == err) {
		roll_back(e_mesg) ;	/* Unlock the locked Records */
		return(ClearScreen()) ;	/* Clear the Screen */
	}

	return(NOERROR);

}	/* Manual() */
/*----------------------------------------------------------------------*/
/* Delete manual cheques.   */
Delete()
{
	int	err ;

	err = ReadKey();
	if(err != NOERROR) return(err) ;

	err = DelManual() ;
	if(PROFOM_ERR == err || DBH_ERR == err) return(err) ;
	if(err < 0 || CANCEL == err) {
		roll_back(e_mesg) ;	/* Unlock the locked Records */
		return(ClearScreen()) ;	/* Clear the Screen */
	}

	return(NOERROR);

}	/* Delete() */
/*-----------------------------------------------------------------------*/
/* Process to delete manual cheques  */
DelManual()
{
	int	err ;

	err = SelectManual() ;
	if(NOERROR != err) return(err) ;

	for( ; ; ) {
		err = Confirm() ;
		if(err != YES) {
			roll_back(e_mesg);
			break;
		}

		err = WriteManual(P_DEL) ;
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

}	/* Delete() */
/*------------------------------------------------------------*/
/* Select manuel cheque to be deleted  */
SelectManual()
{
	int err;

	s_sth.s_cheq_date = HV_LONG;
	s_sth.s_amount = HV_DOUBLE;
	s_sth.s_fund_num = LV_SHORT;
	s_sth.s_acct_num[0] = LV_CHAR;
	s_sth.s_cheq_num = LV_LONG;
	s_sth.s_deduction[0] = LV_CHAR;

	SetDupBuffers(START_FLD,END_FLD,0); /* Off Dup Control */

	err = ReadFields((char *)&s_sth,FUND_FLD, AMOUNT_FLD,
		Validation, WindowHelp,1) ;
	if(PROFOM_ERR == err || DBH_ERR == err) return(err) ;
	if(RET_USER_ESC == err){
		/* When user gives ESC-F while changing fields, assumption is
		* he completed his changes, and remaining fields are same as
		* old. But, at this point STH will be having low values in the 
		* remaining fields. So move the old values form the linked list.
		*/

		err = CopyBack((char *)&s_sth,(char *)&image,
			sr.curfld, END_FLD);
		if(err == PROFOM_ERR) return(err);
		s_sth.s_mesg[0] = HV_CHAR;

		DispMesgFld((char *)&s_sth);

		return(RET_USER_ESC) ;
	}

	return(NOERROR);

}	/* SelectManual() */

/*------------------------------------------------------------*/
/* Allows User to Add Record to File */
GetDetails()
{
	int	i ;
	int	err;

	SetDupBuffers(START_FLD,END_FLD,0); /* Off Dup Control */
	s_sth.s_cheq_date = get_date();
	SetDupBuffers(CHEQ_DATE_FLD,CHEQ_DATE_FLD,1); /* Off Dup Control */

	err = ReadScreen();
	if(PROFOM_ERR == err || DBH_ERR == err) return(err) ;
	if(RET_USER_ESC == err){
		return(RET_USER_ESC) ;
	}

	for( ; ; ) {
		i = Confirm() ;
		if(i != YES) break;

		i = WriteManual(ADD) ;

		if(i < 0) {
			if(i == LOCKED) continue;
		}
		break;
	}
	if(i != NOERROR) return(i);
	return(NOERROR) ;

}	/* GetDetails() */

/*------------------------------------------------------------*/
/* To edit the fields that have already been entered and that
   wants to be change */
ReadScreen()
{
	int err;
	switch(s_sth.s_fn[0]) {
	case  MANUAL :		/* Add Manual */

		s_sth.s_fund_num = LV_SHORT;
		s_sth.s_acct_num[0] = LV_CHAR;
		s_sth.s_cheq_num = LV_LONG;
		s_sth.s_cheq_date = LV_LONG;
		s_sth.s_deduction[0] = LV_CHAR;
		s_sth.s_amount = LV_DOUBLE;



		break;

	}
	err = ReadFields((char *)&s_sth,START_FLD, END_FLD-200,
		Validation, WindowHelp,1) ;
	if(PROFOM_ERR == err || DBH_ERR == err) return(err) ;
	if(RET_USER_ESC == err){
		InitFields(HV_CHAR,HV_SHORT,HV_LONG,HV_DOUBLE);
		ret(WriteFields((char *)&s_sth,START_FLD,END_FLD-200));

		s_sth.s_mesg[0] = HV_CHAR;
		DispMesgFld((char *)&s_sth);

		return(RET_USER_ESC) ;
	}

	return(NOERROR);

}	/* ReadScreen() */

/*----------------------------------------------------------------------*/
/* Reads the employee that the user wants to add or delete */
ReadKey()
{
	int	i;
	char	hold_emp[13];
	
#ifdef ENGLISH
	strcpy(s_sth.s_mesg,"Press ESC-F to Go Back to Fn:");
#else
	strcpy(s_sth.s_mesg,"Appuyer sur ESC-F pour retourner a Fn:");
#endif
	DispMesgFld((char *)&s_sth);

	strcpy(hold_emp,s_sth.s_emp_num);

	s_sth.s_emp_num[0] = LV_CHAR;

	i = ReadFields((char *)&s_sth,EMPLOYEE_FLD, EMPLOYEE_FLD,
		Validation, WindowHelp,1) ;
	if(PROFOM_ERR == i || DBH_ERR == i) return(i) ;
	if(RET_USER_ESC == i){
		strcpy(s_sth.s_emp_num,hold_emp);
		
		ret( WriteFields((char *)&s_sth,EMPLOYEE_FLD, EMPLOYEE_FLD) ) ;

		s_sth.s_mesg[0] = HV_CHAR;
		DispMesgFld((char *)&s_sth);

		return(RET_USER_ESC) ;
	}

	s_sth.s_mesg[0] = HV_CHAR;
	DispMesgFld((char *)&s_sth);

	return(NOERROR);

}	/*  ReadKey() */

/*-----------------------------------------------------------------------*/ 
/* Check to see if manual cheque is to be added or deleted .		 */
WriteManual(mode)
int mode;
{
	int	retval;

	scpy((char *)&pre_man_chq,(char *)&man_chq,sizeof(man_chq));

	strcpy(man_chq.mc_emp_numb,s_sth.s_emp_num);
	man_chq.mc_date = s_sth.s_cheq_date;
	man_chq.mc_fund = s_sth.s_fund_num;
	strcpy(man_chq.mc_acct,s_sth.s_acct_num);
	man_chq.mc_chq_numb = s_sth.s_cheq_num;
	strcpy(man_chq.mc_ded_code,s_sth.s_deduction);
	man_chq.mc_amount = s_sth.s_amount;

	if(mode != ADD) {
		retval = get_man_chq(&man_chq,UPDATE,3,e_mesg) ;
		if(retval < 0 || 
		(strcmp(s_sth.s_emp_num,man_chq.mc_emp_numb) != 0) ||
		s_sth.s_fund_num != man_chq.mc_fund ||
		(strcmp(s_sth.s_acct_num,man_chq.mc_acct) != 0) ||
		(strcmp(s_sth.s_deduction,man_chq.mc_ded_code) != 0)) {
			fomen("No Manual Check For That Employee");
			get();
			roll_back(e_mesg);
			return(retval);
		}
	}

	retval = put_man_chq(&man_chq,mode,e_mesg);
	if(retval < 0) {
		DispError((char *)&s_sth,e_mesg);
		roll_back(e_mesg);
		return(retval);
	}

	if(mode != ADD) {
		retval = rite_audit((char*)&s_sth,MAN_CHQ,mode,(char*)&man_chq,
			(char*)&pre_man_chq,e_mesg);
		if(retval==LOCKED) {
			DispError((char *)&s_sth,e_mesg);
			roll_back(e_mesg) ;
			return(LOCKED) ;
		}
		if(retval < 0 ){
#ifdef	ENGLISH
			DispError((char *)&s_sth,"ERROR in Saving Records"); 
#else
			DispError((char *)&s_sth,
					"ERREUR en conservant les fiches");
#endif
			DispError((char *)&s_sth,e_mesg);
			roll_back(e_mesg);
			return(retval);
		}
	}

	scpy((char *)&pre_man_chq,(char *)&man_chq,sizeof(man_chq));

	retval = WriteJr(mode);
	if(retval < 0) {
		DispError((char *)&s_sth,e_mesg);
		return(retval);
	}

	if(mode != ADD) {
		retval = rite_audit((char*)&s_sth,MAN_CHQ,mode,(char*)&jr_ent,
			(char*)&pre_jr_ent,e_mesg);
		if(retval==LOCKED) {
			DispError((char *)&s_sth,e_mesg);
			roll_back(e_mesg) ;
			return(LOCKED) ;
		}
		if(retval < 0 ){
#ifdef	ENGLISH
			DispError((char *)&s_sth,"ERROR in Saving Records"); 
#else
			DispError((char *)&s_sth,
					"ERREUR en conservant les fiches");
#endif
			DispError((char *)&s_sth,e_mesg);
			roll_back(e_mesg);
			return(retval);
		}
	}

	retval = commit(e_mesg) ;
	if(retval < 0) {
#ifdef ENGLISH
		DispError((char *)&s_sth,"ERROR in Saving Records"); 
#else
		DispError((char *)&s_sth,"ERREUR en conservant les fiches");
#endif
		DispError((char *)&s_sth,e_mesg);
		roll_back(e_mesg);
		return(retval);
	}

	return(NOERROR);

}	/* WriteManual() */
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

	return( NOERROR );

}	/* SetDupBuffers() */

/*----------------------------------------------------------------*/
/* Validation function() for Key and Header fields when PROFOM returns
  RET_VAL_CHK */

Validation()
{
	int	retval;

	switch(sr.curfld){
	case 	EMPLOYEE_FLD:
		if(s_sth.s_emp_num[0] == '\0'){
			s_sth.s_emp_num[0] = LV_CHAR;
			return(-1);
		}

		Right_Justify_Numeric(s_sth.s_emp_num ,(sizeof(emp.em_numb)-1));

		strcpy(emp.em_numb,s_sth.s_emp_num);
		retval = get_employee(&emp,BROWSE,0,e_mesg);
		if(retval == UNDEF){
			fomen("Invalid Employee Number, Please Re-enter");
			s_sth.s_emp_num[0] = LV_CHAR;
			return(-1);
		}

		if(retval < 0){
			DispError((char *)&s_sth,e_mesg);
			return(retval);
		}

		strcpy(s_sth.s_name,emp.em_first_name);
		strcat(s_sth.s_name," ");
		strcat(s_sth.s_name,emp.em_last_name);
		if(emp.em_add1[0] == '\0')
			s_sth.s_mail_adres[0] = ' ';
		else
			strcpy(s_sth.s_mail_adres,emp.em_add1);
		if(emp.em_add2[0] == '\0')
			s_sth.s_street[0] = ' ';
		else
			strcpy(s_sth.s_street,emp.em_add2);
		if(emp.em_add3[0] == '\0')
			s_sth.s_city[0] = ' ';
		else
			strcpy(s_sth.s_city,emp.em_add3);
		if(emp.em_add4[0] == '\0')
			s_sth.s_prov[0] = ' ';
		else
			strcpy(s_sth.s_prov,emp.em_add4);
		if(emp.em_pc[0] == '\0')
			s_sth.s_postal[0] = ' ';
		else
			strcpy(s_sth.s_postal,emp.em_pc);

		ret(WriteFields((char *)&s_sth,EMP_FIRST_FLD,POSTAL_FLD));

		strcpy(barg_unit.b_code,emp.em_barg);
		barg_unit.b_date = get_date();
		flg_reset(BARG);

		retval = get_n_barg(&barg_unit,BROWSE,0,BACKWARD,e_mesg);
		if(retval == EFL ||
			strcmp(barg_unit.b_code, emp.em_barg) != 0){
			sprintf(e_mesg,"Bargaining Unit does no exist: %s",
				emp.em_barg);
  			DispError((char *)&s_sth,e_mesg);
			s_sth.s_emp_num[0] = LV_CHAR;
			return(-1);
		}
		if(retval < 0){
  			DispError((char *)&s_sth,e_mesg);
			s_sth.s_emp_num[0] = LV_CHAR;
  			return(-1);
		}
		seq_over(BARG);

		break;

	case	FUND_FLD:
		if(pa_rec.pa_glmast[0] == YES) {
			ctl_rec.fund = s_sth.s_fund_num;
			retval = get_ctl(&ctl_rec,BROWSE,0,e_mesg);
			if(retval < 0 && retval != UNDEF) {
				DispError((char *)&s_sth);
				return(retval);
			}
			if(retval == UNDEF) {
				s_sth.s_fund_num = LV_SHORT;
				fomer("Fund Does not Exist");
				return(ERROR);
			}
		}
		break;

	case	BANK_ACCT_FLD	:	/* Acct No:	*/
		if (acnt_chk(s_sth.s_acct_num) < 0)  {
			s_sth.s_acct_num[0] = LV_CHAR ;
#ifdef ENGLISH
			fomer("Invalid GL Account Number");
#else
			fomer("Numero de compte G/L invalide");
#endif
			return(ERROR) ;
		}
		if( ((strcmp(s_sth.s_acct_num, ctl_rec.bank1_acnt)) != 0) &&
		    ((strcmp(s_sth.s_acct_num, ctl_rec.bank2_acnt)) != 0) ) {
#ifdef ENGLISH
			fomer("Must be a Bank Account Number") ;
#else
			fomer("Doit etre un numero de compte de banque") ;
#endif
			s_sth.s_acct_num[0] = LV_CHAR ;
			return(ERROR) ;
		}
			
		break ;

	case	CHEQ_NUM_FLD:

		if( s_sth.s_cheq_num < 1 ){
#ifdef ENGLISH
			fomen("Invalid cheque number");
#else
			fomen("Numero de cheque invalide");
#endif

			s_sth.s_cheq_num = LV_LONG;
			return(-1);
		}

		switch (s_sth.s_fn[0]) {
		case MANUAL:

			man_chq.mc_chq_numb = s_sth.s_cheq_num;

			retval = get_man_chq(&man_chq,BROWSE,2,e_mesg);

			if(retval == NOERROR){
				fomen("Invalid Cheque Number, Please Re-enter");
				s_sth.s_cheq_num = LV_LONG;
				return(retval);
			}

			chq_hist.ch_funds = s_sth.s_fund_num;
			strcpy( chq_hist.ch_accno, s_sth.s_acct_num );
			chq_hist.ch_chq_no = s_sth.s_cheq_num;
			retval = get_chqhist(&chq_hist,BROWSE,0,e_mesg);
			if( retval == NOERROR ){
				fomen("Invalid Cheque Number, Please Re-enter");
				s_sth.s_cheq_num = LV_LONG;
				return(retval);
			}

		}
		break;

	case	CHEQ_DATE_FLD:

		strcpy(man_chq.mc_emp_numb,s_sth.s_emp_num);
		man_chq.mc_date = s_sth.s_cheq_date;

		retval = get_man_chq(&man_chq,BROWSE,0,e_mesg);
		if(retval == NOERROR) {
			fomen("Cheque already exist for that date, Please Re-enter");
			s_sth.s_cheq_date = LV_LONG;
			return(-1); 
		}

		break;

	case	DEDUCTION_FLD:

		strcpy(deduction.dd_code,s_sth.s_deduction);
		strcpy(deduction.dd_pp_code,barg_unit.b_pp_code);
		retval = get_deduction(&deduction,BROWSE,0,e_mesg);
		if(retval < 0) {
			fomer("Deduction Code Does not Exist");
			s_sth.s_deduction[0] = LV_CHAR;
			return(ERROR);
		}
		break;

	case 	AMOUNT_FLD:
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

/*----------------------------------------------------------------*/
/* Show Help Windows, if applicable, when user gives ESC-H in key
   and header fields */

WindowHelp()
{
	int	retval ;
	char	temp_pp_code[7];

	temp_pp_code[0] = '\0';
	switch(sr.curfld){

	case	DEDUCTION_FLD:
		retval = deduction_hlp(s_sth.s_deduction,temp_pp_code,7,13);
		if(retval == DBH_ERR) return(retval);
		if(retval >= 0) redraw();
		if(retval == 0) return(ERROR);
		if(retval < 0) return(ERROR);

		break;

	case 	EMPLOYEE_FLD:
		retval = emp_hlp(s_sth.s_emp_num,7,13);
		if(retval == DBH_ERR) return(retval);
		if(retval >= 0) redraw();
		if(retval == 0) return(ERROR);
		if(retval < 0) return(ERROR);

		strcpy(emp.em_numb,s_sth.s_emp_num);
		retval = get_employee(&emp,BROWSE,0,e_mesg);
		if(retval == UNDEF){
			fomen("Invalid Employee Number, Please Re-enter");
			s_sth.s_emp_num[0] = LV_CHAR;
			return(-1);
		}

		if(retval < 0){
			DispError((char *)&s_sth,e_mesg);
			return(retval);
		}

		strcpy(s_sth.s_name,emp.em_first_name);
		strcat(s_sth.s_name," ");
		strcat(s_sth.s_name,emp.em_last_name);
		strcpy(s_sth.s_mail_adres,emp.em_add1);
		strcpy(s_sth.s_street,emp.em_add2);
		strcpy(s_sth.s_city,emp.em_add3);
		strcpy(s_sth.s_prov,emp.em_add4);
		strcpy(s_sth.s_postal,emp.em_pc);

		ret(WriteFields((char *)&s_sth,EMP_FIRST_FLD,POSTAL_FLD));

		strcpy(barg_unit.b_code,emp.em_barg);
		barg_unit.b_date = get_date();
		flg_reset(BARG);

		retval = get_n_barg(&barg_unit,BROWSE,0,BACKWARD,e_mesg);
		if(retval == EFL ||
			strcmp(barg_unit.b_code, emp.em_barg) != 0){
			sprintf(e_mesg,"Bargaining Unit does no exist: %s",
				emp.em_barg);
  			DispError((char *)&s_sth,e_mesg);
			s_sth.s_emp_num[0] = LV_CHAR;
			return(-1);
		}
		if(retval < 0){
  			DispError((char *)&s_sth,e_mesg);
			s_sth.s_emp_num[0] = LV_CHAR;
  			return(ERROR);
		}
		seq_over(BARG);
		break;

	default :
		fomer("No Help Window For This Field");
	}	/* Switch sr.curfld */

	return(NOERROR) ;

}	/* WindowHelp() */

/*-----------------------------------------------------------------------*/
/* Take the confirmation from user for the header part */

Confirm()
{
	int	err ;

	/* Options:
	   Add      - YALSNPC
	   Change   - YALSNPC
	   Delete   - YC
	*/

	for( ; ; ) {
	    switch(s_sth.s_fn[0]) {
	    case  MANUAL :		/* Add Manual */
#ifdef ENGLISH
		err = GetOption((char *)&s_sth,
		"Y(es), E(dit), C(ancel)"
		,"YEC");
#else
		err = GetOption((char *)&s_sth,
		"Y(es), E(dit), C(ancel)"
		,"YEC");
#endif
		break ;
	    case  DELETE :		/* Delete Manual */
#ifdef ENGLISH
		err = GetOption((char *)&s_sth,"Y(es), C(ancel)","YC");
#else
		err = GetOption((char *)&s_sth,"O(ui), A(nnul)","OA");
#endif
		break ;
	    }
	    if(err == PROFOM_ERR) return(err) ;

	    switch(err) {
	    case  YES  :
		return(YES);
	    case  EDIT:
		scpy((char *)&image,(char *)&s_sth,sizeof(image));
		SetDupBuffers(START_FLD,END_FLD,1); /* Off Dup Control */
	
		err = ReadScreen();
		break;
	    case  CANCEL :
#ifdef ENGLISH
		err = GetOption((char *)&s_sth,
				"Confirm the Cancel (Y/N)?", "YN") ;
#else
		err = GetOption((char *)&s_sth,
				"Confirmer l'annulation (O/N)?", "ON") ;
#endif
		if(err == YES) return(CANCEL) ;
		break ;
	    }	/* switch err */

	    if(err == PROFOM_ERR) return(err) ;
	    if(err == DBH_ERR) return(err) ;
	}	/* for(; ; ) */

}	/* Confirm() */

/*------------------------------------------------------------------------*/
/* Move High values to all data fields and clear the screen */

ClearScreen()
{
	/* Move High Values to Hedaer part */
	InitFields(HV_CHAR, HV_SHORT, HV_LONG, HV_DOUBLE) ;

	ret( WriteFields((char *)&s_sth,START_FLD, (END_FLD - 200)) );

	return(NOERROR);

}	/* ClearScreen() */

/*-------------------------------------------------------------------------*/
/* Initialize Screen Header with either Low or High Values */

InitFields( t_char, t_short,t_long,t_double )
char	t_char ;
short	t_short ;
long	t_long;
double	t_double;
{
	s_sth.s_fund_num = t_short;
	s_sth.s_acct_num[0] = t_char;	
	s_sth.s_cheq_num = t_long;
	s_sth.s_cheq_date = t_long;
	s_sth.s_deduction[0] = t_char;
	s_sth.s_amount = t_double;

	s_sth.s_name[0] = t_char;
	s_sth.s_mail_adres[0] = t_char;
	s_sth.s_street[0] = t_char;
	s_sth.s_city[0] = t_char;
	s_sth.s_prov[0] = t_char;
	s_sth.s_postal[0] = t_char;

	return(NOERROR) ;

}	/* InitFields() */

/*-------------------------------------------------------------------------*/
/* Write the journal entry for the deduction code */

WriteJr(mode)
int	mode;
{
	int	retval;

	jr_ent.jr_fund = deduction.dd_fund;
	jr_ent.jr_no = 32767;
	jr_ent.jr_date = s_sth.s_cheq_date;
	strcpy(jr_ent.jr_acct,deduction.dd_lia_acct);
	strcpy(jr_ent.jr_emp_numb,s_sth.s_emp_num);
	strcpy(jr_ent.jr_code,s_sth.s_deduction);
	jr_ent.jr_amount = s_sth.s_amount;

	if(mode == ADD) {
		retval = get_n_jr_ent(&jr_ent,UPDATE,0,BACKWARD,e_mesg);
		if(retval == EFL || s_sth.s_fund_num != jr_ent.jr_fund) {
			jr_ent.jr_fund = s_sth.s_fund_num;
			jr_ent.jr_no = 1;
		}
		else {
			jr_ent.jr_no = jr_ent.jr_no + 1;
		}
	}
	if(mode != ADD) {
		retval = get_jr_ent(&jr_ent,UPDATE,1,e_mesg) ;
		if(retval < 0 || 
		  (strcmp(s_sth.s_emp_num,jr_ent.jr_emp_numb) != 0) ||
		  (strcmp(s_sth.s_acct_num,jr_ent.jr_acct) != 0)) {
			fomen("No Manual Check For That Employee");
			get();
			roll_back(e_mesg);
			return(retval);
		}
	}

	retval = put_jr_ent(&jr_ent,mode,e_mesg);
	if(retval < 0) {
		DispError((char *)&s_sth,e_mesg);
		roll_back(e_mesg);
		return(retval);
	}

	return(NOERROR);
}

/*-------------------- E n d   O f   P r o g r a m ---------------------*/
