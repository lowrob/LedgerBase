/*-----------------------------------------------------------------------
Source Name: convahdr.c
System     : Budgetary Financial system.
Module     : Accounts Receivable system.
Created  On: 29 June 1992  
Created  By: Jamie McLean

DESCRIPTION:
	Change the old arshdr records to the new format.

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

------------------------------------------------------------------------*/
#define  MAIN
#define  MAINFL		ARSHDR  /* main file used */

#include <stdio.h>
#include <isnames.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	SYSTEM		"ACCOUNTS RECEIVABLE"	/*Sub System Name */  
#define	MOD_DATE	"27-JUL-92"		/* Program Last Modified */

typedef struct {
	short	ah_fund;	/* fund# for which invoice is raised */
	long	ah_inv_no;	/* invoice number under fund */
	short	ah_sno;		/* header serial number under invoice */
	char	ah_type[3];	/* Invoice/DM/CM type code */
	char	ah_status[1];	/* Invoice status O/C : Open/Close */
	char	ah_cu_code[7];	/* customer code */
	long	ah_trandt;	/* transaction date */
	long	ah_duedt;	/* due date for the invoice */
	double	ah_oriamt;	/* original amt for which invoice is raised */
	double	ah_gramt;	/* gross amount inclusive of DM and CM */
	double	ah_txpercent;	/* tax % on net amount */
	double	ah_txamt;	/* tax amount on net amount */
	double	ah_netamt;	/* net amount */
	double	ah_balance;	/* payment outstanding against the invoice */
	short	ah_period;	/* accounting period */
	char	ah_remarks[21];	/* remarks on the invoice */
	double  ah_gstpercent;	/* GST percentage	*/
	double  ah_gstamt;	/* GST amount		*/ 

}	Old_Arshdr;

static	Old_Arshdr      old_arshdr;
static  Ar_hdr		ar_hdr;   

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

	strcpy(filenm,"arshdr");

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
	  printf("Error opening old arshdr file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	iostat = isstart(is_retval,(char *)&old_arshdr,0,ISFIRST);
	if(iostat < 0) {
	  printf("Error starting old arshdr file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	for( ; ; ) {
		iostat = isreads(is_retval,(char *)&old_arshdr,0);
		if(iostat < 0) {
			if(iostat == EFL) break;
		  	printf("Error reading old arshdr file. Iserror: %d\n"
					,iserror);
			break;
		}
		
		ar_hdr.ah_fund = old_arshdr.ah_fund;
		ar_hdr.ah_inv_no = old_arshdr.ah_inv_no;
		ar_hdr.ah_sno = old_arshdr.ah_sno;	
		strcpy(ar_hdr.ah_type, old_arshdr.ah_type);
		ar_hdr.ah_status[1] = old_arshdr.ah_status[1];
		strcpy(ar_hdr.ah_cu_code, old_arshdr.ah_cu_code);
		ar_hdr.ah_trandt = old_arshdr.ah_trandt;
		ar_hdr.ah_duedt = old_arshdr.ah_duedt;
		ar_hdr.ah_oriamt = old_arshdr.ah_oriamt;
		ar_hdr.ah_gramt = old_arshdr.ah_gramt;
		ar_hdr.ah_txpercent = old_arshdr.ah_txpercent;
		ar_hdr.ah_txamt = old_arshdr.ah_txamt;
		ar_hdr.ah_netamt = old_arshdr.ah_netamt;
		ar_hdr.ah_balance = old_arshdr.ah_balance;
		ar_hdr.ah_period = old_arshdr.ah_period;
		strcpy(ar_hdr.ah_remarks, old_arshdr.ah_remarks);
		ar_hdr.ah_gstpercent = old_arshdr.ah_gstpercent;
		ar_hdr.ah_gstamt = old_arshdr.ah_gstamt;

		retval = put_arhdr(&ar_hdr,ADD,c_mesg);
		if(retval != NOERROR){
			printf(c_mesg);
			printf("ERROR in Saving new ARSHDR  Records\n"); 
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

