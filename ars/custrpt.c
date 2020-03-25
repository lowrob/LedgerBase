/*
*	Source 	: custrpt.c 
*
*	Program to Print customer Details using REPORT GENERATOR.
*
*/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	PROJNAME	"custrpt"
#define	LOG_REC		1
#define ABB_NAME	1
#define EXIT		12

#ifdef ENGLISH
#define PRINTER 	'P'
#define DISPLAY		'D'
#define FILE_IO		'F'

#define YES		'Y'
#else
#define PRINTER		'I'
#define DISPLAY		'A'
#define FILE_IO		'D'

#define YES		'O'
#endif

static	Pa_rec	pa_rec ;
static  Cu_rec cu_rec;

static short copies;
extern char e_mesg[80] ;

Custrpt(mode)
int mode;
{
	char	chardate[11] ;
	int	code;
	char 	*arayptr[5] ; 	/** Declarations for Report writer usage **/
	char 	projname[50] ;
	int 	outcntl ;
	char 	program[11] ;
	char 	discfile[20] ;
	int     keyno;
	int     formno;
	char	summary[2];
	char	custname1[25], custname2[25];
	char	custno1[11], custno2[11];
	int	retval;


#ifdef ENGLISH
	STRCPY(e_mesg,"P");
#else
	STRCPY(e_mesg,"I");
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
		if( (retval=GetNbrCopies(&copies))<0)
			return(retval);
	if(outcntl == 1) {
		STRCPY(e_mesg,"custrpt.dat");
		if(GetFilename(e_mesg) < 0) return(retval);
		STRCPY(discfile,e_mesg);
	}
	else	discfile[0] = '\0';

	if(mode == ABB_NAME){
		STRCPY(custname1,"");
		STRCPY(custname2,"zzzzzzzzzzzzzzzzzzzz");
		retval = GetCNameRange( custname1, custname2 );
		if(retval < 0) return(retval);
		keyno = 2;
	}
	else {
		STRCPY(custno1,"         1");
		STRCPY(custno2,"ZZZZZZZZZZ");
		retval = GetCNbrRange( custno1, custno2 );
		if(retval < 0) return(retval);
		keyno = 0;
	}

#ifdef ENGLISH
	if((retval = DisplayMessage("Summary (Y/N)?"))< 0) return(retval);
#else
	if((retval = DisplayMessage("Resume (O/N)?"))< 0) return(retval);
#endif
	if((retval = GetResponse(summary, "YN"))<0) return(retval);
	if(summary[0] == YES) {
		if(mode == ABB_NAME) {
			formno = 4;
		}
		else formno = 2;
	}
	else {
		if(mode == ABB_NAME) {
			formno = 3;
		}
		else formno = 1;
	}
	if((retval = Confirm()) < 0) return(-1);
	else	if(!retval) return(0);

	code = get_param(&pa_rec, BROWSE, 1, e_mesg) ;
	if(code < 1) {
		close_dbh() ;
		return(DBH_ERR) ;
	}

	mkdate(get_date(),chardate);
	STRCPY(projname,FMT_PATH) ;
	strcat(projname,PROJNAME) ;
	STRCPY(program, PROG_NAME );

	code = rpopen(projname,LOG_REC,formno,outcntl,discfile,program,
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

	arayptr[0] = (char *)&cu_rec ;
	arayptr[1] = NULL ;

	/***	Read record and call report writer to output it ***/

	/* Initialize Rec. Entry Header Record to start reading form the
	   beginging */
	if (keyno == 0)
		STRCPY(cu_rec.cu_code,custno1);
	else
		STRCPY(cu_rec.cu_abrev,custname1);

	flg_reset( CUSTOMER );

	for( ; ; ) {
		code = get_n_cust(&cu_rec,BROWSE,keyno,FORWARD,e_mesg);
		if( code < 0) {
			break ;
		}
		if(mode == ABB_NAME) {
			if(strcmp(cu_rec.cu_abrev,custname2) > 0) break;
		}
		else {
			if(strcmp(cu_rec.cu_code,custno2) > 0) break;
		}
		if(summary[0] == YES) {
			if  (strcmp(cu_rec.cu_phone, "")) {
				strncpy(cu_rec.cu_adr3,cu_rec.cu_phone,3);
				cu_rec.cu_adr3[3] = '\0';
				strncat(cu_rec.cu_adr3,"-",1);
				strncat(cu_rec.cu_adr3,cu_rec.cu_phone+3,3);
				strncat(cu_rec.cu_adr3,"-",1);
				strncat(cu_rec.cu_adr3,cu_rec.cu_phone+6,4);
			}
			else
				STRCPY(cu_rec.cu_adr3,"");
		}
		if(rpline(arayptr) < 0) break ;
	}
	close_dbh() ;
	rpclose() ;
	if(code < 0 && code != EFL) return(DBH_ERR);
	return(0);
}
