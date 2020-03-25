/*
*	Source 	: rgprint.c 
*	Module  : Report Generator
*
*	Flexible report printing program.
*
*	This progarm prints reports using REPORT-WRITER and temporay index
*	Files. This program needs logical record# and format#.
*	This assumes Calling program built the Temporary index and set it as a
*	TMPINDX_1 file.
*
*	Sets the report's first title as Company/District name.
*/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#ifdef ENGLISH
#define PRINTER	'P'
#define FILE_IO	'F'
#define DISPLAY	'D'
#else
#define PRINTER	'I'
#define FILE_IO	'D'
#define DISPLAY	'A'
#endif

static	char	*big_buff = NULL ;

/* Common Varibales between all modules */
extern	Pa_rec		pa_rec ;
extern	char		e_mesg[] ;

extern	int	rperror ;

char	*malloc() ;

/*----------------------------------------------------------*/

PrintReport(rep_project, output_on, diskfile, copies, logrec, format)
char	*rep_project ;
char	*output_on ;
char	*diskfile ;	/* Disk file name, when output on Disk */
int	copies ;	/* No of Report Copies, when output on Printer */
int	logrec ;
int	format ;
{
	char	chardate[11];
	int	code ;
	char 	*arayptr[5] ; 	/** Declarations for Report writer usage **/
	char 	projname[50] ;
	int 	outcntl ;
	
	switch(output_on[0]) {
	case FILE_IO :	/*  Print to a file */
		outcntl = 1;
		break;
	case PRINTER :	/*  Print to the printer */
		outcntl = 2;
		break;
	case DISPLAY :	/*  Display on the Terminal */
	default:
		outcntl = 0;
		break;
	}

	mkdate(get_date(),chardate);

	/* Form the project name as a index source file */
	strcpy(projname,FMT_PATH) ;
	strcat(projname,rep_project) ;

	code = rpopen(projname,logrec,format,outcntl,diskfile,PROG_NAME,
			chardate);
							  	
	if ( code < 0 ){
#ifdef ENGLISH
		sprintf(e_mesg,"REPORT-WRITER ERROR... rperror: %d\n",rperror) ;
#else
		sprintf(e_mesg,"ERREUR DU REPORT-WRITER... rperror: %d\n",rperror) ;
#endif
		return(ERROR) ;
	}

	if(output_on[0] == PRINTER)
		rpSetCopies( (int)copies ) ;

	/* Change first title line to Company/School district name */
	rpChangetitle(1, pa_rec.pa_co_name);

	/* For Terminals set pagesize to 23 lines */
	if(outcntl == 0)
		rpPagesize(23);

	big_buff = malloc((unsigned)getreclen(TMPINDX_1)) ;
	if(big_buff == NULL) {
#ifdef ENGLISH
		sprintf(e_mesg,"ERROR in Memory Allocation... errno: %d",errno);
#else
		sprintf(e_mesg,"ERREUR dans l'allocation de memoire... errno: %d",errno);
#endif
		rpclose() ;
		return(ERROR) ;
	}

	arayptr[0] = big_buff ;
	arayptr[1] = NULL ;

	/***	Read record and call report writer to output it ***/

	/* Initialize the Record to nulls to start reading form the begining */

	SetKey(big_buff) ;	/* This function is available in rgmain.c */

	flg_reset( TMPINDX_1 );
	for( ; ; ) {
		code = get_next(big_buff,TMPINDX_1,0,FORWARD,BROWSE,e_mesg);
		if( code < 0) {
			if(code == EFL) break ;
			code = DBH_ERR ;
			break ;
		}

#ifdef	ORACLE
		if(ConstrntChk(big_buff) == ERROR) continue ;
#endif
		if((code = rpline(arayptr)) < 0) {
			if(rperror != 0)
#ifdef ENGLISH
			    sprintf(e_mesg,"REPORT-WRITER ERROR... rperror: %d",
					rperror) ;
#else
			    sprintf(e_mesg,"ERREUR DU REPORT-WRITER... rperror: %d",rperror) ;
#endif
			else
			    code = NOERROR ;
			break ;
		}
	}

	free(big_buff) ;
	rpclose() ;

	if(code != EFL) return(code) ;

	return(NOERROR) ;
}
/*-----------------------------END OF FILE------------------------*/
