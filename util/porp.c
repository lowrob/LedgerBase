/******************************************************************************
		Sourcename    : pooutstd.c
		System        : Budgetary Financial system.
		Module        : Purchase Order
		Created on    : 89-09-30
		Created  By   : Cathy Burns 
		Cobol sources : 
*******************************************************************************
About this program:

HISTORY:
 Date           Programmer     Description of modification
____/__/__      __________     ___________________________
*******************************************************************************/
#define MAIN
#define MAINFL		-1

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	PROJNAME	"porpt"
#define	LOG_REC		1
#define	FORMNO		7
#define EXIT		12
#define SYSTEM		"PURCHASE ORDER"
#define MOD_DATE	"28-SEPT-89"

char e_mesg[80]; /* to store error messages */

main(argc,argv)
int	argc;
char	*argv[];
{
Pa_rec 	 pa_rec ;
Supplier supp_rec;
Po_hdr   pohdr_rec;
Po_item  poitem_rec;

char    chardate[11];
char	chardate1[11] ;
char	chardate2[11] ;
int	code;
char 	*arayptr[5] ; 	/** Declarations for Report writer usage **/
char 	projname[30] ;
char 	program[11] ;
int	retval;

	strncpy( SYS_NAME, SYSTEM, 50 ); 	/* Module name */
	strncpy( CHNG_DATE, MOD_DATE, 10 );	/* Last date of change */
	
	proc_switch(argc,argv,MAINFL);
	if(argc < 3)
		exit(0);


	code = get_param(&pa_rec, BROWSE, 1, e_mesg) ;
	if(code < 1) {
		close_dbh() ;
		return(DBH_ERR) ;
	}
	mkdate(get_date(),chardate);

	strcpy(projname,FMT_PATH) ;
	strcat(projname,PROJNAME) ;
	strcpy(program, PROG_NAME );

	code = rpopen(projname,LOG_REC,FORMNO,1,"jon1.dat",program,
			chardate);
							  	
	if ( code < 0 ){
		sprintf(e_mesg,"Rpopen code :%d\n", code ) ;
		close_dbh() ;
		return(REPORT_ERR);
	}

	/* Change first title line to Company/School district name */
	retval = rpChangetitle(2, pa_rec.pa_co_name);

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
	arayptr[1] = (char *)&pohdr_rec ;
        arayptr[2] = (char *)&poitem_rec ;
        arayptr[3] = NULL ;

	/***	Read record and call report writer to output it ***/

	/* Initialize Recs. to start printing po report on po no. */
	pohdr_rec.ph_code = 0;

	flg_reset( POHDR );

	for( ; ; ) {
		code = get_n_pohdr(&pohdr_rec,BROWSE,0,FORWARD,e_mesg);

		if( code < 0) {
			if(code == EFL) break ;
			code = DBH_ERR;
			break ;
		}

		if(pohdr_rec.ph_status[0] == COMPLETE) continue;

		poitem_rec.pi_code = pohdr_rec.ph_code;
 		poitem_rec.pi_item_no = 0;
        	flg_reset( POITEM );
		for( ; ; ) {
#ifndef ORACLE
			code = get_n_poitem(&poitem_rec,BROWSE,0,FORWARD,e_mesg);
#else
			code = get_n_poitem(&poitem_rec,BROWSE,0,EQUAL,e_mesg);
#endif
		  	if( code < 0) {
				if(code == EFL) break ;
				code = DBH_ERR;
				break ;
		       	}
#ifndef ORACLE
		  	if(pohdr_rec.ph_code != poitem_rec.pi_code) break;
#endif

		  	if((code = rpline(arayptr)) < 0) {
#ifdef ENGLISH
				sprintf(e_mesg,"Internal report error");
#else
				sprintf(e_mesg,"Erreur au rapport interne");
#endif
				code = REPORT_ERR;
				break;
			}

		}
	}

	close_dbh() ;
	rpclose() ;
	if(code == EFL) return(0);
	return(code);
}
