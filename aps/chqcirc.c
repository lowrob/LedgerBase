/******************************************************************************
		Sourcename    : chqcirc.c
		System        : Budgetary Financial system.
		Module        : Accounts Payable reports
		Created on    : 89-12-06
		Created  By   : Jonathan Prescott
		Cobol sources : 
*******************************************************************************
About this file:
	This routine generates one of the following :
		1. List of cheques 
		2. List of out standing cheques
	Calling file:	
******************************************************************************/
#define RPTALL		(mode == 0)
#define RPTOUTSTAND	(mode == 1)

#define PROJNAME	"chqhrpt"
#define LOG_REC		1
#define	EXIT		12

#include <stdio.h>
#include <repdef.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#ifdef ENGLISH
#define PRINTER		'P'
#define DISPLAY		'D'
#define FILE_IO		'F'
#define OUTSTANDING	'O'
#else
#define PRINTER		'I'
#define DISPLAY		'A'
#define FILE_IO		'D'
#define OUTSTANDING	'N'
#endif 

static char	chardate[11];
static int	retval;
static int	formno;

static Chq_hist		chq_hist;
static Pa_rec		pa_rec;
static Ctl_rec		ctl_rec;

static char 	discfile[20] ;
static int	outcntl;
static short	copies ;

static short	fund_trans; 
static double	fund_amount;
static short	temp_fund;
static long	date1, date2;
static char	chardate1[11], chardate2[11];

extern char	e_mesg[80] ;

ChequeCirc(mode)
int mode;
{
	
	char	projname[50];
	char	program[11];
	char	*arayptr[5];
	int	first;
	
#ifdef ENGLISH
	STRCPY(e_mesg,"P");
#else
	STRCPY(e_mesg,"I");
#endif
	retval = GetOutputon(e_mesg);
	if(retval < 0) return(retval);
	else if(retval == EXIT) return(0);

	switch (*e_mesg) {
		case DISPLAY :	/*  Display on Terminal */
			outcntl = 0;
			break;
		case FILE_IO : 	/*  Print to a disk file */ 
			outcntl = 1;
			break;
		case PRINTER : 	/*  Print to a printer */ 
		default  :
			outcntl = 2;
			break;
	}

	if(outcntl == 1) {
		STRCPY( discfile, "chqcirc.dat");
		retval = GetFilename(discfile);
		if( retval<0 || retval==EXIT )
			return(retval);
	}
	else discfile[0] = '\0';

	copies = 1;
	if(outcntl == 2) {
		if((retval = GetNbrCopies( &copies )) < 0)
			return(retval);
	}
	date1 = date2 = get_date();
	retval = GetTransDateRange(&date1, &date2);
	if (retval < 0 ) return(-1);
	else if (retval == EXIT) return(-1);
	
	if ( (retval=Confirm())<= 0) 
		return(retval);

	if( get_param(&pa_rec,BROWSE,1,e_mesg)<1 ){
		return(DBH_ERR);
	}

	mkdate(get_date(),chardate);
	STRCPY(projname,FMT_PATH );
	strcat(projname,PROJNAME) ;
	STRCPY(program, PROJNAME );

	if(RPTALL) 
		formno = 1;
	else	formno = 2;

	retval = rpopen(projname,LOG_REC,formno,outcntl,discfile,program,
			   chardate);
	if(retval != NOERROR) {
		sprintf(e_mesg,"Rpopen code :%d\n",retval);
		close_dbh();
		return(REPORT_ERR);
	}

	if(outcntl == 2) 
		rpSetCopies( (int)copies );	/* number of copies to print */

	/* Change first title line to Company/School disctrict name */
	if((retval = rpChangetitle(1, pa_rec.pa_co_name ))<0) {
#ifdef ENGLISH
		sprintf(e_mesg,"Internal report error");
#else
		sprintf(e_mesg,"Erreur au rapport interne");
#endif
		return(REPORT_ERR);
	}

	mkdate(date1, chardate1);
	mkdate(date2, chardate2);
	if (mode == 0){
#ifdef ENGLISH
	sprintf(e_mesg,"CHEQUE LIST FROM %s to %s", chardate1, chardate2);
#else
	sprintf(e_mesg,"DE %s to %s", chardate1, chardate2);
#endif
	}
	else {

#ifdef ENGLISH
	sprintf(e_mesg,"OUTSTANDING CHEQUE LIST FROM %s to %s", chardate1, chardate2);
#else
	sprintf(e_mesg,"DE %s to %s", chardate1, chardate2);
#endif
	}
	rpChangetitle(2, e_mesg);

	/* for terminal set pagesize to 23 */
	if(outcntl == 0) rpPagesize(23);

	arayptr[0] = (char *)&chq_hist ;
	arayptr[1] = (char *)&ctl_rec ;
	arayptr[2] = NULL ;

	fund_trans = 0;
	fund_amount = 0;

	chq_hist.ch_date = date1;
	chq_hist.ch_funds = 0;
	chq_hist.ch_accno[0] = '\0' ;
	chq_hist.ch_chq_no = 0;	

	first = 1;

	flg_reset(CHQHIST);
	for(;;) {
	
		retval = get_n_chqhist(&chq_hist,BROWSE,0, FORWARD,e_mesg);
		if(retval < 0) {
			if(retval == EFL) break;
			retval = DBH_ERR;
			break;
		}
		if(first == 1) {
			ctl_rec.fund = chq_hist.ch_funds;
			retval = get_ctl(&ctl_rec,BROWSE,0,e_mesg);
			if(retval < 0) {
				STRCPY(ctl_rec.desc,"???????????????");	
			}	
			temp_fund = chq_hist.ch_funds;
			first = 2;
		}
		if(RPTOUTSTAND && chq_hist.ch_status[0] != OUTSTANDING) 
			continue;
		if (chq_hist.ch_date < date1 || chq_hist.ch_date > date2)
			continue;	
		if(temp_fund == chq_hist.ch_funds && 
			chq_hist.ch_status[0] == OUTSTANDING) { 
				fund_amount += chq_hist.ch_net_amt;
				fund_trans++;
		}			
		else if(temp_fund != chq_hist.ch_funds) {
			retval = PrintTotals(fund_trans, fund_amount) ;
			if(retval < 0) break;
			fund_trans = 1;
			fund_amount = chq_hist.ch_net_amt;
			temp_fund = chq_hist.ch_funds;
			ctl_rec.fund = chq_hist.ch_funds;
			retval = get_ctl(&ctl_rec,BROWSE,0,e_mesg);
			if(retval < 0) {
				STRCPY(ctl_rec.desc,"???????????????");	
			}	
		}		
		if((retval = rpline(arayptr)) < 0) break;	
	}

	retval = PrintTotals(fund_trans, fund_amount) ;
	if(retval == EFL) retval = 0;
	if(retval == REPORT_ERR) {
#ifdef ENGLISH
		sprintf(e_mesg,"Internal report error");
#else
		sprintf(e_mesg,"Erreur au rapport interne");
#endif
	}
	rpclose() ;
	close_dbh() ;
	return(retval);
}
PrintTotals(trans, amount)
short	trans ;
double	amount ;
{
	char	output[20] ;
	int	offset;

	if(formno == 1) 
		offset = 6;
	else 	offset = 5;
	if(rpPutline() < 0) return(REPORT_ERR);
#ifdef ENGLISH
	rpMkline(2,"No of Cheques");
#else
	rpMkline(2,"Nombre de cheques");
#endif
	sprintf(e_mesg,"%d",trans);
	rpMkline(3,e_mesg);
	if(formno == 1)
#ifdef ENGLISH
		rpMkline(offset,"TOTAL");
#else
		rpMkline(offset,"TOTAL");
#endif
	else
	        rpMkline(offset,
			"                                          TOTAL");
	tedit((char*)&amount, "___,___,_0_.__-", output, R_DOUBLE) ;
	rpMkline(offset+1,output);
	if(rpPutline() < 0) return(REPORT_ERR);
	return(0);
}
