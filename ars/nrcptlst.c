
/****
*	Non Receipts Listing ...
*	Source 	: nrecplst.c 
*	Modifications : rpopen has more paramaters for new rp library
****/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	EXIT	12

#ifdef ENGLISH
#define PRINTER 	'P'
#define DISPLAY		'D'
#define FILE_IO		'F'
#else
#define PRINTER		'I'
#define DISPLAY		'A'
#define FILE_IO		'D'
#endif

non_receipts() 

{
static 	char	chardate[11] ;

Tr_hdr 	tr_hdr ; 	/**** 	Declarations for DBH record reading  ****/
Tr_item tr_item ;
Pa_rec	pa_rec ;
int	code ;
char	e_mesg[80] ;
char 	*arayptr[5] ; 	/**** 	Declarations for Report writer usage ****/
char 	projname[50] ;
int 	logrec ;
int 	formno ;
int 	outcntl ;
short	copies ;
char 	discfile[20] ;
char	program[11];
int	retval;
long	date1, date2 ;
char	chardate1[11], chardate2[11] ;
	

/****	
*	Accept Project code, Logical record number in the 
*	Project and format number for the logical record ..
*	Accept the output media code . Open the report ..
****/
	STRCPY(program,"NRCPTLST");

	STRCPY(projname,FMT_PATH) ;
	strcat(projname, "journal");
	logrec = 1;
	formno = 1;

	code = get_param( &pa_rec, BROWSE, 1, e_mesg);
	if (code < 1) {
		printf("%s\n", e_mesg);
		close_dbh();
		return(-1);
	}

#ifdef ENGLISH
	STRCPY(e_mesg, "P");
#else
	STRCPY(e_mesg, "I");
#endif
	retval = GetOutputon(e_mesg);
	if (retval < 0) return(-1);
	else if (retval == EXIT) return(0);

	switch (*e_mesg) {
		case DISPLAY :	/*  Display on the terminal */
			outcntl = 0;
			break;
		case FILE_IO :	/*  Print to a file */
			outcntl = 1;
			break;
		case PRINTER :	/*  Print to the printer */
			outcntl = 2;
			break;
		default:
			outcntl = 2;
			break;
	}

	if(outcntl == 1)
		{
		STRCPY(e_mesg, "journal.dat");
		if(GetFilename( e_mesg ) < 0) return(-1);
		STRCPY(discfile, e_mesg);
	}
	else
		discfile[0] = '\0' ;
	
	copies = 1;
	if(outcntl == 2) {
		if((retval == GetNbrCopies( &copies )) < 0)
			return(retval);
	}

	date1 = date2 = get_date() ;
	retval = GetTransDateRange(&date1, &date2);
	if (retval < 0 ) return(-1);
	else if (retval == EXIT) return(0);

	if ((retval = Confirm()) < 0) return(-1);
	else if (!retval) return(0);

	mkdate(get_date(), chardate) ;

	code = rpopen(projname,logrec,formno,outcntl,discfile, 
		program,chardate);
							  	
	if ( code < 0 ){
		sprintf(e_mesg, "Rpopen code :%d\n", code ) ;
		fomen(e_mesg);
		get();
		return(-1);
	}

	if(outcntl == 2)
		rpSetCopies( (int)copies );	/* number of copies to print */

 	rpChangetitle(1, pa_rec.pa_co_name); 
	mkdate(date1, chardate1) ;
	mkdate(date2, chardate2) ;
#ifdef ENGLISH
	sprintf(e_mesg,"NON RECEIPTS FROM %s TO %s", chardate1, chardate2) ;
#else
	sprintf(e_mesg,"NON-RECUS DE %s A %s", chardate1, chardate2) ;
#endif
 	rpChangetitle(2, e_mesg); 

	/* For Terminals set pagesize to 23 lines */
	if(outcntl == 0)
		rpPagesize(23);


/***	Prepare to read transaction sequentialy ***/

	tr_hdr.th_sys_dt = date1;
	tr_hdr.th_fund = 0;
	tr_hdr.th_reccod = 0;
	tr_hdr.th_create[0] = 'E';
	tr_hdr.th_seq_no = 0;
	if (GetUserClass(e_mesg) == ORD_USER)
		STRCPY(tr_hdr.th_userid, User_Id);
	else	tr_hdr.th_userid[0] = '\0' ;
	
	flg_reset(GLTRHDR);

/***	Initialise the pointer array's first element to the record ***/

	arayptr[0] = (char *)&tr_hdr ; 
	arayptr[1] = (char *)&tr_item ; 
	arayptr[2] = NULL ;

/***	Read record and call report writer to output it ***/

	for(;;) {
		code = get_n_trhdr(&tr_hdr, BROWSE, 1, 0, e_mesg) ;
		if(code < 0) break ;


		if (GetUserClass(e_mesg) == ORD_USER)
			if (strcmp(tr_hdr.th_userid,User_Id) != 0) continue ;

		if (tr_hdr.th_type[0] != 'N') continue ;

		if (tr_hdr.th_date < date1 || tr_hdr.th_date > date2) 
			continue;

		tr_item.ti_fund = tr_hdr.th_fund ;
		tr_item.ti_reccod = tr_hdr.th_reccod;
		tr_item.ti_create[0] = 'E';
		tr_item.ti_seq_no = tr_hdr.th_seq_no ;
		tr_item.ti_item_no = 0 ;
		flg_reset(GLTRAN);
 		for (;;) {
#ifndef ORACLE
			code = get_n_tritem(&tr_item,BROWSE,0,FORWARD,e_mesg) ;
#else
			code = get_n_tritem(&tr_item,BROWSE,0,EQUAL,e_mesg) ;
#endif
			if(code < 0) break ;
#ifndef ORACLE
			if (tr_item.ti_fund != tr_hdr.th_fund ||
			   tr_item.ti_reccod != tr_hdr.th_reccod ||
			   tr_item.ti_create[0] != 'E' ||
			   tr_item.ti_seq_no != tr_hdr.th_seq_no)
				break ;
#endif
			if(rpline(arayptr) < 0) return(NOERROR); /*break ;*/
		}
	}


/****	Windup .. ****/

	close_dbh() ;
	rpclose() ;
	return(0);
}

