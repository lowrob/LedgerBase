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
#define  MAINFL		SUPPLIER   	/* main file used */

#include <stdio.h>
#include <isnames.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	SYSTEM		"PURCHASE ORDERS"	/*Sub System Name */
#define	MOD_DATE	"18-DEC-90"		/* Program Last Modified */

typedef struct {
	char	s_supp_cd[11];	/* Supplier number */
	char	s_name[49];	/* Supplier Name */
	char	s_category[21];	/* Supplier Category Name */
	char	s_fax[11];	/* Supplier fax number */
	char    s_abb[25];	/* Abbreviation for supplier name */
	char    s_add1[31];	/* Address line 1 */
	char    s_add2[31];	/* Address line 2 */
	char    s_add3[31];	/* Address line 3 */
	char    s_pc[11];	/* postal code */
	char	s_contact[26];	/* contact person at supplier */
	char	s_payee[11];	/* payee code */
	char	s_phone[11];	/* Supplier phone number */
	char    s_tmp_flg[1];	/* Temporary supplier (Y,N) */
	char	s_type[1];	/* Contract or Ordinary */
	double	s_discount;	/* Supplier discount on goods */
	double  s_ytd_ord;	/* Total goods in $ ordered */ 
	double  s_ytd_ret;	/* Total goods in $ returned */ 
	double 	s_ytd_recpt;	/* Amount year to date received */
	double 	s_ytd_disc;	/* Amount year to date received */
	double 	s_balance;	/* balance */
	long	s_last_actv;	/* Date of last Activity */
	} Old_supplier;

static	Old_supplier old_supplier;
static  Supplier   supplier;   

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

	strcpy(filenm,"supplier");

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
	  printf("Error opening old supplier file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	iostat = isstart(is_retval,(char *)&old_supplier,0,ISFIRST);
	if(iostat < 0) {
	  printf("Error starting old supplier file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	for( ; ; ) {
		iostat = isreads(is_retval,(char *)&old_supplier,0);
		if(iostat < 0) {
			if(iostat == EFL) break;
		  	printf("Error reading old supplier file. Iserror: %d\n"
					,iserror);
			break;
		}
		
		strcpy(supplier.s_supp_cd,   old_supplier.s_supp_cd);
		strcpy(supplier.s_name,      old_supplier.s_name);
		strcpy(supplier.s_category,  old_supplier.s_category);
		strcpy(supplier.s_fax,       old_supplier.s_fax);
		strcpy(supplier.s_abb,       old_supplier.s_abb);
		strcpy(supplier.s_add1,      old_supplier.s_add1);
		strcpy(supplier.s_add2,      old_supplier.s_add2);
		strcpy(supplier.s_add3,      old_supplier.s_add3);
		strcpy(supplier.s_pc,        old_supplier.s_pc);
		strcpy(supplier.s_contact,   old_supplier.s_contact);
		strcpy(supplier.s_payee,     old_supplier.s_payee); 
		strcpy(supplier.s_phone,     old_supplier.s_phone);
		supplier.s_tmp_flg[0]=old_supplier.s_tmp_flg[0];
		supplier.s_type[0]=old_supplier.s_type[0];
		supplier.s_discount = old_supplier.s_discount;
		supplier.s_ytd_ord = old_supplier.s_ytd_ord;
		supplier.s_ytd_ret = old_supplier.s_ytd_ret;
		supplier.s_ytd_recpt = old_supplier.s_ytd_recpt;
		supplier.s_ytd_disc = old_supplier.s_ytd_disc;
		supplier.s_balance = old_supplier.s_balance;
		supplier.s_last_actv =	old_supplier.s_last_actv;
		supplier.s_gst_reg[0] = '\0';

		err = put_supplier(&supplier,ADD,c_mesg);
		if(err != NOERROR){
			printf(c_mesg);
			printf("ERROR in Saving new SUPPLIER  Records\n"); 
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

