/*-----------------------------------------------------------------------
Source Name: convcont.c
System     : Budgetary Financial system.
Module     : Control Record
Created  On: 9 May 92  
Created  By: Andre Cormier

DESCRIPTION:

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

------------------------------------------------------------------------*/
#define  MAIN
#define  MAINFL		CONTROL   	/* main file used */

#include <stdio.h>
#include <isnames.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	SYSTEM		"PURCHASE ORDERS"	/*Sub System Name */
#define	MOD_DATE	"9-MAY-92"		/* Program Last Modified */

typedef struct {
	short	fund ;			/* fund number */
	char	desc[31];		/* description of fund */
	char	ap_gen_acnt[19] ;	/* A/P General Account */
	char	ap_cnt_acnt[19] ;	/* A/P Contract Acccount */
	char	ar_gst_acnt[19] ;	/* A/P GST Acccount */
	char	dis_acnt[19] ;		/* Discount General Account */
	char	expdis_acnt[19] ;	/* Expense Discount Account */
	char	sus_acnt[19] ;		/* Suspence general Account */
	char	bank1_acnt[19];		/* Bank-1 Account */
	char	bank2_acnt[19];		/* Bank-1 Account */
	char	pst_tax_acnt[19] ;	/* Provincial tax Account */
	char	gst_tax_acnt[19] ;	/* Provincial tax Account */
	char	ar_acnt[19] ;		/* A/R General Account */
	char	ap_gst_acnt[19] ;	/* A/R General Account */
	char	inv_acnt[19] ;		/* Inventory General Account */
	char	p_and_l_acnt[19] ;	/* Profit & Loss Account */
	char	s_d_accm_acnt[19] ;	/* Surplus/Deficit Accumulated Acct */
	long	bank1_chq ;		/* Last cheque# printed for bank 1 */
	long	bank2_chq ;		/* Last cheque# printed for bank 2 */
	long	last_po ;		/* Last Po number generated */
	long	last_req;		/* Last Requisition number generated */
	short	pst_tax ;		/* Percentage of PST */
	short	gst_tax ;		/* Percentage of GST */
	short	rebate ;		/* Percentage of REBATE */
	} Old_control;

static	Old_control old_control;
static  Ctl_rec   control;   

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

	strcpy(filenm,"control");

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
	  printf("Error opening old control file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	iostat = isstart(is_retval,(char *)&old_control,0,ISFIRST);
	if(iostat < 0) {
	  printf("Error starting old supplier file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	for( ; ; ) {
		iostat = isreads(is_retval,(char *)&old_control,0);
		if(iostat < 0) {
			if(iostat == EFL) break;
		  	printf("Error reading old control file. Iserror: %d\n"
					,iserror);
			break;
		}
		
		control.fund = old_control.fund;
		strcpy(control.desc,      old_control.desc);
		strcpy(control.ap_gen_acnt,  old_control.ap_gen_acnt);
		strcpy(control.ap_cnt_acnt,       old_control.ap_cnt_acnt);
		strcpy(control.ar_gst_acnt,       old_control.ar_gst_acnt);
		strcpy(control.dis_acnt,      old_control.dis_acnt);
		strcpy(control.expdis_acnt,      old_control.expdis_acnt);
		strcpy(control.sus_acnt,      old_control.sus_acnt);
		strcpy(control.bank1_acnt,        old_control.bank1_acnt);
		strcpy(control.bank2_acnt,   old_control.bank2_acnt);
		strcpy(control.pst_tax_acnt,     old_control.pst_tax_acnt); 
		strcpy(control.gst_tax_acnt,     old_control.gst_tax_acnt);
		strcpy(control.ar_acnt,     old_control.ar_acnt);
		strcpy(control.ap_gst_acnt,     old_control.ap_gst_acnt);
		strcpy(control.inv_acnt,     old_control.inv_acnt);
		strcpy(control.p_and_l_acnt,     old_control.p_and_l_acnt);
		strcpy(control.s_d_accm_acnt,     old_control.s_d_accm_acnt);
		control.bank1_chq = old_control.bank1_chq;
		control.bank2_chq = old_control.bank2_chq;
		control.last_po = old_control.last_po;
		control.last_req = old_control.last_req;
		control.pst_tax = old_control.pst_tax;
		control.gst_tax = old_control.gst_tax;
		control.rebate = old_control.rebate;
		control.duetofrom_acct[0] = '\0';

		err = put_ctl(&control,ADD,c_mesg);
		if(err != NOERROR){
			printf(c_mesg);
			printf("ERROR in Saving new CONTROL  Records\n"); 
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

