/*-----------------------------------------------------------------------
Source Name: roe_adj3.c  
System     : Personel Payroll
Module     : Personnel
Created  On: Jan 14, 1992
Created  By: Eugene Roy

DESCRIPTION:
	Program to modify the second screen of the record of employment.

MODIFICATIONS:        

Programmer     	YY/MM/DD       	Description of modification
~~~~~~~~~~     	~~~~~~~~       	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
------------------------------------------------------------------------*/
#include <stdio.h>
#include <ctype.h>
#include <cfomstrc.h>
#include <repdef.h>
#include <bfs_defs.h>
#include <bfs_pp.h>
#include <bfs_com.h>
#include <pp_msgs.h>
#include <roe_adj.h>

static	int	Validate();
static	int	WindowHelp();

/*-------------------------------------------------------------------*/
GetScreen3()
{
	int retval, i, j; 
	short	tot_num_ins_wk;

	strcpy(s3_sth.s_pgname3, PRGNM);
	s3_sth.s_sysdate3 = get_date();	
	s3_sth.s_field3 = s1_sth.s_field ;
	strcpy(s3_sth.s_fn3, s1_sth.s_fn);
	strcpy(s3_sth.s_emp3, s1_sth.s_emp);

	if(first_time3 == 1){
	  s3_sth.s_field3 = HV_SHORT;
	  s3_sth.s_dummy3[0] = ' ';

	  s3_sth.s_start_dt = roe.ro_start_dt;	
	  strcpy(s3_sth.s_wk_days, roe.ro_wk_days);
	  s3_sth.s_week_dno = roe.ro_week_dno;
	  s3_sth.s_amnt = roe.ro_amnt;		/* Amount	*/
	  strcpy(s3_sth.s_e_n_u, roe.ro_e_n_u);	/* Weeks_Days	*/
	  s3_sth.s_ret_dt = roe.ro_ret_dt;	
	  strcpy(s3_sth.s_reason, roe.ro_reason);

	  strcpy(term_rec.t_code, roe.ro_reason);

	  retval = get_pterm(&term_rec,BROWSE,0,e_mesg);
	  if(retval < 0) {
		fomer(e_mesg);
	  }
	  strcpy(s3_sth.s_reas_desc, term_rec.t_desc);

	  strcpy(s3_sth.s_contact, roe.ro_contact);
	  strcpy(s3_sth.s_cntct_tel,roe.ro_cntct_tel);
	  strcpy(s3_sth.s_issuer, roe.ro_issuer);
	  strcpy(s3_sth.s_issuer_tel,roe.ro_issuer_tel);
	  s3_sth.s_issue_dt = roe.ro_issue_dt;
	  strcpy(s3_sth.s_com1, roe.ro_com1);
	  strcpy(s3_sth.s_com2, roe.ro_com2);		
	  strcpy(s3_sth.s_com3, roe.ro_com3);	
	  strcpy(s3_sth.s_com4, roe.ro_com4);
	}
	first_time3 = 0;

	if(WriteFields((char*)&s3_sth,FN_FLD3,END_FLD3 )<0) return(-1);

	retval = Process3();

	strcpy(sr.scrnam,NFM_PATH);
	strcat(sr.scrnam,SCRNM1) ;

	return(retval);
}	/* InitScreens() */

/*-------------------------------------------------------------------*/
static
Process3() 
{
	int retval;
		
	/* read the screen fields */

	retval=Confirm3();		
	if(retval<0) return(retval);
	if(retval==YES) return(YES); /* Exit program if YES */
	if(retval==CANCEL) return(CANCEL); /* Exit program if CANCEL */

	return(NOERROR);
}
/*-----------------------------------------------------------------------*/
/* Take the confirmation from user for the header part */

static
Confirm3()
{
	int	err ;

	/* Options:
	   Add      - YALSNPC
	   Change   - YALSNPC
	   Delete   - YC
	*/
	for( ; ; ) {
	    switch(s3_sth.s_fn3[0]) {
	    case  CHANGE :		/* Change */
#ifdef ENGLISH
		err = GetOption((char *)&s3_sth,
 "Y(es), S(creen Edit), L(ine edit), 1(st screen), 2(nd screen), C(ancel)",
		"YSL12C");  	
		
#else
		err = GetOption((char *)&s3_sth,
 "O(ui), S(creen edit), L(ine edit), 1(ere ecran), 2(ieme ecran), A(nnuler)",
		"OSL12A"); 

#endif
		break ;
	    case  INQUIRE :		/* Change */
#ifdef ENGLISH
		err = GetOption((char *)&s3_sth,
		"Y(es), 1(st screen), 2(nd screen), C(ancel)","Y12C");  
#else
		err = GetOption((char *)&s3_sth,
		"O(ui), 1(ere ecran), 2(ieme ecran), A(nnuler)","O12A"); 
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
	    case  FIRST_SCR :
		strcpy(sr.scrnam,NFM_PATH);
		strcat(sr.scrnam,SCRNM1) ;

		err = GetScreen();
		if(err == YES) return(err);
		if(err == CANCEL) return(err);
		break;

	    case  SEC_SCR : 
		strcpy(sr.scrnam,NFM_PATH);
		strcat(sr.scrnam,SCRNM2) ;

		err = GetScreen2();
		if(err == YES) return(err);
		if(err == CANCEL) return(err);
		break;

	    case  CANCEL :
#ifdef ENGLISH
		err = GetOption((char *)&s3_sth,
				"Confirm the Cancel (Y/N)?", "YN") ;
#else
		err = GetOption((char *)&s3_sth,
				"Confirmer l'annulation (O/N)?", "ON") ;
#endif
		if(err == YES) return(CANCEL) ;
		break ;
	    }	/* switch err */

	    if(err == PROFOM_ERR) return(err) ;
	    if(err == DBH_ERR) return(err) ;
	}	/* for(; ; ) */
}	/* Confirm() */
/*------------------------------------------------------------*/
static
ReadScreen(mode)
int	mode;
{
	int err;

	scpy((char *)&tmp3_sth,(char *)&s3_sth,sizeof(tmp3_sth));

	SetDupBuffers(START_DATE,REASON,1); /* Off Dup Control */
	SetDupBuffers(CONTACT,END_FLD3 - 200,1); /* Off Dup Control */
	
	s3_sth.s_start_dt = LV_LONG;	/* Last Day Worked	*/
	s3_sth.s_wk_days[0] = LV_CHAR;		/* Weeks_Days	*/
	s3_sth.s_week_dno = LV_SHORT;		/* No of weeks or days */
	s3_sth.s_amnt = LV_DOUBLE;		/* Amount	*/
	s3_sth.s_e_n_u[0] = LV_CHAR;	/* Weeks_Days	*/
	s3_sth.s_ret_dt = LV_LONG;		/* Last Day Worked	*/
	s3_sth.s_reason[0] = LV_CHAR;	/* Weeks_Days	*/
	s3_sth.s_contact[0] = LV_CHAR;		/* Weeks_Days	*/
	s3_sth.s_cntct_tel[0] = LV_CHAR;	/* Last Day Worked	*/
	s3_sth.s_issuer[0] = LV_CHAR;		/* Weeks_Days	*/
	s3_sth.s_issuer_tel[0] = LV_CHAR;
	s3_sth.s_issue_dt = LV_LONG;		/* Last Day Worked	*/
	s3_sth.s_com1[0] = LV_CHAR;		/* Weeks_Days	*/
	s3_sth.s_com2[0] = LV_CHAR;		/* Weeks_Days	*/
	s3_sth.s_com3[0] = LV_CHAR;	/* Weeks_Days	*/
	s3_sth.s_com4[0] = LV_CHAR;		/* Weeks_Days	*/

#ifdef	ENGLISH
	strcpy(s3_sth.s_mesg3,"Press ESC-F to Terminate Edit");
#else
	strcpy(s3_sth.s_mesg3,"Appuyer sur ESC-F pour terminer l'ajustement");
#endif
	DispMesgFld((char*)&s3_sth) ;

	/* Read data area of screen in single fomrd() */

	err = ReadFields((char*)&s3_sth, START_DATE, END_FLD3 - 200, Validate,
			WindowHelp, 1) ;
	if(err == DBH_ERR || err == PROFOM_ERR) return(err) ;
	if(PROFOM_ERR == err || DBH_ERR == err) return(err) ;
	if(RET_USER_ESC == err){
		if(mode == ADD) {
			InitFields(LV_LONG, LV_CHAR, HV_SHORT,HV_DOUBLE);
			ret(WriteFields((char *)&s3_sth,START_DATE,END_FLD3 - 200));
		}
		else {
			err = CopyBack((char *)&s3_sth,(char *)&tmp3_sth,
				sr.curfld, END_FLD3 - 200);
			if(err == PROFOM_ERR) return(err);
		}
		s3_sth.s_mesg3[0] = HV_CHAR;
		DispMesgFld((char *)&s3_sth);

		return(RET_USER_ESC) ;
	}

	return(NOERROR);
}
/*-------------------------------------------------------------------------*/
/* Initialize Screen Header with either Low or High Values */

static
InitFields( t_long,t_char,t_short,t_double )
long	t_long;
char	t_char;
short	t_short ;
double	t_double;
{

	s3_sth.s_start_dt = t_long;	/* Last Day Worked	*/
	s3_sth.s_wk_days[0] = t_char;		/* Weeks_Days	*/
	s3_sth.s_week_dno = t_short;		/* No of weeks or days */
	s3_sth.s_amnt = t_double;		/* Amount	*/
	s3_sth.s_e_n_u[0] = t_char;	/* Weeks_Days	*/
	s3_sth.s_ret_dt = t_long;		/* Last Day Worked	*/
	s3_sth.s_reason[0] = t_char;	/* Weeks_Days	*/
	s3_sth.s_contact[0] = t_char;		/* Weeks_Days	*/
	s3_sth.s_cntct_tel[0] = t_char;		/* Last Day Worked	*/
	s3_sth.s_issuer[0] = t_char;		/* Weeks_Days	*/
	s3_sth.s_issuer_tel[0] = t_char;
	s3_sth.s_issue_dt = t_long;		/* Last Day Worked	*/
	s3_sth.s_com1[0] = t_char;		/* Weeks_Days	*/
	s3_sth.s_com2[0] = t_char;		/* Weeks_Days	*/
	s3_sth.s_com3[0] = t_char;	/* Weeks_Days	*/
	s3_sth.s_com4[0] = t_char;		/* Weeks_Days	*/

	return(NOERROR);

}	/* InitScreens() */
/*-----------------------------------------------------------------------
Validate flds when PROFOM returns RET_VAL_CHK 
-----------------------------------------------------------------------*/
static
Validate()	
{
	int	retval;
	
	switch(sr.curfld){
	case WEEK_DAYS:
	  if(s3_sth.s_wk_days[0] == '\0' || s3_sth.s_wk_days[0] == ' '){
		s3_sth.s_week_dno = 0;
		sr.curfld += 200;
		break;
	  }
	  if((strcmp(s3_sth.s_wk_days,"W") != 0) &&
	     (strcmp(s3_sth.s_wk_days,"D") != 0)){ 	

		fomer("Invalid Entry");
		s3_sth.s_wk_days[0] = LV_CHAR;
		return(ERROR);
	  }
	  break;
	case ENU:
	  if((strcmp(s3_sth.s_e_n_u,"E") != 0) &&
	     (strcmp(s3_sth.s_e_n_u,"N") != 0) && 	
	     (strcmp(s3_sth.s_e_n_u,"U") != 0)){ 	

		fomer("Invalid Entry");
		s3_sth.s_e_n_u[0] = LV_CHAR;
		return(ERROR);
	  }
	  break;
	case REASON:
		strcpy(term_rec.t_code,s3_sth.s_reason);
		retval = get_pterm(&term_rec,BROWSE,0,e_mesg);
		if(retval == UNDEF){
			fomer("Termination Code Does not Exist");
			s3_sth.s_reason[0] = LV_CHAR;
			return(ERROR);
		}
		if(retval < 0){
			DispError((char *)&s3_sth,e_mesg);
			s3_sth.s_reason[0] = LV_CHAR;
			return(retval);
		}
		break;
	default:
#ifdef ENGLISH
		sprintf(e_mesg,"No Validity Check Field # %d",sr.curfld);
#else
		sprintf(e_mesg,"Pas de controle de validite pour le Champ#  %d",sr.curfld);
#endif
		fomen(e_mesg);
		get();
		break;

	}/* end switch */	

	return(NOERROR) ;

}	/* Validate() */
/*-----------------------------------------------------------------------
Display help window for applicable fields 
-----------------------------------------------------------------------*/
static
WindowHelp()
{
	int	retval;

	fomer(NOHELP);

	sr.nextfld = sr.curfld ;
	return(NOERROR) ;

}	/* WindowHelp() */

/*-----------------------------------------------------------------------*/
/* Allow all lines on screen to be edited				 */
/*-----------------------------------------------------------------------*/

static
ChangeScreen()
{
	int 	retval;
	int 	firstfld;
	
	scpy((char*)&tmp3_sth, (char*)&s3_sth, sizeof(s3_sth));
	SetDupBuffers(START_DATE,REASON,1); /* Off Dup Control */
	SetDupBuffers(CONTACT,END_FLD3 - 200,1); /* Off Dup Control */

        strcpy( s3_sth.s_mesg3, ITMMSG) ;

	if(WriteFields((char*)&s3_sth, MESG_FLD3, MESG_FLD3)<0) 
		return(-1);

	retval = ReadFields((char*)&s3_sth, firstfld, END_FLD3 - 200, 
					Validate, WindowHelp, 1);
	if(retval < 0) return(-1);
	if(retval == RET_USER_ESC){
		CopyBack((char*)&s3_sth,(char*)&tmp3_sth,
					sr.curfld,END_FLD3 - 200);
		return(NOERROR);
	}

	s3_sth.s_mesg3[0] = HV_CHAR;
	sr.nextfld = MESG_FLD3; 
	fomwf((char *)&s3_sth);

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

	SetDupBuffers(START_DATE,END_FLD3 - 200, 1);

	/* Get The Field to Be Modified */
#ifdef	ENGLISH
	strcpy(s3_sth.s_mesg3,"Enter RETURN to Terminate Edit");
#else
	strcpy(s3_sth.s_mesg3,"Appuyer sur RETURN pour terminer l'ajustement");
#endif
	DispMesgFld((char *)&s3_sth); 
        
     	for (; ;) {
		s3_sth.s_field3 = LV_SHORT;
		retval = ReadFields((char *)&s3_sth,CHG_FLD3,CHG_FLD3,
			(int (*)())NULL,(int (*)())NULL, 1);

		if (retval < 0) return(-1);
		if (retval == RET_USER_ESC) break;	/* User enter ESC-F */

       		if (s3_sth.s_field3 == 0 ) break;

		if (s3_sth.s_field3 > MAX_FIELD3) {
			fomen("Invalid Field Number");
			get();
			continue;
		}
		fld_no = (START_DATE) + (100 * (s3_sth.s_field3-1));
		if(s3_sth.s_field3 == 2){
			end_fld = fld_no + 100;
	
			retval = ReadFields((char *)&s3_sth,fld_no, fld_no,
				Validate, WindowHelp,1) ;
	
			retval = ReadFields((char *)&s3_sth,end_fld, end_fld,
				Validate, WindowHelp,1) ;
		}
		else{
			if(s3_sth.s_field3 > 2)
				fld_no += 100;
			if(s3_sth.s_field3 > 6)
				fld_no += 100;
			end_fld = fld_no;
	
			retval = ReadFields((char *)&s3_sth,fld_no, end_fld,
				Validate, WindowHelp,1) ;
		}
	}
     	s3_sth.s_field3 = HV_SHORT ;
	if ( WriteFields((char *)&s3_sth,CHG_FLD3, CHG_FLD3) < 0 ) return(-1);

     	s3_sth.s_mesg3[0] = HV_CHAR ;
     	DispMesgFld((char *)&s3_sth);

	if(SetDupBuffers(START_DATE, END_FLD3 - 200, 0)<0) return(PROFOM_ERR);

	return(NOERROR);
}	/* ChangeFields() */

/*-----------------------------------------------------------------------*/
static
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
	fomud((char*)&s3_sth );

	return(NOERROR);
}
/*------------------    END OF FILE   ------------------------*/
