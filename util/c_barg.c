/*-----------------------------------------------------------------------
Source Name: c_barg.c
System     : Personel Payroll 
Module     : Database Maintenance

DESCRIPTION:

	Conversion file for the bargaining unit file for updating file after
	the addition of the department field.

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
	char	b_code[7] ; 		/* Bargaining Unit Code */
	long	b_date;			/* Bargaining Unit table eff date */
	char	b_name[31] ;		/* Bargaining Unit Name */
	long	b_contract_dt;		/* Contract End Date */
	char	b_stat_hol[2] ;		/* Statutory Holiday */	
	char	b_pp_code[7] ;		/* Pay Period Code */
	short	b_fund;			/* Fund */
	char	b_cpp_acct[19] ;	/* Bargaining Unit CPP Account */
	char	b_uic_acct[19] ;	/* Bargaining Unit UIC Account */
	char	b_tax_acct[19] ;	/* Bargaining Unit Tax Account */
	double	b_sick_rate;		/* Sick day accrual rate */
	double	b_sick_max;		/* Maximum number of sick days */
	} Old_barg;

static	Old_barg old_barg;
static  Barg_unit   barg_unit;   

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

	strcpy(filenm,"barg_unit");

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
	  printf("Error opening old bargaining unit file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	iostat = isstart(is_retval,(char *)&old_barg,0,ISFIRST);
	if(iostat < 0) {
	  printf("Error starting bargaining unit file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	for( ; ; ) {
		iostat = isreads(is_retval,(char *)&old_barg,0);
		if(iostat < 0) {
			if(iostat == EFL) break;
		  	printf("Error reading old bargaining unit file. Iserror: %d\n"
					,iserror);
			break;
		}
/*		scpy((char *)&barg_unit,(char *)&old_barg,sizeof(old_barg));	*/

		strcpy(barg_unit.b_code,old_barg.b_code) ;
		barg_unit.b_date = old_barg.b_date;
		strcpy(barg_unit.b_name,old_barg.b_name) ;
		barg_unit.b_contract_dt = old_barg.b_contract_dt;
		strcpy(barg_unit.b_stat_hol,old_barg.b_stat_hol) ;
		strcpy(barg_unit.b_pp_code,old_barg.b_pp_code) ;
		barg_unit.b_fund = old_barg.b_fund;
		strcpy(barg_unit.b_cpp_acct,old_barg.b_cpp_acct) ;
		strcpy(barg_unit.b_uic_acct,old_barg.b_uic_acct) ;
		strcpy(barg_unit.b_tax_acct,old_barg.b_tax_acct) ;
		barg_unit.b_sick_rate = old_barg.b_sick_rate;
		barg_unit.b_sick_max = old_barg.b_sick_max;
		barg_unit.b_dept_cd = 0;
		barg_unit.b_stat_thrs = 0;
		barg_unit.b_stat_hpd = 0;

		err = put_barg(&barg_unit,ADD,c_mesg);
		if(err != NOERROR){
			printf(c_mesg);
			printf("ERROR in Saving Bargaining Unit Records\n"); 
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

