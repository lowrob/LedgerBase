/******************************************************************************
		Sourcename   : stclear.c 
		System       : Budgetary Financial system.
		Module       : Inventory System : Stock reports
		Created on   : 1992/09/22 
		Created  By  : Louis Robichaud
		Cobol Source : 

******************************************************************************
About the file:	
	The purpose of this program is to allow the stock transition file to be
	purged. It will accept a date from the user that must be a year older
 	than the system date. The deleted records will be printed out to a 
	file or to the printer depending on the choice of the user.

History:
Programmer      Last change on    Details

L.Robichaud	1993/10/13	Pass new parameter to close_rep function for 
				a banner to print with 3000 family.

******************************************************************************/

#include <stdio.h>
#include <reports.h>
#include <isnames.h>
#include <cfomstrc.h>

#define YES		5
#define ESCAPE		10
#define ONE_YEAR	10000	/* Amount to subtract to go back one year */ 

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

static St_tran	st_tran;
static Pa_rec 	param_rec;

extern char e_mesg[80];	/* for storing error messages */

static short	copies ;
static char	resp[2], code1[11], code2[11], type1[3], type2[3];
static	short	fund1, fund2, period1,period2;
static char discfile[15];	/* for storing outputfile name */
static int	retval;
static short	pgcnt; 		/* for page count */
static long sysdt, date;	/* system date and date range fields */
static short dateflag, typeflag, fundflag, codeflag;
static long	bufdate;
static char	buftype[3];
static short	buffund;
static char	bufcode[11];
static double 	date_total,stock_total,final_total;

/*----------------------------------------------------------------------*/
stclear()
{
	int i;

#ifdef ENGLISH
	STRCPY(resp,"P");
#else
	STRCPY(resp,"I");
#endif
    resp[0]='P';	/* set to Printer */
	if( (retval=GetResp(resp))<0 )
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
    if(resp[0] == 'P'){
        copies = 1;
	if((retval = GetNbrCopies( &copies )) < 0)
			return(retval);
    }
	if( opn_prnt( resp, discfile, 1, e_mesg, 1 )<0 )
		return(REPORT_ERR);

	if(resp[0] == 'P')   /* number of copies to print */
		SetCopies( (int)copies );
	

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
	/* Make default date at least 1 year old */
	date = sysdt-10000;

	for(;;){
		if((retval = GetDate(&date)) < 0) return(retval);
		if(date == 0) {
			date = HV_LONG;
			sprintf(e_mesg,"Date Cannot be Zero");
			fomen(e_mesg);
			continue;
		}
		if(date > (sysdt-ONE_YEAR)){
			date = HV_LONG;
			sprintf(e_mesg,"Date Must be Atleast One Year Old");
			fomen(e_mesg);
			continue;
		}

		if( (retval=Confirm())<=0 ) {
			close_rep(BANNER) ;
			return(retval);
		}
		else
			break;
	}

	date_total = 0.00;
	stock_total = 0.00;
	final_total = 0.00;

	retval = DoDeletion();
	if(retval < 0) return retval;

#ifdef ENGLISH
		STRCPY(e_mesg,"Internal report writing error");
#else
		STRCPY(e_mesg,"Erreur d'inscription au rapport interne");
#endif

	close_file(STTRAN);
	close_rep(BANNER);

	return(NOERROR);
}
/* end of  ->  stclear  */


/*------------------------------------------------------------------------*/
static 
DoDeletion()
{
static char  	text_buff[50];

st_tran.st_date = 0;
st_tran.st_type[0] = '\0';
st_tran.st_seq_no = 0;
flg_reset(STTRAN);

for(;;st_tran.st_seq_no ++){
	retval = get_n_sttran(&st_tran,UPDATE,0,FORWARD,e_mesg);
	if(retval < 0){
		return(retval);
	}
	if(date < st_tran.st_date) break;
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
		if( (retval=PrntHdg())<0 )
				return(retval);
	}
	if(PrintRec() < 0) return(ERROR);
	if(put_sttran(&st_tran,P_DEL,e_mesg) < 0){
		fomer("Error during Deletion");
		get();
	 	return ERROR;
	}
	if(commit(e_mesg) < 0){ 
		fomer("Error during commit");
		get();
		return(ERROR);
	}	
} /* End of for loop */

return(NOERROR);
} /* end of DoDeletion */
/*-------------------------------------------------------------------*/
static
PrintRec()
{
static char  	text_buff[50];
	if((strcmp(st_tran.st_type, ISSUE) == 0) ||
	   (strcmp(st_tran.st_type, WRTOFF) == 0))
		st_tran.st_amount = -(st_tran.st_amount) ;
	tedit( (char *)&st_tran.st_date, "____/__/__",text_buff,R_LONG); 
        mkln(1,text_buff,10);
        mkln(13,st_tran.st_type,2);
	tedit( (char *)&st_tran.st_seq_no, "0_",text_buff,R_SHORT); 
        mkln(18,text_buff,2);
	mkln(26,st_tran.st_code,10);
	tedit( (char *)&st_tran.st_qty, "____0_.____-", 
				text_buff, R_DOUBLE );
	mkln(38,text_buff,12);
	tedit( (char *)&st_tran.st_amount, "______0_.__-", 
				text_buff, R_DOUBLE );
	mkln(52,text_buff,12);
	mkln( 72, st_tran.st_remarks, 30 );
	if(st_tran.st_po_no != 0) {
		tedit( (char *)&st_tran.st_po_no,"_______0_",
				text_buff,R_LONG);
		mkln (105,text_buff,9);
	}

	if( prnt_line()<0 )	return(REPORT_ERR);
	
	if( st_tran.st_location!=HV_SHORT && st_tran.st_location!=0){
		tedit( (char *)&st_tran.st_location, "0_", 
				text_buff, R_SHORT );
		mkln(25,text_buff,2);
	}
	if( st_tran.st_period!=HV_SHORT && st_tran.st_period!=0){
		tedit( (char *)&st_tran.st_period, "0_", 
				text_buff, R_SHORT );
		mkln( 51,text_buff, 2 );
	}
	if( st_tran.st_fund2 != HV_SHORT ) {
		tedit( (char *)&st_tran.st_fund2, "0_", 
					text_buff, R_SHORT );
		mkln( 58,text_buff, 2 );
	}
	if( st_tran.st_cr_acc[0]!=HV_CHAR )
		mkln( 62, st_tran.st_cr_acc, 18 );
	if( st_tran.st_fund != HV_SHORT ) {
		tedit( (char *)&st_tran.st_fund, "0_", 
					line+cur_pos, R_SHORT );
		mkln( 81,text_buff, 2 );
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
} /* End of PrintRep  */
/*-------------------------------------------------------------------*/
static
PrntHdg()	/* Print heading  */
{
	short offset;

	offset = ( LNSZ-strlen(param_rec.pa_co_name) )/2;
	mkln( offset, param_rec.pa_co_name, strlen(param_rec.pa_co_name) );
	if( prnt_line()<0 )	return(REPORT_ERR);
	
	mkln( 1, PROG_NAME, 10 );
#ifdef ENGLISH
	mkln( 53, "PURGED STOCK TRANSACTION REPORT", 31);
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

#ifdef ENGLISH
	mkln( 58, "SORTED ON DATES", 15 );
#else
	mkln( 58, "TRIEES PAR DATES", 16 );
#endif

	tedit( (char *)&sysdt,"____/__/__",  line+cur_pos,R_LONG ); 
	cur_pos += 10;
	if( prnt_line()<0 )	return(REPORT_ERR);
	if( prnt_line()<0 )	return(REPORT_ERR);

#ifdef ENGLISH
		mkln(4,"DATE",4);
		mkln(12,"TYPE",4);
		mkln(18,"SNo",3);
		mkln(26,"STOCK CODE",10);
#else
		mkln(4,"DATE",4);
		mkln(12,"GENRE",5);
		mkln(18,"NoS",3);
		mkln(26,"CODE STOCK",10);
#endif

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
/*------------------    END   OF   PROGRAM    ----------------------------*/
