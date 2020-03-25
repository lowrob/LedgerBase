/*-----------------------------------------------------------------------
Source Name: bdgt_dis.c
System     : Budgetary Financial system.
Module     : General Ledger system.
Created  On: 21st Jul 89.
Created  By: T AMARENDRA.

COBOL Source(s): gl160f--01

DESCRIPTION:
	Function to Print Distributed Actaul/Budget Report.

Usage of SWITCHES when they are ON :
	SW1 :
	SW2 :
		Not Used.
	SW3 :	

	SW4 :
	SW5 :
	SW6 :
		Not Used.
	SW7 :	(COMPANY)
		System Installed for Companies.
	SW8 :
		Not Used.

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
J.Prescott     91/02/19       Added Cost Center Key Range.

------------------------------------------------------------------------*/

#include <stdio.h>
#include <bdgt_rep.h>	/* Reports Variables & Definitions */

/* NOTE: see bdgt_rep.h for all comman definitions */

/* local variables */

static	char	T4_desc[sizeof(gl_rec.desc)] ,	/* Title4 Desc */
		T5_desc[sizeof(gl_rec.desc)] ,	/* Title5 Desc */
		T6_desc[sizeof(gl_rec.desc)] ;	/* Title6 Desc */

static	short	T4_Exists, T5_Exists, T6_Exists ;

/* Active Keys values */
static	long	prev_k4 ;
static	long	prev_k5 ;
static	long	prev_k6 ;
static	short	prev_sect ;
static	short	prev_reccod ;

/* Totalling variables */

static	double	G_total[NO_PERIODS] ;	/* Fund Total  (COBOL 1) */
static	double	F_total[NO_PERIODS] ;	/* Section Total - (COBOL 2) */
static	double	K6_total[NO_PERIODS] ;	/* K6 != 0 (COBOL - 3) */
static	double	K5_total[NO_PERIODS] ;	/* K5 != 0 (COBOL - 4) */
static	double	K4_total[NO_PERIODS] ;	/* K4 != 0 (COBOL 5) */

static	int	RepType ;	/* 4 - Distribution - Budget
				   5  - Distribution - Actaul */

/*-----------------------------------------------------------------------*/
BdgtDistribution(option)	/* Budget Distribution Reports */
int	option ;
{

	/* select the report options */
	ret_cd = GetOptions();
	if(ret_cd != NOERROR) return(ret_cd) ;

	RepType = option ;

	T4_Exists = 0 ;
	T5_Exists = 0 ;
	T6_Exists = 0 ;

	InitPeriods( G_total	) ;
	InitPeriods( F_total	) ;
	InitPeriods( K6_total	) ;
	InitPeriods( K4_total	) ;
	InitPeriods( K5_total	) ;

	/* Read the Budget Data (SEQDATA) file */
	/* Set the file to 1st record */
	flg_reset(TMPINDX_2) ;
	gl_rec.funds = rp_sth.s_fm_fund ;
	gl_rec.sect = 0 ;
	gl_rec.admis = 0 ;
	prev_k4 = 0 ;
	prev_k5 = 0 ;
	prev_k6 = 0 ;

	ctl_rec.fund = -1 ;	/* No fund is active */

	for( ; ; ) {
		ret_cd = get_next((char*)&gl_rec, TMPINDX_2, 0, FORWARD,
						BROWSE,e_mesg);
		if(ret_cd == EFL) break ;
		if(ret_cd != NOERROR) return(DBH_ERR) ;

#ifdef	ORACLE
		if(SeqConstrnt(&gl_rec) == ERROR) continue ;
#endif
		if(gl_rec.funds > rp_sth.s_to_fund) break ;

		if(gl_rec.keys[pa_rec.pa_cc_key-1] < rp_sth.s_fm_ccno || 
		   gl_rec.keys[pa_rec.pa_cc_key-1] > rp_sth.s_to_ccno )
				continue;

		if(gl_rec.funds != ctl_rec.fund) {
			if(ctl_rec.fund != -1) {
				/*** Key Totals printing not there in COBOL Pgm
				if((ret_cd = KeyTotals(4)) < 0) return(ret_cd) ;
				***/
				if((ret_cd = GrandTotal()) < 0) return(ret_cd) ;
			}
			ret_cd = GetFund() ;
			if(ret_cd != NOERROR) return(ret_cd) ;

			ret_cd = GetTitles(4) ;
			if(ret_cd != NOERROR) return(ret_cd) ;
		}
		else if(gl_rec.sect != prev_sect) {
			if((ret_cd = KeyTotals(4)) < 0) return(ret_cd) ;
			if((ret_cd = SectTotals()) < 0) return(ret_cd) ;

			ret_cd = GetTitles(4) ;
			if(ret_cd != NOERROR) return(ret_cd) ;
		}
		else if(KEY_EXISTS(4) && gl_rec.keys[KEY(4)] != prev_k4 ) {
			if((ret_cd = KeyTotals(4)) < 0) return(ret_cd) ;

			ret_cd = GetTitles(4) ;
			if(ret_cd != NOERROR) return(ret_cd) ;
		}
		else if(KEY_EXISTS(5) && gl_rec.keys[KEY(5)] != prev_k5 ) {
			if((ret_cd = KeyTotals(5)) < 0) return(ret_cd) ;

			ret_cd = GetTitles(5) ;
			if(ret_cd != NOERROR) return(ret_cd) ;
		}
		/*
		* Though reccod is after Key 6 in sort key, COBOL program is
		* checking reccod change before checking Key 6. Same logic is
		* followed here also.
		*/
		else if(gl_rec.reccod != 97 && gl_rec.reccod != prev_reccod) {
			if((ret_cd = KeyTotals(5)) < 0) return(ret_cd) ;

			ret_cd = GetTitles(5) ;
			if(ret_cd != NOERROR) return(ret_cd) ;

			prev_reccod = gl_rec.reccod ;
		}
		else if(KEY_EXISTS(6) && gl_rec.keys[KEY(6)] != prev_k6 ) {
			if((ret_cd = KeyTotals(6)) < 0) return(ret_cd) ;

			ret_cd = GetTitles(6) ;
			if(ret_cd != NOERROR) return(ret_cd) ;
		}

		if(gl_rec.reccod != 99) continue ;

		ret_cd = PrntRecord() ;
		if(ret_cd != NOERROR) return(ret_cd) ;
	}

	if(pgcnt)
		if((ret_cd = GrandTotal()) < 0) return(ret_cd) ;

	return(NOERROR) ;
}
/*--------------------------------------------------------*/
static	int
GrandTotal()
{
	if((ret_cd = KeyTotals(4)) < 0) return(ret_cd) ;
	if(prnt_line() < 0) return(ERROR) ;

	if((ret_cd = SectTotals()) < 0) return(ret_cd) ;

#ifdef ENGLISH
	if((ret_cd = PrntTotal( G_total, "Fund Total" )) < 0) return(ret_cd) ;
#else
	if((ret_cd = PrntTotal(G_total,"Total du fond")) < 0) return(ret_cd) ;
#endif

	InitPeriods( G_total );
	return(NOERROR) ;
}	/* GrandToral() */
/*-----------------------------------------------------------------*/
static	int
SectTotals()
{
	int	i ;
	char	desc[60] ;

	if(prev_sect == EXPENDITURE)
#ifdef ENGLISH
		strncpy(desc,"Total - Expenses",sizeof(desc));
	else	/* Section INCOME */
		strncpy(desc,"Total - Revenue",sizeof(desc));
#else
		strncpy(desc,"Total - Depenses",sizeof(desc));
	else	/* Section INCOME */
		strncpy(desc,"Total - Revenu",sizeof(desc));
#endif

	
	if((ret_cd = PrntTotal( F_total, desc )) < 0) return(ret_cd) ;

	/* Update Grand Total */
	for(i = 0 ; i < rp_sth.s_period ; i++)
		G_total[i] += F_total[i] ;

	InitPeriods( F_total ) ;

	return(NOERROR) ;
}	/* SectTotals() */
/*-----------------------------------------------------------------*/
static	int
KeyTotals(key_no)
int	key_no ;
{
	if((ret_cd = Key6Totals()) < 0) return(ret_cd) ;
	if(key_no <= 5)
		if((ret_cd = Key5Totals()) < 0) return(ret_cd) ;
	if(key_no <= 4)
		if((ret_cd = Key4Totals()) < 0) return(ret_cd) ;

	return(NOERROR) ;
}	/* KeyTotals() */
/*-----------------------------------------------------------------*/
/* Print Total 6 */
static	int
Key6Totals()
{
	char	t_desc[10+sizeof(title_rec.desc)] ;

	if( T6_Exists ) {
#ifdef ENGLISH
		STRCPY(t_desc, "Total - ");
#else
		STRCPY(t_desc, "Total - ");
#endif
		strncat(t_desc, T6_desc, sizeof(title_rec.desc));
		if((ret_cd = PrntTotal(K6_total, t_desc)) < 0) return(ret_cd) ;
	}

	InitPeriods( K6_total ) ;
	return(NOERROR) ;
}	/* Key6Totals() */
/*-----------------------------------------------------------------*/
/* Print Total 5 */
static	int
Key5Totals()
{
	char	t_desc[10+sizeof(title_rec.desc)] ;

	if( T5_Exists ) {
#ifdef ENGLISH
		STRCPY(t_desc, "Total - ");
#else
		STRCPY(t_desc, "Total - ");
#endif
		strncat(t_desc, T5_desc, sizeof(title_rec.desc));
		if((ret_cd = PrntTotal(K5_total, t_desc)) < 0) return(ret_cd);
	}

	InitPeriods( K5_total ) ;
	return(NOERROR) ;
}	/* Key5Totals() */
/*-----------------------------------------------------------------*/
/* Print Total 4 */
static	int
Key4Totals()
{
	char	t_desc[10+sizeof(title_rec.desc)] ;

#ifdef ENGLISH
	STRCPY(t_desc, "Total - ");
#else
	STRCPY(t_desc, "Total - ");
#endif
	strncat(t_desc, T4_desc, sizeof(title_rec.desc));
	if((ret_cd = PrntTotal( K4_total, t_desc )) < 0) return(ret_cd) ;

	InitPeriods( K4_total ) ;

	return(NOERROR) ;
}	/* Key4Totals() */
/*--------------------------------------------------------*/
static int
PrntHeadings()	/* Write Report Headings */
{
	int	i;
	char	t_buf[60] ;

	ret_cd = FormFeed();	/* Skip Page, print School District name,
				   Page# & date */
	if(ret_cd != NOERROR) return(ret_cd) ;

	if(RepType == 5)	/* Actual */
#ifdef ENGLISH
		mkln(54,"DISTRIBUTION  OF  ACTUAL",24);
	else
		mkln(54,"DISTRIBUTION  OF  BUDGET",24);
#else
		mkln(53,"DISTRIBUTION  DE  L'ACTUEL",26);
	else
		mkln(54,"DISTRIBUTION  DE  BUDGET",24);
#endif

	if(prnt_line() < 0)return(ERROR);

#ifdef ENGLISH
	mkln(1,"Code: ",6);
#else
	mkln(1,"Code: ",6);
#endif
	tedit((char*)&prev_k4,Mask_5,line+cur_pos,R_LONG) ;
	cur_pos += 5 ;
#ifdef ENGLISH
	mkln(54,"At The End Of Period: ",22);
#else
	mkln(53,"A la fin de la periode: ",24);
#endif
	tedit((char*)&rp_sth.s_period,Mask_2,line+cur_pos,R_SHORT);
	cur_pos += 2 ;
	if(prnt_line() < 0)return(ERROR);

	mkln(1,T4_desc,sizeof(T4_desc));
	/* Centre the Fund Before Printing */
#ifdef ENGLISH
	sprintf(t_buf,"%s:  %d  %s",(COMPANY) ? "CO." : "Fund",
			ctl_rec.fund, ctl_rec.desc);
#else
	sprintf(t_buf,"%s:  %d  %s",(COMPANY) ? "Cie" : "Fond",
			ctl_rec.fund, ctl_rec.desc);
#endif
	i = strlen(t_buf);
	mkln(((LNSZ - i) / 2),t_buf, sizeof(t_buf));
	if(prnt_line() < 0)return(ERROR);
	if(prnt_line() < 0)return(ERROR);

	/* Print Column Headings */
	for(i = 1 ; i <= rp_sth.s_period ; i++) {
#ifdef ENGLISH
		mkln((5 + (i-1) * 10), "Per. ", 5);
#else
		mkln((5 + (i-1) * 10), "Per. ", 5);
#endif
		tedit((char*)&i,Mask_2,line+cur_pos,R_INT);
		cur_pos += 2 ;
	}
	if(prnt_line() < 0)return(ERROR);

	for(i = 0 ; i < rp_sth.s_period ; i++)
#ifdef ENGLISH
		mkln((5 + i * 10), "To Date", 7);
#else
		mkln((5 + i * 10), "a date", 6);
#endif

	if(prnt_line() < 0)return(ERROR);
	if(prnt_line() < 0)return(ERROR);

	return(NOERROR);
}	/* PrntHeadings() */
/*-----------------------------------------------------------------*/
static	int	/* Form Report line for current record and print */
PrntRecord()
{
	int	i ;
	double	cum_total ;
	double	cum_periods[NO_PERIODS] ;

	/******
	*** IF SW4 is on add commiments to period acutal *
	if(SW4)
		gl_rec.currel[period] += gl_rec.comdat ;

	* IF SW5 is on add commiments to Cumulative acutal *
	if(SW5)
		cum_actual += gl_rec.comdat ;
	******/

	/* Check for non zero total */
	for(i = 0 ; i < rp_sth.s_period ; i++)
	    if(RepType == 5) {	/* Actual */
		if(gl_rec.currel[i] < -0.005 || gl_rec.currel[i] > 0.005)break ;
	    }
	    else {
		if(gl_rec.curbud[i] < -0.005 || gl_rec.curbud[i] > 0.005)break ;
	    }


	/* If non zero total exists */
	if(i < rp_sth.s_period) {
		if(linecnt >= (PGSIZE-5))
			if((ret_cd = PrntHeadings()) < 0) return(ret_cd) ;
	}
	else {
		if(linecnt >= (PGSIZE-2))
			if((ret_cd = PrntHeadings()) < 0) return(ret_cd) ;
	}

#ifdef ENGLISH
	mkln(1, "Account:", 8) ;
#else
	mkln(1, "Compte:", 7) ;
#endif
	mkln(10, gl_rec.accno, sizeof(gl_rec.accno));
	mkln((cur_pos+3), gl_rec.desc, 48) ;	/* Leave 2 balnks */
	if(prnt_line() < 0) return(ERROR) ;

	/* If non zero total exists */
	if(i < rp_sth.s_period) {
		/* Update the Totals Records */
		UpdateTotals(gl_rec) ;

		if(RepType == 5) {	/* Actual */
			if(PrntPeriods(gl_rec.currel) < 0) return(ERROR) ;
			/* Print Cumulative to period */
			CumPeriods(cum_periods, gl_rec.currel) ;
			if(PrntPeriods(cum_periods) < 0) return(ERROR) ;

			cum_total = Cumulative(rp_sth.s_period, gl_rec.currel) ;
			if(PrntPercentage(gl_rec.currel, cum_total) < 0)
				return(ERROR) ;
			if(PrntPercentage(cum_periods, cum_total) < 0)
				return(ERROR) ;
		}
		else {
			if(PrntPeriods(gl_rec.curbud) < 0) return(ERROR) ;
			/* Print Cumulative to period */
			CumPeriods(cum_periods, gl_rec.curbud) ;
			if(PrntPeriods(cum_periods) < 0) return(ERROR) ;

			cum_total = Cumulative(rp_sth.s_period, gl_rec.curbud) ;
			if(PrntPercentage(gl_rec.curbud, cum_total) < 0)
				return(ERROR) ;
			if(PrntPercentage(cum_periods, cum_total) < 0)
				return(ERROR) ;
		}
	}

	if(linecnt < PGSIZE)
		if(prnt_line() < 0) return(ERROR) ;

	return(NOERROR) ;
}	/* PrntRecord() */
/*-----------------------------------------------------------------*/
static	int
UpdateTotals(g_rec)
Gl_rec	g_rec ;
{
	/* K6 not zero */
	if( KEY_EXISTS(6) && g_rec.keys[KEY(6)] )
	    AddPeriods(K6_total, g_rec) ;

	/* K5 not zero */
	if( KEY_EXISTS(5) && g_rec.keys[KEY(5)] )
	    AddPeriods(K5_total, g_rec) ;

	/* K4 not zero */
	if( KEY_EXISTS(4) && g_rec.keys[KEY(4)] )
		AddPeriods(K4_total, g_rec) ;

	AddPeriods(F_total, g_rec) ;

	return(NOERROR) ;
}	/* UpdateTotals() */
/*---------------------------------------------------------------*/
static	int	/* Add period totals */
AddPeriods(total, g_rec)
double	total[] ;
Gl_rec	g_rec ;
{
	int	i ;

	for(i = 0 ; i < rp_sth.s_period ; i++)
		if(RepType == 5)	/* Actual */
			total[i] += g_rec.currel[i] ;
		else
			total[i] += g_rec.curbud[i] ;
	return(NOERROR);
}	/* AddPeriods() */
/*-----------------------------------------------------------------*/
static	int
PrntTotal( total, desc )
double	total[] ;
char	*desc ;
{
	double	cum_total ;
	double	cum_periods[NO_PERIODS] ;
	int	i ;

	if(linecnt >= (PGSIZE - 7))
		if((ret_cd = PrntHeadings()) < 0) return(ret_cd) ;

	if(PrntHyphens() < 0) return(ERROR) ;

	mkln(10, desc,55) ;
	if(prnt_line() < 0) return(ERROR) ;

	/* Check for non zero total */
	for(i = 0 ; i < rp_sth.s_period ; i++)
		if(total[i] < -0.005 || total[i] > 0.005) break ;
	/* If non zero total exists */
	if(i < rp_sth.s_period) {
		if(PrntPeriods(total) < 0) return(ERROR) ;
		/* Print Cumulative periods */
		CumPeriods(cum_periods, total) ;
		if(PrntPeriods(cum_periods) < 0) return(ERROR) ;

		cum_total = Cumulative(rp_sth.s_period, total );
		if(PrntPercentage(total, cum_total) < 0)
			return(ERROR) ;
		if(PrntPercentage(cum_periods, cum_total) < 0) return(ERROR) ;
	}
	if(PrntHyphens() < 0) return(ERROR) ;

	if(linecnt < PGSIZE)
		if(prnt_line() < 0) return(ERROR) ;

	return(NOERROR) ;
}	/* PrntTotal */
/*-----------------------------------------------------------------*/
static	int	/* Print Hyphens line */
PrntHyphens()
{
	int	i;

	/* move Hyphens with 1 space */
	for( i = 0 ; i < rp_sth.s_period ; i++)
		mkln((3 + i * 10), Hyphens, 9) ;
	if(prnt_line() < 0) return(ERROR) ;

	return(NOERROR) ;
}	/* PrntHyphens() */
/*-----------------------------------------------------------------*/
static	int	/* Print Report lines for a given array */
PrntPeriods(total)
double	total[] ;
{
	int	i;

	mkln(2, " ", 1);
	for( i = 0 ; i < rp_sth.s_period ; i++) {
		tedit((char*)&total[i], Mask_9, line+cur_pos, R_DOUBLE) ;
		cur_pos += 10 ;
	}

	if(prnt_line() < 0) return(ERROR) ;

	return(NOERROR) ;
}	/* PrntPeriods() */
/*-----------------------------------------------------------------*/
static	int
PrntPercentage( total, cum_total )
double	total[] ;
double	cum_total ;
{
	int	i ;
	double	temp ;

	/* Print Percentage line */
	for( i = 0 ; i < rp_sth.s_period ; i++) {
	    /* if cumulative total not zero then print % */
	    if(cum_total < -0.005 || cum_total > 0.005) {
		temp = (total[i] / cum_total) * 100 ;
		if(temp <= -0.0001 || temp >= 0.0001) {
			mkln((2 + i * 10), " ", 1);
			tedit((char*)&temp, Mask_7_4, line+cur_pos, R_DOUBLE) ;
			cur_pos += 9 ;
			mkln(cur_pos+1, "%", 1);
		}
	    }
	}

	if(prnt_line() < 0) return(ERROR) ;

	return(NOERROR) ;
}	/* PrntPercentage() */
/*-----------------------------------------------------------------*/
static	int
GetTitles(key_no)
int	key_no ;
{
	switch(key_no) {
	case 4 :	/* Key 4 */
		/* Get Key 4 Title */
		ret_cd = GetK4Title() ;
		if(ret_cd != NOERROR) return(ret_cd) ;

		ret_cd = PrntHeadings() ;
		if(ret_cd != NOERROR) return(ret_cd) ;

		prev_sect  = gl_rec.sect  ;
		prev_reccod = gl_rec.reccod ;
		/* Fall thru to next case */
	case 5 :	/* Key 5 */
		/* Get Key 5 Title */
		ret_cd = GetK5Title() ;
		if(ret_cd != NOERROR) return(ret_cd) ;
		/* Fall thru to next case */
	default :	/* Key 6 */
		/* Get Key 6 Title */
		ret_cd = GetK6Title() ;
		if(ret_cd != NOERROR) return(ret_cd) ;
	}

	return(NOERROR) ;
}	/* GetTitles() */
/*-----------------------------------------------------------------*/
/* Get Title 4 */
static	int	
GetK4Title()
{
	int	i ;

	T4_Exists = 0 ;
	prev_k4 = 0 ;
	T4_desc[0] = '\0' ;

	if(KEY_EXISTS(4) == 0 || (prev_k4 = gl_rec.keys[KEY(4)]) == 0)
		return(NOERROR) ;

	/* Form the Key4 Title key */
	title_rec.keys[KEY(4)] = prev_k4;

	/* MOve 0's to first 3 keys */
	for( i = 1 ; i <= 3 ; i++)
		title_rec.keys[KEY(i)] = 0;

	/* MOve 0's to 5,6 & 7  keys */
	for( i = 5 ; i <= USER_KEYS && KEY_EXISTS(i) ; i++)
		title_rec.keys[KEY(i)] = 0;

	ret_cd = get_isrec((char*)&title_rec, TMPINDX_1, 0, BROWSE, e_mesg) ;
	if(ret_cd == ERROR) return(DBH_ERR) ;

	if(ret_cd == UNDEF) return(NOERROR) ;

	STRCPY(T4_desc, title_rec.desc);

	if((ret_cd = AddTitle()) < 0) return(ret_cd) ;

	T4_Exists = 1 ;
	return(NOERROR) ;
}	/* GetK4Title() */
/*-----------------------------------------------------------------*/
/* Get Title 5 */
static	int	
GetK5Title()
{
	int	i ;

	T5_Exists = 0 ;
	prev_k5 = 0 ;
	T5_desc[0] = '\0' ;

	/* K5 user has not given or the value in record is zero */

	if(KEY_EXISTS(5) == 0 || (prev_k5 = gl_rec.keys[KEY(5)]) == 0)
		return(NOERROR) ;

	/* Form the Key5 Title key */
	title_rec.keys[KEY(5)] = gl_rec.keys[KEY(5)] ;

	if(ALL_KEYS) {
		/* MOve first 4 key values also */
		for( i = 1 ; i <= 4 ; i++)
			title_rec.keys[KEY(i)] = gl_rec.keys[KEY(i)] ;
	}
	else {
		/* MOve 0's to first 4 keys */
		for( i = 1 ; i <= 4 ; i++)
			title_rec.keys[KEY(i)] = 0;
	}

	/* MOve 0's to 6 & 7  keys */
	for( i = 6 ; i <= USER_KEYS && KEY_EXISTS(i) ; i++)
		title_rec.keys[KEY(i)] = 0;

	ret_cd = get_isrec((char*)&title_rec, TMPINDX_1, 0, BROWSE, e_mesg) ;
	if(ret_cd == ERROR) return(DBH_ERR) ;

	if(ret_cd == UNDEF) return(NOERROR) ;

	STRCPY(T5_desc, title_rec.desc);

	if((ret_cd = AddTitle()) < 0) return(ret_cd) ;

	if((ret_cd = PrntSubTitle()) < 0) return(ret_cd) ;
	T5_Exists = 1 ;

	return(NOERROR) ;
}	/* GetK5Title() */
/*-----------------------------------------------------------------*/
/* Get Title 6 */
static	int	
GetK6Title()
{
	int	i ;

	T6_Exists = 0 ;
	prev_k6 = 0 ;
	T6_desc[0] = '\0' ;

	/* K6 user has not given or the value in record is zero */

	if(KEY_EXISTS(6) == 0 || (prev_k6 = gl_rec.keys[KEY(6)]) == 0)
		return(NOERROR) ;

	/* Form the Key6 Title key */
	title_rec.keys[KEY(6)] = gl_rec.keys[KEY(6)] ;

	if(ALL_KEYS) {
		/* MOve first 5 key values also */
		for( i = 1 ; i <= 5 ; i++)
			title_rec.keys[KEY(i)] = gl_rec.keys[KEY(i)] ;
	}
	else {
		/* MOve 0's to first 5 keys */
		for( i = 1 ; i <= 5 ; i++)
			title_rec.keys[KEY(i)] = 0;
	}

	/* MOve 0's to  7 key */
	if(KEY_EXISTS(7))
		title_rec.keys[KEY(7)] = 0;

	ret_cd = get_isrec((char*)&title_rec, TMPINDX_1, 0, BROWSE, e_mesg) ;
	if(ret_cd == ERROR) return(DBH_ERR) ;

	if(ret_cd == UNDEF) return(NOERROR) ;

	STRCPY(T6_desc, title_rec.desc);

	if((ret_cd = AddTitle()) < 0) return(ret_cd) ;

	if((ret_cd = PrntSubTitle()) < 0) return(ret_cd) ;
	T6_Exists = 1 ;

	return(NOERROR) ;
}	/* GetK6Title() */
/*-----------------------------------------------------------------*/
static	int	/* Print Title of k5 or Title of k6 as a sub title */
PrntSubTitle()
{
	int	i ;

	if(linecnt > (PGSIZE - 3) ) {	/* Atleast 3 lines required */
		ret_cd = PrntHeadings() ;
		if(ret_cd != NOERROR) return(ret_cd) ;
	}

	if(title_rec.budcur < -0.005 && title_rec.budcur > 0.005)
		return(NOERROR) ;

	i = mkln(10,title_rec.desc,60) ;
	if(prnt_line() < 0) return(ERROR) ;
	/* Print Under line */
	mkln(10,Hyphens,i) ;
	if(prnt_line() < 0) return(ERROR) ;
	if(prnt_line() < 0) return(ERROR) ;

	return(NOERROR) ;
}	/* PrntSubTitle() */
/*----------------------------------------------------------------------*/
static	int
AddTitle()
{
	double	cum_total ;
	double	cum_periods[NO_PERIODS] ;

	if( title_rec.budcur < -0.005 && title_rec.budcur > 0.005 ){

	    UpdateTotals(title_rec) ;

	    if(linecnt >= (PGSIZE-5))
		if((ret_cd = PrntHeadings()) < 0) return(ret_cd) ;

#ifdef ENGLISH
	    mkln(1, "Account:", 8) ;
#else
	    mkln(1, "Compte:", 7) ;
#endif
	    mkln(10, title_rec.accno, sizeof(title_rec.accno));
	    mkln((cur_pos+3), title_rec.desc, 48) ;	/* Leave 2 balnks */
	    if(prnt_line() < 0) return(ERROR) ;

	    if(RepType == 5) {	/* Actual */
		if(PrntPeriods(title_rec.currel) < 0) return(ERROR) ;
		/* Print Cumulative to period */
		CumPeriods(cum_periods, title_rec.currel) ;
		if(PrntPeriods(cum_periods) < 0) return(ERROR) ;

		cum_total = Cumulative(rp_sth.s_period, title_rec.currel) ;
		if(PrntPercentage(title_rec.currel, cum_total) < 0)
			return(ERROR) ;
		if(PrntPercentage(cum_periods, cum_total) < 0)
			return(ERROR) ;
	    }
	    else {
		if(PrntPeriods(title_rec.curbud) < 0) return(ERROR) ;
		/* Print Cumulative to period */
		CumPeriods(cum_periods, title_rec.curbud) ;
		if(PrntPeriods(cum_periods) < 0) return(ERROR) ;

		cum_total = Cumulative(rp_sth.s_period, title_rec.curbud) ;
		if(PrntPercentage(title_rec.curbud, cum_total) < 0)
			return(ERROR) ;
		if(PrntPercentage(cum_periods, cum_total) < 0)
			return(ERROR) ;
	    }
	    if(linecnt < PGSIZE)
		if(prnt_line() < 0) return(ERROR) ;
	}

	return(NOERROR) ;
}	/* AddTitle() */
/*-----------------------------------------------------*/
static	int	/* Cumulate each period upto that period */
CumPeriods( cum_periods, total )
double	cum_periods[] ;
double	total[] ;
{
	int	i ;

	cum_periods[0] = total[0] ;
	/* Cumulate 2nd period onwards */
	for(i = 1 ; i < rp_sth.s_period ; i++)
		cum_periods[i] = total[i] + cum_periods[i-1] ;
	return(NOERROR);
}	/* CumPeriods() */
/*-----------------------------------------------------*/
static	int	/* Initialize given array to 0 */
InitPeriods( total )
double	total[] ;
{
	int	i ;

	for(i = 0 ; i < rp_sth.s_period ; i++)
		total[i] = 0.0 ;
	return(NOERROR);
}	/* InitPeriods() */
/*-------------------- E n d   O f   P r o g r a m ---------------------*/
