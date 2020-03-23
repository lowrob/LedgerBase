/******************************************************************************
		Sourcename   : empperc.c
		System       : Budgetary Financial system.
		Module       : Fixed Assets System : Reports
		Created on   : 92-06-14
		Created  By  : Andre Cormier 
******************************************************************************
About the file:	
	This file has routines to print Fixed Asset Report. 
	It is called by the file pperrep.c .

History:
Programmer      Last change on    Details

******************************************************************************/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_pp.h>
#include <bfs_com.h>
#include <repdef.h>

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

static Pa_rec		pa_rec;
static Sch_rec		sch_rec;
static Emp		emp_rec;
static Class		class;
static Emp_sched1	emp_sched1;
static Tmp_sched1	tmp_sched1;
static Pay_param	pay_param;
static Position		position;
static Barg_unit	barg_unit;

static char	sortop1[2];
static char	sortop2[2];
static char	posi1[7];
static char	posi2[7];
static char	class1[7];
static char	class2[7];
static char	empl1[13];
static char	empl2[13];

static	char 	sched1_flag[2];

/*  Data items for storing the key range end values */
static char	barg1[7];
static char	barg2[7];
static short	center1;
static short	center2;
	
static int	PG_SIZE;
static int	retval;
static char 	discfile[15];	/* for storing outputfile name */
static short	pgcnt; 		/* for page count */
static char	resp[2];	/* for storing response */
static short	copies;

static	short	tmp_cc;
static	char	tmp_barg[7];
static	double	tot_barg,tot_cc,total,grand_total;
static	double	barg_total = 0;

extern char 	e_mesg[80];	/* for storing error messages */

empperc()
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
			PG_SIZE = 22;
			resp[0]='D';
			STRCPY( discfile, terminal );
			break;
		case FILE_IO:
			PG_SIZE = 63;
			resp[0]='F';
			STRCPY( discfile, "empperc.dat" );
			if((retval = GetFilename(discfile))<0 )
				return(retval);
			break;
		case PRINTER:
		default:
			PG_SIZE = 63;
			resp[0]='P';
			discfile[0]= '\0';
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

	strcpy( barg1, "     1");
	strcpy( barg2, "ZZZZZZ" );
	retval = GetBargRange( barg1,barg2 );
	if(retval < 0) return(retval);
	if(strcmp( barg1, "     1")==0)
		barg1[0] = '\0';

	center1 = 0001;
	center2 = 9999;
	retval = GetCenterRange( &center1,&center2 );
	if(retval < 0) return(retval);

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

	strcpy( empl1, "           1");
	strcpy( empl2, "ZZZZZZZZZZZZ" );
	retval = GetEmpRange( empl1,empl2 );
	if(retval < 0) return(retval);

	if( (retval=Confirm())<=0 )
		return(retval);

	retval = get_param( &pa_rec, BROWSE, 1, e_mesg );
	if( retval<1 ){
		return(DBH_ERR);
	}

	if( opn_prnt( resp, discfile, 1, e_mesg, 1 /* spool */)<0 ){
		return(REPORT_ERR);
	}
	if( *resp=='P' )
		SetCopies( (int)copies );
	pgcnt = 0;		/* Page count is zero */
	LNSZ = 80;		/* line size in no. of chars */
	linecnt = PG_SIZE;	/* Page size in no. of lines */

	if(sortop2[0]=='B'||sortop2[0]=='P'){ 
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

	if(sortop2[0]=='C'||sortop2[0]=='L'){
		unlink_file(TMP_SCHED1);/*remove temp file*/
		if(BldEmpPerc()<0){
			return (-1);
		}
	}
	retval = PrintRep();
	if(retval < 0)	return(retval);

	if(retval != EXIT){
		retval = PrintSubTot();
		if(retval < 0)	return(retval);

		retval = PrintTot();
		if(retval < 0)	return(retval);
	}

	close_rep();	/* close output file */
	close_dbh();
	return(retval);
}
/******************************************************************************
Main logic of the program
******************************************************************************/
static
PrintRep()
{
	short	last_center;
	char	last_barg[7], last_pos[7], last_class[7];
	int	first_time = 0, new_barg;

	retval = get_pay_param( &pay_param, BROWSE, 1, e_mesg );
	if( retval<1 ){
		return(DBH_ERR);
	}

	if(sortop2[0] == 'C' || sortop2[0] == 'L') {
		tmp_sched1.tm_cost = 0;
		tmp_sched1.tm_class[0] = '\0';
		strcpy(last_class,tmp_sched1.tm_class);
		last_center = tmp_sched1.tm_cost;
		tmp_sched1.tm_sortk_1[0] = '\0';
		tmp_sched1.tm_sortk_2[0] = '\0';
		tmp_sched1.tm_sortk_3[0] = '\0';

		flg_reset(TMP_SCHED1);

		for(;;) {
			retval = get_n_tmp_sched1(&tmp_sched1,BROWSE,1,
								FORWARD,e_mesg);
			if(retval ==EFL) 
				break;
			if(retval < 0){
				fomen(e_mesg);
				get();
				return(retval);
			}

			strcpy(emp_rec.em_numb,tmp_sched1.tm_numb);
			retval = get_employee(&emp_rec,BROWSE,0,e_mesg);
			if(retval < 0) {
				fomen(e_mesg);
				get();
				continue;
			}

			if(sortop2[0] == 'C'){
				if(strcmp(last_class,tmp_sched1.tm_class)!=0 || 
				   first_time == 0 || linecnt > PG_SIZE) {
					if(first_time ==1 && strcmp(last_class,
					   tmp_sched1.tm_class)!=0) {
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
			   	   first_time == 0 || linecnt > PG_SIZE ||
			      	   strcmp(last_barg,emp_rec.em_barg)!=0) { 
					new_barg = 0;
					if(last_center != tmp_sched1.tm_cost &&
					   first_time == 1) {
						retval = PrintBargTot();
						if(retval < 0) {
							return(retval);
						}
						retval = PrintSubTot();
						if(retval < 0) {
							return(retval);
						}
					}
					else {
						if(strcmp(last_barg,
						   emp_rec.em_barg)!=0 &&
						   first_time==1){
							new_barg = 1;
							retval = PrintBargTot();
							if(retval < 0)
								return(retval);
						}
					}
					if(new_barg != 1) {
						retval = PrntHdg();
						if(retval < 0) 	{
							return(retval);	
						}
					}
					if(retval == EXIT)	return(retval);
					first_time = 1;
				}
			}
			strcpy(tmp_barg,emp_rec.em_barg);
			tmp_cc = tmp_sched1.tm_cost;
			if ((retval = PrntRec())<0)
				return(retval);
			if(retval == EXIT)	return(retval);
			last_center = tmp_sched1.tm_cost;
			strcpy(last_class,tmp_sched1.tm_class);
			strcpy(last_barg,emp_rec.em_barg);
		}

		seq_over(TMP_SCHED1);
	}
/*-------------------  Bargaining Unit or Position Sort Option -------------*/
	if(sortop2[0]=='B'||sortop2[0]== 'P'){
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
		emp_rec.em_last_name[0] = '\0';
		emp_rec.em_first_name[0] = '\0';

		flg_reset(TMPINDX_1);

		for (;;){
			retval = get_next((char*)&emp_rec,TMPINDX_1,0,FORWARD,BROWSE,e_mesg);

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
		   	strcmp(emp_rec.em_numb,empl2) > 0 ) {
				continue;
			}

			if((emp_rec.em_cc < center1) ||
		   	  (emp_rec.em_cc > center2 )) 
				continue;

			if(strcmp(emp_rec.em_class,class1) < 0 ||
		   	   strcmp(emp_rec.em_class,class2) > 0 ) 
				continue;

			if(sortop2[0] == 'B') {
				if(strcmp(last_barg,emp_rec.em_barg)!=0 ||
				   first_time == 0 || linecnt > PG_SIZE) {
					if(first_time == 1 && strcmp(last_barg,
					   emp_rec.em_barg)!=0) {
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
				if(strcmp(last_pos,emp_rec.em_pos)!=0 ||
				   first_time == 0 || linecnt > PG_SIZE) { 
					if(strcmp(last_pos,emp_rec.em_pos)!=0 &&
					   first_time == 1) {
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
			tmp_cc = emp_rec.em_cc;
			strcpy(tmp_barg,emp_rec.em_barg);
			retval = PrntRec();
			if(retval == EXIT)	return(retval);
			strcpy(last_pos,emp_rec.em_pos);
			strcpy(last_barg,emp_rec.em_barg);

		}/*End of the endless loop*/

		seq_over(TMPINDX_1);
	}
	if(pgcnt) {
		if(term < 99) 
			last_page();	
#ifndef	SPOOLER
		else
			rite_top();
#endif
	}
	return(NOERROR);
}
/*----------------------------------------------------------------*/
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
	mkln( 51, "Date:", 5 );
#else
	mkln( 51, "Date:", 5 );
#endif
	sysdt = get_date() ;
	tedit( (char *)&sysdt,"____/__/__",line+cur_pos, R_LONG ); 
	cur_pos += 10;

#ifdef ENGLISH
	mkln( 70, "PAGE:", 5 );
#else
	mkln( 70, "PAGE:", 5 );
#endif
	tedit( (char *)&pgcnt,"__0_",  line+cur_pos, R_SHORT ); 
	cur_pos += 4;
	if( prnt_line()<0 )	return(REPORT_ERR);

	offset = ( LNSZ-strlen(pa_rec.pa_co_name) )/2;
	mkln( offset, pa_rec.pa_co_name, strlen(pa_rec.pa_co_name) );
	if( prnt_line()<0 )	return(REPORT_ERR);

#ifdef ENGLISH
	mkln((LNSZ-30)/2,"LIST OF EMPLOYEE'S PERCENTAGES", 30 );
#else
	mkln((LNSZ-30)/2,"                              ", 30 );
#endif
	if( prnt_line()<0 )	return(REPORT_ERR);
	if( prnt_line()<0 )	return(REPORT_ERR);

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

	if(sortop2[0] == 'P' || sortop2[0] == 'L'){
		mkln(2,"COST",4);
		mkln(11,"BARGAINING",10);
		mkln(25,"EMPLOYEE",8);
		mkln(37,"EMPLOYEE",8);
	
		if( prnt_line()<0 )	return(REPORT_ERR);

		mkln(1,"CENTER",6);
		mkln(14,"UNIT",4);
		mkln(25,"NUMBER",6);
		mkln(39,"NAME",4);
		mkln(67,"PERCENTAGE",10);
	}
	if(sortop2[0] == 'B'){
		mkln(2,"COST",4);
		mkln(15,"EMPLOYEE",8);
		mkln(27,"EMPLOYEE",8);
	
		if( prnt_line()<0 )	return(REPORT_ERR);

		mkln(1,"CENTER",6);
		mkln(16,"NUMBER",6);
		mkln(29,"NAME",4);
		mkln(56,"PERCENTAGE",10);
	}

	if(sortop2[0] == 'C'){
		mkln(1,"BARGAINING",10);
		mkln(15,"EMPLOYEE",8);
		mkln(27,"EMPLOYEE",8);
	
		if( prnt_line()<0 )	return(REPORT_ERR);

		mkln(3,"UNIT",4);
		mkln(16,"NUMBER",6);
		mkln(29,"NAME",4);
		mkln(56,"PERCENTAGE",10);
	}

	if( prnt_line()<0 )	return(REPORT_ERR);
	if( prnt_line()<0 )	return(REPORT_ERR);

	return(NOERROR);
}
/******************************************************************************
Function that prints the headings 
******************************************************************************/
static
PrntRec()
{
	char	txt_buff[132];

	if(sortop2[0] == 'P' || sortop2[0] == 'L') {
		tedit((char *)&tmp_cc, "__0_",txt_buff,R_SHORT) ;
		mkln(2,txt_buff,4) ;

		mkln(13,tmp_barg,6);

		mkln(23,emp_rec.em_numb,12);
		sprintf(txt_buff,"%s, %s",emp_rec.em_last_name,emp_rec.em_first_name);
		mkln(37,txt_buff,strlen(txt_buff));

		tedit((char *)&emp_rec.em_perc, "_0_.___",txt_buff,R_DOUBLE) ;
		mkln(69,txt_buff,7) ;
		mkln(76,"%",1);
	}

	if(sortop2[0] == 'C') {

		mkln(1,tmp_barg,6);

		mkln(8,emp_rec.em_numb,12);
		sprintf(txt_buff,"%s, %s %s",
				emp_rec.em_last_name,
				emp_rec.em_first_name,
				emp_rec.em_mid_name);
		mkln(23,txt_buff,strlen(txt_buff));

		tedit((char *)&emp_rec.em_perc, "_0_.___",txt_buff,R_DOUBLE) ;
		mkln(59,txt_buff,7) ;
		mkln(66,"%",1);
	}

	if(sortop2[0] == 'B') {
		tedit((char *)&tmp_cc, "__0_",txt_buff,R_SHORT) ;
		mkln(2,txt_buff,4) ;

		mkln(13,emp_rec.em_numb,12);
		sprintf(txt_buff,"%s, %s %s",
				emp_rec.em_last_name,
				emp_rec.em_first_name,
				emp_rec.em_mid_name);
		mkln(27,txt_buff,strlen(txt_buff));

		tedit((char *)&emp_rec.em_perc, "_0_.___",txt_buff,R_DOUBLE) ;
		mkln(59,txt_buff,7) ;
		mkln(66,"%",1);
	}

	if( prnt_line()<0 )	return(REPORT_ERR);

	total += emp_rec.em_perc / 100;

	barg_total += emp_rec.em_perc / 100;

	return(NOERROR);
}
/******************************************************************************
Function that prints the headings 
******************************************************************************/
static
PrintSubTot()
{
	char	txt_buff[132];

	if( prnt_line()<0 )	return(REPORT_ERR);

	if(sortop2[0] == 'L')
		mkln(1,"COST CENTER SUBTOTAL:",21);
	if(sortop2[0] == 'B')
		mkln(1,"BARGAINING UNIT SUBTOTAL:",25);
	if(sortop2[0] == 'P')
		mkln(1,"POSITION SUBTOTAL:",18);
	if(sortop2[0] == 'C')
		mkln(1,"CLASS CODE SUBTOTAL:",20);
	tedit((char *)&total, "_,_0_.___",txt_buff,R_DOUBLE) ;
	if(sortop2[0] == 'L') 
		mkln(67,txt_buff,9);
	mkln(54,txt_buff,9) ;

	if( prnt_line()<0 )	return(REPORT_ERR);
	if( prnt_line()<0 )	return(REPORT_ERR);

	grand_total += total;

	total = 0;

	return(NOERROR);
}
/******************************************************************************
Function that prints the subtotals 
******************************************************************************/
static
PrintBargTot()
{
	char	txt_buff[132];

	if( prnt_line()<0 )	return(REPORT_ERR);

	mkln(1,"BARGAINING UNIT SUBTOTAL:",25);
	tedit((char *)&barg_total, "_,_0_.___",txt_buff,R_DOUBLE) ;
	mkln(67,txt_buff,9);

	if( prnt_line()<0 )	return(REPORT_ERR);
	if( prnt_line()<0 )	return(REPORT_ERR);

	barg_total = 0;

	return(NOERROR);
}
/******************************************************************************
Function that prints the headings 
******************************************************************************/
static
PrintTot()
{
	char	txt_buff[132];

	if( prnt_line()<0 )	return(REPORT_ERR);

	mkln(1,"GRAND TOTAL:",12);

	tedit((char *)&grand_total, "_,_0_.___",txt_buff,R_DOUBLE) ;
	if(sortop2[0] == 'L')
		mkln(67,txt_buff,9);
	mkln(54,txt_buff,9) ;

	if( prnt_line()<0 )	return(REPORT_ERR);
	if( prnt_line()<0 )	return(REPORT_ERR);

	grand_total = 0;

	return(NOERROR);
}
/*----------------------------------------------------------------*/ 
BldEmpPerc ()
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
			sprintf(sort_array,"%-d%s",emp_sched1.es_cost,
						   emp_rec.em_barg);
		}
		else {
			sprintf(sort_array,"%-d%s",emp_rec.em_cc,
						   emp_rec.em_barg);
		}
		strcpy(tmp_sched1.tm_sortk_1,sort_array);
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
/*--------------------end of program--------------------------------------*/
