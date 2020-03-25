/*
*	Source 	: farep2.c 
*
*	Program to Print FA transfers using REPORT GENERATOR.
*
*/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>
#include <bfs_fa.h>

#define	PROJNAME	"fatrrpt"
#define	LOG_REC		1
#define EXIT		12

#define BYORIGINAL	1
#define BYCURRENT	2
#define BYDATE		3
#define DAY_END		4

#ifdef ENGLISH
#define PRINTER 	'P'
#define DISPLAY		'D'
#define FILE_IO		'F'
#else
#define PRINTER		'I'
#define DISPLAY		'A'
#define FILE_IO		'D'
#endif

static	Pa_rec	   pa_rec ;
static  Fa_transfer fa_tran;
static  Fa_rec     fa_rec;
static  Fa_dept    fa_dept;
static  Sch_rec	   school;

extern	int 	rperror ;
extern	char	e_mesg[80];
static	short	copies;
farep2(mode)
int mode;
{
	/* mode == 1   report on original cost center */
	/* mode == 2   report on destination cost center */
   	/* mode == 3   report on date transfered */
	/* mode == 4   report on date of transfer (day end) */

	char	chardate[11] ;
	int	code;
	char 	*arayptr[5] ; 	/** Declarations for Report writer usage **/
	char 	projname[50] ;
	char 	program[11] ;
	int 	outcntl ;
	char 	discfile[20] ;
	int     keyno;
	int     formno;
	short   cc1, cc2;
	long	itemid1, itemid2;
	long	date1, date2;
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
	if( outcntl == 2 ){
		if((retval = GetNbrCopies(&copies))<0 )
			return(retval);
	}
	if(outcntl == 1) {
		STRCPY(e_mesg,"fatrrpt.dat");
		if((retval = GetFilename(e_mesg)) < 0) return(retval);
		STRCPY(discfile,e_mesg);
	}
	else	discfile[0] = '\0';
	date1 = date2 = get_date();
	cc1 = 1;
	cc2 = 9999;
	itemid1 = 1;
	itemid2 = 999999;
	switch(mode) {
		case BYORIGINAL:
		case BYCURRENT: 
			if((retval = GetCostcenRange(&cc1,&cc2))<0 )
				return(retval);
			if((retval = GetItemidRange(&itemid1,&itemid2))<0 )
				return(retval);
			if((retval = Confirm())<= 0)
				return(retval);
			break;
		case BYDATE: 
			if( GetDateRange( &date1, &date2 )<0 )
				return(-1);
			itemid1 = 0;
			if((retval = Confirm())<= 0)
				return(retval);
			break;
		case DAY_END:
			itemid1 = 0;
			break;
		}
	switch(mode) {
		case BYORIGINAL: formno = 1; break;
		case BYCURRENT : formno = 2; break;
		case BYDATE :	 formno = 3; break;
		case DAY_END :	 formno = 3; break;
	}

	code = get_param(&pa_rec, BROWSE, 1, e_mesg) ;
	if(code < 1) {
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

	arayptr[0] = (char *)&fa_tran ;
	arayptr[1] = (char *)&fa_rec ;
	arayptr[2] = (char *)&fa_dept ; 
  	arayptr[3] = (char *)&school ;
        arayptr[4] = NULL ;
 
	/***	Read record and call report writer to output it ***/

	/* Initialize Rec. Entry Header Record to start reading form the
	   beginging */
	switch(mode) {
		case BYORIGINAL:
			keyno = 1;
			fa_tran.fatr_costcen = cc1;
			fa_tran.fatr_itemid = itemid1;
			break;
		case BYCURRENT :
			keyno = 2;
			fa_tran.fatr_tocostcen = cc1;
			fa_tran.fatr_itemid = itemid1;
			break;
		case BYDATE :  
		case DAY_END : 
			keyno = 3;
			fa_tran.fatr_date = date1;	
			break;
		}
	fa_tran.fatr_numb = 0;
	flg_reset( FATRAN );

	for( ; ; ) {
		code = get_n_fatran(&fa_tran,BROWSE,keyno,FORWARD,e_mesg);
		if( code < 0) {
			if(code == EFL) break ;
			code = DBH_ERR;
			break ;
		}
		switch(mode) {
			case BYORIGINAL: {
	 			if(fa_tran.fatr_itemid > itemid2) {
					fa_tran.fatr_itemid = itemid1;
					fa_tran.fatr_costcen++;
					if(fa_tran.fatr_costcen > cc2) {
						code = EXIT;
					}
				}
				break;
			}
			case BYCURRENT: {	
 				if(fa_tran.fatr_itemid > itemid2) {
					fa_tran.fatr_itemid = itemid1;
					fa_tran.fatr_tocostcen++;
					if(fa_tran.fatr_tocostcen > cc2) {
						code = EXIT;
					}
				}
				break;
			}
			case BYDATE: 
			case DAY_END: {
				if(fa_tran.fatr_date > date2) {
					code = EXIT;
				}
				break;
			}
		}
		if(code == EXIT) break;
		if(mode != BYCURRENT) {
 			fa_rec.fa_costcen = fa_tran.fatr_costcen;
			fa_rec.fa_itemid = fa_tran.fatr_itemid;
			code = get_famast(&fa_rec,BROWSE,0,e_mesg);
			if( code != NOERROR) {
				code = DBH_ERR;
				break;
			}
			school.sc_numb = fa_tran.fatr_tocostcen;
			get_sch(&school,BROWSE,0,e_mesg);
			if( code == ERROR) {
				STRCPY(school.sc_name,"???????????????");
			}
			STRCPY(fa_dept.desc,school.sc_name);
			school.sc_numb = fa_tran.fatr_costcen;
			get_sch(&school,BROWSE,0,e_mesg);
			if( code == ERROR) {
				STRCPY(school.sc_name,"???????????????");
			}
		}
		else {
 			fa_rec.fa_curcostcen = fa_tran.fatr_tocostcen;
			fa_rec.fa_costcen = fa_tran.fatr_costcen;
			fa_rec.fa_itemid = fa_tran.fatr_itemid;
			code = get_famast(&fa_rec,BROWSE,3,e_mesg);
			if( code == ERROR) {
				code = DBH_ERR;
				break;			
			}
			school.sc_numb = fa_tran.fatr_costcen;
			get_sch(&school,BROWSE,0,e_mesg);
			if( code == ERROR) {
				STRCPY(school.sc_name,"???????????????");
			}
			STRCPY(fa_dept.desc,school.sc_name);
			school.sc_numb = fa_tran.fatr_tocostcen;
			get_sch(&school,BROWSE,0,e_mesg);
			if( code == ERROR) {
				STRCPY(school.sc_name,"???????????????");
			}
		}
		switch( fa_tran.fatr_cond[0] ){
			case CD_EXCELLENT:
				STRCPY(fa_rec.fa_invc, EXCELLENT);
				break;
 			case CD_GOOD:
				STRCPY(fa_rec.fa_invc, GOOD);
				break;
			case CD_FAIR:
				STRCPY(fa_rec.fa_invc, FAIR);
				break;
			case CD_POOR:
				STRCPY(fa_rec.fa_invc, POOR);
				break;
			case CD_OBSOLETE:
				STRCPY(fa_rec.fa_invc, OBSOLETE);
				break;
		}
			
		if((retval = rpline(arayptr)) < 0)  {
			if(rperror < 0)  {
#ifdef ENGLISH
				sprintf(e_mesg,"Internal report error");
#else
				sprintf(e_mesg,"Erreur au rapport interne");
#endif
				retval = REPORT_ERR;
			}
			else
				retval = NOERROR;
			break ;
		}
	}

	close_file(FATRAN) ;
	close_file(FAMAST) ;
	rpclose() ;
	if(retval == EFL) return(0);
	return(retval);
}
