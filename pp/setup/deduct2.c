/*-----------------------------------------------------------------------
Source Name: deduct2.c  
System     : Personel Payroll
Module     : Deduction Maintenance
Created  On: Oct. 24, 1991
Created  By: Sheldon Floyd

DESCRIPTION:
	Reads screen 2 of deduction maintenance. The actual maintenance 
	processing is done in deduct.c 

MODIFICATIONS:        

Programmer     	YY/MM/DD       	Description of modification
~~~~~~~~~~     	~~~~~~~~       	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
------------------------------------------------------------------------*/

#include <cfomstrc.h>
#include <ctype.h>
#include <repdef.h>
#include <bfs_defs.h>
#include <bfs_pp.h>
#include <bfs_com.h>
#include <pp_msgs.h>
#include <deduct.h>

static	int	Validate();
static	int	WindowHelp();

short	curr_item;	

/*-------------------------------------------------------------------*/
GetScreen2()
{
	int retval, i; 

	s2_sth.s_sysdate = get_date();	
	s2_sth.s_field = HV_SHORT ;
	s2_sth.s_ftn[0] = HV_SHORT;

	s2_sth.s_dedcd[0] = HV_CHAR;
	s2_sth.s_paypd[0] = HV_CHAR;
	s2_sth.s_field = HV_SHORT;
	s2_sth.s_fund = HV_SHORT;

	s2_sth.s_account[0] = HV_CHAR;

	for(i=0;i<MAX_KEYS;i++){
		s2_sth.s_acckey[i] = HV_LONG;
	}

	s2_sth.s_mesg[0] = HV_CHAR ;
	s2_sth.s_opt[0] = HV_CHAR ;
	
	if(WriteFields((char*)&s2_sth,FTN_FLD2,OPT_FLD2 )<0) return(-1);
	retval = SetDefaults();	/* set the field defaults */

	retval = Deduct2();

	strcpy(sr.scrnam,NFM_PATH);
	strcat(sr.scrnam,SCRNM1) ;

	if(retval != NOERROR) return(retval);

	return(NOERROR);

}	/* InitScreens() */

/*-------------------------------------------------------------------*/
DisplayScreen2()
{
	int retval;
	strcpy(sr.scrnam,NFM_PATH);
	strcat(sr.scrnam,SCRNM2) ;

	strcpy(s2_sth.s_dedcd,s1_sth.s_dedcd);
	strcpy(s2_sth.s_ftn,s1_sth.s_ftn);
	strcpy(s2_sth.s_paypd,s1_sth.s_paypd);
	s2_sth.s_field = HV_SHORT;

	SetDupBuffers(FTN_FLD2, ACCKEY_FLD12, 1);

	for(;;){
		if(s2_sth.s_ftn[0] == INQREC){
#ifdef	ENGLISH
			retval = GetOpt("Y(es), 1(st screen)", "Y1");
#else	
			retval = GetOpt("Y(es), 1(st screen)", "Y1");
#endif
		}
		else{
#ifdef	ENGLISH
			retval = GetOpt("Y(es), 1(st screen), C(ancel)", "Y1C");
#else	
			retval = GetOpt("Y(es), 1(st screen), C(ancel)", "Y1C");
#endif
		}

		if(retval == FIRST_SCR) break;

		if(retval == YES){
			strcpy(sr.scrnam,NFM_PATH);
			strcat(sr.scrnam,SCRNM1) ;
			return(YES);
		}

		if(retval == CANCEL){
			strcpy(sr.scrnam,NFM_PATH);
			strcat(sr.scrnam,SCRNM1) ;
			return(CANCEL);
		}
	
	}

	strcpy(sr.scrnam,NFM_PATH);
	strcat(sr.scrnam,SCRNM1) ;
	return(NOERROR);
}
/*-------------------------------------------------------------------*/
static
Deduct2() 
{
	int retval;
		
	/* read the screen fields */

	retval = ReadFields((char*)&s2_sth, FUND_FLD, ACCKEY_FLD12, 
						Validate, WindowHelp, 0);
	if(retval <0) return(-1);

	retval=ReadOption2();		
	if(retval<0) return(retval);
	if(retval==CANCEL) return(CANCEL); /* Exit program if CANCEL */

	return(NOERROR);
}
/*-----------------------------------------------------------------------
Validate flds when PROFOM returns RET_VAL_CHK 
-----------------------------------------------------------------------*/
static
Validate()	
{
	int	retval;
	
	switch(sr.curfld){

	case	FUND_FLD:
		if(s2_sth.s_fund == 0){
			s2_sth.s_fund = LV_SHORT;
			return(-1);
		}

		control.fund = 000;
		flg_reset(CONTROL);
		for(;;) {
			retval=get_n_ctl(&control,BROWSE,0,FORWARD,e_mesg);
			if (retval == EFL) {
				fomen("Fund Number does not Exist");
				s2_sth.s_fund = LV_SHORT;
				return(ERROR);	
			}

			if(control.fund != s2_sth.s_fund) 
				continue;
			break;
		}
		break;

	case	ACC_FLD:
		Right_Justify_Numeric(s2_sth.s_account,sizeof(glmast.accno)-1);
		glmast.funds = s2_sth.s_fund;
		strcpy(glmast.accno,s2_sth.s_account);
		glmast.reccod = 99;
		retval=get_gl(&glmast,BROWSE,0,e_mesg);
		if (retval != 0) {
			fomen("Liability Account Number does not Exist");
			s2_sth.s_account[0] = LV_CHAR;
			return(ERROR);	
		}
		break;

	case	ACCKEY_FLD1:
	case	ACCKEY_FLD2:
	case	ACCKEY_FLD3:
	case	ACCKEY_FLD4:
	case	ACCKEY_FLD5:
	case	ACCKEY_FLD6:
	case	ACCKEY_FLD7:
	case	ACCKEY_FLD8:
	case	ACCKEY_FLD9:
	case	ACCKEY_FLD10:
	case	ACCKEY_FLD11:
	case	ACCKEY_FLD12:
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

	fomer("No Help Window for This Field");
	return(NOERROR) ;

}	/* WindowHelp() */
/*-----------------------------------------------------------------------*/
static
SetDefaults()
{
	strcpy(s2_sth.s_dedcd,s1_sth.s_dedcd);
	strcpy(s2_sth.s_ftn,s1_sth.s_ftn);
	strcpy(s2_sth.s_paypd,s1_sth.s_paypd);

	SetDupBuffers(FTN_FLD2, ACCKEY_FLD12, 1);

	/* move low values */
	MoveLows();

	if(WriteFields((char*)&s2_sth,FTN_FLD2,OPT_FLD2)<0) 
		return(-1);

	return(NOERROR);
}

/*-----------------------------------------------------------------------*/
/* Display the confirmation message at the bottom of the screen, take the
   option from user and call necessary functions */
/*-----------------------------------------------------------------------*/
ReadOption2()
{
	int	retval;

	s2_sth.s_field = HV_SHORT;
	WriteFields((char *)&s2_sth,FTN_FLD2,CHG_FLD2);

	for( ; ; ) {

#ifdef	ENGLISH
		retval = GetOpt("1(st screen), S(creen edit), L(ine edit), C(ancel)", "1SLC");
#else	
		retval = GetOpt("1(st screen), S(creen edit), L(ine edit), C(ancel)", "1SLC");
#endif
		switch(retval) {	/* process the option */
		case  FIRST_SCR :

			s2_sth.s_mesg[0] = HV_CHAR;
			sr.nextfld = MESG_FLD2; 
			fomwf((char *)&s2_sth);

			strcpy(sr.scrnam,NFM_PATH);
			strcat(sr.scrnam,SCRNM1) ;

			return(YES);	

		case  SCREDIT :

			retval = ChangeScreen();
			break ;

		case  LINEEDIT  :

			retval = ChangeFields();
			break ;

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

		if(retval < 0) return(retval) ;

	}	/* for(;;) */
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
	
        strcpy( s2_sth.s_mesg, msg );
	sr.nextfld = MESG_FLD2; 
	fomwf((char *)&s2_sth);

	for(;;) {
		s2_sth.s_opt[0] = LV_CHAR ;
		if ( ReadFields((char*)&s2_sth, OPT_FLD2, OPT_FLD2, Validate,
						WindowHelp, 0) < 0 ) 
			return(-1);

                j = strlen(option);
		for ( i = 0; i < j; i++ ) 
			if ( s2_sth.s_opt[0] == option[i] )
                           break;
                if(i != j) break ;
                fomer(INVOPT);
	}
        s2_sth.s_mesg[0] = HV_CHAR ;
        s2_sth.s_opt[0] = HV_CHAR ;
	if (WriteFields((char*)&s2_sth,MESG_FLD2,OPT_FLD2) < 0 ) return(-1);
 
        return( (int)(option[i])) ;
}

/*-----------------------------------------------------------------------*/
/* Allow all lines on screen to be edited				 */
/*-----------------------------------------------------------------------*/

static
ChangeScreen()
{
	int 	retval;
	int 	firstfld;
	
	scpy((char*)&tmp2_sth, (char*)&s2_sth, sizeof(s2_sth));
	SetDupBuffers(FUND_FLD, ACCKEY_FLD12, 1);

        strcpy( s2_sth.s_mesg, ITMMSG) ;
	if(WriteFields((char*)&s2_sth, MESG_FLD2, MESG_FLD2)<0) 
		return(-1);

	MoveLows();
	
	firstfld = FUND_FLD;

	retval = ReadFields((char*)&s2_sth, firstfld, ACCKEY_FLD12, 
					Validate, WindowHelp, 1);
	if(retval < 0) return(-1);
	if(retval == RET_USER_ESC){
		CopyBack((char*)&s2_sth,(char*)&tmp2_sth,
					sr.curfld,ACCKEY_FLD12);
		return(NOERROR);
	}

	s2_sth.s_mesg[0] = HV_CHAR;
	sr.nextfld = MESG_FLD2; 
	fomwf((char *)&s2_sth);

	return(retval);
}
/*-----------------------------------------------------------------------
Changing fields. Accept fld to be changed and read that fld 
-----------------------------------------------------------------------*/
static
ChangeFields()
{
     	int	retval;
	short	st_fld, end_fld; 

     	for(;;){
		SetDupBuffers(FUND_FLD,ACCKEY_FLD12, 1);
		scpy((char*)&tmp2_sth, (char*)&s2_sth, sizeof(s2_sth));

		strcpy(s2_sth.s_mesg, FLDPROMPT);
		DispMesgFld((char*)&s2_sth);

		s2_sth.s_field = LV_SHORT;
		retval = ReadFields((char*)&s2_sth, CHG_FLD2, CHG_FLD2,
					Validate, WindowHelp, 1);
		if (retval < 0) return(-1);

		if(retval == RET_USER_ESC) break;	/* User enters ESC-F */
       		if(s2_sth.s_field == 0 ) break;		/* Finished changing */
 
		s2_sth.s_mesg[0] = HV_CHAR ;
		DispMesgFld((char*)&s2_sth);

		switch(s2_sth.s_field){
		case 1:
			s2_sth.s_fund = LV_SHORT;
			end_fld = st_fld = FUND_FLD;
			break;

		case 2:
			s2_sth.s_account[0] = LV_CHAR;
			end_fld = st_fld = ACC_FLD;
			break;

		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
			s2_sth.s_acckey[s2_sth.s_field-3] = LV_LONG;
			end_fld= st_fld = ACCKEY_FLD1 +((s2_sth.s_field-3)*100);
			break;

		default:

			fomen(BADFIELD);
			continue;

		} /* end switch */	

		retval = ReadFields((char*)&s2_sth, st_fld, end_fld,
				 Validate, WindowHelp, 1);
		if(retval < 0) return(-1);
	
		if(retval == RET_USER_ESC){
			CopyBack((char*)&s2_sth,(char*)&tmp2_sth,
					st_fld,end_fld);
		}
	
	} /* end for loop */

     	s2_sth.s_field = HV_SHORT ;
	if ( WriteFields((char*)&s2_sth, CHG_FLD2, CHG_FLD2) < 0 ) 
		return(-1);

     	s2_sth.s_mesg[0] = HV_CHAR ;
	DispMesgFld((char*)&s2_sth);

     	return(NOERROR);

}	/* ChangeFields() */

/*-----------------------------------------------------------------------*/
static
MoveLows()
{
	int i;
	
	s2_sth.s_fund = LV_SHORT;
	s2_sth.s_account[0] = LV_CHAR;

	for(i=0;i<MAX_KEYS;i++){
		s2_sth.s_acckey[i] = LV_LONG;
	}
	return(NOERROR);
}
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
	fomud((char*)&s2_sth );

	return(NOERROR);
}
/*------------------    END OF FILE   ------------------------*/
