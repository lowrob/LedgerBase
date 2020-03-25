/*-----------------------------------------------------------------------
Source Name: fixsen.c 
System     : Budgetary Financial system.
Module     : Personnel Payroll
Created  On: 26 Aug 95 
Created  By: L. Robichaud

DESCRIPTION: This program is to be used to fix the senority 
	for the month of January.  Currently thier are values
	in these fields and their should not be.  This program will zero them
	out. As well as attempt to fix the month of Dec.

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

static	Emp_sen		emp_sen;

static	char c_mesg[50];

main(argc,argv)
int argc;
char *argv[];
{
	int err;
	
	strncpy(SYS_NAME,SYSTEM,50);	/* Sub system name */
	strncpy(CHNG_DATE,MOD_DATE,10);	/* Modification Date */
	proc_switch(argc, argv, MAINFL) ; 	/* Process Switches */


	emp_sen.esn_numb[0] = '\0';
	emp_sen.esn_month = 0;
	emp_sen.esn_pos[0] = '\0';
	emp_sen.esn_class[0] = '\0';
	flg_reset(EMP_SEN) ;

	for( ; ; ) {
		err = get_n_emp_sen(&emp_sen,BROWSE,0,FORWARD,c_mesg);
		if(err < 0) {
			if(err == EFL) break;
	  		printf("\nError getting next sen rec .: %s\n",c_mesg);
			getchar();
			break;
		}

		if(emp_sen.esn_month == 1){
			err = RemoveValue();
			if(err < 0) break;
			continue;
		}
		if(emp_sen.esn_perm_days == 24 && emp_sen.esn_month == 11 ||
		   emp_sen.esn_perm_days == 27 && emp_sen.esn_month == 12 ||
		   emp_sen.esn_perm_days == 14 && emp_sen.esn_month == 7){
			err = FixPermDays();
			if(err < 0) break;
			continue;
		}
	}
	seq_over(EMP_SEN);
	close_dbh();	/* Close files */
	exit(0);
}

RemoveValue()
{

	int	err;

	err = get_emp_sen(&emp_sen,UPDATE,0,c_mesg);
	if(err < 0) {
		printf(c_mesg);
		printf("ERROR in getting in update prior to delete \n"); 
		getchar();
		return(err);
	}
	err = put_emp_sen(&emp_sen,P_DEL,c_mesg);
	if(err != NOERROR){
		printf(c_mesg);
		printf("ERROR in Deleting senority Records\n"); 
		getchar();
		roll_back(c_mesg);
		exit(0);
	}
	if((err = commit(c_mesg))<0) {
		printf("\nError commiting records: %s",c_mesg);
		getchar();
		return(ERROR);
	}

	inc_str(emp_sen.esn_class, sizeof(emp_sen.esn_class)-1,FORWARD);
	flg_reset(EMP_SEN);

	return(NOERROR);
	
}
/***********************************************************
This routine should only be reached if the senority has to be fixed
for Nov. or Dec.
*************************************************************/
FixPermDays()

{

	int	err;

	err = get_emp_sen(&emp_sen,UPDATE,0,c_mesg);
	if(err < 0) {
		printf(c_mesg);
		printf("ERROR in getting in update prior to fixing sen \n"); 
		getchar();
		return(err);
	}

	/* Correct the perm days */
	if(emp_sen.esn_month == 12) /* For Dec, correct to 8 */
		emp_sen.esn_perm_days = 8;
	else if(emp_sen.esn_month == 11)	/* For Nov, correct to 22 */
		emp_sen.esn_perm_days = 22;
	else if(emp_sen.esn_month == 7)	/* For July Correct to 21 */
		emp_sen.esn_perm_days = 21;

	err = put_emp_sen(&emp_sen,UPDATE,c_mesg);
	if(err != NOERROR){
		printf(c_mesg);
		printf("ERROR in Deleting senority Records\n"); 
		getchar();
		roll_back(c_mesg);
		exit(0);
	}
	if((err = commit(c_mesg))<0) {
		printf("\nError commiting records: %s",c_mesg);
		getchar();
		return(ERROR);
	}

	printf("\n\tFixed employee %s", emp_sen.esn_numb);

	inc_str(emp_sen.esn_class, sizeof(emp_sen.esn_class)-1,FORWARD);
	flg_reset(EMP_SEN);

	return(NOERROR);
}
