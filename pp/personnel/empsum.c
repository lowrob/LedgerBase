/******************************************************************************
		Sourcename   : empsum.c
		System       : 
		Module       :
		Created on   : 92-OCT-01 
		Created  By  : m. galvin 
******************************************************************************
About the file:	
	This program will print an employee summary list.

History:
Programmer      Last change on    Details
L.Robichaud	97/05/20	In about April I add code to this program to 
			recognize a S11 and S12 attendance code to e for a half
			day sick and a full day sick accordingly.  This was 
			added for the employees of the Avalon East School Board.
			Today I added the code needed to recognize a V11 and a 
			V12 for vacations in the same manner.
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
#else
#define PRINTER		'I'

#define DISPLAY		'A'
#define FILE_IO		'D'
#endif

static Emp		emp_rec;
static Emp_ins		emp_ins;
static Pa_rec		pa_rec;
static Sch_rec		sch_rec;
static Teach_ass	teach_ass;
static Area_spec	area_spec;
static Tmp_sched1	tmp_sched1;
static Barg_unit	barg_unit;
static Position		position;
static Class		class;
static Term		termination;
static Pay_param	pay_param;
static Teach_qual	teach_qual;
static Reg_pen		reg_pen;
static Dept		dept_rec;
static Area		area_rec;
static Emp_extra	emp_extra;
static Emp_sched1	emp_sched;
static Emp_at_his	att_his;
static Att		att;
static Emp_earn		emp_earn;
static Pay_per		pay_per;
static Emp_sen		emp_sen;
static Sen_par		sen_par;
static Cert		cert;

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

static	char 	sched1_flag[2];

static int	flag;		/* flag to print the employee or not */
static int	PG_SIZE;
static char 	discfile[15];	/* for storing outputfile name */
static short	pgcnt; 		/* for page count */
static char	resp[2];	/* for storing response */
static short	copies;

extern char 	e_mesg[200];	/* for storing error messages */

static	char	sumtype[2];
static	int	no_teach_ass;
static	long	start_mth, start_month, start_date;

static	char	att_mon[12][4];
static	char	sen_mon[12][4];
static	double	mtd_att_sic[12], mtd_att_vac[12], ytd_att_sic[12],
		cas_days[12], sck_acc[12], tot_cas_days[12],
		casual_days[12], permanent_days[12],
		casual_hours[12], employee_salary,
		ytd_att_vac[12], perm_days[12], runits, rinc, hunits,
		hinc, calytd, schytd, non_sen_days[12],
		ytd_gross,ytd_def,ytd_tax,ytd_cpp,ytd_cpp_pen,ytd_uic,
		ytd_uic_earn,ytd_ben,ytd_reg,ytd_pen_rate1,
		ytd_pen_rate2,ytd_pen_rate3,
		init_castot, init_perm_days, init_cas_days,
		bal_sic, bal_vac;
static	short	perm_yrs[12], cas_yrs[12], casual_yrs[12], permanent_yrs[12],
		init_perm_yrs, init_cas_yrs;

short	d_month[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

static char month[12][4]={"JAN","FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG",
			"SEP", "OCT", "NOV", "DEC" };

double	D_Roundoff();

empsum()
{
	char	tmpindxfile[50];
	char	tnum[5];
	int	retval;

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
			strcpy( discfile, "empsum.dat" );
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

	if((retval = DisplayMessage("For X(-Personnel), Y(-Payroll) or Both (X/Y/B)?"))<0) {
		return(retval);
	}

	if((retval = GetResponse(sumtype))<0) {
		return(retval);
	}
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
	int	first_time = 0, retval;

	/* get the cost center name */
	retval = get_param(&pa_rec,BROWSE,1,e_mesg);
	if (retval < 0){
		fomer(e_mesg);get ();
	}
 	retval = get_pay_param(&pay_param,BROWSE,1,e_mesg);
	if(retval < 0) {
		fomer("Error Found Reading Pay Param File");
		return(retval);
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
			retval=get_n_tmp_sched1(&tmp_sched1,BROWSE,1,FORWARD,
									e_mesg);
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

			retval = PrintDemo();
			if(retval < 0)	continue;
			if(retval == EXIT)	break;
			if( term < 99)   /* new page and display */
				if(next_page()<0) return(EXIT);	
			else	
				if( rite_top()<0 ) return( -1 );

			if(sumtype[0] == 'X'){
				retval = PrintPersonnel();
				if(retval < 0)	continue;
				if(retval == EXIT)	break;
				if( term < 99)   /* new page and display */
					if(next_page()<0) return(EXIT);	
				else	
					if( rite_top()<0 ) return( -1 );
			}
			else {
				if(sumtype[0] == 'Y') {
					retval = PrintPayroll();
					if(retval < 0)	continue;
					if(retval == EXIT)	break;
					if( term < 99)
						if(next_page()<0) return(EXIT);	
					else	
						if( rite_top()<0 ) return( -1 );
				}
				else {
					retval = PrintPersonnel();
					if(retval < 0)	continue;
					if(retval == EXIT)	break;
					
					retval = PrintPayroll();
					if(retval < 0)	continue;
					if(retval == EXIT)	break;
					if( term < 99)
						if(next_page()<0) return(EXIT);	
					else	
						if( rite_top()<0 ) return( -1 );
				}
			}
	
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

			retval = PrintDemo();
			if(retval < 0)	continue;
			if(retval == EXIT)	break;

			if(sumtype[0] == 'X'){
				retval = PrintPersonnel();
				if(retval < 0)	continue;
				if(retval == EXIT)	break;
				if( term < 99)
					if(next_page()<0) return(EXIT);	
				else	
					if( rite_top()<0 ) return( -1 );
			}
			else {
				if(sumtype[0] == 'Y') {
					retval = PrintPayroll();
					if(retval < 0)	continue;
					if(retval == EXIT)	break;
					if( term < 99)
						if(next_page()<0) return(EXIT);	
					else	
						if( rite_top()<0 ) return( -1 );
				}
				else {
					retval = PrintPersonnel();
					if(retval < 0)	continue;
					if(retval == EXIT)	break;
					
					retval = PrintPayroll();
					if(retval < 0)	continue;
					if(retval == EXIT)	break;
					if( term < 99)
						if(next_page()<0) return(EXIT);	
					else	
						if( rite_top()<0 ) return( -1 );
				}
			}
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
	int	retval;


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

 	if(sumtype[0] == 'X'){
#ifdef ENGLISH
	mkln((LNSZ-26)/2,"EMPLOYEE PERSONNEL SUMMARY", 26 );
#else
	mkln((LNSZ-17)/2,"TRANSLATE        ", 17 );
#endif
	}

 	if(sumtype[0] == 'Y'){
#ifdef ENGLISH
	mkln((LNSZ-24)/2,"EMPLOYEE PAYROLL SUMMARY", 24 );
#else
	mkln((LNSZ-17)/2,"TRANSLATE        ", 17 );
#endif
	}

 	if(sumtype[0] == 'B'){
#ifdef ENGLISH
	mkln((LNSZ-34)/2,"EMPLOYEE PERSONNEL/PAYROLL SUMMARY", 34 );
#else
	mkln((LNSZ-17)/2,"TRANSLATE        ", 17 );
#endif
	}

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
PrintDemo()
{
	char	txt_line[132];
	int	retval;

	if( pgcnt && term < 99)   /* new page and display */
		if(next_page()<0) return(EXIT);	
		
	mkln(2,"*DEMOGRAPHICS*",14);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	mkln(2,"EMPLOYEE NUMBER:",16);
	mkln(19,emp_rec.em_numb,12);
	mkln(33,"NAME:",5);
	sprintf(txt_line,"%s, %s %s",
		emp_rec.em_last_name,
		emp_rec.em_first_name,
		emp_rec.em_mid_name);
	mkln(39,txt_line,40);
	mkln(88,"STATUS     :",12);
	mkln(101,emp_rec.em_status,3);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(2,"ADDRESS       :",15);
	mkln(19,emp_rec.em_add1,30);
	mkln(51,"SIN         :",13);
	mkln(65,emp_rec.em_sin,3);
	mkln(68,"-",1);
	mkln(69,emp_rec.em_sin+3,3);
	mkln(72,"-",1);
	mkln(73,emp_rec.em_sin+6,3);
	mkln(88,"TELEPHONE  :",12);
	mkln(101,emp_rec.em_phone,3);
	mkln(104,"-",1);
	mkln(105,emp_rec.em_phone+3,3);
	mkln(108,"-",1);
	mkln(109,emp_rec.em_phone+6,4);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(19,emp_rec.em_add2,30);
	mkln(51,"SEX         :",13);
	mkln(65,emp_rec.em_sex,1);
	mkln(88,"MAIDEN NAME:",12);
	mkln(101,emp_rec.em_maid_name,15);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(19,emp_rec.em_add3,30);
	mkln(51,"MARITAL STAT:",13);
	mkln(65,emp_rec.em_mar_st,1);
	mkln(88,"TITLE      :",12);
	mkln(101,emp_rec.em_title,4);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(19,emp_rec.em_add4,30);
	mkln(51,"BIRTH DATE  :",13);
	tedit( (char *)&emp_rec.em_date,"____/__/__",txt_line, R_LONG ); 
	mkln(65,txt_line,10);
	mkln(88,"RELIGION   :",12);
	mkln(101,emp_rec.em_religion,2);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(19,emp_rec.em_pc,30);
	mkln(51,"COST CENTER :",13);
	tedit( (char *)&emp_rec.em_cc,"____",txt_line, R_SHORT ); 
	mkln(65,txt_line,4);
	sch_rec.sc_numb = emp_rec.em_cc;
	retval = get_sch(&sch_rec,BROWSE,0,e_mesg);
	if (retval >= 0){
		mkln(71,sch_rec.sc_name,28);
	}
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(2,"COMMENT        :",16);
	mkln(19,emp_rec.em_com,51);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	return(NOERROR);
}
/*----------------------------------------------------------------------*/
static
PrintPersonnel()
{
	int	retval;

	retval = PrintMisc();
	
	if (linecnt > PG_SIZE)
		if( term < 99)   /* new page and display */
			if(next_page()<0) return(EXIT);	
		else	
			if( rite_top()<0 ) return( -1 );

	retval = PrintAttSen();
	
	if (linecnt > PG_SIZE)
		if( term < 99)   /* new page and display */
			if(next_page()<0) return(EXIT);	
		else	
			if( rite_top()<0 ) return( -1 );

	retval = PrintQual();
	
	if (linecnt > PG_SIZE)
		if( term < 99)   /* new page and display */
			if(next_page()<0) return(EXIT);	
		else	
			if( rite_top()<0 ) return( -1 );

	retval = PrintAssign();
	
	if (linecnt > PG_SIZE)
		if( term < 99)   /* new page and display */
			if(next_page()<0) return(EXIT);	
		else	
			if( rite_top()<0 ) return( -1 );

	return(NOERROR);
}
/*----------------------------------------------------------------------*/
static
PrintMisc()
{
	char	txt_line[132];
	int	retval;

	if( pgcnt && term < 99)   /* new page and display */
		if(next_page()<0) return(EXIT);	
		
	mkln(2,"*MISCELLANEOUS*",15);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(2,"DATE OF HIRE FT:",16);
	tedit( (char *)&emp_rec.em_st_dt_ft,"____/__/__",txt_line, R_LONG ); 
	mkln(19,txt_line,10);
	mkln(44,"CONTINUOUS DATE :",17);
	tedit( (char *)&emp_rec.em_cont_dt,"____/__/__",txt_line, R_LONG ); 
	mkln(62,txt_line,10);
	mkln(88,"TERMINATION REASON :",20);
	mkln(108,emp_rec.em_term,2);
	strcpy(termination.t_code,emp_rec.em_term);
	
	retval = get_pterm(&term,BROWSE,0,e_mesg);
	if(retval >= 0)	
		mkln(112,termination.t_desc,30);	
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(2,"DATE OF HIRE PT:",16);
	tedit( (char *)&emp_rec.em_st_dt_pt,"____/__/__",txt_line, R_LONG ); 
	mkln(19,txt_line,10);
	mkln(44,"APPOINTMENT DATE:",17);
	tedit( (char *)&emp_rec.em_app_dt,"____/__/__",txt_line, R_LONG ); 
	mkln(62,txt_line,10);
	mkln(88,"TERMINATION DATE   :",20);
	tedit( (char *)&emp_rec.em_term_dt,"____/__/__",txt_line, R_LONG ); 
	mkln(108,txt_line,10);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(2,"DATE OF HIRE CA:",16);
	tedit( (char *)&emp_rec.em_st_dt_ca,"____/__/__",txt_line, R_LONG ); 
	mkln(19,txt_line,10);
	mkln(44,"ANNIVERSARY     :",17);
	tedit( (char *)&emp_rec.em_ann,"__",txt_line, R_SHORT ); 
	mkln(62,txt_line,2);
	mkln(88,"INS WEEK LAST 52   :",20);
	tedit( (char *)&emp_rec.em_num_ins_wk,"__",txt_line, R_LONG ); 
	mkln(108,txt_line,2);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(2,"DATE OF HIRE SU:",16);
	tedit( (char *)&emp_rec.em_st_dt_su,"____/__/__",txt_line, R_LONG ); 
	mkln(19,txt_line,10);
	mkln(44,"LANGUAGE PREF   :",17);
	mkln(62,emp_rec.em_lang,1);
	mkln(88,"UNLISTED TEL#      :",20);
	mkln(108,emp_rec.em_un_tel,3);
	mkln(111,"-",1);
	mkln(112,emp_rec.em_un_tel+3,3);
	mkln(115,"-",1);
	mkln(116,emp_rec.em_un_tel+6,4);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(44,"LAST ROE        :",17);
	tedit( (char *)&emp_rec.em_last_roe,"____/__/__",txt_line, R_LONG ); 
	mkln(62,txt_line,10);
	mkln(88,"INSURANCE CLASS    :",20);
	mkln(108,emp_rec.em_ins,1);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);

}
/*---------------------------------------------------------------------*/
static
PrintAttSen()
{
	int	retval;
	char	txt_line[132];

	GetAtt();

	GetSen();

	if( pgcnt && term < 99)   /* new page and display */
		if(next_page()<0) return(EXIT);	
		
	mkln(2,"*ATTENDANCE*",12);
	mkln(66,"*SENIORITY*",11);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	mkln(2,"SICK BANK    :",14);
	tedit( (char *)&emp_rec.em_sic_bk,"__0_.__-",txt_line, R_DOUBLE ); 
	mkln(17,txt_line,8);
	mkln(26,"MTH  SICK   MTH TO DATE     BALANCE",35);
	mkln(66,"SENIORITY PERCENT    :",22);
	tedit( (char *)&emp_rec.em_sen_perc,"_0_.___",txt_line, R_DOUBLE ); 
	mkln(89,txt_line,7);
	mkln(106,"PERM TOTAL CASUAL TOTAL",23);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(2,"CURR SICK BAL:",14);
	tedit( (char *)&bal_sic,"__0_.__-",txt_line, R_DOUBLE ); 
	mkln(17,txt_line,8);
	mkln(31,"ACC'D  SICK    VAC   SICK    VAC",32);
	mkln(66,"YEARS OUT OF DISTRICT:",22);
	tedit( (char *)&emp_rec.em_yrs_out_dist,"0_",txt_line, R_SHORT ); 
	mkln(89,txt_line,2);
	mkln(99,"MTH    YRS  DAYS   YRS  DAYS",28);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(2,"BEG SICK BAL :",14);
	tedit( (char *)&emp_rec.em_sic_ent,"__0_.__-",txt_line, R_DOUBLE ); 
	mkln(17,txt_line,8);
	mkln(26,att_mon[0],3);
	tedit( (char *)&sck_acc[0],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(31,txt_line,6);
	tedit( (char *)&mtd_att_sic[0],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(38,txt_line,7);
	tedit( (char *)&mtd_att_vac[0],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(45,txt_line,7);
	tedit( (char *)&ytd_att_sic[0],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(52,txt_line,7);
	tedit( (char *)&ytd_att_vac[0],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(59,txt_line,7);
	mkln(66,"DAYS OUT OF DISTRICT :",22);
	tedit( (char *)&emp_rec.em_mth_sic,"_0_.___",txt_line, R_DOUBLE ); 
	mkln(89,txt_line,7);
	mkln(99,"INIT:",5);
	tedit( (char *)&emp_rec.em_per_tot_yrs,"0_",txt_line, R_SHORT ); 
	mkln(106,txt_line,2);
	tedit( (char *)&emp_rec.em_per_tot_days,"_0_.__",txt_line, R_DOUBLE ); 
	mkln(110,txt_line,6);
	tedit( (char *)&emp_rec.em_cas_tot_yrs,"0_",txt_line, R_SHORT ); 
	mkln(118,txt_line,2);
	tedit( (char *)&emp_rec.em_cas_tot_days,"_0_.__",txt_line, R_DOUBLE ); 
	mkln(122,txt_line,6);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(2,"VACATION BANK:",14);
	tedit( (char *)&emp_rec.em_vac_bk,"__0_.__-",txt_line, R_DOUBLE ); 
	mkln(17,txt_line,8);
	mkln(26,att_mon[1],3);
	tedit( (char *)&sck_acc[1],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(31,txt_line,6);
	tedit( (char *)&mtd_att_sic[1],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(38,txt_line,7);
	tedit( (char *)&mtd_att_vac[1],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(45,txt_line,7);
	tedit( (char *)&ytd_att_sic[1],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(52,txt_line,7);
	tedit( (char *)&ytd_att_vac[1],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(59,txt_line,7);
	mkln(66,"YEARS OUT OF PROV    :",22);
	tedit( (char *)&emp_rec.em_yrs_out_prov,"0_",txt_line, R_SHORT ); 
	mkln(89,txt_line,2);
	mkln(99,sen_mon[0],5);
	tedit( (char *)&perm_yrs[0],"0_",txt_line, R_SHORT ); 
	mkln(106,txt_line,2);
	tedit( (char *)&perm_days[0],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(110,txt_line,6);
	tedit( (char *)&cas_yrs[0],"0_",txt_line, R_SHORT ); 
	mkln(118,txt_line,2);
	tedit( (char *)&cas_days[0],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(122,txt_line,6);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(2,"CURR VAC BAL :",14);
	tedit( (char *)&bal_vac,"__0_.__-",txt_line, R_DOUBLE ); 
	mkln(17,txt_line,8);
	mkln(26,att_mon[2],3);
	tedit( (char *)&sck_acc[2],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(31,txt_line,6);
	tedit( (char *)&mtd_att_sic[2],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(38,txt_line,7);
	tedit( (char *)&mtd_att_vac[2],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(45,txt_line,7);
	tedit( (char *)&ytd_att_sic[2],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(52,txt_line,7);
	tedit( (char *)&ytd_att_vac[2],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(59,txt_line,7);
	mkln(66,"DAYS OUT OF PROV     :",22);
	tedit( (char *)&emp_rec.em_mth_vac,"_0_.___",txt_line, R_DOUBLE ); 
	mkln(89,txt_line,7);
	mkln(99,sen_mon[1],5);
	tedit( (char *)&perm_yrs[1],"0_",txt_line, R_SHORT ); 
	mkln(106,txt_line,2);
	tedit( (char *)&perm_days[1],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(110,txt_line,6);
	tedit( (char *)&cas_yrs[1],"0_",txt_line, R_SHORT ); 
	mkln(118,txt_line,2);
	tedit( (char *)&cas_days[1],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(122,txt_line,6);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(2,"BEG VAC BAL  :",14);
	tedit( (char *)&emp_rec.em_vac_ent,"__0_.__-",txt_line, R_DOUBLE ); 
	mkln(17,txt_line,8);
	mkln(26,att_mon[3],3);
	tedit( (char *)&sck_acc[3],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(31,txt_line,6);
	tedit( (char *)&mtd_att_sic[3],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(38,txt_line,7);
	tedit( (char *)&mtd_att_vac[3],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(45,txt_line,7);
	tedit( (char *)&ytd_att_sic[3],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(52,txt_line,7);
	tedit( (char *)&ytd_att_vac[3],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(59,txt_line,7);
	mkln(99,sen_mon[2],5);
	tedit( (char *)&perm_yrs[2],"0_",txt_line, R_SHORT ); 
	mkln(106,txt_line,2);
	tedit( (char *)&perm_days[2],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(110,txt_line,6);
	tedit( (char *)&cas_yrs[2],"0_",txt_line, R_SHORT ); 
	mkln(118,txt_line,2);
	tedit( (char *)&cas_days[2],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(122,txt_line,6);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(26,att_mon[4],3);
	tedit( (char *)&sck_acc[4],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(31,txt_line,6);
	tedit( (char *)&mtd_att_sic[4],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(38,txt_line,7);
	tedit( (char *)&mtd_att_vac[4],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(45,txt_line,7);
	tedit( (char *)&ytd_att_sic[4],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(52,txt_line,7);
	tedit( (char *)&ytd_att_vac[4],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(59,txt_line,7);
	mkln(99,sen_mon[3],5);
	tedit( (char *)&perm_yrs[3],"0_",txt_line, R_SHORT ); 
	mkln(106,txt_line,2);
	tedit( (char *)&perm_days[3],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(110,txt_line,6);
	tedit( (char *)&cas_yrs[3],"0_",txt_line, R_SHORT ); 
	mkln(118,txt_line,2);
	tedit( (char *)&cas_days[3],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(122,txt_line,6);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(26,att_mon[5],3);
	tedit( (char *)&sck_acc[5],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(31,txt_line,6);
	tedit( (char *)&mtd_att_sic[5],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(38,txt_line,7);
	tedit( (char *)&mtd_att_vac[5],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(45,txt_line,7);
	tedit( (char *)&ytd_att_sic[5],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(52,txt_line,7);
	tedit( (char *)&ytd_att_vac[5],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(59,txt_line,7);
	mkln(99,sen_mon[4],5);
	tedit( (char *)&perm_yrs[4],"0_",txt_line, R_SHORT ); 
	mkln(106,txt_line,2);
	tedit( (char *)&perm_days[4],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(110,txt_line,6);
	tedit( (char *)&cas_yrs[4],"0_",txt_line, R_SHORT ); 
	mkln(118,txt_line,2);
	tedit( (char *)&cas_days[4],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(122,txt_line,6);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(26,att_mon[6],3);
	tedit( (char *)&sck_acc[6],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(31,txt_line,6);
	tedit( (char *)&mtd_att_sic[6],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(38,txt_line,7);
	tedit( (char *)&mtd_att_vac[6],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(45,txt_line,7);
	tedit( (char *)&ytd_att_sic[6],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(52,txt_line,7);
	tedit( (char *)&ytd_att_vac[6],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(59,txt_line,7);
	mkln(99,sen_mon[5],5);
	tedit( (char *)&perm_yrs[5],"0_",txt_line, R_SHORT ); 
	mkln(106,txt_line,2);
	tedit( (char *)&perm_days[5],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(110,txt_line,6);
	tedit( (char *)&cas_yrs[5],"0_",txt_line, R_SHORT ); 
	mkln(118,txt_line,2);
	tedit( (char *)&cas_days[5],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(122,txt_line,6);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(26,att_mon[7],3);
	tedit( (char *)&sck_acc[7],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(31,txt_line,6);
	tedit( (char *)&mtd_att_sic[7],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(38,txt_line,7);
	tedit( (char *)&mtd_att_vac[7],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(45,txt_line,7);
	tedit( (char *)&ytd_att_sic[7],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(52,txt_line,7);
	tedit( (char *)&ytd_att_vac[7],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(59,txt_line,7);
	mkln(99,sen_mon[6],5);
	tedit( (char *)&perm_yrs[6],"0_",txt_line, R_SHORT ); 
	mkln(106,txt_line,2);
	tedit( (char *)&perm_days[6],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(110,txt_line,6);
	tedit( (char *)&cas_yrs[6],"0_",txt_line, R_SHORT ); 
	mkln(118,txt_line,2);
	tedit( (char *)&cas_days[6],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(122,txt_line,6);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(26,att_mon[8],3);
	tedit( (char *)&sck_acc[8],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(31,txt_line,6);
	tedit( (char *)&mtd_att_sic[8],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(38,txt_line,7);
	tedit( (char *)&mtd_att_vac[8],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(45,txt_line,7);
	tedit( (char *)&ytd_att_sic[8],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(52,txt_line,7);
	tedit( (char *)&ytd_att_vac[8],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(59,txt_line,7);
	mkln(99,sen_mon[7],5);
	tedit( (char *)&perm_yrs[7],"0_",txt_line, R_SHORT ); 
	mkln(106,txt_line,2);
	tedit( (char *)&perm_days[7],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(110,txt_line,6);
	tedit( (char *)&cas_yrs[7],"0_",txt_line, R_SHORT ); 
	mkln(118,txt_line,2);
	tedit( (char *)&cas_days[7],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(122,txt_line,6);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(26,att_mon[9],3);
	tedit( (char *)&sck_acc[9],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(31,txt_line,6);
	tedit( (char *)&mtd_att_sic[9],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(38,txt_line,7);
	tedit( (char *)&mtd_att_vac[9],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(45,txt_line,7);
	tedit( (char *)&ytd_att_sic[9],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(52,txt_line,7);
	tedit( (char *)&ytd_att_vac[9],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(59,txt_line,7);
	mkln(99,sen_mon[8],5);
	tedit( (char *)&perm_yrs[8],"0_",txt_line, R_SHORT ); 
	mkln(106,txt_line,2);
	tedit( (char *)&perm_days[8],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(110,txt_line,6);
	tedit( (char *)&cas_yrs[8],"0_",txt_line, R_SHORT ); 
	mkln(118,txt_line,2);
	tedit( (char *)&cas_days[8],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(122,txt_line,6);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(26,att_mon[10],3);
	tedit( (char *)&sck_acc[10],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(31,txt_line,6);
	tedit( (char *)&mtd_att_sic[10],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(38,txt_line,7);
	tedit( (char *)&mtd_att_vac[10],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(45,txt_line,7);
	tedit( (char *)&ytd_att_sic[10],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(52,txt_line,7);
	tedit( (char *)&ytd_att_vac[10],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(59,txt_line,7);
	mkln(99,sen_mon[9],5);
	tedit( (char *)&perm_yrs[9],"0_",txt_line, R_SHORT ); 
	mkln(106,txt_line,2);
	tedit( (char *)&perm_days[9],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(110,txt_line,6);
	tedit( (char *)&cas_yrs[9],"0_",txt_line, R_SHORT ); 
	mkln(118,txt_line,2);
	tedit( (char *)&cas_days[9],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(122,txt_line,6);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(26,att_mon[11],3);
	tedit( (char *)&sck_acc[11],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(31,txt_line,6);
	tedit( (char *)&mtd_att_sic[11],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(38,txt_line,7);
	tedit( (char *)&mtd_att_vac[11],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(45,txt_line,7);
	tedit( (char *)&ytd_att_sic[11],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(52,txt_line,7);
	tedit( (char *)&ytd_att_vac[11],"_0_.__-",txt_line, R_DOUBLE ); 
	mkln(59,txt_line,7);
	mkln(99,sen_mon[10],5);
	tedit( (char *)&perm_yrs[10],"0_",txt_line, R_SHORT ); 
	mkln(106,txt_line,2);
	tedit( (char *)&perm_days[10],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(110,txt_line,6);
	tedit( (char *)&cas_yrs[10],"0_",txt_line, R_SHORT ); 
	mkln(118,txt_line,2);
	tedit( (char *)&cas_days[10],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(122,txt_line,6);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(99,sen_mon[11],5);
	tedit( (char *)&perm_yrs[11],"0_",txt_line, R_SHORT ); 
	mkln(106,txt_line,2);
	tedit( (char *)&perm_days[11],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(110,txt_line,6);
	tedit( (char *)&cas_yrs[11],"0_",txt_line, R_SHORT ); 
	mkln(118,txt_line,2);
	tedit( (char *)&cas_days[11],"_0_.__",txt_line, R_DOUBLE ); 
	mkln(122,txt_line,6);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	return(NOERROR);
}
/*-----------------------------------------------------------------------*/
static
PrintQual()
{
	int	retval;
	char	txt_line[132];
	
	if( pgcnt && term < 99)   /* new page and display */
		if(next_page()<0) return(EXIT);	
		
	mkln(2,"*QUALIFICATIONS*",16);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(2,"CERTIFICATION:",14);
	mkln(17,emp_rec.em_cert,6);
	mkln(25,"LEVEL      :",12);
	/* ANDRE */
	tedit( (char *)&emp_rec.em_level,"_0",txt_line, R_SHORT ); 
	mkln(38,txt_line,2);
	mkln(74,"INSTITUTION:",12);
	mkln(87,emp_rec.em_inst[0],15);
	mkln(104,"PROGRAM:",8);
	mkln(113,emp_rec.em_prog[0],15);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(2,"YEARS EXP    :",14);
	tedit( (char *)&emp_rec.em_yrs_exp,"_0_.__",txt_line, R_SHORT ); 
	mkln(17,txt_line,6);
	mkln(25,"SCHOOL PREF:",12);
	tedit( (char *)&emp_rec.em_pref_cc,"____",txt_line, R_SHORT ); 
	mkln(38,txt_line,4);
	if(emp_rec.em_pref_cc != 0){
		sch_rec.sc_numb = emp_rec.em_pref_cc;
		retval = get_sch(&sch_rec,BROWSE,0,e_mesg);
		if (retval >= 0){
			mkln(43,sch_rec.sc_name,28);
		}
	}
	mkln(74,"INSTITUTION:",12);
	mkln(87,emp_rec.em_inst[1],15);
	mkln(104,"PROGRAM:",8);
	mkln(113,emp_rec.em_prog[1],15);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(74,"INSTITUTION:",12);
	mkln(87,emp_rec.em_inst[2],15);
	mkln(104,"PROGRAM:",8);
	mkln(113,emp_rec.em_prog[2],15);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(74,"INSTITUTION:",12);
	mkln(87,emp_rec.em_inst[3],15);
	mkln(104,"PROGRAM:",8);
	mkln(113,emp_rec.em_prog[3],15);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(74,"INSTITUTION:",12);
	mkln(87,emp_rec.em_inst[4],15);
	mkln(104,"PROGRAM:",8);
	mkln(113,emp_rec.em_prog[4],15);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(2,"AREA OF SPECIALIZATION",22);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	mkln(2,"CODE",4);
	mkln(17,"DESCRIPTION",11);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	strcpy(teach_qual.tq_numb,emp_rec.em_numb);	
	teach_qual.tq_code[0] = '\0';
	flg_reset(TEACH_QUAL);
	
	for(;;) {
		retval = get_n_teach_qual(&teach_qual,BROWSE,0,FORWARD,e_mesg);
		if(retval < 0)
			break;
		if(strcmp(teach_qual.tq_numb,emp_rec.em_numb) != 0)
			break;
		strcpy(area_spec.ar_code,teach_qual.tq_code);
		retval = get_area_spec(&area_spec,BROWSE,0,e_mesg);
		if(retval < 0)	break;
		mkln(2,area_spec.ar_code,6);
		mkln(10,area_spec.ar_desc,30);
		if(prnt_line() < 0 )	return(REPORT_ERR);
	}
	seq_over(TEACH_QUAL);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	return(NOERROR);
}
/*--------------------------------------------------------------------------*/
static
PrintAssign()
{
	int	retval;
	char	txt_line[132];

	if( pgcnt && term < 99)   /* new page and display */
		if(next_page()<0) return(EXIT);	
		
	mkln(2,"*ASSIGNMENTS*",13);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	strcpy(teach_ass.tc_numb,emp_rec.em_numb);
	teach_ass.tc_cost = 0;
	teach_ass.tc_ar_sp[0] = '\0';
	flg_reset(TEACH_ASS);

	for(;;){
		retval = get_n_teach_ass(&teach_ass,BROWSE,0,FORWARD,e_mesg);
		if(retval < 0)	break;
		if(strcmp(emp_rec.em_numb,teach_ass.tc_numb) != 0)
			break;
		mkln(2,"COST CENTER:",12);
		sch_rec.sc_numb = teach_ass.tc_cost;
		retval = get_sch(&sch_rec,BROWSE,0,e_mesg);
		if (retval >= 0){
			mkln(43,sch_rec.sc_name,28);
		}
		mkln(56,"AREA OF SPECIALIZATION:",23);
		mkln(80,teach_ass.tc_ar_sp,6);
		strcpy(area_spec.ar_code,teach_ass.tc_ar_sp);
		retval = get_area_spec(&area_spec,BROWSE,0,e_mesg);
		if(retval >= 0)	
			mkln(88,area_spec.ar_desc,30);	
		if(prnt_line() < 0 )	return(REPORT_ERR);

		mkln(2,"GRADE      :",12);
		tedit( (char *)&teach_ass.tc_grade,"__",txt_line, R_SHORT ); 
		mkln(15,txt_line,2);
		mkln(44,"PERCENTAGE:",11);
		tedit( (char *)&teach_ass.tc_perc,"_0_.__",txt_line, R_DOUBLE); 
		mkln(56,txt_line,6);
		mkln(88,"COURSE:",7);
		mkln(96,teach_ass.tc_crs,6);
		if(prnt_line() < 0 )	return(REPORT_ERR);
		
		mkln(2,"SEMESTER   :",12);
		tedit( (char *)&teach_ass.tc_sem,"_0",txt_line, R_DOUBLE); 
		mkln(15,txt_line,2);
		mkln(44,"SECTION   :",11);
		tedit( (char *)&teach_ass.tc_sec,"_0",txt_line, R_DOUBLE); 
		mkln(56,txt_line,2);
		mkln(88,"ROOM  :",7);
		tedit( (char *)&teach_ass.tc_room,"____0",txt_line, R_DOUBLE); 
		mkln(96,txt_line,5);
		if(prnt_line() < 0 )	return(REPORT_ERR);
		
		mkln(2,"NO STUDENTS:",12);
		tedit( (char *)&teach_ass.tc_load,"_0",txt_line, R_DOUBLE); 
		mkln(15,txt_line,2);
		if(prnt_line() < 0 )	return(REPORT_ERR);
		if(prnt_line() < 0 )	return(REPORT_ERR);
	}
	seq_over(TEACH_ASS);

	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	return(NOERROR);
}
/*----------------------------------------------------------------------*/
static
PrintPayroll()
{
	int	retval;

	retval = PrintEmp();

	if (linecnt > PG_SIZE)
		if( term < 99)   /* new page and display */
			if(next_page()<0) return(EXIT);	
		else	
			if( rite_top()<0 ) return( -1 );

	retval = PrintRespDed();

	if (linecnt > PG_SIZE)
		if( term < 99)   /* new page and display */
			if(next_page()<0) return(EXIT);	
		else	
			if( rite_top()<0 ) return( -1 );

	retval = PrintEarn();

	if (linecnt > PG_SIZE)
		if( term < 99)   /* new page and display */
			if(next_page()<0) return(EXIT);	
		else	
			if( rite_top()<0 ) return( -1 );
	
	return(NOERROR);

}
/*----------------------------------------------------------------------*/
static
PrintEmp()
{
	char	txt_line[132];
	int	retval;
	double	temp_calc;

	if( pgcnt && term < 99)   /* new page and display */
		if(next_page()<0) return(EXIT);	
		
	mkln(2,"*EMPLOYMENT*",12);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(2,"BARGAINING UNIT:",16);
	mkln(19,emp_rec.em_barg,6);
	if(emp_rec.em_barg[0] != '\0') {
		strcpy(barg_unit.b_code,emp_rec.em_barg);
		barg_unit.b_date = get_date() ;
		flg_reset(BARG);

		retval = get_n_barg(&barg_unit,BROWSE,0,BACKWARD,e_mesg);
		if(retval >= 0 && strcmp(barg_unit.b_code,emp_rec.em_barg)==0) 
			mkln(27,barg_unit.b_name,30);
	}

	mkln(59,"CLASS CODE   :",14);
	mkln(74,emp_rec.em_class,6);
	if(emp_rec.em_class[0] != '\0'){
		strcpy(class.c_code,emp_rec.em_class);
		class.c_date = get_date();
		flg_reset(CLASSIFICATION);
		retval = get_n_class(&class,BROWSE,0,BACKWARD,e_mesg);
		if(retval >= 0 && strcmp(emp_rec.em_class,class.c_code)==0) 
			mkln(82,class.c_desc,30);
	}
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(2,"POSITION       :",16);
	mkln(19,emp_rec.em_pos,6);
	if(emp_rec.em_pos[0] != '\0'){
		strcpy(position.p_code,emp_rec.em_pos);
		retval = get_position(&position,BROWSE,0,e_mesg);
		if(retval >= 0)
			mkln(27,position.p_desc,30);
	}
	mkln(59,"CERTIFICATION:",14);
	mkln(74,emp_rec.em_cert,6);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(2,"PERCENTAGE     :",16);
	tedit( (char *)&emp_rec.em_perc,"_0_.___",txt_line, R_DOUBLE); 
	mkln(19,txt_line,7);
	mkln(59,"LEVEL        :",14);
	tedit( (char *)&emp_rec.em_level,"_0",txt_line, R_SHORT); 
	mkln(74,txt_line,2);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	strcpy(emp_sched.es_numb,emp_rec.em_numb);
	emp_sched.es_week = 0;
	emp_sched.es_class[0] = '\0';
	emp_sched.es_fund = 0;
	flg_reset(EMP_SCHED1);

	for( ; ; ) {
		retval = get_n_emp_sched1(&emp_sched,BROWSE,0,FORWARD,e_mesg);
		if(retval < 0 || strcmp(emp_sched.es_numb,emp_rec.em_numb)!=0) 
			break;

		mkln(2,"WEEK:",5);
		tedit( (char *)&emp_sched.es_week,"_0",txt_line,R_DOUBLE); 
		mkln(8,txt_line,2);
		mkln(14,"CLASS      :",12);
		mkln(27,emp_sched.es_class,6);
		if(emp_sched.es_class[0] != '\0'){
			strcpy(class.c_code,emp_sched.es_class);
			class.c_date = get_date();
			flg_reset(CLASSIFICATION);

			retval = get_n_class(&class,BROWSE,0,BACKWARD,e_mesg);
			if(retval >= 0 && 
			   strcmp(emp_sched.es_class,class.c_code)==0) {
				mkln(82,class.c_desc,30);
			}
		}
		mkln(68,"DEPT:",5);
		mkln(74,emp_sched.es_dept,6);
		if(emp_sched.es_dept[0] != '\0') {
			strcpy(dept_rec.d_code,emp_sched.es_dept);
			retval = get_dept(&dept_rec,BROWSE,0,e_mesg);
			if(retval > 0)
				mkln(82,dept_rec.d_desc,30);
		}
		if(prnt_line() < 0 )	return(REPORT_ERR);

		mkln(2,"FUND:",5);
		tedit( (char *)&emp_sched.es_fund,"_0",txt_line,R_DOUBLE); 
		mkln(8,txt_line,2);
		mkln(14,"COST CENTER:",12);
		tedit( (char *)&emp_sched.es_cost,"__",txt_line,R_DOUBLE); 
		if(emp_sched.es_cost != 0){
			sch_rec.sc_numb = emp_sched.es_cost;
			retval = get_sch(&sch_rec,BROWSE,0,e_mesg);
			if (retval >= 0){
				mkln(43,sch_rec.sc_name,28);
			}
		}

		mkln(68,"AREA:",5);
		mkln(74,emp_sched.es_area,6);
		if(emp_sched.es_area[0]!='\0'&&emp_sched.es_dept[0] != '\0'){
			strcpy(area_rec.a_code,emp_sched.es_area);
			strcpy(area_rec.a_deptcode,emp_sched.es_dept);
			flg_reset(AREA);

			retval = get_n_area(&area_rec,BROWSE,0,FORWARD,e_mesg);
			if(retval >= 0 && 
			   strcmp(area_rec.a_deptcode,emp_sched.es_dept)==0 &&
			   strcmp(area_rec.a_code,emp_sched.es_area)==0){
				mkln(82,area_rec.a_desc,30);
			}
		}
		if(prnt_line() < 0 )	return(REPORT_ERR);

		mkln(2,"UNITS/DAY",9);
		mkln(14,"1.",2);
		tedit((char *)&emp_sched.es_units[0],"0_.__",txt_line,R_DOUBLE);
		mkln(16,txt_line,5);
		mkln(23,"2.",2);
		tedit((char *)&emp_sched.es_units[1],"0_.__",txt_line,R_DOUBLE);
		mkln(25,txt_line,5);
		mkln(32,"3.",2);
		tedit((char *)&emp_sched.es_units[2],"0_.__",txt_line,R_DOUBLE);
		mkln(34,txt_line,5);
		mkln(41,"4.",2);
		tedit((char *)&emp_sched.es_units[3],"0_.__",txt_line,R_DOUBLE);
		mkln(43,txt_line,5);
		mkln(50,"5.",2);
		tedit((char *)&emp_sched.es_units[4],"0_.__",txt_line,R_DOUBLE);
		mkln(52,txt_line,5);
		mkln(59,"6.",2);
		tedit((char *)&emp_sched.es_units[5],"0_.__",txt_line,R_DOUBLE);
		mkln(61,txt_line,5);
		mkln(68,"7.",2);
		tedit((char *)&emp_sched.es_units[6],"0_.__",txt_line,R_DOUBLE);
		mkln(70,txt_line,5);

		strcpy(position.p_code,emp_sched.es_class);
		retval = get_position(&position,BROWSE,0,e_mesg);
		if(retval >= 0){ 
	    		if((strcmp(position.p_type, "FT") == 0) ||
	       	           (strcmp(position.p_type, "PT") == 0)){
			   	mkln(77,"Percentage :",12);
				tedit((char *)&emp_sched.es_amount,
						"_0_.__",txt_line,R_DOUBLE);
				mkln(90,txt_line,5);
			}
			else {
				mkln(77,"Rate       :",12);
				if(class.c_units != 0){
	 	  		   temp_calc =
					class.c_yrly_inc/class.c_units; 
				   tedit((char *)&temp_calc,
						"___,_0_.__",txt_line,R_DOUBLE);
				   mkln(90,txt_line,10);
				}
				mkln(102,"AMOUNT:",7);
				tedit((char *)&emp_sched.es_amount,
					"_,___,_0_.__",txt_line,R_DOUBLE);
				mkln(110,txt_line,12);
			}
		}
		if(prnt_line() < 0 )	return(REPORT_ERR);
		if(prnt_line() < 0 )	return(REPORT_ERR);
	}
	seq_over(EMP_SCHED1);

	return(NOERROR);

}
/*---------------------------------------------------------------------*/
static
PrintRespDed()
{
	char	txt_line[132];
	int	retval, resp_ded_nbr;

	if( pgcnt && term < 99)   /* new page and display */
		if(next_page()<0) return(EXIT);	
		
	mkln(2,"*RESPONSIBILITIES/DEDUCTIONS*",29);
	mkln(61,"*MISCELLANEOUS*",15);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	mkln(2,"TYPE  RESP/DEDUCTION  EARNINGS   %    AMOUNT    TARGET",54);
	mkln(61,"SALARY      :",13);
	retval = CalcSalary();
	if(retval >= 0){
		tedit((char *)&employee_salary, "__,_0_.__", 
							txt_line,R_DOUBLE);
		mkln(75,txt_line,9);
	}
	mkln(88,"YTD REGULAR UNITS   :",21);
	tedit((char *)&runits, "_,_0_.__", txt_line,R_DOUBLE); 
	mkln(110,txt_line,8);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(2,"(R/D)     CODE          CODE   (F/P)",36);
	mkln(61,"VACATION PAY: %",15);
	tedit((char *)&emp_rec.em_vac_rate, "_0_.__", txt_line,R_DOUBLE); 
	mkln(77,txt_line,6);
	mkln(88,"YTD REGULAR INCOME  :",21); 
	tedit((char *)&rinc, "_,___,_0_.__", txt_line,R_DOUBLE); 
	mkln(110,txt_line,12);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(61,"PRE-PAID    :",13);
	mkln(75,emp_rec.em_pre_paid,1);
	mkln(88,"YTD HIGH UNITS      :",21); 
	tedit((char *)&hunits, "_,_0_.__", txt_line,R_DOUBLE); 
	mkln(110,txt_line,8);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	resp_ded_nbr = 0;
	strcpy(emp_extra.ee_numb,emp_rec.em_numb);
	emp_extra.ee_class[0] = '\0';
	emp_extra.ee_type[0] = '\0';	
	flg_reset(EMP_EXTRA);
	
	for( ; ; ) {
		retval = get_n_emp_extra(&emp_extra,BROWSE,0,FORWARD,e_mesg);
		if(retval < 0) {
			if(retval == EFL) break;
			fomer(e_mesg);
			return(retval);
		}

		if(strcmp(emp_extra.ee_numb,emp_rec.em_numb)!=0) 
			break;

		mkln(4,emp_extra.ee_type,1);
		mkln(11,emp_extra.ee_class,6);
		if(emp_extra.ee_type[0] == 'R')
			mkln(24,emp_extra.ee_earn,6);
		mkln(35,emp_extra.ee_amt_flg,1);
		tedit((char *)&emp_extra.ee_amount, "_,_0_.__", txt_line,
								R_DOUBLE); 
		mkln(39,txt_line,8);
		tedit((char *)&emp_extra.ee_target, "__,_0_.__", txt_line,
								R_DOUBLE); 
		mkln(49,txt_line,9);
		
		if(resp_ded_nbr == 0){
			mkln(88,"YTD HIGH INCOME     :",21);
			tedit((char *)&hinc, "_,___,_0_.__", txt_line,R_DOUBLE);
			mkln(110,txt_line,12);
		}
		if(resp_ded_nbr == 1) {
			mkln(88,"CALENDAR YTD INCOME :",21);
			tedit((char *)&calytd, "_,___,_0_.__", txt_line,
								R_DOUBLE);
			mkln(110,txt_line,12);
		}
		if(resp_ded_nbr == 2) {
			mkln(88,"SCHOOL YTD INCOME   :",21);
			tedit((char *)&schytd, "_,___,_0_.__", txt_line,
								R_DOUBLE);
			mkln(110,txt_line,12);
		}
		resp_ded_nbr++;
		if(prnt_line() < 0 )	return(REPORT_ERR);
	}
	seq_over(EMP_EXTRA);

	if(resp_ded_nbr == 0){
		mkln(88,"YTD HIGH INCOME     :",21);
		tedit((char *)&hinc, "_,___,_0_.__", txt_line,R_DOUBLE);
		mkln(110,txt_line,12);
		if(prnt_line() < 0 )	return(REPORT_ERR);
		resp_ded_nbr++;
	}
	if(resp_ded_nbr == 1) {
		mkln(88,"CALENDAR YTD INCOME :",21);
		tedit((char *)&calytd, "_,___,_0_.__", txt_line,
							R_DOUBLE);
		mkln(110,txt_line,12);
		if(prnt_line() < 0 )	return(REPORT_ERR);
		resp_ded_nbr++;
	}
	if(resp_ded_nbr == 2) {
		mkln(88,"SCHOOL YTD INCOME   :",21);
		tedit((char *)&schytd, "_,___,_0_.__", txt_line,
							R_DOUBLE);
		mkln(110,txt_line,12);
		if(prnt_line() < 0 )	return(REPORT_ERR);
		resp_ded_nbr++;
	}
	if(prnt_line() < 0 )	return(REPORT_ERR);

	return(NOERROR);

}
/*---------------------------------------------------------------------*/
static
PrintEarn()
{
	char	txt_line[132];
	int	retval;

	if( pgcnt && term < 99)   /* new page and display */
		if(next_page()<0) return(EXIT);	
		
	mkln(2,"*EARNINGS*",10);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	mkln(2,"CPP EXEMPT (Y/N)    :",21);
	mkln(24,emp_rec.em_cpp_exp,1);
	mkln(63,"HOUSING DEDUCTION:",18);
	tedit((char *)&emp_rec.em_ho_ded, "__,_0_.__", txt_line,R_DOUBLE);
	mkln(82,txt_line,9);
	mkln(93,"YTD DEFERRED INCOME :",21);
	tedit((char *)&ytd_def, "_,___,_0_.__", txt_line,R_DOUBLE);
	mkln(115,txt_line,12);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(2,"UIC EXEMPT (Y/N)    :",21);
	mkln(24,emp_rec.em_uic_exp,1);
	mkln(63,"ANNUAL DEDUCTIONS:",18);
	tedit((char *)&emp_rec.em_ann_ded, "__,_0_.__", txt_line,R_DOUBLE);
	mkln(82,txt_line,9);
	mkln(93,"YTD CPP CONTR       :",21);
	tedit((char *)&ytd_cpp, "_,___,_0_.__", txt_line,R_DOUBLE);
	mkln(115,txt_line,12);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(2,"UIC RATE            :",21);
	tedit((char *)&emp_rec.em_uic_rate, "_0_.____", txt_line,R_DOUBLE);
	mkln(24,txt_line,8);
	mkln(63,"FAMILY ALLOWANCE :",18);
	tedit((char *)&emp_rec.em_fam_all, "__,_0_.__", txt_line,R_DOUBLE);
	mkln(82,txt_line,9);
	mkln(93,"CPP PENS EARNINGS   :",21);
	tedit((char *)&ytd_cpp_pen, "_,___,_0_.__", txt_line,R_DOUBLE);
	mkln(115,txt_line,12);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(2,"REG PENSION PLAN    :",21);
	mkln(24,emp_rec.em_reg_pen,6);
	strcpy(reg_pen.rg_code,emp_rec.em_reg_pen);
	reg_pen.rg_pp_code[0] = '\0';
	flg_reset(REG_PEN);

	retval = get_n_reg_pen(&reg_pen,BROWSE,0,FORWARD,e_mesg);
	if(retval >= 0 && 
		strcmp(emp_rec.em_reg_pen,reg_pen.rg_code) == 0){ 
		mkln(32,reg_pen.rg_desc,30);
	}
	mkln(63,"OLD AGE SECURITY :",18);
	tedit((char *)&emp_rec.em_old_age, "__,_0_.__", txt_line,R_DOUBLE);
	mkln(82,txt_line,9);
	mkln(93,"UIC PREMIUMS        :",21);
	tedit((char *)&ytd_uic, "_,___,_0_.__", txt_line,R_DOUBLE);
	mkln(115,txt_line,12);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(2,"FED TAX EXEMPT (Y/N):",21);
	mkln(24,emp_rec.em_tax_exp,1);
	mkln(63,"INC TAX UNION DED:",18);
	tedit((char *)&emp_rec.em_union_dues, "__,_0_.__", txt_line,R_DOUBLE);
	mkln(82,txt_line,9);
	mkln(93,"UIC INS EARNINGS    :",21);
	tedit((char *)&ytd_uic_earn, "_,___,_0_.__", txt_line,R_DOUBLE);
	mkln(115,txt_line,12);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(2,"INCREASE TAX DEDUCT :",21);
	tedit((char *)&emp_rec.em_inc_tax, "__,_0_.__", txt_line,R_DOUBLE);
	mkln(24,txt_line,9);
	mkln(63,"NET TAX CREDIT   :",18);
	tedit((char *)&emp_rec.em_net_tax_cr, "__,_0_.__", txt_line,R_DOUBLE);
	mkln(82,txt_line,9);
	mkln(93,"PEN PLAN CONTR RATE1:",21);
	tedit((char *)&ytd_pen_rate1, "_,___,_0_.__", txt_line,R_DOUBLE);
	mkln(115,txt_line,12);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(2,"OTHER FEDERAL TAX CR:",21);
	tedit((char *)&emp_rec.em_other_fed, "__,_0_.__", txt_line,R_DOUBLE);
	mkln(24,txt_line,9);
	mkln(93,"PEN PLAN CONTR RATE2:",21);
	tedit((char *)&ytd_pen_rate2, "_,___,_0_.__", txt_line,R_DOUBLE);
	mkln(115,txt_line,12);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(93,"PEN PLAN CONTR RATE3:",21);
	tedit((char *)&ytd_pen_rate3, "_,___,_0_.__", txt_line,R_DOUBLE);
	mkln(115,txt_line,12);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(93,"YTD INCOME TAX      :",21);
	tedit((char *)&ytd_tax, "_,___,_0_.__", txt_line,R_DOUBLE);
	mkln(115,txt_line,12);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(93,"YTD INCOME          :",21);
	tedit((char *)&ytd_tax, "_,___,_0_.__", txt_line,R_DOUBLE);
	mkln(115,txt_line,12);
	if(prnt_line() < 0 )	return(REPORT_ERR);

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
	int	month_nbr, start_mth;
	long	dollars;
	double	total, fraction;

	curr_month = 1;

	strcpy(barg_unit.b_code,emp_rec.em_barg);
	barg_unit.b_date = get_date();
	flg_reset(BARG);

	retval = get_n_barg(&barg_unit,BROWSE,0,BACKWARD,e_mesg);
	if(retval < 0 || strcmp(barg_unit.b_code,emp_rec.em_barg)!=0){
		fomer("Error Reading Bargaining Unit File");
		return(-1);
	}
	seq_over(BARG);

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
		sck_acc[i] = emp_rec.em_sck_acc[i];
		mtd_att_sic[i] = 0;	
		mtd_att_vac[i] = 0;
	}
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
			if(strcmp(att.at_code, "S11") == 0)
				mtd_att_sic[curr_month - start_mth] += .5;
			else if(strcmp(att.at_code, "S12") == 0)
				mtd_att_sic[curr_month - start_mth] += 1;
			else
				mtd_att_sic[curr_month - start_mth] += 
		      		(att_his.eah_hours/att_his.eah_sen_hours);
		}
		if(strcmp(att.at_vac, "Y") == 0){
			if(strcmp(att.at_code, "V11") == 0)
				mtd_att_vac[curr_month - start_mth] += .5;
			else if(strcmp(att.at_code, "V12") == 0)
				mtd_att_vac[curr_month - start_mth] += 1;
			else
		        	mtd_att_vac[curr_month - (start_mth)] += 
		   		(att_his.eah_hours/att_his.eah_sen_hours);
		}
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
			bal_sic = emp_rec.em_sic_ent -  
				mtd_att_sic[prev_mon-start_mth] +
			        sck_acc[prev_mon-start_mth];
			if(ytd_att_sic[prev_mon-start_mth] > 
			   barg_unit.b_sick_max)
				ytd_att_sic[prev_mon-start_mth] = 
					barg_unit.b_sick_max;
			ytd_att_vac[prev_mon-start_mth] = 
				emp_rec.em_vac_ent -
				mtd_att_vac[prev_mon-start_mth]; 
			bal_vac = emp_rec.em_vac_ent - 
				mtd_att_vac[prev_mon-start_mth]; 
		}	
		else {
			ytd_att_sic[prev_mon-start_mth] = 
				ytd_att_sic[prev_mon-start_mth-1] - 
				mtd_att_sic[prev_mon-start_mth] +
		  		sck_acc[prev_mon-start_mth];
			bal_sic = bal_sic - mtd_att_sic[prev_mon-start_mth] + 
		  		sck_acc[prev_mon-start_mth];
			if(ytd_att_sic[prev_mon-start_mth] > 
			   barg_unit.b_sick_max)
				ytd_att_sic[prev_mon-start_mth] = 
					barg_unit.b_sick_max;
			ytd_att_vac[prev_mon-start_mth] = 
				ytd_att_vac[prev_mon-start_mth-1] - 
				mtd_att_vac[prev_mon-start_mth]; 
			bal_vac =  bal_vac - mtd_att_vac[prev_mon-start_mth];  
		}
		month_nbr++;
	}

	return(NOERROR);

}
/*--------------------------------------------------------------------
 GetSen()     - this routine reads the seniority information and   
   		 copies it onto the screen
--------------------------------------------------------------------*/
static
GetSen()
{
	int 	retval, i, j, curr_day, curr_month;
	int	month_nbr, nbr_of_mths = 12, prev_mon;

	strcpy(sen_par.sn_position,emp_rec.em_pos);
	sen_par.sn_eff_date = get_date();
	flg_reset(SEN_PAR);

	retval = get_n_sen_par(&sen_par,BROWSE,0,BACKWARD,e_mesg);
	if(retval < 0 || strcmp(sen_par.sn_position, emp_rec.em_pos) != 0){
		fomer("Error Reading Seniority File");
		get();
	}

	seq_over(SEN_PAR);

	if(pay_param.pr_st_mth == 1){
		start_date = pay_param.pr_cal_st_dt;
		start_month = ((pay_param.pr_cal_st_dt / 100) % 100);
	}
	if(pay_param.pr_st_mth == 2){
		start_date = pay_param.pr_fisc_st_dt;
		start_month = ((pay_param.pr_fisc_st_dt / 100) % 100);
	}
	if(pay_param.pr_st_mth == 3){
		start_date = pay_param.pr_schl_st_dt;
		start_month = ((pay_param.pr_schl_st_dt / 100) % 100);
	}

	for (i=0; i < 12; i++) {
		if((start_month + i)> 12)
			j = i - start_month+1 ;
		else
			j = i + start_month-1;
		strcpy(sen_mon[i], month[j]);
		casual_hours[i] = 0;
		casual_days[i] = 0;
		permanent_days[i] = 0;
		tot_cas_days[i] = 0;
		perm_yrs[i] = 0; 
		perm_days[i] = 0; 
		cas_yrs[i] = 0;
		cas_days[i] = 0;
	}

	init_castot = emp_rec.em_cas_tot_days;
	init_perm_yrs = emp_rec.em_per_tot_yrs;
	init_perm_days = emp_rec.em_per_tot_days;
	init_cas_yrs = emp_rec.em_cas_tot_yrs;
	init_cas_days = emp_rec.em_cas_tot_days;

	if(cas_days[i] > sen_par.sn_max_days_yr){ 
		cas_yrs[0] ++;
		cas_days[0] -= sen_par.sn_max_days_yr;
	}
	curr_month = 0;

	strcpy(emp_sen.esn_numb, emp_rec.em_numb);
	emp_sen.esn_month = start_date - (start_date % 10000) +
			 (start_month*100) + 01;
	emp_sen.esn_pos[0] = '\0';
	emp_sen.esn_class[0] = '\0';
	flg_reset(EMP_SEN);

	for( ; ; ){
		retval = get_n_emp_sen(&emp_sen, BROWSE, 0, FORWARD, e_mesg);
		if(retval == EFL)
			break;
		if(strcmp(emp_sen.esn_numb, emp_rec.em_numb) != 0)
			break;

		if(retval < 0) {
			fomen(e_mesg);
			get();
			return(-1);
		}

		curr_day = (emp_sen.esn_month % 100);
		curr_month = ((emp_sen.esn_month /100) % 100);

		if(curr_month < start_month)
			curr_month += nbr_of_mths;

		casual_hours[curr_month - start_month] += 
				emp_sen.esn_cas_hrs;
		casual_days[curr_month - start_month] += 
				emp_sen.esn_cas_days;
		permanent_days[curr_month - start_month] += 
				emp_sen.esn_perm_days;
		tot_cas_days[curr_month - start_month] += 
				emp_sen.esn_cas_totd;
	}
	seq_over(EMP_SEN);

	month_nbr = 1;

	for(prev_mon = start_month; month_nbr<= nbr_of_mths; prev_mon++){

		if(prev_mon == start_month){
			perm_yrs[prev_mon-start_month] =
				init_perm_yrs; 
	 		perm_days[prev_mon-start_month] =
				init_perm_days +
				perm_days[prev_mon-start_month];

			cas_yrs[prev_mon-start_month] = 
			   init_cas_yrs;
			cas_days[prev_mon-start_month] = 
        		   init_cas_days +
			   tot_cas_days[prev_mon-start_month];
		}
		else{
			perm_yrs[prev_mon-start_month] =
			   perm_yrs[prev_mon-start_month-1];
	 		perm_days[prev_mon-start_month] =
	 		   perm_days[prev_mon-start_month-1]+
			   permanent_days[prev_mon-start_month];

			cas_yrs[prev_mon-start_month] = 
			   cas_yrs[prev_mon-start_month-1];
			cas_days[prev_mon-start_month] = 
			   cas_days[prev_mon-start_month-1] +
			   tot_cas_days[prev_mon-start_month];
		}

		if(perm_days[prev_mon-start_month] >
			sen_par.sn_max_days_yr){
		    	   perm_yrs[prev_mon-start_month] ++;
		    	   perm_days[prev_mon-start_month] -=
				sen_par.sn_max_days_yr;
		}

		if(cas_days[prev_mon-start_month] >
				sen_par.sn_max_days_yr){
		    	   cas_yrs[prev_mon-start_month] ++;
		    	   cas_days[prev_mon-start_month] -=
				sen_par.sn_max_days_yr;
		}
		month_nbr++;
	}

	return(NOERROR);
}
/*---------------------------------------------------------------------*/
static
GetEarnings()
{
	int	retval;

	runits = 0;
	rinc = 0;
	hunits = 0;
	hinc = 0;
	calytd = 0;
	schytd = 0;
	
	ytd_gross = 0.00;
	ytd_def = 0.00;
	ytd_tax = 0.00;
	ytd_cpp = 0.00;
	ytd_cpp_pen = 0.00;
	ytd_uic = 0.00;
	ytd_uic_earn = 0.00;
	ytd_ben = 0.00;
	ytd_reg = 0.00;
	ytd_pen_rate1 = 0.00;
	ytd_pen_rate2 = 0.00;
	ytd_pen_rate3 = 0.00;

	strcpy(emp_earn.en_numb,emp_rec.em_numb);
	emp_earn.en_pp = 0;
	emp_earn.en_week = 0;
	emp_earn.en_date = 0;
	flg_reset(EMP_EARN);

	for(;;){
		retval = get_n_emp_earn(&emp_earn,BROWSE,0,FORWARD,e_mesg);
		if(retval == EFL) break;
		if(retval < 0){
			fomer(e_mesg);
			return(retval);
		}

		if(strcmp(emp_earn.en_numb,emp_rec.em_numb)!=0) break;

		if(emp_earn.en_date>= 
				pay_param.pr_fisc_st_dt&&emp_earn.en_date <=
			pay_param.pr_fisc_end_dt){
			runits += emp_earn.en_reg_units;
			rinc += emp_earn.en_reg_inc;
			hunits += emp_earn.en_high_units;
			hinc += emp_earn.en_high_inc;
		}

		if(emp_earn.en_date >= 
				pay_param.pr_cal_st_dt && emp_earn.en_date <=
			pay_param.pr_cal_end_dt){
			calytd += (emp_earn.en_reg1 + emp_earn.en_reg1 +
			 emp_earn.en_reg1 + emp_earn.en_high_inc);
		}

		if(emp_earn.en_date>= 
				pay_param.pr_schl_st_dt && emp_earn.en_date <=
			pay_param.pr_schl_end_dt){
			schytd += (emp_earn.en_reg1 + emp_earn.en_reg1 +
			 emp_earn.en_reg1 + emp_earn.en_high_inc);
		}

		ytd_gross += D_Roundoff(emp_earn.en_reg_inc +
					 emp_earn.en_high_inc);
		ytd_def += D_Roundoff(emp_earn.en_def_inc);
		ytd_tax += D_Roundoff(emp_earn.en_tax);
		ytd_cpp += D_Roundoff(emp_earn.en_cpp);
		ytd_cpp_pen += D_Roundoff(emp_earn.en_cpp_pen);
		ytd_uic += D_Roundoff(emp_earn.en_uic);
		ytd_reg += D_Roundoff(emp_earn.en_reg1 + emp_earn.en_reg2 +
					 emp_earn.en_reg3);
		ytd_pen_rate1 += D_Roundoff(emp_earn.en_reg1);
		ytd_pen_rate2 += D_Roundoff(emp_earn.en_reg2);
		ytd_pen_rate3 += D_Roundoff(emp_earn.en_reg3);
	}
	seq_over(EMP_EARN);

	strcpy(emp_ins.in_numb,emp_rec.em_numb);
	emp_ins.in_pp = 0;
	emp_ins.in_date = 0;

	flg_reset(EMP_INS) ;

	for( ; ; ) {
		retval = get_n_emp_ins(&emp_ins,BROWSE,0,FORWARD,e_mesg) ;
		if( retval == EFL ||
			strcmp(emp_ins.in_numb,emp_rec.em_numb) != 0)
			break;

		if(emp_ins.in_date < pay_param.pr_fisc_st_dt)
			continue;

		if( retval < 0) {
			roll_back(e_mesg);
			fomer(e_mesg);
			return(retval) ;
		}
		ytd_uic_earn += D_Roundoff(emp_ins.in_uic_ins);
	}
	seq_over(EMP_INS);

	return(NOERROR);
}
/*---------------------------------------------------------------------------*/
static
CalcSalary()
{
	int retval;
	double 	temp_calc3 = 0;
	double temp_calc2 = 0;
	int	temp_units = 0;
	int j;
	double	one_hundred = 100;

	strcpy(barg_unit.b_code,emp_rec.em_barg);
	barg_unit.b_date = get_date();
	flg_reset(BARG);

	retval = get_n_barg(&barg_unit,BROWSE,0,BACKWARD,e_mesg);
	if(retval < 0 ||
		strcmp(barg_unit.b_code, emp_rec.em_barg) != 0){
  		fomer("Bargaining Unit does not Exist");
		return(NOERROR);
	}
	seq_over(BARG);

	if(emp_rec.em_class[0] == '\0') {
		employee_salary = 0;
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
			employee_salary = 0;
			return(-1);
		}
	 	if(class.c_units != 0) {	
			temp_units = 0;
			strcpy(emp_sched.es_numb,emp_rec.em_numb);
			emp_sched.es_week = 0;
			emp_sched.es_fund = 0;
			emp_sched.es_class[0] = LV_CHAR;
			emp_sched.es_cost = 0;
			emp_sched.es_dept[0] = LV_CHAR;
			emp_sched.es_area[0] = LV_CHAR;
			flg_reset(EMP_SCHED1);

			for( ; ; ) {
		       	  retval = get_n_emp_sched1(&emp_sched,BROWSE,0,
				   FORWARD,e_mesg);
			  if(retval < 0 && retval != EFL) {
				fomer(e_mesg);
			  }
			  if(strcmp(emp_sched.es_numb,emp_rec.em_numb)!=0
				   ||retval == EFL)
				break;
			  for(j=0 ; j<7 ; j++) {
				  temp_units += emp_sched.es_units[j];
				}
			}
			seq_over(EMP_SCHED1);
 		        temp_calc3 = class.c_yrly_inc / class.c_units; 
			temp_calc3 = D_Roundoff(temp_calc3);
			employee_salary = temp_calc3;
		}
		else{
			strcpy(pay_per.pp_code,barg_unit.b_pp_code);
			pay_per.pp_year = 0;
			flg_reset(PAY_PERIOD);

			retval = get_n_pay_per(&pay_per,BROWSE,0,
						FORWARD, e_mesg);
			if(retval < 0 ||
			   strcmp(pay_per.pp_code,barg_unit.b_pp_code)!=0) {
				fomer("Pay Period Code Does not Exist");
				get();
			}
			seq_over(PAY_PERIOD);

			if(class.c_yrly_inc == 0){
				strcpy(cert.cr_code,emp_rec.em_cert);
				/* ANDRE */
				cert.cr_level = emp_rec.em_level;
				cert.cr_date = get_date();
				flg_reset(CERT);
		
				retval = get_n_cert(&cert,BROWSE,2,BACKWARD,
					e_mesg);
				if(retval < 0) {
					fomer("Certificate Code Does not Exist");
					get();
				}
				temp_calc2 = cert.cr_income / 
					(double)pay_per.pp_numb;
				temp_calc3 = temp_calc2 * emp_rec.em_perc / one_hundred;
				temp_calc3 = D_Roundoff(temp_calc3);
			}
			else{
				 	temp_calc2 = class.c_yrly_inc / 
						(double)pay_per.pp_numb;
					temp_calc3 = temp_calc2 * emp_rec.em_perc/ one_hundred; 
					temp_calc3 = D_Roundoff(temp_calc3);
		    	}
			employee_salary = temp_calc3;
		}
	}
	return(NOERROR);
}
/*-----------------------------------------------------------------------*/
static
BldSchedAdd ()
{
	int	x, retval;

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
		strcpy(emp_sched.es_numb,emp_rec.em_numb);
		emp_sched.es_week	= 0;
		emp_sched.es_fund	= 0;
		emp_sched.es_class[0] 	= '\0';
		emp_sched.es_cost	= 0;
		emp_sched.es_dept[0]	= '\0';
		emp_sched.es_area[0]	= '\0';
		flg_reset(EMP_SCHED1);
		
		sched1_flag[0] = 'N';
		for (;;){
			retval=get_n_emp_sched1(&emp_sched,BROWSE,0,FORWARD,e_mesg);
			if (retval < 0)
				break;	

			if(strcmp(emp_rec.em_numb,emp_sched.es_numb) != 0) {
				sched1_flag[0] = 'N';
				break;
			}
			if(strcmp(emp_sched.es_class,class1) < 0 ||
	          	 strcmp(emp_sched.es_class,class2) > 0 ) 
				continue;

			if( (emp_sched.es_cost < center1) || 
		   	(emp_sched.es_cost > center2) )
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
	int	retval;
 	
	if(sortop2[0] == 'L'){
		if(sched1_flag[0] == 'Y') {
			sprintf(sort_array,"%-d",emp_sched.es_cost);
		}
		else {
			sprintf(sort_array,"%-d",emp_rec.em_cc);
		}
		strncpy(tmp_sched1.tm_sortk_1,sort_array,4);
	}
	else{
		if(sched1_flag[0] == 'Y') {
			strncpy(tmp_sched1.tm_sortk_1,emp_sched.es_class,6);
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
		strncpy(tmp_sched1.tm_class,emp_sched.es_class,6);
		tmp_sched1.tm_cost = emp_sched.es_cost;
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
