/* --------------------------------------------------------------------------
	SOURCE NAME:  disrep.c
	SYSTEM     :  BUGETARY FINANCIAL SYSTEM 
	MODULE     :  REQUISITIONING SYSTEM 
	CREATED ON :  18 April 1991
	CREATED BY :  J. Prescott 

DESCRIPTION:
	This program prints a list of the processed requisitions. 

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
#include <requ.h>

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

#define SUPPRESS	0	/* suppress repeating items */
#define	NO_SUPPRESS	1	/* do not suppress repeating items */
#define FIRST_PAGE_ITEM 2	/* first item of new page */

Pa_rec   pa_rec;             /*  Declarations for DBH record reading */
Req_hdr	 req_hdr;
Req_item req_item;
Req_reason req_reason;

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
static 	char	process_only[2];
static	int	mode;

/* Ranges */
static	long	reqno1;
static	long	reqno2;
static	short	fund1;
static	short	fund2;
static	short	ccno1;
static	short	ccno2;
static	long	date1;
static	long	date2;

static	double	grand_total;

static	Tax_cal tax_cal;

disapprovedreq()
{
	int	suppress;
	int	mode;

	retval = get_param(&pa_rec,BROWSE,1,e_mesg);
        if(retval < 1) {
		fomer(e_mesg);
		close_dbh();
		return(-1);
	}

	LNSZ = 132;
	PGSIZE = 60;

	STRCPY(program,"DISREP");

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
			STRCPY(e_mesg,"disrep.dat");
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

	reqno1 = 0;
	reqno2 = 99999999;
	retval = GetReqRange( &reqno1, &reqno2 );
	if(retval < 0) return(retval);

	fund1 = 1;
	fund2 = 999;
	retval = GetFundRange( &fund1, &fund2 );
	if(retval < 0) return(retval);
	
	if(CC_no == pa_rec.pa_distccno) {
		ccno1 = 0;
		ccno2 = 9999;
		retval = GetCCRange( &ccno1, &ccno2 );
		if(retval < 0) return(retval);
	}
	else {
		ccno1 = CC_no;
		ccno2 = CC_no;
	}

	date1 = date2 = get_date();
	retval = GetDateRange( &date1, &date2 );
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

	linecnt = PGSIZE;
	pgcnt = 0 ;

	grand_total = 0.00;

	req_hdr.funds = fund1;
	req_hdr.code = reqno1;

	flg_reset( REQHDR );

        for( ; ; ) {
		retval = get_n_reqhdr(&req_hdr,UPDATE,1,FORWARD,e_mesg);
		if(retval < 0) {
			if(retval == EFL) break;
			fomer(e_mesg);
			break;
		}

		if(req_hdr.status[0] != DISAPPROVED) {
			roll_back(e_mesg);
			continue;
		}

		if(req_hdr.funds > fund2) {
			break;
		}

		if(req_hdr.code < reqno1 || req_hdr.code > reqno2) {
			continue;
		}

		if(req_hdr.costcenter < ccno1 || 
		   req_hdr.costcenter > ccno2) {
			continue;
		}

		if(req_hdr.date < date1 || req_hdr.date > date2) {
			continue;
		}

		req_item.code = req_hdr.code;
		req_item.item_no = 0;

		flg_reset(REQITEM);

		for( ; ; ) {
			retval=get_n_reqitem(&req_item,BROWSE,0,FORWARD,e_mesg);
			if(retval < 0) {
				if(retval == EFL) break;
				fomer(e_mesg);
				break;
			}
			
			if(req_item.code != req_hdr.code) {
				break;
			}

			if(req_item.appstat[0] != DISAPPROVED) {
				continue;
			}

			if(linecnt >= PGSIZE-3) {
				if(*resp == DISPLAY){
					if(pgcnt > 0){ /* Check for 1st page */
/* if display, show one page at a time */	retval = next_page();
						if(retval < 0){
							retval = EXIT;
							break;
						}
					}
				}
				retval = PrintHeading();
				if(retval < 0) {
					break;
				}
				suppress = FIRST_PAGE_ITEM;
			}

			if(req_hdr.status[0] == DISAPPROVED) {
				req_reason.reqr_code = req_hdr.code;
				req_reason.reqr_item_no = 0;
			}
			else {
				req_reason.reqr_code = req_hdr.code;
				req_reason.reqr_item_no = req_item.item_no; 
			}	
			retval=get_reqreason(&req_reason,BROWSE,0,e_mesg);
			if(retval < 0) {
				if(retval == EFL) break;
				fomer(e_mesg);
				break;
			}
			
			retval = print_line(suppress);
			if(retval < 0) {
				break;
			}
		}

		if(retval < 0 && retval != EFL) {
			break;
		}

		req_hdr.print_rpt[0] = YES;
		retval = put_reqhdr(&req_hdr,UPDATE,e_mesg);
		if(retval < 0) {
			break;
		}
		req_hdr.code++;
	}
	if(retval == NOERROR || retval == EFL) {
		retval = commit(e_mesg);
	}

	if(pgcnt != 0) {
		retval=print_totals("TOTAL DISAPPROVED:",grand_total);
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
	return(retval);
}
static
PrintHeading()
{
	char	txt_line[80];
	long	longdate ;
	int	i ;

	linecnt = 0;
	
	if(pgcnt) rite_top();   /* if not first page advance */

	mkln(1,PROG_NAME,10);
	i = strlen(pa_rec.pa_co_name);
	mkln(((LNSZ-1-i)/2)+1,pa_rec.pa_co_name,sizeof(pa_rec.pa_co_name)); 
	longdate = get_date();

#ifdef ENGLISH
        mkln(115,"DATE:",5);
#else
        mkln(115,"DATE:",5);
#endif
	tedit((char *)&longdate,"____/__/__",txt_line,R_LONG);
	mkln(121,txt_line,10);
	if(prnt_line() < 0) return(ERROR);
	
#ifdef ENGLISH
	mkln(50,"LIST OF DISAPPROVED REQUISITIONS",32);
#else
	mkln(48,"LISTE DES REQUISITIONS NON-APPROUVEES",37);
#endif
	pgcnt++;
#ifdef ENGLISH
	mkln(115,"PAGE:",5);
#else
	mkln(115,"PAGE:",5);
#endif
	tedit((char *)&pgcnt,"__0_",txt_line,R_INT);
	mkln(127,txt_line,4);
	if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
	mkln(51,"FROM",4);
	tedit((char *)&date1,"____/__/__",txt_line,R_LONG);
	mkln(56,txt_line,10);
	mkln(67,"TO",5);
	tedit((char *)&date2,"____/__/__",txt_line,R_LONG);
	mkln(70,txt_line,10);
#else
	mkln(53,"DE",4);
	tedit((char *)&date1,"____/__/__",txt_line,R_LONG);
	mkln(56,txt_line,10);
	mkln(67,"A",5);
	tedit((char *)&date2,"____/__/__",txt_line,R_LONG);
	mkln(69,txt_line,10);
#endif
	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
	mkln(3,"REQ",3);
	mkln(12,"CC#",3);
	mkln(17,"ORDER DATE",10);
	mkln(31,"APPROVAL",8);
 	mkln(47,"G/L ACCOUNT",11);
	mkln(64,"STOCK CODE",10);
	mkln(84,"DESCRIPTION",11);
	mkln(107,"QUANTITY",8);
	mkln(120,"AMOUNT",6);
#else
	mkln(3,"#REQ",4);
	mkln(12,"#CC",3);
	mkln(18,"DATE DE",7);
	mkln(32,"DATE",4);
 	mkln(48,"COMPTE G/L",7);
 	mkln(65,"CODE DE",7);
	mkln(84,"DESCRIPTION",11);
	mkln(107,"QUANTITE",8);
	mkln(119,"MONTANT",7);
#endif
	if(prnt_line() < 0) return(ERROR);
	
#ifdef ENGLISH
	mkln(2,"NUMBER",6);
	mkln(33,"DATE",4);
	mkln(107,"ORDERED",7);
#else
	mkln(18,"COMMANDE",8);
	mkln(30,"APPROUVEE",9);
 	mkln(66,"STOCK",5);
 	mkln(107,"COMMANDE",8);
#endif
	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);

	return(NOERROR);
}
/****************************************************************************
   This routine prints two suppliers with full address information.
****************************************************************************/
static
print_line(suppress)
int	suppress;
{
	char	txt_line[80];

	if(suppress == NO_SUPPRESS || suppress == FIRST_PAGE_ITEM) {
		tedit((char *)&req_hdr.code,"______0_",txt_line,R_LONG);
		mkln(1,txt_line,8);
		tedit((char *)&req_hdr.costcenter,"__0_",txt_line,R_SHORT);
		mkln(11,txt_line,4);
		tedit((char *)&req_hdr.date,"____/__/__",txt_line,R_LONG);
		mkln(17,txt_line,10);
		tedit((char *)&req_hdr.appdate,"____/__/__",txt_line,R_LONG);
		mkln(30,txt_line,10);
	}

	mkln(43,req_item.acct,18);
	mkln(64,req_item.st_code,10);
	mkln(77,req_item.desc,27);

	tedit((char *)&req_item.orig_qty,"___0_.____",txt_line,R_DOUBLE);
	mkln(107,txt_line,10);

	calctax(req_item.tax1,req_item.tax2,req_item.value,&tax_cal);
	req_item.value = tax_cal.gros_amt;
	tedit((char *)&req_item.value,"_____0_.__",txt_line,R_DOUBLE);
	mkln(118,txt_line,10);
	if(prnt_line() < 0) return(ERROR);
#ifdef ENGLISH
	mkln(43,"REASON:",7);
#else
	mkln(43,"RAISON:",7);
#endif
	mkln(52,req_reason.reqr_reason,70);
	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);

	grand_total += req_item.value;

	return(NOERROR);
} 

static
print_totals(title,amount) 
char	*title;
double	amount;
{
	char	txt_line[80];

	if(prnt_line() < 0) return(ERROR);
	mkln(1,title,strlen(title));
	
	tedit((char *)&amount,"______0_.__",txt_line,R_DOUBLE);
	mkln(116,txt_line,11);
	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);
	
	return(NOERROR);
}
/*-------------------------- END OF FILE ---------------------------------- */

