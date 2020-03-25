/* --------------------------------------------------------------------------
	SOURCE NAME:  pricevarhist.c
	SYSTEM     :  BUGETARY FINANCIAL SYSTEM 
	MODULE     :  TENDERING SYSTEM 
	CREATED ON :  1 May 1992
	CREATED BY :  J. Prescott 

DESCRIPTION:
	This program prints a Price Variation Report. 

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
Catalogue catalogue;
Tend_Hist tend_hist;

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
static	char	yend_calc[2];
static	double	prev_amount;
static	double	curr_amount;
static	double	prev_qty;
static	double	curr_qty;
static	int	no_hist;

PriceVarHist()
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

	retval = GetItemNoRange( &item1, &item2 );
	if(retval < 0) return(retval);

	yend_calc[0] = ' ';
	retval = GetYendCalc( yend_calc );
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

	LNSZ = 132;

	if(outcntl == 0) {
		PGSIZE = 22;
	}
	else {
		PGSIZE = 60;
	}

	linecnt = PGSIZE;
	pgcnt = 0 ;

	catalogue.cat_num = item1;

	flg_reset( CATALOGUE );

        for( ; ; ) {
		retval = get_n_catalogue(&catalogue,BROWSE,0,FORWARD,e_mesg);
		if(retval < 0) {
			if(retval == EFL) break;
			break;
		}

		if(catalogue.cat_num > item2) {
			break;
		}

		no_hist = 0;
		tend_hist.th_cat_num = catalogue.cat_num;
		retval = get_tendhist(&tend_hist,BROWSE,0,e_mesg);
		if(retval < 0 && retval != UNDEF) {
			break;
		}
		if(retval == UNDEF) {
			no_hist = 1;
		}

		if(linecnt >= PGSIZE-4) {
			retval = PrintHeading();
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
        mkln(101,"DATE:",5);
#else
        mkln(101,"DATE:",5);
#endif
	tedit((char *)&longdate,"____/__/__",txt_line,R_LONG);
	mkln(107,txt_line,10);
	pgcnt++;
#ifdef ENGLISH
	mkln(121,"PAGE:",5);
#else
	mkln(121,"PAGE:",5);
#endif
	tedit((char *)&pgcnt,"_0_",txt_line,R_INT);
	mkln(127,txt_line,3);
	if(prnt_line() < 0) return(ERROR);

	i = strlen(pa_rec.pa_co_name);
	mkln(((LNSZ-1-i)/2)+1,pa_rec.pa_co_name,sizeof(pa_rec.pa_co_name)); 
	if(prnt_line() < 0) return(ERROR);
	
#ifdef ENGLISH
	mkln(55,"PRICE VARIATION HISTORY",23); 
#else
	mkln(55,"HISTORIQUE DE VARIATION DES PRIX",32);
#endif
	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);

	if(yend_calc[0] == YES) {
#ifdef ENGLISH
		mkln(6,"-----THREE YEARS AGO------",27);
		mkln(36,"-------TWO YEARS AGO------",27);
		mkln(66,"-------PREVIOUS YEAR------",27);
		mkln(96,"-------CURRENT YEAR-------",27);
#else
#endif
		if(prnt_line() < 0) return(ERROR);
#ifdef ENGLISH
		mkln(6,"SUPPLIER",8);
		mkln(19,"QTY",3);
		mkln(26,"AMOUNT",6);
		mkln(36,"SUPPLIER",8);
		mkln(49,"QTY",3);
		mkln(56,"AMOUNT",6);
		mkln(66,"SUPPLIER",8);
		mkln(79,"QTY",3);
		mkln(86,"AMOUNT",6);
		mkln(96,"SUPPLIER",8);
		mkln(110,"QTY",3);
		mkln(116,"AMOUNT",6);
		mkln(127,"% DIFF",6);
#else
#endif
		if(prnt_line() < 0) return(ERROR);
	}
	else {
#ifdef ENGLISH
		mkln(8,"----THREE YEARS AGO---",22);
		mkln(33,"-----TWO YEARS AGO----",22);
		mkln(58,"-----PREVIOUS YEAR----",22);
		mkln(83,"-----CURRENT YEAR-----",22);
#else
#endif
		if(prnt_line() < 0) return(ERROR);
#ifdef ENGLISH
		mkln(8,"SUPPLIER",8);
		mkln(20,"UNIT PRICE",10);
		mkln(33,"SUPPLIER",8);
		mkln(45,"UNIT PRICE",10);
		mkln(58,"SUPPLIER",8);
		mkln(70,"UNIT PRICE",10);
		mkln(83,"SUPPLIER",8);
		mkln(95,"UNIT PRICE",10);
		mkln(109,"% DIFF",6);
#else
#endif
		if(prnt_line() < 0) return(ERROR);
	}

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
	int	i;
	double	diff;
	double	amount1;
	double	amount2;
	long	qty1;
	long	qty2;

	if(prnt_line() < 0) return(ERROR);
	mkln(1,"ITEM #:",7);
	tedit((char *)&catalogue.cat_num,"_____0_",txt_line,R_LONG);
	mkln(9,txt_line,7);
	mkln(18,catalogue.cat_desc,40);
	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);
	
	if(yend_calc[0] == YES) {
		for(i=2;i>=0;i--) {
			if(no_hist == 0) {
				mkln(6+(30*(2-i)),tend_hist.th_supp[i],10);
			}
			/* round off to whole number */
			if(no_hist == 0) {
				qty1 = tend_hist.th_qty[i] + .5;
			}
			else {
				qty1 = 0;
			}
			tedit((char *)&qty1,"___0_",txt_line,R_LONG);
			mkln(18+(30*(2-i)),txt_line,5);

			if(no_hist == 0) {
				amount1 = tend_hist.th_price[i] * qty1;
			}
			else {
				amount1 = 0.00;
			}
			tedit((char *)&amount1,"__0_.__",txt_line,R_DOUBLE);
			mkln(25+(30*(2-i)),txt_line,7);
		}
		mkln(96,catalogue.cat_awd_supp,10);

		/* round off to whole number */
		qty2 = catalogue.cat_qty[0] + .5;
		tedit((char *)&qty2,"___0_",txt_line,R_LONG);
		mkln(108,txt_line,5);

		amount2 = catalogue.cat_unit_price[0] * qty2;
		tedit((char *)&amount2,"__0_.__",txt_line,R_DOUBLE);
		mkln(115,txt_line,7);

		if(no_hist==0) {
			diff = ((catalogue.cat_unit_price[0] - 
			 tend_hist.th_price[0]) / tend_hist.th_price[0]) * 100;
		}
		else {
			diff = 0;
		}
		tedit((char *)&diff,"_0_.__-",txt_line,R_DOUBLE);
		mkln(125,txt_line,7);
	}
	else {
		for(i=2;i>=0;i--) {
			if(no_hist == 0) {
				mkln(8+(25*(2-i)),tend_hist.th_supp[0],10);
			}

			if(no_hist == 1) {
				tend_hist.th_price[0] = 0.00;
			}
			tedit((char *)&tend_hist.th_price[0],"__0_.____",txt_line,R_DOUBLE);
			mkln(21+(25*(2-i)),txt_line,9);
		}

		mkln(83,catalogue.cat_awd_supp,10);

		/* round off to whole number */
		tedit((char *)&catalogue.cat_unit_price[0],"__0_.____",txt_line,R_DOUBLE);
		mkln(96,txt_line,9);

		if(no_hist == 0) {
			diff = ((catalogue.cat_unit_price[0] - 
			 tend_hist.th_price[0]) / tend_hist.th_price[0]) * 100;
		}
		else {
			diff = 0.00;
		}
		tedit((char *)&diff,"_0_.__-",txt_line,R_DOUBLE);
		mkln(109,txt_line,7);
	}
	if(prnt_line() < 0) return(ERROR);

	return(NOERROR);
} 
/*-------------------------- END OF FILE ---------------------------------- */

