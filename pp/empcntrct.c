/******************************************************************************
		Sourcename   : empcntrct.c
		System       : 
		Module       :
		Created on   : 92-08-24
		Created  By  : m. galvin 

******************************************************************************
About the program:	
	This program is used to produce the Employee's Listing of 
	Contracts.  The program lists the employee's number, name,
	contract code and contract description.  At the top of
	each report is the selected sort option description.  If
	the report is to be sorted by the cost center, then the 
	records are written to a temporary sort file; otherwise,
	a temporary index is setup on the employee file.
	

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
#define PRINTER		'P'
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
static Term		term_rec;
static Barg_unit	barg_unit;
static Position		position;
static Religion		religion;

/*  Data items for storing the key range end values */
static char	sortop1[2];
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

static char	sched1_flag[2];

static int	flag;		/* flag to print the employee or not */
static int	PG_SIZE;
static int	retval;
static char 	discfile[15];	/* for storing outputfile name */
static short	pgcnt; 		/* for page count */
static char	resp[2];	/* for storing response */
static short	copies;

extern char 	e_mesg[80];	/* for storing error messages */

empcntrct()
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
			STRCPY( discfile, "empcntrct.dat" );
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

	unlink_file(TMP_SCHED1);/*remove temp file*/
	if(BldTempFile()<0){
		return (-1);
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
	int	key, first_time = 0;
	char	last_religion[2];

	/* get the cost center name */
	retval = get_param(&pa_rec,BROWSE,1,e_mesg);
	if (retval < 0){
		fomer(e_mesg);get ();
	}

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
	  	retval=UsrBargVal(BROWSE,emp_rec.em_numb,emp_rec.em_barg,0,
									e_mesg);
	  	if(retval < 0){
			continue;
		}
		if(emp_rec.em_religion[0] != '\0'){
			strcpy(religion.rel_code,emp_rec.em_religion);
			retval=get_rel(&religion,BROWSE,0,e_mesg);
			if(retval < 0) {
				fomer(e_mesg);
				get();
				continue;
			}
		}

		if(strcmp(last_religion,emp_rec.em_religion)!=0 || 
			first_time == 0 || linecnt > PG_SIZE) { 
			retval = PrntHdg();
			if(retval < 0) 	{
				return(retval);	
			}
			if(retval == EXIT)	break;
			first_time = 1;
		}

		if ((retval = PrntRec())<0)
			return(retval);
		if(retval == EXIT)	break;

		strcpy(last_religion,emp_rec.em_religion);

	}/*end of for loop*/

	seq_over(TMP_SCHED1);

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
	mkln((LNSZ-30)/2,"LIST OF EMPLOYEE'S CONTRACTS", 30 );
#else
	mkln((LNSZ-25)/2,"TRANSLATE        ", 17 );
#endif
	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(1,"CONTRACT:",9);
	if(emp_rec.em_religion[0] != '\0'){
		strcpy(religion.rel_code,emp_rec.em_religion);
		retval=get_rel(&religion,BROWSE,0,e_mesg);
		if(retval < 0) {
			fomer(e_mesg);
			get();
		}
		else{
			mkln(12,emp_rec.em_religion,2);
			mkln(15,religion.rel_desc,30);
		}	
	}
	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(3,"EMPLOYEE",8);
	mkln(25,"EMPLOYEE NAME",13);
	mkln(57,"ADDRESS",7);
	mkln(86,"TELEPHONE",9);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	mkln(4,"NUMBER",6);
	mkln(87,"NUMBER",6);

	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	return(NOERROR);
}
/*---------------------------------------------------------------------*/
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
	if(emp_rec.em_add1[0] == '\0')
		mkln(50,emp_rec.em_add2,30);
	else
		mkln(50,emp_rec.em_add1,30);
	mkln(50,emp_rec.em_add1,30);

	mkln(85,emp_rec.em_phone,3);
	mkln(88,"-",1);
	mkln(89,emp_rec.em_phone+3,3);
	mkln(92,"-",1);
	mkln(93,emp_rec.em_phone+6,4);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	sprintf(txt_line,"%s, %s",
		emp_rec.em_add3,
		emp_rec.em_add4);
	mkln(50,txt_line,30);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	mkln(50,emp_rec.em_pc,10);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	return(NOERROR);
}
/*-----------------------------------------------------------------------*/
static
BldTempFile ()
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
 	
	strncpy(tmp_sched1.tm_sortk_1,emp_rec.em_religion,4);
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
