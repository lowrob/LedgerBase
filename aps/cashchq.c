/*-----------------------------------------------------------------------
Source Name: cashchq.c 
System     : Budgetary Financial System
Subsystem  : APS 
Module     : Cheque History
Created  On: Dec. 12, 1990
Created  By: Mario Cormier

DESCRIPTION:
	This program allows the user to cash several cheques in one
	screen.  For each cheque cashed, the user is to enter the cheque
	number, the amount and the Line Status.

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
F.Tao	       91/01/07	      Use global variable instead of 'D' in write
			      session.
------------------------------------------------------------------------*/

#include <stdio.h>
#include <reports.h>
#include <cfomstrc.h>


#define SCR_NAME	"cashchq"
#define ITEMSPERPAGE 	15
#define FORWARD		0
#define BACKWARD	1
#define HL_CHAR(VAL)	(VAL==HIGH) ? HV_CHAR : LV_CHAR

#define	HIGH		1
#define	LOW		-1
#define	CASHCHQFMT	3
#define ALLOC_ERROR	13
#define STEP		300
#define NO_HLP_WIN	(sr.curfld != PGM_FLD)
#define PROJECT		"chqhrpt"

#ifdef ENGLISH                 /* if ENGLISH */
#define	ADDREC	'A'
#define	NEXT	'N'
#define	PREV	'P'
#define INQUIRE	'I'
#define EXITOPT	'E'
#define EDIT	'E'
#define CANCEL	'C'
#define	YES	'Y'
#define	NO	'N'
#define CHANGE		'C'
#define DELETE		'D'
#define REACTIVATE      'R'
#define OUTSTANDING	'O'
#define CASHED		'C'
#define	CASH_FN		'C' 
#else                           /* if FRENCH */
#define	ADDREC	'R'
#define	NEXT	'S'
#define	PREV	'P'
#define INQUIRE	'I'
#define EXITOPT	'F'
#define EDIT	'M'
#define CANCEL	'A'
#define	YES	'O'
#define	NO	'N'
#define CHANGE		'C'
#define DELETE		'E'
#define REACTIVATE      'V'
#define OUTSTANDING	'N'
#define CASHED		'E'
#define	CASH_FN		'E' 
#endif

#define EXIT		12
#define SPACE           ' '



/* PROFOM Field Numbers */
#define	LAST_SNO	1 
#define	CHNG_LIMIT	15		/* change mode can't change beyond this 						field */
/* Screen Control Variables */
#define	END_FLD  	5700		/* screen end field 	*/
#define ST_ITEMS	1100		/* Item starting field 	*/
#define END_ITEMS	5500		/* Item ending field 	*/ 
#define ST_STATUS	1300		/* First status field 	*/

#define PGM_FLD		100		/* Program Name field	*/
#define	FN_FLD		400		/* Fn: 			*/
#define FUND_KEY	500		/* Fund Key		*/
#define ACCT_KEY	600		/* Bank Account Key	*/
#define	CHG_FLD		700		/* Field: 		*/
#define PGNO_FLD	1000		/* Page number field 	*/

#define CHQ_NO_1	1100		/* Cheque Number, Amount and  	*/
#define CHQ_AMT_1	1200		/*  Line Status Fields		*/
#define LINE_STAT_1	1300
#define CHQ_NO_2	1400
#define CHQ_AMT_2	1500
#define LINE_STAT_2	1600
#define CHQ_NO_3	1700
#define CHQ_AMT_3	1800
#define LINE_STAT_3	1900
#define CHQ_NO_4	2000
#define CHQ_AMT_4	2100
#define LINE_STAT_4	2200
#define CHQ_NO_5	2300
#define CHQ_AMT_5	2400
#define LINE_STAT_5	2500
#define CHQ_NO_6	2600
#define CHQ_AMT_6	2700
#define LINE_STAT_6	2800
#define CHQ_NO_7	2900
#define CHQ_AMT_7	3000
#define LINE_STAT_7	3100
#define CHQ_NO_8	3200
#define CHQ_AMT_8	3300
#define LINE_STAT_8	3400
#define CHQ_NO_9	3500
#define CHQ_AMT_9	3600
#define LINE_STAT_9	3700
#define CHQ_NO_10	3800
#define CHQ_AMT_10	3900
#define LINE_STAT_10	4000
#define CHQ_NO_11	4100
#define CHQ_AMT_11	4200
#define LINE_STAT_11	4300
#define CHQ_NO_12	4400
#define CHQ_AMT_12	4500
#define LINE_STAT_12	4600
#define CHQ_NO_13	4700
#define CHQ_AMT_13	4800
#define LINE_STAT_13	4900
#define CHQ_NO_14	5000
#define CHQ_AMT_14	5100
#define LINE_STAT_14	5200
#define CHQ_NO_15	5300
#define CHQ_AMT_15	5400
#define LINE_STAT_15	5500

#define MESG_FLD	5700		/* Mesg option field 	*/

#define	MAX_KEY_LENGTH	6  

struct  stat_rec sr;		/* PROFOM status rec 	*/


/* cashchq.sth - header for C structure generated by PROFOM EDITOR */
typedef struct	{
	long	s_chq_no;
	double	s_amount;
	char	s_line_status[4];
	} Cheque_line;

typedef struct	{
	char	s_pgm[11];		/* 100 program name STRING (10) */
	long	s_sysdate;		/* 300 system date DATE 9999F99F99 */
	char	s_ftn[2];		/* 400 function STRING X */
	short	s_fund;			/* 500 Fund NUMERIC 999		*/
	char	s_account[19];		/* 600 Bank Account STRING (18)	*/
	short	s_field;        	/* 700 field number NUMERIC 99 */
	short	s_page;			/* 1000 page number NUMERIC 99 */
	Cheque_line s_entries[ITEMSPERPAGE];  /* 1100-5500 array */
	char	s_mesg[78];		/* 5600 message line STRING (77) */ 
	char	s_opt[2];		/* 5700 message option STRING X */
	} s_struct;

static	s_struct	s_sth ; 

typedef struct{		/* structure to record current page & line of entry */
	short	page;	
	short	line;
 }	Counter;
	
/* link list node for holding one page of entries */

 typedef	struct 	pgofitems{
	struct 	pgofitems *prevptr;	/* pointer to previous entry */
	int	update;
	int	lines_entered;		/* no of lines entered */
	Cheque_line fields[ITEMSPERPAGE];/* array of lines per page */
	struct	pgofitems *nextptr;	/* pointer to next entry */
}	Page;

static int start_key,end_key;


static char	e_mesg[80];

static char	*CurrentScr;
extern char	chardate[11];
extern char	projname[50];
extern char	*arayptr[5];

extern Pa_rec	pa_rec;
extern Ctl_rec	ctl_rec;
extern Chq_hist	cheque,pre_rec;

static int totalitemsadded;		/* total items added in add session */
static int itemsactive;
static Page	*headptr,*tempptr,   
		*tailptr,*itemptr; 	/* to maintain list */ 
static Counter	current;		/* For current line & page of item
						 entry */

double D_Roundoff();		/* Function  for rounding up */

/*-------------------------------------------------------------------*/
CashMaint()
{
int	retval;


	if (InitScrPtr()<0)
		exit(-1);		/* Initialization routine */
	

	retval = Process();
	ClearScreen();
	return(retval);

} /* CashMaint   */

/*-------------------------------------------------------------------*/
static int
InitScrPtr()	/* initialize profom and screen */
{
	if( InitScreen()<0 ){		/* initialize screen */
		fomcs();
		fomrt();
		return(-1);
	}

	tempptr = itemptr = tailptr = headptr = 0;

	return(0);
}	
/*-------------------------------------------------------------------*/
/* Initialize screens before going to process options */
static int
InitScreen()
{
	CurrentScr = (char*)&s_sth;
	STRCPY(sr.scrnam,NFM_PATH);
	strcat(sr.scrnam,SCR_NAME) ;

	STRCPY(s_sth.s_pgm,PROG_NAME);
	s_sth.s_sysdate = get_date();	/* get Today's Date in YYMMDD format */
	
	s_sth.s_field = HV_SHORT ;
	s_sth.s_mesg[0] = HV_CHAR ;
	s_sth.s_opt[0] = HV_CHAR ;

	/* Hide keys */
	s_sth.s_fund = LV_SHORT;
	s_sth.s_account[0] = LV_CHAR;
	
	if( ClearItemLines() <0 ) return(-1);
	if(WriteFields(FN_FLD,MESG_FLD)<0) return(ERROR);

	return(NOERROR);

}	/* InitScreens() */
/*-------------------------------------------------------------------*/
/*   Fill the item array with high values   */
static int
ClearItemLines()
{
	int j;
	s_sth.s_page = HV_SHORT;

	for( j=0; j < ITEMSPERPAGE ; j++ ){
		s_sth.s_entries[j].s_chq_no = HV_LONG;
		s_sth.s_entries[j].s_amount = HV_DOUBLE;
		s_sth.s_entries[j].s_line_status[0] = HV_CHAR;
	}

	if(WriteFields(PGNO_FLD, END_ITEMS)<0) return(ERROR);
	return(NOERROR);
}

/*-------------------------------------------------------------------*/
/* Accept user's option and call the corresponding routine in a loop */
static int
Process()
{
	int retval;


	for( ; ; ){
		if( ReadFunction()<0 ) return(-1);

		switch( s_sth.s_ftn[0] ){

			case	CASH_FN:
				for( ; ; ) {
				   if((retval=ProcessCashing()) != NOERROR) 
					   return(retval);
				   retval=WriteSession();
				   if(retval==NOERROR) break;
				   if(retval==LOCKED) {
					roll_back(e_mesg);
					continue;
				   }
				   if(retval<0) return(retval);
				}
				FreeList();
				break;
			case 	EXITOPT:	/* exit */
				return(0);
		} /*  switch	*/
	}  /*  for 		*/
}  /*  Process()    		*/
/*-------------------------------------------------------------------*/
static int
ReadFunction()	/* Display options at the bottom and read entry */
{
	
#ifdef ENGLISH
	fomer("C(ash Cheques), E(xit)");
#else
	fomer("E(ncaisser cheques), F(in)");
#endif
	sr.nextfld = FN_FLD;	/* Fn field number */
	fomrf( (char*)&s_sth );
	ret( err_chk(&sr) );
	return(0);
}      /*  ReadFunction()    */
/*-------------------------------------------------------------------*/
/*                                                                   */
static int
ReadFields(begin,end)
int 	begin, end;
{

	sr.nextfld = begin;
	sr.endfld = end;

	for(; ;) {

		fomrd((char*)&s_sth);
		ret(err_chk(&sr));

		if(sr.retcode == RET_USER_ESC){
			if(sr.escchar[0] == 'F' || sr.escchar[0] == 'f') { 
				return(EXIT);
			}
			if(sr.escchar[0] == 'h' || sr.escchar[0] == 'H')
				if( NO_HLP_WIN ) continue;
			continue;
		}

		if(sr.retcode == RET_VAL_CHK){
			if(ItemsValidation(sr.curfld)<0)
				sr.nextfld = sr.curfld;
			continue;
		}
		break;
	}			/* end of for loop */
	return(NOERROR) ;
} 	/* end of ReadFields */
/*-------------------------------------------------------------------*/
/*                                                                   */
static int
WriteFields( start,end )/* write fields within range from start to end */
int start, end;
{
	sr.nextfld = start;
	sr.endfld = end;
	fomwr( (char *)&s_sth );
	ret( err_chk(&sr) );
	return(NOERROR);
}

/*-------------------------------------------------------------------*/
static int
FillKeyFields( value )
short value;
{
	s_sth.s_fund = value * HV_SHORT;
	s_sth.s_account[0] = HL_CHAR( value );

	return(NOERROR);
}   /*   FillKeyFields      */
/*-------------------------------------------------------------------*/
static int
FillFieldPage( value )
short	value;
{
	s_sth.s_field = s_sth.s_page = value * HV_SHORT;
	return(NOERROR);
}   /* FillFieldPage     */
/*-------------------------------------------------------------------*/
static int
ClearScreen()	/* clear the screen except fn field and screen heading */
{
	
	if(FillKeyFields(LOW)<0 ) return(-1);
	if(FillFieldPage(HIGH)<0 ) return(-1);
	if(ClearItemLines()<0 )	return(-1);
	if(HideMessage() <0) return(-1);	

	if( WriteFields(FUND_KEY,END_FLD )<0 ) return(-1);
	return(0);
}
/*-------------------------------------------------------------------*/
static int
ItemsValidation( fld_no )	/* Validate the values entered by the user */
int fld_no;
{
	int	retval = NOERROR;
	int	save_nextfld, save_endfld;
	double	diff;

	switch(fld_no){

		case FUND_KEY:	/* fund code: check by reading control rec */
			ctl_rec.fund = (short)s_sth.s_fund;
			retval = get_ctl( &ctl_rec, BROWSE, 0, e_mesg );
			if( retval==ERROR ){
				fomen(e_mesg); get();
				return(-1);
			}
			if( retval!=NOERROR ){	/* record doesn't exist */
				fomer(e_mesg);
				s_sth.s_fund = LV_SHORT; 
			}
			else
				fomer( ctl_rec.desc );

			save_nextfld = sr.nextfld;
			save_endfld = sr.endfld;
			STRCPY(s_sth.s_account,ctl_rec.bank1_acnt);
			fomca1(ACCT_KEY,19,2);
			fomud((char *)&s_sth);
			s_sth.s_account[0] = LV_CHAR;
			sr.nextfld = save_nextfld;
			sr.endfld = save_endfld;

			break;
		case ACCT_KEY:
			retval = acnt_chk(s_sth.s_account);
			if(retval ==ERROR)  {
#ifdef	ENGLISH
				fomer("Invalid Account number");
#else
				fomer("Numero de compte invalide");
#endif
				s_sth.s_account[0] = LV_CHAR;
				break;
			}

			if( strcmp(s_sth.s_account,ctl_rec.bank1_acnt) &&
				strcmp(s_sth.s_account,ctl_rec.bank2_acnt) ){
#ifdef ENGLISH
				sprintf(e_mesg,"Bank1: %s, Bank2: %s",
						ctl_rec.bank1_acnt,
						ctl_rec.bank2_acnt );
#else
				sprintf(e_mesg,"Banque1: %s, Banque2: %s",
						ctl_rec.bank1_acnt,
						ctl_rec.bank2_acnt );
#endif
				fomer(e_mesg);
				s_sth.s_account[0]=LV_CHAR;
				return(ERROR);
			}
			WriteFields(ACCT_KEY,ACCT_KEY);
			break;
		case CHQ_NO_1:
		case CHQ_NO_2:
		case CHQ_NO_3:
		case CHQ_NO_4:
		case CHQ_NO_5:
		case CHQ_NO_6:
		case CHQ_NO_7:
		case CHQ_NO_8:
		case CHQ_NO_9:
		case CHQ_NO_10:
		case CHQ_NO_11:
		case CHQ_NO_12:
		case CHQ_NO_13:
		case CHQ_NO_14:
		case CHQ_NO_15:
	
			cheque.ch_funds = s_sth.s_fund;
			STRCPY(cheque.ch_accno, s_sth.s_account);
			cheque.ch_chq_no =
			 	s_sth.s_entries[current.line-1].s_chq_no;
			flg_reset(CHQHIST);
			retval= get_chqhist( &cheque, BROWSE, 0, e_mesg);
	
			if (retval!= NOERROR){
#ifdef	ENGLISH
		fomen("Cheque number not found for the above Fund and Bank Account, Please Re-enter");
#else
		fomen("Numero de cheque pas trouve pour fond et compte de banque ci-dessus, reessayer");
#endif
				get();
				s_sth.s_entries[current.line-1].s_chq_no
								     = LV_LONG;
				return(ERROR);
			}
			if(cheque.ch_status[0] != OUTSTANDING) {
#ifdef	ENGLISH
			fomen("Please enter an Outstanding cheque number");
#else
			fomen("S.-V.-P. entrer un numero de cheque non-regle");
#endif
				get();
				s_sth.s_entries[current.line - 1].s_chq_no
					= LV_LONG;
				return(ERROR);
			}
			if(ItemCheck(s_sth.s_entries[current.line-1].s_chq_no,
							current.line)<0){
				s_sth.s_entries[current.line-1].s_chq_no
								= LV_LONG;
 				retval = ERROR;
	  		}
			break;
		case CHQ_AMT_1:
		case CHQ_AMT_2:
		case CHQ_AMT_3:
		case CHQ_AMT_4:
		case CHQ_AMT_5:
		case CHQ_AMT_6:
		case CHQ_AMT_7:
		case CHQ_AMT_8:
		case CHQ_AMT_9:
		case CHQ_AMT_10:
		case CHQ_AMT_11:
		case CHQ_AMT_12:
		case CHQ_AMT_13:
		case CHQ_AMT_14:
		case CHQ_AMT_15:

			cheque.ch_funds = s_sth.s_fund;
			STRCPY(cheque.ch_accno, s_sth.s_account);
			cheque.ch_chq_no =
			 	s_sth.s_entries[current.line-1].s_chq_no;
			flg_reset(CHQHIST);
			retval= get_chqhist( &cheque, BROWSE, 0, e_mesg);
	
			if(retval!= NOERROR)   return(retval);

  			if(s_sth.s_entries[current.line - 1].s_amount
				!= cheque.ch_net_amt)  {
#ifdef	ENGLISH
		fomen("Cheque Amount is not the same");
#else
		fomen("Montant du cheque n'est pas le meme");
#endif
				get();
				s_sth.s_entries[current.line - 1].s_amount
					= LV_DOUBLE;
				return(ERROR);
			}
			break;
		case LINE_STAT_1:	
		case LINE_STAT_2:	
		case LINE_STAT_3:	
		case LINE_STAT_4:	
		case LINE_STAT_5:	
		case LINE_STAT_6:	
		case LINE_STAT_7:	
		case LINE_STAT_8:	
		case LINE_STAT_9:	
		case LINE_STAT_10:	
		case LINE_STAT_11:	
		case LINE_STAT_12:	
		case LINE_STAT_13:	
		case LINE_STAT_14:	
		case LINE_STAT_15:	

#ifdef	ENGLISH
			if((strcmp(s_sth.s_entries[current.line-1].s_line_status,"ACT")!=0)
		&&(strcmp(s_sth.s_entries[current.line-1].s_line_status,"DEL")!=0))  {
		fomen("Line Status must be ACT(ive) or DEL(eted)");
#else
			if((strcmp(s_sth.s_entries[current.line-1].s_line_status,"ACT")!=0)
		&&(strcmp(s_sth.s_entries[current.line-1].s_line_status,"ELI")!=0))  {
		fomen("Etat de la ligne doit etre ACT(if) ou ELI(mine)");
#endif
				get();
 				s_sth.s_entries[current.line-1].s_line_status[0]
					=  LV_CHAR;
				return(ERROR);
			}
			break;
		default:
			break;
	}
	return(retval);
}     /*  ItemsValidation()   */
/*-------------------------------------------------------------------*/
static int
DisplayMessage(mesg)	/* Display the given message in the message field */
char *mesg;
{
	STRCPY( s_sth.s_mesg, mesg );
	if( WriteFields(END_FLD-100,END_FLD-100)<0 )	return(-1);
	return(0);
}
/*-------------------------------------------------------------------*/
static int
HideMessage()	/* Hide the message field */
{
	s_sth.s_opt[0] = HV_CHAR;
	s_sth.s_mesg[0] = HV_CHAR;

	if( WriteFields(END_FLD-100,END_FLD)<0 ) return(-1);
	return(0);
}
/*-------------------------------------------------------------------*/
static int
HideFldNo()	/* Hide the 'Field#' field */
{
	s_sth.s_field = HV_SHORT;

	if( (WriteFields(CHG_FLD,CHG_FLD))<0 )
		return(-1);
	return(0);
}
/*-------------------------------------------------------------------*/
static
Page *PageAllocated()	/* Allocate memory for 1 page of items on screen */
{
	return( (Page *)(malloc((unsigned)sizeof( Page ))) );
}
/*-------------------------------------------------------------------*/
static int
AddItemsInLoop()	/* Allow item lines to be entered */
{
	int	retval;

	for( ; ; ){
		retval=ReadOneItem();   /* read one transaction. (Course code)*/
		if( retval<0 ){
			FreeList();
			return(-1);
		}
		if (tailptr)
			tailptr->lines_entered = current.line;
		if( retval==EXIT || retval==ALLOC_ERROR )
			break;
	}
	return(0);
}
/*-------------------------------------------------------------------*/
static int
ReadOneItem()	/* read one transaction (one line on screen) */
{
	short retval, item_err;

	if( current.line == ITEMSPERPAGE ){ 
		retval = MakeFreshPage(); /* add new page to list */
		if( retval<0 || retval==ALLOC_ERROR )
			return( retval );
		current.page++;
		current.line = 1;
		if( PutPageNumber(current.page)<0 )
			return(-1);
	}
	else
		current.line++;
	
	s_sth.s_page = LV_SHORT;

	s_sth.s_entries[current.line-1].s_chq_no = LV_LONG;
	s_sth.s_entries[current.line-1].s_amount = LV_DOUBLE;
	s_sth.s_entries[current.line-1].s_line_status[0] = LV_CHAR;
	HideMessage();

#ifdef ENGLISH
	fomen("Press ESC-F to Display Option Line ");
	STRCPY(s_sth.s_entries[current.line-1].s_line_status, "ACT");
#else
	fomen("Appuyer sur ESC-F pour retourner a la ligne d'option");
	STRCPY(s_sth.s_entries[current.line-1].s_line_status, "ACT");
#endif
	
	retval=ReadFields( ST_ITEMS+(current.line-1)*STEP,
		ST_STATUS+(current.line-1)*STEP );

	tempptr->fields[current.line-1].s_chq_no = HV_LONG;
	tempptr->fields[current.line-1].s_amount = HV_DOUBLE;
	tempptr->fields[current.line-1].s_line_status[0] = HV_CHAR;
	
	if(retval==EXIT || retval<0 ){
		s_sth.s_page = HV_SHORT;
		s_sth.s_entries[current.line-1].s_chq_no = HV_LONG;
		s_sth.s_entries[current.line-1].s_amount = HV_DOUBLE;
		s_sth.s_entries[current.line-1].s_line_status[0] = HV_CHAR;
	
		if( WriteFields( ST_ITEMS+(current.line-1)*STEP,
			ST_STATUS+(current.line-1)*STEP)<0 )
			return(-1);

		tempptr->fields[current.line-1].s_chq_no = HV_LONG;
		tempptr->fields[current.line-1].s_amount = HV_DOUBLE;
		tempptr->fields[current.line-1].s_line_status[0] = HV_CHAR;
	
		current.line--;
		if(current.line<1){
			if( headptr==tailptr )
				headptr=tailptr=NULL;
			else{
				tailptr = tailptr->prevptr;
				tailptr->nextptr = NULL;
			}
			free( (char *)tempptr );
/*			FreeList( (char *)tempptr );	*/
			tempptr = tailptr;
			if(tempptr)
				current.page--;
			if(PutPageNumber(current.page)<0 )
				return(-1);
			current.line =  ITEMSPERPAGE;
			if( tempptr ) {
				if( ShowPage()<0 )
					return(-1);
			}
		}
		return(retval);
	}
	else{
		tempptr->fields[current.line-1].s_chq_no =
		       s_sth.s_entries[current.line-1].s_chq_no;
		tempptr->fields[current.line-1].s_amount =
		       s_sth.s_entries[current.line-1].s_amount;
		strncpy(tempptr->fields[current.line-1].s_line_status,
			 s_sth.s_entries[current.line-1].s_line_status, 3);
		tempptr->lines_entered = current.line;

		totalitemsadded++;
		itemsactive++;	

	}
	return(0);
}     /*  ReadOneItem()  */

/*-----------------------------------------------------------------------*/
static int
ItemCheck( cheque_no,line_no)  /*Check if the item has already been entered */
long	cheque_no;
short	line_no;

{
	int i, retval;
	
	itemptr = headptr;
	for(;;){	
		if(itemptr==NULL) {
			break;
		}
		for( i=1; i<=ITEMSPERPAGE; i++) {

			if( itemsactive==0)
				break;
			if(itemptr->fields[i-1].s_chq_no == HV_LONG)
				break; 
			
			if(itemptr->fields[i-1].s_chq_no == cheque_no) {
#ifdef ENGLISH
                            fomen("That Item Already Exists, Please Re-enter");
#else
			    fomen("Cet item existe deja, S.-V.-P. reessayer");
#endif
			    get();
                            return(-1);
                        }
		}  /*  2nd for(;;)   */
		itemptr=itemptr->nextptr;
	}  /*   1st for(;;)    */
	return(0);
}   /*  ItemCheck()    */

/*-------------------------------------------------------------------*/
static int
ShowPage()	/* Dump contents of node pointed to by tempptr onto screen */
{
					/* transfer data */
	if( ListToScreen()<0 ) return(-1);
	 				/* display on screen */
	if( WriteFields( ST_ITEMS, END_ITEMS )<0 ) return(-1);
					/* Paginate properly */ 
	if( PutPageNumber( current.page )<0 )return(-1);
	return(0);
}   /*   ShowPage()   */
/*-------------------------------------------------------------------*/
static int
ListToScreen()	/* copy contents of *tempptr to screen (items part) */
{
	short i;

	if( tempptr==NULL )
		return(-1);
	for( i=0; i<tempptr->lines_entered; i++){ /* for entries already made */
		s_sth.s_entries[i].s_chq_no = tempptr->fields[i].s_chq_no;
		s_sth.s_entries[i].s_amount = tempptr->fields[i].s_amount;
		strncpy(s_sth.s_entries[i].s_line_status,
					tempptr->fields[i].s_line_status ,4);
	}
	for(i=tempptr->lines_entered; i<ITEMSPERPAGE; i++){/* for other lines */
		s_sth.s_entries[i].s_chq_no = HV_LONG;
		s_sth.s_entries[i].s_amount = HV_DOUBLE;
		s_sth.s_entries[i].s_line_status[0] = HV_CHAR;
	}
	return(0);
}
/*-------------------------------------------------------------------*/
static int
MakeFreshPage()	/* Add a new node to the linked list */
{
	tempptr=PageAllocated();
	if( tempptr==NULL ){
#ifdef ENGLISH
             fomen("Internal memory allocation error. Press a key");
#else
             fomen("Erreur d'allocation a la memoire interne. Appuyer sur une touche");
#endif
	  get();
	  return(ALLOC_ERROR);
	}
	if( tailptr == NULL ){	/* If not a single node in the list yet */
		headptr=tailptr=tempptr;
		tempptr->prevptr = NULL;
		tempptr->nextptr = NULL;
	}
	else{			/* if at least one node exists */
		tailptr->lines_entered = current.line;
		tailptr->nextptr = tempptr;
		tempptr->prevptr = tailptr;
		tempptr->nextptr = NULL;
		tailptr = tempptr;
	}
	tempptr->lines_entered = 0;
	if( ClearItemLines()<0 ) 
		return(-1);
	if( WriteFields( ST_ITEMS,END_ITEMS )<0 )
		return(-1);
	return(0);
}
/*-------------------------------------------------------------------*/
static int
PutPageNumber(pgno)		/* Display the given number as Page No */
short pgno;
{
	s_sth.s_page = pgno;
	return(WriteFields( PGNO_FLD, PGNO_FLD));
}


/*-------------------------------------------------------------------*/
static int
ProcessCashing()	 
{
	int	retval;
	if(InitPage()<0) return(-1);

	/* set fund default */
	s_sth.s_fund = 1;
	fomca1(FUND_KEY,19,2);
	WriteFields(FUND_KEY,FUND_KEY);
	s_sth.s_fund = LV_SHORT;

	s_sth.s_account[0] = LV_CHAR;
	retval = ReadFields(FUND_KEY, ACCT_KEY);
	if(retval == EXIT) {
		FreeList();
		return(0);
	}
	if(retval < 0)
		return(retval);

    for( ; ; ){
#ifdef ENGLISH
	    DisplayMessage("Y(es),A(dd items),D(elete item),R(eactivate item),N(ext),P(rev),C(ancel)");
#else
	    DisplayMessage("O(ui), R(aj artic), E(lim art), V(ivifier art), S(uivant), P(rec), A(nnul)");
#endif

	sr.nextfld = MESG_FLD;
	fomrf( (char *)&s_sth );	/* Read user's option */
	ret( err_chk(&sr) );
	switch(s_sth.s_opt[0]){

		case ADDREC:	/* Allow additional items to be entered.
					CHG_MODE and ADD_MODE only.	*/
			 
			while( tempptr && tempptr->nextptr ){
				tempptr = tempptr->nextptr;
				current.page ++;
			}
			if( tempptr ){
				current.line = tempptr->lines_entered;
				if( current.line<1 )
					current.line = ITEMSPERPAGE;
				if( ShowPage()<0 ) return(-1);
			}
			if(!tempptr) {
				retval = MakeFreshPage();
				if(retval <0 || retval == ALLOC_ERROR )
					return(retval);
				current.line = 0;
				if( ShowPage()<0 ) return(-1);
			}

			if( AddItemsInLoop()<0 ) return(-1);
			break;

		case DELETE:
		case REACTIVATE:	
			if( !tempptr )	/* if no page is being pointed to */
				break;
			if( ChangeItemStatus()<0 )
				return(-1);
			if( HideFldNo()<0 )
				return(-1);
			break;
		case NEXT:	/* Display the next page of items */
			if(tempptr && tempptr->nextptr){
				tempptr = tempptr->nextptr;
				current.page ++;
				if(ShowPage()<0 ) return(-1);
			}
			else
#ifdef ENGLISH
				fomer("Last Page Displayed");
#else
				fomer("Derniere page affichee");
#endif
			break;
		case PREV:	/* Display the prev page of items */
			if(tempptr && tempptr->prevptr){
				tempptr = tempptr->prevptr;
				current.page --;
				if(ShowPage()<0 ) return(-1);
			}
			else
#ifdef ENGLISH
				fomer("First Page Displayed");
#else
				fomer("Premiere page affichee");
#endif
			break;
		case CANCEL:    /* Cancel the entire session in ADD MODE only*/
#ifdef ENGLISH
			DisplayMessage("Confirm the Cancel (Y/N)?"); 
#else
			DisplayMessage("Confirmer l'annulation (O/N)?"); 
#endif
			for( ; ; ){
				sr.nextfld = MESG_FLD;
				fomrf( (char *)&s_sth);
				if(s_sth.s_opt[0]!= YES &&
				   s_sth.s_opt[0]!= NO)
					continue;
				if( s_sth.s_opt[0]==YES ){
					FreeList();
					if(HideMessage()<0 )
						return(-1);
					return(EXIT);
				}
				else
					break;
			}
			break;
		case YES:	/* Save & Exit in ADD MODE,Exit in INQ MODE */
			if( totalitemsadded<1){
#ifdef ENGLISH
				fomer("No items to save. Cancel to Quit");
#else
				fomer("Pas d'articles a conserver. Annuler pour retourner");
#endif
				break;
			}
			else
				return(HideMessage());
			break;
	}    /*   switch     */
	continue;
   }  /*  for	*/
}     /*   ProcessCashing()     */
/*-------------------------------------------------------------------*/
/*                                                                   */
static int
ChangeItemStatus()	/* Allow deletion or reactivation of an item */
{
	int fld_no, retval;

	for( ; ; ){
		s_sth.s_field = LV_SHORT;
		if( (retval=ReadFields(CHG_FLD,CHG_FLD)) < 0 )
			return(-1);
		if( retval==EXIT )
			return(retval);
		if(s_sth.s_field==0) 
			break;
		if( s_sth.s_field>tempptr->lines_entered )
			continue;
		switch( s_sth.s_opt[0] ){
			case DELETE:
#ifdef ENGLISH
			if( strcmp(s_sth.s_entries[s_sth.s_field-1].s_line_status,"DEL") != 0){
				STRCPY(s_sth.s_entries[s_sth.s_field-1].s_line_status, "DEL");
			   	totalitemsadded--;
			   	itemsactive--;
			}
#else
			if(strcmp(s_sth.s_entries[s_sth.s_field-1].s_line_status,"ELI")!=0)  {
				STRCPY(s_sth.s_entries[s_sth.s_field-1].s_line_status, "ELI");
				totalitemsadded--;
				itemsactive--;
			}
#endif
			break;

			case REACTIVATE:
#ifdef ENGLISH
			if(strcmp(s_sth.s_entries[s_sth.s_field-1].s_line_status,"ACT") != 0){
			   	STRCPY(s_sth.s_entries[s_sth.s_field-1].s_line_status, "ACT");
			   	totalitemsadded++;
			   	itemsactive++;  
			}
#else
			if(strcmp(s_sth.s_entries[s_sth.s_field-1].s_line_status,"ACT") != 0){
			   	STRCPY(s_sth.s_entries[s_sth.s_field-1].s_line_status, "ACT");
			   	totalitemsadded++;
			   	itemsactive++;  
			}
#endif
			break;

			default:
				return(0);
		}  /* switch   */
		STRCPY(tempptr->fields[s_sth.s_field-1].s_line_status ,
			 s_sth.s_entries[s_sth.s_field-1].s_line_status);
		fld_no = ST_STATUS + ( (s_sth.s_field-1)*STEP );
		if( WriteFields(fld_no,fld_no)<0 )
			return(-1);
	}    /*      for     */
	return(0);
}      /*    ChangeItemStatus    */
/*-------------------------------------------------------------------*/
static  int
WriteSession()	/* Write list of items update file */
{
	int i, retval;

				/* delete all items for a specific course */
	tempptr = headptr;	/* initialize tempptr to beginning of list */

	for( ; ; ){	/* to write the items in file */

		if(tempptr==NULL)
			break;
		
		for( i=1; i<=ITEMSPERPAGE; i++){ /* for each line on page */
			if(tempptr->fields[i-1].s_chq_no == HV_LONG)
				break;

			if(itemsactive==0)
				break;
			
			if(tempptr->fields[i-1].s_line_status[0]==DELETE)
				continue;

			retval = WriteItem(i);
			if(retval<0 ){
				roll_back(e_mesg) ;
				return(retval);
			} 
			itemsactive--;
	
		}   /* end inner for loop */ 

		if( tempptr->fields[i-1].s_chq_no == HV_LONG )
			break;	/* terminate when High values are found */
	
		tempptr=tempptr->nextptr;

	}  /* end outer for loop */

	return(0);
}    /*  WriteSession()   */

/*-------------------------------------------------------------------*/
static  int
WriteItem(line_no)/* Write one item to student file from *tempptr */
int line_no;
{
	int	retval, err;

/*	flg_reset(CHQHIST); */	
	cheque.ch_funds = s_sth.s_fund;
	STRCPY(cheque.ch_accno, s_sth.s_account);
	cheque.ch_chq_no = tempptr->fields[line_no-1].s_chq_no;
	
	retval = get_chqhist(&cheque,UPDATE,0,e_mesg);

	if (retval==NOERROR && (cheque.ch_chq_no ==
			tempptr->fields[line_no-1].s_chq_no)){

		scpy((char*)&pre_rec,(char*)&cheque,sizeof(Chq_hist));
		cheque.ch_status[0] = CASHED;
		retval = put_chqhist( &cheque, UPDATE, e_mesg );
		if( retval!=NOERROR ){
			fomen(e_mesg);
			get();
			roll_back(e_mesg);
			return(retval);
		}

	}
	else	{
		roll_back(e_mesg);
		return(retval);
	}

	if(rpline(arayptr)<0)
		return(-1);
	
	err=rite_audit((char*)&s_sth, CHQHIST,UPDATE,(char*)&cheque,
			(char*)&pre_rec,e_mesg);
	if(err==LOCKED) {
		DispError(e_mesg);
		roll_back(e_mesg);
		return(LOCKED);
	}
	if(err != NOERROR){
#ifdef	ENGLISH
		fomen("ERROR in Saving Records"); 
#else
		fomen("ERREUR en conservant les fiches");
#endif
		fomen(e_mesg);
		get();
		roll_back(e_mesg);
		return(DBH_ERR);
	}

	if ( commit(e_mesg) < 0 ){
		fomen(e_mesg);
		get();
		return(-1);
	}

	return(0);
}     /*   WriteItem()   */
/*-------------------------------------------------------------------*/
static  int
FreeList()	/* Free the linked list */
{
	for( tempptr=headptr; tempptr; tempptr=headptr ){
		headptr=headptr->nextptr;
		free( (char *)tempptr );
	}
	tailptr = NULL;
	return(0);
}
/*-------------------------------------------------------------------*/
static int
InitPage()	/* Init first pageful of items */
{
int retval;

	
	tempptr = headptr ;	/* Seek to first node/page of list */
	current.page = 1;
    	if( PutPageNumber(current.page)<0 )
		return(-1);
	
	return(0);
}
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* show or clears message field 					 */
static int
ShowMesg()  
{
	sr.nextfld = MESG_FLD - 100;
	fomwf((char*)&s_sth);
	return(0);
}
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* show ERROR and wait            					 */
static int
DispError(s)
char *s;
{
	STRCPY(s_sth.s_mesg,s);
	ShowMesg();
#ifdef	ENGLISH
	fomen("Press any key to continue");
#else
	fomen("Appuyer sur une touche pour continuer");
#endif
	get();
	s_sth.s_mesg[0] = HV_CHAR;
	ShowMesg();
	return(ERROR);
}   /*   DispError()   */

/*---------------------  E N D   O F   F I L E  -------------------------*/

