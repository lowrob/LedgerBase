/******************************************************************************
		Sourcename    : alloclst.c
		System        : Budgetary Financial system.
		Module        : Inventory reports: Allocations listing
		Created on    : 89-09-19
		Created  By   : K HARISH.
		Cobol sources : 
*******************************************************************************
About this file:
	This routine generates one of the following :
		1. Listing of stock allocations for a given location/school.
		2. Listing of stock allocations for a given stock code.
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

#define NO		'N'
#define COSTCENTER	'C'
#define STCODE		'S'
#else
#define PRINTER		'I'
#define DISPLAY		'A'
#define FILE_IO		'D'

#define NO		'N'
#define COSTCENTER	'C'
#define STCODE		'S'
#endif

extern	int	rperror;
extern	char	e_mesg[80] ;

alloclst() 
{
	static	char	chardate[11] ;
	static	char	chardate2[11] ;
	static	long	longdate ;
	static 	int	retval;
	static	short	fund1, fund2;
	static	char	stcode1[11], stcode2[11];
	static	short	loc1, loc2;
	int	key_init;

	St_mast		stmast;
	Alloc_rec	st_alloc;
	Sch_rec		school;
	Pa_rec		pa_rec;

	int	code ;
	char 	*arayptr[7], *ptr;
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

	if( (retval=get_param(&pa_rec,BROWSE,1,e_mesg))<1 )
		return(retval);

	STRCPY( program, "STOCKREP" );
	STRCPY(projname,FMT_PATH) ;
	strcat( projname, "stockrep" );
	logrec = LR_ALLOC;	/* logical record for allocation listing */

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
		STRCPY( e_mesg, "alloclst.dat");
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

#ifdef ENGLISH
	retval = DisplayMessage("List for Cost-Center/Stock-code Range (C/S)?");
#else
	retval = DisplayMessage("Liste pour les gammes de centre de couts/code de stock (C/S)?");
#endif
	if( retval<0 )
		return(retval);
	resp[0]=NO;
	while( *resp!=COSTCENTER && *resp!=STCODE )
		if( (retval=GetResponse(resp))<0 )
			return(retval);
	if( *resp==COSTCENTER ){	/* list on location */
		loc1 = 1; loc2 = 9999;
		fund1 = 1;
		fund2 = 999;
		STRCPY(stcode1,""); STRCPY(stcode2,"ZZZZZZZZZZ");

		if( (retval=GetLocRange(&loc1,&loc2))<0 )
			return(retval);
		if( (retval=GetFundRange(&fund1,&fund2))<0 )
			return(retval);
		if( (retval=GetCodeRange(stcode1,stcode2))<0 )
			return(retval);
		formno = FM_AL_LOC;	/* format# for location wise list */
	}
	else{		/* list on stock code */
		fund1 = 1;
		fund2 = 999;
		STRCPY(stcode1,""); STRCPY(stcode2,"ZZZZZZZZZZ");

		if( (retval=GetFundRange(&fund1,&fund2))<0 )
			return(retval);
		if( (retval=GetCodeRange(stcode1,stcode2))<0 )
			return(retval);
		formno = FM_AL_COD;	/* format# for code wise listing */
	}
	if ( (retval=Confirm())<= 0) 
		return(retval);

	mkdate(get_date(),chardate);

	code=rpopen(projname,logrec,formno,outcntl,discfile,program,chardate);
							  	
	if ( code < 0 ){
		sprintf( e_mesg,"Rpopen code :%d\n", code ) ;
		return( code );
	}

	if(outcntl == 2) 
		rpSetCopies( (int)copies );	/* number of copies to print */

	if( ( retval = rpChangetitle(1,pa_rec.pa_co_name) )<0 ){
		sprintf(e_mesg,"Rp error: %d", retval );
		return(retval);
	}
	longdate = get_date() ;
	tedit( (char *)&longdate,"____/__/__",chardate2,R_LONG );
#ifdef ENGLISH
	sprintf(e_mesg,"As on: %s", chardate2);
#else
	sprintf(e_mesg,"Comme sur: %s", chardate2);
#endif
	if( ( retval = rpChangetitle(3,e_mesg) )<0 ){
		sprintf(e_mesg,"Rp error: %d", retval );
		return(retval);
	}
/***	Prepare to read allocation file sequentially ***/
	if( *resp==COSTCENTER ){	/* Listing for a given location range */
		st_alloc.st_location = loc1;
		st_alloc.st_fund = fund1;
		STRCPY(st_alloc.st_code,stcode1);
		flg_reset( ALLOCATION );
		key_init = 0;
		flag = 2;	/* 2nd alternate key */
	}
	else{		/* Listing for a given stock range */
		st_alloc.st_fund = fund1;
		STRCPY(st_alloc.st_code,stcode1);
		st_alloc.st_date = 0;
		st_alloc.st_time = 0;
		flg_reset( ALLOCATION );
		key_init = 0;
		flag = 1;	/* alternate key 1 */
	}

/***	Initialize the pointer array's first element to the record ***/

	arayptr[0] = (char *)&stmast ; 
	arayptr[1] = (char *)&st_alloc ; 
	arayptr[2] = (char *)&school ;
	arayptr[3] = NULL;
	
/***	Read record and call report writer to output it ***/
	for(;;) {
		if( key_init==1 ){
			flg_reset(ALLOCATION);
			key_init = 0;
		}
		code = get_n_alloc(&st_alloc, BROWSE, flag, FORWARD, e_mesg) ;
		if(code < 0) break ;

		if(flag==1){	/* list on stock range */
			if( st_alloc.st_fund>fund2 )
				break;
			if( strcmp(st_alloc.st_code,stcode1)<0 ||
			    strcmp(st_alloc.st_code,stcode2)>0 ){
				if( strcmp(st_alloc.st_code,stcode2)>0 )
					st_alloc.st_fund++;
				STRCPY(st_alloc.st_code,stcode1);
				st_alloc.st_location = 0;
				st_alloc.st_expacc[0] = '\0';
				key_init = 1;
				continue;
			}
		}
		else{		/* list on location range */
			if( st_alloc.st_location>loc2 )
				break;
			if( st_alloc.st_fund<fund1 || st_alloc.st_fund>fund2 ){
				if( st_alloc.st_fund>fund2 )
					st_alloc.st_location++;
				st_alloc.st_fund = fund1;
				STRCPY( st_alloc.st_code, stcode1 );
				key_init = 1;
				continue;
			}
			if( strcmp(st_alloc.st_code,stcode1)<0 ||
			    strcmp(st_alloc.st_code,stcode2)>0 ){
				if( strcmp(st_alloc.st_code,stcode2)>0 )
					st_alloc.st_fund++;
				STRCPY(st_alloc.st_code,stcode1);
				key_init = 1;
				continue;
			}
		}
		stmast.st_fund=st_alloc.st_fund;
		STRCPY(stmast.st_code,st_alloc.st_code);
		if( get_stmast(&stmast,BROWSE,0,e_mesg)<0 ){
			STRCPY(stmast.st_desc,"?????");
			STRCPY(stmast.st_unit,"?????");
		}
		school.sc_numb=st_alloc.st_location;
		if( get_sch(&school,BROWSE,0,e_mesg)<0 ){
			STRCPY(school.sc_name,"?????");
		}
		if(rpline(arayptr) < 0) {
			if(rperror < 0) {
#ifdef ENGLISH
			sprintf(e_mesg,"Internal report error");
#else
			sprintf(e_mesg,"Erreur au rapport interne");
#endif
				code = REPORT_ERR;
			}
			else
				code = NOERROR ;
			break ;
		}
	}     /*   for(;;)   */

	if( code==EFL )	code=0;
	close_dbh() ;
	rpclose() ;
	return(retval);
}

