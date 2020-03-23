/*****************************************************************************
		Sourcename   : Benlist.c 
		System       : Budgetary Financial system.
		Module       : Fixed Assets System : Reports
		Created on   : 6th April 1992
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
static Benefit	benefit;

/*  Data items for storing the key range end values */
	
static int	PG_SIZE;
static int	retval;
static char 	discfile[15];	/* for storing outputfile name */
static short	pgcnt; 		/* for page count */
static char	resp[2];	/* for storing response */
static short	copies;

extern char 	e_mesg[80];	/* for storing error messages */


benlist()
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
			STRCPY( discfile, "benlist.dat" );
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
	PG_SIZE = 66;
	linecnt = PG_SIZE;	/* Page size in no. of lines */

	retval = PrintRep();
	close_rep();	/* close output file */
	close_dbh();
	return(retval);
}

static
PrintRep()
{
	PrntHdg();

	
	benefit.bn_code[0] = '\0';
	benefit.bn_pp_code[0] = '\0';

	flg_reset(BENEFIT);

	for(;;) {

		retval = get_n_benefit(&benefit,BROWSE,0,FORWARD,e_mesg);

		if(retval == EFL) break;

		if((retval = PrntRec())<0 )
			return(retval);

	}	

	seq_over(BENEFIT);

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
	if( PrintLine()<0 )	return(REPORT_ERR);

	offset = ( LNSZ-strlen(pa_rec.pa_co_name) )/2;
	mkln( offset, pa_rec.pa_co_name, strlen(pa_rec.pa_co_name) );
	if( PrintLine()<0 )	return(REPORT_ERR);

#ifdef ENGLISH
        mkln((LNSZ-16)/2,"LIST OF BENEFITS",16);
#else
	mkln((LNSZ-16)/2,"LIST OF BENEFITS",16);
#endif
	if( PrintLine()<0 )	return(REPORT_ERR);
	if( PrintLine()<0 )	return(REPORT_ERR);

	mkln(1,"BENEFIT",7);
	mkln(21,"DESCRIPTION",11);
	mkln(42,"PAY PER",7);
	mkln(50,"EMP",3);
	mkln(55,"T4",2);
	mkln(61,"ADD TO MONTHLY",14);
	mkln(76,"PAY PER",7);
	mkln(84,"(F/P)",5);
	mkln(92,"AMOUNT",6);
	mkln(104,"FUND",4);
	mkln(114,"G/L ACCOUNT",11);
	
	if( PrintLine()<0 )	return(REPORT_ERR);

	mkln(2,"CODE",4);
	mkln(43,"CODE",4);
	mkln(50,"INC",3);
	mkln(54,"FLD",3);
	mkln(58,"CD 1    2    3    4    5",24);

	if( PrintLine()<0 )	return(REPORT_ERR);
	if( PrintLine()<0 )	return(REPORT_ERR);

	return(NOERROR);
}
static
PrntRec()
{
	char	txt_buff[132];

        mkln(1,benefit.bn_code,6);
        mkln(11,benefit.bn_desc,30); 
      	mkln(42,benefit.bn_pp_code,6);  
	mkln(51,benefit.bn_inc,1);
	mkln(55,benefit.bn_t4_fld,1);
   	mkln(61,benefit.bn_add_pp[0]);
   	mkln(66,benefit.bn_add_pp[1]);
   	mkln(71,benefit.bn_add_pp[2]);
   	mkln(76,benefit.bn_add_pp[3]);
   	mkln(81,benefit.bn_add_pp[4]);
  	mkln(86,benefit.bn_amt_flag,1);
	tedit((char *)&benefit.bn_amount,"__0__.__",txt_buff, R_DOUBLE); 
	mkln(91,txt_buff,8);
	sprintf(txt_buff,"%d",benefit.bn_fund);
	mkln(105,txt_buff,1);
	mkln(111,benefit.bn_acct,18);

	if(PrintLine()<0 )		return(REPORT_ERR);
	if(PrintLine()<0 )		return(REPORT_ERR);
	
}
static
PrintLine()
{
	if(prnt_line() < 0 )	return(REPORT_ERR);

	if(linecnt > PG_SIZE)
		PrntHdg();

	return(NOERROR);
}

