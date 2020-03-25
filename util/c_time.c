/*-----------------------------------------------------------------------
Source Name: c_time.c
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
	char	tm_numb[13] ;		/* Employee number    */
	long	tm_date;		/* Pay date 		*/
	short	tm_no;			/* Sequence number	*/
	short	tm_pp;			/* Pay period		*/
	short	tm_week;		/* Week			*/
	short	tm_fund;		/* Fund */
	char	tm_adj[2];		/* Adjustment flag	*/
	char	tm_class[7];		/* Classification code	*/
	char	tm_earn[7];		/* Earnings code	*/
	char	tm_trans[7];		/* Transaction code	*/
	char	tm_exp[7];		/* Expense code		*/
	double	tm_units[7];		/* Units/day		*/
	char	tm_att[7][4];		/* Attendance Code 	*/
	double	tm_tot_amt;		/* Total amount		*/
	short	tm_cost;		/* Cost center number	*/
	char	tm_teach[13];		/* Teacher Code 	*/
	char	tm_comment[46];		/* Comment	*/
	char	tm_stat[4];		/* Status		*/
	short	tm_year;
	} Old_time;

static	Old_time old_time;
static  Time   time;   

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

	strcpy(filenm,"time");

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
	  printf("Error opening old time file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	iostat = isstart(is_retval,(char *)&old_time,0,ISFIRST);
	if(iostat < 0) {
	  printf("Error starting time file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	for( ; ; ) {
		iostat = isreads(is_retval,(char *)&old_time,0);
		if(iostat < 0) {
			if(iostat == EFL) break;
	  	printf("Error reading old time file. Iserror: %d\n"
					,iserror);
			break;
		}
		strcpy(time.tm_numb, old_time.tm_numb);	/* Employee number    */
		time.tm_date = old_time.tm_date;
		time.tm_no = old_time.tm_no;
		time.tm_pp = old_time.tm_pp;
		time.tm_week = old_time.tm_week;
		time.tm_fund = old_time.tm_fund;
		strcpy(time.tm_adj, old_time.tm_adj);
		strcpy(time.tm_class, old_time.tm_class);
		strcpy(time.tm_earn, old_time.tm_earn);	
		strcpy(time.tm_trans, old_time.tm_trans);
		strcpy(time.tm_exp, old_time.tm_exp);
		for(i=0;i<7;i++){
			time.tm_units[i] = old_time.tm_units[i];
			strcpy(time.tm_att[i], old_time.tm_att[i]);
		}
		time.tm_tot_amt = old_time.tm_tot_amt;
		time.tm_cost = old_time.tm_cost;
		strcpy(time.tm_teach, old_time.tm_teach);
		strcpy(time.tm_comment, old_time.tm_comment);
		strcpy(time.tm_stat, old_time.tm_stat);
		time.tm_year = old_time.tm_year;
		time.tm_num_days = 0;

		err = put_ptime(&time,ADD,c_mesg);
		if(err != NOERROR){
			printf(c_mesg);
			printf("ERROR in Saving Time Records\n"); 
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

