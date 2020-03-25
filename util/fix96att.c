/*-----------------------------------------------------------------------
Source Name: fixloanh.c 
System     : Budgetary Financial system.
Module     : Personnel Payroll
Created  On: 05 Jan 96
Created  By: L. Robichaud

DESCRIPTION: This program is to be used to fix the attendance history dates
	for the month of November and December of 96.
	It seems that these months have values that should have been for
	the year 1995.  This is because of runnung the pay run with the new
	pay periods set up.


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

static  Emp_at_his      att_his, tmpatt_his;

static	char c_mesg[50];

main(argc,argv)
int argc;
char *argv[];
{
	int err;
	
	strncpy(SYS_NAME,SYSTEM,50);	/* Sub system name */
	strncpy(CHNG_DATE,MOD_DATE,10);	/* Modification Date */
	proc_switch(argc, argv, MAINFL) ; 	/* Process Switches */


	att_his.eah_numb[0] = NULL;
	att_his.eah_date = 0;

	flg_reset(EMP_ATT);

	for(;;){
		err = get_n_emp_at(&att_his, BROWSE, 0, FORWARD, c_mesg);

		if(err == EFL)
			break;

		if(err < 0) {
	  		printf("\nError reading emp att. file.: %s\n",c_mesg);
			getchar();
			break;
		}

		/* Check here if this is a record that has to be changed*/
		if(att_his.eah_date > 19961100)
			if(FixRoutine()) break;
	}
	seq_over(EMP_ATT);
	close_dbh();	/* Close files */
	exit(0);
}
FixRoutine()
{

	int	err;
	long	tmp_date;

	err = get_emp_at(&att_his, UPDATE, 0, c_mesg);
	if(err < 0) {
	  	printf("\nError getting att his rec .: %s\n",c_mesg);
		printf(c_mesg);
		getchar();
		return(ERROR);
	}

	printf("\nFixing att history date %ld for: %s",
		att_his.eah_date,att_his.eah_numb);

	scpy((char*)&tmpatt_his,(char*)&att_his,sizeof(att_his));
	
	err = put_emp_at(&att_his,P_DEL,c_mesg);
	if(err != NOERROR){
		printf(c_mesg);
		printf("\nERROR in deleting old att his Records"); 
		getchar();
		roll_back(c_mesg);
		return(ERROR);
	}

	scpy((char*)&att_his,(char*)&tmpatt_his,sizeof(att_his));

	/* Save the date so the read can pick up where it left off */
	tmp_date = att_his.eah_date;

	/* This will reduce the date from 1996???? to 1995???? */
	att_his.eah_date -= 10000;

	err = put_emp_at(&att_his,ADD,c_mesg);
	if(err != NOERROR){
		printf(c_mesg);
		printf("\nERROR in Saving new att_his history Records"); 
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
	att_his.eah_date = tmp_date;

	/* Incrament key */
	att_his.eah_date ++;

	return(NOERROR);
}
