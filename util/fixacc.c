/*-----------------------------------------------------------------------
Source Name: fixatt.c 
System     : Budgetary Financial system.
Module     : Personnel Payroll
Created  On: 26 Aug 95 
Created  By: L. Robichaud

DESCRIPTION: This program is to be used to fix the vacation and sick bank
	for the month of December and January.  Currently thier are values
	in these fields and their should not be.  This program will zero them
	out.

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

static	Emp		emp_rec;
static	Att		att;
static	Pay_param	pay_param;

static	char c_mesg[50];

main(argc,argv)
int argc;
char *argv[];
{
	int err;
	
	strncpy(SYS_NAME,SYSTEM,50);	/* Sub system name */
	strncpy(CHNG_DATE,MOD_DATE,10);	/* Modification Date */
	proc_switch(argc, argv, MAINFL) ; 	/* Process Switches */

 	err = get_pay_param(&pay_param,BROWSE,1,c_mesg);
	if(err < 0) {
	  	printf("\nError reading pay param file.: %s\n",c_mesg);
		getchar();
	  	close_dbh();
	 	exit(0);
	}

	emp_rec.em_numb[0] = NULL;
	flg_reset(EMPLOYEE);

	for(;;){
		err = get_n_employee(&emp_rec, UPDATE, 0, FORWARD, c_mesg);
		if(err == EFL) break;
		if(err < 0) {
	  		printf("\nError reading employee file.: %s\n",c_mesg);
			getchar();
	  		close_dbh();
	  		exit(0);
		}

		emp_rec.em_vac_acc[5] = 0;
		emp_rec.em_vac_acc[6] = 0;
		emp_rec.em_sck_acc[5] = 0;
		emp_rec.em_sck_acc[6] = 0;

		err = put_employee(&emp_rec,UPDATE,c_mesg);
		if(err != NOERROR){
			printf(c_mesg);
			printf("ERROR in Saving employee Records\n"); 
			getchar();
			roll_back(c_mesg);
			exit(0);
		}
		if((err = commit(c_mesg))<0) {
			printf("\nError commiting records: %s",c_mesg);
			getchar();
			return(ERROR);
		}
		inc_str(emp_rec.em_numb, sizeof(emp_rec.em_numb)-1,FORWARD);
		flg_reset(EMPLOYEE);
	
	}
	seq_over(EMPLOYEE);
	close_dbh();			/* Close files */
	exit(0);
}

