/******************************************************************************
		Sourcename    : poreputl.c
		System        : Budgetary Financial system.
		Module        : PO reports
		Created on    : 89-09-28
		Created  By   : K HARISH.
		Cobol sources : 

HISTORY:
 Date           Programmer     Description of modification
____/__/__      __________     ___________________________
1990/11/23      peter ralph	Right justify numeric supplier codes
				s_supp_no2,s_supp_no1

Calls for the user:	( Check for the value returned: -1 on error )

int	AddMenuItem( char *name, int (* fnptr)() );
int	Initialize( char *terminalname , char *heading );
int	Process();
int	GetPoRange(long *, long * );
int 	GetSuppRange(char *, char *);
int	GetSNameRange( char *, char *);
int	GetFund( short *, short *);
int	GetAcctRange( char *, char * );
int	DisplayMessage( char * );
int	GetResponse( char * );
int	HideMessage();
int	GetOutputon( char * );
int	GetFilename( char * );
int	GetPrinter( short * );
int	GetNbrCopies( short * );
int 	GetDate( long * );
int 	GetDate( long * , long *);
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

#define SCREEN_NAME	"porep"
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
#define MAXQUERYLINES	6
#define PONUMBER        0
#define SUPP_NO		1
#define FUND		2
#define	ACCOUNT		3
#define SUPP_NAME	4
#define DATE		5

#define PONO1		3000
#define PONO2		3100
#define SUPPNO1		3200
#define SUPPNO2		3300
#define FUND1		3325
#define FUND2		3350
#define ACCOUNT1	3400
#define ACCOUNT2	3500
#define SUPPNAME1	3600
#define SUPPNAME2	3700
#define DATE1		3800
#define DATE2		3900
#define MESSAGE		4000
#define RESPONSE	4100

/* porep.sth - header for C structure generated by PROFOM EDITOR */

static struct	s_struct	{
	char	s_progname[11];	/* 100 program name */
	char	s_scrhdg[16];	/* 200 */
	long	s_rundt;	/* 300 DATE YYYYFMMFDD */
	char	s_item[MAXOPTIONS][OPTIONLEN];	/* 400 - 2300 options */
	short	s_option;	/* 2400 user's input */
	char	s_outputon[2];	/* 2600 report output on D / F / P */
	char	s_filename[16];	/* 2700 name of file if option is F */
	short	s_printer;	/* 2800 printer# if option is P */
	short	s_nbrcopies;	/* 2900 page length in no of lines */
	short   s_no_up;	/* 2950 number of labels across */
	long	s_po_no1;	/* 3000 starting po number */
	long	s_po_no2;	/* 3100 ending po number */
	char	s_supp_no1[11];	/* 3200 starting supplier no */
	char	s_supp_no2[11];	/* 3300 ending supplier no */
	short   s_fund1;	/* 3325 staring fund number */
	short	s_fund2;	/* 3350 ending fund number */
	char	s_acc1[19];	/* 3400 starting account */
	char	s_acc2[19];	/* 3500 ending account */
	char	s_sname1[25];   /* 3600 starting supplier name */
	char    s_sname2[25];	/* 3700 ending supplier name */
	long	s_date1;	/* 3800 starting date */
	long	s_date2;	/* 3900 starting date */
	char	s_mesg[77];	/* 4000 message field */
	char	s_resp[2];	/* 4100 response field */
}	s_rec;

typedef struct{
	char	name[OPTIONLEN];
	int	(* fnptr)();
	int	mainflno;
}Menu;

struct stat_rec 	sr;		/* profom status record */

static int		line[MAXQUERYLINES];/* for positioning key queries */
static short		call_no;
static short		totaloptions;
static int retval;	/* Global variable to store function values */
static Menu	menu[MAXOPTIONS];
static	int	initialised ;
static char	txt[80];

extern e_mesg[80];  	/* to store error messages */

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
	s_rec.s_no_up = HV_SHORT * value;
	return(0);
}
static
FillPoNbr( val1, val2 )
short	val1, val2;
{
	s_rec.s_po_no1 = HL_LONG( val1 );
	s_rec.s_po_no2 = HL_LONG( val2 );
	return(0);
}
static
FillSuppNbr( val1, val2 )
short	val1, val2;
{
	s_rec.s_supp_no1[0] = HL_CHAR( val1 );
	s_rec.s_supp_no2[0] = HL_CHAR( val2 );
	return(0);
}
static
FillFund( val1, val2 )
short val1, val2;
{
	s_rec.s_fund1 = HV_SHORT * val1;
	s_rec.s_fund2 = HV_SHORT * val2;
	return(0);
} 
static
FillAccount( val1, val2 )
short	val1, val2;
{
	s_rec.s_acc1[0] = HL_CHAR( val1 );
	s_rec.s_acc2[0] = HL_CHAR( val2 );
	return(0);
}
static
FillSName( val1, val2 )
short	val1, val2;
{
	s_rec.s_sname1[0] = HL_CHAR( val1 );
	s_rec.s_sname2[0] = HL_CHAR( val2 );
	return(0);
}
static
FillSDate( val1, val2 )
int	val1, val2;
{
	s_rec.s_date1 = HL_LONG( val1 );
	s_rec.s_date2 = HL_LONG( val2 );
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

/*	The following Get....() calls display default values given as parameter 
*	and fill up the variable with the value entered by the user 
*	The fields for accepting input are positioned in the order in which
*	they are called. For this, they make use of variable 'call_no'
*/

GetPoRange( ponbr1, ponbr2 )
long *ponbr1, *ponbr2;
{
	int	retval=0;

	if( line[PONUMBER]==0 )	/* no account call made yet */
		line[PONUMBER] = STARTLINENUM + call_no++;
	fomca2( PONO1,1, line[PONUMBER], 1 );
	fomca2( PONO1,2, line[PONUMBER], 17 );
	fomca2( PONO2,1, line[PONUMBER], 43 );
	fomca2( PONO2,2, line[PONUMBER], 48 );

	s_rec.s_po_no1 = *ponbr1;
	s_rec.s_po_no2 = *ponbr2;
	fomca1( PONO1, 19, 2 );
	fomca1( PONO2, 19, 2 );
	sr.nextfld = PONO1;
	sr.endfld = PONO2;
	fomud( (char *)&s_rec);
	s_rec.s_po_no1 = LV_LONG;
	s_rec.s_po_no2 = LV_LONG;
	retval = ReadFields( PONO1, PONO2 );
	if( retval!=EXIT && retval>=0 ){
		*ponbr1 = s_rec.s_po_no1;
		*ponbr2 = s_rec.s_po_no2;
	}
	fomca1( PONO1, 19, 0 );
	fomca1( PONO2, 19, 0 );

	return(retval);
}

GetSuppRange( supp1, supp2 )
char *supp1, *supp2;
{
	int	retval=0;

	if( line[SUPP_NO]==0 )	/* no account call made yet */
		line[SUPP_NO] = STARTLINENUM + call_no++;
	fomca2( SUPPNO1,1, line[SUPP_NO], 1 );
	fomca2( SUPPNO1,2, line[SUPP_NO], 17 );
	fomca2( SUPPNO2,1, line[SUPP_NO], 43 );
	fomca2( SUPPNO2,2, line[SUPP_NO], 48 );

	STRCPY( s_rec.s_supp_no1, supp1 );
	STRCPY( s_rec.s_supp_no2, supp2 );
	fomca1( SUPPNO1, 19, 2 );
	fomca1( SUPPNO2, 19, 2 );
	sr.nextfld = SUPPNO1;
	sr.endfld = SUPPNO2;
	fomud( (char *)&s_rec);
	s_rec.s_supp_no1[0] = LV_CHAR;
	s_rec.s_supp_no2[0] = LV_CHAR;
	retval = ReadFields( SUPPNO1, SUPPNO2 );
	if( retval!=EXIT && retval>=0 ){
		Right_Justify_Numeric(s_rec.s_supp_no1,
					sizeof(s_rec.s_supp_no1)-1);
		Right_Justify_Numeric(s_rec.s_supp_no2,
					sizeof(s_rec.s_supp_no1)-1);
		strcpy( supp1, s_rec.s_supp_no1 );
		strcpy( supp2, s_rec.s_supp_no2 );
	}
	fomca1( SUPPNO1, 19, 0 );
	fomca1( SUPPNO2, 19, 0 );

	return(retval);
}

GetFundRange( fund1, fund2 )
short *fund1, *fund2;
{
	int	retval=0;

	if(line[FUND]==0)	/* no fund call made yet */
		line[FUND] = STARTLINENUM + call_no++;
	fomca2(FUND1,1, line[FUND], 1 );
	fomca2(FUND1,2, line[FUND], 17 );
	fomca2(FUND2,1, line[FUND], 43 );
	fomca2(FUND2,2, line[FUND], 48 );

	s_rec.s_fund1 = *fund1;
	s_rec.s_fund2 = *fund2;
	fomca1( FUND1, 19, 2 );
	fomca1( FUND2, 19, 2 );
	sr.nextfld = FUND1;
	sr.endfld = FUND2;
	fomud( (char *)&s_rec);
	s_rec.s_fund1 = LV_SHORT;
	s_rec.s_fund2 = LV_SHORT;
	retval = ReadFields( FUND1, FUND2 );
	if( retval!=EXIT && retval>=0 ){
		*fund1 = s_rec.s_fund1;
		*fund2 = s_rec.s_fund2;
	}
	fomca1( FUND1, 19, 0 );
	fomca1( FUND2, 19, 0 );

	return(retval);
}

GetAcctRange( acct1, acct2 )
char *acct1, *acct2;
{
	int	retval=0;

	if( line[ACCOUNT]==0 )	/* no account call made yet */
		line[ACCOUNT] = STARTLINENUM + call_no++;
	fomca2( ACCOUNT1,1, line[ACCOUNT], 1 );
	fomca2( ACCOUNT1,2, line[ACCOUNT], 17 );
	fomca2( ACCOUNT2,1, line[ACCOUNT], 43 );
	fomca2( ACCOUNT2,2, line[ACCOUNT], 48 );

	STRCPY( s_rec.s_acc1, acct1 );
	STRCPY( s_rec.s_acc2, acct2 );
	fomca1( ACCOUNT1, 19, 2 );
	fomca1( ACCOUNT2, 19, 2 );
	sr.nextfld = ACCOUNT1;
	sr.endfld = ACCOUNT2;
	fomud( (char *)&s_rec);
	s_rec.s_acc1[0] = LV_CHAR;
	s_rec.s_acc2[0] = LV_CHAR;
	retval = ReadFields( ACCOUNT1, ACCOUNT2 );
	if( retval!=EXIT && retval>=0 ){
		strcpy( acct1, s_rec.s_acc1 );
		strcpy( acct2, s_rec.s_acc2 );
	}
	fomca1( ACCOUNT1, 19, 0 );
	fomca1( ACCOUNT2, 19, 0 );

	return(retval);
}

GetSNameRange( sname1, sname2 )
char *sname1, *sname2;
{
	int	retval=0;

	if( line[SUPP_NAME]==0 )	/* no account call made yet */
		line[SUPP_NAME] = STARTLINENUM + call_no++;
	fomca2( SUPPNAME1,1, line[SUPP_NAME], 1 );
	fomca2( SUPPNAME1,2, line[SUPP_NAME], 17 );
	fomca2( SUPPNAME2,1, line[SUPP_NAME], 43 );
	fomca2( SUPPNAME2,2, line[SUPP_NAME], 48 );

	STRCPY( s_rec.s_sname1, sname1 );
	STRCPY( s_rec.s_sname2, sname2 );
	fomca1( SUPPNAME1, 19, 2 );
	fomca1( SUPPNAME2, 19, 2 );
	sr.nextfld = SUPPNAME1;
	sr.endfld = SUPPNAME2;
	fomud( (char *)&s_rec);
	s_rec.s_sname1[0] = LV_CHAR;
	s_rec.s_sname2[0] = LV_CHAR;
	retval = ReadFields( SUPPNAME1, SUPPNAME2 );
	if( retval!=EXIT && retval>=0 ){
		strcpy( sname1, s_rec.s_sname1 );
		strcpy( sname2, s_rec.s_sname2 );
	}
	fomca1( SUPPNAME1, 19, 0 );
	fomca1( SUPPNAME2, 19, 0 );

	return(retval);
}
GetDate( date )
long	*date;
{
	int	retval=0;

	if( line[DATE]==0 )	/* no fund call made yet */
		line[DATE] = STARTLINENUM + call_no++;
	fomca2( DATE1,1, line[DATE], 1 );
	fomca2( DATE1,2, line[DATE], 17 );
	fomca2( DATE2,1, line[DATE], 43 );
	fomca2( DATE2,2, line[DATE], 48 );

	s_rec.s_date1 = *date ;
	fomca1( DATE1, 19, 2 );
	sr.nextfld = DATE1;
	sr.endfld = DATE1;
	fomud( (char *)&s_rec);
	s_rec.s_date1 = LV_LONG;
	retval = ReadFields( DATE1, DATE1 );
	if( retval!=EXIT && retval>=0 )
		*date = s_rec.s_date1;
	fomca1( DATE1, 19, 0 );

	return(retval);
}

GetDateRange( date1, date2 )
long	*date1, *date2;
{
	int	retval=0;

	if( line[DATE]==0 )	/* no date call made yet */
		line[DATE] = STARTLINENUM + call_no++;
	fomca2( DATE1,1, line[DATE], 1 );
	fomca2( DATE1,2, line[DATE], 17 );
	fomca2( DATE2,1, line[DATE], 43 );
	fomca2( DATE2,2, line[DATE], 48 );

	s_rec.s_date1 = *date1 ;
	s_rec.s_date2 = *date2 ;
	fomca1( DATE1, 19, 2 );
	fomca1( DATE2, 19, 2 );
	sr.nextfld = DATE1;
	sr.endfld = DATE2;
	fomud( (char *)&s_rec);
	if( FillSDate( LOW, LOW )<0 )
		return(-1);
	retval = ReadFields( DATE1, DATE2 );
	if( retval!=EXIT && retval>=0 ){
		*date1 = s_rec.s_date1;
		*date2 = s_rec.s_date2;
	}
	fomca1( DATE1, 19, 0 );
	fomca1( DATE2, 19, 0 );

	return(retval);
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
	for( i=0; i<MAXQUERYLINES; i++ )/* so that next time fields can be */
		line[i] = 0;	/* repositioned for next routine calls */
	if(FillOutputDetails(HIGH)<0 )
		return(-1);
	if(FillPoNbr(HIGH,HIGH)<0)
		return(-1);
	if(FillSuppNbr(HIGH,HIGH)<0)
		return(-1);
	if(FillFund(HIGH,HIGH)<0)
		return(-1);
	if(FillAccount(HIGH,HIGH)<0 )
		return(-1);
	if(FillSName(HIGH,HIGH)<0 )
		return(-1);
	if(FillSDate(HIGH,HIGH)<0 )
		return(-1);
	if(FillMsgRespFields(HIGH)<0 )
		return(-1);
	return(WriteFields(1,0));
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
WriteFields( start,end )/* write fields whose numbers range from start to end */
int start, end;
{
	sr.nextfld = start;
	sr.endfld = end;
	fomwr( (char *)&s_rec );
	ret( err_chk(&sr) );
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
		case PONO1:	/* start po number */
			break;
		case PONO2:	/* end po number */
			if( s_rec.s_po_no2 < s_rec.s_po_no1 ){
#ifdef ENGLISH
				fomer("Ending po no. can't be less than starting po no.");
#else
				fomer("Numero du BC finissant ne peut pas etre moins que le numero du BC debutant");
#endif
				s_rec.s_po_no2 = LV_LONG;
			}
			break;
		case SUPPNO1:      /* starting supplier number */
			Right_Justify_Numeric(s_rec.s_supp_no1,
						sizeof(s_rec.s_supp_no1)-1);
			break;
		case SUPPNO2:      /* ending supplier number */
			Right_Justify_Numeric(s_rec.s_supp_no2,
						sizeof(s_rec.s_supp_no2)-1);
			if(strcmp(s_rec.s_supp_no2,s_rec.s_supp_no1 ) <0){
#ifdef ENGLISH
				fomer("Ending Supp no. can't be less than starting Supp no.");
#else
				fomer("No du fournisseur finissant doit etre moins que no du fournisseur debutant");
#endif
				s_rec.s_supp_no2[0] = LV_CHAR;
			}
			break;
		case FUND1:	/* starting fund number */
			if(s_rec.s_fund1<1) {
#ifdef ENGLISH
				fomer("Starting fund cannot be less than 1");
#else
				fomer("Fond debutant ne peut pas etre moins que 1");
#endif
				s_rec.s_fund1 = LV_SHORT;
			}
			break;
		case FUND2:	/* ending fund number */
			if(s_rec.s_fund1 > s_rec.s_fund2) {
#ifdef ENGLISH
				fomer("Ending fund cannot be less than starting fund");
#else
				fomer("Fond finissant ne peut pas etre moins que fond debutant");
#endif
				s_rec.s_fund2 = LV_SHORT;
			}
			break;
		case ACCOUNT1:	/* starting account number */
			if( acnt_chk(s_rec.s_acc1)==ERROR ){
#ifdef ENGLISH
				fomer("Invalid Account number");
#else
				fomer("Numero de compte invalide");
#endif
				s_rec.s_acc1[0]=LV_CHAR;
				break;
			}
			break;
		case ACCOUNT2:	/* ending account number */
			if( acnt_chk(s_rec.s_acc2)==ERROR ){
#ifdef ENGLISH
				fomer("Invalid Account number");
#else
				fomer("Numero de compte invalide");
#endif
				s_rec.s_acc2[0]=LV_CHAR;
				break;
			}
			if( strcmp( s_rec.s_acc2, s_rec.s_acc1 )<0 ){
#ifdef ENGLISH
				fomer("Ending accno can't be less than starting accno");
#else
				fomer("Numero de compte finissant ne peut pas etre moins que numero de compte debutant");
#endif
				s_rec.s_acc2[0] = LV_CHAR;
			}
		case SUPPNAME2:  	/* ending supplier name */
			if(strcmp(s_rec.s_sname2,s_rec.s_sname1) <0){
#ifdef ENGLISH
				fomer("Ending supplier name can't be less that Starting supplier name");
#else
				fomer("Nom du fournisseur finissant ne peut pas etre avant nom du fournisseur debutant");
#endif
				s_rec.s_sname2[0] = LV_CHAR;
			}
			break;
		case DATE1:	/* starting date */
			break;
		case DATE2:	/* ending date */
			if(s_rec.s_date2 < s_rec.s_date1) {
#ifdef ENGLISH
				fomer("Ending date can't be less than Starting date");
#else
				fomer("Date finnissant ne peut pas etre plus tot que la date debutante");
#endif
			}
			break;
	}
	sr.nextfld = sr.curfld;
	return(0);
}
DisplayMessage(mesg)	/* Display the given message in the message field */
char *mesg;
{
	STRCPY( s_rec.s_mesg, mesg );
	return( WriteFields(MESSAGE,MESSAGE) );
	
}
HideMessage()	/* Hide the message field */
{
	if( FillMsgRespFields(HIGH)<0 )	return(-1);
	return( WriteFields(MESSAGE,RESPONSE) );
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
GetNbrup( nbrlabels )
short	*nbrlabels;
{
	int	retval;

	s_rec.s_no_up = *nbrlabels;
	fomca1( 2950, 19, 2);
	sr.nextfld = 2950;
	sr.endfld = 2950;
	fomud( (char *)&s_rec);
	s_rec.s_no_up = LV_SHORT;
	retval = ReadFields(2950,2950);
	if( retval != EXIT && retval>=0 )
		*nbrlabels = s_rec.s_no_up;
	fomca1( 2950, 19, 0);
	return( retval );
}

