/*
*	apinvc.h
*/

#define	DELTA_AMT	0.005	/* To Check float & double amounts for 0 */
#define	DELTA_QTY	0.00005	/* To Check float & double QTYs for 0 */
#define	ESC_F		1	/* ESC_F is active */
#define	DEBIT		1
#define	CREDIT		-1
#define DUPEREF		-44

/* Transaction Types */
#ifdef ENGLISH
#define	T_INVOICE	"IN"
#define	T_RETURN	"RT"
#define	T_CRMEMO	"CM"
#define	T_DBMEMO	"DM"
#else
#define	T_INVOICE	"FC"
#define	T_RETURN	"RV"
#define	T_CRMEMO	"NC"
#define	T_DBMEMO	"ND"
#endif

#define	INVOICE		0
#define	RETURN		1
#define	CRMEMO		2
#define	DBMEMO		3

Pa_rec		pa_rec ;	/* Parameters Record */
Ctl_rec		ctl_rec ;	/* Fund/Control Record */
Supplier	supp_rec ;	/* Supplier */
Supplier	payee_rec;	/* Payee Record */
Gl_rec		gl_rec ;	/* Gl Master record */
Invoice		in_rec ;	/* Purchase Invoice */
In_hdr		in_hdr ;	/* Invoice Header */
In_item		in_item ;	/* Invoice Item */
Chq_hist	chqhist ;	/* Invoice Item */

char 		e_mesg[100];	/* dbh will return err msg in this */

/* PROFOM Related variables */

char		*CurrentScr ;	/* Ptr to Current Profom Screen */
int		END_FLD	;	/* Last Field# of Current Screen */

/* Partition total Variables */

typedef	struct	{

	double	O_gross ;	/* Total Gross For ordinary Supplier */
	double	O_disc ;	/* Total Discount for ordinary Supplier */
	double	C_gross ;	/* Total Gross for Contract Supplier */
	double	C_disc ;	/* Total Discount for Contract Supplier */

}	TrSummary ;

TrSummary	CR_totals[4],	/* Credit Totals for each transaction type */
		DB_totals[4] ;	/* Debit Totals for each transaction type */

typedef	struct	{

	short	no_trans ;	/* No of trans */
	double	tot_amt ;	/* Total amount */

}	OthSummary ;

OthSummary	RelseHB,	/* Release of HB totals */
		HBchanges,	/* Net Changes to HB thru maintenance */
		DiscChanges,	/* Net Changes to Discount thru maintenance */
		StopPmnt,	/* Stop Payment Totals */
		PartPmnt,	/* Partial Payment Totals */
		ManCheques ;	/* Manual Cheque Totals */

short	TotalsUpdated ;		/* Flag to Print the Summary Reports */

/*-------------------- E n d   O f   F i l e ---------------------*/

