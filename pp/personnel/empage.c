/******************************************************************************
		Sourcename   : empage.c
		Module       : Personnel/Payroll
		Created on   : 93-JAN-11
		Created  By  : Eugene Roy

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
short	age1;
long	date1;
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
static	double	tot_days=0;
static	short	age_yrs;

static	char	sched1_flag[2];

static	short	last_center;
static	char	last_barg[7];
static  char	last_class[7];
static	char	last_position[7];

extern char 	e_mesg[200];	/* for storing error messages */

empage()
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
			strcpy( discfile, "empage.dat" );
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

	age1 = 0;
	retval = GetAge(&age1);
	if(retval < 0) return(retval);

	date1 = get_date();
	retval = GetDate1(&date1);
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

	retval = PrintRpt();
	if(retval < 0)	return(-1);

	close_rep();	/* close output file */
	close_dbh();

	return(retval);
}
/*****************************************************************************/
static	
PrintRpt()
{	
	int	first_time = 0, key = 0;
	int	tmp_age;

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

		retval = PutTmpFile();
		if(retval < 0)
			continue;

	}/*end of for loop*/
	seq_over(EMPLOYEE);

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

		if(sortop2[0] == 'B') {
			if(strcmp(last_barg,emp_rec.em_barg)!=0
			  || first_time == 0 || linecnt > PG_SIZE){
				retval = PrntHdg();
				if(retval == UNDEF) continue;
				if(retval < 0){
					return(retval);
				}
				if(retval == EXIT)	break;
				first_time = 1;
			}
		}
		if(sortop2[0] == 'P') {
			if(strcmp(last_position,emp_rec.em_pos)!=0 
			  || first_time == 0 || linecnt > PG_SIZE){
				retval = PrntHdg();
				if(retval == UNDEF) continue;
				if(retval < 0) 	{
					return(retval);	
				}
				if(retval == EXIT)	break;
				first_time = 1;
			}
		}
		if(sortop2[0] == 'L') {
			if(last_center != emp_rec.em_cc 
			  || first_time == 0 || linecnt > PG_SIZE){
				retval = PrntHdg();
				if(retval == UNDEF) continue;
				if(retval < 0) 	{
					return(retval);	
				}
				if(retval == EXIT)	break;
				first_time = 1;
			}
		}
		
		if(sortop2[0] == 'C'){
			if(strcmp(last_class,emp_rec.em_class)!=0 || 
			   first_time == 0 || linecnt > PG_SIZE){
				retval = PrntHdg();
				if(retval == UNDEF) continue;
				if(retval < 0) 	{
					return(retval);	
				}
				if(retval == EXIT)	break;
				first_time = 1;
			}
		}

		tmp_age = (date1 - emp_rec.em_date)/10000;
		age_yrs = tmp_age;
		
		if(age_yrs >= age1){
			if ((retval = PrntRec())<0)
				return(retval);
			if(retval == EXIT)	break;
		}

		last_center = emp_rec.em_cc;
		strcpy(last_barg,emp_rec.em_barg);
		strcpy(last_position,emp_rec.em_pos);
		strcpy(last_class,emp_rec.em_class);
	}
	seq_over(TMP_SCHED1);

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
	mkln((LNSZ-20)/2,"EMPLOYEE AGE LISTING", 20 );
#else
	mkln((LNSZ-18)/2,"TRANSLATE        ", 20 );
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
	mkln(120,"AGE",3);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(4,"NUMBER",6);
	mkln(26,"NAME",4);
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
	mkln(14,txt_line,35);

	tedit((char*)&emp_rec.em_date,"____/__/__",txt_line,R_LONG);
	mkln(51,txt_line,10);

	tedit((char*)&age_yrs,"0_",txt_line,R_SHORT);
	mkln(118,txt_line,2);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);


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
