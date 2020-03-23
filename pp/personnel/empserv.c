/******************************************************************************
		Sourcename   : empserv.c
		Module       : Personnel/Payroll
		Created on   : 92-SEP-19
		Created  By  : m. galvin 

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
#include <cfomstrc.h>
#include <filein.h>
#include <isnames.h>
#define  FIRST

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

static 	Emp		emp_rec;
static	Emp_sched1	emp_sched1;
static	Pa_rec		pa_rec;
static	Sch_rec		sch_rec;
static	Tmp_sched1	tmp_sched1;
static	Class		class;
static	Sen_par		sen_par;
static	Emp_sen		emp_sen;
static	Pay_param	pay_param;
static 	Att		att_rec;
static	Barg_unit	barg_unit;
static	Position	position;

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
static	short	perm_years=0;
static	double	perm_days=0;
static	short	cas_years=0;
static	double	cas_days=0;
static	short	tot_years=0;
static	short	serv_years=0;
static	double	tot_days=0;
static	double	sick_bal=0;
static	double	vac_bal=0;

static	double	serv_tot=0, break_serv_tot=0, grand_serv_tot=0;

static	short	last_tot_years = 0;
static	char	sched1_flag[2];

static	short	last_center;
static	char	last_barg[7];
static  char	last_class[7];
static	char	last_position[7];

static	char	tottype[2];

extern char 	e_mesg[200];	/* for storing error messages */

empserv()
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
			strcpy( discfile, "empserv.dat" );
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

	if((retval = DisplayMessage("For Totals Only (Y/N)?"))<0) {
		return(retval);
	}

	if((retval = GetResponse(tottype))<0) {
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

	retval = PrintRpt();
	if(retval < 0)	return(-1);

	if(retval != EXIT){
		retval = PrintServTot();
		if(retval < 0)	return(-1);
		retval = PrintSubTot();
		if(retval < 0)	return(-1);
		retval = PrintGrTot();
		if(retval < 0)	return(-1);
	}

	close_rep();	/* close output file */
	close_dbh();

	return(retval);
}
/*****************************************************************************/
static	
PrintRpt()
{	
	int	first_time = 0, key = 0;

	/* Get the payroll parameter record */

	unlink_file(TMP_SCHED1);

 	retval = get_pay_param(&pay_param,BROWSE,1,e_mesg);
	if(retval < 0) {
  		fomen(e_mesg);
		return(retval);
	}

	/* get the cost center name */
	retval = get_param(&pa_rec,BROWSE,1,e_mesg);
	if (retval < 0){
		fomer(e_mesg);get ();
	}

        strcpy(emp_rec.em_numb,empl1);
	flg_reset(EMPLOYEE);

	for(;;){
		retval=get_n_employee(&emp_rec,BROWSE,0,FORWARD,e_mesg);
		if(retval == EFL)
			break;

	  	retval=UsrBargVal(BROWSE,emp_rec.em_numb,emp_rec.em_barg,0,
									e_mesg);
	  	if(retval < 0)
			continue;

		if(retval<0) {
			fomer(e_mesg);
			continue;
		}

		if(strcmp(emp_rec.em_status,"ACT")!=0) 
			continue;

		if(strcmp(emp_rec.em_barg,barg1) < 0 ||
	  	   strcmp(emp_rec.em_barg,barg2) > 0 )
			continue;

		if(strcmp(emp_rec.em_pos,posi1) < 0 ||
	   	   strcmp(emp_rec.em_pos,posi2) > 0 )
			continue;
	
	   	if(strcmp(emp_rec.em_numb,empl2) > 0 )
			break;

		if((emp_rec.em_cc < center1) || 
	   	   (emp_rec.em_cc > center2))
			continue;

		if(strcmp(emp_rec.em_class,class1) < 0 ||
		   strcmp(emp_rec.em_class,class2) > 0 ) 
			continue;

		if((retval=CalcSenInfo())==ERROR)	return(retval);

		if(tot_years < 9)
			tot_years = 8;
		else{
			if(tot_years < 21)
				tot_years = 20; 
			else {
				if(tot_years < 26)
					tot_years = 25;
				else
					tot_years = 26;
			}
		}
	
		retval = PutTmpFile();
		if(retval < 0)
			continue;

	}/*end of for loop*/
	seq_over(EMPLOYEE);

	serv_tot = 0;

	tmp_sched1.tm_sortk_1[0] = '\0';
	tmp_sched1.tm_sortk_2[0] = '\0';
	tmp_sched1.tm_sortk_3[0] = '\0';
	flg_reset(TMP_SCHED1);

	for(;;) {
		retval = get_n_tmp_sched1(&tmp_sched1,BROWSE,1,FORWARD,e_mesg);

		if(retval < 0) {
			break;
		} 
		strcpy(emp_rec.em_numb,tmp_sched1.tm_numb);
		retval=get_employee(&emp_rec,BROWSE,0,e_mesg);
		if(retval < 0){ 
			fomen(e_mesg);
			get();
		}

	  	retval=UsrBargVal(BROWSE,emp_rec.em_numb,emp_rec.em_barg,0,
									e_mesg);
	  	if(retval < 0)
			continue;

		if((retval=CalcSenInfo())==ERROR)	return(retval);

		if(tot_years < 9)
			serv_years = 8;
		else{
			if(tot_years < 21)
				serv_years = 20; 
			else {
				if(tot_years < 26)
					serv_years = 25;
				else
					serv_years = 26;
			}
		}
	
		if(sortop2[0] == 'B') {
			if(strcmp(last_barg,emp_rec.em_barg)!=0
			  || first_time == 0 || linecnt > PG_SIZE||
			  last_tot_years != serv_years) { 
				if(strcmp(emp_rec.em_barg,last_barg)!=0 &&
				  first_time != 0){
					retval = PrintServTot();
					if(retval < 0)	return(retval);
					if(retval == EXIT)	break;
					retval = PrintSubTot();
					if(retval < 0)	return(retval);
					if(retval == EXIT)	break;
				}
				else{	
					if(last_tot_years != serv_years &&
				   	first_time != 0){
				 	  retval = PrintServTot();
					  if(retval < 0)	return(retval);
					  if(retval == EXIT)	break;
					}
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
			  || first_time == 0 || linecnt > PG_SIZE||
			  last_tot_years != serv_years) { 
				if(strcmp(last_position,emp_rec.em_pos)!=0 &&
				  first_time != 0){
					retval = PrintServTot();
					if(retval < 0)	return(retval);
					if(retval == EXIT)	break;
					retval = PrintSubTot();
					if(retval < 0)	return(retval);
					if(retval == EXIT)	break;
				}
				else{	
					if(last_tot_years != serv_years &&
				   	first_time != 0){
				 	  retval = PrintServTot();
					  if(retval < 0)	return(retval);
					  if(retval == EXIT)	break;
					}
				}
				retval = PrntHdg();
				if(retval < 0) 	{
					return(retval);	
				}
				if(retval == EXIT)	break;
				first_time = 1;
			}
		}
		if(sortop2[0] == 'L') {
			if(last_center != emp_rec.em_cc 
			  || first_time == 0 || linecnt > PG_SIZE||
			  last_tot_years != serv_years) { 
				if(last_center != emp_rec.em_cc &&
				  first_time != 0){
					retval = PrintServTot();
					if(retval < 0)	return(retval);
					if(retval == EXIT)	break;
					retval = PrintSubTot();
					if(retval < 0)	return(retval);
					if(retval == EXIT)	break;
				}
				else{	
					if(last_tot_years != serv_years &&
				   	first_time != 0){
				 	  retval = PrintServTot();
					  if(retval < 0)	return(retval);
					  if(retval == EXIT)	break;
					}
				}
				retval = PrntHdg();
				if(retval < 0) 	{
					return(retval);	
				}
				if(retval == EXIT)	break;
				first_time = 1;
			}
		}
		
		if(sortop2[0] == 'C'){
			if(strcmp(last_class,emp_rec.em_class)!=0 || 
			   first_time == 0 || linecnt > PG_SIZE||
			   last_tot_years != serv_years) { 
				if(strcmp(last_class,emp_rec.em_class)!=0 &&
				  first_time != 0){
					retval = PrintServTot();
					if(retval < 0)	return(retval);
					if(retval == EXIT)	break;
					retval = PrintSubTot();
					if(retval < 0)	return(retval);
					if(retval == EXIT)	break;
				}
				else{	
					if(last_tot_years != serv_years &&
				   	first_time != 0){
				 	  retval = PrintServTot();
					  if(retval < 0)	return(retval);
					  if(retval == EXIT)	break;
					}
				}
				retval = PrntHdg();
				if(retval < 0) 	{
					return(retval);	
				}
				if(retval == EXIT)	break;
				first_time = 1;
			}
		}

		if(tottype[0] == 'N'){
			if ((retval = PrntRec())<0)
				return(retval);
			if(retval == EXIT)	break;
		}

		serv_tot++;

		last_center = emp_rec.em_cc;
		strcpy(last_barg,emp_rec.em_barg);
		strcpy(last_position,emp_rec.em_pos);
		strcpy(last_class,emp_rec.em_class);
		last_tot_years = serv_years;

	}
	seq_over(TMP_SCHED1);
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
	if(prnt_line() < 0 )	return(REPORT_ERR);

#ifdef ENGLISH
	mkln((LNSZ-25)/2,"EMPLOYEE YEARS OF SERVICE", 25 );
#else
	mkln((LNSZ-25)/2,"TRANSLATE        ", 17 );
#endif
	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(serv_years == 8 )
		mkln((LNSZ-31)/2,"FOR EMPLOYEES WITH 0 TO 8 YEARS", 31 );
	if(serv_years == 20)
		mkln((LNSZ-32)/2,"FOR EMPLOYEES WITH 9 TO 20 YEARS", 32 );
	if(serv_years == 25)
		mkln((LNSZ-33)/2,"FOR EMPLOYEES WITH 21 TO 25 YEARS", 33 );
	if(serv_years == 26)
		mkln((LNSZ-39)/2,"FOR EMPLOYEES WITH 26 AND GREATER YEARS",39 );
	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);

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
		strcpy(barg_unit.b_code,emp_rec.em_barg);
		barg_unit.b_date = get_date();
		flg_reset(BARG);

		retval = get_n_barg(&barg_unit,BROWSE,0,BACKWARD,e_mesg);
		if(retval < 0){
			fomer(e_mesg);get();
		}
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

	if(tottype[0] != 'Y') {
		mkln(3,"EMPLOYEE",13);
		mkln(24,"EMPLOYEE",13);
		mkln(55,"CC#",3);
		mkln(62,"CLASS",5);
		mkln(71,"POSITION",8);
		mkln(82,"YEARS",5);
		mkln(89,"DAYS",4);
		if(prnt_line() < 0 )	return(REPORT_ERR);

		mkln(4,"NUMBER",6);
		mkln(26,"NAME",4);
		mkln(62,"CODE",4);
		mkln(73,"CODE",4);
		if(prnt_line() < 0 )	return(REPORT_ERR);
		if(prnt_line() < 0 )	return(REPORT_ERR);
	}

	return(NOERROR);
}
/*--------------------------------------------------------------------*/
static
PrntRec()
{
	char	txt_line[132];

	mkln(1,emp_rec.em_numb,12);
	sprintf(txt_line,"%s, %s %s",
		emp_rec.em_last_name,
		emp_rec.em_first_name,
		emp_rec.em_mid_name);
	mkln(14,txt_line,35);

	tedit((char*)&emp_rec.em_cc,"____",txt_line,R_SHORT);
	mkln(55,txt_line,4);
	mkln(62,emp_rec.em_class,6);
	mkln(72,emp_rec.em_pos,6);
		
	tedit((char*)&tot_years,"__",txt_line,R_SHORT);
	mkln(83,txt_line,2);
	tedit((char*)&tot_days,"___.__",txt_line,R_DOUBLE);
	mkln(88,txt_line,6);

	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	return(NOERROR);
}
/*-----------------------------------------------------------------------*/
static
PrintServTot()
{
	char	txt_line[132];

	if(prnt_line() < 0 )	return(REPORT_ERR);

	if(last_tot_years == 8) 
		mkln(1,"TOTAL FOR EMPLOYEES WITH 0 TO 8 YEARS:",38); 
	if(last_tot_years == 20) 
		mkln(1,"TOTAL FOR EMPLOYEES WITH 9 TO 20 YEARS:",39); 
	if(last_tot_years == 25) 
		mkln(1,"TOTAL FOR EMPLOYEES WITH 21 TO 25 YEARS:",40);
	if(last_tot_years == 26) 
		mkln(1,"TOTAL FOR EMPLOYEES WITH 25 YEARS AND GREATER:",46);
	tedit((char*)&serv_tot,"__,_0_",txt_line,R_DOUBLE);
	mkln(88,txt_line,6);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	/* accumulate the total for the break */
	break_serv_tot += serv_tot;

	/*re-init emp totals*/
	serv_tot = 0.0;

	return(NOERROR);
}
/*-----------------------------------------------------------------------*/
static
PrintSubTot()
{
	char	txt_line[132];

	if(prnt_line() < 0 )	return(REPORT_ERR);

	if(sortop2[0] == 'C') 
		mkln(1,"TOTAL FOR CLASS CODE:",20); 
	if(sortop2[0] == 'L') 
		mkln(1,"TOTAL FOR COST CENTER:",22); 
	if(sortop2[0] == 'B') 
		mkln(1,"TOTAL FOR BARGAINING UNIT:",26);
	if(sortop2[0] == 'P') 
		mkln(1,"TOTAL FOR POSITION:",19);
	
	tedit((char*)&break_serv_tot,"__,_0_",txt_line,R_DOUBLE);
	mkln(88,txt_line,6);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	/*re-init emp totals*/
	grand_serv_tot += break_serv_tot;
	
	break_serv_tot = 0;

	return(NOERROR);
}
/*------------------------------------------------------------------------*/
static
PrintGrTot()
{
	char	txt_line[132];

	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	mkln(1,"TOTAL FOR ALL EMPLOYEES:",24);
	tedit((char*)&grand_serv_tot,"__,_0_",txt_line,R_DOUBLE);
	mkln(88,txt_line,6);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	return(NOERROR);
}
/*------------------------------------------------------------------------*/
static
CalcSenInfo()
{
	char	old_position[7];

	tot_days = 0;
	tot_years = 0;
	/*
	************** Calculate the Seniority for the Employees ************
	*/
	perm_years = emp_rec.em_per_tot_yrs;
	perm_days = emp_rec.em_per_tot_days;

	cas_years = emp_rec.em_cas_tot_yrs;
	cas_days = emp_rec.em_cas_tot_days;

	/*tot_days = (cas_days+perm_days);
	tot_years = (cas_years+perm_years);
	*/	
	strcpy(emp_sen.esn_numb, emp_rec.em_numb);
	emp_sen.esn_month = 0;
	emp_sen.esn_pos[0] = '\0';
	emp_sen.esn_class[0] = '\0';
	flg_reset(EMP_SEN);

	old_position[0] = '\0';

	for(;;){

		retval = get_n_emp_sen(&emp_sen, BROWSE, 0, FORWARD, 
			e_mesg);

		if( retval == EFL ||		
		  ( strcmp(emp_sen.esn_numb, emp_rec.em_numb) != 0))
			break;

		if(retval < 0) {
			fomen(e_mesg);
			get();
			return(-1);
		}

		if(strcmp(old_position,emp_sen.esn_pos) != 0) {

			strcpy(sen_par.sn_position,emp_sen.esn_pos);
			sen_par.sn_eff_date = get_date();

			flg_reset(SEN_PAR);

			retval = get_n_sen_par(&sen_par,BROWSE,0,BACKWARD,
				e_mesg);
			if(retval == EFL) {
				fomen("Seniority Parameter Record Not Setup");
				return(retval);
			}
			if(retval < 0) {
				fomen(e_mesg) ;
				get();
				return(ERROR) ;
			}

			strcpy(old_position,emp_sen.esn_pos);
		}

		perm_days += emp_sen.esn_perm_days;
		if(perm_days > sen_par.sn_max_days_yr) {
			perm_years ++;
			perm_days -= sen_par.sn_max_days_yr;
		}

		cas_days +=  emp_sen.esn_cas_totd;
		if(cas_days >= sen_par.sn_max_days_yr) {
			cas_years ++;
			cas_days -= sen_par.sn_max_days_yr;
		}

	}/*end endless loop*/
	seq_over(EMP_SEN);

	if(sen_par.sn_position[0] == '\0'){
		strcpy(sen_par.sn_position,emp_rec.em_pos);
		sen_par.sn_eff_date = get_date();

		flg_reset(SEN_PAR);

		retval = get_n_sen_par(&sen_par,BROWSE,0,BACKWARD,
				e_mesg);
		if(retval == EFL) {
			fomen("Seniority Parameter Record Not Setup");
			return(retval);
		}
		if(retval < 0) {
			fomen(e_mesg) ;
			get();
			return(ERROR) ;
		}
	}

	/* calc tot*/
	tot_years +=(cas_years + perm_years);
	tot_days  +=(cas_days + perm_days);
	if(tot_days >= sen_par.sn_max_days_yr) {
		tot_years ++;
		tot_days -= sen_par.sn_max_days_yr;
	}

	return(NOERROR);
}
/*------------------------------------------------------------------*/
static
BuildTmpFile ()
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

		if( (emp_rec.em_cc < center1) || 
		 (emp_rec.em_cc > center2) )
			continue;

		if(strcmp(emp_rec.em_class,class1) < 0 ||
	         strcmp(emp_rec.em_class,class2) > 0 ) 
			continue;

		sched1_flag[0] = 'N';
		retval = PutTmpFile();
		if(retval < 0)
			break;
	}/*end endless loop*/

	seq_over(EMPLOYEE);
	return(NOERROR);
}
/* ------------------------------------------------------------------------*/
static
PutTmpFile()
{
	char	sort_array[41];
	char	txt_line[132];
 	
	if(sortop2[0] == 'L'){
		sprintf(sort_array,"%-d",emp_rec.em_cc);
		strcpy(tmp_sched1.tm_sortk_1,sort_array);
	}
	if(sortop2[0] == 'C')
		strcpy(tmp_sched1.tm_sortk_1,emp_rec.em_class);
	if(sortop2[0] == 'B')
		strcpy(tmp_sched1.tm_sortk_1,emp_rec.em_barg);
	if(sortop2[0] == 'P')
		strcpy(tmp_sched1.tm_sortk_1,emp_rec.em_pos);

	tedit( (char *)&tot_years,"___",txt_line, R_SHORT ); 
	strcat(tmp_sched1.tm_sortk_1,txt_line);
		
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

	strcpy(tmp_sched1.tm_numb,emp_rec.em_numb);
	strcpy(tmp_sched1.tm_class,emp_rec.em_class);
	tmp_sched1.tm_cost = emp_rec.em_cc;

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
