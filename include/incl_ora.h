/*
*	incl_ora.h
*
*	Oracle call interface definitions
*/

/* #define	DEBUG */

/*
* Cursor Related Data Structure .. The following structure array is indexed by
* file number to point to a linked list containing cursor related info for the
* same file . 
*/

typedef struct csrdef	{

	short	  csrrc;				  /* return code */
	short	  csrft;				/* function type */
	unsigned long  csrrpc;			 /* rows processed count */
	short	  csrpeo;			   /* parse error offset */
	unsigned char  csrfc;				/* function code */
	unsigned char  csrfil;				      /* filler  */
	unsigned short csrarc;			       /* V4 return code */
	unsigned char  csrwrn;				/* warning flags */
	unsigned char  csrflg;				  /* error flags */

	/*	     *** Operating system dependent *** 		 */

	unsigned int   csrcn;				/* cursor number */
	struct {				      /* rowid structure */
	    struct {
		unsigned long	tidtrba;   /* rba of first blockof table */
		unsigned short	tidpid; 	/* partition id of table */
		unsigned char	tidtbl; 	    /* table id of table */
	    }	ridtid;
	    unsigned long   ridbrba;		     /* rba of datablock */
	    unsigned short  ridsqn;   /* sequence number of row in block */
	} csrrid;
	unsigned int   csrose;		      /* os dependent error code */
	unsigned char  csrchk;				   /* check byte */
	unsigned char  crsfill[30];	       /* private, reserved fill */

} curr_struct  ;



typedef struct t_lst {
	short		key_no, 	/* Key Number 0 - Main , n - Alt  */
			dirn, 		/* 0-FORWARD,1-BACKWARD, -1 -RANDOM */
			mode ,		/* 0 - BROWSE, 1- UPDATE */
			op_code ;	/* INSERT, UPDATE_TBL, DELETE, SELECT */
	struct t_lst	*nxt_lst ;	/* Next struct on same file */
	curr_struct	*cursor ;	/* Cursor location */
	char		*stmnt_ptr ;	/* SQL stqtement for debugging */

}	Tbl_struct ;


#define	 LDA		&lda 
#define	HOSTDATAAREA	256	/* Host Data Area size */
#define	RANDOM		-1 	/* to distinguish SEQUENTIAL mode */

curr_struct	lda ;

/*
*	op_codes for SQL operations
*/ 

#define	INSERT		1 		
#define	DELETE		2
#define	SELECT		3
#define	UPDATE_TBL	4
#define	LOCK_TABLE	5
#define	DELETE_ALL	6
#define	CREAT_TBL	7	
#define	CREAT_INDX	8
#define	DROP_TBL	9
#define	MAX_RECNO	10

/*
*	Data Types
*/

#define	ORA_CHAR	1	/* No Null Termination .. length field Must */
#define	ORA_STR 	5	/* Null Terminated String  */
#define	ORA_INT  	3	/* int or short or long based on length field */
#define	ORA_FLOAT	4	/* float or double based on length */

/*
*	Oracle return codes
*/

#define	OREFL		4	/* End of Fetch */
#define	ORDUPERR	-9	/* Duplicate Value in Index */
#define	ORLOCKERR	-54	/* ROW is locked by other user */
#define	ORTBLEXISTS	-955	/* table/view already exists */
#define	ORINDXEXISTS	-1408	/* index already exists with given columns */
#define	ORTBLNOTEXISTS	-942	/* table/view not exists */

/*
*	For SEQ types Oracle DBH adds recno field to file to select the
*	records by record number as in ISAM DBH. The following definition
*	is the field name used in that context. Corresponding variable
*	is declared in recio.c.
*/

#define	SEQREC_FLDNM	"seqrec_no"	/* Field Name */

/*
*	DBH Error Codes Under new recio ..
*/

#define	CUROPNERR	701	/* Cursor Open Error .. See c_mesg */
#define	ORAERROR	702	/* Internal ORACLE Error .. See c_mesg */
#define	BINDVARERR	703	/* Error in binding field list in SQL stmnt */
#define	BINDWHERR	704	/* Error In binding Where CLause */
#define	INVOPCODE	705	/* Op code Not defined or Invalid */
#define	OFFSETERR	706	/* Key part offset is not matching with
				   .def file offset */
#define	INVKEYTYPE	707	/* Improper key type in keysarray */
#define	INVFLDTYPE	708	/* Improper field type in .def file */
#define	DEFFLERROR	709	/* Column Definitions not found in .def file */

/*------------------------------END OF FILE-------------------------*/

