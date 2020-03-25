/*-----------------------------------------------------------------------
Source Name: pocancel.c
System     : Accounts Payables.
Created  On: 10th September 89.
Created  By: CATHY BURNS.

DESCRIPTION:
	Program to cancel Purchase Orders.

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
J.PRESCOTT     90/05/31       Added Remove Item 

------------------------------------------------------------------------*/

#define	MAIN		/* Main program. This is to declare Switches */
#define MAINFL		GLMAST			/* main file used */

#define	SYSTEM		"PURCHASE ORDER"	/* Sub System Name */
#define	MOD_DATE	"31-MAY-90"		/* Program Last Modified */

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	DELTA_AMT  0.005	/* To Check float and double values for zero */
#define	DELTA_QTY  0.00005	/* To Check float and double values for zero */

static	Gl_rec	gl_rec;	/* supplier record */

static	char 	e_mesg[100];  		/* dbh will return err msg in this */


main(argc,argv)
int argc;
char *argv[];
{
	int 	err;

	strncpy(SYS_NAME,SYSTEM,50);	/* Sub system name */
	strncpy(CHNG_DATE,MOD_DATE,10);	/* Modification Date */

	proc_switch(argc, argv, MAINFL) ; 	/* Process Switches */

	gl_rec.funds = 0;
	gl_rec.accno[0] = '\0';
	gl_rec.reccod = 0;

	flg_reset(GLMAST);

	printf("Account No.                  Commit           currel3          currel4           ytd\n\n");

	for( ; ; ) {
		err = get_n_gl(&gl_rec, BROWSE, 0, FORWARD, e_mesg);
		if(err < 0) {
			if(err==EFL) break;
			DispError(e_mesg) ;
			return(DBH_ERR) ;
		}
		if(gl_rec.reccod != 99) continue;

		printf("%18.18s            %12.4lf          %12.4lf          %12.4lf          %12.4lf\n ",gl_rec.accno,gl_rec.comdat,gl_rec.currel[2],gl_rec.currel[3],gl_rec.ytd);
	}
	close_dbh();	/* Close files */
	exit(0);
}

DispError(e_mesg)
char	*e_mesg;
{
	printf("%s\n",e_mesg);
	getchar();
	return(0);
}
