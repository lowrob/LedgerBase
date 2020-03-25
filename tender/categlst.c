/*
*	Source 	: supprpt.c 
*
*	Program to Print Supplier Details using REPORT GENERATOR.
*
*/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	PROJNAME	"categlst"
#define	LOG_REC		1
#define EXIT		12

#ifdef ENGLISH
#define PRINTER 'P'
#define DISPLAY 'D'
#define FILE_IO	'F'
#define SPOOL	'S'	/* added for requisitions */
#define YES	'Y'
#define NO	'N'

#else
#define PRINTER 'I'
#define DISPLAY 'A'
#define FILE_IO	'D'
#define SPOOL	'B'	/* added for requisitions */
#define YES	'O'
#define NO	'N'

#endif

static	Pa_rec	pa_rec ;
static  Category categ_rec;
/*  static	char	show[80];   */

extern	int	rperror ;
extern	char	e_mesg[80] ;

CategoryList()
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
	short	categno1, categno2;
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
			STRCPY(e_mesg,"categlst.dat");
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

	retval = GetCategRange( &categno1, &categno2 );
	if(retval < 0) return(retval);

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

	code = rpopen(projname,LOG_REC,1,outcntl,discfile,program,chardate);
							  	
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

	arayptr[0] = (char *)&categ_rec ;
	arayptr[1] = NULL ;

	/***	Read record and call report writer to output it ***/

	categ_rec.categ_num = categno1;

	flg_reset( CATEGORY );

	for( ; ; ) {
		code = get_n_category(&categ_rec,BROWSE,0,FORWARD,e_mesg);
		if( code < 0) {
			if(code == EFL) break ;
			code = DBH_ERR;
			break ;
		}
		if(categ_rec.categ_num > categno2) {
			break;
		}

		if((code = rpline(arayptr)) < 0){
			if(rperror <  0){
#ifdef ENGLISH
			sprintf(e_mesg,"Internal report error");
#else
			sprintf(e_mesg,"Erreur au rapport interne");
#endif
				code = REPORT_ERR;
			}
			else{
				code = EXIT;/* code = NOERROR ; */
				break ;
			}
		}	
	}

	rpclose() ;
	close_dbh() ;
	if(code == EFL) return(0);
	return(code);
}
