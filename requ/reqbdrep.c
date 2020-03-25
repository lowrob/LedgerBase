/*-----------------------------------------------------------------------
Source Name: reqbdrep.c
System     : Budgetary Financial system.
Module     : Requisition system.
Created  On: 25th Mar 91.
Created  By: Steven Osborne.

MakeFile :	make -f makebdrep

DESCRIPTION:
	Program to Print Budget Reports. This program provides selection of
	report and its options. For a selected report corresponding function
	will be called. And that function first calls options selection function
	GetOptions(), which is available here. These functions should return
	either QUIT or DBH_ERR or ERROR or NOERROR or RET_USER_ESC.

		QUIT - While displaying report user selected quit report
		       option.

		DBH_ERR - Severe Read error in getting records from DBH.

		ERROR - Error while printing report.

		RET_USER_ESC - User terminated the report selection process
			either by typing ESC-F or by not confirming.

Usage of SWITCHES when they are ON :
	SW1 :
	SW2 :
		Not Used.
	SW3 :

	SW4 :
		Add Commitments to Period Actual.
	SW5 :
		Add Commitments to Cumulative Actual.
	SW6 :
		Not Used.
	SW7 :	(COMPANY)
		System Installed for Companies.
	SW8 :
		Not Used.


MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

L.Robichaud	1993/10/13	Pass new parameter to close_rep function for 
				a banner to print with 3000 family.
------------------------------------------------------------------------*/

#define	MAIN	/* Main program. This is to declare Switches */
#define MAINFL		-1	/* no main file used */

#define	SYSTEM		"REQUISITIONS"		/* Sub System Name */
#define	MOD_DATE	"25-MAR-91"		/* Program Last Modified */

#include <stdio.h>
#include <cfomstrc.h>
#include <bdgt_rep.h>	/* Reports Variables & Definitions */

#define	SCR_NAME	"reqbdrep"	/* PROFOM screen name	*/

#ifdef ENGLISH
#define PRINTER	'P'
#define DISPLAY 'D'
#define FILE_IO	'F'
#define SPOOL	'S'
#else
#define PRINTER 'I'
#define DISPLAY	'A'
#define FILE_IO	'D'
#define SPOOL	'B'
#endif
/* NOTE: see bdgt_rep.h for all common definitions */

extern	short	TitleSort ;	/* YES if titles sort is done */

main(argc,argv)
int argc;
char *argv[];
{
	if(argc < 2){
#ifdef  DEVELOP
		printf("MAIN ARGUMENTS ARE NOT PROPER\n");
		printf("Usage : %s {-tterminal name}\n", argv[0]);
#endif
		exit(1);
	}

	strncpy(SYS_NAME,SYSTEM,50);	/* Sub system name */
	strncpy(CHNG_DATE,MOD_DATE,10);	/* Modification Date */

	proc_switch(argc, argv, MAINFL) ; 	/* Process Switches */

	STRCPY(sr.termnm,terminal);	/* Copy Terminal Name */

	ret(InitProfom()) ;			/* Initialize PROFOM */

	ret_cd = get_param(&pa_rec, BROWSE, 1, e_mesg) ;
	if(ret_cd == ERROR)
		DispError(e_mesg);
	else if(ret_cd == UNDEF)
#ifdef ENGLISH
		DispError("Parameters Are Not Setup..");
#else
		DispError("Parametres ne sont pas etablis..");
#endif
	else {
		ret_cd = Process(); 	/* Initiate Process */
		RemoveFiles() ;		/* Remove Temporary sort files */
	}

	/* Set terminal back to normal mode from PROFOM */
	fomcs();
	fomrt();

	close_dbh();	/* Close files */

	if(ret_cd != NOERROR)exit(1);
	exit(0);
} /* END OF MAIN */
/*-------------------------------------------------------------------*/
/* Initialize PROFOM */
InitProfom()
{
	fomin(&sr);
	/* Check for Error */
	if(err_chk(&sr) == PROFOM_ERR){
		fomcs();
		fomrt();
		return(ERROR);
	}
	fomcf(1,1);	/* Enable Snap screen option */
	fomcf(2,0);	/* Fomwr should not update dup buffers */

	/* Initialize Screen */
	STRCPY(sr.scrnam,NFM_PATH);
	strcat(sr.scrnam,SCR_NAME);

	STRCPY(rp_sth.s_pgm,PROG_NAME);
	rp_sth.s_rundt = get_date();	/* get today's date in YYYYMMDD format*/

	ret(InitScreen(HV_CHAR,HV_SHORT)) ;

	return(NOERROR) ;
}	/* InitProfom() */
/*-----------------------------------------------------------------------*/ 
/* Display available reports & take the choice from user */
static	int
Process()
{
int option;
	for( ; ; ){
#ifdef	ENGLISH
		fomer("Select Desired Report else give '0' to Exit");
#else
		fomer("Choisir rapport desire, sinon entrer '0' pour sortir");
#endif
		sr.nextfld = OPTION_FLD ;
		fomrf((char*)&rp_sth);
		ret(err_chk(&sr)) ;

		/* Select function correspong to Selected report */
		option = atoi(rp_sth.s_option);
		switch(option){
		case 0 :
			return(NOERROR);
		case 1 :	/* Distibution - Budget Report */
		case 2 :	/* Distibution - Actual Report */
			if((ret_cd=CheckAccess(GLMAST,BROWSE,e_mesg))<0)
				break;
			ret_cd = BdgtDistribution(option+3);
			break;
		case 3 :	/* Summary - Format A */
		case 4 :	/* Summary - Format B */
		case 5 :	/* Summary - Expenses Report */
		case 6 :	/* Summary - Revenue Report */
		case 7 :	/* Summary - Expenses & Revenue Report */
			if((ret_cd=CheckAccess(GLMAST,BROWSE,e_mesg))<0)
				break;
			ret_cd = BdgtSummary(option+3);
			break;
		default :
			continue ;
		}
		if(ret_cd == NOACCESS) {
			fomen(e_mesg);
			get();
			continue;
		}
		if(ret_cd == PROFOM_ERR) return(ret_cd) ;
		if(ret_cd == DBH_ERR) {
			DispError(e_mesg);
#ifdef ENGLISH
			sprintf(e_mesg,"%s %d Dberror: %d Errno: %d",
				"SYSTEM ERROR... Iserror:",
				iserror, dberror, errno);
#else
			sprintf(e_mesg,"%s %d Dberror: %d Errno: %d",
				"ERREUR DE SYSTEME... Iserror:",
				iserror, dberror, errno);
#endif
			DispError(e_mesg);
			return(DBH_ERR); /* DBH ERROR */
		}

		EndReport(ret_cd);
		if(ret_cd == ERROR)return(ERROR);
	}	/* getting report number */
}
/*-----------------------------------------------------------------*/
static	int
EndReport(code)	/* End of the Report */
int	code;
{
	int	i;

	if(code == QUIT || code == NOERROR){
		if(pgcnt == 0){
			fomce();
			if(code != QUIT) {
#ifdef	ENGLISH
				fomen("No Records Available. Press RETURN<CR>");
#else
				fomen("Pas de fiches disponibles. appuyer sur RETURN<CR>");
#endif
				get() ;
			}
		}
		else {
			if(rp_sth.s_output[0] == DISPLAY){
				if(code != QUIT)
					last_page();
				fomst() ;
				fomcs() ;
			}
#ifndef	SPOOLER
			else if(code != QUIT)
				rite_top() ;	/* Generate FF after report */
#endif
		}
	} else if(code == ERROR){
		/* error message printed wait for user response */
		fomst();
		get();
	}

	close_rep(BANNER);		/* Close Report file */

	sr.nextfld = 1 ;
	sr.endfld = 0 ;
	fomwr((char*)&rp_sth);
	ret(err_chk(&sr));

	/* Update the dup buffers of options */
	/* Turn on The Duplicate buffers */
	for(i = OPTION_START ; i <= OPTION_END ; i += 100 )
		fomca1(i,19,2);

	sr.nextfld = OPTION_START;
	sr.endfld = OPTION_END;
	fomud((char*)&rp_sth);
	ret(err_chk(&sr));

	ret(InitScreen(HV_CHAR,HV_SHORT));	/* Display the screen */

	return(NOERROR);
}
/*-----------------------------------------------------------------*/
GetOptions()	/* get the Report Control option */
{
	int	err ;

#ifdef	ENGLISH
	STRCPY(rp_sth.s_mesg,"Press ESC-F To Go Back To Option:");
#else
	STRCPY(rp_sth.s_mesg,"Appuyer sur ESC-F pour retourner a option:");
#endif
	ShowMesg();

	InitScreen(LV_CHAR,LV_SHORT) ;	/* Set Low Values */

	/* Set Defaults for screen */
	SetDefaults();
	
	sr.nextfld = OPTION_START;
	sr.endfld = OPTION_END;

	for( ; ;){
		fomrd((char*)&rp_sth);
		ret(err_chk(&sr));
		if(sr.retcode == RET_VAL_CHK){
			if((err = Validate()) != NOERROR) return(err) ;
			continue;
		}
		if(sr.retcode == RET_USER_ESC) {
			if(sr.escchar[0] == 'f' || sr.escchar[0] == 'F') {
				HideMesg() ;
				return(RET_USER_ESC);
			}
			continue;
		}
		break;
	}

	cur_pos = 0;
	linecnt = PGSIZE;
	LNSZ = 133 ;
	pgcnt = 0;

	return(Confirm()) ;
}
/*-----------------------------------------------------------------------*/ 
SetDefaults()
{
	int i;

	/* Default Budget Keys */
	for(i=0;i<USER_KEYS;i++) 
		fomca1(SORT_KEYS+(100*i),19,2);

	rp_sth.s_sort_keys[0] = pa_rec.pa_bdgt_key1;
	rp_sth.s_sort_keys[1] = pa_rec.pa_bdgt_key2;
	rp_sth.s_sort_keys[2] = pa_rec.pa_bdgt_key3;
	rp_sth.s_sort_keys[3] = pa_rec.pa_bdgt_key4;
	rp_sth.s_sort_keys[4] = pa_rec.pa_bdgt_key5;
	rp_sth.s_sort_keys[5] = pa_rec.pa_bdgt_key6;
	rp_sth.s_sort_keys[6] = pa_rec.pa_bdgt_key7;
	
	sr.nextfld = SORT_KEYS;
	sr.endfld = SORT_KEYS+600;
	fomud((char*)&rp_sth);
	for(i=0;i<USER_KEYS;i++) 
		rp_sth.s_sort_keys[i] = LV_SHORT;
		
	/* Default OutPut on */
	fomca1(OUTPUT_ON_FLD,19,2);
	rp_sth.s_output[0] = PRINTER;
	sr.nextfld = OUTPUT_ON_FLD;
	sr.endfld = OUTPUT_ON_FLD;
	fomud((char*)&rp_sth);
	rp_sth.s_output[0] = LV_CHAR;

	/* Default starting fund */
	fomca1(FUND_FM_FLD,19,2);
	fomca1(FUND_TO_FLD,19,2);
	rp_sth.s_fm_fund = 1;
	rp_sth.s_to_fund = 999;
	sr.nextfld = FUND_FM_FLD;
	sr.endfld = FUND_TO_FLD;
	fomud((char*)&rp_sth);
	rp_sth.s_fm_fund = LV_SHORT;
	rp_sth.s_to_fund = LV_SHORT;

	/* Default Period */
	fomca1(PERIOD_FLD,19,2);
	rp_sth.s_period = pa_rec.pa_cur_period;
	sr.nextfld = PERIOD_FLD;
	sr.endfld = PERIOD_FLD;
	fomud((char*)&rp_sth);
	rp_sth.s_period = LV_SHORT;

	/* Default cost center range */
	if (CC_no != pa_rec.pa_distccno)
	{
		rp_sth.s_fm_ccno = CC_no;
		rp_sth.s_to_ccno = rp_sth.s_fm_ccno;
	}
	else
	{
		fomca1(CC_FM_FLD,19,2);
		fomca1(CC_TO_FLD,19,2);
		rp_sth.s_fm_ccno = 0;
		rp_sth.s_to_ccno = 99999;
		sr.nextfld = CC_FM_FLD;
		sr.endfld = CC_TO_FLD;
		fomud((char*)&rp_sth);
		rp_sth.s_fm_ccno = LV_LONG;
		rp_sth.s_to_ccno = LV_LONG;
	}

}	/* SetDefaults() */
/*-----------------------------------------------------------------------*/ 
static int
Validate()	/* validates given input */
{
	short	i, j ;

	switch(sr.curfld){
	case SORT_KEYS :	/* Keys 1 */
	case SORT_KEYS + 100 :	/* Keys 2 */
	case SORT_KEYS + 200 :	/* Keys 3 */
	case SORT_KEYS + 300 :	/* Keys 4 */
	case SORT_KEYS + 400 :	/* Keys 3 */
	case SORT_KEYS + 500 :	/* Keys 6 */
	case SORT_KEYS + 600 :	/* Keys 7 */
		i = (sr.curfld - SORT_KEYS) / 100 ;

		if((i == 0 && rp_sth.s_sort_keys[i] < 1) ||
				rp_sth.s_sort_keys[i] > NO_KEYS) {
#ifdef ENGLISH
			fomer("Should Be Between 1 and 12") ;
#else
			fomer("Devrait etre entre 1 et 12") ;
#endif
			rp_sth.s_sort_keys[i] = LV_SHORT ;
			break;
		}

		/* Atleast one key has to be given */
		if(i && rp_sth.s_sort_keys[i] == 0) {
			for(j = i ; j < USER_KEYS ; j++)
				rp_sth.s_sort_keys[j] = 0 ;
			break ;
		}
		/* Check whether the given key is already given */
		if(i){	/* 2nd key onwards */
			for(j = 0 ; j <= (i - 1) ; j++)
			    if(rp_sth.s_sort_keys[i] == rp_sth.s_sort_keys[j]) {
#ifdef ENGLISH
				fomer("Already Given");
#else
				fomer("Deja donne");
#endif
				rp_sth.s_sort_keys[i] = LV_SHORT ;
				break;
			    }
		}
		if(sr.curfld == SORT_KEYS + 600 && 
			rp_sth.s_sort_keys[i] != LV_SHORT) {
#ifdef	ENGLISH
			fomer("D(isplay), P(rinter), F(ile)");
#else
			fomer("A(fficher), I(mprimante), D(ossier)");
#endif
		}
		break;
	case OUTPUT_ON_FLD:	/* Output ON */
		switch(rp_sth.s_output[0]){
		case PRINTER :
			rp_sth.s_output[0] = 'P';
			if(opn_prnt(rp_sth.s_output, "\0", 1, e_mesg, 1) < 0){
				DispError(e_mesg);
				return(ERROR) ;
			}
			PGSIZE = 60;		/* set lines per page to 60 */
			rp_sth.s_output[0] = PRINTER;
			/* Default Number of Copies */
			fomca1(COPIES_FLD,19,2);
			rp_sth.s_copies = 1;
			sr.nextfld = COPIES_FLD;
			sr.endfld = COPIES_FLD;
			fomud((char*)&rp_sth);
			rp_sth.s_copies = LV_SHORT;
			break ;
		case DISPLAY :
			rp_sth.s_output[0] = 'D';
			if(opn_prnt(rp_sth.s_output,terminal,0,e_mesg,1) < 0){
				fomer(e_mesg);
				rp_sth.s_output[0] = LV_CHAR;
				break ;
			}
			rp_sth.s_output[0] = DISPLAY;
			break;
		case FILE_IO :
			rp_sth.s_filenm[0] = LV_CHAR;
			break ;
		case SPOOL :
			sprintf(rp_sth.s_filenm,"spool%d",CC_no);
			rp_sth.s_output[0] = 'F';
			if(opn_prnt(rp_sth.s_output,rp_sth.s_filenm,0,e_mesg,1) < 0){
				fomer(e_mesg);
				rp_sth.s_filenm[0] = LV_CHAR;
				break;
			}
			rp_sth.s_output[0] = FILE_IO;
			PGSIZE = 60;		/* set lines per page to 60 */
			break ;
		default  :
#ifdef	ENGLISH
			fomer("D(isplay), P(rinter), F(ile), S(pool)");
#else
			fomer("A(fficher), I(mprimante), D(ossier), B(obiner)");
#endif
			rp_sth.s_output[0] = LV_CHAR;
		}
		break ;
	case COPIES_FLD:	/* No of Copies */
		if(rp_sth.s_copies < 1) {
#ifdef ENGLISH
			fomer("Can't Be Zero");
#else
			fomer("Ne peut pas etre zero");
#endif
			rp_sth.s_copies = LV_SHORT;
			break;
		}
		SetCopies( (int)rp_sth.s_copies) ;
		break;
	case FILE_FLD:	/* Filename */
		if(rp_sth.s_filenm[0] == '\0') {
			rp_sth.s_filenm[0] = LV_CHAR ;
			break ;
		}

		rp_sth.s_output[0] = 'F';
		if(opn_prnt(rp_sth.s_output,rp_sth.s_filenm,0,e_mesg,1) < 0){
			fomer(e_mesg);
			rp_sth.s_filenm[0] = LV_CHAR;
			break;
		}
		rp_sth.s_output[0] = FILE_IO;
		PGSIZE = 60;		/* set lines per page to 60 */
		break;
	case FUND_TO_FLD :	/* Fund To */
		if( rp_sth.s_to_fund < rp_sth.s_fm_fund ||
				rp_sth.s_fm_fund < 1){
#ifdef ENGLISH
			fomer("Invalid Range");
#else
			fomer("Excede les limites");
#endif
			rp_sth.s_fm_fund = LV_SHORT ;
			rp_sth.s_to_fund = LV_SHORT ;
			sr.curfld = FUND_TO_FLD - 100 ;
		}
		break ;
	case PERIOD_FLD :	/* Period: */
		if(rp_sth.s_period < 1 || rp_sth.s_period > NO_PERIODS ||
				rp_sth.s_period > pa_rec.pa_no_periods) {
#ifdef ENGLISH
			fomer("Invalid Period");
#else
			fomer("Periode invalide");
#endif
			rp_sth.s_period = LV_SHORT ;
		}
		break ;
	case SORT_NO_FLD :	/* Sort#: */
		if(rp_sth.s_sort_no < 1 || rp_sth.s_sort_no > SUM_SORTS) {
#ifdef ENGLISH
			fomer("Invalid Sort#");
#else
			fomer("# de tri invalide");
#endif
			rp_sth.s_sort_no = LV_SHORT ;
		}
		break ;
	case CC_TO_FLD :	/* Cost Center to: */
		if( rp_sth.s_to_ccno < rp_sth.s_fm_ccno){
#ifdef ENGLISH
			fomer("Invalid Range");
#else
			fomer("Excede les limites");
#endif
			rp_sth.s_fm_ccno = LV_LONG ;
			rp_sth.s_to_ccno = LV_LONG ;
			sr.curfld = CC_TO_FLD - 100 ;
		}
		break ;
	case KEY8_TO_FLD :	/* Key 8's to: */
		if( rp_sth.s_to_key8 < rp_sth.s_fm_key8){
#ifdef ENGLISH
			fomer("Invalid Range");
#else
			fomer("Excede les limites");
#endif
			rp_sth.s_fm_key8 = LV_LONG ;
			rp_sth.s_to_key8 = LV_LONG ;
			sr.curfld = KEY8_TO_FLD - 100 ;
		}
		break ;
	default :
#ifdef ENGLISH
		sprintf(e_mesg,"No Validity Check For Field#  %d",sr.curfld);
#else
		sprintf(e_mesg,"Pas de controle de validite pour le champ#  %",sr.curfld);
#endif
		fomen(e_mesg);
		get();
	}
	sr.nextfld = sr.curfld;
	return(NOERROR);
}
/*-----------------------------------------------------------------------*/ 
/* Confirm Selection */
static	int
Confirm()
{
	int	i;

#ifdef ENGLISH
	STRCPY(rp_sth.s_mesg,"Confirm (Y/N)?");
#else
	STRCPY(rp_sth.s_mesg,"Confirmer (O/N)?");
#endif
	rp_sth.s_resp = LV_INT ;
	sr.nextfld = END_FLD - 100 ;
	sr.endfld = END_FLD ;
	fomrd((char*)&rp_sth);

	i = rp_sth.s_resp ;

	if(i) {
#ifdef ENGLISH
		STRCPY(rp_sth.s_mesg,"Sorting Files.... Please Wait...");
#else
		STRCPY(rp_sth.s_mesg,"Trie les dossiers... Attendez S.V.P.");
#endif
		ShowMesg() ;
		fflush(stdout);
		ret_cd = CreatTitles() ; /* Creat Titles & Sort If Necessary */
		if(ret_cd != NOERROR) {
			DispError(e_mesg) ;
			return(DBH_ERR) ;
		}
#ifdef ENGLISH
		STRCPY(rp_sth.s_mesg,"Compiling Report.... Please Wait...");
#else
		STRCPY(rp_sth.s_mesg,"Compile le rapport... Attendez S.V.P.");
#endif
	}
	else
		rp_sth.s_mesg[0] = HV_CHAR ;
	rp_sth.s_resp = HV_INT ;
	sr.nextfld = END_FLD - 100 ;
	sr.endfld = END_FLD ;
	fomwr((char*)&rp_sth);

	if(i == 0) return(RET_USER_ESC) ;

	fflush(stdout);
		
	return(NOERROR) ;
}
/*------------------------------------------------------*/
/* move HVs to report options & display the screen */
static int
InitScreen(t_char,t_short)
char	t_char ;
short	t_short ;
{
	int	i,option;

	if(TitleSort == 0)
		for(i = 0 ; i < USER_KEYS ; i++)
			rp_sth.s_sort_keys[i] = LV_SHORT ;

	rp_sth.s_output[0] = t_char ;

	rp_sth.s_copies = HV_SHORT ;
	rp_sth.s_filenm[0] = HV_CHAR ;

	rp_sth.s_fm_fund = t_short ;
	rp_sth.s_to_fund = t_short ;

	rp_sth.s_period = t_short ;

	if(t_short == LV_SHORT) {
		option = atoi(rp_sth.s_option);
		if(option >= 3)
			rp_sth.s_sort_no = LV_SHORT ;

		rp_sth.s_fm_ccno = LV_LONG ;
		rp_sth.s_to_ccno = LV_LONG ;
	}
	else {
		rp_sth.s_sort_no = HV_SHORT ;

		rp_sth.s_fm_ccno = HV_LONG ;
		rp_sth.s_to_ccno = HV_LONG ;

		rp_sth.s_fm_key8 = HV_LONG ;
		rp_sth.s_to_key8 = HV_LONG ;

		rp_sth.s_mesg[0] = HV_CHAR;
		rp_sth.s_resp = HV_INT;

		sr.nextfld = 1 ;
		sr.endfld = 0 ;
		fomwr((char*)&rp_sth);
		ret(err_chk(&sr));
	}

	return(NOERROR);
}
/*-------------------------------------------------------------------------*/
static	int
DispError(s)	/* show ERROR and wait */
char *s;
{
	STRCPY(rp_sth.s_mesg,s);
	ShowMesg();
#ifdef	ENGLISH
	fomen("Press any key to continue");
#else
        fomen("Appuyer sur une touche pour continuer");
#endif
	get();
	HideMesg() ;
	return(ERROR);
}
/*-----------------------------------------------------------*/
static int	/* Hide message field */
HideMesg()
{
	rp_sth.s_mesg[0] = HV_CHAR ;
	ShowMesg() ;
}	/* HideMesg() */
/*-----------------------------------------------------------*/
static int	/* Display message field */
ShowMesg()
{
	sr.nextfld = END_FLD - 100;
	fomwf((char*)&rp_sth);
}	/* ShowMesg() */
/*-------------------- E n d   O f   P r o g r a m ---------------------*/
