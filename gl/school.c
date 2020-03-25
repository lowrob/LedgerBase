/*-----------------------------------------------------------------------
Source Name: school.c         
System     : Budgetary Financial system.
Module     : General Ledger system.
Created  On: May 15, 1989.
Created  By: D. M. CORMIER.

DESCRIPTION:
	Program to Maintain School File.  Program has Add, Change, Delete,
        and Inquiry capability.  The School file has only 3 fields:  a 
        School number, name, and size or area.


MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
J.Prescott     91/02/12       Added Contact person.
E.Roy	       91/08/22	      Added Rural/Town.
------------------------------------------------------------------------*/

#define	MAIN	/* Main program. This is to declare Switches */
#define MAINFL		SCHOOL		/* main file used */

#define	SYSTEM		"SETUP"		/* Sub System Name */
#define	MOD_DATE	"12-FEB-91"	/* Program Last Modified */

#include <stdio.h>
#include <ctype.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#ifdef ENGLISH
#define ADDREC 	'A'
#define CHANGE 	'C'
#define DELETE	'D'
#define NEXT	'N'
#define PREV	'P'
#define INQUIRE	'I'
#define EXIT	'E'
#define EDIT	'E'
#define CANCEL	'C'
#define YES	'Y'
#define NO	'N'
#define TOWN	'T'
#define RURAL	'R'

#else	/* French */

#define ADDREC 	'R'
#define CHANGE 	'C'
#define DELETE	'E'
#define NEXT	'S'
#define PREV	'P'
#define INQUIRE	'I'
#define EXIT	'F'
#define EDIT	'M'
#define CANCEL	'A'
#define YES	'O'
#define NO	'N'
#define TOWN	'V'
#define RURAL	'T'
#endif

#define	SCR_NAME	"school"         /* First screen  */

/* PROFOM Field Numbers */

#define	FN_FLD_PFOM	400	/* Fn: */
#define	KEY_START_PFOM	500	/* School file key, School Number */
#define	CHG_FLD_PFOM	600     /* Field: */
#define SCH_NAME_PFOM   800     /* School Name */
#define SIZE_AREA_PFOM  1600     /* School's size, area */ 
#define RURAL_TOWN_PFOM 1700     /* School's size, area */ 

/* Screen 1 STH file */

/* Field Numbers on Screen */

#define	FLD1_NAME       1       /* School Name   field */
#define	FLD2_SIZE       9       /* School Size   field */
#define	LAST_FLD_SCR    10	/* Last Field Number on Screen */

/* fa050.sth - header for C structure generated by PROFOM EDITOR */

struct	s1_struct	{

	char	s1_pgm[11];       	/* STRING XXXXXXXXXX  Field 100 */
	long	s1_rundate;    	 	/* DATE YYYYFMMFDD    Field 300 */
	char	s1_fn[2];	 	/* STRING X           Field 400 */
	short	s1_numb_schl;	 	/* NUMERIC 99         Field 500 */
	short	s1_field;         	/* NUMERIC 99         Field 700 */
	char	s1_name_schl[29];	/* STRING X(28)       Field 800 */
	char	s1_add1[31];		/* Address line 1 field 900 */
	char	s1_add2[31];		/* Address line 2 field 1000 */
	char	s1_add3[31];		/* Address line 3 field 1100 */
	char	s1_pc[8];		/* postal code field 1200 */
	char	s1_contact[26];		/* Contact Person field 1300 */
	char	s1_phone[11];		/* telephone number field 1400 */
	char	s1_fax[11];		/* fax number field 1500 */
	long	s1_size_schl;     	/* NUMERIC 999999     Field 1600 */
	char	s1_r_t[2];		/* Flag Rural or Town (R/T)	*/
        char    s1_mesg[78];      	/* STRING X(77)       Field 1700 */
        char    s1_resp[2];       	/* STRING X           Field 1800 */
};


static	struct	s1_struct  s1_sth;	/* Screen Structure */

static	struct  stat_rec sr;		/* PROFOM status rec */

static	Sch_rec schl_rec ,	/* School File Record */
		pre_rec ;	/* Previous School Rec to write audit info */

static	char 	e_mesg[80];  	/* dbh will return err msg in this */

/* Screen Control Variables */
static  int     ST_FLD;         /* Data entry starting field */
static	int	END_FLD;	/* screen end field */
static	short	*Field;		/* Ptr to Change Field number */
static	char	*Mesg;		/* Message fld */
static	char	*Resp;		/* Response fld to user response */
static	char	*CurrentScreen;	/* Ptr to active screen */


main(argc,argv)
int argc;
char *argv[];
{
	int	err;

	if(argc < 2){
#ifdef  DEVELOP
		printf("MAIN ARGUMENTS ARE NOT PROPER\n");
		printf("Usage : %s {-tterminal name}\n", argv[0]);
#endif
		exit(0);
	}

	strncpy(SYS_NAME,SYSTEM,50);	/* Sub system name */
	strncpy(CHNG_DATE,MOD_DATE,10);	/* Modification Date */

	proc_switch(argc, argv, MAINFL) ; 	/* Process Switches */

	STRCPY(sr.termnm,terminal);	/* Copy Terminal Name */

	InitProfom() ;			/* Initialize PROFOM */

	err = Process(); 		/* Initiate Process */

	/* Set terminal back to normal mode from PROFOM */
	fomcs();
	fomrt();

	free_audit() ;	/* Free the space allocated for Fields */
	close_dbh();	/* Close files */

	if(err != NOERROR)exit(-1);
	exit(0);
} /* END OF MAIN */

/*-------------------------------------------------------------------*/
/* Initialize PROFOM */

InitProfom()
{
	fomin(&sr);
	/* Check for Error */
	if(err_chk(&sr) == PROFOM_ERR){
		fomcs();
		fomrt();
		exit(1);
	}
	fomcf(1,1);	/* Enable Snap screen option */
}	/* InitProfom() */
/*-------------------------------------------------------------------*/
/* Get Option from user and call corresponding function */

Process()
{
	int err;

	InitScreens() ;

	for( ; ; ){
		/* Display Fn: options */
#ifdef	ENGLISH
		fomer("A(dd), C(hange), D(elete), N(ext), P(rev), I(nquire), E(xit)");
#else
		fomer("R(ajouter), C(hanger), E(liminer), S(uiv), P(rec), I(nterroger), F(in)");
#endif

		/* Read Fn: field to get the option */
		sr.nextfld = FN_FLD_PFOM ;
		fomrf((char*)&s1_sth);
		ret(err_chk(&sr));	/* Check for PROFOM error */

		switch(s1_sth.s1_fn[0]){
		case ADDREC  :	/* Add */
			CHKACC(err,ADD,e_mesg);
			err = AddNewRec();
			break;
		case CHANGE  :	/* Change */
			CHKACC(err,UPDATE,e_mesg);
			err = ChangeRec();
			break;
		case DELETE  :	/* Delete */
			CHKACC(err,P_DEL,e_mesg);
			err = DeleteRec();
			break;
		case NEXT  :	/* Next    */
			CHKACC(err,BROWSE,e_mesg);
			err = NextRec() ;
			break ;
		case PREV  :	/* Previous*/
			CHKACC(err,BROWSE,e_mesg);
			err = PrevRec() ;
			break ;
		case INQUIRE  : 	/* Inquiry */
			CHKACC(err,BROWSE,e_mesg);
			err = InquiryRec();
			break ;
		case EXIT  :
			return(NOERROR);
		default   : 
			continue; 
		}  /*   end of the switch statement */

		if(err == NOACCESS) {
			fomen(e_mesg);
			get();
		}
		if(err == PROFOM_ERR)return(PROFOM_ERR); /* PROFOM ERROR */
		if(err == DBH_ERR) {
			DispError(e_mesg);
#ifdef ENGLISH
			sprintf(e_mesg,"%s %d Dberror: %d Errno: %d",
				"System Error... Iserror:",
				iserror, dberror, errno);
#else
			sprintf(e_mesg,"%s %d Dberror: %d Errno: %d",
				"Erreur du systeme... Iserror:",
				iserror, dberror, errno);
#endif
			DispError(e_mesg);
			return(DBH_ERR); /* DBH ERROR */
		}
	}      /*   end of the for( ; ; )       */
}	/* Process() */
/*----------------------------------------------------------------*/
/* Initialize screens before going to process options */
InitScreens()
{
	/* set active screen */
	Set1stScreen(0);

	STRCPY(s1_sth.s1_pgm,PROG_NAME);
	s1_sth.s1_rundate = get_date();	/* get Today's Date in YYMMDD format */
        s1_sth.s1_fn[0] = HV_CHAR ;
        s1_sth.s1_numb_schl = 0 ;
	s1_sth.s1_field = HV_SHORT ;


	/* Move High Values to 1st screen data fields & Display */
	MoveHighs() ;

}	/* InitScreens() */
/*---------------------------------------------------------------------*/
/* Set screen  */
Set1stScreen(flg)
int	flg ;	/* If Yes display the screen */
{
	/* move 1st screen name to Profom status structure */
	STRCPY(sr.scrnam,NFM_PATH);
	strcat(sr.scrnam,SCR_NAME) ;

	/* Initialize Ptrs to screen */
	Field = &s1_sth.s1_field ; 
	Mesg = s1_sth.s1_mesg;
	Resp = s1_sth.s1_resp;
	CurrentScreen = (char*)&s1_sth ;
	ST_FLD = 800 ;
	END_FLD = 1900 ;

	if(flg) {
		sr.nextfld = 1;
		sr.endfld = 0;
		fomwr(CurrentScreen);
		ret(err_chk(&sr));
	}
	return(NOERROR) ;
}	/* Set1stScreen() */
/*----------------------------------------------------------------------*/
/* Read the full screen and add the record to data base */
AddNewRec()
{
	int	i,err;

	for( ; ;){
		i = ReadKey();   /* Gets the school # from the screen */
		if(i != NOERROR)return(i);

		/* check whether given key already exists */
		i = BringRecord(BROWSE);
		if(i == NOERROR){
#ifdef	ENGLISH
			fomer("Given Key already in File - Please Enter again");
#else
			fomer("Cle donnee deja dans le dossier - Reessayer") ;
#endif
			continue;
		}
		/* UNDEF */
		break;
	}

	/* Change PROFOM logical field attributes */

	for(i = ST_FLD ; i <= END_FLD - 200  ; i += 100) {
		fomca1(i,19,0); /* Disable Dup control */
		fomca1(i,10,1);	/* Enable User Esacpe */
	}

	i = ReadScreen() ;	/* Read the rest of the screen fields */
	if(i != NOERROR) return(i) ;

	for(;;){

		i = ReadOption() ;
		if(i < 0) return(i) ;

		if(i == CANCEL)
			return(MoveHighs()) ;

		CopyToRecord(ADD) ;
		err = RiteRecord(ADD);
		if(err == LOCKED){
			roll_back(e_mesg);
			continue;
		}else
			break;
		
	}
	return(err);

}	/* AddNewRec() */
/*-----------------------------------------------------------------------*/

/* Accepts the changes to selected record and update the database */
ChangeRec()
{
	int	err ;

	err = SelectRecord(UPDATE) ;
	if(err != NOERROR) return(err) ;

	err = ChangeFields();
	if(err != NOERROR) return(err);
	for(;;){

		err = ReadOption() ;
		if(err < 0) return(err) ;

		if(err == CANCEL) {
			CopyToScreen() ;   /* Display the original record */
			roll_back(e_mesg);  /* Unlocking if recs not modified */
			return(NOERROR) ;
		}

		CopyToRecord(UPDATE) ;
		err = RiteRecord(UPDATE);
		if(err == LOCKED){
			roll_back(e_mesg);
			err = BringRecord(UPDATE);
			if(err < 0){
				fomen(e_mesg);
				get() ;
				if(err == UNDEF || err == LOCKED) continue ;
				return(DBH_ERR);
			}
		}else
		 	break;
	}

	return(err);
}	/* ChangeRec() */
/*-----------------------------------------------------------------------*/
/* Select the record to be deleted and delete from database afetr
   confirmation */
DeleteRec()
{
	int	err;

	err = SelectRecord(UPDATE) ;
	if(err != NOERROR) return(err) ;
	for(;;){

		err = ReadOption() ;
		if(err < 0) return(err) ;

		if(err == CANCEL) {
			roll_back(e_mesg);  /* Unlocking if recs not modified */
			return(NOERROR) ;
		}
		err = RiteRecord(P_DEL);
		if(err == LOCKED){
			roll_back(e_mesg);
			err = BringRecord(UPDATE);
			if(err < 0){
				fomen(e_mesg);
				get() ;
				if(err == UNDEF || err == LOCKED) continue ;
				return(DBH_ERR);
			}
		}else
			break;
	}
	return(err);

}	/* ChangeRec() */
/*-----------------------------------------------------------------------*/
/* Get next rec form file and Display when user selected 'N' in function */
NextRec()
{
	int 	err;

	err = NextSch(FORWARD);
	if (err != NOERROR) return (err) ;

	return(NOERROR) ;
}
/*-----------------------------------------------------------------------*/
/* Get Previous record from file and Display when user selected 'N' in function */
PrevRec()
{
	int 	err;

	err = NextSch(BACKWARD) ;
	if (err != NOERROR) return (err);

	return (NOERROR) ;
}
/*-----------------------------------------------------------------------*/
/* Select the Key, get the record & display */
InquiryRec()
{
	int	err ;

	err = SelectRecord(BROWSE) ;
	if(err != NOERROR) return(err) ;

	return(NOERROR) ;
}
/*----------------------------------------------------------*/
/* Read the key field, get the record with LOCK and display
   the Record */
SelectRecord(md)
int	md ;	/* BROWSE for Inquiry, UPDATE for Others */
{
	int	err ;

	for(; ;){
		err = ReadKey();
		if(err != NOERROR) return(err) ;

		/* Get the record from database */

		err = BringRecord(md);
		if(err < 0){
			fomen(e_mesg);
			get() ;
			if(err == UNDEF || err == LOCKED) continue ;
			return(DBH_ERR);
		}
		CopyToScreen() ;
		return(NOERROR) ;
	}
}	/* SelectRecord() */
/*----------------------------------------------------------------------*/
/* Read Key Fields */
ReadKey()
{
	int 	key_numb_schl ;
	int	err;
	
	if(s1_sth.s1_fn[0] != ADDREC)	/* ADD */
#ifdef	ENGLISH
		strcpy(Mesg,"Press ESC-F to Go Back to Fn and ESC-H for Help:");
#else
		strcpy(Mesg,"Appuyer sur ESC-F pour retourner a Fn et ESC-H pour assistance:");
#endif

	ShowMesg();

	/* In Add mode turn off dup control for key field.
	   Other modes reverse it */
	if(s1_sth.s1_fn[0] == ADDREC)	/* ADD */
		fomca1(KEY_START_PFOM,19,0);    /* Off Dup Control */
	else {
		fomca1(KEY_START_PFOM,19,2) ;

		sr.nextfld = KEY_START_PFOM;
		sr.endfld = KEY_START_PFOM;
		fomud((char*)&s1_sth);
	}

	/* Store key field to copy back when user gives ESC-F */
	key_numb_schl = s1_sth.s1_numb_schl ;

	sr.nextfld = KEY_START_PFOM;
	sr.endfld = KEY_START_PFOM;
	for(; ;) {
		fomrf((char*)&s1_sth);
		ret(err_chk(&sr));

		if (sr.retcode == RET_VAL_CHK){
			if(Validate(sr.curfld) == NOERROR) break;
			sr.nextfld = sr.curfld;
			continue;
		}

		if(sr.retcode == RET_USER_ESC){
			if(sr.escchar[0] == 'F' || sr.escchar[0] == 'f') {
				/* copy back key fields */
                		s1_sth.s1_numb_schl = key_numb_schl ;

				sr.nextfld = KEY_START_PFOM;
				sr.endfld = KEY_START_PFOM;
				fomwr((char*)&s1_sth);

				Mesg[0] = HV_CHAR;
				ShowMesg();
				return(ERROR) ;
			}
			if ((sr.escchar[0] == 'h' || sr.escchar[0] == 'H')
				&& (s1_sth.s1_fn[0] != ADDREC)){
				err = WindowHelp() ;
				if(err == DBH_ERR) return(err) ; 
				if(err == NOERROR) break;
				continue;
			} 
			continue;
		}
		if ((s1_sth.s1_numb_schl < 1) ||
		   (s1_sth.s1_numb_schl > 9999))
		   continue ;
		/* else RET_NO_ERROR */
		break;
	}			/* end of for loop */

	Mesg[0] = HV_CHAR;
	ShowMesg();

	return(NOERROR);
}	/*  ReadKey() */
/*------------------------------------------------------------*/
/* Get all fields from user on screen */
ReadScreen()
{
	/* Initialize screen with Low values */
	MoveLows() ;
	sr.nextfld = ST_FLD;
	sr.endfld = END_FLD - 200;
	for( ; ;){
		fomrd((char*)&s1_sth);
		ret(err_chk(&sr));
		if(sr.retcode == RET_VAL_CHK){
			if (Validate(sr.curfld) == NOERROR) break;
			sr.nextfld = sr.curfld;
			continue;
		}
		if(sr.retcode == RET_USER_ESC){
			if(sr.escchar[0] == 'f' || sr.escchar[0] == 'F') {
				roll_back(e_mesg);
				MoveHighs() ;
				return(ERROR);
			}
			continue;
		}
		/* else RET_NO_ERROR */
		break;
	}

	return(NOERROR) ;
}	/* ReadScreen() */

/*----------------------------------------------------------------*/
/* Check the school calendar to see if the date entered is a      */
WindowHelp()	/* Display help window for applicable fields */
{
	int	err ;

	if (sr.curfld == KEY_START_PFOM){
		err = sch_hlp(&s1_sth.s1_numb_schl, 12, 12);    
		if(err == DBH_ERR) return(err) ;
		if(err >=0 ) redraw();
		if(err > 0) return(NOERROR) ;
		return(ERROR) ;
        }

#ifdef ENGLISH
	fomer("No Help Window For This Field");
#else
	fomer("Pas de fenetre d'assistance pour ce champ");
#endif

	return(ERROR) ;
}	/* WindowHelp() */
/*----------------------------------------------------------------------*/
/* Changing fields. Accept fld to be changed and read that fld */
ChangeFields()
{
	int	i;

	/* Change PROFOM logical field attributes */

	for(i = ST_FLD ; i <= END_FLD - 200  ; i += 100) {
		fomca1(i,19,2); /* enabling Dup control */
		fomca1(i,10,0);	/* Disable User Escape */
	}

	/* Set Dup Buffers */
	sr.nextfld = ST_FLD;
	sr.endfld = END_FLD - 200;
	fomud(CurrentScreen);  /* Updating dup buffer */
	ret(err_chk(&sr));

	/* Get The Field to Be Modified */
#ifdef	ENGLISH
	strcpy(Mesg,"Enter RETURN to terminate Edit");
#else
	strcpy(Mesg,"Appuyer sur RETURN pour terminer l'ajustement");
#endif

	ShowMesg() ;

	for( ; ; ) {
		sr.nextfld = CHG_FLD_PFOM ;
		fomrf(CurrentScreen);
		ret(err_chk(&sr));

		if(*Field == 0) break ;

		i = ReadFld() ;	/* Read Field */
		if(i == PROFOM_ERR) return(i) ;
	}	/* for( ; ; ) */

	*Field = HV_SHORT ;
	fomwf(CurrentScreen);
	ret(err_chk(&sr));

	Mesg[0] = HV_CHAR;
	ShowMesg();

	return(NOERROR);
}	/* ChangeFields() */
/*-----------------------------------------------------------------------*/
/* Read the user selected field in change mode */
ReadFld()
{
	/* Validate  Field Number */
        /* The school number cannot be changed  */

	if( *Field < FLD1_NAME || *Field > LAST_FLD_SCR) 
          return (ERROR) ;

	/* Set PROFOM nextfld */
	sr.nextfld = ST_FLD + (*Field - 1) * 100 ;

	for( ; ; ) {
		fomrf(CurrentScreen);
		ret(err_chk(&sr));

		if (sr.retcode == RET_VAL_CHK){
			if(Validate(sr.curfld) == NOERROR) break;
			sr.nextfld = sr.curfld;
			continue;
		}

		if(sr.retcode != RET_NO_ERROR) continue;
		break ;
	}
	return(NOERROR) ;
}	/* ReadFld() */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Validate flds when PROFOM returns RET_VAL_CHK                         */
Validate(fld_no)	
int	fld_no ;
{
	int	err;

	switch(fld_no){
	case	SCH_NAME_PFOM:		/* School name	*/
		break ;
	case	SIZE_AREA_PFOM:		/* Size (Area)	*/
		break ;
	case 	RURAL_TOWN_PFOM:	/* Supplier Code */
		if (s1_sth.s1_r_t[0] != RURAL && 
				s1_sth.s1_r_t[0] != TOWN) {
#ifdef ENGLISH
			fomen("Must be R(ural) or T(own)");
#else
			fomen("Doit etre V(ille) ou ");
#endif
			s1_sth.s1_r_t[0] = LV_CHAR ;
			return(ERROR) ;
		}
		break ;

	default :
#ifdef ENGLISH
		sprintf(e_mesg,"No Validity Check For Field#  %d",sr.curfld);
#else
		sprintf(e_mesg,"Pas de controle de validite pour le Champ#  %d",sr.curfld);
#endif
		fomen(e_mesg);
		get();
		break;
	}	/* Switch fld_no */

	return(NOERROR) ;
}	/* Validate() */
/*-----------------------------------------------------------------------*/
/* Display the confirmation message at the bottom of the screen, take the
   option from user and call necessary functions */
ReadOption()
{
	int	err ;

	/* Options:
	   Add      - YEC
	   Change   - YEC
	   Delete   - YC
	*/

	for( ; ; ) {
	    switch(s1_sth.s1_fn[0]) {
	    case	ADDREC :	/* Add */
	    case	CHANGE : /* Change */   {
            
#ifdef	ENGLISH
		err = GetOpt("Y(es), E(dit), C(ancel)", "YEC");
#else
		err = GetOpt("O(ui), M(odifier), A(nnuler)", "OMA");
#endif
		break ; 
            }   /* end of case CHANGE */
	    case	DELETE :	/* Delete */
#ifdef	ENGLISH
		err = GetOpt("Y(es), C(ancel)", "YC");
#else
		err = GetOpt("O(ui), A(nnuler)", "OA");
#endif
		break ;
	    }	/* switch fn[] */

	    if(err == PROFOM_ERR) return(err) ;

	    switch(err) {
	    case  YES :
		return(YES) ;
	    case  EDIT  :
		err = ChangeFields();
		break ;
	    case  CANCEL : {
#ifdef	ENGLISH
		err = GetOpt("Confirm the Cancel (Y/N)?", "YN") ;
#else
		err = GetOpt("Confirmer l'annulation (O/N)?", "ON") ;
#endif
		if(err == YES) 
			return(CANCEL) ;
		break ;
            }  /* end of case CANCEL */
	    }	/* switch err */

	    if(err == PROFOM_ERR) return(err) ;
	}	/* for(; ; ) */
}	/* ReadOption() */
/*-----------------------------------------------------------------------*/
/* Display message and get the option */
GetOpt(msg,options)
char	*msg ;
char	*options ;
{
	int	i, j ;

	strcpy(Mesg,msg);
	ShowMesg() ;

	sr.nextfld = END_FLD ;
	for( ; ; ) {
		fomrf(CurrentScreen) ;
		ret(err_chk(&sr)) ;

		j = strlen(options) ;
		for( i = 0 ; i < j ; i++)
			if(Resp[0] == options[i]) break ;
		if(i != j) break ;	/* Valid Option Selected */
#ifdef	ENGLISH
		fomer("Invalid Option..");
#else
		fomer("Option invalide..");
#endif
	}
	Mesg[0] = HV_CHAR ;
	Resp[0] = HV_CHAR ;
	sr.nextfld = END_FLD - 100 ;
	sr.endfld = END_FLD ;
	fomwr(CurrentScreen) ;
	ret(err_chk(&sr)) ;

	return((int)(options[i])) ;
}	/* GetOpt() */
/*-----------------------------------------------------------------------*/
/* Write record in data base */
RiteRecord(md)
int	md ;
{
	int	err;

	err = put_sch(&schl_rec,md,e_mesg);
	if(err != NOERROR){
#ifdef ENGLISH
		DispError("ERROR in Saving Records"); 
#else
		DispError("ERREUR en conservant les fiches");
#endif
		DispError(e_mesg) ;
		roll_back(e_mesg);
		if(err == LOCKED) return(LOCKED);
		MoveHighs() ;
		return(DBH_ERR);
	}

	err = rite_audit((char*)&s1_sth, SCHOOL, md, (char*)&schl_rec,
				 (char*)&pre_rec, e_mesg);

	if(err == LOCKED){
		DispError(e_mesg);
		roll_back(e_mesg);
		 return(LOCKED);
	}

	if(err != NOERROR){
#ifdef ENGLISH
		DispError("ERROR in Saving Records"); 
#else
		DispError("ERREUR en conservant les fiches");
#endif
		DispError(e_mesg);
		roll_back(e_mesg);
		if(err == LOCKED) return(LOCKED);
		MoveHighs() ;
		return(DBH_ERR);
	}

	if(commit(e_mesg) < 0) {
#ifdef ENGLISH
		DispError("ERROR in Saving Records"); 
#else
		DispError("ERREUR en conservant les fiches");
#endif
		DispError(e_mesg) ;
		if(err == LOCKED) return(LOCKED);
		MoveHighs() ;
		return(DBH_ERR);
	}

	return(NOERROR);
}	/* RiteRecord() */
/*-----------------------------------------------------------------------*/ 
/* Copy the key fields from screen to data record and get the record from
   data base */
BringRecord(md)
int md; /* BROWSE or UPDATE */
{
        schl_rec.sc_numb = s1_sth.s1_numb_schl ;

	return(get_sch(&schl_rec, md, 0, e_mesg));
}
/*-----------------------------------------------------------------------*/ 
/* Read the next record from database */
NextSch(direction)
int	direction;
{
	int err;

	/* Check whether file is in seq. read mode */
	if(flg_start(SCHOOL) != direction) {
		/* Set the key to next possible key and set the file to start */
		if(direction == FORWARD)
		        schl_rec.sc_numb = s1_sth.s1_numb_schl + 1 ;
		else
		        schl_rec.sc_numb = s1_sth.s1_numb_schl - 1 ;
		flg_reset(SCHOOL) ;
	}

	err = get_n_sch(&schl_rec, BROWSE, 0, direction, e_mesg);

#ifndef	ORACLE
	seq_over(SCHOOL);
#endif

	if(ERROR == err)return(DBH_ERR) ;
	if(EFL == err) {
#ifdef	ENGLISH
		fomen("No More Records....");
#else
		fomen("Plus de fiches....");
#endif
		get();
		flg_reset(SCHOOL);
		return(ERROR) ;
	}

	CopyToScreen() ;

	return(NOERROR);
}
/*-----------------------------------------------------------------------*/
/* Copy the data record fields to Screen record and display              */
CopyToScreen()
{
	/* Copy screen fields */
	s1_sth.s1_numb_schl = schl_rec.sc_numb ;
	STRCPY(s1_sth.s1_name_schl, schl_rec.sc_name) ;
	STRCPY(s1_sth.s1_add1, schl_rec.sc_add1) ;
	STRCPY(s1_sth.s1_add2, schl_rec.sc_add2) ;
	STRCPY(s1_sth.s1_add3, schl_rec.sc_add3) ;
	STRCPY(s1_sth.s1_pc, schl_rec.sc_pc) ;
	STRCPY(s1_sth.s1_contact, schl_rec.sc_contact) ;
	STRCPY(s1_sth.s1_phone, schl_rec.sc_phone) ;
	STRCPY(s1_sth.s1_fax, schl_rec.sc_fax) ;
	s1_sth.s1_size_schl = schl_rec.sc_size ;
	strcpy(s1_sth.s1_r_t,schl_rec.sc_r_t);

	/* Display Screen */
	sr.nextfld = KEY_START_PFOM ;
	sr.endfld = END_FLD - 200;
	fomwr((char*)&s1_sth);
	return(NOERROR) ; 
}
/*-----------------------------------------------------------------------*/
/* Copy the Screen fields to data record */
CopyToRecord(md)
int	md ;
{
	/* Save the schl_rec in pre_rec to write audit records for changed
	   fields */
	if(md != ADD)
		scpy((char*)&pre_rec, (char*)&schl_rec, sizeof(Sch_rec)) ;

 	schl_rec.sc_numb = s1_sth.s1_numb_schl ;
	STRCPY (schl_rec.sc_name, s1_sth.s1_name_schl) ;
	STRCPY (schl_rec.sc_add1, s1_sth.s1_add1) ;
	STRCPY (schl_rec.sc_add2, s1_sth.s1_add2) ;
	STRCPY (schl_rec.sc_add3, s1_sth.s1_add3) ;
	STRCPY (schl_rec.sc_pc, s1_sth.s1_pc) ;
	STRCPY (schl_rec.sc_contact, s1_sth.s1_contact) ;
	STRCPY (schl_rec.sc_phone, s1_sth.s1_phone) ;
	STRCPY (schl_rec.sc_fax, s1_sth.s1_fax) ;
	schl_rec.sc_size = s1_sth.s1_size_schl ;
	STRCPY (schl_rec.sc_r_t, s1_sth.s1_r_t) ;
}
/*------------------------------------------------------------------------*/
/* Initialize 1st screen data fields with Low Values */
MoveLows()
{

	s1_sth.s1_name_schl[0] = LV_CHAR ;
	s1_sth.s1_add1[0] = LV_CHAR ;
	s1_sth.s1_add2[0] = LV_CHAR ;
	s1_sth.s1_add3[0] = LV_CHAR ;
	s1_sth.s1_pc[0] = LV_CHAR ;
	s1_sth.s1_contact[0] = LV_CHAR ;
	s1_sth.s1_phone[0] = LV_CHAR ;
	s1_sth.s1_fax[0] = LV_CHAR ;
	s1_sth.s1_size_schl = LV_LONG ;
	s1_sth.s1_r_t[0] = LV_CHAR ;

}	/* MoveLows() */
/*------------------------------------------------------------------------*/
/* Initialize screen data fields with High values and display the screen */
MoveHighs()
{
        s1_sth.s1_name_schl[0] = HV_CHAR ;
	s1_sth.s1_add1[0] = HV_CHAR ;
	s1_sth.s1_add2[0] = HV_CHAR ;
	s1_sth.s1_add3[0] = HV_CHAR ;
	s1_sth.s1_pc[0] = HV_CHAR ;
	s1_sth.s1_contact[0] = HV_CHAR ;
	s1_sth.s1_phone[0] = HV_CHAR ;
	s1_sth.s1_fax[0] = HV_CHAR ;
        s1_sth.s1_size_schl = HV_LONG ;
	s1_sth.s1_r_t[0] = HV_CHAR ;
	s1_sth.s1_mesg[0] = HV_CHAR ;
	s1_sth.s1_resp[0] = HV_CHAR ;

	sr.nextfld = ST_FLD ;
	sr.endfld = END_FLD - 200 ;
	fomwr((char*)&s1_sth);
	ret(err_chk(&sr));

	return(NOERROR);
}	/* MoveHighs() */
/*-------------------------------------------------------------------------*/
static
DispError(s)    /* show ERROR and wait */
char *s;
{
	strcpy(Mesg,s);
	ShowMesg();
#ifdef	ENGLISH
	fomen("Press any key to continue");
#else
	fomen("Appuyer sur une touche pour continuer");
#endif
	get();
	Mesg[0] = HV_CHAR;
	ShowMesg();
	return(ERROR);
}
/*------------------------------------------------------------------------*/
ShowMesg()  /* shows or clears message field */
{
	sr.nextfld = END_FLD - 100;
	fomwf(CurrentScreen) ;
}
/*-------------------- E n d   O f   P r o g r a m ---------------------*/

