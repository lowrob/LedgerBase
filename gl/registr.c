/****
*	G/L Transaction Register Report ..
*	Source 	: registr.c 
*	Compile	: cc -I$I registr.c
*	Linking	: cc registr.o rp.o /usr/bfs/dbh/libbfs.a -o registr.out
*	Modifications : rpopen has more parameters for new rp library
*	MODIFIED: JUNE 20, 1990
*	DESC	: change for new journal entry numbering 
*	MODIFIED: NOVEMBER 30, 1990
*	DESC	: added if(rperror < 0) and extra break to make Q(uit)
*		  feature work properly in the Display option.

Usage of SWITCHES when they are ON :
	SW1 :
	SW2 :
	SW3 :
	SW4 :
	SW5 :
	SW6 :
	SW7 :
	SW8 :
		Not Used.
****/


#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>
#include <repdef.h>

#define	EXIT	12

#ifdef ENGLISH
#define PRINTER 'P'
#define DISPLAY 'D'
#define FILE_IO	'F'
#else
#define PRINTER 'I'
#define DISPLAY 'A'
#define FILE_IO	'D'
#endif

registr() 
{
static	char	chardate[11] ;
static	char	chardate2[11] ;
static	long	longdate ;
static 	int	retval;
extern	int	rperror;

Gl_rec	gl_rec ;
Tr_hdr	tr_hdr ;
Tr_item	tr_item ;
Ctl_rec	ctl_rec ; 	/**** 	Declarations for DBH record reading  ****/
Pa_rec	pa_rec ;
int	code ;
char	e_mesg[80] ;
char 	*arayptr[7] ; 	/**** 	Declarations for Report writer usage ****/
char 	projname[50] ;
int 	logrec ;
int 	formno ;
int 	outcntl ;
short	copies ;
char 	discfile[20] ;
char	program[11];
	
double	amount;
double	opnbalance;
char 	str[16];
int	flag;
char	acct1[19], acct2[19] ;
short	fund1, fund2 ;
short	period1, period2 ;
int	i ;
/****	
*	Accept Project code, Logical record number in the 
*	Project and format number for the logical record ..
*	Accept the output media code . Open the report ..
****/

	STRCPY( program, "REGISTR" );
	STRCPY(projname,FMT_PATH) ;
	strcat( projname, "registr" );
	logrec = 1;
	formno = 1 ;

	code = get_param(&pa_rec, BROWSE, 1, e_mesg);
	if (code < 1) {
		printf("%s\n", e_mesg);
		close_dbh();
		return(-1);
	}

#ifdef ENGLISH
	STRCPY( e_mesg, "P" );
#else
	STRCPY( e_mesg, "I" );
#endif
	retval = GetOutputon( e_mesg );
	if ( retval < 0 )
		return(-1);
	else if( retval == EXIT )
		return(0);

	switch (*e_mesg) {
		case DISPLAY :	/*  Display on Terminal */
				outcntl = 0 ;
				break;
		case FILE_IO : 	/*  Print to a disk file */ 
				outcntl = 1 ;
				break;
		case PRINTER : 	/*  Print to a printer */ 
				outcntl = 2 ;
				break;
		default  :
				outcntl = 2 ;
				break;
	}
		
	if(outcntl == 1) {
		STRCPY( e_mesg, "registr.dat");
		if( GetFilename( e_mesg ) < 0 ) return(-1);;
		STRCPY (discfile, e_mesg) ;
	}
	else
		discfile[0] = '\0' ;
	
	copies = 1;
	if(outcntl == 2) {
		if((retval == GetNbrCopies( &copies )) < 0)
			return(retval);
	}

	/*  Getting Fund range from user */
	fund1 = 1;
	fund2 = 999;

	retval = GetFundRange( &fund1, &fund2 );
	if (retval < 0)
		return(-1);
	else if ( retval == EXIT )
			return(0); 

	/*  Getting Account number range from user */
	STRCPY(acct1 ,"                 1");
	STRCPY(acct2 ,"999999999999999999");

	retval = GetAcctRange( acct1, acct2 );
	if (retval < 0)
		return(-1);
	else if ( retval == EXIT )
			return(0); 

	/* Getting Period number range from user */
	period1 = 01;
	period2 = pa_rec.pa_no_periods;

	retval = GetPeriodRange( &period1, &period2 );
	if (retval < 0)
		return(-1);
	else if ( retval == EXIT )
			return(0); 

	if ((retval = Confirm()) < 0) return(-1);
	else
		if (!retval) return(0);
	mkdate(get_date(),chardate);

	code = rpopen(projname,logrec,formno,outcntl,discfile, 
		program,chardate);
							  	
	if ( code < 0 ){
		sprintf( e_mesg,"Rpopen code :%d\n", code ) ;
		fomen(e_mesg);
		get();
		exit(-1);
	}

	if(outcntl == 2)
		rpSetCopies( (int)copies );	/* number of copies to print */

	rpChangetitle(1, pa_rec.pa_co_name);

	/* For Terminals set pagesize to 23 lines */
	if(outcntl == 0)
		rpPagesize(23);

/***	Prepare to read glmast sequentialy ***/
	
	gl_rec.funds = fund1 ;
	STRCPY( gl_rec.accno, acct1);
	gl_rec.reccod = 0 ;
	

	STRCPY(tr_item.ti_accno, "                  ");

/***	Initialize the pointer array's first element to the record ***/

	arayptr[0] = (char *)&gl_rec ; 
	arayptr[1] = (char *)&tr_hdr ; 
	arayptr[2] = (char *)&tr_item ; 
	arayptr[3] = (char *)&ctl_rec ; 
	arayptr[4] = NULL ;
	

/***	Read record and call report writer to output it ***/
	flag = 0 ;
	for(;;) {
		code = get_n_gl(&gl_rec, BROWSE, 0, FORWARD, e_mesg) ;
		if (flag != 0)  {
			flag = 0 ;
			rpPutline();
#ifdef ENGLISH
			rpMkline(6,"CLOSING BALANCE: ");
#else
			rpMkline(6,"BALANCE DE FERMETURE: ");
#endif
			tedit((char *)&amount,"__,___,_0_.__-",str,R_DOUBLE);
			rpMkline(8,str);
			rpPutline();
			rpPutline();
		}

		if(code < 0) break ;
		if(gl_rec.funds > fund2 ) break;
		if(strcmp(gl_rec.accno,acct1)<0 || 
				strcmp(gl_rec.accno,acct2)>0) {
			if(strcmp(gl_rec.accno,acct2)>0) {
				gl_rec.funds++;
			}
			STRCPY(gl_rec.accno,acct1);
			gl_rec.reccod = 99;
			flg_reset(GLMAST);
			continue;
		}
		if(gl_rec.reccod != 99) {
			gl_rec.reccod = 99;
			flg_reset(GLMAST);
			continue;
		}
		/**
		if (gl_rec.reccod != 99) continue;
		if (gl_rec.funds < fund1 || gl_rec.funds > fund2) continue;
		if (strcmp( gl_rec.accno, acct1) < 0 ) continue;
		if (strcmp( acct2, gl_rec.accno) < 0 ) continue;
		**/

		if(gl_rec.funds != ctl_rec.fund) {
			ctl_rec.fund = gl_rec.funds ;
			code = get_ctl(&ctl_rec, BROWSE, 0, e_mesg) ;
			if(code < 0) break ;
		}

		/* calculate  current opening balance */
		if(flag == 0) {
			opnbalance = gl_rec.opbal ;
			for(i = 0; i < period1-1; i++){
				opnbalance += gl_rec.currel[i];
			}
			/* setup new openning balance to print */
			gl_rec.opbal = opnbalance ;
		}
		amount = gl_rec.opbal ;

		tr_item.ti_fund = gl_rec.funds ;
		tr_item.ti_reccod = gl_rec.reccod ;
		STRCPY( tr_item.ti_accno, gl_rec.accno );
		tr_item.ti_period = period1 ;
		tr_item.ti_seq_no = 0 ;
		tr_item.ti_item_no = 0 ;
      		flg_reset(GLTRAN);
	    	for(;;) {
			code = get_n_tritem(&tr_item, BROWSE, 1, FORWARD, e_mesg) ;
			if(code < 0) break ;

			if (gl_rec.funds != tr_item.ti_fund ||
			    gl_rec.reccod != tr_item.ti_reccod ||
			    (strcmp(gl_rec.accno, tr_item.ti_accno) != 0) ||  
			    tr_item.ti_period > period2) 
				break;

			if(tr_item.ti_period < period1) continue;

			tr_hdr.th_fund = tr_item.ti_fund;
			tr_hdr.th_reccod = tr_item.ti_reccod;
			tr_hdr.th_create[0] = tr_item.ti_create[0];
			tr_hdr.th_seq_no = tr_item.ti_seq_no ;
			code = get_trhdr(&tr_hdr, BROWSE, 0, e_mesg) ;
			if(code < 0) break ;
	
			flag = 1;

			if((code = rpline(arayptr)) < 0)  {
				if(rperror < 0)  {
#ifdef ENGLISH
				sprintf(e_mesg,"Internal report error");
#else
				sprintf(e_mesg,"Erreur au rapport interne");
#endif
					code = REPORT_ERR;
				}
				else
					code = EXIT ;
 				break ;
			}
			amount += tr_item.ti_amount ;
		}
		if((code < NOERROR || code == EXIT) && code != EFL) break;	

	}

/****	Windup .. ****/

	close_dbh() ;
	rpclose() ;
	if(code == EFL)  return(0);
	return(code);
   
}
