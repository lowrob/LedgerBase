/****
*	Journal Listing ...
*	Source 	: journal.c 
*	Modifications : rpopen has more paramaters for new rp library
*	MODIFIED: JUNE 20, 1990
*	DESC	: change for new journal entry numbering
*
* 	MODIFICATIONS:
*
*	Programmer	YY/MM/DD	Description
*	~~~~~~~~~~	~~~~~~~~	~~~~~~~~~~~
*	C.Leadbeater	90/11/29	Allow range for transaction type
*					and include items with th_create with
*					'E'.
*
*	M. Cormier	90/11/30	Make the Quit user reponse function
*					properly.
****/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	EXIT	12

#ifdef ENGLISH
#define PRINTER 'P'
#define DISPLAY 'D'
#define FILE_IO	'F'
#else
#define PRINTER 'I'
#define DISPLAY 'A'
#define FILE_IO	'D'
#endif

journal() 

{
static 	char	chardate[11] ;
char  	trans_type1[2], trans_type2[2];	 /* transaction type range */
long	seq_no1,seq_no2;
double  debits,credits;
char	str[200];

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
int	userclass;
extern	int	rperror ;

/****	
*	Accept Project code, Logical record number in the 
*	Project and format number for the logical record ..
*	Accept the output media code . Open the report ..
****/
#ifdef ENGLISH
	STRCPY(program,"JOURNAL");
#else
	STRCPY(program,"JOURNAL");
#endif

	STRCPY(projname,FMT_PATH) ;
	strcat(projname, "journal");
	logrec = 1;
	formno = 1;
	debits = credits = 0.0;

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
	retval = GetDateRange(&date1, &date2);
	if (retval < 0 ) return(-1);
	else if (retval == EXIT) return(0);

		/* Accept transaction type range from user  (CL) */

	STRCPY(trans_type1, "1");
	STRCPY(trans_type2, "Z");
	retval = GetTransRange( trans_type1, trans_type2);
 	if (retval < 0) return(-1);
	else if (retval == EXIT) return(0);	

	seq_no1=0L;
	seq_no2 = 99999999L;
	retval = GetSeqRange( &seq_no1, &seq_no2);
 	if (retval < 0) return(-1);
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
	sprintf(e_mesg,"LIST OF JOURNAL ENTRIES FROM %s TO %s",chardate1,chardate2) ;
#else
	sprintf(e_mesg,"LISTE DES ENTREES DE JOURNAL DE %s A %s",chardate1,chardate2) ;
#endif
 	rpChangetitle(2, e_mesg); 

	/* For Terminals set pagesize to 23 lines */
	if(outcntl == 0)
		rpPagesize(23);


	userclass = GetUserClass(e_mesg);
	if(userclass == ORD_USER)
		STRCPY(tr_hdr.th_userid,User_Id);
	else
		tr_hdr.th_userid[0] = '\0';

/***	Prepare to read transaction sequentialy ***/

	tr_hdr.th_sys_dt = date1;
	tr_hdr.th_fund = 0;
	tr_hdr.th_reccod = 0;
	tr_hdr.th_create[0] = '\0';
	tr_hdr.th_seq_no = seq_no1;
	flg_reset(GLTRHDR);

/***	Initialise the pointer array's first element to the record ***/

	arayptr[0] = (char *)&tr_hdr ; 
	arayptr[1] = (char *)&tr_item ; 
	arayptr[2] = NULL ;

/***	Read record and call report writer to output it ***/

	for(;;) {
		code = get_n_trhdr(&tr_hdr, BROWSE, 1, 0, e_mesg) ;
		if(code < 0) break ;
		if (tr_hdr.th_date > date2) 
			break;;

		if (userclass == ORD_USER && 
				strcmp(tr_hdr.th_userid,User_Id) != 0) 
			continue;

		/* Only accept tr_hdr.th_create equal to E */
		if (tr_hdr.th_create[0] != 'E')
			continue;

		/* Check that tr_hdr.th_type within range specified (CL) */ 

		if (strcmp(tr_hdr.th_type, trans_type1) < 0) continue;
		if (strcmp(trans_type2, tr_hdr.th_type) < 0) continue;
		if ((seq_no2 < tr_hdr.th_seq_no)||(seq_no1 > tr_hdr.th_seq_no))
					 continue;
		credits += tr_hdr.th_credits;		
		debits += tr_hdr.th_debits;		

		tr_item.ti_fund = tr_hdr.th_fund;
		tr_item.ti_reccod = tr_hdr.th_reccod;
		tr_item.ti_create[0] = tr_hdr.th_create[0];
		tr_item.ti_seq_no = tr_hdr.th_seq_no ;
		tr_item.ti_item_no = 0 ;
		flg_reset(GLTRAN);

 		for (;;) {
#ifndef	ORACLE
			code = get_n_tritem(&tr_item,BROWSE,0,FORWARD,e_mesg) ;
#else
			code = get_n_tritem(&tr_item,BROWSE,0,EQUAL,e_mesg) ;
#endif
			if(code < 0) break ;
#ifndef	ORACLE
			if (tr_item.ti_fund != tr_hdr.th_fund ||
			   tr_item.ti_reccod != tr_hdr.th_reccod ||
			   tr_item.ti_create[0] != tr_hdr.th_create[0] ||
			   tr_item.ti_seq_no != tr_hdr.th_seq_no)
				break ;
#endif
			if(rpline(arayptr) < 0)  {
				if(rperror < 0)  {
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
		if((code < NOERROR || code == EXIT) && code != EFL) break;
		seq_over(GLTRAN) ;
	}
	memset(str,' ',sizeof(str));

#ifdef ENGLISH
	sprintf(&str[40],"Total Debits %.2lf   Total Credits  %.2lf\n",debits,
								credits);
#else
	sprintf(&str[40],"Total des Debits %.2lf   Total des Credits  %.2lf\n",
				debits,credits);
#endif

/****	Windup .. ****/
	close_dbh() ;
	rpclose_mesg(str) ;
	return(0);
}
