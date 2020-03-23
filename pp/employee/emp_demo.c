/*----------------------------------------------------------------------
Source Name: emp_demo.c 
System     : Ledger Base 
Module     : Personnel/Payroll
Created  On: 1991/10/18  
Created  By: Cathy Burns 

DESCRIPTION:
	Program to Change/Inquire the Demographic Information.

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
#define	MOD_DATE	"20-AUG-92"		/* Program Last Modified */ 
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
#define SEPARATED	'X'
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
#define SEPARATED	'X'
#define WIDOWED		'W'

#endif

/* PROFOM Releted declarations */

#define PRGNM  		"emp_demo"
#define SCR_NAME	"emp_demo"

#define MAX_FIELD	18		/* Maximum field # to edit */

/* PROFOM Field Numbers */

/* Screen Control Variables */
#define	START_FLD	800 	/* Data entry starting field */
#define	END_FLD  	2700		/* screen end field */
#define	CHG_FLD		600 	/* Data entry starting field */

#define	KEY_START	500 	/* Key Start Field 	*/
#define	KEY_END		500 	/* Key Start Field 	*/

#define	FN_FLD  	400 	/* Fn: Field: 		*/
#define LASTNAME_FLD	800	/* Employee Name field	*/
#define PROV_FLD	1400	/* Province/State Field */	
#define SIN_FLD		1600	/* Sin Field */	
#define SEX_FLD		1800	/* Sex Field */	
#define MARTIAL_FLD	1900	/* Martial Status Field */	
#define CC_FLD		2200	/* Cost Center Number Field */	
#define CC_DESC		2350	/* Cost Center Description Field */
#define REL_FLD		2400	/* Religion fields */

/* Profom 'C' structure for the screen "schl1" i.e., for Demographic data */
typedef struct	{
	char	s_pgm[11];		/* 100 Program Name String 10	*/ 
	long	s_sysdate;		/* 300 System date DATE YYYYFMMFDD */
	char	s_fn[2];		/* 400 Fn: field String 1	*/
	char	s_emp[13];		/* 500 Employee No String 13*/
	short	s_field;		/* 600 Field Number Numeric 99	*/
	char	s_last[26];		/* 800 Empl Last Name String 25	*/
	char	s_status[4];		/* 850 Status String 3	*/
	char	s_first[16];		/* 900 Empl First String 15	*/
	char	s_middle[16];		/* 1000 Empl Middle String 15	*/
	char	s_madd[31];		/* 1100 Mailing Addr. String 30 */
	char	s_sadd[31];		/* 1200 Street Addr. String 30	*/
	char	s_add1[31];		/* 1300 Schl. City String 30	*/
	char	s_add2[31];		/* 1400 Schl. City String 30	*/
	char	s_pc[11];		/* 1500 Postal Code String 12	*/
	char	s_sin[10];		/* 1600 Postal Code String 12	*/
	long	s_birth;		/* 1700 Birth Date ####/##/##	*/
	char	s_sex[2];		/* 1800 Sex String XX		*/
	char	s_martial[2];		/* 1900 Martial Status String XX */
	char	s_phone[11];		/* 2000 Phone Number String 10	*/
	char	s_title[5];		/* 2100 Phone Number String 4	*/
	short	s_cc;			/* 2200 Cost Center# NUMERIC 999*/
	char	s_maiden[16];		/* 2300 Phone Number String 4	*/
	char	s_cc_desc[33];
	char	s_religion[3];		/* 2400 Phone Number String 4	*/
	char	s_comment[52];		/* 2500 Phone Number String 4	*/
	char	s_mesg[78];		/* 2600 Message Line String 77	*/
	char	s_opt[2];		/* 2700 Message Option String X */ 
	} s_struct;

static  Pay_param	param;		/* pay roll parameter file */
static	Sch_rec		sch_rec;
static	Cert	cert;
static	Religion	religion; 

static	s_struct	s_sth, image, image2 ;

static	int	Validate() ;
static	int	WindowHelp() ;

static	char	tmp_numb[13], tmp_stat_flg[2], tmp_prnt_flg[2],
		tmp_last_name[26], tmp_first_name[16], tmp_mid_name[16],
		tmp_add1[31], tmp_add2[31], tmp_add3[31],
		tmp_add4[31], tmp_pc[11], tmp_sin[10];
static	char	tmp_sex[2], tmp_mar_st[2], tmp_maid_name[16], 
		tmp_cpp_exp[2], tmp_uic_exp[2];
static	char	tmp_stat[2], tmp_type[2], tmp_code[6],
		tmp_earn[7], tmp_amt_flg[2];
static	double	tmp_net_tax_cr, tmp_perc, tmp_amt, tmp_target;
static	char	tmp_cert[7], tmp_level[3];
static	long	tmp_st_dt_ft, tmp_st_dt_pt, tmp_st_dt_ca, 
		tmp_st_dt_su, tmp_cont_dt, tmp_app_dt, tmp_date;
static	short	tmp_ann, tmp_cc;
static	char	tmp_lang[2], tmp_ins[2], tmp_pre_paid[2];
static	char	tmp_expense[7], tmp_class[7], tmp_card_cd[3], tmp_card_fld[5],
		tmp_card_value[10];

int	mode;

DemoGraph( )
{
	int	err ;

	err = get_pay_param(&param,BROWSE,1,e_mesg);
	if(err < 0 && err != UNDEF){
		DispError((char *)&s_sth,e_mesg);
		return(err);
	}
	if(err == UNDEF){
#ifdef ENGLISH
		fomen("Payroll Parameters Are Not Setup..");
#else
		fomen("Parametres Payroll ne sont pas etablis..");
#endif
		get() ;
	}

	err = InitScreen() ; 	/* Initialization routine */

	if(emp_rec.em_numb[0] != '\0') {
		flg_reset(EMPLOYEE);

		err = get_n_employee(&emp_rec, BROWSE, 0, FORWARD, e_mesg);
#ifndef ORACLE
/*		seq_over(EMPLOYEE);*/
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

ReadFunction()
{
	/* Display Fn: options */
#ifdef ENGLISH
	fomer("A(dd), C(hange), I(nquire), F(orward), B(ackward), N(ext), S(creen), E(xit)");
#else
	fomer("R(ajouter), C(hanger), I(nterroger), S(uivant), E(cran), F(in)");
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
		mode = ADD;
		return( Add() ) ;
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
		mode = BROWSE;
		return( Next(FORWARD) ) ;
	case PREV_RECORD  :			/* Inquire */
		CHKACC(retval,BROWSE,e_mesg);
		mode = BROWSE;
		return( Next(BACKWARD) ) ;
	case NEXT_SCR  : 	/* Next Screen */
		Cur_Option += 1 ;	/* Set to Next Screen */
		return( JUMP ) ; 
	case SCREEN  : 	/* Next Screen */
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
Add()
{
	int	err ;

	err = ReadKey();
	if(err != NOERROR) return(err) ;

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
Inquire()
{
	int	err ;

	err = SelectRecord() ;
	if(NOERROR != err) return(err) ;

	return(NOERROR) ;
}	/* Inquire() */
/*----------------------------------------------------------*/
/* Show the next or previous Employees                      */

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
	retval = ShowScreen(BROWSE);

	return(NOERROR) ;
}	/* Next() */
/*------------------------------------------------------------*/
/* Allows User to Add Record to File */
/*------------------------------------------------------------*/
GetDetails()
{
	int	i ;
	int	err;

	err = ReadScreen(ADD);
	if(PROFOM_ERR == err || DBH_ERR == err) return(err) ;
	if(RET_USER_ESC == err){
		return(RET_USER_ESC) ;
	}

	emp_rec.em_dir_dep[0] = NO;

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
ReadScreen(mode)
int	mode;
{
	int err;

	scpy((char *)&image2,(char *)&s_sth,sizeof(image2));

	if(mode == ADD) {
		SetDupBuffers(START_FLD,END_FLD,0); /* Off Dup Control */
	}
	else {
		SetDupBuffers(START_FLD,END_FLD,1); /* Off Dup Control */
	}

	if(mode == ADD) {
		if(strcmp(param.pr_prov,"NF") == 0){
#ifdef	ENGLISH
			strcpy(s_sth.s_add2,"NEWFOUNDLAND");
#else
			strcpy(s_sth.s_add2,"TERRE-NEUVE");
#endif
		}
		if(strcmp(param.pr_prov,"NS") == 0){
#ifdef	ENGLISH
			strcpy(s_sth.s_add2,"NOVA SCOTIA");
#else
			strcpy(s_sth.s_add2,"NOUVELLE-ECOSSE");
#endif
		}
		if(strcmp(param.pr_prov,"PE") == 0){
#ifdef	ENGLISH
			strcpy(s_sth.s_add2,"PRINCE EDWARD ISLAND");
#else
			strcpy(s_sth.s_add2,"ILE-DU-PRINCE-EDOUARD");
#endif
		}
		if(strcmp(param.pr_prov,"NB") == 0){
#ifdef	ENGLISH
			strcpy(s_sth.s_add2,"NEW BRUNSWICK");
#else
			strcpy(s_sth.s_add2,"NOUVEAU-BRUNSWICK");
#endif
		}
		if(strcmp(param.pr_prov,"PQ") == 0){
#ifdef	ENGLISH
			strcpy(s_sth.s_add2,"QUEBEC");
#else
			strcpy(s_sth.s_add2,"QUEBEC");
#endif
		}
		if(strcmp(param.pr_prov,"ON") == 0){
#ifdef	ENGLISH
			strcpy(s_sth.s_add2,"ONTARIO");
#else
			strcpy(s_sth.s_add2,"ONTARIo");
#endif
		}
		if(strcmp(param.pr_prov,"MB") == 0){
#ifdef	ENGLISH
			strcpy(s_sth.s_add2,"MANITOBA");
#else
			strcpy(s_sth.s_add2,"MANITOBA");
#endif
		}
		if(strcmp(param.pr_prov,"SK") == 0){
#ifdef	ENGLISH
			strcpy(s_sth.s_add2,"SASKATCHEWAN");
#else
			strcpy(s_sth.s_add2,"SASKATCHEWAN");
#endif
		}
		if(strcmp(param.pr_prov,"AB") == 0){
#ifdef	ENGLISH
			strcpy(s_sth.s_add2,"ALBERTA");
#else
			strcpy(s_sth.s_add2,"ALBERTA");
#endif
		}
		if(strcmp(param.pr_prov,"BC") == 0){
#ifdef	ENGLISH
			strcpy(s_sth.s_add2,"BRITISH COLUMBIA");
#else
			strcpy(s_sth.s_add2,"COLOMBIE-BRITANIQUE");
#endif
		}
		if(strcmp(param.pr_prov,"NT") == 0){
#ifdef	ENGLISH
			strcpy(s_sth.s_add2,"NORTHWEST TERRITORIES");
#else
			strcpy(s_sth.s_add2,"TERRITOIRES DU NORD-OUEST");
#endif
		}
		if(strcmp(param.pr_prov,"YT") == 0){
#ifdef	ENGLISH
			strcpy(s_sth.s_add2,"YUKON TERRITORY");
#else
			strcpy(s_sth.s_add2,"TERRITOIRE DU YUKON");
#endif
		}

		SetDupBuffers(PROV_FLD,PROV_FLD,1); /* Off Dup Control */
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

			err = CopyBack((char *)&s_sth,(char *)&image2,
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
SelectRecord()
{
	int	err ;

	err = ReadKey();
	if(err != NOERROR) return(err) ;
	
	err = ShowScreen(BROWSE);

	return(NOERROR);
}	/* SelectRecord() */
/*----------------------------------------------------------------------*/
/* Get the Student Study Hall key from user. In ADD mode disable dup buffers, */
/* other modes enable dup buffers and show the current key as a default key */
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
WriteRecords(mode)
int mode;
{
	int	i,retval;
	char	acresp[2];

	strcpy(emp_rec.em_numb,s_sth.s_emp);
	if(mode != ADD) {
		retval = get_employee(&emp_rec,UPDATE,0,e_mesg);
		if(retval < 0) {
			fomer(e_mesg);
			return(retval);
		}
	}
	if(mode == ADD) {
		InitEmployee();
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
		ret( WriteFields((char *)&s_sth,KEY_START, KEY_END) ) ;

		if(s_sth.s_emp[0] == '\0'){
			s_sth.s_emp[0] = LV_CHAR;
			return(-1);
		}
		strcpy(emp_rec.em_numb,s_sth.s_emp);
		retval = get_employee(&emp_rec,BROWSE,0,e_mesg);

		if(s_sth.s_fn[0] == ADDREC){
			if(retval == NOERROR){
			  fomen("Employee Already Exist - Please Re-enter");
			  s_sth.s_emp[0] = LV_CHAR;
			  return(-1);
			}
		}
		else{
			if(retval < 0){
				DispError((char *)&s_sth,e_mesg);
			  	s_sth.s_emp[0] = LV_CHAR;
				return(-1);
			}
			retval = UsrBargVal(mode,emp_rec.em_numb,emp_rec.em_barg,1,e_mesg);
			if(retval < 0){
				DispError((char *)&s_sth,e_mesg);
			  	s_sth.s_emp[0] = LV_CHAR;
				return(-1);
			}
		}
		break;

	case LASTNAME_FLD:
		if(s_sth.s_last[0] == '\0') {
			fomen("This is a Required Field");
			return(ERROR);
		}
		break;
	case SIN_FLD:
		value = number = final = 0;
		for(i=0;i<sizeof(s_sth.s_sin)-1;i++) {
			if(!isdigit(s_sth.s_sin[i])) {
				fomen("Social Insurance Number Must Be Numeric");
				s_sth.s_sin[0] = LV_CHAR;
				return(ERROR);
			}
		}
		for(j=0,i=0;i<sizeof(s_sth.s_sin)-2;j++,i++) {
			final += (s_sth.s_sin[i] - '0');
			i=i+1;
			part1[j] = s_sth.s_sin[i];
		}
		part1[4] = '\0';
		part2[4] = '\0';
		value = atoi(part1);
		value = value * 2;
		for(i=0;value != 0;i++) {
			final +=(value % 10);
			value = value / 10;
		}
		number = final / 10;
		if(final % 10 != 0) {
			number++;
		}
		else {	 if(s_sth.s_sin[8] == '0') 
				break;
		     	else {
				fomen("Invalid Social Insurance Number");
				s_sth.s_sin[0] = LV_CHAR;
				return(ERROR);
		     	}
		}

		number = number * 10;
		if((number-final) != (s_sth.s_sin[8]-'0')) {
			fomen("Invalid Social Insurance Number");
			s_sth.s_sin[0] = LV_CHAR;
			return(ERROR);
		} 
		break;
	case SEX_FLD:
		if(s_sth.s_sex[0] != FEMALE && s_sth.s_sex[0] != MALE) {
			fomen("Must be F(emale) or M(ale)");
			s_sth.s_sex[0] = LV_CHAR;
			return(ERROR);
		}
		break;
	case MARTIAL_FLD:
		if(s_sth.s_martial[0] != MARRIED && 
		   s_sth.s_martial[0] != SINGLE && 
		   s_sth.s_martial[0] != DIVORCED &&
		   s_sth.s_martial[0] != SEPARATED &&
		   s_sth.s_martial[0] != WIDOWED) {
			fomen("Must be M(arried), S(ingle), D(ivorced), X(- Separated) or W(idowed)");
			s_sth.s_martial[0] = LV_CHAR;
			return(ERROR);
		}
		break;
	
	case REL_FLD:
		if(s_sth.s_religion[0] == '\0'){
			sr.curfld += 100;
			break;
		}
		Right_Justify_Numeric(s_sth.s_religion,
			sizeof(s_sth.s_religion)-1);
		strcpy(religion.rel_code,s_sth.s_religion);
		retval = get_rel(&religion,BROWSE,0,e_mesg);
		if(retval < 0) {
			fomer("Religion Code Does not Exist");
			s_sth.s_religion[0] = LV_CHAR;
			return(ERROR);
		}

		break;

	case CC_FLD:
		sch_rec.sc_numb = s_sth.s_cc;
		retval = get_sch(&sch_rec,BROWSE,0,e_mesg);
		if(retval < 0) {
			fomer("Cost Center Number Does not Exist");
			s_sth.s_cc = LV_SHORT;
			return(ERROR);
		}
		s_sth.s_cc_desc[0] = LV_CHAR;
		strcpy(s_sth.s_cc_desc,sch_rec.sc_name);
		if ( WriteFields((char *)&s_sth,CC_DESC,CC_DESC) < 0 ) return(-1);

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
				DispError((char *)&s_sth,e_mesg);
			  	s_sth.s_emp[0] = LV_CHAR;
				return(-1);
			}
		}
		break;

	case REL_FLD:
		retval = rel_hlp(s_sth.s_religion,7,12);
		if(retval == DBH_ERR) return(retval) ;
		if(retval >=0 ) redraw();
		if(retval == 0) return(ERROR);
		if(retval < 0)	return(ERROR);
		break;

	case CC_FLD:
		retval = sch_hlp(&s_sth.s_cc,7,12);
		if(retval == DBH_ERR) return(retval) ;
		if(retval >=0 ) redraw();
		if(retval == 0) return(ERROR);
		if(retval < 0)	return(ERROR);
		sch_rec.sc_numb = s_sth.s_cc;
		retval = get_sch(&sch_rec,BROWSE,0,e_mesg);
		if(retval < 0) {
			fomer("Cost Center Number Does not Exist");
			s_sth.s_cc = LV_SHORT;
			return(ERROR);
		}
		s_sth.s_cc_desc[0] = LV_CHAR;
		strcpy(s_sth.s_cc_desc, sch_rec.sc_name);
		if(WriteFields((char *)&s_sth,CC_FLD,CC_FLD+150) < 0 ) return(-1);
		break;
	default :
		fomer("No Help Window For This Field");
	}	/* Switch sr.curfld */

	return(NOERROR) ;
}	/* HdrAndKeyWindowHelp() */
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
ChangeFields()
{
	int	i, retval ;
	int	fld_no, end_fld;

	scpy((char *)&image2,(char *)&s_sth,sizeof(image2));

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
ShowScreen(mode)
int	mode;
{
	int	i, retval;

	retval = ClearScreen() ;

	CopyToScreen();

	ret( WriteFields((char *)&s_sth, KEY_START, END_FLD - 200) ) ;

	scpy((char *)&image,(char *)&s_sth,sizeof(image));

	return(NOERROR) ;
}	/* ShowScreen() */
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
	int	i;

	s_sth.s_last[0] = t_char ;
	if(t_char == HV_CHAR) {
		s_sth.s_status[0] = t_char ;
	}
	s_sth.s_first[0] = t_char ;
	s_sth.s_middle[0] = t_char ;
	s_sth.s_madd[0] = t_char ;
	s_sth.s_sadd[0] = t_char ;
	s_sth.s_add1[0] = t_char ;
	s_sth.s_add2[0] = t_char ;
	s_sth.s_pc[0] = t_char ;
	s_sth.s_sin[0] = t_char ;
	s_sth.s_birth = t_long ;
	s_sth.s_sex[0] = t_char ;
	s_sth.s_martial[0] = t_char ;
	s_sth.s_phone[0] = t_char ;
	s_sth.s_title[0] = t_char ;
	s_sth.s_cc = t_short ;
	s_sth.s_maiden[0] = t_char ;
	if(t_char == HV_CHAR)
		s_sth.s_cc_desc[0] = t_char;

	s_sth.s_religion[0] = t_char ;
	s_sth.s_comment[0] = t_char ;

	return(NOERROR) ;
}	/* InitFields() */
/*----------------------------------------------------------------------*/
InitEmployee()
{
	int i;

	emp_rec.em_pre_paid[0] = '\0';
	emp_rec.em_mth_sic = 0;
	emp_rec.em_mth_vac = 0;
	emp_rec.em_bal_sic = 0;
	emp_rec.em_bal_vac = 0;
	emp_rec.em_yrs_exp = 0;
	emp_rec.em_class[0] = '\0';
	emp_rec.em_pp_code[0] = '\0';
	emp_rec.em_def_pf[0] = '\0';
	emp_rec.em_def_inc = 0.000;
	emp_rec.em_cpp_exp[0] = '\0';
	emp_rec.em_uic_exp[0] = '\0';
	emp_rec.em_tax_exp[0] = '\0';
	emp_rec.em_reg_pen[0] = '\0';
	emp_rec.em_inc_tax = 0.00;
	emp_rec.em_other_fed = 0.00;
	emp_rec.em_union_dues = 0.00;
	emp_rec.em_ho_ded = 0.00;
	emp_rec.em_net_tax_cr = 0.00;
	emp_rec.em_ann_ded = 0.00;
	emp_rec.em_fam_all = 0.00;
	emp_rec.em_old_age = 0.00;
	emp_rec.em_last_pp = 0;
	emp_rec.em_ben_cat[0] = '\0';
	emp_rec.em_ded_cat[0] = '\0';
	emp_rec.em_barg[0] = '\0';
	emp_rec.em_pos[0] = '\0';
	emp_rec.em_perc = 0.00;
	emp_rec.em_cert[0] = '\0';
	emp_rec.em_level = 0;
	emp_rec.em_st_dt_ft = 0;
	emp_rec.em_st_dt_pt = 0;
	emp_rec.em_st_dt_ca = 0;
	emp_rec.em_st_dt_su = 0;
	emp_rec.em_cont_dt = 0;
	emp_rec.em_app_dt = 0;
	emp_rec.em_ann = 0;
	emp_rec.em_lang[0] = '\0';
	emp_rec.em_last_roe = 0;
	emp_rec.em_num_ins_wk = 0;
	emp_rec.em_un_tel[0] = '\0';
	emp_rec.em_ins[0] = '\0';
	emp_rec.em_term_dt = 0;
	emp_rec.em_term[0] = '\0';
	emp_rec.em_vac_rate = 0.00;
	emp_rec.em_uic_rate = 0.00;
	emp_rec.em_dir_dep[0] = '\0';
	emp_rec.em_bank[0] = '\0';
	emp_rec.em_bank_acct = 0;
	emp_rec.em_chq_add1[0] = '\0';
	emp_rec.em_chq_add2[0] = '\0';
	emp_rec.em_chq_add3[0] = '\0';
	emp_rec.em_chq_add4[0] = '\0';
	emp_rec.em_chq_pc[0] = '\0';
	emp_rec.em_cont[0] = '\0';
	emp_rec.em_pre_lev[0] = '\0';
	emp_rec.em_sic_ent = 0.00;
	emp_rec.em_sic_bk = 0.00;
	emp_rec.em_vac_bk = 0.00;
	emp_rec.em_vac_ent = 0.00;

	emp_rec.em_yrs_out_dist = 0;
	emp_rec.em_yrs_out_prov = 0;
	emp_rec.em_cas_hrs = 0;
	emp_rec.em_cas_days = 0;
	emp_rec.em_perm_days = 0;
	emp_rec.em_cas_tot_days = 0;
	emp_rec.em_per_tot_yrs = 0;
	emp_rec.em_per_tot_days = 0;
	for(i=0;i<12;i++){
		emp_rec.em_comm[i][0] = '\0';
		emp_rec.em_sck_acc[i] = 0;
		emp_rec.em_vac_acc[i] = 0;
	}
	for(i=0;i<5;i++){
		emp_rec.em_inst[i][0] = '\0';
		emp_rec.em_prog[i][0] = '\0';
	}
	emp_rec.em_pref_cc = 0;
	emp_rec.em_sen_perc = 0;
	emp_rec.em_reg_prior = 0;
	emp_rec.em_reg_opt = 0;
	emp_rec.em_reg_nonm = 0;
	emp_rec.em_days_out_dist = 0;
	emp_rec.em_days_out_prov = 0;
	emp_rec.em_cas_tot_yrs = 0;
	emp_rec.em_exp = 0;
	emp_rec.em_yrs_out = 0;
	emp_rec.em_days_exp = 0;
	emp_rec.em_ini_casf = 0;
	emp_rec.em_ini_casu = 0;
	emp_rec.em_per_yrs = 0;
	emp_rec.em_per_days = 0;
	emp_rec.em_no_depends = 0;
	emp_rec.em_cntrct_stat[0] = '\0';
	emp_rec.em_emerg_cntct[0] = '\0';
	emp_rec.em_emerg_tel[0] = '\0';
	
	return(NOERROR);
}

/*-----------------------------------------------------------------------*/ 
/*                                                                       */
/* Copy the data record fields to Screen                                 */
/*                                                                       */
/*-----------------------------------------------------------------------*/
static int
CopyToRecord()
{
	int	i;

	strcpy(emp_rec.em_last_name, s_sth.s_last);
	strcpy(emp_rec.em_status, s_sth.s_status);
	strcpy(emp_rec.em_first_name, s_sth.s_first);
	strcpy(emp_rec.em_mid_name, s_sth.s_middle);
	strcpy(emp_rec.em_add1, s_sth.s_madd);
	strcpy(emp_rec.em_add2, s_sth.s_sadd);
	strcpy(emp_rec.em_add3, s_sth.s_add1);
	strcpy(emp_rec.em_add4, s_sth.s_add2);
	strcpy(emp_rec.em_pc, s_sth.s_pc);
	strcpy(emp_rec.em_sin, s_sth.s_sin);
	emp_rec.em_date = s_sth.s_birth;
	strcpy(emp_rec.em_sex, s_sth.s_sex);
	strcpy(emp_rec.em_mar_st, s_sth.s_martial);
	strcpy(emp_rec.em_phone, s_sth.s_phone);
	strcpy(emp_rec.em_title, s_sth.s_title);
	strcpy(emp_rec.em_maid_name, s_sth.s_maiden);
	strcpy(emp_rec.em_religion, s_sth.s_religion);
	strcpy(emp_rec.em_com, s_sth.s_comment);
	emp_rec.em_cc = s_sth.s_cc;

	return(NOERROR) ; 
}	/* CopyToScreen() */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Copy the Screen fields to data record */
static int
CopyToScreen()
{
	int	retval;

	strcpy(s_sth.s_last, emp_rec.em_last_name);
	strcpy(s_sth.s_status, emp_rec.em_status);
	strcpy(s_sth.s_first, emp_rec.em_first_name);
	strcpy(s_sth.s_middle, emp_rec.em_mid_name);
	strcpy(s_sth.s_madd, emp_rec.em_add1);
	strcpy(s_sth.s_sadd, emp_rec.em_add2);
	strcpy(s_sth.s_add1, emp_rec.em_add3);
	strcpy(s_sth.s_add2, emp_rec.em_add4);
	strcpy(s_sth.s_pc, emp_rec.em_pc);
	strcpy(s_sth.s_sin, emp_rec.em_sin);
	s_sth.s_birth = emp_rec.em_date ;
	strcpy(s_sth.s_sex, emp_rec.em_sex);
	s_sth.s_cc = emp_rec.em_cc;

	sch_rec.sc_numb = s_sth.s_cc;
	retval = get_sch(&sch_rec,BROWSE,0,e_mesg);
	if(retval < 0) {
		fomer("Cost Center Number Does not Exist");
		get();
	}
	strcpy(s_sth.s_cc_desc, sch_rec.sc_name);

	strcpy(s_sth.s_martial, emp_rec.em_mar_st);
	strcpy(s_sth.s_phone, emp_rec.em_phone);
	strcpy(s_sth.s_title, emp_rec.em_title);
	strcpy(s_sth.s_maiden, emp_rec.em_maid_name);
	strcpy(s_sth.s_religion, emp_rec.em_religion);
	strcpy(s_sth.s_comment, emp_rec.em_com);


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
	case SCR_2  :		/* SCR - 2  'EMPLOYMENT' */
	case SCR_3  :		/* SCR - 3  'RESPONSIBILITY' */
	case SCR_4  :		/* SCR - 4  'EARNINGS' */
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

