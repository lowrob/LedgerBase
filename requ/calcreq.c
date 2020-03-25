/*-----------------------------------------------------------------------
Source Name: calcreq.c
System     : Budgetary Financial system.
Module     : Requisition system.
Created  On: 19th Mar 91.
Created  By: Steven Osborne.

MakeFile :	make -f makerlib

DESCRIPTION:
	Routine to calculate the amount of commitments to a GL account from
	requisitions.  Only accounts with a record code of 99 may be used.
	A record code of 99 is therefore assumed.

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


MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
------------------------------------------------------------------------*/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>
#include <requ.h>

double	D_Roundoff();

static	Req_hdr	req_hdr;	/* Requistion Header Record */
static	Req_item req_item;	/* Requistion Item Record */

long
calcreqcommits(fund,acct_no,amount,e_mesg)

long	fund;		/* i: The fund the account belongs to.		*/
char	*acct_no;	/* i: The account number that commitments are
				 to be calculated for.			*/
double	*amount;	/* r: The commitments for the account.		*/
char	*e_mesg;	/* r: The error message if an error occurs.	*/

{
	long	last_reqno = -1;/* The number of the last		*/
				/* requisition header read in.		*/
	long	retcode;	/* The value returned by get_n_reqitem	*/
	Tax_cal	tax_amount;	/* Amount of taxes on the item line.	*/

	/* Initialize the commited amount.				*/
	*amount = 0.0;

	/* Form the search key for the requisition item file.  This	*/
	/* will allow access to all requisition line items that have	*/
	/* the same fund and account as those passed.			*/
	req_item.fund = fund;
	STRCPY(req_item.acct,acct_no);
	req_item.code = 0;
	req_item.item_no = 0;

	/* For each requisition line item.				*/
	for (flg_reset(REQITEM) ; (retcode = get_n_reqitem((char*)&req_item,BROWSE,1,FORWARD,e_mesg)) == NOERROR ; )
	{
		/* If the fund or account number do not match the	*/
		/* desired one, then set the retcode to EFL to indicate	*/
		/* an exceptable return code and leave the loop.	*/
		if (req_item.fund != fund || strcmp(req_item.acct,acct_no))
		{
			retcode = EFL;
			break;
		}

		/* If the status is Unapproved then it is not a 	*/
		/* commitment so continue to the next record.		*/
		if (req_item.appstat[0] == DISAPPROVED)
		{
			continue;
		}

		/* If the current requisition number is not the same as	*/
		/* the last retrieved requistion header.		*/
		if (req_item.code != last_reqno)
		{
			/* Read the corresponding requistion header for	*/
			/* this line item.				*/
			req_hdr.code = req_item.code;
			retcode = get_reqhdr((char*)&req_hdr,BROWSE,0,e_mesg);
			if (retcode != NOERROR)
			{
				return(retcode);
			}
			last_reqno = req_hdr.code;
		}

		/* If the status is not Approved, then it is not a	*/
		/* commitment so continue to the next record.		*/
		if (req_hdr.status[0] != APPROVED)
		{
			continue;
		}
	
		/* If we get this far, then the requisition is approved	*/
		/* and the line item is approved or taken from stock.	*/
		/* Therefore we add the amount of the line item to the	*/
		/* total amount of commitments for this account.	*/
		calctax(req_item.tax1,req_item.tax2,req_item.value,&tax_amount);
		*amount += tax_amount.gros_amt;
		*amount = D_Roundoff(*amount);
	}

	/* The only valid return code at this point is EFL, anything	*/
	/* else is an error.						*/
	if (retcode != EFL)
	{
		return(retcode);
	}
	else
	{
		return(NOERROR);
	}
}
