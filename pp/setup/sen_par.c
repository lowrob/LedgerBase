/*-----------------------------------------------------------------------
Source Name: sen_par.c
System     : Personnel/Payroll.
Created  On: April 2, 1992.
Created  By: Eugene Roy

DESCRIPTION:

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
------------------------------------------------------------------------*/
#define	MAIN		/* Main program. This is to declare Switches */
#define MAINFL		SEN_PAR	/* main file used */

#define	SYSTEM		"Setup"	/* Sub System Name */
#define	MOD_DATE	"02-APRIL-92"		/* Progran Last Modified */

#include <stdio.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_pp.h>
#include <bfs_com.h>

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
#define	WEEKEND		'W'
#define	HOLIDAYS	'H'
#define	REMOVE		'R'
#define	LINEEDIT	'L'
#define SCREENEDIT	'S'
#define	CANCEL		'C'

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
#define	WEEKEND		'W'
#define	HOLIDAYS	'H'
#define	REMOVE		'R'
#define	LINEEDIT	'L'
#define SCREENEDIT	'S'
#define	CANCEL		'A'

#endif

/* PROFOM Releted declarations */

#define	SCR_NAME	"sen_par"	/* PROFOM screen Name */

/* PROFOM Screen STH file */

/* Field PROFOM numbers */
#define	FN_FLD		400	/* Fn: */
#define	KEY_START	500	/* Bargaining Unit Code: */
#define	KEY_END		600	/* Effective Date: */
#define	CHG_FLD		700	/* Field: */

#define	MAX_DY_FLD	900
#define	BARG_FLD	1100
#define	MTH_ST		1500
#define	MTH_END		5100
#define START_FLD	900	/* First Field on screen */
#define	DATE_FLD	5200
#define	DATE2_FLD	5300
#define	END_FLD		5500	/* Last Field of the screen */

#define	MAX_FIELD	3
/* sen_par.sth - header for C structure generated by PROFOM EDITOR */

typedef struct	{
	char	s_month[4];
	char	s_month_day[32];
	double	s_poss_days;

}	S_item;

typedef struct	{

	char	s_pgname[11];	/* 100 STRING X(10) */
	long	s_rundate;	/* 300 DATE YYYYFMMFDD */
	char	s_fn[2];	/* 500 STRING X */
	char	s_pos[7];	/* 600 STRING X(6) */
	long	s_eff_date;		/* 700 DATE YYYYFMMFDD */
	short	s_field;	/* 800 NUMERIC 99 */

	double	s_max_dy;	/* 1700 NUMERIC 999.99 */
	double	s_num_hd;	/* 1800 NUMERIC 999.99 */
	char	s_barg[7];
	char	s_bargdesc[31];

	char	s_dum1[2];
	char	s_dum2[4];

	S_item	s_items[12];

	long	s_date1;
	long	s_date2;

	char	s_mesg[78];	/* 1900 STRING X(76) */
	char	s_resp[2];	/* 2000 STRING X */

} s_struct;

static	s_struct  s_sth,image;	/* PROFOM Screen Structure */
static	struct  stat_rec  sr;		/* PROFOM status rec */

static	char 	e_mesg[100];  		/* dbh will return err msg in this */

static	Barg_unit	barg;
static	Stat	stat_rec;
static	Pay_param	pay_param;
static	Position	position;
static	Sen_par	sen_par, pre_sen_par;

short	d_month[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

char	month[12][4] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG",
			"SEP", "OCT", "NOV", "DEC" };
int	start_mth;
#define	JAN	01
#define	JUL	07
#define	DEC	12

/* PROFOM Field Numbers */

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
	s_sth.s_field = HV_SHORT ;
	s_sth.s_mesg[0] = HV_CHAR ;
	s_sth.s_resp[0] = HV_CHAR ;

	/* Get Pay_param File		*/

 	err = get_pay_param(&pay_param,BROWSE,1,e_mesg);
	if(err < 0) {
  		DispError((char *)&s_sth,e_mesg);
		return(err);
	}

	/* Initialize Key Fields. So that, if user selectes 'N' option
	   immediately after invoking program, then he gets the first
	   record in the file */

	s_sth.s_pos[0] = '\0';
	s_sth.s_eff_date = 0 ;

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
	fomer("A(dd), C(hange), D(elete), I(nquire), N(ext), P(rev), E(xit)");
#else
	fomer("R(ajouter), C(hanger), E(liminer), I(nterroger), S(uivant), P(rec), F(in)");
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
		return( Add() ) ;
	case CHANGE  :			/* CHANGE */
		CHKACC(retval,UPDATE,e_mesg);
		return( Change() ) ;
	case DELETE  :			/* CHANGE */
		CHKACC(retval,UPDATE,e_mesg);
		return( Delete() ) ;
	case INQUIRE  :			/* Inquire */
		CHKACC(retval,BROWSE,e_mesg);
		return( Inquire() ) ;
	case NEXT  :			/* Next */
		CHKACC(retval,BROWSE,e_mesg);
		return( Next(FORWARD) ) ;
	case PREV  :			/* Previous */
		CHKACC(retval,BROWSE,e_mesg);
		return( Next(BACKWARD) ) ;
	case EXITOPT  :			/* Exit */
		return( QUIT ) ;
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
	err = ShowScreen(ADD);
	if(err != NOERROR) return(err) ;

	err = GetDetails() ;
	if(PROFOM_ERR == err || DBH_ERR == err) return(err) ;
	if(err < 0 || CANCEL == err) {
		roll_back(e_mesg) ;	/* Unlock the locked Records */
		return(ClearScreen()) ;	/* Clear the Screen */
	}

	return(NOERROR);
}	/* Add() */
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
/* Delete. Student Study Hall Records.  */
/*-----------------------------------------------------------------------*/
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
Inquire()
{
	int	err ;

	err = SelectRecord() ;
	if(NOERROR != err) return(err) ;

	return(NOERROR) ;
}	/* Inquire() */
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
ReadScreen(mode)
int	mode;
{
	int err;

	scpy((char *)&image,(char *)&s_sth,sizeof(image));

	if(mode == ADD) {
		SetDupBuffers(MAX_DY_FLD,END_FLD-200,0); /* Off Dup Control */
	}
	else {
		SetDupBuffers(MAX_DY_FLD,BARG_FLD,1); /* Off Dup Control */
	}
	
	s_sth.s_max_dy = LV_DOUBLE;
	s_sth.s_num_hd = LV_DOUBLE;
	s_sth.s_barg[0] = LV_CHAR;

	err = ReadFields((char *)&s_sth,MAX_DY_FLD,END_FLD-200,
		Validation, WindowHelp,1) ;
	if(PROFOM_ERR == err || DBH_ERR == err) return(err) ;
	if(RET_USER_ESC == err){
		if(mode == ADD) {
			InitFields(HV_CHAR,HV_LONG,HV_DOUBLE);
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
/*----------------------------------------------------------------------*/
/* Get the Student Study Hall key from user. In ADD mode disable dup buffers, */
/* other modes enable dup buffers and show the current key as a default key */
ReadKey()
{
	int	i;
	char	hold_code[7];
	long	hold_date;
	
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

	strcpy(hold_code,s_sth.s_pos);
	hold_date = s_sth.s_eff_date;

	s_sth.s_pos[0] = LV_CHAR;
	s_sth.s_eff_date = LV_LONG;

	i = ReadFields((char *)&s_sth,KEY_START, KEY_END,
		Validation, WindowHelp,1) ;
	if(PROFOM_ERR == i || DBH_ERR == i) return(i) ;
	if(RET_USER_ESC == i){
		strcpy(s_sth.s_pos,hold_code);
		s_sth.s_eff_date = hold_date;
		
		ret( WriteFields((char *)&s_sth,KEY_START, KEY_END) ) ;

		s_sth.s_mesg[0] = HV_CHAR;
		DispMesgFld((char *)&s_sth);

		return(RET_USER_ESC) ;
	}

	s_sth.s_mesg[0] = HV_CHAR;
	DispMesgFld((char *)&s_sth);

	return(NOERROR);
}	/*  ReadKey() */
/*----------------------------------------------------------*/
/* Show the next or previous Students Study halls */

Next(direction)
int	direction ;
{
	int err;

	/* Contrary to most Next/Previous programs a flg_reset() must */
	/* always be done here because to show the assignment information */
	/* it must also read this file so the record pointer must be reset */

	strcpy(sen_par.sn_position,s_sth.s_pos);
	sen_par.sn_eff_date = s_sth.s_eff_date ;
	if (direction == FORWARD) {
		sen_par.sn_eff_date++;
	}
	else {
		sen_par.sn_eff_date--;
	}
	flg_reset(SEN_PAR);

	err = get_n_sen_par(&sen_par, BROWSE, 0, direction, e_mesg);
#ifndef ORACLE
	seq_over(SEN_PAR);
#endif
	if(ERROR == err)return(DBH_ERR) ;
	if(EFL == err) {
		fomen("No More Records....");
		get();
		return(NOERROR) ;
	}

	err = ShowScreen(BROWSE);

	return( err ) ;
}	/* Next() */
/*--------------------------------------------------------------------
 ShowAtt()     - this routine reads the attendance information and   
   		 copies it onto the screen
--------------------------------------------------------------------*/
static
ShowAtt()
{
	int 	retval, idex,j, curr_day, curr_month, last_month;
	long	attdate,temp_date;
	curr_month = 1;

	/* initialize screen */
	Disp_Mth();

	for (idex=0; idex < 12; idex++) {
		if((start_mth + idex )> 12)
			j = idex - start_mth+1 ;
		else
			j = idex + start_mth-1;
		strcpy(s_sth.s_items[idex].s_month, month[j]);
		strncpy(s_sth.s_items[idex].s_month_day, 
		"............................... ",d_month[j]);
		s_sth.s_items[idex].s_month_day[31] = '\0'; /* eugene */
		s_sth.s_items[idex].s_poss_days = d_month[j];
	}

	return(NOERROR);
}
/*-----------------------------------------------------------*/
Disp_Mth()
{
	int	retval;

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
static int
AddDate()
{
	int	retval, i;
	int	day, month;
	int	day2, month2;

     	for (; ;) {
		s_sth.s_date1 = LV_LONG;
		s_sth.s_date2 = LV_LONG;

		retval = ReadFields((char *)&s_sth,DATE_FLD,DATE2_FLD,
			Validation, WindowHelp,1) ;

		if (retval < 0) return(-1);
		if (retval == RET_USER_ESC) break;	/* User enter ESC-F */

		day = s_sth.s_date1 % 100;	
		day2 = s_sth.s_date2 % 100;	
		month = (s_sth.s_date1 / 100)%100;
		month2 = (s_sth.s_date2 / 100)%100;
		if(month < month2){
		  for(i=day; i<=d_month[month-1]; i++){
		    if(month >= start_mth){
		    if((s_sth.s_items[month-start_mth].s_month_day[i-1]!='W')&&
			s_sth.s_items[month-start_mth].s_month_day[i-1]!='H'){
/*			s_sth.s_items[month-start_mth].s_poss_days --;*/
		     }
		     s_sth.s_items[month-start_mth].s_month_day[i-1] =
					 HOLIDAYS;
		    }
		    else{
		   if((s_sth.s_items[month+start_mth-2].s_month_day[i-1]!='W')&&
			s_sth.s_items[month+start_mth-2].s_month_day[i-1]!='H'){
/*			s_sth.s_items[month+start_mth-2].s_poss_days --;*/
		     }
		     s_sth.s_items[month+start_mth-2].s_month_day[i-1] =
					 HOLIDAYS;
		    }
		  }
		  for(i=1; i<=day2; i++){
		    if(month2 >= start_mth){
		    if((s_sth.s_items[month2-start_mth].s_month_day[i-1]!='W')&&
			s_sth.s_items[month2-start_mth].s_month_day[i-1]!='H'){
/*			s_sth.s_items[month2-start_mth].s_poss_days --;*/
		     }
		     s_sth.s_items[month2-start_mth].s_month_day[i-1] =
					 HOLIDAYS;
		    }
		    else{
		  if((s_sth.s_items[month2+start_mth-2].s_month_day[i-1]!='W')&&
		      s_sth.s_items[month2+start_mth-2].s_month_day[i-1]!='H'){
/*			s_sth.s_items[month2+start_mth-2].s_poss_days --;*/
		     }
		     s_sth.s_items[month2+start_mth-2].s_month_day[i-1] =
					 HOLIDAYS;
		    }
		  }
		}
		else{
		  for(i=day; i<=day2; i++){
		    if(month >= start_mth){
		    if((s_sth.s_items[month-start_mth].s_month_day[i-1]!='W')&&
		      s_sth.s_items[month-start_mth].s_month_day[i-1]!='H'){
/*			s_sth.s_items[month-start_mth].s_poss_days --;*/
		     }
		     s_sth.s_items[month-start_mth].s_month_day[i-1] =
					 HOLIDAYS;
		    }
		    else{
		   if((s_sth.s_items[month+start_mth-2].s_month_day[i-1]!='W')&&
		      s_sth.s_items[month+start_mth-2].s_month_day[i-1]!='H'){
/*			s_sth.s_items[month+start_mth-2].s_poss_days --;*/
		     }
		     s_sth.s_items[month+start_mth-2].s_month_day[i-1] =
					 HOLIDAYS;
		    }
		  }
		}
	}
	s_sth.s_date1 = HV_LONG;
	s_sth.s_date2 = HV_LONG;

	ret( WriteFields((char *)&s_sth,MTH_ST, (END_FLD - 200)) );

	return(NOERROR);
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
	int	month;
	int	year;
	int	i;

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

	for(i=0;i<12;i++) {
		s_sth.s_items[i].s_poss_days = 0;
	}

	for( ; ; ){

		julian = days(long_date);
		remain = julian % 7;

		day = long_date % 100;	
		month = (long_date / 100)%100;
		year = long_date / 10000;

		if(remain == 6 || remain == 0){
		  if(month >= start_mth){
		     if(s_sth.s_items[month-start_mth].s_month_day[day-1]!='H'){
/*			s_sth.s_items[month-start_mth].s_poss_days --;*/
		     }
		     s_sth.s_items[month-start_mth].s_month_day[day-1] =
					 WEEKEND;
		  }
		  else{
		   if(s_sth.s_items[month+start_mth-2].s_month_day[day-1]!='H'){
/*			s_sth.s_items[month+start_mth-2].s_poss_days --;*/
		   }
		   s_sth.s_items[month+start_mth-2].s_month_day[day-1] =
					 WEEKEND;
		  }
		}
		else {
		  if(month >= start_mth){
			s_sth.s_items[month-start_mth].s_poss_days ++;
		  }
		  else {
			s_sth.s_items[month+start_mth-2].s_poss_days ++;
		  }
		}
		if((day++) < d_month[month-1])
			long_date ++;
		else{
			if(month <= 11)
			  long_date = (year*10000) + ((month+1)*100) +1; 
			else
			  long_date = ((year+1)*10000) + (1*100) +1; 
		}
		
		if(long_date >= end_date)
			break;

	}
	ret( WriteFields((char *)&s_sth,MTH_ST, (END_FLD - 200)) );

	return(NOERROR);
}	/* get_day() */
/*----------------------------------------------------------------*/
Remove()
{
	int	retval, i;
	int	month, day;
	int	month2, day2;

     	for (; ;) {
		s_sth.s_date1 = LV_LONG;
		s_sth.s_date2 = LV_LONG;

		retval = ReadFields((char *)&s_sth,DATE_FLD,DATE2_FLD,
			Validation, WindowHelp,1) ;

		if (retval < 0) return(-1);
		if (retval == RET_USER_ESC) break;	/* User enter ESC-F */

		day = s_sth.s_date1 % 100;	
		day2 = s_sth.s_date2 % 100;	
		month = (s_sth.s_date1 / 100)%100;
		month2 = (s_sth.s_date2 / 100)%100;
		if(month < month2){
		  for(i=day; i<=d_month[month-1]; i++){
		    if(month >= start_mth){
		     if(s_sth.s_items[month-start_mth].s_month_day[i-1] != 'W'){
			s_sth.s_items[month-start_mth].s_month_day[i-1] =
					 '.';
/*			s_sth.s_items[month-start_mth].s_poss_days ++;*/
		     }
		    }
		    else{
		     if(s_sth.s_items[month+start_mth-2].s_month_day[i-1]!='W'){
			s_sth.s_items[month+start_mth-2].s_month_day[i-1] =
					 '.';
/*			s_sth.s_items[month+start_mth-2].s_poss_days ++;*/
		     }
		    }
		  }
		  for(i=1; i<=day2; i++){
		    if(month2 >= start_mth){
		      if(s_sth.s_items[month2-start_mth].s_month_day[i-1]!='W'){
			s_sth.s_items[month2-start_mth].s_month_day[i-1] =
					 '.';
/*			s_sth.s_items[month2-start_mth].s_poss_days ++;*/
		      }
		    }
		    else{
		      if(s_sth.s_items[month2+start_mth-2].s_month_day[i-1] !=
					 'W'){
			s_sth.s_items[month2+start_mth-2].s_month_day[i-1] =
					 '.';
/*			s_sth.s_items[month2+start_mth-2].s_poss_days ++;*/
		      }
		    }
		  }
		}
		else{
		  for(i=day; i<=day2; i++){
		    if(month >= start_mth){
		     if(s_sth.s_items[month-start_mth].s_month_day[i-1]!='W'){
			s_sth.s_items[month-start_mth].s_month_day[i-1] =
					 '.';
/*			s_sth.s_items[month-start_mth].s_poss_days ++;*/
		     }
		    }
		    else{
		     if(s_sth.s_items[month+start_mth-2].s_month_day[i-1]!='W'){
			s_sth.s_items[month+start_mth-2].s_month_day[i-1] =
					 '.';
/*			s_sth.s_items[month+start_mth-2].s_poss_days ++;*/
		     }
		    }
		  }
		}
	}
	s_sth.s_date1 = HV_LONG;
	s_sth.s_date2 = HV_LONG;

	ret( WriteFields((char *)&s_sth,MTH_ST, (END_FLD - 200)) );

	return(NOERROR);
}
/*----------------------------------------------------------------*/
Get_Hol()
{
	int	retval, day, month;
	
	strcpy(stat_rec.s_code,s_sth.s_barg);
	stat_rec.s_date = 0 ;
	
	flg_reset(STAT_HOL);

	for( ; ; ) {
		retval = get_n_stat(&stat_rec,BROWSE,0,FORWARD,e_mesg);
		if(retval < 0) {
			if(retval == EFL) break;
			DispError((char *)&s_sth,e_mesg);
			return(retval);
		}

		if(strcmp(stat_rec.s_code,s_sth.s_barg)!=0) { 
			break;
		}
		
		day = stat_rec.s_date % 100;	
		month = (stat_rec.s_date / 100)%100;

		if(month >= start_mth){
		   if(s_sth.s_items[month-start_mth].s_month_day[day-1]!='H'&&
		      s_sth.s_items[month-start_mth].s_month_day[day-1]!='W'){
/*			s_sth.s_items[month-start_mth].s_poss_days --;*/
		   }
		   s_sth.s_items[month-start_mth].s_month_day[day-1] =
					 HOLIDAYS;
		}
		else{
		   if(s_sth.s_items[month+start_mth-2].s_month_day[day-1]!='H'&&
		      s_sth.s_items[month+start_mth-2].s_month_day[day-1]!='W'){
/*			s_sth.s_items[month+start_mth-2].s_poss_days --;*/
		   }
		   s_sth.s_items[month+start_mth-2].s_month_day[day-1] =
					 HOLIDAYS;
		}
	}
	seq_over(STAT_HOL);

	ret( WriteFields((char *)&s_sth,MTH_ST, (END_FLD - 200)) );

	return(NOERROR);
}
/*-----------------------------------------------------------------------*/ 
/* Check to see if record is to be added, changed or deleted .		 */
/*-----------------------------------------------------------------------*/
WriteRecords(mode)
int mode;
{
	int	i, retval;

	strcpy(sen_par.sn_position,s_sth.s_pos);
	sen_par.sn_eff_date = s_sth.s_eff_date;

	if(mode != ADD){
		retval = get_sen_par(&sen_par,UPDATE,0,e_mesg);
		if(retval < 0) {
		  fomer(e_mesg);
		  return(ERROR);
		}
	}

	sen_par.sn_max_days_yr = s_sth.s_max_dy;
	sen_par.sn_num_hrs_day = s_sth.s_num_hd;
	strcpy(sen_par.sn_barg, s_sth.s_barg);

	for(i=0; i< 12; i++){
	  strcpy(sen_par.sn_month[i], s_sth.s_items[i].s_month_day);
	  sen_par.sn_poss_days[i] = s_sth.s_items[i].s_poss_days;
	}

	retval = put_sen_par(&sen_par,mode,e_mesg);
	if(retval < 0) {
		DispError((char *)&s_sth,e_mesg);
		roll_back(e_mesg);
		return(retval);
	}

	if(mode != ADD) {
		retval = rite_audit((char*)&s_sth,SEN_PAR,mode,(char*)&sen_par,
			(char*)&pre_sen_par,e_mesg);
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

Validation()
{
	int	save_nextfld, save_endfld ;
	int	retval;

	switch(sr.curfld){
	case KEY_START:
		Right_Justify_Numeric(s_sth.s_pos,
			sizeof(s_sth.s_pos)-1);
		strcpy(position.p_code,s_sth.s_pos);
		flg_reset(POSITION);

		retval = get_position(&position,BROWSE,0,e_mesg);
		if(retval < 0) {
			fomer("Position Code Does Not Exist-Please Re-Enter.");
			s_sth.s_pos[0] = LV_CHAR;
			return(ERROR);
		}
		break;
	case KEY_END:
		if(s_sth.s_eff_date == 0) {
			fomer("Invalid Date. Please Re-Enter.");
			s_sth.s_eff_date = LV_LONG;
			return(ERROR);
		}

		strcpy(sen_par.sn_position,s_sth.s_pos);
		sen_par.sn_eff_date = s_sth.s_eff_date;

		retval = get_sen_par(&sen_par,BROWSE,0,e_mesg);
		if(s_sth.s_fn[0] != ADDREC) {
			if(retval < 0) {
			  fomer("Not a Date for Seniority Parameter File");
			  s_sth.s_eff_date = LV_LONG;
			  return(ERROR);
			}
		}
		else {
			if(retval != UNDEF) {
				fomer("Date Already Exists for Seniority Parameter File");
				s_sth.s_eff_date = LV_LONG;
				return(ERROR);
			}
			/*************
			Check for leap year add 10000 because Feb
			falls in the next year after the effective date 
			**************/
			if(((s_sth.s_eff_date + 10000) / 10000) % 4 == 0)
				d_month[1] = 29;
			else
				d_month[1] = 28; /* Reset to orig value */

		}
		break;
	case BARG_FLD:
		Right_Justify_Numeric(s_sth.s_barg,
			sizeof(s_sth.s_barg)-1);
		strcpy(barg.b_code,s_sth.s_barg);
		barg.b_date = get_date();
		flg_reset(BARG);

		retval = get_n_barg(&barg,BROWSE,0,BACKWARD,e_mesg);
		if((retval < 0 ||
		  strcmp(barg.b_code,s_sth.s_barg) != 0)){
		  fomer("Bargaining Unit Code Does Not Exist - Please Re-enter");
		  s_sth.s_barg[0] = LV_CHAR;
		  return(ERROR);
		}
	 	strcpy(s_sth.s_bargdesc,barg.b_name);
		if ( WriteFields((char *)&s_sth,BARG_FLD, BARG_FLD+100) < 0 ) 
				return(-1);
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
	short	reccod ;
	long	effective_date;

	effective_date = get_date();

	switch(sr.curfld){
	case KEY_START:
		retval = position_hlp(s_sth.s_pos,7,13);
		if(retval == DBH_ERR) return(retval);
		if(retval >= 0) redraw();
		if(retval == 0) return(ERROR);
		if(retval < 0) return(ERROR);
		strcpy(position.p_code,s_sth.s_pos);

		retval = get_position(&position,BROWSE,0,e_mesg);
		if(retval < 0)  {
			fomer(e_mesg);
			s_sth.s_pos[0] = LV_CHAR;
			return(ERROR);
		}
		break;
	case BARG_FLD:
		retval = barg_hlp(s_sth.s_barg,&effective_date,7,13);
		if(retval == DBH_ERR) return(retval);
		if(retval >= 0) redraw();
		if(retval == 0) return(ERROR);
		if(retval < 0) return(ERROR);
		strcpy(barg.b_code,s_sth.s_barg);
		barg.b_date = effective_date;
		flg_reset(BARG);

		retval = get_n_barg(&barg,BROWSE,0,BACKWARD,e_mesg);
		if((retval < 0 ||
		  strcmp(barg.b_code,s_sth.s_barg) != 0)){
		  fomer("Bargaining Unit Code Does Not Exist - Please Re-enter");
		  s_sth.s_barg[0] = LV_CHAR;
		  return(ERROR);
		}
	 	strcpy(s_sth.s_bargdesc,barg.b_name);
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
		"Y(es), A(dd), W(eekends), H(olidays), S(creen), L(ine), R(emove), C(ancel)","YAWHSLRC");

#else
		err = GetOption((char *)&s_sth,
		"O(ui), S(creen edit), L(ine edit), A(nnul)"
		,"ORSLA");
#endif
		break ;
	    case  CHANGE :		/* Change */
#ifdef ENGLISH
		err = GetOption((char *)&s_sth,
		"Y(es), A(dd), W(eekends), H(olidays), S(creen), L(ine), R(emove), C(ancel)","YAWHSLRC");
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
	    case  ADDREC  :
		err = AddDate();
		break;
	    case  WEEKEND   :
		err = Get_day();
		break;
	    case  HOLIDAYS  :
		err = Get_Hol();
		break;
	    case  SCREENEDIT:
		err = ReadScreen(UPDATE);
		break;
	    case  LINEEDIT  :
		err = ChangeFields();
		break ;
	    case  REMOVE :
		err = Remove();
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
/*-----------------------------------------------------------*/
ChangeFields()
{
	int	retval ;
	int	fld_no;

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
		if(s_sth.s_field > 0)

		fld_no = START_FLD + (100 * (s_sth.s_field-1));
		retval = ReadFields((char *)&s_sth,fld_no, fld_no,
			Validation, WindowHelp,1) ;
		if(PROFOM_ERR == retval || DBH_ERR == retval) return(retval) ;
		if(RET_USER_ESC == retval){
		/* When user gives ESC-F while changing fields, assumption is
		* he completed his changes, and remaining fields are same as
		* old. But, at this point STH will be having low values in the 
		* remaining fields. So move the old values form the linked list.
		*/
			retval = CopyBack((char *)&s_sth,(char *)&image,
				sr.curfld, END_FLD);
			if(retval == PROFOM_ERR) return(retval);

     			s_sth.s_field = HV_SHORT ;
			if ( WriteFields((char *)&s_sth,CHG_FLD, CHG_FLD) < 0 ) 
				return(-1);

		     	s_sth.s_mesg[0] = HV_CHAR ;
		     	DispMesgFld((char *)&s_sth);
			return(RET_USER_ESC) ;
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
ShowScreen(mode)
int	mode;
{
	int	i, j, retval;
	
	if(mode != ADD){
		strcpy(s_sth.s_pos,sen_par.sn_position);
		s_sth.s_eff_date = sen_par.sn_eff_date;

		s_sth.s_max_dy = sen_par.sn_max_days_yr;
		s_sth.s_num_hd = sen_par.sn_num_hrs_day;
		strcpy(s_sth.s_barg, sen_par.sn_barg);
		strcpy(barg.b_code,s_sth.s_barg);
		barg.b_date = get_date();
		flg_reset(BARG);

		retval = get_n_barg(&barg,BROWSE,0,BACKWARD,e_mesg);
		if((retval < 0 ||
		  strcmp(barg.b_code,s_sth.s_barg) != 0)){
		  fomer("Bargaining Unit Code Does Not Exist - Please Re-enter");
		  s_sth.s_barg[0] = LV_CHAR;
		  return(ERROR);
		}
	 	strcpy(s_sth.s_bargdesc,barg.b_name);

		Disp_Mth();
		for(i=0; i< 12; i++){
		  if((start_mth + i )> 12)
			j = i - start_mth+1 ;
		  else
			j = i + start_mth-1;
		  strcpy(s_sth.s_items[i].s_month, month[j]);
		  strcpy(s_sth.s_items[i].s_month_day, sen_par.sn_month[i]);
		  s_sth.s_items[i].s_poss_days = sen_par.sn_poss_days[i];
		}
		s_sth.s_date1 = HV_LONG;
		s_sth.s_date2 = HV_LONG;
	}
	else
		ShowAtt();

	s_sth.s_dum1[0] = ' ';
	s_sth.s_dum2[0] = ' ';

	ret( WriteFields((char *)&s_sth, KEY_START, END_FLD - 200) ) ;

	return(NOERROR) ;
}	/* ShowScreen() */
/*------------------------------------------------------------------------*/
/* Move High values to all data fields and clear the screen */

ClearScreen()
{
	int	i;

	/* Move High Values to Hedaer part */
	InitFields(HV_CHAR,HV_LONG,HV_DOUBLE) ;

	ret( WriteFields((char *)&s_sth,START_FLD, (END_FLD - 200)) );

	return(NOERROR);
}	/* ClearScreen() */
/*-------------------------------------------------------------------------*/
/* Initialize Screen Header with either Low or High Values */

InitFields( t_char, t_long, t_double)
char	t_char ;
long	t_long;
double	t_double ;
{
	int	i,  j;

	s_sth.s_max_dy = t_double;
	s_sth.s_num_hd = t_double;
	s_sth.s_barg[0] = t_char;
	s_sth.s_bargdesc[0] = HV_CHAR;

	if(t_char == HV_CHAR){
		s_sth.s_dum1[0] = t_char;
		s_sth.s_dum2[0] = t_char;
	}
	else{
		s_sth.s_dum1[0] = ' ';
		s_sth.s_dum2[0] = ' ';
	}

	for(i=0; i<12; i++){
		s_sth.s_items[i].s_month[0] = t_char;

		if(t_char != HV_CHAR){
		  for(j=0; j< d_month[i]; j++){
			s_sth.s_items[i].s_month_day[j] = t_char;
		  }
		}
		else{
		  for(j=0; j< 32; j++)
			s_sth.s_items[i].s_month_day[j] = t_char;
	 	}
		s_sth.s_items[i].s_poss_days = t_double;

	}
	s_sth.s_date1 = t_long;
	s_sth.s_date2 = t_long;

	return(NOERROR) ;
}	/* InitHdr() */
/*----------------------------------------------------------*/
/* Get the key and show the record */
static int
SelectRecord()
{
	int	err ;

	for( ;; ) {
		err = ReadKey();
		if(err != NOERROR) return(err) ;

		strcpy(sen_par.sn_position,s_sth.s_pos);
		sen_par.sn_eff_date = s_sth.s_eff_date;

		err = get_sen_par(&sen_par,BROWSE,0,e_mesg);
		if(err < 0)  {
			fomen(e_mesg);
			get();
			if(err == EFL) continue;
			return(err);
		}
	
		break;
	}

	ShowScreen(UPDATE);

	return(NOERROR);
}	/* SelectRecord() */
/*-------------------- E n d   O f   P r o g r a m ---------------------*/
