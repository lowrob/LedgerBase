/******************************************************************************
		Sourcename    : roereputl.c
		System        : Employee ROE Report system.
		Module        : Personnel/Payroll
		Created on    : 08-FEB-93 
		Created  By   : m. galvin. 

HISTORY:
 Date           Programmer     Description of modification
____/__/__      __________     ___________________________

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
#include <bfs_pp.h>
#include <isnames.h>
#include <bfs_defs.h>

#ifdef ENGLISH
#define PRINTER 'P'
#define DISPLAY 'D'
#define FILE_IO	'F'
#define YES	'Y'
#define ALPHSORT 'A'
#define NUMSORT 'N'
#define	SENSORT1 'D'
#define	SENSORT2 'X'
#define BARGSORT 'B'
#define CENTERSORT 'L'
#define DEPTSORT 'D'
#define AREASORT 'A'
#define POSISORT 'P'
#define CLASSSORT 'C'
#else
#define PRINTER 'I'
#define DISPLAY 'A'
#define FILE_IO	'D'
#define YES	'O'
#define ALPHSORT 'A'   /*translate for the FRENCH */
#define NUMSORT 'N'
#define SENSORT1 'D'
#define SENSORT2 'X'
#define BARGSORT 'B'
#define CENTERSORT 'L'
#define DEPTSORT 'D'
#define AREASORT 'A'
#define POSISORT 'P'
#define CLASSSORT 'C'
#endif

#define SCREEN_NAME	"ppemprep"
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
#define MAXQUERYLINES	10	
#define BARGAINING      0
#define CENTER		1
#define DEPT		2
#define AREACODE	3
#define	POS		4
#define CLASS		5
#define EMP		6	
#define	AGE		7	
#define ANNIV		8	
#define DAT		9	
#define SORTOP1		3000
#define SORTOP2		3100
#define BARG1		3200
#define BARG2		3300
#define CENTER1		3400	
#define CENTER2		3500
#define DEPT1		3600
#define DEPT2		3700
#define AREA1		3800
#define AREA2		3900
#define POS1		4000
#define POS2		4100
#define CLASS1		4200
#define CLASS2		4300
#define EMP1		4400
#define EMP2		4500
#define	AGE1		4525
#define ANNIV1		4550
#define DATE1		4575
#define DATE2		4590
#define MESSAGE		4600
#define RESPONSE	4700

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
	char	s_sortop1[2];	/* 3000 sort option one A N*/
	char	s_sortop2[2];	/* 3100 sort option two B C D A P*/
	char	s_barg1[7];	/* 3200 starting Bargaining Unit */
	char	s_barg2[7];	/* 3300 ending Bargaining Unit */
	short   s_center1;	/* 3400 starting Cost Center */
	short   s_center2;	/* 3500 ending Cost Center */
	char	s_dept1[7];	/* 3600 starting Department  */
	char	s_dept2[7];	/* 3700 ending Department */
	char	s_area1[7];	/* 3800 starting area */
	char	s_area2[7];	/* 3900 ending area */
	char	s_pos1[7];	/* 4000 starting position */
	char	s_pos2[7];	/* 4100 ending position */
	char	s_class1[7];	/* 4200 starting position */
	char	s_class2[7];	/* 4300 ending position */
	char	s_emp1[13];	/* 4400 starting employee */
	char	s_emp2[13];	/* 4500 ending employee */
	short	s_age;
	short	s_anniv;	/* 4550 anniversary month */
	long	s_date1;	/* 4575 starting date */
	long	s_date2;	/* 4590 ending date */
	char	s_mesg[78];	/* 4600 message field */
	char	s_resp[2];	/* 4700 response field */
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
static int	multi_line; 	/*line for multi column ranger*/
static int	column;		/*column for multi column ranger*/

static	int	sensort_call;

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
static
FillSortop1( value )
short	value;
{
	s_rec.s_sortop1[0] = HL_CHAR( value );
	return(0);
}
static
FillSortop2( value )
short	value;
{
	s_rec.s_sortop2[0] = HL_CHAR( value );
	return(0);
}
static
FillBarg( val1, val2 )
short	val1, val2;
{
	s_rec.s_barg1[0] = HL_CHAR( val1 );
	s_rec.s_barg2[0] = HL_CHAR( val2 );
	return(0);
}
static
FillCenter( val1, val2 )
short	val1, val2;
{
	s_rec.s_center1 = HV_SHORT * val1;
	s_rec.s_center2 = HV_SHORT * val2;
	return(0);
}
static
FillDept( val1, val2 )
short	val1, val2;
{
	s_rec.s_dept1[0] = HL_CHAR( val1 );
	s_rec.s_dept2[0] = HL_CHAR( val2 );
	return(0);
}
static
FillArea( val1, val2 )
short val1, val2;
{
	s_rec.s_area1[0] = HL_CHAR( val1 );
	s_rec.s_area2[0] = HL_CHAR( val2 );
	return(0);
} 
static
FillPosition( val1, val2 )
short	val1, val2;
{
	s_rec.s_pos1[0] = HL_CHAR( val1 );
	s_rec.s_pos2[0] = HL_CHAR( val2 );
	return(0);
}
static
FillClass( val1, val2 )
short	val1, val2;
{
	s_rec.s_class1[0] = HL_CHAR( val1 );
	s_rec.s_class2[0] = HL_CHAR( val2 );
	return(0);
}
static
FillEmployee( val1, val2 )
short	val1, val2;
{
	s_rec.s_emp1[0] = HL_CHAR( val1 );
	s_rec.s_emp2[0] = HL_CHAR( val2 );
	return(0);
}
static
FillAge( val1 )
short	val1;
{
	s_rec.s_age = HV_SHORT * val1;
	return(0);
}
static
FillAnniv( val1 )
short	val1;
{
	s_rec.s_anniv = HV_SHORT * val1;
	return(0);
}
static
FillDate( val1, val2 )
short	val1, val2;
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

GetBargRange( barg1, barg2 )
char *barg1, *barg2;
{
	int	retval=0;

	column = 1; /*initialize column at start each time*/
	if((retval=GetPosition(BARGAINING))<0){
		return(retval);
	}

	line[BARGAINING] = multi_line;
	fomca2( BARG1,1, line[BARGAINING], 1 +  (column * 42) );
	fomca2( BARG1,2, line[BARGAINING], 19 + (column * 40) );
	fomca2( BARG2,1, line[BARGAINING], 28 + (column * 40) );
	fomca2( BARG2,2, line[BARGAINING], 32 + (column * 40) );

	STRCPY( s_rec.s_barg1, barg1 );
	STRCPY( s_rec.s_barg2, barg2 );
	fomca1( BARG1, 19, 2 );
	fomca1( BARG2, 19, 2 );
	sr.nextfld = BARG1;
	sr.endfld = BARG2;
	fomud( (char *)&s_rec);
	s_rec.s_barg1[0] = LV_CHAR;
	s_rec.s_barg2[0] = LV_CHAR;
	retval = ReadFields( BARG1, BARG2 );
	if( retval!=EXIT && retval>=0 ){
		Right_Justify_Numeric(s_rec.s_barg1,
					sizeof(s_rec.s_barg1)-1);
		Right_Justify_Numeric(s_rec.s_barg2,
					sizeof(s_rec.s_barg2)-1);
		strcpy( barg1, s_rec.s_barg1 );
		strcpy( barg2, s_rec.s_barg2 );
	}
	fomca1( BARG1, 19, 0 );
	fomca1( BARG2, 19, 0 );

	return(retval);
}
GetCenterRange( center1, center2 )
short *center1, *center2;
{
	int	retval=0;

	if((retval=GetPosition(CENTER))<0){
		return(retval);
	}
	line[CENTER] = multi_line;
	fomca2( CENTER1,1, line[CENTER], 1 + (42 * column));
	fomca2( CENTER1,2, line[CENTER], 19 + (40 * column ));
	fomca2( CENTER2,1, line[CENTER], 28 + (39 * column) );
	fomca2( CENTER2,2, line[CENTER], 32 + (39 * column) );

	s_rec.s_center1 = *center1;
	s_rec.s_center2 = *center2;
	fomca1( CENTER1, 19, 2 );
	fomca1( CENTER2, 19, 2 );
	sr.nextfld = CENTER1;
	sr.endfld = CENTER2;
	fomud( (char *)&s_rec);
	s_rec.s_center1 = LV_SHORT; 
       	s_rec.s_center2 = LV_SHORT;
	retval = ReadFields( CENTER1, CENTER2 );
	if( retval!=EXIT && retval>=0 ){
		*center1 = s_rec.s_center1; 
		*center2 = s_rec.s_center2;
	}
	fomca1( CENTER1, 19, 0 );
	fomca1( CENTER2, 19, 0 );

	return(retval);
}

GetDeptRange( dept1, dept2 )
char *dept1, *dept2;
{
	int	retval=0;

	if((retval=GetPosition(DEPT))<0){
		return(retval);
	}
	line[DEPT] = multi_line;
	fomca2( DEPT1,1, line[DEPT], 1 + (40 * column));
	fomca2( DEPT1,2, line[DEPT], 19 + (40 * column));
	fomca2( DEPT2,1, line[DEPT], 28 + (40 * column));
	fomca2( DEPT2,2, line[DEPT], 32 + (40 * column));

	STRCPY( s_rec.s_dept1, dept1 );
	STRCPY( s_rec.s_dept2, dept2 );
	fomca1( DEPT1, 19, 2 );
	fomca1( DEPT2, 19, 2 );
	sr.nextfld = DEPT1;
	sr.endfld = DEPT2;
	fomud( (char *)&s_rec);
	s_rec.s_dept1[0] = LV_CHAR;
	s_rec.s_dept2[0] = LV_CHAR;
	retval = ReadFields( DEPT1, DEPT2 );
	if( retval!=EXIT && retval>=0 ){
		Right_Justify_Numeric(s_rec.s_dept1,
					sizeof(s_rec.s_dept1)-1);
		Right_Justify_Numeric(s_rec.s_dept2,
					sizeof(s_rec.s_dept2)-1);
		strcpy( dept1, s_rec.s_dept1 );
		strcpy( dept2, s_rec.s_dept2 );
	}
	fomca1( DEPT1, 19, 0 );
	fomca1( DEPT2, 19, 0 );

	return(retval);
}

GetAreaRange( area1, area2 )
char *area1, *area2;
{
	int	retval=0;

	if((retval=GetPosition(AREACODE))<0){
		return(retval);
	}
	line[AREACODE] = multi_line;
	fomca2( AREA1,1, line[AREACODE], 1 + (40 * column));
	fomca2( AREA1,2, line[AREACODE], 19 + (40 * column));
	fomca2( AREA2,1, line[AREACODE], 30 + (40 * column));
	fomca2( AREA2,2, line[AREACODE], 34 + (40 * column));

	STRCPY( s_rec.s_area1, area1 );
	STRCPY( s_rec.s_area2, area2 );
	fomca1( AREA1, 19, 2 );
	fomca1( AREA2, 19, 2 );
	sr.nextfld = AREA1;
	sr.endfld = AREA2;
	fomud( (char *)&s_rec);
	s_rec.s_area1[0] = LV_CHAR;
	s_rec.s_area2[0] = LV_CHAR;
	retval = ReadFields( AREA1, AREA2 );
	if( retval!=EXIT && retval>=0 ){
		Right_Justify_Numeric(s_rec.s_area1,
					sizeof(s_rec.s_area1)-1);
		Right_Justify_Numeric(s_rec.s_area2,
					sizeof(s_rec.s_area2)-1);
		strcpy( area1, s_rec.s_area1 );
		strcpy( area2, s_rec.s_area2 );
	}
	fomca1( AREA1, 19, 0 );
	fomca1( AREA2, 19, 0 );

	return(retval);

}

GetPosRange( pos1, pos2 )
char *pos1, *pos2;
{
	int	retval=0;

	if((retval=GetPosition(POS))<0){
		return(retval);
	}
	line[POS] = multi_line;
	fomca2( POS1,1, line[POS], 1 +(40 * column)); 
	fomca2( POS1,2, line[POS], 19 + (40 * column));
	fomca2( POS2,1, line[POS], 28 + (40 * column));
	fomca2( POS2,2, line[POS], 32 + (40 * column));

	STRCPY( s_rec.s_pos1, pos1 );
	STRCPY( s_rec.s_pos2, pos2 );
	fomca1( POS1, 19, 2 );
	fomca1( POS2, 19, 2 );
	sr.nextfld = POS1;
	sr.endfld = POS2;
	fomud( (char *)&s_rec);
	s_rec.s_pos1[0] = LV_CHAR;
	s_rec.s_pos2[0] = LV_CHAR;
	retval = ReadFields( POS1, POS2 );
	if( retval!=EXIT && retval>=0 ){
		Right_Justify_Numeric(s_rec.s_pos1,
					sizeof(s_rec.s_pos1)-1);
		Right_Justify_Numeric(s_rec.s_pos2,
					sizeof(s_rec.s_pos2)-1);
		strcpy( pos1, s_rec.s_pos1 );
		strcpy( pos2, s_rec.s_pos2 );
	}
	fomca1( POS1, 19, 0 );
	fomca1( POS2, 19, 0 );

	return(retval);
}

GetClassRange( class1, class2 )
char *class1, *class2;
{
	int	retval=0;

	if((retval=GetPosition(CLASS))<0){
		return(retval);
	}
	line[CLASS] = multi_line;
	fomca2( CLASS1,1, line[CLASS], 1 +(40 * column)); 
	fomca2( CLASS1,2, line[CLASS], 19 + (40 * column));
	fomca2( CLASS2,1, line[CLASS], 28 + (40 * column));
	fomca2( CLASS2,2, line[CLASS], 32 + (40 * column));

	STRCPY( s_rec.s_class1, class1 );
	STRCPY( s_rec.s_class2, class2 );
	fomca1( CLASS1, 19, 2 );
	fomca1( CLASS2, 19, 2 );
	sr.nextfld = CLASS1;
	sr.endfld = CLASS2;
	fomud( (char *)&s_rec);
	s_rec.s_class1[0] = LV_CHAR;
	s_rec.s_class2[0] = LV_CHAR;
	retval = ReadFields( CLASS1, CLASS2 );
	if( retval!=EXIT && retval>=0 ){
		Right_Justify_Numeric(s_rec.s_class1,
					sizeof(s_rec.s_class1)-1);
		Right_Justify_Numeric(s_rec.s_class2,
					sizeof(s_rec.s_class2)-1);
		strcpy( class1, s_rec.s_class1 );
		strcpy( class2, s_rec.s_class2 );
	}
	fomca1( CLASS1, 19, 0 );
	fomca1( CLASS2, 19, 0 );

	return(retval);
}

GetEmpRange( emp1, emp2 )
char *emp1, *emp2;
{
	int	retval=0;

	if (line[EMP] ==0)
		line[EMP] = STARTLINENUM + call_no++;
	fomca2( EMP1,1, line[EMP], 1 );
	fomca2( EMP1,2, line[EMP], 19);
	fomca2( EMP2,1, line[EMP], 35);
	fomca2( EMP2,2, line[EMP], 38);

	STRCPY( s_rec.s_emp1, emp1 );
	STRCPY( s_rec.s_emp2, emp2 );
	fomca1( EMP1, 19, 2 );
	fomca1( EMP2, 19, 2 );
	sr.nextfld = EMP1;
	sr.endfld = EMP2;
	fomud( (char *)&s_rec);
	s_rec.s_emp1[0] = LV_CHAR;
	s_rec.s_emp2[0] = LV_CHAR;
	retval = ReadFields( EMP1, EMP2 );
	if( retval!=EXIT && retval>=0 ){
		Right_Justify_Numeric(s_rec.s_emp1,
					sizeof(s_rec.s_emp1)-1);
		Right_Justify_Numeric(s_rec.s_emp2,
					sizeof(s_rec.s_emp2)-1); 
		strcpy( emp1, s_rec.s_emp1 );
		strcpy( emp2, s_rec.s_emp2 );
	}
	fomca1( EMP1, 19, 0 );
	fomca1( EMP2, 19, 0 );

	return(retval);
}
GetAge( age )
short *age;
{
	int	retval=0;

	if (line[AGE] ==0)
		line[AGE] = STARTLINENUM + call_no++;
#ifdef ENGLISH
	fomer("Enter the Selected Age ");
#else
	fomer("TRANSLATE");
#endif
	fomca2( AGE1,1, line[AGE], 1 );
	fomca2( AGE1,2, line[AGE], 19);

	s_rec.s_age =  *age ;
	fomca1( AGE1, 19, 2 );
	sr.nextfld = AGE1;
	sr.endfld = AGE1;
	fomud( (char *)&s_rec);
	s_rec.s_age = LV_SHORT;
	retval = ReadFields( AGE1, AGE1 );
	if( retval!=EXIT && retval>=0 ){
		*age = s_rec.s_age ;
	}
	fomca1( AGE1, 19, 0 );

	return(retval);
}

GetAnniv( anniv )
short *anniv;
{
	int	retval=0;

	if (line[ANNIV] ==0)
		line[ANNIV] = STARTLINENUM + call_no++;
#ifdef ENGLISH
	fomer("Enter the Selected Anniv. Month or Enter 99 for All Months");
#else
	fomer("TRANSLATE");
#endif
	fomca2( ANNIV1,1, line[ANNIV], 1 );
	fomca2( ANNIV1,2, line[ANNIV], 19);

	s_rec.s_anniv =  *anniv ;
	fomca1( ANNIV1, 19, 2 );
	sr.nextfld = ANNIV1;
	sr.endfld = ANNIV1;
	fomud( (char *)&s_rec);
	s_rec.s_anniv = LV_SHORT;
	retval = ReadFields( ANNIV1, ANNIV1 );
	if( retval!=EXIT && retval>=0 ){
		*anniv = s_rec.s_anniv ;
	}
	fomca1( ANNIV1, 19, 0 );

	return(retval);
}

GetDateRange( date1, date2 )
long *date1, *date2;
{
	int	retval=0;

	if (line[DAT] ==0)
		line[DAT] = STARTLINENUM + call_no++;
	fomca2( DATE1,1, line[DAT], 1 + (40 * column));
	fomca2( DATE1,2, line[DAT], 19 + (40 * column));
	fomca2( DATE2,1, line[DAT], 35 + (40 * column));
	fomca2( DATE2,2, line[DAT], 38 + (40 * column));

	s_rec.s_date1 = *date1 ;
	s_rec.s_date2 = *date2 ;
	fomca1( DATE1, 19, 2 );
	fomca1( DATE2, 19, 2 );
	sr.nextfld = DATE1;
	sr.endfld = DATE2;
	fomud( (char *)&s_rec);
	s_rec.s_date1 = LV_LONG;
	s_rec.s_date2 = LV_LONG;
	retval = ReadFields( DATE1, DATE2 );
	if( retval!=EXIT && retval>=0 ){
		*date1 = s_rec.s_date1 ;
		*date2 = s_rec.s_date2 ;
	}
	/*fomca1( DATE1, 19, 0 );
	fomca1( DATE2, 19, 0 );
*/
	return(retval);
}

GetDate1( date1 )
long *date1;
{
	int	retval=0;

	if(retval=GetPosition(DAT)<0){ 
		return(retval);
	}
	line[DAT] = multi_line;
	fomca2( DATE1,1, line[DAT], 1 + (42 * column));
	fomca2( DATE1,2, line[DAT], 19 + (40 * column ));

	s_rec.s_date1 = *date1;
	fomca1( DATE1, 19, 2 );
	sr.nextfld = DATE1;
	sr.endfld = DATE1;
	fomud( (char *)&s_rec);
	s_rec.s_date1 = LV_LONG; 
	retval = ReadFields( DATE1, DATE1 );
	if( retval!=EXIT && retval>=0 ){
		*date1 = s_rec.s_date1; 
	}
	fomca1( DATE1, 19, 0 );

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
	if(FillSortop1(HIGH)<0)
		return(-1);
	if(FillSortop2(HIGH)<0)
		return(-1);
	if(FillBarg(HIGH,HIGH)<0)
		return(-1);
	if(FillCenter(HIGH,HIGH)<0)
		return(-1);
	if(FillDept(HIGH,HIGH)<0)
		return(-1);
	if(FillArea(HIGH,HIGH)<0)
		return(-1);
	if(FillPosition(HIGH,HIGH)<0)
		return(-1);
	if(FillClass(HIGH,HIGH)<0)
		return(-1);
	if(FillEmployee(HIGH,HIGH)<0 )
		return(-1);
	if(FillDate(HIGH,HIGH)<0 )
		return(-1);
	if(FillAge(HIGH)<0 )
		return(-1);
	if(FillAnniv(HIGH)<0 )
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
		case SORTOP1:	/* sort option one */
			if(sensort_call == 0) {
				if ((s_rec.s_sortop1[0] != ALPHSORT) &&
			   	(s_rec.s_sortop1[0] != NUMSORT)){
#ifdef ENGLISH
					fomer ("(A)lpha, (N)umeric");
#else
					fomer ("TRANSLATE");
#endif
					s_rec.s_sortop1[0] = LV_CHAR;
				}
			}
			else {
				if ((s_rec.s_sortop1[0] != ALPHSORT) &&
			   	(s_rec.s_sortop1[0] != NUMSORT) &&
			   	(s_rec.s_sortop1[0] != SENSORT1) &&
				(s_rec.s_sortop1[0] != SENSORT2)){
#ifdef ENGLISH
					fomer(
				       	   "(A)lpha, (N)umeric, (D)escending, (X) - Ascending");
#else
					fomer ("TRANSLATE");
#endif
					s_rec.s_sortop1[0] = LV_CHAR;
				}
			}

			break;
		case SORTOP2:	/* sort option one */
			if ((s_rec.s_sortop2[0] != BARGSORT) &&
			   (s_rec.s_sortop2[0] != CENTERSORT) &&
			   (s_rec.s_sortop2[0] != POSISORT) &&
			   (s_rec.s_sortop2[0] != CLASSSORT)){
#ifdef ENGLISH
				fomer ("(B)arg, (C)lass, (P)osition, (L)ocation");
#else
				fomer ("TRANSLATE");
#endif
				s_rec.s_sortop2[0] = LV_CHAR;
			}
			break;
		case BARG1:      /* starting Bargaining unit */
			Right_Justify_Numeric(s_rec.s_barg1,
						sizeof(s_rec.s_barg1)-1);
			break;
		case BARG2:      /* ending Bargaining unit */
			Right_Justify_Numeric(s_rec.s_barg2,
						sizeof(s_rec.s_barg2)-1);
			if(strcmp(s_rec.s_barg2,s_rec.s_barg1 ) <0){
#ifdef ENGLISH
				fomer("Ending Bargaining Unit can't be less than starting Bargaining Unit");
#else
				fomer("TRANSLATE");
#endif
				s_rec.s_barg2[0] = LV_CHAR;
			}
			break;
		case POS1:      /* starting Bargaining unit */
			Right_Justify_Numeric(s_rec.s_pos1,
						sizeof(s_rec.s_pos1)-1);
			break;
		case POS2:      /* ending Bargaining unit */
			Right_Justify_Numeric(s_rec.s_pos2,
						sizeof(s_rec.s_pos2)-1);
			if(strcmp(s_rec.s_pos2,s_rec.s_pos1 ) <0){
#ifdef ENGLISH
				fomer("Ending Position Code  can't be less than starting Position Code");
#else
				fomer("TRANSLATE");
#endif
				s_rec.s_pos2[0] = LV_CHAR;
			}
			break;
		case CLASS1:      /* starting Class code */
			Right_Justify_Numeric(s_rec.s_class1,
						sizeof(s_rec.s_class1)-1);
			break;
		case CLASS2:      /* ending Class code */
			Right_Justify_Numeric(s_rec.s_class2,
						sizeof(s_rec.s_class2)-1);
			if(strcmp(s_rec.s_class2,s_rec.s_class1 ) <0){
#ifdef ENGLISH
				fomer("Ending Class Code can't be less than starting Class Code");
#else
				fomer("TRANSLATE");
#endif
				s_rec.s_class2[0] = LV_CHAR;
			}
			break;
		case EMP1:      /* starting Bargaining unit */
			Right_Justify_Numeric(s_rec.s_emp1,
						sizeof(s_rec.s_emp1)-1);
			break;
		case EMP2:      /* ending Bargaining unit */
			Right_Justify_Numeric(s_rec.s_emp2,
						sizeof(s_rec.s_emp2)-1);
			if(strcmp(s_rec.s_dept2,s_rec.s_dept1 ) <0){
#ifdef ENGLISH
				fomer("Ending Employee Number  can't be less than starting Employee Number");
#else
				fomer("TRANSLATE");
#endif
				s_rec.s_emp2[0] = LV_CHAR;
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
GetSortop1( sortop1 )
char	*sortop1;
{
	int	retval;

	sensort_call = 0;
#ifdef ENGLISH
	fomer("A(lpha), N(umeric)");
#else
	fomer("TRANSLATE");
#endif
	STRCPY( s_rec.s_sortop1, sortop1 );
	fomca1( SORTOP1, 19, 2 );
	sr.nextfld = SORTOP1;
	sr.endfld = SORTOP1;
	fomud( (char *)&s_rec);
	s_rec.s_sortop1[0] = LV_CHAR;
	retval = ReadFields(SORTOP1,SORTOP1);
	if( retval!=EXIT && retval>=0 )
		*sortop1 = s_rec.s_sortop1[0];
	fomca1( SORTOP1, 19, 0 );
	return( retval );
}
GetSortop3( sortop1 )
char	*sortop1;
{
	int	retval;

	sensort_call = 1;
#ifdef ENGLISH
	fomer("A(lpha), N(umeric), (D)escending, (X) - Ascending");
#else
	fomer("TRANSLATE");
#endif
	STRCPY( s_rec.s_sortop1, sortop1 );
	fomca1( SORTOP1, 19, 2 );
	sr.nextfld = SORTOP1;
	sr.endfld = SORTOP1;
	fomud( (char *)&s_rec);
	s_rec.s_sortop1[0] = LV_CHAR;
	retval = ReadFields(SORTOP1,SORTOP1);
	if( retval!=EXIT && retval>=0 )
		*sortop1 = s_rec.s_sortop1[0];
	fomca1( SORTOP1, 19, 0 );
	return( retval );
}
GetSortop2( sortop2 )
char	*sortop2;
{
	int	retval;

#ifdef ENGLISH
	fomer("(B)argain, (C)lass, (P)osition, (L)ocation");
#else
	fomer("TRANSLATE");
#endif
	STRCPY( s_rec.s_sortop2, sortop2 );
	fomca1( SORTOP2, 19, 2 );
	sr.nextfld = SORTOP2;
	sr.endfld = SORTOP2;
	fomud( (char *)&s_rec);
	s_rec.s_sortop2[0] = LV_CHAR;
	retval = ReadFields(SORTOP2,SORTOP2);
	if( retval!=EXIT && retval>=0 )
		*sortop2 = s_rec.s_sortop2[0];
	fomca1( SORTOP2, 19, 0 );
	return( retval );
}

GetPosition(field)
int	field;
{
/***********************
	if(line[field] == 0) {
			column = 0;
			multi_line = STARTLINENUM + call_no++;
	}
	else {
		column = 1;
		multi_line = line[field];
	}
	if all fields float between lines and columns
	uncomment this section and comment out the switch 

****************************/ 

	if(column == 1) {
		multi_line = 0;
	}
	if(multi_line == 0) {
		column = 0;
		multi_line = STARTLINENUM + call_no++;
	}
	else {
		column = 1;
	}
	return(0);
}
/*-----------------------------------------------------------------------*/

BuildTmpSched (sortop1,sortop2)
char	*sortop1;
char	*sortop2;
{
	char	txt_line[132];

	Emp	emp_rec;
	Tmp_sched1 tmp_sched1;

	emp_rec.em_numb[0] = '\0';

	flg_reset(EMPLOYEE);
	for (;;){
		
		/*Get the emp name from the employee file*/

		retval = get_n_employee(&emp_rec,BROWSE,0,FORWARD,e_mesg);
		if(retval==EFL)	break;
		if(retval<0){
			return(-1);	
		}
	  	retval=UsrBargVal(BROWSE,emp_rec.em_numb,emp_rec.em_barg,0,
									e_mesg);
	  	if(retval < 0)
			continue;

		strcpy(tmp_sched1.tm_numb,emp_rec.em_numb);
		strcpy(tmp_sched1.tm_class,emp_rec.em_class);
		tmp_sched1.tm_cost = emp_rec.em_cc;
		tmp_sched1.tm_dept[0] = '\0';
		tmp_sched1.tm_area[0] = '\0';

		sprintf(txt_line,"%d",emp_rec.em_cc);
		strcpy(tmp_sched1.tm_sortk_1,
			txt_line);

		if(sortop1[0] == 'A'){
			sprintf(txt_line,"%s%s",
				emp_rec.em_last_name,
				emp_rec.em_first_name);
			strcpy(tmp_sched1.tm_sortk_2,txt_line);
			strcpy(tmp_sched1.tm_sortk_3,emp_rec.em_numb);
		}
		else{
			strcpy(tmp_sched1.tm_sortk_2,emp_rec.em_numb);
			sprintf(txt_line,"%s%s",
				emp_rec.em_last_name,
				emp_rec.em_first_name);
			strcpy(tmp_sched1.tm_sortk_3,txt_line);
		}
		
		retval = put_tmp_sched1(&tmp_sched1,ADD,e_mesg);
		if(retval < 0){
			roll_back(e_mesg);
			return(-1);
		}
		commit(e_mesg);
	}
	seq_over(EMPLOYEE);
	return(0);
}
/*-----------------------------------------------------------------------*/
CreatTempIndx ( tempindxnm,sortop1,sortop2 )
char	*tempindxnm;	/*temparary index file name*/
char	*sortop1;
char	*sortop2;
{
	int	keysarray[20];
	int	step=0;
	int	use_file=0;

	Emp		emp_rec;
	/*
	******************** Sort employee File *************************
	*/
	switch (sortop2[0]){
		case 'B':	/*Name within Bargain unit key*/
			if(sortop1[0]=='A')
				keysarray[step++] = 4;	/*Parts*/
			else	
				keysarray[step++] = 2;


			/*Part 1*/
			keysarray[step++] = CHAR;
			keysarray[step++] =  6;
			keysarray[step++] = (char*)&emp_rec.em_barg[0] - (char*)&emp_rec;
			keysarray[step++] = ASCND;

			if(sortop1[0]=='A'){
			/*Part 2*/
				keysarray[step++] = CHAR;
				keysarray[step++] =  25;
				keysarray[step++] = (char*)&emp_rec.em_last_name[0] - (char*)&emp_rec;
				keysarray[step++] = ASCND;
	
				/*Part 3*/
				keysarray[step++] = CHAR;
				keysarray[step++] =  15;
				keysarray[step++] = (char*)&emp_rec.em_first_name[0] - (char*)&emp_rec;
				keysarray[step++] = ASCND;
			}

	
			/*Part 4*/
			keysarray[step++] = CHAR;
			keysarray[step++] =  12;
			keysarray[step++] = (char*)&emp_rec.em_numb[0] - (char*)&emp_rec;
			keysarray[step++] = ASCND;
			break;

		case 'P':	/*Name within Position code key*/

			if(sortop1[0]=='A')
				keysarray[step++] = 4;	/*Parts*/
			else
				keysarray[step++] = 2;


			/*Part 1*/
			keysarray[step++] = CHAR;
			keysarray[step++] =  6;
			keysarray[step++] = (char*)&emp_rec.em_pos[0] - (char*)&emp_rec;
			keysarray[step++] = ASCND;

			if(sortop1[0]=='A'){
				/*Part 2*/
				keysarray[step++] = CHAR;
				keysarray[step++] =  25;
				keysarray[step++] = (char*)&emp_rec.em_last_name[0] - (char*)&emp_rec;
				keysarray[step++] = ASCND;

				/*Part 3*/
				keysarray[step++] = CHAR;
				keysarray[step++] =  15;
				keysarray[step++] = (char*)&emp_rec.em_first_name[0] - (char*)&emp_rec;
				keysarray[step++] = ASCND;
			}

			/*Part 4*/
			keysarray[step++] = CHAR;
			keysarray[step++] =  12;
			keysarray[step++] = (char*)&emp_rec.em_numb[0] - (char*)&emp_rec;
			keysarray[step++] = ASCND;
			break;
		default: 
			break;
	}/*end switch*/

	retval = CrtTmpIndx(EMPLOYEE,TMPINDX_1,keysarray,tempindxnm,
		(int(*)())NULL,e_mesg);
	return(0);
}
