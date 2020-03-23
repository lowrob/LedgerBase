/* ---------------------------------------------------------------------------
*	Source 	: joursum.c 
*
*	Program to Print Journal Entry Totals Written to Gl.
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

#define	PROJNAME	"joursum"
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

joursum(mode)
int	mode;
{
Jr_ent		jr_ent;
Gl_rec		gl_rec;
Pa_rec		pa_rec;

char    chardate[11];
int	code;
char 	*arayptr[2] ; 	/** Declarations for Report writer usage **/
char 	projname[50] ;
char 	program[11] ;
int 	outcntl ;
char 	discfile[20] ;
int	retval;
double	tot_amt;
char	prev_acct[19];
short	prev_fund;

short	copies ;

short	err;

	code = get_param(&pa_rec, BROWSE, 1, e_mesg);
 	if (code < 1) {
		printf("%s\n", e_mesg);
		close_dbh();
		return(-1);
	}

	outcntl = 2;

	STRCPY(e_mesg,"joursum.dat");
	STRCPY(discfile, e_mesg);

	copies = 1;

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

	arayptr[0] = (char *)&gl_rec ;
        arayptr[1] = NULL ;

	/***	Read record and call report writer to output it ***/

	prev_acct[0] = '\0';
	prev_fund = 0;

	for( ; ; ){
		tot_amt = 0;

		jr_ent.jr_fund = prev_fund;
		strcpy(jr_ent.jr_acct,prev_acct);
		flg_reset(JR_ENT);

		for( ; ; ) {
			err = get_n_jr_ent(&jr_ent,BROWSE,5,FORWARD,e_mesg) ;
			if(err == EFL) break;
			if( err < 0) {
				fomen(e_mesg);
				get();
				break;
			}
			if(prev_acct[0] == '\0'){
				strcpy(prev_acct, jr_ent.jr_acct);
				prev_fund = jr_ent.jr_fund;
			}
			if(prev_fund != jr_ent.jr_fund) {
				break;
			}
			if(strcmp(jr_ent.jr_acct,prev_acct) != 0)
				break;

			tot_amt += jr_ent.jr_amount;
		}
		gl_rec.funds = prev_fund;
		strcpy(gl_rec.accno,prev_acct);
		gl_rec.reccod = 99;

		code = get_gl(&gl_rec,BROWSE,0,e_mesg);
		if( code < 0) {
			fomen(e_mesg);
			get();
		}

		if(tot_amt > 0){
			gl_rec.nextdb = tot_amt;
		}
		else {
			gl_rec.nextcr = (tot_amt * -1);
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
		if(err == EFL)  break;

		strcpy(prev_acct, jr_ent.jr_acct);
		prev_fund = jr_ent.jr_fund;
	}
	rpclose();
	close_file(JR_ENT) ;
	close_file(GLMAST) ;

	if(code == EFL) return(0);
	return(code);

}
