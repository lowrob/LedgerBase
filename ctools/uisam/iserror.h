/*************************************************************************/
/**									**/
/**	User error list							**/
/**									**/
/*************************************************************************/

#define	FLEXSTERR	100	/* file already exists */
#define	INVMODERR	101	/* Mode is other than Read/Write/Read-write */
#define	INVKTYERR	103	/* Invalid part type */
#define	INVKLNERR	104	/* Invalid part length < 0 */
#define	INVKPSERR	105	/* Invalid part position < 0 */
#define	FILCNTERR	106	/* More than 10 ISAM files open */
#define	MEMORYERR	107	/* Memory allocation error */
#define	DFLCRTERR	108	/* Data file creation error */
#define	IFLCRTERR	109	/* Index file creation error */
#define	INVFDERR	110	/* Invalid file descriptor */
#define	FLNOPNERR	111	/* Specified file not open */
#define	IFLCLSERR	112	/* Data file close error */
#define	DFLCLSERR	113	/* Index file close error */
#define	FLACCSERR	114	/* Unable to access file */
#define	INVRLNERR	116	/* Invalid record length */
#define	DFLOPNERR	119	/* Data file open error */
#define	IFLOPNERR	120	/* Index file open error */
#define	NOKEYERR	121	/* Key not defined */
#define	IMPMODERR	122	/* File not opened in proper mode for 
				   specified operation */
#define	INVORDERR	123	/* Invalid part ordering option */
#define	KEYOVFERR	124	/* Too many keys/keyparts specified */
#define	CRPIXFLERR	125	/* Corrupted Index file (root node) */
#define	FLNAMEERR	126	/* file name more than 50 characters */
#define	KEYSIZERR	127	/* Total Key Length exceeds internal limit */
#define	NULRECPTR	128	/* Null Record Pointer Passed to isstart() */


/*************************************************************************/
/**									**/
/**	Internal error list						**/
/**									**/
/*************************************************************************/

#define	INITERR		201	/* File initialisation error */
#define	NODSKERR	202	/* Index file node seek error */
#define	DFLRDERR	203	/* Data file read error */
#define	IFLRDERR	204	/* Index file read error */
#define	DFLWRERR	205	/* Data file write error */
#define	IFLWRERR	206	/* Index file write error */
#define	DFLSKERR	207	/* lseek error in data file */
#define	STKFLERR	208	/* Stack overflow/underflow */
#define	FLOCKERR	209	/* File lock operation error */
#define	INVNODNO	210	/* Invalid index node no. */
