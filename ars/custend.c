/******************************************************************************
		Sourcename    : custend.c
		System        : Budgetary Financial System.
		Subsystem     : Accounts Receivable System 
		Module        : Accounts Receivable 
		Created on    : 89-12-28
		Created  By   : J Prescott.
		Cobol sources : 

HISTORY:
 Date           Programmer     Description of modification
____/__/__      __________     ___________________________

*******************************************************************************
	customer month and year end.  rolls current balance over to monthly
	balance in month end.  it also resets year to date totals to zero in
	year end.
******************************************************************************/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

static  Cu_rec cu_rec;

extern char e_mesg[80] ;

CustEnd(mode)		/* mode == 1  (month end) */
int mode;		/* mode == 2  (year end)  */
{
	int	retval;

	if(mode != MONTH && mode != YEAR) 
		return(0);

	cu_rec.cu_code[0] = '\0';
	flg_reset( CUSTOMER );

	for( ; ; ) {
		retval = get_n_cust(&cu_rec,UPDATE,0,FORWARD,e_mesg);
		if( retval < 0) {
			if(retval == EFL) break;
			roll_back(e_mesg);
			retval = DBH_ERR;
			break;
		}
		cu_rec.cu_mon_op = cu_rec.cu_cur_bal; 
		if(mode == YEAR) {
			cu_rec.cu_ytd_sales = 0;
			cu_rec.cu_ytd_rcpts = 0;
		}
		retval = put_cust(&cu_rec,UPDATE,e_mesg);
		if( retval < 0) {
			retval = DBH_ERR;
			roll_back(e_mesg);
			break;
		}
		if(commit(e_mesg) < 0) {
			retval = DBH_ERR;
			roll_back(e_mesg);
			break;
		}	
		inc_str(cu_rec.cu_code,sizeof(cu_rec.cu_code)-1,FORWARD);
	}
	close_file( CUSTOMER );
	if(retval == DBH_ERR) return(DBH_ERR);
	return(0);
}
