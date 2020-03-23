/*------------------------------------------------------------------------
Source Name: t4_sel.c
System     : Personnel/Payroll System.
Created  On: Sept 24th, 1993.
Created  By: Andre Cormier

DESCRIPTION:
	Program to mass select employee's to receive T4's.

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Louis R.       94/01/12		Added code to calculate box 40 to function
				ProcT4 as well as added functions
				CalcBox40() and CalcYTD. 
Louis R.       95/02/20		Changed the calculation for box 40 so that
			the amnount up to july is put through a calculation
			if the salary is greater than 12500 for half the year
			and the remainder is just added on.
Louis R.       94/01/19		Added code to calculate box 52 called CalcBox52
	The formulas were supplied by the R.C. School board for the year 93.
	These formulas will change. The current formulas for the PSPP and
	MPPP are coded for two conditions. Their is one formala for Jan., Feb.,
	and March and another formula for the rest of the year for both
	insurances (MPPP & PSPP). This is due to the government changeing its
	contributition percentage at the end of March.

------------------------------------------------------------------------*/

#define	MAIN		/* Main program. This is to declare Switches */

#define	SYSTEM		"T4"	/* Sub System Name */
#define	MOD_DATE	"24-SEPT-93"		/* Program Last Modified */

#include <stdio.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_com.h>
#include <bfs_pp.h>

#define EXIT	   	12

#define	T_BENIFITS	12500	/* Taxable benifits for half a year */
#define	INSURANCE	"GRPLF" /* The code for group life insurance */
#define APRIL		19940400 /* Used for box 52 calc. 1st of April */
#define JULY		19940700 /* Calculation on box 40 */

/* User Interface define constants */
#ifdef ENGLISH
#define SELECT		'S'
#define EXITOPT		'E'

#define	YES		'Y'
#define NO		'N'
#define	EDIT		'E'
#define	CANCEL		'C'
#else
#define SELECT		'C'
#define EXITOPT		'F'

#define	YES		'O'
#define NO		'N'
#define	EDIT		'M'

#define	CANCEL		'A'
#endif

/* PROFOM Releted declarations */

#define	SCR_NAME	"t4_sel"	/* PROFOM screen Name */

/* Field PROFOM numbers */
#define START_FLD 	400	/* Start Field in range */
#define	END_FLD		1200	/* Last Field of the screen */

#define BARG1		400	/* Employee Number */
#define BARG2		500	/* Employee Number */
#define POS1		600	/* Employee Number */
#define POS2		700	/* Employee Number */
#define EMP1		800	/* Employee Number */
#define EMP2		900	/* Employee Number */
#define OPTION		1000	/* Option response field */
#define MESSAGE		1100	/* Message Field */
#define RESPONSE	1200	/* Response Field */

/* roe_sel.sth - header for C structure generated by PROFOM EDITOR */

typedef struct	{

	char	s_pgm[11];	/* 100 program name */
	long	s_rundate;	/* 300 run date */
	char	s_barg1[7];	/* 400 employee # */
	char	s_barg2[7];	/* 400 employee # */
	char	s_pos1[7];	/* 400 employee # */
	char	s_pos2[7];	/* 400 employee # */
	char	s_emp1[13];	/* 400 employee # */
	char	s_emp2[13];	/* 400 employee # */
	char	s_option[2];	/* 2000 option choice */
	char	s_mesg[78];	/* 2100 message field */
	char	s_resp[2];	/* 2200 response field */

	} S_STRUCT;


S_STRUCT	s_sth;	/* PROFOM Screen Structure */
static	struct  stat_rec  sr;		/* PROFOM status rec */

/* File structures */
static	T4_adj		t4_adj;
static	Pay_param	pay_param;
static	Emp		emp_rec;
static	Emp_earn	emp_earn;
static	Emp_ins		emp_ins;
static	Emp_dh		emp_dd_his;
static	Class		class;
static	Emp_ded		empded;
static	Gov_param	gov_par;
static	Reg_pen		reg_pen;

static	char 	e_mesg[100];  		/* dbh will return err msg in this */

extern	double	D_Roundoff();

static	double	gross_earnings;
static	double	cpp_cont;
static	double	uic_prem;
static	double	income_tax;
static	double	cpp_pen;
static	double	uic_ins_earn;
static	double	union_dues;
static	double	calc_dedytd;
static	double	afterjul_calc_dedytd; /* used to track grplf after JULY*/

static	Validation();
static	WindowHelp();

main(argc,argv)
int argc;
char *argv[];
{
	int 	retval;

	retval = Initialize(argc,argv);	/* Initialization routine */

	if (retval == NOERROR) retval = Process();

	CloseRtn();			/* return to menu */
	if (retval != NOERROR) exit(1);
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

	proc_switch(argc, argv) ; 	/* Process Switches */

	/*
	*	Initialize PROFOM & Screen
	*/
	STRCPY(sr.termnm,terminal);	/* Copy Terminal Name */
	fomin(&sr);
	ret(err_chk(&sr)) ;		/* Check for PROFOM Error */
	fomcf(1,1);			/* Enable Snap screen option */

	err = InitScreen() ;		/* Initialize Screen */
	if(NOERROR != err) return(err) ;

	return(NOERROR) ;
}	/* Initialize() */

/*--------------------------------------------------------------------------*/
/* Close necessary files and environment before exiting program               */

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
	/* move screen name to Profom status structure */
	STRCPY(sr.scrnam,NFM_PATH);
	strcat(sr.scrnam,SCR_NAME) ;

	STRCPY(s_sth.s_pgm,PROG_NAME);
	s_sth.s_rundate = get_date();	/* get Today's Date in YYYYMMDD format*/
	s_sth.s_option[0] = HV_CHAR;
	s_sth.s_mesg[0] = HV_CHAR ;
	s_sth.s_resp[0] = HV_CHAR ;

	/* Move Low Values to data fields */
	InitFields() ;

	return(NOERROR) ;
}	/* InitScreen() */

/*-------------------------------------------------------------------*/
/* Get Option from user and call corresponding function */

Process()
{
	int	err;

	for( ; ; ){

		if((err = ReadOption())<0) 
			return(err);

		switch(s_sth.s_option[0]) {
		case  EXITOPT :
			return(NOERROR);
		case  SELECT :
			CHKACC(err,ADD,e_mesg);
			err = ProcOption() ;
			break ;
		default :
			continue;
		}

		if(NOACCESS == err)	fomen(e_mesg);
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
/*------------------------------------------------------------*/
ReadOption()
{

	s_sth.s_mesg[0] = HV_CHAR;
	DispMesgFld((char *)&s_sth);	
#ifdef ENGLISH
	fomer("S(elect), E(xit)");
#else
	fomer("C(hoisir), F(in)");
#endif
	sr.nextfld = OPTION;
	fomrf((char *)&s_sth);
	ret(err_chk(&sr));

}	/* ReadOption */
/*------------------------------------------------------------*/
ProcOption()
{
	int	i, err ;

	for(i = START_FLD ; i <= END_FLD - 200 ; i += 100)
		fomca1(i, 19, 0) ;    /* disable dup control */

	err = get_pay_param(&pay_param,BROWSE, 1, e_mesg) ;
	if(err < 0) {
		DispError((char *)&s_sth,e_mesg);
		return(ERROR) ;
	}

	err = ReadRange(ADD) ;
	if(err != NOERROR) return(err) ;

	err = Confirm() ;
	if(err != YES) return(NOERROR) ;

	err = ProcT4Sel() ;

	return(NOERROR);
}	/* ProcSelection() */
/*------------------------------------------------------------*/
/* Get the Header details from user */

ReadRange(mode)
int	mode ;
{
	int	 i, err ;

	if(mode == ADD) {
#ifdef ENGLISH
		STRCPY(s_sth.s_mesg,"Press ESC-F to Go to Option:");
#else
		STRCPY(s_sth.s_mesg,"Appuyer sur ESC-F pour retourner a Option:");
#endif
		DispMesgFld((char *)&s_sth);

		fomca1(BARG1, 19, 1) ;
		fomca1(BARG2, 19, 1) ;
		fomca1(POS1, 19, 1);
		fomca1(POS2, 19, 1) ;
		fomca1(EMP1, 19, 1) ;
		fomca1(EMP2, 19, 1) ;
		strcpy(s_sth.s_barg1, "     0");
		strcpy(s_sth.s_barg2, "ZZZZZZ");
		strcpy(s_sth.s_pos1, "     0");
		strcpy(s_sth.s_pos2, "ZZZZZZ");
		strcpy(s_sth.s_emp1, "           0");
		strcpy(s_sth.s_emp2, "ZZZZZZZZZZZZ");
		sr.nextfld = BARG1 ;
		sr.endfld = END_FLD - 300 ;
		fomud((char*)&s_sth);
		ret(err_chk(&sr));
	}
	InitFields() ;

	i = ReadFields((char *)&s_sth,START_FLD, END_FLD - 200,
			Validation, WindowHelp, 1) ;
	if(PROFOM_ERR == i || DBH_ERR == i) return(i) ;
	if(RET_USER_ESC == i) {	/* ESC-F */
		return(RET_USER_ESC) ;
	}

	return(NOERROR) ;
}	/* ReadRange() */
/*----------------------------------------------------------------*/
/* Validation function() for Key and Header fields when PROFOM returns
  RET_VAL_CHK */

static
Validation()
{
	int	retval;


	switch(sr.curfld){
	case BARG1:  /* starting bargaining unit code */
		Right_Justify_Numeric(s_sth.s_barg1
					,(sizeof(s_sth.s_barg1)-1));
		break;
	case BARG2:  /* ending bargaining unit code */
		Right_Justify_Numeric(s_sth.s_barg2
					,(sizeof(s_sth.s_barg2)-1));
		if(strcmp(s_sth.s_barg2,s_sth.s_barg1) <0) {
#ifdef ENGLISH
			fomer("Ending code cannot precede starting code");
#else
			fomer("Code finissant ne peut pas preceder le code debutant");
#endif
			s_sth.s_barg2[0] = LV_CHAR;
		}
		break;
	case POS1:  
		Right_Justify_Numeric(s_sth.s_pos1
					,(sizeof(s_sth.s_pos1)-1));
		break;
	case POS2: 
		Right_Justify_Numeric(s_sth.s_pos2
					,(sizeof(s_sth.s_pos2)-1));
		if(strcmp(s_sth.s_pos2,s_sth.s_pos1) <0) {
#ifdef ENGLISH
			fomer("Ending code cannot precede starting code");
#else
			fomer("Code finissant ne peut pas preceder le code debutant");
#endif
			s_sth.s_pos2[0] = LV_CHAR;
		}
		break;
	case EMP1:
		Right_Justify_Numeric(s_sth.s_emp1
					,(sizeof(emp_rec.em_numb)-1));
		break;
	case EMP2:  /* ending employee code */
		Right_Justify_Numeric(s_sth.s_emp2
					,(sizeof(emp_rec.em_numb)-1));
		if(strcmp(s_sth.s_emp2,s_sth.s_emp1) <0) {
#ifdef ENGLISH
			fomer("Ending number cannot precede starting number");
#else
			fomer("Numero finissant ne peut pas preceder le numero debutant");
#endif
			s_sth.s_emp2[0] = LV_CHAR;
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
/* Show Help Windows, if applicable, when user gives ESC-H in key
   and header fields */

static
WindowHelp()
{
	int	retval ;
	int	fld_no;


	fomer("No Help Window for This Field");

	return(NOERROR) ;
}	/* HdrAndKeyWindowHelp() */
/*-----------------------------------------------------------------------*/
/* Take the confirmation from user for the header part */

Confirm()
{
	int	err ;

	for( ; ; ) {
#ifdef ENGLISH
		err = GetOption((char *)&s_sth,"Y(es), E(dit), C(ancel)", "YEC");
#else
		err = GetOption((char *)&s_sth,"O(ui), M(odifier), A(nnuler)", "OMA");
#endif
		if(err == PROFOM_ERR) return(err) ;

		switch(err) {
		case  YES :
			return(YES) ;
		case  EDIT  :
			err = FieldEdit();
			break ;
		case  CANCEL :
#ifdef ENGLISH
			err = GetOption((char *)&s_sth,"Confirm the Cancel (Y/N)?", "YN") ;
#else
			err = GetOption((char *)&s_sth,"Confirmer l'annulation (O/N)?", "ON") ;
#endif
			if(err == YES) { 
				roll_back(e_mesg) ;	/* Unlock  Records */
				return(CANCEL) ;
			}
			break ;
		}	/* switch err */

		if(err == PROFOM_ERR) return(err) ;
		if(err == DBH_ERR) return(err) ;
	}	/* for(; ; ) */
}	/* Confirm() */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Changing fields. Accept fld to be changed and read that fld 		 */

FieldEdit()
{
     	int	i,retval;

     	for ( i = START_FLD; i <= END_FLD - 200 ; i += 100 )
       		fomca1( i,19,1 );      		/*  enable Dup Control */

     	sr.nextfld = START_FLD;
     	sr.endfld = END_FLD - 200;
     	fomud( (char *) &s_sth );
     	ret(err_chk(&sr));

	retval = ReadRange(UPDATE);
	if(retval != NOERROR) return(retval) ;

     	return(NOERROR);
}	/* FieldEdit() */
/*-------------------------------------------------------------------------*/
/* Initialize Screen Header with either Low Values */

InitFields()
{
	s_sth.s_barg1[0] = LV_CHAR;
	s_sth.s_barg2[0] = LV_CHAR;
	s_sth.s_pos1[0] = LV_CHAR;
	s_sth.s_pos2[0] = LV_CHAR;
	s_sth.s_emp1[0] = LV_CHAR;
	s_sth.s_emp2[0] = LV_CHAR;
	
	return(NOERROR) ;
}	/* InitFields() */
/*-----------------------------------------------------------------------*/
ProcT4Sel()
{
	int	retval;

	strcpy(emp_rec.em_numb, s_sth.s_emp1);
	flg_reset(EMPLOYEE);

	for(;;) {
		retval = get_n_employee(&emp_rec,BROWSE,0,FORWARD,e_mesg);
		if(retval == EFL)
			break;
		if(retval < 0) {
			DispError((char *)&s_sth,e_mesg);
			return(retval);
		}

		if((strcmp(emp_rec.em_barg,s_sth.s_barg1) < 0) ||
		  (strcmp(emp_rec.em_barg,s_sth.s_barg2) > 0))
		  	continue ;

		if((strcmp(emp_rec.em_pos,s_sth.s_pos1) < 0) ||
		  (strcmp(emp_rec.em_pos,s_sth.s_pos2) > 0))
			continue ;

		if((strcmp(emp_rec.em_numb,s_sth.s_emp1) < 0) ||
		  (strcmp(emp_rec.em_numb,s_sth.s_emp2) > 0))
			break ;

	 	sprintf(e_mesg,"Employee #: %s", emp_rec.em_numb);
		fomen(e_mesg);
		fflush(stdout) ;

		retval = ProcT4();
		if(retval < 0) {
			DispError((char *)&s_sth,e_mesg);
			return(retval);
		}
		inc_str(emp_rec.em_numb,sizeof(emp_rec.em_numb)-1,FORWARD);
		flg_reset(EMPLOYEE);
	}
	return(NOERROR) ;
}
/*--------------------------------------------------------------*/
ProcT4()
{
	int	retval;

	strcpy(t4_adj.ta_numb, emp_rec.em_numb);

	retval = get_t4_adj(&t4_adj,BROWSE,0,e_mesg);
	if(retval < 0 && retval != UNDEF) {
		DispError((char *)&s_sth,e_mesg);
		return(retval);
	}
	if(retval == NOERROR){
		DispError((char *)&s_sth,"Employee Already Selected");
		return(retval);
	}

	retval = GetEarnings();
	if(retval < 0)	return(retval);

	/* If no earnings then get the next employee to process */
	if(gross_earnings <= 0) return(NOERROR);

	t4_adj.ta_emp_inc = D_Roundoff(gross_earnings);
	t4_adj.ta_cpp_cont = D_Roundoff(cpp_cont);
	t4_adj.ta_uic_prem = D_Roundoff(uic_prem);
	t4_adj.ta_tax_ded = D_Roundoff(income_tax);
	t4_adj.ta_cpp_pen_earn = D_Roundoff(cpp_pen);

	retval = GetUICIns();
	if(retval < 0)	return(retval);

	t4_adj.ta_uic_ins_earn = D_Roundoff(uic_ins_earn);

	t4_adj.ta_housing = 0;
	t4_adj.ta_travel = 0;
	t4_adj.ta_auto = 0;
	t4_adj.ta_intrest = 0;
	t4_adj.ta_emp_com = 0;


	retval = GetUnion();
	if(retval < 0)	return(retval);

	t4_adj.ta_union_du = D_Roundoff(union_dues);

	/* get the employees premium of his life insurance for calculation */
	strcpy(empded.ed_numb, emp_rec.em_numb);
	strncpy(empded.ed_code, INSURANCE, sizeof(INSURANCE));
	strncpy(empded.ed_group, INSURANCE, sizeof(INSURANCE));
	flg_reset(EMP_DED);
	retval = get_n_emp_ded(&empded,BROWSE,0,FORWARD, e_mesg);
	if(retval < 0 && retval != EFL){
		DispError((char *)&s_sth,e_mesg);
		return(retval);
	}
	/* If life insurance is found for employee then do calculation */
	if(strcmp(empded.ed_numb, emp_rec.em_numb)==0 &&
	  strncmp(empded.ed_code, INSURANCE, sizeof(INSURANCE))==0){
		retval = CalcBox40();
		if(retval < 0)	return(retval);
		/* Add other tax to the gross income */
		t4_adj.ta_emp_inc += t4_adj.ta_other_tax;
	}
	else
		t4_adj.ta_other_tax = 0; 

/* Nicola - registered pension registration number  */

	if(emp_rec.em_reg_pen[0] == '\0')
		strcpy(t4_adj.ta_reg_pen_num,emp_rec.em_reg_pen);
	else{
		strcpy(reg_pen.rg_code, emp_rec.em_reg_pen);
		strcpy(reg_pen.rg_pp_code, emp_rec.em_pp_code);
		retval = get_reg_pen(&reg_pen,BROWSE,0,e_mesg);
		if(retval < 0) {
			fomer("Registered pension plan Does not Exist");
			return(ERROR);
		}

		strcpy(t4_adj.ta_reg_pen_num, reg_pen.rg_reg_num);
	}

	/* Pension adjustment, box 52 */
	if(t4_adj.ta_uic_ins_earn > 0){
		retval = CalcBox52();
		if(retval < 0)	return(retval);
	}

	retval = put_t4_adj(&t4_adj,ADD,0,e_mesg);
	if(retval < 0) {
		DispError((char *)&s_sth,e_mesg);
		return(retval);
	}
	seq_over(EMP_DED);

	return(NOERROR);
}

/*--------------------------------------------------------------*/
CalcBox40()
{
	int	retval;
	long	th_salery;	/* the salery figure rounded up to thousands. */
	double	tmp_sal; /* field used to hold thousands num of salery */
	long	rem_sal; /* Remaining hundreds part of salery */

	/* Set the other taxes Box 40 */
	strcpy(class.c_code,emp_rec.em_class);
	class.c_date = get_date();
	flg_reset(CLASSIFICATION); 
	retval = get_n_class(&class,BROWSE,0,BACKWARD, e_mesg);
	if(retval < 0 && retval != EFL){
		DispError((char *)&s_sth,e_mesg);
		return(retval);
	}

	if(strcmp(class.c_code, emp_rec.em_class) == 0){/*found correct class*/
		if(class.c_yrly_inc == 0 || /* No yearly income set in file */
		  class.c_yrly_inc/2 <= T_BENIFITS){ /* Is income high enough */
			t4_adj.ta_other_tax = 0; 
			retval=CalcYTD();
			if(retval<0) return(retval);
		}
		else { /* Use formula to determain other_tax */
			tmp_sal = class.c_yrly_inc / 1000;
			th_salery = (long)tmp_sal;
			if((tmp_sal - th_salery) * 1000 == 0)
				/* number is in thousands */
				th_salery = class.c_yrly_inc;
			else{
				th_salery = (th_salery + 1) * 1000;
			}
			/* Do the calculation on only half the salery */
			th_salery = th_salery / 2;
			retval=CalcYTD();
			if(retval<0) return(retval);
			t4_adj.ta_other_tax =
			  (th_salery - T_BENIFITS) * calc_dedytd / th_salery;
			  /*((th_salery - T_BENIFITS)/th_salery)*calc_dedytd;*/
		}
		t4_adj.ta_other_tax += afterjul_calc_dedytd;
		t4_adj.ta_other_tax = D_Roundoff(t4_adj.ta_other_tax); 
	}
	else {
		sprintf(e_mesg, 
			"No class code found for emp: %s  ", emp_rec.em_numb);
		DispError((char *)&s_sth,e_mesg);
		t4_adj.ta_other_tax = 0; 
	}
	seq_over(CLASSIFICATION); 
	return(NOERROR);
}
/*-----------------------------------------------------------------------*/
/* calculate the YTD amount						 */
static
CalcYTD()
{
	int retval;
	
	calc_dedytd = 0;
	afterjul_calc_dedytd = 0;
	strcpy(emp_dd_his.edh_numb,emp_rec.em_numb);
	strcpy(emp_dd_his.edh_code,empded.ed_code);
	strcpy(emp_dd_his.edh_group,empded.ed_group);
	emp_dd_his.edh_date = 0;
	flg_reset(EMP_DED_HIS);

	for(;;){
		retval = get_n_emp_dhis(&emp_dd_his,BROWSE,1,FORWARD,e_mesg);
		if(retval == EFL) break;
		
		if(retval < 0){
			DispError((char *)&s_sth,e_mesg);
			return(retval);
		}

		if(strcmp(emp_dd_his.edh_numb,emp_rec.em_numb)!=0) break;
		if(strcmp(emp_dd_his.edh_code,empded.ed_code)!=0) break;
		if(strcmp(emp_dd_his.edh_group,empded.ed_group)!=0) break;

		if(emp_dd_his.edh_date < pay_param.pr_cal_st_dt) continue;
		if(emp_dd_his.edh_date > pay_param.pr_cal_end_dt) continue;


		if(emp_dd_his.edh_date < JULY)
			calc_dedytd += emp_dd_his.edh_amount;
		else
			afterjul_calc_dedytd += emp_dd_his.edh_amount;
	}
	seq_over(EMP_DED_HIS);
	
	return(NOERROR);
}
/*--------------------------------------------------------------*/
GetEarnings()
{
	int	retval;

	gross_earnings = 0;
	cpp_cont = 0;
	uic_prem = 0;
	income_tax = 0;
	cpp_pen = 0;

	strcpy(emp_earn.en_numb,emp_rec.em_numb);
	emp_earn.en_date = pay_param.pr_cal_st_dt;
	emp_earn.en_pp = 0;
	emp_earn.en_week = 0;
	flg_reset(EMP_EARN) ;

	for( ; ; ) {
		retval = get_n_emp_earn(&emp_earn,BROWSE,0,FORWARD,e_mesg);
		if(retval < 0) {
			if(retval == EFL) break;
			DispError((char *)&s_sth,e_mesg);
			return(retval);
		}
		if(strcmp(emp_earn.en_numb,emp_rec.em_numb)!=0) { 
			break;
		}

		if(emp_earn.en_date < pay_param.pr_cal_st_dt || 
		   emp_earn.en_date > pay_param.pr_cal_end_dt)
			break;

		gross_earnings += emp_earn.en_reg_inc + emp_earn.en_high_inc;

		cpp_cont += emp_earn.en_cpp;

		uic_prem += emp_earn.en_uic;

		income_tax += emp_earn.en_tax;

		cpp_pen += emp_earn.en_cpp_pen;

	}
	seq_over(EMP_EARN);


	return(NOERROR);
}
/*--------------------------------------------------------------*/
GetUICIns()
{
	int	retval;

	uic_ins_earn = 0;

	strcpy(emp_ins.in_numb,emp_rec.em_numb);
	emp_ins.in_pp = 0;
	emp_ins.in_date = 0;

	flg_reset(EMP_INS) ;

	for( ; ; ) {
		retval = get_n_emp_ins(&emp_ins,BROWSE,0,FORWARD,e_mesg) ;
		if( retval == EFL ||
			strcmp(emp_ins.in_numb,emp_rec.em_numb) != 0)
			break;

		if(emp_ins.in_date < pay_param.pr_cal_st_dt ||
		   emp_ins.in_date > pay_param.pr_cal_end_dt)
			continue;

		if( retval < 0) {
			roll_back(e_mesg);
			DispError((char *)&s_sth,e_mesg) ;
			return(retval) ;
		}
		uic_ins_earn += emp_ins.in_uic_ins;
	}
	seq_over(EMP_INS);



	return(NOERROR);
}
/*--------------------------------------------------------------*/
GetUnion()
{
	int	retval;


	union_dues = 0;

	strcpy(emp_dd_his.edh_numb,emp_rec.em_numb);
	strcpy(emp_dd_his.edh_code,"UDUES");
	strcpy(emp_dd_his.edh_group,"UDUES");
	emp_dd_his.edh_date = 0;

	flg_reset(EMP_DED_HIS) ;

	for( ; ; ) {
		retval = get_n_emp_dhis(&emp_dd_his,BROWSE,1,FORWARD,e_mesg) ;
		if( retval == EFL ||
			strcmp(emp_dd_his.edh_numb,emp_rec.em_numb) != 0)
			break;

		if(strcmp(emp_dd_his.edh_code,"UDUES") != 0)
			break;

		if(strcmp(emp_dd_his.edh_group,"UDUES") != 0)
			break;

		if(emp_dd_his.edh_date < pay_param.pr_cal_st_dt ||
		   emp_dd_his.edh_date > pay_param.pr_cal_end_dt)
			continue;

		if( retval < 0) {
			roll_back(e_mesg);
			DispError((char *)&s_sth,e_mesg) ;
			return(retval) ;
		}
		union_dues += emp_dd_his.edh_amount;
	}
	seq_over(EMP_DED_HIS);

	strcpy(emp_dd_his.edh_numb,emp_rec.em_numb);
	strcpy(emp_dd_his.edh_code,"NFEES");
	strcpy(emp_dd_his.edh_group,"NFEES");
	emp_dd_his.edh_date = 0;

	flg_reset(EMP_DED_HIS) ;

	for( ; ; ) {
		retval = get_n_emp_dhis(&emp_dd_his,BROWSE,1,FORWARD,e_mesg) ;
		if( retval == EFL ||
			strcmp(emp_dd_his.edh_numb,emp_rec.em_numb) != 0)
			break;

		if(strcmp(emp_dd_his.edh_code,"NFEES") != 0)
			break;

		if(strcmp(emp_dd_his.edh_group,"NFEES") != 0)
			break;

		if(emp_dd_his.edh_date < pay_param.pr_cal_st_dt ||
		   emp_dd_his.edh_date > pay_param.pr_cal_end_dt)
			continue;

		if( retval < 0) {
			roll_back(e_mesg);
			DispError((char *)&s_sth,e_mesg) ;
			return(retval) ;
		}
		union_dues += emp_dd_his.edh_amount;
	}
	seq_over(EMP_DED_HIS);
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
/* This function is customized. There is a calculation for those employees
   that belong to the PSPP pension plan, and two calculations for
   the MPPP pension plan. One calculation for Jan. Feb. Mar. and a different
   calculation for the rest of the year due to a change in government payments
*/
CalcBox52()
{
	int	retval;
	int	first_pass;	/* Flag for the first time into routine */
	int	tot_months1;	/* Months of PSPP contributions before April*/
	int	tot_months2;	/* Months of PSPP contributions after March*/
	long	pres_month;	/* present month of payment */
	double	pen_diff;	/* amount of pen salary over gov. allouance */
	double	mppp1 = 0;	/* MPPP calculation prior to March */
	double	mppp2 = 0;	/* MPPP calculation after & including March */
	double	mppp_pa = 0;	/* MPPP pension adjustment */
	double	pspp_pa = 0;	/* PSPP pension adjustment */

	/* Round to thousands. ie 1993/00/00 */
	gov_par.gp_eff_date = (pay_param.pr_cal_st_dt/10000) * 10000;
	flg_reset(GOV_PARAM);

	retval = get_n_gov_param(&gov_par,BROWSE, 0, FORWARD, e_mesg) ;
	if(retval == EFL) {
		fomen("Government Parameter Record Not Setup");
		return(retval);
	}
	if(retval < 0) {
		fomer(e_mesg) ;
		return(ERROR) ;
	}

	tot_months1 = 0;
	tot_months2 = 0;

	strcpy(emp_earn.en_numb,emp_rec.em_numb);
	emp_earn.en_date = pay_param.pr_cal_st_dt;
	emp_earn.en_pp = 0;
	emp_earn.en_week = 0;
	flg_reset(EMP_EARN) ;
	first_pass = 0;

	for( ; ; ) {
		retval = get_n_emp_earn(&emp_earn,BROWSE,0,FORWARD,e_mesg);
		if(retval < 0) {
			if(retval == EFL) break;
			DispError((char *)&s_sth,e_mesg);
			return(retval);
		}
		if(strcmp(emp_earn.en_numb,emp_rec.em_numb)!=0) { 
			break;
		}

		if(emp_earn.en_date < pay_param.pr_cal_st_dt || 
		   emp_earn.en_date > pay_param.pr_cal_end_dt)
			break;

		if(strcmp("PSPP", emp_earn.en_reg_pen) == 0){
			if(first_pass == 0){
				first_pass = 1;
				pres_month = 0; 
			}
			if(emp_earn.en_date < APRIL){
				/* Calulate number of months before April */
				if(pres_month != (emp_earn.en_date/100)){
					pres_month=(emp_earn.en_date/100);
					tot_months1++;
				}
			}
			else if(emp_earn.en_date < 19940000) {
				/* Calulate number of months after March */
				if(pres_month != (emp_earn.en_date/100)){
					pres_month=(long)(emp_earn.en_date/100);
					tot_months2++;
				}
			}
		}
		else if(strcmp("MPPP", emp_earn.en_reg_pen) ==0){
			/* Total all the fields for payment into pension */
			if(emp_earn.en_date < APRIL)
				mppp1+=emp_earn.en_cpp_pen;
			else 
				mppp2+=emp_earn.en_cpp_pen;
		}
	}
	/* Amount of pensionable salery over gov allouance YMPE */
	pen_diff = cpp_pen - gov_par.gp_cpp_pen_earn;
	if(pen_diff < 0) /* Don't allow a negative number */
		pen_diff = 0;
	if(tot_months1>0){
		if(cpp_pen > gov_par.gp_cpp_pen_earn) {
			pspp_pa = (.014 * gov_par.gp_cpp_pen_earn) + 
				  (.02 * pen_diff);
		}
		else {
			pspp_pa = (.014 * cpp_pen / (tot_months1 + 
				  tot_months2) * 12) + (.02 * pen_diff);
		}
		pspp_pa = (double)tot_months1 / 12 * pspp_pa ;
	}
	if(tot_months2>0){
		if(cpp_pen > gov_par.gp_cpp_pen_earn) {
			pspp_pa += ((.005 * gov_par.gp_cpp_pen_earn) + 
				   (.011 * pen_diff)) * (double)tot_months2/12;
		}
		else {
			pspp_pa += (((.005 * cpp_pen / (tot_months1 +
				   tot_months2) * 12) + (.011 * pen_diff)) *
				   (double)tot_months2 / 12);
		}
	}
	if(pspp_pa > 0)
		pspp_pa = (9 * pspp_pa) - ((double)1000/(double)12*(double)(tot_months1+tot_months2));
	/****
	   For mppp pen adj, before april employer & employee payed 5% 
	   after that the employer still payed 5% but the employee payed .5%
	****/

	if(mppp1>0)
		mppp_pa = mppp1 * .05 + mppp1 * .005;
	if(mppp2>0)
		mppp_pa += mppp2 * .05 + mppp2 * .0395;

	t4_adj.ta_pen_adj = pspp_pa + mppp_pa;

	seq_over(EMP_EARN);
	return(NOERROR);
}
