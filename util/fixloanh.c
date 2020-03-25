/*-----------------------------------------------------------------------
Source Name: fixloanh.c 
System     : Budgetary Financial system.
Module     : Personnel Payroll
Created  On: 05 Jan 96
Created  By: L. Robichaud

DESCRIPTION: This program is to be used to fix the loan history dates
	for pay periods 51 and 52 in the month of Dec. of 1995.


MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
------------------------------------------------------------------------*/
#define  MAIN
#define  MAINFL		EMPLOYEE   	/* main file used */

#include <stdio.h>
#include <cfomstrc.h>
#include <repdef.h>
#include <isnames.h>
#include <bfs_defs.h>
#include <bfs_com.h>
#include <bfs_pp.h>

#define	SYSTEM		"PAYROLE"	/*Sub System Name */
#define	MOD_DATE	"04-JAN-96"	/* Program Last Modified */

static	Emp_ln_his	emp_ln_his, tmpemp_ln_his;

static	char c_mesg[50];

main(argc,argv)
int argc;
char *argv[];
{
	int err;
	
	strncpy(SYS_NAME,SYSTEM,50);	/* Sub system name */
	strncpy(CHNG_DATE,MOD_DATE,10);	/* Modification Date */
	proc_switch(argc, argv, MAINFL) ; 	/* Process Switches */


	emp_ln_his.elh_numb[0] = NULL;
	emp_ln_his.elh_pp = 0;
	emp_ln_his.elh_date = 0;
	emp_ln_his.elh_code[0] = NULL;
	emp_ln_his.elh_seq = 0;

	flg_reset(EMP_LOAN_HIS);

	for(;;){
		err = get_n_emp_lhis(&emp_ln_his,BROWSE,0,FORWARD,c_mesg) ;
		if(err < 0) {
			if(err == EFL) break;
	  		printf("\nError getting next loan his rec .: %s\n",c_mesg);
			getchar();
			break;
		}

		/* Check here if this is a record that has to be changed*/
		if(emp_ln_his.elh_date == 19951218)
			if(FixRoutine()) break;
		if(emp_ln_his.elh_date == 19951220)
			if(FixRoutine()) break;
	}
	seq_over(EMP_LOAN_HIS);
	close_dbh();	/* Close files */
	exit(0);
}
FixRoutine()
{

	int	err;
	long	tmp_date;

	err = get_emp_lhis(&emp_ln_his,UPDATE,0,c_mesg) ;
	if(err < 0) {
	  	printf("\nError getting loan his rec .: %s\n",c_mesg);
		printf(c_mesg);
		getchar();
		return(ERROR);
	}

	printf("\n\tFixing loan history for: %s",emp_ln_his.elh_numb);

	scpy((char*)&tmpemp_ln_his,(char*)&emp_ln_his,sizeof(emp_ln_his));
	
	err = put_emp_lhis(&emp_ln_his,P_DEL,c_mesg);
	if(err != NOERROR){
		printf(c_mesg);
		printf("\nERROR in deleting old loan his Records"); 
		getchar();
		roll_back(c_mesg);
		return(ERROR);
	}

	scpy((char*)&emp_ln_his,(char*)&tmpemp_ln_his,sizeof(emp_ln_his));

	/* Save the date so the read can pick up where it left off */
	tmp_date = emp_ln_his.elh_date;

	if(emp_ln_his.elh_date == 19951218)
		emp_ln_his.elh_date = 19951219;
	else if(emp_ln_his.elh_date == 19951220)
		emp_ln_his.elh_date = 19951226;

	err = put_emp_lhis(&emp_ln_his,ADD,c_mesg);
	if(err != NOERROR){
		printf(c_mesg);
		printf("\nERROR in Saving new loan history Records"); 
		getchar();
		roll_back(c_mesg);
		return(ERROR);
	}

	if((err = commit(c_mesg))<0) {
		printf("\nError commiting records: %s",c_mesg);
		getchar();
		roll_back(c_mesg);
		return(ERROR);
	}

	/* Replace the date to get the next record */
	emp_ln_his.elh_date = tmp_date;

	/* Incrament key */
	emp_ln_his.elh_seq ++;

	return(NOERROR);
}
