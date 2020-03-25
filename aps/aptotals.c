/*-----------------------------------------------------------------------
Source Name: aptotals.c
System     : Accounts Payables.
Created  On: 29rd November 89.
Created  By: T AMARENDRA.

COBOL Source(s): cp030---07

DESCRIPTION:
	This Program prints the partition summary totals for invoices
	entered or maintained thru Invoice entry screens.

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
F.Tao	       90/12/31       Fix TOTAL GROSS formula on New Year's EVE.
L.Robichaud	1993/10/13	Pass new parameter to close_rep function for 
				a banner to print with 3000 family.
------------------------------------------------------------------------*/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>
#include <apinvc.h>
#include <repdef.h>

static	short	Print ;

/** Must always be P-printer   D-display   F-file **/
/**  used by report writer no user interface **/

#define PRINTER		'P'

#ifdef ENGLISH
static	char	*TypeString[] = { "INVOICE", "RETURN", "CR MEMO", "DB MEMO" } ;
#else
static	char	*TypeString[] = { "FACTURE", "RENVOI", "NOTE DE CR", "NOTE DE DB" } ;
#endif

/*------------------------------------------------------------------------*/

PrintSummary(output_on)
char	*output_on ;
{
	TrSummary	GrCR_total, GrDB_total , Net_total ;
	int		i ;
	double		net_amt ;

	/* Open the Printer */
	if(opn_prnt(output_on , terminal, 1, e_mesg, 1) < 0){
		DispError(e_mesg);
		return(ERROR) ;
	}

	cur_pos = 0;
	linecnt = 0 ;
	if(output_on[0] == PRINTER) {
		LNSZ = 133 ;
		Print = 1 ;
	}
	else {
		fomcs() ;
		fomrt() ;
		LNSZ = 80 ;
		Print = 0 ;
	}

	PrintHeadings(1) ;

	/* Print Transaction Totals */
	GrCR_total.O_gross = 0.0 ;
	GrCR_total.O_disc  = 0.0 ;
	GrCR_total.C_gross = 0.0 ;
	GrCR_total.C_disc  = 0.0 ;
	GrDB_total.O_gross = 0.0 ;
	GrDB_total.O_disc  = 0.0 ;
	GrDB_total.C_gross = 0.0 ;
	GrDB_total.C_disc  = 0.0 ;

	/* Print Each type total lines & Accumulate */

	for(i = 0 ; i < 4 ; i++) {
#ifdef ENGLISH
		PrintTransTotals(TypeString[i], "CR", CR_totals[i]) ;
		PrintTransTotals("\0", "DB", DB_totals[i]) ;
#else
		PrintTransTotals(TypeString[i], "CR", CR_totals[i]) ;
		PrintTransTotals("\0", "DB", DB_totals[i]) ;
#endif

		if(Print)
			if(prnt_line() < 0) return(ERROR);	/* Blank Line */

		GrCR_total.O_gross += CR_totals[i].O_gross ;
		GrCR_total.O_disc  += CR_totals[i].O_disc  ;
		GrCR_total.C_gross += CR_totals[i].C_gross ;
		GrCR_total.C_disc  += CR_totals[i].C_disc  ;

		GrDB_total.O_gross += DB_totals[i].O_gross ;
		GrDB_total.O_disc  += DB_totals[i].O_disc  ;
		GrDB_total.C_gross += DB_totals[i].C_gross ;
		GrDB_total.C_disc  += DB_totals[i].C_disc  ;
	}

	if(prnt_line() < 0) return(ERROR);	/* Blank Line */
#ifdef ENGLISH
	PrintTransTotals("TOTALS", "CR", GrCR_total) ;
	PrintTransTotals("\0", "DB", GrDB_total) ;
#else
	PrintTransTotals("TOTAUX", "CR", GrCR_total) ;
	PrintTransTotals("\0", "DB", GrDB_total) ;
#endif

	/* Print NET Toatl */
	Net_total.O_gross = GrCR_total.O_gross + GrDB_total.O_gross ;
	Net_total.O_disc  = GrCR_total.O_disc  + GrDB_total.O_disc  ;
	Net_total.C_gross = GrCR_total.C_gross + GrDB_total.C_gross ;
	Net_total.C_disc  = GrCR_total.C_disc  + GrDB_total.C_disc  ;
#ifdef ENGLISH
	PrintTransTotals("\0", "NET", Net_total) ;
#else
	PrintTransTotals("\0", "NET", Net_total) ;
#endif

	if(prnt_line() < 0) return(ERROR);
		
	/* Total Gross */
	if(Print) {
		if(prnt_line() < 0) return(ERROR);
		if(prnt_line() < 0) return(ERROR);
	}

	if(Print) mkln(22, " ", 1);
#ifdef ENGLISH
	mkln((cur_pos+1), "TOTAL NET", 11) ;
#else
	mkln((cur_pos+1), "TOTAL NET", 10) ;
#endif
	if(Print)
		mkln(39, " ", 1) ;
	else
		mkln(14, " ", 1) ;
/*	net_amt = GrCR_total.O_gross + GrDB_total.O_gross +
			GrDB_total.C_gross + GrDB_total.C_gross ; */
	net_amt = GrCR_total.O_gross + GrDB_total.O_gross +
			GrCR_total.C_gross + GrDB_total.C_gross - 
			GrCR_total.O_disc - GrDB_total.O_disc - 
			GrCR_total.C_disc - GrDB_total.C_disc  ;
	tedit((char*)&net_amt, "___,___,_0_.__-", line+cur_pos, R_DOUBLE) ;
	cur_pos += 15 ;
	if(prnt_line() < 0) return(ERROR);

	if( !Print ) {
		if(next_page() < 0) {
			close_rep(BANNER) ;
			return(NOERROR) ;
		}
		PrintHeadings(0) ;
	}
	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);

	/* Print Maintenance Totals */

#ifdef ENGLISH
	PrintMaintTotals("Total Release of Holdback ", RelseHB );
	PrintMaintTotals("Net Changes to Holdback  (Maintenance)", HBchanges );
	PrintMaintTotals("Net Changes to Discount (Maintenance)", DiscChanges );
	PrintMaintTotals("No. and Total of Stopped Transactions", StopPmnt );
	PrintMaintTotals("No. and Total of Partial Payments", PartPmnt );
	PrintMaintTotals("No. and Total of Manual Cheques", ManCheques );
#else
	PrintMaintTotals("Relache entiere du paiement retenu", RelseHB );
	PrintMaintTotals("Changements nets au paiement ret. (Entretien)", HBchanges );
	PrintMaintTotals("Changements nets a l'escompte (Entretien)", DiscChanges );
	PrintMaintTotals("No. et total des transactions arretees", StopPmnt );
	PrintMaintTotals("No. et total des paiements partiels", PartPmnt );
	PrintMaintTotals("No. et total des cheques manuels", ManCheques );
#endif

	if( Print ) {
#ifndef	SPOOLER
	rite_top() ;
#endif
	} else {
		last_page() ;
		fomst();
	}

	close_rep(BANNER);		/* Close Report file */

	return(NOERROR) ;
}	/* PrintSummary() */
/*------------------------------------------------------------*/
/* Print Headings */
static	int
PrintHeadings(flg)
int	flg ;	/* 1 - Print Transaction entry Sub Headings */
{
	int	i ;
	long	rundate ;
	char	tnum[5];

	if( !Print) rite_top() ;

	/* Print Program-Id, School District name and Date */
	mkln(1,PROG_NAME, 10) ;
	i = strlen(pa_rec.pa_co_name);
	mkln(((LNSZ-1-i) / 2)+1,pa_rec.pa_co_name, sizeof(pa_rec.pa_co_name));
	if(Print)
		mkln(116," ",1);
	else
		mkln(63," ",1);
#ifdef ENGLISH
	mkln((cur_pos+1),"Date: ",6);
#else
	mkln((cur_pos+1),"Date: ",6);
#endif
	rundate = get_date() ;
	tedit((char*)&rundate, "____/__/__", line+cur_pos,R_LONG);
	cur_pos += 10;
	if(prnt_line() < 0) return(ERROR);
	
	if(Print)
		mkln(50," ",1);
	else
		mkln(24," ",1);
#ifdef ENGLISH
	mkln((cur_pos+1), "SUMMARY  OF  PARTITION  ACTIVITY", 32) ;
#else
	mkln((cur_pos+1), "   RESUME  DE  LA  PARTITION    ", 32) ;
#endif
	if(Print)
		mkln(116," ",1);
	else
		mkln(63," ",1);
#ifdef ENGLISH 
	mkln((cur_pos+1),"Time: ",6);
#else
	mkln((cur_pos+1),"Heure: ",7);
#endif
	i = get_time() ;
	tedit((char*)&i, "__:__", line+cur_pos,R_INT);
	cur_pos += 5;
	if(prnt_line() < 0) return(ERROR);

	if(Print) {
		mkln(61, "Terminal:", 9) ;
		get_tnum(tnum) ;
		mkln(71,tnum,3);
		if(prnt_line() < 0) return(ERROR);

		if(prnt_line() < 0) return(ERROR);
		if(prnt_line() < 0) return(ERROR);
	}

	if(flg == 0) return(NOERROR) ;

	if(prnt_line() < 0) return(ERROR);

	/* Print Transaction Entry Headings */
	if(Print) mkln(14, " ", 1);
#ifdef ENGLISH
	mkln((cur_pos+1), "TRANSACTION  ENTRY", 18) ;
#else
	mkln((cur_pos+1), "ENTREE DE TRANSACTIONS", 22) ;
#endif
	if(prnt_line() < 0) return(ERROR);

	if(Print)
		if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
	if(Print)
		mkln(51, " ", 1);
	else
		mkln(24, " ", 1);
	mkln((cur_pos+1), "ORDINARY SUPPLIERS", 18) ;
#else
	if(Print)
		mkln(52, " ", 1);
	else
		mkln(22, " ", 1);
	mkln((cur_pos+1), "FOURNISSEURS ORDINAIRES", 23) ;
#endif

#ifdef ENGLISH
	if(Print)
		mkln(98, " ", 1);
	else
		mkln(59, " ", 1);
	mkln((cur_pos+1), "CONTRACT SUPPLIERS", 18) ;
#else
	if(Print)
		mkln(99, " ", 1);
	else
		mkln(53, " ", 1);
	mkln((cur_pos+1), "FOURNISSEURS CONTRACTUELS",25) ;
#endif
	if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
	if(Print)
		mkln(48, " ", 1);
	else
		mkln(23, " ", 1);
	mkln((cur_pos+1), "GROSS", 5) ;
#else
	if(Print)
		mkln(49, " ", 1);
	else
		mkln(24, " ", 1);
	mkln((cur_pos+1), "BRUT", 4) ;
#endif
	if(Print)
		mkln(64, " ", 1);
	else
		mkln(35, " ", 1);
#ifdef ENGLISH
	mkln((cur_pos+1), "DISCOUNT", 8) ;
#else
	mkln((cur_pos+1), "ESCOMPTE", 8) ;
#endif
	if(Print)
		mkln(94, " ", 1);
	else
		mkln(58, " ", 1);
#ifdef ENGLISH
	mkln((cur_pos+1), "GROSS", 5) ;
#else
	mkln((cur_pos+1), "BRUT", 4) ;
#endif

#ifdef ENGLISH
	if(Print)
		mkln(109, " ", 1);
	else
		mkln(69, " ", 1);
	mkln((cur_pos+1), "HOLDBACK ", 9) ;
#else
	if(Print)
		mkln(106, " ", 1);
	else
		mkln(66, " ", 1);
	mkln((cur_pos+1), "PAIEMENT RET", 12) ;
#endif
	if(prnt_line() < 0) return(ERROR);

	if(prnt_line() < 0) return(ERROR);

	return(NOERROR) ;
}	/* PrintHeadings() */
/*------------------------------------------------------------*/
/* Print Transaction Totals */
static	int
PrintTransTotals(str1, str2, Totals)
char		*str1, *str2 ;
TrSummary	Totals ;
{
	if(Print) mkln(22, " ", 1) ;
	mkln((cur_pos+1), str1, 7) ;
	if(Print)
		mkln(33, " ", 1);
	else
		mkln(10, " ", 1);
	mkln((cur_pos+1), str2, 3) ;
	if(Print)
		mkln(38, " ", 1);
	else
		mkln(14, " ", 1);
	mkln((cur_pos+1), " ", 1) ;
	tedit((char*)&Totals.O_gross, "___,___,_0_.__-",
				line+cur_pos, R_DOUBLE) ;
	cur_pos += 15 ;

	if(Print) mkln(58, " ", 1) ;
	tedit((char*)&Totals.O_disc, "___,___,_0_.__-",
				line+cur_pos, R_DOUBLE) ;
	cur_pos += 15 ;

	if(Print)
		mkln(85, " ", 1) ;
	else
		mkln(49, " ", 1) ;
	tedit((char*)&Totals.C_gross, "___,___,_0_.__-",
				line+cur_pos, R_DOUBLE) ;
	cur_pos += 15 ;

	if(Print) mkln(104, " ", 1) ;
	tedit((char*)&Totals.C_disc, "___,___,_0_.__-",
				line+cur_pos, R_DOUBLE) ;
	cur_pos += 15 ;

	if(prnt_line() < 0) return(ERROR);

	return(NOERROR) ;
}	/* PrintTransTotals() */
/*------------------------------------------------------------*/
/* Print Maitenance Total line */
static	int
PrintMaintTotals(str, Totals)
char		*str ;
OthSummary	Totals ;
{
	if(prnt_line() < 0) return(ERROR);

	if(Print) mkln(14, " ", 1) ;
	mkln((cur_pos+1), str, 45) ;
	if(Print)
		mkln(57, " ", 1) ;
	else
		mkln(48, " ", 1) ;
	tedit((char*)&Totals.no_trans, "___0_", line+cur_pos, R_SHORT) ;
	cur_pos += 5 ;

	mkln((cur_pos+11), " ", 1) ;
	tedit((char*)&Totals.tot_amt, "___,___,_0_.__-",
				line+cur_pos, R_DOUBLE) ;
	cur_pos += 15 ;
	if(prnt_line() < 0) return(ERROR) ;

	return(NOERROR) ;
}	/* PrintMaintTotals() */
/*-------------------- E n d   O f   P r o g r a m ---------------------*/

