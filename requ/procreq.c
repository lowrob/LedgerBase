/* --------------------------------------------------------------------------
	SOURCE NAME:  procreq.c
	SYSTEM     :  BUGETARY FINANCIAL SYSTEM 
	MODULE     :  REQUISITIONING SYSTEM 
	CREATED ON :  16 April 1991
	CREATED BY :  J. Prescott 

DESCRIPTION:
	This program prints a list of the processed requisitions. 

MODIFICATIONS:

PROGRAMMER           YY/MM/DD        DESCRIPTION OF MODIFICATIONS
^^^^^^^^^^           ^^^^^^^^        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
L.Robichaud	     93/09/21		Make report check cost centers so 
				schools can only see information on their own
				cost center. Only the district can do report
				on any cost center.

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
#define FIRST_ITEM 	2	/* first item of new po */

#define DIRECT_PO	0
#define NON_BULK_PO	1
#define STOCK_ITEM	2

Pa_rec   pa_rec;             /*  Declarations for DBH record reading */
Req_hdr	 req_hdr;
Req_item req_item;

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

static	double	po_total;
static	double	grand_total;
static	long	prev_po;
static	long	prev_req;

static	Tax_cal tax_cal;

processedreq()
{
	int	suppress;
	int	po_type;

	retval = get_param(&pa_rec,BROWSE,1,e_mesg);
        if(retval < 1) {
		fomer(e_mesg);
		close_dbh();
		return(-1);
	}

	LNSZ = 132;
	PGSIZE = 60;

	STRCPY(program,"PROCREQ");

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
			STRCPY(e_mesg,"procreq.dat");
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


#ifdef ENGLISH
	retval = DisplayMessage("Only print processed requisitions since last report (Y/N)?");
#else
	retval = DisplayMessage("Seulement les requisitions traitees depuis le dernier rapport (O/N)?");
#endif
	if(retval < 0) {
		return(retval);
	}

	retval = GetResponse(process_only);
	if(retval < 0) {
		return(retval);
	}

	mode = UPDATE; 	/* set mode to update for requistion header */

	if(process_only[0] == NO) {
		mode == BROWSE;

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
	}
	else {
		fund1 = 0;
		reqno1 = 0;
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
	}

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

	po_total = 0.00;
	grand_total = 0.00;

	req_hdr.funds = fund1;
	req_hdr.code = reqno1;

	flg_reset( REQHDR );

        for( ; ; ) {
		retval = get_n_reqhdr(&req_hdr,mode,1,FORWARD,e_mesg);
		if(retval < 0) {
			if(retval == EFL) break;
			fomer(e_mesg);
			break;
		}

		if(req_hdr.status[0] != PROCESSED) {
			if(process_only[0] != NO)
				roll_back(e_mesg);
			continue;
		}

		if(process_only[0] == NO) {
			if(req_hdr.funds > fund2) {
				break;
			}

			if(req_hdr.code < reqno1)
				continue;
			if(req_hdr.code > reqno2) 
				break;

			if(req_hdr.costcenter < ccno1 || 
			   req_hdr.costcenter > ccno2) {
				continue;
			}

			if(req_hdr.date < date1 || req_hdr.date > date2) {
				continue;
			}
		}
		else {
			if(req_hdr.print_rpt[0] == YES) {
				roll_back(e_mesg);
				continue;
			}
			if(req_hdr.costcenter < ccno1 || 
			   req_hdr.costcenter > ccno2) {
				continue;
			}
		}

		req_item.code = req_hdr.code;
		req_item.st_code[0] = '\0';
		req_item.appstat[0] = '\0';
		req_item.item_no = 0;

		flg_reset(REQITEM);

		for( ; ; ) {
			retval=get_n_reqitem(&req_item,BROWSE,3,FORWARD,e_mesg);
			if(retval < 0) {
				if(retval == EFL) break;
				fomer(e_mesg);
				break;
			}
			
			if(req_item.code != req_hdr.code) {
				break;
			}

			if(req_item.appstat[0] == DISAPPROVED) {
				continue;
			}

			if(req_item.pocode != prev_po) {
			  if(pgcnt != 0) {
			    if(po_type == STOCK_ITEM) {
			      retval=print_totals("TOTAL FROM STOCK:",po_total);
			      po_total = 0.00;
		  	    }
			    else {
			      retval=print_totals("TOTAL PO:",po_total);
			      po_total = 0.00;
			    }
			  }
			  suppress = NO_SUPPRESS;
			  prev_po = req_item.pocode;
			}
			else {
				suppress = SUPPRESS;
			}

			if(linecnt >= PGSIZE) {
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
				suppress = FIRST_ITEM;
			}

			if(req_item.code != prev_req) {
				suppress = FIRST_ITEM;
				prev_req = req_item.code;
			}

			if(req_item.st_code[0] == '\0') {
				po_type = DIRECT_PO;
			}
			else {
				if(req_item.appstat[0] == APPROVED) {
					po_type = NON_BULK_PO;
				}
				else {
					po_type = STOCK_ITEM;
				}
			}

			retval = print_line(po_type,suppress);
			if(retval < 0) {
				break;
			}
		}
		if(retval == EXIT)
			break;
		if(retval < 0 && retval != EFL) {
			roll_back(e_mesg);
			break;
		}
		if(process_only[0] == YES) {
			req_hdr.print_rpt[0] = YES;
			retval = put_reqhdr(&req_hdr,UPDATE,e_mesg);
			if(retval < 0) {
				break;
			}
			retval = commit(e_mesg);
			if(retval < 0) {
				break;
			}

			req_hdr.code++;
		}		
	}
	if(pgcnt != 0) {
		if(po_type == STOCK_ITEM) {
			retval=print_totals("TOTAL FROM STOCK:",po_total);
			po_total = 0.00;
		}
		else {
			retval=print_totals("TOTAL PO:",po_total);
			po_total = 0.00;
		}
		retval=print_totals("GRAND TOTAL:",grand_total);
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
	mkln(((LNSZ - 1 - i) / 2)+1,pa_rec.pa_co_name, sizeof(pa_rec.pa_co_name));
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
	mkln(51,"LIST OF PROCESSED REQUISITIONS",30);
#else
	mkln(51,"LISTE DES REQUISITIONS TRAITEES",31);
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

	if(process_only[0] == NO) {
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
	}

#ifdef ENGLISH
	mkln(3,"REQ",3);
	mkln(14,"SUPPLIER",8);
	mkln(30,"PO",2);
	mkln(39,"ORDER DATE",10);
	mkln(54,"APPROVED",8);
	mkln(67,"CC#",3);
 	mkln(73,"STOCK CODE",10);
	mkln(94,"DESCRIPTION",11);
	mkln(119,"AMOUNT",6);
#else
	mkln(3,"#REQ",4);
	mkln(14,"CODE DE",7);
	mkln(29,"#BC",3);
	mkln(40,"DATE DE",7);
	mkln(55,"DATE",4);
	mkln(67,"#CC",3);
 	mkln(74,"CODE DE",7);
	mkln(94,"DESCRIPTION",11);
	mkln(118,"MONTANT",7);
#endif
	if(prnt_line() < 0) return(ERROR);
	
#ifdef ENGLISH
	mkln(2,"NUMBER",6);
	mkln(16,"CODE",4);
	mkln(28,"NUMBER",6);
	mkln(56,"DATE",4);
#else
	mkln(15,"FOURN",5);
	mkln(40,"COMMANDE",8);
	mkln(53,"APPROUVEE",9);
 	mkln(75,"STOCK",5);
#endif
	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);

	return(NOERROR);
}
/****************************************************************************
   This routine prints two suppliers with full address information.
****************************************************************************/
static
print_line(po_type,suppress)
int	po_type;
int	suppress;
{
	char	txt_line[80];

	if(suppress == FIRST_ITEM) {
		tedit((char *)&req_hdr.code,"______0_",txt_line,R_LONG);
		mkln(1,txt_line,8);
		mkln(13,req_hdr.supp_cd,10);
		tedit((char *)&req_item.pocode,"______0_",txt_line,R_LONG);
		mkln(27,txt_line,8);
	}
	else if(po_type == DIRECT_PO && suppress == NO_SUPPRESS) {
		tedit((char *)&req_hdr.code,"______0_",txt_line,R_LONG);
		mkln(1,txt_line,8);
		mkln(13,req_hdr.supp_cd,10);
		tedit((char *)&req_item.pocode,"______0_",txt_line,R_LONG);
		mkln(27,txt_line,8);
	}
	else if(po_type == NON_BULK_PO && suppress == NO_SUPPRESS) {
		tedit((char *)&req_item.pocode,"______0_",txt_line,R_LONG);
		mkln(27,txt_line,8);
	}

	if(suppress == NO_SUPPRESS || suppress == FIRST_ITEM ) {
		tedit((char *)&req_hdr.date,"____/__/__",txt_line,R_LONG);
		mkln(39,txt_line,10);
		tedit((char *)&req_hdr.appdate,"____/__/__",txt_line,R_LONG);
		mkln(53,txt_line,10);
		tedit((char *)&req_hdr.costcenter,"__0_",txt_line,R_SHORT);
		mkln(67,txt_line,4);
	}

	mkln(73,req_item.st_code,10);
	mkln(87,req_item.desc,26);

	calctax(req_item.tax1,req_item.tax2,req_item.value,&tax_cal);

	req_item.value = tax_cal.gros_amt;
	tedit((char *)&req_item.value,"_____0_.__",txt_line,R_DOUBLE);
	mkln(117,txt_line,10);
	if(prnt_line() < 0) return(ERROR);

	po_total += req_item.value;
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
	mkln(94,title,strlen(title));
	
	tedit((char *)&amount,"______0_.__",txt_line,R_DOUBLE);
	mkln(116,txt_line,11);
	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);
	
	return(NOERROR);
}
/*-------------------------- END OF FILE ---------------------------------- */

