/******************************************************************************
		Sourcename   : t4prelist2.c
		System       : Budgetary Financial system.
		Module       : Fixed Assets System : Reports
		Created on   : 94-01-24
		Created  By  : Nicola McKee  
		Cobol Source : 

******************************************************************************
About the file:	
	This file has routines to print Fixed Asset Report. 
	It is called by the file t4rep.c .

History:
Programmer      Last change on    Details

L.Robichaud	1993/10/13	Pass new parameter to close_rep function for 
				a banner to print with 3000 family.
C.Leadbeater	1994/02/01	Print totals for all columns.

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
static	Pay_param	pay_param;
static Emp		emp_rec;
static T4_adj		t4_adj;
static Pay_param	pay_param;
static	Emp_earn	emp_earn;
static	Emp_ln_his	emp_ln_his;
static	Emp_dh		emp_dd_his;

static int	flag;		/* flag to print the employee or not */
static int	PG_SIZE;
static int	retval;
static char 	discfile[15];	/* for storing outputfile name */
static short	pgcnt; 		/* for page count */
static char	resp[2];	/* for storing response */
static short	copies;
static	double	reg_pen;	/* to add rate 1,2,3 and pension
				buy back together */
static	double	earn_date;

static double	total_other_tax;
static double	total_union_du;
static double	total_pen_adj;
static double	total_reg_pen;

extern char 	e_mesg[80];	/* for storing error messages */
/*** Used for debuging
static FILE *fd;
static int log_fd;
***/

t4prelist2()
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
			PG_SIZE = 20;
			break;
		case FILE_IO:
			resp[0]='F';
			STRCPY( discfile, "t4prelist2.dat" );
			if((retval = GetFilename(discfile))<0 )
				return(retval);
			PG_SIZE = 60;
			break;
		case PRINTER:
		default:
			resp[0]='P';
			discfile[0]= '\0';
			PG_SIZE = 60;
			break;
	}

	copies = 1;
	if( *resp=='P' ){
		if((retval = GetNbrCopies(&copies))<0 )
			return(retval);
	}

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

	retval = PrintT4();

	close_rep();	/* close output file */
	close_dbh();
	return(retval);
}

/******************************************************************************
Main logic of the program
******************************************************************************/
static
PrintT4()
{
	int	old_week;
	char	old_emp_numb[13];

	total_other_tax = 0;
	total_union_du = 0;
	total_pen_adj = 0;
	total_reg_pen = 0;

	retval = PrntHdg();
	if(retval < 0)	return(retval);

	t4_adj.ta_numb[0] = '\0';

	flg_reset(T4_ADJ);

	for(;;) {
		retval = get_n_t4_adj(&t4_adj,BROWSE,0,FORWARD,e_mesg);

		if(retval ==EFL) 
			break;

		strcpy(emp_rec.em_numb,t4_adj.ta_numb);

		retval = get_employee(&emp_rec,BROWSE,0,e_mesg);
		if(retval < 0) {
			fomen(e_mesg);
			get();
			return(ERROR);
		}
		strcpy(emp_earn.en_numb, emp_rec.em_numb);

		retval = GetRegPen();
		retval = PrintRec();
		if(retval < 0)	 return(retval);

		total_other_tax += t4_adj.ta_other_tax;
		total_union_du += t4_adj.ta_union_du;
		total_pen_adj += t4_adj.ta_pen_adj;
		total_reg_pen+= reg_pen;
	}

	seq_over(T4_ADJ);

	retval = PrintTotals();
	if(retval < 0)	 return(retval);

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
/******************************************************************************
Prints the headings of the report T4 Prelist - Deductions      
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
	if( PrintLine()<0 )	return(REPORT_ERR);

	offset = ( LNSZ-strlen(pa_rec.pa_co_name) )/2;
	mkln( offset, pa_rec.pa_co_name, strlen(pa_rec.pa_co_name) );
	if( PrintLine()<0 )	return(REPORT_ERR);

#ifdef ENGLISH
	mkln((LNSZ-10)/2,"T4 PRELIST - DEDUCTIONS", 24 );
#else
	mkln((LNSZ-10)/2,"T4 PRELIST - DEDUCTIONS", 24 );
#endif
	if( PrintLine()<0 )	return(REPORT_ERR);
	if( PrintLine()<0 )	return(REPORT_ERR);

	mkln(3,"EMPLOYEE",8);
	mkln(40,"SIN",3);
	mkln(52,"OTHER",5);
	mkln(65,"UNION",5);
	mkln(78,"PENSION",7);
	mkln(90,"REG PENSION",11);
	mkln(110,"REGISTERED",10);
	
	if( PrintLine()<0 )	return(REPORT_ERR);

	mkln(4,"NUMBER",6);
	mkln(22,"EMPLOYEE NAME",13);
	mkln(50,"TAXABLES",8);
	mkln(65,"DUES",4);
	mkln(77,"ADJUSTMENT",10);
	mkln(92,"NUMBER",6);
	mkln(112,"PENSION",7);

	if( PrintLine()<0 )	return(REPORT_ERR);
	if( PrintLine()<0 )	return(REPORT_ERR);

	return(NOERROR);
}
/******************************************************************************
Prints the detail line from the T4 adjustment file
******************************************************************************/

static
PrintRec()
{
	char	txt_buff[132];

	mkln(1,t4_adj.ta_numb,12);
	strcpy(txt_buff,emp_rec.em_first_name);
	strcat(txt_buff," ");
	strcat(txt_buff,emp_rec.em_last_name);
	mkln(15,txt_buff,strlen(txt_buff));
	mkln(37,emp_rec.em_sin,13);
	tedit((char *)&t4_adj.ta_other_tax,"_,___,_0_.__",txt_buff,R_DOUBLE);
	mkln(48,txt_buff,12);
	tedit((char *)&t4_adj.ta_union_du,"_,___,_0_.__",txt_buff,R_DOUBLE);
	mkln(62,txt_buff,12);
	tedit((char *)&t4_adj.ta_pen_adj,"_,___,_0_.__",txt_buff,R_DOUBLE);
	mkln(76,txt_buff,12);
	mkln(90,t4_adj.ta_reg_pen_num,13);
	tedit((char *)&reg_pen,"_,___,_0_.__",txt_buff,R_DOUBLE);
	mkln(105,txt_buff,12);

	if( PrintLine()<0 )	return(REPORT_ERR);

	return(NOERROR);
}

/******************************************************************************
Function that prints every line of the report 
******************************************************************************/
static
PrintLine()
{
	if(prnt_line() < 0 )	return(REPORT_ERR);

	if(linecnt > PG_SIZE)
		PrntHdg();

	return(NOERROR);
}

/*-----------------------------------------------------------------------*/
/* Get the registered pension figure - from the loan history file        */
/*-----------------------------------------------------------------------*/
GetRegPen()
{
	int	retval;

	reg_pen = 0;
/*** Used for debuging
fd=fopen("test.lou","r");
log_fd=creat("testlou2",TXT_CRMODE);
****/

	retval = get_pay_param(&pay_param, BROWSE, 1, e_mesg) ;
	if(retval == ERROR) {
		return(ERROR) ;
	}

	emp_earn.en_date = pay_param.pr_cal_st_dt;
	emp_earn.en_pp = 0;
	emp_earn.en_week = 0;

	flg_reset(EMP_EARN) ;

	for( ; ; ) {
		retval = get_n_emp_earn(&emp_earn,BROWSE,0,FORWARD,e_mesg);
		if(retval < 0) {
			if(retval == EFL) break;
			return(retval);
		}
		if(strcmp(emp_earn.en_numb,emp_rec.em_numb)!=0) { 
			break;
		}

		if(emp_earn.en_date < pay_param.pr_cal_st_dt || 
		   emp_earn.en_date > pay_param.pr_cal_end_dt)
			break;

		strcpy(emp_ln_his.elh_numb,emp_earn.en_numb);
		emp_ln_his.elh_pp=emp_earn.en_pp;
		strcpy(emp_ln_his.elh_code,"PENBUY");
		emp_ln_his.elh_date = emp_earn.en_date;
		emp_ln_his.elh_seq = 0;
/******************************************
 Found a file in the earning history file did not match the file in the 
   employee loan file,  the payroll for period 13 couldn't have been ran
   on the proper day.
********************************************/
if(emp_ln_his.elh_date == 19940405)
	emp_ln_his.elh_date = 19940330;
/* End Of Patch */

		flg_reset(EMP_LOAN_HIS);
		retval = get_n_emp_lhis(&emp_ln_his,BROWSE,0,FORWARD,e_mesg);
		if(retval < 0 && retval != EFL) {
			return(retval);
		}
		if(strcmp(emp_ln_his.elh_numb,emp_earn.en_numb == 0) &&
		  (emp_ln_his.elh_pp==emp_earn.en_pp ) &&
		 strcmp(emp_ln_his.elh_code,"PENBUY") == 0){
/**** Used for debuging
sprintf(e_mesg,"\nPrinting for emp: %s :pp:%d date: %ld",emp_ln_his.elh_numb,
emp_ln_his.elh_pp,emp_ln_his.elh_date);
write(log_fd, e_mesg, strlen(e_mesg));
****/
			reg_pen += emp_ln_his.elh_amount;
		}

		strcpy(emp_dd_his.edh_numb,emp_earn.en_numb);
		emp_dd_his.edh_pp=emp_earn.en_pp;
		strcpy(emp_dd_his.edh_code,"EX PEN");
		strcpy(emp_dd_his.edh_group,"EX PEN");
		emp_dd_his.edh_date = emp_earn.en_date;

/******************************************
 Found a file in the earning history file did not match the file in the 
   employee loan file,  the payroll for period 13 couldn't have been ran
   on the proper day.
********************************************/
if(emp_dd_his.edh_date == 19940405)
	emp_dd_his.edh_date = 19940330;
/* End Of Patch */

		flg_reset(EMP_DED_HIS);
		retval = get_emp_dhis(&emp_dd_his,BROWSE,0,e_mesg);
		if(retval < 0 && retval != UNDEF) {
			return(retval);
		}
/***************Changed from get next to a direct get 
		if((strcmp(emp_dd_his.edh_numb,emp_earn.en_numb) == 0) &&
		  (emp_dd_his.edh_pp==emp_earn.en_pp ) &&
		 strcmp(emp_dd_his.edh_code,"EX PEN") == 0 &&
		  emp_dd_his.edh_date == emp_earn.en_date){
			reg_pen += emp_dd_his.edh_amount;
		}
*****************/

		if(retval == NOERROR){
			reg_pen += emp_dd_his.edh_amount;
		}

		strcpy(emp_dd_his.edh_numb,emp_earn.en_numb);
		emp_dd_his.edh_pp=emp_earn.en_pp;
		strcpy(emp_dd_his.edh_code,"PENTCH");
		strcpy(emp_dd_his.edh_group,"PENTCH");
		emp_dd_his.edh_date = emp_earn.en_date;

/******************************************
 Found a file in the earning history file did not match the file in the 
   employee loan file,  the payroll for period 13 couldn't have been ran
   on the proper day.
********************************************/
if(emp_dd_his.edh_date == 19940405)
	emp_dd_his.edh_date = 19940330;
/* End Of Patch */

		flg_reset(EMP_DED_HIS);
		retval = get_n_emp_dhis(&emp_dd_his,BROWSE,0,FORWARD,e_mesg);
		if(retval < 0 && retval != EFL) {
			return(retval);
		}
		if((strcmp(emp_dd_his.edh_numb,emp_earn.en_numb) == 0) &&
		  (emp_dd_his.edh_pp==emp_earn.en_pp ) &&
		 strcmp(emp_dd_his.edh_code,"PENTCH") == 0 &&
		  emp_dd_his.edh_date == emp_earn.en_date)
			reg_pen += emp_dd_his.edh_amount;
		
		reg_pen += emp_earn.en_reg1 + emp_earn.en_reg2 + 
			emp_earn.en_reg3 ;

		seq_over(EMP_DED);
	}

	return(NOERROR);
}

/*----------------------------------------------------------------------
Print the totals
-----------------------------------------------------------------------*/

static
PrintTotals()
{
	char	txt_buff[132];

	if( PrintLine()<0 )	return(REPORT_ERR);

	strcpy (txt_buff, "=============");
	mkln(47,txt_buff,13);
	mkln(61,txt_buff,13);
	mkln(75,txt_buff,13);
	mkln(104,txt_buff,13);

	if( PrintLine()<0 )	return(REPORT_ERR);

	tedit((char *)&total_other_tax,"__,___,_0_.__",txt_buff,R_DOUBLE);
	mkln(47,txt_buff,13);
	tedit((char *)&total_union_du,"__,___,_0_.__",txt_buff,R_DOUBLE);
	mkln(61,txt_buff,13);
	tedit((char *)&total_pen_adj,"__,___,_0_.__",txt_buff,R_DOUBLE);
	mkln(75,txt_buff,13);
	tedit((char *)&total_reg_pen,"__,___,_0_.__",txt_buff,R_DOUBLE);
	mkln(104,txt_buff,13);

	if( PrintLine()<0 )	return(REPORT_ERR);

	return(NOERROR);
}
