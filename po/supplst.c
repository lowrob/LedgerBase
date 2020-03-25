/* --------------------------------------------------------------------------
	SOURCE NAME:  SUPPLST.C
	SYSTEM     :  BUGETARY FINANCIAL SYSTEM 
	MODULE     :  PURCHASE ORDER MODULE
	CREATED ON :  12 MAY. 1990
	CREATED BY :  Cathy Burns

DESCRIPTION:
	This program prints a full supplier list 
        supplier number or abbrev. name.

MODIFICATIONS:

PROGRAMMER           YY/MM/DD        DESCRIPTION OF MODIFICATIONS
^^^^^^^^^^           ^^^^^^^^        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
M. Cormier	     90/11/23	     Made use of the returned value of the
				     function PrintHeading() to Quit the 
				     report display.
C.Leadbeater	     90/12/19	     Modified so that blank address lines 
				     are not printed.
F.Tao		     90/12/19	     Added Range Selection.
P.Ralph		     91/02/07        And extra space between each suppler
N.Mckee		     93/06/28	     Fix the titles of the report to show by 
				     name or by code 
N.Mckee		     93/06/28	     Put the phone number and the fax number on
 				     the listing
L.Robichaud	1993/10/13	Pass new parameter to close_rep function for 
				a banner to print with 3000 family.
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <repdef.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define  EXIT  12
#define  BYNO   0
#define  BYNAME 1

#ifdef ENGLISH
#define PRINTER 'P'
#define DISPLAY 'D'
#define FILE_IO	'F'
#define SPOOL	'S'	/* added for requisitions */
#define YES	'Y'
#define NO	'N'
#else
#define PRINTER 'I'
#define DISPLAY 'A'
#define FILE_IO	'D'
#define SPOOL	'B'	/* added for requisitions */
#define YES	'O'
#define NO	'N'
#endif

Pa_rec   pa_rec;             /*  Declarations for DBH record reading */
Supplier supp_rec;

static	char 	 e_mesg[80];
static	int 	 retval;
static 	long	 longdate ;
static 	int	 pgcnt;
static	short    cnt;
static	short	 copies;
static	short	 printer;
static  char	 discfile[15] ;
static 	int	 outcntl ;
static	char     program[11];
static	int	 keyno;
static	int	 code;
static	char	resp[2] ;
char	 s_no1[11], s_no2[11];
char	 s_name1[25], s_name2[25];

struct SUPP_INFO {
	char scode[11];
	char s_tmp_flg[1];
	char sname[31];
	char sadd1[31];
	char sadd2[31];
	char sadd3[31];
	char spc[31];
	char spho[11];
}supp_info[2];

SuppLst(mode)
int mode;
{
	int field;
	char incl_temp[2];

	LNSZ = 80;
	PGSIZE = 60;

	STRCPY(program,"SUPPLST");

#ifdef ENGLISH
	STRCPY(resp,"P");
#else
	STRCPY(resp,"I");
#endif
	retval = GetOutputon(resp);
	if(retval < 0) return(retval);

	switch( *resp){
		case DISPLAY:	/* display on terminal */
			resp[0] = 'D';
			outcntl = 0;
			break;
		case FILE_IO:	/* print to a file */
			resp[0] = 'F';
			outcntl = 1;
			break;
		case SPOOL:	/* spool report */
			outcntl = 1;
			break;
		case PRINTER:	/* print on printer */
			resp[0] = 'P';
			outcntl = 2;
			break;
		default:
			outcntl = 2;
			break;
	}
	if(outcntl == 1) {
		if(resp[0] = 'F') {
			STRCPY(e_mesg,"suprpt.dat");
			if((retval = GetFilename(e_mesg)) < 0)
				return(retval);
			STRCPY(discfile,e_mesg);
		}
		else {
			sprintf(discfile,"spool%d",CC_no);
			resp[0] = 'F';
		}
	}
	else {	if(outcntl == 0 ) 
			STRCPY(discfile, terminal) ;
		else discfile[0] = '\0';
	}

	copies = 1;
	if(outcntl == 2) {
		if((retval = GetNbrCopies( &copies )) < 0)
			return(retval);
	}

	if(mode == BYNO) {
		STRCPY(s_no1,"         0");
		STRCPY(s_no2,"ZZZZZZZZZZ");
		retval = GetSuppRange(s_no1,s_no2);
		if(retval < 0) return(-1);
		else	if(retval == EXIT) return(0);
		keyno = 0;
	}
	else {
		STRCPY(s_name1,"                        ");
		STRCPY(s_name2,"zzzzzzzzzzzzzzzzzzzzzzzz");
		retval = GetSNameRange(s_name1,s_name2);
		if(retval < 0) return(-1);
		else	if(retval == EXIT) return(0);
		keyno = 2;
	}

#ifdef ENGLISH
	if((retval = DisplayMessage("Include Temporary Suppliers (Y/N)?"))<0)
		return(retval);
	if((retval = GetResponse(incl_temp, "YN"))<0)
		return(retval);
#else
	if((retval = DisplayMessage("Inclure les fournisseurs temporaires (O/N)?"))<0)
		return(retval);
	if((retval = GetResponse(incl_temp, "ON"))<0)
		return(retval);
#endif

	if((retval = Confirm()) <= 0)
		return(retval);

	longdate = get_date() ;
	retval = get_param(&pa_rec,BROWSE,1,e_mesg);
        if(retval < 1) {
		fomer(e_mesg);
		close_dbh();
		return(-1);
	}

	retval = opn_prnt(resp,discfile,1,e_mesg,1 /* spool */);  
	if(retval < 0) {
		fomer( e_mesg) ;
		close_dbh() ;
		return(-1);
	}
	SetCopies( (int)copies );

	linecnt = PGSIZE;
	pgcnt = 0 ;
	cnt = 0 ;

	if(keyno == 0 ) {
		STRCPY(supp_rec.s_supp_cd,s_no1);
	}
	else {
		STRCPY(supp_rec.s_abb,s_name1);
	}
/*	
	if(keyno == 0 ) 
		supp_rec.s_supp_cd[0] = '\0' ;
	else 
		supp_rec.s_abb[0] = '\0' ;

*/
	flg_reset( SUPPLIER );

        for( ; ; ) {
		code = get_n_supplier(&supp_rec,BROWSE,keyno,FORWARD,e_mesg);
		if(code < 0) {
			if(code == EFL) break;
			fomer(e_mesg);
			break;
		}
		if(mode == BYNO) {
			if(strcmp(supp_rec.s_supp_cd,s_no2) > 0) break;
		}	
		else {
			if(strcmp(supp_rec.s_abb,s_name2) > 0) break;
		}


		if(incl_temp[0] == NO && supp_rec.s_tmp_flg[0] == YES)
			continue;

		STRCPY(supp_info[cnt].scode,supp_rec.s_supp_cd);
		supp_info[cnt].s_tmp_flg[0] = supp_rec.s_tmp_flg[0];
		if(mode == BYNO) 
			STRCPY(supp_info[cnt].sname,supp_rec.s_name);
		else
			STRCPY(supp_info[cnt].sname,supp_rec.s_abb);

/* print only address lines that are not blank.			*/

/* report address line 1  (sadd1) */

		if(strcmp(supp_rec.s_add1, "\0")){ 
			STRCPY(supp_info[cnt].sadd1,supp_rec.s_add1);
			supp_rec.s_add1[0]='\0';
		}
		else
		if(strcmp(supp_rec.s_add2, "\0")){ 
			STRCPY(supp_info[cnt].sadd1,supp_rec.s_add2);
			supp_rec.s_add2[0]='\0';
		}
		else
		if(strcmp(supp_rec.s_add3, "\0")){ 
			STRCPY(supp_info[cnt].sadd1,supp_rec.s_add3);
			supp_rec.s_add3[0]='\0';
		}
		else
		if(strcmp(supp_rec.s_pc, "\0")){ 
			STRCPY(supp_info[cnt].sadd1,supp_rec.s_pc);
			supp_rec.s_pc[0]='\0';
		}
		else
		if(strcmp(supp_rec.s_phone, "\0")){ 
			STRCPY(supp_info[cnt].sadd1,supp_rec.s_phone);
			supp_rec.s_phone[0]='\0';
		}
		else
			STRCPY(supp_info[cnt].sadd1," ");

/* report address line 2  (sadd2) */

		if(strcmp(supp_rec.s_add1, "\0")){ 
			STRCPY(supp_info[cnt].sadd2,supp_rec.s_add1);
			supp_rec.s_add1[0]='\0';
		}
		else
		if(strcmp(supp_rec.s_add2, "\0")){ 
			STRCPY(supp_info[cnt].sadd2,supp_rec.s_add2);
			supp_rec.s_add2[0]='\0';
		}
		else
		if(strcmp(supp_rec.s_add3, "\0")){ 
			STRCPY(supp_info[cnt].sadd2,supp_rec.s_add3);
			supp_rec.s_add3[0]='\0';
		}
		else
		if(strcmp(supp_rec.s_pc, "\0")){ 
			STRCPY(supp_info[cnt].sadd2,supp_rec.s_pc);
			supp_rec.s_pc[0]='\0';
		}
		else
		if(strcmp(supp_rec.s_phone, "\0")){ 
			STRCPY(supp_info[cnt].sadd2,supp_rec.s_phone);
			supp_rec.s_phone[0]='\0';
		}
		else
			STRCPY(supp_info[cnt].sadd2," ");

/* report address line 3  (sadd3) */

		if(strcmp(supp_rec.s_add1, "\0")){ 
			STRCPY(supp_info[cnt].sadd3,supp_rec.s_add1);
			supp_rec.s_add1[0]='\0';
		}
		else
		if(strcmp(supp_rec.s_add2, "\0")){ 
			STRCPY(supp_info[cnt].sadd3,supp_rec.s_add2);
			supp_rec.s_add2[0]='\0';
		}
		else
		if(strcmp(supp_rec.s_add3, "\0")){ 
			STRCPY(supp_info[cnt].sadd3,supp_rec.s_add3);
			supp_rec.s_add3[0]='\0';
		}
		else
		if(strcmp(supp_rec.s_pc, "\0")){ 
			STRCPY(supp_info[cnt].sadd3,supp_rec.s_pc);
			supp_rec.s_pc[0]='\0';
		}
		else
		if(strcmp(supp_rec.s_phone, "\0")){ 
			STRCPY(supp_info[cnt].sadd3,supp_rec.s_phone);
			supp_rec.s_phone[0]='\0';
		}
		else
			STRCPY(supp_info[cnt].sadd3," ");

/* report address line 4  (spc) */

		if(strcmp(supp_rec.s_add1, "\0")){ 
			STRCPY(supp_info[cnt].spc,supp_rec.s_add1);
			supp_rec.s_add1[0]='\0';
		}
		else
		if(strcmp(supp_rec.s_add2, "\0")){ 
			STRCPY(supp_info[cnt].spc,supp_rec.s_add2);
			supp_rec.s_add2[0]='\0';
		}
		else
		if(strcmp(supp_rec.s_add3, "\0")){ 
			STRCPY(supp_info[cnt].spc,supp_rec.s_add3);
			supp_rec.s_add3[0]='\0';
		}
		else
		if(strcmp(supp_rec.s_pc, "\0")){ 
			STRCPY(supp_info[cnt].spc,supp_rec.s_pc);
			supp_rec.s_pc[0]='\0';
		}
		else
		if(strcmp(supp_rec.s_phone, "\0")){ 
			STRCPY(supp_info[cnt].spc,supp_rec.s_phone);
			supp_rec.s_phone[0]='\0';
		}
		else
			STRCPY(supp_info[cnt].spc," ");

/* report address line 5  (spho)*/

		if(strcmp(supp_rec.s_add1, "\0")){ 
			STRCPY(supp_info[cnt].spho,supp_rec.s_add1);
			supp_rec.s_add1[0]='\0';
		}
		else
		if(strcmp(supp_rec.s_add2, "\0")){ 
			STRCPY(supp_info[cnt].spho,supp_rec.s_add2);
			supp_rec.s_add2[0]='\0';
		}
		else
		if(strcmp(supp_rec.s_add3, "\0")){ 
			STRCPY(supp_info[cnt].spho,supp_rec.s_add3);
			supp_rec.s_add3[0]='\0';
		}
		else
		if(strcmp(supp_rec.s_pc, "\0")){ 
			STRCPY(supp_info[cnt].spho,supp_rec.s_pc);
			supp_rec.s_pc[0]='\0';
		}
		else
		if(strcmp(supp_rec.s_phone, "\0")){ 
			STRCPY(supp_info[cnt].spho,supp_rec.s_phone);
			supp_rec.s_phone[0]='\0';
		}
		else
			STRCPY(supp_info[cnt].spho," ");

		cnt++;
		if(cnt == 2) {
			if(linecnt >= PGSIZE - 8)  {
				retval = PrintHeading(mode);
				if (retval == EXIT) {
					close_rep(BANNER);
					close_dbh();
					return(0);
				}	
			}
			print_line();
			cnt = 0 ;
		}
	}
	if(cnt != 0){
		if(linecnt >= PGSIZE - 8) PrintHeading(mode);
		print_line();
	}
	if(pgcnt) {
		if(term < 99) 
			last_page() ;
#ifndef SPOOLER
		else
			rite_top() ;
#endif
	}
	rite_top();
	cnt = 0	;

	close_rep(BANNER);
	close_dbh();
	return(0);
}
PrintHeading(mode)
{
	char	txt_line[80] ;
	int	i ;


	if ( term < 99 && pgcnt )
		if (next_page() < 0) return(EXIT) ;

	if ( pgcnt || term < 99) {
		if( rite_top() < 0 ) return(REPORT_ERR) ;
	}
	else linecnt = 0 ;

	pgcnt++;

	mkln(2,program,10);
#ifdef ENGLISH
        mkln(50,"Date:",5);
#else
        mkln(50,"Date:",5);
#endif
	tedit((char *)&longdate,"____/__/__",txt_line,R_LONG);
	mkln(57,txt_line,10);
#ifdef ENGLISH
	mkln(68,"Page:",5);
#else
	mkln(68,"Page:",5);
#endif
	tedit((char *)&pgcnt,"__0_",txt_line,R_INT);
	mkln(74,txt_line,4);
	if(prnt_line() < 0) return(-1);
	i = strlen(pa_rec.pa_co_name);
	mkln(((LNSZ - 1 - i) / 2)+1,pa_rec.pa_co_name, sizeof(pa_rec.pa_co_name));
	if(prnt_line() < 0) return(-1);
#ifdef ENGLISH
	if (mode == BYNO){
		mkln(25,"Supplier Address Listing by Code",32);
	}
	else {
		mkln(25,"Supplier Address Listing by Name",32);
	}
#else
	if (mode == BYNO){
		mkln(21,"Liste d'adresses des fournisseurs par code",43);
	}
	else {
		mkln(21,"Liste d'adresses des fournisseurs par nom",42);
	}
#endif
	if(prnt_line() < 0) return(-1);
	if(prnt_line() < 0) return(-1);
}
/****************************************************************************
   This routine prints two suppliers with full address information.
****************************************************************************/
print_line()
{
	int	i;

	prnt_line();
	for(i = 0; i < cnt;i++) {
#ifdef ENGLISH
		mkln(1 +(36 * i),"Supp Code : ", 12);
#else
		mkln(1 +(36 * i),"Code fourn: ", 12);
#endif
		mkln(13 +(36 * i),supp_info[i].scode,10);
		if (supp_info[i].s_tmp_flg[0] == YES) 
			mkln(25 +(36 * i),"*TEMP*",6);
	}
	prnt_line();
	prnt_line();
	for(i = 0; i < cnt;i++) {
		mkln(1 +(36 * i),supp_info[i].sname,24);
	}
	prnt_line();
	 for(i = 0; i < cnt;i++) {
		mkln(1 +(36 * i),supp_info[i].sadd1,30);
	}
	prnt_line();
	 for(i = 0; i < cnt;i++) {
		mkln(1 +(36 * i),supp_info[i].sadd2,30);
	}
	prnt_line();
	for(i = 0; i < cnt;i++) {
		mkln(1 +(36 * i),supp_info[i].sadd3,30);
	}
	prnt_line();
	for(i = 0; i < cnt;i++) {
		mkln(1 +(36 * i),supp_info[i].spc,11);
	}
	prnt_line();
	for(i = 0; i < cnt;i++) {
		mkln(1 +(36 * i),supp_info[i].spho,11);
	}
	prnt_line();
	prnt_line();
} 

/*-------------------------- END OF FILE ---------------------------------- */

