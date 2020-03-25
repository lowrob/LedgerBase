/*-----------------------------------------------------------------------
Source Name: fixreq.c
System     : Budgetary Financial system.
Module     : General Ledger
Created  On: 17 July 95
Created  By: L. Robichaud

DESCRIPTION:
	This program is to be used to remove some old requsitions that have
	been in the system since the begining.  It will purge the Processed
	requ's for before the date given.

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

------------------------------------------------------------------------*/
#define  MAIN
#define  MAINFL		REQHDR   	/* main file used */

#include <stdio.h>
#include <isnames.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	SYSTEM		"GENERAL LEDGER"	/*Sub System Name */
#define	MOD_DATE	"17-JLY-95"		/* Program Last Modified */

#define	REQ_DATE	19950430	/* Delete before this date */
#define PROCESSED	'P'		/* Processed flag */

static  Req_hdr  reqhdr;   
static  Req_item  reqitm;   

static	char c_mesg[50];

main(argc,argv)
int argc;
char *argv[];
{
	int err;
	
	strncpy(SYS_NAME,SYSTEM,50);	/* Sub system name */
	strncpy(CHNG_DATE,MOD_DATE,10);	/* Modification Date */
	proc_switch(argc, argv, MAINFL) ; 	/* Process Switches */

	reqhdr.code = 0;
	flg_reset(REQHDR) ; 

	for( ; ; ) {

		err = get_n_reqhdr((char*)&reqhdr,BROWSE,0,FORWARD,c_mesg);
		if(err == EFL) break;
		if(err < NOERROR){
			printf("\n%s",c_mesg);
			printf("\nERROR in reading next REQHDR  Records\n"); 
			getchar();
			break;
		}

		if(reqhdr.status[0] == PROCESSED && reqhdr.date < REQ_DATE){
			err = RemoveReq();
			if(err) break;
		}
	}
	close_dbh();			/* Close files */
	exit(0);
} /* END OF MAIN */


RemoveReq()
{

	int	err;


	err = get_reqhdr((char*)&reqhdr,UPDATE,0,c_mesg);
	if(err < NOERROR){
		printf("\n%s",c_mesg);
		printf("\nERROR in getting REQHDR for Update Records\n"); 
		getchar();
		return(err);
	}

	/* remove all related items */
	reqitm.code = reqhdr.code;
	reqitm.item_no = 0;

	flg_reset(REQITEM) ; 

	for(;;){

		err = get_n_reqitem((char*)&reqitm,UPDATE,0,FORWARD,c_mesg);
		if(err == EFL) break;
		if(err < NOERROR){
			printf(c_mesg);
			printf("ERROR in reading next REQITM  Records\n"); 
			getchar();
			return(err);
		}

		if(reqitm.code != reqhdr.code) break;

		err = put_reqitem(&reqitm,P_DEL,c_mesg);
		if(err != NOERROR){
			printf("\n%s",c_mesg);
			printf("\nERROR in Deleting REQITEM  Records\n"); 
			roll_back(c_mesg);
			getchar();
			return(err);
		}
		if((err = commit(c_mesg))<0) {
			printf("\n%s",c_mesg);
			printf("\nERROR in commiting Records\n"); 
			roll_back(c_mesg);
			getchar();
			return(err);
		}
	}
	
	err = put_reqhdr(&reqhdr,P_DEL,c_mesg);
	if(err != NOERROR){
		printf("\n%s",c_mesg);
		printf("\nERROR in Deleting REQHDR Records\n"); 
		roll_back(c_mesg);
		getchar();
		return(err);
	}
	if((err = commit(c_mesg))<0) {
		printf("\n%s",c_mesg);
		printf("\nERROR in commiting Records\n"); 
		roll_back(c_mesg);
		getchar();
		return(err);
	}

	return(NOERROR);
}
