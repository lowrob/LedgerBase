/******************************************************************************
		Sourcename   : empabsent.c
		System       : 
		Module       :
		Created on   : 92-03-17
		Created  By  : Littlejohn 
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
static Att		att_rec;
static Emp_at_his	att_hist;
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
double 	ab_grand_tot=0.0;	/*report totals*/
double 	wi_grand_tot=0.0;
double	ytd_grand_tot=0.0;

double 	brk_ab_tot=0.0;	/*report totals*/
double 	brk_wi_tot=0.0;
double	brk_ytd_tot=0.0;

double	ytd_emp_tot=0.0;	/*employee totals*/
double	ab_emp_tot=0.0;
double	wi_emp_tot=0.0;
char	prn_flag[2];		/*flag to determine if emp nam # to be printed*/

static char	sub_name[42];		/* the replacement teacher name*/
static char	sub_numb[12];		/* the replacement teacher number*/

empabsent()
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
			STRCPY( discfile, "empabsent.dat" );
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

	date1 = 0;
	date2 = 99991231;
	if((retval = GetDateRange(&date1,&date2))<0)
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
		if(BldEmpAbs()<0){
			return (-1);
		}
	}

	retval = PrintRpt();
	if(retval < 0)	return(-1);

	if(retval != EXIT){
		retval = PrintSubTot();
		if(retval < 0)	return(-1);
		retval = PrintGrTot();
		if(retval < 0)	return(-1);
	}

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
			*** get the attendance history information ****
			*/
			tmp_code[0] ='\0';
			strcpy(att_hist.eah_numb,emp_rec.em_numb);
			att_hist.eah_date = date1;
			flg_reset(EMP_ATT);

			for (;;){
				retval=get_n_emp_at(&att_hist,BROWSE,0,FORWARD,
									e_mesg);
				if(retval==EFL)		break;

				if(retval<0){
					sprintf(e_mesg,"Error %d reading hist",
						retval);
					fomer(e_mesg);get();
					break;
				}
	
				if(strcmp(att_hist.eah_numb,emp_rec.em_numb)!=0
				  || att_hist.eah_date > date2)
					break;

				/*
				*** Get the attendance code information ***
				*/
				strcpy(att_rec.at_code,att_hist.eah_code);
				flg_reset(ATT);

				retval=get_att(&att_rec,BROWSE,1,e_mesg);
				if(retval==EFL)		continue;
				if(retval<0){
					sprintf(e_mesg,"%d",retval);
					fomer(e_mesg);get();
					continue;
				}
	
				if(strcmp(att_rec.at_code,att_hist.eah_code)!=0)
					continue;

				AccumTots();

				/* move the record values in the print vals*/
				emp_sched1.es_cost = tmp_sched1.tm_cost;
				strcpy(emp_sched1.es_class,tmp_sched1.tm_class);

				prn_flag[0] = 'Y';

				if(strcmp(att_hist.eah_numb,tmp_code) == 0)
					prn_flag[0] = 'N';
			
				sub_name[0] = '\0';
				if (att_hist.eah_teach[0] != '\0')
					GetSubName();
				if(sortop2[0] == 'C'){
				  if(strcmp(last_class,tmp_sched1.tm_class)!=0|| 
					first_time == 0 || linecnt > PG_SIZE) { 
					if(strcmp(last_class,
					    tmp_sched1.tm_class)!=0 && 
					    first_time==1){
						retval = PrintSubTot();
						if(retval < 0) {
							return(retval);
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
				if(sortop2[0] == 'L'){
				  if(last_center != tmp_sched1.tm_cost ||
			   	     first_time == 0 || linecnt > PG_SIZE) { 
					if(first_time == 1 &&
					  last_center != tmp_sched1.tm_cost){
						retval = PrintSubTot();
						if(retval < 0) {
							return(retval);
						}
					}
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

			seq_over(EMP_ATT);

			/*print the emp totals*/
			if(strcmp(emp_rec.em_numb,tmp_code) == 0){
				if((retval=PrntTot()) ==EXIT)	return(retval);
				if(prnt_line() < 0 )	return(REPORT_ERR);
			}
			/* initialize total fields */

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
			*** get the attendance history information ***
			*/
			tmp_code[0] ='\0';
			strcpy(att_hist.eah_numb,emp_rec.em_numb);
			att_hist.eah_date = 0;
			flg_reset(EMP_ATT);

			for (;;){
				retval=get_n_emp_at(&att_hist,BROWSE,0,FORWARD,e_mesg);
				if(retval==EFL)		break;
				if(retval<0){
					sprintf(e_mesg,"%d",retval);
					fomer(e_mesg);get();
					break;
				}

				if(strcmp(att_hist.eah_numb,emp_rec.em_numb)!=0)
					break;
	
				if((att_hist.eah_date < date1) || 
				   (att_hist.eah_date > date2))	{
					continue;
				}
				/*
				***** Get the attendance code information *****
				*/
				strcpy(att_rec.at_code,att_hist.eah_code);
				flg_reset(ATT);

				retval=get_att(&att_rec,BROWSE,1,e_mesg);
				if(retval==EFL)		continue;
				if(retval<0){
					sprintf(e_mesg,"%d",retval);
					fomer(e_mesg);get();
					continue;
				}

				AccumTots();

				seq_over(ATT);

				prn_flag[0] = 'Y';
				if(strcmp(att_hist.eah_numb,tmp_code) == 0)
					prn_flag[0] = 'N';
				
				sub_name[0] = '\0';
				if (att_hist.eah_teach[0] != '\0')
					GetSubName();
				if(sortop2[0] == 'B') {
				  if(strcmp(last_barg,emp_rec.em_barg)!=0
				   || first_time == 0 || linecnt > PG_SIZE) {
					if(first_time != 0 &&
					  strcmp(last_barg,emp_rec.em_barg)!=0){
						retval = PrintSubTot();
						if(retval < 0) {
							return(retval);
						}
					}					
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
					if(first_time != 0 &&
					  strcmp(last_position,
					  emp_rec.em_pos)!=0){
						retval = PrintSubTot();
						if(retval < 0) {
							return(retval);
						}
					}					
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

			seq_over(EMP_ATT);

			/*print the emp totals*/
			if(strcmp(emp_rec.em_numb,tmp_code) == 0){
				if((retval=PrntTot()) ==EXIT)	return(retval);
				if(prnt_line() < 0 )	return(REPORT_ERR);
			}
			/* initialize total fields */

		} /*end of endless for loop*/
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
	mkln((LNSZ-24)/2,"LIST OF EMPLOYEES ABSENT", 24 );
#else
	mkln((LNSZ-25)/2,"TRANSLATE        ", 17 );
#endif
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(51,"FROM ",5);
	tedit( (char *)&date1,"____/__/__",line+cur_pos, R_LONG ); 
	cur_pos += 10;
	mkln(67,"TO ",3);
	tedit( (char *)&date2,"____/__/__",line+cur_pos, R_LONG ); 
	cur_pos += 10;

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

	mkln(1,"CC#",3);
	mkln(6,"POSITION",8);
	mkln(17,"CLASS",5);
	mkln(24,"DATE ABSENT",11);
	mkln(38,"ATT",3);
	mkln(51,"ATTENDANCE",10);
	mkln(72,"ABSENT",6);
	mkln(81,"ABSENT",6);
	mkln(90,"ACCUM",5);
	mkln(106,"REPLACEMENT",11);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(24,"YYYY/MM/DD",10);
	mkln(38,"CODE",4);
	mkln(54,"DESC",4);
	mkln(71,"WITH PAY",8);
	mkln(81,"WITHOUT",7);
	mkln(90,"ABSENT",6);

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
	double	tmp_att;

	if(prn_flag[0]=='Y'){
		mkln(1,"EMPLOYEE :",10);
		mkln(11,att_hist.eah_numb,12);
		sprintf(txt_line,"%s, %s %s",
			emp_rec.em_last_name,
			emp_rec.em_first_name,
			emp_rec.em_mid_name);
		mkln(25,txt_line,40);
		if(prnt_line() < 0 )	return(REPORT_ERR);
		if(prnt_line() < 0 )	return(REPORT_ERR);
	}
	sprintf(txt_line,"%d",emp_rec.em_cc);
	mkln(1,txt_line,4);
	mkln(6,emp_rec.em_pos,6);
	mkln(16,emp_rec.em_class,6);
	sprintf(txt_line,"%ld",att_hist.eah_date);
	mkln(24,txt_line,4);
	mkln(28,"/",1);
	mkln(29,txt_line+4,2);
	mkln(31,"/",1);
	mkln(32,txt_line+6,2);
	mkln(38,att_rec.at_code,3);
	mkln(44,att_rec.at_desc,25);
	tmp_att = (att_hist.eah_hours/att_hist.eah_sen_hours);
	tedit((char*)&tmp_att,"_0_.__",txt_line,R_DOUBLE); 
	if(att_rec.at_earn[0] == 'N'){
		mkln(71,txt_line,6);
		mkln(81,"  0.00",6);
	}
	else{
		mkln(71,"  0.00",6);
		mkln(81,txt_line,6);
	}
	tedit((char*)&ytd_emp_tot,"_0_.__",txt_line,R_DOUBLE);
	mkln(90,txt_line,6);
	if (att_hist.eah_teach[0] != '\0'){
		mkln(98,sub_numb,12);
		mkln(111,sub_name,20);
	}
	if(prnt_line() < 0 )	return(REPORT_ERR);

	return(NOERROR);
}
/*----------------------------------------------------------------------*/
AccumTots()
{

	if(att_rec.at_earn[0] == 'N'){
		ab_emp_tot += (att_hist.eah_hours/att_hist.eah_sen_hours);
	}
	else{
		wi_emp_tot += (att_hist.eah_hours/att_hist.eah_sen_hours);
	}
	ytd_emp_tot += (att_hist.eah_hours/att_hist.eah_sen_hours);	

	return(NOERROR);
}
/*-----------------------------------------------------------------------*/
PrntTot()
{
	char	txt_line[132];

	if(prnt_line() < 0 )	return(REPORT_ERR);
	mkln(1,"TOTAL:",6);
	tedit((char*)&ab_emp_tot,"_0_.__",txt_line,R_DOUBLE);
	mkln(71,txt_line,6);
	tedit((char*)&wi_emp_tot,"_0_.__",txt_line,R_DOUBLE);
	mkln(81,txt_line,6);
	tedit((char*)&ytd_emp_tot,"_0_.__",txt_line,R_DOUBLE);
	mkln(90,txt_line,6);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	/* accumulate the total for the break */
	brk_ytd_tot += ytd_emp_tot;
	brk_ab_tot += ab_emp_tot;
	brk_wi_tot += wi_emp_tot;

	/*re-init emp totals*/
	ytd_emp_tot = 0.0;
	ab_emp_tot=0.0;
	wi_emp_tot=0.0;

	return(NOERROR);
}
/*-----------------------------------------------------------------------*/
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
	
	tedit((char*)&brk_ab_tot,"_,_0_.__",txt_line,R_DOUBLE);
	mkln(69,txt_line,8);
	tedit((char*)&brk_wi_tot,"_,_0_.__",txt_line,R_DOUBLE);
	mkln(79,txt_line,8);
	tedit((char*)&brk_ytd_tot,"_,_0_.__",txt_line,R_DOUBLE);
	mkln(88,txt_line,8);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	/*re-init emp totals*/
	ab_grand_tot += brk_ytd_tot;
	wi_grand_tot += brk_wi_tot;
	ytd_grand_tot += brk_ytd_tot;	
	
	brk_ytd_tot = 0;
	brk_ab_tot = 0;
	brk_wi_tot = 0;

	return(NOERROR);
}
/*------------------------------------------------------------------------*/
PrintGrTot()
{
	char	txt_line[132];

	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	mkln(1,"TOTAL FOR ALL EMPLOYEES:",24);
	tedit((char*)&ab_grand_tot,"__,_0_.__",txt_line,R_DOUBLE);
	mkln(68,txt_line,9);
	tedit((char*)&wi_grand_tot,"__,_0_.__",txt_line,R_DOUBLE);
	mkln(78,txt_line,9);
	tedit((char*)&ytd_grand_tot,"__,_0_.__",txt_line,R_DOUBLE);
	mkln(87,txt_line,9);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	return(NOERROR);
}
/*---------------------------------------------------------------------*/
static
GetSubName ()
{
	char 	temp_emp[13];
	
	strcpy(temp_emp,emp_rec.em_numb);
	strcpy(emp_rec.em_numb,att_hist.eah_teach);
	retval=get_employee(&emp_rec,BROWSE,0,e_mesg);

	retval=UsrBargVal(BROWSE,emp_rec.em_numb,emp_rec.em_barg,0,
								e_mesg);
	if(retval < 0)
		return(NOERROR);

	sprintf(sub_name,"%s, %s %s",emp_rec.em_last_name,
				emp_rec.em_first_name,
				emp_rec.em_mid_name);
	strcpy(sub_numb,att_hist.eah_teach);
	strcpy(emp_rec.em_numb,temp_emp);
	retval=get_employee(&emp_rec,BROWSE,0,e_mesg);
	retval=UsrBargVal(BROWSE,emp_rec.em_numb,emp_rec.em_barg,0,
								e_mesg);
	if(retval < 0)
		return(NOERROR);
	
	return(NOERROR);
}
/*-------------------------------------------------------------------------*/
static
BldEmpAbs()
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
		if(strcmp(emp_rec.em_status,"ACT") != 0) 
			continue;

	  	retval=UsrBargVal(BROWSE,emp_rec.em_numb,emp_rec.em_barg,0,
									e_mesg);
	  	if(retval < 0){
			continue;
		}
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
 	
	if(sortop2[0] == 'L'){
		sprintf(sort_array,"%-d",emp_rec.em_cc);
		strncpy(tmp_sched1.tm_sortk_1,sort_array,4);
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
