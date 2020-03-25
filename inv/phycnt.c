/******************************************************************************
		Sourcename    : phycnt.c
		System        : Budgetary Financial system.
		Module        : Inventory reports
		Created on    : 89-09-19
		Created  By   : K HARISH.
		Cobol sources : 
*******************************************************************************
About this file:
	This routine generates one of the following reports:
		1. Physical count worksheet for noting stock physical count.
		2. Physical count report
	Calling file:	stockrep.c
******************************************************************************/
#define	EXIT	12		/* as defined in streputl.c */

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>
#include <bfs_inv.h>

#ifdef ENGLISH
#define PRINTER		'P'
#define DISPLAY		'D'
#define FILE_IO		'F'

#define YES		'Y'
#define NO		'N'
#else
#define PRINTER		'I'
#define DISPLAY		'A'
#define FILE_IO		'D'

#define YES		'O'
#define NO		'N'
#endif

phycnt(type) 
int	type;	/* 1: Worksheet    2: Report */
{
	static	char	chardate[11] ;
	static	char	chardate2[11] ;
	static	long	longdate ;
	static 	int	retval, mode;
	static 	char 	sum[1];

	St_mast		stmast;
	St_tran		sttran;
	St_sect		st_sect;
	Pa_rec		pa_rec;

	int	code ;
	char	e_mesg[80] ;
	char 	*arayptr[7] , *ptr;
	char 	projname[50] ;
	int 	logrec ;
	int 	formno ;
	int 	outcntl ;
	short	copies ;
	char 	discfile[20] ;
	char	program[11];
	char	resp[2];
		
	int	init_flag, part_flag;

/****	
*	Accept Project code, Logical record number in the 
*	Project and format number for the logical record ..
*	Accept the output media code . Open the report ..
****/

	STRCPY( program, "STOCKREP" );
	STRCPY(projname,FMT_PATH) ;
	strcat( projname, INV_PROJECT );
	logrec = LR_PCREP;	/* Logical record for phy cnt reports */
	if( type==1 )		formno = FM_PCWS; /* Phy cnt worksheet format#*/
	else			formno = FM_PCREP;/* Phy cnt report format# */

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
		if( type==1 )
			STRCPY( e_mesg, "pcws.dat");
		else	/* type==2 */
			STRCPY( e_mesg, "pcrep.dat");
		retval = GetFilename(e_mesg);
		if( retval<0 || retval==EXIT )
			return(retval);
		STRCPY (discfile, e_mesg) ;
	}
	else
		discfile[0] = '\0' ;

	init_flag = 0;	/* default: don't initialize */

	copies = 1;
	if(outcntl == 2) {
		if((retval = GetNbrCopies( &copies )) < 0)
			return(retval);
	}

	if( type==1 ){
	  if( (retval=CheckAccess(UPDATE,e_mesg))<0 )/* Main file already set */
	    init_flag = 0;
	  else{
	    for( ; ; ){
		if (type == 1){
#ifdef ENGLISH
			if( DisplayMessage( "Print Summary Worksheet (Y/N)?") <0)
				return(-1);
#else
			if( DisplayMessage( "Imprimer feuille de travail SOMMAIRE (O/N)?") <0)
				return(-1);
#endif
			if((retval=GetResponse(resp))<0 || retval==EXIT)
				return( retval );
			if( *resp!=YES && *resp!=NO )
				continue;
			else{
				if (*resp == YES)
					formno = 7;
				else
					formno = FM_PCWS; 
			}
		}

		if (type == 1){
#ifdef ENGLISH
			if( DisplayMessage( "Print On Hand Column (Y/N)?") <0)
				return(-1);
#else
			if( DisplayMessage( "Imprimer colonne En maison (O/N)?") <0)
				return(-1);
#endif
			if((retval=GetResponse(resp))<0 || retval==EXIT)
				return( retval );
			if( *resp!=YES && *resp!=NO )
				continue;
			else{
				if (*resp == YES) {
					if (formno == 7) {
						logrec = 4;
						formno = 2;  /* Summ on hand */
					}
					else {
						logrec = 4;
						formno = 1;  /* Dtl on hand */
					}
				}
			}
		}
#ifdef ENGLISH
		if( DisplayMessage(
		 "Initialize before and after count fields to stock on hand (Y/N)?")
		  <0)
			return(-1);
#else
		if( DisplayMessage(
		 "Initialiser au stock en maison les champs de compte d'avant et d'apres (O/N)?")
		  <0)
			return(-1);
#endif
		if((retval=GetResponse(resp))<0 || retval==EXIT)
			return( retval );
		if( *resp!=YES && *resp!=NO )
			continue;
		else
			break;
	    }
	    if( *resp==YES )	init_flag = 1;
	  }
	}

	part_flag = 0;	/* default: print all records */
	if( init_flag==0 ){
	    for( ; ; ){
#ifdef ENGLISH
		if( DisplayMessage(
		"Print record even if before and after count qty is same (Y/N)?")
		  <0)
			return(-1);
#else
		if( DisplayMessage(
		"Imprimer fiche meme si quant. comptees avant et apres sont identiques (O/N)?")
		  <0)
			return(-1);
#endif
		if((retval=GetResponse(resp))<0 || retval==EXIT)
			return( retval );
		if( *resp!=YES && *resp!=NO )
			continue;
		else
			break;
	    }
	    if( *resp==NO )	part_flag = 1;
	}

	if ((retval = Confirm()) <= 0) 
		return(retval);
	if( get_param(&pa_rec,BROWSE,1,e_mesg)<1 ){
		printf(e_mesg);
		getchar();
		return(-1);
	}

	mkdate(get_date(),chardate);

	code = rpopen(projname,logrec,formno,outcntl,discfile, 
		program,chardate);
							  	
	if ( code < 0 ){
		sprintf( e_mesg,"Rpopen code :%d\n", code ) ;
		fomen(e_mesg);
		get();
		exit(-1);
	}

	if(outcntl == 2) 
		rpSetCopies( (int)copies );	/* number of copies to print */

	if( rpChangetitle(1,pa_rec.pa_co_name)<0 ){
		printf(e_mesg);
		getchar();
		return(-1);
	}
/***	Read the stock section file ***/
	if( get_section(&st_sect,BROWSE,1,e_mesg)<1 )
		return(-1);
/***	Prepare to read stmast sequentialy ***/
	
	stmast.st_section = 0;
	stmast.st_fund = 0;
	stmast.st_code[0] = '\0';
	flg_reset( STMAST );

/***	Initialize the pointer array's first element to the record ***/

	arayptr[0] = (char *)&stmast ; 
	arayptr[1] = (char *)&sttran ; 
	arayptr[2] = (char *)&st_sect ; 
	arayptr[3] = NULL;
	
	mode = ( init_flag==1 ) ? UPDATE : BROWSE ;

	outcntl = stmast.st_section;	/* store section # */
/***	Read record and call report writer to output it ***/
	for(;;) {
		code = get_n_stmast(&stmast, mode, 1, FORWARD, e_mesg) ;
		if(code < 0) break ;

		/* if the fields in master aren't being initialized and
		   report is required only on quantities where stock bef count
		   and stock after count differ, skip the record if these two
		   values are same
		*/
		if( init_flag==0 && part_flag==1 &&
		    stmast.st_bef_cnt==stmast.st_aft_cnt )
			continue;

		/* if before and after count quantities are to be updated */
		if( init_flag==1 )
		   stmast.st_bef_cnt = stmast.st_aft_cnt = stmast.st_on_hand;

		if( outcntl!=stmast.st_section ){
			outcntl = stmast.st_section;
			strncpy( sttran.st_remarks, st_sect.name[outcntl-1],
				sizeof(sttran.st_remarks) );
			sttran.st_remarks[sizeof(sttran.st_remarks)-1] = '\0';
		}
		if( mode==UPDATE )
			if( (code=put_stmast(&stmast, mode, e_mesg )<0) ){
				printf(e_mesg);
				getchar();
				return(-1);
			}
		if( type==1 )	/* physical count worksheet: don't show value */
			STRCPY(stmast.st_accno,"____________");

		if(rpline(arayptr) < 0) break ;
		if( mode==UPDATE ){
			/* increment stock code for next read */
			if( (code=commit(e_mesg))<0 )
				break;
			if( inc_str( stmast.st_code, 10, FORWARD )<0 )
				return(-1);
			flg_reset( STMAST );
		}
	}
/****	Windup .. ****/

	if( code<0 && code!=EFL ){
		printf( "\n%s\n",e_mesg );
		getchar();
	}
	else 
		code=0;
	close_dbh() ;
	rpclose() ;
	return(code);
}

