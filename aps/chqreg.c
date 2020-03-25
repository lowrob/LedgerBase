/*
*	Source 	: chqreg.c 
*
*	Program to Print Cheque Register.
*
Modifications:

L.Robichaud	1993/10/13	Pass new parameter to close_rep function for 
				a banner to print with 3000 family.
*/

#include <stdio.h>
#include <reports.h>

#define PRINT_NAME	"register.dat"

#ifdef ENGLISH
#define YES		'Y'
#else
#define YES		'O'
#endif

/*   Defining files being used in program */
static	Pa_rec 	pa_rec ;
extern	Ctl_rec	ctl_rec ;
static	Reg_rec  reg_rec ;
static	Supplier supplier ;

static	char	e_mesg[80] ;

static	char	temp_buf[132] ;
static	double	contract_amt, ordinary_amt, discount_amt, bank_amt ;
static	double	gross_total, disc_total ;

static	short	chq_no ;
static	short	pgcnt ;
static 	char	bank_acnt[19] ;

Register(accno)
char	*accno ;
{
	int	code;

	code = get_param(&pa_rec, BROWSE, 1, e_mesg) ;
	if(code < 1) {
		close_dbh() ;
		return(DBH_ERR) ;
	}

	STRCPY(bank_acnt, accno) ;

	code = OpenReport() ;
	if (code < 0) return(code) ;

	/* Initialize Recs. to start printing po report on po no. */

	reg_rec.cr_funds = 0 ; 
	reg_rec.cr_chq_no = 0 ; 
	flg_reset( CHQREG );
	

	for( ; ; ) {
		code = get_n_reg(&reg_rec,BROWSE,0,FORWARD,e_mesg);
		if( code < 0) {
			break ;
		}
				/* Getting Name of Beneficiary */
		STRCPY( supplier.s_supp_cd, reg_rec.cr_supp_cd ) ;
		code = get_supplier(&supplier,BROWSE,0,e_mesg) ;
		if( code < 0) 
#ifdef ENGLISH
			STRCPY( supplier.s_name, "NOT FOUND IN SUPPLIER FILE") ;
#else
			STRCPY( supplier.s_name, "Pas retrouve dans le dossier des fournisseurs") ;
#endif

		if( supplier.s_type[0] == CONTRACT ) 
		   contract_amt += (reg_rec.cr_gr_amt) ;
		else 
		   ordinary_amt += (reg_rec.cr_gr_amt) ;

		discount_amt += reg_rec.cr_disc_taken ; 
		bank_amt += (reg_rec.cr_gr_amt - reg_rec.cr_disc_taken) ;

		if( reg_rec.cr_cancelled[0] == YES ) 	/* Type of Chq */
			reg_rec.cr_chq_type[0] = CANCELLED ;
		else if (reg_rec.cr_chq_type[0] == REGULAR) 
			reg_rec.cr_chq_type[0] = ' ' ;

		chq_no++ ;			/* Accumulating #of Cheques */

		PrintLine() ;
	}

	code = PrintTotals() ;
	if (code < 0) return(code);

	CloseReport();
	close_dbh() ;

	SpoolReport(PRINT_NAME, 1, 1);

	return(NOERROR);
}
/*------------------------------------------------------------------------*/
/* Open Report File */

OpenReport()
{
	/* Always to file */
	if(opn_prnt("F" , PRINT_NAME, 1, e_mesg, 1) < 0) {
		DispError(e_mesg) ;
		return(ERROR) ;
	}

	LNSZ = 133 ;
	PGSIZE = 60 ;
	cur_pos = 0;
	pgcnt = 0 ;
	linecnt = PGSIZE ;

	/* Initialize Totals variables */
	chq_no = 0 ; 
	ordinary_amt = 0.00 ;
	contract_amt = 0.00 ;
	discount_amt = 0.00 ;
	bank_amt = 0.00 ;
	gross_total = 0.00 ;
	disc_total = 0.00 ;
	
	return(NOERROR) ;
}	/* OpenReport() */
/*------------------------------------------------------------*/
/* Close report files */

CloseReport()
{
	PrintSummary() ;

#ifndef	SPOOLER
		rite_top() ;
#endif
	close_rep(BANNER);		/* Close Report file */

	return(NOERROR) ;
}	/* CloseReport() */
/*----------------------------------------------------------------------------*/
/* Print report detail line 						      */
PrintLine()
{
	int	err ;
	double	net ;

	if(linecnt >= PGSIZE-1)
		if((err = PrintHeadings(0)) != NOERROR) return(err) ;

	tedit((char*)&reg_rec.cr_chq_no, Mask_8, line+cur_pos, R_LONG) ;
	cur_pos += 8 ;
	mkln(9, " ", 1);
	tedit((char*)&reg_rec.cr_date, Date_Mask, line+cur_pos, R_LONG);
	cur_pos += 10 ;
	mkln(22, " ", 1) ;
	mkln(31, reg_rec.cr_chq_type, 1) ;
	mkln(32, " ", 1);
	tedit((char*)&reg_rec.cr_gr_amt, Amt_Mask, line+cur_pos, R_DOUBLE) ;
	cur_pos += 15 ;
	mkln(52, " ", 1);
	tedit((char*)&reg_rec.cr_disc_taken, Amt_Mask, line+cur_pos, R_DOUBLE);
	cur_pos += 15 ;
	mkln(68, " ", 1);
	net = reg_rec.cr_gr_amt - reg_rec.cr_disc_taken ;
	tedit((char*)&net, Amt_Mask, line+cur_pos, R_DOUBLE) ;
	cur_pos += 15 ;
	mkln(84, " ", 1);
	mkln(85, supplier.s_supp_cd, 10) ;
	mkln(96, " ", 1);
	mkln(97, supplier.s_name, 35) ;
	if( prnt_line() < 0) return(-1) ;

	gross_total += reg_rec.cr_gr_amt ;
	disc_total += reg_rec.cr_disc_taken ;

	return(NOERROR) ;
}	/* PrintLine() */
PrintTotals()
{
	double	net ;

	if( prnt_line() < 0) return(-1) ;
#ifdef ENGLISH
	mkln(1,"Number of Cheques",17) ;
#else
	mkln(1,"Nombre de cheques",17) ;
#endif
	tedit((char*)&chq_no, Mask_4, line+cur_pos, R_SHORT) ;
	cur_pos += 4 ;
	mkln(32, " ", 1);
	tedit((char*)&gross_total, Amt_Mask, line+cur_pos, R_DOUBLE) ;
	cur_pos += 15 ;
	mkln(52, " ", 1);
	tedit((char*)&disc_total, Amt_Mask, line+cur_pos, R_DOUBLE);
	cur_pos += 15 ;
	mkln(68, " ", 1);
	net = gross_total - disc_total ;
	tedit((char*)&net, Amt_Mask, line+cur_pos, R_DOUBLE) ;
	cur_pos += 15 ;
	mkln(84, " ", 1);
	if( prnt_line() < 0) return(-1) ;

	return(NOERROR) ;
}
PrintSummary()
{
	double 	ws_amt ;

	PrintHeadings(1) ;
#ifdef ENGLISH
	mkln(15, "Fund", 4) ;
#else
	mkln(15, "Fond", 4) ;
#endif
	tedit((char *)&ctl_rec.fund,"_0_",temp_buf,R_SHORT) ;
	mkln(20,temp_buf,3) ; 

#ifdef ENGLISH
	mkln(34,"G/L Account Ordinary",20) ;
#else
	mkln(34,"Compte G/L ordinaire",20) ;
#endif
	mkln(60,ctl_rec.ap_gen_acnt,18) ;
#ifdef ENGLISH
	mkln(80,"Amount",6) ;
#else
	mkln(80,"Montant",7) ;
#endif
	tedit((char *)&ordinary_amt,Amt_Mask,temp_buf,R_DOUBLE) ;
	mkln(87,temp_buf,15) ;
	if( prnt_line() < 0) return(-1) ;

#ifdef ENGLISH
	mkln(34,"G/L Account Contract",20) ;
#else
	mkln(34,"Compte G/L contract ",20) ;
#endif
	mkln(60,ctl_rec.ap_cnt_acnt,18) ;
#ifdef ENGLISH
	mkln(80,"Amount",6) ;
#else
	mkln(80,"Montant",7) ;
#endif
	tedit((char *)&contract_amt,Amt_Mask,temp_buf,R_DOUBLE) ;
	mkln(87,temp_buf,15) ;
	if( prnt_line() < 0) return(-1) ;

#ifdef ENGLISH
	mkln(34,"G/L Account Discount",20) ;
#else
	mkln(34,"Compte G/L escompte ",20) ;
#endif
	mkln(60,ctl_rec.dis_acnt,18) ;
#ifdef ENGLISH
	mkln(80,"Amount",6) ;
#else
	mkln(80,"Montant",7) ;
#endif
	ws_amt = -(discount_amt) ;
	tedit((char *)&ws_amt,Amt_Mask,temp_buf,R_DOUBLE) ;
	mkln(87,temp_buf,15) ;
	if( prnt_line() < 0) return(-1) ;

#ifdef ENGLISH
	mkln(34,"G/L Account Bank    ",20) ;
#else
	mkln(34,"Compte G/L banque   ",20) ;
#endif
	if( (strcmp(ctl_rec.bank1_acnt,bank_acnt)) == 0 ) 
		mkln(60,ctl_rec.bank1_acnt,18) ;
	else
		mkln(60,ctl_rec.bank2_acnt,18) ;
#ifdef ENGLISH
	mkln(80,"Amount",6) ;
#else
	mkln(80,"Montant",7) ;
#endif
	ws_amt = -(bank_amt) ;
	tedit((char *)&ws_amt,Amt_Mask,temp_buf,R_DOUBLE) ;
	mkln(87,temp_buf,15) ;
	if( prnt_line() < 0) return(-1) ;
	return(NOERROR) ;
}
/*------------------------------------------------------------*/
/* Print Headings */
static	int
PrintHeadings(flg)
int	flg ;
{
	int	i ;
	long	rundate ;

	/* Skip FF generation for first page, becuase operator sets the
	   paper on Top of the Page */
	if( pgcnt ) rite_top() ;
	linecnt = 0 ;

	/* Print Program-Id, School District name and Date */
	mkln(1,PROG_NAME, 10) ;
	STRCPY(temp_buf,pa_rec.pa_co_name) ;
	i = strlen(temp_buf);
	mkln(((LNSZ-1-i) / 2)+1,temp_buf, i);
#ifdef ENGLISH
	mkln(106,"Date: ",6);
#else
	mkln(106,"Date: ",6);
#endif
	rundate = get_date() ;
	tedit((char*)&rundate, Date_Mask, line+cur_pos,R_LONG);
	cur_pos += 10;
	pgcnt++ ;
#ifdef ENGLISH
	mkln(124,"Page: ",6);
#else
	mkln(124,"Page: ",6);
#endif
	tedit((char*)&pgcnt, Mask_3, line+cur_pos,R_SHORT);
	cur_pos += 3 ;
	if(prnt_line() < 0) return(ERROR);
	
#ifdef ENGLISH
	mkln(52,"CHEQUE REGISTER  -  PAYABLES",28) ;
#else
	mkln(51,"REGISTRE DES CHEQUES  -  PAYABLES",34) ;
#endif
	if( prnt_line() < 0) return(-1) ;

	if(prnt_line() < 0) return(ERROR);

	if(flg == 1) return(NOERROR) ;	/* Printing Close Toatls */

	/* Print Column Headings */
#ifdef ENGLISH
	mkln(1, "Cheque      Date           Cheque      Gross    ", 53);
	mkln(57, "Discount          Net    Supplier         Supplier  ",58) ;
#else
	mkln(1, "Numero      Date           Code de     Brut     ", 53);
	mkln(57, "Escompte       Paiement   Code de       Fournisseur",58) ;
#endif
	if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
	mkln(1, "Number                      Code                ", 53) ;
	mkln(57, "                Payment    Code                     ",58) ;
#else
	mkln(1, "de cheque                  Cheque               ", 53) ;
	mkln(57, "                         Founisseur                 ",58) ;
#endif
	if(prnt_line() < 0) return(ERROR);

	if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
	mkln(1, "Fund : ", 7) ;
#else
	mkln(1, "Fond : ", 7) ;
#endif
	tedit((char*)&ctl_rec.fund, Mask_3, line+cur_pos,R_SHORT);
	cur_pos += 3 ;
	mkln(10, " ", 1);
#ifdef ENGLISH
	mkln(13,"Bank Account: ",15) ;
#else
	mkln(13,"Compte de banque: ",18) ;
#endif
	mkln(31,bank_acnt,18) ;
	if(prnt_line() < 0) return(ERROR);

	if(prnt_line() < 0) return(ERROR);

	return(NOERROR) ;
}
