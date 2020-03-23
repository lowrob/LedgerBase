/*----------------------------------------------------------------------
Source Name: roe_adj.c  
System     : Personnel
Module     : Payroll/Personel
Created On : Jan 13, 1992
Created By : Eugene Roy

DESCRIPTION:

	This program alows the user to edit the record of employment
	before it is printed.

MODIFICATIONS:        

Programmer     	YY/MM/DD       	Description of modification
~~~~~~~~~~     	~~~~~~~~       	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
------------------------------------------------------------------------*/
#define	MAIN	
#define	MAINFL		ROE

#include <stdio.h>
#include <ctype.h>
#include <cfomstrc.h>
#include <repdef.h>
#include <bfs_defs.h>
#include <bfs_pp.h>
#include <bfs_com.h>
#include <pp_msgs.h>
#include <roe_adj.h>

#define MOD_DATE	"03-JAN-92"
#define SYSTEM   	"PAYROLL/PERSONEL"

char	txt_buff[80];	
char	txt_buff2[80];	
char	txt_buff3[80];	

static	int	Validate();
static	int	WindowHelp();

/*-------------------------------------------------------------------*/

main(argc,argv)
int argc;
char *argv[];
{
	int 	retval;

	retval = Initialize(argc,argv);	/* Initialization routine */

	retval = get_pay_param(&pay_param,BROWSE,1,e_mesg);
	if(retval < 0) {
  		DispError((char *)&s1_sth,e_mesg);
		return(retval);
	}

	retval = get_param(&param,BROWSE,1,e_mesg);
	if(retval < 0) {
  		DispError((char *)&s1_sth,e_mesg);
		return(retval);
	}

	retval = Process();

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
	fomer("C(hange), I(nquire), N(ext), P(revious), E(xit)");
#else
	fomer("C(hanger), I(nterroger), S(uivant), P(recedent), F(in)");
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
	case INQUIRE  :			/* Inquire */
		CHKACC(retval,BROWSE,e_mesg);
		return( Inquire() ) ;

	case NEXT_RECORD  :			/* Inquire */
		CHKACC(retval,BROWSE,e_mesg);
		return( Next(FORWARD) ) ;
	case PREV_RECORD  :			/* Inquire */
		CHKACC(retval,BROWSE,e_mesg);
		return( Next(BACKWARD) ) ;
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

	ret( WriteFields((char *)&s1_sth,KEY_START, (END_FLD1 - 200)) );

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
/* Show the next or previous Employees                      */

Next(direction)
int	direction ;
{
	int retval;

	strcpy(roe.ro_emp_numb,s1_sth.s_emp);
	if (flg_start(ROE) != direction) {
		inc_str(roe.ro_emp_numb, sizeof(roe.ro_emp_numb)-1, 
			direction);
		flg_reset(ROE);
	}

	retval = get_n_roe(&roe, BROWSE, 0, direction, e_mesg);
#ifndef ORACLE
	seq_over(ROE);
#endif
	if(ERROR == retval)return(DBH_ERR) ;
	if(EFL == retval) {
		fomen("No More Records....");
		get();
		return(NOERROR) ;
	}

	strcpy( s1_sth.s_emp,roe.ro_emp_numb);
	retval = ShowScreen(BROWSE);

	return( retval ) ;
}	/* Next() */
/*----------------------------------------------------------*/
/* Get the key and show the record */
SelectRecord()
{
	int	err ;

	first_time1 = 1;
	first_time2 = 1;
	first_time3 = 1;
	for( ;; ) {
		err = ReadKey();
		if(err != NOERROR) return(err) ;
	
		strcpy(roe.ro_emp_numb,s1_sth.s_emp);
		err = get_roe(&roe,BROWSE,0,e_mesg);
		if(err < 0)  {
			fomen(e_mesg);
			get();
			if( err == EFL ) continue;
			return(err);
		}
		break;
	}

	err = ShowScreen(BROWSE);

	return(NOERROR);
}	/* SelectRecord() */
/*----------------------------------------------------------------------*/
/* Get the Student Study Hall key from user. In ADD mode disable dup buffers, */
/* other modes enable dup buffers and show the current key as a default key */
ReadKey()
{
	int	i;
	char	hold_emp[13];
	
	/* In Add mode turn off dup control for key fields.
	   Other modes reverse it */

	SetDupBuffers(KEY_START, KEY_END, 1);

#ifdef ENGLISH
	strcpy(s1_sth.s_mesg,"Press ESC-F to Go Back to Fn:");
#else
	strcpy(s1_sth.s_mesg,"Appuyer sur ESC-F pour retourner a Fn:");
#endif
	DispMesgFld((char *)&s1_sth);

	strcpy(hold_emp,s1_sth.s_emp);

	s1_sth.s_emp[0] = LV_CHAR;

	i = ReadFields((char *)&s1_sth,KEY_START, KEY_END,
		Validate, WindowHelp,1) ;
	if(PROFOM_ERR == i || DBH_ERR == i) return(i) ;
	if(RET_USER_ESC == i){
		strcpy(s1_sth.s_emp,hold_emp);
		
		ret( WriteFields((char *)&s1_sth,KEY_START, KEY_END) ) ;

		s1_sth.s_mesg[0] = HV_CHAR;
		DispMesgFld((char *)&s1_sth);

		return(RET_USER_ESC) ;
	}

	s1_sth.s_mesg[0] = HV_CHAR;
	DispMesgFld((char *)&s1_sth);

	return(NOERROR);
}	/*  ReadKey() */
/*-----------------------------------------------------------*/
/* Move Header to Screen Hdr Fields */
ShowScreen(mode)
int	mode;
{
	int	i, retval;

	CopyToScreen();

	ret( WriteFields((char *)&s1_sth, KEY_START, END_FLD1-200) ) ;

	return(NOERROR) ;
}	/* ShowScreen() */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Copy the Screen fields to data record */
static int
CopyToScreen()
{
	int	retval;

	/*  Fields	*/

	  strcpy(emp_rec.em_numb, roe.ro_emp_numb);

	  retval = get_employee(&emp_rec,BROWSE,0,e_mesg);
	  if(retval < 0) {
		fomer(e_mesg);
		return(retval);
	  }
	  strcpy(position.p_code, emp_rec.em_pos);

	  retval = get_position(&position,BROWSE,0,e_mesg);
	  if(retval < 0) {
		fomer(e_mesg);
	  }

	  gov_param.gp_eff_date = get_date();

	  flg_reset(GOV_PARAM);

	  retval = get_n_gov_param(&gov_param,BROWSE, 0, BACKWARD, e_mesg) ;
	  if(retval == EFL) {
		fomen("Government Parameter Record Not Setup");
		return(retval);
	  }
	  if(retval < 0) {
		fomer(e_mesg) ;
		return(ERROR) ;
	  }

	  school.sc_numb = param.pa_distccno; 

	  retval = get_sch(&school,BROWSE,0,e_mesg);
	  if(retval < 0) {
		fomer(e_mesg);
		get();
	  }
	  strcpy(s1_sth.s_pay_ref, emp_rec.em_numb);

	  if(emp_rec.em_lang[0] == '\0' || emp_rec.em_lang[0] == ' ')
	  	strcpy(s1_sth.s_comm_pref, "E");
	  else
	  	strcpy(s1_sth.s_comm_pref, emp_rec.em_lang);
	  if(strcmp(position.p_type,"PT") == 0 ||
		strcmp(position.p_type,"FT") == 0)
	  	strcpy(s1_sth.s_rev_can, gov_param.gp_empl_ref);
	  else
	  	strcpy(s1_sth.s_rev_can, gov_param.gp_tax_acct);

	  strcpy(barg_unit.b_code,emp_rec.em_barg);
	  barg_unit.b_date = get_date();
	  flg_reset(BARG);

	  retval = get_n_barg(&barg_unit,BROWSE,0,BACKWARD,e_mesg);
	  if(retval < 0 || strcmp(barg_unit.b_code,emp_rec.em_barg)!=0){
		DispError((char *)&s1_sth,"Error Reading Bargaining Unit File");
		return(-1);
	  }
	  seq_over(BARG);

	  strcpy(pay_per.pp_code,pay_param.pr_payper);
	  pay_per.pp_year = 9999;
	  flg_reset(PAY_PERIOD);

	  retval = get_n_pay_per(&pay_per,BROWSE,0,BACKWARD,e_mesg);
	  if(retval < 0){
		DispError((char *)&s1_sth,e_mesg) ;
	  }
	  if(strcmp(pay_per.pp_code,pay_param.pr_payper)!=0){
		fomer("Error Reading Pay Period File");
	  }
	  seq_over(PAY_PERIOD);

	  strncpy(s1_sth.s_pp_type, pay_per.pp_desc,17);

	  sprintf(s1_sth.s_pp_no,"%d", pay_per.pp_numb);

	  strcpy(s1_sth.s_empl_name, school.sc_name);
	  strcpy(e_mesg,emp_rec.em_last_name);
	  strcat(e_mesg,", ");
	  strcat(e_mesg,emp_rec.em_first_name);
	  strncpy(s1_sth.s_emp_name,e_mesg,30);

	  strcpy(s1_sth.s_empl_add1, school.sc_add1);
	  strcpy(s1_sth.s_emp_add1, emp_rec.em_add1);
	  strcpy(s1_sth.s_empl_add2, school.sc_add2);
	  strcpy(s1_sth.s_emp_add2, emp_rec.em_add2);
	  strcpy(s1_sth.s_empl_add3, school.sc_add3);
	  strcpy(s1_sth.s_emp_add3, emp_rec.em_add3);
	  s1_sth.s_empl_add4[0] = ' ';
	  strcpy(s1_sth.s_emp_add4, emp_rec.em_add4);
	  strcpy(s1_sth.s_empl_pc, school.sc_pc);
	  strcpy(s1_sth.s_emp_pc, emp_rec.em_pc);

	  strncpy(s1_sth.s_occ, position.p_desc, 15);
	  strcpy(s1_sth.s_sin, emp_rec.em_sin);

	  s1_sth.s_fd_date = roe.ro_first_dt;
	  s1_sth.s_ld_date = roe.ro_last_dt;
	  s1_sth.s_upp_date = roe.ro_uic_prdt;
	  s1_sth.s_fppe_date = roe.ro_final_dt;
	  strcpy(s1_sth.s_serial, roe.ro_serial);
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
 "Y(es), S(creen Edit), L(ine edit), 2(nd screen), 3(rd screen), C(ancel)",
		"YSL23C");  	
		
#else
		err = GetOption((char *)&s1_sth,
 "O(ui), S(creen edit), L(ine edit), 2(eme ecran), 3(ieme ecran), A(nnuler)",
		"OSL23A"); 

#endif
		break ;
	    case  INQUIRE :		/* Change */
#ifdef ENGLISH
		err = GetOption((char *)&s1_sth,
		"Y(es), 2(nd screen), 3(rd screen), C(ancel)","Y23C");  
#else
		err = GetOption((char *)&s1_sth,
		"O(ui), 2(eme ecran), 3(ieme ecran), A(nnuler)","O23A"); 
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

	    case  THIRD_SCR : 
		strcpy(sr.scrnam,NFM_PATH);
		strcat(sr.scrnam,SCRNM3) ;

		err = GetScreen3();
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
	int	j,k;
	short	tot_num_ins_wk;

	strcpy(roe.ro_emp_numb,s1_sth.s_emp);
	if(mode != ADD) {
		retval = get_roe(&roe,UPDATE,0,e_mesg);
		if(retval < 0) {
			fomer(e_mesg);
			return(retval);
		}
	}
	
	/*  Fields	*/

	roe.ro_first_dt = s1_sth.s_fd_date;
	roe.ro_last_dt = s1_sth.s_ld_date;
	roe.ro_uic_prdt = s1_sth.s_upp_date;
	roe.ro_final_dt = s1_sth.s_fppe_date;
	strcpy(roe.ro_serial,s1_sth.s_serial);

	if(first_time2 == 0){
	  roe.ro_ins_earn[0] = s2_sth.s_pp_earn1;
	  roe.ro_ins_wks[0] = s2_sth.s_no_pp1;
	  roe.ro_ins_earn[1] = s2_sth.s_pp_earn2;
	  roe.ro_ins_wks[1] = s2_sth.s_no_pp2;
	  roe.ro_ins_earn[2] = s2_sth.s_pp_earn3;
	  roe.ro_ins_wks[2] = s2_sth.s_no_pp3;
	  roe.ro_ins_earn[3] = s2_sth.s_pp_earn4;
	  roe.ro_ins_wks[3] = s2_sth.s_no_pp4;
	  roe.ro_ins_earn[4] = s2_sth.s_pp_earn5;
	  roe.ro_ins_wks[4] = s2_sth.s_no_pp5;
	  roe.ro_ins_earn[5] = s2_sth.s_pp_earn6;
	  roe.ro_ins_wks[5] = s2_sth.s_no_pp6;
	  roe.ro_ins_earn[6] = s2_sth.s_pp_earn7;
	  roe.ro_ins_wks[6] = s2_sth.s_no_pp7;
	  roe.ro_ins_earn[7] = s2_sth.s_pp_earn8;
	  roe.ro_ins_wks[7] = s2_sth.s_no_pp8;
	  roe.ro_ins_earn[8] = s2_sth.s_pp_earn9;
	  roe.ro_ins_wks[8] = s2_sth.s_no_pp9;
	  roe.ro_ins_earn[9] = s2_sth.s_pp_earn10;
	  roe.ro_ins_wks[9] = s2_sth.s_no_pp10;
	  roe.ro_ins_earn[10] = s2_sth.s_pp_earn11;
	  roe.ro_ins_wks[10] = s2_sth.s_no_pp11;
	  roe.ro_ins_earn[11] = s2_sth.s_pp_earn12;
	  roe.ro_ins_wks[11] = s2_sth.s_no_pp12;
	  roe.ro_ins_earn[12] = s2_sth.s_pp_earn13;
	  roe.ro_ins_wks[12] = s2_sth.s_no_pp13;
	  roe.ro_ins_earn[13] = s2_sth.s_pp_earn14;
	  roe.ro_ins_wks[13] = s2_sth.s_no_pp14;
	  roe.ro_ins_earn[14] = s2_sth.s_pp_earn15;
	  roe.ro_ins_wks[14] = s2_sth.s_no_pp15;
	  roe.ro_ins_earn[15] = s2_sth.s_pp_earn16;
	  roe.ro_ins_wks[15] = s2_sth.s_no_pp16;
	  roe.ro_ins_earn[16] = s2_sth.s_pp_earn17;
	  roe.ro_ins_wks[16] = s2_sth.s_no_pp17;
	  roe.ro_ins_earn[17] = s2_sth.s_pp_earn18;
	  roe.ro_ins_wks[17] = s2_sth.s_no_pp18;
	  roe.ro_ins_earn[18] = s2_sth.s_pp_earn19;
	  roe.ro_ins_wks[18] = s2_sth.s_no_pp19;
	  roe.ro_ins_earn[19] = s2_sth.s_pp_earn20;
	  roe.ro_ins_wks[19] = s2_sth.s_no_pp20;

	  roe.ro_vac = s2_sth.s_vac_pay;
	  roe.ro_stat1_dt = s2_sth.s_stat_date1;
	  roe.ro_stat1_amnt = s2_sth.s_stat_amt1;
	  strcpy( roe.ro_reas1, s2_sth.s_reason1);
	  roe.ro_reas1_amnt = s2_sth.s_reas_amt1;

	  roe.ro_stat2_dt = s2_sth.s_stat_date2;
	  roe.ro_stat2_amnt = s2_sth.s_stat_amt2;
	  strcpy( roe.ro_reas2, s2_sth.s_reason2);
	  roe.ro_reas2_amnt = s2_sth.s_reas_amt2;

	  roe.ro_stat3_dt = s2_sth.s_stat_date3;
	  roe.ro_stat3_amnt = s2_sth.s_stat_amt3;
	  strcpy( roe.ro_reas3, s2_sth.s_reason3);
	  roe.ro_reas3_amnt = s2_sth.s_reas_amt3;

	  strcpy(roe.ro_all, s2_sth.s_all_pp);
	  strcpy(roe.ro_all_wks_max, s2_sth.s_all_wks_max);
/* Put the value in the 20 week area for now. Lou June 22/1997 */
roe.ro_hours = s2_sth.s_pp_earn1;
roe.ro_earnings = s2_sth.s_pp_earn2;
	}

	if(first_time3 == 0){
	  roe.ro_start_dt = s3_sth.s_start_dt;
	  strcpy(roe.ro_wk_days, s3_sth.s_wk_days);
	  roe.ro_week_dno = s3_sth.s_week_dno;
	  roe.ro_amnt = s3_sth.s_amnt; 
	  strcpy(roe.ro_e_n_u, s3_sth.s_e_n_u);
	  roe.ro_ret_dt = s3_sth.s_ret_dt;
	  strcpy(roe.ro_reason, s3_sth.s_reason);

	  strcpy(roe.ro_contact, s3_sth.s_contact);
	  strcpy(roe.ro_cntct_tel,s3_sth.s_cntct_tel);
	  strcpy(roe.ro_issuer, s3_sth.s_issuer);
	  strcpy(roe.ro_issuer_tel,s3_sth.s_issuer_tel); 
	  roe.ro_issue_dt = s3_sth.s_issue_dt;

	  strcpy(roe.ro_com1, s3_sth.s_com1);
	  strcpy(roe.ro_com2, s3_sth.s_com2);
	  strcpy(roe.ro_com3, s3_sth.s_com3);
	  strcpy(roe.ro_com4, s3_sth.s_com4);
	}
	roe.ro_ins_week = s2_sth.s_ins_wks;

	retval = put_roe(&roe,mode,e_mesg);
	if(retval < 0) {
		DispError((char *)&s1_sth,e_mesg);
		roll_back(e_mesg);
		return(retval);
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
	int	save_nextfld, save_endfld ;
	int	j, i, retval;
	char	part1[5], part2[5];
	long	value, number, final;

	switch(sr.curfld){
	case KEY_START:
		Right_Justify_Numeric(s1_sth.s_emp,
			sizeof(s1_sth.s_emp)-1);
		ret( WriteFields((char *)&s1_sth,KEY_START, KEY_END) ) ;

		if(s1_sth.s_emp[0] == '\0'){
			s1_sth.s_emp[0] = LV_CHAR;
			return(-1);
		}

		strcpy(roe.ro_emp_numb,s1_sth.s_emp);
		retval = get_roe(&roe,BROWSE,0,e_mesg);
		if(retval < 0 ){
		  DispError((char *)&s1_sth, e_mesg);
		  s1_sth.s_emp[0] = LV_CHAR;
		  return(-1);
		}

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
/*-----------------------------------------------------------------------*/
/*                                                                       */
/*  Display help window for applicable fields                            */
static
WindowHelp()
{
	int	retval ;
	short	reccod ;

	switch(sr.curfld){
	case KEY_START:
		retval = emp_hlp(s1_sth.s_emp,7,13);
		if(retval == DBH_ERR) return(retval);
		if(retval >= 0) redraw();
		if(retval == 0) return(ERROR);
		if(retval < 0) return(ERROR);
		strcpy(emp_rec.em_numb,s1_sth.s_emp);
		retval = get_employee(&emp_rec,BROWSE,0,e_mesg);
		if(retval < 0)  {
			fomer(e_mesg);
			s1_sth.s_emp[0] = LV_CHAR;
			return(ERROR);
		}
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
	SetDupBuffers(FD_DATE,END_FLD1,1); /* Off Dup Control */
	
	s1_sth.s_fd_date = LV_LONG;
	s1_sth.s_ld_date = LV_LONG;
	s1_sth.s_upp_date = LV_LONG;
	s1_sth.s_fppe_date = LV_LONG;
	s1_sth.s_serial[0] = LV_CHAR;

#ifdef	ENGLISH
	strcpy(s1_sth.s_mesg,"Press ESC-F to Terminate Edit");
#else
	strcpy(s1_sth.s_mesg,"Appuyer sur ESC-F pour terminer l'ajustement");
#endif
	DispMesgFld((char*)&s1_sth) ;

	/* Read data area of screen in single fomrd() */

	err = ReadFields((char*)&s1_sth, FD_DATE, END_FLD1-200, Validate,
			WindowHelp, 1) ;
	if(err == DBH_ERR || err == PROFOM_ERR) return(err) ;
	if(PROFOM_ERR == err || DBH_ERR == err) return(err) ;
	if(RET_USER_ESC == err){
		if(mode == ADD) {
			InitFields(HV_CHAR,HV_LONG);
			ret(WriteFields((char *)&s1_sth,FD_DATE,END_FLD1-200));
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
	InitFields(HV_CHAR, HV_LONG) ;

	ret( WriteFields((char *)&s1_sth,KEY_START, (END_FLD1 - 200)) );

	return(NOERROR);
}	/* ClearScreen() */
/*-------------------------------------------------------------------------*/
/* Initialize Screen Header with either Low or High Values */

InitFields( t_char, t_long)
char	t_char ;
long	t_long;
{
	int	i;

	/*  Fields	*/

	s1_sth.s_pay_ref[0] = t_char;
	s1_sth.s_comm_pref[0] = t_char;
	s1_sth.s_rev_can[0] = t_char;
	s1_sth.s_pp_type[0] = t_char;

	pay_per.pp_code[0] = t_char;

	s1_sth.s_pp_no[0] = t_char;

	s1_sth.s_empl_name[0] = t_char;
	s1_sth.s_emp_name[0] = t_char;

	s1_sth.s_empl_add1[0] = t_char;
	s1_sth.s_emp_add1[0] = t_char;
	s1_sth.s_empl_add2[0] = t_char;
	s1_sth.s_emp_add2[0] = t_char;
	s1_sth.s_empl_add3[0] = t_char;
	s1_sth.s_emp_add3[0] = t_char;
	s1_sth.s_empl_add4[0] = t_char;
	s1_sth.s_emp_add4[0] = t_char;
	s1_sth.s_empl_pc[0] = t_char;
	s1_sth.s_emp_pc[0] = t_char;

	s1_sth.s_occ[0] = t_char;
	s1_sth.s_sin[0] = t_char;

	s1_sth.s_fd_date = t_long;
	s1_sth.s_ld_date = t_long;
	s1_sth.s_upp_date = t_long;
	s1_sth.s_fppe_date = t_long;
	s1_sth.s_serial[0] = t_char;

	return(NOERROR) ;
}	/* InitFields() */
/*-----------------------------------------------------------------------*/
/* Allow all lines on screen to be edited				 */
/*-----------------------------------------------------------------------*/
static
ChangeScreen()
{
	int 	retval;
	
	scpy((char*)&tmp1_sth, (char*)&s1_sth, sizeof(s1_sth));
	SetDupBuffers(FD_DATE, END_FLD1-200, 1);

	if(WriteFields((char*)&s1_sth, MESG_FLD1, MESG_FLD1)<0) 
		return(-1);

	retval = ReadFields((char*)&s1_sth, FD_DATE,END_FLD1-200,
					Validate, WindowHelp, 1);
	if(retval < 0) return(-1);
	if(retval == RET_USER_ESC){
			CopyBack((char*)&s1_sth,(char*)&tmp1_sth,
					sr.curfld, END_FLD1-200);
	}

	s1_sth.s_mesg[0] = HV_CHAR;
	sr.nextfld = MESG_FLD1; 
	fomwf((char *)&s1_sth);

	return(retval);
}
/*-----------------------------------------------------------------------
Changing fields. Accept fld to be changed and read that fld 
-----------------------------------------------------------------------*/
static int
ChangeFields()
{
	int	i, retval ;
	int	fld_no, end_fld;

	SetDupBuffers(FD_DATE, END_FLD1-200, 1);

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
		fld_no = (FD_DATE) + (100 * (s1_sth.s_field-1));
		end_fld = fld_no;

		retval = ReadFields((char *)&s1_sth,fld_no, end_fld,
			Validate, WindowHelp,1) ;
	}
     	s1_sth.s_field = HV_SHORT ;
	if(WriteFields((char*)&s1_sth,CHG_FLD1,CHG_FLD1) < 0) return(-1);

     	s1_sth.s_mesg[0] = HV_CHAR ;
     	DispMesgFld((char *)&s1_sth);

	if(SetDupBuffers(FD_DATE, END_FLD1-200, 0)<0) return(PROFOM_ERR);

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
	int retval, err, i, j; 

	CopyToScreen();

	for( ; ; ) {
		err = Confirm() ;
		if(err != YES) {
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

	if(err == YES) return(err);
	if(err == CANCEL) return(err);
	return(NOERROR);

}	/* InitScreens() */

/*-------------------- E n d   O f   P r o g r a m ---------------------*/
