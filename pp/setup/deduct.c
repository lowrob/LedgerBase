/*----------------------------------------------------------------------
Source Name: deduct.c  
System     : PP
Module     : Payroll/Personel
Created On : Oct. 22, 1991
Created By : Sheldon Floyd

DESCRIPTION:

	This program allows add, change, delete and inquire on the
	deduction file. The user must complete two screens of information
	before the file can be modified.

MODIFICATIONS:        

Programmer     	YY/MM/DD       	Description of modification
~~~~~~~~~~     	~~~~~~~~       	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
------------------------------------------------------------------------*/

#define	MAIN	
#define	MAINFL		DEDUCTION

#include <stdio.h>
#include <ctype.h>
#include <cfomstrc.h>
#include <repdef.h>
#include <bfs_defs.h>
#include <bfs_pp.h>
#include <bfs_com.h>
#include <pp_msgs.h>
#include <deduct.h>

#define MOD_DATE	"22-OCT-91"
#define SYSTEM   	"PAYROLL/PERSONEL"

char	txt_buff[80];	
char	txt_buff2[80];	
int	first_time_called = 1;
int	first_time = 0;

static	int	Validate();
static	int	WindowHelp();

/*-------------------------------------------------------------------*/

main(argc,argv)
int argc;
char *argv[];
{
	
	strncpy(SYS_NAME,SYSTEM,50);		/* Sub system name */
	strncpy(CHNG_DATE,MOD_DATE,10);		/* Modification Date */
	proc_switch(argc, argv, MAINFL) ; 	/* Process Switches */

	strcpy(sr.termnm,terminal);	/* Copy Terminal Name */
	
	if (Initialize()<0)
		exit(-1);		/* Initialization routine */

	if (Deduct() < 0){ 		/* maintenance processing */
		Close();
		exit(-1);
	}

	Close();			/* return to menu */
	exit(NOERROR);

} /* end of main */

/*-------------------------------------------------------------------*/
/* Reset information */

static
Close()
{
	/* Set terminal back to normal mode from PROFOM */
	FreeList();
	fomcs();
	fomrt();
	close_dbh();			/* Close files */
	close_rep();

	return(NOERROR);
}

/*-------------------------------------------------------------------*/
/* Initialize PROFOM  and Screens*/

static
Initialize()
{

	if(InitProfom()<0) { 			/* Initialize PROFOM */
		fomcs();
		fomrt();
		return(-1);
	}	
	if(InitScreens()<0) { 
		fomcs();
		fomrt();
		return(-1);
	}	

	return(NOERROR);
}
/*-------------------------------------------------------------------*/
/* Initialize PROFOM */
static
InitProfom()
{
	fomin(&sr);		/* Check for Error */
	ret( err_chk(&sr) );

	fomcf(1,1);		/* Enable Print screen option */
	return(NOERROR);

}	/* InitProfom() */
/*----------------------------------------------------------------*/
/* Initialize screens before going to process options */
static
InitScreens()
{

	int retval, i; 

	strcpy(sr.scrnam,NFM_PATH);
	strcat(sr.scrnam,SCRNM1) ;

	s1_sth.s_sysdate = get_date();	
	s1_sth.s_field = HV_SHORT ;

	s1_sth.s_ftn[0] = HV_CHAR;

	s1_sth.s_mesg[0] = HV_CHAR;
	s1_sth.s_opt[0] = HV_CHAR;

	retval = ClearScreen();

	if(WriteFields((char*)&s1_sth,FTN_FLD1,OPT_FLD1)<0) return(-1);

	return(NOERROR);

}	/* InitScreens() */
/*-------------------------------------------------------------------
process maintenance selections and make changes to file
-------------------------------------------------------------------*/
static
Deduct() 
{
	int retval;
	deduct.dd_code[0] = '\0';
	deduct.dd_pp_code[0] = '\0';
	flg_reset(DEDUCTION);

	retval = get_pay_param(&param,BROWSE,1,e_mesg);
	if(retval < 0 && retval != UNDEF){
		DispError((char *)&s1_sth,e_mesg);
		return(retval);
	}
	if(retval == UNDEF){
#ifdef ENGLISH
		fomen("Payroll Parameters Are Not Setup..");
#else
		fomen("Parametres Payroll ne sont pas etablis..");
#endif
		get() ;
	}

	for(;;){
#ifdef ENGLISH
		fomen("A(dd), C(hange), D(elete), I(nquire), N(ext), P(rev), E(xit)");
#else
		fomen("A(dd), C(hange), D(elete), I(nquire), N(ext), P(rev), E(xit)");
#endif

		s1_sth.s_ftn[0] = LV_CHAR;

		/* read the screen fields */
		retval = ReadFields((char*)&s1_sth,FTN_FLD1,FTN_FLD1,Validate,
							WindowHelp,1);
		if(retval == RET_USER_ESC) continue;
		if(retval <0) return(-1);

		first_time_called = 1;
		retval = ClearScreen();
		if(retval < 0) return(retval);

		if(s1_sth.s_ftn[0] == ADDREC || s1_sth.s_ftn[0] == DELREC ||
			s1_sth.s_ftn[0] == CHGREC || s1_sth.s_ftn[0] == INQREC){
			s1_sth.s_dedcd[0] = LV_CHAR;
			s1_sth.s_paypd[0] = LV_CHAR;

			retval = ReadFields((char*)&s1_sth,DEDCD_FLD1,
				PAYPD_FLD1,Validate,WindowHelp,1);
		
			if(retval == RET_USER_ESC) continue;
			if(retval <0) return(-1);
		}
		
		if(s1_sth.s_ftn[0] == EXIT) break;

		if(s1_sth.s_ftn[0] != ADDREC){
			retval = DisplayRec();
			if(retval < 0) return(retval);
		}

		if(s1_sth.s_ftn[0] == INQREC){
			retval = InquireRec();
			if(retval < 0) return(retval);
			if(retval==CANCEL) continue;
		}

		if(s1_sth.s_ftn[0] == DELREC){
			retval = DeleteRec();
			if(retval < 0) return(retval);
			if(retval==CANCEL) continue;
		}

		if(s1_sth.s_ftn[0] == ADDREC){
			MoveLows();
			retval = ReadFields((char*)&s1_sth,DDESC_FLD,
				PD5_FLD, Validate,WindowHelp,0);
			if(retval <0) return(-1);
	
			retval = ReadFields((char*)&s1_sth,DDESC_FLD,PAYPD_FLD1,
				Validate, WindowHelp,0);

			if(retval <0) return(-1);
		}

		if(s1_sth.s_ftn[0] != ADDREC && s1_sth.s_ftn[0] != CHGREC
			&& s1_sth.s_ftn[0] != DELREC) continue;

		if(s1_sth.s_ftn[0] != DELREC){
			retval=ReadOption();		
			if(retval<0) return(retval);
			if(retval==CANCEL) continue;
		}

		retval = UpdateFiles();
		if(retval < 0) return(retval);
	}

	return(NOERROR);
}
/*-----------------------------------------------------------------------*/
/* allows the user to inquire on both screens of information		 */
InquireRec()
{
	int retval;

	for( ; ; ) {
#ifdef	ENGLISH
		retval = GetOpt("Y(es), 2(nd screen), N(ext), P(rev)", "Y2NP");
#else	
		retval = GetOpt("Y(es), 2(nd screen), N(ext), P(rev)", "Y2NP");
#endif

	 	if(retval == YES) break;

	 	if(retval == SECOND_SCR){
			retval = DisplayScreen2();
			if(retval < 0) return(retval);
			if(retval == YES) break;
		}

		if(retval == NEXTREC){
		  if(CurPage == CurLast || CurLast == NULL) {
#ifdef ENGLISH
			fomer("No More Pages....");
#else
			fomer("Plus de pages....");
#endif
			continue;
		  }
		  CurPage = CurPage->NextPage ;
		  retval = ShowItems(CurPage);
		}

		if(retval == PREVREC){
		  if(CurLast == NULL || CurPage == FirstPage) {
#ifdef ENGLISH
			fomer("No More Pages....");
#else
			fomer("Plus de pages....");
#endif
			continue;
		  }
		  CurPage = CurPage->PrevPage ;
		  retval = ShowItems(CurPage);
		}
	}

	return(CANCEL);
}
/*-----------------------------------------------------------------------*/
/* allows the user to view the second screen before committing the delete*/
DeleteRec()
{
	int retval;

	for( ; ; ) {
#ifdef	ENGLISH
		retval = GetOpt("Y(es), C(ancel)", "YC");
#else	
		retval = GetOpt("Y(es), C(ancel)", "YC");
#endif
		if(retval == YES) break;

		if(retval == CANCEL) return(CANCEL);
	}

	return(NOERROR);
}
/*-------------------------------------------------------------------*/
static
Page *PageAllocated()	/* Allocate memory for 1 page of items on screen */
{
	return( (Page *)(malloc((unsigned)sizeof( Page ))) );
}
/*-----------------------------------------------------------------------*/
/* display record contents for all options but add			 */
DisplayRec()
{
	int	retval, i;

	if(s1_sth.s_ftn[0] == NEXTREC){
		if(first_time != 0){
			inc_str(deduct.dd_pp_code,sizeof(deduct.dd_pp_code)-1,
				FORWARD);
			flg_reset(DEDUCTION);
		}
		first_time = 1;
		retval = get_n_deduction(&deduct,BROWSE,0,FORWARD,e_mesg);
		if(retval == EFL){
			fomen(NOMORE);
			get();
			return(NOERROR);
		}
		if(retval < 0){
			DispError((char *)&s1_sth,e_mesg);
			return(retval);
		}
	}

	if(s1_sth.s_ftn[0] == PREVREC){
		if(first_time != 0){
			inc_str(deduct.dd_pp_code,sizeof(deduct.dd_pp_code)-1,
				BACKWARD);
			flg_reset(DEDUCTION);
		}
		first_time = 1;
		retval = get_n_deduction(&deduct,BROWSE,0,BACKWARD,e_mesg);
		if(retval == EFL){
			fomen(NOMORE);
			get();
			return(NOERROR);

		}
		if(retval < 0){
			DispError((char *)&s1_sth,e_mesg);
			return(retval);
		}
	}

	if(s1_sth.s_ftn[0] != NEXTREC && s1_sth.s_ftn[0] != PREVREC){
		strcpy(deduct.dd_code,s1_sth.s_dedcd);
		strcpy(deduct.dd_pp_code,s1_sth.s_paypd);
		retval = get_deduction(&deduct,BROWSE,0,e_mesg);

		if(retval < 0){
			DispError((char *)&s1_sth,e_mesg);
			return(retval);
		}
	}

	retval = ShowScreen();

	return(NOERROR);
}
/*-----------------------------------------------------------------------*/
/* update changes to the deduction file					 */
UpdateFiles()
{
	int retval, i;
	Page	*temppage;
	int	mode;
	int	write_mode;


	strcpy(deduct.dd_code,s1_sth.s_dedcd);
	strcpy(deduct.dd_pp_code,s1_sth.s_paypd);
	retval = get_deduction(&deduct,UPDATE,0,e_mesg);
	if(s1_sth.s_ftn[0] != ADDREC){	
		if(retval <0){
			DispError((char *)&s1_sth,e_mesg);
			return(retval);
		}
		mode = UPDATE;
	}
	else{
		if(retval < 0 && retval != UNDEF){
			DispError((char *)&s1_sth,e_mesg);
			return(retval);
		}
		if(retval >= 0){
			return(retval);
		}
		mode = ADD;
	}

	strcpy(deduct.dd_desc,s1_sth.s_desc);
	if(s1_sth.s_secode[0] != LV_CHAR && s1_sth.s_secode[0] != HV_CHAR){
		strcpy(deduct.dd_second,s1_sth.s_secode);
	}
	else{
		strcpy(deduct.dd_second,"  ");
	}

	strcpy(deduct.dd_t4_fld,s1_sth.s_t4);
	deduct.dd_min_earn = s1_sth.s_earn;
	deduct.dd_max_contr = s1_sth.s_cont;

	for(i=0;i<NO_PDS;i++){
		strcpy(deduct.dd_ded_pp[i],s1_sth.s_monpd[i].dmp);
	}

	deduct.dd_fund = s2_sth.s_fund;
	strcpy(deduct.dd_lia_acct,s2_sth.s_account);

	for(i=0;i<MAX_KEYS;i++){
		if(s2_sth.s_acckey[i] == LV_SHORT || 
			s2_sth.s_acckey[i] == HV_SHORT){
			deduct.dd_exp_acct[i] = 0;
			continue;
		}
		deduct.dd_exp_acct[i] = s2_sth.s_acckey[i];
	}
	/* If system is not intergrated with General Ledger there is no need
	   to update account file 					     */
/*
	if( param.pr_up_gl[0] == 'Y') {
		retval = UpdateAcct();
		if(retval < 0) {	
			DispError((char *)&s2_sth,e_mesg);
			return(retval);
		}
	}
*/

	if(s1_sth.s_ftn[0] == CHGREC){
		retval = put_deduction(&deduct,UPDATE,e_mesg);
	}
	if(s1_sth.s_ftn[0] == ADDREC){
		retval = put_deduction(&deduct,ADD,e_mesg);
	}
	if(s1_sth.s_ftn[0] == DELREC){
		retval = put_deduction(&deduct,P_DEL,e_mesg);
	}

	if(retval < 0) return(retval);

	retval = commit(e_mesg);
	if(retval < 0) return(retval);

		/* to write the items in file */

	if(CurLast != NULL) {
	   for(temppage=FirstPage; temppage!=NULL;temppage=temppage->NextPage) {
	      for(i =0; i< temppage->NoItems; i++) {
		 retval = GetMode(temppage,i,mode,&write_mode);
		 if(write_mode == NOOP) continue;
		 retval = WriteRecords(temppage,i,write_mode);
  		 if(retval < 0) {
			if(retval == LOCKED) return(LOCKED);
		   	break;
		 }
	      }
	      if(temppage == CurLast) break;
	   }
	}
	retval = commit(e_mesg) ;
	if(retval < 0) {
		DispError((char *)&s1_sth,"ERROR: Saving Records"); 
		DispError((char *)&s1_sth,e_mesg);
		roll_back(e_mesg);
		return(retval);
	}
	return(NOERROR);
}
/*-----------------------------------------------------------------------*/ 
static int
GetMode(temppage,item_no,mode,write_mode)
Page	*temppage;
int	item_no;
int	mode;
int	*write_mode;
{
	if(mode == ADD) {
		if(strcmp(temppage->Items[item_no].s_status,INACTIVE)==0) {
			*write_mode = NOOP;
		}
		else {
			*write_mode = ADD;
		}
	}
	else if(mode == UPDATE) {
		if(strcmp(temppage->Items[item_no].s_status,INACTIVE)==0) {
			*write_mode = P_DEL;
		}
		else if(temppage->I_Status[item_no][0] == ADDITEM) {
			*write_mode = ADD;
		}
		else if(temppage->I_Status[item_no][0] == CHGREC) {
			*write_mode = UPDATE;
		}
		else if(temppage->I_Status[item_no][0] == ' ') {
			*write_mode = NOOP;
		}
	}
	else if(mode == P_DEL) {
		*write_mode = P_DEL;
	}

	return(NOERROR);
}
/*-----------------------------------------------------------------------*/ 
/* Write the Area record to the file.			 		 */ 
/*-----------------------------------------------------------------------*/ 
static int
WriteRecords(temppage,item_no,mode)
Page	*temppage;
int	item_no;
int	mode;
{
	int	retval;
	int	i;

	scpy((char *)&pre_dedgrp,(char *)&dedgrp,sizeof(dedgrp));

	strcpy(dedgrp.dg_group,temppage->Items[item_no].s_group);
	strcpy(dedgrp.dg_code,s1_sth.s_dedcd);
	strcpy(dedgrp.dg_pp_code,s1_sth.s_paypd);
	
	if(mode != ADD) {
		retval = get_ded_grp(&dedgrp,UPDATE,0,e_mesg);
		if(retval < 0 && retval != UNDEF){
			DispError((char *)&s1_sth,e_mesg);
			return(retval);
		}
	}

	dedgrp.dg_amount = temppage->Items[item_no].s_amount;
	strcpy(dedgrp.dg_desc,temppage->Items[item_no].s_gdesc);
	strcpy(dedgrp.dg_amt_flag,temppage->Items[item_no].s_fp);
	dedgrp.dg_employer_sh = temppage->Items[item_no].s_ershare;


	retval = put_ded_grp(&dedgrp,mode,e_mesg) ;
	if(retval < 0) {
		DispError((char *)&s1_sth,e_mesg);
		roll_back(e_mesg);
		return(retval);
	}

	if(mode != ADD) {
		retval = rite_audit((char*)&s1_sth,DED_GRP,mode,(char*)&dedgrp,
			(char*)&pre_dedgrp,e_mesg);
		if(retval==LOCKED) {
			DispError((char *)&s1_sth,e_mesg);
			roll_back(e_mesg) ;
			return(LOCKED) ;
		}
		if(retval < 0 ){
			DispError((char *)&s1_sth,"ERROR: Saving Records"); 
			DispError((char *)&s1_sth,e_mesg);
			roll_back(e_mesg);
			return(retval);
		}
	}

	return(NOERROR);
}
/*-----------------------------------------------------------------------*/
/* Validate flds when PROFOM returns RET_VAL_CHK    			 */
static
Validate()	
{
	int	i, retval, fld_no, item_no, st_fld;
	double	temp_calc, total_units;
	int	temp_fld, check;
	Page	*itemptr;

	if(sr.curfld >= ITEM_ST_FLD) {	
		item_no = (sr.curfld - ITEM_ST_FLD) / STEP;
		fld_no = sr.curfld - (STEP * item_no);

		st_fld = ITEM_ST_FLD + (STEP * item_no);
	}
	else 
		fld_no = sr.curfld;

	switch(sr.curfld){
	case	FTN_FLD1:
		if(s1_sth.s_ftn[0] == '\0'){
			s1_sth.s_ftn[0] = LV_CHAR;
			return(-1);
		}

		if(s1_sth.s_ftn[0] == ADDREC) break;
		if(s1_sth.s_ftn[0] == CHGREC) break;
		if(s1_sth.s_ftn[0] == DELREC) break;
		if(s1_sth.s_ftn[0] == INQREC) break;
		if(s1_sth.s_ftn[0] == NEXTREC) break;
		if(s1_sth.s_ftn[0] == PREVREC) break;
		if(s1_sth.s_ftn[0] == EXIT){
			s1_sth.s_dedcd[0] = HV_CHAR;
			s1_sth.s_paypd[0] = HV_CHAR;
			break;
		}

#ifdef ENGLISH
		fomen("Must Be A, C, D, I, N, P, or E");
#else
		fomen("Must Be A, C, D, I, N, P, or E");
#endif
		s1_sth.s_ftn[0] = LV_CHAR;
		return(-1);

	case	DEDCD_FLD1:
		if(s1_sth.s_dedcd[0] == '\0'){
			fomen("This if a required field");get ();
			s1_sth.s_paypd[0] = LV_CHAR;
			return(-1);
		}
		break;
	case	PAYPD_FLD1:
		if(s1_sth.s_paypd[0] == '\0'){
			s1_sth.s_paypd[0] = LV_CHAR;
			return(-1);
		}

		strcpy(payper.pp_code,s1_sth.s_paypd);
		payper.pp_year = 0;
		flg_reset(PAY_PERIOD);
		for(;;){
			retval = get_n_pay_per(&payper,BROWSE,0,FORWARD,e_mesg);
			if(retval == EFL){
				fomen(NOKEY);
				s1_sth.s_paypd[0] = LV_CHAR;
				return(-1);
			}
			if(retval < 0){	
				DispError((char *)&s1_sth,e_mesg);
				return(-1);
			}

			if(strcmp(payper.pp_code,s1_sth.s_paypd)==0) break;
		}
		seq_over(PAY_PERIOD);

		strcpy(deduct.dd_code,s1_sth.s_dedcd);
		strcpy(deduct.dd_pp_code,s1_sth.s_paypd);
		retval = get_deduction(&deduct,BROWSE,0,e_mesg);
		if(retval == UNDEF && s1_sth.s_ftn[0] != ADDREC){
			fomen(NOKEY);
			s1_sth.s_dedcd[0] = LV_CHAR;
			s1_sth.s_paypd[0] = LV_CHAR;
			sr.curfld-=100;
			return(-1);
		}
		if(retval < 0 && s1_sth.s_ftn[0] != ADDREC){
			DispError((char *)&s1_sth,e_mesg);
			return(retval);
		}

		if(retval >= 0 && s1_sth.s_ftn[0] == ADDREC){
#ifdef ENGLISH
			fomen("Deduction Already Exists");
#else
			fomen("Deduction Already Exists");
#endif
			s1_sth.s_dedcd[0] = LV_CHAR;
			s1_sth.s_paypd[0] = LV_CHAR;
			sr.curfld-=100;
			return(-1);
		}

		if(s1_sth.s_ftn[0] == DELREC){
			check = 0;
			empdh.edh_pp=payper.pp_numb;
			empdh.edh_numb[0] = '\0';
			strcpy(empdh.edh_code,deduct.dd_code);
			empdh.edh_date = 0;
			flg_reset(EMP_DED_HIS);
			for(;;){
				retval = get_n_emp_dhis(&empdh,BROWSE,0,
					FORWARD,e_mesg);
				if(retval == EFL) break;
				if(strcmp(empdh.edh_pp,s1_sth.s_paypd)==0){
					if(empdh.edh_amount > 0) check = 1;
					break;
				}
			}
			seq_over(EMP_DED_HIS);

			if(check == 1){
#ifdef ENGLISH
				fomen("Period Not Year End - Please Re-enter");
#else
				fomen("Period Not Year End - Please Re-enter");
#endif
				s1_sth.s_dedcd[0] = LV_CHAR;
				s1_sth.s_paypd[0] = LV_CHAR;
				sr.curfld-=100;
				return(-1);
			}
		}
		break;

	case	DDESC_FLD:
		if(s1_sth.s_desc[0] == '\0'){
			s1_sth.s_desc[0] = LV_CHAR;
			return(-1);
		}
		break;

	case	T4_FLD:
/******** Found no use for this, no place to add t4_code to t4_rec? L.R.
		if(s1_sth.s_t4[0] == '\0'){
			s1_sth.s_t4[0] = LV_CHAR;
			return(-1);
		}

		strcpy(t4rec.t4_code,s1_sth.s_t4);
		retval = get_t4_rec(&t4rec,BROWSE,0,e_mesg);
		if(retval == UNDEF){
			fomen(NOKEY);
			s1_sth.s_t4[0] = LV_CHAR;
			return(-1);
		}

		if(retval < 0){
			DispError((char *)&s1_sth,e_mesg);
			return(retval);
		}
		
		strcpy(s1_sth.s_tdesc,t4rec.t4_desc);
		WriteFields((char *)&s1_sth,TDESC_FLD,TDESC_FLD);
***************/
		break;

	case	PD1_FLD:
		if (s1_sth.s_monpd[0].dmp[0] != 'Y' &&
		    s1_sth.s_monpd[0].dmp[0] != 'N') {
			fomen("Must enter Y or N");
			get();
			s1_sth.s_monpd[0].dmp[0] = LV_CHAR;
			return(ERROR);
		}
		if(s1_sth.s_monpd[0].dmp[0] == 'N') {
			for(i=1;i<5;i++)
				s1_sth.s_monpd[i].dmp[0] = 'N' ;
		}
		sr.curfld+=100;
		break;
	case	PD2_FLD:
		if (s1_sth.s_monpd[1].dmp[0] != 'Y' &&
		    s1_sth.s_monpd[1].dmp[0] != 'N') {
			fomen("Must enter Y or N");
			get();
			s1_sth.s_monpd[1].dmp[0] = LV_CHAR;
			return(ERROR);
		}
		if(s1_sth.s_monpd[1].dmp[0] == 'N') {
			for(i=2;i<5;i++)
				s1_sth.s_monpd[i].dmp[0] = 'N' ;
		}
		sr.curfld+=100;
		break;
	case	PD3_FLD:
		if (s1_sth.s_monpd[1].dmp[0] != 'Y' &&
		    s1_sth.s_monpd[1].dmp[0] != 'N') {
			fomen("Must enter Y or N");
			get();
			s1_sth.s_monpd[2].dmp[0] = LV_CHAR;
			return(ERROR);
		}
		if(s1_sth.s_monpd[2].dmp[0] == 'N') {
			for(i=3;i<5;i++)
				s1_sth.s_monpd[i].dmp[0] = 'N' ;
		}
		sr.curfld+=100;
		break;
	case	PD4_FLD:
		if (s1_sth.s_monpd[1].dmp[0] != 'Y' &&
		    s1_sth.s_monpd[1].dmp[0] != 'N') {
			fomen("Must enter Y or N");
			get();
			s1_sth.s_monpd[3].dmp[0] = LV_CHAR;
			return(ERROR);
		}
		if(s1_sth.s_monpd[3].dmp[0] == 'N') {
			for(i=4;i<5;i++)
				s1_sth.s_monpd[i].dmp[0] = 'N' ;
		}
		sr.curfld+=100;
		break;
	case	PD5_FLD:
		if (s1_sth.s_monpd[1].dmp[0] != 'Y' &&
		    s1_sth.s_monpd[1].dmp[0] != 'N') {
			fomen("Must enter Y or N");
			get();
			s1_sth.s_monpd[4].dmp[0] = LV_CHAR;
			return(ERROR);
		}
		sr.curfld+=100;
		break;

	case	ITEM_ST_FLD:
	case	GRP_FLD2:
	case	ITEM_END_FLD:
		if(s1_sth.s_groups[item_no].s_group[0] == '\0'){
			s1_sth.s_groups[item_no].s_group[0] = LV_CHAR;
			return(-1);
		}
		retval = ItemCheck(item_no); 
		if(retval < 0){
			s1_sth.s_groups[item_no].s_group[0] = LV_CHAR;
			return(-1);
		}
		break;

	case	STFP_FLD:
	case	FP_FLD2:
	case	ENDFP_FLD:
		if(s1_sth.s_groups[item_no].s_fp[0] == '\0'){
			s1_sth.s_groups[item_no].s_fp[0] = LV_CHAR;
			return(-1);
		}
		
		if(s1_sth.s_groups[item_no].s_fp[0] != FIXED &&
			s1_sth.s_groups[item_no].s_fp[0] != PERCENT){
#ifdef ENGLISH
			fomen("Must Be F or P");
#else
			fomen("Must Be F or P");
#endif
			s1_sth.s_groups[item_no].s_fp[0] = LV_CHAR;
			return(-1);
		}
		break;

	case	STESH_FLD:
	case	ESH_FLD2:
	case	ENDESH_FLD:
		if(s1_sth.s_groups[item_no].s_eeshare > 100){
#ifdef	ENGLISH
			fomen("Employee Share Must Be Less Than 100");
#else
			fomen("Employee Share Must Be Less Than 100");
#endif
			s1_sth.s_groups[item_no].s_eeshare = LV_SHORT;
			return(-1);
		}

		s1_sth.s_groups[item_no].s_ershare =
			100 - s1_sth.s_groups[item_no].s_eeshare;
		WriteFields((char *)&s1_sth,sr.curfld+100,sr.curfld+100);
		break;
		
	default:
		break;
	}/* end switch */	

	return(NOERROR) ;

}	/* Validate() */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/*  Display help window for applicable fields                            */
static
WindowHelp()
{
	int	i, err, retval;

	switch(sr.curfld){
	case DEDCD_FLD1:
		err = deduction_hlp(s1_sth.s_dedcd,s1_sth.s_paypd,7,13);
		if(err < 0) return(err);
		redraw();

		if(s1_sth.s_dedcd[0] == '\0'){
			s1_sth.s_dedcd[0] = LV_CHAR;
			return(-1);
		}
		break;

	case PAYPD_FLD1:
		err = payper_hlp(s1_sth.s_paypd,7,13);
		if(err < 0) return(err);
		redraw();

		if(s1_sth.s_paypd[0] == '\0'){
			s1_sth.s_paypd[0] = LV_CHAR;
			return(-1);
		}

		strcpy(deduct.dd_code,s1_sth.s_dedcd);
		strcpy(deduct.dd_pp_code,s1_sth.s_paypd);
		retval = get_deduction(&deduct,BROWSE,0,e_mesg);
		if(retval == UNDEF && s1_sth.s_ftn[0] != ADDREC){
			fomen(NOKEY);
			s1_sth.s_dedcd[0] = LV_CHAR;
			s1_sth.s_paypd[0] = LV_CHAR;
			return(-1);
		}
		if(retval < 0 && s1_sth.s_ftn[0] != ADDREC){
			DispError((char *)&s1_sth,e_mesg);
			return(retval);
		}

		if(retval >= 0 && s1_sth.s_ftn[0] == ADDREC){
#ifdef ENGLISH
			fomen("Deduction Already Exists");
#else
			fomen("Deduction Already Exists");
#endif
			s1_sth.s_dedcd[0] = LV_CHAR;
			s1_sth.s_paypd[0] = LV_CHAR;
			return(-1);
		}
		break;

	default:
		fomen(NOHELP);
		break;
	}
	sr.nextfld = sr.curfld ;
	return(NOERROR) ;

}	/* WindowHelp() */
/*-----------------------------------------------------------------------*/
/* Prevents duplicate code from being entered				 */
static
ItemCheck(curr_item)
int curr_item;
{
	int  a;
	int	i;
	Page	*temppage;

	/* check to see if item is already in list */
	if(CurLast != NULL) {
	   for(temppage=FirstPage; temppage!=NULL;temppage=temppage->NextPage) {
	      for(i =0; i< temppage->NoItems; i++) {
			if(temppage->Items[i].s_group[0] == NULL) {
				return(NOERROR);
			}
			if(strcmp(temppage->Items[i].s_group,
				s1_sth.s_groups[curr_item].s_group) == 0) {
				return(-1);
			}
	      }
	      if(temppage == CurLast) break;
	   }
	}

	return(NOERROR);
}

/*-----------------------------------------------------------------------*/
/* Display the confirmation message at the bottom of the screen, take the
   option from user and call necessary functions */
/*-----------------------------------------------------------------------*/
static
ReadOption()
{
	int	retval;

	for( ; ; ) {

#ifdef	ENGLISH
		retval = GetOpt("Y(es), H(eader edit), L(ine edit), E(dit item), 2(nd screen), C(ancel)", "YHLE2C");
#else	
		retval = GetOpt("Y(es), H(eader edit), L(ine edit), E(dit item), 2(nd screen), C(ancel)", "YHLE2C");
#endif

		switch(retval) {	/* process the option */
		case  YES :
			if(first_time_called == 1){
#ifdef ENGLISH
				fomen("Second Screen Information Must be Entered");
#else
				fomen("Second Screen Information Must be Entered");
#endif
				get();
				break;
			}	

			s1_sth.s_mesg[0] = HV_CHAR;
			sr.nextfld = MESG_FLD1; 
			fomwf((char *)&s1_sth);
	
			return(YES);	

		case  HDREDIT :
			retval = ChangeScreen();
			if(retval < 0) return(retval);
			break;

		case  LINEEDIT:
			retval = ChangeFields();
			if(retval < 0) return(retval);
			break;

		case  EDITITEMS:
			retval = GetNextOpt();
			if(retval < 0) return(retval);
			break;

		case  SECOND_SCR:
			strcpy(sr.scrnam,NFM_PATH);
			strcat(sr.scrnam,SCRNM2) ;

			if(first_time_called == 1 && s1_sth.s_ftn[0] != CHGREC){
				first_time_called = 0;
				retval = GetScreen2();
				if(retval == CANCEL) return(retval);
			}
			else{
				strcpy(s2_sth.s_ftn,s1_sth.s_ftn);
				strcpy(s2_sth.s_dedcd,s1_sth.s_dedcd);
				strcpy(s2_sth.s_paypd,s1_sth.s_paypd);
				retval = ReadOption2();
				if(retval == CANCEL) return(retval);
			}
			first_time_called = 0;
			break;

		case  CANCEL:
#ifdef	ENGLISH
			retval = GetOpt("Confirm The Cancel (Y/N)?", "YN");
#else	
			retval = GetOpt("Confirmer l'annulation (O/N)?", "ON");
#endif
			if ( retval == YES ) 
				return( CANCEL );
			break ;

		}/* switch */

		if(retval < 0) return(retval);

	}	/* for(;;) */
}	
/*-------------------------------------------------------------------*/
ClearScreen()
{
	int retval, i;

	s1_sth.s_dedcd[0] = HV_CHAR;
	s1_sth.s_paypd[0] = HV_CHAR;
	s1_sth.s_desc[0] = HV_CHAR;

	s1_sth.s_secode[0] = HV_CHAR;
	s1_sth.s_t4[0] = HV_CHAR;
	s1_sth.s_tdesc[0] = HV_CHAR;
	s1_sth.s_earn = HV_DOUBLE;

	s1_sth.s_cont = HV_DOUBLE;
	for(i=0;i<NO_PDS;i++){
		s1_sth.s_monpd[i].dmp[0] = HV_CHAR;
	}

	WriteFields((char *)&s1_sth,DEDCD_FLD1,PAGE_FLD);

	s2_sth.s_fund = HV_SHORT;
	s2_sth.s_account[0] = HV_CHAR;
	
	for(i=0;i<MAX_KEYS;i++){
		s2_sth.s_acckey[i] = HV_SHORT;
	}

	s1_sth.s_page = HV_SHORT;

	for(i=0;i<PAGESIZE1;i++){
		ClearItemLines(HV_CHAR,HV_DOUBLE,HV_SHORT,i);
	}

	retval = WriteFields((char*)&s1_sth,PD5_FLD, OPT_FLD1-100);
	if(retval < 0) return(retval);

	return(NOERROR);
}
/*-------------------------------------------------------------------
Fill the item array with high values
-------------------------------------------------------------------*/
ClearItemLines(t_char,t_double,t_short,item_no)
char	t_char;
double	t_double;
short	t_short;
int	item_no;
{
	s1_sth.s_groups[item_no].s_group[0] = t_char;
	s1_sth.s_groups[item_no].s_gdesc[0] = t_char;
	s1_sth.s_groups[item_no].s_fp[0] = t_char;
	s1_sth.s_groups[item_no].s_amount = t_double;
	s1_sth.s_groups[item_no].s_eeshare = t_short;
	s1_sth.s_groups[item_no].s_ershare = t_short;
	if(t_char == HV_CHAR)
		s1_sth.s_groups[item_no].s_status[0] = t_char;
	else
		strcpy(s1_sth.s_groups[item_no].s_status, "ACT");


	return(NOERROR);
}
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Display message and get the option */
static
GetOpt( msg, option )
       char *msg;
       char *option;
{
        int	j,  i;
	
        strcpy( s1_sth.s_mesg, msg );
	sr.nextfld = MESG_FLD1; 
	fomwf((char *)&s1_sth);

	for ( ; ; ) {
		s1_sth.s_opt[0] = LV_CHAR ;
		if ( ReadFields((char*)&s1_sth, OPT_FLD1, OPT_FLD1, Validate,
						WindowHelp, 0) < 0 ) 
			return(-1);

                j = strlen(option);
		for ( i = 0; i < j; i++ ) 
			if ( s1_sth.s_opt[0] == option[i] )
                           break;
                if(i != j) break ;
                fomer(INVOPT);
	}
        s1_sth.s_mesg[0] = HV_CHAR ;
        s1_sth.s_opt[0] = HV_CHAR ;
	if ( WriteFields((char*)&s1_sth,MESG_FLD1,OPT_FLD1) < 0 ) 
		return(-1);
 
        return( (int)(option[i])) ;
}
/*-----------------------------------------------------------------------*/
/* Allow all lines on screen to be edited				 */
/*-----------------------------------------------------------------------*/
static
ChangeScreen()
{
	int 	retval;
	
	scpy((char*)&tmp1_sth, (char*)&s1_sth, sizeof(s1_sth));
	SetDupBuffers(DDESC_FLD, PD5_FLD, 1);

        strcpy(s1_sth.s_mesg, ITMMSG);
	if(WriteFields((char*)&s1_sth, MESG_FLD1, MESG_FLD1)<0) 
		return(-1);

	MoveLows();

	retval = ReadFields((char*)&s1_sth, DDESC_FLD,PD5_FLD,
					Validate, WindowHelp, 1);
	if(retval < 0) return(-1);
	if(retval == RET_USER_ESC){
			CopyBack((char*)&s1_sth,(char*)&tmp1_sth,
					sr.curfld,PD5_FLD);
	}

	s1_sth.s_mesg[0] = HV_CHAR;
	sr.nextfld = MESG_FLD1; 
	fomwf((char *)&s1_sth);

	return(retval);
}
/*-----------------------------------------------------------------------
Changing fields. Accept fld to be changed and read that fld 
-----------------------------------------------------------------------*/
ChangeFields()
{
     	int	retval, i;
	short	st_fld, end_fld; 

     	for(;;){
		scpy((char*)&tmp1_sth, (char*)&s1_sth, sizeof(s1_sth));
		SetDupBuffers(DDESC_FLD, PD5_FLD, 1);

		strcpy(s1_sth.s_mesg, FLDPROMPT);
		DispMesgFld((char*)&s1_sth);

		s1_sth.s_field = LV_SHORT;
		retval = ReadFields((char*)&s1_sth, CHG_FLD1, CHG_FLD1,
				Validate, WindowHelp, 1);
		if (retval < 0) return(-1);

		if (retval == RET_USER_ESC) break;  /* User enters ESC-F */
       		if (s1_sth.s_field == 0 ) break;   /* Finished changing */
 
		s1_sth.s_mesg[0] = HV_CHAR ;
		DispMesgFld((char*)&s1_sth);

		switch(s1_sth.s_field){

		case 1:
			s1_sth.s_desc[0] = LV_CHAR;
			st_fld = end_fld = DDESC_FLD;
			break;

		case 2:
			if(strcmp(param.pr_prov,"NB")==0){
				s1_sth.s_secode[0] = LV_CHAR;
				st_fld = end_fld = SECODE_FLD;
				break;
			}
			continue;

		case 3:
			s1_sth.s_t4[0] = LV_CHAR;
			st_fld = end_fld = T4_FLD;
			break;
	
		case 4:
			s1_sth.s_earn = LV_DOUBLE;
			st_fld = end_fld = EARN_FLD;
			break;

		case 5:
			s1_sth.s_cont = LV_DOUBLE;
			st_fld = end_fld = CONT_FLD;
			break;

		case 6:
			for(i=0;i<NO_PDS;i++){
				s1_sth.s_monpd[i].dmp[0] = LV_CHAR;
			}
			st_fld = PD1_FLD;
			end_fld = PD5_FLD;
			break;

		default:

			fomen(BADFIELD);

		} /* end switch */	

		retval = ReadFields((char*)&s1_sth, st_fld, end_fld,
					Validate, WindowHelp, 1);
		if(retval < 0) return(-1);

		if(retval == RET_USER_ESC){
			CopyBack((char *)&s1_sth,(char*)&tmp1_sth,st_fld,
				end_fld);
		}
	
	} /* end for loop */

     	s1_sth.s_field = HV_SHORT ;
	if(WriteFields((char*)&s1_sth,CHG_FLD1,CHG_FLD1) < 0) return(-1);

     	s1_sth.s_mesg[0] = HV_CHAR ;
	DispMesgFld((char*)&s1_sth);

     	return(NOERROR);

}	/* ChangeFields() */
/*-----------------------------------------------------------------------*/
ChangeItem()
{
	int retval;
	int day;

	/* make copy screen every time field changed in case user */
	/* presses ESC-F */
	scpy((char*)&tmp1_sth, (char*)&s1_sth, sizeof(s1_sth));

	SetDupBuffers(ITEM_ST_FLD, OPT_FLD1 - 200, 2);

	/* Get The Field to Be Modified */
#ifdef	ENGLISH
	strcpy(s1_sth.s_mesg,"Enter RETURN to Terminate Edit");
#else
	strcpy(s1_sth.s_mesg,"Appuyer sur RETURN pour terminer l'ajustement");
#endif
	DispMesgFld((char *)&s1_sth); ;
        
     	for (; ;) {
		s1_sth.s_field = LV_SHORT;
		retval = ReadFields((char *)&s1_sth,CHG_FLD1,CHG_FLD1,
			(int (*)())NULL,(int (*)())NULL, 1);

		if (retval < 0) return(-1);
		if (retval == RET_USER_ESC) break;	/* User enter ESC-F */

       		if (s1_sth.s_field == 0 ) break;  /* Finished changing fields */
		if (s1_sth.s_field > CurPage->NoItems) {
			fomer("Line Number out of Range");
			continue;
		}

		if(strcmp(CurPage->Items[s1_sth.s_field-1].s_status,INACTIVE)==0){
			fomer("Item Has a Deleted Status Cannot Edit");
			continue;
		}
		retval = ReadItem(s1_sth.s_field - 1,UPDATE);
		if(retval != NOERROR && retval != RET_USER_ESC) {
			return(retval);
		}

		if(retval == RET_USER_ESC)
			continue;

		/* make copy screen every time field changed in case user */
		/* presses ESC-F */
		scpy((char*)&tmp1_sth, (char*)&s1_sth, sizeof(s1_sth));

		scpy((char*)&(CurPage->Items[s1_sth.s_field -1]), 
		     (char*)&(s1_sth.s_groups[s1_sth.s_field -1]),sizeof(S1_item)) ;

		if(s1_sth.s_ftn[0] == CHGREC) {
		  if(CurPage->I_Status[s1_sth.s_field-1][0] != ADD)
			CurPage->I_Status[s1_sth.s_field-1][0] = CHGREC;
		}
	}

     	s1_sth.s_field = HV_SHORT ;
	if ( WriteFields((char *)&s1_sth,CHG_FLD1, CHG_FLD1) < 0 ) return(-1);

     	s1_sth.s_mesg[0] = HV_CHAR ;
     	DispMesgFld((char *)&s1_sth);

	if(SetDupBuffers(ITEM_ST_FLD, OPT_FLD1 - 200, 0)<0) return(PROFOM_ERR);

	return(NOERROR);
}	/* ChangeFields() */
/*------------------------------------------------------------*/
/* Read details of given item# */
/*------------------------------------------------------------*/
static int
ReadItem(item_no,mode)
int	item_no ;
int	mode ;
{
	int	i;
	int	st_fld ;
	int	end_fld ;

	if(mode == ADD) {
		SetDupBuffers(ITEM_ST_FLD, OPT_FLD1 - 200,0);
		s1_sth.s_groups[item_no].s_group[0] = LV_CHAR;
	}
	else {
		SetDupBuffers(ITEM_ST_FLD, OPT_FLD1 - 200,1);
	
	}

#ifdef ENGLISH
	strcpy(s1_sth.s_mesg,"Press ESC-F to Terminate");
#else
	strcpy(s1_sth.s_mesg,"Appuyer sur ESC-F pour terminer");
#endif
	DispMesgFld((char *)&s1_sth);

	st_fld  = ITEM_ST_FLD ;
	end_fld  = ITEM_END_FLD ;

	s1_sth.s_groups[item_no].s_gdesc[0] = LV_CHAR;
	s1_sth.s_groups[item_no].s_fp[0] = LV_CHAR;
	s1_sth.s_groups[item_no].s_amount = LV_DOUBLE;
	s1_sth.s_groups[item_no].s_eeshare = LV_SHORT;
	s1_sth.s_groups[item_no].s_ershare = LV_SHORT;

	i = ReadFields((char *)&s1_sth,st_fld,end_fld,Validate,WindowHelp,1) ;
	if(PROFOM_ERR == i || DBH_ERR == i) return(i) ;
	if(RET_USER_ESC == i) {
		if(mode == ADD) {
			ClearItemLines(HV_CHAR,HV_DOUBLE,HV_SHORT,item_no);
			WriteFields((char *)&s1_sth,st_fld,end_fld);
			return(RET_USER_ESC);
		}
		/* When user gives ESC-F while changing fields, assumption is
		* he completed his changes, and remaining fields are same as
		* old. But, at this point STH will be having low values in the 
		* remaining fields. So move the old values form the linked list.
		*/

		i = CopyBack((char *)&s1_sth,(char *)&tmp1_sth,sr.curfld, OPT_FLD1);
		if(i == PROFOM_ERR) return(i);

		return(RET_USER_ESC) ;
	}

	return(NOERROR) ;
}	/* ReadItem() */
/*------------------------------------------------------------------------*/
static int
ChangeStatus(status)
int	status;
{
	int	retval;
	int	st_fld, end_fld;

	/* Get The Field to Be Modified */
#ifdef	ENGLISH
	strcpy(s1_sth.s_mesg,"Enter RETURN to Terminate Edit");
#else
	strcpy(s1_sth.s_mesg,"Appuyer sur RETURN pour terminer l'ajustement");
#endif
	DispMesgFld((char *)&s1_sth); ;
        
     	for (; ;) {
		s1_sth.s_field = LV_SHORT;
		retval = ReadFields((char *)&s1_sth,CHG_FLD1,CHG_FLD1,
			(int (*)())NULL,(int (*)())NULL, 1);

		if (retval < 0) return(-1);
		if (retval == RET_USER_ESC) break;	/* User enter ESC-F */

       		if (s1_sth.s_field == 0 ) break;  /* Finished changing fields */

	
		st_fld  = ITEM_ST_FLD + (STEP * (s1_sth.s_field-1)) + STATUS_FLD;
		end_fld  = ITEM_ST_FLD + (STEP * (s1_sth.s_field-1))+STATUS_FLD;

		if(status == DELITEM) {
			if(strcmp(s1_sth.s_groups[s1_sth.s_field-1].s_status,
			   INACTIVE)==0) {
				fomer("Item is Already Deleted");
			}
			else {
				strcpy(s1_sth.s_groups[s1_sth.s_field-1].s_status,
			   		INACTIVE);
			}
		}
		else {
			if(strcmp(s1_sth.s_groups[s1_sth.s_field-1].s_status,
			   ACTIVE)==0) {
				fomer("Item is Already Active");
			}
			else {
				strcpy(s1_sth.s_groups[s1_sth.s_field-1].s_status,
			   		ACTIVE);
			}
		}
		/* Update Linked List */
		scpy((char*)&(CurPage->Items[s1_sth.s_field -1]), 
		     (char*)&(s1_sth.s_groups[s1_sth.s_field -1]),sizeof(S1_item)) ;
		
		ret(WriteFields((char *)&s1_sth,st_fld,end_fld));
	}
     	s1_sth.s_field = HV_SHORT ;
	if ( WriteFields((char *)&s1_sth,CHG_FLD1, CHG_FLD1) < 0 ) return(-1);

	return(NOERROR);
}
/*-----------------------------------------------------------------------*/
/* read second line of options						 */
GetNextOpt()
{
	int retval;

	for( ; ; ) {

#ifdef	ENGLISH
		retval = GetOpt("A(dd items), I(tem edit), D(elete), R(eactivate), N(ext), P(rev), E(nd)", "AIDRNPE");
#else	
		retval = GetOpt("A(dd items), I(tem edit), D(elete), R(eactivate), N(ext), P(rev), E(nd)", "AIDRNPE");
#endif

		if(retval == END) break;

		switch(retval) {	/* process the option */
		case	ADDITEM:
			retval = AddItems();
			if(retval < 0) return(retval);
			break;

		case	ITEMEDIT:
			retval = ChangeItem();
			if(retval < 0) return(retval);
			break;

		case	DELITEM:
		case	REACTIVATE:
			retval = ChangeStatus(retval);
			if(retval < 0) return(retval);
			break;

		case NEXT:	/* Display the next page of items */
		  if(CurPage == CurLast || CurLast == NULL) {
#ifdef ENGLISH
			fomer("No More Pages....");
#else
			fomer("Plus de pages....");
#endif
			continue;
		  }
		  CurPage = CurPage->NextPage ;
		  retval = ShowItems(CurPage);
		  break;
		
		case PREV:	/* Display the prev page of items */
		  if(CurLast == NULL || CurPage == FirstPage) {
#ifdef ENGLISH
			fomer("No More Pages....");
#else
			fomer("Plus de pages....");
#endif
			continue;
		  }
		  CurPage = CurPage->PrevPage ;
		  retval = ShowItems(CurPage);
		  break;

		default:
			break;
		}
	}

	return(NOERROR);
}
/*------------------------------------------------------------*/
/* Read Item Details from the User */

AddItems()
{
	int	i, err ;

	/* If the last node of po is Partial filled then Show Page */
	if(CurLast != NULL && CurLast->NoItems < PAGESIZE1 ) {
		ret( ShowItems(CurLast) ) ;
		i = CurLast->NoItems ;
		CurPage = CurLast ;
	}
	else {
		/* Calculate the page# */
		if(CurLast != NULL) {
			i = PAGESIZE1 ;
			CurPage = CurLast ;
		}
		else {
			s1_sth.s_page = 1 ;
			ret( WriteFields((char *)&s1_sth,PAGE_FLD,PAGE_FLD+100));
			i = 0 ;
		}
	}

	for( ; ; ) {
		if( PAGESIZE1 == i) {	/* Page Full */

			/* move High Values to All items exept First one */
			for(i--;i > 0; i--)
				ClearItemLines(HV_CHAR,HV_DOUBLE,HV_SHORT,i);

			/* Calculate the page# */
			s1_sth.s_page = CurLast->Pageno + 1 ;

			ret( WriteFields((char *)&s1_sth,PAGE_FLD, 
				(OPT_FLD1 - 200)) ) ;

			i = 0 ;
		}

		err = ReadItem(i,ADD) ;		/* Read Each Item Line */
		if(PROFOM_ERR == err || DBH_ERR == err) return(err) ;
		if(NOERROR != err) break ;	/* ESC-F */

		if(0 == i)	/* First Item in the Page */
			if((err = MakeFreshPage()) < 0) return(err) ;
		
		/* Copy the Item to List */
		scpy((char*)&(CurPage->Items[i]), (char*)&(s1_sth.s_groups[i]),
			sizeof(S1_item)) ;

		CurPage->I_Status[i][0] = ADDITEM;

		i++ ;

		CurPage->NoItems = i;
	}
	if(i == 0) 
		if((err=ShowItems(CurPage))<0) return(err) ;

	return(NOERROR) ;
}	/* AddItems() */
/*-----------------------------------------------------------------------*/

static
FreeList()	
{
	int 	i;

	/* Free the linked list for the end */
	for( ;LastPage != FirstPage ; ) {
		LastPage = LastPage->PrevPage;
		free((char *)LastPage->NextPage);
		LastPage->NextPage = NULL;
	}
	if(FirstPage != NULL) {
		free((char *)FirstPage);
	}

	FirstPage = LastPage = NULL;

	return(NOERROR);
}
/*------------------------------------------------------------*/
static int
ShowScreen()
{
	int	err ;

	err = ShowHdr();
	if(err < 0) {
		return(err);
	}

	/* Get area records i.e. Build list */
	err = BuildList();
	if(err < 0 && err != EFL) {
		return(err);
	}

	err = ShowItems(FirstPage);
	if(err < 0) {
		return(err);
	}
	
	return(NOERROR);
}
/*------------------------------------------------------------*/
static
BuildList()
{
	int retval;
	int i;
	CurLast = CurPage = NULL;
	i = 0;

	FreeList();

	strcpy(dedgrp.dg_code, s1_sth.s_dedcd);
	strcpy(dedgrp.dg_pp_code, s1_sth.s_paypd);
	dedgrp.dg_group[0] = LV_CHAR;
	flg_reset(DED_GRP);

	for( ; ; ) {
		retval = get_n_ded_grp(&dedgrp,BROWSE,0,FORWARD,e_mesg);
		if(retval < 0) {
			if(retval == EFL) break;
			DispError((char *)&s1_sth,e_mesg);
			return(retval);
		}

		if(strcmp(dedgrp.dg_code,s1_sth.s_dedcd)!=0 || 
		   strcmp(dedgrp.dg_pp_code,s1_sth.s_paypd)!=0) { 
			break;
		}

		if(PAGESIZE1 == i) i = 0;
		if(i == 0) {
			if((retval = MakeFreshPage()) < 0) return(retval);
		}
		strcpy(CurPage->Items[i].s_group,dedgrp.dg_group);
		strcpy(CurPage->Items[i].s_gdesc,dedgrp.dg_desc);
		strcpy(CurPage->Items[i].s_fp,dedgrp.dg_amt_flag);
		CurPage->Items[i].s_amount = dedgrp.dg_amount;
		CurPage->Items[i].s_eeshare = 100 - dedgrp.dg_employer_sh;
		CurPage->Items[i].s_ershare = dedgrp.dg_employer_sh;

		strcpy(CurPage->Items[i].s_status,ACTIVE);
		CurPage->I_Status[i][0] = ' ';
		CurPage->NoItems++;
		i++;
	}
	seq_over(DED_GRP);

	if(CurLast != NULL) {
		CurPage = FirstPage;
	}

	if(retval == EFL) return(retval);
	return(NOERROR);
}
/*------------------------------------------------------------*/
static int
ShowHdr() 
{
	int	i, retval;
	
	strcpy(s1_sth.s_dedcd,deduct.dd_code);
	strcpy(s1_sth.s_secode,deduct.dd_second);
	strcpy(s1_sth.s_paypd,deduct.dd_pp_code);
	strcpy(s1_sth.s_desc,deduct.dd_desc);
	strcpy(s1_sth.s_t4,deduct.dd_t4_fld);
	s1_sth.s_earn = deduct.dd_min_earn;
	s1_sth.s_cont = deduct.dd_max_contr;

	s2_sth.s_fund = deduct.dd_fund;
	strcpy(s2_sth.s_account,deduct.dd_lia_acct);

/**** There are no t4_codes or any place to add them ??? L.R
	strcpy(t4rec.t4_code,deduct.dd_t4_fld);
	retval = get_t4_rec(&t4rec,BROWSE,0,e_mesg);
	if(retval < 0){
		DispError((char *)&s1_sth,e_mesg);
		if(retval != UNDEF) return(retval);
	}
******/

	for(i=0;i<NO_PDS;i++){
		strcpy(s1_sth.s_monpd[i].dmp,deduct.dd_ded_pp[i]);
	}

	for(i=0;i<MAX_KEYS;i++){
		s2_sth.s_acckey[i] = deduct.dd_exp_acct[i];
	}

	WriteFields((char *)&s1_sth,DEDCD_FLD1,PD5_FLD);

	return(NOERROR);
}
/*------------------------------------------------------------*/
/* Show all the items on the current page 		      */
static int
ShowItems(pageptr)
Page	*pageptr ;
{
	int	i ;


	if(pageptr != NULL) {
		/* Copy the items to screen */
		scpy((char*)s1_sth.s_groups, (char*)pageptr->Items,
			(pageptr->NoItems * sizeof(S1_item)) );

		s1_sth.s_page   = pageptr->Pageno ;
		i = pageptr->NoItems ;
	}
	else {
		s1_sth.s_page = HV_SHORT ;
		i = 0 ;
	}

	/* Move High Values to remaining Items */
	for( ; i < PAGESIZE1 ; i++ )
		ClearItemLines(HV_CHAR,HV_DOUBLE,HV_SHORT,i);

	ret( WriteFields((char *)&s1_sth, PAGE_FLD, (OPT_FLD1 - 200)) );

	return(NOERROR) ;
}	/* ShowItems() */
/*-----------------------------------------------------------------------*/
/*	Get the next node in linked list to add invoice items. If the
*	(Cur. invc last page) = (Last Page in linked List) or no
*	nodes in list, allocate node and add to linked list
*/
static int
MakeFreshPage()
{
	Page	*tempptr ;

	/* If, no node is allocated yet or Current invoice used all the nodes,
	   then allocate new node */

	if( LastPage == NULL || CurLast == LastPage ){
		tempptr= (Page *)malloc((unsigned)sizeof(Page)) ;

		if( tempptr == NULL ){
			DispError((char*)&s1_sth,"Memory Allocation Error");
			return(ERROR);
		}
		tempptr->NextPage = NULL ;

		if( LastPage == NULL ){	/* No node is allocated Yet */
			tempptr->PrevPage = NULL ;
			tempptr->Pageno = 1 ;
			FirstPage = tempptr ;
		}
		else {				/* Not a first node in list */
			tempptr->Pageno = LastPage->Pageno + 1 ;
			LastPage->NextPage = tempptr ;
			tempptr->PrevPage = LastPage ;
		}
		LastPage = tempptr ;
	}

	if(CurLast == NULL)
		CurLast = FirstPage ;
	else
		CurLast = CurLast->NextPage ;

	CurLast->NoItems = 0 ;
	CurPage = CurLast ;

	return(NOERROR);
}	/* MakeFreshPage() */
/*-----------------------------------------------------------------------*/
MoveLows()
{
	int i;

	if(strcmp(param.pr_prov,"NB")==0){
		s1_sth.s_secode[0] = LV_CHAR;
	}
	else{
		s1_sth.s_secode[0] = HV_CHAR;
	}

	s1_sth.s_desc[0] = LV_CHAR;
	s1_sth.s_t4[0] = LV_CHAR;
	s1_sth.s_earn = LV_DOUBLE;

	s1_sth.s_cont = LV_DOUBLE;
	for(i=0;i<NO_PDS;i++){
		s1_sth.s_monpd[i].dmp[0] = LV_CHAR;
	}
	return(NOERROR);
}
/*----------------------------------------------------------------*/
/* Set Duplication buffers for fields 				  */
static int
SetDupBuffers( firstfld, lastfld, value )
int	firstfld, lastfld;	/* field numbers range */
int	value;			/* ENABLE or DISABLE */
{
	int i ;

	for( i=firstfld; i<=lastfld; i+=100 )
		fomca1( i, 19, value);
	if( value==0 )
		return(0);
	sr.nextfld = firstfld;
	sr.endfld = lastfld;
	fomud( (char *)&s1_sth );
	ret( err_chk(&sr) );

	return( 0 );
}
/****************************************************************************/
UpdateAcct() 
{
	int	i,	retval;
	int	mode;
	int	ch_mod;

	if(s1_sth.s_ftn[0] == ADDREC)
		mode = ADD;

	if(s1_sth.s_ftn[0] == CHGREC)
		mode = UPDATE;

	if(s1_sth.s_ftn[0] == DELREC)
		mode = P_DEL;

	/*  If changing from account to keys the Payroll GL account record 
	    set up for school zero must be deleted */
	if(mode == UPDATE) {
		gl_acct.gl_fund = s2_sth.s_fund;
		gl_acct.gl_cc = 0;
		gl_acct.gl_type[0] = 'D';
		strcpy(gl_acct.gl_earn,s1_sth.s_dedcd);
		gl_acct.gl_class[0] = '\0';
		retval = get_glacct(&gl_acct,UPDATE,0,e_mesg);
		if(retval >= 0) {
			retval = put_glacct(&gl_acct,P_DEL,e_mesg);
			if(retval < 0) {
				DispError((char *)&s2_sth,e_mesg);
				roll_back(e_mesg);
				return(retval);
			}
		}
	}
	school.sc_numb = 0;
	flg_reset(SCHOOL);

	for( ;; ) {   /* For each Cost Center */
		retval = get_n_sch(&school,BROWSE,0,FORWARD,e_mesg);
		if(retval < 0 ) {
			if(retval == EFL) break;
			DispError((char *)&s2_sth,e_mesg);
			return(retval);
		}
		gl_acct.gl_fund = s2_sth.s_fund;
		gl_acct.gl_cc = school.sc_numb;
		gl_acct.gl_type[0] = 'D';
		strcpy(gl_acct.gl_earn,s1_sth.s_dedcd);
		gl_acct.gl_class[0] = '\0';
		if(mode == UPDATE || mode == P_DEL) {
			retval = get_glacct(&gl_acct,UPDATE,0,e_mesg);
			if(retval < 0)  {
				if(retval == EFL || retval == UNDEF) {
					if(mode == UPDATE)
						ch_mod = ADD;
					else
						continue;
				}
				else {
					DispError((char *)&s2_sth,e_mesg);
					return(retval);
				}
			}
			else
				ch_mod = mode;
		}
		retval = 0;
		if( mode == UPDATE ) {
			retval = NO_CHANGE;
			for(i=0 ; i < NO_KEYS ; i++) {
				if(s2_sth.s_acckey[i] != deduct.dd_exp_acct[i]) {
					retval = GetNewAcct();
					break;
				}
			}
		}
		else  {
			if( mode == ADD ) {
				retval = GetNewAcct();
			}
			ch_mod = mode;
		}
					/* No Change in Keys */
		if(retval == NO_CHANGE) continue; 

		if(retval == NOTFOUND) 

			continue; 	/* No G/L Acct for the CC# */

		gl_acct.gl_fund = s2_sth.s_fund;
		gl_acct.gl_cc = school.sc_numb;
		gl_acct.gl_type[0] = 'D';
		strcpy(gl_acct.gl_earn,s1_sth.s_dedcd);
		gl_acct.gl_class[0] = '\0';
		strcpy(gl_acct.gl_acct, glmast.accno);
			
		retval = put_glacct(&gl_acct,ch_mod,e_mesg);
		if(retval < 0) {
			DispError((char *)&s2_sth,e_mesg);
			roll_back(e_mesg);
			return(retval);
		}
	}
	return(NOERROR);
}	/* UpdateAcct */
/*---------------------------------------------------------------*/
/* Read Sequentially through the glmast file to get G/L Account   */
GetNewAcct()
{
	int	account_flg;		/* Found/NotFound G/L Account */
	int	retval, i;

	glmast.reccod = 99;
	glmast.funds = s2_sth.s_fund;
	glmast.sect = 3;
	strcpy(glmast.accno, "          13100000");
	flg_reset(GLMAST);

	for( ;; ) {
		retval = get_n_gl(&glmast,BROWSE,2,FORWARD,e_mesg);
		if(retval < 0) {
			if(retval == EFL) { 
				return(NOTFOUND);
			}
			DispError((char *)&s2_sth,e_mesg);
			return(retval);
		}
		if( glmast.funds != s2_sth.s_fund ) {
			return(NOTFOUND);
		}
		if(strcmp(glmast.accno, "          31026099") > 0)
			return(NOERROR);

		account_flg = FOUND;
		for(i=0;i < NO_KEYS; i++) {
			if(param.pr_deduct[i][0] == 'Y') {
				if(glmast.keys[i] != s2_sth.s_acckey[i]) {
					account_flg = NOTFOUND;
					break;
				}
			}
		}
		if(account_flg == NOTFOUND) continue;
		if(glmast.keys[param.pr_cost-1] != school.sc_numb) 
			continue;
		
		return(FOUND);
	}
}
/*-------------------- E n d   O f   P r o g r a m ---------------------*/
