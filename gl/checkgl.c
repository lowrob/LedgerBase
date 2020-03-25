/******************************************************************************
		Sourcename    : checkgl.c
		System        : Budgetary Financial System.
		Subsystem     : General Ledger System 
		Module        : General Ledger 
		Created on    : Nov 7, 90.
		Created  By   : J.Prescott.
		Cobol sources : 

HISTORY:
 Date           Programmer     Description of modification
____/__/__      __________     ___________________________

******************************************************************************/
#include <stdio.h>

#define MAIN
#define MAINFL		GLMAST	/* main file used */

#include <bfs_defs.h>
#include <bfs_recs.h>
#include <cfomstrc.h>

#define SYSTEM		"General Ledger"
#define MOD_DATE	"07-NOV-90"
#define ESCAPE		12	/* flag indicates discontinuation of entries */

Pa_rec  pa_rec;
Gl_rec gl_rec;

double	D_Roundoff();

int retval;	/* Global variable to store function values */
char e_mesg[80]; /* to store error messages */
/* Initialize profom fields, call entry procedures */
main( argc, argv )
int argc;
char *argv[];
{
	double	trans_bal;
	double	total_bal;
	int	i;

	strncpy( SYS_NAME, SYSTEM, 50 ); /* Module name */
	strncpy( CHNG_DATE, MOD_DATE, 10 );/* Last date of change */
	
	proc_switch( argc,argv,MAINFL );/* process the switches */
	total_bal = 0.00;

	gl_rec.funds = 0;
	gl_rec.accno[0] = '\0';
	gl_rec.reccod = 0;

	flg_reset(GLMAST);
	for(;;) {
		retval = get_n_gl(&gl_rec,BROWSE,0,FORWARD,e_mesg);
		if(retval == EFL) 
			break;

		if(retval < 0) {
			printf("%s", e_mesg);
			exit(-1);
		}
		if(gl_rec.reccod != 99) {
			continue;
		}
		trans_bal = gl_rec.opbal;
		for(i=0;i<13;i++) {
			trans_bal += gl_rec.currel[i];
		}	
		if(D_Roundoff(gl_rec.ytd) != D_Roundoff(trans_bal))
			printf("%d-%s-%d Not Balanced by %lf\n",gl_rec.funds,gl_rec.accno,gl_rec.reccod,gl_rec.ytd - trans_bal);
	}

	close_dbh();
	exit(0);
}
