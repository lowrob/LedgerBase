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
#define MAINFL	TIME

#include <stdio.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_pp.h>

#define	SYSTEM		"CHEQUE PRE-PROCESSING"	/* Sub System Name */
#define	MOD_DATE	"18-OCT-91"		/* Program Last Modified */

Time		time_rec;	/* Time	*/

char	e_mesg[80];
/*------------------------------------------------------------------------*/

main(argc,argv)
int	argc;
char	*argv[];
{
	int	err;
	int	i ;

	if(argc < 2){
#ifdef  DEVELOP
		printf("MAIN ARGUMENTS ARE NOT PROPER\n");
		printf("Usage: %s {-tTerminal Name} {-dDist#} [{-sSwitches}]\n",
			argv[0]);
#endif
		exit(1);
	}


	/*
	*	Initialize DBH Environment
	*/
	strncpy(SYS_NAME,SYSTEM,50);	/* Sub system name */
	strncpy(CHNG_DATE,MOD_DATE,10);	/* Modification Date */

	proc_switch(argc, argv, MAINFL) ;	/* Process Switches */

	err = Process();	/* Initiate Process */

	CloseProcess() ;

	if(err != NOERROR)exit(1);
	exit(0);
}	/* main() */


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

	time_rec.tm_numb[0] = '\0' ;
	time_rec.tm_date = 0;
	time_rec.tm_pp = 0;
	time_rec.tm_week = 0; 
	time_rec.tm_fund = 0; 
	time_rec.tm_adj[0] = '\0';
	time_rec.tm_class[0] = '\0';
	time_rec.tm_earn[0] = '\0';
	time_rec.tm_trans[0] = '\0';
	time_rec.tm_exp[0] = '\0';
	time_rec.tm_cost = 0;
	flg_reset(TIME);

	for( ; ; ){
		err = get_n_ptime(&time_rec,BROWSE,0,FORWARD,e_mesg) ;

		printf("employee = %s\n",time_rec.tm_numb);
		get();

		if(err < 0) {
			if(err == EFL) break;
			roll_back(e_mesg);
			return(err);
		}
		if(time_rec.tm_adj[0] == ' '){
			time_rec.tm_adj[0] = '\0';
		}
		else {
			continue;
		}
		err = put_ptime(&time_rec,ADD,e_mesg) ;
		if(err < 0) {
			roll_back(e_mesg);
			printf("e_mesg = %s",e_mesg);
			get();
			return(err);
		}
		time_rec.tm_adj[0] = ' ';

		err = get_ptime(&time_rec,UPDATE,0,e_mesg) ;
		if(err < 0) {
			if(err == EFL) break;
			printf("e_mesg = %s",e_mesg);
			get();
			return(err);
		}

		err = put_ptime(&time_rec,P_DEL,e_mesg) ;
		if(err < 0) {
			roll_back(e_mesg);
			printf("e_mesg = %s",e_mesg);
			get();
			return(err);
		}
		err = commit(e_mesg) ;
		if(err < 0) {
			roll_back(e_mesg);
			printf("e_mesg = %s",e_mesg);
			get();
			return(err);
		}
		flg_reset(TIME);
	}
	seq_over(TIME);

	return(NOERROR);
}
/*-------------------- E n d   O f   P r o g r a m ---------------------*/
