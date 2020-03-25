/*-----------------------------------------------------------------------
Source Name: convgltrhdr.c
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
#define  MAINFL		GLTRHDR   	/* main file used */

#include <stdio.h>
#include <isnames.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	SYSTEM		"GENERAL LEDGER"	/*Sub System Name */
#define	MOD_DATE	"20-MAY-92"		/* Program Last Modified */

typedef struct {
	short	th_fund ;		/* Fund */
	short	th_reccod ;		/* Record code */
	char	th_create[2] ;		/* Created E- entered G- generated */
	long	th_seq_no ;		/* Running sno under tr_userid[11]*/
	long	th_sys_dt ;		/* Trans Entered date */
	char	th_userid[11] ;		/* UserId(login name) */
	short	th_period ;		/* Period */
	long	th_date ;		/* Date of Transaction */
	double  th_debits ;		/* Optional .. Sum of all -ve amnts */
	double  th_credits ;		/* Optional .. Sum of all +ve amnts */
	char	th_descr[25] ;		/* Description */
	char	th_supp_cd[11] ;	/* Supplier number */
	char	th_reference[16] ;	/* Reference number */
	char	th_type[1] ;		/* Transaction type */
} Old_gltrhdr;

static	Old_gltrhdr old_tr;
static  Tr_hdr  tr_hdr;   

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

	strcpy(filenm,"gltrhdr");

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
	iostat = isstart(is_retval,(char *)&old_tr,0,ISFIRST);
	if(iostat < 0) {
	  printf("Error starting old sttran file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	i=0;
	for( ; ; ) {
		iostat = isreads(is_retval,(char *)&old_tr,0);
		if(iostat < 0) {
			if(iostat == EFL) break;
		  	printf("Error reading old sttran file. Iserror: %d\n"
					,iserror);
			break;
		}
		i++;
		printf("Records Converted: %d\n",i);
	tr_hdr.th_fund  = old_tr.th_fund ;
	tr_hdr.th_reccod = old_tr.th_reccod ;
	strcpy(tr_hdr.th_create,old_tr.th_create) ;
	tr_hdr.th_seq_no = old_tr.th_seq_no ;
	tr_hdr.th_sys_dt = old_tr.th_sys_dt ;
	strcpy(tr_hdr.th_userid,old_tr.th_userid) ;
	tr_hdr.th_period = old_tr.th_period ;
	tr_hdr.th_date = old_tr.th_date ;
	tr_hdr.th_debits = old_tr.th_debits ;
	tr_hdr.th_credits = old_tr.th_credits ;
	strcpy(tr_hdr.th_descr , old_tr.th_descr) ;
	strcpy(tr_hdr.th_supp_cd , old_tr.th_supp_cd) ;
	strcpy(tr_hdr.th_reference , old_tr.th_reference) ;
	tr_hdr.th_type[0] = old_tr.th_type[0] ;
	tr_hdr.th_print[0] = 'Y';

		err = put_trhdr(&tr_hdr,ADD,c_mesg);
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

