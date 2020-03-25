/*
*	Source 	: suppurge.c 
*
*	Program to Print Supplier Details using REPORT GENERATOR.
*
*/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	PROJNAME	"supprpt"
#define	LOG_REC		1
#define EXIT		12

#ifdef ENGLISH
#define PRINTER 'P'
#define DISPLAY 'D'
#define FILE_IO	'F'
#define YES	'Y'

#else
#define PRINTER 'I'
#define DISPLAY 'A'
#define FILE_IO	'D'
#define YES	'O'

#endif

static	Pa_rec	pa_rec ;
static  Supplier supp_rec;
static	Po_hdr	po_hdr ;
static 	Invoice	in_rec ;

extern	int	rperror	;
extern	char	e_mesg[80] ;

Suppurge()
{
	char	chardate[11] ;
	int	code;
	char 	*arayptr[5] ; 	/** Declarations for Report writer usage **/
	char 	projname[50] ;
	char 	program[11] ;
	int 	outcntl ;
	char 	discfile[20] ;
	int     formno;
	int	retval;
	char 	flag1[2],	flag2[2];
	long	parm_date;
	int	rd_md ;
	short 	copies ;

	formno = 5 ;

	code = get_param(&pa_rec, BROWSE, 1, e_mesg) ;
	if(code < 1) {
		close_dbh() ;
		return(code) ;
	}

#ifdef ENGLISH
	STRCPY(e_mesg,"P");
#else
	STRCPY(e_mesg,"I");
#endif
	retval = GetOutputon(e_mesg);
	if(retval < 0) return(retval);

	switch( *e_mesg){
		case DISPLAY:	/* display on terminal */
			outcntl = 0;
			break;
		case FILE_IO:	/* print to a file */
			outcntl = 1;
			break;
		case PRINTER:	/* print on printer */
			outcntl = 2;
			break;
		default:
			outcntl = 2;
			break;
	}
	if(outcntl == 1) {
		STRCPY(e_mesg,"suppurge.dat");
		if((retval = GetFilename(e_mesg)) < 0) return(retval);
		STRCPY(discfile,e_mesg);
	}
	else	discfile[0] = '\0';

	copies = 1;
	if(outcntl == 2) {
		if((retval = GetNbrCopies( &copies )) < 0)
			return(retval);
	}
	
#ifdef ENGLISH
	STRCPY(e_mesg,"Supplier Not used since date entered");
#else
	STRCPY(e_mesg,"Fournisseur pas utilise depuis que la date fut entree");
#endif
	fomen(e_mesg);
	parm_date = get_date();
	retval = GetDate( &parm_date);
	if(retval < 0) return(retval);

#ifdef ENGLISH
	retval = DisplayMessage("Only Temporary Suppliers (Y/N)?");
#else
	retval = DisplayMessage("Fournisseurs temporaires seulement (O/N)?");
#endif
	if(retval < 0) return(retval);
	retval = GetResponse(flag1);
	if(retval < 0) return(retval);

#ifdef ENGLISH
	retval = DisplayMessage("Purge Suppliers (Y/N)?");
#else
	retval = DisplayMessage("Eliminer fournisseurs (O/N)?");
#endif
	if(retval < 0) return(retval);

	for(;;) {
		retval = GetResponse(flag2);
		if(retval < 0) return(retval);
		if(flag2[0] == YES) {
			if(pa_rec.pa_cur_period == 0) {
#ifdef ENGLISH
			      fomen("Cannot Purge Supplier's Before Year End");
#else
			      fomen("Ne peut pas effacer les fournisseurs avant la fin d'annee");
#endif
			      continue;
			}
			retval = CheckAccess(SUPPLIER,P_DEL,e_mesg);
			if(retval == NOACCESS) {
				fomen(e_mesg);
				continue;
			}
		}
	    break;
	}

	if((retval = Confirm()) <= 0) return(retval);

	mkdate(get_date(),chardate);
	STRCPY(projname,FMT_PATH) ;
	strcat(projname,PROJNAME) ;
	STRCPY(program, PROG_NAME );

	code = rpopen(projname,LOG_REC,formno,outcntl,discfile,program,
			chardate);
							  	
	if ( code < 0 ){
		sprintf(e_mesg, "Rpopen code :%d\n", code ) ;
		close_dbh() ;
		return(REPORT_ERR);
	}

	if(outcntl == 2)
		rpSetCopies( (int)copies );   /* number of copies to print */ 

	/* Change first title line to Company/School district name */
	retval = rpChangetitle(1, pa_rec.pa_co_name);


	if (flag2[0] == YES) {
#ifdef ENGLISH
		retval = rpChangetitle(2, "DELETED SUPPLIERS REPORT");
#else
		retval = rpChangetitle(2, "RAPPORT DES FOURNISSEURS ELIMINES");
#endif
		rd_md = UPDATE ;
	}
	else {
#ifdef ENGLISH
		retval = rpChangetitle(2, "SUPPLIER ACTIVITY REPORT");
#else
		retval = rpChangetitle(2, "RAPPORT D'ACTIVITE DES FOURNISSEURS");
#endif
		rd_md = BROWSE ;
	}

	mkdate( parm_date, chardate);
#ifdef ENGLISH
	sprintf(e_mesg,"Date of Last Activity - %s", chardate);
#else
	sprintf(e_mesg,"Date de la derniere activite - %s", chardate);
#endif
	retval = rpChangetitle(3, e_mesg);
	if(retval < 0) {
#ifdef ENGLISH
		sprintf(e_mesg,"Internal report error");
#else
		sprintf(e_mesg,"Erreur au rapport interne");
#endif
		rpclose();
		close_dbh();
		return(REPORT_ERR);
	}

	 /* For Terminals set pagesize to 23 lines */
	if(outcntl == 0)
		rpPagesize(23);

	arayptr[0] = (char *)&supp_rec ;
	arayptr[1] = NULL ;

	/***	Read record and call report writer to output it ***/

	/* Initialize Rec. Entry Header Record to start reading form the
	   beginging */
	supp_rec.s_supp_cd[0] = '\0' ;
	flg_reset( SUPPLIER );

	for( ; ; ) {
		code = get_n_supplier(&supp_rec,rd_md,0,FORWARD,e_mesg);
		if( code < 0) {
			if(code == EFL) break ;
			code = DBH_ERR;
			break ;
		}

		if(supp_rec.s_last_actv > parm_date) { 	/* Active */
#ifndef	ORACLE
			roll_back(e_mesg) ;
#endif
			continue ;
		}

		if(flag1[0] == YES) {	/* Temporary only */
			if (supp_rec.s_tmp_flg[0] != YES) {
#ifndef	ORACLE
				roll_back(e_mesg) ;
#endif
				continue ;
			}
		}

		/* Check for Outstanding PO's */
		STRCPY(po_hdr.ph_supp_cd, supp_rec.s_supp_cd);
		po_hdr.ph_code = 0;
		flg_reset(POHDR) ;
		code = get_n_pohdr(&po_hdr, BROWSE, 1,FORWARD, e_mesg);
		if (code >= 0) 		/* Record Exists */
		   if ((strcmp(po_hdr.ph_supp_cd, supp_rec.s_supp_cd)) == 0) {  
#ifndef	ORACLE
			roll_back(e_mesg) ;
#endif
			continue ;
		   }

		/* Check for Outstanding INV */
		STRCPY(in_rec.in_supp_cd, supp_rec.s_supp_cd);
		in_rec.in_invc_no[0] = '\0' ;
		in_rec.in_tr_type[0] = '\0' ;
		flg_reset(APINVOICE) ;
		code = get_n_invc(&in_rec, BROWSE, 0, FORWARD, e_mesg);
		if (code >= 0)     		/* Record Exists */
		   if ((strcmp(in_rec.in_supp_cd, supp_rec.s_supp_cd)) == 0) {  
#ifndef	ORACLE
			roll_back(e_mesg) ;
#endif
			continue ;
		   }

		if((code = rpline(arayptr)) < 0) {
			if(rperror < 0) {
#ifdef ENGLISH
			sprintf(e_mesg,"Internal report error");
#else
			sprintf(e_mesg,"Erreur au rapport interne");
#endif
				code = REPORT_ERR;
			}
			else
				code = NOERROR;
			break ;
		}

		if(flag2[0] == YES) {		/* Purge Supplier */
			code = put_supplier(&supp_rec,P_DEL,e_mesg);
			if (code < 0) {
				code = DBH_ERR;
				roll_back(e_mesg) ;
				break;
			}
			if (commit(e_mesg) < 0) {
				code = DBH_ERR;
				break;
			}
			inc_str(supp_rec.s_supp_cd, 
				sizeof(supp_rec.s_supp_cd) -1, FORWARD);
		}
	}

	close_dbh() ;
	rpclose() ;
	if(code == EFL) return(0);
	return(code);
}
