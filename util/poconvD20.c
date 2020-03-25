/*-----------------------------------------------------------------------
Source Name: poconv.c 
System     : Budgetary Financial system.
Module     : Purchase Order
Created  On: 17 Dec. 90  
Created  By: M. Cormier

DESCRIPTION:
	This program converts floating point to double numeric from  the
	old poitem file to the new formated poitem file.
	references: convap.c (J. Prescott)

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
F.Tao 	       90/12/21	      Change all tax flags to EXCEMPT. 
------------------------------------------------------------------------*/
#define  MAIN
#define  MAINFL		POITEM   	/* main file used */

#include <stdio.h>
#include <isnames.h>
#include <cfomstrc.h>
#include <bfs_defs1.h>
#include <bfs_recs1.h>

#define	SYSTEM		"PURCHASE ORDERS"	/*Sub System Name */
#define	MOD_DATE	"18-DEC-90"		/* Program Last Modified */

typedef struct {
	long	pi_code;		/* Purchase order number */
	short	pi_item_no;		/* Purchase order item number */
	short	pi_school;		/* school Number */
	short	pi_fund;		/* Fund Number */
	char	pi_acct[19]; 		/* Account number */
	char	pi_st_code[11];		/* Stock code in inventory */
	char	pi_desc[61];		/* Description of item */
	char	pi_req_no[11];		/* Requisition number */
	char	pi_unit[7];		/* Unit of measurement */
	char	pi_tax1[2];		/* Field for tax */
	char	pi_tax2[2];		/* Field for 2nd tax */
	double	pi_pd_qty;		/* Quantity ordered and paid for */
	double	pi_unitprice;		/* Unit Price of the item */
	double  pi_orig_qty;		/* Quantity originally ordered */
	double	pi_original;		/* Originial price of item */
	double	pi_paid;		/* Price paid for the item */
	double  pi_value;		/* Change in the originial price */
	} Old_poitem;

static	Old_poitem old_poitem;
static  Po_item	 poitem;   

static	char filenm[50];
static	char outfile[50];
static	char tempfile[50];
static	char c_mesg[50];

double 	D_FixRound();

main(argc,argv)
int argc;
char *argv[];
{
	int is_retval;
	int iostat, err, i;
	
	strncpy(SYS_NAME,SYSTEM,50);	/* Sub system name */
	strncpy(CHNG_DATE,MOD_DATE,10);	/* Modification Date */
	proc_switch(argc, argv, MAINFL) ; 	/* Process Switches */

	strcpy(filenm,"poitem");

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
	  printf("Error opening old poitem file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	iostat = isstart(is_retval,(char *)&old_poitem,0,ISFIRST);
	if(iostat < 0) {
	  printf("Error starting old poitem file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	for( ; ; ) {
		iostat = isreads(is_retval,(char *)&old_poitem,0);
		if(iostat < 0) {
			if(iostat == EFL) break;
		  	printf("Error reading old poitem file. Iserror: %d\n"
					,iserror);
			break;
		}
		
		poitem.pi_code	      =	old_poitem.pi_code;
		poitem.pi_item_no     =	old_poitem.pi_item_no;
		poitem.pi_school      =	old_poitem.pi_school;
		poitem.pi_fund        =	old_poitem.pi_fund;
		strcpy(poitem.pi_acct,  	old_poitem.pi_acct);	
		strcpy(poitem.pi_st_code,	old_poitem.pi_st_code);
		strcpy(poitem.pi_desc,  	old_poitem.pi_desc);
		strcpy(poitem.pi_req_no,	old_poitem.pi_req_no);
		strcpy(poitem.pi_unit,  	old_poitem.pi_unit);
		strcpy(poitem.pi_tax1,  	old_poitem.pi_tax1);
		strcpy(poitem.pi_tax2,  	old_poitem.pi_tax2);
		poitem.pi_pd_qty   
		   = D_FixRound((double)old_poitem.pi_pd_qty,&err);
		if(err == -1)
			fprintf(stderr,"PO number  %ld\n PO item no.  %d \n Quantity ordered and paid for  %30.20lf\n",
			old_poitem.pi_code,old_poitem.pi_item_no,old_poitem.pi_pd_qty); 
		poitem.pi_unitprice
		      = D_FixRound((double)old_poitem.pi_unitprice,&err);
		if(err == -1)
			fprintf(stderr,"PO number  %ld\n PO item no.  %d \n Unit price  %30.20lf\n",
			old_poitem.pi_code,old_poitem.pi_item_no,old_poitem.pi_unitprice);
		poitem.pi_orig_qty
		      = D_FixRound((double)old_poitem.pi_orig_qty,&err);
		if(err == -1)
			fprintf(stderr,"PO number  %ld\n PO item no.  %d \n Quantity originally ordered  %30.20lf\n",
			old_poitem.pi_code,old_poitem.pi_item_no,old_poitem.pi_orig_qty);
		poitem.pi_original
		      = D_FixRound((double)old_poitem.pi_original,&err);
		if(err == -1)
			fprintf(stderr,"PO number  %ld\n PO item no.  %d \n Original price of item  %30.20lf",
			old_poitem.pi_code,old_poitem.pi_item_no,old_poitem.pi_original);
		poitem.pi_paid
		      = D_FixRound((double)old_poitem.pi_paid,&err);
		if(err == -1)
			fprintf(stderr,"PO number  %ld\n PO item no.  %d \n Price paid for the item  %30.20lf\n",
			old_poitem.pi_code,old_poitem.pi_item_no,old_poitem.pi_paid);
		poitem.pi_value
		      = D_FixRound((double)old_poitem.pi_value,&err);
		if(err == -1)
			fprintf(stderr,"PO number  %ld\n PO item no.  %d \n Change in the orig. price  %30.20lf\n",
			old_poitem.pi_code,old_poitem.pi_item_no,old_poitem.pi_value);

		err = put_poitem(&poitem,ADD,c_mesg);
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

