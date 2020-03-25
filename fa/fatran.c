/******************************************************************************
		Sourcename    : fatran.c
		System        : Budgetary Financial System.
		Subsystem     : Fixed Assets System 
		Module        : Fixed Assets maintenance 
		Created on    : 89-10-24
		Created  By   : JON PRESCOTT
		Cobol sources : 

HISTORY:
 Date           Programmer     Description of modification
____/__/__      __________     ___________________________
1990/11/19      J. Cormier     added OBSOLETE condition
1991/02/06	J.Prescott     added Item description
******************************************************************************/
#include <stdio.h>

#define MAIN
#define MAINFL	FATRAN

#include <bfs_defs.h>
#include <bfs_recs.h>
#include <cfomstrc.h>
#include <bfs_fa.h>

#define SYSTEM		"FIXED ASSETS"
#define MOD_DATE	"23-JAN-90" 
#define SCREEN_NAME	"fatran"
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
#define NO_HLP_WIN	(sr.curfld!=700 && sr.curfld!=800 &&  \
				sr.curfld!=1000 && sr.curfld!=1100)

#ifdef ENGLISH
#define ADDREC		'A'
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
#define NEXT		'S'
#define PREV		'P'
#define INQUIRE		'I'
#define EXITOPT		'F'

#define EDIT		'M'
#define CANCEL		'A'
#define YES		'O'
#define NO		'N'
#endif

/* fa_maint.sth - header for C structure generated by PROFOM EDITOR */

struct	fa_struct	{
	char	s_progname[11];	/* 100 program name */
	long	s_rundt;	/* 300 system date */
	char	s_fn[2];	/* 400 function */
	short	s_tran_numb;	/* 500 transfer # */
	short	s_fld_no;	/* 600 field# field for editing */
	short	s_costcen;	/* 700 cost centre# */
	long	s_itemid;	/* 800 item id */
	char	s_itemdesc[36];	/* 850 item description */
	short	s_frmcostcen;	/* 900 from cost center */
	short	s_tocostcen;	/* 1000 to cost center */
	char	s_todept[5];	/* 1100 to dept code */
	char	s_toroomno[6];	/* 1200 to room no */
	long	s_trandate;	/* 1300 date of receipt */
	char	s_cond[2];	/* 1400 condition code */
	char	s_condexp[10];	/* 1500 condition explanation */
	char	s_remarks[25];	/* 1600 transfer remarks */
	char	s_mesg[78];	/* 1700 message field */
	char	s_resp[2];	/* 1800 response field */
};
struct fa_struct	s_rec;		/* screen record */
struct stat_rec 	sr;		/* profom status record */

Sch_rec		sch_rec;	/* School record */
Fa_transfer	fa_tran;	/* Fixed Asset Transfer file */
Fa_rec		fa_mast;	/* Fixed Asset Item master file */
Fa_dept		fa_dept;	/* Fixed Asset Department file */
Pa_rec		pa_rec;		/* Parameter file */

int retval;	/* Global variable to store function values */
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
	retval = get_param(&pa_rec, BROWSE, 1, e_mesg);
	if(retval < 0) {
		fomer(e_mesg);
		get();
		CleanExit();
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
	if( FillScrHdg()<0 ) 			return(-1);
	if( FillKeyFields( HIGH )<0 ) 		return(-1);
	if( FillField( HIGH )<0 ) 		return(-1);
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
	s_rec.s_tran_numb = value * HV_SHORT;

	return(0);
}
/* Fill the field# field with high/low values */
FillField( value )
short value;
{
	s_rec.s_fld_no = value * HV_SHORT;
	return(0);
}
/* Fill the non key fields with high/low values */
FillNonKeyFlds( value )
short value;
{
	s_rec.s_costcen		= value * HV_SHORT;
	s_rec.s_itemid		= value * HV_LONG;
	s_rec.s_itemdesc[0]	= HL_CHAR(value);
	s_rec.s_frmcostcen	= value * HV_SHORT;
	s_rec.s_tocostcen	= value * HV_SHORT;
	s_rec.s_todept[0]	= HL_CHAR(value);
	s_rec.s_toroomno[0]	= HL_CHAR(value);
	s_rec.s_trandate	= value * HV_LONG;
	s_rec.s_cond[0] 	= HL_CHAR(value);
	s_rec.s_condexp[0] 	= HL_CHAR(value);
	s_rec.s_remarks[0]	= HL_CHAR(value);

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
	s_rec.s_tran_numb = 0;

	for( ; ; ){
		if( ReadFunction()<0 ) return(-1);
		switch( s_rec.s_fn[0] ){
			case ADDREC:	/* add a record */
				CHKACC(retval,ADD,e_mesg);
				retval = AddRecord();
				roll_back(e_mesg);
				if( retval<0 )
					return(-1);
				break;
			case NEXT:	/* show next record sequence */
				CHKACC(retval,BROWSE,e_mesg);
				if( Inquiry(SEQUENTIAL,FORWARD)<0) return(-1);
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

	for( i=firstfld; i<=lastfld; i+=50 )
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
	fomer("A(dd), N(ext), P(rev), I(nquire), E(xit)");
#else
	fomer("R(ajouter), S(uivant), P(recedent), I(nterroger), F(in)");
#endif
	sr.nextfld = 400;	/* Fn field number */
	fomrf( (char *)&s_rec );
	ret( err_chk(&sr) );
	return(0);
}
/* Add a stock master record */
AddRecord()
{
	if(pa_rec.pa_cur_period == 0) {
#ifdef ENGLISH
		fomer("Not Allowed Before Yearly Closing...");
#else
		fomer("Pas permis avant la fermeture annuelle...");
#endif
		get();
		return(0);
	}
	s_rec.s_cond[0] = '\0' ;
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
	if( (retval=ReadFields(700,1600))<0 )
		return(retval);
	if(retval==ESCAPE){
		if(ClearScreen()<0)	return(-1);
		return(retval);
	}

	/* Allow the user to edit entries, if required, before saving */
	if( (retval=EditFlds())<0 )	return(retval);
	if( retval==ESCAPE ){
		if(ClearScreen()<0)	return(-1);
		return(retval);
	}

	/* Write the record, in ADD mode, to fa transaction file */
	if( WriteSession( ADD )<0 )
		return(-1);
	return(0);
}
/* Read stock master key fields, the fund and stock item code */
RdKeyFlds()
{
#ifdef ORACLE
	long	sno, get_maxsno();
#endif
#ifndef ORACLE
	fa_tran.fatr_numb = HV_SHORT;
	flg_reset(FATRAN);
	retval = get_n_fatran(&fa_tran,BROWSE,0,BACKWARD,e_mesg);
	seq_over(FATRAN);
	if(retval==ERROR) {
		fomen(e_mesg); get(); return(retval);
	}
	if(retval==EFL) s_rec.s_tran_numb = 1;
	else s_rec.s_tran_numb = fa_tran.fatr_numb+1;	
#else
	/* get next sequential number on last part */
	sno = get_maxsno(FATRAN,(char *)&fa_tran,0,-1,e_mesg);
	if(sno < 0) {
		fomen(e_mesg); 
		get(); 
		return(sno);
	}
	s_rec.s_tran_numb = sno + 1;
#endif
	if(WriteFields(500,500)<0) return(-1);
	return(0);
}
static
ReadFields(start,end)	/* read the given range of fields */
{
	sr.nextfld = start;
	sr.endfld = end;
	for( ; ; ){	/* Do in a loop */
		fomrd( (char *)&s_rec );	/* Profom call */
		ret(err_chk(&sr));		/* Check for profom error */
		if( sr.retcode==RET_USER_ESC || sr.retcode==RET_VAL_CHK ){
		    if( sr.retcode==RET_USER_ESC ){
			if( sr.escchar[0]=='F' || sr.escchar[0]=='f' )
				return( ESCAPE );
			else if( sr.escchar[0]=='H' || sr.escchar[0]=='h' ){
				if( NO_HLP_WIN )
					continue;
			}
			else
				continue;
		    }
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
	if(FillField(HIGH)<0 ) return(-1);
	if(FillNonKeyFlds(HIGH)<0 ) return(-1);
	if(FillMesgRespFlds(HIGH)<0 ) return(-1);

	if( WriteFields(700,1600)<0 ) return(-1);
	
	return(0);
}
Validate()	/* Validate the values entered by the user */
{
	int save_nextfld, save_endfld;

	switch( sr.curfld ){
		case 500:	/* transfer # */
			if( ESC_H ){
#ifdef ENGLISH
				fomen("No help available on item"); get();
#else
				fomen("Pas d'assistance disponible pour l'article"); get();
#endif
			}
			fa_tran.fatr_numb = s_rec.s_tran_numb;
			retval = get_fatran( &fa_tran,BROWSE,0,e_mesg);
			if( retval==ERROR ){
				fomen(e_mesg); get();
				return(-1);
			}
			if( retval!=NOERROR ){	/* record doesn't exist */
				fomer(e_mesg);
				s_rec.s_tran_numb = LV_SHORT; 
			}
			break;
		case 600:	/* field # for editing */
			if( sr.fillcode==FIL_OMITTED )	/* Nothing entered */
				return(ESCAPE);
			break;
		case 700:	/* field # for original cost center */
			if( ESC_H ){
				retval = sch_hlp( &s_rec.s_costcen, 7,13 );
				if( retval==DBH_ERR )
					return( retval );
				else if( retval>=0 )
					redraw();
				if( retval==0 )
					break;
			}
			sch_rec.sc_numb = s_rec.s_costcen;
			retval = get_sch( &sch_rec, BROWSE, 0, e_mesg );
			if( retval==ERROR ){
				fomen(e_mesg); get();
				return(-1);
			}
			if( retval!=NOERROR ){	/* record doesn't exist */
				fomer(e_mesg);
				s_rec.s_costcen = LV_SHORT; 
			}
			else
				fomer( sch_rec.sc_name );
			break;
		case 800:	/* fixedasset itemid: check by reading master */
			if( ESC_H ){
				retval = faitem_hlp( 
					s_rec.s_costcen,&s_rec.s_itemid, 7,13 );
				if( retval==DBH_ERR )
					return( retval );
				else if( retval>=0 )
					redraw();
				if( retval==0 )
					break;
			}
			fa_mast.fa_costcen = s_rec.s_costcen;
			fa_mast.fa_itemid = s_rec.s_itemid;
			retval = get_famast( &fa_mast, BROWSE, 0, e_mesg );
			if( retval==ERROR ){
				fomen(e_mesg);get();
				return(retval);
			}
			if( retval!=NOERROR ){ /* no record exists */
				fomer(e_mesg);
				s_rec.s_itemid = LV_LONG; 
			} 
			else{
				STRCPY(s_rec.s_itemdesc,fa_mast.fa_desc);
				s_rec.s_frmcostcen = fa_mast.fa_curcostcen;
			}
			break; 
		case 1000:	/* field # for destination cost center */
			if( ESC_H ){
				retval = sch_hlp( &s_rec.s_tocostcen, 7,13 );
				if( retval==DBH_ERR )
					return( retval );
				else if( retval>=0 )
					redraw();
				if( retval==0 )
					break;
			}
			sch_rec.sc_numb = s_rec.s_tocostcen;
			retval = get_sch( &sch_rec, BROWSE, 0, e_mesg );
			if( retval==ERROR ){
				fomen(e_mesg); get();
				return(-1);
			}
			if( retval!=NOERROR ){	/* record doesn't exist */
				fomer(e_mesg);
				s_rec.s_tocostcen = LV_SHORT; 
			}
			else
				fomer( sch_rec.sc_name );
			break;
		case 1100:	/* destination dept no */
			if( ESC_H ){
				retval = fadept_hlp( s_rec.s_todept, 7,13 );
				if( retval==DBH_ERR )
					return( retval );
				else if( retval>=0 )
					redraw();
				if( retval==0 )
					break;
			}
			STRCPY( fa_dept.code, s_rec.s_todept );
			if( get_fadept(&fa_dept,BROWSE,0,e_mesg)<0 ){
				fomer(e_mesg);
				s_rec.s_todept[0] = LV_CHAR; 
			}
			else
				fomen(fa_dept.desc);
			break;
		case 1200:	/* room no validate */
				/*only sets default date of transfer */
			if (s_rec.s_toroomno[0] == '\0')
                            s_rec.s_toroomno[0] = ' ';
			if(s_rec.s_fn[0] == ADDREC) {
				save_nextfld = sr.nextfld;
				save_endfld = sr.endfld;
				s_rec.s_trandate = s_rec.s_rundt;
				SetDupBuffers(1300,1300,1);
				s_rec.s_trandate = LV_LONG;
				sr.nextfld = save_nextfld ;
				sr.endfld = save_endfld ; 
			}
			break;
		case 1300:	/* date of transfer */
			if( s_rec.s_trandate> s_rec.s_rundt ){
#ifdef ENGLISH
				fomer("Date can't exceed current date");
#else
				fomer("Date ne peut pas etre plus tard que la date courante");
#endif
				s_rec.s_trandate = LV_LONG;
			}
			if(s_rec.s_fn[0] == ADDREC) {
				save_nextfld = sr.nextfld;
				save_endfld = sr.endfld;
				SetDupBuffers(1300,1300,0);
				STRCPY(s_rec.s_cond,fa_mast.fa_cond);
				if(set_condexp(s_rec.s_cond[0],
							s_rec.s_condexp)<0)
					return(-1);
				SetDupBuffers(1400,1400,1);
				s_rec.s_cond[0] = LV_CHAR;
				sr.nextfld = save_nextfld ;
				sr.endfld = save_endfld ; 
			}
			break;
		case 1400:	/* fa item condition */
			if( s_rec.s_cond[0] != CD_EXCELLENT &&
			    s_rec.s_cond[0] != CD_GOOD &&
			    s_rec.s_cond[0] != CD_FAIR &&
			    s_rec.s_cond[0] != CD_POOR &&
			    s_rec.s_cond[0] != CD_OBSOLETE ){
#ifdef ENGLISH
			fomer(" E(xcellent), G(ood), F(air), P(oor), O(bsolete)");
#else
         		fomer(" E(xcellent), B(on), P(assable), M(auvais), O(bsolete)");
#endif
				s_rec.s_cond[0] = LV_CHAR;
			}
			else{
			if(set_condexp(s_rec.s_cond[0],s_rec.s_condexp)<0)
				return(-1);
			}
			break;
		default:
			break;
	}
	sr.nextfld = sr.curfld;
	return(0);
}
EditFlds()	/* Ask if user wants to edit fields before saving */
{
	for( ; ; ){
#ifdef ENGLISH
		if(DisplayMessage( "Y(es), E(dit), C(ancel)")<0 ) 
#else
		if(DisplayMessage( "O(ui), M(odifier), A(nnuler)")<0 ) 
#endif
			return(-1);
		sr.nextfld = 1800;
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
				sr.nextfld = 1800;	/* response field */
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
	if( WriteFields(1700,1700)<0 )	return(-1);
	return(0);
}
HideMessage()	/* Hide the message & response fields */
{
	if( FillMesgRespFlds(HIGH)<0 )	return(-1);
	if( WriteFields(1700,1800)<0 ) return(-1);
	return(0);
}
HideFldNo()	/* Hide the 'Field#' field */
{
	if( FillField(HIGH)<0 )	return(-1);
	if( (WriteFields(600,600))<0 )
		return(-1);
	return(0);
}
FldEdit()	/* Read the field number and read corresponding field */
{
	int firstfld,lastfld;

	for( ; ; ){
		/* Read number of field to be changed */
		if( FillField(LOW)<0 )	return(-1);
		if( (retval = ReadFields(600,600))<0 ) return(-1);
		if( retval==ESCAPE ){
			if( HideFldNo()<0 ) return(-1);
			break;
		}
		switch(s_rec.s_fld_no){
			case 1: 	/* original cost center */
				firstfld = 700;
				lastfld = 900;
				break;
			case 2: 	/* item id */
				firstfld = 800 ;
				lastfld = 900;
				break;
			case 4: 	/* destination cost center */
				firstfld = lastfld = 1000;
				break;
			case 5:		/* destination dept code */
				firstfld = lastfld = 1100;
				break;
			case 6: 	/* destination room no */
				firstfld = lastfld = 1200;
				break;
			case 7:		/* transfer date */
				firstfld = lastfld = 1300;
				break;
			case 8: 	/* condition */
				firstfld = 1400;
				lastfld = 1500;
				break;
			case 9: 	/* transfer remarks */
				firstfld = lastfld = 1600;
				break;
#ifdef ENGLISH
			default: fomer("Can't change specified field");
#else
			default: fomer("Ne peut pas changer le champ specifie");
#endif
				continue;
		}
		retval = ModifyField(firstfld, lastfld);
		if( retval<0 )	return(retval);
		if( retval==ESCAPE ){
			if( HideFldNo()<0 ) return(-1);
			break;
		}
	}
	return(0);
}
ModifyField( firstfld, lastfld )	/* Read & change the specified fields */
int firstfld,lastfld;
{
	int i;

	for( i=firstfld; i<=lastfld; i+=50 ){
		fomca1( i,19,2);	/* enable dup buffers */
		fomca1( i,10,0);	/* disable escape flag */
	}
	sr.nextfld = firstfld;
	sr.endfld = lastfld;
	fomud( (char *)&s_rec);		/* Update dup buffers */
	/* reset because fomud resets endfld to zero */
	sr.nextfld = firstfld;
	sr.endfld = lastfld;
	switch(firstfld){
		case 700:	/* original cost center */
			s_rec.s_costcen = LV_SHORT;
			s_rec.s_itemid = LV_LONG;
			s_rec.s_itemdesc[0] = LV_CHAR;
			break;
		case 800:	/* item id */
			s_rec.s_itemid = LV_LONG;
			s_rec.s_itemdesc[0] = LV_CHAR;
			break;
		case 1000:	/* to cost center */
			s_rec.s_tocostcen = LV_SHORT;
			break;
		case 1100:	/* to department code */
			s_rec.s_todept[0] = LV_CHAR;
			break;
		case 1200:	/* to room no */
			s_rec.s_toroomno[0] = LV_CHAR;
			break;
		case 1300:	/* transfer date */
			s_rec.s_trandate = LV_LONG;
			break;
		case 1400:	/* condition # */
			s_rec.s_cond[0] = LV_CHAR;
			break;
		case 1600:	/* transfer remarks */
			s_rec.s_remarks[0] = LV_CHAR;
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
int	mode;		/* ADD, UPDATE */
{
	fa_tran.fatr_numb = s_rec.s_tran_numb;
	fa_tran.fatr_costcen = s_rec.s_costcen;
	fa_tran.fatr_itemid = s_rec.s_itemid;
	fa_tran.fatr_frcostcen = s_rec.s_frmcostcen;
	fa_tran.fatr_tocostcen = s_rec.s_tocostcen;
	STRCPY(fa_tran.fatr_todept,s_rec.s_todept);
	STRCPY(fa_tran.fatr_toroomno,s_rec.s_toroomno);
	fa_tran.fatr_date = s_rec.s_trandate;
	STRCPY(fa_tran.fatr_cond, s_rec.s_cond );
	STRCPY(fa_tran.fatr_remarks, s_rec.s_remarks);

	retval = put_fatran( &fa_tran, mode, e_mesg );
	if( retval!=NOERROR ){
		fomen(e_mesg);get();
		roll_back(e_mesg);
		return(retval);
	}
	fa_mast.fa_costcen = s_rec.s_costcen;
	fa_mast.fa_itemid = s_rec.s_itemid;
	retval = get_famast(&fa_mast, UPDATE, 0, e_mesg );
	if(retval!=NOERROR){
		fomen(e_mesg); get();
		roll_back(e_mesg);
		return(retval);
	}
	fa_mast.fa_curcostcen = s_rec.s_tocostcen;
	fa_mast.fa_cond[0] = s_rec.s_cond[0];
	retval = put_famast( &fa_mast, UPDATE, e_mesg );
	if( retval!=NOERROR ){
		fomen(e_mesg);get();
		roll_back(e_mesg);
		return(retval);
	}
	if( commit(e_mesg)<0 ){
		fomen(e_mesg);
		get();
		return(-1);
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
		if( retval==UNDEF || retval==ESCAPE )
			return( retval=ESCAPE );
	}
	else{	/* access is sequential, so get next record from file */
		retval = GetNextRec( direction );	/* Get next rec */
		if(retval==EFL)
			return(retval=0);
	}
	if( retval< 0 ){	/* errors in reading */
		fomen(e_mesg);
		get();
		return(-1);
	}
	
	retval = DisplayRecord();	/* Display the header & item records */
	if( retval<0 ) return(retval);

	return(0);
}
DisplayRecord()	/* Get and display the header and item records */
{
	s_rec.s_tran_numb - fa_tran.fatr_numb;
	s_rec.s_costcen = fa_tran.fatr_costcen ; 
	s_rec.s_itemid = fa_tran.fatr_itemid ; 

	/* get item id description */
	fa_mast.fa_costcen = s_rec.s_costcen;
	fa_mast.fa_itemid = s_rec.s_itemid;
	retval = get_famast( &fa_mast, BROWSE, 0, e_mesg );
	if( retval!=NOERROR ){
		if(retval==UNDEF)
			STRCPY(s_rec.s_itemdesc,"????????????");
		else {
			fomen(e_mesg);get();
			return(retval);
		}
	}
	else
		STRCPY(s_rec.s_itemdesc,fa_mast.fa_desc);

	s_rec.s_frmcostcen = fa_tran.fatr_frcostcen ;
	s_rec.s_tocostcen = fa_tran.fatr_tocostcen ;
	STRCPY(s_rec.s_todept,fa_tran.fatr_todept);
	STRCPY(s_rec.s_toroomno,fa_tran.fatr_toroomno);
	s_rec.s_trandate = fa_tran.fatr_date;
	STRCPY( s_rec.s_cond, fa_tran.fatr_cond );
	switch( s_rec.s_cond[0] ){
		case CD_EXCELLENT:
				STRCPY( s_rec.s_condexp,EXCELLENT );
				break;
		case CD_GOOD:
				STRCPY( s_rec.s_condexp,GOOD );
				break;
		case CD_FAIR:
				STRCPY( s_rec.s_condexp,FAIR );
				break;
		case CD_POOR:
				STRCPY( s_rec.s_condexp,POOR );
				break;
		case CD_OBSOLETE:
				STRCPY( s_rec.s_condexp,OBSOLETE );
				break;
		default:	
				s_rec.s_condexp[0] = HV_CHAR;
				break;
	}
	STRCPY(s_rec.s_remarks,fa_tran.fatr_remarks);
	if( WriteFields(700,1600) < 0 )
		return(-1);
	return(0);
}
GetNextRec(direction)	/* Read the next record in the specified direction */
int	direction;
{
	if( flg_start(FATRAN)!=direction ){ 	/* file access mode changed */
		fa_tran.fatr_numb = s_rec.s_tran_numb;
		if( direction==FORWARD )
			fa_tran.fatr_numb++;
		else
			fa_tran.fatr_numb--;
		flg_reset(FATRAN);
	}
	/* Read the next record from stmast file */
	retval = get_n_fatran( &fa_tran, BROWSE, 0, direction, e_mesg );
#ifndef ORACLE
	seq_over( FATRAN );
#endif
	if( retval==EFL ){
#ifdef ENGLISH
		fomen("No More Records....");
#else
		fomen("Plus de fiches....");
#endif
		get();
		flg_reset(FATRAN);
		return(EFL);
	}
	if( retval!=NOERROR ){
		fomen(e_mesg);
		get();
		return(retval);
	}
	/* Write the key fields on to the screen */
	s_rec.s_tran_numb = fa_tran.fatr_numb;
	if( WriteFields(500,500)<0 )
		return(-1);
	return(0);
}
GetRecord()	/* Read the header key values, get terminal info ,read rec */
{
	s_rec.s_tran_numb = LV_SHORT;
	if( (retval=ReadFields(500,500))<0 || retval==ESCAPE ) 
		return( retval );
	fa_tran.fatr_numb = s_rec.s_tran_numb;
	retval = get_fatran( &fa_tran, BROWSE, 0, e_mesg );
	if( retval!=NOERROR ){
		fomer(e_mesg);
		get();
		return(retval);
	}
	return(0);
}			
set_condexp(code,expl)
char code, *expl;	/* condition code and explaination */
{
	switch( code ){
		case CD_EXCELLENT:
			strcpy(expl, EXCELLENT);
			break;
		case CD_GOOD:
			strcpy(expl, GOOD);
			break;
		case CD_FAIR:
			strcpy(expl, FAIR);
			break;
		case CD_POOR:
			strcpy(expl, POOR);
			break;
		case CD_OBSOLETE:
			strcpy(expl, OBSOLETE);
			break;
	}
	return(0);
}