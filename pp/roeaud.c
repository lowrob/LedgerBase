/******************************************************************************
		Sourcename   : roeaud.c
		System       : 
		Module       :
		Created on   : 08-FEB-93 
		Created  By  : m. galvin 
******************************************************************************
About the file:	
	This program will print the audit listing for casual/substitute 
	employees that status data.

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

#define CONTINUE	10
#define EXIT		12

#ifdef ENGLISH
#define PRINTER 	'P'
#define DISPLAY		'D'
#define FILE_IO		'F'
#define WEEKEND		'W'
#else
#define PRINTER		'I'
#define DISPLAY		'A'
#define FILE_IO		'D'
#define WEEKEND		'W'
#endif

static Time		time;
static Time_his		time_his;
static Pay_per		pay_period;
static Pay_per_it	pay_per_it;
static Class		class_rec;
static Sen_par		sen_par;
static Position		position;
static Emp		emp_rec;
static Emp_sched1	emp_sched1;
static Pa_rec		pa_rec;
static Sch_rec		sch_rec;
static Tmp_sched1	tmp_sched1;
static Barg_unit	barg_unit;
static Pay_param	pay_param;

/*  Data items for storing the key range end values */
static char	sortop1[2];
static char	sortop2[2];
static char	barg1[7];
static char	barg2[7];
static char	posi1[7];
static char	posi2[7];
static char	class1[7];
static char	class2[7];
static char	empl1[13];
static char	empl2[13];
static short	center1;	
static short	center2;	
static long	date1;	

static	char 	sched1_flag[2];

static int	flag;		/* flag to print the employee or not */
static int	PG_SIZE;
static int	retval;
static char 	discfile[15];	/* for storing outputfile name */
static short	pgcnt; 		/* for page count */
static char	resp[2];	/* for storing response */
static short	copies;

static double	mtd_hrs[13], mtd_days[13], mtd_tot[13], ytd_bal[13];
static int	start_mth;
static char	stat_mth[13][4], stat[13][32];

static	int	mode;
static	int	first_time;
static	double	non_sen_days;
static	long	start_mth;
static	long	start_date;
static	int	day, mth, year;

static	short d_month[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

static char month[12][4]={"JAN","FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG",
			"SEP", "OCT", "NOV", "DEC" };

extern char 	e_mesg[200];	/* for storing error messages */

roeaud()
{
	char	tmpindxfile[50];
	char	tnum[5];

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
			strcpy( discfile, "roeaud.dat" );
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

	sortop1[0] = 'A';
	if((retval =  GetSortop1(sortop1))<0 )
		return(retval);

	sortop2[0] = 'B';
	if((retval =  GetSortop2(sortop2))<0 )
		return(retval);

	strcpy( barg1, "     0");
	strcpy( barg2, "ZZZZZZ" );
	retval = GetBargRange( barg1,barg2 );
	if(retval < 0) return(retval);
	if(strcmp( barg1, "     0")==0) 
		barg1[0] = '\0';

	center1 = 0001;
	center2 = 9999;
	retval = GetCenterRange (&center1,&center2);
	if (retval < 0) return(retval);

	strcpy( posi1, "     0");
	strcpy( posi2, "ZZZZZZ" );
	retval = GetPosRange( posi1,posi2 );
	if(retval < 0) return(retval);
	if(strcmp( posi1, "     0")==0)
		posi1[0] = '\0';

	strcpy( class1, "     0");
	strcpy( class2, "ZZZZZZ" );
	retval = GetClassRange( class1,class2 );
	if(retval < 0) return(retval);
	if(strcmp( class1, "     0")==0)
		class1[0] = '\0';

	strcpy( empl1, "           0");
	strcpy( empl2, "ZZZZZZZZZZZZ" );
	retval = GetEmpRange( empl1,empl2 );
	if(retval < 0) return(retval);

	date1 = get_date();
	retval = GetDateRange (&date1);
	if (retval < 0) return(retval);

	if( (retval=Confirm())<=0 )
		return(retval);

	if( opn_prnt( resp, discfile, 1, e_mesg, 1 /* spool */)<0 ){
		return(REPORT_ERR);
	}
	if( *resp=='P' )
		SetCopies( (int)copies );
	pgcnt = 0;		/* Page count is zero */
	LNSZ = 132;		/* line size in no. of chars */
	linecnt = PG_SIZE;	/* Page size in no. of lines */

	if(sortop2[0]=='P' || sortop2[0] == 'B'){ 
		unlink_file(TMPINDX_1);
		STRCPY(tmpindxfile,"emptemp");
		get_tnum(tnum);
		strcat(tmpindxfile,tnum);
		if(CreatTempIndx(tmpindxfile,sortop1,sortop2 )<0){
			sprintf(e_mesg,"Error Creating tmp file : %d",iserror);
			fomer(e_mesg);get();
			return (-1);
		}
	}
	if(sortop2[0] == 'C' || sortop2[0] == 'L'){
		unlink_file(TMP_SCHED1);/*remove temp file*/
		if(BldSchedAdd()<0){
			return (-1);
		}
	}

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
	short 	last_center;
	char	last_position[7], last_barg[7], last_class[7];
	int	first_time = 0;

	/* get the cost center name */
	retval = get_param(&pa_rec,BROWSE,1,e_mesg);
	if (retval < 0){
		fomer(e_mesg);get ();
	}
	/*
		******* Sort by Center *******
	*/
	if(sortop2[0]=='C'||sortop2[0]=='L'){
		tmp_sched1.tm_cost = 0;
		last_center = tmp_sched1.tm_cost;
		tmp_sched1.tm_class[0] = '\0';
		strcpy(last_class,tmp_sched1.tm_class);
		tmp_sched1.tm_sortk_1[0] = '\0';	
		tmp_sched1.tm_sortk_2[0] = '\0';	
		tmp_sched1.tm_sortk_3[0] = '\0';
		flg_reset(TMP_SCHED1);

		for(;;){
			retval=get_n_tmp_sched1(&tmp_sched1,BROWSE,1,FORWARD,e_mesg);
			if(retval == EFL) break;
			if(retval<0){
				fomer(e_mesg);
				continue;
			}

			strcpy(emp_rec.em_numb,tmp_sched1.tm_numb);
			retval=get_employee(&emp_rec,BROWSE,0,e_mesg);
			if(retval<0){
				fomer(e_mesg);
				continue;
			}

			retval = GetStat();
			if(retval < 0)	
				continue;
			
			if(sortop2[0] == 'L'){
				if(last_center != tmp_sched1.tm_cost || 
					first_time == 0 || linecnt > PG_SIZE) { 
					retval = PrntHdg();
					if(retval < 0) 	{
						return(retval);	
					}
					if(retval == EXIT)	break;
					first_time = 1;
				}
			}

			if(sortop2[0] == 'C'){
				if(strcmp(last_class,tmp_sched1.tm_class)!=0 || 
					first_time == 0 || linecnt > PG_SIZE) { 
					retval = PrntHdg();
					if(retval < 0) 	{
						return(retval);	
					}
					if(retval == EXIT)	break;
					first_time = 1;
				}
			}

			if ((retval = PrntRec())<0)
				return(retval);
			if(retval == EXIT)	break;

			strcpy(last_class,tmp_sched1.tm_class);
			last_center = tmp_sched1.tm_cost;

		}/*end of for loop*/

		seq_over(TMP_SCHED1);

	}/*end check which files used*/
	/*
		*********** Sort by Alpha and Bargain or Position *************
	*/
	if(sortop2[0]=='B'||sortop2[0]=='P'){
		emp_rec.em_numb[0] = '\0';
		if(sortop2[0] == 'B') {
			strcpy(emp_rec.em_barg,barg1);
			strcpy(last_barg,emp_rec.em_barg);
			emp_rec.em_pos[0] = '\0';
		}
		if(sortop2[0] == 'P') {
			strcpy(emp_rec.em_pos,posi1);
			strcpy(last_position,emp_rec.em_pos);
			emp_rec.em_barg[0] = '\0';
		}
		emp_rec.em_first_name[0] = '\0';
		emp_rec.em_last_name[0] = '\0';
		flg_reset(TMPINDX_1);

		for (;;){
			retval = get_next((char*)&emp_rec,TMPINDX_1,0,FORWARD,BROWSE,e_mesg);

			if(retval==EFL)		break;
			if(retval<0)		return(-1);
	
			if(strcmp(emp_rec.em_status,"ACT") != 0) 
				continue;

			if(sortop2[0] =='B'){
				if(strcmp(emp_rec.em_barg,barg2) > 0)
					break;
			}
			else {
				if(strcmp(emp_rec.em_barg,barg1) < 0 ||
		   		strcmp(emp_rec.em_barg,barg2) > 0 )
					continue;
			}

			if(sortop2[0] == 'P') {
				if(strcmp(emp_rec.em_pos,posi2) > 0)
					break;
			}
			else {
				if(strcmp(emp_rec.em_pos,posi1) < 0 ||
		   		strcmp(emp_rec.em_pos,posi2) > 0 ) 
					continue;
			}
		
			if(strcmp(emp_rec.em_numb,empl1) < 0 ||
		   	strcmp(emp_rec.em_numb,empl2) > 0 ) 
				continue;

			if((emp_rec.em_cc < center1) ||
		   	  (emp_rec.em_cc > center2 )) 
				continue;

			if(strcmp(emp_rec.em_class,class1) < 0 ||
		   	   strcmp(emp_rec.em_class,class2) > 0 ) 
				continue;

			retval = GetStat();
			if(retval < 0)	
				continue;

			if(sortop2[0] == 'B') {
				if(strcmp(last_barg,emp_rec.em_barg)!=0
				   || first_time == 0 || linecnt > PG_SIZE) {
					retval = PrntHdg();
					if(retval < 0){
						return(retval);
					}
					if(retval == EXIT)	break;
					first_time = 1;
				}
			}
			if(sortop2[0] == 'P') {
				if(strcmp(last_position,emp_rec.em_pos)!=0
				   || first_time == 0 || linecnt > PG_SIZE) { 
					retval = PrntHdg();
					if(retval < 0) 	{
						return(retval);	
					}
					if(retval == EXIT)	break;
					first_time = 1;
				}
			}

			if ((retval = PrntRec())<0)
				return(retval);
			if(retval == EXIT)	break;
			strcpy(last_barg,emp_rec.em_barg);
			strcpy(last_position,emp_rec.em_pos);

		} /*end of endless for loop*/
		seq_over(TMPINDX_1);	
	}
	if(pgcnt){
		if(term<99)
			last_page();
	}
	return(NOERROR);
}
/*--------------------------------------------------------------------
GetStat()  - this routine reads the employee seniority file and   
   		 shows what days the employee has hours for 
--------------------------------------------------------------------*/
int
GetStat()
{
	int 	retval, i,j, curr_month, prev_mon, leap_year ;
	int	nbr_of_mths = 12,nbr_of_times = 13, records_found, curr_day;
	int	month_nbr, start_year, curr_year, first_time, this_year;
	short	start_pp_numb, start_pp_year,end_pp_numb,
		end_pp_year;
	long	end_date, time_unit_date;
	long	one = 1, value;

	for(i=0;i<13;i++) {
		strcpy(stat[i], 
			"                               ");
	}

	curr_month = 1;

	/* initialize screen */

	end_date = date1;

	start_mth = ((date1/100)%100);

	start_date = date1 - 10000;
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
		strcpy(stat_mth[i], month[j]);
		/* Check for leap year	*/
		if((this_year % 4 == 0) && j == 1)
			strncpy(stat[i], 
			"...............................",d_month[j]+1);
		else
			strncpy(stat[i], 
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
		fomer("Error Reading Bargaining Unit File");
		return(-1);
	}
	seq_over(BARG);

	strcpy(pay_per_it.ppi_code,barg_unit.b_pp_code);
	pay_per_it.ppi_st_date = start_date;
	flg_reset(PAY_PER_ITEM);

	retval = get_n_pp_it(&pay_per_it,BROWSE,1,BACKWARD,e_mesg);
	if(retval < 0 || strcmp(pay_per_it.ppi_code,barg_unit.b_pp_code)!=0){
		fomer("Error Reading Pay Period Item File");
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
		fomer("Error Reading Pay Period Item File");
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
			fomer("Error Reading Class File");
			get();
			return(-1);
		}

	  	strcpy(position.p_code,class_rec.c_pos);
	  	retval = get_position(&position,BROWSE,0,e_mesg);

		if(retval < 0){
			fomer("Error Reading Position File");
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
			fomer("Error Reading Seniority Parameter File");
			return(-1);
		}
		seq_over(SEN_PAR);

		strcpy(pay_per_it.ppi_code,barg_unit.b_pp_code);
		pay_per_it.ppi_year = time_his.tmh_year;
		pay_per_it.ppi_numb = time_his.tmh_pp;

		retval = get_pp_it(&pay_per_it,BROWSE,0,e_mesg);
		if(retval < 0){ 
			fomer("Error Reading Pay Period Item File");
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
		                 stat[curr_month-start_mth][curr_day-1]= 
				'#';
			  }
			  else{
		                 stat[curr_month-start_mth][curr_day-1]= 
				'*';
			  }
		      }
		      else{	
		          mtd_hrs[curr_month-start_mth] += 
		      	  	time_his.tmh_units[i];
		          stat[curr_month-start_mth][curr_day-1]= 
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
			fomer("Error Reading Class File");
			get();
			return(-1);
		}

	  	strcpy(position.p_code,class_rec.c_pos);
	  	retval = get_position(&position,BROWSE,0,e_mesg);

		if(retval < 0){
			fomer("Error Reading Position File");
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
			fomer("Error Reading Seniority Parameter File");
			return(-1);
		}
		seq_over(SEN_PAR);

		strcpy(pay_per_it.ppi_code,barg_unit.b_pp_code);
		pay_per_it.ppi_year = time.tm_year;
		pay_per_it.ppi_numb = time.tm_pp;

		retval = get_pp_it(&pay_per_it,BROWSE,0,e_mesg);
		if(retval < 0){ 
			fomer("Error Reading Pay Period Item File"); 
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
		          mtd_days[curr_month-start_mth] +=
		     	 	 time.tm_units[i];
			  value = (long)time.tm_units[i];
			  if(value != one){
		                 stat[curr_month-start_mth][curr_day-1]= 
				'#';
			  }
			  else{
		                 stat[curr_month-start_mth][curr_day-1]= 
				'*';
			  }
		      }
		      else{
		          mtd_hrs[curr_month-start_mth] += 
		      	  	time.tm_units[i];
		          stat[curr_month-start_mth][curr_day-1] = 
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
Get_day()
{
	long	julian, long_date, blank_date, end_date;
	int	remain, day, curr_month, year, end_day, end_month, leap_year,
		end_year, blank_year, blank_month, blank_day, i, start_year;

	day = start_date % 100;	
	curr_month = (start_date / 100)%100;
	year = start_date / 10000;
	start_year = year;

	blank_date = (year * 10000) + (curr_month * 100) + 1;
	blank_day = blank_date % 100;
	if(day == 1)
		strcpy(stat[0],
			"                               ");
	else {
		for(i=blank_day;i<day;i++)
			stat[0][i-1] = ' ';
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
				stat[12][i-1] = ' ';
			}
		}
	}
	else {
		if((++end_day) <= d_month[end_month-1]){
			end_date++;
			end_day = end_date % 100;
			for(i=end_day;i<=d_month[end_month-1];i++){
				stat[12][i-1] = ' ';
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
		    stat[curr_month-start_mth][day-1]= WEEKEND; 
		  else{
		    stat[12-start_mth+curr_month][day-1]= WEEKEND;
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
/******************************************************************************
Prints the headings of the report EMPLOYEE ADDRESS LIST
******************************************************************************/
static
PrntHdg()	/* Print heading  */
{
	short	offset;
	long	sysdt ;
	short	name_size;
	char	txt_line[132];

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
	if(prnt_line() < 0 )	return(REPORT_ERR);

	offset = (LNSZ - strlen(pa_rec.pa_co_name))/2;
	mkln(offset,pa_rec.pa_co_name,strlen(pa_rec.pa_co_name));
	if(prnt_line() < 0 )	return(REPORT_ERR);

#ifdef ENGLISH
	mkln((LNSZ-21)/2,"CASUAL STATUS SUMMARY", 21 );
#else
	mkln((LNSZ-17)/2,"TRANSLATE        ", 17 );
#endif
	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	if(sortop2[0] == 'C') {
		strcpy(class_rec.c_code,tmp_sched1.tm_class);
		class_rec.c_date = get_date();
		flg_reset(CLASSIFICATION); 

		retval = get_n_class(&class_rec,BROWSE,0,BACKWARD,
							e_mesg);
		if(retval < 0) {
			fomen(e_mesg);
			get();
		}
		if(strcmp(tmp_sched1.tm_class,class_rec.c_code)!=0){
			strcpy(class_rec.c_desc,
			   "                              ");
		}     
		mkln(1,"CLASS CODE: ",12); 
		mkln(18,tmp_sched1.tm_class,6);
		mkln(25,class_rec.c_desc,30);
		seq_over(CLASSIFICATION);
	}
	if(sortop2[0] == 'L') {
		sch_rec.sc_numb = tmp_sched1.tm_cost;
		retval = get_sch(&sch_rec,BROWSE,0,e_mesg);
		if (retval < 0){
			fomer(e_mesg);get();
		}
		
		mkln(1,"COST CENTER: ",13);
		tedit( (char *)&tmp_sched1.tm_cost,"__0_",txt_line, R_SHORT ); 
		mkln(14,txt_line,4);
		mkln(19,sch_rec.sc_name,28);
	}
	if(sortop2[0] == 'B' ) {
		strcpy(barg_unit.b_code,emp_rec.em_barg);
		barg_unit.b_date = get_date();
		flg_reset(BARG);

		retval = get_n_barg(&barg_unit,BROWSE,0,BACKWARD,e_mesg);
		if(retval < 0){
			fomer(e_mesg);get();
		}
		if(strcmp(barg_unit.b_code,emp_rec.em_barg)!=0) {
			strcpy(barg_unit.b_name, 
				"                              ");
		}
		mkln(1,"BARGAINING UNIT: ",17); 
		mkln(18,emp_rec.em_barg,6);
		mkln(25,barg_unit.b_name,30);
		seq_over(BARG);
	}
	if(sortop2[0] == 'P' ) {
		strcpy(position.p_code,emp_rec.em_pos);
		retval = get_position(&position,BROWSE,0,e_mesg);
		if(retval < 0){
			fomer(e_mesg);get();
		}
		mkln(1,"POSITION: ",10);
		mkln(11,emp_rec.em_pos,6);
		mkln(18,position.p_desc,30);
	}
	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	return(NOERROR);
}
/*----------------------------------------------------------------------*/
static
PrntRec()
{
	char	txt_line[132];

	mkln(2,"EMPLOYEE #:",11);
	mkln(13,emp_rec.em_numb,12);
	mkln(26,"NAME:",5);
	sprintf(txt_line,"%s, %s %s",
		emp_rec.em_last_name,
		emp_rec.em_first_name,
		emp_rec.em_mid_name);
	mkln(35,txt_line,35);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(26,"MTH           1         2         3            MONTH TO DATE          YTD",73);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(31,"1234567890123456789012345678901      DAYS    HOURS    TOTAL    BALANCE",74);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(26,stat_mth[0],3);
	mkln(31,stat[0],31);
	tedit( (char *)&mtd_days[0],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(66,txt_line,7);
	tedit( (char *)&mtd_hrs[0],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(75,txt_line,7);
	tedit( (char *)&mtd_tot[0],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(84,txt_line,7);
	tedit( (char *)&ytd_bal[0],"_,_0_.__-",txt_line, R_DOUBLE ); 
	mkln(93,txt_line,8);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(26,stat_mth[1],3);
	mkln(31,stat[1],31);
	tedit( (char *)&mtd_days[1],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(66,txt_line,7);
	tedit( (char *)&mtd_hrs[1],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(75,txt_line,7);
	tedit( (char *)&mtd_tot[1],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(84,txt_line,7);
	tedit( (char *)&ytd_bal[1],"_,_0_.__-",txt_line, R_DOUBLE ); 
	mkln(93,txt_line,8);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(26,stat_mth[2],3);
	mkln(31,stat[2],31);
	tedit( (char *)&mtd_days[2],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(66,txt_line,7);
	tedit( (char *)&mtd_hrs[2],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(75,txt_line,7);
	tedit( (char *)&mtd_tot[2],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(84,txt_line,7);
	tedit( (char *)&ytd_bal[2],"_,_0_.__-",txt_line, R_DOUBLE ); 
	mkln(93,txt_line,8);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(26,stat_mth[3],3);
	mkln(31,stat[3],31);
	tedit( (char *)&mtd_days[3],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(66,txt_line,7);
	tedit( (char *)&mtd_hrs[3],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(75,txt_line,7);
	tedit( (char *)&mtd_tot[3],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(84,txt_line,7);
	tedit( (char *)&ytd_bal[3],"_,_0_.__-",txt_line, R_DOUBLE ); 
	mkln(93,txt_line,8);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(26,stat_mth[4],3);
	mkln(31,stat[4],31);
	tedit( (char *)&mtd_days[4],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(66,txt_line,7);
	tedit( (char *)&mtd_hrs[4],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(75,txt_line,7);
	tedit( (char *)&mtd_tot[4],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(84,txt_line,7);
	tedit( (char *)&ytd_bal[4],"_,_0_.__-",txt_line, R_DOUBLE ); 
	mkln(93,txt_line,8);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(26,stat_mth[5],3);
	mkln(31,stat[5],31);
	tedit( (char *)&mtd_days[5],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(66,txt_line,7);
	tedit( (char *)&mtd_hrs[5],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(75,txt_line,7);
	tedit( (char *)&mtd_tot[5],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(84,txt_line,7);
	tedit( (char *)&ytd_bal[5],"_,_0_.__-",txt_line, R_DOUBLE ); 
	mkln(93,txt_line,8);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(26,stat_mth[6],3);
	mkln(31,stat[6],31);
	tedit( (char *)&mtd_days[6],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(66,txt_line,7);
	tedit( (char *)&mtd_hrs[6],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(75,txt_line,7);
	tedit( (char *)&mtd_tot[6],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(84,txt_line,7);
	tedit( (char *)&ytd_bal[6],"_,_0_.__-",txt_line, R_DOUBLE ); 
	mkln(93,txt_line,8);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(26,stat_mth[7],3);
	mkln(31,stat[7],31);
	tedit( (char *)&mtd_days[7],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(66,txt_line,7);
	tedit( (char *)&mtd_hrs[7],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(75,txt_line,7);
	tedit( (char *)&mtd_tot[7],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(84,txt_line,7);
	tedit( (char *)&ytd_bal[7],"_,_0_.__-",txt_line, R_DOUBLE ); 
	mkln(93,txt_line,8);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(26,stat_mth[8],3);
	mkln(31,stat[8],31);
	tedit( (char *)&mtd_days[8],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(66,txt_line,7);
	tedit( (char *)&mtd_hrs[8],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(75,txt_line,7);
	tedit( (char *)&mtd_tot[8],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(84,txt_line,7);
	tedit( (char *)&ytd_bal[8],"_,_0_.__-",txt_line, R_DOUBLE ); 
	mkln(93,txt_line,8);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(26,stat_mth[9],3);
	mkln(31,stat[9],31);
	tedit( (char *)&mtd_days[9],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(66,txt_line,7);
	tedit( (char *)&mtd_hrs[9],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(75,txt_line,7);
	tedit( (char *)&mtd_tot[9],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(84,txt_line,7);
	tedit( (char *)&ytd_bal[9],"_,_0_.__-",txt_line, R_DOUBLE ); 
	mkln(93,txt_line,8);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(26,stat_mth[10],3);
	mkln(31,stat[10],31);
	tedit( (char *)&mtd_days[10],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(66,txt_line,7);
	tedit( (char *)&mtd_hrs[10],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(75,txt_line,7);
	tedit( (char *)&mtd_tot[10],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(84,txt_line,7);
	tedit( (char *)&ytd_bal[10],"_,_0_.__-",txt_line, R_DOUBLE ); 
	mkln(93,txt_line,8);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(26,stat_mth[11],3);
	mkln(31,stat[11],31);
	tedit( (char *)&mtd_days[11],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(66,txt_line,7);
	tedit( (char *)&mtd_hrs[11],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(75,txt_line,7);
	tedit( (char *)&mtd_tot[11],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(84,txt_line,7);
	tedit( (char *)&ytd_bal[11],"_,_0_.__-",txt_line, R_DOUBLE ); 
	mkln(93,txt_line,8);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(26,stat_mth[12],3);
	mkln(31,stat[12],31);
	tedit( (char *)&mtd_days[12],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(66,txt_line,7);
	tedit( (char *)&mtd_hrs[12],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(75,txt_line,7);
	tedit( (char *)&mtd_tot[12],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(84,txt_line,7);
	tedit( (char *)&ytd_bal[12],"_,_0_.__-",txt_line, R_DOUBLE ); 
	mkln(93,txt_line,8);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	if(prnt_line() < 0 )	return(REPORT_ERR);

	return(NOERROR);
}
/*-----------------------------------------------------------------------*/
static
BldSchedAdd ()
{
	int	x;

	/*Get the emp name and dates from the employee file*/

	strcpy(emp_rec.em_numb,empl1);
	flg_reset(EMPLOYEE);

	for (;;){
		retval = get_n_employee(&emp_rec,BROWSE,0,FORWARD,e_mesg);
		if(retval<0)
			break;	

		if(strcmp(emp_rec.em_status,"ACT") != 0) 
			continue;

		if(strcmp(emp_rec.em_barg,barg1) < 0 ||
		  strcmp(emp_rec.em_barg,barg2) > 0 ) 
			continue;

		if(strcmp(emp_rec.em_pos,posi1) < 0 ||
		  strcmp(emp_rec.em_pos,posi2) > 0 ) 
			continue;
		
		if(strcmp(emp_rec.em_numb,empl2) > 0) 
			break;

		/* Check for sched1 records */
		strcpy(emp_sched1.es_numb,emp_rec.em_numb);
		emp_sched1.es_week	= 0;
		emp_sched1.es_fund	= 0;
		emp_sched1.es_class[0] 	= '\0';
		emp_sched1.es_cost	= 0;
		emp_sched1.es_dept[0]	= '\0';
		emp_sched1.es_area[0]	= '\0';
		flg_reset(EMP_SCHED1);
		
		sched1_flag[0] = 'N';
		for (;;){
			retval=get_n_emp_sched1(&emp_sched1,BROWSE,0,FORWARD,e_mesg);
			if (retval < 0)
				break;	

			if(strcmp(emp_rec.em_numb,emp_sched1.es_numb) != 0) {
				sched1_flag[0] = 'N';
				break;
			}
			if(strcmp(emp_sched1.es_class,class1) < 0 ||
	          	 strcmp(emp_sched1.es_class,class2) > 0 ) 
				continue;

			if( (emp_sched1.es_cost < center1) || 
		   	(emp_sched1.es_cost > center2) )
				continue;
		
			sched1_flag[0] = 'Y'; /*sched rec found*/
			if((retval = PutTmpAdd()) <0)
				break;
		}/* end check sched1 rec loop */
		seq_over(EMP_SCHED1);

		if( (emp_rec.em_cc < center1) || 
		 (emp_rec.em_cc > center2) )
			continue;

		if(strcmp(emp_rec.em_class,class1) < 0 ||
	         strcmp(emp_rec.em_class,class2) > 0 ) 
			continue;

		sched1_flag[0] = 'N';
		retval = PutTmpAdd();
		if(retval < 0)
			break;
	}/*end endless loop*/

	seq_over(EMPLOYEE);
	return(NOERROR);
}
/* ------------------------------------------------------------------------*/
static
PutTmpAdd()
{
	char	sort_array[41];
 	
	if(sortop2[0] == 'L'){
		if(sched1_flag[0] == 'Y') {
			sprintf(sort_array,"%-d",emp_sched1.es_cost);
		}
		else {
			sprintf(sort_array,"%-d",emp_rec.em_cc);
		}
		strncpy(tmp_sched1.tm_sortk_1,sort_array,4);
	}
	else{
		if(sched1_flag[0] == 'Y') {
			strncpy(tmp_sched1.tm_sortk_1,emp_sched1.es_class,6);
		}
		else {
			strncpy(tmp_sched1.tm_sortk_1,emp_rec.em_class,6);
		}
	}	
	sprintf(sort_array,"%s%s",
		emp_rec.em_last_name,
		emp_rec.em_first_name);
	if(sortop1[0] == 'A') {
		strcpy(tmp_sched1.tm_sortk_2,sort_array);
		strcpy(tmp_sched1.tm_sortk_3,emp_rec.em_numb);
	}
	else {
		strcpy(tmp_sched1.tm_sortk_3,sort_array);
		strcpy(tmp_sched1.tm_sortk_2,emp_rec.em_numb);
	}

	strncpy(tmp_sched1.tm_numb,emp_rec.em_numb,12);
	if(sched1_flag[0] == 'Y') {
		strncpy(tmp_sched1.tm_class,emp_sched1.es_class,6);
		tmp_sched1.tm_cost = emp_sched1.es_cost;
	}
	else {
		strncpy(tmp_sched1.tm_class,emp_rec.em_class,6);
		tmp_sched1.tm_cost = emp_rec.em_cc;
	}

	tmp_sched1.tm_dept[0] = '\0';
	tmp_sched1.tm_area[0] == '\0';

	retval = put_tmp_sched1(&tmp_sched1,ADD,e_mesg);
	if(retval < 0 && retval != DUPE){
		return(retval);	
	}
	if(retval != DUPE) {
		commit(e_mesg);
	}
	return(NOERROR);
}
/******************   END OF PROGRAM *******************************/
