/******************************************************************************
		Sourcename    : t4reputl.c
		System        : Budgetary Financial system.
		Module        : Personnel/Payroll
		Created on    : 92-05-22
		Created  By   : Andre Cormier
		Cobol sources : 

HISTORY:
 Date           Programmer     Description of modification
____/__/__      __________     ___________________________

Calls for the user:	( Check for the value returned: -1 on error )

int	AddMenuItem( char *name, int (* fnptr)() );
int	Initialize( char *terminalname , char *heading );
int	Process();
int	DisplayMessage( char * );
int	GetResponse( char * );
int	HideMessage();
int	GetOutputon( char * );
int	GetFilename( char * );
int	GetPrinter( short * );
int	GetNbrCopies( short * );
int	Confirm();		returns 1 for Yes, and 0 for anyother input

The programmer should make calls for appending menuitems in the following order.
		AddMenuItem( menuname, functionpointer );( max. 9 options )
		Initialize( screenheading );
		Process();	 for menu item selection
		{	Any "Get...." calls }
		Confirm();	It clears the screen and resets terminal
A maximum of 19 menuitems can be added, excluding one for Quitting the screen
which is automatically appended by the system.

For all the calls starting with "Get", user can pass any default values by 
initialising the corresponding variables before passing their address(es)
as parameters. The program doesnot do any validation except that it disallows
user's entry of end value ( range limit ) if it is smaller than starting value.

The last call should be "Confirm()",  which clears the profom screen and resets
the terminal characteristics

******************************************************************************/
#include <stdio.h>

#include <bfs_defs.h>
#include <cfomstrc.h>

#ifdef ENGLISH
#define PRINTER 'P'
#define DISPLAY 'D'
#define FILE_IO	'F'
#define YES	'Y'
#else
#define PRINTER 'I'
#define DISPLAY 'A'
#define FILE_IO	'D'
#define YES	'O'
#endif

#define SCREEN_NAME	"t4rep"
#define EXIT		12
#define PATH_FILE_SIZE	50
#define STARTLINENUM	18
#define LOW 		-1
#define HIGH 		 1
#define HL_CHAR(VAL)	VAL==HIGH ? HV_CHAR : LV_CHAR
#define HL_LONG(VAL)	VAL==HIGH ? HV_LONG : LV_LONG
#define ESC_F		sr.retcode==RET_USER_ESC && \
			(sr.escchar[0]=='F'||sr.escchar[0]=='f')
#define	OPTIONLEN	34
#define MAXOPTIONS	20

#define MESSAGE		3000
#define RESPONSE	3100

/* porep.sth - header for C structure generated by PROFOM EDITOR */

static struct	s_struct	{
	char	s_progname[11];	/* 100 program name */
	char	s_scrhdg[31];	/* 200 */
	long	s_rundt;	/* 300 DATE YYYYFMMFDD */
	char	s_item[MAXOPTIONS][OPTIONLEN];	/* 400 - 2300 options */
	short	s_option;	/* 2400 user's input */
	char	s_outputon[2];	/* 2600 report output on D / F / P */
	char	s_filename[16];	/* 2700 name of file if option is F */
	short	s_printer;	/* 2800 printer# if option is P */
	short	s_nbrcopies;	/* 2900 page length in no of lines */
	char	s_mesg[78];	/* 3000 message field */
	char	s_resp[2];	/* 3100 response field */
}	s_rec;

typedef struct{
	char	name[OPTIONLEN];
	int	(* fnptr)();
	int	mainflno;
}Menu;

struct stat_rec 	sr;		/* profom status record */

static short		call_no;
static short		totaloptions;
static int retval;	/* Global variable to store function values */
static Menu	menu[MAXOPTIONS];
static	int	initialised ;
static char	txt[80];

extern e_mesg[200];  	/* to store error messages */

static
CleanExit()/* clear and exit the screen , close files & exit program */
{
	fomcs();
	fomrt();
	exit(0);
}
Initialize( terminal,heading )	/* initialize profom and screen */
char *terminal;
char *heading;	
{
	if((retval = InitProfom(terminal))<0 ){	/* initialize profom */
		fomcs();
		fomrt();
		return(retval);
	}
	if((retval = InitScreen(heading))<0 ){		/* initialize screen */
		fomcs();
		fomrt();
		return(retval);
	}
	initialised = 1 ;
	return(0);
}	
static
InitProfom(terminal)	/* initialize profom */
char *terminal;
{
	if ( initialised ) return(0) ;
	STRCPY( sr.termnm, terminal );
	fomin( &sr );		/* initialize the screen */
	ret( err_chk(&sr) );	/* if profom error return */
	return(0);
}
static
InitScreen(heading)		/* initialize the screen */
char 	*heading;
{
	/* initialize the profom screen variables */
	STRCPY( sr.scrnam, NFM_PATH );
	strcat( sr.scrnam, SCREEN_NAME );

	/* initialize the fields of the profom screen */
	if( initialised==0 ){
		if( FillScrHdg(heading)<0 ) 	return(-1);
		if( FillMenu()<0 ) 		return(-1);
		if( FillOption(HIGH)<0 )	return(-1);
	}
	/* Move highs to inquiry area and write entire screen */
	if((retval = ClearInquiryArea())<0 )
		return(retval);

	fomcf( 1,1 ) ;		/* Enable snap screen */
	return(0);
}

/* Fill the screen heading fields, the program name and the date */
static
FillScrHdg(heading)
char *heading;
{
	STRCPY( s_rec.s_progname, PROG_NAME );
	STRCPY( s_rec.s_scrhdg, heading );
	s_rec.s_rundt = get_date();
	return(0);
}

/* Add a menu item to the array of report menu items */
AddMenuItem( menuname, fnptr, mainflno )
char	*menuname;
int	(* fnptr)();
int	mainflno;
{
	if( totaloptions==0 ){ /* no options added to array yet */
#ifdef ENGLISH
		STRCPY( menu[totaloptions].name, "RETURN TO PREVIOUS MENU" );
#else
		STRCPY( menu[totaloptions].name, "RETOURNER AU MENU PRECEDENT");
#endif
		menu[totaloptions].fnptr = NULL;
		totaloptions++;
	}

	if( totaloptions>=MAXOPTIONS )	/* array can't accomodate any more */ 
		return(-1);
	if( menuname==NULL )		/* menu name can't be null */
		return(-1);
	STRCPY( menu[totaloptions].name, menuname );
	menu[totaloptions].fnptr = fnptr;	/* copy function pointer */
	menu[totaloptions].mainflno = mainflno;  /* copy file used */
	
	totaloptions++;

	return(totaloptions-1);
}
/* Fill Menu Options with the proper names */
static
FillMenu()
{
	short	i;

	for( i=0; i<totaloptions; i++ )
		STRCPY( s_rec.s_item[i], menu[i].name );
	for( i=totaloptions; i<MAXOPTIONS; i++ )
		s_rec.s_item[i][0] = HV_CHAR;
	return(0);
}
static
FillOption( value )
short value;
{
	s_rec.s_option = value * HV_SHORT;
	return(0);
}
/* Fill output details with high or low values */
static
FillOutputDetails( value )
short value;
{
	s_rec.s_outputon[0] = HL_CHAR(value);
	s_rec.s_filename[0] = HL_CHAR(value);
	s_rec.s_printer = HV_SHORT * value;
	s_rec.s_nbrcopies = HV_SHORT * value;
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
/* Accept user's option and call the corresponding routine in a loop */
Process(terminal, heading)
char	*terminal ;
char	*heading  ;
{
	int	field;

	if((retval = Initialize(terminal, heading)) <0)
		return(retval); 
	call_no = 0;	/* No get calls made yet */
	for( ; ; ){
		mainfileno = -1 ;

		if((retval = ReadOption())<0)
			return(retval);
		if( s_rec.s_option>=totaloptions ){
#ifdef ENGLISH
			fomer("Invalid option");
#else
			fomer("Option invalide");
#endif
			continue;
		}
		if( s_rec.s_option==0 ){   /* option 0 is reserved to quit */
			fomcs();
			fomrt();
			return(0);
		}

		if((retval = CheckAccess(menu[s_rec.s_option].mainflno,
				BROWSE,e_mesg))<0)
			return(retval);	

		mainfileno = menu[s_rec.s_option].mainflno ;

		/* Highlight option. Dehighlight in Confirm() */
		field = 400+(s_rec.s_option*100);
		fomca1( field, 9, 3 );

		/* if a corresponding function exists, call it  */
		if(menu[s_rec.s_option].fnptr ){
			if((retval=(*menu[s_rec.s_option].fnptr)())<0 ) 
				return(retval);
			fomst();
		}
		else 	/* there isn't corr. function,  return value */
			return (s_rec.s_option);

		if((retval = ClearInquiryArea())<0 )
			return(retval);
	}
}

Confirm()	/* returns 1 for yes, 0 for no, -1 for error */
{		/* Clears the profom screen when user's response is 'Y' */
	int	field;

#ifdef ENGLISH
	if((retval = DisplayMessage("Confirm (Y/N)?"))<0 )
		return(retval);
#else
	if((retval = DisplayMessage("Confirmer (O/N)?"))<0 )
		return(retval);
#endif
	if((retval = GetResponse(s_rec.s_resp))<0 ) 
		return(retval);

	field = 400 + (s_rec.s_option * 100);

	if( s_rec.s_resp[0]==YES ){
		fflush(stdout) ;
		fomcs();
		fomrt();
		return(1);
	}
	/* Dehighlight the option which is highlighted in Process() */
	fomca1( field, 9, 5 );

	return( HideMessage() );
}

static
ClearInquiryArea()
{
	short i;

	call_no = 0;	/* Reset the call_no */
	if(FillOutputDetails(HIGH)<0 )
		return(-1);
	if(FillMsgRespFields(HIGH)<0 )
		return(-1);
	return( WriteFields((char *)&s_rec,1,0) );
}
static
ReadOption()
{
	s_rec.s_option = LV_SHORT;
	return( ReadFields( 2400, 2400 ) );
	
}
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
			if( ESC_F ) return(EXIT);
			retval=Validate();
			if( retval<0 || retval==EXIT)	return(retval);
			continue;
		}
		break;
	}
	return(0);
}
static
Validate()	/* Validate the values entered by the user */
{
	int	retval;

	switch( sr.curfld ){
		case 2400:	/* menu option */
			if( s_rec.s_option<0 || s_rec.s_option>=totaloptions ){
#ifdef ENGLISH
				fomer("Invalid option");
#else
				fomer("Option invalide");
#endif
				s_rec.s_option = LV_SHORT;
			}
			break;
		case 2600:	/* output on */
			if( s_rec.s_outputon[0]!= DISPLAY &&
			    s_rec.s_outputon[0]!= PRINTER &&
			    s_rec.s_outputon[0]!= FILE_IO   ){
#ifdef ENGLISH
				fomer("D(isplay), P(rinter), F(ile)");
#else
				fomer("A(fficher), I(mprimante), D(ossier)");
#endif
				s_rec.s_outputon[0] = LV_CHAR;
			}
			break;
		case 2700:	/* Filename */
			break;
		case 2800:	/* Printer */
			break;
		case 2900:	/* Number of copies */
			if(s_rec.s_nbrcopies == 0) {
#ifdef ENGLISH
				fomer("No. of Copies can't be Zero");
#else
				fomer("Nombre de copies ne peut pas etre zero");
#endif
				s_rec.s_nbrcopies = LV_SHORT;
			}
			break;
	}
	sr.nextfld = sr.curfld;
	return(0);
}
static
DisplayMessage(mesg)	/* Display the given message in the message field */
char *mesg;
{
	STRCPY( s_rec.s_mesg, mesg );
	return( WriteFields((char *)&s_rec,MESSAGE,MESSAGE) );
	
}
HideMessage()	/* Hide the message field */
{
	if( FillMsgRespFields(HIGH)<0 )	return(-1);
	return( WriteFields((char *)&s_rec,MESSAGE,RESPONSE) );
}
GetResponse( respchar )
char *respchar;
{
	int	retval;

	s_rec.s_resp[0] = LV_CHAR;
	retval = ReadFields( RESPONSE, RESPONSE );
	if( retval!=EXIT && retval>=0 )
		*respchar = s_rec.s_resp[0];
	return( retval );
}
GetOutputon( outputon )
char	*outputon;
{
	int	retval;

#ifdef ENGLISH
	fomer("D(isplay), P(rinter), F(ile)");
#else
	fomer("A(fficher), I(mprimante), D(ossier)");
#endif
	STRCPY( s_rec.s_outputon, outputon );
	fomca1( 2600, 19, 2 );
	sr.nextfld = 2600;
	sr.endfld = 2600;
	fomud( (char *)&s_rec);
	s_rec.s_outputon[0] = LV_CHAR;
	retval = ReadFields(2600,2600);
	if( retval!=EXIT && retval>=0 )
		*outputon = s_rec.s_outputon[0];
	fomca1( 2600, 19, 0 );
	return( retval );
}
GetFilename( filename )
char	*filename;
{
	int	retval;

	STRCPY( s_rec.s_filename, filename );
	fomca1( 2700, 19, 2 );
	sr.nextfld = 2700;
	sr.endfld = 2700;
	fomud( (char *)&s_rec);
	s_rec.s_filename[0] = LV_CHAR;
	retval = ReadFields(2700,2700);
	if( retval!=EXIT && retval>=0 )
		strcpy( filename , s_rec.s_filename ) ;
	fomca1( 2700, 19, 0 );
	return( retval );
}
GetPrinter( printer )
short	*printer;
{
	int	retval;

	s_rec.s_printer = *printer ;
	fomca1( 2800, 19, 2 );
	sr.nextfld = 2800;
	sr.endfld = 2800;
	fomud( (char *)&s_rec);
	s_rec.s_printer = LV_SHORT;
	retval = ReadFields(2800,2800);
	if( retval!=EXIT && retval>=0 )
		*printer = s_rec.s_printer;
	fomca1( 2800, 19, 0 );
	return( retval );
}
GetNbrCopies( copies )
short	*copies;
{
	int	retval;

	s_rec.s_nbrcopies = *copies ;
	fomca1( 2900, 19, 2 );
	sr.nextfld = 2900;
	sr.endfld = 2900;
	fomud( (char *)&s_rec);
	s_rec.s_nbrcopies = LV_SHORT;
	retval = ReadFields(2900,2900);
	if( retval!=EXIT && retval>=0)
		*copies = s_rec.s_nbrcopies;
	fomca1( 2900, 19, 0 );
	return( retval );
}

