/******************************************************************************
		Sourcename    : padvpurge.c
		System        : Budgetary Financial system.
		Module        : Payroll
		Created on    : April 95
		Created  By   : Louis Robichaud
*******************************************************************************
About this program:
	This routine is called from the monthend program and its function is
to purge any payadvances that have been processed over 30 days old.
It will read the entire man_cheque file and check two dates to see if they
are both over 30 days old.

HISTORY:
 Date           Programmer     Description of modification
____/__/__      __________     ___________________________
*******************************************************************************/
#include <stdio.h>

#include <reports.h>
#include <bfs_defs.h>
#include <bfs_com.h>

static char e_mesg[80]; /* to store error messages */
static	short	pgcnt;
static 	long	sysdate;

static	Man_chq	man_chq;
static	Emp	emp;
static	Pa_rec	pa_rec;

/*-----------------------------------------------------------------------*/
padvpurge()
{
	int	retval;
	long 	expire_date;

	retval = get_param( &pa_rec, BROWSE, 1, e_mesg );
	if( retval<1 ){
		printf("\n\n%s", e_mesg);
		return(DBH_ERR);
	}

	/* Inialize values for report */
	pgcnt = 0;
	LNSZ = 132;
	PGSIZE = 60;

	linecnt = PGSIZE;

	/* always print to  printer# 1 if printer */
	if( opn_prnt( "P", sr.termnm, 1, e_mesg, 1 )<0 ){
		printf(e_mesg);
		return(-1);
	} 

	sysdate = get_date();

	/* Get the date for 31 days old */
	expire_date = date_plus(sysdate, -31);

	man_chq.mc_emp_numb[0] = '\0';
	man_chq.mc_date = 0;
	man_chq.mc_chq_numb = 0;

	flg_reset(MAN_CHQ);

	for(;;){
		/* increment lowest part of key */
		man_chq.mc_chq_numb ++;

		retval = get_n_man_chq(&man_chq,UPDATE,0,FORWARD,e_mesg);
		if(retval == EFL) {
			return(NOERROR); 
		}
		if(retval < 0) {
			printf("\n\n%s", e_mesg);
			return(retval);
		}

		retval = CheckDates();
		if(retval<0) return(retval);

		strcpy(emp.em_numb,man_chq.mc_emp_numb);
		retval = get_employee(&emp,BROWSE,0,e_mesg);
		/* employee must exist for each advance. */
		if(retval < 0){
			printf("\n\ncould not match employee to advance %s",e_mesg);
			return(retval);
		}

	}

	close_rep();
	return(NOERROR);

}
CheckDates(expire_date)
	long expire_date;
{
	int retval ;

	/* If the two dates don't exceed the expire date then return */
	if(man_chq.mc_date > expire_date || man_chq.mc_chq_numb > expire_date)
		return(NOERROR);

	/* Print the entry to a report and Delete the advance. */

	retval = PrintAdv();
	if(retval < 0) return(retval);

	retval = DelAdv();
	if(retval < 0) return(retval);

}
/***
Print the header at the top of a page
***/
PrintHeader()
{
	char	txt_buff[132];
	short 	offset;


	linecnt = 0;

	mkln( 1, PROG_NAME, 10); 

	offset = ( LNSZ-strlen(param_rec.pa_co_name) )/2;
	mkln( offset, param_rec.pa_co_name, strlen(param_rec.pa_co_name) );

	mkln( 103, "DATE: ", 6 );
	tedit( (char *)&sysdt,"____/__/__",  line+cur_pos,R_LONG ); 
	cur_pos += 10;

	pgcnt++
	mkln(122, "Page:", 5);
	tedit( (char *)&pgcnt,"__0_",  txt_buff, R_SHORT ); 
	mkln(127,txt_buff, 10);

	if( prnt_line()<0 )	return(-1);

	mkln((LNSZ-25)/2,"PURGED PAY ADVANCE REPORT",25);
	if( prnt_line()<0 )	return(-1);
	if( prnt_line()<0 )	return(-1);

	mkln(1,"EMPLOYEE",8);
	mkln(20,"EMPLOYEE NAME",13);
	mkln(58,"ISSUE",5);
	mkln(70,"ADJUSTMENT",10);
	mkln(82,"ADVANCE",7);
	mkln(94,"DEDUCTION", 9);
	if( prnt_line()<0 )	return(-1);
	mkln(2,"NUMBER",6);
	mkln(58,"DATE",4);
	mkln(73,"DATE",4);
	mkln(82,"AMOUNT",6);
	mkln(96,"CODE",4);
	if( prnt_line()<0 )	return(-1);
	if( prnt_line()<0 )	return(-1);
	if( prnt_line()<0 )	return(-1);

	return(NOERROR);

}

/***
Procedure that prints the advance that is about to be deleted. This acts as
an audit report.
***/
PrintAdv()
{
	char	txt_buff[132];


	/* Check for page break */
	if(linecnt > PGSIZE)
		PrintHeader();

	/* Employee number */
	mkln(1,man_chq.mc_emp_numb,12);

	/* Employee name */
	sprintf(txt_buff,"%s, %s",emp_rec.em_last_name,
				emp_rec.em_first_name);
	mkln(14,txt_buff,strlen(txt_buff));

	/* Issue date */
	tedit( (char *)&man_chq.mc_date,"____/__/__",  txt_buff, R_LONG ); 
	mkln(58,txt_buff, 10);

	/* Adjustment date */
	tedit( (char *)&man_chq.mc_chq_numb,"____/__/__",  txt_buff, R_LONG ); 
	mkln(70,txt_buff,10);

	/* Amount */
	tedit((char *)&man_chq.mc_amount,"__,_0_.__-",txt_buff,R_DOUBLE);
	mkln(82,txt_buff,10);
	
	/* Deduction Code */
	mkln(94,man_chq.mc_ded_code,6);
	PrintLine();
	if( prnt_line()<0 )	return(-1);

	return(NOERROR);
}

/****
Procedure to delete advances from the man_chq file.
****/
DelAdv()
{
	int	retval;

	retval = put_man_chq(&man_chq,P_DEL,e_mesg);
	if(retval != NOERROR) {
		printf("\n\n%s", e_mesg);
		return(err);
	}

	retval = commit(e_mesg);
	if(retval != NOERROR) {
		printf("\n\n%s", e_mesg);
		return(err);
	}
	return(NOERROR);
}
