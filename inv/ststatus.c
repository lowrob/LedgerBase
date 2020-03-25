/******************************************************************************
		Sourcename    : ststatus.c
		System        : Budgetary Financial system.
		Module        : Inventory reports
		Created on    : 89-09-19
		Created  By   : K HARISH.
		Cobol sources : 
*******************************************************************************
About this file:
	This routine generates one of the following :
		1. Trial balance.
		2. Stock Below Minimum report.
		3. Stock status report for the month.
		4. Stock status report for the year.

	Calling file:	stockrep.c
******************************************************************************/
#define	EXIT	12		/* as defined in streputl.c */

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>
#include <repdef.h>
#include <bfs_inv.h>

#ifdef ENGLISH
#define PRINTER		'P'
#define DISPLAY		'D'
#define FILE_IO		'F'

#define BYMONTH		'M'
#define BYYEAR		'Y'
#else
#define PRINTER		'I'
#define DISPLAY		'A'
#define FILE_IO		'D'

#define BYMONTH		'M'
#define BYYEAR		'A'
#endif

extern	int	rperror ;
extern	char	e_mesg[80] ;

ststatus(report) 
int	report;
{
	static	char	chardate[11] ;
	static	char	chardate2[11] ;
	static	long	longdate ;
	static 	int	retval;

	St_mast		stmast;
	St_tran		sttran;
	St_sect		st_sect;
	Pa_rec		pa_rec;

	int	code ;
	char 	*arayptr[7] ;
	char 	projname[50] ;
	int 	logrec ;
	int 	formno ;
	int 	outcntl ;
	short	copies ;
	char 	discfile[20] ;
	char	program[11];
	char	resp[2];
		
	int	flag;

/****	
*	Accept Project code, Logical record number in the 
*	Project and format number for the logical record ..
*	Accept the output media code . Open the report ..
****/

	STRCPY( program, "STOCKREP" );
	STRCPY(projname,FMT_PATH) ;
	strcat( projname, INV_PROJECT );
	logrec = LR_STATUS;	/* logical record number */

#ifdef ENGLISH
	STRCPY( e_mesg, "P" );
#else
	STRCPY( e_mesg, "I" );
#endif
	retval = GetOutputon( e_mesg );
	if ( retval<0 || retval==EXIT )
		return( retval );

	switch (*e_mesg) {
		case DISPLAY :	/*  Display on Terminal */
				outcntl = 0 ;
				break;
		case FILE_IO : 	/*  Print to a disk file */ 
				outcntl = 1 ;
				break;
		case PRINTER : 	/*  Print to a printer */ 
		default  :
				outcntl = 2 ;
				break;
	}
		
	if(outcntl == 1) {
		STRCPY( e_mesg, "status.dat");
		retval = GetFilename(e_mesg);
		if( retval<0 || retval==EXIT )
			return(retval);
		STRCPY (discfile, e_mesg) ;
	}
	else
		discfile[0] = '\0' ;

	copies = 1;
	if(outcntl == 2) {
		if((retval = GetNbrCopies( &copies )) < 0)
			return(retval);
	}

	/* set the format number */
	switch( report ){
	    case 1: 		/* Month/Year trial balance */
	      for( ; ; ){
#ifdef ENGLISH
		if( (retval=DisplayMessage(
				 "Stock Status for the Month/Year (M/Y)?")) <0)
#else
		if( (retval=DisplayMessage(
				 "Etat des stocks pour le mois/l'annee (M/A)?")) <0)
#endif
			return(retval);
		if((retval=GetResponse(resp))<0 || retval==EXIT)
			return( retval );
		if( resp[0]==BYMONTH || resp[0]==BYYEAR )
			break;
	      }
	      if( resp[0]==BYMONTH )	formno = FM_MSTAT; 
						/* Stock Status for month */
	      else		formno = FM_YSTAT; 
						/* Stock Status for year */
	      break;
	    case 2:
		formno = FM_BELMIN;		/* Below minimum report */
	    	break;
	    case 3:
		formno = FM_TRBAL;		/* Trial Balance */
	    	break;
	    default:
		break;
	}

	if ( (retval=Confirm())<= 0) 
		return(retval);

	mkdate(get_date(),chardate);

	code=rpopen(projname,logrec,formno,outcntl,discfile,program,chardate);
							  	
	if ( code < 0 ){
		sprintf( e_mesg,"Rpopen code :%d\n", code ) ;
		return(retval);
	}

	if(outcntl == 2) 
		rpSetCopies( (int)copies );	/* number of copies to print */

	if( get_param( &pa_rec, BROWSE, 1, e_mesg )<1 )
		return( DBH_ERR );
	if( (retval=rpChangetitle(1,pa_rec.pa_co_name))<0 ){
		sprintf( e_mesg,"Rp error code :%d\n", code ) ;
		return(REPORT_ERR);
	}
	switch( report ){
		case 1:	/* trial balance for the month */
			if( formno==FM_MSTAT ){
#ifdef ENGLISH
				sprintf( e_mesg, "FOR THE PERIOD %d", 
						pa_rec.pa_cur_period);
#else
				sprintf( e_mesg, "POUR LA PERIODE %d", 
						pa_rec.pa_cur_period);
#endif
				if( (code=rpChangetitle( 3, e_mesg ))<0 ){
				    sprintf(e_mesg,"Rp error code :%d\n",code);
				    return(REPORT_ERR);
				}
			}
			break;
		case 3:	/* Trail Balance */	
			longdate = get_date() ;
			tedit( (char *)&longdate, "____/__/__",
				chardate2, R_LONG );
#ifdef ENGLISH
			sprintf(e_mesg,"AS ON : %s",chardate2 );
#else
			sprintf(e_mesg,"COMME SUR : %s",chardate2 );
#endif
			if( (retval=rpChangetitle(3,e_mesg))<0 ){
				sprintf(e_mesg,"Rp error code :%d\n",retval);
				return(REPORT_ERR);
			}
			break;
		default:
			break;
	}
/***	Read the stock section file ***/
	if( get_section(&st_sect,BROWSE,1,e_mesg)<1 )
		return(DBH_ERR);
/***	Prepare to read stmast sequentially ***/
	
	stmast.st_section = 0;
	stmast.st_fund = 0;
	stmast.st_code[0] = '\0';
	flg_reset( STMAST );

/***	Initialize the pointer array's first element to the record ***/

	arayptr[0] = (char *)&stmast ; 
	arayptr[1] = (char *)&sttran ; 
	arayptr[2] = (char *)&st_sect ; 
	arayptr[3] = NULL;
	
	outcntl = 0;	/* to store previous section val */

/***	Read record and call report writer to output it ***/
	for(;;) {
		code = get_n_stmast(&stmast, BROWSE, 1, FORWARD, e_mesg) ;
		if(code < 0 ){
			if(code==EFL)
				code=0;
			break;
		}
		if( report==2 && stmast.st_on_hand>=stmast.st_min )
			continue;

		if( outcntl!=stmast.st_section ){
			outcntl = stmast.st_section;
			STRCPY( sttran.st_remarks,
				st_sect.name[outcntl-1] );
		}
		if((code=rpline(arayptr))<0){
			if(rperror < 0) {
#ifdef ENGLISH
			sprintf(e_mesg,"Rp error code: %d",code);
#else
			sprintf(e_mesg,"Rp error code: %d",code);
#endif
				code = REPORT_ERR;
			}
			else
				code = NOERROR ;
			break ;
		}
	}     /*  for (;;)   */

	close_dbh() ;
	rpclose() ;
	return(code);
}

