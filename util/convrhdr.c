/*-----------------------------------------------------------------------
Source Name: convrhdr.c
System     : Budgetary Financial system.
Module     : Accounts Receivable system.
Created  On: 29 June 1992  
Created  By: Jamie McLean

DESCRIPTION:
	Change the old rcpthdr records to the new format.

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

------------------------------------------------------------------------*/
#define  MAIN
#define  MAINFL		RCPTHDR  /* main file used */

#include <stdio.h>
#include <isnames.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	SYSTEM		"ACCOUNTS RECEIVABLE"	/*Sub System Name */  
#define	MOD_DATE	"27-JUL-92"		/* Program Last Modified */

typedef struct {
	long	rhdr_refno;		/* reference number */
	short	rhdr_fund;		/* Invoice fund number */
	char	rhdr_applied[2];	/* applied or unapplied */
	char	rhdr_cust[7] ;		/* Customer unique code */
	long	rhdr_rcptdate;		/* receipt date */
	char	rhdr_chequeno[16];	/* Cheque number */
	double	rhdr_amount;		/* Receipt amount */
	short	rhdr_period;		/* G/L period */
	char	rhdr_acctno[19];	/* G/L account number */
	char	rhdr_remarks[21];	/* remarks */

}	Old_Rcpt_hdr;

static	Old_Rcpt_hdr    old_rcpthdr;
static  Rcpt_hdr	rcpthdr;   

static	char filenm[50];
static	char outfile[50];
static	char tempfile[50];
static	char c_mesg[50];

main(argc,argv)
int argc;
char *argv[];
{
	int 	retval;
	int is_retval;
	int iostat, err, i;
	
	strncpy(SYS_NAME,SYSTEM,50);	/* Sub system name */
	strncpy(CHNG_DATE,MOD_DATE,10);	/* Modification Date */
	proc_switch(argc, argv, MAINFL) ; 	/* Process Switches */

	strcpy(filenm,"rcpthdr");

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
	  printf("Error opening old rcpthdr file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	iostat = isstart(is_retval,(char *)&old_rcpthdr,0,ISFIRST);
	if(iostat < 0) {
	  printf("Error starting old rcpthdr file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	for( ; ; ) {
		iostat = isreads(is_retval,(char *)&old_rcpthdr,0);
		if(iostat < 0) {
			if(iostat == EFL) break;
		  	printf("Error reading old rcpthdr file. Iserror: %d\n"
					,iserror);
			break;
		}
	
		rcpthdr.rhdr_refno = old_rcpthdr.rhdr_refno;
		rcpthdr.rhdr_fund = old_rcpthdr.rhdr_fund;
		strcpy(rcpthdr.rhdr_applied, old_rcpthdr.rhdr_applied);
		strcpy(rcpthdr.rhdr_cust, old_rcpthdr.rhdr_cust);
		rcpthdr.rhdr_rcptdate = old_rcpthdr.rhdr_rcptdate;
		strcpy(rcpthdr.rhdr_chequeno, old_rcpthdr.rhdr_chequeno);
		rcpthdr.rhdr_amount = old_rcpthdr.rhdr_amount;
		rcpthdr.rhdr_period = old_rcpthdr.rhdr_period;
		strcpy(rcpthdr.rhdr_acctno, old_rcpthdr.rhdr_acctno);
		strcpy(rcpthdr.rhdr_remarks, old_rcpthdr.rhdr_remarks);

		retval = put_rcpthdr(&rcpthdr,ADD,c_mesg);
		if(retval != NOERROR){
			printf(c_mesg);
			printf("ERROR in Saving new RCPTHDR  Records\n"); 
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

