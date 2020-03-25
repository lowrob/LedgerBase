/* --------------------------------------------------------------------------
	SOURCE NAME:  CUSTLST.C
	SYSTEM     :  BUGETARY FINANCIAL SYSTEM 
	MODULE     :  PURCHASE ORDER MODULE
	CREATED ON :  21 DEC. 1992
	CREATED BY :  Louis Robichaud

DESCRIPTION:
	This program prints a full customer list 
        customer number or abbrev. name.

MODIFICATIONS:

PROGRAMMER           YY/MM/DD        DESCRIPTION OF MODIFICATIONS
^^^^^^^^^^           ^^^^^^^^        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

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
Cu_rec 	 cu_rec;

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
static	char	resp[2] ;
char	 c_no1[11], c_no2[11];
char	 c_name1[21], c_name2[21];

struct CUST_INFO {
	char ccode[11];
	char cname[31];
	char cadd1[31];
	char cadd2[31];
	char cadd3[31];
	char cpc[31];
}cust_info[2];

CustLst(mode)
int mode;
{
	int field;

	LNSZ = 80;
	PGSIZE = 60;

	STRCPY(program,"CUSTLST");

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
			STRCPY(e_mesg,"custrpt.dat");
			if((retval = GetFilename(e_mesg)) < 0)
				return(retval);
			STRCPY(discfile,e_mesg);
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
		STRCPY(c_no1,"         0");
		STRCPY(c_no2,"ZZZZZZZZZZ");
		retval = GetCNbrRange(c_no1,c_no2);
		if(retval < 0) return(-1);
		else	if(retval == EXIT) return(0);
		keyno = 0;
	}
	else {
		STRCPY(c_name1,"                        ");
		STRCPY(c_name2,"zzzzzzzzzzzzzzzzzzzzzzzz");
		retval = GetCNameRange(c_name1,c_name2);
		if(retval < 0) return(-1);
		else	if(retval == EXIT) return(0);
		keyno = 2;
	}

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
		STRCPY(cu_rec.cu_code,c_no1);
	}
	else {
		STRCPY(cu_rec.cu_abrev,c_name1);
	}
	flg_reset( CUSTOMER );

        for( ; ; ) {
		retval = get_n_cust(&cu_rec,BROWSE,keyno,FORWARD,e_mesg);
		if(retval < 0) {
			if(retval == EFL) break;
			fomer(e_mesg);
			break;
		}
		if(mode == BYNO) {
			if(strcmp(cu_rec.cu_code,c_no2) > 0) break;
		}	
		else {
			if(strcmp(cu_rec.cu_abrev,c_name2) > 0) break;
		}


		STRCPY(cust_info[cnt].ccode,cu_rec.cu_code);
		if(mode == BYNO) 
			STRCPY(cust_info[cnt].cname,cu_rec.cu_abrev);
		else
			STRCPY(cust_info[cnt].cname,cu_rec.cu_abrev);

/* print only address lines that are not blank.			*/
/* report address line 1  (sadd1) */
		if(strcmp(cu_rec.cu_adr1, "\0")){ 
			STRCPY(cust_info[cnt].cadd1,cu_rec.cu_adr1);
			cu_rec.cu_adr1[0]='\0';
		}
		else
		if(strcmp(cu_rec.cu_adr2, "\0")){ 
			STRCPY(cust_info[cnt].cadd1,cu_rec.cu_adr2);
			cu_rec.cu_adr2[0]='\0';
		}
		else
		if(strcmp(cu_rec.cu_adr3, "\0")){ 
			STRCPY(cust_info[cnt].cadd1,cu_rec.cu_adr3);
			cu_rec.cu_adr3[0]='\0';
		}
		else
		if(strcmp(cu_rec.cu_pc, "\0")){ 
			STRCPY(cust_info[cnt].cadd1,cu_rec.cu_pc);
			cu_rec.cu_pc[0]='\0';
		}
		else
			STRCPY(cust_info[cnt].cadd1," ");

/* report address line 2  (sadd2) */

		if(strcmp(cu_rec.cu_adr1, "\0")){ 
			STRCPY(cust_info[cnt].cadd2,cu_rec.cu_adr1);
			cu_rec.cu_adr1[0]='\0';
		}
		else
		if(strcmp(cu_rec.cu_adr2, "\0")){ 
			STRCPY(cust_info[cnt].cadd2,cu_rec.cu_adr2);
			cu_rec.cu_adr2[0]='\0';
		}
		else
		if(strcmp(cu_rec.cu_adr3, "\0")){ 
			STRCPY(cust_info[cnt].cadd2,cu_rec.cu_adr3);
			cu_rec.cu_adr3[0]='\0';
		}
		else
		if(strcmp(cu_rec.cu_pc, "\0")){ 
			STRCPY(cust_info[cnt].cadd2,cu_rec.cu_pc);
			cu_rec.cu_pc[0]='\0';
		}
		else
			STRCPY(cust_info[cnt].cadd2," ");

/* report address line 3  (sadd3) */

		if(strcmp(cu_rec.cu_adr1, "\0")){ 
			STRCPY(cust_info[cnt].cadd3,cu_rec.cu_adr1);
			cu_rec.cu_adr1[0]='\0';
		}
		else
		if(strcmp(cu_rec.cu_adr2, "\0")){ 
			STRCPY(cust_info[cnt].cadd3,cu_rec.cu_adr2);
			cu_rec.cu_adr2[0]='\0';
		}
		else
		if(strcmp(cu_rec.cu_adr3, "\0")){ 
			STRCPY(cust_info[cnt].cadd3,cu_rec.cu_adr3);
			cu_rec.cu_adr3[0]='\0';
		}
		else
		if(strcmp(cu_rec.cu_pc, "\0")){ 
			STRCPY(cust_info[cnt].cadd3,cu_rec.cu_pc);
			cu_rec.cu_pc[0]='\0';
		}
		else
			STRCPY(cust_info[cnt].cadd3," ");

/* report address line 4  (cpc) */

		if(strcmp(cu_rec.cu_adr1, "\0")){ 
			STRCPY(cust_info[cnt].cpc,cu_rec.cu_adr1);
			cu_rec.cu_adr1[0]='\0';
		}
		else
		if(strcmp(cu_rec.cu_adr2, "\0")){ 
			STRCPY(cust_info[cnt].cpc,cu_rec.cu_adr2);
			cu_rec.cu_adr2[0]='\0';
		}
		else
		if(strcmp(cu_rec.cu_adr3, "\0")){ 
			STRCPY(cust_info[cnt].cpc,cu_rec.cu_adr3);
			cu_rec.cu_adr3[0]='\0';
		}
		else
		if(strcmp(cu_rec.cu_pc, "\0")){ 
			STRCPY(cust_info[cnt].cpc,cu_rec.cu_pc);
			cu_rec.cu_pc[0]='\0';
		}
		else
			STRCPY(cust_info[cnt].cpc," ");

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
int mode;
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
        mkln(50,"Date:",5);
	tedit((char *)&longdate,"____/__/__",txt_line,R_LONG);
	mkln(57,txt_line,10);
	mkln(68,"Page:",5);
	tedit((char *)&pgcnt,"__0_",txt_line,R_INT);
	mkln(74,txt_line,4);
	if(prnt_line() < 0) return(-1);
	i = strlen(pa_rec.pa_co_name);
	mkln(((LNSZ - 1 - i) / 2)+1,pa_rec.pa_co_name, sizeof(pa_rec.pa_co_name));
	if(prnt_line() < 0) return(-1);

	if(mode == BYNO) {
#ifdef ENGLISH
	mkln(25,"Customer Address Listing by Code",32);
#else
	mkln(24,"Liste d'adresses des clients par code",37);
#endif
	}
	else{
#ifdef ENGLISH
	mkln(25,"Customer Address Listing by Name",32);
#else
	mkln(24,"Liste d'adresses des clients par nom",36);
#endif
	}
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
		mkln(1 +(36 * i),"Cust Code : ", 12);
#else
		mkln(1 +(36 * i),"Code client :", 12);
#endif
		mkln(13 +(36 * i),cust_info[i].ccode,10);
	}
	prnt_line();
	prnt_line();
	for(i = 0; i < cnt;i++) {
		mkln(1 +(36 * i),cust_info[i].cname,24);
	}
	prnt_line();
	 for(i = 0; i < cnt;i++) {
		mkln(1 +(36 * i),cust_info[i].cadd1,30);
	}
	prnt_line();
	 for(i = 0; i < cnt;i++) {
		mkln(1 +(36 * i),cust_info[i].cadd2,30);
	}
	prnt_line();
	for(i = 0; i < cnt;i++) {
		mkln(1 +(36 * i),cust_info[i].cadd3,30);
	}
	prnt_line();
	for(i = 0; i < cnt;i++) {
		mkln(1 +(36 * i),cust_info[i].cpc,7);
	}
	prnt_line();
	prnt_line();
} 

/*-------------------------- END OF FILE ---------------------------------- */

