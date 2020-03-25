/*-----------------------------------------------------------------------
Source Name: convsupp.c
System     : Budgetary Financial system.
Module     : Purchase Order
Created  On: 18 Dec. 90  
Created  By: M. Cormier

DESCRIPTION:

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

------------------------------------------------------------------------*/
#define  MAIN
#define  MAINFL		STMAST   	/* main file used */

#include <stdio.h>
#include <isnames.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	SYSTEM		"PURCHASE ORDERS"	/*Sub System Name */
#define	MOD_DATE	"18-DEC-90"		/* Program Last Modified */

typedef struct {
	short	st_fund;		/* Fund to which stock belongs */
	char	st_code[11];		/* Stock item code */
	short	st_section;		/* Stock section number */
	char	st_desc[31];		/* Description/Name of stock */
	char	st_unit[7];		/* Unit of measurement */
	double	st_on_hand;		/* Stock on hand */
	double	st_on_order;		/* Stock on order */
	double	st_alloc;		/* Stock allocated */
	double	st_paidfor;		/* Stock paid for */
	double	st_rate;		/* Average price of a unit */
	short	st_leaddays;		/* No. of leaddays for order */
	double	st_value;		/* Total value of the stock */
	double	st_min;			/* Minimum Limit - Stock on hand */
	double	st_max;			/* Maximum Limit - Stock on hand */
	char	st_accno[19];		/* Default account associated */
	long	st_lastdate;		/* Date of last receipt of stock */
	double	st_y_opb;		/* This year opening balance */
	double	st_y_iss;		/* This year issues */
	double	st_y_rec;		/* This year receipts */
	double	st_y_adj;		/* This year adjustments */
	double	st_m_opb;		/* This month opening balance */
	double	st_m_iss;		/* This month issues */
	double	st_m_rec;		/* This month receipts */
	double	st_m_adj;		/* This month adjustments */
	double	st_bef_cnt;		/* Stock before physical count */
	double	st_aft_cnt;		/* Stock after physical count */
	} Old_stmast;

static	Old_stmast old_stmast;
static  St_mast   st_mast;   

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

	strcpy(filenm,"stmast");

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
	  printf("Error opening old stmast file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	iostat = isstart(is_retval,(char *)&old_stmast,0,ISFIRST);
	if(iostat < 0) {
	  printf("Error starting old stmast file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	for( ; ; ) {
		iostat = isreads(is_retval,(char *)&old_stmast,0);
		if(iostat < 0) {
			if(iostat == EFL) break;
		  	printf("Error reading old stmast file. Iserror: %d\n"
					,iserror);
			break;
		}
		
		st_mast.st_fund = old_stmast.st_fund ;	
		strcpy(st_mast.st_code, old_stmast.st_code) ;
		st_mast.st_section = old_stmast.st_section ;
		strcpy(st_mast.st_desc, old_stmast.st_desc) ;		
		strcpy(st_mast.st_unit, old_stmast.st_unit) ;	
		st_mast.st_on_hand = old_stmast.st_on_hand ;
		st_mast.st_on_order = old_stmast.st_on_order ;		
		st_mast.st_po_ordqty = 0.00 ;	
		st_mast.st_alloc = old_stmast.st_alloc ;	
		st_mast.st_paidfor = old_stmast.st_paidfor ;
		st_mast.st_rate = old_stmast.st_rate ;
		st_mast.st_leaddays = old_stmast.st_leaddays ;		
		st_mast.st_committed = 0.00 ;	
		st_mast.st_value = old_stmast.st_value ;
		st_mast.st_min = old_stmast.st_min ;	
		st_mast.st_max = old_stmast.st_max ;
		strcpy(st_mast.st_accno, old_stmast.st_accno) ;		
		st_mast.st_lastdate = old_stmast.st_lastdate ;	
		st_mast.st_y_opb = old_stmast.st_y_opb ;
		st_mast.st_y_iss = old_stmast.st_y_iss ;		
		st_mast.st_y_rec = old_stmast.st_y_rec ;	
		st_mast.st_y_adj = old_stmast.st_y_adj ;
		st_mast.st_m_opb = old_stmast.st_m_opb ;		
		st_mast.st_m_iss = old_stmast.st_m_iss ;	
		st_mast.st_m_rec = old_stmast.st_m_rec ;
		st_mast.st_m_adj = old_stmast.st_m_adj ;		
		st_mast.st_bef_cnt = old_stmast.st_bef_cnt ;	
		st_mast.st_aft_cnt = old_stmast.st_aft_cnt ;

		err = put_stmast(&st_mast,ADD,c_mesg);
		if(err != NOERROR){
			printf(c_mesg);
			printf("ERROR in Saving new STMAST  Records\n"); 
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

