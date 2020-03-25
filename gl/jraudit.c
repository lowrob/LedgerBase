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
#define YES	'Y'
#else
#define YES	'O'
#endif

extern char	e_mesg[80] ;
extern int 	prntmode;

journal(prntmode) 
{
static 	char	chardate[11] ;
char  	trans_type1[2], trans_type2[2], trans_type3[2], trans_type4[2];	 /* transaction type range */
long	seq_no1,seq_no2;
double  debits,credits;
char	str[200];

Tr_hdr 	tr_hdr ; 	/**** 	Declarations for DBH record reading  ****/
Tr_item tr_item ;
Pa_rec	pa_rec ;
int	code ;
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

	date1 = date2 = get_date() ;

	STRCPY(trans_type1, "7");
	STRCPY(trans_type2, "8");
	STRCPY(trans_type3, "A");
	STRCPY(trans_type4, "R");

	seq_no1=0L;
	seq_no2 = 99999999L;

	mkdate(get_date(), chardate) ;

	code = rpopen(projname,logrec,formno,2,discfile, 
		program,chardate);
							  	
	if ( code < 0 ){
		sprintf(e_mesg, "Rpopen code :%d\n", code ) ;
		fomen(e_mesg);
		get();
		return(-1);
	}

	copies = 1;
	rpSetCopies( (int)copies );	/* number of copies to print */

 	rpChangetitle(1, pa_rec.pa_co_name); 
	mkdate(date1, chardate1) ;
	mkdate(date2, chardate2) ;
if (prntmode > 0){
#ifdef ENGLISH
	sprintf(e_mesg,"JOURNAL ENTRIES AUDIT REPORT for %s to %s",chardate1,chardate2) ;
#else
	sprintf(e_mesg,"LISTE DES ENTREES DE JOURNAL DE %s A %s",chardate1,chardate2) ;
#endif
}
else {

#ifdef ENGLISH
	sprintf(e_mesg,"ACCRUAL ENTRIES AUDIT REPORT for %s to %s",chardate1,chardate2) ;
#else
	sprintf(e_mesg,"LISTE DES ENTREES DE JOURNAL DE %s A %s",chardate1,chardate2) ;
#endif
}
 	rpChangetitle(2, e_mesg); 

	STRCPY(tr_hdr.th_userid,User_Id);

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
		code = get_n_trhdr(&tr_hdr, UPDATE, 1, 0, e_mesg) ;
		if(code < 0) break ;
		if (tr_hdr.th_date > date2) 
			break;;

		if(strcmp(tr_hdr.th_userid,User_Id) != 0)  {
			roll_back(e_mesg);
			continue;
		}
		/* Only accept tr_hdr.th_create equal to E */
		if (tr_hdr.th_create[0] != 'E') {
			roll_back(e_mesg);
			continue;
		}
		if(tr_hdr.th_print[0] == YES) {
			roll_back(e_mesg);
			continue;
		} 
		/* Check that tr_hdr.th_type within range specified (CL) */ 

	if(prntmode > 0){

		if (strcmp(tr_hdr.th_type, trans_type1) < 0) {
			roll_back(e_mesg);
			continue;
		}
		if (strcmp(trans_type2, tr_hdr.th_type) < 0) {
			roll_back(e_mesg);
			continue;
		}
	}
	else	{

		if (strcmp(trans_type3, tr_hdr.th_type) == 0 ||
		strcmp(trans_type4, tr_hdr.th_type) == 0) {
			roll_back(e_mesg);
			continue;
		}
	/*	if (strcmp(trans_type4, tr_hdr.th_type) == 0) {
			roll_back(e_mesg);
			continue;
		}*/
	}
		if ((seq_no2 < tr_hdr.th_seq_no)||(seq_no1 > tr_hdr.th_seq_no)){
			roll_back(e_mesg);
			continue;
		}
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

		tr_hdr.th_print[0] = YES;
		tr_hdr.th_print[1] = '\0';
		code = put_trhdr(&tr_hdr, UPDATE, e_mesg) ;
		if(code < 0) break ;

		if((code = commit(e_mesg))<0) 
			break;
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

	if(code < 0 && code != EFL) 
		return(code);

	return(0);
}
