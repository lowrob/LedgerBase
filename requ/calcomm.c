/*-----------------------------------------------------------------------
Source Name: calcomm.c
System     : Budgetary Financial system.
Module     : Requisition system.
Created  On: 3rd April 91.
Created  By: J. Prescott.

MakeFile :	make -f makerlib

DESCRIPTION:
	this routine calculates the gl commitments for an account 
	taking into account the taxes and the rebate. 
	This differs from Commit_Calculation in that the gst and pst flags 
	are passed in not the actual percentages. 

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

double calc_commit(gst_tax,pst_tax,rebate,net_amt,tax_ret)
char	*gst_tax;
char	*pst_tax;
short	rebate;
double net_amt;
Tax_cal *tax_ret;
{
	char	txt[80];
	double commit_amt;

	calctax(gst_tax,pst_tax,net_amt,tax_ret);
	commit_amt = net_amt + tax_ret->pst_amt + 
			(tax_ret->gst_amt * ((100.0 - ((double)rebate))/100));
	commit_amt = D_Roundoff(commit_amt);
	
	return(commit_amt);
}
