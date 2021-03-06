/*-----------------------------------------------------------------------
Source Name: stathol.c
System     : Personnel/Payroll System.
Created  On: 24th Sept. 91.
Created  By: C. Burns.

DESCRIPTION:
	Program To input the different Statutory Holidays.

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
------------------------------------------------------------------------*/
#define	MAIN		/* Main program. This is to declare Switches */
#define MAINFL		AREA		/* main file used */

#define	SYSTEM		"SETUP"	/* Sub System Name */
#define	MOD_DATE	"23-SEPT-91"		/* Progran Last Modified */

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
#define	LINEEDIT	'L'
#define SCREENEDIT	'S'
#define DELITEM		'D'
#define REACTITEM	'R'
#define	CANCEL		'C'

#define ACTIVE		"ACT"
#define INACTIVE		"DEL"
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
#define	LINEEDIT	'L'
#define SCREENEDIT	'S'
#define DELITEM		'D'
#define REACTITEM	'R'
#define	CANCEL		'A'

#define ACTIVE		"ACT"
#define INACTIVE		"ELI"
#endif

/* PROFOM Releted declarations */

#define	SCR_NAME	"stathol"	/* PROFOM screen Name */

#define	PAGESIZE	15		/* No of Items */
/* PROFOM Screen STH file */

/* Field PROFOM numbers */

#define FN_FLD		400	/* Fn: */

#define CHG_FLD		600
#define KEY_START	500
#define KEY_END		500

#define PAGE_FLD	800	/* Page# Field */
#define	ITEM_ST_FLD	900	/* Item 1 Start Field */
#define	END_FLD		5500	/* Last Field of the screen */
#define	STEP		300	/* NO of fields diff. between 2 items */

#define DATE_FLD	0	/* Offset of Stat Field in item line */
#define DESC_FLD	100	/* Offset of Desc Field in item line */
#define STATUS_FLD	200	/* Offset of Status in item line */

#define ITEM_STAT	900	/* First Stat Date field in items */
#define ITEM_DESC	1000	/* First Description field in items */

/* stathol.sth - header for C structure generated by PROFOM EDITOR */

typedef	struct	{	/* Start Fld 900, Endfld 5300 and Step 300 */

	long	s_date;	/* 900 DATE */
	char	s_desc[32];	/* 1000 STRING X(31) */
	char	s_status[4]; 	/* 1100 STRING X(3) */
}	S_item ;

typedef struct	{

	char	s_pgname[11];	/* 100 STRING X(10) */
	long	s_rundate;	/* 300 DATE YYYYFMMFDD */
	char	s_fn[2];	/* 400 STRING X */
	char	s_code[7];	/* 400 STRING X(7) */
	short	s_field;	/* 600 NUMERIC 99 */

	short	s_page;		/* 800 NUMERIC 99 */

	S_item	s_items[PAGESIZE] ;	/* Start Fld 900, End Fld 5300  */

	char	s_mesg[78];	/* 5400 STRING X(77) */
	char	s_resp[2];	/* 5500 STRING X */
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

static	Stat	stat_rec, pre_area;	/* Statutory record */
static	Barg_unit	barg_rec;	/* Bargaining Unit Record */

void	free() ;
char	*malloc() ;

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

	err = SelectRecord() ;
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
Delete()
{
	int	err ;

	err = SelectRecord() ;
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

	err = SelectRecord() ;
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

	if(flg_start(STAT_HOL) != direction) { 
		strcpy(stat_rec.s_code,s_sth.s_code);
		inc_str(stat_rec.s_code,sizeof(stat_rec.s_code)-1,direction);
		flg_reset(STAT_HOL);
	}

	err = get_n_stat(&stat_rec,BROWSE,0,direction,e_mesg);
	if(err < 0 && err != EFL) {
		DispError((char *)&s_sth,e_mesg);
		return(err);
	}

	if(err == EFL) {
		fomen("No More Records...");
		get();
		err = NOERROR;	
	}
	else {
		strcpy(s_sth.s_code,stat_rec.s_code);
		ret(WriteFields((char *)&s_sth,KEY_START,KEY_END));

		/* Get Statutory records i.e. Build list */
		err = BuildList();
		if(err < 0 && err != EFL) {
			return(err);
		}
	
		err = ShowItems(CurPage);
		if(err < 0) {
			return(err);
		}
	}

	return( err ) ;
}	/* Next() */
/*----------------------------------------------------------------------*/
/* In ADD mode disable dup buffers, other modes enable dup buffers and  */
/* show the current key as a default key 			        */
/*----------------------------------------------------------------------*/
ReadKey()
{
	int	i;
	char	temp_code[7];
	
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

	strcpy(temp_code,s_sth.s_code);

	i = ReadFields((char *)&s_sth,KEY_START, KEY_END,
		Validation, WindowHelp,1) ;
	if(PROFOM_ERR == i || DBH_ERR == i) return(i) ;
	if(RET_USER_ESC == i){
		strcpy(s_sth.s_code,temp_code);
		
		ret( WriteFields((char *)&s_sth,KEY_START, KEY_END) ) ;

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
			ret(WriteFields((char *)&s_sth,PAGE_FLD,PAGE_FLD) ) ;
			i = 0 ;
		}
	}


	for( ; ; ) {
		if( PAGESIZE == i) {	/* Page Full */

			/* Move High Values to All items except first */
			for(i-- ; i > 0 ; i--)
				InitItem(i, HV_CHAR, HV_LONG) ;

			/* Calculate the page# */
			s_sth.s_page = CurLast->Pageno + 1 ;

			ret( WriteFields((char *)&s_sth,PAGE_FLD, 
				(END_FLD - 200)) ) ;

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
SelectRecord()
{
	int	err ;

	err = ReadKey();
	if(err != NOERROR) return(err) ;

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
}	/* SelectRecord() */
/*------------------------------------------------------------*/
BuildList()
{
	int retval;
	int i;
	CurLast = CurPage = NULL;
	i = 0;

	strcpy(stat_rec.s_code,s_sth.s_code);
	stat_rec.s_date = 0 ;
	
	flg_reset(STAT_HOL);

	for( ; ; ) {
		retval = get_n_stat(&stat_rec,BROWSE,0,FORWARD,e_mesg);
		if(retval < 0) {
			if(retval == EFL) break;
			DispError((char *)&s_sth,e_mesg);
			return(retval);
		}

		if(strcmp(stat_rec.s_code,s_sth.s_code)!=0) { 
			break;
		}

		if(PAGESIZE == i) i = 0;
		if(i == 0) {
			if((retval = MakeFreshPage()) < 0) return(retval);
		}

		CurPage->Items[i].s_date = stat_rec.s_date;
		strcpy(CurPage->Items[i].s_desc,stat_rec.s_desc);
		strcpy(CurPage->Items[i].s_status,ACTIVE);
		CurPage->I_Status[i][0] = ' ';
		CurPage->NoItems++;
		i++;
	}
	seq_over(STAT_HOL);


	if(CurLast != NULL) {
		CurPage = FirstPage;
	}

	if(retval == EFL) return(retval);
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

		i = pageptr->NoItems ;
	}
	else {
		s_sth.s_page = HV_SHORT ;
		i = 0 ;
	}

	/* Move High Values to remaining Items */
	for( ; i < PAGESIZE ; i++ )
		InitItem(i, HV_CHAR, HV_LONG);

	ret( WriteFields((char *)&s_sth, PAGE_FLD, (END_FLD - 200)) );

	return(NOERROR) ;
}	/* ShowItems() */
/*-----------------------------------------------------------------------*/ 
/* Process all the items in the link list and write any changes to the   */
/* daily attendance file.						 */
/*-----------------------------------------------------------------------*/ 
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
	retval = commit(e_mesg) ;
	if(retval < 0) {
		DispError((char *)&s_sth,"ERROR: Saving Records"); 
		DispError((char *)&s_sth,e_mesg);
		roll_back(e_mesg);
		return(retval);
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

	if(mode != ADD) {
		scpy((char *)&pre_area,(char *)&stat_rec,
			sizeof(pre_area));

		strcpy(stat_rec.s_code,s_sth.s_code) ;
		stat_rec.s_date = temppage->Items[item_no].s_date;
		retval = get_stat(&stat_rec,UPDATE,0,e_mesg) ;
		if(retval < 0) {
			DispError((char *)&s_sth,e_mesg);
			roll_back(e_mesg);
			return(retval);
		}
	}

	strcpy(stat_rec.s_code,s_sth.s_code);
	stat_rec.s_date = temppage->Items[item_no].s_date;
	strcpy(stat_rec.s_desc,temppage->Items[item_no].s_desc);

	retval = put_stat(&stat_rec,mode,e_mesg) ;
	if(retval < 0) {
		DispError((char *)&s_sth,e_mesg);
		roll_back(e_mesg);
		return(retval);
	}

	if(mode != ADD) {
		retval = rite_audit((char*)&s_sth,STAT_HOL,mode,(char*)&stat_rec,
			(char*)&pre_area,e_mesg);
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
		st_fld  = ITEM_ST_FLD + (STEP * item_no) + DATE_FLD;
		end_fld  = ITEM_ST_FLD + (STEP * item_no) + STATUS_FLD;
		s_sth.s_items[item_no].s_date = LV_LONG;
		s_sth.s_items[item_no].s_desc[0] = LV_CHAR;
		strcpy(s_sth.s_items[item_no].s_status,ACTIVE);
	}
	else {
		st_fld  = ITEM_ST_FLD + (STEP * item_no) + DESC_FLD;
		end_fld  = ITEM_ST_FLD + (STEP * item_no) + DESC_FLD;
	}
	i = ReadFields((char *)&s_sth,st_fld,end_fld,Validation,WindowHelp,1) ;
	if(PROFOM_ERR == i || DBH_ERR == i) return(i) ;
	if(RET_USER_ESC == i) {
		if(mode == ADD) {
			InitItem(item_no,HV_CHAR, HV_LONG);
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
	int	fld_no;
	int	item_no;

	if(sr.curfld == KEY_START) {
		fld_no = KEY_START;
	}
	else {
		item_no = (sr.curfld - ITEM_ST_FLD - DATE_FLD) / STEP;
		fld_no = sr.curfld - (item_no * STEP);
	}

	switch(fld_no){
	case KEY_START:
		if(s_sth.s_code[0] == '\0') {
			fomer("This is a Required Field");
			return(ERROR);
		}
  		Right_Justify_Numeric(s_sth.s_code,sizeof(s_sth.s_code)-1);
		ret(WriteFields((char *)&s_sth,KEY_START,KEY_END));

		strcpy(barg_rec.b_code,s_sth.s_code);
		barg_rec.b_date = 0 ;
		flg_reset(BARG);
		retval = get_n_barg(&barg_rec,BROWSE,0,FORWARD,e_mesg);
		if(retval < 0 ){
			fomer(e_mesg);
			s_sth.s_code[0] = LV_CHAR;
			return(ERROR);
		}
		if(strcmp(barg_rec.b_code, s_sth.s_code) != 0) {
			fomen("Record not Available");
			get();
			s_sth.s_code[0] = LV_CHAR;
			return(ERROR);
		}
		break;
	case ITEM_STAT:
		retval = CheckStatRec(s_sth.s_items[item_no].s_date);
		if(retval == ERROR) {
			s_sth.s_items[item_no].s_date = LV_LONG;
			return(ERROR);
		}
		break;
	case ITEM_DESC:
		if(s_sth.s_items[item_no].s_desc[0] == '\0') {
			fomer("This is a Required Field");
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
CheckStatRec(statdate)
long	statdate;
{
	int	retval;
	int	i;
	Page	*temppage;

	if(statdate == 0) {
		fomer("This is a Required Field");
		return(ERROR);
	}

	strcpy(stat_rec.s_code,s_sth.s_code);
	stat_rec.s_date = statdate;
	retval = get_stat(&stat_rec,BROWSE,0,e_mesg);
	if(retval < 0) {
		if(retval != UNDEF) {
			DispError((char *)&s_sth,e_mesg);
			return(ERROR);
		}
	}
	else { if (retval == NOERROR) {
		fomer("Statutory Date Already Exists on File");
		return(ERROR);
	       }
	}

	/* check to see if item is already in list */
	if(CurLast != NULL) {
	   for(temppage=FirstPage; temppage!=NULL;temppage=temppage->NextPage) {
	      for(i =0; i< temppage->NoItems; i++) {
			if(temppage->Items[i].s_date == 0) {
				return(NOERROR);
			}
			if(temppage->Items[i].s_date == statdate) {
				fomer("Statutory Date Already Entered on Screen");
				return(ERROR);
			}
	      }
	      if(temppage == CurLast) break;
	   }
	}

	return(NOERROR);
}
/*----------------------------------------------------------------*/
/* Show Help Windows, if applicable, when user gives ESC-H in key
   and header fields */

WindowHelp()
{
	int	retval ;
	int	fld_no;
	int	item_no;
	long	eff_date;

	if(sr.curfld == KEY_START) {
		fld_no = KEY_START;
	}
	else {
		item_no = (sr.curfld - ITEM_ST_FLD - DATE_FLD) / STEP;
		fld_no = sr.curfld - (item_no * STEP);
	}

	switch(fld_no){
	case KEY_START:
		retval = barg_hlp(s_sth.s_code,&eff_date,7,13);
		if(retval == DBH_ERR) return(retval);
		if(retval >= 0) redraw();
		if(retval == 0) return(ERROR);
		if(retval < 0) return(ERROR);
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
		"Y(es), A(dd), S(creen), L(ine), D(el), R(eactivate), N(ext), P(rev), C(ancel)"
		,"YASLDRNPC");
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
	    case  SCREENEDIT:
		err = ScreenEdit();
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
/* Change screen.  Allows editing of the students on the screen        */
/*---------------------------------------------------------------------*/
ScreenEdit()
{
     	int i;
	int retval;
	int day;

	/* make copy of screen incase user presses ESC-F */
	scpy((char *)&image,(char *)&s_sth,sizeof(s_sth));

#ifdef ENGLISH
	strcpy(s_sth.s_mesg,"Press ESC-F to Terminate");
#else
	strcpy(s_sth.s_mesg,"Appuyer sur ESC-F pour terminer");
#endif
	DispMesgFld((char *)&s_sth);

	if(SetDupBuffers(ITEM_ST_FLD, END_FLD - 200,1)<0) return(PROFOM_ERR);

	for(i=0;i<CurPage->NoItems;i++) {
		if(strcmp(CurPage->Items[i].s_status,INACTIVE) ==0){
			continue;
		}
		retval = ReadItem(i,UPDATE);
		if(retval != NOERROR && retval != RET_USER_ESC) {
			return(retval);
		}

		if(retval == RET_USER_ESC) 
			return(retval);

		scpy((char*)&(CurPage->Items[i]), 
		     (char*)&(s_sth.s_items[i]),sizeof(S_item)) ;

		if(s_sth.s_fn[0] == CHANGE) {
			CurPage->I_Status[i][0] = CHANGE;
		}
	}

     	s_sth.s_mesg[0] = HV_CHAR ;
     	DispMesgFld((char *)&s_sth);

	if(SetDupBuffers(ITEM_ST_FLD, END_FLD-200,0)<0) return(PROFOM_ERR);

	return(NOERROR);
}	/* ScreenEdit() */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Changing fields. Accept fld of the student to be changed and read     */
/* that fld 		 */
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
		if (s_sth.s_field > CurPage->NoItems)
			continue;

       		if (s_sth.s_field == 0 ) break;  /* Finished changing fields */

		if (s_sth.s_field > CurPage->NoItems)
			continue;

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
/*------------------------------------------------------------------------*/
/* Move High values to all data fields and clear the screen */

ClearScreen()
{
	int	i;

	/* Move High Values to Hedaer part */
	s_sth.s_page = HV_SHORT;
	
	/* Move High Values to All items */
	for(i = 0 ; i < PAGESIZE ; i++)
		InitItem(i,HV_CHAR, HV_LONG);

	ret( WriteFields((char *)&s_sth,ITEM_ST_FLD, (END_FLD - 200)) );

	return(NOERROR);
}	/* ClearScreen() */
/*-------------------------------------------------------------------------*/
/* Initialize Given screen item with either Low values or High values */

InitItem(item_no, t_char, t_long)
int	item_no ;
char	t_char ;
long	t_long ;
{

	s_sth.s_items[item_no].s_date = t_long ;
	s_sth.s_items[item_no].s_desc[0] = t_char ;
	s_sth.s_items[item_no].s_status[0] = t_char ;

	return(NOERROR) ;
}	/* Inititem() */
/*-------------------- E n d   O f   P r o g r a m ---------------------*/
