/******************************************************************************
		Sourcename   : tranlist.c
		System       : Budgetary Financial system.
		Module       : Inventory System : Stock reports
		Created on   : 89-10-03
		Created  By  : K HARISH.
		Cobol Source : 

******************************************************************************
About the file:	
	This file has routines to print stock transaction listing. 
	It is called by the file stockrep.c and end of day process.

	function tranlist() has a parameter which decides whether any 
	interaction is permitted. It interacts with the user when called
	from stock reports. When called by the end of day process, all the
	stock transactions for the day are printed on the printer, without
	any interaction with the user.

History:
Programmer      Last change on    Details

K.HARISH__      1989/10/03

M. Cormier	1990/11/29	  - Changed the default Fund range from 
					 1 to 999.
				  - changed the "if" condition to perform
				    the rite_top() function to the following:
					if(pgcnt || term < 99)

F.Tao		1991/01/31 	  - Subtotal by date  for sorted on date or 
				    period;
				  - Subtotal by stock code for sorted by stock.
				  - Final total  

L.Robichaud	1993/10/13	Pass new parameter to close_rep function for 
				a banner to print with 3000 family.
******************************************************************************/

#include <stdio.h>
#include <reports.h>
#include <isnames.h>
#include <cfomstrc.h>

#define YES		5
#define ESCAPE		10

#ifdef ENGLISH
#define PRINTER		'P'
#define DISPLAY		'D'
#define FILE_IO		'F'

#define LSTDATE		'D'
#define LSTSTCD		'S'
#define LSTPD		'P'

#define ALLOC		"AL"
#define ORDER		"OR"
#define RECPT		"RE"
#define RETSUP		"RS"
#define	ISSUE		"IS"
#define RETURN		"RT"
#define ADJUST		"AD"
#define WRTOFF		"WO"
#else
#define PRINTER		'I'
#define DISPLAY		'A'
#define FILE_IO		'D'
#define LSTPD		'P'

#define LSTDATE		'D'
#define LSTSTCD		'S'

#define ALLOC		"AL"
#define ORDER		"CM"
#define RECPT		"RC"
#define RETSUP		"RF"
#define	ISSUE		"EM"
#define RETURN		"RV"
#define ADJUST		"AJ"
#define WRTOFF		"RD"
#endif

static Pa_rec	param_rec;
static St_mast	st_mast;
static St_tran	st_tran;
static Sch_rec	school;

extern char e_mesg[80];	/* for storing error messages */

static short	copies ;
static char	resp[2], code1[11], code2[11], type1[3], type2[3];
static	short	fund1, fund2, period1,period2;
static char discfile[15];	/* for storing outputfile name */
static int	retval;
static short	pgcnt; 		/* for page count */
static long sysdt, date1, date2;	/* system date and date range fields */
static short dateflag, typeflag, fundflag, codeflag;
static long	bufdate;
static char	buftype[3];
static short	buffund;
static char	bufcode[11];
static double 	date_total,stock_total,final_total;

tranlist( interaction )
int	interaction;	/* called non-interactively by end of day process */
{
	int i;

    if( interaction ){
#ifdef ENGLISH
	STRCPY(resp,"P");
#else
	STRCPY(resp,"I");
#endif
	if( (retval=GetOutputon(resp))<0 )
		return(retval);
	switch(*resp) {
		case DISPLAY:	/* terminal defined globally */
			resp[0]='D';	/* set to display */
			STRCPY( discfile, terminal ); 
			break;
		case FILE_IO:	/* output to file */
			resp[0]='F';	/* set to file */
			STRCPY( discfile, "tranlist.dat" );
			if( (retval=GetFilename(discfile))<0 )
				return(retval);
			break;
		case PRINTER:	/* print to printer */
		default:
			resp[0]='P';
			discfile[0] = '\0';
			break;
	}
    }
    else{
	STRCPY( resp, "P" );
	discfile[0]='\0';
    }

    if( interaction ) {
	copies = 1;
	if(resp[0] == 'P') {
		if((retval = GetNbrCopies( &copies )) < 0)
			return(retval);
	}
    }

	if( opn_prnt( resp, discfile, 1, e_mesg, 1 )<0 )
		return(REPORT_ERR);

    	if( interaction ) {
		if(resp[0] == 'P')   /* number of copies to print */
			SetCopies( (int)copies );
	}

	pgcnt = 0;		/* Page count is zero */
	LNSZ = 131;		/* line size in no. of chars */
	linecnt = PGSIZE;	/* Page size in no. of lines */

	bufdate = 0;
	buftype[0] = '\0';
	buffund = 0;
	bufcode[0] = '\0';
	dateflag = typeflag = fundflag = codeflag = 0;

	retval = get_param( &param_rec, BROWSE, 1, e_mesg );
	if( retval<1 )
		return(DBH_ERR);
	sysdt = get_date();

    if( interaction ){
#ifdef ENGLISH
	if((retval=DisplayMessage("List on Date/Stock-Code range/Period (D/S/P)?"))<0 )
#else
	if((retval=DisplayMessage("Liste par date/gammes de code de stock/periode (D/S/P)?"))<0 )
#endif
		return(retval);
	for( ; ; ){
		if( (retval=GetResponse(resp))<0 )
			return(retval);
		if( resp[0]==LSTDATE || resp[0]==LSTSTCD ||resp[0] ==LSTPD )
			break;
	}
	if( *resp==LSTDATE || *resp == LSTPD){
		date1 = date2 = get_date();
		period1 = 0;
		period2 = 12;
		STRCPY(type1,"");
		STRCPY(type2,"ZZ");
		if (*resp == LSTPD){
		 	/* read in period  */
			if ((retval = GetPeriodRange(&period1,&period2)) < 0)	
				return(retval);
		}
		else{			
			if( (retval=GetDateRange(&date1,&date2))<0 )
				return(retval);
		}
		if( (retval=GetTypeRange(type1,type2))<0 )
			return(retval);
	}
	else{
		STRCPY(code1,""); STRCPY(code2,"ZZZZZZZZZZ");
		date1 = date2 = get_date();
		STRCPY(type1,""); STRCPY(type2,"ZZ");

		if( (retval=GetCodeRange(code1,code2))<0 )
			return(retval);
		if( (retval=GetDateRange(&date1,&date2))<0 )
			return(retval);
		if( (retval=GetTypeRange(type1,type2))<0 )
			return(retval);
	}
	if( (retval=Confirm())<=0 ) {
		close_rep(BANNER) ;
		return(retval);
	}
    }
    else {	/* non interactive call from end of day process */

	resp[0] = LSTDATE;	/* list on dates at end of day */
	date1 = date2 = get_date();
	STRCPY(type1,"");
	STRCPY(type2,"ZZ");
    }

	date_total = 0.00;
	stock_total = 0.00;
	final_total = 0.00;

	retval=PrintRep(interaction);

	if(retval==EFL)	retval=0;
	else if(retval==REPORT_ERR)
#ifdef ENGLISH
		STRCPY(e_mesg,"Internal report writing error");
#else
		STRCPY(e_mesg,"Erreur d'inscription au rapport interne");
#endif

	close_file(STTRAN);
	close_file(SCHOOL);
	close_rep(BANNER);

	return(retval);
}

static
PrintRep(interaction)
int	interaction;
{
	int	key, key_init;

	/* Initialize the stock transaction key as required */
	if( resp[0]==LSTDATE ){	/* List on date range */
		/* Initialize main key */
		st_tran.st_date = date1;
		STRCPY( st_tran.st_type, type1 );
		st_tran.st_seq_no = 0;
		key = 0;
	}
	if( resp[0]==LSTSTCD){
				/* List on stock code range */
		/* Initialize alternate key */
		st_tran.st_fund = 1;
		STRCPY( st_tran.st_code, code1 );
		st_tran.st_date = date1;
		STRCPY( st_tran.st_type, type1 );
		key = 1;
	}
	if( resp[0]==LSTPD){
				/* List on  specific period */
		/* Initialize alternate key */
		st_tran.st_date = 0;
		STRCPY( st_tran.st_type, type1 );
		st_tran.st_seq_no = 0;
		key = 0;
	}
	
	
	flg_reset( STTRAN );
	key_init = 0;	/* whenever flg_reset is done, this flag is set to 0 */
	for( ; ; ){
		if( key_init==1 ){
			flg_reset( STTRAN );
			key_init = 0;
		}
		retval = get_n_sttran( &st_tran,BROWSE,key,FORWARD,e_mesg );
		if( retval==EFL ){
			retval=0;
			break;
		}
		else if( retval<0 )
			return(DBH_ERR);
		if(key==0){	/* list on date range */
			if (resp[0] == LSTPD) {
				if (st_tran.st_period <  period1
					|| st_tran.st_period >  period2)
					continue;
			}
			else {
				if( st_tran.st_date>date2 )
					break;
			}
			if( strcmp(st_tran.st_type,type1)<0 ||
			    strcmp(st_tran.st_type,type2)>0 ){
				if( strcmp(st_tran.st_type,type2)>0 )
					st_tran.st_date++;
				STRCPY(st_tran.st_type,type1);
				key_init = 1;
				continue;
			}
		}
		if (key == 1) {		/*  list on stock code range */
			if( strcmp(st_tran.st_code,code1)<0 ||
			    strcmp(st_tran.st_code,code2)>0 ){
				if( strcmp(st_tran.st_code,code2)>0 )
					break;
				inc_str(st_tran.st_code,
					sizeof(st_tran.st_code)-1,FORWARD);
				st_tran.st_date = date1;
				STRCPY(st_tran.st_type,type1);
				key_init = 1;
				continue;
			}
			if(st_tran.st_date<date1||st_tran.st_date>date2 ){
				if( st_tran.st_date>date2 )
					inc_str( st_tran.st_code,
				   	     sizeof(st_tran.st_code)-1,
					 	FORWARD );
				st_tran.st_date = date1;
				STRCPY(st_tran.st_type,type1);
				key_init = 1;
				continue;
			}

			if( strcmp(st_tran.st_type,type1)<0 ||
			    strcmp(st_tran.st_type,type2)>0 ){
				if( strcmp(st_tran.st_type,type2)>0 )
					st_tran.st_date++;
				STRCPY(st_tran.st_type,type1);
				key_init = 0;
				flg_reset( STTRAN );
				continue;
			}
		}
		st_mast.st_fund=1;
		STRCPY(st_mast.st_code,st_tran.st_code);
		if( get_stmast(&st_mast,BROWSE,0,e_mesg)<0 ){
			STRCPY(st_mast.st_desc,"?????");
			STRCPY(st_mast.st_unit,"?????");
		}
		school.sc_numb=st_tran.st_location;
		if( get_sch(&school,BROWSE,0,e_mesg)<0 ){
			STRCPY(school.sc_name,"?????");
		}	
		if( resp[0]==LSTDATE || resp[0] == LSTPD){
			if( st_tran.st_date!=bufdate ){
				bufdate = st_tran.st_date;
				dateflag = 1;
				/* print daily total 	*/
				if (date_total != 0 ) {
					if ((retval = PrntDailyTotal()) < 0)
						 return(retval);
				}
			}
			else
				dateflag = 0;
			if( strcmp(st_tran.st_type,buftype) ){
				STRCPY( buftype,st_tran.st_type );
				typeflag = 1;
			}
			else
				typeflag = 0;
		}
		else{
			if( st_tran.st_fund!=buffund ){
				buffund = st_tran.st_fund;
				fundflag = 1;
			}
			else
				fundflag = 0;
			if( strcmp(st_tran.st_code,bufcode) ){
				STRCPY( bufcode,st_tran.st_code );
				codeflag = 1;
				if (stock_total != 0 ) {
					if ((retval = PrntStockTotal()) < 0)
						 return(retval);
				}
			}
			else
				codeflag = 0;
		}
		/* if linecount has equalled or exceeded page size */
		if( linecnt >= PGSIZE ){		/* If it has */
			if( pgcnt && term<99 ) {
				if( next_page()<0 )	return(ESCAPE);
			}
			if( pgcnt || term < 99 ) {   /* if not the first page */
				if( rite_top()<0 ) 
					return(REPORT_ERR);	/* form_feed */
				typeflag=dateflag=fundflag=codeflag=1;
			}
			else
				linecnt = 0;
			pgcnt++; 			/* increment page no */
			if( (retval=PrntHdg(interaction))<0 )
				return(retval);
		}

		/* Now print the record values */
		if( (retval=PrntRec())<0 )
			return(retval);

		final_total += st_tran.st_amount;
	}
	/* prints grand total */
	if (date_total != 0) {
		if ((retval = PrntDailyTotal()) < 0)
			 return(retval);
	}
	if (stock_total != 0) {
		if ((retval = PrntStockTotal()) < 0)
			 return(retval);
	}
	
	if(final_total != 0) {
		if ((retval = PrntTotal()) < 0)	
			return(retval);
	}
	if( pgcnt ){
		if( term<99 )
			last_page();
#ifndef SPOOLER
		else
			rite_top();
#endif
	}

	return(0);
}

static
PrntHdg(interaction)	/* Print heading  */
int	interaction;
{
	short offset;

	offset = ( LNSZ-strlen(param_rec.pa_co_name) )/2;
	mkln( offset, param_rec.pa_co_name, strlen(param_rec.pa_co_name) );
	if( prnt_line()<0 )	return(REPORT_ERR);
	
	mkln( 1, PROG_NAME, 10 );
#ifdef ENGLISH
	mkln( 53, "STOCK TRANSACTION LISTING", 25 );
#else
	mkln( 53, "LISTE DES TRANSACTIONS DE STOCK", 31 );
#endif
#ifdef ENGLISH
	mkln( 113, "PAGE: ", 6 );
#else
	mkln( 113, "PAGE: ", 6 );
#endif
	tedit( (char *)&pgcnt,"__0_",  line+cur_pos, R_SHORT ); 
	cur_pos += 4;
	if( prnt_line()<0 )	return(REPORT_ERR);

    if( interaction ){
	switch( resp[0] ){
		case LSTDATE:	/* sorted on dates */
#ifdef ENGLISH
			mkln( 58, "SORTED ON DATES", 15 );
#else
			mkln( 58, "TRIEES PAR DATES", 16 );
#endif
			break;
		case LSTSTCD:	/* sorted on stock codes */
#ifdef ENGLISH
			mkln( 55, "SORTED ON STOCK CODES", 21 );
#else
			mkln( 55, "TRIEES PAR CODES DE STOCK", 25 );
#endif
		case LSTPD:	/* sorted on stock codes */
#ifdef ENGLISH
			mkln( 57, "SORTED ON PERIOD",20);
#else
			mkln( 57, "TRIEES PAR PRIODE", 20 );
#endif
			break;
		default:
			break;
	}
    }
    else
#ifdef ENGLISH
	mkln( 59, "FOR THE DAY", 11 );

	mkln( 113, "DATE: ", 6 );
#else
	mkln( 59, "POUR LE JOUR", 12 );

	mkln( 113, "DATE: ", 6 );
#endif
	tedit( (char *)&sysdt,"____/__/__",  line+cur_pos,R_LONG ); 
	cur_pos += 10;
	if( prnt_line()<0 )	return(REPORT_ERR);
	if( prnt_line()<0 )	return(REPORT_ERR);

	if( resp[0]==LSTDATE  || resp[0] == LSTPD){
#ifdef ENGLISH
		mkln(4,"DATE",4);
		mkln(12,"TYPE",4);
		mkln(17,"SNo",3);
		mkln(26,"STOCK CODE",10);
#else
		mkln(4,"DATE",4);
		mkln(12,"GENRE",5);
		mkln(17,"NoS",3);
		mkln(26,"CODE STOCK",10);
#endif
	}
	else{
#ifdef ENGLISH
		mkln(6,"STOCK CODE",10);
		mkln(20,"DATE",4);
		mkln(28,"TYPE",4);
		mkln(33,"SNo",3);
#else
		mkln(6,"CODE STOCK",10);
		mkln(20,"DATE",4);
		mkln(27,"GENRE",5);
		mkln(33,"NoS",3);
#endif
	}
#ifdef ENGLISH
	mkln(39,"QUANTITY",8);
	mkln(50,"UNITS",5);
	mkln(60,"AMOUNT",6);
	mkln(73,"REMARKS",7); 
	mkln(105,"PO NUMBER",9); 
#else
	mkln(39,"QUANTITE",8);
	mkln(50,"UNITES",6);
	mkln(60,"MONTANT",7);
	mkln(73,"REMARQUES",9);
	mkln(105,"NUMERO DE BC",12); 
#endif
	if( prnt_line()<0 )	return(REPORT_ERR); 
#ifdef ENGLISH 
	mkln(25,"CC#",3); 
	mkln(31,"COST CENTRE NAME",16); 
	mkln(50,"PERIOD",6); 
	mkln(58,"FUND",4);
	mkln(64,"CREDIT ACCOUNT",14); 
	mkln(81,"FUND",4);
	mkln(86,"DEBIT ACCOUNT",13);
	mkln(104,"SUPPLIER",8);
	mkln(115,"REFERENCE",9);
#else
	mkln(25,"#CC",3);
	mkln(31,"NOM CENTRE COUTS",16);
	mkln(50,"PERIODE",7);
	mkln(58,"FOND",4);
	mkln(64,"COMPTE CREDITEUR",16);
	mkln(81,"FOND",4);
	mkln(86,"COMPTE DEBITEUR",15);
	mkln(103,"FOURNISSEUR",11);
	mkln(115,"REFERENCE",9);
#endif
	if( prnt_line()<0 )	return(REPORT_ERR);
	if( prnt_line()<0 )	return(REPORT_ERR);
	
	return(0);
}
static
PrntRec()
{
	cur_pos = 0;
	if((strcmp(st_tran.st_type, ISSUE) == 0) ||
	   (strcmp(st_tran.st_type, WRTOFF) == 0))
		st_tran.st_amount = -(st_tran.st_amount) ;

	if( resp[0]==LSTDATE || resp[0] == LSTPD){
	    date_total += st_tran.st_amount; 	
	    if( dateflag ){
		tedit( (char *)&st_tran.st_date, "____/__/__", 
					line+cur_pos, R_LONG );
		cur_pos += 10;
	    }
	    if( typeflag ){
		mkln(13,st_tran.st_type,2);
	    }
		mkln( 16, " ", 1 );
		tedit( (char *)&st_tran.st_seq_no, "_0_", 
					line+cur_pos, R_SHORT );
		cur_pos += 3;
		mkln(26,st_tran.st_code,10);
	}
	else{
	    if( codeflag ){
	    	stock_total += st_tran.st_amount; 	
		mkln(6,st_tran.st_code,10);
	    }
		mkln( 16, " ", 1 );
		tedit( (char *)&st_tran.st_date, "____/__/__", 
					line+cur_pos, R_LONG );
		cur_pos += 10;
		mkln( 29, st_tran.st_type, 2 );
		mkln( 32," ",1 );
		tedit( (char *)&st_tran.st_seq_no, "_0_", 
					line+cur_pos, R_SHORT );
		cur_pos += 3;
	}
	mkln( 36, " ", 1 );
	tedit( (char *)&st_tran.st_qty, "____0_.____-", 
				line+cur_pos, R_DOUBLE );


	cur_pos += 12;
	mkln( 50, st_mast.st_unit, 6 );
	mkln( 57, " ", 1 );
	tedit( (char *)&st_tran.st_amount, "______0_.__-", 
				line+cur_pos, R_DOUBLE );
	cur_pos += 12;
	mkln( 72, st_tran.st_remarks, 30 );

	if(st_tran.st_po_no != 0) {
		mkln( 104, " ", 1 );
		tedit( (char *)&st_tran.st_po_no,"_______0_",
				line+cur_pos,R_LONG);
		cur_pos += 9;
	}
	if( prnt_line()<0 )	return(REPORT_ERR);

	mkln( 22," ", 1 );
	if( st_tran.st_location!=HV_SHORT && st_tran.st_location!=0){
		tedit( (char *)&st_tran.st_location, "__0_", 
				line+cur_pos, R_SHORT);
		cur_pos += 4;
		mkln( 28,school.sc_name,20 );
	}
	if( st_tran.st_period!=HV_SHORT && st_tran.st_period!=0){
		mkln( 51, " ", 1 );
		tedit( (char *)&st_tran.st_period, "0_", 
				line+cur_pos, R_SHORT );
		cur_pos += 2;
	}
	if( st_tran.st_fund2 != HV_SHORT ) {
		mkln( 58, " ", 1 );
		tedit( (char *)&st_tran.st_fund2, "0_", 
					line+cur_pos, R_SHORT );
		cur_pos += 2;
	}
	if( st_tran.st_cr_acc[0]!=HV_CHAR )
		mkln( 62, st_tran.st_cr_acc, 18 );
	if( st_tran.st_fund != HV_SHORT ) {
		mkln( 81, " ", 1 );
		tedit( (char *)&st_tran.st_fund, "0_", 
					line+cur_pos, R_SHORT );
		cur_pos += 2;
	}
	if( st_tran.st_db_acc[0]!=HV_CHAR )
		mkln( 85, st_tran.st_db_acc, 18 );
	if( st_tran.st_suppl_cd[0]!=HV_CHAR )
		mkln( 104, st_tran.st_suppl_cd, 10 );
	if( st_tran.st_ref[0]!=HV_CHAR )
		mkln( 115, st_tran.st_ref, 12 );
	if( prnt_line()<0 )	return(REPORT_ERR);
	if( prnt_line()<0 )	return(REPORT_ERR);

	return(0);
}
PrntDailyTotal()
{
	mkln(40,"Daily Total: ",13);
	tedit( (char *)&date_total, "_______0_.__-", 
			line+cur_pos, R_DOUBLE );
	cur_pos += 13 ;
	if( prnt_line()<0 )	return(REPORT_ERR);
	if( prnt_line()<0 )	return(REPORT_ERR);
	cur_pos = 0 ;
	date_total = 0;
	return(0);
}
PrntStockTotal()
{
	mkln(40,"Stock Total: ",13);
	tedit( (char *)&stock_total, "_______0_.__-", 
			line+cur_pos, R_DOUBLE );
	cur_pos += 13 ;
	if( prnt_line()<0 )	return(REPORT_ERR);
	if( prnt_line()<0 )	return(REPORT_ERR);
	cur_pos = 0 ;
	stock_total = 0;
	return(0);
}
PrntTotal()
{
	if( prnt_line()<0 )	return(REPORT_ERR);
	mkln(40,"Grand Total: ",15);
	tedit( (char *)&final_total, "__,___,___,_0_.__-", 
			line+cur_pos, R_DOUBLE );
	cur_pos += 18;
	if( prnt_line()<0 )	return(REPORT_ERR);
	if( prnt_line()<0 )	return(REPORT_ERR);
	cur_pos = 0 ;
	final_total = 0 ;
	return(0);
}
