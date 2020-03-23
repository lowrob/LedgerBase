/* repcd.h - used by repcd.c and repcd2.c */

#define	SCRNM1		"roe_adj1"
#define	SCRNM2		"roe_adj2"
#define	SCRNM3		"roe_adj3"
#define	PRGNM		"roe_adj"

#define	ALLOC_ERROR	-19
#define	MAX_KEYS	12

#ifdef ENGLISH

#define	CHANGE		'C'
#define INQUIRE		'I'
#define EXITOPT		'E'
#define NEXT_RECORD		'N'
#define PREV_RECORD		'P'
#define YES		'Y'
#define NO		'N'
#define	LINEEDIT	'L'
#define	SCREENEDIT		'S'
#define	FIRST_SCR	'1'
#define	SEC_SCR		'2'
#define	THIRD_SCR	'3'
#define	CANCEL		'C'

#else		/* if FRENCH */

#define	CHANGE		'C'
#define INQUIRE		'I'
#define EXITOPT		'E'
#define NEXT_RECORD		'N'
#define PREV_RECORD		'P'
#define YES		'O'
#define NO		'N'
#define	LINEEDIT	'M'
#define	SCREENEDIT	'C'
#define	FIRST_SCR	'1'
#define	SEC_SCR		'2'
#define	THIRD_SCR	'3'
#define	CANCEL		'A'

#endif

#define STEP		700
#define	NO_GRPS		3
#define NO_PDS		5

#define FOUND		0
#define NOTFOUND	1
#define NO_CHANGE	2

#define MAX_FIELD1	5	
#define MAX_FIELD2	53
#define MAX_FIELD3	16

#define	KEY_START	600
#define	KEY_END		600
#define	END_FLD1	3400
#define	END_FLD2	6900
#define	END_FLD3	2900

/*-------------------- screen 1 information -------------------------*/

typedef	struct	{
	char	s_pgname[11];
	long	s_sysdate;	/* 300 system date */
	char	s_fn[2];	/* 500 function */
	char	s_emp[13];
	short	s_field;	/* 800 field: */

	char	s_pay_ref[13];	/* 2600	message option */
	char	s_comm_pref[2];	/* 2600	message option */
	char	s_rev_can[13];	/* 2600	message option */
	char	s_pp_type[17];	/* 2600	message option */
	char	s_pp_no[2];	/* 2600	message option */

	char	s_empl_name[31];	/* 2600	message option */
	char	s_emp_name[31];	/* 2600	message option */
	char	s_empl_add1[31];	/* 2600	message option */
	char	s_emp_add1[31];	/* 2600	message option */
	char	s_empl_add2[31];	/* 2600	message option */
	char	s_emp_add2[31];	/* 2600	message option */
	char	s_empl_add3[31];	/* 2600	message option */
	char	s_emp_add3[31];	/* 2600	message option */
	char	s_empl_add4[31];	/* 2600	message option */
	char	s_emp_add4[31];	/* 2600	message option */
	char	s_empl_pc[11];	/* 2600	message option */
	char	s_emp_pc[11];	/* 2600	message option */

	char	s_occ[19];	/* 2600	message option */
	char	s_sin[10];
	long	s_fd_date;
	long	s_ld_date;
	long	s_upp_date;
	long	s_fppe_date;
	char	s_serial[11];

	char	s_mesg[78];	/* 2500 message field */
	char	s_opt[2];	/* 2600	message option */

	} s1_struct;

#define FN_FLD1	500	/* function field */
#define	EMP_FLD1	600	/* STRING XXXXXXXXXXXX */
#define	CHG_FLD1	700	/* NUMERIC 99 */

#define	PAY_REF	900	/* STRING XXXXXXXXXXXX */
#define	COMM_PREF	1000	/* STRING XXXXXXXXXXXX */
#define	REV_ACCT	1100	/* STRING XXXXXX */
#define	PP_TYPE	1200	/* STRING X */

#define	EMPL_NAME	1400
#define	EMP_NAME	1500
#define	EMPL_ADD1	1600
#define	EMP_ADD1	1700
#define	EMPL_ADD2	1800
#define	EMP_ADD2	1900
#define	EMPL_ADD3	2000
#define	EMP_ADD3	2100
#define	EMPL_ADD4	2200
#define	EMP_ADD4	2300

#define	EMPL_PC	2400	/* STRING XXXXXXXXXX */
#define	EMP_PC	2500	/* STRING XXXXXXXXXX */

#define	OCC	2600	/* STRING XXXXXXXXXXXXXXXXXX */
#define	SIN	2700

#define	FD_DATE	2800	/* NUMERIC 9999F99F99 */
#define	LD_DATE	2900	/* NUMERIC 9999F99F99 */
#define	UPP_DATE	3000	/* NUMERIC 9999F99F99 */
#define	FPPE_DATE	3100	/* NUMERIC 9999F99F99 */
#define AMND_SER	3200	/* STRING PIC X(10) */

#define	MESG_FLD1	3300	/* message line */
#define	OPT_FLD1	3400	/* message option field */

/*-------------------- screen 2 information -------------------------*/


typedef	struct	{
	char	s_pgname2[11];
	long	s_sysdate2;	/* 300 system date */
	char	s_fn2[2];	/* 500 function */
	char	s_emp2[13];
	short	s_field2;	/* 800 field: */
	char	s_dummy1[3];

	double	s_pp_earn1;	/* amount */
	short	s_no_pp1;	/* employee share */
	double	s_pp_earn2;	/* amount */
	short	s_no_pp2;	/* employee share */
	double	s_pp_earn3;	/* amount */
	short	s_no_pp3;	/* employee share */
	double	s_pp_earn4;	/* amount */
	short	s_no_pp4;	/* employee share */
	double	s_pp_earn5;	/* amount */
	short	s_no_pp5;	/* employee share */
	double	s_pp_earn6;	/* amount */
	short	s_no_pp6;	/* employee share */
	double	s_pp_earn7;	/* amount */
	short	s_no_pp7;	/* employee share */
	double	s_pp_earn8;	/* amount */
	short	s_no_pp8;	/* employee share */
	double	s_pp_earn9;	/* amount */
	short	s_no_pp9;	/* employee share */
	double	s_pp_earn10;	/* amount */
	short	s_no_pp10;	/* employee share */
	double	s_pp_earn11;	/* amount */
	short	s_no_pp11;	/* employee share */
	double	s_pp_earn12;	/* amount */
	short	s_no_pp12;	/* employee share */
	double	s_pp_earn13;	/* amount */
	short	s_no_pp13;	/* employee share */
	double	s_pp_earn14;	/* amount */
	short	s_no_pp14;	/* employee share */
	double	s_pp_earn15;	/* amount */
	short	s_no_pp15;	/* employee share */
	double	s_pp_earn16;	/* amount */
	short	s_no_pp16;	/* employee share */
	double	s_pp_earn17;	/* amount */
	short	s_no_pp17;	/* employee share */
	double	s_pp_earn18;	/* amount */
	short	s_no_pp18;	/* employee share */
	double	s_pp_earn19;	/* amount */
	short	s_no_pp19;	/* employee share */
	double	s_pp_earn20;	/* amount */
	short	s_no_pp20;	/* employee share */

	double	s_total;	/* amount */
	short	s_ins_wks;		/* 1000 fund number */
	double	s_vac_pay;	/* amount */
	char	s_all_wks_max[2]; /* flag for all weeks earnings are max */

	char	s_dummy2[2];

	long	s_stat_date1;
	double	s_stat_amt1;	/* amount */
	char	s_reason1[12];
	double	s_reas_amt1;	/* amount */
	long	s_stat_date2;
	double	s_stat_amt2;	/* amount */
	char	s_reason2[12];
	double	s_reas_amt2;	/* amount */
	long	s_stat_date3;
	double	s_stat_amt3;	/* amount */
	char	s_reason3[12];
	double	s_reas_amt3;	/* amount */

	char	s_all_pp[2];	/* 2600	message option */
	
	char	s_mesg2[78];	/* 2500 message field */
	char	s_opt2[2];	/* 2600	message option */
	} s2_struct;


#define FN_FLD2	500	/* function field */
#define	EMP_FLD2	600	/* STRING XXXXXXXXXXXX */
#define	CHG_FLD2	700	/* NUMERIC 99 */

#define	EX	900	/* STRING XX */

#define	PP_EARN1	1000	/* NUMERIC 9F999F999.99 */
#define	NO_PP1	1100	/* STRING X */
#define	PP_EARN2	1200	/* NUMERIC 9F999F999.99 */
#define	NO_PP2	1300	/* STRING X */
#define	PP_EARN3	1400	/* NUMERIC 9F999F999.99 */
#define	NO_PP3	1500	/* STRING X */
#define	PP_EARN4	1600	/* NUMERIC 9F999F999.99 */
#define	NO_PP4	1700	/* STRING X */
#define	PP_EARN5	1800	/* NUMERIC 9F999F999.99 */
#define	NO_PP5	1900	/* STRING X */
#define	PP_EARN6	2000	/* NUMERIC 9F999F999.99 */
#define	NO_PP6	2100	/* STRING X */
#define	PP_EARN7	2200	/* NUMERIC 9F999F999.99 */
#define	NO_PP7	2300	/* STRING X */
#define	PP_EARN8	2400	/* NUMERIC 9F999F999.99 */
#define	NO_PP8	2500	/* STRING X */
#define	PP_EARN9	2600	/* NUMERIC 9F999F999.99 */
#define	NO_PP9	2700	/* STRING X */
#define	PP_EARN10	2800	/* NUMERIC 9F999F999.99 */
#define	NO_PP10	2900	/* STRING X */
#define	PP_EARN11	3000	/* NUMERIC 9F999F999.99 */
#define	NO_PP11	3100	/* STRING X */
#define	PP_EARN12	3200	/* NUMERIC 9F999F999.99 */
#define	NO_PP12	3300	/* STRING X */
#define	PP_EARN13	3400	/* NUMERIC 9F999F999.99 */
#define	NO_PP13	3500	/* STRING X */
#define	PP_EARN14	3600	/* NUMERIC 9F999F999.99 */
#define	NO_PP14	3700	/* STRING X */
#define	PP_EARN15	3800	/* NUMERIC 9F999F999.99 */
#define	NO_PP15	3900	/* STRING X */
#define	PP_EARN16	4000	/* NUMERIC 9F999F999.99 */
#define	NO_PP16	4100	/* STRING X */
#define	PP_EARN17	4200	/* NUMERIC 9F999F999.99 */
#define	NO_PP17	4300	/* STRING X */
#define	PP_EARN18	4400	/* NUMERIC 9F999F999.99 */
#define	NO_PP18	4500	/* STRING X */
#define	PP_EARN19	4600	/* NUMERIC 9F999F999.99 */
#define	NO_PP19	4700	/* STRING X */
#define	PP_EARN20	4800	/* NUMERIC 9F999F999.99 */
#define	NO_PP20	4900	/* STRING X */

#define	TOTAL	5000	/* NUMERIC 9F999F999.99 */
#define	NO_INS_WK	5100	/* NUMERIC 99 */
#define	VAC_PAY	5200	/* NUMERIC 9F999F999.99 */
#define	MAX_EARN 5300	/* STRING X */

#define	STAT_HOL1	5500	/* NUMERIC 9999F99F99 */
#define	HOL_AMT1	5600	/* NUMERIC 9F999F999.99 */
#define	REASON1	5700	/* STRING XXXXXXXXXXXXXXX */
#define	REAS_AMT1	5800	/* NUMERIC 9F999F999.99 */
#define	STAT_HOL2	5900	/* NUMERIC 9999F99F99 */
#define	HOL_AMT2	6000	/* NUMERIC 9F999F999.99 */
#define	REASON2	6100	/* STRING XXXXXXXXXXXXXXX */
#define	REAS_AMT2	6200	/* NUMERIC 9F999F999.99 */
#define	STAT_HOL3	6300	/* NUMERIC 9999F99F99 */
#define	HOL_AMT3	6400	/* NUMERIC 9F999F999.99 */
#define	REASON3	6500	/* STRING XXXXXXXXXXXXXXX */
#define	REAS_AMT4	6600	/* NUMERIC 9F999F999.99 */

#define	ALL_FP	6700	/* STRING X */
	
#define	MESG_FLD2	6800	/* message line */
#define	OPT_FLD2	6900	/* message option field */

/*-------------------- screen 3 information -------------------------*/

typedef struct{
	char	s_pgname3[11];
	long	s_sysdate3;	/* 300 system date */
	char	s_fn3[2];	/* 500 function */
	char	s_emp3[13];
	short	s_field3;	/* 800 field: */
	char	s_dummy3[2];

	long	s_start_dt;		/* Last Day Worked	*/
	char	s_wk_days[2];		/* Weeks_Days	*/
	short	s_week_dno;		/* No of weeks or days */
	double	s_amnt;		/* Amount	*/
	char	s_e_n_u[2];		/* Weeks_Days	*/
	long	s_ret_dt;		/* Last Day Worked	*/
	char	s_reason[3];		/* Weeks_Days	*/
	char	s_reas_desc[31];	/* Weeks_Days	*/
	char	s_contact[31];		/* Weeks_Days	*/
	char	s_cntct_tel[11];		/* Last Day Worked	*/
	char	s_issuer[31];		/* Weeks_Days	*/
	char	s_issuer_tel[11];		/* Last Day Worked	*/
	long	s_issue_dt;		/* Last Day Worked	*/
	char	s_com1[31];		/* Weeks_Days	*/
	char	s_com2[31];		/* Weeks_Days	*/
	char	s_com3[31];		/* Weeks_Days	*/
	char	s_com4[31];		/* Weeks_Days	*/

	char	s_mesg3[78];	/* 2500 message field */
	char	s_opt3[2];	/* 2600	message option */

	} s3_struct;

#define	FN_FLD3	500	/* STRING X */
#define	EMP_FLD3	600	/* STRING XXXXXXXXXXXX */
#define	CHG_FLD3	700	/* NUMERIC 99 */

#define	START_DATE	1000	/* NUMERIC 9999F99F99 */
#define	WEEK_DAYS	1100	/* STRING X */
#define	WD_NO	1200	/* NUMERIC 99 */
#define	AMOUNT	1300	/* NUMERIC 9F999F999.99 */
#define	ENU	1400	/* STRING X */
#define	DATE	1500	/* NUMERIC 9999F99F99 */
#define	REASON	1600	/* STRING X */
#define	REAS_DESC	1700	/* STRING XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX */
#define	CONTACT	1800	/* STRING XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX */
#define	CNT_TEL	1900	/* NUMERIC 999F999F9999 */
#define	ISSUER	2000	/* STRING XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX */
#define	ISS_TEL	2100	/* NUMERIC 999F999F9999 */
#define	DATE_ISS	2200	/* NUMERIC 9999F99F99 */
#define	COM1	2300
#define	COM2	2400
#define	COM3	2500
#define	COM4	2600
#define	MESG_FLD3	2700	/* message line */
#define	OPT_FLD3	2800	/* message option field */

Emp		emp_rec;
Pay_param	pay_param;
Barg_unit	barg_unit;
Emp_ins	emp_ins,	pre_emp_ins;
Position	position;
Pa_rec	param;
Gov_param	gov_param;	/* payroll parameter file */
Sch_rec	school;
Pay_per	pay_per;
Term	term_rec;
Roe	roe,	pre_roe;

struct stat_rec	sr;
s1_struct	s1_sth, tmp1_sth;
s2_struct	s2_sth, tmp2_sth;
s3_struct	s3_sth, tmp3_sth;

char	e_mesg[200];

int first_time1;
int first_time2;
int first_time3;

static	char	*Mesg;
static	char	*Res;
static	char	*CurrentScreen;

