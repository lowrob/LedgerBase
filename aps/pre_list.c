/*-----------------------------------------------------------------------
Source Name: pre_list.c
System     : Accounts Payables.
Created  On: 29rd November 89.
Created  By: T AMARENDRA.

COBOL Source(s): cp120---02 (Same code in cp120a---02 also)

DESCRIPTION:
	This Program prints the Cheque Pre-List Report.

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

L.Robichaud	1993/10/13	Pass new parameter to close_rep function for 
				a banner to print with 3000 family.

------------------------------------------------------------------------*/

#include <stdio.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_recs.h>
#include <repdef.h>

#define	MASK_3		"_0_"			/* 3 Char Mask */
#define	MASK_6		"__,_0_"		/* 6 Char Mask */
#define	MASK_8		"______0_"		/* 8 Char Mask */
#define	DATE_MASK	"____/__/__"		/* Date in YYYY/MM/DD format */
#define	MASK_13_2	"___,___,_0_.__-"	/* 13.2 Mask + Minus Sign */

#define EXIT		12

#ifdef ENGLISH
#define PRINTER		'P'
#define DISPLAY		'D'
#define FILE_IO		'F'

#define YES		'Y'
#else
#define PRINTER		'I'
#define DISPLAY		'A'
#define FILE_IO		'D'

#define YES		'O'
#endif

static	short	FundChng, ChqChng ;
static	short	pgcnt ;

/* Totals Variables */
static	short	C_suppliers, O_suppliers ;
static	short	C_cheques, O_cheques;
static  short	C_mancheques, O_mancheques ;
static  short	C_cancheques, O_cancheques ;
static	short	C_trans, O_trans ;
static	double	s_gr_amt, s_disc_taken ;
static	double	ch_gr_amt, ch_disc_taken ;
static	double	C_gr_amt, O_gr_amt, C_disc_taken, O_disc_taken ;

/* Data Record Definitions */
static	Pa_rec		pa_rec ;	/* Parameters Record */
static	Supplier	supp_rec ;	/* Supplier */
static	Chq_rec		chq_rec ;	/* Cheque Record */

extern	char 		e_mesg[80];	/* dbh will return err msg in this */

/* Flag */
static	short	chq_type ;	/* 'R' - Regular, 'M' - Manual or
				   'C' - Cancelled cheques */
/*------------------------------------------------------------------------*/
ChequePreList()
{
	int	err ;
	char	prev_suppcd[11] ;
	long	prev_chq_no ;
	short	prev_fund ;

	err = OpenReport() ;
	if(err == EXIT) return(0);
	if(err < 0) return(err) ;

	/* Get the parameters record */
	err = get_param(&pa_rec, BROWSE, 1, e_mesg) ;
	if(err < 0) {
		return(DBH_ERR) ;
	}
	/* Intialize che_rec main key */
	chq_rec.c_supp_cd[0] = '\0'  ;
	chq_rec.c_funds = 0 ;
	chq_rec.c_chq_no = 0 ;
	chq_rec.c_invc_no[0] = '\0' ;
	chq_rec.c_tr_type[0] = '\0' ;

	FundChng = -1 ;
	flg_reset(CHEQUE) ;
	for( ; ; ) {
		err = get_n_chq(&chq_rec, BROWSE, 0, FORWARD, e_mesg) ;
		if(err == ERROR)  {
			return(DBH_ERR) ;
		}
		if(err == EFL) {
			if(FundChng != -1){	/* If Not First Time */
				err = ChequeTotals() ;
				if(err < 0) break ;
				err = SupplierTotals() ;
				if(err < 0) break ;
			}
			break ;
		}
		/*** Skip cancelled cheques
		if(chq_rec.c_cancelled[0] == YES) continue ;
		****/

		if(FundChng == -1 ||
				strcmp(prev_suppcd, chq_rec.c_supp_cd) != 0) {
			if(FundChng != -1){	/* If Not First Time */
				err = ChequeTotals() ;
				if(err < 0) break ;
				err = SupplierTotals() ;
				if(err < 0) break ;
			}
			err = PrintSubHdg() ;
			if(err == EXIT) break;
			if(err < 0) break ;
			STRCPY(prev_suppcd, chq_rec.c_supp_cd) ;
			prev_chq_no = chq_rec.c_chq_no ;
			prev_fund = chq_rec.c_funds ;
		}
		else if( prev_fund != chq_rec.c_funds) {
			err = ChequeTotals() ;
			if(err < 0) break ;
			FundChng = 1 ;
			ChqChng  = 1 ;
			prev_chq_no = chq_rec.c_chq_no ;
			prev_fund = chq_rec.c_funds ;
		}
		else if( prev_chq_no != chq_rec.c_chq_no) {
			err = ChequeTotals() ;
			if(err < 0) break ;
			ChqChng  = 1 ;
			prev_chq_no = chq_rec.c_chq_no ;
		}
		err = PrintCheque() ;
		if(err==EXIT) break;
		if(err < 0) break ;
	}
	seq_over(CHEQUE) ;

	if(err < 0 && err != EFL) {
#ifdef ENGLISH
		fomen("Printing Error");
#else
		fomen("Erreur d'impression");
#endif
		get() ;
	}

	if(err==EXIT)
		close_rep(BANNER);		/* Close Report file */
	else	
		CloseReport() ;
	close_dbh() ;

	return(NOERROR) ;
}
/*------------------------------------------------------------------------*/
/* Open Report File */

OpenReport()
{
	int	err;
	char	outdev[2];
	char	discfile[20];
	short	copies ;

#ifdef ENGLISH
 	STRCPY( outdev, "P" );
#else
 	STRCPY( outdev, "I" );
#endif
	err = GetOutputon( outdev );
	if ( err<0 || err==EXIT )
		return( err );

	switch (*outdev) {
		case DISPLAY :	/*  Display on Terminal */
			outdev[0] = 'D';
			STRCPY(discfile,terminal);
			PGSIZE = 22;
			break;
		case FILE_IO : 	/*  Print to a disk file */ 
			outdev[0] = 'F';
			STRCPY( e_mesg, "status.dat");
			err = GetFilename(e_mesg);
			if( err<0 || err==EXIT )
			return(err);
			STRCPY (discfile, e_mesg) ;
			PGSIZE = 60;
			break;
		case PRINTER : 	/*  Print to a printer */ 
		default  :
			outdev[0] = 'P';
			discfile[0] = '\0';
			PGSIZE = 60;
			break;
	}
	copies = 1;
		/* Do not Change must be 'P' to open printer */
	if(*outdev == 'P') {
		if((err = GetNbrCopies( &copies )) < 0)
			return(err);
	}
	if ( (err=Confirm())<= 0) 
		return(EXIT);

	/* Open the Printer */
	if(opn_prnt(outdev , discfile, 1, e_mesg, 1) < 0) {
		fomer(e_mesg) ;
		get() ;
		return(ERROR) ;
	}

		/* Do not Change must be 'P' to open printer */
	if(*outdev == 'P') 
		SetCopies( (int)copies ); 	/* number of copies to print */

	LNSZ = 133 ;
	cur_pos = 0;
	pgcnt = 0 ;
	linecnt = PGSIZE ;
	FundChng = 0 ;
	ChqChng = 0 ;

	/* Initialize Totals variables */
	C_suppliers = 0 ;
	O_suppliers = 0 ;
	O_cheques = 0 ;
	C_cheques = 0 ;
	O_mancheques = 0 ;
	C_mancheques = 0 ;
	O_cancheques = 0 ;
	C_cancheques = 0 ;
	C_trans = 0 ;
	O_trans = 0 ;
	C_gr_amt = 0.0 ;
	O_gr_amt = 0.0 ;
	C_disc_taken = 0.0 ;
	O_disc_taken = 0.0 ;
	ch_gr_amt = 0.0 ;
	ch_disc_taken = 0.0 ;
	chq_type = REGULAR ;
	
	return(NOERROR) ;
}	/* OpenReport() */
/*------------------------------------------------------------*/
/* Close report files */

CloseReport()
{
	PrintCloseTotals() ;

	if(pgcnt)
		if(term < 99)
			last_page();
#ifndef	SPOOLER
		else
			rite_top() ;
#endif
	close_rep(BANNER);		/* Close Report file */

	return(NOERROR) ;
}	/* CloseReport() */
/*------------------------------------------------------------*/
/* Print Supplier Code, Name etc as a subheading */

PrintSubHdg()
{
	int	err ;

	STRCPY(supp_rec.s_supp_cd, chq_rec.c_supp_cd) ;
	err = get_supplier(&supp_rec, BROWSE, 0, e_mesg) ;
	if( err == ERROR) return(DBH_ERR) ;
	if(err < 0)
		STRCPY(supp_rec.s_name, "?????????????????");

	if(linecnt >= (PGSIZE - 4)) {
		if((err = PrintHeadings(0)) != NOERROR) return(err) ;
		/* Need not call PrintSupplier(), because PrintHeadings()
		  does it */
	}
	else
		if((err = PrintSupplier()) != NOERROR) return(err) ;

	if(supp_rec.s_type[0] == ORDINARY)
		O_suppliers++ ;
	else
		C_suppliers++ ;

	/* Initialize Totals */
	s_gr_amt = 0.0 ;
	s_disc_taken = 0.0 ;

	return(NOERROR) ;
}	/* PrintSubhdg() */
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
	if(term < 99 && pgcnt)
		if(next_page() <0) return(EXIT);

	if( pgcnt || term < 99 ) 
		if(rite_top() <0) return(EXIT);

	linecnt = 0 ;

	/* Print Program-Id, School District name and Date */
	mkln(1,PROG_NAME, 10) ;
	i = strlen(pa_rec.pa_co_name);
	mkln(((LNSZ-1-i) / 2)+1,pa_rec.pa_co_name, sizeof(pa_rec.pa_co_name));
#ifdef ENGLISH
	mkln(106,"Date: ",6);
#else
	mkln(106,"Date: ",6);
#endif
	rundate = get_date() ;
	tedit((char*)&rundate, DATE_MASK, line+cur_pos,R_LONG);
	cur_pos += 10;
	pgcnt++ ;
#ifdef ENGLISH
	mkln(124,"Page: ",6);
#else
	mkln(124,"Page: ",6);
#endif
	tedit((char*)&pgcnt, MASK_3, line+cur_pos,R_SHORT);
	cur_pos += 3 ;
	if(prnt_line() < 0) return(ERROR);
	
#ifdef ENGLISH
	mkln(52, "C H E Q U E    P R E - L I S T", 30) ;
#else
	mkln(37, "L I S T E   P R E L I M I N A I R E   D E S   C H E Q U E S", 59) ;
#endif
	if(prnt_line() < 0) return(ERROR);

	if(prnt_line() < 0) return(ERROR);

	if(flg == 1) return(NOERROR) ;	/* Printing Close Toatls */

	/* Print Column Headings */
#ifdef ENGLISH
	mkln(1, "Fund Cheque#   ---Transaction--   Per    Trans", 46);
	mkln(56, "Due    Status/        Trans           Gross", 44);
	mkln(108,"Discount             Net", 24);
#else
	mkln(1, "Fond #Cheque   ---Transaction--    Per    Date", 46);
	mkln(53, "Echeance     Etat/      Balance         Paiement", 48);
	mkln(109,"Escompte       Paiement", 24);
#endif
	if(prnt_line() < 0) return(ERROR);
#ifdef ENGLISH
	mkln(18, "Ref#      Type",14);
	mkln(43, "Date", 4) ;
	mkln(55, "Date", 4) ;
	mkln(62, "Bank Acct", 9) ;
	mkln(77, "Balance", 7) ;
	mkln(93, "Payment", 7) ;
	mkln(110, "Taken", 5) ;
	mkln(125, "Payment", 7) ;
#else
	mkln(18, "#Ref      Genre",15);
	mkln(43, "Trans", 5) ;
	mkln(55, "    ", 4) ;
	mkln(62, "Compte banque", 13) ;
	mkln(77, "de trans", 8) ;
	mkln(93, "  Net  ", 7) ;
	mkln(110, "Prise", 5) ;
	mkln(125, " Net   ", 7) ;
#endif
	if(prnt_line() < 0) return(ERROR);

	if(prnt_line() < 0) return(ERROR);

	if(PrintSupplier() < 0) return(ERROR) ;

	return(NOERROR) ;
}	/* PrintHeadings() */
/*------------------------------------------------------------*/
/* Print Supplier Details as a sub heading before printing trans of Cheque */
static	int
PrintSupplier()
{
	if(prnt_line() < 0) return(ERROR);	/* Print Blank line */

#ifdef ENGLISH
	mkln(1, "Supplier:", 9) ;
#else
	mkln(1, "Fournisseur:", 12) ;
#endif
	mkln(14, supp_rec.s_supp_cd, 10) ;
	mkln(25, supp_rec.s_name, 48 );
#ifdef ENGLISH
	mkln(73, "Type:", 5) ;
#else
	mkln(73, "Genre:", 6) ;
#endif
	mkln((cur_pos+2), supp_rec.s_type, 1) ;
	if(prnt_line() < 0) return(ERROR);

	FundChng = 1 ;	/* Print Fund in PrintCheque() */
	ChqChng = 1 ;	/* Print cheque# in PrintCheque() */

	return(NOERROR) ;
}	/* PrintSupplier() */
/*------------------------------------------------------------*/
/* Print cheque details */

PrintCheque()
{
	double	net ;
	int	err ;

	if(linecnt >= PGSIZE-1)
		if((err = PrintHeadings(0)) != NOERROR) return(err) ;

	if(FundChng || ChqChng)
		if(prnt_line() < 0) return(ERROR) ;

	if(FundChng) {
		tedit((char*)&chq_rec.c_funds, MASK_3, line+cur_pos, R_SHORT) ;
		cur_pos += 3 ;
		FundChng = 0 ;
	}
	if(ChqChng) {
		mkln(4, " ", 1);
		tedit((char*)&chq_rec.c_chq_no, MASK_8, line+cur_pos, R_LONG) ;
		cur_pos += 8 ;
		ChqChng = 0 ;
	}

	mkln(14, chq_rec.c_invc_no, 15) ;
	mkln(30, chq_rec.c_tr_type, 2) ;
	mkln(34, " ", 1);
	tedit((char*)&chq_rec.c_period, MASK_3, line+cur_pos, R_SHORT) ;
	cur_pos += 3 ;
	mkln(39, " ", 1);
	tedit((char*)&chq_rec.c_invc_dt, DATE_MASK, line+cur_pos, R_LONG) ;
	cur_pos += 10 ;
	mkln(51, " ", 1);
	tedit((char*)&chq_rec.c_due_dt, DATE_MASK, line+cur_pos, R_LONG) ;
	cur_pos += 10 ;
	if(chq_rec.c_chq_type[0] == MANUAL) {
#ifdef ENGLISH
		mkln(65, "MAN", 3);
#else
		mkln(65, "MAN", 3);
#endif
		chq_type = MANUAL ;
	}
	if(chq_rec.c_cancelled[0] == YES) {
#ifdef ENGLISH
		mkln(65, "CAN", 3);
#else
		mkln(65, "ANN", 3);
#endif
		chq_type = CANCELLED ;
	}
	mkln(69, " ", 1);
	tedit((char*)&chq_rec.c_in_amt, MASK_13_2, line+cur_pos, R_DOUBLE) ;
	cur_pos += 15 ;
	mkln(85, " ", 1);
	tedit((char*)&chq_rec.c_gr_amt, MASK_13_2, line+cur_pos, R_DOUBLE) ;
	cur_pos += 15 ;
	mkln(101, " ", 1);
	tedit((char*)&chq_rec.c_disc_taken, MASK_13_2, line+cur_pos, R_DOUBLE) ;
	cur_pos += 15 ;

	net = chq_rec.c_gr_amt - chq_rec.c_disc_taken ;
	mkln(117, " ", 1);
	tedit((char*)&net, MASK_13_2, line+cur_pos, R_DOUBLE) ;
	cur_pos += 15 ;
	if(prnt_line() < 0) return(ERROR);

	if(supp_rec.s_type[0] == ORDINARY)
		O_trans++ ;
	else
		C_trans++ ;

	ch_gr_amt     += chq_rec.c_gr_amt ;
	ch_disc_taken += chq_rec.c_disc_taken ;

	if(chq_rec.c_chq_type[0] == MANUAL || chq_rec.c_cancelled[0] == YES) {
		mkln(52,chq_rec.c_accno,18);
		if(prnt_line() < 0) return(ERROR);
	}
	return(NOERROR) ;
}	/* PrintCheque() */
/*------------------------------------------------------------*/
/* Print Cheque totals */
static	int
ChequeTotals()
{
	double	net ;

	if(prnt_line() < 0) return(ERROR);
#ifdef ENGLISH
	mkln(45, "Cheque Totals", 13) ;
#else
	mkln(45, "Totaux des cheques", 18) ;
#endif
	mkln(85, " ", 1);
	tedit((char*)&ch_gr_amt, MASK_13_2, line+cur_pos, R_DOUBLE) ;
	cur_pos += 15 ;
	mkln(101, " ", 1);
	tedit((char*)&ch_disc_taken, MASK_13_2, line+cur_pos, R_DOUBLE) ;
	cur_pos += 15 ;

	net = ch_gr_amt - ch_disc_taken ;
	mkln(117, " ", 1);
	tedit((char*)&net, MASK_13_2, line+cur_pos, R_DOUBLE) ;
	cur_pos += 15 ;
	if(prnt_line() < 0) return(ERROR);

	if(supp_rec.s_type[0] == ORDINARY) {
		if(chq_type == CANCELLED) /* cancelled cheque */
			O_cancheques++ ;
		else if(chq_type == MANUAL) /* manual cheque */
			O_mancheques++ ;
		else
			O_cheques++ ;
	}
	else {
		if(chq_type == CANCELLED) /* cancelled cheque */
			C_cancheques++ ;
		else if(chq_type == MANUAL) /* manual cheque */
			C_mancheques++ ;
		else
			C_cheques++ ;
	}

	s_gr_amt += ch_gr_amt ;
	s_disc_taken += ch_disc_taken ;

	ch_gr_amt = 0.0 ;
	ch_disc_taken = 0.0 ;
	chq_type = REGULAR ;

	return(NOERROR) ;
}	/* ChequeTotals() */
/*------------------------------------------------------------*/
/* Print Supplier totals */
static	int
SupplierTotals()
{
	double	net ;

	if(prnt_line() < 0) return(ERROR);
#ifdef ENGLISH
	mkln(4, "Balance: ", 9);
#else
	mkln(4, "Balance: ", 9);
#endif
	tedit((char*)&supp_rec.s_balance, MASK_13_2, line+cur_pos, R_DOUBLE) ;
	cur_pos += 15 ;
	if(supp_rec.s_type[0] == ORDINARY)
#ifdef ENGLISH
		mkln((cur_pos+4),"Discount: ", 10) ;
	else
		mkln((cur_pos+4),"Hold Back: ", 11) ;
#else
		mkln((cur_pos+4),"Escompte: ", 10) ;
	else
		mkln((cur_pos+4),"Paiement retenu: ", 17) ;
#endif
	tedit((char*)&supp_rec.s_ytd_disc, MASK_13_2, line+cur_pos, R_DOUBLE) ;
	cur_pos += 15 ;

	mkln(85, " ", 1);
	tedit((char*)&s_gr_amt, MASK_13_2, line+cur_pos, R_DOUBLE) ;
	cur_pos += 15 ;
	mkln(101, " ", 1);
	tedit((char*)&s_disc_taken, MASK_13_2, line+cur_pos, R_DOUBLE) ;
	cur_pos += 15 ;

	net = s_gr_amt - s_disc_taken ;
	mkln(117, " ", 1);
	tedit((char*)&net, MASK_13_2, line+cur_pos, R_DOUBLE) ;
	cur_pos += 15 ;
	if(prnt_line() < 0) return(ERROR);

	if(supp_rec.s_type[0] == ORDINARY) {
		O_gr_amt += s_gr_amt ;
		O_disc_taken += s_disc_taken ;
	}
	else {
		C_gr_amt += s_gr_amt ;
		C_disc_taken += s_disc_taken ;
	}

	s_gr_amt = 0.0 ;
	s_disc_taken = 0.0 ;

	return(NOERROR) ;
}	/* SupplierTotals() */
/*------------------------------------------------------------*/
/* Print Closing Process Toatls */
static	int
PrintCloseTotals()
{
	int	err ;
	short	tr_total ;
	double	amt_total ;

	if(linecnt >= (PGSIZE - 15)) {
		if((err = PrintHeadings(1)) != NOERROR) return(err) ;
	}
	else {
		if(prnt_line() < 0) return(ERROR);
		if(prnt_line() < 0) return(ERROR);
		if(prnt_line() < 0) return(ERROR);
		if(prnt_line() < 0) return(ERROR);
	}

	/* Print Column Headings */
#ifdef ENGLISH
	mkln(36, "ORDINARY        CONTRACT           TOTAL", 40) ;
#else
	mkln(34, "FOURNISSEURS   FOURNISSEURS          TOTAL", 42) ;
#endif
	if(prnt_line() < 0) return(ERROR);
#ifdef ENGLISH
	mkln(35,"SUPPLIERS       SUPPLIERS", 25) ;
#else
	mkln(35,"ORDINAIRES    CONTRACTUELS", 26) ;
#endif
	if(prnt_line() < 0) return(ERROR);

	if(prnt_line() < 0) return(ERROR);

	/* Print totals */
#ifdef ENGLISH
	mkln(1, "No of Suppliers to be Paid", 30) ;
#else
	mkln(1, "Nombre de fournisseurs a payer", 34) ;
#endif
	mkln(37, " ", 1);
	tedit((char*)&O_suppliers, MASK_6, line+cur_pos, R_SHORT) ;
	cur_pos += 6 ;
	mkln(53, " ", 1);
	tedit((char*)&C_suppliers, MASK_6, line+cur_pos, R_SHORT) ;
	cur_pos += 6 ;
	tr_total = O_suppliers + C_suppliers ;
	mkln(69, " ", 1);
	tedit((char*)&tr_total, MASK_6, line+cur_pos, R_SHORT) ;
	cur_pos += 6 ;
	if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
	mkln(1, "No of Cheques to be Written", 30) ;
#else
	mkln(1, "Nombre de cheques a ecrire", 29) ;
#endif
	mkln(37, " ", 1);
	tedit((char*)&O_cheques, MASK_6, line+cur_pos, R_SHORT) ;
	cur_pos += 6 ;
	mkln(53, " ", 1);
	tedit((char*)&C_cheques, MASK_6, line+cur_pos, R_SHORT) ;
	cur_pos += 6 ;
	tr_total = O_cheques + C_cheques ;
	mkln(69, " ", 1);
	tedit((char*)&tr_total, MASK_6, line+cur_pos, R_SHORT) ;
	cur_pos += 6 ;
	if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
	mkln(1, "No of Manual Cheques", 20) ;
#else
	mkln(1, "Nombre de cheques manuels", 25) ;
#endif
	mkln(37, " ", 1);
	tedit((char*)&O_mancheques, MASK_6, line+cur_pos, R_SHORT) ;
	cur_pos += 6 ;
	mkln(53, " ", 1);
	tedit((char*)&C_mancheques, MASK_6, line+cur_pos, R_SHORT) ;
	cur_pos += 6 ;
	tr_total = O_mancheques + C_mancheques ;
	mkln(69, " ", 1);
	tedit((char*)&tr_total, MASK_6, line+cur_pos, R_SHORT) ;
	cur_pos += 6 ;
	if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
	mkln(1, "No of Cancelled Cheques", 23) ;
#else
	mkln(1, "Nombre de transactions annullees", 32) ;
#endif
	mkln(37, " ", 1);
	tedit((char*)&O_cancheques, MASK_6, line+cur_pos, R_SHORT) ;
	cur_pos += 6 ;
	mkln(53, " ", 1);
	tedit((char*)&C_cancheques, MASK_6, line+cur_pos, R_SHORT) ;
	cur_pos += 6 ;
	tr_total = O_cancheques + C_cancheques;
	mkln(69, " ", 1);
	tedit((char*)&tr_total, MASK_6, line+cur_pos, R_SHORT) ;
	cur_pos += 6 ;
	if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
	mkln(1, "No of Transactions Paid", 30) ;
#else
	mkln(1, "Nombre de transactions payees", 36) ;
#endif
	mkln(37, " ", 1);
	tedit((char*)&O_trans, MASK_6, line+cur_pos, R_SHORT) ;
	cur_pos += 6 ;
	mkln(53, " ", 1);
	tedit((char*)&C_trans, MASK_6, line+cur_pos, R_SHORT) ;
	cur_pos += 6 ;
	tr_total = O_trans + C_trans ;
	mkln(69, " ", 1);
	tedit((char*)&tr_total, MASK_6, line+cur_pos, R_SHORT) ;
	cur_pos += 6 ;
	if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
	mkln(1, "Gross Payment Amount", 25) ;
#else
	mkln(1, "Montant du paiement brut", 29) ;
#endif
	mkln(29, " ", 1);
	tedit((char*)&O_gr_amt, MASK_13_2, line+cur_pos, R_DOUBLE) ;
	cur_pos += 15 ;
	mkln((cur_pos+1), " ", 1);
	tedit((char*)&C_gr_amt, MASK_13_2, line+cur_pos, R_DOUBLE) ;
	cur_pos += 15 ;
	amt_total = O_gr_amt + C_gr_amt ;
	mkln((cur_pos+1), " ", 1);
	tedit((char*)&amt_total, MASK_13_2, line+cur_pos, R_DOUBLE) ;
	cur_pos += 15 ;
	if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
	mkln(1, "Discount Taken", 25) ;
#else
	mkln(1, "Escompte prise", 25) ;
#endif
	mkln(29, " ", 1);
	tedit((char*)&O_disc_taken, MASK_13_2, line+cur_pos, R_DOUBLE) ;
	cur_pos += 15 ;
	mkln((cur_pos+1), " ", 1);
	tedit((char*)&C_disc_taken, MASK_13_2, line+cur_pos, R_DOUBLE) ;
	cur_pos += 15 ;
	amt_total = O_disc_taken + C_disc_taken ;
	mkln((cur_pos+1), " ", 1);
	tedit((char*)&amt_total, MASK_13_2, line+cur_pos, R_DOUBLE) ;
	cur_pos += 15 ;
	if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
	mkln(1, "Net Payment Amount", 30) ;
#else
	mkln(1, "Montant du paiement net", 35) ;
#endif
	mkln(29, " ", 1);
	amt_total = O_gr_amt - O_disc_taken ;
	tedit((char*)&amt_total, MASK_13_2, line+cur_pos, R_DOUBLE) ;
	cur_pos += 15 ;
	mkln((cur_pos+1), " ", 1);
	amt_total = C_gr_amt - C_disc_taken ;
	tedit((char*)&amt_total, MASK_13_2, line+cur_pos, R_DOUBLE) ;
	cur_pos += 15 ;
	amt_total = O_gr_amt - O_disc_taken + C_gr_amt - C_disc_taken ;
	mkln((cur_pos+1), " ", 1);
	tedit((char*)&amt_total, MASK_13_2, line+cur_pos, R_DOUBLE) ;
	cur_pos += 15 ;
	if(prnt_line() < 0) return(ERROR);
	
	return(NOERROR) ;
}	/* PrintCloseTotals() */
/*-------------------- E n d   O f   P r o g r a m ---------------------*/

