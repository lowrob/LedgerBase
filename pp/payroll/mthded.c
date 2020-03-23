/******************************************************************************
		Sourcename   : mthded.c
		System       : Budgetary Financial system.
		Module       : Fixed Assets System : Reports
		Created on   : 92-04-30
		Created  By  : Andre Cormier 
		Cobol Source : 

******************************************************************************
About the file:	
	This file has routines to print Fixed Asset Report. 
	It is called by the file ppayrep.c .

History:
Programmer      Last change on    Details

L.Robichaud	1993/10/13	Pass new parameter to close_rep function for 
				a banner to print with 3000 family.

******************************************************************************/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_pp.h>
#include <bfs_com.h>
#include <repdef.h>

#define CONTINUE	10
#define EXIT		12

#ifdef ENGLISH
#define PRINTER 	'P'
#define DISPLAY		'D'
#define FILE_IO		'F'
#else
#define PRINTER		'I'

#define DISPLAY		'A'
#define FILE_IO		'D'
#endif

static Pa_rec		pa_rec;
static Deduction	deduction;
static Emp		emp_rec;
static Emp_earn		emp_earn;
static Emp_dh		emp_dh;
static Emp_ln_his	emp_ln_his;
static Emp_gr_his	emp_garn_his;
static Csb_loan		loan;
static Pay_param	pay_param;
static Reg_pen		reg_pen;

/*  Data items for storing the key range end values */
static char	ded1[7];
static char	ded2[7];
static long	date1;
static long	date2;
static double	month_to_date;
static	double	year_to_date;
	
static int	flag;		/* flag to print the employee or not */
static int	PG_SIZE;
static int	retval;
static char 	discfile[15];	/* for storing outputfile name */
static short	pgcnt; 		/* for page count */
static char	resp[2];	/* for storing response */
static short	copies;

extern 	char 	e_mesg[200];	/* for storing error messages */
static	char	old_emp_numb[13];
static	short	first_time=0;
static	char	last_emp_numb[13];
static	char	last_reg_pen[7];
static	double	tot_mth_date;
static	double	tot_yr_date;
static	int	print_flag;
mthded()
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
			STRCPY( discfile, "mthded.dat" );
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

	strcpy( ded1, "     1");
	strcpy( ded2, "ZZZZZZ" );
	retval = GetDedRange( ded1,ded2 );
	if(retval < 0) return(retval);

	date1 = 0;
	date2 = get_date();
	retval = GetDateRange( &date1,&date2 );
	if(retval < 0) return(retval);

	if( (retval=Confirm())<=0 )
		return(retval);

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

	retval = PrintRep();

	close_rep();	/* close output file */
	close_dbh();
	return(NOERROR);
}

/******************************************************************************
Main logic of the program
******************************************************************************/
static
PrintRep()
{
	retval = get_pay_param( &pay_param, BROWSE, 1, e_mesg );
	if( retval<1 ){
		return(DBH_ERR);
	}

	/******** Print CPP UIC, INCOME TAX portion of the report *******/
	retval=Prnt_C_U_I();
	if((retval==EXIT)||(retval<0))
		return(retval);

	/******* Print the REGULAR DEDUCTION portion of the report *********/
	retval=PrntRegPen();
	if((retval==EXIT)||(retval<0))
		return(retval);

	/************ Print deduction from the emp ded hist file ***********/
	retval=PrntEmpDed();
	if((retval==EXIT)||(retval<0))
		return(retval);

	/***** Print Canada Savings Bonds from the emp loan hist file ******/
	retval=PrntLoanDed();
	if((retval==EXIT)||(retval<0))
		return(retval);

	retval=PrntGarnDed();
	if((retval==EXIT)||(retval<0))
		return(retval);


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
Prnt_C_U_I()
{
	year_to_date = 0.0;
	month_to_date = 0.0;

	tot_yr_date = 0.0;
	tot_mth_date = 0.0;

	for(print_flag = 0;print_flag < 3;print_flag++) {
		switch(print_flag) {
		case	0:
			if((strcmp("CPP",ded1) < 0) ||
			   (strcmp("CPP",ded2) > 0)) {
				continue;
			}
			break;
		case	1:
			if((strcmp("UIC",ded1) < 0) ||
			   (strcmp("UIC",ded2) > 0)) {
				continue;
			}
			break;
		case	2:
			if((strcmp("TAX",ded1) < 0) ||
			  (strcmp("TAX",ded2) > 0)) {
				continue;
			}
			break;
		}
		
		retval = PrntHdg();
		if(retval == EXIT) break;
		retval = PrntHdgDed(print_flag);
		if(retval == EXIT) break;

		month_to_date = 0;
		first_time = 0;

		emp_earn.en_numb[0] = '\0';
		emp_earn.en_date = 0;
		emp_earn.en_pp = 0;
		emp_earn.en_week = 0;
		flg_reset(EMP_EARN);

		for(;;) {
			retval = get_n_emp_earn(&emp_earn,BROWSE,0,FORWARD,
				e_mesg);
			if(retval == EFL) 
				break;
			if(retval < 0){
				fomer(e_mesg);
				get();
				return(retval);
			}
	
			if(first_time == 0) {
				strcpy(old_emp_numb, emp_earn.en_numb);
				first_time = 1;
			}

			if(strcmp(old_emp_numb,emp_earn.en_numb) != 0) {
				tot_mth_date += month_to_date;
				tot_yr_date += year_to_date;
				retval = PrntRec(print_flag);
				year_to_date = 0.0;
				month_to_date = 0.0;
				if(retval==EXIT)	return(retval);
				strcpy(old_emp_numb, emp_earn.en_numb);
			}

			if((emp_earn.en_date / 10000) == (date2  / 10000)){
				switch(print_flag) {
				case	0:
					year_to_date += emp_earn.en_cpp;
					break;
				case	1:
					year_to_date += emp_earn.en_uic;
					break;
				case	2:
					year_to_date += emp_earn.en_tax;
					break;
				}
			}
	
			if((emp_earn.en_date < date1)  ||
			   (emp_earn.en_date > date2) ) {
				continue;
			}

			switch(print_flag) {
			case	0:
				month_to_date += emp_earn.en_cpp;
				break;
			case	1:
				month_to_date += emp_earn.en_uic;
				break;
			case	2:
				month_to_date += emp_earn.en_tax;
				break;
			}
		}/*end loop*/

		seq_over(EMP_EARN);
	
		if(first_time == 1) {
			tot_mth_date += month_to_date;
			tot_yr_date += year_to_date;
			retval = PrntRec(print_flag);
			year_to_date = 0.0;
			month_to_date = 0.0;
			if(retval==EXIT)	return(retval);

			retval = PrntTot();
			if(retval == EXIT) break;
			if(retval < 0) {
				return(retval);
			}
		}
	}/*end loop*/
	
	return(NOERROR);
}

static
PrntRegPen()
{

	print_flag = 89;

	strcpy(reg_pen.rg_code, ded1);
	reg_pen.rg_pp_code[0] = '\0';

	for(;;){
		inc_str(reg_pen.rg_pp_code,
			sizeof(reg_pen.rg_pp_code)-1,FORWARD);
		flg_reset(REG_PEN);
		retval = get_n_reg_pen(&reg_pen,BROWSE,0,FORWARD,e_mesg);
		if(retval==EFL)	break;
		if(retval < 0) {
			fomen(e_mesg);
			get();
			return(retval);
		}	

		/* check if the reg pen code is within the code range*/
		if((strcmp(reg_pen.rg_code,ded1)<0))
			continue;
		if((strcmp(reg_pen.rg_code,ded2)>0))
			break;

		retval = PrntHdg();
		if(retval == EXIT) break;
		retval = PrntHdgDed(print_flag);
		if(retval == EXIT) break;

		emp_earn.en_numb[0] = NULL;
		emp_earn.en_date = 0;
		emp_earn.en_pp = 0;
		emp_earn.en_week = 0;
		flg_reset(EMP_EARN);

		for(;;) {
			retval=get_n_emp_earn(&emp_earn,BROWSE,0,FORWARD, e_mesg);
			if(retval == EFL) 
				break; /*get next pension*/
			if(retval<0){
				fomer(e_mesg);
				get();
				return(retval);
			}

			if(first_time == 0){
				strcpy(old_emp_numb,emp_earn.en_numb);
				year_to_date = 0.0;
				month_to_date = 0.0;
				first_time = 1;
			}

			/* Check if the employee number has changed */
			if(strcmp(old_emp_numb,emp_earn.en_numb)!=0){
				/* if year & month to date have values print */
				if(year_to_date && month_to_date){
					tot_mth_date += month_to_date;
					tot_yr_date += year_to_date;
					retval = PrntRec(99);
					if(retval == EXIT) break;
					year_to_date = 0.0;
					month_to_date = 0.0;
				}
				strcpy(old_emp_numb,emp_earn.en_numb);

			}

			/* Check for the same pension type */
			if(strcmp(emp_earn.en_reg_pen, reg_pen.rg_code))
				continue;

			if((emp_earn.en_date / 10000) == (date2  / 10000)){
				/*accumulate the regular deduction*/
				year_to_date += (emp_earn.en_reg1 +
						 emp_earn.en_reg2 +
						 emp_earn.en_reg3 +
						 emp_earn.en_reg_prior +
						 emp_earn.en_reg_opt +
						 emp_earn.en_reg_nonm);

			}

			if((emp_earn.en_date < date1)  ||
			  (emp_earn.en_date > date2) )
				continue;

			/*accumulate the regular deduction*/
			month_to_date += (emp_earn.en_reg1 +
					  emp_earn.en_reg2 +
					  emp_earn.en_reg3 +
					  emp_earn.en_reg_prior +
					  emp_earn.en_reg_opt +
					  emp_earn.en_reg_nonm);

		}/*end loop*/

		seq_over(EMP_EARN);

		retval = PrntTot();
		if(retval == EXIT) return(retval);
		if(retval < 0)	return(retval);

	}/*end loop*/

	seq_over(REG_PEN);

	return(NOERROR);
}

static
PrntEmpDed()
{
	char	tmp_code[7];
	char	old_code[7];
	double	tmp_var;

	first_time = 0;
	print_flag = 99;
	emp_dh.edh_code[0] = '\0';
	emp_dh.edh_group[0] = '\0';
	emp_dh.edh_date = 0;
	emp_dh.edh_numb[0] = '\0';

	flg_reset(EMP_DED_HIS);

	for(;;){
		retval = get_n_emp_dhis(&emp_dh,BROWSE,2,FORWARD,e_mesg);
		if(retval==EFL)	{
			strcpy(emp_dh.edh_code,old_code);
			break;
		}
		if(retval < 0) {
			fomen(e_mesg);
			get();
			return(retval);
		}	
		strcpy(old_code,emp_dh.edh_code);

		if(emp_dh.edh_code[0] == '\0')
			continue;
	
		/* check if the emp ded code is within the code range*/
		if((strcmp(emp_dh.edh_code,ded1)<0) ||
		   (strcmp(emp_dh.edh_code,ded2)>0))
			continue;

		if((emp_dh.edh_date / 10000) == (date2  / 10000)){
			/*accumulate the regular deduction*/
			tmp_var = emp_dh.edh_amount;
		}
		else {
			tmp_var = 0.0;
		}

		year_to_date += tmp_var;

		/* Check the date range */
	
		if((emp_dh.edh_date < date1)  ||
		   (emp_dh.edh_date > date2) ) {
			continue;
		}

		if(first_time == 0){
			retval = PrntHdg();
			if(retval == EXIT) break;
			retval = PrntHdgDed(print_flag);
			if(retval == EXIT) break;
			first_time = 1;
			strcpy(old_emp_numb,emp_dh.edh_numb);
			strcpy(tmp_code,emp_dh.edh_code);
		}

		if(strcmp(tmp_code,emp_dh.edh_code) != 0 ) {
			year_to_date -= tmp_var;
			tot_mth_date += month_to_date;
			tot_yr_date += year_to_date;
			retval = PrntRec(99);
			if(retval == EXIT) break;
			year_to_date = tmp_var;
			month_to_date = 0.0;
			retval = PrntTot();
			if(retval == EXIT) break;
			if(retval < 0) {
				return(retval);
			}
			retval = PrntHdg();
			if(retval == EXIT) break;
			retval = PrntHdgDed(print_flag);
			if(retval == EXIT) break;
			strcpy(old_emp_numb,emp_dh.edh_numb);
			strcpy(tmp_code,emp_dh.edh_code);
		}

		if(strcmp(tmp_code,emp_dh.edh_code) == 0 &&
		   strcmp(old_emp_numb,emp_dh.edh_numb) != 0) {
			year_to_date -= tmp_var;
			tot_mth_date += month_to_date;
			tot_yr_date += year_to_date;
			retval = PrntRec(99);
			year_to_date = tmp_var;
			month_to_date = 0.0;
			strcpy(old_emp_numb,emp_dh.edh_numb);	
			if(retval == EXIT)	return(retval);
		}
	
		/* Accumulate the pension */
		month_to_date += emp_dh.edh_amount;

	}/*end loop*/

	seq_over(EMP_DED_HIS);

	if(first_time == 1) {
		tot_mth_date += month_to_date;
		tot_yr_date += year_to_date;
		retval = PrntRec(99);
		if(retval == EXIT) return(retval);
		retval = PrntTot();
		if(retval == EXIT) return(retval);
		if(retval < 0) {
			return(retval);
		}
	}
	
	return(NOERROR);
}
	
static
PrntLoanDed()
{
	char	tmp_code[7];
	double	tmp_var;

	print_flag = 79;
	first_time = 0;

	month_to_date = 0;
	year_to_date = 0;

	emp_ln_his.elh_code[0] = '\0';
	emp_ln_his.elh_numb[0] = '\0';
	emp_ln_his.elh_date = 0;

	flg_reset(EMP_LOAN_HIS);

	for(;;){
		retval = get_n_emp_lhis(&emp_ln_his,BROWSE,1,FORWARD,e_mesg);
		if(retval==EFL)	break;
		if(retval < 0) {
			fomen(e_mesg);
			get();
			return(retval);
		}	

		if(emp_ln_his.elh_code[0] == '\0')
			continue;
	
		/* check if the emp ded code is within the code range*/
		if((strcmp(emp_ln_his.elh_code,ded1)<0) ||
		   (strcmp(emp_ln_his.elh_code,ded2)>0))
			continue;

		if((emp_ln_his.elh_date / 10000) == (date2  / 10000)){
			/*accumulate the regular deduction*/
			tmp_var = emp_ln_his.elh_amount;
		}
		else {
			tmp_var = 0.0;
		}

		year_to_date += tmp_var;

		/* Check the date range */
	
		if((emp_ln_his.elh_date < date1)  ||
		   (emp_ln_his.elh_date > date2) ) {
			continue;
		}

		if(first_time == 0){
			retval = PrntHdg();
			if(retval == EXIT) break;
			retval = PrntHdgDed(print_flag);
			if(retval == EXIT) break;
			first_time = 1;
			strcpy(old_emp_numb,emp_ln_his.elh_numb);
			strcpy(tmp_code,emp_ln_his.elh_code);
		}

		if(strcmp(tmp_code,emp_ln_his.elh_code) != 0 ) {
			year_to_date -= tmp_var;
			tot_mth_date += month_to_date;
			tot_yr_date += year_to_date;
			retval = PrntRec(79);
			if(retval == EXIT) break;
			year_to_date = tmp_var;
			month_to_date = 0.0;
			retval = PrntTot();
			if(retval == EXIT) break;
			if(retval < 0) {
				return(retval);
			}
			retval = PrntHdg();
			if(retval == EXIT) break;
			retval = PrntHdgDed(print_flag);
			if(retval == EXIT) break;
			strcpy(old_emp_numb,emp_ln_his.elh_numb);
			strcpy(tmp_code,emp_ln_his.elh_code);
		}

		if(strcmp(tmp_code,emp_ln_his.elh_code) == 0 &&
		   strcmp(old_emp_numb,emp_ln_his.elh_numb) != 0) {
			year_to_date -= tmp_var;
			tot_mth_date += month_to_date;
			tot_yr_date += year_to_date;
			retval = PrntRec(79);
			year_to_date = tmp_var;
			month_to_date = 0.0;
			strcpy(old_emp_numb,emp_ln_his.elh_numb);	
			if(retval == EXIT)	return(retval);
		}
	
		/* Accumulate the pension */
		month_to_date += emp_ln_his.elh_amount;

	}/*end loop*/

	seq_over(EMP_LOAN_HIS);

	if(first_time == 1) {
		tot_mth_date += month_to_date;
		tot_yr_date += year_to_date;
		retval = PrntRec(79);
		if(retval == EXIT) return(retval);

		retval = PrntTot();
		if(retval == EXIT) return(retval);
		if(retval < 0) {
			return(retval);
		}
	}
	
	return(NOERROR);
}
static
PrntGarnDed()
{
	char	tmp_code[7];
	double	tmp_var;

	if((strcmp("GARN",ded1)<0) ||
	   (strcmp("GARN",ded2)>0))
		return(NOERROR);;

	print_flag = 59;
	first_time = 0;

	month_to_date = 0;
	year_to_date = 0;

	emp_garn_his.egh_numb[0] = '\0';
	emp_garn_his.egh_pp = 0;
	emp_garn_his.egh_date = 0;
	emp_garn_his.egh_pr_cd = 0;
	emp_garn_his.egh_seq = 0;

	flg_reset(EMP_GARN_HIS);

	for(;;){
		retval = get_n_emp_ghis(&emp_garn_his,BROWSE,0,FORWARD,e_mesg);
		if(retval==EFL)	break;
		if(retval < 0) {
			fomen(e_mesg);
			get();
			return(retval);
		}	

		/* check if the emp ded code is within the code range*/
		if((emp_garn_his.egh_date / 10000) == (date2  / 10000)){
			/*accumulate the regular deduction*/
			tmp_var = emp_garn_his.egh_amount;
		}
		else {
			tmp_var = 0.0;
		}

		year_to_date += tmp_var;

		/* Check the date range */
	
		if((emp_garn_his.egh_date < date1)  ||
		   (emp_garn_his.egh_date > date2) ) {
			continue;
		}

		if(first_time == 0){
			retval = PrntHdg();
			if(retval == EXIT) break;
			retval = PrntHdgDed(print_flag);
			if(retval == EXIT) break;
			first_time = 1;
			strcpy(old_emp_numb,emp_garn_his.egh_numb);
			strcpy(tmp_code,"GARN");
		}

		if(strcmp(tmp_code,"GARN") != 0 ) {
			year_to_date -= tmp_var;
			tot_mth_date += month_to_date;
			tot_yr_date += year_to_date;
			retval = PrntRec(59);
			if(retval == EXIT) break;
			year_to_date = tmp_var;
			month_to_date = 0.0;
			retval = PrntTot();
			if(retval == EXIT) break;
			if(retval < 0) {
				return(retval);
			}
			retval = PrntHdg();
			if(retval == EXIT) break;
			retval = PrntHdgDed(print_flag);
			if(retval == EXIT) break;
			strcpy(old_emp_numb,emp_garn_his.egh_numb);
			strcpy(tmp_code,"GARN");
		}

		if(strcmp(tmp_code,"GARN") == 0 &&
		   strcmp(old_emp_numb,emp_garn_his.egh_numb) != 0) {
			year_to_date -= tmp_var;
			tot_mth_date += month_to_date;
			tot_yr_date += year_to_date;
			retval = PrntRec(59);
			year_to_date = tmp_var;
			month_to_date = 0.0;
			strcpy(old_emp_numb,emp_garn_his.egh_numb);	
			if(retval == EXIT)	return(retval);
		}
	
		/* Accumulate the pension */
		month_to_date += emp_garn_his.egh_amount;

	}/*end loop*/

	seq_over(EMP_GARN_HIS);

	if(first_time == 1) {
		tot_mth_date += month_to_date;
		tot_yr_date += year_to_date;
		retval = PrntRec(59);
		if(retval == EXIT) return(retval);

		retval = PrntTot();
		if(retval == EXIT) return(retval);
		if(retval < 0) {
			return(retval);
		}
	}
	
	return(NOERROR);
}
	
/******************************************************************************
Prints the headings of the report GROSS EARNINGS FOR PAY PERIOD
******************************************************************************/
static
PrntHdg()	/* Print heading  */
{
	short	offset;
	long	sysdt ;

	if( pgcnt && term < 99)   /* new page and display */
		if(next_page()<0) return(EXIT);	
		
	if( pgcnt || term < 99) { /* if not the first page or display */
		if( rite_top()<0 ) return( -1 );	/* form_feed */
	}
/***
	else
***/
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
	if(retval == EXIT)	return(retval);
	if(retval < 0)	return(REPORT_ERR);

	offset = ( LNSZ-strlen(pa_rec.pa_co_name) )/2;
	mkln( offset, pa_rec.pa_co_name, strlen(pa_rec.pa_co_name) );
	retval = PrintLine(); 
	if(retval == EXIT)	return(retval);
	if(retval < 0)	return(REPORT_ERR);

#ifdef ENGLISH
	mkln((LNSZ-24)/2,"MONTHLY DEDUCTION REPORT", 24 );
#else
	mkln((LNSZ-25)/2,"Translate               ", 25 );
#endif
	retval = PrintLine(); 
	if(retval == EXIT)	return(retval);
	if(retval < 0)	return(REPORT_ERR);

#ifdef ENGLISH
	sprintf(e_mesg,"From %ld To %ld",date1,date2);
	mkln((LNSZ-(strlen(e_mesg)))/2,e_mesg,strlen(e_mesg));
#else
	sprintf(e_mesg,"From %ld To %ld",date1,date2);
	mkln((LNSZ-(strlen(e_mesg)))/2,e_mesg,strlen(e_mesg));
#endif

	retval = PrintLine(); 
	if(retval == EXIT)	return(retval);
	if(retval < 0)	return(REPORT_ERR);
	retval = PrintLine(); 
	if(retval == EXIT)	return(retval);
	if(retval < 0)	return(REPORT_ERR);

	return(NOERROR);
}

/******************************************************************************
Prints the headings of the report GROSS EARNINGS FOR PAY PERIOD
******************************************************************************/
static
PrntHdgDed(flag)	/* Print heading  */
int	flag;
{
	switch(flag) {
	case	0:
		mkln(1,"DESCRIPTION:",12);
		mkln(14,"CPP",3);
		break;
	case	1:
		mkln(1,"DESCRIPTION:",12);
		mkln(14,"UIC",3);
		break;
	case	2:
		mkln(1,"DESCRIPTION:",12);
		mkln(14,"INCOME TAX",10);
		break;
	case	59:
		mkln(1,"DESCRIPTION:",12);
		mkln(14,"GARNISHMENT",11);
		break;
	case 	79:
		mkln(1,"DEDUCTION CODE:",15);
		mkln(16,emp_ln_his.elh_code,6);
		strcpy(loan.cs_code,emp_ln_his.elh_code);

		retval = get_loan(&loan,BROWSE,0,e_mesg);
		if((retval < 0) && (retval != EFL)) {
			fomen(e_mesg);
			get();
			return(retval);
		}
		if(retval==UNDEF) 
			loan.cs_desc[0] = '\0';

		mkln(23,"DESCRIPTION:",12);
		mkln(46,loan.cs_desc,30);
		break;
	case	89:
		mkln(1,"DEDUCTION CODE:",15);
		mkln(16,reg_pen.rg_code,6);
		mkln(23,"DESCRIPTION:",12);
		mkln(37,reg_pen.rg_desc,30);

		break;
	case 	99:
		mkln(1,"DEDUCTION CODE:",15);
		mkln(16,emp_dh.edh_code,6);

		strcpy(deduction.dd_code,emp_dh.edh_code);
		deduction.dd_pp_code[0]='\0';

		flg_reset(DEDUCTION);

		retval = get_n_deduction(&deduction,BROWSE,0,FORWARD,e_mesg);
		if((retval < 0) && (retval != EFL)) {
			fomen(e_mesg);
			get();
			return(retval);
		}

		if((retval==EFL) ||
		(strcmp(deduction.dd_code,emp_dh.edh_code)!=0))
			deduction.dd_desc[0] = '\0';

		mkln(23,"DESCRIPTION:",12);
		mkln(46,deduction.dd_desc,30);

		seq_over(DEDUCTION);

		break;
	}


	retval = PrintLine(); 
	if(retval == EXIT)	return(retval);
	if(retval < 0)	return(REPORT_ERR);
	retval = PrintLine(); 
	if(retval == EXIT)	return(retval);
	if(retval < 0)	return(REPORT_ERR);

	mkln(1,"EMPLOYEE",8);
	mkln(25,"EMPLOYEE NAME",13);
	mkln(58,"SIN",3);
	mkln(67,"MONTH TO DATE",13);
	mkln(85,"YEAR TO DATE",12);
	
	retval = PrintLine(); 
	if(retval == EXIT)	return(retval);
	if(retval < 0)	return(REPORT_ERR);

	mkln(2,"NUMBER",6);

	retval = PrintLine(); 
	if(retval == EXIT)	return(retval);
	if(retval < 0)	return(REPORT_ERR);
	retval = PrintLine(); 
	if(retval == EXIT)	return(retval);
	if(retval < 0)	return(REPORT_ERR);

	return(NOERROR);
}
/******************************************************************************
Prints the detail line for employee schedule file
******************************************************************************/
static
PrntRec(ded_type)
short	ded_type;
{
	char	txt_buff[132];
	char	name[50];
	char	sin[12];
	int	retval;

	strcpy(last_emp_numb,emp_rec.em_numb);/*keep last emp numb*/
	strcpy(last_reg_pen,emp_rec.em_reg_pen);/*keep last reg pen*/

	strcpy(emp_rec.em_numb,old_emp_numb);
	retval = get_employee(&emp_rec,BROWSE,0,e_mesg);
	if(retval < 0) {
		fomen(e_mesg);
		get();
		return(retval);
	}

	mkln(1,emp_rec.em_numb,12);
	sprintf(txt_buff,"%s, %s",emp_rec.em_last_name,
				emp_rec.em_first_name);
	mkln(14,txt_buff,strlen(txt_buff));

	strncpy(sin,emp_rec.em_sin,3);
	sin[3] = '\0';
	strcat(sin,"-");
	sin[4] = '\0';
	strcat(sin,emp_rec.em_sin+3,3);
	sin[7] = '\0';
	strcat(sin,"-");
	sin[8] = '\0';
	strcat(sin,emp_rec.em_sin+6,3);
	mkln(54,sin,11);
	tedit((char *)&month_to_date,"_,___,_0_.__-",txt_buff,R_DOUBLE);
	mkln(67,txt_buff,13);
	tedit((char *)&year_to_date,"_,___,_0_.__-",txt_buff,R_DOUBLE);
	mkln(85,txt_buff,13);
	retval = PrintLine(); 
	if(retval == EXIT)	return(retval);
	if(retval < 0)	return(REPORT_ERR);

	/****************************************************************/
	/*    set the emp file pointer back to correct position	        */
	/****************************************************************/

	if(ded_type == 99){
		strcpy(emp_rec.em_reg_pen,last_reg_pen);
		strcpy(emp_rec.em_numb,last_emp_numb);

		flg_reset(EMPLOYEE);
	
		retval = get_n_employee(&emp_rec,BROWSE,4,FORWARD,e_mesg);
		if(retval==EFL)	return(retval);
		if(retval < 0) {
			fomen(e_mesg);
			get();
			return(retval);
		}	

		seq_over(EMPLOYEE);
	}

	return(NOERROR);
}
/******************************************************************************
Prints the detail line for employee schedule file
******************************************************************************/
static
PrntTot()
{
	char	txt_buff[132];
	int	retval;
/***
	retval = PrintLine(); 
	if(retval == EXIT)	return(retval);
	if(retval < 0)	return(REPORT_ERR);
***/
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(1,"Total :",7);
	tedit((char *)&tot_mth_date,"_,___,_0_.__-",txt_buff,R_DOUBLE);
	mkln(67,txt_buff,13);
	tedit((char *)&tot_yr_date,"_,___,_0_.__-",txt_buff,R_DOUBLE);
	mkln(85,txt_buff,13);
	retval = PrintLine(); 
	if(retval == EXIT)	return(retval);
	if(retval < 0)	return(REPORT_ERR);

	tot_mth_date = 0;
	tot_yr_date = 0;

	return(NOERROR);
}
/******************************************************************************
Function that prints every line of the report 
******************************************************************************/
static
PrintLine()
{
	if(prnt_line() < 0 )	return(REPORT_ERR);

	if(linecnt > PG_SIZE){
		linecnt = 0;
		PrntHdg();
		PrntHdgDed(print_flag);
		if(retval < 0)	return(retval);
		if(retval == EXIT)	return(retval);
	}

	return(NOERROR);
}

