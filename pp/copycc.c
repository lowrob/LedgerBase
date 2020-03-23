/*-----------------------------------------------------------------------
Source Name: transfer.c
System     : Personnel/Payroll System.
Created  By: Eugene Roy.

DESCRIPTION:

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
------------------------------------------------------------------------*/
#define	MAIN		/* Main program. This is to declare Switches */
#define MAINFL		EMPLOYEE			/* main file used */

#define	SYSTEM		"SETUP"			/* Sub System Name */
#define	MOD_DATE	"03-OCT-91"		/* Progran Last Modified */

#include <stdio.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_com.h>
#include <bfs_pp.h>

static	char 	e_mesg[100];  		/* dbh will return err msg in this */

int	Argc;
char	**Argv;

static	Emp	emp_rec, pre_emp;			/* Employee record */
static	Emp_sched1	emp_sched1;

void	free() ;
char	*malloc() ;
/*------------------------------------------------------------------------*/
main(argc,argv)
int argc;
char *argv[];
{
	int 	retval;

	/* These two are passed to execute() when it is called */
	Argc = argc;
	Argv = argv;

	retval = Initialize(argc,argv);	/* Initialization routine */

	if (retval == NOERROR) retval = Process();

	CloseRtn();			/* return to menu */
	if (retval < NOERROR) exit(-1);
	exit(0);
}

/*-------------------------------------------------------------------*/
/* Initialize PROFOM */

Initialize(argc, argv)
int	argc ;
char	*argv[] ;
{
	int	err ;

	/*
	*	Initialize DBH Environment
	*/
	proc_switch(argc, argv, MAINFL) ; 	/* Process Switches */

	/*
	*	Initialize PROFOM & Screen
	*/

	return(NOERROR) ;
}	/* Initialize() */

/*--------------------------------------------------------------------------*/
/* Close nessary files and environment before exiting program               */

CloseRtn() 
{
	/* Set terminal back to normal mode from PROFOM */
	fomcs();
	fomrt();

	close_dbh();	/* Close files */

	return(NOERROR);
}	/* CloseRtn() */
/*-------------------------------------------------------------------*/
/* Get Option from user and call corresponding function */

Process()
{
	int err;
	int	i,retval;

	emp_rec.em_numb[0] = LV_CHAR;
	flg_reset(EMPLOYEE);

	for( ; ; ){
		err = get_n_employee(&emp_rec,UPDATE,0,FORWARD,e_mesg);
		if(err == EFL) break;
		if(err < 0) 
			return(err);

		printf("%s", emp_rec.em_numb);
		fflush(stdin);

		strcpy(emp_sched1.es_numb,emp_rec.em_numb);
		emp_sched1.es_week = 0;
		emp_sched1.es_fund = 0;
		emp_sched1.es_class[0] = LV_CHAR;
		emp_sched1.es_cost = 0;
		emp_sched1.es_dept[0] = LV_CHAR;
		emp_sched1.es_area[0] = LV_CHAR;
		flg_reset(EMP_SCHED1);

		retval = get_n_emp_sched1(&emp_sched1,BROWSE,0,
					   FORWARD,e_mesg);
		if(retval == UNDEF) continue;
		if(retval < 0) continue;
		if(strcmp(emp_sched1.es_numb,emp_rec.em_numb) !=0)
			continue;

		emp_rec.em_cc = emp_sched1.es_cost;

		retval = put_employee(&emp_rec,UPDATE,e_mesg);
		if(retval < 0) {
			roll_back(e_mesg);
			return(retval);
		}
	
		retval = rite_audit((char*)&s_sth,EMPLOYEE,UPDATE,(char*)&emp_rec,
			(char*)&pre_emp,e_mesg);
		if(retval==LOCKED) {
			DispError((char *)&emp_rec,e_mesg);
			roll_back(e_mesg) ;
			return(LOCKED) ;
		}
		if(retval < 0 ){
		roll_back(e_mesg);
		return(retval);
		}

		retval = commit(e_mesg) ;
		if(retval < 0) {
		roll_back(e_mesg);
		return(retval);

		}
		inc_str(emp_rec.em_numb, sizeof(emp_rec.em_numb)-1, 
				FORWARD);
		flg_reset(EMPLOYEE);

	}
	return(NOERROR);
	
}	/* Process() */
/*-------------------- E n d   O f   P r o g r a m ---------------------*/
