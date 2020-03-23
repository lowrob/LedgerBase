/* ---------------------------------------------------------------------------
*	Source 	: terminat.c 
*
*	Program to Print Termination Codes using REPORT GENERATOR.
*	By : Andre Cormier

Modifications:

	YY/MM/DD	Programmer	Description
	~~~~~~~~	~~~~~~~~~~	~~~~~~~~~~~
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_pp.h>
#include <bfs_com.h>
#include <repdef.h>

#define	PROJNAME	"terminat"
#define	LOG_REC		1
#define	FORMNO		1
#define EXIT		12

#ifdef ENGLISH
#define PRINTER 'P'
#define DISPLAY 'D'
#define FILE_IO	'F'
#define SPOOL	'S'
#define YES	'Y'
#define NO	'N'
#else
#define PRINTER 'I'
#define DISPLAY 'A'
#define FILE_IO	'D'
#define SPOOL	'B'
#define YES	'O'
#define NO	'N'
#endif

extern	int	rperror	;
extern char	e_mesg[80] ;

terminat()
{
Term		term;
Pa_rec		pa_rec;


char    chardate[11];
int	code;
char 	*arayptr[2] ; 	/** Declarations for Report writer usage **/
char 	projname[50] ;
char 	program[11] ;
int 	outcntl ;
char 	discfile[20] ;
int	retval;

short	copies ;

short	err;

	code = get_param(&pa_rec, BROWSE, 1, e_mesg);
 	if (code < 1) {
		printf("%s\n", e_mesg);
		close_dbh();
		return(-1);
	}


#ifdef ENGLISH
	STRCPY(e_mesg, "P");
#else
	STRCPY(e_mesg, "I");
#endif
	retval = GetOutputon(e_mesg);
	if(retval < 0) return(retval);

	switch( *e_mesg){
		case DISPLAY:	/* display on Terminal */
			outcntl = 0;
			break;
		case FILE_IO:	/* print to a file */
		case SPOOL:	/* spool report */
			outcntl = 1;
			break;
		case PRINTER:
			outcntl = 2;
			break;
		default:
			outcntl = 2;
			break;
	}
	if(outcntl == 1) {
		if(e_mesg[0]==FILE_IO) {
			STRCPY(e_mesg,"terminat.dat");
			if((retval = GetFilename(e_mesg)) < 0)
				return(retval);
			STRCPY(discfile, e_mesg);
		}
		else {
			STRCPY(discfile,"spool.dat");
		}
	}
	else 	discfile[0] = '\0';

	copies = 1;
	if(outcntl == 2) {
		if((retval = GetNbrCopies( &copies )) < 0)
			return(retval);
	}

	if((retval = Confirm()) <= 0) 
		return(retval);


	mkdate(get_date(),chardate);

	STRCPY(projname,FMT_PATH) ;
	strcat(projname,PROJNAME) ;
	STRCPY(program, PROG_NAME );

	code = rpopen(projname,LOG_REC,FORMNO,outcntl,discfile,program,
			chardate);
							  	
	if ( code < 0 ){
		sprintf(e_mesg,"Rpopen code :%d\n", code ) ;
		close_dbh() ;
		return(REPORT_ERR);
	}

	if(outcntl == 2) 
		rpSetCopies( (int)copies );   /* number of copies to print */

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
	
 
	/* For Terminals set pagesize to 22 lines */ 
	if(outcntl == 0)
		rpPagesize(22);

	arayptr[0] = (char *)&term ;
        arayptr[1] = NULL ;

	/***	Read record and call report writer to output it ***/

	/* Initialize Recs. to start printing benefit report on benefit no. */
	term.t_code[0] = '\0';

	flg_reset(TERM);

	for( ; ; ) {
		code = get_n_pterm(&term,BROWSE,0,FORWARD,e_mesg);

		if( code < 0) {
			if(code == EFL)  break ;
			code = DBH_ERR;
			break ;
		}

		code = rpline(arayptr);
		if(code<0) {
			if(rperror < 0)  {
#ifdef ENGLISH
				sprintf(e_mesg,"Internal report error");
#else
				sprintf(e_mesg,"Erreur au rapport interne");
#endif
				code = REPORT_ERR;
			}
			else {
				code = EXIT ;
				break;
			}
		}
	}
	close_file(TERM) ;
	rpclose();

	if(code == EFL) return(0);
	return(code);

}
