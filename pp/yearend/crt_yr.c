/*------------------------------------------------------------------------
Source Name: crt_yr.c
System     : Personnel/Payroll System.
Created  On: July 05th, 93.
Created  By: Andre Cormier.

DESCRIPTION:
	Program to create a new directory and transfer necessary files.

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
------------------------------------------------------------------------*/

#define	MAIN		/* Main program. This is to declare Switches */

#define	SYSTEM		"END OF YEAR"	/* Sub System Name */
#define	MOD_DATE	"05-JUL-93"		/* Program Last Modified */

#include <stdio.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_com.h>
#include <bfs_pp.h>
#include <repdef.h>

/* User Interface define constants */
#ifdef ENGLISH
#define CREATE		'C'
#define EXITOPT		'E'

#define	YES		'Y'
#define NO		'N'
#define	EDIT		'E'
#define	CANCEL		'C'
#else
#define CREATE		'C'
#define EXITOPT		'F'

#define	YES		'O'
#define NO		'N'
#define	EDIT		'M'

#define	CANCEL		'A'
#endif

/* PROFOM Releted declarations */

#define	SCR_NAME	"crt_yr"	/* PROFOM screen Name */

/* Field PROFOM numbers */
#define START_FLD 	400	/* Start Field in range */
#define	END_FLD		900	/* Last Field of the screen */

#define	STEP		100

#define YEAR1		400	/* Starting Bargaining Unit		*/
#define OPTION		500	/* */
#define MESSAGE		600	/* */
#define RESPONSE	700	/* */

/* crt_yr.sth - header for C structure generated by PROFOM EDITOR */

typedef struct	{

	char	s_pgm[11];	/* 100 program name */
	long	s_rundate;	/* 300 run date */
	short	s_year;		/* 1000 ending trans ref. no. */
	char	s_option[2];	/* 1300 option choice */
	char	s_mesg[78];	/* 1500 message field */
	char	s_resp[2];	/* 1600 response field */
	} S_STRUCT;


S_STRUCT	s_sth;	/* PROFOM Screen Structure */
static	struct  stat_rec  sr;		/* PROFOM status rec */

static	char 	e_mesg[180];  		/* dbh will return err msg in this */

static	Validation();
static	WindowHelp();

static	int	PG_SIZE;
static	char	discfile[15];	/* for storing output name */
static	short 	pgcnt;		/* for page count */	

double 	D_Roundoff();
main(argc,argv)
int argc;
char *argv[];
{
	int 	retval;

	LNSZ = 132;
	retval = Initialize(argc,argv);	/* Initialization routine */

	if (retval == NOERROR) retval = Process();

	CloseRtn();			/* return to menu */
	if (retval != NOERROR) exit(1);
	exit(0);
}

/*-------------------------------------------------------------------*/
/* Initialize PROFOM */

Initialize(argc, argv)
int	argc ;
char	*argv[] ;
{
	int	retval ;

	/*
	*	Initialize DBH Environment
	*/
	strncpy(SYS_NAME,SYSTEM,50);	/* Sub system name */
	strncpy(CHNG_DATE,MOD_DATE,10);	/* Modification Date */

	proc_switch(argc, argv, CHEQUE) ; 	/* Process Switches */

	/*
	*	Initialize PROFOM & Screen
	*/
	STRCPY(sr.termnm,terminal);	/* Copy Terminal Name */
	fomin(&sr);
	ret(err_chk(&sr)) ;		/* Check for PROFOM Error */
	fomcf(1,1);			/* Enable Snap screen option */

	retval = InitScreen() ;		/* Initialize Screen */
	if(NOERROR != retval) return(retval) ;

	return(NOERROR) ;
}	/* Initialize() */

/*--------------------------------------------------------------------------*/
/* Close nessary files and environment before exiting program               */

CloseRtn() 
{
	/* Set terminal back to normal mode from PROFOM */
	fomcs();
	fomrt();

	close_dbh();	/* Close files */
	close_rep();	/* Close report */

	return(NOERROR);
}	/* CloseRtn() */
/*----------------------------------------------------------------*/
/* Initialize screen before going to process options */

InitScreen()
{
	/* move screen name to Profom status structure */
	STRCPY(sr.scrnam,NFM_PATH);
	strcat(sr.scrnam,SCR_NAME) ;

	STRCPY(s_sth.s_pgm,PROG_NAME);
	s_sth.s_rundate = get_date();	/* get Today's Date in YYYYMMDD format*/
	s_sth.s_option[0] = HV_CHAR;
	s_sth.s_mesg[0] = HV_CHAR ;
	s_sth.s_resp[0] = HV_CHAR ;

	/* Move Low Values to data fields */
	InitFields() ;

	return(NOERROR) ;
}	/* InitScreen() */

/*-------------------------------------------------------------------*/
/* Get Option from user and call corresponding function */

Process()
{
	int	retval;

	for( ; ; ){

		if((retval = ReadOption())<0) 
			return(retval);

		switch(s_sth.s_option[0]) {
		case  EXITOPT :
			return(NOERROR);
		case  CREATE :
			CHKACC(retval,ADD,e_mesg);
			retval = ProcOption() ;
			break ;
		default :
			continue;
		}

		if(NOACCESS == retval)	fomen(e_mesg);
		if(PROFOM_ERR == retval)	return(PROFOM_ERR);  /* PROFOM ERROR */
		if(DBH_ERR == retval) {
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
/*------------------------------------------------------------*/
ReadOption()
{

	s_sth.s_mesg[0] = HV_CHAR;
	DispMesgFld((char *)&s_sth);	
#ifdef ENGLISH
	fomer("C(reate), E(xit)");
#else
	fomer("C(reation), F(in)");
#endif
	sr.nextfld = OPTION;
	fomrf((char *)&s_sth);
	ret(err_chk(&sr));

}	/* ReadOption */
/*------------------------------------------------------------*/
ProcOption()
{
	int	i, retval ;

	for(i = START_FLD ; i <= END_FLD - 300 ; i += 100)
		fomca1(i, 19, 0) ;    /* disable dup control */

	retval = ReadYear(ADD) ;
	if(retval != NOERROR) return(retval) ;

	retval = Confirm() ;
	if(retval != YES) return(NOERROR) ;

	retval = ProcYear() ;
	if(retval < 0)	return(retval);

	CopyFile("emp_att");
	CopyFile("emp_sen");
	CopyFile("employee");

	return(NOERROR);
}	/* ProcSelection() */
/*------------------------------------------------------------*/
/* Get the Header details from user */

ReadYear(mode)
int	mode ;
{
	int	 i, retval ;

	if(mode == ADD) {
#ifdef ENGLISH
		STRCPY(s_sth.s_mesg,"Press ESC-F to Go to Option:");
#else
		STRCPY(s_sth.s_mesg,"Appuyer sur ESC-F pour retourner a Option:");
#endif
		DispMesgFld((char *)&s_sth);

		fomca1(YEAR1, 19, 2) ;
		s_sth.s_year = 0 ;
		sr.nextfld = YEAR1 ;
		sr.endfld = END_FLD - 300 ;
		fomud((char*)&s_sth);
		ret(err_chk(&sr));
	}
	InitFields() ;

	i = ReadFields((char *)&s_sth,START_FLD, END_FLD - 300,
			Validation, WindowHelp, 1) ;
	if(PROFOM_ERR == i || DBH_ERR == i) return(i) ;
	if(RET_USER_ESC == i) {	/* ESC-F */
		return(RET_USER_ESC) ;
	}

	return(NOERROR) ;
}	/* ReadYear() */
/*----------------------------------------------------------------*/
/* Validation function() for Key and Header fields when PROFOM returns
  RET_VAL_CHK */

static
Validation()
{

	return(NOERROR) ;
}	/* Validation() */
/*----------------------------------------------------------------*/
/* Show Help Windows, if applicable, when user gives ESC-H in key
   and header fields */

static
WindowHelp()
{

	fomer("No Help Window for This Field");

	return(NOERROR) ;
}	/* HdrAndKeyWindowHelp() */
/*-----------------------------------------------------------------------*/
/* Take the confirmation from user for the header part */

Confirm()
{
	int	retval ;

	for( ; ; ) {
#ifdef ENGLISH
		retval = GetOption((char *)&s_sth,"Y(es), E(dit), C(ancel)", "YEC");
#else
		retval = GetOption((char *)&s_sth,"O(ui), M(odifier), A(nnuler)", "OMA");
#endif
		if(retval == PROFOM_ERR) return(retval) ;

		switch(retval) {
		case  YES :
			return(YES) ;
		case  EDIT  :
			retval = FieldEdit();
			break ;
		case  CANCEL :
#ifdef ENGLISH
			retval = GetOption((char *)&s_sth,"Confirm the Cancel (Y/N)?", "YN") ;
#else
			retval = GetOption((char *)&s_sth,"Confirmer l'annulation (O/N)?", "ON") ;
#endif
			if(retval == YES) { 
				roll_back(e_mesg) ;	/* Unlock  Records */
				return(CANCEL) ;
			}
			break ;
		}	/* switch retval */

		if(retval == PROFOM_ERR) return(retval) ;
		if(retval == DBH_ERR) return(retval) ;
	}	/* for(; ; ) */
}	/* Confirm() */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Changing fields. Accept fld to be changed and read that fld 		 */

FieldEdit()
{
     	int	i,retval;

     	for ( i = START_FLD; i <= END_FLD - 300 ; i += 100 )
       		fomca1( i,19,2 );      		/*  enable Dup Control */

     	sr.nextfld = START_FLD;
     	sr.endfld = END_FLD - 300;
     	fomud( (char *) &s_sth );
     	ret(err_chk(&sr));

	retval = ReadYear(UPDATE);
	if(retval != NOERROR) return(retval) ;

     	return(NOERROR);
}	/* FieldEdit() */
/*-------------------------------------------------------------------------*/
/* Initialize Screen Header with either Low Values */

InitFields()
{
	s_sth.s_year = LV_SHORT;
	
	return(NOERROR) ;
}	/* InitFields() */
/*-----------------------------------------------------------------------*/
ProcYear()
{
	char	filenm[5];
	char	outfile[30];
	char	txt_buff[30];

	sprintf(filenm,"%d", s_sth.s_year);
        form_f_name(filenm,outfile);

#ifdef MSDOS
	sprintf(txt_buff,"mkdir %s", outfile);
#else
	sprintf(txt_buff,"mkdir %s", outfile);
#endif
	system(txt_buff);
	return(NOERROR) ;
}

/*-----------------------------------------------------------------------
Copy the specified file name from the school present year data directory 
into the school new year data directory.
-----------------------------------------------------------------------*/
static
CopyFile(filename)
char	*filename;
{
	char	filenm[5];
	static	int	retval;
	char	outfile[30];
	char	old_yr[30];
	char	txt_buff[30];

	sprintf(filenm,"%d", s_sth.s_year);
        form_f_name(filenm,old_yr);

        form_f_name("",outfile);

	sprintf(e_mesg, "Copying %-14s file", filename);
	fomen(e_mesg);
	fflush(stdout);

	sprintf(txt_buff,"cp %s%s.IX %s%s",outfile,filename,outfile,filenm);
	system(txt_buff);

	sprintf(txt_buff,"cp %s%s %s%s",outfile,filename,outfile,filenm);
	system(txt_buff);
	
	return(NOERROR);
}
/******************   END OF PROGRAM *******************************/
