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

Time		time_rec;	/* Time	*/
Pay_earn	pp_earn;	/* Pay Period Earnings File	*/
Pp_ben		pp_ben;		/* Pay Period benefit file	*/
Jr_ent		jr_ent;		/* Journal Entry File		*/


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

/*
	pp_earn.pe_numb[0] = LV_CHAR ;
	pp_earn.pe_pp = LV_SHORT;
	pp_earn.pe_date = LV_LONG;
	flg_reset(PP_EARN);

	for( ; ; ){
		err = get_n_pp_earn(&pp_earn,UPDATE, 0, FORWARD, e_mesg) ;
		if(err < 0) {
			if(err == EFL) break;
			return(ERROR) ;
		}
		err = put_pp_earn(&pp_earn,P_DEL,e_mesg) ;
		if(err < 0) {
			roll_back(e_mesg);
			return(err);
		}
	}
	pp_ben.pb_numb[0] = LV_CHAR ;
	pp_ben.pb_pp = LV_SHORT;
	pp_ben.pb_date = LV_LONG;
	pp_ben.pb_code[0] = LV_CHAR;
	flg_reset(PP_BEN);

	for( ; ; ){
		err = get_n_pp_ben(&pp_ben,UPDATE, 0, FORWARD, e_mesg) ;
		if(err < 0) {
			if(err == EFL) break;
			return(ERROR) ;
		}
		err = put_pp_ben(&pp_ben,P_DEL,e_mesg) ;
		if(err < 0) {
			roll_back(e_mesg);
			return(err);
		}
	}
	jr_ent.jr_emp_numb[0] = LV_CHAR;
	jr_ent.jr_code[0] = LV_CHAR;
	jr_ent.jr_fund = LV_SHORT;
	jr_ent.jr_no = LV_SHORT;

	for( ; ;){
		err = get_n_jr_ent(&jr_ent,UPDATE,1,FORWARD,e_mesg);
		if(err < 0) {
			if(err == EFL) break;
			return(ERROR) ;
		}

		err = put_jr_ent(&jr_ent,P_DEL,e_mesg);	
		if(err < 0) {
		  	return(ERROR);
		}
	}
*/

	time_rec.tm_numb[0] = '\0' ;
	time_rec.tm_date = 0;
	time_rec.tm_no = 0;

	flg_reset(TIME);

	for( ; ; ){
		err = get_n_ptime(&time_rec,UPDATE,0,FORWARD,e_mesg) ;
		if(err < 0) {
			if(err == EFL) break;
			roll_back(e_mesg);
			return(err);
		}
		if(strcmp(time_rec.tm_stat,"SEL") == 0){
			strcpy(time_rec.tm_stat,"ACT");
			err = put_ptime(&time_rec,UPDATE,e_mesg) ;
			if(err < 0) {
				roll_back(e_mesg);
				return(err);
			}
		}
		err = commit(e_mesg) ;
		if(err < 0) {
			roll_back(e_mesg);
			return(err);
		}
	}
	err = commit(e_mesg) ;
	if(err < 0) {
		roll_back(e_mesg);
		return(err);
	}

	return(NOERROR);
}
/*-------------------- E n d   O f   P r o g r a m ---------------------*/
