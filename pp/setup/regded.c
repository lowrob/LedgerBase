/******************************************************************************
		Sourcename   : regded.c
		System       : personal payroll system.
		Module       : Fixed Assets System : Reports
		Created on   : 92-03-09
		Created  By  : david green 
		Cobol Source : 

******************************************************************************
About the file:	
	This file has routines to print Fixed Asset Report. 
	It is called by the file setpp.c .

History:
Programmer      Last change on    Details

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

static Pa_rec	pa_rec;
static Deduction ded_rec;
static Ded_group  grp_rec;
 
/*  Data items for storing the key range end values */
	
static int	PG_SIZE;
static int	retval;
static char 	discfile[15];	/* for storing outputfile name */
static short	pgcnt; 		/* for page count */
static char	resp[2];	/* for storing response */
static short	copies;

extern char 	e_mesg[200];	/* for storing error messages */


regded()
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
			break;
		case FILE_IO:
			resp[0]='F';
			STRCPY( discfile, "regded.dat" );
			if((retval = GetFilename(discfile))<0 )
				return(retval);
			break;
		case PRINTER:
		default:
			resp[0]='P';
			discfile[0]= '\0';
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
	if( *resp=='P' ) SetCopies( (int)copies ); 
	pgcnt = 0;		/* Page count is zero */ 
	LNSZ = 132;		/* line size in no. of chars */ 
	if(*resp=='D')
		PG_SIZE = 20; 
	else
		PG_SIZE = 65; 
	linecnt = PG_SIZE;	/* Page size in no. of lines */ 
	retval = PrintRep(); 

	close_rep();	/* close output file */ 
	close_dbh(); 
	return(retval); 
}	

static
PrintRep() 
{ 
	int	retval;

	retval = PrntHdg();

	ded_rec.dd_code[0]='\0';
	ded_rec.dd_pp_code[0]='\0';
	flg_reset(DEDUCTION);

	for(;;) {
		retval = get_n_deduction(&ded_rec,BROWSE,0,FORWARD,e_mesg);
		if(retval == EFL) break;
		if(retval <0) return(retval); 

		strcpy(grp_rec.dg_code,ded_rec.dd_code);
		strcpy(grp_rec.dg_pp_code,ded_rec.dd_pp_code);
		grp_rec.dg_group[0]='\0';
		flg_reset(DED_GRP);

		for(;;) {
			retval = get_n_ded_grp(&grp_rec,BROWSE,0,FORWARD,e_mesg);
			if(retval == EFL) break;
			if(retval <0)
				return(retval);

			if((strcmp(grp_rec.dg_code,ded_rec.dd_code)!=0)||
			(strcmp(grp_rec.dg_pp_code,ded_rec.dd_pp_code)!=0))
					break;

			retval = PrntRec();
			if(retval == EXIT)
				return(retval);
			if(retval < 0)
				return(retval);
					
		}
		seq_over(DED_GRP);
	}

	seq_over(DEDUCTION);

	if(pgcnt) {
		if(term < 99) 
			last_page();	
	}
	else
		rite_top();

	return(NOERROR);
}

static
PrntHdg()	/* Print heading  */
{
	int	retval;
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
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);

	offset = ( LNSZ-strlen(pa_rec.pa_co_name) )/2;
	mkln( offset, pa_rec.pa_co_name, strlen(pa_rec.pa_co_name) );
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);

#ifdef ENGLISH
	mkln((LNSZ-23)/2,"LIST OF DEDUCTION CODES", 23 );
#else
	mkln((LNSZ-30)/2,"LIST OF REGULAR DEDUCTIONS", 30 ); 
#endif 
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);
	mkln(1,"DEDUCTION",9);
	mkln(22,"DESCRIPTION",11);
	mkln(42,"PAY PER",7);
	mkln(50,"SEC  T4",7);
	mkln(62,"MIN",3);
	mkln(71,"YEARLY MAX",10);
	mkln(84,"DEDUCT MONTHLY PAY PER",22);

	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);
	mkln(3,"CODE",4); 
	mkln(50,"CD FLD CD",9);
	mkln(61,"EARNINGS    CONTR",17);
	mkln(84,"1",1);
	mkln(89,"2",1);
	mkln(93,"3",1);
	mkln(98,"4",1);
	mkln(103,"5",1); 
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);

	return(NOERROR);
}

static
PrntRec()
{
	int	retval;
	char	txt_buff[132];
	double  tempval;

	mkln(1,ded_rec.dd_code,6);
	mkln(10,ded_rec.dd_desc,30);
	mkln(42,ded_rec.dd_pp_code,6);
	mkln(50,ded_rec.dd_second,2);
	mkln(55,ded_rec.dd_t4_fld,1);
	tedit( (char *)&ded_rec.dd_min_earn,"___,_0_.__",txt_buff, 
							R_DOUBLE ); 
	mkln(60,txt_buff,10);
	tedit( (char *)&ded_rec.dd_max_contr,"___,_0_.__",txt_buff, 
							R_DOUBLE ); 
	mkln(72,txt_buff,10);
	mkln(84,ded_rec.dd_ded_pp[0],1);
	mkln(89,ded_rec.dd_ded_pp[1],1);
	mkln(94,ded_rec.dd_ded_pp[2],1);
	mkln(99,ded_rec.dd_ded_pp[3],1);
	mkln(104,ded_rec.dd_ded_pp[4],1);
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);
	mkln(50,"FUND:",5);
	tedit( (char *)&ded_rec.dd_fund,"_0_",txt_buff, R_SHORT ); 
	mkln(56,txt_buff,3);
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);
	mkln(50,"LIABILITY",9);
	mkln(60,"G/L",3);
	mkln(64,"ACCOUNT:",8); 
	mkln(74,ded_rec.dd_lia_acct,18);
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);
	mkln(50,"EXPENSE",7);
	mkln(59,"G/L",3);
	mkln(64,"ACCTS",5);		
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);
	mkln(50,"KEY-01:",7);
	mkln(58,ded_rec.dd_exp_acct[0],5);
	mkln(67,"KEY-02:",7);
	mkln(75,ded_rec.dd_exp_acct[1],5);
	mkln(84,"KEY-03:",7);
	mkln(92,ded_rec.dd_exp_acct[2],5);
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);
	mkln(50,"KEY-04:",7);
	mkln(58,ded_rec.dd_exp_acct[3],5);
	mkln(67,"KEY-05:",7);
	mkln(75,ded_rec.dd_exp_acct[4],5);
	mkln(84,"KEY-06:",7);
	mkln(92,ded_rec.dd_exp_acct[5],5);
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);

	mkln(50,"KEY-07:",7);
	mkln(58,ded_rec.dd_exp_acct[6],5);
	mkln(67,"KEY-08:",7);
	mkln(75,ded_rec.dd_exp_acct[7],5);
	mkln(84,"KEY-09:",7);
	mkln(92,ded_rec.dd_exp_acct[8],5);
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);
	mkln(50,"KEY-10:",7);
	mkln(58,ded_rec.dd_exp_acct[9],5);
	mkln(67,"KEY-11:",7);
	mkln(75,ded_rec.dd_exp_acct[10],5);
	mkln(84,"KEY-12:",7);
	mkln(95,ded_rec.dd_exp_acct[11],5);
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);
	mkln(50,"GROUP:",6);
	mkln(58,grp_rec.dg_group,6);
	mkln(68,"DESC:",5);
	mkln(74,grp_rec.dg_desc,29);
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);
	mkln(50,"AMOUNT (F/P):",13);
	mkln(92,grp_rec.dg_amt_flag,1);
	tedit((char *)&grp_rec.dg_amount,"__,_0_.__",txt_buff,R_DOUBLE);
	mkln(94,txt_buff,9);
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);
	mkln(50,"EMPLOYEE SHARE:",14);
	tempval=100-grp_rec.dg_employer_sh;
	tedit((char *)&tempval,"__,_0_.__",txt_buff,R_DOUBLE);
	mkln(66,txt_buff,9);
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);
	mkln(50,"EMPLOYER SHARE:",14);
	tedit((char *)&grp_rec.dg_employer_sh,"__,_0_.__",txt_buff,
							R_DOUBLE);
	mkln(66,txt_buff,9);	
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);
	

	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);

	return(NOERROR);
}

static
PrintLine()
{
	int	retval;

	retval = prnt_line();
	if(retval < 0) return(REPORT_ERR);
	if(retval == EXIT) return(EXIT);
	if(linecnt > PG_SIZE){
		retval = PrntHdg();
		if(retval < 0) return(REPORT_ERR);
		if(retval == EXIT) return(EXIT);
	}

	return(NOERROR);
}

