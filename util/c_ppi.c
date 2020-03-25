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
	char	ppi_code[7] ;	 	/* Pay Period Code */
	short	ppi_numb ;		/* Pay Period Number */
	long	ppi_st_date;		/* Pay Period start date */
	long	ppi_end_date;		/* Pay Period end date */
	short	ppi_mthly;		/* Monthly Pay Period */	
	} Old_ppi;

static	Old_ppi old_ppi;
static  Pay_per_it	pay_per_it;   

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

	strcpy(filenm,"pay_per_it");

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
	  printf("Error opening old pay period item file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	iostat = isstart(is_retval,(char *)&old_ppi,0,ISFIRST);
	if(iostat < 0) {
	  printf("Error starting pay period item file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	for( ; ; ) {
		iostat = isreads(is_retval,(char *)&old_ppi,0);
		if(iostat < 0) {
			if(iostat == EFL) break;
	  	printf("Error reading old pay period item file. Iserror: %d\n"
					,iserror);
			break;
		}
		strcpy(pay_per_it.ppi_code, old_ppi.ppi_code);
		pay_per_it.ppi_year = 1992;
		pay_per_it.ppi_numb = old_ppi.ppi_numb;
		pay_per_it.ppi_st_date = old_ppi.ppi_st_date;
		pay_per_it.ppi_end_date = old_ppi.ppi_end_date;
		pay_per_it.ppi_mthly = old_ppi.ppi_mthly;

		err = put_pp_it(&pay_per_it,ADD,c_mesg);
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

