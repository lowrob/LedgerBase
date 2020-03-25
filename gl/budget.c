/*-----------------------------------------------------------------------
Source Name: budget.c
System     : Budgetary Financial system.
Module     : General Ledger system.
Created  On: 2nd May 89.
Created  By: Cathy Burns.

COBOL Source(s): gl080f--02

DESCRIPTION:
	Program to Maintain Budget Info in the G/L Master File.
        This program provides Add, Change, Inquiry & Delete
	options on Budget Info in the G/L Master File.
	This Program also writes Audit records for the changes.

Usage of SWITCHES when they are ON :
	SW7 - (Company) :
		Shows different Prompts for some of the fields.


	NOTE: SW2, SW4 & SW6 are reverted to their COBOL program meaning.

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
C.Leadbeater   90/11/21       	Changed ReadOption() so that budget code of 
				zero (annual budget) would not call the 
				AutoAdj() function. 

C.Leadbeater   90/11/22		AutoAdj() to allow entry of negative
				annual budget and new budget values for each
				period to be entered and automatically
				adjusted.

F. Tao		90/12/10	Move high value to Admissibility field. 
F. Tao 	    	90/12/18	Round up amounts before writing to file.
F. Tao 	    	90/12/30	Handle LOCKED file problem.             
------------------------------------------------------------------------*/
#define  MAIN
#define	 MAINFL		GLMAST

#include <stdio.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	SYSTEM		"GENERAL LEDGER"	/* Sub System Name */
#define	MOD_DATE	"23-JAN-90"		/* Program Last Modified */

#ifdef ENGLISH
#define ADDREC		'A'
#define CHANGE		'C'
#define DELETE		'D'
#define NEXT		'N'
#define PREV		'P'
#define INQUIRE		'I'
#define EXIT_OPT	'E'

#define	YES		'Y'
#define	EDIT		'E'
#define	CANCEL		'C'
#define NO		'N'
#else	/* FRENCH */
#define ADDREC		'R'
#define CHANGE		'C'
#define DELETE		'E'
#define NEXT		'S'
#define PREV		'P'
#define INQUIRE		'I'
#define EXIT_OPT	'F'

#define	YES		'O'
#define	EDIT		'M'
#define	CANCEL		'A'
#define NO		'N'
#endif

#define EXIT		12

#define SPACE           ' '

#define	SCR_NAME	"budget"	/* First screen */

/* PROFOM Field Numbers */

/* Screen Control Variables */
#define	ST_FLD	        1100		/* Data entry starting field */
#define	READ_FLD	1300		/* Data entry starting field */
#define	COL_BEGIN_FLD	2200		/* Data entry starting field */
#define	END_FLD  	7500		/* screen end field */

#define	FN_FLD		500		/* Fn: */
#define	KEY_START	600		/* Key Start Field */
#define FUND_FLD	600
#define	KEY_END		800		/* Key Start Field */
#define	CHG_FLD		900		/* Field: */
#define	RECCD_FLD	800		/* Record Code */
#define	ACCNO_FLD	700		/* Account Code */
#define	BUDGET_FLD	1300		/* budget Code */
#define	ANNUAL_FLD	1600		/* Annual budget Amount */
	
#define LASTCOL         3               /* The percent of budget column */

static	struct  stat_rec sr;		/* PROFOM status rec */

static	Gl_rec	gl_rec, pre_rec ;		/* G/L Master Record */
static 	Pa_rec	pa_rec;
static	char 	e_mesg[80];  		/* dbh will return err msg in this */


typedef struct {
	double	s_actual;	        /* 2200 - 7000 NUMERIC S99F999F999.99 */
	double	s_percent;	        /* 2300 - 7100 NUMERIC S999.9999 */
	long	s_new_budget;	        /* 2400 - 7200 NUMERIC S99F999F999.99 */
	double	s_new_percent;	        /* 2500 - 7300 NUMERIC S999.9999 */
        } S_PERIODS;
  
typedef struct  {
	char   	    s_pgm[11];		/* 100 STRING X(10) */
	long	    s_rundate;	        /* 300 DATE YYYYFMMFDD */
	char	    s_fn[2];    	/* 500 STRING X */
        char        s_fund_desc[6];     /* 550 FUND DESCRIPTION */
	short	    s_funds;	        /* 600 NUMERIC 999 */
	char	    s_accno[19];        /* 700 STRING X(18) */
	short	    s_rec_cod;	        /* 800 NUMERIC 99 */
	short	    s_field;	        /* 900 NUMERIC 999 */
	char	    s_desc[49];		/* 1100 STRING X(48) */ 
	char        s_desc_admis[15];   /* 1150 ADMIS DESCRIPTION */
	short	    s_admis;   		/* 1200 NUMERIC 9 */
	short	    s_budget_code;	/* 1300 NUMERIC 9 */
	char	    s_flg[2];		/* 1400 NUMERIC 9 */
	double	    s_increase;	        /* 1500 NUMERIC S999.9999 */
	long	    s_annual_budget;	/* 1600 NUMERIC S99F999F999 */
        char        s_dummy[3];         /* 1700 NOT USED  */
        S_PERIODS   s_periods[NO_PERIODS]; /* 2200 - 7300 periods */ 
        long 	    s_budtotal;         /* 7350 NUMERIC */
        double      s_pertotal;         /* 7375 NUMERIC */
	char	    s_mesg[78];	        /* 7400 STRING X(77) */
	char	    s_opt[2];	        /* 7500 STRING X */
	} s_struct;

static	s_struct  s_sth, image;		/* Screen-1 Structure */
static  int       no_periods;           /* Togle periods displayed */
static  short	  Prev_budget_code;
static	long	  prev_annual_budget;
static	long    remaining_budget;
 
double D_Roundoff();

/*-------------------------------------------------------------------*/

main(argc,argv)
int argc;
char *argv[];
{
	strncpy(SYS_NAME,SYSTEM,50);	/* Sub system name */
	strncpy(CHNG_DATE,MOD_DATE,10);	/* Modification Date */
	proc_switch(argc, argv, MAINFL) ; 	/* Process Switches */

	STRCPY(sr.termnm,terminal);	/* Copy Terminal Name */

	if (Initialize()<0)
		exit(-1);		/* Initialization routine */


	if ( Process() < 0) {; 		/* Initiate Process */
		Close();
		exit(-1);
	}

	Close();			/* return to menu */
	exit(NOERROR);

} /* END OF MAIN */

/*-------------------------------------------------------------------*/
/* Reset information */
Close()
{
	/* Set terminal back to normal mode from PROFOM */
	fomcs();
	fomrt();
	close_dbh();			/* Close files */
	free_audit();
}
/*-------------------------------------------------------------------*/
/* Initialize PROFOM  and Screens*/
Initialize()
{
	if(InitProfom()<0) { 			/* Initialize PROFOM */
		fomcs();
		fomrt();
		return(-1);
	}	
	if(InitScreens()<0) { 
		fomcs();
		fomrt();
		return(-1);
	}	
	return(NOERROR);
}

/*-------------------------------------------------------------------*/
/* Initialize PROFOM */

InitProfom()
{
	fomin(&sr);
	/* Check for Error */
	ret( err_chk(&sr) );

	fomcf(1,1);			/* Enable Print screen option */
	return(NOERROR);
}	/* InitProfom() */
/*----------------------------------------------------------------*/
/* Initialize screens before going to process options */
InitScreens()
{
	STRCPY(sr.scrnam,NFM_PATH);
	strcat(sr.scrnam,SCR_NAME) ;
	if(SW7) 
#ifdef ENGLISH
		STRCPY(s_sth.s_fund_desc," Co.:");
	else 
		STRCPY(s_sth.s_fund_desc,"Fund:");
#else
		STRCPY(s_sth.s_fund_desc," Cie:");
	else 
		STRCPY(s_sth.s_fund_desc,"Fond:");
#endif

	STRCPY(s_sth.s_pgm,PROG_NAME);
	s_sth.s_rundate = get_date();	/* get Today's Date in YYMMDD format */
	s_sth.s_field = HV_SHORT ;
	s_sth.s_mesg[0] = HV_CHAR ;
	s_sth.s_opt[0] = HV_CHAR ;

	/* Move Low Values to key fields. This gets 1st record from the file
	   if user selects 'N' option immediatly after invoking */
	s_sth.s_funds = 1 ;
	s_sth.s_accno[0] = LV_CHAR ;
	s_sth.s_rec_cod = LV_SHORT;
/*
	s_sth.s_rec_cod = 99;
*/
	/* Move High Values to 1st screen data fields & Display */
	MoveHighs() ;

	return(NOERROR);
}	/* InitScreens() */
/*-------------------------------------------------------------------*/
/* Get Option from user and call corresponding function */

Process()
{
	int retval;

	retval = get_param(&pa_rec,BROWSE,1,e_mesg);
	if (retval < 0) {
		if (retval == UNDEF)
#ifdef ENGLISH
			return(DispError("Parameters are not Setup ..."));
#else
			return(DispError("Parametres ne sont pas etablis..."));
#endif
		else	
			return(DispError(e_mesg));
	}

	for( ; ; ){
		/* Display Fn: options */

		if (ReadFunction() < 0 ) return(-1);

		switch(s_sth.s_fn[0]){
		case ADDREC  :	/* Add */
			CHKACC(retval,ADD,e_mesg);
			if( AddNewRec()<0 ) return(-1);
			break;
		case CHANGE  :	/* Change */
			CHKACC(retval,UPDATE,e_mesg);
			if( ChangeRec()<0 ) return(-1);
			break;
		case DELETE  :	/* Delete */
			CHKACC(retval,P_DEL,e_mesg);
			if( DeleteRec()<0 ) return(-1);
			break;
		case NEXT  : 	/* Next */
			CHKACC(retval,BROWSE,e_mesg);
			if( NextRec()<0 ) return(-1);  
			break ;
		case PREV  :	/* Previous */
			CHKACC(retval,BROWSE,e_mesg);
			if( PrevRec()<0 ) return(-1); 
			break ;
		case INQUIRE  :	/* Inquiry */
			CHKACC(retval,BROWSE,e_mesg);
			if( InquiryRec()<0 ) return(-1);
			break ;
		case EXIT_OPT  :
			return(NOERROR);
		default   : 
			continue; 
		}  /*   end of the switch statement */

		if( retval<0 ){
			fomen(e_mesg);
			get();
		}
		if( retval==DBH_ERR )
			return(retval);

	}      /*   end of the for( ; ; )       */

}	/* Process() */
/*-------------------------------------------------------------------*/
/* Reads the function they would like to execute      */
ReadFunction()
{

#ifdef	ENGLISH
	fomer("A(dd), C(hange), D(elete), N(ext), P(rev), I(nquire), E(xit)");
#else
	fomer("R(ajouter), C(hanger), E(liminer), S(uivant), P(recedent), I(nterroger), F(in)");
#endif

	/* Read Fn: field to get the option */
	sr.nextfld = FN_FLD ;
	fomrf((char*)&s_sth);
	ret(err_chk(&sr));	/* Check for PROFOM error */
	return(0);
}
/*---------------------------------------------------------------------*/
/*                                                      */
/* Read the full screen and add the record to data base */
AddNewRec()
{
     int row, err, i ,retval;

     if( (retval = SelectRecord(UPDATE)) < 0 ) return(-1);
     if( retval == RET_USER_ESC) return(retval);
     
     if (gl_rec.budcur != 0) {
#ifdef ENGLISH
        fomen("Budget already added");
#else
        fomen("Budget deja ajoute");
#endif
        get();
        return(NOERROR);
     }
     for ( i = READ_FLD; i <= 1600; i += 100 ) {
       fomca1( i,19,0 );	        /*  disable Dup Control */
       fomca1( i,10,1 );      	       	/*  enable user escape */
     }
     for ( i = COL_BEGIN_FLD + 200; i <= END_FLD - 200; i += 400 ) {
       fomca1( i,19,0 );       		/*  disable Dup Control */
       fomca1( i+100,19,0 );      	/*  disable Dup Control */
       fomca1( i,10,1 );       		/*  enable user escape */
       fomca1( i+100,10,1 );    	/*  enable user escape */
     }

     scpy((char*)&pre_rec,(char*)&gl_rec,sizeof(Gl_rec));
     err = ReadScreen(BUDGET_FLD);
     if (err == EXIT) return(NOERROR);
     if (err == RET_USER_ESC) {
	if(sr.curfld < 2200)
		row = 0 ;
	else
	        row = (sr.curfld - 2200) / 400;
        for(i = row; i < no_periods; i++) {
		s_sth.s_periods[i].s_new_budget = 0;
		s_sth.s_periods[i].s_new_percent = 0.00;
	}
     	if ((WriteFields(sr.curfld, END_FLD-200)) < 0) return(-1);
     } 

     for( ; ; ) {
	     err = ReadOption() ;
	     if (err < 0) return(-1);
  
	     if ( err == CANCEL ) 
       		 return( MoveHighs() ) ;

	     CopyToRecord();

	     err = RiteRecord(UPDATE);  
     
	     if(err==NOERROR) break;
	     if(err == LOCKED){
		roll_back(e_mesg);
		err = BringRecord(UPDATE); /* lock gl record again */
		if(err < 0){
			DispError(e_mesg);
			if(err == UNDEF || err == LOCKED) continue ;
			return(-1);
		}
		continue;
	     }
	     if (err < 0) {
		roll_back(e_mesg);
		return(err) ;
             }

     }
     return(NOERROR);

}   /*  AddNewRec  */
/*---------------------------------------------------------------------*/
/*                                                                     */
/* Change by field and update the record to data base                  */
ChangeRec()
{
     int err,retval,i; 

     remaining_budget = 0.00;

     if ((retval = SelectRecord(UPDATE)) < 0 ) return(-1);
     if( retval == RET_USER_ESC) return(retval);
     
     prev_annual_budget = s_sth.s_annual_budget;
     /* find amount of old budget used in remaining periods */
     for(i=pa_rec.pa_cur_period -1 ; i < no_periods;i++)
		remaining_budget += s_sth.s_periods[i].s_new_budget;

     scpy((char*)&pre_rec,(char*)&gl_rec,sizeof(Gl_rec));
     err = ChangeFields();
     if (err == EXIT) 
		return( NOERROR );
     else
	if ( err < 0 ) return(-1);

     
     for( ; ; ) {
	err = ReadOption() ;
	if ( err < 0 ) return(err);
  
        if ( err == CANCEL )  {
      	   gl_rec.cdbud = Prev_budget_code; /* display original screen */
           CopyToScreen() ;
           roll_back(e_mesg) ;
           return( NOERROR ) ;
         }

        CopyToRecord();
    

        err = RiteRecord(UPDATE);  
     	
	if(err==NOERROR) break;
	if(err == LOCKED) {
		roll_back(e_mesg);
		err = BringRecord(UPDATE); /* lock gl record again */
		if(err < 0){
			DispError(e_mesg);
			if(err == UNDEF || err == LOCKED) continue ;
			return(-1);
		}
		continue;
	}
        if (err < 0) {
	   roll_back(e_mesg);
	   return(err) ;
        }
     }
     return(NOERROR);

}	/* ChangeRec() */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Select the record to be deleted and delete from database after	 */
/* confirmation 							 */
DeleteRec()
{
       	int i, err,retval;
 
       	if( (retval = SelectRecord(UPDATE)) <0 ) return(-1) ;
        if( retval == RET_USER_ESC) return(retval);

    	if (gl_rec.budcur == 0) {
#ifdef ENGLISH
       		fomen("Budget information does not exist" );
#else
       		fomen("Information du budget n'existe pas" );
#endif
        	get();
        	return(NOERROR);
     	}
	
      	scpy((char*)&pre_rec,(char*)&gl_rec,sizeof(Gl_rec)); /* make a copy*/
	for( ; ; ) {
       	   err =  ReadOption();
          	if ( err < 0 ) return(err);

          	if ( err == CANCEL ) {
          	   	roll_back(e_mesg);
                	return(NOERROR);
       	        }

       	   s_sth.s_budget_code = 0 ;
           STRCPY(s_sth.s_flg, " ");
       	   s_sth.s_increase = 0.0000 ;
       	   s_sth.s_annual_budget = 0 ;

       	   for(i = 0 ; i < no_periods ; i++) {
	   	s_sth.s_periods[i].s_new_budget = 0 ;
           	s_sth.s_periods[i].s_new_percent = 0.0000 ;
       	   }

       	   CopyToRecord();
     
	   err=RiteRecord(UPDATE);
	   if(err==NOERROR) break;
	   if(err==LOCKED) {
		roll_back(e_mesg);
		err = BringRecord(UPDATE); /* lock gl record again */
		if(err < 0){
			DispError(e_mesg);
			if(err == UNDEF || err == LOCKED) continue ;
			return(-1);
		}
		continue;
	   }
           if (err < 0) {
	      roll_back(e_mesg);
	      return(err) ;
           }
	}
	return(NOERROR);
}	/* DeleteRec() */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Select the Key, get the record & display */
InquiryRec()
{
        int      err;
  
        if(  SelectRecord(BROWSE)<0 ) return(-1) ;

        return( NOERROR );
}
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Get next rec from file & Display when user selected 'N' in function */
NextRec()
{
       int    err;
 
       if( BringNext(0)<0 ) return(-1) ;

       return(NOERROR);
}
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Get Previous record from file & Display when user selected 'N' in function */
PrevRec()
{
       int    err;
 
       if( BringNext(1)<0 ) return(-1) ;

       return(NOERROR);
}
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Read the key fields, get the record with LOCK and display the record  */
SelectRecord(md)
int	md ;	/* BROWSE for Inquiry, UPDATE for Others */
{
	int	err,retval ;

	for(; ;){
		if( (retval =  ReadKey()) < 0 ) return(-1);
		if (retval == RET_USER_ESC) return(retval);

		/* Get the record from database */

		err = BringRecord(md);
		if(err < 0){
			DispError(e_mesg);
			if(err == UNDEF || err == LOCKED) continue ;
			return(-1);
		}
		CopyToScreen() ;
		return(NOERROR) ;
	}
}	/* SelectRecord() */
/*----------------------------------------------------------------------*/
/*                                                                       */
/* Read Key Fields */
ReadKey()
{
	short 	key_fund ;
	char 	key_acnt[sizeof(s_sth.s_accno)];
	short 	key_rec_cd ;
	int	i, err;

#ifdef	ENGLISH
	STRCPY(s_sth.s_mesg,"Press ESC-F to Go Back to Fn:");
#else
	STRCPY(s_sth.s_mesg,"Appuyer sur ESC-F pour retourner a Fn:");
#endif
	ShowMesg();

	/* Store key fields to copy back when user gives ESC-F */
	key_fund = s_sth.s_funds ;
	STRCPY(key_acnt,s_sth.s_accno) ;
	key_rec_cd = s_sth.s_rec_cod;

	for(i=KEY_START;i <= KEY_END;i+=100)
		fomca1(i,19,2);

	sr.nextfld = KEY_START;
	sr.endfld = KEY_END;
	fomud((char*)&s_sth);
	fund_default();
	sr.nextfld = KEY_START;
	sr.endfld = KEY_END;
	s_sth.s_funds = LV_SHORT ;
	s_sth.s_accno[0] = LV_CHAR ;
	s_sth.s_rec_cod = LV_SHORT ; 
	for(; ;) {
		fomrd((char*)&s_sth);
		ret(err_chk(&sr));

		if(sr.retcode == RET_VAL_CHK){
			Validate(sr.curfld); 
			sr.nextfld = sr.curfld;
			continue;
		}
		if(sr.retcode == RET_USER_ESC){
			if(sr.escchar[0] == 'F' || sr.escchar[0] == 'f') {
				/* copy back key fields */
				s_sth.s_funds = key_fund ;
				STRCPY(s_sth.s_accno,key_acnt) ;
				s_sth.s_rec_cod = key_rec_cd ; 

				if ((WriteFields(KEY_START,KEY_END)) < 0)
					return(-1);

				s_sth.s_mesg[0] = HV_CHAR;
				ShowMesg();
				return(RET_USER_ESC) ;
			}
			if(sr.escchar[0] == 'h' || sr.escchar[0] == 'H'){
				err = WindowHelp() ;
				if(err == DBH_ERR) return(err) ;
				continue;
			}
			continue;
		}
		break;
	}			/* end of for loop */

	s_sth.s_mesg[0] = HV_CHAR;
	ShowMesg();
	STRCPY(s_sth.s_pgm,PROG_NAME);

	return(NOERROR);
}	/*  ReadKey() */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Copy the key fields from screen to data record and get the record from
   data base */
static int
BringRecord(md)
int md; /* BROWSE or UPDATE */
{
	gl_rec.funds = s_sth.s_funds ;
	STRCPY(gl_rec.accno, s_sth.s_accno);
	gl_rec.reccod = s_sth.s_rec_cod ;

	return(get_gl(&gl_rec, md, 0, e_mesg));
}	/* BringRecord() */ 
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Changing fields. Accept fld to be changed and read that fld 		 */
ChangeFields()
{
     int i, j, k, err,fld_no;
     short row, col;

     for ( i = ST_FLD; i <= READ_FLD; i += 100 ) {
       fomca1( i,19,2 );       		/*  disable Dup Control */
       fomca1( i,10,1 );       		/*  enable user escape */
     }
     for ( i = READ_FLD; i <= ANNUAL_FLD; i += 100 ) {
       fomca1( i,19,2 );       		/*  disable Dup Control */
       fomca1( i,10,1 );       		/*  enable user escape */
     }
     for ( i = COL_BEGIN_FLD + 200; i <= END_FLD - 200; i += 400 ) {
       fomca1( i,19,2 );       		/*  disable Dup Control */
       fomca1( i+100,19,2 );       	/*  enable user escape */
       fomca1( i,10,1 );       		/*  disable Dup Control */
       fomca1( i+100,10,1 );       	/*  enable user escape */
     }

     sr.nextfld = READ_FLD;
     sr.endfld = END_FLD-600;
     fomud( (char *) &s_sth );
     ret(err_chk(&sr));

	/* Get The Field to Be Modified */
#ifdef	ENGLISH
	STRCPY(s_sth.s_mesg,"Press RETURN to terminate Edit");
#else
	STRCPY(s_sth.s_mesg,"Appuyer sur RETURN pour terminer l'ajustement");
#endif
	ShowMesg() ;
        
     for (; ;) {
	sr.nextfld = CHG_FLD;
       	fomrf( (char *) &s_sth);
       	ret( err_chk(&sr) );

	if(sr.retcode == RET_USER_ESC){
		if(sr.escchar[0] == 'f' || sr.escchar[0] == 'F') {
			break;
		}
  		continue;
	}


       	if (s_sth.s_field == 0 ) break;
       	if (s_sth.s_field == 2) continue;
	if (s_sth.s_budget_code == 0) 
		if (s_sth.s_field > 10 && s_sth.s_field < 133) continue;

 
        if ( s_sth.s_field >= 1 && s_sth.s_field <= 4 )
           fld_no = READ_FLD + (100 * (s_sth.s_field - 1)) ;
        else {
           row = s_sth.s_field / 10 ;
           col = s_sth.s_field % 10 ;
           if ( row < 1 || row > no_periods ) continue;
           if ( col < 1 || col > 2 ) continue;  
           fld_no = (COL_BEGIN_FLD + 200) + ((row - 1) * 400) + ((col-1)*100);
        }
     	scpy((char *)&image,(char *)&s_sth,sizeof(s_struct));
	if (fld_no == 1200 ) {
		s_sth.s_admis = HV_SHORT;
		continue;
	}
       	err = ReadScreen(fld_no) ;  		/*  Read Field */
	if (err == EXIT) break;
     	if (err == RET_USER_ESC ) {
		if (sr.curfld > 2200)
			sr.curfld -= 100;
		fomfp(sr.curfld,&j,&k);
     		ret(err_chk(&sr));
		i = sizeof(s_struct) - j ;
                scpy((char *)&s_sth + j,(char *)&image + j, i);
		if ((WriteFields(sr.curfld,END_FLD-200)) < 0) return(-1);
    	}
        else
       		if (err != NOERROR ) return(err) ;
     }  /*  for( ; ; ) */

     s_sth.s_field = HV_SHORT ;
     sr.nextfld = CHG_FLD;
     fomwf( (char *) &s_sth );
     ret( err_chk(&sr) );
     
     s_sth.s_mesg[0] = HV_CHAR ;
     ShowMesg();

     if (err == EXIT) return(EXIT);

     return(NOERROR);

}	/* ChangeFields() */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Read the user selected field in change mode				 */
ReadScreen(fld_no)
int	fld_no;
{
	int err;
	for ( ; ; ){
        	switch(fld_no) {
		case	BUDGET_FLD	:
			s_sth.s_budget_code = LV_SHORT;
			sr.nextfld = BUDGET_FLD;
			if( GetField() == RET_USER_ESC) return(EXIT);
			s_sth.s_increase = 0;
			if (s_sth.s_budget_code == 1 ||
			    s_sth.s_budget_code == 3)
				no_periods = 13;
			else {
				no_periods = 12;
				/* Blank out last line due to only 12 periods */
       				s_sth.s_periods[NO_PERIODS-1].s_actual = HV_DOUBLE;
         			s_sth.s_periods[NO_PERIODS-1].s_percent = HV_DOUBLE;
         			s_sth.s_periods[NO_PERIODS-1].s_new_budget = HV_LONG;
         			s_sth.s_periods[NO_PERIODS-1].s_new_percent = HV_DOUBLE;
	 			if ((WriteFields(END_FLD-500,END_FLD-200)) < 0) return(-1);
			}
			if(s_sth.s_budget_code == 1 ||
			   s_sth.s_budget_code == 2)  /* manual budget entry */
				fld_no = 1400;      
                	else 	fld_no = 1600;
       	        	break;
		case	1400		:	/* amt or % entry accept */
			s_sth.s_flg[0] = LV_CHAR; 
			sr.nextfld = 1400; 	
			if( GetField() == RET_USER_ESC) return(EXIT);
			fld_no = 1600;
                	break;
		case	1500		:	/*  increase % accept */
			if (s_sth.s_increase != 0) {   /* calculating orginal */
				s_sth.s_annual_budget = 
				    s_sth.s_annual_budget / 
				    (1.0+((double)s_sth.s_increase/100)) + 0.50;
			}

			s_sth.s_increase = LV_DOUBLE;
			sr.nextfld = 1500;
			if( GetField() == RET_USER_ESC) return(EXIT);
			if (s_sth.s_increase != 0) {  /* calculatin new value */
				s_sth.s_annual_budget =
				 s_sth.s_annual_budget * 
			         (1.0 + ((double)s_sth.s_increase/100)) + 0.50 ;
			}
			if ((WriteFields(1600,1600)) < 0) return(-1);
			if (s_sth.s_budget_code == 2 ||
			    s_sth.s_budget_code == 4) 
               			return(GetDetail());
			if ((WriteFields(1600,END_FLD-200)) < 0) return(-1);
	 		return(NOERROR);	
		case	1600	:		/* annual budget accept */
			s_sth.s_annual_budget = LV_LONG;
			sr.nextfld = 1600;
			if( GetField() == RET_USER_ESC) return(EXIT);
               		return(GetDetail());
         	default		:        
			sr.nextfld = fld_no;
        		if( GetField()<0 ) return(-1);
               		sr.nextfld = sr.curfld;
			fomwf((char*)&s_sth);
			ret(err_chk(&sr));
        		return(NOERROR);
		}
	}   /*  end of for( ; ;)  */

}	/* ReadScreen() */
/*------------------------------------------------------------*/
/* Write fields on Screen for a given Range */

static
WriteFields(st_fld, end_fld)
int	st_fld ;
int	end_fld ;
{
	sr.nextfld = st_fld ;
	sr.endfld  = end_fld ;

	if (end_fld > 2200) {
		if (no_periods < 13) {
			/* Blank out last line due to only 12 periods */
       			s_sth.s_periods[NO_PERIODS-1].s_actual = HV_DOUBLE;
         		s_sth.s_periods[NO_PERIODS-1].s_percent = HV_DOUBLE;
         		s_sth.s_periods[NO_PERIODS-1].s_new_budget = HV_LONG;
         		s_sth.s_periods[NO_PERIODS-1].s_new_percent = HV_DOUBLE;
		}
	}

	fomwr( (char *)&s_sth );
	ret(err_chk(&sr));

	return(NOERROR) ;
}	/* WriteFields() */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Reads only one field from the screen                                  */
GetField()             		
{
        for ( ; ;) {
		fomrf((char*)&s_sth);
		ret(err_chk(&sr));
		if(sr.retcode == RET_VAL_CHK) {
			if(Validate(sr.curfld) == NOERROR) break;
 			sr.nextfld = sr.curfld ;
  			continue;
		} 
	
		if(sr.retcode == RET_USER_ESC){
			if(sr.escchar[0] == 'f' || sr.escchar[0] == 'F') {
				roll_back(e_mesg);
				return(RET_USER_ESC);
			}
  			continue;
		}
       		break;
        }
        return(NOERROR);
}  /*  GetField */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Gets period information either budget amounts or percentages          */
GetDetail()
{
	double diff = 0.00;
        double budget, percent ; 
        int i;

	MoveLows();
        sr.nextfld = COL_BEGIN_FLD;
        sr.endfld = END_FLD -200;
					/* automatic budget distribution */
        if(s_sth.s_budget_code == 3 || s_sth.s_budget_code == 4) { 
/****
		budget = ((double)s_sth.s_annual_budget / (no_periods 
				 - pa_rec.pa_cur_period  + 1)) ;  
****/

		/* use remaining budget amount in each period */
		prev_annual_budget -= remaining_budget;

		budget = (((double)s_sth.s_annual_budget - 
			   (double)prev_annual_budget) / (no_periods -
				pa_rec.pa_cur_period  + 1)) ;  
                /*  This is so amounts will be truncated not rounded up */
		sprintf(e_mesg,"%ld",(long)(budget));
		sscanf(e_mesg,"%lf",&budget);
                /*  End of truncating code */
		if(s_sth.s_annual_budget == 0.00) {
			percent = 0.00;	
		}
		else {
	                percent = (budget * 100) / (double)s_sth.s_annual_budget ;
		}

     		for(i = pa_rec.pa_cur_period - 1;i < no_periods;i++ ) {
                	s_sth.s_periods[i].s_new_budget = budget ;
			s_sth.s_periods[i].s_new_percent = percent ;
   		}
        	fomwr( (char *) &s_sth );
        	ret( err_chk(&sr) );
                return(NOERROR);
        }
        for ( ; ; ) {
		fomrd((char *)&s_sth);
		ret(err_chk(&sr));
		if(sr.retcode == RET_VAL_CHK) {
			Validate(sr.curfld); 
			sr.nextfld = sr.curfld;
			continue;
		}
		if(sr.retcode == RET_USER_ESC) {
		   if(sr.escchar[0] == 'f' || sr.escchar[0] == 'F') 
			return(RET_USER_ESC);
		   continue;
                }
		break;
	}

        return(NOERROR);
}   /* GetDetail */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Display the confirmation message at the bottom of the screen, take the
   option from user and call necessary functions */
ReadOption()
{
	int	err ;

	/* Options:
	   Add      - YEC
	   Change   - YEC
	   Delete   - YC
	   Next     - Y
	   Previous - Y
	   Inquiry  - Y
	*/

	for( ; ; ) {
	    switch(s_sth.s_fn[0]) {
#ifdef ENGLISH
	    case	ADDREC :	/* Add */
	    case    	CHANGE :	/* Change */
                    err = GetOpt( "Y(es), E(dit), C(ancel)", "YEC");
	            break ;
	    case	DELETE :	/* Delete */
                    err = GetOpt( "Y(es), C(ancel)", "YC");
		    break ;
#else
	    case	ADDREC :	/* Add */
	    case    	CHANGE :	/* Change */
                 err = GetOpt( "O(ui), M(odifier), A(nnuler)", "OMA");
	            break ;
	    case	DELETE :	/* Delete */
                    err = GetOpt( "O(ui), A(nnuler)", "OA");
		    break ;
#endif
	    }	/* switch fn[] */

	    if(err == PROFOM_ERR) return(err) ;

	    switch(err) {
	    case  YES :		/* AutoAdj() only if budget code not 0. (CL) */
		if( (s_sth.s_fn[0] == ADDREC || s_sth.s_fn[0] == CHANGE) &&
		    (s_sth.s_budget_code != 0) ) {
			if(AutoAdj() == ERROR) continue ;
		}
		return(YES) ;
	    case  EDIT  :
		err = ChangeFields();
		break ;
	    case  CANCEL :
#ifdef	ENGLISH
                err = GetOpt("Confirm the Cancel (Y/N)?", "YN") ;
#else
                err = GetOpt("Confirmer l'annulation (O/N)?", "ON");
#endif
                if ( err == YES ) 
                   return( CANCEL );
		break ;
	    }	/* switch err */

	    if(err == PROFOM_ERR) return(err) ;
	}	/* for(; ; ) */
}	/* ReadOption() */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/*  Display help window for applicable fields                            */
WindowHelp()
{
	int	err ;

	switch(sr.curfld){
	case	ACCNO_FLD  :	/* Account# */
        case	RECCD_FLD  :	/* Record Code */
		err = gl_hlp(s_sth.s_funds, s_sth.s_accno,
			&s_sth.s_rec_cod, 7, 13 );
	 	if(err == DBH_ERR) return(err) ;
		if(err >=0 ) redraw();

		if(s_sth.s_rec_cod != 97 &&
		   s_sth.s_rec_cod != 98 &&
		   s_sth.s_rec_cod != 99 ) {
#ifdef ENGLISH
			fomer("Valid Record Codes are 97,98 and 99");
#else
			fomer("Codes de fiches valables sont 97,98 et 99");
#endif
			s_sth.s_rec_cod = LV_SHORT ;
			return(ERROR) ;
		}
		break ;
	default :
#ifdef ENGLISH
		fomer("No Help Window For This Field");
#else
		fomer("Pas de fenetre d'assistance pour ce champ");
#endif
	}	/* Switch fld_no */

	sr.nextfld = sr.curfld ;
	return(NOERROR) ;
}	/* WindowHelp() */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Automatic adjustment to last period entered                           */
AutoAdj()      
{
	int err, i, j, temp_amt, last_period ;
	
	double total_percent, percent;
	long total_budget, diff;
	
	total_budget = 0;
	total_percent = 0.0;
	
	for(i=0; i < pa_rec.pa_cur_period ; i++) {
		if(s_sth.s_periods[i].s_new_budget == HV_LONG)
			s_sth.s_periods[i].s_new_budget=0;
		if(s_sth.s_periods[i].s_new_percent == HV_DOUBLE)
			s_sth.s_periods[i].s_new_percent=0;
	}
	for(i=pa_rec.pa_cur_period - 1 ;i < no_periods;i++) {

	    if( s_sth.s_annual_budget > 0 ){
		if(s_sth.s_periods[i].s_new_budget > 0)
			last_period = i;
	    }

	    if( s_sth.s_annual_budget < 0 ){ 
		if(s_sth.s_periods[i].s_new_budget < 0)
			last_period = i;
	    } 
	}
	for(i=0;i < no_periods;i++) {
	    total_budget += s_sth.s_periods[i].s_new_budget;
	    total_percent += (double)s_sth.s_periods[i].s_new_percent;
	}

	if ( total_budget == 0 ) 
		last_period = 0 ;
	
        if ( total_budget == s_sth.s_annual_budget )
		return(NOERROR);

	if(s_sth.s_budget_code == 1 || s_sth.s_budget_code == 2) { 
		
		s_sth.s_budtotal = total_budget;
		s_sth.s_pertotal = total_percent;
                
		sr.nextfld = 7350;
                sr.endfld =7375;
        	fomwr( (char *) &s_sth );
        	ret( err_chk(&sr) );
#ifdef ENGLISH
               	err = GetOpt("Automatic adjustment (Y/N)?", "YN");
#else
               	err = GetOpt("Ajustement automatique (O/N)?", "ON");
#endif
        	s_sth.s_budtotal = HV_LONG;
        	s_sth.s_pertotal = HV_DOUBLE;
                sr.nextfld = 7350;
                sr.endfld =7375;
        	fomwr( (char *) &s_sth );
        	ret( err_chk(&sr) );
                if (err == PROFOM_ERR) return(err);
        	if (err == NO) return(ERROR);  
	}
			
	diff = s_sth.s_annual_budget - total_budget;
/*	temp_amt =  diff /no_periods;	*/	
		
	if( s_sth.s_annual_budget < 0 ){

	    for ( ; last_period >= pa_rec.pa_cur_period - 1; last_period--) {

		s_sth.s_periods[last_period].s_new_budget += diff;
		if (s_sth.s_periods[last_period].s_new_budget > 0 ) {

			diff = s_sth.s_periods[last_period].s_new_budget ;
			s_sth.s_periods[last_period].s_new_budget = 0 ;
			s_sth.s_periods[last_period].s_new_percent = 0.0 ;
		}
		else
			break;	
	    }
	} 
	
	else
	if( s_sth.s_annual_budget > 0 ){

	    for ( ; last_period >= pa_rec.pa_cur_period - 1; last_period--) {

		s_sth.s_periods[last_period].s_new_budget += diff;
		if (s_sth.s_periods[last_period].s_new_budget < 0 ) {
	
			diff = s_sth.s_periods[last_period].s_new_budget ;
			s_sth.s_periods[last_period].s_new_budget = 0 ;
			s_sth.s_periods[last_period].s_new_percent = 0.0 ;
		}
		else
			break;

	    }	
	} 
	
	for (i=pa_rec.pa_cur_period - 1;i < no_periods;i++) {
			percent = ((s_sth.s_periods[i].s_new_budget * 100.0) 
					/ s_sth.s_annual_budget );
			s_sth.s_periods[i].s_new_percent = (double)percent;
	}
/*	s_sth.s_periods[last_period].s_new_budget +=
			 (diff - temp_amt * no_periods);
	percent = ((s_sth.s_periods[last_period].s_new_budget * 100.0) 
			/ s_sth.s_annual_budget );
	s_sth.s_periods[last_period].s_new_percent = (double)percent;
*/

	if ((WriteFields(1600, END_FLD-200)) < 0) return(-1);
	
	if (s_sth.s_budget_code == 3 || s_sth.s_budget_code == 4)
		return(NOERROR);
	else
        	return(ERROR);
}  /*  Automatic Adjustment */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Write record in data base */
RiteRecord(md)
{
        int	action, err;

	err = put_gl(&gl_rec,md,e_mesg);

	if (err == LOCKED) {
		DispError(e_mesg);
       	   	roll_back(e_mesg);
          	return(LOCKED);
	}

	if(err != NOERROR){
#ifdef	ENGLISH
		DispError("ERROR in Saving Records"); 
#else
		DispError("ERREUR en conservant les fiches");
#endif
		roll_back(e_mesg);
		MoveHighs() ;
		return(DBH_ERR);
	}
        if (s_sth.s_fn[0] == ADDREC) action = UPDATE; 
	else if (s_sth.s_fn[0] == CHANGE) action = UPDATE; 
	else if (s_sth.s_fn[0] == DELETE) action = P_DEL;

	err = rite_audit((char)&s_sth, GLMAST,action,(char*)&gl_rec,
				(char*)&pre_rec,e_mesg);
	
	if (err == LOCKED) {
		DispError(e_mesg);
       	   	roll_back(e_mesg);
          	return(LOCKED);
	}

	if(err != NOERROR){
#ifdef	ENGLISH
		DispError("ERROR in Saving Records"); 
#else
		DispError("ERREUR en conservant les fiches");
#endif
		DispError(e_mesg);
		roll_back(e_mesg);
		MoveHighs() ;
		return(err);
	}

	err = commit(e_mesg);
	if (err == LOCKED) {
		DispError(e_mesg);
       	   	roll_back(e_mesg);
          	return(NOERROR);
	}
	if (err < 0) {
#ifdef	ENGLISH
		DispError("ERROR in Saving Records"); 
#else
		DispError("ERREUR en conservant les fiches");
#endif
		DispError(e_mesg);
		MoveHighs() ;
		return(err);
	}

	return(NOERROR);
}	/* RiteRecord() */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Validate flds when PROFOM returns RET_VAL_CHK                         */
Validate(fld_no)	
int	fld_no ;
{
        int row,col ;
        double percent, budget;

        if (fld_no >= 2200) {
        	row = (fld_no - 2200) / 400;
        	col = ((fld_no - 2200) % 400)/100 ;
   
	        if ( col == LASTCOL ) {
                	if (s_sth.s_periods[row].s_new_percent != 0) {
  			   budget=((double)s_sth.s_periods[row].s_new_percent
				    / 100) * (double)s_sth.s_annual_budget;
			   sprintf(e_mesg,"%ld",(long)(budget));
			   sscanf(e_mesg,"%lf",&budget);
		    	   s_sth.s_periods[row].s_new_budget = budget ;
			}
                        else 
                            s_sth.s_periods[row].s_new_budget = 0 ;
                        sr.curfld -= 100;
                }
                else {	
               
			if( (s_sth.s_periods[row].s_new_budget > 99999999) ||
			    (s_sth.s_periods[row].s_new_budget < -99999999) ){

#ifdef	ENGLISH
				fomen("Period budget amount must be between $-99,999,999 and $99,999,999.");
#else
				fomen("Montant du budget de periode doit etre entre $-99,999,999 et $99,999,999.");
#endif
				sr.nextfld = sr.curfld;
			    	s_sth.s_periods[row].s_new_budget=LV_LONG;
				return(ERROR);		
			}

			if (s_sth.s_periods[row].s_new_budget != 0) {
			   percent = ((double)s_sth.s_periods[row].s_new_budget
				       * 100.0) / (double)s_sth.s_annual_budget;
		    	   s_sth.s_periods[row].s_new_percent = percent ;
                         }
                	else
                            s_sth.s_periods[row].s_new_percent = 0.00 ;
                        sr.curfld += 100;
		     }          
    		return(NOERROR);
        }

	switch(fld_no){
        case    ACCNO_FLD   :   	/* Account number */
                if (acnt_chk(s_sth.s_accno) == ERROR ) {
#ifdef ENGLISH
                   fomer( "Invalid Account Number" );
#else
                   fomer( "Numero de compte invalide" );
#endif
                   s_sth.s_accno[0] = LV_CHAR ;
                   return(ERROR);
                }
               break;         
	case	RECCD_FLD  :		/*  Record code */
		/* Record code 97,98 or 99 can be used in budget entry */
		if (s_sth.s_rec_cod != 97 &&
		    s_sth.s_rec_cod != 98 && s_sth.s_rec_cod != 99) {
#ifdef ENGLISH
			fomer("Valid Record Codes are 97,98 and 99");
#else
			fomer("Codes de fiches valables sont 97,98 et 99");
#endif
			s_sth.s_rec_cod = LV_SHORT ;
			return(ERROR);
		}
		break;
        case    BUDGET_FLD  : 		/*  Budget code */
		/* If only 12 periods set up in this system then only
		   budget codes 0,2, or 4 can be used			*/
		if (s_sth.s_budget_code == 1 ||
		    s_sth.s_budget_code == 3) {
			if (pa_rec.pa_no_periods == 12) {
#ifdef	ENGLISH
		                fomer("Valid Codes are 0,2 or 4");
#else
		                fomer("Codes valables sont 0,2 ou 4");
#endif
				s_sth.s_budget_code = LV_SHORT ;
				return(ERROR) ;
			}
		}
                /* Valid Budget Codes are 0 thru 4 */
                if(s_sth.s_budget_code >= 0 && s_sth.s_budget_code <= 4) { 
			if (s_sth.s_budget_code == 1 ||
			    s_sth.s_budget_code == 3) {
				no_periods = 13;
				/* show period 13 information */
				Prev_budget_code = gl_rec.cdbud;
				gl_rec.cdbud = s_sth.s_budget_code;
				CopyToScreen();
			}
			else {
				no_periods = 12;
				/* Blank out last line due to only 12 periods */
       				s_sth.s_periods[NO_PERIODS-1].s_actual = HV_DOUBLE;
         			s_sth.s_periods[NO_PERIODS-1].s_percent = HV_DOUBLE;
         			s_sth.s_periods[NO_PERIODS-1].s_new_budget = HV_LONG;
         			s_sth.s_periods[NO_PERIODS-1].s_new_percent = HV_DOUBLE;

			}
                   	break;
		}
#ifdef	ENGLISH
                fomer("Valid Codes are 0 to 4");
#else
                fomer("Codes valables sont de 0 a 4");
#endif
		s_sth.s_budget_code = LV_SHORT ;
		return(ERROR) ;

        case 	ANNUAL_FLD   :		/*  Annual budget amount */    

		if( (s_sth.s_annual_budget > 99999999) ||
		    (s_sth.s_annual_budget < -99999999) ){
#ifdef	ENGLISH
			fomen("Annual budget amount must be between $-99,999,999 and $99,999,999.");
#else
			fomen("Montant du budget annuel doit etre entre $-99,999,999 et $99,999,999.");
#endif
			s_sth.s_annual_budget = LV_LONG; 
			sr.nextfld = sr.curfld;
			return(ERROR) ;
		}

		break;

/* C.Burns April 15,1991 Allow entry of zero budget 
		if (s_sth.s_annual_budget != 0)
			break;
#ifdef	ENGLISH
                fomer("Annual budget cannot equal 0");
#else
                fomer("Budget annuel doit etre different de 0");
#endif
		s_sth.s_annual_budget = LV_LONG ;
		return(ERROR) ;
*********************/
	default :
#ifdef ENGLISH
		sprintf(e_mesg,"No Validity Check For Field#  %d",sr.curfld);
#else
		sprintf(e_mesg,"Pas de controle de validite pour le Champ#  %d",sr.curfld);
#endif
		fomen(e_mesg);
		get();
	}	/* Switch fld_no */

	return(NOERROR) ;
}	/* Validate() */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Display message and get the option */
GetOpt( msg, option )
       char *msg;
       char *option;
{
        int	j,  i;
	
        STRCPY( s_sth.s_mesg, msg );
        ShowMesg();

        sr.nextfld = END_FLD; 
 
	for ( ; ; ) {
		fomrf((char*)&s_sth);
		ret(err_chk(&sr));	/* Check for PROFOM error */
                j = strlen(option);
		for ( i = 0; i < j; i++ ) 
			if ( s_sth.s_opt[0] == option[i ] )
                           break;
                if(i != j) break ;
#ifdef  ENGLISH
                fomer( " Invalid Option." );
#else
                fomer( "Option invalide." );
#endif
	}
        s_sth.s_mesg[0] = HV_CHAR ;
        s_sth.s_opt[0] = HV_CHAR ;
        sr.nextfld = END_FLD - 100;
        sr.endfld = END_FLD ;
        fomwr( (char *) &s_sth );
        ret( err_chk(&sr) );
 
        return( (int)(option[i])) ;
}  /* GetOpt */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Read the next record from data base */
BringNext(direction)
int	direction;
{
	int err;

	/* Check whether file is in seq. read mode */
	if(flg_start(GLMAST) != direction) {
		/* Set the least part of the key to next possible key and
		  set the file to start */
		gl_rec.funds = s_sth.s_funds ;
		STRCPY(gl_rec.accno, s_sth.s_accno);
		if(direction == FORWARD)
			gl_rec.reccod = s_sth.s_rec_cod + 1 ;
		else
			gl_rec.reccod = s_sth.s_rec_cod - 1 ;

		flg_reset(GLMAST);
	}

	err = get_n_gl(&gl_rec, BROWSE, 0, direction, e_mesg);

#ifndef	ORACLE
	seq_over(GLMAST);
#endif

	if(ERROR == err)return(DBH_ERR) ;
	if(EFL == err) {
#ifdef	ENGLISH
		fomen("No More Records....");
#else
		fomen("Plus de fiches....");
#endif
		get();
		flg_reset(GLMAST);
		return(NOERROR) ;
	}

	CopyToScreen() ;

	return(NOERROR);
}
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Copy the data record fields to Scrren1 & Screen 2 records and display
   the 1st scrren */
CopyToScreen()
{
	int	i;
        double  percent;

/*	if(SW7) 
#ifdef ENGLISH
 		STRCPY(s_sth.s_desc_admis, "  Report Code:");
	else 
 		STRCPY(s_sth.s_desc_admis, "Admissibility:");
#else
 		STRCPY(s_sth.s_desc_admis, "Code rapport:");
	else 
 		STRCPY(s_sth.s_desc_admis, "Admissibilite:");
#endif
*/
	s_sth.s_funds = gl_rec.funds ;
	STRCPY(s_sth.s_accno, gl_rec.accno);
	s_sth.s_rec_cod = gl_rec.reccod ;
/*	s_sth.s_admis = gl_rec.admis ;		*/
	s_sth.s_admis = HV_SHORT;  
        s_sth.s_budget_code = gl_rec.cdbud ;
 	STRCPY(s_sth.s_desc, gl_rec.desc) ;
        STRCPY(s_sth.s_flg, " ");
        s_sth.s_increase = gl_rec.grad;
        s_sth.s_annual_budget = gl_rec.budcur;
	if (s_sth.s_budget_code == 1 || s_sth.s_budget_code == 3)
		no_periods = 13 ;
	else
		no_periods = 12 ;

	for(i = 0 ; i < no_periods ; i++) { 
		s_sth.s_periods[i].s_actual = gl_rec.currel[i] ; 
		s_sth.s_periods[i].s_new_budget = gl_rec.curbud[i] ;
                if(gl_rec.curbud[i] != 0 && gl_rec.budcur != 0) {
			percent = (gl_rec.curbud[i] * 100) / 
                       	   gl_rec.budcur ;  
			s_sth.s_periods[i].s_new_percent = percent ; 
                }
                else
                        s_sth.s_periods[i].s_new_percent = 0.00;
                if(gl_rec.currel[i] != 0 && gl_rec.ytd != 0) {
			percent = (gl_rec.currel[i] * 100) / 
                       	   gl_rec.ytd ;  
			s_sth.s_periods[i].s_percent = percent ; 
                }
                else
                        s_sth.s_periods[i].s_percent = 0.00;
	}
        s_sth.s_budtotal = HV_LONG;
        s_sth.s_pertotal = HV_DOUBLE;
        STRCPY(s_sth.s_dummy, " ");    /* Show column prompt */
	if ((WriteFields(KEY_START, END_FLD-200)) < 0) return(-1);

	return(NOERROR) ; 
}	/* CopyToScreen() */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Copy the Screen fields to data record */
CopyToRecord()
{
	int	i;

        gl_rec.cdbud = s_sth.s_budget_code ;
        gl_rec.grad = s_sth.s_increase ;
	gl_rec.grad = D_Roundoff(gl_rec.grad);
        gl_rec.budcur = s_sth.s_annual_budget ;

        for ( i = 0; i < no_periods; i++ ){ 
            	gl_rec.curbud[i] = s_sth.s_periods[i].s_new_budget;
	}

}	/* CopyToRecord() */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Initialize 1st screen data fields with Low Values */
MoveLows()
{
     int	i;

     STRCPY(s_sth.s_dummy, " ");   
     if(s_sth.s_budget_code == 0 ) { 
     		for ( i = 0; i < no_periods; i++ )
     		{
         		s_sth.s_periods[i].s_new_budget = 0;
         		s_sth.s_periods[i].s_new_percent = 0.00; 
     		}
    		return;
     }

     if(s_sth.s_budget_code == 3 || s_sth.s_budget_code == 4) { 
/****
     		for ( i = 0; i < no_periods; i++ )
     		{
         		s_sth.s_periods[i].s_new_budget = HV_LONG;
			s_sth.s_periods[i].s_new_percent = HV_DOUBLE; 
     		}
****/
    		return;
     }
     if (s_sth.s_flg[0] != YES) 
     	for ( i = 0; i < no_periods; i++ ) {
        	s_sth.s_periods[i].s_new_budget = LV_LONG;
        	s_sth.s_periods[i].s_new_percent = HV_DOUBLE; 
	}
     else
     	for ( i = 0; i < no_periods; i++ ) {
        	s_sth.s_periods[i].s_new_percent = LV_DOUBLE; 
        	s_sth.s_periods[i].s_new_budget = HV_LONG;
	}
}	/* MoveLows() */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Initialize 1st screen data fields with High values and display the screen */
MoveHighs()
{
     int i;

     s_sth.s_desc[0] = HV_CHAR;
     s_sth.s_desc_admis[0] = HV_CHAR;
     s_sth.s_admis = HV_SHORT;
     s_sth.s_budget_code = HV_SHORT;
     s_sth.s_flg[0] =  HV_CHAR;
     s_sth.s_increase = HV_DOUBLE;
     s_sth.s_annual_budget = HV_LONG;
    
     for ( i = 0; i < NO_PERIODS; i++ )
     {
         s_sth.s_periods[i].s_actual = HV_DOUBLE;
         s_sth.s_periods[i].s_percent = HV_DOUBLE;
         s_sth.s_periods[i].s_new_budget = HV_LONG;
         s_sth.s_periods[i].s_new_percent = HV_DOUBLE;
     }

     s_sth.s_budtotal = HV_LONG ;
     s_sth.s_pertotal = HV_DOUBLE ;
     s_sth.s_dummy[0] = HV_CHAR;
	
     no_periods = 13 ;

     if ((WriteFields(ST_FLD, END_FLD-200)) < 0) return(-1);

     return(NOERROR);

}	/* MoveHighs() */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* show ERROR and wait 							 */
static
DispError(s)    
char *s;
{
        STRCPY( s_sth.s_mesg, s) ;
	ShowMesg();
	get();
	s_sth.s_mesg[0] = HV_CHAR;
	ShowMesg();
	return(ERROR);
}
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* show or clears message field 					 */
ShowMesg()  
{
	sr.nextfld = END_FLD - 100;
        fomwf((char*)&s_sth);
}

fund_default()
{
	s_sth.s_funds = 1;
/*
	s_sth.s_rec_cod = 99;
	WriteFields(FUND_FLD,RECCD_FLD);
*/
	WriteFields(FUND_FLD,FUND_FLD);
	s_sth.s_funds = LV_SHORT;
/*
	s_sth.s_rec_cod = LV_SHORT;
*/
}
/*-------------------- E n d   O f   P r o g r a m ---------------------*/

