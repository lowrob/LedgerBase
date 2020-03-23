/*-----------------------------------------------------------------------
Source Name: resetgl.c
System     : Cheque Pre-processing
Created  On: September 18, 1992.
Created  By: Andre Cormier.

DESCRIPTION:
	Program to Reset the amount of the gl to 0.

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
#define	MOD_DATE	"18-OCT-91"		/* Program Last Modified */

Gl_acct	gl_acct;		/* Gl master file	*/

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

	gl_acct.gl_fund = 0 ;
	gl_acct.gl_cc = 0;
	gl_acct.gl_type[0] = '\0';
	gl_acct.gl_earn[0] = '\0';
	gl_acct.gl_class[0] = '\0';
	flg_reset(GLACCT);

	for( ; ; ){
		err = get_n_glacct(&gl_acct,UPDATE,0,FORWARD,e_mesg) ;
		if(err < 0) {
			if(err == EFL) break;
			roll_back(e_mesg);
			return(err);
		}
		if(strcmp(gl_acct.gl_earn,"CPP   ") != 0){
			roll_back(e_mesg);
			continue;
		}
		else {
			printf("Fund = %d CC = %d Type = %s Earn = %s Class = %s\r",gl_acct.gl_fund,gl_acct.gl_cc,gl_acct.gl_type,gl_acct.gl_earn,gl_acct.gl_class);
			err = put_glacct(&gl_acct,P_DEL,e_mesg) ;
			if(err < 0) {
				roll_back(e_mesg);
				return(err);
			}
			err = commit(e_mesg) ;
			if(err < 0) {
				roll_back(e_mesg);
				return(err);
			}
		}
	}

	return(NOERROR);
}
/*-------------------- E n d   O f   P r o g r a m ---------------------*/
