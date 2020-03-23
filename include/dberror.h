/*************************************************************************/
/**									**/
/**	User DBH error list						**/
/**									**/
/*************************************************************************/

#define  dbexit(N, CODE)    { dberror = N ; return(CODE); } 

#define	NODBERROR	0	/* No DBH error				*/
#define	FLNOTEXST	101	/* File does not Exist			*/
#define	INVFILENO	102	/* Invalid file number			*/
#define	KEYOPNERR	103	/* KeyDesc.id File Open Error		*/
#define	KEYSIZERR	104	/* KeyDesc.id File Size Error		*/
#define	FLMISSING	105	/* One of the ISAM files is missing	*/
#define	ISMCRTERR	106	/* ISAM file creation error		*/
#define	ISMOPNERR	107	/* ISAM file Open error			*/
#define	NONISMMOD	108	/* Not an ISAM file Mode		*/
#define	ISMCLSERR	109	/* ISAM file Close error		*/
#define	LOCKRELER	110	/* Lock Release Error			*/
#define	SEQCRTERR	111	/* SEQ file creation error		*/
#define	SEQOPNERR	112	/* SEQ file Open error			*/
#define	NONSEQMOD	113	/* Not a sequential file Mode		*/
#define	LSEEKERR	114	/* Record Seek error on SEQ file	*/
#define	SEQREADER	115	/* Sequential file read error		*/
#define	ADDLOCKER	116	/* Locking Error while adding SEQ rec	*/
#define	MALLOCERR	117	/* Memory Allocation Error		*/
#define	TMPKEYERR	118	/* Temporary file's keys exceed limit	*/

/*
*	JOURNAL ERRORS
*/

#define	JROPENERR	201	/* Journal Open Error			*/
#define	JRALLOCER	202	/* Memory Allocation Error		*/
#define	JRLOCKERR	203	/* Journal File Lock Error		*/
#define	JRWRITERR	204	/* Journal File Write Error		*/
#define	JRKTYPERR	205	/* Key Part type error in SrchArea	*/
#define	JRNOTINRL	206	/* Record not read in UPDATE mode 	*/

/*-------------------------End Of File --------------------------------*/
