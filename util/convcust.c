/*-----------------------------------------------------------------------
Source Name: convcust.c
System     : Budgetary Financial system.
Module     : Accounts Receivable system.
Created  On: 29 June 1992  
Created  By: M. Galvin 

DESCRIPTION: This program is used to convert the old supplier file's format
             to the new format.  A new field for the customer's abreviated
	     name has been added and the postal code field ( or zip code field)
             has been increased in size from 7 to 10 characters in size.  The 
	     new field allows the operator to sort or read the file by another
             key other than the customer's name or code.  Each record on the old	     customer file is read and the data is placed onto the new
	     customer file that has the new file layout.  This program is
	     a batch program and it warns the operator if there are any problems
             opening, reading, or rewriting the files. 

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

------------------------------------------------------------------------*/
#define  MAIN
#define  MAINFL		CUSTOMER  /* main file used */

#include <stdio.h>
#include <isnames.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	SYSTEM		"ACCOUNTS RECEIVABLE"	/*Sub System Name */  
#define	MOD_DATE	"30-JUN-92"		/* Program Last Modified */

typedef struct {
	char	cu_code[7];	/* Customer unique code */
	char	cu_name[31];	/* Customer's Name */
	char	cu_adr1[31];	/* Customer's address line 1 */
	char	cu_adr2[31];	/* Customer's address line 2 */
	char    cu_adr3[31];	/* Customer's address line 3 */
	char    cu_pc[8];	/* postal code */
	char	cu_phone[11];	/* Phone number */
	char	cu_fax[11];	/* Fax number */
	char	cu_contper[26];	/* Contact person */
	char    cu_prnt_cd[1];	/* Statement Print Code.. 
					A - Always Print
					B - Don't print if balance = 0
					C - Don't print if balance < 0
					D - Don't print if balance <= 0 */
        long  	cu_open_dt ;   /* Either date of account opened or 
				business started */	
	long	cu_sale_dt ;	/* Last sales date */
	long    cu_rcpt_dt ;	/* Last Receipt date */ 
	double  cu_mon_op ;	/* Monthly opening balance in value */ 
	double 	cu_cur_bal ;	/* Current outstanding balance */ 
	double 	cu_ytd_sales ;	/* YTD sales */
	double 	cu_ytd_rcpts ;	/* YTD receipts */
	} Old_customer;

static	Old_customer old_customer;
static  Cu_rec		cu_rec;   

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

	strcpy(filenm,"customer");

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
	  printf("Error opening old customer file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	iostat = isstart(is_retval,(char *)&old_customer,0,ISFIRST);
	if(iostat < 0) {
	  printf("Error starting old customer file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	for( ; ; ) {
		iostat = isreads(is_retval,(char *)&old_customer,0);
		if(iostat < 0) {
			if(iostat == EFL) break;
		  	printf("Error reading old customer file. Iserror: %d\n"
					,iserror);
			break;
		}
		
		strcpy(cu_rec.cu_code,     old_customer.cu_code);
		Right_Justify_Numeric(cu_rec.cu_code,sizeof(cu_rec.cu_code)-1);
		strcpy(cu_rec.cu_name,     old_customer.cu_name);
	        cu_rec.cu_abrev[0]='\0';	
		strcpy(cu_rec.cu_adr1,       old_customer.cu_adr1);
		strcpy(cu_rec.cu_adr2,     old_customer.cu_adr2);
		strcpy(cu_rec.cu_adr3,     old_customer.cu_adr3);
		strcpy(cu_rec.cu_pc,       old_customer.cu_pc);
		strcpy(cu_rec.cu_phone,    old_customer.cu_phone);
		strcpy(cu_rec.cu_fax,      old_customer.cu_fax);
		strcpy(cu_rec.cu_contper,  old_customer.cu_contper);
		strcpy(cu_rec.cu_prnt_cd,  old_customer.cu_prnt_cd);
		cu_rec.cu_open_dt = old_customer.cu_open_dt; 
		cu_rec.cu_sale_dt = old_customer.cu_sale_dt;
		cu_rec.cu_rcpt_dt = old_customer.cu_rcpt_dt;
		cu_rec.cu_mon_op = old_customer.cu_mon_op;
		cu_rec.cu_cur_bal = old_customer.cu_cur_bal;
		cu_rec.cu_ytd_sales = old_customer.cu_ytd_sales;
		cu_rec.cu_ytd_rcpts = old_customer.cu_ytd_rcpts;

		err = put_cust(&cu_rec,ADD,c_mesg);
		if(err != NOERROR){
			printf(c_mesg);
			printf("ERROR in Saving new CUSTOMER  Records\n"); 
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

