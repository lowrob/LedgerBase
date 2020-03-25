/* --------------------------------------------------------------------------
	SOURCE NAME:  tendspread.c
	SYSTEM     :  BUGETARY FINANCIAL SYSTEM 
	MODULE     :  TENDERING SYSTEM 
	CREATED ON :  21 April 1992
	CREATED BY :  J. Prescott 

DESCRIPTION:
	This program prints a Tender SpreadSheet. 

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
Category category;
Item_grp item_grp;
Catalogue catalogue;
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
static	short	category1;
static	short	category2;
static	long	item1;
static	long	item2;
static	char	rpt_qty[2];

TenderSpread()
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

	retval = GetCategRange( &category1, &category2 );
	if(retval < 0) return(retval);

	retval = GetItemNoRange( &item1, &item2 );
	if(retval < 0) return(retval);

	rpt_qty[0] = ' ';
	retval = GetRptonQty( rpt_qty );
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

	category.categ_num = category1;

	flg_reset( CATEGORY );

        for( ; ; ) {
		retval = get_n_category(&category,BROWSE,0,FORWARD,e_mesg);
		if(retval < 0) {
			if(retval == EFL) break;
			fomer(e_mesg);
			break;
		}

		if(category.categ_num > category2) {
			break;
		}

		retval = PrintHeading();
		if(retval < 0) {
			break;
		}

		item_grp.itmgrp_num = category.categ_num * 100;

		flg_reset(ITEM_GROUP);

		for( ; ; ) {
			retval=get_n_itemgrp(&item_grp,BROWSE,0,FORWARD,e_mesg);
			if(retval < 0) {
				if(retval == EFL) break;
				fomer(e_mesg);
				break;
			}
			
			if(item_grp.itmgrp_num / 100 != category.categ_num) {
				break;
			}

			if(linecnt >= PGSIZE-7) {
				retval = PrintHeading();
				if(retval == EXIT)	return(EXIT);
				if(retval < 0) {
					break;
				}
			}

			if(PrintItemGrpHeading() < 0) return(ERROR);

			catalogue.cat_num = item_grp.itmgrp_num * 1000;

			flg_reset(CATALOGUE);
			for( ; ; ) {
				retval=get_n_catalogue(&catalogue,BROWSE,0,
					FORWARD,e_mesg);
				if(retval < 0) {
					if(retval == EFL) break;
					fomer(e_mesg);
					break;
				}
			
				if(catalogue.cat_num / 1000 != 
				   item_grp.itmgrp_num) {
					break;
				}

				if(linecnt >= PGSIZE-3) {
					retval = PrintHeading();
					if(retval == EXIT)	return(EXIT);
					if(retval < 0) {
						break;
					}
					if(PrintItemGrpHeading() < 0) 
						return(ERROR);
				}

				if(PrintItemHeading() < 0) {
					return(ERROR);
				}

				retval = PrintSuppliers();
				if(retval < 0) break;
			}
		}

		if(retval < 0 && retval != EFL) {
			break;
		}

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
	mkln(28,"SPREAD SHEET - ",15); 
#else
	mkln(28,"TABLEAU",7);
#endif
	year = longdate / 10000;
	tedit((char *)&year,"____",txt_line,R_INT);
	mkln(43,txt_line,4);
	mkln(48,"TENDER",6);

	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);

	sprintf(txt_line,"%d %s",category.categ_num,category.categ_desc);
	mkln(((LNSZ-1-i)/2)+1,txt_line,strlen(txt_line)); 
	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
	mkln(1,"SNO",3);
	mkln(7,"SUPPLIER CODE",13);
	mkln(29,"SUPPLIER NAME",13);
	mkln(50,"NEW DESCRIPTION",15);
	mkln(69,"BID",3);
#else
	mkln(1,"NO S",4);
	mkln(7,"CD FOURN",8);
	mkln(29,"NOM FOURN",9);
	mkln(50,"NOUV DESC",9);
	mkln(69,"OFFRE",5);
#endif
	if(prnt_line() < 0) return(ERROR);

	return(NOERROR);
}

static
PrintItemGrpHeading() 
{
	char	txt_line[80];

	if(prnt_line() < 0) return(ERROR);
	mkln(15,"ITEM GR:",8);
	tedit((char *)&item_grp.itmgrp_num,"__0_",txt_line,R_SHORT);
	mkln(24,txt_line,4);
	mkln(31,item_grp.itmgrp_desc1,40);
	if(prnt_line() < 0) return(ERROR);
	if(item_grp.itmgrp_desc2[0] != '\0') {
		mkln(31,item_grp.itmgrp_desc2,40);
		if(prnt_line() < 0) return(ERROR);
	}
	if(item_grp.itmgrp_desc3[0] != '\0') {
		mkln(31,item_grp.itmgrp_desc3,40);
		if(prnt_line() < 0) return(ERROR);
	}
	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);
	
	return(NOERROR);
}

PrintItemHeading()
{
	char	txt_line[80];
	long	qty;

	mkln(7,"ITEM #:",7);
	tedit((char *)&catalogue.cat_num,"_____0_",txt_line,R_LONG);
	mkln(15,txt_line,7);
	mkln(24,catalogue.cat_desc,40);
	if(rpt_qty[0] == YES) {
		mkln(70,"QTY:",4);
		qty = catalogue.cat_qty[1] + .5;
		tedit((char *)&qty,"___0_",txt_line,R_LONG);
		mkln(75,txt_line,5);
	}
	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);

	return(NOERROR);
}

PrintSuppliers()
{
	int	retval;
	int	sno;

	bid.bid_cat_num = catalogue.cat_num;
	bid.bid_price = 0.00;
	bid.bid_supp_cd[0] = '\0';
	flg_reset(BID);
	for(sno=0; ;) {
		retval = get_n_bid(&bid,BROWSE,2,FORWARD,e_mesg);
		if(retval < 0 && retval != EFL) return(retval);
		if(retval == EFL) break;

		if(bid.bid_cat_num != catalogue.cat_num) break;

		sno++;

		retval = print_line(sno);
		if(retval < 0) {
			return(ERROR);
		}
	}

	/* Print Seperator Line */
	if(prnt_line() < 0) return(ERROR);
	mkln(1,"----------------------------------------",40);
	mkln(41,"---------------------------------------",39);
	if(prnt_line() < 0) return(ERROR);
	
	return(NOERROR);
}
/****************************************************************************
   This routine prints two suppliers with full address information.
****************************************************************************/
static
print_line(sno)
int	sno;
{
	char	txt_line[80];
	int	retval;

	if(linecnt >= PGSIZE) {
		if(PrintHeading() < 0) return(ERROR);

		if(PrintItemGrpHeading() < 0) return(ERROR);

		if(PrintItemHeading() < 0) return(ERROR);
	}

	tedit((char *)&sno,"_0_",txt_line,R_INT);
	mkln(1,txt_line,3);
	mkln(8,bid.bid_supp_cd,10);

	strcpy(supplier.s_supp_cd,bid.bid_supp_cd);
	retval = get_supplier(&supplier,BROWSE,0,e_mesg);
	if(retval < 0) {
		return(retval);
	}
	mkln(23,supplier.s_name,25);
	mkln(51,bid.bid_desc,13);

	tedit((char *)&bid.bid_price,"__0_.____",txt_line,R_DOUBLE);
	mkln(67,txt_line,9);
	if(prnt_line() < 0) return(ERROR);

	return(NOERROR);
} 
/*-------------------------- END OF FILE ---------------------------------- */

