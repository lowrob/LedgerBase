/******************************************************************************
		Sourcename    : checkbal.c
		System        : Budgetary Financial System.
		Subsystem     : General Ledger System 
		Module        : General Ledger 
		Created on    : Nov 7, 90.
		Created  By   : J.Prescott.
		Cobol sources : 

	This program is used to compare the GL and the transactions to find
out what accounts are out of balance and by how much.

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
Gl_rec  gl_rec;
Tr_item	tr_item;

double	D_Roundoff();

int retval;	/* Global variable to store function values */
char e_mesg[80]; /* to store error messages */
/* Initialize profom fields, call entry procedures */
main( argc, argv )
int argc;
char *argv[];
{
	double	trans_bal;
	int	i;

	strncpy( SYS_NAME, SYSTEM, 50 ); /* Module name */
	strncpy( CHNG_DATE, MOD_DATE, 10 );/* Last date of change */
	
	proc_switch( argc,argv,MAINFL );/* process the switches */

	gl_rec.funds = 0;
	gl_rec.accno[0] = '\0';
	gl_rec.reccod = 0;
	flg_reset(GLMAST);
	for( ; ; ) {
		retval = get_n_gl(&gl_rec,BROWSE,0,FORWARD,e_mesg);
		if(retval == EFL) 
			break;
		if(retval < 0) {
			printf("%s", e_mesg);
			exit(-1);
		}

		if(gl_rec.reccod != 99) continue;


		printf("acct: %d-%s\n",gl_rec.funds,gl_rec.accno);

		tr_item.ti_fund = gl_rec.funds ;
		tr_item.ti_reccod = gl_rec.reccod ;
		strcpy(tr_item.ti_accno,gl_rec.accno);
		tr_item.ti_seq_no = 0 ;
		tr_item.ti_item_no = 0 ;

		flg_reset(GLTRAN);
		trans_bal = gl_rec.opbal;

		for(;;) {
		 	retval = get_n_tritem(&tr_item,BROWSE,1,0,e_mesg);
	
			if(retval == EFL) 
				break;
	
			if(retval < 0) {
				printf("%s", e_mesg);
				exit(-1);
			}

			if(strcmp(tr_item.ti_accno,gl_rec.accno)!=0) break;
			trans_bal += tr_item.ti_amount;
		}
		
		if(D_Roundoff(trans_bal) != 
		   D_Roundoff(gl_rec.ytd /** + gl_rec.opbal **/)) { 
			fprintf(stderr,
		"acct: %d-%s not bal. - gl: %13.2lf trans: %13.2lf diff: %13.2lf\n",
			gl_rec.funds,gl_rec.accno,D_Roundoff(gl_rec.ytd+gl_rec.opbal),
			trans_bal,D_Roundoff(gl_rec.ytd+gl_rec.opbal)-trans_bal);
		}

		for(i=0;i<NO_PERIODS;i++) {
			if(gl_rec.currel[i] < -168958483.47 || 
			   gl_rec.currel[i] > 168958483.47) {
				fprintf(stderr,
				  "account %s currel period %d = %13.2lf\n",
					gl_rec.accno,i+1,gl_rec.currel[i]);
			}
		}
	}
	close_dbh();
	exit(0);
}
