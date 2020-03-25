/*-----------------------------------------------------------------------
Source Name: c_pp.c
System     : Personel Payroll 
Module     : Database Maintenance

DESCRIPTION:

	Conversion file for the pay period file for updating file after
	the addition of the year field.

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
------------------------------------------------------------------------*/
#define  MAIN
#define  MAINFL		BARG   	/* main file used */


#include <stdio.h>
#include <isnames.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_pp.h>


#define	SYSTEM		"DATABASE MAINTENANCE"	/*Sub System Name */
#define	MOD_DATE	"02-SEP-92"		/* Program Last Modified */

typedef struct {
	char	pp_code[7] ;	 	/* Pay Period Code */
	short	pp_numb ;		/* Pay Period Number */
	char	pp_desc[31] ;		/* Pay Period Decription */
	} Old_pp;

static	Old_pp old_pp;
static  Pay_per   pay_period;   

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

	strcpy(filenm,"pay_period");

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
	  printf("Error opening old pay period file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	iostat = isstart(is_retval,(char *)&old_pp,0,ISFIRST);
	if(iostat < 0) {
	  printf("Error starting pay period file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	for( ; ; ) {
		iostat = isreads(is_retval,(char *)&old_pp,0);
		if(iostat < 0) {
			if(iostat == EFL) break;
	  	printf("Error reading old pay period file. Iserror: %d\n"
					,iserror);
			break;
		}
		strcpy(pay_period.pp_code,old_pp.pp_code) ;
		strcpy(pay_period.pp_desc,old_pp.pp_desc) ;
		pay_period.pp_year = 1992;
		pay_period.pp_numb = old_pp.pp_numb;

		err = put_pay_per(&pay_period,ADD,c_mesg);
		if(err != NOERROR){
			printf(c_mesg);
			printf("ERROR in Saving Pay Period Records\n"); 
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

