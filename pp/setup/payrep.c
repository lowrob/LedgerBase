/******************************************************************************
		Sourcename   : payrep.c
		System       : Budgetary Financial system.
		Module       : Uic Table
		Created on   : 91-11-19
		Created  By  : Sheldon Floyd 

******************************************************************************
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
static Emp	emp;
static Pay_earn	pay_earn;
static Earn	earn;
static Pp_ben	pp_ben;
static Pay_ded	pay_ded;
static Time	time;
static Pay_param	pay_param;
static Deduction	deduction;
static Uic		uic;


/*  Data items for storing the key range end values */
static char	barg1[7];
static char	barg2[7];
static char	posi1[7];
static char	posi2[7];
static char	empl1[13];
static char	empl2[13];
	
static int	PG_SIZE;
static int	retval;
static double	tot_ben;
static double	tot_ded;
static char 	discfile[15];	/* for storing outputfile name */
static short	pgcnt; 		/* for page count */
static char	resp[2];	/* for storing response */
static short	copies;

extern char 	e_mesg[80];	/* for storing error messages */
static short	pp_no[10];
static short    hours[10];
static double	min_ins[10], max_ins[10], max_prem[10], yr_prem[10];
static double	yr_earn[10];

/*--------------------------------------------------------------------------*/
pay_rep()
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
			STRCPY( discfile, "payrep.dat" );
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
/*---------------------------------------------------------------------------*/
static
PrintRep()
{

	retval = PrntRec();
	if(retval < 0)		return(retval);
	if(retval == EXIT)	return(NOERROR);

	return(NOERROR);
}
/*----------------------------------------------------------------------------*/
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

	mkln( 1,"pay_rep", 10 );
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
	mkln((LNSZ-9)/2,"UIC TABLE",9);
#else
	mkln((LNSZ-9)/2,"UIC TABLE",9);
#endif
	if( PrintLine()<0 )	return(REPORT_ERR);
	if( PrintLine()<0 )	return(REPORT_ERR);

	return(NOERROR);
}
/*---------------------------------------------------------------------------*/
static
PrntRec()
{
	int nbr = 0, tmp_date, first=0;

	uic.ui_numb = 0;
	uic.ui_date = 0;
	flg_reset(UIC);

	for(;;){
		retval = get_n_uic(&uic,BROWSE,1,FORWARD,e_mesg);
		if(retval == EFL) break;
		if(retval < 0){
			fomen(e_mesg);
			return(retval);
		}
	 	
		if(first == 0){
			retval = PrntHdg();
			if(retval < 0) return(-1);
			if(retval == EXIT);
			first = 1;
		}
		else if(uic.ui_date != tmp_date){
			retval = PrntTable(nbr-1,tmp_date);
			if(retval < 0) return(retval);
 			if(retval == EXIT) return(EXIT);
			nbr = 0;
		}
	
		tmp_date = uic.ui_date;

		pp_no[nbr] = uic.ui_numb;
		hours[nbr] = uic.ui_min_hrs;
		max_ins[nbr] = uic.ui_max_earn;
		min_ins[nbr] = uic.ui_min_earn;
		max_prem[nbr] = uic.ui_max_prem;
		yr_prem[nbr] = uic.ui_yrly_prem;
		yr_earn[nbr] = uic.ui_yrly_earn;

		nbr++;
	}

	seq_over(UIC);

	if(nbr != 0){
		retval = PrntTable(nbr-1,tmp_date);
		if(retval < 0) return(retval);
		if(retval == EXIT) return (EXIT);
	}

	if(term < 99)   /* new page and display */
		last_page();	

	return(NOERROR);
}
/*----------------------------------------------------------------------------*/
static
PrntTable(no_pds,tmp_date)
int no_pds;
long tmp_date;
{
	int i, st_pos = 31, st, end;

	tedit( (char *)&tmp_date,"____/__/__",e_mesg,R_LONG); 
	mkln(1,"DATE:",5);
	mkln(7,e_mesg,10);

	retval =  PrintLine();
	if(retval < 0) return(REPORT_ERR);
	if(retval == EXIT) return(EXIT);

	retval =  PrintLine();
	if(retval < 0) return(REPORT_ERR);
	if(retval == EXIT) return(EXIT);

	st = 0;
	end = 5;

	for(;;){
		mkln(1,"NUMBER OF PAY PERIODS",21);
		for(i=st;i<end;i++){
			if(i > no_pds) break;
			sprintf(e_mesg,"%d",pp_no[i]);
			mkln(st_pos+(i-st)*15,e_mesg,2);
		}
		retval =  PrintLine();
		if(retval < 0) return(REPORT_ERR);
		if(retval == EXIT) return(EXIT);
		retval =  PrintLine();
		if(retval < 0) return(REPORT_ERR);
		if(retval == EXIT) return(EXIT);

		mkln(1,"MINIMUM HOURS",13);
		for(i=st;i<end;i++){
			if(i > no_pds) break;
			sprintf(e_mesg,"%d",hours[i]);
			mkln(st_pos+(i-st)*15,e_mesg,2);
		}
		retval =  PrintLine();
		if(retval < 0) return(REPORT_ERR);
		if(retval == EXIT) return(EXIT);

		mkln(1,"MINIMUM INSURABLE EARNINGS",26);
		for(i=st;i<end;i++){
			if(i > no_pds) break;
			tedit((char *)&min_ins[i],"_____0__.__",e_mesg,R_DOUBLE); 
			mkln(st_pos+(i-st)*15,e_mesg,10);
		}
		retval =  PrintLine();
		if(retval < 0) return(REPORT_ERR);
		if(retval == EXIT) return(EXIT);

		mkln(1,"MAXIMUM INSURABLE EARNINGS",26);
		for(i=st;i<end;i++){
			if(i > no_pds) break;
			tedit((char *)&max_ins[i],"_____0__.__",e_mesg,R_DOUBLE); 
			mkln(st_pos+(i-st)*15,e_mesg,10);
		}
		retval =  PrintLine();
		if(retval < 0) return(REPORT_ERR);
		if(retval == EXIT) return(EXIT);

		mkln(1,"MAXIMUM PREMIUM",15);
		for(i=st;i<end;i++){
			if(i > no_pds) break;
			tedit((char *)&max_prem[i],"_____0__.__",e_mesg,R_DOUBLE); 
			mkln(st_pos+(i-st)*15,e_mesg,10);
		}
		retval =  PrintLine();
		if(retval < 0) return(REPORT_ERR);
		if(retval == EXIT) return(EXIT);

		mkln(1,"YEARLY MAXIMUM PREMIUM",22);
		for(i=st;i<end;i++){
			if(i > no_pds) break;
			tedit((char *)&yr_prem[i],"_____0__.__",e_mesg,R_DOUBLE); 
			mkln(st_pos+(i-st)*15,e_mesg,10);
		}
		retval =  PrintLine();
		if(retval < 0) return(REPORT_ERR);
		if(retval == EXIT) return(EXIT);

		mkln(1,"YEARLY MAXIMUM EARNINGS",23);
		for(i=st;i<end;i++){
			if(i > no_pds) break;
			tedit((char *)&yr_earn[i],"_____0__.__",e_mesg,R_DOUBLE); 
			mkln(st_pos+(i-st)*15,e_mesg,10);
		}
		retval =  PrintLine();
		if(retval < 0) return(REPORT_ERR);
		if(retval == EXIT) return(EXIT);

		if(no_pds < 5) break;
		st += 5;
		end += 5;
		if(st > 5) break;
	}
	retval =  PrintLine();
	if(retval < 0) return(REPORT_ERR);
	if(retval == EXIT) return(EXIT);
	retval =  PrintLine();
	if(retval < 0) return(REPORT_ERR);
	if(retval == EXIT) return(EXIT);

	for(i=0;i<10;i++){
		pp_no[i] = 0;
		hours[i] = 0;
		min_ins[i] = 0;
		max_ins[i] = 0;

		max_prem[i] = 0;
		yr_prem[i] = 0;
		yr_earn[i] = 0;
	}
	
	return(NOERROR);
}
/*----------------------------------------------------------------------------*/
static
PrintLine()
{
	if(prnt_line() < 0 )	return(REPORT_ERR);

	if(linecnt > PG_SIZE){
		retval = PrntHdg();
		if(retval < 0)	return(-1);
		if(retval == EXIT)	return(EXIT);
	}

	return(NOERROR);
}
/*---------------------------  END  OF  FILE  ------------------------------*/
