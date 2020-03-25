/******************************************************************************
		Sourcename    : artrlist.c
		System        : Budgetary Financial system.
		Module        : Accounts Receivable reports
		Created on    : 89-11-23
		Created  By   : Jonathan Prescott
		Cobol sources : 
*******************************************************************************
About this file:
	This routine generates one of the following :
		1. List of Accounts Receivable Transactions 
	Calling file:
-----------------------------------------------------------------------------	
Modified:	Frank Tao
Date:		Nov. 22, 1990
Description:	Add option to allow user selecting only outstanding invoices 
		for the listing.

-----------------------------------------------------------------------------	
Modified:	C.Leadbeater
Date:		Nov. 29, 1990
Description:	Allow the user to enter a specific range for the customers
		to include on the report by entering the customer codes.	
-----------------------------------------------------------------------------	
Modified:	Frank Tao
Date:		Dec. 27, 1990
Description:	Change tax amount calculation.  
-----------------------------------------------------------------------------	

1992/07/27	J McLean       Changed customer code from 6 to 10 characters.
-----------------------------------------------------------------------------	
Modified:	Louis Robichaud
Date:		May 21, 1993
Description:	Removed the page break for each change in transaction dates
		and add a sub total.

L.Robichaud	1993/10/13	Pass new parameter to close_rep function for 
				a banner to print with 3000 family.
******************************************************************************/
#define	EXIT	12		/* as defined in apreputl.c */

#include <stdio.h>
#include <repdef.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define DATE_FMT	"____/__/__"
#define AMT_FMT		"______0_.__-"

#define OPEN		'O'

#ifdef ENGLISH
#define T_INVOICE	"IN"
#define T_CRMEMO	"CM"
#define YES		'Y'
#define PRINTER 	'P'
#define DISPLAY		'D'
#define FILE_IO		'F'
#else
#define T_INVOICE	"FC"
#define T_CRMEMO	"ND"
#define YES		'O'
#define PRINTER		'I'
#define DISPLAY		'A'
#define FILE_IO		'D'
#endif

static int 	pgcnt;
static long	longdate ;
static int	retval;

static Ar_hdr	ar_hdr;
static Ar_item	ar_item;
static Pa_rec	pa_rec;

static char 	discfile[20] ;
static char	outdev[2];	
static char	program[11] ;

static long	prev_trandt;
static short	prev_fund;
static long	prev_inv_no;

static long	transdt1;
static long	transdt2;
static short	fund1;
static short	fund2;
static long	invcno1;
static long	invcno2;
static char 	outstanding[2];
static char	custcd1[11];
static char	custcd2[11];  /* used to specify customer code range. (CL) */
static short	copies;
static	double	total_amt ;
static	double	item_sub ;
extern char e_mesg[80];
double 	D_Roundoff();

Trans_list(mode) 
int	mode;		/* 0 if interactive, 1 if day end process */
{
	discfile[0] = '\0';

	STRCPY(program, PROG_NAME);

#ifdef ENGLISH
 	STRCPY( outdev, "P" );
#else
 	STRCPY( outdev, "I" );
#endif

	if(mode == 0) {		/* interactive report */
		if((retval = GetOutputon( outdev )) <0)
			return( retval );
	}

	switch (*outdev) {
		case DISPLAY :	/*  Display on Terminal */
			outdev[0]='D'; /* must be set to 'D'*/
			STRCPY(discfile,terminal);
			break;
		case FILE_IO : 	/*  Print to a disk file */ 
			outdev[0]='F'; /* must be set to 'F' */
			STRCPY( e_mesg, "status.dat");
			if((retval = GetFilename(e_mesg))<0)
				return(retval);
			STRCPY (discfile, e_mesg) ;
			break;
		case PRINTER : 	/*  Print to a printer */ 
		default  :
			outdev[0]='P'; /* must be set to 'P' */
			discfile[0] = '\0';
			break;
	}

	copies = 1;
	if( *outdev=='P' ){
		if((retval = GetNbrCopies( &copies )) <0)
			return( retval );
	}
	/* set default ranges */
	longdate =  transdt1 = transdt2 = get_date();
	fund1 = 1;
	fund2 = 999;
	invcno1 = 0;
	invcno2 = 99999999;

	if(mode == 0) {		/* interactive report */
		/* get transaction date range */
		retval = GetTransDateRange(&transdt1, &transdt2);
		if(retval < 0)	return(retval);

		/* get fund number range */
		retval = GetFundRange(&fund1, &fund2);
		if(retval < 0)	return(retval);
	
		/* get invoice number range */
		retval = GetInvcRange(&invcno1, &invcno2);
		if(retval < 0)	return(retval);

		/* get range for customer code from user (CL-90/11/29) */
		
		STRCPY(custcd1,"         1");
		STRCPY(custcd2,"ZZZZZZZZZZ");
		retval = GetCNbrRange( custcd1, custcd2 );
		if(retval < 0) return(retval);

/* Added by F.Tao, 90/11/22 						*/
#ifdef ENGLISH
		DisplayMessage("Do you want only outstanding Invoices (Y/N)?");
		GetResponse(outstanding, "YN");
#else
		DisplayMessage("Desirez-vous seulement les factures non-reglees (O/N)?");
		GetResponse(outstanding, "ON");
#endif

		if ( (retval=Confirm())<= 0) 
			return(retval);
	}

	if(opn_prnt(outdev,discfile,1,e_mesg,1) < 0) {
		return(REPORT_ERR) ;
	}
	if( *outdev=='P' )
		SetCopies( (int)copies );

	/** set page size parameters **/
	pgcnt = 0;
	LNSZ = 133;
	linecnt = PGSIZE;

	if( get_param(&pa_rec,BROWSE,1,e_mesg)<1 ){
		return(DBH_ERR);
	}
	
	ar_hdr.ah_trandt = transdt1;
	ar_hdr.ah_fund = fund1;
	ar_hdr.ah_inv_no = invcno1;
	ar_hdr.ah_sno = 0;
	
	prev_trandt = 0;
	prev_fund = 0;
	prev_inv_no = 0;

	flg_reset(ARSHDR);
	for(;;) {
		retval = get_n_arhdr(&ar_hdr,BROWSE,2,FORWARD,e_mesg);
		if(retval < 0) {
			if(retval == EFL) break;
			retval = DBH_ERR; 
			break;
		}

		if(ar_hdr.ah_trandt > transdt2)
			break;

		if(ar_hdr.ah_fund < fund1 || ar_hdr.ah_fund > fund2) {
			if(ar_hdr.ah_fund > fund2) {
				ar_hdr.ah_trandt++;
			}
			ar_hdr.ah_fund = fund1;
			ar_hdr.ah_inv_no = invcno1;
			flg_reset(ARSHDR);
			continue;
		}

		if(ar_hdr.ah_inv_no < invcno1 || ar_hdr.ah_inv_no > invcno2) {
			if(ar_hdr.ah_inv_no > invcno2) {
				ar_hdr.ah_fund++;
			}
			ar_hdr.ah_inv_no = invcno1;
			ar_hdr.ah_sno = 0;
			flg_reset(ARSHDR);
			continue;
		}

		/* check that a.r. header customer code is within range (CL) */
	
		if (strcmp(ar_hdr.ah_cu_code, custcd1) < 0) continue;
		if (strcmp(custcd2, ar_hdr.ah_cu_code) < 0) continue;

		if (outstanding[0] == YES && ar_hdr.ah_status[0] != OPEN)
			continue;

/*louis 	if(linecnt >= PGSIZE-1 || ar_hdr.ah_trandt != prev_trandt) { */
		if(linecnt >= PGSIZE-1) {
			retval = print_heading(mode);
			if(retval == REPORT_ERR || retval == EXIT) break;
		}
/*louis*/	if(ar_hdr.ah_trandt != prev_trandt) { 
			if(prev_trandt != 0){
/*louis*/			retval = print_sub_totals();
/*louis*/				if(retval==REPORT_ERR || retval==EXIT)
/*louis*/				 	break;
			}
			prev_trandt = ar_hdr.ah_trandt; /* set last tran date */
			prev_fund = 0;    /* reset previous values */
			prev_inv_no = 0;  /* to stop suppression */
		}
		if(print_invoice_line() < 0) break; 
		ar_item.ai_fund = ar_hdr.ah_fund;
		ar_item.ai_inv_no = ar_hdr.ah_inv_no;
		ar_item.ai_hno = ar_hdr.ah_sno;
		ar_item.ai_sno = 0;
		flg_reset(ARSITEM);
		for(;;){
#ifndef ORACLE
			retval = get_n_aritem(&ar_item,BROWSE,0,FORWARD,e_mesg);
#else
			retval = get_n_aritem(&ar_item,BROWSE,0,EQUAL,e_mesg);
#endif
			if(retval < 0) {
				if(retval == EFL) break;
				retval = DBH_ERR; 
				break;
			}
#ifndef ORACLE
			if(ar_item.ai_fund != ar_hdr.ah_fund ||
				ar_item.ai_inv_no != ar_hdr.ah_inv_no ||
				ar_item.ai_hno != ar_hdr.ah_sno) break;
#endif
			if(linecnt >= PGSIZE-1) {
				retval = print_heading(mode);
				if(retval == REPORT_ERR || retval == EXIT)
					 break;
				prev_fund = 0;    /* reset previous values */
				prev_inv_no = 0;  /* to stop suppression */
				/*if(print_invoice_line() < 0) break;*/ 
       	/* nicola - last line was duplicating on first line of next page*/	
			}
			if(print_item_line() < 0) break;
		}
		if(retval < 0 && retval != EFL) 
			 break;
	}
	if(pgcnt) {
		print_sub_totals();
		print_totals();
		if(term < 99)
			last_page();
#ifndef SPOOLER
		else
			rite_top();
#endif
	}

	close_rep(BANNER) ;
	close_file(ARSHDR) ;
	close_file(ARSITEM) ;
	if(retval == REPORT_ERR) 
#ifdef ENGLISH
		sprintf(e_mesg,"Internal report writing error");
#else
		sprintf(e_mesg,"Erreur d'inscription au rapport interne");
#endif
	if(retval < 0 && retval != EFL) return(retval);

	return(0);	
}

print_heading(mode)
int	mode ;
{
	char	txt_line[80];
	int	offset ;

	if(term < 99 && pgcnt) /* if not first page and display */
		if(next_page() < 0) return(EXIT);

	if(pgcnt || term < 99) {
		if(rite_top() < 0) return(REPORT_ERR);
	}
	else linecnt = 0;
	pgcnt++;

	mkln(1,program,10);
	offset = ( LNSZ-strlen(pa_rec.pa_co_name) )/2;
	mkln( offset, pa_rec.pa_co_name, strlen(pa_rec.pa_co_name) );
#ifdef ENGLISH
        mkln(115,"DATE:",5);
#else
        mkln(115,"DATE:",5);
#endif
	tedit((char *)&longdate,DATE_FMT,txt_line,R_LONG);
	mkln(121,txt_line,10);
	if(prnt_line() < 0) return(REPORT_ERR);

#ifdef ENGLISH
	mkln(54,"SALES TRANSACTION LISTING",25);
	mkln(115,"PAGE:",5);
#else
	mkln(51,"LISTE DES TRANSACTIONS DE VENTE",31);
	mkln(115,"PAGE:",5);
#endif
	tedit((char *)&pgcnt,"__0_",txt_line,R_INT);
	mkln(121,txt_line,4);
	if(prnt_line() < 0) return(REPORT_ERR);

	if(mode == 0) {			/* INTERACTIVE */
#ifdef ENGLISH
		mkln(50,"FROM",4);
#else
		mkln(52,"DE",2);
#endif
		tedit((char *)&transdt1,DATE_FMT,txt_line,R_LONG);
		mkln(cur_pos+2,txt_line,10);
#ifdef ENGLISH
		mkln(cur_pos+3,"TO",2);
#else
		mkln(cur_pos+3,"A",1);
#endif
		tedit((char *)&transdt2,DATE_FMT,txt_line,R_LONG);
		mkln(cur_pos+3,txt_line,10);
	}
#ifdef ENGLISH
	else	mkln(61,"FOR THE DAY",11);
#else
	else	mkln(59,"POUR LA JOURNEE",15);
#endif
	if(prnt_line() < 0) return(REPORT_ERR);
	if(prnt_line() < 0) return(REPORT_ERR);

#ifdef ENGLISH
	mkln(1,"TRANS DATE:",11);
#else
	mkln(1,"DATE TRANS:",11);
#endif
	tedit((char *)&ar_hdr.ah_trandt,DATE_FMT,txt_line,R_LONG);
	mkln(13,txt_line,10);
	if(prnt_line() < 0) return(REPORT_ERR);
	if(prnt_line() < 0) return(REPORT_ERR);

#ifdef ENGLISH
	mkln(1,"FUND",4);
	mkln(7,"INVOICE",7);
	mkln(16,"TYPE",4);
	mkln(22,"SNO",3);
	mkln(27,"CUSTOMER",8);
	mkln(38,"DUE DATE",8);
	mkln(52,"NET AMOUNT",10);
	mkln(65,"TAX AMT",7);
	mkln(79,"TOTAL",5);
	mkln(90,"PERIOD",6);
	mkln(98,"STATUS",6);
	mkln(113,"REMARKS",7);
#else
	mkln(1,"FOND",4);
	mkln(7,"FACTURE",7);
	mkln(16,"GENRE",5);
	mkln(22,"NoS",3);
	mkln(27,"CLIENT",6);
	mkln(38,"ECHEANCE",8);
	mkln(52,"MONT NET",8);
	mkln(65,"MONT TAXE",9);
	mkln(79,"MONT TOTAL",10);
	mkln(90,"PERIODE",7);
	mkln(98,"ETAT",4);
	mkln(113,"REMARQUES",9);
#endif
	if(prnt_line() < 0) return(REPORT_ERR);
	
#ifdef ENGLISH
	mkln(94,"ITEM#",5);
	mkln(104,"G/L ACCOUNT",11);
	mkln(122,"ITEM AMT",8);
#else
	mkln(94,"#ARTICLE",8);
	mkln(104,"COMPTE G/L",10);
	mkln(122,"MONTANT ART",11);
#endif
	if(prnt_line() < 0) return(REPORT_ERR);
	if(prnt_line() < 0) return(REPORT_ERR);

	return(NOERROR);
}

print_invoice_line()
{
	char txt_line[80];
	double temp_taxamt;
	short	flg;
	double	temp_total;
	double	temp_bal;

	if(strcmp(ar_hdr.ah_type,T_CRMEMO) == 0)
		flg = -1;
	else
		flg = 1;

	if(strcmp(ar_hdr.ah_type,T_INVOICE) != 0 && ar_hdr.ah_sno != 1) {
		temp_total = (ar_hdr.ah_oriamt * flg);
	}
	else {
		temp_total = (ar_hdr.ah_netamt * flg);
	}
	if(prnt_line() < 0) return(REPORT_ERR);
	if(ar_hdr.ah_fund != prev_fund) { 
		tedit((char *)&ar_hdr.ah_fund,"_0_",txt_line,R_SHORT);
		mkln(2,txt_line,3);
		prev_fund = ar_hdr.ah_fund; 
	}
	if(ar_hdr.ah_inv_no != prev_inv_no) {
		tedit((char *)&ar_hdr.ah_inv_no,"______0_",txt_line,R_LONG);
		mkln(6,txt_line,8);
		prev_inv_no = ar_hdr.ah_inv_no;
	}
	mkln(17,ar_hdr.ah_type,2);
	tedit((char *)&ar_hdr.ah_sno,"_0_",txt_line,R_SHORT);
	mkln(22,txt_line,3);
	mkln(26,ar_hdr.ah_cu_code,10);
	if(strcmp(ar_hdr.ah_type,T_INVOICE) == 0) {
		tedit((char *)&ar_hdr.ah_duedt,DATE_FMT,txt_line,R_LONG);
		mkln(38,txt_line,10);
	}
	temp_bal = ar_hdr.ah_oriamt * flg;
	tedit((char *)&temp_bal,AMT_FMT,txt_line,R_DOUBLE);
	mkln(50,txt_line,12);
	if(strcmp(ar_hdr.ah_type,T_INVOICE) == 0 || ar_hdr.ah_sno == 1) {
		temp_taxamt = (ar_hdr.ah_txamt + ar_hdr.ah_gstamt) * flg;
		temp_taxamt = D_Roundoff(temp_taxamt);  
		tedit((char *)&temp_taxamt,AMT_FMT,txt_line,R_DOUBLE);
		mkln(63,txt_line,12);
	}
	tedit((char *)&temp_total,"______0_.__-",txt_line,R_DOUBLE);
	mkln(76,txt_line,12);
	
	tedit((char *)&ar_hdr.ah_period,"0_",txt_line,R_SHORT);
	mkln(92,txt_line,2);
	mkln(101,ar_hdr.ah_status,1);
	mkln(106,ar_hdr.ah_remarks,20);
	if(prnt_line() < 0) return(REPORT_ERR);
	
	item_sub += temp_total ; 
	total_amt += temp_total ; 
	
	return(NOERROR);
}

print_item_line()
{
	char txt_line[80];
	double	amount;
	int	flg;

	if(strcmp(ar_hdr.ah_type,T_CRMEMO) == 0)
		flg = -1;
	else
		flg = 1;

	tedit((char *)&ar_item.ai_sno,"_0_",txt_line,R_SHORT);
	mkln(95,txt_line,3);
	mkln(100,ar_item.ai_accno,18);
	amount = ar_item.ai_amount * flg;
	tedit((char *)&amount,AMT_FMT,txt_line,R_DOUBLE);
	mkln(120,txt_line,12);
	if(prnt_line() < 0) return(REPORT_ERR);
	
	return(NOERROR);
}

print_sub_totals()
/* prints the sub total, added by L.R. */ 
{
	char txt_line[80];

	if(prnt_line() < 0) return(REPORT_ERR);
	mkln(64,"SUB TOTAL",9);
	tedit((char *)&item_sub,"_______0_.__-",txt_line,R_DOUBLE);
	mkln(75,txt_line,13);
	if(prnt_line() < 0) return(REPORT_ERR);
	if(prnt_line() < 0) return(REPORT_ERR);
	item_sub = 0 ;
	if(retval == !EFL){
#ifdef ENGLISH
		mkln(1,"TRANS DATE:",11);
#else
		mkln(1,"DATE TRANS:",11);
#endif
		tedit((char *)&ar_hdr.ah_trandt,DATE_FMT,txt_line,R_LONG);
		mkln(13,txt_line,10);
		if(prnt_line() < 0) return(REPORT_ERR);
	}
		
	
	return(NOERROR);
}

print_totals()
{
	char txt_line[80];

	if(prnt_line() < 0) return(REPORT_ERR);
	if(prnt_line() < 0) return(REPORT_ERR);
	mkln(68,"TOTAL",5);
	tedit((char *)&total_amt,"_______0_.__-",txt_line,R_DOUBLE);
	mkln(75,txt_line,13);
	if(prnt_line() < 0) return(REPORT_ERR);
	total_amt = 0 ;
	
	return(NOERROR);
}

