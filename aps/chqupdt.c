/*-------------------------------------------------------------------------
Source Name: chqupdt.c
System     : Accounts Payables.
Created  On: 10th December 89.
Created  By: CATHY BURNS.

DESCRIPTION:
	Program to update history records.

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
------------------------------------------------------------------------*/

#include <stdio.h>
#include <reports.h>

#define FORMNO		1
#define LOGREC		1
#define PROJECT		"chqpurge"

static	Chq_rec		chq_rec ;	/* Cheque record */
static	Invoice		invoice ;
static	Pa_rec		pa_rec ;

static	char 	e_mesg[100];	/* dbh will return err msg in this */
static	long	parm_date ;
static	char 	*arayptr[5] ; 	/** Declarations for Report writer usage **/

extern	int	rperror ;

ChequeUpdate(fund, accno)
int	fund ;
char	*accno ;
{
int 	retval ;

	if (InitReport() < 0) return(ERROR) ;

	chq_rec.c_funds = fund ;
	chq_rec.c_accno[0] = '\0' ;
	chq_rec.c_chq_no = 0 ;
	chq_rec.c_supp_cd[0] = '\0' ;
	chq_rec.c_invc_no[0] = '\0' ;
	chq_rec.c_tr_type[0] = '\0' ;
	
	flg_reset(CHEQUE) ;

	for( ; ; ) {
		retval = get_n_chq(&chq_rec,UPDATE,1,FORWARD,e_mesg) ;
		if (retval == EFL) break ;
		if (retval < 0) {
			DispError(e_mesg) ;
			roll_back(e_mesg) ;
			return(retval) ;
		}
		if (chq_rec.c_funds != fund) {
			roll_back(e_mesg) ;
			break ;
		}
		/* Manual or Cancelled Cheques comes at the end with in fund.
		   Regular cheques will be having chq_no & accno as 0s. So
		   if the bank account is smaller than current account skip the
		   records or if greater, then no more records to process */
		if(chq_rec.c_chq_no != 0) {
			retval = strcmp(chq_rec.c_accno, accno) ;
			if(retval < 0) {
				STRCPY(chq_rec.c_accno, accno) ;
				chq_rec.c_chq_no = 0 ;
				chq_rec.c_supp_cd[0] = '\0' ;
				chq_rec.c_invc_no[0] = '\0' ;
				chq_rec.c_tr_type[0] = '\0' ;
				flg_reset(CHEQUE) ;
				continue ;
			}
			if(retval > 0) {
				roll_back(e_mesg) ;
				break;
			}
		}

		invoice.in_funds = chq_rec.c_funds ;
		STRCPY( invoice.in_supp_cd, chq_rec.c_supp_cd ) ;
		STRCPY( invoice.in_invc_no, chq_rec.c_invc_no ) ;
		STRCPY( invoice.in_tr_type, chq_rec.c_tr_type ) ;

		retval = get_invc(&invoice,UPDATE,1,e_mesg) ;
		if (retval < 0 && retval != UNDEF) {
			DispError(e_mesg) ;
			roll_back(e_mesg) ;
			return(retval) ;
		}

		/* UNDEF comes when, If there is one manual cheque and one
		   regular cheque in the same process, by that time it gets
		   to the manual cheque record, it might have been deleted */

		if( retval != UNDEF && invoice.in_pmtcode[0] == COMPLETE) {
								/* Flag as */
			retval = rpline(arayptr) ;
			if ( retval < 0 ){
#ifdef ENGLISH
		 	  sprintf(e_mesg,"REPORT-WRITER ERROR...rpline() rperror: %d", rperror) ;
#else
		 	  sprintf(e_mesg,"Erreur du REPORT-WRITER...rpline() rperror: %d", rperror) ;
#endif
		  	  DispError(e_mesg) ;
			  roll_back(e_mesg) ;
		  	  break ;
			}
			retval = DeleteInvoice() ;
			if (retval < 0) {
				DispError(e_mesg) ;
				roll_back(e_mesg) ;
				return(retval) ;
			}
		}
		retval = DeleteCheque();	/* Once Cheque printed */
						/* need to be deleted */
		if (retval < 0) {
			DispError(e_mesg) ;
			roll_back(e_mesg) ;
			return(retval) ;
		}
		if(commit(e_mesg) < 0) {
#ifdef	ENGLISH
			DispError("ERROR in Saving Records"); 
#else
			DispError("ERREUR en conservant les fiches");
#endif
			return(DBH_ERR);
		}
		inc_str(chq_rec.c_tr_type, (sizeof(chq_rec.c_tr_type) - 1),
			FORWARD) ;

	} /* end of for */

	seq_over(CHEQUE) ;
		
	unlink_file(CHQREG) ;

#ifndef ORACLE
	fomcs();
	fflush(stdout) ;
	ixrecreat(APINVOICE) ;
	ixrecreat(CHEQUE) ;
#endif
	rpclose() ;
	close_dbh() ;
	return(NOERROR) ;
}
/*---------------------------------------------------------------------------*/
/* Initialize report generator and check parameter file 		     */
InitReport()
{
	char	chardate[11];
	char	projname[50] ;
	int	code;

	parm_date = get_date() ;

	/*
	*	Get The Parameter Record
	*/
	code = get_param(&pa_rec, BROWSE, 1, e_mesg) ;
	if(code == ERROR) {
		DispError(e_mesg);
		return(ERROR) ;
	}
	else if(code == UNDEF) {
#ifdef ENGLISH
		DispError("Parameters Are Not Setup..");
#else
		DispError("Parametres ne sont pas etablis..");
#endif
		return(ERROR) ;
	}
	
	/* Form the project name with full path */
	STRCPY(projname,FMT_PATH) ;
	strcat(projname,PROJECT) ;

	mkdate(get_date(),chardate);

	code = rpopen(projname,LOGREC,FORMNO,2,"\0",PROG_NAME,chardate);
	if ( code < 0 ){
#ifdef ENGLISH
		sprintf(e_mesg,"REPORT-WRITER ERROR... rpopen()   rperror: %d",
#else
		sprintf(e_mesg,"Erreur du REPORT-WRITER... rpopen()   rperror: %d",
#endif
			rperror ) ;
		DispError(e_mesg) ;
		return(ERROR) ;
	}

	/* Change first title line to Company/School district name */
	rpChangetitle(1, pa_rec.pa_co_name);

	arayptr[0] = (char*)&invoice ;
	arayptr[1] = NULL ;
		
	return(NOERROR) ;

}	/* InitReport() */
/*---------------------------------------------------------------------------*/
/*  Remove invoice from file once it is fully paid.            	     */
DeleteInvoice()
{
	return(put_invc(&invoice,P_DEL,e_mesg) ) ;
}
/*---------------------------------------------------------------------------*/
/*  Once cheque is fully processed then it must be deleted from file.*/
DeleteCheque()
{
	return(put_chq(&chq_rec,P_DEL,e_mesg) ) ;
}
