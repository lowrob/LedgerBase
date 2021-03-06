/******************************************************************************
		Sourcename    : menuscrn.c
		System        : Budgetary Financial system.
		Created on    : 89-07-21
		Created  By   : K HARISH.
		Cobol sources :
*******************************************************************************

About:	This is a general purpose menu driver program, which maintains a
	hierarchy of menus down to MAX_LEVELS (10) number of levels.

	The program reads menu data files from the directory /usr/bfs/nfm.
	The menu options are displayed on the screen for user's selection.
	Depending on the menu option selected, the program further reads 
	another menu (from another data file) and displays it, or invokes
	a child process.

	The format for the menudata file is as follows:

	Screen heading on the first line of data file.(max.HDG_LENGTH chars)
	For each menuitem, the following data seperated by spaces,
	in a single line:
	1.	Menu item index for selection	(2 chars max.)
	2.	Menu item name for identification ( 31 chars max.)
	3.	Menu item type for further processing by the menu driver. Valid
		types, and results after selection are:

		      TYPE	Result when menuitem is selected

			P	takes to previous menu
			I	takes to first level menu in the hierarchy
			M	shows another menu
			E	executes a process in multi user environment
			S	executes a process in single user environment
			R	does nothing with that option (dummy)

	4.	If Menu item type is M or E, a filename corresponding to the 
		type, i.e., another menudata filename (for M) or executable file
		name (for E). 

	If any of the above items are made of more than 1 word, the seperating
	spaces within the word should be filled with '~' character.

	If a blank line (as a seperator) between menuitems is required, the
	data file should contain a blank line ( with just a return ) at the
	corresponding location. There should not be any blank lines after the
	last menuitem in the data file.
	A maximum of MAX_OPTIONS (16) number of options are allowed in each
	menu.

	SAMPLE DATA FILE:
-------------------------------------------------------------------------------
Screen~Heading
P	RETURN~TO~PREVIOUS~MENU		P
I	RETURN~TO~INITIAL~MENU		I

OM	OPTION~WITH~MORE~MENU		M	menu001
AO	ANOTHER~OPTION~WITH~MENU	M	menu002
DO	DUMMY~OPTION			R
EO	EXECUTABLE~OPTION		E	program~-s1
SP	SINGLEUSER~PROCESS		S	sprogram~-s1
-------------------------------------------------------------------------------
The above sample has 8 menu options, including the blank line in between.


HISTORY:
 Date           Programmer     Description of modification
____/__/__      __________     ___________________________

******************************************************************************/

#define	MAIN

#define	BFS	/* used in calling BFS database handling routines */

#define SYSTEM		"UTILITIES"
#define MOD_DATE	"23-JAN-90"

#include <stdio.h>
#include bfs_defs.h
#include <cfomstrc.h>

#ifdef ENGLISH
#define COMPANY	'C'
#else
#define COMPANY	'C'
#endif

#ifdef BFS
#include <bfs_recs.h>
#else
#include <time.h>
#endif

#ifndef	MS_DOS		/* Only on XENIX/UNIX */
#define	USE_SEMAPHORE
#endif

#ifdef	USE_SEMAPHORE
#include <sys/types.h>	/* used for implementing semaphores */
#include <sys/ipc.h>
#include <sys/sem.h>

#define	SEMFLAG		( 00666 ^ umask(maskvalue) | IPC_CREAT | IPC_NOWAIT )
#define	SINGLEUSERSEMVAL	1000
#endif

#define SCREEN_NAME	"menuscrn"
#define	BOLD		7
#define	BLINK		8
#define	REVERSE		9
#define	OFF		5
#define	ON		3
#define	PERIOD		2

#define INDEX_LEN	3	/* no. of chars forming menu index on screen */
#define	MAX_LEVELS	10	/* Max. no. of levels in menu hierarchy */
#define HDG_LENGTH	35	/* Screen heading length. maximum */
#define	MAX_OPTIONS	16	/* Max. no of options in a single menu */
#define	OPTION_LEN	31	/* Max. length of the menu item name */
#define	CMNDLINE_LEN	80	/* Command line length . Maximum */
#define	FILLER_CHAR	'~'	/* Used in data file to link multiword names */
#define	SPACE	 	' '	/* character to replace '~' during display */
#define	MENU	 	10	/* Returned when M type option is selected */
#define	EXIT	 	30
#define	EXECUTE	 	40	/* Returned when E type option is selected */

#define	CHECK( X )	if( FlushFillers( X )<0 )	return(-1)

typedef struct{
	char	index[INDEX_LEN+1];	/* index for menu selection */
	char	name[OPTION_LEN+1];	/* name of the menu item */
}	Line;				/* a menu line on the screen */

/* menuscrn.sth - header for C structure generated by PROFOM EDITOR */

struct	s_struct	{
	char	menufile[11];	/*  100 STRING */
	char	scrhdg[HDG_LENGTH+1];	/* 200 STRING */
	long	rundt;		/* 300 NUMERIC */
	long	runtime;	/* 350 NUMERIC ##:##:## */
	Line	menuitem[MAX_OPTIONS];	/* 400 - 3500 */
	char	option1[2];	/* 3600 Option field for user's entry */
	char	option2[3];	/* 3700 Option field for user's entry */
	char	mesg[77];	/* 3800 STRING Message field*/
	char	resp[2];	/* 3900 STRING User's response field */
} screen;

typedef struct{
	char	index[INDEX_LEN+1];
	char	name[OPTION_LEN+1];
	char	type[2];
	char	filename[CMNDLINE_LEN];
}MenuItem;	/* Buffer to store the data read from menu data file */

#ifdef	USE_SEMAPHORE
int	semid;
key_t	sem_key;
union	semun{
	int	val;
	struct	semid_ds	*buf;
	ushort	*array;
}	arg;
static	int	maskvalue ;
#endif

#ifdef	ORACLE
static	char	opfilename[50];
#endif

static	char	e_mesg[80];
static	struct stat_rec	sr;
static	MenuItem	m_item[MAX_OPTIONS];
static	char	screen_stack[MAX_LEVELS][OPTION_LEN+1];
static	int	stackindex, last_highlighted = OFF;
short	no_of_options, last_sind, cur_sind, offset;
short	optionfldlen;	/* option field length, 1 or 2, changes dynamically */

#ifdef BFS
Pa_rec	param_rec;
#endif

char	in_str[80];

main( argc, argv )
int	argc;
char	*argv[];
{
	short	i;
	int	retval, j, index;
	char	command[80], *cptr;
	FILE	*fopen(), *fp;

	if( argc < 3 )	/* Don't say anything. Just quit for security. */
		exit(0);

	/* Set the environment */
	strncpy(SYS_NAME, SYSTEM, 50);
	strncpy(CHNG_DATE, MOD_DATE, 10);
	proc_switch(argc, argv, -1) ;

	/* Set the District */
	if(dist_no[0] == '\0')
		set_dist() ;
	strcpy(e_mesg,DATA_PATH);
	strcat( e_mesg, dist_no );
	if( access(e_mesg,0)<0 ){
#ifdef ENGLISH
		printf("\nInvalid District.\n");
#else
		printf("\nDistrict invalide.\n");
#endif
		exit(0);
	}

#ifdef	ORACLE
	/* Create oracle user's password temporary file with path */
	strcpy( opfilename, DATA_PATH );
	strcat(opfilename, PASSWDPREFIX );
	get_tnum( opfilename+strlen(opfilename) );
#endif


	if( gettname()<0 )
		exit(-1);

	if( CheckTrmlAccess(e_mesg)<0 ){
		printf("\n\n%s\n\n", e_mesg);
		close_dbh() ;
		exit(-1);
	}

	if( InitProfom(sr.termnm)<0 )
		exit(-1);

	if( InitScreen()<0 )
		ErrExit();

	if(strcmp(CC_code,"99")==0) {
		chdir(WORK_DIR) ;		/* Change working dir */
	}
	else {
		strcpy(in_str,WORK_DIR);
		strcat(in_str, "/");
		strcat(in_str, CC_code);
		chdir(in_str);
	}

	stackindex = 0;	/* counter for screen stack  */
	strcpy( screen_stack[stackindex], argv[1] );	/* first menudatafile */

	if( (no_of_options = ReadMenuData())<1 )    /* Read from stack top */
		ErrExit();

#ifdef	USE_SEMAPHORE
	sscanf( dist_no, "%d", &retval );
	sscanf( argv[2], "%d", &j );
	sem_key = (key_t)((j * 100) + retval) ;
	maskvalue = umask(0) ;
	umask(maskvalue) ;
#endif

	for( ; ; ){
		
		if( ShowMenu()<0 )	/* Show the latest menu read */
			ErrExit();

		retval = ProcMenu();	/* Allow selection and return value */
		if( retval<0 )
			ErrExit();

		switch( retval ){	/* stack check done by ProcMenu */
			case MENU:	/* show menu */
				if( (no_of_options = ReadMenuData())<1 )
					ErrExit();
				continue;
			case EXECUTE:	/* find executable program & execute */
#ifdef	ORACLE
				/* Store oracle user's passwd in a temp file */
				fp = fopen( opfilename, "w" );
				if( fp!=NULL ){
					fprintf( fp, "%s", UserPasswd );
					fclose( fp );
				}
#endif
				i = cur_sind - offset;
				strcpy( e_mesg, m_item[i].filename );
				cptr = e_mesg ;
				for( ; *cptr ; cptr++)
					if ( *cptr == SPACE ) {
						*cptr++  = '\0' ;
						 break ;
					}
				/**** arguments from m_item buffer ****/
				sprintf(command,"%s%s -t%s -d%s", EXE_PATH,
					e_mesg, sr.termnm, dist_no );
#ifdef	BFS
				if(param_rec.pa_co_or_dist[0]==COMPANY)
					strcat( command, " -s7");
#endif

				for(index=2; index< argc; index++) {
					if(strncmp(argv[index],"-t",2)==0 ||
						strncmp(argv[index],"-d",2)==0)
						continue ;
					sprintf(command+strlen(command)," %s",
								 argv[index] ) ;
				}
				sprintf(command+strlen(command)," %s", cptr) ;

#ifdef	USE_SEMAPHORE
				/* create a semaphore if it doesn't exist */
				semid = semget( sem_key, 1, SEMFLAG );
				if( semid<0 ){
					CloseProcess() ;
#ifdef ENGLISH
					printf("Semaphore Error: %d\n",errno );
#else
					printf("Erreur de semaphore: %d\n",errno );
#endif
					exit( -1) ;
				}

				j = GetSem();
				if( j>=SINGLEUSERSEMVAL ){
#ifdef ENGLISH
					fomer("Can't proceed as the system is in SINGLE USER mode.");
#else
					fomer("Ne peut pas proceder car le systeme est en mode du monousager.");
#endif
					continue;
				}
				if( m_item[i].type[0]=='S' ){
				    if( j>0 ){
#ifdef ENGLISH
					fomer("Can't proceed as other processes are running.");
#else
					fomer("Ne peut pas proceder car d'autres operation en progres.");
#endif
					continue;
				    }

				    SetSem( SINGLEUSERSEMVAL );
				}
				else
					SetSem( j+1 );
#endif
				fomcs(); 
				fomrt();
				fflush(stdout) ;

				retval = system( command ) ;

#ifdef	USE_SEMAPHORE
				/* The current process is the only one
				   running, (true for single user process)
				   remove semaphore and exit
				   else decrement the semaphore.
				*/
				if( m_item[i].type[0]=='S' )
#ifdef	REM_SEMAPHORE
					RemSem();	/* Remove Semaphore */
#else
					SetSem( 0 ) ;
#endif
				else{	
					j = GetSem();
					if( j>1 )
						SetSem( j-1 );
					else
#ifdef	REM_SEMAPHORE
						RemSem();
#else
						SetSem( 0 ) ;
#endif
				}
#endif
#ifdef	ORACLE
				/* Delete oracle user's password temp file */
				unlink( opfilename );
#endif
				fomst();
				if(retval) {
#ifdef ENGLISH
					fomen("Press Any key to Continue");
#else
					fomen("Appuyer sur une touche pour continuer");
#endif
					get() ;
				}
				break;
			case EXIT:	/* exit the session */
				CloseProcess();
				exit(0);
			default:	/* continue */
				break;
		}
	}
}

static
CloseProcess()
{
	fomcs();
	fomrt();

	close_dbh() ;

	return(0);
}

static
InitProfom(terminal)
char *terminal;
{
	strcpy( sr.termnm, terminal );
	fomin( &sr );		/* initialize the screen */
	ret( err_chk(&sr));
	return(0) ;
}

/* Initialize profom screen with high values in all fields */
static
InitScreen()
{
 	strcpy( sr.scrnam, NFM_PATH );
	strcat( sr.scrnam, SCREEN_NAME );/* fill profom screen name */

	screen.menufile[0] = HV_CHAR ;
	screen.scrhdg[0] = HV_CHAR;	/* move highs into all fields */
	screen.rundt = HV_LONG;
	screen.runtime = HV_LONG;
	screen.option1[0] = HV_CHAR;
	screen.option2[0] = HV_CHAR;
	screen.mesg[0] = screen.resp[0] = HV_CHAR;

	fomcf( 1, 1 ) ;	/* enable snap screen */
	return(0);
}

/* Scan the string and replace FILLER_CHAR s by SPACE characters */
static
FlushFillers( string )
char	*string;
{
	while( *string ){
		if( *string == FILLER_CHAR )
			*string = SPACE;
		string++;
	}
	return(0);
}

/* Read Menudata file and fill the buffer area with data */
static
ReadMenuData()	/* returns -1 on error,  or no_of_options */
{
	char *s, filename[OPTION_LEN+1]; 
	short	i, j;
	int	retval;
	FILE	*fp;

	strcpy( filename, NFM_PATH );/* Find the data file in NFM directory */
	strcat( filename, screen_stack[stackindex] );
	fp = fopen( filename, "r" );
	if( fp==NULL ){
		strcpy(e_mesg,filename);
#ifdef ENGLISH
		strcat(e_mesg, " file not found");
#else
		strcat(e_mesg, " Ne peut pas trouver ce dossier");
#endif
		fomen(e_mesg);
		get();
		return(-1);
	}
	for( ; ; ){	/* skip all blank lines in the beginning */ 
		s = fgets( e_mesg, 79, fp ); 
		if( s==NULL ) return(-1);
		if( *e_mesg=='\n' )
			continue;
		else
			break;	/* get a data line into e_mesg buffer */
	}
	sscanf( e_mesg, "%s",screen.scrhdg ); /* read into screen.scrhdg */
	CHECK(screen.scrhdg);			/* Check filler chars */
	j = (HDG_LENGTH-strlen(screen.scrhdg))/2;
	for( i=0; i<j ; i++ )
		e_mesg[i] = SPACE;	/* To position heading at centre */
	strcpy( e_mesg+i, screen.scrhdg );
	/*** e_mesg[i+strlen(screen.scrhdg)] = '\0'; ****/
	strcpy( screen.scrhdg, e_mesg );
	
	for( i=0; i<MAX_OPTIONS; i++ ){	/* for each menu option, do */
		if( (s=fgets( e_mesg, 79, fp ))==NULL )
			break;		/* if end of file , break */

		if( *e_mesg=='\n' ){	/* if blank line, i.e, seperator */
			m_item[i].index[0]    =  HV_CHAR; /* fill high values */
			m_item[i].index[1]    = '\0';
			m_item[i].name[0]     = HV_CHAR;
			m_item[i].name[1]     = '\0';
			m_item[i].type[0]     ='\0';
			m_item[i].filename[0] ='\0';
		}
		else{	/* if menu data exists in the line */
			/* read first three words and find the type */
			/* if the type is M or E, re-scanf first three */
			/* and the fourth word, which corresponds to filename */

		    sscanf( e_mesg,"%s%s%s",
			m_item[i].index, m_item[i].name, m_item[i].type );
		    if( m_item[i].type[0]=='M'||m_item[i].type[0]=='E'||
				m_item[i].type[0]=='S' ){
			sscanf( e_mesg,"%*s%*s%*s%s",m_item[i].filename );
			CHECK( m_item[i].filename );	
		    }
		    ToUpper( m_item[i].index );	/* Index always in upper case */
		    CHECK( m_item[i].index );	/* Replace ~ chars with spaces*/
		    CHECK( m_item[i].name );	
		    CHECK( m_item[i].type );	
		}
	}
	retval = i;	/* no. of options */	

	/* if no_of options is < MAX_OPTIONS , move high values to remaining */
	for( ; i<MAX_OPTIONS; i++ ){
		m_item[i].index[0] = HV_CHAR;
		m_item[i].index[1] = '\0';
		m_item[i].name[0] = HV_CHAR;
		m_item[i].name[1] = '\0';
		m_item[i].type[0] ='\0';
		m_item[i].filename[0] ='\0';
	}
	/* scan the selection indices & determine the option field length */
	optionfldlen = 1;
	for( i=0; i<MAX_OPTIONS; i++ ){
		if( strlen(m_item[i].index)>1 ){
			optionfldlen = 2;
			break;
		}
	}
	fclose(fp);

	return(retval);	/* return no_of_options */
}

ToUpper(string)
char *string;
{
	for( ; *string; string++ )
		if( *string <= 'z' && *string >= 'a' )
			*string += ('A'-'a');
	return(0);
}

/* Display the menu items after reading from the m_item buffer */
static
ShowMenu()
{
	short	i;

	strcpy( screen.menufile,screen_stack[stackindex] );
	screen.rundt = get_date() ;

	screen.runtime = HV_LONG;

	offset = (MAX_OPTIONS-no_of_options) / 2;

	/* move high values upto offset in the screen's menuitem array */
	for( i=0; i<offset; i++ ){
		screen.menuitem[i].index[0] = HV_CHAR;
		screen.menuitem[i].name[0] = HV_CHAR;
	}

	/* copy data from m_item buffer into screen's menuitem array */
	for( i=offset; i<no_of_options+offset; i++ ){
		strcpy(screen.menuitem[i].index,m_item[i-offset].index);
		strcpy(screen.menuitem[i].name,m_item[i-offset].name);
	}

	/* move high values into remaining of the screen's menuitem array */
	for( ; i<MAX_OPTIONS; i++ ){
		screen.menuitem[i].index[0] = HV_CHAR;
		screen.menuitem[i].name[0] = HV_CHAR;
	}

	screen.option1[0] = HV_CHAR;	/* Hide other fields */
	screen.option2[0] = HV_CHAR;	/* Hide other fields */
	screen.mesg[0] = HV_CHAR;
	screen.resp[0] = HV_CHAR;

	sr.nextfld = 1;			/* Write the entire screen */
	sr.endfld = 0;
	fomwr( (char *)&screen );
	ret( err_chk(&sr) );

	return(0) ;
}

static
ErrExit()
{
	CloseProcess();
	exit(-1);
}

#ifdef	USE_SEMAPHORE
GetSem()
{
	int	j;

	arg.val = 0;
	j=semctl( semid, 0, GETVAL, arg );
	if( j<0 ){
		CloseProcess() ;
#ifdef ENGLISH
		printf("Semaphore Error: %d\n",errno );
#else
		printf("Erreur de semaphore: %d\n",errno );
#endif
		exit(j);
	}
	return(j);
}
SetSem(value)
int	value;
{
	int	j;

	arg.val = value;
	j=semctl( semid, 0, SETVAL, arg );
	if( j<0 ){
		CloseProcess() ;
#ifdef ENGLISH
		printf("Semaphore Error: %d\n",errno );
#else
		printf("Erreur de semaphore: %d\n",errno );
#endif
		exit(j);
	}
	return(j);	/* it is 0 if properly set */
}
#ifdef	REM_SEMAPHORE
RemSem()
{
	int	j;

	arg.val = 0;
	j=semctl( semid, 0, IPC_RMID, arg );
	if( j<0 ){
		CloseProcess() ;
#ifdef ENGLISH
		printf("Semaphore Error: %d\n",errno );
#else
		printf("Erreur de semaphore: %d\n",errno );
#endif
		exit(j);
	}
	return(j);	/* it is 0 if properly set */
}
#endif
#endif

/* Highlight a menu option when selected */
static
HighlightOption( scr_index, value )
short	scr_index, /* index of item in screen's menuitem array */
	value;		/* ON of OFF flag: to highlight or dehighlight */
{
	short	field;

	field = (scr_index*200) + 500;
	if( last_highlighted==value )
		return(0);

	fomca1( field, REVERSE, value );
/***
	fomca1( field, BOLD, value );
	fomca1( field, BLINK, value );
***/

	last_highlighted = value;

	return(0);
}

/* Read user's option, validate and return a value depending on it's type */
static
ProcMenu()
{
	int	field_no;
	short	i, 
		propvalent = 0;	/* 1 if previous value entered is proper */

	field_no = 3600 + (optionfldlen-1)*100; /* 3600 or 3700 */
	last_sind = -1;		/* last option's screen index */
	for( ; ; ){	
		/* Read option */
		sr.nextfld = field_no;
		fomrf( (char *)&screen );
		ret( err_chk(&sr) );

		/* Scan all the indices till a match occurs */
		i=0;
		if( optionfldlen==1 ){	/* value is read into screen.option1 */
			while( strcmp(screen.option1,screen.menuitem[i].index )
				&& i<MAX_OPTIONS )
			i++;
		}
		else{		/* value is read into screen.option2 */
			while( strcmp(screen.option2,screen.menuitem[i].index )
				&& i<MAX_OPTIONS )
			i++;
		}
		if( i>=MAX_OPTIONS ){	/* means improper selection */
#ifdef ENGLISH
			fomer("Invalid selection");
#else
			fomer("Selection invalide");
#endif
			if( HighlightOption(last_sind,OFF)<0 )
				return(-1);
			last_sind = -1;
			continue;	/* Read again */
		}
		else{			/* proper selection */
			cur_sind = i;	/* current option's screen index */
			if( propvalent ){	/* If previous option proper */
				/* if user hit a return now, he confirmed it */
				/* dehighlight that option */

				if( sr.fillcode==FIL_DUP ){
					if( HighlightOption(cur_sind,OFF)<0 )
						return(-1);
					fomca1(field_no,19,0 );
					last_sind = -1;
					propvalent = 0;
				}
				else{	/* user entered different option */
					/* dehighlight previous option */
					/* and highlight the current one */

					if( HighlightOption(last_sind,OFF)<0 )
						return(-1);
					if( HighlightOption(cur_sind,ON)<0 )
						return(-1);
					last_sind = cur_sind;
					sr.nextfld = sr.endfld = field_no;
					fomud( (char *)&screen );
#ifdef ENGLISH
					fomer("Enter RETURN to confirm");
#else
					fomer("Appuyer sur RETURN pour confirmer");
#endif
					continue;
				}
			}
			else{	/* fresh selection, highlight current option */
				last_sind = cur_sind;
				if( HighlightOption(cur_sind,ON)<0 )
					return(-1);
				propvalent = 1;
				fomca1( field_no, 19, 2 );
				sr.nextfld = sr.endfld = field_no;	
				fomud( (char *)&screen );
#ifdef ENGLISH
				fomer("Enter RETURN to confirm");
#else
				fomer("Appuyer sur RETURN pour confirmer");
#endif
				continue;
			}
		}

		i = cur_sind-offset;
		switch( m_item[i].type[0] ){
			case 'P':	/* Return to prev menu type */
				if( stackindex<=0 )
					return(EXIT);
				else{
					stackindex--;
					return( MENU );
				}
			case 'I':
				if( stackindex!=0 ){
					stackindex = 0;
					return(MENU);
				}
				else
					continue;
			case 'R':
				continue;
			case 'M':
				if( stackindex < MAX_LEVELS-1 ){
					stackindex++;
					strcpy(screen_stack[stackindex],
					       m_item[i].filename );
					return(MENU);
				}

#ifdef ENGLISH
				fomen("Next menu level beyond range");
#else
				fomen("Prochain niveau de menu au-dessus des limites");
#endif
				get();
				continue;
			case 'E':
			case 'S':
				return( EXECUTE ); 
			default:
				continue;
		} 
	}
}

#ifndef	BFS
err_chk(sr)
struct stat_rec *sr;
{
	int i,n;

	if(sr->retcode == RET_ERROR)
	{
		i=sr->errno;
		n=sr->endfld;
		if(i == 88)           /* free error */
			return(NOERROR);
		fomcs();
#ifdef ENGLISH
		printf ("PROFOM Error :%d at fld :%d\n", i,n);
#else
		printf ("Erreur du PROFOM:%d a champ:%d\n", i,n);
#endif
		get();
		if (n == 40 )
#ifdef ENGLISH
			printf ("PROFOM internal error :%d fld: %d\n",i,n);
#else
			printf ("Erreur interne de PROFOM: %d a champ: %d\n",i,n);
#endif
		return(PROFOM_ERR);
	}
	return(NOERROR);
}
/*---------------------------------------------------------*/
/*   Returns system date in YYYYMMDD format */
static	long
get_date()
{
	char 		e_mesg[80];
	int		retval;
	struct	tm	*newtime;
	long	ltime ;
	long	run_date ;

	time (&ltime) ;
	newtime = localtime (&ltime) ;
	run_date = (long)newtime->tm_year * 10000 +
			(long)(++(newtime->tm_mon)) * 100 +
			(long)newtime->tm_mday ;
	if(newtime->tm_year < 70)
		run_date = 20000000 + run_date ;
	else
		run_date = 19000000 + run_date ;

	return(run_date);
}
#endif
/************************************************************/
/*                                                          */
/* Reads "tnames" file in NFM_PATH directory and gets the   */
/* terminals PROFOM name.                                   */
/*                                                          */
/* This file will be different for different installations. */
/* It contains the information of terminals and printers for*/
/* this installation. Format is                             */
/*                                                          */
/* Terminals:                                               */
/*        {Device Name} {Terminal PROFOM Name} {"0"}  */
/*                                                          */
/* Printers:                                                */
/*        {Device Name} {"PRN"} {No of Print lines}         */
/*                                                          */
/*                                                          */
/* Example:                                                 */
/*    For a Installation containg 2 terminals & 2 printers  */
/*    file looks like this                                  */
/*                                                          */
/*        /dev/console MM 0                                 */
/*        /dev/tty1 MM 0                                    */
/*        /dev/tty2 PRN 60                                  */
/*        /dev/lp PRN  60                                   */
/*                                                          */
/************************************************************/

gettname()
{
	FILE *fpt,*fopen();
	char file_nam[30];
	char devname[30];
#ifndef MS_DOS				/* if not MS_DOS */
	char *ttyname();
	register char *tmpn;
#else
	static	char	tmpn[] = "console";
#endif

	strcpy(file_nam,NFM_PATH);
	strcat(file_nam, "tnames");

	fpt=fopen(file_nam, "r");
	if(fpt == NULL){
#ifdef	MS_DOS
		/* if tnames file not available then take default termnal
		   name as "MM" */
		strcpy(sr.termnm, "MM");
		return(NOERROR);
#else
#ifdef ENGLISH
		printf("\n\nTerminal names file  \'%s\' not found\n" ,file_nam);
#else
		printf("\n\nNom du dossier du terminal \'%s\' pas trouve\n" ,file_nam);
#endif
		return(ERROR);
#endif
	}
#ifndef	MS_DOS				/* If Not MS_DOS */
	tmpn = ttyname(fileno(stdin));
#ifdef	OMEGA
	/* On Omega-58000 when you execute this program in windows ttyname()
	   returns NULL. For windows take terminal name as perq */
	if(tmpn == NULL){
		strcpy(sr.termnm, "perq");
		fclose(fpt);
		return(NOERROR);
	} else
#endif	/* OMEGA */

#endif
	for(; ;){
		if(fscanf(fpt, "%s %s %*d" ,devname,sr.termnm)<=0)break;
		if(strcmp(tmpn,devname) == 0){
			fclose(fpt);
			return(NOERROR);
		}
	}
	fclose(fpt);
#ifdef	MS_DOS
		/* if tnames file not available then take default termnal
		   name as "MM" */
		strcpy(sr.termnm, "MM");
		return(NOERROR);
#endif
	/*****
	printf("\n\nThis terminal not found in \'%s\' file\n" ,file_nam);
	******/
#ifdef ENGLISH
	printf("\n\nTerminal not set up for LedgerBase\n\n");
#else
	printf("\n\nTerminal non configurer pour LedgerBase\n\n");
#endif
	return(ERROR);
}

