/* --------------------------------------------------------------------------
	SOURCE NAME:  histrep.c
	SYSTEM     :  BUGETARY FINANCIAL SYSTEM 
	MODULE     :  TENDERING SYSTEM 
	CREATED ON :  3 May 1992
	CREATED BY :  J. Prescott 

DESCRIPTION:
	This program prints an Award History Report. 

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

#define	PREV	'P'
#define ALL	'A'
#else
#define PRINTER 'I'
#define DISPLAY 'A'
#define FILE_IO	'D'
#define SPOOL	'B'	/* added for requisitions */
#define YES	'O'
#define NO	'N'

#define	PREV	'P'
#define ALL	'T'
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
static	char	year[2];
static	long	prev_item;

HistReport()
{
	int	i;

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

	year[0] = ' ';
	retval = GetYear( year );
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

	prev_item = 0;

	tend_hist.th_cat_num = item1;

	flg_reset( TENDHIST );

        for( ; ; ) {
		retval = get_n_tendhist(&tend_hist,BROWSE,0,FORWARD,e_mesg);
		if(retval < 0) {
			if(retval == EFL) break;
			break;
		}

		if(tend_hist.th_cat_num > item2) {
			break;
		}

		if(linecnt >= PGSIZE-3 || 
		   (linecnt >= PGSIZE-5 && year[0] == ALL)) {
			retval = PrintHeading();
			if(retval == EXIT) return(EXIT);
			if(retval < 0) {
				break;
			}
		}

		if(year[0] == ALL) {
			for(i=0;i<3;i++) {
				retval = print_item(i);
				if(retval < 0) break;
				prev_item = tend_hist.th_cat_num;
			}
		}
		else if(year[0] == PREV) {
				retval = print_item(0);
				if(retval < 0) break;
				prev_item = tend_hist.th_cat_num;
		}
		else {
				retval = print_item(year-1);
				if(retval < 0) break;
				prev_item = tend_hist.th_cat_num;
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
	mkln(34,"AWARD HISTORY",13); 
#else
	mkln(34,"HISTORIQUE DES ATTRIBUTIONS",27);
#endif
	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
	mkln(7,"YEAR",4);
	mkln(18,"SUPPLIER",8);
	mkln(34,"QTY",3);
	mkln(44,"UNIT PRICE",10);
	mkln(59,"MIN $ REQUIRED",14);
#else
#endif
	if(prnt_line() < 0) return(ERROR);

	return(NOERROR);
}

/****************************************************************************
   This routine prints two suppliers with full address information.
****************************************************************************/
static
print_item(mode)
int	mode;
{
	char	txt_line[80];
	int	retval;

	if(prev_item != tend_hist.th_cat_num) {
		catalogue.cat_num = tend_hist.th_cat_num;
		retval = get_catalogue(&catalogue,BROWSE,0,e_mesg);
		if(retval < 0 && retval != UNDEF) {
			return(retval);
		}	

		if(prnt_line() < 0) return(ERROR);
		mkln(1,"ITEM #:",7);
		tedit((char *)&tend_hist.th_cat_num,"_____0_",txt_line,R_LONG);
		mkln(9,txt_line,7);
		if(retval != UNDEF) {
			mkln(18,catalogue.cat_desc,40);
		}
		if(prnt_line() < 0) return(ERROR);
		if(prnt_line() < 0) return(ERROR);
	}

	switch(mode) {
		case 0:
			mkln(5,"PREV YR",7);
		case 1:
			mkln(5,"PREV YR2",8);
		case 2:
			mkln(5,"PREV YR3",8);
	}

	mkln(17,tend_hist.th_supp[mode],10);

	tedit((char *)&tend_hist.th_qty[mode],"__0_.____",txt_line,R_DOUBLE);
	mkln(31,txt_line,9);

	tedit((char *)&tend_hist.th_price[mode],"__0_.____",txt_line,R_DOUBLE);
	mkln(45,txt_line,9);

	tedit((char *)&tend_hist.th_min_dollar[mode],"__0_.__",txt_line,R_DOUBLE);
	mkln(62,txt_line,7);
	if(prnt_line() < 0) return(ERROR);

	return(NOERROR);
} 
/*-------------------------- END OF FILE ---------------------------------- */

