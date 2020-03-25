/*
*	Source 	: rec_rep.c 
*
*	Program to Print Recurring entry Details using REPORT GENERATOR.
*
*/
/***
#define	MAIN 
***/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	PROJNAME	"recentry"
#define	LOG_REC		1
#define	FORMNO		1
#define	EXIT		12

#ifdef ENGLISH
#define PRINTER 'P'
#define DISPLAY 'D'
#define FILE_IO	'F'
#else
#define PRINTER 'I'
#define DISPLAY 'A'
#define FILE_IO	'D'
#endif

extern	int	rperror ;
static	Pa_rec	pa_rec ;
static	Gl_rec	gl_rec ;
static	Re_hdr	re_hdr ;
static	Re_item	re_item ;

static	char	e_mesg[100] ;

rec_rep()
{
	char	chardate[11], output[2];
	int	code , j;
	char 	*arayptr[5] ; 	/** Declarations for Report writer usage **/
	char 	projname[50] ;
	char 	program[11] ;
	int 	outcntl ;
	short	copies ;
	char 	discfile[20] ;

	code = get_param(&pa_rec, BROWSE, 1, e_mesg) ;
	if(code < 1) {
		printf("%s\n",e_mesg);
		close_dbh() ;
		return(-1) ;
	}

#ifdef ENGLISH
	STRCPY( output, "P" );
#else
	STRCPY( output, "I" );
#endif
	if( GetOutputon(output)<0 )
		return(-1);
	switch( output[0] ){
		case DISPLAY:	/* Display */
			outcntl = 0;
			break;
		case FILE_IO:	/* File */
			outcntl = 1;
			break;
		case PRINTER:	/* Printer */
		default:
			outcntl = 2;
			break;
	}

	discfile[0] = '\0';
	if(outcntl == 1 || outcntl == 3 || outcntl == 5 || outcntl == 6) {
		STRCPY(discfile,"recurring.dat");
		if( GetFilename( discfile )<0 )
			return(-1);
	}

	copies = 1;
	if(outcntl == 2) {
		if((code == GetNbrCopies( &copies )) < 0)
			return(code);
	}

	code = Confirm();
	if( code<0 )
		return(-1);
	if( code==0 )
		return(0);

	mkdate(get_date(),chardate);

	STRCPY(projname,FMT_PATH) ;
	strcat(projname,PROJNAME) ;
	STRCPY(program, "REC_REP" );

	code = rpopen(projname,LOG_REC,FORMNO,outcntl,discfile,program,
			chardate);
							  	
	if ( code < 0 ){
		printf( "Rpopen code :%d\n", code ) ;
		close_dbh() ;
		return(-1);
	}

	if(outcntl == 2)
		rpSetCopies( (int)copies );	/* number of copies to print */

	/* Change first title line to Company/School district name */
	rpChangetitle(1, pa_rec.pa_co_name);

	/* For Terminals set pagesize to 23 lines */
	if(outcntl == 0)
		rpPagesize(23);

	arayptr[0] = (char *)&re_hdr ;
	arayptr[1] = (char *)&re_item ; 
	arayptr[2] = (char *)&gl_rec ; 
	arayptr[3] = NULL ;

	/***	Read record and call report writer to output it ***/

	/* Initialize Rec. Entry Header Record to start reading form the
	   beginging */

	re_hdr.rh_fund = 0 ;
	re_hdr.rh_sno  = 0 ;
	flg_reset( RECHDR );

	for( ; ; ) {
		code = get_n_rehdr(&re_hdr, BROWSE, 0, FORWARD, e_mesg);
		if( code < 0) {
			if(code == EFL) break ;
			printf("%s\n",e_mesg);
			break ;
		}

		/* Initialize Rec. Entry Trans to current Hdr */
		re_item.ri_fund = re_hdr.rh_fund ;
		re_item.ri_sno  = re_hdr.rh_sno  ;
		re_item.ri_item_no = 0  ;
		flg_reset(RECTRAN) ;

		for( ; ; ) {
#ifndef	ORACLE
			code = get_n_reitem(&re_item,BROWSE,0,FORWARD,e_mesg);
#else
			code = get_n_reitem(&re_item,BROWSE,0,EQUAL,e_mesg);
#endif
			if( code < 0)  break ;

#ifndef	ORACLE
			/* If key changes break */
			if(re_item.ri_sno != re_hdr.rh_sno ||
				re_item.ri_fund != re_hdr.rh_fund) break ;
#endif

			/* Get The Acccount record  for the account# in item */
			gl_rec.funds = re_hdr.rh_fund ;
			strncpy(gl_rec.accno, re_item.ri_accno, sizeof(gl_rec.accno)) ;
			gl_rec.reccod = re_hdr.rh_reccod ;

			code = get_gl( &gl_rec, BROWSE, 0, e_mesg );

			if(code == ERROR) break ;

			if( code < 0 )
				STRCPY(gl_rec.desc,"?????????????");

			if(rpline(arayptr) < 0)  {
				if(rperror < 0)  {
#ifdef	ENGLISH
				sprintf(e_mesg,"Internal report error");
#else
				sprintf(e_mesg,"Erreur au rapport interne");
#endif
					code = REPORT_ERR;
				}
				else
					code = EXIT;
				break;
			}
		}

		/* If internal for() has received DBH error */
		if(code != EFL && code < 0) {
			printf("%s\n",e_mesg);
			break ;
		}
		if(code == EXIT)  break;
	}

	close_dbh() ;
	rpclose() ;
	return(0);
}
