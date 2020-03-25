/******************************************************************************
		Sourcename   : agecust.c
		System       : Budgetary Financial system.
		Module       : ARS 
		Created on   : 89-12-08
		Created  By  : K HARISH.
		Cobol Source : 

******************************************************************************
About the file:	
	This routine generates a report on aged outstanding invoices by customer

	For a given range of customers, details of all the outstanding invoices
	including the ageing details are reported.

	It is called by the file arrep.c

History:
Programmer      Last change on    Details

K.HARISH__      1989/12/08
J. McLean	1992/07/22	  Added code to handle Unapplied receipts
				  and Credit Memos.  Changed customer code 
				  from 6 to 10 characters.
J. McLean	1992/07/28	  Summary or Detail Report Option.
S.Whittaker	1993/03/15	  Summary required total for customer.
S.Whittaker	1993/05/12	  If a receipt was applied to multiply invoices
				  the total was applied against the invoice.
				It needed to be changed to the correct applied
				amount.

L.Robichaud	1993/10/13	Pass new parameter to close_rep function for 
				a banner to print with 3000 family.
*****************************************************************************/

#include <stdio.h>
#include <reports.h>

#define	ESCAPE	10

#ifdef ENGLISH
#define PRINTER 	'P'
#define DISPLAY 	'D'
#define FILE_IO		'F'
#define SUMMARY		'S'
#else
#define PRINTER 	'I'
#define DISPLAY 	'A'
#define FILE_IO		'D'
#define SUMMARY		'R'
#endif

#define APPLIED		'A'
#define DETAIL		'D'

extern char 	e_mesg[80];	/* for storing error messages */

static char	resp[2];	/* for storing user's response */
static double	tot_bal, tslot[4];	/* to store aged outstanding amounts */
static double	grand_tot, grand_tslot[4];/* to store grand totals */
static char 	discfile[15];	/* for storing outputfile name */
static int	retval, index;
static short	pgcnt; 		/* for page count */
static short	copies;		/* number of copies to print */
static long 	sysdt, age;	/* system date */
static char	bufcust[11], cust1[11], cust2[11];
static char	type[2];	/* report type - summary or detail */
static Ar_hdr	ar_hdr;		/* invoice header record */
static Pa_rec	param_rec;	/* parameter record */
static Rcpt_hdr	rcpt_hdr;	/* rcpt header record */
static Rcpt_item	rcpt_itm;	/* rcpt item record */
static Cu_rec	cu_rec;

static double	rcpt_total;
static double	balance;
static int	cnt;

/*---------------------------------------------------------------------------*/

agecust()
{
	/* Get the parameter record */
	if( (retval=get_param(&param_rec, BROWSE, 1, e_mesg))<1 )
		return(DBH_ERR);

	/* Accept output medium details */
#ifdef ENGLISH
	strcpy(resp,"P");
#else
	strcpy(resp,"I");
#endif
	if( (retval=GetOutputon(resp))<0 )
		return(retval);

	switch(*resp) {
		case DISPLAY:
			resp[0]='D';
			strcpy( discfile, terminal );
			break;
		case FILE_IO:
			resp[0]='F';
			strcpy( discfile, "agecust.dat" );
			if( (retval=GetFilename(discfile))<0 )
				return(retval);
			break;
		case PRINTER:
		default:
			resp[0]='P';
			discfile[0]='\0';
			break;

	}

	copies = 1;	 /* set number of copies to 1 */

	if(resp[0]=='P') {
		if((retval = GetNbrCopies( &copies ))<0)
			return(retval);
	}

	sysdt = get_date();		/* store system date */

	/* Accept range of customer codes */
	strcpy( cust1, "         1");
	strcpy( cust2, "ZZZZZZZZZZ");
	if( GetCNbrRange( cust1, cust2 )<0 )
		return(-1);
	
	/*  Get Report Type - Summary or Detail  */
	for( ; ; ) {
		type[0] = DETAIL;
		if( GetReportType( type ) < 0)
			return(-1);
#ifdef ENGLISH
		if( type[0] == SUMMARY || type[0] == DETAIL)
#else
		if( type[0] == SUMMARY || type[0] == DETAIL)
#endif
			break;
	}

	/* Let user confirm entries */
	if( (retval=Confirm())<=0 )
		return(retval);

	/* Open the output file */
	if( opn_prnt( resp, discfile, 1, e_mesg, 1 )<0 ){
#ifdef ENGLISH
		sprintf(e_mesg,"Error in opening output medium");
#else
		sprintf(e_mesg,"Erreur en ouvrant l'instrument de sortie des donnees");
#endif
		return(REPORT_ERR);
	}


	if(resp[0] == 'P') 
		SetCopies( (int)copies );

	/* Initialize printing variables */
	pgcnt = 0;		/* Page count is zero */
	LNSZ = 131;		/* line size in no. of chars */
	linecnt = PGSIZE;	/* Page size set by opn_prnt in no. of lines */

	/* Initialize aged outstanding totals to zeros */
	tot_bal = tslot[0] = tslot[1] = tslot[2] = tslot[3] = 0.0;

	/* Initialize grand totals to zero */
	grand_tot = grand_tslot[0] = grand_tslot[1] = grand_tslot[2] = 
						grand_tslot[3] = 0.0;

	/* Scan the invoice header file and print all relevant records */
	retval = PrintRep();

	/* if output is on display, display last page msg, else formfeed */
	if(term<99 && pgcnt)
		last_page();

	/* Close report writer and database */
	close_dbh();
	close_rep(BANNER);

	if(retval==REPORT_ERR)
#ifdef ENGLISH
		sprintf(e_mesg,"Internal report writing error");
#else
		sprintf(e_mesg,"Erreur d'inscription au rapport interne");
#endif
	return(NOERROR);
}

/*---------------------------------------------------------------------------*/
/* Scan the invoice header file and print all relevant records */
static
PrintRep()
{
	int	first_invc;	/* First invoice for customer flag */
	int	invoice_complete; /* flag to see if invoice is to be printed */
	long	prev_invc;	
	int 	daysinsys;	/* Records the number of days in the system */

	bufcust[0] = '\0';

	/* Copy customer code to a buffer to detect change in cust code */
	strcpy(cu_rec.cu_code, cust1) ;
	flg_reset(CUSTOMER);

	for( ; ; ) {
		cnt = 0;
		balance = 0;
		retval = get_n_cust(&cu_rec,BROWSE,0,FORWARD,e_mesg);
		if( retval<0 && retval!=EFL )
			return(retval);

		if( retval==EFL || strcmp( cu_rec.cu_code, cust2 )>0 ){
			break;
		}

		/* Initialize alternate key */
		strcpy( ar_hdr.ah_cu_code, cu_rec.cu_code );
		ar_hdr.ah_fund = 0;
		ar_hdr.ah_inv_no = 0;
		ar_hdr.ah_sno = 0;
		flg_reset( ARSHDR );

		first_invc = 0;
		invoice_complete = 0;
		/* Read records sequentially and print them (in a loop) */
		for( ; ; ) {
			retval = get_n_arhdr( &ar_hdr,BROWSE,1,FORWARD,e_mesg );
	
			if( retval<0 && retval!=EFL )
				return(retval);
	
			if(strcmp(ar_hdr.ah_cu_code, cu_rec.cu_code) != 0 ||
			   retval == EFL) {
				break;
			}

			/* if invoice is complete and more than 31 days old */
			/* skip all related DM/CM */
			daysinsys = days(get_date())-days(ar_hdr.ah_trandt);
			if(ar_hdr.ah_status[0]==COMPLETE && daysinsys>31
			 && ar_hdr.ah_sno == 1){
				ar_hdr.ah_inv_no++;
				ar_hdr.ah_sno = 0;
				flg_reset(ARSHDR);
				continue;
			}

			if(first_invc == 0) {
				prev_invc = ar_hdr.ah_inv_no;
			}
		
			if(linecnt >= PGSIZE-4) {
				if((retval = PrntHdg())<0)
					return(retval);
				bufcust[0] = '\0';
			}

			if(strcmp(bufcust,cu_rec.cu_code) != 0) {
				if((retval = pr_cust()) < 0) 
					return(retval);
				strcpy(bufcust,cu_rec.cu_code);
			}

			if(prev_invc != ar_hdr.ah_inv_no && first_invc !=0) {
				if(CheckForRcpt(cu_rec.cu_code,prev_invc)<0) 
					return(ERROR);
			}
				
			/* Now print the record values */
			if( (retval=PrntRec())<0 )
				return(retval);
			first_invc = -1;
			prev_invc = ar_hdr.ah_inv_no;
		}

		if(prev_invc != ar_hdr.ah_inv_no && first_invc !=0) {
			if(CheckForRcpt(cu_rec.cu_code,prev_invc)<0) 
				return(ERROR);
		}

		if((retval = GetUnapplied()) < 0) {
			return(retval);
		}

		if(cnt != 0) { 
			if((retval = pr_subtot()) < 0) {
				return(retval);
			}
		} 
	}

	if(grand_tot != 0.00) {
		if((retval = pr_grandtot()) < 0) 
			return(retval);
	}

#ifndef	SPOOLER
	if( rite_top()<0 )
		return(REPORT_ERR);
#endif

	return(0);
}
/*---------------------------------------------------------------------------*/
/* Given the age in days, return the slot# to which the age belongs */
findslot( age_in_days )
long	age_in_days;
{
	int	index;

	if	( age_in_days<31 ) 	index=0;
	else if	( age_in_days<61 )	index=1;
	else if	( age_in_days<91 )	index=2;
	else 				index=3;

	return( index );
}
/*---------------------------------------------------------------------------*/
/*	Print subtotal line */
pr_subtot()
{
	if( prnt_line()<0 )	return(REPORT_ERR);
#ifdef ENGLISH
	mkln( 34, "TOTAL", 5 );
#else
	mkln( 34, "TOTAL", 5 );
#endif
	mkln( 66," ",1 );
	tedit( (char *)&tot_bal,"______0_.__-",line+cur_pos,R_DOUBLE );
	cur_pos += 12;
	mkln( 79," ",1 );
	tedit( (char *)&tslot[0],"______0_.__-",line+cur_pos,R_DOUBLE );
	cur_pos += 12;
	mkln( 92," ",1 );
	tedit( (char *)&tslot[1],"______0_.__-",line+cur_pos,R_DOUBLE );
	cur_pos += 12;
	mkln( 105," ",1 );
	tedit( (char *)&tslot[2],"______0_.__-",line+cur_pos,R_DOUBLE );
	cur_pos += 12;
	mkln( 118," ",1 );
	tedit( (char *)&tslot[3],"______0_.__-",line+cur_pos,R_DOUBLE );
	cur_pos += 12;
	if( prnt_line()<0 )	return(REPORT_ERR);
	if( prnt_line()<0 )	return(REPORT_ERR);

	tot_bal=tslot[0]=tslot[1]=tslot[2]=tslot[3]=0.0;

	return(0);
}
/*---------------------------------------------------------------------------*/
/*	Print grand total line */
pr_grandtot()
{
	if( prnt_line()<0 )	return(REPORT_ERR);
#ifdef ENGLISH
	mkln( 28, "GRAND TOTAL", 11 );
#else
	mkln( 28, "GRAND TOTAL", 11 );
#endif
	mkln( 66," ",1 );
	tedit( (char *)&grand_tot,"______0_.__-",line+cur_pos,R_DOUBLE );
	cur_pos += 12;
	mkln( 79," ",1 );
	tedit( (char *)&grand_tslot[0],"______0_.__-",line+cur_pos,R_DOUBLE );
	cur_pos += 12;
	mkln( 92," ",1 );
	tedit( (char *)&grand_tslot[1],"______0_.__-",line+cur_pos,R_DOUBLE );
	cur_pos += 12;
	mkln( 105," ",1 );
	tedit( (char *)&grand_tslot[2],"______0_.__-",line+cur_pos,R_DOUBLE );
	cur_pos += 12;
	mkln( 118," ",1 );
	tedit( (char *)&grand_tslot[3],"______0_.__-",line+cur_pos,R_DOUBLE );
	cur_pos += 12;
	if( prnt_line()<0 )	return(REPORT_ERR);
	if( prnt_line()<0 )	return(REPORT_ERR);

	return(0);
}
/*---------------------------------------------------------------------------*/
/* Print customer code and name in a line */
pr_cust()
{
	int	retval;

	if( prnt_line()<0 )	return(REPORT_ERR);

#ifdef ENGLISH
	mkln( 1, "CUSTOMER CODE:", 14 );
	mkln( 16, cu_rec.cu_code, 10 );
	mkln( 28, "NAME:", 5 ); 
#else
	mkln( 1, "CODE DU CLIENT:", 15 );
	mkln( 16, cu_rec.cu_code, 10 );
	mkln( 28, "NOM:", 4 ); 
#endif
	mkln( 34, cu_rec.cu_name, strlen( cu_rec.cu_name ) );
	if( prnt_line()<0 )	return(REPORT_ERR);
	if( prnt_line()<0 )	return(REPORT_ERR);

	return(0);
}
/*---------------------------------------------------------------------------*/
/*	Print heading (after doing formfeed, if necessary) */
static
PrntHdg()	/* Print heading  */
{
	int	offset;

	/* For display pause the report */
	if( term<99 && pgcnt )	/* Output On terminal */
		if(next_page()<0)  return(QUIT); /* return(ESCAPE); */

	if( pgcnt || term<99 ){	/* if not the first page or screen display */
		if( rite_top()<0 ) 
			return( REPORT_ERR );	/* form_feed */
	}
	else
		linecnt = 0;
	pgcnt++; 			/* increment page no */

	offset = ( LNSZ-strlen(param_rec.pa_co_name) )/2;
	mkln( offset, param_rec.pa_co_name, strlen(param_rec.pa_co_name) );
	if( prnt_line()<0 )	return(REPORT_ERR);
	
	mkln( 1, PROG_NAME, 10 );
#ifdef ENGLISH
	mkln( 51, "AR AGED TRIAL BALANCE - ", 24 );
	if(type[0] == SUMMARY)
		mkln( 76,"SUMMARY",7 );
	else
		mkln( 76,"DETAIL",6 );
	mkln( 113, "PAGE: ", 6 );
#else
	mkln( 51, "BALANCE CHRONOLOGIQUE DES C/R - ", 32 );
	if(type[0] == SUMMARY)
		mkln( 76,"RESUME",6 );
	else
		mkln( 76,"DETAIL",6 );
	mkln( 113, "PAGE: ", 6 );
#endif
	tedit( (char *)&pgcnt,"__0_",  line+cur_pos, R_SHORT ); 
	cur_pos += 4;
	if( prnt_line()<0 )	return(REPORT_ERR);

#ifdef ENGLISH
	mkln( 56, "SORTED ON CUSTOMER", 18 );
	mkln( 113, "DATE: ", 6 );
#else
	mkln( 56, " TRIEE PAR CLIENT ", 18 );
	mkln( 113, "DATE: ", 6 );
#endif
	tedit( (char *)&sysdt,"____/__/__",  line+cur_pos,R_LONG ); 
	cur_pos += 10;
	if( prnt_line()<0 )	return(REPORT_ERR);
	if( prnt_line()<0 )	return(REPORT_ERR);

#ifdef ENGLISH
	mkln(1,"FUND",4);
	mkln(6,"TY",2);
	mkln(11,"INVOICE",7);
	mkln(21,"TRANS",5);
	mkln(30,"DUE DATE",8);
	mkln(47,"DEBIT",5);
	mkln(60,"CREDIT",6);
	mkln(72,"BALANCE",7);
	mkln(85,"A G E I N G       A N A L Y S I S  ( DAYS )",43);
#else
	mkln(1,"FOND",4);
	mkln(6,"GE",2);
	mkln(11,"FACTURE",7);
	mkln(21,"DATE",4);
	mkln(30,"ECHEANCE",8);
	mkln(47,"DEBIT",5);
	mkln(60,"CREDIT",6);
	mkln(72,"BALANCE",7);
	mkln(85,"A N A L Y S E  C H R O N O L O G I Q U E (JOUR)",47);
#endif
	if( prnt_line()<0 )	return(REPORT_ERR);
#ifdef ENGLISH
	mkln(11,"NUMBER",6);
	mkln(22,"DATE",4);
	mkln(85,"0 - 30",6);
	mkln(98,"31 - 60",7);
	mkln(111,"61 - 90",7);
	mkln(123,"ABOVE 90",8);
#else
	mkln(11,"NUMERO",6);
	mkln(22,"TRANS",5);
	mkln(85,"0 - 30",6);
	mkln(98,"31 - 60",7);
	mkln(111,"61 - 90",7);
	mkln(123,"PLUS DE 90",10); 
#endif 

	if( prnt_line()<0 )	return(REPORT_ERR);
	if( prnt_line()<0 )	return(REPORT_ERR);

	return(0);
}
/*---------------------------------------------------------------------------*/
/* Print report line from invoice header record */
static
PrntRec()
{
	int	posn;
	static	short	flg;
	static	double	temp_bal;

	cur_pos = 0;

	if(strcmp(ar_hdr.ah_type,"CM") == 0)
		flg = -1;
	else
		flg = 1;

	if(strcmp(ar_hdr.ah_type,"IN") != 0 && ar_hdr.ah_sno != 1) {
		ar_hdr.ah_netamt = ar_hdr.ah_oriamt;
	}

	age = days(sysdt) - days(ar_hdr.ah_duedt);
	index = findslot(age);
	
/*****
	tot_bal += ar_hdr.ah_balance * flg;
	tslot[index] += ar_hdr.ah_balance * flg;

	grand_tot += ar_hdr.ah_balance * flg;
	grand_tslot[index] += ar_hdr.ah_balance * flg;
******/

	tot_bal += ar_hdr.ah_netamt * flg;
	tslot[index] += ar_hdr.ah_netamt * flg;

	grand_tot += ar_hdr.ah_netamt * flg;
	grand_tslot[index] += ar_hdr.ah_netamt * flg;

	/*  Just do the calculations if a summary report was requested  */
	if(type[0] == DETAIL){
		/* return(0); */
		mkln( 1," ",1 );
		tedit( (char *)&ar_hdr.ah_fund, "_0_", line+cur_pos, R_SHORT );
		cur_pos += 3;
		mkln(6,ar_hdr.ah_type,2);
		mkln(8," ",1 );
		tedit( (char *)&ar_hdr.ah_inv_no, "______0_", line+cur_pos, R_LONG );
		cur_pos += 8;
		mkln(17," ",1 );
		tedit( (char *)&ar_hdr.ah_trandt,"____/__/__", line+cur_pos, R_LONG );
		cur_pos += 10;
		mkln(28," ",1 );
		tedit( (char *)&ar_hdr.ah_duedt,"____/__/__", line+cur_pos, R_LONG );
		cur_pos += 10;

		if(strcmp(ar_hdr.ah_type,"CM") == 0){
			mkln( 54," ",1 );
			balance = balance - ar_hdr.ah_netamt;
		}
		else{
			mkln(43," ",1 );
			balance = balance + ar_hdr.ah_netamt;
		}
	
/******
	temp_bal = ar_hdr.ah_balance * flg;
******/
		temp_bal = ar_hdr.ah_netamt * flg;

		tedit( (char *)&temp_bal,"______0_.__",line+cur_pos,R_DOUBLE );
		cur_pos += 12;

		mkln(66," ",1);
		tedit( (char *)&balance,"______0_.__-",line+cur_pos,R_DOUBLE );
		cur_pos += 12;


		switch( index ){
			case 0: posn=79;	break;
			case 1: posn=92;	break;
			case 2: posn=105;	break;
			case 3: posn=118;	break;
			default:	break;
		}
		mkln( posn," ",1 );

	/******
		temp_bal = ar_hdr.ah_balance * flg;
	******/
		temp_bal = ar_hdr.ah_netamt * flg;

		tedit( (char *)&temp_bal,"______0_.__-",line+cur_pos,R_DOUBLE );
		cur_pos += 12;
		if( prnt_line()<0 )	return(REPORT_ERR);

	}
	cnt = 1;
	return(0);
}
/* ------------------------------------------------------------------------ */
/* Function: Print Invoice Header Part					    */
/* ------------------------------------------------------------------------ */
static
GetUnapplied()
{
	int err;

	/*  Reset */
	rcpt_total = 0.00;

	strcpy(rcpt_hdr.rhdr_cust,cu_rec.cu_code);
	rcpt_hdr.rhdr_fund = 0;
	rcpt_hdr.rhdr_refno = 0;
	flg_reset(RCPTHDR);

	for( ; ; ) {
		err = get_n_rcpthdr(&rcpt_hdr,BROWSE,2,FORWARD,e_mesg);
		if(err == EFL) break;
		if(err < 0) {
			fomer(e_mesg); get();
			return(DBH_ERR);
		}
		if(strcmp(rcpt_hdr.rhdr_cust,cu_rec.cu_code)!=0) break;
		if(rcpt_hdr.rhdr_applied[0] == APPLIED) continue;

		rcpt_itm.ritm_refno = rcpt_hdr.rhdr_refno;
		rcpt_itm.ritm_seqno = 0;
		flg_reset(RCPTITEM);

		for( ; ; ) {
			err=get_n_rcptitem(&rcpt_itm,BROWSE,0,FORWARD,e_mesg);
			if(err == EFL) break;
			if(err < 0) {
				fomer(e_mesg); get();
				return(DBH_ERR);
			}

			if(rcpt_itm.ritm_refno != rcpt_hdr.rhdr_refno) break;
			rcpt_hdr.rhdr_amount -= rcpt_itm.ritm_amount;
		}
	
		rcpt_total = rcpt_hdr.rhdr_amount;
		/*  If greater than zero then it has not all been paid off  */
		if(rcpt_total > 0.00) {
			if(linecnt >= PGSIZE-4) {
				if((retval = PrntHdg())<0)
					return(retval);
				bufcust[0] = '\0';
			}

			if(strcmp(bufcust,cu_rec.cu_code) != 0) {
				if((retval = pr_cust()) < 0) 
					return(retval);
				strcpy(bufcust,cu_rec.cu_code);
			}

			if(PrntRcpt(0) < 0) return(ERROR);
		}
	}
	return(NOERROR);
}
/* ------------------------------------------------------------------------ */
/* Function: Check to see if receipt applied to this invoice 	            */
/* ------------------------------------------------------------------------ */
static
CheckForRcpt(customer,invcnbr)
char	customer[11];
long	invcnbr;
{
	int	err;
	int	exists;

	/* Assume receipt does not exist until it finds it */
	exists = 0;

	strcpy(rcpt_itm.ritm_cust,customer);
	rcpt_itm.ritm_invnumb = invcnbr;
	rcpt_itm.ritm_refno = 0;
	rcpt_itm.ritm_seqno = 0;
	flg_reset(RCPTITEM);
	for( ; ; ) {
		err = get_n_rcptitem(&rcpt_itm,BROWSE,1,FORWARD,e_mesg);
		if(err == EFL) break;
		if(err < 0) {
			fomer(e_mesg); get();
			return(DBH_ERR);
		}

		if(strcmp(rcpt_itm.ritm_cust,customer) != 0 ||
		   rcpt_itm.ritm_invnumb != invcnbr) break;

		if(rcpt_itm.ritm_invnumb == invcnbr)
			exists = 1;

		if(exists == 1) {
			rcpt_hdr.rhdr_refno = rcpt_itm.ritm_refno;
			err = get_rcpthdr(&rcpt_hdr,BROWSE,0,e_mesg);
			if(err < 0) {
				fomer(e_mesg); get();
				return(DBH_ERR);
			}

			if(linecnt >= PGSIZE-4) {
				if((retval = PrntHdg())<0)
					return(retval);
				bufcust[0] = '\0';
			}

			if(strcmp(bufcust,cu_rec.cu_code) != 0) {
				if((retval = pr_cust()) < 0) 
					return(retval);
				strcpy(bufcust,cu_rec.cu_code);
			}

			if(PrntRcpt(1)< 0) 
				return(ERROR);
		}

	}
	seq_over(RCPTITEM);
	return(NOERROR);
}
/*---------------------------------------------------------------------------*/
/* Print report line from invoice header record */
static
PrntRcpt(mode)
int	mode;
{
	int	posn;
	static	short	flg;
	static	double	temp_bal;

	cur_pos = 0;

	flg = -1;

/**
	age = days(sysdt) - days(ar_hdr.ah_duedt);
	index = findslot(age);
**/	
	index = 0;
	cnt = 1;

	if(mode == 0){
		tot_bal += rcpt_hdr.rhdr_amount * flg;
		tslot[index] += rcpt_hdr.rhdr_amount * flg;
		grand_tot += rcpt_hdr.rhdr_amount * flg;
		grand_tslot[index] += rcpt_hdr.rhdr_amount * flg;
	}
	if(mode == 1){
		tot_bal += rcpt_itm.ritm_amount * flg;
		tslot[index] += rcpt_itm.ritm_amount * flg;
		grand_tot += rcpt_itm.ritm_amount * flg;
		grand_tslot[index] += rcpt_itm.ritm_amount * flg;
	}


	/*  Just do the calculations if a summary report was requested  */
	if(type[0] == SUMMARY)
		return(0);

	mkln( 1," ",1 );
	tedit( (char *)&rcpt_hdr.rhdr_fund, "_0_", line+cur_pos, R_SHORT );
	cur_pos += 3;
	if(mode == 0) {
#ifdef ENGLISH
		mkln(6,"UA",2);
#else
		mkln(6,"PA",2);
#endif
	}
	else {
#ifdef ENGLISH
		mkln(6,"AP",2);
#else
		mkln(6,"PI",2);
#endif
	}
	mkln( 8," ",1 );
	tedit( (char *)&rcpt_hdr.rhdr_refno, "______0_", line+cur_pos, R_LONG );
	cur_pos += 8;
	mkln( 17," ",1 );
	tedit( (char *)&rcpt_hdr.rhdr_rcptdate,"____/__/__", line+cur_pos, R_LONG );
	cur_pos += 10;
	mkln( 54," ",1 );
	
	if(mode == 0){
		temp_bal = rcpt_hdr.rhdr_amount * flg;
	}
	if(mode == 1){
		temp_bal = rcpt_itm.ritm_amount * flg;
	}


	tedit( (char *)&temp_bal,"______0_.__",line+cur_pos,R_DOUBLE );
	cur_pos += 12;

	balance = balance - rcpt_hdr.rhdr_amount;

	mkln(66," ",1);
	tedit( (char *)&balance,"______0_.__-",line+cur_pos,R_DOUBLE );
	cur_pos += 12;

	switch( index ){
		case 0: posn=79;	break;
		case 1: posn=92;	break;
		case 2: posn=105;	break;
		case 3: posn=118;	break;
		default:	break;
	}
	mkln( posn," ",1 );

	if(mode == 0){
		temp_bal = rcpt_hdr.rhdr_amount * flg;
	}
	if(mode == 1){
		temp_bal = rcpt_itm.ritm_amount * flg;
	}

	tedit( (char *)&temp_bal,"______0_.__-",line+cur_pos,R_DOUBLE );
	cur_pos += 12;
	if( prnt_line()<0 )	return(REPORT_ERR);

	return(0);
}
