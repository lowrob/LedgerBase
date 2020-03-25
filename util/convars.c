/*-----------------------------------------------------------------------
Source Name: R_arshdr.c   
System     : Budgetary Financial system.
Module     : Account Receivable System.
Created  On: 17 Dec. 90  
Created  By: F. Tao

DESCRIPTION:
	This program converts floating point to double numeric from the
	old arshdr file to the new formated arshdr file.
	references: convap.c (J. Prescott)

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
F.Tao          90/12/21	      Change new fields "gst" and "gst amt." to 0.
------------------------------------------------------------------------*/
#define  MAIN
#define  MAINFL		ARSHDR   	/* main file used */

#include <stdio.h>
#include <isnames.h>
#include <cfomstrc.h>
#include <bfs_defs1.h>
#include <bfs_recs1.h>

#define	SYSTEM		"ACCOUNT RECEIVABLE"	/*Sub System Name */
#define	MOD_DATE	"18-DEC-90"		/* Program Last Modified */

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
	} Old_ah;

static	Old_ah 	oah_rec;
static  Ar_hdr 	arshdr;   

static	char filenm[50];
static	char outfile[50];
static	char tempfile[50];
static	char c_mesg[50];

double 	D_FixRound();
int 	err;

main(argc,argv)
int argc;
char *argv[];
{
	int ah_fd;
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

	ah_fd = isopen(tempfile,RWR);
	if(ah_fd < 0) {
	  printf("Error opening old arshdr file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(ah_fd);
	  exit(-1);
	}
	iostat = isstart(ah_fd,(char *)&oah_rec,0,ISFIRST);
	if(iostat < 0) {
	  printf("Error starting old arshdr file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(ah_fd);
	  exit(-1);
	}
	for( ; ; ) {
		iostat = isreads(ah_fd,(char *)&oah_rec,0);
		if(iostat < 0) {
			if(iostat == EFL) break;
		  	printf("Error reading old arshdr file. Iserror: %d\n"
					,iserror);
			break;
		}
	
		arshdr.ah_fund 		= 	oah_rec.ah_fund;
		arshdr.ah_inv_no 	= 	oah_rec.ah_inv_no;
		arshdr.ah_sno		= 	oah_rec.ah_sno;
		strcpy(arshdr.ah_type, oah_rec.ah_type);
		strcpy(arshdr.ah_cu_code, oah_rec.ah_cu_code); 
		strcpy(arshdr.ah_status, oah_rec.ah_status);
		arshdr.ah_trandt 	= 	oah_rec.ah_trandt;
		arshdr.ah_duedt 	= 	oah_rec.ah_duedt;
		arshdr.ah_oriamt    = D_FixRound((double)oah_rec.ah_oriamt,&err);
		if (err == -1)
			fprintf(stderr,"\n Key: %ld-%ld-%ld   Value: %30.20lf\n",
				oah_rec.ah_fund, oah_rec.ah_inv_no,
				oah_rec.ah_sno, oah_rec.ah_oriamt);
		arshdr.ah_gramt     = D_FixRound((double)oah_rec.ah_gramt,&err);
		if (err == -1)
			fprintf(stderr,"\n Key: %ld-%ld-%ld   Value: %30.20lf\n",
				oah_rec.ah_fund, oah_rec.ah_inv_no,
				oah_rec.ah_sno, oah_rec.ah_gramt);
		arshdr.ah_txpercent = D_FixRound((double)oah_rec.ah_txpercent,
						&err);
		if (err == -1)
			fprintf(stderr,"\n Key: %ld-%ld-%ld   Value: %30.20lf\n",
				oah_rec.ah_fund, oah_rec.ah_inv_no,
				oah_rec.ah_sno, oah_rec.ah_txpercent);
		arshdr.ah_txamt     = D_FixRound((double)oah_rec.ah_txamt,&err);
		if (err == -1)
			fprintf(stderr,"\n Key: %ld-%ld-%ld   Value: %30.20lf\n",
				oah_rec.ah_fund, oah_rec.ah_inv_no,
				oah_rec.ah_sno, oah_rec.ah_txamt);
		arshdr.ah_netamt    = D_FixRound((double)oah_rec.ah_netamt,&err);
		if (err == -1)
			fprintf(stderr,"\n Key: %ld-%ld-%ld   Value: %30.20lf\n",
				oah_rec.ah_fund, oah_rec.ah_inv_no,
				oah_rec.ah_sno, oah_rec.ah_netamt);
		arshdr.ah_balance   = D_FixRound((double)oah_rec.ah_balance,
						&err);
		if (err == -1)
			fprintf(stderr,"\n Key: %ld-%ld-%ld   Value: %30.20lf\n",
				oah_rec.ah_fund, oah_rec.ah_inv_no,
				oah_rec.ah_sno, oah_rec.ah_balance);
		arshdr.ah_period  	= oah_rec.ah_period;
 		strcpy(arshdr.ah_remarks, oah_rec.ah_remarks);
		arshdr.ah_gstpercent= 0; 
/*		arshdr.ah_gstpercent= D_FixRound((double)oah_rec.ah_gstpercent,
						&err);
*/
		if (err == -1)
			fprintf(stderr,"\n Key: %ld-%ld-%ld   Value: %30.20lf\n",
				oah_rec.ah_fund, oah_rec.ah_inv_no,
				oah_rec.ah_sno, oah_rec.ah_gstpercent);
/*		arshdr.ah_gstamt = D_FixRound((double)oah_rec.ah_gstamt,&err);*/
		arshdr.ah_gstamt    = 0; 
		if (err == -1)
			fprintf(stderr,"\n Key: %ld-%ld-%ld  Value: %30.20lf\n",
				oah_rec.ah_fund, oah_rec.ah_inv_no,
				oah_rec.ah_sno, oah_rec.ah_gstamt);

		err = put_arhdr(&arshdr,ADD,c_mesg);
		if(err != NOERROR){
			printf(c_mesg);
			printf("ERROR in Saving new ARSHDR  Records\n"); 
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
	isclose(ah_fd);
	close_dbh();			/* Close files */
	exit(0);
} /* END OF MAIN */

