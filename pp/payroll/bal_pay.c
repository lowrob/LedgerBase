/* ---------------------------------------------------------------------------
*	Source 	: pay_bal.c 
*
* 	Program to pick up differances between the pay register and the journal
listing.  Sometimes when employees are not set up for a cost center in the G/L
the register amount will not match the journal amount.  This report can be ran
after the selectiona and calculation is done.
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

#define	PROJNAME	"bal_pay"
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

extern char	e_mesg[80] ;
#define		INTERACTIVE	0

static	Jr_ent		jr_ent;
static	Gl_rec		gl_rec;
static	Pa_rec		pa_rec;
static	Emp		emp_rec;
static	Pay_earn	pay_earn;
static	Man_chq		man_chq; /* Pay advance file */

int 	outcntl ;
static	int	retval;
static	short	copies ;
static int	PG_SIZE;
static char 	discfile[15];	/* for storing outputfile name */
static short	pgcnt; 		/* for page count */
static char	resp[2];	/* for storing response */
static	short	err;

static	double	man_amount;
static	double	journalamount;

bal_pay()
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

	if( *resp=='P' ){
		copies = 1;
		if((retval = GetNbrCopies(&copies))<0 )
			return(retval);
	}

	if((retval = Confirm()) <= 0)
		return(retval);


	pgcnt = 0;		/* Page count is zero */
	LNSZ = 132;		/* line size in no. of chars */
	linecnt = PG_SIZE;	/* Page size in no. of lines */

	if( opn_prnt( resp, discfile, 1, e_mesg, 1 /* spool */)<0 ){
		return(REPORT_ERR);
	}
	retval = MainLogic();
	if(retval == EXIT)	return(retval);

	close_rep();
	close_dbh();
	return(retval);
}
/*---------------------------------------------------------------------*/
static
MainLogic()
{
	int	retval;
	double	tmp_value;
	
	retval = get_param(&pa_rec, BROWSE, 1, e_mesg);
 	if (retval < 1) {
		printf("%s\n", e_mesg);
		return(-1);
	}

	retval = PrntHdg();
	if(retval == EXIT)	return(NOERROR);

	/* read the pay earnings */

	pay_earn.pe_numb[0] = NULL;
	pay_earn.pe_pp = 0;
	pay_earn.pe_date = 0;
	flg_reset(PP_EARN);
	for(;;){
		retval=get_n_pp_earn(&pay_earn,BROWSE,0,FORWARD,e_mesg);
		if(retval < 0 && retval != EFL) {
			fomen(e_mesg);
			get();
			return(retval);
		}
		if(retval ==  EFL)
			break;

		if(retval = GetJournalAmount())
			return(retval);

	/* The net amount in the pay earn file does not include advances*/
		if(retval = ProcessAdv())
			return(retval);

		/*****
		The two amounts are comparing more that 2 decimial places
		Had to add the net amount to the journal amount and see if 
		the value is 0.
		*****/
		tmp_value = journalamount * -1 + pay_earn.pe_net + man_amount;
		if(tmp_value < -0.001 || tmp_value > 0.001)
			if(retval = PrntDetail())
				return(retval);
	}
	close_file(PP_EARN) ;
	return(NOERROR);
}
static
GetJournalAmount()
{
	int	retval;

	journalamount = 0;

	/* Initialize Recs. to start printing benefit report on benefit no. */
	strcpy(jr_ent.jr_emp_numb, pay_earn.pe_numb);;
	jr_ent.jr_fund = 0;
	jr_ent.jr_acct[0] = '\0';

	flg_reset(JR_ENT);

	for( ; ; ) {
		retval = get_n_jr_ent(&jr_ent,BROWSE,4,FORWARD,e_mesg);

		if( retval < 0) {
			if(retval == EFL)  break ;
			fomer(e_mesg);
			return(retval);
		}

		if(strcmp(jr_ent.jr_emp_numb, pay_earn.pe_numb))
			break;

		journalamount += jr_ent.jr_amount;

	}
	close_file(JR_ENT) ;
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

/* Process manual cheque associated with employee cheque */
static
ProcessAdv()
{
	int	retval;

	man_amount = 0;
	strcpy(man_chq.mc_emp_numb,pay_earn.pe_numb);
	man_chq.mc_date = pay_earn.pe_date;
	flg_reset(MAN_CHQ);

	/* get all the advances and add together */
	for(;;){
		retval = get_n_man_chq(&man_chq,BROWSE,1,FORWARD,e_mesg) ;
		if( retval == EFL)
			break;
		if( retval < 0 ) {
			fomen(e_mesg);
			get();
			return(retval);
			}
		if(strcmp(man_chq.mc_emp_numb,pay_earn.pe_numb)==0 &&
		  man_chq.mc_date == pay_earn.pe_date)
			man_amount += man_chq.mc_amount;
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
			fomen(e_mesg);
			get();
			return(retval);
		}
		if(strcmp(man_chq.mc_emp_numb,pay_earn.pe_numb)==0 &&
		  man_chq.mc_chq_numb == pay_earn.pe_date)
			man_amount += (man_chq.mc_amount * -1);
		else
			if(man_chq.mc_chq_numb > pay_earn.pe_date)
				break;
	}
	close_file(MAN_CHQ);
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

	mkln((LNSZ-32)/2,"BALANCE JOURNAL/REGISTER LISTING", 32 );

	if( PrintLine()<0 )	return(REPORT_ERR);
	if( PrintLine()<0 )	return(REPORT_ERR);

	mkln(3,"EMPLOYEE #",10);
	mkln(25,"NAME",4);
	mkln(55,"NET PAY",7);
	mkln(67,"JOURNAL AMT",11);
	mkln(85,"DIFFERANCE",10);
	if( PrintLine()<0 )	return(REPORT_ERR);
	if( PrintLine()<0 )	return(REPORT_ERR);

	return(NOERROR);
}
/*----------------------------------------------------------------------*/
static
PrntDetail()
{
	char	txt_line[132];
	double	tmp_value;

	strcpy(emp_rec.em_numb,pay_earn.pe_numb);
	retval = get_employee(&emp_rec,BROWSE,0,e_mesg);
	if(retval < 0){
		fomer(e_mesg);
		return(ERROR);
	}	

	mkln(2,emp_rec.em_numb,12);
	sprintf(txt_line,"%s %s", emp_rec.em_last_name,
		emp_rec.em_first_name);
	mkln(17,txt_line,30);

	/* Print net pay */
	tmp_value = pay_earn.pe_net + man_amount;
	tedit((char *)&tmp_value, "_,___,_0_.__-",txt_line,R_DOUBLE);
	mkln(50,txt_line,13);

	/* Print journal amount */
	tedit((char *)&journalamount,"_,___,_0_.__-",txt_line,R_DOUBLE);
	mkln(65,txt_line,13);

	/* Print the differance */
	tmp_value -= journalamount;
	tedit((char *)&tmp_value,"_,___,_0_.__-",txt_line,R_DOUBLE);
	mkln(80,txt_line,13);

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
