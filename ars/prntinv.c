/*--------------------------------------------------------------------------
	SOURCE NAME:  PRNTINV.C
	SYSTEM     :  BUGETARY FINANCIAL SYSTEM 
	MODULE     :  Account Receivable Module
	CREATED ON :  23 Nov. 1990 
	CREATED BY :  Frank Tao

DESCRIPTION:
	This program prints a range of specified invoice numbers.
	It allows for the user to do a test print to allign the forms.

MODIFICATIONS:

PROGRAMMER           YY/MM/DD        DESCRIPTION OF MODIFICATIONS
C.Leadbeater	     90/12/06	     Change so that blank lines are not 
				     printed for address.
C.Leadbeater	     90/12/07	     Add GST and PST total fields.
F.Tao		     90/12/11	     If  number of G/L account # is more
				     than ten, put the next one on the new
				     page.
J McLean       	     92/07/27	     Changed customer code from 6 to 10 
				     characters.
J McLean	     92/08/11	     Changed so that remarks are not printed
				     but each item description is.
J McLean	     92/11/04	     Added code to properly print one physical
				     item that uses two item lines for the
				     description.
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
#define POPRNT	'P'
#define LABEL	'L'

#define TAXABLE	'T'
#define EXEMPT	'E'
#define FREIGHT 'F'

#define T_CRMEMO "CM"
#define T_DBMEMO "DM"
#else
#define YES	'O'
#define NO	'N'

#define POPRNT	'I'
#define LABEL	'E'

#define TAXABLE	'T'
#define EXEMPT	'E'
#define FREIGHT 'F'

#define T_CRMEMO "NC" 
#define T_DBMEMO "ND"
#endif

#define  EXIT  12
#define  POOPT 1600

Pa_rec   pa_rec;             /*  Declarations for DBH record reading */
Sch_rec  sch_rec;
Ar_hdr   arhdr_rec;
Ar_item  aritem_rec;
Cu_rec	 cu_rec;

static char	 answer[2];
static char	 print_type[2];/* kyle */
static int 	 retval,err; 
static short	 printer;
static long	 invr1, invr2;
static short	 fund1, fund2;
static char	 program[11];

static double	 inv_total;    
static char 	 inv_gl_acct[20][19];
static char 	 temp_acct[19];
extern char 	 e_mesg[132];

/* -----------------------------------------------------------------------*/
/* Function Print Invoice Listing				          */
/* -----------------------------------------------------------------------*/

prntinv()
{
	char 	valans[4];

	LNSZ = 132;
	inv_total = 0;

	strcpy(program,"PRNTINV");
	printer = 1;
	if((retval = GetPrinter( &printer ))<0)
		return(retval);
/* kyle */
	/* Select Which Type to Print */
#ifdef ENGLISH
	if(DisplayMessage("Print I(nvoices), D(ebit Memos), C(redit Memos)")<0)
		return(-1);
	strcpy(valans, "IDC");
#else
	if(DisplayMessage("Imprimer F(actures), D(notes debit), C(notes credit)")<0)
		return(-1);
	strcpy(valans, "FDC");
#endif

	for( ; ; ){
		if((retval = GetResponse(print_type, valans)) <0)
			continue;

#ifdef ENGLISH
		if(print_type[0] == 'I' || print_type[0] == 'D' ||
						print_type[0] == 'C')
			break;

		fomce();
		fomen("Must Be Print I(nvoices), D(ebit Memos), Or C(redit Memos)");
		fflush(stdout);
#else
		if(print_type[0] == 'F' || print_type[0] == 'D' ||
						print_type[0] == 'C')
			break;

		fomce();
		fomen("Doit etre imprimer F(actures), D(notes Debit), C(notes Credit");
		fflush(stdout);
#endif
	}
/* kyle */
	fund1 = 0;
	fund2 = 999;
	retval = GetFundRange( &fund1, &fund2 );
	if(retval < 0) return(retval);

	invr1 = 0;
	invr2 = 99999999;
	retval = GetInvcRange( &invr1, &invr2 );
	if(retval < 0) return(retval);

	/* always to printer */
	retval = opn_prnt("P",'\0',printer,e_mesg,0);  
	if(retval < 0) {
		return(REPORT_ERR);
	}

        for( ; ; ) {
#ifdef ENGLISH
		if((retval=DisplayMessage("Is INVOICE Form Aligned (Y/N)?"))<0)
			break;	
		if((retval = GetResponse(answer, "YN")) <0)
			break;
#else
		if((retval=DisplayMessage("Formulaire facture est-il aligne (O/N)?"))<0)
			break;	
		if((retval = GetResponse(answer, "ON")) <0)
			break;
#endif
		if(answer[0] == YES) break;
		retval = align_po();
		if(retval < 0)	return(retval);
		retval = rite_top();
		if(retval < 0)	return(retval);
		if(retval != NOERROR) break;
	}
	if(( retval = INVConfirm()) <= 0)
		return(retval);


	/*
	*	Get The Parameter Record
	*/
	err = get_param(&pa_rec, BROWSE, 1, e_mesg) ;
	if(err == ERROR) {
		DispError(e_mesg);
		return(ERROR) ;
	}
	else if(err == UNDEF) {
#ifdef ENGLISH
		DispError("Parameters Are Not Set Up ...");
#else
		DispError("Parametres ne sont pas etablis... ");
#endif
		return(ERROR) ;
	}
	retval = print_inv();
	if(retval < 0)	return(retval);

	/* Dehighlight the option which is highlighted in Process() */
	fomca1( POOPT, 9, 5 );

	close_rep(NOBANNER);
	close_dbh();
	return(retval);
}

/* ---------------------------------------------------------------------------
	DEFINITION:  prints (*) in a invoice form to test for alignment.
--------------------------------------------------------------------------- */
align_po()
{
        int cnt;
        static char tst_string[] = 
		"************************************************************";

       	if(prnt_line() < 0) return(ERROR);	/* line 1 	*/	
       	if(prnt_line() < 0) return(ERROR); 	/* line 2 	*/	
       	/* line 2 	*/	
	mkln(7,tst_string,10);
       	if(prnt_line() < 0) return(ERROR); 	/* line 3 	*/	
       	if(prnt_line() < 0) return(ERROR); 	/* line 4 	*/	
       	if(prnt_line() < 0) return(ERROR); 	/* line 5 	*/	
       	if(prnt_line() < 0) return(ERROR); 	/* line 6 	*/	

	/* payable address */
	mkln(1,tst_string,30);
	mkln(47,tst_string,30);
       	if(prnt_line() < 0) return(ERROR);	/* line 7 	*/	

	mkln(1,tst_string,30);
	mkln(47,tst_string,30);

       	if(prnt_line() < 0) return(ERROR);	/* line 8 	*/	

	mkln(1,tst_string,30);
	mkln(47,tst_string,30);

       	if(prnt_line() < 0) return(ERROR);	/* line 9 	*/	

	mkln(1,tst_string,30);
	mkln(47,tst_string,30);

       	if(prnt_line() < 0) return(ERROR);	/* line 10 	*/	

	mkln(1,tst_string,30);
	mkln(47,tst_string,30);

       	if(prnt_line() < 0) return(ERROR);	/* line 11 	*/	
	

	mkln(52,"###",3);			/* bill to phone number */
	mkln(55,"-",1);				/* with dashes */
	mkln(56,"###",3);
	mkln(59,"-",1);
	mkln(60,"####",4);
	mkln(70,"###",3);			/* bill to fax number */
	mkln(73,"-",1);				/* with dashes */
	mkln(74,"###",3);
	mkln(77,"-",1);
	mkln(78,"####",4);
       	if(prnt_line() < 0) return(ERROR);	/* line 12 	*/	
	if(!SW1) {
#ifdef ENGLISH
	mkln(19,"Registration #: ",16);
#else
	mkln(19,"No. Matricule : ",16);
#endif
	mkln(35,tst_string,10);	 
	}
       	if(prnt_line() < 0) return(ERROR);	/* line 13 	*/	
       	if(prnt_line() < 0) return(ERROR);	/* line 14 	*/	

	mkln(7,tst_string,10);
	mkln(38,"####/##/##",10);
	mkln(65,"##########",10);
       	if(prnt_line() < 0) return(ERROR);	/* line 15 */
       	if(prnt_line() < 0) return(ERROR);	/* line 16 */
       	if(prnt_line() < 0) return(ERROR);	/* line 17 */

if(SW1) {
	for (cnt=1; cnt <22; cnt++){
		mkln(7,tst_string,60);
		mkln(70,"###,###.##", 10);
        	if(prnt_line() < 0) return(ERROR); /* line 18 - 37   */	
	}
}
else {
	for (cnt=1; cnt <19; cnt++){
		mkln(7,tst_string,60);
		mkln(70,"###,###.##", 10);
        	if(prnt_line() < 0) return(ERROR); /* line 18 - 37   */	
	}
}

#ifdef ENGLISH
	mkln(61,"GST :",5);
#else
	mkln(61,"TPS :",5);
#endif
	mkln(70,"###,###.##", 10);		
        if(prnt_line() < 0) return(ERROR);	 /* line 38 	*/	
#ifdef ENGLISH
	mkln(61,"PST :",5);
#else
	mkln(61,"TVP :",5);
#endif
	mkln(70,"###,###.##", 10);		
        if(prnt_line() < 0) return(ERROR);	 /* line 39 	*/	
if(!SW1){
#ifdef ENGLISH
	mkln(43,"Please Pay This Amount: ",24);
#else
	mkln(43,"SVP remettre ce montant:",24);
#endif
}
	mkln(70,"###,###.##", 10);		
        if(prnt_line() < 0) return(ERROR);	 /* line 40 	*/	
        if(prnt_line() < 0) return(ERROR);	 /* line 41 	*/	

	mkln(1,tst_string,18);		
	mkln(20,tst_string,18);		
        if(prnt_line() < 0) return(ERROR);	 /* line 43 	*/	
	mkln(1,tst_string,18);		
	mkln(20,tst_string,18);		
        if(prnt_line() < 0) return(ERROR);	 /* line 44 	*/	
	mkln(1,tst_string,18);		
	mkln(20,tst_string,18);		
        if(prnt_line() < 0) return(ERROR);	 /* line 45 	*/	
	mkln(1,tst_string,18);		
	mkln(20,tst_string,18);		
        if(prnt_line() < 0) return(ERROR);	 /* line 46 	*/	
	mkln(1,tst_string,18);		
	mkln(20,tst_string,18);		
        if(prnt_line() < 0) return(ERROR);	 /* line 47 	*/	
}

/* ------------------------------------------------------------------------ */
/* Function: Print Invoice 						    */
/* ------------------------------------------------------------------------ */
print_inv()
{
	static	int 	i=0;
	static	char	txt[132];
	static	char	prnt_desc[61];
	static	double	prnt_amount;
	static  short	first_flg;
	static	char	txt_line[20];
	static	int	flag;
	static	double	amount;

	arhdr_rec.ah_fund    = fund1;
	arhdr_rec.ah_inv_no  = invr1;
	arhdr_rec.ah_sno = 0;
	flg_reset(ARSHDR);

	for( ; ; ) {
		retval = get_n_arhdr(&arhdr_rec,BROWSE,0,FORWARD,e_mesg);
		if( retval < 0 ){
			if(retval == EFL) break;
			DisplayError(e_mesg);
			break;
		}
		if(arhdr_rec.ah_fund > fund2) break;
		if(arhdr_rec.ah_inv_no < invr1 || 
		   arhdr_rec.ah_inv_no > invr2)
			continue;

		switch(print_type[0]){
		case 'I':
		case 'F':
			if(strcmp(arhdr_rec.ah_type, "IN") != 0)
				continue;
			break;
		case 'D':
			if(strcmp(arhdr_rec.ah_type, "DM") != 0)
				continue;
			break;
		case 'C':
			if(strcmp(arhdr_rec.ah_type, "CM") != 0)
				continue;
			break;
		}

		strcpy(cu_rec.cu_code,arhdr_rec.ah_cu_code);
		retval = get_cust(&cu_rec,BROWSE,0,e_mesg);
		if( retval < 0 ){
			if(retval == EFL) break;
			DisplayError(e_mesg);
			break;
		}
		sch_rec.sc_numb = pa_rec.pa_distccno;
		retval = get_sch(&sch_rec,BROWSE,0,e_mesg);
		if( retval < 0 ){
			if(retval == EFL) break;
			DisplayError(e_mesg);
			break;
		}
/*
		rite_top();
*/

		prnt_desc[0] = '\0';
		prnt_amount = 0.0;

		first_flg = 1;

		if(print_header() == ERROR) 
			return(ERROR);
		aritem_rec.ai_fund   = arhdr_rec.ah_fund;
		aritem_rec.ai_hno    = arhdr_rec.ah_sno;
		aritem_rec.ai_inv_no = arhdr_rec.ah_inv_no;
		aritem_rec.ai_sno    = 0;		
		flg_reset( ARSITEM );

		for( ; ; ) {
#ifndef ORACLE
			retval = get_n_aritem(&aritem_rec,BROWSE,0,FORWARD,
								e_mesg);
#else
			retval = get_n_aritem(&aritem_rec,BROWSE,0,EQUAL,									e_mesg);
#endif
			if(retval < 0){
				if(retval == EFL) break;
				DisplayError(e_mesg);
				break;
			}
#ifndef ORACLE

			if(aritem_rec.ai_fund != arhdr_rec.ah_fund ||
	         	   aritem_rec.ai_hno != arhdr_rec.ah_sno ||
			   aritem_rec.ai_inv_no != arhdr_rec.ah_inv_no)
				break;
#endif
			inv_total = inv_total + aritem_rec.ai_amount;

			/*  Initialize values  */
			if(first_flg) {
				strcpy(prnt_desc, aritem_rec.ai_desc);
				prnt_amount = aritem_rec.ai_amount;
			}

			/*put different G/L account number into the array */ 
			for(i=0; ; i++){
				if(!strcmp(inv_gl_acct[i],aritem_rec.ai_accno))
				     break;
				if(!strcmp(inv_gl_acct[i],"")){
			             strcpy(inv_gl_acct[i],aritem_rec.ai_accno);
				     break;
				}	
			}
			if(strcmp(inv_gl_acct[10],"")){
				strcpy(temp_acct,inv_gl_acct[10]);
				strcpy(inv_gl_acct[10],"");
				retval = print_total(2);
				if(retval < 0)	return(retval);
				retval = rite_top();	
				if(retval < 0)	return(retval);
				if(print_header() == ERROR) return(ERROR);
				/* clean up array	*/
				for (i=0; i<10;i++) {
					strcpy(inv_gl_acct[i],"");
				}
				strcpy(inv_gl_acct[0], temp_acct);	
			}

if(SW1) { 
			if(linecnt >= 38) { 
				retval = print_total(0);
				if(retval < 0)	return(retval);
				retval = rite_top();
				if(retval < 0)	return(retval);
				if(print_header() == ERROR) return(ERROR);
			}
}
else {
			if(linecnt >= 35) { 
				retval = print_total(0);
				if(retval < 0)	return(retval);
				retval = rite_top();
				if(retval < 0)	return(retval);
				if(print_header() == ERROR) return(ERROR);
			}
}

			/*  Print the item detail information  */
			if(strcmp(arhdr_rec.ah_type,T_CRMEMO) == 0)
				flag = -1;
			else
				flag = 1;

			if(!first_flg) {
				if(aritem_rec.ai_amount != 0) {
					mkln(7,prnt_desc,60);
					amount = prnt_amount * flag;
					tedit((char *)&amount,"___,_0_.__-",
							txt_line,R_DOUBLE);
					mkln(70,txt_line,11);
					strcpy(prnt_desc, aritem_rec.ai_desc);
					prnt_amount = aritem_rec.ai_amount;
				}
				else {
					mkln(7,prnt_desc,60);
					strcpy(prnt_desc, aritem_rec.ai_desc);
				}
				
        			if(prnt_line() < 0) return(ERROR);
			}
			first_flg = 0;

/*****
			if(prnt_item_line() == ERROR) return(ERROR);
*****/
		}
		mkln(7,prnt_desc,60);
		amount = prnt_amount * flag;
		tedit((char *)&amount,"___,_0_.__-", txt_line,R_DOUBLE);
		mkln(70,txt_line,11);
        	if(prnt_line() < 0) return(ERROR);

		seq_over( ARSITEM );
		retval = print_total(1);
		if(retval < 0) return(retval);
		retval = rite_top();
		if(retval < 0) return(retval);

		inv_total = 0;
	}
	seq_over( ARSHDR ); 
	return(NOERROR);
}
/* ------------------------------------------------------------------------ */
/* Function: Print Invoice Header Part					    */
/* ------------------------------------------------------------------------ */
print_header()
{
	long longdate;
	char txt_buffer[20];
	int  len,i;
	int  curr_line = 1;	 	

       	if(prnt_line() < 0) return(ERROR);		
       	if(prnt_line() < 0) return(ERROR);	
 	tedit((char *)&arhdr_rec.ah_inv_no,"______0_",txt_buffer,R_LONG);
	mkln(9,txt_buffer,8);
       	if(prnt_line() < 0) return(ERROR);
       	if(prnt_line() < 0) return(ERROR);
	if(strcmp(arhdr_rec.ah_type,T_CRMEMO)==0) {
#ifdef ENGLISH
		mkln(1,"*** CREDIT MEMO ***",19);
#else
		mkln(1,"*** NOTE DE CREDIT ***",22);
#endif
	}
	if(strcmp(arhdr_rec.ah_type,T_DBMEMO)==0) {
#ifdef ENGLISH
		mkln(1,"*** DEBIT MEMO ***",18);
#else
		mkln(1,"*** NOTE DE DEBIT ***",21); 
#endif
	}
       	if(prnt_line() < 0) return(ERROR);
       	if(prnt_line() < 0) return(ERROR);

	/* payable address */
	mkln(1,cu_rec.cu_name,30);
	mkln(47,sch_rec.sc_name,28);
       	if(prnt_line() < 0) return(ERROR);	/* name (line 1) */

	sch_rec.sc_numb = pa_rec.pa_distccno;
	retval = get_sch(&sch_rec,BROWSE,0,e_mesg);

			/* first line of address  */

	if (strcmp(cu_rec.cu_adr1,"\0")) { 
		mkln(1,cu_rec.cu_adr1,30);
	
		if (strcmp(sch_rec.sc_add1,"\0")) {
			mkln(47,sch_rec.sc_add1,30);
			sch_rec.sc_add1[0]='\0';
		}			
		else
		if (strcmp(sch_rec.sc_add2,"\0")) {
			mkln(47,sch_rec.sc_add2,30);
			sch_rec.sc_add2[0]='\0';
		}
		else
		if (strcmp(sch_rec.sc_add3,"\0")) {
			mkln(47,sch_rec.sc_add3,30);
			sch_rec.sc_add3[0]='\0';
		}
	 	else 
		if (strcmp(sch_rec.sc_pc,"\0")) {
			mkln(47,sch_rec.sc_pc,7);
			sch_rec.sc_pc[0]='\0';
		}
		curr_line++;       	
		if(prnt_line() < 0) return(ERROR);	
	}

			/* second line of address  */

	if (strcmp(cu_rec.cu_adr2,"\0")) { 
		mkln(1,cu_rec.cu_adr2,30);
	
		if (strcmp(sch_rec.sc_add1,"\0")) {
			mkln(47,sch_rec.sc_add1,30);
			sch_rec.sc_add1[0]='\0';
		}			
		else
		if (strcmp(sch_rec.sc_add2,"\0")) {
			mkln(47,sch_rec.sc_add2,30);
			sch_rec.sc_add2[0]='\0';
		}
		else
		if (strcmp(sch_rec.sc_add3,"\0")) {
			mkln(47,sch_rec.sc_add3,30);
			sch_rec.sc_add3[0]='\0';
		}
		else
		if (strcmp(sch_rec.sc_pc,"\0")) {
			mkln(47,sch_rec.sc_pc,7);
			sch_rec.sc_pc[0]='\0';
		}
		curr_line++;       	
		if(prnt_line() < 0) return(ERROR);	
	}

			/* third line of address  */

	if (strcmp(cu_rec.cu_adr3,"\0")) { 
		mkln(1,cu_rec.cu_adr3,30);
	
		if (strcmp(sch_rec.sc_add1,"\0")) {
			mkln(47,sch_rec.sc_add1,30);
			sch_rec.sc_add1[0]='\0';
		}			
		else	
		if (strcmp(sch_rec.sc_add2,"\0")) {
			mkln(47,sch_rec.sc_add2,30);
			sch_rec.sc_add2[0]='\0';
		}
		else	
		if (strcmp(sch_rec.sc_add3,"\0")) {
			mkln(47,sch_rec.sc_add3,30);
			sch_rec.sc_add3[0]='\0';
		}
		else	
		if (strcmp(sch_rec.sc_pc,"\0")) {
			mkln(47,sch_rec.sc_pc,7);
			sch_rec.sc_pc[0]='\0';
		}
		curr_line++;       	
		if(prnt_line() < 0) return(ERROR);	
	}

			/* final line of address  */

	if (strcmp(cu_rec.cu_pc,"\0")) { 
		mkln(1,cu_rec.cu_pc,10);
	
		if (strcmp(sch_rec.sc_add1,"\0")) {
			mkln(47,sch_rec.sc_add1,30);
			sch_rec.sc_add1[0]='\0';
		}			
		else	
		if (strcmp(sch_rec.sc_add2,"\0")) {
			mkln(47,sch_rec.sc_add2,30);
			sch_rec.sc_add2[0]='\0';
		}
		else	
		if (strcmp(sch_rec.sc_add3,"\0")) {
			mkln(47,sch_rec.sc_add3,30);
			sch_rec.sc_add3[0]='\0';
		}
		else	
		if (strcmp(sch_rec.sc_pc,"\0")) {
			mkln(47,sch_rec.sc_pc,7);
			sch_rec.sc_pc[0]='\0';
		}
		curr_line++;       	
		if(prnt_line() < 0) return(ERROR);	
	}


	if (strcmp(sch_rec.sc_add1,"\0")) {
		mkln(47,sch_rec.sc_add1,30);
		curr_line++;       	
		if(prnt_line() < 0) return(ERROR);	
	}			
	if (strcmp(sch_rec.sc_add2,"\0")) {
		mkln(47,sch_rec.sc_add2,30);
		curr_line++;       	
		if(prnt_line() < 0) return(ERROR);	
	}
	if (strcmp(sch_rec.sc_add3,"\0")) {
		mkln(47,sch_rec.sc_add3,30);
		curr_line++;       	
		if(prnt_line() < 0) return(ERROR);	
	}
	if (strcmp(sch_rec.sc_pc,"\0")) {
		mkln(47,sch_rec.sc_pc,7);
		curr_line++;       	
		if(prnt_line() < 0) return(ERROR);	
       	}

	for(; curr_line < 5; curr_line++)
		if(prnt_line() < 0) return(ERROR);	
			 
		/* school phone & fax numbers */

	mkln(52,sch_rec.sc_phone,3);	
	mkln(55,"-",1);		
	mkln(56,sch_rec.sc_phone+3,3);
	mkln(59,"-",1);
	mkln(60,sch_rec.sc_phone+6,4);
	mkln(70,sch_rec.sc_fax,3);
	mkln(73,"-",1);		
	mkln(74,sch_rec.sc_fax+3,3);
	mkln(77,"-",1);
	mkln(78,sch_rec.sc_fax+6,4);
       	if(prnt_line() < 0) return(ERROR);	
if(!SW1) {
#ifdef ENGLISH
	mkln(19,"Registration #: ",16);
#else
	mkln(19,"No. Matricule : ",16);
#endif
	mkln(35,pa_rec.pa_regist,10);	 
}
       	if(prnt_line() < 0) return(ERROR);
       	if(prnt_line() < 0) return(ERROR);

	mkln(7,arhdr_rec.ah_cu_code,10);
	longdate = conv_date(arhdr_rec.ah_trandt,6,4);
 	tedit((char *)&arhdr_rec.ah_trandt,"____/__/__",txt_buffer,R_LONG);
	mkln(38,txt_buffer,10);
if(SW1) {
	mkln(65,pa_rec.pa_regist,10);	 
}
else {
 	tedit((char *)&arhdr_rec.ah_inv_no,"______0_",txt_buffer,R_LONG);
	mkln(67,txt_buffer,8);
}
       	if(prnt_line() < 0) return(ERROR);
       	if(prnt_line() < 0) return(ERROR);
       	if(prnt_line() < 0) return(ERROR);

	return(NOERROR);
}
/* ------------------------------------------------------------------------ */
/* Function: Print Invoice Detail Part					    */
/* ------------------------------------------------------------------------ */
prnt_item_line()

/*	Function no longer being used.		JM 92/11/04  */

{
	char txt_line[20];
	int	flag;
	double	amount;
	
	if(strcmp(arhdr_rec.ah_type,T_CRMEMO) == 0)
		flag = -1;
	else
		flag = 1;
	
	mkln(7,aritem_rec.ai_desc,60);
	amount = aritem_rec.ai_amount * flag;
	tedit((char *)&amount,"___,_0_.__-",txt_line,R_DOUBLE);
	mkln(70,txt_line,11);
	
        if(prnt_line() < 0) return(ERROR);

	return(NOERROR);
}
/* ------------------------------------------------------------------------ */
/* Function: Print Invoice Total Part					    */
/* ------------------------------------------------------------------------ */
print_total(mode)
int	mode;	/* 0 - if multiple pages,  1 - if last page */
{
	int i=0;
	int flag;
	double	pst_amt, gst_amt, total;
	char txt_buffer[20];

	if(strcmp(arhdr_rec.ah_type,T_CRMEMO)==0)
		flag = -1;
	else
		flag = 1;

if(SW1) {
	for( ;linecnt < 39; ) {
		prnt_line();	
	}
}
else {
	for( ;linecnt < 36; ) {
		prnt_line();	
	}
}

	if(mode == 1) {

#ifdef ENGLISH
    	   	mkln(61,"GST :",5);
#else
		mkln(61,"TPS :",5);
#endif
		gst_amt = arhdr_rec.ah_gstamt * flag;
		tedit((char *)&gst_amt,"___,_0_.__-",txt_buffer,R_DOUBLE);
		mkln(70,txt_buffer, 11);		
       		if(prnt_line() < 0) return(ERROR);
#ifdef ENGLISH
		mkln(61,"PST :",5);
#else
		mkln(61,"TVP :",5);
#endif
		pst_amt = arhdr_rec.ah_txamt * flag;
		tedit((char *)&pst_amt,"___,_0_.__-",txt_buffer,R_DOUBLE);
		mkln(70,txt_buffer, 11);		
		if(prnt_line() < 0) return(ERROR);
if(!SW1) {
#ifdef ENGLISH
		mkln(43,"Please Pay This Amount: ",24);
#else
		mkln(43,"SVP remettre ce montant:",24);
#endif
}
		inv_total = inv_total+arhdr_rec.ah_gstamt+arhdr_rec.ah_txamt;
		total = inv_total * flag;
		tedit((char *)&total,"___,_0_.__-",txt_buffer,R_DOUBLE);
		mkln(70,txt_buffer,11);		
        	if(prnt_line() < 0) return(ERROR);
        	if(prnt_line() < 0) return(ERROR);
        	if(prnt_line() < 0) return(ERROR);
	}		
	if (mode != 1) {
if(SW1) {
		for (; linecnt < 43; ){
        		if(prnt_line() < 0) return(ERROR);
		}
}
else {
		for (; linecnt < 41; ){
        		if(prnt_line() < 0) return(ERROR);
		}
}
	}	

	for (i=0; ; i= i + 2) {
		if (!strcmp(inv_gl_acct[i], ""))
			break;
		else {
			mkln(1,inv_gl_acct[i],18);
			strcpy(inv_gl_acct[i],"");		
			if (!strcmp(inv_gl_acct[i+1], "")){
        			if(prnt_line() < 0) return(ERROR);
				break;
			}	
			else{ 
				mkln(20,inv_gl_acct[i+1],18);		
				strcpy(inv_gl_acct[i+1],"");		
        			if(prnt_line() < 0) return(ERROR);
			}
		}
		if (i > 10) return(NOERROR); 
	}	
	return(NOERROR);	
}
/* ------------------------------------------------------------------------ */
/* Function: Display Error Message   					    */
/* ------------------------------------------------------------------------ */
DisplayError(error)
char *error;
{
	fomer(error);
	roll_back(error);
}

INVConfirm()	/* returns 1 for yes, 0 for no, -1 for error */
{		/* Clears the profom screen when user's response is 'Y' */
	int	field;
	char	valans[3];

#ifdef ENGLISH
	if((retval = DisplayMessage("Confirm (Y/N)?"))<0 )
		return(retval);
	strcpy(valans, "YN");
#else
	if((retval = DisplayMessage("Confirmer (O/N)?"))<0 )
		return(retval);
	strcpy(valans, "ON");
#endif
	if((retval = GetResponse(answer, valans))<0 )
		return(retval);
	field = POOPT;		/* field number where print po option is */

	if(answer[0]==YES ){
		return(1);
	}
	/* Dehighlight the option which is highlighted in Process() */
	fomca1( field, 9, 5 );

	return( HideMessage() );
}
/*-------------------------- END OF FILE ---------------------------------- */
