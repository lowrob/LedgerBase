/*-----------------------------------------------------------------------
Source Name: poconv.c 
System     : Budgetary Financial system.
Module     : Purchase Order
Created  On: 17 Dec. 90  
Created  By: J. Prescott

DESCRIPTION:

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
------------------------------------------------------------------------*/
#define  MAIN
#define  MAINFL		POHDR   	/* main file used */

#include <stdio.h>
#include <isnames.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	SYSTEM		"PURCHASE ORDERS"	/*Sub System Name */
#define	MOD_DATE	"18-DEC-90"		/* Program Last Modified */

typedef struct {
	long	ph_code;		/* Purchase order number */
	char	ph_supp_cd[11];		/* Supplier number */
	short	ph_billto;		/* Bill To Number */
	short	ph_shipto;		/* Ship To Number */
	char	ph_attention[16];	/* contact person */
	char	ph_status[1];		/* Status indicating deletion of PO */
	char	ph_type[1] ;		/* Type of Purchase Order */
	char	ph_print[1] ;		/* PO Print Status */
	long	ph_date;		/* Purchase order date */
	long	ph_due_date;		/* Purchase order due date */
	short   ph_funds;		/* Purchase order fund no */
	short	ph_period ;		/* Accounting Period */
	double	ph_comm;		/* PO total committed amount. */	
	double	ph_freight;		/* Freight amount. */	
	long	ph_lqdate;		/* Date of liquidation of PO */
	double	ph_lqamt;		/* Liquidated amount */
	} Old_pohdr;

static	Old_pohdr old_pohdr;
static  Po_hdr	 pohdr;   

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

	strcpy(filenm,"pohdr");

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
	  printf("Error opening old pohdr file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	iostat = isstart(is_retval,(char *)&old_pohdr,0,ISFIRST);
	if(iostat < 0) {
	  printf("Error starting old pohdr file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	for( ; ; ) {
		iostat = isreads(is_retval,(char *)&old_pohdr,0);
		if(iostat < 0) {
			if(iostat == EFL) break;
		  	printf("Error reading old pohdr file. Iserror: %d\n"
					,iserror);
			break;
		}

		pohdr.ph_code = old_pohdr.ph_code;		
		strcpy(pohdr.ph_supp_cd,old_pohdr.ph_supp_cd);	
		pohdr.ph_payee[0] = '\0';
		pohdr.ph_billto = old_pohdr.ph_billto;
		pohdr.ph_shipto = old_pohdr.ph_shipto;	
		strcpy(pohdr.ph_attention,old_pohdr.ph_attention);
		pohdr.ph_req_no[0] = '\0';	
		pohdr.ph_status[0] = old_pohdr.ph_status[0];
		pohdr.ph_type[0] = old_pohdr.ph_type[0];
		pohdr.ph_print[0] = old_pohdr.ph_print[0]; 
		pohdr.ph_date = old_pohdr.ph_date;
		pohdr.ph_due_date = old_pohdr.ph_due_date;	
		pohdr.ph_funds = old_pohdr.ph_funds;
		pohdr.ph_period = old_pohdr.ph_period ;
		pohdr.ph_comm = old_pohdr.ph_comm;
		pohdr.ph_lqdate = old_pohdr.ph_lqdate;	
		pohdr.ph_lqamt = old_pohdr.ph_lqamt;
		pohdr.ph_gstmsg[0] = ' ';


		err = put_pohdr(&pohdr,ADD,c_mesg);
		if(err != NOERROR){
			printf(c_mesg);
			printf("ERROR in Saving new POITEM  Records\n"); 
			roll_back(c_mesg);
			exit(-1);
		}
	
		i++;
		if(i % 10 == 0)
			if((err = commit(c_mesg))<0) {
				printf(c_mesg);
				break;
		}
	}
	if(i % 10 != 0)
		if((err = commit(c_mesg))<0) {
			printf(c_mesg);
	}
	isclose(is_retval);
	close_dbh();			/* Close files */
	exit(0);
} /* END OF MAIN */

