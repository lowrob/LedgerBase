/*-----------------------------------------------------------------------
Source Name: convrqit.c
System     : Budgetary Financial system.
Module     : General Ledger
Created  On: 16 May. 92  
Created  By: C. Burns

DESCRIPTION:

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
#define	MOD_DATE	"16-MAY-92"		/* Program Last Modified */

typedef struct {
	long	code;		/* requisition number */
	short	item_no;	/* requisition item number */
	char	acct[19]; 	/* Account number */
	short	fund; 		/* Fund number */
	char	st_code[11];	/* Stock code in inventory */
	char	appstat[2];	/* approval status */
	short	school;		/* school Number */
	char	desc[61];	/* Description of items */
	char	tax1[2];	/* Field for tax */
	char	tax2[2];	/* Field for 2nd tax */
	double  orig_qty;	/* Quantity originally requisted */
	char	unit[7];	/* Unit of measurement */
	double	unitprice;	/* Unit Price of the item */
	double	value;		/* Originial price of items */
	long	pocode;		/* purchase order number */
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
		
		reqitem.code = old_reqitem.code ;
		reqitem.item_no = old_reqitem.item_no ;
		strcpy(reqitem.acct, old_reqitem.acct);
		reqitem.fund = old_reqitem.fund ;
		strcpy(reqitem.st_code, old_reqitem.st_code);
		strcpy(reqitem.appstat, old_reqitem.appstat);
		reqitem.school = old_reqitem.school ;
		strcpy(reqitem.desc, old_reqitem.desc);
		strcpy(reqitem.tax1, old_reqitem.tax1);
		strcpy(reqitem.tax2, old_reqitem.tax2);
		reqitem.orig_qty = old_reqitem.orig_qty ;
		strcpy(reqitem.unit, old_reqitem.unit);
		reqitem.unitprice = old_reqitem.unitprice ;
		reqitem.value = old_reqitem.value ;
		reqitem.pocode = old_reqitem.pocode ;
		strcpy(reqitem.bdgt_flag, "N");

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

