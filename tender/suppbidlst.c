/* --------------------------------------------------------------------------
	SOURCE NAME:  suppbidlst.c
	SYSTEM     :  BUGETARY FINANCIAL SYSTEM 
	MODULE     :  TENDERING SYSTEM 
	CREATED ON :  21 April 1992
	CREATED BY :  J. Prescott 

DESCRIPTION:
	This program prints a Supplier Bid List. 

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
Bid	 bid;
Supplier supplier;

static	char 	e_mesg[80];
static	int 	retval;
static 	long	longdate ;
static 	int	pgcnt;
static	short	copies;
static	short	printer;
static  char	discfile[15] ;
static 	int	outcntl ;
static	char    program[11];
static	char	resp[2] ;
static	int	mode;

/* Ranges */
static	long	item1;
static	long	item2;
static	char	suppcd1[11];
static	char	suppcd2[11];
static	char	prev_supp[11];

SuppBidList()
{
	retval = get_param(&pa_rec,BROWSE,1,e_mesg);
        if(retval < 1) {
		fomer(e_mesg);
		close_dbh();
		return(-1);
	}

	STRCPY(program,"TENDREP");

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
			STRCPY(e_mesg,"tender.dat");
			if((retval = GetFilename(e_mesg)) < 0)
				return(retval);
			STRCPY(discfile,e_mesg);
		}
		else {
			sprintf(discfile,"spool%d",CC_no);
			resp[0] = 'F';
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

	retval = GetSuppRange( suppcd1, suppcd2 );
	if(retval < 0) return(retval);

	retval = GetItemNoRange( &item1, &item2 );
	if(retval < 0) return(retval);

	if((retval = Confirm()) <= 0)
		return(retval);

	longdate = get_date() ;

	retval = opn_prnt(resp,discfile,1,e_mesg,1 /* spool */);  
	if(retval < 0) {
		fomer( e_mesg) ;
		close_dbh() ;
		return(-1);
	}
	SetCopies( (int)copies );

	LNSZ = 80;

	if(outcntl == 0) {
		PGSIZE = 22;
	}
	else {
		PGSIZE = 60;
	}

	linecnt = PGSIZE;
	pgcnt = 0 ;

	prev_supp[0] = '\0';

	strcpy(bid.bid_supp_cd,suppcd1);
	bid.bid_cat_num = 0;
	flg_reset( BID );

        for( ; ; ) {
		retval = get_n_bid(&bid,BROWSE,0,FORWARD,e_mesg);
		if(retval < 0) {
			if(retval == EFL) break;
			fomer(e_mesg);
			break;
		}

		if(strcmp(bid.bid_supp_cd,suppcd2) > 0) {
			break;
		}

		if(bid.bid_cat_num < item1 || bid.bid_cat_num > item2) {
			continue;
		}
		
		if(linecnt >= PGSIZE) {
			retval = PrintHeading();
			if(retval == EXIT)	return(EXIT);
			if(retval < 0) {
				break;
			}
			if((retval = PrintSuppHeading())<0) break;
		}

		if(strcmp(prev_supp,bid.bid_supp_cd) !=0) {
			if(linecnt >= PGSIZE-4) {
				retval = PrintHeading();
				if(retval == EXIT)	return(EXIT);
				if(retval < 0) {
					break;
				}
			}
			if((retval = PrintSuppHeading())<0) break;
		}

		retval = print_item();
		if(retval < 0) break;
	}

	if(pgcnt) {
		if(term < 99) 
			last_page() ;
#ifndef SPOOLER
		else
			rite_top() ;
#endif
	}
	close_rep(BANNER);
	close_dbh();
	if(retval == EFL) retval = NOERROR;
	return(retval);
}
static
PrintHeading()
{
	char	txt_line[80];
	long	longdate ;
	int	i ;
	int	year;

	if ( term < 99 && pgcnt )
		if (next_page() < 0) return(EXIT) ;
	
	if(pgcnt || term < 99) rite_top();   /* if not first page advance */

	linecnt = 0;

	mkln(1,PROG_NAME,10);
	longdate = get_date();
#ifdef ENGLISH
        mkln(51,"DATE:",5);
#else
        mkln(51,"DATE:",5);
#endif
	tedit((char *)&longdate,"____/__/__",txt_line,R_LONG);
	mkln(57,txt_line,10);
	pgcnt++;
#ifdef ENGLISH
	mkln(71,"PAGE:",5);
#else
	mkln(71,"PAGE:",5);
#endif
	tedit((char *)&pgcnt,"_0_",txt_line,R_INT);
	mkln(77,txt_line,3);
	if(prnt_line() < 0) return(ERROR);

	i = strlen(pa_rec.pa_co_name);
	mkln(((LNSZ-1-i)/2)+1,pa_rec.pa_co_name,sizeof(pa_rec.pa_co_name)); 
	if(prnt_line() < 0) return(ERROR);
	
#ifdef ENGLISH
	mkln(32,"SUPPLIER BID LIST",17); 
#else
	mkln(32,"LISTE DES OFFRES D'ACHAT DES FOURNISSEURS",41);
#endif
	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
	mkln(4,"ITEM #",6);
	mkln(27,"NEW DESCRIPTION",15);
	mkln(53,"BID PRICE",9);
#else
	mkln(4,"#ARTICLE",8);
	mkln(27,"NOUV DESCRIPTION",16);
	mkln(53,"PRIX D'OFFRE",12);
#endif
	if(prnt_line() < 0) return(ERROR);

	return(NOERROR);
}

static
PrintSuppHeading() 
{
	char	txt_line[80];


	strcpy(supplier.s_supp_cd,bid.bid_supp_cd);
	retval = get_supplier(&supplier,BROWSE,0,e_mesg);
	if(retval < 0) {
		return(retval);
	}
	if(prnt_line() < 0) return(ERROR);
	mkln(1,"SUPPLIER:",9);
	mkln(12,bid.bid_supp_cd,10);
	mkln(25,supplier.s_name,25);
	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);
	
	strcpy(prev_supp,bid.bid_supp_cd);
	return(NOERROR);
}

/****************************************************************************
   This routine prints two suppliers with full address information.
****************************************************************************/
static
print_item()
{
	char	txt_line[80];
	int	retval;

	tedit((char *)&bid.bid_cat_num,"_____0_",txt_line,R_LONG);
	mkln(5,txt_line,7);
	mkln(15,bid.bid_desc,35);
	tedit((char *)&bid.bid_price,"__0_.____",txt_line,R_DOUBLE);
	mkln(53,txt_line,9);
	if(prnt_line() < 0) return(ERROR);

	return(NOERROR);
} 
/*-------------------------- END OF FILE ---------------------------------- */

