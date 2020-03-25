/*-----------------------------------------------------------------------
Source Name: convaitm.c
System     : Budgetary Financial system.
Module     : Accounts Receivable system.
Created  On: 29 June 1992  
Created  By: Jamie McLean

DESCRIPTION:
	Change the old arsitm records to the new format.

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

------------------------------------------------------------------------*/
#define  MAIN
#define  MAINFL		ARSITEM  /* main file used */

#include <stdio.h>
#include <isnames.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	SYSTEM		"ACCOUNTS RECEIVABLE"	/*Sub System Name */  
#define	MOD_DATE	"27-JUL-92"		/* Program Last Modified */

typedef struct {

	short	ai_fund;	/* fund of the header file record */
	long	ai_inv_no;	/* Invoice number in the header file record */
	short	ai_hno;		/* Header serial number under above invoice */
	short	ai_sno;		/* Item number under above key */
	char	ai_accno[19];	/* Account number to be debited */
	double	ai_amount;	/* Amount debited to above account */
}	Old_Arsitm;

static	Old_Arsitm      old_arsitm;
static  Ar_item		ar_itm;   

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

	strcpy(filenm,"arsitem");

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
	  printf("Error opening old arsitm file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	iostat = isstart(is_retval,(char *)&old_arsitm,0,ISFIRST);
	if(iostat < 0) {
	  printf("Error starting old arsitm file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	for( ; ; ) {
		iostat = isreads(is_retval,(char *)&old_arsitm,0);
		if(iostat < 0) {
			if(iostat == EFL) break;
		  	printf("Error reading old arsitm file. Iserror: %d\n"
					,iserror);
			break;
		}
		
		ar_itm.ai_fund = old_arsitm.ai_fund;
		ar_itm.ai_inv_no = old_arsitm.ai_inv_no;
		ar_itm.ai_hno = old_arsitm.ai_hno;	
		ar_itm.ai_sno = old_arsitm.ai_sno;	
		strcpy(ar_itm.ai_accno, old_arsitm.ai_accno);
		ar_itm.ai_amount = old_arsitm.ai_amount;
		ar_itm.ai_desc[0] = '\0';

		retval = put_aritem(&ar_itm,ADD,c_mesg);
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

