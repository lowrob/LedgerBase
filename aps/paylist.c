/******************************************************************************
		Sourcename    : paylist.c
		System        : Budgetary Financial system.
		Module        : Accounts Payable reports
		Created on    : 89-11-23
		Created  By   : Jonathan Prescott
		Cobol sources : 
*******************************************************************************
About this file:
	This routine generates one of the following :
		1. List of Payables
		2. List of Payables by Fund
	Calling file:	

MODIFICATIONS:
PROGRAMMER	DD/MM/YY	DESCRIPTION

L.Robichaud	25/11/92	Stop suppliers with a zero balance from showing
				up on liability report
	NOTE: The above change stoped the bank of Montreal from showing up so 
		it was commented out. L.R.

L.Robichaud	1993/10/13	Pass new parameter to close_rep function for 
				a banner to print with 3000 family.
******************************************************************************/
#define	EXIT	12		/* as defined in apreputl.c */
#define BYSUPP  0
#define BYFUND	1
#define PAST	0
#define CURRENT 1
#define NEXT	2

#include <stdio.h>
#include <repdef.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#ifdef ENGLISH
#define PRINTER		'P'
#define DISPLAY		'D'
#define FILE_IO		'F'

#define DUE_DATE	'D'
#define YES		'Y'
#define NO		'N'
#else
#define PRINTER		'I'
#define DISPLAY		'A'
#define FILE_IO		'D'

#define DUE_DATE	'E'
#define YES		'O'
#define NO		'N'
#endif

static short 	pgcnt;
static long	longdate ;
static long	currdate ;
static long	nextdate ;
static long	folldate ;
static char	reporton[2];
static short	fund1, fund2;
static char	suppcd1[11], suppcd2[11];
static char	invcno1[16], invcno2[16];
static char	temp_suppcd[11];
static int	retval;
static int 	offset;

static Invoice		in_rec;
static Supplier		su_rec;
static Pa_rec		pa_rec;

static char 	discfile[20] ;
static char	program[11];
static char	outdev[2];	
static short	copies;

static double	payment_amt;
static double   future_amt;
static double	grand_trans_amt , total_amt_supp;
static double	grand_Xactions , total_Xaction_supp;
static double	grand_hb , total_hb_supp;
static double	grand_pastdue , total_pastdue_supp;
static double	grand_currdue , total_currdue_supp;
static double	grand_nextdue , total_nextdue_supp;
static double	grand_futrdue , total_futrdue_supp;	

extern char	e_mesg[80] ;

PayList(key,summary,stoppmnt) 
int key;		/* key to sort report on */
char *summary;		/* if 'Y' print only total figures */
char *stoppmnt;		/* if 'S' stop payments only report */
{
	discfile[0] = '\0';

	temp_suppcd[0] = '\0';

#ifdef ENGLISH
 	STRCPY( outdev, "P" );
#else
 	STRCPY( outdev, "I" );
#endif
	retval = GetOutputon( outdev );
	if ( retval<0 || retval==EXIT )
		return( retval );

	switch (*outdev) {
		case DISPLAY :	/*  Display on Terminal */
			outdev[0] = 'D';
			STRCPY(discfile,terminal);
			break;
		case FILE_IO : 	/*  Print to a disk file */ 
			outdev[0] = 'F';
			STRCPY( e_mesg, "status.dat");
			retval = GetFilename(e_mesg);
			if( retval<0 || retval==EXIT )
			return(retval);
			STRCPY (discfile, e_mesg) ;
			break;
		case PRINTER : 	/*  Print to a printer */ 
		default  :
			outdev[0] = 'P';
			discfile[0] = '\0';
			break;
	}
	copies = 1;
		/* Do not change must be 'P' to open printer */
	if(*outdev == 'P') {
		if((retval = GetNbrCopies( &copies )) < 0)
			return(retval);
	}

	STRCPY(program, "PAYLIST") ;

	longdate = get_date();

#ifdef ENGLISH
	if((retval = DisplayMessage("Report on Due/Trans date (D/T)?"))<0)
		return(retval);
#else
	if((retval = DisplayMessage("Rapport par echeance/date de trans (E/T)?"))<0)
		return(retval);
#endif
	retval = GetResponse( reporton );	
	if( retval<0 || retval==EXIT )
		return(retval);
	retval = GetCurrDate( &currdate );
	if( retval<0 || retval==EXIT )
		return(retval);
	retval = GetNextDate( &nextdate );
	if( retval<0 || retval==EXIT )
		return(retval);
	retval = GetFollDate( &folldate );
	if( retval<0 || retval==EXIT )
		return(retval);
	if(key == BYFUND) {
		fund1 = 1;
		fund2 = 999;
		retval = GetFundRange( &fund1, &fund2 );	
		if( retval<0 || retval==EXIT )
			return(retval);
	}
	STRCPY(suppcd1,"         1");
	STRCPY(suppcd2,"ZZZZZZZZZZ");
	retval = GetSCodeRange( suppcd1, suppcd2 );	
	if( retval<0 || retval==EXIT )
		return(retval);

	invcno1[0] = '\0';
	STRCPY(invcno2,"ZZZZZZZZZZZZZZZZ");
	retval = GetTrefnoRange( invcno1, invcno2 );
	if( retval<0 || retval==EXIT )
		return(retval);

	if ( (retval=Confirm())<= 0) 
		return(retval);

	if(opn_prnt(outdev,discfile,1,e_mesg,1) < 0) {
		return(REPORT_ERR) ;
	}

		/* do not change must be 'P' to open Printer */
	if(*outdev == 'P') 
		SetCopies( (int)copies );	/* number of copies to print */
	
	/** set page size parameters **/
	pgcnt = 0;
	LNSZ = 133;
	linecnt = PGSIZE;

	if( get_param(&pa_rec,BROWSE,1,e_mesg)<1 ){
		return(DBH_ERR);
	}
	if(key == BYFUND) {
		in_rec.in_funds = fund1;
	}
	STRCPY(in_rec.in_supp_cd,suppcd1);
	STRCPY(in_rec.in_invc_no,invcno1);	
	in_rec.in_tr_type[0] = '\0' ;
	flg_reset(APINVOICE);
	for(;;) {
		retval = get_n_invc((char *)&in_rec,BROWSE,key,FORWARD,e_mesg);
		if(retval < 0) {
			if(retval == EFL) break;
			retval = DBH_ERR;
			break;
		}
		if(key == BYSUPP) {
			if(strcmp(in_rec.in_supp_cd,suppcd2) > 0) {
				break;
			}
			if(strcmp(in_rec.in_invc_no,invcno1) < 0 ||
			   strcmp(in_rec.in_invc_no,invcno2) > 0) {
				if(strcmp(in_rec.in_invc_no,invcno2) > 0) 
					inc_str(in_rec.in_supp_cd,
					sizeof(in_rec.in_supp_cd)-1,FORWARD);
				STRCPY(in_rec.in_invc_no,invcno1);
				in_rec.in_tr_type[0] = '\0' ;
				flg_reset(APINVOICE);
				continue;
			}		
		}
		else {
			if(in_rec.in_funds > fund2) {
				break;
			}
			if(strcmp(in_rec.in_supp_cd,suppcd1) < 0 ||
			   strcmp(in_rec.in_supp_cd,suppcd2) > 0) {
				if(strcmp(in_rec.in_supp_cd,suppcd2) > 0) 
					in_rec.in_funds++;	
				STRCPY(in_rec.in_supp_cd,suppcd1);
				STRCPY(in_rec.in_invc_no,invcno1);
				in_rec.in_tr_type[0] = '\0' ;
				flg_reset(APINVOICE);
				continue;
			}
			if(strcmp(in_rec.in_invc_no,invcno1) < 0 ||
			   strcmp(in_rec.in_invc_no,invcno2) > 0 ) {
				if(strcmp(in_rec.in_invc_no,invcno2) > 0)
					inc_str(in_rec.in_supp_cd,
					sizeof(in_rec.in_supp_cd)-1,FORWARD);
				STRCPY(in_rec.in_invc_no,invcno1);
				in_rec.in_tr_type[0] = '\0' ;
				flg_reset(APINVOICE);
				continue;
			}
		}

		if(in_rec.in_pmtcode[0] == COMPLETE) 
			continue;  /* if payment complete skip record */

		if(*stoppmnt == STOPPMT && in_rec.in_pmtcode[0] != STOPPMT) 
			continue;  /* if stop payments only report */

		if(linecnt >= PGSIZE) {
			retval = print_heading(stoppmnt,summary,key);
			if(retval < 0) break;
			if(retval == EXIT) return(NOERROR);
			
/*** took out to fix problem with printing supplier totals at bottom of screen
			temp_suppcd[0] = '\0';
***/
		}
		if(strcmp(temp_suppcd,in_rec.in_supp_cd) != 0) {
			if(temp_suppcd[0] != '\0'){
		  		retval=print_supp_totals(summary,stoppmnt,key);
				if(retval < 0)
					break;
				if(retval == EXIT)
					return(NOERROR);
			}
			STRCPY(su_rec.s_supp_cd,in_rec.in_supp_cd);	
			retval = get_supplier((char *)&su_rec,BROWSE,0,e_mesg);
			if(retval == ERROR) {
				retval = DBH_ERR;
				break;
			}
			/* L.Robichaud - Don't show suppliers with 0 bal */
		/**************************
			if(su_rec.s_balance == 0.00) {
				temp_suppcd[0] = '\0';
				continue;
			}
		**************************/
			if(linecnt+6 >= PGSIZE) {
				retval=print_heading(stoppmnt,summary,key);
				if(retval < 0) return(REPORT_ERR);
				if(retval == EXIT) return(NOERROR);
				temp_suppcd[0] = '\0';
			}
			if((retval = print_supplier_line()) < 0) {
				break;
			}
			STRCPY(temp_suppcd,su_rec.s_supp_cd);
		}
		if((retval = calc_supp_totals()) < 0) {
			break;
		}
		if(*summary == NO) {
			if((retval = print_detail_line()) < 0) {
				break;
			}
		}
		/* reset payment_amt & future_amt */
		payment_amt = future_amt = 0;
	}

	retval = print_supp_totals(summary,stoppmnt,key);
	retval = print_grand_totals();

	/*** Initialize grand totals to zero ***/
	grand_trans_amt = grand_Xactions = grand_hb = 0;
	grand_pastdue = grand_currdue = 0;
	grand_nextdue = grand_futrdue = 0;	

	if(pgcnt) {
		if(term < 99)
			last_page();
#ifndef SPOOLER
		else
			rite_top();
#endif
	}
	if(retval == EFL) retval = 0; 
	if(retval == REPORT_ERR) {
#ifdef ENGLISH
		sprintf(e_mesg,"Internal report error");
#else
		sprintf(e_mesg,"Erreur du rapport interne");
#endif
	}

	close_dbh() ;
	close_rep(BANNER) ;
	return(retval);
}

print_heading(stoppmnt,summary,key)
char	*stoppmnt;
char	*summary;
int	key;
{
	char	txt_line[80];
	int	len;

	if(term < 99 && pgcnt) /* if not first page and display */
		if(next_page()<0) return(EXIT);

	if(pgcnt || term < 99) {
		if(rite_top() < 0) return(REPORT_ERR);
	}
	else linecnt = 0;
	pgcnt++;

	mkln(1,program,10);
	len = strlen(pa_rec.pa_co_name);
	mkln((LNSZ-len)/2,pa_rec.pa_co_name, len);
#ifdef ENGLISH
        mkln(115,"Date:",5);
#else
        mkln(115,"Date:",5);
#endif
	tedit((char *)&longdate,"____/__/__",txt_line,R_LONG);
	mkln(121,txt_line,10);
	if(prnt_line() < 0) return(REPORT_ERR);

	if(*stoppmnt == STOPPMT) 
#ifdef ENGLISH
		STRCPY(txt_line,"STOP PAYMENTS REPORT BY ");
	else 
		STRCPY(txt_line,"LIABILITY REPORT BY ");
#else
		STRCPY(txt_line,"RAPPORT DES ARRETS DE PAIEMENTS PAR ");
	else 
		STRCPY(txt_line,"RAPPORT DU PASSIF PAR ");
#endif

	if(key == 0) 
#ifdef ENGLISH
		strcat(txt_line,"SUPP. CODE");
	else
		strcat(txt_line,"FUND");
#else
		strcat(txt_line,"CODE DE FOURNISSEUR");
	else
		strcat(txt_line,"FOND");
#endif

	if(*summary == YES)
#ifdef ENGLISH
		strcat(txt_line," (SUMMARY)");
#else
		strcat(txt_line," (RESUME)");
#endif

	len = strlen(txt_line);
	mkln((LNSZ-len)/2,txt_line,len);
#ifdef ENGLISH
	mkln(115,"Page:",5);
#else
	mkln(115,"Page:",5);
#endif
	tedit((char *)&pgcnt,"__0_",txt_line,R_SHORT);
	mkln(127,txt_line,4);
	if(prnt_line() < 0) return(REPORT_ERR);
#ifdef ENGLISH
	mkln(1,"End Of Current Period",21);
	tedit((char *)&currdate,"____/__/__",txt_line,R_LONG);
	mkln(27,txt_line,10);
#else
	mkln(1,"Fin de la periode courante",26);
	tedit((char *)&currdate,"____/__/__",txt_line,R_LONG);
	mkln(30,txt_line,10);
#endif

#ifdef ENGLISH
	if(reporton[0] == DUE_DATE) 
		mkln(61,"BY DUE DATE",11);
	else
		mkln(60,"BY TRANS DATE",13);
#else
	if(reporton[0] == DUE_DATE) 
		mkln(60,"PAR ECHEANCE",12);
	else
		mkln(54,"PAR DATE DE TRANSACTION",23);
#endif
	if(prnt_line() < 0) return(REPORT_ERR);

#ifdef ENGLISH
	mkln(1,"End Of Next Period",18);
	tedit((char *)&nextdate,"____/__/__",txt_line,R_LONG);
	mkln(27,txt_line,10);
#else
	mkln(1,"Fin de la prochaine periode",27);
	tedit((char *)&nextdate,"____/__/__",txt_line,R_LONG);
	mkln(30,txt_line,10);
#endif
	if(prnt_line() < 0) return(REPORT_ERR);

#ifdef ENGLISH
	mkln(1,"End Of Following Period",23);
	tedit((char *)&folldate,"____/__/__",txt_line,R_LONG);
	mkln(27,txt_line,10);
#else
	mkln(1,"Fin de la periode suivante",26);
	tedit((char *)&folldate,"____/__/__",txt_line,R_LONG);
	mkln(30,txt_line,10);
#endif
	if(prnt_line() < 0) return(REPORT_ERR);
	if(prnt_line() < 0) return(REPORT_ERR);

#ifdef ENGLISH
	mkln(80,"PAST DUE",8);
	mkln(92,"CURRENTLY DUE",13);
	mkln(109,"DUE NEXT",8);
	mkln(124,"DUE IN",6);
#else
	mkln(76,"PASSE L'ECHEANCE",16);
	mkln(95,"PRESENT",7);
	mkln(109,"PROCHAINE",9);
	mkln(124,"DANS",4);
#endif
	if(prnt_line() < 0) return(REPORT_ERR);

#ifdef ENGLISH
	mkln(1,"FUND",4);
	mkln(8,"TRAN REF#",9);
	mkln(20,"PMT",3);
	mkln(24,"TYPE",4);
	mkln(29,"TRAN DATE",9);
	mkln(40,"DUE DATE",8);
	mkln(50,"GROSS AMOUNT",12);
	mkln(67,"DISC/HB",7);
#else
	mkln(1,"FOND",4);
	mkln(8,"#REF TRANS",10);
	mkln(20,"PAY",3);
	mkln(24,"GEN.",5);
	mkln(29,"DATE TRANS",10);
	mkln(40,"ECHEANCE",8);
	mkln(50,"MONTANT BRUT",12);
	mkln(67,"ESCO/PR",7);
#endif
	tedit((char *)&currdate,"____/__/__",txt_line,R_LONG);
	mkln(79,txt_line,10);
	tedit((char *)&nextdate,"____/__/__",txt_line,R_LONG);
	mkln(94,txt_line,10);
	tedit((char *)&folldate,"____/__/__",txt_line,R_LONG);
	mkln(108,txt_line,10);

#ifdef ENGLISH
	mkln(124,"FUTURE",6);
#else
	mkln(124,"L'AVENIR",8);
#endif
	if(prnt_line() < 0) return(REPORT_ERR);
	return(NOERROR);
}

calc_supp_totals()
{
 long reporton_date;

	if(reporton[0] == DUE_DATE) {
		reporton_date = in_rec.in_due_dt;	/* by due date */
	}
	else reporton_date = in_rec.in_invc_dt;		/* by trans date */

	/***
		setup payment amount position 	
	***/
	payment_amt = in_rec.in_amount - in_rec.in_disc_amt;
	if(su_rec.s_type[0] == CONTRACT) {
		future_amt = in_rec.in_disc_amt;
	}
	else	future_amt = 0.0 ;

	if(reporton_date < currdate) {
		offset = PAST * 14;
		if(su_rec.s_type[0] == ORDINARY) {
			payment_amt += in_rec.in_disc_amt;
			in_rec.in_disc_amt = 0;
		}
		total_pastdue_supp += payment_amt;
	}
	else if(reporton_date <= nextdate) {
		offset = (CURRENT * 14)+1; 
		total_currdue_supp += payment_amt;
	}
	else if(reporton_date <= folldate) {
		offset = (NEXT * 14)+1;
		total_nextdue_supp += payment_amt;
	} 
	else {
		future_amt += payment_amt;
		payment_amt = 0;
	}

	total_futrdue_supp += future_amt ; /*  payment_amt; */

	total_amt_supp += in_rec.in_amount;
	total_Xaction_supp++;
	total_hb_supp += in_rec.in_disc_amt;
	return(NOERROR);
}

print_supp_totals(summary,stoppmnt,key)
char summary[2];
char *stoppmnt;
int  key;
{
 char txt_line[80];

	su_rec.s_type[0] = ' ';
	if(total_Xaction_supp != 0) {
		/****
		if(summary[0] == NO) {
			if(print_supplier_line() <0) {
				return(REPORT_ERR);
			}
		}
		****/
		if(summary[0] == NO)
			if(prnt_line() < 0) return(REPORT_ERR);
#ifdef ENGLISH
		mkln(20,"No Of Transactions",18);
#else
		mkln(20,"Nombre de transactions",22);
#endif
		tedit((char *)&total_Xaction_supp,"__0_",txt_line,R_DOUBLE);

		/** for spacing **/
#ifdef ENGLISH
		mkln(40,txt_line,4);
#else
		mkln(44,txt_line,4);
#endif

		tedit((char *)&total_amt_supp,"_______0_.__-",txt_line,R_DOUBLE);
		mkln(49,txt_line,13);
		tedit((char *)&total_hb_supp,"_______0_.__-",txt_line,R_DOUBLE);
		mkln(63,txt_line,13);
		tedit((char *)&total_pastdue_supp,"_________._0-",txt_line,R_DOUBLE);
		mkln(77,txt_line,13);
		tedit((char *)&total_currdue_supp,"_________._0-",txt_line,R_DOUBLE);
		mkln(92,txt_line,13);
		tedit((char *)&total_nextdue_supp,"_________._0-",txt_line,R_DOUBLE);
		mkln(106,txt_line,13);
		tedit((char *)&total_futrdue_supp,"_________._0-",txt_line,R_DOUBLE);
		mkln(120,txt_line,13);
		if(prnt_line() < 0) return(REPORT_ERR);
	}
	else {
		if(print_heading(stoppmnt,summary,key) < 0) return(REPORT_ERR);
		if(retval == EXIT) return(EXIT);	
		}
	if(calc_grand_totals() < 0) {
		return(REPORT_ERR);
	}
	return(NOERROR);
	
}

calc_grand_totals()
{
	grand_trans_amt += total_amt_supp;
	grand_Xactions += total_Xaction_supp;
	grand_hb += total_hb_supp;
	grand_pastdue += total_pastdue_supp;
	grand_currdue += total_currdue_supp;
	grand_nextdue += total_nextdue_supp;
	grand_futrdue += total_futrdue_supp;	
	total_amt_supp = total_Xaction_supp = total_hb_supp = 0;
	total_pastdue_supp = total_currdue_supp = total_nextdue_supp = 0;
	total_futrdue_supp = 0;
	return(NOERROR);
}

print_supplier_line()
{
	if(prnt_line() < 0) return(REPORT_ERR);
#ifdef ENGLISH
	mkln(1,"Supplier:",9);
#else
	mkln(1,"Fournisseur:", 12);
#endif
	mkln(11,su_rec.s_supp_cd,10);
	mkln(23,su_rec.s_name,48);
	if(su_rec.s_type[0] != ' ') {
#ifdef ENGLISH
		mkln(73,"Type:",5);
#else
		mkln(73,"Genre:", 6);
#endif
		mkln(80,su_rec.s_type,1);
	}
	if(prnt_line() < 0) return(REPORT_ERR);
	if(prnt_line() < 0) return(REPORT_ERR);
	return(NOERROR);
}

print_detail_line()
{
	char	txt_line[80];

	tedit((char *)&in_rec.in_funds,"_0_",txt_line,R_SHORT);
	mkln(1,txt_line,3);
	mkln(5,in_rec.in_invc_no,15);
	if(in_rec.in_pmtcode[0] != OPEN){
		if(in_rec.in_pmtcode[0] == PARTIAL && in_rec.in_chq_no != 0)
			in_rec.in_pmtcode[0] = MANUAL;
		mkln(21,in_rec.in_pmtcode,1);
	}
	mkln(25,in_rec.in_tr_type,2);	
	tedit((char *)&in_rec.in_invc_dt,"____/__/__",txt_line,R_LONG);
	mkln(28,txt_line,10);
	tedit((char *)&in_rec.in_due_dt,"____/__/__",txt_line,R_LONG);
	mkln(39,txt_line,10);
	tedit((char *)&in_rec.in_amount,"______0_.__-",txt_line,R_DOUBLE);
	mkln(50,txt_line,12);
	tedit((char *)&in_rec.in_disc_amt,"______0_.__-",txt_line,R_DOUBLE);
	mkln(64,txt_line,12);
	tedit((char *)&payment_amt,"________._0-",txt_line,R_DOUBLE);
	mkln(78+offset,txt_line,12);
	tedit((char *)&future_amt,"________._0-",txt_line,R_DOUBLE);
	mkln(121,txt_line,12);
	if(prnt_line() < 0) return(REPORT_ERR);
	return(NOERROR);
}

print_grand_totals()
{
	char txt_line[80];

	if(prnt_line() < 0) return(REPORT_ERR);
	if(prnt_line() < 0) return(REPORT_ERR);
#ifdef ENGLISH
	mkln(5,"GRAND TOTALS",12);
	mkln(20,"No Of Transactions",18);
#else
	mkln(5,"SOMMES TOTALES",14);
	mkln(20,"Nombre de transactions",22);
#endif
	tedit((char *)&grand_Xactions,"__0_",txt_line,R_DOUBLE);

#ifdef ENGLISH
	mkln(40,txt_line,4);
#else
	mkln(44,txt_line,4);
#endif

	tedit((char *)&grand_trans_amt,"_______0_.__-",txt_line,R_DOUBLE);
	mkln(49,txt_line,13);
	tedit((char *)&grand_hb,"_______0_.__-",txt_line,R_DOUBLE);
	mkln(63,txt_line,13);
	tedit((char *)&grand_pastdue,"_________._0-",txt_line,R_DOUBLE);
	mkln(78,txt_line,13);
	tedit((char *)&grand_currdue,"_________._0-",txt_line,R_DOUBLE);
	mkln(92,txt_line,13);
	tedit((char *)&grand_nextdue,"_________._0-",txt_line,R_DOUBLE);
	mkln(106,txt_line,13);
	tedit((char *)&grand_futrdue,"_________._0-",txt_line,R_DOUBLE);
	mkln(120,txt_line,13);
	if(prnt_line() < 0) return(REPORT_ERR);
	return(NOERROR);
	
}

