/******************************************************************************
		Sourcename   : neg_pay.c
		System       : Budgetary Financial system.
		Module       : Fixed Assets System : Reports
		Created on   : 91-10-30
		Created  By  : Andre Cormier 
		Cobol Source : 

******************************************************************************
About the file:	
	This file has routines to print Fixed Asset Report. 
	It is called by the file pprep.c .

History:
Programmer      Last change on    Details
m. galvin	92-JUL-18	  Added page break and quit checks.  
L.Robichaud	97-AUG-18	Re-designed report output and added pay advance
				calculation.

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

static Pa_rec	pa_rec;
static Emp	emp;
static Barg_unit	barg_unit;
static Pay_earn	pay_earn;
static Earn	earn;
static Pp_ben	pp_ben;
static Pay_ded	pay_ded;
static Time	time;
static Pay_param	pay_param;
static Deduction	deduction;
static	Man_chq	man_chq;

/*  Data items for storing the key range end values */
static char	barg1[7];
static char	barg2[7];
static char	posi1[7];
static char	posi2[7];
static char	empl1[13];
static char	empl2[13];
	
static int	PG_SIZE;
static int	retval;
static char 	discfile[15];	/* for storing outputfile name */
static short	pgcnt; 		/* for page count */
static char	resp[2];	/* for storing response */
static short	copies;
static	double	man_amount;	/* Track any payadvances for or against net */

extern char 	e_mesg[80];	/* for storing error messages */

neg_pay()
{

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
			PG_SIZE = 22;
			break;
		case FILE_IO:
			resp[0]='F';
			STRCPY( discfile, "neg_pay.dat" );
			if((retval = GetFilename(discfile))<0 )
				return(retval);
			PG_SIZE = 63;
			break;
		case PRINTER:
		default:
			resp[0]='P';
			discfile[0]= '\0';
			PG_SIZE = 63;
			break;
	}

	copies = 1;
	if( *resp=='P' ){
		if((retval = GetNbrCopies(&copies))<0 )
			return(retval);
	}

	strcpy( barg1, "     0");
	strcpy( barg2, "ZZZZZZ" );
	retval = GetBargRange( barg1,barg2 );
	if(retval < 0) return(retval);

	strcpy( posi1, "     0");
	strcpy( posi2, "ZZZZZZ" );
	retval = GetPosRange( posi1,posi2 );
	if(retval < 0) return(retval);

	strcpy( empl1, "           1");
	strcpy( empl2, "999999999999" );
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
	LNSZ = 132;		/* line size in no. of chars */
	linecnt = PG_SIZE;	/* Page size in no. of lines */

	retval = PrintRep();
	close_rep();	/* close output file */
	close_dbh();
	return(retval);
}

static
PrintRep()
{

	PrntHdg();

	pay_earn.pe_numb[0] = '\0';
	pay_earn.pe_pp = 0;
	pay_earn.pe_date = 0;

	flg_reset(PP_EARN);

	for(;;) {

		retval = get_n_pp_earn(&pay_earn,BROWSE,0,FORWARD,e_mesg);

		if(retval == EFL) break;
		if(retval < 0) {
			fomen(e_mesg);
			get();
			roll_back(e_mesg);
			return(-1);
		}
		strcpy(emp.em_numb,pay_earn.pe_numb);
		retval = get_employee(&emp,BROWSE,0,e_mesg);
		if(retval < 0) {
			fomen(e_mesg);
			get();
			roll_back(e_mesg);
			return(-1);
		}

	/* Subtract/add any pay_advances from the net pay */
		if(retval = ProcessAdv() < 0)
			return(retval);

		if(pay_earn.pe_net + man_amount >= 0)
			continue;

		if(strcmp(emp.em_barg,barg1) < 0 ||
		   strcmp(emp.em_barg,barg2) > 0) {
			roll_back(e_mesg);
			continue;
		}

		if(strcmp(emp.em_pos,posi1) < 0 ||
		   strcmp(emp.em_pos,posi2) > 0) {
			roll_back(e_mesg);
			continue;
		}

		if(strcmp(emp.em_numb,empl1) < 0 ||
		   strcmp(emp.em_numb,empl2) > 0) {
			roll_back(e_mesg);
			continue;
		}

		strcpy(barg_unit.b_code,emp.em_barg);
		barg_unit.b_date = get_date();
		flg_reset(BARG);

		retval = get_n_barg(&barg_unit,BROWSE,0,BACKWARD,e_mesg);
		if(retval == EFL ||
			strcmp(barg_unit.b_code, emp.em_barg) != 0){
			sprintf(e_mesg,"Bargaining Unit does no exist: %s",
				emp.em_barg);
			fomer(e_mesg);
			return(NOERROR);
		}
		if(retval < 0){
			fomer(e_mesg);
  			return(ERROR);
		}
		seq_over(BARG);

		retval = PrntRec();
		if(retval < 0)		return(retval);
		if(retval == EXIT)  return(EXIT);
	}	
	seq_over(PP_EARN);

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

/* Process manual cheque associated with employee cheque */
static
ProcessAdv()
{
	int	retval;

	man_amount = 0;
	strcpy(man_chq.mc_emp_numb,emp.em_numb);
	man_chq.mc_date = pay_earn.pe_date;
	flg_reset(MAN_CHQ);

	/* get all the advances and add together */
	for(;;){
		retval = get_n_man_chq(&man_chq,BROWSE,1,FORWARD,e_mesg) ;
		if( retval == EFL)
			break;
		if( retval < 0 ) {
			fomen(e_mesg);
			get();
			return(retval);
			}
		if(strcmp(man_chq.mc_emp_numb,emp.em_numb)==0 &&
		  man_chq.mc_date == pay_earn.pe_date)
			man_amount += man_chq.mc_amount;
		else
			break;
	}

	man_chq.mc_chq_numb = pay_earn.pe_date;
	flg_reset(MAN_CHQ);
	/* Get any adjustments for this cheque */
	for(;;){
		retval = get_n_man_chq(&man_chq,BROWSE,2,FORWARD,e_mesg) ;
		if( retval == EFL)
			break;
		if( retval < 0 ) {
			fomen(e_mesg);
			get();
			return(retval);
		}
		if(strcmp(man_chq.mc_emp_numb,emp.em_numb)==0 &&
		  man_chq.mc_chq_numb == pay_earn.pe_date)
			man_amount += (man_chq.mc_amount * -1);
		else
			if(man_chq.mc_chq_numb > pay_earn.pe_date)
				break;
	}
	return(NOERROR);
}		
static
PrntHdg()	/* Print heading  */
{
	short	offset;
	long	sysdt ;

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
	if( prnt_line()<0 )	return(REPORT_ERR);

	offset = ( LNSZ-strlen(pa_rec.pa_co_name) )/2;
	mkln( offset, pa_rec.pa_co_name, strlen(pa_rec.pa_co_name) );
	if( prnt_line()<0 )	return(REPORT_ERR);

#ifdef ENGLISH
	mkln((LNSZ-28)/2,"NEGATIVE PAY PERIOD REGISTER", 28 );
#else
	mkln((LNSZ-19)/2,"PAY PERIOD REGISTER", 19 );
#endif
	if( prnt_line()<0 )	return(REPORT_ERR);
	if( prnt_line()<0 )	return(REPORT_ERR);

	mkln(1,"EMPLOYEE #",10);
	mkln(20,"NAME",4);
	mkln(70,"ADV. AMOUNT",11);
	mkln(86,"PAY AMOUNT",10);
	mkln(110,"NET",3);
	if( prnt_line()<0 )	return(REPORT_ERR);
	if( prnt_line()<0 )	return(REPORT_ERR);

	return(NOERROR);
}

static
PrntRec()
{
	char	txt_buff[80];
	double neg_amount;

	mkln(1,emp.em_numb,12);
	sprintf(txt_buff,"%s %s",emp.em_first_name,emp.em_last_name);
	mkln(15,txt_buff,strlen(txt_buff));
	tedit((char*)&man_amount,"_,___,_0_.__-",txt_buff,R_DOUBLE);
	mkln(68,txt_buff,13);
	tedit((char*)&pay_earn.pe_net,"_,___,_0_.__-",txt_buff,R_DOUBLE);
	mkln(84,txt_buff,13);
	
	neg_amount = pay_earn.pe_net + man_amount;
	tedit((char*)&neg_amount,"_,___,_0_.__-",txt_buff,R_DOUBLE);
	mkln(106,txt_buff,13);
	if( PrintLine()<0 )	return(REPORT_ERR);

	return(NOERROR);
}
static
PrintLine()
{
	if(prnt_line() < 0 )	return(REPORT_ERR);

	if(linecnt > PG_SIZE)
		retval = PrntHdg();
 		if(retval < 0)	return(-1);
		if(retval == EXIT) 	return(EXIT);

	return(NOERROR);
}

