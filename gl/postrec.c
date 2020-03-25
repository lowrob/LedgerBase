/******************************************************************************
		Sourcename    : postrec.c
		System        : Budgetary Financial system.
		Module        : Posting Recurring Entries at Closing Time
		Created on    : 89-07-18
		Created  By   : K HARISH.
		Cobol sources : 
********************************************************************************

About the File:
	It comprises of the function PostRecurringEntries() which is called
	by the  day/ month/ year closing processes.

	This routine is for posting recurring entries at the 
		1. END OF DAY
		2. END OF PERIOD
	It reads recurring entries header and items files and makes 
	postings depending on the process.


HISTORY:
 Date           Programmer     Description of modification
____/__/__      __________     ___________________________
1990/12/18	J.Prescott	Added D_Roundoff to fix precision problem.
1991/02/15	J.Prescott	Changed The way posting occurs.
				Posting now takes place when the date or
				period matches. Then Increments the date
				and/or period the the next post date.
				In this situation the period or date will
				reflect when the next posting will take place.
******************************************************************************/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define DELTA_AMT	0.0001
#define HIGH_LONG	32767

#ifdef ENGLISH
#define DAILY		'D'
#define WEEKLY		'W'
#define BIWEEKLY	'B'
#define MONTHLY		'M'
#define QUARTERLY	'Q'
#define HALFYEARLY	'H'
#define YEARLY		'Y'
#else
#define DAILY		'Q'
#define WEEKLY		'H'
#define BIWEEKLY	'B'
#define MONTHLY		'M'
#define QUARTERLY	'T'
#define HALFYEARLY	'S'
#define YEARLY		'A'
#endif

static	char e_mesg[80];
static	Pa_rec	parec;
static	Re_hdr	rehdr;
static	Re_item	reitem;
static	Tr_hdr	trhdr;
static	Tr_item	tritem;
static	Gl_rec	glrec;
static	long	sysdt ;

double	D_Roundoff();

PostRecurringEntries()	/* main process */
{
	int retval;

	/* Get the parameter record for the current period & other details */
	if ( get_param(&parec, BROWSE, 1, e_mesg) < 0 ) {
		printf(e_mesg) ;
		return(-1) ;
	}

	if( parec.pa_cur_period==0 || parec.pa_cur_period==13 )
		return(0);

	sysdt = get_date() ;

	if( InitRecurKey()<0 )	/* initialize rehdr record key */
		return(0);
	for( ; ; ){	/* For each header record in recurring hdr file */

		retval = get_n_rehdr( &rehdr,BROWSE,0,FORWARD,e_mesg );
		if( retval== EFL )	/* No more records, so quit */
			break;
		if( retval != NOERROR ){
			printf( e_mesg );
			return( retval );
		}
		if( rehdr.rh_date > sysdt )
			continue;

		/* Check if this record is to be posted */
		if(rehdr.rh_date == sysdt)  {
			if( PostJournalEntries()<0 )
				return(-1);
		}
		rehdr.rh_sno++;
	}
	close_file(RECHDR) ;
	close_file(RECTRAN) ;
	close_file(GLTRHDR) ;
	close_file(GLTRAN) ;
	close_file(GLMAST) ;
	return(0);
}

static
InitRecurKey()	/* Initialize the recurring entry header record key */
{
	rehdr.rh_fund = 0;
	rehdr.rh_sno = 0;
	flg_reset( RECHDR );
	return(0);
}

static
PostJournalEntries()	/* Write journal entries through gltrhdr, gltritems,
			   and update the glmaster file */
{
	if( PostHdr()<0 ){		/* Re_hdr to Tr_hdr to file */
		roll_back(e_mesg);
		return(-1);
	}
	if( PostItem()<0 ){		/* Re_item to Tr_item to file */
					/* Update glmaster also */
		roll_back(e_mesg);
		return(-1);
	}
	if( UpdtRehdr()<0 ){	/* Write the last date of change in Re_hdr */
		roll_back(e_mesg);
		return(-1);
	}
	if( commit(e_mesg)<0 ){	/* If everything goes right, commit */
		printf(e_mesg);
		return(-1);
	}
	return(0);
}

static
PostHdr()	/* Write gltrhdr from rehdr */
{
	int retval;

	if( MakeHeaderKey()<0 )
		return( -1 );
	STRCPY(trhdr.th_userid,User_Id);
	trhdr.th_sys_dt = sysdt;
	trhdr.th_period = parec.pa_cur_period;
	trhdr.th_date = trhdr.th_sys_dt;
	STRCPY( trhdr.th_descr, rehdr.rh_descr );
	trhdr.th_supp_cd[0] = '\0';
	sprintf(trhdr.th_reference,"%ld-%d-%c",trhdr.th_sys_dt,rehdr.rh_sno,
			rehdr.rh_type[0]); 
	trhdr.th_type[0] = rehdr.rh_type[0];
	trhdr.th_debits = rehdr.rh_debits;
	trhdr.th_credits = rehdr.rh_credits;

	retval = put_trhdr( &trhdr, ADD, e_mesg );
	if( retval!=NOERROR ){
		printf( e_mesg );
		return(-1);
	}
	return(0);
}

static
PostItem()	/* Write tritem and update glmast simultaneously */
{
	int retval;
	int item_no;

	item_no = 0;

	reitem.ri_fund = rehdr.rh_fund;
	reitem.ri_sno = rehdr.rh_sno;
	reitem.ri_item_no = 0;
	flg_reset( RECTRAN );

	for( ; ; ){
#ifndef	ORACLE
		retval = get_n_reitem( &reitem, BROWSE, 0, FORWARD, e_mesg );
		if( retval==EFL ||
		    reitem.ri_fund != rehdr.rh_fund ||
		    reitem.ri_sno != rehdr.rh_sno )
			break;
#else
		retval = get_n_reitem( &reitem, BROWSE, 0, EQUAL, e_mesg );
		if( retval==EFL ) break ;
#endif
		if( retval != NOERROR ){
			printf( e_mesg );
			return(-1);
		}
		item_no++;

		glrec.funds = trhdr.th_fund;
		glrec.reccod = trhdr.th_reccod;
		STRCPY( glrec.accno,reitem.ri_accno );
		retval = get_gl( &glrec, UPDATE, 0, e_mesg );
		if( retval != NOERROR ){
			printf( e_mesg );
			return( -1 );
		}

		if( PostTrItem(item_no)<0 )
			return(-1);
		if( UpdtGlmast()<0 )
			return(-1);
	}
	seq_over( RECTRAN );
	return(0);
}

static
PostTrItem(item_no)	/* Write the tritem record to corresponding file */
int item_no;
{
	int retval;

	tritem.ti_fund = trhdr.th_fund ;
	tritem.ti_reccod = trhdr.th_reccod ;
	tritem.ti_create[0] = trhdr.th_create[0];
	tritem.ti_seq_no = trhdr.th_seq_no ;
	tritem.ti_item_no = item_no ;

	tritem.ti_sys_dt = trhdr.th_sys_dt ;
	tritem.ti_period = trhdr.th_period ;
	STRCPY( tritem.ti_accno, reitem.ri_accno ) ;
	tritem.ti_amount = reitem.ri_amount ;
	tritem.ti_status = 0 ;
	tritem.ti_section = glrec.sect;

	retval = put_tritem( &tritem, ADD, e_mesg );
	if( retval != NOERROR ){
		printf( e_mesg );
		return(-1);
	}

	return(0) ;
}

static
UpdtGlmast()	/* Update glmaster by the amount of transaction */
{
	int retval;

	if( tritem.ti_amount < 0.0 ) {
		glrec.curcr += tritem.ti_amount;
		glrec.curcr = D_Roundoff(glrec.curcr);
	}
	else {
		glrec.curdb += tritem.ti_amount;
		glrec.curdb = D_Roundoff(glrec.curdb);
	}
	glrec.ytd += tritem.ti_amount;
	glrec.ytd = D_Roundoff(glrec.ytd);
	
	glrec.currel[ tritem.ti_period-1] += tritem.ti_amount;
	glrec.currel[tritem.ti_period-1] =
			 D_Roundoff(glrec.currel[tritem.ti_period-1]);

	retval = put_gl( &glrec, UPDATE, e_mesg );
	if( retval != NOERROR ){
		printf( e_mesg );
		return( -1 );
	}
	return(0);
}

static
UpdtRehdr()	/* Modify date stamp in recurring header to current date */
{
	int retval;

	/* if every thing went properly, proper key will still be there  
	   for this record */

	retval = get_rehdr( &rehdr, UPDATE, 0, e_mesg );
	if( retval != NOERROR ){
		printf( e_mesg );
		return( -1 );
	}
	/* Get Next Date to Post on */
	if(NextPostDate(rehdr.rh_type[0])<0) return(-1);

	rehdr.rh_lperiod = parec.pa_cur_period;
	retval = put_rehdr( &rehdr, UPDATE, e_mesg );
	if( retval != NOERROR ){
		printf( e_mesg );
		return( -1 );
	}

	return(0);
}

static
NextPostDate( tr_type )	
char tr_type;		/* transaction type of current interest */
{
	long	date_plus();
	int	month;

	switch( tr_type ){
		case DAILY:	/* Recurring Daily add 1 day */
			rehdr.rh_date = date_plus(rehdr.rh_date,1);
			break;
		case WEEKLY:	/* Recurring Weekly add 7 day */
			rehdr.rh_date = date_plus(rehdr.rh_date,7);
			break;
		case BIWEEKLY:	/* Recurring Bi-Weekly add 14 day */
			rehdr.rh_date = date_plus(rehdr.rh_date,14);
			break;
		case MONTHLY:	/* Recurring Monthly */
			month = (rehdr.rh_date / 100) % 100;
			if(month == 12) {		/* if 12th month    */
				rehdr.rh_date += 10000L; /* Add 1 year       */
				rehdr.rh_date -= 1100L;  /* Set month to one */
			}
			else				/* otherwise        */
				rehdr.rh_date += 100L;	/* Add one month    */
			break;
		case QUARTERLY:	/* Recurring Quarterly */
			month = (rehdr.rh_date / 100) % 100;
			if(month > 9) {			/* if past 9th month */ 
				rehdr.rh_date += 10000L; /* ADD 1 year        */
				rehdr.rh_date += 	/* Set to appropriate*/
				   ((month+3-12) * 100);/* month of next year*/
			}
			else				/* otherwise	     */
				rehdr.rh_date += 300L;	/* Add 3 months      */
			break;
		case HALFYEARLY:	/* Recurring Half-Yearly */
			month = (rehdr.rh_date / 100) % 100;
			if(month > 6) {			/* if past 9th month */ 
				rehdr.rh_date += 10000L; /* ADD 1 year        */
				rehdr.rh_date += 	/* Set to appropriate*/
				   ((month+6-12) * 100);/* month of next year*/
			}
			else				/* otherwise	     */
				rehdr.rh_date += 600L;	/* Add 6 months      */
			break;
		case YEARLY:	/* Recurring Yearly */
			rehdr.rh_date += 10000L; /* ADD 1 year */
			break;  
	}
	return(NOERROR);
}

static
MakeHeaderKey()
{
	/***
	char	terml[4] ;
	***/
	int	retval;
#ifdef	ORACLE
	long	get_maxsno(), sno ;
#endif

	/*****
	if( get_tnum( terml )<0 ) return(-1);
	****/

	trhdr.th_fund = rehdr.rh_fund;
	trhdr.th_reccod = rehdr.rh_reccod;
	trhdr.th_create[0] = 'G';

#ifndef	ORACLE
	/* Move highs to serial no, and access previous record in sequence */
	trhdr.th_seq_no = HIGH_LONG;
	flg_reset(GLTRHDR);

	retval = get_n_trhdr( &trhdr,BROWSE,0,BACKWARD,e_mesg );
	if( retval==ERROR ){
		printf(e_mesg);
		return(-1);
	}
	
	seq_over(GLTRHDR);

	if( trhdr.th_fund != rehdr.rh_fund ||
	    trhdr.th_reccod != rehdr.rh_reccod ||
	    trhdr.th_create[0] != 'G' ||
	    retval==EFL ){		/* It is 1 if end of file is reached */
		trhdr.th_fund = rehdr.rh_fund;
		trhdr.th_reccod = rehdr.rh_reccod;
		trhdr.th_create[0] = 'G';
		trhdr.th_seq_no = 1;
	}
	else  /* The new serial# is one greater than that of the record read */
		trhdr.th_seq_no++;
#else
	sno = get_maxsno(GLTRHDR, (char*)&trhdr, 0, -1, e_mesg) ;
	if(sno < 0) {
		printf(e_mesg);
		return(-1);
	}
	trhdr.tr_seq_no = sno + 1;
#endif
	return(0);
}

