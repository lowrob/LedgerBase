
#define	SCRNM1		"pay_par"
#define	SCRNM2		"pay_par2"
#define	PRGNM		"pay_par"

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
#define	CANCEL		'C'
#define	ENG		'E'
#define FRENCH		'F'

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
#define	ENG		'E'
#define FRENCH		'F'

#endif

#define FOUND		0
#define NOTFOUND	1
#define NO_CHANGE	2

#define MAX_FIELD1	29
#define MAX_FIELD2	95

#define	END_FLD1	3800
#define	END_FLD2	9600

/*-------------------- screen 1 information -------------------------*/

typedef	struct	{
	char	s_pgname[11];
	long	s_sysdate;	/* 300 system date */
	char	s_fn[2];	/* 500 function */
	short	s_field;	/* 600 field: */

	char	s_tp_ft[7];	/* 2600	message option */
	char	s_pay_att[2];	/* 2600	message option */
	char	s_tp_pt[7];	/* 2600	message option */
	char	s_pay_tch[2];	/* 2600	message option */
	double	s_tw_units;
	char	s_sub[7];	/* 2600	message option */
	char	s_area_def[2];	/* 2600	message option */

	long	s_retro;	
	long	s_cs_date;
	char	s_updt_gl[2];	/* 2600	message option */
	long	s_ce_date;

	short	s_dept;
	long	s_fs_date;
	short	s_area;
	long	s_fe_date;

	short	s_cc;
	long	s_ss_date;
	char	s_reg_cd[7];
	long	s_se_date;
	char	s_retro_cd[7];
	short	s_st_mth;
	char	s_vac_cd[7];
	long	s_last_chq;

	char	s_prov[4];	/* 2600	message option */
	long	s_last_ec;
	char	s_comm_pref[2];	/* 2600	message option */
	long	s_10mth_st_dt;
	char	s_payper[7];
	long	s_10mth_end_dt;

	char	s_mesg[78];	/* 2500 message field */
	char	s_opt[2];	/* 2600	message option */

	} s1_struct;

#define FN_FLD1	500	/* function field */
#define	CHG_FLD1	600	/* NUMERIC 99 */

#define	TEACH_FT	800
#define	PAY_ATT		900
#define	TEACH_PT	1000
#define	PAY_TEACH	1100
#define	SUB_TEACH	1300
#define	AREA_DEF	1400
#define	UPDT_GL		1700
#define	REG_CD	 	2500
#define	RETRO_CD	2700
#define	VAC_CD  	2900
#define	PROV		3100
#define	COMM_PREF	3300
#define	PP_FLD		3500

#define	MESG_FLD1	3700	/* message line */
#define	OPT_FLD1	3800	/* message option field */

/*-------------------- screen 2 information -------------------------*/

typedef struct{
	char	s_acct_keys[12][2];
	} S1_item;

typedef	struct	{
	char	s_pgname2[11];
	long	s_sysdate2;	/* 300 system date */
	char	s_fn2[2];	/* 500 function */
	short	s_field2;	/* 800 field: */

	S1_item	s_gl[6];

	short	s_fund;		/* 1000 fund number */
	char	s_acct[19];
	long	s_keys[12];

	char	s_mesg2[78];	/* 2500 message field */
	char	s_opt2[2];	/* 2600	message option */
	} s2_struct;

#define FN_FLD2	500	/* function field */
#define	CHG_FLD2	600	/* NUMERIC 99 */

#define	ACCT_KEY1	900
#define	ACCT_KEY2	2100
#define	ACCT_KEY3	3300
#define	ACCT_KEY4	4500
#define	ACCT_KEY5	5700
#define	ACCT_KEY6	6900
#define	FUND		8100
#define	TEACH_GL	8200

#define	MESG_FLD2	9500	/* message line */
#define	OPT_FLD2	9600	/* message option field */

static	Gl_rec	glmast;
static	Earn	earnings;
static	Position	position;
static	Pay_per	pay_per;
static	Pay_param pay_param,	pre_pay_param;
static	Ctl_rec	control;

struct stat_rec	sr;
s1_struct	s1_sth, tmp1_sth;
s2_struct	s2_sth, tmp2_sth;

char	e_mesg[80];

int first_time1;
int first_time2;

