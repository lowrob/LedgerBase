/******************************************************************************
		Sourcename   : t4prelist.c
		System       : Budgetary Financial system.
		Module       : Fixed Assets System : Reports
		Created on   : 92-05-22
		Created  By  : Andre Cormier 
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
static Emp		emp;
static T4_adj		t4_adj;
static Pay_param	pay_param;

static int	flag;		/* flag to print the employee or not */
static int	PG_SIZE;
static int	retval;
static char 	discfile[15];	/* for storing outputfile name */
static short	pgcnt; 		/* for page count */
static char	resp[2];	/* for storing response */
static short	copies;

static double	total_emp_inc;
static double	total_cpp_cont;
static double	total_uic_prem;
static double	total_cpp_pen_earn;
static double	total_tax_ded;
static double	total_uic_ins_earn;

extern char 	e_mesg[80];	/* for storing error messages */

t4prelist()
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
			STRCPY( discfile, "t4prelist.dat" );
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

	total_emp_inc = 0;
	total_cpp_cont = 0;
	total_uic_prem = 0;
	total_cpp_pen_earn = 0;
	total_tax_ded = 0;
	total_uic_ins_earn = 0;

	retval = PrntHdg();
	if(retval < 0)	return(retval);

	t4_adj.ta_numb[0] = '\0';

	flg_reset(T4_ADJ);

	for(;;) {
		retval = get_n_t4_adj(&t4_adj,BROWSE,0,FORWARD,e_mesg);

		if(retval ==EFL) 
			break;

		strcpy(emp.em_numb,t4_adj.ta_numb);

		retval = get_employee(&emp,BROWSE,0,e_mesg);
		if(retval < 0) {
			fomen(e_mesg);
			get();
			return(ERROR);
		}

		total_emp_inc += t4_adj.ta_emp_inc ;
		total_cpp_cont += t4_adj.ta_cpp_cont ;
		total_uic_prem += t4_adj.ta_uic_prem ;
		total_cpp_pen_earn += t4_adj.ta_cpp_pen_earn ;
		total_tax_ded += t4_adj.ta_tax_ded ;
		total_uic_ins_earn += t4_adj.ta_uic_ins_earn ;

		retval = PrintRec();
		if(retval < 0)	 return(retval);
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
	mkln((LNSZ-10)/2,"T4 PRELIST", 10 );
#else
	mkln((LNSZ-10)/2,"T4 PRELIST", 10 );
#endif
	if( PrintLine()<0 )	return(REPORT_ERR);
	if( PrintLine()<0 )	return(REPORT_ERR);

	mkln(3,"EMPLOYEE",8);
	mkln(51,"TOTAL",5);
	mkln(93,"PENSION",7);
	mkln(106,"INCOME",6);
	mkln(119,"INSURABLE",9);
	
	if( PrintLine()<0 )	return(REPORT_ERR);

	mkln(4,"NUMBER",6);
	mkln(22,"EMPLOYEE NAME",13);
	mkln(50,"EARNINGS",8);
	mkln(67,"CPP",3);
	mkln(81,"UIC",3);
	mkln(95,"PLAN",4);
	mkln(108,"TAX",3);
	mkln(120,"EARNINGS",8);

	if( PrintLine()<0 )	return(REPORT_ERR);
	if( PrintLine()<0 )	return(REPORT_ERR);

	return(NOERROR);
}

/******************************************************************************
Prints the detail line for employee schedule file
******************************************************************************/

static
PrintRec()
{
	char	txt_buff[132];

	mkln(1,t4_adj.ta_numb,12);
	strcpy(txt_buff,emp.em_first_name);
	strcat(txt_buff," ");
	strcat(txt_buff,emp.em_last_name);
	mkln(15,txt_buff,strlen(txt_buff));
	tedit((char *)&t4_adj.ta_emp_inc,"_,___,_0_.__",txt_buff,R_DOUBLE);
	mkln(48,txt_buff,12);
	tedit((char *)&t4_adj.ta_cpp_cont,"_,___,_0_.__",txt_buff,R_DOUBLE);
	mkln(62,txt_buff,12);
	tedit((char *)&t4_adj.ta_uic_prem,"_,___,_0_.__",txt_buff,R_DOUBLE);
	mkln(76,txt_buff,12);
	tedit((char *)&t4_adj.ta_cpp_pen_earn,"_,___,_0_.__",txt_buff,R_DOUBLE);
	mkln(90,txt_buff,12);
	tedit((char *)&t4_adj.ta_tax_ded,"__,___,_0_.__",txt_buff,R_DOUBLE);
	mkln(104,txt_buff,13);
	tedit((char *)&t4_adj.ta_uic_ins_earn,"_,___,_0_.__",txt_buff,R_DOUBLE);
	mkln(118,txt_buff,12);

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

/******************************************************************************
Print out totals accumulated for all numeric colums 
******************************************************************************/

PrintTotals()
{
	char	txt_buff[132];

	if( PrintLine()<0 )	return(REPORT_ERR);

	strcpy(txt_buff, "=============");

	mkln( 47, txt_buff, 13);
	mkln( 61, txt_buff, 13);
	mkln( 75, txt_buff, 13);
	mkln( 89, txt_buff, 13);
	mkln( 103, txt_buff, 13);
	mkln( 117, txt_buff, 13);
	if( PrintLine()<0 )	return(REPORT_ERR);

	tedit((char *)&total_emp_inc,"__,___,_0_.__",txt_buff,R_DOUBLE);
	mkln(47,txt_buff,13);
	tedit((char *)&total_cpp_cont,"__,___,_0_.__",txt_buff,R_DOUBLE);
	mkln(61,txt_buff,13);
	tedit((char *)&total_uic_prem,"__,___,_0_.__",txt_buff,R_DOUBLE);
	mkln(75,txt_buff,13);
	tedit((char *)&total_cpp_pen_earn,"__,___,_0_.__",txt_buff,R_DOUBLE);
	mkln(89,txt_buff,13);
	tedit((char *)&total_tax_ded,"__,___,_0_.__",txt_buff,R_DOUBLE);
	mkln(103,txt_buff,13);
	tedit((char *)&total_uic_ins_earn,"__,___,_0_.__",txt_buff,R_DOUBLE);
	mkln(117,txt_buff,13);

	if( PrintLine()<0 )	return(REPORT_ERR);

	return;
}
