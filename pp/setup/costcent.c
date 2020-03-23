/******************************************************************************
		Sourcename   : costcent.c
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
static Sch_rec	sch_rec;

/*  Data items for storing the key range end values */
	
static int	PG_SIZE;
static int	retval;
static char 	discfile[15];	/* for storing outputfile name */
static short	pgcnt; 		/* for page count */
static char	resp[2];	/* for storing response */
static short	copies;

extern char 	e_mesg[80];	/* for storing error messages */


costcent()
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
			PG_SIZE = 16;
			break;
		case FILE_IO:
			resp[0]='F';
			STRCPY( discfile, "costcent.dat" );
			PG_SIZE = 63;
			if((retval = GetFilename(discfile))<0 )
				return(retval);
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
	return(NOERROR);
}

static
PrintRep()
{
	int first_time = 0;
	sch_rec.sc_numb = 0;

	flg_reset(SCHOOL);

	for(;;) {

		retval = get_n_sch(&sch_rec,BROWSE,0,FORWARD,e_mesg);

		if(retval == EFL) break;

		if(first_time == 0) {
			PrntHdg();
			first_time = 1;	
		}
		
		if((retval = PrntRec())<0 )
			return(retval);
		if(retval == EXIT)	return(retval);

	}	

	seq_over(SCHOOL);

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
	mkln((LNSZ-20)/2,"LIST OF COST CENTERS", 20 );
#else
	mkln((LNSZ-20)/2,"LIST OF COST CENTERS", 20 );
#endif
	if( PrintLine()<0 )	return(REPORT_ERR);
	if( PrintLine()<0 )	return(REPORT_ERR);

	mkln(1,"COST CENTER",11);
	mkln(24,"NAME",4);
	mkln(54,"ADDRESS",7);
	mkln(85,"CONTACT",7);
	mkln(107,"TELEPHONE #",11);
	mkln(125,"SIZE",4);

	if( PrintLine()<0 )	return(REPORT_ERR);

	mkln(3,"NUMBER",6);
	mkln(107,"FAX #",5);
	mkln(122,"RURAL/TOWN",10);

	if( PrintLine()<0 )	return(REPORT_ERR);
	if( PrintLine()<0 )	return(REPORT_ERR);

	return(NOERROR);
}
static
PrntRec()
{
	char	txt_buff[132];

	tedit((char*)&sch_rec.sc_numb,"_0_",txt_buff,R_SHORT);
	mkln(4,txt_buff,3);
	mkln(13,sch_rec.sc_name,28);
	mkln(45,sch_rec.sc_add1,30);
	mkln(76,sch_rec.sc_contact,25);
	mkln(107,"(",1);
	mkln(108,sch_rec.sc_phone,3);
	mkln(111,")",1);
	mkln(113,sch_rec.sc_phone+3,3);
	mkln(116,"-",1);
	mkln(117,sch_rec.sc_phone+6,4);
	tedit((char*)&sch_rec.sc_size,"____0_",txt_buff,R_LONG);
	mkln(125,txt_buff,6);

	retval = PrintLine();
	if(retval < 0)	return(REPORT_ERR);
	if(retval == EXIT)	return(EXIT);

	mkln(45,sch_rec.sc_add2,30);
	mkln(107,"(",1);
	mkln(108,sch_rec.sc_fax,3);
	mkln(111,")",1);
	mkln(113,sch_rec.sc_fax+3,3);
	mkln(116,"-",1);
	mkln(117,sch_rec.sc_fax+6,4);
	mkln(127,sch_rec.sc_r_t,1);

	retval = PrintLine();
	if(retval < 0)	return(REPORT_ERR);
	if(retval == EXIT)	return(EXIT);

	mkln(45,sch_rec.sc_add3,30);

	retval = PrintLine();
	if(retval < 0)	return(REPORT_ERR);
	if(retval == EXIT)	return(EXIT);

	mkln(45,sch_rec.sc_pc,7);

	retval = PrintLine();
	if(retval < 0)	return(REPORT_ERR);
	if(retval == EXIT)	return(EXIT);
	retval = PrintLine();
	if(retval < 0)	return(REPORT_ERR);
	if(retval == EXIT)	return(EXIT);

	return(NOERROR);
}

static
PrintLine()
{
	if(prnt_line() < 0 )	return(REPORT_ERR);

	if(linecnt > PG_SIZE)
		retval = PrntHdg();
		if(retval <0) return(-1);
		if(retval == EXIT) return(EXIT);

	return(NOERROR);
}

