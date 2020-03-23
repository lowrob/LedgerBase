/*-----------------------------------------------------------------------
Source Name: testac.c
System     : 
Created  On: June 3, 1992.
Created  By: Andre Cormier.

DESCRIPTION:
	Program to reset the outstanding amount in emp_loan file.

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
------------------------------------------------------------------------*/

#define	MAIN

#include <stdio.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_pp.h>

#define	SYSTEM		"CHEQUE PRE-PROCESSING"	/* Sub System Name */
#define	MOD_DATE	"27-MAY-92"		/* Program Last Modified */

Emp_loan	emp_loan;	/* Employee Loan file	*/


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

	proc_switch(argc, argv, TIME) ;	/* Process Switches */

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
	int	err ;

	emp_loan.el_numb[0] = '\0';
	emp_loan.el_code[0] = '\0';
	emp_loan.el_seq = 0;

	flg_reset(EMP_LOAN);

	for( ; ; ){

		err = get_n_emp_loan(&emp_loan,UPDATE, 0, FORWARD, e_mesg) ;
		if(err < 0) {
			if(err == EFL) break;
			return(ERROR) ;
		}

		emp_loan.el_amnt_out = emp_loan.el_amount;

		err = put_emp_loan(&emp_loan,UPDATE,e_mesg);	
		if(err < 0) {
			return(ERROR);
		}
		err = commit(e_mesg) ;
			if(err < 0) {
			roll_back(e_mesg);
			return(err);
		}
		
		emp_loan.el_seq++;
		flg_reset(EMP_LOAN);
	}

	return(NOERROR);
}
/*-------------------- E n d   O f   P r o g r a m ---------------------*/
