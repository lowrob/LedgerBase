/*-----------------------------------------------------------------------
Source Name: download.c
System     : Budgetary Financial system.
Created  On: 21 April 92.
Created  By: Chris Leadbeater.

DESCRIPTION:


MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~

------------------------------------------------------------------------*/
#define  MAIN
#define  MAINFL		SUPPLIER		/* main file used */

#include <stdio.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define SYSTEM 		"PURCHASE ORDER"
#define	MOD_DATE	"21-APR-92"		/* Program Last Modified */

#ifdef ENGLISH

#define	EDIT		'E'
#define	CANCEL		'C'
#define	YES		'Y'
#define NO		'N'
#else

#define	EDIT		'M'
#define	CANCEL		'A'
#define	YES		'O'
#define NO		'N'
#endif

#define EXIT		12

#define	SCR_NAME	"download"	/* First screen */

/* PROFOM Field Numbers */
#define	LAST_SNO	2
#define	CHNG_LIMIT	2	/* change mode can't change beyond this fld */

/* Screen Control Variables */
#define	CHG_FLD		400
#define	ST_FLD	        600		/* Data entry starting field */
#define	READ_FLD	600		/* Data entry starting field */
#define	END_FLD  	700		/* screen end field */
#define MESG_FLD	900		/* Mesg field */

#define	UNIXFILE_FLD	600		/* Unix Filename (SOURCE) */
#define	DOSFILE_FLD	700		/* DOS Filename (DESTINATION) */
	
static	struct  stat_rec sr;		/* PROFOM status rec */

static	char 	e_mesg[80];  		/* dbh will return err msg in this */

typedef struct {
	char	s_pgm[11];		/* 100 String 10 */
	long	s_rundate;		/* 300 Date 9999F99F99 */
	short	s_field;		/* 400 Field no */
	char 	s_unixfile[21];		/* 600 Source File */
	char	s_dosfile[21];		/* 700 Destination File */
	char	s_mesg[78];		/* 800 message line */
	char	s_opt[2];		/* 900 option entry */
	} s_struct;

static	s_struct	s_sth ; 

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

	if ( Process() < 0) { 		/* Initiate Process */
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

	STRCPY(s_sth.s_pgm,PROG_NAME);
	s_sth.s_rundate = get_date();	/* get Today's Date in YYMMDD format */
	s_sth.s_field = HV_SHORT ;
	s_sth.s_mesg[0] = HV_CHAR ;
	s_sth.s_opt[0] = HV_CHAR ;

	/* Move High Values to 1st screen data fields & Display */
	MoveHighs() ;

	return(NOERROR);

}	/* InitScreens() */

/*-------------------------------------------------------------------*/

Process()
{
	int retval;

	retval = GetFiles();
	if(retval < 0) return(retval);

	retval = TransferFile();
	if(retval < 0) return(retval);

	return(NOERROR);

}	/* Process() */

/*-------------------------------------------------------------------*/

GetFiles()
{
	int	i, retval;

     	for ( i = ST_FLD; i <= END_FLD ; i += 100 ) {
       		fomca1( i,19,0 );	        /*  disable Dup Control */
       		fomca1( i,10,1 );      	       	/*  enable user escape */
     	}

     	sr.nextfld = ST_FLD;
     	sr.endfld = END_FLD;
     	fomud( (char *) &s_sth );
     	ret(err_chk(&sr));

	MoveLows();

     	retval = ReadFields(ST_FLD,END_FLD);
	if(retval == PROFOM_ERR) return(retval) ;
	if(retval == EXIT) {
		roll_back(e_mesg);
       		return( MoveHighs() ) ;
	}

	for( ; ; ) {
     		retval = ReadOption() ;
     		if (retval < 0) return(-1);
  
  	   	if ( retval == CANCEL ) {
       			MoveHighs();
			return(CANCEL);
		}

		if( retval == YES) break;

		if(retval<0) return(retval);
	}

	return(NOERROR);

}   

/* include file for anzio to transfer files to PC */

#include "send-pc.c"

/*-----------------------------------------------------------------------*/

TransferFile()
{

	int	retval;

	retval = Send_PC(s_sth.s_unixfile, s_sth.s_dosfile, e_mesg);
	if(retval < 0) return(retval);

	return(NOERROR);
}




/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Changing fields. Accept fld to be changed and read that fld 		 */
ChangeFields()
{
     	int i,retval,fld_no;

     	for ( i = ST_FLD; i <= END_FLD ; i += 100 ) {
       		fomca1( i,19,2 );      		/*  enable Dup Control */
       		fomca1( i,10,1 );      		/*  enable user escape */
     	}

     	sr.nextfld = ST_FLD;
     	sr.endfld = END_FLD;
     	fomud( (char *) &s_sth );
     	ret(err_chk(&sr));

	/* Get The Field to Be Modified */
#ifdef	ENGLISH
	STRCPY(s_sth.s_mesg,"Enter RETURN to terminate Edit");
#else
	STRCPY(s_sth.s_mesg,"Appuyer sur RETURN pour terminer l'ajustement");
#endif
	ShowMesg() ;
        
     	for (; ;) {
		s_sth.s_field = LV_SHORT;
		retval = ReadFields(CHG_FLD, CHG_FLD);

		if (retval < 0) return(-1);
		if (retval == EXIT) break;		/* User enter ESC-F */

       		if (s_sth.s_field == 0 ) break;	/* Finished changing fields */

		if(s_sth.s_field > LAST_SNO) continue ;

        	fld_no = ST_FLD + (100 * (s_sth.s_field - 1)) ;

		sr.nextfld = fld_no;
		for(; ;) {
			fomrf((char*)&s_sth);
			ret(err_chk(&sr));

			if(sr.retcode == RET_VAL_CHK){
				if (Validate() == NOERROR) break; 
				sr.nextfld = sr.curfld;
				continue;
			}

			if(sr.retcode == RET_USER_ESC){
			   if(sr.escchar[0] == 'F' || sr.escchar[0] == 'f')  
					break;
			   if(sr.escchar[0] == 'h' || sr.escchar[0] == 'H')
					if( WindowHelp() < 0 ) return(-1) ;
			   continue;
			}
			break;
		}
	}

     	s_sth.s_field = HV_SHORT ;
	if ( WriteFields(CHG_FLD, CHG_FLD) < 0 ) return(-1);

     	s_sth.s_mesg[0] = HV_CHAR ;
     	ShowMesg();

	if (retval == EXIT) 
		return(EXIT);
	else
     		return(NOERROR);

}	/* ChangeFields() */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Reads the fields in the range given in the parameters.                */
static
ReadFields(begin,end)
int 	begin, end;
{
	int	err;

	sr.nextfld = begin;
	sr.endfld = end;

	for(; ;) {
		fomrd((char*)&s_sth);
		ret(err_chk(&sr));

		if(sr.retcode == RET_VAL_CHK){
			err = Validate(sr.curfld) ;
			if (err == DBH_ERR) return(-1); 
			sr.nextfld = sr.curfld;
			continue;
		}

		if(sr.retcode == RET_USER_ESC){
			if(sr.escchar[0] == 'F' || sr.escchar[0] == 'f') { 
				return(EXIT);
			}
			if(sr.escchar[0] == 'h' || sr.escchar[0] == 'H'){
				if( WindowHelp() < 0 ) return(-1) ;
			}
			continue;
		}
		break;
	}			/* end of for loop */
	return(NOERROR) ;

} 	/* end of ReadFields */

/*-----------------------------------------------------------------------*/

Validate()
{

	return(NOERROR);
}

/*-----------------------------------------------------------------------*/
WindowHelp()
{

	return(NOERROR);
}

/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Writes the fields in the range given in the parameters.               */

static
WriteFields(begin,end)
int 	begin, end;
{
	sr.nextfld = begin;
	sr.endfld = end;

	fomwr((char*)&s_sth);
	ret(err_chk(&sr));
	return(NOERROR);

}	/* end of WriteFields */
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
	*/

	for( ; ; ) {
#ifdef ENGLISH
	    err = GetOpt( "Y(es), E(dit), C(ancel)", "YEC");
#else
	    err = GetOpt( "O(ui), M(odifier), A(nnuler)", "OMA");
#endif

	    if(err == PROFOM_ERR) return(err) ;

	    switch(err) {
	    case  YES :
		return(YES) ;
	    case  EDIT  :
		err = ChangeFields();
		break ;
	    case  CANCEL :
#ifdef	ENGLISH
                err = GetOpt("Confirm the Cancel (Y/N)?", "YN") ;
#else
                err = GetOpt("Confirmer l'annulation (O/N)?", "ON") ;
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
/* Display message and get the option */
GetOpt( msg, option )
       char *msg;
       char *option;
{
        int	j,  i;
	
        STRCPY( s_sth.s_mesg, msg );
        ShowMesg();

	for ( ; ; ) {
		s_sth.s_opt[0] = LV_CHAR ;
		if ( ReadFields(MESG_FLD, MESG_FLD) < 0 ) return(-1);
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
	if ( WriteFields(MESG_FLD -100, MESG_FLD) < 0 ) return(-1);
 
        return( (int)(option[i])) ;
}  /* GetOpt */

/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Initialize 1st screen data fields with Low Values */
MoveLows()
{

	s_sth.s_unixfile[0] = LV_CHAR ;
	s_sth.s_dosfile[0] = LV_CHAR ;

	return(NOERROR);


}	/* MoveLows() */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Initialize 1st screen data fields with High values and display the screen */
MoveHighs()
{

	s_sth.s_unixfile[0] = HV_CHAR ;
	s_sth.s_dosfile[0] = HV_CHAR ;

	if ( WriteFields(ST_FLD, END_FLD) < 0 ) return(-1);

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
	sr.nextfld = MESG_FLD - 100;
        fomwf((char*)&s_sth);
}
/*-------------------- E n d   O f   P r o g r a m ---------------------*/

