/*-----------------------------------------------------------------------
Source Name: resetdate.c
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
Emp_earn	emp_earn;		/* Gl master file	*/
Emp_ins		emp_ins;		/* Gl master file	*/
Emp_dh		emp_ded_his;		/* Gl master file	*/
Emp_gr_his	emp_garn_his;		/* Gl master file	*/
Jrh_ent		jrh_ent;		/* Gl master file	*/

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
	int	retval, i ;

	retval = ProcessLoan();
	if(retval < 0)	return(retval);	
/*
	retval = ProcessEarn();
	if(retval < 0)	return(retval);	

	retval = ProcessIns();
	if(retval < 0)	return(retval);	

	retval = ProcessDed();
	if(retval < 0)	return(retval);	

	retval = ProcessGarn();
	if(retval < 0)	return(retval);	

	retval = ProcessJr();
	if(retval < 0)	return(retval);	*/

	return(NOERROR) ;
}	/* CloseProcess() */
/*-----------------------------------------------------------------------*/
/* Take the Keys for de-selection from the user. When he confirms it
   do the de-selection */
ProcessLoan()
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
/*-----------------------------------------------------------------------*/
/* Take the Keys for de-selection from the user. When he confirms it
   do the de-selection */
ProcessEarn()
{
	int	err, i ;

	emp_earn.en_numb[0] = '\0';
	emp_earn.en_date = 0;
	emp_earn.en_pp = 0;
	emp_earn.en_week = 0;
	flg_reset(EMP_LOAN_HIS);

	for( ; ; ){
		err = get_n_emp_earn(&emp_earn,UPDATE,0,FORWARD,e_mesg) ;
		if(err < 0) {
			if(err == EFL) break;
			roll_back(e_mesg);
			return(err);
		}
		if(emp_earn.en_date != 19930408){
			roll_back(e_mesg);
			continue;
		}

		printf("Employee = %s\r",emp_earn.en_numb);
		err = put_emp_earn(&emp_earn,P_DEL,e_mesg) ;
		if(err < 0) {
			roll_back(e_mesg);
			return(err);
		}
		err = commit(e_mesg) ;
		if(err < 0) {
			roll_back(e_mesg);
			return(err);
		}
		emp_earn.en_date = 19930413;
		err = put_emp_earn(&emp_earn,ADD,e_mesg) ;
		if(err < 0) {
			roll_back(e_mesg);
			return(err);
		}
		err = commit(e_mesg) ;
		if(err < 0) {
			roll_back(e_mesg);
			return(err);
		}
		emp_earn.en_week++;
		flg_reset(EMP_EARN);
	}

	return(NOERROR);
}
/*-----------------------------------------------------------------------*/
/* Take the Keys for de-selection from the user. When he confirms it
   do the de-selection */
ProcessIns()
{
	int	err, i ;

	emp_ins.in_numb[0] = '\0';
	emp_ins.in_pp = 0;
	emp_ins.in_date = 0;
	flg_reset(EMP_LOAN_HIS);

	for( ; ; ){
		err = get_n_emp_ins(&emp_ins,UPDATE,0,FORWARD,e_mesg) ;
		if(err < 0) {
			if(err == EFL) break;
			roll_back(e_mesg);
			return(err);
		}
		if(emp_ins.in_date != 19930408){
			roll_back(e_mesg);
			continue;
		}

		printf("Employee = %s\r",emp_ins.in_numb);
		err = put_emp_ins(&emp_ins,P_DEL,e_mesg) ;
		if(err < 0) {
			roll_back(e_mesg);
			return(err);
		}
		err = commit(e_mesg) ;
		if(err < 0) {
			roll_back(e_mesg);
			return(err);
		}
		emp_ins.in_date = 19930413;
		err = put_emp_ins(&emp_ins,ADD,e_mesg) ;
		if(err < 0) {
			roll_back(e_mesg);
			return(err);
		}
		err = commit(e_mesg) ;
		if(err < 0) {
			roll_back(e_mesg);
			return(err);
		}
		emp_ins.in_date++;
		flg_reset(EMP_INS);
	}

	return(NOERROR);
}
/*-----------------------------------------------------------------------*/
/* Take the Keys for de-selection from the user. When he confirms it
   do the de-selection */
ProcessDed()
{
	int	err, i ;

	emp_ded_his.edh_numb[0] = '\0';
	emp_ded_his.edh_pp = 0;
	emp_ded_his.edh_date = 0;
	flg_reset(EMP_DED_HIS);

	for( ; ; ){
		err = get_n_emp_dhis(&emp_ded_his,UPDATE,0,FORWARD,e_mesg) ;
		if(err < 0) {
			if(err == EFL) break;
			roll_back(e_mesg);
			return(err);
		}
		if(emp_ded_his.edh_date != 19930408){
			roll_back(e_mesg);
			continue;
		}

		printf("Employee = %s\r",emp_ded_his.edh_numb);
		err = put_emp_dhis(&emp_ded_his,P_DEL,e_mesg) ;
		if(err < 0) {
			roll_back(e_mesg);
			return(err);
		}
		err = commit(e_mesg) ;
		if(err < 0) {
			roll_back(e_mesg);
			return(err);
		}
		emp_ded_his.edh_date = 19930413;
		err = put_emp_dhis(&emp_ded_his,ADD,e_mesg) ;
		if(err < 0) {
			roll_back(e_mesg);
			return(err);
		}
		err = commit(e_mesg) ;
		if(err < 0) {
			roll_back(e_mesg);
			return(err);
		}
		emp_ded_his.edh_date++;
		flg_reset(EMP_DED_HIS);
	}

	return(NOERROR);
}
/*-----------------------------------------------------------------------*/
/* Take the Keys for de-selection from the user. When he confirms it
   do the de-selection */
ProcessGarn()
{
	int	err, i ;

	emp_garn_his.egh_numb[0] = '\0';
	emp_garn_his.egh_pp = 0;
	emp_garn_his.egh_date = 0;
	emp_garn_his.egh_pr_cd = 0;
	emp_garn_his.egh_seq = 0;
	flg_reset(EMP_GARN_HIS);

	for( ; ; ){
		err = get_n_emp_ghis(&emp_garn_his,UPDATE,0,FORWARD,e_mesg) ;
		if(err < 0) {
			if(err == EFL) break;
			roll_back(e_mesg);
			return(err);
		}
		if(emp_garn_his.egh_date != 19930408){
			roll_back(e_mesg);
			continue;
		}

		printf("Employee = %s\r",emp_garn_his.egh_numb);
		err = put_emp_ghis(&emp_garn_his,P_DEL,e_mesg) ;
		if(err < 0) {
			roll_back(e_mesg);
			return(err);
		}
		err = commit(e_mesg) ;
		if(err < 0) {
			roll_back(e_mesg);
			return(err);
		}
		emp_garn_his.egh_date = 19930413;
		err = put_emp_ghis(&emp_garn_his,ADD,e_mesg) ;
		if(err < 0) {
			roll_back(e_mesg);
			return(err);
		}
		err = commit(e_mesg) ;
		if(err < 0) {
			roll_back(e_mesg);
			return(err);
		}
		emp_garn_his.egh_seq++;
		flg_reset(EMP_GARN_HIS);
	}

	return(NOERROR);
}
/*-----------------------------------------------------------------------*/
/* Take the Keys for de-selection from the user. When he confirms it
   do the de-selection */
ProcessJr()
{
	int	err, i ;

	jrh_ent.jrh_fund = 0;
	jrh_ent.jrh_no = 0;
	jrh_ent.jrh_cheque = 0;
	flg_reset(JRH_ENT);

	for( ; ; ){
		err = get_n_jrh_ent(&jrh_ent,UPDATE,0,FORWARD,e_mesg) ;
		if(err < 0) {
			if(err == EFL) break;
			roll_back(e_mesg);
			return(err);
		}
		if(jrh_ent.jrh_date != 19930408){
			roll_back(e_mesg);
			continue;
		}

		printf("Employee = %s\r",jrh_ent.jrh_emp_numb);

		jrh_ent.jrh_date = 19930413;

		err = put_jrh_ent(&jrh_ent,UPDATE,e_mesg) ;
		if(err < 0) {
			roll_back(e_mesg);
			return(err);
		}
		err = commit(e_mesg) ;
		if(err < 0) {
			roll_back(e_mesg);
			return(err);
		}
		jrh_ent.jrh_cheque++;
		flg_reset(JRH_ENT);
	}

	return(NOERROR);
}
/*-------------------- E n d   O f   P r o g r a m ---------------------*/
