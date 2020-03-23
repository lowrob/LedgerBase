/******************************************************************************
		Sourcename   : bargunit.c
		System       : Budgetary Financial system.
		Module       : Fixed Assets System : Reports
		Created on   : 91-11-30
		Created  By  : Andre Cormier 
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
static Barg_unit	barg_unit;

/*  Data items for storing the key range end values */
	
static int	PG_SIZE;
static int	retval;
static char 	discfile[15];	/* for storing outputfile name */
static short	pgcnt; 		/* for page count */
static char	resp[2];	/* for storing response */
static short	copies;

extern char 	e_mesg[80];	/* for storing error messages */


bargunit()
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
			STRCPY( discfile, "bargunit.dat" );
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
	LNSZ = 80;		/* line size in no. of chars */
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
	if(retval < 0) return(-1);
	if(retval == EXIT)
		return(EXIT);
 
	barg_unit.b_code[0] = '\0';
	barg_unit.b_date = 0;

	flg_reset(BARG);

	for(;;) {

		retval = get_n_barg(&barg_unit,BROWSE,0,FORWARD,e_mesg);

		if(retval == EFL) break;

		if((retval = PrntRec())<0 )
			return(retval);
		if(retval == EXIT)	return(retval);
	}	

	seq_over(BARG);

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
	mkln( 51, "Date:", 5 );
#else
	mkln( 51, "Date:", 5 );
#endif
	sysdt = get_date() ;
	tedit( (char *)&sysdt,"____/__/__",line+cur_pos, R_LONG ); 
	cur_pos += 10;

#ifdef ENGLISH
	mkln( 70, "PAGE:", 5 );
#else
	mkln( 70, "PAGE:", 5 );
#endif
	tedit( (char *)&pgcnt,"__0_",  line+cur_pos, R_SHORT ); 
	cur_pos += 4;
	if( PrintLine()<0 )	return(REPORT_ERR);

	offset = ( LNSZ-strlen(pa_rec.pa_co_name) )/2;
	mkln( offset, pa_rec.pa_co_name, strlen(pa_rec.pa_co_name) );
	if( PrintLine()<0 )	return(REPORT_ERR);

#ifdef ENGLISH
	mkln((LNSZ-24)/2,"LIST OF BARGAINING UNITS", 24 );
#else
	mkln((LNSZ-24)/2,"LIST OF BARGAINING UNITS", 24 );
#endif
	if( PrintLine()<0 )	return(REPORT_ERR);
	if( PrintLine()<0 )	return(REPORT_ERR);

	return(NOERROR);
}
static
PrntRec()
{
	char	txt_buff[132];

	mkln(1,"BARGAINING UNIT CODE        :",29);
	mkln(31,barg_unit.b_code,6);
	retval = PrintLine();
	if (retval < 0)	return(REPORT_ERR);
	if (retval == EXIT) return(EXIT);

	mkln(1,"NAME                        :",29);
	mkln(31,barg_unit.b_name,30);
	retval = PrintLine();
	if (retval < 0)	return(REPORT_ERR);
	if (retval == EXIT) return(EXIT);

	mkln(1,"EFFECTIVE DATE              :",29);
	tedit((char*)&barg_unit.b_date,"____/__/__",txt_buff,R_LONG);
	mkln(31,txt_buff,10);
	retval = PrintLine(); 
	if(retval < 0)  	return(REPORT_ERR);
	if(retval == EXIT) 	return(EXIT);

	mkln(1,"CONTRACT END DATE           :",29);
	tedit((char*)&barg_unit.b_contract_dt,"____/__/__",txt_buff,R_LONG);
	mkln(31,txt_buff,10);
	retval = PrintLine();	
	if (retval < 0) 	return(REPORT_ERR);
	if (retval == EXIT)	return(EXIT);

	mkln(1,"STATUTORY HOLIDAY           :",29);
	mkln(31,barg_unit.b_stat_hol,1);
	retval = PrintLine();	
	if(retval < 0) 	return(REPORT_ERR);
	if(retval == EXIT)	return(EXIT);

	mkln(1,"PAY PERIOD CODE             :",29);
	mkln(31,barg_unit.b_pp_code,6);
	retval = PrintLine();
	if (retval < 0)		return(REPORT_ERR);
	if (retval == EXIT)	return(EXIT);

	mkln(1,"FUND NUMBER                 :",29);
	tedit((char*)&barg_unit.b_fund,"_0_",txt_buff,R_SHORT);
	mkln(31,txt_buff,3);
	retval = PrintLine();
	if (retval < 0)		return(REPORT_ERR);
	if (retval == EXIT)	return(EXIT);

	mkln(1,"UIC ACCOUNT NUMBER          :",29);
	mkln(31,barg_unit.b_uic_acct,18);
	retval = PrintLine();
	if(retval < 0) 	return(REPORT_ERR);
	if(retval == EXIT)	return(EXIT);

	mkln(1,"CPP ACCOUNT NUMBER          :",29);
	mkln(31,barg_unit.b_cpp_acct,18);
	retval =  PrintLine();	
	if(retval < 0) 	return(REPORT_ERR);
	if(retval == EXIT) 	return(EXIT);

	mkln(1,"INCOME TAX ACCOUNT NUMBER   :",29);
	mkln(31,barg_unit.b_tax_acct,18);
	retval =  PrintLine();	
	if(retval < 0) 	return(REPORT_ERR);
	if(retval == EXIT) 	return(EXIT);

	mkln(1,"SICK DAY ACCRUAL RATE       :",29);
	tedit((char*)&barg_unit.b_sick_rate,"_,___,_0_.__-",txt_buff,R_DOUBLE);
	mkln(31,txt_buff,13);
	retval =  PrintLine();	
	if(retval < 0) 	return(REPORT_ERR);
	if(retval == EXIT) 	return(EXIT);

	mkln(1,"MAXIMUM NUMBER OF SICK DAYS :",29);
	tedit((char*)&barg_unit.b_sick_max,"_,___,_0_.__-",txt_buff,R_DOUBLE);
	mkln(31,txt_buff,13);
	retval =  PrintLine();	
	if(retval < 0) 	return(REPORT_ERR);
	if(retval == EXIT) 	return(EXIT);

	mkln(1,"DEPARTMENT CODE             :",29);
	mkln(31,barg_unit.b_dept_cd,6);
	retval =  PrintLine();	
	if(retval < 0) 	return(REPORT_ERR);
	if(retval == EXIT) 	return(EXIT);

	mkln(1,"STATUS TOTAL HOURS          :",29);
	tedit((char*)&barg_unit.b_stat_thrs,"_,_0_.__",txt_buff,R_DOUBLE);
	mkln(31,txt_buff,8);
	retval =  PrintLine();	
	if(retval < 0) 	return(REPORT_ERR);
	if(retval == EXIT) 	return(EXIT);

	mkln(1,"STATUS HOURS/DAY            :",29);
	tedit((char*)&barg_unit.b_stat_hpd,"0_.__",txt_buff,R_DOUBLE);
	mkln(31,txt_buff,5);
	retval =  PrintLine();	
	if(retval < 0) 	return(REPORT_ERR);
	if(retval == EXIT) 	return(EXIT);

	retval =  PrintLine();	
	if(retval < 0) 	return(REPORT_ERR);
	if(retval == EXIT) 	return(EXIT);

	return(NOERROR);
}

static
PrintLine()
{
	if(prnt_line() < 0 )	return(REPORT_ERR);

	if(linecnt >= PG_SIZE)
		retval =	PrntHdg();
		if(retval < 0) return(-1);
		if(retval == EXIT)	return(EXIT);

	return(NOERROR);
}

