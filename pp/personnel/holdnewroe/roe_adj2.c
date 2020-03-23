/*-----------------------------------------------------------------------
Source Name: roe_adj2.c  
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
GetScreen2()
{
	int retval, i, j, k; 
	short	tot_num_ins_wk = 0;

	strcpy(s2_sth.s_pgname2, PRGNM);
	s2_sth.s_sysdate2 = get_date();	
	s2_sth.s_field2 = s1_sth.s_field ;
	strcpy(s2_sth.s_fn2, s1_sth.s_fn);
	strcpy(s2_sth.s_emp2, s1_sth.s_emp);

	if(first_time2 == 1){
		s2_sth.s_field2 = HV_SHORT;
		strcpy(s2_sth.s_dummy1,"Ex");

		s2_sth.s_pp_earn1 = roe.ro_ins_earn[0];
		s2_sth.s_no_pp1 = roe.ro_ins_wks[0];
		s2_sth.s_pp_earn2 = roe.ro_ins_earn[1];
		s2_sth.s_no_pp2 = roe.ro_ins_wks[1];
		s2_sth.s_pp_earn3 = roe.ro_ins_earn[2];
		s2_sth.s_no_pp3 = roe.ro_ins_wks[2];
		s2_sth.s_pp_earn4 = roe.ro_ins_earn[3];
		s2_sth.s_no_pp4 = roe.ro_ins_wks[3];
		s2_sth.s_pp_earn5 = roe.ro_ins_earn[4];
		s2_sth.s_no_pp5 = roe.ro_ins_wks[4];
		s2_sth.s_pp_earn6 = roe.ro_ins_earn[5];
		s2_sth.s_no_pp6 = roe.ro_ins_wks[5];
		s2_sth.s_pp_earn7 = roe.ro_ins_earn[6];
		s2_sth.s_no_pp7 = roe.ro_ins_wks[6];
		s2_sth.s_pp_earn8 = roe.ro_ins_earn[7];
		s2_sth.s_no_pp8 = roe.ro_ins_wks[7];
		s2_sth.s_pp_earn9 = roe.ro_ins_earn[8];
		s2_sth.s_no_pp9 = roe.ro_ins_wks[8];
		s2_sth.s_pp_earn10 = roe.ro_ins_earn[9];
		s2_sth.s_no_pp10 = roe.ro_ins_wks[9];
		s2_sth.s_pp_earn11 = roe.ro_ins_earn[10];
		s2_sth.s_no_pp11 = roe.ro_ins_wks[10];
		s2_sth.s_pp_earn12 = roe.ro_ins_earn[11];
		s2_sth.s_no_pp12 = roe.ro_ins_wks[11];
		s2_sth.s_pp_earn13 = roe.ro_ins_earn[12];
		s2_sth.s_no_pp13 = roe.ro_ins_wks[12];
		s2_sth.s_pp_earn14 = roe.ro_ins_earn[13];
		s2_sth.s_no_pp14 = roe.ro_ins_wks[13];
		s2_sth.s_pp_earn15 = roe.ro_ins_earn[14];
		s2_sth.s_no_pp15 = roe.ro_ins_wks[14];
		s2_sth.s_pp_earn16 = roe.ro_ins_earn[15];
		s2_sth.s_no_pp16 = roe.ro_ins_wks[15];
		s2_sth.s_pp_earn17 = roe.ro_ins_earn[16];
		s2_sth.s_no_pp17 = roe.ro_ins_wks[16];
		s2_sth.s_pp_earn18 = roe.ro_ins_earn[17];
		s2_sth.s_no_pp18 = roe.ro_ins_wks[17];
		s2_sth.s_pp_earn19 = roe.ro_ins_earn[18];
		s2_sth.s_no_pp19 = roe.ro_ins_wks[18];
		s2_sth.s_pp_earn20 = roe.ro_ins_earn[19];
		s2_sth.s_no_pp20 = roe.ro_ins_wks[19];
		s2_sth.s_ins_wks = roe.ro_ins_week;

		retval = TotalPayPer();
		s2_sth.s_vac_pay = roe.ro_vac;
		strcpy(s2_sth.s_all_wks_max,roe.ro_all_wks_max);

		s2_sth.s_dummy2[0] = ' ';

		s2_sth.s_stat_date1 = roe.ro_stat1_dt;
		s2_sth.s_stat_amt1 = roe.ro_stat1_amnt;
		strcpy(s2_sth.s_reason1, roe.ro_reas1);
		s2_sth.s_reas_amt1 = roe.ro_reas1_amnt;

		s2_sth.s_stat_date2 = roe.ro_stat1_dt;
		s2_sth.s_stat_amt2 = roe.ro_stat2_amnt;
		strcpy(s2_sth.s_reason2, roe.ro_reas2);
		s2_sth.s_reas_amt2 = roe.ro_reas2_amnt;

		s2_sth.s_stat_date3 = roe.ro_stat1_dt;
		s2_sth.s_stat_amt3 = roe.ro_stat3_amnt;
		strcpy(s2_sth.s_reason3, roe.ro_reas3);
		s2_sth.s_reas_amt3 = roe.ro_reas3_amnt;

		strcpy(s2_sth.s_all_pp, roe.ro_all);
	}
	first_time2 = 0;

	if(WriteFields((char*)&s2_sth,FN_FLD2,END_FLD2 )<0) return(-1);

	retval = Process2();

	strcpy(sr.scrnam,NFM_PATH);
	strcat(sr.scrnam,SCRNM1) ;

	return(retval);
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
 "Y(es), S(creen Edit), L(ine edit), 1(st screen), 3(rd screen), C(ancel)",
		"YSL13C");  	
		
#else
		err = GetOption((char *)&s2_sth,
 "O(ui), S(creen edit), L(ine edit), 1(ere ecran), 3(ieme ecran), A(nnuler)",
		"OSL13A"); 

#endif
		break ;
	    case  INQUIRE :		/* Change */
#ifdef ENGLISH
		err = GetOption((char *)&s2_sth,
		"Y(es), 1(st screen), 3(rd screen), C(ancel)","Y13C");  
#else
		err = GetOption((char *)&s2_sth,
		"O(ui), 1(ere ecran), 3(ieme ecran), A(nnuler)","O13A"); 
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

	    case  THIRD_SCR : 
		strcpy(sr.scrnam,NFM_PATH);
		strcat(sr.scrnam,SCRNM3) ;

		err = GetScreen3();
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
ReadScreen(mode)
int	mode;
{
	int	err, i,j;
	int	retval;

	scpy((char *)&tmp2_sth,(char *)&s2_sth,sizeof(tmp2_sth));

#ifdef ENGLISH
	strcpy(s2_sth.s_mesg2,"Press ESC-F to Terminate");
#else
	strcpy(s2_sth.s_mesg2,"Appuyer sur ESC-F pour terminer");
#endif
	DispMesgFld((char *)&s2_sth);

	SetDupBuffers(PP_EARN1,MAX_EARN,1); /* Off Dup Control */
	SetDupBuffers(STAT_HOL1,END_FLD2,1); /* Off Dup Control */
	
	InitFields(LV_SHORT,LV_DOUBLE,LV_LONG,LV_CHAR);

#ifdef	ENGLISH
	strcpy(s2_sth.s_mesg2,"Press ESC-F to Terminate Edit");
#else
	strcpy(s2_sth.s_mesg2,"Appuyer sur ESC-F pour terminer l'ajustement");
#endif
	DispMesgFld((char*)&s2_sth) ;

	/* Read data area of screen in single fomrd() */

	err = ReadFields((char*)&s2_sth, PP_EARN1, END_FLD2-200, Validate,
			WindowHelp, 1) ;
	if(err == DBH_ERR || err == PROFOM_ERR) return(err) ;
	if(PROFOM_ERR == err || DBH_ERR == err) return(err) ;
	if(RET_USER_ESC == err){
		if(mode == ADD) {
			InitFields(HV_SHORT,HV_DOUBLE,HV_LONG,HV_CHAR);
			ret(WriteFields((char *)&s2_sth,PP_EARN1,END_FLD2-200));
		}
		else {
			err = CopyBack((char *)&s2_sth,(char *)&tmp2_sth,
				sr.curfld, END_FLD2-200);
			if(err == PROFOM_ERR) return(err);
		}
		s2_sth.s_mesg2[0] = HV_CHAR;
		DispMesgFld((char *)&s2_sth);

		return(RET_USER_ESC) ;
	}
	retval = TotalPayPer();
	if(WriteFields((char*)&s2_sth,TOTAL,TOTAL+100 )<0) return(-1);

	return(NOERROR);
}
/*-------------------------------------------------------------------------*/
/* Initialize Screen Header with either Low or High Values */

static
InitFields( t_short,t_double,t_long,t_char)
short	t_short ;
double	t_double;
long	t_long;
char	t_char;
{
	int	i,j;

	s2_sth.s_pp_earn1 = t_double;
	s2_sth.s_no_pp1 = t_short;
	s2_sth.s_pp_earn2 = t_double;
	s2_sth.s_no_pp2 = t_short;
	s2_sth.s_pp_earn3 = t_double;
	s2_sth.s_no_pp3 = t_short;
	s2_sth.s_pp_earn4 = t_double;
	s2_sth.s_no_pp4 = t_short;
	s2_sth.s_pp_earn5 = t_double;
	s2_sth.s_no_pp5 = t_short;
	s2_sth.s_pp_earn6 = t_double;
	s2_sth.s_no_pp6 = t_short;
	s2_sth.s_pp_earn7 = t_double;
	s2_sth.s_no_pp7 = t_short;
	s2_sth.s_pp_earn8 = t_double;
	s2_sth.s_no_pp8 = t_short;
	s2_sth.s_pp_earn9 = t_double;
	s2_sth.s_no_pp9 = t_short;
	s2_sth.s_pp_earn10 = t_double;
	s2_sth.s_no_pp10 = t_short;
	s2_sth.s_pp_earn11 = t_double;
	s2_sth.s_no_pp11 = t_short;
	s2_sth.s_pp_earn12 = t_double;
	s2_sth.s_no_pp12 = t_short;
	s2_sth.s_pp_earn13 = t_double;
	s2_sth.s_no_pp13 = t_short;
	s2_sth.s_pp_earn14 = t_double;
	s2_sth.s_no_pp14 = t_short;
	s2_sth.s_pp_earn15 = t_double;
	s2_sth.s_no_pp15 = t_short;
	s2_sth.s_pp_earn16 = t_double;
	s2_sth.s_no_pp16 = t_short;
	s2_sth.s_pp_earn17 = t_double;
	s2_sth.s_no_pp17 = t_short;
	s2_sth.s_pp_earn18 = t_double;
	s2_sth.s_no_pp18 = t_short;
	s2_sth.s_pp_earn19 = t_double;
	s2_sth.s_no_pp19 = t_short;
	s2_sth.s_pp_earn20 = t_double;
	s2_sth.s_no_pp20 = t_short;

	if(t_short == HV_SHORT){
		s2_sth.s_total = t_double;
	}
	s2_sth.s_ins_wks = t_short;
	s2_sth.s_vac_pay = t_double;
	s2_sth.s_all_wks_max[0] = t_char;

	s2_sth.s_stat_date1 = t_long;
	s2_sth.s_stat_amt1 = t_double;
	s2_sth.s_reason1[0] = t_char;
	s2_sth.s_reas_amt1 = t_double;
	s2_sth.s_stat_date2 = t_long;
	s2_sth.s_stat_amt2 = t_double;
	s2_sth.s_reason2[0] = t_char;
	s2_sth.s_reas_amt2 = t_double;
	s2_sth.s_stat_date3 = t_long;
	s2_sth.s_stat_amt3 = t_double;
	s2_sth.s_reason3[0] = t_char;
	s2_sth.s_reas_amt3 = t_double;

	s2_sth.s_all_pp[0] = t_char;

	return(NOERROR) ;
}	/* InitFields() */
/*-----------------------------------------------------------------------
Validate flds when PROFOM returns RET_VAL_CHK 
-----------------------------------------------------------------------*/
static
Validate()	
{
	int	retval;
	
	switch(sr.curfld){
	case	MAX_EARN:
		if(s2_sth.s_all_wks_max[0] != 'N' &&
		   s2_sth.s_all_wks_max[0] != 'Y'){
		        fomen(YORN);
			s2_sth.s_all_wks_max[0] = LV_CHAR;
			return(-1);
		}
		break;

	case	ALL_FP:
		if(s2_sth.s_all_pp[0] != 'N' &&
		   s2_sth.s_all_pp[0] != 'Y'){
		        fomen(YORN);
			s2_sth.s_all_pp[0] = LV_CHAR;
			return(-1);
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
	}

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

/*-----------------------------------------------------------------------
Changing fields. Accept fld to be changed and read that fld 
-----------------------------------------------------------------------*/
static int
ChangeFields()
{
	int	i, retval ;
	int	fld_no, end_fld;

	SetDupBuffers(PP_EARN1,MAX_EARN,1); /* Off Dup Control */
	SetDupBuffers(STAT_HOL1,END_FLD2,1); /* Off Dup Control */

	/* Get The Field to Be Modified */
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
		if (retval == RET_USER_ESC) break;	/* User enter ESC-F */

       		if (s2_sth.s_field2 == 0 ) break;

		if (s2_sth.s_field2 > MAX_FIELD2) {
			fomen("Invalid Field Number");
			get();
			continue;
		}
		if(s2_sth.s_field2 <= 20)
			fld_no = (PP_EARN1) + (200 * (s2_sth.s_field2-1));
		else{
			if(s2_sth.s_field2 ==21){
				fld_no = TOTAL+100+((s2_sth.s_field2 - 21)*100);
				end_fld = fld_no;
			}
			else{
			  if(s2_sth.s_field2 ==22)
				fld_no = TOTAL+200+((s2_sth.s_field2 - 22)*100);
			  else{
			    if(s2_sth.s_field2 ==23)
			       fld_no = TOTAL+300+((s2_sth.s_field2 - 23)*100);
			    else
			       fld_no = 3200 + (100 * (s2_sth.s_field2-1));
			  }
			}
		}

		if(s2_sth.s_field2 > 40)
			fld_no += 300;
		if(s2_sth.s_field2 <= 20){
			end_fld = fld_no+100;
			retval = ReadFields((char *)&s2_sth,fld_no, fld_no,
				Validate, WindowHelp,1) ;

			retval = ReadFields((char *)&s2_sth,end_fld, end_fld,
				Validate, WindowHelp,1) ;
		}
		else{
			end_fld = fld_no;

			retval = ReadFields((char *)&s2_sth,fld_no, end_fld,
				Validate, WindowHelp,1) ;
		}
	}

     	s2_sth.s_field2 = HV_SHORT ;
	if ( WriteFields((char *)&s2_sth,CHG_FLD2, CHG_FLD2) < 0 ) return(-1);

     	s2_sth.s_mesg2[0] = HV_CHAR ;
     	DispMesgFld((char *)&s2_sth);

	if(SetDupBuffers(PP_EARN1, NO_PP20, 0)<0) return(PROFOM_ERR);

	retval = TotalPayPer();
	if(WriteFields((char*)&s2_sth,TOTAL,TOTAL)<0) return(-1);

	return(NOERROR);
}	/* ChangeFields() */

/*----------------------------------------------------------------*/
/* Set Duplication buffers for fields 				  */
static
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
	fomud( (char *)&s2_sth );
	ret( err_chk(&sr) );

	return( 0 );
}
/*-----------------------------------------------------------------------*/
static
TotalPayPer()
{
	int	i,j;

	s2_sth.s_total = 0;

	s2_sth.s_total += s2_sth.s_pp_earn1;
	s2_sth.s_total += s2_sth.s_pp_earn2;
	s2_sth.s_total += s2_sth.s_pp_earn3;
	s2_sth.s_total += s2_sth.s_pp_earn4;
	s2_sth.s_total += s2_sth.s_pp_earn5;
	s2_sth.s_total += s2_sth.s_pp_earn6;
	s2_sth.s_total += s2_sth.s_pp_earn7;
	s2_sth.s_total += s2_sth.s_pp_earn8;
	s2_sth.s_total += s2_sth.s_pp_earn9;
	s2_sth.s_total += s2_sth.s_pp_earn10;

	s2_sth.s_total += s2_sth.s_pp_earn11;
	s2_sth.s_total += s2_sth.s_pp_earn12;
	s2_sth.s_total += s2_sth.s_pp_earn13;
	s2_sth.s_total += s2_sth.s_pp_earn14;
	s2_sth.s_total += s2_sth.s_pp_earn15;
	s2_sth.s_total += s2_sth.s_pp_earn16;
	s2_sth.s_total += s2_sth.s_pp_earn17;
	s2_sth.s_total += s2_sth.s_pp_earn18;
	s2_sth.s_total += s2_sth.s_pp_earn19;
	s2_sth.s_total += s2_sth.s_pp_earn20;

	return(NOERROR);
}
/*------------------    END OF FILE   ------------------------*/
