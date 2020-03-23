/****************************************************************************
Sourcename	: substute.c
System		: Payroll
Module		: Attendance
Created on	: 96-09-08
Created  By	: L.Robichaud 
******************************************************************************
About the file: 
	This program will print a list of employee that have had replacements
to a form.

History:
Programmer	Last change on	Details

******************************************************************************/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_pp.h>
#include <bfs_com.h>
#include <repdef.h>
#include <cfomstrc.h>
#include <filein.h>
#include <isnames.h>

#define EXIT		12

#ifdef ENGLISH
#define PRINTER		'P'
#define DISPLAY		'D'
#define FILE_IO		'F'
#else
#define PRINTER		'I'

#define DISPLAY		'A'
#define FILE_IO		'D'
#endif

static Emp	emp_rec;
static Emp_at_his	att_hist;
static Pa_rec		pa_rec;
static Sch_rec		sch_rec;

struct Day_of_wk{
	double	hours;
	char	code[4];
};
struct Day_of_wk	day_of_wk[6];

/*  Data items for storing the key range end values */
static long	date1;
static long	date2;
static char	barg1[7];
static char	barg2[7];
static char	empl1[13];
static char	empl2[13];
static int	page_flag;	/* flag to print new form or not */
static int	PG_SIZE;
static int	retval;
static char	discfile[15];	/* for storing outputfile name */
static char	resp[2];        /* for storing response */
static short	copies;
static double	att_total;

extern char	e_mesg[80];     /* for storing error messages */

substute()
{
	char	tmpindxfile[50];
	char	tnum[5];
	long	julian, remain;


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
			PG_SIZE = 13;
			break;
		case FILE_IO:
			resp[0]='F';
			STRCPY( discfile, "subst.dat" );
			if((retval = GetFilename(discfile))<0 )
				return(retval);
			PG_SIZE = 50;
			break;
		case PRINTER:
		default:
			resp[0]='P';
			discfile[0]= '\0';
			PG_SIZE = 50;
			break;
	}

	copies = 1;
	if( *resp=='P' ){
		if((retval = GetNbrCopies(&copies))<0 )
			return(retval);
	}

	date1 = get_date();
/*****
	date2 = date1 + 10000;
	if((retval = GetDateRange(&date1,&date2))<0)
*****/
	/* Put a continuious loop to get a monday */
	for(;;){
		if(retval = GetDate1(&date1)<0)
			return(retval);
		/* Figure out the day of the week */
		julian = days(date1);
		remain = julian %7;
		/* Monday is equal to 1 */
		if(remain == 1)
			break;
		fomen("This date must be a Monday");
	}
	/* Set date2 to friday, (4 days after date1) */
	date2 = date_plus(date1,4);
	sprintf(e_mesg,"The end date for this report is %ld",date2);
	fomen(e_mesg);

	strcpy( barg1, "     0");
	strcpy( barg2, "ZZZZZZ" );
	retval = GetBargRange( barg1,barg2 );
	if(retval < 0) return(retval);
	if(strcmp( barg1, "     0")==0) 
		barg1[0] = '\0';

	strcpy( empl1, "           0");
	strcpy( empl2, "ZZZZZZZZZZZZ" );
	retval = GetEmpRange( empl1,empl2 );
	if(retval < 0) return(retval);

	if( (retval=Confirm())<=0 )
		return(retval);

	if( opn_prnt( resp, discfile, 1, e_mesg, 1 /* spool */)<0 ){
		return(REPORT_ERR);
	}
	if( *resp=='P' )
		SetCopies( (int)copies );
	LNSZ = 66;             /* line size in no. of chars */
	linecnt = PG_SIZE;      /* Page size in no. of lines */

	retval = ProcessRep();

	close_rep();    /* close output file */
	close_dbh();
	return(retval);
}

/******************************************************************************
Main logic of the program
******************************************************************************/
static
ProcessRep()
{
	double  total_hours = 0.0;
	char    tmp_emp_numb[13];

	/* get the cost center name */
	retval = get_param(&pa_rec,BROWSE,1,e_mesg);
	if (retval < 0){
		fomer(e_mesg);get ();
	}

	sch_rec.sc_numb = pa_rec.pa_distccno;
	retval = get_sch(&sch_rec,BROWSE,0,e_mesg);
	if (retval < 0){
		fomer(e_mesg);get();
	}

	strcpy(att_hist.eah_numb, empl1);
	att_hist.eah_date = date1;
	flg_reset(EMP_ATT);
	for(;;){
		retval = get_n_emp_at(&att_hist,BROWSE,0,FORWARD,e_mesg);
		if(retval == EFL)       break;
		if(retval < 0)          return(retval);         

		if(strcmp(att_hist.eah_numb, empl2) > 0)
			break;

		if(att_hist.eah_date < date1)
			continue;
		if(att_hist.eah_date > date2)
			continue;

		/* Check for a replacement */
		if(att_hist.eah_teach[0] == NULL)
			continue;

		/* Check the bargining unit range */
		
		strcpy(emp_rec.em_numb,att_hist.eah_numb);
		retval=get_employee(&emp_rec,BROWSE,0,e_mesg);
		if(retval<0){
			fomer(e_mesg);
			continue;
		}
		
		if(strcmp(emp_rec.em_barg,barg1) < 0)
			continue;
		if(strcmp(emp_rec.em_barg,barg2) > 0)
			continue;

		/* If the employee changed, print info if there is any */
		if(strcmp(att_hist.eah_numb, tmp_emp_numb)){
			if(att_total){
				if(retval=PrintTotalAtt()<0)
					return(retval);
				if( rite_top()<0 ) return( -1 );
			}

			if(retval=PrntHdg() < 0)
				return(retval);
			strcpy(tmp_emp_numb, att_hist.eah_numb);
		}

		if((retval = PrntRectostock())<0)
			return(retval);
		if(retval == EXIT)      break;

		att_hist.eah_date++;

	} /* End of for */

	if(att_total)
		if(retval=PrintTotalAtt()<0)
			return(retval);

	return(NOERROR);
}



/******************************************************************************
Prints the headings of the report 
******************************************************************************/
static
PrntHdg()       /* Print heading  */
{
	short   offset;
	long    sysdt ;
	short   name_size;
	char    txt_line[132];

	mkln( 1, PROG_NAME, 10 );

	att_total = 0.0;

	sysdt = get_date() ;
	tedit( (char *)&sysdt,"____/__/__",line+cur_pos, R_LONG ); 
	cur_pos += 10;

	if(prnt_line() < 0 )    return(REPORT_ERR);
	if(prnt_line() < 0 )    return(REPORT_ERR);
	if(prnt_line() < 0 )    return(REPORT_ERR);

	mkln(15,"01",2);
	mkln(25,pa_rec.pa_co_name,50);
	if(prnt_line() < 0 )    return(REPORT_ERR);
	if(prnt_line() < 0 )    return(REPORT_ERR);
	if(prnt_line() < 0 )    return(REPORT_ERR);

	sprintf(txt_line,"%l",sch_rec.sc_numb);
	mkln(15,txt_line,2);
	mkln(25,sch_rec.sc_name,28);
	if(prnt_line() < 0 )    return(REPORT_ERR);
	if(prnt_line() < 0 )    return(REPORT_ERR);
	if(prnt_line() < 0 )    return(REPORT_ERR);

	mkln(3,emp_rec.em_sin,9);
	mkln(22,emp_rec.em_last_name,25);
	mkln(48,emp_rec.em_first_name,15);

	if(prnt_line() < 0 )    return(REPORT_ERR);
	if(prnt_line() < 0 )    return(REPORT_ERR);
	if(prnt_line() < 0 )    return(REPORT_ERR);

	mkln(22,emp_rec.em_maid_name,15);
	if(prnt_line() < 0 )    return(REPORT_ERR);
	if(prnt_line() < 0 )    return(REPORT_ERR);
	if(prnt_line() < 0 )    return(REPORT_ERR);
	if(prnt_line() < 0 )    return(REPORT_ERR);
	if(prnt_line() < 0 )    return(REPORT_ERR);
	if(prnt_line() < 0 )    return(REPORT_ERR);

	/* Get Substute Info */
	strcpy(emp_rec.em_numb,att_hist.eah_teach);
	retval=get_employee(&emp_rec,BROWSE,0,e_mesg);
	if(retval){
		fomer(e_mesg);
		get();
		fomer("Substitute employee info not found");
		get();
	}
	else{
		mkln(3,emp_rec.em_sin,9);
		mkln(22,emp_rec.em_last_name,25);
		mkln(48,emp_rec.em_first_name,15);
	}

	if(prnt_line() < 0 )    return(REPORT_ERR);
	if(prnt_line() < 0 )    return(REPORT_ERR);
	if(prnt_line() < 0 )    return(REPORT_ERR);

	tedit( (char *)&date2,"____/__/__",txt_line, R_LONG ); 
	mkln(45,txt_line,10);

	if(prnt_line() < 0 )    return(REPORT_ERR);
	if(prnt_line() < 0 )    return(REPORT_ERR);

	page_flag = 'Y';

	return(NOERROR);
}

static
PrntRectostock()
{
	long	julian, remain;

	julian = days(att_hist.eah_date);
	remain = julian %7;

	day_of_wk[remain-1].hours = att_hist.eah_hours;
	strcpy(day_of_wk[remain-1].code, att_hist.eah_code);

	att_total+=att_hist.eah_hours;

	return(NOERROR);
}

static
PrntRec()
{
	char    txt_line[132];
	int	i;
	int	line_cnt;	/* Used to adjust print position */

	/* reset the line count to print the hours absent */
	line_cnt = 10;

	for(i=0;i<5;i++){
		sprintf(txt_line,"%3.2f",day_of_wk[i].hours);
		mkln(line_cnt,txt_line,5);

		line_cnt+=6;
	}

	if(prnt_line() < 0 )    return(REPORT_ERR);

	/* reset the line count to print the attendance codes */
	line_cnt = 10;

	for(i=0;i<5;i++){
		mkln(line_cnt,day_of_wk[i].code,5);

		line_cnt+=6;
	}

	if(prnt_line() < 0 )    return(REPORT_ERR);

	/* Clear the Structure */
	for(i=0;i<5;i++){
		day_of_wk[i].hours = 0;
		day_of_wk[i].code[0] = NULL;
	}

	return(NOERROR);
}

PrintTotalAtt ()
{
	char    txt_line[132];

	if((retval = PrntRec())<0)
		return(retval);

	sprintf(txt_line,"%3.2f",att_total);
	mkln(40,"Total:",6);
	mkln(47,txt_line,5);
	if(prnt_line() < 0 )    return(REPORT_ERR);

	att_total = 0;

	return(NOERROR);
}
	
/******************   END OF PROGRAM *******************************/
