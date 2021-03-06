/*-----------------------------------------------------------------------
Source Name: spooler.c
System     : UTILITIES.
Created  On: 6th Aug. 91.
Created  By: J. Prescott.

DESCRIPTION:

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
------------------------------------------------------------------------*/
#define	MAIN		/* Main program. This is to declare Switches */
#define MAINFL		-1 /* main file used */

#define	SYSTEM		"Utilities"	/* Sub System Name */
#define	MOD_DATE	"6-AUG-91"		/* Progran Last Modified */

#include <stdio.h>
#include <ftw.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

/* User Interface define constants */
#ifdef ENGLISH
#define	YES		'Y'
#define NO		'N'

#define SELECTRPT	'S'
#define DESELECTRPT	'D'
#define	REMOVE_FILE	'R'
#define	SPOOL_FILE	'S'
#define	END_OPT		'E'
#define NEXT		'N'
#define PREV		'P'
#define CANCEL		'C'
#else
#define	YES		'O'

#define SELECTRPT	'S'
#define DESELECTRPT	'D'
#define	REMOVE_FILE	'R'
#define	SPOOL_FILE	'S'
#define	END_OPT		'E'
#define NEXT		'N'
#define PREV		'P'
#define CANCEL		'C'
#endif

/* PROFOM Releted declarations */

#define	SCR_NAME	"spooler"	/* PROFOM screen Name */

#define	PAGESIZE	30		/* No of Items */

/* Field PROFOM numbers */

#define ST_FLD		700	/* Start Field */
#define END_FLD		7100	/* End Field */
#define ST_ITEM_FLD	1000	/* Starting Item Field */
#define	STEP		200	/* NO of fields diff. between 2 items */
#define	CHG_FLD		500	/* Field: */
#define PAGE_FLD	700 
#define MESG_FLD	7000
#define RESP_FLD	7100

/* spooler.sth - header for C structure generated by PROFOM EDITOR */

typedef	struct	{	/* Start Fld 1000, Endfld 6900 and Step 200 */

	char	s_filename[12];	/* 1000 - 6800 STRING X(11) */
	char	s_filedate[14];	/* 1100 - 6900 STRING X(11) */
}	S_item ;

typedef struct	{

	char	s_pgname[11];	/* 100 STRING X(10) */
	long	s_rundate;	/* 300 DATE YYYYFMMFDD */
	char	s_school[41];	/* 400 STRING X(40) */
	short	s_field;	/* 500 NUMERIC 99 */
	short	s_page;		/* 700 NUMERIC 99 */
	char	s_dummy1[3];	/* 800 STRING XX */
	char	s_dummy2[3];	/* 900 STRING XX */
	S_item	s_items[PAGESIZE] ;	/* Start Fld 1000, End Fld 6900  */
	char	s_mesg[78];	/* 7000 STRING X(77) */
	char	s_resp[2];	/* 7100 STRING X */

} s_struct;

static	s_struct  s_sth;	/* PROFOM Screen Structure */
static	struct  stat_rec  sr;		/* PROFOM status rec */
static	short	first_page = 1;

static	char 	e_mesg[100];  		/* dbh will return err msg in this */

typedef struct Page {
	S_item	Items[PAGESIZE] ;	/* Items Information */
	struct	Page	*PrevPage ;	/* ptr to previous page */
	struct	Page	*NextPage ;	/* ptr to next page */
	char	SelectStat[PAGESIZE][2];	/* Selection Status */
	short	NoItems;		/* number of Items on the page */
	short	Pageno;			/* Page number */
}	Page;

static	Page	*FirstPage,		/* Address of First Page */
		*CurPage,		/* Address of Current Page */
		*FileLast,		/* Address of Curr. record last page */
		*LastPage;		/* Address of Last Page of Memory
					   Allocated */
void	free() ;
char	*malloc() ;

main(argc,argv)
int argc;
char *argv[];
{
	int 	retval;

	retval = Initialize(argc,argv);	/* Initialization routine */

	if(retval == NOERROR) retval = Process();

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

static
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
static
InitScreen()
{
	int	err ;

	/* move screen name to Profom status structure */
	strcpy(sr.scrnam,NFM_PATH);
	strcat(sr.scrnam,SCR_NAME) ;

	strcpy(s_sth.s_pgname,PROG_NAME);

	s_sth.s_school[0] = HV_CHAR ; 

	s_sth.s_rundate = get_date();	/* get Today's Date in YYYYMMDD format*/
	s_sth.s_field = HV_SHORT ;
	s_sth.s_mesg[0] = HV_CHAR ;
	s_sth.s_resp[0] = HV_CHAR ;

	/* Move High Values to data fields and Display the screen */
	err = ClearScreen() ;
	if(NOERROR != err) return(err) ;

	return(NOERROR) ;
}	/* InitScreen() */

/*--------------------------------------------------------------*/

Process()
{
	int	retval;
	int	getfiles();
	int	err;

	/* Create Linked List of Files to Spool */
	ftw(".",getfiles,1);

	if(FileLast != NULL) {
		CurPage = FirstPage;
	}

	if((retval = ShowItems(CurPage)) < 0) return(retval);

	/* allow user to select function (Spool or Remove report files) */

	for(;;){

#ifdef	ENGLISH
		err = GetOption((char *)&s_sth, "E(xit), S(pool file) or R(emove file)", "ESR");
#else
		err = GetOption((char *)&s_sth, "E(xit), S(pool file) or R(emove file)", "ESR");
#endif
		switch(err){
		case REMOVE_FILE:
			retval = ConfirmRemove();
			if(retval < 0) return(-1); 

			if(retval == YES){
				err = RemoveRpt();
				if(err < 0) return(-1);
			}
			break;
	
		case SPOOL_FILE:
			retval = ConfirmSpool();
			if(retval < 0) return(-1); 
			if(retval == YES) {
				if((retval=PrintReport())<0) {
					redraw();
					DispError((char *)&s_sth,e_mesg);
					return(retval);
				}
			}
			break;

		case END_OPT:
	
			return(NOERROR);
		}
	}	

}	/* Process() */

/*------------------------------------------------------------*/
/* Get Filenames and add to linked List			      */

int getfiles(fname,statptr)
char	*fname;
struct	stat	*statptr;
{
	char	*ctime();
	int	retval;

	if(strcmp(strrchr(fname,'.')+1,"dat") == 0) { 
		if(PAGESIZE == CurPage->NoItems || first_page == 1) {
			if((retval = MakeFreshPage()) < 0) return(retval);
			first_page = 0;
		}

		CurPage->Items[CurPage->NoItems].s_filename[0] = '\0';
		strncpy(CurPage->Items[CurPage->NoItems].s_filename,
			strrchr(fname,'/')+1,11);
		CurPage->Items[CurPage->NoItems].s_filename[12] = '\0';

		strncpy(CurPage->Items[CurPage->NoItems].s_filedate,
			ctime(&statptr->st_mtime)+4,12);
		CurPage->Items[CurPage->NoItems].s_filedate[12] = '\0';
		CurPage->SelectStat[CurPage->NoItems][0] = ' ';
		CurPage->NoItems++;
	}

	return(NOERROR);
}

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

	if( LastPage == NULL || FileLast == LastPage ){
		tempptr= (Page *)malloc((unsigned)sizeof(Page)) ;

		if( tempptr == NULL ){
			DispError((char*)&s_sth,"Memory Allocation Error...");
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

	if(FileLast == NULL)
		FileLast = FirstPage ;
	else
		FileLast = FileLast->NextPage ;

	FileLast->NoItems = 0 ;
	CurPage = FileLast ;
	
	return(NOERROR);

}	/* MakeFreshPage() */
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

		s_sth.s_dummy1[0] = ' ' ;
		if(pageptr->NoItems > PAGESIZE / 2) {
			s_sth.s_dummy2[0] = ' ' ;
		}

		i = pageptr->NoItems ;
	}
	else {
		s_sth.s_page = HV_SHORT ;
		s_sth.s_dummy1[0] = HV_CHAR ;
		s_sth.s_dummy2[0] = HV_CHAR ;
		i = 0 ;
	}

	/* Move High Values to remaining Items */
	for( ; i < PAGESIZE ; i++ )
		InitItem(i, HV_CHAR) ;

	ret( WriteFields((char *)&s_sth, ST_FLD, (END_FLD - 200)) );

	return(NOERROR) ;
}	/* ShowItems() */

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
ConfirmSpool()
{
	int	err ;

	/* Options:
	     YSLNPC
	*/

	for( ; ; ) {
#ifdef ENGLISH
		err = GetOption((char *)&s_sth, "Y(es), S(elect Report), D(e-Select Report), N(ext), P(rev), C(ancel)", "YSDNPC");
#else
		err = GetOption((char *)&s_sth, "Y(es), S(elect Report), D(e-Select Report), N(ext), P(rev), C(ancel)", "YSDNPC");
#endif

	    switch(err) {
	    case  YES  :
		return(YES);

	    case  SELECTRPT  :
	    case  DESELECTRPT  :
		err = ChangeStatus(err);
		break ;

	    case  NEXT:
		if(CurPage == FileLast || FileLast == NULL) {
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
		if(FileLast == NULL || CurPage == FirstPage) {
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

	}	/* for(;;) */

}	/* ConfirmSpool() */

/*-----------------------------------------------------------------------
Changing fields. Accept fld of the student to be changed and read 
that fld.
-----------------------------------------------------------------------*/

ChangeStatus(status)
int	status;
{
	int retval;
	int fld_no;

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

		if (s_sth.s_field > CurPage->NoItems) continue;

		fld_no = ST_ITEM_FLD + (( s_sth.s_field - 1) * STEP);

		if(status == SELECTRPT) {
			if(CurPage->SelectStat[s_sth.s_field-1][0] != ' ') {
				fomer("Report Already Selected");
				continue;
			}
			else {
				CurPage->SelectStat[s_sth.s_field-1][0] = 
					SELECTRPT;
				/* Turn On HighLight */
				fomca1(fld_no,9,3);
				fomca1(fld_no+100,9,3);
			}
		}
		else {
			if(CurPage->SelectStat[s_sth.s_field-1][0] != 'S') {
				fomer("Report Not Selected");
			}
			else {
				/* Turn Off HighLight */
				CurPage->SelectStat[s_sth.s_field-1][0] = ' ';
				fomca1(fld_no,9,5);
				fomca1(fld_no+100,9,5);
			}
		}
	}

     	s_sth.s_field = HV_SHORT ;
	if ( WriteFields((char *)&s_sth,CHG_FLD, CHG_FLD) < 0 ) return(-1);

     	s_sth.s_mesg[0] = HV_CHAR ;
     	DispMesgFld((char *)&s_sth);

	return(NOERROR);
}	/* ChangeFields() */

/*------------------------------------------------------------------------*/
/* Move High values to all data fields and clear the screen */

ClearScreen()
{
	int	i;

	/* Move High Values to Hedaer part */

	s_sth.s_dummy1[0] = HV_CHAR ;
	s_sth.s_dummy2[0] = HV_CHAR ;
	s_sth.s_page = HV_SHORT;

	/* Move High Values to All items */
	for(i = 0 ; i < PAGESIZE ; i++)
		InitItem(i, HV_CHAR) ;

	ret( WriteFields((char *)&s_sth,ST_FLD, (END_FLD - 200)) );

	return(NOERROR);
}	/* ClearScreen() */
/*-------------------------------------------------------------------------*/
/* Initialize Given screen item with either Low values or High values */
static
InitItem(item_no, t_char)
int	item_no ;
char	t_char ;
{
	s_sth.s_items[item_no].s_filename[0] = t_char ;
	s_sth.s_items[item_no].s_filedate[0] = t_char ;

	return(NOERROR) ;
}	/* Inititem() */

PrintReport()
{
	int	retval;
	int	i;
	Page	*tempptr;
 
	if(FileLast != NULL) {
		for(tempptr=FirstPage;tempptr!=NULL;tempptr=tempptr->NextPage){
			for(i=0;i<tempptr->NoItems;i++) { 
				if(tempptr->SelectStat[i][0] == 'S') {
					retval = Send_PC(tempptr->Items[i].s_filename,"LST:",e_mesg);
					if(retval < 0) {
						return(retval);
					}
				}
			}
			if(tempptr == FileLast) break;
		}
	}
	return(NOERROR);
}

/*-------------------------------------------------------------------------
  Remove Spool Files 
-------------------------------------------------------------------------*/
ConfirmRemove()
{
	int	err ;

	/* Options: YRNP */

	for(;;) {
#ifdef ENGLISH
		err = GetOption((char *)&s_sth, "Y(es), S(elect Report), D(e-Select Report), N(ext), P(rev), C(ancel)", "YSDNPC");
#else
		err = GetOption((char *)&s_sth, "Y(es), S(elect Report), D(e-Select Report), N(ext), P(rev), C(ancel)", "YSDNPC");
#endif
		switch(err) {
		case  YES  :
			return(YES);

		case  NEXT:
			if(CurPage == FileLast || FileLast == NULL) {
#ifdef ENGLISH
				fomer("No More Pages....");
#else
				fomer("Plus de pages....");
#endif
				continue;
			}
			CurPage = CurPage->NextPage ;
			err = ShowItems(CurPage);
			if(err < 0) return(err);
			break;

		case  PREV:
			if(FileLast == NULL || CurPage == FirstPage) {
#ifdef ENGLISH
				fomer("No More Pages....");
#else
				fomer("Plus de pages....");
#endif
				continue;
			}

			CurPage = CurPage->PrevPage ;
			err = ShowItems(CurPage);
			if(err < 0) return(err);
			break;
	
		case  SELECTRPT  :
		case  DESELECTRPT  :
			err = ChangeStatus(err);
			break;

		}	/* switch err */

	}	/* for(;;) */

}	/* ConfirmRemove() */

/*-----------------------------------------------------------------------
Find what reports are selected for removal and remove them.
-----------------------------------------------------------------------*/

RemoveRpt()
{

	int	retval;
	int	i, fld_no;
	int	getfiles();
	Page	*tempptr;
	char	txt_buffer[40];
 
	if(FileLast != NULL) {
		for(tempptr=FirstPage;tempptr!=NULL;tempptr=tempptr->NextPage){
			for(i=0;i<tempptr->NoItems;i++) { 
				if(tempptr->SelectStat[i][0] == 'S') {

					/* remove selected file */

					strcpy(txt_buffer, "rm ");
					strcat(txt_buffer, 
						tempptr->Items[i].s_filename);
					system(txt_buffer);

					/* un-highlight field */

					fld_no = ST_ITEM_FLD + (i * STEP);
					fomca1(fld_no, 9, 5);
					fomca1(fld_no+100, 9, 5);
				}
			}
			if(tempptr == FileLast) break;
		}
	}

	first_page = 1;

	/* Free the linked list for the end */

	for( ;LastPage != FirstPage ; ) {
		LastPage = LastPage->PrevPage;
		free((char *)LastPage->NextPage);
		LastPage->NextPage = NULL;
	}

	if(FirstPage != NULL) {
		free((char *)FirstPage);
	}

	CurPage = FileLast = FirstPage = LastPage = NULL;

	/* Clear the screen */

	retval = ClearScreen() ;
	if(retval != NOERROR) return(retval) ;
 
	/* redisplay the spool files again */

	ftw(".",getfiles,1);
	if((retval = ShowItems(CurPage)) < 0) return(retval);

	return(NOERROR);
}

/*-------------------- E n d   O f   P r o g r a m ---------------------*/
