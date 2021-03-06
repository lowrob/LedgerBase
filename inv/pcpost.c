/******************************************************************************
		Sourcename    : pcpost.c
		System        : Budgetary Financial System.
		Subsystem     : Inventory System 
		Module        : Physical count posting 
		Created on    : 89-09-20
		Created  By   : K HARISH.
		Cobol sources : 
*******************************************************************************
About the program:
	This program displays a pageful of stockmaster record fields, relevant
	to physical count posting, viz.,
		The stock code and description,
		The quantity before count,
		The quantity after count,
		The variance.
	The quantity before and aftercount are initially shown equal. The user
	is allowed to edit the quantity after count. The variance is updated
	by the system. After confirmation, the system posts the changes to the
	stock master records and sends  a report of changed records to the 
	printer.


HISTORY:
 Date           Programmer     Description of modification
____/__/__      __________     ___________________________

******************************************************************************/
#include <stdio.h>

#define MAIN
#define MAINFL	STMAST

#include <bfs_defs.h>
#include <bfs_recs.h>
#include <bfs_inv.h>
#include <cfomstrc.h>

#define SYSTEM		"INVENTORY"
#define MOD_DATE	"23-JAN-90"
#define SCREEN_NAME	"pcpost"
#define ESCAPE		12	/* flag indicates discontinuation of entries */
#define LINESPERPAGE	15
#define	DESC_LENGTH	16	/* Max. display length of description field */
#define	DELTA_DIFF	0.001
#define LOW 		-1
#define HIGH 		 1
#define HL_CHAR(VAL)	(VAL==HIGH) ? HV_CHAR : LV_CHAR
#define ESC_F		(sr.retcode==RET_USER_ESC && \
			(sr.escchar[0]=='F'||sr.escchar[0]=='f'))
#define ESC_H		(sr.retcode==RET_USER_ESC && \
			(sr.escchar[0]=='H'||sr.escchar[0]=='h'))
#define NO_HLP_WIN	(sr.curfld!=500 && sr.curfld!=700)

#ifdef ENGLISH
#define GETPAGE		'G'
#define NEXTPAGE	'N'
#define EXITOPT		'E'

#define EDIT		'E'
#define YES		'Y'
#else
#define GETPAGE		'O'
#define NEXTPAGE	'S'
#define EXITOPT		'F'

#define EDIT		'M'
#define YES		'O'
#endif

#define	FUND_FLD	600

struct	pc_line{
	short	s_section;	/* stock section NUMERIC 99 */
	short	s_fund;		/* stock fund NUMERIC 999 */
	char	s_code[11];	/* stock code STRING */
	char	s_desc[17];	/* stock description STRING */
	double	s_bef_cnt;	/* qty before count NUMERIC 9999999.9999 */
	double	s_aft_cnt;	/* qty after count NUMERIC 9999999.9999 */
	double	s_variance;	/* variance 9999999.9999 */
};

/* phycnt.sth - header for C structure generated by PROFOM EDITOR */
struct	pc_struct	{
	char	s_progname[11];	/* 100 program name */
	long	s_rundt;	/* 300 system date DATE YYYYFMMFDD */
	char	s_fn[2];	/* 400 function field */
	short	s_section;	/* 500 stock section number */
	short	s_stfund;	/* 600 stock fund # */
	char	s_stcode[11];	/* 700 stock code */
	short	s_editline;	/* 800 line# field for edit */
	struct pc_line	line[LINESPERPAGE]; /* 1100 to 10000 */
	char	s_mesg[78];	/* 10100 message field STRING */
	char	s_resp[2];	/* 10200 response field STRING */
};

struct	{
	short	b_fund;		/* fund # */
	char	b_code[11];	/* stock code */
	short	b_section;	/* stock section */
}	buffer;			/* to store the key of last record displayed */
struct pc_struct	s_rec;		/* screen record */
St_mast			st_mast;	/* stock master record */
St_tran			st_tran;	/* stock transaction record */
struct stat_rec 	sr;		/* profom status record */
St_sect			section;	/* stock section file */
Pa_rec			pa_rec;		/* parameter file */

char	*arayptr[4], chardate[11];	/* for report generation */

int 	retval;		/* Global variable to store function values */
char 	e_mesg[80]; 	/* to store error messages */
short	no_of_lines;	/* No of lines filled with records in current page */
double	old_aftcnt[LINESPERPAGE];	/* to store the aftcnt values read from
					   file */

/* Initialize profom fields, call entry procedures */
main( argc, argv )
int argc;
char *argv[];
{
	strncpy( SYS_NAME, SYSTEM, 50 ); /* Module name */
	strncpy( CHNG_DATE, MOD_DATE, 10 );/* Last date of change */
	
	proc_switch( argc,argv,MAINFL );/* process the switches */

	retval = CheckAccess( UPDATE, e_mesg );
	if( retval<0 ){	/* Includes NOACCESS */
		printf(e_mesg);
		return( 1 );
	}
	if( Initialize()<0 )	/* Initialize profom & report enviroment */
		exit(-1);
	retval = Process();	/* Physical count posting */
	CleanExit() ;
}

/* clear and exit the screen , close files & exit program */
CleanExit()
{
	fomcs();
	fomrt();
	close_dbh();
	rpclose();
	exit(retval);
}
/* Initialize profom and report writer */
Initialize()
{
	/* initialize the profom status variables */
	STRCPY( sr.scrnam, NFM_PATH );
	strcat( sr.scrnam, SCREEN_NAME );
	STRCPY( sr.termnm, terminal );

	/* initialize the fields and the profom screen */
	if( FillScrHdg()<0 ) 			return(-1);
	if( FillKeyFields( HIGH )<0 ) 		return(-1);
	if( FillLineNo( HIGH )<0 ) 		return(-1);
	if( FillPage( 1, LINESPERPAGE, HIGH )<0 ) 	return(-1);
	if( FillMesgRespFlds( HIGH )<0 ) 	return(-1);
	if( InitProfom()<0 ){
		fomcs();
		fomrt();
		return(-1);
	}

	/* Initialize the report writer */
	if( InitReport()<0 )
		return(-1);
	return(0);
}
/* Initialize the report writer */
InitReport()
{
	if( get_param(&pa_rec,BROWSE,1,e_mesg)<1 ){
		fomen(e_mesg);
		get();
		return(-1);
	}
	if( get_section(&section,BROWSE,1,e_mesg)<1 ){
		fomen(e_mesg);
		get();
		return(-1);
	}
	arayptr[0] = (char *)&st_mast;
	arayptr[1] = (char *)&st_tran;
	arayptr[2] = (char *)&section;
	arayptr[3] = NULL;

	mkdate( get_date(), chardate );

	/* Open report writer with following paramters */
	/* project name, logical record#, report format#, output medium,
	   discfilename, programname, date string */
	STRCPY( e_mesg, FMT_PATH );
	strcat( e_mesg, INV_PROJECT );
	retval = rpopen( e_mesg, LR_PCREP, FM_PCREP, 2, "", "pcpost",
			 chardate );
	if( retval<0 ){
#ifdef ENGLISH
		sprintf(e_mesg,"Error %d in opening report writer",retval);
#else
		sprintf(e_mesg,"Erreur %d en ouvrant le REPORT-WRITER",retval);
#endif
		fomen(e_mesg);
		get();
		return(-1);
	}
	if( (retval = rpChangetitle(1,pa_rec.pa_co_name))<0 ){
#ifdef ENGLISH
		sprintf(e_mesg,"Error %d in rpChangetitle",retval );
#else
		sprintf(e_mesg,"Erreur %d dans rpChangetitle",retval );
#endif
		fomen( e_mesg );
		get();
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
	s_rec.s_section = value * HV_SHORT;
	s_rec.s_stfund = value * HV_SHORT;
	s_rec.s_stcode[0] = HL_CHAR(value);

	return(0);
}
/* Fill the field# field with high/low values */
FillLineNo( value )
short value;
{
	s_rec.s_editline = value * HV_SHORT;
	return(0);
}
/* Fill the non key fields with high/low values */
FillPage( from, to, value )
short from, to, value;	/* from, to : line numbers. */
{
	short	i;

	for( i=from-1; i<=to-1; i++ ){
		s_rec.line[i].s_section = value * HV_SHORT;
		s_rec.line[i].s_fund = value * HV_SHORT;
		s_rec.line[i].s_code[0] = HL_CHAR(value);
		s_rec.line[i].s_desc[0] = HL_CHAR(value);
		s_rec.line[i].s_bef_cnt = value * HV_DOUBLE;	
		s_rec.line[i].s_aft_cnt = value * HV_DOUBLE;
		s_rec.line[i].s_variance = value * HV_DOUBLE;
	}
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
Process()
{
	/* Initialize the key fields to zeros. Used if seq. search is made */
	s_rec.s_section = 0;
	s_rec.s_stfund = 0;
	s_rec.s_stcode[0] = '\0';

	for( ; ; ){
		if( ReadFunction()<0 ) return(-1);
		switch( s_rec.s_fn[0] ){
			case GETPAGE:	/* Get pageful of recs from given key */
			case NEXTPAGE:	/* Get nextpageful of recs */
				CHKACC(retval,UPDATE,e_mesg);
				if( GetPage()<0 ) return(-1);
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
ReadFunction()	/* Display options at the bottom, and read entry */
{
#ifdef ENGLISH
	fomer("G(et page), N(ext page), E(xit)");
#else
	fomer("O(btenir page), S(uivant), F(in)");
#endif
	sr.nextfld = 400;	/* Fn field number */
	fomrf( (char *)&s_rec );
	ret( err_chk(&sr) );
	return(0);
}
GetPage()	/* Get a pageful of stock items from given key */
{
	if( s_rec.s_fn[0]==GETPAGE ){	/* Get pageful from a given key */
		if( (retval=ReadKeyFlds())<0 || retval==ESCAPE )
			return(retval); 
		flg_reset(STMAST);
	}

	if( (retval=ReadPageful())<0 )
		return(-1);
	if( retval==ESCAPE )
		return( retval );

	if( ShowPage()<0 )
		return(-1);

	if( EditFields()<0 )
		return(-1);

	if( HideMessage()<0 )
		return(-1);

	return(0);
}
/* Read stock master key fields, the fund and stock item code */
ReadKeyFlds()
{
	if( FillKeyFields(LOW)<0 ) return(-1);
	fund_default();
	if( (retval=ReadFields(500,700))<0 )
		return(retval);

	st_mast.st_section = s_rec.s_section;
	st_mast.st_fund = s_rec.s_stfund;
	STRCPY( st_mast.st_code, s_rec.s_stcode );

	return(0);
}
ReadPageful()
{
	int	i;

	for( i=0; i<LINESPERPAGE; i++ ){
		retval=get_n_stmast( &st_mast, BROWSE, 1, FORWARD, e_mesg );
		if( retval==ERROR ){
			DispError();
			return(-1);
		}
		else if( retval==EFL )
			break;
		else{
			ReadLine(i);
		}
	}
	seq_over( STMAST );
	if( i==0 ){
#ifdef ENGLISH
		fomen("No more items to be displayed...");
#else
		fomen("Plus d'articles a afficher...");
#endif
		get();
		return( ESCAPE );
	}
	no_of_lines = i;	/* store the no. of records read */

	/* store the key of the last record read */
	if( i>0 ){
		buffer.b_fund = st_mast.st_fund;
		STRCPY( buffer.b_code, st_mast.st_code );
		buffer.b_section = st_mast.st_section;
	}

	return(0);
}
ShowPage()
{
	if( FillPage( no_of_lines+1, LINESPERPAGE, HIGH )<0 )
		return(-1);
	if( WriteFields( 1100, 10000 )<0 )
		return(-1);
	return(0);
}
EditFields()
{
    for( ; ; ){
#ifdef ENGLISH
	if( DisplayMessage("E(dit), Y(es)")<0 )
#else
	if( DisplayMessage("M(odifier), O(ui)")<0 )
#endif
		return(-1);

	for ( ; ; ){
		if(GetResponse()<0 )
			return(-1);
		if( s_rec.s_resp[0]==EDIT || s_rec.s_resp[0]==YES )	
			break;
	}

	switch( s_rec.s_resp[0] ){
		case EDIT:
			if( FldEdit()<0 )
				return(-1);
			break;
		case YES:
			if( SaveChanges()<0 )
				return(-1);
			return(0);
		default:
			break;
	}
    }
}
ReadLine(i)
short	i;
{
	s_rec.line[i].s_section = st_mast.st_section;
	s_rec.line[i].s_fund = st_mast.st_fund;
	STRCPY( s_rec.line[i].s_code, st_mast.st_code );
	strncpy( s_rec.line[i].s_desc, st_mast.st_desc, DESC_LENGTH );
		s_rec.line[i].s_desc[DESC_LENGTH] = '\0';
	s_rec.line[i].s_bef_cnt = st_mast.st_bef_cnt;
	s_rec.line[i].s_aft_cnt = old_aftcnt[i] = st_mast.st_aft_cnt;
	s_rec.line[i].s_variance = s_rec.line[i].s_aft_cnt
				 - s_rec.line[i].s_bef_cnt;
}

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
WriteFields( start,end )	/* write the given range of fields */
int start, end;			/* start & end profom field numbers */
{
	sr.nextfld = start;
	sr.endfld = end;
	fomwr( (char *)&s_rec );
	ret( err_chk(&sr) );
	return(0);
}
Validate()	/* Validate the values entered by the user */
{
	Gl_rec temp_rec;
	int index;

	switch( sr.curfld ){
		case 500:	/* Stock section number */
			if( ESC_H ){
				retval = sect_hlp( &s_rec.s_section, 6,13 );
				if( retval==DBH_ERR )
					return( retval );
				else if( retval>=0 )
					redraw();
			}
		case 700:	/* stock item code: check reading master rec */
			if( ESC_H ){
				retval = stock_hlp( 
					s_rec.s_stfund,s_rec.s_stcode, 7,13 );
				if( retval==DBH_ERR )
					return( retval );
				else if( retval>=0 )
					redraw();
			}
			break; 
		case 800:	/* line# for editing */
			if( sr.fillcode==FIL_OMITTED )	/* Nothing entered */
				return(ESCAPE);
			if(s_rec.s_editline<1 || s_rec.s_editline>no_of_lines){
#ifdef ENGLISH
				fomer("Line number beyond range");
#else
                                fomer("Numero de ligne au-dela des limites");
#endif
				s_rec.s_editline = LV_SHORT;
			}
			break;
		default:
			break;
	}
	sr.nextfld = sr.curfld;
	return(0);
}
DisplayMessage(mesg)	/* Display the given message in the message field */
char *mesg;
{
	STRCPY( s_rec.s_mesg, mesg );
	if( WriteFields(10100,10100)<0 )	return(-1);
	return(0);
}
GetResponse()
{
	s_rec.s_resp[0] = LV_CHAR;
	return ( ReadFields(10200,10200) );
}
HideMessage()	/* Hide the message & response fields */
{
	if( FillMesgRespFlds(HIGH)<0 )	return(-1);
	if( WriteFields(10100,10200)<0 ) return(-1);
	return(0);
}
HideLineNo()	/* Hide the 'Line#' field */
{
	if( FillLineNo(HIGH)<0 )	return(-1);
	if( (WriteFields(800,800))<0 )
		return(-1);
	return(0);
}
FldEdit()	/* Read the field number and read corresponding field */
{
	int firstfld,lastfld;

	for( ; ; ){
		/* Read number of field to be changed */
		if( FillLineNo(LOW)<0 )	return(-1);
		if( (retval = ReadFields(800,800))<0 ) return(-1);
		if( retval==ESCAPE ){
			if( HideLineNo()<0 ) return(-1);
			break;
		}
		/* validate the field# entry */
		if( s_rec.s_editline<1 || s_rec.s_editline>no_of_lines ){
#ifdef ENGLISH
			fomer("Cannot access specified field");
#else
			fomer("Ne peut pas acceder au champ specifie");
#endif
			continue;
		}
		/* allow value to be changed */
		retval = ModifyField( s_rec.s_editline );
		if( retval<0 )	return(retval);
		if( retval==ESCAPE ){
			if( HideLineNo()<0 ) return(-1);
			break;
		}
	}
	return(0);
}
ModifyField( line )	/* Read & change the specified field */
int line;	/* only 1 field can be changed in any line */
{
	int	field;

	/* Get the  profom field# for aftercount field of given line# */
	field = 1500 + (line-1)*600;

	/* Do the following for the field */
	fomca1( field,19,2);		/* enable dup buffers */
	fomca1( field,10,0);		/* disable escape flag */
	sr.nextfld = sr.endfld = field;
	fomud( (char *)&s_rec);		/* Update dup buffers */

	/* read the new value from the user */
	s_rec.line[line-1].s_aft_cnt = LV_DOUBLE;
	retval = ReadFields( field, field ); 
	if( retval<0 || retval==ESCAPE )	return(retval);

	fomca1( field,19,0);	/* disable dup buffers */
	fomca1( field,10,1);	/* enable escape flag */

	/* Recalculate the variance */
	s_rec.line[line-1].s_variance = 
		s_rec.line[line-1].s_aft_cnt-s_rec.line[line-1].s_bef_cnt;
	if( WriteFields(field+100, field+100)<0 )
		return(-1);

	return(0);
}
/* Write the changes to stock master and write report line */
SaveChanges()
{
	short 	i, atleast_one_write = 0;

	retval = 0;
	/* Do the following for all the lines displayed in the page */
	for( i=0; i<no_of_lines; i++ ){
		/* Consider the line only if any change is made */
		if( s_rec.line[i].s_aft_cnt != old_aftcnt[i] ){
			/* prepare key and read stock master record */
			st_mast.st_fund = s_rec.line[i].s_fund;
			STRCPY( st_mast.st_code,s_rec.line[i].s_code );
			retval = get_stmast(&st_mast,UPDATE,0,e_mesg);
			if( retval==ERROR ){
				DispError();
				break;
			}
			else if( retval==LOCKED ){
				fomen(e_mesg);
				get();
				break;
			}
			else if( retval==UNDEF ){
				fomen(e_mesg);
				get();
				break;
			}

			/* record obtained properly: modify aft_count field */
			st_mast.st_aft_cnt = s_rec.line[i].s_aft_cnt;
			/* write the record */
			if( put_stmast(&st_mast,UPDATE,e_mesg) <0 ){
				fomen(e_mesg); get();
				break;
			}
			STRCPY( st_tran.st_remarks,
				section.name[s_rec.line[i].s_section-1] );
			atleast_one_write = 1;
			/* Write a line to the report writer */
			if( (retval=rpline(arayptr))<0 ){
				sprintf(e_mesg, "Rp Error: %d",retval );
				fomen(e_mesg);	
				get();
				break;
			}
		}
	}
	if( retval<0 ){
		roll_back(e_mesg);
		return(retval);
	}
	/*   should  atleast_one_write be checked ?? */
	else if( atleast_one_write  &&  commit(e_mesg)<0 ){
		fomen(e_mesg);
		get();
		return(-1);
	}
	/* start the key for next page call */
	if( no_of_lines>0 ){
		st_mast.st_section = buffer.b_section;
		st_mast.st_fund = buffer.b_fund;
		STRCPY( st_mast.st_code,buffer.b_code );
		if( inc_str(st_mast.st_code, 10, FORWARD)<0 )
			return(-1);
		flg_reset( STMAST );
	}

	return(0);
}
DispError()
{
	if(DisplayMessage(e_mesg)<0)
		return(-1);
#ifdef ENGLISH
	fomen("Press any key");
#else
	fomen("Appuyer sur une touche");
#endif
	get();
	sprintf(e_mesg,"iserror: %d, dberror: %d, errno: %d",
			iserror, dberror, errno );
	if(DisplayMessage(e_mesg)<0)
		return(-1);
#ifdef ENGLISH
	fomen("Press any key");
#else
	fomen("Appuyer sur une touche");
#endif
	get();
	return(HideMessage());
}

fund_default()
{
	fomca1(FUND_FLD,19,2);
	s_rec.s_stfund= 1;
	WriteFields(FUND_FLD,FUND_FLD);
	s_rec.s_stfund = LV_SHORT;
}

