/* --------------------------------------------------------------------------
	SOURCE NAME:  PRINTPO.C
	SYSTEM     :  BUGETARY FINANCIAL SYSTEM 
	MODULE     :  PURCHASE ORDER MODULE
	CREATED ON :  25 SEPT. 1989
	CREATED BY :  Jonathan Prescott

DESCRIPTION:
	This program prints a range of specified purchase orders.
	It allows for the user to do a test print to allign the forms.

MODIFICATIONS:

PROGRAMMER           YY/MM/DD        DESCRIPTION OF MODIFICATIONS
J.PRESCOTT           90/05/14        Changed Format For New PO Form
F.TAO 		     90/11/20	     Changed Formats
F.TAO 		     90/11/30	     Add GST stuff s
C.Leadbeater         90/12/06	     Change so blank address lines are not 
				     printed.
C.Leadbeater	     90/12/20	     Add 2 additional places to items TOTAL
				     COST column and 1 place to UNIT COST
				     column.
F.Tao	             91/01/16	     Changed PO formats so that it begins
				     to print at column 1.
F.Tao 	             91/01/30	     Changed position for description.
J.PRESCOTT           91/02/05        now prints ASAP if due date and trans.
				     date is the same.
C.Leadbeater	     91/10/22	     Added code for Newfoundland to print
				     the requisition number from the first
				     po item at the top of each form. (SW1)
C.Leadbeater	     91/10/25	     Added code to print requistion number
				     from the po header for New Brunswick.
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
#define POPRNT	'P'
#define LABEL	'L'

#define TAXABLE	'T'
#define EXEMPT	'E'
#define FREIGHT 'F'
#else
#define	ENGLISH_MSG	'A'
#define	FRENCH_MSG	'F'
#define YES	'O'
#define NO	'N'

#define POPRNT	'I'
#define LABEL	'E'

#define TAXABLE	'T'
#define EXEMPT	'E'
#define FREIGHT 'F'
#endif

#define  EXIT  12

#define  SUPP_POS  6        /* beginning position  of supplier information */
#define	 NBRCHAR	28
#define  SCHL_POS  44       /* beginning position  of school information  */
#define  TOTAL_POS 70       /* Beginning position  of Total amount        */

Pa_rec   pa_rec;             /*  Declarations for DBH record reading */
Supplier supp_rec;
Sch_rec  sch_rec;
Po_hdr   pohdr_rec;
Po_item  poitem_rec, prev_poitem;
St_mast  stmast_rec;
Ctl_rec	 ctl_rec;
Tax_cal	 tax_cal;

char	 answer[2];
int 	 retval, i, ii;
short	 printer;
long	 ponbr1, ponbr2;
char     program[11];
double	 gst_tax;
double	 pst_tax;
double	 total_freight;
double   order_total,sub_total,po_total;
short	 gst_tax_rate, pst_tax_rate;
char 	 contact_name[26];
char 	 po_type[1]; 
extern char 	 e_mesg[80];
int	blank_line = 0;		/* used to make up for any blank 
				   address lines not printed	 */ 
static	char 	prev_tax1[2];
static	char	prev_tax2[2];
static	double	prev_qty;
static	double	prev_unitprice = 0;
static	double	prev_value = 0;

static	long	po_req;	/* used to store the requisition number from
				the first po item for printing at
				the top of each po. (FOR NFLD - SW1) */

char	po_req_no[sizeof(pohdr_rec.ph_req_no)]; /* same as po_req above but
							for NB */

char	item_desc[250];
long	len;
long	lines;
static	short	qty_printed;
prntpo()
{
	LNSZ = 132;
	order_total = sub_total = po_total = 0;

	STRCPY(program,"PRNTPO");
	printer = 1;
	if((retval = GetPrinter( &printer ))<0)
		return(retval);
	ponbr1 = 0;
	ponbr2 = 99999999;
	retval = GetPoRange( &ponbr1, &ponbr2 );
	if(retval < 0) return(retval);

	/* always to printer */
	retval = opn_prnt("P",'\0',printer,e_mesg,0);  
	if(retval < 0) {
		return(REPORT_ERR);
	}

        for( ; ; ) {
#ifdef ENGLISH
		if((retval=DisplayMessage("Is PO Form Aligned (Y/N)?"))<0)
#else
		if((retval=DisplayMessage("Formulaire BC est-il aligne (O/N)?"))<0)
#endif
			break;	
		if((retval = GetResponse(answer)) <0)
			break;
		if(answer[0] == YES) break;
		retval = align_po();
		rite_top();
		if(retval != NOERROR) break;
	}
	if(retval == NOERROR) {
		if(( retval = Confirm()) <= 0)
			return(retval);

		retval = get_param(&pa_rec,BROWSE,1,e_mesg);
        	if(retval < 1) {
			retval = DBH_ERR;
		}
		if(retval != DBH_ERR)
			retval = print_po();
	}

	close_rep(NOBANNER);
	close_dbh();
	return(retval);
}

/* -----------------------  END OF PRNTPO --------------------------------- */
/* ---------------------------------------------------------------------------
	DEFINITION:  prints (*) in a po form to test for alignment.
--------------------------------------------------------------------------- */
align_po()
{
        int cnt;
        static char tst_string[] = "****************************************";

        if(prnt_line() < 0) return(REPORT_ERR);		/* line 1 */
	mkln(65,tst_string,10);
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 2 */
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 3 */

	mkln(SCHL_POS,tst_string,28);
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 4 */

	mkln(SCHL_POS,tst_string,30);
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 5 */

	mkln(SCHL_POS,tst_string,30);
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 6 */

	if(!SW1) {
		mkln(18,tst_string,3);
	}
	mkln(SCHL_POS,tst_string,30);
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 7 */

	mkln(SCHL_POS,tst_string,8);
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 8 */

	mkln(48,"***-***-****",12);
	mkln(66,"***-***-****",12);
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 9 */

	mkln(SUPP_POS,tst_string,29);
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 10 */

	mkln(SUPP_POS,tst_string,29);
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 11 */

	mkln(SUPP_POS,tst_string,29);
	mkln(SCHL_POS,tst_string,33);
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 12 */

	mkln(SUPP_POS,tst_string,29);
	mkln(SCHL_POS,tst_string,33);
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 13 */

	mkln(SUPP_POS,tst_string,8);
	mkln(SCHL_POS,tst_string,33);
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 14 */

	mkln(SCHL_POS,tst_string,30);
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 15 */

	mkln(SCHL_POS,tst_string,7);
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 16 */

        if(prnt_line() < 0) return(REPORT_ERR);		/* line 17 */
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 18 */

	mkln(SCHL_POS,tst_string,15);
	mkln(61,"***-***-****",12);
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 19 */

        if(prnt_line() < 0) return(REPORT_ERR);		/* line 20 */
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 21 */

	mkln(SUPP_POS,tst_string,10);
	mkln(22,"****/**/**",10);
	mkln(45,"****/**/**",10);
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 22 */
        if(prnt_line() < 0) return(REPORT_ERR);		/* line 23 */

       	if(prnt_line() < 0) return(REPORT_ERR);		/* line 24 */

	mkln(43,tst_string,10);
	mkln(62,tst_string,8);
       	if(prnt_line() < 0) return(REPORT_ERR);		/* line 25 */

       	if(prnt_line() < 0) return(REPORT_ERR);		/* line 26 */
       	if(prnt_line() < 0) return(REPORT_ERR);		/* line 27 */
       	if(prnt_line() < 0) return(REPORT_ERR);		/* line 28 */

	for(cnt = 0;cnt < 19; cnt++) {
		mkln(4,tst_string,5); 
		mkln(9,tst_string,2); 
               	mkln(11,tst_string,1);
               	mkln(12,tst_string,1);
               	mkln(13,tst_string,30);
               	mkln(43,tst_string,18);
		mkln(61,"**,***.**",9); 
		mkln(TOTAL_POS,"***,***.**",10); 
        	if(prnt_line() < 0) return(REPORT_ERR);	/* line 29 - 47 */
	}
	for(cnt = 0;cnt < 5; cnt++) {
        	if(prnt_line() < 0) return(REPORT_ERR);	/* line 48 - 52 */
	}
       	if(prnt_line() < 0) return(REPORT_ERR);	/* line 53 */
	mkln(TOTAL_POS,"***,***.**",10); 
       	if(prnt_line() < 0) return(REPORT_ERR);	/* line 54 */
	mkln(TOTAL_POS,"***,***.**",10); 
       	if(prnt_line() < 0) return(REPORT_ERR);	/* line 55 */
	mkln(TOTAL_POS,"***,***.**",10); 
       	if(prnt_line() < 0) return(REPORT_ERR);	/* line 56 */
	mkln(TOTAL_POS,"***,***.**",10); 
       	if(prnt_line() < 0) return(REPORT_ERR);	/* line 57 */
	mkln(TOTAL_POS,"***,***.**",10); 
       	if(prnt_line() < 0) return(REPORT_ERR);	/* line 58 */
       	if(prnt_line() < 0) return(REPORT_ERR);	/* line 59 */
	mkln(TOTAL_POS,"***,***.**",10); 
       	if(prnt_line() < 0) return(REPORT_ERR);	/* line 60 */

	return(NOERROR);
}

/* ------------------------------------------------------------------------ */
print_po()
{
	char txt[80];
	char	txt_line[11];
	char	valans[3];
	short 	prev_fund;
	static 	short	first_time;

	pohdr_rec.ph_code = ponbr1;

	flg_reset( POHDR );
	for( ; ; ) {
		retval = get_n_pohdr(&pohdr_rec,UPDATE,0,FORWARD,e_mesg);
		if( retval < 0 ){
			if(retval == EFL) break;
			DisplayError(e_mesg);
			break;
		}
		if(pohdr_rec.ph_code > ponbr2) {
			roll_back(e_mesg);
			break;
		}
		if(pohdr_rec.ph_print[0] == YES ||
			pohdr_rec.ph_print[0] == POPRNT ||
			pohdr_rec.ph_print[0] == LABEL ) {
			if(ponbr1 != ponbr2) {
#ifndef	ORACLE
				roll_back(e_mesg);
#endif
		 		continue;
			}
			else {
#ifdef ENGLISH
				sprintf(e_mesg,"PO#: %ld already printed. Do you want to print it again (Y/N)?",pohdr_rec.ph_code);
#else
				sprintf(e_mesg,"BC#: %ld deja imprime. Desirez-vous le reimprimer (O/N)?",pohdr_rec.ph_code);
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

		if(pohdr_rec.ph_code >= 90000000) /* don't print Manual PO */
/*			continue;		*/
			break;

		if (prev_fund != pohdr_rec.ph_funds) {
			ctl_rec.fund 	= pohdr_rec.ph_funds;
			retval = get_ctl(&ctl_rec,BROWSE,0,e_mesg);
			if( retval < 0 ){
				DisplayError(e_mesg);
				break;
			}
			gst_tax_rate	= ctl_rec.gst_tax;	
			pst_tax_rate	= ctl_rec.pst_tax;
			prev_fund	= ctl_rec.fund; 
		}	

		STRCPY(supp_rec.s_supp_cd,pohdr_rec.ph_supp_cd);
		retval = get_supplier(&supp_rec,BROWSE,0,e_mesg);
		if( retval < 0 ){
			DisplayError(e_mesg);
			break;
		}

		/* if SW1 set then read the first po item and get the 
			requisition number from it before printing
			first header (FOR NEWFOUNDLAND) */

		if(SW1){

			po_type[0] = pohdr_rec.ph_type[0];
			poitem_rec.pi_code = pohdr_rec.ph_code;
			poitem_rec.pi_item_no = 0;
			flg_reset( POITEM );
#ifndef ORACLE
			retval = get_n_poitem(&poitem_rec,BROWSE,0,FORWARD,
								e_mesg);
#else
			retval = get_n_poitem(&poitem_rec,BROWSE,0,EQUAL,
								e_mesg);
#endif
			if( retval < 0 ){
				if(retval == EFL) break;
				DisplayError(e_mesg);
				break;
			}
#ifndef ORACLE
			if(pohdr_rec.ph_code != poitem_rec.pi_code) {
				break;
			}
#endif
			/* copy the po requisition number to the temp
				variable */

			po_req = poitem_rec.pi_req_no;

			seq_over( POITEM );
		}
		else{
			strcpy(po_req_no, pohdr_rec.ph_req_no);
		}		

		if(print_header() == ERROR) return(ERROR);

		first_time = 1;
		qty_printed = 0;
		
		po_type[0] = pohdr_rec.ph_type[0];
		poitem_rec.pi_code = pohdr_rec.ph_code;
		poitem_rec.pi_item_no = 0;
		flg_reset( POITEM );
		for( ; ; ) {
#ifndef ORACLE
			retval = get_n_poitem(&poitem_rec,BROWSE,0,FORWARD,
								e_mesg);
#else
			retval = get_n_poitem(&poitem_rec,BROWSE,0,EQUAL,
								e_mesg);
#endif
			if( retval < 0 ){
				if(retval == EFL) break;
				DisplayError(e_mesg);
				break;
			}
#ifndef ORACLE
			if(pohdr_rec.ph_code != poitem_rec.pi_code) {
				break;
			}
#endif
			/*  Initialize  */
			if(first_time) {
				scpy((char *)&prev_poitem,(char *)&poitem_rec,
					sizeof(poitem_rec));
				strcpy(prev_tax1, poitem_rec.pi_tax1);
				strcpy(prev_tax2, poitem_rec.pi_tax2);
				prev_qty = poitem_rec.pi_orig_qty;
				prev_unitprice = poitem_rec.pi_unitprice;
				prev_value = poitem_rec.pi_value;
			}
			strcpy(item_desc,poitem_rec.pi_desc);

			len = strlen(item_desc);
			lines = (len / 30) + .5;

			if(linecnt >= 51-lines) { 
				print_total(0);
				rite_top();
				if(print_header() == ERROR) return(ERROR);
			}
			if(poitem_rec.pi_tax2[0] == FREIGHT) {
				total_freight += poitem_rec.pi_value;
				continue;
			}
			
			if(!first_time) {
				if(poitem_rec.pi_orig_qty != 0) {
					/*  Print item totals  */
					retval = print_item_totals();
					if(retval < 0) return(-1);
					qty_printed = 0;
				}
				prnt_line();
				/*  Save current totals if not equal to 0  */
				if(poitem_rec.pi_value != 0){
					strcpy(prev_tax1, poitem_rec.pi_tax1);
					strcpy(prev_tax2, poitem_rec.pi_tax2);
					prev_qty = poitem_rec.pi_orig_qty;
					prev_value = poitem_rec.pi_value;
					prev_unitprice=poitem_rec.pi_unitprice;
				}
			}

			if(!qty_printed) {
				/*  Print quantity and tax flags  */
				retval = print_qty();
				if(retval < 0) return(-1);
				qty_printed = 1;
			}

			/*  Always print description  */
			retval = print_description();
			if(retval < 0) return(-1);

			first_time = 0;
			
			/*  Copy required parts of the structure  */
			scpy((char *)&prev_poitem,(char *)&poitem_rec,
				sizeof(poitem_rec));
		}

		if(!first_time) {
			if(poitem_rec.pi_orig_qty != 0) {
				/*  Print totals  */
				retval = print_item_totals();
				if(retval < 0) return(-1);
				qty_printed = 0;
			}
			prnt_line();
			/*  Save current totals if ! 0  */
		}
		seq_over( POITEM );
		if(retval == ERROR) break;
		print_total(1);
		rite_top();
		pohdr_rec.ph_print[0] = POPRNT;
		retval = put_pohdr(&pohdr_rec,UPDATE,e_mesg);
		if(retval < 0) {
			DisplayError(e_mesg);
			break;
		}
		if(commit(e_mesg) < 0) {
			DisplayError(e_mesg);
			break;
		}
		pohdr_rec.ph_code++;
		order_total = sub_total = po_total = 0;
		pst_tax = 0;
		total_freight=0;
		gst_tax = 0;
	}
	seq_over( POHDR );
	return(NOERROR);
}

print_header()
{
	long longdate;
	char txt_buffer[11];
	int len,j;
	int curr_line = 4;

	blank_line = 0;

       	if(prnt_line() < 0) return(ERROR);		/* line 1 */
	tedit((char *)&pohdr_rec.ph_code,"________0_",txt_buffer,R_LONG);
	mkln(66,txt_buffer,10);
       	if(prnt_line() < 0) return(ERROR);		/* line 2 */
       	if(prnt_line() < 0) return(ERROR);		/* line 3 */

		/* bill to address */
	sch_rec.sc_numb = pohdr_rec.ph_billto;
	retval = get_sch(&sch_rec,BROWSE,0,e_mesg);
	if( retval < 0 ){
		DisplayError(e_mesg);
		return(-1);
	}

	mkln(SCHL_POS,sch_rec.sc_name,28);
       	if(prnt_line() < 0) return(ERROR);		/* line 4 */

		/* check address lines (5 to 8) and only print if not NULL */
	
	if (strcmp(sch_rec.sc_add1,"\0")) {
		curr_line++;
		mkln(SCHL_POS,sch_rec.sc_add1,30); 
		if(prnt_line() < 0) return(ERROR);
	}
 	else
		blank_line++;
	
	if (strcmp(sch_rec.sc_add2,"\0")) {
		curr_line++;
		mkln(SCHL_POS,sch_rec.sc_add2,30);
		if(prnt_line() < 0) return(ERROR);
	}
	else
		blank_line++;

	if (strcmp(sch_rec.sc_add3,"\0")) {
		curr_line++;
		if( curr_line == 7 ) {
			if(!SW1) {
				mkln(18,dist_no,3);
			}
		}
		mkln(SCHL_POS,sch_rec.sc_add3,30);
		if(prnt_line() < 0) return(ERROR);
	}
	else
		blank_line++;

	if (strcmp(sch_rec.sc_pc,"\0")) {
		curr_line++;
		if( curr_line == 7 ) {
			if(!SW1) {
				mkln(18,dist_no,3);
			}
		}
		mkln(SCHL_POS,sch_rec.sc_pc,8);
		if(prnt_line() < 0) return(ERROR);
	}
	else
		blank_line++;

		/* print any blank lines now (after school address) */

	for(i=1; i<=blank_line; i++) {
		curr_line++;
		if( curr_line == 7 ) {
			if(!SW1) {
				mkln(18,dist_no,3);
			}
		}
		if(prnt_line() < 0) return(ERROR);
	}

	mkln(48,sch_rec.sc_phone,3);	/* bill to phone number */
	mkln(51,"-",1);			/* with dashes */
	mkln(52,sch_rec.sc_phone+3,3);
	mkln(55,"-",1);
	mkln(56,sch_rec.sc_phone+6,4);
	mkln(66,sch_rec.sc_fax,3);	/* bill to fax number */
	mkln(69,"-",1);			/* with dashes */
	mkln(TOTAL_POS,sch_rec.sc_fax+3,3);
	mkln(73,"-",1);
	mkln(74,sch_rec.sc_fax+6,4);
	curr_line++;
       	if(prnt_line() < 0) return(ERROR);		/* line 9 */

		/* supplier name & address */

	mkln(SUPP_POS,supp_rec.s_name,29);
	curr_line++;
       	if(prnt_line() < 0) return(ERROR);		/* line 10 */

		/* check supp. address (lines 11 to 14) print if not NULL */
	
	if (strcmp(supp_rec.s_add1,"\0")) {
		curr_line++;
		mkln(SUPP_POS,supp_rec.s_add1,29);
		if(prnt_line() < 0) return(ERROR);	/* max line 11 */
	}
	
		/* get school to ship to address */

	sch_rec.sc_numb = pohdr_rec.ph_shipto;
	retval = get_sch(&sch_rec,BROWSE,0,e_mesg);
	if( retval < 0 ){
		DisplayError(e_mesg);
		return(-1);
	}

	if (strcmp(supp_rec.s_add2,"\0")) {
		curr_line++;
		mkln(SUPP_POS,supp_rec.s_add2,29);
		if( curr_line == 12 )
			mkln(SCHL_POS,sch_rec.sc_name,33);
		if(prnt_line() < 0) return(ERROR);	/* max line 12 */
	}

	if (strcmp(supp_rec.s_add3,"\0")) {
		curr_line++;
		mkln(SUPP_POS,supp_rec.s_add3,29);
		if( curr_line == 12 )
			mkln(SCHL_POS,sch_rec.sc_name,33);

		if( curr_line == 13 ) {
			if(strcmp(sch_rec.sc_add1,"\0")) {
				mkln(SCHL_POS,sch_rec.sc_add1,30);
				sch_rec.sc_add1[0]='\0';
			}
			else   
			if(strcmp(sch_rec.sc_add2,"\0")){	
				mkln(SCHL_POS,sch_rec.sc_add2,30);
				sch_rec.sc_add2[0]='\0';
			}
			else   
			if(strcmp(sch_rec.sc_add3,"\0")){	
				mkln(SCHL_POS,sch_rec.sc_add3,30);
				sch_rec.sc_add3[0]='\0';
			}
			else   
			if(strcmp(sch_rec.sc_pc,"\0")){	
				mkln(SCHL_POS,sch_rec.sc_pc,8);
				sch_rec.sc_pc[0]='\0';
			}
  		}		
		if(prnt_line() < 0) return(ERROR);	/* max line 13 */
	}


	if (strcmp(supp_rec.s_pc,"\0")) {
		curr_line++;
		mkln(SUPP_POS,supp_rec.s_pc,10);
		if( curr_line == 12 )
			mkln(SCHL_POS,sch_rec.sc_name,33);
		
		if( curr_line == 13 ) {
			if(strcmp(sch_rec.sc_add1,"\0")) {
				mkln(SCHL_POS,sch_rec.sc_add1,30);
				sch_rec.sc_add1[0]='\0';
			}
			else   
			if(strcmp(sch_rec.sc_add2,"\0")){	
				mkln(SCHL_POS,sch_rec.sc_add2,30);
				sch_rec.sc_add2[0]='\0';
			}
			else   
			if(strcmp(sch_rec.sc_add3,"\0")){	
				mkln(SCHL_POS,sch_rec.sc_add3,30);
				sch_rec.sc_add3[0]='\0';
			}
			else   
			if(strcmp(sch_rec.sc_pc,"\0")){	
				mkln(SCHL_POS,sch_rec.sc_pc,8);
				sch_rec.sc_pc[0]='\0';
			}
		}
		if( curr_line == 14 ) {
			if(strcmp(sch_rec.sc_add1,"\0")) {
				mkln(SCHL_POS,sch_rec.sc_add1,30);
				sch_rec.sc_add1[0]='\0';
			}
			else   
			if(strcmp(sch_rec.sc_add2,"\0")){	
				mkln(SCHL_POS,sch_rec.sc_add2,30);
				sch_rec.sc_add2[0]='\0';
			}
			else   
			if(strcmp(sch_rec.sc_add3,"\0")){	
				mkln(SCHL_POS,sch_rec.sc_add3,30);
				sch_rec.sc_add3[0]='\0';
			}
			else   
			if(strcmp(sch_rec.sc_pc,"\0")){	
				mkln(SCHL_POS,sch_rec.sc_pc,8);
				sch_rec.sc_pc[0]='\0';
			}
  		}	
		if(prnt_line() < 0) return(ERROR);	/* max line 14 */
	}
	else {
		if( curr_line == 12 ) {
			if(prnt_line() < 0) return(ERROR);/* max line 14 */
			mkln(SCHL_POS,sch_rec.sc_name,33);
			if(prnt_line() < 0) return(ERROR); /* max line 14 */
			curr_line++;
			curr_line++;
		}
	}

	if(strcmp(sch_rec.sc_add1,"\0")) {	
		curr_line++;
		mkln(SCHL_POS,sch_rec.sc_add1,30);
		if(prnt_line() < 0) return(ERROR);	
	}

	if(strcmp(sch_rec.sc_add2,"\0")) {	
		curr_line++;
		mkln(SCHL_POS,sch_rec.sc_add2,30);
		if(prnt_line() < 0) return(ERROR);
	}

	if(strcmp(sch_rec.sc_add3,"\0")) {	
		curr_line++;
		mkln(SCHL_POS,sch_rec.sc_add3,30);
		if(prnt_line() < 0) return(ERROR);
	}

	if(strcmp(sch_rec.sc_pc,"\0")) {	
		curr_line++;
		mkln(SCHL_POS,sch_rec.sc_pc,8);
		if(prnt_line() < 0) return(ERROR);
	}
	
		/* print blank lines up to contact person name */

	for(;curr_line < 18; curr_line++) 
		if(prnt_line() < 0) return(ERROR);	/* max line 18 */

		/* print contact person name */		

	mkln(SUPP_POS,supp_rec.s_contact,26);

		/* attention  name/phone */

	mkln(SCHL_POS,pohdr_rec.ph_attention,15);
	txt_buffer[0]='\0';
	strncat(txt_buffer,sch_rec.sc_phone,3);
	strncat(txt_buffer,"-",1);
	strncat(txt_buffer,sch_rec.sc_phone+3,3);
	strncat(txt_buffer,"-",1);
	strncat(txt_buffer,sch_rec.sc_phone+6,4);
	strcat(txt_buffer,"\0");
	mkln(61,txt_buffer,12);
        if(prnt_line() < 0) return(ERROR);		/* line 19 */

        if(prnt_line() < 0) return(ERROR);		/* line 20 */
        if(prnt_line() < 0) return(ERROR);		/* line 21 */

	/* if switch 1 set print the requisition number on the po */ 

	if(SW1){
		tedit((char*)&po_req,"________", txt_buffer,R_LONG);
		mkln(2, txt_buffer, 8);
	}
	else{

		mkln(2, po_req_no, 11);
	}

	if(pohdr_rec.ph_due_date == pohdr_rec.ph_date) {
#ifdef ENGLISH
		mkln(23,"ASAP",4);
#else
		mkln(25,"PROMPTE",7);
#endif
	}
	else {
	 	tedit((char *)&pohdr_rec.ph_due_date,"____/__/__",
				txt_buffer,R_LONG);
		mkln(22,txt_buffer,10);
	}
	longdate = conv_date(pohdr_rec.ph_date,6,4);
 	tedit((char *)&pohdr_rec.ph_date,"____/__/__",txt_buffer,R_LONG);
	mkln(45,txt_buffer,10);
        if(prnt_line() < 0) return(ERROR);		/* line 22 */

        if(prnt_line() < 0) return(ERROR);		/* line 23 */
        if(prnt_line() < 0) return(ERROR);		/* line 24 */

	len = strlen(supp_rec.s_supp_cd);      /* right justify supplier */
	for(j = 0; j < 10 - len; j++) {        /* code.                  */
		txt_buffer[j] = ' ';
	}
	txt_buffer[j] = '\0';
	strcat(txt_buffer,supp_rec.s_supp_cd);
	mkln(43,txt_buffer,10);

	tedit((char *)&pohdr_rec.ph_code,"________0_",txt_buffer,R_LONG);
	mkln(60,txt_buffer,10);
        if(prnt_line() < 0) return(ERROR);		/* line 25 */

       	if(prnt_line() < 0) return(ERROR);	/* line 26 */
       	if(prnt_line() < 0) return(ERROR);	/* line 27 */
       	if(prnt_line() < 0) return(ERROR);	/* line 28 */

	return(NOERROR);
}

print_qty()
{
	static	char	txt_line[11];


	tedit((char *)&prev_qty,"___0_.__",txt_line,R_DOUBLE);
	mkln(1,txt_line,8);
	
	mkln(9,prev_poitem.pi_unit,2);
	mkln(11, prev_poitem.pi_tax1,1);
	mkln(12, prev_poitem.pi_tax2,1);

	return(NOERROR);
}
print_description()
{
	int 	o,j,lines,totchar;

	/* Changed by F.Tao, 01/30/91			*/
	/* print 28 chars for desc. instead of 30 	*/

	/* If description is more than 28 characters print on two lines */
	lines = strlen(item_desc)/28;
	totchar = 0;
	for(o=0;o<=lines;o++)  { 
		if(strlen(item_desc)-totchar > 28) {
			for(j=28; j>=0;j--){
				/* break at first space */
				if(item_desc[totchar+j] == ' ') break;
			}
			if(j<=0) j=28;
			if(totchar==0)
				mkln(14,item_desc+totchar,j);
			else
				mkln(14,item_desc+totchar+1,j);
			totchar= totchar + j;
        		if(prnt_line() < 0) return(ERROR);/* lines 22 - 42 */
		}
	} 
	if(totchar==0)
		mkln(14,item_desc,strlen(item_desc));
	else
		mkln(14,item_desc+totchar+1,strlen(item_desc)-totchar);
	return(NOERROR);
}

print_item_totals()
{
	static	char txt_line[11];
	static	short	tax1, tax2;

	tax1 = 0; 
	tax2 = 0; 

	if (po_type[0] != DIRECT){
        	if(prnt_line() < 0) return(ERROR); 
		mkln(13, "Stock #:", 9);
		mkln(24, prev_poitem.pi_st_code,10);
	}  
	mkln(43,prev_poitem.pi_acct,18);

	tedit((char *)&prev_unitprice,"___0_.___",txt_line,R_DOUBLE);
	mkln(61,txt_line,9);
	
	if (prev_tax1[0] == TAXABLE) 
		tax1 = gst_tax_rate;
	if (prev_tax2[0] == TAXABLE)  
		tax2 = pst_tax_rate;	 

	Tax_Calculation(tax1, tax2, prev_value, &tax_cal);
 	pst_tax		+= tax_cal.pst_amt;		
 	gst_tax 	+= tax_cal.gst_amt;		
	order_total	+= prev_value; 
	tedit((char *)&prev_value,"___,_0_.__",txt_line,R_DOUBLE);
	mkln(TOTAL_POS,txt_line,10);

	return(NOERROR);
}

print_total(mode)
int	mode;	/* 0 - if multiple pages,  1 - if last page */
{
	char txt_buffer[20];

	for( ;linecnt < 53; ) {
		prnt_line();		/* advance to sub-total */
	}

	tedit((char *)&order_total,"___,_0_.__",txt_buffer,R_DOUBLE);
	mkln(TOTAL_POS,txt_buffer,10);			/* sub total */
	prnt_line();

	if(mode == 1) {
  		tedit((char *)&total_freight,"___,_0_.__", txt_buffer,R_DOUBLE);
		mkln(TOTAL_POS,txt_buffer,10);			/* freight */
		prnt_line();
	}

	tedit((char *)&gst_tax,"___,_0_.__",txt_buffer,R_DOUBLE);
	mkln(TOTAL_POS,txt_buffer,10);			
	prnt_line();

	if(mode == 1) {
		sub_total = order_total + total_freight + gst_tax; 
			/** + fst.tax; **/
		tedit((char *)&sub_total,"___,_0_.__",txt_buffer,R_DOUBLE);
		mkln(TOTAL_POS,txt_buffer,10);			/* sub total */
	}
	prnt_line();

	tedit((char *)&pst_tax,"___,_0_.__",txt_buffer,R_DOUBLE);
	mkln(TOTAL_POS,txt_buffer,10);				/* P.S.T tax */
	prnt_line();
	prnt_line();

	if(mode == 1) {
		po_total = sub_total + pst_tax; 
		tedit((char *)&po_total,"___,_0_.__",txt_buffer,R_DOUBLE);
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

