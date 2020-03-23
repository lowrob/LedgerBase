/* ---------------------------------------------------------------------------
*	Source 	: jrhlist.c 
*
*	Program to Print Journal History using REPORT GENERATOR.
*

Modifications:

	YY/MM/DD	Programmer	Description
	~~~~~~~~	~~~~~~~~~~	~~~~~~~~~~~
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_pp.h>
#include <bfs_com.h>
#include <repdef.h>

#define	PROJNAME	"jrhlist"
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

extern	char	outside_emp[13]	;
extern	long	outside_date	;
extern	long	outside_cheque;
extern	char	outside_bank[19];
extern	short	outside_fund;

extern	int	rperror	;
extern char	e_mesg[80] ;
#define		INTERACTIVE	0

jrhlist(mode)
int	mode;
{
Jrh_ent		jrh_ent;
Gl_rec		gl_rec;
Pa_rec		pa_rec;

char    chardate[11];
int	code;
char 	*arayptr[3] ; 	/** Declarations for Report writer usage **/
char 	projname[50] ;
char 	program[11] ;
int 	outcntl ;
char 	discfile[20] ;
int	retval;
double	total_amount = 0;

short	copies ;

short	err;

	code = get_param(&pa_rec, BROWSE, 1, e_mesg);
 	if (code < 1) {
		printf("%s\n", e_mesg);
		close_dbh();
		return(-1);
	}

#ifdef ENGLISH
	STRCPY(e_mesg, "F");
#else
	STRCPY(e_mesg, "F");
#endif

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
			STRCPY(e_mesg,"jrhlist.dat");
			if((retval = GetFilename(e_mesg)) < 0)
				return(retval);
			STRCPY(discfile, e_mesg);
		}
		else {
			STRCPY(discfile,"spool.dat");
		}
	}
	else{
			STRCPY(e_mesg,"jrhlist.dat");
			STRCPY(discfile, e_mesg);
	}

	copies = 1;
	if(outcntl == 2 && mode == INTERACTIVE) {
		if((retval = GetNbrCopies( &copies )) < 0)
			return(retval);
	}

	if(mode == INTERACTIVE){
		if((retval = Confirm()) <= 0) 
			return(retval);
	}

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

        arayptr[0] = (char *)&jrh_ent ;
	arayptr[1] = (char *)&gl_rec ;
        arayptr[2] = NULL ;

	/***	Read record and call report writer to output it ***/

	/* Initialize Recs. to start printing benefit report on benefit no. */
	strcpy(jrh_ent.jrh_emp_numb,outside_emp);
	jrh_ent.jrh_date = outside_date;
	jrh_ent.jrh_fund = 0;
	jrh_ent.jrh_acct[0] = '\0';

	flg_reset(JRH_ENT);

	for( ; ; ) {
		code = get_n_jrh_ent(&jrh_ent,BROWSE,1,FORWARD,e_mesg);

		if( code < 0) {
			if(code == EFL) { 
				break ;
			}
			code = DBH_ERR;
			return(code) ;
		}

		if(strcmp(jrh_ent.jrh_emp_numb,outside_emp) != 0 ||
		   jrh_ent.jrh_date != outside_date)
			break;

		if(outside_cheque != jrh_ent.jrh_cheque)
			continue;

		gl_rec.funds = jrh_ent.jrh_fund;
		strcpy(gl_rec.accno,jrh_ent.jrh_acct);
		gl_rec.reccod = 99;

		code = get_gl(&gl_rec,BROWSE,0,e_mesg);
		if( code < 0) {
			if(code == UNDEF) break;
			code = DBH_ERR;
			break;
		}

		if(jrh_ent.jrh_amount < 0){
			gl_rec.nextdb = (jrh_ent.jrh_amount * -1);
		}
		else {
			gl_rec.nextcr = jrh_ent.jrh_amount;
		}

		total_amount +=  jrh_ent.jrh_amount;

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

	gl_rec.funds = outside_fund;
	strcpy(gl_rec.accno,outside_bank);
	gl_rec.reccod = 99;

	code = get_gl(&gl_rec,BROWSE,0,e_mesg);
	if( code < 0) {
		return(code);
	}

	strcpy(jrh_ent.jrh_acct,gl_rec.accno);

	if(total_amount >= 0){
		gl_rec.nextdb = total_amount ;
		gl_rec.nextcr = 0;
	}
	else {
		gl_rec.nextcr = (total_amount * -1);
		gl_rec.nextdb = 0; 
	}

	strcpy(jrh_ent.jrh_emp_numb,outside_emp);

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
		}
	}

	code = EFL;
	rpclose();
	close_file(JRH_ENT) ;
	close_file(GLMAST) ;

	if(code == EFL) return(0);
	return(code);
}
