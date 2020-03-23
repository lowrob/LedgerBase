/*-----------------------------------------------------------------------
Source Name: emp_chq.c 
System     : Ledger Base 
Module     : Personnel/Payroll
Created  On: 1991/10/18  
Created  By: Eugene Roy 

DESCRIPTION:
	Program to Change/Inquire the Cheque Location Information.

Usage of SWITCHES when they are ON :

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
#define	MOD_DATE	"11-DEC-91"		/* Program Last Modified */


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

#define	FIXED		'F'
#define	PERCENT		'P'

#define	FEMALE		'F'
#define MALE		'M'
#define MARRIED		'M'
#define SINGLE		'S'
#define DIVORCED	'D'
#define SEPARATED	'R'
#define WIDOWED		'W'

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

#define	FIXED		'F'
#define	PERCENT		'P'

#define	FEMALE		'F'
#define MALE		'M'
#define MARRIED		'M'
#define SINGLE		'S'
#define DIVORCED	'D'
#define SEPARATED	'R'
#define WIDOWED		'W'

#endif

/* PROFOM Releted declarations */

#define PRGNM  		"emp_chq"
#define SCR_NAME	"emp_chq"

#define MAX_FIELD	17		/* Maximum field # to edit */

/* PROFOM Field Numbers */

/* Screen Control Variables */
#define	START_FLD	900 	/* Data entry starting field */
#define	END_FLD  	1800		/* screen end field */
#define	CC_FLD		900 	/* Data entry starting field */
#define	BANK_FLD	1600 	/* Data entry starting field */
#define	CHG_FLD		600 	/* Data entry starting field */

#define	KEY_START	500 	/* Key Start Field 	*/
#define	KEY_END		500 	/* Key Start Field 	*/

#define	FN_FLD  	400 	/* Fn: Field: 		*/

/* Profom 'C' structure for the screen "schl1" i.e., for Demographic data */
typedef struct	{

	char	s_pgm[11];		/* 100 Program Name String 10	*/ 
	long	s_sysdate;		/* 300 System date DATE YYYYFMMFDD */
	char	s_fn[2];		/* 400 Fn: field String 1	*/
	char	s_emp[13];		/* 500 Employee No String 13*/
	short	s_field;		/* 600 Field Number Numeric 99	*/
	char	s_empname[41];		/* 800 Empl Last Name String 25	*/
	char	s_status[7];		/* 850 Status String 3	*/
	short	s_cc;		/* 600 Field Number Numeric 99	*/
	char	s_madd[31];		/* 1100 Mailing Addr. String 30 */
	char	s_sadd[31];		/* 1200 Street Addr. String 30	*/
	char	s_add1[31];		/* 1300 Schl. City String 30	*/
	char	s_add2[31];		/* 1400 Schl. City String 30	*/
	char	s_pc[11];		/* 1500 Postal Code String 12	*/
	char	s_att[31];		/* 1800 Sex String XX		*/
	char	s_bank[7];		/* 1900 Martial Status String XX */
	char	s_bk_name[31];		/* 2000 Phone Number String 10	*/
	char	s_bk_acct[19];		/* 2100 Phone Number String 4	*/
	char	s_mesg[78];		/* 2500 Message Line String 77	*/
	char	s_opt[2];		/* 2600 Message Option String X */ 

	} s_struct;

static	s_struct	s_sth, image ;

static	Bank	bank;
static	Sch_rec	sch_rec;
static	Pay_param	pay_param;
static	int	mode;

static	int	Validate();
static	int	WindowHelp();

Emp_chq()
{
	int	err;

	err = InitScreen(); 	/* Initialization routine */

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
			err = ShowScr(BROWSE);
		}
	}

	err = Process(); 	/* Initiate Process */

	return( err ) ;

} /* DemoGraph() */
/*----------------------------------------------------------------*/
/* Initialize screen before going to process options */
static
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
}	/* InitScr() */
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

static
ReadFunction()
{
	/* Display Fn: options */
#ifdef ENGLISH
	fomer("C(hange), I(nquire), F(orward), B(ackward), N(ext), P(rev), S(creen), E(xit)");
#else
	fomer("C(hanger), I(nterroger), S(uivant), F(in)");
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
/*----------------------------------------------------------------------*/
/* Adding.  Get the unique Key, accept details and update the files */
static
Add()
{
	int	err ;

	for( ;; ) {
		err = ReadKey();
		if(err != NOERROR) return(err) ;
		strcpy(emp_rec.em_numb,s_sth.s_emp);
		err = get_employee(&emp_rec,BROWSE,0,e_mesg);
		if(err = UNDEF) break;
		if(err == NOERROR) {
			fomen("Employee Already Exists");
			continue;
		}
		if(err < 0)  {
			fomen(e_mesg);
			get();
			return(err);
		}
	}

	/* Clear The Screen */
	err = ClearScreen();
	if(err != NOERROR) return(err) ;
	
	strcpy(s_sth.s_status, "ACT");

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
static
Inquire()
{
	int	err ;

	err = SelectRecord() ;
	if(NOERROR != err) return(err) ;

	return(NOERROR) ;
}	/* Inquire() */
/*----------------------------------------------------------*/
/* Show the next or previous Employees                      */

static
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
			return(ERROR);
		}
	  }
	  else{
		break;
	  }
	}
	strcpy( s_sth.s_emp,emp_rec.em_numb);
	retval = ShowScr(BROWSE);

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
			ret(WriteFields((char *)&s_sth,START_FLD,END_FLD-200));
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
static
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

	err = ShowScr(BROWSE);

	return(NOERROR);
}	/* SelectRecord() */
/*----------------------------------------------------------------------*/
/* Get the Student Study Hall key from user. In ADD mode disable dup buffers, */
/* other modes enable dup buffers and show the current key as a default key */
static
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
static
WriteRecords(mode)
int mode;
{
	int	i,retval;

	strcpy(emp_rec.em_numb,s_sth.s_emp);
	
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
		retval = UsrBargVal(mode,emp_rec.em_numb,emp_rec.em_barg,1,e_mesg);
		if(retval < 0){
			DispError((char *)&s_sth,e_mesg);
		  	s_sth.s_emp[0] = LV_CHAR;
			return(-1);
		}
		break;

	case CC_FLD:
		if(s_sth.s_cc == '\0'){
			sr.curfld += 100;
			break;
		}

		sch_rec.sc_numb = s_sth.s_cc;

		retval = get_sch(&sch_rec,BROWSE,0,e_mesg);
		if(retval < 0) {
			fomer("Cost Center Number Does not Exist");
			s_sth.s_cc = LV_SHORT;
			return(ERROR);
		}
		strcpy(s_sth.s_madd, sch_rec.sc_add1);
		strcpy(s_sth.s_sadd, sch_rec.sc_add2);
		strcpy(s_sth.s_add1, sch_rec.sc_add3);
		strcpy(s_sth.s_add2, " ");
		strcpy(s_sth.s_pc, sch_rec.sc_pc);

		ret( WriteFields((char *)&s_sth, START_FLD, END_FLD - 200) ) ;

		break;
	case BANK_FLD:
		strcpy(bank.bk_numb,s_sth.s_bank);
		retval = get_bank(&bank,BROWSE,0,e_mesg);
		if(retval < 0) {
			fomer("Bank Number Does not Exist");
			s_sth.s_bank[0] = LV_CHAR;
			return(ERROR);
		}
		strcpy(s_sth.s_bk_name,bank.bk_name);
		break;
	default:
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

static
WindowHelp()
{
	int	retval ;
	short	reccod ;

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
	case CC_FLD:
		retval = sch_hlp(&s_sth.s_cc,7,12);
		if(retval == DBH_ERR) return(retval);
		redraw();
		sch_rec.sc_numb = s_sth.s_cc;

		retval = get_sch(&sch_rec,BROWSE,0,e_mesg);
		if(retval < 0) {
			fomer("Cost Center Number Does not Exist");
			s_sth.s_cc = LV_SHORT;
			return(ERROR);
		}
		strcpy(s_sth.s_madd, sch_rec.sc_add1);
		strcpy(s_sth.s_sadd, sch_rec.sc_add2);
		strcpy(s_sth.s_add1, sch_rec.sc_add3);
		strcpy(s_sth.s_add2, " ");
		strcpy(s_sth.s_pc, sch_rec.sc_pc);

		ret( WriteFields((char *)&s_sth, START_FLD, END_FLD - 200) ) ;

		break;

	default :
		fomer("No Help Window For This Field");
	}	/* Switch sr.curfld */

	return(NOERROR) ;
}	/* HdrAndKeyWindowHelp() */
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
	int	fld_no, end_fld;

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

		fld_no = START_FLD + (100 * (s_sth.s_field-1));
		end_fld = fld_no;

		retval = ReadFields((char *)&s_sth,fld_no, end_fld,
			Validate, WindowHelp,1) ;
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
ShowScr(mode)
int	mode;
{
	int	i, retval;

	CopyToScreen();

	ret( WriteFields((char *)&s_sth, KEY_START, END_FLD - 200) ) ;

	return(NOERROR) ;
}	/* ShowScr() */
/*------------------------------------------------------------------------*/
/* Move High values to all data fields and clear the screen */

static
ClearScreen()
{
	/* Move High Values to Hedaer part */
	InitFields(HV_CHAR, HV_SHORT, HV_LONG, HV_DOUBLE) ;

	ret( WriteFields((char *)&s_sth,START_FLD, (END_FLD - 200)) );

	return(NOERROR);
}	/* ClearScreen() */
/*-------------------------------------------------------------------------*/
/* Initialize Screen Header with either Low or High Values */

static
InitFields( t_char, t_short,t_long,t_double )
char	t_char ;
short	t_short ;
long	t_long;
double	t_double;
{
	int	i;

	s_sth.s_empname[0] = t_char ;
	if(t_char == HV_CHAR) {
		s_sth.s_status[0] = t_char ;
	}
	if((t_char == HV_CHAR) || (strcmp(pay_param.pr_bk_dep, "Y") != 0)){
		s_sth.s_cc = t_short ;
		s_sth.s_madd[0] = t_char ;
		s_sth.s_sadd[0] = t_char ;
		s_sth.s_add1[0] = t_char ;
		s_sth.s_add2[0] = t_char ;
		s_sth.s_pc[0] = t_char ;
		s_sth.s_att[0] = t_char;
	}
	if((t_char == HV_CHAR) || (strcmp(pay_param.pr_bk_dep, "Y") == 0)){
		s_sth.s_bank[0] = t_char ;
		s_sth.s_bk_name[0] = t_char ;
		s_sth.s_bk_acct[0] = t_char ;
	}

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
	if(strcmp(pay_param.pr_bk_dep, "Y") == 0){
		strcpy(emp_rec.em_bank, s_sth.s_bank);
		strcpy(emp_rec.em_bank_acct, s_sth.s_bk_acct);
	}
	if(strcmp(pay_param.pr_bk_dep, "Y") != 0){
		emp_rec.em_cc = s_sth.s_cc;
		strcpy(emp_rec.em_chq_add1, s_sth.s_madd);
		strcpy(emp_rec.em_chq_add2, s_sth.s_sadd);
		strcpy(emp_rec.em_chq_add3, s_sth.s_add1);
		strcpy(emp_rec.em_chq_add4, s_sth.s_add2);
		strcpy(emp_rec.em_chq_pc, s_sth.s_pc);
		strcpy(emp_rec.em_cont, s_sth.s_att);
	}

	return(NOERROR) ; 
}	/* CopyToScreen() */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Copy the Screen fields to data record */
static int
CopyToScreen()
{
	int	retval;

	strcpy(e_mesg,emp_rec.em_first_name);
	strcat(e_mesg,", ");
	strcat(e_mesg,emp_rec.em_last_name);
	strncpy(s_sth.s_empname,e_mesg,41);

	strcpy(s_sth.s_status, emp_rec.em_status);
	if(strcmp(pay_param.pr_bk_dep, "Y") != 0){
		s_sth.s_cc = emp_rec.em_cc ;

		sch_rec.sc_numb = s_sth.s_cc;

		retval = get_sch(&sch_rec,BROWSE,0,e_mesg);
		if(retval < 0) {
			fomer("Cost Center Number Does not Exist");
			s_sth.s_cc = LV_SHORT;
			return(ERROR);
		}
		strcpy(s_sth.s_madd, sch_rec.sc_add1);
		strcpy(s_sth.s_sadd, sch_rec.sc_add2);
		strcpy(s_sth.s_add1, sch_rec.sc_add3);
		strcpy(s_sth.s_add2, " ");
		strcpy(s_sth.s_pc, sch_rec.sc_pc);

		strcpy(s_sth.s_att, emp_rec.em_cont);
	}

	if(strcmp(pay_param.pr_bk_dep, "Y") == 0){
		strcpy(s_sth.s_bank, emp_rec.em_bank);

/*	Get Bank Name		*/
		strcpy(bank.bk_numb, s_sth.s_bank);

		retval = get_bank(&bank,UPDATE,0,e_mesg);
		if(retval < 0) {
			fomer(e_mesg);
			return(retval);
		}

		strcpy(s_sth.s_bk_name, bank.bk_name);
		strcpy(s_sth.s_bk_acct, emp_rec.em_bank_acct);

	}
	return(NOERROR);

}	/* Copy to Record */


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
	case SCR_8  :		/* SCR - 8  'CSB/LOAN' */
	case SCR_9  :		/* SCR - 9  'GARNISHMENT' */
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

