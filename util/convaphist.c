/*-----------------------------------------------------------------------
Source Name: convaphist.c 
System     : Budgetary Financial system.
Module     : Account Payable System.
Created  On: 17 Dec. 90  
Created  By: F. Tao

DESCRIPTION:
	This program converts the old aphist file to the new formated 
	aphist file.
	references: convap.c (J. Prescott)

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

------------------------------------------------------------------------*/
#define  MAIN
#define  MAINFL		APHIST   	/* main file used */

#include <stdio.h>
#include <isnames.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	SYSTEM		"ACCOUNT PAYABLE"	/*Sub System Name */
#define	MOD_DATE	"18-DEC-90"		/* Program Last Modified */

typedef struct {
	char	a_supp_cd[11] ; 	/* Supplier number */
	char	a_invc_no[16] ;		/* Purchase invoice number */
	char	a_tr_type[3] ;		/* IN(voice),RT(Return),CM(Cr Memo),*/
	short	a_sno ;			/* Running Sno under a_supp_cd + */
	char	a_paid[1] ;		/* Invioce Complete - 'Y'es or 'N'o */
	char	a_accno[19] ;		/* Cheque Bant Acct# */
	short	a_fund ;		/* Transaction Fund */
	long	a_chq_no ;		/* Cheque number */
	long	a_tr_date ;		/* Invoice date */
	double	a_disc_taken ;		/* Discount Taken */
	double	a_gr_amt ;		/* Cheque Gross Amount (Including */
	} Old_ap;

static	Old_ap oap_rec;
static  Ap_hist  aphist;   

static	char filenm[50];
static	char outfile[50];
static	char tempfile[50];
static	char c_mesg[50];

main(argc,argv)
int argc;
char *argv[];
{
	int ap_fd;
	int iostat, err, i;

	strncpy(SYS_NAME,SYSTEM,50);	/* Sub system name */
	strncpy(CHNG_DATE,MOD_DATE,10);	/* Modification Date */
	proc_switch(argc, argv, MAINFL) ; 	/* Process Switches */

	strcpy(filenm,"aphist");

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
	getchar();

	ap_fd = isopen(tempfile,RWR);
	if(ap_fd < 0) {
	  printf("Error opening old aphist file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(ap_fd);
	  exit(-1);
	}
	iostat = isstart(ap_fd,(char *)&oap_rec,0,ISFIRST);
	if(iostat < 0) {
	  printf("Error starting old aphist file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(ap_fd);
	  exit(-1);
	}
	for( ; ; ) {
		iostat = isreads(ap_fd,(char *)&oap_rec,0);
		if(iostat < 0) {
			if(iostat == EFL) break;
		  	printf("Error reading old aphist file. Iserror: %d\n"
					,iserror);
			break;
		}
		
		strcpy(aphist.a_supp_cd,	oap_rec.a_supp_cd);	
		strcpy(aphist.a_invc_no, 	oap_rec.a_invc_no);
		strcpy(aphist.a_tr_type,	oap_rec.a_tr_type);
		aphist.a_sno		=	oap_rec.a_sno;
		strcpy(aphist.a_paid,    	oap_rec.a_paid);
		strcpy(aphist.a_accno,   	oap_rec.a_accno);
		aphist.a_fund		=	oap_rec.a_fund;
		aphist.a_po_no		=	0;
		aphist.a_period		=	0;
		aphist.a_chq_no		=	oap_rec.a_chq_no;
		aphist.a_tr_date	=	oap_rec.a_tr_date;
		aphist.a_disc_taken	=	oap_rec.a_disc_taken;
		aphist.a_gr_amt    	=	oap_rec.a_gr_amt;

		err = put_aphist(&aphist,ADD,c_mesg);
		if(err != NOERROR){
			printf(c_mesg);
			printf("ERROR in Saving new APHIST  Records\n"); 
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
	isclose(ap_fd);
	close_dbh();			/* Close files */
	exit(0);
} /* END OF MAIN */

