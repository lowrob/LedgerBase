/*-----------------------------------------------------------------------
Source Name: c_emp.c
System     : Personel Payroll 
Module     : Database Maintenance

DESCRIPTION:

	Conversion file for employee master file for updating file after
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
	char	em_numb[13] ;		/* Employee number    */
	char	em_last_name[26] ;	/* Last name            */
	char	em_first_name[16] ;	/* First name           */
	char	em_mid_name[16] ;	/* Middle name          */
	char	em_add1[31];		/* Address Line 1 */
	char	em_add2[31];		/* Address Line 2 */
	char	em_add3[31];		/* Address Line 3 */
	char	em_add4[31];		/* Address Line 4 */
	char	em_pc[11];		/* postal code */
	char	em_sin[10];		/* sin         */
	long	em_date;		/* birth date         */
	char	em_sex[2];		/* sex */
	char	em_mar_st[2];		/* Marital Status */
	char	em_title[5];		/* Title          */
	char	em_maid_name[16] ;	/* Maiden name  */
	char	em_phone[11];		/* telephone number */
	char	em_religion[3];		/* Religion code	*/
	char	em_com[52];		/* Comment */
	char	em_status[4];		/* Status */
	char	em_pp_code[7];		/* Pay Period Code	*/
	char	em_def_pf[2];		/* Deffered income Flag(P/F)  */
	double	em_def_inc;		/* Deffered income    */
	char	em_cpp_exp[2];		/* CPP exempt  */
	char	em_uic_exp[2];		/* UIC exempt  */
	char	em_tax_exp[2];		/* Tax exempt  */
	char	em_reg_pen[7];		/* Registered Pens Plan    */
	double	em_inc_tax;		/* Increased tax deduction */
	double	em_other_fed;		/* Other federal tax credit */
	double	em_union_dues;		/* Union dues   */
	double	em_ho_ded;		/* Housing Deduction	*/
	double	em_net_tax_cr;		/* Net tax credit       */
	double	em_ann_ded;		/* Annual deduction     */
	double	em_fam_all;   		/* Family Allowance     */
	double	em_old_age;   		/* Old age security     */
	short	em_last_pp;		/* Last Pay Period	*/
	char	em_ben_cat[7];		/* Benefit category	*/
	char	em_ded_cat[7];		/* Deduction category	*/
	char	em_barg[7];		/* Bargaining unit code    */
	char	em_pos[7];		/* Position code           */
	double	em_perc;		/* Percentage           */
	char	em_cert[6];		/* Certificate    */
	short	em_level;		/* Level		*/
	long	em_st_dt_ft;		/* Start date Ft      */
	long	em_st_dt_pt;		/* Start date Pt      */
	long	em_st_dt_ca;		/* Start date Ca      */
	long	em_st_dt_su;		/* Start date Su      */
	long	em_cont_dt;		/* Continuous date    */
	long	em_app_dt;		/* Appointment date   */
	short	em_ann;			/* Anniversary        */
	char	em_lang[2];		/* Language Preffered  */
	long	em_last_roe;		/* Last Record of employment   */
	short	em_num_ins_wk;		/* Number of insurable weeks in */
	char	em_un_tel[11];		/* Unlisted Telephone number   */
	char	em_ins[2];		/* Insurance class     */
	char	em_pre_paid[2];		/* Pre-paid		*/
	long	em_term_dt;		/* Termination date   */
	char	em_term[7];		/* Termination Code        */
	double	em_vac_rate;		/* Vacation Pay rate       */
	double	em_uic_rate;		/* UIC rate		*/
	char	em_dir_dep[2];		/* Direct Deposit      */
	char	em_bank[13] ;		/* Bank number    */
	short	em_bank_acct ;		/* Bank account       */
	short	em_cc ;			/* Cost center number */
	char	em_chq_add1[31];	/* Cheque address line 1 */
	char	em_chq_add2[31];	/* Cheque address line 2 */
	char	em_chq_add3[31];	/* Cheque address line 3 */
	char	em_chq_add4[31];	/* Cheque address line 4 */
	char	em_chq_pc[8];		/* postal code */
	char	em_cont[31];		/* contact person */
	char	em_pre_lev[3];		/* Preffered Teaching level   */
	double	em_sic_ent;		/* Sick days entitled 		*/
	double	em_sic_bk;		/* Sick bank		*/
	double	em_vac_bk;		/* Vacation bank    	*/
	double	em_vac_ent;		/* Vacation entitled 	*/
	long	em_exp;			/* Days/years experience	*/
	long	em_yrs_out;		/* Years oustside of prov/state	*/
	long	em_days_exp;		/* Days experience		*/
	long	em_yrs_exp;		/* Years experience	 	*/
	long	em_ini_casf;		/* Initial casual flag		*/
	long	em_ini_casu;		/* Initial casual units		*/	
	long	em_per_yrs;		/* Initial permanent years	*/
	long	em_per_days;		/* Initial permanent days	*/
	double	em_mth_vac;
	double	em_mth_sic;
	double	em_bal_sic;
	double	em_bal_vac;
	double	em_reg_prior;
	double	em_reg_opt;
	double	em_reg_nonm;
	} Old_employee;

static	Old_employee old_employee;
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

	strcpy(filenm,"employee");

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
	iostat = isstart(is_retval,(char *)&old_employee,0,ISFIRST);
	if(iostat < 0) {
	  printf("Error starting old employee file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(is_retval);
	  exit(-1);
	}
	for( ; ; ) {
		iostat = isreads(is_retval,(char *)&old_employee,0);
		if(iostat < 0) {
			if(iostat == EFL) break;
		  	printf("Error reading old employee file. Iserror: %d\n"
					,iserror);
			break;
		}
		scpy((char *)&emp_rec,(char *)&old_employee,sizeof(old_employee));	

		emp_rec.em_term[0] = '\0';
		emp_rec.em_cc = old_employee.em_cc;
		for(i=0;i<11;i++){
			emp_rec.em_sck_acc[i] = 0;
			emp_rec.em_vac_acc[i] = 0;
		}
		emp_rec.em_pref_cc = old_employee.em_cc;
		for(i=0;i<11;i++){
			emp_rec.em_comm[i][0] = '\0';
		}
		for(i=0;i<5;i++){
			emp_rec.em_inst[i][0] = '\0';
			emp_rec.em_prog[i][0] = '\0';
		}
		emp_rec.em_sen_perc = old_employee.em_perc;
		emp_rec.em_yrs_out_dist = 0;	
		emp_rec.em_yrs_out_prov = 0;
		emp_rec.em_days_out_dist = 0;
		emp_rec.em_days_out_prov = 0;
		emp_rec.em_cas_hrs = 0;	
		emp_rec.em_cas_days = 0;
		emp_rec.em_perm_days = 0;
		emp_rec.em_cas_tot_days = 0;
		emp_rec.em_cas_tot_yrs = 0;
		emp_rec.em_per_tot_yrs = 0;
		emp_rec.em_per_tot_days = 0;
		emp_rec.em_cntrct_status[0] = '\0';
		emp_rec.em_emerg_cntct[0] = '\0';
		emp_rec.em_emerg_tel[0] = '\0';
		emp_rec.em_no_depends = 0;	
		strcpy(emp_rec.em_class,emp_rec.em_pos);

		err = put_employee(&emp_rec,ADD,c_mesg);
		if(err != NOERROR){
			printf(c_mesg);
			printf("ERROR in Saving new EMPLOYEE  Records\n"); 
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

