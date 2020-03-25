/*-----------------------------------------------------------------------
Source Name: payper.c
System     : Personnel/Payroll System.
Created  On: 10th Sept. 91.
Created  By: J. Prescott.

DESCRIPTION:
	Used to enter the monthly pay periods for a different number of
	pay periods.  

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
------------------------------------------------------------------------*/
#define	MAIN		/* Main program. This is to declare Switches */
#define MAINFL		PAY_PERIOD		/* main file used */

#define	SYSTEM		"SETUP"	/* Sub System Name */
#define	MOD_DATE	"10-SEPT-91"		/* Progran Last Modified */

#include <stdio.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_pp.h>

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
#define	HEADEREDIT	'H'
#define	LINEEDIT	'E'
#define	DELITEM		'D'
#define	REACTITEM	'R'
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
#define	HEADEREDIT	'H'
#define	LINEEDIT	'E'
#define	DELITEM		'D'
#define	REACTITEM	'R'
#define	CANCEL		'A'

#define ACTIVE		"ACT"
#define INACTIVE	"ELI"
#endif

/* PROFOM Releted declarations */

#define	SCR_NAME	"payper"	/* PROFOM screen Name */

#define	PAGESIZE	12		/* No of Items */
/* PROFOM Screen STH file */

/* Field PROFOM numbers */

#define FN_FLD		400	/* Fn: */

#define KEY_START	500	/* Key Start Field */
#define KEY_END		550	/* Key End Field */
#define CHG_FLD		600

#define HDR_ST_FLD	800	/* Number of Pay Periods Field */
#define HDR_END_FLD	900	/* Description Field */

#define PAGE_FLD	1000	/* Page# Field */

#define	ITEM_ST_FLD	1300	/* Item Start Field of first line */
#define ITEM_END_FLD	1700	/* Item End Field of first line */
#define STEP		500	/* step from on item line to the next */

#define ITEM_START_DT	1400	/* Start Date Field */
#define ITEM_END_DT	1500	/* End Date Field */
#define	STATUS_FLD	400

#define	END_FLD		7400	/* Last Field of the screen */

/* area.sth - header for C structure generated by PROFOM EDITOR */

typedef	struct	{	/* Start Fld 1300, Endfld 6000 */

	short	s_pp_no;	/* 1300 NUMERIC 99 */
	long	s_start_dt;	/* 1400 DATE YYYYFMMFDD */
	long	s_end_dt;	/* 1500 DATE YYYYFMMFDD */
	short	s_monthly_pp;	/* 1600 NUMERIC 9 */
	char	s_status[6];	/* 1650 STRING X */

}	S_item ;

typedef struct	{

	char	s_pgname[11];	/* 100 STRING X(10) */
	long	s_rundate;	/* 300 DATE YYYYFMMFDD */
	char	s_fn[2];	/* 400 STRING X */
	char	s_pp_code[7];	/* 500 STRING X(6) */
	short	s_pp_year;	/* 550 short 9999 */
	short	s_field;	/* 600 NUMERIC 99 */

	short	s_nbr_pps;	/* 800 NUMERIC 99 */
	char	s_desc[31];	/* 900 STRING X(30) */
	short	s_page;		/* 1000 NUMERIC 99 */

	char	s_dummy1[4];	/* 1100 STRING XXX */
	char	s_dummy2[2];	/* 1200 STRING X */

	S_item	s_items[PAGESIZE] ;	/* Start Fld 1300, End Fld 6000  */

	char	s_mesg[78];	/* 6100 STRING X(77) */
	char	s_resp[2];	/* 6200 STRING X */
} s_struct;

static	s_struct  s_sth,image;	/* PROFOM Screen Structure */
static	struct  stat_rec  sr;		/* PROFOM status rec */

static	char 	e_mesg[100];  		/* dbh will return err msg in this */

int	Validation() ;
int	WindowHelp() ;

int	Argc;
char	**Argv;

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

static	Pay_per	pay_per, pre_payper;
static	Pay_per_it pp_item, pre_pp_item;

void	free() ;
char	*malloc() ;

static	int  prev_month;
static	short mthly_pp;

main(argc,argv)
int argc;
char *argv[];
{
	int 	retval;

	/* These two are passed to execute() when it is called */
	Argc = argc;
	Argv = argv;

	retval = Initialize(argc,argv);	/* Initialization routine */

	if (retval == NOERROR) retval = Process();

	CloseRtn();			/* return to menu */
	if (retval != NOERROR) exit(-1);
	exit(0);
}

/*-------------------------------------------------------------------*/
/* Initialize PROFOM */

Initialize(argc, argv)
int	argc ;
char	*argv[] ;
{
	int	err ;

	/*
	*	Initialize DBH Environment
	*/
	strncpy(SYS_NAME,SYSTEM,50);	/* Sub system name */
	strncpy(CHNG_DATE,MOD_DATE,10);	/* Modification Date */

	proc_switch(argc, argv, MAINFL) ; 	/* Process Switches */

	/*
	*	Initialize PROFOM & Screen
	*/
	strcpy(sr.termnm,terminal);	/* Copy Terminal Name */
	fomin(&sr);
	ret(err_chk(&sr)) ;		/* Check for PROFOM Error */
	fomcf(1,1);			/* Enable Snap screen option */

	err = InitScreen() ;		/* Initialize Screen */
	if(NOERROR != err) return(err) ;

	FirstPage = NULL;
	LastPage = NULL;

	return(NOERROR) ;
}	/* Initialize() */

/*--------------------------------------------------------------------------*/
/* Close nessary files and environment before exiting program               */

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

InitScreen()
{
	int	err ;

	/* move screen name to Profom status structure */
	strcpy(sr.scrnam,NFM_PATH);
	strcat(sr.scrnam,SCR_NAME) ;

	strcpy(s_sth.s_pgname,PROG_NAME);

	s_sth.s_rundate = get_date();	/* get Today's Date in YYYYMMDD format*/
	s_sth.s_field = HV_SHORT ;
	s_sth.s_mesg[0] = HV_CHAR ;
	s_sth.s_resp[0] = HV_CHAR ;

	/* Move High Values to data fields and Display the screen */
	err = ClearScreen() ;
	if(NOERROR != err) return(err) ;

	return(NOERROR) ;
}	/* InitScreen() */
/*-------------------------------------------------------------------*/
/* Get Option from user and call corresponding function */

Process()
{
	int err;

	for( ; ; ){

		/* Get the Fn: option from the user */
		if((err = ReadFunction()) != NOERROR) return(err) ;

		err = ProcFunction() ;	/* Process Function */

		if(QUIT == err)		return(NOERROR) ;    /* Exit */
		if(NOACCESS == err)	fomen(e_mesg);	     /* security */
		if(PROFOM_ERR == err)	return(PROFOM_ERR);  /* PROFOM ERROR */
		if(DBH_ERR == err) {
			DispError((char *)&s_sth,e_mesg);
#ifdef ENGLISH
			sprintf(e_mesg,"%s %d Dberror: %d Errno: %d",
				"System Error... Iserror:",
				iserror, dberror, errno);
#else
			sprintf(e_mesg,"%s %d Dberror: %d Errno: %d",
				"Erreur du systeme... Iserror:",
				iserror, dberror, errno);
#endif
			DispError((char *)&s_sth,e_mesg);
			return(DBH_ERR); /* DBH ERROR */
		}
	}      /*   end of the for( ; ; )       */
}	/* Process() */
/*----------------------------------------------------------------*/
/* Display the Function (Fn:) options and get the option from the user */

ReadFunction()
{
	/* Display Fn: options */
#ifdef ENGLISH
	fomer("A(dd), C(hange), D(elete), I(nquire), N(ext), P(rev), E(xit)");
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

ProcFunction()
{
	int retval;

	switch (s_sth.s_fn[0]) {
	case ADDREC  :			/* ADD */
		CHKACC(retval,ADD,e_mesg);
		return( Add() ) ;
	case CHANGE  :			/* CHANGE */
		CHKACC(retval,UPDATE,e_mesg);
		return( Change() ) ;
	case DELETE  :			/* CHANGE */
		CHKACC(retval,UPDATE,e_mesg);
		return( Delete() ) ;
	case INQUIRE  :			/* Inquire */
		CHKACC(retval,BROWSE,e_mesg);
		return( Inquire() ) ;
	case NEXT  :			/* Next */
		CHKACC(retval,BROWSE,e_mesg);
		return( Next(FORWARD) ) ;
	case PREV  :			/* Previous */
		CHKACC(retval,BROWSE,e_mesg);
		return( Next(BACKWARD) ) ;
	case EXITOPT  :			/* Exit */
		return( QUIT ) ;
	}  /*   end of the switch statement */

	return(retval);
}	/* ProcFunction() */
/*----------------------------------------------------------------------*/
/* Adding.  Get the unique Key, accept details and update the files */
Add()
{
	int	err ;

	FirstPage = NULL;
	LastPage = NULL;
	CurLast = NULL;

	err = ReadKey();
	if(err != NOERROR) return(err) ;

	/* Clear The Screen */
	err = ClearScreen();
	if(err != NOERROR) return(err) ;

	FreeList();

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
Change()
{
	int	err ;

	err = SelectRecord(UPDATE) ;
	if(err < 0) return(err);

	for( ; ; ) {
		err = ConfirmItems() ;
		if(err == CANCEL)	break;
		if(err != YES) {
			roll_back(e_mesg);
			continue;
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

Next(direction)
int	direction ;
{
	int err;

	if(flg_start(PAY_PERIOD) != direction) {
		strcpy(pay_per.pp_code,s_sth.s_pp_code);
		pay_per.pp_year = s_sth.s_pp_year;
		inc_str(pay_per.pp_code,sizeof(pay_per.pp_code)-1,direction);
		flg_reset(PAY_PERIOD);
	}
	err = get_n_pay_per(&pay_per,BROWSE,0,direction,e_mesg);
#ifndef ORACLE
	seq_over(PAY_PERIOD);
#endif

	if(ERROR == err) return(DBH_ERR);
	if(EFL == err) {
#ifdef ENGLISH
		fomen("No More Records....");
#else
		fomen("Plus de fiches....");
#endif
		get();
		return(NOERROR);
	}

	return(ShowScreen());
}	/* Next() */
/*----------------------------------------------------------------------*/
/* Get the Student Study Hall key from user. In ADD mode disable dup buffers, */
/* other modes enable dup buffers and show the current key as a default key */
/*----------------------------------------------------------------------*/
ReadKey()
{
	int	i;
	char	temp_code[7];
	short	temp_year;
	
	/* In Add mode turn off dup control for key fields.
	   Other modes reverse it */
	if(s_sth.s_fn[0] == ADDREC){	/* ADD */
		SetDupBuffers(KEY_START,KEY_END,0); /* Off Dup Control */
	}
	else {
		SetDupBuffers(KEY_START,KEY_END,1);
	}

	strcpy(temp_code,s_sth.s_pp_code);
	temp_year = s_sth.s_pp_year;

#ifdef ENGLISH
	strcpy(s_sth.s_mesg,"Press ESC-F to Go Back to Fn:");
#else
	strcpy(s_sth.s_mesg,"Appuyer sur ESC-F pour retourner a Fn:");
#endif
	DispMesgFld((char *)&s_sth);

	s_sth.s_pp_code[0] = LV_CHAR;
	s_sth.s_pp_year = LV_SHORT;

	i = ReadFields((char *)&s_sth,KEY_START, KEY_END,
		Validation, WindowHelp,1) ;
	if(PROFOM_ERR == i || DBH_ERR == i) return(i) ;
	if(RET_USER_ESC == i){
		if(s_sth.s_fn[0] != ADDREC) {
			strcpy(s_sth.s_pp_code,temp_code);
			s_sth.s_pp_year = temp_year;
			ret( WriteFields((char *)&s_sth,KEY_START, KEY_END) ) ;
		}
		s_sth.s_mesg[0] = HV_CHAR;
		DispMesgFld((char *)&s_sth);

		return(RET_USER_ESC) ;
	}

	return(NOERROR);
}	/*  ReadKey() */
/*------------------------------------------------------------*/
/* Read the Area Details from the User */

GetDetails()
{
	int	retval ;

	retval = ReadHdr(ADD);
	if(retval < 0) return(retval);

	if(retval != RET_USER_ESC){
		retval = AddItems();
		if(retval < 0) return(retval) ;
	}

	for( ; ; ) {
		retval = ConfirmItems() ;
		if(retval != YES) break;

		retval = ProcItemUpdates(ADD) ;
		if(retval < 0) {
			if(retval == LOCKED) continue;
			break;
		}
		break;
	}
	if(retval != NOERROR) return(retval);
	return(NOERROR) ;
}	/* GetDetails() */
/*------------------------------------------------------------*/
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
	  if(mode == ADD){
		InitHdr(HV_CHAR,HV_SHORT);
		WriteFields((char *)&s_sth,HDR_ST_FLD,HDR_END_FLD);
	  	return(RET_USER_ESC) ;
	  }
	  retval = CopyBack((char *)&s_sth,
				(char *)&image,HDR_ST_FLD,HDR_END_FLD);
	  if(retval == PROFOM_ERR) return(retval);

	  return(RET_USER_ESC) ;
	}
	return(NOERROR);
}
/*------------------------------------------------------------*/
/* Read Item Details from the User */

AddItems()
{
	int	i, err ;
	int	k;

	prev_month = 0;
	mthly_pp = 0;
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
			s_sth.s_dummy2[0] = ' ';
			s_sth.s_page = 1 ;
			ret( WriteFields((char *)&s_sth,PAGE_FLD,PAGE_FLD+200));
			i = 0 ;
		}
	}

	for(k=0;k<s_sth.s_nbr_pps;k++) {
		if( PAGESIZE == i) {	/* Page Full */

			for(i-- ; i > 0 ; i-- )
				InitItem(i,HV_SHORT,HV_LONG);

			/* Calculate the page# */
			s_sth.s_page = CurLast->Pageno + 1 ;

			ret( WriteFields((char *)&s_sth,PAGE_FLD, 
				(END_FLD - 200)) ) ;

			i = 0 ;
		}

		err = ReadItem(i,ADD) ;		/* Read Each Item Line */
		if(PROFOM_ERR == err || DBH_ERR == err) return(err) ;
		if(NOERROR != err) break ;

		if(0 == i)	/* First Item in the Page */
			if((err = MakeFreshPage()) < 0) return(err) ;
		
		/* Copy the Item to List */
		scpy((char*)&(CurPage->Items[i]), (char*)&(s_sth.s_items[i]),
			sizeof(S_item)) ;

		prev_month = (s_sth.s_items[i].s_end_dt / 100 ) % 100;

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

MakeFreshPage()
{
	Page	*tempptr ;

	/* If, no node is allocated yet or Current invoice used all the nodes,
	   then allocate new node */

	if( LastPage == NULL || CurLast == LastPage ){
		tempptr= (Page *)malloc((unsigned)sizeof(Page)) ;

		if( tempptr == NULL ){
			DispError((char *)&s_sth,"Memory Allocation Error");
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
SelectRecord(mode)
int	mode;
{
	int	err ;

	err = ReadKey();
	if(err != NOERROR) return(err) ;

	strcpy(pay_per.pp_code,s_sth.s_pp_code);
	pay_per.pp_year = s_sth.s_pp_year;
	err = get_pay_per(&pay_per,mode,0,e_mesg);
	if(err < 0){
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
BuildList()
{
	int retval;
	int i;
	int j;
	CurLast = CurPage = NULL;
	i = 0;

	FreeList();
	
	strcpy(pp_item.ppi_code,pay_per.pp_code);
	pp_item.ppi_year = pay_per.pp_year;
	pp_item.ppi_numb = 0;
	flg_reset(PAY_PER_ITEM);

	for( ; ; ) {
		retval = get_n_pp_it(&pp_item,BROWSE,0,FORWARD,e_mesg);
		if(retval < 0) {
			if(retval == EFL) break;
			DispError((char *)&s_sth,e_mesg);
			return(retval);
		}

		if(strcmp(pp_item.ppi_code,pay_per.pp_code)!=0 ||
		   pp_item.ppi_year != pay_per.pp_year) { 
			break;
		}

		if(PAGESIZE == i) i = 0;
		if(i == 0) {
			if((retval = MakeFreshPage()) < 0) return(retval);
		}

		CurPage->Items[i].s_pp_no = pp_item.ppi_numb;
		CurPage->Items[i].s_start_dt = pp_item.ppi_st_date;
		CurPage->Items[i].s_end_dt = pp_item.ppi_end_date;
		CurPage->Items[i].s_monthly_pp = pp_item.ppi_mthly;
		strcpy(CurPage->Items[i].s_status, "ACT");

		CurPage->I_Status[i][0] = ' ';
		CurPage->NoItems++;
		i++;
	}
	seq_over(PAY_PER_ITEM);


	if(CurLast != NULL) {
		CurPage = FirstPage;
	}

	if(retval == EFL) return(retval);
	return(NOERROR);
}
/*------------------------------------------------------------*/
ShowHdr() 
{
	int	retval;
	
	/* Copy Record to Screen */
	strcpy(s_sth.s_pp_code,pay_per.pp_code);
	s_sth.s_pp_year = pay_per.pp_year;
	
	s_sth.s_nbr_pps = pay_per.pp_numb;
	strcpy(s_sth.s_desc,pay_per.pp_desc);

	ret( WriteFields((char *)&s_sth, KEY_START, HDR_END_FLD) );
	
	return(NOERROR);
}
/*------------------------------------------------------------*/
/* Show all the items on the current page 		      */
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
		InitItem(i,HV_SHORT,HV_LONG);

	ret( WriteFields((char *)&s_sth, PAGE_FLD, (END_FLD - 200)) );

	return(NOERROR) ;
}	/* ShowItems() */
/*-----------------------------------------------------------------------*/ 
/* Process all the items in the link list and write any changes to the   */
/* file.								 */
/*-----------------------------------------------------------------------*/ 
ProcItemUpdates(mode)
int	mode;
{
	Page	*temppage;
	int	i;
	int	retval;
	int	write_mode;

	scpy((char *)&pre_payper,(char *)&pay_per,sizeof(pay_per));
	
	/* Write Header */
	strcpy(pay_per.pp_code,s_sth.s_pp_code);
	pay_per.pp_year = s_sth.s_pp_year;
	pay_per.pp_numb = s_sth.s_nbr_pps;
	strcpy(pay_per.pp_desc,s_sth.s_desc);

	retval = put_pay_per(&pay_per,mode,e_mesg);
	if(retval < 0) {
		DispError((char *)&s_sth,e_mesg);
		roll_back(e_mesg);
		return(retval);
	}

	if(mode != ADD) {
		retval = rite_audit((char*)&s_sth,PAY_PERIOD,mode,(char*)&pay_per,
			(char*)&pre_payper,e_mesg);
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
		   	return(retval);
		 }
	      }
	      if(temppage == CurLast) break;
	   }
	}
	return(NOERROR);
}
/*-----------------------------------------------------------------------*/ 
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
WriteRecords(temppage,item_no,mode)
Page	*temppage;
int	item_no;
int	mode;
{
	int	retval;
	int	i;

	scpy((char *)&pre_pp_item,(char *)&pp_item,sizeof(pp_item));

	strcpy(pp_item.ppi_code,s_sth.s_pp_code);
	pp_item.ppi_year = s_sth.s_pp_year;
	pp_item.ppi_numb = temppage->Items[item_no].s_pp_no;
	if(mode != ADD) {
		retval = get_pp_it(&pp_item,UPDATE,0,e_mesg) ;
		if(retval < 0) {
			DispError((char *)&s_sth,e_mesg);
			roll_back(e_mesg);
			return(retval);
		}
	}
	pp_item.ppi_st_date = temppage->Items[item_no].s_start_dt;
	pp_item.ppi_end_date = temppage->Items[item_no].s_end_dt;
	pp_item.ppi_mthly = temppage->Items[item_no].s_monthly_pp;

	retval = put_pp_it(&pp_item,mode,e_mesg) ;
	if(retval < 0) {
		DispError((char *)&s_sth,e_mesg);
		roll_back(e_mesg);
		return(retval);
	}

	if(mode != ADD) {
		retval = rite_audit((char*)&s_sth,PAY_PER_ITEM,mode,(char*)&pp_item,
			(char*)&pre_pp_item,e_mesg);
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

	retval = commit(e_mesg) ;
	if(retval < 0) {
		DispError((char *)&s_sth,"ERROR: Saving Records"); 
		DispError((char *)&s_sth,e_mesg);
		roll_back(e_mesg);
		return(retval);
	}
	return(NOERROR);
}
/*------------------------------------------------------------*/
/* Read details of given item# */
/*------------------------------------------------------------*/
ReadItem(item_no,mode)
int	item_no ;
int	mode ;
{
	int	i;
	int	st_fld ;
	int	end_fld ;
	long	next_st_date;

#ifdef ENGLISH
	strcpy(s_sth.s_mesg,"Press ESC-F to Terminate");
#else
	strcpy(s_sth.s_mesg,"Appuyer sur ESC-F pour terminer");
#endif
	DispMesgFld((char *)&s_sth);

	st_fld  = ITEM_ST_FLD + (item_no * STEP);
	end_fld  = ITEM_END_FLD + (item_no * STEP);

	if(mode == ADD) {

		if(item_no != 0 ) {
			s_sth.s_items[item_no].s_pp_no = 
				s_sth.s_items[item_no-1].s_pp_no + 1;
			next_st_date =
				 date_plus(s_sth.s_items[item_no-1].s_end_dt,1);
			s_sth.s_items[item_no].s_start_dt = next_st_date;
			SetDupBuffers(st_fld, end_fld,1);
		}
		else if(item_no == 0 && s_sth.s_page != 1) {
			s_sth.s_items[item_no].s_pp_no = 
				CurPage->Items[11].s_pp_no + 1;
			next_st_date =
				 date_plus(CurPage->Items[11].s_end_dt,1);
			s_sth.s_items[item_no].s_start_dt = next_st_date;
			s_sth.s_items[item_no].s_end_dt = 0;
			s_sth.s_items[item_no].s_monthly_pp = 0;
			SetDupBuffers(st_fld, end_fld,1);
		}
		else
			SetDupBuffers(ITEM_ST_FLD, END_FLD - 200,0);
	}
	else {
		SetDupBuffers(ITEM_ST_FLD, END_FLD - 200,1);
	}

	InitItem(item_no,LV_SHORT,LV_LONG);

	i = ReadFields((char *)&s_sth,st_fld,end_fld,Validation,WindowHelp,1) ;
	if(PROFOM_ERR == i || DBH_ERR == i) return(i) ;
	if(RET_USER_ESC == i) {
		if(mode == ADD) {
			InitItem(item_no,HV_SHORT,HV_LONG);
		        WriteFields((char *)&s_sth,ITEM_ST_FLD,END_FLD-200);
			return(RET_USER_ESC);
		}
		/* When user gives ESC-F while changing fields, assumption is
		* he completed his changes, and remaining fields are same as
		* old. But, at this point STH will be having low values in the 
		* remaining fields. So move the old values form the linked list.
		*/

		i = CopyBack((char *)&s_sth,(char *)&image,ITEM_ST_FLD,END_FLD);
		if(i == PROFOM_ERR) return(i);

		return(RET_USER_ESC) ;
	}

	return(NOERROR) ;
}	/* ReadItem() */
/*----------------------------------------------------------------*/
/* Set Duplication buffers for fields 				  */
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

Validation()
{
	int	retval;
	int	item_no;
	int	fld_no;
	long	next_st_date;

	if(sr.curfld >= ITEM_ST_FLD) {
		item_no = (sr.curfld - ITEM_ST_FLD) / STEP;
		fld_no = sr.curfld - (item_no * STEP);
	}
	else {
		fld_no = sr.curfld;
	}
	switch(fld_no){
	case KEY_START:
		Right_Justify_Numeric(s_sth.s_pp_code,
			sizeof(s_sth.s_pp_code)-1);
		strcpy(pay_per.pp_code,s_sth.s_pp_code);
		pay_per.pp_year = 0;
		flg_reset(PAY_PERIOD);
		retval = get_n_pay_per(&pay_per,BROWSE,0,FORWARD,e_mesg);
		if((retval < 0 || strcmp(pay_per.pp_code,s_sth.s_pp_code)!=0)&&
		  s_sth.s_fn[0] != ADDREC) {
			fomer("Pay Period Code Does not Exist");
			s_sth.s_pp_code[0] = LV_CHAR;
			return(ERROR);
		}
		seq_over(PAY_PERIOD);
		break;
	case KEY_END:
		strcpy(pay_per.pp_code,s_sth.s_pp_code);
		pay_per.pp_year = s_sth.s_pp_year;
		retval = get_pay_per(&pay_per,BROWSE,0,e_mesg);
		if(retval < 0 && s_sth.s_fn[0] != ADDREC) {
			fomer("Pay Period Code Does not Exist");
			s_sth.s_pp_code[0] = LV_CHAR;
			s_sth.s_pp_year = LV_SHORT;
			sr.curfld -= 50;
			return(ERROR);
		}
		else if(retval != UNDEF && s_sth.s_fn[0] == ADDREC) {
			fomer("Pay Period Code Already Exists");
			s_sth.s_pp_code[0] = LV_CHAR;
			s_sth.s_pp_year = LV_SHORT;
			sr.curfld -= 50;
			return(ERROR);
		}
		if(strcmp(pay_per.pp_code,s_sth.s_pp_code)!=0 &&
		   s_sth.s_pp_year != pay_per.pp_year) {
			fomer("Pay Period Code Does not Exist");
			s_sth.s_pp_code[0] = LV_CHAR;
			s_sth.s_pp_year = LV_SHORT;
			sr.curfld -= 50;
			return(ERROR);
		}
		break;
	case HDR_ST_FLD:
		break;
	case HDR_END_FLD:
		if(s_sth.s_desc[0] == '\0') {
			fomer("This is a Required Field");
			return(ERROR);
		}
		break;
	case ITEM_START_DT:
		if(s_sth.s_items[item_no].s_start_dt == 0) {
			fomer("Invalid Date");
			s_sth.s_items[item_no].s_start_dt = LV_LONG;
			return(ERROR);
		}
		if(item_no > 0) {
		  if(s_sth.s_items[item_no].s_start_dt <= 
		  	 s_sth.s_items[item_no-1].s_end_dt) {
		fomer("Start Date Must be Greater Than Previous End Date");
			s_sth.s_items[item_no].s_start_dt = LV_LONG;
			return(ERROR);
		  }
		}
		break;
	case ITEM_END_DT:
		if(s_sth.s_items[item_no].s_end_dt <= 
		   s_sth.s_items[item_no].s_start_dt) {
			fomer("End Date Must be Greater Than Start Date");
			s_sth.s_items[item_no].s_end_dt = LV_LONG;
			return(ERROR);
		}

		if((s_sth.s_items[item_no].s_end_dt / 100)%100 == prev_month) {
			mthly_pp++;
			s_sth.s_items[item_no].s_monthly_pp = mthly_pp;
		}
		else {
			s_sth.s_items[item_no].s_monthly_pp = 1;
			mthly_pp = 1;
		}
		if(SetDupBuffers(sr.curfld+100,sr.curfld+100,1)<0) return(PROFOM_ERR);
		s_sth.s_items[item_no].s_monthly_pp = LV_SHORT;
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

WindowHelp()
{
	int	retval ;
	int	item_no;
	long	eff_date;

	switch(sr.curfld){
	case KEY_START:
		retval = payper_hlp(s_sth.s_pp_code,7,13);
		if(retval == DBH_ERR) return(retval);
		if(retval >= 0) redraw();
		if(retval == 0) return(ERROR);
		if(retval < 0) return(ERROR);
		strcpy(pay_per.pp_code,s_sth.s_pp_code);
		pay_per.pp_year = 0;
		flg_reset(PAY_PERIOD);
		retval = get_n_pay_per(&pay_per,BROWSE,0,FORWARD,e_mesg);
		if((retval < 0 || strcmp(pay_per.pp_code,
					s_sth.s_pp_code)!=0)&& 
		   s_sth.s_fn[0] != ADDREC) {
			fomer("Error Reading Pay Period File");
			s_sth.s_pp_code[0] = LV_CHAR;
			return(ERROR);
		}
		seq_over(PAY_PERIOD);
		break;
	case HDR_ST_FLD:
		/***
		retval = uic_hlp(&s_sth.s_nbr_pps,&eff_date,7,13);
		if(retval == DBH_ERR) return(retval);
		if(retval >= 0) redraw();
		if(retval == 0) return(ERROR);
		if(retval < 0) return(ERROR);
		***/
		break;
	default :
		fomer("No Help Window for This Field");
	}	/* Switch sr.curfld */

	return(NOERROR) ;
}	/* HdrAndKeyWindowHelp() */
/*-----------------------------------------------------------------------*/
/* Take the confirmation from user for the items part of the screen      */
/*-----------------------------------------------------------------------*/
ConfirmItems()
{
	int	err ;

	/* Options:
	     YSLNPC
	*/

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
		"Y(es), H(eader), A(dd), E(dit), D(el), R(eactivate), N(ext), P(rev), C(ancel)"
		,"YHAEDRNPC");
#else
			err = GetOption((char *)&s_sth,
		"O(ui), E(ntete), R(ajouter), M(odifier), S(uiv), P(rec), A(nnul)"
		,"OHRMSPA");
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
	    case  HEADEREDIT  :
		err = ReadHdr();
		if(err < 0) return(err);
		break ;
	    case  ADDITEMS  :
		err = AddItems();
		if(err < 0) return(err);
		break ;
	    case  LINEEDIT  :
		err = LineEdit();
		if(err < 0) return(err);
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
	if(retval <0)
		return(retval);

	if(retval == RET_USER_ESC) 
		return(retval);

     	s_sth.s_mesg[0] = HV_CHAR ;
     	DispMesgFld((char *)&s_sth);

	if(SetDupBuffers(HDR_ST_FLD, HDR_END_FLD,0)<0) return(PROFOM_ERR);

	return(NOERROR);
}	/* HeaderEdit() */

/*---------------------------------------------------------------------*/
/* Change Line items.  Allows editing of the students on the screen    */
/*---------------------------------------------------------------------*/
LineEdit()
{
     	int i;
	int retval;
	Page	*temppage;

	/* make copy of screen incase user presses ESC-F */
	scpy((char *)&image,(char *)&s_sth,sizeof(s_sth));

#ifdef ENGLISH
	strcpy(s_sth.s_mesg,"Press ESC-F to Terminate");
#else
	strcpy(s_sth.s_mesg,"Appuyer sur ESC-F pour terminer");
#endif
	DispMesgFld((char *)&s_sth);

	if(SetDupBuffers(ITEM_ST_FLD, END_FLD - 200,1)<0) return(PROFOM_ERR);

	if(CurLast != NULL) {
	   for(temppage=CurPage; temppage!=NULL;temppage=temppage->NextPage) {
	      for(i =0; i< temppage->NoItems; i++) {
		if(s_sth.s_page != 1 && i == 0) {
		   temppage = temppage->PrevPage;
		   /*next_st_date = date_plus(temppage->Items[11].s_end_dt,1);*/
		   temppage = temppage->NextPage;
		}
		if((retval=ShowItems(temppage))<0) return(retval) ;
		retval = ReadItem(i,UPDATE);
		if(retval < 0)
			return(retval);

		if(retval == RET_USER_ESC) {
		i = CopyBack((char *)&s_sth,(char *)&image,ITEM_ST_FLD,END_FLD);
		if(i == PROFOM_ERR) return(i);
			return(retval);
		}

		scpy((char*)&(temppage->Items[i]), 
		     (char*)&(s_sth.s_items[i]),sizeof(S_item)) ;
		if(s_sth.s_fn[0] == CHANGE) {
			CurPage->I_Status[i][0] = CHANGE;
		}
		/*next_st_date = date_plus(s_sth.s_items[i].s_end_dt,1);*/
		prev_month = (s_sth.s_items[i].s_end_dt / 100 ) % 100;

	      }
	      if(temppage == CurLast) break;
	   }
	}

     	s_sth.s_mesg[0] = HV_CHAR ;
     	DispMesgFld((char *)&s_sth);

	if(SetDupBuffers(ITEM_ST_FLD, END_FLD-200,0)<0) return(PROFOM_ERR);

	return(NOERROR);
}	/* LineEdit() */
/*------------------------------------------------------------------------*/
/* Move High values to all data fields and clear the screen */

ClearScreen()
{
	int	i;

	/* Move High Values to Hedaer part */
	s_sth.s_page = HV_SHORT;
	s_sth.s_dummy1[0] = HV_CHAR;
	s_sth.s_dummy2[0] = HV_CHAR;
	
	InitHdr(HV_CHAR,HV_SHORT);

	/* Move High Values to The all item */
	for(i=0;i<PAGESIZE;i++) {
		InitItem(i,HV_SHORT,HV_LONG);
	}
	s_sth.s_mesg[0] = HV_CHAR ;
	s_sth.s_resp[0] = HV_CHAR ;
	ret( WriteFields((char *)&s_sth,HDR_ST_FLD, END_FLD ) );

	return(NOERROR);
}	/* ClearScreen() */
/*-------------------------------------------------------------------------*/
InitHdr(t_char,t_short)
char	t_char;
short	t_short;
{
	s_sth.s_nbr_pps = t_short;
	s_sth.s_desc[0] = t_char;

	return(NOERROR);
}
/*-------------------------------------------------------------------------*/
/* Initialize Given screen item with either Low values or High values */

InitItem(item_no,t_short, t_long)
int	item_no;
short	t_short ;
long	t_long ;
{
/*
	if(t_short == HV_SHORT) {
		s_sth.s_items[item_no].s_pp_no = t_short ;
	}
	if(s_sth.s_page == 1 && item_no == 0) {
		s_sth.s_items[item_no].s_start_dt = LV_LONG ;
	}
	if(t_long == HV_LONG) {
		s_sth.s_items[item_no].s_start_dt = HV_LONG ;
	}
*/
	s_sth.s_items[item_no].s_pp_no = t_short ;
	s_sth.s_items[item_no].s_start_dt = t_long ;
	s_sth.s_items[item_no].s_end_dt = t_long ;
	s_sth.s_items[item_no].s_monthly_pp = t_short ;
	if(t_long == HV_LONG)
		s_sth.s_items[item_no].s_status[0] = HV_CHAR;
	else
		s_sth.s_items[item_no].s_status[0] = ' ';

	return(NOERROR) ;
}	/* Inititem() */

/* Free the linked list */
static
FreeList()	
{
	int 	i;

	/* clear the screen items from linked list */

	for(CurPage = FirstPage;CurPage;CurPage = CurPage->NextPage){

		for( i=0; i <= PAGESIZE; i++) {
			if(i >= CurPage->NoItems) break;
		}
	}

	for( CurPage=LastPage; CurPage; CurPage=LastPage){
		LastPage=LastPage->PrevPage;
		free((char *)CurPage );
	}

	FirstPage = NULL;

	return(NOERROR);
}
/*------------------------------------------------------------------------*/
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
		if (s_sth.s_field > CurPage->NoItems)
			continue;

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
/*-------------------- E n d   O f   P r o g r a m ---------------------*/