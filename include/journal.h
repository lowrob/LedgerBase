/* 
*	journal.h
*
*	Global structure for in-memory management of committed and 
* 	to_be_rolled back records ..
*/


typedef struct	{

	char	termnm[5] ;	/* Terminal short name/number */
	char	usrnm[11] ;	/* User Id */
	char	program[11] ;	/* Program Name */
	short	c_time ;	/* Commited Time */
	short	rec_count ;	/* No of records logged */
	long	size ;		/* Size after area header */

}	Area_hdr ;

#define	COMMIT_AREA	0  
#define	ROLLBK_AREA	1  

/*-------------------------------End Of File----------------------------*/
