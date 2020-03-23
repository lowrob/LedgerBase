/******************************************************************************
		Sourcename   : time_sheet.c
		System       : Budgetary Financial system.
		Module       : Fixed Assets System : Reports
		Created on   : 93-09-20
		Created  By  : Andre Cormier 

******************************************************************************
About the file:	
	This file has routines to print Fixed Asset Report. 
	It is called by the file ppayrep.c .

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
#define YES		'Y'
#else
#define PRINTER		'I'
#define DISPLAY		'A'
#define FILE_IO		'D'
#define YES		'O'
#endif

static Pa_rec		pa_rec;
static Emp		emp_rec;
static Pay_param	pay_param;

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
	
static int	PG_SIZE;
static int	retval;
static char 	discfile[15];	/* for storing outputfile name */
static short	pgcnt; 		/* for page count */
static char	resp[2];	/* for storing response */
static short	copies;
static short	printer_no;

static short   d_month[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
static short   calendar[7];

extern char 	e_mesg[80];	/* for storing error messages */

time_sheet()
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
			STRCPY( discfile, "time_sheet.dat" );
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
	printer_no = 1;
	if( *resp=='P' ){
		if((retval = GetPrinter(&printer_no))<0 )
			return(retval);
		if((retval = GetNbrCopies(&copies))<0 )
			return(retval);
	}

	sortop1[0] = 'A';
	if((retval = GetSortop1(sortop1)) < 0)
		return(retval);

	sortop2[0] = 'B';
	if((retval = GetSortop2(sortop2)) < 0)
		return(retval);

	strcpy( barg1, "     0");
	strcpy( barg2, "ZZZZZZ" );
	retval = GetBargRange( barg1,barg2 );
	if(retval < 0) return(retval);

	center1 = 1;
	center2 = 9999;
	retval = GetCenterRange( &center1,&center2 );
	if(retval < 0) return(retval);

	strcpy( posi1, "     0");
	strcpy( posi2, "ZZZZZZ" );
	retval = GetPosRange( posi1,posi2 );
	if(retval < 0) return(retval);

	strcpy( class1, "     0");
	strcpy( class2, "ZZZZZZ" );
	retval = GetClassRange( class1,class2 );
	if(retval < 0) return(retval);

	strcpy( empl1, "           1");
	strcpy( empl2, "ZZZZZZZZZZZZ" );
	retval = GetEmpRange( empl1,empl2 );
	if(retval < 0) return(retval);

	date1 = 0;
	retval = GetEndDateRange( &date1 );
	if(retval < 0) return(retval);

	if( *resp=='P' ) {

		if(InitPrinter()<0) {
			return(-1);
		}

		retval = DoTestPrint() ;
		if( retval<0 ){
			return(retval);
		}

		close_rep();
	}

	if( (retval=Confirm())<=0 )
		return(retval);

	retval = get_param( &pa_rec, BROWSE, 1, e_mesg );
	if( retval<1 ){
		return(DBH_ERR);
	}

	if(InitPrinter()<0) {
		return(-1);
	}

	if( *resp=='P' )
		SetCopies( (int)copies );
	pgcnt = 0;		/* Page count is zero */
	LNSZ = 80;		/* line size in no. of chars */
	linecnt = PG_SIZE;	/* Page size in no. of lines */

	unlink_file(TMPINDX_1);
	STRCPY(tmpindxfile,"emptemp");
	get_tnum(tnum);
	strcat(tmpindxfile,tnum);
	if(CreatTempIndx(tmpindxfile,sortop1,sortop2) < 0) {
		sprintf(e_mesg,"Error Creating tmp file :%d", iserror);
		fomer(e_mesg);	get();
		return(-1);
	}

	retval = PrintRep();
	if(retval < 0)	return(retval);

	close_rep();	/* close output file */
	close_dbh();
	return(retval);
}

/*-----------------------------------------------------------------*/
InitPrinter()
{
	char	resp[2] ;
	char	discfile[15] ;

	/* Always to Printer */
	STRCPY( resp, "P" );
	discfile[0] = '\0';

	if( opn_prnt( resp, discfile, printer_no, e_mesg, 0 )<0 ){
		fomen(e_mesg); get();
		return(-1);
	}
	return(NOERROR) ;
}
/******************************************************************************
Main logic of the program
******************************************************************************/
static
PrintRep()
{
	short	day,month,year;
	int	j,i;

	retval = get_pay_param( &pay_param, BROWSE, 1, e_mesg );
	if( retval<1 ){
		return(DBH_ERR);
	}

	day = date1 % 100;
	month = date1 % 10000 / 100;
	year = date1 / 10000;

	j = 6;
	for(i=0;i<7;i++) {
		if(day == 0) {
			/* Check for leap year	*/
			if(month -1 == 02 && year % 4 == 0){
				d_month[month-2]++;		
			}
			calendar[j] = d_month[month-2];
			day = d_month[month-2]-1;
		}
		else {
			calendar[j] = day;
			day--;
		}
		j--;
	}

	emp_rec.em_numb[0] = '\0';
	emp_rec.em_first_name[0] = '\0';
	emp_rec.em_last_name[0] = '\0';

	if(sortop2[0] == 'C') {
		emp_rec.em_class[0] = '\0';
	}

	if(sortop2[0] == 'L') {
		emp_rec.em_cc = 0;
	}

	if(sortop2[0] == 'B') {
		emp_rec.em_barg[0] = '\0';
	}

	if(sortop2[0] == 'P') {
		emp_rec.em_pos[0] = '\0';
	}

	flg_reset(TMPINDX_1);

	for(;;) {
		retval = get_next((char*)&emp_rec,TMPINDX_1,0,FORWARD,BROWSE,e_mesg);

		if(retval ==EFL) 
			break;
		if(retval < 0){
			fomen(e_mesg);
			get();
			return(retval);
		}

		if(strcmp(emp_rec.em_status,"ACT") != 0)
			continue;

		if(sortop2[0] == 'L') {
			if(emp_rec.em_cc > center2)
				break;
		}
		else {
			if(emp_rec.em_cc < center1 || emp_rec.em_cc > center2)
				continue;
		}

		if(sortop2[0] == 'C') {
			if(strcmp(emp_rec.em_class,class2) > 0)
				break;
		}
		else {
			if(strcmp(emp_rec.em_class,class1) < 0 ||
			   strcmp(emp_rec.em_class,class2) > 0)
				continue;
		}

		if(sortop2[0] == 'B') {
			if(strcmp(emp_rec.em_barg,barg2) > 0)
				break;
		}
		else {
			if(strcmp(emp_rec.em_barg,barg1) < 0 ||
			   strcmp(emp_rec.em_barg,barg2) > 0)
				continue;
		}

		if(sortop2[0] == 'P') {
			if(strcmp(emp_rec.em_pos,posi2) > 0)
				break;
		}
		else {
			if(strcmp(emp_rec.em_pos,posi1) < 0 ||
			   strcmp(emp_rec.em_pos,posi2) > 0)
				continue;
		}

		if(strcmp(emp_rec.em_numb,empl1) < 0 ||
		   strcmp(emp_rec.em_numb,empl2) > 0) {
			continue;
		}

		retval = PrntSheet();
		if(retval == EXIT)	return(retval);
	}	

	seq_over(TMPINDX_1);

	return(NOERROR);
}


/******************************************************************************
Function that prints the headings 
******************************************************************************/
static
PrntSheet()
{
	char	txt_buff[132];
	int	i,j;

	if( pgcnt && term < 99)   /* new page and display */
		if(next_page()<0) return(EXIT);	
		
	pgcnt++; 			/* increment page no */

	tedit((char *)&date1, "____/__/__",txt_buff,R_LONG) ;
	mkln(64,txt_buff,10) ;

	if( PrintLine()<0 )	return(REPORT_ERR);

	mkln(44,"CC #:",5) ;
	tedit((char *)&emp_rec.em_cc, "___0",txt_buff,R_SHORT) ;
	mkln(64,txt_buff,4) ;

	if( PrintLine()<0 )	return(REPORT_ERR);

	sprintf(txt_buff,"%s %s",emp_rec.em_first_name,emp_rec.em_last_name);
	mkln(21,txt_buff,strlen(txt_buff));

	if( PrintLine()<0 )	return(REPORT_ERR);

	mkln(21,emp_rec.em_numb,12);
	mkln(69,"1      1",8);

	if( PrintLine()<0 )	return(REPORT_ERR);

	mkln(23,emp_rec.em_pos,6);

	if( PrintLine()<0 )	return(REPORT_ERR);
	if( PrintLine()<0 )	return(REPORT_ERR);
	if( PrintLine()<0 )	return(REPORT_ERR);

	j = 24;

	for(i=0;i<7;i++) {
		tedit((char *)&calendar[i], "__",txt_buff,R_SHORT) ;
		mkln(j,txt_buff,2) ;
		j+=8;
	}

	if( PrintLine()<0 )	return(REPORT_ERR);
	rite_top();

	return(NOERROR);
}
/*-----------------------------------------------------------------------*/
/*  Printing the Test as many times needed for cheques to line up        */
DoTestPrint()
{
	int 	i,j,	retval ;
	char	resp;

	for( ; ; ) {
#ifdef ENGLISH
		if((retval=fomen("Do you want a Test Print (Y/N)?"))<0)
			return(retval);
#else
		if((retval=fomen("Desirez-vous un test d'impression (O/N)?"))<0)
			return(retval);
#endif
		for(;;) {
			resp = getchar();
			retval = valid_char(resp);
			if(retval == 1 || retval == 2)	break;
		}

		if(retval == 2)	break;

		mkln(64,"9999/99/99",10) ;

		if( PrintLine()<0 )	return(REPORT_ERR);

		mkln(46,"CC #:",5) ;
		mkln(64,"9999",4) ;

		if( PrintLine()<0 )	return(REPORT_ERR);

		mkln(21,"XXXXXXXXXXXXXXXXXX",18);

		if( PrintLine()<0 )	return(REPORT_ERR);

		mkln(21,"999999999999",12);
		mkln(69,"9      9",8);

		if( PrintLine()<0 )	return(REPORT_ERR);

		mkln(23,"XXXXXX",6);

		if( PrintLine()<0 )	return(REPORT_ERR);
		if( PrintLine()<0 )	return(REPORT_ERR);
		if( PrintLine()<0 )	return(REPORT_ERR);

		j = 24;

		for(i=0;i<7;i++) {
			mkln(j,"99",2) ;
			j+=8;
		}

		if( PrintLine()<0 )	return(REPORT_ERR);

#ifdef ENGLISH
		fomen("The Test Print Is Printing. Press any key to continue.");
#else
		fomen("La copie d'essaie imprime. Appuyer une touche pour continuer.");
#endif
		get();
		rite_top();
	}
	return(NOERROR) ;
}
/*--------------------------------------------------------------------*/
static
valid_char(c) 
char	c ;
{
	if(c == 'Y' || c == 'y' )
		return(1);
	if(c == 'N' || c == 'n' )
		return(2);

	return(0) ;
}
/******************************************************************************
Function that prints every line of the report 
******************************************************************************/
static
PrintLine()
{
	if(prnt_line() < 0 )	return(REPORT_ERR);

	return(NOERROR);
}

