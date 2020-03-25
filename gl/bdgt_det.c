/*-----------------------------------------------------------------------
Source Name: bdgt_det.c
System     : Budgetary Financial system.
Module     : General Ledger system.
Created  On: 21st Jul 89.
Created  By: T AMARENDRA.

COBOL Source(s): gl146f--nb

DESCRIPTION:
	Function to Print Detailed Budget Report, Annual Budget report and
	Budget report with account#s.

Usage of SWITCHES when they are ON :
	SW1 :
	SW2 :
		Not Used.
	SW3 :	

	SW4 :
		Add Commitmemts to Period Actaul.
	SW5 :
		Add Commitmemts to Cumulative Actaul.
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

static	short	T4_Exists, T5_Exists, T6_Exists, FirstTitle ;

/* Active Keys values */
static	long	prev_k4 ;
static	long	prev_k5 ;
static	long	prev_k6 ;
static	short	prev_admis ;
static	short	prev_sect ;
static	short	prev_reccod ;

static	char	Um_desc[10][sizeof(gl_rec.desc)] ;

static	double	prec1 , prec2 ;	/* Similar to COBOL PROGRAM. Total-6 line
				   printing will be skipped if the totals are
				   same as previous line */

/* Totalling variables */

static	Btotal	G_total ;	/* Fund Total - reccod == 99 ||
					(K4 == 0 && reccod == 98) (COBOL 1) */
static	Btotal	F_total ;	/* Section Total - reccod == 99 ||
					(k4 == 0 && reccod == 98) (COBOL 2) */
static	Btotal	K6_total ;	/* K6 != 0 && reccod == 99 or 98 (COBOL - 3) */
static	Btotal	K5_total ;	/* K5 != 0 && reccod == 99 or 98 (COBOL - 4) */
static	Btotal	K4_total ;	/* K4 != 0 && reccod == 99 (COBOL 5) */
static	Btotal	K4R98_total ;	/* K4 != 0 && reccod == 98 && UnitCD == 0
					(COBOL 7) */
static	Btotal	S_total ;	/* reccod == 99 && (K5 is within K8's range
					(Salaries) (COBOL 8) */
static	Btotal	Um_total[10] ;	/* Reccod == 97 & UnitCode == 1 to 10 
					(COBOL 6, 9 to 17 ) */
static	Btotal	H_total ;	/* k4 != 0 && reccod == 98 (COBOL 18) */


static	int	RepType ;	/* 'A' - Detailed, 'B' - Annual and
				    'C' - With Account */


/*-----------------------------------------------------------------------*/
BdgtReport(option)	/* Budget Report */
int	option ;
{
	int	i;

	/* select the report options */
	ret_cd = GetOptions();
	if(ret_cd != NOERROR) return(ret_cd) ;

	RepType = option ;

	T4_Exists = 0 ;
	T5_Exists = 0 ;
	T6_Exists = 0 ;
	FirstTitle = 1 ;
	prec1 = prec2 = 0.0 ;

	InitTotal( &G_total	) ;
	InitTotal( &F_total	) ;
	InitTotal( &K6_total	) ;
	InitTotal( &K4_total	) ;
	InitTotal( &K5_total	) ;
	InitTotal( &K4R98_total	) ;
	InitTotal( &S_total	) ;
	InitTotal( &H_total	) ;

	for(i = 0 ; i < 10 ; i++) {
		InitTotal( &Um_total[i] ) ;
		Um_desc[i][0] = '\0' ;
	}

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
		else if(gl_rec.admis != prev_admis ||
						gl_rec.sect != prev_sect) {
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
		/***
		else if(gl_rec.reccod != 97 && gl_rec.reccod != prev_reccod) {
		***/
		else if(gl_rec.reccod != prev_reccod) {
			if(prev_reccod != 97)
				if((ret_cd = KeyTotals(5)) < 0) return(ret_cd) ;

			/***
			ret_cd = GetTitles(5) ;
			if(ret_cd != NOERROR) return(ret_cd) ;
			***/

			prev_reccod = gl_rec.reccod ;
		}
		else if(KEY_EXISTS(6) && gl_rec.keys[KEY(6)] != prev_k6 ) {
			if((ret_cd = KeyTotals(6)) < 0) return(ret_cd) ;

			ret_cd = GetTitles(6) ;
			if(ret_cd != NOERROR) return(ret_cd) ;
		}

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
	double	temp1, temp2 ;

	if((ret_cd = KeyTotals(4)) < 0) return(ret_cd) ;
	if(prnt_line() < 0) return(ERROR) ;

	temp1 = S_total.cum_actual - prec1 ;
	temp2 = S_total.cum_curbud - prec2 ;

	if( (temp1 < -0.005 || temp1 > 0.005) ||
				(temp2 < -0.005 || temp2 >0.005) ) {
		if(linecnt > (PGSIZE - 4) )
			if((ret_cd = PrntHeadings()) < 0) return(ret_cd) ;

		if((ret_cd = PrntHyphens()) < 0) return(ret_cd) ;
#ifdef ENGLISH
		if((ret_cd = PrntTotal( S_total, "Total - Salaries" )) < 0)
#else
		if((ret_cd = PrntTotal( S_total, "Total - Salaires" )) < 0)
#endif
			return(ret_cd) ;
	}
	else {
		if(linecnt > (PGSIZE - 2) )
			if((ret_cd = PrntHeadings()) < 0) return(ret_cd) ;
	}

	if((ret_cd = PrntPercentage( S_total )) < 0) return(ret_cd) ;

	if((ret_cd = SectTotals()) < 0) return(ret_cd) ;

	if(linecnt > (PGSIZE - 4) )
		if((ret_cd = PrntHeadings()) < 0) return(ret_cd) ;

	if((ret_cd = PrntHyphens()) < 0) return(ret_cd) ;
#ifdef ENGLISH
	if((ret_cd = PrntTotal( G_total, "Fund Total" )) < 0) return(ret_cd) ;
#else
	if((ret_cd = PrntTotal(G_total,"Total du fond")) < 0) return(ret_cd) ;
#endif
	if((ret_cd = PrntPercentage( G_total )) < 0) return(ret_cd) ;

	InitTotal( &S_total );
	InitTotal( &G_total );
	return(NOERROR) ;
}	/* GrandToral() */
/*--------------------------------------------------------*/
static int
PrntHeadings()	/* Write Report Headings */
{
	int	i;
	char	t_buf[60] ;

	ret_cd = FormFeed();	/* Skip Page, print School District name,
				   Page# & date */
	if(ret_cd != NOERROR) return(ret_cd) ;

#ifdef ENGLISH
	mkln(54,"B U D G E T    R E P O R T",26);
#else
	mkln(50,"R A P P O R T   D E   B U D G E T",33);
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
	mkln(59,"For Period: ",12);
#else
	mkln(56,"Pour la periode: ",17);
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
	sprintf(t_buf,"%s:  %d  %s",(COMPANY) ? "Cie." : "Fond",
			ctl_rec.fund, ctl_rec.desc);
#endif
	i = strlen(t_buf);
	mkln(((LNSZ - i) / 2),t_buf, sizeof(t_buf));
	if(prnt_line() < 0)return(ERROR);
	if(prnt_line() < 0)return(ERROR);

	/* Print Column Headings */
#ifdef ENGLISH
	if(RepType == 1)
		mkln(1,"- - - - - - - - P E R I O D - - - - - - - -",44);
	else if(RepType == 2)
		mkln(1,"- - - - -  P E R I O D  - - - - -",33);
	else
		mkln(1,"---- P E R I O D ----     Account",33);
#else
	if(RepType == 1)
		mkln(1,"- - - - - - - - P E R I O D E - - - - - - -",44);
	else if(RepType == 2)
		mkln(1,"- - - - -  P E R I O D E- - - - -",33);
	else
		mkln(1,"---- P E R I O D E---       No de",33);
#endif

#ifdef ENGLISH
	if(RepType == 1) {
		mkln(50,"Description",11);
		mkln(78,"- - - - - - Cumulative  To  Date - - - - - -",44);
	}
	else {
		mkln(39,"Description",11);
		mkln(67,"- - - - - - Cumulative  To  Date - - - - - -",44);
		mkln(115,"Annual",6);
	}
#else
	if(RepType == 1) {
		mkln(50,"Description",11);
		mkln(78,"- - - - - - -Cumulatif  a  date- - - - - - -",44);
	}
	else {
		mkln(39,"Description",11);
		mkln(67,"- - - - - - -Cumulatif  a  date- - - - - - -",44);
		mkln(115,"Budget",6);
	}
#endif

#ifdef ENGLISH
	mkln(126,"Budget",6);
#else
	mkln(125,"Solde",5);
#endif
	if(prnt_line() < 0)return(ERROR);

#ifdef ENGLISH
	if(RepType == 1) {
		mkln(2,"Prev. Yr.     Actual     Budget      Diff.",43);
		mkln(79,"Prev. Yr.     Actual     Budget      Diff.",44);
	}
	else {
		mkln(5,"Actual     Budget",20);
		if(RepType == 2)
			mkln(28,"Diff.",5);
		else
			mkln(28,"Number",6);
		mkln(68,"Prev. Yr.     Actual     Budget      Diff.",44);
		mkln(115,"Budget",6);
	}
#else
	if(RepType == 1) {
		mkln(2,"Annee prec.   Actuel     Budget      Diff.",43);
		mkln(79,"Annee prec.   Actuel     Budget      Diff.",44);
	}
	else {
		mkln(5,"Actuel     Budget",20);
		if(RepType == 2)
			mkln(28,"Diff.",5);
		else
			mkln(28,"Compte",6);
		mkln(68,"Annee prec.   Actuel     Budget      Diff.",44);
		mkln(115,"Annuel",6);
	}
#endif

#ifdef ENGLISH
	mkln(125,"Balance",7);
#else
	mkln(124,"du budget",9);
#endif

	if(prnt_line() < 0)return(ERROR);
	if(prnt_line() < 0)return(ERROR);

	return(NOERROR);
}	/* PrntHeadings() */
/*-----------------------------------------------------------------*/
static	int	/* Form Report line for current record and print */
PrntRecord()
{
	double	cum_prev_yr,
		cum_actual,
		cum_budget,
		temp ;
	int	period ;

	cum_prev_yr = Cumulative(rp_sth.s_period, gl_rec.prerel) ;
	cum_actual = Cumulative(rp_sth.s_period, gl_rec.currel) ;
	cum_budget = Cumulative(rp_sth.s_period, gl_rec.curbud) ;

	period = rp_sth.s_period - 1;

	/* IF SW4 is on add commiments to period acutal */
	if(SW4)
		gl_rec.currel[period] += gl_rec.comdat ;

	/* IF SW5 is on add commiments to Cumulative acutal */
	if(SW5)
		cum_actual += gl_rec.comdat ;

	if( gl_rec.reccod == 97) {
		if( gl_rec.cdunit < 1 || gl_rec.cdunit > 10) return(NOERROR) ;

		strncpy(Um_desc[gl_rec.cdunit-1], gl_rec.desc,
				sizeof(gl_rec.desc)) ;
		AddTotals(&Um_total[gl_rec.cdunit-1], gl_rec, cum_prev_yr,
			cum_actual, cum_budget );

		return(NOERROR) ;
	}

	if(linecnt >= PGSIZE)
		if((ret_cd = PrntHeadings()) < 0) return(ret_cd) ;

	if(RepType == 1) {
		tedit((char*)&gl_rec.prerel[period], Mask_10, line+cur_pos,
					R_DOUBLE ) ;
		cur_pos += 11 ;
	}
	tedit((char*)&gl_rec.currel[period], Mask_10, line+cur_pos, R_DOUBLE ) ;
	cur_pos += 11 ;
	tedit((char*)&gl_rec.curbud[period], Mask_10, line+cur_pos, R_DOUBLE ) ;
	cur_pos += 11 ;

	if(RepType == 1 || RepType == 2) {
		temp = gl_rec.curbud[period] - gl_rec.currel[period] ;
		tedit((char*)&temp, Mask_10, line+cur_pos, R_DOUBLE ) ;
		cur_pos += 11 ;
	}
	else
		/* Copy the last 11 charaters */
		mkln(23, gl_rec.accno+(sizeof(gl_rec.accno)-1-11), 11);

	if(RepType ==  1) {
		mkln(46, gl_rec.desc, 30) ;
		mkln(77, " ", 1) ;
	}
	else {
		mkln(35, gl_rec.desc, 30) ;
		mkln(66, " ", 1) ;
	}
	tedit((char*)&cum_prev_yr, Mask_10, line+cur_pos, R_DOUBLE ) ;
	cur_pos += 11 ;
	tedit((char*)&cum_actual, Mask_10, line+cur_pos, R_DOUBLE ) ;
	cur_pos += 11 ;
	tedit((char*)&cum_budget, Mask_10, line+cur_pos, R_DOUBLE ) ;
	cur_pos += 11 ;
	temp = cum_budget - cum_actual ;
	tedit((char*)&temp, Mask_10, line+cur_pos, R_DOUBLE ) ;
	cur_pos += 11 ;

	if(RepType != 1) {
		tedit((char*)&gl_rec.budcur, Mask_10, line+cur_pos, R_DOUBLE ) ;
		cur_pos += 11 ;
	}

	temp = gl_rec.budcur - cum_actual ;
	tedit((char*)&temp, Mask_10, line+cur_pos, R_DOUBLE ) ;
	cur_pos += 11 ;

	if(prnt_line() < 0) return(ERROR);

	prec1 = cum_actual ;	/* Similar to COBOL PROGRAM */
	prec2 = gl_rec.budcur ;

	/* Update Salaries Total */
	if( gl_rec.reccod == 99 && KEY_EXISTS(5) )
		/* Assuming Key 8 is given as a 5th User key */
		if( gl_rec.keys[KEY(5)] >= rp_sth.s_fm_key8 &&
				gl_rec.keys[KEY(5)] <= rp_sth.s_to_key8 )
			AddTotals(&S_total, gl_rec, cum_prev_yr,
				cum_actual, cum_budget) ;

	/* Update the Totals For 98 & 99 Records */

	UpdateTotals(cum_prev_yr, cum_actual, cum_budget) ;

	return(NOERROR) ;
}	/* PrntRecord() */
/*-----------------------------------------------------------------*/
static	int
UpdateTotals(cum_prev_yr, cum_actual, cum_budget)
double	cum_prev_yr ,
	cum_actual ,
	cum_budget ;
{
	/* K6 not zero */
	if( KEY_EXISTS(6) && gl_rec.keys[KEY(6)] )
	    AddTotals(&K6_total,gl_rec, cum_prev_yr, cum_actual, cum_budget) ;

	/* K5 not zero */
	if( KEY_EXISTS(5) && gl_rec.keys[KEY(5)] )
	    AddTotals(&K5_total, gl_rec, cum_prev_yr, cum_actual, cum_budget) ;

	/* K4 not zero */
	if( KEY_EXISTS(4) && gl_rec.keys[KEY(4)] ) {
	    if(gl_rec.reccod == 98) {
		AddTotals(&H_total,gl_rec, cum_prev_yr, cum_actual, cum_budget);

		if( gl_rec.cdunit == 0)
			AddTotals(&K4R98_total, gl_rec, cum_prev_yr,
				cum_actual, cum_budget) ;
		return(NOERROR) ;
	    }
	    else	/* 99 */
		AddTotals(&K4_total,gl_rec,cum_prev_yr, cum_actual, cum_budget);
	}

	/* Reccod == 99 || (K4 == 0 && reccod == 98) */
	AddTotals(&F_total, gl_rec, cum_prev_yr, cum_actual, cum_budget) ;

	return(NOERROR) ;
}	/* UpdateTotals() */
/*-----------------------------------------------------------------*/
static	int
SectTotals()
{
	char	desc[31] ;

	if(linecnt > (PGSIZE -4)) {
		if((ret_cd = PrntHeadings()) < 0) return(ret_cd) ;
	} else
		if(prnt_line() < 0) return(ERROR) ;

	if(prev_sect == EXPENDITURE) {
		if(prev_admis == 0) 
#ifdef ENGLISH
			strncpy(desc,"Expenses Admissible",sizeof(desc));
		else
			strncpy(desc,"Expenses Not Admissible",sizeof(desc));
#else
			strncpy(desc,"Depenses admissibles",sizeof(desc));
		else
			strncpy(desc,"Depenses inadmissibles",sizeof(desc));
#endif
	}
	else {	/* Section INCOME */
		if(prev_admis == 0) 
#ifdef ENGLISH
			strncpy(desc,"Revenue Admissible",sizeof(desc));
		else
			strncpy(desc,"Revenue Not Admissible",sizeof(desc));
#else
			strncpy(desc,"Revenu admissible",sizeof(desc));
		else
			strncpy(desc,"Revenu inadmissible",sizeof(desc));
#endif
	}
	
	if((ret_cd = PrntHyphens()) < 0) return(ret_cd) ;
	if((ret_cd = PrntTotal( F_total, desc )) < 0) return(ret_cd) ;
	if((ret_cd = PrntPercentage( F_total )) < 0) return(ret_cd) ;

	if(prev_sect == EXPENDITURE) {
		if(prev_admis == 0) 
#ifdef ENGLISH
			strncpy(desc,"Hours Admissible",sizeof(desc));
		else
			strncpy(desc,"Hours Not Admissible",sizeof(desc));
#else
			strncpy(desc,"Heures admissibles",sizeof(desc));
		else
			strncpy(desc,"Heures inadmissibles",sizeof(desc));
#endif

		if(linecnt > (PGSIZE -4)) {
			if((ret_cd = PrntHeadings()) < 0) return(ret_cd) ;
		}
		else
			if(prnt_line() < 0) return(ERROR) ;

		if((ret_cd = PrntHyphens()) < 0) return(ret_cd) ;
		if((ret_cd = PrntTotal( H_total, desc )) < 0) return(ret_cd) ;
		if((ret_cd = PrntPercentage( H_total )) < 0) return(ret_cd) ;
		InitTotal( &H_total ) ;
	}

	/* Update Grand Total */
	G_total.per_prev_yr += F_total.per_prev_yr ;
	G_total.per_actual  += F_total.per_actual  ;
	G_total.per_budget  += F_total.per_budget  ;
	G_total.cum_prev_yr += F_total.cum_prev_yr ;
	G_total.cum_actual  += F_total.cum_actual  ;
	G_total.cum_budget  += F_total.cum_budget  ;
	G_total.cum_curbud  += F_total.cum_curbud  ;
	G_total.cum_comdat  += F_total.cum_comdat  ;

	InitTotal( &F_total ) ;

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
	double	temp1, temp2 ;
	char	t_desc[10+sizeof(title_rec.desc)] ;

	if( T6_Exists ) {
		temp1 = K6_total.cum_actual - prec1 ;
		temp2 = K6_total.cum_curbud - prec2 ;

		if( (temp1 < -0.005 || temp1 > 0.005) ||
					(temp2 < -0.005 || temp2 >0.005) ) {
			if(linecnt > (PGSIZE - 4) )
				if((ret_cd = PrntHeadings()) < 0)return(ret_cd);

			if((ret_cd = PrntHyphens()) < 0) return(ret_cd) ;
#ifdef ENGLISH
			STRCPY(t_desc, "Total - ");
#else
			STRCPY(t_desc, "Total - ");
#endif
			strncat(t_desc, T6_desc, sizeof(title_rec.desc));
			if((ret_cd = PrntTotal( K6_total, t_desc )) < 0)
					return(ret_cd) ;
		}
		else {
			if(linecnt > (PGSIZE - 2) )
				if((ret_cd = PrntHeadings()) < 0)return(ret_cd);
		}

		if((ret_cd = PrntPercentage( K6_total )) < 0) return(ret_cd) ;
		FirstTitle = 1 ;
	}

	InitTotal( &K6_total ) ;
	return(NOERROR) ;
}	/* Key6Totals() */
/*-----------------------------------------------------------------*/
/* Print Total 5 */
static	int
Key5Totals()
{
	double	temp1, temp2 ;
	char	t_desc[10+sizeof(title_rec.desc)] ;

	if( T5_Exists ) {
		temp1 = K5_total.cum_actual - prec1 ;
		temp2 = K5_total.cum_curbud - prec2 ;

		if( (temp1 < -0.005 || temp1 > 0.005) ||
					(temp2 < -0.005 || temp2 >0.005) ) {
			if(linecnt > (PGSIZE - 4) )
				if((ret_cd = PrntHeadings()) < 0)return(ret_cd);

			if((ret_cd = PrntHyphens()) < 0) return(ret_cd) ;
#ifdef ENGLISH
			STRCPY(t_desc, "Total - ");
#else
			STRCPY(t_desc, "Total - ");
#endif
			strncat(t_desc, T5_desc, sizeof(title_rec.desc));
			if((ret_cd = PrntTotal(K5_total, t_desc)) < 0)
				return(ret_cd);
		}
		else {
			if(linecnt > (PGSIZE - 2) )
				if((ret_cd = PrntHeadings()) < 0)return(ret_cd);
		}

		if((ret_cd = PrntPercentage( K5_total )) < 0) return(ret_cd) ;
		FirstTitle = 1 ;
	}

	InitTotal( &K5_total ) ;
	return(NOERROR) ;
}	/* Key5Totals() */
/*-----------------------------------------------------------------*/
/* Print Total 4 */
static	int
Key4Totals()
{
	int	i ;
	char	t_desc[10+sizeof(title_rec.desc)] ;

	if(linecnt > (PGSIZE - 4) ) {
		if((ret_cd = PrntHeadings()) < 0)return(ret_cd);
	}
	else
		if(prnt_line() < 0) return(ERROR) ;

	if((ret_cd = PrntHyphens()) < 0) return(ret_cd) ;
#ifdef ENGLISH
	STRCPY(t_desc, "Total - ");
#else
	STRCPY(t_desc, "Total - ");
#endif
	strncat(t_desc, T4_desc, sizeof(title_rec.desc));
	if((ret_cd = PrntTotal( K4_total, t_desc )) < 0) return(ret_cd) ;

	if((ret_cd = PrntPercentage( K4_total )) < 0) return(ret_cd) ;

	if((ret_cd = PrntAvgTime()) < 0) return(ret_cd) ;
	if((ret_cd = PrntUnitCosts()) < 0) return(ret_cd) ;

	InitTotal( &K4_total ) ;
	InitTotal( &K4R98_total ) ;
	InitTotal( &S_total ) ;
	for(i = 0 ; i < 10 ; i++) {
		InitTotal( &Um_total[i] );
		Um_desc[i][0] = '\0' ;
	}

	FirstTitle = 1 ;

	return(NOERROR) ;
}	/* Key4Totals() */
/*-----------------------------------------------------------------*/
static	int
PrntPercentage( tot_struct )
Btotal	tot_struct ;
{
	double	temp, temp1 ;

	if((ret_cd = PrntHyphens()) < 0) return(ret_cd) ;

	/* Print Percentage line */
	/* if Per Budget not zero then print % of budget diff */
	if( (RepType == 1 || RepType == 2) &&
	    (tot_struct.per_budget < -0.005 || tot_struct.per_budget > 0.005)) {
		/* Budget Difference */
		temp1 = tot_struct.per_budget - tot_struct.per_actual ;
		temp = (temp1 / tot_struct.per_budget) * 100 ;
		if(temp <= -0.0001 || temp >= 0.0001) {
			if(RepType == 1)
				mkln(33, " ", 1);
			else
				mkln(22, " ", 1);
			tedit((char*)&temp, Mask_7_4, line+cur_pos, R_DOUBLE) ;
			cur_pos += 9 ;
			mkln(cur_pos+1, "%", 1);
		}
	}

	/* if Cumulative Budget not zero then print % of budget diff */
	if( tot_struct.cum_budget < -0.005 || tot_struct.cum_budget > 0.005) {
		/* Budget Difference */
		temp1 = tot_struct.cum_budget - tot_struct.cum_actual ;
		temp = (temp1 / tot_struct.cum_budget) * 100 ;
		if(temp <= -0.0001 || temp >= 0.0001) {
			if(RepType == 1)
				mkln(110, " ", 1);
			else
				mkln(99, " ", 1);
			tedit((char*)&temp, Mask_7_4, line+cur_pos, R_DOUBLE) ;
			cur_pos += 9 ;
			mkln(cur_pos+1, "%", 1);
		}
	}

	/* if Currnt Budget not zero then print % of budget Balance */
	if( tot_struct.cum_curbud < -0.005 || tot_struct.cum_curbud > 0.005) {
		/* Budget Balnace */
		temp1 = tot_struct.cum_curbud - tot_struct.cum_actual ;
		temp = (temp1 / tot_struct.cum_curbud) * 100 ;
		if(temp <= -0.0001 || temp >= 0.0001) {
			mkln(121, " ", 1);
			tedit((char*)&temp, Mask_7_4, line+cur_pos, R_DOUBLE) ;
			cur_pos += 9 ;
			mkln(cur_pos+1, "%", 1);
		}
	}
	if(cur_pos > 1)
		if(prnt_line() < 0) return(ERROR) ;

	if(linecnt < PGSIZE)
		if(prnt_line() < 0) return(ERROR) ;

	return(NOERROR) ;
}	/* PrntPercentage() */
/*-----------------------------------------------------------------*/
static	int	/* Print Hyphens line */
PrntHyphens()
{
	mkln(1, Hyphens, 10) ;
	mkln(12, Hyphens, 10) ;
	if(RepType == 1 || RepType == 2)
		mkln(23, Hyphens, 10) ;
	if(RepType == 1)
		mkln(34, Hyphens, 10) ;
	if(RepType != 1)
		mkln(67, Hyphens, 10) ;
	mkln(78, Hyphens, 10) ;
	mkln(89, Hyphens, 10) ;
	mkln(100, Hyphens, 10) ;
	mkln(111, Hyphens, 10) ;
	mkln(122, Hyphens, 10) ;
	if(prnt_line() < 0) return(ERROR) ;

	return(NOERROR) ;
}	/* PrntHyphens() */
/*-----------------------------------------------------------------*/
static	int
PrntTotal( tot_struct, desc )
Btotal	tot_struct ;
char	*desc ;
{
	double	temp ;

	if(RepType == 1) {
		tedit((char*)&tot_struct.per_prev_yr, Mask_10,
				line+cur_pos, R_DOUBLE );
		cur_pos += 11 ;
	}
	tedit((char*)&tot_struct.per_actual, Mask_10, line+cur_pos, R_DOUBLE ) ;
	cur_pos += 11 ;
	tedit((char*)&tot_struct.per_budget, Mask_10, line+cur_pos, R_DOUBLE ) ;
	cur_pos += 11 ;

	if(RepType == 1 || RepType == 2) {
		temp = tot_struct.per_budget - tot_struct.per_actual ;
		tedit((char*)&temp, Mask_10, line+cur_pos, R_DOUBLE ) ;
		cur_pos += 11 ;
	}

	if(RepType == 1) {
		mkln(46, desc,30) ;
		mkln(77, " ", 1) ;
	}
	else {
		mkln(35, desc,30) ;
		mkln(66, " ", 1) ;
	}
	tedit((char*)&tot_struct.cum_prev_yr, Mask_10, line+cur_pos, R_DOUBLE );
	cur_pos += 11 ;
	tedit((char*)&tot_struct.cum_actual, Mask_10, line+cur_pos, R_DOUBLE ) ;
	cur_pos += 11 ;
	tedit((char*)&tot_struct.cum_budget, Mask_10, line+cur_pos, R_DOUBLE ) ;
	cur_pos += 11 ;
	temp = tot_struct.cum_budget - tot_struct.cum_actual ;
	tedit((char*)&temp, Mask_10, line+cur_pos, R_DOUBLE ) ;
	cur_pos += 11 ;

	if(RepType != 1) {
		tedit((char*)&tot_struct.cum_curbud, Mask_10,
					line+cur_pos, R_DOUBLE ) ;
		cur_pos += 11 ;
	}
	temp = tot_struct.cum_curbud - tot_struct.cum_actual ;
	tedit((char*)&temp, Mask_10, line+cur_pos, R_DOUBLE ) ;
	cur_pos += 11 ;

	if(prnt_line() < 0) return(ERROR);

	return(NOERROR) ;
}	/* PrntTotal */
/*-----------------------------------------------------------------*/
static	int	/* Print Average time */
PrntAvgTime()
{
	/* if Cum-Actual == 0 && Cum Current-BUdget == 0 */
	if(K4R98_total.cum_actual > -0.005 && K4R98_total.cum_actual < 0.005 &&
	    K4R98_total.cum_curbud > -0.005 && K4R98_total.cum_curbud < 0.005)
		return(NOERROR) ;

	if(linecnt >= PGSIZE)
		if((ret_cd = PrntHeadings()) < 0)return(ret_cd);

#ifdef ENGLISH
	if((ret_cd = PrntAverages(S_total, K4R98_total, "Total Average Time"))
#else
	if((ret_cd = PrntAverages(S_total, K4R98_total, "Temps moyen total"))
#endif
			< 0) return(ret_cd) ;
	return(NOERROR) ;
}	/* PrntAvgTime() */
/*-----------------------------------------------------------------*/
static	int	/* Print Unit Costs */
PrntUnitCosts()
{
	int	i ;
	char	desc[31] ;
	double	temp1, temp2 ;

	for( i = 0 ; i < 2 ; i++) {
		if(Um_desc[i][0] == '\0') continue ;

		if(linecnt >= PGSIZE)
			if((ret_cd = PrntHeadings()) < 0)return(ret_cd);

		if((ret_cd = PrntTotal( Um_total[i], Um_desc[i])) < 0)
				return(ret_cd) ;

		/* if Cum-Actual != 0 or Cum Current-BUdget != 0 */
		if( !(Um_total[i].cum_actual > -0.005 &&
					Um_total[i].cum_actual < 0.005) ||
			!(Um_total[i].cum_curbud > -0.005 &&
					Um_total[i].cum_curbud < 0.005)) {

			if(linecnt > (PGSIZE - 4))
				if((ret_cd = PrntHeadings()) < 0)return(ret_cd);

#ifdef ENGLISH
			if((ret_cd = PrntAverages(K4_total, Um_total[i],
				"Unit Costs")) < 0) return(ret_cd) ;
#else
			if((ret_cd = PrntAverages(K4_total, Um_total[i],
				"Cout par unite")) < 0) return(ret_cd) ;
#endif

#ifdef ENGLISH
			STRCPY(desc,"Hrs. Remun./");
#else
			STRCPY(desc,"Hrs. remun./");
#endif
			strncat(desc, Um_desc[i], 18) ;
			if((ret_cd = PrntAverages(K4R98_total, Um_total[i],
					desc)) < 0) return(ret_cd) ;

#ifdef ENGLISH
			if(RepType == 1)
				mkln(46, "Variations: Due To Volume", 30) ;
			else
				mkln(35, "Variations: Due To Volume", 30) ;
#else
			if(RepType == 1)
				mkln(46, "Variations: dues au volume", 30) ;
			else
				mkln(35, "Variations: dues au volume", 30) ; 
#endif

			if(Um_total[i].cum_budget < -0.005 ||
					Um_total[i].cum_budget > 0.005) {
			    temp1= K4_total.cum_budget / Um_total[i].cum_budget;
			    temp1 = temp1 *
			      (Um_total[i].cum_budget - Um_total[i].cum_actual);
			}
			else
			    temp1 = 0.0  ;

			if(RepType == 1)
				mkln(110, " ", 1);
			else
				mkln(99, " ", 1);
			tedit((char*)&temp1, Mask_10, line+cur_pos, R_DOUBLE) ;
			cur_pos += 11 ;
			if(prnt_line() < 0) return(ERROR) ;

#ifdef ENGLISH
			if(RepType == 1)
				mkln(46, "Variations: Due To Price", 30) ;
			else
				mkln(35, "Variations: Due To Price", 30) ;
#else
			if(RepType == 1)
				mkln(46, "Variations: dues au prix", 30) ;
			else
				mkln(35, "Variations: dues au prix", 30) ;
#endif

			if(Um_total[i].cum_actual < -0.005 ||
					Um_total[i].cum_actual > 0.005)
			    temp2= K4_total.cum_actual / Um_total[i].cum_actual;
			else
			    temp2 = 0.0  ;

			temp1 -= temp2 ;
			temp2 = temp2 * Um_total[i].cum_actual ;

			if(RepType == 1)
				mkln(110, " ", 1);
			else
				mkln(99, " ", 1);
			tedit((char*)&temp2, Mask_10, line+cur_pos, R_DOUBLE) ;
			cur_pos += 11 ;
			if(prnt_line() < 0) return(ERROR) ;

			if(linecnt < PGSIZE)
				if(prnt_line() < 0) return(ERROR) ;
		}
	}	/* For () */

	for( ; i < 10 ; i++) {
		if(Um_desc[i][0] == '\0') continue ;

		if(linecnt > (PGSIZE - 2)) {
			if((ret_cd = PrntHeadings()) < 0)return(ret_cd);
		}
		else
			if(prnt_line() < 0) return(ERROR) ;

		if((ret_cd = PrntTotal(Um_total[i], Um_desc[i])) < 0)
			return(ret_cd) ;
	}

	return(NOERROR) ;
}	/* PrntUnitCosts() */
/*-----------------------------------------------------------------*/
static	int	/* Print Averages */
PrntAverages( tot_st1, tot_st2, desc)
Btotal	tot_st1 ,
	tot_st2 ;
char	*desc ;
{
	double	temp1, temp2 ;

	if(RepType == 1) {
		if(tot_st2.per_prev_yr < -0.005 || tot_st2.per_prev_yr > 0.005)
			temp1 = tot_st1.per_prev_yr / tot_st2.per_prev_yr ;
		else
			temp1 = 0.0 ;
		tedit((char*)&temp1, Mask_7_4, line+cur_pos, R_DOUBLE) ;
		cur_pos += 9 ;
	}

	if(tot_st2.per_actual < -0.005 || tot_st2.per_actual > 0.005)
		temp1 = tot_st1.per_actual / tot_st2.per_actual ;
	else
		temp1 = 0.0 ;
	if(RepType == 1)
		mkln(11, " ", 1);
	tedit((char*)&temp1, Mask_7_4, line+cur_pos, R_DOUBLE) ;
	cur_pos += 9 ;

	if(tot_st2.per_budget < -0.005 || tot_st2.per_budget > 0.005)
		temp2 = tot_st1.per_budget / tot_st2.per_budget ;
	else
		temp2 = 0.0 ;
	if(RepType == 1)
		mkln(22, " ", 1);
	else
		mkln(11, " ", 1);
	tedit((char*)&temp2, Mask_7_4, line+cur_pos, R_DOUBLE) ;
	cur_pos += 9 ;

	if(RepType == 1 || RepType == 2) {
		temp2 -= temp1 ;
		if(RepType == 1)
			mkln(33, " ", 1);
		else
			mkln(22, " ", 1);
		tedit((char*)&temp2, Mask_7_4, line+cur_pos, R_DOUBLE) ;
		cur_pos += 9 ;
	}

	if(RepType == 1)
		mkln(46, desc, 30) ;
	else
		mkln(35, desc, 30) ;

	if(tot_st2.cum_prev_yr < -0.005 || tot_st2.cum_prev_yr > 0.005)
		temp1 = tot_st1.cum_prev_yr / tot_st2.cum_prev_yr ;
	else
		temp1 = 0.0 ;
	if(RepType == 1)
		mkln(77, " ", 1);
	else
		mkln(66, " ", 1);
	tedit((char*)&temp1, Mask_7_4, line+cur_pos, R_DOUBLE) ;
	cur_pos += 9 ;

	if(tot_st2.cum_actual < -0.005 || tot_st2.cum_actual > 0.005)
		temp1 = tot_st1.cum_actual / tot_st2.cum_actual ;
	else
		temp1 = 0.0 ;
	if(RepType == 1)
		mkln(88, " ", 1);
	else
		mkln(77, " ", 1);
	tedit((char*)&temp1, Mask_7_4, line+cur_pos, R_DOUBLE) ;
	cur_pos += 9 ;

	if(tot_st2.cum_budget < -0.005 || tot_st2.cum_budget > 0.005)
		temp2 = tot_st1.cum_budget / tot_st2.cum_budget ;
	else
		temp2 = 0.0 ;
	if(RepType == 1)
		mkln(99, " ", 1);
	else
		mkln(88, " ", 1);
	tedit((char*)&temp2, Mask_7_4, line+cur_pos, R_DOUBLE) ;
	cur_pos += 9 ;

	temp2 -= temp1 ;
	if(RepType == 1)
		mkln(110, " ", 1);
	else
		mkln(99, " ", 1);
	tedit((char*)&temp2, Mask_7_4, line+cur_pos, R_DOUBLE) ;
	cur_pos += 9 ;

	if(prnt_line() < 0) return(ERROR) ;

	return(NOERROR) ;
}	/* PrntAverages() */
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

		prev_admis = gl_rec.admis ;
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

	if(RepType == 1)
		i = mkln(46,title_rec.desc,30) ;
	else
		i = mkln(35,title_rec.desc,30) ;
	if(prnt_line() < 0) return(ERROR) ;
	/* Print Under line */
	if(RepType == 1)
		mkln(46,Hyphens,i) ;
	else
		mkln(35,Hyphens,i) ;
	if(prnt_line() < 0) return(ERROR) ;
	if(prnt_line() < 0) return(ERROR) ;

	return(NOERROR) ;
}	/* PrntSubTitle() */
/*----------------------------------------------------------------------*/
static	int
AddTitle()
{
	double	cum_budget ;

	if(FirstTitle == 0 &&
		(title_rec.budcur < -0.005 && title_rec.budcur > 0.005)){

		if(linecnt >= PGSIZE)
			if((ret_cd = PrntHeadings()) < 0)return(ret_cd);

		title_rec.prerel[rp_sth.s_period-1] = 0.0 ;
		title_rec.currel[rp_sth.s_period-1] = 0.0 ;

		cum_budget = Cumulative(rp_sth.s_period, title_rec.curbud) ;

		if(RepType == 1)
			mkln(22, " ", 1) ;
		else
			mkln(11, " ", 1) ;
		tedit((char*)&title_rec.curbud[rp_sth.s_period-1],
				Mask_10, line+cur_pos, R_DOUBLE ) ;
		cur_pos += 11 ;

		/* Diffrence will be same as budget */
		if(RepType == 1 || RepType == 2) {
			tedit((char*)&title_rec.curbud[rp_sth.s_period-1],
				Mask_10, line+cur_pos, R_DOUBLE ) ;
			cur_pos += 11 ;
		}

		if(RepType == 1) {
			mkln(46, title_rec.desc, 30) ;
			mkln(99, " ", 1) ;
		}
		else {
			mkln(35, title_rec.desc, 30) ;
			mkln(88, " ", 1) ;
		}
		tedit((char*)&cum_budget, Mask_10, line+cur_pos, R_DOUBLE ) ;
		cur_pos += 11 ;
		tedit((char*)&cum_budget, Mask_10, line+cur_pos, R_DOUBLE ) ;
		cur_pos += 11 ;
		if(RepType != 1) {
			tedit((char*)&title_rec.budcur, Mask_10,
					line+cur_pos, R_DOUBLE ) ;
			cur_pos += 11 ;
		}
		tedit((char*)&title_rec.budcur, Mask_10,
				line+cur_pos, R_DOUBLE);
		cur_pos += 11 ;

		if(prnt_line() < 0) return(ERROR);

		/* K6 not zero */
		if( KEY_EXISTS(6) && title_rec.keys[KEY(6)] )
		    AddTotals(&K6_total, title_rec, 0.0, 0.0, cum_budget) ;

		/* K5 not zero */
		if( KEY_EXISTS(5) && title_rec.keys[KEY(5)] )
		    AddTotals(&K5_total, title_rec, 0.0, 0.0, cum_budget) ;

		/* K4 not zero */
		if( KEY_EXISTS(4) && title_rec.keys[KEY(4)] )
		    AddTotals(&K4_total, title_rec, 0.0, 0.0, cum_budget) ;

		AddTotals(&F_total, title_rec, 0.0, 0.0, cum_budget) ;

	}
	else
		FirstTitle = 0 ;

	return(NOERROR) ;
}	/* AddTitle() */
/*-------------------- E n d   O f   P r o g r a m ---------------------*/
