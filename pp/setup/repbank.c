/******************************************************************************
		Sourcename   : repbank.c
		System       : Budgetary Financial system.
		Module       : Fixed Assets System : Reports
		Created on   : 91-11-25
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
static Bank	bank;

/*  Data items for storing the key range end values */
	
static int	PG_SIZE;
static int	retval;
static char 	discfile[15];	/* for storing outputfile name */
static short	pgcnt; 		/* for page count */
static char	resp[2];	/* for storing response */
static short	copies;

extern char 	e_mesg[80];	/* for storing error messages */


repbank()
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
			STRCPY( discfile, "repbank.dat" );
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
	LNSZ = 132;		/* line size in no. of chars */
	linecnt = PG_SIZE;	/* Page size in no. of lines */

	retval = PrintRep();
	close_rep();	/* close output file */
	close_dbh();
	return(retval);
}

static
PrintRep()
{
	int first_time;
	first_time = 0;
	bank.bk_numb[0] = '\0';

	flg_reset(BANK);

	for(;;) {

		retval = get_n_bank(&bank,BROWSE,0,FORWARD,e_mesg);

		if(retval == EFL) break;

 		if (first_time == 0) {
			retval = PrntHdg();
			if(retval < 0) return(-1);
			if(retval == EXIT)	return(EXIT);
			first_time = 1;
		}
		if((retval = PrntRec())<0 )
			return(retval);

		if (retval = EXIT)	return(retval);
	}	

	seq_over(BANK);

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
	mkln((LNSZ-22)/2,"LIST OF BANK LOCATIONS", 22 );
#else
	mkln((LNSZ-22)/2,"LIST OF BANK LOCATIONS", 22 );
#endif
	if( PrintLine()<0 )	return(REPORT_ERR);
	if( PrintLine()<0 )	return(REPORT_ERR);

	mkln(5,"BANK",4);
	mkln(20,"BANK NAME",9);
	mkln(54,"ADDRESS",7);
	mkln(77,"POSTAL/ZIP",10);
	mkln(94,"CONTACT",7);
	mkln(120,"TELEPHONE #",11);

	if( PrintLine()<0 )	return(REPORT_ERR);

	mkln(4,"NUMBER",6);
	mkln(79,"CODE",4);

	if( PrintLine()<0 )	return(REPORT_ERR);
	if( PrintLine()<0 )	return(REPORT_ERR);

	return(NOERROR);
}
static
PrntRec()
{
	char	txt_buff[132];

	mkln(1,bank.bk_numb,12);
	mkln(15,bank.bk_name,30);
	mkln(45,bank.bk_add1,30);
	mkln(77,bank.bk_pc,8);
	mkln(88,bank.bk_cont,30);
	mkln(119,"(",1);
	mkln(120,bank.bk_phone,3);
	mkln(123,")",1);
	mkln(124,bank.bk_phone+3,3);
	mkln(127,"-",1);
	mkln(128,bank.bk_phone+6,4);

	retval = PrintLine();
	if(retval < 0) return(REPORT_ERR);
	if(retval == EXIT) return(EXIT);

	mkln(45,bank.bk_add2,30); 
	
	retval = PrintLine();	
	if(retval < 0) return(REPORT_ERR);
	if(retval == EXIT) return(EXIT);

	mkln(45,bank.bk_add3,30);

	retval =  PrintLine();
	if(retval < 0)	return(REPORT_ERR);
	if(retval == EXIT) return(EXIT);

	mkln(45,bank.bk_add4,30);

	retval = PrintLine();
	if(retval < 0) return(REPORT_ERR);
	if(retval == EXIT) return(EXIT);
	retval = PrintLine();
	if(retval < 0) return(REPORT_ERR);
	if(retval == EXIT) return(EXIT);

	return(NOERROR);
}

static
PrintLine()
{
	if(prnt_line() < 0 )	return(REPORT_ERR);

	if(linecnt > PG_SIZE)
		retval = PrntHdg();
		if(retval < 0) return(-1);
		if(retval == EXIT) return(EXIT);

	return(NOERROR);
}

