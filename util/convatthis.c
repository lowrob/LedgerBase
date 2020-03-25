/*-----------------------------------------------------------------------
Source Name: convatthis.c 
System     : Budgetary Financial system.
Module     : Personnel Payroll
Created  On: 26 April 95 
Created  By: L. Robichaud

DESCRIPTION: This program is to add null to a new field that was added to the
	emp_att file.  The field is vacproc that is used in the pay_cheque.c
	to determine weather to deduct from the vacation bank if the attendance
	has already been deducted then this flag is set to Y so it is not 
	re-deducted in the future.

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
------------------------------------------------------------------------*/
#define  MAIN
#define  MAINFL		EMP_ATT   	/* main file used */

#include <stdio.h>
#include <cfomstrc.h>
#include <repdef.h>
#include <isnames.h>
#include <bfs_defs.h>
#include <bfs_com.h>
#include <bfs_pp.h>

#define	SYSTEM		"PAYROLE"	/*Sub System Name */
#define	MOD_DATE	"26-APR-95"		/* Program Last Modified */

typedef struct {

	char	eah_numb[13] ;		/* Employee number    */
	char	eah_code[4] ; 		/* Attendance code	 */
	long	eah_date;		/* Date of the transaction	*/
	char	eah_teach[13] ;		/* Teacher number    */
	double	eah_sen_hours ;		/* Employee sen hours */
	double 	eah_hours ;		/* Employee hours absent */
	} Old_att_his;

static	Old_att_his old_att_his;
static  Emp_at_his	 att_his;   

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

	strcpy(filenm,"emp_att");

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
	  printf("Error opening old old_att_his file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	iostat = isstart(is_retval,(char *)&old_att_his,0,ISFIRST);
	if(iostat < 0) {
	  printf("Error starting old att_his file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	for( ; ; ) {
		iostat = isreads(is_retval,(char *)&old_att_his,0);
		if(iostat < 0) {
			if(iostat == EFL) break;
		  	printf("Error reading old att_his file. Iserror: %d\n"
					,iserror);
			break;
		}

		strcpy(att_his.eah_numb, old_att_his.eah_numb);
		strcpy(att_his.eah_code, old_att_his.eah_code); 
		att_his.eah_date = old_att_his.eah_date;
		strcpy(att_his.eah_teach, old_att_his.eah_teach);
		att_his.eah_sen_hours = old_att_his.eah_sen_hours; 
		att_his.eah_hours, old_att_his.eah_hours; 
		att_his.eah_vacproc[0] = NULL;
		
		err = put_emp_at(&att_his,ADD,c_mesg);
		if(err != NOERROR){
			printf(c_mesg);
			printf("ERROR in Saving new att_his  Records\n"); 
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

