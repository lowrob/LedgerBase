/*-----------------------------------------------------------------------
Source Name: bdgt_sum.c
System     : Budgetary Financial system.
Module     : General Ledger system.
Created  On: 21st Jul 89.
Created  By: T AMARENDRA.

COBOL Source(s): gl172f--lg

DESCRIPTION:
	Function to Print Summary Budget Reports.

Usage of SWITCHES when they are ON :
	SW1 :
	SW2 :
		Not Used.
	SW3 :	(ALL_KEYS)
		All Keys. Title Key formation changes when it is ON.
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
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

------------------------------------------------------------------------*/

#include <stdio.h>
#include <bdgt_rep.h>	/* Reports Variables & Definitions */

/* NOTE: see bdgt_rep.h for all comman definitions */

/* local variables */

/* Active Keys values */
static	long	prev_sk1 ;	/* Prev Sort Key1 */
static	long	prev_sk2 ;	/* Prev Sort Key 2 */
static	long	prev_sk3 ;	/* Prev Sort Key 3 */
static	short	prev_admis ;
static	short	prev_sect ;

static	char	T1_desc[sizeof(title_rec.desc)] ,
		T2_desc[sizeof(title_rec.desc)] ,
		T3_desc[sizeof(title_rec.desc)] ;

static	double	prec1 , prec2 ;	/* Similar to COBOL PROGRAM. Total-6 line
				   printing will be skipped if the totals are
				   same as previous line */

/* Totalling variables */

static	Btotal	G_total ;	/* Grand Total */
static	Btotal	F_total ;	/* Section Total */
static	Btotal	K1_total ;	/* Sort K1 != 0 */
static	Btotal	K2_total ;	/* Sort K2 != 0 */
static	Btotal	K3_total ;	/* Sort K3 != 0 */

static	int	RepType ;	/* 1 - Detailed, 2 - Annual and
				    3 - With Account */

extern	short	SortKeys[SUM_SORTS][3] ;	/* Sorts User keys */

/*-----------------------------------------------------------------------*/
BdgtSummary(option)	/* Budget Summary Reports */
int	option ;
{
	/* select the report options */
	ret_cd = GetOptions();
	if(ret_cd != NOERROR) return(ret_cd) ;

	RepType = option ;

	prec1 = prec2 = 0.0 ;

	InitTotal( &G_total	) ;
	InitTotal( &F_total	) ;
	InitTotal( &K1_total	) ;
	InitTotal( &K2_total	) ;
	InitTotal( &K3_total	) ;

	/* Read the Budget Data (SEQDATA) file */
	/* Set the file to 1st record */
	flg_reset(TMPINDX_2);
	gl_rec.funds = rp_sth.s_fm_fund ;
	if(RepType == 8)		/* Expenses Only */
		gl_rec.sect = EXPENDITURE ;
	else if(RepType == 9)		/* Revenue Only */
		gl_rec.sect = INCOME ;
	else
		gl_rec.sect = 0 ;
	gl_rec.admis = 0 ;
	if( SK1 && KEY_EXISTS(SK1) )
		gl_rec.keys[ KEY( SK1 ) ] = 0 ;
	else
		gl_rec.reccod = 0;
	prev_sk1 = 0 ;
	prev_sk2 = 0 ;
	prev_sk3 = 0 ;

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

		if( RepType == 8 ) {		/* Expenses Report */
			if(gl_rec.sect != EXPENDITURE) continue ;
		}
		else if(RepType == 9) {	/* Revenue Report */
			if(gl_rec.sect != INCOME) continue ;
		}
		else {			/* Others - Revenue & Expenditure Rep */
			if(gl_rec.sect != EXPENDITURE && gl_rec.sect != INCOME)
				continue ;
		}

		if(gl_rec.reccod != 99) continue ;

		if(gl_rec.funds != ctl_rec.fund) {
			if(ctl_rec.fund != -1) {
				/*** Key Totals printing not there in COBOL Pgm 
				if((ret_cd = KeyTotals(1)) < 0) return(ret_cd) ;
				***/
				if((ret_cd = GrandTotal()) < 0) return(ret_cd) ;
			}
			ret_cd = GetFund() ;
			if(ret_cd != NOERROR) return(ret_cd) ;

			ret_cd = GetTitles(0) ;
			if(ret_cd != NOERROR) return(ret_cd) ;
		}
		else if(gl_rec.admis != prev_admis ||
						gl_rec.sect != prev_sect) {
			if((ret_cd = KeyTotals(1)) < 0) return(ret_cd) ;
			if((ret_cd = SectTotals()) < 0) return(ret_cd) ;

			ret_cd = GetTitles(0) ;
			if(ret_cd != NOERROR) return(ret_cd) ;
		}
		else if(SK1 && KEY_EXISTS( SK1 ) && T1_desc[0] != '\0' &&
					gl_rec.keys[KEY( SK1 )] != prev_sk1 ) {
			if((ret_cd = KeyTotals(1)) < 0) return(ret_cd) ;

			/* If all the sort keys exists, then do page break
			   whenever SK1 changes */
			if(SK2 && SK3)
				ret_cd = GetTitles(0) ;
			else
				ret_cd = GetTitles(1) ;
			if(ret_cd != NOERROR) return(ret_cd) ;
		}
		else if(SK2 && KEY_EXISTS( SK2 ) && T2_desc[0] != '\0' &&
					gl_rec.keys[KEY( SK2 )] != prev_sk2 ) {
			if((ret_cd = KeyTotals(2)) < 0) return(ret_cd) ;

			ret_cd = GetTitles(2) ;
			if(ret_cd != NOERROR) return(ret_cd) ;
		}
		else if(SK3 && KEY_EXISTS( SK3 ) && T3_desc[0] != '\0' &&
					gl_rec.keys[KEY( SK3 )] != prev_sk3 ) {
			if((ret_cd = KeyTotals(3)) < 0) return(ret_cd) ;

			ret_cd = GetTitles(3) ;
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
	if((ret_cd = KeyTotals(1)) < 0) return(ret_cd) ;
	if(prnt_line() < 0) return(ERROR) ;

	if((ret_cd = SectTotals()) < 0) return(ret_cd) ;

	if(linecnt > (PGSIZE - 4) )
		if((ret_cd = PrntHeadings()) < 0) return(ret_cd) ;

	if((ret_cd = PrntHyphens()) < 0) return(ret_cd) ;
#ifdef ENGLISH
	if((ret_cd = PrntTotal( G_total, 0, "Fund Total" )) < 0)
		return(ret_cd) ;
#else
	if((ret_cd = PrntTotal( G_total, 0, "Total du fond" )) < 0)
		return(ret_cd) ;
#endif
	if((ret_cd = PrntPercentage( G_total )) < 0) return(ret_cd) ;

	InitTotal( &G_total ) ;
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

	switch(rp_sth.s_sort_no) {
#ifdef ENGLISH
	case	1 :
		mkln(53, "OPERATING  BUDGET  SUMMARY", 26) ;
		break ;
	case	2 :
		mkln(53, "OPERATING  BUDGET  REPORT", 25) ;
		break ;
	case	3 :
		mkln(55, "SCHOOL  BUDGET  REPORT", 22) ;
		break ;
	case	4 :
		mkln(49, "BUDGET REPORT BY MAJOR OBJECT CODE", 34) ;
		break ;
	case	5 :
		mkln(48, "BUDGET REPORT BY PRIMARY OBJECT CODE", 36) ;
		break ;
	case	6 :
		mkln(53, "??????????????????????????", 26) ;
		break ;
	}
#else
	case	1 :
		mkln(51, "RESUME DU BUDGET OPERATIONNEL", 29) ;
		break ;
	case	2 :
		mkln(51, "RAPPORT DU BUDGET OPERATIONNEL", 30) ;
		break ;
	case	3 :
		mkln(55, "RAPPORT DU BUDGET D'ECOLE", 25) ;
		break ;
	case	4 :
		mkln(45, "RAPPORT DE BUDGET PAR CODE D'OBJET MAJEUR", 41) ;
		break ;
	case	5 :
		mkln(44, "RAPPORT DE BUDGET PAR CODE D'OBJET PRIMAIRE", 43) ;
		break ;
	case	6 :
		mkln(53, "??????????????????????????", 26) ;
		break ;
	}
#endif

	if(prnt_line() < 0)return(ERROR);

#ifdef ENGLISH
	mkln(59,"For Period: ",12);
#else
	mkln(56,"Pour la periode: ",17);
#endif
	tedit((char*)&rp_sth.s_period,Mask_2,line+cur_pos,R_SHORT);
	cur_pos += 2 ;
	if(prnt_line() < 0)return(ERROR);

	/* Centre the Fund Before Printing */
#ifdef ENGLISH
	sprintf(t_buf,"%s:  %d  %s",(COMPANY) ? "CO." : "Fund",
#else
	sprintf(t_buf,"%s:  %d  %s",(COMPANY) ? "Cie" : "Fond",
#endif
			ctl_rec.fund, ctl_rec.desc);
	i = strlen(t_buf);
	mkln(((LNSZ - i) / 2),t_buf, sizeof(t_buf));
	if(prnt_line() < 0)return(ERROR);
	if(prnt_line() < 0)return(ERROR);

	/* Print Column Headings */

	/* Expenses or Revenue or (Expenses & Revenue) Report */
	if(RepType == 8 || RepType == 9 || RepType == 10) {
#ifdef ENGLISH
		mkln(1,"---- P E R I O D ----",21);
		mkln(26,"Account",7) ;
#else
		mkln(1,"----P E R I O D E----",21);
		mkln(26,"Numero",6) ;
#endif
	}
	else
#ifdef ENGLISH
		mkln(3,"Account",7) ;
#else
		mkln(3,"Numero",6) ;
#endif

	/* Expenses or Revenue or (Expenses & Revenue) Report */
	if(RepType == 8 || RepType == 9 || RepType == 10) {
#ifdef ENGLISH
		mkln(39,"Description",11);
		mkln(67,"- - - - - - Cumulative  To  Date - - - - - -",44);
#else
		mkln(39,"Description",11);
		mkln(67,"- - - - - - -Cumulatif  a  date- - - - - - -",44);
#endif
	}
	else {
#ifdef ENGLISH
		mkln(17,"Description",11);
		mkln(45,"---- P E R I O D ----",21);
#else
		mkln(17,"Description",11);
		mkln(45,"----P E R I O D E----",21);
#endif
		if(RepType == 6) {	/* Modified Format */
#ifdef ENGLISH
			mkln(70, "Encumb.",7);
			mkln(78,"----- Cumulative  To  Date -----",33);
#else
			mkln(70, "Montant",7);
			mkln(78,"-------Cumulatif  a  date-------",33);
#endif
		}
		else		/* 'G' - French Format */
#ifdef ENGLISH
		    mkln(67,"- - - - - - Cumulative  To  Date - - - - - -",44);
#else
		    mkln(67,"- - - - - - -Cumulatif  a  date- - - - - - -",44);
#endif
	}

#ifdef ENGLISH
	mkln(115,"Annual",6);
	mkln(126,"Budget",6);
#else
	mkln(115,"Budget",6);
	mkln(125," Solde ",7);
#endif
	if(prnt_line() < 0)return(ERROR);

	/* Column headings 2nd line */

	/* Expenses or Revenue or (Expenses & Revenue) Report */
	if(RepType == 8 || RepType == 9 || RepType == 10) {
#ifdef ENGLISH
		mkln(5,"Actual     Budget",20);
		mkln(27,"Number",6) ;
		mkln(68,"Prev. Yr.     Actual     Budget      Diff.",44);
#else
		mkln(5,"Actuel     Budget",20);
		mkln(26,"de compte",9) ;
		mkln(67,"Annee prec     Actuel     Budget      Diff.",44);
#endif
	}
	else {
#ifdef ENGLISH
		mkln(4,"Number",6) ;
#else
		mkln(3,"de compte",9) ;
#endif
		if(RepType == 6) {	/* Modified Format */
#ifdef ENGLISH
			mkln(46,"Prev. Yr.  Curr. Yr.",20);
			mkln(71, "Amount",6);
			mkln(79,"Prev. Yr.  Curr. Yr.     Budget",31);
#else
			mkln(45,"Annee prec Annee cour",21);
			mkln(70, "Engage.",7);
			mkln(78,"Annee prec Annee cour    Budget",31);
#endif
		}
		else {		/* 'G' - French Format */
#ifdef ENGLISH
		    mkln(49,"Actual     Budget",17);
		    mkln(68,"Prev. Yr.  Curr. Yr.     Budget      Diff.",42);
#else
		    mkln(49,"Actuel     Budget",17);
		    mkln(67,"Annee prec Annee cour   Budget      Diff.",41);
#endif
		}
	}

#ifdef ENGLISH
	mkln(115,"Budget",6);
	mkln(125,"Balance",7);
#else
	mkln(115,"Annuel",6);
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

	prec1 = cum_actual ;	/* Similar to COBOL PROGRAM */
	prec2 = gl_rec.budcur ;

	/* If lowest level doesn't have totalling (no Title) */
	if( (SK3 && T3_desc[0] == '\0') ||
			(SK2 && T2_desc[0] == '\0') ||
			(SK1 && T1_desc[0] == '\0') ) {	/* Print Line */

		if(linecnt >= PGSIZE)
			if((ret_cd = PrntHeadings()) < 0) return(ret_cd) ;

		/* Expenses or Revenue or (Expenses & Revenue) Report */
		if(RepType == 8 || RepType == 9 || RepType == 10) {
			tedit((char*)&gl_rec.currel[period], Mask_10,
				line+cur_pos, R_DOUBLE ) ;
			cur_pos += 11 ;
			tedit((char*)&gl_rec.curbud[period], Mask_10,
				line+cur_pos, R_DOUBLE ) ;
			cur_pos += 11 ;
			/* Copy the last 11 charaters */
			mkln(23, gl_rec.accno+(sizeof(gl_rec.accno)-1-11), 11);
			mkln(35, gl_rec.desc, 30) ;
			mkln(66, " ", 1) ;
		}
		else {
			/* Copy the last 11 charaters */
			mkln(1, gl_rec.accno+(sizeof(gl_rec.accno)-1-11), 11);
			mkln(13, gl_rec.desc, 30) ;
			mkln(44, " ", 1) ;
			if(RepType == 6) {	/* Modified Format */
				tedit((char*)&gl_rec.prerel[period], Mask_10,
					line+cur_pos, R_DOUBLE ) ;
				cur_pos += 11 ;
				tedit((char*)&gl_rec.currel[period], Mask_10,
					line+cur_pos, R_DOUBLE ) ;
				cur_pos += 11 ;
				tedit((char*)&gl_rec.comdat, Mask_10,
					line+cur_pos, R_DOUBLE ) ;
				cur_pos += 11 ;
			}
			else {
				tedit((char*)&gl_rec.currel[period], Mask_10,
					line+cur_pos, R_DOUBLE ) ;
				cur_pos += 11 ;
				tedit((char*)&gl_rec.curbud[period], Mask_10,
					line+cur_pos, R_DOUBLE ) ;
				cur_pos += 11 ;
			}
		}

		tedit((char*)&cum_prev_yr, Mask_10, line+cur_pos, R_DOUBLE ) ;
		cur_pos += 11 ;
		tedit((char*)&cum_actual, Mask_10, line+cur_pos, R_DOUBLE ) ;
		cur_pos += 11 ;
		tedit((char*)&cum_budget, Mask_10, line+cur_pos, R_DOUBLE ) ;
		cur_pos += 11 ;
		if(RepType != 6) {	/* Non Modified Foramt */
			temp = cum_budget - cum_actual ;
			tedit((char*)&temp, Mask_10, line+cur_pos, R_DOUBLE ) ;
			cur_pos += 11 ;
		}

		tedit((char*)&gl_rec.budcur, Mask_10, line+cur_pos, R_DOUBLE ) ;
		cur_pos += 11 ;

		temp = gl_rec.budcur - cum_actual ;
		tedit((char*)&temp, Mask_10, line+cur_pos, R_DOUBLE ) ;
		cur_pos += 11 ;

		if(prnt_line() < 0) return(ERROR);
	}

	UpdateTotals(cum_prev_yr, cum_actual, cum_budget) ;

	return(NOERROR) ;
}	/* PrntRecord() */
/*-----------------------------------------------------------------*/
static	int
UpdateTotals(cum_prev_yr, cum_actual, cum_budget)
double	cum_prev_yr,
	cum_actual,
	cum_budget ;
{
	/* Sort K3 not = zero */
	if( SK3 && KEY_EXISTS( SK3 ) &&
			T3_desc[0] != '\0' && gl_rec.keys[KEY( SK3 )] )
	    AddTotals(&K3_total,gl_rec, cum_prev_yr, cum_actual, cum_budget) ;

	/* Sort K2 not zero */
	if( SK2 && KEY_EXISTS( SK2 ) &&
			T2_desc[0] != '\0' && gl_rec.keys[KEY( SK2 )] )
	    AddTotals(&K2_total, gl_rec, cum_prev_yr, cum_actual, cum_budget) ;

	/* Sort K1 not zero */
	if( SK1 && KEY_EXISTS( SK1 ) &&
			T1_desc[0] != '\0' && gl_rec.keys[KEY( SK1 )] )
		AddTotals(&K1_total,gl_rec,cum_prev_yr, cum_actual, cum_budget);

	AddTotals(&F_total, gl_rec, cum_prev_yr, cum_actual, cum_budget) ;

	return(NOERROR) ;
}	/* UpdateTotals() */
/*-----------------------------------------------------------------*/
static	int
SectTotals()
{
	char	desc[35] ;

	if(linecnt > (PGSIZE - 4)) {
		if((ret_cd = PrntHeadings()) < 0) return(ret_cd) ;
	} else
		if(prnt_line() < 0) return(ERROR) ;

	if(prev_sect == EXPENDITURE) {
		if(prev_admis == 0) 
#ifdef ENGLISH
			strncpy(desc,"Total - Admissible Expenses",
							sizeof(desc));
		else
			strncpy(desc,"Total - Inadmissible Expenses",
							sizeof(desc));
#else
			strncpy(desc,"Total - Depenses admissibles",
							sizeof(desc));
		else
			strncpy(desc,"Total - Depenses inadmissibles",
							sizeof(desc));
#endif
	}
	else {	/* Section INCOME */
		if(prev_admis == 0) 
#ifdef ENGLISH
			strncpy(desc,"Total - Admissible Revenue",
							sizeof(desc));
		else
			strncpy(desc,"Total - Inadmissible Revenue",
							sizeof(desc));
#else
			strncpy(desc,"Total - Revenu admissible",
							sizeof(desc));
		else
			strncpy(desc,"Total - Revenu inadmissible",
							sizeof(desc));
#endif
	}

	if((ret_cd = PrntHyphens()) < 0) return(ret_cd) ;
	if((ret_cd = PrntTotal( F_total, 0, desc )) < 0) return(ret_cd) ;
	if((ret_cd = PrntPercentage( F_total )) < 0) return(ret_cd) ;

	/* Update Grans Total */
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
	if((ret_cd = SKey3Totals()) < 0) return(ret_cd) ;
	if(key_no <= 2)
		if((ret_cd = SKey2Totals()) < 0) return(ret_cd) ;
	if(key_no <= 1)
		if((ret_cd = SKey1Totals()) < 0) return(ret_cd) ;

	return(NOERROR) ;
}	/* KeyTotals() */
/*-----------------------------------------------------------------*/
/* Print Total for sort key 3 */
SKey3Totals()
{
	double	temp1, temp2 ;

	if( !SK3 || T3_desc[0] == '\0') return(NOERROR) ;

	temp1 = K3_total.cum_actual - prec1 ;
	temp2 = K3_total.cum_curbud - prec2 ;

	if( (temp1 < -0.005 || temp1 > 0.005) ||
					(temp2 < -0.005 || temp2 >0.005) ) {
		if(linecnt > PGSIZE)
			if((ret_cd = PrntHeadings()) < 0)return(ret_cd);

		if((ret_cd = PrntTotal( K3_total, title_rec.keys[KEY(SK3)],
				title_rec.desc )) < 0) return(ret_cd) ;
	}

	InitTotal( &K3_total ) ;
	return(NOERROR) ;
}	/* SKey3Totals() */
/*-----------------------------------------------------------------*/
/* Print Totals for sort key 2 */
SKey2Totals()
{
	double	temp1, temp2 ;
	char	t_desc[10+sizeof(title_rec.desc)] ;

	if( !SK2 || T2_desc[0] == '\0') return(NOERROR) ;

	if( SK3 ) {
		temp1 = K2_total.cum_actual - prec1 ;
		temp2 = K2_total.cum_curbud - prec2 ;

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
			strncat(t_desc, T2_desc, sizeof(title_rec.desc));
			if((ret_cd = PrntTotal(K2_total, 0, t_desc)) < 0)
					return(ret_cd);
		}
		else {
			if(linecnt > (PGSIZE - 2) )
				if((ret_cd = PrntHeadings()) < 0)return(ret_cd);
		}

		if((ret_cd = PrntPercentage( K2_total )) < 0) return(ret_cd) ;
	}	/* SK3 */
	else {
		if(linecnt > PGSIZE)
			if((ret_cd = PrntHeadings()) < 0)return(ret_cd);

		if((ret_cd = PrntTotal(K2_total, title_rec.keys[KEY(SK2)],
				title_rec.desc)) < 0) return(ret_cd);
	}

	InitTotal( &K2_total ) ;
	return(NOERROR) ;
}	/* SKey2Totals() */
/*-----------------------------------------------------------------*/
/* Print Totals for sort key 1 */
SKey1Totals()
{
	char	t_desc[10+sizeof(title_rec.desc)] ;

	if( T1_desc[0] == '\0') return(NOERROR) ;

	if( SK2 ) {
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
		strncat(t_desc, T1_desc, sizeof(title_rec.desc));

		if((ret_cd = PrntTotal( K1_total, 0, t_desc)) < 0)
			return(ret_cd) ;

		if((ret_cd = PrntPercentage( K1_total )) < 0) return(ret_cd) ;
	}
	else {
		if(linecnt > PGSIZE)
			if((ret_cd = PrntHeadings()) < 0)return(ret_cd);

		if((ret_cd = PrntTotal( K1_total, title_rec.keys[KEY(SK1)],
				title_rec.desc)) < 0) return(ret_cd) ;
	}

	InitTotal( &K1_total ) ;

	return(NOERROR) ;
}	/* SKey1Totals() */
/*-----------------------------------------------------------------*/
static	int
PrntPercentage( tot_struct )
Btotal	tot_struct ;
{
	double	temp, temp1 ;

	if((ret_cd = PrntHyphens()) < 0) return(ret_cd) ;

	/* Print Percentage line */

	/* if Cumulative Budget not zero then print % of budget diff */
	if( RepType != 6 &&
	    (tot_struct.cum_budget < -0.005 || tot_struct.cum_budget > 0.005)) {
		/* Budget Difference */
		temp1 = tot_struct.cum_budget - tot_struct.cum_actual ;
		temp = (temp1 / tot_struct.cum_budget) * 100 ;
		if(temp <= -0.0001 || temp >= 0.0001) {
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
	/* Expenses or Revenue or (Expenses & Revenue) Report */
	if(RepType == 8 || RepType == 9 || RepType == 10) {
		mkln(1, Hyphens, 10) ;
		mkln(12, Hyphens, 10) ;
	}
	else {
		mkln(45, Hyphens, 10) ;
		mkln(56, Hyphens, 10) ;
	}
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
PrntTotal( tot_struct, key_val, desc )
Btotal	tot_struct ;
long	key_val ;
char	*desc ;
{
	double	temp ;

	/* Expenses or Revenue or (Expenses & Revenue) Report */
	if(RepType == 8 || RepType == 9 || RepType == 10) {
		tedit((char*)&tot_struct.per_actual, Mask_10,
						line+cur_pos, R_DOUBLE ) ;
		cur_pos += 11 ;
		tedit((char*)&tot_struct.per_budget, Mask_10,
						line+cur_pos, R_DOUBLE ) ;
		cur_pos += 11 ;

		if( key_val != 0) {
			mkln(28, " ", 1) ;
			tedit((char*)&key_val, Mask_5, line+cur_pos, R_LONG ) ;
			cur_pos += 5 ;
		}

		mkln(35, desc,30) ;
		mkln(66, " ", 1) ;

	}
	else {
		if( key_val != 0) {
			mkln(4, " ", 1) ;
			tedit((char*)&key_val, Mask_5, line+cur_pos, R_LONG ) ;
			cur_pos += 5 ;
		}

		mkln(13, desc,30) ;
		mkln(44," ",1);
		if(RepType == 6) {	/* Modified Foemat */
			tedit((char*)&tot_struct.per_prev_yr, Mask_10,
						line+cur_pos, R_DOUBLE ) ;
			cur_pos += 11 ;
			tedit((char*)&tot_struct.per_actual, Mask_10,
						line+cur_pos, R_DOUBLE ) ;
			cur_pos += 11 ;

			tedit((char*)&tot_struct.cum_comdat, Mask_10,
						line+cur_pos, R_DOUBLE ) ;
			cur_pos += 11 ;
		}
		else {
			tedit((char*)&tot_struct.per_actual, Mask_10,
						line+cur_pos, R_DOUBLE ) ;
			cur_pos += 11 ;
			tedit((char*)&tot_struct.per_budget, Mask_10,
						line+cur_pos, R_DOUBLE ) ;
			cur_pos += 11 ;
		}
	}

	tedit((char*)&tot_struct.cum_prev_yr, Mask_10, line+cur_pos, R_DOUBLE );
	cur_pos += 11 ;
	tedit((char*)&tot_struct.cum_actual, Mask_10, line+cur_pos, R_DOUBLE ) ;
	cur_pos += 11 ;
	tedit((char*)&tot_struct.cum_budget, Mask_10, line+cur_pos, R_DOUBLE ) ;
	cur_pos += 11 ;

	/* Cumultaive Diff */
	if(RepType != 6) {
		temp = tot_struct.cum_budget - tot_struct.cum_actual ;
		tedit((char*)&temp, Mask_10, line+cur_pos, R_DOUBLE ) ;
		cur_pos += 11 ;
	}

	tedit((char*)&tot_struct.cum_curbud, Mask_10, line+cur_pos, R_DOUBLE ) ;
	cur_pos += 11 ;

	temp = tot_struct.cum_curbud - tot_struct.cum_actual ;
	tedit((char*)&temp, Mask_10, line+cur_pos, R_DOUBLE ) ;
	cur_pos += 11 ;

	if(prnt_line() < 0) return(ERROR);

	return(NOERROR) ;
}	/* PrntTotal */
/*-----------------------------------------------------------------*/
static	int
GetTitles(key_no)
int	key_no ;
{
	switch(key_no) {
	case 0 :
		ret_cd = PrntHeadings() ;
		if(ret_cd != NOERROR) return(ret_cd) ;

		/* Fall thru to next case */
	case 1 :	/* Sort Key 1 */
		/* Get Sort Key 1 Title */
		ret_cd = GetKTitle(SK1, &prev_sk1) ;
		if(ret_cd != NOERROR && ret_cd != UNDEF) return(ret_cd) ;

		if( SK2 ) {		/* If next sort key exists */
			if(ret_cd == UNDEF)
				STRCPY(T1_desc,"????????????????????");
			else
				strncpy(T1_desc, title_rec.desc,
						sizeof(title_rec.desc) );
			if((ret_cd = PrntSubTitle(T1_desc)) < 0)
				return(ret_cd) ;
		}
		else {
			if(ret_cd == UNDEF)
				T1_desc[0] = '\0' ;
			else
				strncpy(T1_desc, title_rec.desc,
						sizeof(title_rec.desc) );
		}

		prev_admis = gl_rec.admis ;
		prev_sect  = gl_rec.sect  ;
		/* Fall thru to next case */
	case 2 :	/* Sort Key 2 */
		/* Get Sort Key 2 Title */
		ret_cd = GetKTitle(SK2, &prev_sk2) ;
		if(ret_cd != NOERROR && ret_cd != UNDEF) return(ret_cd) ;

		if( SK3 ) {		/* If next sort key exists */
			if(ret_cd == UNDEF)
				STRCPY(T2_desc,"????????????????????");
			else
				strncpy(T2_desc, title_rec.desc,
						sizeof(title_rec.desc) );
			if((ret_cd = PrntSubTitle(T2_desc)) < 0)
				return(ret_cd) ;
		}
		else {
			if(ret_cd == UNDEF)
				T2_desc[0] = '\0' ;
			else
				strncpy(T2_desc, title_rec.desc,
						sizeof(title_rec.desc) );
		}

		/* Fall thru to next case */
	default :	/* Sort key 3 */
		/* Get Sort Key 3 Title */
		ret_cd = GetKTitle(SK3, &prev_sk3) ;
		if(ret_cd != NOERROR && ret_cd != UNDEF) return(ret_cd) ;

		if(ret_cd == UNDEF)
			T3_desc[0] = '\0';
		else {
			strncpy(T3_desc, title_rec.desc,
						sizeof(title_rec.desc) );
			if((ret_cd = PrntSubTitle(T3_desc)) < 0)
				return(ret_cd) ;
		}
	}

	return(NOERROR) ;
}	/* GetTitles() */
/*-----------------------------------------------------------------*/
/* Get Title for sort key */
static	int	
GetKTitle(SK, prev_sk)
short	SK ;	/* Sort Key# */
long	*prev_sk ;
{
	int	i ;

	*prev_sk = 0 ;

	/* SK = zero or Sort SK user has not given or the value in record is
	   zero */

	if(SK == 0 || KEY_EXISTS( SK ) == 0 ||
				(*prev_sk = gl_rec.keys[KEY( SK )]) == 0)
		return(UNDEF) ;

	/* Form the Sort SK Title key */
	title_rec.keys[KEY( SK )] = gl_rec.keys[KEY( SK )] ;

	if(ALL_KEYS) {
		/* MOve key before values also */
		for( i = 1 ; i < SK ; i++)
			title_rec.keys[KEY(i)] = gl_rec.keys[KEY(i)] ;
	}
	else {
		/* MOve 0's to first keys */
		for( i = 1 ; i < SK ; i++)
			title_rec.keys[KEY(i)] = 0;
	}

	/* MOve 0's to Tail  keys */
	for( i = SK+1 ; i <= USER_KEYS && KEY_EXISTS(i) ; i++)
		title_rec.keys[KEY(i)] = 0;

	ret_cd = get_isrec((char*)&title_rec, TMPINDX_1, 0, BROWSE, e_mesg) ;
	if(ret_cd == ERROR) return(DBH_ERR) ;

	if(ret_cd == UNDEF) return(UNDEF) ;

	if((ret_cd = AddTitle()) < 0) return(ret_cd) ;

	return(NOERROR) ;
}	/* GetKTitle() */
/*-----------------------------------------------------------------*/
static	int	/* Print Title of k5 or Title of k6 as a sub title */
PrntSubTitle(desc)
char	*desc ;
{
	int	i ;

	if(linecnt > (PGSIZE - 4) ) {	/* Atleast 4 lines required
					   including next report line */
		ret_cd = PrntHeadings() ;
		if(ret_cd != NOERROR) return(ret_cd) ;
	}

	/* NOTE: This condition is wrong, when title not found. Contains
		 previous Titles value */
	if(title_rec.budcur < -0.005 && title_rec.budcur > 0.005)
		return(NOERROR) ;

	/* Expenses or Revenue or (Expenses & Revenue) Report */
	if(RepType == 8 || RepType == 9 || RepType == 10) {
		i = mkln(35,desc,30) ;

		if(prnt_line() < 0) return(ERROR) ;

		/* Print Under line */
		mkln(35,Hyphens,i) ;
	}
	else {
		i = mkln(13,desc,30) ;

		if(prnt_line() < 0) return(ERROR) ;

		/* Print Under line */
		mkln(13,Hyphens,i) ;
	}
	if(prnt_line() < 0) return(ERROR) ;
	if(prnt_line() < 0) return(ERROR) ;

	return(NOERROR) ;
}	/* PrntSubTitle() */
/*----------------------------------------------------------------------*/
static	int
AddTitle()
{
	double	cum_budget ;

	if( title_rec.budcur < -0.005 && title_rec.budcur > 0.005 ){

		if(linecnt >= PGSIZE)
			if((ret_cd = PrntHeadings()) < 0)return(ret_cd);

		title_rec.prerel[rp_sth.s_period-1] = 0.0 ;
		title_rec.currel[rp_sth.s_period-1] = 0.0 ;

		cum_budget = Cumulative(rp_sth.s_period, title_rec.curbud) ;

		/* Expenses or Revenue or (Expenses & Revenue) Report */
		if(RepType == 8 || RepType == 9 || RepType == 10) {
			mkln(11, " ", 1) ;
			tedit((char*)&title_rec.curbud[rp_sth.s_period-1],
				Mask_10, line+cur_pos, R_DOUBLE ) ;
			cur_pos += 11 ;

			mkln(35, title_rec.desc, 30) ;
			mkln(88, " ", 1) ;
		}
		else {
			mkln(13, title_rec.desc, 30) ;
			if(RepType == 7) { /* French Format Report */
			    mkln(55, " ", 1) ;
			    tedit((char*)&title_rec.curbud[rp_sth.s_period-1],
				Mask_10, line+cur_pos, R_DOUBLE ) ;
			    cur_pos += 11 ;
			    mkln(88, " ", 1) ;
			}
			else
			    mkln(99, " ", 1) ;

		}

		tedit((char*)&cum_budget, Mask_10, line+cur_pos, R_DOUBLE ) ;
		cur_pos += 11 ;
		if(RepType != 6) {
			/* Cumultaive Budget Diff (No actual for titles) */
			tedit((char*)&cum_budget, Mask_10,
						line+cur_pos, R_DOUBLE ) ;
			cur_pos += 11 ;
		}

		tedit((char*)&title_rec.budcur, Mask_10,
					line+cur_pos, R_DOUBLE ) ;
		cur_pos += 11 ;

		tedit((char*)&title_rec.budcur, Mask_10,
				line+cur_pos, R_DOUBLE);
		cur_pos += 11 ;

		if(prnt_line() < 0) return(ERROR);

		/* Sort K3 not zero */
		if( SK3 && KEY_EXISTS( SK3 ) && title_rec.keys[KEY( SK3 )] )
		    AddTotals(&K3_total, title_rec, 0.0, 0.0, cum_budget) ;

		/* Sort K2 not zero */
		if( SK2 && KEY_EXISTS( SK2 ) && title_rec.keys[KEY( SK2 )] )
		    AddTotals(&K2_total, title_rec, 0.0, 0.0, cum_budget) ;

		/* Sort K1 not zero */
		if( SK1 && KEY_EXISTS( SK1 ) && title_rec.keys[KEY( SK1 )] )
		    AddTotals(&K1_total, title_rec, 0.0, 0.0, cum_budget) ;

		AddTotals(&F_total, title_rec, 0.0, 0.0, cum_budget) ;

	}

	return(NOERROR) ;
}	/* AddTitle() */
/*-------------------- E n d   O f   P r o g r a m ---------------------*/
