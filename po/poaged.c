/*
*	Source 	: poaged.c 
*
*	Program to Print Purchase Orders Aged Report.
*
*/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	PROJNAME	"porpt"
#define	LOG_REC		1
#define	FORMNO		2
#define EXIT		12
#define DELTA		0.005		/* To check float or double values*/

#ifdef ENGLISH
#define PRINTER 'P'
#define DISPLAY 'D'
#define FILE_IO	'F'
#else
#define PRINTER 'I'
#define DISPLAY 'A'
#define FILE_IO	'D'
#endif

extern	int	rperror;
extern char	e_mesg[80] ;

PoAged()
{
Pa_rec 	 pa_rec ;
Supplier supp_rec;
Po_hdr   pohdr_rec;
Po_item  poitem_rec;

char	chardate[11] ;
int	code;
char 	*arayptr[5] ; 	/** Declarations for Report writer usage **/
char 	projname[50] ;
char 	program[11] ;
int 	outcntl ;
char 	discfile[20] ;
int     keyno;
int     formno;
int	retval;
char    suppno1[11], suppno2[11];
char    tempsupp[11];
long	parm_date ;
long	diff ;
short	copies ;

	code = get_param(&pa_rec, BROWSE, 1, e_mesg) ;
	if(code < 1) {
		close_dbh() ;
		return(DBH_ERR) ;
	}
	formno = 5;
	keyno = 1;
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
			outcntl = 1;
			break;
		case PRINTER:       /* Print to printer */
			outcntl = 2;
			break;
		default:
			outcntl = 2;
			break;
	}
	if(outcntl == 1) {
		STRCPY(e_mesg,"poaged.dat");
		if((retval = GetFilename(e_mesg)) < 0)
			return(retval);
		STRCPY(discfile, e_mesg);
	}
	else 	discfile[0] = '\0';

	copies = 1;
	if( outcntl == 2) {
		if((retval = GetNbrCopies( &copies )) < 0)
			return(retval);
	}

	STRCPY(suppno1,"         0");
	STRCPY(suppno2,"ZZZZZZZZZZ");
	retval = GetSuppRange( suppno1, suppno2 );
	if(retval < 0) return(retval);

	parm_date = get_date();
	retval = GetDate( &parm_date );
	if(retval < 0) return(retval);

	if((retval = Confirm()) <= 0) return(retval);

	mkdate(get_date(),chardate);

	STRCPY(projname,FMT_PATH) ;
	strcat(projname,PROJNAME) ;
	STRCPY(program, PROG_NAME );

	code = rpopen(projname,LOG_REC,formno,outcntl,discfile,program,
			chardate);
							  	
	if ( code < 0 ){
		sprintf(e_mesg,"Rpopen code :%d\n", code ) ;
		close_dbh() ;
		return(REPORT_ERR);
	}

	if( outcntl == 2) 
		rpSetCopies( (int)copies );   /* number of copies to print */

	/* Change first title line to Company/School district name */
	if((code = rpChangetitle(1, pa_rec.pa_co_name))<0) {
#ifdef ENGLISH
		sprintf(e_mesg,"Internal report error");
#else
		sprintf(e_mesg,"Erreur au rapport interne");
#endif
		rpclose();
		close_dbh();
		return(REPORT_ERR);
	}


	mkdate( parm_date, chardate );
#ifdef ENGLISH
	sprintf(e_mesg,"As of %s",chardate);
#else
	sprintf(e_mesg,"a partir de %s",chardate);
#endif
	if((code = rpChangetitle(3,e_mesg))<0) {
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

	arayptr[0] = (char *)&supp_rec ;
	arayptr[1] = (char *)&pohdr_rec ;
        arayptr[2] = (char *)&poitem_rec ;
        arayptr[3] = NULL ;

	/***	Read record and call report writer to output it ***/

	/* Initialize Recs. to start printing po report on po no. */
	STRCPY(tempsupp,"          ");         
	STRCPY(pohdr_rec.ph_supp_cd,suppno1);
	pohdr_rec.ph_code = 0;

	flg_reset( POHDR );

	for( ; ; ) {
		code = get_n_pohdr(&pohdr_rec,BROWSE,keyno,FORWARD,e_mesg);
		if( code < 0) {
			if(code == EFL) break;
			code = DBH_ERR;
			break ;
		}
		if(strcmp(pohdr_rec.ph_supp_cd,suppno2) > 0) break;

		if(pohdr_rec.ph_comm - pohdr_rec.ph_lqamt > -(DELTA) &&
		   pohdr_rec.ph_comm - pohdr_rec.ph_lqamt < DELTA)
			continue;

		if(strcmp(pohdr_rec.ph_supp_cd,tempsupp) != 0) {
		     STRCPY(supp_rec.s_supp_cd,pohdr_rec.ph_supp_cd);
		     code = get_supplier(&supp_rec,BROWSE,0,e_mesg);
		     if( code != NOERROR) {
			 STRCPY(supp_rec.s_name,"???????????????");
		     }
		     STRCPY(tempsupp,pohdr_rec.ph_supp_cd);
		     sprintf(supp_rec.s_add1,"%10s - %17s",supp_rec.s_supp_cd,
				supp_rec.s_name);
		}

		supp_rec.s_ytd_ord = supp_rec.s_ytd_ret = 0 ;
		supp_rec.s_ytd_recpt = supp_rec.s_ytd_disc = 0 ;
		
		diff = days(pohdr_rec.ph_due_date) - days(parm_date);

		if(diff < 0)	/* Past Due */ 
		  supp_rec.s_ytd_ord = pohdr_rec.ph_comm - pohdr_rec.ph_lqamt;

		else if (diff <= 30) /* Currently Due */
		  supp_rec.s_ytd_ret = pohdr_rec.ph_comm - pohdr_rec.ph_lqamt;

		else if (diff <= 60)  /*Due Next Period */
		  supp_rec.s_ytd_recpt = pohdr_rec.ph_comm - pohdr_rec.ph_lqamt;

		else					/* Due in Future */ 
		  supp_rec.s_ytd_disc = pohdr_rec.ph_comm - pohdr_rec.ph_lqamt;

	       	if((code = rpline(arayptr)) < 0) {
			if( rperror < 0 )  {
#ifdef ENGLISH
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

	close_dbh() ;
	rpclose() ;
	if(code == EFL ) return(0);
	return(code);
}
