/*
*    Source Name : bfs_pp.h
*    System      : Payroll/Personnel System.
*
*    Created On  : August 12, 1991.
*    By		 : Eugene Roy
*
*    Contains Structure/Record Definitions used in this system.
*/

#define	NO_KEYS	12	/* number of user entered keys */

/*
*	Department File.  Stores the department codes and descriptions.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: d_code
*/

typedef struct	{

	char	d_code[7] ; 		/* Department Code */
	char	d_desc[31] ;		/* Department description */

}	Dept ;

/*
*	Area File.  Stores the area codes and descriptions.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: a_deptcode + a_code
*/

typedef struct	{

	char	a_deptcode[7] ; 	/* Department Code */
	char	a_code[7] ;		/* Area Code */
	char	a_desc[31] ;		/* Department description */

}	Area ;

/*
*	Position File.  Maintains the different classification codes.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: p_code
*/

typedef struct	{

	char	p_code[7] ; 		/* Position Code */
	char	p_desc[31] ;		/* Psition Description */
	char	p_type[3] ;		/* Position type */
	char	p_ctr_type[3] ;		/* Position contract type */
	long	p_ctr_lgth;		/* Position contract length */

}	Position ;


/*
*	Classification File.  Maintains the different classification codes.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: c_code + c_date
*/

typedef struct	{

	char	c_code[7] ; 		/* Classification Code */
	long	c_date;			/* Classification table eff date */
	char	c_desc[31] ;		/* Classification Description */
	double	c_yrly_inc;		/* Classification yearly income */
	double	c_units;		/* Classification units/year */
	char	c_pos[7] ; 		/* Classification position code */

}	Class ;

/*
*	Classification Item File.  Maintains the different classification items
*	codes.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: ci_code + ci_date + ci_fund
*/

typedef struct	{

	char	ci_code[7] ; 		/* Classification Code */
	long	ci_date;		/* Classification table eff date */
	short	ci_fund;		/* Classification Fund */
	char	ci_cpp_acct[19] ;	/* Classification CPP Account */
	long	ci_cpp_keys[NO_KEYS] ; 	/* Classification CPP Acct Keys */
	char	ci_uic_acct[19] ;	/* Classification UIC Account */
	long	ci_uic_keys[NO_KEYS] ; 	/* Classification UIC Acct Keys */

}	Class_item ;

/*
*	Bargaining Unit File.  Maintains the bargaining unit information.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: b_code + b_date
*/

typedef struct	{

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
	short	b_dept_cd;
	double	b_stat_thrs;		/* Maximum number of sick days */
	double	b_stat_hpd;		/* Maximum number of sick days */

}	Barg_unit ;

/*
*	Structure/record definition of Bank file
*
*	File Type: ISAM. No of Isam keys are : 1.
*
*	Main key : bk_numb
*/

typedef struct {

	char	bk_numb[13] ;		/* Bank number PIC 99 */
	char	bk_name[31] ;		/* Bank name PIC X(28)  */
	char	bk_add1[31];		/* Address Line 1 */
	char	bk_add2[31];		/* Address Line 2 */
	char	bk_add3[31];		/* Address Line 3 */
	char	bk_add4[31];		/* Address Line 4 */
	char	bk_pc[8];		/* postal code */
	char	bk_cont[31];		/* contact person */
	char	bk_phone[11];		/* telephone number */

}	Bank ;

/*
*	Pay Period File.  Stores the Pay Period codes and descriptions.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: pp_code + pp_numb 
*/

typedef struct	{

	char	pp_code[7] ;	 	/* Pay Period Code */
	short	pp_year;
	short	pp_numb ;		/* Pay Period Number */
	char	pp_desc[31] ;		/* Pay Period Decription */

}	Pay_per ;

/*
*	Pay Period Item File.  Stores the Pay Period information.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: ppi_code + ppi_numb + ppi_st_date  
*/

typedef struct	{

	char	ppi_code[7] ;	 	/* Pay Period Code */
	short	ppi_year;
	short	ppi_numb ;		/* Pay Period Number */
	long	ppi_st_date;		/* Pay Period start date */
	long	ppi_end_date;		/* Pay Period end date */
	short	ppi_mthly;		/* Monthly Pay Period */	

}	Pay_per_it ;

/*
*	UIC Table File.  Maintains the UIC Table.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: ui_numb + ui_date
*/

typedef struct	{

	short	ui_numb ;		/* UIC Number of Pay Periods.	*/
	long	ui_date;		/* UIC table effective date */
	short	ui_min_hrs;		/* UIC Minimum hours */
	double	ui_min_earn;		/* UIC mimimum insurable earnings */
	double	ui_max_earn;		/* UIC maximum insurable earnings */
	double	ui_max_prem;		/* UIC maximum premium */
	double	ui_yrly_prem;		/* UIC Yearly maximum Premium */
	double	ui_yrly_earn;		/* UIC Yealry maximum Earnings */

}	Uic ;

/*
*	Tax Table File.  Maintains the Tax Table.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: tx_low_amnt + tx_date
*/

typedef struct	{

	long	tx_date;		/* Tax date */
	double	tx_low_amnt ;		/* Tax Low Amount.  */
	double	tx_high_amnt;		/* Tax High Amount. */
	double	tx_rate;		/* Tax rate */
	double	tx_fed_const;		/* Tax constant */

}	Tax ;

/*
*	Certification Table File.  Maintains the Certification Table.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: cr_low_amnt + cr_date
*/

typedef struct	{

	char	cr_code[7];		/* Certification code  */
	long	cr_date;		/* Certification date */
	short	cr_level ;		/* Certification Level.  */
	double	cr_income;		/* Certification Yearly income.  */

}	Cert ;

/*
*	Earnings Code File.  Maintains the Earnings codes.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: ea_code + ea_date
*/

typedef struct	{

	char	ea_code[7];		/* Earnings code  */
	long	ea_date;		/* Earnings date */
	char	ea_desc[31];		/* Earnings Description  */
	char	ea_type[2];		/* Earnings type; fixed amount or 
					   percentage (F/P)     */
	double	ea_amount;		/* Earnings amount.  */
	char	ea_reg_hgh[2];		/* Earnings Regular/High (R/H)   */
	char	ea_lump_sum[2];		/* Earnings Lump Sum or Hourly */

}	Earn ;

/*
*	Transaction Code File.  Maintains the Transaction codes.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: tr_code
*/

typedef struct	{

	char	tr_code[7];		/* Transaction code  */
	char	tr_desc[31];		/* Transaction Description  */

}	Trans ;

/*
*	Transaction Code Item File.  Maintains the Transaction code
*	associations.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: tri_code + tri_class + tri_earn
*/

typedef struct	{

	char	tri_code[7];		/* Transaction code  */
	char	tri_class[7];		/* Classification code */
	char	tri_earn[7];		/* Earnings code  */

}	Trans_item ;

/*
*	Expense Code File.  Maintains the Expense codes.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: ex_code
*/

typedef struct	{

	char	ex_code[7];		/* Expense code  */
	char	ex_desc[31];		/* Expense Description  */

}	Exp ;

/*
*	Expense Code Item File.  Maintains the Expense code
*	associations.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: ex_code + ex_class + ex_earn
*/

typedef struct	{

	char	exi_code[7];		/* Expense code  */
	char	exi_class[7];		/* Classification code */
	char	exi_earn[7];		/* Earnings code  */
	short	exi_fund;		/* Fund #         */
	char	exi_acct[19];		/* G/L account    */

}	Exp_item ;

/*
*	Termination Code File.  Stores the Termination codes and descriptions.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: t_code
*/

typedef struct	{

	char	t_code[3] ; 		/* Termination Code */
	char	t_desc[31] ;		/* Termination description */

}	Term ;


/*
*	Statutory Holiday File.  Stores the Statutory Holidays and descriptions.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: s_code
*/

typedef struct	{

	char	s_code[7] ; 		/* Statutory Holiday bargaining unit 
					   code*/
	long	s_date;			/* statutory Holiday date  */
	char	s_desc[31] ;		/* Statutory Holiday description */

}	Stat ;

/*
*	Attendance Code File.  Stores the attendance codes and descriptions.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: at_code
*/

typedef struct	{

	char	at_disp_code[2] ; 	/* Attendance display code.   */
	char	at_code[4] ; 		/* Attendance code.   */
	char	at_desc[31] ;		/* Attendance description */
	char	at_sen[2];		/* Attendance seniority flag  */
	char	at_sick[2];		/* Attendance sick flag  */
	char	at_earn[2];		/* Attendance earnings flag  */
	char	at_vac[2];		/* Attendance vacation flag  */
	char	at_sckbank[2];		/* Attendance sick bank flag  */
	char	at_vacbank[2];		/* Attendance vac. bank flag  */
	char	at_sic_acc[2];		/* Attendance sick accrual flag */
	char	at_vac_acc[2];		/* Attendance vac accrual flag */

}	Att ;

/*
*	Inactivation Code File.  Stores the Inactivation codes and descriptions.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: i_code
*/

typedef struct	{

	char	i_code[4] ; 		/* Inactivation Code */
	char	i_desc[31] ;		/* Inactivation description */
	char	i_at_code[4] ; 		/* Attendance code.   */

}	Inact ;

/*
*	Area of Specialization Code File.  Stores the Area of Specialization
*	codes and descriptions.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: ar_code
*/

typedef struct	{

	char	ar_code[7] ; 		/* Area of Specialization Code */
	char	ar_desc[31] ;		/* Area of Specialization description */

}	Area_spec ;

/*
*	CSB/Loan Code File.  Maintains the CSB/Loan codes.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: cs_code
*/

typedef struct	{

	char	cs_code[7];		/* CSB/Loan code  */
	char	cs_desc[31];		/* CSB/Loans code  */
	short	cs_fund;		/* Fund #         */
	char	cs_amt_acct[19];	/* Amount G/L account    */
	char	cs_int_acct[19];	/* Interest G/L account    */

}	Csb_loan ;

/*
*	Parameter File.  Maintains the personnel/payroll parameters.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: 
*/

typedef struct	{

	char	pr_tp_ft_code[7];	/* Teacher Position Code Ft   */ 
	char	pr_tp_pt_code[7];	/* Teacher Position Code Pt   */ 
	double	pr_tot_units;		/* Total Units for pay period	*/
	char	pr_sb_code[7];		/* Substitute Teacher Code */ 
	long	pr_retro;		/* Retropay effective after    */
	char	pr_up_gl[2];		/* Update G/L                */
	short	pr_dept;		/* Department Key */
	short	pr_area;		/* Area Key       */
	short	pr_cost;		/* Cost Center Key*/
	char	pr_langu[2];		/* Language Preffered        */
	char	pr_prov[3];		/* Province        */
	char	pr_att_pay[2];		/* Attendance entry during payroll  */
	char	pr_bk_dep[2];
	char	pr_teach_pay[2];	/* Teacher # entry during payroll   */
	char	pr_area_def[2];		/* Area defined              */
	char	pr_reg_earn[7];		/* Regular Earnings Code	*/
	char	pr_vac_earn[7];		/* Vacation Earnings Code	*/
	short	pr_st_mth;
	long	pr_fisc_st_dt;		/* Fiscal year start date  */
	long	pr_fisc_end_dt;		/* Fiscal year end date  */
	long	pr_cal_st_dt;		/* Calendar year start date  */
	long	pr_cal_end_dt;		/* Calendar year end date  */
	long	pr_schl_st_dt;		/* School year start date  */
	long	pr_schl_end_dt;		/* School year end date  */
	long	pr_last_chq;		/* Last cheque number    */
	long	pr_last_ec;     	/* Last ec number  */
	char	pr_cpp_exp[12][2];	/* Y/N/A for key */
	char	pr_uic_exp[12][2];	/* Y/N/A for key */
	char	pr_salary[12][2];	/* Y/N/A for key */
	char	pr_deduct[12][2];	/* Y/N/A for key */
	char	pr_reg_pen[12][2];	/* Y/N/A for key */
	char	pr_teacher[12][2];	/* Y/N/A for key */
	double	pr_ben_acc;		/* Benefit Accrual maximum */
	double	pr_low_earn;	  	/* Low excluded earnings   */
	double	pr_high_earn;	  	/* High excluded earnings   */
	short	pr_pen_mult;    	/* Pension Adjustment Multiplier  */
	double	pr_pen_const;		/* Pension Adjustment Constant    */
	long	pr_tel_ext;		/* Telephone ext. for Payroll	*/
	char	pr_retro_cd[7];
	short	pr_fund;
	char	pr_teach_gl[19];
	long	pr_keys[12];
	char	pr_trans_cd[7];
	long	pr_10mth_st_dt;
	long	pr_10mth_end_dt;
	char	pr_exp_cd[7];
	char	pr_payper[7];
	long	pr_week_date;		/* Last Complete week of period */
	long	pr_bi_date;		/* Last Complete week of period */
}	Pay_param ;

/*
*	Salary G/L Account File.  Maintains the Salary G/L account keys.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: sa_fund + sa_class + sa_earn
*/

typedef struct	{

	short	sa_fund;		/* Fund #         */
	char	sa_class[7];		/* Classification code */
	char	sa_earn[7];		/* Earnings code  */
	long	sa_keys[12];		/* G/L account Keys   */

}	Salary ;

/*
*	Benefit File.  Maintains the Benefit information.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: bn_code + bn_pp_code
*/

typedef struct	{

	char	bn_code[7] ; 		/* Benefit Code */
	char	bn_pp_code[7] ;		/* Benefit Pay Period Code */
	char	bn_desc[31] ;		/* Benefit description */
	char	bn_inc[2] ;		/* Employee income  */	
	char	bn_t4_fld[2] ;		/* T4 field code     */	
	char	bn_amt_flag[2] ;	/* Amount flag: fixed or percentage */
	double	bn_amount;  		/* Amount   */
	char	bn_add_pp[5][2];	/* Add monthly pay period    */
	short	bn_fund;		/* Fund 		 */
	char	bn_acct[19] ;		/* G/L Account */

}	Benefit ;

/*
*	Benefit Category File.  Maintains the Benefit category information.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: bc_cat_code + bc_code
*/

typedef struct	{

	char	bc_cat_code[7] ; 	/* Benefit category Code */
	char	bc_code[7] ; 		/* Benefit Code */
	char	bc_pp_code[7] ;		/* Pay Period Code */

}	Ben_cat ;

/*
*	Deduction File.  Maintains the Deduction information.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: dd_code + dd_pp_code
*/

typedef struct	{

	char	dd_code[7] ; 		/* Deduction Code */
	char	dd_pp_code[7] ;		/* Deduction Pay Period Code */
	char	dd_desc[31] ;		/* Deduction description */
	char	dd_second[3] ;		/* Second Code */
	char	dd_t4_fld[2] ;		/* T4 field code     */	
	double	dd_min_earn;		/* Minimum Earnings               */
	double	dd_max_contr;		/* Maximum yearly contribution     */
	char	dd_ded_pp[5][2];	/* Add to monthly pay period        */
	short	dd_fund;		/* Fund */
	char	dd_lia_acct[19] ;	/* G/L Account */
	long	dd_exp_acct[NO_KEYS] ;	/* G/L Account */

}	Deduction ;

/*
*	Deduction group File.  Maintains the Deduction group information.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: dg_code + dg_pp_code + dg_group 
*/

typedef struct	{

	char	dg_code[7] ; 		/* Deduction Code */
	char	dg_pp_code[7] ;		/* Deduction Pay Period Code */
	char	dg_group[7];		/* Deduction Group Code   */
	char	dg_desc[16] ;		/* Deduction description */
	char	dg_amt_flag[2] ;	/* Amount flag: fixed or percentage */
	double	dg_amount;  		/* Amount   */
	double	dg_employer_sh;		/* Employer share    */

}	Ded_group ;

/*
*	Deduction Category File.  Maintains the Deduction category information.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: dc_cat_code + dc_code
*/

typedef struct	{

	char	dc_cat_code[7] ; 	/* Deduction category Code */
	char	dc_code[7] ; 		/* Deduction Code */
	char	dc_pp_code[7];		/* Deduction Pay Period Code */

}	Ded_cat ;

/*
*	Registered Pension Plan File.  Maintains the Registered Pension Plan information.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: rg_code + rg_pp_code
*/

typedef struct	{

	char	rg_code[7] ; 		/* Reg Pension Plan Code */
	char	rg_pp_code[7] ;		/* Reg Pension Plan Pay Period Code */
	char	rg_desc[31] ;		/* Reg Pension Plan description */
	char	rg_t4_fld[2] ;		/* T4 field code     */	
	short	rg_pac;			/* Pension Adj Calc Number	*/
	char	rg_dd_code[3] ; 	/* Second Deduction Code */
	char	rg_reg_num[13];  		/* Registration number */
	char	rg_type[3] ; 		/* Type		 */
	double	rg_perc1;  		/* Percentage 1   */ 
	double	rg_perc2;  		/* Percentage 2   */
	double	rg_perc3;  		/* Percentage 3   */ 
	double	rg_amount;		/* Amount		 */
	double	rg_amount2;		/* Amount		 */
	double	rg_employer_sh;		/* Employer Share */
	double	rg_min_earn;		/* Minimum earnings   */
	double	rg_max_contr;		/* Maximum contribution */
	char	rg_ded_pp[5][2];	/* Deduct monthly pay period        */
	short	rg_fund;		/* Fund */
	char	rg_lia_acct[19] ;	/* G/L Account */
	long	rg_exp_acct[12] ;	/* G/L Account */

}	Reg_pen ;

/*
*	Structure/record definition of the Employee file
*
*	File Type: ISAM. No of Isam keys are : 1.
*
*	Main key : em_numb
*/

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
	char	em_term[3];		/* Termination Code        */
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
	double	em_sck_acc[12];		/* the sick accrual for each month */
	double	em_vac_acc[12];		/* the vac accrual for each month */
	short	em_pref_cc;		/* Cost Center preference */
	char	em_comm[12][28];	/* Comments */
	char	em_inst[5][16];		/* Institution */
	char	em_prog[5][16];		/* Program */
	double	em_sen_perc;		/* Employee's seniority percent */
	short	em_yrs_out_dist;	/* sen years out of district */
	short	em_yrs_out_prov;	/* sen years out of province */
	double	em_days_out_dist;	/* sen days out of district */
	double	em_days_out_prov;	/* sen days out of province */
	double	em_cas_hrs;	/* sen casual hours */
	double	em_cas_days;	/* sen casual days */
	double	em_perm_days;	/* sen perm days */
	double	em_cas_tot_days;	/* sen casual total days */
	short	em_cas_tot_yrs;		/* sen cas total yrs */
	short	em_per_tot_yrs;		/* sen perm total yrs */
	double	em_per_tot_days;		/* sen perm total days */
	char	em_cntrct_stat[2];	/* contract status */
	char	em_emerg_cntct[31];	/* emergency contact name */
	char	em_emerg_tel[11];	/* emergency contact tel */
	short	em_no_depends;		/* nbr of dependants */
	char	em_class[7];	/* class code */

}	Emp ;

/*
*	Structure/record definition of the Employee employment file
*	for New Brunswick (additional)
*
*	File Type: ISAM. No of Isam keys are : 1.
*
*	Main key : ep_numb
*/

typedef struct {
	char	ep_numb[13] ;		/* Employee number    */
	char	ep_class[7];		/* Classification code	*/
	char	ep_dept[7];		/* Department code	*/
	char	ep_area[7];		/* Area code	*/
	char	ep_trans[7];		/* Transaction code	*/
	char	ep_exp[7];		/* Expense code	*/

}	Emp_emp ;

/*
*	Structure/record definition of the Employee schedule1 file
*
*	File Type: ISAM. No of Isam keys are : 1.
*
*	Main key : es_numb
*/

typedef struct {
	char	es_numb[13] ;		/* Employee number      */
	short	es_week;		/* Week			*/
	short	es_fund;		/* Fund			*/
	char	es_class[7];		/* Classification code	*/
	short	es_cost;		/* Cost center number	*/
	double	es_units[7];		/* Units/day		*/
	double	es_amount;		/* Amount		*/
	char	es_dept[7];		/* Department code	*/
	char	es_area[7];		/* Area code		*/

}	Emp_sched1 ;

/*
*	Structure/record definition of the Employee schedule2 file
*	for New Brunswick (additional)
*
*	File Type: ISAM. No of Isam keys are : 1.
*
*	Main key : es_numb
*/

typedef struct {
	char	es2_numb[13] ;		/* Employee number    */
	short	es2_week;		/* Week			*/
	double	es2_units[7];		/* Units/day		*/

}	Emp_sched2 ;

/*
*	Structure/record definition of the Employee schedule extra file
*	for New Brunswick (additional)
*
*	File Type: ISAM. No of Isam keys are : 1.
*
*	Main key : ee_numb
*/

typedef struct {
	char	ee_numb[13] ;		/* Employee number    */
	char	ee_type[2];		/* Type (earnings/deduction)	*/
	char	ee_class[7];		/* Classification code	*/
	char	ee_earn[7];		/* Earnings code	*/
	char	ee_amt_flg[2];		/* Fixed/Percentage amount flag */
	double	ee_amount;		/* Amount		*/
	double	ee_target;		/* Target		*/
	short	ee_cc;			/* Cost Center Number	*/

}	Emp_extra ;

/*
*	Structure/record definition of the Employee earnings file
*
*	File Type: ISAM. No of Isam keys are : 1.
*
*	Main key : en_numb + en_pp + en_date
*/

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
	short	en_year;
	char	en_reg_pen[7];

}	Emp_earn ;

/*
*	Structure/record definition of the Employee insurable earnings file
*
*	File Type: ISAM. No of Isam keys are : 1.
*
*	Main key : in_numb + in_pp + in_date
*/

typedef struct {
	char	in_numb[13] ;		/* Employee number    */
	short	in_pp;			/* Pay period		*/
	long	in_date;		/* Date of the transaction	*/
	double	in_uic_ins;		/* UIC insurable earnings	*/
	short	in_num_ins_wk;		/* Number of insurable weeks in
					   pay period	*/

}	Emp_ins ;

/*
*	Structure/record definition of the Employee benefit file
*
*	File Type: ISAM. No of Isam keys are : 1.
*
*	Main key : eb_numb + eb_code
*/

typedef struct {

	char	eb_numb[13] ;		/* Employee number    */
	char	eb_code[7];		/* Benefit code		*/
	char	eb_pp_code[7];		/* Pay Period code		*/
	double	eb_amount;		/* Amount for the pay period	*/

}	Emp_ben ;

/*
*	Structure/record definition of the Employee benefit history file
*
*	File Type: ISAM. No of Isam keys are : 1.
*
*	Main key : ebh_numb + ebh_code + ebh_pp + ebh_date
*/

typedef struct {

	char	ebh_numb[13] ;		/* Employee number    */
	short	ebh_pp;			/* Pay period		*/
	long	ebh_date;		/* Date of the transaction	*/
	char	ebh_code[7];		/* Benefit code		*/
	double	ebh_amount;		/* Amount for the pay period	*/

}	Emp_bh ;

/*
*	Structure/record definition of the Employee deduction file
*
*	File Type: ISAM. No of Isam keys are : 1.
*
*	Main key : ed_numb + ed_code
*/

typedef struct {

	char	ed_numb[13] ;		/* Employee number    */
	char	ed_code[7];		/* Deduction code		*/
	char	ed_group[7];		/* Deduction group code		*/
	double	ed_amount;		/* Amount for the pay period	*/

}	Emp_ded ;

/*
*	Structure/record definition of the Employee deduction history file
*
*	File Type: ISAM. No of Isam keys are : 1.
*
*	Main key : edh_numb + edh_code + edh_pp + edh_date
*/

typedef struct {

	char	edh_numb[13] ;		/* Employee number    */
	short	edh_pp;			/* Pay period		*/
	long	edh_date;		/* Date of the transaction	*/
	char	edh_code[7];		/* Benefit code		*/
	char	edh_group[7];
	double	edh_amount;		/* Amount for the pay period	*/

}	Emp_dh ;

/*
*	CSB/Loan File.  Maintains the employee CSB/Loan information.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: el_numb + el_code + el_seq
*/

typedef struct	{

	char	el_numb[13] ;		/* Employee number    */
	char	el_code[7] ; 		/* CSB/Loan Code */
	short	el_seq ;		/* Sequence number    */
	double	el_amount;		/* Amount of loan (target)	*/
	double	el_amnt_out;		/* Amount of loan Outstanding   */
	short	el_pp_num ;		/* Number of pay periods	*/
	short	el_pp_num_el ;		/* Number of pay periods ellapsed*/
	char	el_amt_flg[2];		/* Fixed/Percentage amount flag	*/
	double	el_pp_amount;		/* Fixed/Percentage Amount	*/
	double	el_int;			/* Interest rate	*/
	char	el_ded_pp[5][2];	/* Deduct monthly pay period        */

}	Emp_loan ;

/*
*	CSB/Loan History File.  Maintains the employee CSB/Loan history.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: elh_numb + elh_code + elh_seq
*/

typedef struct	{

	char	elh_numb[13] ;		/* Employee number    */
	char	elh_code[7] ; 		/* CSB/Loan Code */
	short	elh_seq ;		/* Sequence number    */
	short	elh_pp;			/* Pay period		*/
	long	elh_date;		/* Date of the transaction	*/
	double	elh_amount;		/* Fixed/Percentage Amount	*/
	double	elh_int_amt;		/* Interet amount		*/

}	Emp_ln_his ;

/*
*	Garnishment File.  Maintains the employee Garnishment information.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: eg_numb + eg_pr_cd + eg_seq
*/

typedef struct	{

	char	eg_numb[13] ;		/* Employee number    */
	short	eg_pr_cd ;		/* Priority code    */
	short	eg_seq ;		/* Sequence number    */
	char	eg_desc[31];		/* Description */
	double	eg_amount;		/* Amount of loan (target)	*/
	double	eg_amnt_out;		/* Amount of loan outstanding	*/
	char	eg_amt_flg[2];		/* Fixed/Percentage amount flag	*/
	double	eg_pp_amount;		/* Fixed/Percentage Amount	*/
	double	eg_min_tresh;		/* Minimum Treshold		*/
	char	eg_ded_pp[5][2];	/* Deduct monthly pay period        */
	short	eg_fund;		/* Fund */
	char	eg_lia_acct[19] ;	/* Liability G/L Account */

}	Emp_garn ;

/*
*	Garnishment History File.  Maintains the employee Garnishment History.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: egh_numb + egh_code + egh_seq
*/

typedef struct	{

	char	egh_numb[13] ;		/* Employee number    */
	short	egh_pr_cd;		/* Priority Code */
	short	egh_seq ;		/* Sequence number    */
	short	egh_pp;			/* Pay period		*/
	long	egh_date;		/* Date of the transaction	*/
	double	egh_amount;		/* Fixed/Percentage Amount	*/

}	Emp_gr_his ;

/*
*	Structure/record definition of the Teacher assignment file
*
*	File Type: ISAM. No of Isam keys are : 1.
*
*	Main key : tc_numb
*/

typedef struct {

	char	tc_numb[13] ;		/* Employee number    */
	short	tc_cost;		/* Cost center number	*/
	char	tc_ar_sp[7];		/* Area of specialization	*/
	short	tc_grade;		/* Grade		*/
	double	tc_perc;		/* Percentage		*/
	char	tc_crs[7];		/* Course code	*/
	short	tc_sem;			/* Semester code	*/
	short	tc_sec;			/* Section code		*/
	char	tc_room[5];		/* Room code		*/
	short	tc_load;		/* Load			*/

}	Teach_ass ;

/*
*	Structure/record definition of the Teacher qualification file
*
*	File Type: ISAM. No of Isam keys are : 1.
*
*	Main key : tq_numb
*/

typedef struct {

	char	tq_numb[13] ;		/* Employee number    */
	char	tq_code[7] ;		/* Qualification code */

}	Teach_qual ;

/*
*	Attendance History File.  Maintains the employee Attendance History.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: eah_numb + eah_code + eah_seq
*/

typedef struct	{

	char	eah_numb[13] ;		/* Employee number    */
	char	eah_code[4] ; 		/* Attendance code	 */
	long	eah_date;		/* Date of the transaction	*/
	char	eah_teach[13] ;		/* Teacher number    */
	double	eah_sen_hours ;		/* Employee sen hours */
	double 	eah_hours ;		/* Employee hours absent */
	char	eah_vacproc[2];	/*Y when deduct from vac bank is done */

}	Emp_at_his ;

/*
*	Time File.  Maintains the time inforamtion.
*
*	File Type: ISAM. No of Isam keys are : 1.
*
*	Main key : tm_numb
*/

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

}	Time ;

/*
*	Time History File.  Maintains the time history inforamtion.
*
*	File Type: ISAM. No of Isam keys are : 1.
*
*	Main key : tmh_numb
*/

typedef struct {

	char	tmh_numb[13] ;		/* Employee number	*/
	long	tmh_date;		/* Pay date 		*/
	short	tmh_no;			/* Sequence Number	*/
	short	tmh_pp;			/* Pay period		*/
	short	tmh_week;		/* Week			*/
	short	tmh_fund;		/* Fund 		*/
	char	tmh_adj[2];		/* Adjustment flag	*/
	char	tmh_class[7];		/* Classification code	*/
	char	tmh_earn[7];		/* Earnings code	*/
	char	tmh_trans[7];		/* Transaction code	*/
	char	tmh_exp[7];		/* Expense code		*/
	double	tmh_units[7];		/* Units/day		*/
	double	tmh_tot_amt;		/* Total amount		*/
	short	tmh_cost;		/* Cost center number	*/
	char	tmh_comment[46];	/* Comment		*/
	char	tmh_stat[4];		/* Status		*/
	short	tmh_year;

}	Time_his ;

/*
*	Pay Period Earnings File.  Maintains the pay period earnings
*	information.
*
*	File Type: ISAM. No of Isam keys are : 1.
*
*	Main key : pe_numb + pe_pp
*/

typedef struct {

	char	pe_numb[13] ;		/* Employee number    */
	short	pe_cc;			/* Cost Center	*/
	short	pe_pp;			/* Pay period		*/
	long	pe_date;		/* Date of the transaction	*/
	double	pe_reg_units;		/* Regular units      */
	double	pe_high_units;		/* High units         */
	double	pe_reg_inc1;		/* Reg income		*/
	double	pe_reg_inc2;		/* Reg income		*/
	double	pe_high_inc;		/* High income    */
	double	pe_ben;			/* Benefits	*/
	double	pe_def_inc;		/* Deffered income    */
	double	pe_cpp;			/* CPP contribution */
	double	pe_cpp_pen;		/* CPP pensionable earnings	*/
	double	pe_uic;			/* UIC premiums 	*/
	double	pe_uic_ins;		/* UIC insurable earnings	*/
	short	pe_num_ins_wk;		/* Number of insurable weeks in pp */
	double	pe_reg1;		/* Registered pen plan contr rate1 */
	double	pe_reg2;		/* Registered pen plan contr rate2 */
	double	pe_reg3;		/* Registered pen plan contr rate3 */
	double	pe_tax;			/* Income tax		*/
	double	pe_net;			/* Net Income 		*/
	double	pe_vac;			/* Vacation Pay		*/
	long	pe_chq_no;		/* Cheque No 		*/
	long	pe_cp_chq_no;		/* Cheque No for rerun	*/
	char	pe_chq_type[2];		/* Type M(anual) or R(egular) */
	char	pe_accno[19];		/* Account Number */
	double	pe_uic_employer;	/* Amount paid by employee */
	double	pe_reg_prior;
	double	pe_reg_opt;
	double	pe_reg_nonm;
	double	pe_week_inc[5];		/*  income		*/
	double	pe_week_hinc[5];	/*  income		*/
	double	pe_week_units[5];	/* Regular units      */
	double	pe_week_hunits[5];	/* High units         */
	double	pe_week_vac[5];
	double	pe_wk_def_inc[5];
	double	pe_week_cpp[5];
	double	pe_wk_cpp_pen[5];
	double	pe_week_uic[5];
	double	pe_week_reg1[5];
	double	pe_week_reg2[5];
	double	pe_week_reg3[5];
	double	pe_wk_reg_pr[5];
	double	pe_wk_reg_opt[5];
	double	pe_wk_reg_nonm[5];
	double	pe_week_tax[5];
	double	pe_week_net[5];

}	Pay_earn ;

/*
*	Pay Period Benefit File.  Maintains the pay period benefit
*	information.
*
*	File Type: ISAM. No of Isam keys are : 1.
*
*	Main key : pb_numb + pb_code + pb_pp + pb_date
*/

typedef struct {

	char	pb_numb[13] ;		/* Employee number    */
	short	pb_pp;			/* Pay period		*/
	long	pb_date;		/* Date of the transaction	*/
	short	pb_fund ;		/* Fund number    */
	char	pb_acct[19] ;		/* G/L Payroll Account	*/
	char	pb_code[7];		/* Benefit code		*/
	double	pb_amount;		/* Amount for the pay period	*/

}	Pp_ben ;

/*
*	Pay Period Deduction File.  Maintains the pay period deduction 
*	information.
*
*	File Type: ISAM. No of Isam keys are : 1.
*
*	Main key : pd_numb + pd_code + pd_pp + pd_date
*/

typedef struct {

	char	pd_numb[13] ;		/* Employee number    */
	short	pd_pp;			/* Pay period		*/
	long	pd_date;		/* Date of the transaction	*/
	short	pd_fund ;		/* Fund number    */
	char	pd_acct[19] ;		/* G/L Payroll Account	*/
	char	pd_code[7];		/* Benefit code		*/
	char	pd_group[7];		/* Benefit code		*/
	double	pd_amount;		/* Amount for the pay period	*/

}	Pay_ded ;

/*
*	Pay Period CSB/Loan File.  Maintains the employee pay period
*	CSB/Loan information.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: pc_numb + pc_code + pc_seq
*/

typedef struct	{

	char	pc_numb[13] ;		/* Employee number    */
	char	pc_code[7] ; 		/* CSB/Loan Code */
	short	pc_seq ;		/* Sequence number    */
	short	pc_pp;			/* Pay period		*/
	long	pc_date;		/* Date of the transaction	*/
	short	pc_fund ;		/* Fund number    */
	char	pc_acct[19] ;		/* G/L Payroll Account	*/
	double	pc_amount;		/* Fixed/Percentage Amount	*/
	double	pc_int_amt;		/* Interest amount		*/

}	Pay_loan ;

/*
*	Pay Period Garnishment File.  Maintains the employee  pay period
*	garnishment information.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: pg_numb + pg_code + pg_seq
*/

typedef struct	{

	char	pg_numb[13] ;		/* Employee number    */
	short	pg_pr_cd ; 		/* Priority Code */
	short	pg_seq ;		/* Sequence number    */
	short	pg_pp;			/* Pay period		*/
	short	pg_fund ;		/* Fund number    */
	char	pg_acct[19] ;		/* G/L Payroll Account	*/
	long	pg_date;		/* Date of the transaction	*/
	double	pg_amount;		/* Fixed/Percentage Amount	*/

}	Pay_garn ;

/*
*	Cheque Message File.  Stores the messages.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: m_code
*/

typedef struct	{

	char	m_code[7] ; 		/* Department Code */
	char	m_desc[31] ;		/* Department description */

}	Chq_mess ;

/*
*	Cheque Message Assignment File.  Stores the assignments of the messages.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: ma_numb
*/

typedef struct	{

	char	ma_numb[13] ;		/* Employee number    */
	char	ma_code[7] ; 		/* Garnishment Code */

}	Chq_mess_ass ;

/*
*	Employee Reg Pen Adj File.  Stores the employee's registered pensaion
*	adjustment amount.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: rpa_numb
*/

typedef struct	{

	char	rpa_numb[13] ;		/* Employee number    */
	double	rpa_amount;		/* Amount		*/

}	Reg_pen_adj ;

/*
*	T4 field code File.  Stores the codes associated to each field on 
*	the T4 form.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: t4_code
*/

typedef struct	{

	char	t4_code[2];		/* Employee number    */
	char  	t4_desc[13];		/* Description */

}	T4_rec ;

/*
*	Payroll G/L Account File.  Stores the G/L accounts that need to be 
*	posted to.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: gl_fund + gl_cc + gl_type[0] + gl_earn[0] + gl_class[0]
*/

typedef struct	{

	short	gl_fund ;		/* Fund number    */
	short	gl_cc ;			/* Cost Center number    */
	char	gl_type[2] ;		/* Type S,U,C,D,R,T    */
	char	gl_earn[7] ;		/* Earnings Code	*/
	char	gl_class[7] ;		/* Classification Code	*/
	char	gl_acct[19] ;		/* G/L Payroll Account	*/

}	Gl_acct ;

/*
*	Payroll Journal File.  Stores the journal entries to be performed
*	at a later date.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: jr_fund + jr_no
*/

typedef struct	{

	short	jr_fund ;		/* Fund number    */
	short	jr_no ;			/* Item no	*/
	long	jr_date;		/* Date of Transaction	*/
	char	jr_acct[19] ;		/* G/L Payroll Account	*/
	char	jr_emp_numb[13];	/* Employee Number	*/
	char	jr_code[7];		/* Code			*/
	char	jr_type[2];		/* Type */
	double	jr_amount;		/* Amount of transaction*/
	char	jr_class[7];		/* classification code*/
	char	jr_pay_sen[2];		/* From payroll or seniority*/

}	Jr_ent;

/*
*	Payroll Journal History File. Stores the journal entries to be used
*	when cancelling a cheque.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: jrh_fund + jrh_no
*/

typedef struct	{

	short	jrh_fund ;		/* Fund number    */
	short	jrh_no ;		/* Item no	*/
	long	jrh_cheque ;		/* Cheque Number	*/
	long	jrh_date;		/* Date of Transaction	*/
	char	jrh_acct[19] ;		/* G/L Payroll Account	*/
	char	jrh_emp_numb[13];	/* Employee Number	*/
	char	jrh_code[7];		/* Code			*/
	char	jrh_type[2];		/* Type */
	double	jrh_amount;		/* Amount of transaction*/
	char	jrh_class[7];		/* Amount of transaction*/

}	Jrh_ent;

/*
*	Payroll Audit File.  Stores the changes made to the employees
*	earnings, benefit and deductions amounts.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: aud_emp + aud_code
*/

typedef struct	{

	char	aud_emp[13] ;		/* Employee number	*/
	char	aud_code[21] ;		/* Code	*/
	char	aud_flag[2];		/* Earns/Deducted	*/
	double	aud_old_amnt;		/* Previous Amount	*/
	double	aud_new_amnt;		/* Adjusted Amount	*/

}	Aud_pay;

/*
*	Ec File.  
*	 
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: ec_numb 
*/

typedef struct	{

	char	ec_numb[13] ;		/* Employee number	*/
	short	ec_cc;			/* Cost Center		*/
	char	ec_exp[7] ;		/* Expense Code		*/
	short	ec_pp_numb;		/* Pay Period Number	*/
	char	ec_trans[7] ;		/* Transaction Code	*/
	char	ec_earn[7] ;		/* Earnings Code	*/
	double	ec_wk1_rate;		/* Week 1 Rate	*/
	double	ec_wk1_units;		/* Week 1 Units	*/
	double	ec_wk1_earn;		/* Week 1 Earn	*/
	double	ec_wk2_rate;		/* Week 2 Rate	*/
	double	ec_wk2_units;		/* Week 2 Units	*/
	double	ec_wk2_earn;		/* Week 2 Earnings	*/
	double	ec_prev_wk1;		/* 	*/
	double	ec_prev_wk2;		/* 	*/

}	Ec_rec;

/*
*	Cheque Register File.  
*	 
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: cr_numb + cr_date
*/

typedef struct	{

	long	cr_numb ;		/* Cheque number	*/
	long	cr_date;		/* Cheque Date		*/
	char	cr_emp_numb[13];	/* Employee Number	*/
	char	cr_status[2];		/* Cheque Status 	*/
	double	cr_amount;		/* Amount		*/

}	Chq_reg ;

/*
*	Manual Cheque File.  
*	 
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: mc_emp_numb + mc_date
*/

typedef struct	{

	char	mc_emp_numb[13];	/* Employee Number	*/
	long	mc_date;		/* Cheque Date		*/
	short	mc_fund;		/* Fund Number		*/	
	char	mc_acct[19];		/* Account		*/
	long	mc_chq_numb ;		/* Cheque number	*/
	char	mc_ded_code[7];		/* Deduction Code	*/
	double	mc_amount;		/* Amount	*/

}	Man_chq ;

/*
*	Record of Employment File.  
*	 
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: ro_emp_numb + ro_date
*/

typedef struct	{

	char	ro_emp_numb[13];	/* Employee Number	*/
	char	ro_serial[11];		/* Serial Number        */
	long	ro_first_dt;		/* First Day Worked	*/
	long	ro_last_dt;		/* Last Day Worked	*/
	long	ro_uic_prdt;		/* UIC Prem Pay Up To	*/
	long	ro_final_dt;		/* Final Pay Per Ending	*/
	long	ro_stat1_dt;		/* Statutory Holiday	*/
	double	ro_stat1_amnt;		/* Amount	*/
	long	ro_stat2_dt;		/* Statutory Holiday	*/
	double	ro_stat2_amnt;		/* Amount	*/
	long	ro_stat3_dt;		/* Statutory Holiday	*/
	double	ro_stat3_amnt;		/* Amount	*/
	short	ro_fund;		/* Fund Number		*/	
	char	ro_reas1[12];		/* Account		*/
	double	ro_reas1_amnt;		/* Amount	*/
	char	ro_reas2[12];		/* Account		*/
	double	ro_reas2_amnt;		/* Amount	*/
	char	ro_reas3[12];		/* Account		*/
	double	ro_reas3_amnt;		/* Amount	*/
	char	ro_all[3];		/* Allocated Final Pay	*/
	long	ro_start_dt;		/* Last Day Worked	*/
	char	ro_wk_days[2];		/* Weeks_Days	*/
	short	ro_week_dno;		/* No of weeks or days */
	double	ro_amnt;		/* Amount	*/
	char	ro_e_n_u[2];		/* Weeks_Days	*/
	long	ro_ret_dt;		/* Last Day Worked	*/
	char	ro_reason[2];		/* Weeks_Days	*/
	char	ro_contact[31];		/* Weeks_Days	*/
	char	ro_cntct_tel[11];		/* Last Day Worked	*/
	char	ro_issuer[31];		/* Weeks_Days	*/
	char	ro_issuer_tel[11];		/* Last Day Worked	*/
	long	ro_issue_dt;		/* Last Day Worked	*/
	char	ro_com1[31];		/* Weeks_Days	*/
	char	ro_com2[31];		/* Weeks_Days	*/
	char	ro_com3[31];		/* Weeks_Days	*/
	char	ro_com4[31];		/* Weeks_Days	*/
	double	ro_ins_earn[27];	/* 27 weeks earnings */
	short	ro_ins_wks[27];		/* 27 weeks nbr of weeks */
	double	ro_vac;			/* vacation earnings */
	char	ro_all_wks_max[2];	/* all weeks max earnings */
	short	ro_ins_week;
	double  ro_hours;               /* Total hours for 53 or 27 pay p. */
	double	ro_earnings;		/* Total insurable earnings */
	char    ro_un_ins[2];           /* Flag weather to print details */
}	Roe ;

/*
*	Structure/record definition of the Temporary Employee schedule1 file
*
*	File Type: ISAM. No of Isam keys are : 1.
*
*	Main key : tm_numb
*/

typedef struct {
	char	tm_numb[13] ;		/* Employee number    */
	short	tm_week;
	short	tm_fund;
	char	tm_class[7];		/* Classification code	*/
	short	tm_cost;		/* Cost center number	*/
	char	tm_dept[7];		/* Department code	*/
	char	tm_area[7];		/* Area code		*/
	char	tm_sortk_1[41];		
	char	tm_sortk_2[41];		
	char	tm_sortk_3[41];		
}	Tmp_sched1 ;

/*
*	Seniority Parameter. 
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: sn_position + sn_eff_date
*	
*/

typedef struct	{

	char	sn_position[7];
	long	sn_eff_date;
	double	sn_max_days_yr;
	double	sn_num_hrs_day;
	char	sn_barg[7];
	char	sn_month[12][32] ;
	double	sn_poss_days[12];

}	Sen_par ;

/*
*	Employee Seniority File. 
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: esn_numb + esn_pp + esn_date
*	
*/

typedef struct	{

	char	esn_numb[13];
	short	esn_month;
	char	esn_pos[7];
	char	esn_class[7];
	double	esn_cas_hrs;
	double	esn_cas_days;
	double	esn_perm_days;
	double	esn_cas_totd;

}	Emp_sen ;

/*
*	Temporary Employee Seniority File. 
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: tsn_numb + tsn_pp + tsn_date
*	
*/

typedef struct	{

	char	tsn_numb[13];
	short	tsn_month;
	char	tsn_pos[7];
	char	tsn_class[7];
	double	tsn_cas_hrs0;
	double	tsn_cas_days0;
	double	tsn_perm_days0;
	double	tsn_cas_totd0;
	double	tsn_sick_acc0;
	double	tsn_vac_acc0;
	double	tsn_cas_hrs1;
	double	tsn_cas_days1;
	double	tsn_perm_days1;
	double	tsn_cas_totd1;
	double	tsn_sick_acc1;
	double	tsn_vac_acc1;

}	Tmp_sen ;

/*
*	Government parameter File.  Maintains the personnel/payroll government
*	parameters.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: 
*/

typedef struct	{

	long	gp_eff_date;		/* Effective Date		*/
	double	gp_fed_sur1;		/* Federal Surtax Rate 1      */
	double	gp_fed_sur_lim;		/* Fed Surtax Limit          */
	double	gp_fed_sur2;		/* Federal Surtax Rate 2      */
	double	gp_net_ptcr;		/* Net Pers Tax Cr Rate      */
	double	gp_cpp_incr;		/* CPP Income Tax Rate            */
	double	gp_cpp_incm;		/* CPP Income Tax Max        */ 
	double	gp_cpp_ytd_inc;		/* CPP YTD Max Inc Tax       */
	double	gp_uic_incr;		/* UIC Income Tax Rate       */
	double	gp_uic_incm;		/* UIC Income Tax Max       */
	double	gp_uic_ytd_inc;		/* UIC YTD Max Inc Tax       */
	double	gp_tax_red; 		/* Prov Tax Reduction              */
	double	gp_prov_tax_rate;	/* Provincial Tax Rate         */
	double	gp_prov_sur;		/* Provincial Surtax         */
	double	gp_prov_net;		/* Prov Net Inc Tax Surtax       */
	double	gp_prov_sur_rate;	/* Prov Surtax Rate         */
	double	gp_prov_sur_lim;	/* Prov Surtax Limit         */
	double	gp_prov_flat1;		/* Provincial Flat Tax 1        */
	double	gp_prov_flat2;		/* Provincial Flat Tax2      */
	short	gp_cpp_min_age;		/* CPP Minimum Age             */
	double	gp_cpp_pen_earn;	/* Yearly CPP Pen Earn       */
	short	gp_cpp_max_age;		/* CPP Maximum Age              */
	double	gp_cpp_max_contr;	/* Yearly CPP Max Contr      */
	double	gp_cpp_reg_rate;	/* CPP Regular Rate            */
	double	gp_cpp_bac_expt;	/* CPP Basic Exemption       */
	double	gp_uic_prem;		/* UIC Emp Premium Rate           */
	double	gp_uic_empl_rate;	/* UIC Employer's Rate          */
	char	gp_empl_ref[12];	/* Rev can Tax Acc # 1       */
	double	gp_net_inc_rate;	/* Net Income Tax Rate         */
	char	gp_tax_acct[12];	/* Rev Can Tax Acc # 2      */
	double	gp_soc_ben;		/* Social Benefit Repay     */
	double	gp_soc_benl;		/* Social Benefit Limit      */

}	Gov_param ;

/*
*	T4 Adjustment File.  Maintains the personnel/payroll T4
*	adjustment
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: 	ta_numb;
*/

typedef struct	{

	char	ta_numb[13] ;		/* Employee number    */
	double	ta_emp_inc;		/* Employment Income */
	double	ta_cpp_cont;		/* CPP Contribution */
	double	ta_uic_prem;		/* UIC Premiums */
	double	ta_tax_ded;		/* Income Tax Deducted */
	double	ta_uic_ins_earn;	/* UIC Insurance Earnings */
	double	ta_cpp_pen_earn;	/* CPP Pension Earnings */
	double	ta_housing;		/* Housing Board and Lodging */
	double	ta_travel;		/* Travel in a Prescribed Area */
	double	ta_auto;		/* Personal Use of Employer's Auto */
	double	ta_intrest;		/* Intrest Free & Low Intrest Loans */
	double	ta_other_tax;		/* Other Tax Allowance & Benefits */
	double	ta_emp_com;		/* Employment Commissions */
	double	ta_union_du;		/* Union Dues */
	double	ta_donnat;		/* Charitable Donations */
	double	ta_pay_dpsp;		/* Payments to DPSP */
	double	ta_pen_adj;		/* Pension Adjustment */
	char	ta_reg_pen_num[13];	/* Reg num for registered pen plan */
}	T4_adj ;

/*
*	Religion Code File.  Stores the codes and descriptions.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: rel_code
*/

typedef struct	{

	char	rel_code[3] ; 		/* Religion Code */
	char	rel_desc[31] ;		/* Religion description */

}	Religion ;

/*
*	Payroll User Security File.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: ub_id + ub_barg
*/

typedef struct	{

	char	ub_id[11];	/* user's login name */
	char	ub_barg[7];	/* user's login teminal */
	char	ub_add[2];	/* user's class: Administrator or User */
	char	ub_update[2];	/* user's class: Administrator or User */
	char	ub_delete[2];	/* user's class: Administrator or User */
	char	ub_browse[2];	/* user's class: Administrator or User */
	/* access rights by bargaining unit & operation
					( DELETE, UPDATE, ADD, BROWSE) */

}	Userbarg;

/*
*	Structure/record definition of the Temporary Employee file
*
*	File Type: ISAM. No of Isam keys are : 1.
*
*	Main key : tem_numb
*/

typedef struct {
	char	tem_numb[13] ;		/* Employee number    */
	char	tem_last_name[26] ;	/* Last name            */
	char	tem_first_name[16] ;	/* First name           */
	char	tem_mid_name[16] ;	/* Middle name          */
	char	tem_barg[7];		/* Bargaining unit code    */
	char	tem_pos[7];		/* Position code           */
	short	tem_cc ;			/* Cost center number */

}	Tmp_Emp ;

/*
*	Structure/record definition of the Temporary Employee file
*
*	File Type: ISAM. No of Isam keys are : 1.
*
*	Main key : tem2_cc + tem2_last_name + tem2_first_name + tem2_numb
*/

typedef struct {
	char	tem2_numb[13] ;		/* Employee number    */
	char	tem2_last_name[26] ;	/* Last name            */
	char	tem2_first_name[16] ;	/* First name           */
	char	tem2_mid_name[16] ;	/* Middle name          */
	char	tem2_barg[7];		/* Bargaining unit code    */
	char	tem2_pos[7];		/* Position code           */
	short	tem2_cc ;			/* Cost center number */

}	Tmp2_Emp ;

/*
*	Structure/record definition of the Temporary Sort Seniority file
*
*	File Type: ISAM. No of Isam keys are : 1.
*
*	Main key : ts_total_years + ts_total_days   
*/

typedef struct {
	short	ts_total_years;		/* employee's total years */
	double	ts_total_days;		/* employee's total days */
	char	ts_position[7];		/* employee's position */
	char	ts_barg[7];		/* employee's bargaining unit */
	short	ts_cc;			/* employee's cost center */
	char	ts_class[7];		/* employee's class code */
	char	ts_emp_numb[13] ;	/* Employee number    */
	short	ts_cas_years;		/* employee's casual years */
	double	ts_cas_days;		/* employee's casual days */
	short	ts_perm_years;		/* employee's perm years */
	double	ts_perm_days;		/* employee's perm days */
	double	ts_sick_bal;		/* employee's sick balance */
	double	ts_vac_bal;		/* employee's vacation balance */

}	Ts_sen ;

/*
*	Structure/record definition of the Vacation Accrual file
*
*	File Type: ISAM. No of Isam keys are : 1.
*
*	Main key : vc_barg + vc_low_sen + vc_high_sen   
*/

typedef struct {

	char	vc_barg[7];		/* Vacation Bargaining Unit */
	double	vc_low_sen ;		/* Vac Low Seniority Years  */
	double	vc_high_sen;		/* Vac High Seniority Years */
	double	vc_days;		/* Vacation Days */

}	Vc_acc ;

/*
*	Employee Competition File.  Maintains the employee competition codes.
*
*	File Type: ISAM.
*	No of Index Keys: 1.
*
*	Main Key: ec_numb + ec_code
*/

typedef struct	{

	char	ec_numb[13] ;		/* Employee number    */
	char	ec_code[8];		/* Competition Code */

}	Emp_comp ;

/*
*	Structure/record definition of the Competition file
*
*	File Type: ISAM. No of Isam keys are : 1.
*
*	Main key : cm_code   
*/

typedef struct {

	char	cm_code[8];		/* Competition Code */
	char	cm_desc[41];		/* Competition Description */

}	Comp ;
/*--------------------------------END OF FILE----------------------------*/
