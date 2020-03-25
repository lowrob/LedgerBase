/*-----------------------------------------------------------------------
Source Name: M_gltran.c   
System     : Budgetary Financial system.
Module     : Account Receivable System.
Created  On: 18th December 1990  
Created  By: F.Tao

DESCRIPTION:
	This program merges two GLTRAN data files together.

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

------------------------------------------------------------------------*/
#define  MAIN
#define  MAINFL		GLTRAN   	/* main file used */

#include <stdio.h>
#include <isnames.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	SYSTEM		"ACCOUNT RECEIVABLE"	/*Sub System Name */
#define	MOD_DATE	"JAN-25-91"		/* Program Last Modified */

static  Tr_item gltran;   
static  Tr_item oldgltran;   

static	char c_mesg[50];

int 	err;

main(argc,argv)
int argc;
char *argv[];
{
	int ti_fd;
	int iostat, err, i;
	int	fund;
	
	strncpy(SYS_NAME,SYSTEM,50);	/* Sub system name */
	strncpy(CHNG_DATE,MOD_DATE,10);	/* Modification Date */
	proc_switch(argc, argv, MAINFL) ; 	/* Process Switches */

	ti_fd = isopen("oldgltran",RWR);
	if(ti_fd < 0) {
	  printf("Error opening old gltran file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(ti_fd);
	  exit(-1);
	}

/*
	printf("Enter Fund: ");
	scanf("%d",&fund);

	gltran.ti_fund = fund;
	gltran.ti_reccod = 0;
	gltran.ti_create[0] = '\0';
	gltran.ti_seq_no = 0;
	gltran.ti_item_no = 0;

	printf("Record: %d %d %c %d %d\n",gltran.ti_fund,gltran.ti_reccod,gltran.ti_create[0],gltran.ti_seq_no,gltran.ti_item_no);
*/
	iostat = isstart(ti_fd,(char *)&gltran,0,ISFIRST);
	if(iostat < 0) {
	  printf("Error starting old gltran file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(ti_fd);
	  exit(-1);
	}
	for( ; ; ) {
		iostat = isreads(ti_fd,(char *)&gltran,0);
		if(iostat < 0) {
			if(iostat == EFL) break;
		  	printf("Error reading old gltran file. Iserror: %d\n"
					,iserror);
			break;
		}

		printf("Record: %d %d %c %d %d\n",gltran.ti_fund,gltran.ti_reccod,gltran.ti_create[0],gltran.ti_seq_no,gltran.ti_item_no);

		err = put_tritem(&gltran,ADD,c_mesg);
		if(err != NOERROR){
			printf(c_mesg);
			printf("ERROR in Saving new GLTRAN  Records\n"); 
			roll_back(c_mesg);
			exit(-1);
		}
	
		if((err = commit(c_mesg))<0) {
			printf(c_mesg);
			break;
		}
	}
	isclose(ti_fd);
	close_dbh();			/* Close files */
	exit(0);
} /* END OF MAIN */

