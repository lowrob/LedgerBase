/*-----------------------------------------------------------------------
Source Name: Pay_des.c
System     : Cheque Pre-processing
Created  On: October 18, 1991.
Created  By: Eugene Roy.

DESCRIPTION:
	Program to De-Select the employee selected for payment.

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
#define	MOD_DATE	"18-OCT-91"		/* Program Last Modified */

Gl_acct	gl_acct;


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
	char	cc[5];

	strcpy(gl_acct.gl_earn, "GRPLF");	
	gl_acct.gl_fund = 0;	
	gl_acct.gl_type[0] = '\0';	
	gl_acct.gl_class[0] = '\0';	
	gl_acct.gl_cc = 0;	
	flg_reset(GLACCT);

	for( ; ; ) {
		err = get_n_glacct(&gl_acct,BROWSE,3,FORWARD,e_mesg);
		if(err < 0) {
			if(err == EFL) break;
			return(err);
		}
		if(strcmp(gl_acct.gl_earn, "GRPLF") != 0) break;

		strcpy(gl_acct.gl_earn, "PENBUY");

		if((strcmp(gl_acct.gl_class, " 39532") == 0) ||
		   ((strcmp(gl_acct.gl_class, " 31532") >= 0) &&
		    (strcmp(gl_acct.gl_class, " 33529") <= 0)))
	  		strcpy(e_mesg,"          30007");
		if(strcmp(gl_acct.gl_class, " 34534") == 0) 
	  		strcpy(e_mesg,"          30031");
		if((strcmp(gl_acct.gl_class, " 10531") >= 0) &&
		    (strcmp(gl_acct.gl_class, " 16531") <= 0))
	  		strcpy(e_mesg,"          30065");
		if((strcmp(gl_acct.gl_class, " 41530") == 0) ||
		   (strcmp(gl_acct.gl_class, " 38529") == 0) ||
		   ((strcmp(gl_acct.gl_class, " 17531") >= 0) &&
		    (strcmp(gl_acct.gl_class, " 30529") <= 0)))
	  		strcpy(e_mesg,"          30079");
	 	if( gl_acct.gl_cc < 10)
			strcat(e_mesg,"0");
		strcpy(e_mesg,gl_acct.gl_acct);
		strncpy(gl_acct.gl_acct,e_mesg,14);
		sprintf(cc,"%d",gl_acct.gl_cc);
		strcat(e_mesg,cc);
		strncpy(gl_acct.gl_acct,e_mesg,18);

	  	err = put_glacct(&gl_acct,P_DEL,e_mesg) ;
	 	if(err < 0) {
			roll_back(e_mesg);
			strcpy(gl_acct.gl_earn, "GRPLF");	
			gl_acct.gl_cc ++;	
			flg_reset(GLACCT);

			continue;
	  	}
	  	err = put_glacct(&gl_acct,ADD,e_mesg) ;
	 	if(err < 0) {
			roll_back(e_mesg);
			strcpy(gl_acct.gl_earn, "GRPLF");	
			gl_acct.gl_cc ++;	
			flg_reset(GLACCT);

			continue;
	  	}

	  	err = commit(e_mesg) ;
	 	if(err < 0) {
			roll_back(e_mesg);
			return(err);
		}

		strcpy(gl_acct.gl_earn, "GRPLF");	
		gl_acct.gl_cc ++;	
		flg_reset(GLACCT);

	}
	seq_over(GLACCT);

	return(NOERROR);
}
/*-------------------- E n d   O f   P r o g r a m ---------------------*/
