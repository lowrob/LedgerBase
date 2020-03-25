/*-----------------------------------------------------------------------
Source Name: calctax.c
System     : Budgetary Financial system.
Module     : Requisition system.
Created  On: 19th Mar 91.
Created  By: Steven Osborne.

MakeFile :	make -f makerlib

DESCRIPTION:
	Routine to calculate taxes; given the tax exemption flags.

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

long
calctax(gst_flag,pst_flag,amount,taxes)

char	*gst_flag;	/* i: The exemption flag for GST tax.		*/
char	*pst_flag;	/* i: The exemption flag for PST tax.		*/
double	amount;		/* i: The amount to be taxed.			*/
Tax_cal	*taxes;		/* r: The resulting taxes and gross amount.	*/

{
	char	txt[80];

	/* Calculate the amount of GST tax.	*/
	if (*gst_flag == TAXABLE)
	{
		taxes->gst_amt = D_Roundoff(amount * Gst_Tax);
	}
	else
	{
		taxes->gst_amt = 0.0;
	}

	/* Calculate the amount of PST tax.	*/
	if (*pst_flag == TAXABLE)
	{
		taxes->pst_amt = D_Roundoff( (amount+taxes->gst_amt) * Pst_Tax);
	}
	else
	{
		taxes->pst_amt = 0.0;
	}

	/* Calculate the gross amount.	*/
	taxes->gros_amt = D_Roundoff(amount+taxes->gst_amt+taxes->pst_amt);

	return(NOERROR);
}
