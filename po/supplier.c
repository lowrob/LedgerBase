/*-----------------------------------------------------------------------
Source Name: supplier.c
System     : Budgetary Financial system.
Module     : General Ledger system.
Created  On: 2nd Sept 89.
Created  By: Cathy Burns.

DESCRIPTION:
	Program to Maintain Supplier Info.
        This program provides Add, Change, Inquiry & Delete
	options on Supplier File.
	This Program also writes Audit records for the changes.


MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
J.Prescott     90/11/21       Added Payee Field
p ralph        90/11/23       right justify numeric supplier code.
S.Whittaker	93/05/28	Updated the rite_audit function to include
				passing the screen structure.
------------------------------------------------------------------------*/
#define  MAIN
#define  MAINFL		SUPPLIER		/* main file used */

#include <stdio.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	SYSTEM		"PURCHASE ORDER"	/* Sub System Name */
#define	MOD_DATE	"21-NOV-90"		/* Program Last Modified */

#ifdef ENGLISH
#define ADDREC		'A'
#define CHANGE		'C'
#define DELETE		'D'
#define NEXT		'N'
#define PREV		'P'
#define INQUIRE		'I'
#define EXITOPT		'E'

#define	EDIT		'E'
#define	CANCEL		'C'
#define	YES		'Y'
#define NO		'N'
#else
#define ADDREC		'R'
#define CHANGE		'C'
#define DELETE		'E'
#define NEXT		'S'
#define PREV		'P'
#define INQUIRE		'I'
#define EXITOPT		'F'

#define	EDIT		'M'
#define	CANCEL		'A'
#define	YES		'O'
#define NO		'N'
#endif

#define EXIT		12

#define SPACE           ' '

#define	SCR_NAME	"supplier"	/* First screen */

/* PROFOM Field Numbers */
#define	LAST_SNO	21
#define	CHNG_LIMIT	16	/* change mode can't change beyond this fld */
#define ADD_LIMIT	20	/* add mode can't change beyond this field  */

/* Screen Control Variables */
#define	ST_FLD	        800		/* Data entry starting field */
#define	READ_FLD	800		/* Data entry starting field */
#define	END_FLD  	2800		/* screen end field */
#define MESG_FLD	3000		/* Mesg field */

#define	FN_FLD		400		/* Fn: */
#define	KEY_START	500		/* Key Start Field */
#define	KEY_END		500		/* Key Start Field */
#define	CHG_FLD		600		/* Field: */
#define	SUPP_FLD	500		/* Supplier code field */
#define NAME_FLD	800		/* Supplier name field */
#define ABB_FLD		900		/* Supplier abbreviated name field */
#define PC_FLD		1400		/* Postal Code Field */
#define	FAX_FLD		1500		/* Fax Number Field */
#define PHONE_FLD	1700		/* Phone Number Field */
#define	TYPE_FLD	1900		/* Supplier Type field */
#define PAYEE_FLD 	2000		/* Payee Field */
#define GST_FLD 	2100		/* GST Registration Field */
#define DISC_FLD 	2200		/* Discount Field */
	
static	struct  stat_rec sr;		/* PROFOM status rec */

static  Supplier supplier, pre_rec, payee_rec;	/* Supplier Master Record */
static 	Pa_rec	pa_rec;
static	Invoice	in_rec;
static 	Po_hdr	po_hdr;
static	char 	e_mesg[80];  		/* dbh will return err msg in this */

typedef struct {
	char	s_pgm[11];		/* 100 String 10 */
	long	s_rundate;		/* 300 Date 9999F99F99 */
	char 	s_fn[2];		/* 400 Function */
	char	s_supp_cd[11];		/* 500 Supplier Code key */
	short	s_field;		/* 600 Field no */
	char 	s_name[49];		/* 800 Supplier name */
	char	s_abb[25];		/* 900 Supplier abbreviated name */
	char	s_category[21];		/* 1000 Supplier abbreviated name */
	char	s_add1[31];		/* 1100 Address line 1 */
	char	s_add2[31];		/* 1200 Address line 2 */
	char	s_add3[31];		/* 1300 Address line 3 */
	char	s_pc[11];		/* 1400 Postal Code */
	char 	s_fax[11];		/* 1500 Fax XXXFXXXFXXXX */
	char	s_contact[26];		/* 1600 Contact name */
	char 	s_phone[11];		/* 1700 Phone XXXFXXXFXXXX */
	int	s_tmp_flg;		/* 1800 Temporary Supplier Flag */
	char	s_type[2]; 		/* 1900 Supplier Type */
	char	s_payee[11];		/* 2000 Payee Code */
	char	s_gst_reg[11];		/* 2100 GST Registration number */
	double	s_discount;		/* 2200 discount percentage */
	double	s_min_dollar;		/* 2300 Min $ Required */
	double	s_ytd_ord;		/* 2400 discount percentage */
	double 	s_ytd_ret;		/* 2500 ytd returns */
	double	s_ytd_recpt;		/* 2600 ytd receipts */
	double 	s_ytd_disc;		/* 2700 ytd discount */
	double	s_balance;		/* 2800 supplier balance */
	char	s_mesg[78];		/* 2900 message line */
	char	s_opt[2];		/* 3000 option entry */
	} s_struct;

static	s_struct	s_sth ; 


main(argc,argv)
int argc;
char *argv[];
{
	strncpy(SYS_NAME,SYSTEM,50);	/* Sub system name */
	strncpy(CHNG_DATE,MOD_DATE,10);	/* Modification Date */
	proc_switch(argc, argv, MAINFL) ; 	/* Process Switches */

	STRCPY(sr.termnm,terminal);	/* Copy Terminal Name */

	if (Initialize()<0)
		exit(-1);		/* Initialization routine */


	if ( Process() < 0) { 		/* Initiate Process */
		Close();
		exit(-1);
	}

	Close();			/* return to menu */
	exit(NOERROR);

} /* END OF MAIN */

/*-------------------------------------------------------------------*/
/* Reset information */
Close()
{
	/* Set terminal back to normal mode from PROFOM */
	fomcs();
	fomrt();
	close_dbh();			/* Close files */
	free_audit();
}
/*-------------------------------------------------------------------*/
/* Initialize PROFOM  and Screens*/
Initialize()
{
	if(InitProfom()<0) { 			/* Initialize PROFOM */
		fomcs();
		fomrt();
		return(-1);
	}	
	if(InitScreens()<0) { 
		fomcs();
		fomrt();
		return(-1);
	}	
	return(NOERROR);
}

/*-------------------------------------------------------------------*/
/* Initialize PROFOM */

InitProfom()
{
	fomin(&sr);
	/* Check for Error */
	ret( err_chk(&sr) );

	fomcf(1,1);			/* Enable Print screen option */
	return(NOERROR);
}	/* InitProfom() */
/*----------------------------------------------------------------*/
/* Initialize screens before going to process options */
InitScreens()
{
	STRCPY(sr.scrnam,NFM_PATH);
	strcat(sr.scrnam,SCR_NAME) ;

	STRCPY(s_sth.s_pgm,PROG_NAME);
	s_sth.s_rundate = get_date();	/* get Today's Date in YYMMDD format */
	s_sth.s_field = HV_SHORT ;
	s_sth.s_mesg[0] = HV_CHAR ;
	s_sth.s_opt[0] = HV_CHAR ;

	/* Move Low Values to key fields. This gets 1st record from the file
	   if user selects 'N' option immediatly after invoking */

	s_sth.s_supp_cd[0] = LV_CHAR ;
	/* Move High Values to 1st screen data fields & Display */
	MoveHighs() ;

	return(NOERROR);

}	/* InitScreens() */
/*-------------------------------------------------------------------*/
/* Get Option from user and call corresponding function */

Process()
{
	int retval;

	/*  Checking if inventory system exists */
	retval = get_param(&pa_rec,BROWSE,1,e_mesg);
	if (retval < 0) {
		if (retval == UNDEF)
#ifdef ENGLISH
			return(DispError("Parameters are not Setup ..."));
#else
			return(DispError("Parametres ne sont pas etablis..."));
#endif
		else	
			return(DispError(e_mesg));
	}

	for( ; ; ){
		/* Display Fn: options */

		if (ReadFunction() < 0 ) return(-1);

		switch(s_sth.s_fn[0]){
		case ADDREC  :	/* Add */
			CHKACC(retval,ADD,e_mesg);
			if( AddNewRec()<0 ) return(-1);
			break;
		case CHANGE  :	/* Change */
			CHKACC(retval,UPDATE,e_mesg);
			if( ChangeRec()<0 ) return(-1);
			break;
		case DELETE  :	/* Delete */
			CHKACC(retval,P_DEL,e_mesg);
			if( DeleteRec()<0 ) return(-1);
			break;
		case NEXT  : 	/* Next */
			CHKACC(retval,BROWSE,e_mesg);
			if( NextRec()<0 ) return(-1);  
			break ;
		case PREV  :	/* Previous */
			CHKACC(retval,BROWSE,e_mesg);
			if( PrevRec()<0 ) return(-1); 
			break ;
		case INQUIRE  : 	/* Inquiry */
			CHKACC(retval,BROWSE,e_mesg);
			if( InquiryRec()<0 ) return(-1);
			break ;
		case EXITOPT  :
			return(NOERROR);
		default   : 
			continue; 
		}  /*   end of the switch statement */
		if( retval<0 ){
			fomen(e_mesg);
			get();
		}
		if( retval==DBH_ERR )
			return(retval);

	}      /*   end of the for( ; ; )       */
}	/* Process() */
/*-------------------------------------------------------------------*/
/* Reads the function they would like to execute      */
ReadFunction()
{

#ifdef	ENGLISH
	fomer("A(dd), C(hange), D(elete), N(ext), P(rev), I(nquire), E(xit)");
#else
	fomer("R(ajouter), C(hanger), E(liminer), S(uivant), P(recedent), I(nterroger), F(in)");
#endif

	/* Read Fn: field to get the option */
	sr.nextfld = FN_FLD ;
	fomrf((char*)&s_sth);
	ret(err_chk(&sr));	/* Check for PROFOM error */
	return(0);
}
/*---------------------------------------------------------------------*/
/*                                                      */
/* Read the full screen and add the record to data base */
AddNewRec()
{
     	int retval, i ;

	for( ; ;){
		if( (retval = ReadKey()) < 0 ) return(-1);

		if (retval == EXIT) return(EXIT);
		/* check whether given key already exists */
		if(BringRecord(BROWSE) == NOERROR){
#ifdef	ENGLISH
			fomer("Given Key already in File - Please Enter again") ;
#else
			fomer("Cle donnee deja dans le dossier - Reessayer") ;
#endif
			continue;
		}
		break;
	}


     	for ( i = ST_FLD; i <= END_FLD ; i += 100 ) {
       		fomca1( i,19,0 );	        /*  disable Dup Control */
       		fomca1( i,10,1 );      	       	/*  enable user escape */
     	}
	
	MoveLows();
	s_sth.s_ytd_ord = LV_DOUBLE ;
	s_sth.s_ytd_recpt = LV_DOUBLE ;
	s_sth.s_ytd_ret = LV_DOUBLE ;
	s_sth.s_ytd_disc = LV_DOUBLE ;
	s_sth.s_balance = 0.00 ;

     	retval = ReadFields(ST_FLD,END_FLD);
	if(retval == PROFOM_ERR) return(retval) ;
	if(retval == EXIT) {
		roll_back(e_mesg);
       		return( MoveHighs() ) ;
	}

	for( ; ; ) {
     		retval = ReadOption() ;
     		if (retval < 0) return(-1);
  
  	   	if ( retval == CANCEL ) 
       			return( MoveHighs() ) ;

 	    	CopyToRecord();

 	    	retval = RiteRecord(ADD) ;
		if(retval==NOERROR) break;
		if(retval==LOCKED)  {
			roll_back(e_mesg);
			if(retval < 0) {
				fomen(e_mesg);
				get();
				if(retval==UNDEF || retval==LOCKED) continue;
				return(DBH_ERR);
			}
			continue;
		}
		if(retval<0) return(retval);
	}
	return(NOERROR);
}   /*  AddNewRec  */
/*---------------------------------------------------------------------*/
/*                                                                     */
/* Change by field and update the record to data base                  */
ChangeRec()
{
     	int retval; 

     	if ( (retval = SelectRecord(UPDATE)) < 0 ) return(-1);

	if (retval == EXIT) return(EXIT);
     
     	scpy((char*)&pre_rec,(char*)&supplier,sizeof(supplier));
     	retval = ChangeFields();
	if ( retval < 0 ) return(-1);

    	for( ; ; ) { 
     		retval = ReadOption() ;
     		if ( retval < 0 ) return(-1);
  
  	   	if ( retval == CANCEL )  {
        		CopyToScreen() ;
        		roll_back(e_mesg) ;
        		return( NOERROR ) ;
      		}

 	    	CopyToRecord();

 	    	retval = RiteRecord(UPDATE);
		if(retval==NOERROR) break;
		if(retval==LOCKED)  {
			roll_back(e_mesg);
			retval = BringRecord(UPDATE);
			if(retval < 0) {
				fomen(e_mesg);
				get();
				if(retval==UNDEF || retval==LOCKED) continue;
				return(DBH_ERR);
			}
			continue;
		}
		if(retval<0) return(retval);
	}
	return(NOERROR);
}	/* ChangeRec() */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Select the record to be deleted and delete from database after	 */
/* confirmation 							 */
DeleteRec()
{
       	int retval;
 
       	if( (retval = SelectRecord(UPDATE)) < 0 ) return(-1) ;


	if (retval == EXIT) return(EXIT);

	if (supplier.s_balance != 0.00) {
#ifdef ENGLISH
		fomen("Cannot Delete Supplier, Balance not Zero");
#else
		fomen("Ne peut pas eliminer le fournisseur, solde n'est pas zero");
#endif
		get();
		return(NOERROR);
	}

	for( ; ; ) {
       	retval =  ReadOption();
       	if ( retval < 0 ) return(-1);

       	if ( retval == CANCEL ) {
       	   	roll_back(e_mesg);
          	return(NOERROR);
       	}

	/* Check for Outstanding PO's */
	STRCPY(po_hdr.ph_supp_cd, supplier.s_supp_cd);
	po_hdr.ph_code = 0;
	flg_reset(POHDR) ;
	retval = get_n_pohdr(&po_hdr, BROWSE, 1,FORWARD, e_mesg);
	if (retval >= 0) 		/* Record Exists */
	   if ((strcmp(po_hdr.ph_supp_cd, supplier.s_supp_cd)) == 0) {  
#ifdef ENGLISH
		fomen("PO's outstanding for that supplier");
#else
		fomen("BC non-regles pour ce fournisseur");
#endif
		get();
		return(NOERROR);
	   }

	/* Check for Outstanding INV */
	STRCPY(in_rec.in_supp_cd, supplier.s_supp_cd);
	in_rec.in_invc_no[0] = '\0' ;
	in_rec.in_tr_type[0] = '\0';
	flg_reset(APINVOICE) ;
	retval = get_n_invc(&in_rec, BROWSE, 0, FORWARD, e_mesg);
	if (retval >= 0)     		/* Record Exists */
	   if ((strcmp(in_rec.in_supp_cd, supplier.s_supp_cd)) == 0) {  
#ifdef ENGLISH
		fomen("Invoices outstanding for that supplier");
#else
		fomen("Factures non-reglees pour ce fournisseur");
#endif
		get();
		return(NOERROR);
	   }

 	    	retval = RiteRecord(P_DEL);
		if(retval==NOERROR) break;
		if(retval==LOCKED)  {
			roll_back(e_mesg);
			retval = BringRecord(UPDATE);
			if(retval < 0) {
				fomen(e_mesg);
				get();
				if(retval==UNDEF || retval==LOCKED) continue;
				return(DBH_ERR);
			}
			continue;
		}
		if(retval<0) return(retval);
	}
	return(NOERROR);
}	/* DeleteRec() */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Select the Key, get the record & display */
InquiryRec()
{
        int      retval;
  
        if( retval == SelectRecord(BROWSE)<0 ) return(-1) ;

	if( retval == EXIT) return(EXIT);

        return( NOERROR );
}
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Get next rec from file & Display when user selected 'N' in function */
NextRec()
{
       BringNext(0) ;

       return(NOERROR);
}
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Get Previous record from file & Display when user selected 'N' in function */
PrevRec()
{
       BringNext(1) ;

       return(NOERROR);
}
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Read the key fields, get the record with LOCK and display the record  */
SelectRecord(md)
int	md ;	/* BROWSE for Inquiry, UPDATE for Others */
{
	int	retval, err ;

	for(; ;){
		if( (retval = ReadKey()) < 0 ) return(-1);
	
		if (retval == EXIT) return(EXIT);

		/* Get the record from database */

		err = BringRecord(md);
		if(err < 0){
			DispError(e_mesg);
			if(err == UNDEF || err == LOCKED) continue ;
			return(-1);
		}
		CopyToScreen() ;
		return(NOERROR) ;
	}
}	/* SelectRecord() */
/*----------------------------------------------------------------------*/
/*                                                                       */
/* Read Key Fields */
ReadKey()
{
	int	retval;
	char 	key_supplier[sizeof(s_sth.s_supp_cd)];
	int	i;

#ifdef	ENGLISH
	STRCPY(s_sth.s_mesg,"Press ESC-F to Go Back to Fn:");
#else
	STRCPY(s_sth.s_mesg,"Appuyer sur ESC-F pour retourner a Fn:");
#endif
	ShowMesg();

	/* Store key fields to copy back when user gives ESC-F */

	STRCPY( key_supplier, s_sth.s_supp_cd );

	if (s_sth.s_fn[0] == ADDREC) 
		fomca1(KEY_START,19,0);
	else {
		fomca1(KEY_START,19,2);
		sr.nextfld = KEY_START;
		sr.endfld = KEY_END;
		fomud((char*)&s_sth);
	}
	s_sth.s_supp_cd[0] = LV_CHAR ;

	retval = ReadFields(KEY_START, KEY_END);

	if (retval < 0) return(-1);

	s_sth.s_mesg[0] = HV_CHAR;
	ShowMesg();

	if (retval == EXIT) {
		STRCPY( s_sth.s_supp_cd, key_supplier);
		if ( WriteFields(KEY_START,KEY_END) < 0) return(-1);
		return(EXIT);
	}
	STRCPY(s_sth.s_pgm,PROG_NAME);

	return(NOERROR);

}	/*  ReadKey() */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Copy the key fields from screen to data record and get the record from
   data base */
static int
BringRecord(md)
int md; /* BROWSE or UPDATE */
{
	STRCPY(supplier.s_supp_cd, s_sth.s_supp_cd);

	return(get_supplier(&supplier, md, 0, e_mesg));
}	/* BringRecord() */ 
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Changing fields. Accept fld to be changed and read that fld 		 */
ChangeFields()
{
     	int i,retval,fld_no;

     	for ( i = ST_FLD; i <= END_FLD ; i += 100 ) {
       		fomca1( i,19,2 );      		/*  enable Dup Control */
       		fomca1( i,10,1 );      		/*  enable user escape */
     	}

     	sr.nextfld = ST_FLD;
     	sr.endfld = END_FLD;
     	fomud( (char *) &s_sth );
     	ret(err_chk(&sr));

	/* Get The Field to Be Modified */
#ifdef	ENGLISH
	STRCPY(s_sth.s_mesg,"Enter RETURN to terminate Edit");
#else
	STRCPY(s_sth.s_mesg,"Appuyer sur RETURN pour terminer l'ajustement");
#endif
	ShowMesg() ;
        
     	for (; ;) {
		s_sth.s_field = LV_SHORT;
		retval = ReadFields(CHG_FLD, CHG_FLD);

		if (retval < 0) return(-1);
		if (retval == EXIT) break;		/* User enter ESC-F */

       		if (s_sth.s_field == 0 ) break;	/* Finished changing fields */

		if(s_sth.s_field > LAST_SNO) continue ;
		if(s_sth.s_fn[0] == CHANGE && s_sth.s_field > CHNG_LIMIT)
			continue;
		if(s_sth.s_fn[0] == ADDREC && s_sth.s_field > ADD_LIMIT)
			continue;

        	fld_no = ST_FLD + (100 * (s_sth.s_field - 1)) ;

		sr.nextfld = fld_no;
		for(; ;) {
			fomrf((char*)&s_sth);
			ret(err_chk(&sr));

			if(sr.retcode == RET_VAL_CHK){
				if (Validate(sr.curfld) == NOERROR) break; 
				sr.nextfld = sr.curfld;
				continue;
			}

			if(sr.retcode == RET_USER_ESC){
			   if(sr.escchar[0] == 'F' || sr.escchar[0] == 'f')  
					break;
			   if(sr.escchar[0] == 'h' || sr.escchar[0] == 'H')
					if( WindowHelp() < 0 ) return(-1) ;
			   continue;
			}
			break;
		}
	}

     	s_sth.s_field = HV_SHORT ;
	if ( WriteFields(CHG_FLD, CHG_FLD) < 0 ) return(-1);

     	s_sth.s_mesg[0] = HV_CHAR ;
     	ShowMesg();

	if (retval == EXIT) 
		return(EXIT);
	else
     		return(NOERROR);

}	/* ChangeFields() */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Reads the fields in the range given in the parameters.                */
static
ReadFields(begin,end)
int 	begin, end;
{
	int	err;

	sr.nextfld = begin;
	sr.endfld = end;

	for(; ;) {
		fomrd((char*)&s_sth);
		ret(err_chk(&sr));

		if(sr.retcode == RET_VAL_CHK){
			err = Validate(sr.curfld) ;
			if (err == DBH_ERR) return(-1); 
			sr.nextfld = sr.curfld;
			continue;
		}

		if(sr.retcode == RET_USER_ESC){
			if(sr.escchar[0] == 'F' || sr.escchar[0] == 'f') { 
				return(EXIT);
			}
			if(sr.escchar[0] == 'h' || sr.escchar[0] == 'H'){
				if( WindowHelp() < 0 ) return(-1) ;
			}
			continue;
		}
		break;
	}			/* end of for loop */
	return(NOERROR) ;
} 	/* end of ReadFields */

/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Writes the fields in the range given in the parameters.               */

static
WriteFields(begin,end)
int 	begin, end;
{
	sr.nextfld = begin;
	sr.endfld = end;

	fomwr((char*)&s_sth);
	ret(err_chk(&sr));
	return(NOERROR);

}	/* end of WriteFields */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Display the confirmation message at the bottom of the screen, take the
   option from user and call necessary functions */
ReadOption()
{
	int	err ;

	/* Options:
	   Add      - YEC
	   Change   - YEC
	   Delete   - YC
	*/

	for( ; ; ) {
	    switch(s_sth.s_fn[0]) {
	    case	ADDREC :	/* Add */
#ifdef ENGLISH
                    err = GetOpt( "Y(es), E(dit), C(ancel)", "YEC");
#else
                    err = GetOpt( "O(ui), M(odifier), A(nnuler)", "OMA");
#endif
		    break ;
		/* Fall thru */
	    case	CHANGE :	/* Change */
#ifdef ENGLISH
                    err = GetOpt( "Y(es), E(dit), C(ancel)", "YEC");
#else
                    err = GetOpt( "O(ui), M(odifier), A(nnuler)", "OMA");
#endif
	            break ;
	    case	DELETE :	/* Delete */
#ifdef ENGLISH
                    err = GetOpt( "Y(es), C(ancel)", "YC");
#else
                    err = GetOpt( "O(ui), A(nnuler)", "OA");
#endif
		    break ;
	    }	/* switch fn[] */

	    if(err == PROFOM_ERR) return(err) ;

	    switch(err) {
	    case  YES :
		return(YES) ;
	    case  EDIT  :
		err = ChangeFields();
		break ;
	    case  CANCEL :
#ifdef	ENGLISH
                err = GetOpt("Confirm the Cancel (Y/N)?", "YN") ;
#else
                err = GetOpt("Confirmer l'annulation (O/N)?", "ON") ;
#endif
                if ( err == YES ) 
                   return( CANCEL );
		break ;
	    }	/* switch err */

	    if(err == PROFOM_ERR) return(err) ;
	}	/* for(; ; ) */
}	/* ReadOption() */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/*  Display help window for applicable fields                            */
WindowHelp()
{
	int	err ;

	switch(sr.curfld){
	case	SUPP_FLD	:
		err = supp_hlp(s_sth.s_supp_cd, 7, 13 );
		if (err == DBH_ERR) return(-1) ;
		if (err >= 0 ) redraw();
		break;
	case	PAYEE_FLD	:
		err = supp_hlp(s_sth.s_payee, 7, 13 );
		if (err == DBH_ERR) return(-1) ;
		if (err >= 0 ) redraw();
		if(s_sth.s_fn[0] == CHANGE) {
     			sr.nextfld = PAYEE_FLD;
		     	sr.endfld = PAYEE_FLD;
		     	fomud( (char *) &s_sth );
		     	ret(err_chk(&sr));
     			sr.nextfld = ST_FLD;
		     	sr.endfld = END_FLD;
		}
		break;
	default :
#ifdef ENGLISH
		fomer("No Help Window For This Field");
#else
		fomer("Pas de fenetre d'assistance pour ce champ");
#endif
	}	/* Switch fld_no */

	sr.nextfld = sr.curfld ;
	return(NOERROR) ;
}	/* WindowHelp() */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Write record in data base */
RiteRecord(md)
int md;
{
        int	err;

	err = put_supplier(&supplier,md,e_mesg);
	if(err != NOERROR){
#ifdef	ENGLISH
		DispError("ERROR in Saving Records"); 
#else
		DispError("ERREUR en conservant les fiches");
#endif
		roll_back(e_mesg);
		MoveHighs() ;
		return(err);
	}

	err = rite_audit((char*)&s_sth,SUPPLIER,md,(char*)&supplier,(char*)&pre_rec,e_mesg);
	if(err != NOERROR){
#ifdef	ENGLISH
		DispError("ERROR in Saving Records"); 
#else
		DispError("ERREUR en conservant les fiches");
#endif
		DispError(e_mesg);
		roll_back(e_mesg);
		MoveHighs() ;
		return(err);
	}

	err = commit(e_mesg);
	if (err == LOCKED) {
		DispError(e_mesg);
       	   	roll_back(e_mesg);
          	return(NOERROR);
	}
	if (err < 0) {
#ifdef	ENGLISH
		DispError("ERROR in Saving Records"); 
#else
		DispError("ERREUR en conservant les fiches");
#endif
		DispError(e_mesg);
		MoveHighs() ;
		return(err);
	}

	return(NOERROR);

}	/* RiteRecord() */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Validate flds when PROFOM returns RET_VAL_CHK                         */
Validate(fld_no)	
int	fld_no ;
{
	int	err, valid_flag, i;

	switch(fld_no){
	case 	SUPP_FLD:	/* Supplier Code */
		Right_Justify_Numeric(s_sth.s_supp_cd,
					sizeof(s_sth.s_supp_cd)-1);
		break ;
        case    TYPE_FLD   :   	/* Account number */
		if (s_sth.s_type[0] != CONTRACT && 
				s_sth.s_type[0] != ORDINARY) {
#ifdef ENGLISH
			fomen("Must be C(ontract) or O(rdinary)");
#else
			fomen("Doit etre C(ontractuel) ou O(rdinaire)");
#endif
			s_sth.s_type[0] = LV_CHAR ;
			return(ERROR) ;
		}
		if (s_sth.s_type[0] == CONTRACT) 
			s_sth.s_ytd_disc = 0.00 ;
		break;         
	case	NAME_FLD   :	/* Name of supplier */
		/* Show name as default */
		fomca1(ABB_FLD, 19, 2) ;
		sr.nextfld = ABB_FLD ;
		sr.endfld  = ABB_FLD ;
		strncpy(s_sth.s_abb, s_sth.s_name,24);
		fomud((char*)&s_sth);
		ret(err_chk(&sr)) ;
		s_sth.s_abb[0] = LV_CHAR ;

		break;
	case	PAYEE_FLD   :	/* Payee Fld */
		if(s_sth.s_payee[0] == '\0') {
			sr.curfld = GST_FLD ;
			break;
		}
		Right_Justify_Numeric(s_sth.s_payee,
					sizeof(s_sth.s_payee)-1);
		STRCPY(payee_rec.s_supp_cd, s_sth.s_payee) ;
		err = get_supplier(&payee_rec, BROWSE, 0, e_mesg) ;
		if(ERROR == err) return(DBH_ERR) ;
		if(err < 0) {
			fomer(e_mesg) ;
			s_sth.s_payee[0] = LV_CHAR ;
			return(ERROR) ;
		}
		fomer(payee_rec.s_name);
		break ;
	case	DISC_FLD    :	/* Discount field */
/*		if(s_sth.s_discount > 100) { 
#ifdef ENGLISH
			fomer("Discount percent cannot be greater than 100");
#else
			fomer("Pourcentage de l'escompte ne peut pas etre plus que 100");
#endif
			s_sth.s_discount = LV_DOUBLE ;
			return(ERROR) ;
		}
		break ;
*/
	case PHONE_FLD:
		if(s_sth.s_phone[0] == '\0'){
			sr.curfld += 100;
			return(NOERROR);
		}

		valid_flag = 0;
		for (i=0; i<10 && !valid_flag; i++) {
			if ((isdigit((int)s_sth.s_phone[i]))==0)
				valid_flag = 1;
		}
		if (valid_flag) {
#ifdef	ENGLISH
			fomen("Must be All Digits");
#else
			fomen("Doit etre des chiffres");
#endif
			s_sth.s_phone[0] = LV_CHAR;
			sr.nextfld = PHONE_FLD;
			return (ERROR);
		}
		break;
	case FAX_FLD:
		if(s_sth.s_fax[0] == '\0'){
			sr.curfld += 100;
			return(NOERROR);
		}

		valid_flag = 0;
		for (i=0; i<10 && !valid_flag; i++) {
			if ((isdigit((int)s_sth.s_fax[i]))==0)
				valid_flag = 1;
		}
		if (valid_flag) {
#ifdef	ENGLISH
			fomen("Must be All Digits");
#else
			fomen("Doit etre des chiffres");
#endif
			s_sth.s_fax[0] = LV_CHAR;
			sr.nextfld = FAX_FLD;
			return (ERROR);
		}
		break;
	case PC_FLD:
		if(s_sth.s_pc[0] == '\0'){
			sr.curfld += 100;
			return(NOERROR);
		}
/***** REMOVED DUE TO AMERICAN ZIP CODES DO NOT FOLLOW THIS FORMAT (L.R.)

		valid_flag = 0;
		if ((isalpha((int)s_sth.s_pc[0]))==0) valid_flag = 1;
		if ((isdigit((int)s_sth.s_pc[1]))==0) valid_flag = 1;
		if ((isalpha((int)s_sth.s_pc[2]))==0) valid_flag = 1;
		if ((isdigit((int)s_sth.s_pc[3]))==0) valid_flag = 1;
		if ((isalpha((int)s_sth.s_pc[4]))==0) valid_flag = 1;
		if ((isdigit((int)s_sth.s_pc[5]))==0) valid_flag = 1;

		if (valid_flag) {
#ifdef	ENGLISH
			fomen("Must be in LNLNLN Format");
#else
			fomen("Doit avoir le format LNLNLN");
#endif
			s_sth.s_pc[0] = LV_CHAR;
			sr.nextfld = PC_FLD;
			return (ERROR);
		}
**************/
		break;
	default :
#ifdef ENGLISH
		sprintf(e_mesg,"No Validity Check For Field#  %d",sr.curfld);
#else
		sprintf(e_mesg,"Pas de controle de validite pour le Champ#  %d",sr.curfld);
#endif
		fomen(e_mesg);
		get();
		break;
	}	/* Switch fld_no */

	return(NOERROR) ;
}	/* Validate() */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Display message and get the option */
GetOpt( msg, option )
       char *msg;
       char *option;
{
        int	j,  i;
	
        STRCPY( s_sth.s_mesg, msg );
        ShowMesg();

	for ( ; ; ) {
		s_sth.s_opt[0] = LV_CHAR ;
		if ( ReadFields(MESG_FLD, MESG_FLD) < 0 ) return(-1);
                j = strlen(option);
		for ( i = 0; i < j; i++ ) 
			if ( s_sth.s_opt[0] == option[i ] )
                           break;
                if(i != j) break ;
#ifdef  ENGLISH
                fomer( " Invalid Option." );
#else
                fomer( "Option invalide." );
#endif
	}
        s_sth.s_mesg[0] = HV_CHAR ;
        s_sth.s_opt[0] = HV_CHAR ;
	if ( WriteFields(MESG_FLD -100, MESG_FLD) < 0 ) return(-1);
 
        return( (int)(option[i])) ;
}  /* GetOpt */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Read the next record from data base */
BringNext(direction)
int	direction;
{
	int err;

	/* Check whether file is in seq. read mode */
	if(flg_start(SUPPLIER) != direction) {
		STRCPY( supplier.s_supp_cd, s_sth.s_supp_cd);
		inc_str( supplier.s_supp_cd, sizeof(supplier.s_supp_cd) - 1,
			 direction);
		flg_reset(SUPPLIER);
	}
	err = get_n_supplier(&supplier, BROWSE, 0, direction, e_mesg);

#ifndef ORACLE
	seq_over(SUPPLIER);
#endif

	if(ERROR == err)return(DBH_ERR) ;
	if(EFL == err) {
#ifdef	ENGLISH
		fomen("No More Records....");
#else
		fomen("Plus de fiches....");
#endif
		get();
		flg_reset(SUPPLIER);
		return(ERROR) ;
	}

	CopyToScreen() ;

	return(NOERROR);
}
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Copy the data record fields to Scrren1 & Screen 2 records and display
   the 1st scrren */
CopyToScreen()
{

	STRCPY( s_sth.s_supp_cd, supplier.s_supp_cd);
	STRCPY( s_sth.s_name, supplier.s_name);
	STRCPY( s_sth.s_category, supplier.s_category);
	STRCPY( s_sth.s_fax, supplier.s_fax);
	STRCPY( s_sth.s_abb, supplier.s_abb);
	STRCPY( s_sth.s_add1, supplier.s_add1);
	STRCPY( s_sth.s_add2, supplier.s_add2);
	STRCPY( s_sth.s_add3, supplier.s_add3);
	STRCPY( s_sth.s_pc, supplier.s_pc);
	STRCPY( s_sth.s_contact, supplier.s_contact);
	STRCPY( s_sth.s_phone, supplier.s_phone);
	if (supplier.s_tmp_flg[0] == YES) 
		s_sth.s_tmp_flg = 1 ;
	else
		s_sth.s_tmp_flg = 0 ;

	s_sth.s_type[0] = supplier.s_type[0] ;
	s_sth.s_type[1] = '\0' ;

	STRCPY( s_sth.s_payee, supplier.s_payee);
	STRCPY( s_sth.s_gst_reg, supplier.s_gst_reg);
	s_sth.s_discount = supplier.s_discount ;
	s_sth.s_min_dollar = supplier.s_min_dollar;
	s_sth.s_ytd_ord = supplier.s_ytd_ord ;
	s_sth.s_ytd_recpt = supplier.s_ytd_recpt ;
	s_sth.s_ytd_ret = supplier.s_ytd_ret ;
	s_sth.s_ytd_disc = supplier.s_ytd_disc ;
	s_sth.s_balance = supplier.s_balance ;

	if ( WriteFields(KEY_START, END_FLD) < 0 ) return(-1);

	return(NOERROR) ; 
}	/* CopyToScreen() */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Copy the Screen fields to data record */
CopyToRecord()
{

	STRCPY( supplier.s_supp_cd, s_sth.s_supp_cd);
	STRCPY( supplier.s_name, s_sth.s_name);
	STRCPY( supplier.s_category, s_sth.s_category);
	STRCPY( supplier.s_fax, s_sth.s_fax);
	STRCPY( supplier.s_abb, s_sth.s_abb);
	STRCPY( supplier.s_add1, s_sth.s_add1);
	STRCPY( supplier.s_add2, s_sth.s_add2);
	STRCPY( supplier.s_add3, s_sth.s_add3);
	STRCPY( supplier.s_pc, s_sth.s_pc);
	STRCPY( supplier.s_contact, s_sth.s_contact);
	STRCPY( supplier.s_phone, s_sth.s_phone);
	if (s_sth.s_tmp_flg)
		supplier.s_tmp_flg[0] = YES ;
	else
		supplier.s_tmp_flg[0] = NO ;

	supplier.s_type[0] = s_sth.s_type[0] ;
	STRCPY( supplier.s_payee, s_sth.s_payee);
	STRCPY( supplier.s_gst_reg, s_sth.s_gst_reg);
	supplier.s_discount = s_sth.s_discount ;
	supplier.s_min_dollar = s_sth.s_min_dollar ;
	supplier.s_ytd_ord = s_sth.s_ytd_ord ;
	supplier.s_ytd_recpt = s_sth.s_ytd_recpt ;
	supplier.s_ytd_ret = s_sth.s_ytd_ret ;
	supplier.s_ytd_disc = s_sth.s_ytd_disc ;
	supplier.s_balance = s_sth.s_balance ;
	supplier.s_last_actv = get_date() ;

}	/* Copy to Record */

/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Initialize 1st screen data fields with Low Values */
MoveLows()
{

	s_sth.s_name[0] = LV_CHAR ;
	s_sth.s_abb[0] = LV_CHAR ;
	s_sth.s_category[0] = LV_CHAR ;
	s_sth.s_add1[0] = LV_CHAR ;
	s_sth.s_add2[0] = LV_CHAR ;
	s_sth.s_add3[0] = LV_CHAR ;
	s_sth.s_pc[0] = LV_CHAR ;
	s_sth.s_fax[0] = LV_CHAR ;
	s_sth.s_contact[0] = LV_CHAR ;
	s_sth.s_phone[0] = LV_CHAR ;
	s_sth.s_tmp_flg = LV_INT ;
	s_sth.s_type[0] = LV_CHAR ;
	s_sth.s_payee[0] = LV_CHAR ;
	s_sth.s_gst_reg[0] = LV_CHAR ;
	s_sth.s_discount = LV_DOUBLE ;
	s_sth.s_min_dollar = LV_DOUBLE ;

	return(NOERROR);


}	/* MoveLows() */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Initialize 1st screen data fields with High values and display the screen */
MoveHighs()
{

	s_sth.s_name[0] = HV_CHAR ;
	s_sth.s_abb[0] = HV_CHAR ;
	s_sth.s_category[0] = HV_CHAR ;
	s_sth.s_add1[0] = HV_CHAR ;
	s_sth.s_add2[0] = HV_CHAR ;
	s_sth.s_add3[0] = HV_CHAR ;
	s_sth.s_pc[0] = HV_CHAR ;
	s_sth.s_fax[0] = HV_CHAR ;
	s_sth.s_contact[0] = HV_CHAR ;
	s_sth.s_phone[0] = HV_CHAR ;
	s_sth.s_tmp_flg = HV_INT ;
	s_sth.s_type[0] = HV_CHAR ;
	s_sth.s_payee[0] = HV_CHAR ;
	s_sth.s_gst_reg[0] = HV_CHAR ;
	s_sth.s_discount = HV_DOUBLE ;
	s_sth.s_min_dollar = HV_DOUBLE ;
	s_sth.s_ytd_ord = HV_DOUBLE ;
	s_sth.s_ytd_recpt = HV_DOUBLE ;
	s_sth.s_ytd_ret = HV_DOUBLE ;
	s_sth.s_ytd_disc = HV_DOUBLE ;
	s_sth.s_balance = HV_DOUBLE ;

	if ( WriteFields(ST_FLD, END_FLD) < 0 ) return(-1);

     	return(NOERROR);

}	/* MoveHighs() */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* show ERROR and wait 							 */
static
DispError(s)    
char *s;
{
        STRCPY( s_sth.s_mesg, s) ;
	ShowMesg();
	get();
	s_sth.s_mesg[0] = HV_CHAR;
	ShowMesg();
	return(ERROR);
}
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* show or clears message field 					 */
ShowMesg()  
{
	sr.nextfld = MESG_FLD - 100;
        fomwf((char*)&s_sth);
}
/*-------------------- E n d   O f   P r o g r a m ---------------------*/

