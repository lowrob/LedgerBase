/*------------------------------------------------------------- */
/*	Source Name: dbhpp.c	(link DBH)			*/
/*	Created On : 9th Aug 1991.				*/
/*	System     : Budgetary Financial System			*/
/*								*/
/*	This file defines interface between user programs and	*/
/*	DBH. Here for each data file seperate function is pro-	*/
/*	vided for user convenience. And also good to have this	*/
/*	level, in future if any special attention to be given	*/
/*	to any file, then this is useful.			*/
/*--------------------------------------------------------------*/

#include <bfs_defs.h>
#include <bfs_pp.h>

extern	int	*alt_array ;	/* Controls the alternate keys writing
				   of ISAM files */

/*-----------------------------------------------------------*/ 

/* ******************  PERSONNEL/PAYROLL  ****************** */ 

/*-----------------------------------------------------------*/
get_dept( rec, mode, key_no, c_mesg )
Dept	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,DEPARTMENT,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_dept( rec, mode, key_no, direction, c_mesg )
Dept	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,DEPARTMENT,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_dept( rec, mode, c_mesg )
Dept	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,DEPARTMENT,c_mesg));
}
/*-----------------------------------------------------------*/
get_area( rec, mode, key_no, c_mesg )
Area	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,AREA,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_area( rec, mode, key_no, direction, c_mesg )
Area	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,AREA,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_area( rec, mode, c_mesg )
Area	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,AREA,c_mesg));
}
/*-----------------------------------------------------------*/
get_position( rec, mode, key_no, c_mesg )
Position	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,POSITION,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_position( rec, mode, key_no, direction, c_mesg )
Position	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,POSITION,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_position( rec, mode, c_mesg )
Position	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,POSITION,c_mesg));
}
/*-----------------------------------------------------------*/
get_class( rec, mode, key_no, c_mesg )
Class	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,CLASSIFICATION,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_class( rec, mode, key_no, direction, c_mesg )
Class	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,CLASSIFICATION,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_class( rec, mode, c_mesg )
Class	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,CLASSIFICATION,c_mesg));
}
/*-----------------------------------------------------------*/
get_class_it( rec, mode, key_no, c_mesg )
Class_item	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,CLASS_ITEM,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_class_it( rec, mode, key_no, direction, c_mesg )
Class_item	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,CLASS_ITEM,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_class_it( rec, mode, c_mesg )
Class_item	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,CLASS_ITEM,c_mesg));
}
/*-----------------------------------------------------------*/
get_barg( rec, mode, key_no, c_mesg )
Barg_unit	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,BARG,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_barg( rec, mode, key_no, direction, c_mesg )
Barg_unit	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,BARG,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_barg( rec, mode, c_mesg )
Barg_unit	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,BARG,c_mesg));
}
/*-----------------------------------------------------------*/
get_bank( rec, mode, key_no, c_mesg )
Bank	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,BANK,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_bank( rec, mode, key_no, direction, c_mesg )
Bank	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,BANK,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_bank( rec, mode, c_mesg )
Bank	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,BANK,c_mesg));
}
/*-----------------------------------------------------------*/
get_pay_per( rec, mode, key_no, c_mesg )
Pay_per	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,PAY_PERIOD,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_pay_per( rec, mode, key_no, direction, c_mesg )
Pay_per	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,PAY_PERIOD,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_pay_per( rec, mode, c_mesg )
Pay_per	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,PAY_PERIOD,c_mesg));
}
/*-----------------------------------------------------------*/
get_pp_it( rec, mode, key_no, c_mesg )
Pay_per_it	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,PAY_PER_ITEM,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_pp_it( rec, mode, key_no, direction, c_mesg )
Pay_per_it	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,PAY_PER_ITEM,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_pp_it( rec, mode, c_mesg )
Pay_per_it	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,PAY_PER_ITEM,c_mesg));
}
/*-----------------------------------------------------------*/
get_uic( rec, mode, key_no, c_mesg )
Uic	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,UIC,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_uic( rec, mode, key_no, direction, c_mesg )
Uic	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,UIC,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_uic( rec, mode, c_mesg )
Uic	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,UIC,c_mesg));
}
/*-----------------------------------------------------------*/
get_tax( rec, mode, key_no, c_mesg )
Tax	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,TAX,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_tax( rec, mode, key_no, direction, c_mesg )
Tax	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,TAX,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_tax( rec, mode, c_mesg )
Tax	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,TAX,c_mesg));
}
/*-----------------------------------------------------------*/
get_cert( rec, mode, key_no, c_mesg )
Cert	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,CERT,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_cert( rec, mode, key_no, direction, c_mesg )
Cert	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,CERT,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_cert( rec, mode, c_mesg )
Cert	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,CERT,c_mesg));
}
/*-----------------------------------------------------------*/
get_earn( rec, mode, key_no, c_mesg )
Earn	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,EARN,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_earn( rec, mode, key_no, direction, c_mesg )
Earn	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,EARN,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_earn( rec, mode, c_mesg )
Earn	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,EARN,c_mesg));
}
/*-----------------------------------------------------------*/
get_trans( rec, mode, key_no, c_mesg )
Trans	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,TRANS,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_trans( rec, mode, key_no, direction, c_mesg )
Trans	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,TRANS,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_trans( rec, mode, c_mesg )
Trans	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,TRANS,c_mesg));
}
/*-----------------------------------------------------------*/
get_trans_it( rec, mode, key_no, c_mesg )
Trans_item	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,TRANS_ITEM,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_trans_it( rec, mode, key_no, direction, c_mesg )
Trans_item	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,TRANS_ITEM,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_trans_it( rec, mode, c_mesg )
Trans_item	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,TRANS_ITEM,c_mesg));
}
/*-----------------------------------------------------------*/
get_exp( rec, mode, key_no, c_mesg )
Exp	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,EXPENSE,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_exp( rec, mode, key_no, direction, c_mesg )
Exp	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,EXPENSE,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_exp( rec, mode, c_mesg )
Exp	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,EXPENSE,c_mesg));
}
/*-----------------------------------------------------------*/
get_exp_it( rec, mode, key_no, c_mesg )
Exp_item	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,EXP_ITEM,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_exp_it( rec, mode, key_no, direction, c_mesg )
Exp_item	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,EXP_ITEM,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_exp_it( rec, mode, c_mesg )
Exp_item	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,EXP_ITEM,c_mesg));
}
/*-----------------------------------------------------------*/
get_pterm( rec, mode, key_no, c_mesg )
Term	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,TERM,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_pterm( rec, mode, key_no, direction, c_mesg )
Term	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,TERM,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_pterm( rec, mode, c_mesg )
Term	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,TERM,c_mesg));
}
/*-----------------------------------------------------------*/
get_stat( rec, mode, key_no, c_mesg )
Stat	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,STAT_HOL,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_stat( rec, mode, key_no, direction, c_mesg )
Stat	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,STAT_HOL,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_stat( rec, mode, c_mesg )
Stat	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,STAT_HOL,c_mesg));
}
/*-----------------------------------------------------------*/
get_att( rec, mode, key_no, c_mesg )
Att	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,ATT,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_att( rec, mode, key_no, direction, c_mesg )
Att	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,ATT,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_att( rec, mode, c_mesg )
Att	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,ATT,c_mesg));
}
/*-----------------------------------------------------------*/
get_inact( rec, mode, key_no, c_mesg )
Inact	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,INACT_CODE,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_inact( rec, mode, key_no, direction, c_mesg )
Inact	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,INACT_CODE,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_inact( rec, mode, c_mesg )
Inact	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,INACT_CODE,c_mesg));
}
/*-----------------------------------------------------------*/
get_area_spec( rec, mode, key_no, c_mesg )
Area_spec	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,AREA_SPEC,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_area_spec( rec, mode, key_no, direction, c_mesg )
Area_spec	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,AREA_SPEC,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_area_spec( rec, mode, c_mesg )
Area_spec	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,AREA_SPEC,c_mesg));
}
/*-----------------------------------------------------------*/
get_loan( rec, mode, key_no, c_mesg )
Csb_loan	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,LOAN,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_loan( rec, mode, key_no, direction, c_mesg )
Csb_loan	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,LOAN,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_loan( rec, mode, c_mesg )
Csb_loan	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,LOAN,c_mesg));
}
/*-----------------------------------------------------------*/
get_pay_param( rec, mode, rec_no, c_mesg )
Pay_param	*rec ;
int 	mode ;
int	rec_no ;
char	*c_mesg ;
{
	return(get_rec((char*)rec,PAY_PARAM,rec_no,mode,c_mesg));
}

/*
*	NOTE:	get_n_param() is not provided, because there is
*		only one Parameter record in the system.
*/

/*-----------------------------------------------------------*/
put_pay_param( rec, mode, rec_no, c_mesg )
Pay_param	*rec ;
int 	mode ;
int	rec_no ;
char	*c_mesg ;
{
	if(rec_no != 1) {
#ifdef ENGLISH
		strcpy(c_mesg,
			"ERROR: Invalid Write Operation on PAY PARAMETER File..");
#else
		strcpy(c_mesg,
			"ERREUR: Operation d'inscription invalide sur le dossier PARAMETRE..");
#endif
		return(ERROR) ;
	}
	return(put_rec((char*)rec,mode,PAY_PARAM,rec_no,c_mesg));
}
/*-----------------------------------------------------------*/
get_salary( rec, mode, key_no, c_mesg )
Salary	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,SALARY,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_salary( rec, mode, key_no, direction, c_mesg )
Salary	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,SALARY,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_salary( rec, mode, c_mesg )
Salary	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,SALARY,c_mesg));
}
/*-----------------------------------------------------------*/
get_benefit( rec, mode, key_no, c_mesg )
Benefit	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,BENEFIT,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_benefit( rec, mode, key_no, direction, c_mesg )
Benefit	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,BENEFIT,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_benefit( rec, mode, c_mesg )
Benefit	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,BENEFIT,c_mesg));
}
/*-----------------------------------------------------------*/
get_ben_cat( rec, mode, key_no, c_mesg )
Ben_cat	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,BEN_CAT,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_ben_cat( rec, mode, key_no, direction, c_mesg )
Ben_cat	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,BEN_CAT,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_ben_cat( rec, mode, c_mesg )
Ben_cat	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,BEN_CAT,c_mesg));
}
/*-----------------------------------------------------------*/
get_deduction( rec, mode, key_no, c_mesg )
Deduction	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,DEDUCTION,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_deduction( rec, mode, key_no, direction, c_mesg )
Deduction	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,DEDUCTION,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_deduction( rec, mode, c_mesg )
Deduction	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,DEDUCTION,c_mesg));
}
/*-----------------------------------------------------------*/
get_ded_grp( rec, mode, key_no, c_mesg )
Ded_group	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,DED_GRP,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_ded_grp( rec, mode, key_no, direction, c_mesg )
Ded_group	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,DED_GRP,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_ded_grp( rec, mode, c_mesg )
Ded_group	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,DED_GRP,c_mesg));
}
/*-----------------------------------------------------------*/
get_ded_cat( rec, mode, key_no, c_mesg )
Ded_cat	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,DED_CAT,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_ded_cat( rec, mode, key_no, direction, c_mesg )
Ded_cat	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,DED_CAT,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_ded_cat( rec, mode, c_mesg )
Ded_cat	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,DED_CAT,c_mesg));
}
/*-----------------------------------------------------------*/
get_reg_pen( rec, mode, key_no, c_mesg )
Reg_pen	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,REG_PEN,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_reg_pen( rec, mode, key_no, direction, c_mesg )
Reg_pen	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,REG_PEN,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_reg_pen( rec, mode, c_mesg )
Reg_pen	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,REG_PEN,c_mesg));
}
/*-----------------------------------------------------------*/
get_employee( rec, mode, key_no, c_mesg )
Emp	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,EMPLOYEE,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_employee( rec, mode, key_no, direction, c_mesg )
Emp	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,EMPLOYEE,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_employee( rec, mode, c_mesg )
Emp	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,EMPLOYEE,c_mesg));
}
/*-----------------------------------------------------------*/
get_emp_emp( rec, mode, key_no, c_mesg )
Emp_emp	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,EMP_EMP,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_emp_emp( rec, mode, key_no, direction, c_mesg )
Emp_emp	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,EMP_EMP,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_emp_emp( rec, mode, c_mesg )
Emp_emp	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,EMP_EMP,c_mesg));
}
/*-----------------------------------------------------------*/
get_emp_sched1( rec, mode, key_no, c_mesg )
Emp_sched1	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,EMP_SCHED1,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_emp_sched1( rec, mode, key_no, direction, c_mesg )
Emp_sched1	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,EMP_SCHED1,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_emp_sched1( rec, mode, c_mesg )
Emp_sched1	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,EMP_SCHED1,c_mesg));
}
/*-----------------------------------------------------------*/
get_emp_sched2( rec, mode, key_no, c_mesg )
Emp_sched2	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,EMP_SCHED2,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_emp_sched2( rec, mode, key_no, direction, c_mesg )
Emp_sched2	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,EMP_SCHED2,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_emp_sched2( rec, mode, c_mesg )
Emp_sched2	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,EMP_SCHED2,c_mesg));
}
/*-----------------------------------------------------------*/
get_emp_extra( rec, mode, key_no, c_mesg )
Emp_extra	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,EMP_EXTRA,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_emp_extra( rec, mode, key_no, direction, c_mesg )
Emp_extra	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,EMP_EXTRA,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_emp_extra( rec, mode, c_mesg )
Emp_extra	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,EMP_EXTRA,c_mesg));
}
/*-----------------------------------------------------------*/
get_emp_earn( rec, mode, key_no, c_mesg )
Emp_earn	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,EMP_EARN,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_emp_earn( rec, mode, key_no, direction, c_mesg )
Emp_earn	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,EMP_EARN,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_emp_earn( rec, mode, c_mesg )
Emp_earn	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,EMP_EARN,c_mesg));
}
/*-----------------------------------------------------------*/
get_emp_ins( rec, mode, key_no, c_mesg )
Emp_ins	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,EMP_INS,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_emp_ins( rec, mode, key_no, direction, c_mesg )
Emp_ins	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,EMP_INS,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_emp_ins( rec, mode, c_mesg )
Emp_ins	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,EMP_INS,c_mesg));
}
/*-----------------------------------------------------------*/
get_emp_ben( rec, mode, key_no, c_mesg )
Emp_ben	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,EMP_BEN,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_emp_ben( rec, mode, key_no, direction, c_mesg )
Emp_ben	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,EMP_BEN,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_emp_ben( rec, mode, c_mesg )
Emp_ben	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,EMP_BEN,c_mesg));
}
/*-----------------------------------------------------------*/
get_emp_bhis( rec, mode, key_no, c_mesg )
Emp_bh	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,EMP_BEN_HIS,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_emp_bhis( rec, mode, key_no, direction, c_mesg )
Emp_bh	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,EMP_BEN_HIS,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_emp_bhis( rec, mode, c_mesg )
Emp_bh	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,EMP_BEN_HIS,c_mesg));
}
/*-----------------------------------------------------------*/
get_emp_ded( rec, mode, key_no, c_mesg )
Emp_ded	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,EMP_DED,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_emp_ded( rec, mode, key_no, direction, c_mesg )
Emp_ded	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,EMP_DED,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_emp_ded( rec, mode, c_mesg )
Emp_ded	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,EMP_DED,c_mesg));
}
/*-----------------------------------------------------------*/
get_emp_dhis( rec, mode, key_no, c_mesg )
Emp_dh	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,EMP_DED_HIS,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_emp_dhis( rec, mode, key_no, direction, c_mesg )
Emp_dh	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,EMP_DED_HIS,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_emp_dhis( rec, mode, c_mesg )
Emp_dh	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,EMP_DED_HIS,c_mesg));
}
/*-----------------------------------------------------------*/
get_emp_loan( rec, mode, key_no, c_mesg )
Emp_loan	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,EMP_LOAN,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_emp_loan( rec, mode, key_no, direction, c_mesg )
Emp_loan	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,EMP_LOAN,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_emp_loan( rec, mode, c_mesg )
Emp_loan	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,EMP_LOAN,c_mesg));
}
/*-----------------------------------------------------------*/
get_emp_lhis( rec, mode, key_no, c_mesg )
Emp_ln_his	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,EMP_LOAN_HIS,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_emp_lhis( rec, mode, key_no, direction, c_mesg )
Emp_ln_his	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,EMP_LOAN_HIS,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_emp_lhis( rec, mode, c_mesg )
Emp_ln_his	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,EMP_LOAN_HIS,c_mesg));
}
/*-----------------------------------------------------------*/
get_emp_garn( rec, mode, key_no, c_mesg )
Emp_garn	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,EMP_GARN,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_emp_garn( rec, mode, key_no, direction, c_mesg )
Emp_garn	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,EMP_GARN,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_emp_garn( rec, mode, c_mesg )
Emp_garn	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,EMP_GARN,c_mesg));
}
/*-----------------------------------------------------------*/
get_emp_ghis( rec, mode, key_no, c_mesg )
Emp_gr_his	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,EMP_GARN_HIS,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_emp_ghis( rec, mode, key_no, direction, c_mesg )
Emp_gr_his	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,EMP_GARN_HIS,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_emp_ghis( rec, mode, c_mesg )
Emp_gr_his	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,EMP_GARN_HIS,c_mesg));
}
/*-----------------------------------------------------------*/
get_teach_ass( rec, mode, key_no, c_mesg )
Teach_ass	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,TEACH_ASS,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_teach_ass( rec, mode, key_no, direction, c_mesg )
Teach_ass	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,TEACH_ASS,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_teach_ass( rec, mode, c_mesg )
Teach_ass	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,TEACH_ASS,c_mesg));
}
/*-----------------------------------------------------------*/
get_teach_qual( rec, mode, key_no, c_mesg )
Teach_qual	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,TEACH_QUAL,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_teach_qual( rec, mode, key_no, direction, c_mesg )
Teach_qual	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,TEACH_QUAL,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_teach_qual( rec, mode, c_mesg )
Teach_qual	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,TEACH_QUAL,c_mesg));
}
/*-----------------------------------------------------------*/
get_emp_at( rec, mode, key_no, c_mesg )
Emp_at_his	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,EMP_ATT,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_emp_at( rec, mode, key_no, direction, c_mesg )
Emp_at_his	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,EMP_ATT,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_emp_at( rec, mode, c_mesg )
Emp_at_his	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,EMP_ATT,c_mesg));
}
/*-----------------------------------------------------------*/
get_ptime( rec, mode, key_no, c_mesg )
Time	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,TIME,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_ptime( rec, mode, key_no, direction, c_mesg )
Time	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,TIME,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_ptime( rec, mode, c_mesg )
Time	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,TIME,c_mesg));
}
/*-----------------------------------------------------------*/
get_time_his( rec, mode, key_no, c_mesg )
Time_his	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,TIME_HIS,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_time_his( rec, mode, key_no, direction, c_mesg )
Time_his	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,TIME_HIS,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_time_his( rec, mode, c_mesg )
Time_his	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,TIME_HIS,c_mesg));
}
/*-----------------------------------------------------------*/
get_pp_earn( rec, mode, key_no, c_mesg )
Pay_earn	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,PP_EARN,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_pp_earn( rec, mode, key_no, direction, c_mesg )
Pay_earn	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,PP_EARN,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_pp_earn( rec, mode, c_mesg )
Pay_earn	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,PP_EARN,c_mesg));
}
/*-----------------------------------------------------------*/
get_pp_ben( rec, mode, key_no, c_mesg )
Pp_ben	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,PP_BEN,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_pp_ben( rec, mode, key_no, direction, c_mesg )
Pp_ben	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,PP_BEN,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_pp_ben( rec, mode, c_mesg )
Pp_ben	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,PP_BEN,c_mesg));
}
/*-----------------------------------------------------------*/
get_pp_ded( rec, mode, key_no, c_mesg )
Pay_ded	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,PP_DED,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_pp_ded( rec, mode, key_no, direction, c_mesg )
Pay_ded	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,PP_DED,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_pp_ded( rec, mode, c_mesg )
Pay_ded	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,PP_DED,c_mesg));
}
/*-----------------------------------------------------------*/
get_pp_loan( rec, mode, key_no, c_mesg )
Pay_loan	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,PP_LOAN,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_pp_loan( rec, mode, key_no, direction, c_mesg )
Pay_loan	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,PP_LOAN,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_pp_loan( rec, mode, c_mesg )
Pay_loan	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,PP_LOAN,c_mesg));
}
/*-----------------------------------------------------------*/
get_pp_garn( rec, mode, key_no, c_mesg )
Pay_garn	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,PP_GARN,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_pp_garn( rec, mode, key_no, direction, c_mesg )
Pay_garn	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,PP_GARN,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_pp_garn( rec, mode, c_mesg )
Pay_garn	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,PP_GARN,c_mesg));
}
/*-----------------------------------------------------------*/
get_chq_mess( rec, mode, key_no, c_mesg )
Chq_mess	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,CHQ_MESS,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_chq_mess( rec, mode, key_no, direction, c_mesg )
Chq_mess	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,CHQ_MESS,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_chq_mess( rec, mode, c_mesg )
Chq_mess	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,CHQ_MESS,c_mesg));
}
/*-----------------------------------------------------------*/
get_chq_ms_ass( rec, mode, key_no, c_mesg )
Chq_mess_ass	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,CHQ_MESS_ASS,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_chq_ms_ass( rec, mode, key_no, direction, c_mesg )
Chq_mess_ass	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,CHQ_MESS_ASS,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_chq_ms_ass( rec, mode, c_mesg )
Chq_mess_ass	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,CHQ_MESS_ASS,c_mesg));
}
/*-----------------------------------------------------------*/
get_rg_pen_adj( rec, mode, key_no, c_mesg )
Reg_pen_adj	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,REG_PEN_ADJ,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_rg_pen_adj( rec, mode, key_no, direction, c_mesg )
Reg_pen_adj	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,REG_PEN_ADJ,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_rg_pen_adj( rec, mode, c_mesg )
Reg_pen_adj	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,REG_PEN_ADJ,c_mesg));
}
/*-----------------------------------------------------------*/
get_t4_rec( rec, mode, key_no, c_mesg )
T4_rec	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,T4_REC,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_t4_rec( rec, mode, key_no, direction, c_mesg )
T4_rec	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,T4_REC,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_t4_rec( rec, mode, c_mesg )
T4_rec	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,T4_REC,c_mesg));
}
/*-----------------------------------------------------------*/
get_glacct( rec, mode, key_no, c_mesg )
Gl_acct	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,GLACCT,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_glacct( rec, mode, key_no, direction, c_mesg )
Gl_acct	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,GLACCT,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_glacct( rec, mode, c_mesg )
Gl_acct	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,GLACCT,c_mesg));
}
/*-----------------------------------------------------------*/
get_jr_ent( rec, mode, key_no, c_mesg )
Jr_ent	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,JR_ENT,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_jr_ent( rec, mode, key_no, direction, c_mesg )
Jr_ent	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,JR_ENT,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_jr_ent( rec, mode, c_mesg )
Jr_ent	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,JR_ENT,c_mesg));
}
/*-----------------------------------------------------------*/
get_jrh_ent( rec, mode, key_no, c_mesg )
Jrh_ent	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,JRH_ENT,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_jrh_ent( rec, mode, key_no, direction, c_mesg )
Jrh_ent	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,JRH_ENT,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_jrh_ent( rec, mode, c_mesg )
Jrh_ent	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,JRH_ENT,c_mesg));
}
/*-----------------------------------------------------------*/
get_aud_pay( rec, mode, key_no, c_mesg )
Aud_pay	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,AUD_PAY,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_aud_pay( rec, mode, key_no, direction, c_mesg )
Aud_pay	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,AUD_PAY,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_aud_pay( rec, mode, c_mesg )
Aud_pay	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,AUD_PAY,c_mesg));
}
/*-----------------------------------------------------------*/
get_ec( rec, mode, key_no, c_mesg )
Ec_rec	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,EC_REC,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_ec( rec, mode, key_no, direction, c_mesg )
Ec_rec	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,EC_REC,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_ec( rec, mode, c_mesg )
Ec_rec	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,EC_REC,c_mesg));
}
/*-----------------------------------------------------------*/
get_chq_reg( rec, mode, key_no, c_mesg )
Chq_reg	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,CHQ_REG,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_chq_reg( rec, mode, key_no, direction, c_mesg )
Chq_reg	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,CHQ_REG,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_chq_reg( rec, mode, c_mesg )
Chq_reg	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,CHQ_REG,c_mesg));
}
/*-----------------------------------------------------------*/
get_man_chq( rec, mode, key_no, c_mesg )
Man_chq	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,MAN_CHQ,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_man_chq( rec, mode, key_no, direction, c_mesg )
Man_chq	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,MAN_CHQ,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_man_chq( rec, mode, c_mesg )
Man_chq	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,MAN_CHQ,c_mesg));
}
/*-----------------------------------------------------------*/
get_roe( rec, mode, key_no, c_mesg )
Roe	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,ROE,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_roe( rec, mode, key_no, direction, c_mesg )
Roe	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,ROE,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_roe( rec, mode, c_mesg )
Roe	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,ROE,c_mesg));
}
/*-----------------------------------------------------------*/
get_tmp_sched1( rec, mode, key_no, c_mesg )
Tmp_sched1	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,TMP_SCHED1,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_tmp_sched1( rec, mode, key_no, direction, c_mesg )
Tmp_sched1	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,TMP_SCHED1,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_tmp_sched1( rec, mode, c_mesg )
Tmp_sched1	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,TMP_SCHED1,c_mesg));
}
/*-----------------------------------------------------------*/
get_tmp_indx1( rec, mode, key_no, c_mesg )
Emp	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,TMPINDX_1,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_tmp_indx1( rec, mode, key_no, direction, c_mesg )
Emp 	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,TMPINDX_1,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_tmp_indx1( rec, mode, c_mesg )
Emp	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,TMPINDX_1,c_mesg));
}
/*-----------------------------------------------------------*/
get_sen_par( rec, mode, key_no, c_mesg )
Sen_par	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,SEN_PAR,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_sen_par( rec, mode, key_no, direction, c_mesg )
Sen_par	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,SEN_PAR,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_sen_par( rec, mode, c_mesg )
Sen_par	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,SEN_PAR,c_mesg));
}
/*-----------------------------------------------------------*/
get_emp_sen( rec, mode, key_no, c_mesg )
Emp_sen	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,EMP_SEN,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_emp_sen( rec, mode, key_no, direction, c_mesg )
Emp_sen	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,EMP_SEN,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_emp_sen( rec, mode, c_mesg )
Emp_sen	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,EMP_SEN,c_mesg));
}
/*-----------------------------------------------------------*/
get_tmp_sen( rec, mode, key_no, c_mesg )
Tmp_sen	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,TMP_SEN,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_tmp_sen( rec, mode, key_no, direction, c_mesg )
Tmp_sen	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,TMP_SEN,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_tmp_sen( rec, mode, c_mesg )
Tmp_sen	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,TMP_SEN,c_mesg));
}
/*-----------------------------------------------------------*/
get_gov_param( rec, mode, key_no, c_mesg )
Gov_param	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,GOV_PARAM,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_gov_param( rec, mode, key_no, direction, c_mesg )
Gov_param	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,GOV_PARAM,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_gov_param( rec, mode, c_mesg )
Gov_param	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,GOV_PARAM,c_mesg));
}
/*-----------------------------------------------------------*/
get_t4_adj( rec, mode, key_no, c_mesg )
T4_adj	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,T4_ADJ,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_t4_adj( rec, mode, key_no, direction, c_mesg )
T4_adj	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,T4_ADJ,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_t4_adj( rec, mode, c_mesg )
T4_adj	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,T4_ADJ,c_mesg));
}
/*-----------------------------------------------------------*/
get_rel( rec, mode, key_no, c_mesg )
Religion	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,RELIGION,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_rel( rec, mode, key_no, direction, c_mesg )
Religion	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,RELIGION,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_rel( rec, mode, c_mesg )
Religion	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,RELIGION,c_mesg));
}
/*-----------------------------------------------------------*/
get_userbarg( rec, mode, key_no, c_mesg )
Userbarg	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,USERBARG,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_userbarg( rec, mode, key_no, direction, c_mesg )
Userbarg	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,USERBARG,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_userbarg( rec, mode, c_mesg )
Userbarg	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,USERBARG,c_mesg));
}
/*-----------------------------------------------------------*/
get_tmp_emp( rec, mode, key_no, c_mesg )
Tmp_Emp	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,TMP_EMP,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_tmp_emp( rec, mode, key_no, direction, c_mesg )
Tmp_Emp	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,TMP_EMP,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_tmp_emp( rec, mode, c_mesg )
Tmp_Emp	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,TMP_EMP,c_mesg));
}
/*-----------------------------------------------------------*/
get_tmp2_emp( rec, mode, key_no, c_mesg )
Tmp2_Emp	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,TMP2_EMP,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_tmp2_emp( rec, mode, key_no, direction, c_mesg )
Tmp2_Emp	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,TMP2_EMP,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_tmp2_emp( rec, mode, c_mesg )
Tmp2_Emp	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,TMP2_EMP,c_mesg));
}
/*-----------------------------------------------------------*/
get_ts_sen( rec, mode, key_no, c_mesg )
Ts_sen	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,TS_SEN,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_ts_sen( rec, mode, key_no, direction, c_mesg )
Ts_sen	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,TS_SEN,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_ts_sen( rec, mode, c_mesg )
Ts_sen	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,TS_SEN,c_mesg));
}
/*-----------------------------------------------------------*/
get_vc_acc( rec, mode, key_no, c_mesg )
Vc_acc	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,VC_ACC,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_vc_acc( rec, mode, key_no, direction, c_mesg )
Vc_acc	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,VC_ACC,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_vc_acc( rec, mode, c_mesg )
Vc_acc	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,VC_ACC,c_mesg));
}
/*-----------------------------------------------------------*/
get_emp_comp( rec, mode, key_no, c_mesg )
Emp_comp	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,EMP_COMP,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_emp_comp( rec, mode, key_no, direction, c_mesg )
Emp_comp	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,EMP_COMP,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_emp_comp( rec, mode, c_mesg )
Emp_comp	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,EMP_COMP,c_mesg));
}
/*-----------------------------------------------------------*/
get_comp( rec, mode, key_no, c_mesg )
Comp	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,COMP,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_comp( rec, mode, key_no, direction, c_mesg )
Comp	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,COMP,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_comp( rec, mode, c_mesg )
Comp	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,COMP,c_mesg));
}
/*-----------------------------END OF FILE------------------------------------*/
