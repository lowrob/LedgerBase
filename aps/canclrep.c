/*-----------------------------------------------------------------------
Source Name: canclrep.c
System     : Accounts Payables.
Created  On: 29rd November 89.
Created  By: T AMARENDRA.

COBOL Source(s): cp170---01

DESCRIPTION:
	This Program prints the Cheque Cancellation Process Report.

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

#define	MASK_2		"0_"			/* 3 Char Mask */
#define	MASK_3		"_0_"			/* 3 Char Mask */
#define	MASK_6		"__,_0_"		/* 6 Char Mask */
#define	MASK_8		"______0_"		/* 8 Char Mask */
#define	DATE_MASK	"____/__/__"		/* Date in YYYY/MM/DD format */
#define	MASK_13_2	"___,___,_0_.__-"	/* 13.2 Mask + Minus Sign */
#define	MASK_12_2	"__,___,_0_.__-"	/* 12.2 Mask + Minus Sign */

static	short	first = 1 ;
static	short	pgcnt ;

/* Totals Variables */
static	short	C_cheques, O_cheques ;
static	short	C_trans, O_trans ;
static	double	ch_gr_amt, ch_disc_taken ;
static	double	C_gr_amt, O_gr_amt, C_disc_taken, O_disc_taken ;

/* Data Record Definitions */
extern	Pa_rec		pa_rec ;	/* Parameters Record */
extern	Ctl_rec		ctl_rec ;	/* Fund/Control Record */
extern	Supplier	supp_rec ;	/* Supplier */
extern	Invoice		in_rec ;	/* Purchase Invoice */
extern	Chq_rec		chq_rec ;	/* Cheque Record */

extern	char 		e_mesg[100];	/* dbh will return err msg in this */

/*------------------------------------------------------------------------*/
/* Open Report File */

OpenReport()
{
	/* Always Open the Printer */
	if(opn_prnt("P" , terminal, 1, e_mesg, 1) < 0) {
		DispError(e_mesg) ;
		return(ERROR) ;
	}

	LNSZ = 133 ;
	cur_pos = 0;
	pgcnt = 0 ;
	linecnt = PGSIZE ;
	first = 0 ;

	/* Initialize Totals variables */
	C_cheques = 0 ;
	O_cheques = 0 ;
	C_trans = 0 ;
	O_trans = 0 ;
	C_gr_amt = 0.0 ;
	O_gr_amt = 0.0 ;
	C_disc_taken = 0.0 ;
	O_disc_taken = 0.0 ;
	
	return(NOERROR) ;
}	/* OpenReport() */
/*------------------------------------------------------------*/
/* Close report files */

CloseReport()
{
	if(pgcnt) {	/* If report started, ie Cheque is cancelled */
		PrintCloseTotals() ;
#ifndef	SPOOLER
		rite_top() ;
#endif
	}
	close_rep(BANNER);		/* Close Report file */

	return(NOERROR) ;
}	/* CloseReport() */
/*------------------------------------------------------------*/
/* Print Supplier Code, Name etc as a subheading */

PrintSubHdg()
{
	int	err ;

	if(linecnt >= (PGSIZE - 4)) {
		if((err = PrintHeadings(0)) != NOERROR) return(err) ;
		/* Need not call PrintSupplier(), because PrintHeadings()
		  does it */
	}
	else
		if((err = PrintSupplier()) != NOERROR) return(err) ;

	if(supp_rec.s_type[0] == ORDINARY)
		O_cheques++ ;
	else
		C_cheques++ ;

	/* Initialize Totals */
	ch_gr_amt = 0.0 ;
	ch_disc_taken = 0.0 ;

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
	if( pgcnt ) rite_top() ;
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
	mkln(55, "CANCELLED CHEQUES REPORT", 26) ;
#else
        mkln(54, "RAPPORT DES CHEQUES ANNULES", 29) ;
#endif
	if(prnt_line() < 0) return(ERROR);

	if(prnt_line() < 0) return(ERROR);

	if(flg == 1) return(NOERROR) ;	/* Printing Close Totals */

	/* Print Column Headings */
#ifdef ENGLISH
	mkln(1, "Fund      Supplier     Hold BACK/  Cheque#       Tran", 53);
	mkln(69, "Tran", 4) ;
	mkln(82, "Due    Gross Payment       Discount          Net", 50);
#else
	mkln(1, "Fond     Balance du    Pay.retenu/  #Cheque      #Ref", 53);
	mkln(69, "Date de", 7) ;
	mkln(82, "        Paiement brut      Escompte        Paiement", 51);
#endif
	if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
	mkln(12, "Balance       Discount", 22) ;
	mkln(50, "Ref#    Type Per   Date        Date    /Tran Balance", 52) ;
	mkln(111, "Taken         Payment", 21) ;
#else
	mkln(10, "Fournisseur     Escompte", 24) ;
	mkln(50, "Trans  Genre Per   Trans     Echeance  /Balance trans", 52) ;
	mkln(111, "Prise           Net", 19) ;
#endif
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
	mkln(27, supp_rec.s_name, 48 );
#ifdef ENGLISH
	mkln(73, "Type:", 5) ;
#else
	mkln(73, "Genre:", 6) ;
#endif
	mkln((cur_pos+2), supp_rec.s_type, 1) ;
	if(prnt_line() < 0) return(ERROR);

	if(prnt_line() < 0) return(ERROR);	/* Print Blank line */

	first = 1 ;	/* Print Supplier Total, Fund etc in PrintPayment() */

	return(NOERROR) ;
}	/* PrintSupplier() */
/*------------------------------------------------------------*/
/* Print Cancelled cheque details. Print this information from cheque
   records. Invoice Date, Due date etc print from Invoice Record */

PrintPayment()
{
	double	net ;
	int	err ;

	if(linecnt >= PGSIZE-1)
		if((err = PrintHeadings(0)) != NOERROR) return(err) ;

	if( first) {
		tedit((char*)&chq_rec.c_funds, MASK_3, line+cur_pos, R_SHORT) ;
		cur_pos += 3 ;
		mkln(4, " ", 1);
		tedit((char*)&supp_rec.s_balance, MASK_12_2, line+cur_pos,
				R_DOUBLE) ;
		cur_pos += 14 ;
		mkln(19, " ", 1);
		tedit((char*)&supp_rec.s_ytd_disc, MASK_12_2, line+cur_pos,
				R_DOUBLE) ;
		cur_pos += 14 ;
		mkln(34, " ", 1);
		tedit((char*)&chq_rec.c_chq_no, MASK_8, line+cur_pos, R_LONG) ;
		cur_pos += 8 ;

		first = 0 ;
	}

	mkln(44, chq_rec.c_invc_no, 15) ;
	mkln(60, chq_rec.c_tr_type, 2) ;
	mkln(62, " ", 1);
	tedit((char*)&in_rec.in_period, MASK_2, line+cur_pos, R_SHORT) ;
	cur_pos += 2 ;
	mkln(65, " ", 1);
	tedit((char*)&in_rec.in_invc_dt, DATE_MASK, line+cur_pos, R_LONG) ;
	cur_pos += 10 ;
	mkln(77, " ", 1);
	tedit((char*)&in_rec.in_due_dt, DATE_MASK, line+cur_pos, R_LONG) ;
	cur_pos += 10 ;
	mkln(88, " ", 1);
	tedit((char*)&chq_rec.c_gr_amt, MASK_12_2, line+cur_pos, R_DOUBLE) ;
	cur_pos += 14 ;
	mkln(103, " ", 1);
	tedit((char*)&chq_rec.c_disc_taken, MASK_12_2, line+cur_pos, R_DOUBLE) ;
	cur_pos += 14 ;

	net = chq_rec.c_gr_amt - chq_rec.c_disc_taken ;
	mkln(118, " ", 1);
	tedit((char*)&net, MASK_12_2, line+cur_pos, R_DOUBLE) ;
	cur_pos += 14 ;
	if(prnt_line() < 0) return(ERROR);

	mkln(88, " ", 1);
	tedit((char*)&in_rec.in_amount, MASK_12_2, line+cur_pos, R_DOUBLE) ;
	cur_pos += 14 ;
	if(prnt_line() < 0) return(ERROR);

	if(supp_rec.s_type[0] == ORDINARY)
		O_trans++ ;
	else
		C_trans++ ;

	ch_gr_amt     += chq_rec.c_gr_amt ;
	ch_disc_taken += chq_rec.c_disc_taken ;

	return(NOERROR) ;
}	/* PrintPayment() */
/*------------------------------------------------------------*/
/* Print Cheque Totals */

PrintChqTotals()
{
	double	net ;

	if(prnt_line() < 0) return(ERROR);
	mkln(4, " ", 1);
	tedit((char*)&supp_rec.s_balance, MASK_12_2, line+cur_pos, R_DOUBLE) ;
	cur_pos += 14 ;
	mkln(19, " ", 1);
	tedit((char*)&supp_rec.s_ytd_disc, MASK_12_2, line+cur_pos, R_DOUBLE) ;
	cur_pos += 14 ;

	mkln(88, " ", 1);
	tedit((char*)&ch_gr_amt, MASK_12_2, line+cur_pos, R_DOUBLE) ;
	cur_pos += 14 ;
	mkln(103, " ", 1);
	tedit((char*)&ch_disc_taken, MASK_12_2, line+cur_pos, R_DOUBLE) ;
	cur_pos += 14 ;

	net = ch_gr_amt - ch_disc_taken ;
	mkln(118, " ", 1);
	tedit((char*)&net, MASK_12_2, line+cur_pos, R_DOUBLE) ;
	cur_pos += 14 ;
	if(prnt_line() < 0) return(ERROR);

	if(supp_rec.s_type[0] == ORDINARY) {
		O_gr_amt += ch_gr_amt ;
		O_disc_taken += ch_disc_taken ;
	}
	else {
		C_gr_amt += ch_gr_amt ;
		C_disc_taken += ch_disc_taken ;
	}

	return(NOERROR) ;
}	/* PrintChqTotals() */
/*------------------------------------------------------------*/
/* Print Closing Process Toatls */
static	int
PrintCloseTotals()
{
	int	err ;
	short	tr_total ;
	double	amt_total ;

	if(linecnt >= (PGSIZE - 12)) {
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
	mkln(1, "No of Cheques Cancelled", 30) ;
#else
	mkln(1, "Nombre de cheques annules", 32) ;
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
	mkln(1, "No of Transactions Reversed", 30) ;
#else
	mkln(1, "Nombre de transactions renversees", 36) ;
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
	mkln(1, "Gross Amount Cancelled", 25) ;
#else
	mkln(1, "Montant brut annule", 22) ;
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
	mkln(1, "Discount Cancelled", 30) ;
#else
	mkln(1, "Escompte annule", 27) ;
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
	mkln(1, "Net Amount Cancelled", 30) ;
#else
	mkln(1, "Montant net annule", 28) ;
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

