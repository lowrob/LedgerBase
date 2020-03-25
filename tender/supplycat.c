/* --------------------------------------------------------------------------
	SOURCE NAME:  supplycat.c
	SYSTEM     :  BUGETARY FINANCIAL SYSTEM 
	MODULE     :  TENDERING SYSTEM 
	CREATED ON :  21 April 1992
	CREATED BY :  J. Prescott 

DESCRIPTION:
	This program prints a General Supply Catalogue. 

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
Item_grp item_grp;
Category category;
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
static	long	cat1, cat2;
static	short	categ1, categ2;
static	short	itemgrp1, itemgrp2;
static	char	bycategory[2];
static	char	byitemgrp[2];
static	short	prev_categ;
static	short	prev_itemgrp;
static	int	extralines;

SupplyCatalogue()
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

	bycategory[0] = ' ';
	retval = GetRptonCategory( bycategory ) ;
	if(retval < 0) return(retval);

	byitemgrp[0] = ' ';
	retval = GetRptonItemGrp( byitemgrp ) ;
	if(retval < 0) return(retval);
	
	if(bycategory[0] == YES) {
		retval = GetCategRange( &categ1, &categ2 );
		if(retval < 0) return(retval);
	}

	if(byitemgrp[0] == YES) {
		retval = GetItemGrpRange( &itemgrp1, &itemgrp2 );
		if(retval < 0) return(retval);
	}

	retval = GetCatalogueRange( &cat1, &cat2 );
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

	extralines = 0;
	if(byitemgrp[0] == YES) 
		extralines = 5;

	prev_categ = 0;
	prev_itemgrp = 0;

	catalogue.cat_num = cat1;
	flg_reset( CATALOGUE );

        for( ; ; ) {
		retval = get_n_catalogue(&catalogue,BROWSE,0,FORWARD,e_mesg);
		if(retval < 0) {
			if(retval == EFL) break;
			fomer(e_mesg);
			break;
		}

		if(bycategory[0] == YES) { 
		   	if(catalogue.cat_num / 100000 < categ1) {
				continue;
			}
		   	if(catalogue.cat_num / 100000 > categ2) {
				break;
			}
			
			category.categ_num = catalogue.cat_num / 100000;
			retval = get_category(&category,BROWSE,0,e_mesg);
			if(retval < 0) {
				break;
			}
			if(category.categ_num > categ2) break;
			if(category.categ_num < categ1) {
				catalogue.cat_num = 
					category.categ_num * 100000;
				flg_reset(CATALOGUE);
				continue;
			}
		}
		if(byitemgrp[0] == YES) {
		   	if(catalogue.cat_num / 1000 < itemgrp1) {
				continue;
			}
		   	if(catalogue.cat_num / 1000 > itemgrp2) {
				break;
			}

			item_grp.itmgrp_num = catalogue.cat_num / 1000;
			retval = get_itemgrp(&item_grp,BROWSE,0,e_mesg);
			if(retval < 0) {
				break;
			}
			if(item_grp.itmgrp_num < itemgrp1 ||
			   item_grp.itmgrp_num > itemgrp2) {
				catalogue.cat_num = 
					item_grp.itmgrp_num * 1000;
				flg_reset(CATALOGUE);
				continue;
			}
		}
		if(bycategory[0] == YES || byitemgrp[0] == YES) {
			if(catalogue.cat_num<cat1 || catalogue.cat_num>cat2){
				continue;
			}
		}
		else {
			if(catalogue.cat_num > cat2){
				break;
			}
		}

		if(linecnt >= PGSIZE-extralines || (bycategory[0] == YES &&
		   category.categ_num != prev_categ)) {
			retval = PrintHeading();
			if(retval == EXIT) return(EXIT);
			if(retval < 0) {
				break;
			}
		  	if(bycategory[0] == YES) {
				retval = PrintCategory();
				if(retval < 0) {
					break;
				}
			}	
			if(byitemgrp[0] == YES) {
				retval = PrintItemGroup();
				if(retval < 0) {
					break;
				}
			}	
		}
		if(byitemgrp[0] == YES && prev_itemgrp != item_grp.itmgrp_num) {
			retval = PrintItemGroup();
			if(retval < 0) {
				break;
			}
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
	mkln(28,"GENERAL SUPPLIES CATALOGUE",26); 
#else
	mkln(28,"CATALOGUE DE MATERIEL GENERAL",29);
#endif
	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
	mkln(1,"ITEM #",6);
	mkln(23,"DESCRIPTION",11);
	mkln(51,"UOM",3);
	mkln(57,"UNIT PRICE",10);
	mkln(71,"SUPPLIER",8);
#else
	mkln(1,"#ARTICLE",8);
	mkln(23,"DESCRIPTION",11);
	mkln(51,"UDM",3);
	mkln(57,"PRIX UNIT",9);
	mkln(71,"FOURNISSEUR",11);
#endif
	if(prnt_line() < 0) return(ERROR);

	return(NOERROR);
}

static
PrintCategory() 
{
	char	txt_line[80];

	if(prnt_line() < 0) return(ERROR);
	mkln(10,"CATEGORY:",9);
	mkln(20,category.categ_desc,40);
	if(prnt_line() < 0) return(ERROR);
	
	if(byitemgrp[0] == NO) {
		if(prnt_line() < 0) return(ERROR);
	}

	prev_categ = category.categ_num;
	return(NOERROR);
}

static
PrintItemGroup() 
{
	char	txt_line[80];

	if(prnt_line() < 0) return(ERROR);
	mkln(10,"ITEM GR :",9);
	mkln(20,item_grp.itmgrp_desc1,40);
	if(prnt_line() < 0) return(ERROR);
	if(item_grp.itmgrp_desc2[0] != '\0') {
		mkln(20,item_grp.itmgrp_desc2,40);
		if(prnt_line() < 0) return(ERROR);
	}
	if(item_grp.itmgrp_desc3[0] != '\0') {
		mkln(20,item_grp.itmgrp_desc3,40);
		if(prnt_line() < 0) return(ERROR);
	}
	if(prnt_line() < 0) return(ERROR);
	prev_itemgrp = item_grp.itmgrp_num;
	return(NOERROR);
}

/****************************************************************************
****************************************************************************/
static
print_item()
{
	char	txt_line[80];

	tedit((char *)&catalogue.cat_num,"_____0_",txt_line,R_LONG);
	mkln(1,txt_line,7);
	mkln(10,catalogue.cat_desc,40);
	mkln(50,catalogue.cat_uom,6);
	if(catalogue.cat_unit_price[0] != 0.00) {
		tedit((char *)&catalogue.cat_unit_price[0],"__0_.____",
			txt_line,R_DOUBLE);
		mkln(58,txt_line,9);
	}
	if(catalogue.cat_awd_supp[0] != '\0') {
		mkln(70,catalogue.cat_awd_supp,10);
	}
	if(prnt_line() < 0) return(ERROR);

	return(NOERROR);
} 
/*-------------------------- END OF FILE ---------------------------------- */

