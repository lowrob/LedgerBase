/*-----------------------------------------------------------------------
Source Name: convbdhr.c
System     : Budgetary Financial system.
Module     : General Ledger
Created  On: 19 Aug. 91  
Created  By: C. Burns

DESCRIPTION:

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

------------------------------------------------------------------------*/
#define  MAIN
#define  MAINFL		GLBDITEM   	/* main file used */

#include <stdio.h>
#include <isnames.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	SYSTEM		"GENERAL LEDGER"	/*Sub System Name */
#define	MOD_DATE	"19-AUG-91"		/* Program Last Modified */

typedef struct {
	char	tr_term[4] ;		/* Terminal name */
	long	tr_sys_dt ;		/* Trans Entered date */
	short	tr_seq_no ;		/* Running sno under tr_term+tr_sys_dt*/
	short	tr_item_no ;		/* Item No. 	                  */
	short	tr_fund ;		/* Fund Code        Key-2, Part-2 */
	short	tr_reccod ;		/* Record code      Key-2, Part-3 */
	char	tr_accno[19] ;		/* Account code     Key-2, part-4 */
	short	tr_period ;		/* Period           Key-2, Part-1 */
	double	tr_amount ;		/* Transaction Amount 	  	  */
	short	tr_status ;		/* Transaction Update status 	  */
	} Old_bditem;

static	Old_bditem old_bditem;
static  Bd_item  bd_item;   

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

	strcpy(filenm,"glbditem");

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
	  printf("Error opening old glbditem file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	iostat = isstart(is_retval,(char *)&old_bditem,0,ISFIRST);
	if(iostat < 0) {
	  printf("Error starting old glbditem file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	for( ; ; ) {
		iostat = isreads(is_retval,(char *)&old_bditem,0);
		if(iostat < 0) {
			if(iostat == EFL) break;
		  	printf("Error reading old glbditem file. Iserror: %d\n"
					,iserror);
			break;
		}
		
		strcpy(bd_item.tr_term,   old_bditem.tr_term);
		bd_item.tr_sys_dt = old_bditem.tr_sys_dt;
		bd_item.tr_seq_no = old_bditem.tr_seq_no;
		bd_item.tr_item_no = old_bditem.tr_item_no;
		bd_item.tr_fund = old_bditem.tr_fund;
		bd_item.tr_reccod = old_bditem.tr_reccod;
		strcpy(bd_item.tr_accno,   old_bditem.tr_accno);
		bd_item.tr_period = old_bditem.tr_period;
		bd_item.tr_amount = old_bditem.tr_amount;

		err = put_bditem(&bd_item,ADD,c_mesg);
		if(err != NOERROR){
			printf(c_mesg);
			printf("ERROR in Saving new GLBDHDR  Records\n"); 
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

