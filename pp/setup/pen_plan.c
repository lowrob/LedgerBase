/*****************************************************************************
		Sourcename   : pen_plan.c
		System       : Budgetary Financial system.
		Module       : Fixed Assets System : Reports
		Created on   : 92-03-16
		Created  By  : Dave Frier 
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
static Reg_pen  reg_pen; 

/*  Data items for storing the key range end values */
	
static int	PG_SIZE;
static int	retval;
static char 	discfile[15];	/* for storing outputfile name */
static short	pgcnt; 		/* for page count */
static char	resp[2];	/* for storing response */
static short	copies;

extern char 	e_mesg[80];	/* for storing error messages */


pen_plan()
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
			STRCPY( discfile, "pen_plan.dat" );
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
	if( *resp=='P' )
		SetCopies( (int)copies );
	pgcnt = 0;		/* Page count is zero */
	LNSZ = 132;		/* line size in no. of chars */
	if(*resp == 'D')
		PG_SIZE = 20;
	else
		PG_SIZE = 60;
	linecnt = PG_SIZE;	/* Page size in no. of lines */

	retval = PrintRep();
	close_rep();	/* close output file */
	close_dbh();
	return(retval);
}

static
PrintRep()
{
	retval = PrntHdg();
	if(retval == EXIT)	return(EXIT);
	if(retval < 0)	return(retval);

	reg_pen.rg_code[0] ='\0';
	reg_pen.rg_pp_code[0] ='\0';
	
	flg_reset(REG_PEN);

	for(;;) {

		retval = get_n_reg_pen(&reg_pen,BROWSE,0,FORWARD,e_mesg);

		if(retval == EFL) break;

		retval = PrntRec(); 
		if(retval == EXIT) return(EXIT);
		if(retval < 0)	return(retval);

	}	

	seq_over(REG_PEN);

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
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);

	offset = ( LNSZ-strlen(pa_rec.pa_co_name) )/2;
	mkln( offset, pa_rec.pa_co_name, strlen(pa_rec.pa_co_name) );
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);

#ifdef ENGLISH
        mkln((LNSZ-32)/2,"LIST OF REGISTERED PENSION PLANS",32);	
#else
	mkln((LNSZ-32)/2,"LIST OF REGISTERED PENSION PLANS", 32);
#endif
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);

	mkln(1,"PENSION",7);
	mkln(19,"DESCRIPTION",11);
	mkln(42,"PAY",3);
	mkln(53,"REG",3);
	mkln(61,"TYPE",4);
	mkln(66,"PEN ADJ",7);
	mkln(75,"DEDUCT MON PAY",14);
 	mkln(90,"% RATE1",7);
	mkln(98,"% RATE3",7);
        mkln(106,"AMOUNT1",7);	
	mkln(114,"EMPLOYEE SHARE",14);
	
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);

	mkln(3,"CODE",4);
	mkln(42,"PER",3);
	mkln(54,"#",1);
	mkln(67,"CALC# 1  2  3  4  5",19);
	mkln(90,"% RATE2",7);
        mkln(106,"AMOUNT2",7);
	mkln(114,"EMPLOYER SHARE",14);
	
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
	char	txt_buff[132];
	double	tmp_val;

        mkln(2,reg_pen.rg_code,6);
        mkln(9,reg_pen.rg_desc,30); 
      	mkln(41,reg_pen.rg_pp_code,6);  
	mkln(48,reg_pen.rg_reg_num,12);
	mkln(61,reg_pen.rg_type,2);
	tedit( (char *)&reg_pen.rg_pac,"_0_",txt_buff, R_SHORT ); 
	mkln(67,txt_buff,3);
   	mkln(73,reg_pen.rg_ded_pp[0]);
   	mkln(76,reg_pen.rg_ded_pp[1]);
   	mkln(79,reg_pen.rg_ded_pp[2]);
   	mkln(82,reg_pen.rg_ded_pp[3]);
   	mkln(85,reg_pen.rg_ded_pp[4]);
	tedit( (char *)&reg_pen.rg_perc1,"_0_.__",txt_buff, R_DOUBLE ); 
        mkln(91,txt_buff,6); 
	tedit( (char *)&reg_pen.rg_perc3,"_0_.__",txt_buff, R_DOUBLE ); 
        mkln(99,txt_buff,6);
	tedit( (char *)&reg_pen.rg_amount,"__,_0_.__",txt_buff, R_DOUBLE ); 
	mkln(107,txt_buff,9);
	tmp_val = 100 - reg_pen.rg_employer_sh;
	tedit( (char *)&tmp_val,"_0_.__",txt_buff, R_DOUBLE ); 
	mkln(118,txt_buff,6);

	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);
	
	tedit( (char *)&reg_pen.rg_perc2,"_0_.__",txt_buff, R_DOUBLE ); 
	mkln(91,txt_buff,6);
	tedit( (char *)&reg_pen.rg_amount2,"__,_0_.__",txt_buff, R_DOUBLE ); 
	mkln(107,txt_buff,9);
	tedit( (char *)&reg_pen.rg_employer_sh,"_0_.__",txt_buff, R_DOUBLE ); 
 	mkln(118,txt_buff,6);
 
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);

	mkln(48,"MINIMUM EARNINGS:",17);
	tedit( (char *)&reg_pen.rg_min_earn,"__,_0_.__",txt_buff, R_DOUBLE ); 
	mkln(66,txt_buff,9);
	
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);

	mkln(48,"YEARLY MAX CONTR:",17);
	tedit( (char *)&reg_pen.rg_max_contr,"___,_0_.__",txt_buff, R_DOUBLE ); 
	mkln(66,txt_buff,11);
 
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);
	
	mkln(48,"FUND#:",6);
	tedit( (char *)&reg_pen.rg_fund,"_0_",txt_buff, R_SHORT ); 
	mkln(55,txt_buff,1);

	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);
	
	mkln(48,"LIABILITY G/L ACCT:",19);
	mkln(68,reg_pen.rg_lia_acct,18);
        	
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);
	
	mkln(48,"EXPENSE G/L ACCT:",17);
  	
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);

	mkln(49,"KEY 01:",7);
	sprintf(txt_buff,"%d",reg_pen.rg_exp_acct[0]);
	mkln(57,txt_buff,5);
	mkln(66,"KEY 02:",7);
	sprintf(txt_buff,"%d",reg_pen.rg_exp_acct[1]);
	mkln(74,txt_buff,5);
	mkln(83,"KEY 03:",7);
	sprintf(txt_buff,"%d",reg_pen.rg_exp_acct[2]);
	mkln(91,txt_buff,5);

	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);

	mkln(49,"KEY 04:",7);
	sprintf(txt_buff,"%d",reg_pen.rg_exp_acct[3]);
	mkln(57,txt_buff,5);
	mkln(66,"KEY 05:",7);
	sprintf(txt_buff,"%d",reg_pen.rg_exp_acct[4]);
	mkln(74,txt_buff,5);
	mkln(83,"KEY 06:",7);
	sprintf(txt_buff,"%d",reg_pen.rg_exp_acct[5]);
	mkln(91,txt_buff,5);

	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);

	mkln(49,"KEY 07:",7);
	sprintf(txt_buff,"%d",reg_pen.rg_exp_acct[6]);
	mkln(57,txt_buff,5);
	mkln(66,"KEY 08:",7);
	sprintf(txt_buff,"%d",reg_pen.rg_exp_acct[7]);
	mkln(74,txt_buff,5);
	mkln(83,"KEY 09:",7);
	sprintf(txt_buff,"%d",reg_pen.rg_exp_acct[8]);
	mkln(91,txt_buff,5);
	
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);

	mkln(49,"KEY 10:",7);
	sprintf(txt_buff,"%d",reg_pen.rg_exp_acct[9]);
	mkln(57,txt_buff,5);
	mkln(66,"KEY 11:",7);
	sprintf(txt_buff,"%d",reg_pen.rg_exp_acct[10]);
	mkln(74,txt_buff,5);
	mkln(83,"KEY 12:",7);
	sprintf(txt_buff,"%d",reg_pen.rg_exp_acct[11]);
	mkln(91,txt_buff,5);
	
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);
	retval = PrintLine();
	if(retval == EXIT) return(EXIT);
	if(retval < 0) return(REPORT_ERR);
}
static
PrintLine()
{
	retval = prnt_line();	
	if(retval < 0) return(REPORT_ERR);
	if(retval == EXIT)	return(EXIT);

	if(linecnt > PG_SIZE){
		retval = PrntHdg();
		if(retval == EXIT)	return(EXIT);
		if(retval < 0)	return(REPORT_ERR);
	}

	return(NOERROR);
}

