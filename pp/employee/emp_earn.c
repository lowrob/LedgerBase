/*-----------------------------------------------------------------------
Source Name: emp_earn.c 
System     : Ledger Base 
Module     : Personnel/Payroll
Created  On: 1991/10/18  
Created  By: Cathy Burns 

DESCRIPTION:
	Program to Change/Inquire the Earnings Information.
	This Program also writes Audit records for the changes.

Usage of SWITCHES when they are ON :

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
L. Robichaud   94/01/19		Make the reading of the pay param file
				the first thing done in the main process
				Earnings. Causes a problem with the next
				function otherwise.
------------------------------------------------------------------------*/

#define  MAINFL		EMPLOYEE  		/* main file used */


#include <stdio.h>
#include <ctype.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_pp.h>
#include <bfs_com.h>
#include <empldrvr.h>

#define	SYSTEM		"PESONNEL/PAYROLL"	/* Sub System Name */
#define	MOD_DATE	"18-OCT-91"		/* Program Last Modified */


/* PROFOM Field Numbers */
#define	LAST_SNUM	12
/* User Interface define constants */
#ifdef ENGLISH
#define ADDREC		'A'
#define CHANGE		'C'
#define DELETE		'D'
#define INQUIRE		'I'
#define EXITOPT		'E'
#define	NEXT		'N'
#define	PREV		'P'

#define	YES		'Y'
#define NO		'N'
#define	LINEEDIT	'L'
#define SCREENEDIT	'S'
#define	CANCEL		'C'

#define PERCENT		'P'
#define FIXED		'F'

#else
#define ADDREC		'R'
#define CHANGE		'C'
#define DELETE		'E'
#define INQUIRE		'I'
#define EXITOPT		'F'
#define	NEXT		'S'
#define	PREV		'P'

#define	YES		'O'
#define NO		'N'
#define	LINEEDIT	'L'
#define SCREENEDIT	'S'
#define	CANCEL		'A'

#define PERCENT		'P'
#define FIXED		'F'

#endif

/* PROFOM Related declarations */

#define PRGNM  		"emp_earn"
#define SCR_NAME	"emp_earn"

#define MAX_FIELD	17		/* Maximum field # to edit */

/* PROFOM Field Numbers */

/* Screen Control Variables */
#define	START_FLD	900 	/* Data entry starting field */
#define	END_FLD  	4400		/* screen end field */
#define	CHG_FLD		700 	/* Data entry starting field */

#define	KEY_START	500 	/* Key Start Field 	*/
#define	KEY_END		500 	/* Key Start Field 	*/

#define	FN_FLD  	400 	/* Fn: Field: 		*/
#define DEF_PF		1100	/* FED Exempt Field */
#define DEFERRED	1200	/* FED Exempt Field */
#define CPP_EXP		1300	/* CPP Exempt Field */
#define UIC_EXP		1400	/* UIC Exempt Field */
#define REG_FLD		1700	/* Registered Pension Plan Field */
#define TAX_EXP		2500	/* FED Exempt Field */

/* Profom 'C' structure for the screen "schl1" i.e., for Demographic data */
typedef struct	{

	char	s_pgm[11];		/* 100 Program Name String 10	*/ 
	long	s_sysdate;		/* 300 System date DATE YYYYFMMFDD */
	char	s_fn[2];		/* 400 Fn: field String 1	*/
	char	s_emp[13];		/* 500 Employee No String 13 */
	char	s_name[31];		/* 600 Employee No String 30 */
	short	s_field;		/* 700 Field Number Numeric 99	*/
	short	s_nopp;			/* 900 No of Pay Periods 	*/
	char	s_status[4];		/* 1000 Status String 3	*/
	char	s_def_pf[2];		/* 1100 Empl First String 15	*/
	double	s_def_inc;		/* 1150 Deferred Amount */
	char	s_cpp_exp[2];		/* 1200 Empl First String 15	*/
	char	s_uic_exp[2];		/* 1300 Empl Middle String 15	*/
	double	s_ytd_inc;		/* 1650 YTD Income 9,999,999.99 */
	double	s_uic_rate;		/* 1400 Deferred Amount */
	char	s_reg_pen[7];		/* 1500 Mailing Addr. String 30 */
	char	s_regdesc[31];		/* 1550 Mailing Addr. String 30 */
	double	s_reg_prior;		/* 1700 Increase Tax Amount */
	double	s_ytd_def;		/* 1750 YTD Deferred Amount */
	double	s_reg_opt;		/* 1700 Increase Tax Amount */
	double	s_ytd_cpp;		/* 1850 YTD CPP Amount */
	double	s_reg_nonm;		/* 1700 Increase Tax Amount */
	double	s_cpp_pen;		/* 1950 CPP Pens Earnings Amount */
	char	s_tax_exp[2];		/* 1600 Empl Middle String 15	*/
	double	s_uic_prem;		/* 2050 UIC Premiums Amount */
	double	s_inc_tax;		/* 1700 Increase Tax Amount */
	double	s_uic_ins;		/* 2150 UIC Insurable Amount */
	double	s_other_fed;		/* 1800 Other Federal Tax Amount */
	double	s_pen_rate1;		/* 2250 Pension contribution 1 Amount */
	double	s_ho_ded;		/* 1900 Housing Allowance Amount */
	double	s_pen_rate2;		/* 2350 Pension contribution 2 Amount */
	double	s_ann_ded;		/* 2000 Annual Deductions Amount */
	double	s_pen_rate3;		/* 2450 Pension contribution 3 Amount */
	double	s_fam_all;		/* 2100 Family Allowance Amount */
	double	s_pen_prior;
	double	s_old_age;		/* 2200 Old Age Security Amount */
	double	s_pen_opt;
	double	s_union_dues;		/* 2300 Union Dues Amount */
	double	s_pen_nonm;
	double	s_net_tax_cr;		/* 2400 Deferred Amount */
	double	s_ytd_inc_tax;		/* 2475 YTD Income Tax Amount */
	char	s_mesg[78];		/* 2500 Message Line String 77	*/
	char	s_opt[2];		/* 2600 Message Option String X */ 
	} s_struct;

static	s_struct	s_sth, image ;

static	Pay_per		payper;
static	Emp_earn	emp_earn;
static	Emp_ins		emp_ins;
static	Pay_param	pay_param;
static	Reg_pen		reg_pen;
static	Barg_unit	barg_unit;

static	int	Validate() ;
static	int	WindowHelp() ;
extern	double	D_Roundoff();


Earnings( )
{
	int	err ;
	err = get_pay_param(&pay_param, BROWSE, 1, e_mesg) ;
	if(err < 0) {
		if(err == UNDEF) {
#ifdef ENGLISH
			fomen("Payroll Parameters Are Not Setup..");
#else
			fomen("Parametres Payroll ne sont pas etablis..");
#endif
			get() ;
		}
		else{
			fomen(e_mesg);
			get() ;
		}
		return(err);
	}
	
	err = InitScreen() ; 	/* Initialization routine */

	if(emp_rec.em_numb[0] != '\0') {
		flg_reset(EMPLOYEE);

		err = get_n_employee(&emp_rec, BROWSE, 0, FORWARD, e_mesg);
#ifndef ORACLE
		seq_over(EMPLOYEE);
#endif
		if(ERROR == err)return(DBH_ERR) ;
		if(EFL == err) {
			fomen("No More Records....");
			get();
			return(NOERROR) ;
		}

		err = UsrBargVal(BROWSE,emp_rec.em_numb,emp_rec.em_barg,1,e_mesg);
		if(err < 0){
			DispError((char *)&s_sth,e_mesg);
		}
		else{
			strcpy( s_sth.s_emp,emp_rec.em_numb);
			err = ShowScreen(BROWSE);
		}
	}

	err = Process(); 	/* Initiate Process */

	return( err ) ;

} /* DemoGraph() */
/*----------------------------------------------------------------*/
/* Initialize screen before going to process options */
static int
InitScreen()
{
	int	err ;

	/* move screen name to Profom status structure */
	strcpy(sr.scrnam,NFM_PATH);
	strcat(sr.scrnam,SCR_NAME) ;

	strcpy(s_sth.s_pgm,PRGNM);

	s_sth.s_sysdate = get_date();	/* get Today's Date in YYYYMMDD format*/
	s_sth.s_field = HV_SHORT ;
	s_sth.s_mesg[0] = HV_CHAR ;
	s_sth.s_opt[0] = HV_CHAR ;

	/* Initialize Key Fields. So that, if user selectes 'N' option
	   immediately after invoking program, then he gets the first
	   record in the file */

	s_sth.s_emp[0] = '\0';

	/* Move High Values to data fields and Display the screen */
	err = ClearScreen() ;
	if(NOERROR != err) return(err) ;

	return(NOERROR) ;
}	/* InitScreen() */
/*--------------------------------------------------------------------------*/
/* Close nessary files and environment before exiting program               */

static int
CloseRtn() 
{
	/* Set terminal back to normal mode from PROFOM */
	fomcs();
	fomrt();

	close_dbh();	/* Close files */

	return(NOERROR);
}	/* CloseRtn() */

/*-------------------------------------------------------------------*/
/* Get Option from user and call corresponding function */
static	int
Process()
{
	int	err;

	for( ; ; ){
		/* Get the Fn: from the user */
		if((err = ReadFunction()) != NOERROR) return(err) ;

		err = ProcFunction() ;	/* Process Fn */

		if(QUIT == err)		return(NOERROR) ;	/* Exit */
		if(JUMP == err)		return(JUMP) ;
		if(NOACCESS == err) {
			fomen(e_mesg);
			get();
		}
		if(PROFOM_ERR == err)	return(PROFOM_ERR);  /* PROFOM ERROR */
		if(DBH_ERR == err) {
			DispError((char*)&s_sth, e_mesg);
#ifdef	ENGLISH
			sprintf(e_mesg,"%s %d Dberror: %d Errno: %d",
				"System Error... Iserror:",
				iserror, dberror, errno);
#else
			sprintf(e_mesg,"%s %d Dberror: %d Errno: %d",
				"Erreur du systeme... Iserror:",
				iserror, dberror, errno);
#endif
			DispError((char*)&s_sth, e_mesg);
			return(DBH_ERR);	/* DBH ERROR */
		}
	}      /*   end of the for( ; ; )       */
}	/* Process() */
/*----------------------------------------------------------------*/
/* Display the Function (Fn:) options and get the option from the user */

static int
ReadFunction()
{
	/* Display Fn: options */
#ifdef ENGLISH
	fomer("C(hange), I(nquire), F(orward), B(ackward), N(ext), P(rev), S(creen), E(xit)");
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

static int
ProcFunction()
{
	int retval, mode;

	switch (s_sth.s_fn[0]) {
	case CHANGE  :			/* CHANGE */
		CHKACC(retval,UPDATE,e_mesg);
		mode = UPDATE;
		return( Change() ) ;
	case INQUIRE  :			/* Inquire */
		CHKACC(retval,BROWSE,e_mesg);
		mode = BROWSE;
		return( Inquire() ) ;
	case NEXT_RECORD  :			/* Inquire */
		CHKACC(retval,BROWSE,e_mesg);
		return( Next(FORWARD) ) ;
	case PREV_RECORD  :			/* Inquire */
		CHKACC(retval,BROWSE,e_mesg);
		return( Next(BACKWARD) ) ;
	case NEXT_SCR  : 	/* Next Screen */
		Cur_Option += 1 ;	/* Set to Next Screen */
		return( JUMP ) ; 
	case PREV_SCR  : 	/* Next Screen */
		Cur_Option -= 1 ;	/* Set to Next Screen */
		return( JUMP ) ; 
	case SCREEN  :			/* Inquire */
		CHKACC(retval,BROWSE,e_mesg);
		return( Screen() ) ;

	case EXITOPT  :
		return(QUIT);

	default   : 
		return(NOERROR);
	}  /*   end of the switch statement */

	return(retval);
}	/* ProcFunction() */
/*-----------------------------------------------------------------------*/
/* Change. Students Study halls and update the files if a day/semester   */
/* is changed to NO delete record.			  		 */
/*-----------------------------------------------------------------------*/
static int
Change()
{
	int	err ;

	err = SelectRecord() ;
	if(NOERROR != err) return(err) ;

	for( ; ; ) {
		err = Confirm() ;
		if(err != YES) {
			roll_back(e_mesg);
			break;
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
}	/* Change() */
/*-----------------------------------------------------------------------*/
/* Show Student Employee Demographic Date                                */
static int
Inquire()
{
	int	err ;

	err = SelectRecord() ;
	if(NOERROR != err) return(err) ;

	return(NOERROR) ;
}	/* Inquire() */
/*----------------------------------------------------------*/
/* Show the next or previous Employees                      */

static int
Next(direction)
int	direction ;
{
	int retval;

	strcpy(emp_rec.em_numb,s_sth.s_emp);
	if (flg_start(EMPLOYEE) != direction) {
		inc_str(emp_rec.em_numb, sizeof(emp_rec.em_numb)-1, 
			direction);
		flg_reset(EMPLOYEE);
	}

	for( ; ; ){
	  retval = get_n_employee(&emp_rec, BROWSE, 0, direction, e_mesg);
#ifndef ORACLE
	  seq_over(EMPLOYEE);
#endif
	  if(ERROR == retval)return(DBH_ERR) ;
	  if(EFL == retval) {
		fomen("No More Records....");
		get();
		return(NOERROR) ;
	  }

	  retval = UsrBargVal(BROWSE,emp_rec.em_numb,emp_rec.em_barg,0,e_mesg);
	  if(retval < 0){
		if(retval == UNDEF){
			DispError((char *)&s_sth,e_mesg);
			continue;
		}
	  }
	  else{
		break;
	  }
	}
	strcpy( s_sth.s_emp,emp_rec.em_numb);
	retval = ShowScreen(BROWSE);

	return( retval ) ;
}	/* Next() */
/*------------------------------------------------------------*/
/* Allows User to Add Record to File */
/*------------------------------------------------------------*/
static int
GetDetails()
{
	int	i ;
	int	err;

	err = ReadScreen(ADD);
	if(PROFOM_ERR == err || DBH_ERR == err) return(err) ;
	if(RET_USER_ESC == err){
		return(RET_USER_ESC) ;
	}

	for( ; ; ) {
		i = Confirm() ;
		if(i != YES) break;

		i = WriteRecords(ADD) ;
		if(i < 0) {
			if(i == LOCKED) continue;
		}
		break;
	}
	if(i != NOERROR) return(i);
	return(NOERROR) ;
}	/* GetDetails() */
/*------------------------------------------------------------*/
/*------------------------------------------------------------*/
static int
ReadScreen(mode)
int	mode;
{
	int err;

	scpy((char *)&image,(char *)&s_sth,sizeof(image));

	if(mode == ADD) {
		SetDupBuffers(START_FLD,END_FLD,0); /* Off Dup Control */
		SetDupBuffers(DEFERRED,DEFERRED,0);
	}
	else {
		SetDupBuffers(START_FLD,END_FLD,1); /* Off Dup Control */
		SetDupBuffers(DEFERRED,DEFERRED,1);
	}
	
	InitFields(LV_CHAR,LV_SHORT,LV_LONG,LV_DOUBLE);
#ifdef	ENGLISH
	strcpy(s_sth.s_mesg,"Press ESC-F to Terminate Edit");
#else
	strcpy(s_sth.s_mesg,"Appuyer sur ESC-F pour terminer l'ajustement");
#endif
	DispMesgFld((char*)&s_sth) ;

	/* Read data area of screen in single fomrd() */

	err = ReadFields((char*)&s_sth, START_FLD, END_FLD-200, Validate,
			WindowHelp, 1) ;
	if(err == DBH_ERR || err == PROFOM_ERR) return(err) ;
	if(PROFOM_ERR == err || DBH_ERR == err) return(err) ;
	if(RET_USER_ESC == err){
		if(mode == ADD) {
			InitFields(HV_CHAR,HV_SHORT,HV_LONG,HV_DOUBLE);
			ret(WriteFields((char *)&s_sth,START_FLD,END_FLD-125));
		}
		else {
		/* When user gives ESC-F while changing fields, assumption is
		* he completed his changes, and remaining fields are same as
		* old. But, at this point STH will be having low values in the 
		* remaining fields. So move the old values form the linked list.
		*/

			err = CopyBack((char *)&s_sth,(char *)&image,
				sr.curfld, END_FLD);
			if(err == PROFOM_ERR) return(err);
		}
		s_sth.s_mesg[0] = HV_CHAR;
		DispMesgFld((char *)&s_sth);

		return(RET_USER_ESC) ;
	}

	return(NOERROR);
}
/*----------------------------------------------------------*/
/* Get the key and show the record */
static int
SelectRecord()
{
	int	err ;

	for( ;; ) {
		err = ReadKey();
		if(err != NOERROR) return(err) ;
	
		strcpy(emp_rec.em_numb,s_sth.s_emp);
		err = get_employee(&emp_rec,BROWSE,0,e_mesg);
		if(err < 0)  {
			fomen(e_mesg);
			get();
			if( err == EFL ) continue;
			return(err);
		}
		break;
	}

	err = ShowScreen(BROWSE);

	return(NOERROR);
}	/* SelectRecord() */
/*----------------------------------------------------------------------*/
/* Get the Student Study Hall key from user. In ADD mode disable dup buffers, */
/* other modes enable dup buffers and show the current key as a default key */
static int
ReadKey()
{
	int	i;
	char	hold_emp[13];
	
	/* In Add mode turn off dup control for key fields.
	   Other modes reverse it */
	if(s_sth.s_fn[0] == ADDREC){	/* ADD */
		SetDupBuffers(KEY_START,KEY_END,0); /* Off Dup Control */
	}
	else {
		SetDupBuffers(KEY_START,KEY_END,1);
	}

#ifdef ENGLISH
	strcpy(s_sth.s_mesg,"Press ESC-F to Go Back to Fn:");
#else
	strcpy(s_sth.s_mesg,"Appuyer sur ESC-F pour retourner a Fn:");
#endif
	DispMesgFld((char *)&s_sth);

	strcpy(hold_emp,s_sth.s_emp);

	s_sth.s_emp[0] = LV_CHAR;

	i = ReadFields((char *)&s_sth,KEY_START, KEY_END,
		Validate, WindowHelp,1) ;
	if(PROFOM_ERR == i || DBH_ERR == i) return(i) ;
	if(RET_USER_ESC == i){
		strcpy(s_sth.s_emp,hold_emp);
		
		ret( WriteFields((char *)&s_sth,KEY_START, KEY_END) ) ;

		s_sth.s_mesg[0] = HV_CHAR;
		DispMesgFld((char *)&s_sth);

		return(RET_USER_ESC) ;
	}

	s_sth.s_mesg[0] = HV_CHAR;
	DispMesgFld((char *)&s_sth);

	return(NOERROR);
}	/*  ReadKey() */
/*-----------------------------------------------------------------------*/ 
/* Check to see if record is to be added, changed or deleted .		 */
/*-----------------------------------------------------------------------*/ 
static int
WriteRecords(mode)
int mode;
{
	int	i,retval;

	strcpy(emp_rec.em_numb,s_sth.s_emp);
	if(mode != ADD) {
		retval = get_employee(&emp_rec,UPDATE,0,e_mesg);
		if(retval < 0) {
			fomer(e_mesg);
			return(retval);
		}
	}
	
	CopyToRecord();

	retval = put_employee(&emp_rec,mode,e_mesg);
	if(retval < 0) {
		DispError((char *)&s_sth,e_mesg);
		roll_back(e_mesg);
		return(retval);
	}

	if(mode != ADD) {
		retval = rite_audit((char*)&s_sth,EMPLOYEE,mode,(char*)&emp_rec,
			(char*)&pre_emp,e_mesg);
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
}
/*----------------------------------------------------------------*/
/* Set Duplication buffers for fields 				  */
static int
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
/* Validation function() for Key and Header fields when PROFOM returns
  RET_VAL_CHK */

static int
Validate()
{
	int	save_nextfld, save_endfld ;
	int	j, i, retval;
	char	part1[5], part2[5];
	long	value, number, final;

	switch(sr.curfld){
	case KEY_START:
		Right_Justify_Numeric(s_sth.s_emp,
			sizeof(s_sth.s_emp)-1);
		strcpy(emp_rec.em_numb,s_sth.s_emp);
		retval = get_employee(&emp_rec,BROWSE,0,e_mesg);
		if(retval < 0) {
			fomer("Employee Does not Exist");
			s_sth.s_emp[0] = LV_CHAR;
			return(ERROR);
		}
		retval = UsrBargVal(UPDATE,emp_rec.em_numb,emp_rec.em_barg,1,e_mesg);
		if(retval < 0){
			DispError((char *)&s_sth,e_mesg);
		  	s_sth.s_emp[0] = LV_CHAR;
			return(-1);
		}
		strcpy(s_sth.s_name, emp_rec.em_last_name);
		strcat(s_sth.s_name, ", ");
		strcat(s_sth.s_name, emp_rec.em_first_name);
		strcat(s_sth.s_name, " ");
		strcat(s_sth.s_name, emp_rec.em_mid_name);
		break;
	case CPP_EXP:
		if(s_sth.s_cpp_exp[0] != YES && s_sth.s_cpp_exp[0] != NO) {
			fomen("Must be Y(es) or N(o)");
			s_sth.s_cpp_exp[0] = LV_CHAR;
			return(ERROR);
		}
		break;
	case UIC_EXP:
		if(s_sth.s_uic_exp[0] != YES && s_sth.s_uic_exp[0] != NO) {
			fomen("Must be Y(es) or N(o)");
			s_sth.s_uic_exp[0] = LV_CHAR;
			return(ERROR);
		}
		break;
	case DEF_PF:
		if(s_sth.s_def_pf[0] == '\0'){
			sr.curfld += 100;
			break;
		}
		if(s_sth.s_def_pf[0] != PERCENT && s_sth.s_def_pf[0] != FIXED){
		        fomen("Must be P(ercent) or F(ixed amount)");
			s_sth.s_def_pf[0] = LV_CHAR;
			return(ERROR);
		}
		break;
	case DEFERRED:
		if(s_sth.s_def_pf[0] == PERCENT && s_sth.s_def_inc > 100.00) {
			fomen("Percent cannot be greater than 100 %");
			s_sth.s_def_inc = LV_DOUBLE;
			return(ERROR);
		}
		break;
	case REG_FLD:
		if(s_sth.s_reg_pen[0] == '\0' || s_sth.s_reg_pen[0] == ' ') {
			s_sth.s_reg_pen[0] = '\0';
			s_sth.s_regdesc[0] = HV_CHAR;
			s_sth.s_reg_prior = 0;
			s_sth.s_reg_opt = 0;
			s_sth.s_reg_nonm = 0;
			ret(WriteFields((char*)&s_sth,REG_FLD,REG_FLD+600));
			sr.curfld += 500;
			return(NOERROR);
		}

		Right_Justify_Numeric(s_sth.s_reg_pen,
			sizeof(s_sth.s_reg_pen)-1);
		strcpy(reg_pen.rg_code,s_sth.s_reg_pen);
		reg_pen.rg_pp_code[0] = '\0';
		flg_reset(REG_PEN);
		retval = get_n_reg_pen(&reg_pen,BROWSE,0,FORWARD,e_mesg);
		if(retval < 0)  {
			fomer(e_mesg);
			s_sth.s_reg_pen[0] = LV_CHAR;
			return(ERROR);
		}
		if(strcmp(s_sth.s_reg_pen,reg_pen.rg_code) != 0) {
			fomen("Registered Pension Plan Code Does Not Exist - Please Re-enter");
			s_sth.s_reg_pen[0] = LV_CHAR;
			return(ERROR);
		}
		strcpy(s_sth.s_regdesc, reg_pen.rg_desc);
		if(strcmp(reg_pen.rg_code, "PSPP") == 0){
			s_sth.s_reg_prior = LV_DOUBLE;
			s_sth.s_reg_opt = LV_DOUBLE;
			s_sth.s_reg_nonm = LV_DOUBLE;
		}
		else {
			s_sth.s_reg_prior = HV_DOUBLE;
			s_sth.s_reg_opt = HV_DOUBLE;
			s_sth.s_reg_nonm = HV_DOUBLE;
		}
		ret( WriteFields((char *)&s_sth, REG_FLD+100, REG_FLD+600) ) ;
		break;
	case TAX_EXP:
		if(s_sth.s_tax_exp[0] != YES && s_sth.s_tax_exp[0] != NO) {
			fomen("Must be Y(es) or N(o)");
			s_sth.s_tax_exp[0] = LV_CHAR;
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
}	/* Validate() */
/*----------------------------------------------------------------*/
/* Show Help Windows, if applicable, when user gives ESC-H in key
   and header fields */

static int
WindowHelp()
{
	int	retval ;
	short	reccod ;
	char	temp_pp[7];

	temp_pp[0] = '\0';

	switch(sr.curfld){
	case KEY_START:
		retval = emp_hlp(s_sth.s_emp,7,13);
		if(retval == DBH_ERR) return(retval);
		if(retval >= 0) redraw();
		if(retval == 0) return(ERROR);
		if(retval < 0) return(ERROR);
		strcpy(emp_rec.em_numb,s_sth.s_emp);
		retval = get_employee(&emp_rec,BROWSE,0,e_mesg);
		if(s_sth.s_fn[0] == ADDREC) {
			if(retval != UNDEF) {
				fomer("Employee Already Exist");
				s_sth.s_emp[0] = LV_CHAR;
				return(ERROR);
			}
		}
		else {
			if(retval < 0)  {
				fomer(e_mesg);
				s_sth.s_emp[0] = LV_CHAR;
				return(ERROR);
			}
		}
		break;
	case REG_FLD:
		retval = reg_pen_hlp(s_sth.s_reg_pen,temp_pp,7,13);
		if(retval == DBH_ERR) return(retval);
		if(retval >= 0) redraw();
		if(retval == 0) return(ERROR);
		if(retval < 0) return(ERROR);
		strcpy(reg_pen.rg_code,s_sth.s_reg_pen);
		strcpy(reg_pen.rg_pp_code,temp_pp);
		retval = get_reg_pen(&reg_pen,BROWSE,0,e_mesg);
		if(retval < 0)  {
			fomer(e_mesg);
			s_sth.s_reg_pen[0] = LV_CHAR;
			return(ERROR);
		}
		strcpy(s_sth.s_regdesc, reg_pen.rg_desc);
		if(strcmp(reg_pen.rg_code, "PSPP") == 0){
			s_sth.s_reg_prior = LV_DOUBLE;
			s_sth.s_reg_opt = LV_DOUBLE;
			s_sth.s_reg_nonm = LV_DOUBLE;
		}
		ret( WriteFields((char *)&s_sth, REG_FLD+100, REG_FLD+600) ) ;
		break;
	default :
		fomer("No Help Window For This Field");
	}	/* Switch sr.curfld */

	return(NOERROR) ;
}	/* HdrAndKeyWindowHelp() */
/*-----------------------------------------------------------------------*/
/*                                                                       */
static	int
EmpYtd()
{
	int 	retval;
	double  ytd_gross,ytd_def,ytd_tax,ytd_cpp,ytd_cpp_pen,ytd_uic;
	double	ytd_uic_earn,ytd_ben,ytd_reg,ytd_pen_rate1;
	double	ytd_pen_rate2,ytd_pen_rate3;
	double	ytd_pen_nonm, ytd_pen_prior, ytd_pen_opt;

	ytd_gross = 0.00;
	ytd_def = 0.00;
	ytd_tax = 0.00;
	ytd_cpp = 0.00;
	ytd_cpp_pen = 0.00;
	ytd_uic = 0.00;
	ytd_uic_earn = 0.00;
	ytd_ben = 0.00;
	ytd_reg = 0.00;
	ytd_pen_rate1 = 0.00;
	ytd_pen_rate2 = 0.00;
	ytd_pen_rate3 = 0.00;
	ytd_pen_prior = 0.00;
	ytd_pen_opt = 0.00;
	ytd_pen_nonm = 0.00;

	strcpy(emp_earn.en_numb,emp_rec.em_numb);
	emp_earn.en_date = 0;
	emp_earn.en_pp = 0;
	emp_earn.en_week = 0;

	flg_reset(EMP_EARN) ;

	for( ; ; ) {
		retval = get_n_emp_earn(&emp_earn,UPDATE,0,FORWARD,e_mesg) ;
		if( retval == EFL ||
			strcmp(emp_earn.en_numb,emp_rec.em_numb) != 0)
			break;

		if( retval < 0) {
			roll_back(e_mesg);
			DispError((char *)&s_sth,e_mesg) ;
			return(retval) ;
		}
		if(emp_earn.en_date < pay_param.pr_cal_st_dt)
			continue;

		ytd_gross += D_Roundoff(emp_earn.en_reg_inc +
					 emp_earn.en_high_inc);
		ytd_def += D_Roundoff(emp_earn.en_def_inc);
		ytd_tax += D_Roundoff(emp_earn.en_tax);
		ytd_cpp += D_Roundoff(emp_earn.en_cpp);
		ytd_cpp_pen += D_Roundoff(emp_earn.en_cpp_pen);
		ytd_uic += D_Roundoff(emp_earn.en_uic);
		ytd_reg += D_Roundoff(emp_earn.en_reg1 + emp_earn.en_reg2 +
					 emp_earn.en_reg3);
		ytd_pen_rate1 += D_Roundoff(emp_earn.en_reg1);
		ytd_pen_rate2 += D_Roundoff(emp_earn.en_reg2);
		ytd_pen_rate3 += D_Roundoff(emp_earn.en_reg3);
		ytd_pen_prior += D_Roundoff(emp_earn.en_reg_prior);
		ytd_pen_opt += D_Roundoff(emp_earn.en_reg_opt);
		ytd_pen_nonm += D_Roundoff(emp_earn.en_reg_nonm);
	}

	seq_over(EMP_EARN) ;

	strcpy(emp_ins.in_numb,emp_rec.em_numb);
	emp_ins.in_pp = 0;
	emp_ins.in_date = 0;

	flg_reset(EMP_INS) ;

	for( ; ; ) {
		retval = get_n_emp_ins(&emp_ins,UPDATE,0,FORWARD,e_mesg) ;
		if( retval == EFL ||
			strcmp(emp_ins.in_numb,emp_rec.em_numb) != 0)
			break;

		if(emp_ins.in_date < pay_param.pr_cal_st_dt)
			continue;

		if( retval < 0) {
			roll_back(e_mesg);
			DispError((char *)&s_sth,e_mesg) ;
			return(retval) ;
		}
		ytd_uic_earn += D_Roundoff(emp_ins.in_uic_ins);
	}
	seq_over(EMP_INS);

	s_sth.s_ytd_inc	= ytd_gross;
	s_sth.s_ytd_def = ytd_def ;
	s_sth.s_ytd_cpp = ytd_cpp ;
	s_sth.s_cpp_pen = ytd_cpp_pen ;
	s_sth.s_uic_prem = ytd_uic;
	s_sth.s_uic_ins = ytd_uic_earn ;
	s_sth.s_pen_rate1 = ytd_pen_rate1 ;
	s_sth.s_pen_rate2 = ytd_pen_rate2 ;
	s_sth.s_pen_rate3 = ytd_pen_rate3 ;
	s_sth.s_ytd_inc_tax = ytd_tax;
	s_sth.s_pen_opt = ytd_pen_opt;
	s_sth.s_pen_prior = ytd_pen_prior;
	s_sth.s_pen_nonm = ytd_pen_nonm;

	return(NOERROR);
}
/*-----------------------------------------------------------------------*/
/* Take the confirmation from user for the header part */

static int
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
	    case  ADDREC :		/* Add */
#ifdef ENGLISH
		err = GetOption((char *)&s_sth,
		"Y(es), S(creen Edit), L(ine edit), C(ancel)"
		,"YASLC");
#else
		err = GetOption((char *)&s_sth,
		"O(ui), S(creen edit), L(ine edit), A(nnul)"
		,"ORSLA");
#endif
		break ;
	    case  CHANGE :		/* Change */
#ifdef ENGLISH
		err = GetOption((char *)&s_sth,
		"Y(es), S(creen edit), L(ine edit), C(ancel)","YSLC");
#else
		err = GetOption((char *)&s_sth,
		"O(ui), S(creen edit), L(ine edit), A(nnul)","OSLA");
#endif
		break ;
	    case  DELETE :		/* Delete */
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
	    case  SCREENEDIT:
		err = ReadScreen(UPDATE);
		break;
	    case  LINEEDIT  :
		err = ChangeFields();
		break ;
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
/*-----------------------------------------------------------*/
static int
ChangeFields()
{
	int	i, retval ;
	int	fld_no, end_fld;

	SetDupBuffers(START_FLD, END_FLD - 125, 1);
	SetDupBuffers(DEFERRED,DEFERRED,1);

	/* Get The Field to Be Modified */
#ifdef	ENGLISH
	strcpy(s_sth.s_mesg,"Enter RETURN to Terminate Edit");
#else
	strcpy(s_sth.s_mesg,"Appuyer sur RETURN pour terminer l'ajustement");
#endif
	DispMesgFld((char *)&s_sth); ;
        
     	for (; ;) {
		s_sth.s_field = LV_SHORT;
		retval = ReadFields((char *)&s_sth,CHG_FLD,CHG_FLD,
			(int (*)())NULL,(int (*)())NULL, 1);

		if (retval < 0) return(-1);
		if (retval == RET_USER_ESC) break;	/* User enter ESC-F */

       		if (s_sth.s_field == 0 ) break;  /* Finished changing fields */

		if (s_sth.s_field > MAX_FIELD) {
			fomen("Invalid Field Number");
			get();
			continue;
		}

		switch(s_sth.s_field) {
		case 1:
			s_sth.s_def_pf[0] = LV_CHAR;
			s_sth.s_def_inc = LV_DOUBLE;
			fld_no = DEF_PF;
			end_fld = DEFERRED;
			break;
		case 2:
			fld_no = end_fld = 1300;
			break;
		case 3:
			fld_no = end_fld = 1400;
			break;
		case 4:
			fld_no = end_fld = 1600;
			break;
		case 5:
			fld_no = 1700;
			end_fld = 2300;
			s_sth.s_reg_pen[0] = LV_CHAR;
			break;
		case 6:
			fld_no = end_fld = 1900;
			break;
		case 7:
			fld_no = end_fld = 2100;
			break;
		case 8:
			fld_no = end_fld = 2300;
			break;
		case 9:
			fld_no = end_fld = 2500;
			break;
		case 10:
			fld_no = end_fld = 2700;
			break;
		case 11:
			fld_no = end_fld = 2900;
			break;
		case 12:
			fld_no = end_fld = 3100;
			break;
		case 13:
			fld_no = end_fld = 3300;
			break;
		case 14:
			fld_no = end_fld = 3500;
			break;
		case 15:
			fld_no = end_fld = 3700;
			break;
		case 16:
			fld_no = end_fld = 3900;
			break;
		case 17:
			fld_no = end_fld = 4100;
			break;
		}

			
/*		fld_no = (START_FLD+200) + (100 * (s_sth.s_field-1));
		if(fld_no == DEF_PF) {
			s_sth.s_def_pf[0] = LV_CHAR;
			s_sth.s_def_inc = LV_DOUBLE;
			end_fld = DEFERRED;
		}
		else
			end_fld = fld_no;*/

		retval = ReadFields((char *)&s_sth,fld_no, end_fld,
			Validate, WindowHelp,1) ;
	}
     	s_sth.s_field = HV_SHORT ;
	if ( WriteFields((char *)&s_sth,CHG_FLD, CHG_FLD) < 0 ) return(-1);

     	s_sth.s_mesg[0] = HV_CHAR ;
     	DispMesgFld((char *)&s_sth);

	if(SetDupBuffers(START_FLD, END_FLD-125, 0)<0) return(PROFOM_ERR);
	if(SetDupBuffers(DEFERRED,DEFERRED,0)<0) return(PROFOM_ERR);

	return(NOERROR);
}
/*-----------------------------------------------------------*/
/* Move Header to Screen Hdr Fields */
static int
ShowScreen(mode)
int	mode;
{
	int	i, retval;

	strcpy(s_sth.s_name, emp_rec.em_last_name);
	strcat(s_sth.s_name, ", ");
	strcat(s_sth.s_name, emp_rec.em_first_name);
	strcat(s_sth.s_name, " ");
	strcat(s_sth.s_name, emp_rec.em_mid_name);
	s_sth.s_name[30] = '\0';
	strcpy(s_sth.s_status, emp_rec.em_status);

	strcpy(barg_unit.b_code,emp_rec.em_barg);
	barg_unit.b_date = get_date();
	flg_reset(BARG);

	retval = get_n_barg(&barg_unit,BROWSE,0,BACKWARD,e_mesg);
	if(retval == EFL ||
		strcmp(barg_unit.b_code, emp_rec.em_barg) != 0){
  		DispError((char *)&s_sth,"Bargaining Unit does not Exist");
		return(NOERROR);
	}
	if(retval < 0){
  		DispError((char *)&s_sth,e_mesg);
  		return(ERROR);
	}
	seq_over(BARG);

	strcpy(payper.pp_code,barg_unit.b_pp_code);
	payper.pp_year = 0;
	flg_reset(PAY_PERIOD);

	retval = get_n_pay_per(&payper,BROWSE,0,FORWARD,e_mesg);
	if(retval == EFL || strcmp(payper.pp_code,barg_unit.b_pp_code)!=0){
		fomer("Error Reading Pay Period File");
		return(-1);
	}
	if(retval < 0){	
		DispError((char *)&s_sth,e_mesg);
		return(-1);
	}
	seq_over(PAY_PERIOD);

	s_sth.s_nopp = payper.pp_numb ;
	
	retval = EmpYtd();
	strcpy(reg_pen.rg_code,emp_rec.em_reg_pen);
	reg_pen.rg_pp_code[0] = '\0';
	flg_reset(REG_PEN);
	retval = get_n_reg_pen(&reg_pen,BROWSE,0,FORWARD,e_mesg);
	if((retval < 0) ||
	   (strcmp(emp_rec.em_reg_pen,reg_pen.rg_code) != 0)) 
		strcpy(s_sth.s_regdesc, "** Not Setup **");
	else
		strcpy(s_sth.s_regdesc, reg_pen.rg_desc);

	CopyToScreen();

	ret( WriteFields((char *)&s_sth, KEY_START, END_FLD - 125) ) ;

	return(NOERROR) ;
}	/* ShowScreen() */
/*------------------------------------------------------------------------*/
/* Move High values to all data fields and clear the screen */

static int
ClearScreen()
{
	/* Move High Values to Hedaer part */
	InitFields(HV_CHAR, HV_SHORT, HV_LONG, HV_DOUBLE) ;

	ret( WriteFields((char *)&s_sth,START_FLD, (END_FLD - 125)) );

	return(NOERROR);
}	/* ClearScreen() */
/*-------------------------------------------------------------------------*/
/* Initialize Screen Header with either Low or High Values */

static int
InitFields( t_char, t_short,t_long,t_double )
char	t_char ;
short	t_short ;
long	t_long;
double	t_double;
{
	int	i;

	s_sth.s_def_pf[0] = t_char;
	s_sth.s_def_inc = t_double;	
	s_sth.s_cpp_exp[0] = t_char;
	s_sth.s_uic_exp[0] = t_char;
	s_sth.s_uic_rate = t_double;
	s_sth.s_reg_pen[0] = t_char;		
	s_sth.s_tax_exp[0] = t_char;
	if(t_double == HV_DOUBLE) {
		s_sth.s_reg_prior = t_double;
		s_sth.s_reg_opt = t_double;
		s_sth.s_reg_nonm = t_double;
		s_sth.s_name[0] = t_char;
		s_sth.s_status[0] = t_char;	
		s_sth.s_regdesc[0] = t_char;	
		s_sth.s_nopp = t_short;
		s_sth.s_ytd_inc = t_double;
		s_sth.s_ytd_def = t_double;
		s_sth.s_ytd_cpp = t_double;
		s_sth.s_cpp_pen = t_double;
		s_sth.s_uic_prem = t_double;
		s_sth.s_uic_ins = t_double;	
		s_sth.s_pen_rate1 = t_double;	
		s_sth.s_pen_rate2 = t_double;
		s_sth.s_pen_rate3 = t_double;
		s_sth.s_pen_prior = t_double;
		s_sth.s_pen_opt = t_double;
		s_sth.s_pen_nonm = t_double;
		s_sth.s_ytd_inc_tax = t_double;
	}
	s_sth.s_inc_tax = t_double;	
	s_sth.s_other_fed = t_double;
	s_sth.s_ho_ded = t_double;
	s_sth.s_ann_ded = t_double;
	s_sth.s_fam_all = t_double;	
	s_sth.s_old_age = t_double;	
	s_sth.s_union_dues = t_double;
	s_sth.s_net_tax_cr = t_double;	

	return(NOERROR) ;
}	/* InitFields() */

/*-----------------------------------------------------------------------*/ 
/*                                                                       */
/* Copy the data record fields to Screen                                 */
/*                                                                       */
/*-----------------------------------------------------------------------*/
static int
CopyToRecord()
{
	strcpy(emp_rec.em_def_pf, s_sth.s_def_pf);
	emp_rec.em_def_inc = s_sth.s_def_inc;
	strcpy(emp_rec.em_cpp_exp,s_sth.s_cpp_exp);
	strcpy(emp_rec.em_uic_exp,s_sth.s_uic_exp);
	strcpy(emp_rec.em_tax_exp,s_sth.s_tax_exp);
	emp_rec.em_uic_rate = s_sth.s_uic_rate;
	strcpy(emp_rec.em_reg_pen,s_sth.s_reg_pen);
	emp_rec.em_reg_prior = s_sth.s_reg_prior;
	emp_rec.em_reg_opt = s_sth.s_reg_opt;
	emp_rec.em_reg_nonm = s_sth.s_reg_nonm;
	emp_rec.em_inc_tax = s_sth.s_inc_tax;
	emp_rec.em_other_fed = s_sth.s_other_fed;
	emp_rec.em_union_dues = s_sth.s_union_dues;
	emp_rec.em_ho_ded = s_sth.s_ho_ded;
	emp_rec.em_net_tax_cr = s_sth.s_net_tax_cr;
	emp_rec.em_ann_ded = s_sth.s_ann_ded;
	emp_rec.em_fam_all = s_sth.s_fam_all;
	emp_rec.em_old_age = s_sth.s_old_age;

	return(NOERROR) ; 
}	/* CopyToRecord() */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Copy the Screen fields to data record */
static int
CopyToScreen()
{
	strcpy(s_sth.s_def_pf, emp_rec.em_def_pf);
	s_sth.s_def_inc = emp_rec.em_def_inc ;
	strcpy(s_sth.s_cpp_exp,emp_rec.em_cpp_exp);
	strcpy(s_sth.s_uic_exp,emp_rec.em_uic_exp);
	strcpy(s_sth.s_tax_exp,emp_rec.em_tax_exp);
	s_sth.s_uic_rate = emp_rec.em_uic_rate ;
	strcpy(s_sth.s_reg_pen,emp_rec.em_reg_pen);
	if(strcmp(emp_rec.em_reg_pen, "PSPP") != 0){
		s_sth.s_reg_prior = HV_DOUBLE;
		s_sth.s_reg_opt = HV_DOUBLE;
		s_sth.s_reg_nonm = HV_DOUBLE;
	}
	else{
		s_sth.s_reg_prior = emp_rec.em_reg_prior;
		s_sth.s_reg_opt = emp_rec.em_reg_opt;
		s_sth.s_reg_nonm = emp_rec.em_reg_nonm;
	}
	s_sth.s_inc_tax = emp_rec.em_inc_tax ;
	s_sth.s_other_fed = emp_rec.em_other_fed ;
	s_sth.s_union_dues = emp_rec.em_union_dues ;
	s_sth.s_ho_ded = emp_rec.em_ho_ded ;
	s_sth.s_net_tax_cr = emp_rec.em_net_tax_cr ;
	s_sth.s_ann_ded = emp_rec.em_ann_ded ;
	s_sth.s_fam_all = emp_rec.em_fam_all ;
	s_sth.s_old_age = emp_rec.em_old_age ;

	return(NOERROR);

}	/* Copy to Screen */


/*-----------------------------------------------------------------------*/
/*                                                                       */
static
Screen()
{
	int	retval;

	s_sth.s_field = LV_SHORT;
	retval = ReadFields((char *)&s_sth,CHG_FLD,CHG_FLD,
		(int (*)())NULL,(int (*)())NULL, 1);

	if (retval < 0) return(-1);
	if (retval == RET_USER_ESC) return(NOERROR);	/* User enter ESC-F */

       	if (s_sth.s_field == 0 ) return(NOERROR); /* Finished changing fields */

	if (s_sth.s_field > MAX_SCREEN) {
		fomen("Invalid Field Number");
		get();
		s_sth.s_field = LV_SHORT;
		return(NOERROR);
	}

	switch (s_sth.s_field) {
	case SCR_1  :		/* SCR - 1  'DEMOGRAPHIC' */
	case SCR_2  :		/* SCR - 2  'EMPLOYMENT' */
	case SCR_3  :		/* SCR - 3  'RESPONSIBILITY' */
	case SCR_5  :		/* SCR - 5  'MISCELLANIOUS' */
	case SCR_6  :		/* SCR - 6  'BENEFIT' */
	case SCR_7  :		/* SCR - 7  'DEDUCTION' */
	case SCR_8  :		/* SCR - 8  'CSB/LOAN' */
	case SCR_9  :		/* SCR - 9  'GARNISHMENT' */
	case SCR_10 :		/* SCR - 10 'CHEQUE LOCATION' */
	case SCR_11 :		/* SCR - 11 'ATTENDANCE' */
	case SCR_12 :		/* SCR - 12 'SENIORITY' */
	case SCR_13 :		/* SCR - 13 'TEACHER QUALIFICATION' */
	case SCR_14 :		/* SCR - 14 'TEACHER ASSIGNMENT' */
	case SCR_15 :		/* SCR - 15 'COMPETITION' */
	case SCR_16 :		/* SCR - 16 'STATUS' */
		Cur_Option = s_sth.s_field ;
		return( JUMP ) ;
	default   : 
		return(NOERROR);
	}  /*   end of the switch statement */
}
/*-------------------- E n d   O f   P r o g r a m ---------------------*/

