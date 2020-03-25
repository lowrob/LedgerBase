/*--------------------------------------------------------------------------
	SOURCE NAME:  CUSTSTAT.C
	SYSTEM     :  BUGETARY FINANCIAL SYSTEM 
	MODULE     :  Account Receivable Module
	CREATED ON :  26 Nov. 1990 
	CREATED BY :  Frank Tao

DESCRIPTION:
	This program prints a range of customer statement by customer codes.
	It allows for the user to do a test print to allign the forms.

MODIFICATIONS:

PROGRAMMER   	YY/MM/DD     	DESCRIPTION OF MODIFICATIONS
__________	__/__/__	____________________________

C.Leadbeater 	90/12/06 	Change so that blank lines are not 
				printed in address.
F.Tao  		90/12/07  	Add direct print customer statement,
			 	test print from monthend menu.
			  	Add unapplied receipts shown on statement.
C.Leadbeater 	90/12/07  	Make value in 'AMOUNT' column equal to
			  	'DEBIT' or 'CREDIT' column not 'BALANCE'
				column.
F.Tao  		90/12/11  	Change amount to taxable amount. 
F.Tao 	     	91/01/22	1.Print only the statements with transaction,
				2.Print transactions within current month,
				  alghough they may be completed.
				3.Fix bugs.  (did not print all transactions)
J.Prescott	91/03/01	Changed to use the receipts head & items file.
J McLean        92/07/27	Changed customer code from 6 to 10 characters.
J McLean	92/08/17	Changed code to correctly print Applied 
				payments under their related invoices.
L.Robichaud	1993/10/13	Pass new parameter to close_rep function for 
				a banner to print with 3000 family.
----------------------------------------------------------------------------*/
#include <stdio.h>
#include <repdef.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#ifdef ENGLISH
#define YES	'Y'
#define NO	'N'

#define APPLIED		'A'

#define INVOICE		"IN"
#define DBMEMO		"DM"
#define CRMEMO		"CM"
#define APPLY_STAT	"AP"
#define UNAPP_STAT	"UA"
#else
#define YES	'O'
#define NO	'N'

#define APPLIED		'A'

#define INVOICE		"FC"
#define DBMEMO		"ND"
#define CRMEMO		"NC"
/* XXXXXX translate */
#define APPLY_STAT	"AP"
#define UNAPP_STAT	"NA"
#endif

#define	 ITEMLINES	38
#define  EXIT  12

static Pa_rec   pa_rec;             /*  Declarations for DBH record reading */
static Ar_hdr   arhdr_rec;
static Cu_rec	cu_rec;
static Rcpt_hdr rcpt_hdr;
static Rcpt_item rcpt_item;

static char	 answer[2];
static int 	 retval; 
static short	 printer;
static char      program[11];
static char 	 cust1[11], cust2[11];
static char  	 ref_no[12];
static double    balance = 0, total_debit = 0, total_credit = 0; 
static double	 due_90  = 0, due_60 = 0, due_30 = 0, due_0 = 0;
extern char 	 e_mesg[80];

/* -----------------------------------------------------------------------*/
/* Function Print Customer Statement	     			          */
/* -----------------------------------------------------------------------*/

custstat(mode)
int 	mode;
{
	LNSZ = 132;
		
	strcpy(program,"PRNTCUST");
	printer = 1;
	if (mode != 1){
		if((retval = GetPrinter( &printer ))<0)
			return(retval);
	}
	strcpy(cust1 ,"         1");
	strcpy(cust2 ,"ZZZZZZZZZZ");
	if (mode != 1){
		retval = GetCNbrRange( cust1, cust2 );
			if(retval < 0) return(retval);
	}

	/* always to printer */
	retval = opn_prnt("P",'\0',printer,e_mesg,0);  
	if(retval < 0) {
		return(REPORT_ERR);
	}


	if (mode == 2){
		retval = align_cust();
		rite_top();
		return(retval);
	}

        for( ; ; ) {
		if (mode == 1) break;
#ifdef ENGLISH
		if((retval=DisplayMessage("Is Customer Statement Aligned (Y/N)?"))<0)
#else
		if((retval=DisplayMessage("Le formulaire pour releves de comptes des clients est-il aligne (O/N)?"))<0)
#endif
			break;	
		if((retval = GetResponse(answer,"YN")) <0)
			break;
		if(answer[0] == YES) break;
		retval = align_cust();
		rite_top();
		if(retval != NOERROR) break;
	}
	if(retval == NOERROR) {
		if(mode == 0) {
			if(( retval = Confirm()) <= 0)
				return(retval);
		}
		retval = get_param(&pa_rec,BROWSE,1,e_mesg);
        	if(retval < 1) {
			retval = DBH_ERR;
		}
		if(retval != DBH_ERR)
			retval = print_cust();
	}
	close_rep(NOBANNER);
	close_dbh();
	return(retval);
}

/* ---------------------------------------------------------------------------
Function:  prints (*) in a customer statement to test for alignment.
--------------------------------------------------------------------------- */
static
align_cust()
{
        int cnt;
        static char tst_string[] = "****************************************";
       	
	if(prnt_line() < 0) return(ERROR);		
	mkln(36,tst_string,2);

	/* payable address */
       	if(prnt_line() < 0) return(ERROR);		
       	if(prnt_line() < 0) return(ERROR);		
       	if(prnt_line() < 0) return(ERROR);		
       	if(prnt_line() < 0) return(ERROR);		
       	if(prnt_line() < 0) return(ERROR);		
       	if(prnt_line() < 0) return(ERROR);		
       	if(prnt_line() < 0) return(ERROR);	
       	if(prnt_line() < 0) return(ERROR);	
	mkln(3,tst_string,30);
       	if(prnt_line() < 0) return(ERROR);
	mkln(3,tst_string,30);
	mkln(45, tst_string, 10);	
       	if(prnt_line() < 0) return(ERROR);		
	mkln(3,tst_string,30);
       	if(prnt_line() < 0) return(ERROR);
	mkln(3,tst_string,30);
	if(prnt_line() < 0) return(ERROR);
	mkln(3,tst_string,30);
	mkln(45,tst_string, 10);  
	mkln(60, tst_string, 10);	
	mkln(71,tst_string, 10);  
       	if(prnt_line() < 0) return(ERROR);
       	if(prnt_line() < 0) return(ERROR);
       	if(prnt_line() < 0) return(ERROR);
       	if(prnt_line() < 0) return(ERROR);
       	if(prnt_line() < 0) return(ERROR);
	for( ;linecnt < ITEMLINES; ) {
		mkln(1,tst_string,10); 
		mkln(12,tst_string,10); 
		mkln(23,tst_string,10);
		mkln(34,tst_string,10);
		mkln(45,tst_string,10);
		mkln(55,"-",1);	
		mkln(60,tst_string,10); 
		mkln(71,tst_string,10);
		mkln(81,"-",1);	
       		if(prnt_line() < 0) return(ERROR);
	}
       	if(prnt_line() < 0) return(ERROR);
       	if(prnt_line() < 0) return(ERROR);
	mkln(1,tst_string,10);		
	mkln(12,tst_string,10);		
	mkln(23,tst_string,10);		
	mkln(34,tst_string,10);		
	mkln(45,tst_string,10);		
	mkln(55,"-",1);	
	mkln(71,tst_string,10);		
	mkln(81,"-",1);	
      	if(prnt_line() < 0) return(ERROR);
      	if(prnt_line() < 0) return(ERROR);
       	if(prnt_line() < 0) return(ERROR);
	return(NOERROR);

}

/* ------------------------------------------------------------------------ */
/* Function: Print Customer statement					    */
/* ------------------------------------------------------------------------ */
static
print_cust()
{
	int 	retval;
	int 	first_invc;	/* first invoice for customer flag */
	int 	invoice_complete; /* flag to see if complete invoice's
				     should be printed */
	long	prev_invc;
	int	rcptexist;	/* flag to see if receipt is for invoice */
	int	daysinsys;	/* number of days invoice in system */

        strcpy(cu_rec.cu_code, cust1); 
	flg_reset( CUSTOMER);

	for( ; ; ) {
		retval = get_n_cust(&cu_rec,BROWSE,0,FORWARD,e_mesg);
		if( retval < 0 ){
			if(retval == EFL) break;
			DisplayError(e_mesg);
			break;
		}
		if (strcmp(cu_rec.cu_code,cust2) > 0) break;
		if (strcmp(cu_rec.cu_code,cust1) < 0) continue;
		if (cu_rec.cu_prnt_cd[0] == 'B' && cu_rec.cu_cur_bal == 0)
			continue;
		if (cu_rec.cu_prnt_cd[0] == 'C' && cu_rec.cu_cur_bal < 0)
			continue;
		if (cu_rec.cu_prnt_cd[0] == 'D' && cu_rec.cu_cur_bal <= 0)
			continue;

		strcpy(arhdr_rec.ah_cu_code, cu_rec.cu_code);
		arhdr_rec.ah_inv_no = 0;
	    	arhdr_rec.ah_fund = 0;
	    	arhdr_rec.ah_sno = 0;
		flg_reset(ARSHDR);
	
		first_invc = 0;
		invoice_complete = 0;
		for( ; ; ) {
			retval=get_n_arhdr(&arhdr_rec,BROWSE,1,FORWARD,e_mesg);
			if(retval < 0) {
				if (retval == EFL) break;
				DisplayError(e_mesg);
				break;
			}
			
			/* if invoice is complete and more than 31 days old */
			/* skip invoice. */
			daysinsys = days(get_date())-days(arhdr_rec.ah_trandt);
			if(arhdr_rec.ah_status[0]==COMPLETE && daysinsys>31) {
				invoice_complete = 1;
				continue;
			}

			/* if invoice for Applied DM/CM is complete */
			/* Skip DM/CM. */
			if(arhdr_rec.ah_sno!=1 && invoice_complete == 1) 
				continue;

			if(first_invc == 0 ) 
				prev_invc = arhdr_rec.ah_inv_no;

			/* print any applied receipts */
			if(prev_invc != arhdr_rec.ah_inv_no && first_invc!=0) {
				if(CheckForRcpt(&rcptexist,cu_rec.cu_code,
				   prev_invc)<0)
					return(ERROR);
			}

			if(strcmp(arhdr_rec.ah_cu_code, cu_rec.cu_code) != 0) 
				break;

			/* used for Applied DM/CM */
			if(arhdr_rec.ah_netamt == 0.00) {
				arhdr_rec.ah_netamt = arhdr_rec.ah_oriamt;
			}

			if(linecnt==0) 
				if(print_header()<0) return(ERROR);

			if(linecnt >= ITEMLINES) {
				if(print_total(0)<0) return(ERROR);
				if(rite_top()<0) return(ERROR);
				if(print_header(0)<0) return(ERROR);
			}
				
			if(prnt_item_line(0) < 0) return(ERROR);
			prev_invc = arhdr_rec.ah_inv_no;
			first_invc = 1;	
			invoice_complete = 0;
		}

		if(retval == EFL) {
			if(CheckForRcpt(&rcptexist,cu_rec.cu_code,prev_invc)<0)
				return(ERROR);
		}

		if(GetUnappliedRcpt()<0) return(ERROR);

		if(linecnt != 0) {
			if(print_total(1)<0) return(ERROR);
			if(rite_top()<0) return(ERROR);
		}
	}
	seq_over( CUSTOMER );
	return(NOERROR);
}
/* ------------------------------------------------------------------------ */
/* Function: Check to see if receipt applied to this invoice 	            */
/* ------------------------------------------------------------------------ */
CheckForRcpt(exists,customer,invcnbr)
int	*exists;
char	customer[11];
long	invcnbr;
{
	int	err;

	/* Assume receipt does not exist until it finds it */
	*exists = 0;

	strcpy(rcpt_item.ritm_cust,customer);
	rcpt_item.ritm_invnumb = invcnbr;
	rcpt_item.ritm_refno = 0;
	rcpt_item.ritm_seqno = 0;
	flg_reset(RCPTITEM);
	for( ; ; ) {
		err = get_n_rcptitem(&rcpt_item,BROWSE,1,FORWARD,e_mesg);
		if(err == EFL) break;
		if(err < 0) {
			fomer(e_mesg); get();
			return(DBH_ERR);
		}

		if(strcmp(rcpt_item.ritm_cust,customer) != 0 ||
		   rcpt_item.ritm_invnumb != invcnbr) break;

		if(rcpt_item.ritm_invnumb == invcnbr)
			*exists = 1;

		if(*exists == 1) {
			rcpt_hdr.rhdr_refno = rcpt_item.ritm_refno;
			err = get_rcpthdr(&rcpt_hdr,BROWSE,0,e_mesg);
			if(err < 0) {
				fomer(e_mesg); get();
				return(DBH_ERR);
			}

			if(prnt_item_line(1) < 0) 
				return(ERROR);
		}

	}
	seq_over(RCPTITEM);
	return(NOERROR);
}
/* ------------------------------------------------------------------------ */
/* Function: Print Invoice Header Part					    */
/* ------------------------------------------------------------------------ */
GetUnappliedRcpt()
{
	int err;

	strcpy(rcpt_hdr.rhdr_cust,cu_rec.cu_code);
	rcpt_hdr.rhdr_fund = 0;
	rcpt_hdr.rhdr_refno = 0;
	flg_reset(RCPTHDR);
	for( ; ; ) {
		err = get_n_rcpthdr(&rcpt_hdr,BROWSE,2,FORWARD,e_mesg);
		if(err == EFL) break;
		if(err < 0) {
			fomer(e_mesg); get();
			return(DBH_ERR);
		}
		if(strcmp(rcpt_hdr.rhdr_cust,cu_rec.cu_code)!=0) break;
		if(rcpt_hdr.rhdr_applied[0] == APPLIED) continue;

			if(linecnt >= ITEMLINES){
				if(print_total(0)<0) return(ERROR);
				if(rite_top()<0) return(ERROR);
				if(print_header(0)<0) return(ERROR);
			}
			if (linecnt==0)
				if(print_header(0)<0) return(ERROR);

		rcpt_item.ritm_refno = rcpt_hdr.rhdr_refno;
		rcpt_item.ritm_seqno = 0;
		flg_reset(RCPTITEM);
		for( ; ; ) {
			err=get_n_rcptitem(&rcpt_item,BROWSE,0,FORWARD,e_mesg);
			if(err == EFL) break;
			if(err < 0) {
				fomer(e_mesg); get();
				return(DBH_ERR);
			}

			if(rcpt_item.ritm_refno != rcpt_hdr.rhdr_refno) break;
			rcpt_hdr.rhdr_amount -= rcpt_item.ritm_amount;
		}
		
		if(prnt_item_line(2) < 0) return(ERROR);
	}
	return(NOERROR);
}
/* ------------------------------------------------------------------------ */
/* Function: Print Invoice Header Part					    */
/* ------------------------------------------------------------------------ */
static
print_header()
{
	long longdate;
	char txt_buffer[12];
	int len,j;
	int curr_line = 1;

       	if(prnt_line() < 0) return(ERROR);		
	mkln(36,dist_no,2);

	/* payable address */
       	if(prnt_line() < 0) return(ERROR);		
       	if(prnt_line() < 0) return(ERROR);		
       	if(prnt_line() < 0) return(ERROR);		
       	if(prnt_line() < 0) return(ERROR);		
       	if(prnt_line() < 0) return(ERROR);		
       	if(prnt_line() < 0) return(ERROR);		
       	if(prnt_line() < 0) return(ERROR);	
       	if(prnt_line() < 0) return(ERROR);	
	mkln(3,cu_rec.cu_name,30);
       	if(prnt_line() < 0) return(ERROR);

	longdate = get_date();
	tedit((char *)&longdate, "____/__/__", txt_buffer, R_LONG);

		/* Check each line for NULL before printing to prevent 
		   blank lines from printing (CL)			*/
	
	if (strcmp(cu_rec.cu_adr1,"\0")) {
		curr_line++;
		mkln(3,cu_rec.cu_adr1,30);
		if (curr_line == 2)	
			mkln(45, txt_buffer, 10);	

	       	if(prnt_line() < 0) return(ERROR);		
	}

	if (strcmp(cu_rec.cu_adr2,"\0")) {
		curr_line++;
		mkln(3,cu_rec.cu_adr2,30);
		if (curr_line == 2)	
			mkln(45, txt_buffer, 10);	
       
		if(prnt_line() < 0) return(ERROR);
	}

	if (strcmp(cu_rec.cu_adr3,"\0")) {
		curr_line++;
		mkln(3,cu_rec.cu_adr3,30);
		if (curr_line == 2)	
			mkln(45, txt_buffer, 10);	

		if(prnt_line() < 0) return(ERROR);
	}

	if (strcmp(cu_rec.cu_pc,"\0")) {
	 	curr_line++;
		mkln(3,cu_rec.cu_pc,8);
		if (curr_line == 2)	
			mkln(45, txt_buffer, 10);	
		
		if( curr_line == 5){		
			mkln(45,cu_rec.cu_code, 10);  
			mkln(60, txt_buffer, 10);	
			mkln(72,cu_rec.cu_code, 10);  
		}
       		if(prnt_line() < 0) return(ERROR);
	}
	
	for(; curr_line < 9; ) {
		curr_line++;
		if (curr_line == 2)	
			mkln(45, txt_buffer, 10);	
		if (curr_line == 5){		
			mkln(45,cu_rec.cu_code, 10);  
			mkln(60, txt_buffer, 10);	
			mkln(72,cu_rec.cu_code, 10);  
		}
		if(prnt_line() < 0) return(ERROR);
	}	

	return(NOERROR);
}
/* ------------------------------------------------------------------------ */
/* Function: Print Statement Part					    */
/* ------------------------------------------------------------------------ */
static
prnt_item_line(mode)
int	mode;		/* 0 - IN/DM/CM   1 - Applied Recipt  2 - unapplied */
{
	char txt_buff[12];
	int 	i;	 
	double	negative;	/* used to show CM/Recipts as negatives */
				/* on remittance slip */
	switch (mode) {

	case 0:
       	    	tedit((char *)&arhdr_rec.ah_trandt,"____/__/__",txt_buff,R_LONG);
	     	mkln(1,txt_buff,10);
	     	tedit((char *)&arhdr_rec.ah_inv_no,"______0_",txt_buff,R_LONG);
		if (!strcmp(arhdr_rec.ah_type,CRMEMO)){
			strcpy(ref_no,CRMEMO);
			strcat(ref_no, txt_buff);

			mkln(12,ref_no,10); 
			tedit((char *)&arhdr_rec.ah_netamt,"___,_0_.__-",txt_buff,R_DOUBLE);
			mkln(34,txt_buff,11);
			balance = balance -arhdr_rec.ah_netamt;
			tedit((char *)&balance,"___,_0_.__-",txt_buff,R_DOUBLE);
			mkln(45,txt_buff,11);

			mkln(60,ref_no,10); 
			negative = arhdr_rec.ah_netamt * -1;
			tedit((char *)&negative,"___,_0_.__-",txt_buff,R_DOUBLE);
			mkln(71,txt_buff,11);

			total_credit= total_credit + arhdr_rec.ah_netamt;
			if (due_date_calc(1) < 0 ) return (ERROR);
       			if(prnt_line() < 0) return(ERROR);
		}
		if (!strcmp(arhdr_rec.ah_type,INVOICE)){
			strcpy(ref_no,INVOICE);
			strcat(ref_no,txt_buff);

			mkln(12,ref_no,10); 
			tedit((char *)&arhdr_rec.ah_netamt,"___,_0_.__-",txt_buff,R_DOUBLE);
			mkln(23,txt_buff,11);
			balance = balance + arhdr_rec.ah_netamt;
			tedit((char *)&balance,"___,_0_.__-",txt_buff,R_DOUBLE);
			mkln(45,txt_buff,11);

			mkln(60,ref_no,10); 
			tedit((char *)&arhdr_rec.ah_netamt,"___,_0_.__-",txt_buff,R_DOUBLE);
			mkln(71,txt_buff,11);

			if (due_date_calc(0) < 0 ) return (ERROR);
			total_debit = total_debit +arhdr_rec.ah_netamt;
       			if(prnt_line() < 0) return(ERROR);
		}
		if (!strcmp(arhdr_rec.ah_type,DBMEMO)){
			strcpy(ref_no,DBMEMO);
			strcat(ref_no, txt_buff);
			mkln(12,ref_no,10); 
			tedit((char *)&arhdr_rec.ah_netamt,"___,_0_.__-",txt_buff,R_DOUBLE);
			mkln(23,txt_buff,11);
			balance = balance + arhdr_rec.ah_netamt;
			tedit((char *)&balance,"___,_0_.__-",txt_buff,R_DOUBLE);
			mkln(45,txt_buff,11);

			mkln(60,ref_no,10); 
			tedit((char *)&arhdr_rec.ah_netamt,"___,_0_.__-",txt_buff,R_DOUBLE);
			mkln(71,txt_buff,11);
			total_debit = total_debit +arhdr_rec.ah_netamt;
			if (due_date_calc(0) < 0 ) return (ERROR);
       			if(prnt_line() < 0) return(ERROR);
		}
		break;
	case 1:
		strcpy(ref_no,APPLY_STAT);

       	    	tedit((char *)&rcpt_hdr.rhdr_rcptdate,"____/__/__",txt_buff,R_LONG);
	     	mkln(1,txt_buff,10);
	     	tedit((char *)&rcpt_item.ritm_refno,"______0_",txt_buff,R_LONG);
		strcat(ref_no, txt_buff);
		mkln(12,ref_no,10); 
		tedit((char *)&rcpt_item.ritm_amount,"___,_0_.__-",txt_buff,R_DOUBLE);
		mkln(34,txt_buff,11);
		balance = balance -rcpt_item.ritm_amount;
		tedit((char *)&balance,"___,_0_.__-",txt_buff,R_DOUBLE);
		mkln(45,txt_buff,11);
		mkln(60,ref_no,10); 
		negative=rcpt_item.ritm_amount * -1;
		tedit((char *)&negative,"___,_0_.__-",txt_buff,R_DOUBLE);
		mkln(71,txt_buff,11);
		if (due_date_calc(2) < 0) return(ERROR);
       		if(prnt_line() < 0) return(ERROR);
		break;
	case 2:
		strcpy(ref_no,UNAPP_STAT);

       	    	tedit((char *)&rcpt_hdr.rhdr_rcptdate,"____/__/__",txt_buff,R_LONG);
	     	mkln(1,txt_buff,10);
	     	tedit((char *)&rcpt_hdr.rhdr_refno,"______0_",txt_buff,R_LONG);
		strcat(ref_no, txt_buff);
		mkln(12,ref_no,10); 
		tedit((char *)&rcpt_hdr.rhdr_amount,"___,_0_.__-",txt_buff,R_DOUBLE);
		mkln(34,txt_buff,11);
		balance = balance -rcpt_hdr.rhdr_amount;
		tedit((char *)&balance,"___,_0_.__-",txt_buff,R_DOUBLE);
		mkln(45,txt_buff,11);
		mkln(60,ref_no,10); 
		negative=rcpt_hdr.rhdr_amount * -1;
		tedit((char *)&negative,"___,_0_.__-",txt_buff,R_DOUBLE);
		mkln(71,txt_buff,11);
		if (due_date_calc(3) < 0) return(ERROR);
       		if(prnt_line() < 0) return(ERROR);
		break;

	}		
	return(NOERROR);
}
/* ------------------------------------------------------------------------ */
/* Function: Print Statement Total Part					    */
/* ------------------------------------------------------------------------ */
static
print_total(mode)
int	mode;	/* 0 - if multiple pages,  1 - if last page */
{
	int i=0;
	char txt_buffer[20];

	for( ;linecnt < 40; ) {
		prnt_line();	
	}
	

	if(mode == 1) {
		tedit((char *)&due_90,"___,_0_.__-",txt_buffer,R_DOUBLE);
		mkln(1,txt_buffer,11);		
		tedit((char *)&due_60,"___,_0_.__-",txt_buffer,R_DOUBLE);
		mkln(12,txt_buffer,11);		
		tedit((char *)&due_30,"___,_0_.__-",txt_buffer,R_DOUBLE);
		mkln(23,txt_buffer,11);		
		tedit((char *)&due_0,"___,_0_.__-",txt_buffer,R_DOUBLE);
		mkln(34,txt_buffer,11);		
		tedit((char *)&balance,"___,_0_.__-",txt_buffer,R_DOUBLE);
		mkln(45,txt_buffer,11);		

		mkln(71,txt_buffer,11);		

        	if(prnt_line() < 0) return(ERROR);
        	if(prnt_line() < 0) return(ERROR);
        	if(prnt_line() < 0) return(ERROR);
 		total_debit = 0; 
		total_credit = 0;
		balance = 0;
		due_90 = 0;
		due_60 = 0;
		due_30 = 0;
		due_0  = 0;		 
	}
	return(NOERROR);	
}
/* ------------------------------------------------------------------------ */
/* Function: due payment calculation					    */
/* ------------------------------------------------------------------------ */
static 
due_date_calc(mode) 
int 	mode;		/* 0 - IN/DM   1 - CM 
			   2 - Applied rcpt  3 - unapplied rcpt */
{
	int 	number_days, type;
	double  temp_amt= 0;


	switch(mode) {
	case 0:
	case 1:
		number_days = days(get_date()) - days(arhdr_rec.ah_duedt);
		temp_amt = arhdr_rec.ah_netamt;
		break;
	case 2:
		number_days = days(get_date()) -days(rcpt_hdr.rhdr_rcptdate);
		temp_amt = rcpt_item.ritm_amount;
		break;
	case 3:
		number_days = days(get_date()) -days(rcpt_hdr.rhdr_rcptdate);
		temp_amt = rcpt_hdr.rhdr_amount;
		break;
	}

	type = number_days/30;
	if(type < 0) type = 0;  /* if due in future make due current */
	switch(type) {
	case 0:
		if (mode >  0)
			due_0 = due_0 -temp_amt;
		else
			
			due_0 = due_0 + temp_amt;
		break;
	case 1:
		if (mode >  0)
			due_30 = due_30 -temp_amt;
		else
			
			due_30 = due_30 + temp_amt;
		break;
	case 2:
		if (mode >  0)
			due_60 = due_60 -temp_amt;
		else
			
			due_60 = due_60 + temp_amt;
		break;
	default: 
		if (mode >  0)
			due_90 = due_90 -temp_amt;
		else
			
			due_90 = due_90 + temp_amt;
		break;
	}
	return(NOERROR);
}
/* ------------------------------------------------------------------------ */
/* Function: Display Error Message   					    */
/* ------------------------------------------------------------------------ */
static
DisplayError(error)
char *error;
{
	fomer(error);
	roll_back(error);
}

/*-------------------------- END OF FILE ---------------------------------- */
