/*-----------------------------------------------------------------------
Source Name: emp_st.c 
System     : Ledger Base 
Module     : Personnel/Payroll
Created  On: 1992/10/04  
Created  By: Eugene Roy

DESCRIPTION:
	Program to Inquire on the Employee's Status.

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
------------------------------------------------------------------------*/
#define  MAINFL		TIME 		/* main file used */

#include <stdio.h>
#include <ctype.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_pp.h>
#include <bfs_com.h>
#include <empldrvr.h>

#define	SYSTEM		"PESONNEL/PAYROLL"	/* Sub System Name */
#define	MOD_DATE	"92-OCT-04"		/* Program Last Modified */

/* PROFOM Field Numbers */
/* User Interface define constants */
#ifdef ENGLISH
#define INQUIRE		'I'
#define EXITOPT		'E'
#define	NEXT		'N'
#define	PREV		'P'
#define	SCREEN		'S'

#define	YES		'Y'
#define NO		'N'
#define	CANCEL		'C'

#else
#define INQUIRE		'I'
#define EXITOPT		'F'
#define	NEXT		'S'
#define	PREV		'P'
#define	SCREEN		'S'

#define	YES		'O'
#define NO		'N'
#define	CANCEL		'A'
#endif

#define	WEEKEND		'W'


/* PROFOM Related declarations */

#define SCR_NAME	"emp_st"

/* number of days in months */

char		e_mesg[180];  		/* dbh will return err msg in this */

static short d_month[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

static char month[12][4] =
		 { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG",
			"SEP", "OCT", "NOV", "DEC" };

/* PROFOM Field Numbers */

/* Screen Control Variables */
#define	START_FLD	900 	/* Data entry starting field */
#define	END_FLD  	9100		/* screen end field */
#define	CHG_FLD		700 	/* Data entry starting field */

#define	KEY_START	500 	/* Key Start Field 	*/

#define	MTH_ST		1100 	/* Key Start Field 	*/

#define	FN_FLD  	400 	/* Fn: Field: 		*/

#ifdef MAIN
struct stat_rec	sr;
#else
extern struct stat_rec sr;
#endif

typedef	struct	{	/* Start Fld 1100, Endfld 8800 */

	char	s_mth[4];		/* Empl Month Desc */
	char	s_stat[32];		/* Display status field for month */
	double	s_mtd_days;		/* month to date total days  */
	double	s_mtd_hrs;		/* month to date total hours */
	double	s_mtd_tot;		/* month to date total hours */
	double	s_ytd_bal;		/* accum. year to date hours */

}	S_item ;

/* Profom 'C' structure for the screen "schl1" i.e., for Demographic data */

typedef struct	{

	char	s_pgm[11];		/* 100 Program Name String 10	*/ 
	long	s_sysdate;		/* 300 System date DATE YYYYFMMFDD */
	char	s_fn[2];		/* 400 Fn: field String 1	*/
	char	s_emp[13];		/* 500 Employee No String 13 */
	long	s_date;
	short	s_field;		/* 700 Field Number Numeric 99	*/
	
	char	s_name[28];		/* 600 Employee No String 30 */
	char	s_dummy1[4];            /* 900 Dummy field # 1 */
	char	s_dummy2[2];            /* 1000 Dummy field # 2 */

	S_item	s_items[13] ;	/* Start Fld 1100, End Fld 8800  */

	char	s_mesg[78];		/* 8900 Message Line String 77	*/
	char	s_opt[2];		/* 9000 Message Option String X */ 

	} s_struct;

static	s_struct	s_sth, image ;

static	Pay_param	pay_param;
static	Barg_unit	barg_unit;
static	Time	time;
static	Time_his	time_his;
static	Pay_per	pay_period;
static	Pay_per_it	pay_per_it;
static	Class	class_rec;
static	Sen_par	sen_par;
static	Position	position;

static	int	Validate() ;
static	int	WindowHelp() ;
extern	double	D_Roundoff();

static	int	mode;
static	int	first_time;
static	double	non_sen_days;
static	long	start_mth;
static	long	start_date;
static	int	day, mth, year;

Emp_st()
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
			err = ShowStatus();
		}
	}

	err = Process(); 	/* Initiate Process */

	return( err ) ;

} /* DemoGraph() */
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
/*----------------------------------------------------------------*/
/* Initialize screen before going to process options */
static int
InitScreen()
{
	int	err ;

	/* move screen name to Profom status structure */
	strcpy(sr.scrnam,NFM_PATH);
	strcat(sr.scrnam,SCR_NAME) ;

	strcpy(s_sth.s_pgm,"emp_st");

	s_sth.s_sysdate = get_date();	/* get Today's Date in YYYYMMDD format*/
	s_sth.s_date = get_date();
	s_sth.s_name[0] = HV_CHAR ;
	s_sth.s_field = HV_SHORT ;
	s_sth.s_mesg[0] = HV_CHAR ;
	s_sth.s_opt[0] = HV_CHAR ;

	/* Move High Values to data fields and Display the screen */
	err = ClearScreen() ;
	if(NOERROR != err) return(err) ;

	return(NOERROR) ;
}	/* InitScreen() */
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
	fomer("I(nquire), F(orward), B(ackward), P(revious), S(creen), E(xit)");
#else
	fomer("I(nterroger), P(recedent), F(in)");
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
	case INQUIRE  :			/* Inquire */
		CHKACC(retval,BROWSE,e_mesg);
		mode = BROWSE;
		return( Inquire() ) ;
	case NEXT_RECORD  :			/* Next */
		CHKACC(retval,BROWSE,e_mesg);
		return( Next(FORWARD) ) ;
	case PREV_RECORD  :			/* Previous */
		CHKACC(retval,BROWSE,e_mesg);
		return( Next(BACKWARD) ) ;
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
	case SCR_12 :		/* SCR - 12 'SENIORITY' */
	case SCR_13 :		/* SCR - 13 'TEACHER QUALIFICATION' */
	case SCR_14 :		/* SCR - 14 'TEACHER ASSIGNMENT' */
	case SCR_15 :		/* SCR - 15 'COMPETITION' */
		Cur_Option = s_sth.s_field ;
		return( JUMP ) ;
	default   : 
		return(NOERROR);
	}  /*   end of the switch statement */

	return(NOERROR);
}
/*-----------------------------------------------------------------------*/
/* Show Employee Status Data                                             */
static int
Inquire()
{
	int	err ;

	err = SelectRecord() ;
	if(NOERROR != err) return(err) ;

	return(NOERROR) ;
}	/* Inquire() */
/*----------------------------------------------------------*/
/* Show the next or previous Employees status data          */

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
	retval = ShowStatus();

	return( retval ) ;
}	/* Next() */
/*------------------------------------------------------------------------*/
/* Move High values to all data fields and clear the screen */

static
ClearScreen()
{
	/* Move High Values to Header part */
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
	if(t_char == HV_CHAR){
		s_sth.s_dummy1[0] = HV_CHAR;
		s_sth.s_dummy2[0] = HV_CHAR;
	}
	else{
		s_sth.s_dummy1[0] = t_char;
		s_sth.s_dummy2[0] = t_char;
	}

	for( i = 0; i < 13; i++){
		s_sth.s_items[i].s_mth[0] = t_char;	
		s_sth.s_items[i].s_stat[0] = t_char;
		s_sth.s_items[i].s_mtd_days = t_double;
		s_sth.s_items[i].s_mtd_hrs = t_double;	
		s_sth.s_items[i].s_mtd_tot = t_double;
		s_sth.s_items[i].s_ytd_bal = t_double;
	}

	return(NOERROR) ;
}	/* InitFields() */

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
	err = ShowStatus();

	return(NOERROR);
}	/* SelectRecord() */
/*--------------------------------------------------------------------
 ShowStatus()  - this routine reads the employee seniority file and   
   		 shows what days the employee has hours for 
--------------------------------------------------------------------*/
int
ShowStatus()
{
	int 	retval, i,j, curr_month, prev_mon, leap_year ;
	int	nbr_of_mths = 12,nbr_of_times = 13, records_found, curr_day;
	int	month_nbr, start_year, curr_year, first_time, this_year;
	short	start_pp_numb, start_pp_year,end_pp_numb,
		end_pp_year;
	long	end_date, time_unit_date;
	long	one = 1, value;

	for(i=0;i<13;i++) {
		strcpy(s_sth.s_items[i].s_stat, 
			"                               ");
	}

	curr_month = 1;

	/* initialize screen */

	end_date = s_sth.s_date;

	start_mth = ((s_sth.s_date/100)%100);

	start_date = s_sth.s_date - 10000;
	start_date += 1;
	mth = ((start_date/100)%100);
	day = (start_date % 100);
	year = (start_date / 10000);
	leap_year = year % 4; 
	if(mth == 2 && day == 29 && leap_year == 0)
		start_date = (year * 10000) + (mth * 100) + day;
	else {
		if(day > d_month[mth-1]) {
			day = 1;
			if(mth == 12){
				mth = 1;
				year++;
			}
			else
				mth++;
			start_date = (year * 10000) + (mth * 100) + day;
		}
	}
	
	start_year = start_date / 10000;

	first_time = 0;

	for (i=0; i < 13; i++) {
		if((start_mth + i )> 12)
			j = start_mth+i - 12 - 1 ;
		else
			j = i + start_mth-1;
		if(first_time == 0){
			first_time = 1;
			this_year = start_year;
		}
		else {
			if(j == 0)
				this_year++;
		}	
		strcpy(s_sth.s_items[i].s_mth, month[j]);
		/* Check for leap year	*/
		if((this_year % 4 == 0) && j == 1)
			strncpy(s_sth.s_items[i].s_stat, 
			"...............................",d_month[j]+1);
		else
			strncpy(s_sth.s_items[i].s_stat, 
			"...............................",d_month[j]);
		s_sth.s_items[i].s_mtd_days = 0;	
		s_sth.s_items[i].s_mtd_hrs = 0;
		s_sth.s_items[i].s_mtd_tot = 0;
		s_sth.s_items[i].s_ytd_bal = 0; 
	}

	Get_day();

	strcpy(barg_unit.b_code,emp_rec.em_barg);
	barg_unit.b_date = s_sth.s_date;
	flg_reset(BARG);

	retval = get_n_barg(&barg_unit,BROWSE,0,BACKWARD,e_mesg);
	if(retval < 0 || strcmp(barg_unit.b_code,emp_rec.em_barg)!=0){
		DispError((char *)&s_sth,"Error Reading Bargaining Unit File");
		return(-1);
	}
	seq_over(BARG);

	strcpy(pay_per_it.ppi_code,barg_unit.b_pp_code);
	pay_per_it.ppi_st_date = start_date;
	flg_reset(PAY_PER_ITEM);

	retval = get_n_pp_it(&pay_per_it,BROWSE,1,BACKWARD,e_mesg);
	if(retval < 0 || strcmp(pay_per_it.ppi_code,barg_unit.b_pp_code)!=0){
		DispError((char *)&s_sth,"Error Reading Pay Period Item File");
		return(NOERROR);
	}
	seq_over(PAY_PER_ITEM);
	start_pp_numb = pay_per_it.ppi_numb;
	start_pp_year = pay_per_it.ppi_year;

	strcpy(pay_per_it.ppi_code,barg_unit.b_pp_code);
	pay_per_it.ppi_st_date = end_date;
	flg_reset(PAY_PER_ITEM);

	retval = get_n_pp_it(&pay_per_it,BROWSE,1,BACKWARD,e_mesg);
	if(retval < 0 || strcmp(pay_per_it.ppi_code,barg_unit.b_pp_code)!=0){
		DispError((char *)&s_sth,"Error Reading Pay Period Item File");
		return(NOERROR);
	}
	seq_over(PAY_PER_ITEM);
	end_pp_numb = pay_per_it.ppi_numb;
	end_pp_year = pay_per_it.ppi_year;

	strcpy(time_his.tmh_numb, emp_rec.em_numb);
	time_his.tmh_year = start_pp_year;
	time_his.tmh_pp = start_pp_numb;
	time_his.tmh_week = 0;

	flg_reset(TIME_HIS);

	for(;;){
		retval = get_n_time_his(&time_his,BROWSE,1,FORWARD,e_mesg);
		if(retval == EFL ||	
		   (strcmp(time_his.tmh_numb, emp_rec.em_numb)) != 0){
			break;
		}
		if(retval < 0) {
			fomen(e_mesg);
			get();
			return(-1);
		}
		if(time_his.tmh_pp > end_pp_numb && 
		   time_his.tmh_year >= end_pp_year)
			break;
		
		strcpy(class_rec.c_code,time_his.tmh_class);
		class_rec.c_date = s_sth.s_date;
		flg_reset(CLASSIFICATION);

		retval = get_n_class(&class_rec,BROWSE,0,BACKWARD,e_mesg);
		if(retval < 0 || 
		   strcmp(class_rec.c_code,time_his.tmh_class) != 0) {
			DispError((char *)&s_sth,
			         "Error Reading Class File");
			get();
			return(-1);
		}

	  	strcpy(position.p_code,class_rec.c_pos);
	  	retval = get_position(&position,BROWSE,0,e_mesg);

		if(retval < 0){
			DispError((char *)&s_sth,
			         "Error Reading Position File");
			get();
			return(-1);
		}

	 	if((strcmp(position.p_type,"FT")==0)|| 
				 (strcmp(position.p_type,"PT")== 0))
			continue;

		strcpy(sen_par.sn_position,class_rec.c_pos);
		sen_par.sn_eff_date = s_sth.s_date;
		flg_reset(SEN_PAR);

		retval = get_n_sen_par(&sen_par, BROWSE, 0, BACKWARD, e_mesg);
		if(retval<0 || strcmp(sen_par.sn_position,class_rec.c_pos)!=0) {
			DispError((char *)&s_sth,
			         "Error Reading Seniority Parameter File");
			return(-1);
		}
		seq_over(SEN_PAR);

		strcpy(pay_per_it.ppi_code,barg_unit.b_pp_code);
		pay_per_it.ppi_year = time_his.tmh_year;
		pay_per_it.ppi_numb = time_his.tmh_pp;

		retval = get_pp_it(&pay_per_it,BROWSE,0,e_mesg);
		if(retval < 0){ 
			DispError((char *)&s_sth,
			         "Error Reading Pay Period Item File");
			continue;
		}

		if(time_his.tmh_week == 1)
			time_unit_date = pay_per_it.ppi_st_date;
		else{
			time_unit_date = pay_per_it.ppi_st_date;
			for(i=0;i<7;i++){
				curr_day = (time_unit_date % 100);
				curr_month = ((time_unit_date/100)%100);
				curr_year = time_unit_date / 10000;
				curr_day++;
				leap_year = curr_year % 4;
				if(leap_year == 0 && curr_month == 2){
				  if(curr_day > 29){
					if(curr_month == 12){
						curr_year++;
						curr_month = 1;
					}
					else
						curr_month++;
					curr_day = 1;
				  }
				}
				else {
				  if(curr_day > d_month[curr_month - 1]){
					if(curr_month == 12){
						curr_year++;
						curr_month = 1;
					}
					else
						curr_month++;
					curr_day = 1;
				  }
				}
			      time_unit_date=(curr_year*10000)+(curr_month*100)+
					curr_day;
			}
		}
		for(i=0;i<7;i++){
		    if(time_unit_date > end_date)
			break;
			
		    if(time_his.tmh_units[i]!=0&&time_unit_date>=start_date){
		      curr_day = (time_unit_date % 100);
		      curr_month = ((time_unit_date / 100) % 100);
		      curr_year = time_unit_date / 10000;
		      if((curr_month == start_mth && 
		        curr_year > start_year) 
		        || (curr_month < start_mth)){
		 	  curr_month += nbr_of_mths;
		      }
		      if(sen_par.sn_num_hrs_day == 0){
		          s_sth.s_items[curr_month-start_mth].s_mtd_days+=
		     	 	 time_his.tmh_units[i];
			  value = (long)time_his.tmh_units[i];
			  if(value != one){
		                 s_sth.s_items[curr_month-start_mth].s_stat[curr_day-1]= 
				'#';
			  }
			  else{
		                 s_sth.s_items[curr_month-start_mth].s_stat[curr_day-1]= 
				'*';
			  }
		      }
		      else{	
		          s_sth.s_items[curr_month-start_mth].s_mtd_hrs += 
		      	  	time_his.tmh_units[i];
		          s_sth.s_items[curr_month-start_mth].s_stat[curr_day-1]= 
				'#';
		      }
		    }
		    if(i != 6){	
		        curr_day = (time_unit_date % 100);
		        curr_month = ((time_unit_date / 100) % 100);
		        curr_year = time_unit_date / 10000;
			curr_day++;
			leap_year = curr_year % 4;
			if(leap_year == 0 && curr_month == 2) {
			  if(curr_day > 29){
				if(curr_month == 12){
					curr_year++;
					curr_month = 1;
				}
				else
					curr_month++;
				curr_day = 1;
			  }
			}
			else {
			  if(curr_day > d_month[curr_month - 1]){
				if(curr_month == 12){
					curr_year++;
					curr_month = 1;
				}
				else
					curr_month++;
				curr_day = 1;
			  }
			}
			time_unit_date=(curr_year*10000)+(curr_month*100)+
					curr_day;
		    }
		}
	}
	seq_over(TIME_HIS);

	strcpy(time.tm_numb, emp_rec.em_numb);
	time.tm_year = start_pp_year;
	time.tm_pp = start_pp_numb;
	time.tm_week = 0;

	flg_reset(TIME);

	for(;;){
		retval = get_n_ptime(&time,BROWSE,3,FORWARD,e_mesg);
		if(retval == EFL ||	
		   (strcmp(time.tm_numb, emp_rec.em_numb)) != 0){
			break;
		}
		if(retval < 0) {
			fomen(e_mesg);
			get();
			return(-1);
		}
		if(time.tm_pp > end_pp_numb && 
		   time.tm_year >= end_pp_year){
			break;
		}
		strcpy(class_rec.c_code,time.tm_class);
		class_rec.c_date = s_sth.s_date;
		flg_reset(CLASSIFICATION);

		retval = get_n_class(&class_rec,BROWSE,0,BACKWARD,e_mesg);
		if(retval < 0 || 
		   strcmp(class_rec.c_code,time.tm_class) != 0) {
			DispError((char *)&s_sth,
			         "Error Reading Class File");
			get();
			return(-1);
		}

	  	strcpy(position.p_code,class_rec.c_pos);
	  	retval = get_position(&position,BROWSE,0,e_mesg);

		if(retval < 0){
			DispError((char *)&s_sth,
			         "Error Reading Position File");
			get();
			return(-1);
		}

	 	if((strcmp(position.p_type,"FT")==0)|| 
				 (strcmp(position.p_type,"PT")== 0))
			continue;

		strcpy(sen_par.sn_position,class_rec.c_pos);
		sen_par.sn_eff_date = s_sth.s_date;
		flg_reset(SEN_PAR);

		retval = get_n_sen_par(&sen_par, BROWSE, 0, BACKWARD, e_mesg);
		if(retval<0 || strcmp(sen_par.sn_position,class_rec.c_pos)!=0) {
			DispError((char *)&s_sth,
			         "Error Reading Seniority Parameter File");
			return(-1);
		}
		seq_over(SEN_PAR);

		strcpy(pay_per_it.ppi_code,barg_unit.b_pp_code);
		pay_per_it.ppi_year = time.tm_year;
		pay_per_it.ppi_numb = time.tm_pp;

		retval = get_pp_it(&pay_per_it,BROWSE,0,e_mesg);
		if(retval < 0){ 
			DispError((char *)&s_sth,
			         "Error Reading Pay Period Item File");
			continue;
		}

		if(time.tm_week == 1)
			time_unit_date = pay_per_it.ppi_st_date;
		else{
			time_unit_date = pay_per_it.ppi_st_date;
			for(i=0;i<7;i++){
				curr_day = (time_unit_date % 100);
				curr_month = ((time_unit_date/100)%100);
				curr_year = time_unit_date / 10000;
				curr_day++;
				leap_year = curr_year % 4;
			    	if(leap_year == 0 && curr_month == 2){	
				  if(curr_day > 29){
					if(curr_month == 12){
						curr_year++;
						curr_month = 1;
					}
					else
						curr_month++;
					curr_day = 1;
				  }
			      	}
				else {
				  if(curr_day > d_month[curr_month-1]){
					if(curr_month == 12){
						curr_year++;
						curr_month = 1;
					}
					else
						curr_month++;
					curr_day = 1;
				  }
				}
			      time_unit_date=(curr_year*10000)+(curr_month*100)+
					curr_day;
			}
		}
		for(i=0;i<7;i++){
		    if(time_unit_date > end_date)
			break;
			
		    if(time.tm_units[i]!=0&&time_unit_date>=start_date){
		      curr_day = (time_unit_date % 100);
		      curr_month = ((time_unit_date / 100) % 100);
		      curr_year = time_unit_date / 10000;
		      if((curr_month == start_mth && 
		        curr_year > start_year) 
		        || (curr_month < start_mth)){
		 	  curr_month += nbr_of_mths;
		      }
		      if(sen_par.sn_num_hrs_day == 0){
		          s_sth.s_items[curr_month-start_mth].s_mtd_days+=
		     	 	 time.tm_units[i];
			  value = (long)time.tm_units[i];
			  if(value != one){
		                 s_sth.s_items[curr_month-start_mth].s_stat[curr_day-1]= 
				'#';
			  }
			  else{
		                 s_sth.s_items[curr_month-start_mth].s_stat[curr_day-1]= 
				'*';
			  }
		      }
		      else{
		          s_sth.s_items[curr_month-start_mth].s_mtd_hrs += 
		      	  	time.tm_units[i];
		          s_sth.s_items[curr_month-start_mth].s_stat[curr_day-1] = 
				'#';
		      }
		    }
		    if(i != 6){	
		        curr_day = (time_unit_date % 100);
		        curr_month = ((time_unit_date / 100) % 100);
		        curr_year = time_unit_date / 10000;
			curr_day++;
			leap_year = curr_year % 4;
			if(leap_year == 0 && curr_month == 2){
			  if(curr_day > 29){
				if(curr_month == 12){
					curr_year++;
					curr_month = 1;
				}
				else
					curr_month++;
				curr_day = 1;
			  }
			}
			else{
			  if(curr_day > d_month[curr_month - 1]){
				if(curr_month == 12){
					curr_year++;
					curr_month = 1;
				}
				else
					curr_month++;
				curr_day = 1;
			  }
			}
			time_unit_date=(curr_year*10000)+(curr_month*100)+
					curr_day;
		    }
		}
	}
	seq_over(TIME);

	month_nbr = 1;

	for(prev_mon=start_mth;month_nbr<=nbr_of_times;prev_mon++) {
		s_sth.s_items[prev_mon-start_mth].s_mtd_tot = 
			s_sth.s_items[prev_mon-start_mth].s_mtd_hrs +
		        (s_sth.s_items[prev_mon-start_mth].s_mtd_days *
			barg_unit.b_stat_hpd);
		month_nbr++;
	}

	month_nbr = 1;

	for(prev_mon=start_mth;month_nbr<=nbr_of_times;prev_mon++) {

		if(prev_mon == start_mth){
			s_sth.s_items[prev_mon-start_mth].s_ytd_bal = 
				s_sth.s_items[prev_mon-start_mth].s_mtd_tot;
		}	
		else {
			s_sth.s_items[prev_mon-start_mth].s_ytd_bal = 
				s_sth.s_items[prev_mon-start_mth-1].s_ytd_bal + 
				s_sth.s_items[prev_mon-start_mth].s_mtd_tot; 
		}
		month_nbr++;
	}

	if((WriteFields((char*)&s_sth, MTH_ST, END_FLD-200))<0)
		return(-1);

	return(NOERROR);
}
/*-----------------------------------------------------------*/
/* Move Header to Screen Hdr Fields */
static int
ShowScreen()
{
	strcpy(s_sth.s_name, emp_rec.em_last_name);
	strcat(s_sth.s_name, ", ");
	strcat(s_sth.s_name, emp_rec.em_first_name);
	strcat(s_sth.s_name, " ");
	strcat(s_sth.s_name, emp_rec.em_mid_name);
	s_sth.s_name[27] = '\0';

	s_sth.s_dummy1[0] = ' ';
	s_sth.s_dummy2[0] = ' ';
	ret( WriteFields((char *)&s_sth, KEY_START, MTH_ST) ) ;

	return(NOERROR) ;
}	/* ShowScreen() */
/*----------------------------------------------------------------------*/
/* Get the employee number key from user. In ADD mode disable dup buffers, */
/* other modes enable dup buffers and show the current key as a default key */
static int
ReadKey()
{
	int	i;
	char	hold_emp[13];
	long	hold_date;
	
	/* In Add mode turn off dup control for key fields.
	   Other modes reverse it */
	SetDupBuffers(KEY_START,KEY_START+100,1);

#ifdef ENGLISH
	strcpy(s_sth.s_mesg,"Press ESC-F to Go Back to Fn:");
#else
	strcpy(s_sth.s_mesg,"Appuyer sur ESC-F pour retourner a Fn:");
#endif
	DispMesgFld((char *)&s_sth);

	strcpy(hold_emp,s_sth.s_emp);
	hold_date = s_sth.s_date;

	s_sth.s_emp[0] = LV_CHAR;
	s_sth.s_date = LV_LONG;

	i = ReadFields((char *)&s_sth,KEY_START, KEY_START+100,
		Validate, WindowHelp,1) ;
	if(PROFOM_ERR == i || DBH_ERR == i) return(i) ;

	if(RET_USER_ESC == i){
		strcpy(s_sth.s_emp,hold_emp);
		s_sth.s_date = hold_date;
		
		ret( WriteFields((char *)&s_sth,KEY_START, KEY_START+100) ) ;

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
		if(retval < 0)  {
			fomer(e_mesg);
			s_sth.s_emp[0] = LV_CHAR;
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
	       retval=UsrBargVal(mode,emp_rec.em_numb,emp_rec.em_barg,1,e_mesg);
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
static
Get_day()
{
	long	julian, long_date, blank_date, end_date;
	int	remain, day, curr_month, year, end_day, end_month, leap_year,
		end_year, blank_year, blank_month, blank_day, i, start_year;

	day = start_date % 100;	
	curr_month = (start_date / 100)%100;
	year = start_date / 10000;
	start_year = year;

	blank_date = (year * 10000) + (curr_month * 100) + 1;
	blank_day = blank_date % 100;
	if(day == 1)
		strcpy(s_sth.s_items[0].s_stat,
			"                               ");
	else {
		for(i=blank_day;i<day;i++)
			s_sth.s_items[0].s_stat[i-1] = ' ';
	}
	end_date = s_sth.s_date;
	end_year = end_date / 10000;
	end_day = end_date % 100;
	end_month = (end_date/100)%100;
	leap_year = end_year % 4;
	if(leap_year == 0 && mth == 2) {
		if((++end_day) <= 29){
			end_date++;
			end_day = end_date % 100;
			for(i=end_day;i<=29;i++){
				s_sth.s_items[12].s_stat[i-1] = ' ';
			}
		}
	}
	else {
		if((++end_day) <= d_month[end_month-1]){
			end_date++;
			end_day = end_date % 100;
			for(i=end_day;i<=d_month[end_month-1];i++){
				s_sth.s_items[12].s_stat[i-1] = ' ';
			}
		}
	}
		
	long_date = start_date;

	for( ; ; ){

		julian = days(long_date);
		remain = julian % 7;

		day = long_date % 100;	
		curr_month = (long_date / 100)%100;
		year = long_date / 10000;
		leap_year = year % 4;
		
		if(curr_month == start_mth && year > start_year)
			curr_month += 12;

		if(remain == 6 || remain == 0){
		  if(curr_month >= start_mth)
		    s_sth.s_items[curr_month-start_mth].s_stat[day-1]= WEEKEND; 
		  else{
		    s_sth.s_items[12-start_mth+curr_month].s_stat[day-1]=
									WEEKEND;
		  }
		}
		curr_month = (long_date / 100)%100;
		if(leap_year == 0 && curr_month == 2){
			if((++day) <= 29)
				long_date++;
			else{
				if(curr_month <= 11)
				  long_date=(year*10000)+((curr_month+1)*100)+1; 
				else
			  	  long_date = ((year+1)*10000) + (1*100) +1; 
			}
		}
		else{	
			if((++day) <= d_month[curr_month-1])
				long_date++;
			else{
				if(curr_month <= 11)
				  long_date=(year*10000)+((curr_month+1)*100)+1;
				else
			          long_date = ((year+1)*10000) + (1*100) +1; 
			}
		}
		if(long_date >= s_sth.s_date)
			break;
	}

	return(NOERROR);
}	/* get_day() */
/*-------------------- E n d   O f   P r o g r a m ---------------------*/
