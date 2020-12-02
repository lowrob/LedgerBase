/******************************************************************************
		Sourcename    : userprof.c
		System        : Budgetary Financial System.
		Subsystem     : Utilities ( Security )
		Module        : User Profile maintenance 
		Created on    : 90-01-24
		Created  By   : K HARISH.
******************************************************************************
About the Program:
		This program is used for maintenance of UserProfile Records.
	Each user of the system (ledgerbase) has a user profile record, whose
	contents determine the accessibility of various files for that user.

	Each record includes the following.
		1. Login name of the user (Key for the record).
		2. User's name for identification.
		3. User's terminal name 
		4. Class of the user ( Administrator/User )
		5. For each file in the database,
			flags that indicate if the user can
			BROWSE, ADD, UPDATE, DELETE records in the file.

HISTORY:
 Date           Programmer     Description of modification
____/__/__      __________     ___________________________
1991/01/01	C.Leadbeater   Implemented record locking for audit file.
2020/07/20	L.Robichaud	New compiler needs to have function prototypes and declares
******************************************************************************/
#include <stdio.h>

#define MAIN
#define MAINFL	USERPROF

#include <bfs_defs.h>
#include <bfs_recs.h>
#include <cfomstrc.h>

#define SYSTEM		"UTILITIES"
#define MOD_DATE	"01-JAN-91"
#define SCREEN_NAME	"userprof"
#define ESCAPE		12	/* flag indicates discontinuation of entries */
#define ALLOC_ERROR	13
#define RANDOM  	18
#define SEQUENTIAL	19
#define	ITEMSPERPAGE	12
#define LOW 		-1
#define HIGH 		 1
#define HL_CHAR(VAL)	(VAL==HIGH) ? HV_CHAR : LV_CHAR
#define ESC_F		(sr.retcode==RET_USER_ESC && \
			(sr.escchar[0]=='F'||sr.escchar[0]=='f'))

#ifdef ENGLISH
#define ADDREC		'A'
#define CHANGE		'C'
#define DELETE		'D'
#define INQUIRE		'I'
#define NEXT		'N'
#define PREV		'P'
#define EXITOPT		'E'

#define ADDITEMS	'I'
#define EDIT		'E'
#define CANCEL		'C'
#define YES		'Y'
#define NO		'N'
#else
#define ADDREC		'R'
#define CHANGE		'C'
#define DELETE		'E'
#define INQUIRE		'I'
#define NEXT		'S'
#define PREV		'P'
#define EXITOPT		'F'

#define ADDITEMS	'C'
#define EDIT		'M'
#define CANCEL		'A'
#define YES		'O'
#define NO		'N'
#endif

typedef struct{
	char	s_usrflnm[21];	/* User file name */
	int	s_delete;	/* DELETE access allowed ? YES/NO B */
	int	s_update;	/* UPDATE access allowed ? YES/NO B */
	int	s_add;		/* ADD    access allowed ? YES/NO B */
	int	s_browse;	/* BROWSE access allowed ? YES/NO B */
}	
	Flacc;	/* Four types of access indicators for given file */

/* usrprof.sth - header for C structure generated by PROFOM EDITOR */
struct	u_struct	{
	char	s_progname[11];	/* 100 program name */
	long	s_rundt;	/* 300 system date */
	char	s_fn[2];	/* 400 function option */
	char	s_userid[12];	/* 500 login name of user */
	short	s_fld_no;	/* 600 file# field */
	char	s_username[21];	/* 800 user's name for identification */
	char	s_terminal[4];	/* 900 user's login terminal# */
	char	s_class[2];	/* 1000 user's class (Administrator/User) */
	char	s_seperator[2];	/* 1100 seperator */
	char	s_hdgmask[3];	/* 1700 Heading mask */
	Flacc	s_entries[ITEMSPERPAGE];/* array file acc modes 2200 to 9300 */
	char	s_mesg[78];	/* 9400 message field */
	char	s_resp[2];	/* 9500 response field */
};

typedef struct{		/* structure to record current page & line of entry */
	short	page;	
	short	line;
}	Counter;

/* link list node for holding one page of entries */
typedef	struct pgofitems{
	struct pgofitems *prevptr;	/* pointer to previous entry */
	int	lines_entered;		/* no of lines entered */
	Flacc fields[ITEMSPERPAGE];	/* array of lines per page */
	struct pgofitems *nextptr;	/* pointer to next entry */
}	Page;

static Page	*headptr,*tempptr,*tailptr;	/* to maintain list */
static Counter	current; /* For current line & page of entry */

struct u_struct	s_rec;		/* screen record */
struct stat_rec sr;		/* profom status record */

static int retval;	/* Global variable to store function values */
static char e_mesg[80]; /* to store error messages */
static	UP_rec		up_rec, oup_rec;	/* user profile record */
struct stat_rec 	sr;		/* profom status record */

static short lastitemnumber;	/* keeps track of the last item number */
static int totalitemsactive;	/* total items added in any add session */

/* Declare functions and prototypes */
static int Initialize (char *);
static int InitProfom(char *);
static int InitScreen(void);
static int FillScrHdg(void);
static int FillKeyFields(short);
static int FillSeperatorLine(short);
static int FillFieldNum(short);
static int FillHeaderFields(short);
static int FillMsgRespFields(short);
static int FillLineHeading(int);
static int FillItemLines(int, short, short);
static int Process(void);
static int ReadFunction(void);
static int AddRecords(void);
static int ReadKeyFields(void);
static int ReadFields(int, int);
static int WriteFields(int,int);
static int ClearScreen(void);
static int Validate(void);
static int ConfirmHeader(v0id);
static int DisplayMessage(char *);
static int HideMessage(void);
static int HideFldNo(void);
static int EditHeaderFields(void);
static Page *PageAllocated(void);
static int ShowPage(void);
static int ListToScreen(void);
static int ConfirmItems(int);
static int EditItemFields(void);
static int LineChange(int);
static int WriteSession(int);
static int WriteRecord(int);
static int FreeList(void);
static int Inquire(int, int, int);
static int DisplayRecord(int);
static int GetNextRec(int);
static int GetRecord(int);
static int ShowHeaderFields(void);
static int ShowItems(int);
static int BuildList(int);



/* Initialize profom fields, call entry procedures */
main( argc, argv )
int argc;
char *argv[];
{
	strncpy( SYS_NAME, SYSTEM, 50 ); 	/* Module name */
	strncpy( CHNG_DATE, MOD_DATE, 10 );	/* Last date of change */
	proc_switch( argc,argv,MAINFL ); 	/* process the switches */

	if( Initialize(terminal) <0 )	/* initialize profom and screen */
		exit(0);

	/* Is this fellow previliged to run this program ? */
	retval = GetUserClass(e_mesg);
	if( retval==DBH_ERR ){
		fomen(e_mesg);
		get();
		exit(-1);
	}
	else if ( retval != ADMINISTRATOR && retval != SUPERUSER ){
#ifdef ENGLISH
		fomer( "ACCESS DENIED. Press any Key");
#else
		fomen( "ACCESS NIE. Appuyer sur une touche");
#endif
		get();
		exit(-1);
	}

	retval = Process();

	fomcs();	/* Clean up the mess and quit */
	fomrt();
	close_dbh();
	free_audit();
	exit( retval );
}
/* initialize profom and screen */
static int
Initialize( terminal )
char *terminal;
{
	if( InitProfom(terminal)<0 ){	/* initialize profom */
		fomcs();
		fomrt();
		return( -1 );
	}
	if( InitScreen()<0 ){		/* initialize screen */
		fomcs();
		fomrt();
		return(-1);
	}
	fomcf(1,1);			/* Enable Snap screen option */

	return(0);
}	
static
InitProfom(terminal)
char *terminal;
{
	strcpy( sr.termnm, terminal );
	fomin( &sr );
	ret( err_chk(&sr) );	/* if profom error return */
	return(0);
}
static
InitScreen()
{
	/* initialize the profom screen variables */
	strcpy( sr.scrnam, NFM_PATH );
	strcat( sr.scrnam, SCREEN_NAME );

	/* initialize the fields of the profom screen */
	if( FillScrHdg()<0 ) 				return(-1);

	if( FillKeyFields( LOW )<0 ) 			return(-1);

	if( FillFieldNum( HIGH )<0 ) 			return(-1);
	if( FillHeaderFields( HIGH )<0 ) 		return(-1);

	if( FillSeperatorLine(HIGH)<0 ) 		return(-1);
	if( FillLineHeading( HIGH )<0 ) 		return(-1);
	if( FillItemLines( HIGH,1,ITEMSPERPAGE )<0 )	return(-1);

	if( FillMsgRespFields( HIGH )<0 ) 		return(-1);

	if( WriteFields(1,0)<0 )			return(-1);

	return(0);
}
/* Fill the screen heading fields, the program name and the date */
static
FillScrHdg()
{
	strcpy( s_rec.s_progname, PROG_NAME );
	s_rec.s_rundt = get_date();
	return(0);
}
/* Fill the keyfields with high or low values */
static
FillKeyFields( value )
short value;
{
	s_rec.s_userid[0] = HL_CHAR(value);
	return(0);
}
/* Fill the demarkating line field with high/low values */
static
FillSeperatorLine( value )
short value;
{
	s_rec.s_seperator[0] = HL_CHAR(value);
	return(0);
}
/* Fill the file# field with high/low values */
static
FillFieldNum( value )
short value;
{
	s_rec.s_fld_no = value * HV_SHORT;
	return(0);
}
/* Fill the header part with high/low values */
static
FillHeaderFields( value )
short value;
{
	s_rec.s_username[0]	= HL_CHAR( value );
	s_rec.s_terminal[0]	= HL_CHAR( value );
	s_rec.s_class[0]	= HL_CHAR( value );
	return(0);
}
/* Fill the message and response fields with high or low values */
static
FillMsgRespFields( value )
short value;
{
	s_rec.s_mesg[0] = HL_CHAR( value );
	s_rec.s_resp[0] = HL_CHAR( value );
	return(0);
}
/* Fill the item heading line with high/low values */
static
FillLineHeading(val)
int val;
{
	if( val==HIGH )
		s_rec.s_hdgmask[0] = HV_CHAR;
	else
		s_rec.s_hdgmask[0] = ' ';
	return(0);
}
/* Fill the items array with high or low values */
static
FillItemLines( value,firstline,lastline )
int value;
short firstline, lastline;
{
	short i;

	for( i=firstline-1; i<lastline; i++ ){
		s_rec.s_entries[i].s_usrflnm[0] = HL_CHAR(value);
		s_rec.s_entries[i].s_delete 	= value * HV_INT;
		s_rec.s_entries[i].s_update 	= value * HV_INT;
		s_rec.s_entries[i].s_add 	= value * HV_INT;
		s_rec.s_entries[i].s_browse 	= value * HV_INT;
	}
	return(0);
}
/* Accept user's option and call the corresponding routine in a loop */
static
Process()
{
	FillKeyFields( LOW );
	/*	Read and Process user's option in a loop */
	for( ; ; ){
		if( ReadFunction()<0 ) 
			return(-1);
		switch( s_rec.s_fn[0] ){
			case ADDREC:	/* add a profile */
				CHKACC(retval,ADD,e_mesg);
				if( AddRecords()<0 ) return(-1);
				break;
			case CHANGE:	/* change a profile */
				CHKACC(retval,UPDATE,e_mesg);
				if( Inquire(RANDOM,0,UPDATE)<0 ) 
					return(-1);
				break;
			case DELETE:	/* delete a profile */
				CHKACC(retval,P_DEL,e_mesg);
				if( Inquire(RANDOM,0,P_DEL)<0 ) 
					return(-1);
				break;
			case NEXT:	/* show next profile in sequence */
				CHKACC(retval,BROWSE,e_mesg);
				if( Inquire(SEQUENTIAL,FORWARD,BROWSE)<0 ) 
					return(-1);
				break;
			case PREV:	/* show prev profile in sequence */
				CHKACC(retval,BROWSE,e_mesg);
				if( Inquire(SEQUENTIAL,BACKWARD,BROWSE)<0 ) 
					return(-1);
				break;
			case INQUIRE:	/* show selected profile */
				CHKACC(retval,BROWSE,e_mesg);
				if( Inquire(RANDOM,0,BROWSE)<0 ) 
					return(-1);
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
/* Display options at the bottom and read entry */
static
ReadFunction()
{
#ifdef ENGLISH
	fomer("A(dd), C(hange), D(elete), I(nquire), N(ext), P(rev), E(xit)");
#else
	fomer("R(ajouter), C(hanger), E(liminer), I(nterroger), S(uiv), P(rec), F(in)");
#endif
	sr.nextfld = 400;	/* Fn field number */
	fomrf( (char *)&s_rec );
	ret( err_chk(&sr) );
	return(0);
}
/* Add a user profile record */
static
AddRecords()
{

	/* accept key of the new record */
	if( ClearScreen()<0 )	return(-1);
	if( (retval=ReadKeyFields())<0 || retval==ESCAPE){
		if(ClearScreen()<0)	return(-1);
		return(retval);
	}

	/* accept header part of the record */
	if( FillHeaderFields(LOW)<0 ) return(-1);
	if( (retval=ReadFields(800,1000))<0 || retval==ESCAPE ){
		if(ClearScreen()<0)	return(-1);
		return(retval);
	}

	/* Confirm the header entries */
	if( (retval=ConfirmHeader())<0 || retval==ESCAPE ){
		if(ClearScreen()<0)	return(-1);
		return(retval);
	}

	/* Fill and display default set of permissions to the user */
	if( (retval=ShowItems( ADD ))<0 || retval==ESCAPE ){
		if(ClearScreen()<0)	return(-1);
		return(retval);
	}

	/* Allow editing of the displayed values */
	
	for(;;){
		retval = ConfirmItems(ADD);
		if( retval<0 )	return(retval);
		if( retval==ESCAPE ){
			if(ClearScreen()<0)		/* Clear the screen */
				return(-1);	
			return(ESCAPE);
		}

	/* Write the record to the database if everything is right */
		retval = WriteSession(ADD);

		if(retval==NOERROR) break;
		if(retval==LOCKED) {
			roll_back(e_mesg);
			continue;
		}

		if(retval<0) return(retval);
	}
	
	/* Free allocated memory */
	return( FreeList() );
}
/* Read the key fields through profom screen */
static
ReadKeyFields()
{
	int retval;

	if( FillKeyFields(LOW)<0 )
		return(-1);
	retval = ReadFields( 500, 500 );
	if( retval<0 || retval==ESCAPE )
		return(retval);
	return(0);
}
/* Read the given range of profom fields */
static
ReadFields(start,end)	/* read fields whose numbers range from start to end */
int start, end;		/* start and end fields */
{
	int retval;

	sr.nextfld = start;
	sr.endfld = end;
	for( ; ; ){
		fomrd( (char *)&s_rec );
		ret(err_chk(&sr));
		if( sr.retcode==RET_USER_ESC || sr.retcode==RET_VAL_CHK ){
			if( ESC_F ) return(ESCAPE);
			if( sr.retcode==RET_USER_ESC )
				continue; /* if invalid char after escape */
			retval=Validate();
			if( retval<0 || retval==ESCAPE)	return(retval);
			continue;
		}
		break;
	}
	return(0);
}
/* write the fields whose numbers range from start to end */
static
WriteFields( start,end )
int start, end;
{
	sr.nextfld = start;
	sr.endfld = end;
	fomwr( (char *)&s_rec );
	ret( err_chk(&sr) );
	return(0);
}
/* clear the screen except fn field and screen heading */
static
ClearScreen()
{
	if(FillFieldNum(HIGH)<0 ) return(-1);
	if(FillHeaderFields(HIGH)<0 ) return(-1);
	if(FillSeperatorLine(HIGH)<0 ) return(-1);
	if(FillLineHeading(HIGH)<0 ) return(-1);
	if(FillItemLines(HIGH,1,ITEMSPERPAGE)<0 ) return(-1);
	if(FillMsgRespFields(HIGH)<0 ) return(-1);

	if( WriteFields(600,9500)<0 ) return(-1);
	
	return(0);
}
/* Validate user's entries wherever required */
static
Validate()	/* Validate the values entered by the user */
{
	int mode;

	switch( sr.curfld ){
		case 500:	/* User Id */
			strcpy(up_rec.u_id, s_rec.s_userid );
			if( s_rec.s_fn[0]==CHANGE || s_rec.s_fn[0]==DELETE )
				mode = UPDATE;
			else
				mode = BROWSE;
			retval = get_userprof(&up_rec,mode,0,e_mesg);
			if( retval==ERROR ){
				fomen(e_mesg);
				get();
				return(retval);
			}
			if( s_rec.s_fn[0]==ADDREC){
				if( retval!=UNDEF ){
#ifdef ENGLISH
					fomen("Record already exists");
#else
					fomen("Cette fiches existe deja");
#endif
					get();
					s_rec.s_userid[0] = LV_CHAR;
				}
			}
			else if( retval!=NOERROR ){
				fomen(e_mesg);
				get();
				s_rec.s_userid[0] = LV_CHAR;
			}
			break;
		case 600:	/* field # for editing */
			if( sr.fillcode==FIL_OMITTED )	return(ESCAPE);
			break;
		case 1000:	/* user class */
			switch( s_rec.s_class[0] ){
				case ADMINISTRATOR:	/* Administrator */
				case ORD_USER:	/* User */
					break;
				default:
#ifdef ENGLISH
					fomer("Enter A(dministrator)/U(ser)");
#else
					fomer("Entrer A(dministrateur)/U(sager)");
#endif
					s_rec.s_class[0] = LV_CHAR;
					break;
			}
			break;
		/* if any of P_DEL, UPDATE or ADD is set, set BROWSE */
		case 2600:	/* 'ADD' access fields */
		case 3200:
		case 3800:
		case 4400:
		case 5000:
		case 5600:
		case 6200:
		case 6800:
		case 7400:
		case 8000:
		case 8600:
		case 9200:
			mode = (sr.curfld-2600)/600;
			if( s_rec.s_entries[mode].s_delete ||
			    s_rec.s_entries[mode].s_update ||
			    s_rec.s_entries[mode].s_add )
				s_rec.s_entries[mode].s_browse = 1;
	}
	sr.nextfld = sr.curfld;
	return(0);
}
/* Ask if user wants to edit hdr items before going to items */
static
ConfirmHeader()
{
	for( ; ; ){
#ifdef ENGLISH
		if(DisplayMessage( "I(tems), E(dit header), C(ancel)")<0 ) 
			return(-1);
#else
		if(DisplayMessage( "C(hoisir article), M(odifier en-tete), A(nnuler)")<0 ) 
			return(-1);
#endif
		sr.nextfld = 9500;
		fomrf( (char *)&s_rec );
		ret( err_chk(&sr) );
		switch( s_rec.s_resp[0] ){
			case ADDITEMS:	/* Go to addition of items */
				return(HideMessage());
			case EDIT:	/* Edit the fields in header */
				if( (retval=EditHeaderFields())<0 )
					return(retval);
				break;
			case CANCEL:	/* Cancel the session */
#ifdef ENGLISH
				if(DisplayMessage( 
					"Confirm the cancel (Y/N)?")<0)
					return(-1);
#else
				if(DisplayMessage(
					"Confirmer l'annulation (O/N) ?")<0)
					return(-1);
#endif
				sr.nextfld = 9500;
				fomrf( (char *)&s_rec );
				ret( err_chk(&sr) );
				if( s_rec.s_resp[0]!=YES )
					break;
				if( HideMessage()<0 ) return(-1);
				return(ESCAPE);
		}
	}
}
/* Display the given message in the message field */
static
DisplayMessage(mesg)
char *mesg;
{
	strcpy( s_rec.s_mesg, mesg );
	if( WriteFields(9400,9400)<0 )	return(-1);
	return(0);
}
/* Hide the message field */
static
HideMessage()
{
	if( FillMsgRespFields(HIGH)<0 )	return(-1);
	if( WriteFields(9400,9500)<0 ) return(-1);
	return(0);
}
/* Hide the 'Field#' field */
static
HideFldNo()
{
	if( FillFieldNum(HIGH)<0 )	return(-1);
	if( (WriteFields(600,600))<0 )
		return(-1);
	return(0);
}
/* Edit Header fields */
static
EditHeaderFields()
{
	int	i;

	/* enable dup buffers */
	for( i=800; i<=1000; i+=100 ){
		fomca1( i, 19, 2 );	/* enable dup buffer */
		fomca1( i, 10, 0 );	/* disable user escape */
	}
	sr.nextfld = 800;	sr.endfld = 1000;
	fomud( (char *)&s_rec );
	ret( err_chk(&sr) );

	for( ; ; ){
		FillFieldNum( LOW );
		if( (retval = ReadFields(600,600))<0 ) return(retval);
		if( retval==ESCAPE ){
			if( (retval = HideFldNo())<0 ) 
				return(retval);
			break;
		}
		switch( s_rec.s_fld_no ){
			case 1:
				s_rec.s_username[0] = LV_CHAR;
				if( (retval = ReadFields(800,800))<0 ) 
					return(retval);
				break;
			case 2:
				s_rec.s_terminal[0] = LV_CHAR;
				if( (retval = ReadFields(900,900))<0 ) 
					return(retval);
				break;
			case 3:
				s_rec.s_class[0] = LV_CHAR;
				if( (retval = ReadFields(1000,1000))<0 ) 
					return(retval);
				break;
			default:
				break;
		}
	}
		
	/* disable dup buffers */
	for( i=800; i<=1000; i+=100 ){
		fomca1( i, 19, 0 );	/* disable dup buffer */
		fomca1( i, 10, 1 );	/* enable user escape */
	}
	return(0);
}
static
Page *PageAllocated()	/* Allocate memory for 1 page of items on screen */
{
	return( (Page *)(malloc((unsigned)sizeof( Page ))) );
}
static
ShowPage()	/* Dump contents of node pointed to by tempptr onto screen */
{
	if( ListToScreen()<0 ) return(-1);	/* transfer data */
	if( WriteFields( 2200,9300 )<0 ) return(-1); /* display on screen */
	return(0);
}
static
ListToScreen()	/* copy contents of *tempptr to screen (items part) */
{
	short i;

	if( tempptr==NULL )
		return(-1);
	for( i=0; i<tempptr->lines_entered; i++){ 
		strcpy( s_rec.s_entries[i].s_usrflnm,tempptr->fields[i].s_usrflnm );
		s_rec.s_entries[i].s_delete = tempptr->fields[i].s_delete ;
		s_rec.s_entries[i].s_update = tempptr->fields[i].s_update ;
		s_rec.s_entries[i].s_add = tempptr->fields[i].s_add ;
		s_rec.s_entries[i].s_browse = tempptr->fields[i].s_browse ;
	}
	for( ; i<ITEMSPERPAGE; i++){ 
		s_rec.s_entries[i].s_usrflnm[0] = HV_CHAR;
		s_rec.s_entries[i].s_delete = HV_INT;
		s_rec.s_entries[i].s_update = HV_INT;
		s_rec.s_entries[i].s_add = HV_INT;
		s_rec.s_entries[i].s_browse = HV_INT;
	}
	return(0);
}
static
ConfirmItems( mode )	/* Allow user to change entered data */
int mode;		/* ADD, BROWSE, UPDATE */
{
    for( ; ; ){
	if( mode==ADD || mode==UPDATE )
#ifdef ENGLISH
	    DisplayMessage("E(dit items), C(ancel), N(ext), P(rev), Y(es)");
#else
	    DisplayMessage("M(odifier article), A(nnuler), S(uivant), P(recedent), O(ui)");
#endif
	else if( mode==BROWSE ){
		if( headptr->nextptr )
#ifdef ENGLISH
			DisplayMessage("Y(es), N(ext), P(rev)");
#else
			DisplayMessage("S(uivant), P(recedent), O(ui)");
#endif
		else
			break;
	}
	else if( mode==P_DEL ){
#ifdef ENGLISH
	    DisplayMessage("Y(es), C(ancel)");
#else
	    DisplayMessage("O(ui), A(nnuler)");
#endif
	}

	sr.nextfld = 9500;
	fomrf( (char *)&s_rec );	/* Read user's option */
	ret( err_chk(&sr) );
	switch(s_rec.s_resp[0]){
		case EDIT:	/* Edit data entered */
			if( mode==BROWSE || mode==P_DEL)
				break;	
			if( !tempptr )	/* if no page is being pointed to */
				break;
			retval = EditItemFields();
			if( retval<0 )
				return(-1);
			if( HideFldNo()<0 )
				return(-1);
			break;
		case NEXT:	/* Display the next page of items */
			if( mode==P_DEL)
				break;	
			if(tempptr && tempptr->nextptr){
				tempptr = tempptr->nextptr;
				current.page ++;
				if(ShowPage()<0 ) return(-1);
			}
			else
#ifdef ENGLISH
				fomer("Last Page Displayed");
#else
				fomer("Derniere page affiche");
#endif
			break;
		case PREV:	/* Display the prev page of items */
			if( mode==P_DEL)
				break;	
			if(tempptr && tempptr->prevptr){
				tempptr = tempptr->prevptr;
				current.page --;
				if(ShowPage()<0 ) return(-1);
			}
			else
#ifdef ENGLISH
				fomer("First Page Displayed");
#else
				fomer("Premiere page affiche");
#endif
			break;
		case CANCEL:    /* Cancel the entire session in ADD MODE only*/
			if( mode==BROWSE )
				break;
#ifdef ENGLISH
			DisplayMessage("Confirm the cancel (Y/N)?"); 
#else
			DisplayMessage("Confirmer l'annulation (O/N) ?"); 
#endif
			for( ; ; ){
				sr.nextfld = 9500;
				fomrf( (char *)&s_rec);
				if(s_rec.s_resp[0]!=YES&&
				   s_rec.s_resp[0]!=NO)
					continue;
				if( s_rec.s_resp[0]==YES ){
					if(HideMessage()<0 )
						return(-1);
					FreeList();
					return(ESCAPE);
				}
				else
					break;
			}
			break;
		case YES:	/* Save & Exit in ADD/UPDATE,Exit in INQ MODE */
			if( HideFldNo()<0 )
				return(-1);
			if( mode==BROWSE ){
				if(HideMessage()<0 )
					return(-1);
				FreeList();
				return(ESCAPE);
			}
			return(HideMessage());
	}
	continue;
   }
   return(0);
}
/* Read the field# and allow changes on that field(s) */
static
EditItemFields()
{
	int retval;

	for( ; ; ){
		s_rec.s_fld_no = LV_SHORT;
		if( (retval=ReadFields(600,600)) < 0 )
			return(-1);
		if( retval==ESCAPE )
			return(retval);
		if( s_rec.s_fld_no<1 || s_rec.s_fld_no>ITEMSPERPAGE*10+4 )
			continue;
		if( s_rec.s_fld_no>0 && s_rec.s_fld_no<=ITEMSPERPAGE ){
						/* Chg a line */ 
			if( (retval=LineChange(s_rec.s_fld_no)) < 0 )
				return(retval);
		}
	}
}
/* Read new values at given line */
static
LineChange(line_no )
int line_no;
{
	int i, firstfld, lastfld;

	/* Don't allow changes/access if item not active or not entered yet */
	if( !tempptr )	return(0);	/* if no pages, return */
	if( line_no>tempptr->lines_entered )
		return(0);

	firstfld = 2400+(line_no-1)*600;
	lastfld = 2700+(line_no-1)*600;
	for( i = firstfld; i <= lastfld; i+=100 ){
		fomca1( i, 19, 2 );		/* enable dup buffers */
		fomca1( i, 10, 0 );		/* disable user Escape */
	}
	sr.nextfld = firstfld;
	sr.endfld = lastfld;
	fomud( (char *)&s_rec );		/* update dup buffers */
	s_rec.s_entries[line_no-1].s_delete = LV_INT;
	s_rec.s_entries[line_no-1].s_update = LV_INT;
	s_rec.s_entries[line_no-1].s_add = LV_INT;
	s_rec.s_entries[line_no-1].s_browse = LV_INT;
	i= ReadFields( firstfld, lastfld );
	if( i<0 )	return(i);
	for( i = firstfld; i <= lastfld; i+=100 ){
		fomca1( i, 19, 0 );		/* disable dup buffers */
		fomca1( i, 10, 1 );		/* enable user Escape */
	}
	scpy( (char *)(tempptr->fields+line_no-1), 
		(char *)(s_rec.s_entries+line_no-1), 
		sizeof( Flacc ) );
	return(0);
}
static
WriteSession(mode)
int	mode;		/* ADD  or  UPDATE or P_DEL */
{
	int err;

	if((err= WriteRecord(mode))<0 ){/* Write the header record file */
		roll_back(e_mesg) ;
		return(err);
	}
		
	if (commit(e_mesg) < 0 ){
		fomen(e_mesg) ;
		get() ;
		return(-1);
	}
	return(0);
}
static
WriteRecord(mode)	/* Write the header of the Xaction in gltrhdr file */
int	mode;	/* ADD or P_DEL */
{
	short	i, j;
	int	access;

	strcpy( up_rec.u_id, s_rec.s_userid ) ;
	strcpy( up_rec.u_name, s_rec.s_username );
	strcpy( up_rec.u_passwd, "\0" );
	strcpy( up_rec.u_trml, s_rec.s_terminal );
	strcpy( up_rec.u_class, s_rec.s_class );

    if( mode!=P_DEL ){
	j=0;
	for( tempptr=headptr; tempptr!=NULL; tempptr=tempptr->nextptr){	
		for( i=1; i<=ITEMSPERPAGE; i++){ /* for each line on page */
			if( j==TOTAL_FILES )
				break;
			access = DFLT_CHAR ;
			if( tempptr->fields[i-1].s_delete )
				access |= P_DEL;
			if( tempptr->fields[i-1].s_update )
				access |= UPDATE;
			if( tempptr->fields[i-1].s_add )
				access |= ADD;
			if( tempptr->fields[i-1].s_browse )
				access |= BROWSE;
			up_rec.u_access[j] = (char)access;

			j++;
		}
	}
	tempptr = headptr;
    }
	retval = put_userprof( &up_rec, mode, e_mesg );
	if( retval!=NOERROR ){
		fomen(e_mesg);get();
		roll_back(e_mesg);
		return(retval);
	}

	retval = rite_audit((char*)&s_rec, USERPROF,mode,(char *)&up_rec,
					(char *)&oup_rec,e_mesg);
	if(retval==LOCKED) {
		fomen(e_mesg);get();
		roll_back(e_mesg);
		return(LOCKED);
	}

	if( retval<0 ){
		fomen(e_mesg);get();
		roll_back(e_mesg);
		return(retval);
	}
	return(0);
}

static
FreeList()	/* Free the linked list */
{
	for( tempptr=headptr; tempptr; tempptr=headptr ){
		headptr=headptr->nextptr;
		free( (char *)tempptr );
	}
	tailptr = NULL;
	return(0);
}
static
Inquire(access,direction,mode)	/* Screen inquiry, random/sequential */
int access, direction,mode;	/* RANDOM, SEQUENTIAL accesses */
				/* BROWSE, UPDATE modes */
{
	int retval;

	if( access==RANDOM ){
		retval = GetRecord(mode);	/* Read the key values */
		if(retval==UNDEF)
			return(0);
		if(retval==ESCAPE || retval==ERROR){
			if(ClearScreen()<0)	return(-1);
			return(retval);
		}
	}
	else{
		retval = GetNextRec( direction );	/* Get next rec */
		if(retval==EFL)
			return(0);
	}
	if( retval<0 ){
		fomen(e_mesg);get();
		return(-1);
	}

	/* Copy the record contents to image for audit purpose */
	scpy( (char *)&oup_rec, (char *)&up_rec, sizeof( UP_rec ) );

	retval = DisplayRecord(mode);	/* Display the record */
	if( retval<0 ) return(retval);
	if( retval==ESCAPE ) {
		roll_back(e_mesg);
		return(NOERROR);
	}

	for( ; ; ) {

		/* Confirm Item values */
		retval = ConfirmItems(mode);	/* Allow scanning of pages */
		if( retval<0 || retval==ESCAPE ) return(retval);
		if(retval == CANCEL) {
			roll_back(e_mesg);  /* Unlocking if recs not modified */
			return(NOERROR) ;
		}

		if( mode == UPDATE || mode == P_DEL ){	/* Changes made */
			retval = WriteSession(mode);
			if(retval==NOERROR) break;
			if(retval==LOCKED) {
				roll_back(e_mesg);
				retval = get_userprof(&up_rec,mode,0,e_mesg);
				if(retval==LOCKED || retval==DUPE) continue;
				if( retval==ERROR ){
					fomen(e_mesg);
					get();
					return(retval);
				}
				continue;
			}
			if(retval<0) return(retval);
		}
	}

	return( FreeList() );
}

static
DisplayRecord(mode)	/* Get and display the header and item records */
int	mode;
{
	int retval;

	retval = ShowHeaderFields();	/* Display header information */
	if( retval<0 || retval==ESCAPE ) return(retval);

	retval = ShowItems(mode);	/* Display first pageful of items */
	if( retval<0 || retval==ESCAPE ) return(retval);

	/* If N(ext) or P(rev) quit after showing first page */
	if( s_rec.s_fn[0]==NEXT || s_rec.s_fn[0]==PREV )
		return(ESCAPE);

	/* Confirm Header values if a record has to be changed */
	if( mode==UPDATE && (retval=ConfirmHeader())<0 )
		return(retval);
	if( retval==ESCAPE )
		return(retval);

	return(0);
}
static
GetNextRec(direction)	/* Read the next rec in specified direction */
int	direction ;
{
	int retval;
	
	if( flg_start(USERPROF) != direction){ 	/* file not in seq read mode */
		/* Set the least part of the key to next possible key and set
		   the file to start */
		strcpy( up_rec.u_id, s_rec.s_userid );
		inc_str( up_rec.u_id, sizeof( up_rec.u_id)-1, direction );
		flg_reset(USERPROF) ;
	}
	/* Read the next record from file */
	retval = get_n_userprof( &up_rec, BROWSE, 0, direction, e_mesg );
#ifndef ORACLE
	seq_over(USERPROF) ;
#endif
	if( retval==EFL ){
#ifdef ENGLISH
		fomen("No More Records....");
#else
		fomen("Plus de fiches....");
#endif
		get();
		flg_reset(USERPROF);
		return(EFL);
	}
	if( retval!=NOERROR ){
		fomen(e_mesg);	get();
		return(retval);
	}
	strcpy( s_rec.s_userid, up_rec.u_id );
	if( WriteFields(500,500)<0 )
		return(-1);
	return(0);
}
static
GetRecord(mode)/* Read the header key values, get terminal info ,read rec */
int	mode;	/* UPDATE, DELETE, BROWSE */
{
	int retval;

	/* Enable dup buffers for reading key */
	fomca1( 500, 19, 2 );
	sr.nextfld=500; 
	sr.endfld=500;
	fomud( (char *)&s_rec );

	s_rec.s_userid[0] = LV_CHAR;
	if( (retval=ReadFields(500,500))<0 || retval==ESCAPE ) 
		return(retval);
	strcpy( up_rec.u_id, s_rec.s_userid );

	if( mode==P_DEL )	mode = UPDATE;	/* Get record in UPDATE mode */

	retval = get_userprof( &up_rec, mode, 0, e_mesg );
	if( retval!=NOERROR ){
		fomer(e_mesg);
		get();
		return(retval);
	}
	/* Disable dup buffers after reading key */
	fomca1( 500, 19, 2 );
	if( up_rec.u_class[0]==SUPERUSER && mode!=BROWSE ){
#ifdef ENGLISH
		fomen("Can't change Superuser Profile!  Press any key.");	
#else
		fomen("Ne peut pas changer le profil du superutilisateur!  Appuyer sur une touche.");	
#endif
		get();
		return( ESCAPE );
	}
	return(0);
}
static
ShowHeaderFields()	/* Display header info */
{
	strcpy(s_rec.s_username, up_rec.u_name );
	strcpy(s_rec.s_terminal, up_rec.u_trml );
	strcpy(s_rec.s_class, up_rec.u_class );
	if( WriteFields(800,1000)<0 ) return(-1);
	return(0);
}
static
ShowItems(mode)	/* Display first pageful of items */
int	mode;
{
	if( FillSeperatorLine(LOW)<0 ) return(-1);
	if( FillLineHeading(LOW)<0 )	return(-1);
	if( WriteFields(1100,1700)<0 )	return(-1);

	if( BuildList(mode)<0 )	/* Build linked list of pages in memory */ 
		return(-1);
	if( headptr==NULL ){	/* No items present */
#ifdef ENGLISH
		fomen("No transaction items");
#else
		fomen("Aucune article de transaction");
#endif
		get();
		return( ESCAPE );
	}
	tempptr = headptr ;	/* Seek to first node/page of list */
	current.page = 1;
	if( ShowPage()<0 ) return(-1);	/* Display contents of page sought */ 
	return(0);
}
static
BuildList(mode)	/* Read item by item from file and build list */
int	mode;
{
	int i, filenum, access;

	headptr = tailptr = tempptr = NULL;

	totalitemsactive = 0;
	filenum = 0;
	for( ; ; ){
		if( filenum==TOTAL_FILES )
			break;
		current.line = (totalitemsactive+1) % ITEMSPERPAGE;
		if( current.line==0 ) current.line = ITEMSPERPAGE;
		if( current.line==1 ){	/* New page to be formed */
		    tempptr = PageAllocated();	/* Allocate memory for page */
		    if( tempptr==NULL ){
#ifdef ENGLISH
			fomen("Memory allocation error. Press any key");
#else
			fomen("Erreur d'allocation de memoire. Appuyer sur une touche");
#endif
			get();
			return(-1);
		    }
		    tempptr->lines_entered = 0;
		    if( totalitemsactive==0 ){	/* First item */
			headptr = tailptr = tempptr;
			tempptr->nextptr = tempptr->prevptr = NULL;
		    }
		    else{
			tailptr->nextptr = tempptr;
			tempptr->prevptr = tailptr;
			tempptr->nextptr = NULL;
			tailptr = tempptr;
		    }
		}
		
		access = up_rec.u_access[filenum];

		getuserflnm( filenum,tempptr->fields[current.line-1].s_usrflnm);
		tempptr->fields[current.line-1].s_usrflnm[20] = '\0';

	/* In add mode, default all access to Y if Administrator, else to N */
	    if( mode==ADD ){
		tempptr->fields[current.line-1].s_delete = 
			( s_rec.s_class[0]==ADMINISTRATOR ) ?  1  :  0;
		tempptr->fields[current.line-1].s_update = 
			( s_rec.s_class[0]==ADMINISTRATOR ) ?  1  :  0;
		tempptr->fields[current.line-1].s_add = 
			( s_rec.s_class[0]==ADMINISTRATOR ) ?  1  :  0;
		tempptr->fields[current.line-1].s_browse = 
			( s_rec.s_class[0]==ADMINISTRATOR ) ?  1  :  0;
	    }
	/* In update or delete mode, copy values from the linked list */
	    else{
		tempptr->fields[current.line-1].s_delete = 
			( access & P_DEL ) ?  1  :  0;
		tempptr->fields[current.line-1].s_update = 
			( access & UPDATE ) ?  1  :  0;
		tempptr->fields[current.line-1].s_add = 
			( access & ADD ) ?  1  :  0;
		tempptr->fields[current.line-1].s_browse = 
			( access & BROWSE ) ?  1  :  0;
	    }
		tempptr->lines_entered++;
		totalitemsactive++;
		filenum++;
		lastitemnumber = filenum;
	} 
	if( headptr==NULL ){
#ifdef ENGLISH
		fomen("No items to display. Press any key");
#else
		fomen("Pas d'articles a afficher. Appuyer sur une touche");
#endif
		get();
		return(-1);
	}

	/* upto current.line values have been entered */
	/* now fill high values for rest of the page */
	for(i=current.line+1;i<=ITEMSPERPAGE;i++){
		tempptr->fields[i-1].s_usrflnm[0]= HV_CHAR; 
		tempptr->fields[i-1].s_delete = HV_SHORT;
		tempptr->fields[i-1].s_update = HV_SHORT;
		tempptr->fields[i-1].s_add = HV_SHORT;
		tempptr->fields[i-1].s_browse = HV_SHORT;
	}
	return(0);
}

