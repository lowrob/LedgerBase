/*-----------------------------------------------------------------------
Source Name: reset_loan.c
System     : Cheque Pre-processing
Created  On: July 27, 1993.
Created  By: Andre Cormier.

DESCRIPTION:
	Program to change the date to a pay period date.

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
------------------------------------------------------------------------*/

#define	MAIN

#include <stdio.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_pp.h>
#include <bfs_com.h>

#define	SYSTEM		"CHEQUE PRE-PROCESSING"	/* Sub System Name */
#define	MOD_DATE	"27-JUL-93"		/* Program Last Modified */

Emp_ln_his	emp_ln_his;		/* Gl master file	*/

char	e_mesg[100];
/*------------------------------------------------------------------------*/

main(argc,argv)
int	argc;
char	*argv[];
{
	int	err;

	if(argc < 2){
#ifdef  DEVELOP
		printf("MAIN ARGUMENTS ARE NOT PROPER\n");
		printf("Usage: %s {-tTerminal Name} {-dDist#} [{-sSwitches}]\n",
			argv[0]);
#endif
		exit(1);
	}

	err = Initialize(argc,argv) ;	/* Initialize Variables , DBH
						Environment and PROFOM */
	if(err == NOERROR) err = Process();	/* Initiate Process */

	CloseProcess() ;

	if(err != NOERROR)exit(1);
	exit(0);
}	/* main() */
/*-------------------------------------------------------------------*/
/* Initialize Variables, PROFOM, DBH etc. */

Initialize(argc, argv)
int	argc ;
char	*argv[] ;
{
	int	i ;

	/*
	*	Initialize DBH Environment
	*/
	strncpy(SYS_NAME,SYSTEM,50);	/* Sub system name */
	strncpy(CHNG_DATE,MOD_DATE,10);	/* Modification Date */

	proc_switch(argc, argv, GLMAST) ;	/* Process Switches */

	return(NOERROR) ;
}	/* Initialize() */
/* Close necessary files and environment before exiting program */

CloseProcess()
{
	/* Set terminal back to normal mode from PROFOM */
	fomcs();
	fomrt();

	close_dbh();	/* Close DBH(files) */

	return(NOERROR) ;
}	/* CloseProcess() */
/*-----------------------------------------------------------------------*/
/* Take the Keys for de-selection from the user. When he confirms it
   do the de-selection */

Process()
{
	int	err, i ;

	emp_ln_his.elh_numb[0] = '\0';
	emp_ln_his.elh_pp = 0;
	emp_ln_his.elh_date = 0;
	emp_ln_his.elh_code[0] = '\0';
	emp_ln_his.elh_seq = 0;
	flg_reset(EMP_LOAN_HIS);

	for( ; ; ){
		err = get_n_emp_lhis(&emp_ln_his,UPDATE,0,FORWARD,e_mesg) ;
		if(err < 0) {
			if(err == EFL) break;
			roll_back(e_mesg);
			return(err);
		}
		if(emp_ln_his.elh_date != 19930408){
			roll_back(e_mesg);
			continue;
		}

		printf("Employee = %s\r",emp_ln_his.elh_numb);
		err = put_emp_lhis(&emp_ln_his,P_DEL,e_mesg) ;
		if(err < 0) {
			roll_back(e_mesg);
			return(err);
		}
		err = commit(e_mesg) ;
		if(err < 0) {
			roll_back(e_mesg);
			return(err);
		}
		emp_ln_his.elh_date = 19930413;
		err = put_emp_lhis(&emp_ln_his,ADD,e_mesg) ;
		if(err < 0) {
			roll_back(e_mesg);
			return(err);
		}
		err = commit(e_mesg) ;
		if(err < 0) {
			roll_back(e_mesg);
			return(err);
		}
		emp_ln_his.elh_seq++;
		flg_reset(EMP_LOAN_HIS);
	}

	return(NOERROR);
}
/*-------------------- E n d   O f   P r o g r a m ---------------------*/
