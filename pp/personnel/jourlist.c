/* ---------------------------------------------------------------------------
*	Source 	: jourlist.c 
*
*	Program to Print Employee Pay Period Benefit using REPORT GENERATOR.
*

Modifications:

	YY/MM/DD	Programmer	Description
	~~~~~~~~	~~~~~~~~~~	~~~~~~~~~~~
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_pp.h>
#include <bfs_com.h>
#include <repdef.h>

#define	PROJNAME	"jourlist"
#define EXIT		12
#define CONTINUE	10

#ifdef ENGLISH
#define PRINTER 'P'
#define DISPLAY 'D'
#define FILE_IO	'F'
#define SPOOL	'S'
#define YES	'Y'
#define NO	'N'
#else
#define PRINTER 'I'
#define DISPLAY 'A'
#define FILE_IO	'D'
#define SPOOL	'B'
#define YES	'O'
#define NO	'N'
#endif

char	e_mesg[80] ;

static	Jr_ent		jr_ent;
static	Gl_rec		gl_rec;
static	Pa_rec		pa_rec;
static	Emp		emp_rec;

int 	outcntl ;
static	int	retval;
static	short	copies ;
static int	PG_SIZE;
static char 	discfile[15];	/* for storing outputfile name */
static short	pgcnt; 		/* for page count */
static char	resp[2];	/* for storing response */
static	short	err;

static	double	dbtot = 0;
static	double	crtot = 0;
static	double	empdbtot = 0;
static	double	empcrtot = 0;
static	int	print_full = 0;
static	int	first_time = 0;
static	char	prev_numb[13];

jourlist()
{

	strcpy(resp, "P");

	outcntl = 2;
	PG_SIZE = 60;

	discfile[0] = '\0';

	copies = 1;
	SetCopies( (int)copies );   /* number of copies to print */

	pgcnt = 0;		/* Page count is zero */
	LNSZ = 132;		/* line size in no. of chars */
	linecnt = PG_SIZE;	/* Page size in no. of lines */

	if( opn_prnt( resp, discfile, 1, e_mesg, 1 /* spool */)<0 ){
		return(REPORT_ERR);
	}
	retval = PrntRep();
	if(retval == EXIT)	return(retval);

	close_rep();
	close_dbh();
	return(retval);
}
/*---------------------------------------------------------------------*/
static
PrntRep()
{
	int	code;
	
	code = get_param(&pa_rec, BROWSE, 1, e_mesg);
 	if (code < 1) {
		printf("%s\n", e_mesg);
		return(-1);
	}

	retval = PrntHdg();
	if(retval == EXIT)	return(NOERROR);

	/* Initialize Recs. to start printing benefit report on benefit no. */
	jr_ent.jr_emp_numb[0] = '\0';
	jr_ent.jr_fund = 0;
	jr_ent.jr_acct[0] = '\0';

	flg_reset(JR_ENT);

	for( ; ; ) {
		code = get_n_jr_ent(&jr_ent,BROWSE,4,FORWARD,e_mesg);

		if( code < 0) {
			if(code == EFL)  break ;
			code = DBH_ERR;
			break ;
		}

		strcpy(emp_rec.em_numb,jr_ent.jr_emp_numb);
		retval = get_employee(&emp_rec,BROWSE,0,e_mesg);
		if(retval < 0){
			fomer(e_mesg);
			continue;
		}	
		gl_rec.funds = jr_ent.jr_fund;
		strcpy(gl_rec.accno,jr_ent.jr_acct);
		gl_rec.reccod = 99;

		code = get_gl(&gl_rec,BROWSE,0,e_mesg);
		if( code < 0) {
			fomen(e_mesg);
			get();
/*
			if(code == UNDEF) break;
			code = DBH_ERR;
			break;
*/
		}


		if(jr_ent.jr_amount > 0){
			gl_rec.nextdb = jr_ent.jr_amount;
		}
		else {
			gl_rec.nextcr = (jr_ent.jr_amount * -1);
		}

		print_full = 1;

		if(first_time == 0){
			strcpy(prev_numb,emp_rec.em_numb);	 
			print_full = 0;
			first_time = 1;
		}
		if(strcmp(prev_numb,emp_rec.em_numb)!=0){
			retval = PrntEmp();
			if(retval == EXIT)	return(retval);
			print_full = 0;
			empdbtot = 0;
			empcrtot = 0;
		}

		empdbtot += gl_rec.nextdb;
		empcrtot += gl_rec.nextcr;

		dbtot += gl_rec.nextdb;
		crtot += gl_rec.nextcr;

		retval = PrntDetail();
		if(retval == EXIT)	return(NOERROR);

		strcpy(prev_numb,emp_rec.em_numb);
	}
	close_file(JR_ENT) ;
	close_file(GLMAST) ;

	retval = PrntEmp();
	if(retval == EXIT)	return(retval);

	retval = PrntGrnd();
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

/******************************************************************************
Prints the headings of the report GROSS EARNINGS BY EARNINGS CODE 
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
	mkln((LNSZ-15)/2,"JOURNAL LISTING", 15 );
#else
	mkln((LNSZ-15)/2,"GROSS EARNINGS FOR PERIOD", 25 );
#endif
	if( PrintLine()<0 )	return(REPORT_ERR);
	if( PrintLine()<0 )	return(REPORT_ERR);

	mkln(3,"EMPLOYEE #",10);
	mkln(25,"NAME",4);
	mkln(48,"FUND",4);
	mkln(60,"ACCOUNT",7);
	mkln(74,"CODE",4);
	mkln(79,"TYPE",4);
	mkln(89,"DESCRIPTION",11);
	mkln(110,"DEBIT",5);
	mkln(122,"CREDIT",6);
	if( PrintLine()<0 )	return(REPORT_ERR);
	if( PrintLine()<0 )	return(REPORT_ERR);

	return(NOERROR);
}
/*----------------------------------------------------------------------*/
static
PrntDetail()
{
	char	txt_line[132], txt_buff[132];

	if(print_full == 0){
		mkln(2,emp_rec.em_numb,12);
		sprintf(txt_line,"%s, %s %s",
			emp_rec.em_last_name,
			emp_rec.em_first_name);
		mkln(17,txt_line,30);
	}
	tedit((char *)&jr_ent.jr_fund,"_0_",txt_buff,R_SHORT);
	mkln(49,txt_buff,3);
	mkln(53,jr_ent.jr_acct,18);
	mkln(73,jr_ent.jr_code,6);
	mkln(81,jr_ent.jr_type,20);
	mkln(83,gl_rec.desc,19);
	tedit((char *)&gl_rec.nextdb,"_,___,_0_.__-",txt_buff,R_DOUBLE);
	mkln(103,txt_buff,13);
	tedit((char *)&gl_rec.nextcr,"_,___,_0_.__-",txt_buff,R_DOUBLE);
	mkln(117,txt_buff,13);

	if( PrintLine()<0 )	return(REPORT_ERR);
	if(retval == EXIT)	return(retval);

	return(NOERROR);

}
/**************************************************************************/
static
PrntGrnd()
{
	char	txt_line[132], txt_buff[132];

	mkln(2,"GRAND TOTAL:",15);
	tedit((char *)&dbtot,"_,___,_0_.__-",txt_buff,R_DOUBLE);
	mkln(103,txt_buff,13);
	tedit((char *)&crtot,"_,___,_0_.__-",txt_buff,R_DOUBLE);
	mkln(117,txt_buff,13);

	if( PrintLine()<0 )	return(REPORT_ERR);
	if(retval == EXIT)	return(retval);

	return(NOERROR);

}
/***************************************************************************/
static
PrntEmp()
{
	char	txt_line[132], txt_buff[132];

	if( PrintLine()<0 )	return(REPORT_ERR);
	if(retval == EXIT)	return(retval);

	mkln(2,"EMPLOYEE TOTAL:",15);
	tedit((char *)&empdbtot,"_,___,_0_.__-",txt_buff,R_DOUBLE);
	mkln(103,txt_buff,13);
	tedit((char *)&empcrtot,"_,___,_0_.__-",txt_buff,R_DOUBLE);
	mkln(117,txt_buff,13);

	if( PrintLine()<0 )	return(REPORT_ERR);
	if(retval == EXIT)	return(retval);

	if( PrintLine()<0 )	return(REPORT_ERR);
	if(retval == EXIT)	return(retval);

	return(NOERROR);

}
/***************************************************************************
Function that prints every line of the report 
******************************************************************************/
static
PrintLine()
{
	if(prnt_line() < 0 )	return(REPORT_ERR);

	if(linecnt > PG_SIZE) {
		retval = PrntHdg();
		if(retval == EXIT)	return(retval);
	}

	return(NOERROR);
}
/*------------------------------------------------------------------------*/
