/*-----------------------------------------------------------------------
Source Name: poconv.c 
System     : Budgetary Financial system.
Module     : Purchase Order
Created  On: 17 Dec. 90  
Created  By: M. Cormier

DESCRIPTION:
	This program converts floating point to double numeric from  the
	references: convap.c (J. Prescott)

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
F.Tao 	       90/12/21	      Change all tax flags to EXCEMPT. 
------------------------------------------------------------------------*/
#define  MAIN
#define  MAINFL		POITEM   	/* main file used */

#include <stdio.h>

main(argc,argv)
int argc;
char *argv[];
{
	double dblvalue;

dblvalue = 12.99;
printf(" \nthe value in ld is %ld", dblvalue);
printf(" \nthe value in d is %d", dblvalue);
printf(" \nthe value in d is %lf", dblvalue);

} /* END OF MAIN */

