/*-----------------------------------------------------------------------
Source Name: bdgt_com.c
System     : Budgetary Financial system.
Module     : General Ledger system.
Created  On: 21st Jul 89.
Created  By: T AMARENDRA.


DESCRIPTION:
	Common Functions Used by all Budget Report Programs

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
J.Prescott     91/02/19       Added Cost Center Key Range.

------------------------------------------------------------------------*/

#include <stdio.h>
#include <bdgt_rep.h>

/*-----------------------------------------------------------------*/
/* Read The Control for the code in GL Rec */
GetFund()
{
	ctl_rec.fund = gl_rec.funds ;
	ret_cd = get_ctl(&ctl_rec, BROWSE, 0, e_mesg) ;

	if(ret_cd == ERROR || ret_cd == UNDEF) return(DBH_ERR) ;
	return( ret_cd ) ;
}	/* GetFund() */
/*-----------------------------------------------------------------*/
/* Add upto given period & return cumulative value */
double
Cumulative(period, array)
short	period ;
double	*array ;
{
	double	total ;

	total = 0.0 ;
	while(period) {
		total += *array ;
		period-- ;
		array++ ;
	}

	return( total ) ;
}	/* Cumulative() */
/*-----------------------------------------------------------------*/
/* Add Totals */
AddTotals(tot_struct, g_rec, cum_prev_yr, cum_actual, cum_budget)
Btotal	*tot_struct ;
Gl_rec	g_rec ;
double	cum_prev_yr ,
	cum_actual ,
	cum_budget ;
{
	int	period ;

	period = rp_sth.s_period - 1;

	tot_struct->per_prev_yr += g_rec.prerel[period] ;
	tot_struct->per_actual  += g_rec.currel[period] ;
	tot_struct->per_budget  += g_rec.curbud[period] ;

	tot_struct->cum_prev_yr += cum_prev_yr ;
	tot_struct->cum_actual  += cum_actual  ;
	tot_struct->cum_budget  += cum_budget  ;

	tot_struct->cum_curbud  += g_rec.budcur ;
	tot_struct->cum_comdat  += g_rec.comdat ;

}	/* AddTotals() */
/*-----------------------------------------------------------------*/
/* Initialize Totals structure */
InitTotal(tot_struct)
Btotal	*tot_struct ;
{
	tot_struct->per_prev_yr = 0.0 ;
	tot_struct->per_actual  = 0.0 ;
	tot_struct->per_budget  = 0.0 ;

	tot_struct->cum_prev_yr = 0.0 ;
	tot_struct->cum_actual  = 0.0 ;
	tot_struct->cum_budget  = 0.0 ;

	tot_struct->cum_curbud  = 0.0 ;
	tot_struct->cum_comdat  = 0.0 ;

}	/* InitTotal() */
/*----------------------------------------------------------*/
FormFeed()	/* Skip Page */
{
	int	i ;

	/* For display pause the report */
	if(term < 99){	/* Output On terminal */
		if(pgcnt){
			if(next_page() < 0)return(QUIT);
		}
		else {
			fomcs();
			fomrt() ;
		}
	}
	/* Skip first time page skipping for not display outputs */
	if(pgcnt || term < 99 ){
		if(rite_top() < 0)return(ERROR);
	}
	else
		linecnt = 0;

	/* Print Program-Id, School District name, Date and Page# */
	mkln(1,PROG_NAME, sizeof(PROG_NAME)) ;
	i = strlen(pa_rec.pa_co_name);
	mkln(((LNSZ - i) / 2),pa_rec.pa_co_name, i);
#ifdef ENGLISH
	mkln(106,"Date: ",6);
#else
	mkln(106,"Date: ",6);
#endif
	tedit((char*)&rp_sth.s_rundt, Date_Mask, line+cur_pos,R_LONG);
	cur_pos += 10;
	pgcnt++;
#ifdef ENGLISH
	mkln(124,"Page:",5);
#else
	mkln(124,"Page:",5);
#endif
	tedit((char*)&pgcnt,Mask_4,line+cur_pos,R_SHORT);
	cur_pos += 4;

	if(prnt_line() < 0)return(ERROR);
	
	return(NOERROR) ;
}
/*----------------------------END OF PROGRAM---------------------------*/

