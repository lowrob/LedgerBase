/*-----------------------------------------------------------------------
Source Name: procgl.c
System     : Personnel/Payroll System.

DESCRIPTION:

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
------------------------------------------------------------------------*/
#define	MAIN		/* Main program. This is to declare Switches */
#define MAINFL		GLACCT		/*main file used */

#define	SYSTEM		"SETUP"			/* Sub System Name */
#define	MOD_DATE	"10-DEC-91"		/* Progran Last Modified */

#include <stdio.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_pp.h>
#include <bfs_com.h>

/* User Interface define constants */
#ifdef ENGLISH
#define ADDREC		'A'
#define CHANGE		'C'
#define INQUIRE		'I'
#define EXITOPT		'E'

#define	YES		'Y'
#define NO		'N'
#define ADDITEMS	'A'
#define	SCREENEDIT	'S'
#define	LINEEDIT	'L'
#define HEADEREDIT	'H'
#define DELITEM		'D'
#define REACTITEM	'R'
#define	CANCEL		'C'
#define	NEXT		'N'
#define	PREV		'P'

#define ACTIVE		"ACT"
#define DELETED		"DEL"

#define FT		"FT"
#define PT		"PT"
#define SU		"SU"
#define CA		"CA"

#else
#define ADDREC		'R'
#define CHANGE		'C'
#define DELETE		'E'
#define INQUIRE		'I'
#define EXITOPT		'F'
#define	NEXT		'N'
#define	PREV		'P'

#define	YES		'O'
#define NO		'N'
#define ADDITEMS	'A'
#define	SCREENEDIT	'S'
#define	LINEEDIT	'L'
#define DELITEM		'D'
#define REACTITEM	'V'
#define	CANCEL		'A'

#define ACTIVE		"ACT"
#define DELETED		"ELI"

#define FT		"PT"
#define PT		"TP"
#define SU		"SU"
#define CA		"TE"
#endif

#define	REACT		99

/* PROFOM Releted declarations */

#define	SCR_NAME	"procgl"	/* PROFOM screen Name */

#define	PAGESIZE	15		/* No of Items */
/* PROFOM Screen STH file */

/* Field PROFOM numbers */

#define FN_FLD		400	/* Fn: */
#define PAGE_FLD	500	/* Page# Field */
#define CHG_FLD		600
#define COLHDR1_FLD	900	/* Desc. Field */

#define	FUND_FLD	100	/* CSB/Loan Code */
#define TYPE_FLD	200	/* Description field */
#define EARN_FLD	300	/* Description field */
#define CLASS_FLD	400	/* Description field */
#define ACCT_FLD	500	/* Description field */
#define STATUS_FLD	600	/* Status Field */

#define	ITEM_ST_FLD	900	/* Item Start Field */
#define ITEM_END_FLD	9800	/* Item End Field */
#define	STEP		600	/* # of Fields on one line */

#define	END_FLD		100000	/* Last Field of the screen */

/* area.sth - header for C structure generated by PROFOM EDITOR */

typedef	struct	{	/* Start Fld 1500, Endfld 4200 */

	short	s_fund;
	char	s_type[2];	/* 800 STRING X(6) */
	char	s_earn[7];	/* 900 STRING X(30) */
	char	s_class[7];
	char	s_acct[16];
	char	s_status[6];	/* 1300 STRING XXX */

}	S_item ;

typedef struct	{


	char	s_pgname[11];	/* 100 STRING X(10) */
	long	s_rundate;	/* 300 DATE YYYYFMMFDD */
	char	s_fn[2];	/* 400 STRING X */
	short	s_page;		/* 700 NUMERIC 99 */
	short	s_field;	/* 500 NUMERIC 99 */
	short	s_dummy;

	S_item	s_items[PAGESIZE] ;	/* Start Fld 800, End Fld 3100  */

	char	s_mesg[78];	/* 3200 STRING X(77) */
	char	s_resp[2];	/* 3300 STRING X */
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

static	Gl_acct	gl_acct,	pre_gl_acct;
static	Gl_rec	gl_rec;
static	Sch_rec	school;
static	Class	class;
static	Earn	earn;
static	Ctl_rec	ctl_rec;

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
	s_sth.s_page = HV_SHORT;
	s_sth.s_field = HV_SHORT ;
	s_sth.s_mesg[0] = HV_CHAR ;
	s_sth.s_resp[0] = HV_CHAR ;
	s_sth.s_dummy = HV_SHORT ;

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
	fomer("A(dd), C(hange), I(nquire), E(xit)");
#else
	fomer("R(ajouter), C(hanger), I(nterroger), F(in)");
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
	case INQUIRE  :			/* Inquire */
		CHKACC(retval,BROWSE,e_mesg);
		return( Inquire() ) ;
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

	err = ShowScreen() ;
	if(NOERROR != err) return(err);

	s_sth.s_page = 1;

	ret(WriteFields((char*)&s_sth,COLHDR1_FLD,COLHDR1_FLD));

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
Inquire()
{
	int	err ;

	s_sth.s_page = 1;

	err = ShowScreen() ;
	if(NOERROR != err) return(err) ;

	err = ConfirmItems() ;
	
	return(NOERROR) ;
}	/* Inquire() */
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
		SetDupBuffers(FUND_FLD,ACCT_FLD,0); /* Off Dup Control */
	}
	else {
		SetDupBuffers(FUND_FLD,ACCT_FLD,1);
	}

#ifdef ENGLISH
	strcpy(s_sth.s_mesg,"Press ESC-F to Go Back to Fn:");
#else
	strcpy(s_sth.s_mesg,"Appuyer sur ESC-F pour retourner a Fn:");
#endif
	DispMesgFld((char *)&s_sth);

	s_sth.s_items[0].s_fund = LV_SHORT;
	s_sth.s_items[0].s_type[0] = LV_CHAR;
	s_sth.s_items[0].s_earn[0] = LV_CHAR;
	s_sth.s_items[0].s_class[0] = LV_CHAR;

	i = ReadFields((char *)&s_sth,FUND_FLD, ACCT_FLD,
		Validation, WindowHelp,1) ;
	if(PROFOM_ERR == i || DBH_ERR == i) return(i) ;
	if(RET_USER_ESC == i){
		
		ret( WriteFields((char *)&s_sth,FUND_FLD, ACCT_FLD) ) ;

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
			WriteFields((char*)&s_sth,COLHDR1_FLD,COLHDR1_FLD);
			i = 0 ;
		}
	}

	for( ; ; ) {
		if( PAGESIZE == i) {	/* Page Full */

			/* move High Values to All items exept First one */
			for(i--;i > 0; i--)
				InitItem(HV_CHAR,HV_SHORT,i);

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
SelectRecord(mode)
int	mode;
{
	int	err ;

	err = ReadKey();
	if(err != NOERROR) return(err) ;

	gl_acct.gl_fund = s_sth.s_items[0].s_fund;
	strcpy(gl_acct.gl_type,s_sth.s_items[0].s_type);
	strcpy(gl_acct.gl_earn,s_sth.s_items[0].s_earn);
	strcpy(gl_acct.gl_class,s_sth.s_items[0].s_class);

	err = get_glacct(&gl_acct,mode,0,e_mesg);
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
ShowScreen()
{
	int	err ;


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

	gl_acct.gl_fund = 0;	
	gl_acct.gl_type[0] = '\0';	
	gl_acct.gl_earn[0] = '\0';	
	gl_acct.gl_class[0] = '\0';	
	flg_reset(GLACCT);

	for( ; ; ) {
		retval = get_n_glacct(&gl_acct,BROWSE,0,FORWARD,e_mesg);
		if(retval < 0) {
			if(retval == EFL) break;
			DispError((char *)&s_sth,e_mesg);
			return(retval);
		}

		if(PAGESIZE == i) i = 0;
		if(i == 0) {
			if((retval = MakeFreshPage()) < 0) return(retval);
		}

		CurPage->Items[i].s_fund =  gl_acct.gl_fund;
		strcpy(CurPage->Items[i].s_type, gl_acct.gl_type);
		strcpy(CurPage->Items[i].s_earn,  gl_acct.gl_earn);
		strcpy(CurPage->Items[i].s_class,  gl_acct.gl_class);
		strcpy(CurPage->Items[i].s_acct,  gl_acct.gl_acct);
		strcpy(CurPage->Items[i].s_status,ACTIVE);

		CurPage->I_Status[i][0] = ' ';
		CurPage->NoItems++;
		i++;
	}
	seq_over(GLACCT);


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
		InitItem(HV_CHAR,HV_SHORT,i);

	ret( WriteFields((char *)&s_sth, FUND_FLD, (END_FLD - 200)) );

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

	scpy((char *)&pre_gl_acct,(char *)&gl_acct,sizeof(gl_acct));
	
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
		if(strcmp(temppage->Items[item_no].s_status,DELETED)==0) {
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
/* Write the Loan record to the file.			 		 */ 
/*-----------------------------------------------------------------------*/ 
WriteRecords(temppage,item_no,mode)
Page	*temppage;
int	item_no;
int	mode;
{
	int	retval;
	int	i;
	char	cc[5];

	school.sc_numb = 0;
	flg_reset(SCHOOL);

	for( ; ; ){

	  retval = get_n_sch(&school,BROWSE,0,FORWARD,e_mesg);
	  if(retval < 0) {
		if(retval == EFL) break;
		DispError((char *)&s_sth,e_mesg);
		roll_back(e_mesg);
		return(retval);
	  }

	  scpy((char *)&pre_gl_acct,(char *)&gl_acct,sizeof(gl_acct));

	  gl_acct.gl_fund = temppage->Items[item_no].s_fund;
	  gl_acct.gl_cc = school.sc_numb;
	  strcpy(gl_acct.gl_type,temppage->Items[item_no].s_type);
	  strcpy(gl_acct.gl_earn,temppage->Items[item_no].s_earn);
	  strcpy(gl_acct.gl_class,temppage->Items[item_no].s_class);
	  if(mode != ADD) {
		retval = get_glacct(&gl_acct,UPDATE,0,e_mesg) ;
		if(retval < 0) {
			DispError((char *)&s_sth,e_mesg);
			roll_back(e_mesg);
			return(retval);
		}
	  }
 	  
	  strcpy(e_mesg,temppage->Items[item_no].s_acct);
	  if( school.sc_numb < 10)
		strcat(e_mesg,"0");
	  sprintf(cc,"%d",school.sc_numb);
	  strcat(e_mesg,cc);
	  strncpy(gl_acct.gl_acct,e_mesg,18);

	  retval = put_glacct(&gl_acct,mode,e_mesg) ;
	  if(retval < 0) {
		DispError((char *)&s_sth,e_mesg);
		roll_back(e_mesg);
		return(retval);
	  }

	  if(mode != ADD) {
		retval = rite_audit((char*)&s_sth,POSITION,mode,(char*)&gl_acct,
			(char*)&pre_gl_acct,e_mesg);
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
	}
	seq_over(SCHOOL);

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
	s_sth.s_items[item_no].s_fund = LV_SHORT;
	s_sth.s_items[item_no].s_type[0] = LV_CHAR;
	s_sth.s_items[item_no].s_earn[0] = LV_CHAR;
	s_sth.s_items[item_no].s_class[0] = LV_CHAR;
	s_sth.s_items[item_no].s_acct[0] = LV_CHAR;
	if(mode == ADD) {
		InitItem(LV_CHAR,LV_SHORT,item_no);
		strcpy(s_sth.s_items[item_no].s_status,ACTIVE);
	}
	i = ReadFields((char *)&s_sth,st_fld,end_fld,Validation,WindowHelp,1) ;
	if(PROFOM_ERR == i || DBH_ERR == i) return(i) ;
	if(RET_USER_ESC == i) {
		if(mode == ADD) {
			InitItem(HV_CHAR,HV_SHORT,item_no);
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
	Page	*temppage;
	int	retval;
	int	i;
	int	item_no, fld_no;
	char	tempcode[7];

	item_no = (sr.curfld - ITEM_ST_FLD) / STEP;
	fld_no = (sr.curfld - ITEM_ST_FLD) % STEP;

	switch(fld_no){

	case FUND_FLD :			/* Position code field */
			ctl_rec.fund = s_sth.s_items[item_no].s_fund;
			retval = get_ctl(&ctl_rec,BROWSE,0,e_mesg);
			if(retval < 0) {
				fomer("Fund Number Does not Exist");
				s_sth.s_items[item_no].s_fund = LV_SHORT;
				return(ERROR);
			}
			break;
	case TYPE_FLD :			/* Position code field */
		  if((strcmp(s_sth.s_items[item_no].s_type,"S") != 0) &&
		     (strcmp(s_sth.s_items[item_no].s_type,"T") != 0) &&
		     (strcmp(s_sth.s_items[item_no].s_type,"C") != 0) &&
		     (strcmp(s_sth.s_items[item_no].s_type,"U") != 0) &&
		     (strcmp(s_sth.s_items[item_no].s_type,"R") != 0) &&
		     (strcmp(s_sth.s_items[item_no].s_type,"D") != 0)){ 	

			fomer("Invalide Entry");
			s_sth.s_items[item_no].s_type[0] = LV_CHAR;
			return(ERROR);
		  }
		  break;
	case EARN_FLD :			/* Position code field */
		  Right_Justify_Numeric(s_sth.s_items[item_no].s_earn,
				sizeof(s_sth.s_items[item_no].s_earn)-1);
		  strcpy(earn.ea_code,s_sth.s_items[item_no].s_earn);
		  earn.ea_date = s_sth.s_rundate;
		  flg_reset(EARN);

		  retval = get_n_earn(&earn,BROWSE,0,BACKWARD,e_mesg);
		  if(retval < 0){
		      if(retval == EFL ||
		      strcmp(earn.ea_code,
				s_sth.s_items[item_no].s_earn) != 0)
			 break;
			fomer("Earnings Code Does Not Exist - Please Re-enter");
			s_sth.s_items[item_no].s_earn[0] = LV_CHAR;
		    	return(ERROR);
		  }
		  break;
	case CLASS_FLD :			/* Position code field */
		  Right_Justify_Numeric(s_sth.s_items[item_no].s_class,
				sizeof(s_sth.s_items[item_no].s_class)-1);
		  strcpy(class.c_code,s_sth.s_items[item_no].s_class);
		  class.c_date = s_sth.s_rundate;
		  flg_reset(CLASSIFICATION);

		  retval = get_n_class(&class,BROWSE,0,BACKWARD,e_mesg);
		  if(retval < 0){
		     if(retval == EFL ||
		      strcmp(class.c_code,s_sth.s_items[item_no].s_class) != 0)
			 break;
			fomer("Classification Code Does Not Exist - Please Re-enter");
			s_sth.s_items[item_no].s_class[0] = LV_CHAR;
		    	return(ERROR);
		  }
		  break;
	case ACCT_FLD :			/* Position code field */
  		Right_Justify_Numeric(s_sth.s_items[item_no].s_acct,
			sizeof(s_sth.s_items[item_no].s_acct)-1);
  	  break;

	default : 
#ifdef ENGLISH
		sprintf(e_mesg,"No Validity Check For Field# %d", sr.curfld);
#else
		sprintf(e_mesg,"Pas de controle de validite pour le Champ#  %d",				sr.curfld);
#endif
		fomen(e_mesg);
		get();
		return(ERROR);
	}			/* Switch sr.curfld	*/
	sr.nextfld = sr.curfld;

	return(NOERROR) ;
}	/* Validation() */
/*----------------------------------------------------------------*/
WindowHelp()
{
	return(NOERROR);
}	/* WindowHelp() */

/*-----------------------------------------------------------------------*/
/* Take the confirmation from user for the items part of the screen      */
/*-----------------------------------------------------------------------*/
ConfirmItems()
{
	int	err ;

	/* Options:
	     YLNPC
	*/

	for( ; ; ) {
	    if(s_sth.s_fn[0] == CHANGE) {
#ifdef ENGLISH
			err = GetOption((char *)&s_sth,
		"Y(es), S(creen edit), L(ine), D(el), R(eactivate), N(ext), P(rev), C(ancel)"
		,"YSLDRNPC");
#else
			err = GetOption((char *)&s_sth,
		"O(ui), S(creen edit), L(ine edit), S(uiv), P(rec), A(nnul)"
		,"OCLEVSPA");
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
		,"ORCLEVSPA");
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
	    case SCREENEDIT:
		err = ScreenEdit();
		break;
	    case  LINEEDIT  :
		err = ChangeFields(UPDATE);
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
/* Change screen.  Allows editing of all the students on the screen    */
/*---------------------------------------------------------------------*/
ScreenEdit()
{
     	int i;
	int retval;

	/* make copy of screen incase user presses ESC-F */
	scpy((char *)&image,(char *)&s_sth,sizeof(s_sth));

	SetDupBuffers(ITEM_ST_FLD, END_FLD - 200, 1);

#ifdef ENGLISH
	strcpy(s_sth.s_mesg,"Press ESC-F to Terminate");
#else
	strcpy(s_sth.s_mesg,"Appuyer sur ESC-F pour terminer");
#endif
	DispMesgFld((char *)&s_sth);

	for(i=0;i < PAGESIZE; i++) {
		if(s_sth.s_items[i].s_fund == HV_SHORT)
			break;

		retval = ReadItem(i,UPDATE) ;	/* Read Each Item Line */
		if(PROFOM_ERR == retval || DBH_ERR == retval) return(retval) ;
		if(NOERROR != retval) break ;	/* ESC-F */

/* Andre */
		if(s_sth.s_fn[0] == CHANGE) {
			CurPage->I_Status[i][0] = CHANGE;
		}
		if(s_sth.s_fn[0] == ADDREC) {
			CurPage->I_Status[i][0] = ADDITEMS;
		}
/* Andre */
		/* Copy the Item to Page List */
		scpy((char*)&(CurPage->Items[i]),
		   (char*)&(s_sth.s_items[i]),sizeof(S_item)) ; 
	}
	ret( WriteFields((char *)&s_sth,ITEM_ST_FLD, (END_FLD - 200)) );
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

		if(strcmp(CurPage->Items[s_sth.s_field-1].s_status,DELETED)==0){
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
			CurPage->I_Status[s_sth.s_field -1][0] = CHANGE;
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

	/* Move High Values to The one item */
	for( i=0; i < PAGESIZE ; i++) {
		InitItem(HV_CHAR,HV_SHORT,i);
	}

	ret( WriteFields((char *)&s_sth,ITEM_ST_FLD, (END_FLD - 200)) );

	return(NOERROR);
}	/* ClearScreen() */
/*-------------------------------------------------------------------------*/
/* Initialize Given screen item with either Low values or High values */

InitItem(t_char, t_short, item_no)
char	t_char ;
short	t_short ;
short	item_no ;
{
	int	i;

	if(s_sth.s_fn[0] != CHANGE) {
		s_sth.s_items[item_no].s_fund = t_short ;
		s_sth.s_items[item_no].s_type[0] = t_char ;
		s_sth.s_items[item_no].s_earn[0] = t_char;
		s_sth.s_items[item_no].s_class[0] = t_char;
	}
	else {
		if(t_char != LV_CHAR) {
			s_sth.s_items[item_no].s_fund = t_short ;
			s_sth.s_items[item_no].s_type[0] = t_char ;
			s_sth.s_items[item_no].s_earn[0] = t_char;
			s_sth.s_items[item_no].s_class[0] = t_char;
		}
	}
	s_sth.s_items[item_no].s_acct[0] = t_char;
	if(t_char == HV_CHAR) {
		s_sth.s_items[item_no].s_status[0] = t_char ;
	}

	return(NOERROR) ;
}	/* Inititem() */
/*-----------------------------------------------------------------------*/
/*  */
Mesg()
{
/*		Display error message				*/
#ifdef ENGLISH
	sprintf(e_mesg,	"Position Code Already Exists, Please Re-enter");
#else
	sprintf(e_mesg, "Code de gl_acct existe deja, SVP reseille");
#endif
	DispError((char *)&s_sth,e_mesg);
	return(ERROR);
}
/*-------------------- E n d   O f   P r o g r a m ---------------------*/