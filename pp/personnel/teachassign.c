/******************************************************************************
		Sourcename   : teachassign.c
		System       : 
		Module       :
		Created on   : 92-SEP-18
		Created  By  : m. galvin 
******************************************************************************
About the file:	
	This program lists the teacher's assignments.

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

static Emp		emp_rec;
static Emp_sched1	emp_sched1;
static Pa_rec		pa_rec;
static Sch_rec		sch_rec;
static Tmp_sched1	tmp_sched1;
static Pay_param	param_rec;
static Barg_unit	barg_unit;
static Position		position;
static Class		class;
static Teach_ass	teach_ass;
static Area_spec	area_spec;

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
static long	date2;

static int	flag;		/* flag to print the employee or not */
static int	PG_SIZE;
static int	retval;
static char 	discfile[15];	/* for storing outputfile name */
static short	pgcnt; 		/* for page count */
static char	resp[2];	/* for storing response */
static short	copies;

extern char 	e_mesg[200];	/* for storing error messages */

/* Report Totals */
char	prn_flag[2];		/*flag to determine if emp nam # to be printed*/
static	char	sched1_flag[2];

teachassign()
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
			STRCPY( discfile, "teachassgn.dat" );
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

	if(sortop2[0]=='B'||sortop2[0]=='P'){ 
		unlink_file(TMPINDX_1);
		strcpy(tmpindxfile,"emptemp");
		get_tnum(tnum);
		strcat(tmpindxfile,tnum);
		if(CreatTempIndx(tmpindxfile,sortop1,sortop2 )<0){
			return(1);
		}
	}
	if(sortop2[0]=='C'||sortop2[0]=='L'){
		unlink_file(TMP_SCHED1);/*remove temp file*/
		if(BldEmpResp()<0){
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
	char	tmp_code[13],last_barg[7],last_position[7],last_class[7];
	short	last_center;
	int	first_time = 0;

	/* get the cost center name */
	retval = get_param(&pa_rec,BROWSE,1,e_mesg);
	if (retval < 0){
		fomer(e_mesg);get ();
	}

	retval = get_pay_param(&param_rec,BROWSE,1,e_mesg);
	if(retval<0){
		fomer(e_mesg);get();
	}

	if(sortop2[0]=='C'||sortop2[0]=='L'){
		tmp_sched1.tm_numb[0] = '\0';	
		tmp_sched1.tm_class[0] = '\0';	
		tmp_sched1.tm_cost = 0;	
		last_center = tmp_sched1.tm_cost;
		tmp_sched1.tm_sortk_1[0] = '\0';
		tmp_sched1.tm_sortk_2[0] = '\0';
		tmp_sched1.tm_sortk_3[0] = '\0';
		flg_reset(TMP_SCHED1);
		for(;;){
			retval=get_n_tmp_sched1(&tmp_sched1,BROWSE,1,FORWARD,
									e_mesg);
			if(retval == EFL){break;}
			if(retval<0){
				break;
			}

			strcpy(emp_rec.em_numb,tmp_sched1.tm_numb);

			retval=get_employee(&emp_rec,BROWSE,0,e_mesg);
			if(retval<0){
				sprintf(e_mesg,"Error %d on reading employee",
					retval);
				fomer(retval); 
				continue;
			}
	  		retval=UsrBargVal(BROWSE,emp_rec.em_numb,
						emp_rec.em_barg,0,e_mesg);
	  		if(retval < 0)
				continue;

			/*
			*** get the teacher's assignments ****
			*/
			tmp_code[0] ='\0';
			strcpy(teach_ass.tc_numb,emp_rec.em_numb);
			if(sortop2[0] == 'L')
				teach_ass.tc_cost = tmp_sched1.tm_cost;
			else
				teach_ass.tc_cost = 0;
			teach_ass.tc_ar_sp[0] = '\0';
			flg_reset(TEACH_ASS);

			for (;;){
				retval=get_n_teach_ass(&teach_ass,BROWSE,0,
									FORWARD,
									e_mesg);
				if(retval==EFL)		break;

				if(retval<0){
					sprintf(e_mesg,"Error %d reading recs",
						retval);
					fomer(e_mesg);get();
					break;
				}
	
				if(strcmp(teach_ass.tc_numb,emp_rec.em_numb)!=0)
					break;

				if(sortop2[0] == 'L'){
					if(teach_ass.tc_cost != 
					   tmp_sched1.tm_cost)
						break;
				}
				prn_flag[0] = 'Y';

				if(strcmp(teach_ass.tc_numb,tmp_code) == 0)
					prn_flag[0] = 'N';
			
				if(sortop2[0] == 'C'){
				  if(strcmp(last_class,tmp_sched1.tm_class)!=0||
					first_time == 0 || linecnt > PG_SIZE) { 
					retval = PrntHdg();
					if(retval < 0) 	{
						return(retval);	
					}
					if(retval == EXIT)	break;
					first_time = 1;
				  }
				}
				if(sortop2[0] == 'L'){
				  if(last_center != tmp_sched1.tm_cost ||
			   	     first_time == 0 || linecnt > PG_SIZE) { 
					retval = PrntHdg();
					if(retval < 0) 	{
						return(retval);	
					}
					if(retval == EXIT)	return(retval);
					first_time = 1;
				  }
				}

				if ((retval = PrntRec(prn_flag))<0)
					return(retval);
				if(retval==EXIT)	return(retval);
				last_center = tmp_sched1.tm_cost;
				strcpy(tmp_code,emp_rec.em_numb);
				strcpy(last_class,tmp_sched1.tm_class);
			}/*end of for loop*/

			seq_over(TEACH_ASS);

			if(strcmp(emp_rec.em_numb,tmp_code) == 0){
				if(prnt_line() < 0 )	return(REPORT_ERR);
			}

		}/*end of for loop*/

		seq_over(TMP_SCHED1);

	}/*end check which files used*/
	/*
		*********** Sort by Bargain or Position *************
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
			retval = get_next((char*)&emp_rec,TMPINDX_1,0,
							FORWARD,BROWSE,e_mesg);
			if(retval==EFL)		break;
			if(retval<0){
				sprintf(e_mesg,"%d",retval);
				fomer(e_mesg);get();
				break;
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

			if( (emp_rec.em_cc < center1)  ||
		   	    (emp_rec.em_cc > center2) ) 
				continue;

			if(strcmp(emp_rec.em_class,class1) < 0 ||
		   	   strcmp(emp_rec.em_class,class2) > 0 ) 
				continue;

			/*
			*** get the teacher's assignment ***
			*/
			tmp_code[0] ='\0';
			strcpy(teach_ass.tc_numb,emp_rec.em_numb);
			teach_ass.tc_cost = 0;
			teach_ass.tc_ar_sp[0] = '\0';
			flg_reset(TEACH_ASS);

			for (;;){
				retval=get_n_teach_ass(&teach_ass,BROWSE,0,
							     FORWARD,e_mesg);
				if(retval==EFL)		break;
				if(retval<0){
					sprintf(e_mesg,"%d",retval);
					fomer(e_mesg);get();
					break;
				}

				if(strcmp(teach_ass.tc_numb,emp_rec.em_numb)!=0)
					break;

				prn_flag[0] = 'Y';
				if(strcmp(teach_ass.tc_numb,tmp_code) == 0)
					prn_flag[0] = 'N';
				
				if(sortop2[0] == 'B') {
				  if(strcmp(last_barg,emp_rec.em_barg)!=0
				   || first_time == 0 || linecnt > PG_SIZE) {
					retval = PrntHdg();
					if(retval < 0){
						return(retval);
					}
					if(retval == EXIT)	return(retval);
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
					if(retval == EXIT)	return(retval);
					first_time = 1;
				  }
				}

				if ((retval = PrntRec(prn_flag))<0)
					return(retval);
				if(retval==EXIT)	return(retval);

				strcpy(tmp_code,emp_rec.em_numb);
				strcpy(last_barg,emp_rec.em_barg);
				strcpy(last_position,emp_rec.em_pos);

			} /*end of endless for */

			seq_over(EMP_EXTRA);

			if(strcmp(emp_rec.em_numb,tmp_code) == 0){
				if(prnt_line() < 0 )	return(REPORT_ERR);
			}

		} /*end of endless for loop*/
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
	mkln((LNSZ-30)/2,"LIST OF EMPLOYEE'S ASSIGNMENTS",30 );
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

	mkln(2,"COST",4);
	mkln(14,"AREA OF SPECIALIZATION",22);
	mkln(46,"GRADE",5);
	mkln(52,"PERCENTAGE",10);
	mkln(73,"COURSE",6);
	mkln(101,"SEMESTER",8);
	mkln(110,"SECTION",7);
	mkln(118,"ROOM",4);
	mkln(123,"NUMBER OF",9);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	mkln(1,"CENTER",6);
	mkln(123,"STUDENTS",8);
	
	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	return(NOERROR);
}
/*------------------------------------------------------------------*/
static
PrntRec(prn_flag)
char	prn_flag[2];
{
	char	txt_line[132];

	if(prn_flag[0]=='Y'){
		mkln(1,"EMPLOYEE :",10);
		mkln(11,emp_rec.em_numb,12);
		sprintf(txt_line,"%s, %s %s",
			emp_rec.em_last_name,
			emp_rec.em_first_name,
			emp_rec.em_mid_name);
		mkln(25,txt_line,40);
		if(prnt_line() < 0 )	return(REPORT_ERR);
		if(prnt_line() < 0 )	return(REPORT_ERR);
	}

	tedit((char*)&teach_ass.tc_cost,"__0_",txt_line,R_SHORT);
	mkln(2,txt_line,4);
	mkln(7,teach_ass.tc_ar_sp,6);
	
	strcpy(area_spec.ar_code,teach_ass.tc_ar_sp);
	flg_reset(AREA_SPEC);
	retval = get_area_spec(&area_spec,BROWSE,0,e_mesg);
	if(retval < 0) {
		fomen(e_mesg);
		get();
	}	
	else
		mkln(14,area_spec.ar_desc,30);
	tedit((char*)&teach_ass.tc_grade,"_0",txt_line,R_SHORT);
	mkln(47,txt_line,2);
	tedit((char*)&teach_ass.tc_perc,"__0.__",txt_line,R_DOUBLE);
	mkln(54,txt_line,6);
	mkln(63,teach_ass.tc_crs,6);
	tedit((char*)&teach_ass.tc_sem,"_0",txt_line,R_SHORT);
	mkln(104,txt_line,2);
	tedit((char*)&teach_ass.tc_sec,"_0",txt_line,R_SHORT);
	mkln(112,txt_line,2);
	mkln(117,teach_ass.tc_room,4);
	tedit((char*)&teach_ass.tc_load,"__0",txt_line,R_SHORT);
	mkln(126,txt_line,3);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	return(NOERROR);
}
/*-------------------------------------------------------------------------*/
static
BldEmpResp()
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

		strcpy(teach_ass.tc_numb,emp_rec.em_numb);
		teach_ass.tc_cost = 0;
		teach_ass.tc_ar_sp[0] = '\0';
		flg_reset(TEACH_ASS);

		for(;;) {
			retval = get_n_teach_ass(&teach_ass,BROWSE,0,FORWARD,
								e_mesg);
			
			if (retval < 0)
				break;	

			if(strcmp(emp_rec.em_numb,teach_ass.tc_numb) != 0) {
				sched1_flag[0] = 'N';
				break;
			}

			if( (teach_ass.tc_cost < center1) || 
		   	(teach_ass.tc_cost > center2) )
				continue;
		
			sched1_flag[0] = 'Y'; /*sched rec found*/
			if((retval = PutTmpAdd()) <0)
				break;
		}/* end check sched1 rec loop */
		seq_over(TEACH_ASS);

		if( (emp_rec.em_cc < center1) || 
		 (emp_rec.em_cc > center2) )
			continue;

		if(strcmp(emp_rec.em_class,class1) < 0 ||
	         strcmp(emp_rec.em_class,class2) > 0 ) 
			continue;

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
		if(sched1_flag[0] == 'Y'){
			sprintf(sort_array,"%-d",teach_ass.tc_cost);
			strncpy(tmp_sched1.tm_sortk_1,sort_array,4);
		}
		else {
			sprintf(sort_array,"%-d",emp_rec.em_cc);
			strncpy(tmp_sched1.tm_sortk_1,sort_array,4);
		}
	}
	else{
		strncpy(tmp_sched1.tm_sortk_1,emp_rec.em_class,6);
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
	strncpy(tmp_sched1.tm_class,emp_rec.em_class,6);
	if(sched1_flag[0] == 'Y')
		tmp_sched1.tm_cost = teach_ass.tc_cost;
	else
		tmp_sched1.tm_cost = emp_rec.em_cc;

	strcpy(tmp_sched1.tm_dept,teach_ass.tc_crs);
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
