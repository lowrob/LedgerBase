/******************************************************************************
		Sourcename   : salary.c
		System       : Budgetary Financial system.
		Module       : Personal Payroll
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
static Salary	sal;

/*  Data items for storing the key range end values */
static int	PG_SIZE;
static int 	retval;
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
salary()
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
			STRCPY( discfile, "salary.dat" );
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

	if((retval = PrntRec())<0 )
		return(retval);
	if(retval == EXIT)	return(retval);

	return(NOERROR);
}
/*----------------------------------------------------------------------------*/
static
PrntHdg()	/* Print heading  */
{
	int retval;
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
	mkln(56,"SALARY G/L ACCOUNT",18);
#else
	mkln(56,"SALARY G/L ACCOUNT",18);
#endif
	if( PrintLine()<0 )	return(REPORT_ERR);
	if( PrintLine()<0 )	return(REPORT_ERR);
	if( PrintLine()<0 )	return(REPORT_ERR);

#ifdef ENGLISH
	mkln(1,"FUND",4);
	mkln(8,"CLASSIFICATION  EARNINGS",24);
	mkln(50,"G/L SALARY ACCOUNT KEYS",23);
#else
	mkln(1,"FUND",4);
	mkln(8,"CLASSIFICATION  EARNINGS",24);
	mkln(50,"G/L SALARY ACCOUNT KEYS",23);
#endif
	if( PrintLine()<0 )	return(REPORT_ERR);

#ifdef ENGLISH
	mkln(12,"CODE",4);
	mkln(26,"CODE",4);
#else
	mkln(12,"CODE",4);
	mkln(26,"CODE",4);
#endif
	if( PrintLine()<0 )	return(REPORT_ERR);
	if( PrintLine()<0 )	return(REPORT_ERR);

	return(NOERROR);
}
/*---------------------------------------------------------------------------*/
static
PrntRec()
{
	int retval, col = 39, j, i, count;
	int	first_time;

	first_time = 0;

	sal.sa_fund = 0;
	sal.sa_class[0] = '\0';
	sal.sa_earn[0] = '\0';
	flg_reset(SALARY);

	for(;;){
		retval = get_n_salary(&sal,BROWSE,0,FORWARD,e_mesg);
		if(retval == EFL) break;
		if(retval < 0){
			
			fomen(e_mesg);
			return(retval);
		}

		if(first_time == 0) {
			retval = PrntHdg();
			if(retval < 0) return(-1);
			if(retval == EXIT)	return(EXIT);

			first_time = 1;
		}
	 	
		sprintf(e_mesg,"%d",sal.sa_fund);
		mkln(1,e_mesg,3);
		mkln(11,sal.sa_class,6);
		mkln(25,sal.sa_earn,6);

		count = 0;
		for(i=0;i<4;i++){
			for(j=0;j<3;j++){
				mkln(col+j*18,"KEY-",4);
				sprintf(e_mesg,"%d",count+1);
				mkln((col+4)+j*18,e_mesg,2);
				sprintf(e_mesg,"%d",sal.sa_keys[count]);
				mkln((col+8)+j*18,e_mesg,6);
				count++;
			}
			retval = PrintLine();
			if(retval < 0)	return(REPORT_ERR);
			if(retval == EXIT) 	return(EXIT);
		}
		retval = PrintLine();
		if(retval < 0)	return(REPORT_ERR);
		if(retval == EXIT)	return(EXIT);
	}

	seq_over(SALARY);
	if(retval == EFL) return(0);
 	rpclose();
	return(NOERROR);
}
/*----------------------------------------------------------------------------*/
static
PrintLine()
{
	if(prnt_line() < 0 )	return(REPORT_ERR);

	if(linecnt > PG_SIZE)
		retval = PrntHdg();
		if(retval < 0)	return(-1); 
		if(retval == EXIT) return(EXIT);

	return(NOERROR);
}
/*---------------------------  END  OF  FILE  ------------------------------*/
