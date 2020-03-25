/*-----------------------------------------------------------------------
Source Name: settax.c
System     : Budgetary Financial system.
Module     : Requisition system.
Created  On: 20th Mar 91.
Created  By: Steven Osborne.

MakeFile :	make -f makerlib

DESCRIPTION:
	Routine to read the taxes from the control file and initialize the
	global variables, Gst_Tax and Pst_Tax.

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

long
settax(e_mesg)

char	*e_mesg;	/* r: The error message.		*/

{
	static	Ctl_rec	ctl_rec;
	long	retcode;	/* Return code	*/

	/* Set up the ctl_rec to get the first control record.	*/
	ctl_rec.fund = 0;
	flg_reset(CONTROL);

	/* Read the control record.	*/
	retcode = get_n_ctl((char*)&ctl_rec,BROWSE,0,FORWARD,e_mesg);
	if (retcode != NOERROR)
	{
		return(retcode);
	}

	/* Set the global variables.	*/
	Gst_Tax = ctl_rec.gst_tax / (double)100.0;
	Pst_Tax = ctl_rec.pst_tax / (double)100.0;

	return(NOERROR);
}
