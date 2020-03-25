/*-----------------------------------------------------------------------
Source Name: poconvd20.c
System     : Accounts Payables.
Created  On: 10th September 89.
Created  By: CATHY BURNS.

DESCRIPTION:

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification

------------------------------------------------------------------------*/

#define	MAIN		/* Main program. This is to declare Switches */
#define MAINFL		POHDR			/* main file used */

#define	SYSTEM		"PURCHASE ORDER"	/* Sub System Name */
#define	MOD_DATE	"31-MAY-90"		/* Program Last Modified */

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

static  Po_item 	po_item ;	/* Purchase Order Item Record */

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

	strncpy(SYS_NAME,SYSTEM,50);	/* Sub system name */
	strncpy(CHNG_DATE,MOD_DATE,10);	/* Modification Date */

	proc_switch(argc, argv, MAINFL) ; 	/* Process Switches */

	po_item.pi_code = 0;
	po_item.pi_item_no = 0;
	flg_reset(POITEM);

	for( ; ; ) {
		err = get_n_poitem(&po_item, UPDATE, 0, FORWARD,e_mesg);
		if(err < 0) {
			if(err == EFL) break;
			DispError(e_mesg) ;
			roll_back(e_mesg);
			break;
		}

		printf("Po: %ld  Po item: %ld\n",po_item.pi_code,po_item.pi_item_no);
		if(po_item.pi_tax2[0]=='T') {

			/* rounding  po item value */
			origfloat = (po_item.pi_value / 1.11) + .005;
			wholeint = origfloat;
			decimals = origfloat - wholeint;
			truncate = decimals * 100;
			newdec = truncate;
			newdec = newdec / 100;
			origfloat = wholeint + newdec;
			/* end of rounding */ 
			po_item.pi_value = origfloat;
		
			po_item.pi_original = po_item.pi_value;
		}
		else
			po_item.pi_tax2[0] = 'E';
			
		po_item.pi_tax1[0] = 'E';

		err = put_poitem(&po_item, UPDATE, e_mesg);
		if(err < 0) {
			DispError(e_mesg) ;
			roll_back(e_mesg);
			break; ;
		}

		po_item.pi_item_no++;

		if( commit(e_mesg)<0 ){
			DispError(e_mesg);
			return(-1);
		}
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
