/*
*	Source 	: supprpt.c 
*
*	Program to Print Supplier Details using REPORT GENERATOR.
*
*/
#define  MAIN
#define  MAINFL		STMAST		/* main file used */

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	SYSTEM		"STOCK"	/* Sub System Name */
#define	MOD_DATE	"21-NOV-90"		/* Program Last Modified */

static	Pa_rec	pa_rec ;
static  St_mast st_mast;

/*  static	char	show[80];   */
static	char	e_mesg[80];
static	double	qty;

main(argc,argv)
int argc;
char *argv[];
{
	int code;
	int retval;

	strncpy(SYS_NAME,SYSTEM,50);	/* Sub system name */
	strncpy(CHNG_DATE,MOD_DATE,10);	/* Modification Date */
	proc_switch(argc, argv, MAINFL) ; 	/* Process Switches */

	code = get_param(&pa_rec, BROWSE, 1, e_mesg) ;
	if(code < 1) {
		close_dbh() ;
		exit(-1) ;
	}

	st_mast.st_fund = 0;
	st_mast.st_code[0] = '\0';
	flg_reset( STMAST );

	for( ; ; ) {
		code = get_n_stmast(&st_mast,UPDATE,0,FORWARD,e_mesg);
		if( code < 0) {
			if(code == EFL) break ;
			printf("%s\n",e_mesg);
			getchar();
			break ;
		}
		if(st_mast.st_committed>=0.00 && st_mast.st_po_ordqty>=0.00){
			roll_back(e_mesg);
			continue;
		}
		printf("Stock: %d-%s\n",st_mast.st_fund,st_mast.st_code);

		st_mast.st_committed = 0.00;
		st_mast.st_po_ordqty = 0.00;

		qty = st_mast.st_on_hand + st_mast.st_paidfor;
		if(qty > 0.00) {
			st_mast.st_rate =  st_mast.st_value / 
				(st_mast.st_on_hand + st_mast.st_paidfor);
		}
		code = put_stmast(&st_mast,UPDATE,e_mesg);
		if( code < 0) {
			printf("%s\n",e_mesg);
			getchar();
			break ;
		}

		code = commit(e_mesg);
		if(code < 0) {
			printf("%s\n",e_mesg);
			printf("Error Saving Records\n");
			roll_back(e_mesg);
			break;
		}
		inc_str(st_mast.st_code,sizeof(st_mast.st_code)-1,FORWARD);
	}

	close_dbh() ;
	if(code == EFL) exit(0);
	exit(0);
}
