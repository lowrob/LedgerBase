/******************************************************************************
		Sourcename   : senlist.c
		Module       : Personnel/Payroll
		Created on   : 92-02-25
		Created  By  : Littlejohn 

******************************************************************************
About the file:	

History:
Programmer      Last change on    Details
m. galvin	1992/08/24	  I modified the report due to the changes 
				  made to the files used.  I also changed
				  the report to print both teaching and
				  non-teaching staff.  I also added a few
				  things: a code and description of the
				  sort option selected, a check to see
				  if the employee's status was active and
				  changed the check to see if an employee 
				  was a teacher.

L.Robichaud	1993/10/13	Pass new parameter to close_rep function for 
				a banner to print with 3000 family.
				  
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
static	Pa_rec		pa_rec;
static	Sch_rec		sch_rec;
static	Class		class;
static	Sen_par		sen_par;
static	Emp_sen		emp_sen;
static	Pay_param	pay_param;
static 	Att		att_rec;
static 	Emp_at_his	att_hist;
static	Barg_unit	barg_unit;
static	Position	position;
static	Ts_sen		ts_sen;

/*  Data items for storing the key range end values */
static	char	sortop3[2];
static	char	sortop2[2];
static	char	barg1[7];
static	char	barg2[7];
static	char	posi1[7];
static	char	posi2[7];
static	char	class1[7];
static	char	class2[7];
static	char	empl1[13];
static	char	empl2[13];
static	short	center1;	
static	short	center2;	

static int	PG_SIZE;
static int	retval;
static char 	discfile[15];	/* for storing outputfile name */
static short	pgcnt; 		/* for page count */
static char	resp[2];	/* for storing response */
static short	copies;
static	short	perm_years = 0;
static	double	perm_days=0;
static	short	cas_years=0;
static	double	cas_days=0;
static	short	tot_years=0;
static	double	tot_days=0;
static	double	bal_sic=0;
static	double	bal_vac=0;


static	short	last_center;
static	char	last_barg[7];
static  char	last_class[7];
static	char	last_position[7];
static	int	teacher;

static char 	e_mesg[200];	/* for storing error messages */

senlist()
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
			strcpy( discfile, "senlist.dat" );
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

	sortop3[0] = 'A';
	if((retval =  GetSortop3(sortop3))<0 )
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

	if(sortop3[0] == 'D' || sortop3[0] == 'X') {
		unlink_file(TS_SEN);
		retval = PrintDescRpt();
	}
	else {
		unlink_file(TMPINDX_1);
		STRCPY(tmpindxfile,"emptemp");
		get_tnum(tnum);
		strcat(tmpindxfile,tnum);
		if(CreatTempIndx(tmpindxfile,sortop3,sortop2 )<0){
			return (-1);
		}

		retval = PrintRpt();
	}

	close_rep(BANNER);	/* close output file */
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
	
	/* Get the payroll parameter record */

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

	emp_rec.em_numb[0] = '\0';
	if(sortop2[0] == 'B') {
		strcpy(emp_rec.em_barg,barg1);
		last_barg[0] = '\0';
	}
	if(sortop2[0] == 'P') {
		strcpy(emp_rec.em_pos,posi1);
		last_position[0] = '\0';
	}
	if(sortop2[0] == 'C') {
		strcpy(emp_rec.em_class,class1);
		last_class[0] = '\0';
	}
	if(sortop2[0] == 'L') {
		emp_rec.em_cc = center1;
		last_center = 0;
	}
	emp_rec.em_first_name[0] = '\0';
	emp_rec.em_last_name[0] = '\0';
	flg_reset(TMPINDX_1);

	for (;;){
		retval = get_next((char*)&emp_rec,TMPINDX_1,0,FORWARD,BROWSE,e_mesg);
		if(retval==EFL)		break;
		if(retval<0) {
			fomen(e_mesg);
			get();
		}
		/*check if the employee is a teacher*/
		if(strcmp(emp_rec.em_status,"ACT")!=0) {
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
	
		if(sortop2[0] == 'C') {
			if(strcmp(emp_rec.em_class,class2) > 0)
				break;
		}
		else {
			if(strcmp(emp_rec.em_class,class1) < 0 ||
	   		strcmp(emp_rec.em_class,class2) > 0 ) 
				continue;
		}
	
		if(sortop2[0] == 'L') {
			if(emp_rec.em_cc > center2)
				break;
		}
		else {
			if(emp_rec.em_cc < center1 ||
	   		   emp_rec.em_cc > center2 ) 
				continue;
		}
	
		if(strcmp(emp_rec.em_numb,empl1) < 0 ||
	   	strcmp(emp_rec.em_numb,empl2) > 0 )
			continue;

		teacher = 0;
		/* SJ class codes 
		if(strcmp(emp_rec.em_class,"  0101") == 0) 
			teacher = 1;
		if(strcmp(emp_rec.em_class," 01012") == 0)
			teacher = 1;
		if(strcmp(emp_rec.em_class," 01013") == 0)
			teacher = 1;
		if(strcmp(emp_rec.em_class," 01014") == 0)
			teacher = 1;
		if(strcmp(emp_rec.em_class," 01015") == 0) 
			teacher = 1;
		if(strcmp(emp_rec.em_class," 01016") == 0) 
			teacher = 1;
		if(strcmp(emp_rec.em_class," 01017") == 0)
			teacher = 1;
		if(strcmp(emp_rec.em_class," 01018") == 0)
			teacher = 1; */

		/* Fredericton class codes */
		if(strncmp(emp_rec.em_class,"0101",4) == 0)
			teacher = 1;
		
		if(teacher == 1) {
			sch_rec.sc_numb = emp_rec.em_cc;
			retval = get_sch(&sch_rec,BROWSE,0,e_mesg);
			if (retval < 0){
				fomer(e_mesg);get();
			}
		}
		else {
			GetClassDesc();
		}

		if((retval=CalcSenInfo())==ERROR)	return(retval);
		if((retval=CalcBal())==ERROR)	return(retval);
		if(teacher == 1)
			bal_vac = 0;

		if(sortop2[0] == 'B') {
			if(strcmp(last_barg,emp_rec.em_barg)!=0 ||
			   first_time == 0 || linecnt > PG_SIZE) {
				retval = PrntHdg();
				if(retval < 0){
					return(retval);
				}
				if(retval == EXIT)	break;
				first_time = 1;
			}
		}
		if(sortop2[0] == 'P') {
			if(strcmp(last_position,emp_rec.em_pos)!=0 ||
			   first_time == 0 || linecnt > PG_SIZE) { 
				retval = PrntHdg();
				if(retval < 0) 	{
					return(retval);	
				}
				if(retval == EXIT)	break;
				first_time = 1;
			}
		}

		if(sortop2[0] == 'C') {
			if(strcmp(last_class,emp_rec.em_class)!=0 ||
			   first_time == 0 || linecnt > PG_SIZE) { 
				retval = PrntHdg();
				if(retval < 0) 	{
					return(retval);	
				}
				if(retval == EXIT)	break;
				first_time = 1;
			}
		}

		if(sortop2[0] == 'L') {
			if(last_center != emp_rec.em_cc ||
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
		strcpy(last_barg,emp_rec.em_barg);
		strcpy(last_position,emp_rec.em_pos);
		strcpy(last_class,emp_rec.em_class);
		last_center = emp_rec.em_cc;
	} /*end of endless for loop*/
	
	if(pgcnt){
		if(term<99)
			last_page();
	}

	return(NOERROR);
}
/*****************************************************************************/
static	
PrintDescRpt()
{
	int	first_time = 0, direction;

	/* Get the payroll parameter record */

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

		if(retval<0) {
			fomer(e_mesg);
			continue;
		}

	  	retval=UsrBargVal(BROWSE,emp_rec.em_numb,emp_rec.em_barg,0,
									e_mesg);
	  	if(retval < 0)
			continue;

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

		teacher = 0;
		/* SJ teacher codes
		if(strcmp(emp_rec.em_class,"  0101") == 0) 
			teacher = 1;
		if(strcmp(emp_rec.em_class," 01012") == 0)
			teacher = 1;
		if(strcmp(emp_rec.em_class," 01013") == 0)
			teacher = 1;
		if(strcmp(emp_rec.em_class," 01014") == 0)
			teacher = 1;
		if(strcmp(emp_rec.em_class," 01015") == 0) 
			teacher = 1;
		if(strcmp(emp_rec.em_class," 01016") == 0) 
			teacher = 1;
		if(strcmp(emp_rec.em_class," 01017") == 0)
			teacher = 1;
		if(strcmp(emp_rec.em_class," 01018") == 0)
			teacher = 1; */

		/* Fredericton teacher codes */
		if(strncmp(emp_rec.em_class,"0101",4) == 0)
			teacher = 1;

		retval=CalcSenInfo();	
		if(retval < 0)	continue;

		retval = CalcBal();
		if(retval < 0)	continue;
		if(teacher == 1)
			bal_vac = 0;

		ts_sen.ts_total_years = tot_years;
	 	ts_sen.ts_total_days = tot_days;
		strcpy(ts_sen.ts_position,emp_rec.em_pos);
		strcpy(ts_sen.ts_barg,emp_rec.em_barg);
		ts_sen.ts_cc = emp_rec.em_cc;
		strcpy(ts_sen.ts_emp_numb,emp_rec.em_numb);
		ts_sen.ts_cas_years = cas_years;
		ts_sen.ts_cas_days = cas_days;
		ts_sen.ts_perm_years = perm_years;
		ts_sen.ts_perm_days = perm_days;
		ts_sen.ts_sick_bal = bal_sic;
		ts_sen.ts_vac_bal = bal_vac;
	
		retval = put_ts_sen(&ts_sen,ADD,e_mesg);
		if(retval < 0 && retval != DUPE){
			return(retval);	
		}
		if(retval != DUPE) {
			commit(e_mesg);
		}
	}/*end of for loop*/
	seq_over(EMPLOYEE);

	if(sortop3[0] == 'D'){
		ts_sen.ts_total_years = HV_SHORT;
		ts_sen.ts_total_days = HV_DOUBLE;
		strcpy(ts_sen.ts_emp_numb,"ZZZZZZZZZZZZ");

		if(sortop2[0] == 'B') 
			strcpy(ts_sen.ts_barg,"ZZZZZZ");
		if(sortop2[0] == 'P') 
			strcpy(ts_sen.ts_position,"ZZZZZZ");
		if(sortop2[0] == 'C') 
			strcpy(ts_sen.ts_class,"ZZZZZZ");
		if(sortop2[0] == 'L') 
			ts_sen.ts_cc = HV_SHORT;
		direction = BACKWARD;
	}
	else{
		ts_sen.ts_total_years = LV_SHORT;
		ts_sen.ts_total_days = LV_DOUBLE;
		ts_sen.ts_emp_numb[0] = '\0';

		if(sortop2[0] == 'B') 
			ts_sen.ts_barg[0] = '\0';
		if(sortop2[0] == 'P') 
			ts_sen.ts_position[0] = '\0';
		if(sortop2[0] == 'C') 
			ts_sen.ts_class[0] = '\0';
		if(sortop2[0] == 'L') 
			ts_sen.ts_cc = LV_SHORT;
		direction = FORWARD;

	}
	flg_reset(TS_SEN);

	for(;;) {

		if(sortop2[0] == 'B') {
			strcpy(last_barg,ts_sen.ts_barg);
			retval =get_n_ts_sen(&ts_sen,BROWSE,0,direction,e_mesg);
		}
		if(sortop2[0] == 'P'){ 
			strcpy(last_position,ts_sen.ts_position);
			retval =get_n_ts_sen(&ts_sen,BROWSE,1,direction,e_mesg);
		}
		if(sortop2[0] == 'L'){ 
			last_center = ts_sen.ts_cc;
			retval =get_n_ts_sen(&ts_sen,BROWSE,2,direction,e_mesg);
		}
		if(sortop2[0] == 'C'){
			strcpy(last_class,ts_sen.ts_class);
			retval =get_n_ts_sen(&ts_sen,BROWSE,3,direction,e_mesg);
		}

		if(retval < 0) {
			break;
		} 
		strcpy(emp_rec.em_numb,ts_sen.ts_emp_numb);
		retval=get_employee(&emp_rec,BROWSE,0,e_mesg);
		if(retval < 0){ 
			fomen(e_mesg);
			get();
		}

	  	retval=UsrBargVal(BROWSE,emp_rec.em_numb,emp_rec.em_barg,0,
									e_mesg);
	  	if(retval < 0)
			continue;
		
		teacher = 0;
		/* SJ teacher codes
		if(strcmp(emp_rec.em_class,"  0101") == 0) 
			teacher = 1;
		if(strcmp(emp_rec.em_class," 01012") == 0)
			teacher = 1;
		if(strcmp(emp_rec.em_class," 01013") == 0)
			teacher = 1;
		if(strcmp(emp_rec.em_class," 01014") == 0)
			teacher = 1;
		if(strcmp(emp_rec.em_class," 01015") == 0) 
			teacher = 1;
		if(strcmp(emp_rec.em_class," 01016") == 0) 
			teacher = 1;
		if(strcmp(emp_rec.em_class," 01017") == 0)
			teacher = 1;
		if(strcmp(emp_rec.em_class," 01018") == 0)
			teacher = 1; */

		/* Fredericton teacher codes */
		if(strncmp(emp_rec.em_class,"0101",4) == 0)
			teacher = 1;

		if(teacher == 1) {
			sch_rec.sc_numb = emp_rec.em_cc;

			retval = get_sch(&sch_rec,BROWSE,0,e_mesg);
			if (retval < 0){
				fomer(e_mesg);get();
			}
		}
		else {
			GetClassDesc();
		}

		tot_years = ts_sen.ts_total_years;
	 	tot_days = ts_sen.ts_total_days;
		cas_years = ts_sen.ts_cas_years;
		cas_days = ts_sen.ts_cas_days;
		perm_years = ts_sen.ts_perm_years;
		perm_days = ts_sen.ts_perm_days;
		bal_sic = ts_sen.ts_sick_bal;
		bal_vac = ts_sen.ts_vac_bal;

		if(sortop2[0] == 'B') {
			if(strcmp(last_barg,ts_sen.ts_barg)!=0
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
			if(strcmp(last_position,ts_sen.ts_position)!=0 
			  || first_time == 0 || linecnt > PG_SIZE) { 
				retval = PrntHdg();
				if(retval < 0) 	{
					return(retval);	
				}
				if(retval == EXIT)	break;
				first_time = 1;
			}
		}
		if(sortop2[0] == 'L') {
			if(last_center != ts_sen.ts_cc 
			  || first_time == 0 || linecnt > PG_SIZE) { 
				retval = PrntHdg();
				if(retval < 0) 	{
					return(retval);	
				}
				if(retval == EXIT)	break;
				first_time = 1;
			}
		}
		
		if(sortop2[0] == 'C'){
			if(strcmp(last_class,ts_sen.ts_class)!=0 || 
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

		last_center = ts_sen.ts_cc;
		strcpy(last_barg,ts_sen.ts_barg);
		strcpy(last_position,ts_sen.ts_position);
		strcpy(last_class,ts_sen.ts_class);

	}
	seq_over(TS_SEN);
}

/******************************************************************************
Prints the headings of the report
******************************************************************************/
static
PrntHdg()	/* Print heading  */
{
	short	offset;
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
	mkln((LNSZ-17)/2,"SENIORITY LISTING", 17 );
#else
	mkln((LNSZ-25)/2,"TRANSLATE        ", 17 );
#endif
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

	mkln(3,"EMPLOYEE",13);
	mkln(24,"EMPLOYEE",13);
	mkln(40,"CLASSIFICATION DESC/",20);
	mkln(62,"DATE OF",7);
	mkln(73,"DATE OF",7);
	mkln(84,"CASUAL",6);
	mkln(95,"PERMA",5);
	mkln(105,"TOTAL",5);
	mkln(116,"SICK",4);
	mkln(123,"VACATION",8);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(4,"NUMBER",6);
	mkln(26,"NAME",4);
	mkln(42,"TEACHER LOCATION",16); 
	mkln(62,"HIRE(CA)",8);
	mkln(73,"HIRE(FT)",8);
	mkln(83,"YRS DAYS",8);
	mkln(93,"YRS DAYS",8);
	mkln(103,"YRS DAYS",8);
	mkln(120,"DAYS",4);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);

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
	mkln(14,txt_line,25);
	if(teacher == 1) {
		mkln(40,sch_rec.sc_name,20);
	}
	else {
		mkln(40,class.c_desc,20);
	}
	if (emp_rec.em_st_dt_ca > 0){
		sprintf(txt_line,"%ld",emp_rec.em_st_dt_ca);
		mkln(61,txt_line,4);
		mkln(65,"/",1);
		mkln(66,txt_line+4,2);
		mkln(68,"/",1);
		mkln(69,txt_line+6,2);
	}
	else if (emp_rec.em_st_dt_su > 0){
		sprintf(txt_line,"%ld",emp_rec.em_st_dt_su);
		mkln(61,txt_line,4);
		mkln(65,"/",1);
		mkln(66,txt_line+4,2);
		mkln(68,"/",1);
		mkln(69,txt_line+6,2);
	}
	if(emp_rec.em_st_dt_ft > 0){
		sprintf(txt_line,"%ld",emp_rec.em_st_dt_ft);
		mkln(72,txt_line,4);
		mkln(76,"/",1);
		mkln(77,txt_line+4,2);
		mkln(79,"/",1);
		mkln(80,txt_line+6,2);
	}
	else if(emp_rec.em_st_dt_pt > 0){
		sprintf(txt_line,"%ld",emp_rec.em_st_dt_pt);
		mkln(72,txt_line,4);
		mkln(76,"/",1);
		mkln(77,txt_line+4,2);
		mkln(79,"/",1);
		mkln(80,txt_line+6,2);
	}

	tedit((char*)&cas_years,"__",txt_line,R_SHORT);
	mkln(83,txt_line,2);
	tedit((char*)&cas_days,"___.__",txt_line,R_DOUBLE); 
	mkln(86,txt_line,6);
	tedit((char*)&perm_years,"__",txt_line,R_SHORT);
	mkln(93,txt_line,2);
	tedit((char*)&perm_days,"___.__",txt_line,R_DOUBLE);
	mkln(96,txt_line,6);
	tedit((char*)&tot_years,"__",txt_line,R_SHORT);
	mkln(103,txt_line,2);
	tedit((char*)&tot_days,"___.__",txt_line,R_DOUBLE);
	mkln(106,txt_line,6);

	tedit((char*)&bal_sic,"___.__-",txt_line,R_DOUBLE);
	mkln(115,txt_line,7);
	tedit((char*)&bal_vac,"___.__-",txt_line,R_DOUBLE);
	mkln(124,txt_line,7);

	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	return(NOERROR);
}
/*--------------------------------------------------------------------------*/
static
GetClassDesc()
{
	/*
	************* Get Class desc for each employee ********************
	*/
	strcpy(class.c_code,emp_rec.em_class);
	class.c_date = get_date();

	flg_reset(CLASSIFICATION);
      	retval=get_n_class(&class,BROWSE,0,BACKWARD,e_mesg);
	if(retval==EFL){	
	        class.c_desc[0]='\0';
		return(ERROR);
	}

	if(retval<0){
	        class.c_desc[0]='\0';
		return(ERROR);
	}

	if(strcmp(class.c_code,emp_rec.em_class)!=0){
	        class.c_desc[0]='\0';
		return(ERROR);
	}
	seq_over(CLASSIFICATION);

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
/*--------------------------------------------------------------------
 CalcBal()     - this routine reads the attendance information and   
   		 copies it onto the screen
--------------------------------------------------------------------*/
static
CalcBal()
{
	int 	retval, i;

	bal_vac = emp_rec.em_vac_ent;
	bal_sic = emp_rec.em_sic_ent;
	for (i=0; i < 12; i++) {
		bal_sic += emp_rec.em_sck_acc[i];
		bal_vac += emp_rec.em_vac_acc[i];
	}

	strcpy(att_hist.eah_numb, emp_rec.em_numb);
	if(pay_param.pr_st_mth == 1){
		att_hist.eah_date = pay_param.pr_cal_st_dt;  
	}
	if(pay_param.pr_st_mth == 2){
		att_hist.eah_date = pay_param.pr_fisc_st_dt;  
	}
	if(pay_param.pr_st_mth == 3){
		att_hist.eah_date = pay_param.pr_schl_st_dt;  
	}
	flg_reset(EMP_ATT);

	for(;;){
		retval = get_n_emp_at(&att_hist, BROWSE, 0, FORWARD, e_mesg);

		if(retval == EFL ||		
		   (strcmp(att_hist.eah_numb, emp_rec.em_numb)) != 0){
			break;
		}
		if(retval < 0) {
			fomen(e_mesg);
			get();
			return(-1);
		}
		strcpy(att_rec.at_code, att_hist.eah_code);

		retval = get_att(&att_rec,BROWSE,1,e_mesg);
		if(retval < 0)  {
			fomen(e_mesg);
			get();
			return(retval);
		}
		if(strcmp(att_rec.at_sick, "Y") == 0){
			if(strcmp(att_rec.at_code, "S11"))
				bal_sic -= .5;
			else if(strcmp(att_rec.at_code, "S12"))
				bal_sic -= 1;
			else
				bal_sic -= 
		      		(att_hist.eah_hours/att_hist.eah_sen_hours);
		}
		if(strcmp(att_rec.at_vac, "Y") == 0){
		        bal_vac -= 
		   		(att_hist.eah_hours/att_hist.eah_sen_hours);
		}
	}
	seq_over(EMP_ATT);

	strcpy(barg_unit.b_code,emp_rec.em_barg);
	barg_unit.b_date = get_date();
	flg_reset(BARG);

	retval = get_n_barg(&barg_unit,BROWSE,0,BACKWARD,e_mesg);
	if(retval < 0 || strcmp(barg_unit.b_code,emp_rec.em_barg)!=0){
		fomer("Error Reading Bargaining Unit File");
		return(-1);
	}
	seq_over(BARG);

	if(bal_sic > barg_unit.b_sick_max)
		bal_sic = barg_unit.b_sick_max;

	return(NOERROR);
}
/******************   END OF PROGRAM *******************************/
