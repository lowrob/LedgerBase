/*-----------------------------------------------------------------------
Source Name: empldrvr.c 
System     : LedgerBase - Personnel/Payroll Module. 
Module     : Employee Maintenance
Created  On: 1991/10/18  
Created  By: Cathy Burns.

DESCRIPTION:
	empldrvr.c : Program to Drive the Employee Maintenance Screens.

NOTE:
	All these programs maintain only one employee record, which is passed
	as a command line argument and which is available in global variable.
	

Usage of SWITCHES when they are ON :
	SW1 - (DEMOGRAPHIC DATA)		
	SW2 - (EMPLOYMENT  DATA)
	SW3 - (RESPONSIBILITY  DATA)	
	SW4 - (EARNINGS  DATA)	
	SW5 - (MISCELLANEOUS DATA)
	SW6 - (BENEFIT DATA)
	SW7 - (DEDUCTION DATA)
	SW8 - (CSB/LOAN DATA)
	SW9 - (GARNISHMENT DATA)
	SW10 - (CHEQUE LOCATION DATA)
	SW11 - (ATTENDANCE DATA)
	SW12 - (SENIORITY DATA)
	SW13 - (TEACHER QUALIFICATIONS)
	SW14 - (TEACHER ASSIGNMENTS)
	SW15 - (EMPLOYEE COMPETITION)
	SW16 - (STATUS INQUIRY)

	At any time only one switch should be ON.

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
------------------------------------------------------------------------*/

#define	MAIN	/* Main program. This is to declare Switches */

#define  MAINFL		EMPLOYEE  		/* main file used */

#include <stdio.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_pp.h>
#include <bfs_com.h>
#include <empldrvr.h>

#define	SYSTEM		"PERSONNEL/PAYROLL"	/* Sub System Name */
#define	MOD_DATE	"18-OCT-91"		/* Program Last Modified */

#define PRGNM  		"empldrvr"

static void CloseProc() ;
static int InitProfom(), Process() ;

double	D_Roundoff();

int	DemoGraph(),
	Employment1(),
	Responsibility(),
	Earnings(),
	Miscellaneous(),
	EmpBenefit(),
	Deductions(),
	CsbLoan(),
	EmpGarn(),
	Emp_chq(),
	Emp_att(),
	Emp_senior(),
	Tch_qual(),
	Tch_ass(),
	EmpComp(),
	Emp_st();

/*-------------------------------------------------------------------*/

main(argc,argv)
int	argc;
char	*argv[];
{
	int	err ;

	strncpy(SYS_NAME,SYSTEM,50);	/* Sub system name */

	strncpy(CHNG_DATE,MOD_DATE,10);	/* Modification Date */
	proc_switch(argc, argv, MAINFL) ;  	/* Process Switches */

	err = InitProfom() ;		/* Initialize PROFOM */
	if(err == NOERROR)
		err = Process(); 	/* Initiate Process */

	CloseProc();			/* return to menu */

	if(err != NOERROR) exit(-1);
	exit(0);

} /* main() */

/*-------------------------------------------------------------------*/

/* Reset information */
static void
CloseProc()
{
	free_audit() ;	/* Free the space allocated in rite_audit() */
	close_dbh();			/* Close files */

	/* Set terminal back to normal mode from PROFOM */
	fomcs();
	fomrt();
}

/*-------------------------------------------------------------------*/
/* Initialize PROFOM */
static int
InitProfom()
{
	strcpy(sr.termnm,terminal);	/* Copy Terminal Name */


	fomin(&sr);
	/* Check for Error */
	ret( err_chk(&sr) );

	fomcf(1,1);			/* Enable Print screen option */
	return(NOERROR);
}	/* InitProfom() */
/*-------------------------------------------------------------------*/
/* Get Option from user and call corresponding function */
static int
Process()
{
	int err;

	if((SW1) && !(SW2) && !(SW3) && !(SW4) && !(SW5) && !(SW6) && !(SW7)
	   && !(SW8) && !(SW9))
		Cur_Option = SCR_1 ;	/* DEMOGRAPHIC DATA */
	else if (!(SW1) && (SW2) && !(SW3) && !(SW4) && !(SW5) && !(SW6) && !(SW7))
		Cur_Option = SCR_2 ;	/* EMPLOYMENT */
	else if (!(SW1) && !(SW2) && (SW3))
		Cur_Option = SCR_3 ;	/* RESPONSIBILITY */
	else if (!(SW1) && !(SW2) && (SW4))
		Cur_Option = SCR_4 ;	/* EARNINGS */ 
	else if (!(SW1) && !(SW2) && (SW5))
		Cur_Option = SCR_5 ;	/* MISCELLANEOUS */
	else if (!(SW1) && !(SW2) && (SW6))
		Cur_Option = SCR_6 ;	/* BENEFIT */
	else if (!(SW1) && !(SW2) && (SW7))
		Cur_Option = SCR_7 ;	/* DEDUCTION */
	else if ((SW1) && (SW2))
		Cur_Option = SCR_8 ;	/* CSB/LOAN */
	else if ((SW1) && (SW3))
		Cur_Option = SCR_9 ;	/* CHEQUE LOCATION */
	else if ((SW1) && (SW4))
		Cur_Option = SCR_10 ;	/* ATTENDANCE */
	else if ((SW1) && (SW5))
		Cur_Option = SCR_11 ;	/* SENIORITY */
	else if ((SW1) && (SW6))
		Cur_Option = SCR_12 ;	/* TEACHER QUALIFICATION */
	else if ((SW1) && (SW7))
		Cur_Option = SCR_13 ;	/* TEACHER ASSIGNMENT */
	else if ((SW2) && (SW3))
		Cur_Option = SCR_14 ;	/* GRANISHMENT DATA */
	else if ((SW2) && (SW4))
		Cur_Option = SCR_15 ;	/* COMPETITION */
	else if ((SW2) && (SW5))
		Cur_Option = SCR_16 ;	/* STATUS */
	else
		Cur_Option = '0' ;

	for( ; ; ){

		switch(Cur_Option) {
		case SCR_1 :		/* DEMOGRAPHIC DATA */
			err = DemoGraph() ;
			break ;

		case SCR_2 :		/* EMPLOYMENT DATA */
			err = Employment1() ;
			break ;

		case SCR_3 :		/* RESPONSIBILITY DATA */

			err = Responsibility() ;
			break ;

		case SCR_4 :		/* EMPLOYMENT DATA */
			err = Earnings() ;
			break ;

		case SCR_5 :		/* GARNISHMENT DATA */
			err = Miscellaneous() ;
			break ;

		case SCR_6 :		/* BENEFIT DATA */
			err = EmpBenefit() ; 
			break ;

		case SCR_7 :		/* DEDUCTION DATA */
			err = Deductions() ;
			break ;

		case SCR_8 :		/* CSB/LOAN DATA */
			err = CsbLoan() ;
			break ;

		case SCR_9 :		/* MISCELLANEOUS DATA */
			err = EmpGarn() ;
			break ;

		case SCR_10 :		/* CHEQUE DATA */
			err = Emp_chq() ;
			break ;

		case SCR_11 :		/* ATTENDANCE DATA */
			err = Emp_att() ;
			break ;

		case SCR_12 :		/* Seniority DATA */
			err = Emp_senior() ;
			break ;

		case SCR_13 :	
			err = Tch_qual() ;
			break ;

		case SCR_14 :
			err = Tch_ass() ;
			break ;

		case SCR_15 :
			err = EmpComp() ;
			break ;

		case SCR_16 :
			err = Emp_st() ;
			break ;

		default :
			printf("\n\nInvalid Switch Option\n") ;
			return(ERROR) ;
		} 

		if( err == JUMP || err == ERROR) continue ;

		if( err == QUIT ) return(NOERROR);

		return( err ); /* DBH ERROR or PROFOM_ERR */

	}      /*   end of the for( ; ; )       */

}	/* Process() */
/*--------------------End of the Program-----------------------------*/
