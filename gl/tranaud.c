/****
*	Transaction Listing Audit Report ...
*	Nicola Mckee - December 16 1993
*	Object:	To show where the transaction listing totals differs from 
*		the glmast totals.
*/

#include <stdio.h>
#include <reports.h>
#include <cfomstrc.h>
#include <isnames.h>

#define EXIT	12

#define ESCAPE	10

#ifdef ENGLISH
#define PRINTER	 'P'
#define DISPLAY	 'D'
#define FILE_IO	 'F'

#define YES	'Y'

#define	BYDATE	'D'
#else
#define PRINTER	 'I'
#define DISPLAY	 'A'
#define FILE_IO  'D'

#define YES	'O'

#define	BYDATE	'D'
#endif

extern	char	e_mesg[80] ;

static	char	chardate[11] ;
static	char	chardate2[11] ;

static	Tr_hdr	tr_hdr ;
static	Tr_item	tr_item ;
static	Ctl_rec	ctl_rec ; /**** Declarations for DBH record reading  ****/
static	Pa_rec	pa_rec ;
static	Gl_rec	gl_rec ;

static	int	code ;
static	char 	*arayptr[7] ; 	/**** Declarations for Report writer usage ****/
static	char 	projname[50] ;
static	short	copies ;
static	char 	discfile[20] ;
static	char	program[11];
static	int	retval;
static	short	pgcnt;
static	short	fund1, fund2;
static	short	period1, period2, prev_sect, prev_period;	
static	long	sysdt, date1, date2;
static	char	acct1[19],acct2[19],prev_acct[19];
static	short	reccode;
static	char  	trans_type1[2], trans_type2[2];		/* trans type range */
static	char	flag1[2], flag2[2];

static	double	amount,opnbalance,acct_total,sect_total,fund_total,period_total;
static	char	print_open[2];
static	int	i;
static	char	str[20];
static	short	outcntl;
static	char	resp[2];

char	txt[80];
char	hold_desc[30];
double	hold_ytd;
double	hold_period;
double	D_Roundoff();

tranaud() 
{
/****	
*	Accept Project code, Logical record number in the 
*	Project and format number for the logical record ..
*	Accept the output media code . Open the report ..
****/

	STRCPY(program, "TRANAUD");

#ifdef ENGLISH
	STRCPY(e_mesg, "P");
#else
	STRCPY(e_mesg, "I");
#endif
	retval = GetOutputon(e_mesg);
	if (retval < 0) return(-1);
	else	if (retval == EXIT) return(0);

	switch( *e_mesg) {
		case DISPLAY :	/*  Display on the Terminal */
			resp[0] = 'D';
			outcntl = 0;
			break;
		case FILE_IO :	/*  Print to a file */
			resp[0] = 'F';
			outcntl = 1;
			break;
		case PRINTER :	/*  Print to the printer */
		default:
			resp[0] = 'P';
			outcntl = 2;
			break;
	}
	if(outcntl == 1) {
		STRCPY(e_mesg, "tranaud.dat");
		if(GetFilename(e_mesg) < 0) return(-1);
		STRCPY(discfile, e_mesg);
	}
	else { 	if (outcntl == 0) 
			STRCPY(discfile, terminal);
		else
			discfile[0] = '\0' ;
	     }

	copies = 1;
	if(outcntl == 2) {
		if((retval == GetNbrCopies( &copies )) < 0)
			return(retval);
	}

	if( (retval = opn_prnt( resp, discfile, 1, e_mesg, 1)) <0) {
		return(REPORT_ERR);
	}

	if (resp[0] == DISPLAY) 
		PGSIZE=22;
	else
		PGSIZE=60;


	pgcnt = 0;
	LNSZ= 131;
	linecnt = PGSIZE;
	if(outcntl == 2)
		SetCopies( (int)copies );	/* number of copies to print */
							  	

	code = get_param(&pa_rec, BROWSE, 1, e_mesg);
 	if (code < 1) {
		printf("%s\n", e_mesg);
		close_dbh();
		return(-1);
	}
	sysdt = get_date();
/*
#ifdef	ENGLISH
	DisplayMessage("Print by Period (P)?");
#else
	DisplayMessage("Imprime par periode (P)?");
#endif
*/
	retval = ("P" );
	if(retval < 0)  return(retval);
	
	fund1 = 1;
 	fund2 = 9;
	retval = GetFundRange( &fund1, &fund2);
 	if (retval < 0) return(-1);
	else if (retval == EXIT) return(0);	

	if(flag2[0] == BYDATE) {
		date1 = date2 = get_date();
		retval = GetDateRange( &date1, &date2);
 		if (retval < 0) return(-1);
		else if (retval == EXIT) return(0);	
	}
	else {
		period1 = pa_rec.pa_cur_period;
		period2 = pa_rec.pa_cur_period;
		/*retval = GetPeriodRange( &period1 , &period2 );
 		if (retval < 0) return(-1);
		else if (retval == EXIT) return(0);	*/
	}

	STRCPY(acct1, "                 1");
	STRCPY(acct2, "999999999999999999");
	retval = GetAcctRange( acct1, acct2);
 	if (retval < 0) return(-1);
	else if (retval == EXIT) return(0);	

 	reccode = 99;
	retval = GetReccod( &reccode);
 	if (retval < 0) return(-1);
	else if (retval == EXIT) return(0);	

	STRCPY(trans_type1, "1");
	STRCPY(trans_type2, "Z");
	retval = GetTransRange( trans_type1, trans_type2);
 	if (retval < 0) return(-1);
	else if (retval == EXIT) return(0);	

#ifdef	ENGLISH
	/*DisplayMessage("Do you only want manually entered entries (Y/N)?");*/
	retval =  "N" ;
#else
	DisplayMessage("Desirez-vous les entrees manuelles seulement (O/N)?");
	retval = GetResponse( flag1, "ON" );
#endif
	if(retval < 0)  return(retval);

	if(( retval = Confirm()) < 0) return(-1);
	else if (!retval) return(0);

	mkdate(get_date(),chardate);

	retval = PrintRep();

	if(retval==EFL) retval=0;
	else if(retval==REPORT_ERR)
#ifdef ENGLISH
		STRCPY(e_mesg,"Internal report writing error");
#else
		STRCPY(e_mesg,"Erreur d'inscription au rapport interne");
#endif

	close_dbh();
	close_rep();
	return(NOERROR);
}

static
PrintRep()
{
	prev_period = 0 ;
	prev_acct[0] = '\0' ;
	prev_sect = 0 ;

	period_total = 0;
	acct_total = 0;
	amount = 0;
	sect_total = 0;
	fund_total = 0;

/***	Prepare to read transaction sequentialy ***/

	tr_item.ti_fund = fund1 ;
	tr_item.ti_reccod = reccode ;
	STRCPY( tr_item.ti_accno, acct1 )   ;	

	if(flag2[0] == BYDATE) 
	        tr_item.ti_period = 0 ;
	else	
	        tr_item.ti_period = period1 ;

        tr_item.ti_seq_no = 0 ;
        tr_item.ti_item_no = 0 ;
	flg_reset(GLTRAN);


	for(;;) {
		code = get_n_tritem(&tr_item, BROWSE, 1, FORWARD, e_mesg) ;
		if(code == EFL) {
			retval=0;
			break;
		}
		else if (retval<0)
			return(DBH_ERR);

		if (tr_item.ti_fund < fund1 || fund2 < tr_item.ti_fund)
			continue;
		if(tr_item.ti_reccod != reccode)
			continue;
		if ((strcmp(tr_item.ti_accno, acct1)) < 0) continue;
		if ((strcmp(acct2, tr_item.ti_accno)) < 0) continue;
		if(flag2[0] != BYDATE) { 
			if (tr_item.ti_period < period1) continue;
			if (tr_item.ti_period > period2) continue;
		}
		if(flag1[0] == YES)		/* Manual entries only  */
			if(tr_item.ti_create[0] == 'G')
				continue;

		tr_hdr.th_fund = tr_item.ti_fund;
		tr_hdr.th_reccod = tr_item.ti_reccod;
		tr_hdr.th_create[0] = tr_item.ti_create[0];
		tr_hdr.th_seq_no = tr_item.ti_seq_no ;
		code = get_trhdr(&tr_hdr, BROWSE, 0, e_mesg) ;
		if(code < 0) break ;

		if(flag2[0] == BYDATE) 
			if(tr_hdr.th_date < date1 || tr_hdr.th_date > date2)
				continue;

		if (strcmp(tr_hdr.th_type, trans_type1) < 0) continue;
		if (strcmp(trans_type2, tr_hdr.th_type) < 0) continue;

		if( (prev_period != tr_item.ti_period ) ||
	   	    (strcmp(prev_acct, tr_item.ti_accno) != 0) ) { 
			if (prev_period != 0) 
				retval = PrntPeriodTotal() ;
			prev_period = tr_item.ti_period ;
			period_total = 0.00 ;
		}

		if (strcmp(prev_acct, tr_item.ti_accno)!=0 ) {
			if (prev_acct[0] != '\0') { 
					retval = PrntClosingBalance() ;
			}
			acct_total = 0.00;
			amount = 0.00;
			hold_ytd = 0.00;
			hold_period = 0.00;

			STRCPY(prev_acct,tr_item.ti_accno);

			gl_rec.funds = tr_item.ti_fund;
			STRCPY(gl_rec.accno, tr_item.ti_accno);

			gl_rec.reccod  = tr_item.ti_reccod;

			code=get_gl(&gl_rec,BROWSE, 0 , e_mesg);
			if(code < 0) break;
			STRCPY(hold_desc,gl_rec.desc);
			hold_period=gl_rec.currel[period1-1];
			hold_ytd=D_Roundoff(gl_rec.ytd);

			/* calculate  current opening balance */
			opnbalance = gl_rec.opbal ;
			for(i = 0; i < period1-1; i++){
				opnbalance += gl_rec.currel[i];
			}
			/* setup new opening balance to print */
			amount = opnbalance ;
			print_open[0] = 'Y';

		}

		if(flag2[0] == BYDATE) {
			if (prev_sect != gl_rec.sect) {
				if (prev_sect != 0) 
					retval = PrntSectTotal() ;
				prev_sect = gl_rec.sect ;
				sect_total = 0.00 ;
			}
		}

		if(tr_item.ti_fund != ctl_rec.fund) {
			if(ctl_rec.fund != 0) {
				if(flag2[0] == BYDATE) {
						retval = PrntFundTotal() ;
				}
			}
			ctl_rec.fund = tr_item.ti_fund ;
			code = get_ctl(&ctl_rec, BROWSE, 0, e_mesg) ;
			if(code < 0) break ;
			fund_total = 0.00;
		}

		period_total += tr_item.ti_amount;
		acct_total += tr_item.ti_amount;
		amount += tr_item.ti_amount;
		sect_total += tr_item.ti_amount;
		fund_total += tr_item.ti_amount;

		if (linecnt >= PGSIZE) {
			if (pgcnt && term<99) {
				if (next_page()<0)	return(ESCAPE);	
			}
			if( pgcnt || term<99 ) {
				if( rite_top()<0 )
					return(REPORT_ERR);
			}
			else
				linecnt = 0;
			pgcnt++;
			if( (retval=PrntHdg())<0)
				return(retval);
		}
		if( (retval=PrntRec())<0)
			return(retval);

	}
	if( (prev_period != tr_item.ti_period ) ||
	    (strcmp(prev_acct, tr_item.ti_accno) != 0) ||
	    (prev_sect != gl_rec.sect) ||
	    (ctl_rec.fund != tr_item.ti_fund) ) {
		if (prev_period != 0) { 
			retval = PrntPeriodTotal() ;
			if(flag2[0] != BYDATE) 
				retval = PrntClosingBalance() ;
			else {
				retval = PrntAcctTotal() ;
				retval = PrntSectTotal() ;
				retval = PrntFundTotal() ;
			}
		}
	}

	if(resp[0] == DISPLAY) 
		PGSIZE=22;
	else
		PGSIZE=60;

	if( pgcnt ) {
		if( resp[0] == DISPLAY ) {
			retval = last_page();
		}
#ifndef SPOOLER
		else {
			rite_top();
		}
#endif
	}

	return(NOERROR);
   
}

static
PrntHdg()	/* Print heading  */
{
	short offset;

	mkln( 1, PROG_NAME, 10 );
	offset = ( LNSZ-strlen(pa_rec.pa_co_name) )/2;
	mkln( offset, pa_rec.pa_co_name, strlen(pa_rec.pa_co_name) );
#ifdef ENGLISH
	mkln( 95, "DATE: ", 6 );
#else
	mkln( 95, "DATE: ", 6 );
#endif
	tedit( (char *)&sysdt,"____/__/__",  line+cur_pos,R_LONG ); 
	cur_pos += 10;
#ifdef ENGLISH
	mkln( 113, "PAGE: ", 6 );
#else
	mkln( 113, "PAGE: ", 6 );
#endif
	tedit( (char *)&pgcnt,"__0_",  line+cur_pos, R_SHORT ); 
	cur_pos += 4;
	if( prnt_line()<0 )	return(REPORT_ERR);
	
#ifdef ENGLISH
	mkln( 53, "   TRANSACTION LISTING   ", 25 );
#else
	mkln( 53, "    LISTE DES TRANSACTIONS     ", 31 );
#endif
	if( prnt_line()<0 )	return(REPORT_ERR);
	if(flag2[0] != BYDATE)  {
#ifdef	ENGLISH
		mkln( 52, "FOR THE PERIOD FROM ", 20 );
#else
		mkln( 52, "POUR LA PERIODE DE  ", 20 ); 
#endif 
		tedit( (char *)&period1,"0_",  line+cur_pos, R_SHORT ); 
		cur_pos += 2;
#ifdef	ENGLISH
		mkln( 75, "TO ", 3 );
#else
		mkln( 75, " A ", 3 );
#endif 
		tedit( (char *)&period2,"0_",  line+cur_pos, R_SHORT ); 
		cur_pos += 2;
	}
	else {
#ifdef	ENGLISH
		mkln( 44, "FOR THE PERIOD FROM ", 20 );
#else
		mkln( 44, "POUR LA PERIODE DE  ", 20 ); 
#endif 
		tedit( (char *)&date1,"____/__/__",  line+cur_pos,R_LONG ); 
		cur_pos += 10;
#ifdef	ENGLISH
		mkln( 75, "TO ", 3 );
#else
		mkln( 75, " A ", 3 );
#endif 
		tedit( (char *)&date2,"____/__/__",  line+cur_pos,R_LONG ); 
		cur_pos += 10;
	}
	if( prnt_line()<0 )	return(REPORT_ERR);

	if( prnt_line()<0 )	return(REPORT_ERR);

#ifdef ENGLISH
	mkln(4,"ACCOUNT NO.",11);
	mkln(24,"ACCT DESC",9);
	mkln(38,"PER RC",6);
	mkln(48,"TRANS DESC",10);
	mkln(63,"TC",2);
	mkln(69,"REFERENCE",9);
	mkln(82,"SUPPLIER",8);
	mkln(92,"DATE",4);
	if(flag2[0] != BYDATE)  {
		mkln(103,"OPENING BALANCE",15);
		mkln(124,"AMOUNT",6);
	}
	else
		mkln(106,"AMOUNT",6);
#else
	mkln(3,"#DE COMPTE",10);
	mkln(19,"DESCRIPTION",11);
	mkln(24,"  PER  CF",9);
	mkln(39,"DESCRIPTION",11);
	mkln(59,"CT",2);
	mkln(65,"REFERENCE",9);
	mkln(78,"CODE FOURN",10);
	mkln(94,"DATE",4);
	if(flag2[0] != BYDATE) { 
		mkln(102,"SOLDE D'OUV",11);
		mkln(128,"MONTANT",7);
	}
	else
		mkln(110,"MONTANT",7);
#endif
	if( prnt_line()<0 )	return(REPORT_ERR); 
	if( prnt_line()<0 )	return(REPORT_ERR);
	
	return(0);
}
static
PrntRec()
{
	mkln(1,tr_item.ti_accno,18);
	mkln(21,hold_desc,15);
	mkln(37," ",1);
	tedit( (char*)&tr_item.ti_period,"0_",line+cur_pos,R_SHORT);
	cur_pos+= 2;
	mkln(41," ",1);
	tedit( (char*)&tr_hdr.th_reccod,"0_",line+cur_pos,R_SHORT);
	cur_pos+= 2;
	mkln(44," ",1);
	mkln(46,tr_hdr.th_descr,15);
	mkln(63,tr_hdr.th_type,1);
	mkln(67,tr_hdr.th_reference,15);
	mkln(81,tr_hdr.th_supp_cd,10);
	mkln(91," ",1);
	tedit( (char *)&tr_hdr.th_date,"____/__/__",line+cur_pos,R_LONG);
	cur_pos+=10;
	mkln(102," ",1);
	if(flag2[0] != BYDATE) { 
		if (print_open[0] == 'Y' ) {
			tedit( (char *)&opnbalance,"__,___,_0_.__-",line+cur_pos,R_DOUBLE);
			cur_pos+=14;
			print_open[0] = 'N' ;
		}
		else
			mkln(102,"              ",14);
		mkln(116," ",1);
	}
	tedit( (char *)&tr_item.ti_amount,"__,___,_0_.__-",line+cur_pos,R_DOUBLE);
	cur_pos+=14;
	if( prnt_line()<0 ) 	return(REPORT_ERR);
	return(NOERROR);
}
static
PrntPeriodTotal()
{
	int	count;
	if(flag2[0] == BYDATE)  
		count =-17;
	else
		count = 0;
#ifdef ENGLISH
	mkln(95+count,"TRANSACTION PERIOD ",19);
#else
	mkln(95+count,"PERIODE",7);
#endif
	tedit( (char *)&prev_period,"0_",line+cur_pos,R_SHORT);
	cur_pos+=2 ;

#ifdef ENGLISH
	mkln(105+count,"TOTAL       ",12);
#else
	mkln(105+count,"TOTAL       ",12);
#endif
	tedit( (char *)&period_total,"__,___,_0_.__-",line+cur_pos,R_DOUBLE);
	cur_pos+=14;
	if( prnt_line()<0 ) 	return(REPORT_ERR);
	if( prnt_line()<0 ) 	return(REPORT_ERR);

#ifdef ENGLISH
	mkln(80+count,"G/L MASTER PERIOD",18);
#else
	mkln(95+count,"PERIODE",7);
#endif
	tedit( (char *)&prev_period,"0_",line+cur_pos,R_SHORT);
	cur_pos+=2 ;
#ifdef ENGLISH
	mkln(105+count,"TOTAL       ",12);
#else
	mkln(105+count,"TOTAL       ",12);
#endif
	tedit( (char *)&hold_period,"__,___,_0_.__-",line+cur_pos,R_DOUBLE);
	cur_pos+=14;
	if( prnt_line()<0 ) 	return(REPORT_ERR);
	if( prnt_line()<0 ) 	return(REPORT_ERR);

	return(NOERROR);
}
static
PrntAcctTotal()
{
	int	count;
	if(flag2[0] == BYDATE)  
		count = -17;
	else
		count = 0;
#ifdef ENGLISH
	mkln(95+count,"ACCOUNT TOTAL         " ,22);
 #else
	mkln(95+count,"TOTAL COMPTE          " ,22);
#endif
	tedit( (char *)&acct_total,"__,___,_0_.__-",line+cur_pos,R_DOUBLE);
	cur_pos+=14;
	if( prnt_line()<0 ) 	return(REPORT_ERR);
	if( prnt_line()<0 ) 	return(REPORT_ERR);
	return(NOERROR);
}
static
PrntSectTotal()
{
	int	count;
	if(flag2[0] == BYDATE)  
		count = -17;
	else
		count = 0;
#ifdef ENGLISH
	mkln(95+count,"SECTION TOTAL         " ,22);
#else
	mkln(95+count,"TOTAL SECTION         " ,22);
#endif
	tedit( (char *)&sect_total,"__,___,_0_.__-",line+cur_pos,R_DOUBLE);
	cur_pos+=14;
	if( prnt_line()<0 ) 	return(REPORT_ERR);
	if( prnt_line()<0 ) 	return(REPORT_ERR);
	return(NOERROR);
}
static
PrntFundTotal()
{
	int	count;
	if(flag2[0] == BYDATE)  
		count = -17;
	else
		count = 0;
#ifdef ENGLISH
	mkln(95+count,"FUND TOTAL            " ,22);
#else
	mkln(95+count,"TOTAL FOND            " ,22);
#endif
	tedit( (char *)&fund_total,"__,___,_0_.__-",line+cur_pos,R_DOUBLE);
	cur_pos+=14;
	if( prnt_line()<0 ) 	return(REPORT_ERR);
	if( prnt_line()<0 ) 	return(REPORT_ERR);
	return(NOERROR);
}
static
PrntClosingBalance()
{
#ifdef ENGLISH
	mkln(80,"TRANSACTION CLOSING BALANCE ",28);
#else
	mkln(95,"SOLDE FERMETURE ",16);
#endif
	mkln(112,"     ",5);
	tedit( (char *)&amount,"__,___,_0_.__-",line+cur_pos,R_DOUBLE);
	cur_pos+=14;
	if( prnt_line()<0 ) 	return(REPORT_ERR);
	if( prnt_line()<0 ) 	return(REPORT_ERR);

#ifdef ENGLISH
	mkln(80,"G/L MASTER YTD TOTAL   ",23);
#else
	mkln(97,"YTD TOTAL   ",12);
#endif
	tedit( (char *)&hold_ytd,"__,___,_0_.__-",line+cur_pos,R_DOUBLE);
	cur_pos+=14;
	if( prnt_line()<0 ) 	return(REPORT_ERR);
	if( prnt_line()<0 ) 	return(REPORT_ERR);

	amount = D_Roundoff(amount);
	hold_ytd = D_Roundoff(hold_ytd);

	if(hold_ytd != amount){

#ifdef ENGLISH
		mkln(10," YTD balance and Transaction Totals are not equal",50);
#else
		mkln(10," YTD balance and Transaction Totals are not equal",50);
#endif
		if( prnt_line()<0 ) 	return(REPORT_ERR);
	}

	hold_ytd = 0.00;
	amount = 0.00;

	return(NOERROR);
}
