/*-----------------------------------------------------------------------
Source Name: fixdedh.c 
System     : Budgetary Financial system.
Module     : Personnel Payroll
Created  On: 04 Jan 96
Created  By: L. Robichaud

DESCRIPTION: This program is to be used to fix the deduction history dates
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

static	Emp_dh	emp_dh, tmpemp_dh;

static	char c_mesg[50];

main(argc,argv)
int argc;
char *argv[];
{
	int err;
	
	strncpy(SYS_NAME,SYSTEM,50);	/* Sub system name */
	strncpy(CHNG_DATE,MOD_DATE,10);	/* Modification Date */
	proc_switch(argc, argv, MAINFL) ; 	/* Process Switches */


	emp_dh.edh_numb[0] = NULL;
	emp_dh.edh_pp = 0;
	emp_dh.edh_date = 0;
	emp_dh.edh_code[0] = NULL;
	emp_dh.edh_group[0] = NULL;

	flg_reset(EMP_DED_HIS);

	for(;;){
		err = get_n_emp_dhis(&emp_dh,BROWSE,0,FORWARD,c_mesg) ;
		if(err < 0) {
			if(err == EFL) break;
	  		printf("\nError getting next ded his rec .: %s\n",c_mesg);
			getchar();
			break;
		}

		/* Check here if this is a record that has to be changed*/
		if(emp_dh.edh_date == 19951218)
			if(FixRoutine()) break;
		if(emp_dh.edh_date == 19951220)
			if(FixRoutine()) break;
	}
	seq_over(EMP_DED_HIS);
	close_dbh();	/* Close files */
	exit(0);
}
FixRoutine()
{

	int	err;
	long	tmp_date;

	err = get_emp_dhis(&emp_dh,UPDATE,0,c_mesg) ;
	if(err < 0) {
	  	printf("\nError getting next earn rec .: %s\n",c_mesg);
		getchar();
		return(ERROR);
	}

	printf("\n\tFixing deduction history for: %s",emp_dh.edh_numb);

	scpy((char*)&tmpemp_dh,(char*)&emp_dh,sizeof(emp_dh));
	
	err = put_emp_dhis(&emp_dh,P_DEL,c_mesg);
	if(err != NOERROR){
		printf(c_mesg);
		printf("\nERROR in deleting old ded his Records"); 
		getchar();
		roll_back(c_mesg);
		return(ERROR);
	}

	scpy((char*)&emp_dh,(char*)&tmpemp_dh,sizeof(emp_dh));

	/* Save the date so the read can pick up where it left off */
	tmp_date = emp_dh.edh_date;

	if(emp_dh.edh_date == 19951218)
		emp_dh.edh_date = 19951219;
	else if(emp_dh.edh_date == 19951220)
		emp_dh.edh_date = 19951226;

	err = put_emp_dhis(&emp_dh,ADD,c_mesg);
	if(err != NOERROR){
		printf(c_mesg);
		printf("\nERROR in Saving new deduction history Records"); 
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
	emp_dh.edh_date = tmp_date;

	/* Incrament key */
	inc_str(emp_dh.edh_group,sizeof(emp_dh.edh_group)-1,FORWARD);

	return(NOERROR);
}
