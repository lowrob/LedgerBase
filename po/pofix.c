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
static  Po_item poitem;
static	Po_hdr	pohdr;

/*  static	char	show[80];   */
static	char	e_mesg[80];
static	long	prev_code;


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

	prev_code = 0;
	poitem.pi_code = 0;
	flg_reset( POITEM );

	for( ; ; ) {
		code = get_n_poitem(&poitem,UPDATE,0,FORWARD,e_mesg);
		if( code < 0) {
			if(code == EFL) break ;
			code = DBH_ERR;
			break ;
		}
		printf("Po: %ld-%ld\n",poitem.pi_code,poitem.pi_item_no);
		if(poitem.pi_code != prev_code) {
			pohdr.ph_code = poitem.pi_code;
			retval = get_pohdr(&pohdr,BROWSE,0,e_mesg);
			if( retval < 0 && retval != UNDEF) {
				code = DBH_ERR;
				break ;
			}
			if(retval != UNDEF) {
				roll_back(e_mesg);
				continue;
			}
		}

		if(retval == UNDEF) {
			if(poitem.pi_paid == poitem.pi_value) {
				code = put_poitem(&poitem,P_DEL,e_mesg);
				if( code < 0 ) {
					code = DBH_ERR;
					break ;
				}
				code = commit(e_mesg);
				if(code < 0) {
					printf("%s\n",e_mesg);
					printf("Error Saving Records\n");
					code = DBH_ERR;
					break;
				}
				printf("Po: %ld-%ld Deleted\n",poitem.pi_code,poitem.pi_item_no);
				poitem.pi_item_no++;
			}
			else {
				printf("po item: %ld-%ld not complete and no header\n",poitem.pi_code,poitem.pi_item_no);
			}
		}
		else {
			roll_back(e_mesg);
		}
		prev_code = poitem.pi_code;
	}

	close_dbh() ;
	if(code == EFL) exit(0);
	exit(0);
}
