/******************************************************************************
		Sourcename   : sickbank.c
		Module       : Personnel/Payroll
		Created on   : 93-JUL-22
		Created  By  : Andre Cormier

******************************************************************************
About the file:	

History:
Programmer      Last change on    Details
L.Robichaud	March 17/97	The sick bank is supose to be a calculated
	field.  Add the routine Calcsickbk and reads to att and att hist.
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
#define PRINTER 	'P'
#define DISPLAY		'D'
#define FILE_IO		'F'
#else
#define PRINTER		'I'
#define DISPLAY		'A'
#define FILE_IO		'D'
#endif

static 	Pay_per		pay_per;
static 	Emp		emp_rec;
static	Pa_rec		pa_rec;
static	Sch_rec		sch_rec;
static	Class		class;
static	Sen_par		sen_par;
static	Emp_sen		emp_sen;
static	Pay_param	pay_param;
static	Barg_unit	barg_unit;
static	Position	position;
static	Emp_at_his	att_his;
static	Att		att;

/*  Data items for storing the key range end values */
char	sortop1[2];
char	sortop2[2];
char	barg1[7];
char	barg2[7];
char	posi1[7];
char	posi2[7];
char	class1[7];
char	class2[7];
char	empl1[13];
char	empl2[13];
short	center1;	
short	center2;	

static int	flag;		/* flag to print the employee or not */
static int	PG_SIZE;
static int	retval;
static char 	discfile[15];	/* for storing outputfile name */
static short	pgcnt; 		/* for page count */
static char	resp[2];	/* for storing response */
static short	copies;

static	char	sched1_flag[2];

static	short	last_center;
static	char	last_barg[7];
static  char	last_class[7];
static	char	last_position[7];
static	double	hours;
static	double	salary;
static	double	total;
static	double	code_total;
static	double	grand_total;

double	D_Roundoff();

extern	char 	e_mesg[200];	/* for storing error messages */

sickbank()
{
	char	tmpindxfile[50];
	char	tnum[5];

	code_total = 0;
	grand_total = 0;

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
			strcpy( discfile, "sickbank.dat" );
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

	unlink_file(TMPINDX_1);
	STRCPY(tmpindxfile,"emptemp");
	get_tnum(tnum);
	strcat(tmpindxfile,tnum);
	if(CreatTempIndx(tmpindxfile,sortop1,sortop2 )<0){
		sprintf(e_mesg,"Error Creating tmp file : %d",iserror);
		fomer(e_mesg);get();
		return (-1);
	}

	retval = PrintRpt();
	if(retval < 0)	return(-1);

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
	int	first_time = 0;

	/* get the cost center name */
	retval = get_param(&pa_rec,BROWSE,1,e_mesg);
	if (retval < 0){
		fomer(e_mesg);get ();
	}

	emp_rec.em_numb[0] = '\0';
	if(sortop2[0] == 'C') {
		strcpy(emp_rec.em_class,class1);
		strcpy(last_class,emp_rec.em_class);
		emp_rec.em_class[0] = '\0';
	}
	if(sortop2[0] == 'L') {
		emp_rec.em_cc = center1;
		last_center = emp_rec.em_cc;
		emp_rec.em_cc = 0;
	}
	if(sortop2[0] == 'B') {
		strcpy(emp_rec.em_barg,barg1);
		strcpy(last_barg,emp_rec.em_barg);
		emp_rec.em_barg[0] = '\0';
	}
	if(sortop2[0] == 'P') {
		strcpy(emp_rec.em_pos,posi1);
		strcpy(last_position,emp_rec.em_pos);
		emp_rec.em_pos[0] = '\0';
	}
	emp_rec.em_first_name[0] = '\0';
	emp_rec.em_last_name[0] = '\0';
	flg_reset(TMPINDX_1);

	for (;;){
		retval = get_next((char*)&emp_rec,TMPINDX_1,0,FORWARD,BROWSE,e_mesg);
		if(retval==EFL)		break;
		if(retval<0)		return(-1);

		if(sortop2[0] =='L'){
			if(emp_rec.em_cc > center2)
				break;
		}
		else {
			if(emp_rec.em_cc < center1 ||
	   		emp_rec.em_cc > center2)
				continue;
		}

		if(sortop2[0] =='C'){
			if(strcmp(emp_rec.em_class,class2) > 0)
				break;
		}
		else {
			if(strcmp(emp_rec.em_class,class1) < 0 ||
	   		strcmp(emp_rec.em_class,class2) > 0 )
				continue;
		}

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
	   	strcmp(emp_rec.em_numb,empl2) > 0 ) {
			continue;
		}

		if(sortop2[0] == 'C') {
			if(strcmp(last_class,emp_rec.em_class)!=0 ||
			   first_time == 0 || linecnt > PG_SIZE) {
				if(first_time == 1 && strcmp(last_class,
				   emp_rec.em_class) != 0) {
					retval = PrntCodeTot();
					if(retval < 0)	return(retval);
				}
				retval = PrntHdg();
				if(retval < 0){
					return(retval);
				}
				if(retval == EXIT)	break;
				first_time = 1;
			}
		}
		if(sortop2[0] == 'L') {
			if(last_center != emp_rec.em_cc ||
			   first_time == 0 || linecnt > PG_SIZE) {
				if(first_time == 1 && last_center !=
				   emp_rec.em_cc) {
					retval = PrntCodeTot();
					if(retval < 0)	return(retval);
				}
				retval = PrntHdg();
				if(retval < 0){
					return(retval);
				}
				if(retval == EXIT)	break;
				first_time = 1;
			}
		}
		if(sortop2[0] == 'B') {
			if(strcmp(last_barg,emp_rec.em_barg)!=0 ||
			   first_time == 0 || linecnt > PG_SIZE) {
				if(first_time == 1 && strcmp(last_barg,
				   emp_rec.em_barg) != 0) {
					retval = PrntCodeTot();
					if(retval < 0)	return(retval);
				}
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
				if(first_time == 1 && strcmp(last_position,
				   emp_rec.em_pos) != 0) {
					retval = PrntCodeTot();
					if(retval < 0)	return(retval);
				}
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
		last_center = emp_rec.em_cc;
		strcpy(last_class,emp_rec.em_class);
		strcpy(last_position,emp_rec.em_pos);

	} /*end of endless for loop*/
	seq_over(TMPINDX_1);	

	retval = PrntCodeTot();
	if(retval < 0)	return(retval);

	retval = PrntGrndTot();
	if(retval < 0)	return(retval);

	if(pgcnt){
		if(term<99)
			last_page();
	}
	return(NOERROR);
}

/******************************************************************************
Prints the headings of the report
******************************************************************************/
static
PrntHdg()	/* Print heading  */
{
	short	offset, name_size;
	long	sysdt ;
	char	txt_line[132];

	if( pgcnt && term < 99)   /* new page and display */
		if(next_page()<0) return(EXIT);	
		
	if( pgcnt || term < 99) { /* if not the first page or display */
		if( rite_top()<0 ) return( -1 );	/* form_feed */
	}
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
	mkln((LNSZ-48)/2,"SICK BANK OBLIGATION REPORT (PRE 1985 SICK DAYS)", 48 );
#else
	mkln((LNSZ-48)/2,"TRANSLATE        ", 48 );
#endif
	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	strcpy(barg_unit.b_code,emp_rec.em_barg);
	barg_unit.b_date = get_date();
	flg_reset(BARG);

	retval = get_n_barg(&barg_unit,BROWSE,0,BACKWARD,e_mesg);
	if(retval < 0){
		fomer(e_mesg);get();
		return(UNDEF);
	}

	if(sortop2[0] == 'C') {
		strcpy(class.c_code,emp_rec.em_class);
		class.c_date = get_date();
		flg_reset(CLASSIFICATION); 

		retval = get_n_class(&class,BROWSE,0,BACKWARD,
							e_mesg);
		if(retval < 0) {
			fomen(e_mesg);
			get();
		}
		if(strcmp(emp_rec.em_class,class.c_code)!=0){
			strcpy(class.c_desc,
			   "                              ");
		}     
		mkln(1,"CLASS CODE: ",12); 
		mkln(18,emp_rec.em_class,6);
		mkln(25,class.c_desc,30);
		seq_over(CLASSIFICATION);
	}
	if(sortop2[0] == 'L') {
		sch_rec.sc_numb = emp_rec.em_cc;
		retval = get_sch(&sch_rec,BROWSE,0,e_mesg);
		if (retval < 0){
			fomer(e_mesg);get();
		}
		
		mkln(1,"COST CENTER: ",13);
		tedit( (char *)&emp_rec.em_cc,"__0_",txt_line, R_SHORT ); 
		mkln(14,txt_line,4);
		mkln(19,sch_rec.sc_name,28);
	}
	if(sortop2[0] == 'B' ) {
		if(strcmp(emp_rec.em_barg,barg_unit.b_code)!=0) {
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

	mkln(3,"EMPLOYEE",8);
	mkln(16,"EMPLOYEE",8);
	mkln(48,"SICK",4);
	mkln(62,"HOURS WORKED",12);
	mkln(78,"HOURLY WORKED",13);
	mkln(97,"SICK BANK",9);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(4,"NUMBER",6);
	mkln(18,"NAME",4);
	mkln(41,"BANK BALANCE (DAYS)",19);
	mkln(64,"PER DAY",7);
	mkln(82,"RATE",4);
	mkln(96,"OBLIGATION",10);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	return(NOERROR);
}
/*--------------------------------------------------------------------*/
static
PrntRec()
{
	char	txt_line[132];


	retval = CalcHours();
	if(retval < 0)	return(retval);	

	retval = CalcSalary();
	if(retval < 0)	return(retval);	

	retval = CalcSickbk();
	if(retval < 0) return(retval);

	total = D_Roundoff(emp_rec.em_sic_bk * hours * salary);
	if(total == 0)	return(NOERROR);

	code_total += total;

	mkln(1,emp_rec.em_numb,12);
	sprintf(txt_line,"%s, %s %s",
		emp_rec.em_last_name,
		emp_rec.em_first_name,
		emp_rec.em_mid_name);
	mkln(16,txt_line,35);

	tedit((char*)&emp_rec.em_sic_bk,"___,_0_.__",txt_line,R_DOUBLE);
	mkln(50,txt_line,10);

	tedit((char*)&hours,"___,_0_.__",txt_line,R_DOUBLE);
	mkln(64,txt_line,10);

	tedit((char*)&salary,"___,_0_.__",txt_line,R_DOUBLE);
	mkln(81,txt_line,10);

	tedit((char*)&total,"___,_0_.__",txt_line,R_DOUBLE);
	mkln(96,txt_line,10);

	if(prnt_line() < 0 )	return(REPORT_ERR);


	return(NOERROR);
}

/*--------------------------------------------------------------------*/
static
CalcSickbk()

{

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
			fomen(e_mesg);
			get();
			return(-1);
		}
		strcpy(att.at_code, att_his.eah_code);

		retval = get_att(&att,BROWSE,1,e_mesg);
		if(retval < 0)  {
			fomen(e_mesg);
			get();
			return(retval);
		}

		if(strcmp(att.at_sckbank,"Y")==0){
			emp_rec.em_sic_bk -= att_his.eah_hours / 
						att_his.eah_sen_hours;
		}
	}
	seq_over(EMP_ATT);
	return(NOERROR);
}
/*--------------------------------------------------------------------*/
static
PrntCodeTot()
{
	char	txt_line[132];

	if(prnt_line() < 0 )	return(REPORT_ERR);

	if(sortop2[0] == 'C'){
		mkln(1,"TOTAL FOR CLASS CODE: ",22); 
		mkln(23,last_class,6);
	}

	if(sortop2[0] == 'L'){
		mkln(1,"TOTAL FOR COST CENTER: ",23);
		tedit( (char *)&last_center,"__0_",txt_line, R_SHORT ); 
		mkln(24,txt_line,4);
	}

	if(sortop2[0] == 'B'){
		mkln(1,"TOTAL FOR BARGAINING UNIT: ",27); 
		mkln(28,last_barg,6);
	}
	if(sortop2[0] == 'P'){
		mkln(1,"TOTAL FOR POSITION UNIT: ",25); 
		mkln(26,last_position,6);
	}

	tedit((char*)&code_total,"___,_0_.__",txt_line,R_DOUBLE);
	mkln(110,txt_line,10);

	if(prnt_line() < 0 )	return(REPORT_ERR);

	grand_total += code_total;	
	code_total = 0;	

	return(NOERROR);
}
/*--------------------------------------------------------------------*/
static
PrntGrndTot()
{
	char	txt_line[132];

	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(1,"GRAND TOTAL",11); 

	tedit((char*)&grand_total,"___,_0_.__",txt_line,R_DOUBLE);
	mkln(110,txt_line,10);

	if(prnt_line() < 0 )	return(REPORT_ERR);

	return(NOERROR);
}
/*---------------------------------------------------------------------------*/
static
CalcHours()
{
	int retval;

	strcpy(sen_par.sn_position,emp_rec.em_pos);
	sen_par.sn_eff_date = get_date();

	flg_reset(SEN_PAR);

	retval = get_n_sen_par(&sen_par,BROWSE,0,BACKWARD,e_mesg);
	if(retval == EFL || strcmp(emp_rec.em_pos,sen_par.sn_position) != 0) {
		sprintf(e_mesg,"Seniority Parameter Not Setup, Employee: %s",
			emp_rec.em_numb); 
		fomen(e_mesg);
		return(retval);
	}
	if(retval < 0) {
		fomen(e_mesg) ;
		get();
		return(ERROR) ;
	}
	hours = D_Roundoff(sen_par.sn_num_hrs_day);

	return(NOERROR);
}
/*---------------------------------------------------------------------------*/
static
CalcSalary()
{
	int retval;
	double 	temp_calc = 0;
	double temp_calc2 = 0;
	int	temp_units = 0;
	int j;
	double	one_hundred;

	one_hundred = 100;

	strcpy(barg_unit.b_code,emp_rec.em_barg);
	barg_unit.b_date = get_date();
	flg_reset(BARG);

	retval = get_n_barg(&barg_unit,BROWSE,0,BACKWARD,e_mesg);
	if(retval == EFL ||
		strcmp(barg_unit.b_code, emp_rec.em_barg) != 0){
  		fomer("Bargaining Unit does not Exist");
		return(NOERROR);
	}
	if(retval < 0){
  		fomer(e_mesg);
  		return(ERROR);
	}
	seq_over(BARG);

	if(emp_rec.em_class[0] == '\0') {
		salary = 0;
	}
	else {
		strcpy(class.c_code,emp_rec.em_class);
		class.c_date = get_date();
		flg_reset(CLASSIFICATION);
		retval = get_n_class(&class,BROWSE,0,BACKWARD,e_mesg);
		if((retval < 0 ||
		    strcmp(class.c_code,emp_rec.em_class) != 0)){
			fomer("Classification Code Does Not Exist - Please Re-enter");
			get();
			salary = 0;
			return(-1);
		}
	 	if(class.c_units != 0) {	
 		        salary = D_Roundoff(class.c_yrly_inc / class.c_units); 
		}
		else{
			strcpy(pay_per.pp_code,barg_unit.b_pp_code);
			pay_per.pp_year = 0;
			flg_reset(PAY_PERIOD);

			retval = get_n_pay_per(&pay_per,BROWSE,0,FORWARD,
								e_mesg);
			if(retval < 0 || strcmp(pay_per.pp_code,
					        barg_unit.b_pp_code)!=0) {
				fomer("Pay Period Code Does not Exist");
				get();
			}
			seq_over(PAY_PERIOD);
			if(class.c_yrly_inc == 0){
				salary = 0;
			}
			else{
				 	temp_calc2 = class.c_yrly_inc / 
						(double)pay_per.pp_numb;
					temp_calc = temp_calc2 * emp_rec.em_perc/ one_hundred; 
					temp_calc = D_Roundoff(temp_calc);
		    	}
			salary = (double)temp_calc;
		}
	}
	return(NOERROR);
}
/******************   END OF PROGRAM *******************************/
