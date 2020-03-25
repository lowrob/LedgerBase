/*
*	Source 	: custrpt.c 
*
*	Program to Print customer Details using REPORT GENERATOR.
*
*/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	PROJNAME	"custrpt"
#define	LOG_REC		1
#define ABB_NAME	1
#define EXIT		12

#ifdef ENGLISH
#define PRINTER 	'P'
#define DISPLAY		'D'
#define FILE_IO		'F'

#define YES		'Y'
#else
#define PRINTER		'I'
#define DISPLAY		'A'
#define FILE_IO		'D'

#define YES		'O'
#endif

static	Pa_rec	pa_rec ;
static  Cu_rec cu_rec;

extern	char	e_mesg[80] ;

CustPurge()
{
	char	chardate[11] ;
	int	code;
	char 	*arayptr[5] ; 	/** Declarations for Report writer usage **/
	char 	projname[50] ;
	char 	program[11] ;
	int 	outcntl ;
	char 	discfile[20] ;
	char	answer[2];
	int	retval;
	int	mode;
	long	cutoffdt;	/* deletion cutoff date */
	short	copies;

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
		STRCPY(e_mesg,"custpurg.dat");
		if((retval = GetFilename(e_mesg)) < 0) return(retval);
		STRCPY(discfile,e_mesg);
	}
	else	discfile[0] = '\0';

	copies = 1;
	if(outcntl == 2) {
		if((retval = GetNbrCopies(&copies)) < 0)
			return(retval);
	}
	cutoffdt = get_date();

	code = GetDate(&cutoffdt); 
	if(code < 0)	return(code);

#ifdef ENGLISH
	code = DisplayMessage("Delete customers with Zero balance (Y/N)?");
#else
	code = DisplayMessage("Eliminer les clients avec un solde de zero (O/N)?");
#endif
	if(code < 0)	return(code);
	for( ; ; ) {
		code = GetResponse(answer,"YN");
		if(code < 0)	return(code);
		if(answer[0] == YES) {
			if(pa_rec.pa_cur_period == 0) {
#ifdef ENGLISH
			    fomen("Cannot Purge Customers Before Year End");
#else
			    fomen("Ne peut pas eliminer les clients avant la fin d'annee");
#endif
			    continue;
			}	
			code = CheckAccess(CUSTOMER,P_DEL,e_mesg);	
			if(code == NOACCESS) {
			    fomen(e_mesg);
			    continue;
			}
		}
		break ;
	}
	
	if((retval = Confirm()) <= 0)
		 return(retval);

	code = get_param(&pa_rec, BROWSE, 1, e_mesg) ;
	if(code < 1) {
		return(DBH_ERR) ;
	}

	mkdate(get_date(),chardate);
	STRCPY(projname,FMT_PATH) ;
	strcat(projname,PROJNAME) ;
	STRCPY(program, PROG_NAME );

	code = rpopen(projname,LOG_REC,5,outcntl,discfile,program,
			chardate);
							  	
	if ( code < 0 ){
		sprintf(e_mesg, "Rpopen code :%d\n", code ) ;
		close_dbh() ;
		return(REPORT_ERR);
	}

	rpSetCopies((int)copies);

	/* Change first title line to Company/School district name */
	if((retval = rpChangetitle(1, pa_rec.pa_co_name))<0) {
#ifdef ENGLISH
		sprintf(e_mesg,"Internal report error");
#else
		sprintf(e_mesg,"Erreur au rapport interne");
#endif
		rpclose();
		close_dbh();
		return(REPORT_ERR);
	}
	mkdate( cutoffdt, chardate);
#ifdef ENGLISH
	sprintf(e_mesg,"DATE OF LAST ACTIVITY - %s", chardate); 
#else
	sprintf(e_mesg,"DATE DE LA DERNIERE ACTIVITE - %s", chardate); 
#endif
	if((retval = rpChangetitle(3, e_mesg))<0) {
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

	arayptr[0] = (char *)&cu_rec ;
	arayptr[1] = NULL ;

	/***	Read record and call report writer to output it ***/

	/* Initialize Rec. Entry Header Record to start reading form the
	   beginging */

	if(answer[0] == YES)
		mode = UPDATE;
	else	mode = BROWSE;

	cu_rec.cu_code[0] = '\0';
	flg_reset( CUSTOMER );

	for( ; ; ) {
		code = get_n_cust(&cu_rec,mode,0,FORWARD,e_mesg);
		if( code < 0) {
			if(code == EFL) break ;
			code = DBH_ERR;
			break ;
		}
		if(cu_rec.cu_rcpt_dt > cu_rec.cu_sale_dt) {
			cu_rec.cu_sale_dt = cu_rec.cu_rcpt_dt;
		}
		if(cu_rec.cu_sale_dt < cutoffdt && cu_rec.cu_cur_bal == 0) {
			if(answer[0] == YES) {
				code = put_cust(&cu_rec,P_DEL,e_mesg);
				if(code < 0) {
					roll_back(e_mesg);
					code = DBH_ERR;
					break;
				}	
				if(commit(e_mesg) < 0) {
					code = DBH_ERR;
					roll_back(e_mesg);
					break;
				}
			}
		}
		else {
#ifndef	ORACLE
			roll_back(e_mesg);
#endif
			continue;
		}
		if((retval =  rpline(arayptr)) < 0) {
#ifdef ENGLISH
			sprintf(e_mesg,"Internal report error");
#else
			sprintf(e_mesg,"Erreur au rapport interne");
#endif
			retval = REPORT_ERR;
			break ;
		}
	}

	close_dbh() ;
	rpclose() ;
	if(retval == EFL) return(0);
	return(retval);
}
