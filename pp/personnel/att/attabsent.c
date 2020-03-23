/******************************************************************************
		Sourcename   : attabsent.c
		System       : 
		Module       :
		Created on   : 92-03-12
		Created  By  : Littlejohn 
		Cobol Source : 

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
#define PRINTER		'P'
#define DISPLAY		'D'
#define FILE_IO		'F'
#else
#define PRINTER		'I'

#define DISPLAY		'A'
#define FILE_IO		'D'
#endif

static Att		att_rec;
static Emp_at_his	att_hist;
static Pa_rec		pa_rec;
static Sch_rec		sch_rec;

/*  Data items for storing the key range end values */
static long	date1;
static long	date2;
static int	flag;		/* flag to print the employee or not */
static int	PG_SIZE;
static int	retval;
static char 	discfile[15];	/* for storing outputfile name */
static short	pgcnt; 		/* for page count */
static char	resp[2];	/* for storing response */
static short	copies;

extern char 	e_mesg[80];	/* for storing error messages */

attabsent()
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
			STRCPY( discfile, "attabsent.dat" );
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

	date1 = get_date();
	date2 = date1 + 10000;
	if((retval = GetDateRange(&date1,&date2))<0)
		return(retval);
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

	retval = PrintRpt(date1);

	close_rep();	/* close output file */
	close_dbh();
	return(retval);
}

/******************************************************************************
Main logic of the program
******************************************************************************/
static
PrintRpt(date1)
long	date1;
{
	double	att_total = 0.0;

	/* get the cost center name */
	retval = get_param(&pa_rec,BROWSE,1,e_mesg);
	if (retval < 0){
		fomer(e_mesg);get ();
	}

	sch_rec.sc_numb = pa_rec.pa_distccno;
	retval = get_sch(&sch_rec,BROWSE,0,e_mesg);
	if (retval < 0){
		fomer(e_mesg);get();
	}

	PrntHdg();

	/*
	***************** Get the Att records *********************
	*/
	att_rec.at_disp_code[0] 	= '\0';
	att_rec.at_code[0]		= '\0';
	flg_reset(ATT);

	for(;;){
		/*
		******** Get each corresponding Att Hist record **********
		*/
		att_total = 0.0;

		retval = get_n_att(&att_rec,BROWSE,0,FORWARD,e_mesg);
		if(retval == EFL)	break;
		if(retval < 0)		return(retval);		

		att_hist.eah_numb[0] 	= '\0';
		att_hist.eah_date = 0;
		strcpy(att_hist.eah_code,att_rec.at_code);
		flg_reset(EMP_ATT);
		for(;;){
			retval = get_n_emp_at(&att_hist,BROWSE,0,FORWARD,e_mesg);
			if(retval == EFL)	break;
			if(retval < 0)		return(retval);		

			if(strcmp(att_hist.eah_code,att_rec.at_code)!=0)
				continue;

			if((att_hist.eah_date < date1) ||
		  	(att_hist.eah_date > date2))
				continue;

			att_total+=(att_hist.eah_hours/att_hist.eah_sen_hours);
		}

		if((retval = PrntRec(att_total))<0)
			return(retval);
		if(retval == EXIT)	break;
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

	name_size = FindNameSize();
 
	mkln(((LNSZ-name_size)/2),sch_rec.sc_name,name_size);
	if(prnt_line() < 0 )	return(REPORT_ERR);

#ifdef ENGLISH
	mkln((LNSZ-28)/2,"ATTENDANCE CODE TOTAL ABSENT", 28 );
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

	mkln(1,"ATTENDANCE",10);
	mkln(16,"DESCRIPTION",120);
	mkln(41,"TOTAL SICK",11);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	mkln(3,"CODE",4);
	mkln(44,"DAYS",4);

	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	return(NOERROR);
}

static
PrntRec(att_total)
double	att_total;
{
	char	txt_line[132];

	mkln(3,att_rec.at_code,3);
	mkln(7,att_rec.at_desc,30);
	sprintf(txt_line,"%3.2f",att_total);
	mkln(43,txt_line,6);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	
	if (linecnt > PG_SIZE)
		if((retval=PrntHdg()) == EXIT)
			return(retval);

	return(NOERROR);
}

static
FindNameSize ()
{
	int	x;
	
	for(x=0;x<=30;x++){
		if(sch_rec.sc_name[x] == '\0')
			break;
	}
	return(x);
}
	
/******************   END OF PROGRAM *******************************/
