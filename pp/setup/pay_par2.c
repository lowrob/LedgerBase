/*-----------------------------------------------------------------------
Source Name: pay_par2.c  
System     : Personel Payroll
Module     : Personnel
Created  On: Feb 11, 1992
Created  By: Eugene Roy

DESCRIPTION:
	Program to modify the second screen of the Payroll Parameter.

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
#include <pay_par.h>

static	long	six;
static	long 	twelve;

static	int	Validate();
static	int	WindowHelp();

/*-------------------------------------------------------------------*/
GetScreen2()
{
	int retval, i; 

	retval = get_pay_param(&pay_param,BROWSE,1,e_mesg);
	if(retval < 0 && retval != UNDEF)  {
		fomen(e_mesg);
		get();
		return(retval);
	}

	strcpy(s2_sth.s_pgname2, PRGNM);
	s2_sth.s_sysdate2 = get_date();	
	s2_sth.s_field2 = s1_sth.s_field ;
	strcpy(s2_sth.s_fn2, s1_sth.s_fn);

	if(first_time2 == 1){
		for(i= 0; i < 12; i++){
		 strcpy(s2_sth.s_gl[0].s_acct_keys[i], pay_param.pr_cpp_exp[i]);
		}
		for(i= 0; i < 12; i++){
		 strcpy(s2_sth.s_gl[1].s_acct_keys[i], pay_param.pr_uic_exp[i]);
		}
		for(i= 0; i < 12; i++){
		 strcpy(s2_sth.s_gl[2].s_acct_keys[i], pay_param.pr_salary[i]);
		}
		for(i= 0; i < 12; i++){
		 strcpy(s2_sth.s_gl[3].s_acct_keys[i], pay_param.pr_deduct[i]);
		}
		for(i= 0; i < 12; i++){
		 strcpy(s2_sth.s_gl[4].s_acct_keys[i], pay_param.pr_reg_pen[i]);
		}
		for(i= 0; i < 12; i++){
		 strcpy(s2_sth.s_gl[5].s_acct_keys[i], pay_param.pr_teacher[i]);
		}
		s2_sth.s_fund = pay_param.pr_fund;
		strcpy(s2_sth.s_acct, pay_param.pr_teach_gl);

		for(i= 0; i < 12; i++){
			s2_sth.s_keys[i] = pay_param.pr_keys[i];
		}
	}
	first_time2 = 0;

	if(WriteFields((char*)&s2_sth,FN_FLD2,END_FLD2 )<0) return(-1);

	retval = Process2();

	strcpy(sr.scrnam,NFM_PATH);
	strcat(sr.scrnam,SCRNM1) ;

	if(retval != NOERROR) return(retval);

	return(NOERROR);

}	/* InitScreens() */

/*-------------------------------------------------------------------*/
static
Process2() 
{
	int retval;
		
	/* read the screen fields */

	retval=Confirm2();		
	if(retval<0) return(retval);
	if(retval==YES) return(YES); /* Exit program if YES */
	if(retval==CANCEL) return(CANCEL); /* Exit program if CANCEL */

	return(NOERROR);
}
/*-----------------------------------------------------------------------*/
/* Take the confirmation from user for the header part */

static
Confirm2()
{
	int	err ;

	/* Options:
	   Add      - YALSNPC
	   Change   - YALSNPC
	   Delete   - YC
	*/

	for( ; ; ) {
	    switch(s2_sth.s_fn2[0]) {
	    case  CHANGE :		/* Change */
#ifdef ENGLISH
		err = GetOption((char *)&s2_sth,
 "Y(es), S(creen Edit), L(ine edit), 1(st screen), C(ancel)",
		"YSL1C");  	
		
#else
		err = GetOption((char *)&s2_sth,
 "O(ui), S(creen edit), L(ine edit), 1(ere ecran), A(nnuler)",
		"OSL1A"); 

#endif
		break ;
	    case  INQUIRE :		/* Change */
#ifdef ENGLISH
		err = GetOption((char *)&s2_sth,
		"Y(es), 1(st screen), C(ancel)","Y1C");  
#else
		err = GetOption((char *)&s2_sth,
		"O(ui), 1(ere ecran), A(nnuler)","O1A"); 
#endif
		break ;
	    }
	    if(err == PROFOM_ERR) return(err) ;

	    switch(err) {
	    case  YES  :
		return(YES);
	    case  SCREENEDIT:
		err = ReadScreen2();
		break;
	    case  LINEEDIT  :
		err = ChangeFields2();
		break ;
	    case  FIRST_SCR :
		strcpy(sr.scrnam,NFM_PATH);
		strcat(sr.scrnam,SCRNM1) ;

		err = GetScreen();
		if(err == YES) return(err);
		if(err == CANCEL) return(err);
		break;

	    case  CANCEL :
#ifdef ENGLISH
		err = GetOption((char *)&s2_sth,
				"Confirm the Cancel (Y/N)?", "YN") ;
#else
		err = GetOption((char *)&s2_sth,
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
ReadScreen2()
{
	int err;

	scpy((char *)&tmp2_sth,(char *)&s2_sth,sizeof(tmp2_sth));

	SetDupBuffers2(ACCT_KEY1,END_FLD2 - 200,1); 
	
	InitFields2(LV_SHORT, LV_CHAR, LV_LONG);
#ifdef	ENGLISH
	strcpy(s2_sth.s_mesg2,"Press ESC-F to Terminate Edit");
#else
	strcpy(s2_sth.s_mesg2,"Appuyer sur ESC-F pour terminer l'ajustement");
#endif
	DispMesgFld((char*)&s2_sth) ;

	err = ReadFields((char*)&s2_sth, ACCT_KEY1, END_FLD2 - 200, Validate,
			WindowHelp, 1) ;
	if(err == DBH_ERR || err == PROFOM_ERR) return(err) ;
	if(PROFOM_ERR == err || DBH_ERR == err) return(err) ;
	if(RET_USER_ESC == err){
		err = CopyBack((char *)&s2_sth,(char *)&tmp2_sth,
			sr.curfld, END_FLD2 - 200);
		if(err == PROFOM_ERR) return(err);

		s2_sth.s_mesg2[0] = HV_CHAR;
		DispMesgFld((char *)&s2_sth);

		return(RET_USER_ESC) ;
	}

	return(NOERROR);
}
/*-------------------------------------------------------------------------*/
/* Initialize Screen Header with either Low or High Values */

static
InitFields2( t_short, t_char, t_long)
short	t_short ;
char	t_char;
long	t_long;
{

	for(six= 0; six < 6; six++){
	   for(twelve= 0; twelve < 12; twelve++){
		s2_sth.s_gl[six].s_acct_keys[twelve][0] = t_char;
	   }
	}
	s2_sth.s_fund = t_short;
	s2_sth.s_acct[0] = t_char;

	for(twelve= 0; twelve < 12; twelve++){
		s2_sth.s_keys[twelve] = t_long;
	}

	return(NOERROR);
}	/* InitFields() */
/*-----------------------------------------------------------------------
Validate flds when PROFOM returns RET_VAL_CHK 
-----------------------------------------------------------------------*/
static
Validate()	
{
	int	retval, j;
	
	if(sr.curfld >= ACCT_KEY1 && sr.curfld < ACCT_KEY2){
		j = (sr.curfld - ACCT_KEY1) / 100;
		sr.curfld = ACCT_KEY1;
	}
	if(sr.curfld >= ACCT_KEY2 && sr.curfld < ACCT_KEY3){
		j = (sr.curfld - ACCT_KEY2) / 100;
		sr.curfld = ACCT_KEY2;
	}
	if(sr.curfld >= ACCT_KEY3 && sr.curfld < ACCT_KEY4){
		j = (sr.curfld - ACCT_KEY3) / 100;
		sr.curfld = ACCT_KEY3;
	}
	if(sr.curfld >= ACCT_KEY4 && sr.curfld < ACCT_KEY5){
		j = (sr.curfld - ACCT_KEY4) / 100;
		sr.curfld = ACCT_KEY4;
	}
	if(sr.curfld >= ACCT_KEY5 && sr.curfld < ACCT_KEY6){
		j = (sr.curfld - ACCT_KEY5) / 100;
		sr.curfld = ACCT_KEY5;
	}
	if(sr.curfld >= ACCT_KEY6 && sr.curfld < FUND){
		j = (sr.curfld - ACCT_KEY6) / 100;
		sr.curfld = ACCT_KEY6;
	}

	switch(sr.curfld){
	case ACCT_KEY1:
		if(s2_sth.s_gl[0].s_acct_keys[j][0] != YES &&
		   s2_sth.s_gl[0].s_acct_keys[j][0] != NO){
			fomen("Must be Y(es) or N(o)");
			s2_sth.s_gl[0].s_acct_keys[j][0] = LV_CHAR;
			return(ERROR);
		}
		break;
	case ACCT_KEY2:
		if(s2_sth.s_gl[1].s_acct_keys[j][0] != YES &&
		   s2_sth.s_gl[1].s_acct_keys[j][0] != NO){
			fomen("Must be Y(es) or N(o)");
			s2_sth.s_gl[1].s_acct_keys[j][0] = LV_CHAR;
			return(ERROR);
		}
		break;
	case ACCT_KEY3:
		if(s2_sth.s_gl[2].s_acct_keys[j][0] != YES &&
		   s2_sth.s_gl[2].s_acct_keys[j][0] != NO){
			fomen("Must be Y(es) or N(o)");
			s2_sth.s_gl[2].s_acct_keys[j][0] = LV_CHAR;
			return(ERROR);
		}

		break;
	case ACCT_KEY4:
		if(s2_sth.s_gl[3].s_acct_keys[j][0] != YES &&
		   s2_sth.s_gl[3].s_acct_keys[j][0] != NO){
			fomen("Must be Y(es) or N(o)");
			s2_sth.s_gl[3].s_acct_keys[j][0] = LV_CHAR;
			return(ERROR);
		}
		break;
	case ACCT_KEY5:
		if(s2_sth.s_gl[4].s_acct_keys[j][0] != YES &&
		   s2_sth.s_gl[4].s_acct_keys[j][0] != NO){
			fomen("Must be Y(es) or N(o)");
			s2_sth.s_gl[4].s_acct_keys[j][0] = LV_CHAR;
			return(ERROR);
		}
		break;
	case ACCT_KEY6:
		if(s2_sth.s_gl[5].s_acct_keys[j][0] != YES &&
		   s2_sth.s_gl[5].s_acct_keys[j][0] != NO){
			fomen("Must be Y(es) or N(o)");
			s2_sth.s_gl[5].s_acct_keys[j][0] = LV_CHAR;
			return(ERROR);
		}
		break;
	case FUND:
		control.fund = s2_sth.s_fund;
		retval = get_ctl(&control,BROWSE,0,e_mesg);
		if(retval < 0) {
			fomer("Fund Number Does not Exist");
			s2_sth.s_fund = LV_SHORT;
			return(ERROR);
		}
		break;
	case TEACH_GL:
		Right_Justify_Numeric(s2_sth.s_acct,sizeof(glmast.accno)-1);
		if(WriteFields((char*)&s2_sth, TEACH_GL, TEACH_GL)<0) 
			return(-1);
		glmast.funds = s2_sth.s_fund;
		strcpy(glmast.accno,s2_sth.s_acct);
		glmast.reccod = 99;
		retval=get_gl(&glmast,BROWSE,0,e_mesg);
		if (retval != 0) {
			fomen("Account Number does not Exist");
			s2_sth.s_acct[0] = LV_CHAR;
			return(ERROR);	
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

	fomer("No Help Window for This Field");
	return(NOERROR) ;

}	/* WindowHelp() */
/*-----------------------------------------------------------------------
Changing fields. Accept fld to be changed and read that fld 
-----------------------------------------------------------------------*/
static int
ChangeFields2()
{
	int	retval ;
	int	fld_no, end_fld;

	SetDupBuffers2(ACCT_KEY1,END_FLD2,1);

#ifdef	ENGLISH
	strcpy(s2_sth.s_mesg2,"Enter RETURN to Terminate Edit");
#else
	strcpy(s2_sth.s_mesg2,"Appuyer sur RETURN pour terminer l'ajustement");
#endif
	DispMesgFld((char *)&s2_sth); 
        
     	for (; ;) {
		s2_sth.s_field2 = LV_SHORT;
		retval = ReadFields((char *)&s2_sth,CHG_FLD2,CHG_FLD2,
			(int (*)())NULL,(int (*)())NULL, 1);

		if (retval < 0) return(-1);
		if (retval == RET_USER_ESC) break;

       		if (s2_sth.s_field2 == 0 ) break;

		if (s2_sth.s_field2 > MAX_FIELD2) {
			fomen("Invalid Field Number");
			get();
			continue;
		}

		fld_no = (ACCT_KEY1);
		if(s2_sth.s_field2 == 2)
			fld_no = (ACCT_KEY2);
		if(s2_sth.s_field2 == 3)
			fld_no = (ACCT_KEY3);
		if(s2_sth.s_field2 == 4)
			fld_no = (ACCT_KEY4);
		if(s2_sth.s_field2 == 5)
			fld_no = (ACCT_KEY5);
		if(s2_sth.s_field2 == 6)
			fld_no = (ACCT_KEY6);
		if(s2_sth.s_field2 > 6)
			fld_no = (FUND) + (100 * (s2_sth.s_field2-7));
		end_fld = fld_no;

		if(fld_no == ACCT_KEY1)
			end_fld = fld_no + 1100;
		if(fld_no == ACCT_KEY2)
			end_fld = fld_no + 1100;
		if(fld_no == ACCT_KEY3)
			end_fld = fld_no + 1100;
		if(fld_no == ACCT_KEY4)
			end_fld = fld_no + 1100;
		if(fld_no == ACCT_KEY5)
			end_fld = fld_no + 1100;
		if(fld_no == ACCT_KEY6)
			end_fld = fld_no + 1100;

		retval = ReadFields((char *)&s2_sth,fld_no, end_fld,
			Validate, WindowHelp,1) ;
	}

     	s2_sth.s_field2 = HV_SHORT ;
	if ( WriteFields((char *)&s2_sth,CHG_FLD2, CHG_FLD2) < 0 ) return(-1);

     	s2_sth.s_mesg2[0] = HV_CHAR ;
     	DispMesgFld((char *)&s2_sth);

	if(SetDupBuffers2(ACCT_KEY1, END_FLD2 - 200, 0)<0) return(PROFOM_ERR);

	return(NOERROR);
}	/* ChangeFields() */

/*-----------------------------------------------------------------------*/
static
SetDupBuffers2(stfld, endfld, value)
int	stfld, endfld, value;
{
	int	i;

	for(i = stfld; i <= endfld; i += 100){

		fomca1(i, 19, value);
		fomca1(i, 10, value);
	}

	sr.nextfld = stfld;
	sr.endfld = endfld;
	fomud((char*)&s2_sth );

	return(NOERROR);
}
/*------------------    END OF FILE   ------------------------*/
