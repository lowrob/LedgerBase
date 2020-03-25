/******************************************************************************
		Sourcename    : fadept.c
		System        : Budgetary Financial System.
		Subsystem     : Fixed Assets System 
		Module        : FA department file maintenance 
		Created on    : 89-10-9
		Created  By   : K HARISH.
		Cobol sources : 

HISTORY:
 Date           Programmer     Description of modification
____/__/__      __________     ___________________________
1991/01/01	F.Tao		Add codes to protect users from file
				locked system message.
******************************************************************************/
#include <stdio.h>

#define MAIN
#define MAINFL	FADEPT

#include <bfs_defs.h>
#include <bfs_recs.h>
#include <cfomstrc.h>

#define SYSTEM		"FIXED ASSETS"
#define MOD_DATE	"23-JAN-90"
#define SCREEN_NAME	"fadept"
#define ESCAPE		12	/* flag indicates discontinuation of entries */
#define RANDOM  	18
#define SEQUENTIAL	19
#define	DELTA_DIFF	0.001
#define LOW 		-1
#define HIGH 		 1
#define HL_CHAR(VAL)	(VAL==HIGH) ? HV_CHAR : LV_CHAR
#define ESC_F		(sr.retcode==RET_USER_ESC && \
			(sr.escchar[0]=='F'||sr.escchar[0]=='f'))
#define ESC_H		(sr.retcode==RET_USER_ESC && \
			(sr.escchar[0]=='H'||sr.escchar[0]=='h'))
#define NO_HLP_WIN	(sr.curfld!=500)
#define END_FLD		800

#ifdef ENGLISH
#define ADDREC		'A'
#define CHANGE 		'C'
#define DELETE		'D'
#define NEXT		'N'
#define PREV		'P'
#define INQUIRE		'I'
#define EXITOPT		'E'

#define EDIT		'E'
#define CANCEL		'C'
#define YES		'Y'
#define NO		'N'
#else	/* FRENCH */
#define ADDREC 		'R'
#define CHANGE 		'C'
#define DELETE		'E'
#define NEXT		'S'
#define PREV		'P'
#define INQUIRE		'I'
#define EXITOPT		'F'

#define EDIT		'M'
#define CANCEL		'A'
#define YES		'O'
#define NO		'N'
#endif

/* fadept.sth - header for C structure generated by PROFOM EDITOR */

struct	fa_scr	{
	char	s_progname[11];	/* 100 program name */
	long	s_rundt;	/* 300 system date */
	char	s_fn[2];	/* 400 function */
	char	s_code[5];	/* 500 code */
	char	s_desc[31];	/* 600 description */
	char	s_mesg[78];	/* 700 message field */
	char	s_resp[2];	/* 800 response field */
};
struct fa_scr	s_rec;		/* screen record */
struct stat_rec 	sr;		/* profom status record */

Fa_dept		fadept, oldfadept;	/* Fixed Asset types file */
Pa_rec		pa_rec;

static	int retval,err;	/* Global variable to store function values */
char e_mesg[80]; /* to store error messages */

/* Initialize profom fields, call entry procedures */
main( argc, argv )
int argc;
char *argv[];
{
	strncpy( SYS_NAME, SYSTEM, 50 ); /* Module name */
	strncpy( CHNG_DATE, MOD_DATE, 10 );/* Last date of change */
	
	proc_switch( argc,argv,MAINFL );/* process the switches */

	if( Initialize()<0 )	/* Initialize profom enviroment */
		exit(-1);
	if( get_param(&pa_rec,BROWSE,1,e_mesg)<1 ){
		fomen(e_mesg);
		get();
		exit(-1);
	}
	if( pa_rec.pa_fa[0]!=YES ){
#ifdef ENGLISH
		fomen("FA system absent. See Parameter Maintenance");
#else
		fomen("Systeme AI absent. Voir l'entretien des parametres");
#endif
		get();
		exit(0);
	}
	retval = Process();	/* Interact with the user */
	CleanExit();
}

CleanExit()
{
	/* clear and exit the screen , close files & exit program */
	free_audit();	/* free memory allocated for writing audit rec */
	fomcs();
	fomrt();
	close_dbh();
	exit(retval);
}

Initialize()
{
	/* initialize the profom status variables */
	STRCPY( sr.scrnam, NFM_PATH );
	strcat( sr.scrnam, SCREEN_NAME );
	STRCPY( sr.termnm, terminal );

	/* initialize the fields and the profom screen */
	if( FillScrHdg()<0 ) 		return(-1);
	if( FillKeyFields( HIGH )<0 ) 		return(-1);
	if( FillNonKeyFlds( HIGH )<0 ) 		return(-1);
	if( FillMesgRespFlds( HIGH )<0 ) 	return(-1);
	if( InitProfom()<0 ){
		fomcs();
		fomrt();
		return(-1);
	}
	return(0);
}
/* initialize the profom and the screen */
InitProfom()
{
	fomin( &sr );			/* initialize profom */
	ret( err_chk(&sr) );		/* if profom error return */
	if( WriteFields(1,0)<0 )	/* Write all fields once */
		return(-1);
	fomcf(1,1);			/* Enable snap-screen option */
	return(0);
}
/* Fill the screen heading fields: the program name and the date */
FillScrHdg()
{
	STRCPY( s_rec.s_progname, PROG_NAME );
	s_rec.s_rundt = get_date();
	return(0);
}
/* Fill the keyfields with high or low values */
FillKeyFields( value )
short value;
{
	s_rec.s_code[0] =  HL_CHAR( value );
	return(0);
}
/* Fill the non key fields with high/low values */
FillNonKeyFlds( value )
short value;
{
	s_rec.s_desc[0] 	= HL_CHAR(value);
	return(0);
}
/* Fill the message and response fields with high or low values */
FillMesgRespFlds( value )
short value;
{
	s_rec.s_mesg[0] = HL_CHAR(value);
	s_rec.s_resp[0] = HL_CHAR(value);
	return(0);
}
/* Accept user's option and call the corresponding routine in a loop */
Process()
{
	/* Initialize the key fields to zeros. Used if seq. search is made */
	s_rec.s_code[0] = '\0';

	for( ; ; ){
		if( ReadFunction()<0 ) return(-1);
		switch( s_rec.s_fn[0] ){
			case ADDREC:	/* add a record */
				CHKACC(retval,ADD,e_mesg);
				if( AddRecord()<0 ) return(-1);
				break;
			case CHANGE:	/* Change a record */
				CHKACC(retval,UPDATE,e_mesg);
				if( ChangeRecord()<0 )	return(-1);
				break;
			case DELETE:	/* Delete a record */
				CHKACC(retval,P_DEL,e_mesg);
				if( DeleteRecord()<0 )	return(-1);
				break;
			case NEXT:	/* show next record in sequence */
				CHKACC(retval,BROWSE,e_mesg);
				if( Inquiry(SEQUENTIAL,FORWARD)<0 ) return(-1);
				break;
			case PREV:	/* show prev record in sequence */
				CHKACC(retval,BROWSE,e_mesg);
				if( Inquiry(SEQUENTIAL,BACKWARD)<0 ) return(-1);
				break;
			case INQUIRE:	/* show selected record */
				CHKACC(retval,BROWSE,e_mesg);
				if( Inquiry(RANDOM,0)<0 ) return(-1);
				break;
			case EXITOPT:	/* exit */
				return(0);
			default:
				break;
		}
		if( retval<0 ){
			fomen(e_mesg);
			get();
		}
		if( retval==DBH_ERR )
			return(retval);
	}
}
SetDupBuffers( firstfld, lastfld, value )
int	firstfld, lastfld;	/* field numbers range */
int	value;			/* ENABLE or DISABLE */
{
	int i;

	for( i=firstfld; i<=lastfld; i+=100 )
		fomca1( i, 19, value);
	if( value==0 )
		return(0);
	sr.nextfld = firstfld;
	sr.endfld = lastfld;
	fomud( (char *)&s_rec );
	ret( err_chk(&sr) );

	return( 0 );
}
ReadFunction()	/* Display options at the bottom, and read entry */
{
#ifdef ENGLISH
	fomer("A(dd), C(hange), D(elete), N(ext), P(rev), I(nquire), E(xit)");
#else
	fomer("R(ajouter), C(hanger), E(liminer), S(uivant), P(recedent), I(nterroger), F(in)");
#endif
	sr.nextfld = 400;	/* Fn field number */
	fomrf( (char *)&s_rec );
	ret( err_chk(&sr) );
	return(0);
}
/* Add a stock master record */
AddRecord()
{
	if( ClearScreen()<0 )	return(-1);
	if( SetDupBuffers(500,500,0)<0 )
		return(-1);
	if( (retval=RdKeyFlds())<0 )	/* Read key fields */
		return(retval);
	if(retval==ESCAPE){
		if(ClearScreen()<0)	
			return(-1);
		return(retval);
	}
	if( FillNonKeyFlds(LOW)<0 ) 	/* Prepare to read other fields */
		return(-1);

	/* Read the non key fields now */
	if( (retval=ReadFields(600,600))<0 )
		return(retval);
	if(retval==ESCAPE){
		if(ClearScreen()<0)	return(-1);
		return(retval);
	}

	/* Allow the user to edit entries, if required, before saving */
	/* Write the record, in ADD mode, to stock master file */

	if( (retval=EditFlds(ADD))<0 )	return(retval);
	if( retval==ESCAPE ){
		if(ClearScreen()<0)	return(-1);
		return(retval);
	}

	for( ; ; ) {
		err = WriteSession( ADD );
		if(err==NOERROR) break;
		if(err==LOCKED) {
			roll_back(e_mesg);
			if( (retval=EditFlds(LOCKED))<0 )	return(retval);
			if( retval==ESCAPE ){
				if(ClearScreen()<0)	return(-1);
				return(retval);
			}
			continue;
		}
		if(err<0) return(err);
	}
	return(0);
}
/* Read stock master key fields, the fund and stock item code */
RdKeyFlds()
{
	if( FillKeyFields(LOW)<0 ) return(-1);
	return ( ReadFields(500,500) );
}
static
ReadFields(start,end)	/* read the given range of fields */
int start, end;		/* start and end profom field numbers */
{
	int retval;

	sr.nextfld = start;
	sr.endfld = end;
	for( ; ; ){	/* Do in a loop */
		fomrd( (char *)&s_rec );	/* Profom call */
		ret(err_chk(&sr));		/* Check for profom error */
		if( sr.retcode==RET_USER_ESC || sr.retcode==RET_VAL_CHK ){
			if( ESC_F ) return( ESCAPE );
			retval=Validate();
			if(retval<0 || retval==ESCAPE)
				return(retval);
			else
				continue;
		}
		break;
	}
	return(0);
}
static
WriteFields( start,end )	/* write the given range of fields */
int start, end;			/* start & end profom field numbers */
{
	sr.nextfld = start;
	sr.endfld = end;
	fomwr( (char *)&s_rec );
	ret( err_chk(&sr) );
	return(0);
}
ClearScreen()	/* clear the screen except fn field key flds & screen heading */
{
	if(FillNonKeyFlds(HIGH)<0 ) return(-1);
	if(FillMesgRespFlds(HIGH)<0 ) return(-1);

	if( WriteFields(600,800)<0 ) return(-1);
	
	return(0);
}
Validate()	/* Validate the values entered by the user */
{
	int index;

	switch( sr.curfld ){
		case 500:	/* Code */
			if( ESC_H ){
				retval = fadept_hlp( s_rec.s_code, 7,13 );
				if( retval==DBH_ERR )
					return( retval );
				else if( retval>=0 )
					redraw();
				if( retval==0 )
					break;
			}
			STRCPY( fadept.code,s_rec.s_code );
			index = get_fadept( &fadept, BROWSE, 0, e_mesg );
			if( index==ERROR ){
				fomen(e_mesg); get();
				return(-1);
			}
			if( s_rec.s_fn[0]==ADDREC ){
				if( index!=UNDEF ){
#ifdef ENGLISH
					fomer("Record already exists");
#else
					fomer("Fiche existe deja");
#endif
					s_rec.s_code[0] = LV_CHAR;
				}
			}
			else{
			    if ( index!=NOERROR ){
				fomer(e_mesg);
				s_rec.s_code[0] = LV_CHAR;
			    }
			}
			break;
		default:
			break;
	}
	sr.nextfld = sr.curfld;
	return(0);
}
EditFlds(mode)	/* Ask if user wants to edit fields before saving */
int	mode;
{
	if( s_rec.s_fn[0]==CHANGE && mode != LOCKED) /* if locked error */
		if( FldEdit()<0 ) 		     /*	don't do */
			return(-1);
	for( ; ; ){
#ifdef ENGLISH
		if(DisplayMessage( "Y(es), E(dit), C(ancel)")<0 ) 
#else
		if(DisplayMessage( "O(ui), M(odifier), A(nnuler)")<0 )
#endif
			return(-1);
		sr.nextfld = 800;
		fomrf( (char *)&s_rec );
		ret( err_chk(&sr) );
		switch( s_rec.s_resp[0] ){
			case EDIT:	/* Edit the fields */
				if( (retval=FldEdit())<0 )
					return(retval);
				break;
			case CANCEL:	/* Cancel the session */
#ifdef ENGLISH
				if( DisplayMessage("Confirm the Cancel (Y/N)?")<0 )
#else
				if( DisplayMessage("Confirmer l'annulation (O/N)?")<0 )
#endif
					return(-1);
				sr.nextfld = 800;	/* response field */
				fomrf( (char *)&s_rec );
				ret( err_chk(&sr) );
				if( s_rec.s_resp[0]==YES ){
					if( HideMessage()<0 ) return(-1);
					return(ESCAPE);
				}
				break;
			case YES:
				if( HideMessage()<0 ) return(-1);
				return(0);
		}
	}
}
DisplayMessage(mesg)	/* Display the given message in the message field */
char *mesg;
{
	STRCPY( s_rec.s_mesg, mesg );
	if( WriteFields(700,700)<0 )	return(-1);
	return(0);
}
HideMessage()	/* Hide the message & response fields */
{
	if( FillMesgRespFlds(HIGH)<0 )	return(-1);
	if( WriteFields(700,800)<0 ) return(-1);
	return(0);
}
FldEdit()	/* Read the field number and read corresponding field */
{
	return( ModifyField(600,600) );
}
ModifyField( firstfld, lastfld )	/* Read & change the specified fields */
int firstfld,lastfld;
{
	int i;

#ifdef	ENGLISH
		STRCPY(s_rec.s_mesg,"Enter RETURN to Terminate Edit");
#else
		STRCPY(s_rec.s_mesg,"Appuyer sur RETURN pour terminer l'ajustement");
#endif
	ShowMesg();

	for( i=firstfld; i<=lastfld; i+=100 ){
		fomca1( i,19,2);	/* enable dup buffers */
		fomca1( i,10,0);	/* disable escape flag */
	}
	sr.nextfld = firstfld;
	sr.endfld = lastfld;
	fomud( (char *)&s_rec);		/* Update dup buffers */
	switch(firstfld){
		case 600:	/* description */
			s_rec.s_desc[0] = LV_CHAR;
			break;
		default:
			break;
	}
	retval = ReadFields( firstfld, lastfld );
	if( retval<0 || retval==ESCAPE )	return(retval);
	for( i=firstfld; i<=lastfld; i+=100 ){
		fomca1( i,19,0);	/* disable dup buffers */
		fomca1( i,10,1);	/* enable escape flag */
	}
	return(0);
}
WriteSession(mode)	/* Write the stock master record */
int	mode;		/* ADD, UPDATE, P_DEL */
{
	STRCPY( fadept.desc, s_rec.s_desc );
	retval = put_fadept( &fadept, mode, e_mesg );
	if( retval!=NOERROR ){
		fomen(e_mesg);get();
		roll_back(e_mesg);
		return(retval);
	}
	err = rite_audit((char*)&s_rec, FADEPT,mode,(char *)&fadept,
				(char *)&oldfadept,e_mesg);
	if(err==LOCKED) {
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
		MoveHighs() ;
		return(DBH_ERR);
	}
	if(commit(e_mesg) < 0) {
#ifdef ENGLISH
		DispError("ERROR in Saving Records"); 
#else
		DispError("ERREUR en conservant les fiches");
#endif
		DispError(e_mesg);
		MoveHighs() ;
		return(DBH_ERR);
	}
	return(0);
}
Inquiry( access, direction )	/* Screen inquiry, random/sequential */
int access, direction;		/* RANDOM, SEQUENTIAL accesses */
{
	if( access==RANDOM ){
		if( SetDupBuffers(500,500,2)<0 )
			return(-1);
		retval = GetRecord();	/* Read the key values */
		if( retval==UNDEF || retval==ESCAPE || retval==LOCKED )
			return( retval=ESCAPE );
	}
	else{	/* access is sequential, so get next record from file */
		retval = GetNextRec( direction );	/* Get next rec */
		if(retval==EFL)
			return(retval=0);
	}
	if( retval<0 ){	/* errors in reading */
		fomen(e_mesg);get();
		return(-1);
	}
	retval = DisplayRecord();	/* Display record */
	if( retval<0 ) return(retval);

	return(0);
}
DisplayRecord()	/* Get and display records */
{
	STRCPY( s_rec.s_desc, fadept.desc );

	if( WriteFields(500,600) < 0 )
		return(-1);
	return(0);
}
GetNextRec(direction)	/* Read the next record in the specified direction */
int	direction;
{
	if( flg_start(FADEPT)!=direction ){ 	/* file access mode changed */
		STRCPY( fadept.code, s_rec.s_code );
		inc_str( fadept.code, (sizeof(s_rec.s_code)-1), direction );
		flg_reset(FADEPT);
	}
	/* Read the next record from stmast file */
	retval = get_n_fadept( &fadept, BROWSE, 0, direction, e_mesg );
#ifndef ORACLE
	seq_over( FADEPT );
#endif
	if( retval==EFL ){
#ifdef ENGLISH
		fomen("No More Records....");
#else
		fomen("Plus de fiches....");
#endif
		get();
		flg_reset(FADEPT);
		return(EFL);
	}
	if( retval!=NOERROR ){
		fomen(e_mesg);	get();
		return(retval);
	}
	/* Write the key fields on to the screen */
	STRCPY( s_rec.s_code, fadept.code );
	if( WriteFields(500,500)<0 )
		return(-1);
	return(0);
}
GetRecord()	/* Read the header key values, get terminal info ,read rec */
{
	int	mode;

	/* Enable and update dup buffers for key */
	fomca1( 500, 19, 2 );
	sr.nextfld = sr.endfld = 500;
	fomud( (char *)&s_rec );

	if( s_rec.s_fn[0]==CHANGE || s_rec.s_fn[0]==DELETE )
		mode = UPDATE;
	else
		mode = BROWSE;
	s_rec.s_code[0] = LV_CHAR;
	if( (retval=ReadFields(500,500))<0 || retval==ESCAPE ) 
		return( retval );
	STRCPY( fadept.code, s_rec.s_code );
	retval = get_fadept( &fadept, mode, 0, e_mesg );
	if( retval!=NOERROR ){
		fomer(e_mesg);
		get();
		return(retval);
	}

	/* Disable dup buffers for key */
	fomca1( 500, 19, 0 );

	return(0);
}
ChangeRecord()	/* Change an existing record */
{
	if( SetDupBuffers(500,500,2)<0 )
		return(-1);
	/* Get the record & display it */
	retval = Inquiry( RANDOM, 0 );
	if( retval<0 )	return(retval);
	if( retval==ESCAPE )  return(0);

	/* Copy the record to another buffer, for writing audit */
	scpy( (char *)&oldfadept, (char *)&fadept, sizeof( Fa_dept ) );

	/* Allow changes on the record fields */
	retval = EditFlds(UPDATE);
	if( retval<0 )	return(retval);
	if( retval==ESCAPE ){	/* Cancellation of changes */
		roll_back(e_mesg);	/* release locked record */
		return(0);
	}

	for( ; ; ) {
		err= WriteSession( UPDATE );

		if(err==NOERROR) break;
		if(err==LOCKED) {
			roll_back(e_mesg);
			STRCPY( fadept.code, s_rec.s_code );
			retval = get_fadept( &fadept, UPDATE, 0, e_mesg );
			if(retval < 0){
				fomen(e_mesg);
				get() ;
				if(retval== UNDEF || retval== LOCKED) continue;
				return(DBH_ERR);
			}
			retval = EditFlds(LOCKED);
			if( retval<0 )	return(retval);
			if( retval==ESCAPE ){	/* Cancellation of changes */
				roll_back(e_mesg);  /* release locked record */
				return(0);
			}
			continue;
		}
		if(err<0) return(err);
	}
	return(0);
}
DeleteRecord()	/* delete an existing record */
{
	if( SetDupBuffers(500,500,2)<0 )
		return(-1);
	/* Get the record & display it */
	retval = Inquiry( RANDOM, 0 );
	if( retval<0 )	return(retval);
	if( retval==ESCAPE )
		return(0);
	for( ; ;){
		for( ; ; ){
#ifdef ENGLISH
			if( DisplayMessage("Confirm (Y/N)?")<0 ) return(-1);
#else	
			if( DisplayMessage("Confirmer (O/N)?")<0 ) return(-1);
#endif
			s_rec.s_resp[0] = LV_CHAR;
			if( ReadFields(800,800)<0 ) return(-1);
			if( s_rec.s_resp[0]!=YES && s_rec.s_resp[0]!=NO )
				continue;
			else
				break;
		}
		if( s_rec.s_resp[0]==NO ){ /* Cancellation of deletion */
			roll_back(e_mesg);	/* release locked record */
			return( HideMessage() );
		}

	/* Write the updated record */

		err= WriteSession( P_DEL );

		if(err==NOERROR) break;
		if(err==LOCKED) {
			roll_back(e_mesg);
			STRCPY( fadept.code, s_rec.s_code );
			retval = get_fadept( &fadept, UPDATE, 0, e_mesg );
			if(retval < 0){
				fomen(e_mesg);
				get() ;
				if(retval== UNDEF || retval== LOCKED) continue;
				return(DBH_ERR);
			}
			continue;
		}
	}
	return( ClearScreen() );
}
/*----------------------------------------------------------------------*/
/* Initialize screen data fields with High values and display the screen*/
/*----------------------------------------------------------------------*/
static
MoveHighs()
{
	if( FillKeyFields( HIGH )<0 ) 		return(-1);
	if( FillNonKeyFlds( HIGH )<0 ) 		return(-1);
	return(NOERROR);
}	/* MoveHighs() */
/*----------------------------------------------------------------------*/
/* show ERROR and wait 							*/
/*----------------------------------------------------------------------*/
static
DispError(s)    /* show ERROR and wait */
char *s;
{
	STRCPY(s_rec.s_mesg,s);
	ShowMesg();
#ifdef	ENGLISH
	fomen("Press any key to continue");
#else
	fomen("Appuyer sur une touche pour continuer");
#endif
	get();
	s_rec.s_mesg[0] = HV_CHAR;
	ShowMesg();
	return(ERROR);
}
/*----------------------------------------------------------------------*/
/* shows or clears message field 					*/
/*----------------------------------------------------------------------*/
static
ShowMesg()  /* shows or clears message field */
{
	sr.nextfld = END_FLD - 100;
	fomwf((char*)&s_rec) ;
}
/*-------------------- E n d   O f   P r o g r a m ---------------------*/
