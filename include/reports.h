/*
*	Contains common variables & definitions shared by different report
*	programs.
*/

#include <bfs_defs.h>
#include <bfs_recs.h>
#include <repdef.h>

/* common variables for all reports */

#ifdef	MAIN

/* 80 Hyphens line */
char	hyp_line[] = "--------------------------------------------------------------------------------" ;

char	Date_Mask[] = "____/__/__" ;	/* Date mask */
char	Mask_1[] = "_" ;		/* 1 Char mask */
char	Mask_2[] = "0_" ;		/* 2 Char mask */
char	Mask_3[] = "_0_" ;		/* 3 Char mask */
char	Mask_4[] = "__0_" ;		/* 4 Char mask */
char	Mask_5[] = "___0_" ;		/* 5 Char mask */
char	Mask_6[] = "____0_" ;		/* 6 Char mask */
char	Mask_7[] = "_____0_-" ;		/* 7 Char mask + Minus Sign */
char	Mask_8[] = "______0_-" ;	/* 8 Char mask + Minus Sign */
char	Mask_5_2[]  = "___0_.__-" ;	/* 5.2 Mask + Minus Sign */
char	Mask_7_2[]  = "_____0_.__-" ;	/* 7.2 Mask + Minus Sign */
char	Mask_8_2[]  = "______0_.__-" ;	/* 8.2 Mask + Minus Sign */
char	Amt_Mask[] = "__,___,_0_.__-";	/* Amount mask + Minus Sign, 14 chars*/

char	*mth_tbl[] = {  " ","JAN", "FEB", "MAR", "APR", "MAY", "JUN",
			"JUL", "AUG", "SEP", "OCT", "NOV", "DEC"} ;

#else

extern	char	hyp_line[] ;

extern	char	Date_Mask[]  ;	/* Date mask */
extern	char	Mask_1[] ;	/* 1 Char mask */
extern	char	Mask_2[] ;	/* 2 Char mask */
extern	char	Mask_3[] ;	/* 3 Char mask */
extern	char	Mask_4[] ;	/* 4 Char mask */
extern	char	Mask_5[] ;	/* 5 Char mask */
extern	char	Mask_6[] ;	/* 6 Char mask */
extern	char	Mask_7[] ;	/* 7 Char mask + Minus Sign */
extern	char	Mask_8[] ;	/* 8 Char mask + Minus Sign */
extern	char	Mask_5_2[] ;	/* 5.2 Mask + Minus Sign */
extern	char	Mask_7_2[] ;	/* 7.2 Mask + Minus Sign */
extern	char	Mask_8_2[] ;	/* 8.2 Mask + Minus Sign */
extern 	char	Amt_Mask[] ;	/* Amt mask+Minus Sign,14chars*/

extern	char	*mth_tbl[] ;

#endif

/*--------------------------END OF FILE----------------------------*/

