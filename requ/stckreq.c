/*-----------------------------------------------------------------------
Source Name: stckreq.c
System     : Budgetary Financial system.
Module     : Requisition system.
Created  On: 20th Mar 91.
Created  By: J. Prescott.

MakeFile :	make -f makerlib

DESCRIPTION:
	Routine to calculate the quantity requisitions for a given
	stock item.

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

static	Req_item req_item;	/* Requistion Item Record */

long
stckreq(req_code,stock_code,quantity,e_mesg)

char	*req_code;	/* i: Requisition number requisitioned */
char	*stock_code;	/* i: Stock code qty requisitioned is calculated for. */
double	*quantity;	/* r: The Requisitioned quantity .		*/
char	*e_mesg;	/* r: The error message if an error occurs.	*/

{
	long	retcode;	/* The value returned by get_n_reqitem	*/

	/* Initialize the Requisitioned quantity.	 		*/
	*quantity = 0.0;

	/* Form the search key for the requisition item file.  This	*/
	/* will allow access to all requisition line items that have	*/
	/* the same stock code as passed .			*/
	STRCPY(req_item.st_code,stock_code);
	req_item.code = 0;
	req_item.item_no = 0;

	/* For each requisition line item.				*/
	for (flg_reset(REQITEM) ; (retcode = get_n_reqitem((char*)&req_item,BROWSE,2,FORWARD,e_mesg)) == NOERROR ; )
	{
		/* If the stock code does not match the	desired one, */
		/* then set the retcode to EFL to indicate an exceptable */
		/* return code and leave the loop.	*/
		if (strcmp(req_item.st_code,stock_code))
		{
			retcode = EFL;
			break;
		}

		/* If the status is not Stock then it is not a 	*/
		/* requisitioned quantity so continue to the next record. */
		if (req_item.appstat[0] != STOCK)
		{
			continue;
		}

		/* If the requisition matches the requisition number passed */
		/* in to the function skip this record */
		if (strcmp(req_item.code,req_code)) {
			continue;
		}

		/* If this point is reached then the stock is a requisitioned 
		/* stock item. */
		*quantity += req_item.orig_qty;
		*quantity = D_Roundoff(*quantity);
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
