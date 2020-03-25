/*-----------------------------------------------------------------------
Source Name: pbmaint.c
System     : Tendering System.
Created  On: 12th Apr. 92.
Created  By: J. Prescott.

DESCRIPTION:
	Maintains Potential Bidders for specified Categorys.

MODIFICATIONS:        

Programmer	YY/MM/DD	Description of modification
S.Whittaker	93/05/28	Updated the rite_audit function to include
				passing the screen structure.
------------------------------------------------------------------------*/
#define	MAIN		/* Main program. This is to declare Switches */
#define MAINFL		POTBIDDER	/* main file used */

#define	SYSTEM		"TENDERING"	/* Sub System Name */
#define	MOD_DATE	"12-APR-92"		/* Progran Last Modified */

#include <stdio.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

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
#define DELITEM		'D'
#define REACTITEM	'R'
#define	CANCEL		'C'

#define ACTIVE		"ACT"
#define DELETED		"DEL"
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
#define ADDITEMS	'R'
#define DELITEM		'E'
#define REACTITEM	'V'
#define	CANCEL		'A'

#define ACTIVE		"ACT"
#define DELETED		"ELI"
#endif

/* PROFOM Releted declarations */

#define	SCR_NAME	"pbmaint"	/* PROFOM screen Name */

#define	PAGESIZE	15		/* No of Items */
/* PROFOM Screen STH file */

/* Field PROFOM numbers */

#define FN_FLD		400	/* Fn: */

#define KEY_START	500	/* Key Start Field */
#define KEY_END		500	/* Key End Field */
#define CHG_FLD		600

#define PAGE_FLD	800	/* Page# Field */
#define	ITEM_ST_FLD	1000	/* Item 1 Start Field */
#define	END_FLD		5600	/* Last Field of the screen */
#define	STEP		300	/* NO of fields diff. between 2 items */

#define SUPP_FLD	0	/* Offset of Area Field in item line */
#define SNAME_FLD	100	/* Offset of Desc Field in item line */
#define STATUS_FLD	200	/* Offset of Status in item line */

#define ITEM_SUPP	1000	/* First Area code field in items */
#define ITEM_SNAME	1100	/* First Description field in items */

/* area.sth - header for C structure generated by PROFOM EDITOR */

typedef	struct	{	/* Start Fld 900, Endfld 5300 and Step 300 */

	char	s_supp_cd[11];	/* 900 STRING X(10) */
	char	s_sname[29];/* 1000 STRING X(25) */
	char	s_status[4]; 	/* 1100 STRING X(3) */
}	S_item ;

typedef struct	{

	char	s_pgname[11];	/* 100 STRING X(10) */
	long	s_rundate;	/* 300 DATE YYYYFMMFDD */
	char	s_fn[2];	/* 400 STRING X */
	short	s_categ_num;	/* 500 NUMERIC 99 */
	short	s_field;	/* 600 NUMERIC 99 */

	short	s_page;		/* 800 NUMERIC 99 */
	char	s_dummy[4];	/* dummy field */

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

static	Pa_rec	pa_rec;		 	/* Parameter record */
static	PotBidder pb_rec, pre_potbid;	/* Potential Bidder record */
static	Category  category;		/* Category record */
static	Supplier supplier;		/* Supplier record */ 

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
	if(pa_rec.pa_tendering[0] != YES) {
#ifdef ENGLISH
	     DispError((char *)&s_sth,"Tendering System Absent.  See Parameter Maintenance.");
#else
	     DispError((char *)&s_sth,"Systeme Soumission absent. Voir l'entretien des parametres.");
#endif
	     return(ERROR);
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
	fomer("R(ajouter), C(hanger), E(liminer), I(nterroger), S(uivant), P(recedent), F(in)");
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
/* Change. Potential Bidders 						 */
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
/* Delete. Potential Bidders.  						 */
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
/* Show Potential Bidders */
Inquire()
{
	int	err ;

	err = SelectRecord() ;
	if(NOERROR != err) return(err) ;

	err = ConfirmItems() ;
	
	return(NOERROR) ;
}	/* Inquire() */
/*----------------------------------------------------------*/
/* Show the next or previous Potential Bidders */

Next(direction)
int	direction ;
{
	int err;

	/* Check whether file is in seq. read mode */
	/* if(flg_start(POTBIDDER) != direction) { */
		pb_rec.pb_categ_num = s_sth.s_categ_num ;
		if (direction == FORWARD) {
			pb_rec.pb_categ_num++;
			pb_rec.pb_supp[0] = '\0' ;
		}
		else {
			pb_rec.pb_categ_num--;
			strcpy(pb_rec.pb_supp,"ZZZZZZZZZZ") ;
		}
		flg_reset(POTBIDDER);
	/* } */

	err = get_n_potbidder(&pb_rec,BROWSE,0,direction,e_mesg);
	if(err < 0 && err != EFL) {
		DispError((char *)&s_sth,e_mesg);
		return(err);
	}

	if(err == EFL) {
		fomer("No More Records...");
		get();
		err = NOERROR;	
	}
	else {
		s_sth.s_categ_num = pb_rec.pb_categ_num;
		ret(WriteFields((char *)&s_sth,KEY_START,KEY_END));

		/* Get area records i.e. Build list */
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
/* Get the Category key from user. In ADD mode disable dup buffers,     */
/* other modes enable dup buffers and show the current key as a default key */
/*----------------------------------------------------------------------*/
ReadKey()
{
	int	i;
	short	temp_categ_num;
	
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

	temp_categ_num = s_sth.s_categ_num;

	i = ReadFields((char *)&s_sth,KEY_START, KEY_END,
		Validation, WindowHelp,1) ;
	if(PROFOM_ERR == i || DBH_ERR == i) return(i) ;
	if(RET_USER_ESC == i){
		s_sth.s_categ_num = temp_categ_num;
		
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
			s_sth.s_dummy[0] = ' ';
			ret(WriteFields((char *)&s_sth,PAGE_FLD,PAGE_FLD+100));
			i = 0 ;
		}
	}


	for( ; ; ) {
		if( PAGESIZE == i) {	/* Page Full */

			/* Move High Values to All items except first */
			for(i-- ; i > 0 ; i--)
				InitItem(i, HV_CHAR) ;

			/* Calculate the page# */
			s_sth.s_page = CurLast->Pageno + 1 ;
			s_sth.s_dummy[0] = ' ';

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

	pb_rec.pb_categ_num = s_sth.s_categ_num;
	pb_rec.pb_supp[0] = '\0';
	
	flg_reset(POTBIDDER);

	for( ; ; ) {
		retval = get_n_potbidder(&pb_rec,BROWSE,0,FORWARD,e_mesg);
		if(retval < 0) {
			if(retval == EFL) break;
			DispError((char *)&s_sth,e_mesg);
			return(retval);
		}

		if(pb_rec.pb_categ_num != s_sth.s_categ_num) { 
			break;
		}

		if(PAGESIZE == i) i = 0;
		if(i == 0) {
			if((retval = MakeFreshPage()) < 0) return(retval);
		}

		strcpy(CurPage->Items[i].s_supp_cd,pb_rec.pb_supp);

		strcpy(supplier.s_supp_cd,pb_rec.pb_supp);
		retval = get_supplier(&supplier,BROWSE,0,e_mesg);	
		if(retval < 0) {
			DispError((char *)&s_sth,e_mesg);
			return(retval);
		}

		strncpy(CurPage->Items[i].s_sname,supplier.s_name,28);
		strcpy(CurPage->Items[i].s_status,ACTIVE);
		CurPage->I_Status[i][0] = ' ';
		CurPage->NoItems++;
		i++;
	}
	seq_over(POTBIDDER);


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
		s_sth.s_dummy[0] = ' ';

		i = pageptr->NoItems ;
	}
	else {
		s_sth.s_page = HV_SHORT ;
		s_sth.s_dummy[0] = HV_CHAR;
		i = 0 ;
	}

	/* Move High Values to remaining Items */
	for( ; i < PAGESIZE ; i++ )
		InitItem(i, HV_CHAR);

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
		if(strcmp(temppage->Items[item_no].s_status,DELETED)==0) {
			*write_mode = NOOP;
		}
		else {
			*write_mode = ADD;
		}
	}
	else if(mode == UPDATE) {
		if(strcmp(temppage->Items[item_no].s_status,DELETED)==0 &&
		   temppage->I_Status[item_no][0] != ADDITEMS) {
			*write_mode = P_DEL;
		}
		else if(strcmp(temppage->Items[item_no].s_status,DELETED)==0 &&
			temppage->I_Status[item_no][0] == ADDITEMS) {
			*write_mode = NOOP;
		}
		else if(temppage->I_Status[item_no][0] == ADDITEMS) {
			*write_mode = ADD;
		}
		else {
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
	PotBidder	pb_tmp;

	if(mode != ADD) {
		scpy((char *)&pre_potbid,(char *)&pb_rec,
			sizeof(pre_potbid));

		pb_rec.pb_categ_num = s_sth.s_categ_num ;
		strcpy(pb_rec.pb_supp,temppage->Items[item_no].s_supp_cd);
		retval = get_potbidder(&pb_rec,UPDATE,0,e_mesg) ;
		if(retval < 0) {
			DispError((char *)&s_sth,e_mesg);
			roll_back(e_mesg);
			return(retval);
		}
	}

	/* Add potential bidder to file with the same chance of being */
	/* Selected as the supplier(s) selected the least amount of times */
	pb_tmp.pb_categ_num = s_sth.s_categ_num;
	pb_tmp.pb_select = 0;
	pb_tmp.pb_supp[0] = '\0';
	flg_reset(POTBIDDER);
	retval = get_n_potbidder(&pb_tmp,BROWSE,1,FORWARD,e_mesg);
	if(retval<0 && retval != EFL) {
		DispError((char *)&s_sth,e_mesg);
		roll_back(e_mesg);
		return(retval);
	}
	else if(retval == EFL || pb_tmp.pb_categ_num != s_sth.s_categ_num) {
		pb_rec.pb_select = 0;
	}
	else {
		pb_rec.pb_select = pb_tmp.pb_select;
	}

	pb_rec.pb_categ_num = s_sth.s_categ_num;
	strcpy(pb_rec.pb_supp,temppage->Items[item_no].s_supp_cd);
	retval = put_potbidder(&pb_rec,mode,e_mesg) ;
	if(retval < 0) {
		DispError((char *)&s_sth,e_mesg);
		roll_back(e_mesg);
		return(retval);
	}

	if(mode != ADD) {
		retval = rite_audit((char*)&s_sth,POTBIDDER,mode,(char*)&pb_rec,
			(char*)&pre_potbid,e_mesg);
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

#ifdef ENGLISH
	strcpy(s_sth.s_mesg,"Press ESC-F to Terminate");
#else
	strcpy(s_sth.s_mesg,"Appuyer sur ESC-F pour terminer");
#endif
	DispMesgFld((char *)&s_sth);

	if(mode == ADD) {
		st_fld  = ITEM_ST_FLD + (STEP * item_no) + SUPP_FLD;
		end_fld  = ITEM_ST_FLD + (STEP * item_no) + STATUS_FLD;
		s_sth.s_items[item_no].s_supp_cd[0] = LV_CHAR;
		s_sth.s_items[item_no].s_sname[0] = LV_CHAR;
		strcpy(s_sth.s_items[item_no].s_status,ACTIVE);
	}
	i = ReadFields((char *)&s_sth,st_fld,end_fld,Validation,WindowHelp,1) ;
	if(PROFOM_ERR == i || DBH_ERR == i) return(i) ;
	if(RET_USER_ESC == i) {
		if(mode == ADD) {
			InitItem(item_no,HV_CHAR);
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
		item_no = (sr.curfld - ITEM_ST_FLD - SUPP_FLD) / STEP;
		fld_no = sr.curfld - (item_no * STEP);
	}

	switch(fld_no){
	case KEY_START:
		if(s_sth.s_fn[0] == ADDREC) {
			category.categ_num = s_sth.s_categ_num;
			retval = get_category(&category,BROWSE,0,e_mesg);
			if(retval < 0 && retval != UNDEF){
				fomer(e_mesg);
				s_sth.s_categ_num = LV_SHORT;
				return(ERROR);
			}
			if(retval == UNDEF) {
				fomer("Category Number Does Not Exist - Please Re-Enter");
				s_sth.s_categ_num = LV_SHORT;
				return(ERROR);
			}
		}
		else {
			pb_rec.pb_categ_num = s_sth.s_categ_num;
			pb_rec.pb_supp[0] = '\0';
			flg_reset(POTBIDDER);
			retval = get_n_potbidder(&pb_rec,BROWSE,0,
				FORWARD,e_mesg);
			if(retval < 0 && retval != EFL) {
				fomer(e_mesg);
				s_sth.s_categ_num = LV_SHORT;
				return(ERROR);
			}
			if(retval == EFL ||
			   pb_rec.pb_categ_num != s_sth.s_categ_num) {
				fomer("No Potential Suppliers Avalaible - Please Re-Enter");
				s_sth.s_categ_num = LV_SHORT;
				return(ERROR);
			}
		}
		break;
	case ITEM_SUPP:
  		Right_Justify_Numeric(s_sth.s_items[item_no].s_supp_cd,
			sizeof(s_sth.s_items[item_no].s_supp_cd)-1);
		if(CheckSuppCode(s_sth.s_items[item_no].s_supp_cd)<0) {
			s_sth.s_items[item_no].s_supp_cd[0] = LV_CHAR;
			return(ERROR);
		}
		strncpy(s_sth.s_items[item_no].s_sname,supplier.s_name,28);
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
CheckSuppCode(supp_cd)
char	*supp_cd;
{
	int	retval;
	int	i;
	Page	*temppage;

	if(supp_cd[0] == '\0') {
		fomer("This is a Required Field");
		return(ERROR);
	}

	strcpy(supplier.s_supp_cd,supp_cd);
	retval = get_supplier(&supplier,BROWSE,0,e_mesg);
	if(retval < 0) {
		DispError((char *)&s_sth,e_mesg);
		return(ERROR);
	}

	pb_rec.pb_categ_num = s_sth.s_categ_num;
	strcpy(pb_rec.pb_supp,supp_cd);
	retval = get_potbidder(&pb_rec,BROWSE,0,e_mesg);
	if(retval < 0) {
		if(retval != UNDEF) {
			DispError((char *)&s_sth,e_mesg);
			return(ERROR);
		}

	}
	else {
		fomer("Supplier is Already a Potential Bidder For This Category");
		return(ERROR);
	}

	/* check to see if item is already in list */
	if(CurLast != NULL) {
	   for(temppage=FirstPage; temppage!=NULL;temppage=temppage->NextPage) {
	      for(i =0; i< temppage->NoItems; i++) {
			if(temppage->Items[i].s_supp_cd[0] == NULL) {
				return(NOERROR);
			}
			if(strcmp(temppage->Items[i].s_supp_cd,supp_cd) == 0) {
				fomer("Supplier Already Entered on Screen");
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

	if(sr.curfld == KEY_START) {
		fld_no = KEY_START;
	}
	else {
		item_no = (sr.curfld - ITEM_ST_FLD - SUPP_FLD) / STEP;
		fld_no = sr.curfld - (item_no * STEP);
	}

	switch(fld_no){
	case KEY_START:
		retval = category_hlp(&s_sth.s_categ_num,7, 13);
		if(retval == DBH_ERR) return(retval);
		if(retval >= 0) redraw();
		if(retval == 0) return(ERROR);
		if(retval < 0) return(ERROR);
		if(s_sth.s_fn[0] != ADDREC) {
			pb_rec.pb_categ_num = s_sth.s_categ_num;
			pb_rec.pb_supp[0] = '\0';
			flg_reset(POTBIDDER);
			retval = get_n_potbidder(&pb_rec,BROWSE,0,
				FORWARD,e_mesg);
			if(retval < 0 && retval != EFL) {
				fomer(e_mesg);
				s_sth.s_categ_num = LV_SHORT;
				return(ERROR);
			}
			if(retval == EFL ||
			   pb_rec.pb_categ_num != s_sth.s_categ_num) {
				fomer("No Potential Suppliers Avalaible - Please Re-Enter");
				s_sth.s_categ_num = LV_SHORT;
				return(ERROR);
			}
		}
/*
		WriteFields((char *)&s_sth,KEY_START, KEY_END);
*/
		break;
	case ITEM_SUPP:
		retval = supp_hlp(s_sth.s_items[item_no].s_supp_cd,7,13);
		if(retval == DBH_ERR) return(retval);
		if(retval >= 0) redraw();
		if(retval == 0) return(ERROR);
		if(retval < 0) return(ERROR);
		if(CheckSuppCode(s_sth.s_items[item_no].s_supp_cd)<0) {
			s_sth.s_items[item_no].s_supp_cd[0] = LV_CHAR;
			return(ERROR);
		}
		strncpy(s_sth.s_items[item_no].s_sname,supplier.s_name,28);
		break;
	default :
		fomer("No Help Window for This Field");
	}	/* Switch sr.curfld */

	return(NOERROR) ;
}	/* WindowHelp() */
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
			err = GetOption((char *)&s_sth,"O(ui), A(nnuler)","OA");
#endif 
	    }
	    else if(s_sth.s_fn[0] != INQUIRE) {
#ifdef ENGLISH
			err = GetOption((char *)&s_sth,
		"Y(es), A(dd), D(el), R(eactivate), N(ext), P(rev), C(ancel)"
		,"YADRNPC");
#else
			err = GetOption((char *)&s_sth,
		"O(ui), R(aj), E(lim), V(ivif), S(uiv), P(rec), A(nnul)"
		,"OSLA");
#endif
	    }
	    else {
#ifdef ENGLISH
			err = GetOption((char *)&s_sth,
		"Y(es), N(ext), P(rev)","YNP");
#else
			err = GetOption((char *)&s_sth,
		"O(ui), S(uivant), P(recedent)" ,"OSP");
#endif
	    }
	    switch(err) {
	    case  YES  :
		return(YES);
	    case  ADDITEMS:
		err = AddItems();
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
			   DELETED)==0) {
				fomer("Item is Already Deleted");
			}
			else {
				strcpy(s_sth.s_items[s_sth.s_field-1].s_status,
			   		DELETED);
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
	s_sth.s_dummy[0] = HV_CHAR;
	
	/* Move High Values to All items */
	for(i = 0 ; i < PAGESIZE ; i++)
		InitItem(i,HV_CHAR);

	ret( WriteFields((char *)&s_sth,ITEM_ST_FLD, (END_FLD - 200)) );

	return(NOERROR);
}	/* ClearScreen() */
/*-------------------------------------------------------------------------*/
/* Initialize Given screen item with either Low values or High values */

InitItem(item_no, t_char)
int	item_no ;
char	t_char ;
{
	int	i;

	s_sth.s_items[item_no].s_supp_cd[0] = t_char ;
	if(t_char == HV_CHAR) {
		s_sth.s_items[item_no].s_sname[0] = t_char ;
	}
	s_sth.s_items[item_no].s_status[0] = t_char ;

	return(NOERROR) ;
}	/* Inititem() */
/*-------------------- E n d   O f   P r o g r a m ---------------------*/
