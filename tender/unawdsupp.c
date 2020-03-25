/* --------------------------------------------------------------------------
	SOURCE NAME:  unawdsupp.c
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
#define AWARD	'A'

#define AWARD_STAT	"AWD"
#else
#define PRINTER 'I'
#define DISPLAY 'A'
#define FILE_IO	'D'
#define SPOOL	'B'	/* added for requisitions */
#define YES	'O'
#define NO	'N'
#define AWARD	'A'

#define AWARD_STAT	"ATT"
#endif

Pa_rec   pa_rec;             /*  Declarations for DBH record reading */
Bid	 bid;
Supplier supplier;
Category category;

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
static	short	tender1;
static	short	tender2;
static	char	awd_unawd[2];
static	int	first;
static  short	prev_tender;
static	char	prev_supp[11];
static	int	awd_cnt;
static	int	new_supp;

AwdUnawdSupp()
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

	retval = GetTenderRange( &tender1, &tender2 );
	if(retval < 0) return(retval);

	awd_unawd[0] = '\0';
	retval = GetAwdUnawd(awd_unawd);
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
	new_supp = 0;
	prev_tender = 0;

	category.categ_num = tender1;

	flg_reset( CATEGORY );

	for( ; ; ) {
		retval = get_n_category(&category,BROWSE,0,FORWARD,e_mesg);
		if(retval < 0) {
			if(retval == EFL) break;
			fomer(e_mesg);
			break;
		}

		if(category.categ_num > tender2) {
			break;
		}

		awd_cnt = 0;
		bid.bid_tend_num = category.categ_num;
		bid.bid_supp_cd[0] = '\0';
		bid.bid_cat_num = 0;
		flg_reset( BID );

       		for( ; ; ) {
			retval = get_n_bid(&bid,BROWSE,1,FORWARD,e_mesg);
			if(retval < 0) {
				if(retval == EFL) break;
				fomer(e_mesg);
				break;
			}

			if(bid.bid_tend_num != category.categ_num) {
				break;
			}
			if(new_supp == 0) {
				strcpy(prev_supp,bid.bid_supp_cd);
				new_supp = 1;
			}

			if(strcmp(prev_supp,bid.bid_supp_cd) !=0 ) { 
			  if((awd_unawd[0] == AWARD && awd_cnt != 0) ||
			    (awd_unawd[0] != AWARD && awd_cnt == 0)) {
				if(linecnt >= PGSIZE) {
					retval = PrintHeading();
					if(retval == EXIT)  return(EXIT);
					if(retval < 0) {
						break;
					}
				}
		
				retval = print_item();
				if(retval < 0) break;
				prev_tender = bid.bid_tend_num;
			  }
			  strcpy(prev_supp,bid.bid_supp_cd);
			  awd_cnt=0;
			}

			if(strcmp(bid.bid_status,AWARD_STAT)==0) {
				awd_cnt++;
			}
		}
		if((awd_unawd[0] == AWARD && awd_cnt != 0) ||
		  (awd_unawd[0] != AWARD && awd_cnt == 0)) {
			if(linecnt >= PGSIZE) {
				retval = PrintHeading();
				if(retval == EXIT)  return(EXIT);
				if(retval < 0) {
					break;
				}
			}
		
			retval = print_item();
			if(retval < 0) break;
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
	
	if(awd_unawd[0] == AWARD) {
#ifdef ENGLISH
		mkln(32,"AWARDED SUPPLIERS",17); 
#else
		mkln(32,"FOURNISSEURS ATTRIBUES",22);
#endif
	}
	else {
#ifdef ENGLISH
		mkln(31,"UNAWARDED SUPPLIERS",19); 
#else
		mkln(31,"FOURNISSEURS NON-ATTRIBUES",26);
#endif
	}
	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
	mkln(1,"TENDER #",8);
	mkln(12,"SUPPLIER CODE",13);
	mkln(32,"SUPPLIER NAME",13);
#else
	mkln(1,"#SOUMIS",7);
	mkln(12,"CD FOURN",8);
	mkln(32,"NOM FOURN",9);
#endif
	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);

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

	strcpy(supplier.s_supp_cd,prev_supp);
	retval = get_supplier(&supplier,BROWSE,0,e_mesg);
	if(retval < 0) {
		return(retval);
	}

	if(prev_tender != category.categ_num) {
		tedit((char *)&category.categ_num,"0_",txt_line,R_SHORT);
		mkln(4,txt_line,2);
	}
	mkln(13,supplier.s_supp_cd,10);
	mkln(28,supplier.s_name,40);
	if(prnt_line() < 0) return(ERROR);

	return(NOERROR);
} 
/*-------------------------- END OF FILE ---------------------------------- */

