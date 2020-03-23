/*
*
*	filein.h
*
*/
 
/*---- File type ------*/
#define	ISAM   		1 	/* ISAM file */
#define	SEQ		2	/* Ordinary Binary SEQ file */

/*---- File Access Modes ------*/
#define	IRAND 		1	/* Random read on ISAM File */
#define	IDYN		2	/* Next/Previous read on ISAM File */
#define	SRAND 		3	/* Random read on SEQ File */
#define	SDYN		4	/* Next/Previous read on SEQ File */

#ifdef	O_ISAM
#define	STATUS_LEN	0	/* Del status Byte */
#else
#define	STATUS_LEN	1	/* Del status Byte */
#define	SET_DEL		'1'	/* Record Deleted */
#define	SET_ACTIVE	'0'	/* Record is Active (Not deleted) */
/******
#define	FIXED_RECLEN		* Fixed Length ISAM. Same flag is there
				   in isdef.h. Changes should reflect there
				   also *
*******/
#endif

#define	DATE_TYPE	YYYYMMDD	/* ISAM Date Type */

/*
*	Record Length Macro for SEQ type files
*/

#if	STATUS_LEN == 1
#define	RECLEN(X)	(id_array[X].reclen+1)	/* Including Status byte */
#else
#define	RECLEN(X)	id_array[X].reclen
#endif

/*
*   file descriptor packet ..  
*/

/***
#define	KMAX		100	* size of keysarray *
****/
#define	TMPMAX		50	/* Max keysarray size for temp files */
#define	NAMAX		11	/* Max file name length (without Path).
				   Unix Allows 14 char file names. ISAM appends
				   ".IX" for index files. So we are left with 11
				   chars. By rounding off it comes to 10.  */
 
typedef  struct  id_f { 

	char	id_f_name[NAMAX] ; /* file name under DBH  */ 
	char	fl_user_nm[21];	/* this name is passed to user thru c_mesg */
				/* If you increase it to more than 20 see
				    crt_msg() in recio.c also	 	*/
	char	id_lock[1] ;	/* file lock flag ..set to 1 if locked	*/
	int	id_f_type ;	/* file type (SEQ / ISAM)		*/ 
	int	reclen ,	/* Record Length for the file		*/	
		tot_keys ,	/* number of total keys			*/ 
	/****	keysarray[KMAX], {part, {type,len,pos,order}..}		*/ 
		id_io_mode ,	/* IO mode (RWR/W/R/RWMODE/RDMODE) etc.	*/ 
		id_fd ,		/* fd number of an open file		*/ 
		id_data ,	/* data file fd opened under ISAM	*/ 
		id_start ,	/* -1(RAND),0(DYN Forward),1(DYN Prev)	*/
		id_curkey ,	/* cur key number for DYN access	*/ 
		keys_offset ;	/* Zero relative offset in keysarray	*/
} f_id ;
 
/**-- errlog record definition . Only DBH can use it so here --**/ 
                                                           

typedef struct  {

	char	user[11] ;	/* originator of the request */
	char	prog_nm[11] ;	/* Program being executed */
	char	on_file[11] ;	/* file being operated at error */
	char	routine[10] ;	/* routine where error originated */
	char	request[8] ;	/* read/write/rewite request */ 
	char	err_is[6] ;	/* error code from ISAM io */
	char	err_db[6] ;	/* error code from DBH */
	char	err_no[6] ;	/* error code from system  */ 
	char	key[41] ;	/* key on which error  occured */
}
 err_log;


#ifdef FILEST  
 
f_id	id_array[TOTAL_FILES] ;

int	dberror ;  /* externed in file_fd for DBH errors.. */ 
 
char	User_Id[11] = "\0"; 	/* externed in [PROJ]def.h for Current User.. */
#ifdef	ORACLE
char	UserPasswd[15] = "\0" ;	/* externed in [PROJ]def.h */
#endif
 
/**-- the following external variables are available from ISAm --*/ 
 
extern int      data_fd ;       /* fd of last data file */ 
extern long     last_posn;      /* last write's position */ 

#endif 

#define	UNLOCK	0 
#define	RDLOCK	4
#define	WRLOCK	2

/*---------------------------End Of File--------------------------------*/
