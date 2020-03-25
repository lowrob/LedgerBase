/*-----------------------------------------------------------------------
Source Name: podelete.c 
System     : Budgetary Financial system.
Module     : Purchase Order
Created  On: 3 Feb. 94  
Created  By: A. Cormier

DESCRIPTION:

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
------------------------------------------------------------------------*/
#define  MAIN
#define  MAINFL		POITEM   	/* main file used */

#include <stdio.h>
#include <isnames.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_recs.h>
#include <repdef.h>

#define	SYSTEM		"PURCHASE ORDERS"	/*Sub System Name */
#define	MOD_DATE	"18-DEC-90"		/* Program Last Modified */

static  Po_item	 poitem;   
static  Po_hdr	 pohdr;   

static	char filenm[50];
static	char outfile[50];
static	char tempfile[50];
static	char c_mesg[50];
static	char temp_buf[50];
static	long save_po_code;
static	int retval, i;

main(argc,argv)
int argc;
char *argv[];
{
	
	strncpy(SYS_NAME,SYSTEM,50);	/* Sub system name */
	strncpy(CHNG_DATE,MOD_DATE,10);	/* Modification Date */
	proc_switch(argc, argv, MAINFL) ; 	/* Process Switches */

	InitPrinter();

	poitem.pi_code = 0 ;
	poitem.pi_item_no = 0 ;
	flg_reset(POITEM);
	for(;;) {
		retval = get_n_poitem(&poitem,BROWSE,0,0,c_mesg);
		if(retval == EFL) break;
		if(retval < 0) {
			printf("\n\t%s",c_mesg);
			roll_back(c_mesg);
			return(retval);
		}
		save_po_code = poitem.pi_code ;
		pohdr.ph_code = poitem.pi_code ;

		retval = get_pohdr(&pohdr,BROWSE,0,c_mesg);
		if(retval < 0 && retval != UNDEF) {
			printf("\n\t%s",c_mesg);
			roll_back(c_mesg);
			return(retval);
		}
		if(retval == UNDEF) {
			retval = Delete_Item();
		}
		poitem.pi_code = save_po_code + 1;
		poitem.pi_item_no = 0 ;
		flg_reset(POITEM);
	}

	close_dbh();			/* Close files */
	close_rep(NOBANNER);

	exit(0);

} /* END OF MAIN */

Delete_Item()
{

	printf("POITEM po code: %ld \n",poitem.pi_code);

	PrntPO();

	poitem.pi_code = pohdr.ph_code ;
	poitem.pi_item_no = 0 ;

	flg_reset(POITEM);

	for(;;) {
		retval = get_n_poitem(&poitem,UPDATE,0,0,c_mesg);
		if(retval == EFL) break;
		if(retval < 0) {
			printf("\n\t%s",c_mesg);
			roll_back(c_mesg);
			return(retval);
		}

		if(poitem.pi_code != pohdr.ph_code)	break;

		retval = put_poitem(&poitem,P_DEL,c_mesg);
		if(retval != NOERROR){
			printf(c_mesg);
			printf("ERROR in Saving new POITEM  Records\n"); 
			roll_back(c_mesg);
			exit(-1);
		}
		commit(c_mesg);

		poitem.pi_item_no++;
		flg_reset(POITEM);
	}
	return(NOERROR);
}

/*-----------------------------------------------------------------*/
InitPrinter()
{
	char	resp[2] ;
	char	discfile[15] ;

	/* Always to Printer */
	STRCPY( resp, "F" );
	strcpy(discfile,"podelete.dat");

	if( opn_prnt( resp, discfile, 0, c_mesg, 0 )<0 ){
		printf(c_mesg);
		printf("ERROR in Openning printer\n"); 
		return(-1);
	}
	return(NOERROR) ;
}
/*-----------------------------------------------------------------*/
PrntPO()
{

	tedit((char *)&save_po_code,"______0_",temp_buf,R_LONG);
	mkln(1,temp_buf,8);

	if( prnt_line() < 0) return(-1) ;

	return(NOERROR) ;
}
