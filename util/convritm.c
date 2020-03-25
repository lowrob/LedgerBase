/*-----------------------------------------------------------------------
Source Name: convritm.c
System     : Budgetary Financial system.
Module     : Accounts Receivable system.
Created  On: 29 June 1992  
Created  By: Jamie McLean

DESCRIPTION:
	Converts the old rcptitem records to the new format.

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

------------------------------------------------------------------------*/
#define  MAIN
#define  MAINFL		RCPTITEM  /* main file used */

#include <stdio.h>
#include <isnames.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	SYSTEM		"ACCOUNTS RECEIVABLE"	/*Sub System Name */  
#define	MOD_DATE	"27-JUL-92"		/* Program Last Modified */

typedef struct {

	long	ritm_refno;		/* reference number */
	long	ritm_seqno ;		/* sequence number */
	long	ritm_invnumb;		/* Invoice number */
	char	ritm_cust[7] ;		/* Customer unique code */
	double	ritm_amount;		/* Receipt amount */

}	Old_Rcpt_item;

static	Old_Rcpt_item   old_rcptitem;
static  Rcpt_item	rcptitem;   

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

	strcpy(filenm,"rcptitem");

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
	  printf("Error opening old rcptitem file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	iostat = isstart(is_retval,(char *)&old_rcptitem,0,ISFIRST);
	if(iostat < 0) {
	  printf("Error starting old rcptitem file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	for( ; ; ) {
		iostat = isreads(is_retval,(char *)&old_rcptitem,0);
		if(iostat < 0) {
			if(iostat == EFL) break;
		  	printf("Error reading old rcptitem file. Iserror: %d\n"
					,iserror);
			break;
		}

		rcptitem.ritm_refno = old_rcptitem.ritm_refno;
		rcptitem.ritm_seqno =  old_rcptitem.ritm_seqno;
		rcptitem.ritm_invnumb = old_rcptitem.ritm_invnumb;
		strcpy(rcptitem.ritm_cust,old_rcptitem.ritm_cust);
		rcptitem.ritm_amount = old_rcptitem.ritm_amount;

		err = put_rcptitem(&rcptitem,ADD,c_mesg);
		if(err != NOERROR){
			printf(c_mesg);
			printf("ERROR in Saving new RCPTITEM  Records\n"); 
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

