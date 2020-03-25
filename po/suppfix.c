/*
*	Source 	: supprpt.c 
*
*	Program to Print Supplier Details using REPORT GENERATOR.
*
*/
#define  MAIN
#define  MAINFL		SUPPLIER		/* main file used */

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	SYSTEM		"PURCHASE ORDER"	/* Sub System Name */
#define	MOD_DATE	"21-NOV-90"		/* Program Last Modified */

static	Pa_rec	pa_rec ;
static  Supplier supp_rec;
/*  static	char	show[80];   */
static	char	e_mesg[80];


main(argc,argv)
int argc;
char *argv[];
{
	int code;

	strncpy(SYS_NAME,SYSTEM,50);	/* Sub system name */
	strncpy(CHNG_DATE,MOD_DATE,10);	/* Modification Date */
	proc_switch(argc, argv, MAINFL) ; 	/* Process Switches */

	code = get_param(&pa_rec, BROWSE, 1, e_mesg) ;
	if(code < 1) {
		close_dbh() ;
		return(DBH_ERR) ;
	}

	supp_rec.s_supp_cd[0] = '\0' ;
	flg_reset( SUPPLIER );

	for( ; ; ) {
		code = get_n_supplier(&supp_rec,BROWSE,0,FORWARD,e_mesg);
		if( code < 0) {
			if(code == EFL) break ;
			code = DBH_ERR;
			break ;
		}
		fprintf(stderr,"Supplier: %s\n",supp_rec.s_supp_cd);
		fprintf(stderr,"\t\t DISCOUNT:  %30.20lf \t\t YTD ORDERED: %30.20lf\n", supp_rec.s_discount,supp_rec.s_ytd_ord);
		fprintf(stderr,"\t\t YTD RETURN:  %30.20lf \t\t YTD RECEIPTS: %30.20lf\n", supp_rec.s_ytd_ret,supp_rec.s_ytd_recpt);
		fprintf(stderr,"\t\t YTD DISCOUNT:  %30.20lf \t\t BALANCE: %30.20lf\n", supp_rec.s_ytd_disc,supp_rec.s_balance);
	}

	close_dbh() ;
	if(code == EFL) return(0);
	return(code);
}
