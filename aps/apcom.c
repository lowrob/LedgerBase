/*-----------------------------------------------------------------------
Source Name: apcom.c
System     : Accounts Payables.
Created  On: 23rd November 89.
Created  By: T AMARENDRA.

DESCRIPTION:
	This file maintains common functions for Invoice entry progarm.

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

------------------------------------------------------------------------*/

#define	MOD_DATE	"30-NOV-89"		/* Program Last Modified */

#include <stdio.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_recs.h>
#include <apinvc.h>

struct	stat_rec	sr ;

/*------------------------------------------------------------*/
/* Read the PROFOM Screen for a given Range of fields */

ReadFields(st_fld, end_fld, Validate, WindowHelp, mode, esc_f)
int	st_fld ;
int	end_fld ;
int	(*Validate)() ;
int	(*WindowHelp)() ;
int	mode ;		/* ADD or UPDATE */
int	esc_f ;		/* Is ESC_F active? */
{
	int	err ;

	sr.nextfld = st_fld ;

	for( ; ;){
		sr.endfld  = end_fld ;

		fomrd( CurrentScr );
		ret(err_chk(&sr));
		if(sr.retcode == RET_VAL_CHK){
			err = (*Validate)(mode) ;
			if(DBH_ERR == err || PROFOM_ERR == err ||
			   DUPEREF == err) return(err);
			sr.nextfld = sr.curfld ;
			continue;
		}
		if(sr.retcode == RET_USER_ESC){
			if(esc_f &&
			    (sr.escchar[0] == 'f' || sr.escchar[0] == 'F') )
				return(RET_USER_ESC) ;
			if(sr.escchar[0] == 'h' || sr.escchar[0] == 'H'){
				err = (*WindowHelp)() ;
				if(DBH_ERR == err || PROFOM_ERR == err ||
				   DUPEREF == err)
					return(err) ;
			}
			sr.nextfld = sr.curfld ;
			continue;
		}
		/* else RET_NO_ERROR */
		break;
	}

	return(NOERROR) ;
}	/* ReadFields() */
/*------------------------------------------------------------------------*/
/* Write fields on Screen for a given Range */

WriteFields(st_fld, end_fld)
int	st_fld ;
int	end_fld ;
{
	sr.nextfld = st_fld ;
	sr.endfld  = end_fld ;

	fomwr( CurrentScr ) ;
	ret(err_chk(&sr));

	return(NOERROR) ;
}	/* WriteFields() */
/*-----------------------------------------------------------------------*/
/* Display message and get the option */

GetOption(msg,options)
char	*msg ;
char	*options ;
{
	int	i, j ;
	int	k, l ;

	fomfp(END_FLD-100, &k, &i) ;
	ret(err_chk(&sr)) ;

	fomfp(END_FLD, &l, &j) ;
	ret(err_chk(&sr)) ;

	strncpy((CurrentScr+k),msg,i);
	ShowMesg() ;

	sr.nextfld = END_FLD ;
	for( ; ; ) {
		fomrf( CurrentScr ) ;
		ret(err_chk(&sr)) ;

		j = strlen(options) ;
		for( i = 0 ; i < j ; i++)
			if((CurrentScr+l)[0] == options[i]) break ;
		if(i != j) break ;	/* Valid Option Selected */
#ifdef ENGLISH
		fomer("Invalid Option..");
#else
		fomer("Option invalide..");
#endif
	}
	(CurrentScr+k)[0] = HV_CHAR;
	(CurrentScr+l)[0] = HV_CHAR;

	ret( WriteFields((END_FLD - 100), END_FLD) );

	return((int)(options[i])) ;
}	/* GetOption() */
/*-------------------------------------------------------------------------*/
/* Copy the given message to SCR message line, display it and seek
   user response */

DispError(s)    /* show ERROR and wait */
char	*s;
{
	int	i, j ;

	fomfp(END_FLD-100, &i, &j) ;
	ret(err_chk(&sr)) ;

	strncpy((CurrentScr+i), s, j);
	ShowMesg();
#ifdef ENGLISH
	fomen("Press any key to continue");
#else
	fomen("Appuyer sur une touche pour continuer");
#endif
	get();
	(CurrentScr+i)[0] = HV_CHAR;
	ShowMesg();

	return(NOERROR);
}	/* DispError() */
/*-------------------------------------------------------------------------*/
/* Write the Message Field */

ShowMesg()  /* shows or clears message field */
{
	sr.nextfld = END_FLD - 100;
	fomwf( CurrentScr ) ;

	return(NOERROR) ;
}	/* ShowMesg() */
/*----------------------------------------------------------------*/
/* Check given Transaction type */

CheckTransType(tr_type)
char	*tr_type ;
{
	if( strcmp(tr_type,T_INVOICE) == 0) return(INVOICE) ;
	if( strcmp(tr_type,T_RETURN)  == 0) return(RETURN) ;
	if( strcmp(tr_type,T_CRMEMO)  == 0) return(CRMEMO) ;
	if( strcmp(tr_type,T_DBMEMO)  == 0) return(DBMEMO) ;

#ifdef ENGLISH
	sprintf(e_mesg,
"Valid Types are %s(Invoice), %s(Return), %s(Credit Memo) and %s(Debit Memo)",
T_INVOICE, T_RETURN, T_CRMEMO, T_DBMEMO) ;
#else
	sprintf(e_mesg,
"Genre valables sont %s(Facture),%s(Renvoi),%s(Note credit),%s(Note debit)",
T_INVOICE, T_RETURN, T_CRMEMO, T_DBMEMO) ;
#endif
	fomer(e_mesg) ;

	return(ERROR) ;
}	/* CheckTransType() */
/*-----------------------------------------------------------------------*/ 
/* Copy the key fields to Invoice record and get the record from
   data base */

GetInvoice(supp_cd, invc_no, tr_type, md)
char	*supp_cd ;
char	*invc_no ;
char	*tr_type ;	/* Type IN, CM, CM & RT */
int	md;		/* BROWSE or UPDATE */
{
	STRCPY(in_rec.in_supp_cd, supp_cd) ;
	STRCPY(in_rec.in_invc_no, invc_no) ;
	STRCPY(in_rec.in_tr_type, tr_type) ;

	return(get_invc(&in_rec, md, 0, e_mesg));
}	/* GetInvoice() */
/*----------------------------------------------------------------*/
/* Check the given stock codes availability in file */

CheckSupp(supp_cd,mode)
char	*supp_cd ;
int	mode ;
{
	int	err ;
	Right_Justify_Numeric(supp_cd,sizeof(supp_rec.s_supp_cd)-1);
	STRCPY(supp_rec.s_supp_cd, supp_cd) ;
	err = get_supplier(&supp_rec, mode, 0, e_mesg) ;
	if(ERROR == err) return(DBH_ERR) ;
	if(err < 0) {
		fomer(e_mesg) ;
		return(err) ;
	}

	return(NOERROR) ;
}	/* CheckSupp() */
/*----------------------------------------------------------------*/
/* Check the given stock codes availability in file */

CheckPayee(payee_cd,mode)
char	*payee_cd ;
int	mode ;
{
	int	err ;
	Right_Justify_Numeric(payee_cd,sizeof(payee_rec.s_supp_cd)-1);
	STRCPY(payee_rec.s_supp_cd, payee_cd) ;
	err = get_supplier(&payee_rec, mode, 0, e_mesg) ;
	if(ERROR == err) return(DBH_ERR) ;
	if(err < 0) {
		fomer(e_mesg) ;
		return(err) ;
	}

	return(NOERROR) ;
}	/* CheckPayee() */
/*----------------------------------------------------------------*/
/* Check the given funds availability in file */

CheckFund(fund)
short	fund ;
{
	int	err ;

	ctl_rec.fund = fund ;

	err = get_ctl(&ctl_rec, BROWSE, 0, e_mesg) ;
	if(ERROR == err) return(DBH_ERR);
	if(err < 0) {
		fomer(e_mesg) ;
		return(ERROR) ;
	}
	return(NOERROR) ;
}	/* CheckFund() */
/*----------------------------------------------------------------*/
/* Read the given account in given mode */

CheckGlAcnt(fund, accno,reccod,mode)
short	fund ;
char	*accno ;
int	reccod ;
int	mode ;
{
	int	err ;

	gl_rec.funds = fund ;
	STRCPY(gl_rec.accno, accno) ;
	gl_rec.reccod = reccod ;

	err = get_gl(&gl_rec, mode, 0, e_mesg) ;
	if(ERROR == err) return(DBH_ERR);
	if(err == LOCKED) return(LOCKED) ;
	if(err < 0) {
		fomer(e_mesg) ;
		return(ERROR) ;
	}
	return(NOERROR) ;
}	/* CheckGlAcnt() */
/*---------------------------------------------------------------------*/
/* Make Unique Invoice Hdr key */

UniqueHdrKey(batch)
int	batch ;
{
	char	tnum[5] ;
	int	err ;
#ifdef	ORACLE
	long	get_maxsno(), sno ;
#endif

	/* generate the Unique Invc Hdr key */
	get_tnum(tnum) ;			/* get the terminal# */
	STRCPY(in_hdr.h_term, tnum) ;
	in_hdr.h_batch = batch ;
#ifndef	ORACLE
	in_hdr.h_sno = HV_SHORT ;

	flg_reset( APINHDR ) ;

	err = get_n_inhdr(&in_hdr, BROWSE, 0, BACKWARD, e_mesg) ;
	seq_over( APINHDR );
	if(err < 0 && err != EFL) return(DBH_ERR) ;
	/* If EFL or Key Cahnged */
	if(err == EFL || strcmp(tnum, in_hdr.h_term ) ||
				in_hdr.h_batch != batch) {
		STRCPY(in_hdr.h_term, tnum) ;
		in_hdr.h_batch = batch ;
		in_hdr.h_sno = 1 ;
	}
	else
		in_hdr.h_sno++ ;
#else
	sno = get_maxsno(APINHDR, (char*)&in_hdr, 0, -1, e_mesg);
	if(sno < 0) return(DBH_ERR) ;
	in_hdr.h_sno = sno + 1 ;
#endif

	return(NOERROR) ;
}	/* UniqueHdrKey() */
/*-----------------------------------------------------------*/
/* Move Invoice Fields to Invoice Header */

InitInvcHdr()
{
	STRCPY(in_hdr.h_supp_cd, in_rec.in_supp_cd) ;
	STRCPY(in_hdr.h_invc_no, in_rec.in_invc_no) ;
	STRCPY(in_hdr.h_tr_type, in_rec.in_tr_type) ;

	in_hdr.h_type[0]  = in_rec.in_type[0] ;
	in_hdr.h_pmtcode[0] = in_rec.in_pmtcode[0] ;
	STRCPY(in_hdr.h_accno, in_rec.in_accno) ;
	STRCPY(in_hdr.h_remarks, in_rec.in_remarks) ;
	in_hdr.h_funds    = in_rec.in_funds ;
	in_hdr.h_period   = in_rec.in_period ;
	in_hdr.h_po_no    = in_rec.in_po_no ;
	in_hdr.h_invc_dt  = in_rec.in_invc_dt ;
	in_hdr.h_due_dt   = in_rec.in_due_dt ;
	in_hdr.h_chq_no   = in_rec.in_chq_no ;
	in_hdr.h_disc_per = in_rec.in_disc_per ;
	in_hdr.h_disc_amt = in_rec.in_disc_amt ;
	in_hdr.h_amount   = in_rec.in_amount ;
	in_hdr.h_gsttax   = in_rec.in_gsttax ;
	in_hdr.h_psttax   = in_rec.in_psttax ;
	in_hdr.h_part_amt = in_rec.in_part_amt ;

	return(NOERROR) ;
}	/* InitInvcHdr() */
/*-------------------- E n d   O f   P r o g r a m ---------------------*/

