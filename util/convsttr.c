/*-----------------------------------------------------------------------
Source Name: convsttr.c
System     : Budgetary Financial system.
Module     : General Ledger
Created  On: 20 May. 92  
Created  By: C. Burns

DESCRIPTION:

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

------------------------------------------------------------------------*/
#define  MAIN
#define  MAINFL		STTRAN   	/* main file used */

#include <stdio.h>
#include <isnames.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	SYSTEM		"GENERAL LEDGER"	/*Sub System Name */
#define	MOD_DATE	"20-MAY-92"		/* Program Last Modified */

typedef struct {
	long	st_date;		/* Transaction date */
	char	st_type[3];		/* Type of transaction */
	short	st_seq_no;		/* Transaction number */
	short	st_fund;		/* Fund to which stock belongs */
	char	st_code[11];		/* Stock item code */
	char	st_suppl_cd[11];	/* supplier code */
	char	st_ref[13];		/* Reference: PO# or Inv# */
	short	st_location;		/* Location/Cost centre # */
	short	st_period;		/* Period to which GL posting is made */
	char	st_db_acc[19];		/* Debit account number */
	char	st_cr_acc[19];		/* Credit account number */
	double	st_qty;			/* Quantity of transaction */
	double	st_amount;		/* Value of transaction */
	char	st_remarks[31];		/* Remarks on the transaction */
	short	st_fund2;		/* Fund to which stock belongs */
} Old_sttran;

static	Old_sttran old_sttran;
static  St_tran  sttran;   

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

	strcpy(filenm,"sttran");

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
	  printf("Error opening old sttran file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	iostat = isstart(is_retval,(char *)&old_sttran,0,ISFIRST);
	if(iostat < 0) {
	  printf("Error starting old sttran file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	for( ; ; ) {
		iostat = isreads(is_retval,(char *)&old_sttran,0);
		if(iostat < 0) {
			if(iostat == EFL) break;
		  	printf("Error reading old sttran file. Iserror: %d\n"
					,iserror);
			break;
		}
		
		sttran.st_date = old_sttran.st_date ;
		strcpy(sttran.st_type, old_sttran.st_type) ;
		sttran.st_seq_no = old_sttran.st_seq_no ;
		sttran.st_fund = old_sttran.st_fund ;
		strcpy(sttran.st_code, old_sttran.st_code) ;
		strcpy(sttran.st_suppl_cd, old_sttran.st_suppl_cd) ;
		strcpy(sttran.st_ref, old_sttran.st_ref) ;
		sttran.st_location = old_sttran.st_location ;
		sttran.st_period = old_sttran.st_period ;
		strcpy(sttran.st_db_acc, old_sttran.st_db_acc) ;
		strcpy(sttran.st_cr_acc, old_sttran.st_cr_acc) ;
		sttran.st_qty = old_sttran.st_qty ;
		sttran.st_amount = old_sttran.st_amount ;
		strcpy(sttran.st_remarks, old_sttran.st_remarks) ;
		sttran.st_fund2 = 1;
		if(strcmp(sttran.st_type, "AL") == 0) {
			sttran.st_fund2=0;
		}
		
		if(strcmp(sttran.st_type, "RE") == 0) {
			sttran.st_fund=0;
			sttran.st_fund2=0;
		}

		if(strcmp(sttran.st_type, "RS") == 0) {
			sttran.st_fund=0;
			sttran.st_fund2=0;
		}

		sttran.st_po_no = 0;
		err = put_sttran(&sttran,ADD,c_mesg);
		if(err != NOERROR){
			printf(c_mesg);
			printf("ERROR in Saving new STTRAN  Records\n"); 
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

