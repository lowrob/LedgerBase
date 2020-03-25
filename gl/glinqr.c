/******************************************************************************
		Sourcename    : glinqr.c
		System        : Budgetary Financial system.
		Module        : General Ledger system inquiry.
		Created on    : 89-05-02
		Created  By   : K HARISH.
		Cobol sources : gl030f-03 & gl033f--00
*******************************************************************************
About the program:

	This program displays/prints out General Ledger master records.

	1.Display on the screen:
		The program has to be invoked with switch s1 ON for display
	on the screen.
		It accepts the key values for the gl master record, validates
	and displays if on the screen in two parts. The first part or first 
	screen displays the codes and keys, and the second part or second
	screen displays the period-wise budgets for the current year and the 
	previous year.

	2.Listing on the printer:
		The program has to be called with no switches ON.
		It displays a menu on the screen consisting of the following
	options.
		1.Full record complete listing
		2.Full record partial listing
		3.Codes and keys complete listing
		4.Codes and keys partial listing
		5.Balances complete listing
		6.Balances partial listing
		7.End
	For options 1,3 and 5 , all the records in the GL master are printed 
	out. For options 2, 4 and 6, the program accepts the range of records
	to be printed from the user, in the form of starting record keys and
	ending record keys.

HISTORY:
 Date           Programmer     Description of modification
____/__/__      __________     ___________________________

1990/11/21	J.Prescott	Change program to display 
				"GENERAL LEDGER INQUIRY"  if to screen
				"GENERAL LEDGER LISTING"  if to printer.
				Also Removed Displaying CR after amounts.

1991/02/06 	P.Ralph		Default fund to 1 & record code to 99

1993/10/13	L.Robichaud	Pass new parameter to close_rep function for 
				a banner to print with 3000 family.
******************************************************************************/
#include <stdio.h>

#define MAIN
#define MAINFL	GLMAST

#include <reports.h>
#include <ctype.h>
#include <cfomstrc.h>

#define SCREEN_NAME  "glinqr"

#define HIGH 		1
#define LOW 		0
#define YES 		10
#define NO 		11
#define CONTINUE 	12
#define EXIT 		13

#ifdef ENGLISH
#define COD_KEY_HDG 	"LIST OF CODES AND KEYS"
#define BALANCE_HDG 	"LIST OF BALANCES FOR THE PERIOD"
#define COMP_HDG 	"LIST OF COMPLETE MASTER RECORDS"
#else
#define COD_KEY_HDG 	"LISTE DES CODES ET CLES"
#define BALANCE_HDG 	"LISTE DES BALANCES POUR LA PERIODE"
#define COMP_HDG 	"LISTE DES FICHES MAITRES COMPLETES"
#endif

#define PATH_FILE_SIZE 	50

#define SYSTEM		"GENERAL LEDGER"
#define MOD_DATE	"21-NOV-90"

/* glinqr.sth - header for C structure generated by PROFOM EDITOR */

struct	in_struct	{

	char	s_progname[11];	/* STRING XXXXXXXXXX field 100 */
	char	s_title[23];	/* STRING X(22) field 200 */
	long	s_rundt;	/* DATE YYYYFMMFDD field 300 */
	char	s_end_prog[2];	/* STRING X field 400 */
	char	s_rec_compl[2];	/* STRING X field 500 */
	char	s_rec_partl[2];	/* STRING X field 550 */
	char	s_code_compl[2];	/* STRING X field 600 */
	char	s_code_partl[2];	/* STRING X field 650 */
	char	s_bal_compl[2];	/* STRING X field 700 */
	char	s_bal_partl[2];	/* STRING X field 750 */
	char	s_listopt[2];	/* STRING X field 800 */
	short	s_copies;	/* NUMERIC 9 field 850 */
	char	s_keyhdng[33];	/* STRING X field 900 */
	short	s_st_fund;	/* NUMERIC 999 field 1000 */
	char	s_st_acct[19];	/* STRING XXXXXXXXXXXXXXXX field 1100 */
	short	s_st_code;	/* NUMERIC 99 field 1200 */
	short	s_end_fund;	/* NUMERIC 999 field 1300 */
	char	s_end_acct[19];	/* STRING XXXXXXXXXXXXXXXX field 1400 */
	short	s_end_code;	/* NUMERIC 99 field 1500 */
	short	s_period;	/* NUMERIC 99 field 1600 */
	int	s_confirm;	/* YES/NO B field 1700 */
	};

static struct in_struct s_rec;
static Gl_rec gl_rec;
static Ctl_rec cntrl_rec;
struct stat_rec sr;

long get_date();
static int retval;	/* Global variable to store function values */
static int list;	/* Flag to indicate complete or partial listing */ 
static Pa_rec param_rec; /* to store the name from parameter file */
char e_mesg[80]; 	/* to store error messages */
static short pgcnt, copies;	/* to hold page number and no of copies */

/* Initialize profom fields, call menu & inquiry procedures */
main( argc, argv )
int argc;
char *argv[];
{
	strncpy( SYS_NAME, SYSTEM, 50 ); /* Module name */
	strncpy( CHNG_DATE, MOD_DATE, 10 );/* Last date of change */
	
	/* process the switches */
	proc_switch( argc,argv,MAINFL );

	if( Initialize(terminal)<0 )
		exit(-1);

	if( process()<0 ){	/* accept and process user's input */
		fomcs();
		fomrt();
		close_dbh();
		exit(-1);
	}

	/* clear and exit the screen , close files & exit program */
	fomcs();
	fomrt();
	close_dbh();
	exit(0);
}

static
Initialize(trml)
char *trml;
{
	/* initialize the profom screen variables */
	STRCPY( sr.scrnam, NFM_PATH );
	strcat( sr.scrnam, SCREEN_NAME );
	STRCPY( sr.termnm, trml );

	/* initialize the fields */
	if( in_scrhdg()<0 )
		exit(-1);
	if( in_listmenu( HIGH )<0 )
		exit(-1);
	if( in_copies( HIGH )<0 )
		exit(-1);
	if( in_keyhdng( HIGH )<0 )
		exit(-1);
	if( in_stflds( HIGH )<0 )
		exit(-1);
	if( in_endflds( HIGH )<0 )
		exit(-1);
	if( in_period( HIGH )<0 )
		exit(-1);
	if( in_confirm( HIGH )<0 )
		exit(-1);

	if( init_profom()<0 ) 	/* initialize profom screen */
		exit(-1);

	return(0);
}

/* initialize the profom screen */
static
init_profom()
{
	fomin( &sr );		/* initialize the screen */
	ret( err_chk(&sr) );	/* if profom error return */

	sr.nextfld = 1;
	sr.endfld = 0;
	fomwr( (char *)&s_rec );
	ret( err_chk(&sr) );
	fomcf(1,1);

	return(0);
}

/* Fill the screen heading fields, the program name and the date */
static
in_scrhdg()
{
	STRCPY( s_rec.s_progname, PROG_NAME );
	s_rec.s_rundt = get_date();
	if(SW1)
#ifdef ENGLISH
	 	STRCPY(s_rec.s_title,"GENERAL LEDGER INQUIRY");
#else
		STRCPY(s_rec.s_title," INTERROGATION AU G/L ");
#endif
             
	else
#ifdef ENGLISH
		STRCPY(s_rec.s_title,"GENERAL LEDGER LISTING");
#else
		STRCPY(s_rec.s_title," LISTE DU GRAND LIVRE ");
#endif

	return(0);
}
/* 	Read control file */
static
rd_cntrl()
{
	cntrl_rec.fund = gl_rec.funds;
	retval = get_ctl( &cntrl_rec, BROWSE, 0, e_mesg );
	if( retval==ERROR )
		return(-1);
	if(retval!=NOERROR )
		STRCPY(param_rec.pa_co_name,"\0");
	else
		STRCPY(param_rec.pa_co_name,cntrl_rec.desc);

	return(0);
}
/* Read the gl master file */
static
rd_glmast()
{
	gl_rec.funds = (short)s_rec.s_st_fund;
	STRCPY( gl_rec.accno, s_rec.s_st_acct );
	gl_rec.reccod = (short)s_rec.s_st_code;
	retval = get_gl( &gl_rec,BROWSE,0,e_mesg );
	return(retval);
}
/* initialize the listing inquiry part of the menu */
static
in_listmenu( value )
short value;
{
	if( value==HIGH ){
		s_rec.s_end_prog[0] = HV_CHAR;
		s_rec.s_rec_compl[0] = HV_CHAR;
		s_rec.s_rec_partl[0] = HV_CHAR;
		s_rec.s_code_compl[0] = HV_CHAR;
		s_rec.s_code_partl[0] = HV_CHAR;
		s_rec.s_bal_compl[0] = HV_CHAR;
		s_rec.s_bal_partl[0] = HV_CHAR;
		s_rec.s_listopt[0] = HV_CHAR;
	}
	else{
		s_rec.s_end_prog[0] = ' ';
		s_rec.s_rec_compl[0] = ' ';
		s_rec.s_rec_partl[0] = ' ';
		s_rec.s_code_compl[0] = ' ';
		s_rec.s_code_partl[0] = ' ';
		s_rec.s_bal_compl[0] = ' ';
		s_rec.s_bal_partl[0] = ' ';
		s_rec.s_listopt[0] = LV_CHAR;
	}
	return(0);
}
/* initialize the  prntr no field */
static
in_copies(value)
int value;
{
	if(value==HIGH)
		s_rec.s_copies = HV_SHORT;
	else
		s_rec.s_copies = LV_SHORT;
	return(0);
}
/* initialize the  heading fields for the key values accepted */
static
in_keyhdng( value )
short value;
{
	if( value==HIGH )
		s_rec.s_keyhdng[0] = HV_CHAR;
	else
		s_rec.s_keyhdng[0] = LV_CHAR;
	return(0);
}
/* initialize the  fields for the starting key values accepted */
static
in_stflds( value )
short value;
{
	if( value==HIGH ){
		s_rec.s_st_fund = HV_SHORT;
		s_rec.s_st_acct[0] = HV_CHAR;
		s_rec.s_st_code = HV_SHORT;
	}
	else{
		s_rec.s_st_fund = LV_SHORT;
		s_rec.s_st_acct[0] = LV_CHAR;
		s_rec.s_st_code = LV_SHORT;
#ifdef ENGLISH
	fomer("Press ESC-F  to Exit");
#else 
	fomer("Appuyer sur ESC-F pour sortir");
#endif
	}
	return(0);
}
/* initialize the  fields for the ending key values accepted */
static
in_endflds( value )
short value;
{
	if( value==HIGH ){
		s_rec.s_end_fund = HV_SHORT;
		s_rec.s_end_acct[0] = HV_CHAR;
		s_rec.s_end_code = HV_SHORT;
	}
	else{
		s_rec.s_end_fund = LV_SHORT;
		s_rec.s_end_acct[0] = LV_CHAR;
		s_rec.s_end_code = LV_SHORT;
	}
	return(0);
}
/* initialize the period field */
static
in_period( value )
short value;
{
	if( value==HIGH )
		s_rec.s_period = HV_SHORT;
	else
		s_rec.s_period = LV_SHORT;
	return(0);
}
/* initialize the confirmation message field */
/* it confirms if the data entered by the user is correct */
static
in_confirm( value )
short value;
{
	if( value==HIGH )
		s_rec.s_confirm = HV_INT;
	else
		s_rec.s_confirm = LV_INT;
	return(0);
}
/* check for switches and branch to the relevant process */
process()
{
	retval = get_param( &param_rec, BROWSE, 1, e_mesg );
	if( retval!=1 ){
		fomen(e_mesg);
		get();
		return(-1);
	}
	/* if SW1 switch on, call screen inquiry routine */
	if( SW1 ){
		if( init_prnt("D")<0 )	/* always to screen */
			return(-1);
		retval = inq_scr();
#ifndef	SPOOLER
		if(pgcnt)
			rite_top() ;
#endif
		close_rep(BANNER);
		return( retval );
	}
	else {	/* SW1 not on call listing inquiry routine */
		retval = inq_list();
		return( retval );
	}
}

/* Screen inquiry routine */
static
inq_scr()
{
	/* Do the following in a loop until user opts to quit */
	for( ; ; ){
		/* Accept the key values of the record to be displayed */
		if( (retval=scr_menu())<0 )
			return(-1);
		if( retval==EXIT )
			break;
		/* if SW7 is on, read the sp_file & fill parameter name */
		if( SW7 ){
			if( sp_to_param()<0 )
				return(-1);
		}
		/* Read the glmaster and get the record */
		retval = rd_glmast();
		switch( retval ){
			case UNDEF:
				if( backtomenu()<0 )
					return(-1);
				continue;
			case ERROR:
				return(-1);
		}
		if( SW7 )
			if( rd_cntrl()<0 )
				return(-1);
		fomcs();
		fomrt();
		/* Display the records on the screen */
		retval=displ_rec();
		/* If no error, if user wishes to quit, break out of loop */
		if (retval == 0){
			if (backtomenu() <0) return(-1);
		}
		if( retval==EXIT ){
			if( backtomenu()<0 )
				return(-1);
		}
		else if( retval<0 )
			return( retval );
	}
	return(0);
}

static
backtomenu()
{
	fomst();
	sr.nextfld = 1;
	sr.endfld = 0;
	fomwr( (char *)&s_rec );
	ret( err_chk(&sr) );

	fflush(stdout);

#ifdef ENGLISH
	fomen("Press ESC-F  to Exit");
#else 
	fomen("Appuyer sur ESC-F pour sortir");
#endif
	fomwr( (char *)&s_rec );
	ret( err_chk(&sr) );
	return(0);
}

/* Read the sp file on entered key values and copy name to param name rec */
static
sp_to_param()
{
	return(0);
}

/* Display options and accept input */
static
scr_menu()
{
	/* If SW7 or SW-CIE in cobol is on, display different heading */
	if( SW7 )
#ifdef ENGLISH
		STRCPY( s_rec.s_keyhdng, "   CO       ACCT NUMBER   CODE" );
	else
		STRCPY( s_rec.s_keyhdng, "  FUND      ACCT NUMBER   CODE" );
#else
		STRCPY( s_rec.s_keyhdng, "   CIE      # DE COMPTE   CODE" );
	else
		STRCPY( s_rec.s_keyhdng, "  FOND      # DE COMPTE   CODE" );
#endif

	for( ; ; ){
#ifdef ENGLISH
	fomer("Press ESC-F  to Exit");
#else 
	fomer("Appuyer sur ESC-F pour sortir");
#endif
		/* Accept key values after validating */
		if( (retval=get_key())<0 )
			return(-1);
		if( retval==EXIT )
			return(EXIT);
#ifdef ENGLISH
	fomer("Press ESC-F  to Exit");
#else 
	fomer("Appuyer sur ESC-F pour sortir");
#endif
		/* Let user confirm the entry */
		if( (retval = confirm())<0 )
			return(-1);
		if( retval==YES )
			break;
	}

	if( in_confirm( HIGH )<0 )
		return(-1);

	return(0);
}

/* Get the key values for the first record to be shown */
static
get_key()
{
	/* move low-values to the fields */

	if( in_stflds( LOW )<0 ) 
		return(-1);

	/* set up default start fund and record code */
	fomca1(1000,19,2);
	fomca1(1200,19,2);
	fomud((char *)&s_rec);
	s_rec.s_st_fund = 1;
	s_rec.s_st_code = 99;	
	sr.nextfld = 900;
	sr.endfld = 1200;
	fomwr((char *)&s_rec);	
	s_rec.s_st_fund = LV_SHORT;
	s_rec.s_st_code = LV_SHORT;	

	sr.nextfld = 1000;
	sr.endfld = 1200;

	/* Read the input and validate */
	for( ; ; ){
		fomrd( (char *)&s_rec );
		ret( err_chk(&sr) ); 
		if( sr.retcode==RET_USER_ESC ){
			if(sr.escchar[0]=='F'||sr.escchar[0]=='f')
				return( EXIT );
			else
				continue;
		}
		if( sr.retcode==RET_VAL_CHK ){
			validate();
			continue;
		}
		break;
	}
	return(0);
}

/* Get the key values for the last record to be shown */
static
get_end_key()
{
	/* move low-values to the fields */
	if( in_endflds( LOW )<0 ) 
		return(-1);

	/* set up default start fund and record code */
	fomca1(1300,19,2);
	fomca1(1500,19,2);
	fomud((char *)&s_rec);
	s_rec.s_end_fund = 1;
	s_rec.s_end_code = 99;	
	sr.nextfld = 1200;
	sr.endfld = 1500;
	fomwr((char *)&s_rec);	
	s_rec.s_end_fund = LV_SHORT;
	s_rec.s_end_code = LV_SHORT;	

	sr.nextfld = 1300;
	sr.endfld = 1500;

	/* Read the input and validate */
	for( ; ; ){
#ifdef ENGLISH
	fomer("Press ESC-F  to Exit");
#else 
	fomer("Appuyer sur ESC-F pour sortir");
#endif
		fomrd( (char *)&s_rec );
		ret( err_chk(&sr) ); 
		if( sr.retcode==RET_USER_ESC ){
			if(sr.escchar[0]=='F'||sr.escchar[0]=='f')
				return( EXIT );
			else
				continue;
		}
		if( sr.retcode==RET_VAL_CHK ){
			validate();
			continue;
		}
		break;
	}
	return(0);
}

/* Does the user confirm the data entered ? */
static
confirm()
{
	sr.nextfld = 1700; /* input confirmation field */
	fomrf( (char *)&s_rec );
	ret( err_chk(&sr) ); 

	if( s_rec.s_confirm )
		return(YES);
	else 
		return(NO);
}

/* Validate the data entered */
static
validate()
{
	short i;
	char *str;

	switch( sr.curfld ){
	  case 1100:
		str = s_rec.s_st_acct;
		break;
	  case 1400:
		str = s_rec.s_end_acct;
		break;
	  default :
		break;
	}

	switch( sr.curfld ){
	  case 1100:
	  case 1400:
		for(i = 0 ; i < 18 && str[i] != '\0' ; i++)
			if( !isdigit(str[i]) ) {
#ifdef	ENGLISH
				fomer("Invalid Account Number");
#else
				fomer("Numero de compte invalid");
#endif
				str[0] = LV_CHAR ;
				break;
			/* return(ERROR) ; */
			}

		if( !i ) {	/* (No Characters) Account# = 0 */
#ifdef	ENGLISH
			fomer("Invalid Account Number");
#else
			fomer("Numero de compte invalid");
#endif
			str[0] = LV_CHAR ;
				break;
			/* return(ERROR) ; */
		}
		/* Right Justify the account number */
		sprintf(e_mesg,"%18.18s",str);
		strcpy(str,e_mesg);
		break ;
	}
	sr.nextfld = sr.curfld;
	return(0);
}

/* Listing enquiry routine */
static
inq_list()
{
	/* Get user's option and list out records until user opts to quit */
	for( ; ; ){
		if( ( retval=list_menu() )<0 )
			return(-1);
		if( retval==EXIT )
			return(0);
		if( init_prnt("P")<0 )	/* always to printer */
			return(-1);
		/* use retval to determine record reading */
		if( prnt_records()<0 )
			return(-1);
#ifndef	SPOOLER
		if(pgcnt)
			rite_top() ;
#endif
		if( close_rep(BANNER)<0 )
			return(-1);
	}
}

/* Read the user's input and validate until user confirms */
static
list_menu()
{
	retval = 0; 

	for( ; ; ){
		if( in_keyhdng(HIGH)<0 )
			return(-1);
		if( in_stflds(HIGH)<0 )
			return(-1);
		if( in_endflds(HIGH)<0 )
			return(-1);
		if( in_copies( HIGH )<0 )
			return(-1);
		if( in_period(HIGH)<0 )
			return(-1);
		if( in_confirm(HIGH)<0 )
			return(-1);
		sr.nextfld = 400;
		sr.endfld = 1700;
		fomwr( (char *)&s_rec );
		ret(err_chk(&sr));
		/* Display menu of 7 listing options */
		if( in_listmenu( LOW )<0 )
			return(-1);
		sr.nextfld = 400;
		sr.endfld = 750;
		fomwr( (char *)&s_rec );
		ret(err_chk(&sr));

		/* Read input until valid option is entered */
		for( ; ; ){
			sr.nextfld = 800;
			fomrf( (char *)&s_rec );
			ret( err_chk(&sr) );
			if( s_rec.s_listopt[0]>='0' && s_rec.s_listopt[0]<='6' )
				break;
		}
		switch( s_rec.s_listopt[0] ){
			case '2':
			case '4':
			case '6':
				/* partial list, take range & let confirm */
				if( (retval = get_lst_extent() )<0 )
					return(-1);
				if( retval==EXIT )
					retval=CONTINUE; 
				break;
			case '0':
				return( EXIT );
			default:
				break;
		}
		if(retval==CONTINUE ){
		         retval = 0;
			 continue;
		}
		if( s_rec.s_listopt[0]=='5' || s_rec.s_listopt[0]=='6' )
			if( rd_period()<0 )
				return(-1);

		if( rd_copies()<0 )
			return(-1);
		copies = s_rec.s_copies;
		if( ( retval=confirm() )<0 )
			return(-1);

		if( retval==YES  )
			break;
	}
	return(retval);
}

static
rd_copies()
{
	
	s_rec.s_copies = 1;
	fomca1( 850, 19, 2 );
	sr.nextfld = 850 ;
	sr.endfld = 850 ;
	fomud( (char *)&s_rec);
	s_rec.s_copies = LV_SHORT;
	fomrf( (char *)&s_rec );
	ret(err_chk(&sr));

	fomca1( 850, 19, 0 );
	return(0);
}

/* Read the period for which the list of balances is to be printed */
static
rd_period()
{
	if( in_period( LOW )<0 )
		return(-1);
	for( ; ; ){
		sr.nextfld = 1600;
		fomrf( (char *)&s_rec );
		ret( err_chk(&sr) );
		if( s_rec.s_period>0 && s_rec.s_period<=13 )
			break;
	}
	return(0);
}

/* Take the key values for starting and ending records */
static
get_lst_extent()
{
	if( SW7 )
#ifdef ENGLISH
		STRCPY( s_rec.s_keyhdng, "   CO       ACCT NUMBER   CODE" );
	else
		STRCPY( s_rec.s_keyhdng, "  FUND      ACCT NUMBER   CODE" );
#else
		STRCPY( s_rec.s_keyhdng, "   CIE      # DE COMPTE   CODE" );
	else
		STRCPY( s_rec.s_keyhdng, "  FOND      # DE COMPTE   CODE" );
#endif

	if( (retval = get_key())<0 )
		return(-1);
	if( retval==EXIT )
		return(EXIT);
	if( (retval = get_end_key())<0 )
		return(-1);
	if( retval==EXIT )
		return(EXIT);
	return( 0 );
}

/* display the record on the screen */
static
displ_rec()
{
	/* display 1st screen & wait until user hits a key */
	if( pgcnt!=0 )
		if( next_page()<0 )	/* clear the screen */
			return( EXIT );
	if( rite_top()<0 )
		return(-1);

	if( scr1()<0 )
		return(-1);

	/* display second screen */
	if( next_page()<0 )
		return(EXIT);
	if( rite_top()<0 )
		return(-1);
	if( scr2()<0 )
		return(-1);

	if( last_page()<0 )
		return( EXIT );
	else  
		return( 0 );
}

static
scr1()
{
	double budbal;
	int count;

	mkln( (LNSZ-strlen(param_rec.pa_co_name))/2, 
		param_rec.pa_co_name, strlen( param_rec.pa_co_name ) );
	if( prnt_line()<0 ) return(-1);
	if( prnt_line()<0 ) return(-1);

#ifdef ENGLISH
	mkln( 1," 1. ACCT#   ", 12 );
	mkln( 13,gl_rec.accno, strlen(gl_rec.accno) );
	mkln( 33," 2. RECORD CD ", 14 );
#else
	mkln( 1," 1. #COMPTE ", 12 );
	mkln( 13,gl_rec.accno, strlen(gl_rec.accno) );
	mkln( 33," 2. CD FICHE  ", 14 );
#endif
	tedit( (char *)&gl_rec.reccod, Mask_2, line+cur_pos, R_SHORT );
	cur_pos += 2;
	if( SW7)
#ifdef ENGLISH
		mkln( 52, "3. CO   ", 8 );
	else
		mkln( 52, "3. FUND ", 8 );
#else
		mkln( 52, "3. CIE  ", 8 );
	else
		mkln( 52, "3. FOND ", 8 );
#endif
	tedit( (char *)&gl_rec.funds, Mask_3, line+cur_pos, R_SHORT );
	cur_pos += 3;
#ifdef ENGLISH
	mkln( 67, "4. SECTION ", 12 );
#else
	mkln( 67, "4. SECTION ", 12 );
#endif
	tedit( (char *)&gl_rec.sect, Mask_1, line+cur_pos, R_SHORT );
	cur_pos += 1;
	if( prnt_line()<0 ) return(-1);

	if( SW7) {
#ifdef ENGLISH
		mkln( 1, " 5. REPORT CODE       ", 22 );
#else
		mkln( 1, " 5. CODE DE RAPPORT   ", 22 );
#endif
		tedit( (char *)&gl_rec.admis, Mask_1, line+cur_pos, R_SHORT );
		cur_pos += 1;
	}
/*
	else
#ifdef ENGLISH
		mkln( 1, " 5. ADMISSIBILITY CODE", 22 );
#else
		mkln( 1, " 5. CODE D'ADMISSIBILITE", 23 )
#endif
	tedit( (char *)&gl_rec.admis, Mask_1, line+cur_pos, R_SHORT );
	cur_pos += 1;    */

	if( prnt_line()<0 ) return(-1);

	if( SW7)
#ifdef ENGLISH
		mkln( 1," 6. SEQUENCE  ", 14 );
	else
		mkln( 1," 6. KEY 1     ", 14 );
#else
		mkln( 1," 6. SEQUENCE  ", 14 );
	else
		mkln( 1," 6. CLE 1     ", 14 );
#endif
	tedit( (char *)&gl_rec.keys[0], Mask_5, line+cur_pos, R_LONG );
	cur_pos += 5;
	if( SW7)
#ifdef ENGLISH
		mkln( 20,"  7. DEPT      ", 15 );
	else
		mkln( 20,"  7. KEY 2     ", 15 );
#else
		mkln( 20,"  7. DEPT      ", 15 );
	else
		mkln( 20,"  7. CLE 2     ", 15 );
#endif
	tedit( (char *)&gl_rec.keys[1], Mask_5, line+cur_pos, R_LONG );
	cur_pos += 5;
	if( SW7)
#ifdef ENGLISH
		mkln( 40,"  8. REGROUP   ", 15 );
	else
		mkln( 40,"  8. KEY 3     ", 15 );
#else
		mkln( 40,"  8. REGROUP   ", 15 );
	else
		mkln( 40,"  8. CLE 3     ", 15 );
#endif
	tedit( (char *)&gl_rec.keys[2], Mask_5, line+cur_pos, R_LONG );
	cur_pos += 5;
	if( prnt_line()<0 ) return(-1);
	 
	if( SW7)
#ifdef ENGLISH
		mkln( 1," 9. SUCC      ", 14 );
	else
		mkln( 1," 9. KEY 4     ", 14 );
#else
		mkln( 1," 9. SUCC      ", 14 );
	else
		mkln( 1," 9. CLE 4     ", 14 );
#endif
	tedit( (char *)&gl_rec.keys[3], Mask_5, line+cur_pos, R_LONG );
	cur_pos += 5;
	if( SW7)
#ifdef ENGLISH
		mkln( 20," 10. REG SUCC  ", 15 );
	else
		mkln( 20," 10. KEY 5     ", 15 );
#else
		mkln( 20," 10. REG SUCC  ", 15 );
	else
		mkln( 20," 10. CLE 5     ", 15 );
#endif
	tedit( (char *)&gl_rec.keys[4], Mask_5, line+cur_pos, R_LONG );
	cur_pos += 5;
	if( SW7)
#ifdef ENGLISH
		mkln( 40," 11. REG DEPT  ", 15 );
	else
		mkln( 40," 11. KEY 6     ", 15 );
#else
		mkln( 40," 11. REG DEPT  ", 15 );
	else
		mkln( 40," 11. CLE 6     ", 15 );
#endif
	tedit( (char *)&gl_rec.keys[5], Mask_5, line+cur_pos, R_LONG );
	cur_pos += 5;
	if( prnt_line()<0 ) return(-1);
	 
#ifdef ENGLISH
	mkln( 1,"12. KEY 7     ", 14 );
#else
	mkln( 1,"12. CLE 7     ", 14 );
#endif
	tedit( (char *)&gl_rec.keys[6], Mask_5, line+cur_pos, R_LONG );
	cur_pos += 5;

#ifdef ENGLISH
	mkln( 20," 13. KEY 8     ", 15 );
#else
	mkln( 20," 13. CLE 8     ", 15 );
#endif

	tedit( (char *)&gl_rec.keys[7], Mask_5, line+cur_pos, R_LONG );
	cur_pos += 5;
#ifdef ENGLISH
	mkln( 40," 14. KEY 9     ", 15 );
#else
	mkln( 40," 14. CLE 9     ", 15 );
#endif
	tedit( (char *)&gl_rec.keys[8], Mask_5, line+cur_pos, R_LONG );
	cur_pos += 5;
	if( prnt_line()<0 ) return(-1);
	 
#ifdef ENGLISH
	mkln( 1,"15. KEY 10    ", 14 );
#else
	mkln( 1,"15. CLE 10    ", 14 );
#endif
	tedit( (char *)&gl_rec.keys[9], Mask_5, line+cur_pos, R_LONG );
	cur_pos += 5;
#ifdef ENGLISH
	mkln( 20," 16. KEY 11    ", 15 );
#else
	mkln( 20," 16. CLE 11    ", 15 );
#endif
	tedit( (char *)&gl_rec.keys[10], Mask_5, line+cur_pos, R_LONG );
	cur_pos += 5;
#ifdef ENGLISH
	mkln( 40," 17. KEY 12    ", 15 );
#else
	mkln( 40," 17. CLE 12    ", 15 );
#endif
	tedit( (char *)&gl_rec.keys[11], Mask_5, line+cur_pos, R_LONG );
	cur_pos += 5;
	if( prnt_line()<0 ) return(-1);
	 
#ifdef ENGLISH
	mkln( 1,"18. BUDGET CODE ", 16 );
#else
	mkln( 1,"18. CODE BUDGET ", 16 );
#endif
	tedit( (char *)&gl_rec.cdbud, Mask_1, line+cur_pos, R_SHORT );
	cur_pos += 1;
/*
#ifdef ENGLISH
	mkln( 18,"  19. PROJECTION CODE", 21 );
#else
	mkln( 18,"  19. CODE DE PROJECTION", 24 );
#endif
	tedit( (char *)&gl_rec.cdpro, Mask_1, line+cur_pos, R_SHORT );
	cur_pos += 1;
#ifdef ENGLISH
	mkln( 39,"  20. UNIT MEASURE ", 19 );
#else
	mkln( 39,"  20. UNITE DE MESURE", 21 );
#endif
	tedit( (char *)&gl_rec.cdunit, Mask_3, line+cur_pos, R_SHORT );
	cur_pos += 3;
*/
	if( prnt_line()<0 ) return(-1);
	
#ifdef ENGLISH
	mkln( 1,"21. DESCRIPTION ", 16 );
#else
	mkln( 1,"21. DESCRIPTION ", 16 );
#endif
	mkln( 17, gl_rec.desc, strlen( gl_rec.desc ) );
	if( prnt_line()<0 ) return(-1);

	mkln(1, hyp_line, 75 );
	if( prnt_line()<0 ) return(-1);

#ifdef ENGLISH
	mkln( 1,"23. COMMITMENTS TO DATE", 23 );
#else
	mkln( 1,"23. ENGAGEMENTS A DATE", 22 );
#endif
	mkln( 34," ", 1 );
	tedit( (char *)&gl_rec.comdat, Amt_Mask, line+cur_pos, R_DOUBLE );
	cur_pos += strlen(Amt_Mask);
	if( prnt_line()<0 ) return(-1);

#ifdef ENGLISH
	mkln( 1,"24. CURRENT PERIOD DEBIT", 24 );
#else
	mkln( 1,"24. DEBIT DE LA PERIODE COURANTE", 33 );
#endif
	mkln( 34," ", 1 );
	tedit( (char *)&gl_rec.curdb, Amt_Mask, line+cur_pos, R_DOUBLE );
	cur_pos += strlen( Amt_Mask );
	if( prnt_line()<0 ) return(-1);

#ifdef ENGLISH
	mkln( 1,"25. CURRENT PERIOD CREDIT", 25 );
#else
	mkln( 1,"25. CREDIT DE LA PERIODE COURANTE", 33 );
#endif
	mkln( 34," ", 1 );
	tedit( (char *)&gl_rec.curcr, Amt_Mask, line+cur_pos, R_DOUBLE );
	cur_pos += strlen( Amt_Mask );
	if( prnt_line()<0 ) return(-1);

#ifdef ENGLISH
	mkln( 1,"26. CURRENT YEAR TO DATE", 24 );
#else
	mkln( 1,"26. CUMUL COURANT       ", 24 );
#endif
	mkln( 34," ", 1 );
	tedit( (char *)&gl_rec.ytd, Amt_Mask, line+cur_pos, R_DOUBLE );
	cur_pos += strlen( Amt_Mask );
	if( prnt_line()<0 ) return(-1);

#ifdef ENGLISH
	mkln( 1,"27. CURRENT YEAR BUDGET", 23 );
#else
	mkln( 1,"27. BUDGET DE L'ANNEE COURANTE", 30 );
#endif
	mkln( 34," ", 1 );
	tedit( (char *)&gl_rec.budcur, Amt_Mask, line+cur_pos, R_DOUBLE );
	cur_pos += strlen( Amt_Mask );
	if( prnt_line()<0 ) return(-1);

#ifdef ENGLISH
	mkln( 1,"28. LAST YEAR BUDGET", 20 );
#else
	mkln( 1,"28. BUDGET DE L'ANNEE PASSEE", 28 );
#endif
	mkln( 34," ", 1 );
	tedit( (char *)&gl_rec.budpre, Amt_Mask, line+cur_pos, R_DOUBLE );
	cur_pos += strlen( Amt_Mask );
	if( prnt_line()<0 ) return(-1);

	budbal = gl_rec.budcur-gl_rec.ytd-gl_rec.comdat;
#ifdef ENGLISH
	mkln( 1,"29. CURRENT BUDGET BALANCE", 26 );
#else
	mkln( 1,"29. BALANCE DU BUDGET COURANT", 29 );
#endif
	mkln( 34," ", 1 );
	tedit( (char *)&budbal, Amt_Mask, line+cur_pos, R_DOUBLE );
	cur_pos += strlen( Amt_Mask );
	if( prnt_line()<0 ) return(-1);

#ifdef ENGLISH
	mkln( 1,"30. OPENING BALANCE", 19 );
#else
	mkln( 1,"30. BALANCE D'OUVERTURE", 23 );
#endif
	mkln( 34," ", 1 );
	tedit( (char *)&gl_rec.opbal, Amt_Mask, line+cur_pos, R_DOUBLE );
	cur_pos += strlen( Amt_Mask );
	if( prnt_line()<0 ) return(-1);

	budbal = 0.0; /* Being used to calculate last year actual total */
	for( count=0; count<param_rec.pa_no_periods; count++ )
		budbal+=gl_rec.prerel[count];
#ifdef ENGLISH
	mkln( 1,"31. ACTUAL LAST YEAR", 20 );
#else
	mkln( 1,"31. ACTUEL DE L'ANNEE PASSEE", 28 );
#endif
	mkln( 34," ", 1 );
	tedit( (char *)&budbal, Amt_Mask, line+cur_pos, R_DOUBLE );
	cur_pos += strlen( Amt_Mask );
	if( prnt_line()<0 ) return(-1);

	return(0);
}

scr2()
{
	int count;
	if( SW1 ){ /* if it is screen inquiry */
		mkln( (LNSZ-strlen(param_rec.pa_co_name))/2, 
			param_rec.pa_co_name, strlen( param_rec.pa_co_name ) );
		if( prnt_line()<0 ) return(-1);
	
#ifdef ENGLISH
		mkln( 1,"ACCOUNT NUMBER ", 15 );
		mkln( 16, gl_rec.accno, strlen( gl_rec.accno ) );
		mkln( 37,"RECORD CODE ", 12 );
#else
		mkln( 1," # DE COMPTE   ", 15 );
		mkln( 16, gl_rec.accno, strlen( gl_rec.accno ) );
		mkln( 37,"CD DE FICHE ", 12 );
#endif
		tedit( (char *)&gl_rec.reccod, Mask_2, line+cur_pos, R_SHORT );
		cur_pos += 2;
		if( prnt_line()<0 ) return(-1);

#ifdef ENGLISH
		mkln( 1,"DESCRIPTION ", 12 );
#else
		mkln( 1,"DESCRIPTION ", 12 );
#endif
		mkln( 13, gl_rec.desc, strlen( gl_rec.desc ) );
		if( prnt_line()<0 ) return(-1);
	}
#ifdef ENGLISH
	mkln( 9,"<------------ACTUAL------------>", 32 );
	mkln( 45,"<----------BUDGET-------------->", 32 );
#else
	mkln( 9,"<------------ACTUEL------------>", 32 );
	mkln( 45,"<----------BUDGET-------------->", 32 );
#endif
	if( prnt_line()<0 ) return(-1);

#ifdef ENGLISH
	mkln( 1,"PERIOD",6 );
	mkln( 9,"CURRENT YEAR",12 );
	mkln( 27,"PREVIOUS YEAR",13 );
	mkln( 45,"CURRENT YEAR",12 );
	mkln( 63,"PREVIOUS YEAR",13 );
#else
	mkln( 1,"PERIODE",7 );
	mkln( 9,"ANNEE COURANTE",14 );
	mkln( 27,"ANNEE PRECEDENTE",16 );
	mkln( 45,"ANNEE COURANTE",14 );
	mkln( 63,"ANNEE PRECEDENTE",16 );
#endif
	if( prnt_line()<0 ) 
		return(-1);

	for( count=0; count<param_rec.pa_no_periods; count++ )
		if( bud_line( count )<0 )
			return(-1);
	linecnt = PGSIZE;
	return(0);
}

static
bud_line( ln_no )
int ln_no;	/* period no */
{
	int period_no;

	period_no = ln_no+1;
	mkln( 2, " ", 1 );
	tedit( (char *)&period_no, Mask_2, line+cur_pos, R_INT );
	cur_pos += 2;

	mkln( 8, " ", 1 );
	tedit( (char*)&gl_rec.currel[ln_no], Amt_Mask, line+cur_pos, R_DOUBLE );
	cur_pos += strlen( Amt_Mask );

	mkln( 26, " ", 1 );
	tedit( (char*)&gl_rec.prerel[ln_no], Amt_Mask, line+cur_pos, R_DOUBLE );
	cur_pos += strlen( Amt_Mask );

	mkln( 44, " ", 1 );
	tedit( (char*)&gl_rec.curbud[ln_no], Amt_Mask, line+cur_pos, R_DOUBLE );
	cur_pos += strlen( Amt_Mask );

	mkln( 62, " ", 1 );
	tedit( (char*)&gl_rec.prebud[ln_no], Amt_Mask, line+cur_pos, R_DOUBLE );
	cur_pos += strlen( Amt_Mask );

	if( prnt_line()<0 )
		return(-1);
	return(0);	
}

/* print the records on the printer */
static
prnt_records()
{
	int hdg_reqd;

	/* initialize the linecount and pagecount variables for printing */

	/* initialize the record values for getting the isam records */
	if( init_recinfo()<0 )
		return(-1);

	flg_reset(GLMAST);

	/* do until no more records to be printed */
	for( ; ; ){
		/* get the next record from the file and check for errors */
		retval = get_n_gl( &gl_rec, BROWSE, 0, FORWARD, e_mesg );
		if(retval==ERROR){
			fomen(e_mesg);
			get();
			return(-1);
		}

		if( retval==EFL ) {
			seq_over(GLMAST);
			return(0);
		}

		/* if partial listing is requested, check if specified range
			is crossed */
		retval = range_crossed();
		if( retval==YES )
			return(0);
		/* now prepare to print */

		/* check if page advancing is required & do accordingly */
		hdg_reqd = chk_pg_adv();
		if( hdg_reqd<0 )
			return(0);

		/* This hdg_reqd is used in calling print routines below */
		/* switch to the required printing process */
		switch( s_rec.s_listopt[0] ){
			case '1':
			case '2':
				if( pr_comp()<0 )
					return(-1);
				break;
			case '3':
			case '4':
				if( pr_codkey( hdg_reqd )<0 )
					return(-1);
				break;
			case '5':
			case '6':
				if( pr_bal( hdg_reqd )<0 )
					return(-1);
				break;
		}
	}
}

/* No manual entry made for medium so #defines not required */
static
init_prnt( medium )
char *medium;
{
	switch( medium[0] ){
		case 'D':	/* must be set to 'D' here */
			pgcnt = 0;
			retval = opn_prnt( "D", sr.termnm, 1, e_mesg, 1 );
			LNSZ = 80;
			/*  PGSIZE  set by  opn_prnt() */
			break;
		case 'P':	/* must be set to 'P' here */
			pgcnt = 0;
			retval = opn_prnt( "P", sr.termnm, 1, e_mesg, 1 );
			SetCopies ( (int)copies );
			LNSZ = 131;
			/*  PGSIZE  set by  opn_prnt() */
			break;
	}		

	if( retval<0 ){
		fomen(e_mesg);
		get();
		return(-1);
	}
	linecnt = PGSIZE;
	pgcnt = 0;
	
	return(0);
}

static
init_recinfo()
{
	switch( s_rec.s_listopt[0] ){
		case '1':
		case '3':
		case '5':
			gl_rec.accno[0] = '\0';
			gl_rec.reccod = 0;
			gl_rec.funds = 0;
			break;
		case '2':
		case '4':
		case '6':
			STRCPY(gl_rec.accno,s_rec.s_st_acct);
			gl_rec.reccod = (short)s_rec.s_st_code;
			gl_rec.funds = (short)s_rec.s_st_fund;
			break;
		default:
			break;
	}
	return( 0 );
}

static
range_crossed()
{
	int key1, key2, key3;

	key1 = gl_rec.funds - (short)s_rec.s_end_fund;
	key2 = strcmp( gl_rec.accno, s_rec.s_end_acct );
	key3 = gl_rec.reccod - (short)s_rec.s_end_code;

	if( key1>0 )
		return(YES);
	else if( key1<0 )
		return(NO);
	else{
		if( key2>0 )
			return(YES);
		else if( key2<0 )
			return(NO);
		else{
			if( key3>0 )
				return(YES);
			else return(NO);
		}
	}
}

static
chk_pg_adv()
{
	if( linecnt>=PGSIZE ){
		if(pgcnt) {
			if( rite_top()<0 )
				return(-1);
		}
		else
			linecnt = 0;
		pgcnt++;
		return( YES );
	}
	else
		return( NO );
}

static
pr_comp()
{
	if( common_hdg()<0 )
		return(-1);
	if( hdg_comp()<0 )
		return(-1);
	if( scr1()<0 )
		return(-1);
	if( prnt_line()<0 )
		return(-1);
	if( prnt_line()<0 )
		return(-1);
	if( scr2()<0 )
		return(-1);
	return(0);
}

static
pr_codkey( hdg_reqd )
int hdg_reqd;
{
	if( hdg_reqd==YES ){
		if( common_hdg()<0 )
			return(-1);
		if( hdg_codkey()<0 )
			return(-1);
		if( prnt_line()<0 )
			return(-1);
	}

	tedit( (char *)&gl_rec.funds, Mask_3, line, R_SHORT );
	cur_pos=3;
	mkln( 6,gl_rec.accno,strlen(gl_rec.accno) );
	mkln( 34,gl_rec.desc, strlen(gl_rec.desc) );
	if( prnt_line()<0 )
		return(-1);

	mkln(24," ",1);
	tedit( (char *)&gl_rec.reccod, Mask_2,line+cur_pos, R_SHORT);
	cur_pos+=2;
	mkln(27," ",1);
	tedit( (char *)&gl_rec.sect, Mask_1,line+cur_pos, R_SHORT);
	cur_pos+=1;
	mkln(30," ",1);
	tedit( (char *)&gl_rec.admis, Mask_1,line+cur_pos, R_SHORT);
	cur_pos+=1;
	
	mkln(33," ",1);
	tedit( (char *)&gl_rec.keys[0], Mask_5, line+cur_pos, R_LONG );
	cur_pos+=5;

	mkln(39," ",1);
	tedit( (char *)&gl_rec.keys[1], Mask_5, line+cur_pos, R_LONG );
	cur_pos+=5;

	mkln(45," ",1);
	tedit( (char *)&gl_rec.keys[2], Mask_5, line+cur_pos, R_LONG );
	cur_pos+=5;

	mkln(51," ",1);
	tedit( (char *)&gl_rec.keys[3], Mask_5, line+cur_pos, R_LONG );
	cur_pos+=5;

	mkln(57," ",1);
	tedit( (char *)&gl_rec.keys[4], Mask_5, line+cur_pos, R_LONG );
	cur_pos+=5;

	mkln(63," ",1);
	tedit( (char *)&gl_rec.keys[5], Mask_5, line+cur_pos, R_LONG );
	cur_pos+=5;

	mkln(69," ",1);
	tedit( (char *)&gl_rec.keys[6], Mask_5, line+cur_pos, R_LONG );
	cur_pos+=5;

	mkln(75," ",1);
	tedit( (char *)&gl_rec.keys[7], Mask_5, line+cur_pos, R_LONG );
	cur_pos+=5;

	mkln(81," ",1);
	tedit( (char *)&gl_rec.keys[8], Mask_5, line+cur_pos, R_LONG );
	cur_pos+=5;

	mkln(87," ",1);
	tedit( (char *)&gl_rec.keys[9], Mask_5, line+cur_pos, R_LONG );
	cur_pos+=5;

	mkln(93," ",1);
	tedit( (char *)&gl_rec.keys[10], Mask_5, line+cur_pos, R_LONG );
	cur_pos+=5;

	mkln(99," ",1);
	tedit( (char *)&gl_rec.keys[11], Mask_5, line+cur_pos, R_LONG );
	cur_pos+=5;

	mkln(107," ",1);
	tedit( (char *)&gl_rec.cdbud, Mask_2, line+cur_pos, R_SHORT );
	cur_pos+=2;

	mkln(111," ",1);
	tedit( (char *)&gl_rec.cdpro, Mask_2, line+cur_pos, R_SHORT );
	cur_pos+=2;

	mkln(115," ",1);
	tedit( (char *)&gl_rec.cdunit, Mask_3, line+cur_pos, R_SHORT );
	cur_pos+=3;

	if( prnt_line()<0 )
		return(-1);
	if( prnt_line()<0 )
		return(-1);

	return(0);
}

static
common_hdg()
{
	long rundt;
	int i;

	mkln(1,s_rec.s_progname,strlen(s_rec.s_progname));
#ifdef ENGLISH
	mkln(116,"PAGE ",5);
#else
	mkln(116,"PAGE ",5);
#endif
	tedit( (char *)&pgcnt,Mask_3,line+cur_pos,R_SHORT );
	cur_pos+=3;
	if( prnt_line()<0 )
		return(-1);

	i = strlen( param_rec.pa_co_name );
	mkln( (LNSZ-i)/2, param_rec.pa_co_name, i );
#ifdef ENGLISH
	mkln(116,"DATE ",5);
#else
	mkln(116,"DATE ",5);
#endif
	rundt = get_date();
	tedit( (char *)&rundt,Date_Mask,line+cur_pos,R_LONG);
	cur_pos+=strlen(Date_Mask);
	if( prnt_line()<0 )
		return(-1);
	
	return(0);
}

static
hdg_comp()
{
	int i;

	i = (LNSZ-strlen(COMP_HDG))/2;
	mkln( i, COMP_HDG, strlen(COMP_HDG) );
	if( prnt_line()<0 )
		return(-1);
	return(0);
}

static
hdg_codkey()
{
	int i;  /* loop counter & temporary storage */

	i = (LNSZ-strlen(COD_KEY_HDG))/2;	
	mkln(i,COD_KEY_HDG,strlen(COD_KEY_HDG));
	if( prnt_line()<0 )
		return(-1);
	if( prnt_line()<0 )
		return(-1);
	if(SW7)
#ifdef ENGLISH
		mkln(1," CO  <-ACCOUNT NUMBER->", 23);
	else
		mkln(1,"FUND <-ACCOUNT NUMBER->", 23);
#else
		mkln(1," CIE <-NUMERO DE COMPTE->", 25);
	else
		mkln(1,"FOND <-NUMERO DE COMPTE->", 25);
#endif
#ifdef ENGLISH
	mkln(34,"<------------ D E S C R I P T I O N ----------->",48);
#else
	mkln(34,"<------------ D E S C R I P T I O N ----------->",48);
#endif
	if( prnt_line()<0 )
		return(-1);

#ifdef ENGLISH
	mkln(34,"<-------------------------- GROUP OF KEYS",41);
	mkln(75," ---------------------------->",30);
#else
	mkln(34,"<------------------------- GROUPE DE CLES",41);
	mkln(75," ---------------------------->",30);
#endif
	if( prnt_line()<0 )
		return(-1);

#ifdef ENGLISH
	mkln(25,"RC SC ",6);
#else
	mkln(25,"CF CS ",6);
#endif
	if(SW7)
#ifdef ENGLISH
		mkln(31,"LC ",3);
	else
		mkln(31,"AC ",3);
#else
                mkln(31,"LC ",3);
	else
		mkln(31,"CA ",3);
#endif
	for( i=0; i<NO_KEYS; i++)
		mkln(i*6+34,"<---> ",6);
#ifdef ENGLISH
	mkln(108,"BC  PC   UM",11);
#else
	mkln(108,"CB  CP   UM",11);
#endif
	if( prnt_line()<0 )
		return(-1);
	
	if(SW7)
#ifdef ENGLISH
		mkln(35," SEQ",4);
#else
		mkln(35," SEQ",4);
#endif
	else
		mkln(36,"1",1);
	if(SW7)
#ifdef ENGLISH
		mkln(41,"DEPT",4);
#else
		mkln(41,"DEPT",4);
#endif
	 else
		mkln(42,"2",1);
	if(SW7)
#ifdef ENGLISH
		mkln(47," RGP",4);
#else
		mkln(47," RGP",4);
#endif
	else
		mkln(48,"3",1);
	if(SW7)
#ifdef ENGLISH
		mkln(53," BRA",4);
#else
		mkln(53,"SUCC",4);
#endif
	else
		mkln(54,"4",1);
	if(SW7)
#ifdef ENGLISH
		mkln(58,"R BRA",5);
#else
		mkln(58,"R BRA",5);
#endif
	else
		mkln(60,"5",1);
	if(SW7)
#ifdef ENGLISH
		mkln(64,"R DEP",5);
#else
		mkln(64,"R DEP",5);
#endif
	else
		mkln(66,"6",1);
	mkln(72,"7",1);
	mkln(78,"8",1);
	mkln(84,"9",1);
	mkln(90,"10",2);
	mkln(96,"11",2);
	mkln(102,"12",2);
	if( prnt_line()<0 )
		return(-1);
	
	return(0);
}

static
pr_bal(hdg_reqd)
int hdg_reqd;
{
	double total;
	int count;

	if( hdg_reqd==YES ){
		if( common_hdg()<0 )
			return(-1);
		if( hdg_bal()<0 )
			return(-1);
		if( prnt_line()<0 )
			return(-1);
	}

	tedit( (char *)&gl_rec.funds, Mask_3, line, R_SHORT );
	cur_pos=3;
	mkln(6," ",1); 
	mkln( 7,gl_rec.accno,strlen(gl_rec.accno) );
	mkln(26," ",1); 
	tedit( (char *)&gl_rec.reccod, Mask_2, line+cur_pos, R_SHORT );
	cur_pos+=2;
	if( prnt_line()<0 )
		return(-1);

	tedit( (char *)&gl_rec.comdat, Amt_Mask, line, R_DOUBLE );
	cur_pos+=strlen( Amt_Mask );
	mkln(15," ",1); 
	tedit( (char *)&gl_rec.currel[s_rec.s_period-1], 
			Amt_Mask, line+cur_pos, R_DOUBLE );
	cur_pos+=strlen( Amt_Mask );

	total = 0.0;
	for( count=0 ; count<s_rec.s_period; count++ )
		total+=gl_rec.prerel[count];
	mkln(30," ",1); 
	tedit( (char *)&total, Amt_Mask, line+cur_pos, R_DOUBLE );
	cur_pos+=strlen( Amt_Mask );

	total = 0.0;
	for( count=0; count<s_rec.s_period; count++ )
		total+=gl_rec.currel[count];
	mkln(45," ",1); 
	tedit( (char *)&total, Amt_Mask, line+cur_pos, R_DOUBLE );
	cur_pos+=strlen( Amt_Mask );

	mkln(60," ",1); 
	tedit( (char *)&gl_rec.curbud[s_rec.s_period-1], 
			Amt_Mask, line+cur_pos, R_DOUBLE );
	cur_pos+=strlen( Amt_Mask );

	mkln(75," ",1); 
	tedit( (char *)&gl_rec.budcur, Amt_Mask, line+cur_pos, R_DOUBLE );
	cur_pos+=strlen( Amt_Mask );

	mkln(90," ",1); 
	tedit( (char *)&gl_rec.budpre, Amt_Mask, line+cur_pos, R_DOUBLE );
	cur_pos+=strlen( Amt_Mask );

	mkln(105," ",1); 
	tedit( (char *)&gl_rec.opbal, Amt_Mask, line+cur_pos, R_DOUBLE );
	cur_pos+=strlen( Amt_Mask );

	if( prnt_line()<0 )
		return(-1);
	if( prnt_line()<0 )
		return(-1);

	return(0);
}

static
hdg_bal()
{
	int i;

	i = (LNSZ-strlen(BALANCE_HDG))/2;	
	mkln(i,BALANCE_HDG,strlen(BALANCE_HDG));
	tedit( (char *)&s_rec.s_period, Mask_2, line+cur_pos, R_SHORT );
	cur_pos+=2;
	if( prnt_line()<0 )
		return(-1);

	if(SW7)
#ifdef ENGLISH
                mkln(1," CO ",4);
	else
		mkln(1,"FUND",4);
#else
                mkln(1,"CIE ",4);
	else
		mkln(1,"FOND",4);
#endif
#ifdef ENGLISH
        mkln(7,"<----ACCOUNT----->",18);
	mkln(27,"RC",2);
#else
        mkln(7,"<-----COMPTE----->",18);
	mkln(27,"CF",2);
#endif
	if( prnt_line()<0 )
		return(-1);

#ifdef ENGLISH
	mkln(3,"COMMITMENTS",11);
	mkln(16,"CURRENT PERIOD",14);
	mkln(32,"PREVIOUS YEAR",13);
	mkln(47,"CURRENT YEAR",12);
	mkln(65,"BUDGET",6);
	mkln(80,"BUDGET",6);
	mkln(95,"BUDGET",6);
	mkln(109,"OPENING",7);
#else
	mkln(3,"ENGAGEMENTS",11);
	mkln(16," PERIODE COUR ",14);
	mkln(32,"  ANNEE PREC ",13);
	mkln(47," ANNEE COUR ",12);
	mkln(65,"BUDGET",6);
	mkln(80,"BUDGET",6);
	mkln(95,"BUDGET",6);
	mkln(111,"BALANCE",7);
#endif
	if( prnt_line()<0 )
		return(-1);

#ifdef ENGLISH
	mkln(5,"TO DATE",7);
	mkln(35,"TO DATE",7);
	mkln(49,"TO DATE",7);
	mkln(61,"CURRENT PERIOD",14);
	mkln(77,"CURRENT YEAR",12);
	mkln(91,"PREVIOUS YEAR",13);
	mkln(109,"BALANCE",7);
#else
	mkln(6,"A DATE",7);
	mkln(36,"A DATE",7);
	mkln(50,"A DATE",7);
	mkln(61," PERIODE COUR ",14);
	mkln(77," ANNEE COUR ",12);
	mkln(91,"  ANNEE PREC ",13);
	mkln(109,"D'OUVERTURE",11);
#endif
	if( prnt_line()<0 )
		return(-1);

	return(0);	
}