/* --------------------------------------------------------------------------
	SOURCE NAME:  suppawdlst.c
	SYSTEM     :  BUGETARY FINANCIAL SYSTEM 
	MODULE     :  TENDERING SYSTEM 
	CREATED ON :  21 April 1992
	CREATED BY :  J. Prescott 

DESCRIPTION:
	This program prints a Supplier Award List. 

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

#define AWARD_STAT	"AWD"
#else
#define PRINTER 'I'
#define DISPLAY 'A'
#define FILE_IO	'D'
#define SPOOL	'B'	/* added for requisitions */
#define YES	'O'
#define NO	'N'

#define AWARD_STAT	"ATT"
#endif

Pa_rec   pa_rec;             /*  Declarations for DBH record reading */
Bid	 bid;
Supplier supplier;
Item_grp item_grp;
Catalogue catalogue;

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
static	char	rptonqty[2];
static	char	prev_supp[11];
static	short	prev_grp;
static	short	no_award;
static	double	total_award;

SuppAwardList()
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

	rptonqty[0] = '\0';
	retval = GetRptonQty(rptonqty);
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
	prev_grp = 0;

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
		
		if(strcmp(bid.bid_status,AWARD_STAT)!=0) {
			continue;
		}

		if(strcmp(prev_supp,bid.bid_supp_cd)!=0 && 
		   prev_supp[0] != '\0' && rptonqty[0] == YES) {
			if(linecnt >= PGSIZE-4) {
				retval = PrintHeading();
				if(retval == EXIT) return(EXIT);
				if(retval < 0) {
					break;
				}
				if((retval = PrintSuppHeading())<0) break;
			}
			retval = PrintSuppTotal();
			if(retval < 0) {
				break;
			}
		}

		if(linecnt >= PGSIZE || strcmp(prev_supp,bid.bid_supp_cd)!=0) {
			retval = PrintHeading();
			if(retval == EXIT) return(EXIT);
			if(retval < 0) {
				break;
			}
			if((retval = PrintSuppHeading())<0) break;
		}

		if(rptonqty[0] == YES && bid.bid_cat_num / 100 != prev_grp) {
			if(linecnt >= PGSIZE - 5) {
				retval = PrintHeading();
				if(retval == EXIT) return(EXIT);
				if(retval < 0) {
					break;
				}
				if((retval = PrintSuppHeading())<0) break;
			}
			retval = PrintItemGrpHeading();
			if(retval < 0) break;
		}
		retval = print_item();
		if(retval < 0) break;
	}
	if(rptonqty[0] == YES) {
		retval = PrintSuppTotal();
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
	mkln(31,"SUPPLIER AWARD LIST",19); 
#else
	mkln(32,"LISTE DES ATTRIBUTIONS DES FOURNISSEURS",39);
#endif
	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);

	if(rptonqty[0] == YES) {
#ifdef ENGLISH
		mkln(5,"ITEM #",6);
		mkln(25,"DESCRIPTION",11);
		mkln(49,"UOM",3);
		mkln(56,"QTY",3);
		mkln(62,"UNIT PRICE",10);
		mkln(74,"TOTAL",5);
#else
		mkln(5,"#ARTICLE",8);
		mkln(25,"DESCRIPTION",11);
		mkln(49,"UDM",3);
		mkln(56,"QTE",3);
		mkln(62,"PRIX UNIT",9);
		mkln(74,"TOTAL",5);
#endif
	}
	else {
#ifdef ENGLISH
		mkln(5,"ITEM #",6);
		mkln(23,"DESCRIPTION",11);
		mkln(56,"UNIT PRICE",10);
		mkln(69,"ADJUSTMENT",10);
#else
		mkln(5,"#ARTICLE",8);
		mkln(23,"DESCRIPTION",11);
		mkln(56,"PRIX UNIT",9);
		mkln(69,"AJUSTEMENT",10);
#endif
	}
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

	if(rptonqty[0] != YES) {
		if(prnt_line() < 0) return(ERROR);
	}
	
	strcpy(prev_supp,bid.bid_supp_cd);
	prev_grp = 0;
	return(NOERROR);
}
PrintItemGrpHeading()
{
	int	retval;
	char	txt_line[80];

	item_grp.itmgrp_num = bid.bid_cat_num / 1000;
	retval = get_itemgrp(&item_grp,BROWSE,0,e_mesg);
	if(retval < 0) {
		return(retval);
	}

	if(prnt_line() < 0) return(ERROR);
	mkln(12,"ITEM GR:",8);
	mkln(21,item_grp.itmgrp_desc1,40);	
	if(prnt_line() < 0) return(ERROR);
	if(item_grp.itmgrp_desc2[0] != '\0') {
		mkln(21,item_grp.itmgrp_desc2,40);	
		if(prnt_line() < 0) return(ERROR);
	}
	if(item_grp.itmgrp_desc3[0] != '\0') {
		mkln(21,item_grp.itmgrp_desc3,40);	
		if(prnt_line() < 0) return(ERROR);
	}
	if(prnt_line() < 0) return(ERROR);
	prev_grp = item_grp.itmgrp_num;
	return(NOERROR);
}
/****************************************************************************
   This routine prints two suppliers with full address information.
****************************************************************************/
static
print_item()
{
	char	txt_line[80];
	double	total;
	int	retval;
	long	qty;

	catalogue.cat_num = bid.bid_cat_num;
	retval = get_catalogue(&catalogue,BROWSE,0,e_mesg);
	if(retval < 0) {
		return(retval);
	}

	if(rptonqty[0] == NO) {
		tedit((char *)&bid.bid_cat_num,"_____0_",txt_line,R_LONG);
		mkln(5,txt_line,7);
		mkln(15,catalogue.cat_desc,40);
		tedit((char *)&bid.bid_price,"__0_.____",txt_line,R_DOUBLE);
		mkln(57,txt_line,9);
		mkln(70,"________",8);
	}
	else {
		tedit((char *)&bid.bid_cat_num,"_____0_",txt_line,R_LONG);
		mkln(5,txt_line,7);
		mkln(14,catalogue.cat_desc,40);
		mkln(47,catalogue.cat_uom,6);
		qty = catalogue.cat_qty[1] + .5;
		tedit((char *)&qty,"__0_",txt_line,R_LONG);
		mkln(55,txt_line,4);
		tedit((char *)&bid.bid_price,"__0_.____",txt_line,R_DOUBLE);
		mkln(62,txt_line,9);
		total = bid.bid_price * catalogue.cat_qty[1];
		tedit((char *)&total,"___0_.__",txt_line,R_DOUBLE);
		mkln(72,txt_line,8);

		no_award++;
		total_award += total;
	}
	if(prnt_line() < 0) return(ERROR);
	return(NOERROR);
} 

PrintSuppTotal()
{
	char	txt_line[80];

	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);
	mkln(25,"NO OF ITEMS AWARDED",19);
	tedit((char *)&no_award,"_0_",txt_line,R_SHORT);
	mkln(67,txt_line,3);
	mkln(71,"=========",9);
	if(prnt_line() < 0) return(ERROR);
		
	mkln(25,"VALUE OF AWARDS BASED ON TENDER ESTIMATES",41);
	tedit((char *)&total_award,"____0_.__",txt_line,R_DOUBLE);
	mkln(71,txt_line,9);
	if(prnt_line() < 0) return(ERROR);
	
	return(NOERROR);
}
/*-------------------------- END OF FILE ---------------------------------- */

