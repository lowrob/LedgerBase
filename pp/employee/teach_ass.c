/*-----------------------------------------------------------------------
Source Name: teach_ass.c
System     : Ledger Base 
Module     : Personnel/Payroll
Created  On: Feb 26, 1992
Created  By: Eugene Roy 

DESCRIPTION:
	Program to Change/Inquire the Teacher Assignments.

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
#define	MOD_DATE	"26-FEB-92"		/* Program Last Modified */


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

#define PRGNM  		"teach_ass"
#define SCR_NAME	"teach_ass"

#define MAX_FIELD	3		/* Maximum field # to edit */

/* PROFOM Field Numbers */

#define FN_FLD		400	/* Fn: */

#define KEY_START	500	/* Key Start Field */
#define KEY_END		500	/* Key End Field */
#define CHG_FLD		700

#define	CC_FLD		1000
#define	AS_FLD		1200
#define	PERC_FLD	1500

#define PAGE_FLD	900	/* Page# Field */

#define	ITEM_ST_FLD	1000	/* Item Start Field */
#define ITEM_END_FLD	3500	/* Item End Field */

#define	END_FLD		3700	/* Last Field of the screen */
#define	STEP		1300	/* Diff in Line */
#define	STATUS_FLD	1200

/* Profom 'C' structure for the screen "schl1" i.e., for Demographic data */

#define	PAGESIZE	2

typedef struct	{

	short	s_cc;
	char	s_cc_desc[31];
	char	s_area_spec[7];
	char	s_desc[30];	
	short	s_grade;
	double	s_perc;
	char	s_course[7];
	char	s_crs_desc[31];
	short	s_semester;
	short	s_sect;
	char	s_room[5];
	short	s_no_stud;
	char	s_status[6];

	} S_item;

typedef struct	{

	char	s_pgm[11];		/* 100 Program Name String 10	*/ 
	long	s_sysdate;		/* 300 System date DATE YYYYFMMFDD */
	char	s_fn[2];		/* 400 Fn: field String 1	*/
	char	s_emp[13];		/* 500 Employee No String 12 */
	char	s_name[31];		/* 600 Employee Name String 30*/
	short	s_field;		/* 700 Field Number Numeric 99	*/

	short	s_page;			/* 1700 Page No 99 */
	short	s_dummy;

	S_item	s_items[PAGESIZE];	/* 1800 - 5500 */

	char	s_mesg[78];		/* 5600 Message Line String 77	*/
	char	s_opt[2];		/* 5700 Message Option String X */ 

	} s_struct;

static	Sch_rec		sch_rec;
static  Cert		cert;
static	Area_spec	area_spec;
static	Teach_ass	teach_ass, pre_teach_ass;

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

Tch_ass()
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
	fomer("C(hange), I(nquire), F(orward), B(ackward), N(ext), P(rev), S(creen), E(xit)");
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
static
BuildList()
{
	int retval;
	int i;
	CurLast = CurPage = NULL;
	i = 0;

	strcpy(teach_ass.tc_numb,s_sth.s_emp) ;
	teach_ass.tc_cost = 0;
	teach_ass.tc_ar_sp[0] = '\0';
	flg_reset(TEACH_ASS);
	
	for( ; ; ){
		retval = get_n_teach_ass(&teach_ass,BROWSE,0,FORWARD,e_mesg);
		if(retval < 0) {
			if(retval == EFL) break;
			DispError((char *)&s_sth,e_mesg);
			return(retval);
		}
		if(strcmp(teach_ass.tc_numb,s_sth.s_emp)!=0) { 
			break;
		}
		if(PAGESIZE == i) i = 0;
		if(i == 0) {
			if((retval = MakeFreshPage()) < 0) return(retval);
		}
		CurPage->Items[i].s_cc = teach_ass.tc_cost;
		sch_rec.sc_numb = CurPage->Items[i].s_cc;
		retval = get_sch(&sch_rec,BROWSE,0,e_mesg);
		if(retval < 0) {
			fomer("Cost Center Number Does not Exist");
		}
		strcpy(CurPage->Items[i].s_cc_desc, sch_rec.sc_name);

		strcpy(CurPage->Items[i].s_area_spec,teach_ass.tc_ar_sp);
		strcpy(area_spec.ar_code,teach_ass.tc_ar_sp);

		retval = get_area_spec(&area_spec,BROWSE,0,e_mesg);
		if(retval < 0) {
			fomer("Area of Specialization Code Does not Exist");
		}
		strcpy(CurPage->Items[i].s_desc, area_spec.ar_desc);

		CurPage->Items[i].s_grade = teach_ass.tc_grade;
		CurPage->Items[i].s_perc = teach_ass.tc_perc;
		strcpy(CurPage->Items[i].s_course,teach_ass.tc_crs);
		CurPage->Items[i].s_crs_desc[0] = HV_CHAR;
		CurPage->Items[i].s_semester = teach_ass.tc_sem;
		CurPage->Items[i].s_sect = teach_ass.tc_sec;
		strcpy(CurPage->Items[i].s_room,teach_ass.tc_room);
		CurPage->Items[i].s_no_stud = teach_ass.tc_load;
		strcpy(CurPage->Items[i].s_status,ACTIVE);
		CurPage->I_Status[i][0] = ' ';
		CurPage->NoItems++;
		i++;
	}
	seq_over(TEACH_ASS);

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

	ret( WriteFields((char *)&s_sth, KEY_START, KEY_START+100));
	
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
		i = pageptr->NoItems ;
	}
	else {
		s_sth.s_page = HV_SHORT ;
		i = 0 ;
	}

	/* Move High Values to remaining Items */
	for( ; i < PAGESIZE ; i++ )
		InitItem(HV_CHAR,HV_SHORT,HV_DOUBLE,i);

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

	scpy((char *)&pre_teach_ass,(char *)&teach_ass,sizeof(teach_ass));

	strcpy(teach_ass.tc_numb, s_sth.s_emp);
	teach_ass.tc_cost =  temppage->Items[item_no].s_cc;
	strcpy(teach_ass.tc_ar_sp, temppage->Items[item_no].s_area_spec);

	if(mode != ADD) {
		retval = get_teach_ass(&teach_ass,UPDATE,0,e_mesg) ;
		if(retval == UNDEF && mode == P_DEL){
			roll_back(e_mesg);
			return(NOERROR);
		}
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
	teach_ass.tc_grade = temppage->Items[item_no].s_grade;
	teach_ass.tc_perc = temppage->Items[item_no].s_perc;
	strcpy(teach_ass.tc_crs, temppage->Items[item_no].s_course);
	teach_ass.tc_sem = temppage->Items[item_no].s_semester;
	teach_ass.tc_sec = temppage->Items[item_no].s_sect;
	strcpy(teach_ass.tc_room, temppage->Items[item_no].s_room);
	teach_ass.tc_load = temppage->Items[item_no].s_no_stud;

	retval = put_teach_ass(&teach_ass,mode,e_mesg) ;
	if(retval < 0) {
		DispError((char *)&s_sth,e_mesg);
		roll_back(e_mesg);
		return(retval);
	}

	if(mode != ADD) {
		retval = rite_audit((char*)&s_sth,TEACH_ASS,mode,(char*)&teach_ass,
			(char*)&pre_teach_ass,e_mesg);
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


#ifdef ENGLISH
	strcpy(s_sth.s_mesg,"Press ESC-F to Terminate");
#else
	strcpy(s_sth.s_mesg,"Appuyer sur ESC-F pour terminer");
#endif
	DispMesgFld((char *)&s_sth);

	if(mode == ADD) {
		SetDupBuffers(ITEM_ST_FLD, END_FLD - 200,0);
		InitItem(LV_CHAR,LV_SHORT,LV_DOUBLE,item_no);
	}
	else {
		SetDupBuffers(ITEM_ST_FLD, END_FLD - 200,1);
		s_sth.s_items[item_no].s_grade = LV_SHORT;
		s_sth.s_items[item_no].s_perc = LV_DOUBLE;
		s_sth.s_items[item_no].s_course[0] = LV_CHAR;
		s_sth.s_items[item_no].s_semester = LV_SHORT;
		s_sth.s_items[item_no].s_sect = LV_SHORT;
		s_sth.s_items[item_no].s_room[0] = LV_CHAR;
		s_sth.s_items[item_no].s_no_stud = LV_SHORT;
	}

	st_fld = ITEM_ST_FLD + (STEP * item_no);
	end_fld  = st_fld + STEP-100 ;

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
		if(retval < 0) {
			DispError((char *)&s_sth,e_mesg);
			s_sth.s_emp[0] = LV_CHAR;
			return(ERROR);
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

	case CC_FLD:
		sch_rec.sc_numb = s_sth.s_items[item_no].s_cc;
		retval = get_sch(&sch_rec,BROWSE,0,e_mesg);
		if(retval < 0) {
			fomer("Cost Center Number Does not Exist");
			s_sth.s_items[item_no].s_cc = LV_SHORT;
			return(ERROR);
		}
		strcpy(s_sth.s_items[item_no].s_cc_desc, sch_rec.sc_name);
		break;

	case AS_FLD:
		strcpy(area_spec.ar_code,s_sth.s_items[item_no].s_area_spec);

		retval = get_area_spec(&area_spec,BROWSE,0,e_mesg);
		if(retval < 0) {
			fomer("Area of Specialization Does not Exist");
			s_sth.s_items[item_no].s_area_spec[0] = LV_CHAR;
			return(ERROR);
		}
		strcpy(s_sth.s_items[item_no].s_desc, area_spec.ar_desc);
		retval = CheckCode(item_no);
		if(retval < 0){
			s_sth.s_items[item_no].s_cc = LV_SHORT;
			s_sth.s_items[item_no].s_area_spec[0] = LV_CHAR;
			sr.curfld -=200;
			return(ERROR);
		}
		break;

	case PERC_FLD:
		if(s_sth.s_items[item_no].s_perc > 100){
			fomer("Percentage Cannot Be Larger Than 100");
			s_sth.s_items[item_no].s_perc = LV_DOUBLE;
			return(ERROR);
		}
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
	case CC_FLD:
		retval = sch_hlp(&s_sth.s_items[item_no].s_cc,7,13);
		if(retval == DBH_ERR) return(retval);
		redraw();
		sch_rec.sc_numb = s_sth.s_items[item_no].s_cc;
		retval = get_sch(&sch_rec,BROWSE,0,e_mesg);
		if(retval < 0) {
			fomer("Cost Center Number Does not Exist");
			s_sth.s_items[item_no].s_cc = LV_SHORT;
			return(ERROR);
		}
		strncpy(s_sth.s_items[item_no].s_cc_desc,sch_rec.sc_name,31);
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
		retval = CheckCode(item_no);
		if(retval < 0){
			s_sth.s_items[item_no].s_cc = LV_SHORT;
			s_sth.s_items[item_no].s_area_spec[0] = LV_CHAR;
			sr.curfld -=200;
			return(ERROR);
		}
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
		"Y(es), A(dd), L(ine), D(el), R(eactivate), N(ext), P(rev), C(ancel)"
		,"YALDRNPC");
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
	s_sth.s_dummy = HV_SHORT;

	for(i=0 ; i < PAGESIZE ; i++ ){
		InitItem(HV_CHAR,HV_SHORT,HV_DOUBLE,i);
	}

	ret( WriteFields((char *)&s_sth,PAGE_FLD, (END_FLD - 200)) );

	return(NOERROR);
}	/* ClearScreen() */
/*-------------------------------------------------------------------------*/
/* Initialize Given screen item with either Low values or High values	 */
static int
InitItem(t_char, t_short, t_double, item_no)
char	t_char ;
short	t_short;
double	t_double;
int	item_no;
{
	int	i;

	s_sth.s_items[item_no].s_cc = t_short;
	s_sth.s_items[item_no].s_cc_desc[0] = t_char;
	s_sth.s_items[item_no].s_area_spec[0] = t_char;		
	s_sth.s_items[item_no].s_desc[0] = t_char;		
	s_sth.s_items[item_no].s_grade = t_short;
	s_sth.s_items[item_no].s_perc = t_double;
	s_sth.s_items[item_no].s_course[0] = t_char;
	if(t_char == HV_CHAR)
		s_sth.s_items[item_no].s_crs_desc[0] = t_char;
	s_sth.s_items[item_no].s_semester = t_short;
	s_sth.s_items[item_no].s_sect = t_short;
	s_sth.s_items[item_no].s_room[0] = t_char;
	s_sth.s_items[item_no].s_no_stud = t_short;
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
	case SCR_13 :		/* SCR - 13 'TEACHER QUALIFICATION' */
	case SCR_15 :		/* SCR - 15 'COMPETITION' */
	case SCR_16 :		/* SCR - 16 'STATUS' */
		Cur_Option = s_sth.s_field ;
		return( JUMP ) ;
	default   : 
		return(NOERROR);
	}  /*   end of the switch statement */

	return(NOERROR);
}
/*----------------------------------------------------------------*/
CheckCode(item_no)
int	item_no;
{
	int	i;
	Page	*temppage;

	/* check to see if item is already in list */
	if(CurLast != NULL) {
	   for(temppage=FirstPage; temppage!=NULL;temppage=temppage->NextPage) {
	      for(i =0; i< temppage->NoItems; i++) {
		if(temppage->Items[i].s_cc == s_sth.s_items[item_no].s_cc && 
		 strcmp(temppage->Items[i].s_area_spec,
			s_sth.s_items[item_no].s_area_spec) == 0){ 
		 fomer("Assignment Information Already Entered on Screen"); 
			return(ERROR);
		}
	      }
	      if(temppage == CurLast) break;
	   }
	}

	return(NOERROR);
}

/*-------------------- E n d   O f   P r o g r a m ---------------------*/
