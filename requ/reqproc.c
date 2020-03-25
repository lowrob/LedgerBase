/*
Programmer     dd/mm/yy              Description  		
**********     ********              ***********	

L. Robichaud   15/09/92		Get the grand total for multiple pages

    "          16/09/92		Page breaks for different requistions
				with their totals. 

D. Richardson  25/09/92		Modified the print_heading function to print
				the "Requested By" field on the issue slip. 
				Had to pass the Req_hdr pointer down from the
				req_process function to the PrintIssue function,
				then on to the print_heading function.

L. Robichaud   30/11/92		Modified the field that displays the Deliver To
				information to display the shipto field in the 
				requestion header file.

L. Robichaud   11/12/92		Allow interfunding for requsitions. A second 
				fund was added (stock fund) to allow this.
				Added function WriteTrhdr2 and WriteTritems2.

L. Robichaud   18/12/92         Remove printer option & print issue slips every
				time needed rather than at the end of a session.

L.Robichaud    09/09/93		Read the param file for the current period to 
				be written to for all transactions. Currently 
				the system was writing all transactions to the 
				period in the requisition header. I have
				replaced all the "reqh_ptr->period" with
				"pa_rec.pa_cur_period".

L.Robichaud   01/10/93		Check if the G/L record is Locked and keep 
				trying to lock it until succesful for every 
				read on the G/L file.
*/

#include	<stdio.h>
#include	<bfs_defs.h>
#include	<bfs_recs.h>
#include	<requ.h>
#include	<linklist.h>
#include	<repdef.h>

#define	NO	'N'
#define	YES	'Y'
#define	ROLLBACK	-1
#define	FALSE	0
#define	TRUE	1
#define DELTA_QTY	0.0005
#define IS	5

extern	Pa_rec	pa_rec;

static long	po_hdr_empty[MAX_PO]; /* flag to see if po header is empty  */
static long	stock_empty;	      /* flag to see if stock is empty */
static List	po_item_list[MAX_PO]; /* list of po items for each po */
static List	req_item_list[MAX_PO]; /* list of req items for each po */
static List	stock_list;	       /* list of req items for stock */
static Po_hdr	po_hdr[MAX_PO];	      /* the po headers to be created */
static Po_item  *po_item;	      /* a pointer to a po item */
static Ctl_rec	ctl_rec;	      /* control record */
static Last_po  lastpo_rec;	      /* Last Po number */
static short 	Dbdue_sect;	  /* Buffer to store section of db or cr acct */
static short 	Crdue_sect;	  /* Buffer to store section of db or cr acct 
				     for due to and from accounts */
St_mast		st_mast;	/* stock master record */
St_tran		st_tran,st_tmp;	/* stock transaction record */
Gl_rec 		gl_rec;		/* GL master record */
Alloc_rec	alloc_rec;	/* allocation file record */
Tr_hdr		tr_hdr, tr_tmp;	/* Gl transaction header record */
Tr_item		tr_item;	/* Gl transaction item record */

double D_Roundoff();
long write_pohdr();
long write_poitem();
long UpdateInventory();
long check_limit();
long get_payee();
long complete_reqs();
long req_rollback();

static int prnt_flg;
static int new_page;
static int issue_done;
static int printer_open;
static double grand_total;

static int pgcnt;
static int req_fund;
static int Dbacc_sect;
static int Cracc_sect;
static char filename[50];
static char Crfund_dtf[19], Dbfund_dtf[19];

long
init_printer(reqi_ptr, reqh_ptr, e_mesg)
Req_item *reqi_ptr;
Req_hdr *reqh_ptr;
char	*e_mesg;
{
	int	err;

	sprintf(filename,"%ld.iss",reqh_ptr->code);
	err = opn_prnt("F",filename,1,e_mesg,0);  
	if(err < 0) {
		return(ERROR);
	}
	grand_total = 0;
	prnt_flg = TRUE;
	PGSIZE = 60;
	LNSZ = 80;
	linecnt = PGSIZE;
	new_page = FALSE;
	issue_done = TRUE;
	printer_open = TRUE;
	return(NOERROR);
}
long
close_printer()
{
	char	devname[80];
	char	command[80];

	print_footer();
	close_rep(BANNER);
	printer_open = FALSE;

	sprintf(command,"lp -s -o nobanner %s",filename);
	system(command);
	
	return(NOERROR);
}
/****************************************************************************/
/*	This routine sets the po header empty flag and the po items list    */
/*	flag to empty.							    */
/*	po_hdr_empty: 0 - is empty  1 - is occupied.			    */
/*	po_item_list: NULL - is empty  any this else is occupied.	    */
/****************************************************************************/
long
init_po()
{
	long	i;
	for(i=0;i<MAX_PO;i++) {
		po_hdr_empty[i] = PO_HDR_EMPTY;

		po_hdr[i].ph_comm = 0.00; /* set po commitments to zero */

		po_item_list[i] = list_make(sizeof(Po_item));
		if(po_item_list[i] == NOLIST) {
			return(ERROR);
		}

		req_item_list[i] = list_make(sizeof(Req_item));
		if(req_item_list[i] == NOLIST) {
			return(ERROR);
		}
	}
	stock_empty = TRUE;
	stock_list = list_make(sizeof(Req_item));
	if(stock_list == NOLIST) {
		return(ERROR);
	}

	return(NOERROR);
}

/****************************************************************************/
/*	This routine processes all information dealing with the created     */
/*	po's. It checks to see if the po is occupied. If so writes the po   */
/*	to the file. If it is a non-bulk po it creates the allocations. It  */
/*	also writes the gl commitments. It also updates the control file    */
/*	with the last po number.					    */
/****************************************************************************/
long
complete_po(po_nos,e_mesg)
long	po_nos[];
char	*e_mesg;
{
	long	i;
	long	err;
	short 	po_item_no;
	Po_item *item_ptr;

	/* Get and Lock control record for updating last po number */
	ctl_rec.fund = 1;
	err = get_ctl(&ctl_rec,BROWSE,0,e_mesg);
	if(err < 0) {
		return(err);
	}

	err = get_lastpo(&lastpo_rec,UPDATE, 1, e_mesg) ;
	if(err < 0) {
		DispError(e_mesg);
		return(err) ;
	}

	/* if i == 0 - po is Direct */
	/* if i == 1 - po is Non-Bulk */
	for(i=0;i < MAX_PO;i++) {
		if(po_hdr_empty[i] == PO_HDR_EMPTY) {
			po_nos[i] = 0;
			continue;	/* skip po because it is empty */
		}
		else {
			lastpo_rec.last_po++; /* update last po number */
		}	
		item_ptr = list_get(po_item_list[i],FIRST_NODE);
		if(item_ptr==NOOBJ) {
			return(ERROR);
		}

		po_hdr[i].ph_code = lastpo_rec.last_po;
		err = write_pohdr(&po_hdr[i],&po_nos[i],e_mesg);
		if(err != NOERROR) {
			return(err);
		}
		/* this is incase po number is already taken */
		/* and a new one is generated in write_pohdr() */
		lastpo_rec.last_po = po_hdr[i].ph_code;
		
		po_item_no = 0;
		for(;item_ptr!=NOOBJ;
				item_ptr=list_get(po_item_list[i],NEXT_NODE)) {
			po_item_no++;
			item_ptr->pi_code = po_hdr[i].ph_code;
			item_ptr->pi_item_no = po_item_no;
			err = write_poitem(item_ptr,i,e_mesg);
			if(err != NOERROR) {
				return(err);
			}
		}
	}
	err = complete_reqs(po_nos,e_mesg);
	if(err < 0) {
		return(err);
	}

	err = put_lastpo(&lastpo_rec, UPDATE, 1, e_mesg) ;
	if(err < 0) {
		return(err);
	}

	err = commit(e_mesg) ;
	if(err < 0) {
#ifdef ENGLISH
		sprintf(e_mesg,"ERROR in Saving Records"); 
#else
		sprintf(e_mesg,"ERREUR en conservant les fiches");
#endif
		return(err);
	}

	unlock_file(GLTRHDR);
	close_file(GLTRHDR);
	return(NOERROR);
}

/****************************************************************************/
/*	Write Po header to file and generate a new po number if current     */
/*	Po number Already exists.					    */
/****************************************************************************/
long
write_pohdr(po_header,po_no,e_mesg)
Po_hdr	*po_header;
long	*po_no;
char	*e_mesg;
{
	long	err;

	for(;(err = put_pohdr(po_header,ADD,e_mesg)) == DUPE;) {
		po_header->ph_code++;
		
	}
	if(err < 0) {
		return(err);
	}

	*po_no = po_header->ph_code;
	return(NOERROR);
}

/****************************************************************************/
/*	Write Po item to file and write allocation in a stock related item. */
/*	Also write the G/L commitments.					    */
/****************************************************************************/
long
write_poitem(item_ptr,type,e_mesg)
Po_item	*item_ptr;
long	type;		/* po type: 0-Direct, 1-Non-Bulk */
char	*e_mesg;
{
	int i;
	long err;
	Gl_rec	gl_rec;
	Tax_cal	tax_cal;

	/* Write PO item to file */
	err = put_poitem(item_ptr,ADD,e_mesg);
	if(err < 0) {
		return(err);
	}

	/* Write Allocation and Update Stock master record */
	if(type!=MERGE_DIRECT && pa_rec.pa_stores[0] != NO) {
		err = UpdateInventory(item_ptr,type,e_mesg);
		if(err < 0) {
			return(err);
		}
	}

	/* Write Commitments */
	gl_rec.funds =  item_ptr->pi_fund;  
	strcpy(gl_rec.accno,item_ptr->pi_acct);
	gl_rec.reccod = 99;
	for(i=1;;i++){ 	/* Keep trying to get G/L record L.R. */
		err = get_gl(&gl_rec,UPDATE,0,e_mesg);
		if(err == LOCKED){
			fomce();
			fomen(e_mesg);
			getchar();
			fomce();
			sprintf(e_mesg, "Retry: %d - Press any key",i);
			fomen(e_mesg);
			getchar();
			fomce();
		}
		else
			break;
	}
	if(err < 0) {
		roll_back(e_mesg);
		return(err);
	}
	
	gl_rec.comdat += calc_commit(item_ptr->pi_tax1,item_ptr->pi_tax2,
				ctl_rec.rebate,item_ptr->pi_original,&tax_cal);

	gl_rec.comdat = D_Roundoff(gl_rec.comdat);
	err = put_gl(&gl_rec,UPDATE,e_mesg);
	if(err < 0) {
		roll_back(e_mesg);
		return(err);
	}

	return(NOERROR);
}

/****************************************************************************/
/*	Update Inventory Files.                                             */
/*	Write Allocation if NON-BULK and update stock master record stock   */
/*	allocated and stock on order.				    	    */
/****************************************************************************/
long
UpdateInventory(item_ptr,type,e_mesg)
Po_item	*item_ptr;
long	type;
char	*e_mesg;
{
	int	mode;
	long	err;
	Tax_cal	tax_cal;
	Alloc_rec	aloc_rec;
	St_mast		st_rec;
	double	qty;

	calctax(item_ptr->pi_tax1,item_ptr->pi_tax2,item_ptr->pi_original,
			&tax_cal);
	
	/* Update Stock Master */
	st_rec.st_fund =  item_ptr->pi_fund;  
	strcpy(st_rec.st_code,item_ptr->pi_st_code);	
	err = get_stmast(&st_rec,UPDATE,0,e_mesg);
	if(err < 0) {
		return(err);
	}
	
	if(pa_rec.pa_poinv[0] == YES) {
		st_rec.st_po_ordqty += item_ptr->pi_orig_qty;
		if (st_rec.st_po_ordqty < DELTA_QTY) 
			st_rec.st_po_ordqty = 0.00;
		st_rec.st_committed += calc_commit(item_ptr->pi_tax1,
			item_ptr->pi_tax2,
			ctl_rec.rebate,item_ptr->pi_original,&tax_cal);
		if (st_rec.st_committed < DELTA_QTY) 
			st_rec.st_committed = 0.00;
	}
	st_rec.st_on_order += item_ptr->pi_orig_qty;
	st_rec.st_on_order = D_Roundoff(st_rec.st_on_order);
	st_rec.st_po_ordqty = D_Roundoff(st_rec.st_po_ordqty);
	st_rec.st_committed = D_Roundoff(st_rec.st_committed);

	if(pa_rec.pa_poinv[0] == YES) {
		qty = (st_rec.st_on_hand + st_rec.st_paidfor +
			st_rec.st_po_ordqty) ;
		if(qty > DELTA_QTY) {
			st_rec.st_rate  = 
			(st_rec.st_value + st_rec.st_committed) / qty;
		}
	}
	else {
		qty = (st_rec.st_on_hand + st_rec.st_paidfor);
		if(qty > DELTA_QTY) {
			st_rec.st_rate  = st_rec.st_value / qty;
		}
	}
	err = put_stmast(&st_rec,UPDATE,e_mesg);
	if(err < 0) {
		return(err);
	}
	
	return(NOERROR);
}
/************************************************************************/
/* 	The following routine takes in two PO numbers and fills the	*/
/* PO number field of each of the processed requisition lines for that	*/
/* PO.  If any of the updates fail, an ERROR and message is returned.	*/
/************************************************************************/
long
complete_reqs(po_nos,emesg)

long	po_nos[MAX_PO];	/* i: The PO numbers used for the POs.	*/
char	*emesg;		/* r: The error message if there is an error.	*/

{
	long i;
	Req_item	*ptr;
	
	/* For each PO,	*/
	for (i=0; i<MAX_PO; i++)
	{
		/* If the PO header is empty, go to next PO.	*/
		if (po_hdr_empty[i] == PO_HDR_EMPTY)
		{
			continue;
		}

		/* For each requisition line item in the list.	*/
		for (ptr=list_get(req_item_list[i],FIRST_NODE); ptr != NOOBJ;
			ptr=list_get(req_item_list[i],NEXT_NODE) )
		{
			/* Get the requisition item in update mode.	*/
			if (get_reqitem(ptr,UPDATE,0,emesg) != NOERROR)
			{
				return(ERROR);
			}

			/* Copy in the PO number.	*/
			ptr->pocode = po_nos[i];
			ptr->appstat[0] = COMPLETE;

			/* Write the updated requisition item.	*/
			if (put_reqitem(ptr,UPDATE,emesg) != NOERROR)
			{
				return(ERROR);
			}
		}
	}
	if(stock_empty == FALSE) {
		/* For each requisition line item in the list.	*/
		for (ptr=list_get(stock_list,FIRST_NODE); ptr != NOOBJ;
			ptr=list_get(stock_list,NEXT_NODE) )
		{
			/* Get the requisition item in update mode.	*/
			if (get_reqitem(ptr,UPDATE,0,emesg) != NOERROR)
			{
				return(ERROR);
			}

			/* Copy in the PO number.	*/
			ptr->appstat[0] = COMPLETE;

			/* Write the updated requisition item.	*/
			if (put_reqitem(ptr,UPDATE,emesg) != NOERROR)
			{
				return(ERROR);
			}
		}

	}

	return(NOERROR);
}
/*----------------------------------------------------------------------*/
/* 	The following routine processes a requisition, adding the line	*/
/* items to either a direct or a non-bulk PO.				*/
/*----------------------------------------------------------------------*/
long
req_process(reqh_ptr,emesg)

Req_hdr	*reqh_ptr;	/* i: Header of requisition to be processed.	*/
char	*emesg;		/* r: Error message if there is an error.	*/

{
	long	i;			/* Loop index.	*/
	double	line_item_total;	/* Total amount of line items with
					   the same stock code;		*/
	char	payee[11];		/* Payee code.	*/
	Po_item	po_item;		/* PO line item.	*/
	Po_item	*poi_ptr;		/* PO line item pointer.	*/
	long	po_type;		/* Type of the PO: Direct, Non-bulk. */
	Req_item	req_item;	/* Requisition line item.	*/
	Req_item	*reqi_ptr;	/* Requisition line item pointer. */
	long	retcode;		/* Return code.	*/
	Tax_cal	taxes;			/* Taxes on line items.		*/
	long	po_added_to[MAX_PO];	/* Records if this requisition
					   added lines to the PO.	*/
	long	req_complete;		/* req comlete flag */
	int	err;

	issue_done = FALSE;
	printer_open = FALSE;

	/* Initialize whether the POs have been added to.	*/
	for (i=0; i<MAX_PO; i++)
	{
		po_added_to[i] = FALSE;
	}
	req_complete = TRUE;	/* always complete */

	/* Set up for the reading of the line items.	*/
	req_item.code = reqh_ptr->code;
	req_item.item_no = 0;

	/* For each requisition line item.	*/
	for (flg_reset(REQITEM) ; (retcode = get_n_reqitem((char*)&req_item,BROWSE,0,FORWARD,emesg)) == NOERROR ; )
	{
		/* If the requisition number does not match the	current one
		   then the requisition has been fully processed, so quit. */
		if (req_item.code != reqh_ptr->code)
		{
			new_page = TRUE;
			break;
		}

		/* If the line item is not approved, go to the next one. */
		if (req_item.appstat[0] == COMPLETE || 
		    req_item.appstat[0] == DISAPPROVED ||
		    /* req_item.appstat[0] == BACKORDER || */
		    req_item.appstat[0] == ' ' ||
		    req_item.appstat[0] == '\0') {
			if (req_item.appstat[0] == '\0' ||
		    	    /* req_item.appstat[0] == BACKORDER || */
			    req_item.appstat[0] == ' ') {
				req_complete = FALSE;
			}
			/* if (req_item.appstat[0] == BACKORDER) {
				err = PrintIssue(&req_item,reqh_ptr,emesg);
				if(err < 0) return(err);
			} */
			continue;
		}
		if (req_item.appstat[0] == STOCK) {
			/* A.Cormier Oct. 28/92 Try to Lock GL Trans file
			 until successful */
			for( ; ; ) {
				if((err = lock_file(GLTRHDR)) < 0) {
					if(err == LOCKED) { 
						continue;
					}
					DispError(emesg);
					roll_back(emesg);
					return(err);
				}
				else break;
			}
			/***********************************************/

			err = UpdateStock(reqh_ptr,&req_item,emesg);
			if(err < 0) return(err);
			err = PrintIssue(&req_item,reqh_ptr,emesg);
			if(err < 0) return(err);
			/* err = UpdateStock(reqh_ptr,&req_item,emesg);
			if(err < 0) return(err); */

			/* Add requisition line item to linked list.	*/
			list_set(stock_list,LAST_NODE);
			reqi_ptr = list_add(stock_list,&req_item,AFTER);

			/* Mark Stock as added to.	*/
			stock_empty = FALSE;
			continue;
		}
		/* Initialize total stock amount. */
		line_item_total = 0.0;

		/* Initialize default PO type as direct. */
		po_type = MERGE_DIRECT;

		/* If the line item has a stock code	*/
		if (req_item.st_code[0] != '\0')
		{
			/* Set PO type as non-bulk.	*/
			po_type = MERGE_NONBULK;

			/* For each line item of the non-bulk PO,	*/
			for (poi_ptr=list_get(po_item_list[MERGE_NONBULK],FIRST_NODE); poi_ptr != NOOBJ; poi_ptr=list_get(po_item_list[MERGE_NONBULK],NEXT_NODE));
			{
				/* If the stock code matches, add the line
				   item amount to the total.	*/
				if (!strcmp(req_item.st_code,poi_ptr->pi_st_code))
				{
					calctax(poi_ptr->pi_tax1,poi_ptr->pi_tax2,poi_ptr->pi_original,&taxes);
				}
			}
		}

		/* Add amount for current requisition line item.	*/
		calctax(req_item.tax1,req_item.tax2,req_item.value,&taxes);
		line_item_total = D_Roundoff(line_item_total + taxes.gros_amt);

		/* If rollback is selected.	*/
		if (check_limit(line_item_total,req_item.desc) == ROLLBACK)
		{
			/* Rollback requisition.	*/
			req_rollback(req_item.code);

			/* Return with no errors.	*/
			return(NOERROR);
		}

		/* Create new PO line item.	*/
		po_item.pi_school = req_item.school; 
		po_item.pi_fund = req_item.fund; /*from fund to st fund*/ 
		/* po_item.pi_fund = req_item.stock_fund; */
		strcpy(po_item.pi_acct,req_item.acct); 
		strcpy(po_item.pi_st_code,req_item.st_code); 
		strcpy(po_item.pi_desc,req_item.desc); 
		po_item.pi_req_no = req_item.code; 
		strcpy(po_item.pi_unit,req_item.unit); 
		strcpy(po_item.pi_tax1,req_item.tax1); 
		strcpy(po_item.pi_tax2,req_item.tax2); 
		po_item.pi_pd_qty = 0.0; 
		po_item.pi_unitprice = req_item.unitprice; 
		po_item.pi_orig_qty = req_item.orig_qty; 
		po_item.pi_original = req_item.value; 
		po_item.pi_paid = 0.0; 
		po_item.pi_value = req_item.value; 

		/* Add new PO line item to linked list.	*/
		list_set(po_item_list[po_type],LAST_NODE);
		poi_ptr = list_add(po_item_list[po_type],&po_item,AFTER);

		/* Add requisition line item to linked list.	*/
		list_set(req_item_list[po_type],LAST_NODE);
		reqi_ptr = list_add(req_item_list[po_type],&req_item,AFTER);

		/* Mark PO as added to.	*/
		po_added_to[po_type] = TRUE;

		/* Add the committed amount to the PO.	*/
		po_hdr[po_type].ph_comm = D_Roundoff(po_hdr[po_type].ph_comm + taxes.gros_amt);
	}


	if (issue_done == TRUE)
		close_printer();

	/* If there was a read error on the requisition line items,	*/
	if (retcode != NOERROR && retcode != EFL)
	{
		/* Rollback the requisition.	*/
		req_rollback(reqh_ptr->code);

		/* Return with an error.	*/
		return(ERROR);
	}

	/* For each PO,	*/
	for (i=0; i<MAX_PO; i++)
	{
		/* If the PO has been added to.	*/
		if (po_added_to[i] == TRUE)
		{
			/* If PO header is empty, fill in the PO header. */
			if (po_hdr_empty[i] == PO_HDR_EMPTY)
			{
				strcpy(po_hdr[i].ph_supp_cd,reqh_ptr->supp_cd);

				if (get_payee(reqh_ptr->supp_cd,payee,emesg) != NOERROR)
				{
					/* Rollback the requisition.	*/
					req_rollback(reqh_ptr->code);

					/* Return the error.	*/
					return(ERROR);
				}
				
				strcpy(po_hdr[i].ph_payee,payee);

				po_hdr[i].ph_billto = reqh_ptr->billto;

				po_hdr[i].ph_shipto = reqh_ptr->shipto;
/***** `pa_rec.pa_wareccno;  ****/

				strcpy(po_hdr[i].ph_attention,reqh_ptr->attention);

				sprintf(po_hdr[i].ph_req_no,"\0");

				po_hdr[i].ph_status[0] = OPEN;
				po_hdr[i].ph_status[1] = '\0';

				if (i == MERGE_DIRECT)
				{
					po_hdr[i].ph_type[0] = DIRECT;
				}
				else if (i == MERGE_NONBULK &&
				  reqh_ptr->costcenter != pa_rec.pa_wareccno )
				{
					po_hdr[i].ph_type[0] = NON_BULK;
				}
				else {
					po_hdr[i].ph_type[0] = BULK;
				}
				po_hdr[i].ph_type[1] = '\0';

				po_hdr[i].ph_print[0] = NO;
				po_hdr[i].ph_print[1] = '\0';

				po_hdr[i].ph_date = get_date();

				/* If the requisition due date has already
				   passed, then set the PO due date to the
				   entry date.				*/
				if (po_hdr[i].ph_date > reqh_ptr->due_date)
				{
					po_hdr[i].ph_due_date = po_hdr[i].ph_date;
				}
				else
				{
					po_hdr[i].ph_due_date = reqh_ptr->due_date;
				}

				po_hdr[i].ph_funds = reqh_ptr->funds;
				/* changed from fund to stock fund */

				po_hdr[i].ph_period = pa_rec.pa_cur_period;

				po_hdr[i].ph_lqdate = 0;

				po_hdr[i].ph_lqamt = 0.0;
	
				po_hdr_empty[i] = PO_HDR_OCCUPIED;
			}
			/* Else update the PO header.	*/
			else
			{
			/***** If the shipto's are different, ship to the
				   billto cost center.	
				if (po_hdr[i].ph_shipto != reqh_ptr->shipto)
				{
					po_hdr[i].ph_shipto = po_hdr[i].ph_billto;
					strcpy(po_hdr[i].ph_attention,"");
					strcpy(po_hdr[i].ph_req_no,"");
				}
******/
				po_hdr[i].ph_shipto = reqh_ptr->shipto;
/******** pa_rec.pa_wareccno; ****/
				strcpy(po_hdr[i].ph_attention,"");
				strcpy(po_hdr[i].ph_req_no,"");

				/* If this requisition has an earlier due date
				   then replace the due date.	*/
				if (po_hdr[i].ph_due_date > reqh_ptr->due_date)
				{
					/* If the requisition due date has
			 		   already passed, then set the PO due
					   date to the entry date.	*/
					if (po_hdr[i].ph_date > reqh_ptr->due_date)
					{
						po_hdr[i].ph_due_date = po_hdr[i].ph_date;
					}
					else
					{
						po_hdr[i].ph_due_date = reqh_ptr->due_date;
					}
				}
			}
		}
	}

	/* Mark the requisition as processed.	*/
	if(req_complete == TRUE) {
		reqh_ptr->status[0] = PROCESSED;
	}
	else {
		reqh_ptr->status[0] = OPEN;
	}
	/* Update the requisition header.	*/
	retcode = put_reqhdr(reqh_ptr,UPDATE,emesg);
	if (retcode != NOERROR)
	{
		return(ERROR);
	}
	return(NOERROR);
}

/*----------------------------------------------------------------------*/
/* 	The following routine checks if the line item total is higher	*/
/* than the purchasing limit.  If it is, the user is allowed to accept	*/
/* or reject the requisition.  The routine returns ROLLBACK if the item	*/
/* is rejected; otherwise NOERROR is returned.				*/
/*----------------------------------------------------------------------*/
long
check_limit(item_total,item_desc)

double	item_total;	/* i: Total of line items with same stock code. */
char	*item_desc;	/* i: Description of line item.	*/

{
	char	cmesg[79];	/* Message to be displayed on screen.	*/
	char	imesg[80];	/* Message to be displayed on screen.	*/

	/* If total amount exceeds regulations for line items.	*/
	if (item_total > pa_rec.pa_purlimit)
	{
		/* Create confirmation message.	*/
#ifdef	ENGLISH
		strcpy(cmesg,"Item total exceeds purchasing regulations.  Accept (Y/N)?");
#else
		strcpy(cmesg,"Total d'article depasse les regles d'achats.  Accepter (O/N)?");
#endif

		/* Create info message.	*/
#ifdef	ENGLISH
		sprintf(imesg,"Item: %50s   Total: %.2lf",item_desc,item_total);
#else
		sprintf(imesg,"Item: %50s   Total: %.2lf",item_desc,item_total);
#endif

		/* Display info message.	*/
		fomen(imesg);

		/* Prompt for rollback.	*/
		if (confirm(cmesg) == NO)
		{
			return(ROLLBACK);
		}
	}

	return(NOERROR);
}

/*----------------------------------------------------------------------*/
/* 	The following routine rolls back the current requisition so that*/
/* it will not be included in the PO.					*/
/*----------------------------------------------------------------------*/
long
req_rollback(req_no)
long	req_no;		/* i: Number of requisition to be rolled back. */
{
	long	i;	/* Loop index.	*/
	Po_item	*poi_ptr;	/* Pointer to a PO item line.	*/
	Tax_cal	taxes;	/* Data structure for the calculation of taxes.	*/

	/* For each PO,	*/
	for (i=0; i<MAX_PO; i++)
	{
		/* For each line item of the PO,	*/
		for (poi_ptr=list_get(po_item_list[i],FIRST_NODE);
			poi_ptr != NOOBJ;
			poi_ptr=list_get(po_item_list[i],NEXT_NODE));
		{
			/* If the PO item was from the requisition.	*/
			if (poi_ptr->pi_req_no == req_no)
			{
				/* Remove the committed amount from the PO. */
				calctax(poi_ptr->pi_tax1,poi_ptr->pi_tax2,poi_ptr->pi_original,&taxes);
				po_hdr[i].ph_comm = D_Roundoff(po_hdr[i].ph_comm - taxes.gros_amt);
				
				/* Remove the PO line item.	*/
				list_remove(po_item_list[i]);
			}
		}
	}
	return(NOERROR);
}

/*----------------------------------------------------------------------*/
/* 	The following routine retrieves the payee code given the 	*/
/* supplier code.							*/
/*----------------------------------------------------------------------*/
long
get_payee(supp_code,payee_code,emesg)

char	*supp_code;	/* i: Supplier code. */
char	*payee_code;	/* r: Payee code. */
char	*emesg;		/* r: Error message if there is an error.	*/

{
	long	retcode;	/* Return code.	*/
	Supplier	supplier;	/* Supplier record.	*/

	/* Read the supplier record for the given supplier.	*/
	strcpy(supplier.s_supp_cd,supp_code);
	retcode = get_supplier(&supplier,BROWSE,0,emesg);

	/* If it does not exist, set the payee to the NULL string.	*/
	if (retcode != NOERROR)
	{
		*payee_code = '\0';
		return(ERROR);
	}

	/* Copy the payee code from the supplier record.	*/
	strncpy(payee_code,supplier.s_payee,10);
	return(NOERROR);
}
PrintIssue(reqi_ptr,reqh_ptr,e_mesg)
Req_item *reqi_ptr;
Req_hdr *reqh_ptr;
char	*e_mesg;
{
	int	err;

	if(printer_open == FALSE){
		err = init_printer(reqi_ptr, reqh_ptr, e_mesg);
		if(err < 0) return(err);
	}

	if(linecnt >= PGSIZE || new_page == TRUE) {
		if(print_heading(reqi_ptr,reqh_ptr,e_mesg) <0) return(ERROR);
		new_page = FALSE;
	}

	if(print_item(reqi_ptr) < 0) return(ERROR);

	return(NOERROR);
}
static int
print_heading(reqi_ptr,reqh_ptr,e_mesg)
Req_item	*reqi_ptr;
Req_hdr		*reqh_ptr;
char	*e_mesg;
{
	char	txt_line[80];
	long	longdate ;
	int	i ;
	Sch_rec sch_rec;
	int	retval;
	
	linecnt = 0;
	
	if(pgcnt) rite_top();   /* if not first page advance */

	i = strlen(pa_rec.pa_co_name);
	mkln(((LNSZ-1-i)/2)+1,pa_rec.pa_co_name,sizeof(pa_rec.pa_co_name)); 
	longdate = get_date();

#ifdef ENGLISH
        mkln(64,"DATE:",5);
#else
        mkln(64,"DATE:",5);
#endif
	tedit((char *)&longdate,"____/__/__",txt_line,R_LONG);
	mkln(70,txt_line,10);
	if(prnt_line() < 0) return(ERROR);
	
#ifdef ENGLISH
	mkln(32,"STOCK ISSUE SLIP",16);
#else
	mkln(32,"",37);
#endif
	pgcnt++;
#ifdef ENGLISH
	mkln(64,"PAGE:",5);
#else
	mkln(64,"PAGE:",5);
#endif
	tedit((char *)&pgcnt,"__0_",txt_line,R_INT);
	mkln(70,txt_line,4);
	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
	mkln(1,"REQUISITION",11);
#else
#endif
	tedit((char *)&reqi_ptr->code,"______0_",txt_line,R_LONG);
	mkln(14,txt_line,8);
	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);
	
#ifdef ENGLISH
	mkln(1,"DELIVER TO",10);
	mkln(40,"REQUESTED BY",12);   /*  Added Sept 25/92  */
#else
 	mkln(1,"",8);
#endif
	if(prnt_line() < 0) return(ERROR);

	/* format school to ship to address */
	sch_rec.sc_numb = reqh_ptr->shipto; /* L.R. 30/11/92 */ 
	retval = get_sch(&sch_rec,BROWSE,0,e_mesg);
	if( retval < 0 ){
		return(ERROR);
	}

	mkln(4,sch_rec.sc_name,28);
	mkln(44,reqh_ptr->attention,16);   /*  Added Sept 25/92  */
	if(prnt_line() < 0) return(ERROR);

	if(sch_rec.sc_add1[0] != '\0') {
		mkln(4,sch_rec.sc_add1,28);
		if(prnt_line() < 0) return(ERROR);
	}
	if(sch_rec.sc_add2[0] != '\0') {
		mkln(4,sch_rec.sc_add2,28);
		if(prnt_line() < 0) return(ERROR);
	}
	if(sch_rec.sc_add3[0] != '\0') {
		mkln(4,sch_rec.sc_add3,28);
		if(prnt_line() < 0) return(ERROR);
	}
	if(sch_rec.sc_pc[0] != '\0') {
		mkln(4,sch_rec.sc_pc,8);
		if(prnt_line() < 0) return(ERROR);
	}

	if(prnt_line() < 0) return(ERROR);

	mkln(2,"WAREHOUSE",9);
	mkln(15,"QUANTITY",8);
	if(prnt_line() < 0) return(ERROR);

	mkln(2,"SIGNATURE",9);
	mkln(15,"ORDERED",7);
	mkln(27,"G/L ACCOUNT",11);
	mkln(46,"STOCK CODE",10);
	mkln(59,"UNIT COST",9);
	mkln(73,"TOTAL",5);
	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);

	return(NOERROR);
}
static int
print_item(reqi_ptr)
Req_item	*reqi_ptr;
{
	Tax_cal	tax_cal;
	char	txt_line[80];
	double	amount;
	
	/* if(reqi_ptr->appstat[0] == BACKORDER) {
		mkln(2,"BACKORDER",9);
	}
	else { */
		mkln(2,"__________",10);
	/* } */
	tedit((char *)&reqi_ptr->orig_qty,"__0_.__",txt_line,R_DOUBLE);
	mkln(15,txt_line,7);
	mkln(25,reqi_ptr->acct,18);
	mkln(46,reqi_ptr->st_code,10);
	tedit((char *)&reqi_ptr->unitprice,"___0_.___",txt_line,R_DOUBLE);
	mkln(59,txt_line,9);
	amount = calc_commit(reqi_ptr->tax1,reqi_ptr->tax2,
				ctl_rec.rebate,reqi_ptr->value,&tax_cal);

	grand_total += amount;

	tedit((char *)&amount,"___,_0_.__",txt_line,R_DOUBLE);
	mkln(70,txt_line,10);
	if(prnt_line() < 0) return(ERROR);

	mkln(5,"DESCRIPTION",11);
	mkln(18,reqi_ptr->desc,60);
	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);

	return(NOERROR);
}

static int
print_footer()
{
	char	txt_line[80];

	if(prnt_line() < 0) return(ERROR);
	mkln(68,"============",12);
	if(prnt_line() < 0) return(ERROR);

	/* Print the grand total */
	mkln(50,"Grand Total: ",13);
	tedit((char *)&grand_total,"___,___,_0_.__",txt_line,R_DOUBLE);
	mkln(66,txt_line,14);
	if(prnt_line() < 0) return(ERROR);
	grand_total = 0;
	pgcnt = 0;


	mkln(2,"AUTHORIZED SIGNATURE: ____________________",42);

	if(prnt_line() < 0) return(ERROR);
	/* rite_top(); */
	return(NOERROR);
}

UpdateStock(reqh_ptr,reqi_ptr,e_mesg)
Req_hdr  *reqh_ptr;
Req_item *reqi_ptr;
char	*e_mesg;
{
	int	 retval;

	ctl_rec.fund = reqh_ptr->stock_fund;     /* current test */
	retval = get_ctl(&ctl_rec,BROWSE,0,e_mesg);
	if(retval < 0) return(retval);


	/* Order of operations should not be changed */
	if((retval= MakeStmast(reqi_ptr,e_mesg))<0 )	return(retval);
	if((retval= WriteGlmast(reqh_ptr,reqi_ptr,e_mesg))<0 )	return(retval);
	if((retval= WriteAlloc(reqi_ptr,e_mesg))<0 )	return(retval);
	if((retval= MakeSttran(reqh_ptr,reqi_ptr,e_mesg))<0 )	return(retval);

	st_mast.st_alloc 	= D_Roundoff(st_mast.st_alloc);
	st_mast.st_on_hand 	= D_Roundoff(st_mast.st_on_hand);
	st_mast.st_m_rec 	= D_Roundoff(st_mast.st_m_rec);
	st_mast.st_y_rec 	= D_Roundoff(st_mast.st_y_rec);
	st_mast.st_on_order 	= D_Roundoff(st_mast.st_on_order);
	st_mast.st_paidfor 	= D_Roundoff(st_mast.st_paidfor);
	st_mast.st_value 	= D_Roundoff(st_mast.st_value);
	/*******************************************************/
	/* Don't Round st_rate because it is an average        */
	/* st_mast.st_rate 	= D_Roundoff(st_mast.st_rate); */
	/*******************************************************/
	st_mast.st_m_iss 	= D_Roundoff(st_mast.st_m_iss);
	st_mast.st_y_iss 	= D_Roundoff(st_mast.st_y_iss);
	st_mast.st_m_adj 	= D_Roundoff(st_mast.st_m_adj);
	st_mast.st_y_adj 	= D_Roundoff(st_mast.st_y_adj);
	
	/* Write the stock master record */
	retval = put_stmast( &st_mast, UPDATE, e_mesg );
	if( retval!=NOERROR ){
		roll_back(e_mesg);
		return(retval);
	}
	/*   Perform Rounding function on item 
		defined double		*/
	st_tran.st_amount 	= D_Roundoff(st_tran.st_amount);
	st_tran.st_qty		= D_Roundoff(st_tran.st_qty); 


	for ( ; ; ) {
		st_tmp.st_date = st_tran.st_date ;
		STRCPY( st_tmp.st_type, st_tran.st_type);
		st_tmp.st_seq_no = st_tran.st_seq_no;

		retval = get_sttran( &st_tmp, BROWSE,0, e_mesg );

		if(retval<0) {
			if(retval!=UNDEF) {
				roll_back(e_mesg);
				return(retval);
			}
		}
		else 
			st_tran.st_seq_no++;

		/* Write the stock transaction record */
		retval = put_sttran( &st_tran, ADD, e_mesg );
		if( retval!=NOERROR ){
			if(retval==DUPE) {
				st_tran.st_seq_no++;
				continue;
			}
			roll_back(e_mesg);
			return(retval);
		}
		else break;
	}
	/* Write Gl Trans Here incase issue number changes */ 
	if((retval= WriteTrhdr(reqh_ptr,reqi_ptr,e_mesg))<0 )	return(retval);
	if((retval= WriteTritems(reqh_ptr,reqi_ptr,e_mesg))<0)	return(retval);

	/******************************************************************
	   If different funds are being used then we require two more items
	   for the duetofrom_acct
	******************************************************************/

	if(reqh_ptr->funds != reqh_ptr->stock_fund){
		if((retval= WriteTrhdr2(reqh_ptr,reqi_ptr,e_mesg))<0 )
			return(retval);
		if((retval= WriteTritems2(reqh_ptr,reqi_ptr,e_mesg))<0 )
			return(retval);
	}
	return(NOERROR);
}

MakeSttran(reqh_ptr,reqi_ptr,e_mesg)
Req_hdr	 *reqh_ptr;
Req_item *reqi_ptr;
char	*e_mesg;
{
	Tax_cal	tax_cal;
	int	err;

	st_tmp.st_date = pa_rec.pa_date ;
	STRCPY( st_tmp.st_type, "IS");
	st_tmp.st_seq_no = 32767;

	err = get_n_sttran( &st_tmp, BROWSE,0,BACKWARD,e_mesg );
	if(err==ERROR) {
		roll_back(e_mesg);
		return(err);
	}
	if(st_tmp.st_date != pa_rec.pa_date ||
	   strcmp(st_tmp.st_type,"IS") != 0 ||
	   err == EFL) {
		st_tmp.st_seq_no = 1;
	}
	else  {
		st_tmp.st_seq_no++;
	}

	st_tran.st_seq_no = st_tmp.st_seq_no;
	st_tran.st_date = pa_rec.pa_date;
	st_tran.st_fund = reqi_ptr->fund;
	st_tran.st_fund2 = reqi_ptr->stock_fund;
	strcpy( st_tran.st_type , "IS" );
	strcpy( st_tran.st_code , reqi_ptr->st_code );
	strcpy( st_tran.st_suppl_cd, reqh_ptr->supp_cd );

	st_tran.st_location = reqi_ptr->school;
/* This line posted to the wrong account. Changed to read param curent period */
/**	st_tran.st_period = reqh_ptr->period;  LR**/
	st_tran.st_period = pa_rec.pa_cur_period;

	strcpy( st_tran.st_db_acc, reqi_ptr->acct );

	strcpy( st_tran.st_cr_acc, ctl_rec.inv_acnt);

	st_tran.st_qty = reqi_ptr->orig_qty;
/**	st_tran.st_amount = reqi_ptr->value; **/
	st_tran.st_amount = calc_commit(reqi_ptr->tax1,reqi_ptr->tax2,
				ctl_rec.rebate,reqi_ptr->value,&tax_cal);

	sprintf(st_tran.st_remarks, "REQ# %ld",reqi_ptr->code);

	return(0);
}
MakeStmast(reqi_ptr,e_mesg)
Req_item *reqi_ptr;
char	*e_mesg;
{
	int	option;
	double	qty, amt ;
	int	err;
	Tax_cal	tax_cal;

	/* st_mast.st_fund = reqi_ptr->fund; lr*/
	st_mast.st_fund = reqi_ptr->stock_fund;
	strcpy(st_mast.st_code,reqi_ptr->st_code);
	err = get_stmast(&st_mast,UPDATE,0,e_mesg);
	if(err <0) {
		return(err);
	}
	/* Decrement allocated qty in the stock master */
	st_mast.st_alloc -= reqi_ptr->orig_qty;
	st_mast.st_on_hand -= reqi_ptr->orig_qty;
	st_mast.st_m_iss += reqi_ptr->orig_qty;
	st_mast.st_y_iss += reqi_ptr->orig_qty;
	st_mast.st_value -= calc_commit(reqi_ptr->tax1,reqi_ptr->tax2,
				ctl_rec.rebate,reqi_ptr->value,&tax_cal);

	if( pa_rec.pa_aps==NO ) {
		qty = st_mast.st_on_hand;
		amt = st_mast.st_value ;
	}
	else
		if ( pa_rec.pa_poinv[0] == YES) {
			qty = st_mast.st_on_hand + st_mast.st_paidfor + 
				st_mast.st_po_ordqty;
			amt = st_mast.st_value + st_mast.st_committed ;
		}
		else {
			qty = st_mast.st_on_hand + st_mast.st_paidfor;
			amt = st_mast.st_value ;
		}

	if( qty > DELTA_QTY )
		st_mast.st_rate = amt / qty;

	return(0);
}
WriteAlloc(reqi_ptr,e_mesg)
Req_item *reqi_ptr;
char	*e_mesg;
{
	int	write_mode = UPDATE;
	double	amount, fraction_issued;
	int	retval;

	/* alloc_rec.st_fund = reqi_ptr->fund; lr*/
	alloc_rec.st_fund = reqi_ptr->stock_fund;
	strcpy( alloc_rec.st_code, reqi_ptr->st_code );
	alloc_rec.st_location = reqi_ptr->school;
	strcpy( alloc_rec.st_expacc, reqi_ptr->acct );
	if( (retval = get_alloc( &alloc_rec, UPDATE, 0, e_mesg ))==NOERROR ){
		fraction_issued = reqi_ptr->orig_qty/alloc_rec.st_alloc;
		if(fraction_issued>1.0 )	
				fraction_issued = 1.0;

		alloc_rec.st_issued += reqi_ptr->orig_qty;
		if( alloc_rec.st_issued<DELTA_QTY )
			alloc_rec.st_issued = 0.0;
		alloc_rec.st_alloc -= reqi_ptr->orig_qty;
		if( alloc_rec.st_alloc<DELTA_QTY ){
			alloc_rec.st_alloc = 0.0;
			write_mode = P_DEL;
		}
		/* Value of balance to be changed proportionately */
		amount = fraction_issued * alloc_rec.st_value;
		alloc_rec.st_value -= amount;
		/* Roundoff items that have calculated double values */
		alloc_rec.st_issued 	= D_Roundoff(alloc_rec.st_issued);
		alloc_rec.st_alloc 	= D_Roundoff(alloc_rec.st_alloc);
		alloc_rec.st_value 	= D_Roundoff(alloc_rec.st_value);

		/* Write the allocation record */
		if((retval=put_alloc( &alloc_rec, write_mode, e_mesg ))
				!=NOERROR ){
			roll_back(e_mesg);
			return(retval);
		}
	}
	else if(retval!=UNDEF){
		roll_back(e_mesg);
		return(retval);
	}
		
	return(0);
}

WriteTrhdr(reqh_ptr,reqi_ptr,e_mesg)
Req_hdr	 *reqh_ptr;
Req_item *reqi_ptr;
char	*e_mesg;
{
	Tax_cal	tax_cal;
	long	sysdt ;
	int	retval;
#ifdef ORACLE
	long	sno, get_maxsno();
#endif

	tr_hdr.th_fund = reqi_ptr->fund;
	tr_hdr.th_reccod = 99;
	tr_hdr.th_create[0] = 'G';

#ifndef ORACLE
	tr_hdr.th_seq_no = 32767;
	flg_reset(GLTRHDR);
	retval = get_n_trhdr( &tr_hdr, BROWSE, 0, BACKWARD, e_mesg );
	seq_over( GLTRHDR );
	if( retval==ERROR ){
		roll_back(e_mesg);
		return(-1);
	}
	if( retval==EFL || 
	    tr_hdr.th_fund != reqi_ptr->fund ||	
	    tr_hdr.th_reccod != 99 || tr_hdr.th_create[0] != 'G' ){
		tr_hdr.th_fund = reqi_ptr->fund;
		tr_hdr.th_reccod = 99;
		tr_hdr.th_create[0] = 'G';
		tr_hdr.th_seq_no = 1;
	}
	else
		tr_hdr.th_seq_no++;
#else
	sno = get_maxsno(GLTRHDR,(char *)&tr_hdr,0,-1,e_mesg);
	if(sno < 0) {
		roll_back(e_mesg);
		return(-1);
	}
	tr_hdr.th_seq_no = sno + 1;
#endif
	
	strcpy( tr_hdr.th_userid, User_Id );
	tr_hdr.th_sys_dt = sysdt = get_date() ;
	tr_hdr.th_period = pa_rec.pa_cur_period;
	tr_hdr.th_date = sysdt;
	tr_hdr.th_debits = tr_hdr.th_credits = 
		calc_commit(reqi_ptr->tax1,reqi_ptr->tax2,
			ctl_rec.rebate,reqi_ptr->value,&tax_cal);

	/*  Roundoff double items that have calculated values  */
	tr_hdr.th_debits 	= D_Roundoff(tr_hdr.th_debits);
	tr_hdr.th_credits 	= D_Roundoff(tr_hdr.th_credits);

#ifdef ENGLISH
	strcpy(tr_hdr.th_descr,"Stock Issues");
#else
	strcpy(tr_hdr.th_descr,"Emissions de stocks");
#endif

	sprintf(tr_hdr.th_reference, "%s-%d", "IS", st_tran.st_seq_no) ;
	tr_hdr.th_supp_cd[0] = '\0';
	tr_hdr.th_type[0] = 'I';

	for( ; ; ) {
		tr_tmp.th_fund = tr_hdr.th_fund ;
		tr_tmp.th_reccod = tr_hdr.th_reccod ;
		tr_tmp.th_create[0] = tr_hdr.th_create[0] ;
		tr_tmp.th_seq_no = tr_hdr.th_seq_no ;

		retval = get_trhdr( &tr_tmp, BROWSE,0,e_mesg );
		if(retval<0) {
			if( retval!=UNDEF ) {
				roll_back(e_mesg);
				return(retval);
			}
		}

		else 
			tr_hdr.th_seq_no++;

		retval = put_trhdr( &tr_hdr, ADD, e_mesg );
		if(retval<0) {
			if(retval==DUPE) {
				tr_hdr.th_seq_no++;
				continue;
			}
			roll_back(e_mesg);
			return(retval);
		}
		if(retval==NOERROR) break;
	}
	return(0);
}

WriteTrhdr2(reqh_ptr,reqi_ptr,e_mesg)
Req_hdr	 *reqh_ptr;
Req_item *reqi_ptr;
char	*e_mesg;
{
	Tax_cal	tax_cal;
	long	sysdt ;
	int 	retval;
#ifdef ORACLE
	long	sno, get_maxsno();
#endif
	/* replaced "s_rec.s_fund2" with the stored value in the trhdr file
	   "reqh_ptr->stock_fund" */ 

	tr_hdr.th_fund = reqh_ptr->stock_fund;
	tr_hdr.th_reccod = 99;
	tr_hdr.th_create[0] = 'G';
	
#ifndef ORACLE
	tr_hdr.th_seq_no = 32767;
	retval = get_n_trhdr( &tr_hdr, BROWSE, 0, BACKWARD, e_mesg );
	seq_over( GLTRHDR );
	if( retval==ERROR ){
		DispError();
		roll_back(e_mesg);
		return(-1);
	}
	if( retval==EFL || 
	    tr_hdr.th_fund != reqh_ptr->stock_fund ||
	    tr_hdr.th_reccod != 99 || tr_hdr.th_create[0] != 'G' ){
		tr_hdr.th_fund = reqh_ptr->stock_fund;
		tr_hdr.th_reccod = 99;
		tr_hdr.th_create[0] = 'G';
		tr_hdr.th_seq_no = 1;
	}
	else
		tr_hdr.th_seq_no++;
#else
	sno = get_maxsno(GLTRHDR,(char *)&tr_hdr,0,-1,e_mesg);
	if(sno < 0) {
		DispError();
		roll_back(e_mesg);
		return(-1);
	}
	tr_hdr.th_seq_no = sno + 1;
#endif
	
	STRCPY( tr_hdr.th_userid, User_Id );
	tr_hdr.th_sys_dt = sysdt = get_date() ;
	tr_hdr.th_period = pa_rec.pa_cur_period;
	tr_hdr.th_date = sysdt;
	tr_hdr.th_debits = tr_hdr.th_credits = 
		calc_commit(reqi_ptr->tax1,reqi_ptr->tax2,
			ctl_rec.rebate,reqi_ptr->value,&tax_cal);

	/*  Roundoff double items that have calculated values  */
	tr_hdr.th_debits 	= D_Roundoff(tr_hdr.th_debits);
	tr_hdr.th_credits 	= D_Roundoff(tr_hdr.th_credits);

#ifdef ENGLISH
	strcpy(tr_hdr.th_descr,"Stock Issues");
#else
	strcpy(tr_hdr.th_descr,"Emissions de stocks");
#endif

	sprintf(tr_hdr.th_reference, "%s-%d", "IS", st_tran.st_seq_no) ;
	tr_hdr.th_supp_cd[0] = '\0';
	tr_hdr.th_type[0] = 'I';

	for( ; ; ) {
		tr_tmp.th_fund = tr_hdr.th_fund ;
		tr_tmp.th_reccod = tr_hdr.th_reccod ;
		tr_tmp.th_create[0] = tr_hdr.th_create[0] ;
		tr_tmp.th_seq_no = tr_hdr.th_seq_no ;

		retval = get_trhdr( &tr_tmp, BROWSE,0,e_mesg );
		if(retval<0) {
			if( retval!=UNDEF ) {
				DispError();
				roll_back(e_mesg);
				return(retval);
			}
		}
		else 
			tr_hdr.th_seq_no++;

		retval = put_trhdr( &tr_hdr, ADD, e_mesg );
		if(retval<0) {
			if(retval==DUPE) {
				tr_hdr.th_seq_no++;
				continue;
			}
			DispError();
			roll_back(e_mesg);
			return(retval);
		}
		if(retval==NOERROR) break;
	}

	return(0);
}

WriteTritems(reqh_ptr,reqi_ptr,e_mesg)
Req_hdr *reqh_ptr;
Req_item *reqi_ptr;
char	*e_mesg;
{
	Tax_cal	tax_cal;
	int	err;

	tr_item.ti_fund = tr_hdr.th_fund;
	tr_item.ti_reccod = tr_hdr.th_reccod;
	tr_item.ti_create[0] = 'G';
	tr_item.ti_seq_no = tr_hdr.th_seq_no;
	tr_item.ti_item_no = 1;
	tr_item.ti_sys_dt = tr_hdr.th_sys_dt;
	tr_item.ti_period = tr_hdr.th_period;
	strcpy(tr_item.ti_accno,reqi_ptr->acct);
	tr_item.ti_amount = calc_commit(reqi_ptr->tax1,reqi_ptr->tax2,
				ctl_rec.rebate,reqi_ptr->value,&tax_cal);
	tr_item.ti_status = 0;

	tr_item.ti_section = Dbacc_sect;

	/* Roundoff ti_amount which is double	*/
	tr_item.ti_amount = D_Roundoff(tr_item.ti_amount);

	if( put_tritem(&tr_item, ADD, e_mesg)<0 ){
		roll_back(e_mesg);
		return(-1);
	}

	tr_item.ti_fund = tr_hdr.th_fund;
	tr_item.ti_reccod = tr_hdr.th_reccod;
	tr_item.ti_create[0] = 'G';
	tr_item.ti_seq_no = tr_hdr.th_seq_no;
	tr_item.ti_item_no = 2;
	tr_item.ti_sys_dt = tr_hdr.th_sys_dt;
	tr_item.ti_period = tr_hdr.th_period;

	if(reqh_ptr->funds != reqh_ptr->stock_fund)
		strcpy(tr_item.ti_accno, Crfund_dtf); /* duetofrom acct# for Cr Fund */
	else
		strcpy(tr_item.ti_accno,ctl_rec.inv_acnt);
		
	tr_item.ti_amount = calc_commit(reqi_ptr->tax1,reqi_ptr->tax2,
				ctl_rec.rebate,reqi_ptr->value,&tax_cal) * -1;
	tr_item.ti_status = 0;

	tr_item.ti_section = Cracc_sect;

	/* Roundoff ti_amount which is double	*/
	tr_item.ti_amount = D_Roundoff(tr_item.ti_amount);

	if( put_tritem(&tr_item, ADD, e_mesg)<0 ){
		roll_back(e_mesg);
		return(-1);
	}

	return(NOERROR);
}

/********************************************************************
  If the Cr Fund and Db Fund are not equal then there must be two 
  additional records written for both funds duetofrom_acct for the
  current trheader 
********************************************************************/

WriteTritems2(reqh_ptr,reqi_ptr,e_mesg)
Req_hdr *reqh_ptr;
Req_item *reqi_ptr;
char	*e_mesg;
{
	Tax_cal	tax_cal;

	tr_item.ti_fund = tr_hdr.th_fund;
	tr_item.ti_reccod = tr_hdr.th_reccod;
	tr_item.ti_create[0] = 'G';
	tr_item.ti_seq_no = tr_hdr.th_seq_no;
	tr_item.ti_item_no = 3;
	tr_item.ti_sys_dt = tr_hdr.th_sys_dt;
	tr_item.ti_period = tr_hdr.th_period;
	
	STRCPY(tr_item.ti_accno, Dbfund_dtf); /* duetofrom acct# for Db Fund */

	tr_item.ti_amount = calc_commit(reqi_ptr->tax1,reqi_ptr->tax2,
				ctl_rec.rebate,reqi_ptr->value,&tax_cal);
	tr_item.ti_status = 0;

	tr_item.ti_section = Dbdue_sect;

	/* Roundoff ti_amount which is double	*/
	tr_item.ti_amount = D_Roundoff(tr_item.ti_amount);

	if( put_tritem(&tr_item, ADD, e_mesg)<0 ){
		DispError();
		roll_back(e_mesg);
		return(-1);
	}

	tr_item.ti_item_no = 4;
	STRCPY(tr_item.ti_accno,ctl_rec.inv_acnt/*reqi_ptr->acct*/);
	tr_item.ti_amount = -reqh_ptr->amount;
	
	tr_item.ti_section = Crdue_sect;

	/* Roundoff ti_amount which is double	*/
	tr_item.ti_amount = calc_commit(reqi_ptr->tax1,reqi_ptr->tax2,
				ctl_rec.rebate,reqi_ptr->value,&tax_cal) * -1;

 	tr_item.ti_amount = D_Roundoff(tr_item.ti_amount); 

	if( put_tritem(&tr_item, ADD, e_mesg)<0 ){
		DispError();
		roll_back(e_mesg);
		return(-1);
	}
	return(NOERROR);
}

WriteGlmast(reqh_ptr,reqi_ptr,e_mesg)
Req_hdr  *reqh_ptr;
Req_item *reqi_ptr;
char	*e_mesg;
{
	Tax_cal	tax_cal;
	double	tempamount;
	int	retval, i;

	tempamount = calc_commit(reqi_ptr->tax1,reqi_ptr->tax2,
			ctl_rec.rebate,reqi_ptr->value,&tax_cal);

	gl_rec.funds = reqi_ptr->fund;
	strcpy( gl_rec.accno, reqi_ptr->acct);
	gl_rec.reccod = 99;
	for(i=1;;i++){ 	/* Keep trying to get G/L record L.R. */
		retval = get_gl( &gl_rec, UPDATE, 0, e_mesg );
		if(retval == LOCKED){
			fomce();
			fomen(e_mesg);
			getchar();
			fomce();
			sprintf(e_mesg, "Retry: %d - Press any key",i);
			fomen(e_mesg);
			getchar();
			fomce();
		}
		else
			break;
	}
	if( retval!=NOERROR ){
		roll_back(e_mesg);
		return(retval);
	}

	Dbacc_sect = gl_rec.sect;  /* section of db acct : used in GL Xaction */
	gl_rec.curdb += tempamount;
	gl_rec.ytd += tempamount;
	gl_rec.currel[pa_rec.pa_cur_period-1] += tempamount;
	/*  Round off double items that may have been calculated  */
	gl_rec.comdat	= D_Roundoff(gl_rec.comdat);
	gl_rec.curdb	= D_Roundoff(gl_rec.curdb);
	gl_rec.curcr	= D_Roundoff(gl_rec.curcr);
	gl_rec.ytd	= D_Roundoff(gl_rec.ytd);
	gl_rec.currel[pa_rec.pa_cur_period-1] =
		D_Roundoff(gl_rec.currel[pa_rec.pa_cur_period-1]);

	retval = put_gl( &gl_rec, UPDATE, e_mesg );
	if( retval!=NOERROR ){
		roll_back(e_mesg);
		return(retval);
	}

	gl_rec.funds = reqi_ptr->stock_fund;
	strcpy( gl_rec.accno, ctl_rec.inv_acnt);
	gl_rec.reccod = 99;
	for(i=1;;i++){ 	/* Keep trying to get G/L record L.R. */
		retval = get_gl( &gl_rec, UPDATE, 0, e_mesg );
		if(retval == LOCKED){
			fomce();
			fomen(e_mesg);
			getchar();
			fomce();
			sprintf(e_mesg, "Retry: %d - Press any key",i);
			fomen(e_mesg);
			getchar();
			fomce();
		}
		else
			break;
	}
	if( retval!=NOERROR ){
		roll_back(e_mesg);
		return(retval);
	}
	Cracc_sect = gl_rec.sect;  /* section of cr acct : used in GL Xaction */
	gl_rec.curcr -= tempamount;
	gl_rec.ytd -= tempamount;
	gl_rec.currel[pa_rec.pa_cur_period-1] -= tempamount;
	/*  Round off double items that may have been calculated  */
	gl_rec.comdat	= D_Roundoff(gl_rec.comdat);
	gl_rec.curdb	= D_Roundoff(gl_rec.curdb);
	gl_rec.curcr	= D_Roundoff(gl_rec.curcr);
	gl_rec.ytd	= D_Roundoff(gl_rec.ytd);
	gl_rec.currel[pa_rec.pa_cur_period-1] =
		D_Roundoff(gl_rec.currel[pa_rec.pa_cur_period-1]);

	retval = put_gl( &gl_rec, UPDATE, e_mesg );
	if( retval!=NOERROR ){
		roll_back(e_mesg);
		return(retval);
	}

	if (reqh_ptr->funds == reqh_ptr->stock_fund) return(0);
/* Louis */
	/**************************************************** 
	 if Cr Fund not equal to Db Fund then we must also 
	 perform the following reverse entries for interfunding
	 ****************************************************/

	/* read the ctl record for the duetofrom_acct number for the Db Fund */

	ctl_rec.fund = reqh_ptr->funds; /* SHERRY */
	retval = get_ctl( &ctl_rec, BROWSE, 0, e_mesg );
	if( retval < 0 ){
		DispError();
		return(-1);
	}

	gl_rec.funds = reqh_ptr->funds;
	STRCPY( gl_rec.accno, ctl_rec.duetofrom_acct );
	gl_rec.reccod = 99;
	for(i=1;;i++){ 	/* Keep trying to get G/L record L.R. */
		retval = get_gl( &gl_rec, UPDATE, 0, e_mesg );
		if(retval == LOCKED){
			fomce();
			fomen(e_mesg);
			getchar();
			fomce();
			sprintf(e_mesg, "Retry: %d - Press any key",i);
			fomen(e_mesg);
			getchar();
			fomce();
		}
		else
			break;
	}
	if( retval!=NOERROR ){
		DispError();
		roll_back(e_mesg);
		return(retval);
	}

	/* reverse the transaction for the duetofrom acct for the fund */

	STRCPY(Crfund_dtf, ctl_rec.duetofrom_acct);
	Dbdue_sect = gl_rec.sect;  /* section of db due to from acct : 
						used in GL Xaction */
	gl_rec.curdb -= tempamount;
	gl_rec.ytd -= tempamount;
	gl_rec.currel[pa_rec.pa_cur_period-1] -= tempamount; 
	/*  Round off double items that may have been calculated  */
	gl_rec.comdat	= D_Roundoff(gl_rec.comdat);
	gl_rec.curdb	= D_Roundoff(gl_rec.curdb);
	gl_rec.curcr	= D_Roundoff(gl_rec.curcr);
	gl_rec.ytd	= D_Roundoff(gl_rec.ytd);
	gl_rec.currel[pa_rec.pa_cur_period-1] =
		D_Roundoff(gl_rec.currel[pa_rec.pa_cur_period-1]);

	retval = put_gl( &gl_rec, UPDATE, e_mesg );
	if( retval!=NOERROR ){
		DispError();
		roll_back(e_mesg);
		return(retval);
	}


	/* read the ctl record for the duetofrom_acct number for the Cr Fund */

	ctl_rec.fund = reqh_ptr->stock_fund;
	retval = get_ctl( &ctl_rec, BROWSE, 0, e_mesg );
	if( retval < 0 ){
		DispError();
		return(-1);
	}

	gl_rec.funds = reqh_ptr->stock_fund;
	STRCPY( gl_rec.accno, ctl_rec.duetofrom_acct );
	gl_rec.reccod = 99;
	for(i=1;;i++){ 	/* Keep trying to get G/L record L.R. */
		retval = get_gl( &gl_rec, UPDATE, 0, e_mesg );
		if(retval == LOCKED){
			fomce();
			fomen(e_mesg);
			getchar();
			fomce();
			sprintf(e_mesg, "Retry: %d - Press any key",i);
			fomen(e_mesg);
			getchar();
			fomce();
		}
		else
			break;
	}
	if( retval!=NOERROR ){
		DispError();
		roll_back(e_mesg);
		return(retval);
	}

	/* reverse the transaction for the duetofrom acct for the fund2 */

	STRCPY(Dbfund_dtf, ctl_rec.duetofrom_acct);
	Crdue_sect = gl_rec.sect;  /* section of cr due to from acct : 
						used in GL Xaction */
	gl_rec.curcr += tempamount;
	gl_rec.ytd += tempamount;
	gl_rec.currel[pa_rec.pa_cur_period-1] += tempamount; 
	/*  Round off double items that may have been calculated  */
	gl_rec.comdat	= D_Roundoff(gl_rec.comdat);
	gl_rec.curdb	= D_Roundoff(gl_rec.curdb);
	gl_rec.curcr	= D_Roundoff(gl_rec.curcr);
	gl_rec.ytd	= D_Roundoff(gl_rec.ytd);
	gl_rec.currel[pa_rec.pa_cur_period-1] =
		D_Roundoff(gl_rec.currel[pa_rec.pa_cur_period-1]);

	retval = put_gl( &gl_rec, UPDATE, e_mesg );
	if( retval!=NOERROR ){
		DispError();
		roll_back(e_mesg);
		return(retval);
	}
	return(0);
}
/************************ Debugging Routines ***************************/
print_pos()
{
	long	i;
	Po_item	*ptr;

	fprintf(stderr,"++++++++++   PO INFORMATION   ++++++++++\n\n");
	for (i=0; i<MAX_PO; i++)
	{
		fprintf(stderr,">>>>>>>>>>   %s PO header   <<<<<<<<<<\n",i?"Nonbulk" : "Direct");
		if (po_hdr_empty[i] == PO_HDR_EMPTY)
		{
			fprintf(stderr,"PO header is empty\n");
			continue;
		}

		print_pohdr(&po_hdr[i]);

		fprintf(stderr,"----------   Start of Items   ----------\n");
		for ( ptr=list_get(po_item_list[i],FIRST_NODE); ptr != NOOBJ;
			ptr=list_get(po_item_list[i],NEXT_NODE) )
			print_poitem(ptr);
		fprintf(stderr,"-----------   End of Items   -----------\n");
		fprintf(stderr,"\n\n");
	}
	return(NOERROR);
}

print_pohdr(phdr)
Po_hdr	*phdr;
{
	fprintf(stderr,"KEY: %ld\n\n",phdr->ph_code);

	fprintf(stderr,"\tsupp: %s\tpayee: %s\tamount: %lf\n",
		phdr->ph_supp_cd,phdr->ph_payee,phdr->ph_comm);

	fprintf(stderr,"\tentry_dt: %ld\tdue_dt: %ld\tliq amt: %lf\n",
		phdr->ph_date,phdr->ph_due_date,phdr->ph_lqamt);

	fprintf(stderr,"\tperiod: %d\tfund: %d\tlq_dt: %ld\n",
		phdr->ph_period,phdr->ph_funds,phdr->ph_lqdate);

	fprintf(stderr,"\tshipto: %d\ttype: %s\tbillto: %d\n",
		phdr->ph_shipto,phdr->ph_type,phdr->ph_billto);

	fprintf(stderr,"\tattention: %s\tstatus: %s\n\n",
		phdr->ph_attention,phdr->ph_status);
	
	return(NOERROR);
}
print_poitem(pitm)
Po_item	*pitm;
{
	fprintf(stderr,"\n>>> PO ITEM <<<\n");
	fprintf(stderr,"KEY: %ld-%d\n\n",pitm->pi_code,pitm->pi_item_no);

	fprintf(stderr,"\taccount: %s\tstock: %s\tuom: %s\tcc: %d\n",
		pitm->pi_acct,pitm->pi_st_code,pitm->pi_unit,pitm->pi_school);

	fprintf(stderr,"\tdesc: %s\n",pitm->pi_desc);
	
	fprintf(stderr,"\tTax1 Tax2     QTY    C/U    NET  REQNO\n"); 
	fprintf(stderr,"\t %s   %s   %lf    %lf    %lf      %ld\n\n",
		pitm->pi_tax1,pitm->pi_tax2,pitm->pi_orig_qty,
		pitm->pi_unitprice,pitm->pi_original,pitm->pi_req_no);

	return(NOERROR);
}
