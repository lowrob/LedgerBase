/*
*	Source 	: farpt.c 
*
*	Program to Print List of OBSOLETE Fixed Assets
*	by original cost centers.
*/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>
#include <bfs_fa.h>

#define	PROJNAME	"farpt"
#define	LOG_REC		1
#define FORMNO		1
#define EXIT		12

#ifdef ENGLISH
#define PRINTER 'P'
#define DISPLAY 'D'
#define FILE_IO	'F'
#define YES	'Y'
#define NO 	'N'
#else
#define PRINTER 'I'
#define DISPLAY 'A'
#define FILE_IO	'D'
#define YES	'O'
#define NO 	'N'
#endif

static	Pa_rec	pa_rec ;
static	Fa_rec	fa_rec ;
extern	char	e_mesg[80] ;

farpt()
{
	char	chardate[11] ;
	int	code;
	char 	*arayptr[5] ; 	/** Declarations for Report writer usage **/
	char 	projname[50] ;
	char 	program[11] ;
	int 	outcntl ;
	char 	discfile[20] ;
	int     keyno;
	int	retval;
	short	copies ;
	short	cc_nbr1,cc_nbr2;

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
		STRCPY(e_mesg,"farpt.dat");
		if((retval = GetFilename(e_mesg)) < 0)
			return(retval);
		STRCPY(discfile,e_mesg);
	}
	else	discfile[0] = '\0';

	copies = 1;
	if(outcntl == 2) {
		if((retval = GetNbrCopies( &copies )) < 0)
			return(retval);
	}


	cc_nbr1 = 1;
	cc_nbr2 = 9999;
	retval = GetCostcenRange(&cc_nbr1,&cc_nbr2);
	if (retval < 0) return(retval);

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

	code = rpopen(projname,LOG_REC,FORMNO,outcntl,discfile,program,
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

	arayptr[0] = (char *)&fa_rec ;
	arayptr[1] = NULL ;

	/***	Read record and call report writer to output it ***/

	/* Initialize Rec. Entry Header Record to start reading form the
	   beginging */

 	fa_rec.fa_costcen = cc_nbr1;
	flg_reset( FAMAST );

	for( ; ; ) {
		code = get_n_famast(&fa_rec,BROWSE,0,FORWARD,e_mesg);
		if( code < 0) {
			if(code == EFL) break ;
			code = DBH_ERR;
			break ;
		}
		if (fa_rec.fa_costcen > cc_nbr2 )
			break;

		if (fa_rec.fa_cond[0] != CD_OBSOLETE)
			continue;

		if((code = rpline(arayptr)) < 0){
#ifdef ENGLISH
			sprintf(e_mesg,"Internal report error");
#else
			sprintf(e_mesg,"Erreur au rapport interne");
#endif
			code = REPORT_ERR;
			break ;
		}
	}

	rpclose() ;
	close_dbh() ;
	if(code == EFL) return(0);
	return(code);
}
