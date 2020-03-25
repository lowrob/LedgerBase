/*
*	Source 	: bdtrrep.c 
*
*	Program to Print Budget transaction Details using REPORT GENERATOR.
*      
*       modification by : J. Cormier
*                date   : 1990/09/27
*            location   : lines 51, 94,95,138,153 thru 162 on the
*                         the date 1990/09/27 (line numbers subject
*                         to verification should later modifications
*                         occur)
*
*/
/***
#define	MAIN 
***/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	EXIT		12

#define	PROJNAME	"bdtrrep"
#define	LOG_REC		1
#define	FORMAT_NO	1

#ifdef ENGLISH
#define PRINTER	'P'
#define DISPLAY 'D'
#define FILE_IO	'F'
#else
#define PRINTER	'I'
#define DISPLAY 'A'
#define FILE_IO	'D'
#endif
static	Pa_rec	pa_rec ;
static	Bd_hdr	bd_hdr ;
static	Bd_item	bd_item ;

extern	int	rperror ;
static	char	e_mesg[100] ;

bdtrrep()
{
	char	chardate[11], output[2];
	int	code ;
	char 	*arayptr[5] ; 	/** Declarations for Report writer usage **/
	char 	projname[50] ;
	char 	program[11] ;
	int 	outcntl ;
	short	copies ;
	char 	discfile[20] ;
        long    date1, date2;

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

	copies = 1;
	if(outcntl == 2) {
		if((code == GetNbrCopies( &copies )) < 0)
			return(code);
	}

	discfile[0] = '\0';
	if(outcntl == 1) {
		STRCPY(discfile,"bdtrrep.dat");
		if( GetFilename( discfile )<0 )
			return(-1);
	}
        /* * */
        /* next line modification added 90/09/27  -date range */
	date1 = date2 = pa_rec.pa_date;
        GetDateRange(&date1, &date2);
        /* * */
	code = Confirm();
	if( code<0 )
		return(-1);
	if( code==0 )
		return(0);

	mkdate(get_date(),chardate);

	STRCPY(projname,FMT_PATH) ;
	strcat(projname,PROJNAME) ;
	STRCPY(program, "BDTRREP" );

	code = rpopen(projname,LOG_REC,FORMAT_NO,outcntl,discfile,program,
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

	arayptr[0] = (char *)&bd_hdr ;
	arayptr[1] = (char *)&bd_item ; 
	arayptr[2] = NULL ;

	/***	Read record and call report writer to output it ***/

	/* Initialize Budget trans Header Record to start reading from the
	   beginning */

	bd_hdr.tr_term[0] = '\0' ;
	bd_hdr.tr_sys_dt =  date1;
	bd_hdr.tr_seq_no  = 0 ;
	flg_reset( GLBDHDR );

	for( ; ; ) {
		code = get_n_bdhdr(&bd_hdr, BROWSE, 0, FORWARD, e_mesg);
		if( code < 0) {
			if(code == EFL) break ;
			printf("%s\n",e_mesg);
			break ;
		}
               /*   */
               /* the following lines, of modification for */
               /* inclusion of a date ranged report */
               /*   */
                if( bd_hdr.tr_sys_dt < date1 ) {
                    continue;
                }
                if( bd_hdr.tr_sys_dt > date2 ) {
                   inc_str( bd_hdr.tr_term, sizeof (bd_hdr.tr_term)+1, FORWARD);
                   bd_hdr.tr_seq_no = 0;
                   flg_reset(GLBDHDR);
                   continue;
                }
                /* */
                /* * * end of modification */
		/* Initialize Rec. Entry Trans to current Hdr */
		STRCPY( bd_item.tr_term, bd_hdr.tr_term );
		bd_item.tr_sys_dt = bd_hdr.tr_sys_dt ;
		bd_item.tr_seq_no = bd_hdr.tr_seq_no ;
		bd_item.tr_item_no = 0  ;
		flg_reset(GLBDITEM) ;

		for( ; ; ) {
#ifndef	ORACLE
			code = get_n_bditem(&bd_item,BROWSE,0,FORWARD,e_mesg);
#else
			code = get_n_bditem(&bd_item,BROWSE,0,EQUAL,e_mesg);
#endif
			if( code < 0)  break ;

#ifndef	ORACLE
			/* If key changes break */
			if( strcmp(bd_item.tr_term, bd_hdr.tr_term) ||
			    bd_item.tr_sys_dt != bd_hdr.tr_sys_dt   ||
			    bd_item.tr_seq_no != bd_hdr.tr_seq_no     )
				break;
#endif

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
				 break ;
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
