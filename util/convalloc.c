/*-----------------------------------------------------------------------
Source Name: convalloc.c 
System     : Ledger Base system
Module     : Requisitions
Created  On: 15 January 96
Created  By: L. Robichaud

DESCRIPTION: This program is to add zero to a new field that was added to the
	allocation file.  The field is reqcode that is used in the making of
	new allocations.  The requisition number will be stored in the alloc
	file to allow the more accurate tracking of allocations.  It was also
	discovered that the allocations were responsible for the stock going
	into a negative.  Before allocations of the same gl and stock were 
	being merged as one.

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
------------------------------------------------------------------------*/
#define  MAIN
#define  MAINFL		ALLOCATION   	/* main file used */

#include <stdio.h>
#include <isnames.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	SYSTEM		"REQUISITIONING"	/*Sub System Name */
#define	MOD_DATE	"15-JAN-95"		/* Program Last Modified */

typedef struct {

	short	st_fund;		/* Fund to which stock belongs */
	char	st_code[11];		/* Stock item code */
	short	st_location;		/* Location/Cost centre # */
	long	st_date;		/* Date of allocation */
	short	st_time;		/* Time of allocation */
	char	st_expacc[19];		/* Expense acnt for this allocation */
	double	st_issued;		/* Total quantity issued so far */
	double	st_alloc;		/* Balance of stock allocated */
	double	st_value;		/* Value of balance */
	} Old_acc;

static	char filenm[50];
static	char outfile[50];
static	char tempfile[50];
static	char c_mesg[50];

static	Old_acc 	old_acc;
static	Alloc_rec	aloc_rec;

main(argc,argv)
int argc;
char *argv[];
{
	int is_retval;
	int iostat, err, i;
	
	strncpy(SYS_NAME,SYSTEM,50);	/* Sub system name */
	strncpy(CHNG_DATE,MOD_DATE,10);	/* Modification Date */
	proc_switch(argc, argv, MAINFL) ; 	/* Process Switches */

	strcpy(filenm,"st_alloc");

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
	  printf("Error opening old old_acc file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	iostat = isstart(is_retval,(char *)&old_acc,0,ISFIRST);
	if(iostat < 0) {
	  printf("Error starting old acc file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	for( ; ; ) {
		iostat = isreads(is_retval,(char *)&old_acc,0);
		if(iostat < 0) {
			if(iostat == EFL) break;
		  	printf("Error reading old acc file. Iserror: %d\n"
					,iserror);
			break;
		}

		aloc_rec.st_fund = old_acc.st_fund;
		strcpy(aloc_rec.st_code, old_acc.st_code);
		aloc_rec.st_location = old_acc.st_location;
		aloc_rec.st_date = old_acc.st_date;
		aloc_rec.st_time = old_acc.st_time;
		strcpy(aloc_rec.st_expacc, old_acc.st_expacc);
		aloc_rec.st_issued = old_acc.st_issued;
		aloc_rec.st_alloc = old_acc.st_alloc;
		aloc_rec.st_value = old_acc.st_value;
		aloc_rec.st_reqcode = 0;
		
		err = put_alloc(&aloc_rec,ADD,c_mesg);
		if(err != NOERROR){
			printf(c_mesg);
			printf("ERROR in Saving new aloc_rec  Records\n"); 
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

