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
#define MAINFL		POHDR			/* main file used */

#define	SYSTEM		"PURCHASE ORDER"	/* Sub System Name */
#define	MOD_DATE	"31-MAY-90"		/* Program Last Modified */

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	DELTA_AMT  0.005	/* To Check float and double values for zero */
#define	DELTA_QTY  0.00005	/* To Check float and double values for zero */

static	Po_hdr		po_hdr ;	/* Purchase Order Header */
static  Po_item 	po_item ;	/* Purchase Order Item Record */
static	Gl_rec		gl_rec ;	/* Gl Master rec, for general purpose */
static	Supplier	supp_rec;	/* supplier record */

static	char 	e_mesg[100];  		/* dbh will return err msg in this */


main(argc,argv)
int argc;
char *argv[];
{
	int 	err;

	/* varialbes needed for rounding and truncation because of tax */
	long truncate, wholeint;
	double origfloat, decimals, newdec;
	/* end of variables required */

	double po_commit;

	strncpy(SYS_NAME,SYSTEM,50);	/* Sub system name */
	strncpy(CHNG_DATE,MOD_DATE,10);	/* Modification Date */

	proc_switch(argc, argv, MAINFL) ; 	/* Process Switches */

	po_commit = 0.00;
	
	po_hdr.ph_code = 0;
	flg_reset(POHDR);

	for( ; ; ) {
		err = get_n_pohdr(&po_hdr, BROWSE, 0, FORWARD, e_mesg);
		if(err < 0) {
			if(err==EFL) break;
			DispError(e_mesg) ;
			return(DBH_ERR) ;
		}

		if(po_hdr.ph_status[0] == COMPLETE) {
			continue;
		}

		printf("PO#: %ld\n",po_hdr.ph_code);

		po_item.pi_code = po_hdr.ph_code;
		po_item.pi_item_no = 0;
		flg_reset(POITEM);

		for( ; ; ) {
			err = get_n_poitem(&po_item, BROWSE, 0, FORWARD,e_mesg);
			if(err < 0) {
				if(err == EFL) break;
				DispError(e_mesg) ;
				return(DBH_ERR) ;
			}
			if(po_item.pi_code != po_hdr.ph_code)
				break;
	
			if(strcmp(po_item.pi_acct,"           1000350")==0){
				printf("value: %12.6lf  paid: %12.6lf\n",po_item.pi_value,po_item.pi_paid);
				po_commit += po_item.pi_value - po_item.pi_paid;
				printf("Commit: %12.4lf\n",po_commit);
			}				
		}

	}
	printf("Commit: %12.4lf\n",po_commit);
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
