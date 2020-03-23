/******************************************************************************
		Sourcename   : strep.c
		System       : 
		Module       :
		Created on   : 93-06-24
		Created  By  : Andre Cormier 
******************************************************************************
About the file:	
	This program will print a list of casual employees with their calendar.

History:
Programmer      Last change on    Details
******************************************************************************/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_pp.h>
#include <bfs_com.h>
#include <repdef.h>
#include <cfomstrc.h>
#include <filein.h>
#include <isnames.h>


#define	WEEKEND		'W'
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

static Emp		emp_rec;
static Pa_rec		pa_rec;
static Barg_unit	barg_unit;
static Time		time;
static Time_his		time_his;
static Pay_per_it	pay_per_it;
static Class		class_rec;
static Sen_par		sen_par;
static Position		position;

/*  Data items for storing the key range end values */
static char	empl1[13];
static char	empl2[13];
static char	pos1[7];
static char	pos2[7];
static char	barg1[7];
static char	barg2[7];
static long	date1;

static int	PG_SIZE;
static int	retval;
static char 	discfile[15];	/* for storing outputfile name */
static short	pgcnt; 		/* for page count */
static char	resp[2];	/* for storing response */
static short	copies;

static int	count,day, mth, year;
static long	first_day;
static long	last_day;
static long	pay_end_date;
static long	start_mth;
static long	start_date;
extern char 	e_mesg[200];	/* for storing error messages */
char   items[13][32];	/* for storing error messages */
char   s_mth[13][4];	/* for storing error messages */

double mtd_days[13];	
double mtd_hrs[13];
double mtd_tot[13];
double ytd_bal[13]; 

static short d_month[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

static char month[12][4] =
		 { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG",
			"SEP", "OCT", "NOV", "DEC" };

strep()
{

	count = 0;
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
			strcpy( discfile, terminal );
			PG_SIZE = 13;
			break;
		case FILE_IO:
			resp[0]='F';
			strcpy( discfile, "strep.dat" );
			if((retval = GetFilename(discfile))<0 )
				return(retval);
			PG_SIZE = 55;
			break;
		case PRINTER:
		default:
			resp[0]='P';
			discfile[0]= '\0';
			PG_SIZE = 55;
			break;
	}

	copies = 1;
	if( *resp=='P' ){
		if((retval = GetNbrCopies(&copies))<0 )
			return(retval);
	}

	strcpy( empl1, "           0");
	strcpy( empl2, "ZZZZZZZZZZZZ" );
	retval = GetEmpRange( empl1,empl2 );
	if(retval < 0) return(retval);

	strcpy( pos1, "     0");
	strcpy( pos2, "ZZZZZZ" );
	retval = GetPosRange( pos1,pos2 );
	if(retval < 0) return(retval);

	strcpy( barg1, "     0");
	strcpy( barg2, "ZZZZZZ" );
	retval = GetBargRange( barg1,barg2 );
	if(retval < 0) return(retval);

	date1= get_date();
	retval = GetDate( &date1 );
	if(retval < 0) return(retval);

	if( (retval=Confirm())<=0 )
		return(retval);

	if( opn_prnt( resp, discfile, 1, e_mesg, 1 /* spool */)<0 ){
		return(REPORT_ERR);
	}
	if( *resp=='P' )
		SetCopies( (int)copies );
	pgcnt = 0;		/* Page count is zero */
	LNSZ = 80;		/* line size in no. of chars */
	linecnt = PG_SIZE;	/* Page size in no. of lines */

	retval = PrintRpt();

	close_rep();	/* close output file */
	close_dbh();
	return(retval);
}
/******************************************************************************
Main logic of the program
******************************************************************************/
static
PrintRpt()
{
	/* get the cost center name */
	retval = get_param(&pa_rec,BROWSE,1,e_mesg);
	if (retval < 0){
		fomer(e_mesg);get ();
	}

	strcpy(emp_rec.em_numb,empl1);
	flg_reset(EMPLOYEE);

	for(;;) {
		retval=get_n_employee(&emp_rec,BROWSE,0,FORWARD,e_mesg);
		if(retval==EFL)	return(retval);
		if(retval<0){
			fomer(e_mesg);
			continue;
		}
  		retval=UsrBargVal(BROWSE,emp_rec.em_numb,
					emp_rec.em_barg,0,e_mesg);
  		if(retval < 0)
			continue;

		if(strcmp(emp_rec.em_numb,empl2) > 0){
 			break;
		}

		if(strcmp(emp_rec.em_pos,pos1) < 0 ||
		   strcmp(emp_rec.em_pos,pos2) > 0){
 			continue;
		}

		if(strcmp(emp_rec.em_barg,barg1) < 0 ||
		   strcmp(emp_rec.em_barg,barg2) > 0){
 			continue;
		}

		if ((retval = GetStatus())<0)
			return(retval);

		if ((retval = PrntCal())<0)
			return(retval);
		if(retval == EXIT)	break;

	}/*end of for loop*/

	seq_over(EMPLOYEE);

	if(pgcnt){
		if(term<99)
			last_page();
	}
	return(NOERROR);
}
/******************************************************************************
Prints the headings of the report EMPLOYEE ADDRESS LIST
******************************************************************************/
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
	if(prnt_line() < 0 )	return(REPORT_ERR);

	offset = (LNSZ - strlen(pa_rec.pa_co_name))/2;
	mkln(offset,pa_rec.pa_co_name,strlen(pa_rec.pa_co_name));
	if(prnt_line() < 0 )	return(REPORT_ERR);

#ifdef ENGLISH
	mkln((LNSZ-22)/2,"CASUAL EMPLOYEE STATUS", 22 );
#else
	mkln((LNSZ-22)/2,"TRANSLATE        ", 22 );
#endif

	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	return(NOERROR);
}
/*----------------------------------------------------------------------*/
static
PrntCal()
{
	char	txt_line[132];
	int	i;


	if(first_day == 29999999)
		return(NOERROR);

	if(count%2 == 0) { 
		retval = PrntHdg();
		if(retval < 0) 	{
			return(retval);	
		}
		if(retval == EXIT)	return(retval);
	}
	count++;

	strcpy(pay_per_it.ppi_code, barg_unit.b_pp_code);
	pay_per_it.ppi_st_date = last_day;
	flg_reset(PAY_PER_ITEM);

	retval = get_n_pp_it(&pay_per_it,BROWSE,1,BACKWARD,e_mesg);
	if(retval < 0 || strcmp(pay_per_it.ppi_code,barg_unit.b_pp_code)!=0){
		fomen("Error Reading Pay Period Item File");
		return(retval);
	}
	pay_end_date = pay_per_it.ppi_end_date;	
	
	mkln(1,emp_rec.em_numb,12);
	sprintf(txt_line,"%s, %s %s",
		emp_rec.em_last_name,
		emp_rec.em_first_name,
		emp_rec.em_mid_name);
	mkln(14,txt_line,35);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	mkln(5,"         1         2         3          MONTH TO DATE          YTD",66);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	mkln(5,"1234567890123456789012345678901    DAYS   HOURS     TOTAL    BALANCE",68);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	for(i=0;i<13;i++) {
		mkln(1,s_mth[i],3);
		mkln(5,items[i],31);
		tedit( (char *)&mtd_days[i],"0_.__",txt_line, R_DOUBLE ); 
		mkln(39,txt_line,5);	
		tedit( (char *)&mtd_hrs[i],"__0_.__",txt_line, R_DOUBLE ); 
		mkln(46,txt_line,7);	
		tedit( (char *)&mtd_tot[i],"__0_.__",txt_line, R_DOUBLE ); 
		mkln(56,txt_line,7);	
		tedit( (char *)&ytd_bal[i],"__0_.__",txt_line, R_DOUBLE ); 
		mkln(66,txt_line,7);	
		if(prnt_line() < 0 )	return(REPORT_ERR);
	}
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(1,"First Day Worked:",17);
	tedit( (char *)&first_day,"____/__/__",txt_line, R_LONG ); 
	mkln(19,txt_line,10);	
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(1,"Last Day Worked:",16);
	tedit( (char *)&last_day,"____/__/__",txt_line, R_LONG ); 
	mkln(18,txt_line,10);	
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(1,"Pay Period Ending Date:",23);
	tedit( (char *)&pay_end_date,"____/__/__",txt_line, R_LONG ); 
	mkln(25,txt_line,10);	
	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	return(NOERROR);
}

/*--------------------------------------------------------------------
 GetStatus()  - this routine reads the employee seniority file and   
   		 shows what days the employee has hours for 
--------------------------------------------------------------------*/
int
GetStatus()
{
	int 	i,j, curr_month, prev_mon, leap_year ;
	int	nbr_of_mths = 12,nbr_of_times = 13, curr_day;
	int	month_nbr, start_year, curr_year, first_time, this_year;
	short	start_pp_numb, start_pp_year,end_pp_numb,
		end_pp_year,temp_mth;
	long	end_date, time_unit_date;
	long	temp_day,one = 1, value;

	for(i=0;i<13;i++) {
		strcpy(items[i], "                               ");
	}

	first_day = 29999999;
	last_day = 0;
	temp_day = 0;
	curr_month = 1;

	/* initialize screen */

	end_date = date1;

	start_mth = ((end_date/100)%100);

	start_date = end_date - 10000;
	start_date += 1;
	mth = ((start_date/100)%100);
	day = (start_date % 100);
	year = (start_date / 10000);
	leap_year = year % 4; 
	if(mth == 2 && day == 29 && leap_year == 0)
		start_date = (year * 10000) + (mth * 100) + day;
	else {
		if(day > d_month[mth-1]) {
			day = 1;
			if(mth == 12){
				mth = 1;
				year++;
			}
			else
				mth++;
			start_date = (year * 10000) + (mth * 100) + day;
		}
	}
	
	start_year = start_date / 10000;

	first_time = 0;

	for (i=0; i < 13; i++) {
		if((start_mth + i )> 12)
			j = start_mth+i - 12 - 1 ;
		else
			j = i + start_mth-1;
		if(first_time == 0){
			first_time = 1;
			this_year = start_year;
		}
		else {
			if(j == 0)
				this_year++;
		}	
		strcpy(s_mth[i], month[j]);
		/* Check for leap year	*/
		if((this_year % 4 == 0) && j == 1)
			strncpy(items[i], 
			"...............................",d_month[j]+1);
		else
			strncpy(items[i], 
			"...............................",d_month[j]);
		mtd_days[i] = 0;	
		mtd_hrs[i] = 0;
		mtd_tot[i] = 0;
		ytd_bal[i] = 0; 
	}

	Get_day();

	strcpy(barg_unit.b_code,emp_rec.em_barg);
	barg_unit.b_date = date1;
	flg_reset(BARG);

	retval = get_n_barg(&barg_unit,BROWSE,0,BACKWARD,e_mesg);
	if(retval < 0 || strcmp(barg_unit.b_code,emp_rec.em_barg)!=0){
		fomen("Error Reading Bargaining Unit File");
		return(-1);
	}
	seq_over(BARG);

	strcpy(pay_per_it.ppi_code,barg_unit.b_pp_code);
	pay_per_it.ppi_st_date = start_date;
	flg_reset(PAY_PER_ITEM);

	retval = get_n_pp_it(&pay_per_it,BROWSE,1,BACKWARD,e_mesg);
	if(retval < 0 || strcmp(pay_per_it.ppi_code,barg_unit.b_pp_code)!=0){
		fomen("Error Reading Pay Period Item File");
		return(-1);
	}
	seq_over(PAY_PER_ITEM);
	start_pp_numb = pay_per_it.ppi_numb;
	start_pp_year = pay_per_it.ppi_year;

	strcpy(pay_per_it.ppi_code,barg_unit.b_pp_code);
	pay_per_it.ppi_st_date = end_date;
	flg_reset(PAY_PER_ITEM);

	retval = get_n_pp_it(&pay_per_it,BROWSE,1,BACKWARD,e_mesg);
	if(retval < 0 || strcmp(pay_per_it.ppi_code,barg_unit.b_pp_code)!=0){
		fomen("Error Reading Pay Period Item File");
		return(-1);
	}
	seq_over(PAY_PER_ITEM);
	end_pp_numb = pay_per_it.ppi_numb;
	end_pp_year = pay_per_it.ppi_year;

	strcpy(time_his.tmh_numb, emp_rec.em_numb);
	time_his.tmh_year = start_pp_year;
	time_his.tmh_pp = start_pp_numb;
	time_his.tmh_week = 0;

	flg_reset(TIME_HIS);

	for(;;){
		retval = get_n_time_his(&time_his,BROWSE,1,FORWARD,e_mesg);
		if(retval == EFL ||	
		   (strcmp(time_his.tmh_numb, emp_rec.em_numb)) != 0){
			break;
		}
		if(retval < 0) {
			fomen(e_mesg);
			get();
			return(-1);
		}
		if(time_his.tmh_pp > end_pp_numb && 
		   time_his.tmh_year >= end_pp_year)
			break;
		
		strcpy(class_rec.c_code,time_his.tmh_class);
		class_rec.c_date = date1;
		flg_reset(CLASSIFICATION);

		retval = get_n_class(&class_rec,BROWSE,0,BACKWARD,e_mesg);
		if(retval < 0 || 
		   strcmp(class_rec.c_code,time_his.tmh_class) != 0) {
			fomen("Error Reading Class File");
			get();
			return(-1);
		}

	  	strcpy(position.p_code,class_rec.c_pos);
	  	retval = get_position(&position,BROWSE,0,e_mesg);

		if(retval < 0){
			fomen("Error Reading Position File");
			get();
			return(-1);
		}

	 	if((strcmp(position.p_type,"FT")==0)|| 
				 (strcmp(position.p_type,"PT")== 0))
			continue;

		strcpy(sen_par.sn_position,class_rec.c_pos);
		sen_par.sn_eff_date = date1;
		flg_reset(SEN_PAR);

		retval = get_n_sen_par(&sen_par, BROWSE, 0, BACKWARD, e_mesg);
		if(retval<0 || strcmp(sen_par.sn_position,class_rec.c_pos)!=0) {
			fomen("Error Reading Seniority Parameter File");
			return(-1);
		}
		seq_over(SEN_PAR);

		strcpy(pay_per_it.ppi_code,barg_unit.b_pp_code);
		pay_per_it.ppi_year = time_his.tmh_year;
		pay_per_it.ppi_numb = time_his.tmh_pp;

		retval = get_pp_it(&pay_per_it,BROWSE,0,e_mesg);
		if(retval < 0){ 
			fomen("Error Reading Pay Period Item File");
			continue;
		}

		if(time_his.tmh_week == 1)
			time_unit_date = pay_per_it.ppi_st_date;
		else{
			time_unit_date = pay_per_it.ppi_st_date;
			for(i=0;i<7;i++){
				curr_day = (time_unit_date % 100);
				curr_month = ((time_unit_date/100)%100);
				curr_year = time_unit_date / 10000;
				curr_day++;
				leap_year = curr_year % 4;
				if(leap_year == 0 && curr_month == 2){
				  if(curr_day > 29){
					if(curr_month == 12){
						curr_year++;
						curr_month = 1;
					}
					else
						curr_month++;
					curr_day = 1;
				  }
				}
				else {
				  if(curr_day > d_month[curr_month - 1]){
					if(curr_month == 12){
						curr_year++;
						curr_month = 1;
					}
					else
						curr_month++;
					curr_day = 1;
				  }
				}
			      time_unit_date=(curr_year*10000)+(curr_month*100)+
					curr_day;
			}
		}
		for(i=0;i<7;i++){
		    if(time_unit_date > end_date)
			break;
			
		    if(time_his.tmh_units[i]!=0&&time_unit_date>=start_date){
		      curr_day = (time_unit_date % 100);
		      curr_month = ((time_unit_date / 100) % 100);
		      curr_year = time_unit_date / 10000;
		      if((curr_month == start_mth && 
		        curr_year > start_year) 
		        || (curr_month < start_mth)){
		 	  curr_month += nbr_of_mths;
		      }
		      if(sen_par.sn_num_hrs_day == 0){
		          mtd_days[curr_month-start_mth]+=
		     	 	 time_his.tmh_units[i];
			  value = (long)time_his.tmh_units[i];
			  if(value != one){
				 if(curr_month > 12)
					temp_mth = curr_month - 12;
				 else
					temp_mth = curr_month;
				 temp_day = (curr_year*10000)+
					    (temp_mth*100)+
					    (curr_day);
				 if(temp_day < first_day && 
				    temp_day > emp_rec.em_last_roe) {
					first_day = temp_day;
				 }
				 if(temp_day > last_day)
					last_day = temp_day;
		                 items[curr_month-start_mth][curr_day-1]= 
				'#';
			  }
			  else{
				 if(curr_month > 12)
					temp_mth = curr_month - 12;
				 else
					temp_mth = curr_month;
				 temp_day = (curr_year*10000)+
					    (temp_mth*100)+
					    (curr_day);
				 if(temp_day < first_day && 
				    temp_day > emp_rec.em_last_roe) {
					first_day = temp_day;
				 }
				 if(temp_day > last_day)
					last_day = temp_day;
		                 items[curr_month-start_mth][curr_day-1]= 
				'*';
			  }
		      }
		      else{	
			  if(curr_month > 12)
				temp_mth = curr_month - 12;
			  else
				temp_mth = curr_month;
			  temp_day = (curr_year*10000)+
				    (temp_mth*100)+
				    (curr_day);
			  if(temp_day < first_day && 
			     temp_day > emp_rec.em_last_roe) {
				first_day = temp_day;
			  }
			  if(temp_day > last_day)
				last_day = temp_day;
		          mtd_hrs[curr_month-start_mth] += 
		      	  	time_his.tmh_units[i];
		          items[curr_month-start_mth][curr_day-1]= 
				'#';
		      }
		    }
		    if(i != 6){	
		        curr_day = (time_unit_date % 100);
		        curr_month = ((time_unit_date / 100) % 100);
		        curr_year = time_unit_date / 10000;
			curr_day++;
			leap_year = curr_year % 4;
			if(leap_year == 0 && curr_month == 2) {
			  if(curr_day > 29){
				if(curr_month == 12){
					curr_year++;
					curr_month = 1;
				}
				else
					curr_month++;
				curr_day = 1;
			  }
			}
			else {
			  if(curr_day > d_month[curr_month - 1]){
				if(curr_month == 12){
					curr_year++;
					curr_month = 1;
				}
				else
					curr_month++;
				curr_day = 1;
			  }
			}
			time_unit_date=(curr_year*10000)+(curr_month*100)+
					curr_day;
		    }
		}
	}
	seq_over(TIME_HIS);

	strcpy(time.tm_numb, emp_rec.em_numb);
	time.tm_year = start_pp_year;
	time.tm_pp = start_pp_numb;
	time.tm_week = 0;

	flg_reset(TIME);

	for(;;){
		retval = get_n_ptime(&time,BROWSE,3,FORWARD,e_mesg);
		if(retval == EFL ||	
		   (strcmp(time.tm_numb, emp_rec.em_numb)) != 0){
			break;
		}
		if(retval < 0) {
			fomen(e_mesg);
			get();
			return(-1);
		}
		if(time.tm_pp > end_pp_numb && 
		   time.tm_year >= end_pp_year){
			break;
		}
		strcpy(class_rec.c_code,time.tm_class);
		class_rec.c_date = date1;
		flg_reset(CLASSIFICATION);

		retval = get_n_class(&class_rec,BROWSE,0,BACKWARD,e_mesg);
		if(retval < 0 || 
		   strcmp(class_rec.c_code,time.tm_class) != 0) {
			fomen("Error Reading Class File");
			get();
			return(-1);
		}

	  	strcpy(position.p_code,class_rec.c_pos);
	  	retval = get_position(&position,BROWSE,0,e_mesg);

		if(retval < 0){
			fomen("Error Reading Position File");
			get();
			return(-1);
		}

	 	if((strcmp(position.p_type,"FT")==0)|| 
				 (strcmp(position.p_type,"PT")== 0))
			continue;

		strcpy(sen_par.sn_position,class_rec.c_pos);
		sen_par.sn_eff_date = date1;
		flg_reset(SEN_PAR);

		retval = get_n_sen_par(&sen_par, BROWSE, 0, BACKWARD, e_mesg);
		if(retval<0 || strcmp(sen_par.sn_position,class_rec.c_pos)!=0) {
			fomen("Error Reading Seniority Parameter File");
			return(-1);
		}
		seq_over(SEN_PAR);

		strcpy(pay_per_it.ppi_code,barg_unit.b_pp_code);
		pay_per_it.ppi_year = time.tm_year;
		pay_per_it.ppi_numb = time.tm_pp;

		retval = get_pp_it(&pay_per_it,BROWSE,0,e_mesg);
		if(retval < 0){ 
			fomen( "Error Reading Pay Period Item File");
			continue;
		}

		if(time.tm_week == 1)
			time_unit_date = pay_per_it.ppi_st_date;
		else{
			time_unit_date = pay_per_it.ppi_st_date;
			for(i=0;i<7;i++){
				curr_day = (time_unit_date % 100);
				curr_month = ((time_unit_date/100)%100);
				curr_year = time_unit_date / 10000;
				curr_day++;
				leap_year = curr_year % 4;
			    	if(leap_year == 0 && curr_month == 2){	
				  if(curr_day > 29){
					if(curr_month == 12){
						curr_year++;
						curr_month = 1;
					}
					else
						curr_month++;
					curr_day = 1;
				  }
			      	}
				else {
				  if(curr_day > d_month[curr_month-1]){
					if(curr_month == 12){
						curr_year++;
						curr_month = 1;
					}
					else
						curr_month++;
					curr_day = 1;
				  }
				}
			      time_unit_date=(curr_year*10000)+(curr_month*100)+
					curr_day;
			}
		}
		for(i=0;i<7;i++){
		    if(time_unit_date > end_date)
			break;
			
		    if(time.tm_units[i]!=0&&time_unit_date>=start_date){
		      curr_day = (time_unit_date % 100);
		      curr_month = ((time_unit_date / 100) % 100);
		      curr_year = time_unit_date / 10000;
		      if((curr_month == start_mth && 
		        curr_year > start_year) 
		        || (curr_month < start_mth)){
		 	  curr_month += nbr_of_mths;
		      }
		      if(sen_par.sn_num_hrs_day == 0){
		          mtd_days[curr_month-start_mth]+=
		     	 	 time.tm_units[i];
			  value = (long)time.tm_units[i];
			  if(value != one){
			  	 if(curr_month > 12)
					temp_mth = curr_month - 12;
			  	 else
					temp_mth = curr_month;
				 temp_day = (curr_year*10000)+
					    (temp_mth*100)+
					    (curr_day);
				 if(temp_day < first_day && 
				    temp_day > emp_rec.em_last_roe) {
					first_day = temp_day;
				 }
				 if(temp_day > last_day)
					last_day = temp_day;
		                 items[curr_month-start_mth][curr_day-1]= 
				'#';
			  }
			  else{
			  	 if(curr_month > 12)
					temp_mth = curr_month - 12;
			  	 else
					temp_mth = curr_month;
				 temp_day = (curr_year*10000)+
					    (temp_mth*100)+
					    (curr_day);
				 if(temp_day < first_day && 
				    temp_day > emp_rec.em_last_roe) {
					first_day = temp_day;
				 }
				 if(temp_day > last_day)
					last_day = temp_day;
		                 items[curr_month-start_mth][curr_day-1]= 
				'*';
			  }
		      }
		      else{
			  if(curr_month > 12)
				temp_mth = curr_month - 12;
			  else
				temp_mth = curr_month;
			  temp_day = (curr_year*10000)+
				     (temp_mth*100)+
				     (curr_day);
			  if(temp_day < first_day && 
			     temp_day > emp_rec.em_last_roe) {
				first_day = temp_day;
			  }
			  if(temp_day > last_day)
				last_day = temp_day;
		          mtd_hrs[curr_month-start_mth] += 
		      	  	time.tm_units[i];
		          items[curr_month-start_mth][curr_day-1] = 
				'#';
		      }
		    }
		    if(i != 6){	
		        curr_day = (time_unit_date % 100);
		        curr_month = ((time_unit_date / 100) % 100);
		        curr_year = time_unit_date / 10000;
			curr_day++;
			leap_year = curr_year % 4;
			if(leap_year == 0 && curr_month == 2){
			  if(curr_day > 29){
				if(curr_month == 12){
					curr_year++;
					curr_month = 1;
				}
				else
					curr_month++;
				curr_day = 1;
			  }
			}
			else{
			  if(curr_day > d_month[curr_month - 1]){
				if(curr_month == 12){
					curr_year++;
					curr_month = 1;
				}
				else
					curr_month++;
				curr_day = 1;
			  }
			}
			time_unit_date=(curr_year*10000)+(curr_month*100)+
					curr_day;
		    }
		}
	}
	seq_over(TIME);

	month_nbr = 1;

	for(prev_mon=start_mth;month_nbr<=nbr_of_times;prev_mon++) {
		mtd_tot[prev_mon-start_mth] = 
			mtd_hrs[prev_mon-start_mth] +
		        (mtd_days[prev_mon-start_mth] *
			barg_unit.b_stat_hpd);
		month_nbr++;
	}

	month_nbr = 1;

	for(prev_mon=start_mth;month_nbr<=nbr_of_times;prev_mon++) {

		if(prev_mon == start_mth){
			ytd_bal[prev_mon-start_mth] = 
				mtd_tot[prev_mon-start_mth];
		}	
		else {
			ytd_bal[prev_mon-start_mth] = 
				ytd_bal[prev_mon-start_mth-1] + 
				mtd_tot[prev_mon-start_mth]; 
		}
		month_nbr++;
	}

	return(NOERROR);
}
/*----------------------------------------------------------------*/
/* takes the date entered on the screen and get the corresponding */
/* day's name. i.e. MON, TUE, etc.                                */
/*----------------------------------------------------------------*/
static
Get_day()
{
	long	julian, long_date, blank_date, end_date;
	int	remain, curr_month, end_day, end_month, leap_year,
		end_year, blank_day, i, start_year;

	day = start_date % 100;	
	curr_month = (start_date / 100)%100;
	year = start_date / 10000;
	start_year = year;

	blank_date = (year * 10000) + (curr_month * 100) + 1;
	blank_day = blank_date % 100;
	if(day == 1)
		strcpy(items[0],
			"                               ");
	else {
		for(i=blank_day;i<day;i++)
			items[0][i-1] = ' ';
	}
	end_date = date1;
	end_year = end_date / 10000;
	end_day = end_date % 100;
	end_month = (end_date/100)%100;
	leap_year = end_year % 4;
	if(leap_year == 0 && mth == 2) {
		if((++end_day) <= 29){
			end_date++;
			end_day = end_date % 100;
			for(i=end_day;i<=29;i++){
				items[12][i-1] = ' ';
			}
		}
	}
	else {
		if((++end_day) <= d_month[end_month-1]){
			end_date++;
			end_day = end_date % 100;
			for(i=end_day;i<=d_month[end_month-1];i++){
				items[12][i-1] = ' ';
			}
		}
	}
		
	long_date = start_date;

	for( ; ; ){

		julian = days(long_date);
		remain = julian % 7;

		day = long_date % 100;	
		curr_month = (long_date / 100)%100;
		year = long_date / 10000;
		leap_year = year % 4;
		
		if(curr_month == start_mth && year > start_year)
			curr_month += 12;

		if(remain == 6 || remain == 0){
		  if(curr_month >= start_mth)
		    items[curr_month-start_mth][day-1]= WEEKEND; 
		  else{
		    items[12-start_mth+curr_month][day-1]= WEEKEND;
		  }
		}
		curr_month = (long_date / 100)%100;
		if(leap_year == 0 && curr_month == 2){
			if((++day) <= 29)
				long_date++;
			else{
				if(curr_month <= 11)
				  long_date=(year*10000)+((curr_month+1)*100)+1; 
				else
			  	  long_date = ((year+1)*10000) + (1*100) +1; 
			}
		}
		else{	
			if((++day) <= d_month[curr_month-1])
				long_date++;
			else{
				if(curr_month <= 11)
				  long_date=(year*10000)+((curr_month+1)*100)+1;
				else
			          long_date = ((year+1)*10000) + (1*100) +1; 
			}
		}
		if(long_date >= date1)
			break;
	}

	return(NOERROR);
}	/* get_day() */
/*-------------------- E n d   O f   P r o g r a m ---------------------*/
