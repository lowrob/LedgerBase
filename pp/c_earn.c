/*-----------------------------------------------------------------------
Source Name: c_earn.c
System     : Personel Payroll 
Module     : Database Maintenance

DESCRIPTION:

	Conversion file for employee earnings history file for updating
	file after the additions of fields.

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
------------------------------------------------------------------------*/
#define  MAIN
#define  MAINFL		EMP_EARN   	/* main file used */


#include <stdio.h>
#include <isnames.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_pp.h>


#define	SYSTEM		"DATABASE MAINTENANCE"	/*Sub System Name */
#define	MOD_DATE	"07-JAN-92"		/* Program Last Modified */

typedef struct {
	char	en_numb[13] ;		/* Employee number    */
	short	en_pp;			/* Pay period		*/
	long	en_date;		/* Date of the transaction	*/
	double	en_reg_units;		/* Regular units      */
	double	en_high_units;		/* High units         */
	double	en_reg_inc;		/* Reg income		*/
	double	en_high_inc;		/* High income    */
	double	en_def_inc;		/* Deffered income    */
	double	en_cpp;			/* CPP contribution */
	double	en_cpp_pen;		/* CPP pensionable earnings	*/
	double	en_uic;			/* UIC premiums 	*/
	double	en_reg1;		/* Registered pen plan contr rate1 */
	double	en_reg2;		/* Registered pen plan contr rate2 */
	double	en_reg3;		/* Registered pen plan contr rate3 */
	double	en_tax;			/* Income tax		*/
	double	en_net;			/* Net income	*/
	long	en_chq_no;		/* Cheque No 		*/
	char	en_chq_type[2];		/* Type M(anual) or R(egular) */
	char	en_accno[19];		/* Account Number */
	double	en_reg_prior;
	double	en_reg_opt;
	double	en_reg_nonm;
	short	en_week;
	short	en_year;		/* the year earnings were earned */
	} Old_emp_earn;

static	Old_emp_earn old_emp_earn;
static  Emp_earn   emp_earn;   
static  Emp   emp_rec;   

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

	strcpy(filenm,"emp_earn");

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
	  printf("Error opening old employee earnings history file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	iostat = isstart(is_retval,(char *)&old_emp_earn,0,ISFIRST);
	if(iostat < 0) {
	  printf("Error starting old employee earnings history file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	for( ; ; ) {
		iostat = isreads(is_retval,(char *)&old_emp_earn,0);
		if(iostat < 0) {
			if(iostat == EFL) break;
		  	printf("Error reading old employee earnings history file. Iserror: %d\n"
					,iserror);
			break;
		}
		scpy((char *)&emp_earn,(char *)&old_emp_earn,sizeof(old_emp_earn));	


		strcpy(emp_rec.em_numb, emp_earn.en_numb);

		err = get_employee(&emp_rec,BROWSE,0,c_mesg);
		if( err < 0) {
			fomen(c_mesg);
			get();
			return(-1);
		}

		strcpy(emp_earn.en_reg_pen,emp_rec.em_reg_pen);

		err = put_emp_earn(&emp_earn,ADD,c_mesg);
		if(err != NOERROR){
			printf(c_mesg);
			printf("ERROR in Saving new EMPLOYEE EARNINGS HISTORY Records\n"); 
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

