/*
*	Source 	: supprpt.c 
*
*	Program to Print Supplier Details using REPORT GENERATOR.
*
*/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	PROJNAME	"supprpt"
#define	LOG_REC		1
#define ABB_NAME	2
#define EXIT		12

#ifdef ENGLISH
#define PRINTER 'P'
#define DISPLAY 'D'
#define FILE_IO	'F'
#define SPOOL	'S'	/* added for requisitions */
#define YES	'Y'
#define NO	'N'

#define TEMPORARY	'T'
#define PERMENANT	'P'
#define BOTH		'B'
#else
#define PRINTER 'I'
#define DISPLAY 'A'
#define FILE_IO	'D'
#define SPOOL	'B'	/* added for requisitions */
#define YES	'O'
#define NO	'N'

#define TEMPORARY	'T'
#define PERMENANT	'P'
#define BOTH		'E'
#endif

static	Pa_rec	pa_rec ;
static  Supplier supp_rec;
/*  static	char	show[80];   */

extern	int	rperror ;
extern	char	e_mesg[80] ;

Supprpt(mode)
int mode;
{
	char	chardate[11] ;
	int	code;
	char 	*arayptr[5] ; 	/** Declarations for Report writer usage **/
	char 	projname[50] ;
	char 	program[11] ;
	int 	outcntl ;
	char 	discfile[20] ;
	int     keyno;
	int     formno;
	char	summary[2], suppflg[2];
	char	suppname1[25], suppname2[25];
	char	suppno1[11], suppno2[11];
	int	retval;
	short	copies ;

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
		case SPOOL:	/* spool report */
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
		if(e_mesg[0] == FILE_IO) {
			STRCPY(e_mesg,"supprpt.dat");
			if((retval = GetFilename(e_mesg)) < 0)
				return(retval);
			STRCPY(discfile,e_mesg);
		}
		else {
			sprintf(discfile,"spool%d",CC_no);
		}
	}
	else	discfile[0] = '\0';

	copies = 1;
	if(outcntl == 2) {
		if((retval = GetNbrCopies( &copies )) < 0)
			return(retval);
	}

	if(mode == ABB_NAME){
		STRCPY(suppname1,"                        ");
		STRCPY(suppname2,"zzzzzzzzzzzzzzzzzzzzzzzz");
		retval = GetSNameRange( suppname1, suppname2 );
		if(retval < 0) return(retval);
		keyno = 2;
	}
	else {
		STRCPY(suppno1,"         0");
		STRCPY(suppno2,"ZZZZZZZZZZ");
		retval = GetSuppRange( suppno1, suppno2 );
		if(retval < 0) return(retval);
		keyno = 0;
	}

#ifdef ENGLISH
	retval = DisplayMessage("Do You Want Only Temporary/Permanent Suppliers or Both (T/P/B)?");
#else
	retval = DisplayMessage("Voulez-vous les fournisseurs en entier, temporaires, ou permanents (E/T/P)?");
#endif
	if(retval < 0) return(retval);
	if((retval = GetResponse(suppflg)) <0) return(retval);

#ifdef ENGLISH
	retval = DisplayMessage("Summary (Y/N)?");
#else
	retval = DisplayMessage("Resume (O/N)?");
#endif
	if(retval < 0) return(retval);
	if((retval = GetResponse(summary)) <0) return(retval);
	if(summary[0] == YES) {
		if(mode == ABB_NAME) {
			formno = 4;
		}
		else formno = 2;
	}
	else {
		if(mode == ABB_NAME) {
			formno = 3;
		}
		else formno = 1;
	}
	if((retval = Confirm()) <= 0) 
		return(retval);

	code = get_param(&pa_rec, BROWSE, 1, e_mesg) ;
	if(code < 1) {
		close_dbh() ;
		return(DBH_ERR) ;
	}

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

	/* For Terminals set pagesize to 23 lines */
	if(outcntl == 0)
		rpPagesize(23);

	/* Change first title line to Company/School district name */
	retval = rpChangetitle(1, pa_rec.pa_co_name);
	retval = rpChangetitle(3, " ");
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

	arayptr[0] = (char *)&supp_rec ;
	arayptr[1] = NULL ;

	/***	Read record and call report writer to output it ***/

	/* Initialize Rec. Entry Header Record to start reading from the
	   begining */
	if (keyno == 0)
		STRCPY(supp_rec.s_supp_cd,suppno1);
	else
		STRCPY(supp_rec.s_abb,suppname1);

	flg_reset( SUPPLIER );

	for( ; ; ) {
		code = get_n_supplier(&supp_rec,BROWSE,keyno,FORWARD,e_mesg);
		if( code < 0) {
			if(code == EFL) break ;
			code = DBH_ERR;
			break ;
		}
		if(mode == ABB_NAME) {
			if(strcmp(supp_rec.s_abb,suppname2) > 0) break;
		}
		else {
			if(strcmp(supp_rec.s_supp_cd,suppno2) > 0) break;
		}
		if(suppflg[0] != BOTH) {	/* Not all Suppliers */
			if(suppflg[0] == TEMPORARY) { /* Temporary suppliers */
				if(supp_rec.s_tmp_flg[0] != YES)
					continue;
			}
			if(suppflg[0] == PERMENANT) { /* Permenant suppliers */
				if(supp_rec.s_tmp_flg[0] != NO)
					continue;
			}
		}
		if(summary[0] == YES && supp_rec.s_phone[0] != '\0') {
			strncpy(supp_rec.s_category,supp_rec.s_phone,3);
			supp_rec.s_category[3] = '\0';
			strncat(supp_rec.s_category,"-",1);
			strncat(supp_rec.s_category,supp_rec.s_phone+3,3);
			strncat(supp_rec.s_category,"-",1);
			strncat(supp_rec.s_category,supp_rec.s_phone+6,4);
		}
		else if(summary[0] == YES) {
			supp_rec.s_category[0] = '\0';
		}
		if (mode == ABB_NAME)
		 	STRCPY(supp_rec.s_name,supp_rec.s_abb) ;

		if((code = rpline(arayptr)) < 0){
			if(rperror <  0){
#ifdef ENGLISH
			sprintf(e_mesg,"Internal report error");
#else
			sprintf(e_mesg,"Erreur au rapport interne");
#endif
				code = REPORT_ERR;
			}
			else
				code = NOERROR ;
			break ;
		}
	}

	rpclose() ;
	close_dbh() ;
	if(code == EFL) return(0);
	return(code);
}
