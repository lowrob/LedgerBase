/*
*	Source 	: glfix.c 
*
*	Program to Analyze Gl master data.
*
*/
#define  MAIN
#define  MAINFL		SUPPLIER		/* main file used */

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	SYSTEM		"GENERAL LEDGER"	/* Sub System Name */
#define	MOD_DATE	"03-JUN-91"		/* Program Last Modified */

static	Pa_rec	pa_rec ;
static  Gl_rec 	gl_rec;
static	char	e_mesg[80];


main(argc,argv)
int argc;
char *argv[];
{
	int code;
	int	i; 

	strncpy(SYS_NAME,SYSTEM,50);	/* Sub system name */
	strncpy(CHNG_DATE,MOD_DATE,10);	/* Modification Date */
	proc_switch(argc, argv, MAINFL) ; 	/* Process Switches */

	code = get_param(&pa_rec, BROWSE, 1, e_mesg) ;
	if(code < 1) {
		close_dbh() ;
		return(DBH_ERR) ;
	}

	gl_rec.funds = 1 ;
	gl_rec.accno[0] = '\0' ;
	gl_rec.reccod = 99 ;
	flg_reset( GLMAST );

	for( ; ; ) {
		code = get_n_gl(&gl_rec,BROWSE,0,FORWARD,e_mesg);
		if( code < 0) {
			if(code == EFL) break ;
			code = DBH_ERR;
			break ;
		}
		if(gl_rec.reccod != 99) continue;
		fprintf(stderr,"Glmaster: %d-%s%d\n",gl_rec.funds,gl_rec.accno, gl_rec.reccod);
		fprintf(stderr,"\t\t OPBAL:  %30.20lf \t\t YTD: %30.20lf\n", gl_rec.opbal, gl_rec.ytd);
		fprintf(stderr,"\t\t COMDAT:  %30.20lf \t\t BUDCUR: %30.20lf\n", gl_rec.comdat, gl_rec.budcur);
		fprintf(stderr,"\t\t CURCR:  %30.20lf \t\t CURDB: %30.20lf\n", gl_rec.curcr, gl_rec.curdb);
		for(i=0;i<13;i++) {
			fprintf(stderr,"\t\t CUREL:  %30.20lf \t\t CURBUD: %30.20lf\n", gl_rec.currel[i], gl_rec.curbud[i]);
		}
	}

	close_dbh() ;
	if(code == EFL) return(0);
	return(code);
}
