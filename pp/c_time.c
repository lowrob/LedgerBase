/*-----------------------------------------------------------------------
Source Name: c_time.c
System     : Personel Payroll 
Module     : Database Maintenance

DESCRIPTION:

	Conversion file for time file for updating file after
	the additions of fields.

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
------------------------------------------------------------------------*/
#define  MAIN
#define  MAINFL		EMPLOYEE   	/* main file used */


#include <stdio.h>
#include <isnames.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_pp.h>


#define	SYSTEM		"DATABASE MAINTENANCE"	/*Sub System Name */
#define	MOD_DATE	"07-JAN-92"		/* Program Last Modified */

typedef struct {

	char	tm_numb[13] ;		/* Employee number    */
	long	tm_date;		/* Pay date 		*/
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
	char	tm_stat[4];		/* Status		*/

	} Old_time;

static	Old_time old_time;
static  Time   time_rec;   
static	short temp_no;

static	char filenm[50];
static	char outfile[50];
static	char tempfile[50];
static	char c_mesg[50];

main(argc,argv)
int argc;
char *argv[];
{
	int retval;
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
	  printf("Error opening old employee file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	iostat = isstart(is_retval,(char *)&old_time,0,ISFIRST);
	if(iostat < 0) {
	  printf("Error starting old employee file. Iserror: %d\n",iserror);
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

		temp_no = 1;
		strcpy(time_rec.tm_numb,old_time.tm_numb);
		time_rec.tm_date = old_time.tm_date;
		time_rec.tm_no = 999;
		flg_reset(TIME);
		for(;;) {
			retval = get_n_ptime(&time_rec,BROWSE,0,BACKWARD,c_mesg);
			if(retval != NOERROR) {
				if(retval == EFL) {
					strcpy(time_rec.tm_numb,old_time.tm_numb);
					time_rec.tm_date = old_time.tm_date;
					time_rec.tm_no = temp_no;
					break;
				}
				else {
					printf(c_mesg);
					get();
					roll_back(c_mesg);
					exit(-1);
				}
			}
			if(strcmp(time_rec.tm_numb,old_time.tm_numb) == 0 &&
			   time_rec.tm_date == old_time.tm_date) {
				temp_no++;
				continue;
			}
			else {
				strcpy(time_rec.tm_numb,old_time.tm_numb);
				time_rec.tm_date = old_time.tm_date;
				time_rec.tm_no = temp_no;
				break;
			}
		}

		time_rec.tm_pp = old_time.tm_pp;
		time_rec.tm_week = old_time.tm_week;
		time_rec.tm_fund = old_time.tm_fund;
		time_rec.tm_adj[0] = '\0';
		strcpy(time_rec.tm_class,old_time.tm_class);
		strcpy(time_rec.tm_earn,old_time.tm_earn);
		time_rec.tm_trans[0] = '\0';
		time_rec.tm_exp[0] = '\0';
		time_rec.tm_units[0] = old_time.tm_units[0];
		time_rec.tm_units[1] = old_time.tm_units[1];
		time_rec.tm_units[2] = old_time.tm_units[2];
		time_rec.tm_units[3] = old_time.tm_units[3];
		time_rec.tm_units[4] = old_time.tm_units[4];
		time_rec.tm_units[5] = old_time.tm_units[5];
		time_rec.tm_units[6] = old_time.tm_units[6];
		time_rec.tm_att[0][0] = '\0';
		time_rec.tm_att[0][1] = '\0';
		time_rec.tm_att[0][2] = '\0';
		time_rec.tm_att[0][3] = '\0';
		time_rec.tm_att[0][4] = '\0';
		time_rec.tm_att[0][5] = '\0';
		time_rec.tm_att[0][6] = '\0';
		time_rec.tm_tot_amt = old_time.tm_tot_amt;
		time_rec.tm_cost = old_time.tm_cost;
		time_rec.tm_teach[0] = '\0';
		time_rec.tm_comment[0] = '\0';
		strcpy(time_rec.tm_stat,"ACT");

		err = put_ptime(&time_rec,ADD,c_mesg);
		if(err != NOERROR){
			printf(c_mesg);
			printf("ERROR in Saving new TIME  Records\n"); 
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

