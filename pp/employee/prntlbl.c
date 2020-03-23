/******************************************************************************
		Sourcename   : prntlbl.c
		System       : 
		Module       :
		Created on   : 92-SEP-06 
******************************************************************************
About the file:	
	This program will print a list of employee mailing address labels.

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
static Teach_ass	teach_ass;
static Area_spec	area_spec;
static Tmp_sched1	tmp_sched1;
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

static	char 	sched1_flag[2];

static int	flag;		/* flag to print the employee or not */
static int	PG_SIZE;
static int	retval;
static char 	discfile[15];	/* for storing outputfile name */
static short	pgcnt; 		/* for page count */
static char	resp[2];	/* for storing response */
static short	copies;

extern char 	e_mesg[200];	/* for storing error messages */

static	int	label_nbr;

static	char	lbl1_last_name[26], lbl2_last_name[26], lbl3_last_name[26],
		lbl1_first_name[16], lbl2_first_name[16], lbl3_first_name[16],
		lbl1_mid_name[16], lbl2_mid_name[16], lbl3_mid_name[16],
		lbl1_add1[31], lbl2_add1[31], lbl3_add1[31], lbl1_add2[31],
		lbl2_add2[31], lbl3_add2[31], lbl1_add3[31], lbl2_add3[31],
		lbl1_pc[11], lbl2_pc[11], lbl3_pc[11], lbl3_add3[31]; 

prntlbl()
{
	char	tmpindxfile[50];
	char	tnum[5];
	char	firstresp[2];

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
			strcpy( discfile, "prntlbl.dat" );
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

	pgcnt = 0;		/* Page count is zero */
	LNSZ = 132;		/* line size in no. of chars */
	linecnt = PG_SIZE;	/* Page size in no. of lines */

	if( *resp=='P'){
		if((retval=DisplayMessage("Are You Ready to Print Labels (Y/N)?"))<0){
			return(retval);
		}

		if((retval = GetResponse(firstresp))<0) {
			return(retval);
		}

		if( firstresp[0] == 'N' ) 
			return(0) ;

		if( opn_prnt( resp, discfile, 1, e_mesg, 0 /*do not spool*/)<0 )
			return(REPORT_ERR);
		retval = DoTestPrint();
		if(retval < 0)	return(retval);
	}

	if( (retval=Confirm())<=0 )
		return(retval);

	if( opn_prnt( resp, discfile, 1, e_mesg, 0 /* do not spool */)<0 ){
		return(REPORT_ERR);
	}
	if( *resp=='P' )
		SetCopies( (int)copies );

	if(sortop2[0]=='B'||sortop2[0]=='P'){ 
		unlink_file(TMPINDX_1);
		STRCPY(tmpindxfile,"emptemp");
		get_tnum(tnum);
		strcat(tmpindxfile,tnum);
		if(CreatTempIndx(tmpindxfile,sortop2,sortop1 )<0){
			sprintf(e_mesg,"Error Creating tmp file : %d",iserror);
			fomer(e_mesg);get();
			return (-1);
		}
	}
	if(sortop2[0]=='C'||sortop2[0]=='L'){
		unlink_file(TMP_SCHED1);/*remove temp file*/
		if(BldSchedAdd()<0){
			return (-1);
		}
	}

	retval = PrintLbl();
	if(label_nbr != 0){
		if(label_nbr == 3){
			retval = PrntAddress();
			if(retval < 0) 	return(retval);
		}
		else {
			strcpy(lbl3_first_name,
				"                         ");
			strcpy(lbl3_mid_name,
				"               ");
			strcpy(lbl3_last_name,
				"               ");
			strcpy(lbl3_add1,"                              ");
			strcpy(lbl3_add2,"                              ");
			strcpy(lbl3_add3,"                              ");
			strcpy(lbl3_pc,"          ");
			if(label_nbr == 2) {
				retval = PrntAddress();
				if(retval < 0) 	return(retval);
			}
			else {
			  strcpy(lbl2_first_name,
				"                         ");
			  strcpy(lbl2_mid_name,
				"               ");
			  strcpy(lbl2_last_name,
				"               ");
			  strcpy(lbl2_add1,"                              ");
			  strcpy(lbl2_add2,"                              ");
			  strcpy(lbl2_add3,"                              ");
			  strcpy(lbl2_pc,"          ");
			  retval = PrntAddress();
			  if(retval < 0) 	return(retval);
			}
		}
	}
	close_rep();	/* close output file */
	close_dbh();
	return(retval);
}

/*-----------------------------------------------------------------------*/
/*  Printing the Test as many times needed for labels to line up        */
DoTestPrint()
{
	int 	i,	retval ;
	char	temp_buf[132];
	char	testresp[2];

	STRCPY(temp_buf, 
	  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX") ;

	for( ; ; ) {

		if((retval=DisplayMessage("Do You Want a Test Print(Y/N)?"))<0){
			return(retval);
		}

		if((retval = GetResponse(testresp))<0) {
			return(retval);
		}

		if( testresp[0] == 'N' ) 
			break ;

		if(prnt_line() < 0 )	return(REPORT_ERR);
		mkln(1,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",35);
		mkln(37,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",35);
		mkln(74,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",35);
		if(prnt_line() < 0 )	return(REPORT_ERR);

		mkln(1,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",30);
		mkln(37,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",30);
		mkln(74,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",30);
		if(prnt_line() < 0 )	return(REPORT_ERR);

		mkln(1,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",30);
		mkln(37,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",30);
		mkln(74,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",30);
		if(prnt_line() < 0 )	return(REPORT_ERR);

		mkln(1,"XXXXXXXXXX",10);
		mkln(37,"XXXXXXXXXX",10);
		mkln(74,"XXXXXXXXXX",10);
		if(prnt_line() < 0 )	return(REPORT_ERR);
		if(prnt_line() < 0 )	return(REPORT_ERR);

		if(prnt_line() < 0 )	return(REPORT_ERR);
		mkln(1,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",35);
		mkln(37,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",35);
		mkln(74,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",35);
		if(prnt_line() < 0 )	return(REPORT_ERR);

		mkln(1,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",30);
		mkln(37,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",30);
		mkln(74,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",30);
		if(prnt_line() < 0 )	return(REPORT_ERR);

		mkln(1,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",30);
		mkln(37,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",30);
		mkln(74,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",30);
		if(prnt_line() < 0 )	return(REPORT_ERR);

		mkln(1,"XXXXXXXXXX",10);
		mkln(37,"XXXXXXXXXX",10);
		mkln(74,"XXXXXXXXXX",10);
		if(prnt_line() < 0 )	return(REPORT_ERR);
		if(prnt_line() < 0 )	return(REPORT_ERR);

		if(prnt_line() < 0 )	return(REPORT_ERR);
		mkln(1,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",35);
		mkln(37,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",35);
		mkln(74,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",35);
		if(prnt_line() < 0 )	return(REPORT_ERR);

		mkln(1,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",30);
		mkln(37,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",30);
		mkln(74,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",30);
		if(prnt_line() < 0 )	return(REPORT_ERR);

		mkln(1,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",30);
		mkln(37,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",30);
		mkln(74,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",30);
		if(prnt_line() < 0 )	return(REPORT_ERR);

		mkln(1,"XXXXXXXXXX",10);
		mkln(37,"XXXXXXXXXX",10);
		mkln(74,"XXXXXXXXXX",10);
		if(prnt_line() < 0 )	return(REPORT_ERR);
		if(prnt_line() < 0 )	return(REPORT_ERR);

		if(prnt_line() < 0 )	return(REPORT_ERR);
		mkln(1,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",35);
		mkln(37,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",35);
		mkln(74,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",35);
		if(prnt_line() < 0 )	return(REPORT_ERR);

		mkln(1,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",30);
		mkln(37,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",30);
		mkln(74,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",30);
		if(prnt_line() < 0 )	return(REPORT_ERR);

		mkln(1,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",30);
		mkln(37,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",30);
		mkln(74,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",30);
		if(prnt_line() < 0 )	return(REPORT_ERR);

		mkln(1,"XXXXXXXXXX",10);
		mkln(37,"XXXXXXXXXX",10);
		mkln(74,"XXXXXXXXXX",10);
		if(prnt_line() < 0 )	return(REPORT_ERR);
		if(prnt_line() < 0 )	return(REPORT_ERR);

#ifdef ENGLISH
		fomen("The Test Print Is Printing. Press any key to continue.");
#else
		fomen("La copie d'essaie imprime. Appuyer une touche pour continuer.");
#endif
		get();
	}
	return(NOERROR) ;
}
/******************************************************************************
Main logic of the program
******************************************************************************/
static
PrintLbl()
{
	label_nbr = 0;

	/* get the cost center name */
	retval = get_param(&pa_rec,BROWSE,1,e_mesg);
	if (retval < 0){
		fomer(e_mesg);get ();
	}
	/*
		******* Sort by Center *******
	*/
	if(sortop2[0]=='C'||sortop2[0]=='L'){
		tmp_sched1.tm_cost = 0;
		tmp_sched1.tm_class[0] = '\0';
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

	  		retval=UsrBargVal(BROWSE,emp_rec.em_numb,
						emp_rec.em_barg,0, e_mesg);
	  		if(retval < 0)
				continue;

			label_nbr++;
			switch	(label_nbr) {
			  case 1:	
				strcpy(lbl1_first_name,
					emp_rec.em_first_name);
				strcpy(lbl1_mid_name,
					emp_rec.em_mid_name);
				strcpy(lbl1_last_name,
					emp_rec.em_last_name);
				if(emp_rec.em_add1[0] == '\0')
					strcpy(lbl1_add1,emp_rec.em_add2);
				else
					strcpy(lbl1_add1,emp_rec.em_add1);
				strcpy(lbl1_add2,emp_rec.em_add3);
				strcpy(lbl1_add3,emp_rec.em_add4);
				strcpy(lbl1_pc,emp_rec.em_pc);
				break;
			  case 2:	
				strcpy(lbl2_first_name,
					emp_rec.em_first_name);
				strcpy(lbl2_mid_name,
					emp_rec.em_mid_name);
				strcpy(lbl2_last_name,
					emp_rec.em_last_name);
				if(emp_rec.em_add1[0] == '\0')
					strcpy(lbl2_add1,emp_rec.em_add2);
				else
					strcpy(lbl2_add1,emp_rec.em_add1);
				strcpy(lbl2_add2,emp_rec.em_add3);
				strcpy(lbl2_add3,emp_rec.em_add4);
				strcpy(lbl2_pc,emp_rec.em_pc);
				break;
			  case 3:	
				strcpy(lbl3_first_name,
					emp_rec.em_first_name);
				strcpy(lbl3_mid_name,
					emp_rec.em_mid_name);
				strcpy(lbl3_last_name,
					emp_rec.em_last_name);
				if(emp_rec.em_add1[0] == '\0')
					strcpy(lbl3_add1,emp_rec.em_add2);
				else
					strcpy(lbl3_add1,emp_rec.em_add1);
				strcpy(lbl3_add2,emp_rec.em_add3);
				strcpy(lbl3_add3,emp_rec.em_add4);
				strcpy(lbl3_pc,emp_rec.em_pc);
				break;
			}
					
			if(label_nbr == 3){
				if ((retval = PrntAddress())<0)
					return(retval);
				if(retval == EXIT)	break;
			}

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
			emp_rec.em_pos[0] = '\0';
		}
		if(sortop2[0] == 'P') {
			strcpy(emp_rec.em_pos,posi1);
			emp_rec.em_barg[0] = '\0';
		}
		emp_rec.em_first_name[0] = '\0';
		emp_rec.em_last_name[0] = '\0';
		flg_reset(TMPINDX_1);

		for (;;){
			retval = get_next((char*)&emp_rec,TMPINDX_1,0,FORWARD,BROWSE,e_mesg);
			if(retval==EFL)		break;
			if(retval<0)		return(-1);
	
			if(strcmp(emp_rec.em_status,"ACT") != 0) {
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

			label_nbr++;
			switch	(label_nbr) {
			  case 1:	
				strcpy(lbl1_first_name,
					emp_rec.em_first_name);
				strcpy(lbl1_mid_name,
					emp_rec.em_mid_name);
				strcpy(lbl1_last_name,
					emp_rec.em_last_name);
				if(emp_rec.em_add1[0] == '\0')
					strcpy(lbl1_add1,emp_rec.em_add2);
				else
					strcpy(lbl1_add1,emp_rec.em_add1);
				strcpy(lbl1_add2,emp_rec.em_add3);
				strcpy(lbl1_add3,emp_rec.em_add4);
				strcpy(lbl1_pc,emp_rec.em_pc);
				break;
			  case 2:	
				strcpy(lbl2_first_name,
					emp_rec.em_first_name);
				strcpy(lbl2_mid_name,
					emp_rec.em_mid_name);
				strcpy(lbl2_last_name,
					emp_rec.em_last_name);
				if(emp_rec.em_add1[0] == '\0')
					strcpy(lbl2_add1,emp_rec.em_add2);
				else
					strcpy(lbl2_add1,emp_rec.em_add1);
				strcpy(lbl2_add2,emp_rec.em_add3);
				strcpy(lbl2_add3,emp_rec.em_add4);
				strcpy(lbl2_pc,emp_rec.em_pc);
				break;
			  case 3:	
				strcpy(lbl3_first_name,
					emp_rec.em_first_name);
				strcpy(lbl3_mid_name,
					emp_rec.em_mid_name);
				strcpy(lbl3_last_name,
					emp_rec.em_last_name);
				if(emp_rec.em_add1[0] == '\0')
					strcpy(lbl3_add1,emp_rec.em_add2);
				else
					strcpy(lbl3_add1,emp_rec.em_add1);
				strcpy(lbl3_add2,emp_rec.em_add3);
				strcpy(lbl3_add3,emp_rec.em_add4);
				strcpy(lbl3_pc,emp_rec.em_pc);
				break;
			}

			if(label_nbr == 3) {
				if ((retval = PrntAddress())<0)
					return(retval);
				if(retval == EXIT)	break;
			}

		} /*end of endless for loop*/
		seq_over(TMPINDX_1);	
	}
	if(pgcnt){
		if(term<99)
			last_page();
	}
	return(NOERROR);
}
/******************************************************************************
Prints the headings of the labels 
******************************************************************************/
static
PrntHdg()	/* Print heading  */
{
	short	offset;
	long	sysdt ;
	short	name_size;

	if( pgcnt && term < 99)   /* new page and display */
		if(next_page()<0) return(EXIT);	
		
	if( pgcnt || term < 99) { /* if not the first page or display */
		if( rite_top()<0 ) return( -1 );	/* form_feed */
	}
	else
		linecnt = 0;
	pgcnt++; 			/* increment page no */

	return(NOERROR);
}
/*----------------------------------------------------------------------*/
static
PrntAddress()
{
	char	txt_line[132];

	if(prnt_line() < 0 )	return(REPORT_ERR);
	sprintf(txt_line,"%s %s %s",
		lbl1_first_name,
		lbl1_mid_name,
		lbl1_last_name);
	mkln(1,txt_line,35);
	sprintf(txt_line,"%s %s %s",
		lbl2_first_name,
		lbl2_mid_name,
		lbl2_last_name);
	mkln(37,txt_line,35);
	sprintf(txt_line,"%s %s %s",
		lbl3_first_name,
		lbl3_mid_name,
		lbl3_last_name);
	mkln(74,txt_line,35);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(1,lbl1_add1,30);
	mkln(37,lbl2_add1,30);
	mkln(74,lbl3_add1,30);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	sprintf(txt_line,"%s, %s",lbl1_add2,
				  lbl1_add3);
	mkln(1,txt_line,30);
	sprintf(txt_line,"%s, %s",lbl2_add2,
				  lbl2_add3);
	mkln(37,txt_line,30);
	sprintf(txt_line,"%s, %s",lbl3_add2,
				  lbl3_add3);
	mkln(74,txt_line,30);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(1,lbl1_pc,10);
	mkln(37,lbl2_pc,10);
	mkln(74,lbl3_pc,10);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	label_nbr = 0;

	if (linecnt > PG_SIZE && resp[0] == 'D')
		if((retval=PrntHdg()) == EXIT)
			return(retval);

	return(NOERROR);
}
/*-----------------------------------------------------------------------*/
static
BldSchedAdd ()
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

	  	retval=UsrBargVal(BROWSE,emp_rec.em_numb,
					emp_rec.em_barg,0, e_mesg);
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
