/*----------------------------------------------------------------------
Source Name: pay_par.c  
System     : Personnel
Module     : Payroll/Personel
Created On : Jan 11, 1992
Created By : Eugene Roy

DESCRIPTION:

	This program alows the user to edit the Payroll Parameter File.

MODIFICATIONS:        

Programmer     	YY/MM/DD       	Description of modification
~~~~~~~~~~     	~~~~~~~~       	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
------------------------------------------------------------------------*/

#define	MAIN	
#define	MAINFL		PAY_PARAM

#include <stdio.h>
#include <ctype.h>
#include <cfomstrc.h>
#include <repdef.h>
#include <bfs_defs.h>
#include <bfs_pp.h>
#include <bfs_com.h>
#include <pp_msgs.h>
#include <pay_par.h>

#define MOD_DATE	"11-JAN-92"
#define SYSTEM   	"PAYROLL/PERSONEL"

char	txt_buff[80];	
char	txt_buff2[80];	

static	int	Validate();
static	int	WindowHelp();

/*-------------------------------------------------------------------*/

main(argc,argv)
int argc;
char *argv[];
{
	int 	retval;

	retval = Initialize(argc,argv);	/* Initialization routine */

	if (retval == NOERROR) retval = Process();

	CloseRtn();			/* return to menu */
	if (retval != NOERROR) exit(-1);
	exit(0);
}

/*-------------------------------------------------------------------*/
/* Initialize PROFOM */

Initialize(argc, argv)
int	argc ;
char	*argv[] ;
{
	int	err ;

	/*
	*	Initialize DBH Environment
	*/
	strncpy(SYS_NAME,SYSTEM,50);	/* Sub system name */
	strncpy(CHNG_DATE,MOD_DATE,10);	/* Modification Date */

	proc_switch(argc, argv, MAINFL) ; 	/* Process Switches */

	/*
	*	Initialize PROFOM & Screen
	*/
	strcpy(sr.termnm,terminal);	/* Copy Terminal Name */
	fomin(&sr);
	ret(err_chk(&sr)) ;		/* Check for PROFOM Error */
	fomcf(1,1);			/* Enable Snap screen option */

	err = InitScreen() ;		/* Initialize Screen */
	if(NOERROR != err) return(err) ;

	return(NOERROR) ;
}	/* Initialize() */

/*--------------------------------------------------------------------------*/
/* Close nessary files and environment before exiting program               */

CloseRtn() 
{

	/* Set terminal back to normal mode from PROFOM */
	fomcs();
	fomrt();

	close_dbh();	/* Close files */

	return(NOERROR);
}	/* CloseRtn() */
/*----------------------------------------------------------------*/
/* Initialize screen before going to process options */

InitScreen()
{
	int	err ;

	/* move screen name to Profom status structure */
	strcpy(sr.scrnam,NFM_PATH);
	strcat(sr.scrnam,SCRNM1) ;

	strcpy(s1_sth.s_pgname,PRGNM);

	s1_sth.s_sysdate = get_date();	/* get Today's Date in YYYYMMDD format*/
	s1_sth.s_field = HV_SHORT ;
	s1_sth.s_mesg[0] = HV_CHAR;
	s1_sth.s_opt[0] = HV_CHAR ;

	/* Move High Values to data fields and Display the screen */

	err = ClearScreen();
	if(NOERROR != err) return(err) ;

	return(NOERROR) ;
}	/* InitScreen() */
/*-------------------------------------------------------------------*/
/* Get Option from user and call corresponding function */
static	int
Process()
{
	int	err;

	first_time1 = 1;
	first_time2 = 1;
	for( ; ; ){
		/* Get the Fn: from the user */
		if((err = ReadFunction()) != NOERROR) return(err) ;

		err = ProcFunction() ;	/* Process Fn */

		if(QUIT == err)		return(NOERROR) ;	/* Exit */
		if(NOACCESS == err) {
			fomen(e_mesg);
			get();
		}
		if(PROFOM_ERR == err)	return(PROFOM_ERR);  /* PROFOM ERROR */
		if(DBH_ERR == err) {
			DispError((char*)&s1_sth, e_mesg);
#ifdef	ENGLISH
			sprintf(e_mesg,"%s %d Dberror: %d Errno: %d",
				"System Error... Iserror:",
				iserror, dberror, errno);
#else
			sprintf(e_mesg,"%s %d Dberror: %d Errno: %d",
				"Erreur du systeme... Iserror:",
				iserror, dberror, errno);
#endif
			DispError((char*)&s1_sth, e_mesg);
			return(DBH_ERR);	/* DBH ERROR */
		}
	}      /*   end of the for( ; ; )       */
}	/* Process() */
/*----------------------------------------------------------------*/
/* Display the Function (Fn:) options and get the option from the user */

ReadFunction()
{
	/* Display Fn: options */
#ifdef ENGLISH
	fomer("C(hange), I(nquire), E(xit)");
#else
	fomer("C(hanger), I(nterroger), F(in)");
#endif
	/* Read Fn: field to get the option */
	sr.nextfld = FN_FLD1 ;
	fomrf( (char *)&s1_sth );
	ret(err_chk(&sr));	/* Check for PROFOM error */

	return(NOERROR) ;
}	/* ReadFunction() */

/*----------------------------------------------------------------*/
/* Process the user selected Fn: option */

ProcFunction()
{
	int retval;

	switch (s1_sth.s_fn[0]) {
	case CHANGE  :			/* CHANGE */
		CHKACC(retval,UPDATE,e_mesg);
		return( Change() ) ;
	case INQUIRE  :			/* CHANGE */
		CHKACC(retval,UPDATE,e_mesg);
		return( Inquire() ) ;
	case EXITOPT  :
		return(QUIT);
	default   : 
		return(NOERROR);
	}  /*   end of the switch statement */

	return(retval);
}	/* ProcFunction() */
/*-----------------------------------------------------------------------*/
/* Change. Students Study halls and update the files if a day/semester   */
/* is changed to NO delete record.			  		 */
/*-----------------------------------------------------------------------*/
Change()
{
	int	err ;

	err = SelectRecord() ;
	if(NOERROR != err) return(err) ;

	for( ; ; ) {
		err = Confirm() ;
		if(err != YES) {
			roll_back(e_mesg);
			break;
		}

		err = WriteRecords(UPDATE) ;
		if(err==NOERROR) break;
		if(err==LOCKED) {
			roll_back(e_mesg) ;
			continue;
		}
		if (err < 0) {
			roll_back(e_mesg);
			return(err) ;
		}
	}

	ret( WriteFields((char *)&s1_sth,TEACH_FT, (END_FLD1 - 200)) );

	return(NOERROR) ;
}	/* Change() */
/*-----------------------------------------------------------------------*/
/* Show Student Employee Demographic Date                                */
Inquire()
{
	int	err ;

	err = SelectRecord() ;
	if(NOERROR != err) return(err) ;

	err = Confirm() ;

	return(NOERROR) ;
}	/* Inquire() */
/*----------------------------------------------------------*/
/* Get the key and show the record */
SelectRecord()
{
	int	err ;

	err = get_pay_param(&pay_param,BROWSE,1,e_mesg);
	if(err < 0 && err != UNDEF)  {
		fomen(e_mesg);
		get();
		return(err);
	}

	if(err == UNDEF)
		InitFields(LV_CHAR,LV_LONG,LV_SHORT,LV_DOUBLE);
	else
		err = CopyToScreen();

	ret( WriteFields((char *)&s1_sth, TEACH_FT, END_FLD1-200) ) ;

	return(NOERROR);
}	/* SelectRecord() */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Copy the Screen fields to data record */
static int
CopyToScreen()
{

	strcpy(s1_sth.s_tp_ft, pay_param.pr_tp_ft_code);
	strcpy(s1_sth.s_pay_att, pay_param.pr_att_pay);
	strcpy(s1_sth.s_tp_pt, pay_param.pr_tp_pt_code); 
	strcpy(s1_sth.s_pay_tch, pay_param.pr_teach_pay);
	s1_sth.s_tw_units = pay_param.pr_tot_units;
	strcpy(s1_sth.s_sub, pay_param.pr_sb_code); 
	strcpy(s1_sth.s_area_def, pay_param.pr_area_def);

	s1_sth.s_retro = pay_param.pr_retro;	
	s1_sth.s_cs_date = pay_param.pr_cal_st_dt;
	strcpy(s1_sth.s_updt_gl, pay_param.pr_up_gl);
	s1_sth.s_ce_date = pay_param.pr_cal_end_dt;  

	s1_sth.s_dept = pay_param.pr_dept;
	s1_sth.s_fs_date = pay_param.pr_fisc_st_dt; 
	s1_sth.s_area = pay_param.pr_area;
	s1_sth.s_fe_date = pay_param.pr_fisc_end_dt;

	s1_sth.s_cc = pay_param.pr_cost;
	s1_sth.s_ss_date = pay_param.pr_schl_st_dt;
	strcpy(s1_sth.s_reg_cd, pay_param.pr_reg_earn);
	s1_sth.s_se_date = pay_param.pr_schl_end_dt;
	strcpy(s1_sth.s_retro_cd, pay_param.pr_retro_cd);
	s1_sth.s_st_mth = pay_param.pr_st_mth;

	strcpy(s1_sth.s_vac_cd, pay_param.pr_vac_earn);
	s1_sth.s_last_chq = pay_param.pr_last_chq;
	strcpy(s1_sth.s_prov, pay_param.pr_prov);
	s1_sth.s_last_ec = pay_param.pr_last_ec;

	strcpy(s1_sth.s_comm_pref, pay_param.pr_langu);
	s1_sth.s_10mth_st_dt = pay_param.pr_10mth_st_dt;
	strcpy(s1_sth.s_payper, pay_param.pr_payper);
	s1_sth.s_10mth_end_dt = pay_param.pr_10mth_end_dt;

	return(NOERROR);

}	/* Copy to Record */
/*-----------------------------------------------------------------------*/
/* Take the confirmation from user for the header part */

static
Confirm()
{
	int	err ;

	/* Options:
	   Add      - YALSNPC
	   Change   - YALSNPC
	   Delete   - YC
	*/

	for( ; ; ) {
	    switch(s1_sth.s_fn[0]) {
	    case  CHANGE :		/* Change */
#ifdef ENGLISH
		err = GetOption((char *)&s1_sth,
 "Y(es), S(creen Edit), L(ine edit), 2(nd screen), C(ancel)",
		"YSL2C");  	
		
#else
		err = GetOption((char *)&s1_sth,
 "O(ui), S(creen edit), L(ine edit), 2(eme ecran), A(nnuler)",
		"OSL2A"); 

#endif
		break ;
	    case  INQUIRE :		/* Change */
#ifdef ENGLISH
		err = GetOption((char *)&s1_sth,
		"Y(es), 2(nd screen), C(ancel)","Y2C");  
#else
		err = GetOption((char *)&s1_sth,
		"O(ui), 2(eme ecran), A(nnuler)","O2A"); 
#endif
		break ;
	    }
	    if(err == PROFOM_ERR) return(err) ;

	    switch(err) {
	    case  YES  :
		return(YES);
	    case  SCREENEDIT:
		err = ReadScreen(UPDATE);
		break;
	    case  LINEEDIT  :
		err = ChangeFields();
		break ;
	    case  SEC_SCR :
		strcpy(sr.scrnam,NFM_PATH);
		strcat(sr.scrnam,SCRNM2) ;

		err = GetScreen2();
		if(err == YES) return(err);
		if(err == CANCEL) return(err);
		break;

	    case  CANCEL :
#ifdef ENGLISH
		err = GetOption((char *)&s1_sth,
				"Confirm the Cancel (Y/N)?", "YN") ;
#else
		err = GetOption((char *)&s1_sth,
				"Confirmer l'annulation (O/N)?", "ON") ;
#endif
		if(err == YES) return(CANCEL) ;
		break ;
	    }	/* switch err */

	    if(err == PROFOM_ERR) return(err) ;
	    if(err == DBH_ERR) return(err) ;
	}	/* for(; ; ) */
}	/* Confirm() */
/*-----------------------------------------------------------------------*/ 
/* Check to see if record is to be added, changed or deleted .		 */
/*-----------------------------------------------------------------------*/ 
WriteRecords(mode)
int mode;
{
	int	i,retval;

	retval = get_pay_param(&pay_param,UPDATE,1,e_mesg);

	if(retval == UNDEF)
		mode = ADD;
	else
		mode = UPDATE;
	if(retval < 0 && retval != UNDEF) {
		fomer(e_mesg);
		return(retval);
	}
	
	/*  Fields	*/

	strcpy( pay_param.pr_tp_ft_code,s1_sth.s_tp_ft);
	strcpy( pay_param.pr_att_pay, s1_sth.s_pay_att);
	strcpy( pay_param.pr_tp_pt_code, s1_sth.s_tp_pt); 
	strcpy( pay_param.pr_teach_pay,s1_sth.s_pay_tch);
	pay_param.pr_tot_units = s1_sth.s_tw_units;
	strcpy( pay_param.pr_sb_code,s1_sth.s_sub); 
	strcpy( pay_param.pr_area_def, s1_sth.s_area_def);

	pay_param.pr_retro = s1_sth.s_retro ;	
	pay_param.pr_cal_st_dt = s1_sth.s_cs_date ;
	strcpy( pay_param.pr_up_gl,s1_sth.s_updt_gl);
	pay_param.pr_cal_end_dt = s1_sth.s_ce_date ;  

	pay_param.pr_dept = s1_sth.s_dept ;
	pay_param.pr_fisc_st_dt = s1_sth.s_fs_date ; 
	pay_param.pr_area = s1_sth.s_area ;
	pay_param.pr_fisc_end_dt = s1_sth.s_fe_date ;

	pay_param.pr_cost = s1_sth.s_cc ;
	pay_param.pr_schl_st_dt = s1_sth.s_ss_date ;
	strcpy(pay_param.pr_reg_earn, s1_sth.s_reg_cd );
	pay_param.pr_schl_end_dt = s1_sth.s_se_date ;
	strcpy(pay_param.pr_retro_cd, s1_sth.s_retro_cd);
	strcpy(pay_param.pr_vac_earn, s1_sth.s_vac_cd);  
	pay_param.pr_st_mth = s1_sth.s_st_mth;
	pay_param.pr_last_chq = s1_sth.s_last_chq;
	pay_param.pr_last_ec = s1_sth.s_last_ec ;

	strcpy(pay_param.pr_prov, s1_sth.s_prov );
	strcpy( pay_param.pr_langu, s1_sth.s_comm_pref);
	pay_param.pr_10mth_st_dt = s1_sth.s_10mth_st_dt;
	strcpy( pay_param.pr_payper, s1_sth.s_payper);
	pay_param.pr_10mth_end_dt = s1_sth.s_10mth_end_dt;

	if(first_time2 == 0){
		for(i= 0; i < 12; i++){
		 strcpy( pay_param.pr_cpp_exp[i],s2_sth.s_gl[0].s_acct_keys[i]);
		}
		for(i= 0; i < 12; i++){
		 strcpy( pay_param.pr_uic_exp[i],s2_sth.s_gl[1].s_acct_keys[i]);
		}
		for(i= 0; i < 12; i++){
		 strcpy( pay_param.pr_salary[i],s2_sth.s_gl[2].s_acct_keys[i]);
		}
		for(i= 0; i < 12; i++){
		 strcpy( pay_param.pr_deduct[i],s2_sth.s_gl[3].s_acct_keys[i]);
		}
		for(i= 0; i < 12; i++){
		 strcpy( pay_param.pr_reg_pen[i],s2_sth.s_gl[4].s_acct_keys[i]);
		}
		for(i= 0; i < 12; i++){
		 strcpy( pay_param.pr_teacher[i],s2_sth.s_gl[5].s_acct_keys[i]);
		}
		pay_param.pr_fund = s2_sth.s_fund ;
		strcpy(pay_param.pr_teach_gl, s2_sth.s_acct);

		for(i= 0; i < 12; i++){
			 pay_param.pr_keys[i] = s2_sth.s_keys[i];
		}
	}

	retval = put_pay_param(&pay_param,mode,1,e_mesg);
	if(retval < 0) {
		DispError((char *)&s1_sth,e_mesg);
		roll_back(e_mesg);
		return(retval);
	}

	if(mode != ADD) {
		retval = rite_audit((char*)&s1_sth,PAY_PARAM,mode,(char*)&pay_param,
			(char*)&pre_pay_param,e_mesg);
		if(retval==LOCKED) {
			DispError((char *)&s1_sth,e_mesg);
			roll_back(e_mesg) ;
			return(LOCKED) ;
		}
		if(retval < 0 ){
#ifdef	ENGLISH
			DispError((char *)&s1_sth,"ERROR in Saving Records"); 
#else
			DispError((char *)&s1_sth,
					"ERREUR en conservant les fiches");
#endif
			DispError((char *)&s1_sth,e_mesg);
			roll_back(e_mesg);
			return(retval);
		}
	}

	retval = commit(e_mesg) ;
	if(retval < 0) {
#ifdef ENGLISH
		DispError((char *)&s1_sth,"ERROR in Saving Records"); 
#else
		DispError((char *)&s1_sth,"ERREUR en conservant les fiches");
#endif
		DispError((char *)&s1_sth,e_mesg);
		roll_back(e_mesg);
		return(retval);
	}

	return(NOERROR);
}
/*-----------------------------------------------------------------------*/
/* Validate flds when PROFOM returns RET_VAL_CHK    			 */
static
Validate()	
{
	int	retval;

	switch(sr.curfld){


	case TEACH_FT:
		Right_Justify_Numeric(s1_sth.s_tp_ft,
			sizeof(s1_sth.s_tp_ft)-1);
		retval = Read_Position(s1_sth.s_tp_ft);
		if(retval<0) {
			fomer("Invalid Position Code");
			s1_sth.s_tp_ft[0] = LV_CHAR;
			return(ERROR);
		}
		break;

	case PAY_ATT:
		if(s1_sth.s_pay_att[0] != YES && s1_sth.s_pay_att[0] != NO) {
			fomen("Must be Y(es) or N(o)");
			s1_sth.s_pay_att[0] = LV_CHAR;
			return(ERROR);
		}
		break;
	case TEACH_PT:
		Right_Justify_Numeric(s1_sth.s_tp_pt,
			sizeof(s1_sth.s_tp_pt)-1);
		retval = Read_Position(s1_sth.s_tp_pt);
		if(retval<0) {
			fomer("Invalid Position Code");
			s1_sth.s_tp_pt[0] = LV_CHAR;
			return(ERROR);
		}
		break;
	case PAY_TEACH:
		if(s1_sth.s_pay_tch[0] != YES && s1_sth.s_pay_tch[0] != NO) {
			fomen("Must be Y(es) or N(o)");
			s1_sth.s_pay_tch[0] = LV_CHAR;
			return(ERROR);
		}
		break;

	case SUB_TEACH:
		Right_Justify_Numeric(s1_sth.s_sub,
			sizeof(s1_sth.s_sub)-1);
		retval = Read_Position(s1_sth.s_sub);
		if(retval<0) {
			fomer("Invalid Position Code");
			s1_sth.s_sub[0] = LV_CHAR;
			return(ERROR);
		}
		break;
	case AREA_DEF:
		if(s1_sth.s_area_def[0] != YES && s1_sth.s_area_def[0] != NO) {
			fomen("Must be Y(es) or N(o)");
			s1_sth.s_area_def[0] = LV_CHAR;
			return(ERROR);
		}
		break;
	case UPDT_GL:
		if(s1_sth.s_updt_gl[0] != YES && s1_sth.s_updt_gl[0] != NO) {
			fomen("Must be Y(es) or N(o)");
			s1_sth.s_updt_gl[0] = LV_CHAR;
			return(ERROR);
		}
		break;
	case RETRO_CD:
	  Right_Justify_Numeric(s1_sth.s_retro_cd,
			sizeof(s1_sth.s_retro_cd)-1);
	  strcpy(earnings.ea_code,s1_sth.s_retro_cd);
	  earnings.ea_date = s1_sth.s_sysdate;
	  flg_reset(EARN);

	  retval = get_n_earn(&earnings,BROWSE,0,BACKWARD,e_mesg);
	  if(retval < 0){
	 	if(retval == EFL ||
	 	  strcmp(earnings.ea_code,
			s1_sth.s_retro_cd) != 0)
			break;
		fomer("Earnings Code Does Not Exist - Please Re-enter");
		s1_sth.s_retro_cd[0] = LV_CHAR;
	    	return(ERROR);
	  }
	  break;
	case VAC_CD:
	  Right_Justify_Numeric(s1_sth.s_vac_cd,
			sizeof(s1_sth.s_vac_cd)-1);
	  strcpy(earnings.ea_code,s1_sth.s_vac_cd);
	  earnings.ea_date = s1_sth.s_sysdate;
	  flg_reset(EARN);

	  retval = get_n_earn(&earnings,BROWSE,0,BACKWARD,e_mesg);
	  if(retval < 0){
	 	if(retval == EFL ||
	 	  strcmp(earnings.ea_code,
			s1_sth.s_vac_cd) != 0)
			break;
		fomer("Earnings Code Does Not Exist - Please Re-enter");
		s1_sth.s_vac_cd[0] = LV_CHAR;
	    	return(ERROR);
	  }
	  break;
	case REG_CD:
	  Right_Justify_Numeric(s1_sth.s_reg_cd,
			sizeof(s1_sth.s_reg_cd)-1);
	  strcpy(earnings.ea_code,s1_sth.s_reg_cd);
	  earnings.ea_date = s1_sth.s_sysdate;
	  flg_reset(EARN);

	  retval = get_n_earn(&earnings,BROWSE,0,BACKWARD,e_mesg);
	  if(retval < 0){
	 	if(retval == EFL ||
	 	  strcmp(earnings.ea_code,
			s1_sth.s_reg_cd) != 0)
			break;
		fomer("Earnings Code Does Not Exist - Please Re-enter");
		s1_sth.s_reg_cd[0] = LV_CHAR;
	    	return(ERROR);
	  }
	  break;
	case PROV:
		if((strcmp(s1_sth.s_prov,"NB") != 0) &&
		   (strcmp(s1_sth.s_prov,"NS") != 0) &&
		   (strcmp(s1_sth.s_prov,"NF") != 0)) {
			fomen("Must be NB, NS or NF");
			s1_sth.s_prov[0] = LV_CHAR;
			return(ERROR);
		}
		break;
	case COMM_PREF:
		if(s1_sth.s_comm_pref[0] != ENG &&
		   s1_sth.s_comm_pref[0] != FRENCH) {
			fomen("Must be E(nglish) or F(rench)");
			s1_sth.s_comm_pref[0] = LV_CHAR;
			return(ERROR);
		}
		break;
	case PP_FLD:
		Right_Justify_Numeric(s1_sth.s_payper,sizeof(s1_sth.s_payper)-1);
		strcpy(pay_per.pp_code,s1_sth.s_payper);
		pay_per.pp_year = 0;
		flg_reset(PAY_PERIOD);

		retval = get_n_pay_per(&pay_per,BROWSE,0,FORWARD,e_mesg);
		if(retval < 0 || strcmp(pay_per.pp_code,s1_sth.s_payper)!=0) {
			fomer("Pay Period Code Does not Exist");
			s1_sth.s_payper[0] = LV_CHAR;
			return(ERROR);
		}
		seq_over(PAY_PERIOD);
		break;
	default :
#ifdef ENGLISH
		sprintf(e_mesg,"No Validity Check For Field#  %d",sr.curfld);
#else
		sprintf(e_mesg,"Pas de controle de validite pour le Champ#  %d",sr.curfld);
#endif
		fomen(e_mesg);
		get();
		return(ERROR) ;

	}	/* Switch sr.curfld */

	return(NOERROR) ;

}	/* Validate() */
/*****************************************************************************/
/*	Read Position file 						     */
/*								   	     */
Read_Position(temp_pos)
char	*temp_pos;
{
	int	retval;

	strcpy(position.p_code,temp_pos);
	retval = get_position(&position,BROWSE,0,e_mesg);
	return(retval);
}
/*****************************************************************************/
/*                                                                       */
/*  Display help window for applicable fields                            */
static
WindowHelp()
{
	int	retval ;

	switch(sr.curfld){
	case TEACH_FT:
		retval = position_hlp(s1_sth.s_tp_ft,7,13);
		printf("Andre");
	
		if(retval == DBH_ERR) return(retval);
		if(retval >= 0) redraw();
		if(retval == 0) return(ERROR);
		if(retval < 0) return(ERROR);
		retval = Read_Position(s1_sth.s_tp_ft);
		break;
	case TEACH_PT:
		retval = position_hlp(s1_sth.s_tp_pt,7,13);
		if(retval == DBH_ERR) return(retval);
		if(retval >= 0) redraw();
		if(retval == 0) return(ERROR);
		if(retval < 0) return(ERROR);
		retval = Read_Position(s1_sth.s_tp_pt);
		break;
	case SUB_TEACH:
		retval = position_hlp(s1_sth.s_sub,7,13);
		if(retval == DBH_ERR) return(retval);
		if(retval >= 0) redraw();
		if(retval == 0) return(ERROR);
		if(retval < 0) return(ERROR);
		retval = Read_Position(s1_sth.s_sub);
		break;
	case RETRO_CD:
		retval = earn_hlp("\0",s1_sth.s_retro_cd,7,13); 
		if(retval == DBH_ERR) return(retval);
		redraw();
		break;
	case REG_CD:
		retval = earn_hlp("\0",s1_sth.s_reg_cd,7,13); 
		if(retval == DBH_ERR) return(retval);
		redraw();
		break;
	case VAC_CD:
		retval = earn_hlp("\0",s1_sth.s_vac_cd,7,13); 
		if(retval == DBH_ERR) return(retval);
		redraw();
		break;
	default :

		fomer("No Help Window For This Field");
	}	/* Switch sr.curfld */

	return(NOERROR) ;
}	/* HdrAndKeyWindowHelp() */
/*------------------------------------------------------------*/
static
ReadScreen(mode)
int	mode;
{
	int err;

	scpy((char *)&tmp1_sth,(char *)&s1_sth,sizeof(s1_sth));
	SetDupBuffers(TEACH_FT,END_FLD1-200,1); /* Off Dup Control */
	
#ifdef	ENGLISH
	strcpy(s1_sth.s_mesg,"Press ESC-F to Terminate Edit");
#else
	strcpy(s1_sth.s_mesg,"Appuyer sur ESC-F pour terminer l'ajustement");
#endif
	DispMesgFld((char*)&s1_sth) ;

	InitFields(LV_CHAR,LV_LONG,LV_SHORT,LV_DOUBLE);
	/* Read data area of screen in single fomrd() */

	err = ReadFields((char*)&s1_sth, TEACH_FT, END_FLD1-200, Validate,
			WindowHelp, 1) ;
	if(err == DBH_ERR || err == PROFOM_ERR) return(err) ;
	if(PROFOM_ERR == err || DBH_ERR == err) return(err) ;
	if(RET_USER_ESC == err){
		if(mode == ADD) {
			InitFields(HV_CHAR,HV_LONG,HV_SHORT, HV_DOUBLE);
			ret(WriteFields((char *)&s1_sth,TEACH_FT,END_FLD1-200));
		}
		else {
		/* When user gives ESC-F while changing fields, assumption is
		* he completed his changes, and remaining fields are same as
		* old. But, at this point STH will be having low values in the 
		* remaining fields. So move the old values form the linked list.
		*/

			err = CopyBack((char *)&s1_sth,(char *)&tmp1_sth,
				sr.curfld, END_FLD1);
			if(err == PROFOM_ERR) return(err);
		}
		s1_sth.s_mesg[0] = HV_CHAR;
		DispMesgFld((char *)&s1_sth);

		return(RET_USER_ESC) ;
	}

	return(NOERROR);
}

/*------------------------------------------------------------------------*/
/* Move High values to all data fields and clear the screen */
static
ClearScreen()
{
	/* Move High Values to Hedaer part */
	InitFields(HV_CHAR, HV_LONG, HV_SHORT, HV_DOUBLE) ;

	ret( WriteFields((char *)&s1_sth,TEACH_FT, (END_FLD1 - 200)) );

	return(NOERROR);
}	/* ClearScreen() */
/*-------------------------------------------------------------------------*/
/* Initialize Screen Header with either Low or High Values */

InitFields( t_char, t_long, t_short, t_double)
char	t_char ;
long	t_long;
short	t_short;
double	t_double;
{

	/*  Fields	*/

	s1_sth.s_tp_ft[0] = t_char;	/* 2600	message option */
	s1_sth.s_pay_att[0] = t_char;	/* 2600	message option */
	s1_sth.s_tp_pt[0] = t_char;	/* 2600	message option */
	s1_sth.s_pay_tch[0] = t_char;	/* 2600	message option */
	s1_sth.s_tw_units = t_double;
	s1_sth.s_sub[0] = t_char;	/* 2600	message option */
	s1_sth.s_area_def[0] = t_char;	/* 2600	message option */

	s1_sth.s_retro = t_long;	
	s1_sth.s_cs_date = t_long;
	s1_sth.s_updt_gl[0] = t_char;	/* 2600	message option */
	s1_sth.s_ce_date = t_long;

	s1_sth.s_dept = t_short;
	s1_sth.s_fs_date = t_long;
	s1_sth.s_area = t_short;
	s1_sth.s_fe_date = t_long;

	s1_sth.s_cc = t_short;
	s1_sth.s_ss_date = t_long;
	s1_sth.s_reg_cd[0] = t_char;
	s1_sth.s_se_date = t_long;
	s1_sth.s_retro_cd[0] = t_char;
	s1_sth.s_st_mth = t_short;
	s1_sth.s_vac_cd[0] = t_char;
	s1_sth.s_last_chq = t_long;

	s1_sth.s_prov[0] = t_char;	/* 2600	message option */
	s1_sth.s_last_ec = t_long;
	s1_sth.s_comm_pref[0] = t_char;
	s1_sth.s_10mth_st_dt = t_long;
	s1_sth.s_payper[0] = t_char;
	if(t_char == HV_CHAR)
		s1_sth.s_payper[0] = HV_CHAR;
	s1_sth.s_10mth_end_dt = t_long;

	return(NOERROR) ;
}	/* InitFields() */
/*-----------------------------------------------------------------------
Changing fields. Accept fld to be changed and read that fld 
-----------------------------------------------------------------------*/
static int
ChangeFields()
{
	int	retval ;
	int	fld_no, end_fld;

	SetDupBuffers(TEACH_FT, END_FLD1-200, 1);

	/* Get The Field to Be Modified */
#ifdef	ENGLISH
	strcpy(s1_sth.s_mesg,"Enter RETURN to Terminate Edit");
#else
	strcpy(s1_sth.s_mesg,"Appuyer sur RETURN pour terminer l'ajustement");
#endif
	DispMesgFld((char *)&s1_sth); 
     	for (; ;) {
		s1_sth.s_field = LV_SHORT;
		retval = ReadFields((char *)&s1_sth,CHG_FLD1,CHG_FLD1,
			(int (*)())NULL,(int (*)())NULL, 1);

		if (retval < 0) return(-1);
		if (retval == RET_USER_ESC) break;	/* User enter ESC-F */

       		if (s1_sth.s_field == 0 ) break;  /* Finished changing fields */

		if (s1_sth.s_field > MAX_FIELD1) {
			fomen("Invalid Field Number");
			get();
			continue;
		}
		fld_no = (TEACH_FT) + (100 * (s1_sth.s_field-1));
		end_fld = fld_no;

		retval = ReadFields((char *)&s1_sth,fld_no, end_fld,
			Validate, WindowHelp,1) ;
	}
     	s1_sth.s_field = HV_SHORT ;
	if(WriteFields((char*)&s1_sth,CHG_FLD1,CHG_FLD1) < 0) return(-1);

     	s1_sth.s_mesg[0] = HV_CHAR ;
     	DispMesgFld((char *)&s1_sth);

	if(SetDupBuffers(TEACH_FT, END_FLD1-200, 0)<0) return(PROFOM_ERR);

	return(NOERROR);
}
/*-----------------------------------------------------------------------*/
SetDupBuffers(stfld, endfld, value)
int	stfld, endfld, value;
{
	int	i;

	for(i = stfld; i <= endfld; i += 100){

		fomca1(i, 19, value);
		fomca1(i, 10, value);
	}

	sr.nextfld = stfld;
	sr.endfld = endfld;
	fomud((char*)&s1_sth );

	return(NOERROR);
}

/*-------------------------------------------------------------------*/
GetScreen()
{
	int retval; 

	ret( WriteFields((char *)&s1_sth, TEACH_FT, END_FLD1-200) ) ;

	for( ; ; ) {
		retval = Confirm() ;
		if(retval != YES) {
			roll_back(e_mesg);
			break;
		}

		retval = WriteRecords(UPDATE) ;
		if(retval==NOERROR) break;
		if(retval==LOCKED) {
			roll_back(e_mesg) ;
			continue;
		}
		if (retval < 0) {
			roll_back(e_mesg);
			return(retval) ;
		}
	}

	strcpy(sr.scrnam,NFM_PATH);
	strcat(sr.scrnam,SCRNM1) ;

	if(retval != NOERROR) return(retval);

	return(NOERROR);

}	/* InitScreens() */

/*-------------------- E n d   O f   P r o g r a m ---------------------*/
