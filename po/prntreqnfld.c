/* --------------------------------------------------------------------------
	SOURCE NAME:  PRINTREQ.C
	SYSTEM     :  BUGETARY FINANCIAL SYSTEM 
	MODULE     :  PURCHASE ORDER MODULE
	CREATED ON :  6 NOV. 1991
	CREATED BY :  Jonathan Prescott

DESCRIPTION:
	This program prints a range of specified Requisitions.
	It allows for the user to do a test print to allign the forms.

MODIFICATIONS:

PROGRAMMER           YY/MM/DD        DESCRIPTION OF MODIFICATIONS
J.Prescott	     91/11/26	    Copied From prntpo.c

L.Robichaud	1993/10/13	Pass new parameter to close_rep function for 
				a banner to print with 3000 family.
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <repdef.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#ifdef ENGLISH
#define	ENGLISH_MSG	'E'
#define	FRENCH_MSG	'F'
#define YES	'Y'
#define NO	'N'
#define OUTSTANDING	'O'

#define TAXABLE	'T'
#define EXEMPT	'E'
#define FREIGHT 'F'
#else
#define	ENGLISH_MSG	'A'
#define	FRENCH_MSG	'F'
#define YES	'O'
#define NO	'N'

#define OUTSTANDING	'O'

#define TAXABLE	'T'
#define EXEMPT	'E'
#define FREIGHT 'F'
#endif

#define  EXIT  12

#define  TAB 2       	    /* this is the offset to start all header lines */
#define  TOTAL_POS 74       /* was 72 Beginning position  of Total amount        */

Pa_rec   pa_rec;             /*  Declarations for DBH record reading */
Supplier supp_rec;
Sch_rec  sch_rec;
Req_hdr  reqhdr_rec;
Req_item reqitem_rec;
Ctl_rec	 ctl_rec;
Tax_cal	 tax_cal;

char	 answer[2];
int 	 retval, i, ii;
char	 printer[3];
char	 tnum[5];
int	 printer_nbr;
char	 output[2];
char	 filename[12];
long	 reqnbr1, reqnbr2;
char     program[11];
double	 gst_tax;
double	 pst_tax;
double	 total_freight;
double   order_total,sub_total,req_total;
short	 gst_tax_rate, pst_tax_rate;
char 	 contact_name[26];
char 	 req_type[1]; 
extern char 	 e_mesg[80];

typedef struct {
	char	add1[31];
	char	add2[31];
	char	add3[31];
	char	postal_cd[11];
} Address;
Address	address[2];
int	address_line;

prntreq()
{
	LNSZ = 132;
	order_total = sub_total = req_total = 0;

	STRCPY(program,"PRNTREQ");
	strcpy(printer,"1");
	if((retval = GetPrinter( printer ))<0)
		return(retval);
	reqnbr1 = 0;
	reqnbr2 = 99999999;
	retval = GetReqRange( &reqnbr1, &reqnbr2 );
	if(retval < 0) return(retval);

	printer_nbr = atoi(printer);
	if(printer_nbr == 0) {
		output[0] = 'F';
		get_tnum(tnum);
		sprintf(filename,"reqform%s",tnum);
	}
	else {
		output[0] = 'P';
		filename[0] = '\0';
	}

	retval = opn_prnt(output,filename,printer_nbr,e_mesg,0);  
	if(retval < 0) {
		return(REPORT_ERR);
	}

        for( ; ; ) {
#ifdef ENGLISH
		if((retval=DisplayMessage("Is Form Aligned (Y/N)?"))<0)
#else
		if((retval=DisplayMessage("Formulaire est-il aligne (O/N)?"))<0)
#endif
			break;	
		if((retval = GetResponse(answer)) <0)
			break;
		if(answer[0] == YES) break;
		retval = align_req();
		if(retval < 0) break;

		rite_top();

		if(printer_nbr==0) {
			close_rep(NOBANNER);
/*
			retval = Send_PC(filename,"LST:",e_mesg);
*/
			redraw();
		}
		if(retval != NOERROR) break;
	}
	if(retval == NOERROR) {
		if(( retval = Confirm()) <= 0)
			return(retval);

		retval = get_param(&pa_rec,BROWSE,1,e_mesg);
        	if(retval < 1) {
			retval = DBH_ERR;
		}
		if(retval != DBH_ERR) {
			if(printer_nbr == 0) {
				retval = opn_prnt(output,filename,printer_nbr,
						e_mesg,0);  
				if(retval < 0) {
					return(REPORT_ERR);
				}
			}
			retval = print_req();
			if(printer_nbr==0) {
				close_rep(NOBANNER);
/*
				retval = Send_PC(filename,"LST:",e_mesg);
*/
				redraw();
			}
		}
	}

	if(printer_nbr != 0) {
		close_rep(NOBANNER);
	}
	close_dbh();
	return(retval);
}

/* -----------------------  END OF PRNTPO --------------------------------- */
/* ---------------------------------------------------------------------------
	DEFINITION:  prints (*) in a po form to test for alignment.
--------------------------------------------------------------------------- */
align_req()
{
        int cnt;
        static char tst_string[] = "****************************************";

        if(prnt_line() < 0) return(REPORT_ERR);		/* line 1 */
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 2 */
	mkln(TAB+63,tst_string,8);
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 3 */
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 4 */
	mkln(TAB+63,tst_string,8);
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 5 */
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 6 */
	mkln(TAB+59,"****/**/**",10);
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 7 */
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 9 */
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 9 */

	mkln(TAB+8,"***-***-****",12);
	mkln(TAB+26,"***-***-****",12);

	mkln(TAB+47,"****/**/**",10);
	mkln(TAB+64,"****/**/**",10);
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 10 */
	mkln(TAB+15,tst_string,10);
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 11 */
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 12 */
	mkln(TAB+4,tst_string,44);
	mkln(TAB+46,tst_string,30);
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 13 */
	mkln(TAB+4,tst_string,30);
	mkln(TAB+46,tst_string,30);
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 14 */
	mkln(TAB+4,tst_string,30);
	mkln(TAB+46,tst_string,30);
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 15 */
	mkln(TAB+4,tst_string,30);
	mkln(TAB+46,tst_string,30);
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 16 */
	mkln(TAB+4,tst_string,10);
	mkln(TAB+46,tst_string,10);
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 17 */
	mkln(TAB+11,"***-***-****",12);
	mkln(TAB+57,"***-***-****",12);
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 18 */
	mkln(TAB+11,"***-***-****",12);
	mkln(TAB+57,"***-***-****",12);
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 19 */
	mkln(TAB+58,tst_string,15);
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 20 */
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 21 */
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 22 */
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 23 */

	for(cnt = 0;cnt < 27; cnt++) {
		mkln(1,"****.**",7); 
		mkln(8,tst_string,2); 
		mkln(10,tst_string,1); 
		mkln(12,tst_string,1); 
               	mkln(13,tst_string,11);
               	mkln(25,tst_string,31);
               	mkln(56,tst_string,6);
		mkln(65,"*****.**",8); 
		mkln(TOTAL_POS,"****,***.**",11); 
        	if(prnt_line() < 0) return(REPORT_ERR);	/* line 24 - 50 */
	}

       	if(prnt_line() < 0) return(REPORT_ERR);	/* line 51 */
	mkln(TOTAL_POS-1,"*,***,***.**",12); 
       	if(prnt_line() < 0) return(REPORT_ERR);	/* line 52 */
       	if(prnt_line() < 0) return(REPORT_ERR);	/* line 53 */
	mkln(TOTAL_POS-1,"*,***,***.**",12); 
       	if(prnt_line() < 0) return(REPORT_ERR);	/* line 54 */
       	if(prnt_line() < 0) return(REPORT_ERR);	/* line 55 */
	mkln(TOTAL_POS-1,"*,***,***.**",12); 
       	if(prnt_line() < 0) return(REPORT_ERR);	/* line 56 */
       	if(prnt_line() < 0) return(REPORT_ERR);	/* line 57 */
	mkln(TOTAL_POS-1,"*,***,***.**",12); 
       	if(prnt_line() < 0) return(REPORT_ERR);	/* line 58 */

	return(NOERROR);
}
/* ------------------------------------------------------------------------ */
print_req()
{
	char txt[80];
	short 	prev_fund;

	reqhdr_rec.code = reqnbr1;

	flg_reset( REQHDR );
	for( ; ; ) {
		retval = get_n_reqhdr(&reqhdr_rec,UPDATE,0,FORWARD,e_mesg);
		if( retval < 0 ){
			if(retval == EFL) break;
			DisplayError(e_mesg);
			break;
		}
		if(reqhdr_rec.code > reqnbr2){
			roll_back(e_mesg);
			break;
		}
		if(reqhdr_rec.status[0] != OUTSTANDING){
			roll_back(e_mesg);
			continue;
		}
		if(reqhdr_rec.print_form[0] == YES){
			if(reqnbr1 != reqnbr2){
#ifndef	ORACLE
				roll_back(e_mesg);
#endif
		 		continue;
			}
			else{
#ifdef ENGLISH
				sprintf(e_mesg,"Req#: %ld already printed. Do you want to print it again (Y/N)?",reqhdr_rec.code);
#else
				sprintf(e_mesg,"BC#: %ld deja imprime. Desirez-vous le reimprimer (O/N)?",reqhdr_rec.code);
#endif
				DisplayMessage(e_mesg);
				GetResponse(e_mesg);
				if(e_mesg[0] == NO) {
#ifndef	ORACLE
					roll_back(e_mesg);
#endif
					continue;
				}
			}
		}

		if(reqhdr_rec.code >= 90000000) /* don't print Manual Requ */
			break;

		if (prev_fund != reqhdr_rec.funds) {
			ctl_rec.fund 	= reqhdr_rec.funds;
			retval = get_ctl(&ctl_rec,BROWSE,0,e_mesg);
			if( retval < 0 ){
				DisplayError(e_mesg);
				break;
			}
			gst_tax_rate	= ctl_rec.gst_tax;	
			pst_tax_rate	= ctl_rec.pst_tax;
			prev_fund	= ctl_rec.fund; 
		}	

		if(print_header() == ERROR) return(ERROR);

		reqitem_rec.code = reqhdr_rec.code;
		reqitem_rec.item_no = 0;
		flg_reset( REQITEM );
		for( ; ; ) {
#ifndef ORACLE
			retval = get_n_reqitem(&reqitem_rec,BROWSE,0,FORWARD,
								e_mesg);
#else
			retval = get_n_reqitem(&reqitem_rec,BROWSE,0,EQUAL,
								e_mesg);
#endif
			if( retval < 0 ){
				if(retval == EFL) break;
				DisplayError(e_mesg);
				break;
			}
#ifndef ORACLE
			if(reqhdr_rec.code != reqitem_rec.code) {
				break;
			}
#endif
			if((strlen(reqitem_rec.desc)-1 > 30 && linecnt >= 48)
							|| linecnt > 49) { 
				print_total(0);
				rite_top();
				if(print_header() == ERROR) return(ERROR);
			}
			if(reqitem_rec.tax2[0] == FREIGHT) {
				total_freight += reqitem_rec.value;
				continue;
			}
			if(print_item_line() == ERROR) return(ERROR);
		}
		seq_over( REQITEM );
		if(retval == ERROR) break;
		print_total(1);
		rite_top();
		reqhdr_rec.print_form[0] = YES;
		retval = put_reqhdr(&reqhdr_rec,UPDATE,e_mesg);
		if(retval < 0) {
			DisplayError(e_mesg);
			break;
		}
		if(commit(e_mesg) < 0) {
			DisplayError(e_mesg);
			break;
		}
		reqhdr_rec.code++;
		order_total = sub_total = req_total = 0;
		pst_tax = 0;
		total_freight=0;
		gst_tax = 0;
	}
	seq_over( REQHDR );
	return(NOERROR);
}

print_header()
{
	long longdate;
	char txt_buffer[11];
	int len,j;

       	if(prnt_line() < 0) return(ERROR);		/* line 1 */
       	if(prnt_line() < 0) return(ERROR);		/* line 2 */
       	if(prnt_line() < 0) return(ERROR);		/* line 2 */
	tedit((char *)&reqhdr_rec.code,"________0_",txt_buffer,R_LONG);
	mkln(TAB+63,txt_buffer,10);
       	if(prnt_line() < 0) return(ERROR);		/* line 3 */
       	if(prnt_line() < 0) return(ERROR);		/* line 4 */
       	if(prnt_line() < 0) return(ERROR);		/* line 6 */
       	if(prnt_line() < 0) return(ERROR);		/* line 5 */

	/* bill to address */
	sch_rec.sc_numb = reqhdr_rec.billto;
	retval = get_sch(&sch_rec,BROWSE,0,e_mesg);
	if( retval < 0 ){
		DisplayError(e_mesg);
		return(-1);
	}

       	if(prnt_line() < 0) return(ERROR);		/* line 8 */
       	if(prnt_line() < 0) return(ERROR);		/* line 8 */
       	if(prnt_line() < 0) return(ERROR);		/* line 8 */

	mkln(TAB+1,"PHONE:",6);		/* bill to phone number */
	mkln(TAB+8,sch_rec.sc_phone,3);	/* bill to phone number */
	mkln(TAB+11,"-",1);			/* with dashes */
	mkln(TAB+12,sch_rec.sc_phone+3,3);
	mkln(TAB+15,"-",1);
	mkln(TAB+16,sch_rec.sc_phone+6,4);

	mkln(TAB+21,"FAX:",4);		/* bill to fax number */
	mkln(TAB+26,sch_rec.sc_fax,3);	/* bill to fax number */
	mkln(TAB+29,"-",1);			/* with dashes */
	mkln(TAB+30,sch_rec.sc_fax+3,3);
	mkln(TAB+33,"-",1);
	mkln(TAB+34,sch_rec.sc_fax+6,4);

	tedit((char *)&reqhdr_rec.date,"____/__/__",txt_buffer,R_LONG);
	mkln(TAB+46,txt_buffer,10);
	tedit((char *)&reqhdr_rec.due_date,"____/__/__",txt_buffer,R_LONG);
	mkln(TAB+63,txt_buffer,10);
       	if(prnt_line() < 0) return(ERROR);		/* line 10 */


	/* format supplier name & address */
	STRCPY(supp_rec.s_supp_cd,reqhdr_rec.supp_cd);
	retval = get_supplier(&supp_rec,BROWSE,0,e_mesg);
	if( retval < 0 ){
		DisplayError(e_mesg);
		return(retval);
	}

	/* Initialize address lines */
	address[0].add1[0] = '\0';
	address[0].add2[0] = '\0';
	address[0].add3[0] = '\0';
	address[0].postal_cd[0] = '\0';
	address_line = 0;

	if(supp_rec.s_add1[0] != '\0') {
		strcpy(address[0].add1,supp_rec.s_add1);	
		address_line++;
	}
	if(supp_rec.s_add2[0] != '\0') {
		if(address_line == 0) 
			strcpy(address[0].add1,supp_rec.s_add2);	
		else 
			strcpy(address[0].add2,supp_rec.s_add2);	
		address_line++;
	}
	if(supp_rec.s_add3[0] != '\0') {
		if(address_line == 0) 
			strcpy(address[0].add1,supp_rec.s_add3);	
		else if(address_line == 1) 
			strcpy(address[0].add2,supp_rec.s_add3);	
		else 
			strcpy(address[0].add3,supp_rec.s_add3);	
		address_line++;
	}
	if(supp_rec.s_pc[0] != '\0') {
		if(address_line == 0) 
			strcpy(address[0].add1,supp_rec.s_pc);	
		else if(address_line == 1) 
			strcpy(address[0].add2,supp_rec.s_pc);	
		else if(address_line == 2)
			strcpy(address[0].add3,supp_rec.s_pc);	
		else 
			strcpy(address[0].postal_cd,supp_rec.s_pc);	
	}
	
	/* format school to ship to address */
	sch_rec.sc_numb = reqhdr_rec.shipto;
	retval = get_sch(&sch_rec,BROWSE,0,e_mesg);
	if( retval < 0 ){
		DisplayError(e_mesg);
		return(-1);
	}

	/* Initialize address lines */
	address[1].add1[0] = '\0';
	address[1].add2[0] = '\0';
	address[1].add3[0] = '\0';
	address[1].postal_cd[0] = '\0';
	address_line = 0;

	if(sch_rec.sc_add1[0] != '\0') {
		strcpy(address[1].add1,sch_rec.sc_add1);	
		address_line++;
	}
	if(sch_rec.sc_add2[0] != '\0') {
		if(address_line == 0) 
			strcpy(address[1].add1,sch_rec.sc_add2);	
		else 
			strcpy(address[1].add2,sch_rec.sc_add2);	
		address_line++;
	}
	if(sch_rec.sc_add3[0] != '\0') {
		if(address_line == 0) 
			strcpy(address[1].add1,sch_rec.sc_add3);	
		else if(address_line == 1) 
			strcpy(address[1].add2,sch_rec.sc_add3);	
		else 
			strcpy(address[1].add3,sch_rec.sc_add3);	
		address_line++;
	}
	if(sch_rec.sc_pc[0] != '\0') {
		if(address_line == 0) 
			strcpy(address[1].add1,sch_rec.sc_pc);	
		else if(address_line == 1) 
			strcpy(address[1].add2,sch_rec.sc_pc);	
		else if(address_line == 2)
			strcpy(address[1].add3,sch_rec.sc_pc);	
		else 
			strcpy(address[1].postal_cd,sch_rec.sc_pc);	
	}
	
	mkln(TAB+13,supp_rec.s_supp_cd,10);
	if(prnt_line() < 0) return(ERROR);	/* line 11 */
	if(prnt_line() < 0) return(ERROR);	/* line 12 */

	mkln(TAB+4,supp_rec.s_name,44);
	mkln(TAB+46,sch_rec.sc_name,28);
	if(prnt_line() < 0) return(ERROR);	/* line 13 */

	mkln(TAB+4,address[0].add1,30);
	mkln(TAB+46,address[1].add1,30);
	if(prnt_line() < 0) return(ERROR);	/* line 14 */
	mkln(TAB+4,address[0].add2,30);
	mkln(TAB+46,address[1].add2,30);
	if(prnt_line() < 0) return(ERROR);	/* line 15 */
	mkln(TAB+4,address[0].add3,30);
	mkln(TAB+46,address[1].add3,30);
	if(prnt_line() < 0) return(ERROR);	/* line 16 */
	mkln(TAB+4,address[0].postal_cd,10);
	mkln(TAB+46,address[1].postal_cd,10);
	if(prnt_line() < 0) return(ERROR);	/* line 17 */

	mkln(TAB+11,supp_rec.s_phone,3);	/* supplier to phone number */
	mkln(TAB+14,"-",1);			/* with dashes */
	mkln(TAB+15,supp_rec.s_phone+3,3);
	mkln(TAB+18,"-",1);
	mkln(TAB+19,supp_rec.s_phone+6,4);

	mkln(TAB+57,sch_rec.sc_phone,3);	/* Ship To to phone number */
	mkln(TAB+60,"-",1);			/* with dashes */
	mkln(TAB+61,sch_rec.sc_phone+3,3);
	mkln(TAB+64,"-",1);
	mkln(TAB+65,sch_rec.sc_phone+6,4);
       	if(prnt_line() < 0) return(ERROR);		/* line 18 */

	mkln(TAB+11,supp_rec.s_fax,3);	/* Supplier fax number */
	mkln(TAB+14,"-",1);			/* with dashes */
	mkln(TAB+15,supp_rec.s_fax+3,3);
	mkln(TAB+18,"-",1);
	mkln(TAB+19,supp_rec.s_fax+6,4);

	mkln(TAB+57,sch_rec.sc_fax,3);	/* bill to fax number */
	mkln(TAB+60,"-",1);			/* with dashes */
	mkln(TAB+61,sch_rec.sc_fax+3,3);
	mkln(TAB+64,"-",1);
	mkln(TAB+65,sch_rec.sc_fax+6,4);
       	if(prnt_line() < 0) return(ERROR);		/* line 19 */

	mkln(TAB+58,reqhdr_rec.attention,15);
       	if(prnt_line() < 0) return(ERROR);		/* line 20 */
       	if(prnt_line() < 0) return(ERROR);		/* line 21 */
       	if(prnt_line() < 0) return(ERROR);		/* line 22 */
       	if(prnt_line() < 0) return(ERROR);		/* line 23 */

	return(NOERROR);
}

print_item_line()
{
	char txt_line[11];
	short	tax1, tax2;
	int	str_pos;
	tax1 = 0; 
	tax2 = 0; 

	tedit((char *)&reqitem_rec.orig_qty,"__0_.__",txt_line,R_DOUBLE);
	mkln(1,txt_line,7);

	mkln(8,reqitem_rec.unit,2); 
	mkln(10,reqitem_rec.tax1,1);		/* print GST taxable always */
	mkln(12,reqitem_rec.tax2,1);		/* print PST taxable always */

	mkln(13,reqitem_rec.acct+7,11);

	/* subtract 1 extra because of null */
	for(str_pos=0;str_pos<=strlen(reqitem_rec.desc)-30; ) {
		for(i=29+str_pos;i>str_pos;i--) {
			if(reqitem_rec.desc[i] == ' ' )  {
				break;
			}
		}
		if(i == str_pos) {
			mkln(25,reqitem_rec.desc+str_pos,30);
        		if(prnt_line() < 0) return(ERROR);
			str_pos += 30;
		}
		else {
			mkln(25,reqitem_rec.desc+str_pos,i-(str_pos-1));
        		if(prnt_line() < 0) return(ERROR);
			i = i - (str_pos-1);
			str_pos += i;
		}	
	}
	mkln(25,reqitem_rec.desc+str_pos,strlen(reqitem_rec.desc)-str_pos);

/***
	if (req_type[0] != DIRECT){
        	if(prnt_line() < 0) return(ERROR); 
		mkln(13, "Stock #:", 9);
		mkln(24, reqitem_rec.st_code,10);
	}  
***/
	mkln(56, reqitem_rec.st_code,6);

	tedit((char *)&reqitem_rec.unitprice,"__,_0_.__",txt_line,R_DOUBLE);
	mkln(65,txt_line,9);
	
	if (reqitem_rec.tax1[0] == TAXABLE) 
		tax1 = gst_tax_rate;
	if (reqitem_rec.tax2[0] == TAXABLE)  
		tax2 = pst_tax_rate;	 

	Tax_Calculation(tax1, tax2, reqitem_rec.value, &tax_cal);
 	pst_tax		+= tax_cal.pst_amt;		
 	gst_tax 	+= tax_cal.gst_amt;		
	order_total	+= reqitem_rec.value; 
	tedit((char *)&reqitem_rec.value,"___,_0_.__",txt_line,R_DOUBLE);
	mkln(TOTAL_POS,txt_line,10);
        if(prnt_line() < 0) return(ERROR);		/* lines 22 - 42 */


	return(NOERROR);
}

print_total(mode)
int	mode;	/* 0 - if multiple pages,  1 - if last page */
{
	char txt_buffer[20];

	for( ;linecnt < 52; ) {
		prnt_line();		/* advance to sub-total */
	}

	tedit((char *)&order_total,"____,_0_.__",txt_buffer,R_DOUBLE);
	mkln(TOTAL_POS-1,txt_buffer,11);		/* sub total */
	prnt_line();	/* line 52 */
	prnt_line();	/* line 53 */

	tedit((char *)&gst_tax,"____,_0_.__",txt_buffer,R_DOUBLE);
	mkln(TOTAL_POS-1,txt_buffer,11);			
	prnt_line(); 	/* line 54 */
	prnt_line(); 	/* line 55 */

	tedit((char *)&pst_tax,"____,_0_.__",txt_buffer,R_DOUBLE);
	mkln(TOTAL_POS-1,txt_buffer,11);			/* P.S.T tax */
	prnt_line();	/* line 56 */
	prnt_line();	/* line 57 */

	sub_total += order_total + gst_tax + pst_tax;
	if(mode == 1) {
		req_total = sub_total; 
		tedit((char *)&req_total,"___,_0_.__",txt_buffer,R_DOUBLE);
		mkln(TOTAL_POS,txt_buffer,10);			/* total */
		prnt_line();
	}

	return(NOERROR);	
}

DisplayError(error)
char *error;
{
	fomer(error);
	roll_back(error);
}
/*-------------------------- END OF FILE ---------------------------------- */

