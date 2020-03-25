/*-----------------------------------------------------------------------
Source Name: updtatt.c 
System     : Budgetary Financial system.
Module     : Personnel Payroll
Created  On: 26 May 95 
Created  By: L. Robichaud

DESCRIPTION: This program is to be used to update the employee's overtime
	bank at any time. It will process all the employees, and read through
	the attendance file sequentialy and check to see all the attendances
	that affect the overtime bank have been calculated against the employee
	overtime bank.

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
------------------------------------------------------------------------*/
#define  MAIN
#define  MAINFL		EMP_ATT   	/* main file used */

#include <stdio.h>
#include <cfomstrc.h>
#include <repdef.h>
#include <isnames.h>
#include <bfs_defs.h>
#include <bfs_com.h>
#include <bfs_pp.h>

#define	SYSTEM		"PAYROLE"	/*Sub System Name */
#define	MOD_DATE	"26-MAY-95"		/* Program Last Modified */

static  Emp_at_his	att_his;   
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
		err = get_n_employee(&emp_rec, BROWSE, 0, FORWARD, c_mesg);
		if(err == EFL) break;
		if(err < 0) {
	  		printf("\nError reading employee file.: %s\n",c_mesg);
			getchar();
	  		close_dbh();
	  		exit(0);
		}
		err = CheckAtt();
		if(err < 0) {
	  		close_dbh();
	  		exit(0);
		}
	}
	seq_over(EMPLOYEE);
	close_dbh();			/* Close files */
	exit(0);
}

static
CheckAtt()
{

	int	err, retval;

	
	strcpy(att_his.eah_numb, emp_rec.em_numb);
	if(pay_param.pr_st_mth == 1){
		att_his.eah_date = pay_param.pr_cal_st_dt;  
	}
	if(pay_param.pr_st_mth == 2){
		att_his.eah_date = pay_param.pr_fisc_st_dt;  
	}
	if(pay_param.pr_st_mth == 3){
		att_his.eah_date = pay_param.pr_schl_st_dt;  
	}

	flg_reset(EMP_ATT);

	for(;;){
		retval = get_n_emp_at(&att_his, BROWSE, 0, FORWARD, c_mesg);

		if(retval == EFL ||		
		   (strcmp(att_his.eah_numb, emp_rec.em_numb)) != 0){
			break;
		}
		if(retval < 0) {
	  		printf("\nError reading emp att. file.: %s\n",c_mesg);
			getchar();
			return(-1);
		}

		if(strcmp(att_his.eah_vacproc, "Y") == 0)
			continue;

		strcpy(att.at_code, att_his.eah_code);

		retval = get_att(&att,BROWSE,1,c_mesg);
		if(retval < 0)  {
	  		printf("\nError reading att. file.: %s\n",c_mesg);
			getchar();
			return(retval);
		}

		/* If the attendace entry does not affect the vac bank, cont*/
		if(strcmp(att.at_vacbank,"Y")!=0)
			continue;

		retval = get_employee(&emp_rec, UPDATE, 0, c_mesg);
		if(retval < 0)  {
	  		printf("\nError reading Employee. file.: %s\n",c_mesg);
			getchar();
			return(retval);
		}

		emp_rec.em_vac_bk -= att_his.eah_hours / 
			att_his.eah_sen_hours;
		err = put_employee(&emp_rec,UPDATE,c_mesg);
		if(err != NOERROR){
			printf(c_mesg);
			printf("ERROR in Saving employee Records\n"); 
			getchar();
			roll_back(c_mesg);
			exit(0);
		}
	

		retval = get_emp_at(&att_his, UPDATE, 0, c_mesg);
		if(retval < 0){
	  		printf("\nError reading Emp Att. file.: %s\n",c_mesg);
			getchar();
			roll_back(c_mesg);
			return(retval);
		}

		att_his.eah_vacproc[0] = 'Y';
		err = put_emp_at(&att_his,UPDATE,c_mesg);
		if(err != NOERROR){
			printf(c_mesg);
			printf("ERROR in Saving att_his  Records\n"); 
			getchar();
			roll_back(c_mesg);
			exit(0);
		}
		if((err = commit(c_mesg))<0) {
			printf("\nError commiting records: %s",c_mesg);
			getchar();
			return(ERROR);
		}

	}
	seq_over(EMP_ATT);
	return(NOERROR);
}

