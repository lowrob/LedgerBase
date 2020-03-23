/******************************************************************************
		Sourcename   : pp_reg.c
		System       : Budgetary Financial system.
		Module       : Fixed Assets System : Reports
		Created on   : 91-10-30
		Created  By  : Andre Cormier 
		Cobol Source : 

******************************************************************************
About the file:	
	This file has routines to print Fixed Asset Report. 
	It is called by the file pprep.c .

History:
Programmer      Last change on    Details

Andre Cormier	92-AUG-21	  Change that it only selects employees from
				  the current pay period. Before it was taking
				  employee from the previous pay period. 

L.Robichaud	94-May-17	  After allowing an advance to be done, the 
				register had to show any advances or adjustments
				Added two new functions. ProcessAdv is for the
				initial information on the register and 
				SummaryAdv is where the advance information is
				processed for the register summary and the 
				register total summary.
******************************************************************************/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_pp.h>
#include <bfs_com.h>
#include <repdef.h>

#define CONTINUE	10
#define EXIT		12
#define	PAGESIZE	1

#ifdef ENGLISH
#define PRINTER 	'P'
#define DISPLAY		'D'
#define FILE_IO		'F'
#else
#define PRINTER		'I'
#define DISPLAY		'A'
#define FILE_IO		'D'
#endif

static Pa_rec	pa_rec;
static Sch_rec	school;
static Emp	emp_rec;
static Emp_sched1	emp_sched1;
static Pay_earn	pay_earn, tmp_pay_earn;
static Earn	earn;
static Pay_ded	pay_ded;
static Pay_loan	pp_loan;
static Pay_garn pp_garn;
static Csb_loan	csb_loan;
static Time	time;
static Pay_param	pay_param;
static Deduction	deduction;
static Reg_pen	reg_pen;
static Barg_unit	barg_unit;
static Man_chq	man_chq;
/*  Data items for storing the key range end values */
	
typedef	struct	{		/* For summary report after each cc # */
	char	s_code[7];
	double	s_income;
	char	s_ded_code[7];
	char	s_pp_code[7];
	double	s_amount;

}	S_item1;

typedef	struct	{		/* For summary report at the end of the report*/
	char	s_code[7];
	double	s_income;
	char	s_ded_code[7];
	char	s_pp_code[7];
	double	s_amount;

}	S_item2;

typedef struct	{

	S_item1	s_items1[PAGESIZE] ;
} s_struct1;

typedef struct	{

	S_item2	s_items2[PAGESIZE] ;
} s_struct2;

static	s_struct1  s_sth1,image1;
static	s_struct2  s_sth2,image2;

typedef struct Page1 {
	S_item1	Items1[PAGESIZE] ;	/* Items Information */
	struct	Page1	*PrevPage1 ;	/* ptr to previous page */
	struct	Page1	*NextPage1 ;	/* ptr to next page */
	short	NoItems1;		/* number of Items on the page */
	short	Pageno1;			/* Page number */
}	Page1;

typedef struct Page2 {
	S_item2	Items2[PAGESIZE] ;	/* Items Information */
	struct	Page2	*PrevPage2 ;	/* ptr to previous page */
	struct	Page2	*NextPage2 ;	/* ptr to next page */
	short	NoItems2;		/* number of Items on the page */
	short	Pageno2;			/* Page number */
}	Page2;

static	Page1	*FirstPage1,		/* Address of First Page */
		*CurPage1,		/* Address of Current Page */
		*CurLast1,		/* Address of Curr. record last page */
		*LastPage1;		/* Address of Last Page of Memory
					   Allocated */
static	Page2	*FirstPage2,		/* Address of First Page */
		*CurPage2,		/* Address of Current Page */
		*CurLast2,		/* Address of Curr. record last page */
		*LastPage2;		/* Address of Last Page of Memory
					   Allocated */
/*  Data items for storing the key range end values */
static char	barg1[7];
static char	barg2[7];
static char	posi1[7];
static char	posi2[7];
static char	empl1[13];
static char	empl2[13];
	
static int	PG_SIZE;
static double	tot_ben;
static double	tot_ded;
static int	retval;
static char 	discfile[15];	/* for storing outputfile name */
static short	pgcnt; 		/* for page count */
static char	resp[2];	/* for storing response */
static short	copies;
static short	old_cost_center;
static long	sysdt ;

double	D_Roundoff();
extern char 	e_mesg[200];	/* for storing error messages */

pp_reg()
{

	/* Get details for output medium */
#ifdef ENGLISH
	STRCPY(resp,"P");
#else
	STRCPY(resp,"I");
#endif
	if((retval =  GetOutputon(resp))<0 )
		return(retval);

	switch(*resp) {
		case DISPLAY:
			resp[0]='D';
			STRCPY( discfile, terminal );
			PG_SIZE = 22;
			break;
		case FILE_IO:
			resp[0]='F';
			STRCPY( discfile, "pp_reg.dat" );
			if((retval = GetFilename(discfile))<0 )
				return(retval);
			PG_SIZE = 63;
			break;
		case PRINTER:
		default:
			resp[0]='P';
			discfile[0]= '\0';
			PG_SIZE = 63;
			break;
	}

	copies = 1;
	if( *resp=='P' ){
		if((retval = GetNbrCopies(&copies))<0 )
			return(retval);
	}

	strcpy( barg1, "     0");
	strcpy( barg2, "ZZZZZZ" );
	retval = GetBargRange( barg1,barg2 );
	if(retval < 0) return(retval);

	strcpy( posi1, "     0");
	strcpy( posi2, "ZZZZZZ" );
	retval = GetPosRange( posi1,posi2 );
	if(retval < 0) return(retval);

	strcpy( empl1, "           1");
	strcpy( empl2, "ZZZZZZZZZZZZ" );
	retval = GetEmpRange( empl1,empl2 );
	if(retval < 0) return(retval);

	if( (retval=Confirm())<=0 )
		return(retval);

	retval = get_pay_param(&pay_param,BROWSE,1,e_mesg);
	if(retval < 0) {
		fomen(e_mesg);
		return(retval);
	}

	retval = get_param( &pa_rec, BROWSE, 1, e_mesg );
	if( retval<1 ){
		return(DBH_ERR);
	}

	if( opn_prnt( resp, discfile, 1, e_mesg, 1 /* spool */)<0 ){
		return(REPORT_ERR);
	}
	if( *resp=='P' )
		SetCopies( (int)copies );
	pgcnt = 0;		/* Page count is zero */
	LNSZ = 132;		/* line size in no. of chars */
	linecnt = PG_SIZE;	/* Page size in no. of lines */

	FirstPage1 = NULL;
	LastPage1 = NULL;

	FirstPage2 = NULL;
	LastPage2 = NULL;

	retval = PrintRep();

	CloseRtn1();			/* return to menu */
	CloseRtn2();			/* return to menu */

	close_rep();	/* close output file */
	close_dbh();	/* Close files */
	return(retval);
}
/*----------------------------------------------------------------*/
/* Close nessary files and environment before exiting program               */

CloseRtn1() 
{
	/* Free the linked list for the end */
	for( ;LastPage1 != FirstPage1 ; ) {
		LastPage1 = LastPage1->PrevPage1;
		free((char *)LastPage1->NextPage1);
		LastPage1->NextPage1 = NULL;
	}
	if(FirstPage1 != NULL) {
		free((char *)FirstPage1);
	}

	CurLast1 = FirstPage1 = LastPage1 = NULL;

	return(NOERROR);
}	/* CloseRtn1() */
/*----------------------------------------------------------------*/
/* Close nessary files and environment before exiting program               */

CloseRtn2() 
{
	/* Free the linked list for the end */
	for( ;LastPage2 != FirstPage2 ; ) {
		LastPage2 = LastPage2->PrevPage2;
		free((char *)LastPage2->NextPage2);
		LastPage2->NextPage2 = NULL;
	}
	if(FirstPage2 != NULL) {
		free((char *)FirstPage2);
	}

	CurLast2 = FirstPage2 = LastPage2 = NULL;

	return(NOERROR);
}	/* CloseRtn2() */
/*----------------------------------------------------------------*/
static
PrintRep()
{
	int	first_time;


	first_time = 0;
	old_cost_center = 999;
	pay_earn.pe_cc = 0;

	for(;;) {
		if(pay_earn.pe_cc != old_cost_center) {

			old_cost_center = pay_earn.pe_cc;
			pay_earn.pe_numb[0] = '\0';
			pay_earn.pe_pp = 0;
			pay_earn.pe_date = 0;

			flg_reset(PP_EARN);
		}

		retval = get_n_pp_earn(&pay_earn,BROWSE,1,FORWARD,e_mesg);

		if(retval == EFL) break;

		strcpy(emp_rec.em_numb,pay_earn.pe_numb);
		retval = get_employee(&emp_rec,BROWSE,0,e_mesg);

		if(strcmp(emp_rec.em_barg,barg1) < 0 ||
		   strcmp(emp_rec.em_barg,barg2) > 0) {
			roll_back(e_mesg);
			continue;
		}

		if(strcmp(emp_rec.em_pos,posi1) < 0 ||
		   strcmp(emp_rec.em_pos,posi2) > 0) {
			roll_back(e_mesg);
			continue;
		}

		if(strcmp(emp_rec.em_numb,empl1) < 0 ||
		   strcmp(emp_rec.em_numb,empl2) > 0) {
			roll_back(e_mesg);
			continue;
		}

		strcpy(barg_unit.b_code,emp_rec.em_barg);
		barg_unit.b_date = get_date();
		flg_reset(BARG);

		retval = get_n_barg(&barg_unit,BROWSE,0,BACKWARD,e_mesg);
		if(retval == EFL ||
			strcmp(barg_unit.b_code, emp_rec.em_barg) != 0){
			sprintf(e_mesg,"Bargaining Unit does no exist: %s",
				emp_rec.em_barg);
			fomer(e_mesg);
			return(NOERROR);
		}
		if(retval < 0){
			fomer(e_mesg);
  			return(ERROR);
		}
		seq_over(BARG);

		if(first_time == 0) {
			PrntHdg(0);
			old_cost_center = pay_earn.pe_cc;
			first_time = 1;
		}
		if(old_cost_center != pay_earn.pe_cc) {

			PrntHdg(1);

			if((retval = PrntSum())<0 )
				return(retval);

			CloseRtn1();

			PrntHdg(0);
			continue;
		}
		if((retval = PrntRec())<0 )
			return(retval);
		if(retval == EXIT)	return(retval);
	}
	seq_over(PP_EARN);

	PrntHdg(1);

	if((retval = PrntSum())<0 )
		return(retval);
	if(retval == EXIT)	return(retval);

	PrntHdg(2);

	if((retval = PrntTotSum())<0 )
		return(retval);
	if(retval == EXIT)	return(retval);

	if(pgcnt) {
		if(term < 99) 
			last_page();	
#ifndef	SPOOLER
		else
			rite_top();
#endif
	}

	return(NOERROR);
}

static
PrntCostLine()
{
	char	txt_buff[132];
	int	retval;

	school.sc_numb = pay_earn.pe_cc;
	retval = get_sch(&school,BROWSE,0,e_mesg);
	if(retval < 0) {
		fomer("Cost Center Number Does not Exist");
		if(retval != UNDEF) return(ERROR);
	}

	mkln(1,"Cost Center Number :",20);
	tedit((char*)&pay_earn.pe_cc,"___",txt_buff,R_SHORT);
	mkln(22,txt_buff,3);
	mkln(26,school.sc_name,28);

	retval = PrintLine();
	if(retval == EXIT)	return(retval);
	if(retval < 0)	return(retval);

	return(NOERROR);
}
static
PrntHdg(flag)	/* Print heading  */
int	flag;
{
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
	retval = PrintLine();
	if(retval < 0)	return(retval);

	offset = ( LNSZ-strlen(pa_rec.pa_co_name) )/2;
	mkln( offset, pa_rec.pa_co_name, strlen(pa_rec.pa_co_name) );
	retval = PrintLine();
	if(retval < 0)	return(retval);

#ifdef ENGLISH
	mkln((LNSZ-19)/2,"PAY PERIOD REGISTER", 19 );
#else
	mkln((LNSZ-19)/2,"PAY PERIOD REGISTER", 19 );
#endif
	retval = PrintLine();
	if(retval < 0)	return(retval);

	if(flag == 1) {
#ifdef ENGLISH
		mkln((LNSZ-7)/2,"SUMMARY", 7 );
#else
		mkln((LNSZ-7)/2,"SUMMARY", 7 );
#endif
		retval = PrintLine();
		if(retval < 0)	return(retval);
	}

	if(flag == 2) {
#ifdef ENGLISH
		mkln((LNSZ-13)/2,"TOTAL SUMMARY", 13 );
#else
		mkln((LNSZ-13)/2,"TOTAL SUMMARY", 13 );
#endif
		retval = PrintLine();
		if(retval < 0)	return(retval);
	}

	retval = PrintLine();
	if(retval < 0)	return(retval);

	if(flag == 0) {
		retval = PrntCostLine();
		if(retval < 0)	return(retval);
	}

	return(NOERROR);
}

static
PrntHdgDed()
{
	char	name[50];

	mkln(1,"EMPLOYEE #:",11);
	mkln(13,emp_rec.em_numb,12);
	mkln(29,"NAME :",6);
	strcpy(name, emp_rec.em_first_name);
	strcat(name," ");
	strcat(name, emp_rec.em_last_name);
	mkln(35,name,strlen(name));
	
	retval = PrintLine();
	if(retval == EXIT)	return(retval);
	if(retval < 0)	return(retval);
	retval = PrintLine();
	if(retval == EXIT)	return(retval);
	if(retval < 0)	return(retval);

#ifdef ENGLISH
	mkln(2,"EARNING",7);
	mkln(20,"DESCRIPTION",11);
	mkln(52,"AMOUNT",6);
	mkln(64,"DEDUCTION",9);
	mkln(82,"DESCRIPTION",11);
	mkln(111,"AMOUNT",6);
	mkln(124,"NET",3);
#else
	mkln(2,"EARNING",7);
	mkln(20,"DESCRIPTION",11);
	mkln(52,"AMOUNT",6);
	mkln(64,"DEDUCTION",9);
	mkln(82,"DESCRIPTION",11);
	mkln(111,"AMOUNT",6);
	mkln(124,"NET",3);
#endif
	retval = PrintLine();
	if(retval == EXIT)	return(retval);
	if(retval < 0)	return(retval);

#ifdef ENGLISH
	mkln(3,"CODE",4);
	mkln(66,"CODE",4);
	mkln(123,"INCOME",6);
#else
	mkln(3,"CODE",4);
	mkln(79,"CODE",4);
	mkln(123,"INCOME",6);
#endif
	retval = PrintLine();
	if(retval == EXIT)	return(retval);
	if(retval < 0)	return(retval);

	return(NOERROR);
}
static
PrntRec()
{
	if((retval = PrntHdgDed())<0 )
		return(retval);

	if((retval = BuildEarn())<0 )
		return(retval);

	if((retval = PrntLines()) <0)
		return(retval);

	if((retval = ProcessAdv()) <0)
		return(retval);

	if((retval = PrntTot())<0 )
		return(retval);


	return(NOERROR);
}

/*-----------------------------------------------------------------------*/
/*                                                                       */
static
PrntLines()
{
	char	code[7];
	char	prev_earn[7];
	char	txt_buff[132];
	int	count;
	int	earn_flag;
	int	ded_flag;
	int	retval, col,i;
	double	units, temp_calc, income, empr_share;
	int	schedule;

	strcpy(time.tm_numb,emp_rec.em_numb);
	time.tm_earn[0] = '\0';
	time.tm_date = 0;
	time.tm_no = 0;

	flg_reset(TIME);

	strcpy(emp_sched1.es_numb,emp_rec.em_numb);
	emp_sched1.es_week = 0;
	emp_sched1.es_fund = 0;
	emp_sched1.es_class[0] = '\0';
	emp_sched1.es_cost = 0;
	flg_reset(EMP_SCHED1);

	prev_earn[0] = '\0';
	units = 0;
	income = 0;
	earn_flag = 1;
	schedule = 0;
	for( ; ; ){
	  retval = get_n_ptime(&time,BROWSE,4,FORWARD,e_mesg);
	  if((strcmp(time.tm_numb,emp_rec.em_numb)!=0) ||
	    (retval == EFL)){
		if(income == 0){
			schedule = 1;
			break;
		}
		earn_flag = 0;
		break;
	  }
	  else{
	    if(time.tm_date != pay_earn.pe_date) continue;

	    if(prev_earn[0] == '\0')
		strcpy(prev_earn, time.tm_earn);
	    if(strcmp(prev_earn, time.tm_earn) != 0){
		flg_reset(TIME);
		break;
	    }
	    strcpy(code,prev_earn);
	    income += D_Roundoff(time.tm_tot_amt);
	  }
	}

	if(schedule == 1){
	  for( ; ; ){
	    retval = get_n_emp_sched1(&emp_sched1,BROWSE,0,FORWARD,e_mesg);
	    if((strcmp(emp_sched1.es_numb,emp_rec.em_numb)!=0) ||
	      (retval == EFL)){
		earn_flag = 0;
		break;
	    }
	    strcpy(code, pay_param.pr_reg_earn);
	    income += D_Roundoff(emp_sched1.es_amount);
	  }
	  
	}
	GetDesc(code);

	mkln(2,code,6);
	mkln(12,earn.ea_desc,30);
	tedit((char*)&income,
		"_,___,_0_.__-",txt_buff,R_DOUBLE);
	mkln(46,txt_buff,13);

	strcpy(prev_earn, time.tm_earn);
	/***** DEDUCTIONS *****/

	mkln(74,"CPP CONTRIBUTION",16);
	tedit((char*)&pay_earn.pe_cpp,"_,___,_0_.__-",txt_buff,R_DOUBLE);
	mkln(104,txt_buff,13);

	tot_ded = tot_ded + pay_earn.pe_cpp;

	CheckDedCd1("CPP", barg_unit.b_pp_code, pay_earn.pe_cpp);
	CheckDedCd2("CPP", barg_unit.b_pp_code, pay_earn.pe_cpp);

	retval = PrintLine();
	if(retval == EXIT)	return(retval);
	if(retval < 0)	return(retval);

	if(earn_flag != 0){
	 if(schedule != 1){
	  units = 0;
	  income = 0;
	  for( ; ; ){
	    retval = get_n_ptime(&time,BROWSE,4,FORWARD,e_mesg);
	    if((strcmp(time.tm_numb,emp_rec.em_numb)!=0) ||
	      (retval == EFL)){
		earn_flag = 0;
		break;
	    }
	    else{
	      if(time.tm_date != pay_earn.pe_date) continue;
	      if(strcmp(prev_earn, time.tm_earn) != 0){
		flg_reset(TIME);
		break;
	      }
	      strcpy(code,time.tm_earn);
	      income += D_Roundoff(time.tm_tot_amt);
	    }
	  }
	  strcpy(code,prev_earn);
	  GetDesc(code);

	  mkln(2,code,6);
	  mkln(12,earn.ea_desc,30);
	  tedit((char*)&income,
		"_,___,_0_.__-",txt_buff,R_DOUBLE);
	  mkln(46,txt_buff,13);
	 }
	}

	strcpy(prev_earn, time.tm_earn);
	/***** DEDUCTIONS *****/

	mkln(74,"UIC PREMIUM",11);
	tedit((char*)&pay_earn.pe_uic,"_,___,_0_.__-",txt_buff,R_DOUBLE);
	mkln(104,txt_buff,13);

	tot_ded = tot_ded + pay_earn.pe_uic;

	CheckDedCd1("UIC", barg_unit.b_pp_code, pay_earn.pe_uic);
	CheckDedCd2("UIC", barg_unit.b_pp_code, pay_earn.pe_uic);

	retval = PrintLine();
	if(retval == EXIT)	return(retval);
	if(retval < 0)	return(retval);

	strcpy(pay_ded.pd_numb, emp_rec.em_numb);
	pay_ded.pd_pp = 0;
	pay_ded.pd_date = 0;
	pay_ded.pd_code[0] = '\0';
	pay_ded.pd_fund = 0;
	pay_ded.pd_acct[0] = '\0';

	flg_reset(PP_DED);

	strcpy(pp_loan.pc_numb,emp_rec.em_numb);
	pp_loan.pc_pp = 0;
	pp_loan.pc_date = 0;
	pp_loan.pc_code[0] = '\0';
	pp_loan.pc_seq = 0;
	pp_loan.pc_fund = 0;
	pp_loan.pc_acct[0] = '\0';

	flg_reset(PP_LOAN);

	strcpy(pp_garn.pg_numb,emp_rec.em_numb);
	pp_garn.pg_pp = 0;
	pp_garn.pg_date = 0;
	pp_garn.pg_pr_cd = 0;
	pp_garn.pg_seq = 0;
	pp_garn.pg_fund = 0;
	pp_garn.pg_acct[0] = '\0';

	flg_reset(PP_GARN);

	ded_flag = 3;
	count = 0;

	for(;;) {
		count++;
		if(earn_flag == 0 && ded_flag == 0){
			break;
		}
		if(earn_flag == 1){
		  if(schedule != 1){
		   income = 0;
		   for( ; ; ){
		    retval = get_n_ptime(&time,BROWSE,4,FORWARD,e_mesg);
		    if((strcmp(time.tm_numb,emp_rec.em_numb)!=0) ||
	  	    (retval == EFL)){
			earn_flag = 0;
			break;
		    }
	      	    if(time.tm_date != pay_earn.pe_date) continue;
		    if(strcmp(prev_earn, time.tm_earn) != 0){
			flg_reset(TIME);
			break;
		    }
		    income += D_Roundoff(time.tm_tot_amt);
		   }
		  }
		  strcpy(code, prev_earn);
		  GetDesc(code);

		  mkln(2,code,6);
		  mkln(12,earn.ea_desc,30);
		  tedit((char*)&income,
			"_,___,_0_.__-",txt_buff,R_DOUBLE);
		  mkln(46,txt_buff,13);
		}
		strcpy(prev_earn, time.tm_earn);
	/*****  Deductions *****/
		if(ded_flag == 3) {
			if(count == 1) {
				mkln(74,"INCOME TAX",10);
				tedit((char*)&pay_earn.pe_tax,"_,___,_0_.__-",
					txt_buff,R_DOUBLE);
				mkln(104,txt_buff,13);

				tot_ded = tot_ded + pay_earn.pe_tax;

				CheckDedCd1("TAX", barg_unit.b_pp_code,
							 pay_earn.pe_tax);
				CheckDedCd2("TAX", barg_unit.b_pp_code,
							 pay_earn.pe_tax);
			}
			if(count == 2) {
			   pay_earn.pe_reg1 = (pay_earn.pe_reg1 +
					    pay_earn.pe_reg2 +
					    pay_earn.pe_reg3);
			   if(pay_earn.pe_reg1 != 0) {

				strcpy(reg_pen.rg_code,emp_rec.em_reg_pen);
				strcpy(reg_pen.rg_pp_code,barg_unit.b_pp_code);
		  	  	retval = get_reg_pen(&reg_pen,BROWSE,0,e_mesg);
				if(retval == NOERROR){
				  	mkln(65,reg_pen.rg_code,6);
					mkln(74,reg_pen.rg_desc,23);
				}
				tedit((char*)&pay_earn.pe_reg1,"_,___,_0_.__-",
					txt_buff,R_DOUBLE);
				mkln(104,txt_buff,13);

			tot_ded = tot_ded + pay_earn.pe_reg1;

				CheckDedCd1(emp_rec.em_reg_pen,
				 barg_unit.b_pp_code, pay_earn.pe_reg1);
				CheckDedCd2(emp_rec.em_reg_pen,
				 barg_unit.b_pp_code, pay_earn.pe_reg1);
			   }
			   else {
				count = 3;
			   }
			}
			if(count >= 3) {
			  retval = get_n_pp_ded(&pay_ded,BROWSE,0,
					 FORWARD,e_mesg);

			  if(strcmp(pay_ded.pd_numb,emp_rec.em_numb) != 0 ||
					retval == EFL)
						ded_flag = 2;

			  else{
				strcpy(deduction.dd_code,pay_ded.pd_code);
				strcpy(deduction.dd_pp_code,barg_unit.b_pp_code);

				get_deduction(&deduction,BROWSE,0,e_mesg);

				mkln(65,pay_ded.pd_code,6);
				mkln(74,deduction.dd_desc,30);
				tedit((char*)&pay_ded.pd_amount,"_,___,_0_.__-",
					txt_buff,R_DOUBLE);
				mkln(104,txt_buff,13);

			tot_ded = tot_ded + pay_ded.pd_amount;

				CheckDedCd1(pay_ded.pd_code,barg_unit.b_pp_code,
					 pay_ded.pd_amount);
				CheckDedCd2(pay_ded.pd_code,barg_unit.b_pp_code,
					 pay_ded.pd_amount);
			  }
			}
		}
		if(ded_flag == 2) {
		  retval = get_n_pp_loan(&pp_loan,BROWSE,0,FORWARD,e_mesg);

		  if((strcmp(pp_loan.pc_numb,emp_rec.em_numb)!=0) || 
		    (retval == EFL)) ded_flag = 1;

		  else{

		  	mkln(65,pp_loan.pc_code,6);
			strcpy(csb_loan.cs_code,pp_loan.pc_code);
			retval = get_loan(&csb_loan,BROWSE,0,e_mesg);
			if(retval < 0) {
				fomen(e_mesg);
				get();
				return(retval);
			}
			mkln(74,csb_loan.cs_desc,30);

		  	tedit((char*)&pp_loan.pc_amount,"_,___,_0_.__-",
				txt_buff,R_DOUBLE);
			mkln(104,txt_buff,13);

			tot_ded = tot_ded + pp_loan.pc_amount;

			CheckDedCd1(csb_loan.cs_code,barg_unit.b_pp_code,
					 pp_loan.pc_amount);
			CheckDedCd2(csb_loan.cs_code,barg_unit.b_pp_code,
					 pp_loan.pc_amount);
		  }
		}

		if(ded_flag == 1) {
		  retval = get_n_pp_garn(&pp_garn,BROWSE,0,FORWARD,e_mesg);
		  if((strcmp(pp_garn.pg_numb,emp_rec.em_numb)!=0) ||
		    (retval == EFL)) ded_flag = 0;

		  else{
		  	mkln(65,"GARN  ",6);
			mkln(74,"GARNISHMENT",30);
		 	tedit((char*)&pp_garn.pg_amount,"_,___,_0_.__-",
				txt_buff,R_DOUBLE);
			mkln(104,txt_buff,13);

			tot_ded = tot_ded + pp_garn.pg_amount;

			CheckDedCd1("GARN",barg_unit.b_pp_code,
					 pp_garn.pg_amount);
			CheckDedCd2("GARN",barg_unit.b_pp_code,
					 pp_garn.pg_amount);
		  }
		}
		retval = PrintLine();
		if(retval == EXIT)	return(retval);
		if(retval < 0)	return(retval);
	}
	seq_over(TIME);
	seq_over(EMP_SCHED1);
	seq_over(PP_DED);
	seq_over(PP_LOAN);
	seq_over(PP_GARN);

	retval = PrintLine();
	if(retval == EXIT)	return(retval);
	if(retval < 0)	return(retval);

	return(NOERROR);

}
static
GetDesc(code)
char	*code;
{
	strcpy(earn.ea_code,code);
	earn.ea_date = get_date();

	flg_reset(EARN);

	retval=get_n_earn(&earn,BROWSE,1,BACKWARD,e_mesg);
	seq_over(EARN);


	return(NOERROR);

}
/*----------------------------------------------------------------*/
/* Process manual cheque associated with employee cheque */

ProcessAdv()
{
	int	retval;
	double	adv_amount;
	double	adj_amount;
	char	txt_buff[132];

	adv_amount = 0;
	adj_amount = 0;
	strcpy(man_chq.mc_emp_numb,emp_rec.em_numb);
	man_chq.mc_date = pay_earn.pe_date;
	flg_reset(MAN_CHQ);

	/* get all the advances and add together */
	for(;;){
		retval = get_n_man_chq(&man_chq,BROWSE,1,FORWARD,e_mesg) ;
		if( retval == EFL)
			break;
		if( retval < 0 ) {
			DispError((char *)&s_sth1,e_mesg) ;
			roll_back(e_mesg);
			return(retval) ;
			}
		if(strcmp(man_chq.mc_emp_numb,emp_rec.em_numb)==0 &&
		  man_chq.mc_date == pay_earn.pe_date)
			adv_amount += man_chq.mc_amount;
		else
			break;
	}

	man_chq.mc_chq_numb = pay_earn.pe_date;
	flg_reset(MAN_CHQ);
	/* Get any adjustments for this cheque */
	for(;;){
		retval = get_n_man_chq(&man_chq,BROWSE,2,FORWARD,e_mesg) ;
		if( retval == EFL)
			break;
		if( retval < 0 ) {
			DispError((char *)&s_sth1,e_mesg) ;
			roll_back(e_mesg);
			return(retval) ;
		}
		if(strcmp(man_chq.mc_emp_numb,emp_rec.em_numb)==0 &&
		  man_chq.mc_chq_numb == pay_earn.pe_date)
			adj_amount += man_chq.mc_amount;
		else
			if(man_chq.mc_chq_numb > pay_earn.pe_date)
				break;
	}
	if(adj_amount > 0){
		/***** DEDUCTIONS *****/
		mkln(74,"ADVANCE ADJUSTMENT",19);
		tedit((char*)&adj_amount,"_,___,_0_.__-",txt_buff,R_DOUBLE);
		mkln(104,txt_buff,13);
		tot_ded += adj_amount;
		retval = PrintLine();
		if(retval == EXIT)	return(retval);
		if(retval < 0)	return(retval);
	}
	if(adv_amount > 0){
		/***GetDesc(code);
		mkln(2,code,6);****/
		mkln(12,"ADVANCE",30);
		tedit((char*)&adv_amount,
			"_,___,_0_.__-",txt_buff,R_DOUBLE);
		mkln(46,txt_buff,13);
		tot_ben += adv_amount;
		retval = PrintLine();
		if(retval == EXIT)	return(retval);
		if(retval < 0)	return(retval);
	}
	return(NOERROR);
}

static
PrntTot()
{
	double	fin_tot;
	char	txt_buff[132];

	retval = PrintLine();
	if(retval == EXIT)	return(retval);
	if(retval < 0)	return(retval);

	fin_tot = tot_ben - tot_ded;

	mkln(1,"TOTAL:",6);
	tedit((char*)&tot_ben,"_,___,_0_.__-", txt_buff,R_DOUBLE);
	mkln(46,txt_buff,13);
	tedit((char*)&tot_ded,"_,___,_0_.__-",txt_buff,R_DOUBLE);
	mkln(104,txt_buff,13);
	tedit((char*)&fin_tot,"_,___,_0_.__-",txt_buff,R_DOUBLE);
	mkln(119,txt_buff,13);

	retval = PrintLine();
	if(retval == EXIT)	return(retval);
	if(retval < 0)	return(retval);
	retval = PrintLine();
	if(retval == EXIT)	return(retval);
	if(retval < 0)	return(retval);

	tot_ben = 0;
	tot_ded = 0;
	fin_tot = 0;

	return(NOERROR);
}
/*----------------------------------------------------------------*/
BuildEarn()
{
	tot_ben = tot_ben + pay_earn.pe_reg_inc1;

	CheckCode1(pay_param.pr_reg_earn, pay_earn.pe_reg_inc1);
	CheckCode2(pay_param.pr_reg_earn, pay_earn.pe_reg_inc1);

	strcpy(time.tm_numb,emp_rec.em_numb);
	time.tm_date = 0;
	time.tm_no = 0;
	flg_reset(TIME);

	for(;;) {
		retval = get_n_ptime(&time,BROWSE,0,FORWARD,e_mesg);

		if((strcmp(time.tm_numb,emp_rec.em_numb)!=0) ||
		  (retval == EFL)){
			break;
		}
		else{
	      	    if(time.tm_date != pay_earn.pe_date) continue;

			tot_ben = tot_ben + D_Roundoff(time.tm_tot_amt);

			CheckCode1(time.tm_earn, D_Roundoff(time.tm_tot_amt));
			CheckCode2(time.tm_earn, D_Roundoff(time.tm_tot_amt));
		}
	}
	seq_over(TIME);

	return(NOERROR);
}
/*----------------------------------------------------------------*/
CheckCode1(code, amount)
char	*code;
double	amount;
{
	Page1	*temppage;
	int	pass;

	pass = 0;

	/* check to see if item is already in list */
	if(CurLast1 != NULL) {
	   for(temppage=FirstPage1;temppage!=NULL;temppage=temppage->NextPage1){
		if(temppage->Items1[0].s_code[0] == NULL) {
			strcpy(temppage->Items1[0].s_code,code);
	  		temppage->Items1[0].s_income = amount;
			pass = 1;
			break;
		}
		if(strcmp(temppage->Items1[0].s_code,code) == 0) {
			temppage->Items1[0].s_income += amount;
			pass = 1;
			break;
		}
	   }
	   if(pass == 0){

		MakeFreshPage1();

		strcpy(CurPage1->Items1[0].s_code,code);
	  	CurPage1->Items1[0].s_income = amount;

	   }
	}
	else{
		MakeFreshPage1();

		strcpy(CurPage1->Items1[0].s_code,code);
	  	CurPage1->Items1[0].s_income = amount;
	}

	return(NOERROR);
}
/*----------------------------------------------------------------*/
CheckCode2(code, amount)
char	*code;
double	amount;
{
	Page2	*temppage;
	int	pass;

	pass = 0;

	/* check to see if item is already in list */
	if(CurLast2 != NULL) {
	   for(temppage=FirstPage2;temppage!=NULL;temppage=temppage->NextPage2){
		if(temppage->Items2[0].s_code[0] == NULL) {
			strcpy(temppage->Items2[0].s_code,code);
	  		temppage->Items2[0].s_income = amount;
			pass = 1;
			break;
		}
		if(strcmp(temppage->Items2[0].s_code,code) == 0) {
			temppage->Items2[0].s_income += amount;
			pass = 1;
			break;
		}
	   }
	   if(pass == 0){

		MakeFreshPage2();

		strcpy(CurPage2->Items2[0].s_code,code);
	  	CurPage2->Items2[0].s_income = amount;

	   }
	}
	else{
		MakeFreshPage2();

		strcpy(CurPage2->Items2[0].s_code,code);
	  	CurPage2->Items2[0].s_income = amount;
	}

	return(NOERROR);
}
/*----------------------------------------------------------------*/
CheckDedCd1(ded_code, pp_code, amount)
char	*ded_code;
char	*pp_code;
double	amount;
{
	Page1	*temppage;
	int	pass;

	pass = 0;

	/* check to see if item is already in list */
	if(CurLast1 != NULL) {
	   for(temppage=FirstPage1;temppage!=NULL;temppage=temppage->NextPage1){
		if(temppage->Items1[0].s_ded_code[0] == NULL) {
			strcpy(temppage->Items1[0].s_ded_code,ded_code);
			strcpy(temppage->Items1[0].s_pp_code,pp_code);
		  	temppage->Items1[0].s_amount= amount;
			pass = 1;
			break;
		}
		if((strcmp(temppage->Items1[0].s_ded_code,ded_code) == 0) &&
		   (strcmp(temppage->Items1[0].s_pp_code,pp_code) == 0)) {
			temppage->Items1[0].s_amount += amount;
			pass = 1;
			break;
		}
	   }
   	   if(pass == 0){
		MakeFreshPage1();

		strcpy(CurPage1->Items1[0].s_ded_code,ded_code);
		strcpy(CurPage1->Items1[0].s_pp_code,pp_code);
	  	CurPage1->Items1[0].s_amount= amount;
	   }
	}
	else{
		MakeFreshPage1();

		strcpy(CurPage1->Items1[0].s_ded_code,ded_code);
		strcpy(CurPage1->Items1[0].s_pp_code,pp_code);
		CurPage1->Items1[0].s_amount= amount;
	}
	return(NOERROR);
}
/*----------------------------------------------------------------*/
CheckDedCd2(ded_code, pp_code, amount)
char	*ded_code;
char	*pp_code;
double	amount;
{
	Page2	*temppage;
	int	pass;

	pass = 0;

	/* check to see if item is already in list */
	if(CurLast2 != NULL) {
	   for(temppage=FirstPage2;temppage!=NULL;temppage=temppage->NextPage2){
		if(temppage->Items2[0].s_ded_code[0] == NULL) {
			strcpy(temppage->Items2[0].s_ded_code,ded_code);
			strcpy(temppage->Items2[0].s_pp_code,pp_code);
		  	temppage->Items2[0].s_amount= amount;
			pass = 1;
			break;
		}
		if((strcmp(temppage->Items2[0].s_ded_code,ded_code) == 0) &&
		   (strcmp(temppage->Items2[0].s_pp_code,pp_code) == 0)) {
			temppage->Items2[0].s_amount += amount;
			pass = 1;
			break;
		}
	   }
	   if(pass == 0){
		MakeFreshPage2();

		strcpy(CurPage2->Items2[0].s_ded_code,ded_code);
		strcpy(CurPage2->Items2[0].s_pp_code,pp_code);
	  	CurPage2->Items2[0].s_amount= amount;
	   }
	}
	else{
		MakeFreshPage2();

		strcpy(CurPage2->Items2[0].s_ded_code,ded_code);
		strcpy(CurPage2->Items2[0].s_pp_code,pp_code);
		CurPage2->Items2[0].s_amount= amount;
	}
	return(NOERROR);
}
/*-----------------------------------------------------------------------*/
/*	Get the next node in linked list to add invoice items. If the
*	(Cur. invc last page) = (Last Page in linked List) or no
*	nodes in list, allocate node and add to linked list
*/

MakeFreshPage1()
{
	Page1	*tempptr ;

	/* If, no node is allocated yet or Current invoice used all the nodes,
	   then allocate new node */

	if( LastPage1 == NULL || CurLast1 == LastPage1 ){
		tempptr= (Page1 *)malloc((unsigned)sizeof(Page1)) ;

		if( tempptr == NULL ){
			DispError((char*)&s_sth1,"Memory Allocation Error");
			return(ERROR);
		}
		tempptr->NextPage1 = NULL ;

		if( LastPage1 == NULL ){	/* No node is allocated Yet */
			tempptr->PrevPage1 = NULL ;
			tempptr->Pageno1 = 1 ;
			FirstPage1 = tempptr ;
		}
		else {				/* Not a first node in list */
			tempptr->Pageno1 = LastPage1->Pageno1 + 1 ;
			LastPage1->NextPage1 = tempptr ;
			tempptr->PrevPage1 = LastPage1 ;
		}
		LastPage1 = tempptr ;
	}

	if(CurLast1 == NULL)
		CurLast1 = FirstPage1 ;
	else
		CurLast1 = CurLast1->NextPage1 ;

	CurLast1->NoItems1 = 0 ;
	CurPage1 = CurLast1 ;
	CurPage1->Items1[0].s_code[0] = NULL;
  	CurPage1->Items1[0].s_income = 0;
	CurPage1->Items1[0].s_ded_code[0] = NULL;
	CurPage1->Items1[0].s_pp_code[0] = NULL;
  	CurPage1->Items1[0].s_amount = 0;

	return(NOERROR);
}	/* MakeFreshPage1() */
/*-----------------------------------------------------------------------*/
/*	Get the next node in linked list to add invoice items. If the
*	(Cur. invc last page) = (Last Page in linked List) or no
*	nodes in list, allocate node and add to linked list
*/

MakeFreshPage2()
{
	Page2	*tempptr ;

	/* If, no node is allocated yet or Current invoice used all the nodes,
	   then allocate new node */

	if( LastPage2 == NULL || CurLast2 == LastPage2 ){
		tempptr= (Page2 *)malloc((unsigned)sizeof(Page2)) ;

		if( tempptr == NULL ){
			DispError((char*)&s_sth2,"Memory Allocation Error");
			return(ERROR);
		}
		tempptr->NextPage2 = NULL ;

		if( LastPage2 == NULL ){	/* No node is allocated Yet */
			tempptr->PrevPage2 = NULL ;
			tempptr->Pageno2 = 1 ;
			FirstPage2 = tempptr ;
		}
		else {				/* Not a first node in list */
			tempptr->Pageno2 = LastPage2->Pageno2 + 1 ;
			LastPage2->NextPage2 = tempptr ;
			tempptr->PrevPage2 = LastPage2 ;
		}
		LastPage2 = tempptr ;
	}

	if(CurLast2 == NULL)
		CurLast2 = FirstPage2 ;
	else
		CurLast2 = CurLast2->NextPage2 ;

	CurLast2->NoItems2 = 0 ;
	CurPage2 = CurLast2 ;
	CurPage2->Items2[0].s_code[0] = NULL;
  	CurPage2->Items2[0].s_income = 0;
	CurPage2->Items2[0].s_ded_code[0] = NULL;
	CurPage2->Items2[0].s_pp_code[0] = NULL;
  	CurPage2->Items2[0].s_amount = 0;

	return(NOERROR);
}	/* MakeFreshPage2() */
/*----------------------------------------------------------*/
static
PrntSum()
{
	if((retval = PrntHdgDed2())<0 )
		return(retval);

	if((retval = PrintSum())<0 )
		return(retval);

	return(NOERROR);
}
/*----------------------------------------------------------*/
static
PrntTotSum()
{
	if((retval = PrntHdgDed2())<0 )
		return(retval);

	if((retval = PrintTotSum())<0 )
		return(retval);

	return(NOERROR);
}

static
PrntHdgDed2()
{

#ifdef ENGLISH
	mkln(2,"EARNING",7);
	mkln(20,"DESCRIPTION",11);
	mkln(52,"AMOUNT",6);
	mkln(64,"DEDUCTION",9);
	mkln(82,"DESCRIPTION",11);
	mkln(111,"AMOUNT",6);
	mkln(124,"NET",3);
#else
	mkln(2,"EARNING",7);
	mkln(20,"DESCRIPTION",11);
	mkln(52,"AMOUNT",6);
	mkln(64,"DEDUCTION",9);
	mkln(82,"DESCRIPTION",11);
	mkln(111,"AMOUNT",6);
	mkln(124,"NET",3);
#endif
	retval = PrintLine();
	if(retval == EXIT)	return(retval);
	if(retval < 0)	return(retval);

#ifdef ENGLISH
	mkln(3,"CODE",4);
	mkln(66,"CODE",4);
	mkln(123,"INCOME",6);
#else
	mkln(3,"CODE",4);
	mkln(79,"CODE",4);
	mkln(123,"INCOME",6);
#endif
	retval = PrintLine();
	if(retval == EXIT)	return(retval);
	if(retval < 0)	return(retval);

	return(NOERROR);
}

static
PrintSum()
{
	char	code[7];
	char	txt_buff[132];
	int	retval;
	Page1	*temppage;
	double	fin_tot;

	/* check to see if item is already in list */
	if(CurLast1 != NULL) {
	   for(temppage=FirstPage1;temppage!=NULL;temppage=temppage->NextPage1){
		if(temppage->Items1[0].s_code[0] != NULL) {
			strcpy(code,temppage->Items1[0].s_code);
			GetDesc(code);

			mkln(2,temppage->Items1[0].s_code,6);
			mkln(12,earn.ea_desc,30);
			tedit((char*)&temppage->Items1[0].s_income,
					"_,___,_0_.__-",txt_buff,R_DOUBLE);
			mkln(46,txt_buff,13);

			tot_ben += temppage->Items1[0].s_income;

		}
		if(temppage->Items1[0].s_ded_code[0] != NULL) {
			/***** DEDUCTIONS *****/
			if(strcmp(temppage->Items1[0].s_ded_code, "CPP") ==0)
				mkln(74,"CPP CONTRIBUTION",16);
			else if(strcmp(temppage->Items1[0].s_ded_code,
							 "UIC") ==0)
				mkln(74,"UIC PREMIUM",11);
			else if(strcmp(temppage->Items1[0].s_ded_code,
							 "TAX") ==0)
				mkln(74,"INCOME TAX",10);
			else{
			  strcpy(reg_pen.rg_code,
						temppage->Items1[0].s_ded_code);
			  strcpy(reg_pen.rg_pp_code,
						temppage->Items1[0].s_pp_code);

		  	  retval = get_reg_pen(&reg_pen,BROWSE,0,e_mesg);
			  if(retval == NOERROR){
			  	mkln(65,reg_pen.rg_code,6);
				mkln(74,reg_pen.rg_desc,23);
			  }
			  else {
			    if(retval == UNDEF){
			       strcpy(deduction.dd_code,
						temppage->Items1[0].s_ded_code);
			       strcpy(deduction.dd_pp_code,
						temppage->Items1[0].s_pp_code);

			       retval=get_deduction(&deduction,BROWSE,0,e_mesg);
			       if(retval == NOERROR) {	
			    	  mkln(65,deduction.dd_code,6);
			    	  mkln(74,deduction.dd_desc,30);
			       }
			       else {
			   	  if(retval == UNDEF){
			             strcpy(csb_loan.cs_code,
						temppage->Items1[0].s_ded_code);

			    	     retval=get_loan(&csb_loan,BROWSE,0,e_mesg);
			    	     if(retval == NOERROR) {	
			    	  	 mkln(65,csb_loan.cs_code,6);
			    	 	 mkln(74,csb_loan.cs_desc,30);
			    	     }
				     else{
					if(strcmp(
				           temppage->Items1[0].s_ded_code, 
								"GARN") ==0)
			    	  	    mkln(65,"GARN  ",6);
					    mkln(74,"GARNISHMENT",11);
			             }		
			     	  }
			       }
			   }
			}
		}

		tedit((char*)&temppage->Items1[0].s_amount,
				"_,___,_0_.__-",txt_buff,R_DOUBLE);
		mkln(104,txt_buff,13);

		tot_ded += temppage->Items1[0].s_amount;

		}
		retval = PrintLine();
		if(retval == EXIT)	return(retval);
		if(retval < 0)	return(retval);
	   	if(temppage == CurLast1)
			break;
	   }
	   retval = SummaryAdv(0);
	   if(retval < 0) return retval;

	   retval = PrintLine();
	   if(retval == EXIT)	return(retval);
	   if(retval < 0)	return(retval);
	   retval = PrintLine();
	   if(retval == EXIT)	return(retval);
	   if(retval < 0)	return(retval);

	   fin_tot = tot_ben - tot_ded;

	   mkln(1,"TOTAL:",6);
	   tedit((char*)&tot_ben,"_,___,_0_.__-", txt_buff,R_DOUBLE);
	   mkln(46,txt_buff,13);
	   tedit((char*)&tot_ded,"_,___,_0_.__-",txt_buff,R_DOUBLE);
	   mkln(104,txt_buff,13);
	   tedit((char*)&fin_tot,"_,___,_0_.__-",txt_buff,R_DOUBLE);
	   mkln(119,txt_buff,13);

	   retval = PrintLine();
	   if(retval == EXIT)	return(retval);
	   if(retval < 0)	return(retval);
	   retval = PrintLine();
	   if(retval == EXIT)	return(retval);
	   if(retval < 0)	return(retval);

	   tot_ben = 0;
	   tot_ded = 0;
	   fin_tot = 0;

	}

	return(NOERROR);
}

static
PrintTotSum()
{
	char	code[7];
	char	txt_buff[132];
	int	retval;
	Page2	*temppage;
	double	fin_tot;

	tot_ben = 0;
	tot_ded = 0;

	/* check to see if item is already in list */
	if(CurLast2 != NULL) {
	   for(temppage=FirstPage2;temppage!=NULL;temppage=temppage->NextPage2){
		if(temppage->Items2[0].s_code[0] != NULL) {
			strcpy(code,temppage->Items2[0].s_code);
			GetDesc(code);

			mkln(2,temppage->Items2[0].s_code,6);
			mkln(12,earn.ea_desc,30);
			tedit((char*)&temppage->Items2[0].s_income,
					"_,___,_0_.__-",txt_buff,R_DOUBLE);
			mkln(46,txt_buff,13);

			tot_ben += temppage->Items2[0].s_income;

		}
		if(temppage->Items2[0].s_ded_code[0] != NULL) {
			/***** DEDUCTIONS *****/
			if(strcmp(temppage->Items2[0].s_ded_code, "CPP") ==0)
				mkln(74,"CPP CONTRIBUTION",16);
			else if(strcmp(temppage->Items2[0].s_ded_code,
							 "UIC") ==0)
				mkln(74,"UIC PREMIUM",11);
			else if(strcmp(temppage->Items2[0].s_ded_code,
							 "TAX") ==0)
				mkln(74,"INCOME TAX",10);
			else{
			  strcpy(reg_pen.rg_code,
						temppage->Items2[0].s_ded_code);
			  strcpy(reg_pen.rg_pp_code,
						temppage->Items2[0].s_pp_code);

		  	  retval = get_reg_pen(&reg_pen,BROWSE,0,e_mesg);
			  if(retval == NOERROR){
			  	mkln(65,reg_pen.rg_code,6);
				mkln(74,reg_pen.rg_desc,23);
			  }
			  else {
			    if(retval == UNDEF){
			       strcpy(deduction.dd_code,
						temppage->Items2[0].s_ded_code);
			       strcpy(deduction.dd_pp_code,
						temppage->Items2[0].s_pp_code);

			       retval=get_deduction(&deduction,BROWSE,0,e_mesg);
			       if(retval == NOERROR) {	
			    	  mkln(65,deduction.dd_code,6);
			    	  mkln(74,deduction.dd_desc,30);
			       }
			       else {
			   	  if(retval == UNDEF){
			             strcpy(csb_loan.cs_code,
						temppage->Items2[0].s_ded_code);

			    	     retval=get_loan(&csb_loan,BROWSE,0,e_mesg);
			    	     if(retval == NOERROR) {	
			    	  	 mkln(65,csb_loan.cs_code,6);
			    	 	 mkln(74,csb_loan.cs_desc,30);
			    	     }
				     else{
					if(strcmp(
				           temppage->Items2[0].s_ded_code, 
								"GARN") ==0)
			    	  	    mkln(65,"GARN  ",6);
					    mkln(74,"GARNISHMENT",11);
			             }		
					
			     	  }
			       }
			   }
			}
		}

		tedit((char*)&temppage->Items2[0].s_amount,
				"_,___,_0_.__-",txt_buff,R_DOUBLE);
		mkln(104,txt_buff,13);

		tot_ded += temppage->Items2[0].s_amount;

		}
		retval = PrintLine();
		if(retval == EXIT)	return(retval);
		if(retval < 0)	return(retval);
	   	if(temppage == CurLast2)
			break;
	   }
	   retval = SummaryAdv(1);
	   if(retval < 0) return retval;

	   retval = PrintLine();
	   if(retval == EXIT)	return(retval);
	   if(retval < 0)	return(retval);
	   retval = PrintLine();
	   if(retval == EXIT)	return(retval);
	   if(retval < 0)	return(retval);

	   fin_tot = tot_ben - tot_ded;

	   mkln(1,"TOTAL:",6);
	   tedit((char*)&tot_ben,"_,___,_0_.__-", txt_buff,R_DOUBLE);
	   mkln(46,txt_buff,13);
	   tedit((char*)&tot_ded,"_,___,_0_.__-",txt_buff,R_DOUBLE);
	   mkln(104,txt_buff,13);
	   tedit((char*)&fin_tot,"_,___,_0_.__-",txt_buff,R_DOUBLE);
	   mkln(119,txt_buff,13);

	   retval = PrintLine();
	   if(retval == EXIT)	return(retval);
	   if(retval < 0)	return(retval);
	   retval = PrintLine();
	   if(retval == EXIT)	return(retval);
	   if(retval < 0)	return(retval);

	}

	return(NOERROR);
}
SummaryAdv(sum_flag)
/**** 
   Added by L.Robichaud
   sum_flag is equal to 0 when processing the register summary for the
   register to break up the totals by cost centers and equal to 1 when 
   processing the register total summary.
****/
int	sum_flag;
{
	int	retval;
	int	pp_flag;
	double	adv_amount;
	double	adj_amount;
	char	txt_buff[132];

	adv_amount = 0;
	adj_amount = 0;
	/* Make copy of position in pay earn file */
	scpy((char*)&tmp_pay_earn,(char*)&pay_earn,sizeof(pay_earn));

	man_chq.mc_emp_numb[0] = '\0';
	man_chq.mc_date = 0;
	flg_reset(MAN_CHQ);

	/* get all the advances and add together */
	for(;;){
		retval = get_n_man_chq(&man_chq,BROWSE,1,FORWARD,e_mesg) ;
		if( retval == EFL)
			break;
		if( retval < 0 ) {
			DispError((char *)&s_sth1,e_mesg) ;
			roll_back(e_mesg);
			return(retval) ;
		}
		/* Make sure advance is for current pay run */
		strcpy(pay_earn.pe_numb,man_chq.mc_emp_numb);
		pay_earn.pe_pp = tmp_pay_earn.pe_pp;
		pay_earn.pe_date = man_chq.mc_date;
		flg_reset(PP_EARN);
		pp_flag = 0;
		for(;;){
			retval=get_n_pp_earn(&pay_earn,BROWSE,0,FORWARD,e_mesg);
			if(retval < 0 && retval != EFL) {
				fomen(e_mesg);
				get();
				return(retval);
			}
			if(retval ==  EFL)
				break;

			if(strcmp(pay_earn.pe_numb,man_chq.mc_emp_numb) != 0)
				break;
			if(man_chq.mc_date == pay_earn.pe_date){
				pp_flag = 1;
				break;
			}
		}
		if(pp_flag == 0) /* This flag makes sure of the pay  date */
			continue;

		if(sum_flag == 0){
			/* Check for cost center */
			if(old_cost_center != pay_earn.pe_cc)
				continue;
		}
		adv_amount += man_chq.mc_amount;
	}
	seq_over(MAN_CHQ);
	seq_over(PP_EARN);

	man_chq.mc_chq_numb = 0;
	flg_reset(MAN_CHQ);
	/* Get any advance adjustments for this cheque */
	for(;;){
		retval = get_n_man_chq(&man_chq,BROWSE,2,FORWARD,e_mesg) ;
		if( retval == EFL)
			break;
		if( retval < 0 ) {
			DispError((char *)&s_sth1,e_mesg) ;
			roll_back(e_mesg);
			return(retval) ;
		}
		/* Make sure advance adjustment is for current pay run */
		strcpy(pay_earn.pe_numb,man_chq.mc_emp_numb);
		pay_earn.pe_pp = 0;
		pay_earn.pe_date = man_chq.mc_chq_numb;
		flg_reset(PP_EARN);
		pp_flag = 0;
		for(;;){
			retval=get_n_pp_earn(&pay_earn,BROWSE,0,FORWARD,e_mesg);
			if(retval < 0 && retval != EFL){
				fomen(e_mesg);
				get();
				return(retval);
			}
			if(retval ==  EFL)
				break;
			if(strcmp(pay_earn.pe_numb,man_chq.mc_emp_numb) != 0)
				break;
			if(man_chq.mc_chq_numb == pay_earn.pe_date){
				pp_flag = 1; /* proper pay dates */
				break;
			}
		}
		if(pp_flag == 0) /* This flag makes sure of the pay date */
			continue;
		if(sum_flag == 0){
			/* Check for cost center */
			if(old_cost_center != pay_earn.pe_cc)
				continue;
		}
		adj_amount += man_chq.mc_amount;
	}
	seq_over(MAN_CHQ);
	seq_over(PP_EARN);

	if(adj_amount > 0){
		/***** DEDUCTIONS *****/
		mkln(74,"ADVANCE ADJUSTMENT",19);
		tedit((char*)&adj_amount,"_,___,_0_.__-",txt_buff,R_DOUBLE);
		mkln(104,txt_buff,13);
		tot_ded += adj_amount;
		retval = PrintLine();
		if(retval == EXIT)	return(retval);
		if(retval < 0)	return(retval);
	}
	if(adv_amount > 0){
		/***GetDesc(code);
		mkln(2,code,6);****/
		mkln(12,"ADVANCE",30);
		tedit((char*)&adv_amount,
			"_,___,_0_.__-",txt_buff,R_DOUBLE);
		mkln(46,txt_buff,13);
		tot_ben += adv_amount;
		retval = PrintLine();
		if(retval == EXIT)	return(retval);
		if(retval < 0)	return(retval);
	}

	/* Reset the pay earnings file */
	scpy((char*)&pay_earn,(char*)&tmp_pay_earn,sizeof(tmp_pay_earn));
	flg_reset(MAN_CHQ);
	flg_reset(PP_EARN);
	return(NOERROR);
}
static
PrintLine()
{
	if( prnt_line()<0 )	return(REPORT_ERR);

	if(linecnt > PG_SIZE) {
		retval = PrntHdg(0);
		if(retval < 0)	return(retval);
		if(retval == EXIT)	return(retval);
	}
	return(NOERROR);
}
