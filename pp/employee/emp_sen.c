/*-----------------------------------------------------------------------
Source Name: emp_sen.c 
System     : Ledger Base 
Module     : Personnel/Payroll
Created  On: 1992/01/20  
Created  By: Eugene Roy 

DESCRIPTION:
	Program to Maintain the Employee's Seniority.

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
#define	MOD_DATE	"20-JAN-92"		/* Program Last Modified */


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

#define PRGNM  		"emp_sen"
#define SCR_NAME	"emp_sen"

#define MAX_FIELD	5		/* Maximum field # to edit */

/* number of days in months */

static	char	month[12][4] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG",
			"SEP", "OCT", "NOV", "DEC" };

#define	JAN	01
#define	JUL	07
#define	DEC	12

/* PROFOM Field Numbers */

/* Screen Control Variables */
#define	START_FLD	900 	/* Data entry starting field */
#define	END_FLD  	14400		/* screen end field */
#define	CHG_FLD		700 	/* Data entry starting field */

#define	KEY_START	500 	/* Key Start Field 	*/

#define	FN_FLD  	400 	/* Fn: Field: 		*/

#define	INIT_FLD  	1400 	/* Fn: Field: 		*/
#define	INIT_TOT_FLD  	1900 	/* Fn: Field: 		*/

typedef	struct	{	/* Start Fld 1500, Endfld 4200 */

	char	s_mth[4];		
	double	s_cas_hours;
	double	s_cas_days;
	double	s_perm_days;
	double	s_tot_cas_days;
	short	s_tot_perm_yrs;
	double	s_tot_perm_days;
	short	s_tot_yrs;
	double	s_tot_days;
	char	s_comm[16];

}	S_item ;

/* Profom 'C' structure for the screen "schl1" i.e., for Demographic data */

typedef struct	{

	char	s_pgm[11];		/* 100 Program Name String 10	*/ 
	long	s_sysdate;		/* 300 System date DATE YYYYFMMFDD */
	char	s_fn[2];		/* 400 Fn: field String 1	*/
	char	s_emp[13];		/* 500 Employee No String 13 */
	char	s_name[31];		/* 600 Employee No String 30 */
	short	s_field;		/* 700 Field Number Numeric 99	*/
	double	s_sen_perc;
	short	s_yrs_out_dist;		/* 700 Field Number Numeric 99	*/
	double	s_days_out_dist;
	short	s_yrs_out_prov;		/* 700 Field Number Numeric 99	*/
	double	s_days_out_prov;
	char	s_dummy1[2];		/* 600 Employee No String 30 */
	char	s_dummy2[2];		/* 600 Employee No String 30 */
	double	s_init_cashrs;		/* 1150 Deferred Amount */
	double	s_init_casdays;		/* 1150 Deferred Amount */
	double	s_init_permdays;		/* 1150 Deferred Amount */
	double	s_init_castot;		/* 1150 Deferred Amount */
	short	s_init_permytot;		/* 1150 Deferred Amount */
	double	s_init_permdtot;		/* 1150 Deferred Amount */
	short	s_init_tyrs;		/* 700 Field Number Numeric 99	*/
	double	s_init_tdays;		/* 1150 Deferred Amount */
	char	s_init_comm[16];

	S_item	s_items[12] ;	/* Start Fld 1500, End Fld 4200  */

	char	s_mesg[78];		/* 2500 Message Line String 77	*/
	char	s_opt[2];		/* 2600 Message Option String X */ 

	} s_struct;

static	s_struct	s_sth, image ;

static	Emp_sen	emp_sen,	 pre_emp_sen;
static	Sen_par	sen_par;
static	Pay_param	pay_param;
static	Att	att;

static	int	Validate() ;
static	int	WindowHelp() ;
extern	double	D_Roundoff();

static	int	mode;
static	int	first_time;
static	long	start_mth;
static	long	start_date;

Emp_senior()
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
	s_sth.s_dummy1[0] = HV_CHAR ;
	s_sth.s_dummy2[0] = HV_CHAR ;
	s_sth.s_mesg[0] = HV_CHAR ;
	s_sth.s_opt[0] = HV_CHAR ;

	/* Initialize Key Fields. So that, if user selectes 'N' option
	   immediately after invoking program, then he gets the first
	   record in the file */

	s_sth.s_emp[0] = '\0';
	s_sth.s_name[0] = HV_CHAR;

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
	fomer("C(hanger), I(nterroger), S(uivant), P(recedent), F(in)");
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
	retval = ShowScreen();

	return( retval ) ;
}	/* Next() */
/*-----------------------------------------------------------------------*/
/* Take the confirmation from user for the header part */

static int
Confirm()
{
	int	err ;

	/* Options:
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
	    case  SCREENEDIT:
		err = ReadScreen();
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

	SetDupBuffers(START_FLD,END_FLD-200,1); /* Off Dup Control */

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
	ShowSen();
	ret( WriteFields((char *)&s_sth, KEY_START, END_FLD -200) ) ;

	return(NOERROR);
}
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
InitFields( t_char,t_short,t_double )
char	t_char ;
short	t_short;
double	t_double;
{
	int	i;

	s_sth.s_sen_perc = t_double;
	s_sth.s_yrs_out_dist = t_short;
	s_sth.s_days_out_dist = t_double;
	s_sth.s_yrs_out_prov = t_short;
	s_sth.s_days_out_prov = t_double;
	s_sth.s_dummy1[0] = t_char;
	s_sth.s_dummy2[0] = t_char;
	s_sth.s_init_cashrs = t_double;
	s_sth.s_init_casdays = t_double;
	s_sth.s_init_permdays = t_double;
	s_sth.s_init_castot = t_double;	
	s_sth.s_init_permytot = t_short;	
	s_sth.s_init_permdtot = t_double;
	s_sth.s_init_tyrs = t_short;
	s_sth.s_init_tdays = t_double;
	s_sth.s_init_comm[0] = t_char;


	for( i = 0; i < 12; i++){
		s_sth.s_items[i].s_mth[0] = t_char;	
		s_sth.s_items[i].s_cas_hours = t_double;
		s_sth.s_items[i].s_cas_days = t_double;
		s_sth.s_items[i].s_perm_days = t_double;
		s_sth.s_items[i].s_tot_cas_days = t_double;
		s_sth.s_items[i].s_tot_perm_yrs = t_short;	
		s_sth.s_items[i].s_tot_perm_days = t_double;
		s_sth.s_items[i].s_tot_yrs = t_short;
		s_sth.s_items[i].s_tot_days = t_double;
		s_sth.s_items[i].s_comm[0] = t_char;	
	}

	return(NOERROR) ;
}	/* InitFields() */

/*-----------------------------------------------------------------------*/ 
/* Check to see if record is to be added, changed or deleted .		 */
/*-----------------------------------------------------------------------*/ 
static int
WriteRecords()
{
	int	i,retval;

	strcpy(emp_rec.em_numb,s_sth.s_emp);
	
	strcpy(emp_rec.em_numb,s_sth.s_emp);
	retval = get_employee(&emp_rec,UPDATE,0,e_mesg);
	if(retval < 0 ) {
		DispError((char *)&s_sth,e_mesg);
		return(retval);
	}

	emp_rec.em_sen_perc = s_sth.s_sen_perc;
	emp_rec.em_yrs_out_dist = s_sth.s_yrs_out_dist;
	emp_rec.em_days_out_dist = s_sth.s_days_out_dist;
	emp_rec.em_yrs_out_prov = s_sth.s_yrs_out_prov;
	emp_rec.em_days_out_prov = s_sth.s_days_out_prov;
/*	emp_rec.em_cas_hrs = s_sth.s_init_cashrs;
	emp_rec.em_cas_days = s_sth.s_init_casdays;
	emp_rec.em_perm_days = s_sth.s_init_permdays; */
	emp_rec.em_per_tot_yrs = s_sth.s_init_permytot;
	emp_rec.em_per_tot_days = s_sth.s_init_permdtot;
	emp_rec.em_cas_tot_yrs = s_sth.s_init_tyrs;
	emp_rec.em_cas_tot_days = s_sth.s_init_tdays;

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

		first_time = 1;

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

	strcpy(sen_par.sn_position,emp_rec.em_pos);
	sen_par.sn_eff_date = get_date();
	flg_reset(SEN_PAR);

	err = get_n_sen_par(&sen_par,UPDATE,0,BACKWARD,e_mesg);
	if(strcmp(sen_par.sn_position, emp_rec.em_pos) != 0){
		fomer("Seniority Parameter Not Setup for Position Code");
		get();
	}
	if(err < 0){
		fomer(e_mesg);
		get();
	}

	err = ShowScreen();

	return(NOERROR);
}	/* SelectRecord() */
/*--------------------------------------------------------------------
 ShowSen()     - this routine reads the seniority information and   
   		 copies it onto the screen
--------------------------------------------------------------------*/
static	int
ShowSen()
{
	int 	retval, i, j, curr_month;
	int	month_nbr, nbr_of_mths = 12, prev_mon;

	strcpy(sen_par.sn_position,emp_rec.em_pos);
	sen_par.sn_eff_date = get_date();
	flg_reset(SEN_PAR);

	retval = get_n_sen_par(&sen_par,BROWSE,0,BACKWARD,e_mesg);
	if(strcmp(sen_par.sn_position, emp_rec.em_pos) != 0){
		fomer("Seniority Parameter Not Setup for Position Code");
		get();
	}
	if(retval < 0){
		fomer(e_mesg);
		get();
	}
	seq_over(SEN_PAR);
	/* initialize screen */
	Disp_Mth();

	for (i=0; i < 12; i++) {
		if((start_mth + i)> 12)
			j = i - start_mth+1 ;
		else
			j = i + start_mth-1;
		strcpy(s_sth.s_items[i].s_mth, month[j]);
		s_sth.s_items[i].s_cas_hours = 0;
		s_sth.s_items[i].s_cas_days = 0;
		s_sth.s_items[i].s_perm_days = 0;
		s_sth.s_items[i].s_tot_cas_days = 0;
		s_sth.s_items[i].s_tot_perm_yrs = 0; 
		s_sth.s_items[i].s_tot_perm_days = 0; 
		s_sth.s_items[i].s_tot_yrs = 0;
		s_sth.s_items[i].s_tot_days = 0;
		strcpy(s_sth.s_items[i].s_comm, emp_rec.em_comm[j]);
	}

	if(first_time == 1){
		s_sth.s_init_cashrs = 0;
		s_sth.s_init_casdays = 0;
		s_sth.s_init_permdays = 0;
		s_sth.s_init_castot =  0; 
		s_sth.s_init_permytot = emp_rec.em_per_tot_yrs;
		s_sth.s_init_permdtot = emp_rec.em_per_tot_days;
		s_sth.s_init_tyrs = emp_rec.em_cas_tot_yrs;
		s_sth.s_init_tdays = emp_rec.em_cas_tot_days;
	}
	first_time = 0;

	if(s_sth.s_items[0].s_tot_days > sen_par.sn_max_days_yr){ 
		s_sth.s_items[0].s_tot_yrs ++;
		s_sth.s_items[0].s_tot_days -= sen_par.sn_max_days_yr;
	}
	curr_month = 0;

	strcpy(emp_sen.esn_numb, emp_rec.em_numb);
	emp_sen.esn_month =  0;
	emp_sen.esn_pos[0] = '\0';
	emp_sen.esn_class[0] = '\0';
	flg_reset(EMP_SEN);

	for( ; ; ){
		retval = get_n_emp_sen(&emp_sen, BROWSE, 0, FORWARD, e_mesg);
		if(retval == EFL)
			break;
		if(strcmp(emp_sen.esn_numb, emp_rec.em_numb) != 0)
			break;

		if(retval < 0) {
			fomen(e_mesg);
			get();
			return(-1);
		}

		curr_month = emp_sen.esn_month;

		if(curr_month < start_mth)
			curr_month += nbr_of_mths;

		s_sth.s_items[curr_month - start_mth].s_cas_hours += 
				emp_sen.esn_cas_hrs;
		s_sth.s_items[curr_month - start_mth].s_cas_days += 
				emp_sen.esn_cas_days;
		s_sth.s_items[curr_month - start_mth].s_perm_days += 
				emp_sen.esn_perm_days;
		s_sth.s_items[curr_month - start_mth].s_tot_cas_days += 
				emp_sen.esn_cas_totd;
	}
	seq_over(EMP_SEN);

	month_nbr = 1;

	for(prev_mon = start_mth; month_nbr<= nbr_of_mths; prev_mon++){

		if(prev_mon == start_mth){
			s_sth.s_items[prev_mon-start_mth].s_tot_perm_yrs =
				s_sth.s_init_permytot; 
	 		s_sth.s_items[prev_mon-start_mth].s_tot_perm_days =
				s_sth.s_init_permdtot +
				s_sth.s_items[prev_mon-start_mth].s_perm_days;

			s_sth.s_items[prev_mon-start_mth].s_tot_yrs = 
			   s_sth.s_init_tyrs;
			s_sth.s_items[prev_mon-start_mth].s_tot_days = 
        		   s_sth.s_init_tdays +
			   s_sth.s_items[prev_mon-start_mth].s_tot_cas_days;
		}
		else{
			s_sth.s_items[prev_mon-start_mth].s_tot_perm_yrs =
			   s_sth.s_items[prev_mon-start_mth-1].s_tot_perm_yrs;
	 		s_sth.s_items[prev_mon-start_mth].s_tot_perm_days =
	 		   s_sth.s_items[prev_mon-start_mth-1].s_tot_perm_days+
			   s_sth.s_items[prev_mon-start_mth].s_perm_days;

			s_sth.s_items[prev_mon-start_mth].s_tot_yrs = 
			   s_sth.s_items[prev_mon-start_mth-1].s_tot_yrs;
			s_sth.s_items[prev_mon-start_mth].s_tot_days = 
			   s_sth.s_items[prev_mon-start_mth-1].s_tot_days +
			   s_sth.s_items[prev_mon-start_mth].s_tot_cas_days;
		}

		if(s_sth.s_items[prev_mon-start_mth].s_tot_perm_days >
			sen_par.sn_max_days_yr){
		    	   s_sth.s_items[prev_mon-start_mth].s_tot_perm_yrs ++;
		    	   s_sth.s_items[prev_mon-start_mth].s_tot_perm_days -=
				sen_par.sn_max_days_yr;
		}

		if(s_sth.s_items[prev_mon-start_mth].s_tot_days >
				sen_par.sn_max_days_yr){
		    	   s_sth.s_items[prev_mon-start_mth].s_tot_yrs ++;
		    	   s_sth.s_items[prev_mon-start_mth].s_tot_days -=
				sen_par.sn_max_days_yr;
		}
		month_nbr++;
	}

	return(NOERROR);
}
/*-----------------------------------------------------------*/
static
Disp_Mth()
{
	int	retval;

 	retval = get_pay_param(&pay_param,BROWSE,1,e_mesg);
	if(retval < 0) {
  		DispError((char *)&s_sth,e_mesg);
		return(retval);
	}

	if(pay_param.pr_st_mth == 1){
		start_date = pay_param.pr_cal_st_dt;
		start_mth = ((pay_param.pr_cal_st_dt / 100) % 100);
	}
	if(pay_param.pr_st_mth == 2){
		start_date = pay_param.pr_fisc_st_dt;
		start_mth = ((pay_param.pr_fisc_st_dt / 100) % 100);
	}
	if(pay_param.pr_st_mth == 3){
		start_date = pay_param.pr_schl_st_dt;
		start_mth = ((pay_param.pr_schl_st_dt / 100) % 100);
	}
	return(NOERROR);
}
/*-----------------------------------------------------------*/
/* Move Header to Screen Hdr Fields */
static int
ShowScreen()
{
	int	i, retval;

	strcpy(e_mesg,emp_rec.em_first_name);
	strcat(e_mesg," ");
	strcat(e_mesg,emp_rec.em_last_name);
	strncpy(s_sth.s_name,e_mesg,31);

	s_sth.s_sen_perc = emp_rec.em_sen_perc;
	s_sth.s_yrs_out_dist = emp_rec.em_yrs_out_dist;
	s_sth.s_days_out_dist = emp_rec.em_days_out_dist;
	s_sth.s_yrs_out_prov = emp_rec.em_yrs_out_prov;
	s_sth.s_days_out_prov = emp_rec.em_days_out_prov;
	s_sth.s_dummy1[0] = ' ';
	s_sth.s_dummy2[0] = ' ';
	s_sth.s_init_cashrs = 0;
	s_sth.s_init_casdays = 0;
	s_sth.s_init_permdays = 0;
	s_sth.s_init_castot = 0; 
	s_sth.s_init_permytot = emp_rec.em_per_tot_yrs;
	s_sth.s_init_permdtot = emp_rec.em_per_tot_days;
	s_sth.s_init_tyrs = emp_rec.em_cas_tot_yrs;
	s_sth.s_init_tdays = emp_rec.em_cas_tot_days;

	ShowSen();

	ret( WriteFields((char *)&s_sth, KEY_START, END_FLD -200) ) ;

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
		strcpy(s_sth.s_name, emp_rec.em_last_name);
		strcat(s_sth.s_name, ", ");
		strcat(s_sth.s_name, emp_rec.em_first_name);
		strcat(s_sth.s_name, " ");
		strcat(s_sth.s_name, emp_rec.em_mid_name);
		break;
	case INIT_TOT_FLD :
/*	        s_sth.s_init_tdays = s_sth.s_init_permdtot +
				s_sth.s_init_castot;
		s_sth.s_init_tyrs = s_sth.s_init_permytot;
		if(s_sth.s_init_tdays > sen_par.sn_max_days_yr){
			s_sth.s_init_tyrs ++;
			s_sth.s_init_tdays -= sen_par.sn_max_days_yr;
		}*/
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
	case SCR_11 :		/* SCR - 11 'ATTENDANCE' */
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
static
ReadScreen()
{
	int err, i;

	scpy((char *)&image,(char *)&s_sth,sizeof(image));

	SetDupBuffers(START_FLD,END_FLD-200,1); /* Off Dup Control */

#ifdef	ENGLISH
	strcpy(s_sth.s_mesg,"Press ESC-F to Terminate Edit");
#else
	strcpy(s_sth.s_mesg,"Appuyer sur ESC-F pour terminer l'ajustement");
#endif
	DispMesgFld((char*)&s_sth) ;

	/* Read data area of screen in single fomrd() */
	s_sth.s_sen_perc = LV_DOUBLE;
	s_sth.s_yrs_out_dist = LV_SHORT;
	s_sth.s_days_out_dist = LV_DOUBLE;
	s_sth.s_yrs_out_prov = LV_SHORT;
	s_sth.s_days_out_prov = LV_DOUBLE;
/*	s_sth.s_init_cashrs = LV_DOUBLE;
	s_sth.s_init_casdays = LV_DOUBLE;
	s_sth.s_init_permdays = LV_DOUBLE;
	s_sth.s_init_castot = LV_DOUBLE;  */	
	s_sth.s_init_permytot = LV_SHORT;	
	s_sth.s_init_permdtot = LV_DOUBLE;
	s_sth.s_init_tyrs = LV_SHORT;
	s_sth.s_init_tdays = LV_DOUBLE;
	for(i=0; i<12; i++)
		s_sth.s_items[i].s_comm[0] = LV_CHAR;

	err = ReadFields((char*)&s_sth, START_FLD, END_FLD-200, Validate,
			WindowHelp, 1) ;
	if(err == DBH_ERR || err == PROFOM_ERR) return(err) ;
	if(PROFOM_ERR == err || DBH_ERR == err) return(err) ;
	if(RET_USER_ESC == err){
		err = CopyBack((char *)&s_sth,(char *)&image,
			sr.curfld, END_FLD);
		if(err == PROFOM_ERR) return(err);
		s_sth.s_mesg[0] = HV_CHAR;
		DispMesgFld((char *)&s_sth);

		return(RET_USER_ESC) ;
	}
	ShowSen();
	ret( WriteFields((char *)&s_sth, KEY_START, END_FLD -200) ) ;

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
/*-------------------- E n d   O f   P r o g r a m ---------------------*/
