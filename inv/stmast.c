/******************************************************************************
		Sourcename    : stmast.c
		System        : Budgetary Financial System.
		Subsystem     : Inventory System 
		Module        : Inventory master maintenance 
		Created on    : 89-06-9
		Created  By   : K HARISH.
		Cobol sources : 

HISTORY:
 Date           Programmer     Description of modification
____/__/__      __________     ___________________________
1990/12/18	M. Cormier	Put D_Roundoff() routine to round
				an item defined as double before writing
				it to the file.
1991/01/21	C.Leadbeater	Modified so that ESC-F would return to the 
				function prompt not exit the program.
1991/02/05	J.Prescott	if SW5 switch ON you are Not allowed to edit 
				stock on hand, or average price. 
				Also now display's stock section description
				by the stock section number. 
1991/06/11	C.Burns		Modified for PO-Inventory Interface.  When   
				adding a record the two new field st_committed
				and st_po_ordqty must be initialized.

1996/01/02	L.Robichaud	Added a routine that will reconsile the stock
				master file st_alloc field to the allocation 
				file.
******************************************************************************/
#include <stdio.h>

#define MAIN
#define MAINFL	STMAST

#include <bfs_defs.h>
#include <bfs_recs.h>
#include <cfomstrc.h>

#define SYSTEM		"INVENTORY"
#define MOD_DATE	"21-JAN-91"
#define SCREEN_NAME	"stmast"
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
#define NO_HLP_WIN	(sr.curfld!=700 && sr.curfld!=900 && sr.curfld!=1800)

#ifdef ENGLISH
#define ADDREC		'A'
#define CHANGE		'C'
#define DELETE		'D'
#define NEXT		'N'
#define PREV		'P'
#define INQUIRE		'I'
#define EXITOPT		'E'

#define EDIT		'E'
#define CANCEL		'C'
#define YES		'Y'
#define NO		'N'
#else
#define ADDREC		'R'
#define CHANGE		'C'
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

#define	FUND_FLD	600

struct	st_struct	{

	char	s_progname[11];	/* 100 program name */
	long	s_rundt;	/* 300 system date */
	char	s_fn[2];	/* 400 function */
	short	s_fld_no;	/* 500 field # for modification */
	short	s_fund;		/* 600 fund supporting the stock */
	char	s_code[11];	/* 700 stock code */
	short	s_section;	/* 900 stock section */
	char	s_sectdesc[31];	/* 950 stock section description */
	char	s_descr[31];	/* 1000 stock description */
	char	s_unit[7];	/* 1100 stock unit of measurement */
	double	s_on_hand;	/* 1200 stock on hand */
	double	s_on_order;	/* 1300 stock on order */
	double	s_alloc;	/* 1400 stock allocated */
	double	s_paidfor;	/* 1500 stock paid for */
	double	s_po_ordqty;	/* 1500 stock committed */
	double	s_min;		/* 1600 minimum limit on stock */
	double	s_max;		/* 1700 maximum limit on stock */
	char	s_accno[19];	/* 1800 default account # */
	long	s_lastdate;	/* 1900 last date of stock receipt */
	short	s_leaddays;	/* 1950 lead days for stock order */
	double	s_value;	/* 2000 total value of stock on hand */
	double	s_rate;		/* 2100 average price of each unit */
	double	s_committed;	/* 2125 committed amount from PO */
	char	s_m_y_hdg[2];	/* 2200 heading mask */
	double	s_y_opb;	/* 2300 opening balance for the year */
	double	s_y_iss;	/* 2400 total issues for the year */
	double	s_y_rec;	/* 2500 total receipts for the year */
	double	s_y_adj;	/* 2600 total adjustments for the year */
	double	s_m_opb;	/* 2700 opening balance for the month */
	double	s_m_iss;	/* 2800 total issues for the month */
	double	s_m_rec;	/* 2900 total receipts for the month */
	double	s_m_adj;	/* 3000 total adjustments for the month */
	char	s_phy_hdg[2];	/* 3100 heading mask */
	double	s_befcount;	/* 3200 physical stock before count */
	double	s_aftcount;	/* 3300 physical stock after count */
	char	s_mesg[77];	/* 3400 message field */
	char	s_resp[2];	/* 3500 response field */
};	

struct st_struct	s_rec;		/* screen record */

St_mast	oldstmast,st_mast;	/* stock master records */
struct stat_rec 	sr;		/* profom status record */
Ctl_rec		ctl_rec;	/* control file record */
St_sect		section;	/* stock section file */
Alloc_rec	aloc_rec;	/* stock allocation file */

int retval;	/* Global variable to store function values */
char e_mesg[80]; /* to store error messages */
double	D_Roundoff();

/* Initialize profom fields, call entry procedures */
main( argc, argv )
int argc;
char *argv[];
{
	strncpy( SYS_NAME, SYSTEM, 50 ); /* Module name */
	strncpy( CHNG_DATE, MOD_DATE, 10 );/* Last date of change */
	
	proc_switch( argc,argv,MAINFL );/* process the switches */

	if( (retval=Initialize())<0 )	/* Initialize profom enviroment */
		exit(-1);
	if( retval!=ESCAPE )
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
	if( FillKeyFields( LOW )<0 ) 		return(-1);
	if( FillField( HIGH )<0 ) 		return(-1);
	if( FillNonKeyFlds( HIGH )<0 ) 		return(-1);
	if( FillMesgRespFlds( HIGH )<0 ) 	return(-1);
	if( InitProfom()<0 ){
		fomcs();
		fomrt();
		return(-1);
	}
	/* Read the section file record */
	if( get_section(&section,BROWSE,1,e_mesg)<1 ){
#ifdef ENGLISH
		fomen("Cannot read stock section file. Press any key");
#else
		fomen("Ne peut pas lire le dossier de section de stocks. Appuyer sur une touche");
#endif
		get();
		return(ESCAPE);
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
	s_rec.s_fund = value * HV_SHORT;
	s_rec.s_code[0] = HL_CHAR(value);

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
	s_rec.s_section 	= value * HV_SHORT;
	s_rec.s_sectdesc[0] 	= HL_CHAR(value);
	s_rec.s_descr[0] 	= HL_CHAR(value);
	s_rec.s_unit[0] 	= HL_CHAR(value);
	s_rec.s_on_hand 	= value * HV_DOUBLE;
	s_rec.s_on_order 	= value * HV_DOUBLE;
	s_rec.s_alloc 		= value * HV_DOUBLE;
	s_rec.s_paidfor 	= value * HV_DOUBLE;
	s_rec.s_po_ordqty 	= value * HV_DOUBLE;
	s_rec.s_min 		= value * HV_DOUBLE;
	s_rec.s_max 		= value * HV_DOUBLE;
	s_rec.s_accno[0] 	= HL_CHAR(value);
	s_rec.s_lastdate 	= value * HV_LONG;
	s_rec.s_value 		= value * HV_DOUBLE;
	s_rec.s_rate 		= value * HV_DOUBLE;
	s_rec.s_committed	= value * HV_DOUBLE;
	s_rec.s_leaddays	= value * HV_SHORT;
	s_rec.s_m_y_hdg[0] 	= (value==HIGH) ? HV_CHAR : ' ' ;
	s_rec.s_y_opb 		= value * HV_DOUBLE;
	s_rec.s_y_iss 		= value * HV_DOUBLE;
	s_rec.s_y_rec 		= value * HV_DOUBLE;
	s_rec.s_y_adj 		= value * HV_DOUBLE;
	s_rec.s_m_opb 		= value * HV_DOUBLE;
	s_rec.s_m_iss 		= value * HV_DOUBLE;
	s_rec.s_m_rec 		= value * HV_DOUBLE;
	s_rec.s_m_adj 		= value * HV_DOUBLE;
	s_rec.s_phy_hdg[0] 	= (value==HIGH) ? HV_CHAR : ' ' ;
	s_rec.s_befcount 	= value * HV_DOUBLE;
	s_rec.s_aftcount 	= value * HV_DOUBLE;
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
	s_rec.s_fund = 0;
	s_rec.s_code[0] = '\0';

	for( ; ; ){
		if( ReadFunction()<0 ) return(-1);
		switch( s_rec.s_fn[0] ){
			case ADDREC:	/* add a record */
				CHKACC(retval,ADD,e_mesg);
				retval = AddRecord();
				roll_back(e_mesg);
				
				if( retval < 0 )
					return(-1);
				break;
			case CHANGE:	/* Change a record */
				CHKACC(retval,UPDATE,e_mesg);
				retval = ChangeRecord();
				roll_back(e_mesg);

				if( retval < 0)
					return(-1);
				break;
			case DELETE:	/* Delete a record */
				CHKACC(retval,P_DEL,e_mesg);
				retval = DeleteRecord();
				roll_back(e_mesg);

				if( retval < 0 )
					return(-1);
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
int err;

	if( ClearScreen()<0 )	return(-1);
	if( SetDupBuffers( 600, 700, 0 )<0 )
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

	/* render the following fields inaccessible to user */
	s_rec.s_on_order = s_rec.s_alloc = s_rec.s_paidfor = 0.0;
	s_rec.s_y_iss = s_rec.s_y_rec = s_rec.s_y_adj = 0.0;
	s_rec.s_m_iss = s_rec.s_m_rec = s_rec.s_m_adj = 0.0;
	s_rec.s_befcount = s_rec.s_aftcount = 0.0;
	s_rec.s_committed = s_rec.s_po_ordqty = 0.0;
/*
	s_rec.s_rate = 0.0;
*/
	/* Read the non key fields now */
	if( (retval=ReadFields(900,3300))<0 )
		return(retval);
	if(retval==ESCAPE){
		if(ClearScreen()<0)	return(-1);
		return(retval);
	}


	/* Allow the user to edit entries, if required, before saving */
	if( (retval=EditFlds(ADD))<0 )	return(retval);
	if( retval==ESCAPE ){
		if(ClearScreen()<0)	return(-1);
		return(retval);
	}

	for(;;){
		/* Write the record, in ADD mode, to stock master file */
		err = WriteSession( ADD );
		
		if(err == NOERROR) return(NOERROR);
		if(err == LOCKED){
			roll_back(e_mesg);
			if( (retval=EditFlds(LOCKED))<0 )	return(retval);
			if( retval==ESCAPE ){
				if(ClearScreen()<0)	return(-1);
				return(retval);
			}
			continue;
		}
		if(err<0) break;
	}
	return(err);
}
/* Read stock master key fields, the fund and stock item code */
RdKeyFlds()
{
	if( FillKeyFields(LOW)<0 ) return(-1);
	fund_default();
	return ( ReadFields(600,700) );
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
		    if( sr.retcode==RET_USER_ESC ){
			if( (sr.escchar[0]=='F' || sr.escchar[0]=='f')&&
				s_rec.s_resp[0]!=EDIT &&
				s_rec.s_fn[0]!=CHANGE )
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
ClearScreen()	/* clear the screen except fn field and screen heading */
{
	/****
		if(FillKeyFields(HIGH)<0 ) return(-1);
	****/
	if(FillField(HIGH)<0 ) return(-1);
	if(FillNonKeyFlds(HIGH)<0 ) return(-1);
	if(FillMesgRespFlds(HIGH)<0 ) return(-1);

	if( WriteFields(500,3500)<0 ) return(-1);
	
	return(0);
}
Validate()	/* Validate the values entered by the user */
{
	Gl_rec temp_rec;
	int index;

	switch( sr.curfld ){
		case 500:	/* field # for editing */
			if( sr.fillcode==FIL_OMITTED )	/* Nothing entered */
				return(ESCAPE);
			break;
		case 600:	/* fund code: check by reading control rec */
			ctl_rec.fund = (short)s_rec.s_fund;
			index = get_ctl( &ctl_rec, BROWSE, 0, e_mesg );
			if( index==ERROR ){
				fomen(e_mesg); get();
				return(-1);
			}
			if( index!=NOERROR ){	/* record doesn't exist */
				fomer(e_mesg);
				s_rec.s_fund = LV_SHORT; 
			}
			else
				fomer( ctl_rec.desc );
			break;
		case 700:	/* stock item code: check reading master rec */
			if( ESC_H ){
				retval = stock_hlp( s_rec.s_fund,s_rec.s_code,
						7,13 );
				if( retval==DBH_ERR )
					return( retval );
				else if( retval>=0 )
					redraw();
				if( retval==0 )
					break;
			}
			st_mast.st_fund = (short)s_rec.s_fund;
			STRCPY( st_mast.st_code, s_rec.s_code );
			retval = get_stmast( &st_mast, BROWSE, 0, e_mesg );
			if( retval==ERROR ){
				fomen(e_mesg);get();
				return(retval);
			}
			if( s_rec.s_fn[0]==ADDREC ){	/* add mode */
				if( retval!=UNDEF ){
#ifdef ENGLISH
					fomer("Record already exists");
#else
					fomer("Fiche existe deja");
#endif
					s_rec.s_code[0] = LV_CHAR; 
				}
			}
			else{
				if( retval!=NOERROR ){ /* no record exists */
					fomer(e_mesg);
					s_rec.s_code[0] = LV_CHAR; 
				} 
				else
					fomer( st_mast.st_desc );
			}
			break; 
		case 900:	/* stock item section */
			if( ESC_H ){
				retval = sect_hlp( &s_rec.s_section,6,13 );
				if( retval==DBH_ERR )
					return( retval );
				else if( retval>=0 )
					redraw();
				if( retval==0 )
					break;
			}
			if( s_rec.s_section>section.no_of_sections ||
				 s_rec.s_section<1 ){
#ifdef ENGLISH
				fomer("Not defined in section file");
#else
				fomer("Pas defini dans le dossier de section");
#endif
				s_rec.s_section = LV_SHORT; 
			}
			else
				STRCPY(s_rec.s_sectdesc,
					section.name[s_rec.s_section-1]);
			break;
		case 1200:	/* stock on hand */
			if( s_rec.s_fn[0]==ADDREC )
				s_rec.s_y_opb = s_rec.s_m_opb = s_rec.s_on_hand;
			break;
		case 1700:	/* max. limit on stock on hand */
			if(s_rec.s_max < s_rec.s_min){
#ifdef ENGLISH
				fomer("Can't be allowed as Max. limit.");
#else
				fomer("Pas permis comme limite maximum");
#endif
				s_rec.s_max = LV_DOUBLE;
			}
			break;
		case 1800:	/* account number of the item */
			if( ESC_H ){
			   retval = gl_hlp(s_rec.s_fund, s_rec.s_accno, 
					&temp_rec.reccod,
					7, 15 );
			   if( retval<0 )	/* error */
					return( retval );
			   if( retval==0 )	/* nothing selected  */
					redraw(); /* remove hlp window*/
			   else if( retval==1 ){/* selected */
					/* check for reccod compatibility */
				if( temp_rec.reccod!=99 ){
				   redraw();
#ifdef ENGLISH
				   fomer("Invalid Record code: Try again");
#else
				   fomer("Code de fiche invalide: Reessayer");
#endif
				   s_rec.s_accno[0]=LV_CHAR;
				   sr.nextfld = sr.curfld;
				   break;
				}
				else 
					redraw();
			   }
			   break;
			}
			if( s_rec.s_accno[0] == '\0') {
				strcpy(s_rec.s_accno, "0");
				break;
			}
			/* check if account # is numeric */
			if( acnt_chk(s_rec.s_accno)==ERROR ){
#ifdef ENGLISH
				fomer("Invalid Account number");
#else
				fomer("Numero de compte invalide");
#endif
				s_rec.s_accno[0]=LV_CHAR;
				break;
			}
			/* Check if Gl master record exists */
			retval = ChkMastKey(s_rec.s_accno,&temp_rec); 
			if( retval==ERROR )	return(-1);
			if( retval!=NOERROR )
				s_rec.s_accno[0]=LV_CHAR;
			else
				fomer(temp_rec.desc);
			break; 
		case 1900:	/* Last date of stock receipt */
			if( s_rec.s_lastdate > s_rec.s_rundt ){
#ifdef ENGLISH
				fomer("Last date can't exceed current date");
#else
				fomer("Derniere date ne peut etre plus tard que la date courante");
#endif
				s_rec.s_lastdate = LV_LONG;
			}
			break; 
		case 2000:	/* Value of stock */
			/*if(s_rec.s_fn[0] == CHANGE) { */
			if (s_rec.s_value != 0.00) 
				s_rec.s_rate = 
				(s_rec.s_value + s_rec.s_committed)/(s_rec.s_on_hand+s_rec.s_paidfor+s_rec.s_po_ordqty);
			/*} */
			if( s_rec.s_on_hand+s_rec.s_paidfor>DELTA_DIFF &&
				s_rec.s_value > DELTA_DIFF)
				(s_rec.s_value + s_rec.s_committed)/(s_rec.s_on_hand+s_rec.s_paidfor+s_rec.s_po_ordqty);
			else
				s_rec.s_rate = LV_DOUBLE;
			break;
		case 2100:	/* average stock rate */
			if( s_rec.s_rate<DELTA_DIFF ){
#ifdef ENGLISH
				fomer("Invalid average price");
#else
				fomer("Prix moyen invalide");
#endif
				s_rec.s_rate = LV_DOUBLE;
			}
			break;
		default:
			break;
	}
	sr.nextfld = sr.curfld;
	return(0);
}
ChkMastKey(accno,temp_rec) /* Check if a record exists in the GL master */
char *accno;
Gl_rec *temp_rec;
{
	int retval;

	temp_rec->funds = (short)s_rec.s_fund;
	STRCPY( temp_rec->accno,accno );
	temp_rec->reccod = 99;
	retval = get_gl( temp_rec,BROWSE,0,e_mesg );
	if(retval!=NOERROR ){
		fomen(e_mesg);
		get();
		return(retval);
	}
	return(0);
}
EditFlds(mode)	/* Ask if user wants to edit fields before saving */
int	mode;
{
	double diff;

	/* Go to field editing directly if in change mode */
	if( s_rec.s_fn[0]==CHANGE && mode != LOCKED)
		if( FldEdit()<0 )
			return(-1);
	for( ; ; ){
#ifdef ENGLISH
		if(DisplayMessage( "Y(es), E(dit), C(ancel)")<0 ) 
#else
		if(DisplayMessage( "M(odifier), A(nnuler), O(ui)")<0 ) 
#endif
			return(-1);
		sr.nextfld = 3500;
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
				sr.nextfld = 3500;	/* response field */
				fomrf( (char *)&s_rec );
				ret( err_chk(&sr) );
				if( s_rec.s_resp[0]==YES ){
					if( HideMessage()<0 ) return(-1);
					return(ESCAPE);
				}
				break;
			case YES:
/**********
				if( s_rec.s_on_hand<DELTA_DIFF &&
				    s_rec.s_value>DELTA_DIFF ){
#ifdef ENGLISH
					fomer("Qty/Value Mismatch");
#else
					fomer("Divergence quantite/valeur");
#endif
					continue;
				}
*******/
				if( s_rec.s_on_hand+s_rec.s_paidfor>DELTA_DIFF )
				   diff = s_rec.s_rate-
					(s_rec.s_value + s_rec.s_committed)/
					(s_rec.s_on_hand+s_rec.s_paidfor+
					 s_rec.s_po_ordqty);
				else
					diff = 0.0;
				if( diff>DELTA_DIFF || diff<-DELTA_DIFF ){
#ifdef ENGLISH
				  fomer("Average price improper: Touch value");
#else
				  fomer("Prix moyen inconvenant: Ajuster la valeur");
#endif
				  continue;
				}
				if( s_rec.s_max-s_rec.s_min<0.0 ){
#ifdef ENGLISH
				   fomer("Min. limit can't exceed Max. limit");
#else
				   fomer("Limite min. ne peut pas exceder la limite max.");
#endif
				   continue;
				}
				if( s_rec.s_fn[0]==ADDREC ){
				  if(s_rec.s_on_hand!=s_rec.s_y_opb ||
					s_rec.s_on_hand!=s_rec.s_m_opb) {
#ifdef ENGLISH
					fomer("Opg. bal not = to stock on hand");
#else
					fomer("Balance d'ouverture pas = aux stocks en maison");
#endif
					continue;
				  }
				}
				if( HideMessage()<0 ) return(-1);
				return(0);
		}
	}
}
DisplayMessage(mesg)	/* Display the given message in the message field */
char *mesg;
{
	STRCPY( s_rec.s_mesg, mesg );
	if( WriteFields(3400,3400)<0 )	return(-1);
	return(0);
}
HideMessage()	/* Hide the message & response fields */
{
	if( FillMesgRespFlds(HIGH)<0 )	return(-1);
	if( WriteFields(3400,3500)<0 ) return(-1);
	return(0);
}
HideFldNo()	/* Hide the 'Field#' field */
{
	if( FillField(HIGH)<0 )	return(-1);
	if( (WriteFields(500,500))<0 )
		return(-1);
	return(0);
}
FldEdit()	/* Read the field number and read corresponding field */
{
	int firstfld,lastfld;

	for( ; ; ){
		firstfld = lastfld = 0 ;
#ifdef	ENGLISH
		STRCPY(s_rec.s_mesg,"Enter RETURN to terminate Edit");
#else
		STRCPY(s_rec.s_mesg,"Appuyer sur RETURN pour terminer l'ajustement");
#endif
		if( (WriteFields(3400,3400))<0 )
			return(-1);
		/* Read number of field to be changed */
		if( FillField(LOW)<0 )	return(-1);
		if( (retval = ReadFields(500,500))<0 ) return(-1);
		if( retval==ESCAPE ){
			if( HideFldNo()<0 ) return(-1);
			break;
		}
		/* validate the field# entry */
		if( s_rec.s_fld_no < 1 || s_rec.s_fld_no > 15 ){
#ifdef ENGLISH
			fomer("Cannot access specified field");
#else
			fomer("Ne peur pas acceder au champ specifie");
#endif
			continue;
		}
		switch(s_rec.s_fld_no){
			case 1: firstfld = 900;	/* stock section */
				lastfld  = 950; /* section description */
				break;
			case 2: firstfld =lastfld=1000;	/* description */
				break;
			case 3: firstfld =lastfld=1100;/* unit  */
				break;
			case 4: /* stock on hand */
				if( s_rec.s_fn[0]==ADDREC ){
					firstfld = 1200;
					lastfld = 2700;
				}
				else {
				 if(SW5) { /* no mods */
#ifdef ENGLISH
				 fomer("Can't change specified field");
#else
				 fomer("Ne peut pas changer le champ specifie");
#endif
				 continue;
				 }
				 else {
					firstfld = 1200;
					lastfld = 2700;
				 }
				}
				break;
			case 5: /* stock on order */
				if( s_rec.s_fn[0]==ADDREC ){
					firstfld = 1300;
					lastfld = 1300;
				}
				else {
				 if(SW5) { /* no mods */
#ifdef ENGLISH
				 fomer("Can't change specified field");
#else
				 fomer("Ne peut pas changer le champ specifie");
#endif
				 continue;
				 }
				 else {
					firstfld = 1300;
					lastfld = 1300;
				 }
				}
				break;
			case 7: /* stock on order */
				if( s_rec.s_fn[0]==ADDREC ){
					firstfld = 1500;
					lastfld = 1500;
				}
				else {
				 if(SW5) { /* no mods */
#ifdef ENGLISH
				 fomer("Can't change specified field");
#else
				 fomer("Ne peut pas changer le champ specifie");
#endif
				 continue;
				 }
				 else {
					firstfld = 1500;
					lastfld = 1500;
				 }
				}
				break;
			case 9: firstfld = 1600;/* Minimum Limit */
				lastfld = 1700; /* Maximum Limit */
				break;
			case 10: firstfld = lastfld=1700;/* Maximum Limit */
				break;
			case 11: 
				if(s_rec.s_accno[0] == 0) 
					s_rec.s_accno[0] = LV_CHAR ;
				firstfld = lastfld=1800;/* Account number */
				break;
			case 12: firstfld = lastfld=1900;/* Date of last rept */
				break;
			case 13: firstfld = lastfld = 1950;	/* lead days */
				break;
			case 14: 
				if( s_rec.s_fn[0]==ADDREC ) {
					firstfld=2000;	/* Value of stock */
					lastfld = 2100; /* Average Price */
				}
				else {
				 if(SW5) {
#ifdef ENGLISH
				 fomer("Can't change specified field");
#else
				 fomer("Ne peut pas changer le champ specifie");
#endif
					continue;
				 }
				 else {
					firstfld=2000;	/* Value of stock */
					lastfld = 2100; /* Average Price */
					
				 }
				}
				break;
			case 15: 
				if( s_rec.s_fn[0]==ADDREC ) 
					firstfld=lastfld=2100;/*Average Price */
				else {
				 if(SW5) { 
#ifdef ENGLISH
					 fomer("Can't change specified field");
#else
					 fomer("Ne peut pas changer le champ specifie");
#endif
					continue;
				 }
				 else 
					firstfld = lastfld = 2100;/*Average Price */
				}
				break;
#ifdef ENGLISH
			default: fomer("Can't change specified field");
#else
			default: fomer("Ne peut pas changer le champ specifie");
#endif
				continue;
		}
/*  Commented out by C Burns July 17, 1991 Already being done in prior code
		if(s_rec.s_fn[0] != ADDREC)
			if(SW5 && (s_rec.s_fld_no == 4 || s_rec.s_fld_no == 14))
				continue;
*/

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

	for( i=firstfld; i<=lastfld; i+=100 ){
		fomca1( i,19,2);	/* enable dup buffers */
	}
	sr.nextfld = firstfld;
	sr.endfld = lastfld;
	fomud( (char *)&s_rec);		/* Update dup buffers */
	switch(firstfld){
		case 900:	/* stock section */
			s_rec.s_section = LV_SHORT;
			break;
		case 1000:	/* stock description */
			s_rec.s_descr[0] = LV_CHAR;
			break;
		case 1100:	/* unit of measurement */
			s_rec.s_unit[0] = LV_CHAR;
			break;
		case 1200:	/* stock on hand */
			s_rec.s_on_hand = LV_DOUBLE;
			break;
		case 1300:	/* stock on order */
			s_rec.s_on_order = LV_DOUBLE;
			break;
		case 1500:	/* pre-paid*/
			s_rec.s_paidfor = LV_DOUBLE;
			break;
		case 1600:	/* minimum and maximum limits */
			s_rec.s_min = LV_DOUBLE;
			s_rec.s_max = LV_DOUBLE;
			break;
		case 1700:	/* maximum limit */
			s_rec.s_max = LV_DOUBLE;
			break;
		case 1800:	/* account number */
			s_rec.s_accno[0] = LV_CHAR;
			break;
		case 1900:	/* date of last receipt */
			s_rec.s_lastdate = LV_LONG;
			break;
		case 1950:	/* date of last receipt */
			s_rec.s_leaddays = LV_SHORT;
			break;
		case 2000:	/* stock value */
			s_rec.s_value = LV_DOUBLE;
			s_rec.s_rate = LV_DOUBLE;
			break;
		case 2100:	/* stock rate */
			s_rec.s_rate = LV_DOUBLE;
			break;
		default:
			break;
	}
	retval = ReadFields( firstfld, lastfld );
	if( retval<0 || retval==ESCAPE )	return(retval);
	for( i=firstfld; i<=lastfld; i+=100 ){
		fomca1( i,19,0);	/* disable dup buffers */
	}
	return(0);
}
WriteSession(mode)	/* Write the stock master record */
int	mode;		/* ADD, UPDATE, P_DEL */
{
    if( mode!=P_DEL ){
	st_mast.st_fund = (short)s_rec.s_fund;
	STRCPY( st_mast.st_code, s_rec.s_code );
	st_mast.st_section = (short)s_rec.s_section;
	STRCPY( st_mast.st_desc, s_rec.s_descr );
	STRCPY( st_mast.st_unit, s_rec.s_unit );
	st_mast.st_on_hand = s_rec.s_on_hand;
	st_mast.st_on_order = s_rec.s_on_order;
	st_mast.st_alloc = s_rec.s_alloc;
	st_mast.st_paidfor = s_rec.s_paidfor;
	st_mast.st_po_ordqty = s_rec.s_po_ordqty;
	st_mast.st_min = s_rec.s_min;
	st_mast.st_max = s_rec.s_max;
	if(s_rec.s_accno[0] != 0) {
		STRCPY( st_mast.st_accno, s_rec.s_accno );
	}
	st_mast.st_lastdate = s_rec.s_lastdate;
	st_mast.st_value = s_rec.s_value;
	/* don't round rate because it is an average */
	st_mast.st_rate = s_rec.s_rate;
	st_mast.st_committed = s_rec.s_committed;
	st_mast.st_leaddays = s_rec.s_leaddays;
	st_mast.st_y_opb = s_rec.s_y_opb;
	st_mast.st_y_iss = s_rec.s_y_iss;
	st_mast.st_y_rec = s_rec.s_y_rec;
	st_mast.st_y_adj = s_rec.s_y_adj;
	st_mast.st_m_opb = s_rec.s_m_opb;
	st_mast.st_m_iss = s_rec.s_m_iss;
	st_mast.st_m_rec = s_rec.s_m_rec;
	st_mast.st_m_adj = s_rec.s_m_adj;
	st_mast.st_bef_cnt = s_rec.s_befcount;
	st_mast.st_aft_cnt = s_rec.s_aftcount;
    }
    if( mode==ADD ){
	st_mast.st_committed = 0.00;
	st_mast.st_po_ordqty = 0.00;
    }
	retval = put_stmast( &st_mast, mode, e_mesg );
	if( retval!=NOERROR ){
		fomen(e_mesg);get();
		roll_back(e_mesg);
		return(retval);
	}
	if((retval = rite_audit((char*)&s_rec, STMAST,mode,
			(char *)&st_mast,(char *)&oldstmast,e_mesg))<0 ){
		fomen(e_mesg);
		get();
		roll_back(e_mesg);
		return(retval);
	}
	if((retval = commit(e_mesg))<0 ){
		fomen(e_mesg);
		get();
		return(retval);
	}
	return(NOERROR);
}
Inquiry( access, direction )	/* Screen inquiry, random/sequential */
int access, direction;		/* RANDOM, SEQUENTIAL accesses */
{
	int retval;

	if( access==RANDOM ){
		retval = GetRecord();	/* Read the key values */
		if( retval==UNDEF || retval==ESCAPE || retval==LOCKED )
			return( ESCAPE );
	}
	else{	/* access is sequential, so get next record from file */
		retval = GetNextRec( direction );	/* Get next rec */
		if(retval==EFL)
			return(0);
	}
	if( retval<0 ){	/* errors in reading */
		fomen(e_mesg);get();
		return(-1);
	}
	retval = DisplayRecord();	/* Display the header & item records */
	if( retval<0 ) return(retval);

	return(0);
}
DisplayRecord()	/* Get and display the header and item records */
{
	if(CheckAlloctoStock()) return(-1);

	s_rec.s_fund = st_mast.st_fund ; 
	STRCPY( s_rec.s_code, st_mast.st_code );
	s_rec.s_section = st_mast.st_section ; 
	STRCPY(s_rec.s_sectdesc, section.name[s_rec.s_section-1]);
	STRCPY( s_rec.s_descr,st_mast.st_desc ); 
	STRCPY( s_rec.s_unit,st_mast.st_unit );
	s_rec.s_on_hand = st_mast.st_on_hand ; 
	s_rec.s_on_order = st_mast.st_on_order ; 
	s_rec.s_alloc = st_mast.st_alloc ; 
	s_rec.s_paidfor = st_mast.st_paidfor ; 
	s_rec.s_po_ordqty = st_mast.st_po_ordqty ; 
	s_rec.s_min = st_mast.st_min ; 
	s_rec.s_max = st_mast.st_max ; 
	STRCPY( s_rec.s_accno,st_mast.st_accno );
	s_rec.s_lastdate = st_mast.st_lastdate ; 
	s_rec.s_value = st_mast.st_value ; 
	s_rec.s_rate = st_mast.st_rate ; 
	s_rec.s_committed = st_mast.st_committed ; 
	s_rec.s_leaddays = st_mast.st_leaddays;
	s_rec.s_m_y_hdg[0] = ' ';
	s_rec.s_y_opb = st_mast.st_y_opb ; 
	s_rec.s_y_iss = st_mast.st_y_iss ; 
	s_rec.s_y_rec = st_mast.st_y_rec ; 
	s_rec.s_y_adj = st_mast.st_y_adj ; 
	s_rec.s_m_opb = st_mast.st_m_opb ; 
	s_rec.s_m_iss = st_mast.st_m_iss ; 
	s_rec.s_m_rec = st_mast.st_m_rec ; 
	s_rec.s_m_adj = st_mast.st_m_adj ; 
	s_rec.s_phy_hdg[0] = ' ';
	s_rec.s_befcount = st_mast.st_bef_cnt ; 
	s_rec.s_aftcount = st_mast.st_aft_cnt ; 

	if( WriteFields(900,3300) < 0 )
		return(-1);
	return(0);
}
/***********************************
This routine is to verify that the allocation ammounts in the stock master
file and the total of the allocation against that stock in the allocation 
file are the same.  If they are not then adjust the stockmaster file to match
the total allocations.
****************** Louis R. ***/
CheckAlloctoStock()
{

	int	retval;
	double	alloc_amount;

	alloc_amount = 0;
	aloc_rec.st_fund = st_mast.st_fund; 
	STRCPY(aloc_rec.st_code, st_mast.st_code);
	aloc_rec.st_location = 0;
	aloc_rec.st_expacc[0] = NULL;

	flg_reset(ALLOCATION);

	for(;;){
		retval = get_n_alloc(&aloc_rec, BROWSE, 0, FORWARD, e_mesg) ;
		if(retval == EFL) break;
		if(retval < 0){
			fomer(e_mesg);
			roll_back(e_mesg);
			return(-1);
		}

		if(aloc_rec.st_fund != st_mast.st_fund) break;
		if(strcmp(aloc_rec.st_code, st_mast.st_code)) break;

		alloc_amount += aloc_rec.st_alloc;
	}

	seq_over(ALLOCATION);

	/* Check if the two amount are the same */
	if(alloc_amount == st_mast.st_alloc) return(NOERROR);

	/* The amounts are not the same, must be corrected in the stock file*/

	sprintf(e_mesg,"Correcting allocation in stock file from %lf to %lf - Press Any Key",st_mast.st_alloc, alloc_amount);
	fomer(e_mesg);
	get();

	retval = get_stmast( &st_mast, UPDATE, 0, e_mesg );
	if( retval!=NOERROR ){
		fomer(e_mesg);
		get();
		if(retval != LOCKED) return(retval);
	}

	st_mast.st_alloc = alloc_amount; 

	retval = put_stmast( &st_mast, UPDATE, e_mesg );
	if( retval!=NOERROR ){
		fomen(e_mesg);get();
		roll_back(e_mesg);
		return(retval);
	}
	if((retval = commit(e_mesg))<0 ){
		fomen(e_mesg);
		get();
		return(retval);
	}
	
	return(NOERROR);
}

GetNextRec(direction)	/* Read the next record in the specified direction */
int	direction;
{
	int retval;

	if( flg_start(STMAST)!=direction ){ 	/* file access mode changed */
		st_mast.st_fund = (short)s_rec.s_fund;
		STRCPY( st_mast.st_code, s_rec.s_code );
		inc_str( st_mast.st_code, sizeof( st_mast.st_code )-1, 
					direction );
		flg_reset(STMAST);
	}
	/* Read the next record from stmast file */
	retval = get_n_stmast( &st_mast, BROWSE, 0, direction, e_mesg );
#ifndef ORACLE
	seq_over( STMAST );
#endif
	if( retval==EFL ){
#ifdef ENGLISH
		fomen("No More Records....");
#else
		fomen("Plus de fiches....");
#endif
		get();
		flg_reset(STMAST);
		return(EFL);
	}
	if( retval!=NOERROR ){
		fomen(e_mesg);	get();
		return(retval);
	}
	/* Write the key fields on to the screen */
	s_rec.s_fund = st_mast.st_fund;
	STRCPY( s_rec.s_code, st_mast.st_code );
	if( WriteFields(600,700)<0 )
		return(-1);
	return(0);
}
SetDupBuffers( firstfld, lastfld, value )
int	firstfld, lastfld;	/* field numbers range */
int	value;			/* ENABLE or DISABLE */
{
	int i;

	for( i=firstfld; i<=lastfld; i+=100 )
		fomca1( i, 19, value);
	sr.nextfld = firstfld;
	sr.endfld = lastfld;
	fomud( (char *)&s_rec );
	ret( err_chk(&sr) );

	return( 0 );
}
GetRecord()	/* Read the header key values, get terminal info ,read rec */
{
	int retval;
	int mode;

	if( s_rec.s_fn[0]==CHANGE || s_rec.s_fn[0]==DELETE )
		mode = UPDATE;
	else
		mode = BROWSE;
	if( SetDupBuffers( 600, 700, 2 )<0 )
		return(-1);
	s_rec.s_fund = LV_SHORT;
	s_rec.s_code[0] = LV_CHAR;
	fund_default();
	if( (retval=ReadFields(600,700))<0 || retval==ESCAPE ) 
		return( retval );
	st_mast.st_fund = (short)s_rec.s_fund;
	STRCPY( st_mast.st_code, s_rec.s_code );
	retval = get_stmast( &st_mast, mode, 0, e_mesg );
	if( retval!=NOERROR ){
		fomer(e_mesg);
		get();
		return(retval);
	}
	return(0);
}

ChangeRecord()	/* Change an existing record */
{
	int retval;

	/* Get the record & display it */
	retval = Inquiry( RANDOM, 0 );
	if( retval<0 )	return(retval);
	if( retval==ESCAPE )  return(0);

	/* Copy the record to another buffer, for writing audit */
	scpy( (char *)&oldstmast, (char *)&st_mast, sizeof( St_mast ) );


	/* Allow changes on the record fields */
	retval = EditFlds(UPDATE);
	if( retval<0 )	return(retval);
	if( retval==ESCAPE ){	/* Cancellation of changes */
		roll_back(e_mesg);	/* release locked record */
		return(NOERROR);
	}

	for(;;){
		/* Write the updated record */
		retval = WriteSession( UPDATE );
		if(retval == LOCKED){
			roll_back(e_mesg);
			retval = get_stmast( &st_mast, UPDATE, 0, e_mesg );
			if( retval!=NOERROR ){
				fomer(e_mesg);
				get();
				if(retval != LOCKED) return(retval);
			}
			retval = EditFlds(LOCKED);
			if( retval<0 )	return(retval);
			if( retval==ESCAPE ){	/* Cancellation of changes */
				roll_back(e_mesg);/* release locked record */
				return(NOERROR);
			}
		}
		else	break;

	}
	return(retval);
}

DeleteRecord()	/* delete an existing record */
{
	int retval;

	/* Get the record & display it */
	retval = Inquiry( RANDOM, 0 );
	if( retval<0 )	return(retval);
	if( retval==ESCAPE )
		return(0);
	if( s_rec.s_alloc>0.0 || s_rec.s_on_order>0.0 ||
	    s_rec.s_on_hand>0.0 || s_rec.s_paidfor>0.0 ){
#ifdef ENGLISH
	    fomen("Allocations/On-order/On-hand/Qty paid for not zero.Can't delete record");
#else
	    fomen("Allocations/sur commande/en maison/quantite payee pas zero.n'elimine pas fiche");
#endif
		get();
		return(0);
	}
	if( s_rec.s_y_iss>0.0 || s_rec.s_y_rec>0.0 || s_rec.s_y_adj>0.0 ){
#ifdef ENGLISH
		fomen("Stock issues/recpts/adjustments not zero.Can't Delete");
#else
		fomen("Emissions/recus/ajustements de stocks ne sont pas zero.Ne peut pas eliminer");
#endif
		get();
		return(0);
	}
	for(;;){

		for( ; ; ){
#ifdef ENGLISH
			if( DisplayMessage("Confirm (Y/N)?")<0 ) return(-1);
#else
			if( DisplayMessage("Confirmer (O/N)?")<0 ) return(-1);
#endif
			s_rec.s_resp[0] = LV_CHAR;
			if( ReadFields(3500,3500)<0 ) return(-1);
			if( s_rec.s_resp[0]!=YES && s_rec.s_resp[0]!=NO )
				continue;
			else
				break;
		}
		if( s_rec.s_resp[0]==NO ) return(HideMessage());

		/* Write the updated record */
		retval = WriteSession( P_DEL );
		if(retval == LOCKED){
			roll_back(e_mesg);
			retval = get_stmast( &st_mast, UPDATE, 0, e_mesg );
			if( retval!=NOERROR ){
				fomer(e_mesg);
				get();
				if(retval != LOCKED) return(retval);
			}
		}else
			break;
	}
	if( HideMessage()<0 )
		return(-1);
	return(0);
}
fund_default()
{
	fomca1(FUND_FLD,19,2);
	s_rec.s_fund = 1;
	WriteFields(FUND_FLD,FUND_FLD);
	s_rec.s_fund = LV_SHORT;
}
/*-------------------------------------------------------------------------*/
