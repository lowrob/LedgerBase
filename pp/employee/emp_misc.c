/*----------------------------------------------------------------------------
		Sourcename    : emp_misc.c
		System        : Personel Payroll System.
		Subsystem     : PP 
		Module        : Miscellaneous       
		Created on    : 91-11-14
		Created  By   : Sheldon Floyd

HISTORY:
 Date           Programmer     Description of modification

____/__/__      __________     ___________________________

-----------------------------------------------------------------------------*/
#define MAINFL		EMPLOYEE	/* main file used */

#include <bfs_pp.h>
#include <stdio.h>
#include <reports.h>
#include <cfomstrc.h>
#include <pp_msgs.h>
#include <ctype.h>
#include <empldrvr.h>

#define SYSTEM		"PERSONEL PAYROLL"
#define MOD_DATE	"14-NOV-91"
#define SCREEN_NAME	"misc"

#ifdef ENGLISH

#define INQUIRE		'I'
#define EXITOPT		'E'
#define	YES		'Y'
#define CANCEL		'C'
#define LINEEDIT	'L'
#define SCREDIT		'S'
#define CHGREC  	'C'
#define INQREC  	'I'
#define	NO		'N'
#define AP_SYS		'A'

#else

#define INQUIRE		'I'
#define EXITOPT		'F'
#define	YES		'O'
#define CANCEL		'C'
#define SCREDIT		'S'
#define LINEEDIT	'L'
#define CHGREC  	'C'
#define INQREC  	'I'
#define	NO		'N'
#define AP_SYS		'S'

#endif

#define DATE_FLD	300		/* system date field */
#define	FN_FLD		400		/* function field */
#define EMP_FLD		500		/* employee number field */
#define CHG_FLD		600		/* change field */

#define NAME_FLD	800		/* employee name field */
#define STAT_FLD	1000		/* status field */

#define FT_FLD		1100		/* start FT date field */

#define	PT_FLD		1200		/* start PT date field */
#define	CA_FLD		1300		/* start CA date field */
#define	SU_FLD		1400		/* start SU date field */
#define CONT_FLD	1500		/* continuous date field */
#define APP_FLD		1600		/* appointment date field */
#define ANN_FLD		1700		/* anniversary field */
#define LANG_FLD	1800		/* language preferred field */
#define ROE_FLD		1900		/* last ROE date field */
#define INSWK_FLD	2000		/* no of insurable weeks of last 52 */
#define PHONE_FLD	2100		/* unlisted telephone number field */
#define VACPAY_FLD	2200		/* vacation pay field */
#define CLASS_FLD	2300		/* insurance class field */
#define PAID_FLD	2400		/* pre-paid field */
#define EXPENSE_FLD	2500		/* pre-paid field */

#define	TERMDT_FLD	2600		/* termination date field */
#define TERMCD_FLD	2700		/* termination code field */

#define RUNIT_FLD	2800		/* YTD regular units field */
#define RINC_FLD	2900		/* YTD regular income field */
#define HUNIT_FLD	3000		/* YTD high units field */
#define HINC_FLD	3100		/* YTD high income field */

#define CALYTD_FLD	3200		/* calendar YTD income */
#define SCHYTD_FLD	3300		/* school YTD income field */

#define MSG_FLD		3400		/* Message line   field */
#define RESP_FLD	3500		/* Option response field */

/* misc.sth - header for C structure generated by PROFOM EDITOR */
struct	hi_struct	{
	long	s_rundt;	/* 300 system date */
	char	s_fn[2];	/* 400 function */
	char	s_empcd[13];	/* 500 employee number */
	short	s_field;	/* 600 field number */
	char	s_name[31];	/* 800 employee name */
	double	s_salary;	/* 850	employee's salary */
	char	s_status[4];	/* 900 employee status */
	long	s_stft;		/* 1100 start FT date */
	long	s_stpt;		/* 1200 start PT date */
	long	s_stca;		/* 1300	start CA date */
	long  	s_stsu;		/* 1400 start SU date */
	long   	s_cont;		/* 1500 continuous date */
	long	s_app;		/* 1600 appointment date */
	short	s_ann;		/* 1700 anniversary */
	char	s_lang[2];	/* 1800 language of preference */
	long	s_roe;		/* 1900 last ROE */
	short	s_ins_wk;	/* 2000 number of insurable weeks last 52 */
	char	s_phone[11];	/* 2100 unlisted telephone number */
	double	s_vacpay;	/* 2200 vacation pay percentage */
	char	s_inscl[2];	/* 2300 insurance class */
	char	s_paid[2];	/* 2400 pre-paid Y/N */
	char	s_expense[7];	/* 2500 pre-paid Y/N */
	long	s_term_dt;	/* 2600 termination date */
	char	s_termcd[7];	/* 2700 termination code */
	double	s_runits;	/* 2800 YTD regular units */
	double 	s_rinc;		/* 2900 YTD regular income */
	double	s_hunits;	/* 3000 YTD high units */
	double  s_hinc;		/* 3100 YTD high income */
	double	s_calytd;	/* 3200 calendar YTD income */
	double	s_schytd;	/* 3300 school YTD income */
	char	s_mesg[78];	/* 3400 message line */
	char	s_resp[2];	/* 3500 response line */
};

static struct hi_struct s_sth, image;	/* screen record */
static Emp_earn		empearn;
static Pay_param	param;
static Exp	expense;
static Cert	cert;
static Class 	class;
static Pay_per	pay_per;
static Emp_sched1	emp_sched1;
static Term	termination;
static	Barg_unit	barg_unit;

double	D_Roundoff();

char	chardate[11];
char	projname[50];
char	*arayptr[5];

static int	first_time = 0;
static	int	mode;

static int	Validate();
static int	WindowHelp();

/*---------------------------------------------------------------------------*/
/* Initialize profom fields, call entry procedures */
Miscellaneous()
{
	int retval,err;

	if( (retval=Initialize())<0 )	/* Initialize profom enviroment */
		exit(-1);

	/* Get the parameter file record */
	if( get_pay_param(&param,BROWSE,1,e_mesg)<1 ){
		fomen(e_mesg);
		get();
		return(-1);
	}

	if(emp_rec.em_numb[0] != '\0') {
		strcpy(s_sth.s_empcd,emp_rec.em_numb);
		err = UsrBargVal(BROWSE,emp_rec.em_numb,emp_rec.em_barg,1,e_mesg);
		if(err < 0){
			DispError((char *)&s_sth,e_mesg);
		}
		else{
			retval = DisplayRec();
		}
	}

	retval = Process();	/* Interact with the user */
	err = CleanExit();
	if(err < 0) 
		return(err);
	return(retval);

}    /*  Main()   */
/*---------------------------------------------------------------------------*/
static	int
CleanExit()
{
	/* clear and exit the screen , close files & exit program */
	free_audit();	/* free memory allocated for writing audit rec */
	fomcs();
	fomrt();
	close_dbh();

	return(NOERROR);
}  /* CleanExit  */
/*---------------------------------------------------------------------------*/
static	int
Initialize()
{
	/* initialize the profom status variables */
	strcpy( sr.scrnam, NFM_PATH );
	strcat( sr.scrnam, SCREEN_NAME );

	/* initialize the fields and the profom screen */
	if( FillKeyFields()<0) 		return(-1);
	fomin( &sr );			/* initialize profom */
	fomcf(1,1);			/* Enable snap-screen option */

	return(NOERROR);
}   /*  Initialize()  */
/*---------------------------------------------------------------------------*/
/* Fill the keyfields with high or low values */
static
FillKeyFields()
{
	int	retval;

	s_sth.s_fn[0] = LV_CHAR;
	s_sth.s_rundt = get_date();
	s_sth.s_empcd[0] = LV_CHAR;
	s_sth.s_field = HV_SHORT;

	s_sth.s_name[0] = HV_CHAR;
	s_sth.s_salary = HV_DOUBLE;
	s_sth.s_status[0] = HV_CHAR;
	s_sth.s_stft = HV_LONG;
	s_sth.s_term_dt = HV_LONG;

	s_sth.s_stpt = HV_LONG;
	s_sth.s_termcd[0] = HV_CHAR;
	s_sth.s_stca = HV_LONG;
	s_sth.s_stsu = HV_LONG;

	s_sth.s_cont = HV_LONG;
	s_sth.s_runits = HV_DOUBLE;
	s_sth.s_app = HV_LONG;
	s_sth.s_rinc = HV_DOUBLE;

	s_sth.s_ann = HV_SHORT;
	s_sth.s_hunits = HV_DOUBLE;
	s_sth.s_lang[0] = HV_CHAR;
	s_sth.s_hinc = HV_DOUBLE;

	s_sth.s_roe = HV_LONG;
	s_sth.s_ins_wk = HV_SHORT;
	s_sth.s_calytd = HV_DOUBLE;
	s_sth.s_phone[0] = HV_CHAR;

	s_sth.s_schytd = HV_DOUBLE;
	s_sth.s_vacpay = HV_DOUBLE;
	s_sth.s_inscl[0] = HV_CHAR;
	s_sth.s_paid[0] = HV_CHAR;
	s_sth.s_expense[0] = HV_CHAR;

	s_sth.s_mesg[0] = HV_CHAR;
	s_sth.s_resp[0] = HV_CHAR;

	retval = WriteFields((char *)&s_sth,NAME_FLD,RESP_FLD);
	if(retval < 0) return(retval);

	return(NOERROR);
}   /* FillKeyfields()  */
/*---------------------------------------------------------------------------*/
/* Accept user's option and call the corresponding routine in a loop */
static int
Process()
{
	int retval;

	for( ; ; ){
#ifdef ENGLISH
		fomen("C(hange), I(nquire), F(orward), B(ackward), N(ext), P(rev), S(creen), E(xit)");
#else
		fomen("C(hange), I(nquire), F(orward), B(ackward), N(ext), P(revious), E(xit)");
#endif

		/* Allow user to choose option */
		
		retval = ReadFields((char *)&s_sth,FN_FLD,FN_FLD,Validate,
			WindowHelp,0);
		if(retval < 0) return(retval);

		if(s_sth.s_fn[0] == NEXT_SCR) { 	/* Next Screen */
			Cur_Option += 1 ;	/* Set to Next Screen */
			return( JUMP ) ; 
		}
		if(s_sth.s_fn[0] == PREV_SCR){ 	/* Next Screen */
			Cur_Option -= 1 ;	/* Set to Next Screen */
			return( JUMP ) ; 
		}

		if(s_sth.s_fn[0] == EXITOPT) break;

		if(s_sth.s_fn[0] == CHGREC || s_sth.s_fn[0] == INQREC){
			if(s_sth.s_fn[0] == INQREC)	mode = BROWSE;
			else				mode = UPDATE;
			SetDupBuffers(EMP_FLD,EMP_FLD,1);
			s_sth.s_empcd[0] = LV_CHAR;
			retval = ReadFields((char *)&s_sth,EMP_FLD,EMP_FLD,
				Validate,WindowHelp,1);
			if(retval < 0) return(retval);
			if(retval == RET_USER_ESC) continue;
			SetDupBuffers(EMP_FLD,EMP_FLD,1);
		}
		if(s_sth.s_fn[0] == SCREEN){
			retval = Screen();
			return(retval);
		}
		retval = DisplayRec();
		if(retval < 0) return(retval);

		if(s_sth.s_fn[0] == INQREC){
			retval = InquireRec();
			if(retval < 0) return(retval);
		}

		if(s_sth.s_fn[0] == CHGREC){
			retval = ChangeRec();
			if(retval < 0) return(retval);
		}
	}

	return(NOERROR);
}    /*  Process()   */
/*-----------------------------------------------------------------------*/
/* display suboption line for inquiry and get response		         */
static
InquireRec()
{
	int retval;

	strcpy(s_sth.s_mesg,"Y(es)");
	retval = WriteFields((char *)&s_sth,MSG_FLD,MSG_FLD);
	if(retval < 0) return(retval);

	for(;;){
		s_sth.s_resp[0] = LV_CHAR;
		retval = ReadFields((char *)&s_sth,RESP_FLD,RESP_FLD,Validate,
			WindowHelp,0);
		if(retval < 0) return(retval);

		if(s_sth.s_resp[0] == YES) break;
	}

	s_sth.s_mesg[0] = HV_CHAR;
	s_sth.s_resp[0] = HV_CHAR;
	retval = WriteFields((char *)&s_sth,MSG_FLD,RESP_FLD);
	if(retval < 0) return(retval);

	return(NOERROR);
}
/*-----------------------------------------------------------------------*/
/* display record contents for all options but add			 */
static
DisplayRec()
{
	int	retval;

	if(s_sth.s_fn[0] == NEXT_RECORD){
		if(first_time != 0){
			inc_str(emp_rec.em_numb,sizeof(emp_rec.em_numb)-1,
				FORWARD);
			flg_reset(EMPLOYEE);
		}
		first_time = 1;
		for( ; ; ){
		  retval = get_n_employee(&emp_rec,BROWSE,0,FORWARD,e_mesg);
		  if(retval == EFL){
			fomen(NOMORE);
			get();
			return(NOERROR);
		  }
		  if(retval < 0){
			DispError((char *)&s_sth,e_mesg);
			return(retval);
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
	}

	if(s_sth.s_fn[0] == PREV_RECORD){
		if(first_time != 0){
			inc_str(emp_rec.em_numb,sizeof(emp_rec.em_numb)-1,
				BACKWARD);
			flg_reset(EMPLOYEE);
		}
		first_time = 1;
		for( ; ; ){
		  retval = get_n_employee(&emp_rec,BROWSE,0,FORWARD,e_mesg);
		  if(retval == EFL){
			fomen(NOMORE);
			get();
			return(NOERROR);
		  }
		  if(retval < 0){
			DispError((char *)&s_sth,e_mesg);
			return(retval);
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
	}

	if(s_sth.s_fn[0] != NEXT_RECORD && s_sth.s_fn[0] != PREV_RECORD){
		if(s_sth.s_fn[0] == CHGREC) mode = UPDATE;
		if(s_sth.s_fn[0] == INQUIRE) mode = BROWSE;

		strcpy(emp_rec.em_numb,s_sth.s_empcd);
		retval = get_employee(&emp_rec,BROWSE,0,e_mesg);

		if(retval < 0){
			DispError((char *)&s_sth,e_mesg);
			return(retval);
		}
		retval = UsrBargVal(mode,emp_rec.em_numb,emp_rec.em_barg,1,e_mesg);
		if(retval < 0){
			DispError((char *)&s_sth,e_mesg);
			return(-1);
		}
	}

	strcpy(s_sth.s_empcd,emp_rec.em_numb);
	sprintf(s_sth.s_name,"%s, %s",emp_rec.em_last_name,
				      emp_rec.em_first_name);

	retval = CalcSalary();
	if(retval < 0)	return(retval);
	strcpy(s_sth.s_status,emp_rec.em_status);
	s_sth.s_stft = emp_rec.em_st_dt_ft;
	s_sth.s_stpt = emp_rec.em_st_dt_pt;
	s_sth.s_stca = emp_rec.em_st_dt_ca;
	s_sth.s_stsu = emp_rec.em_st_dt_su;

	s_sth.s_term_dt = emp_rec.em_term_dt;
	strcpy(s_sth.s_termcd,emp_rec.em_term);

	s_sth.s_cont = emp_rec.em_cont_dt;
	s_sth.s_app = emp_rec.em_app_dt;
	s_sth.s_ann = emp_rec.em_ann;
	strcpy(s_sth.s_lang,emp_rec.em_lang);

	s_sth.s_roe = emp_rec.em_last_roe;
	s_sth.s_ins_wk = emp_rec.em_num_ins_wk;
	strcpy(s_sth.s_phone,emp_rec.em_un_tel);
	s_sth.s_vacpay = emp_rec.em_vac_rate;

	if(strcmp(param.pr_prov,"NB")==0){
		strcpy(s_sth.s_inscl,emp_rec.em_ins);
		strcpy(s_sth.s_paid,emp_rec.em_pre_paid);
		s_sth.s_expense[0] = '\0';
	}
	else{
		s_sth.s_inscl[0] = HV_CHAR;
		s_sth.s_paid[0] = HV_CHAR;
		s_sth.s_expense[0] = HV_CHAR;
	}

	retval = CalcAmnts();
	if(retval < 0) return(retval);

	WriteFields((char *)&s_sth,EMP_FLD,SCHYTD_FLD);

	return(NOERROR);
}
/*---------------------------------------------------------------------------*/
/* Calculate amounts and display them on screen				     */
static
CalcAmnts()
{
	int retval;

	s_sth.s_runits = 0;
	s_sth.s_rinc = 0;
	s_sth.s_hunits = 0;
	s_sth.s_hinc = 0;
	s_sth.s_schytd = 0;
	s_sth.s_calytd = 0;

	strcpy(empearn.en_numb,s_sth.s_empcd);
	empearn.en_pp = 0;
	empearn.en_week = 0;
	empearn.en_date = 0;
	flg_reset(EMP_EARN);

	for(;;){
		retval = get_n_emp_earn(&empearn,BROWSE,0,FORWARD,e_mesg);
		if(retval == EFL) break;
		if(retval < 0){
			DispError((char *)&s_sth,e_mesg);
			return(retval);
		}

		if(strcmp(empearn.en_numb,s_sth.s_empcd)!=0) break;

		if(empearn.en_date >= param.pr_fisc_st_dt && empearn.en_date <=
			param.pr_fisc_end_dt){
			s_sth.s_runits += empearn.en_reg_units;
			s_sth.s_rinc += empearn.en_reg_inc;
			s_sth.s_hunits += empearn.en_high_units;
			s_sth.s_hinc += empearn.en_high_inc;
		}

		if(empearn.en_date >= param.pr_cal_st_dt && empearn.en_date <=
			param.pr_cal_end_dt){
			s_sth.s_calytd += (empearn.en_reg1 + empearn.en_reg1 +
			 empearn.en_reg1 + empearn.en_high_inc);
		}

		if(empearn.en_date >= param.pr_schl_st_dt && empearn.en_date <=
			param.pr_schl_end_dt){
			s_sth.s_schytd += (empearn.en_reg1 + empearn.en_reg1 +
			 empearn.en_reg1 + empearn.en_high_inc);
		}
	}
	seq_over(EMP_EARN);

	return(NOERROR);
}
/*---------------------------------------------------------------------------*/
static
CalcSalary()
{
	int retval;
	double 	temp_calc = 0;
	double temp_calc2 = 0;
	int	temp_units = 0;
	int j;
	double	one_hundred = 100;

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

	if(emp_rec.em_class[0] == '\0') {
		s_sth.s_salary = 0;
	}
	else {
		strcpy(class.c_code,emp_rec.em_class);
		class.c_date = get_date();
		flg_reset(CLASSIFICATION);
		retval = get_n_class(&class,BROWSE,0,BACKWARD,e_mesg);
		if((retval < 0 ||
		    strcmp(class.c_code,emp_rec.em_class) != 0)){
			fomer("Classification Code Does Not Exist - Please Re-enter");
			get();
			s_sth.s_salary = 0;
			return(-1);
		}
	 	if(class.c_units != 0) {	
			temp_units = 0;
			strcpy(emp_sched1.es_numb,emp_rec.em_numb);
			emp_sched1.es_week = 0;
			emp_sched1.es_fund = 0;
			emp_sched1.es_class[0] = LV_CHAR;
			emp_sched1.es_cost = 0;
			emp_sched1.es_dept[0] = LV_CHAR;
			emp_sched1.es_area[0] = LV_CHAR;
			flg_reset(EMP_SCHED1);

			for( ; ; ) {
		       	  retval = get_n_emp_sched1(&emp_sched1,BROWSE,0,
				   FORWARD,e_mesg);
			  if(retval < 0 && retval != EFL) {
				DispError((char *)&s_sth,e_mesg);
			  }
			  if(strcmp(emp_sched1.es_numb,emp_rec.em_numb)!=0
				   ||retval == EFL)
				break;
			  for(j=0 ; j<7 ; j++) {
				  temp_units += emp_sched1.es_units[j];
				}
			}
			seq_over(EMP_SCHED1);
 		        temp_calc = class.c_yrly_inc / class.c_units; 
			temp_calc = D_Roundoff(temp_calc);
			s_sth.s_salary = (double)temp_calc;
		}
		else{
			strcpy(pay_per.pp_code,barg_unit.b_pp_code);
			pay_per.pp_year = 0;
			flg_reset(PAY_PERIOD);

			retval = get_n_pay_per(&pay_per,BROWSE,0,FORWARD,
								e_mesg);
			if(retval < 0 || strcmp(pay_per.pp_code,
					        barg_unit.b_pp_code)!=0) {
				fomer("Pay Period Code Does not Exist");
				get();
			}
			seq_over(PAY_PERIOD);
			if(class.c_yrly_inc == 0){
				s_sth.s_salary = 0;
			}
			else{
				 	temp_calc2 = class.c_yrly_inc / 
						(double)pay_per.pp_numb;
					temp_calc = temp_calc2 * emp_rec.em_perc/ one_hundred; 
					temp_calc = D_Roundoff(temp_calc);
		    	}
			s_sth.s_salary = (double)temp_calc;
		}
	}
	return(NOERROR);
}
/*---------------------------------------------------------------------------*/
/* change all screen fields and update the changes on file 	             */
static
ChangeRec()
{
	int retval;

	retval = SetDupBuffers(FT_FLD,SCHYTD_FLD,1);
	if(retval < 0) return(retval);

	retval = GetSubOpt();
	if(retval < 0) return(retval);

	return(NOERROR);
}
/*---------------------------------------------------------------------------*/
/* display sub-option line and get response 				     */
static
GetSubOpt()
{
	int retval;
	
	for(;;){
#ifdef ENGLISH
		strcpy(s_sth.s_mesg,"Y(es), S(creen edit), L(ine edit), C(ancel)");
#else
		strcpy(s_sth.s_mesg,"Y(es), S(creen edit), L(ine edit), C(ancel)");
#endif

	  	retval = WriteFields((char *)&s_sth,MSG_FLD,MSG_FLD);
		if(retval < 0) return(retval);

		s_sth.s_resp[0] = LV_CHAR;
		retval = ReadFields((char *)&s_sth,RESP_FLD,RESP_FLD,
			Validate,WindowHelp,0);
		if(retval < 0) return(retval);

		if(s_sth.s_resp[0] == SCREDIT){
			s_sth.s_mesg[0] = HV_CHAR;
			s_sth.s_resp[0] = HV_CHAR;
	  		retval = WriteFields((char *)&s_sth,MSG_FLD,MSG_FLD);
			if(retval < 0) return(retval);
			retval = ChangeScreen();
			if(retval < 0) return(retval);
		}

		if(s_sth.s_resp[0] == LINEEDIT){
			s_sth.s_mesg[0] = HV_CHAR;
			s_sth.s_resp[0] = HV_CHAR;
	  		retval = WriteFields((char *)&s_sth,MSG_FLD,RESP_FLD);
			if(retval < 0) return(retval);
			retval = ChangeFields();
			if(retval < 0) return(retval);
		}

		if(s_sth.s_resp[0] == YES){
			s_sth.s_mesg[0] = HV_CHAR;
			s_sth.s_resp[0] = HV_CHAR;
	  		retval = WriteFields((char *)&s_sth,MSG_FLD,RESP_FLD);
			if(retval < 0) return(retval);
			retval = UpdateFiles();
			if(retval < 0) return(retval);
			break;
		}

		if(s_sth.s_resp[0] == CANCEL){
#ifdef ENGLISH
			strcpy(s_sth.s_mesg,"Confirm The Cancel (Y/N)");
#else
			strcpy(s_sth.s_mesg,"Confirm The Cancel (Y/N)");
#endif
			retval = WriteFields((char *)&s_sth,MSG_FLD,MSG_FLD);
			if(retval < 0) return(retval);
		
			s_sth.s_resp[0] = LV_CHAR;
			retval = ReadFields((char *)&s_sth,RESP_FLD,RESP_FLD,
				Validate,WindowHelp,0);
			if(retval < 0) return(retval);

			if(s_sth.s_resp[0] == NO) continue;
			s_sth.s_mesg[0] = HV_CHAR;
			s_sth.s_resp[0] = HV_CHAR;
	  		retval = WriteFields((char *)&s_sth,MSG_FLD,RESP_FLD);
			if(retval < 0) return(retval);
			break;
		}
	}

	return(NOERROR);
}
/*---------------------------------------------------------------------------*/
/* update cheque information on file					     */
static
UpdateFiles()
{
	int retval;

	strcpy(emp_rec.em_numb,s_sth.s_empcd);
	retval = get_employee(&emp_rec,UPDATE,0,e_mesg);
	if(retval < 0){
		DispError((char *)&s_sth,e_mesg);
		return(retval);
	}

	emp_rec.em_st_dt_ft = s_sth.s_stft;
	emp_rec.em_st_dt_pt = s_sth.s_stpt;
	emp_rec.em_st_dt_ca = s_sth.s_stca;
	emp_rec.em_st_dt_su = s_sth.s_stsu;

	emp_rec.em_cont_dt = s_sth.s_cont;
	emp_rec.em_app_dt = s_sth.s_app;
	emp_rec.em_ann = s_sth.s_ann;
	strcpy(emp_rec.em_lang,s_sth.s_lang);

	emp_rec.em_last_roe = s_sth.s_roe;
	emp_rec.em_num_ins_wk = s_sth.s_ins_wk;
	strcpy(emp_rec.em_un_tel,s_sth.s_phone);
	emp_rec.em_vac_rate = s_sth.s_vacpay;

	emp_rec.em_term_dt = s_sth.s_term_dt;
	strcpy(emp_rec.em_term,s_sth.s_termcd);

	if(strcmp(param.pr_prov,"NB")==0){
		strcpy(emp_rec.em_ins,s_sth.s_inscl);
		strcpy(emp_rec.em_pre_paid,s_sth.s_paid);
	}

	retval = put_employee(&emp_rec,UPDATE,e_mesg);
	if(retval < 0) return(retval);

	retval = commit(e_mesg);
	if(retval < 0) return(retval);

	return(NOERROR);
}
/*---------------------------------------------------------------------------*/
/* change screen fields							     */
static
ChangeScreen()
{
	int retval;
	
	/* make copy of screen incase user presses ESC-F */
	scpy((char *)&image,(char *)&s_sth,sizeof(s_sth));

#ifdef ENGLISH
	strcpy(s_sth.s_mesg,"Press ESC-F to Terminate");
#else
	strcpy(s_sth.s_mesg,"Appuyer sur ESC-F pour terminer");
#endif
	DispMesgFld((char *)&s_sth);

	SetDupBuffers(FT_FLD,SCHYTD_FLD,1);
	scpy((char *)&image,(char *)&s_sth,sizeof(s_sth));

	retval = MoveLows();
	if(retval < 0) return(retval);
	
	retval = ReadFields((char *)&s_sth,FT_FLD,SCHYTD_FLD,
			Validate,WindowHelp,1);
	if(retval < 0) return(retval);
	if(retval == RET_USER_ESC){
		retval=CopyBack((char *)&s_sth,(char *)&image,sr.curfld,
			SCHYTD_FLD);
		if(retval < 0) return(retval);
	}

	return(NOERROR);
}
/*-----------------------------------------------------------------------*/ 
/* change one field at a time					    	 */
static
ChangeFields()
{
	int retval, st_fld;

	/* make copy of screen incase user presses ESC-F */
	scpy((char *)&image,(char *)&s_sth,sizeof(s_sth));

#ifdef ENGLISH
	strcpy(s_sth.s_mesg,"Press ESC-F to Terminate");
#else
	strcpy(s_sth.s_mesg,"Appuyer sur ESC-F pour terminer");
#endif
	DispMesgFld((char *)&s_sth);

	for(;;){
		retval = SetDupBuffers(FT_FLD,SCHYTD_FLD,1);
		if(retval < 0) return(retval);

		s_sth.s_field = LV_SHORT;
		retval = ReadFields((char *)&s_sth,CHG_FLD,CHG_FLD,
			Validate,WindowHelp,1);
		if(retval < 0) return(retval);
		
		if(s_sth.s_field == 0) break;
		if(retval == RET_USER_ESC) break;
		if(s_sth.s_field > 17) continue;

		switch(s_sth.s_field){
		case 1:
			st_fld = FT_FLD;
			s_sth.s_stft = LV_LONG;
			break;

		case 2:
			st_fld = PT_FLD;
			s_sth.s_stpt = LV_LONG;
			break;

		case 3:
			st_fld = CA_FLD;
			s_sth.s_stca = LV_LONG;
			break;

		case 4:
			st_fld = SU_FLD;
			s_sth.s_stsu = LV_LONG;
			break;

		case 5:
			st_fld = CONT_FLD;
			s_sth.s_cont = LV_LONG;
			break;

		case 6:
			st_fld = APP_FLD;
			s_sth.s_app = LV_LONG;
			break;

		case 7:
			st_fld = ANN_FLD;
			s_sth.s_ann = LV_SHORT;
			break;

		case 8:
			st_fld = LANG_FLD;
			s_sth.s_lang[0] = LV_CHAR;
			break;

		case 9:
			st_fld = ROE_FLD;
			s_sth.s_roe = LV_LONG;
			break;

		case 10:
			st_fld = INSWK_FLD;
			s_sth.s_roe = LV_SHORT;
			break;

		case 11:
			st_fld = PHONE_FLD;
			s_sth.s_phone[0] = LV_CHAR;
			break;

		case 12:
			st_fld = VACPAY_FLD;
			s_sth.s_vacpay = LV_DOUBLE;
			break;

		case 13:
			st_fld = CLASS_FLD;
			if(strcmp(param.pr_prov,"NB")!=0) continue;
			s_sth.s_inscl[0] = LV_CHAR;
			break;

		case 14:
			st_fld = PAID_FLD;
			if(strcmp(param.pr_prov,"NB")!=0) continue;
			s_sth.s_paid[0] = LV_CHAR;
			break;
		case 15:
			st_fld = EXPENSE_FLD;
			if(strcmp(param.pr_prov,"NB")!=0) continue;
			s_sth.s_expense[0] = LV_CHAR;
			break;
		case 16:
			st_fld = TERMDT_FLD;
			s_sth.s_term_dt = LV_LONG;
			break;

		case 17:
			st_fld = TERMCD_FLD;
			s_sth.s_termcd[0] = LV_CHAR;
			break;


		default:
			break;
		}

		retval = ReadFields((char *)&s_sth,st_fld,st_fld,Validate,
			WindowHelp,1);
		if(retval < 0) return(retval);
		if(retval == RET_USER_ESC){
			retval=CopyBack((char *)&s_sth,(char *)&image,st_fld,
				st_fld);
			if(retval < 0) return(retval);
			break;
		}
	}

	s_sth.s_field = HV_SHORT;
	retval = WriteFields((char *)&s_sth,CHG_FLD,CHG_FLD);
	if(retval < 0) return(retval);

	return(NOERROR) ;
}	/* InitGlTrans() */
/*---------------------------------------------------------------------------*/
static
Validate()	/* Validate the values entered by the user */
{
	int  retval;

	switch( sr.curfld ){
		case FN_FLD:
			if(s_sth.s_fn[0] == '\0'){
				s_sth.s_fn[0] = LV_CHAR;
				return(-1);
			}
			
			if(s_sth.s_fn[0] == EXITOPT) break;
			if(s_sth.s_fn[0] == CHGREC) break;
			if(s_sth.s_fn[0] == INQREC) break;
			if(s_sth.s_fn[0] == NEXT_RECORD) break;
			if(s_sth.s_fn[0] == PREV_RECORD) break;
			if(s_sth.s_fn[0] == NEXT_SCR) break;
			if(s_sth.s_fn[0] == PREV_SCR) break;
			if(s_sth.s_fn[0] == SCREEN) break;
			
		case EMP_FLD:
			Right_Justify_Numeric(s_sth.s_empcd,
						sizeof(emp_rec.em_numb)-1);
			if(s_sth.s_empcd[0] == '\0'){
				s_sth.s_empcd[0] = LV_CHAR;
				return(-1);
			}

			strcpy(emp_rec.em_numb,s_sth.s_empcd);
			retval = get_employee(&emp_rec,BROWSE,0,e_mesg);
			if(retval == UNDEF){
				fomen(NOKEY);
				s_sth.s_empcd[0] = LV_CHAR;
				return(-1);
			}

			if(retval < 0){
				DispError((char *)&s_sth,e_mesg);
				return(retval);
			}

			retval = UsrBargVal(BROWSE,emp_rec.em_numb,
				emp_rec.em_barg,1,e_mesg);
			if(retval < 0){
				DispError((char *)&s_sth,e_mesg);
			  	s_sth.s_empcd[0] = LV_CHAR;
				return(-1);
			}
			strcpy(s_sth.s_empcd,emp_rec.em_numb);
			strcpy(e_mesg,emp_rec.em_last_name);
			strcat(e_mesg,", ");
			strcat(e_mesg,emp_rec.em_first_name);
			strncpy(s_sth.s_name,e_mesg,30);
			retval = CalcSalary();
	
			retval = WriteFields((char *)&s_sth,EMP_FLD,NAME_FLD);
			if(retval < 0) return(retval);
			break;
			
		case PAID_FLD:
			if(s_sth.s_paid[0] == '\0'){
				sr.curfld += 100;
				break;
			}
		
			if(s_sth.s_paid[0] == YES) break;
			if(s_sth.s_paid[0] == NO) break;

			fomen(YORN);	
			s_sth.s_paid[0] = LV_CHAR;
			return(-1);

		case INSWK_FLD:
			if(s_sth.s_ins_wk > 52){
#ifdef ENGLISH
				fomen("Must Be Less Than 52");
#else
				fomen("Must Be Less Than 52");
#endif
				s_sth.s_ins_wk = LV_SHORT;
				return(-1);
			}
			break;

		case ANN_FLD:
			if(s_sth.s_ann < 1 || s_sth.s_ann > 12){
#ifdef ENGLISH
				fomen("Must Be 1 - 12");
#else
				fomen("Must Be 1 - 12");
#endif
				s_sth.s_ann = LV_SHORT;
				return(-1);
			}
			break;


		case VACPAY_FLD:
			if(s_sth.s_vacpay > 100){
				s_sth.s_vacpay = LV_DOUBLE;
				return(-1);
			}
			break;
		case CLASS_FLD:
			if(s_sth.s_inscl[0] == '\0'){
				sr.curfld += 100;
				break;
			}

			if(s_sth.s_inscl[0] == '1') break;
			if(s_sth.s_inscl[0] == '2') break;
			if(s_sth.s_inscl[0] == '3') break;
			if(s_sth.s_inscl[0] == '4') break;
			
#ifdef ENGLISH
			fomen("Must Be '1', '2', '3' or '4'");
#else
			fomen("Must Be '1', '2', '3' or '4'");
#endif
			s_sth.s_inscl[0] = LV_CHAR;
			return(-1);

		case LANG_FLD:
			if(s_sth.s_lang[0] == '\0'){
				s_sth.s_lang[0] = LV_CHAR;
				return(-1);
			}

			if(s_sth.s_lang[0] == 'F') break;
			if(s_sth.s_lang[0] == 'E') break;
			
#ifdef ENGLISH
			fomen("Must Be F or E");
#else
			fomen("Must Be F or E");
#endif
			s_sth.s_lang[0] = LV_CHAR;
			return(-1);

		case EXPENSE_FLD:
			if(s_sth.s_expense[0] == '\0'){
				sr.curfld += 100;
				break;
			}
			Right_Justify_Numeric(s_sth.s_expense,
						sizeof(expense.ex_code)-1);
			strcpy(expense.ex_code,s_sth.s_expense);
			retval = get_exp(&expense,BROWSE,0,e_mesg);
			if(retval == UNDEF){
				fomen(NOKEY);
				s_sth.s_expense[0] = LV_CHAR;
				return(-1);
			}
			if(retval < 0){
				DispError((char *)&s_sth,e_mesg);
				return(retval);
			}
			retval = WriteFields((char *)&s_sth,
						EXPENSE_FLD,EXPENSE_FLD);
			if(retval < 0)
				return(retval);
			break;

		case TERMCD_FLD:
			if(s_sth.s_termcd[0] == '\0'){
				sr.curfld += 100;
				break;
			}
			Right_Justify_Numeric(s_sth.s_termcd
						,(sizeof(s_sth.s_termcd)-1));
			strcpy(termination.t_code,s_sth.s_termcd);

			retval = get_pterm(&termination,BROWSE,0,e_mesg);
			if(retval == UNDEF) {
				fomen(NOKEY) ;
				s_sth.s_termcd[0] = LV_CHAR;
				return(retval);
			}
			if(retval < 0) {
				fomen(e_mesg);
				get();
				return(retval);
			}

			break;

		default:
#ifdef ENGLISH
			fomen("No validation for this field");
#else
			fomen("Pas de validation pour ce champ");
#endif
			break;
	}
	sr.nextfld = sr.curfld;
	return(NOERROR);
}    /*  Validate()   */
/*---------------------------------------------------------------------------*/
static
SetDupBuffers( firstfld, lastfld, value )
int	firstfld, lastfld;	/* field numbers range */
int	value;			/* ENABLE or DISABLE */
{
	int i;

	for( i=firstfld; i<=lastfld; i+=50 )
		fomca1( i, 19, value);
	sr.nextfld = firstfld;
	sr.endfld = lastfld;
	fomud( (char *)&s_sth );
	ret( err_chk(&sr) );

	return( 0 );
}     /*    SetDupBuffers()    */
/*---------------------------------------------------------------------------*/
/* help window for screen fields					     */
static
WindowHelp()
{
	int retval;

	switch(sr.curfld){
	case EMP_FLD:
		retval = emp_hlp(s_sth.s_empcd,7,13);
		if(retval < 0) return(retval);
		redraw();

		strcpy(emp_rec.em_numb,s_sth.s_empcd);
		retval = get_employee(&emp_rec,BROWSE,0,e_mesg);
		if(retval < 0){
			DispError((char *)&s_sth,e_mesg);
			return(retval);
		}
		
		strcpy(s_sth.s_empcd,emp_rec.em_numb);
		strcpy(e_mesg,emp_rec.em_last_name);
		strcat(e_mesg,", ");
		strcat(e_mesg,emp_rec.em_first_name);
		strncpy(s_sth.s_name,e_mesg,30);
		retval = CalcSalary();

		strcpy(s_sth.s_status,emp_rec.em_status);
		retval = WriteFields((char *)&s_sth,NAME_FLD,STAT_FLD);
		if(retval < 0) return(retval);
		break;

	case EXPENSE_FLD:
		retval = exp_hlp(s_sth.s_expense,7,13);
		if(retval == DBH_ERR) return(retval);
		if(retval >= 0) redraw();
		if(retval == 0) return(ERROR);
		if(retval < 0) return(ERROR);
		break;

	case TERMCD_FLD:
		retval = term_hlp(s_sth.s_termcd,7,13);
		if(retval == DBH_ERR) return(retval);
		if(retval >= 0) redraw();
		if(retval == 0) return(ERROR);
		if(retval < 0) return(ERROR);
		break;
	default:
		fomen(NOHELP);
		break;
	}

	return(NOERROR);
}
/*---------------------------------------------------------------------------*/
/* move low values to data entry fields 				     */
static
MoveLows()
{
	s_sth.s_stft = LV_LONG;
	s_sth.s_stpt = LV_LONG;
	s_sth.s_stca = LV_LONG;
	s_sth.s_stsu = LV_LONG;

	s_sth.s_cont = LV_LONG;
	s_sth.s_app = LV_LONG;
	s_sth.s_ann = LV_SHORT;
	s_sth.s_lang[0] = LV_CHAR;

	s_sth.s_roe = LV_LONG;
	s_sth.s_ins_wk = LV_SHORT;
	s_sth.s_phone[0] = LV_CHAR;
	s_sth.s_vacpay = LV_DOUBLE;

	if(strcmp(param.pr_prov,"NB")==0){
		s_sth.s_paid[0] = LV_CHAR;
		s_sth.s_inscl[0] = LV_CHAR;
		s_sth.s_expense[0] = LV_CHAR;
	}
	else{
		s_sth.s_inscl[0] = HV_CHAR;
		s_sth.s_paid[0] = HV_CHAR;
		s_sth.s_expense[0] = HV_CHAR;
	}
	
	s_sth.s_termcd[0] = LV_CHAR;
	s_sth.s_term_dt = LV_LONG; 

	return(NOERROR);
}
/*---------------------------------------------------------------------------*/
/* clear screen fields							     */
static
ClearScreen()
{
	int retval;

	s_sth.s_field = HV_SHORT;
	s_sth.s_mesg[0] = HV_CHAR;
	s_sth.s_resp[0] = HV_CHAR;

	s_sth.s_name[0] = HV_CHAR;
	s_sth.s_salary = HV_DOUBLE;
	s_sth.s_status[0] = HV_CHAR;
	s_sth.s_stft = HV_LONG;
	s_sth.s_term_dt = HV_LONG;

	s_sth.s_stpt = HV_LONG;
	s_sth.s_termcd[0] = HV_CHAR;
	s_sth.s_stca = HV_LONG;
	s_sth.s_stsu = HV_LONG;

	s_sth.s_cont = HV_LONG;
	s_sth.s_runits = HV_DOUBLE;
	s_sth.s_app = HV_LONG;
	s_sth.s_rinc = HV_DOUBLE;

	s_sth.s_ann = HV_SHORT;
	s_sth.s_hunits = HV_DOUBLE;
	s_sth.s_lang[0] = HV_CHAR;
	s_sth.s_hinc = HV_DOUBLE;

	s_sth.s_roe = HV_LONG;
	s_sth.s_ins_wk = HV_SHORT;
	s_sth.s_calytd = HV_DOUBLE;
	s_sth.s_phone[0] = HV_CHAR;

	s_sth.s_schytd = HV_DOUBLE;
	s_sth.s_vacpay = HV_DOUBLE;
	s_sth.s_inscl[0] = HV_CHAR;
	s_sth.s_paid[0] = HV_CHAR;
	s_sth.s_expense[0] = HV_CHAR;

	retval = WriteFields((char *)&s_sth,NAME_FLD,RESP_FLD);
	if(retval < 0) return(retval);

	return(NOERROR);
}
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
/*-----------------------   End of program   --------------------------------*/