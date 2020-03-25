/* --------------------------------------------------------------------------
	SOURCE NAME:  prnttend.c
	SYSTEM     :  BUGETARY FINANCIAL SYSTEM 
	MODULE     :  TENDERING SYSTEM 
	CREATED ON :  20 April 1992
	CREATED BY :  J. Prescott 

DESCRIPTION:
	This program prints a Tenders . 

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

#define SCHL_OPT 'S'
#else
#define PRINTER 'I'
#define DISPLAY 'A'
#define FILE_IO	'D'
#define SPOOL	'B'	/* added for requisitions */
#define YES	'O'
#define NO	'N'

#define SCHL_OPT 'E'
#endif

Pa_rec   pa_rec;             /*  Declarations for DBH record reading */
Category category;
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
static	short	tenderno1;
static	short	tenderno2;
static	short	itemgrp1;
static	short	itemgrp2;
static	char	delivery[2];

PrintTender()
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

	retval = GetTenderRange( &tenderno1, &tenderno2 );
	if(retval < 0) return(retval);

	retval = GetItemGrpRange( &itemgrp1, &itemgrp2 );
	if(retval < 0) return(retval);

	delivery[0] = ' ';
	retval = GetDelivery( delivery );
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
	if(resp[0] == DISPLAY) {
		PGSIZE = 22;
	}
	else {
		PGSIZE = 60;
	}

	linecnt = PGSIZE;
	pgcnt = 0 ;

	category.categ_num = tenderno1;

	flg_reset( CATEGORY );

        for( ; ; ) {
		retval = get_n_category(&category,BROWSE,0,FORWARD,e_mesg);
		if(retval < 0) {
			if(retval == EFL) break;
			fomer(e_mesg);
			break;
		}

		if(category.categ_num > tenderno2) {
			break;
		}

		retval = PrintHeading();
		if(retval == EXIT) 	return(EXIT);
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

			if(item_grp.itmgrp_num < itemgrp1) {
				continue;
			}
			if(item_grp.itmgrp_num > itemgrp2) {
				break;
			}

			if(linecnt >= PGSIZE-6) {
				retval = PrintHeading();
				if(retval == EXIT) 	return(EXIT);
				if(retval < 0) {
					break;
				}
			}

			if(PrintSubHeading() < 0) return(ERROR);

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

				if(linecnt >= PGSIZE-2) {
					retval = PrintHeading();
					if(retval == EXIT) 	break;
					if(retval < 0) {
						break;
					}
					if(PrintSubHeading() < 0) return(ERROR);
				}

				retval = print_line();
				if(retval < 0) {
					break;
				}
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
	mkln(35,"TENDER #",8); 
#else
	mkln(35,"SOUMISSION",10);
#endif
	tedit((char *)&category.categ_num,"0_",txt_line,R_SHORT);
	mkln(43,txt_line,2);

	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);
#ifdef ENGLISH
	mkln(11,"NOTE: DELIVERY WILL BE TO THE DISTRICT'S",40);
#else
	mkln(11,"NOTE: LA LIVRAISON SERA FAITE AUX CONSEILS SCOLAIRES",52);
#endif
	if(delivery[0] == SCHL_OPT) {
#ifdef ENGLISH
		mkln(52,"SCHOOLS",7);
#else
		mkln(52,"ECOLES",6);
#endif
	}
	else {
#ifdef ENGLISH
		mkln(52,"WAREHOUSE",9);
#else
		mkln(52,"ENTREPOT",8);
#endif
	}
	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
	mkln(1,"ITEM #",6);
	mkln(25,"DESCRIPTION",11);
	mkln(56,"UOM",3);
	mkln(64,"QTY",3);
	mkln(70,"COST/UNIT",9);
#else
	mkln(1,"#ARTICLE",8);
	mkln(25,"DESCRIPTION",11);
	mkln(56,"UDM",3);
	mkln(64,"QTE",3);
	mkln(70,"COUT/UNITE",10);
#endif
	if(prnt_line() < 0) return(ERROR);
	
#ifdef ENGLISH
	mkln(23,"NEW DESCRIPTION",15);
	mkln(53,"QTY/CASE",8);
#else
	mkln(23,"NOUVELLE DESCRIPTION",20);
	mkln(53,"QUANT/CAISSE",12);
#endif
	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
	mkln(10,"CATEGORY:",9);
#else
	mkln(10,"CATEGORIE:",10);
#endif
	mkln(20,category.categ_desc,40);
	if(prnt_line() < 0) return(ERROR);

	return(NOERROR);
}

static
PrintSubHeading() 
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
	if(prnt_line() < 0) return(ERROR);
	
	return(NOERROR);
}

/****************************************************************************
   This routine prints two suppliers with full address information.
****************************************************************************/
static
print_line()
{
	char	txt_line[80];
	long	qty;

	tedit((char *)&catalogue.cat_num,"_____0_",txt_line,R_LONG);
	mkln(1,txt_line,7);
	mkln(10,catalogue.cat_desc,40);
	mkln(54,catalogue.cat_uom,6);
	qty = catalogue.cat_qty[1] + .5;
	tedit((char *)&qty,"__0_",txt_line,R_LONG);
	mkln(63,txt_line,4);
	mkln(71,"________",8);
	if(prnt_line() < 0) return(ERROR);
	mkln(10,"_____________________________________________",40);
	mkln(54,"______",6);
	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);

	return(NOERROR);
} 
/*-------------------------- END OF FILE ---------------------------------- */

