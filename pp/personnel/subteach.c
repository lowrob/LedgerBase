/******************************************************************************
		Sourcename   : subteach.c
		System       : PERSONNEL/PAYROLL 
		Module       : EMPLOYEE
		Created on   : 92-SEP-06
		Created  By  : m. galvin 

******************************************************************************
About the file:	
	This program will print a list of the district's active supply 
	teachers.

Histor:
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

static Pa_rec		pa_rec;
static Sch_rec		sch_rec;
static Position		position;
static Emp		emp_rec;
static Class		class;

/*  Data items for storing the key range end values */
static char	sortop1[2];
static int	flag;		/* flag to print the employee or not */
static int	PG_SIZE;
static int	retval;
static char 	discfile[15];	/* for storing outputfile name */
static short	pgcnt; 		/* for page count */
static char	resp[2];	/* for storing response */
static short	copies;

extern char 	e_mesg[80];	/* for storing error messages */

static char	subtype[2];

subteach()
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
			STRCPY( discfile, "subteach.dat" );
			if((retval = GetFilename(discfile))<0 )
				return(retval);
			PG_SIZE = 60;
			break;
		case PRINTER:
		default:
			resp[0]='P';
			discfile[0]= '\0';
			PG_SIZE = 60;
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

	if((retval = DisplayMessage("For Certificate Holders, Local Permit Holders or Both (C/L/B)?"))<0) {
		return(retval);
	}

	if((retval = GetResponse(subtype))<0) {
		return(retval);
	}

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
	int	key, subteacher;

	/* get the cost center name */
	retval = get_param(&pa_rec,BROWSE,1,e_mesg);
	if (retval < 0){
		fomer(e_mesg);get ();
	}

	PrntHdg();
	if(sortop1[0] == 'A'){
		key = 4;
		emp_rec.em_last_name[0] = '\0';
		emp_rec.em_first_name[0] = '\0';
	}
	else{
		key = 0;
		emp_rec.em_numb[0] = '\0';
	}
	flg_reset(EMPLOYEE);

	for(;;){
		retval = get_n_employee(&emp_rec,BROWSE,key,FORWARD,e_mesg);
		if(retval == EFL)	break;
		if(retval < 0)		return(retval);		

	  	retval=UsrBargVal(BROWSE,emp_rec.em_numb,emp_rec.em_barg,0,
									e_mesg);
	  	if(retval < 0)
			continue;

		if(strcmp(emp_rec.em_status,"ACT")!=0)
			continue;

		subteacher = 0;
		/* fredericton district codes */ 
		if(strncmp(emp_rec.em_class,"0101S",5) == 0) 
			subteacher = 1;

		if(emp_rec.em_religion[0] == 'S')
			subteacher = 1;

		if(subtype[0] == 'C'){
			if((strcmp(emp_rec.em_cert,"    18")==0) ||
			  (strcmp(emp_rec.em_cert,"    21")==0) ||
			  (strcmp(emp_rec.em_cert,"    22")==0) ||
			  (strcmp(emp_rec.em_cert,"    23")==0) ||
			  (strcmp(emp_rec.em_cert,"    24")==0) ||
			  (strcmp(emp_rec.em_cert,"    25")==0) ||
			  (strcmp(emp_rec.em_cert,"    26")==0)) { 
				continue;
			}
		}
		if(subtype[0] == 'L'){
			if((strcmp(emp_rec.em_cert,"    18")!=0) &&
			  (strcmp(emp_rec.em_cert,"    21")!=0) &&
			  (strcmp(emp_rec.em_cert,"    22")!=0) &&
			  (strcmp(emp_rec.em_cert,"    23")!=0) &&
			  (strcmp(emp_rec.em_cert,"    24")!=0) &&
			  (strcmp(emp_rec.em_cert,"    25")!=0) &&
			  (strcmp(emp_rec.em_cert,"    26")!=0)){ 
				continue;
			}
		}

		/* saint john's codes 

		if(emp_rec.em_religion[1] == 'S')
			subteacher = 1;

		if(strncmp(emp_rec.em_class,"01018",5) == 0) 
			subteacher = 1;
		if(strncmp(emp_rec.em_class," 01018",6) == 0)
			subteacher = 1;

		if(subtype[0] == 'C'){
			if(strncmp(emp_rec.em_cert,"LP",2)==0) 
				continue;
		}
		if(subtype[0] == 'L'){
			if(strncmp(emp_rec.em_cert,"LP",2)!=0) 
				continue;
		}
		*/
	
		if(subteacher == 0)
			continue;
			
		if (linecnt > PG_SIZE)
			if((retval=PrntHdg()) == EXIT)
				return(retval);

		if((retval = PrntRec())<0)
			return(retval);
		if(retval == EXIT)	break;
	}
	seq_over(EMPLOYEE);

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
	mkln((LNSZ-23)/2,"LIST OF SUPPLY TEACHERS", 23 );
#else
	mkln((LNSZ-23)/2,"TRANSLATE        ", 23 );
#endif
	if(prnt_line() < 0 )	return(REPORT_ERR);

	if(subtype[0] == 'C')
		mkln((LNSZ-17)/2,"WITH CERTIFICATES", 17 );
	if(subtype[0] == 'L')
		mkln((LNSZ-18)/2,"WITH LOCAL PERMITS", 18 );
	if(subtype[0] == 'B')
		mkln((LNSZ-34)/2,"WITH CERTIFICATES OR LOCAL PERMITS", 34 );

	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);
		
	mkln(3,"EMPLOYEE",8);
	mkln(20,"NAME",4);
	mkln(50,"ADDRESS",7);
	mkln(77,"TELEPHONE",9);
	mkln(92,"CERT",4);
	mkln(98,"LEVEL",5);
	mkln(110,"COMMENT",7);

	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(4,"NUMBER",6);
	mkln(78,"NUMBER",6);

	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	return(NOERROR);
}
/*-------------------------------------------------------------------*/
static
PrntRec()
{
	char	txt_line[132];

	mkln(1,emp_rec.em_numb,12);
	sprintf(txt_line,"%s, %s %s",
				emp_rec.em_last_name,
				emp_rec.em_first_name,
				emp_rec.em_mid_name);
	mkln(14,txt_line,30);
	if(emp_rec.em_add1[0] != '\0')
		mkln(45,emp_rec.em_add1,30);
	else
		mkln(45,emp_rec.em_add2,30);
	mkln(76,emp_rec.em_phone,3);
	mkln(79,"-",1);
	mkln(80,emp_rec.em_phone+3,3);
	mkln(83,"-",1);
	mkln(84,emp_rec.em_phone+6,4);
	mkln(92,emp_rec.em_cert,6);
	mkln(99,emp_rec.em_level,2);
	mkln(102,emp_rec.em_com,30);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	sprintf(txt_line,"%s, %s",
				emp_rec.em_add3,
				emp_rec.em_add4);
	mkln(45,txt_line,30);
	if(strlen(emp_rec.em_com) > 30)
		mkln(102,emp_rec.em_com+30,21);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	mkln(45,emp_rec.em_pc,10);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	return(NOERROR);
}
/******************   END OF PROGRAM *******************************/
