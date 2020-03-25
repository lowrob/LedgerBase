/*
*	Source 	: pobyacct.c 
*
*  Program to Print purchase orders by G/L Account no. using REPORT GENERATOR.
*
* Modified:
	Date		Programmer		Description
	-----------	----------	---------------------------------------
	Nov 30,1990	F.Tao		Change Amount by using Commitment 
					Calculation.
*/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	PROJNAME	"porpt"
#define	LOG_REC		1
#define	FORMNO		4
#define EXIT		12
#define TAXABLE		'T'

#ifdef ENGLISH
#define PRINTER 'P'
#define DISPLAY 'D'
#define FILE_IO	'F'
#define YES	'Y'
#else
#define PRINTER 'I'
#define DISPLAY 'A'
#define FILE_IO	'D'
#define YES	'O'
#endif

extern	int	rperror;
extern char	e_mesg[80] ;

PobyAcct()
{
Pa_rec 	 pa_rec ;
Supplier supp_rec;
Po_hdr   pohdr_rec;
Po_item  poitem_rec;
Gl_rec	 gl_rec;
Ctl_rec	 ctl_rec;
Tax_cal	 tax_ret;

char	chardate[11] ;
int	code;
char 	*arayptr[5] ; 	/** Declarations for Report writer usage **/
char 	projname[50] ;
char 	program[11] ;
int 	outcntl ;
char 	discfile[20] ;
int	retval;
short	fund1, fund2;
short	prev_fund = 0;
short	gst_rate, pst_rate, rebate;
short	gst_rate_temp, pst_rate_temp, rebate_temp;
short	copies ;
char    acct1[19], acct2[19];
char	outstanding[2] ;
double  Commit_Calculation();
double	amount;

#ifdef ENGLISH
	STRCPY(e_mesg, "P");
#else
	STRCPY(e_mesg, "I");
#endif
	retval = GetOutputon(e_mesg);
	if(retval < 0) return(retval);

	switch( *e_mesg){
		case DISPLAY:	/* display on Terminal */
			outcntl = 0;
			break;
		case FILE_IO:	/* print to a file */
			outcntl = 1;
			break;
		case PRINTER:
			outcntl = 2;
			break;
		default:
			outcntl = 2;
			break;
	}
	if(outcntl == 1) {
		STRCPY(e_mesg,"pobyacct.dat");
		if((retval = GetFilename(e_mesg)) < 0)
			return(retval);
		STRCPY(discfile, e_mesg);
	}
	else 	discfile[0] = '\0';
	
	copies = 1;
	if(outcntl == 2) {
		if((retval = GetNbrCopies( &copies )) < 0)
			return(retval);
	}

	fund1 = 1;
	fund2 = 999;
	retval = GetFundRange( &fund1, &fund2 ); 
	if(retval < 0) return(retval);
	STRCPY(acct1,"                 1");
	STRCPY(acct2,"999999999999999999");
	retval = GetAcctRange( acct1, acct2 );
	if(retval < 0) return(retval);

	if((retval = 
#ifdef ENGLISH
		DisplayMessage("Do you want only outstanding PO's (Y/N)?"))<0)
#else
		DisplayMessage("Desirez-vous seulement les BC non-regles (O/N)?"))<0)
#endif
		return(retval);
	if((retval = GetResponse( outstanding )) < 0)
		return(retval); 

	if((retval = Confirm()) <= 0) return(retval);

	code = get_param(&pa_rec, BROWSE, 1, e_mesg) ;
	if(code < 1) {
		close_dbh() ;
		return(DBH_ERR) ;
	}

	mkdate(get_date(),chardate);

	STRCPY(projname,FMT_PATH) ;
	strcat(projname,PROJNAME) ;
	STRCPY(program, PROG_NAME );

	code = rpopen(projname,LOG_REC,FORMNO,outcntl,discfile,program,
			chardate);
							  	
	if ( code < 0 ){
		sprintf(e_mesg,"Rpopen code :%d\n", code ) ;
		close_dbh() ;
		return(REPORT_ERR);
	}
	if(outcntl == 2) 
		rpSetCopies( (int)copies );   /* number of copies to print */

	/* Change first title line to Company/School district name */
	if((retval = rpChangetitle(1, pa_rec.pa_co_name))<0) {
#ifdef ENGLISH
		sprintf(e_mesg,"Internal report error");
#else
		sprintf(e_mesg,"Erreur au rapport interne");
#endif
		rpclose();
		close_dbh();
		return(REPORT_ERR);
	}
	/* For Terminals set pagesize to 23 lines */
	if(outcntl == 0)
		rpPagesize(23);

	arayptr[0] = (char *)&supp_rec ;
	arayptr[1] = (char *)&pohdr_rec ;
        arayptr[2] = (char *)&poitem_rec ;
        arayptr[3] = (char *)&gl_rec ;
        arayptr[4] = NULL ;

	/***	Read record and call report writer to output it ***/

	/* Initialize Recs. to start printing po report on po no. */
	poitem_rec.pi_fund = fund1;
	STRCPY(poitem_rec.pi_acct,acct1);
	poitem_rec.pi_code = 0;
	flg_reset( POITEM );

	for( ; ; ) {
		code = get_n_poitem(&poitem_rec,BROWSE,1,FORWARD,e_mesg);
		if( code < 0) {
			if(code == EFL) break ;
			code = DBH_ERR;
			break ;
		}
		if(poitem_rec.pi_fund > fund2) break;
		if(strcmp(poitem_rec.pi_acct, acct2) > 0) {
			if(poitem_rec.pi_fund++ > fund2) break;
			STRCPY(poitem_rec.pi_acct,acct1);
			flg_reset( POITEM );
			continue;
		} 
		/* get gst_rate */
		if (prev_fund != poitem_rec.pi_fund) {
			ctl_rec.fund 	= poitem_rec.pi_fund;
			retval = get_ctl(&ctl_rec,BROWSE,0,e_mesg);
			if( retval < 0 ){
				code = DBH_ERR;
				break;
			}
			gst_rate_temp	= ctl_rec.gst_tax;	
			pst_rate_temp	= ctl_rec.pst_tax;
			rebate_temp	= ctl_rec.rebate;
			prev_fund	= ctl_rec.fund;
		}
		if (poitem_rec.pi_tax1[0] == TAXABLE){ 
			gst_rate = gst_rate_temp;  
			rebate = rebate_temp;
		}
		else{
			gst_rate = 0.0;    
			rebate = 0.0 ;
		}    
		if (poitem_rec.pi_tax2[0] == TAXABLE)
			pst_rate = pst_rate_temp;
		else
			pst_rate = 0.0;    

		/* call commi_cal and store comm_amt in pi_value */
		amount = poitem_rec.pi_value - poitem_rec.pi_paid ;
		poitem_rec.pi_value = Commit_Calculation(gst_rate,pst_rate,
					rebate,	amount,&tax_ret);
		/*****
		 get account description from gl master file
		*****/
		gl_rec.funds = poitem_rec.pi_fund ;
		STRCPY(gl_rec.accno, poitem_rec.pi_acct) ;
	     	gl_rec.reccod = 99 ;
		code = get_gl(&gl_rec,BROWSE,0,e_mesg);
		if( code < 0) {
			STRCPY(gl_rec.desc,"???????????????");
		}
		strncpy(supp_rec.s_add1, gl_rec.desc, 30);
		pohdr_rec.ph_code = poitem_rec.pi_code;
		code = get_pohdr(&pohdr_rec,BROWSE,0,e_mesg);
		if( code < 0) {
			code = DBH_ERR;
			break;	
		}
		if(outstanding[0] == YES) {
			if(pohdr_rec.ph_status[0] != OPEN) continue ;
		}
		if(poitem_rec.pi_orig_qty == poitem_rec.pi_pd_qty) continue;

		if((code = rpline(arayptr)) < 0){
			if(rperror < 0)  {
#ifdef ENGLISH
			sprintf(e_mesg,"Internal report error");
#else
			sprintf(e_mesg,"Erreur au rapport interne");
#endif
				code = REPORT_ERR;
			}
			else
				code = EXIT;
			break ;
		}
	}
	close_dbh() ;

	rpclose() ;
	if(code == EFL) return(0);
	return(code);
}
