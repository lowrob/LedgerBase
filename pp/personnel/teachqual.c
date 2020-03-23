/******************************************************************************
		Sourcename   : teachqual.c
		Created on   : 92-02-26
		Created  By  : Littlejohn 

******************************************************************************
About the file:	

History:
Programmer      Last change on    Details
m. galvin	92/09/04	  I modified the program since the
				  files used have been modified.

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
static Area_spec	area_spec;
static Teach_qual	qual_rec;
static Pay_param	param_rec;
static Barg_unit	barg_unit;
static Position		position;
static Class		class;

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

static char	sched1_flag[2];

static int	flag;		/* flag to print the employee or not */
static int	PG_SIZE;
static int	retval;
static char 	discfile[15];	/* for storing outputfile name */
static short	pgcnt; 		/* for page count */
static char	resp[2];	/* for storing response */
static short	copies;

extern char 	e_mesg[80];	/* for storing error messages */

teachqual()
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
			STRCPY( discfile, "teachqual.dat" );
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
	if(barg2[0] == 0)
		barg2[0] = '\0';

	center1 = 0001;
	center2 = 9999;
	retval = GetCenterRange (&center1,&center2);
	if (retval < 0) return(retval);

	strcpy( posi1, "     0");
	strcpy( posi2, "ZZZZZZ" );
	retval = GetPosRange( posi1,posi2 );
	if(retval < 0) return(retval);
	if(posi1[0] == 0)
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

	if(sortop2[0]=='B'||sortop2[0]=='P') {
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
		if(BldTmpSched()<0){
			return (-1);
		}
	}

	retval = PrintRpt();

	close_dbh();
	close_rep();	/* close output file */
	return(retval);
}

/******************************************************************************
Main logic of the program
******************************************************************************/
static
PrintRpt()
{
	int	first_time = 0 ;
	char	last_barg[7], last_pos[7], last_class[7] ;
	short	last_center;

	/* get the cost center name */
	retval = get_param(&pa_rec,BROWSE,1,e_mesg);
	if (retval < 0){
		fomer(e_mesg);
		get ();
	}
	/*
		******* Sort by Alpha and Center *******
	*/
	if(sortop2[0]=='C'||sortop2[0]=='L'){
		tmp_sched1.tm_class[0] = '\0';	
		tmp_sched1.tm_cost = 0;	
		last_center = tmp_sched1.tm_cost;
		strcpy(last_class,tmp_sched1.tm_class);
		tmp_sched1.tm_sortk_1[0] = '\0';
		tmp_sched1.tm_sortk_2[0] = '\0';
		tmp_sched1.tm_sortk_3[0] = '\0';
		flg_reset(TMP_SCHED1);

		for(;;){
			retval=get_n_tmp_sched1(&tmp_sched1,BROWSE,1,FORWARD,e_mesg);

			if(retval == EFL){break;}
			if(retval<0){
				break;
			}

			strcpy(emp_rec.em_numb,tmp_sched1.tm_numb);
			retval=get_employee(&emp_rec,BROWSE,0,e_mesg);
			if(retval < 0) {
				fomer(e_mesg);
				get();
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
			}
			if(sortop2[0] == 'L') {
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

			if ((retval = PrntRec())<0)
				return(retval);
			if(retval == EXIT)	break;
			last_center = tmp_sched1.tm_cost;
			strcpy(last_class,tmp_sched1.tm_class);

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
			strcpy(last_pos,emp_rec.em_pos);
			emp_rec.em_barg[0] = '\0';
		}
		emp_rec.em_barg[0] = '\0';
		emp_rec.em_pos[0] = '\0';
		emp_rec.em_first_name[0] = '\0';
		emp_rec.em_last_name[0] = '\0';
		flg_reset(TMPINDX_1);

		for (;;){
			retval = get_next((char*)&emp_rec,TMPINDX_1,0,FORWARD,BROWSE,e_mesg);
			if(retval==EFL)		break;
			if(retval<0)		break;
	
			if(strcmp(emp_rec.em_status,"ACT")!=0)
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
				if(strcmp(last_pos,emp_rec.em_pos)!=0
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
			strcpy(last_pos,emp_rec.em_pos);
			strcpy(last_barg,emp_rec.em_barg);

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
	mkln((LNSZ-23)/2,"EMPLOYEE QUALIFICATIONS", 23 );
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

	mkln(3,"EMPLOYEE",8);
	mkln(15,"NAME",4);
	mkln(49,"CERT",4);
	mkln(55,"STEP",4);
	mkln(62,"LEVEL",5);
	mkln(71,"SCHOOL",6);
	mkln(80,"MAJOR",5);
	mkln(107,"PROGRAM",7);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(4,"NUMBER",6);
	mkln(62,"(E/J/S)",7);
	mkln(72,"PREF",4);
	mkln(80,"INST",4);
	mkln(108,"(BA)",4);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	return(NOERROR);
}
/*-----------------------------------------------------------------*/
static
PrntRec()
{
	char	txt_line[132];
	int	x=0;

	mkln(1,emp_rec.em_numb,12);
	sprintf(txt_line,"%s, %s %s",
		emp_rec.em_last_name,
		emp_rec.em_first_name,
		emp_rec.em_mid_name);
	mkln(15,txt_line,35);
	mkln(50,emp_rec.em_cert,2);
	tedit( (char *)&txt_line,"__",emp_rec.em_level, R_SHORT ); 
	mkln(55,txt_line,2);
	mkln(64,emp_rec.em_pre_lev,1);
	sprintf(txt_line,"%d",emp_rec.em_pref_cc);
	mkln(72,txt_line,4);
	for(x=0;x<5;x++){
		if(emp_rec.em_inst[x][0] != '\0' &&
		   emp_rec.em_prog[x][0] != '\0'){
			mkln(78,emp_rec.em_inst[x],15);
			mkln(100,emp_rec.em_prog[x],15);
			if(prnt_line() < 0 )	return(REPORT_ERR);
		}
	}
	if(prnt_line() < 0 )	return(REPORT_ERR);

	PrntQual();	

	return(NOERROR);
}
/*------------------------------------------------------------------*/
PrntQual()
{
	int	first_time;

	first_time = 0;
	strcpy(qual_rec.tq_numb,emp_rec.em_numb);	
	qual_rec.tq_code[0] = '\0';
	flg_reset(TEACH_QUAL);

	for(;;){
		retval=get_n_teach_qual(&qual_rec,BROWSE,0,FORWARD,e_mesg);
		if(retval<0)		continue;
		if(retval == EFL ||
			(strcmp(emp_rec.em_numb,qual_rec.tq_numb)!=0)){
			break;
		}

		if(first_time == 0){
			mkln(50,"AREA OF SPECIALIZATION:",23);
			first_time = 1;
		}
		strcpy(area_spec.ar_code,qual_rec.tq_code);
		flg_reset(AREA_SPEC);

		retval=get_area_spec(&area_spec,BROWSE,0,e_mesg);
		if(retval<0)		continue;
		mkln(75,area_spec.ar_desc,30);
		if(linecnt > PG_SIZE)
			if((retval =PrntHdg()) == EXIT)
				return(retval);

		if(prnt_line() < 0 )	return(REPORT_ERR);
	}/*end loop*/
	seq_over(TEACH_QUAL);

	if(prnt_line() < 0 )	return(REPORT_ERR);

	return(NOERROR);
}
/*-----------------------------------------------------------------------*/
static
BldTmpSched ()
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
