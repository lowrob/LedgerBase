/******************************************************************************
		Sourcename   : stdate.c
		System       : 
		Module       :
		Created on   : 92-02-22
		Created  By  : Littlejohn 

******************************************************************************
About the file:	
	This program will print a list of employee addressed.

History:
Programmer      Last change on    Details
m. galvin	1992/08/24	  Modified the program since the files used 
				  were changed.  I also added a description
				  of the selected sort option to print
				  at the top of each page.

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
static Barg_unit	barg_unit;
static Position		position;
static Class		class;

static char	sched1_flag[2];

/*  Data items for storing the key range end values */
static char	sortop1[2];
static char	sortop2[2];
static char	barg1[7];
static char	barg2[7];
static char	posi1[7];
static char	posi2[7];
static char	empl1[13];
static char	empl2[13];
static short	center1;	
static short	center2;	
static char	class1[7];
static char	class2[7];

static int	flag;		/* flag to print the employee or not */
static int	PG_SIZE;
static int	retval;
static char 	discfile[15];	/* for storing outputfile name */
static short	pgcnt; 		/* for page count */
static char	resp[2];	/* for storing response */
static short	copies;

extern char 	e_mesg[200];	/* for storing error messages */

stdate(menuop)
int	menuop;
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
			strcpy( discfile, "stlist.dat" );
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
	if(strcmp( barg1, "     0") == 0)
		barg1[0] = '\0';

	center1 = 0001;
	center2 = 9999;
	retval = GetCenterRange (&center1,&center2);
	if (retval < 0) return(retval);

	strcpy( posi1, "     0");
	strcpy( posi2, "ZZZZZZ" );
	retval = GetPosRange( posi1,posi2 );
	if(retval < 0) return(retval);
	if(strcmp( posi1, "     0") == 0)
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

	if(sortop2[0]=='P'||sortop2[0]=='B') {
		unlink_file(TMPINDX_1);
		strcpy(tmpindxfile,"emptemp");
		get_tnum(tnum);
		strcat(tmpindxfile,tnum);
		if(CrtIndx2( menuop,tmpindxfile )<0){
			fomer(e_mesg);
		}
	}
	if(sortop2[0]=='C'||sortop2[0]=='L') {
		unlink_file(TMP_SCHED1);/*remove temp file*/
		if(BldSched2(menuop)<0){
			fomer(e_mesg);
		}
	}

	retval = PrintRpt(menuop);

	close_rep();	/* close output file */
	close_dbh();
	return(retval);
}
/******************************************************************************
Main logic of the program
******************************************************************************/
static
PrintRpt(menuop)
int	menuop;
{

	short	last_center;
	char	last_pos[7];
	char	last_class[7];
	char	last_barg[7];
	int	first_time = 0;

	/* get the cost center name */
	retval = get_param(&pa_rec,BROWSE,1,e_mesg);
	if (retval < 0){
		fomer(e_mesg);get ();
	}
	/*
		******* Sort by Center *******
	*/
	if(sortop2[0]=='C' || sortop2[0]=='L'){
		tmp_sched1.tm_cost = 0;
 		tmp_sched1.tm_class[0] = '\0';
		last_center = tmp_sched1.tm_cost;
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
					retval = PrntHdg(menuop);
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
					retval = PrntHdg(menuop);
					if(retval < 0) {
						return(retval);
					}
					if(retval == EXIT)	break;
					first_time = 1;
				}
			}
			if ((retval = PrntRec(menuop))<0)
				return(retval);
			if(retval == EXIT)	break;

			last_center = tmp_sched1.tm_cost;
			strcpy(last_class,tmp_sched1.tm_class);

		}/*end of for loop*/
		seq_over(TMP_SCHED1);
	}/*end check which files used*/
	/*
		*********** Sort by Alpha and Bargain or Position *************
	*/
	if(sortop2[0]=='P'||sortop2[0]=='B'){
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
		emp_rec.em_st_dt_ft = 0;
		emp_rec.em_st_dt_pt = 0;
		emp_rec.em_st_dt_ca = 0;
		emp_rec.em_st_dt_su = 0;
		emp_rec.em_first_name[0] = '\0';
		emp_rec.em_last_name[0] = '\0';
		flg_reset(TMPINDX_1);

		for (;;){
			retval = get_next((char*)&emp_rec,TMPINDX_1,0,FORWARD,BROWSE,e_mesg);
			if(retval==EFL)		break;
			if(retval<0)	{
				fomer(e_mesg);
				get();
				return(-1);
			}	
			if(strcmp(emp_rec.em_status,"ACT") != 0) 
				continue;
			/*
			******* Check the start dates ************
			*/
			if((emp_rec.em_st_dt_ft <= 0) && (menuop == 1))
				continue;
			if((emp_rec.em_st_dt_pt <= 0) && (menuop == 2))
				continue;
			if((emp_rec.em_st_dt_ca <= 0) && (menuop == 3))
				continue;
			if((emp_rec.em_st_dt_su <= 0) && (menuop == 4))
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
			/*
			**************** Check input ranges ***********
			*/
			if( (emp_rec.em_cc < center1)  ||
		   	    (emp_rec.em_cc > center2) )
				continue;

			if(strcmp(emp_rec.em_class,class1) < 0 ||
		   	   strcmp(emp_rec.em_class,class2) > 0 ) 
				continue;

			if(sortop2[0] == 'P') {
				if(strcmp(last_pos,emp_rec.em_pos)!=0 ||
				    first_time == 0 || linecnt > PG_SIZE) {
					retval = PrntHdg(menuop);
					if(retval < 0) {
						return(retval);
					}
					if(retval == EXIT)	break;
					first_time = 1;
				}
			}
			if(sortop2[0] == 'B') {
				if(strcmp(last_barg,emp_rec.em_barg)!=0 ||
				   first_time == 0 || linecnt > PG_SIZE) {
					retval = PrntHdg(menuop);
					if(retval < 0) {
						return(retval);
					}
					if(retval == EXIT)	break;
					first_time = 1;
				}
			}
			if ((retval = PrntRec(menuop))<0)
				return(retval);
			if(retval == EXIT)	break;

			strcpy(last_pos,emp_rec.em_pos);
			strcpy(last_barg,emp_rec.em_barg);

		} /*end of endless for loop*/
	}
	
	if(pgcnt){
		if(term<99)
		{
			last_page();
		}
	}
	return(NOERROR);
}
/******************************************************************************
Prints the headings of the report 
******************************************************************************/
static
PrntHdg(menuop)		/* Print heading  */
int	menuop;
{
	short	offset;
	long	sysdt ;
	char	txt_line[132];

	if( pgcnt && term < 99)   /* new page and display */
		if((retval=next_page())<0) 
			return(EXIT);	
		
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
	switch (menuop){
	case 1:		/*FULL TIME*/
		mkln((LNSZ-34)/2,"LIST OF EMPLOYEES BY START DATE FT", 34 );
		break;
	case 2:		/*FULL TIME*/
		mkln((LNSZ-34)/2,"LIST OF EMPLOYEES BY START DATE PT", 34 );
		break;
	case 3:		/*FULL TIME*/
		mkln((LNSZ-34)/2,"LIST OF EMPLOYEES BY START DATE CA", 34 );
		break;
	case 4:		/*FULL TIME*/
		mkln((LNSZ-34)/2,"LIST OF EMPLOYEES BY START DATE SU", 34 );
		break;
	default:
		break;
	}
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
		if(retval < 0) {
			fomer(e_mesg);
			get();
		}
		mkln(1,"COST CENTER: ",13);
		tedit( (char *)&tmp_sched1.tm_cost,"__0_",txt_line,R_SHORT);
		mkln(14,txt_line,4);
		mkln(19,sch_rec.sc_name,28);
	}
	if(sortop2[0] == 'B') {
		strcpy(barg_unit.b_code,emp_rec.em_barg);
		barg_unit.b_date = get_date();
		flg_reset(BARG);

		retval = get_n_barg(&barg_unit,BROWSE,0,BACKWARD,e_mesg);
		if(retval < 0) {
			fomer(e_mesg);
			get();
		}
		mkln(1,"BARGAINING UNIT: ",17);
		mkln(18,emp_rec.em_barg,6);
		if(strcmp(barg_unit.b_code,emp_rec.em_barg)==0) {
			mkln(25,barg_unit.b_name,30);
		}
		seq_over(BARG);
	}
	if(sortop2[0] == 'P') {
		strcpy(position.p_code,emp_rec.em_pos);
		retval = get_position(&position,BROWSE,0,e_mesg);
		if(retval < 0) {
			fomer(e_mesg);
			get();
		}
		mkln(1,"POSITION: ",10);
		mkln(11,emp_rec.em_pos,6);
		mkln(18,position.p_desc,30);
	}

	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(1,"START DATE",10);
	mkln(15,"EMPLOYEE",8);
	mkln(36,"EMPLOYEE NAME",13);
	mkln(64,"SIN",3);
	mkln(73,"BIRTH DATE",10);
	mkln(85,"SEX",3);
	mkln(90,"MARITAL",7);
	mkln(99,"TITLE",5);
	mkln(106,"MAIDEN",6);
	mkln(121,"TELEPHONE",9);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(1,"YYYY/MM/DD",10);
	mkln(16,"NUMBER",6);
	mkln(73,"YYYY/MM/DD",10);
	mkln(90,"STATUS",6);
	mkln(106,"NAME",4);
	mkln(122,"NUMBER",6);

	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	return(NOERROR);
}
/*---------------------------------------------------------------------*/
static
PrntRec(menuop)
int	menuop;
{
	char	txt_line[132];

	strcpy(txt_line,"        "); 
  	if (menuop == 1)
		sprintf(txt_line,"%ld",emp_rec.em_st_dt_ft);
  	if (menuop == 2)
		sprintf(txt_line,"%ld",emp_rec.em_st_dt_pt);
  	if (menuop == 3)
		sprintf(txt_line,"%ld",emp_rec.em_st_dt_ca);
  	if (menuop == 4)
		sprintf(txt_line,"%ld",emp_rec.em_st_dt_su);
	mkln(1,txt_line,4);	
	mkln(5,"/",1);
	mkln(6,txt_line+4,2);
	mkln(8,"/",1);
	mkln(9,txt_line+6,2);
	mkln(13,emp_rec.em_numb,12);
	sprintf(txt_line,"%s, %c",
		emp_rec.em_last_name,
		emp_rec.em_first_name[0]);
	mkln(28,txt_line,26);
	mkln(60,emp_rec.em_sin,9);
	strcpy(txt_line,"        "); 
	sprintf(txt_line,"%ld",emp_rec.em_date);
	mkln(73,txt_line,4);	
	mkln(77,"/",1);
	mkln(78,txt_line+4,2);
	mkln(80,"/",1);
	mkln(81,txt_line+6,2);

	mkln(86,emp_rec.em_sex,1);
	mkln(92,emp_rec.em_mar_st,1);
	mkln(99,emp_rec.em_title,4);
	mkln(104,emp_rec.em_maid_name,15);
	mkln(120,emp_rec.em_phone,3);
	mkln(123,"-",1);
	mkln(124,emp_rec.em_phone+3,3);
	mkln(127,"-",1);
	mkln(128,emp_rec.em_phone+6,4);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	return(NOERROR);
}
/*---------------------------------------------------------------------*/	
static
BldSched2 (menuop)
int	menuop;
{
	int	x;

	/*Get the emp name and dates from the employee file*/

	strcpy(emp_rec.em_numb,empl1);
	flg_reset(EMPLOYEE);
	for (;;){
		retval = get_n_employee(&emp_rec,BROWSE,0,FORWARD,e_mesg);
		if(retval<0){
			roll_back(e_mesg);
			break;	
		}
	
	  	retval=UsrBargVal(BROWSE,emp_rec.em_numb,emp_rec.em_barg,0,
									e_mesg);
	  	if(retval < 0)
			continue;

		/* disregard records without correct dates */
		if((emp_rec.em_st_dt_ft <= 0) && (menuop == 1))
			continue;
		if((emp_rec.em_st_dt_pt <= 0) && (menuop == 2))
			continue;
		if((emp_rec.em_st_dt_ca <= 0) && (menuop == 3))
			continue;
		if((emp_rec.em_st_dt_su <= 0) && (menuop == 4))
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
				roll_back(e_mesg);
				break;	
			}
			if(strcmp(emp_rec.em_numb,emp_sched1.es_numb)!=0) {
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
			if((retval = PutTmpSched(menuop)) <0)
				break;
		}/* end check sched1 rec loop */

		if( (emp_rec.em_cc < center1) || 
		 (emp_rec.em_cc > center2) )
			continue;

		if(strcmp(emp_rec.em_class,class1) < 0 ||
	         strcmp(emp_rec.em_class,class2) > 0 ) 
			continue;

		sched1_flag[0] = 'N';
		retval = PutTmpSched(menuop);
		if(retval < 0)
			break;
	}/*end endless loop*/
	seq_over(EMPLOYEE);
	return(NOERROR);
}
/*----------------------------------------------------------------------*/
static
PutTmpSched(menuop)
int	menuop;
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
	if(menuop == 1)
		sprintf(sort_array,"%-ld",emp_rec.em_st_dt_ft);
	if(menuop == 2)
		sprintf(sort_array,"%-ld",emp_rec.em_st_dt_pt);
	if(menuop == 3)
		sprintf(sort_array,"%-ld",emp_rec.em_st_dt_ca);
	if(menuop == 4)
		sprintf(sort_array,"%-ld",emp_rec.em_st_dt_su);
	strncpy(tmp_sched1.tm_sortk_2,sort_array,8);

	sprintf(sort_array,"%s%c",emp_rec.em_last_name,
			      emp_rec.em_first_name[0]);
	if(sortop1[0] == 'A') {
		strncpy(tmp_sched1.tm_sortk_3,sort_array,40);
	}
	else {
		strcpy(tmp_sched1.tm_sortk_3,emp_rec.em_numb);
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
/*----------------------------------------------------------------------*/
static
CrtIndx2 ( menuop,tempindxnm )
int	menuop;
char	*tempindxnm;	/*temparary index file name*/
{
	int	keysarray[25];
	int	step=0;
	
	switch (sortop2[0]){
		case 'B':	/*Name within Bargain unit key*/
			if(sortop1[0]=='A')
				keysarray[step++] = 5;	/*Parts*/
			else	
				keysarray[step++] = 3;

			/*Part 1*/
			keysarray[step++] = CHAR;
			keysarray[step++] =  6;
			keysarray[step++] = (char*)&emp_rec.em_barg[0] - (char*)&emp_rec;
			keysarray[step++] = ASCND;

			/*Part 2*/
			keysarray[step++] = LONG;
			keysarray[step++] =  1;

			if(menuop == 1)
				keysarray[step++] = (char*)&emp_rec.em_st_dt_ft - (char*)&emp_rec;
			if(menuop == 2)
				keysarray[step++] = (char*)&emp_rec.em_st_dt_pt - (char*)&emp_rec;
			if(menuop == 3)
				keysarray[step++] = (char*)&emp_rec.em_st_dt_ca - (char*)&emp_rec;
			if(menuop == 4)
				keysarray[step++] = (char*)&emp_rec.em_st_dt_su - (char*)&emp_rec;

			keysarray[step++] = ASCND;

			if(sortop1[0]=='A'){
			/*Part 3*/
				keysarray[step++] = CHAR;
				keysarray[step++] =  25;
				keysarray[step++] = (char*)&emp_rec.em_last_name[0] - (char*)&emp_rec;
				keysarray[step++] = ASCND;
	
				/*Part 4*/
				keysarray[step++] = CHAR;
				keysarray[step++] =  15;
				keysarray[step++] = (char*)&emp_rec.em_first_name[0] - (char*)&emp_rec;
				keysarray[step++] = ASCND;
			}

			/*Part 5*/
			keysarray[step++] = CHAR;
			keysarray[step++] =  12;
			keysarray[step++] = (char*)&emp_rec.em_numb[0] - (char*)&emp_rec;
			keysarray[step++] = ASCND;
			break;

		case 'P':	/*Name within Position code key*/

			if(sortop1[0]=='A')
				keysarray[step++] = 5;	/*Parts*/
			else
				keysarray[step++] = 3;


			/*Part 1*/
			keysarray[step++] = CHAR;
			keysarray[step++] =  6;
			keysarray[step++] = (char*)&emp_rec.em_pos[0] - (char*)&emp_rec;
			keysarray[step++] = ASCND;

			/*Part 2*/
			keysarray[step++] = LONG;
			keysarray[step++] =  1;

			if(menuop == 1)
				keysarray[step++] = (char*)&emp_rec.em_st_dt_ft - (char*)&emp_rec;
			if(menuop == 2)
				keysarray[step++] = (char*)&emp_rec.em_st_dt_pt - (char*)&emp_rec;
			if(menuop == 3)
				keysarray[step++] = (char*)&emp_rec.em_st_dt_ca - (char*)&emp_rec;
			if(menuop == 4)
				keysarray[step++] = (char*)&emp_rec.em_st_dt_su - (char*)&emp_rec;

			keysarray[step++] = ASCND;

			if(sortop1[0]=='A'){
				/*Part 3*/
				keysarray[step++] = CHAR;
				keysarray[step++] =  25;
				keysarray[step++] = (char*)&emp_rec.em_last_name[0] - (char*)&emp_rec;
				keysarray[step++] = ASCND;

				/*Part 4*/
				keysarray[step++] = CHAR;
				keysarray[step++] =  15;
				keysarray[step++] = (char*)&emp_rec.em_first_name[0] - (char*)&emp_rec;
				keysarray[step++] = ASCND;
			}

			/*Part 5*/
			keysarray[step++] = CHAR;
			keysarray[step++] =  12;
			keysarray[step++] = (char*)&emp_rec.em_numb[0] - (char*)&emp_rec;
			keysarray[step++] = ASCND;
			break;
		default: 
			break;
	}/*end switch*/

	retval = CrtTmpIndx(EMPLOYEE,TMPINDX_1,keysarray,tempindxnm,
		(int(*)())NULL,e_mesg);

	return(NOERROR);
}
/******************   END OF PROGRAM *******************************/
