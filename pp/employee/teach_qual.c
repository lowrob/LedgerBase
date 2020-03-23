/*-----------------------------------------------------------------------
Source Name: teach_qual.c
System     : Ledger Base 
Module     : Personnel/Payroll
Created  On: Feb 25, 1992
Created  By: Eugene Roy 

DESCRIPTION:
	Program to Change/Inquire the Teacher Qualifications.

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
#define	MOD_DATE	"25-FEB-92"		/* Program Last Modified */


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

#define PRGNM  		"teach_qual"
#define SCR_NAME	"teach_qual"

#define MAX_FIELD	14		/* Maximum field # to edit */

/* PROFOM Field Numbers */

#define FN_FLD		400	/* Fn: */

#define KEY_START	500	/* Key Start Field */
#define KEY_END		500	/* Key End Field */
#define CHG_FLD		700

#define HDR_ST_FLD	900	/* Desc. Field */
#define HDR_END_FLD	2400	/* Units/Year Field */
#define	CERT_FLD	900
#define	CC_FLD		1300
#define	AS_FLD		2800

#define PAGE_FLD	2600	/* Page# Field */

#define	ITEM_ST_FLD	2800	/* Item Start Field */
#define ITEM_END_FLD	4500	/* Item End Field */

#define	END_FLD		4700	/* Last Field of the screen */
#define	STEP		300	/* Diff in Line */
#define	STATUS_FLD	3000

/* Profom 'C' structure for the screen "schl1" i.e., for Demographic data */

#define	PAGESIZE	6

typedef struct	{

	char	s_area_spec[7];
	char	s_desc[31];	
	char	s_status[6];

	} S_item;

typedef struct	{

	char	s_pgm[11];		/* 100 Program Name String 10	*/ 
	long	s_sysdate;		/* 300 System date DATE YYYYFMMFDD */
	char	s_fn[2];		/* 400 Fn: field String 1	*/
	char	s_emp[13];		/* 500 Employee No String 12 */
	char	s_name[31];		/* 600 Employee Name String 30*/
	short	s_field;		/* 700 Field Number Numeric 99	*/

	char	s_cert[7];		/* 900 Bargaining unit */
	short	s_level;		/* 1100 Position */
	char	s_level_desc[16];		/* 1400 Certification 99 */
	short	s_years;
	short	s_cc;
	char	s_cc_desc[21];
	char	s_inst1[21];		/* 1400 Certification 99 */
	char	s_prog1[21];		/* 1400 Certification 99 */
	char	s_inst2[21];		/* 1400 Certification 99 */
	char	s_prog2[21];		/* 1400 Certification 99 */
	char	s_inst3[21];		/* 1400 Certification 99 */
	char	s_prog3[21];		/* 1400 Certification 99 */
	char	s_inst4[21];		/* 1400 Certification 99 */
	char	s_prog4[21];		/* 1400 Certification 99 */
	char	s_inst5[21];		/* 1400 Certification 99 */
	char	s_prog5[21];		/* 1400 Certification 99 */

	char	s_dummy1[5];		/* 1600 Prompt for separator */
	short	s_page;			/* 1700 Page No 99 */
	char	s_dummy2[4];		/* 1600 Prompt for separator */

	S_item	s_items[PAGESIZE];	/* 1800 - 5500 */

	char	s_mesg[78];		/* 5600 Message Line String 77	*/
	char	s_opt[2];		/* 5700 Message Option String X */ 

	} s_struct;

static	Sch_rec		sch_rec;
static  Cert		cert;
static	Area_spec	area_spec;
static	Teach_qual	teach_qual, pre_teach_qual;

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

void	free() ;
char	*malloc() ;
static	s_struct	s_sth, image ;

static	int	mode;
static	long	effective_date;
static	int	Validation() ;
static	int	WindowHelp() ;

Tch_qual()
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
		else{
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

	/* Move High Values to data fields and Display the screen */
	err = ClearScreen() ;
	if(NOERROR != err) return(err) ;

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
	fomer("C(hanger), I(nterroger), S(uivant), P(recedent), F(in)");
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
	case CHANGE  :			/* CHANGE */
		CHKACC(retval,UPDATE,e_mesg);
		mode = UPDATE;
		return( Change() ) ;
	case INQUIRE  :			/* Inquire */
		CHKACC(retval,BROWSE,e_mesg);
		mode = BROWSE;
		return( Inquire() ) ;
	case NEXT_RECORD  :			/* Next */
		CHKACC(retval,BROWSE,e_mesg);
		return( Next(FORWARD) ) ;
	case PREV_RECORD  :			/* Previous */
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
/* Delete. Student Study Hall Records.  */
/*-----------------------------------------------------------------------*/
static int
Delete()
{
	int	err ;

	err = SelectRecord(UPDATE) ;
	if(NOERROR != err) return(err) ;

	for( ; ; ) {
		err = ConfirmItems() ;
		if(err != YES) {
			roll_back(e_mesg);
			break;
		}

		err = ProcItemUpdates(P_DEL) ;
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
}	/* Delete() */
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
	for( ; ; ){
	  err = get_n_employee(&emp_rec, BROWSE, 0, direction, e_mesg);
#ifndef ORACLE
	  seq_over(EMPLOYEE);
#endif
	  if(ERROR == err)return(DBH_ERR) ;
	  if(EFL == err) {
		fomen("No More Records....");
		get();
		return(NOERROR) ;
	  }

	  err = UsrBargVal(BROWSE,emp_rec.em_numb,emp_rec.em_barg,0,e_mesg);
	  if(err < 0){
		if(err == UNDEF){
			DispError((char *)&s_sth,e_mesg);
			return(ERROR);
		}
		inc_str(emp_rec.em_numb, sizeof(emp_rec.em_numb)-1, 
			direction);
		flg_reset(EMPLOYEE);
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

	InitHdr(LV_CHAR,LV_SHORT);

	retval = ReadFields((char *)&s_sth,HDR_ST_FLD,HDR_END_FLD,
		Validation,WindowHelp,1) ;
	if(PROFOM_ERR == retval || DBH_ERR == retval) return(retval) ;
	if(RET_USER_ESC == retval) {
		if(mode == ADD) {
			InitHdr(HV_CHAR,HV_SHORT);
			WriteFields((char *)&s_sth,HDR_ST_FLD,HDR_END_FLD);
			return(RET_USER_ESC) ;
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
			strcpy(s_sth.s_dummy1,"---");
			s_sth.s_page = 1 ;
			s_sth.s_dummy2[0] = ' ';
			ret( WriteFields((char *)&s_sth,PAGE_FLD,PAGE_FLD+100));
			i = 0 ;
		}
	}


	for( ; ; ) {
		if( PAGESIZE == i) {	/* Page Full */

			/* Calculate the page# */
			strcpy(s_sth.s_dummy1,"---");
			s_sth.s_dummy2[0] = ' ';
			s_sth.s_page = CurLast->Pageno + 1 ;

			ret( WriteFields((char *)&s_sth,PAGE_FLD, 
				(END_FLD - 200)) ) ;

			InitItem(HV_CHAR,1);
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
static
BuildList()
{
	int retval;
	int i;
	CurLast = CurPage = NULL;
	i = 0;

	strcpy(teach_qual.tq_numb, s_sth.s_emp);
	teach_qual.tq_code[0] = LV_CHAR;		
	flg_reset(TEACH_QUAL);

	for( ; ; ) {
		retval = get_n_teach_qual(&teach_qual,BROWSE,0,FORWARD,e_mesg);
		if(retval < 0) {
			if(retval == EFL) break;
			DispError((char *)&s_sth,e_mesg);
			return(retval);
		}

		if(strcmp(teach_qual.tq_numb,s_sth.s_emp)!=0) { 
			break;
		}

		if(PAGESIZE == i) i = 0;
		if(i == 0) {
			if((retval = MakeFreshPage()) < 0) return(retval);
		}
		strcpy(CurPage->Items[i].s_area_spec,teach_qual.tq_code);

		strcpy(area_spec.ar_code,teach_qual.tq_code);

		retval = get_area_spec(&area_spec,BROWSE,0,e_mesg);
		if(retval < 0) {
			fomer("Area of Specialization Code Does not Exist");
			return(ERROR);
		}
		strcpy(CurPage->Items[i].s_desc, area_spec.ar_desc);
		strcpy(CurPage->Items[i].s_status,ACTIVE);
		CurPage->I_Status[i][0] = ' ';
		CurPage->NoItems++;
		i++;
	}
	seq_over(TEACH_QUAL);


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

	strcpy(s_sth.s_cert, emp_rec.em_cert);
	s_sth.s_level = emp_rec.em_level;
	
	strcpy(cert.cr_code,s_sth.s_cert);
	cert.cr_date = get_date();
	cert.cr_level = s_sth.s_level;
	flg_reset(CERT);

	retval = get_n_cert(&cert,BROWSE,0,FORWARD,e_mesg);
	if(retval < 0) {
	  if(retval == EFL || strcmp(cert.cr_code,s_sth.s_cert) != 0){
			fomer("Certification Code Does not Exist");
	  }
	  fomer("Certification Code Does not Exist");
	}

	s_sth.s_level_desc[0] = HV_CHAR;
	s_sth.s_years = emp_rec.em_yrs_exp;
	s_sth.s_cc = emp_rec.em_pref_cc;

	if(s_sth.s_cc == 0)
		s_sth.s_cc_desc[0] = HV_CHAR;
	else{
	  sch_rec.sc_numb = s_sth.s_cc;

	  retval = get_sch(&sch_rec,BROWSE,0,e_mesg);
	  if(retval < 0 && retval != UNDEF) {
		fomer("Cost Center Number Does not Exist");
	  }
	  strncpy(s_sth.s_cc_desc, sch_rec.sc_name,21);
	}

	strcpy(s_sth.s_inst1, emp_rec.em_inst[0]);
	strcpy(s_sth.s_prog1, emp_rec.em_prog[0]);
	strcpy(s_sth.s_inst2, emp_rec.em_inst[1]);
	strcpy(s_sth.s_prog2, emp_rec.em_prog[1]);
	strcpy(s_sth.s_inst3, emp_rec.em_inst[2]);
	strcpy(s_sth.s_prog3, emp_rec.em_prog[2]);
	strcpy(s_sth.s_inst4, emp_rec.em_inst[3]);
	strcpy(s_sth.s_prog4, emp_rec.em_prog[3]);
	strcpy(s_sth.s_inst5, emp_rec.em_inst[4]);
	strcpy(s_sth.s_prog5, emp_rec.em_prog[4]);

	strcpy(s_sth.s_dummy1,"---");
	ret( WriteFields((char *)&s_sth, KEY_START, HDR_END_FLD+100) );
	
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
		strcpy(s_sth.s_dummy1,"---");
		s_sth.s_dummy2[0] = ' ';
		i = pageptr->NoItems ;
	}
	else {
		s_sth.s_page = HV_SHORT ;
		s_sth.s_dummy1[0] = HV_CHAR;
		s_sth.s_dummy2[0] = HV_CHAR;
		i = 0 ;
	}

	/* Move High Values to remaining Items */
	for( ; i < PAGESIZE ; i++ )
		InitItem(HV_CHAR,i);

	ret( WriteFields((char *)&s_sth, PAGE_FLD, (END_FLD - 200)) );

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

	strcpy(emp_rec.em_cert, s_sth.s_cert);
	emp_rec.em_yrs_exp = s_sth.s_years;
	emp_rec.em_pref_cc = s_sth.s_cc;
	emp_rec.em_level = s_sth.s_level;

	strcpy(emp_rec.em_inst[0], s_sth.s_inst1);
	strcpy(emp_rec.em_prog[0], s_sth.s_prog1);
	strcpy(emp_rec.em_inst[1], s_sth.s_inst2);
	strcpy(emp_rec.em_prog[1], s_sth.s_prog2);
	strcpy(emp_rec.em_inst[2], s_sth.s_inst3);
	strcpy(emp_rec.em_prog[2], s_sth.s_prog3);
	strcpy(emp_rec.em_inst[3], s_sth.s_inst4);
	strcpy(emp_rec.em_prog[3], s_sth.s_prog4);
	strcpy(emp_rec.em_inst[4], s_sth.s_inst5);
	strcpy(emp_rec.em_prog[4], s_sth.s_prog5);

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

	scpy((char *)&pre_teach_qual,(char *)&teach_qual,sizeof(teach_qual));

	strcpy(teach_qual.tq_numb, s_sth.s_emp);
	strcpy(teach_qual.tq_code, s_sth.s_items[item_no].s_area_spec);		

	if(mode != ADD) {
		retval = get_teach_qual(&teach_qual,UPDATE,0,e_mesg) ;
		if(retval < 0) {
			DispError((char *)&s_sth,e_mesg);
			roll_back(e_mesg);
			return(retval);
		}
	}

	retval = put_teach_qual(&teach_qual,mode,e_mesg) ;
	if(retval < 0) {
		DispError((char *)&s_sth,e_mesg);
		roll_back(e_mesg);
		return(retval);
	}

	if(mode != ADD) {
		retval = rite_audit((char*)&s_sth,TEACH_QUAL,mode,(char*)&teach_qual,
			(char*)&pre_teach_qual,e_mesg);
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

	st_fld  = ITEM_ST_FLD ;
	end_fld  = ITEM_END_FLD ;
	InitItem(LV_CHAR,item_no);

	i = ReadFields((char *)&s_sth,st_fld,end_fld,Validation,WindowHelp,1) ;
	if(PROFOM_ERR == i || DBH_ERR == i) return(i) ;
	if(RET_USER_ESC == i) {
		if(mode == ADD) {
			InitItem(HV_CHAR,item_no);
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
	int	i, retval, fld_no, item_no, st_fld;
	double	temp_calc, total_units;
	int	temp_fld;
	Page	*itemptr;

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
		retval = UsrBargVal(mode,emp_rec.em_numb,emp_rec.em_barg,1,e_mesg);
		if(retval < 0){
			DispError((char *)&s_sth,e_mesg);
		  	s_sth.s_emp[0] = LV_CHAR;
			return(-1);
		}
		strcpy(s_sth.s_name, emp_rec.em_last_name);
		strcat(s_sth.s_name, ", ");
		strcat(s_sth.s_name, emp_rec.em_first_name);
		strcat(s_sth.s_name, " ");
		strcat(s_sth.s_name, emp_rec.em_mid_name);
		break;

	case CERT_FLD:
		Right_Justify_Numeric(s_sth.s_cert,
			sizeof(s_sth.s_cert)-1);
		strcpy(cert.cr_code,s_sth.s_cert);
		cert.cr_date = s_sth.s_sysdate;
		
		retval = get_n_cert(&cert,BROWSE,0,BACKWARD,e_mesg);
		if(retval < 0) {
			fomer("Certificate Code Does not Exist");
			s_sth.s_cert[0] = LV_CHAR;
			return(ERROR);
		}
		break;

	case CC_FLD:
		if(s_sth.s_cc == '\0'){
			sr.curfld +=200;
			break;
		}
		sch_rec.sc_numb = s_sth.s_cc;
		retval = get_sch(&sch_rec,BROWSE,0,e_mesg);
		if(retval < 0) {
			fomer("Cost Center Number Does not Exist");
			s_sth.s_cc = LV_SHORT;
			return(ERROR);
		}
		strncpy(s_sth.s_cc_desc,sch_rec.sc_name,21);
		break;

	case AS_FLD:
		strcpy(area_spec.ar_code,s_sth.s_items[item_no].s_area_spec);

		retval = get_area_spec(&area_spec,BROWSE,0,e_mesg);
		if(retval < 0) {
			fomer("Area of Specialization Code Does not Exist");
			s_sth.s_items[item_no].s_area_spec[0] = LV_CHAR;
			return(ERROR);
		}
		strcpy(s_sth.s_items[item_no].s_desc, area_spec.ar_desc);
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
		break;
	case CERT_FLD:
		retval = cert_hlp(s_sth.s_cert,&effective_date,7,13);
		if(retval == DBH_ERR) return(retval);
		if(retval >= 0) redraw();
		if(retval == 0) return(ERROR);
		if(retval < 0) return(ERROR);
		break;
	case CC_FLD:
		retval = sch_hlp(&s_sth.s_cc,7,13);
		if(retval == DBH_ERR) return(retval);
		redraw();
		sch_rec.sc_numb = s_sth.s_cc;
		retval = get_sch(&sch_rec,BROWSE,0,e_mesg);
		if(retval < 0) {
			fomer("Cost Center Number Does not Exist");
			s_sth.s_cc = LV_SHORT;
			return(ERROR);
		}
		strncpy(s_sth.s_cc_desc,sch_rec.sc_name,21);
		break;

	case AS_FLD :
		retval = area_spec_hlp( s_sth.s_items[item_no].s_area_spec,
			 7,13);
		if (retval == DBH_ERR) return(retval);
		if (retval >= 0) redraw();
		if (retval < 1) return(ERROR);
		strcpy(area_spec.ar_code,s_sth.s_items[item_no].s_area_spec);

		retval = get_area_spec(&area_spec,BROWSE,0,e_mesg);
		if(retval < 0) {
			fomer("Area of Specialization Code Does not Exist");
			s_sth.s_items[item_no].s_area_spec[0] = LV_CHAR;
			return(ERROR);
		}
		strcpy(s_sth.s_items[item_no].s_desc, area_spec.ar_desc);
		break;
	default :
		fomer("No Help Window for This Field");
	}	/* Switch sr.curfld */

	return(NOERROR) ;
}	/* HdrAndKeyWindowHelp() */
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
		"Y(es), A(dd), H(eader), D(elete), R(eactivate), N(ext), P(reviou), C(ancel)"
		,"YAHDRNPC");
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
	InitHdr(HV_CHAR,HV_SHORT);

	s_sth.s_dummy1[0] = HV_CHAR;
	s_sth.s_page = HV_SHORT;
	s_sth.s_dummy2[0] = HV_CHAR;

	for(i=0 ; i < PAGESIZE ; i++ ){
		InitItem(HV_CHAR,i);
	}

	ret( WriteFields((char *)&s_sth,HDR_ST_FLD, (END_FLD - 200)) );

	return(NOERROR);
}	/* ClearScreen() */
/*-------------------------------------------------------------------------*/
static int
InitHdr(t_char,t_short)
char	t_char;
short	t_short;
{

	s_sth.s_cert[0] = t_char;
	s_sth.s_level = t_short;
	s_sth.s_level_desc[0] = HV_CHAR;
	s_sth.s_years = t_short;
	s_sth.s_cc = t_short;
	s_sth.s_cc_desc[0] = t_char;
	s_sth.s_inst1[0] = t_char;
	s_sth.s_prog1[0] = t_char;
	s_sth.s_inst2[0] = t_char;
	s_sth.s_prog2[0] = t_char;
	s_sth.s_inst3[0] = t_char;
	s_sth.s_prog3[0] = t_char;
	s_sth.s_inst4[0] = t_char;
	s_sth.s_prog4[0] = t_char;
	s_sth.s_inst5[0] = t_char;
	s_sth.s_prog5[0] = t_char;

	return(NOERROR);
}
/*-------------------------------------------------------------------------*/
/* Initialize Given screen item with either Low values or High values	 */
static int
InitItem(t_char, item_no)
char	t_char ;
int	item_no;
{
	int	i;

	s_sth.s_items[item_no].s_area_spec[0] = t_char;		
	s_sth.s_items[item_no].s_desc[0] = t_char;		
	if(t_char == HV_CHAR)
		s_sth.s_items[item_no].s_status[0] = t_char;		
	else
		strcpy(s_sth.s_items[item_no].s_status, "ACT");

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
	case SCR_2  :		/* SCR - 2  'EMPLOYMENT' */
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
	case SCR_14 :		/* SCR - 14 'TEACHER ASSIGNMENT' */
	case SCR_15 :		/* SCR - 15 'COMPETITION' */
	case SCR_16 :		/* SCR - 16 'STATUS' */
		Cur_Option = s_sth.s_field ;
		return( JUMP ) ;
	default   : 
		return(NOERROR);
	}  /*   end of the switch statement */

	return(NOERROR);
}
/*-------------------- E n d   O f   P r o g r a m ---------------------*/
