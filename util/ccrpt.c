/*
*	Source 	: ccrpt.c 
*
*	Program to Print a cost center report using REPORT GENERATOR.
*
*/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	PROJNAME	"cc_rpt"
#define	LOG_REC		1
#define EXIT		12

#ifdef ENGLISH
#define PRINTER		'P'
#define FILE_IO		'F'
#define DISPLAY		'D'
#else
#define PRINTER		'I'
#define FILE_IO		'D'
#define DISPLAY		'A'
#endif

static	Pa_rec	pa_rec ;
static  Sch_rec sch_rec;

static short copies;
extern char e_mesg[] ;

CCrpt()
{
	char	chardate[11] ;
	int	code;
	char 	*arayptr[5] ; 	/** Declarations for Report writer usage **/
	char 	projname[30] ;
	int 	outcntl ;
	char 	program[11] ;
	char 	discfile[20] ;
	char	summary[2];
	char	ccname1[28], ccname2[28];
	short	ccno1, ccno2;
	int	i,j;
	int	retval;


#ifdef ENGLISH
	strcpy(e_mesg,"P");
#else
	strcpy(e_mesg,"I");
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
	copies = 1;
	if( outcntl==2 )
		if( (retval=GetNbrCopies(&copies))<0 )
			return( retval );
	if(outcntl == 1) {
		strcpy(e_mesg,"ccrpt.dat");
		if(GetFilename(e_mesg) < 0) return(retval);
		strcpy(discfile,e_mesg);
	}
	else	discfile[0] = '\0';

	ccno1 = 0;
	ccno2 = 99;

	if((retval = Confirm()) < 0) return(-1);
	else	if(!retval) return(0);

	code = get_param(&pa_rec, BROWSE, 1, e_mesg) ;
	if(code < 1) {
		close_dbh() ;
		return(DBH_ERR) ;
	}

	mkdate(get_date(),chardate);
	strcpy(projname,FMT_PATH) ;
	strcat(projname,PROJNAME) ;
	strcpy(program, PROG_NAME );

	code = rpopen(projname,LOG_REC,1,outcntl,discfile,program,
			chardate);
	if( outcntl==2 )
		rpSetCopies( (int)copies );
	if ( code < 0 ){
		sprintf(e_mesg, "Rpopen code :%d\n", code ) ;
		close_dbh() ;
		return(REPORT_ERR);
	}

	/* Change first title line to Company/School district name */
	rpChangetitle(1, pa_rec.pa_co_name);

	/* For Terminals set pagesize to 23 lines */
	if(outcntl == 0)
		rpPagesize(23);

	arayptr[0] = (char *)&sch_rec ;
	arayptr[1] = NULL ;

	/***	Read record and call report writer to output it ***/

	/* Initialize Rec. Entry Header Record to start reading form the
	   beginging */
	sch_rec.sc_numb = ccno1;

	flg_reset( SCHOOL );

	for( ; ; ) {
		code = get_n_sch(&sch_rec,BROWSE,0,FORWARD,e_mesg);
		if( code < 0) 
			break ;
		
		for(i=3,j=0;sch_rec.sc_phone[i] != '\0';i++,j++) {
			if(i==6) {
				sch_rec.sc_phone[j] = '-';
				j++;
			}
			sch_rec.sc_phone[j] = sch_rec.sc_phone[i];
		}
		sch_rec.sc_phone[j] = '\0';

		if(rpline(arayptr) < 0) break ;
	}

	close_dbh() ;
	rpclose() ;
	if(code < 0 && code != EFL) return(DBH_ERR);
	return(0);
}
