/*-----------------------------------------------------------------------
Source Name: emp_att.c 
System     : Ledger Base 
Module     : Personnel/Payroll
Created  On: 1991/12/17  
Created  By: Eugene Roy 

DESCRIPTION:
	Program to Maintain the Employee's Attendance.

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
L.Robichaud	97/04/24	Added code to look for a S11 (half day) and 
			S12 (full day) attendance codes.
L.Robichaud	97/05/20	Added code to look for a V11 (half day) and 
			V12 (full day) attendance codes.
------------------------------------------------------------------------*/

#define  MAINFL		EMP_ATT  		/* main file used */

#include <stdio.h>
#include <ctype.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_pp.h>
#include <bfs_com.h>
#include <empldrvr.h>

#define	SYSTEM		"PESONNEL/PAYROLL"	/* Sub System Name */
#define	MOD_DATE	"17-DEC-91"		/* Program Last Modified */

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
#define	WEEKEND		'W'

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
#define	WEEKEND		'W'

#endif

/* PROFOM Related declarations */

#define PRGNM  		"emp_att"
#define SCR_NAME	"emp_att"

#define MAX_FIELD	4		/* Maximum field # to edit */

/* number of days in months */

short	d_month[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

char	month[12][4] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG",
			"SEP", "OCT", "NOV", "DEC" };

/* Screen Control Variables */
#define	START_FLD	800 	/* Data entry starting field */
#define	END_FLD  	11400		/* screen end field */
#define	CHG_FLD		700 	/* Data entry starting field */

#define	KEY_START	500 	/* Key Start Field 	*/
#define SICK_BK		900
#define VAC_BK		1300
#define	ITEM_FLD	1500

#define	DATE_FLD	1100 	/* Key Start Field 	*/
#define	CODE_FLD	1200 	/* Key Start Field 	*/
#define	MTH_ST		1700 	/* Key Start Field 	*/
#define	MTH_END		11200 	/* Key Start Field 	*/

#define	FN_FLD  	400 	/* Fn: Field: 		*/

typedef	struct	{	/* Start Fld 1500, Endfld 4200 */

	char	s_mth[4];		/* 1300 Empl Middle String 15	*/
	char	s_att[32];		/* 1300 Empl Middle String 15	*/
	double	s_sck_acc;		/* 1650 YTD Income 9,999,999.99 */
	double	s_vac_acc;		/* 1650 YTD Income 9,999,999.99 */
	double	s_mth_sck;		/* 1700 Increase Tax Amount */
	double	s_mth_vac;		/* 1750 YTD Deferred Amount */
	double	s_bal_sck;		/* 1800 Other Federal Tax Amount */
	double	s_bal_vac;		/* 1850 YTD CPP Amount */

}	S_item ;

/* Profom 'C' structure for the screen "schl1" i.e., for Demographic data */

typedef struct	{

	char	s_pgm[11];		/* 100 Program Name String 10	*/ 
	long	s_sysdate;		/* 300 System date DATE YYYYFMMFDD */
	char	s_fn[2];		/* 400 Fn: field String 1	*/
	char	s_emp[13];		/* 500 Employee No String 13 */
	char	s_name[28];		/* 600 Employee No String 30 */
	short	s_field;		/* 700 Field Number Numeric 99	*/
	
	double	s_sck_bk;		/* Sick bank */
	double	s_sd_ent;		/* Sick balance */
	long	s_date;			/* 300 System date DATE YYYYFMMFDD */
	char	s_code[4];		/* attendance code */
	double	s_vac_bk;
	double	s_vac_ent;		
	char	s_dummy1[2];
	char	s_dummy2[2];

	S_item	s_items[12] ;	/* Start Fld 1500, End Fld 4200  */

	char	s_mesg[78];		/* 2500 Message Line String 77	*/
	char	s_opt[2];		/* 2600 Message Option String X */ 

	} s_struct;

static	s_struct	s_sth, image ;

static	Emp_at_his	att_his;
static	Att	att;
static	Pay_param	pay_param;
static	Barg_unit	barg_unit;
static	Position	position;

static	int	Validate() ;
static	int	WindowHelp() ;
extern	double	D_Roundoff();

static	int	mode;
static	int	first_time;
static	double	non_sen_days;
static	double	real_bal_sick[12];
static	long	start_mth;
static	int	check_dates;

Emp_att( )
{
	int	err ;

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
			err = ShowScreen();
			err = ShowAtt();
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

 	err = get_pay_param(&pay_param,BROWSE,1,e_mesg);
	if(err < 0) {
  		DispError((char *)&s_sth,e_mesg);
		return(err);
	}

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

		err = WriteRecords() ;
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
			return(ERROR);
		}
	  }
	  else{
		break;
	  }
	}

	strcpy( s_sth.s_emp,emp_rec.em_numb);
	first_time = 1;
	retval = ShowScreen();
	retval = ShowAtt();

	return( retval ) ;
}	/* Next() */
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
	    case  CHANGE :		/* Add */
#ifdef ENGLISH
		err = GetOption((char *)&s_sth,
		"Y(es), S(creen Edit), L(ine edit), C(ancel)"
		,"YSLC");
#else
		err = GetOption((char *)&s_sth,
		"O(ui), S(creen edit), L(ine edit), A(nnul)"
		,"OSLA");
#endif
		break ;
	    }
	    if(err == PROFOM_ERR) return(err) ;

	    switch(err) {
	    case  YES  :
		return(YES);
	    case  ADDREC :
		err = Add();
		break;
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
	int	retval ;
	int	fld_no, end_fld;

	SetDupBuffers(START_FLD+100,START_FLD+200,1); /* Off Dup Control */
	SetDupBuffers(START_FLD+500,START_FLD+600,1); /* Off Dup Control */
	SetDupBuffers(ITEM_FLD+200,ITEM_FLD+600,1); /* Off Dup Control */

	/* Get The Field to Be Modified */
#ifdef	ENGLISH
	strcpy(s_sth.s_mesg,"Enter RETURN to Terminate Edit");
#else
	strcpy(s_sth.s_mesg,"Appuyer sur RETURN pour terminer l'ajustement");
#endif
	DispMesgFld((char *)&s_sth); 
        
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
		if(s_sth.s_field > 2)
			fld_no = (START_FLD+300) + (100 * (s_sth.s_field-1));
		else
			fld_no = (START_FLD+100) + (100 * (s_sth.s_field-1));
		end_fld = fld_no;

		retval = ReadFields((char *)&s_sth,fld_no, end_fld,
			Validate, WindowHelp,1) ;
	}
     	s_sth.s_field = HV_SHORT ;
	if ( WriteFields((char *)&s_sth,CHG_FLD, CHG_FLD) < 0 ) return(-1);

     	s_sth.s_mesg[0] = HV_CHAR ;
     	DispMesgFld((char *)&s_sth);

	if(SetDupBuffers(START_FLD, END_FLD-200, 0)<0) return(PROFOM_ERR);
	ShowAtt();

	return(NOERROR);
}
/*------------------------------------------------------------------------*/
/* Move High values to all data fields and clear the screen */

static
ClearScreen()
{
	/* Move High Values to Hedaer part */
	InitFields(HV_CHAR, HV_LONG, HV_DOUBLE) ;

	ret( WriteFields((char *)&s_sth,START_FLD, (END_FLD - 200)) );

	return(NOERROR);
}	/* ClearScreen() */
/*-------------------------------------------------------------------------*/
/* Initialize Screen Header with either Low or High Values */

static
InitFields( t_char,t_long,t_double )
char	t_char ;
long	t_long;
double	t_double;
{
	int	i;

	s_sth.s_name[0] = t_char;
	s_sth.s_sck_bk = t_double;
	s_sth.s_date = t_long;
	s_sth.s_code[0] = t_char;
	s_sth.s_sd_ent = t_double;
	s_sth.s_vac_ent = t_double;
	s_sth.s_vac_bk = t_double;
	if(t_char == HV_CHAR){
		s_sth.s_dummy1[0] = HV_CHAR;
		s_sth.s_dummy2[0] = HV_CHAR;
	}
	else{
		s_sth.s_dummy1[0] = t_char;
		s_sth.s_dummy2[0] = t_char;
	}

	for( i = 0; i < 12; i++){
		s_sth.s_items[i].s_mth[0] = t_char;	
		s_sth.s_items[i].s_att[0] = t_char;
		s_sth.s_items[i].s_sck_acc = t_double;
		s_sth.s_items[i].s_vac_acc = t_double;
		s_sth.s_items[i].s_mth_sck = t_double;	
		s_sth.s_items[i].s_mth_vac = t_double;
		s_sth.s_items[i].s_bal_sck = t_double;
		s_sth.s_items[i].s_bal_vac = t_double;
	}

	return(NOERROR) ;
}	/* InitFields() */

/*-----------------------------------------------------------------------*/ 
/* Check to see if record is to be added, changed or deleted .		 */
/*-----------------------------------------------------------------------*/ 
static int
WriteRecords()
{
	int	retval,i;

	strcpy(emp_rec.em_numb,s_sth.s_emp);
	
	strcpy(emp_rec.em_numb,s_sth.s_emp);
	retval = get_employee(&emp_rec,UPDATE,0,e_mesg);
	if(retval < 0 ) {
		DispError((char *)&s_sth,e_mesg);
		return(retval);
	}
	emp_rec.em_sic_ent = s_sth.s_sd_ent;
	emp_rec.em_vac_ent = s_sth.s_vac_ent;
	emp_rec.em_vac_bk = s_sth.s_vac_bk;
/*	emp_rec.em_sic_bk = s_sth.s_sck_bk; */

	/* update all sick accruals and vacation accruals */
	for(i=0;i<12;i++){
		emp_rec.em_sck_acc[i] = s_sth.s_items[i].s_sck_acc;
		emp_rec.em_vac_acc[i] = s_sth.s_items[i].s_vac_acc;
	}

	retval = put_employee(&emp_rec,UPDATE,e_mesg);
	if(retval < 0) {
		DispError((char *)&s_sth,e_mesg);
		roll_back(e_mesg);
		return(retval);
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
			if(err == EFL) continue;
			return(err);
		}
	
		break;
	}

	first_time = 1;
	err = ShowScreen();
	err = ShowAtt();

	return(NOERROR);
}	/* SelectRecord() */
/*--------------------------------------------------------------------
 ShowAtt()     - this routine reads the attendance information and   
   		 copies it onto the screen
--------------------------------------------------------------------*/
int
ShowAtt()
{
	int 	retval, i,j, curr_month, prev_mon;
	int	nbr_of_mths = 12, records_found, curr_day;
	int	month_nbr;
	double	total, fraction;
	long	dollars;

	curr_month = 1;

	strcpy(barg_unit.b_code,emp_rec.em_barg);
	barg_unit.b_date = get_date();
	flg_reset(BARG);

	retval = get_n_barg(&barg_unit,BROWSE,0,BACKWARD,e_mesg);
	if(retval < 0 || strcmp(barg_unit.b_code,emp_rec.em_barg)!=0){
		DispError((char *)&s_sth,"Error Reading Bargaining Unit File");
		return(-1);
	}
	seq_over(BARG);

	/* initialize screen */
	Disp_Mth();

	for (i=0; i < 12; i++) {
		if((start_mth + i )> 12)
			j = i - start_mth+1 ;
		else
			j = i + start_mth-1;
		strcpy(s_sth.s_items[i].s_mth, month[j]);
		/* Check for leap year	*/
		if(((get_date() / 10000) % 4 == 0) && j == 1)
			strncpy(s_sth.s_items[i].s_att, 
			"...............................",d_month[j]+1);
		else
			strncpy(s_sth.s_items[i].s_att, 
			"...............................",d_month[j]);
		s_sth.s_items[i].s_mth_sck = 0;	
		s_sth.s_items[i].s_mth_vac = 0;
	}
	Get_day();

	strcpy(att_his.eah_numb, emp_rec.em_numb);
	if(pay_param.pr_st_mth == 1){
		att_his.eah_date = pay_param.pr_cal_st_dt;  
	}
	if(pay_param.pr_st_mth == 2){
		att_his.eah_date = pay_param.pr_fisc_st_dt;  
	}
	if(pay_param.pr_st_mth == 3){
		att_his.eah_date = pay_param.pr_schl_st_dt;  
	}

	flg_reset(EMP_ATT);

	for(;;){
		retval = get_n_emp_at(&att_his, BROWSE, 0, FORWARD, e_mesg);

		if(retval == EFL ||		
		   (strcmp(att_his.eah_numb, emp_rec.em_numb)) != 0){
			break;
		}
		if(retval < 0) {
			fomen(e_mesg);
			get();
			return(-1);
		}
		strcpy(att.at_code, att_his.eah_code);

		retval = get_att(&att,BROWSE,1,e_mesg);
		if(retval < 0)  {
			fomen(e_mesg);
			get();
			return(retval);
		}

		curr_day = (att_his.eah_date % 100);
		curr_month = ((att_his.eah_date / 100) % 100);
		if(curr_month < start_mth)
			curr_month += nbr_of_mths;

		/* L.R. add code to look for S11 and S12 attend codes */
		if(strcmp(att.at_sick, "Y") == 0){
			if(strcmp(att.at_code, "S11") == 0)
			   s_sth.s_items[curr_month-start_mth].s_mth_sck += .5;
			else if(strcmp(att.at_code, "S12") == 0)
			   s_sth.s_items[curr_month-start_mth].s_mth_sck += 1;
			else
			   s_sth.s_items[curr_month - start_mth].s_mth_sck += 
		      		att_his.eah_hours / att_his.eah_sen_hours;

		}
		if(strcmp(att.at_vac, "Y") == 0){
			if(strcmp(att.at_code, "V11") == 0)
			   s_sth.s_items[curr_month-start_mth].s_mth_vac += .5;
			else if(strcmp(att.at_code, "V12") == 0)
			   s_sth.s_items[curr_month-start_mth].s_mth_vac += 1;
			else
		           s_sth.s_items[curr_month - (start_mth)].s_mth_vac += 
		   		att_his.eah_hours / att_his.eah_sen_hours;

		} 
		s_sth.s_items[curr_month - start_mth].s_att[curr_day-1] =
			att.at_disp_code[0];
		if(strcmp(att.at_sckbank,"Y")==0){
			s_sth.s_sck_bk -= att_his.eah_hours / 
						att_his.eah_sen_hours;
		}
/******Louis this is to be always calculated & stored in the employee file
		if(strcmp(att.at_vacbank,"Y")==0){
			s_sth.s_vac_bk -= att_his.eah_hours / 
						att_his.eah_sen_hours;
		}
**************/
	}
	seq_over(EMP_ATT);

	month_nbr = 1;

	for(prev_mon=start_mth;month_nbr<=nbr_of_mths;prev_mon++) {

		if(prev_mon < start_mth)
			prev_mon += nbr_of_mths;
		
		if(prev_mon == start_mth){
			real_bal_sick[prev_mon-start_mth] = 
				s_sth.s_sd_ent - 
				s_sth.s_items[prev_mon-start_mth].s_mth_sck +
			        s_sth.s_items[prev_mon-start_mth].s_sck_acc;
			if(real_bal_sick[prev_mon-start_mth] >
			   barg_unit.b_sick_max)
				s_sth.s_items[prev_mon-start_mth].s_bal_sck = 
					barg_unit.b_sick_max;
			else
				s_sth.s_items[prev_mon-start_mth].s_bal_sck= 
					real_bal_sick[prev_mon-start_mth];
			s_sth.s_items[prev_mon-start_mth].s_bal_vac = 
				s_sth.s_vac_ent -
				s_sth.s_items[prev_mon-start_mth].s_mth_vac +
				s_sth.s_items[prev_mon-start_mth].s_vac_acc; 
		}	
		else {
			real_bal_sick[prev_mon-start_mth] =
				real_bal_sick[prev_mon-start_mth-1] - 
				s_sth.s_items[prev_mon-start_mth].s_mth_sck +
		  		s_sth.s_items[prev_mon-start_mth].s_sck_acc;
			if(real_bal_sick[prev_mon-start_mth] >
			   barg_unit.b_sick_max)
				s_sth.s_items[prev_mon-start_mth].s_bal_sck = 
					barg_unit.b_sick_max;
			else
				s_sth.s_items[prev_mon-start_mth].s_bal_sck = 
					real_bal_sick[prev_mon-start_mth];
			s_sth.s_items[prev_mon-start_mth].s_bal_vac = 
				s_sth.s_items[prev_mon-start_mth-1].s_bal_vac - 
				s_sth.s_items[prev_mon-start_mth].s_mth_vac +
				s_sth.s_items[prev_mon-start_mth].s_vac_acc; 
		}
		month_nbr++;
	}

	if((WriteFields((char*)&s_sth, SICK_BK, SICK_BK))<0)
		return(-1);
	if((WriteFields((char*)&s_sth, VAC_BK, VAC_BK))<0)
		return(-1);
	if((WriteFields((char*)&s_sth, MTH_ST, END_FLD-200))<0)
		return(-1);

	return(NOERROR);

}
/*-----------------------------------------------------------*/
Disp_Mth()
{
	int	retval;

 	retval = get_pay_param(&pay_param,BROWSE,1,e_mesg);
	if(retval < 0) {
  		DispError((char *)&s_sth,e_mesg);
		return(retval);
	}

	if(pay_param.pr_st_mth == 1){
		start_mth = ((pay_param.pr_cal_st_dt / 100) % 100);
	}
	if(pay_param.pr_st_mth == 2){
		start_mth = ((pay_param.pr_fisc_st_dt / 100) % 100);
	}
	if(pay_param.pr_st_mth == 3){
		start_mth = ((pay_param.pr_schl_st_dt / 100) % 100);
	}
	return(NOERROR);
}
/*-----------------------------------------------------------*/
/* Move Header to Screen Hdr Fields */
static int
ShowScreen()
{
	int	i;

	strcpy(s_sth.s_name, emp_rec.em_last_name);
	strcat(s_sth.s_name, ", ");
	strcat(s_sth.s_name, emp_rec.em_first_name);
	strcat(s_sth.s_name, " ");
	strcat(s_sth.s_name, emp_rec.em_mid_name);
	s_sth.s_name[27] = '\0';

	s_sth.s_sck_bk = emp_rec.em_sic_bk;
	s_sth.s_sd_ent = emp_rec.em_sic_ent;
	s_sth.s_vac_ent = emp_rec.em_vac_ent;
	s_sth.s_vac_bk = emp_rec.em_vac_bk;

	for(i=0;i<12;i++){
		s_sth.s_items[i].s_sck_acc = emp_rec.em_sck_acc[i];
		s_sth.s_items[i].s_vac_acc = emp_rec.em_vac_acc[i];
	}
	s_sth.s_dummy1[0] = ' ';
	s_sth.s_dummy2[0] = ' ';
	ret( WriteFields((char *)&s_sth, KEY_START, MTH_ST) ) ;

	return(NOERROR) ;
}	/* ShowScreen() */
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
	SetDupBuffers(KEY_START,KEY_START,1);

#ifdef ENGLISH
	strcpy(s_sth.s_mesg,"Press ESC-F to Go Back to Fn:");
#else
	strcpy(s_sth.s_mesg,"Appuyer sur ESC-F pour retourner a Fn:");
#endif
	DispMesgFld((char *)&s_sth);

	strcpy(hold_emp,s_sth.s_emp);

	s_sth.s_emp[0] = LV_CHAR;

	i = ReadFields((char *)&s_sth,KEY_START, KEY_START,
		Validate, WindowHelp,1) ;
	if(PROFOM_ERR == i || DBH_ERR == i) return(i) ;
	if(RET_USER_ESC == i){
		strcpy(s_sth.s_emp,hold_emp);
		
		ret( WriteFields((char *)&s_sth,KEY_START, KEY_START) ) ;

		s_sth.s_mesg[0] = HV_CHAR;
		DispMesgFld((char *)&s_sth);

		return(RET_USER_ESC) ;
	}

	s_sth.s_mesg[0] = HV_CHAR;
	DispMesgFld((char *)&s_sth);

	return(NOERROR);
}	/*  ReadKey() */
/*----------------------------------------------------------------*/
/* Show Help Windows, if applicable, when user gives ESC-H in key
   and header fields */

static int
WindowHelp()
{
	int	retval ;
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
				fomer("Employee Already Exists");
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
		/* Saint John Codes */	
		if(strcmp(emp_rec.em_barg,"    03")==0)
			check_dates = 0; 

		break;
	case CODE_FLD:
		retval = att_hlp(s_sth.s_code,7,13);
		if(retval == DBH_ERR) return(retval);
		if(retval >= 0) redraw();
		if(retval == 0) return(ERROR);
		if(retval < 0) return(ERROR);
		strcpy(att.at_code,s_sth.s_code);

		retval = get_att(&att,BROWSE,1,e_mesg);
		if(retval < 0) {
			fomer(e_mesg);
			s_sth.s_code[0] = LV_CHAR;
			return(ERROR);
		}
		break;
	default :
		fomer("No Help Window For This Field");
	}	/* Switch sr.curfld */

	return(NOERROR) ;
}	/* HdrAndKeyWindowHelp() */
/*----------------------------------------------------------------*/
/* Validation function() for Key and Header fields when PROFOM returns
  RET_VAL_CHK */

static int
Validate()
{
	int	retval;
	long	julian;
	int	remain;

	if(s_sth.s_fn[0] == CHANGE)	mode = UPDATE;
	if(s_sth.s_fn[0] == INQUIRE)	mode = BROWSE;
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
		strcpy(s_sth.s_name, emp_rec.em_last_name);
		strcat(s_sth.s_name, ", ");
		strcat(s_sth.s_name, emp_rec.em_first_name);
		strcat(s_sth.s_name, " ");
		strcat(s_sth.s_name, emp_rec.em_mid_name);
		check_dates = 1;
		/* Saint John Codes */	
		if(strcmp(emp_rec.em_barg,"    03")==0)
			check_dates = 0; 

		break;
	case DATE_FLD:
		julian = days(s_sth.s_date);
		remain = julian % 7;

		if((remain == 6 || remain == 0) && check_dates == 1){
			fomer("Date is a Weekend - Please Re-enter");
			s_sth.s_date = LV_LONG;
			return(ERROR);
		}
		break;
	case CODE_FLD: 
		Right_Justify_Numeric(s_sth.s_code,
			sizeof(s_sth.s_code)-1);
		strcpy(att.at_code,s_sth.s_code);

		retval = get_att(&att,BROWSE,1,e_mesg);
		if(retval < 0) {
			fomer("Invalid Attendance Code");
			s_sth.s_code[0] = LV_CHAR;
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
	case SCR_10 :		/* SCR - 10 'CHEQUE LOCATION' */
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
/*------------------------------------------------------------*/
static int
ReadScreen(mode)
int	mode;
{
	int err, i;

	scpy((char *)&image,(char *)&s_sth,sizeof(image));

	if(mode == ADD) {
		SetDupBuffers(START_FLD,END_FLD,0); /* Off Dup Control */
		InitFields(HV_CHAR,HV_LONG,HV_DOUBLE);
	}
	else {
		SetDupBuffers(START_FLD+100,START_FLD+200,1);
		SetDupBuffers(START_FLD+500,START_FLD+600,1);
		for(i=1900;i<=10900;i+=800){
			SetDupBuffers(i,i+100,1);
		}
		s_sth.s_sck_bk = LV_DOUBLE;
		s_sth.s_sd_ent = LV_DOUBLE;
		s_sth.s_vac_ent = LV_DOUBLE;
		s_sth.s_vac_bk = LV_DOUBLE;

		/* Do all sick accruals and all vacation accruals */
		for( i = 0; i < 12; i++){
			s_sth.s_items[i].s_sck_acc = LV_DOUBLE;
			s_sth.s_items[i].s_vac_acc = LV_DOUBLE;
		}
	}
	

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
			InitFields(HV_CHAR,HV_LONG,HV_DOUBLE);
			ret(WriteFields((char *)&s_sth,START_FLD,END_FLD-200));
		}
		else {

			err = CopyBack((char *)&s_sth,(char *)&image,
				sr.curfld, END_FLD);
			if(err == PROFOM_ERR) return(err);
		}
		s_sth.s_mesg[0] = HV_CHAR;
		DispMesgFld((char *)&s_sth);


		return(RET_USER_ESC) ;
	}
	ShowAtt();

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
/* takes the date entered on the screen and get the corresponding */
/* day's name. i.e. MON, TUE, etc.                                */
/*----------------------------------------------------------------*/
Get_day()
{
	long	julian;
	int	remain;
	long	long_date, end_date;
	int	day;
	int	curr_month;
	int	year;

	if(pay_param.pr_st_mth == 1){
		long_date = pay_param.pr_cal_st_dt;
		end_date = pay_param.pr_cal_end_dt;
	}
	if(pay_param.pr_st_mth == 2){
		long_date = pay_param.pr_fisc_st_dt;
		end_date = pay_param.pr_fisc_end_dt;
	}
	if(pay_param.pr_st_mth == 3){
		long_date = pay_param.pr_schl_st_dt;
		end_date = pay_param.pr_schl_end_dt;
	}

	for( ; ; ){

		julian = days(long_date);
		remain = julian % 7;

		day = long_date % 100;	
		curr_month = (long_date / 100)%100;
		year = long_date / 10000;

		if(remain == 6 || remain == 0){
		  if(curr_month >= start_mth)
		    s_sth.s_items[curr_month-start_mth].s_att[day-1] = WEEKEND; 
		  else
		    s_sth.s_items[curr_month+start_mth-2].s_att[day-1] =WEEKEND; 
		}
		if((++day) <= d_month[curr_month-1])
			long_date ++;
		else{
			if(curr_month <= 11)
			  long_date = (year*10000) + ((curr_month+1)*100) +1; 
			else
			  long_date = ((year+1)*10000) + (1*100) +1; 
		}
		if(long_date >= end_date)
			break;
	}

	return(NOERROR);
}	/* get_day() */
/*-------------------- E n d   O f   P r o g r a m ---------------------*/
