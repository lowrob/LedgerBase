/*-----------------------------------------------------------------------
Source Name: payadv .c
System     : Personnel/Payroll.
Created  On: 9th Nov 91.
Created  By: Andre Cormier

DESCRIPTION: This program does the maintenance of the pay advance  
  	     cheques (add and delete advance)

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
------------------------------------------------------------------------*/
#define	MAIN		/* Main program. This is to declare Switches */
#define MAINFL		MAN_CHQ		/* main file used */

#define	SYSTEM		"Setup"			/* Sub System Name */
#define	MOD_DATE	"8-NOV-91"		/* Progran Last Modified */

#include <stdio.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_pp.h>
#include <bfs_com.h>

/* User Interface define constants */
#ifdef ENGLISH
#define ADVANCE		'P'
#define DELETE		'D'
#define INQUIRE		'I'
#define NEXT		'N'
#define EXITOPT		'E'

#define EDIT		'E'
#define YES		'Y'
#define CANCEL		'C'

#else
#define ADVANCE		'P'
#define DELETE		'D'
#define INQUIRE		'I'
#define NEXT		'N'
#define EXITOPT		'E'

#define EDIT		'E'
#define YES		'Y'
#define CANCEL		'C'

#endif

/* PROFOM Releted declarations */

#define	SCR_NAME	"payadv"	/* PROFOM screen Name */

/* PROFOM Screen STH file */

/* Field PROFOM numbers */

#define FN_FLD		400	/*  Function : */

#define EMPLOYEE_FLD	500	/*  Description: */
#define FUND_FLD 	600	/*  fund */
#define AR_ACCT_FLD	700
#define PAY_DATE_FLD 	800
#define	ADJ_DATE_FLD	900	/*  Amt Flag P or F */
#define EARN_FLD	1000	/*  T4 field */
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
/* payadv.sth - header for C structure generated by PROFOM EDITOR */

typedef struct	{

	char	s_pgname[11];		/* 100 STRING X(10) */
	long	s_rundate;		/* 300 DATE YYYYFMMFDD */
	char	s_fn[2];		/* 400 STRING X */

	char	s_emp_num[13];		/* 500 STRING X(12) */
	short	s_fund_num;		/* 600 NUMERIC 999 */
	char	s_ar_acct[19];		/* 700 STRING X(18) */
	long	s_pay_date;		/* 800 NUMERIC 9(8) */
	long	s_adj_date;		/* 900 DATE YYYYFMMFDD X */
	char	s_earn[7];		/* 1000 STRING X(6) */
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

static	Gl_rec		gl_rec;
static	Man_chq		man_chq,	pre_man_chq;
static	Jr_ent		jr_ent,		pre_jr_ent;
static	Emp		emp;
static	Ctl_rec		ctl_rec;
static	Pa_rec		pa_rec;
static	Chq_hist	chq_hist;
static 	Barg_unit	barg_unit;
static	Earn		earnings;
static	Time		time;
static	Emp_sched1	emp_sched1;
static	Pay_per_it	pay_per_it;
static	Class		class;
static	Position	position;

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
	fomer("P(ay Advance), D(elete Pay Advance), N(ext), I(nquire), E(xit)");
#else
	fomer("P(ay Advance), D(elete Pay Advance), N(ext), I(nquire), E(xit)");
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
	case ADVANCE  :			/* ADD */
		CHKACC(retval,ADD,e_mesg);
		return( Manual() ) ;
	case DELETE  :			/* CHANGE */
		CHKACC(retval,UPDATE,e_mesg);
		return( Delete() ) ;
	case INQUIRE  :
		CHKACC(retval,INQUIRE,e_mesg);
		return( Inquire() ) ;
	case NEXT  :
		CHKACC(retval,INQUIRE,e_mesg);
		return( Next() ) ;
	case EXITOPT  :			/* Exit */
		return( QUIT ) ;
	}  /*   end of the switch statement */

	return(retval);

}	/* ProcFunction() */

/*----------------------------------------------------------------------*/
/* Adding pay advance cheques.   */
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
/* Delete pay advance cheques.   */
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
/* Inquire on a given record */
Inquire()
{
	int	retval;

	retval = ReadKey();
	if(retval != NOERROR) return(retval) ;

	strcpy(man_chq.mc_emp_numb,s_sth.s_emp_num);
	man_chq.mc_date = 0;
	man_chq.mc_chq_numb = 0;
	flg_reset(MAN_CHQ);

	retval = get_n_man_chq(&man_chq,BROWSE,0,FORWARD,e_mesg);
	if(retval == EFL ||strcmp(man_chq.mc_emp_numb, s_sth.s_emp_num) !=0){
		DispError((char *)&s_sth,
			"Advance Does Not Exist for That Employee");
		ClearScreen();
		return(NOERROR); 
	}
	if(retval < 0) {
		DispError((char *)&s_sth,e_mesg);
		return(retval);
	}
	retval = CopyToScreen();
	if(retval<0) return(retval);
	return(NOERROR);
}
/*-----------------------------------------------------------------------*/
Next()
{
	int	retval;

	if(s_sth.s_emp_num[0] == HV_CHAR || s_sth.s_emp_num[0] == LV_CHAR){
		man_chq.mc_emp_numb[0] = '\0';
		man_chq.mc_date = 0;
		man_chq.mc_chq_numb = 0;
	}
	else{
		strcpy(man_chq.mc_emp_numb,s_sth.s_emp_num);
		if(s_sth.s_pay_date == HV_LONG){
			man_chq.mc_date = 0;
			man_chq.mc_chq_numb = 0;
		}
		else{
			man_chq.mc_date = s_sth.s_pay_date;
			man_chq.mc_chq_numb = s_sth.s_adj_date + 1;
		}
	}
	flg_reset(MAN_CHQ);

	retval = get_n_man_chq(&man_chq,BROWSE,0,FORWARD,e_mesg);
	if(retval == EFL) {
		DispError((char *)&s_sth,"No More Recods");
		return(NOERROR); 
	}
	if(retval < 0) {
		DispError((char *)&s_sth,e_mesg);
		return(retval);
	}
	strcpy(emp.em_numb,man_chq.mc_emp_numb);
	retval = get_employee(&emp,BROWSE,0,e_mesg);
	if(retval < 0){
		DispError((char *)&s_sth,e_mesg);
		return(retval);
	}

	strcpy(s_sth.s_emp_num, man_chq.mc_emp_numb);
	CopyEmpDemo();
	retval = CopyToScreen();
	if(retval<0) return(retval);
}

/*-----------------------------------------------------------------------*/
CopyToScreen()
{
	s_sth.s_fund_num = man_chq.mc_fund;
	strcpy(s_sth.s_ar_acct, man_chq.mc_acct);
	s_sth.s_adj_date = man_chq.mc_chq_numb;
	s_sth.s_pay_date = man_chq.mc_date;
	strcpy(s_sth.s_earn, man_chq.mc_ded_code);
	s_sth.s_amount = man_chq.mc_amount;
	ret(WriteFields((char *)&s_sth,EMPLOYEE_FLD,AMOUNT_FLD));
	return(NOERROR);
}
	
/*-----------------------------------------------------------------------*/
/* Process to delete pay advance cheques  */
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

	s_sth.s_pay_date = LV_LONG;
	s_sth.s_amount = HV_DOUBLE;
	s_sth.s_fund_num = LV_SHORT;
	s_sth.s_ar_acct[0] = LV_CHAR;
	s_sth.s_pay_date = LV_LONG;
	s_sth.s_adj_date = LV_LONG;
	s_sth.s_earn[0] = LV_CHAR;

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
	/*s_sth.s_pay_date = get_date();*/
	SetDupBuffers(PAY_DATE_FLD,PAY_DATE_FLD,1); /* Off Dup Control */

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
	case  ADVANCE :		/* Add Manual */

		s_sth.s_fund_num = LV_SHORT;
		s_sth.s_ar_acct[0] = LV_CHAR;
		s_sth.s_pay_date = LV_LONG;
		s_sth.s_adj_date = LV_LONG;
		s_sth.s_earn[0] = LV_CHAR;
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
/* Check to see if advance cheque is to be added or deleted .		 */
WriteManual(mode)
int mode;
{
	int	retval, j;
	short	seq_no;

	scpy((char *)&pre_man_chq,(char *)&man_chq,sizeof(man_chq));

	strcpy(man_chq.mc_emp_numb,s_sth.s_emp_num);
	man_chq.mc_date = s_sth.s_pay_date;
	man_chq.mc_fund = s_sth.s_fund_num;
	strcpy(man_chq.mc_acct,s_sth.s_ar_acct);
	man_chq.mc_chq_numb = s_sth.s_adj_date;
	strcpy(man_chq.mc_ded_code,s_sth.s_earn);
	man_chq.mc_amount = s_sth.s_amount;

	if(mode != ADD) {
		retval = get_man_chq(&man_chq,UPDATE,0,e_mesg) ;
		if(retval < 0 || 
		(strcmp(s_sth.s_emp_num,man_chq.mc_emp_numb) != 0) ||
		s_sth.s_fund_num != man_chq.mc_fund ||
		(strcmp(s_sth.s_ar_acct,man_chq.mc_acct) != 0) ||
		(strcmp(s_sth.s_earn,man_chq.mc_ded_code) != 0)) {
			fomen("No Pay Advance For That Employee");
			get();
			roll_back(e_mesg);
			return(retval);
		}
	}
	seq_no = 0;
	strcpy(barg_unit.b_code,emp.em_barg);
	barg_unit.b_date = get_date();
	flg_reset(BARG);

	retval = get_n_barg(&barg_unit,BROWSE,0,BACKWARD,e_mesg);
	if(retval == EFL ||
		strcmp(barg_unit.b_code, emp.em_barg) != 0){
  		DispError((char *)&s_sth,"Bargaining Unit does not Exist");
		return(NOERROR);
	}
	if(retval < 0){
  		DispError((char *)&s_sth,e_mesg);
  		return(ERROR);
	}
	seq_over(BARG);

	strcpy(pay_per_it.ppi_code, barg_unit.b_pp_code);
	pay_per_it.ppi_st_date = s_sth.s_adj_date;
	flg_reset(PAY_PER_ITEM);

	retval = get_n_pp_it(&pay_per_it,BROWSE, 1, BACKWARD, e_mesg) ;
	if(retval == EFL ||
      	   strcmp(pay_per_it.ppi_code, barg_unit.b_pp_code) != 0){ 
		DispError((char *)&s_sth,
			"Pay Period Item does not Exist");
		return(NOERROR);
	}
	if(retval < 0){
  		DispError((char *)&s_sth,e_mesg);
  		return(ERROR);
	}
	seq_over(PAY_PER_ITEM);

	if(mode == ADD){
		strcpy(emp_sched1.es_numb,s_sth.s_emp_num);
		emp_sched1.es_week = 0;
		emp_sched1.es_class[0] = '\0';
		emp_sched1.es_fund = 0;
		flg_reset(EMP_SCHED1);

		for( ; ; ) {
			retval = get_n_emp_sched1(&emp_sched1,BROWSE,0,
							FORWARD,e_mesg);
			if(retval < 0) {
				if(retval == EFL) break;
				DispError((char *)&s_sth,e_mesg);
				return(retval);
			}
			if(strcmp(emp_sched1.es_numb,emp.em_numb)!=0) { 
				break;
			}

			GetClass(emp_sched1.es_class);

			GetPos(class.c_pos);

			seq_no ++;
			strcpy(time.tm_numb,emp.em_numb);
			time.tm_date = s_sth.s_adj_date;
			time.tm_no = seq_no;
			time.tm_pp = pay_per_it.ppi_numb;
			time.tm_year = pay_per_it.ppi_year;
			time.tm_week = emp_sched1.es_week;
			time.tm_fund = emp_sched1.es_fund;
			time.tm_adj[0] = '\0';

			strcpy(time.tm_class,emp_sched1.es_class);
			strcpy(time.tm_earn,man_chq.mc_ded_code);

			time.tm_trans[0] = '\0';
			time.tm_exp[0] = '\0';
			time.tm_tot_amt = emp_sched1.es_amount;	

			for(j=0 ; j<7 ; j++) {
		  	  time.tm_units[j] = emp_sched1.es_units[j];
		  	  time.tm_att[j][0] = '\0';
			}

			time.tm_cost = emp_sched1.es_cost;

			time.tm_teach[0] = '\0';
			time.tm_comment[0] = '\0';
			strcpy(time.tm_stat,"ACT");

			for(;;) {
		  	  retval = put_ptime(&time,ADD,e_mesg) ;
		  	  if(retval == DUPE){
			 	time.tm_no++;
				roll_back(e_mesg);
			  	continue;
		  	  }
	  	  	  if(retval < 0) {
				DispError((char *)&s_sth,e_mesg);
				roll_back(e_mesg);
				return(retval);
		  	  }

		  	  retval = commit(e_mesg) ;
		  	  if(retval < 0) {
			    DispError((char*)&s_sth,"ERROR: Saving Records"); 
			    DispError((char *)&s_sth,e_mesg);
			    roll_back(e_mesg);
			    return(retval);
		  	  }
		 	  break;
			}
	        }
		seq_over(EMP_SCHED1);
	}
	else{
	  strcpy(time.tm_numb,s_sth.s_emp_num);
	  time.tm_date = s_sth.s_adj_date;
	  time.tm_no = 0;
	  flg_reset(TIME);

	  for(;;){
		retval = get_n_ptime(&time,UPDATE,0,FORWARD,e_mesg);
		if(retval < 0) {
			if(retval == EFL) break;
			DispError((char *)&s_sth,e_mesg);
			roll_back(e_mesg);
			return(retval);
		}
		if(strcmp(time.tm_numb,emp.em_numb)!=0) { 
			roll_back(e_mesg);
			break;
		}
		if(time.tm_date != s_sth.s_adj_date){
			roll_back(e_mesg);
			continue;
		}

		retval = put_ptime(&time,P_DEL,e_mesg) ;
	  	if(retval < 0) {
			DispError((char *)&s_sth,e_mesg);
			roll_back(e_mesg);
			return(retval);
		}

		retval = commit(e_mesg) ;
		if(retval < 0) {
		    DispError((char*)&s_sth,"ERROR: Saving Records"); 
		    DispError((char *)&s_sth,e_mesg);
		    roll_back(e_mesg);
		    return(retval);
		}
	  }
	  seq_over(TIME);
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
/*--------------------------------------------------------------*/
GetClass(class_rec)
char	*class_rec;
{
	int	retval;

	strcpy(class.c_code,class_rec);
	class.c_date = get_date();
	flg_reset(CLASSIFICATION);

	retval = get_n_class(&class,BROWSE,0,BACKWARD,e_mesg);
	if(retval < 0 && retval != EFL){
		DispError((char *)&s_sth, e_mesg);
		return(ERROR);
	}
	if((strcmp(class.c_code,class_rec) != 0) || retval == EFL){
	    	return(EFL);
	}
	return(NOERROR);
}
/*--------------------------------------------------------------*/
GetPos(pos)
char	*pos;
{
	int	retval;

	strcpy(position.p_code,pos);

	retval = get_position(&position,BROWSE,0,e_mesg);
	if(retval < 0) {
		DispError((char *)&s_sth,e_mesg);
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

		CopyEmpDemo();
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
		strcpy(s_sth.s_ar_acct,ctl_rec.ar_acnt);
		ret(WriteFields((char *)&s_sth,FUND_FLD,AR_ACCT_FLD));
		SetDupBuffers(AR_ACCT_FLD,AR_ACCT_FLD,1); /* Off Dup Control */
		s_sth.s_ar_acct[0] = LV_CHAR;
		break;

	case	AR_ACCT_FLD	:	/* Acct No:	*/
		if (acnt_chk(s_sth.s_ar_acct) < 0)  {
			s_sth.s_ar_acct[0] = LV_CHAR ;
#ifdef ENGLISH
			fomer("Invalid GL Account Number");
#else
			fomer("Numero de compte G/L invalide");
#endif
			return(ERROR) ;
		}
		/* Get G/L account from G/L to see if valid acc. */
		gl_rec.funds = s_sth.s_fund_num ;
		STRCPY(gl_rec.accno, s_sth.s_ar_acct);
		gl_rec.reccod = 99;
 
		retval = get_gl(&gl_rec, BROWSE, 0, e_mesg);

		if(retval == UNDEF){
			fomer("Must be a valid Account Number") ;
			s_sth.s_ar_acct[0] = LV_CHAR ;
			return(ERROR) ;
		}
		if(retval < 0){
			fomer(e_mesg);
			s_sth.s_ar_acct[0] = LV_CHAR ;
			return(ERROR) ;
		}

	/*	if( ((strcmp(s_sth.s_ar_acct, ctl_rec.bank1_acnt)) != 0) &&
		    ((strcmp(s_sth.s_ar_acct, ctl_rec.bank2_acnt)) != 0) ) {
#ifdef ENGLISH
			fomer("Must be a Bank Account Number") ;
#else
			fomer("Doit etre un numero de compte de banque") ;
#endif
			s_sth.s_ar_acct[0] = LV_CHAR ;
			return(ERROR) ;
		} */
			
		break ;

	case	ADJ_DATE_FLD:

		switch (s_sth.s_fn[0]) {
		case ADVANCE:
			strcpy(man_chq.mc_emp_numb,s_sth.s_emp_num);
			man_chq.mc_date = s_sth.s_pay_date;
			man_chq.mc_chq_numb = s_sth.s_adj_date;

			retval = get_man_chq(&man_chq,BROWSE,0,e_mesg);
			if(retval == NOERROR) {
				fomen("Advance Already Exists for That Date, Please Re-enter");
				s_sth.s_adj_date = LV_LONG;
				return(-1); 
			}
			break;
		case DELETE:
			strcpy(man_chq.mc_emp_numb,s_sth.s_emp_num);
			man_chq.mc_date = s_sth.s_pay_date;
			man_chq.mc_chq_numb = s_sth.s_adj_date;

			retval = get_man_chq(&man_chq,BROWSE,0,e_mesg);
			if(retval == UNDEF) {
				fomen("Advance Does Not Exist for That Date, Please Re-enter");
				s_sth.s_adj_date = LV_LONG;
				return(-1); 
			}
			break;
		}

		break;

	case	EARN_FLD:

		Right_Justify_Numeric(s_sth.s_earn, sizeof(s_sth.s_earn)-1);
		strcpy(earnings.ea_code,s_sth.s_earn);
		earnings.ea_date = s_sth.s_rundate;
		flg_reset(EARN);

		retval = get_n_earn(&earnings,BROWSE,1,BACKWARD,e_mesg);
		if((retval < 0 ||
	 	    strcmp(earnings.ea_code,
			s_sth.s_earn) != 0)){
			fomer("Earnings Code Does Not Exist - Please Re-enter");
			s_sth.s_earn[0] = LV_CHAR;
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

}	/* Validation() */

/*----------------------------------------------------------------*/
CopyEmpDemo()
{
		strcpy(s_sth.s_name,emp.em_first_name);
		strcat(s_sth.s_name," ");
		strcat(s_sth.s_name,emp.em_last_name);
		if(emp.em_add1[0] == '\0')
			s_sth.s_mail_adres[0] = HV_CHAR;
		else
			strcpy(s_sth.s_mail_adres,emp.em_add1);
		if(emp.em_add2[0] == '\0')
			s_sth.s_street[0] = HV_CHAR;
		else
			strcpy(s_sth.s_street,emp.em_add2);
		if(emp.em_add3[0] == '\0')
			s_sth.s_city[0] = HV_CHAR;
		else
			strcpy(s_sth.s_city,emp.em_add3);
		if(emp.em_add4[0] == '\0')
			s_sth.s_prov[0] = HV_CHAR;
		else
			strcpy(s_sth.s_prov,emp.em_add4);
		if(emp.em_pc[0] == '\0')
			s_sth.s_postal[0] = HV_CHAR;
		else
			strcpy(s_sth.s_postal,emp.em_pc);

		ret(WriteFields((char *)&s_sth,EMP_FIRST_FLD,POSTAL_FLD));
}
/*----------------------------------------------------------------*/
/* Show Help Windows, if applicable, when user gives ESC-H in key
   and header fields */

WindowHelp()
{
	int	retval ;
	char	temp_pp_code[7];
	short	rec_code;

	temp_pp_code[0] = '\0';
	switch(sr.curfld){

	case EARN_FLD:
		  retval = earn_hlp("\0",s_sth.s_earn,
				7,13);
		  if(retval == DBH_ERR) return(retval);
		  if(retval >= 0) redraw();
		  if(retval == 0) return(ERROR);
		  if(retval < 0) return(ERROR);
		  strcpy(earnings.ea_code,s_sth.s_earn);
		  earnings.ea_date = s_sth.s_rundate;
		  flg_reset(EARN);

		  retval = get_n_earn(&earnings,BROWSE,1,BACKWARD,e_mesg);
		  if((retval < 0 ||
		      strcmp(earnings.ea_code,
				s_sth.s_earn) != 0)){
			fomer("Earnings Code Does Not Exist - Please Re-enter");
			s_sth.s_earn[0] = LV_CHAR;
		    	return(ERROR);
		  }
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

		if(emp.em_add1[0] == '\0')
			s_sth.s_mail_adres[0] = HV_CHAR;
		else
			strcpy(s_sth.s_mail_adres,emp.em_add1);
		if(emp.em_add2[0] == '\0')
			s_sth.s_street[0] = HV_CHAR;
		else
			strcpy(s_sth.s_street,emp.em_add2);
		if(emp.em_add3[0] == '\0')
			s_sth.s_city[0] = HV_CHAR;
		else
			strcpy(s_sth.s_city,emp.em_add3);
		if(emp.em_add4[0] == '\0')
			s_sth.s_prov[0] = HV_CHAR;
		else
			strcpy(s_sth.s_prov,emp.em_add4);
		if(emp.em_pc[0] == '\0')
			s_sth.s_postal[0] = HV_CHAR;
		else
			strcpy(s_sth.s_postal,emp.em_pc);

		ret(WriteFields((char *)&s_sth,EMPLOYEE_FLD,POSTAL_FLD));

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

 
        case    AR_ACCT_FLD:      /* Account# */
 
		rec_code = 99;
                retval = gl_hlp(s_sth.s_fund_num, s_sth.s_ar_acct
			,&rec_code, 7, 13 );
                if(retval == DBH_ERR) return(retval) ;
                if(retval >=0 ) redraw();
                break ;

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
	    case  ADVANCE :		/* Add Manual */
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
	s_sth.s_ar_acct[0] = t_char;	
	s_sth.s_adj_date = t_long;
	s_sth.s_pay_date = t_long;
	s_sth.s_earn[0] = t_char;
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
/* Write the journal entry for the earn code */
/* I have elected to code all pay advance journal entries with a type of M */
/* Since the amount of the advance pay will be taken off of the net pay
   when printing the pay cheque this function is not needed. */
WriteJr(mode)
int	mode;
{
	int	retval;

/*
	if(mode == ADD) {
		jr_ent.jr_fund = s_sth.s_fund_num;
		jr_ent.jr_no = HV_SHORT;
		flg_reset(JR_ENT);

		retval = get_n_jr_ent(&jr_ent,BROWSE,0,BACKWARD,e_mesg);
		if(retval == EFL ||  jr_ent.jr_no < 1 ||
		   s_sth.s_fund_num != jr_ent.jr_fund) {
			jr_ent.jr_fund = s_sth.s_fund_num;
			jr_ent.jr_no = 1;
		}
		else {
			jr_ent.jr_no++;
		}
		strcpy(jr_ent.jr_emp_numb,s_sth.s_emp_num);
		strcpy(jr_ent.jr_code,s_sth.s_earn);
		jr_ent.jr_date = s_sth.s_pay_date;
		strcpy(jr_ent.jr_acct,earnings.ea_code);
		strcpy(jr_ent.jr_type,"M");
		jr_ent.jr_amount = s_sth.s_amount * -1;
		jr_ent.jr_class[0] = '\0';
	}
	if(mode != ADD) {
		strcpy(jr_ent.jr_emp_numb,s_sth.s_emp_num);
		strcpy(jr_ent.jr_code,s_sth.s_earn);
		jr_ent.jr_fund = s_sth.s_fund_num;
		jr_ent.jr_no = HV_SHORT;
		flg_reset(JR_ENT);

		for(;;){
		  retval = get_n_jr_ent(&jr_ent,BROWSE,1,BACKWARD,e_mesg) ;
		  if(retval < 0){
			fomen("No Pay Advance For Selected Employee");
			get();
			roll_back(e_mesg);
			return(retval);
		  }
		  if(strcmp(s_sth.s_emp_num,jr_ent.jr_emp_numb) != 0 ||
		     strcmp(jr_ent.jr_code,s_sth.s_earn)!=0 ||
		     jr_ent.jr_date != s_sth.s_pay_date ||
		     strcmp(jr_ent.jr_acct,s_sth.s_ar_acct)!=0 ||
		     jr_ent.jr_fund != s_sth.s_fund_num ||
		     strcmp(jr_ent.jr_type,"M")!=0)
			continue;

		  retval = get_jr_ent(&jr_ent,UPDATE,0,e_mesg) ;
		  if(retval < 0){
			fomen("No Pay Advance For That Employee");
			get();
			roll_back(e_mesg);
			return(retval);
		  }
		  break;
		}
	}
	seq_over(JR_ENT);

	retval = put_jr_ent(&jr_ent,mode,e_mesg);
	if(retval < 0) {
		DispError((char *)&s_sth,e_mesg);
		roll_back(e_mesg);
		return(retval);
	}

	retval = commit(e_mesg);
	if(retval < 0)
		return(ERROR);
*/
	return(NOERROR);
}

/*-------------------- E n d   O f   P r o g r a m ---------------------*/