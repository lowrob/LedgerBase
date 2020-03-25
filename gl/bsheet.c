/*-----------------------------------------------------------------------
Source Name: bsheet.c
System     : Balance Sheet Report
Module     : General Ledger system.
Created  On: 19 July 89.
Created  By: Cathy Burns.


DESCRIPTION:
	Program to select options for the user to generate the
	Balance Sheet Report.

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
M. Cormier     90/11/30		Made Quit option work for Display of 
				balance sheet reports.
L.Robichaud	1993/10/13	Pass new parameter to close_rep function for 
				a banner to print with 3000 family.

------------------------------------------------------------------------*/

#include <stdio.h>
#include <repdef.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define EXIT	12 

#define	program	"bsheet"

#ifdef ENGLISH
#define PRINTER 'P'
#define DISPLAY 'D'
#define FILE_IO	'F'
#define YES	'Y'
#define NO	'N'
#else
#define PRINTER 'I'
#define DISPLAY 'A'
#define FILE_IO	'D'
#define YES	'O'
#define NO	'N'
#endif

Gl_rec	gl_rec ; 	/**** 	Declarations for DBH record reading  ****/
Ctl_rec	ctl_rec ; 	/**** 	Declarations for DBH record reading  ****/
Pa_rec	pa_rec;				/* Parameter record */

static short	pgcnt;
static char	txt_line[132];
static short	prev_sect;
static double	sect_total1;
static double	sect_total2;
static double	sect_total3;
static double	sect_total4;
static double	fund_total1;
static double	fund_total2;
static double	fund_total3;
static double	fund_total4;
static double	grand_total1;
static double	grand_total2;
static double	grand_total3;
static double	grand_total4;
static char	e_mesg[80];
static int 	i;
static short	period1, period2, fund1, fund2;
static double	sd_per1_chg, sd_per2_chg; 
static double	sd_cur1_bal, sd_cur2_bal, sd_prev_bal, sd_prev_chg;

bsheet(report_no) 
int	report_no;
{

int	code ;
char	resp[2];
short	copies ;
char 	discfile[20] ;
char	summary[2];
int	retval;
int	err;
char	tmp[2];


	err = get_param(&pa_rec,BROWSE,1,e_mesg);
	if (err == ERROR)
		fomen(e_mesg);
	else if (err == UNDEF)
#ifdef ENGLISH
		fomen("Parameters are no Setup ...");
#else
		fomen("Parametres ne sont pas etablis... ");
#endif

#ifdef ENGLISH
	STRCPY( resp, "P");
#else
	STRCPY( resp, "I");
#endif
	retval = GetOutputon(resp);
	if (retval < 0 ) return(-1);
	else if (retval == EXIT) return(0);

        switch( *resp ) {
		case DISPLAY :	/*  Display on the terminal */
			resp[0]='D';
			STRCPY(discfile,terminal);
			break;
		case FILE_IO :	/*  Write to a file */
			resp[0]='F';
			STRCPY(discfile, "bsheet.dat");
			if(GetFilename( discfile ) < 0) return(-1);
			break;
		case PRINTER :	/*  Print to a printer */
		default:
			resp[0]='P';
			discfile[0] = '\0' ;
			break;
	}

	copies = 1;
	if(*resp == 'P') {
		if((retval == GetNbrCopies( &copies )) < 0)
			return(retval);
	}

	fund1 = 1;
	fund2 = 999;
	retval = GetFundRange( &fund1, &fund2);
	if (retval < 0) return(-1);
	else if (retval == EXIT) return(0);

	period1 = pa_rec.pa_cur_period ;

	retval = GetPeriod(&period1);
	if (retval < 0) return(-1);
	else if (retval == EXIT) return(0);

	if (report_no == 3) {
		period2 = period1 + 1 ;
		for (; ;) {
			retval = GetPeriod2(&period2);
			if (retval < 0) return(-1);
			else if (retval == EXIT) return(0);
			if ( period1 != period2 ) break ;
#ifdef ENGLISH
			fomer("Comparative periods cannot be the same") ;
#else
			fomer("Periodes comparatives ne peuvent pas etre les memes") ;
#endif
		}
	}
#ifdef ENGLISH
	DisplayMessage("Summary (Y/N)? ");
#else
	DisplayMessage("Resume (O/N)? ");
#endif
	GetResponse( summary );


	if (( retval = Confirm()) < 0) return(-1);
	else if (!retval) return(0);

	code = opn_prnt(resp,discfile,1,e_mesg,1 /* spool */);
	if ( code < 0 ){
#ifdef ENGLISH
		fomen("Problem with report"); 
#else
		fomen("Probleme avec le rapport");
#endif
		get();
		return(-1);
	}

	if(*resp == 'P')
		SetCopies( (int)copies );	/* number of copies to print */

	pgcnt = 0;
	LNSZ = 131;
	linecnt = PGSIZE;

	prev_sect = 1;


/***	Prepare to read gl_mast sequentialy ***/

	gl_rec.funds = fund1 ;
	gl_rec.sect = 0 ;
	gl_rec.reccod = 0;
	gl_rec.accno[0] = '\0';

	ctl_rec.fund = 0;

	for(;;) {
		code = get_n_gl(&gl_rec, BROWSE, 3, 0, e_mesg) ;
		if(code < 0) break ;
		
		gl_rec.budcur = 0 ;
		gl_rec.budpre = 0 ;

		if(gl_rec.funds < fund1 || fund2 < gl_rec.funds) continue;
		if(gl_rec.sect == 3 || gl_rec.sect == 4) {
			if(gl_rec.reccod != 99) continue;
			sd_per1_chg += gl_rec.currel[period1-1];
			sd_per2_chg += gl_rec.currel[period2-1];
			sd_prev_chg += gl_rec.prerel[period1-1];
			for(i=0;i<period1;i++) {
				sd_cur1_bal += gl_rec.currel[i];
				sd_prev_bal += gl_rec.prerel[i];
			}
			for(i=0;i<period2;i++) {
				sd_cur2_bal += gl_rec.currel[i];
			}
			continue;
		}

		if(gl_rec.reccod != 99) {
			if(gl_rec.reccod >= 40 && gl_rec.reccod <= 52) {
				gl_rec.currel[0] = 0 ;
				gl_rec.prerel[0] = 0 ;
			}
			else
				continue;
		}
		else {
			if (report_no ==3) {  /* two periods comparison */
				for (i = 0;i < period1;i++) 
					gl_rec.budcur += gl_rec.currel[i];
				for (i = 0;i < period2;i++) 
					gl_rec.budpre += gl_rec.currel[i];
			}
			else {
				for (i = 0;i < period1;i++) {
					gl_rec.budcur += gl_rec.currel[i];
					gl_rec.budpre += gl_rec.prerel[i];
				}

			}
			gl_rec.currel[0] = gl_rec.currel[period1 - 1];
			gl_rec.prerel[0] = gl_rec.prerel[period1 - 1];
			if (report_no == 3) 
			   gl_rec.prerel[0] = gl_rec.currel[period2 - 1];
		}
		if(gl_rec.funds != ctl_rec.fund) {
			if(pgcnt) {	/* if not first page */
				if(PrintSurplus(report_no,summary)<0) 
					return(-1);
				if(PrintSectTotals(report_no)<0) return(-1) ;
				if(PrintFundTotals(report_no) < 0) return(-1);
			}
			ctl_rec.fund = gl_rec.funds ;
			code = get_ctl(&ctl_rec, BROWSE, 0, e_mesg) ;
			if(code < 0) break ;
			prev_sect = gl_rec.sect;
			fund_total1 = fund_total2 = 0.00;
			fund_total3 = fund_total4 = 0.00;
			retval = PrintHeadings(report_no);
			if(retval < 0)		return(-1);
			if(retval == EXIT)	break;	
		}

		if(strcmp(gl_rec.accno, ctl_rec.p_and_l_acnt) == 0)
			continue;

		if(linecnt+1 >= PGSIZE)  {
			retval = PrintHeadings(report_no);
			if(retval < 0)		return(-1);
			if(retval == EXIT)	break;	
		}

	  	if(gl_rec.sect != prev_sect) {
			PrintSectTotals(report_no) ;
			prev_sect = gl_rec.sect;
			retval = PrintHeadings(report_no);
			if(retval < 0)		return(-1);
			if(retval == EXIT)	break;	
		}

		PrintDetail(report_no,summary) ;
	}

	if(pgcnt) {  	/* if at least one page printed */
		if(PrintSurplus(report_no,summary)<0) return(-1);
		if(PrintSectTotals(report_no)<0) return(-1) ;
		if(PrintFundTotals(report_no) < 0) return(-1);
		if(PrintGrandTotals(report_no) < 0) return(-1);
	}
	close_dbh();
	close_rep(BANNER) ;
	if (*resp == 'D')  {
#ifdef ENGLISH
		write(1," Press RETURN to Continue ", 26);
#else
		write(1," Appuyer sur RETURN pour continuer", 34);
#endif
		read(0,tmp,2);
	}
	return(0);
}

PrintHeadings(report_no)
int	report_no;
{

	short	offset;
	long	longdate;

	if(pgcnt && term < 99)	/* new page and display */
		if(next_page()<0) return(EXIT);

	if(pgcnt || term < 99) { /* if not first page or display */
		if(rite_top()<0) return (-1);
	}
	else
		linecnt = 0;

	pgcnt++;

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
	
	switch(report_no) {
		case 1:		/* By Period */
#ifdef ENGLISH
			mkln(60,"BALANCE SHEET",13);
#else
			mkln(60,"   BILAN     ",13);
#endif
			mkln(115,"PAGE:",4);
			tedit((char *)&pgcnt,"_0_",txt_line,R_SHORT);
			mkln(121,txt_line,3);
			if(prnt_line() < 0) return(ERROR);
#ifdef ENGLISH
			sprintf(e_mesg,"For the End of Period %d", period1);
#else
			sprintf(e_mesg,"Pour la fin de la periode %d",period1);
#endif
			mkln((LNSZ-1-strlen(e_mesg))/2,e_mesg,strlen(e_mesg));
			if(prnt_line() < 0) return(ERROR);
			if(prnt_line() < 0) return(ERROR);
			
#ifdef ENGLISH
			mkln(1,"FUND",4);
#else
			mkln(1,"FOND",4);
#endif
			mkln(6,ctl_rec.desc,strlen(ctl_rec.desc));
			mkln(11+strlen(ctl_rec.desc),"SECTION",7);
			tedit((char *)&gl_rec.sect,"_0",txt_line,R_SHORT);
			mkln(20+strlen(ctl_rec.desc),txt_line,2);
			if(prnt_line() < 0) return(ERROR);
			if(prnt_line() < 0) return(ERROR);
#ifdef ENGLISH
			mkln(64,
	"- - - - - - - - - - -   CURRENT   YEAR   - - - - - - - - - - - -",
				64);
#else
			mkln(64,
	"- - - - - - - - - - -   ANNEE COURANTE   - - - - - - - - - - - -",
				64);
#endif
			if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
			mkln(20,"DESCRIPTION",11);
			mkln(80,"PERIOD",6);
			mkln(123,"NEW",3);
#else
			mkln(20,"DESCRIPTION",11);
			mkln(80,"CHANGEMENTS",11);
			mkln(121,"NOUVEAU",7);
#endif
			if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
			mkln(80,"CHANGES",7);
			mkln(121,"BALANCE",7);
#else
			mkln(80,"EN PERIODE",10);
			mkln(122,"SOLDE",5);
#endif
			if(prnt_line() < 0) return(ERROR);

			break;
		case 2:		/* Yearly comparision */
#ifdef ENGLISH
			mkln(50,"YEARLY COMPARATIVE BALANCE SHEET",32);
#else
			mkln(50,"   BILAN COMPARATIF D'ANNEES    ",32);
#endif
			mkln(115,"PAGE:",4);
			tedit((char *)&pgcnt,"_0_",txt_line,R_SHORT);
			mkln(121,txt_line,3);
			if(prnt_line() < 0) return(ERROR);
#ifdef ENGLISH
			sprintf(e_mesg,"For the End of Period %d", period1);
#else
			sprintf(e_mesg,"Pour la fin de la periode %d",period1);
#endif
			mkln((LNSZ-1-strlen(e_mesg))/2,e_mesg,strlen(e_mesg));
			if(prnt_line() < 0) return(ERROR);
			if(prnt_line() < 0) return(ERROR);
			
#ifdef ENGLISH
			mkln(1,"FUND",4);
#else
			mkln(1,"FOND",4);
#endif
			mkln(6,ctl_rec.desc,strlen(ctl_rec.desc));
			mkln(11+strlen(ctl_rec.desc),"SECTION",7);
			tedit((char *)&gl_rec.sect,"_0",txt_line,R_SHORT);
			mkln(20+strlen(ctl_rec.desc),txt_line,2);
			if(prnt_line() < 0) return(ERROR);
			if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
			mkln(59,"-------- CURRENT YEAR ---------",31);
			mkln(99,"------- PREVIOUS YEAR --------",30);
#else
			mkln(59,"--------ANNEE COURANTE---------",31);
			mkln(99,"-------ANNEE PRECEDENTE-------",30);
#endif
			if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
			mkln(20,"DESCRIPTION",11);
			mkln(59,"PERIOD",6);
			mkln(81,"NEW",3);
			mkln(99,"PERIOD",6);
			mkln(121,"NEW",3);
#else
			mkln(20,"DESCRIPTION",11);
			mkln(59,"CHANGEMENTS",11);
			mkln(81,"NOUVEAU",7);
			mkln(99,"CHANGEMENTS",11);
			mkln(119,"NOUVEAU",7);
#endif
			if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
			mkln(59,"CHANGES",7);
			mkln(79,"BALANCE",7);
			mkln(99,"CHANGES",7);
			mkln(119,"BALANCE",7);
#else
			mkln(59,"EN PERIODE",10);
			mkln(82,"SOLDE",5);
			mkln(99,"EN PERIODE",10);
			mkln(120,"SOLDE",5);
#endif
			if(prnt_line() < 0) return(ERROR);

			break;
		case 3:		/* Period Comparision */
#ifdef ENGLISH
			mkln(50,"PERIOD COMPARATIVE BALANCE SHEET",32);
#else
			mkln(50,"  BILAN COMPARATIF DE PERIODES  ",32);
#endif
			mkln(115,"PAGE:",4);
			tedit((char *)&pgcnt,"_0_",txt_line,R_SHORT);
			mkln(121,txt_line,3);
			if(prnt_line() < 0) return(ERROR);
#ifdef ENGLISH
			sprintf(e_mesg,
			"Comparison of Period %d to Period %d",
				period1,period2);
#else
			sprintf(e_mesg,
			"Comparaison de la periode %d avec la periode %d",
				period1,period2);
#endif

			mkln((LNSZ-1-strlen(e_mesg))/2,e_mesg,strlen(e_mesg));
			if(prnt_line() < 0) return(ERROR);
			if(prnt_line() < 0) return(ERROR);
			
#ifdef ENGLISH
			mkln(1,"FUND",4);
#else
			mkln(1,"FOND",4);
#endif
			mkln(6,ctl_rec.desc,strlen(ctl_rec.desc));
			mkln(11+strlen(ctl_rec.desc),"SECTION",7);
			tedit((char *)&gl_rec.sect,"_0",txt_line,R_SHORT);
			mkln(20+strlen(ctl_rec.desc),txt_line,2);
			if(prnt_line() < 0) return(ERROR);
			if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
			sprintf(e_mesg,
				"--------- PERIOD %d ----------",period1);
			mkln(59,e_mesg,strlen(e_mesg));
			sprintf(e_mesg,
				"-------- PERIOD %d ----------",period2);
			mkln(99,e_mesg,strlen(e_mesg));
#else
			sprintf(e_mesg,
				"---------PERIODE %d-----------",period1);
			mkln(59,e_mesg,strlen(e_mesg));
			sprintf(e_mesg,
				"--------PERIODE %d-----------",period2);
			mkln(99,e_mesg,strlen(e_mesg));
#endif
			if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
			mkln(20,"DESCRIPTION",11);
			mkln(59,"PERIOD",6);
			mkln(81,"NEW",3);
			mkln(99,"PERIOD",6);
			mkln(121,"NEW",3);
#else
			mkln(20,"DESCRIPTION",11);
			mkln(59,"CHANGEMENTS",11);
			mkln(79,"NOUVEAU",7);
			mkln(99,"CHANGEMENTS",11);
			mkln(119,"NOUVEAU",7);
#endif
			if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
			mkln(59,"CHANGES",6);
			mkln(79,"BALANCE",7);
			mkln(99,"CHANGES",6);
			mkln(119,"BALANCE",7);
#else
			mkln(59,"EN PERIODE",10);
			mkln(80,"SOLDE",5);
			mkln(99,"EN PERIODE",10);
			mkln(120,"SOLDE",5);
#endif
			if(prnt_line() < 0) return(ERROR);

			break;
	}	

	if(prnt_line() < 0) return(ERROR);
	return(0);
}

/* Prints the details lines for the balance sheet  */
/* But if the summary flag is set to yes then only */
/* calculate the section totals.		   */

PrintDetail(report_no,summary)
int	report_no;
char	summary[2];
{
	double	total1;
	double	total2;

	mkln(1,gl_rec.desc,strlen(gl_rec.desc));
	if(gl_rec.reccod != 99) {
		if(prnt_line() < 0) return(ERROR);
		if(prnt_line() < 0) return(ERROR);
		return(0);
	}

	switch(report_no)
	{
		case 1:
			/* Calculate section totals */
			total1 = gl_rec.budcur + gl_rec.opbal;

			sect_total1 += gl_rec.currel[0];
			sect_total2 += total1;
			if(*summary == YES) break;

			tedit((char *)&gl_rec.currel[0],"___,___,_0_.__-",
				txt_line,R_DOUBLE);
			mkln(76,txt_line,15);
			tedit((char *)&total1,"___,___,_0_.__-",
				txt_line,R_DOUBLE);
			mkln(116,txt_line,15);

			break;
		case 2:
		case 3:
			/* Calculate section totals */
			total1 = gl_rec.budcur + gl_rec.opbal;
			total2 = gl_rec.budpre + gl_rec.opbal;

			sect_total1 += gl_rec.currel[0];
			sect_total2 += total1;
			sect_total3 += gl_rec.prerel[0];
			sect_total4 += total2;
			if(*summary == YES) break;

			tedit((char *)&gl_rec.currel[0],"___,___,_0_.__-",
				txt_line,R_DOUBLE);
			mkln(55,txt_line,15);
			tedit((char *)&total1,"___,___,_0_.__-",
				txt_line,R_DOUBLE);
			mkln(75,txt_line,15);
			tedit((char *)&gl_rec.prerel[0],"___,___,_0_.__-",
				txt_line,R_DOUBLE);
			mkln(95,txt_line,15);
			tedit((char *)&total2,"___,___,_0_.__-",
				txt_line,R_DOUBLE);
			mkln(115,txt_line,15);

			break;
	}
	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);

	return(0);
}

/* Print Section totals and calculate fund totals */
PrintSectTotals(report_no)
int	report_no;
{
	
	mkln(3,"SECTION",7);
	tedit((char *)&prev_sect,"_0_",txt_line,R_SHORT);
	mkln(11,txt_line,3);
	if(prnt_line() < 0) return(ERROR);
	
	switch(report_no)
	{
		case 1:
			tedit((char *)&sect_total1,"___,___,_0_.__-",
				txt_line,R_DOUBLE);
			mkln(76,txt_line,15);
			tedit((char *)&sect_total2,"___,___,_0_.__-",
				txt_line,R_DOUBLE);
			mkln(116,txt_line,15);
			if(prnt_line() < 0) return(ERROR);
			
			/* Calculate fund totals */
			fund_total1 += sect_total1;
			fund_total2 += sect_total2;
			break;
		case 2:
		case 3:
			tedit((char *)&sect_total1,"___,___,_0_.__-",
				txt_line,R_DOUBLE);
			mkln(55,txt_line,15);
			tedit((char *)&sect_total2,"___,___,_0_.__-",
				txt_line,R_DOUBLE);
			mkln(75,txt_line,15);
			tedit((char *)&sect_total3,"___,___,_0_.__-",
				txt_line,R_DOUBLE);
			mkln(95,txt_line,15);
			tedit((char *)&sect_total4,"___,___,_0_.__-",
				txt_line,R_DOUBLE);
			mkln(115,txt_line,15);
			if(prnt_line() < 0) return(ERROR);
			
			/* Calculate fund totals */
			fund_total1 += sect_total1;
			fund_total2 += sect_total2;
			fund_total3 += sect_total3;
			fund_total4 += sect_total4;
			break;
	}
	if(prnt_line() < 0) return(ERROR);

	/* Initialize section totals */
	sect_total1 = sect_total2 = 0.00;
	sect_total3 = sect_total4 = 0.00;
	return(0);
}

/* Print surplus/deficit for the current year */
PrintSurplus(report_no,summary)
int	report_no;
char	summary[2];
{
	
	if(*summary != YES)
#ifdef ENGLISH
		mkln(1,"Surplus/Deficit for year",24);
#else
		mkln(1,"Surplus/Deficit pour l'annee",28);
#endif
	switch(report_no)
	{
		case 1:
			/* Calculate section totals */
			sect_total1 += sd_per1_chg;
			sect_total2 += sd_cur1_bal;

			if(*summary == YES) break;

			tedit((char *)&sd_per1_chg,"___,___,_0_.__-",
				txt_line,R_DOUBLE);
			mkln(76,txt_line,15);
			tedit((char *)&sd_cur1_bal,"___,___,_0_.__-",
				txt_line,R_DOUBLE);
			mkln(116,txt_line,15);

			break;
		case 2:
			sect_total1 += sd_per1_chg;
			sect_total2 += sd_cur1_bal;
			sect_total3 += sd_prev_chg;
			sect_total4 += sd_prev_bal;

			if(*summary == YES) break;

			tedit((char *)&sd_per1_chg,"___,___,_0_.__-",
				txt_line,R_DOUBLE);
			mkln(55,txt_line,15);
			tedit((char *)&sd_cur1_bal,"___,___,_0_.__-",
				txt_line,R_DOUBLE);
			mkln(75,txt_line,15);
			tedit((char *)&sd_prev_chg,"___,___,_0_.__-",
				txt_line,R_DOUBLE);
			mkln(95,txt_line,15);
			tedit((char *)&sd_prev_bal,"___,___,_0_.__-",
				txt_line,R_DOUBLE);
			mkln(115,txt_line,15);

			break;
		case 3:
			sect_total1 += sd_per1_chg;
			sect_total2 += sd_cur1_bal;
			sect_total3 += sd_per2_chg;
			sect_total4 += sd_cur2_bal;

			if(*summary == YES) break;

			tedit((char *)&sd_per1_chg,"___,___,_0_.__-",
				txt_line,R_DOUBLE);
			mkln(55,txt_line,15);
			tedit((char *)&sd_cur1_bal,"___,___,_0_.__-",
				txt_line,R_DOUBLE);
			mkln(75,txt_line,15);
			tedit((char *)&sd_per2_chg,"___,___,_0_.__-",
				txt_line,R_DOUBLE);
			mkln(95,txt_line,15);
			tedit((char *)&sd_cur2_bal,"___,___,_0_.__-",
				txt_line,R_DOUBLE);
			mkln(115,txt_line,15);

			break;
	}
	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);

	sd_per1_chg = 0.00;
	sd_cur1_bal = 0.00;
	sd_per2_chg = 0.00;
	sd_cur2_bal = 0.00;
	sd_prev_chg = 0.00;
	sd_prev_bal = 0.00;
	sd_per1_chg = 0.00;

	return(0);
}

/* Print Fund totals and calculate Grand totals */
PrintFundTotals(report_no)
int	report_no;
{
#ifdef ENGLISH	
	mkln(3,"FUND",4);
#else
	mkln(3,"FOND",4);
#endif
	mkln(9,ctl_rec.desc,strlen(ctl_rec.desc));
	if(prnt_line() < 0) return(ERROR);
	
	switch(report_no)
	{
		case 1:
			tedit((char *)&fund_total1,"___,___,_0_.__-",
				txt_line,R_DOUBLE);
			mkln(76,txt_line,15);
			tedit((char *)&fund_total2,"___,___,_0_.__-",
				txt_line,R_DOUBLE);
			mkln(116,txt_line,15);
			if(prnt_line() < 0) return(ERROR);
			
			/* Calculate fund totals */
			grand_total1 += fund_total1;
			grand_total2 += fund_total2;
			break;
		case 2:
		case 3:
			tedit((char *)&fund_total1,"___,___,_0_.__-",
				txt_line,R_DOUBLE);
			mkln(55,txt_line,15);
			tedit((char *)&fund_total2,"___,___,_0_.__-",
				txt_line,R_DOUBLE);
			mkln(75,txt_line,15);
			tedit((char *)&fund_total3,"___,___,_0_.__-",
				txt_line,R_DOUBLE);
			mkln(95,txt_line,15);
			tedit((char *)&fund_total4,"___,___,_0_.__-",
				txt_line,R_DOUBLE);
			mkln(115,txt_line,15);
			if(prnt_line() < 0) return(ERROR);
			
			/* Calculate fund totals */
			grand_total1 += fund_total1;
			grand_total2 += fund_total2;
			grand_total3 += fund_total3;
			grand_total4 += fund_total4;
			break;
	}
	fund_total1 = 0.00;
	fund_total2 = 0.00;
	fund_total3 = 0.00;
	fund_total4 = 0.00;

	return(0);
}

/* Print Grand totals */
PrintGrandTotals(report_no)
int	report_no;
{
	
	mkln(3,"*** Grand Total ***",19);
	if(prnt_line() < 0) return(ERROR);
	
	switch(report_no)
	{
		case 1:
			tedit((char *)&grand_total1,"___,___,_0_.__-",
				txt_line,R_DOUBLE);
			mkln(76,txt_line,15);
			tedit((char *)&grand_total2,"___,___,_0_.__-",
				txt_line,R_DOUBLE);
			mkln(116,txt_line,15);
			if(prnt_line() < 0) return(ERROR);
			
			break;
		case 2:
		case 3:
			tedit((char *)&grand_total1,"___,___,_0_.__-",
				txt_line,R_DOUBLE);
			mkln(55,txt_line,15);
			tedit((char *)&grand_total2,"___,___,_0_.__-",
				txt_line,R_DOUBLE);
			mkln(75,txt_line,15);
			tedit((char *)&grand_total3,"___,___,_0_.__-",
				txt_line,R_DOUBLE);
			mkln(95,txt_line,15);
			tedit((char *)&grand_total4,"___,___,_0_.__-",
				txt_line,R_DOUBLE);
			mkln(115,txt_line,15);
			if(prnt_line() < 0) return(ERROR);
			
			break;
	}
	grand_total1 = 0.00;
	grand_total2 = 0.00;
	grand_total3 = 0.00;
	grand_total4 = 0.00;

	return(0);
}
/*-------------------- E n d   O f   P r o g r a m ---------------------*/

