/*-------------------------------------------------------------------
	Source Name: hlp_win.c
	Created By : T Amarendra.
	Created On : 17-SEP-88.

	Displays help windows with relevent info against item number 1 to 9.
	User can select either an item number indicating the choise or
	give return to get the next set of values in the window. 'Q' will exit
	from window without any selection.

	RETURN VALUES:
	--------------
	Return value 0,1,-1 are to be checked by the user for appropirate
	action.

	Returns :  1 - For successful display of window and selection.
		   0 - Window was displayed but No selection made by user.
		   -1 -No window was displayed so no quesion of selection.

	For DBH or ISAM erros returns DBH_ERR.

	NOTE:
	----- 
		The routines for displaying the help windows read the respective
	dbh file in BROWSE mode. This involves a risk of unlocking any records
	previously locked by the same process. 
		A solution to this is implemented by locking the previously 
	locked record ( if locked ) again.  Function SettleLock() detects the
	lock and relocks the record.
		The detection of lock is possible only if Journalling is ON.
		If journalling is OFF, there is no way to know if a record is
	previously locked, and so the risk mentioned above remains.

-----------------------------------------------------------------------------*/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>
#include <fld_defs.h>
#include <cfomstrc.h>

#ifndef	MS_DOS		/* If Not Ms_dos */
#define	O_PROFOM
#endif

#ifdef ENGLISH
#define U_YES		'Y'
#define L_YES		'y'
#define U_QUIT		'Q'
#define L_QUIT		'q'
#else
#define U_YES		'O'
#define L_YES		'o'
#define U_QUIT		'R'
#define L_QUIT		'r'
#endif

/******************* 	Screen Constants & Variables	*****************/

#define MAX_LINES 	10		/* maximun lines in window */
#define WIDTH	(TOT_STRNG+6)	/* Width of a window */
#define	MAX_STRING	57	/* Largest TOT_STRNG value in this file.
				   Update this when assigned value of
				   TOT_STRNG changes.	SBO	910225	*/
/* This is the most text that will be stored in the array: line
	SBO	910225						*/
#define	MAX_TEXT	(MAX_LINES+1)*(MAX_STRING+1)+1

static	int	TOT_STRNG;

static	int	max_val;		/* No of rows in current window */
static	int	height,
		put_blanks;

static	char	*text ; 
static	char	line[MAX_TEXT] ;	/* max chars in window */
static	char	err_msg[100];
static	int	code;
static	char	kee_name[50];

/********************   Terminal Constants 	****************/

#define T_DEL	8				/* Backspace*/
#define T_TAB	9				/* Tab */
#define T_NL	10 				/* New line */
#define T_CRF   13 				/* Return   */

/*-----------------------------------------------------------------------
	Usage :   reqacnt_hlp   (fund_cd,acnt_no,rec_cd,cc#,gl cc key#,row,col)
		  fund_cd	; For this fund code help is displayed.
		  acnt_no	; Selected account# will be moved to this.
		  rec_cd	; selected record code will be moved to this.
		  row, col 	; self explanatory .
	This help window in only so accounts for a given school will
	only be displayed .
	Added by J.Prescott Mar. 15/91.
------------------------------------------------------------------------*/

reqacnt_hlp(fund_cd,acnt_no,rec_cd,cc_no,cckey,row,col) 
short	fund_cd ;
char	*acnt_no ;
short	*rec_cd ;
short	cc_no ;
long	cckey ;
int	row,
	col;  	/* window parameters */
{
 	int 		i,k ; 
	Gl_rec		g_rec;
	char		t_acnt[MAX_LINES][19] ;
	short		t_reccod[MAX_LINES] ;
	int		key_no ;

	int	on_desc = 0 ;	/* If Yes help on description */

	TOT_STRNG = 57 ;

	if( !on_desc ) {	/* If Not on Description */
#ifdef ENGLISH
		get_name(row, col, 1, "Help on Desc(Y/N)?  ", kee_name, 0);
#else
		get_name(row, col, 1, "Assistance sur Desc.(O/N)?   ", kee_name, 0);
#endif
		if(kee_name[0] == L_YES || kee_name[0] == U_YES)
			on_desc = 1;
                if (kee_name[0] == NULL) on_desc = 1;
	}
	else {
#ifdef ENGLISH
		get_name(row, col, 1, "Help on Acct#(Y/N)? ", kee_name, 0);
#else
		get_name(row, col, 1, "Assistance sur #compte(O/N)? ", kee_name, 0);
#endif
		if(kee_name[0] == L_YES || kee_name[0] == U_YES)
			on_desc = 0;
	}

	/* Make The Starting key */
	g_rec.funds = fund_cd ;

	/* accept the starting key */
	if( on_desc) {
#ifdef ENGLISH
		get_name(row, col, DESC_KEY, "Starting Desc:  ", kee_name, 0);
#else
		get_name(row, col, DESC_KEY, "Desc. debutante:  ", kee_name, 0);
#endif
		strcpy(g_rec.desc,kee_name);
		key_no = 1;
	}
	else {
#ifdef ENGLISH
		get_name(row, col, 18, "Starting Acct#: ", kee_name, 0);
#else
		get_name(row, col, 18, "#Compte debutant: ", kee_name, 0);
#endif
		sprintf(g_rec.accno,"%18.18s", kee_name) ;
		g_rec.reccod = 0;
		key_no = 0;
	}

	/* Start the file */
	flg_reset(GLMAST);

#ifdef ENGLISH
	strcpy(line,"No    Account Number  RecCd  Description") ;
#else
	strcpy(line,"No    Numero compte  Cd Fiche  Description") ;
#endif
	expnd_line(line) ;
	height = 0;
	put_blanks = 1 ;

	code = get_n_gl(&g_rec, BROWSE, key_no, FORWARD, err_msg) ;

	for(;;){
		i = 1;
		max_val = 0;
		for( ; ;code = get_n_gl(&g_rec,BROWSE,key_no,FORWARD,err_msg)){

			if(ERROR == code )return(DBH_ERR);

			if(code == EFL)break;
#ifdef JRNL
			SettleLock( (char *)&g_rec, GLMAST, err_msg );
#endif

			if(g_rec.funds != fund_cd) {
				code = EFL ;
				break;
			}

	/* if cost center not 99 and key not matching cost centers key
	   skip record */
			if(g_rec.keys[cckey-1] != cc_no && cc_no != 99) 
				continue;

			if(i == (23 - row - 5) || i == MAX_LINES)break;

			strcpy(t_acnt[max_val],g_rec.accno );
			t_reccod[max_val] = g_rec.reccod ;

			text = line+i*(TOT_STRNG+1);

			sprintf(text, "%1d. %18.18s  %2d  %-30.30s",
				(max_val+1), g_rec.accno,
				g_rec.reccod, g_rec.desc );
			expnd_line(text) ;
			i++;
			max_val++;
		} 
#ifndef	ORACLE
		seq_over(GLMAST);
#endif
		if(put_blanks == 1 )
			if(i == 1){
#ifdef ENGLISH
				fomen("No Accounts to be Displayed ");
#else
				fomen("Pas d'articles a afficher ");
#endif
				/***
				return(-1);
				***/
				/* Returning 0 to redraw() the screen to wipe
				   out "Starting Name:" message */
				return(0);
			}
  		line[i*(TOT_STRNG+1)] = '\0';
		expnd_line(line+i*(TOT_STRNG+1));
		i++;
		line[i*(TOT_STRNG+1)] = '\0';

		if(i > height)height = i;

		text = line;
		if(window(row, col,WIDTH, (height+4), text, put_blanks ) < 0)
			return(0); 

		/* Get the user Response and Validate against
		   displayed Item Numbers ..**/

		if( code != EFL)
#ifdef ENGLISH
			k = acpt_resp("RETURN for more, #(Item), Q(uit):",
							row+i+2, col+4,1) ;
#else
			k = acpt_resp("RETURN pour plus, #(Article), R(etourner):",
							row+i+2, col+4,1) ;
#endif
		else
#ifdef ENGLISH
			k = acpt_resp("Select #(Item) Or Q(uit)  :",
							row+i+2, col+4,0) ;
#else
			k = acpt_resp("Choisir #(Article) ou R(etourner)  :",
							row+i+2, col+4,0) ;
#endif

		if ( k >= 1 && k < i-1 ){
			strcpy(acnt_no, t_acnt[k - 1]) ;
			*rec_cd = t_reccod[k - 1] ;
			return(1) ;
		}
		if(k < 0)return(0);
		put_blanks = 2 ;
	}
} 

/*-----------------------------------------------------------------------
	Help for ARS Debit and Credit memos.... 
	Displays accounts relevant to the given invoice only.
	Usage :   dm_cm_hlp  (fund_cd, invc, acnt_no, row, col )
		  fund_cd	; For this fund code help is displayed.
		  invc		; For this invoice help is displayed.
		  acnt_no	; Selected account# will be moved to this.
		  rec_cd	; selected record code will be moved to this.
		  row, col 	; self explanatory .
------------------------------------------------------------------------*/

dm_cm_hlp  (fund_cd, invc, acnt_no, row, col )
short	fund_cd ;
long	invc;
char	*acnt_no ;
int	row,
	col;  	/* window parameters */
{
 	int 		i,k ; 
	Ar_item		ar_item;
	Gl_rec		g_rec ;
	char		t_acnt[MAX_LINES][19], e_mesg ;
	int		retval;

	TOT_STRNG = 57 ;

	ar_item.ai_fund = fund_cd;
	ar_item.ai_inv_no = invc;
	ar_item.ai_hno = 1;
	ar_item.ai_sno = 0;
	/* Start the file */
	flg_reset(ARSITEM);

#ifdef ENGLISH
	strcpy(line,"No    Account Number  Description") ;
#else
	strcpy(line,"No    Numero de compte  Description") ;
#endif
	expnd_line(line) ;
	height = 0;
	put_blanks = 1 ;

#ifndef ORACLE
	code = get_n_aritem(&ar_item, BROWSE, 0, FORWARD, err_msg) ;
#else
	code = get_n_aritem(&ar_item, BROWSE, 0, EQUAL, err_msg) ;
#endif

	for(;;){
		i = 1;
		max_val = 0;
#ifndef ORACLE
		for( ; ;code = get_n_aritem(&ar_item,BROWSE,0,FORWARD,err_msg)){
#else
		for( ; ;code = get_n_aritem(&ar_item,BROWSE,0,EQUAL,err_msg)){
#endif
			if(ERROR == code )return(DBH_ERR);

			if(code == EFL)break;
#ifdef JRNL
			SettleLock( (char *)&ar_item, ARSITEM, err_msg );
#endif

#ifndef ORACLE
			if( ar_item.ai_fund != fund_cd ||
			    ar_item.ai_inv_no != invc ||
			    ar_item.ai_hno != 1 ||
			    ar_item.ai_sno < 1 ){
				code = EFL ;
				break;
			}
#endif

			g_rec.funds = fund_cd;
			strcpy( g_rec.accno, ar_item.ai_accno );
			g_rec.reccod = 99;
			retval = get_gl( &g_rec,BROWSE,0,e_mesg );
			if( retval!=NOERROR )
				return( DBH_ERR );

			if(i == (23 - row - 5) || i == MAX_LINES)break;

			strcpy(t_acnt[max_val],ar_item.ai_accno );

			text = line+i*(TOT_STRNG+1);

			sprintf(text, "%1d. %18.18s  %-30.30s",
				(max_val+1), ar_item.ai_accno, g_rec.desc );
			expnd_line(text) ;
			i++;
			max_val++;
		} 
#ifndef	ORACLE
		seq_over(ARSITEM);
#endif
		if(put_blanks == 1 ){
			if(i == 1){
#ifdef ENGLISH
				fomen("No Accounts to be Displayed ");
#else
				fomen("Pas de comptes a afficher ");
#endif
				/***
				return(-1);
				***/
				/* Returning 0 to redraw() the screen to wipe
				   out "Starting Name:" message */
				return(0);
			}
			if( code==EFL && i==2 ){ /* only one item */
				strcpy( acnt_no, t_acnt[0]);
				return(2);
			}
		}
  		line[i*(TOT_STRNG+1)] = '\0';
		expnd_line(line+i*(TOT_STRNG+1));
		i++;
		line[i*(TOT_STRNG+1)] = '\0';

		if(i > height)height = i;

		text = line;
		if(window(row, col,WIDTH, (height+4), text, put_blanks ) < 0)
			return(0); 

		/* Get the user Response and Validate against
		   displayed Item Numbers ..**/

		if( code != EFL)
#ifdef ENGLISH
			k = acpt_resp("RETURN for more, #(Item), Q(uit):",
							row+i+2, col+4,1) ;
#else
			k = acpt_resp("RETURN pour plus, #(Article), R(etourner):",
							row+i+2, col+4,1) ;
#endif
		else
#ifdef ENGLISH
			k = acpt_resp("Select #(Item) Or Q(uit)  :",
							row+i+2, col+4,0) ;
#else
			k = acpt_resp("Choisir #(Article) ou R(etourner)  :",
							row+i+2, col+4,0) ;
#endif

		if ( k >= 1 && k < i-1 ){
			strcpy(acnt_no, t_acnt[k - 1]) ;
			return(1) ;
		}
		if(k < 0)return(0);
		put_blanks = 2 ;
	}
} 

/*-----------------------------------------------------------------------
	Usage :   stock_hlp   (fund_cd, stck_cd, row, col )
		  fund_cd	; For this fund code help is displayed.
		  stck_cd	; Stock code
		  row, col 	; self explanatory .
------------------------------------------------------------------------*/

stock_hlp(fund_cd,stck_cd,row,col) 
short	fund_cd ;
char	*stck_cd ;
int	row,
	col;  	/* window parameters */
{
 	int 		i,k ; 
	char		t_code[MAX_LINES][11] ;
	St_mast		st_rec;
	int		key_no ;

	int	on_desc = 0 ;	/* If Yes help on description */

	TOT_STRNG = 47 ;

	if( !on_desc ) {	/* If Not on Description */
#ifdef ENGLISH
		get_name(row, col, 1, "Help on Desc(Y/N)? ", kee_name, 0);
#else
		get_name(row, col, 1, "Assistance sur Desc.(O/N)? ", kee_name, 0);
#endif
		if(kee_name[0] == L_YES || kee_name[0] == U_YES)
			on_desc = 1;
                if (kee_name[0] == NULL) on_desc = 1;
	}
	else {
#ifdef ENGLISH
		get_name(row, col, 1, "Help on Code(Y/N)? ", kee_name, 0);
#else
		get_name(row, col, 1, "Assistance sur Code(O/N)?  ", kee_name, 0);
#endif
		if(kee_name[0] == L_YES || kee_name[0] == U_YES)
			on_desc = 0;
	}

	/* Make The Starting key */
	st_rec.st_fund = fund_cd ;

	/* accept the starting key */
	if( on_desc) {
#ifdef ENGLISH
		get_name(row, col, DESC_KEY, "Starting Desc: ", kee_name, 0);
#else
		get_name(row, col, DESC_KEY, "Desc. debutante: ", kee_name, 0);
#endif
		strcpy(st_rec.st_desc,kee_name);
		key_no = 2;
	}
	else {
#ifdef ENGLISH
		get_name(row, col, 10, "Starting Code: ", kee_name, 0);
#else
		get_name(row, col, 10, "Code debutant:   ", kee_name, 0);
#endif
		strcpy( st_rec.st_code, kee_name );
		key_no = 0;
	}


	/* Start the file */
	flg_reset(STMAST);

#ifdef ENGLISH
	strcpy(line,"No  Stock_Code  Description") ;
#else
	strcpy(line,"No  Code_Stock  Description") ;
#endif
	expnd_line(line) ;
	height = 0;
	put_blanks = 1 ;

#ifndef ORACLE
	code = get_n_stmast(&st_rec, BROWSE, key_no, FORWARD, err_msg) ;
#else
	code = get_n_stmast(&st_rec, BROWSE, key_no, EQUAL, err_msg) ;
#endif

	for(;;){
		i = 1;
		max_val = 0;
#ifndef ORACLE
		for( ; ;code = get_n_stmast(&st_rec,BROWSE,key_no,FORWARD,err_msg)){
#else
		for( ; ;code = get_n_stmast(&st_rec,BROWSE,key_no,EQUAL,err_msg)){
#endif
			if(ERROR == code )return(DBH_ERR);

			if(code == EFL)break;
#ifdef JRNL
			SettleLock( (char *)&st_rec, STMAST, err_msg );
#endif

#ifndef ORACLE
			if(st_rec.st_fund != fund_cd) {
				code = EFL ;
				break;
			}
#endif

			if(i == (23 - row - 5) || i == MAX_LINES)break;

			strcpy(t_code[max_val],st_rec.st_code );

			text = line+i*(TOT_STRNG+1);

			sprintf(text, "%1d.  %-10.10s  %-31.31s",
				(max_val+1), st_rec.st_code, st_rec.st_desc);
			expnd_line(text) ;
			i++;
			max_val++;
		} 
#ifndef	ORACLE
		seq_over(STMAST);
#endif
		if(put_blanks == 1 )
			if(i == 1){
#ifdef ENGLISH
				fomen("No Codes to be Displayed ");
#else
				fomen("Pas de codes a afficher ");
#endif
				return(0);
			}
  		line[i*(TOT_STRNG+1)] = '\0';
		expnd_line(line+i*(TOT_STRNG+1));
		i++;
		line[i*(TOT_STRNG+1)] = '\0';

		if(i > height)height = i;

		text = line;
		if(window(row, col,WIDTH, (height+4), text, put_blanks ) < 0)
			return(0); 

		/* Get the user Response and Validate against
		   displayed Item Numbers ..**/

		if( code != EFL)
#ifdef ENGLISH
                        k = acpt_resp("RETURN for more, #(Item) or Q(uit):",
                                                        row+i+2, col+4,1) ;
#else
                        k = acpt_resp("RETURN pour plus, #(Article) ou R(etourner):",
                                                        row+i+2, col+4,1) ;
#endif
		else
#ifdef ENGLISH
			k = acpt_resp("Select #(Item) Or Q(uit)  :",
							row+i+2, col+4,0) ;
#else
			k = acpt_resp("Choisir #(Article) ou R(etourner)  :",
							row+i+2, col+4,0) ;
#endif

		if ( k >= 1 && k < i-1 ){
			strcpy(stck_cd, t_code[k - 1]) ;
			return(1) ;
		}
		if(k < 0)return(0);
		put_blanks = 2 ;
	}
} 

/*-----------------------------------------------------------------------
	Function to display stock section names 
	Usage :   sect_hlp   ( sect,row, col )
		  sect		pointer to section number
		  row, col 	; self explanatory .
------------------------------------------------------------------------*/

sect_hlp(sect,row,col) 
short	*sect;
int	row,
	col;  	/* window parameters */
{
 	int 		i,j,k ;
	short		t_sect[MAX_LINES];
	St_sect		st_rec;

	TOT_STRNG = 35 ;

#ifdef ENGLISH
	strcpy(line,"No  Sect    Description") ;
#else
	strcpy(line,"No  Sect    Description") ;
#endif
	expnd_line(line) ;
	height = 0;
	put_blanks = 1 ;
	j = 0;			/* section number */

	code = get_section(&st_rec, BROWSE, 1, err_msg) ;
	if( code<0 )	return(-1);
	for(;;){
		i = 1;
		max_val = 0;
		for( ; ;j++ ){
			if( j==st_rec.no_of_sections ){
				code = EFL ;
				break;
			}
			if(i == (23 - row - 5) || i == MAX_LINES)break;

			t_sect[max_val] = j+1;

			text = line+i*(TOT_STRNG+1);

			sprintf(text, "%2d.   %2d   %-20.20s",
				(max_val+1), j+1, st_rec.name[j] );
			expnd_line(text) ;
			i++;
			max_val++;
		} 
		if(put_blanks == 1 )
			if(i == 1){
#ifdef ENGLISH
				fomen("No Names to be Displayed ");
#else
				fomen("Pas de noms a afficher ");
#endif
				return(0);
			}
  		line[i*(TOT_STRNG+1)] = '\0';
		expnd_line(line+i*(TOT_STRNG+1));
		i++;
		line[i*(TOT_STRNG+1)] = '\0';

		if(i > height)height = i;

		text = line;
		if(window(row, col,WIDTH, (height+4), text, put_blanks ) < 0)
			return(0); 

		/* Get the user Response and Validate against
		   displayed Item Numbers ..**/

		if( code != EFL)
#ifdef ENGLISH
			k = acpt_resp("RETURN for more, #(Item), Q(uit):",
							row+i+2, col+4,1) ;
#else
			k = acpt_resp("RETURN pour plus, #(Article), R(etourner):",
							row+i+2, col+4,1) ;
#endif
		else
#ifdef ENGLISH
			k = acpt_resp("Select #(Item) Or Q(uit)  :",
							row+i+2, col+4,0) ;
#else
			k = acpt_resp("Choisir #(Article) ou R(etourner)  :",
							row+i+2, col+4,0) ;
#endif

		if ( k >= 1 && k < i-1 ){
			*sect = t_sect[k-1];
			return(1) ;
		}
		if(k < 0)return(0);
		put_blanks = 2 ;
	}
} 

/*-----------------------------------------------------------------------
	Usage :   supp_hlp   (supp_cd, row, col )
		  row, col 	; self explanatory .
------------------------------------------------------------------------*/

supp_hlp(supp_cd,row,col) 
char	*supp_cd ;
int	row,
	col;  	/* window parameters */
{
 	int 		i,k ; 
	Supplier	s_rec ;
	char		t_supp[MAX_LINES][11] ;
	int		key_no ;
	int	on_name = 0 ;	/* If Yes help on description */

	TOT_STRNG = 51 ;

	if( !on_name ) {	/* If Not on Description */
#ifdef ENGLISH
		get_name(row, col, 1, "Help on Name(Y/N)?      ", kee_name, 0);
#else
		get_name(row, col, 1, "Assistance sur nom (O/N)?         ", kee_name, 0);
#endif
		if(kee_name[0] == L_YES || kee_name[0] == U_YES)
			on_name = 1;
                if (kee_name[0] == NULL) on_name = 1;
	}
	else {
#ifdef ENGLISH
		get_name(row, col, 1, "Help on Supplier#(Y/N)? ", kee_name, 0);
#else
		get_name(row, col, 1, "Assistance sur #fournisseur(O/N)? ", kee_name, 0);
#endif
		if(kee_name[0] == L_YES || kee_name[0] == U_YES)
			on_name = 0;
	}

	/* accept the starting key */
	if( on_name) {
#ifdef ENGLISH
		get_name(row, col, DESC_KEY, "Starting Name:      ", kee_name, 0);
#else
		get_name(row, col, DESC_KEY, "Nom debutant:          ", kee_name, 0);
#endif
		strcpy(s_rec.s_abb,kee_name);
		key_no = 2;
	}
	else {
#ifdef ENGLISH
		get_name(row, col, 10, "Starting Supplier#: ", kee_name, 0);
#else
		get_name(row, col, 10, "#Fournisseur debutant: ", kee_name, 0);
#endif
		strcpy(s_rec.s_supp_cd,kee_name);
		Right_Justify_Numeric(s_rec.s_supp_cd,
						sizeof(s_rec.s_supp_cd)-1);
		key_no = 0;
	}

	/* Start the file */
	flg_reset(SUPPLIER);

#ifdef ENGLISH
	strcpy(line,"No  Supp Code   Supplier Name") ;
#else
	strcpy(line,"No  Code Four.  Nom fournisseur") ;
#endif
	expnd_line(line) ;
	height = 0;
	put_blanks = 1 ;

	code = get_n_supplier(&s_rec, BROWSE, key_no, FORWARD, err_msg) ;

	for(;;){
		i = 1;
		max_val = 0;
		for( ; ;code = get_n_supplier(&s_rec,BROWSE,key_no,FORWARD,err_msg)){
			if(ERROR == code )return(DBH_ERR);

			if(code == EFL)break;
#ifdef JRNL
			SettleLock( (char *)&s_rec, SUPPLIER, err_msg );
#endif

			if(i == (23 - row - 5) || i == MAX_LINES)break;

			strcpy(t_supp[max_val],s_rec.s_supp_cd );

			text = line+i*(TOT_STRNG+1);

			sprintf(text, "%1d.  %-10.10s  %-35.35s",
				(max_val+1), s_rec.s_supp_cd,
				 s_rec.s_name );
			expnd_line(text) ;
			i++;
			max_val++;
		} 
#ifndef	ORACLE
		seq_over(SUPPLIER);
#endif
		if(put_blanks == 1 )
			if(i == 1){
#ifdef ENGLISH
				fomen("No Suppliers to be Displayed ");
#else
				fomen("Pas de fournisseurs a afficher ");
#endif
				/***
				return(-1);
				***/
				/* Returning 0 to redraw() the screen to wipe
				   out "Starting Name:" message */
				return(0);
			}
  		line[i*(TOT_STRNG+1)] = '\0';
		expnd_line(line+i*(TOT_STRNG+1));
		i++;
		line[i*(TOT_STRNG+1)] = '\0';

		if(i > height)height = i;

		text = line;
		if(window(row, col,WIDTH, (height+4), text, put_blanks ) < 0)
			return(0); 

		/* Get the user Response and Validate against
		   displayed Item Numbers ..**/

		if( code != EFL)
#ifdef ENGLISH
			k = acpt_resp("RETURN for more, #(Item), Q(uit):",
							row+i+2, col+4,1) ;
#else
			k = acpt_resp("RETURN pour plus, #(Article), R(etourner):",
							row+i+2, col+4,1) ;
#endif
		else
#ifdef ENGLISH
			k = acpt_resp("Select #(Item) Or Q(uit)  :",
							row+i+2, col+4,0) ;
#else
			k = acpt_resp("Choisir #(Article) ou R(etourner)  :",
							row+i+2, col+4,0) ;
#endif

		if ( k >= 1 && k < i-1 ){
			strcpy(supp_cd, t_supp[k - 1]) ;
			return(1) ;
		}
		if(k < 0)return(0);
		put_blanks = 2 ;
	}
} 

/*-----------------------------------------------------------------------
	Usage :   fatype_hlp   (type_cd, row, col )
		  type_cd	; Fixed asset type code
		  row, col 	; self explanatory .
------------------------------------------------------------------------*/

fatype_hlp(type_cd,row,col) 
char	*type_cd ;
int	row,
	col;  	/* window parameters */
{
 	int 		i,k ; 
	char		t_code[MAX_LINES][5] ;
	Fa_type		fa_type;

	int	on_desc = 0;

	TOT_STRNG = 35 ;

	/* accept the starting key */
#ifdef ENGLISH
	get_name(row, col, 2, "Starting Type: ", kee_name, 0);
#else
	get_name(row, col, 2, "Genre debutant: ", kee_name, 0);
#endif

	/* Make The Starting key */
	strncpy(fa_type.code,kee_name,4) ;

	/* Start the file */
	flg_reset(FATYPE);

#ifdef ENGLISH
	strcpy(line,"No Type     Description") ;
#else
	strcpy(line,"No Genre    Description") ;
#endif
	expnd_line(line) ;
	height = 0;
	put_blanks = 1 ;

	code = get_n_fatype(&fa_type, BROWSE, 0, FORWARD, err_msg) ;

	for(;;){
		i = 1;
		max_val = 0;
		for( ; ;code = get_n_fatype(&fa_type,BROWSE,0,FORWARD,err_msg)){
			if(ERROR == code )return(DBH_ERR);

			if(code == EFL)break;
#ifdef JRNL
			SettleLock( (char *)&fa_type, FATYPE, err_msg );
#endif

			if(i == (23 - row - 5) || i == MAX_LINES)break;

			strcpy(t_code[max_val],fa_type.code );

			text = line+i*(TOT_STRNG+1);

			sprintf(text, "%1d. %-4.4s %-23.23s",
				(max_val+1), fa_type.code, fa_type.desc);
			expnd_line(text) ;
			i++;
			max_val++;
		} 
#ifndef	ORACLE
		seq_over(FATYPE);
#endif
		if(put_blanks == 1 )
			if(i == 1){
#ifdef ENGLISH
				fomen("No Types to be Displayed ");
#else
				fomen("Pas de genres a afficher ");
#endif
				return(0);
			}
  		line[i*(TOT_STRNG+1)] = '\0';
		expnd_line(line+i*(TOT_STRNG+1));
		i++;
		line[i*(TOT_STRNG+1)] = '\0';

		if(i > height)height = i;

		text = line;
		if(window(row, col,WIDTH, (height+4), text, put_blanks ) < 0)
			return(0); 

		/* Get the user Response and Validate against
		   displayed Item Numbers ..**/

		if( code != EFL)
#ifdef ENGLISH
			k = acpt_resp("RETURN for more, #(Item), Q(uit):",
							row+i+2, col+4,1) ;
#else
			k = acpt_resp("RETURN pour plus, #(Article), R(etourner):",
							row+i+2, col+4,1) ;
#endif
		else
#ifdef ENGLISH
			k = acpt_resp("Select #(Item) Or Q(uit)  :",
							row+i+2, col+4,0) ;
#else
			k = acpt_resp("Choisir #(Article) ou R(etourner)  :",
							row+i+2, col+4,0) ;
#endif

		if ( k >= 1 && k < i-1 ){
			strcpy(type_cd, t_code[k - 1]) ;
			return(1) ;
		}
		if(k < 0)return(0);
		put_blanks = 2 ;
	}
} 

/*-----------------------------------------------------------------------
	Usage :   fadept_hlp   (dept_cd, row, col )
		  dept_cd	; Fixed asset dept code
		  row, col 	; self explanatory .
------------------------------------------------------------------------*/

fadept_hlp(dept_cd,row,col) 
char	*dept_cd ;
int	row,
	col;  	/* window parameters */
{
 	int 		i,k ; 
	char		t_code[MAX_LINES][5] ;
	Fa_dept		fa_dept;

	TOT_STRNG = 35 ;

	/* accept the starting key */
#ifdef ENGLISH
	get_name(row, col, 2, "Starting Dept:   ", kee_name, 0);
#else
	get_name(row, col, 2, "Dept debutant:  ", kee_name, 0);
#endif

	/* Make The Starting key */
	strncpy(fa_dept.code,kee_name,4) ;

	/* Start the file */
	flg_reset(FADEPT);

#ifdef ENGLISH
	strcpy(line,"No Dept     Description") ;
#else
	strcpy(line,"No Dept     Description") ;
#endif
	expnd_line(line) ;
	height = 0;
	put_blanks = 1 ;

	code = get_n_fadept(&fa_dept, BROWSE, 0, FORWARD, err_msg) ;

	for(;;){
		i = 1;
		max_val = 0;
		for( ; ;code = get_n_fadept(&fa_dept,BROWSE,0,FORWARD,err_msg)){
			if(ERROR == code )return(DBH_ERR);

			if(code == EFL)break;

#ifdef JRNL
			SettleLock( (char *)&fa_dept, FADEPT, err_msg );
#endif
			if(i == (23 - row - 5) || i == MAX_LINES)break;

			strcpy(t_code[max_val],fa_dept.code );

			text = line+i*(TOT_STRNG+1);

			sprintf(text, "%1d. %-4.4s %-23.23s",
				(max_val+1), fa_dept.code, fa_dept.desc);
			expnd_line(text) ;
			i++;
			max_val++;
		} 
#ifndef	ORACLE
		seq_over(FADEPT);
#endif
		if(put_blanks == 1 )
			if(i == 1){
#ifdef ENGLISH
				fomen("No Depts to be Displayed ");
#else
				fomen("Pas de departements a afficher");
#endif
				return(0);
			}
  		line[i*(TOT_STRNG+1)] = '\0';
		expnd_line(line+i*(TOT_STRNG+1));
		i++;
		line[i*(TOT_STRNG+1)] = '\0';

		if(i > height)height = i;

		text = line;
		if(window(row, col,WIDTH, (height+4), text, put_blanks ) < 0)
			return(0); 

		/* Get the user Response and Validate against
		   displayed Item Numbers ..**/

		if( code != EFL)
#ifdef ENGLISH
			k = acpt_resp("RETURN for more, #(Item), Q(uit):",
							row+i+2, col+4,1) ;
#else
			k = acpt_resp("RETURN pour plus, #(Article), R(etourner):",
							row+i+2, col+4,1) ;
#endif
		else
#ifdef ENGLISH
			k = acpt_resp("Select #(Item) Or Q(uit)  :",
							row+i+2, col+4,0) ;
#else
			k = acpt_resp("Choisir #(Article) ou R(etourner)  :",
							row+i+2, col+4,0) ;
#endif

		if ( k >= 1 && k < i-1 ){
			strcpy(dept_cd, t_code[k - 1]) ;
			return(1) ;
		}
		if(k < 0)return(0);
		put_blanks = 2 ;
	}
} 

/*-----------------------------------------------------------------------
	Usage :   faitem_hlp   (costcen, item_cd, row, col )
		  costcen	; For this cost center help is displayed.
		  item_cd	; Item code
		  row, col 	; self explanatory .
------------------------------------------------------------------------*/

faitem_hlp(costcen,item_cd,row,col) 
short	costcen ;
long	*item_cd ;
int	row,
	col;  	/* window parameters */
{
 	int 		key_no, i,k ; 
	long		t_numb[MAX_LINES] ;
	Fa_rec		fa_rec;
	
	int	on_desc = 0 ;

	TOT_STRNG = 37 ;

	if( !on_desc ) {	/* If Not on Description */
#ifdef ENGLISH
		get_name(row, col, 1, "Help on Desc(Y/N)?  ", kee_name, 0);
#else
		get_name(row, col, 1, "Assistance sur Desc.(O/N)?   ", kee_name, 0);
#endif
		if(kee_name[0] == L_YES || kee_name[0] == U_YES)
			on_desc = 1;
                if (kee_name[0] == NULL) on_desc = 1;
	}

	/* accept the starting key */
	if( on_desc) {
#ifdef ENGLISH
		get_name(row, col, DESC_KEY, "Starting Desc:  ", kee_name, 0);
#else
		get_name(row, col, DESC_KEY, "Desc. debutante:  ", kee_name, 0);
#endif
		fa_rec.fa_costcen = costcen ;
		strcpy(fa_rec.fa_desc,kee_name);
		key_no = 5;
	}
	else {
		/* Start the file */
		/* accept the starting key */
#ifdef ENGLISH
		get_name(row, col, 2, "Starting Item ID: ", kee_name, 1);
#else
		get_name(row, col, 2, "Identite d'article debutant: ", kee_name, 1);
#endif
		fa_rec.fa_costcen = costcen ;
		sscanf( kee_name, "%ld", &fa_rec.fa_itemid );
		key_no = 0;
	}

	/* Start the file */
	flg_reset(FAMAST);

#ifdef ENGLISH
	strcpy(line,"No  Item     Description") ;
#else
	strcpy(line,"No  Article  Description") ;
#endif
	expnd_line(line) ;
	height = 0;
	put_blanks = 1 ;

#ifndef ORACLE
	code = get_n_famast(&fa_rec, BROWSE, 0, FORWARD, err_msg) ;
#else
	code = get_n_famast(&fa_rec, BROWSE, 0, EQUAL, err_msg) ;
#endif

	for(;;){
		i = 1;
		max_val = 0;
#ifndef ORACLE
		for( ; ;code = get_n_famast(&fa_rec,BROWSE,0,FORWARD,err_msg)){
#else
		for( ; ;code = get_n_famast(&fa_rec,BROWSE,0,EQUAL,err_msg)){
#endif
			if(ERROR == code )return(DBH_ERR);

			if(code == EFL)break;
#ifdef JRNL
			SettleLock( (char *)&fa_rec, FAMAST, err_msg );
#endif

#ifndef ORACLE
			if(fa_rec.fa_costcen != costcen) {
				code = EFL ;
				break;
			}
#endif

			if(i == (23 - row - 5) || i == MAX_LINES)break;

			t_numb[max_val] = fa_rec.fa_itemid;

			text = line+i*(TOT_STRNG+1);

			sprintf(text, "%1d. %6d %-31.31s",
				(max_val+1), fa_rec.fa_itemid, fa_rec.fa_desc);
			expnd_line(text) ;
			i++;
			max_val++;
		} 
#ifndef	ORACLE
		seq_over(FAMAST);
#endif
		if(put_blanks == 1 )
			if(i == 1){
#ifdef ENGLISH
				fomen("No Items to be Displayed ");
#else
				fomen("Pas d'articles a afficher");
#endif
				return(0);
			}
  		line[i*(TOT_STRNG+1)] = '\0';
		expnd_line(line+i*(TOT_STRNG+1));
		i++;
		line[i*(TOT_STRNG+1)] = '\0';

		if(i > height)height = i;

		text = line;
		if(window(row, col,WIDTH, (height+4), text, put_blanks ) < 0)
			return(0); 

		/* Get the user Response and Validate against
		   displayed Item Numbers ..**/

		if( code != EFL)
#ifdef ENGLISH
			k = acpt_resp("RETURN for more, #(Item), Q(uit):",
							row+i+2, col+4,1) ;
#else
			k = acpt_resp("RETURN pour plus, #(Article), R(etourner):",
							row+i+2, col+4,1) ;
#endif
		else
#ifdef ENGLISH
			k = acpt_resp("Select #(Item) Or Q(uit)  :",
							row+i+2, col+4,0) ;
#else
			k = acpt_resp("Choisir #(Article) ou R(etourner)  :",
							row+i+2, col+4,0) ;
#endif

		if ( k >= 1 && k < i-1 ){
			*item_cd = t_numb[k - 1] ;
			return(1) ;
		}
		if(k < 0)return(0);
		put_blanks = 2 ;
	}
} 

/*-----------------------------------------------------------------------
	Usage :   cust_hlp   (cust_cd, row, col )
		  cust_cd	; pointer to customer code
		  row, col 	; self explanatory .
------------------------------------------------------------------------*/

cust_hlp(cust_cd,row,col) 
char	*cust_cd ;
int	row,
	col;  	/* window parameters */
{
 	int 		i,k ; 
	Cu_rec		s_rec ;
	char		t_cust[MAX_LINES][sizeof(s_rec.cu_code)] ;
	int		key_no ;
	int	on_name = 0 ;	/* If Yes help on description */

	TOT_STRNG = 48 ;

	if( !on_name ) {	/* If Not on Description */
#ifdef ENGLISH
		get_name(row, col, 1, "Help on Name(Y/N)?       ", kee_name, 0);
#else
		get_name(row, col, 1, "Assistance sur nom(O/N)?         ", kee_name, 0);
#endif
		if(kee_name[0] == L_YES || kee_name[0] == U_YES)
			on_name = 1;
                if (kee_name[0] == NULL) on_name = 1;
	}
	else {
#ifdef ENGLISH
		get_name(row, col, 1, "Help on Cust. Code(Y/N)? ", kee_name, 0);
#else
		get_name(row, col, 1, "Assistance sur Code client(O/N)? ", kee_name, 0);
#endif
		if(kee_name[0] == L_YES || kee_name[0] == U_YES)
			on_name = 0;
	}

	/* accept the starting key */
	if( on_name) {
#ifdef ENGLISH
		get_name(row, col, DESC_KEY, "Starting Name:       ", kee_name, 0);
#else
		get_name(row, col, DESC_KEY, "Nom debutant:            ", kee_name, 0);
#endif
		strcpy(s_rec.cu_name,kee_name);
		key_no = 1;
	}
	else {
#ifdef ENGLISH
		get_name(row, col, 10, "Starting Cust. Code: ", kee_name, 0);
#else
		get_name(row, col, 10, "Code de client debutant: ",kee_name, 0);
#endif
		strcpy(s_rec.cu_code,kee_name);
		key_no = 0;
	}

	/* Start the file */
	flg_reset(CUSTOMER);

#ifdef ENGLISH
	strcpy(line,"No  Cust Cd  Customer Name") ;
#else
	strcpy(line,"No  Cd Client  Nom Client") ;
#endif
	expnd_line(line) ;
	height = 0;
	put_blanks = 1 ;

	code = get_n_cust(&s_rec, BROWSE, key_no, FORWARD, err_msg) ;

	for(;;){
		i = 1;
		max_val = 0;
		for( ; ;code = get_n_cust(&s_rec,BROWSE,key_no,FORWARD,err_msg)){
			if(ERROR == code )return(DBH_ERR);

			if(code == EFL)break;

#ifdef JRNL
			SettleLock( (char *)&s_rec, CUSTOMER, err_msg );
#endif
			if(i == (23 - row - 5) || i == MAX_LINES)break;

			strcpy(t_cust[max_val],s_rec.cu_code );

			text = line+i*(TOT_STRNG+1);

			sprintf(text, "%1d.  %-6.6s   %-35.35s",
				(max_val+1), s_rec.cu_code,
				 s_rec.cu_name );
			expnd_line(text) ;
			i++;
			max_val++;
		} 
#ifndef	ORACLE
		seq_over(CUSTOMER);
#endif
		if(put_blanks == 1 )
			if(i == 1){
#ifdef ENGLISH
				fomen("No Customers to be Displayed ");
#else
				fomen("Pas de clients a afficher ");
#endif
				/***
				return(-1);
				***/
				/* Returning 0 to redraw() the screen to wipe
				   out "Starting Name:" message */
				return(0);
			}
  		line[i*(TOT_STRNG+1)] = '\0';
		expnd_line(line+i*(TOT_STRNG+1));
		i++;
		line[i*(TOT_STRNG+1)] = '\0';

		if(i > height)height = i;

		text = line;
		if(window(row, col,WIDTH, (height+4), text, put_blanks ) < 0)
			return(0); 

		/* Get the user Response and Validate against
		   displayed Item Numbers ..**/

		if( code != EFL)
#ifdef ENGLISH
			k = acpt_resp("RETURN for more, #(Item), Q(uit):",
							row+i+2, col+4,1) ;
#else
			k = acpt_resp("RETURN pour plus, #(Article), R(etourner):",
							row+i+2, col+4,1) ;
#endif
		else
#ifdef ENGLISH
			k = acpt_resp("Select #(Item) Or Q(uit)  :",
							row+i+2, col+4,0) ;
#else
			k = acpt_resp("Choisir #(Article) ou R(etourner)  :",
							row+i+2, col+4,0) ;
#endif

		if ( k >= 1 && k < i-1 ){
			strcpy(cust_cd, t_cust[k - 1]) ;
			return(1) ;
		}
		if(k < 0)return(0);
		put_blanks = 2 ;
	}
} 

/*-----------------------------------------------------------------------
	Usage :   fld_hlp   (fld_hdr, fld_info, fld_no, row, col )
		  fld_hdr	; Header information of Fields
		  fld_info	; Fields information 
		  fld_no	; Selected field# wiil be moved to this
		  row, col 	; self explanatory .
------------------------------------------------------------------------*/

fld_hlp(fld_hdr, fld_info, fld_no, row, col) 
Fld_hdr	fld_hdr ;
Field	*fld_info ;
int	*fld_no ;
int	row,
	col;  	/* window parameters */
{
 	int 	i, j, k ; 
	short	t_numb[MAX_LINES] ;

	TOT_STRNG = 34 ;

#ifdef ENGLISH
	sprintf(line,"File: %s",fld_hdr.filenm) ;
#else
	sprintf(line,"Dossier: %s",fld_hdr.filenm) ;
#endif
	expnd_line(line) ;
	text = line+TOT_STRNG+1 ;
#ifdef ENGLISH
	strcpy(text,"     Fld#    Name");
#else
	strcpy(text,"    #Champ    Nom");
#endif
	expnd_line(text) ;
	height = 0;
	put_blanks = 1 ;

	for(j = 0 ; j < fld_hdr.no_fields ; ){
		i = 2;
		max_val = 0;
		for( ; j < fld_hdr.no_fields ; j++ ){
			if(i == (23 - row - 5) || i == MAX_LINES)break;

			t_numb [max_val] = (j+1) ;

			text = line+i*(TOT_STRNG+1);

			sprintf(text, "%1d.    %3d    %-15.15s",
				(max_val+1), (j+1), (fld_info+j)->name) ;
			expnd_line(text) ;
			i++;
			max_val++;
		} 
		if(put_blanks == 1 )
			if(i == 2){
#ifdef ENGLISH
				fomen("No Fields to be Displayed ");
#else
				fomen("Pas de champs a afficher ");
#endif
				return(-1);
			}

  		line[i*(TOT_STRNG+1)] = '\0';
		expnd_line(line+i*(TOT_STRNG+1));
		i++;
		line[i*(TOT_STRNG+1)] = '\0';

		if(i > height)height = i;

		text = line;
		if(window(row, col,WIDTH, (height+4), text, put_blanks ) < 0)
			return(0); 

		/* Get the user Response and Validate against
		   displayed Item Numbers ..**/

		if( j < fld_hdr.no_fields)
#ifdef ENGLISH
			k = acpt_resp("RETURN for more, #(Item), Q(uit):",
#else
			k = acpt_resp("RETURN pour plus, #(Article), R(etourner):",
#endif
							row+i+2, col+4,1) ;
		else
#ifdef ENGLISH
			k = acpt_resp("Select #(Item) Or Q(uit)  :",
							row+i+2, col+4,0) ;
#else
			k = acpt_resp("Choisir #(Article) ou R(etourner)  :",
							row+i+2, col+4,0) ;
#endif

		if ( k >= 1 && k < i-1 ){
			*fld_no = t_numb[k - 1] ;
			return(1) ;
		}
		if(k < 0)return(0);
		put_blanks = 2 ;
	}
}	/* fld_hlp ends */
/*---------------------------------------------------------------*/
/* pads line with blanks to expnd it to required characters. */

expnd_line(line) 
char	*line ;
{
	int 	j ;

	j=strlen(line);
	if(j > TOT_STRNG)j = TOT_STRNG;
	for ( ; j < TOT_STRNG; j++)
		*(line + j) = ' ' ;
	*(line+j) = '\n';
}

acpt_resp(message, row, col,more)
char 	*message ;
int 	row,col ;
int	more;
{
	int 	prompt_len ;
	char	d1 ;

	prompt_len = strlen(message) ;

	poscur( row, col) ;
	puts( message ) ;

	/** To Clear-off the junk which is appearing for some vague reason ***/
	puts(" ") ;

	while(1)
	{
		poscur( row, col+prompt_len) ;
#ifdef MS_DOS
		d1 = getch() ;
#else
		d1 = getchar() ;
#endif
		fomce();	/* Clears error message */
		if ( !valid_char(d1) ) continue ;
		if ( d1 == T_DEL ) continue ;
		putchar(d1) ;
		if ( more && (d1 == T_NL || d1 == T_CRF) ) return(0) ;
		if ( d1 == U_QUIT || d1 == L_QUIT ) return(-1) ;
		if (  d1 < '1' ||  d1 > (char)('0'+max_val) ) {
#ifdef ENGLISH
			fomen("Invalid Item Number .. Enter Again");
#else
			fomen("Numero d'article invalide.. Ressayer");
#endif
			continue ;
		}
		return(d1-'0') ;
	}
}
/*--------------------------------------------------------------------*/
valid_char(c) 
char	c ;
{
	if((c >= 'A' && c <= 'Z' ) || (c >= 'a' && c <= 'z') ||
	    (c >= '0' && c <= '9' ) || (c >= T_DEL && c <= T_CRF) || 
	    (c=='!'||c=='#'||c=='*'||c=='%'||c=='$'||c=='+'||c=='-'||c=='_'))
		return(1);
	else
		return(0) ;
}

#include <ctype.h>

#ifdef	O_PROFOM

#include <cfomtm.h>
#include <cfomtcr.h>

extern	struct tcrblk tb;

#else

extern	char	cviatb;

#endif
/*----------------------------------------------------------*/
get_name(row, col,len, prompt, ptr,type)
int 	row, col ;
int	len ;
char	*prompt ;
char	*ptr ;
int	type;	/* 1 - Numeric, 0 - String */
{
	int 	i ;
	char	c ;

#ifdef	O_PROFOM
	if(telm(ttyp) == 0){
        	c= telm(vabase) ; c |= telm(hiten) ; 
       		poscur(row,col+strlen(prompt)); puts(cas(efa)) ; 
      		poscur(row,col); 
       		puts(cas(sfa)) ; putchar(c) ;
		puts( prompt) ;
	} else {
        	poscur(row,col); 
        	puts(cas(hiten)) ; 
		puts( prompt) ;
       		poscur(row,col+strlen(prompt)); puts(cas(efa)) ; 
		putchar(' ');
	}
#else
   	poscur(row,col); 
	cviatb |= 0x8;
	puts(prompt);
#endif

	poscur(row, col+1+strlen(prompt) );
	puts("        ") ;   
	poscur(row, col+1+strlen(prompt) );

	for ( i = 0 ; i < len;) {
#ifdef MS_DOS
		c = getch();
#else
		c = getchar();
#endif
		if ( !valid_char(c) ) continue ;
		if ( c == T_NL  || c == T_CRF) 	/* Newline or Return */
			break ;
		if( c != T_DEL ) {		/* If not Backspace */
			if(type == 1 && !isdigit(c))continue;	/* numeric */
			/**
			*(ptr+i) = toupper(c) ;
			***/
			*(ptr+i) = c ;
			i++ ;
		} else
		    if ( i > 0 ) i-- ;
		/**
		putchar(toupper(c)) ;
		**/
		putchar(c) ;
	}
	if(type == 1 && i == 0){    /* if first char NULL sscanf won't work */
		*(ptr+i) = '0';
		i++;
	}
	*(ptr+i) = '\0' ;
}	
/*--------------------------------------------------------------*/
#ifdef	O_PROFOM
window(row,col,nch,nlines,text1, put_blanks ) 
int	row,
	col,
	nch,
	nlines ; 
int 	put_blanks ;		/* If The screen to be blanked before */
char	*text1 ; 
	        /*Displays window margin in reverse video */ 
{
	int n, m ;  
	char	*text;

	text = text1;
        if (row<1 || row>telm(lines) || col<1 || col>telm(cols)) {
#ifdef ENGLISH
		fomer("Window Row/Col Coordinates not Proper... ");
#else
		fomer("Coordonnees Rangee/Colonne ne sont pas convenables... ");
#endif
                return(-1);        /* row/col out of range */ 
	}
        if ((row+nlines)<1 || (row+nlines)>telm(lines)){
#ifdef ENGLISH
		fomer("Too Large a Window");
#else
		fomer("Fenetre trop grande");
#endif
		return(-1) ; 
	}
        if ((col+nch)<1 || (col+nch)>telm(cols)){
#ifdef ENGLISH
		fomer("Too Wide a Window");
#else
		fomer("Fenetre trop large");
#endif
		return(-1) ; 
	}
	if (put_blanks == 1) {
        	if (telm(ttyp)== 0) { 
			draw_0_ttyp(row,col,nch);
			draw_0_ttyp((row+1),col,2);
			draw_0_ttyp((row+1),(col+nch-2),2);
		} else {
			draw_1_ttyp(row,col,nch);
			draw_1_ttyp((row+1),col,2);
			draw_1_ttyp((row+1),(col+nch-2),2);
		}
        	poscur(row+1,col+3) ; 	/* Clearing First Line */
		for(n=0 ; n < nch-5 ; n++)putchar(' ');
	}
        for(n=2 ; n<nlines ; n++) { 
		if(put_blanks == 1){
			if(telm(ttyp) == 0){
				draw_0_ttyp(row+n,col,2);
				poscur(row+n,col+3);
				putchar(' ') ; 
        		} else
				draw_1_ttyp(row+n,col,2);
		}
                poscur(row+n,col+3);
		for(m = 0 ;*text && m < nch-6 ; m++, text++ ){
                	if(*text == '\n')break;
			putchar(*text);
                }
		for( ; *text ; text++)
			if(*text == '\n'){
				text++;
				break;
			}
	        for (; m < nch-6 ; m++)putchar(' ');

		if(put_blanks == 1){
        		if(telm(ttyp) == 0)poscur(row+n,col+nch-3) ; 
			putchar(' ') ; 
			if(telm(ttyp) == 0)
				draw_0_ttyp(row+n,(col+nch-2),2);
        		else
				draw_1_ttyp(row+n,(col+nch-2),2);
		}
	}
	if (put_blanks == 1)
        	if (telm(ttyp)== 0)
			draw_0_ttyp(row+nlines,col,nch);
		else
			draw_1_ttyp(row+nlines,col,nch);

	return(0);
} 

draw_0_ttyp(row,col,nch)
int	row,col,nch;
{
	int	n;
	char	c;

        c= telm(vabase) ; c |= telm(rv) ; 
       	poscur(row,col+nch); puts(cas(efa)) ; 
      	poscur(row,col); 
       	puts(cas(sfa)) ; putchar(c) ;
	for(n = 2; n <= nch ; n++)
		putchar(' ') ; 
}

draw_1_ttyp(row,col,nch)
int	row,col,nch;
{
	int n;

        poscur(row,col); 
        puts(cas(rv)) ; 
	for(n = 0 ; n < nch ; n++)
		putchar(' ');
       	poscur(row,col+nch); puts(cas(efa)) ; 
	putchar(' ');
}
#endif

/* Help functions for the Tendering Module */
#include "hlp_tend.c"

/*----------------E N D   O F   F I L E---------------------*/
