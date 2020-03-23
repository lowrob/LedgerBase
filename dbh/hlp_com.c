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
static	char	line[MAX_TEXT] ;	/* max chars in widow WIDTH*MAX_LINES*/
static	char	err_msg[100];
static	int	code;
static	char	kee_name[50];

/********************   Terminal Constants 	****************/

#define T_DEL	8				/* Backspace*/
#define T_TAB	9				/* Tab */
#define T_NL	10 				/* New line */
#define T_CRF   13 				/* Return   */

/*-----------------------------------------------------------------------
	Usage :   gl_hlp   (fund_cd, acnt_no, rec_cd , row, col )
		  fund_cd	; For this fund code help is displayed.
		  acnt_no	; Selected account# will be moved to this.
		  rec_cd	; selected record code will be moved to this.
		  row, col 	; self explanatory .
------------------------------------------------------------------------*/

gl_hlp(fund_cd,acnt_no,rec_cd,row,col) 
short	fund_cd ;
char	*acnt_no ;
short	*rec_cd ;
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
	Usage :   sch_hlp   (sch_numb, row, col )
		  sch_numb	; selected school# will be moved to this.
		  row, col 	; self explanatory .
------------------------------------------------------------------------*/

sch_hlp(sch_numb, row, col) 
short	*sch_numb ;
int	row,
	col;  	/* window parameters */
{
 	int 		i,k ; 
	Sch_rec		schl_rec ;
	int		t_numb[MAX_LINES] ;

	TOT_STRNG = 44 ;

	/* Make The Starting key */

	/* accept the starting key */
#ifdef ENGLISH
	get_name(row, col, 2, "Starting CC#:  ", kee_name, 1);
#else
	get_name(row, col, 2, "#CC debutant:   ", kee_name, 1);
#endif
	sscanf(kee_name,"%hd", &schl_rec.sc_numb ) ;

	/* Start the file */
	flg_reset(SCHOOL);

#ifdef ENGLISH
	strcpy(line,"No  CC#   Name                          Size");
#else
	strcpy(line,"No  #CC   Nom                     Superficie");
#endif
	expnd_line(line) ;
	height = 0;
	put_blanks = 1 ;

	code = get_n_sch(&schl_rec, BROWSE, 0, FORWARD, err_msg) ;

	for(;;){
		i = 1;
		max_val = 0;
		for( ; ;code = get_n_sch(&schl_rec,BROWSE,0,FORWARD,err_msg)){
			if(ERROR == code )return(DBH_ERR);

			if(code == EFL)break;

#ifdef JRNL
			SettleLock( (char *)&schl_rec, SCHOOL, err_msg );
#endif
			if(i == (23 - row - 5) || i == MAX_LINES)break;

			t_numb [max_val] = schl_rec.sc_numb ;

			text = line+i*(TOT_STRNG+1);

			sprintf(text, "%1d.  %2d  %-28.28s  %6ld",
				(max_val+1), schl_rec.sc_numb,
				schl_rec.sc_name, schl_rec.sc_size);
			expnd_line(text) ;
			i++;
			max_val++;
		} 
#ifndef	ORACLE
		seq_over(SCHOOL);
#endif
		if(put_blanks == 1 )
			if(i == 1){
#ifdef ENGLISH
				fomen("No Cost Centres to be Displayed ");
#else
				fomen("Pas de centres de couts a afficher ");
#endif
				/***
				return(-1);
				***/
				/* Returning 0 to redraw() the screen to wipe
				   out "Starting Number:" message */
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
			*sch_numb = t_numb[k - 1] ;
			return(1) ;
		}
		if(k < 0)return(0);
		put_blanks = 2 ;
	}
}  /* sch_hlp ends */

/*---------------------------------------------------------------*/
/* pads line with blanks to expnd it to required characters. */

static
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

static
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
static
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
static
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
static
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

static
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
static
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

/*----------------E N D   O F   F I L E---------------------*/
