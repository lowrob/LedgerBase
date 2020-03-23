/*-----------------------------------------------------------------------
Source Name: emp_emp2.c 
System     : Ledger Base 
Module     : Personnel/Payroll
Created  On: 1991/10/18  
Created  By: Cathy Burns 

DESCRIPTION:
	Program to Change/Inquire the Employment Information.
	This Program also writes Audit records for the changes.

Usage of SWITCHES when they are ON :

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
------------------------------------------------------------------------*/

#define  MAINFL		EMPLOYEE  		/* main file used */


#include <stdio.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_pp.h>
#include <bfs_com.h>
#include <empldrvr.h>

#define	SYSTEM		"PESONNEL/PAYROLL"	/* Sub System Name */
#define	MOD_DATE	"18-OCT-91"		/* Program Last Modified */


/* PROFOM Field Numbers */
#define	LAST_SNUM	12

/* User Interface define constants */
#ifdef ENGLISH
#define ADDREC		'A'
#define CHANGE		'C'
#define DELETE		'D'
#define INQUIRE		'I'
#define EXITOPT		'E'
#define	NEXT		'N'
#define	PREV		'P'

#define	YES		'Y'
#define NO		'N'
#define ADDITEMS	'A'
#define HEADEREDIT	'H'
#define	LINEEDIT	'L'
#define SCREENEDIT	'S'
#define DELITEM		'D'
#define REACTITEM	'R'
#define	CANCEL		'C'

#define ACTIVE		"ACT"
#define INACTIVE	"DEL"
#else
#define ADDREC		'R'
#define CHANGE		'C'
#define DELETE		'E'
#define INQUIRE		'I'
#define EXITOPT		'F'
#define	NEXT		'S'
#define	PREV		'P'

#define	YES		'O'
#define NO		'N'
#define ADDITEMS	'A'
#define HEADEREDIT	'H'
#define	LINEEDIT	'L'
#define SCREENEDIT	'S'
#define DELITEM		'D'
#define REACTITEM	'R'
#define	CANCEL		'A'

#define ACTIVE		"ACT"
#define INACTIVE	"ELI"
#endif

/* PROFOM Releted declarations */

#define PRGNM  		"emp_emp2"
#define SCR_NAME	"emp_emp2"

#define MAX_FIELD	17		/* Maximum field # to edit */

/* PROFOM Field Numbers */

#define FN_FLD		400	/* Fn: */

#define KEY_START	500	/* Key Start Field */
#define KEY_END		500	/* Key End Field */
#define CHG_FLD		700

#define HDR_ST_FLD	900	/* Desc. Field */
#define HDR_END_FLD	1500	/* Units/Year Field */

#define PAGE_FLD	1600	/* Page# Field */

#define	ITEM_ST_FLD	1800	/* Item Start Field */
#define ITEM_END_FLD	5700	/* Item End Field */

#define	END_FLD		5900	/* Last Field of the screen */
#define	STATUS_FLD	1750	/* Offset of Status Field */
#define	STEP		2000	/* Diff in Line */

#define	BARG_FLD	900
#define	MNCLASS_FLD	1100
#define	POS_FLD		1300

#define	WEEK_FLD	1800
#define	FUND_FLD	1900
#define	CLASS_FLD	2000
#define	CC_FLD		2300
#define	UNIT7_FLD	3100
#define	DEPT_FLD	3200
#define	AREA_FLD	3400
#define	RATE_FLD	3600
#define	AMT_FLD		3700
#define RATE_FLD2	5700
#define	AMT_FLD2	5800

/* Profom 'C' structure for the screen "schl1" i.e., for Demographic data */

#define	PAGESIZE	2

typedef struct	{

	short	s_week;			/* 1800-3700 Certification 99 */
	short	s_fund;			/* 1900-3800 Certification 99 */
	char	s_class[7];		/* 2000-3900 Position */
	char	s_classdesc[31];	/* 2100-4000 Position Desc */
	char	s_dummy1[2];		/* 2150-4050 dummy */
	short	s_cc;			/* 2900-4800 Cost Center */
	char	s_ccdesc[31];		/* 3000-4900 Cost Center Desc */
	double	s_units[7];		/* 2200-2800 4100-4700 Units 99.99 */
	char	s_dept[7];		/* 3100-5000 Department */
	char	s_deptdesc[31];		/* 3200-5100 Department Desc */
	char	s_area[7];		/* 3300-5200 Area */
	char	s_areadesc[31];		/* 3400-5300 Area Desc */
	char	s_status[4];		/* 2850-4750 Status */
	double	s_rate;			/* 3500-5400 Rate     999,999.99 */
	double	s_amount;		/* 3600-5500 Amount 9,999,999.99 */

	} S_item;

typedef struct	{

	char	s_pgm[11];		/* 100 Program Name String 10	*/ 
	long	s_sysdate;		/* 300 System date DATE YYYYFMMFDD */
	char	s_fn[2];		/* 400 Fn: field String 1	*/
	char	s_emp[13];		/* 500 Employee No String 12 */
	char	s_name[31];		/* 600 Employee Name String 30*/
	short	s_field;		/* 700 Field Number Numeric 99	*/
	char	s_barg[7];		/* 900 Bargaining unit */
	char	s_bargdesc[31];		/* 1000 Bargaining Unit Desc */
	char	s_classcd[7];		/* 1100 Class Code */
	char	s_classdsc[31];		/* 1200 Class Desc */
	char	s_position[7];		/* 1300 Position */
	char	s_posdesc[31];		/* 1400 Position Desc */
	double	s_percent;		/* 1500 Percentage 999.99 */
	char	s_dummy[2];		/* 1600 Prompt for separator */
	short	s_page;			/* 1700 Page No 99 */
	S_item	s_items[PAGESIZE];	/* 1800 - 5500 */
	char	s_mesg[78];		/* 5600 Message Line String 77	*/
	char	s_opt[2];		/* 5700 Message Option String X */ 

	} s_struct;

static  Emp_sched1	emp_sched, pre_emp_sched;
static	Position	pos_rec;
static	Barg_unit	barg_rec;
static	Class		class_rec;
static	Sch_rec		sch_rec;
static	Dept		dept_rec;
static	Area		area_rec;
static	Pay_per		pay_period;
static	Pa_rec		pa_rec;
static  Pay_param	param;
static  Cert		cert;

typedef struct Page {
	S_item	Items[PAGESIZE] ;	/* Items Information */
	struct	Page	*PrevPage ;	/* ptr to previous page */
	struct	Page	*NextPage ;	/* ptr to next page */
	char	I_Status[PAGESIZE][2];	/* item status ie A(DD) C(hange) */
	short	NoItems;		/* number of Items on the page */
	short	Pageno;			/* Page number */
}	Page;

static	Page	*FirstPage,		/* Address of First Page */
		*CurPage,		/* Address of Current Page */
		*CurLast,		/* Address of Curr. record last page */
		*LastPage;		/* Address of Last Page of Memory
					   Allocated */

extern	double	D_Roundoff();
void	free() ;
char	*malloc() ;
static	s_struct	s_sth, image ;

static	int	Validation() ;
static	int	WindowHelp() ;
static	long	effective_date;

Employment1()
{
	int	err ;

	err = InitScreen() ; 	/* Initialization routine */

	if(emp_rec.em_numb[0] != '\0') {
		flg_reset(EMPLOYEE);

		err = get_n_employee(&emp_rec, BROWSE, 0, FORWARD, e_mesg);
#ifndef ORACLE
		seq_over(EMPLOYEE);
#endif
		if(ERROR == err)return(DBH_ERR) ;
		if(EFL == err) {
			fomen("No More Records....");
			get();
			return(NOERROR) ;
		}


		err = UsrBargVal(BROWSE,emp_rec.em_numb,emp_rec.em_barg,1,e_mesg);
		if(err < 0){
			DispError((char *)&s_sth,e_mesg);
		}
		else {
			strcpy( s_sth.s_emp,emp_rec.em_numb);
			err = ShowScreen();
		}
	}

	err = Process(); 	/* Initiate Process */

	return( err ) ;

} /* DemoGraph() */
/*--------------------------------------------------------------------------*/
/* Close nessary files and environment before exiting program               */
static int
CloseRtn() 
{
	/* Free the linked list for the end */
	for( ;LastPage != FirstPage ; ) {
		LastPage = LastPage->PrevPage;
		free((char *)LastPage->NextPage);
		LastPage->NextPage = NULL;
	}
	if(FirstPage != NULL) {
		free((char *)FirstPage);
	}

	FirstPage = LastPage = NULL;

	/* Set terminal back to normal mode from PROFOM */
	fomcs();
	fomrt();

	close_dbh();	/* Close files */

	return(NOERROR);
}	/* CloseRtn() */
/*----------------------------------------------------------------*/
/* Initialize screen before going to process options */
static int
InitScreen()
{
	int	err ;

	FirstPage = NULL;
	LastPage = NULL;

	/* move screen name to Profom status structure */
	strcpy(sr.scrnam,NFM_PATH);
	strcat(sr.scrnam,SCR_NAME) ;

	strcpy(s_sth.s_pgm,PROG_NAME);

	s_sth.s_sysdate = get_date();	/* get Today's Date in YYYYMMDD format*/
	s_sth.s_name[0] = HV_CHAR ;
	s_sth.s_field = HV_SHORT ;
	s_sth.s_mesg[0] = HV_CHAR ;
	s_sth.s_opt[0] = HV_CHAR ;

	s_sth.s_items[0].s_dummy1[0] = HV_CHAR;	
	s_sth.s_items[1].s_dummy1[0] = HV_CHAR;	
	/* Move High Values to data fields and Display the screen */
	err = ClearScreen() ;
	if(NOERROR != err) return(err) ;

	/*
	*	Get The Parameter Record
	*/
	err = get_pay_param(&param, BROWSE, 1, e_mesg) ;
	if(err == ERROR) {
		DispError((char *)&s_sth,e_mesg);
		return(ERROR) ;
	}
	else if(err == UNDEF) {
#ifdef ENGLISH
		DispError((char *)&s_sth,"Parameters Are Not Set Up ...");
#else
		DispError((char *)&s_sth,"Parametres ne sont pas etablis... ");
#endif
		return(ERROR) ;
	}

	/*
	*	Get The Parameter Record
	*/
	err = get_param(&pa_rec, BROWSE, 1, e_mesg) ;
	if(err == ERROR) {
		DispError((char *)&s_sth,e_mesg);
		return(ERROR) ;
	}
	else if(err == UNDEF) {
#ifdef ENGLISH
		DispError((char *)&s_sth,"Parameters Are Not Set Up ...");
#else
		DispError((char *)&s_sth,"Parametres ne sont pas etablis... ");
#endif
		return(ERROR) ;
	}

	return(NOERROR) ;
}	/* InitScreen() */
/*-------------------------------------------------------------------*/
/* Get Option from user and call corresponding function */
static	int
Process()
{
	int	err;

	for( ; ; ){
		/* Get the Fn: from the user */
		if((err = ReadFunction()) != NOERROR) return(err) ;

		err = ProcFunction() ;	/* Process Fn */

		if(QUIT == err)		return(NOERROR) ;	/* Exit */
		if(JUMP == err)		return(JUMP) ;
		if(NOACCESS == err) {
			fomen(e_mesg);
			get();
		}
		if(PROFOM_ERR == err)	return(PROFOM_ERR);  /* PROFOM ERROR */
		if(DBH_ERR == err) {
			DispError((char*)&s_sth, e_mesg);
#ifdef	ENGLISH
			sprintf(e_mesg,"%s %d Dberror: %d Errno: %d",
				"System Error... Iserror:",
				iserror, dberror, errno);
#else
			sprintf(e_mesg,"%s %d Dberror: %d Errno: %d",
				"Erreur du systeme... Iserror:",
				iserror, dberror, errno);
#endif
			DispError((char*)&s_sth, e_mesg);
			return(DBH_ERR);	/* DBH ERROR */
		}
	}      /*   end of the for( ; ; )       */
}	/* Process() */
/*----------------------------------------------------------------*/
/* Display the Function (Fn:) options and get the option from the user */
static int
ReadFunction()
{
	/* Display Fn: options */
#ifdef ENGLISH
	fomer("C(hange), I(nquire), F(orward), B(ack), N(ext), P(rev), S(creen), E(xit)");
#else
	fomer("R(ajouter), C(hanger), I(nterroger), S(uivant), P(recedent), F(in)");
#endif
	/* Read Fn: field to get the option */
	sr.nextfld = FN_FLD ;
	fomrf( (char *)&s_sth );
	ret(err_chk(&sr));	/* Check for PROFOM error */

	return(NOERROR) ;
}	/* ReadFunction() */
/*----------------------------------------------------------------*/
/* Process the user selected Fn: option */
static int
ProcFunction()
{
	int retval;

	switch (s_sth.s_fn[0]) {
/*
	case ADDREC  :			/ * ADD * /
		CHKACC(retval,ADD,e_mesg);
		return( Add() ) ;
*/
	case CHANGE  :			/* CHANGE */
		CHKACC(retval,UPDATE,e_mesg);
		return( Change() ) ;
	case INQUIRE  :			/* Inquire */
		CHKACC(retval,BROWSE,e_mesg);
		return( Inquire() ) ;
	case NEXT_RECORD  :			/* Inquire */
		CHKACC(retval,BROWSE,e_mesg);
		return( Next(FORWARD) ) ;
	case PREV_RECORD  :			/* Inquire */
		CHKACC(retval,BROWSE,e_mesg);
		return( Next(BACKWARD) ) ;
	case NEXT_SCR  : 	/* Next Screen */
		Cur_Option += 1 ;	/* Set to Next Screen */
		return( JUMP ) ; 
	case PREV_SCR  : 	/* Next Screen */
		Cur_Option -= 1 ;	/* Set to Next Screen */
		return( JUMP ) ; 
	case SCREEN  :			/* Inquire */
		CHKACC(retval,BROWSE,e_mesg);
		return( Screen() ) ;
	case EXITOPT  :			/* Exit */
		CHKACC(retval,BROWSE,e_mesg);
		return( QUIT ) ;
	}  /*   end of the switch statement */

	return(retval);
}	/* ProcFunction() */
/*----------------------------------------------------------------------*/
/* Adding.  Get the unique Key, accept details and update the files */
static int
Add()
{
	int	err ;

	FirstPage = NULL;
	LastPage = NULL;
	CurLast = NULL;
	CurPage = NULL;

	err = ReadKey();
	if(err != NOERROR) return(err) ;

	/* Clear The Screen */
	err = ClearScreen();
	if(err != NOERROR) return(err) ;

	err = GetDetails() ;
	if(PROFOM_ERR == err || DBH_ERR == err) return(err) ;
	if(err < 0 || CANCEL == err) {
		roll_back(e_mesg) ;	/* Unlock the locked Records */
		return(ClearScreen()) ;	/* Clear the Screen */
	}

	return(NOERROR);
}	/* Add() */
/*-----------------------------------------------------------------------*/
/* Change. Students Study halls and update the files if a day/semester   */
/* is changed to NO delete record.			  		 */
/*-----------------------------------------------------------------------*/
static int
Change()
{
	int	err ;

	err = SelectRecord(UPDATE) ;
	if(NOERROR != err) return(err);

	for( ; ; ) {
		err = ConfirmItems() ;
		if(err != YES) {
			roll_back(e_mesg);
			break;
		}

		err = ProcItemUpdates(UPDATE) ;
		if(err==NOERROR) break;
		if(err==LOCKED) {
			roll_back(e_mesg) ;
			continue;
		}
		if (err < 0) {
			roll_back(e_mesg);
			return(err) ;
		}
	}

	return(NOERROR) ;
}	/* Change() */
/*-----------------------------------------------------------------------*/
/* Show Student Student halls base on users input */
static int
Inquire()
{
	int	err ;

	err = SelectRecord(BROWSE) ;
	if(NOERROR != err) return(err) ;

	err = ConfirmItems() ;
	
	return(NOERROR) ;
}	/* Inquire() */
/*----------------------------------------------------------*/
/* Show the next or previous Students Study halls */
static int
Next(direction)
int	direction ;
{
	int err;

	if(flg_start(EMPLOYEE) != direction) {
		strcpy(emp_rec.em_numb,s_sth.s_emp);
		inc_str(emp_rec.em_numb,sizeof(emp_rec.em_numb)-1,direction);
		flg_reset(EMPLOYEE);
	}
	for(;;) {
		err = get_n_employee(&emp_rec,BROWSE,0,direction,e_mesg);
#ifndef ORACLE
		seq_over(EMPLOYEE);
#endif

		if(ERROR == err) return(DBH_ERR);
		if(EFL == err) {
#ifdef ENGLISH
			fomen("No More Records....");
#else
			fomen("Plus de fiches....");
#endif
			get();
			flg_reset(EMPLOYEE);
			return(NOERROR);
		}

		err = UsrBargVal(BROWSE,emp_rec.em_numb,emp_rec.em_barg,
			0,e_mesg);
		if(err < 0){
			continue;
		}
		else{
			break;
		}
	}
	strcpy(s_sth.s_emp,emp_rec.em_numb);
	return(ShowScreen());
}	/* Next() */
/*----------------------------------------------------------------------*/
/* Get the Student Study Hall key from user. In ADD mode disable dup buffers, */
/* other modes enable dup buffers and show the current key as a default key */
/*----------------------------------------------------------------------*/
static int
ReadKey()
{
	int	i;
	
	/* In Add mode turn off dup control for key fields.
	   Other modes reverse it */
	if(s_sth.s_fn[0] == ADDREC){	/* ADD */
		SetDupBuffers(KEY_START,KEY_END,0); /* Off Dup Control */
	}
	else {
		SetDupBuffers(KEY_START,KEY_END,1);
	}

#ifdef ENGLISH
	strcpy(s_sth.s_mesg,"Press ESC-F to Go Back to Fn:");
#else
	strcpy(s_sth.s_mesg,"Appuyer sur ESC-F pour retourner a Fn:");
#endif
	DispMesgFld((char *)&s_sth);

	s_sth.s_emp[0] = LV_CHAR;

	i = ReadFields((char *)&s_sth,KEY_START, KEY_END,
		Validation, WindowHelp,1) ;
	if(PROFOM_ERR == i || DBH_ERR == i) return(i) ;
	if(RET_USER_ESC == i){
		
		ret( WriteFields((char *)&s_sth,KEY_START, KEY_END) ) ;

		s_sth.s_mesg[0] = HV_CHAR;
		DispMesgFld((char *)&s_sth);

		return(RET_USER_ESC) ;
	}

	s_sth.s_mesg[0] = HV_CHAR;
	DispMesgFld((char *)&s_sth);

	return(NOERROR);
}	/*  ReadKey() */
/*------------------------------------------------------------*/
/* Read the Area Details from the User */
static int
GetDetails()
{
	int	i ;

	i = ReadHdr(ADD);
	if(NOERROR != i) return(i) ;

	i = AddItems();
	if(NOERROR != i) return(i) ;

	for( ; ; ) {
		i = ConfirmItems() ;
		if(i != YES) break;

		i = ProcItemUpdates(ADD) ;
		if(i < 0) {
			if(i == LOCKED) continue;
			break;
		}
		break;
	}
	if(i != NOERROR) return(i);
	return(NOERROR) ;
}	/* GetDetails() */
/*------------------------------------------------------------*/
static int
ReadHdr(mode)
int	mode;
{
	int	retval;

	if(mode == ADD) {
		SetDupBuffers(HDR_ST_FLD,HDR_END_FLD,0);
	}
	else {
		SetDupBuffers(HDR_ST_FLD,HDR_END_FLD,1);
	}

#ifdef ENGLISH
	strcpy(s_sth.s_mesg,"Press ESC-F to Go Back to Fn:");
#else
	strcpy(s_sth.s_mesg,"Appuyer sur ESC-F pour retourner a Fn:");
#endif
	DispMesgFld((char *)&s_sth);

	InitHdr(LV_CHAR,LV_SHORT,LV_DOUBLE);

	retval = ReadFields((char *)&s_sth,HDR_ST_FLD,HDR_END_FLD,
		Validation,WindowHelp,1) ;
	if(PROFOM_ERR == retval || DBH_ERR == retval) return(retval) ;
	if(RET_USER_ESC == retval) {
		if(mode == ADD) {
			InitHdr(HV_CHAR,HV_SHORT,HV_DOUBLE);
			WriteFields((char *)&s_sth,HDR_ST_FLD,HDR_END_FLD);
		}
		retval = CopyBack((char *)&s_sth,(char *)&image,sr.curfld, HDR_END_FLD);
		if(retval == PROFOM_ERR) return(retval);

	     	s_sth.s_mesg[0] = HV_CHAR ;
	     	DispMesgFld((char *)&s_sth);

		return(RET_USER_ESC) ;
	}

	return(NOERROR);
}
/*------------------------------------------------------------*/
/* Read Item Details from the User */
static int
AddItems()
{
	int	i, err ;

	/* If the last node of po is Partial filled then Show Page */
	if(CurLast != NULL && CurLast->NoItems < PAGESIZE ) {
		ret( ShowItems(CurLast) ) ;
		i = CurLast->NoItems ;
		CurPage = CurLast ;
	}
	else {
		/* Calculate the page# */
		if(CurLast != NULL) {
			i = PAGESIZE ;
			CurPage = CurLast ;
		}
		else {
			strcpy(s_sth.s_dummy,"--");
			s_sth.s_page = 1 ;
			ret( WriteFields((char *)&s_sth,PAGE_FLD,PAGE_FLD+100));
			i = 0 ;
		}
	}


	for( ; ; ) {
		if( PAGESIZE == i) {	/* Page Full */

			/* Calculate the page# */
			s_sth.s_page = CurLast->Pageno + 1 ;

			ret( WriteFields((char *)&s_sth,PAGE_FLD, 
				(END_FLD - 200)) ) ;

			InitItem(HV_CHAR,HV_SHORT,HV_DOUBLE,1);
			WriteFields((char *)&s_sth,ITEM_ST_FLD,ITEM_END_FLD);

			i = 0 ;
		}

		err = ReadItem(i,ADD) ;		/* Read Each Item Line */
		if(PROFOM_ERR == err || DBH_ERR == err) return(err) ;
		if(NOERROR != err) break ;	/* ESC-F */

		if(0 == i)	/* First Item in the Page */
			if((err = MakeFreshPage()) < 0) return(err) ;
		
		/* Copy the Item to List */
		scpy((char*)&(CurPage->Items[i]), (char*)&(s_sth.s_items[i]),
			sizeof(S_item)) ;

		CurPage->I_Status[i][0] = ADDITEMS;

		i++ ;

		CurPage->NoItems = i;
	}
	if(i == 0) 
		if((err=ShowItems(CurPage))<0) return(err) ;

	return(NOERROR) ;
}	/* AddItems() */
/*-----------------------------------------------------------------------*/
/*	Get the next node in linked list to add invoice items. If the
*	(Cur. invc last page) = (Last Page in linked List) or no
*	nodes in list, allocate node and add to linked list
*/
static int
MakeFreshPage()
{
	Page	*tempptr ;

	/* If, no node is allocated yet or Current invoice used all the nodes,
	   then allocate new node */

	if( LastPage == NULL || CurLast == LastPage ){
		tempptr= (Page *)malloc((unsigned)sizeof(Page)) ;

		if( tempptr == NULL ){
			DispError((char*)&s_sth,"Memory Allocation Error");
			return(ERROR);
		}
		tempptr->NextPage = NULL ;

		if( LastPage == NULL ){	/* No node is allocated Yet */
			tempptr->PrevPage = NULL ;
			tempptr->Pageno = 1 ;
			FirstPage = tempptr ;
		}
		else {				/* Not a first node in list */
			tempptr->Pageno = LastPage->Pageno + 1 ;
			LastPage->NextPage = tempptr ;
			tempptr->PrevPage = LastPage ;
		}
		LastPage = tempptr ;
	}

	if(CurLast == NULL)
		CurLast = FirstPage ;
	else
		CurLast = CurLast->NextPage ;

	CurLast->NoItems = 0 ;
	CurPage = CurLast ;

	return(NOERROR);
}	/* MakeFreshPage() */
/*----------------------------------------------------------*/
/* Get the key and show the record */
static int
SelectRecord(mode)
int	mode;
{
	int	err ;

	err = ReadKey();
	if(err != NOERROR) return(err) ;

	strcpy(emp_rec.em_numb,s_sth.s_emp);
	err = get_employee(&emp_rec,mode,0,e_mesg);
	if(err < 0) {
		DispError((char *)&s_sth,e_mesg);
		return(err);
	}

	err = ShowScreen();
	if(err < 0) {
		return(err);
	}

	return(NOERROR);
}	/* SelectRecord() */
/*------------------------------------------------------------*/
static int
ShowScreen()
{
	int	err ;


	err = ShowHdr();
	if(err < 0) {
		return(err);
	}

	/* Get area records i.e. Build list */
	err = BuildList();
	if(err < 0 && err != EFL) {
		return(err);
	}

	err = ShowItems(CurPage);
	if(err < 0) {
		return(err);
	}
	
	return(NOERROR);
}
/*------------------------------------------------------------*/
static int
BuildList()
{
	int retval;
	int i;
	int j;
	double	temp_calc, total_units;
	CurLast = CurPage = NULL;
	i = 0;

	
	strcpy(emp_sched.es_numb,emp_rec.em_numb);
	emp_sched.es_week = 0;
	emp_sched.es_class[0] = '\0';
	emp_sched.es_fund = 0;
	flg_reset(EMP_SCHED1);

	for( ; ; ) {
		retval = get_n_emp_sched1(&emp_sched,BROWSE,0,FORWARD,e_mesg);
		if(retval < 0) {
			if(retval == EFL) break;
			DispError((char *)&s_sth,e_mesg);
			return(retval);
		}

		if(strcmp(emp_sched.es_numb,emp_rec.em_numb)!=0) { 
			break;
		}

		if(PAGESIZE == i) i = 0;
		if(i == 0) {
			if((retval = MakeFreshPage()) < 0) return(retval);
		}

		CurPage->Items[i].s_week = emp_sched.es_week;
		CurPage->Items[i].s_fund = emp_sched.es_fund;
		strcpy(CurPage->Items[i].s_class, emp_sched.es_class);		
		retval = Read_Class(CurPage->Items[i].s_class);
		if(retval < 0)
			strcpy(CurPage->Items[i].s_classdesc,
				"** Not Set up **");
		else
			strcpy(CurPage->Items[i].s_classdesc,
				class_rec.c_desc);
		if(class_rec.c_units != 0){
	   		temp_calc =
				class_rec.c_yrly_inc/class_rec.c_units; 
	  		CurPage->Items[i].s_rate =temp_calc;
		}
		else
	  		CurPage->Items[i].s_rate = 0;
		strcpy(CurPage->Items[i].s_dummy1, " ");
		CurPage->Items[i].s_cc = emp_sched.es_cost;	
		retval = Read_CostCenter(CurPage->Items[i].s_cc);
		if(retval < 0)
			strcpy(CurPage->Items[i].s_ccdesc, "** Not Set up **");
		else
			strcpy(CurPage->Items[i].s_ccdesc, sch_rec.sc_name);
		strcpy(CurPage->Items[i].s_dept, emp_sched.es_dept);		
		retval = Read_Dept(CurPage->Items[i].s_dept);
		if(retval < 0)
			strcpy(CurPage->Items[i].s_deptdesc, "** Not Set up **");
		else
			strcpy(CurPage->Items[i].s_deptdesc, dept_rec.d_desc);
		strcpy(CurPage->Items[i].s_area, emp_sched.es_area);		
		retval = Read_Area(CurPage->Items[i].s_area,CurPage->Items[i].s_dept);
		if(retval < 0)
			strcpy(CurPage->Items[i].s_areadesc, "** Not Set up **");
		else
			strcpy(CurPage->Items[i].s_areadesc, area_rec.a_desc);
		CurPage->Items[i].s_amount = emp_sched.es_amount;	
		for(j=0; j < 7; j++) {
			CurPage->Items[i].s_units[j] = emp_sched.es_units[j];	
			total_units += emp_sched.es_units[j];
		}
		strcpy(CurPage->Items[i].s_status,ACTIVE);

		CurPage->I_Status[i][0] = ' ';
		CurPage->NoItems++;
		i++;
	}
	seq_over(EMP_SCHED1);


	if(CurLast != NULL) {
		CurPage = FirstPage;
	}

	if(retval == EFL) return(retval);
	return(NOERROR);
}
/*------------------------------------------------------------*/
static int
ShowHdr() 
{
	Page	*itemptr;
	int	retval;
	
	strcpy(s_sth.s_name, emp_rec.em_last_name);
	strcat(s_sth.s_name, ", ");
	strcat(s_sth.s_name, emp_rec.em_first_name);
	strcat(s_sth.s_name, " ");
	strcat(s_sth.s_name, emp_rec.em_mid_name);
	/* Copy Record to Screen */
	strcpy(s_sth.s_barg,emp_rec.em_barg);
	retval = Read_Barg(s_sth.s_barg);
	if(retval < 0) 
		strcpy(s_sth.s_bargdesc,"**  Not Setup **");		
	else
	 	strcpy(s_sth.s_bargdesc,barg_rec.b_name);
	strcpy(s_sth.s_classcd,emp_rec.em_class);
	retval = Read_Class(s_sth.s_classcd);
	if(retval < 0) 
		strcpy(s_sth.s_classdsc,"**  Not Setup **");		
	else
	 	strcpy(s_sth.s_classdsc,class_rec.c_desc);
	strcpy(s_sth.s_position,emp_rec.em_pos);
	retval = Read_Position(s_sth.s_position);
	if(retval < 0) 
		strcpy(s_sth.s_posdesc,"**  Not Setup **");		
	else
	 	strcpy(s_sth.s_posdesc,pos_rec.p_desc);
	s_sth.s_percent = emp_rec.em_perc;	

	itemptr = CurPage;
	CurPage = itemptr;

	ret( WriteFields((char *)&s_sth, KEY_START, HDR_END_FLD) );
	
	return(NOERROR);
}
/*------------------------------------------------------------*/
/* Show all the items on the current page 		      */
static int
ShowItems(pageptr)
Page	*pageptr ;
{
	int	i ;


	if(pageptr != NULL) {
		/* Copy the items to screen */
		scpy((char*)s_sth.s_items, (char*)pageptr->Items,
			(pageptr->NoItems * sizeof(S_item)) );

		s_sth.s_page   = pageptr->Pageno ;
		strcpy(s_sth.s_dummy,"--");
		i = pageptr->NoItems ;
	}
	else {
		s_sth.s_page = HV_SHORT ;
		s_sth.s_dummy[0] = HV_CHAR;
		i = 0 ;
	}

	/* Move High Values to remaining Items */
	for( ; i < PAGESIZE ; i++ )
		InitItem(HV_CHAR,HV_SHORT,HV_DOUBLE,i);

	WriteFields((char *)&s_sth, PAGE_FLD, (END_FLD - 200));

	return(NOERROR) ;
}	/* ShowItems() */
/*-----------------------------------------------------------------------*/ 
/* Process all the items in the link list and write any changes to the   */
/* file.								 */
/*-----------------------------------------------------------------------*/ 
static int
ProcItemUpdates(mode)
int	mode;
{
	Page	*temppage;
	int	i;
	int	retval;
	int	write_mode;

	strcpy(emp_rec.em_numb,s_sth.s_emp);
	retval = get_employee(&emp_rec,mode,0,e_mesg);
	if(retval < 0) {
		DispError((char *)&s_sth,e_mesg);
		roll_back(e_mesg);
		return(retval);
	}
	scpy((char *)&pre_emp,(char *)&emp_rec,sizeof(emp_rec));
	
	/* Write Header */
	strcpy(emp_rec.em_barg,s_sth.s_barg);
	strcpy(emp_rec.em_pp_code,barg_rec.b_pp_code);
	strcpy(emp_rec.em_class,s_sth.s_classcd);
	strcpy(emp_rec.em_pos,s_sth.s_position);
	emp_rec.em_perc = s_sth.s_percent;
	emp_rec.em_sen_perc = s_sth.s_percent;
	emp_rec.em_cert[0] = '\0';
	emp_rec.em_level = 0;

	retval = put_employee(&emp_rec,mode,e_mesg);
	if(retval < 0) {
		DispError((char *)&s_sth,e_mesg);
		roll_back(e_mesg);
		return(retval);
	}

	if(mode != ADD) {
		retval = rite_audit((char*)&s_sth,EMPLOYEE,mode,(char*)&emp_rec,
			(char*)&pre_emp,e_mesg);
		if(retval==LOCKED) {
			DispError((char *)&s_sth,e_mesg);
			roll_back(e_mesg) ;
			return(LOCKED) ;
		}
		if(retval < 0 ){
			DispError((char *)&s_sth,"ERROR: Saving Records"); 
			DispError((char *)&s_sth,e_mesg);
			roll_back(e_mesg);
			return(retval);
		}
	}

	if(CurLast != NULL) {
	   for(temppage=FirstPage; temppage!=NULL;temppage=temppage->NextPage) {
	      for(i =0; i< temppage->NoItems; i++) {
		 retval = GetMode(temppage,i,mode,&write_mode);
		 if(write_mode == NOOP) continue;
		 retval = WriteRecords(temppage,i,write_mode);
  		 if(retval < 0) {
			if(retval == LOCKED) return(LOCKED);
		   	break;
		 }
	      }
	      if(temppage == CurLast) break;
	   }
	}
	retval = commit(e_mesg) ;
	if(retval < 0) {
		DispError((char *)&s_sth,"ERROR: Saving Records"); 
		DispError((char *)&s_sth,e_mesg);
		roll_back(e_mesg);
		return(retval);
	}
	return(NOERROR);
}
/*-----------------------------------------------------------------------*/ 
static int
GetMode(temppage,item_no,mode,write_mode)
Page	*temppage;
int	item_no;
int	mode;
int	*write_mode;
{
	if(mode == ADD) {
		if(strcmp(temppage->Items[item_no].s_status,INACTIVE)==0) {
			*write_mode = NOOP;
		}
		else {
			*write_mode = ADD;
		}
	}
	else if(mode == UPDATE) {
		if(strcmp(temppage->Items[item_no].s_status,INACTIVE)==0) {
			*write_mode = P_DEL;
		}
		else if(temppage->I_Status[item_no][0] == ADDITEMS) {
			*write_mode = ADD;
		}
		else if(temppage->I_Status[item_no][0] == CHANGE) {
			*write_mode = UPDATE;
		}
		else if(temppage->I_Status[item_no][0] == ' ') {
			*write_mode = NOOP;
		}
	}
	else if(mode == P_DEL) {
		*write_mode = P_DEL;
	}

	return(NOERROR);
}
/*-----------------------------------------------------------------------*/ 
/* Write the Area record to the file.			 		 */ 
/*-----------------------------------------------------------------------*/ 
static int
WriteRecords(temppage,item_no,mode)
Page	*temppage;
int	item_no;
int	mode;
{
	int	retval;
	int	i;

	scpy((char *)&pre_emp_sched,(char *)&emp_sched,sizeof(emp_sched));

	strcpy(emp_sched.es_numb,s_sth.s_emp);
	emp_sched.es_week = temppage->Items[item_no].s_week;
	emp_sched.es_fund = temppage->Items[item_no].s_fund;
	strcpy(emp_sched.es_class,temppage->Items[item_no].s_class);
	emp_sched.es_cost = temppage->Items[item_no].s_cc;
	if(mode != ADD) {
		retval = get_emp_sched1(&emp_sched,UPDATE,0,e_mesg) ;
		if(retval == UNDEF && mode == UPDATE)
			mode = ADD;
		else{
		  if(retval < 0) {
			DispError((char *)&s_sth,e_mesg);
			roll_back(e_mesg);
			return(retval);
		  }
		}
	}
	emp_sched.es_amount = temppage->Items[item_no].s_amount;
	strcpy(emp_sched.es_dept,temppage->Items[item_no].s_dept);
	strcpy(emp_sched.es_area,temppage->Items[item_no].s_area);

	for(i=0;i<7;i++) {
		emp_sched.es_units[i] = 
			temppage->Items[item_no].s_units[i];
	}

	retval = put_emp_sched1(&emp_sched,mode,e_mesg) ;
	if(retval < 0) {
		DispError((char *)&s_sth,e_mesg);
		roll_back(e_mesg);
		return(retval);
	}

	if(mode != ADD) {
		retval = rite_audit((char*)&s_sth,EMP_SCHED1,mode,(char*)&emp_sched,
			(char*)&pre_emp_sched,e_mesg);
		if(retval==LOCKED) {
			DispError((char *)&s_sth,e_mesg);
			roll_back(e_mesg) ;
			return(LOCKED) ;
		}
		if(retval < 0 ){
			DispError((char *)&s_sth,"ERROR: Saving Records"); 
			DispError((char *)&s_sth,e_mesg);
			roll_back(e_mesg);
			return(retval);
		}
	}

	return(NOERROR);
}
/*****************************************************************************/
/*	Read Bargaining Unit file 					     */
/*								   	     */
Read_Barg(temp_barg)
char	*temp_barg;
{
	int	retval;

	strcpy(barg_rec.b_code,temp_barg);
	barg_rec.b_date = 0 ;
	flg_reset(BARG);
	retval = get_n_barg(&barg_rec,BROWSE,0,FORWARD,e_mesg);
	if(retval < 0) return(retval);
	if(strcmp(barg_rec.b_code, temp_barg) != 0) {
		return(UNDEF);
	}
	return(NOERROR);
}
/*****************************************************************************/
/*	Read Position file 						     */
/*								   	     */
Read_Position(temp_pos)
char	*temp_pos;
{
	int	retval;

	strcpy(pos_rec.p_code,temp_pos);
	retval = get_position(&pos_rec,BROWSE,0,e_mesg);
	return(retval);
}
/*****************************************************************************/
/*	Read Classification file 					     */
/*								   	     */
Read_Class(temp_class)
char	*temp_class;
{
	int	retval;

	strcpy(class_rec.c_code,temp_class);
	class_rec.c_date = s_sth.s_sysdate;
	flg_reset(CLASSIFICATION);
	retval = get_n_class(&class_rec,BROWSE,0,BACKWARD,e_mesg);
	if(retval != EFL && retval != UNDEF && retval < 0) {
		fomen(e_mesg);
		get();
		return(retval);
	}
	if(retval < 0) return(UNDEF);
	if(strcmp(class_rec.c_code, temp_class) != 0) {
		return(UNDEF);
	}
	return(NOERROR);
}
/*****************************************************************************/
/*	Read Cost Center file 					     */
/*								   	     */
Read_CostCenter(temp_cc)
short	temp_cc;
{
	int	retval;

	sch_rec.sc_numb=temp_cc;
	retval = get_sch(&sch_rec,BROWSE,0,e_mesg);
	return(retval);
}
/*****************************************************************************/
/*	Read Department file 						     */
/*								   	     */
Read_Dept(temp_dept)
char	*temp_dept;
{
	int	retval;

	strcpy(dept_rec.d_code,temp_dept);
	retval = get_dept(&dept_rec,BROWSE,0,e_mesg);
	return(retval);
}
/*****************************************************************************/
/*	Read Area file		 					     */
/*								   	     */
Read_Area(temp_area,temp_dept)
char	*temp_area;
char	*temp_dept;
{
	int	retval;

	strcpy(area_rec.a_code,temp_area);
	strcpy(area_rec.a_deptcode,temp_dept);
	flg_reset(AREA);
	retval = get_n_area(&area_rec,BROWSE,0,FORWARD,e_mesg);
	if(retval < 0) return(retval);
	if(strcmp(area_rec.a_code, temp_area) != 0) {
		return(UNDEF);
	}
	if(strcmp(area_rec.a_deptcode, temp_dept) != 0) {
		return(UNDEF);
	}
	return(NOERROR);
}
/*------------------------------------------------------------*/
/* Read details of given item# */
/*------------------------------------------------------------*/
static int
ReadItem(item_no,mode)
int	item_no ;
int	mode ;
{
	int	i;
	int	st_fld ;
	int	end_fld ;

	if(mode == ADD) {
		SetDupBuffers(ITEM_ST_FLD, END_FLD - 200,0);
	}
	else {
		SetDupBuffers(ITEM_ST_FLD, END_FLD - 200,1);
	
	}

#ifdef ENGLISH
	strcpy(s_sth.s_mesg,"Press ESC-F to Terminate");
#else
	strcpy(s_sth.s_mesg,"Appuyer sur ESC-F pour terminer");
#endif
	DispMesgFld((char *)&s_sth);

	st_fld = ITEM_ST_FLD + (STEP * item_no);
	end_fld  = st_fld + 1900;
	if(mode == ADD)
		InitItem(LV_CHAR,LV_SHORT,LV_DOUBLE,item_no);
	else{
		for(i=0; i < 7; i++) 
			s_sth.s_items[item_no].s_units[i] = LV_DOUBLE;		

		s_sth.s_items[item_no].s_rate = LV_DOUBLE;

		s_sth.s_items[item_no].s_amount = LV_DOUBLE;	
	}

	i = ReadFields((char *)&s_sth,st_fld,end_fld,Validation,WindowHelp,1) ;
	if(PROFOM_ERR == i || DBH_ERR == i) return(i) ;
	if(RET_USER_ESC == i) {
		if(mode == ADD) {
			InitItem(HV_CHAR,HV_SHORT,HV_DOUBLE,item_no);
			WriteFields((char *)&s_sth,st_fld,end_fld);
			return(RET_USER_ESC);
		}
		/* When user gives ESC-F while changing fields, assumption is
		* he completed his changes, and remaining fields are same as
		* old. But, at this point STH will be having low values in the 
		* remaining fields. So move the old values form the linked list.
		*/

		i = CopyBack((char *)&s_sth,(char *)&image,sr.curfld, END_FLD);
		if(i == PROFOM_ERR) return(i);

		return(RET_USER_ESC) ;
	}

	return(NOERROR) ;
}	/* ReadItem() */
/*----------------------------------------------------------------*/
/* Set Duplication buffers for fields 				  */
static int
SetDupBuffers( firstfld, lastfld, value )
int	firstfld, lastfld;	/* field numbers range */
int	value;			/* ENABLE or DISABLE */
{
	int i ;

	for( i=firstfld; i<=lastfld; i+=100 )
		fomca1( i, 19, value);
	if( value==0 )
		return(0);
	sr.nextfld = firstfld;
	sr.endfld = lastfld;
	fomud( (char *)&s_sth );
	ret( err_chk(&sr) );

	return( 0 );
}
/*----------------------------------------------------------------*/
/* Validation function() for Key and Header fields when PROFOM returns
  RET_VAL_CHK */
static	int
Validation()
{
	int	mode,i, retval, fld_no, item_no, st_fld;
	double	temp_calc, total_units;
	int	temp_fld;
	Page	*itemptr;

	if(s_sth.s_fn[0] == CHANGE)	mode = UPDATE;
	if(s_sth.s_fn[0] == INQUIRE)	mode = BROWSE;

	if(sr.curfld >= ITEM_ST_FLD) {	
		item_no = (sr.curfld - ITEM_ST_FLD) / STEP;
		fld_no = sr.curfld - (STEP * item_no);

		st_fld = ITEM_ST_FLD + (STEP * item_no);
	}
	else 
		fld_no = sr.curfld;

	switch(fld_no){
	case KEY_START:
		Right_Justify_Numeric(s_sth.s_emp,
			sizeof(s_sth.s_emp)-1);
		strcpy(emp_rec.em_numb,s_sth.s_emp);
		retval = get_employee(&emp_rec,BROWSE,0,e_mesg);
		if(s_sth.s_fn[0] != ADDREC) {
			if(retval < 0) {
				fomer("Employee Does not Exist");
				s_sth.s_emp[0] = LV_CHAR;
				return(ERROR);
			}
		}
		else {
			if(retval == 0) {
				fomer("Employee Already Exist");
				s_sth.s_emp[0] = LV_CHAR;
				return(ERROR);
			}
		}
		retval = UsrBargVal(mode,emp_rec.em_numb,emp_rec.em_barg,1,e_mesg);
		if(retval < 0){
			DispError((char *)&s_sth,e_mesg);
			s_sth.s_emp[0] = LV_CHAR;
			return(ERROR);
		}
		strcpy(s_sth.s_name, emp_rec.em_last_name);
		strcat(s_sth.s_name, ", ");
		strcat(s_sth.s_name, emp_rec.em_first_name);
		strcat(s_sth.s_name, " ");
		strcat(s_sth.s_name, emp_rec.em_mid_name);

		strcpy(barg_rec.b_code,emp_rec.em_barg);
		barg_rec.b_date = get_date();
		flg_reset(BARG);

		retval = get_n_barg(&barg_rec,BROWSE,0,BACKWARD,e_mesg);
		if(retval == EFL ||
			strcmp(barg_rec.b_code, emp_rec.em_barg) != 0){
			sprintf(e_mesg,"Bargaining Unit does no exist: %s",
				emp_rec.em_barg);
  			DispError((char *)&s_sth,e_mesg);
			return(NOERROR);
		}
		if(retval < 0){
  			DispError((char *)&s_sth,e_mesg);
  			return(ERROR);
		}
		seq_over(BARG);
	
		break;
	case BARG_FLD:
		Right_Justify_Numeric(s_sth.s_barg,
			sizeof(s_sth.s_barg)-1);
		retval = Read_Barg(s_sth.s_barg);
		if (retval<0){
			fomer("Bargaining Code Does Not Exist - Please Re-enter");
			s_sth.s_barg[0] = LV_CHAR;
		    	return(ERROR);
		} 
	 	strcpy(s_sth.s_bargdesc,barg_rec.b_name);
		strcpy(emp_rec.em_pp_code,barg_rec.b_pp_code);
		break;

	case POS_FLD:
		Right_Justify_Numeric(s_sth.s_position,
			sizeof(s_sth.s_position)-1);
		retval = Read_Position(s_sth.s_position);
		if(retval<0) {
			fomer("Invalid Position Code");
			s_sth.s_position[0] = LV_CHAR;
			return(ERROR);
		}
	 	strcpy(s_sth.s_posdesc,pos_rec.p_desc);

		itemptr = CurPage;
		CurPage = itemptr;
		break;

	case WEEK_FLD:
		if((s_sth.s_items[item_no].s_week <= 0) || (s_sth.s_items[item_no].s_week >= 5)){ 
			fomer("Invalid Week Number");
			s_sth.s_items[item_no].s_week = LV_SHORT;
			return(ERROR);
		}

		break;
	case FUND_FLD:
		retval = CheckFund(s_sth.s_items[item_no].s_fund);
		if(retval < 0) {
			s_sth.s_items[item_no].s_fund = LV_SHORT;
			return(ERROR);
		}
		break;
	case MNCLASS_FLD:
	  	Right_Justify_Numeric(s_sth.s_classcd,
			sizeof(s_sth.s_classcd)-1);
		retval = Read_Class(s_sth.s_classcd);
		if(retval == UNDEF) {
			fomer("Classification Code Does Not Exist - Please Re-enter");
			s_sth.s_classcd[0] = LV_CHAR;
		    	return(ERROR);
		} 

		strcpy(s_sth.s_classdsc, class_rec.c_desc);
		strcpy(s_sth.s_position,class_rec.c_pos);
		retval = Read_Position(s_sth.s_position);
	 	strcpy(s_sth.s_posdesc,pos_rec.p_desc);
	
		break;
	case CLASS_FLD:
	  	Right_Justify_Numeric(s_sth.s_items[item_no].s_class,
			sizeof(s_sth.s_items[item_no].s_class)-1);
		retval = Read_Class(s_sth.s_items[item_no].s_class);
		if(retval == UNDEF) {
			fomer("Classification Code Does Not Exist - Please Re-enter");
			s_sth.s_items[item_no].s_class[0] = LV_CHAR;
		    	return(ERROR);
		} 

		strcpy(s_sth.s_items[item_no].s_classdesc, class_rec.c_desc);
	
		if(class_rec.c_units != 0){
	 	    temp_calc =	class_rec.c_yrly_inc /class_rec.c_units;  
		    s_sth.s_items[item_no].s_rate =
					 temp_calc;
		}
		else{
		    strcpy(pay_period.pp_code,barg_rec.b_pp_code);
		    pay_period.pp_year = 0;
		    flg_reset(PAY_PERIOD);

		    retval = get_n_pay_per(&pay_period,BROWSE,0,FORWARD,e_mesg);
		    if(retval < 0){
			    DispError((char *)&s_sth,e_mesg);
			    return(retval);
		    }
		    if(strcmp(pay_period.pp_code,barg_rec.b_pp_code)!=0){
#ifdef ENGLISH
		    	sprintf(e_mesg,"Pay Period Code %s Not on File",
			    	barg_rec.b_pp_code);
#else
			    sprintf(e_mesg,"Pay Period Code %s Not on File",
	    			barg_rec.b_pp_code);
#endif
			    DispError((char *)&s_sth,e_mesg);
			    return(ERROR);
		    }
		    seq_over(PAY_PERIOD);

	 	    temp_calc = class_rec.c_yrly_inc / (double)pay_period.pp_numb;
		    s_sth.s_items[item_no].s_rate = 0;
		    s_sth.s_items[item_no].s_amount = D_Roundoff(temp_calc) ;
		    if(item_no == 0){
			SetDupBuffers(AMT_FLD, AMT_FLD , 1);
			ret(WriteFields((char *)&s_sth,RATE_FLD,AMT_FLD));
		    }
		    else{
			SetDupBuffers(AMT_FLD2,AMT_FLD2,1);
			ret(WriteFields((char *)&s_sth,RATE_FLD2,AMT_FLD2));
		    }
		    s_sth.s_items[item_no].s_amount = LV_DOUBLE ;
		}

		break;
	case CC_FLD:
		sch_rec.sc_numb = s_sth.s_items[item_no].s_cc;
		retval = get_sch(&sch_rec,BROWSE,0,e_mesg);
		if(retval < 0) {
			fomer("Cost Center Number Does not Exist");
			s_sth.s_items[item_no].s_cc = LV_SHORT;
			return(ERROR);
		}
		strcpy(s_sth.s_items[item_no].s_ccdesc,sch_rec.sc_name);

		itemptr = CurPage;
		retval = ItemCheck(s_sth.s_items[item_no].s_class,
			s_sth.s_items[item_no].s_fund,
			s_sth.s_items[item_no].s_week,
			s_sth.s_items[item_no].s_cc,item_no);
		CurPage = itemptr;

		if(retval < 0){
			sr.curfld -= 500;
			s_sth.s_items[item_no].s_week = LV_SHORT;
			s_sth.s_items[item_no].s_fund = LV_SHORT;
			s_sth.s_items[item_no].s_class[0] = LV_CHAR;
			s_sth.s_items[item_no].s_cc = LV_SHORT;
			return(-1);
		}
		break;

	case	DEPT_FLD:
		if(s_sth.s_items[item_no].s_dept[0] == '\0'){
			sr.curfld += 100;
			break;
		}
		Right_Justify_Numeric(s_sth.s_items[item_no].s_dept,
			sizeof(s_sth.s_items[item_no].s_dept)-1);
		retval = Read_Dept(s_sth.s_items[item_no].s_dept);
		if(retval<0) {
			fomer("Invalid Department Code");
			s_sth.s_items[item_no].s_dept[0] = LV_CHAR;
			return(ERROR);
		}
	 	strcpy(s_sth.s_items[item_no].s_deptdesc,dept_rec.d_desc);
		break;
	case	AREA_FLD:
		if(s_sth.s_items[item_no].s_area[0] == '\0'){
			sr.curfld += 100;
			break;
		}
		Right_Justify_Numeric(s_sth.s_items[item_no].s_area,
			sizeof(s_sth.s_items[item_no].s_area)-1);
		retval = Read_Area(s_sth.s_items[item_no].s_area,s_sth.s_items[item_no].s_dept);
		if(retval<0) {
			fomer("Invalid Area Code");
			s_sth.s_items[item_no].s_area[0] = LV_CHAR;
			return(ERROR);
		}
	 	strcpy(s_sth.s_items[item_no].s_areadesc,area_rec.a_desc);

		break;

	case RATE_FLD:
	case UNIT7_FLD:
		if(fld_no == UNIT7_FLD){
		  if(class_rec.c_units != 0){
	 	    temp_calc =	class_rec.c_yrly_inc /class_rec.c_units;  
		    s_sth.s_items[item_no].s_rate =
					 temp_calc;
		  }
		  else{
		    strcpy(pay_period.pp_code,barg_rec.b_pp_code);
		    pay_period.pp_year = 0;
		    flg_reset(PAY_PERIOD);

		    retval = get_n_pay_per(&pay_period,BROWSE,0,FORWARD,e_mesg);
		    if(retval < 0){
			    DispError((char *)&s_sth,e_mesg);
			    return(retval);
		    }
		    if(strcmp(pay_period.pp_code,barg_rec.b_pp_code)!=0){
#ifdef ENGLISH
		    	sprintf(e_mesg,"Pay Period Code %s Not on File",
			    	barg_rec.b_pp_code);
#else
			    sprintf(e_mesg,"Pay Period Code %s Not on File",
	    			barg_rec.b_pp_code);
#endif
			    DispError((char *)&s_sth,e_mesg);
			    return(ERROR);
		    }
		    seq_over(PAY_PERIOD);

	 	    temp_calc = class_rec.c_yrly_inc / (double)pay_period.pp_numb;
		    s_sth.s_items[item_no].s_rate = 0;
		    s_sth.s_items[item_no].s_amount = D_Roundoff(temp_calc) ;
		    if(item_no == 0){
			SetDupBuffers(AMT_FLD, AMT_FLD , 1);
			ret(WriteFields((char *)&s_sth,RATE_FLD,AMT_FLD));
		    }
		    else{
			SetDupBuffers(AMT_FLD2,AMT_FLD2,1);
			ret(WriteFields((char *)&s_sth,RATE_FLD2,AMT_FLD2));
		    }
		    s_sth.s_items[item_no].s_amount = LV_DOUBLE ;
		  }
		}


		total_units = 0;
		for(i=0; i<7 ; i++) 
			 total_units += s_sth.s_items[item_no].s_units[i] ;
		total_units = D_Roundoff(total_units);	
		s_sth.s_items[item_no].s_amount = total_units *   
			s_sth.s_items[item_no].s_rate ;
		s_sth.s_items[item_no].s_amount = D_Roundoff(s_sth.s_items[item_no].s_amount);
		if(fld_no == UNIT7_FLD){
		    if(item_no == 0){
			SetDupBuffers(RATE_FLD, RATE_FLD , 1);
			ret(WriteFields((char *)&s_sth,RATE_FLD,RATE_FLD));
		    }
		    else{
			SetDupBuffers(RATE_FLD2,RATE_FLD2,1);
			ret(WriteFields((char *)&s_sth,RATE_FLD2,RATE_FLD2));
		    }
		    s_sth.s_items[item_no].s_rate = LV_DOUBLE;
		}
		if(item_no == 0){
			SetDupBuffers(AMT_FLD, AMT_FLD , 1);
			ret(WriteFields((char *)&s_sth,AMT_FLD,AMT_FLD));
		}
		else{
			SetDupBuffers(AMT_FLD2, AMT_FLD2 , 1);
			ret(WriteFields((char *)&s_sth,AMT_FLD2,AMT_FLD2));
		}
		s_sth.s_items[item_no].s_amount = LV_DOUBLE;
		break;
	default:
#ifdef ENGLISH
		sprintf(e_mesg,"No Validity Check for Field#  %d",sr.curfld);
#else
		sprintf(e_mesg,"Pas de controle de validite pour le Champ#  %d",sr.curfld);
#endif

			fomen(e_mesg);
			get();
			return(ERROR) ;
	}	/* Switch sr.curfld */

	return(NOERROR) ;
}	/* Validation() */
/*-----------------------------------------------------------------------
Hide items or display items depending on position code			
-----------------------------------------------------------------------*/
HideItem(t_char,t_short,t_double)
char	t_char;
short	t_short;
double	t_double;
{
	int 	i, j, retval;

	CurPage = FirstPage;
	for(j=1; ; j++){

		if(CurPage==NULL) break;

		for( i=0; i <= PAGESIZE; i++) {

			if(CurPage->Items[i].s_week == HV_SHORT) break;
			if(CurPage->Items[i].s_week == LV_SHORT) break;
			if(CurPage->Items[i].s_week == 0) break;

			CurPage->Items[i].s_fund = t_short;
			CurPage->Items[i].s_class[0] = t_char;
			CurPage->Items[i].s_rate = t_double;
		}
		CurPage=CurPage->NextPage;
	}
	
	if(s_sth.s_items[0].s_week != HV_SHORT){
		s_sth.s_items[0].s_fund = t_short;
		s_sth.s_items[0].s_class[0] = t_char;
		s_sth.s_items[0].s_rate = t_double;

		if(t_char == HV_CHAR){
			s_sth.s_items[0].s_classdesc[0] = HV_CHAR;
		}
		else{
			s_sth.s_items[0].s_classdesc[0] = ' ';
		}
	}

	if(s_sth.s_items[1].s_week != HV_SHORT){
		s_sth.s_items[1].s_fund = t_short;
		s_sth.s_items[1].s_class[0] = t_char;
		s_sth.s_items[1].s_rate = t_double;

		if(t_char == HV_CHAR){
			s_sth.s_items[1].s_classdesc[0] = HV_CHAR;
		}
		else{
			s_sth.s_items[1].s_classdesc[0] = ' ';
		}
	}
	retval = WriteFields((char *)&s_sth,ITEM_ST_FLD,ITEM_END_FLD);
	if(retval < 0) return(retval);

	return(NOERROR);
}
/*----------------------------------------------------------------*/
/* calculate the amount if the rate is not entered		  */
static int
CalcAmt(item)
int item;
{
	int retval, i;
	double tmp_amt;

	tmp_amt = 0;
	for(i=0;i<7;i++){
		tmp_amt += s_sth.s_items[item].s_units[i];
	}

	strcpy(pay_period.pp_code,barg_rec.b_pp_code);
	pay_period.pp_year = 0;
	flg_reset(PAY_PERIOD);

	retval = get_n_pay_per(&pay_period,BROWSE,0,FORWARD,e_mesg);
	if(retval < 0){
		DispError((char *)&s_sth,e_mesg);
		return(retval);
	}
	if(strcmp(pay_period.pp_code,barg_rec.b_pp_code)!=0){
#ifdef ENGLISH
		sprintf(e_mesg,"Pay Period Code %s Not on File",
			barg_rec.b_pp_code);
#else
		sprintf(e_mesg,"Pay Period Code %s Not on File",
			barg_rec.b_pp_code);
#endif
		DispError((char *)&s_sth,e_mesg);
		return(ERROR);
	}
	seq_over(PAY_PERIOD);

	if(param.pr_tot_units == 0) param.pr_tot_units = 1;
	if(pay_period.pp_numb == 0) pay_period.pp_numb = 1;
	tmp_amt = tmp_amt/param.pr_tot_units / pay_period.pp_numb *
		cert.cr_income;
	
	s_sth.s_items[item].s_amount = D_Roundoff(tmp_amt);
}
/*----------------------------------------------------------------*/
/* Show Help Windows, if applicable, when user gives ESC-H in key
   and header fields */
static int
WindowHelp()
{
	Page   	*itemptr;
	int	i, fld_no, item_no, st_fld;
	double	temp_calc;
	int	temp_fld, retval;
	char	tmp_dept[7];

	if(sr.curfld >= ITEM_ST_FLD) {	
		item_no = (sr.curfld - ITEM_ST_FLD) / STEP;
		fld_no = sr.curfld - (STEP * item_no);

		st_fld = ITEM_ST_FLD + (STEP * item_no);
	}
	else 
		fld_no = sr.curfld;

	switch(fld_no){
	case KEY_START:
		retval = emp_hlp(s_sth.s_emp,7,13);
		if(retval == DBH_ERR) return(retval);
		if(retval >= 0) redraw();
		if(retval == 0) return(ERROR);
		if(retval < 0) return(ERROR);
		strcpy(emp_rec.em_numb,s_sth.s_emp);
		retval = get_employee(&emp_rec,BROWSE,0,e_mesg);
		if(retval < 0) {
			fomer(e_mesg);
			s_sth.s_emp[0] = LV_CHAR;
			return(ERROR);
		}
		strcpy(s_sth.s_name, emp_rec.em_last_name);
		strcat(s_sth.s_name, ", ");
		strcat(s_sth.s_name, emp_rec.em_first_name);
		strcat(s_sth.s_name, " ");
		strcat(s_sth.s_name, emp_rec.em_mid_name);

		strcpy(barg_rec.b_code,emp_rec.em_barg);
		barg_rec.b_date = get_date();
		flg_reset(BARG);

		retval = get_n_barg(&barg_rec,BROWSE,0,BACKWARD,e_mesg);
		if(retval == EFL ||
			strcmp(barg_rec.b_code, emp_rec.em_barg) != 0){
			sprintf(e_mesg,"Bargaining Unit does no exist: %s",
				emp_rec.em_barg);
  			DispError((char *)&s_sth,e_mesg);
			return(NOERROR);
		}
		if(retval < 0){
  			DispError((char *)&s_sth,e_mesg);
  			return(ERROR);
		}
		seq_over(BARG);

		break;
	case POS_FLD:
		retval = position_hlp(s_sth.s_position,7,13);
		if(retval == DBH_ERR) return(retval);
		if(retval >= 0) redraw();
		if(retval == 0) return(ERROR);
		if(retval < 0) return(ERROR);
		retval = Read_Position(s_sth.s_position);
	 	strcpy(s_sth.s_posdesc,pos_rec.p_desc);

		itemptr = CurPage;
		CurPage = itemptr;
		break;
	case BARG_FLD:
		retval = barg_hlp(s_sth.s_barg,&effective_date,7,13);
		if(retval == DBH_ERR) return(retval);
		if(retval >= 0) redraw();
		if(retval == 0) return(ERROR);
		if(retval < 0) return(ERROR);
		retval = Read_Barg(s_sth.s_barg);
	 	strcpy(s_sth.s_bargdesc,barg_rec.b_name);
	 	strcpy(emp_rec.em_pp_code,barg_rec.b_pp_code);
		break;
	case MNCLASS_FLD:
		retval = class_hlp(s_sth.s_classcd,&effective_date,7,13);
		if(retval == DBH_ERR) return(retval);
		if(retval >= 0) redraw();
		if(retval == 0) return(ERROR);
		if(retval < 0) return(ERROR);
		retval = Read_Class(s_sth.s_classcd);

	 	strcpy(s_sth.s_classdsc,class_rec.c_desc);
		strcpy(s_sth.s_position,class_rec.c_pos);
		retval = Read_Position(s_sth.s_position);
	 	strcpy(s_sth.s_posdesc,pos_rec.p_desc);
		break;

	case CLASS_FLD:
		retval = class_hlp(s_sth.s_items[item_no].s_class,&effective_date,7,13);
		if(retval == DBH_ERR) return(retval);
		if(retval >= 0) redraw();
		if(retval == 0) return(ERROR);
		if(retval < 0) return(ERROR);
		retval = Read_Class(s_sth.s_items[item_no].s_class);

	 	strcpy(s_sth.s_items[item_no].s_classdesc,class_rec.c_desc);

		if(class_rec.c_units != 0){
	 	  temp_calc = class_rec.c_yrly_inc / class_rec.c_units; 
		  s_sth.s_items[item_no].s_rate = temp_calc;
		}
		else{
		    strcpy(pay_period.pp_code,barg_rec.b_pp_code);
		    pay_period.pp_year = 0;
		    flg_reset(PAY_PERIOD);

		    retval = get_n_pay_per(&pay_period,BROWSE,0,FORWARD,e_mesg);
		    if(retval < 0){
			    DispError((char *)&s_sth,e_mesg);
			    return(retval);
		    }
		    if(strcmp(pay_period.pp_code,barg_rec.b_pp_code)!=0){
#ifdef ENGLISH
		    	sprintf(e_mesg,"Pay Period Code %s Not on File",
			    	barg_rec.b_pp_code);
#else
			    sprintf(e_mesg,"Pay Period Code %s Not on File",
	    			barg_rec.b_pp_code);
#endif
			    DispError((char *)&s_sth,e_mesg);
			    return(ERROR);
		    }
		    seq_over(PAY_PERIOD);

	 	    temp_calc = class_rec.c_yrly_inc / (double)pay_period.pp_numb;
		    temp_calc = D_Roundoff(temp_calc);
		    s_sth.s_items[item_no].s_rate = 0;
		    s_sth.s_items[item_no].s_amount = temp_calc ;
		    if(item_no == 0){
			SetDupBuffers(AMT_FLD, AMT_FLD , 1);
			ret(WriteFields((char *)&s_sth,AMT_FLD,AMT_FLD));
		    }
		    else{
			SetDupBuffers(AMT_FLD2,AMT_FLD2,1);
			ret(WriteFields((char *)&s_sth,AMT_FLD2,AMT_FLD2));
		    }
		    s_sth.s_items[item_no].s_amount = LV_DOUBLE ;
		}
		break;

	case CC_FLD:
		sch_rec.sc_numb = s_sth.s_items[item_no].s_cc;
		retval = sch_hlp(s_sth.s_items[item_no].s_cc,7,13);
		if(retval == DBH_ERR) return(retval);
		redraw();
		strcpy(s_sth.s_items[item_no].s_ccdesc,sch_rec.sc_name);

		itemptr = CurPage;
		retval = ItemCheck(s_sth.s_items[item_no].s_class,
			s_sth.s_items[item_no].s_fund,
			s_sth.s_items[item_no].s_week,
			s_sth.s_items[item_no].s_cc,item_no);
		CurPage = itemptr;

		if(retval < 0){
			sr.curfld -= 500;
			s_sth.s_items[item_no].s_week = LV_SHORT;
			s_sth.s_items[item_no].s_fund = LV_SHORT;
			s_sth.s_items[item_no].s_class[0] = LV_CHAR;
			s_sth.s_items[item_no].s_cc = LV_SHORT;
			return(-1);
		}
		break;

	case DEPT_FLD:
		retval = dept_hlp(s_sth.s_items[item_no].s_dept,7,13);
		if(retval == DBH_ERR) return(retval);
		if(retval >= 0) redraw();
		if(retval == 0) return(ERROR);
		if(retval < 0) return(ERROR);
		retval = Read_Dept(s_sth.s_items[item_no].s_dept);
	 	strcpy(s_sth.s_items[item_no].s_deptdesc,dept_rec.d_desc);
		break;

	case AREA_FLD:
		tmp_dept[0] = '\0';
		retval = area_hlp(tmp_dept,
			s_sth.s_items[item_no].s_area,7,13);
		if(retval == DBH_ERR) return(retval);

		if(retval >= 0) redraw();
		if(retval == 0) return(ERROR);
		if(retval < 0) return(ERROR);
		retval = Read_Area(s_sth.s_items[item_no].s_area,
			s_sth.s_items[item_no].s_dept);
	 	strcpy(s_sth.s_items[item_no].s_areadesc,area_rec.a_desc);

		break;

	default :
		fomer("No Help Window for This Field");
	}	/* Switch sr.curfld */

	return(NOERROR) ;
}	/* HdrAndKeyWindowHelp() */
/*-----------------------------------------------------------------------*/
static int
CheckFund(fund)
short	fund;
{
	int	retval;
	int	i;
	Page	*temppage;
	Ctl_rec	ctl_rec;

	if(fund == 0) {
		fomer("This is a Required Field");
		return(ERROR);
	}

	if(pa_rec.pa_glmast[0] == YES) {
		ctl_rec.fund = fund;
		retval = get_ctl(&ctl_rec,BROWSE,0,e_mesg);
		if(retval < 0 && retval != UNDEF) {
			DispError((char *)&s_sth,e_mesg);
			return(retval);
		}
		if(retval == UNDEF) {
			fomer("Fund Does not Exist");
			return(ERROR);
		}
	}

	return(NOERROR);
}
/*-----------------------------------------------------------------------*/
/* Take the confirmation from user for the items part of the screen      */
/*-----------------------------------------------------------------------*/
static int
ConfirmItems()
{
	int	err ;

	/* Options: YSLNPC */

	for( ; ; ) {
	    if(s_sth.s_fn[0] == DELETE) {
#ifdef ENGLISH
			err = GetOption((char *)&s_sth,"Y(es), C(ancel)","YC");
#else
			err = GetOption((char *)&s_sth,"O(ui), A(nnul)","OA");
#endif
	    }
	    else if(s_sth.s_fn[0] != INQUIRE) {
#ifdef ENGLISH
			err = GetOption((char *)&s_sth,
		"Y(es), A(dd), H(eader), L(ine), D(el), R(eactivate), N(ext), P(rev), C(ancel)"
		,"YAHLDRNPC");
#else
			err = GetOption((char *)&s_sth,
		"O(ui), S(creen edit), L(ine edit), S(uiv), P(rec), A(nnul)"
		,"OSLA");
#endif
	    }
	    else {
#ifdef ENGLISH
			err = GetOption((char *)&s_sth,
		"Y(es), N(ext), P(rev)","YNP");
#else
			err = GetOption((char *)&s_sth,
		"O(ui), S(uiv), P(rec)" ,"OSP");
#endif
	    }
	    switch(err) {
	    case  YES  :
		return(YES);
	    case  ADDITEMS:
		err = AddItems();
		break;
	    case  HEADEREDIT:
		err = HeaderEdit();
		break;
	    case  LINEEDIT  :
		err = ChangeFields();
		break ;
	    case  DELITEM:
	    case  REACTITEM:
		err = ChangeStatus(err);
		break;
	    case  NEXT:
		if(CurPage == CurLast || CurLast == NULL) {
#ifdef ENGLISH
			fomer("No More Pages....");
#else
			fomer("Plus de pages....");
#endif
			continue;
		}
		CurPage = CurPage->NextPage ;
		err = ShowItems(CurPage);
		break;
	    case  PREV:
		if(CurLast == NULL || CurPage == FirstPage) {
#ifdef ENGLISH
			fomer("No More Pages....");
#else
			fomer("Plus de pages....");
#endif
			continue;
		}
		CurPage = CurPage->PrevPage ;
		err = ShowItems(CurPage);
		break;
	    case  CANCEL :
#ifdef ENGLISH
		err = GetOption((char *)&s_sth,
				"Confirm the Cancel (Y/N)?", "YN") ;
#else
		err = GetOption((char *)&s_sth,
				"Confirmer l'annulation (O/N)?", "ON") ;
#endif
		if(err == YES) return(CANCEL) ;
		break ;
	    }	/* switch err */

	    if(err == PROFOM_ERR) return(err) ;
	    if(err == DBH_ERR) return(err) ;
	}	/* for(; ; ) */
}	/* ConfirmItems() */
/*---------------------------------------------------------------------*/
/* Change Header.  Allows editing of the students on the screen        */
/*---------------------------------------------------------------------*/
static int
HeaderEdit()
{
     	int i;
	int retval;

	/* make copy of screen incase user presses ESC-F */
	scpy((char *)&image,(char *)&s_sth,sizeof(s_sth));

#ifdef ENGLISH
	strcpy(s_sth.s_mesg,"Press ESC-F to Terminate");
#else
	strcpy(s_sth.s_mesg,"Appuyer sur ESC-F pour terminer");
#endif
	DispMesgFld((char *)&s_sth);

	if(SetDupBuffers(HDR_ST_FLD, HDR_END_FLD,1)<0) return(PROFOM_ERR);

	retval = ReadHdr(UPDATE);
	if(retval != NOERROR && retval != RET_USER_ESC) {
		return(retval);
	}

	if(retval == RET_USER_ESC) 
		return(retval);

     	s_sth.s_mesg[0] = HV_CHAR ;
     	DispMesgFld((char *)&s_sth);

	if(SetDupBuffers(HDR_ST_FLD, HDR_END_FLD,0)<0) return(PROFOM_ERR);

	return(NOERROR);
}	/* HeaderEdit() */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Changing fields. Accept fld of the student to be changed and read     */
/* that fld 		 */
static int
ChangeFields()
{
	int retval;
	int day;

	/* make copy screen every time field changed in case user */
	/* presses ESC-F */
	scpy((char*)&image,(char*)&s_sth, sizeof(s_sth));

	SetDupBuffers(ITEM_ST_FLD, END_FLD - 200, 2);

	/* Get The Field to Be Modified */
#ifdef	ENGLISH
	strcpy(s_sth.s_mesg,"Enter RETURN to Terminate Edit");
#else
	strcpy(s_sth.s_mesg,"Appuyer sur RETURN pour terminer l'ajustement");
#endif
	DispMesgFld((char *)&s_sth); ;
        
     	for (; ;) {
		s_sth.s_field = LV_SHORT;
		retval = ReadFields((char *)&s_sth,CHG_FLD,CHG_FLD,
			(int (*)())NULL,(int (*)())NULL, 1);

		if (retval < 0) return(-1);
		if (retval == RET_USER_ESC) break;	/* User enter ESC-F */

       		if (s_sth.s_field == 0 ) break;  /* Finished changing fields */
		if (s_sth.s_field > CurPage->NoItems) {
			fomer("Line Number out of Range");
			continue;
		}

		if(strcmp(CurPage->Items[s_sth.s_field-1].s_status,INACTIVE)==0){
			fomer("Item Has a Deleted Status Cannot Edit");
			continue;
		}
		retval = ReadItem(s_sth.s_field - 1,UPDATE);
		if(retval != NOERROR && retval != RET_USER_ESC) {
			return(retval);
		}

		if(retval == RET_USER_ESC)
			continue;

		/* make copy screen every time field changed in case user */
		/* presses ESC-F */
		scpy((char*)&image,(char*)&s_sth, sizeof(s_sth));

		scpy((char*)&(CurPage->Items[s_sth.s_field -1]), 
		     (char*)&(s_sth.s_items[s_sth.s_field -1]),sizeof(S_item)) ;

		if(s_sth.s_fn[0] == CHANGE) {
		  if(CurPage->I_Status[s_sth.s_field-1][0] != ADD)
			CurPage->I_Status[s_sth.s_field-1][0] = CHANGE;
		}
	}

     	s_sth.s_field = HV_SHORT ;
	if ( WriteFields((char *)&s_sth,CHG_FLD, CHG_FLD) < 0 ) return(-1);

     	s_sth.s_mesg[0] = HV_CHAR ;
     	DispMesgFld((char *)&s_sth);

	if(SetDupBuffers(ITEM_ST_FLD, END_FLD - 200, 0)<0) return(PROFOM_ERR);

	return(NOERROR);
}	/* ChangeFields() */
/*------------------------------------------------------------------------*/
static int
ChangeStatus(status)
int	status;
{
	int	retval;
	int	st_fld, end_fld;

	/* Get The Field to Be Modified */
#ifdef	ENGLISH
	strcpy(s_sth.s_mesg,"Enter RETURN to Terminate Edit");
#else
	strcpy(s_sth.s_mesg,"Appuyer sur RETURN pour terminer l'ajustement");
#endif
	DispMesgFld((char *)&s_sth); ;
        
     	for (; ;) {
		s_sth.s_field = LV_SHORT;
		retval = ReadFields((char *)&s_sth,CHG_FLD,CHG_FLD,
			(int (*)())NULL,(int (*)())NULL, 1);

		if (retval < 0) return(-1);
		if (retval == RET_USER_ESC) break;	/* User enter ESC-F */

       		if (s_sth.s_field == 0 ) break;  /* Finished changing fields */

	
		st_fld  = ITEM_ST_FLD + (STEP * (s_sth.s_field-1)) + STATUS_FLD;
		end_fld  = ITEM_ST_FLD + (STEP * (s_sth.s_field-1))+STATUS_FLD;

		if(status == DELITEM) {
			if(strcmp(s_sth.s_items[s_sth.s_field-1].s_status,
			   INACTIVE)==0) {
				fomer("Item is Already Deleted");
			}
			else {
				strcpy(s_sth.s_items[s_sth.s_field-1].s_status,
			   		INACTIVE);
			}
		}
		else {
			if(strcmp(s_sth.s_items[s_sth.s_field-1].s_status,
			   ACTIVE)==0) {
				fomer("Item is Already Active");
			}
			else {
				strcpy(s_sth.s_items[s_sth.s_field-1].s_status,
			   		ACTIVE);
			}
		}
		/* Update Linked List */
		scpy((char*)&(CurPage->Items[s_sth.s_field -1]), 
		     (char*)&(s_sth.s_items[s_sth.s_field -1]),sizeof(S_item)) ;
		
		ret(WriteFields((char *)&s_sth,st_fld,end_fld));
	}
     	s_sth.s_field = HV_SHORT ;
	if ( WriteFields((char *)&s_sth,CHG_FLD, CHG_FLD) < 0 ) return(-1);

	return(NOERROR);
}
/*------------------------------------------------------------------------*/
/* Move High values to all data fields and clear the screen */
static int
ClearScreen()
{
	int	i;

	/* Move High Values to Hedaer part */
	s_sth.s_page = HV_SHORT;
	s_sth.s_dummy[0] = HV_CHAR;
	
	InitHdr(HV_CHAR,HV_SHORT,HV_DOUBLE);

	for(i=0 ; i < PAGESIZE ; i++ ){
		InitItem(HV_CHAR,HV_SHORT,HV_DOUBLE,i);
		s_sth.s_items[i].s_dummy1[0] = HV_CHAR;	
	}

	ret( WriteFields((char *)&s_sth,HDR_ST_FLD, (END_FLD - 200)) );

	return(NOERROR);
}	/* ClearScreen() */
/*-------------------------------------------------------------------------*/
static int
InitHdr(t_char,t_short,t_double)
char	t_char;
short	t_short;
double	t_double;
{
	s_sth.s_barg[0] = t_char;
	if(t_char == HV_CHAR) {
		s_sth.s_bargdesc[0] = t_char;		
		s_sth.s_classdsc[0] = t_char;	
		s_sth.s_posdesc[0] = t_char;	
	}
	s_sth.s_classcd[0] = t_char;	
	s_sth.s_position[0] = t_char;	
	s_sth.s_percent = t_double;	

	return(NOERROR);
}
/*-----------------------------------------------------------------------
Check if the item has already been entered 
-----------------------------------------------------------------------*/
ItemCheck( class_in, fund_in, week_in, cc_in, curr_line )
char	*class_in;
short	fund_in, cc_in;
short	week_in;
int	curr_line;
{
	int 	i, j;

	CurPage = FirstPage;
	for(j=1; ; j++){

		if(CurPage==NULL) break;

		for( i=0; i <= PAGESIZE; i++) {

			if (CurPage->Pageno == j && curr_line == i){
				continue;
			}

			if(class_in[0] == LV_CHAR || class_in[0] == HV_CHAR)
				return(NOERROR);

			if(CurPage->Items[i].s_class[0] == HV_CHAR ||
			    CurPage->Items[i].s_class[0] == '\0')
				return(NOERROR);

			if(strcmp(CurPage->Items[i].s_class, class_in) == 0 &&
			   CurPage->Items[i].s_fund == fund_in &&
			   CurPage->Items[i].s_week == week_in &&
			   CurPage->Items[i].s_cc == cc_in &&
			   strcmp(CurPage->Items[i].s_status, INACTIVE)==0){
#ifdef ENGLISH
				fomen("Item Already Exists - Please Reactivate");
#else
				fomen("Item Already Exists - Please Reactivate");
#endif
				get();
				return(-1);
			}

			if(strcmp(CurPage->Items[i].s_class,class_in) == 0 &&
			   CurPage->Items[i].s_fund == fund_in &&
			   CurPage->Items[i].s_week == week_in &&
			   CurPage->Items[i].s_cc == cc_in ){
#ifdef ENGLISH
				fomen("Item Already Exists - Please Re-enter");
#else
				fomen("Item Already Exists - Please Re-enter");
#endif
				get();
				return(-1);
			}
		}
		CurPage=CurPage->NextPage;
	}
	return(NOERROR);
}
/*-----------------------------------------------------------------------
Check if the item has already been entered 
-----------------------------------------------------------------------*/
ItemCheck1(fund_in, curr_line )
short	fund_in;
int	curr_line;
{
	int 	i, j;

	CurPage = FirstPage;
	for(j=1; ; j++){

		if(CurPage==NULL) break;

		for( i=0; i <= PAGESIZE; i++) {

			if (CurPage->Pageno == j && curr_line == i){
				continue;
			}

			if(fund_in == LV_SHORT || fund_in == HV_SHORT)
				return(NOERROR);

			if(CurPage->Items[i].s_fund == HV_SHORT ||
			    CurPage->Items[i].s_fund == 0)
				return(NOERROR);

			if(CurPage->Items[i].s_fund == fund_in &&
			   strcmp(CurPage->Items[i].s_status, INACTIVE)==0){
#ifdef ENGLISH
				fomen("Item Already Exists - Please Reactivate");
#else
				fomen("Item Already Exists - Please Reactivate");
#endif
				get();
				return(-1);
			}

			if(CurPage->Items[i].s_fund == fund_in){
#ifdef ENGLISH
				fomen("Item Already Exists - Please Re-enter");
#else
				fomen("Item Already Exists - Please Re-enter");
#endif
				get();
				return(-1);
			}
		}
		CurPage=CurPage->NextPage;
	}
	return(NOERROR);
}
/*-------------------------------------------------------------------------*/
/* Initialize Given screen item with either Low values or High values	 */
static int
InitItem(t_char, t_short, t_double,item_no)
char	t_char ;
short	t_short ;
double	t_double ;
int	item_no;
{
	int	i;

	s_sth.s_items[item_no].s_fund = t_short;			
	s_sth.s_items[item_no].s_class[0] = t_char;		

	s_sth.s_items[item_no].s_cc = t_short;			

	s_sth.s_items[item_no].s_week = t_short;			
	for(i=0; i < 7; i++) {
		s_sth.s_items[item_no].s_units[i] = t_double;		
	}
	if(t_char == HV_CHAR) {
		s_sth.s_items[item_no].s_classdesc[0] = HV_CHAR;	
		s_sth.s_items[item_no].s_ccdesc[0] = HV_CHAR;		
		s_sth.s_items[item_no].s_deptdesc[0] = HV_CHAR;		
		s_sth.s_items[item_no].s_areadesc[0] = HV_CHAR;	
		s_sth.s_items[item_no].s_status[0] = HV_CHAR;
	}
	else
		strcpy(s_sth.s_items[item_no].s_status,ACTIVE);

	s_sth.s_items[item_no].s_dept[0] = t_char;		
	s_sth.s_items[item_no].s_area[0] = t_char;		
	s_sth.s_items[item_no].s_rate = t_double;			
	s_sth.s_items[item_no].s_amount = t_double;	

	return(NOERROR) ;
}	/* Inititem() */
/*-----------------------------------------------------------------------*/
/*                                                                       */
static
Screen()
{
	int	retval;

	s_sth.s_field = LV_SHORT;
	retval = ReadFields((char *)&s_sth,CHG_FLD,CHG_FLD,
		(int (*)())NULL,(int (*)())NULL, 1);

	if (retval < 0) return(-1);
	if (retval == RET_USER_ESC) return(NOERROR);	/* User enter ESC-F */

       	if (s_sth.s_field == 0 ) return(NOERROR); /* Finished changing fields */

	if (s_sth.s_field > MAX_SCREEN) {
		fomen("Invalid Field Number");
		get();
		s_sth.s_field = LV_SHORT;
		return(NOERROR);
	}

	switch (s_sth.s_field) {
	case SCR_1  :		/* SCR - 1  'DEMOGRAPHIC' */
	case SCR_3  :		/* SCR - 3  'RESPONSIBILITY' */
	case SCR_4  :		/* SCR - 4  'EARNINGS' */
	case SCR_5  :		/* SCR - 5  'MISCELLANIOUS' */
	case SCR_6  :		/* SCR - 6  'BENEFIT' */
	case SCR_7  :		/* SCR - 7  'DEDUCTION' */
	case SCR_8  :		/* SCR - 8  'CSB/LOAN' */
	case SCR_9  :		/* SCR - 9  'GARNISHMENT' */
	case SCR_10 :		/* SCR - 10 'CHEQUE LOCATION' */
	case SCR_11 :		/* SCR - 11 'ATTENDANCE' */
	case SCR_12 :		/* SCR - 12 'SENIORITY' */
	case SCR_13 :		/* SCR - 13 'TEACHER QUALIFICATION' */
	case SCR_14 :		/* SCR - 14 'TEACHER ASSIGNMENT' */
	case SCR_15 :		/* SCR - 15 'COMPETITION' */
	case SCR_16 :		/* SCR - 16 'STATUS' */
		Cur_Option = s_sth.s_field ;
		return( JUMP ) ;
	default   : 
		return(NOERROR);
	}  /*   end of the switch statement */
}
/*-------------------- E n d   O f   P r o g r a m ---------------------*/
