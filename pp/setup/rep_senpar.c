/******************************************************************************
		Sourcename   : rep_senpar.c
		System       : Personnel/Payroll
		Created on   : 92-03-15
		Created  By  : Eugene Roy

******************************************************************************
About the file:	

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
static	Sen_par	sen_par;
static	Barg_unit	barg_unit;
static	Position	position;
static	Pay_param	pay_param;

/*  Data items for storing the key range end values */
	
static int	PG_SIZE;
static int	retval;
static char 	temp_buf[80];
static char 	discfile[15];	/* for storing outputfile name */
static short	pgcnt; 		/* for page count */
static char	resp[2];	/* for storing response */
static short	copies;
static long	old_date;
static int	noitems;

short	d_month[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

char	month[12][4] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG",
			"SEP", "OCT", "NOV", "DEC" };
int	start_mth;
#define	JAN	01
#define	JUL	07
#define	DEC	12

char	cal_month[12][4];
char	month_day[12][32];
double	poss_days[12];

extern char 	e_mesg[80];	/* for storing error messages */

rep_senpar()
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
			PG_SIZE = 21;
			resp[0]='D';
			STRCPY( discfile, terminal );
			break;
		case FILE_IO:
			PG_SIZE = 63;
			resp[0]='F';
			STRCPY( discfile, "rep_senpar.dat" );
			if((retval = GetFilename(discfile))<0 )
				return(retval);
			break;
		case PRINTER:
		default:
			PG_SIZE = 63;
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
	linecnt = PG_SIZE;	/* Page size in no. of lines */

	retval = PrintRep();
	if(retval < 0)	return(retval);

	retval = CloseRtn();
	return(retval);
}

/*----------------------------------------------------------------*/
/* Close nessary files and environment before exiting program               */

static
CloseRtn() 
{
	close_rep();	/* close output file */
	close_dbh();	/* Close files */

	return(NOERROR);
}	/* CloseRtn() */
/*----------------------------------------------------------------*/
static
PrintRep()
{
	int retval;

 	retval = get_pay_param(&pay_param,BROWSE,1,e_mesg);
	if(retval < 0) {
		return(retval);
	}

	retval = PrntHdg();
	if(retval == EXIT)
		return(NOERROR);
	
	sen_par.sn_position[0] = '\0';
	sen_par.sn_eff_date = 0;
	flg_reset(SEN_PAR);

	for( ; ; ){
		retval = get_n_sen_par(&sen_par, BROWSE, 0, FORWARD, e_mesg);
		if(retval == ERROR) return(DBH_ERR) ;
		if(retval == EFL) 
			break;

		retval = PrintRec();
		if(retval == EXIT)
			break;
	}
	seq_over(SEN_PAR);

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
/*----------------------------------------------------------*/
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
	if(retval == EXIT)	return(retval);
	if(retval < 0)		return(REPORT_ERR);

	offset = ( LNSZ-strlen(pa_rec.pa_co_name) )/2;
	mkln( offset, pa_rec.pa_co_name, strlen(pa_rec.pa_co_name) );
	retval = PrintLine();
	if(retval == EXIT)	return(retval);
	if(retval < 0)		return(REPORT_ERR);

#ifdef ENGLISH
	mkln((LNSZ-26)/2,"SENIORITY PARAMETER REPORT", 26 );
#else
	mkln((LNSZ-20)/2,"SENIORITY PARAMETER REPORT", 25 ); 
#endif
	retval = PrintLine();
	if(retval == EXIT)	return(retval);
	if(retval < 0)		return(REPORT_ERR);
	retval = PrintLine();
	if(retval == EXIT)	return(retval);
	if(retval < 0)		return(REPORT_ERR);

	return(NOERROR);
}
/*----------------------------------------------------------------*/
static
PrintLine()
{
	retval = prnt_line();	
	if(retval == EXIT)
		return(EXIT);
	if(retval < 0)
		return(REPORT_ERR);

	if(linecnt > PG_SIZE)
		retval = PrntHdg();

	if(retval == EXIT)
		return(EXIT);

	return(NOERROR);
}
/*-----------------------------------------------------------*/
/* Move Header to Screen Hdr Fields */
PrintRec()
{
	int	i, j, retval;
	
	mkln( 1,sen_par.sn_position, 6 );

	strcpy(position.p_code, sen_par.sn_position);
	retval = get_position(&position,BROWSE,0,e_mesg);
	if(retval < 0 ){
	  fomer("Bargaining Unit Code Does Not Exist");
	  return(ERROR);
	}
	mkln( 8,position.p_desc, 30 );

	tedit( (char *)&sen_par.sn_eff_date,"____/__/__", temp_buf, R_LONG ); 
	mkln( 50,temp_buf, 10 );
	retval = PrintLine();
	if(retval == EXIT)	return(retval);
	if(retval < 0)		return(REPORT_ERR);
	retval = PrintLine();
	if(retval == EXIT)	return(retval);
	if(retval < 0)		return(REPORT_ERR);

	tedit( (char *)&sen_par.sn_max_days_yr,"_0_.__", temp_buf, R_DOUBLE ); 
	mkln( 50,temp_buf, 6 );
	tedit( (char *)&sen_par.sn_num_hrs_day,"_0_.__", temp_buf, R_DOUBLE ); 
	mkln( 60,temp_buf, 6 );
	retval = PrintLine();
	if(retval == EXIT)	return(retval);
	if(retval < 0)		return(REPORT_ERR);

	strcpy(barg_unit.b_code,sen_par.sn_barg);
	barg_unit.b_date = get_date();
	flg_reset(BARG);

	retval = get_n_barg(&barg_unit,BROWSE,0,BACKWARD,e_mesg);
	if((retval < 0 ||
	  strcmp(barg_unit.b_code,sen_par.sn_barg) != 0)){
	  fomer("Bargaining Unit Code Does Not Exist - Please Re-enter");
	  return(ERROR);
	}
	mkln( 1,barg_unit.b_code, 6 );
	
	retval = PrintLine();
	if(retval == EXIT)	return(retval);
	if(retval < 0)		return(REPORT_ERR);

	Disp_Mth();
	for(i=0; i< 12; i++){
	  if((start_mth + i )> 12)
		j = i - start_mth+1 ;
	  else
		j = i + start_mth-1;
	  mkln( 1,month[j], 3 );
	  mkln( 8,sen_par.sn_month[i], 31);
	  tedit( (char *)&sen_par.sn_poss_days[i],"0_", temp_buf, R_DOUBLE ); 
	  mkln( 54,temp_buf, 2 );

	  retval = PrintLine();
	  if(retval == EXIT)	return(retval);
	  if(retval < 0)		return(REPORT_ERR);

	  retval = PrintLine();
	  if(retval == EXIT)	return(retval);
	  if(retval < 0)		return(REPORT_ERR);
	}
	if(resp[0] != 'D')
		rite_top();

	return(NOERROR) ;
}	/* ShowScreen() */
/*-----------------------------------------------------------*/
Disp_Mth()
{
	int	retval;

	if(pay_param.pr_st_mth == 1){
		start_mth = ((pay_param.pr_cal_st_dt / 100) % 100);
	}
	if(pay_param.pr_st_mth == 2){
		start_mth = ((pay_param.pr_fisc_st_dt / 100) % 100);
	}
	if(pay_param.pr_st_mth == 3){
		start_mth = ((pay_param.pr_schl_st_dt / 100) % 100);
	}
	return(NOERROR);
}
/*-------------------- E n d   O f   P r o g r a m ---------------------*/
