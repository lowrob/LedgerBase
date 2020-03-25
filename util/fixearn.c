/*-----------------------------------------------------------------------
Source Name: fixearn.c 
System     : Budgetary Financial system.
Module     : Personnel Payroll
Created  On: 26 Aug 95 
Created  By: L. Robichaud

DESCRIPTION: This program is to be used to fix the earnings 
	for the month of Dec.  The pay run was made using pay period 50
	of the comming year. The problem is the pay periods were set up for
	the new year.

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
#define	MOD_DATE	"26-AUG-95"	/* Program Last Modified */

static	Emp_earn	emp_earn, tmpemp_earn;

static	char c_mesg[50];

main(argc,argv)
int argc;
char *argv[];
{
	int err;
	
	strncpy(SYS_NAME,SYSTEM,50);	/* Sub system name */
	strncpy(CHNG_DATE,MOD_DATE,10);	/* Modification Date */
	proc_switch(argc, argv, MAINFL) ; 	/* Process Switches */

	emp_earn.en_numb[0] = '\0';
	emp_earn.en_year = 0;
	emp_earn.en_pp = 0;
	emp_earn.en_week = 0;
	flg_reset(EMP_EARN);

	for(;;){
		err = get_n_emp_earn(&emp_earn,BROWSE,1,FORWARD,c_mesg) ;
		if(err < 0) {
			if(err == EFL) break;
	  		printf("\nError getting next earn rec .: %s\n",c_mesg);
			getchar();
			break;
		}

		if(emp_earn.en_year != 1996)
			continue;

		err = get_emp_earn(&emp_earn,UPDATE,1,c_mesg) ;
		if(err < 0) {
			if(err == EFL) break;
	  		printf("\nError getting next earn rec .: %s\n",c_mesg);
			getchar();
			break;
		}

	    	printf("\n\tFixing earnings for: %s",emp_earn.en_numb);

		scpy((char*)&tmpemp_earn,(char*)&emp_earn,sizeof(emp_earn));
	
		err = put_emp_earn(&emp_earn,P_DEL,c_mesg);
		if(err != NOERROR){
			printf(c_mesg);
			printf("ERROR in deleting old earn Records\n"); 
			getchar();
			roll_back(c_mesg);
			break;
		}

		scpy((char*)&emp_earn,(char*)&tmpemp_earn,sizeof(emp_earn));

	  	emp_earn.en_date = 19951212;
	    	emp_earn.en_year = 1995;

		err = put_emp_earn(&emp_earn,ADD,c_mesg);
		if(err != NOERROR){
			printf(c_mesg);
			printf("ERROR in Saving new earn Records\n"); 
			getchar();
			roll_back(c_mesg);
			break;
		}

		if((err = commit(c_mesg))<0) {
			printf("\nError commiting records: %s",c_mesg);
			getchar();
			roll_back(c_mesg);
			break;
		}

		emp_earn.en_week ++; /* Incrament key */

	}
	seq_over(EMP_EARN);
	close_dbh();	/* Close files */
	exit(0);
}
