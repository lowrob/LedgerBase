/******************************************************************************
		Sourcename   : repgl.c
		System       : Budgetary Financial system.
		Module       : Personal Payroll
		Created on   : 92-11-11
		Created  By  : Andre Cormier 

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
static Gl_acct	gl_acct;

/*  Data items for storing the key range end values */
static int	PG_SIZE;
static char 	discfile[15];	/* for storing outputfile name */
static short	pgcnt; 		/* for page count */
static char	resp[2];	/* for storing response */
static short	copies;
static char	earn1[7];
static char	earn2[7];
static int	first_line,first_time;

extern char 	e_mesg[80];	/* for storing error messages */

/*--------------------------------------------------------------------------*/
repgl()
{
	int retval;

	/* Get details for output medium */
#ifdef ENGLISH
	STRCPY(resp,"P");
#else
	STRCPY(resp,"I");
#endif
	if((retval =  GetOutputon(resp))<0 )
		return(retval);

	PG_SIZE = 63;
	switch(*resp) {
		case DISPLAY:
			resp[0]='D';
			STRCPY( discfile, terminal );
			PG_SIZE = 22;
			break;
		case FILE_IO:
			resp[0]='F';
			STRCPY( discfile, "repgl.dat" );
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

	strcpy( earn1, "     1");
	strcpy( earn2, "ZZZZZZ" );
	retval = GetEarnRange( earn1,earn2 );
	if(retval < 0) return(retval);

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
/*---------------------------------------------------------------------------*/
static
PrintRep()
{
	int retval;

	if((retval = PrntRec())<0 )
		return(retval);

	return(NOERROR);
}
/*----------------------------------------------------------------------------*/
static
PrntHdg()	/* Print heading  */
{
	int retval;
	short	offset;
	long	sysdt ;

	first_line = 0;

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
	mkln(30,"PAYROLL G/L ACCOUNT",19);
#else
	mkln(30,"PAYROLL G/L ACCOUNT",19);
#endif
	if( PrintLine()<0 )	return(REPORT_ERR);
	if( PrintLine()<0 )	return(REPORT_ERR);
	if( PrintLine()<0 )	return(REPORT_ERR);


#ifdef ENGLISH
	mkln(20,"CLASSIFICATION",14);
	mkln(40,"EARNINGS",8);
#else
	mkln(20,"CLASSIFICATION",14);
	mkln(40,"EARNINGS",8);
#endif
	if( PrintLine()<0 )	return(REPORT_ERR);

#ifdef ENGLISH
	mkln(1,"FUND",4);
	mkln(10,"TYPE",4);
	mkln(25,"CODE",4);
	mkln(42,"CODE",4);
	mkln(54,"CC #",4);
	mkln(64,"ACCOUNT #",9);
#else
	mkln(1,"FUND",4);
	mkln(10,"TYPE",4);
	mkln(25,"CODE",4);
	mkln(42,"CODE",4);
	mkln(54,"CC #",4);
	mkln(65,"ACCOUNT #",9);
#endif
	if( PrintLine()<0 )	return(REPORT_ERR);
	if( PrintLine()<0 )	return(REPORT_ERR);

	return(NOERROR);
}
/*---------------------------------------------------------------------------*/
static
PrntRec()
{
	int retval;
	short	old_fund;
	char	old_type[2],	old_class[7],	old_earn[7];


	first_time = 0;

	gl_acct.gl_fund = 0;
	gl_acct.gl_type[0] = '\0';
	gl_acct.gl_class[0] = '\0';
	gl_acct.gl_earn[0] = '\0';
	gl_acct.gl_cc = 0;
	flg_reset(GLACCT);

	for(;;){
		retval = get_n_glacct(&gl_acct,BROWSE,1,FORWARD,e_mesg);
		if(retval == EFL) break;
		if(retval < 0){
			fomen(e_mesg);
			return(retval);
		}
	 	
		if(strcmp(gl_acct.gl_earn,earn1) < 0 ||
		   strcmp(gl_acct.gl_earn,earn2) > 0) {
			continue;
		}

		if(first_time == 0) {
			first_time = 1;
			if((retval = PrntHdg())<0 )
				return(retval);
			old_fund = gl_acct.gl_fund;
			strcpy(old_type,gl_acct.gl_type);
			strcpy(old_class,gl_acct.gl_class);
			strcpy(old_earn,gl_acct.gl_earn);
		}
		if(first_line == 0 || old_fund != gl_acct.gl_fund) {
			first_line = 1;
			sprintf(e_mesg,"%d",gl_acct.gl_fund);
			mkln(1,e_mesg,3);
			mkln(11,gl_acct.gl_type,1);
			mkln(24,gl_acct.gl_class,6);
			mkln(42,gl_acct.gl_earn,6);

			old_fund = gl_acct.gl_fund;
			strcpy(old_type,gl_acct.gl_type);
			strcpy(old_class,gl_acct.gl_class);
			strcpy(old_earn,gl_acct.gl_earn);
		}
		else  {
			if(strcmp(old_type,gl_acct.gl_type) != 0) {
				mkln(11,gl_acct.gl_type,1);
				mkln(24,gl_acct.gl_class,6);
				mkln(42,gl_acct.gl_earn,6);

				strcpy(old_type,gl_acct.gl_type);
				strcpy(old_class,gl_acct.gl_class);
				strcpy(old_earn,gl_acct.gl_earn);
			}
			else {
				if(strcmp(old_class,gl_acct.gl_class) != 0) {
					mkln(24,gl_acct.gl_class,6);
					mkln(42,gl_acct.gl_earn,6);

					strcpy(old_class,gl_acct.gl_class);
					strcpy(old_earn,gl_acct.gl_earn);
				}
				else {
					if(strcmp(old_earn,gl_acct.gl_earn)!=0){
						mkln(42,gl_acct.gl_earn,6);

						strcpy(old_earn,gl_acct.gl_earn);
					}
				}
			}
		}

		sprintf(e_mesg,"%d",gl_acct.gl_cc);
		mkln(54,e_mesg,4);
		mkln(61,gl_acct.gl_acct,18);

		retval = PrintLine();
		if(retval < 0)	return(REPORT_ERR);
		if(retval == EXIT)	return(EXIT);
	}

	return(NOERROR);
}
/*----------------------------------------------------------------------------*/
static
PrintLine()
{
	int	retval;

	if(prnt_line() < 0 )	return(REPORT_ERR);

	if(linecnt > PG_SIZE){
		retval = PrntHdg();
		if(retval <0) return(-1);
		if(retval == EXIT) return(EXIT);
	}

	return(NOERROR);
}

/*---------------------------  END  OF  FILE  ------------------------------*/
