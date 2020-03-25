/******************************************************************************
		Sourcename    : checktr.c
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
Tr_hdr	tr_hdr;
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
	double	total_bal;
	int	i;

	strncpy( SYS_NAME, SYSTEM, 50 ); /* Module name */
	strncpy( CHNG_DATE, MOD_DATE, 10 );/* Last date of change */
	
	proc_switch( argc,argv,MAINFL );/* process the switches */
	total_bal = 0.00;

	tr_hdr.th_fund = 1;
	tr_hdr.th_reccod = 99;
	tr_hdr.th_create[0] = 'G';
	tr_hdr.th_seq_no = 10071;

	flg_reset(GLTRHDR);
	for(;;) {
		retval = get_n_trhdr(&tr_hdr,BROWSE,0,FORWARD,e_mesg);
		if(retval == EFL) 
			break;

		if(retval < 0) {
			printf("%s", e_mesg);
			exit(-1);
		}
		
		tr_item.ti_fund = tr_hdr.th_fund ;
		tr_item.ti_reccod = tr_hdr.th_reccod ;
		strcpy(tr_item.ti_create,tr_hdr.th_create);
		tr_item.ti_seq_no = tr_hdr.th_seq_no;
		tr_item.ti_item_no = 0 ;

		flg_reset(GLTRAN);
		trans_bal = 0;

		for(;;) {
		 	retval = get_n_tritem(&tr_item,BROWSE,0,0,e_mesg);
	
			if(retval == EFL) 
				break;
	
			if(retval < 0) {
				printf("%s", e_mesg);
				exit(-1);
			}

			if(tr_item.ti_seq_no != tr_hdr.th_seq_no) break;
			if(tr_item.ti_amount > 0.00)
				trans_bal += tr_item.ti_amount;
			if(strcmp(tr_item.ti_accno,"             99999")==0) {
				trans_bal += tr_item.ti_amount;
			}
		}
		
		if(D_Roundoff(trans_bal) != D_Roundoff(tr_hdr.th_debits)) {
			PrintTrans();
		}
	}

	close_dbh();
	exit(0);
}
PrintTrans() 
{
 	double cr_bal;
	double db_bal;

	tr_item.ti_fund = tr_hdr.th_fund ;
	tr_item.ti_reccod = tr_hdr.th_reccod ;
	strcpy(tr_item.ti_create,tr_hdr.th_create);
	tr_item.ti_seq_no = tr_hdr.th_seq_no;
	tr_item.ti_item_no = 0 ;

	flg_reset(GLTRAN);
	cr_bal = 0;
	db_bal = 0;

	fprintf(stderr,"seqno: %ld  period: %d  debits: %13.2lf  credits: %13.2lf\n\n",
		tr_hdr.th_seq_no,tr_hdr.th_period,tr_hdr.th_debits,tr_hdr.th_credits);
	for(;;) {
	 	retval = get_n_tritem(&tr_item,BROWSE,0,0,e_mesg);
	
		if(retval == EFL) 
			break;

		if(retval < 0) {
			printf("%s", e_mesg);
			exit(-1);
		}

		if(tr_item.ti_seq_no != tr_hdr.th_seq_no) break;
		if(tr_item.ti_amount < 0) {
			cr_bal += tr_item.ti_amount;
			fprintf(stderr,"item: %d\t%s\t\t\t%13.2lf\n",tr_item.ti_item_no,tr_item.ti_accno,tr_item.ti_amount);
		}
		else {
			db_bal += tr_item.ti_amount;
			fprintf(stderr,"item: %d\t%s\t%13.2lf\n",tr_item.ti_item_no,tr_item.ti_accno,tr_item.ti_amount);
		}
			
	}
	fprintf(stderr,"        \t%13.2lf\t\t%13.2lf\n\n",db_bal,cr_bal);
	return(0);
}
