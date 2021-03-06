/******************************************************************************
		Sourcename   : attsum.c
		System       : 
		Module       :
		Created on   : 92-OCT-10 
		Created  By  : m. galvin 
******************************************************************************
About the file:	
	This program will print a list of employee addressed.

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

static Emp		emp_rec;
static Emp_sched1	emp_sched1;
static Pa_rec		pa_rec;
static Sch_rec		sch_rec;
static Teach_ass	teach_ass;
static Area_spec	area_spec;
static Tmp_sched1	tmp_sched1;
static Barg_unit	barg_unit;
static Position		position;
static Class		class;
static Emp_at_his	att_his;
static Att		att;
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

static	char	att_mon[12][4], at_disp_code[12][32]; 

static	double	sck_acc[12], vac_acc[12], mtd_att_sic[12], mtd_att_vac[12],
		ytd_att_sic[12], ytd_att_vac[12];
static	int	start_mth;

static	char 	sched1_flag[2];

static int	flag;		/* flag to print the employee or not */
static int	PG_SIZE;
static int	retval;
static char 	discfile[15];	/* for storing outputfile name */
static short	pgcnt; 		/* for page count */
static char	resp[2];	/* for storing response */
static short	copies;

static	short d_month[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

static char month[12][4]={"JAN","FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG",
			"SEP", "OCT", "NOV", "DEC" };

extern char 	e_mesg[200];	/* for storing error messages */

static	int	no_teach_ass;

attsum()
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
			strcpy( discfile, "attsum.dat" );
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
	  		retval=UsrBargVal(BROWSE,emp_rec.em_numb,
						emp_rec.em_barg,0,e_mesg);
	  		if(retval < 0)
				continue;

			retval = GetAtt();
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
	
/**** lou Want to report on all employees, active or not
			if(strcmp(emp_rec.em_status,"ACT") != 0) 
				continue;
***/

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

			retval = GetAtt();
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
 GetAtt()     - this routine reads the attendance information and   
   		 copies it onto the screen
--------------------------------------------------------------------*/
static
GetAtt()
{
	int 	retval, i,j, curr_month, prev_mon;
	int	nbr_of_mths = 12, records_found, curr_day;
	int	month_nbr;
	long	dollars;
	double	total, fraction;

	curr_month = 1;

 	retval = get_pay_param(&pay_param,BROWSE,1,e_mesg);
	if(retval < 0) {
		fomer("Error Found Reading Pay Param File");
		return(retval);
	}

	if(pay_param.pr_st_mth == 1){
		start_mth = ((pay_param.pr_cal_st_dt / 100) % 100);
	}
	if(pay_param.pr_st_mth == 2){
		start_mth = ((pay_param.pr_fisc_st_dt / 100) % 100);
	}
	if(pay_param.pr_st_mth == 3){
		start_mth = ((pay_param.pr_schl_st_dt / 100) % 100);
	}

	for (i=0; i < 12; i++) {
		if((start_mth + i )> 12)
			j = i - start_mth+1 ;
		else
			j = i + start_mth-1;
		strcpy(att_mon[i], month[j]);
		if(((get_date() / 1000) % 4 != 0) && i == 1)
			strncpy(at_disp_code[i], 
			"...............................",d_month[j]+1);
		else
			strncpy(at_disp_code[i], 
			"...............................",d_month[j]);
		sck_acc[i] = emp_rec.em_sck_acc[i];
		vac_acc[i] = emp_rec.em_vac_acc[i];
		mtd_att_sic[i] = 0;	
		mtd_att_vac[i] = 0;
	}
	
	GetDay();

	strcpy(att_his.eah_numb, emp_rec.em_numb);
	if(pay_param.pr_st_mth == 1){
		att_his.eah_date = pay_param.pr_cal_st_dt;  
	}
	if(pay_param.pr_st_mth == 2){
		att_his.eah_date = pay_param.pr_fisc_st_dt;  
	}
	if(pay_param.pr_st_mth == 3){
		att_his.eah_date = pay_param.pr_schl_st_dt;  
	}

	flg_reset(EMP_ATT);

	for(;;){
		retval = get_n_emp_at(&att_his, BROWSE, 0, FORWARD, e_mesg);

		if(retval == EFL ||		
		   (strcmp(att_his.eah_numb, emp_rec.em_numb)) != 0){
			break;
		}
		if(retval < 0) {
			return(-1);
		}
		strcpy(att.at_code, att_his.eah_code);

		retval = get_att(&att,BROWSE,1,e_mesg);
		if(retval < 0)  {
			return(retval);
		}

		curr_day = (att_his.eah_date % 100);
		curr_month = ((att_his.eah_date / 100) % 100);
		if(curr_month < start_mth)
			curr_month += nbr_of_mths;

		if(strcmp(att.at_sick, "Y") == 0){
			mtd_att_sic[curr_month - start_mth] += 
		      		(att_his.eah_hours/att_his.eah_sen_hours);
		}
		if(strcmp(att.at_vac, "Y") == 0){
		        mtd_att_vac[curr_month - (start_mth)] += 
		      		(att_his.eah_hours/att_his.eah_sen_hours);
		}
		if(strcmp(att.at_sckbank, "Y") == 0){
			emp_rec.em_sic_bk -= 
		      		(att_his.eah_hours/att_his.eah_sen_hours);
		}
		if(strcmp(att.at_vacbank, "Y") == 0){
			emp_rec.em_vac_bk -= 
		      		(att_his.eah_hours/att_his.eah_sen_hours);
		}
		at_disp_code[curr_month - start_mth][curr_day-1] = 
			att.at_disp_code[0];
	}
	seq_over(EMP_ATT);

	month_nbr = 1;

	for(prev_mon=start_mth;month_nbr<=nbr_of_mths;prev_mon++) {

		if(prev_mon < start_mth)
			prev_mon += nbr_of_mths;
		
		if(prev_mon == start_mth){
			ytd_att_sic[prev_mon-start_mth] = 
				emp_rec.em_sic_ent - 
				mtd_att_sic[prev_mon-start_mth] +
			        sck_acc[prev_mon-start_mth];
			ytd_att_vac[prev_mon-start_mth] = 
				emp_rec.em_vac_ent -
				mtd_att_vac[prev_mon-start_mth] + 
			        vac_acc[prev_mon-start_mth];
		}	
		else {
			ytd_att_sic[prev_mon-start_mth] = 
				ytd_att_sic[prev_mon-start_mth-1] - 
				mtd_att_sic[prev_mon-start_mth] +
		  		sck_acc[prev_mon-start_mth];
			ytd_att_vac[prev_mon-start_mth] = 
				ytd_att_vac[prev_mon-start_mth-1] - 
				mtd_att_vac[prev_mon-start_mth] + 
		  		vac_acc[prev_mon-start_mth];
		}
		month_nbr++;
	}

	return(NOERROR);

}
/*----------------------------------------------------------------*/
/* takes the date entered on the screen and get the corresponding */
/* day's name. i.e. MON, TUE, etc.                                */
/*----------------------------------------------------------------*/
GetDay()
{
	long	julian;
	int	remain;
	long	long_date, end_date;
	int	day;
	int	curr_month;
	int	year;

	if(pay_param.pr_st_mth == 1){
		long_date = pay_param.pr_cal_st_dt;
		end_date = pay_param.pr_cal_end_dt;
	}
	if(pay_param.pr_st_mth == 2){
		long_date = pay_param.pr_fisc_st_dt;
		end_date = pay_param.pr_fisc_end_dt;
	}
	if(pay_param.pr_st_mth == 3){
		long_date = pay_param.pr_schl_st_dt;
		end_date = pay_param.pr_schl_end_dt;
	}

	for( ; ; ){

		julian = days(long_date);
		remain = julian % 7;

		day = long_date % 100;	
		curr_month = (long_date / 100)%100;
		year = long_date / 10000;

		if(remain == 6 || remain == 0){
		  if(curr_month >= start_mth)
		    at_disp_code[curr_month-start_mth][day-1] = WEEKEND; 
		  else
		    at_disp_code[curr_month+start_mth-2][day-1] = WEEKEND; 
		}
		if((++day) <= d_month[curr_month-1])
			long_date ++;
		else{
			if(curr_month <= 11)
			  long_date = (year*10000) + ((curr_month+1)*100) +1; 
			else
			  long_date = ((year+1)*10000) + (1*100) +1; 
		}
		if(long_date >= end_date)
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
	mkln((LNSZ-27)/2,"EMPLOYEE ATTENDANCE SUMMARY", 27 );
#else
	mkln((LNSZ-17)/2,"TRANSLATE        ", 17 );
#endif
	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	if(sortop2[0] == 'C') {
		strcpy(class.c_code,tmp_sched1.tm_class);
		class.c_date = get_date();
		flg_reset(CLASSIFICATION); 

		retval = get_n_class(&class,BROWSE,0,BACKWARD,
							e_mesg);
		if(retval < 0) {
			fomen(e_mesg);
			get();
		}
		if(strcmp(tmp_sched1.tm_class,class.c_code)!=0){
			strcpy(class.c_desc,
			   "                              ");
		}     
		mkln(1,"CLASS CODE: ",12); 
		mkln(18,tmp_sched1.tm_class,6);
		mkln(25,class.c_desc,30);
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

	mkln(2,"SICK BANK    :",14);
	tedit( (char *)&emp_rec.em_sic_bk,"__0_.__-",txt_line, R_DOUBLE ); 
	mkln(17,txt_line,8);
	mkln(26,"MTH           1         2         3       SICK  VAC    MTH TO DATE     BALANCE",88);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(2,"BEG SICK BAL :",14);
	tedit( (char *)&emp_rec.em_sic_ent,"__0_.__-",txt_line, R_DOUBLE ); 
	mkln(17,txt_line,8);
	mkln(31,"1234567890123456789012345678901        ACCRUAL    SICK    VAC   SICK    VAC",80);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(2,"VACATION BANK:",14);
	tedit( (char *)&emp_rec.em_vac_bk,"__0_.__-",txt_line, R_DOUBLE ); 
	mkln(17,txt_line,8);
	mkln(26,att_mon[0],3);
	mkln(31,at_disp_code[0],31);
	tedit( (char *)&sck_acc[0],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(65,txt_line,6);
	tedit( (char *)&vac_acc[0],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(72,txt_line,7);
	tedit( (char *)&mtd_att_sic[0],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(79,txt_line,7);
	tedit( (char *)&mtd_att_vac[0],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(86,txt_line,7);
	tedit( (char *)&ytd_att_sic[0],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(93,txt_line,7);
	tedit( (char *)&ytd_att_vac[0],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(100,txt_line,7);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(2,"BEG VAC BAL  :",14);
	tedit( (char *)&emp_rec.em_vac_ent,"__0_.__-",txt_line, R_DOUBLE ); 
	mkln(17,txt_line,8);
	mkln(26,att_mon[1],3);
	mkln(31,at_disp_code[1],31);
	tedit( (char *)&sck_acc[1],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(65,txt_line,6);
	tedit( (char *)&vac_acc[1],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(72,txt_line,7);
	tedit( (char *)&mtd_att_sic[1],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(79,txt_line,7);
	tedit( (char *)&mtd_att_vac[1],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(86,txt_line,7);
	tedit( (char *)&ytd_att_sic[1],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(93,txt_line,7);
	tedit( (char *)&ytd_att_vac[1],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(100,txt_line,7);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(26,att_mon[2],3);
	mkln(31,at_disp_code[2],31);
	tedit( (char *)&sck_acc[2],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(65,txt_line,6);
	tedit( (char *)&vac_acc[2],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(72,txt_line,7);
	tedit( (char *)&mtd_att_sic[2],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(79,txt_line,7);
	tedit( (char *)&mtd_att_vac[2],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(86,txt_line,7);
	tedit( (char *)&ytd_att_sic[2],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(93,txt_line,7);
	tedit( (char *)&ytd_att_vac[2],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(100,txt_line,7);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(26,att_mon[3],3);
	mkln(31,at_disp_code[3],31);
	tedit( (char *)&sck_acc[3],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(65,txt_line,6);
	tedit( (char *)&vac_acc[3],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(72,txt_line,7);
	tedit( (char *)&mtd_att_sic[3],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(79,txt_line,7);
	tedit( (char *)&mtd_att_vac[3],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(86,txt_line,7);
	tedit( (char *)&ytd_att_sic[3],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(93,txt_line,7);
	tedit( (char *)&ytd_att_vac[3],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(100,txt_line,7);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(26,att_mon[4],3);
	mkln(31,at_disp_code[4],31);
	tedit( (char *)&sck_acc[4],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(65,txt_line,6);
	tedit( (char *)&vac_acc[4],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(72,txt_line,7);
	tedit( (char *)&mtd_att_sic[4],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(79,txt_line,7);
	tedit( (char *)&mtd_att_vac[4],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(86,txt_line,7);
	tedit( (char *)&ytd_att_sic[4],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(93,txt_line,7);
	tedit( (char *)&ytd_att_vac[4],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(100,txt_line,7);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(26,att_mon[5],3);
	mkln(31,at_disp_code[5],31);
	tedit( (char *)&sck_acc[5],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(65,txt_line,6);
	tedit( (char *)&vac_acc[5],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(72,txt_line,7);
	tedit( (char *)&mtd_att_sic[5],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(79,txt_line,7);
	tedit( (char *)&mtd_att_vac[5],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(86,txt_line,7);
	tedit( (char *)&ytd_att_sic[5],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(93,txt_line,7);
	tedit( (char *)&ytd_att_vac[5],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(100,txt_line,7);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(26,att_mon[6],3);
	mkln(31,at_disp_code[6],31);
	tedit( (char *)&sck_acc[6],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(65,txt_line,6);
	tedit( (char *)&vac_acc[6],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(72,txt_line,7);
	tedit( (char *)&mtd_att_sic[6],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(79,txt_line,7);
	tedit( (char *)&mtd_att_vac[6],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(86,txt_line,7);
	tedit( (char *)&ytd_att_sic[6],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(93,txt_line,7);
	tedit( (char *)&ytd_att_vac[6],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(100,txt_line,7);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(26,att_mon[7],3);
	mkln(31,at_disp_code[7],31);
	tedit( (char *)&sck_acc[7],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(65,txt_line,6);
	tedit( (char *)&vac_acc[7],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(72,txt_line,7);
	tedit( (char *)&mtd_att_sic[7],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(79,txt_line,7);
	tedit( (char *)&mtd_att_vac[7],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(86,txt_line,7);
	tedit( (char *)&ytd_att_sic[7],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(93,txt_line,7);
	tedit( (char *)&ytd_att_vac[7],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(100,txt_line,7);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(26,att_mon[8],3);
	mkln(31,at_disp_code[8],31);
	tedit( (char *)&sck_acc[8],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(65,txt_line,6);
	tedit( (char *)&vac_acc[8],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(72,txt_line,7);
	tedit( (char *)&mtd_att_sic[8],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(79,txt_line,7);
	tedit( (char *)&mtd_att_vac[8],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(86,txt_line,7);
	tedit( (char *)&ytd_att_sic[8],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(93,txt_line,7);
	tedit( (char *)&ytd_att_vac[8],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(100,txt_line,7);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(26,att_mon[9],3);
	mkln(31,at_disp_code[9],31);
	tedit( (char *)&sck_acc[9],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(65,txt_line,6);
	tedit( (char *)&vac_acc[9],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(72,txt_line,7);
	tedit( (char *)&mtd_att_sic[9],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(79,txt_line,7);
	tedit( (char *)&mtd_att_vac[9],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(86,txt_line,7);
	tedit( (char *)&ytd_att_sic[9],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(93,txt_line,7);
	tedit( (char *)&ytd_att_vac[9],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(100,txt_line,7);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(26,att_mon[10],3);
	mkln(31,at_disp_code[10],31);
	tedit( (char *)&sck_acc[10],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(65,txt_line,6);
	tedit( (char *)&vac_acc[10],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(72,txt_line,7);
	tedit( (char *)&mtd_att_sic[10],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(79,txt_line,7);
	tedit( (char *)&mtd_att_vac[10],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(86,txt_line,7);
	tedit( (char *)&ytd_att_sic[10],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(93,txt_line,7);
	tedit( (char *)&ytd_att_vac[10],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(100,txt_line,7);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(26,att_mon[11],3);
	mkln(31,at_disp_code[11],31);
	tedit( (char *)&sck_acc[11],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(65,txt_line,6);
	tedit( (char *)&vac_acc[11],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(72,txt_line,7);
	tedit( (char *)&mtd_att_sic[11],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(79,txt_line,7);
	tedit( (char *)&mtd_att_vac[11],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(86,txt_line,7);
	tedit( (char *)&ytd_att_sic[11],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(93,txt_line,7);
	tedit( (char *)&ytd_att_vac[11],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(100,txt_line,7);
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
	  	retval=UsrBargVal(BROWSE,emp_rec.em_numb,emp_rec.em_barg,0,
									e_mesg);
	  	if(retval < 0)
			continue;

/**** lou Want to report on all employees, active or not
		if(strcmp(emp_rec.em_status,"ACT") != 0) 
			continue;
***/

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
