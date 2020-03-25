/*-----------------------------------------------------------------------
Source Name: csbloan.c
System     : Personnel/Payroll.
Created  On: 12th Nov 91.
Created  By: Andre Cormier

DESCRIPTION:

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
------------------------------------------------------------------------*/
#define MAINFL		LOAN		/* main file used */

#define	SYSTEM		"Setup"			/* Sub System Name */
#define	MOD_DATE	"12-NOV-91"		/* Progran Last Modified */

#include <stdio.h>
#include <ctype.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_pp.h>
#include <bfs_com.h>
#include <empldrvr.h>
#include <repdef.h>

/* User Interface define constants */
#ifdef ENGLISH
#define ADDREC		'A'
#define CHANGE		'C'
#define DELETE		'D'
#define INQUIRE		'I'
#define EXITOPT		'E'

#define	YES		'Y'
#define NO		'N'
#define	LINEEDIT	'L'
#define SCREENEDIT	'S'
#define	CANCEL		'C'

#define	FIXED		'F'
#define	PERCENT		'P'

#else
#define ADDREC		'R'
#define CHANGE		'C'
#define DELETE		'E'
#define INQUIRE		'I'
#define EXITOPT		'F'

#define	YES		'O'
#define NO		'N'
#define	LINEEDIT	'L'
#define SCREENEDIT	'S'
#define	CANCEL		'A'

#define	FIXED		'F'
#define	PERCENT		'P'

#endif

/* PROFOM Releted declarations */

#define	SCR_NAME	"csbloan"	/* PROFOM screen Name */

#define MAX_FIELD	5		/* Maximum field # to edit */

/* PROFOM Screen STH file */

/* Field PROFOM numbers */
#define	FN_FLD		400	/* Fn: */
#define	KEY_START	500	/* Employee Number */
#define	KEY_MID		600	/* CSB/LOAN Code */
#define	KEY_END		700	/* Sequence Number */
#define	CHG_FLD		800	/* Field: */

#define EMP_NAME_FLD	1000
#define DESC_FLD	1100	/*  Description: */
#define AMT_LOAN_FLD 	1200	/*  Amount of the loan */
#define BALANCE_FLD	1300	/*  Outstanding balance */
#define NUM_PAY_PER_FLD	1400	/*  Number of pay period */
#define	AMT_FLG_FLD	1500	/*  Amt Flag P or F */
#define AMT_DED_FLD	1600	/*  Deduction amount */
#define INT_RATE_FLD	1700	/*  Interest rate: */
#define INT_DUE_FLD	1800	/*  Interest due */
#define TOTAL_DUE_FLD	1900	/*  Total due fo pay period */
#define PP_FLD1		2000	/*  Pay Period 1 */
#define PP_FLD2		2100	/*  Pay Period 2 */
#define PP_FLD3		2200	/*  Pay Period 3 */
#define PP_FLD4		2300	/*  Pay Period 4 */
#define PP_FLD5		2400	/*  Pay Period 5 */


#define START_FLD	1000	/* First Field on screen */
#define	END_FLD		2600	/* Last Field of the screen */
/* bargain.sth - header for C structure generated by PROFOM EDITOR */

typedef struct	{

	char	s_pgname[11];	/* 100 STRING X(10) */
	long	s_rundate;	/* 300 DATE YYYYFMMFDD */
	char	s_fn[2];	/* 400 STRING X */
	char	s_emp_numb[13];	/* 500 STRING X(12) */
	char	s_csb_loan[7];	/* 600 STRING X(6) */
	short	s_seq_no;	/* 700 STRING X(6) */
	short	s_field;	/* 800 NUMERIC 99 */

	char	s_emp_name[31];	/* 1000 STRING X(30) */
	char	s_desc[31];	/* 1100 STRING X(30) */
	double	s_amt_loan;	/* 1200 NUMERIC 9,999,999.99 */
	double	s_balance;	/* 1300 NUMERIC 9,999,999.99 */
	short	s_numb_pp;	/* 1400 NUMERIC 99 */
	char	s_amt_flg[2];	/* 1500 STRING X */
	double	s_amt_ded;	/* 1600 NUMERIC 9,999,999.99 */
	double	s_int_rate;	/* 1700 NUMERIC 999.99 */
	double	s_int_due;	/* 1800 NUMERIC 9,999,999.99 */
	double	s_tot_due;	/* 1900 NUMERIC 9,999,999.99 */
	char	s_pp[5][2];	/* 2000 -2400 STRING X */

	char	s_mesg[78];	/* 2500 STRING X(76) */
	char	s_resp[2];	/* 2600 STRING X */
} s_struct;

static	s_struct  s_sth,image;	/* PROFOM Screen Structure */

static	char 	e_mesg[100];  		/* dbh will return err msg in this */

/*static	Emp		emp;*/
static	Csb_loan	csb_loan;
static	Emp_loan	emp_loan,	pre_emp_loan;

int	Validation() ;
int	WindowHelp();

static	int	mode;

void	free() ;
char	*malloc() ;
CsbLoan()
{
	int 	retval, err;

	retval = Initialize();	/* Initialization routine */
	if(retval < 0)	return(retval);

	if(emp_rec.em_numb[0] != '\0') {
		flg_reset(EMPLOYEE);
		retval = get_n_employee(&emp_rec,BROWSE,0,FORWARD,e_mesg);
		if(retval < 0){
			fomen(e_mesg);
			return(retval);
		}

		strcpy(emp_loan.el_numb, emp_rec.em_numb);
		emp_loan.el_code[0] = '\0';
		emp_loan.el_seq = 0;
		flg_reset(EMP_LOAN);

		retval = get_n_emp_loan(&emp_loan, BROWSE, 0, FORWARD, e_mesg);
#ifndef ORACLE
		seq_over(EMP_LOAN);
#endif
		if(ERROR == retval)return(DBH_ERR) ;

		err = UsrBargVal(BROWSE,emp_rec.em_numb,emp_rec.em_barg,1,e_mesg);
		if(err < 0){
			DispError((char *)&s_sth,e_mesg);
		}
		else{
		  if(strcmp(emp_loan.el_numb,emp_rec.em_numb)==0 &&
				 retval !=EFL){
			strcpy(s_sth.s_emp_numb,emp_rec.em_numb);
			retval = ShowScreen();
		  }
		}
	}

	retval = Process();

	err = CloseRtn();			/* return to menu */
	if(err < 0)	return(err);

	return(retval);
}
/*-------------------------------------------------------------------*/
/* Initialize PROFOM */
static
Initialize()
{
	int	err ;

	err = InitScreen() ;		/* Initialize Screen */
	if(NOERROR != err) return(err) ;

	return(NOERROR) ;
}	/* Initialize() */

/*--------------------------------------------------------------------------*/
/* Close nessary files and environment before exiting program               */
static
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
static
InitScreen()
{
	int	err ;

	/* move screen name to Profom status structure */
	strcpy(sr.scrnam,NFM_PATH);
	strcat(sr.scrnam,SCR_NAME);

	strcpy(s_sth.s_pgname,"csbloan");

	s_sth.s_rundate = get_date();	/* get Today's Date in YYYYMMDD format*/
	s_sth.s_field = HV_SHORT ;
	s_sth.s_mesg[0] = HV_CHAR ;
	s_sth.s_resp[0] = HV_CHAR ;
	s_sth.s_emp_name[0] = HV_CHAR;
	s_sth.s_desc[0] = HV_CHAR;

	/* Initialize Key Fields. So that, if user selectes 'N' option
	   immediately after invoking program, then he gets the first
	   record in the file */

	s_sth.s_emp_numb[0] = LV_CHAR;
	s_sth.s_csb_loan[0] = LV_CHAR;
	s_sth.s_seq_no = LV_SHORT;

	SetDupBuffers(KEY_START,KEY_END,0); /* Off Dup Control */

	/* Move High Values to data fields and Display the screen */
	err = ClearScreen() ;
	if(NOERROR != err) return(err) ;

	return(NOERROR) ;
}	/* InitScreen() */

/*-------------------------------------------------------------------*/
/* Get Option from user and call corresponding function */

static
Process()
{
	int err;

	for( ; ; ){

		/* Get the Fn: option from the user */
		if((err = ReadFunction()) != NOERROR) return(err) ;

		err = ProcFunction() ;	/* Process Function */

		if(err == JUMP) return(JUMP); 
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

static
ReadFunction()
{
	/* Display Fn: options */
#ifdef ENGLISH
	fomer("A(dd), C(hange), I(nquire), F(orward), B(ack), N(ext), P(rev), S(creen), E(xit)");
#else
	fomer("R(ajouter), C(hanger), I(nter), F(orward), B(ack), S(uiv), P(rec), E(cran), F(in)");
#endif
	/* Read Fn: field to get the option */
	sr.nextfld = FN_FLD ;
	fomrf( (char *)&s_sth );
	ret(err_chk(&sr));	/* Check for PROFOM error */

	return(NOERROR) ;
}	/* ReadFunction() */
/*----------------------------------------------------------------*/
/* Process the user selected Fn: option */

static
ProcFunction()
{
	int retval;

	switch (s_sth.s_fn[0]) {
	case ADDREC  :			/* ADD */
		CHKACC(retval,ADD,e_mesg);
		mode = ADD;
		return( Add() ) ;
	case CHANGE  :			/* CHANGE */
		CHKACC(retval,UPDATE,e_mesg);
		mode = UPDATE;
		return( Change() ) ;
	case DELETE  :			/* CHANGE */
		CHKACC(retval,UPDATE,e_mesg);
		mode = P_DEL;
		return( Delete() ) ;
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

	case EXITOPT  :			/* Exit */
		return( QUIT ) ;
	}  /*   end of the switch statement */

	return(retval);
}	/* ProcFunction() */
/*----------------------------------------------------------------------*/
/* Adding.  Get the unique Key, accept details and update the files */
static
Add()
{
	int	err ;

	err = ReadKey();
	if(err != NOERROR) return(err) ;

	/* Clear The Screen */
	err = ClearScreen();
	if(err != NOERROR) return(err) ;


	strcpy(emp_rec.em_numb,s_sth.s_emp_numb);
	get_employee(&emp_rec,BROWSE,0,e_mesg);
	strcpy(s_sth.s_emp_name, emp_rec.em_last_name);
	strcat(s_sth.s_emp_name, ", ");
	strcat(s_sth.s_emp_name, emp_rec.em_first_name);
	strcat(s_sth.s_emp_name, " ");
	strcat(s_sth.s_emp_name, emp_rec.em_mid_name);

	strcpy(csb_loan.cs_code,s_sth.s_csb_loan);
	get_loan(&csb_loan,BROWSE,0,e_mesg);
	strcpy(s_sth.s_desc,csb_loan.cs_desc);

	if ( WriteFields((char *)&s_sth,KEY_END,DESC_FLD ) < 0 ) 
		return(-1);

	err = GetDetails() ;
	if(PROFOM_ERR == err || DBH_ERR == err) return(err) ;
	if(err < 0 || CANCEL == err) {
		roll_back(e_mesg) ;	/* Unlock the locked Records */
		return(ClearScreen()) ;	/* Clear the Screen */
	}

	return(NOERROR);
}	/* Add() */
/*-----------------------------------------------------------------------*/
/* Change. Students Study halls and update the files if a day/semester   */
/* is changed to NO delete record.			  		 */
/*-----------------------------------------------------------------------*/
static
Change()
{
	int	err ;

	err = SelectRecord() ;
	if(NOERROR != err) return(err) ;

	strcpy(emp_rec.em_numb,s_sth.s_emp_numb);
	get_employee(&emp_rec,BROWSE,0,e_mesg);
	strcpy(s_sth.s_emp_name, emp_rec.em_last_name);
	strcat(s_sth.s_emp_name, ", ");
	strcat(s_sth.s_emp_name, emp_rec.em_first_name);
	strcat(s_sth.s_emp_name, " ");
	strcat(s_sth.s_emp_name, emp_rec.em_mid_name);

	strcpy(csb_loan.cs_code,s_sth.s_csb_loan);
	get_loan(&csb_loan,BROWSE,0,e_mesg);
	strcpy(s_sth.s_desc,csb_loan.cs_desc);

	if ( WriteFields((char *)&s_sth,KEY_END,DESC_FLD ) < 0 ) 
		return(-1);


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
/* Delete. Student Study Hall Records.  */
/*-----------------------------------------------------------------------*/
static
Delete()
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

		err = WriteRecords(P_DEL) ;
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
/*-----------------------------------------------------------------------*/
/* Show Student Student halls base on users input */
static
Inquire()
{
	int	err ;

	err = SelectRecord() ;
	if(NOERROR != err) return(err) ;

	strcpy(emp_rec.em_numb,s_sth.s_emp_numb);
	get_employee(&emp_rec,BROWSE,0,e_mesg);
	strcpy(s_sth.s_emp_name, emp_rec.em_last_name);
	strcat(s_sth.s_emp_name, ", ");
	strcat(s_sth.s_emp_name, emp_rec.em_first_name);
	strcat(s_sth.s_emp_name, " ");
	strcat(s_sth.s_emp_name, emp_rec.em_mid_name);

	strcpy(csb_loan.cs_code,s_sth.s_csb_loan);
	get_loan(&csb_loan,BROWSE,0,e_mesg);
	strcpy(s_sth.s_desc,csb_loan.cs_desc);

	if ( WriteFields((char *)&s_sth,KEY_END,DESC_FLD ) < 0 ) 
		return(-1);

	return(NOERROR) ;
}	/* Inquire() */
/*----------------------------------------------------------*/
/* Show the next or previous Students Study halls */

static
Next(direction)
int	direction ;
{
	int retval;


	if(flg_start(EMPLOYEE) != direction) {
		inc_str(emp_rec.em_numb,sizeof(emp_rec.em_numb)-1,direction);
		flg_reset(EMPLOYEE);
	}
	for( ; ; ){
	  retval = get_n_employee(&emp_rec, BROWSE, 0, direction, e_mesg);
	  if(ERROR == retval)return(DBH_ERR) ;
	  if(EFL == retval) {
		fomen("No More Records....");
		get();
		return(NOERROR) ;
	  }

	  strcpy(s_sth.s_emp_numb,emp_rec.em_numb);

	  retval = UsrBargVal(BROWSE,emp_rec.em_numb,emp_rec.em_barg,0,e_mesg);
	  if(retval < 0){
		return(ERROR);
	  }
	  else{
		break;
	  }
	  strcpy(emp_loan.el_numb,emp_rec.em_numb);
	  emp_loan.el_code[0] = '\0';
	  emp_loan.el_seq = 0;

	  flg_reset(EMP_LOAN);
 
	  retval = get_n_emp_loan(&emp_loan, BROWSE, 0, FORWARD, e_mesg);
#ifndef ORACLE
	  seq_over(EMP_LOAN);
#endif
	  if(strcmp(emp_loan.el_numb,emp_rec.em_numb) != 0) 
		continue;
	}


	retval = ShowScreen();

	return( retval ) ;
}	/* Next() */
/*------------------------------------------------------------*/
/* Allows User to Add Record to File */
/*------------------------------------------------------------*/
static
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
static
ReadScreen(mode)
int	mode;
{
	int err;

	scpy((char *)&image,(char *)&s_sth,sizeof(image));

	if(mode == ADD) {
		SetDupBuffers(START_FLD,END_FLD,0); /* Off Dup Control */
	}
	else {
		SetDupBuffers(START_FLD,END_FLD,1); /* Off Dup Control */
	}
	
	InitFields(LV_CHAR,LV_SHORT,LV_DOUBLE);

	err = ReadFields((char *)&s_sth,START_FLD, END_FLD-200,
		Validation, WindowHelp,1) ;
	if(PROFOM_ERR == err || DBH_ERR == err) return(err) ;
	if(RET_USER_ESC == err){
		if(mode == ADD) {
			InitFields(HV_CHAR,HV_SHORT,HV_DOUBLE);
			ret(WriteFields((char *)&s_sth,START_FLD,END_FLD-200));
		}
		else {
		/* When user gives ESC-F while changing fields, assumption is
		* he completed his changes, and remaining fields are same as
		* old. But, at this point STH will be having low values in the 
		* remaining fields. So move the old values form the linked list.
		*/

			if(sr.curfld == AMT_FLG_FLD) {
				sr.curfld = NUM_PAY_PER_FLD;
			}
			if(sr.curfld == AMT_DED_FLD) {
				sr.curfld = NUM_PAY_PER_FLD;
			}

			err = CopyBack((char *)&s_sth,(char *)&image,
				sr.curfld, END_FLD);
			if(err == PROFOM_ERR) return(err);
		}

		/*s_sth.s_balance = s_sth.s_amt_loan;*/
		s_sth.s_int_due = s_sth.s_int_rate * s_sth.s_balance / 100;
		if(s_sth.s_amt_flg[0] == 'F')
			s_sth.s_tot_due = s_sth.s_amt_ded + s_sth.s_int_due;
		else
			s_sth.s_tot_due = 0;
		ret( WriteFields((char *)&s_sth,START_FLD, (END_FLD - 200)) );

		s_sth.s_mesg[0] = HV_CHAR;
		DispMesgFld((char *)&s_sth);

		return(RET_USER_ESC) ;
	}

	return(NOERROR);
}
/*----------------------------------------------------------*/
/* Get the key and show the record */
static
SelectRecord()
{
	int	err ;

	err = ReadKey();
	if(err != NOERROR) return(err) ;

	err = ShowScreen();

	return(NOERROR);
}	/* SelectRecord() */
/*----------------------------------------------------------------------*/
/* Get the Student Study Hall key from user. In ADD mode disable dup buffers, */
/* other modes enable dup buffers and show the current key as a default key */
static
ReadKey()
{
	int	i;
	char	hold_numb[13];
	char	hold_code[7];
	
	/* In Add mode turn off dup control for key fields.
	   Other modes reverse it */
	if(s_sth.s_fn[0] == ADDREC){	/* ADD */
		SetDupBuffers(KEY_START,KEY_END,0); /* Off Dup Control */
	}
	else {
		SetDupBuffers(KEY_START,KEY_END,0);
	}

#ifdef ENGLISH
	strcpy(s_sth.s_mesg,"Press ESC-F to Go Back to Fn:");
#else
	strcpy(s_sth.s_mesg,"Appuyer sur ESC-F pour retourner a Fn:");
#endif
	DispMesgFld((char *)&s_sth);

	strcpy(hold_numb,s_sth.s_emp_numb);
	strcpy(hold_code,s_sth.s_csb_loan);

	s_sth.s_emp_numb[0] = LV_CHAR;
	s_sth.s_csb_loan[0] = LV_CHAR;

	if(s_sth.s_fn[0] == ADDREC) {
		i = ReadFields((char *)&s_sth,KEY_START, KEY_MID,
			Validation, WindowHelp,1) ;
	}
	else {
		i = ReadFields((char *)&s_sth,KEY_START, KEY_END,
			Validation, WindowHelp,1) ;

	}


	if(PROFOM_ERR == i || DBH_ERR == i) return(i) ;
	if(RET_USER_ESC == i){
		strcpy(s_sth.s_emp_numb,hold_numb);
		strcpy(s_sth.s_csb_loan,hold_code);
		
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
static
WriteRecords(mode)
int mode;
{
	int	i,retval;

	strcpy(emp_loan.el_numb,s_sth.s_emp_numb);
	strcpy(emp_loan.el_code,s_sth.s_csb_loan);
	emp_loan.el_seq = s_sth.s_seq_no;
	if(mode != ADD) {
		retval = get_emp_loan(&emp_loan,UPDATE,0,e_mesg);
		if(retval < 0) {
			fomer(e_mesg);
			return(retval);
		}
	}
	emp_loan.el_amount = s_sth.s_amt_loan;
	emp_loan.el_amnt_out = s_sth.s_balance;
	emp_loan.el_pp_num = s_sth.s_numb_pp;
	emp_loan.el_pp_num_el = 0;
	strcpy(emp_loan.el_amt_flg,s_sth.s_amt_flg);
	emp_loan.el_pp_amount = s_sth.s_amt_ded;
	emp_loan.el_int = s_sth.s_int_rate;
	for(i=0 ; i<5 ; i++) {
		strcpy(emp_loan.el_ded_pp[i], s_sth.s_pp[i]);
	}

	retval = put_emp_loan(&emp_loan,mode,e_mesg);
	if(retval < 0) {
		DispError((char *)&s_sth,e_mesg);
		roll_back(e_mesg);
		return(retval);
	}

	if(mode != ADD) {
		retval = rite_audit((char*)&s_sth,EMP_LOAN,mode,(char*)&emp_loan,
			(char*)&pre_emp_loan,e_mesg);
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
static
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

static
Validation()
{
	int	i, retval;

	switch(sr.curfld){
	case KEY_START:
		Right_Justify_Numeric(s_sth.s_emp_numb,sizeof(emp_rec.em_numb)-1);
		strcpy(emp_rec.em_numb,s_sth.s_emp_numb);

		retval = get_employee(&emp_rec,BROWSE,0,e_mesg);
		if (retval != NOERROR) {
			s_sth.s_emp_numb[0] = LV_CHAR;
			fomen("Could Not Find The Employee Number - Please Re-enter");
			return(retval);
		}

		retval = UsrBargVal(mode,emp_rec.em_numb,emp_rec.em_barg,1,e_mesg);
		if(retval < 0){
			DispError((char *)&s_sth,e_mesg);
		  	s_sth.s_emp_numb[0] = LV_CHAR;
			return(-1);
		}
		if(s_sth.s_fn[0] != ADDREC) {

			strcpy(emp_loan.el_numb,s_sth.s_emp_numb);
			emp_loan.el_code[0] = '\0';
			emp_loan.el_seq = 0;

			flg_reset(EMP_LOAN);

			retval = get_n_emp_loan(&emp_loan,BROWSE,0,FORWARD,e_mesg);
			if (retval == EFL || 
			strcmp(emp_loan.el_numb,s_sth.s_emp_numb) != 0) {
				s_sth.s_emp_numb[0] = LV_CHAR;
				fomen("Could Not Find The Employee Number - Please Re-enter");
				return(retval);
			}
		}

		break;

	case KEY_MID:
		strcpy(csb_loan.cs_code,s_sth.s_csb_loan);

		retval = get_loan(&csb_loan,BROWSE,0,e_mesg);
		if(retval != NOERROR) {
			s_sth.s_csb_loan[0] = LV_CHAR;
			fomen("Could Not Find CSB/Loan Code - Please Re-enter");
			return(retval);
		}
		if(s_sth.s_fn[0] != ADDREC) {
			s_sth.s_seq_no = LV_SHORT;
		}

		if(s_sth.s_fn[0] == ADDREC) {

			strcpy(emp_loan.el_numb,s_sth.s_emp_numb);
			strcpy(emp_loan.el_code,s_sth.s_csb_loan);
			emp_loan.el_seq = 32767;
			flg_reset(EMP_LOAN);

			retval = get_n_emp_loan(&emp_loan,BROWSE,0,BACKWARD,e_mesg);
			if(retval == EFL || 
			strcmp(s_sth.s_emp_numb,emp_loan.el_numb) != 0 ||
			strcmp(s_sth.s_csb_loan,emp_loan.el_code) != 0) {
				s_sth.s_seq_no = 1;
			}
			else {
				s_sth.s_seq_no = emp_loan.el_seq + 1;
			}
		}

		break;

	case KEY_END:
		if(s_sth.s_fn[0] != ADDREC) {
			strcpy(emp_loan.el_numb,s_sth.s_emp_numb);
			strcpy(emp_loan.el_code,s_sth.s_csb_loan);
			emp_loan.el_seq = s_sth.s_seq_no;

			retval = get_emp_loan(&emp_loan,BROWSE,0,e_mesg);
			if(retval != NOERROR) {
				s_sth.s_seq_no = LV_SHORT;
				fomen("Sequence Number Does Not Exist For That Employee");
				return(retval);
			}	
		}
		break;

	case	DESC_FLD:
		if(s_sth.s_desc[0] == '\0') {
			fomer("This is a Required Field");
			return(ERROR);
		}
		break;
	case	AMT_LOAN_FLD:
		break;

	case	BALANCE_FLD:
		if(s_sth.s_balance > s_sth.s_amt_loan){
			fomer("The Outstanding Balance can not be Greater than the Amount of the Loan");
			s_sth.s_balance = LV_DOUBLE;
			return(ERROR);
		}
		break;

	case	NUM_PAY_PER_FLD:
		if(s_sth.s_numb_pp != 0) {
			s_sth.s_amt_flg[0] = FIXED;
			s_sth.s_amt_ded = s_sth.s_amt_loan / s_sth.s_numb_pp;
		}	
		else {
			s_sth.s_amt_flg[0] = LV_CHAR;
			s_sth.s_amt_ded = LV_DOUBLE;
		}

		break;
	case	AMT_FLG_FLD:
		if(s_sth.s_numb_pp == 0) {
			if(s_sth.s_amt_flg[0] != FIXED &&
			   s_sth.s_amt_flg[0] != PERCENT) {
				fomer("Must Be F(ixed) or P(ercentage)");
				s_sth.s_amt_flg[0] = LV_CHAR;
				return(ERROR);
			}
		}
		break;
	case	AMT_DED_FLD:
		if(s_sth.s_numb_pp == 0) {
			if(s_sth.s_amt_flg[0] == PERCENT) {
				if(s_sth.s_amt_ded > 100) {
					fomen("Value Must Not Exceed 100");
					s_sth.s_amt_ded = LV_DOUBLE;
					return(ERROR);
				}
				if(s_sth.s_amt_ded < 0) {
					fomen("Value Must Not Be Lower Than 0");
					s_sth.s_amt_ded = LV_DOUBLE;
					return(ERROR);
				}
			}
			else {
				if(s_sth.s_amt_ded > s_sth.s_amt_loan) {
					fomen("Deduction Amount Must Not Be Greater Than The Amount of The Loan");
					s_sth.s_amt_ded = LV_DOUBLE;
					return(ERROR);
				}
			}
		}
		break;
	case	INT_RATE_FLD:
		if(s_sth.s_int_rate > 100) {
			fomen("Percentage Rate Must Not Exceed 100");
			s_sth.s_int_rate = LV_DOUBLE;
			return(ERROR);
		}
		s_sth.s_int_due = s_sth.s_int_rate * s_sth.s_balance / 100;
		if(s_sth.s_amt_flg[0] == 'F')
			s_sth.s_tot_due = s_sth.s_amt_ded + s_sth.s_int_due;
		else
			s_sth.s_tot_due = 0;

		break;
	case	PP_FLD1:
		if (s_sth.s_pp[0][0] != 'Y' &&
		    s_sth.s_pp[0][0] != 'N') {
			fomen("Must enter Y(es) or N(o)");
			get();
			s_sth.s_pp[0][0] = LV_CHAR;
			return(ERROR);
		}
		if(s_sth.s_pp[0][0] == 'N') {
			for(i=1;i<5;i++)
				s_sth.s_pp[i][0] = 'N' ;
		}
		sr.curfld+=100;
		break;
	case	PP_FLD2:
		if (s_sth.s_pp[1][0] != 'Y' &&
		    s_sth.s_pp[1][0] != 'N') {
			fomen("Must enter Y(es) or N(o)");
			get();
			s_sth.s_pp[1][0] = LV_CHAR;
			return(ERROR);
		}
		if(s_sth.s_pp[1][0] == 'N') {
			for(i=2;i<5;i++)
				s_sth.s_pp[i][0] = 'N' ;
		}
		sr.curfld+=100;
		break;
	case	PP_FLD3:
		if (s_sth.s_pp[2][0] != 'Y' &&
		    s_sth.s_pp[2][0] != 'N') {
			fomen("Must enter Y(es) or N(o)");
			get();
			s_sth.s_pp[2][0] = LV_CHAR;
			return(ERROR);
		}
		if(s_sth.s_pp[2][0] == 'N') {
			for(i=3;i<5;i++)
				s_sth.s_pp[i][0] = 'N' ;
		}
		sr.curfld+=100;
		break;
	case	PP_FLD4:
		if (s_sth.s_pp[3][0] != 'Y' &&
		    s_sth.s_pp[3][0] != 'N') {
			fomen("Must enter Y(es) or N(o)");
			get();
			s_sth.s_pp[3][0] = LV_CHAR;
			return(ERROR);
		}
		if(s_sth.s_pp[3][0] == 'N') {
			for(i=4;i<5;i++)
				s_sth.s_pp[i][0] = 'N' ;
		}
		sr.curfld+=100;
		break;
	case	PP_FLD5:
		if (s_sth.s_pp[4][0] != 'Y' &&
		    s_sth.s_pp[4][0] != 'N') {
			fomen("Must enter Y(es) or N(o)");
			get();
			s_sth.s_pp[4][0] = LV_CHAR;
			return(ERROR);
		}
		sr.curfld+=100;
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

static
WindowHelp()
{
	int	retval ;

	switch(sr.curfld){
	case KEY_START:
		retval = emp_loan_hlp(s_sth.s_emp_numb,s_sth.s_csb_loan,
			 &s_sth.s_seq_no,7,13);

		if(retval == DBH_ERR) return(retval);
		if(retval >= 0) redraw();
		if(retval == 0) return(ERROR);
		if(retval < 0) return(ERROR);
		if(s_sth.s_fn[0] != ADDREC) {
			SetDupBuffers(KEY_MID, KEY_END, 1);
			s_sth.s_csb_loan[0] = LV_CHAR;
			s_sth.s_seq_no = LV_SHORT;
		}
		else {
			s_sth.s_csb_loan[0] = LV_CHAR;
			s_sth.s_seq_no = LV_SHORT;
			SetDupBuffers(KEY_MID, KEY_END, 1);
		}

		break;

	case KEY_MID:
		retval = loan_hlp(s_sth.s_csb_loan,7,13);
		if(retval == DBH_ERR) return(retval);
		if(retval >= 0) redraw();
		if(retval == 0) return(ERROR);
		if(retval < 0) return(ERROR);
		strcpy(csb_loan.cs_code,s_sth.s_csb_loan);
		retval = get_loan(&csb_loan,BROWSE,0,e_mesg);
		if(retval < 0) {
			fomer(e_mesg);
			s_sth.s_csb_loan[0] = LV_CHAR;
			return(ERROR);
		}

		strcpy(emp_rec.em_numb,s_sth.s_emp_numb);

		retval = get_employee(&emp_rec,BROWSE,0,e_mesg);
		if(retval < 0) {
			fomen(e_mesg);
			get();
			return(ERROR);
		}

		break;

	default :
		fomer("No Help Window For This Field");
	}	/* Switch sr.curfld */

	return(NOERROR) ;
}	/* WindowHelp() */
/*-----------------------------------------------------------------------*/
/* Take the confirmation from user for the header part */

static
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
static
ChangeFields()
{
	int	i, retval ;
	int	first_fld, last_fld;

	SetDupBuffers(START_FLD, END_FLD - 200, 1);

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

		scpy((char *)&image,(char *)&s_sth,sizeof(image));

		switch(s_sth.s_field) {
			case 1:
				s_sth.s_amt_loan = LV_DOUBLE;
				first_fld = AMT_LOAN_FLD;
				last_fld = AMT_LOAN_FLD;
				break;

			case 2:
				s_sth.s_balance = LV_DOUBLE;
				first_fld = BALANCE_FLD;
				last_fld = BALANCE_FLD;
				break;

			case 3:
				s_sth.s_numb_pp = LV_SHORT;
				first_fld = NUM_PAY_PER_FLD;
				last_fld = AMT_DED_FLD;
				break;

			case 4:
				s_sth.s_amt_flg[0] = LV_CHAR;
				s_sth.s_amt_ded = LV_DOUBLE;
				first_fld = AMT_FLG_FLD;
				last_fld = AMT_DED_FLD;
				break;

			case 5:
				s_sth.s_int_rate = LV_DOUBLE;
				first_fld = INT_RATE_FLD;
				last_fld = TOTAL_DUE_FLD;
				break;

			case 6:
				for(i=0; i<5 ; i++) {
					s_sth.s_pp[i][0] = LV_CHAR;
				}
				first_fld = PP_FLD1;
				last_fld = PP_FLD5;
				break;

			default:
				fomen("Invalid field number");
				continue;
		}

		retval = ReadFields((char *)&s_sth,first_fld, last_fld,
			Validation, WindowHelp,1) ;

/*		s_sth.s_balance = s_sth.s_amt_loan; */
		s_sth.s_int_due = s_sth.s_int_rate * s_sth.s_balance / 100;
		s_sth.s_tot_due = s_sth.s_amt_ded + s_sth.s_int_due;
		ret( WriteFields((char *)&s_sth,START_FLD, (END_FLD - 200)) );

		if(RET_USER_ESC == retval){
			if(sr.curfld == AMT_FLG_FLD) {
				sr.curfld = NUM_PAY_PER_FLD;
			}
			if(sr.curfld == AMT_DED_FLD) {
				sr.curfld = NUM_PAY_PER_FLD;
			}
			retval = CopyBack((char *)&s_sth,(char *)&image,
				sr.curfld,END_FLD);
			if(retval == PROFOM_ERR) return(retval);
		}
	}
     	s_sth.s_field = HV_SHORT ;
	if ( WriteFields((char *)&s_sth,CHG_FLD, CHG_FLD) < 0 ) return(-1);

     	s_sth.s_mesg[0] = HV_CHAR ;
     	DispMesgFld((char *)&s_sth);

	if(SetDupBuffers(START_FLD, END_FLD-200, 0)<0) return(PROFOM_ERR);

	return(NOERROR);
}
/*-----------------------------------------------------------*/
/* Move Header to Screen Hdr Fields */
static
ShowScreen()
{
	int	i;

	strcpy(s_sth.s_emp_numb, emp_loan.el_numb);
	strcpy(s_sth.s_csb_loan, emp_loan.el_code);
	s_sth.s_seq_no = emp_loan.el_seq;

	strcpy(s_sth.s_emp_name, emp_rec.em_last_name);
	strcat(s_sth.s_emp_name, ", ");
	strcat(s_sth.s_emp_name, emp_rec.em_first_name);
	strcat(s_sth.s_emp_name, " ");
	strcat(s_sth.s_emp_name, emp_rec.em_mid_name);

	strcpy(csb_loan.cs_code,s_sth.s_csb_loan);
	get_loan(&csb_loan,BROWSE,0,e_mesg);
	strcpy(s_sth.s_desc, csb_loan.cs_desc);

	s_sth.s_amt_loan = emp_loan.el_amount;
	s_sth.s_balance = emp_loan.el_amnt_out;
	s_sth.s_numb_pp = emp_loan.el_pp_num;
	strcpy(s_sth.s_amt_flg, emp_loan.el_amt_flg);
	s_sth.s_amt_ded = emp_loan.el_pp_amount;
	s_sth.s_int_rate = emp_loan.el_int;
	s_sth.s_int_due = s_sth.s_balance * s_sth.s_int_rate / 100;
	s_sth.s_tot_due = s_sth.s_amt_ded + s_sth.s_int_due;
	for (i=0 ; i<5 ; i++) {
		strcpy(s_sth.s_pp[i],emp_loan.el_ded_pp[i]);
	}

	ret( WriteFields((char *)&s_sth, KEY_START, END_FLD - 200) ) ;

	return(NOERROR) ;
}	/* ShowScreen() */
/*------------------------------------------------------------------------*/
/* Move High values to all data fields and clear the screen */

static
ClearScreen()
{
	/* Move High Values to Hedaer part */
	InitFields(HV_CHAR, HV_SHORT, HV_DOUBLE) ;

	ret( WriteFields((char *)&s_sth,START_FLD, (END_FLD - 200)) );

	return(NOERROR);
}	/* ClearScreen() */
/*-------------------------------------------------------------------------*/
/* Initialize Screen Header with either Low or High Values */

static
InitFields( t_char, t_short,t_double )
char	t_char ;
short	t_short ;
double	t_double;
{
	int	i;

	s_sth.s_amt_loan = t_double;
	s_sth.s_balance = t_double;
	s_sth.s_numb_pp = t_short;
	s_sth.s_amt_flg[0] = t_char;
	s_sth.s_amt_ded = t_double;
	s_sth.s_int_rate = t_double;
	s_sth.s_int_due = t_double;
	s_sth.s_tot_due = t_double;

	for( i=0; i<5 ; i++) {
		s_sth.s_pp[i][0] = t_char;
	}

	return(NOERROR) ;
}	/* InitFields() */
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
	case SCR_4  :		/* SCR - 4  'EARNINGS' */
	case SCR_5  :		/* SCR - 5  'MISCELLANIOUS' */
	case SCR_6  :		/* SCR - 6  'BENEFIT' */
	case SCR_7  :		/* SCR - 7  'DEDUCTION' */
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