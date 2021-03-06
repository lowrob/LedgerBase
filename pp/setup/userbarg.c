/*-----------------------------------------------------------------------
Source Name: userbarg.c
System     : Personnel/Payroll System.
Created  On: June 8, 1992.
Created  By: Eugene Roy.

DESCRIPTION:
	The UserBargaining Unit Screen is used to identify the 
	Bargaining Units each user as access to.

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
------------------------------------------------------------------------*/
#define	MAIN		/* Main program. This is to declare Switches */
#define MAINFL		USERBARG		/* main file used */

#define	SYSTEM		"SETUP"	/* Sub System Name */
#define	MOD_DATE	"8-JUNE-92"		/* Progran Last Modified */

#include <stdio.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_pp.h>
#include <bfs_com.h>

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
#define	SCREENEDIT	'S'
#define	LINEEDIT	'L'
#define HEADEREDIT	'H'
#define DELITEM		'D'
#define REACTITEM	'R'
#define	CANCEL		'C'

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
#define	SCREENEDIT	'S'
#define	LINEEDIT	'L'
#define HEADEREDIT	'H'
#define DELITEM		'D'
#define REACTITEM	'R'
#define	CANCEL		'A'

#endif

#define FOUND		0
#define NOTFOUND	1
#define NO_CHANGE	2

/* PROFOM Releted declarations */

#define	SCR_NAME	"userbarg"	/* PROFOM screen Name */

#define	PAGESIZE	14		/* No of Items */
/* PROFOM Screen STH file */

/* Field PROFOM numbers */

#define FN_FLD		400	/* Fn: */

#define KEY_START	500	/* Key Start Field */
#define KEY_END		500	/* Key End Field */
#define CHG_FLD		600

#define PAGE_FLD	700	/* Page# Field */

#define	ITEM_ST_FLD	1000	/* Item Start Field */
#define	ADD_FLG		100
#define	UPDATE_FLG	200
#define	DELETE_FLG	300
#define	BROWSE_FLG	400
#define	STEP		600
#define ITEM_END_FLD	9300	/* Item End Field */

#define	END_FLD		9500	/* Last Field of the screen */

/* area.sth - header for C structure generated by PROFOM EDITOR */

typedef	struct	{	/* Start Fld 1500, Endfld 4200 */

	char	s_barg[7];
	char	s_barg_desc[29];
	char	s_add[2];	/* 1600 STRING XXX */
	char	s_update[2];	/* 1600 STRING XXX */
	char	s_delete[2];	/* 1600 STRING XXX */
	char	s_browse[2];	/* 1600 STRING XXX */

}	S_item ;

typedef struct	{

	char	s_pgname[11];	/* 100 STRING X(10) */
	long	s_rundate;	/* 300 DATE YYYYFMMFDD */
	char	s_fn[2];	/* 400 STRING X */
	char	s_username[11];
	short	s_field;	/* 600 NUMERIC 99 */
	short	s_page;		/* 1300 NUMERIC 99 */

	char	s_dummy[4];

	S_item	s_items[PAGESIZE] ;	/* Start Fld 1500, End Fld 4200  */

	char	s_mesg[78];	/* 4300 STRING X(77) */
	char	s_resp[2];	/* 4400 STRING X */
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
	char	I_Status[2];	/* item status ie A(DD) C(hange) */
	short	NoItems;		/* number of Items on the page */
	short	Pageno;			/* Page number */
}	Page;

static	Page	*FirstPage,		/* Address of First Page */
		*CurPage,		/* Address of Current Page */
		*CurLast,		/* Address of Curr. record last page */
		*LastPage;		/* Address of Last Page of Memory
					   Allocated */

static	Pa_rec	pa_rec;
static	Pay_param pay_param;
static	Barg_unit	barg_unit;
static	UP_rec	userprof;
static	Userbarg userbarg, pre_userbarg;

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

	/*
	*	Get The Payroll Parameter Record
	*/
	err = get_pay_param(&pay_param, BROWSE, 1, e_mesg) ;
	if(err == ERROR) {
		DispError((char *)&s_sth,e_mesg);
		return(ERROR) ;
	}
	else if(err == UNDEF) {
#ifdef ENGLISH
		DispError((char *)&s_sth,"Payroll Parameter is Not Set Up ...");
#else
		DispError((char *)&s_sth,"Payroll Parametres ne sont pas etablis... ");
#endif
		return(ERROR) ;
	}
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
		CHKACC(retval,P_DEL,e_mesg);
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
Delete()
{
	int	err ;

	err = SelectRecord(BROWSE) ;
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

	return(NOERROR);
}	/* Next() */
/*----------------------------------------------------------------------*/
/* Get the Student Study Hall key from user. In ADD mode disable dup buffers, */
/* other modes enable dup buffers and show the current key as a default key */
/*----------------------------------------------------------------------*/
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

	s_sth.s_username[0] = LV_CHAR;

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

GetDetails()
{
	int	i, err;

	err = BuildBargList();
	if(err < 0 && err != EFL) {
		return(err);
	}

	err = ShowItems(CurPage);
	if(err < 0) {
		return(err);
	}
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
SelectRecord(mode)
int	mode;
{
	int	err ;

	err = ReadKey();
	if(err != NOERROR) return(err) ;

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
BuildBargList()
{
	int retval;
	int i;
	int j;
	CurLast = CurPage = NULL;
	i = 0;

	
	barg_unit.b_code[0] = '\0';
	barg_unit.b_date = 0;
	flg_reset(BARG);

	for( ; ; ) {
		retval = get_n_barg(&barg_unit,BROWSE,0,FORWARD,e_mesg);
		if(retval < 0) {
			if(retval == EFL) break;
			DispError((char *)&s_sth,e_mesg);
			return(retval);
		}

		if(PAGESIZE == i) i = 0;
		if(i == 0) {
			if((retval = MakeFreshPage()) < 0) return(retval);
		}

		strcpy(CurPage->Items[i].s_barg, barg_unit.b_code);
		strcpy(CurPage->Items[i].s_barg_desc, barg_unit.b_name);
		strcpy(userprof.u_id, s_sth.s_username);

		retval = get_userprof(&userprof,BROWSE,0,e_mesg) ;
		if(retval < 0) {
			DispError((char *)&s_sth,e_mesg);
			roll_back(e_mesg);
			return(retval);
		}
		if(strcmp(userprof.u_class, "A") == 0 || 
		   strcmp(userprof.u_class, "S") == 0){
		  CurPage->Items[i].s_add[0] = 'Y';
		  CurPage->Items[i].s_update[0] = 'Y';
		  CurPage->Items[i].s_delete[0] = 'Y';
		  CurPage->Items[i].s_browse[0] = 'Y';
		}
		else{
		  CurPage->Items[i].s_add[0] = 'N';
		  CurPage->Items[i].s_update[0] = 'N';
		  CurPage->Items[i].s_delete[0] = 'N';
		  CurPage->Items[i].s_browse[0] = 'N';
		}
		CurPage->I_Status[0] = ' ';
		CurPage->NoItems++;
		i++;
	}
	seq_over(BARG);

	if(CurLast != NULL) {
		CurPage = FirstPage;
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

	strcpy(userbarg.ub_id,s_sth.s_username);
	userbarg.ub_barg[0] = '\0';
	flg_reset(USERBARG);

	for( ; ; ) {
		retval = get_n_userbarg(&userbarg,BROWSE,0,FORWARD,e_mesg);
		if(retval < 0) {
			if(retval == EFL) break;
			DispError((char *)&s_sth,e_mesg);
			return(retval);
		}
		if(strcmp(userbarg.ub_id,s_sth.s_username) != 0)
			break;

		if(PAGESIZE == i) i = 0;
		if(i == 0) {
			if((retval = MakeFreshPage()) < 0) return(retval);
		}

		strcpy(CurPage->Items[i].s_barg, userbarg.ub_barg);
		strcpy(barg_unit.b_code, userbarg.ub_barg);
		barg_unit.b_date = get_date();
		flg_reset(BARG);

		retval = get_n_barg(&barg_unit,BROWSE,0,BACKWARD,e_mesg);
		if(retval < 0) {
			if(retval == EFL) break;
			DispError((char *)&s_sth,e_mesg);
			return(retval);
		}
		strcpy(CurPage->Items[i].s_barg_desc, barg_unit.b_name);

		strcpy(CurPage->Items[i].s_add, userbarg.ub_add);
		strcpy(CurPage->Items[i].s_update, userbarg.ub_update);
		strcpy(CurPage->Items[i].s_delete, userbarg.ub_delete);
		strcpy(CurPage->Items[i].s_browse, userbarg.ub_browse);
		CurPage->I_Status[0] = ' ';
		CurPage->NoItems++;
		i++;
	}
	seq_over(USERBARG);

	if(CurLast != NULL) {
		CurPage = FirstPage;
	}
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
		strcpy(s_sth.s_dummy," ");
		i = pageptr->NoItems ;
	}
	else {
		s_sth.s_page = HV_SHORT ;
		s_sth.s_dummy[0] = HV_CHAR;
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
		 retval = WriteRecords(temppage,i,mode);
  		 if(retval < 0) {
			if(retval == LOCKED) return(LOCKED);
		   	break;
		 }
	      }
	      if(temppage == CurLast) break;
	   }
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

	scpy((char *)&pre_userbarg,(char *)&userbarg,sizeof(userbarg));

	strcpy(userbarg.ub_id, s_sth.s_username);
	strcpy(userbarg.ub_barg, temppage->Items[item_no].s_barg);
	if(mode != ADD) {
		retval = get_userbarg(&userbarg,UPDATE,0,e_mesg) ;
		if(retval < 0) {
			DispError((char *)&s_sth,e_mesg);
			roll_back(e_mesg);
			return(retval);
		}
	}
	strcpy(userbarg.ub_add,temppage->Items[item_no].s_add);
	strcpy(userbarg.ub_update,temppage->Items[item_no].s_update);
	strcpy(userbarg.ub_delete,temppage->Items[item_no].s_delete);
	strcpy(userbarg.ub_browse,temppage->Items[item_no].s_browse);

	retval = put_userbarg(&userbarg,mode,e_mesg);
	if(retval < 0) {
		DispError((char *)&s_sth,e_mesg);
		roll_back(e_mesg);
		return(retval);
	}
	retval = commit(e_mesg) ;
	if(retval < 0) {
		DispError((char *)&s_sth,"ERROR: Saving Records"); 
		DispError((char *)&s_sth,e_mesg);
		roll_back(e_mesg);
		return(retval);
	}

	if(mode != ADD) {
		retval = rite_audit((char*)&s_sth,USERBARG,mode,(char*)&userbarg,
			(char*)&pre_userbarg,e_mesg);
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

	SetDupBuffers(ITEM_ST_FLD, END_FLD - 200,1);

#ifdef ENGLISH
	strcpy(s_sth.s_mesg,"Press ESC-F to Terminate");
#else
	strcpy(s_sth.s_mesg,"Appuyer sur ESC-F pour terminer");
#endif
	DispMesgFld((char *)&s_sth);

	st_fld  = ITEM_ST_FLD ;
	end_fld  = ITEM_END_FLD ;
	s_sth.s_items[item_no].s_add[0] = LV_CHAR;
	s_sth.s_items[item_no].s_update[0] = LV_CHAR ;
	s_sth.s_items[item_no].s_delete[0] = LV_CHAR ;
	s_sth.s_items[item_no].s_browse[0] = LV_CHAR ;

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
	int	i;
	int	item_no, fld_no, st_fld;

	if(sr.curfld >= ITEM_ST_FLD) {	
		item_no = (sr.curfld - ITEM_ST_FLD) / STEP;
		fld_no = (sr.curfld - ITEM_ST_FLD) % STEP - 100;

		st_fld = ITEM_ST_FLD + (STEP * item_no);
	}
	else 
		fld_no = sr.curfld;

	switch(fld_no){
	case KEY_START:
		strcpy(userprof.u_id, s_sth.s_username);

		retval = get_userprof(&userprof,BROWSE,0,e_mesg) ;
		if(retval < 0) {
			DispError((char *)&s_sth,e_mesg);
			s_sth.s_username[0] = LV_CHAR;
			return(retval);
		}
		break;
	case ADD_FLG:
		if(s_sth.s_items[item_no].s_add[0] != YES &&
		   s_sth.s_items[item_no].s_add[0] != NO) {
			fomen("Must be Y(es) or N(o)");
			s_sth.s_items[item_no].s_add[0] = LV_CHAR;
			return(ERROR);
		}
		break;
	case UPDATE_FLG:
		if(s_sth.s_items[item_no].s_update[0] != YES &&
		   s_sth.s_items[item_no].s_update[0] != NO) {
			fomen("Must be Y(es) or N(o)");
			s_sth.s_items[item_no].s_update[0] = LV_CHAR;
			return(ERROR);
		}
		break;
	case DELETE_FLG:
		if(s_sth.s_items[item_no].s_delete[0] != YES &&
		   s_sth.s_items[item_no].s_delete[0] != NO) {
			fomen("Must be Y(es) or N(o)");
			s_sth.s_items[item_no].s_delete[0] = LV_CHAR;
			return(ERROR);
		}
		break;
	case BROWSE_FLG:
		if(s_sth.s_items[item_no].s_browse[0] != YES &&
		   s_sth.s_items[item_no].s_browse[0] != NO) {
			fomen("Must be Y(es) or N(o)");
			s_sth.s_items[item_no].s_browse[0] = LV_CHAR;
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

WindowHelp()
{
	int	retval ;
	int	item_no;
	short	reccod;
	int	i;

	switch(sr.curfld){
	case KEY_START:
		break;
	default :
		fomer("No Help Window for This Field");
	}	/* Switch sr.curfld */

	return(NOERROR) ;
}	/* KeyWindowHelp() */
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
		"Y(es), S(creen Edit), L(ine Edit), N(ext), P(rev), C(ancel)"
		,"YHSLNPC");
#else
			err = GetOption((char *)&s_sth,
		"O(ui), S(creen), L(ine), S(uiv), P(rec), A(nnul)"
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
	    case  SCREENEDIT  :
		err = ScreenEdit();
		break ;
	    case  LINEEDIT  :
		err = ChangeFields();
		break ;
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
/* Change Line items.  Allows editing of the students on the screen    */
/*---------------------------------------------------------------------*/
ScreenEdit()
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

	if(SetDupBuffers(ITEM_ST_FLD, END_FLD - 200,1)<0) return(PROFOM_ERR);

	for(i=0;i<CurPage->NoItems;i++) {
		retval = ReadItem(i,UPDATE);
		if(retval != NOERROR && retval != RET_USER_ESC) {
			return(retval);
		}

		if(retval == RET_USER_ESC) 
			return(retval);

		scpy((char*)&(CurPage->Items[i]), 
		     (char*)&(s_sth.s_items[i]),sizeof(S_item)) ;
		if(s_sth.s_fn[0] == CHANGE) {
			CurPage->I_Status[0] = CHANGE;
		}
	}

     	s_sth.s_mesg[0] = HV_CHAR ;
     	DispMesgFld((char *)&s_sth);

	if(SetDupBuffers(ITEM_ST_FLD, END_FLD-200,0)<0) return(PROFOM_ERR);

	return(NOERROR);
}	/* ChangeFields  */
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

	}

     	s_sth.s_field = HV_SHORT ;
	if ( WriteFields((char *)&s_sth,CHG_FLD, CHG_FLD) < 0 ) return(-1);

     	s_sth.s_mesg[0] = HV_CHAR ;
     	DispMesgFld((char *)&s_sth);

	if(SetDupBuffers(ITEM_ST_FLD, END_FLD - 200, 0)<0) return(PROFOM_ERR);

	return(NOERROR);
}	/* ChangeFields() */
/*------------------------------------------------------------------------*/
/* Move High values to all data fields and clear the screen */

ClearScreen()
{
	int	item_no;

	/* Move High Values to Hedaer part */
	s_sth.s_page = HV_SHORT;
	s_sth.s_dummy[0] = HV_CHAR;
	
	/* Move High Values to The one item */
	for(item_no = 0; item_no <= PAGESIZE; item_no++)
		InitItem(HV_CHAR, item_no);

	ret( WriteFields((char *)&s_sth,ITEM_ST_FLD, (END_FLD - 200)) );

	return(NOERROR);
}	/* ClearScreen() */
/*-------------------------------------------------------------------------*/
/* Initialize Given screen item with either Low values or High values */

InitItem(t_char, item_no)
char	t_char ;
int	item_no;
{
	s_sth.s_items[item_no].s_barg[0] = t_char ;
	s_sth.s_items[item_no].s_barg_desc[0] = t_char ;
	s_sth.s_items[item_no].s_add[0] = t_char ;
	s_sth.s_items[item_no].s_update[0] = t_char ;
	s_sth.s_items[item_no].s_delete[0] = t_char ;
	s_sth.s_items[item_no].s_browse[0] = t_char ;

	return(NOERROR) ;
}	/* Inititem() */
/*-------------------- E n d   O f   P r o g r a m ---------------------*/
