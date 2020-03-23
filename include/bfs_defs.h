
/*
*    Source Name : bfs_defs.h
*    System      : Budgetary Financial System.
*
*    Created On  : 2nd May 1989.
*
*    Contains Defined Constants used by system.
*/

#include "gen_routine.h"

#define float double
/* No longer have to define English or French as */
/* It is passed as a parameter in the make files */
/* as -DENGLISH or -DFRENCH respectively.        */

/* define XENIX, UNIX, OMEGA or MS_DOS for IBM-AT XENIX version,
   68000 Family Unix, Omega 58000 and MS-DOS respectively */

#define	UNIX			/* 68000 Family Unix (NCR Tower) */

#ifdef	UNUSED
#define	XENIX			/* XENIX Version */
#define	MS_DOS			/* MS_DOS Version */
#define	OMEGA			/* Omega 58000 */
#endif

#ifdef	MS_DOS
#define	SINGLE_USER		/* Single User system */
#else
#define	MULTI_USER		/* Multi User system */
#endif

/*
*	ORACLE dbh is activated by defining ORACLE at compilation cmnd line
*
*/

/*
#ifndef ORACLE

/ *
*	DBH journalling is active
* /
#define	JRNL

#endif
*/

/*
*	Security is active
*/
#define	SECURITY

/*	User classes are defined here, used if security is enabled */


#define	ORD_USER	'U'	/* ordinary user */
#define	ADMINISTRATOR	'A'	/* database administrator */
#define	SUPERUSER	'S'	/* database superuser, max: 1 per database */

/* File Numbers used in DBH (recio.c) */

#define	PARAM		0	/* Parameters file */
#define	CONTROL		1	/* Control/Fund File */
#define	SCHOOL		2	/* School File */
#define	AUDIT		3	/* Audit File */

#define	GLMAST		4	/* G/L Master File */
#define	GSTDIST		5	/* GST Distribution File */
#define	RECHDR		6	/* Recurring Entry Header File */
#define	RECTRAN		7	/* Recurring Entry Items File */
#define	GLTRHDR		8	/* G/L Trans Header File */
#define	GLTRAN		9	/* G/L Trans Items File */
#define	GLTRHDRNY	10	/* G/L Trans Header File New Year */
#define	GLTRANNY	11	/* G/L Trans Items File New Year */
#define	GLBDHDR		12	/* Budget transaction header file */
#define	GLBDITEM	13	/* Budget transaction item file  */

#define	STMAST		14	/* Stock Master File */
#define	STTRAN		15	/* Stock Transaction File */
#define	ALLOCATION	16	/* Stock Allocation File */
#define	SECTION		17	/* Stock section File */

#define SUPPLIER	18      /* Supplier File */
#define POHDR		19	/* Purchase Order Header File */
#define POITEM		20	/* Purchase Order Item File */

#define REQHDR		21	/* Requisition Header File */
#define REQITEM		22	/* Requisition Item File */
#define REQREASON	23	/* Requisition Reason File */




#define	CATEGORY	24	/* Category File */
#define	ITEM_GROUP	25	/* Item Group File */
#define	CATALOGUE	26	/* Catalogue File */
#define	POTBIDDER	27	/* Potential Bidder File */
#define	BID		28	/* Bid File	*/
#define	TENDHIST	29	/* Tender History File */

#define	FAMAST		30	/* Fixed Assets Item Master */
#define	FATYPE		31	/* Fixed Asset Item Types */
#define	FADEPT		32	/* Fixed Asset Dept Codes */
#define FATRAN		33	/* Fixed Asset Transfers */

#define	CUSTOMER	34	/* Customer Master File */
#define	ARSHDR		35	/* Sales Invoice header file */
#define ARSITEM		36	/* Sales Invoice item file */
#define	RCPTHDR		37	/* Customer Receipts Header File */
#define	RCPTITEM	38	/* Customer Receipts Item File */

#define	APINVOICE	39	/* Purchase Invoices File */
#define	APINHDR		40	/* Purchase Invoice Header */
#define	APINITEM	41	/* Purchase Invoice Items */ 
#define	CHEQUE		42	/* Cheques File */
#define CHQHIST		43	/* Cheques History File */
#define CHQREG		44	/* Cheques History File */
#define	APHIST		45	/* AP Invoice History File */

#define LAST_PO		46	/* Last PO Number */
#define LAST_REQ	47	/* Last Requisition Number */

#define	DEPARTMENT	48	/* Department File	*/
#define	AREA		49	/* Area File	*/
#define	POSITION	50	/* Position File	*/
#define	CLASSIFICATION	51	/* Classification File	*/
#define	CLASS_ITEM	52	/* Classification item file	*/
#define	BARG		53	/* Bargaining unit file */
#define	BANK		54	/* Bank code file	*/
#define	PAY_PERIOD	55	/* Pay period code file	*/
#define	PAY_PER_ITEM	56	/* Pay period item file	*/
#define	UIC		57	/* UIC table file	*/
#define	TAX		58	/* Income tax table file	*/
#define	CERT		59	/* Certification table file	*/
#define	EARN		60	/* Earnings code file	*/
#define	TRANS		61	/* Transaction code file	*/
#define	TRANS_ITEM 	62	/* Transaction item file	*/
#define	EXPENSE		63	/* Expense code file	*/
#define	EXP_ITEM	64	/* Expense item file	*/

#define	TERM		65	/* Termination code file */
#define	STAT_HOL	66	/* Statutory Holiday	*/
#define	ATT		67	/* Attendance code file	*/
#define	INACT_CODE	68	/* Inactivation code file	*/
#define	AREA_SPEC	69	/* Area of specialization code file	*/
#define	LOAN		70	/* CSB/Loan code file	*/
#define	PAY_PARAM	71	/* Parameter file	*/
#define	SALARY		72	/* Salary account keys	*/

#define	BENEFIT		73	/* Benefit file		*/
#define	BEN_CAT		74	/* Benefit category file	*/
#define	DEDUCTION	75	/* Deduction file		*/
#define	DED_GRP		76	/* Deduction category file	*/
#define	DED_CAT		77	/* Deduction category file	*/
#define	REG_PEN		78	/* Registered pension plan file	*/

#define	EMPLOYEE	79	/* Employee file		*/
#define	EMP_EMP		80	/* Employee employment file	*/
#define	EMP_SCHED1	81	/* Employee schedule 1 file	*/
#define	EMP_SCHED2	82	/* Employee schedule 2 file	*/
#define	EMP_EXTRA	83	/* Employee schedule extra file	*/
#define	EMP_EARN	84	/* Employee earnings file	*/
#define	EMP_INS		85	/* Employee insurable earnings file	*/
#define	EMP_BEN		86	/* Employee benefit file	*/
#define	EMP_BEN_HIS	87	/* Employee benefit history file	*/
#define	EMP_DED		88	/* Employee deduction file	*/
#define	EMP_DED_HIS	89	/* Employee deduction history file	*/
#define	EMP_LOAN	90	/* Employee CSB/Loan file	*/
#define	EMP_LOAN_HIS	91	/* Employee CSB/Loan history file	*/
#define	EMP_GARN	92	/* Employee garnishment	file 	*/
#define	EMP_GARN_HIS	93	/* Employee garnishment history	file	*/

#define	TEACH_ASS	94	/* Teacher assignment file	*/
#define	TEACH_QUAL	95	/* Teacher qualifications file	*/

#define	EMP_ATT		96	/* Employee attendance file 	*/

#define	TIME		97	/* Time file		*/
#define	TIME_HIS	98	/* Time history file	*/

#define	PP_EARN		99	/* Pay Period earnings file	*/
#define	PP_BEN		100	/* Pay Period benefit file	*/
#define	PP_DED		101	/* Pay Period deduction file	*/
#define	PP_LOAN		102	/* Pay Period CSB/Loan file	*/
#define	PP_GARN		103	/* Pay Period garnishment file	*/

#define	CHQ_MESS	104	/* Cheque message file	*/
#define	CHQ_MESS_ASS	105	/* Cheque message assignment file	*/

#define	REG_PEN_ADJ	106	/* Registered pension adjustment file	*/
#define	T4_REC		107	/* T4 field code file	*/
#define	GLACCT		108	/* G/L Payroll Account 			*/
#define	JR_ENT		109	/* Payroll Journal			*/
#define	JRH_ENT		110	/* Payroll Journal			*/
#define	AUD_PAY		111	/* Payroll Audit File			*/
#define	EC_REC		112	/* Ec File			*/
#define	CHQ_REG		113	/* Payroll Cheque Register	*/
#define	MAN_CHQ		114	/* Manual Cheque Register	*/
#define	ROE		115	/* Manual Cheque Register	*/
#define TMP_SCHED1	116	/* Temp. sched file */
#define SEN_PAR		117	/* Seniority param file */
#define EMP_SEN		118	/* Employee Seniority file */
#define TMP_SEN		119	/* Tmp Employee Seniority file */
#define GOV_PARAM	120	/* Govern. Seniority file */
#define T4_ADJ		121	/* T4 adjustment file */
#define RELIGION	122	/* Religion code file */
#define USERBARG	123	/* Userbarg security file */
#define TMP_EMP		124	/* Temp employee file */
#define TMP2_EMP	125	/* Temp2 employee file */
#define TS_SEN		126	/* Temp seniority file */
#define VC_ACC		127
#define EMP_COMP	128
#define COMP		129

#define USERPROF	130	/* User Profile file */

/*
*   NOTE: Whenever new file is added to system, give the number as TMPINDX_1
*	and change the TMPINDXs to next numbers, i.e. keep TMPINDXs always
*	at the end.
*/

#define	TMPINDX_1	131	/* Temporary File to build online sorts */
#define	TMPINDX_2	132	/* Temporary File to build online sorts */


#define	TOTAL_FILES	133	/* Last File + 1 (Files are started with 0) */


/*
*	File names
*/

#define	KEY_DESC	"bfs_keys.id"	/* Isam keys descriptor file */
#define	ERR_LOG		"errlog"	/* Isam/Dbh Errors Will be Written to
					   this file */
#define	FLDDEF_FILE	"flddefs.def"	/* Field Definitions File */
#ifndef	ORACLE
#define	JOURNAL		"jrnl"		/* To be appended by date */
#define BACK_UP		"backup"	/* backup directory name */
#define	PREV_YEAR	"prev_yr"	/* previous year directory */
#else
#define	PREV_YEAR	"py"		/* This is suffixed to cur. year tables
					   owner to make prev. year owner */
#endif

#define	PARAM_FILE	"param"		/* Parameters File */
#define	CONTROL_FILE	"control"	/* Fund/Co. Codes file */
#define	SCHOOL_FILE	"school"	/* School Codes file */

#ifdef ORACLE
#define	AUDIT_FILE	"dbhaudit"	/* Audit File */
#else
#define	AUDIT_FILE	"audit"		/* Audit File */
#endif

#define	GLMAST_FILE	"glmast"	/* G/L Master File */
#define	GSTDIST_FILE	"gstdist"	/* GST Distribution File */
#define	RECHDR_FILE	"rechdr"	/* Recurring Entry header File */
#define	RECTRAN_FILE	"rectran"	/* Recurring Entry Item File */
#define	GLTRHDR_FILE	"gltrhdr"	/* G/L Transaction header File */
#define	GLTRAN_FILE	"gltran"	/* G/L Transactions Item File */
#define	GLTRHDRNY_FILE	"gltrhdrny"	/* G/L Trans. header File new year */
#define	GLTRANNY_FILE	"gltranny"	/* G/L Trans. Item File new year */
#define	BDHDR_FILE	"glbdhdr"	/* G/L Budget Transaction header File */
#define	BDITEM_FILE	"glbditem"	/* G/L Budget Transactions Item File */

#define	STMAST_FILE	"stmast"	/* Stock master File */
#define	STTRAN_FILE	"sttran"	/* Stock Transaction File */
#define	ALLOC_FILE	"st_alloc"	/* Stock Allocation File */
#define	SECTION_FILE	"st_sect"	/* Stock Section File */

#define SUPPLIER_FILE   "supplier"	/* Supplier File */
#define POHDR_FILE	"pohdr"		/* Purchase Order Header File */
#define POITEM_FILE	"poitem"	/* Purchase Order Item File */

#define REQHDR_FILE	"reqhdr"	/* Requisition Header File */
#define REQITEM_FILE	"reqitem"	/* Requisition Item File */
#define REQREAS_FILE	"reqreas"	/* Requisition Reason File */



#define	CATEGORY_FILE	"category"	/* Category File */
#define	ITEMGROUP_FILE	"itemgroup"	/* Item Group File */
#define	CATALOGUE_FILE	"catalogue"	/* Catalogue File */
#define	POTBIDDER_FILE	"potbidder"	/* Potential Bidder File */
#define	BID_FILE	"bid"		/* Bid File */
#define	TENDHIST_FILE	"tendhist"	/* Tender History File */



#define FAMAST_FILE	"famast"	/* Fixed Asset Item Master File */
#define FATYPE_FILE	"fatype"	/* Fixed Asset Item Type File */
#define FADEPT_FILE	"fadept"	/* Fixed Asset Department File */
#define FATRAN_FILE	"fatran"	/* Fixed Asset Transfers File */

#define	CUST_FILE	"customer"	/* Customer File */
#define	ARSHDR_FILE	"arshdr"	/* ARS Invoices Header File */
#define	ARSITEM_FILE	"arsitem"	/* ARS Invoices Item File */
#define	RCPTHDR_FILE	"rcpthdr"	/* Customer Receipts Header File */
#define	RCPTITEM_FILE	"rcptitem"	/* Customer Receipts Item File */

#define	APINVOICE_FILE	"invoice"	/* Purchase Invoice Header File */
#define	APINHDR_FILE	"invchdr"	/* Purchase Invoice Header File */
#define	APINITEM_FILE	"invcitem"	/* Purchase Invoice Items File */
#define	CHEQUE_FILE	"cheques"	/* Cheques File */
#define CHQHIST_FILE	"chqhist"	/* Cheques History file */
#define CHQREG_FILE	"chqreg"	/* Cheques Register file */
#define APHIST_FILE	"aphist"	/* AP Invoice History file */

#define LASTPO_FILE	"lastpo"	/* Last PO file */
#define LASTREQ_FILE	"lastreq"	/* Last Requistion file */

#define	DEPARTMENT_FILE	"department"	/* Department File	*/
#define	AREA_FILE	"area"		/* Area File	*/
#define	POSITION_FILE	"position"	/* Position File	*/
#define	CLASS_FILE	"class"		/* Classification File	*/
#define	CLASS_ITEM_FILE	"class_item"	/* Classification item file	*/
#define	BARG_FILE	"barg_unit"	/* Bargaining unit file */
#define	BANK_FILE	"bank"		/* Bank code file	*/
#define	PAY_PERIOD_FILE	"pay_period"	/* Pay period code file	*/
#define	PAY_PER_IT_FILE	"pay_per_it"	/* Pay period item file	*/
#define	UIC_FILE	"uic_table"	/* UIC table file	*/
#define	TAX_FILE	"tax_table"	/* Income tax table file	*/
#define	CERT_FILE	"cert_table"	/* Certification table file	*/
#define	EARN_FILE	"earnings"	/* Earnings code file	*/
#define	TRANS_FILE	"trans"		/* Transaction code file	*/
#define	TRANS_ITEM_FILE	"trans_item"	/* Transaction item file	*/
#define	EXPENSE_FILE	"expense"	/* Expense code file	*/
#define	EXP_ITEM_FILE	"exp_item"	/* Expense item file	*/
#define	TERM_FILE	"term"		/* Termination code file      	*/
#define	STAT_HOL_FILE	"stat_hol"	/* Statutory Holiday	*/
#define	ATT_FILE	"att_code"	/* Attendance code file	*/
#define	INACT_CODE_FILE	"inact_code"	/* Inactivation code file	*/
#define	AREA_SPEC_FILE	"area_spec"	/* Area of special code file	*/
#define	LOAN_FILE	"loan"		/* CSB/Loan code file	*/
#define	PAY_PARAM_FILE	"pay_param"	/* Payroll parameter file	*/
#define	SALARY_FILE	"salary"	/* Salary account keys	*/

#define	BENEFIT_FILE	"benefit"	/* Benefit file		*/
#define	BEN_CAT_FILE	"ben_cat"	/* Benefit category file	*/
#define	DEDUCTION_FILE	"deduction"	/* Deduction file		*/
#define	DED_GRP_FILE	"ded_grp"	/* Deduction category file	*/
#define	DED_CAT_FILE	"ded_cat"	/* Deduction category file	*/
#define	REG_PEN_FILE	"reg_pen"	/* Registered pension plan file	*/

#define	EMPLOYEE_FILE	"employee"	/* Employee file		*/
#define	EMP_EMP_FILE	"emp_emp"	/* Employee employment file	*/
#define	EMP_SCHED1_FILE	"emp_sched1"	/* Employee schedule 1 file	*/
#define	EMP_SCHED2_FILE	"emp_sched2"	/* Employee schedule 2 file	*/
#define	EMP_EXTRA_FILE	"emp_extra"	/* Employee schedule extra file	*/
#define	EMP_EARN_FILE	"emp_earn"	/* Employee earnings file	*/
#define	EMP_INS_FILE	"emp_ins"	/* Employee ins earnings file	*/
#define	EMP_BEN_FILE	"emp_ben"	/* Employee benefit file	*/
#define	EMP_BN_HIS_FILE	"emp_bn_his"	/* Emp benefit history file	*/
#define	EMP_DED_FILE	"emp_ded"	/* Employee deduction file	*/
#define	EMP_DD_HIS_FILE	"emp_dd_his"	/* Emp deduction history file	*/
#define	EMP_LOAN_FILE	"emp_loan"	/* Employee CSB/Loan file	*/
#define	EMP_LN_HIS_FILE	"emp_ln_his"	/* Emp CSB/Loan history file	*/
#define	EMP_GARN_FILE	"emp_garn"	/* Employee garnishment	file 	*/
#define	EMP_GR_HIS_FILE	"emp_gr_his"	/* Emp garnishment history file	*/

#define	TEACH_ASS_FILE	"teach_ass"	/* Teacher assignment file	*/
#define	TEACH_QUAL_FILE	"teach_qual"	/* Teacher qualifications file	*/

#define	EMP_ATT_FILE	"emp_att"	/* Employee attendance file 	*/

#define	TIME_FILE	"time"		/* Time file		*/
#define	TIME_HIS_FILE	"time_his"	/* Time history file	*/

#define	PP_EARN_FILE	"pp_earn"	/* Pay Period earnings file	*/
#define	PP_BEN_FILE	"pp_ben"	/* Pay Period benefit file	*/
#define	PP_DED_FILE	"pp_ded"	/* Pay Period deduction file	*/
#define	PP_LOAN_FILE	"pp_loan"	/* Pay Period CSB/Loan file	*/
#define	PP_GARN_FILE	"pp_garn"	/* Pay Period garnishment file	*/

#define	CHQ_MESS_FILE	"chq_mess"	/* Cheque message file	*/
#define	CHQ_MS_ASS_FILE	"chq_ms_ass"	/* Cheque message assign file	*/

#define	RG_PEN_ADJ_FILE	"rg_pen_adj"	/* Reg pension adjustment file	*/
#define	T4_REC_FILE	"t4_rec"	/* T4 field code file	*/
#define	GLACCT_FILE	"glacct"	/* G/L payroll account file	*/
#define	JR_ENT_FILE	"jr_ent"	/* Journal entry file	*/
#define	JRH_ENT_FILE	"jrh_ent"	/* Payroll Journal		*/
#define	AUD_PAY_FILE	"aud_pay"	/* Payroll Audit File		*/
#define	EC_REC_FILE	"ec_rec"	/* Ec File			*/
#define	CHQ_REG_FILE	"chq_reg"	/* Payroll Cheque Register	*/
#define	MAN_CHQ_FILE	"man_chq"	/* Manual Cheque            */
#define	ROE_FILE	"roe"		/* Record of Employment	*/
#define	TMP_SCHED1_FILE	"tmp_sched1"	/* Temp schedule	*/
#define	SEN_PAR_FILE	"sen_par"	/* Seniority Param */
#define	EMP_SEN_FILE	"emp_sen"	/* Employee Seniority */
#define	TMP_SEN_FILE	"tmp_sen"	/* Temp Employee Seniority */
#define	GOV_PARAM_FILE	"gov_param"	/* Government Param  */
#define	T4_ADJ_FILE	"t4_adj"	/* T4 adjustments  */
#define	RELIGION_FILE	"religion"	/* Religion  */
#define	USERBARG_FILE	"userbarg"	/* Userbarg security */
#define	TMP_EMP_FILE	"tmp_emp"	/* Temp employee  */
#define	TMP2_EMP_FILE	"tmp2_emp"	/* Temp2 employee  */
#define	TS_SEN_FILE	"tmp_sen"	/* Temp seniority */
#define	VC_ACC_FILE	"vacacc"	/* Vacation Accrual File */
#define	EMP_COMP_FILE	"emp_comp"	/* Employee Competition File */
#define	COMP_FILE	"comp"		/* Competition File */

#define USERPROF_FILE	"userprof"	/* User Profile file */

#define	TMPIX_FILE_1	"gltmp"		/* Temporary Index file */
#define	TMPIX_FILE_2	"gltmp"		/* Second Temporary Index file */

#ifdef	ORACLE
#define	PASSWDPREFIX	"op"		/* Part of file storing Oracle passwd */
#endif

#ifndef	FILEST
extern	int	dberror ;
extern	int	iserror ;
extern	char	User_Id[];		/* defined in filein.h */
#ifdef	ORACLE
extern	char	UserPasswd[];		/* defined in filein.h */
#endif
#endif


/*---- various modes used while writing a record in a file ------------*/
/*---- Operations' values changed to render byte visible as ascii char */

#define	NOOP		000	/* Perform No Operation */
#define	BROWSE		040
#define	P_DEL		004
#define	UPDATE		002
#define	ADD		001

#ifdef	SECURITY
#define	DFLT_CHAR	0100	/* No Operations. To make printable char */
#endif

#ifndef	UNDEF

/* These Definitions are copied from isnames.h. Don't make any changes to
   these */
#define	UNDEF		-3
#define	DUPE		-5
#define	EFL		-10
#define	NOERROR		0
#define	ERROR		-100

#define	LOCKED		-50

#endif

#define	QUIT		-20
#define	PROFOM_ERR	-75
#define	REPORT_ERR	-80
#define	DBH_ERR		-300	/* seek/read/write error */
#define	NOACCESS	-150	/* user not allowed access: security reasons */

#define	FORWARD		0
#define	BACKWARD	1
#ifdef	ORACLE
#define	EQUAL		2
#endif


#define	ret(X)		{if((X) == PROFOM_ERR)return(PROFOM_ERR);}

/* file opening modes, executable files path and their extension */

#ifdef	MS_DOS

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define	RDMODE		(O_BINARY | O_RDONLY)
#define	WRMODE		(O_BINARY | O_WRONLY)
#define	RWMODE		(O_BINARY | O_RDWR)
#define	CRMODE		(O_BINARY | S_IREAD | S_IWRITE)

#define	TXT_RDMODE	(O_TEXT | O_RDONLY)
#define	TXT_WRMODE	(O_TEXT | O_WRONLY)
#define	TXT_RWMODE	(O_TEXT | O_RDWR)
#define	TXT_CRMODE	(O_TEXT | S_IREAD | S_IWRITE)

#else

#define	RDMODE		0
#define	WRMODE		1
#define	RWMODE		2
#define	CRMODE		0666

#define	TXT_RDMODE	0
#define	TXT_WRMODE	1
#define	TXT_RWMODE	2
#define	TXT_CRMODE	0666

#endif

#define	END_ARG		NULL	/* End argument for execl call */

/* Executable files extension */

#ifdef	MS_DOS
#define	EXTN		".exe"
#else
#define	EXTN		".out"
#endif

#ifdef	MAIN

char	PROG_NAME[11]	= "\0" ;	/* Program Name, which is being executed */
char	SYS_NAME[51]	= "\0" ;	/* Sub System Name */
char	CHNG_DATE[10]	= "\0" ;	/* Last Modification date of the Program */
char	dist_no[4]	= "\0" ;	/* District Number as passed in Cmnd line Arg */
char	CC_code[4]	= "\0" ;	/* Cost Center as passed in Cmnd line Arg */
short	CC_no		= -1;		/* Cost Center in numeric form	*/
char	terminal[9]	= "\0" ;	/* PROFOM Terminal Name	*/
int	mainfileno	=  -1  ;	/* Main DBH file# for a prog for security chk */

/* PATH NAME variables will be set to appropriate name in switch.c of DBH.
   Here except the WORK_DIR, all the Path names will be set including the '/'
   at the end. Generally work directory is used to change the current
   work directory, so that report files will be created there, instead of
   wherever you are now */

char	DATA_PATH[50]   = "\0" ;	/* data files directory path */
char	NFM_PATH[50]    = "\0" ;	/* nfm files directory path */
char	EXE_PATH[50]    = "\0" ;	/* Executable files path */
char	FMT_PATH[50]    = "\0" ;	/* Repgen Format files directory path */
char	CTOOLS_PATH[50] = "\0" ;	/* C-Tools Executables path */

char	WORK_DIR[50]  = "\0" ;	/* Work directory when system invoked */

#else

extern	char	PROG_NAME[11] ;
extern	char	SYS_NAME[51] ;
extern	char	CHNG_DATE[10];
extern	char	dist_no[4] ;
extern	char	CC_code[4] ;
extern	short	CC_no;
extern	char	terminal[9] ;
extern	int	mainfileno ;

extern	char	DATA_PATH[50] ;
extern	char	NFM_PATH[50] ;
extern	char	EXE_PATH[50] ;
extern	char	FMT_PATH[50] ;
extern	char	CTOOLS_PATH[50] ;

extern	char	WORK_DIR[50] ;

#endif

#define	VERSION_NO	"1.0"
#define	RELEASE_NO	"1.0.0"

/*
*	Run-time Switches. Derived from the main() arguments.
*
*	The switches are used as follows in G/L maintenance program.
*	The usage may not be same for other programs.
*
*	SW7 is always used to pass Company YES or NO.
*
*	SW8 and SW9 are used in DBH. If one of these switches is
*	ON, then data is taken form 'data/dist#/backup' or
*	'data/dist#/prev_yr' directories, corresponding to SW8 and SW9,
*	instaed of 'data/dist#' directory.
*/

#ifdef	MAIN
int	SW1,		/* Hospital */
	SW2,		/* Budget */
	SW3,		/* ??? */
	SW4,		/* Keys */
	SW5,		/* No Modification */
	SW6,		/* No Budget */
	SW7,		/* Companies */
	SW8,		/* Backed-up Data */ 
	SW9;		/* Previous Year Data */
#else
extern	int	SW1, SW2, SW3, SW4, SW5, SW6, SW7, SW8, SW9 ;
#endif

#define DAY	1
#define MONTH	2
#define YEAR	3

extern	int	errno ;
extern	long	lseek(),
		date_plus(),
		days(),
		get_date(),
		conv_date() ;
extern	char	*malloc(),
		*strcpy(),
		*strncpy(),
		*strcat(),
		*strncat() ;
extern	void	free() ;
#ifdef	ORACLE
long	get_maxsno() ;
#endif

#define	CHKACC(RET,MODE,MESG)	if((RET=CheckAccess(mainfileno,MODE,MESG))<0)\
					break
/*-----------------------E N D   O F   F I L E--------------------------------*/

