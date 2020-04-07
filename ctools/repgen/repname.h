/*
	File name : repname.h
	Written By : V.Subramani
*/



/**********************************************************************/
/* Define all report names for a system	*/
/* Application programmer can define basic structures for each report	*/
/* This supports input from more than one structure for one report	*/
/* Each report will contain the structure names and structure number	*/
/* defined by application programmer					*/
/* To create the file of this structure the output file created by 	*/
/* structure process program is used. File Name : proj.RPNM		*/
/* Formats are available in proj.RPFM file. Max. of 10 formats allowed	*/
/************************************************************************/


#ifdef	MS_DOS
#define	NFM_PATH	"\\ctools\\bin\\"	/* NFM files path */
/* The else has been commented out to avoid th hard codeing of the path
#else
#define	NFM_PATH	"/joe/ctools/bin/"	*/	/* NFM files path */
#endif

#define REPNAME		".RPN"	/* report name extension	*/
#define FRMFILE		".RPF"	/* Format file extention	*/
#define STRFILE		".RPS" 	/* Struc. file extension	*/
#define	FOMFILE		".RFM"	/* Ascii file from format */
#define HEADEXT		".h"	/* Project header file extension */

#define COMP_CODE	"CO"	/* Code entered for Computational field	*/
#define DELETED		'D'	/* Record is deleted	*/
#define NOTDELETED	'N'	/* Record not deleted	*/

/* Define the error codes	*/
#define NOERROR		0
#define ERROR		-1
#define	CREATERR	-10	/* file creation error	*/
#define OPENERR		-20	/* file open error	*/
#define WRITEERR	-30	/* Write error	*/
#define READERR		-40	/* Read error	*/
#define SEEKERR		-50	/* Seek error	*/
#define MEMERR		-90	/* Memory allocation error */
#define INTERR		-100	/* Internal error - consistency	*/


struct 	str_def {
	short	strnum ;	/* structure number */
	char	strname[NAME_LEN] ;	/* structure name */
	} ;

/* for each report defined by DBA one such record is created	*/

struct rp_name {
	char	del_flag ;		/* DELETED / NOTDELETD		*/
	char	rep_name[NAME_LEN] ;
	short	numstruct ;	/* #of structures referred for this report */
	struct 	str_def defstruct[MAXSTRUCT] ;
	short	formrecs ;	/* #of format detailed recs., available	*/
	short	formoff[MAXFORMAT] ; 	/* Format rec# on proj.RPFM file */

} ;

