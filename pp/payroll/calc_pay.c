/******************************************************************************
		Sourcename   : calc_pay.c
		System       : Personnel/Payroll system.
		Module       : Cheque Pre-processing.
		Created on   : 91-10-21
		Created  By  : Eugene Roy.
		Cobol Source : 
******************************************************************************
About the program:

	This program does end of day processes which include:

	1.Calculate CPP.
	2.Calculate UIC.
	3.Calculate Registered Pension Plan.
	4.Calculate Income Tax.
	5.Calculate Deductions, Garnishments, CSB/Loans.

History:
Programmer      Last change on    Details
__________      ____/__/__       __________________________________

L.Robichaud	1994/04/13	The garnishments would go into a negative
				because the out standing amount was only
				checked for 0. I added the code to check if
				the payment is greater than the amount left.
			(ie. out standing is $50.00 and payment is $75 would
			put the garnishments into a negative -$25.00. Since 
			this is not 0 the deductions would keep on going).

******************************************************************************/

#define MAIN
#define MAINFL	-1

#include <stdio.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_com.h>
#include <bfs_pp.h>
#include <repdef.h>

#define EXIT	   	12
#define CLOSEDBHEXIT(X)	{ close_dbh(); exit(X); }
#define SYSTEM		"CHEQUE PRE-PROCESSING"
#define MOD_DATE	"20-OCT-91"

#ifdef ENGLISH
#define U_YES	'Y'
#define	L_YES	'y'
#else
#define U_YES	'Y'
#define	L_YES	'y'
#endif
#ifdef ENGLISH
#define U_NO	'N'
#define	L_NO	'n'
#else
#define U_NO	'N'
#define	L_NO	'n'
#endif

static	Pa_rec	pa_rec;
static	Pay_earn	pp_earn;
static	Pay_ded	pp_ded;
static	Pay_garn	pp_garn;
static	Pay_loan	pp_loan;
static	Pay_param	pay_param;
static	Emp	emp_rec;
static	Emp_earn	emp_earn;
static	Emp_ins	emp_ins;
static	Emp_ded	emp_ded;
static	Emp_dh	emp_dd_his;
static	Emp_garn	emp_garn;
static	Emp_loan	emp_loan;
static	Time	time_rec;
static	Pay_per	pay_period;
static	Pay_per_it	pay_per_it;
static	Uic	uic_table;
static	Tax	tax_table;
static	Reg_pen	reg_pen;
static	Csb_loan	loan;
static	Class_item	class_item;
static	Gl_acct	gl_acct;
static	Jr_ent	jr_ent;
static	Barg_unit	barg_unit;
static	Deduction	deduction;
static	Ded_group	ded_grp;
static	Gov_param	gov_param;

char e_mesg[80];	/* for storing error messages */
long	get_date(),date_plus() ;
static long sysdt;	/* system date */
static int	err,retval;
double	D_Roundoff();

extern	int	errno;
static int	PG_SIZE;
static char 	discfile[15];	/* for storing outputfile name */
static short	pgcnt; 		/* for page count */

main( argc, argv)
int argc;
char *argv[];
{
	
	strncpy( SYS_NAME, SYSTEM, 50 );	/* Module name */
	strncpy( CHNG_DATE, MOD_DATE, 10 );	/* Last date of change */
	proc_switch( argc,argv,MAINFL );	/* process the switches */

	retval = GetUserClass(e_mesg) ;

	sysdt = get_date();
	mkdate(sysdt, e_mesg);

#ifdef ENGLISH
	printf("\n\n\n\n\n\t\t\tPAY PERIOD CALCULATIONS \n\n\n", e_mesg);
	printf("\t\t1. Calculate CPP\n");
	printf("\t\t2. Calculate UIC\n");
	printf("\t\t3. Calculate Registered Pension Plan\n");
	printf("\t\t4. Calculate Income Tax\n");
	printf("\t\t5. Calculate Deductions, Garnishment\n");
	printf("\t\t6. Calculate CSB/Loans\n");
#else

#endif

#ifdef ENGLISH
	printf("\n\t\tProceed (Y/N)? ");
#else
	printf("\n\t\tProceder (O/N)? ");
#endif
	scanf("%s",e_mesg);
	if(e_mesg[0] != U_YES && e_mesg[0] != L_YES) {
		close_dbh();
		close_rep();
		exit(-1);
	}

	if(Pay_Calc() < 0){
		CLOSEDBHEXIT(-1);
	}
	close_rep();

#ifdef ENGLISH
	printf("\n\nPay Period Calculations Successful. Press RETURN ");
#else
#endif
	fflush( stdin );
	getchar();

	CLOSEDBHEXIT(0);
}
/*************************************************************************/
static
Pay_Calc()
{
	int 	last_jr, i, j, no_weeks;
	long	sysdate, temp_date, sys_month, emp_month, emp_age;
	short	week, last_fund;
	double	uic_rate, emp_inc, emp_cpp_contr, emp_cpp_pen, cpp_max_contr,
		temp_cpp, emp_uic_inc, tot_uic_ins, emp_uic_units, 
		emp_uic_prem, emp_reg_contr, emp_ded_contr, week_min_hrs, 
		week_min_earn, week_inc, week_units, UIC_Emp_Share, 
		tot_reg_pen, Union, Ann_Tax_Inc, Net_Pers_Tc, CPP_UIC_Tc, 
		Oth_Fed_Tc, CPP_UIC_Tc_temp1, CPP_UIC_Tc_temp2, 
		Ann_Bas_Fed_Tax, Fed_Surtax, Fed_Tax_Ded, Prov_Inc_Tax, 
		temppe_net, empr_share, tot_amount, week_max_earn, 
		tot_income, tmp_share, tmp_income, week_income[5];
	char	last_type[2];

	/* get the cost center name */
	retval = get_param(&pa_rec,BROWSE,1,e_mesg);
	if (retval < 0){
		fomer(e_mesg);get ();
	}

	err = get_pay_param(&pay_param,BROWSE, 1, e_mesg) ;
	if(err < 0) {
		DispErr(e_mesg) ;
		return(ERROR) ;
	}

	gov_param.gp_eff_date = get_date();

	flg_reset(GOV_PARAM);

	retval = get_n_gov_param(&gov_param,BROWSE, 0, BACKWARD, e_mesg) ;
	if(retval == EFL) {
		fomen("Government Parameter Record Not Setup");
		return(retval);
	}
	if(retval < 0) {
		fomer(e_mesg) ;
		return(ERROR) ;
	}

	sysdate = get_date();
	if(InitPrinter1()<0) {
		return(-1);
	}	
	retval = PrntHdg();
	if(retval < 0)	return(retval);

	last_jr = 0;

	pp_earn.pe_numb[0] = '\0';
	pp_earn.pe_pp = 0;
	pp_earn.pe_date = 0;
	flg_reset(PP_EARN);

	for( ; ; ){
	  err = get_n_pp_earn(&pp_earn,UPDATE, 0, FORWARD, e_mesg) ;
	  if(err == EFL) break;
	  if(err < 0) {
		DispErr(e_mesg) ;
	  	retval = PrntRec(e_mesg);
		roll_back(e_mesg);
		seq_over(PP_EARN);
		return(retval);
	  }

	  if(pp_earn.pe_net > 0.00)
		continue;

	  strcpy(emp_rec.em_numb,pp_earn.pe_numb);
	  err = get_employee(&emp_rec,BROWSE, 0, e_mesg) ;
	  if(err < 0) {
		DispErr(e_mesg) ;
	  	retval = PrntRec(e_mesg);
		roll_back(e_mesg);
		continue;
	  }

	  printf("Employee # %s\r",emp_rec.em_numb);
	  fflush( stdout );

	  strcpy(barg_unit.b_code,emp_rec.em_barg);
	  barg_unit.b_date = sysdate;
	  flg_reset(BARG);

	  retval = get_n_barg(&barg_unit,BROWSE,0,BACKWARD,e_mesg);
	  if(retval == EFL ||
		   strcmp(barg_unit.b_code, emp_rec.em_barg) != 0){
		sprintf(e_mesg,"Bargaining Unit Does Not Exist: %s",emp_rec.em_barg);
	  	retval = PrntRec(e_mesg);
		roll_back(e_mesg);
		seq_over(BARG);
		continue;
	  }
	  if(retval < 0){
  		DispErr(e_mesg) ;
	  	retval = PrntRec(e_mesg);
		roll_back(e_mesg);
		seq_over(BARG);
		return(retval);	
	  }
	  seq_over(BARG);

	  strcpy(pay_period.pp_code, barg_unit.b_pp_code);
	  pay_period.pp_year = 0;
	  flg_reset(PAY_PERIOD);

	  retval = get_n_pay_per(&pay_period,BROWSE,0,FORWARD,e_mesg);
	  if(retval < 0 && retval != EFL){
		DispErr(e_mesg) ;
	  	retval = PrntRec(e_mesg) ;
	  	if(retval < 0){
			DispErr(e_mesg) ;
	  	}
		roll_back(e_mesg);
		return(retval);
	  }
	  if(strcmp(pay_period.pp_code,barg_unit.b_pp_code)!=0 ||
	     retval == EFL){
#ifdef ENGLISH
	    	sprintf(e_mesg,"Pay Period Code %s Not on File",
		    	barg_unit.b_pp_code);
#else
		sprintf(e_mesg,"Pay Period Code %s Not on File",
	    		barg_unit.b_pp_code);
#endif

		DispErr(e_mesg) ;
	  	retval = PrntRec(e_mesg) ;
	  	if(retval < 0){
			DispErr(e_mesg) ;
	  	}
		roll_back(e_mesg);
		continue;
	  }
	  seq_over(PAY_PERIOD);

	  strcpy(pay_per_it.ppi_code, barg_unit.b_pp_code);
	  pay_per_it.ppi_numb = pp_earn.pe_pp;
	  pay_per_it.ppi_year = 9999;
	  flg_reset(PAY_PER_ITEM);

	  retval = get_n_pp_it(&pay_per_it,BROWSE,3,BACKWARD,e_mesg);
	  if(strcmp(pay_per_it.ppi_code,barg_unit.b_pp_code)!=0 ||
	     pay_per_it.ppi_numb != pp_earn.pe_pp ||
	     retval == EFL){
		sprintf(e_mesg,"Pay Period Item Not on File For Code: %s Pay Period: %d",
	    		barg_unit.b_pp_code, pp_earn.pe_pp);
		DispErr(e_mesg) ;
	  	retval = PrntRec(e_mesg) ;
	  	if(retval < 0){
			DispErr(e_mesg) ;
	  	}
		roll_back(e_mesg);
		continue;
	  }
	  if(retval < 0){
		DispErr(e_mesg) ;
	  	retval = PrntRec(e_mesg) ;
	  	if(retval < 0){
			DispErr(e_mesg) ;
	  	}
		roll_back(e_mesg);
		return(retval);
	  }

	  pp_earn.pe_net = D_Roundoff(pp_earn.pe_reg_inc1 +
			          pp_earn.pe_reg_inc2 + pp_earn.pe_high_inc
				  + pp_earn.pe_ben + pp_earn.pe_vac);

	  tot_income = D_Roundoff(pp_earn.pe_net);

	  for(i=0;i<5;i++)
		week_income[i] = D_Roundoff(pp_earn.pe_week_inc[i] +
			pp_earn.pe_week_hinc[i]);

	  if(pp_earn.pe_net <= 0.00)
		continue;

	  emp_cpp_contr = 0;
	  emp_cpp_pen = 0;
	  emp_inc = 0;
	  emp_reg_contr = 0;
	  emp_uic_prem = 0;

	  strcpy(emp_earn.en_numb,pp_earn.pe_numb); 
	  emp_earn.en_date = 0;
	  emp_earn.en_pp = 0;
	  emp_earn.en_week = 0;
	  flg_reset(EMP_EARN);

	  for( ; ; ){
		err = get_n_emp_earn(&emp_earn,BROWSE, 0,FORWARD, e_mesg) ;
		if(err == EFL) break;
		if(err < 0) {
			DispErr(e_mesg) ;
	  		retval = PrntRec(e_mesg);
			roll_back(e_mesg);
			seq_over(EMP_EARN);
			return(retval);
		}
		if(strcmp(emp_earn.en_numb,pp_earn.pe_numb) != 0)
			break;
		if(emp_earn.en_date < pay_param.pr_cal_st_dt)
			continue;
		emp_inc += D_Roundoff(emp_earn.en_reg_inc +
					 emp_earn.en_high_inc);

		emp_cpp_contr += emp_earn.en_cpp;			
		emp_cpp_pen += emp_earn.en_reg_inc;			
		emp_uic_prem += emp_earn.en_uic;			
		emp_reg_contr += D_Roundoff(emp_earn.en_reg1 +
				 emp_earn.en_reg2 + emp_earn.en_reg3);
	  }
	  seq_over(EMP_EARN);
	  close_file(EMP_EARN);
	  /*********************************************************/
	  /* Deferred Salary Calculation			*/

	  if(emp_rec.em_def_inc > 0){
		pp_earn.pe_def_inc = (pp_earn.pe_net * 
				     (emp_rec.em_def_inc/100.0));
	  	pp_earn.pe_net -= pp_earn.pe_def_inc;
	  	for(i=0;i<5;i++){
			pp_earn.pe_wk_def_inc[i] =
				D_Roundoff((week_income[i] / tot_income) *
				pp_earn.pe_def_inc);
	  	}
	  }

	  /*********************************************************/

	  /* Calculate CPP	*/

	  if(emp_rec.em_cpp_exp[0] == U_NO){

#ifdef ENGLISH
	    printf("\n\tCalculating CPP for: %s\n",pp_earn.pe_numb);
#else
#endif

	    emp_age = (sysdate - emp_rec.em_date) / 10000; 
	    if((emp_age >= gov_param.gp_cpp_min_age) ||  
		(emp_age <= gov_param.gp_cpp_max_age)){

	      if((emp_age == gov_param.gp_cpp_min_age) || 
		 (emp_age == gov_param.gp_cpp_max_age)){
	   	 temp_date = emp_rec.em_date;
		 temp_date = temp_date / 100;
		 emp_month = temp_date % 100;

	   	 temp_date = sysdate;
		 temp_date = temp_date / 100;
		 sys_month = temp_date % 100;

	      }
	      if((emp_age == gov_param.gp_cpp_min_age)&&(emp_month >sys_month)){
		      cpp_max_contr = (gov_param.gp_cpp_max_contr /
						 12) * (12 - emp_month);
	      }
	      if((emp_age == gov_param.gp_cpp_max_age)&&(emp_month<=sys_month)){
		      cpp_max_contr = (gov_param.gp_cpp_max_contr /
							 12) * emp_month;
	      }

	      if((emp_age > gov_param.gp_cpp_min_age) && 
		 (emp_age < gov_param.gp_cpp_max_age)){
		      cpp_max_contr = gov_param.gp_cpp_max_contr;
	      }
	      pp_earn.pe_cpp_pen = pp_earn.pe_reg_inc1 +
				 pp_earn.pe_reg_inc2 + pp_earn.pe_high_inc +
				 pp_earn.pe_ben + pp_earn.pe_vac; 

	      strcpy(time_rec.tm_numb,emp_rec.em_numb);
	      time_rec.tm_date = pp_earn.pe_date;
	      time_rec.tm_no = 0;
	      flg_reset(TIME);

 	      for( ; ; ) {
		retval = get_n_ptime(&time_rec,BROWSE,0,FORWARD,e_mesg);
		if(retval == EFL)
			break ;
		if(retval < 0){
			DispErr(e_mesg) ;
	  		retval = PrntRec(e_mesg);
			roll_back(e_mesg);
			seq_over(TIME);
			return(retval);
		}
		if(time_rec.tm_date != pp_earn.pe_date)
			continue;

		if(strcmp(time_rec.tm_numb, emp_rec.em_numb) != 0)
			break;

		/* earnings code 08 is severance pay */

		if(strcmp(time_rec.tm_earn, "    08") == 0)
			pp_earn.pe_cpp_pen -= time_rec.tm_tot_amt;
	      }
	      seq_over(TIME);

	      pp_earn.pe_cpp_pen = D_Roundoff(pp_earn.pe_cpp_pen);
			
	      temp_cpp = cpp_max_contr - emp_cpp_contr;
	      pp_earn.pe_cpp = gov_param.gp_cpp_reg_rate *
		      (pp_earn.pe_cpp_pen - (double)(gov_param.gp_cpp_bac_expt /
		      (double)pay_period.pp_numb));

	      if(temp_cpp < pp_earn.pe_cpp)
		      pp_earn.pe_cpp = temp_cpp;

	      if(pp_earn.pe_cpp < 0)
		      pp_earn.pe_cpp = 0;

	      pp_earn.pe_cpp = D_Roundoff(pp_earn.pe_cpp);
	      pp_earn.pe_net -= pp_earn.pe_cpp;
	      for(i=0;i<5;i++){
		pp_earn.pe_week_cpp[i] =
			D_Roundoff((week_income[i] / tot_income) *
			pp_earn.pe_cpp);
		pp_earn.pe_wk_cpp_pen[i] =
			D_Roundoff((week_income[i] / tot_income) *
			pp_earn.pe_cpp_pen);
	      }

	      tmp_share = pp_earn.pe_cpp;
	      tmp_income = tot_income;
		    
	      /* Create journal entries for the employer share */

	      strcpy(jr_ent.jr_emp_numb,emp_rec.em_numb);
	      strcpy(jr_ent.jr_type,"B");
	      jr_ent.jr_fund = 0;
	      jr_ent.jr_no = 0;
		
	      flg_reset(JR_ENT);
	      for( ; ; ){
	        retval = get_n_jr_ent(&jr_ent,BROWSE,2,FORWARD,e_mesg);
	        if(retval < 0){
		  if(retval == EFL) break;
		  DispErr(e_mesg) ;
	  	  retval = PrntRec(e_mesg);
		  roll_back(e_mesg);
		  seq_over(JR_ENT);
		  return(retval);
		}
		if(strcmp(jr_ent.jr_emp_numb,emp_rec.em_numb) != 0) 
			break;
		
		if(strcmp(jr_ent.jr_type,"E") != 0 &&
		   strcmp(jr_ent.jr_type,"X") != 0 &&
		   strcmp(jr_ent.jr_type,"B") != 0) 
			continue;

		strcpy(last_type,jr_ent.jr_type);
		last_fund = jr_ent.jr_fund;
		last_jr = jr_ent.jr_no;

		tot_amount = D_Roundoff(jr_ent.jr_amount);

		/* this first glacct read finds the needed fields
		   in order to do the second read - these fields are
		   the fund, the cost center and the class */

		gl_acct.gl_fund = jr_ent.jr_fund;
		strcpy(gl_acct.gl_acct,jr_ent.jr_acct);
		strcpy(gl_acct.gl_class,jr_ent.jr_class);
	
		retval = get_glacct(&gl_acct,BROWSE,2,e_mesg);
		if(retval < 0){
		  	DispErr(e_mesg) ;
	  	 	retval = PrntRec(e_mesg);
			roll_back(e_mesg);
			continue;
		}
		strcpy(gl_acct.gl_type,"C");
		strcpy(gl_acct.gl_earn,"CPP");

		retval=get_glacct(&gl_acct,BROWSE,0,e_mesg);
		if(retval < 0){
		  	DispErr(e_mesg) ;
	  	 	retval = PrntRec(e_mesg);
			roll_back(e_mesg);
			continue;
		}
		seq_over(GLACCT);
		retval = GetJr(gl_acct.gl_fund);
		if(retval < 0) {
			continue;
		}

		strcpy(jr_ent.jr_emp_numb,emp_rec.em_numb);
		jr_ent.jr_fund = gl_acct.gl_fund;
		strcpy(jr_ent.jr_acct,gl_acct.gl_acct);
		strcpy(jr_ent.jr_code,"CPP");
		strcpy(jr_ent.jr_type,"S");
		jr_ent.jr_amount = ((tmp_share * tot_amount) /
						 tmp_income);

		jr_ent.jr_amount = D_Roundoff(jr_ent.jr_amount);
		tmp_share -= jr_ent.jr_amount;
		tmp_income -= tot_amount;

		if(jr_ent.jr_amount != 0.00) {
		     retval = put_jr_ent(&jr_ent,ADD,e_mesg);	
		     if(retval < 0) {
		 	DispErr(e_mesg);
	  	 	retval = PrntRec(e_mesg);
			roll_back(e_mesg);
			continue;
		     }
		     retval = commit(e_mesg) ;
	  	     if(retval < 0) {
		 	DispErr(e_mesg);
	  	 	retval = PrntRec(e_mesg);
			roll_back(e_mesg);
			continue;
		     }
		}
		strcpy(jr_ent.jr_emp_numb,emp_rec.em_numb);
		strcpy(jr_ent.jr_type, last_type);
		jr_ent.jr_fund = last_fund;
		jr_ent.jr_no = last_jr + 1;

		flg_reset(JR_ENT);
	      }
	      seq_over(JR_ENT);

	      /* Create journal entry for the liability */

	      jr_ent.jr_fund = barg_unit.b_fund;
	      retval = GetJr(barg_unit.b_fund);
	      if(retval < 0) {
		continue;
	      }
	      strcpy(jr_ent.jr_acct,barg_unit.b_cpp_acct);

	      /* Journal entry includes matching shares of the employee 
		 and employer share.	*/

	      jr_ent.jr_amount = D_Roundoff(pp_earn.pe_cpp * 2.0);
	      strcpy(jr_ent.jr_emp_numb,pp_earn.pe_numb);
	      strcpy(jr_ent.jr_code,"CPP");
	      strcpy(jr_ent.jr_type,"D");

	      if(jr_ent.jr_amount != 0.00) {
		   jr_ent.jr_amount = jr_ent.jr_amount * -1;
		   retval = put_jr_ent(&jr_ent,ADD,e_mesg);	
		   if(retval < 0) {
		 	DispErr(e_mesg);
	  		retval = PrntRec(e_mesg);
			roll_back(e_mesg);
			continue;
		   }
	           retval = commit(e_mesg) ;
	           if(retval < 0) {
	  		DispErr(e_mesg);
	  		retval = PrntRec(e_mesg);
			roll_back(e_mesg);
			continue;
	           }
	      }
	    }
	  }
	  /*********************************************************/
	  /* Calculate UIC	*/

	  if(emp_rec.em_uic_exp[0] == U_NO){
		pp_earn.pe_uic = 0;
		uic_table.ui_numb = pay_period.pp_numb;
		uic_table.ui_date = sysdate;
		flg_reset(UIC);

#ifdef ENGLISH
		printf("\tCalculating UIC for: %s\n",pp_earn.pe_numb);
#else
#endif

		retval = get_n_uic(&uic_table,BROWSE,0,BACKWARD,e_mesg);
		if(retval < 0 && retval != EFL){
	  		DispErr(e_mesg) ;
	  		retval = PrntRec(e_mesg);
			roll_back(e_mesg);
			seq_over(UIC);
			return(retval);
		}
		seq_over(UIC);

		if(uic_table.ui_numb != pay_period.pp_numb ||
		   retval == EFL) {
			sprintf(e_mesg,
				"UIC Table for %d Pay Periods Not on File",
	    			pay_period.pp_numb);
			DispErr(e_mesg) ;
	  		retval = PrntRec(e_mesg) ;
			continue;
		}

		no_weeks = 52 / uic_table.ui_numb;	/* No of weeks
						   per pay period */

		if(emp_rec.em_uic_rate != 0){
			uic_rate = emp_rec.em_uic_rate;
		}
		else{
			uic_rate = gov_param.gp_uic_prem;
		}
		tot_uic_ins = 0;

		strcpy(emp_ins.in_numb,emp_rec.em_numb);
	  	emp_ins.in_pp = 0;
		emp_ins.in_date = 0;
		flg_reset(EMP_INS) ;

		for( ; ; ) {
		    retval = get_n_emp_ins(&emp_ins,BROWSE,0,FORWARD,e_mesg) ;
		    if( retval == EFL ||
			strcmp(emp_ins.in_numb,emp_rec.em_numb) != 0)
			break;

		    if( retval < 0) {
	  		DispErr(e_mesg) ;
	  		retval = PrntRec(e_mesg);
			roll_back(e_mesg);
			seq_over(EMP_INS);
			return(retval);
		    }
		    if(emp_ins.in_date < pay_param.pr_cal_st_dt)
			continue;
		    tot_uic_ins += D_Roundoff(emp_ins.in_uic_ins);
		}
		seq_over(EMP_INS);
		close_file(EMP_INS);

 		emp_uic_inc = 0;
		emp_uic_units = 0;
		emp_uic_inc = (pp_earn.pe_reg_inc1 + pp_earn.pe_reg_inc2
			 + pp_earn.pe_high_inc + pp_earn.pe_vac);
		emp_uic_units =(pp_earn.pe_reg_units + pp_earn.pe_high_units);
		if((emp_uic_inc < uic_table.ui_min_earn) &&
		   (emp_uic_units < uic_table.ui_min_hrs)) {
		  pp_earn.pe_uic = 0;
		  pp_earn.pe_uic_ins = 0;

		  /* Verify if pay period is insurable.  If not then
		     review each individual week.	*/

		  if(pay_period.pp_numb != 52 && pay_period.pp_numb != 53){
		    week_min_earn = D_Roundoff(uic_table.ui_min_earn /
						 (double)no_weeks);
		    week_max_earn = D_Roundoff(uic_table.ui_max_earn /
						 (double)no_weeks);
		    week_min_hrs = uic_table.ui_min_hrs / (double)no_weeks;

		    for(i=0;i<5;i++){
			if(((pp_earn.pe_week_inc[i] + 
				pp_earn.pe_week_hinc[i]) >=
			    			week_min_earn) ||
			    ((pp_earn.pe_week_units[i] + 
				pp_earn.pe_week_hunits[i]) >=
						week_min_hrs)){
			    if((pp_earn.pe_week_inc[i] +
			        pp_earn.pe_week_hinc[i]) >
			    			week_max_earn){
		    		  week_inc = D_Roundoff(uic_table.ui_max_earn /
						 	(double)no_weeks);
			    } 
			    else{
		    		  week_inc = D_Roundoff(pp_earn.pe_week_inc[i]+ 
			    			pp_earn.pe_week_hinc[i]);
			    }
			    pp_earn.pe_uic += D_Roundoff(uic_rate * week_inc);
			    pp_earn.pe_uic_ins += D_Roundoff(week_inc);
			    pp_earn.pe_num_ins_wk++;
			}
		    }
		  }
		}
		else{
		    /* If pay period earnings are insurable	*/

		    if(emp_uic_inc > uic_table.ui_max_earn)
			emp_uic_inc = uic_table.ui_max_earn;
		    pp_earn.pe_uic = D_Roundoff(uic_rate * emp_uic_inc);
		    pp_earn.pe_uic_ins = D_Roundoff(emp_uic_inc);
		    pp_earn.pe_num_ins_wk = (short)no_weeks;
		}

		if(pp_earn.pe_uic != 0){
		    if((pp_earn.pe_uic + emp_uic_prem) > uic_table.ui_yrly_prem)
			pp_earn.pe_uic = D_Roundoff(uic_table.ui_yrly_prem -
						emp_uic_prem);
		    if((pp_earn.pe_uic_ins + tot_uic_ins) >
					 uic_table.ui_yrly_earn)
			pp_earn.pe_uic_ins = D_Roundoff(uic_table.ui_yrly_earn -
							 tot_uic_ins);
		    UIC_Emp_Share = D_Roundoff(gov_param.gp_uic_empl_rate *
							 pp_earn.pe_uic);
	  	    pp_earn.pe_uic_employer = UIC_Emp_Share;
	  	    pp_earn.pe_net -= pp_earn.pe_uic;
		    tmp_share = UIC_Emp_Share;
		    tmp_income = tot_income;
		  
		    for(i=0;i<5;i++){
			pp_earn.pe_week_uic[i] = D_Roundoff((week_income[i] /
				tot_income) * pp_earn.pe_uic);
		    }
		    /* Create journal entries for the employer shares */

		    strcpy(jr_ent.jr_emp_numb,emp_rec.em_numb);
		    strcpy(jr_ent.jr_type,"B");
		    jr_ent.jr_fund = 0;
		    jr_ent.jr_no = 0;
		    flg_reset(JR_ENT);

		    for( ; ; ){
		      retval = get_n_jr_ent(&jr_ent,BROWSE,2,FORWARD,e_mesg);
		      if(retval < 0){
			 if(retval == EFL) break;
			 DispErr(e_mesg);
	  		 retval = PrntRec(e_mesg);
			 roll_back(e_mesg);
			 seq_over(JR_ENT);
			 return(retval);
		      }
		      if(strcmp(jr_ent.jr_emp_numb,emp_rec.em_numb) != 0) 
				break;
		      if((strcmp(jr_ent.jr_type,"E") != 0) &&
		         (strcmp(jr_ent.jr_type,"X") != 0) &&
		         (strcmp(jr_ent.jr_type,"B") != 0)) 
				continue;

		      strcpy(last_type, jr_ent.jr_type);
		      last_fund = jr_ent.jr_fund;
		      last_jr = jr_ent.jr_no;

		      tot_amount = D_Roundoff(jr_ent.jr_amount);

		      /* this first read gets the rest of the keys needed
			 for the second glacct read */
		      gl_acct.gl_fund = jr_ent.jr_fund;
		      strcpy(gl_acct.gl_acct,jr_ent.jr_acct);
		      strcpy(gl_acct.gl_class,jr_ent.jr_class);
	
		      retval = get_glacct(&gl_acct,BROWSE,2,e_mesg);
		      if(retval < 0 && retval != UNDEF){
		 	   DispErr(e_mesg);
	  		   retval = PrntRec(e_mesg);
			   roll_back(e_mesg);
			   return(retval);
		      }
		      if(retval == UNDEF)
			continue;
		      strcpy(gl_acct.gl_type,"U");
		      strcpy(gl_acct.gl_earn,"UIC");
	
		      retval = get_glacct(&gl_acct,BROWSE,0,e_mesg);
		      if(retval < 0 && retval != UNDEF){
		 	   DispErr(e_mesg);
	  		   retval = PrntRec(e_mesg);
			   roll_back(e_mesg);
			   return(retval);
		      }
		      if(retval == UNDEF)
			continue;
		      seq_over(GLACCT);
		      retval = GetJr(gl_acct.gl_fund);
		      if(retval < 0) {
			   continue;
		      }
  
		      strcpy(jr_ent.jr_emp_numb,pp_earn.pe_numb);
		      jr_ent.jr_fund = gl_acct.gl_fund;
		      strcpy(jr_ent.jr_acct,gl_acct.gl_acct);
		      strcpy(jr_ent.jr_code,"UIC");
		      strcpy(jr_ent.jr_type,"S");
		      jr_ent.jr_amount = D_Roundoff((tmp_share * tot_amount)/
							 tmp_income);

		      tmp_share -= jr_ent.jr_amount;
		      tmp_income -= tot_amount;

		      if(jr_ent.jr_amount != 0.00) {
		      	  retval = put_jr_ent(&jr_ent,ADD,e_mesg);	
		       	  if(retval < 0) {
		 	     DispErr(e_mesg);
	  		     retval = PrntRec(e_mesg);
			     roll_back(e_mesg);
			     continue;
		       	  }
		          retval = commit(e_mesg) ;
	  	          if(retval < 0) {
		 		DispErr(e_mesg);
	  			retval = PrntRec(e_mesg);
				roll_back(e_mesg);
				seq_over(JR_ENT);
				continue;
		          }
		      }
		      strcpy(jr_ent.jr_emp_numb,emp_rec.em_numb);
		      strcpy(jr_ent.jr_type,last_type);
		      jr_ent.jr_fund = last_fund;
		      jr_ent.jr_no = last_jr + 1;

		      flg_reset(JR_ENT);
	  	    }
		    seq_over(JR_ENT);

		    /* Create journal entry for liability	*/

		    jr_ent.jr_fund = barg_unit.b_fund;
		    retval = GetJr(barg_unit.b_fund);
		    if(retval < 0) {
			 continue;
		    }
		    jr_ent.jr_fund = barg_unit.b_fund;
		    strcpy(jr_ent.jr_acct,barg_unit.b_uic_acct);

		    jr_ent.jr_amount = D_Roundoff(pp_earn.pe_uic +
						UIC_Emp_Share);
		    strcpy(jr_ent.jr_emp_numb,pp_earn.pe_numb);
		    strcpy(jr_ent.jr_code,"UIC");
		    strcpy(jr_ent.jr_type,"D");

		    if(jr_ent.jr_amount != 0.00) {
			jr_ent.jr_amount = jr_ent.jr_amount * -1;
			retval = put_jr_ent(&jr_ent,ADD,e_mesg);	
			if(retval < 0) {
		 	 	DispErr(e_mesg);
	  			retval = PrntRec(e_mesg);
				roll_back(e_mesg);
				continue;
			}
		    	retval = commit(e_mesg) ;
	  	    	if(retval < 0) {
		 		DispErr(e_mesg);
	  			retval = PrntRec(e_mesg);
				roll_back(e_mesg);
				continue;
		    	}
		    }
		}
	  }
	  /*********************************************************/
	  /* Calculate Registered Pension Plan			*/

	  pp_earn.pe_reg1 = 0;
	  pp_earn.pe_reg2 = 0;
	  pp_earn.pe_reg3 = 0;
	  pp_earn.pe_reg_prior = 0;
	  pp_earn.pe_reg_opt = 0;
	  pp_earn.pe_reg_nonm = 0;
	  tot_reg_pen = 0;
	  if(emp_rec.em_reg_pen[0] != NULL){
#ifdef ENGLISH
	    printf("\tCalculating Registered Pension Plan for: %s\n",
		pp_earn.pe_numb);
#else
#endif

	    strcpy(reg_pen.rg_code,emp_rec.em_reg_pen);
	    strcpy(reg_pen.rg_pp_code,barg_unit.b_pp_code);

	    retval = get_reg_pen(&reg_pen,BROWSE,0,e_mesg);
	    if(retval < 0 && retval != UNDEF){
		DispErr(e_mesg);
	  	retval = PrntRec(e_mesg);
		roll_back(e_mesg);
		return(retval);
	    }

	    if(retval != UNDEF){
		if(strcmp(reg_pen.rg_ded_pp[pay_per_it.ppi_mthly-1],"Y") == 0){
		  if(emp_inc >= reg_pen.rg_min_earn){
		    if(strcmp(reg_pen.rg_type,"1A") == 0)
		      pp_earn.pe_reg1 = D_Roundoff((pp_earn.pe_reg_inc1 +
				pp_earn.pe_reg_inc2 + pp_earn.pe_vac) 
				* reg_pen.rg_perc1 / 100); 
		    if(strcmp(reg_pen.rg_type,"1B") == 0)
			  pp_earn.pe_reg1 = D_Roundoff((pp_earn.pe_reg_inc1 +
				pp_earn.pe_reg_inc2 + pp_earn.pe_high_inc +
				pp_earn.pe_vac) * reg_pen.rg_perc1 / 100); 

		    if((strcmp(reg_pen.rg_type,"1C") == 0) ||
			 (strcmp(reg_pen.rg_type,"1D") == 0))
			  pp_earn.pe_reg1 = D_Roundoff((pp_earn.pe_reg_inc1 +
				 pp_earn.pe_reg_inc2 + pp_earn.pe_high_inc +
				 pp_earn.pe_ben) * reg_pen.rg_perc1 / 100); 

		    if(strcmp(reg_pen.rg_type,"2A") == 0)
			  pp_earn.pe_reg1 = D_Roundoff((pp_earn.pe_reg_inc1 +
				 pp_earn.pe_reg_inc2 - pp_earn.pe_cpp) *
				 reg_pen.rg_perc1 / 100); 
		
		    if(strcmp(reg_pen.rg_type,"2B") == 0)
			  pp_earn.pe_reg1 = D_Roundoff((pp_earn.pe_reg_inc1 +
				 pp_earn.pe_reg_inc2 +
				 pp_earn.pe_high_inc + pp_earn.pe_ben -
				 pp_earn.pe_cpp) * reg_pen.rg_perc1 / 100);

		    if(strcmp(reg_pen.rg_type,"3A") == 0){
			if((pp_earn.pe_reg_inc1 + pp_earn.pe_reg_inc2) >
				 reg_pen.rg_amount){
			    pp_earn.pe_reg2 = D_Roundoff((pp_earn.pe_reg_inc1 +
				 pp_earn.pe_reg_inc2 -
				 reg_pen.rg_amount) * reg_pen.rg_perc2 / 100);
			    pp_earn.pe_reg1 = D_Roundoff(reg_pen.rg_amount *
				reg_pen.rg_perc1 / 100);
		    	}
		    	else
			    pp_earn.pe_reg1 = D_Roundoff((pp_earn.pe_reg_inc1 +
				 pp_earn.pe_reg_inc2) *	reg_pen.rg_perc1 / 100); 
		    }
		    if(strcmp(reg_pen.rg_type,"4A") == 0)
			pp_earn.pe_reg1 = D_Roundoff(reg_pen.rg_amount);

		    if(strcmp(reg_pen.rg_type,"5A") == 0){

		    if((pp_earn.pe_reg_inc1 + pp_earn.pe_reg_inc2 +
			pp_earn.pe_vac + emp_cpp_pen) > reg_pen.rg_amount2){
			if((reg_pen.rg_amount2 - emp_cpp_pen) > 0){
			   pp_earn.pe_reg2 = D_Roundoff((reg_pen.rg_amount2 - 
						emp_cpp_pen)
				 		* (reg_pen.rg_perc2/100));
			   pp_earn.pe_reg3 = D_Roundoff((pp_earn.pe_reg_inc1 + 
					pp_earn.pe_reg_inc2 + pp_earn.pe_vac +
					emp_cpp_pen - reg_pen.rg_amount2)
					* (reg_pen.rg_perc3/100));
			}
			else
			   pp_earn.pe_reg3 = D_Roundoff((pp_earn.pe_reg_inc1 + 
					pp_earn.pe_reg_inc2 + pp_earn.pe_vac) 
					* (reg_pen.rg_perc3/100));
		    }
		    else{
			if((pp_earn.pe_reg_inc1 + pp_earn.pe_reg_inc2 +
			    emp_cpp_pen + pp_earn.pe_vac) > reg_pen.rg_amount){
				if((reg_pen.rg_amount - emp_cpp_pen) > 0){
				   pp_earn.pe_reg1 = D_Roundoff((
						      reg_pen.rg_amount - 
						      emp_cpp_pen) *
				  		     (reg_pen.rg_perc1/100));
				   pp_earn.pe_reg2 = D_Roundoff((
					pp_earn.pe_reg_inc1 +
					pp_earn.pe_reg_inc2 +
					pp_earn.pe_vac + emp_cpp_pen -
					reg_pen.rg_amount) *
					(reg_pen.rg_perc2/100));
				}
				else
				    pp_earn.pe_reg2 = D_Roundoff((
					pp_earn.pe_reg_inc1 +
					pp_earn.pe_reg_inc2 + pp_earn.pe_vac)
					* (reg_pen.rg_perc2/100));
			}
			else
				pp_earn.pe_reg1 = D_Roundoff((
					pp_earn.pe_reg_inc1 +
					pp_earn.pe_reg_inc2 + pp_earn.pe_vac)
					* (reg_pen.rg_perc1/100)); 
			
		  }

		  }
		  if(strcmp(reg_pen.rg_type,"3A") != 0){
		    if(D_Roundoff(emp_reg_contr + pp_earn.pe_reg1) >
		       reg_pen.rg_max_contr)
		       pp_earn.pe_reg1 = D_Roundoff(reg_pen.rg_max_contr -
				 emp_reg_contr);
		    else{
		       if(D_Roundoff(emp_reg_contr + pp_earn.pe_reg1 +
				 pp_earn.pe_reg2) > reg_pen.rg_max_contr)
			   pp_earn.pe_reg1 = D_Roundoff(reg_pen.rg_max_contr -
				 emp_reg_contr);

/*		       if(pp_earn.pe_reg1 > reg_pen.rg_amount){
			   pp_earn.pe_reg2 = D_Roundoff((pp_earn.pe_reg_inc1 +
				 pp_earn.pe_reg_inc2 -
				 reg_pen.rg_amount) * reg_pen.rg_perc2 / 100);
			   pp_earn.pe_reg1 = reg_pen.rg_amount;
		       } */
		    }
		  }
		}
	      }
	    }
	    tot_reg_pen = D_Roundoff(pp_earn.pe_reg1 + pp_earn.pe_reg2 +
					 pp_earn.pe_reg3);
	    if(strncmp(reg_pen.rg_code, "PSPP",4) == 0){
			tot_reg_pen += D_Roundoff(emp_rec.em_reg_prior +
				emp_rec.em_reg_opt);
			pp_earn.pe_reg_prior = emp_rec.em_reg_prior;
			pp_earn.pe_reg_opt = emp_rec.em_reg_opt;
	    }

	    empr_share = D_Roundoff((tot_reg_pen / ((100 - 
			reg_pen.rg_employer_sh) / 100)) - tot_reg_pen);

	    if(strncmp(reg_pen.rg_code, "PSPP",4) == 0){
		tot_reg_pen += D_Roundoff(emp_rec.em_reg_nonm);
		pp_earn.pe_reg_nonm = emp_rec.em_reg_nonm;
	    }
	    pp_earn.pe_net -= tot_reg_pen;
	    for(i=0;i<5;i++){
		pp_earn.pe_week_reg1[i] =
			D_Roundoff((week_income[i] / tot_income) *
			pp_earn.pe_reg1);
		pp_earn.pe_week_reg2[i] =
			D_Roundoff((week_income[i] / tot_income) *
			pp_earn.pe_reg2);
		pp_earn.pe_week_reg3[i] =
			D_Roundoff((week_income[i] / tot_income) *
			pp_earn.pe_reg3);
		pp_earn.pe_wk_reg_pr[i] =
			D_Roundoff((week_income[i] / tot_income) *
			pp_earn.pe_reg_prior);
		pp_earn.pe_wk_reg_opt[i] =
			D_Roundoff((week_income[i] / tot_income) *
			pp_earn.pe_reg_opt);
		pp_earn.pe_wk_reg_nonm[i] =
			D_Roundoff((week_income[i] / tot_income) *
			pp_earn.pe_reg_nonm);
	    }

	    tmp_share = empr_share;
	    tmp_income = tot_income;
	
	    /* Create journal entries for employer share	*/

	    strcpy(jr_ent.jr_emp_numb,emp_rec.em_numb);
	    strcpy(jr_ent.jr_type,"B");
	    jr_ent.jr_fund = 0;
	    jr_ent.jr_no = 0;
		
	    flg_reset(JR_ENT);
	    for( ; ; ){
	      retval = get_n_jr_ent(&jr_ent,BROWSE,2,FORWARD,e_mesg);
	      if(retval < 0){
		if(retval == EFL) break;
		DispErr(e_mesg);
	  	retval = PrntRec(e_mesg);
		roll_back(e_mesg);
		seq_over(JR_ENT);
		return(retval);
	      }
	      if(strcmp(jr_ent.jr_emp_numb,emp_rec.em_numb) != 0)
			break;
	      if((strcmp(jr_ent.jr_type,"E") != 0) &&
	         (strcmp(jr_ent.jr_type,"X") != 0) &&
	         (strcmp(jr_ent.jr_type,"B") != 0))
			continue;

	      strcpy(last_type,jr_ent.jr_type);
	      last_fund = jr_ent.jr_fund;
	      last_jr = jr_ent.jr_no;

	      tot_amount = D_Roundoff(jr_ent.jr_amount);

	      gl_acct.gl_fund = jr_ent.jr_fund;
	      strcpy(gl_acct.gl_acct,jr_ent.jr_acct);
	      strcpy(gl_acct.gl_class,jr_ent.jr_class);
	
	      retval = get_glacct(&gl_acct,BROWSE,2,e_mesg);
	      if(retval < 0 && retval != UNDEF){
		DispErr(e_mesg);
	  	retval = PrntRec(e_mesg);
		roll_back(e_mesg);
		return(retval);
	      }
	      if(retval == UNDEF)
		continue;
	      strcpy(gl_acct.gl_type,"R");
	      strcpy(gl_acct.gl_earn,reg_pen.rg_code);
	
	      retval = get_glacct(&gl_acct,BROWSE,0,e_mesg);
	      if(retval < 0 && retval != UNDEF){
		DispErr(e_mesg);
	  	retval = PrntRec(e_mesg);
		roll_back(e_mesg);
		return(retval);
	      }
	      if(retval == UNDEF)
		continue;
	      seq_over(GLACCT);
	      retval = GetJr(gl_acct.gl_fund);
	      if(retval < 0) {
		continue;
	      }
	      strcpy(jr_ent.jr_emp_numb,pp_earn.pe_numb);
	      jr_ent.jr_fund = gl_acct.gl_fund;
	      strcpy(jr_ent.jr_acct,gl_acct.gl_acct);
	      strcpy(jr_ent.jr_code,emp_rec.em_reg_pen);
	      strcpy(jr_ent.jr_type,"S");
	      jr_ent.jr_amount = D_Roundoff((tmp_share * tot_amount) /
							 tmp_income);

	      tmp_share -= jr_ent.jr_amount;
	      tmp_income -= tot_amount;

	      if(jr_ent.jr_amount != 0.00) {
		retval = put_jr_ent(&jr_ent,ADD,e_mesg);	
		if(retval < 0) {
		   DispErr(e_mesg);
	  	   retval = PrntRec(e_mesg);
		   roll_back(e_mesg);
		   continue;
		}
	        retval = commit(e_mesg) ;
	        if(retval < 0) {
		   DispErr(e_mesg);
	  	   retval = PrntRec(e_mesg);
		   roll_back(e_mesg);
		   continue;
	        }
	      }
	      strcpy(jr_ent.jr_emp_numb,emp_rec.em_numb);
	      strcpy(jr_ent.jr_type,last_type);
	      jr_ent.jr_fund = last_fund;
	      jr_ent.jr_no = last_jr + 1;

	      flg_reset(JR_ENT);
	    }
	    seq_over(JR_ENT);

	    /* Create journal entry for the liability	*/

	    jr_ent.jr_fund = reg_pen.rg_fund;
	    retval = GetJr(reg_pen.rg_fund);
	    if(retval < 0) {
	       continue;
	    }
	    jr_ent.jr_fund = reg_pen.rg_fund;
	    strcpy(jr_ent.jr_acct,reg_pen.rg_lia_acct);

	    jr_ent.jr_amount = D_Roundoff(tot_reg_pen + empr_share);
	    strcpy(jr_ent.jr_emp_numb,pp_earn.pe_numb);
	    strcpy(jr_ent.jr_code,emp_rec.em_reg_pen);
	    strcpy(jr_ent.jr_type,"D");

	    if(jr_ent.jr_amount != 0.00) {
		jr_ent.jr_amount = jr_ent.jr_amount * -1;
		retval = put_jr_ent(&jr_ent,ADD,e_mesg);	
		if(retval < 0) {
	       		DispErr(e_mesg);
	      		retval = PrntRec(e_mesg);
	      		roll_back(e_mesg);
	     		continue;
		}
	    	retval = commit(e_mesg) ;
	    	if(retval < 0){
	       		DispErr(e_mesg);
	      		retval = PrntRec(e_mesg);
	      		roll_back(e_mesg);
	     		continue;
	    	}
	    }
	  }

	  /*********************************************************/
	  /* Calculate Income Tax	*/

	  if(emp_rec.em_tax_exp[0] == U_NO){

#ifdef ENGLISH
		printf("\tCalculating Income Tax for: %s\n",pp_earn.pe_numb);
#else
#endif

		strcpy(emp_ded.ed_numb,emp_rec.em_numb);
		strcpy(emp_ded.ed_code,"UDUES");
		strcpy(emp_ded.ed_group,"UDUES");
		retval = get_emp_ded(&emp_ded,BROWSE,0,e_mesg);
		if(retval < 0 && retval != UNDEF) {
	       	   DispErr(e_mesg);
	      	   retval = PrntRec(e_mesg);
	      	   roll_back(e_mesg);
	     	   return(retval);
		}
		if(retval == UNDEF )
			Union = 0;
		else 
	  	 	Union = D_Roundoff((pp_earn.pe_reg_inc1 +
			      pp_earn.pe_reg_inc2 + pp_earn.pe_vac) * 0.011); 

		/* Step 1	*/
 		Ann_Tax_Inc = (((double)pay_period.pp_numb *
			 (tot_income - tot_reg_pen - Union)) -
			 emp_rec.em_ho_ded - emp_rec.em_ann_ded);

		tax_table.tx_date = sysdate;
		tax_table.tx_low_amnt = HV_DOUBLE;
		tax_table.tx_high_amnt = HV_DOUBLE;
		flg_reset(TAX);

		for( ; ; ){
			retval = get_n_tax(&tax_table,BROWSE,0,BACKWARD,e_mesg);
			if(retval < 0){
	       	   		DispErr(e_mesg);
	      	  		retval = PrntRec(e_mesg);
	      			roll_back(e_mesg);
				seq_over(TAX);
	   		  	continue;
			}
			if(Ann_Tax_Inc >= tax_table.tx_low_amnt && 
			    Ann_Tax_Inc < tax_table.tx_high_amnt){
				break;
				
			}
		}
		seq_over(TAX);

		/* Step 2	*/
		Net_Pers_Tc = (gov_param.gp_net_ptcr * emp_rec.em_net_tax_cr); 

		CPP_UIC_Tc_temp1 = (gov_param.gp_cpp_incr *
			((double)pay_period.pp_numb * pp_earn.pe_cpp)) ;
		if(CPP_UIC_Tc_temp1 > gov_param.gp_cpp_incm)
			CPP_UIC_Tc_temp1 = (gov_param.gp_cpp_incm);
		CPP_UIC_Tc_temp2 = (gov_param.gp_uic_incr *
			((double)pay_period.pp_numb * pp_earn.pe_uic));
		if(CPP_UIC_Tc_temp2 > gov_param.gp_uic_incm)
			CPP_UIC_Tc_temp2 = (gov_param.gp_uic_incm);

		CPP_UIC_Tc = (CPP_UIC_Tc_temp1 + CPP_UIC_Tc_temp2);
		Oth_Fed_Tc = emp_rec.em_other_fed;

		/* Step 3	*/
		Ann_Bas_Fed_Tax = ((tax_table.tx_rate * Ann_Tax_Inc) -
			tax_table.tx_fed_const - Net_Pers_Tc -
			CPP_UIC_Tc - Oth_Fed_Tc); 

		if(Ann_Bas_Fed_Tax < 0)
			Ann_Bas_Fed_Tax = 0;

		/* Step 4	*/
		if(Ann_Bas_Fed_Tax > gov_param.gp_fed_sur_lim)
			Fed_Surtax = ((gov_param.gp_fed_sur1 *
			Ann_Bas_Fed_Tax) + (gov_param.gp_fed_sur2 *
			(Ann_Bas_Fed_Tax - gov_param.gp_fed_sur_lim)));
		else
			Fed_Surtax = (gov_param.gp_fed_sur1 * Ann_Bas_Fed_Tax); 
			
		Fed_Tax_Ded = (Ann_Bas_Fed_Tax + Fed_Surtax);

		/* Step 5	*/
		if(strcmp(pay_param.pr_prov,"NS") == 0){
		  if((gov_param.gp_prov_tax_rate *
		  	gov_param.gp_fed_sur_lim) <=gov_param.gp_prov_sur_lim)
			gov_param.gp_prov_sur = 0;
		  else
		  	gov_param.gp_prov_sur = (gov_param.gp_prov_sur_rate * 
			((gov_param.gp_prov_tax_rate * Ann_Bas_Fed_Tax) -
			gov_param.gp_prov_sur_lim));
		
		}
		Prov_Inc_Tax = ((gov_param.gp_prov_tax_rate *
			 Ann_Bas_Fed_Tax) + gov_param.gp_prov_sur +
			 gov_param.gp_prov_flat1 + gov_param.gp_prov_flat2 +
			 gov_param.gp_prov_net - gov_param.gp_tax_red);

		/* Step 6	*/
		pp_earn.pe_tax = D_Roundoff(((Fed_Tax_Ded + Prov_Inc_Tax) /
			(double)pay_period.pp_numb) + emp_rec.em_inc_tax); 

	  	pp_earn.pe_net -= pp_earn.pe_tax;

		for(i=0;i<5;i++){
			pp_earn.pe_week_tax[i] =
				D_Roundoff((week_income[i] / tot_income) *
				pp_earn.pe_tax);
		}
/*** Louis - Used to find problem with income tax
printf("\n\tincome tax: %lf\n",pp_earn.pe_tax);
printf("\n\tFed. inc. tax: %lf\n",Fed_Tax_Ded);
printf("\n\tProv. inc. tax: %lf\n",Prov_Inc_Tax);
printf("\n\tEmp. record inc. tax: %lf\n",emp_rec.em_inc_tax);
printf("\n\tPay period numb.: %d\n",pay_period.pp_numb);
get();
*****/

		/* There is no employer share for income tax.	*/
		/* Therefore there is no journal entries.	*/
		/* Create journal entry for the liability	*/

		jr_ent.jr_fund = barg_unit.b_fund;
		retval = GetJr(barg_unit.b_fund);
		if(retval < 0) {
	   	  	continue;
		}
		jr_ent.jr_fund = barg_unit.b_fund;
		strcpy(jr_ent.jr_acct,barg_unit.b_tax_acct);

		jr_ent.jr_amount = D_Roundoff(pp_earn.pe_tax);
		strcpy(jr_ent.jr_emp_numb,pp_earn.pe_numb);
		strcpy(jr_ent.jr_code,"TAX");
		strcpy(jr_ent.jr_type,"D");

		if(jr_ent.jr_amount != 0.00) {
		  jr_ent.jr_amount = jr_ent.jr_amount * -1;
		  retval = put_jr_ent(&jr_ent,ADD,e_mesg);	
		  if(retval < 0) {
	       	  	DispErr(e_mesg);
	      	  	retval = PrntRec(e_mesg);
	      		roll_back(e_mesg);
	   	  	continue;
		  }
		  retval = commit(e_mesg) ;
	  	  if(retval < 0) {
	       	  	DispErr(e_mesg);
	      	  	retval = PrntRec(e_mesg);
	      		roll_back(e_mesg);
	   	  	continue;
		  }
		}
	  }

	  /*********************************************************/
	  /* Calculate Garnishments			*/

#ifdef ENGLISH
	printf("\tCalculating Deductions, Garnishments for: %s\n",
		pp_earn.pe_numb);
#else
#endif

	  strcpy(emp_garn.eg_numb,emp_rec.em_numb);
	  emp_garn.eg_pr_cd = 0;
	  emp_garn.eg_seq = 0;
	  flg_reset(EMP_GARN);

	  for( ; ; ){
		  retval = get_n_emp_garn(&emp_garn,BROWSE,0,FORWARD,e_mesg);
		  if(retval < 0){
			if(retval == EFL) break;
	       	  	DispErr(e_mesg);
	      	  	retval = PrntRec(e_mesg);
	      		roll_back(e_mesg);
			seq_over(EMP_GARN);
	   	  	continue;
		  }
		  
		  if(strcmp(emp_garn.eg_numb,emp_rec.em_numb) != 0)
			break;
		  if(strcmp(emp_garn.eg_ded_pp[pay_per_it.ppi_mthly - 1],
								"Y") == 0){
		    if(emp_garn.eg_amnt_out != 0){
		      if(strcmp(emp_garn.eg_amt_flg,"P") == 0)
			  pp_garn.pg_amount = D_Roundoff(tot_income *
				 emp_garn.eg_pp_amount);
		      else{
			/* Added check here for payment amount */
			if(emp_garn.eg_amnt_out < emp_garn.eg_pp_amount)
			  pp_garn.pg_amount = D_Roundoff(emp_garn.eg_amnt_out);
			else
			  pp_garn.pg_amount = D_Roundoff(emp_garn.eg_pp_amount);
		      }

		      temppe_net = D_Roundoff(pp_earn.pe_net -
					 pp_garn.pg_amount);
		      if(temppe_net < emp_garn.eg_min_tresh)
			pp_garn.pg_amount = D_Roundoff(pp_earn.pe_net -
					 emp_garn.eg_min_tresh);

	  	      pp_earn.pe_net -= pp_garn.pg_amount;

		      /* Create journal entry for the liability	*/

		      jr_ent.jr_fund = emp_garn.eg_fund;
		      retval = GetJr(emp_garn.eg_fund);
		      if(retval < 0) {
	   	  	continue;
		      }
		      jr_ent.jr_fund = emp_garn.eg_fund;
		      strcpy(jr_ent.jr_acct,emp_garn.eg_lia_acct);
		      strcpy(jr_ent.jr_type,"D");

		      jr_ent.jr_amount = D_Roundoff(pp_garn.pg_amount);
		      strcpy(jr_ent.jr_emp_numb,pp_earn.pe_numb);
		      strcpy(jr_ent.jr_code,"GARN");

		      if(jr_ent.jr_amount != 0.00) {
			jr_ent.jr_amount = jr_ent.jr_amount * -1;
			retval = put_jr_ent(&jr_ent,ADD,e_mesg);	
			if(retval < 0) {
	       	  		DispErr(e_mesg);
	      	  		retval = PrntRec(e_mesg);
	      			roll_back(e_mesg);
	   	  		continue;
			}
		  	retval = commit(e_mesg) ;
	  	 	if(retval < 0) {
	       	  		DispErr(e_mesg);
	      	  		retval = PrntRec(e_mesg);
	      			roll_back(e_mesg);
	   	  		continue;
		  	}
		      }

		      strcpy(pp_garn.pg_numb,pp_earn.pe_numb);
		      pp_garn.pg_pr_cd = emp_garn.eg_pr_cd;
		      pp_garn.pg_seq = emp_garn.eg_seq;
		      pp_garn.pg_pp = pp_earn.pe_pp;
		      pp_garn.pg_fund = emp_garn.eg_fund;
		      strcpy(pp_garn.pg_acct,emp_garn.eg_lia_acct);
		      pp_garn.pg_date = pp_earn.pe_date;

		      retval = put_pp_garn(&pp_garn,ADD,e_mesg) ;
		      if(retval < 0) {
	       	  	DispErr(e_mesg);
	      	  	retval = PrntRec(e_mesg);
	      		roll_back(e_mesg);
	   	  	continue;
		      }
		      retval = commit(e_mesg) ;
	  	      if(retval < 0) {
	       	  	DispErr(e_mesg);
	      	  	retval = PrntRec(e_mesg);
	      		roll_back(e_mesg);
	   	  	continue;
		      }
		    }
		  }
	  }
	  seq_over(EMP_GARN);
	  close_file(EMP_GARN);

	  /*********************************************************/
	  /* Calculate Deductions  			*/

	  strcpy(emp_ded.ed_numb,emp_rec.em_numb);
	  emp_ded.ed_code[0] = '\0';
	  emp_ded.ed_group[0] = '\0';
	  flg_reset(EMP_DED);

	  for( ; ; ){
		retval = get_n_emp_ded(&emp_ded,BROWSE,0,FORWARD,e_mesg);
		if(retval < 0){
			if(retval == EFL) break;
	       	  	DispErr(e_mesg);
	      	  	retval = PrntRec(e_mesg);
	      		roll_back(e_mesg);
			seq_over(EMP_DED);
			return(retval);
		}
		if(strcmp(emp_ded.ed_numb,emp_rec.em_numb) != 0)
			break;

		emp_ded_contr = 0;
		  
		strcpy(emp_dd_his.edh_numb,pp_earn.pe_numb);
		emp_dd_his.edh_pp = '\0';
		emp_dd_his.edh_date = '\0';
		strcpy(emp_dd_his.edh_code,emp_ded.ed_code);
		strcpy(emp_dd_his.edh_group,emp_ded.ed_group);
		flg_reset(EMP_DED_HIS);

		for( ; ; ){
		    err = get_n_emp_dhis(&emp_dd_his,BROWSE,0,FORWARD,e_mesg) ;
		    if(err < 0) {
			if(err == EFL) break;
	       	  	DispErr(e_mesg);
	      	  	retval = PrntRec(e_mesg);
	      		roll_back(e_mesg);
			seq_over(EMP_DED_HIS);
	   	  	continue;
		    }
		    if(strcmp(emp_dd_his.edh_numb,pp_earn.pe_numb) != 0) 
			break;
		    if(strcmp(emp_dd_his.edh_code,emp_ded.ed_code) != 0 &&
		       strcmp(emp_dd_his.edh_group,emp_ded.ed_group) != 0)
			continue;
		    emp_ded_contr += D_Roundoff(emp_dd_his.edh_amount);
		}
		seq_over(EMP_DED_HIS);
		close_file(EMP_DED_HIS);

		strcpy(deduction.dd_code,emp_ded.ed_code);
		strcpy(deduction.dd_pp_code,barg_unit.b_pp_code);

		retval = get_deduction(&deduction,BROWSE,0,e_mesg);
		if(retval < 0){
	       	  	DispErr(e_mesg);
	      	  	retval = PrntRec(e_mesg);
	      		roll_back(e_mesg);
	   	  	continue;
		}

		if(strcmp(deduction.dd_ded_pp[pay_per_it.ppi_mthly - 1],
								"Y") == 0){
		    strcpy(ded_grp.dg_code,emp_ded.ed_code);
		    strcpy(ded_grp.dg_pp_code,barg_unit.b_pp_code);
		    strcpy(ded_grp.dg_group,emp_ded.ed_group);

		    retval = get_ded_grp(&ded_grp,BROWSE,0,e_mesg);
		    if(retval < 0){
	       	  	DispErr(e_mesg);
	      	  	retval = PrntRec(e_mesg);
	      		roll_back(e_mesg);
	   	  	continue;
		    }

		    strcpy(pp_ded.pd_numb,pp_earn.pe_numb);
		    pp_ded.pd_pp = pp_earn.pe_pp;
		    pp_ded.pd_date = pp_earn.pe_date;
		    strcpy(pp_ded.pd_code,emp_ded.ed_code);
		    strcpy(pp_ded.pd_group,emp_ded.ed_group);
		    pp_ded.pd_fund = deduction.dd_fund;
		    strcpy(pp_ded.pd_acct,deduction.dd_lia_acct);
		  	
		    retval = get_pp_ded(&pp_ded,BROWSE,0,e_mesg);
		    if(retval < 0 && retval != UNDEF){
	       	  	DispErr(e_mesg);
	      	  	retval = PrntRec(e_mesg);
	      		roll_back(e_mesg);
	   	  	continue;
		    }

		    if(emp_inc >= deduction.dd_min_earn){
		  
			/* Percentage or full amount */
		      if(strcmp(ded_grp.dg_amt_flag,"P") == 0){

			/* Temporary Union Calc */
		/** Coment this out so all percentages are on Regular income
			if(strcmp(emp_ded.ed_code, "UDUES") == 0)
	  	  	  pp_ded.pd_amount = D_Roundoff((pp_earn.pe_reg_inc1 +
				pp_earn.pe_vac + pp_earn.pe_reg_inc2) *
				ded_grp.dg_amount/100.0);  
			else
			  pp_ded.pd_amount = D_Roundoff(tot_income *
				ded_grp.dg_amount/100.0);
		*** Louis Sept 1997 **/
			/* Following was cut from above if */
	  	  	  pp_ded.pd_amount = D_Roundoff((pp_earn.pe_reg_inc1 +
				pp_earn.pe_vac + pp_earn.pe_reg_inc2) *
				ded_grp.dg_amount/100.0);  
		      }
		      else
			pp_ded.pd_amount = D_Roundoff(emp_ded.ed_amount);
		    }
		    if((emp_ded_contr+pp_ded.pd_amount) >
			deduction.dd_max_contr)
		        pp_ded.pd_amount = D_Roundoff(deduction.dd_max_contr -
					 emp_ded_contr);

	  	    pp_earn.pe_net -= pp_ded.pd_amount;

		    retval = put_pp_ded(&pp_ded,ADD,e_mesg) ;
		    if(retval < 0) {
	       	  	DispErr(e_mesg);
	      	  	retval = PrntRec(e_mesg);
	      		roll_back(e_mesg);
	   	  	continue;
		    }
		    retval = commit(e_mesg) ;
	  	    if(retval < 0) {
	       	  	DispErr(e_mesg);
	      	  	retval = PrntRec(e_mesg);
	      		roll_back(e_mesg);
	   	  	continue;
		    }
		}
		empr_share = D_Roundoff((pp_ded.pd_amount / ((100 - 
			ded_grp.dg_employer_sh) / 100)) - pp_ded.pd_amount);

		tmp_income = tot_income;
		tmp_share = empr_share;

		/* Create journal entries for the employer share */

		if(empr_share != 0) {
		  strcpy(jr_ent.jr_emp_numb,emp_rec.em_numb);
		  strcpy(jr_ent.jr_type,"B");
		  jr_ent.jr_fund = 0;
		  jr_ent.jr_no = 0;
		
		  flg_reset(JR_ENT);
		  for( ; ; ){
		    retval = get_n_jr_ent(&jr_ent,BROWSE,2,FORWARD,e_mesg);
		    if(retval < 0){
			if(retval == EFL) break;
	       	  	DispErr(e_mesg);
	      	  	retval = PrntRec(e_mesg);
	      		roll_back(e_mesg);
			seq_over(JR_ENT);
	   	  	continue;
		    }
		    if(strcmp(jr_ent.jr_emp_numb,emp_rec.em_numb) != 0)
			break;

		    if((strcmp(jr_ent.jr_type,"E") != 0) &&
		       (strcmp(jr_ent.jr_type,"X") != 0) &&
		       (strcmp(jr_ent.jr_type,"B") != 0))
			continue;	

		    strcpy(last_type,jr_ent.jr_type);
		    last_fund = jr_ent.jr_fund;
		    last_jr = jr_ent.jr_no;

		    tot_amount = D_Roundoff(jr_ent.jr_amount);

		    gl_acct.gl_fund = jr_ent.jr_fund;
		    strcpy(gl_acct.gl_acct,jr_ent.jr_acct);
		    strcpy(gl_acct.gl_class,jr_ent.jr_class);
	
		    retval = get_glacct(&gl_acct,BROWSE,2,e_mesg);
		    if(retval < 0){
	       	  	DispErr(e_mesg);
	      	  	retval = PrntRec(e_mesg);
	      		roll_back(e_mesg);
	   	  	continue;
		    }
		    strcpy(gl_acct.gl_type,"D");
		    strcpy(gl_acct.gl_earn,deduction.dd_code);
	
		    retval = get_glacct(&gl_acct,BROWSE,0,e_mesg);
		    if(retval < 0){
	       	  	DispErr(e_mesg);
	      	  	retval = PrntRec(e_mesg);
	      		roll_back(e_mesg);
	   	  	continue;
		    }
		    seq_over(GLACCT);
		    retval = GetJr(gl_acct.gl_fund);
		    if(retval < 0) {
	   	  	continue;
		    }

		    strcpy(jr_ent.jr_emp_numb,emp_rec.em_numb);
		    jr_ent.jr_fund = gl_acct.gl_fund;
		    strcpy(jr_ent.jr_acct,gl_acct.gl_acct);
		    strcpy(jr_ent.jr_code,emp_ded.ed_code);
		    strcpy(jr_ent.jr_type,"S");

		    jr_ent.jr_amount = D_Roundoff((tmp_share * tot_amount) /
							 tmp_income);
		    tmp_share -= jr_ent.jr_amount;
		    tmp_income -= tot_amount;

		    if(jr_ent.jr_amount != 0.00) {
			retval = put_jr_ent(&jr_ent,ADD,e_mesg);	
			if(retval < 0) {
	       	  		DispErr(e_mesg);
	      	  		retval = PrntRec(e_mesg);
	      			roll_back(e_mesg);
	   	  		continue;
			}
		    	retval = commit(e_mesg) ;
	  	    	if(retval < 0) {
	       	  		DispErr(e_mesg);
	      	  		retval = PrntRec(e_mesg);
	      			roll_back(e_mesg);
	   	  		continue;
		    	}
		    }	
		    strcpy(jr_ent.jr_emp_numb,emp_rec.em_numb);
		    strcpy(jr_ent.jr_type,last_type);
		    jr_ent.jr_fund = last_fund;
		    jr_ent.jr_no = last_jr + 1;

		    flg_reset(JR_ENT);
		  }
		  seq_over(JR_ENT);

		}

		/* Create journal entry for liability	*/

		jr_ent.jr_fund = deduction.dd_fund;
		retval = GetJr(deduction.dd_fund);
		if(retval < 0) {
	   	  	continue;
	  	}
		jr_ent.jr_fund = deduction.dd_fund;
		strcpy(jr_ent.jr_acct,deduction.dd_lia_acct);

		jr_ent.jr_amount = D_Roundoff(pp_ded.pd_amount + empr_share);
		strcpy(jr_ent.jr_emp_numb,pp_earn.pe_numb);
		strcpy(jr_ent.jr_code,emp_ded.ed_code);
		strcpy(jr_ent.jr_type,"D");

		if(jr_ent.jr_amount != 0.00) {
			jr_ent.jr_amount = jr_ent.jr_amount * -1;
			retval = put_jr_ent(&jr_ent,ADD,e_mesg);	
			if(retval < 0) {
	       	  		DispErr(e_mesg);
	      	  		retval = PrntRec(e_mesg);
	      			roll_back(e_mesg);
	   	  		continue;
		  	}
			retval = commit(e_mesg) ;
	  		if(retval < 0) {
	       		  	DispErr(e_mesg);
	      		  	retval = PrntRec(e_mesg);
	      			roll_back(e_mesg);
	   		  	continue;
			}
		}
	  }
	  seq_over(EMP_DED);
	  close_file(EMP_DED);

	  /*********************************************************/
	  /* Calculate CSB/Loans   			*/

#ifdef ENGLISH
	printf("\tCalculating CSB/Loans for: %s\n",pp_earn.pe_numb);
#else
#endif
	printf("\n");

	strcpy(emp_loan.el_numb,emp_rec.em_numb);
	emp_loan.el_code[0] = '\0';
	emp_loan.el_seq = 0;

	flg_reset(EMP_LOAN);

	for( ; ; ){
		retval = get_n_emp_loan(&emp_loan,BROWSE,0,FORWARD,e_mesg);
		if(retval < 0){
			if(retval == EFL) break;
			DispErr(e_mesg);
			retval = PrntRec(e_mesg);
			roll_back(e_mesg);
			seq_over(EMP_LOAN);
			continue;
		}
		if(strcmp(emp_loan.el_numb,pp_earn.pe_numb) != 0) 
			break;

		strcpy(loan.cs_code,emp_loan.el_code);

		retval = get_loan(&loan,BROWSE,0,e_mesg) ;
		if(retval < 0 && retval != UNDEF) {
			DispErr(e_mesg);
			retval = PrntRec(e_mesg);
			roll_back(e_mesg);
			continue;
		}
		  
		strcpy(pp_loan.pc_numb,emp_rec.em_numb);
		pp_loan.pc_pp = pp_earn.pe_pp;
		pp_loan.pc_date = pp_earn.pe_date;
		pp_loan.pc_seq = emp_loan.el_seq;
		strcpy(pp_loan.pc_code,emp_loan.el_code);
		pp_loan.pc_fund = loan.cs_fund;
		strcpy(pp_loan.pc_acct,loan.cs_amt_acct);

		retval = get_pp_loan(&pp_loan,BROWSE,0,e_mesg) ;
		if(retval < 0 && retval != UNDEF) {
			DispErr(e_mesg);
			retval = PrntRec(e_mesg);
			roll_back(e_mesg);
			continue;
		}
		  
		if(strcmp(emp_loan.el_ded_pp[pay_per_it.ppi_mthly - 1],
		   "Y") == 0 && emp_loan.el_amnt_out != 0){
			if(emp_loan.el_pp_num == 0 && strcmp(emp_loan.el_amt_flg
			   ,"P") == 0)
				pp_loan.pc_amount = 
				   D_Roundoff(emp_loan.el_pp_amount *
				   tot_income);
			else
				pp_loan.pc_amount = emp_loan.el_pp_amount;

			if(emp_loan.el_amnt_out < pp_loan.pc_amount)
				pp_loan.pc_amount = emp_loan.el_amnt_out;

			pp_loan.pc_int_amt = D_Roundoff(pp_loan.pc_amount *
				emp_loan.el_int);
		
			pp_earn.pe_net -= pp_loan.pc_amount +pp_loan.pc_int_amt;
	
			retval = put_pp_loan(&pp_loan,ADD,e_mesg) ;
			if(retval < 0) {
				DispErr(e_mesg);
				retval = PrntRec(e_mesg);
				roll_back(e_mesg);
				continue;
			}
			retval = commit(e_mesg) ;
			if(retval < 0) {
				DispErr(e_mesg);
				retval = PrntRec(e_mesg);
				roll_back(e_mesg);
				continue;
			}
			emp_loan.el_seq ++;

			/* Create jounral entry for liability */

			jr_ent.jr_fund = loan.cs_fund;
			retval = GetJr(loan.cs_fund);
			if(retval < 0) {
				continue;
			}
			jr_ent.jr_fund = loan.cs_fund;
			strcpy(jr_ent.jr_acct,loan.cs_amt_acct);

			jr_ent.jr_amount = D_Roundoff(pp_loan.pc_amount);
			strcpy(jr_ent.jr_emp_numb,pp_earn.pe_numb);
			strcpy(jr_ent.jr_code,pp_loan.pc_code);
			strcpy(jr_ent.jr_type,"D");

			if(jr_ent.jr_amount != 0.00) {
				jr_ent.jr_amount = jr_ent.jr_amount * -1;
				retval = put_jr_ent(&jr_ent,ADD,e_mesg);	
				if(retval < 0) {
					DispErr(e_mesg);
					retval = PrntRec(e_mesg);
					roll_back(e_mesg);
					continue;
				}
				retval = commit(e_mesg) ;
				if(retval < 0) {
					DispErr(e_mesg);
	      	 	 		retval = PrntRec(e_mesg);
	      				roll_back(e_mesg);
	   	  			continue;
				}
			}

			/* Create jounral entry for the liability for the
			   interest 	*/

			jr_ent.jr_fund = loan.cs_fund;
			retval = GetJr(loan.cs_fund);
			if(retval < 0) {
				continue;
			}
			jr_ent.jr_fund = loan.cs_fund;
			strcpy(jr_ent.jr_acct,loan.cs_int_acct);

			jr_ent.jr_amount = D_Roundoff(pp_loan.pc_int_amt);
			strcpy(jr_ent.jr_emp_numb,pp_earn.pe_numb);
			strcpy(jr_ent.jr_code,pp_loan.pc_code);
			strcpy(jr_ent.jr_type,"D");

			if(jr_ent.jr_amount != 0.00) {
				jr_ent.jr_amount = jr_ent.jr_amount * -1;
				retval = put_jr_ent(&jr_ent,ADD,e_mesg);	
				if(retval < 0) {
					DispErr(e_mesg);
					retval = PrntRec(e_mesg);
					roll_back(e_mesg);
					continue;
				}
				retval = commit(e_mesg) ;
				if(retval < 0) {
					DispErr(e_mesg);
					retval = PrntRec(e_mesg);
					roll_back(e_mesg);
					continue;
				}
			}
		}
	}	 
	seq_over(EMP_LOAN);
	close_file(EMP_LOAN);

	for(i=0;i<5;i++){
		pp_earn.pe_week_net[i] =
			D_Roundoff((week_income[i] / tot_income) *
			pp_earn.pe_net);
	}

	retval = put_pp_earn(&pp_earn,UPDATE,e_mesg) ;
	if(retval < 0) {
	  	DispErr(e_mesg);
	 	retval = PrntRec(e_mesg);
	  	roll_back(e_mesg);
	  	continue;
	}
	retval = commit(e_mesg) ;
	if(retval < 0) {
	  	DispErr(e_mesg);
	 	retval = PrntRec(e_mesg);
	  	roll_back(e_mesg);
	  	continue;
	}
	pp_earn.pe_date ++;
	flg_reset(PP_EARN);
     }/* get_n_pp_earn */
	seq_over(PP_EARN);
	close_file(PP_EARN);

	if(pgcnt){
		if(term < 99)
			last_page();
#ifndef	SPOOLER
	else
		rite_top();
#endif
	}

	return(0);

}
/*************************************************************************/

static
DispErr(s)
char *s;
{
	printf("%s\n", s) ;
#ifdef ENGLISH
	printf("Press RETURN<CR>\n");
#else
	printf("Appuyer sur RETURN<CR>\n");
#endif
	fflush( stdin );
	getchar();
	return(NOERROR);
}
/*-------------------------------------------------------------------------*/
GetJr(fund)
short	fund;
{
	int	retval;

	jr_ent.jr_no = HV_SHORT;
	flg_reset(JR_ENT);

	retval = get_n_jr_ent(&jr_ent,UPDATE,0,BACKWARD,e_mesg);
	if(retval < 0 && retval != EFL){
	  	DispErr(e_mesg);
	 	retval = PrntRec(e_mesg);
	  	roll_back(e_mesg);
		seq_over(JR_ENT);
		return(retval);
	}
	if(retval == EFL || jr_ent.jr_no < 1 || jr_ent.jr_fund != fund){
	  jr_ent.jr_fund = fund;
	  jr_ent.jr_no = 1;
	}
	else 
	  jr_ent.jr_no ++;
	jr_ent.jr_date = pp_earn.pe_date;

	return(NOERROR);
}
/*-----------------------------------------------------------------*/
InitPrinter1()
{
	char	resp[2] ;
	char	discfile[15] ;

	/* Always to Printer */
	STRCPY(resp,"P");
	discfile[0]= '\0';
	PG_SIZE = 63;

	if( opn_prnt( resp, discfile, 1, e_mesg, 1 /* spool */)<0 ){
		return(REPORT_ERR);
	}
	pgcnt = 0;		/* Page count is zero */
	LNSZ = 132;		/* line size in no. of chars */
	linecnt = PG_SIZE;	/* Page size in no. of lines */

	return(NOERROR) ;
}
/******************************************************************************
Prints the headings
******************************************************************************/
static
PrntHdg()	/* Print heading  */
{
	long	sysdt ;
	short	offset;

	if( pgcnt && term < 99)   /* new page and display */
		if(next_page()<0) return(EXIT);	
		
	if( pgcnt || term < 99) { /* if not the first page or display */
		if( rite_top()<0 ) return( -1 );	/* form_feed */
	}
	else
		linecnt = 0;
	pgcnt++; 			/* increment page no */

	mkln( 1, PROG_NAME, 10 );
#ifdef ENGLISH
	mkln( 103, "Date:", 5 );
#else
	mkln( 103, "Date:", 5 );
#endif
	sysdt = get_date() ;
	tedit( (char *)&sysdt,"____/__/__",line+cur_pos, R_LONG ); 
	cur_pos += 10;

#ifdef ENGLISH
	mkln( 122, "PAGE:", 5 );
#else
	mkln( 122, "PAGE:", 5 );
#endif
	tedit( (char *)&pgcnt,"__0_",  line+cur_pos, R_SHORT ); 
	cur_pos += 4;
	if(prnt_line() < 0 )	return(REPORT_ERR);
 
	offset = (LNSZ - strlen(pa_rec.pa_co_name))/2;
	mkln(offset,pa_rec.pa_co_name,strlen(pa_rec.pa_co_name));
	if(prnt_line() < 0 )	return(REPORT_ERR);

#ifdef ENGLISH
	mkln((LNSZ-29)/2,"CALCULATION ERROR AUDIT TRAIL", 29 );
#else
	mkln((LNSZ-29)/2,"TRANSLATE        ", 29 );
#endif
	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(3,"EMPLOYEE",8);
	mkln(14,"EMPLOYEE NAME",13);
	mkln(14,"COMMENT",7);

	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	return(NOERROR);
}
/****************************************************************************/
static
PrntRec(err_mesg)
char	*err_mesg;
{
	char	txt_line[132];
	int	retval;

	mkln(1,emp_rec.em_numb,12);
	sprintf(txt_line,"%s, %s",
		emp_rec.em_last_name,
		emp_rec.em_first_name);
	mkln(15,txt_line,22);
	mkln(40,err_mesg,90);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);
			
	else if(linecnt > PG_SIZE) {
		if((retval=PrntHdg()) == EXIT)	
			return(retval);
	}

	return(NOERROR);
}
/*-------------------- E n d   O f   P r o g r a m ---------------------*/
