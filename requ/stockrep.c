/* ---------------------------------------------------------------------------
*	Source 	: stockrep.c 
*
*	Program to Print stock codes and descriptions using REPORT GENERATOR.
*

Modifications:

	YY/MM/DD	Programmer	Description
	~~~~~~~~	~~~~~~~~~~	~~~~~~~~~~~
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	PROJNAME	"reqrpt2"
#define	LOG_REC		1
#define	FORMNO		3
#define EXIT		12

#ifdef ENGLISH
#define PRINTER 'P'
#define DISPLAY 'D'
#define FILE_IO	'F'
#define SPOOL	'S'
#define YES	'Y'
#else
#define PRINTER 'I'
#define DISPLAY 'A'
#define FILE_IO	'D'
#define SPOOL	'B'
#define YES	'O'
#endif

extern	int	rperror	;
extern char	e_mesg[80] ;

stockrep()
{
Pa_rec 	 pa_rec ;
Req_hdr	 req_hdr;
Req_item req_item;
Gl_rec	 glmast;
St_mast  stmast;

char	desc[14];
char	total_str[132];

char    chardate[11];
char	chardate1[11] ;
char	chardate2[11] ;
int	code;
char 	*arayptr[5] ; 	/** Declarations for Report writer usage **/
char 	projname[50] ;
char 	program[11] ;
int 	outcntl ;
char 	discfile[20] ;
int     keyno;
int	retval;
char	stockcd1[11];
char	stockcd2[11];
short	fund1;
short	fund2;

short	copies ;

short	err;

	code = get_param(&pa_rec, BROWSE, 1, e_mesg) ;
	if(code < 1) {
		return(DBH_ERR) ;
	}

	keyno = 0;
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
			STRCPY(e_mesg,"stockrep.dat");
			if((retval = GetFilename(e_mesg)) < 0)
				return(retval);
			STRCPY(discfile, e_mesg);
		}
		else {
			sprintf(discfile,"spool%d",CC_no);
		}
	}
	else 	discfile[0] = '\0';

	copies = 1;
	if(outcntl == 2) {
		if((retval = GetNbrCopies( &copies )) < 0)
			return(retval);
	}

	fund1 = 1;
	fund2 = 999;
	retval = GetFundRange( &fund1, &fund2 );
	if(retval < 0) return(retval);

	STRCPY(stockcd1,"         1");
	STRCPY(stockcd2,"ZZZZZZZZZZ");
	retval = GetStckRange( stockcd1, stockcd2 );
	if(retval < 0) return(retval);
	
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
	
	/* For Terminals set pagesize to 23 lines */ 
	if(outcntl == 0)
		rpPagesize(23);

	arayptr[0] = (char *)&req_hdr ;
        arayptr[1] = (char *)&req_item ;
	arayptr[2] = (char *)&glmast ;
	arayptr[3] = (char *)&stmast ;
        arayptr[4] = NULL ;

	/***	Read record and call report writer to output it ***/

	/* Initialize Recs. to start printing req report on req no. */
	stmast.st_fund = fund1;
	STRCPY(stmast.st_code,stockcd1);

	flg_reset( STMAST );

	for( ; ; ) {
		code = get_n_stmast(&stmast,BROWSE,keyno,FORWARD,e_mesg);
		if( code < 0) {
			if(code == EFL)  break ;
			code = DBH_ERR;
			break ;
		}

		if(stmast.st_fund > fund2) {  /* fund range */
			break;
		}

		if(strcmp(stmast.st_code,stockcd1) < 0 || 
		   strcmp(stmast.st_code,stockcd2) > 0) {
			continue;
		}

		if((code = rpline(arayptr)) <0) {
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
	rpclose();
	close_dbh() ;

	if(code == EFL) return(0);
	return(code);
}


