/*-----------------------------------------------------------------------
Source Name: convreqh.c
System     : Budgetary Financial system.
Module     : General Ledger
Created  On: 07 Dec. 92  
Created  By: L. Robichaud

DESCRIPTION:
	Add a new field called "stock_fund" to be used to store the second
	fund to allow interfund requisitioning. It is initialy given the same 
	value as in the first fund (funds).

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

------------------------------------------------------------------------*/
#define  MAIN
#define  MAINFL		REQHDR   	/* main file used */

#include <stdio.h>
#include <isnames.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	SYSTEM		"GENERAL LEDGER"	/*Sub System Name */
#define	MOD_DATE	"07-DEC-92"		/* Program Last Modified */

typedef struct {
	long	reqh_code;		/* requisition number */
	char	reqh_supp_cd[11];	/* Supplier number */
	double	reqh_amount;		/* requisition amount. */	
	long	reqh_date;		/* requisition date */
	long	reqh_due_date;		/* requisition due date */
	double	reqh_appamt;		/* approved amount */
	short	reqh_billto;		/* Bill To Number */
	short   reqh_funds;		/* requisition fund no */
	long	reqh_appdate;		/* approval date */
	short	reqh_shipto;		/* Ship To Number */
	short	reqh_costcenter;	/* Cost Center Requesting goods */
	short	reqh_period ;		/* Accounting Period */
	char	reqh_attention[16];	/* Contact for shipment */
	char	reqh_status[2];		/* Status Approval status */ 
	char	reqh_print_form[2] ;	/* Requisition form Print Status */
	char	reqh_print_stock[2] ;	/* Requisition stock Print Status */
	char	reqh_print_rpt[2] ;	/* Requisition report Print Status */
} Old_reqhdr;

static	Old_reqhdr old_reqhdr;
static  Req_hdr  reqhdr;   

static	char filenm[50];
static	char outfile[50];
static	char tempfile[50];
static	char c_mesg[50];

main(argc,argv)
int argc;
char *argv[];
{
	int is_retval;
	int iostat, err, i;
	
	strncpy(SYS_NAME,SYSTEM,50);	/* Sub system name */
	strncpy(CHNG_DATE,MOD_DATE,10);	/* Modification Date */
	proc_switch(argc, argv, MAINFL) ; 	/* Process Switches */

	strcpy(filenm,"reqhdr");

        form_f_name(filenm,outfile);
	strcpy(tempfile,"CFXXXXXX");
	mktemp(tempfile);
#ifdef  MS_DOS
        rename(outfile,tempfile);
#else
        link(outfile,tempfile);
        unlink(outfile);        
#endif

        strcat(outfile,".IX");
	strcpy(c_mesg, tempfile) ;
        strcat(c_mesg,".IX");
#ifdef	MS_DOS
        rename(outfile,c_mesg);
#else
        link(outfile,c_mesg);
        unlink(outfile);
#endif

	printf("outfile: %s  tempfile: %s\n", outfile,tempfile);

	is_retval = isopen(tempfile,RWR);
	if(is_retval < 0) {
	  printf("Error opening old reqhdr file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	iostat = isstart(is_retval,(char *)&old_reqhdr,0,ISFIRST);
	if(iostat < 0) {
	  printf("Error starting old reqhdr file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	for( ; ; ) {
		iostat = isreads(is_retval,(char *)&old_reqhdr,0);
		if(iostat < 0) {
			if(iostat == EFL) break;
		  	printf("Error reading old reqhdr file. Iserror: %d\n"
					,iserror);
			break;
		}
		
		reqhdr.code = old_reqhdr.reqh_code;
		strcpy(reqhdr.supp_cd, old_reqhdr.reqh_supp_cd);
		reqhdr.amount = old_reqhdr.reqh_amount;
		reqhdr.date = old_reqhdr.reqh_date;
		reqhdr.due_date = old_reqhdr.reqh_due_date;
		reqhdr.appamt = old_reqhdr.reqh_appamt;
		reqhdr.billto = old_reqhdr.reqh_billto;
		reqhdr.funds = old_reqhdr.reqh_funds;
		reqhdr.appdate = old_reqhdr.reqh_appdate;
		reqhdr.shipto = old_reqhdr.reqh_shipto;
		reqhdr.costcenter = old_reqhdr.reqh_costcenter;
		reqhdr.period = old_reqhdr.reqh_period;
		strcpy(reqhdr.attention, old_reqhdr.reqh_attention);
		strcpy(reqhdr.status, old_reqhdr.reqh_status);
		strcpy(reqhdr.print_form, old_reqhdr.reqh_print_form);
		strcpy(reqhdr.print_stock, old_reqhdr.reqh_print_stock);
		strcpy(reqhdr.print_rpt, old_reqhdr.reqh_print_rpt);

		reqhdr.stock_fund = reqhdr.funds;

		err = put_reqhdr(&reqhdr,ADD,c_mesg);
		if(err != NOERROR){
			printf(c_mesg);
			printf("ERROR in Saving new REQHDR  Records\n"); 
			roll_back(c_mesg);
			exit(-1);
		}
	
		if((err = commit(c_mesg))<0) {
			printf(c_mesg);
			break;
		}
	}
	isclose(is_retval);
	close_dbh();			/* Close files */
	exit(0);
} /* END OF MAIN */

