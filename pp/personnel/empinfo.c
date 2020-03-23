/******************************************************************************
		Sourcename   : empinfo.c
		System       : 
		Module       :
		Created on   : 92-02-25
		Created  By  : Littlejohn 

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
#else
#define PRINTER		'I'

#define DISPLAY		'A'
#define FILE_IO		'D'
#endif

static Emp		emp_rec, 	tmp_rec;
static Emp_sched1	emp_sched1;
static Pa_rec		pa_rec;
static Sch_rec		sch_rec;
static Tmp_sched1	tmp_sched1;
static Dept		dept_rec;
static Area		area_rec;
static Position		pos_rec;
static Class		class;
static Barg_unit	barg_rec;

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

static	char	sched1_flag[2];

static int	flag;		/* flag to print the employee or not */
static int	PG_SIZE;
static int	retval;
static char 	discfile[15];	/* for storing outputfile name */
static short	pgcnt; 		/* for page count */
static char	resp[2];	/* for storing response */
static short	copies;

extern char 	e_mesg[200];	/* for storing error messages */
char		cc_name[40];	/* display field for the cost center name */

empinfo()
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
			STRCPY( discfile, terminal );
			PG_SIZE = 13;
			break;
		case FILE_IO:
			resp[0]='F';
			STRCPY( discfile, "empinfo.dat" );
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

	if(sortop2[0]=='B' || sortop2[0]=='P') {
		unlink_file(TMPINDX_1);
		STRCPY(tmpindxfile,"emptemp");
		get_tnum(tnum);
		strcat(tmpindxfile,tnum);
		if(CreatTempIndx(tmpindxfile,sortop1,sortop2 )<0){
			return (-1);
		}
	}
	if(sortop2[0]=='C'||sortop2[0]=='L'){
		unlink_file(TMP_SCHED1);/*remove temp file*/
		if(BldEmpInfo()<0){
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
	short	last_center;
	char	last_numb[13],last_barg[7],last_pos[7],last_class[7];
	int	first_time = 0;

	retval = get_param(&pa_rec,BROWSE,1,e_mesg);
	if (retval < 0){
		fomer(e_mesg);get ();
	}
	/*
		******* Sort by Center *******
	*/
	if(sortop2[0]=='C'||sortop2[0]=='L') {
		tmp_sched1.tm_cost = 0;	
		tmp_sched1.tm_class[0] = '\0';
		strcpy(last_class,tmp_sched1.tm_class);
		tmp_sched1.tm_sortk_1[0] = '\0';
		tmp_sched1.tm_sortk_2[0] = '\0';
		tmp_sched1.tm_sortk_3[0] = '\0';
		last_center = tmp_sched1.tm_cost;
		strcpy(last_numb,tmp_sched1.tm_numb);
		flg_reset(TMP_SCHED1);

		for(;;){
			retval=get_n_tmp_sched1(&tmp_sched1,BROWSE,1,
							FORWARD,e_mesg);
			if(retval == EFL){break;}
			if(retval<0){
				return(-1);
			}

			strcpy(emp_rec.em_numb,tmp_sched1.tm_numb);

			retval=get_employee(&emp_rec,BROWSE,0,e_mesg);
			if(retval < 0) {
				fomer(e_mesg);
				get();
				continue;
			}
	  		retval=UsrBargVal(BROWSE,emp_rec.em_numb,
						emp_rec.em_barg,0,e_mesg);
	  		if(retval < 0)
				continue;
		
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

				if((strcmp(last_numb,tmp_sched1.tm_numb)!= 0)|| 
			   	   (strcmp(last_class,tmp_sched1.tm_class)!=0)){
					sched1_flag[0] = 'N';

					if ((retval = GetEmpInfo())<0)
						return(retval);

					if ((retval = PrntRec())<0)
						return(retval);

					if(retval == EXIT)	break;
					strcpy(last_class,emp_rec.em_class);
					strcpy(last_numb,tmp_sched1.tm_numb);
					
				}
			}
			if(sortop2[0] == 'L'){
				if(last_center != tmp_sched1.tm_cost || 
			   	   first_time == 0 || linecnt > PG_SIZE) { 
					retval = PrntHdg();
					if(retval < 0) 	{
						return(retval);	
					}
				}
				if(retval == EXIT)	break;
				first_time = 1;

				if((strcmp(last_numb,tmp_sched1.tm_numb)!= 0)|| 
			   	   (last_center != tmp_sched1.tm_cost)) {
					sched1_flag[0] = 'N';

					if ((retval = GetEmpInfo())<0)
						return(retval);

					if ((retval = PrntRec())<0)
						return(retval);

					if(retval == EXIT)	break;
					last_center = tmp_sched1.tm_cost;
					strcpy(last_numb,tmp_sched1.tm_numb);
				}
			}

			emp_sched1.es_week = tmp_sched1.tm_week;	
			emp_sched1.es_fund = tmp_sched1.tm_fund;	
			strcpy(emp_sched1.es_class,tmp_sched1.tm_class);
			emp_sched1.es_cost = tmp_sched1.tm_cost;	
			strcpy(emp_sched1.es_dept,tmp_sched1.tm_dept);
			strcpy(emp_sched1.es_area,tmp_sched1.tm_area);
			strcpy(emp_sched1.es_numb,emp_rec.em_numb);
			flg_reset(EMP_SCHED1);
			retval = get_emp_sched1(&emp_sched1,BROWSE,0,e_mesg);
			if(retval < 0) {
				if(retval == UNDEF) continue;
				fomer(e_mesg);
				get();
				continue;
			}
			sched1_flag[0] = 'Y';

			if ((retval = GetEmpInfo())<0)
				return(retval);

			if ((retval = PrntSchedRec())<0)
				return(retval);

			seq_over(EMP_SCHED1);

			if(retval == EXIT)	break;

		}/*end of for loop*/

		seq_over(TMP_SCHED1);

	}/*end check which files used*/
	/*
		******** Sort by Bargain and Position *******
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
			strcpy(last_pos,emp_rec.em_pos);
			emp_rec.em_barg[0] = '\0';
		}
		strcpy(last_pos,emp_rec.em_pos);
		strcpy(last_barg,emp_rec.em_barg);

		flg_reset(TMPINDX_1);

		for (;;){
			retval = get_next((char*)&emp_rec,TMPINDX_1,0,
							FORWARD,BROWSE,e_mesg);

			if(retval == EFL) break;
			if(retval<0){
				fomer(e_mesg);
			}
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
				if(strcmp(last_pos,emp_rec.em_pos)!=0 ||
				   first_time == 0 || linecnt > PG_SIZE) { 
					retval = PrntHdg();
					if(retval < 0) 	{
						return(retval);	
					}
					if(retval == EXIT)	break;
					first_time = 1;
				}
			}
			sched1_flag[0] = 'N';

			if ((retval = GetEmpInfo())<0)
				return(retval);

			if ((retval = PrntRec())<0)
				return(retval);

			if(retval == EXIT)	break;

			strcpy(last_pos,emp_rec.em_pos);
			strcpy(last_barg,emp_rec.em_barg);

			emp_sched1.es_week = 0;	
			emp_sched1.es_fund = 0;	
			emp_sched1.es_class[0] = '\0';	
			emp_sched1.es_cost = 0;	
			emp_sched1.es_dept[0] = '\0';
			emp_sched1.es_area[0] = '\0';
			strcpy(emp_sched1.es_numb,emp_rec.em_numb);
			flg_reset(EMP_SCHED1);
			for(;;) {
				retval = get_n_emp_sched1(&emp_sched1,BROWSE,
							0,FORWARD,e_mesg);
				if(retval < 0)	break;

				if(strcmp(emp_rec.em_numb,
						emp_sched1.es_numb) !=0) 
					break;

				if( (emp_sched1.es_cost < center1)  ||
		   	    	    (emp_sched1.es_cost > center2) ) 
					continue;

				if( strcmp(emp_sched1.es_class,class1) < 0  ||
		   	    	    strcmp(emp_sched1.es_class,class2) > 0 ) 
					continue;

				sched1_flag[0] = 'Y';

				if ((retval = GetEmpInfo())<0)
					return(retval);

				if ((retval = PrntSchedRec())<0)
					return(retval);

				if(retval == EXIT)	break;
			}
			seq_over(EMP_SCHED1);
			if(retval == EXIT)	break;
		}/*End of the endless loop*/

		seq_over(TMPINDX_1);
	}

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
	sysdt = get_date();

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
	mkln((LNSZ-20)/2,"EMPLOYEE INFORMATION", 20 );
#else
	mkln((LNSZ-25)/2,"TRANSLATE        ", 17 );
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
		mkln(1,"BARGAINING UNIT: ",17); 
		mkln(18,emp_rec.em_barg,6);
		strcpy(barg_rec.b_code,emp_rec.em_barg);
		barg_rec.b_date = get_date();
		flg_reset(BARG);

		retval = get_n_barg(&barg_rec,BROWSE,0,BACKWARD,e_mesg);
		if(retval < 0){
			fomer(e_mesg);get();
		}
		if(strcmp(barg_rec.b_code,emp_rec.em_barg)==0) {
			mkln(25,barg_rec.b_name,30);
		}
		seq_over(BARG);
	}
	if(sortop2[0] == 'P' ) {
		strcpy(pos_rec.p_code,emp_rec.em_pos);
		retval = get_position(&pos_rec,BROWSE,0,e_mesg);
		if(retval < 0){
			fomer(e_mesg);get();
		}
		mkln(1,"POSITION: ",10);
		mkln(11,emp_rec.em_pos,6);
		mkln(18,pos_rec.p_desc,30);
	}
	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	return(NOERROR);
}
/*------------------------------------------------------------------*/
static
PrntRec()
{
	char	txt_line[132];

	mkln(1,"EMPLOYEE NUMBER:",16);
	mkln(18,emp_rec.em_numb,12);
	if(prnt_line()<0)	return(REPORT_ERR);

	mkln(1,"EMPLOYEE NAME  :",16);
	txt_line[0] = '\0';
	sprintf(txt_line,"%s, %s %s",
		emp_rec.em_last_name,
		emp_rec.em_first_name,
		emp_rec.em_mid_name);
	mkln(18,txt_line,40);
	if(prnt_line()<0)	return(REPORT_ERR);
	if(prnt_line()<0)	return(REPORT_ERR);

	mkln(1,"BARGAINING UNIT:",16);
	mkln(18,emp_rec.em_barg,6);
	mkln(27,"DESC:",5);
	mkln(33,barg_rec.b_name,30);
	mkln(71,"PERCENTAGE:",11);
	sprintf(txt_line,"%3.3f",emp_rec.em_perc);
	mkln(83,txt_line,6);
	if(prnt_line()<0)	return(REPORT_ERR);

	mkln(1,"POSITION CODE  :",16);
	mkln(18,emp_rec.em_pos,6);
	mkln(27,"DESC:",5);
	mkln(33,pos_rec.p_desc,30);
	mkln(71,"CERTIFICATION:",14);
	mkln(86,emp_rec.em_cert,6);
	mkln(95,"LEVEL :",7);
	txt_line[0] = '\0';
	mkln(103,emp_rec.em_level,2);
	if(prnt_line()<0)	return(REPORT_ERR);
	mkln(1,"CLASS CODE     :",15);
	mkln(18,emp_rec.em_class,6);
	mkln(27,"DESC:",5);
	mkln(33,class.c_desc,30);

	if(prnt_line()<0)	return(REPORT_ERR);
	if(prnt_line()<0)	return(REPORT_ERR);

	return(NOERROR);
}
/*----------------------------------------------------------------------*/
static
PrntSchedRec()
{
	char	txt_line[132];
	int	unit_col=0;
	int	x=0;

	mkln(1,"WEEK",4);
	mkln(24,"UNITS/DAY",9);
	mkln(56,"CODE",4);
	mkln(81,"DESCRIPTION",11);
	if(prnt_line()<0)	return(REPORT_ERR);

	mkln(9,"1",1);
	mkln(16,"2",1);
	mkln(23,"3",1);
	mkln(30,"4",1);
	mkln(37,"5",1);
	mkln(44,"6",1);
	mkln(51,"7",1);
	if(prnt_line()<0)	return(REPORT_ERR);

	sprintf(txt_line,"%d",emp_sched1.es_week);
	mkln(2,txt_line,1);
	sprintf(txt_line,"%d",emp_sched1.es_fund);
	mkln(4,txt_line,1);
	unit_col = 7;
	for(x=0;x<7;x++,unit_col+=7){
	        tedit((char*)&emp_sched1.es_units[x],"__.__",txt_line,R_DOUBLE);
		mkln(unit_col,txt_line,5);
	}
	mkln(56,"COST CENTER:",12);
	sprintf(txt_line,"%4d",emp_sched1.es_cost);
	mkln(70,txt_line,4);
	mkln(81,sch_rec.sc_name,28);
	if(prnt_line()<0)	return(REPORT_ERR);

	mkln(56,"DEPARTMENT :",12);
	mkln(70,emp_sched1.es_dept,6);
	mkln(81,dept_rec.d_desc,30);
	if(prnt_line()<0)	return(REPORT_ERR);

	mkln(56,"AREA       :",12);
	mkln(70,emp_sched1.es_area,6);
	mkln(81,area_rec.a_desc,30);
	if(prnt_line()<0)	return(REPORT_ERR);

	mkln(56,"CLASS      :",12);
	mkln(70,emp_sched1.es_class,6);
	mkln(81,class.c_desc,30);
	if(prnt_line()<0)	return(REPORT_ERR);

	mkln(56,"AMOUNT     :",12);
	sprintf(txt_line,"%3.3f",emp_sched1.es_amount);
	mkln(70,txt_line,10);
	if(prnt_line()<0)	return(REPORT_ERR);
	if(prnt_line()<0)	return(REPORT_ERR);
	if(prnt_line()<0)	return(REPORT_ERR);
		
	if(linecnt > PG_SIZE)
		if((retval=PrntHdg()) == EXIT)
			return(retval);

	return(NOERROR);
}
/*----------------------------------------------------------------------*/
GetEmpInfo ()
{
	long	sys_date;

	/* Get the system date */
	sys_date = get_date();

	/*Get bargain desc*/
	strcpy(barg_rec.b_code,emp_rec.em_barg);
	barg_rec.b_date = sys_date;
	flg_reset(BARG);
	for (;;){
		retval = get_n_barg(&barg_rec,BROWSE,0,BACKWARD,e_mesg);
		if(strcmp(barg_rec.b_code,emp_rec.em_barg)==0)
			break;
		if(retval < 0 )	{
			strcpy(barg_rec.b_name,
				"                              ");
			break;
		}
	}
	seq_over(BARG);

	/*Get position description*/
	strcpy(pos_rec.p_code,emp_rec.em_pos);
	retval = get_position(&pos_rec,BROWSE,0,e_mesg);
	if(retval<0){
		strcpy(pos_rec.p_desc,
				"                              ");
	}

	/*Get cost center description*/	
	if(sched1_flag[0] == 'N')
		sch_rec.sc_numb = emp_rec.em_cc;
	else
		sch_rec.sc_numb = emp_sched1.es_cost;
	retval = get_sch(&sch_rec,BROWSE,0,e_mesg);
	if(retval<0){
		strcpy(sch_rec.sc_name,
				"                              ");
	}

	/*Get department desc*/
	if(sched1_flag[0] == 'Y') {
		strcpy(dept_rec.d_code,emp_sched1.es_dept);
		retval = get_dept(&dept_rec,BROWSE,0,e_mesg);
		if(retval<0){
			strcpy(dept_rec.d_desc,
					"                              ");
		}
	
		/*Get area desc*/
		strcpy(area_rec.a_deptcode,emp_sched1.es_dept);
		strcpy(area_rec.a_code,emp_sched1.es_area);
		retval = get_area(&area_rec,BROWSE,0,e_mesg);
		if(retval<0){
			strcpy(area_rec.a_desc,
					"                              ");
		}
	}

	/*Get Class desc and the rate*/
	if(sched1_flag[0] == 'N') 
		strcpy(class.c_code,emp_rec.em_class);
	else
		strcpy(class.c_code,emp_sched1.es_class);
	class.c_date = sys_date;
	flg_reset(CLASSIFICATION);
	for (;;){
		retval = get_n_class(&class,BROWSE,0,BACKWARD,e_mesg);
		if(retval==EFL)	break;
		if(sched1_flag[0] == 'N') {
			if(strcmp(class.c_code,emp_rec.em_class)==0)
				break;
		}
		else{
			if(strcmp(class.c_code,emp_sched1.es_class)==0)
				break;
		}
		if(retval<0){
			strcpy(class.c_desc,
				"                              ");
		        class.c_desc[0]='\0';
			break;
		}
	}

	seq_over(CLASSIFICATION);
	return(NOERROR);

}
/*-----------------------------------------------------------------------*/
static
BldEmpInfo ()
{
	int	x;

	/*Get the emp name and dates from the employee file*/

	strcpy(emp_rec.em_numb,empl1);
	flg_reset(EMPLOYEE);
	for (;;){
		retval = get_n_employee(&emp_rec,BROWSE,0,FORWARD,e_mesg);
		if(retval<0){
			break;	
		}
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
			if (retval == EFL)	break;
			if (retval < 0){
				break;	
			}

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
		strcpy(tmp_sched1.tm_dept,emp_sched1.es_dept);
		strcpy(tmp_sched1.tm_area,emp_sched1.es_area);
		tmp_sched1.tm_week = emp_sched1.es_week;
		tmp_sched1.tm_fund = emp_sched1.es_fund;
	}
	else {
		strncpy(tmp_sched1.tm_class,emp_rec.em_class,6);
		tmp_sched1.tm_cost = emp_rec.em_cc;
		tmp_sched1.tm_dept[0] = '\0';
		tmp_sched1.tm_area[0] == '\0';
		tmp_sched1.tm_week = 0;
		tmp_sched1.tm_fund = 0;
	}

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
