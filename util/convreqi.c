/*-----------------------------------------------------------------------
Source Name: convreqi.c
System     : Budgetary Financial system.
Module     : General Ledger
Created  On: 10 Dec. 92  
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
#define  MAINFL		REQITEM   	/* main file used */

#include <stdio.h>
#include <isnames.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	SYSTEM		"GENERAL LEDGER"	/*Sub System Name */
#define	MOD_DATE	"10-DEC-92"		/* Program Last Modified */

typedef struct {
	long	reqi_code;		/* requisition number */
	short	reqi_item_no;	/* requisition item number */
	char	reqi_acct[19]; 	/* Account number */
	short	reqi_fund; 		/* Fund number */
	char	reqi_st_code[11];	/* Stock code in inventory */
	char	reqi_appstat[2];	/* approval status */
	short	reqi_school;		/* school Number */
	char	reqi_desc[61];	/* Description of items */
	char	reqi_tax1[2];	/* Field for tax */
	char	reqi_tax2[2];	/* Field for 2nd tax */
	double  reqi_orig_qty;	/* Quantity originally requisted */
	char	reqi_unit[7];	/* Unit of measurement */
	double	reqi_unitprice;	/* Unit Price of the item */
	double	reqi_value;		/* Originial price of items */
	long	reqi_pocode;		/* purchase order number */
	char	reqi_bdgt_flag[2];	/* Over Budget Flag */
} Old_reqitem;

static	Old_reqitem old_reqitem;
static  Req_item  reqitem;   

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

	strcpy(filenm,"reqitem");

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
	  printf("Error opening old reqitem file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	iostat = isstart(is_retval,(char *)&old_reqitem,0,ISFIRST);
	if(iostat < 0) {
	  printf("Error starting old reqitem file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	for( ; ; ) {
		iostat = isreads(is_retval,(char *)&old_reqitem,0);
		if(iostat < 0) {
			if(iostat == EFL) break;
		  	printf("Error reading old reqitem file. Iserror: %d\n"
					,iserror);
			break;
		}
		
		reqitem.code = old_reqitem.reqi_code;
		reqitem.item_no = old_reqitem.reqi_item_no;
		strcpy(reqitem.acct, old_reqitem.reqi_acct);
		reqitem.fund = old_reqitem.reqi_fund;
		strcpy(reqitem.st_code, old_reqitem.reqi_st_code);
		strcpy(reqitem.appstat, old_reqitem.reqi_appstat);
		reqitem.school = old_reqitem.reqi_school;
		strcpy(reqitem.desc, old_reqitem.reqi_desc);
		strcpy(reqitem.tax1, old_reqitem.reqi_tax1);
		strcpy(reqitem.tax2, old_reqitem.reqi_tax2);
		reqitem.orig_qty = old_reqitem.reqi_orig_qty;
		strcpy(reqitem.unit, old_reqitem.reqi_unit);
		reqitem.unitprice = old_reqitem.reqi_unitprice;
		reqitem.value = old_reqitem.reqi_value;
		reqitem.pocode = old_reqitem.reqi_pocode;
		strcpy(reqitem.bdgt_flag, old_reqitem.reqi_bdgt_flag);

		reqitem.stock_fund = reqitem.fund;

		err = put_reqitem(&reqitem,ADD,c_mesg);
		if(err != NOERROR){
			printf(c_mesg);
			printf("ERROR in Saving new REQITEM  Records\n"); 
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

