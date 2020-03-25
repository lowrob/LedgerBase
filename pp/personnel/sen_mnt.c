/*-----------------------------------------------------------------------
Source Name: sen_mnt.c
System     : Personnel/Payroll System.
Created  On: Oct 13, 1992.
Created  By: Eugene Roy.

DESCRIPTION:
	The Earnings Maintenance Screen is used to verify and update
	seniority for an individual.

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
------------------------------------------------------------------------*/
#define	MAIN		/* Main program. This is to declare Switches */
#define MAINFL		EMP_SEN		/* main file used */

#define	SYSTEM		"PAYROLL"	/* Sub System Name */
#define	MOD_DATE	"13-OCT-92"		/* Progran Last Modified */

#include <stdio.h>

#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_pp.h>
#include <pp_msgs.h>

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
#define HEADEREDIT	'H'
#define DELITEM		'D'
#define REACTITEM	'R'
#define	CANCEL		'C'

#define ACTIVE		"ACT"
#define INACTIVE	"DEL"
#define	WEEKEND		'W'
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
#define HEADEREDIT	'H'
#define DELITEM		'D'
#define REACTITEM	'R'
#define	CANCEL		'A'

#define ACTIVE		"ACT"
#define INACTIVE	"ELI"
#define	WEEKEND		'W'
#endif

#define FOUND		0
#define NOTFOUND	1
#define NO_CHANGE	2

/* PROFOM Releted declarations */

#define	SCR_NAME	"sen_mnt"	/* PROFOM screen Name */

#define	PAGESIZE	4		/* No of Items */
/* PROFOM Screen STH file */


/* Field PROFOM numbers */

#define FN_FLD		400	/* Fn: */

#define KEY_START	500	/* Key Start Field */
#define KEY_END		500	/* Key End Field */
#define CHG_FLD		700

#define NAME_FLD	600	/* Description field */
#define STAT_FLD	1000	/* Description field */

#define PAGE_FLD	900	/* Page# Field */

#define	ITEM_ST_FLD	1000	/* Item Start Field */
#define ITEM_END_FLD	4300	/* Item End Field */

#define	MONTH_FLD	0
#define	POS_FLD		100
#define	CLASS_FLD	200
#define STATUS_FLD	700	/* Status Field */

#define	END_FLD		13300	/* Last Field of the screen */
#define	STEP		800	/* Last Field of the screen */

/* sen_mnt.sth - header for C structure generated by PROFOM EDITOR */

typedef	struct	{	/* Start Fld 1500, Endfld 4200 */

	short	s_month;	
	char	s_pos[7];
	char	s_class[7];
	double	s_cashrs;
	double	s_casdays;
	double	s_totdays;
	double	s_permdays;
	char	s_status[4];

}	S_item ;

typedef struct	{

	char	s_pgname[11];	/* 100 STRING X(10) */
	long	s_rundate;	/* 300 DATE YYYYFMMFDD */
	char	s_fn[2];	/* 400 STRING X */
	char	s_emp[13];	/* 500 STRING X(6) */
	char	s_name[29];	/* 500 STRING X(6) */
	short	s_field;	/* 600 NUMERIC 99 */
	short	s_page;	/* 600 NUMERIC 99 */

	S_item	s_items[PAGESIZE] ;	/* Start Fld 1500, End Fld 4200  */

	char	s_mesg[78];	/* 4300 STRING X(77) */
	char	s_resp[2];	/* 4400 STRING X */
} s_struct;

static	s_struct  s_sth,image;	/* PROFOM Screen Structure */
static	struct  stat_rec  sr;		/* PROFOM status rec */

static	char 	e_mesg[200];  		/* dbh will return err msg in this */

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

static	Emp_sen	emp_sen, pre_emp_sen;
static	Emp	emp_rec;
static	Pay_param	pay_param;
static	Pay_per		pay_period;
static	Pay_per_it	pay_per_it;
static	Class	class;
static	Position	position;

static	int	mode;

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

 	err = get_pay_param(&pay_param,BROWSE,1,e_mesg);
	if(err < 0) {
  		DispError((char *)&s_sth,e_mesg);
		return(err);
	}

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
	fomer("C(hange), I(nquire), N(ext), P(rev), E(xit)");
#else
	fomer("R(ajouter), C(hanger), I(nterroger), S(uivant), P(rec), F(in)");
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
	case CHANGE  :			/* CHANGE */
		CHKACC(retval,UPDATE,e_mesg);
		mode = UPDATE;
		return( Change() ) ;
	case INQUIRE  :			/* Inquire */
		CHKACC(retval,BROWSE,e_mesg);
		mode = INQUIRE;
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
	int retval;

	strcpy(emp_rec.em_numb,s_sth.s_emp);
	if (flg_start(EMPLOYEE) != direction) {
		inc_str(emp_rec.em_numb, sizeof(emp_rec.em_numb)-1, 
			direction);
		flg_reset(EMPLOYEE);
	}

	retval = get_n_employee(&emp_rec, BROWSE, 0, direction, e_mesg);
#ifndef ORACLE
	seq_over(EMPLOYEE);
#endif
	if(ERROR == retval)return(DBH_ERR) ;
	if(EFL == retval) {
		fomen("No More Records....");
		get();
		return(NOERROR) ;
	}
	retval=UsrBargVal(BROWSE,emp_rec.em_numb,emp_rec.em_barg,1,e_mesg);
	if(retval < 0){
		DispError((char *)&s_sth,e_mesg);
		return(NOERROR);
	}
	
	strcpy(s_sth.s_emp,emp_rec.em_numb);
	strcpy(s_sth.s_name, emp_rec.em_last_name);
	strcat(s_sth.s_name, ", ");
	strcat(s_sth.s_name, emp_rec.em_first_name);
	strcat(s_sth.s_name, " ");
	strcat(s_sth.s_name, emp_rec.em_mid_name);
	s_sth.s_name[28] = '\0';

	ret(WriteFields((char *)&s_sth,KEY_START,NAME_FLD));

	retval = ShowScreen();
	if(retval < 0) {
		return(retval);
	}

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
			ret( WriteFields((char *)&s_sth,PAGE_FLD,PAGE_FLD+100));
			i = 0 ;
		}
	}

	for( ; ; ) {
		if( PAGESIZE == i) {	/* Page Full */

			/* Move High Values to All items except first */
			for(i-- ; i > 0 ; i--)
			  InitItem(HV_CHAR,HV_SHORT,HV_LONG,HV_DOUBLE,i);

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
	int i,retval;
	
	CurLast = CurPage = '\0';
	i = 0;

	strcpy(emp_sen.esn_numb,s_sth.s_emp);
	emp_sen.esn_month = 0;
	emp_sen.esn_pos[0] = '\0';
	emp_sen.esn_class[0] = '\0';
	flg_reset(EMP_SEN) ;

	for( ; ; ) {
		retval = get_n_emp_sen(&emp_sen,BROWSE,0,FORWARD,e_mesg);
		if(retval < 0) {
			if(retval == EFL) break;
			DispError((char *)&s_sth,e_mesg);
			return(retval);
		}
		if(strcmp(emp_sen.esn_numb,s_sth.s_emp)!=0) { 
			break;
		}

		if(PAGESIZE == i) i = 0;
		if(i == 0) {
			if((retval = MakeFreshPage()) < 0) return(retval);
		}

		CurPage->Items[i].s_month = emp_sen.esn_month;	
		strcpy(CurPage->Items[i].s_pos,emp_sen.esn_pos); 
		strcpy(CurPage->Items[i].s_class,emp_sen.esn_class); 

		CurPage->Items[i].s_cashrs = emp_sen.esn_cas_hrs;
		CurPage->Items[i].s_casdays = emp_sen.esn_cas_days;
		CurPage->Items[i].s_totdays = emp_sen.esn_cas_totd;
		CurPage->Items[i].s_permdays = emp_sen.esn_perm_days;

		strcpy(CurPage->Items[i].s_status,ACTIVE);

		CurPage->I_Status[i][0] = ' ';
		CurPage->NoItems++;
		i++;
	}
	seq_over(EMP_SEN);

	if(CurLast != NULL) {
		CurPage = FirstPage;
	}

	if(retval == EFL) return(retval);
	return(NOERROR);
}
/*-------------------------------------------------------------*/
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
		InitItem(HV_CHAR,HV_SHORT,HV_LONG,HV_DOUBLE,i);

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
/*----------------------------------------------------------------------*/ 
/* Getting the mode which the record should be read to allow updates 	*/

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
	long	temp_date;
	int	retval, no_del;
	int	i, tmp_mode;

	no_del = 0;
	scpy((char *)&pre_emp_sen,(char *)&emp_sen,sizeof(emp_sen));

	tmp_mode = mode;
	if((temppage->Items[item_no].s_cashrs +
	    temppage->Items[item_no].s_casdays +
	    temppage->Items[item_no].s_totdays +
	    temppage->Items[item_no].s_permdays) == 0)
		mode = P_DEL;

	strcpy(emp_sen.esn_numb,s_sth.s_emp);
	emp_sen.esn_month = temppage->Items[item_no].s_month;
	strcpy(emp_sen.esn_pos,temppage->Items[item_no].s_pos);
	strcpy(emp_sen.esn_class,temppage->Items[item_no].s_class);
	flg_reset(EMP_SEN) ;


	if(mode != ADD) {
		retval = get_emp_sen(&emp_sen,UPDATE,0,e_mesg) ;
		if(retval == UNDEF && mode == P_DEL)
			no_del = 1;
		else{
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
	}
	if(no_del != 1){
	  emp_sen.esn_month = temppage->Items[item_no].s_month;
	  emp_sen.esn_cas_hrs = temppage->Items[item_no].s_cashrs;
	  emp_sen.esn_cas_days = temppage->Items[item_no].s_casdays;
	  emp_sen.esn_cas_totd = temppage->Items[item_no].s_totdays;
	  emp_sen.esn_perm_days = temppage->Items[item_no].s_permdays;

	  retval = put_emp_sen(&emp_sen,mode,e_mesg) ;
	  if(retval < 0) {
		DispError((char *)&s_sth,e_mesg);
		roll_back(e_mesg);
		return(retval);
	  }

	  if(mode != ADD) {
		retval = rite_audit((char*)&s_sth,EMP_SEN,mode,(char*)&emp_sen,
			(char*)&pre_emp_sen,e_mesg);
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

	scpy((char *)&image,(char *)&s_sth,sizeof(image));

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


	if(mode == ADD){
		st_fld  = ITEM_ST_FLD + (STEP * item_no) + 0;
		end_fld  = ITEM_ST_FLD + (STEP * item_no) + (STEP - 100);
		InitItem(LV_CHAR,LV_SHORT,LV_LONG,LV_DOUBLE,item_no); 
	}
	else {
		st_fld  = ITEM_ST_FLD + (STEP * item_no) + 400;
		end_fld  = ITEM_ST_FLD + (STEP * item_no) + (STEP - 100);
		s_sth.s_items[item_no].s_cashrs = LV_DOUBLE;
		s_sth.s_items[item_no].s_casdays = LV_DOUBLE;
		s_sth.s_items[item_no].s_totdays = LV_DOUBLE;
		s_sth.s_items[item_no].s_permdays = LV_DOUBLE;
	}
	strcpy(s_sth.s_items[item_no].s_status,ACTIVE);

	i = ReadFields((char *)&s_sth,st_fld,end_fld,Validation,WindowHelp,1) ;
		
	if(PROFOM_ERR == i || DBH_ERR == i) return(i) ;
	if(RET_USER_ESC == i) {
		if(mode == ADD) {
			InitItem(HV_CHAR,HV_SHORT,HV_LONG,HV_DOUBLE,item_no);
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
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Changing fields. Accept fld of the student to be changed and read     */
/* that fld 		 */

ChangeFields()
{
	int retval;

	/* make copy screen every time field changed in case user */
	/* presses ESC-F */
	scpy((char*)&image,(char*)&s_sth, sizeof(image));

	SetDupBuffers(ITEM_ST_FLD, END_FLD - 200,1);

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
			(int (*)())'\0',(int (*)())'\0', 1);

		if (retval < 0) return(-1);
		if (retval == RET_USER_ESC) break;	/* User enter ESC-F */

       		if (s_sth.s_field == 0 ) break;  /* Finished changing fields */

		if(s_sth.s_field > PAGESIZE){
			DispError((char *)&s_sth,BADFIELD);
			continue;
		}

		if(CurPage->Items[s_sth.s_field-1].s_status[0] == '\0')
			continue;
		if(strcmp(CurPage->Items[s_sth.s_field-1].s_status,INACTIVE)==0){
			fomer("Item Has a Deleted Status Cannot Edit");
			continue;
		}
		retval = ReadItem(s_sth.s_field - 1,UPDATE);
		if(retval < 0 && retval != RET_USER_ESC) {
			return(retval);
		}

		if(retval == RET_USER_ESC)
			continue;

		/* make copy screen every time field changed in case user */
		/* presses ESC-F */
		scpy((char*)&image,(char*)&s_sth, sizeof(image));

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
	int	i, item_no, fld_no;
	long	julian, curr_date;
	int	remain;

	if(sr.curfld == KEY_START) {
		fld_no = KEY_START;
	}
	else { 
		item_no = (sr.curfld - ITEM_ST_FLD) / STEP;
		fld_no = (sr.curfld - ITEM_ST_FLD) % STEP;
	}

	switch(fld_no){
	case KEY_START:
		Right_Justify_Numeric(s_sth.s_emp,
			sizeof(s_sth.s_emp)-1);
		strcpy(emp_rec.em_numb,s_sth.s_emp);

		retval = get_employee(&emp_rec,BROWSE,0,e_mesg);
		if(retval < 0) {
			fomer("Employee Number Does not Exist");
			s_sth.s_emp[0] = LV_CHAR;
			return(ERROR);
		}
	  	retval=UsrBargVal(BROWSE,emp_rec.em_numb,emp_rec.em_barg,1,
									e_mesg);
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
		s_sth.s_name[28] = '\0';

		ret(WriteFields((char *)&s_sth,NAME_FLD,NAME_FLD));
		break;
	case MONTH_FLD:
		if(s_sth.s_items[item_no].s_month < 1 ||
		   s_sth.s_items[item_no].s_month > 12){
			fomen("Invalid Month");
			s_sth.s_items[item_no].s_month = LV_SHORT;
			return(ERROR);
		}
		break;
	
	case POS_FLD:
		Right_Justify_Numeric(s_sth.s_items[item_no].s_pos,
				sizeof(s_sth.s_items[item_no].s_pos)-1);
		retval = GetPos(s_sth.s_items[item_no].s_pos);
		if(retval < 0){
			s_sth.s_items[item_no].s_pos[0] = LV_CHAR;
		    	return(ERROR);
		}
		break;
	case CLASS_FLD:
		Right_Justify_Numeric(s_sth.s_items[item_no].s_class,
				sizeof(s_sth.s_items[item_no].s_class)-1);
		retval = GetClass(s_sth.s_items[item_no].s_class);
		if(retval < 0){
			s_sth.s_items[item_no].s_class[0] = LV_CHAR;
		    	return(ERROR);
		}
		retval = CheckCode(item_no);
		if(retval < 0){
			s_sth.s_items[item_no].s_month = LV_SHORT;
			s_sth.s_items[item_no].s_pos[0] = LV_CHAR;
			s_sth.s_items[item_no].s_class[0] = LV_CHAR;
			sr.curfld -=400;
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
	int	fld_no, item_no;
	long	tmp_date;
	int	i;

	tmp_date = 0;
	if(sr.curfld == KEY_START) {
		fld_no = KEY_START;
	}
	else { 
		item_no = (sr.curfld - ITEM_ST_FLD) / STEP;
		fld_no = (sr.curfld - ITEM_ST_FLD) % STEP;
	}
	switch(fld_no){
	case KEY_START:
		retval = emp_hlp(s_sth.s_emp,7,13);
		if(retval == DBH_ERR) return(retval);
		if(retval >= 0) redraw();
		if(retval == 0) return(ERROR);
		if(retval < 0) return(ERROR);
		Right_Justify_Numeric(s_sth.s_emp,
			sizeof(s_sth.s_emp)-1);
		strcpy(emp_rec.em_numb,s_sth.s_emp);

		retval = get_employee(&emp_rec,BROWSE,0,e_mesg);
		if(retval < 0) {
			fomer("Employee Number Does not Exist");
			s_sth.s_emp[0] = LV_CHAR;
			return(ERROR);
		}
	  	retval=UsrBargVal(BROWSE,emp_rec.em_numb,emp_rec.em_barg,1,
									e_mesg);
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
		s_sth.s_name[28] = '\0';

		ret(WriteFields((char *)&s_sth,KEY_START,NAME_FLD));
		break;
	case POS_FLD:
		  retval = position_hlp(s_sth.s_items[item_no].s_pos,
				7,13);
		  if(retval == DBH_ERR) return(retval);
		  if(retval >= 0) redraw();
		  if(retval == 0) return(ERROR);
		  if(retval < 0) return(ERROR);
		  retval = GetPos(s_sth.s_items[item_no].s_pos);
		  if(retval <0){
			s_sth.s_items[item_no].s_pos[0] = LV_CHAR;
		    	return(ERROR);
		  }
		  break;
	case CLASS_FLD:
		  retval = class_hlp(s_sth.s_items[item_no].s_class,
				&tmp_date,7,13);
		  if(retval == DBH_ERR) return(retval);
		  if(retval >= 0) redraw();
		  if(retval == 0) return(ERROR);
		  if(retval < 0) return(ERROR);
		  retval = GetClass(s_sth.s_items[item_no].s_class);
		  if(retval <0){
			s_sth.s_items[item_no].s_class[0] = LV_CHAR;
		    	return(ERROR);
		  }
		  retval = CheckCode(item_no);
		  if(retval < 0){
			s_sth.s_items[item_no].s_month = LV_SHORT;
			s_sth.s_items[item_no].s_pos[0] = LV_CHAR;
			s_sth.s_items[item_no].s_class[0] = LV_CHAR;
			sr.curfld -=400;
			return(ERROR);
		  }
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
			err = GetOption((char *)&s_sth,"O(ui), A(nnul)","OA");
#endif
	    }
	    else if(s_sth.s_fn[0] != INQUIRE) {
#ifdef ENGLISH
			err = GetOption((char *)&s_sth,
		"Y(es), A(dd), L(ine), D(el), R(eactivate), N(ext), P(rev), C(ancel)"
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
/* Change Line items.  Allows editing of the students on the screen    */
/*---------------------------------------------------------------------*/
LineEdit()
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
		if(strcpy(CurPage->Items[i].s_status,INACTIVE)==0) {
			fomer("Item Has a Deleted Status");
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
	if ( WriteFields((char *)&s_sth,STATUS_FLD,STATUS_FLD)< 0) return(-1);

     	s_sth.s_mesg[0] = HV_CHAR ;
     	DispMesgFld((char *)&s_sth);

	if(SetDupBuffers(ITEM_ST_FLD, END_FLD-200,0)<0) return(PROFOM_ERR);

	return(NOERROR);
}	/* LineEdit() */
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

	/* Move High Values to The one item */
	for(i = 0 ; i < PAGESIZE ; i++ )
		InitItem(HV_CHAR,HV_SHORT,HV_LONG,HV_DOUBLE,i);

	ret( WriteFields((char *)&s_sth,PAGE_FLD, (END_FLD - 200)) );

	return(NOERROR);
}	/* ClearScreen() */
/*-------------------------------------------------------------------------*/
/* Initialize Given screen item with either Low values or High values */
InitItem(t_char, t_short, t_long, t_double, item_no)
char	t_char ;
short	t_short ;
long	t_long ;
double	t_double ;
int	item_no;
{
	int	i;
	int	retval;

	s_sth.s_items[item_no].s_month = t_short;	
	s_sth.s_items[item_no].s_pos[0] = t_char;
	s_sth.s_items[item_no].s_class[0] = t_char;
	s_sth.s_items[item_no].s_cashrs = t_double;
	s_sth.s_items[item_no].s_casdays = t_double;
	s_sth.s_items[item_no].s_totdays = t_double;
	s_sth.s_items[item_no].s_permdays = t_double;
	if(t_char == HV_CHAR)
		s_sth.s_items[item_no].s_status[0] = t_char;

	return(NOERROR);
}	/* Inititem() */
/*--------------------------------------------------------------*/
GetClass(class_rec)
char	*class_rec;
{
	int	retval;

	strcpy(class.c_code,class_rec);
	class.c_date = s_sth.s_rundate;
	flg_reset(CLASSIFICATION);

	retval = get_n_class(&class,BROWSE,0,BACKWARD,e_mesg);
	if(retval < 0 && retval != EFL){
		DispError((char *)&s_sth, e_mesg);
		return(ERROR);
	}
	if((strcmp(class.c_code,class_rec) != 0) || retval == EFL){
	    	return(EFL);
	}
	return(NOERROR);
}
/*--------------------------------------------------------------*/
GetPos(pos)
char	*pos;
{
	int	retval;

	strcpy(position.p_code,pos);

	retval = get_position(&position,BROWSE,0,e_mesg);
	if(retval < 0) {
		DispError((char *)&s_sth,e_mesg);
		return(ERROR);
	}

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
		if(temppage->Items[i].s_month == s_sth.s_items[item_no].s_month &&
		 strcmp(temppage->Items[i].s_pos,
			s_sth.s_items[item_no].s_pos) == 0 &&
		 strcmp(temppage->Items[i].s_class,
			s_sth.s_items[item_no].s_class) == 0){ 

		 fomer(
		"Employee Seniority Information Already Entered on Screen");
			return(ERROR);
		}
	      }
	      if(temppage == CurLast) break;
	   }
	}

	return(NOERROR);
}

/*-------------------- E n d   O f   P r o g r a m ---------------------*/