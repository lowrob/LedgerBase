/* --------------------------------------------------------------------------
	SOURCE NAME:  invlbl20.c
	SYSTEM     :
	MODULE     : 
	CREATED    : 
	CREATED    :  J. Cormier

DESCRIPTION:

MODIFICATIONS:

PROGRAMMER           YY/MM/DD        DESCRIPTION OF MODIFICATIONS
^^^^^^^^^^           ^^^^^^^^        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
C.Leadbeater	     90/12/06        Added "GST TAX:" field on line 5.
M. Cormier	     90/12/17	     Added printer option for the user.
F. Tao               91/01/23	     Change starting margin and formats for 
				     District 20.
L.Robichaud	1993/10/13	Pass new parameter to close_rep function for 
				a banner to print with 3000 family.
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <repdef.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#ifdef ENGLISH
#define YES  'Y'
#define INVOICE "IN"
#define NO   'N'
#define POPRNT 'P'
#define LABEL 'L'
#else
#define YES  'O'
#define INVOICE "FC"
#define NO   'N'
#define POPRNT 'I'
#define LABEL 'E'
#endif

#define	EXIT	12

Ctl_rec  ctl_rec;
Po_hdr   po_hdr;
Po_item  po_item;
Supplier supp_rec;

extern   char 	 e_mesg[80];
static   int 	 retval;
static   short	 printer;
static   char    program[11];
static   int	 code;
static   int     code2;
static   char    text_line[132];
static   short   page_counter;
static   char    answer[2]; 
static   long    po_nbr1,po_nbr2;

inv_labels()
{
        int detailcnt = 0;
	int field;

	LNSZ = 132;

	printer = 1;
	if((retval = GetPrinter(&printer)) < 0)
		return(retval);

	po_nbr1 = 0;
	po_nbr2 = 99999999;
	retval = GetPoRange(&po_nbr1,&po_nbr2);
	if (retval < 0) return(retval);

	/* opening of output - printer */
	retval = opn_prnt("P",'\0',printer,e_mesg,0);  
	if(retval < 0) {
		fomer(e_mesg); 
		close_dbh() ;
		return(-1);
	}

	PGSIZE = 30;
	if (test_pattern()<0) {
		close_rep(NOBANNER);
		return(-1);
	}
    
	if ( (retval=Confirm())<= 0) 
		return(EXIT);

	po_hdr.ph_code = po_nbr1;
	flg_reset( POHDR );

	for( ; ; ) {
		code = get_n_pohdr(&po_hdr,UPDATE,0,FORWARD,e_mesg);
		if (code < 0) {
			if(code == EFL) break;
			return(code);  
		}
		if (po_hdr.ph_code == 0) continue;
		if (po_hdr.ph_code > po_nbr2) {
			roll_back(e_mesg);
			break;
		}
		if (po_hdr.ph_print[0] == NO && po_hdr.ph_code < 90000000 )  {
#ifndef ORACLE
			roll_back(e_mesg);
#endif
			continue;
		}
		if (po_hdr.ph_print[0] == LABEL)  {
			if (po_nbr1 != po_nbr2) {
#ifndef ORACLE
				roll_back(e_mesg);
#endif
				continue;
			}                                
			else {
#ifdef ENGLISH                                  
				sprintf(e_mesg,"LABEL FOR PO #: %ld ALREADY PRINTED. WANT TO PRINT IT AGAIN (Y/N)?",po_hdr.ph_code);
#else
				sprintf(e_mesg,"L'ETIQUETTE (BON DE COM. # %d) EST DEJA IMPRIMER. LA RE-IMPRIMER (O/N)?",po_hdr.ph_code);
#endif
				DisplayMessage(e_mesg);
				GetResponse(e_mesg);
				if (e_mesg[0] == NO) {
#ifndef ORACLE
					roll_back(e_mesg);
#endif
					continue;
				}
			}   
		}
/**
		STRCPY(supp_rec.s_supp_cd,po_hdr.ph_supp_cd);
		code = get_supplier(&supp_rec,BROWSE,0,e_mesg);
		if (code < 0 ) {
			stcpy(supp_rec.s_supp_cd,"??????????");
		}
**/

		ctl_rec.fund = po_hdr.ph_funds;
		code = get_ctl(&ctl_rec,BROWSE,0,e_mesg);
		if(code < 0) {
			return(code);
		}
		page_counter = 0;
		if ((retval = prnt_form_hdr()) < 0 )
			break;
	
		po_item.pi_code = po_hdr.ph_code;
		po_item.pi_item_no = 0;
		flg_reset( POITEM );

                detailcnt = 0;  
		for ( ; ; ) {
			code = get_n_poitem(&po_item,BROWSE,0,FORWARD,e_mesg);
			if (code < 0 ) {
				if(code == EFL) break;
				return(code);
			}
			if (po_hdr.ph_code != po_item.pi_code)    
				break;
	  	     /* if (linecnt >= PGSIZE-1) { */     
	  	        if (detailcnt >= 5) {        
		                rite_top();
                                detailcnt = 0;
				if ((retval = prnt_form_hdr()) < 0 )
					break;
			}
			if ((retval = prnt_itm_line()) < 0)
				break; 
                        detailcnt++;
		}
		seq_over(POITEM);
		if (retval < 0) break;
		rite_top();
		if ((po_hdr.ph_print[0] == YES) ||
			(po_hdr.ph_print[0] == POPRNT))
			po_hdr.ph_print[0] = LABEL; 
		if ((po_hdr.ph_print[0] == NO) &&
			(po_hdr.ph_code > 90000000)) 
			po_hdr.ph_print[0] = LABEL;
		retval = put_pohdr(&po_hdr,UPDATE,e_mesg);
		if (retval < 0) {
			DisplayError(e_mesg);
			break;
		}
		if(commit(e_mesg) < 0) {
			DisplayError(e_mesg);
			break;
		}
		po_hdr.ph_code++;
	}           		
	seq_over(POHDR);
	close_rep(NOBANNER);
	close_dbh();
	if (retval < 0) return(retval);
	return(0);
}




prnt_form_hdr()
{
	if(prnt_line() < 0) return(REPORT_ERR);       

#ifdef ENGLISH 
        mkln(2,"SUPP:",5);
	mkln(8,po_hdr.ph_supp_cd,10);
        mkln(19,"REF: _______________ TYPE:",26);
        mkln(46,"IN",2);
        mkln(50,"PO:",3);
	tedit((char *)&po_hdr.ph_code,"______0_",text_line,R_LONG);
	mkln(54,text_line,8);
#else
        mkln(2,"SUPP:",5);
	mkln(8,po_hdr.ph_supp_cd,10);
        mkln(19,"REF: _______________ TYPE:",26);
        mkln(46,"IN",2);
        mkln(50,"PO:",3);
	tedit((char *)&po_hdr.ph_code,"________",text_line,R_LONG);
	mkln(54,text_line,8);
#endif
	if(prnt_line() < 0) return(REPORT_ERR);  
        if(prnt_line() < 0) return(REPORT_ERR);       

#ifdef ENGLISH 
        mkln(2,"TRANS DT: __/__/__   DUE DT: __/__/__     PO TYPE:",50);
#else
        mkln(2,"TRANS DT: __/__/__   DUE DT: __/__/__     PO TYPE:",50);
#endif
        mkln(54,po_hdr.ph_type,1);
	if(prnt_line() < 0) return(REPORT_ERR);       
        if(prnt_line() < 0) return(REPORT_ERR);       

#ifdef ENGLISH
        mkln(2,"GROSS: __________    GST: _ __________",38); 
        mkln(44,"PST: _ __________",17);
#else
        mkln(2,"GROSS: __________    GST: _ __________",38); 
        mkln(44,"PST: _ __________",17);
#endif
	if(prnt_line() < 0) return(REPORT_ERR);       
	if(prnt_line() < 0) return(REPORT_ERR);       

#ifdef ENGLISH
        mkln(2,"DISC: _ __________   CHEQUE: ________",37);
        mkln(41,"PO COMPLETE: _",14);
#else
        mkln(2,"DISC: _ __________   CHEQUE: ________",37);
        mkln(41,"PO COMPLETE: _",14);
#endif
	if(prnt_line() < 0) return(REPORT_ERR);       
	if(prnt_line() < 0) return(REPORT_ERR);       

#ifdef ENGLISH
        mkln(2,"STOCK      G/L ACCOUNT         QUANTITY",39);
        mkln(46,"AMOUNT",6);
#else
        mkln(2,"STOCK      G/L ACCOUNT         QUANTITY",39);
        mkln(46,"AMOUNT",6);
#endif
	if(prnt_line() < 0) return(REPORT_ERR);       
	if(prnt_line() < 0) return(REPORT_ERR);       
	return(NOERROR);
}



prnt_itm_line()
{	
	/* line item code */
	/* if not high values */
	if(po_item.pi_st_code[0] != '\377') {
		mkln(2,po_item.pi_st_code,10);
	}
	mkln(13,po_item.pi_acct,18);
        mkln(33,"__________   __________",23);
	if (prnt_line() < 0) return(REPORT_ERR); /* line 6 to 25 max. approx */
	return(NOERROR);
}

test_pattern()
{
	char  answer1[2]; 
	for( ; ; )  {
#ifdef  ENGLISH
		if((retval=DisplayMessage("IS FORM ALIGNED (Y/N)?"))<0)
#else
		if((retval=DisplayMessage("FORMULAIRE EST-IL ALIGNE (O/N)?"))<0) 
#endif
			break;
		if ((retval = GetResponse(answer1)) <0) 
			break;
		if(answer1[0] == YES) break;
		retval = prnt_tst_ptrn();
		rite_top();
		if(retval < 0) break;              
	}     
	if (retval < 0) return(retval);

	return(NOERROR);
}

prnt_tst_ptrn()
{
   	short  cnt;
	if(prnt_line() < 0) return(REPORT_ERR);           

#ifdef	ENGLISH
        mkln(2,"SUPP:",5);
	mkln(8,"__________",10);
        mkln(19,"REF:",4);
	mkln(24,"_______________",15);  
        mkln(40,"TYPE:",5);
	mkln(46,"__",2);
        mkln(50,"PO:",3);
	mkln(54,"________",8);
#else
        mkln(2,"SUPP:",5);
	mkln(8,"**********",10);
        mkln(19,"REF:",4);
	mkln(24,"***************",15);  
        mkln(40,"TYPE:",5);
	mkln(46,"**",2);
        mkln(50,"PO:",3);
	mkln(54,"********",8);
#endif
	if(prnt_line() < 0) return(REPORT_ERR); /*** line 1 ***/         
	if(prnt_line() < 0) return(REPORT_ERR);       

#ifdef	ENGLISH
        mkln(2,"TRANS DT:",9);
	mkln(12,"__/__/__",8);
        mkln(23,"DUE DT:",7);
	mkln(31,"__/__/__",8);
        mkln(44,"PO TYPE:",8);
        mkln(53,"_",1);
#else
        mkln(2,"TRANS DT:",9);
	mkln(12,"YY/MM/DD",8);
        mkln(23,"DUE DT:",7);
	mkln(31,"YY/MM/DD",8);
        mkln(44,"PO TYPE:",8);
        mkln(53,"*",1);
#endif

	if(prnt_line() < 0) return(REPORT_ERR); /*** line 2 ***/         
	if(prnt_line() < 0) return(REPORT_ERR);       

#ifdef	ENGLISH
        mkln(2,"GROSS:",6);  
	mkln(9,"__________",10);
        mkln(23,"GST:",4); 
	mkln(28,"_ __________",12);
        mkln(44,"PST:",4);
	mkln(49,"_ __________",12);
#else
        mkln(2,"GROSS:",6);  
	mkln(9,"**********",10);
        mkln(23,"GST:",4); 
	mkln(28,"* **********",12);
        mkln(44,"PST:",4);
	mkln(49,"* **********",12);
#endif

	if(prnt_line() < 0) return(REPORT_ERR); /*** line 3 ***/         
	if(prnt_line() < 0) return(REPORT_ERR);          

#ifdef	ENGLISH
        mkln(2,"DISC:",5);
	mkln(8,"_ __________",12);
        mkln(23,"CHEQUE:",7);
	mkln(31,"________",8);
        mkln(41,"PO COMPLETE:",12);
	mkln(54,"_",1);
#else
        mkln(2,"DISC:",5);
	mkln(8,"* **********",12);
        mkln(23,"CHEQUE:",7);
	mkln(31,"********",8);
        mkln(41,"PO COMPLETE:",12);
	mkln(54,"*",1);
#endif

	if(prnt_line() < 0) return(REPORT_ERR);  /** line 4 ***/         
	if(prnt_line() < 0) return(REPORT_ERR);          

#ifdef ENGLISH
        mkln(2,"STOCK      G/L ACCOUNT         QUANTITY",39);
        mkln(46,"AMOUNT",6);
#else
        mkln(2,"STOCK      G/L ACCOUNT         QUANTITY",39);
        mkln(46,"AMOUNT",6);
#endif
	if(prnt_line() < 0) return(REPORT_ERR);       
	if(prnt_line() < 0) return(REPORT_ERR);       
	for (cnt = 0;cnt < 5; cnt++) {
		mkln(2,"___________",10); 
		mkln(13,"__________________",18);
		mkln(33,"___________",10);
		mkln(46,"___________",10);
		if(prnt_line() < 0) return(REPORT_ERR); /** line 6 to 25  **/         
	}

	return(NOERROR);
}
/*-------------------------- END OF FILE ---------------------------------- */

