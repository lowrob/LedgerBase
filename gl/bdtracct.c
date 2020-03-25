/*
*	Source 	: bdtrrep.c 
*
*	Program to Print Budget transaction Details using REPORT GENERATOR.
*      
*       Author        : Robert Masson
*       Creation Date : 11/20/1990
*/
/***
#define	MAIN 
***/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	PROJNAME	"bdtrrep"
#define	LOG_REC		1
#define	FORMAT_NO	2

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

static	char	e_mesg[100] ;

bdtracct()
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
	short   fund1, fund2;
	char    acct1[19], acct2[19];

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
		STRCPY(discfile,"bdtracct.dat");
		if( GetFilename( discfile )<0 )
			return(-1);
	}
        /* * */
        /* next line modification added 90/09/27  -date range */
	date1 = date2 = pa_rec.pa_date;
	fund1 = 1;
        fund2 = 999;
        STRCPY (acct1,"                 1");
        STRCPY (acct2,"999999999999999999");
        GetDateRange(&date1, &date2);
        GetFundRange(&fund1, &fund2);
        GetAcctRange(acct1, acct2);
        /* * */
	code = Confirm();
	if( code<0 )
		return(-1);
	if( code==0 )
		return(0);

	mkdate(get_date(),chardate);

	STRCPY(projname,FMT_PATH) ;
	strcat(projname,PROJNAME) ;
	STRCPY(program, "BDTRACCT" );

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

        STRCPY(bd_item.tr_accno, acct1);
	bd_item.tr_fund = fund1;
	bd_item.tr_reccod = 0;
	flg_reset( GLBDITEM );

	for( ; ; ) {
		code = get_n_bditem(&bd_item, BROWSE, 2, FORWARD, e_mesg);
		if( code < 0) {
			if(code == EFL) break ;
			printf("%s\n",e_mesg);
			break ;
		}
               /*   */
               /* the following lines, of modification for */
               /* inclusion of a date ranged report */
               /*   */
                if( bd_item.tr_sys_dt < date1 ) 
                    continue;
                if( bd_item.tr_sys_dt > date2 ) 
                   continue;

                if( bd_item.tr_fund < fund1 ) 
                    continue;
                if( bd_item.tr_fund > fund2 ) 
                   continue;
  
                code = strcmp(bd_item.tr_accno, acct1);
                if( code < 0 ) 
                    continue;
                code = strcmp(bd_item.tr_accno, acct2);
                if( code > 0 ) 
                   continue;
                
                /* */
                /* * * end of modification */
		if(rpline(arayptr) < 0) break ;

	}

	close_dbh() ;
	rpclose() ;
	return(0);
}
