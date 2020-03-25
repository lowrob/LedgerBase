/*-----------------------------------------------------------------------
Source Name: convinvc.c
System     : Budgetary Financial system.
Module     : General Ledger
Created  On: 1 Dec. 92  
Created  By: L. Robichaud

DESCRIPTION:
	Inserts a new field called in_orgsupp_cd into the invoice file,
 	to be used to keep track of the originial supplier.
	It is given a null value.


MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

------------------------------------------------------------------------*/
#define  MAIN
#define  MAINFL		APINVOICE   	/* main file used */

#include <stdio.h>
#include <isnames.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	SYSTEM		"GENERAL LEDGER"	/*Sub System Name */
#define	MOD_DATE	"01-DEC-92"		/* Program Last Modified */

typedef struct {
	char	in_supp_cd[11] ; 	/* Supplier number */
	char	in_invc_no[16] ;	/* Purchase invoice number */
	char	in_tr_type[3] ;		/* IN,RT,CM, DM */
	char	in_type[1] ;		/* D(irect Charge),B(ulk), N(on Bulk) */
	char	in_pmtcode[1] ;		/*Open,Stop,Comp.,Relse HB,Part/Manual*/
	char	in_accno[19] ;		/* Bank Account nbr for manual cheque */
	char	in_remarks[21] ;	/* Invoice Remarks */
	short   in_funds ;		/* Fund no */
	short	in_period ;		/* Period */
	long	in_po_no ;		/* Purchase Order Number */
	long	in_invc_dt ;		/* Invoice date */
	long	in_due_dt ;		/* Payment Due date */
	long	in_chq_no ;		/* Latest Paid cheque# */
	double	in_disc_per ;		/* Discount % */
	double	in_disc_amt ;		/* Discount on Gross Amount */
	double	in_amount ;		/* Gross amount (Without Discount) */
	double	in_gsttax ;		/* GST Tax amount */
	double	in_psttax ;		/* PST Tax amount */
	double	in_part_amt ;		/* Partial Pmnt Amt/Manual Chq Paid */

} Old_invc;

static	Old_invc old_invc;
static  Invoice  invoice;   

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

	strcpy(filenm,"invoice");

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
	  printf("Error opening old invoice file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	iostat = isstart(is_retval,(char *)&old_invc,0,ISFIRST);
	if(iostat < 0) {
	  printf("Error starting old invoice file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	for( ; ; ) {
		iostat = isreads(is_retval,(char *)&old_invc,0);
		if(iostat < 0) {
			if(iostat == EFL) break;
		  	printf("Error reading old invoice file. Iserror: %d\n"
					,iserror);
			break;
		}
		
		strcpy(invoice.in_supp_cd, old_invc.in_supp_cd);
		strcpy(invoice.in_invc_no, old_invc.in_invc_no);
		strcpy(invoice.in_tr_type, old_invc.in_tr_type);
		strcpy(invoice.in_type, old_invc.in_type);
		strcpy(invoice.in_pmtcode, old_invc.in_pmtcode);
		strcpy(invoice.in_accno, old_invc.in_accno);
		strcpy(invoice.in_remarks, old_invc.in_remarks);
		invoice.in_funds = old_invc.in_funds;
		invoice.in_period = old_invc.in_period;
		invoice.in_po_no = old_invc.in_po_no;
		invoice.in_invc_dt = old_invc.in_invc_dt;
		invoice.in_due_dt = old_invc.in_due_dt;
		invoice.in_chq_no = old_invc.in_chq_no;
		invoice.in_disc_per = old_invc.in_disc_per;
		invoice.in_disc_amt = old_invc.in_disc_amt;
		invoice.in_amount = old_invc.in_amount;
		invoice.in_gsttax = old_invc.in_gsttax;
		invoice.in_psttax = old_invc.in_psttax;
		invoice.in_part_amt = old_invc.in_part_amt;

		invoice.in_orgsupp_cd[0] = '\0';
		err = put_invc(&invoice,ADD,c_mesg);
		if(err != NOERROR){
			printf(c_mesg);
			printf("ERROR in Saving new INVOICE  Records\n"); 
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

