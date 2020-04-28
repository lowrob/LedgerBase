
/*
*    Source Name : bfs_defs.h
*    System      : Budgetary Financial System.
*
*    Created On  : 2nd May 1989.
*
*    Contains Defined Constants used by system.
*/

/*#define float double*/
/* define ENGLISH or FRENCH for Corresponding Versions  */

#define ENGLISH  		 /* English Version */

#ifdef	UNUSED
#define	FRENCH			 /* French Version */
#endif

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

#ifndef ORACLE

/*
*	DBH journalling is active
*/
#define	JRNL

#endif

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
#define	RECHDR		5	/* Recurring Entry Header File */
#define	RECTRAN		6	/* Recurring Entry Items File */
#define	GLTRHDR		7	/* G/L Trans Header File */
#define	GLTRAN		8	/* G/L Trans Items File */
#define	GLBDHDR		9	/* Budget transaction header file */
#define	GLBDITEM	10	/* Budget transaction item file  */

#define	STMAST		11	/* Stock Master File */
#define	STTRAN		12	/* Stock Transaction File */
#define	ALLOCATION	13	/* Stock Allocation File */
#define	SECTION		14	/* Stock section File */

#define SUPPLIER	15      /* Supplier File */
#define POHDR		16	/* Purchase Order Header File */
#define POITEM		17	/* Purchase Order Item File */

#define	FAMAST		18	/* Fixed Assets Item Master */
#define	FATYPE		19	/* Fixed Asset Item Types */
#define	FADEPT		20	/* Fixed Asset Dept Codes */
#define FATRAN		21	/* Fixed Asset Transfers */

#define	CUSTOMER	22	/* Customer Master File */
#define	ARSHDR		23	/* Sales Invoice header file */
#define ARSITEM		24	/* Sales Invoice item file */
#define	RECEIPTS	25	/* Customer Receipts File */

#define	APINVOICE	26	/* Purchase Invoices File */
#define	APINHDR		27	/* Purchase Invoice Header */
#define	APINITEM	28	/* Purchase Invoice Items */
#define	CHEQUE		29	/* Cheques File */
#define CHQHIST		30	/* Cheques History File */
#define CHQREG		31	/* Cheques History File */
#define	APHIST		32	/* AP Invoice History File */

#define USERPROF	33	/* User Profile file */

/*
*   NOTE: Whenever new file is added to system, give the number as TMPINDX_1
*	and change the TMPINDXs to next numbers, i.e. keep TMPINDXs always
*	at the end.
*/

#define	TMPINDX_1	34	/* Temporary File to build online sorts */
#define	TMPINDX_2	35	/* Temporary File to build online sorts */


#define	TOTAL_FILES	36	/* Last File + 1 (Files are started with 0) */


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
#define	RECHDR_FILE	"rechdr"	/* Recurring Entry header File */
#define	RECTRAN_FILE	"rectran"	/* Recurring Entry Item File */
#define	GLTRHDR_FILE	"gltrhdr"	/* G/L Transaction header File */
#define	GLTRAN_FILE	"gltran"	/* G/L Transactions Item File */
#define	BDHDR_FILE	"glbdhdr"	/* G/L Budget Transaction header File */
#define	BDITEM_FILE	"glbditem"	/* G/L Budget Transactions Item File */

#define	STMAST_FILE	"stmast"	/* Stock master File */
#define	STTRAN_FILE	"sttran"	/* Stock Transaction File */
#define	ALLOC_FILE	"st_alloc"	/* Stock Allocation File */
#define	SECTION_FILE	"st_sect"	/* Stock Section File */

#define SUPPLIER_FILE   "supplier"	/* Supplier File */
#define POHDR_FILE	"pohdr"		/* Purchase Order Header File */
#define POITEM_FILE	"poitem"	/* Purchase Order Item File */

#define FAMAST_FILE	"famast"	/* Fixed Asset Item Master File */
#define FATYPE_FILE	"fatype"	/* Fixed Asset Item Type File */
#define FADEPT_FILE	"fadept"	/* Fixed Asset Dept Code File */
#define FATRAN_FILE	"fatran"	/* Fixed Asset Transfers File */

#define	CUST_FILE	"customer"	/* Customer File */
#define	ARSHDR_FILE	"arshdr"	/* ARS Invoices Header File */
#define	ARSITEM_FILE	"arsitem"	/* ARS Invoices Item File */
#define	RCPT_FILE	"receipt"	/* Customer Receipts File */

#define	APINVOICE_FILE	"invoice"	/* Purchase Invoice Header File */
#define	APINHDR_FILE	"invchdr"	/* Purchase Invoice Header File */
#define	APINITEM_FILE	"invcitem"	/* Purchase Invoice Items File */
#define	CHEQUE_FILE	"cheques"	/* Cheques File */
#define CHQHIST_FILE	"chqhist"	/* Cheques History file */
#define CHQREG_FILE	"chqreg"	/* Cheques Register file */
#define APHIST_FILE	"aphist"	/* AP Invoice History file */

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
#include <errno.h>

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

char	PROG_NAME[11] = "\0" ;	/* Program Name, which is being executed */
char	SYS_NAME[51]  = "\0" ;	/* Sub System Name */
char	CHNG_DATE[10] = "\0" ;	/* Last Modification date of the Program */
char	dist_no[4]    = "\0" ;	/* District Number as passed in Cmnd line Arg */
char	terminal[9]   = "\0" ;	/* PROFOM Terminal Name */
int	mainfileno    =  -1  ;	/* Main DBH file# for a prog for security chk */

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

extern	char	PROG_NAME[] ;
extern	char	SYS_NAME[] ;
extern	char	CHNG_DATE[];
extern	char	dist_no[] ;
extern	char	terminal[] ;
extern	int	mainfileno ;

extern	char	DATA_PATH[] ;
extern	char	NFM_PATH[] ;
extern	char	EXE_PATH[] ;
extern	char	FMT_PATH[] ;
extern	char	CTOOLS_PATH[] ;

extern	char	WORK_DIR[] ;

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
	SW8;		/* Backed-up Data */ 
	SW9;		/* Previous Year Data */
#else
extern	int	SW1, SW2, SW3, SW4, SW5, SW6, SW7, SW8, SW9 ;
#endif

#define DAY	1
#define MONTH	2
#define YEAR	3

//extern	int	errno ;
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

