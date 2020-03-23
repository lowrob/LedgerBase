/*
*	orclio1.c
*
*	Oracle call interfaced(ORACLE OCI) DBH IO handler.
*
*
*	NOTES:
*
*	LOCK_TABLE op_code is used to lock and unlock files. lock_file() issues
*	"SELECT fld1, fld2, fld3 .. from table FOR UPDATE fld1, fld2, fld3 ..".
*	ORACLE doesn't have any direct call to unlock the table which is locked.
*	However next COMMIT or ROLLBACK unlocks all locks. In unlock_file() call
*	unlocking before commit is simulated by cancelling the above SQL
*	statement cursor opeartion.
*
*	ISAM DBH is supporting SEQ type files, for which records are selected
*	by their number. There is no SQL statement to select the nth record in
*	the file. In ORACLE DBH extra field SEQREC_FLDNM is added to each SEQ
*	type file. This field is transparent to user programs. Whenever a record
*	is being added to data base, a SQL statement which gets max(SEQRECNO)
*	is executed. The new record is inserted with this value as primary key.
*	Similarly for select and update, the clause "where SEQRECNO == rec_no" 
*	is used.
*
*	To insert the records into ISAM files, for which one of key part is
*	running sno, user programs reads the file in previous direction and
*	adds 1 to the last record sno. For ORACLE version it is an expensive
*	operation. Here new function get_maxsno() is provided. This generates
*	sno for a given key_no & part_no. This uses the following SQL statement.
*	Here where clause is added, only when Key parts are more than one.
*
*		select nvl(max(KPN name),0) from TABLE
*			[ where KP1 = KV1 and ...  KPN-1 = KVN-1 ] ;
*
*	For SEQ type files DELETION of record is not supported, because records
*	are inserted with its number as a physical field. Deletion of record
*	creates hole in record number. Also this feature is not supported in
*	ISAM based DBH.
*
*	UPDATE_TBL and DELETE are always assumed to have where clause with
*	Main key, SELECT may have any key number.
*/

#define  FILEST 1	/* to supress some definitions which are common in
			   isnames.h and USERdef.h */

#include <stdio.h>
#include <bfs_defs.h>
#include <isnames.h>
#include <fld_defs.h>
#include <filein.h>
#include <dberror.h>
#include <incl_ora.h>

#define	ID_SIZE		(sizeof(f_id)*TOTAL_FILES)
#define	PATH_LEN	50

#define	 HST		hst  

/*
*	Globals.. externed in other files like orclio2.c..
*/

int	*keysarray = NULL ;	/* Keys array read from keydesc.id */
int	dbh_init = 0 ;		/* To read in id_array from descriptor file */

Tbl_struct 	*tbl_lst[TOTAL_FILES] ;

/*
*	Declaration of Other SQL required variables 
*/

static	char		hst[HOSTDATAAREA] ;	/* Host Definition */

/*
*	Variable to insert SEQRECNO column for SEQ files and to generate 
*	next sno using MAX(least key part) for ISAM files.
*/

long	Seqrec_no ;

/*
*  pointer to statement string  and record space.. Allocated by init_dbh() .
*/

static	char	*sql_rec = NULL ;

/*
* Since each table has to be referenced w.r.t. its owner, and its creation can
* be in a specific TABLESPACE, an owner name and tablespace are declared here.
* Owner name will be prefixed for each table name in all SQL statements. And
* tablespace is used in create table DDL statement. These variables are 
* initialized using OWNER_PREFIX and TABLESPACE environment variables.
* OWNER_PREFIX+dist_no becomes real owner name.
*/

char	owner_prefix[sizeof(User_Id)+2] = "\0" ;	/* Owner of tables */
char	tablespace[31] = "\0" ;				/* TABLESPACE name */

#ifdef	SECURITY
extern	int	SecurityStatus ;	/* Security ON/OFF */
#endif

Tbl_struct *get_cursor(), *AllocateCursor() ;

#ifdef	DEBUG
FILE	*fopen(), *sqlfp = NULL ;
#endif

/*------------------------------------------------------------------*/
/*
*	This routine retrieves a handle for the given file,op_code, key_no 
*	and mode. If cursor is not yet allocated then calls init_cursor
*	to initialise the cursor ..
*	Maintains linked list of cursors of a file ( table ) .
*/

static	Tbl_struct  *
get_cursor ( file_no, op_code, key_no, mode, dirn, c_mesg)
int	file_no ;	/* Table DBH number */
int	op_code ;	/* 1-INSERT, 2-DELETE, 3-SELECT, 4-UPDATE_TBL etc.. */
int	key_no ;	/* 0-Main key, 1-Alt K1 etc.. */
int	mode ;		/* BROWSE, UPDATE etc.. */
int	dirn ;		/* -1(RANDOM), 0(FORWARD), 1(BACKWARD) and 2(EQUAL) */
char	*c_mesg ;
{
	Tbl_struct	*ptr ;
	int		code ;
	char		o_mesg[133] ;

	dberror = iserror = 0 ;

	if ( dbh_init == 0 )
	    if ( (code = init_dbh()) < 0 ) {
		if(code == ORAERROR) {
			oermsg(lda.csrrc, o_mesg);
#ifdef	DEBUG
			fprintf(sqlfp, "LDA error code: %d  op: %d\n%s\n\n",
				lda.csrrc, lda.csrfc, o_mesg);
#endif
			/* Truncate to max. 80 characters for display through
			   profom screen */
			strncpy( c_mesg, o_mesg, 79 );
			c_mesg[79] = '\0';
		}
		else	/* DBH related Error */
			strcpy(c_mesg, "DBH Initializing ERROR") ;

		log_err(file_no, "init_dbh", "get_cursor", c_mesg);
		return (NULL) ;
	}

	if ( file_no < 0 || file_no >= TOTAL_FILES ) {
		sprintf(c_mesg,"ERROR: Invalid File#: %d", file_no) ;
		log_err(-1, "", "get_cursor", c_mesg);
		dbexit(INVFILENO,NULL)
	}

	if ( tbl_lst[file_no] == NULL ) {
		if((tbl_lst[file_no] = AllocateCursor()) == NULL) goto last ;
		ptr = tbl_lst[file_no] ;
	}
	else {
		for (ptr =  tbl_lst[file_no] ; ; ptr = ptr->nxt_lst) {
			if ( ptr->key_no == key_no && ptr->mode == mode 
			    &&  ptr->op_code == op_code && ptr->dirn == dirn )
				return(ptr);

			if ( ptr->nxt_lst == NULL ) break ;
		}
		if ((ptr->nxt_lst = AllocateCursor()) == NULL) goto last;
		ptr = ptr->nxt_lst ;
	}

	ptr->key_no  = key_no ;
	ptr->dirn    = dirn ;
	ptr->mode    = mode ;
	ptr->op_code = op_code ;
	ptr->nxt_lst = NULL ;
	ptr->stmnt_ptr = NULL ;

	if ( init_cursor ( file_no, ptr, c_mesg ) == ERROR ) return(NULL) ;

	return ( ptr ) ;

last:
	strcpy(c_mesg,"MEMORY ALLOCATION ERROR FOR CURSOR") ;
	log_err(file_no, "malloc", "AllocateCursor", c_mesg);
	return(NULL) ;
}

/*
*	Allocate the memory for new cursor
*/

Tbl_struct *
AllocateCursor()
{
	Tbl_struct	*ptr = NULL ;

	if ( ptr = (Tbl_struct *)malloc(sizeof(Tbl_struct)) )
		if ( ptr->cursor = (curr_struct *)malloc(sizeof(curr_struct)) )
			return(ptr) ;

	/* Error in allocating Memory */

	if(ptr != NULL) free((char*)ptr);
	dbexit(MALLOCERR, NULL)
}	/* AllocateCursor() */

/*
*	This routine opens cursor, forms SQL statement and binds
*	its variable list with actual record's variable's addresses ..
*/

static	int
init_cursor( file_no, ptr, c_mesg )
Tbl_struct	*ptr ;
char		*c_mesg ;
{
	Fld_hdr		hdr ;
	Field		*fld = NULL ;

#ifdef	DEBUG
	fprintf(sqlfp,
	"New Cursor..  File_no: %d  Op_code: %d Key_no: %d Mode: %d Dir: %d\n",
	     file_no, ptr->op_code, ptr->key_no, ptr->mode, ptr->dirn);
#endif

	if (oopen(ptr->cursor, LDA, (char*)0, -1, -1, (char*)0, -1))
		return ( errrpt( file_no, ptr, CUROPNERR, c_mesg ) ) ;

	/* Field definitions are not required for (SEQ file & MAX_RECNO),
	   DELETE_ALL and DROP_TBL SQL statements. MAX_RECNO is constructed
	   with local variable */

	if( !(id_array[file_no].id_f_type == SEQ && ptr->op_code == MAX_RECNO)
		&& !(ptr->op_code == DELETE_ALL || ptr->op_code == DROP_TBL) ){

		/*
		*  Get the record definition from .def file in an array of
		*  fields. Use this list to create the SQL statement for the
		*  required table under specified operation, mode, key number
		*  etc ..
		*/

		if ( GetFields (id_array[file_no].id_f_name, &hdr, &fld, c_mesg)
					!= NOERROR) {
			dberror = DEFFLERROR ;
			log_err(file_no, "GetFields", "init_cursor",
					".def file not available" );
			return(ERROR);
		}
	}
	else
		hdr.no_fields = 0 ;	/* To be safe */

	if ( form_sql (file_no, ptr ,  &hdr, fld ) != NOERROR ) {
		if(fld != NULL) free((char*)fld) ;
		strcpy(c_mesg,"DBH ERROR in SQL statement construction");
		log_err(file_no, "form_sql", "init_cursor", NULL );
		return(ERROR);
	}

#ifdef	DEBUG
	prnt_sql( ptr->stmnt_ptr ) ;
#endif

	/* 
	*  Have the SQL stmnt parsed for once only . and bind select list, where
	*  clause, order by clause etc ..
	*/

	if ( osql3( ptr->cursor, ptr->stmnt_ptr, -1 ) ) {
		if(fld != NULL) free((char*)fld) ;
		return ( errrpt( file_no, ptr, ORAERROR, c_mesg ) ) ;
	}

	if ( bind_vars ( file_no, ptr , &hdr, fld, c_mesg ) == ERROR  || 
		bind_where ( file_no, ptr, &hdr, fld, c_mesg ) == ERROR ) {
		if(fld != NULL) free((char*)fld) ;
		return (ERROR) ;
	}

	if( fld != NULL)
		free ( (char *)fld ) ;

	return ( NOERROR ) ;
}

/*
* 	init_file()
*
*	Will attempt create the table based on the file number.
*	Assumption made is create on existing table does no harm ..
*	Primary Index creation follows the successful table creation.
*
*	Caution : Advice to use only in init_id.c ..
*
*	Limitations : Does Not Check for existance of Table Or Index .. Can
* 		be done by issuing the following comands ._
*
*		Select tname from tab where tname =  "TABLE_NAME" ;
*		Select iname from indexes where iname = "INDEX_NAME " ;
*/

int
init_file( file_no, c_mesg)
int	file_no ;
char	*c_mesg ;
{
	if ( creat_tbl(file_no, c_mesg) ==  ERROR ) return(ERROR) ;

	if(creat_indx(file_no, 0, c_mesg) == ERROR) return(ERROR);

	/*
	*  Since this cursor is not useful anyfurthure... let's get rid of it..
	*/

	close_file ( file_no ) ;

	return( NOERROR ) ;
}

/*
*	Writes Oracle errors to "errlog" file
*/

static	int 
errrpt( file_no, handle, dbherr, c_mesg)
int		file_no;
Tbl_struct	*handle ;
int		dbherr;
char		*c_mesg ;
{
	char	o_mesg[133];	/* To collect oracle error messages */

#ifdef DEBUG
	fprintf(sqlfp, "ORACLE error code: %d op is: %d\n",
		(int)handle->cursor->csrrc, (int)handle->cursor->csrfc);
	fprintf(sqlfp, "Statement: %s\n\n", handle->stmnt_ptr  ) ;
#endif

	oermsg( handle->cursor->csrrc, o_mesg );

	/* Truncate to max. 80 characters for display through profom screen */
	if( dbh_init==1 )
		sprintf( c_mesg, "%-20.20s ", id_array[file_no].fl_user_nm );
	else
		c_mesg[0] = '\0';
	strncat( c_mesg, o_mesg, 79-strlen(c_mesg) );
	c_mesg[79] = '\0';

#ifdef DEBUG
	fprintf(sqlfp, "%s\n", o_mesg) ;
#endif
	iserror = handle->cursor->csrarc ;	/* V4 error number */
	dberror = dbherr ;

	sprintf(o_mesg, "%d", handle->op_code) ;
	log_err(file_no, o_mesg, "Op Code:", "ORACLE ERROR") ;

	return (ERROR) ;
}
/*
*	Routine to setup DBH. Allocates necessary initial memory for record
*	and statement, reads in keydescriptor file and initialises pointer
*	to cursor descriptors to NULL ..
*/

init_dbh()
{
	char	key_file[50] ;
	int	i, j, no_keys, fd ;
	char	uid[30] ;
	char	tnum[5];
#ifdef SECURITY
	int	security;
#endif

	if ( dbh_init ) return(0) ;	/* Should Not Initialise Twice */

#ifdef	DEBUG
	strcpy(key_file, WORK_DIR) ;
	strcat(key_file, "/");
	strcat(key_file, PROG_NAME) ;
	strcat(key_file, ".");
	get_tnum(tnum);
	strcat(key_file,tnum);
	sqlfp = fopen(key_file, "w" ) ;
	if(sqlfp == NULL) sqlfp = stderr ;
#endif

	strcpy( uid, User_Id) ;
	strcat( uid, "/");
	strcat( uid, UserPasswd ) ;

	if ( orlon(LDA, HST, uid, -1, (char *)0, -1, -1))
		dbexit(ORAERROR, ERROR)

	ocof ( LDA ) ;			/* Disable Autocommit */

	for ( i = 0 ; i < TOTAL_FILES; i++)
		tbl_lst[i] = NULL ;

	/* Initialise id_array and keysarray by reading in the keydesc.id file
	    as exists in current recio ..*/

	DT_TYPE = DATE_TYPE ;	/* ISAM Date type. Defined in XXXX_def.h */

	strcpy(key_file, DATA_PATH) ;
	strcat(key_file, KEY_DESC) ;
	/***
	form_f_name(KEY_DESC, key_file);
	****/

	if ( (fd=open(key_file, RDMODE)) < 0 )
		dbexit(KEYOPNERR, ERROR)

#ifdef SECURITY
	/* Read Security On/Off flag */
	if(read(fd, (char*)&security, sizeof(int)) < sizeof(int)) {
		close(fd) ;
		dbexit(KEYSIZERR,ERROR)
	}
#endif

	/* Read keys information */
	if ( read(fd, (char *)id_array, ID_SIZE) < ID_SIZE ) {
		close(fd) ;
		dbexit(KEYSIZERR,ERROR)
	}

	read(fd, (char *)&no_keys , sizeof(int)) ;

	/* Allocate the size for no of parts & 1 more to store size in the
	   beginning of array */
	if ( (keysarray = (int *)malloc( (no_keys+1) * sizeof(int) )) == NULL )
		dbexit(MALLOCERR,ERROR)

		    keysarray[0] = no_keys ;

	if ( read(fd, (char *)(keysarray+1) , sizeof(int) * no_keys)
					< sizeof(int) * no_keys){
		close(fd) ;
		dbexit(KEYSIZERR,ERROR)
	}
	close(fd) ;

	/* Allocate memory for data record area. Find out Max Reclen and
	   allocate memory */
	j = 0 ;
	for(i = 0 ; i < TOTAL_FILES ; i++)
		if(j < id_array[i].reclen) j = id_array[i].reclen ;
	if ( (sql_rec = malloc((unsigned) j)) == NULL ) dbexit(MALLOCERR, ERROR)

	dbh_init = 1 ;

#ifdef SECURITY
	/* Security should be turned on/off after dbh_init is set to 1.
	   Otherwise it might go in recursion */
	if( security==1 )
		SecurityStatus = 1;	/* Turn ON */
	else
		SecurityStatus = 0;	/* Turn OFF */
#endif

	return(0) ;
}

/*
* Bind the Substitution variables in the Statements  to the actual memory
* location in the record ..  
* Where clause is not Bound here ..
*/

int
bind_vars ( file_no, ptr, hdr, fld, c_mesg )
int		file_no ;
Tbl_struct	*ptr ;
Fld_hdr		*hdr ;
Field		*fld ;
char		*c_mesg;
{
	char	fld_name[31] ;
	int	i, fld_type, code, fld_len, SeqFld ;
	char	*fld_address ;

	/* binding of variables required only for following op_codes */
	if ( !(ptr->op_code == SELECT ||
	    ptr->op_code == UPDATE_TBL || 
	    ptr->op_code == INSERT ||
	    ptr->op_code == MAX_RECNO)
	) return(0) ;

	/* For MAX_RECNO op_code, bind the local variable to get MAX value */
	if(ptr->op_code == MAX_RECNO) {
		code = odefin (ptr->cursor, 1, (char*)&Seqrec_no, sizeof(long),
			ORA_INT, -1, (short*)0, (char*)0, -1, -1,
			(short*)0, (short *)0);

		if ( code )
			return( errrpt( file_no, ptr, BINDVARERR, c_mesg ) );

		return(0) ;
	}

	SeqFld = 0 ;

	/* For SEQ type files for "insert" or "select" SQL statement bind the
	   local Seqrec_no variable */
	if(id_array[file_no].id_f_type == SEQ) {
	    if(ptr->op_code == INSERT) {
		strcpy( fld_name, ":") ;
		strcat( fld_name, SEQREC_FLDNM ) ;
		code = obndrv (ptr->cursor, fld_name, -1, (char*)&Seqrec_no,
		    sizeof(long), ORA_INT, -1, (short*)0, (char*)0, -1, -1) ;

		if ( code )
			return( errrpt( file_no, ptr, BINDVARERR, c_mesg ) );
	    }
	    else if(ptr->op_code == SELECT) {
		code = odefin (ptr->cursor, 1, (char*)&Seqrec_no, sizeof(long),
			ORA_INT, -1, (short*)0, (char*)0, -1, -1,
			(short*)0, (short *)0);

		if ( code )
			return( errrpt( file_no, ptr, BINDVARERR, c_mesg ) );

		SeqFld = 1 ;	/* extra host variable */
	    }
	}

	for ( i = 0 ; i < hdr->no_fields; i++) {
		if ( ptr->op_code == INSERT || ptr->op_code == UPDATE_TBL ) {
			strcpy( fld_name, ":") ;
			strcat( fld_name, (fld+i)->name ) ;
		}
		fld_address =  sql_rec  + (fld+i)->offset ;

		switch ( (fld+i)->type ) {
		case T_CHAR	: 
			fld_len = (fld+i)->len ;
			/** fld_type = ORA_CHAR ; **/
			if(fld_len == 1)
				fld_type = ORA_CHAR ;
			else
				fld_type = ORA_STR ;
			break ;
		case T_SHORT	: 
			fld_len = sizeof(short) ;
			fld_type = ORA_INT ;
			break ;
		case T_LONG	: 
			fld_len = sizeof(long) ;
			fld_type = ORA_INT ;
			break ;
		case T_FLOAT	: 
			fld_len = sizeof(float) ;
			fld_type = ORA_FLOAT ;
			break ;
		case T_DOUBLE	: 
			fld_len = sizeof(double) ;
			fld_type = ORA_FLOAT ;
			break ;
		default:
			strcpy(c_mesg,"DBH ERROR..  Variables binding ERROR");
			log_err(file_no, NULL, "bind_vars", c_mesg );
			dbexit( INVFLDTYPE, ERROR )
		}

		if ( ptr->op_code == INSERT || ptr->op_code == UPDATE_TBL )
		    code = obndrv (ptr->cursor, fld_name, -1, fld_address,
			fld_len, fld_type, -1, (short*)0, (char*)0, -1, -1) ;
		else if (ptr->op_code == SELECT)
		    code = odefin (ptr->cursor, SeqFld+i+1, fld_address,
			fld_len, fld_type, -1, (short*)0, (char*)0, -1, -1,
			(short*)0, (short *)0);

		if ( code )
			return( errrpt( file_no, ptr, BINDVARERR, c_mesg ) );
	}	/* for loop */
	return (0) ;
}	/* bind_vars() */

/*
* Bind the substitution variables in the Where clause of SQL stmnt ..
* Substitution variables are numbered as :1, :2, :3 etc . keysarray is
* used to locate the address of the variable in the record for given key no .
*
* 	Note: For Sequential files the where clause not constructed with local
*		varibale Seqrec_no.
*/

int
bind_where (file_no,  ptr, hdr, fld, c_mesg )
int		file_no ;
Tbl_struct	*ptr ;
Fld_hdr		*hdr ;
Field		*fld ;
char		*c_mesg;
{
	int	code, ofset, parts, i, j, k ;
	int	fld_type, fld_len ;
	char	*fld_offset ;

	/* binding of variables for where clause is required only for following
	   op_codes */
	if ( !(ptr->op_code == SELECT || ptr->op_code == UPDATE_TBL ||
	    ptr->op_code == DELETE ||
	    (id_array[file_no].id_f_type == ISAM && ptr->op_code == MAX_RECNO)))
		return(0) ;

	if( id_array[file_no].id_f_type == SEQ ) {
		/* bind the where Seqrec_no == rec_no */
		code = obndrn( ptr->cursor, 1, (char*)&Seqrec_no, sizeof(long),
			ORA_INT, -1, (char *)0, -1,-1 ) ;

		if ( code )
			return( errrpt( file_no, ptr, BINDWHERR, c_mesg ) );
		return(0) ;
	}

	ofset = id_array[file_no].keys_offset ;

	for ( i = 0 ; i < (int)ptr->key_no ; i++ )
		ofset += keysarray[ofset] * 4 + 1 ;

	parts = keysarray[ofset] ;
	ofset++ ;

	if(ptr->op_code == MAX_RECNO) {
		/* bind only n-1 variables */
		if(ptr->mode >= 0)	/* Part number */
			parts = ptr->mode ;
		else
			parts = parts - 1 ;
	}

	for ( j = 0 ; j < parts ; j++, ofset += 4 ) {

		switch ( keysarray[ofset] ) {
		case	SHORT : 
			fld_type = ORA_INT ;
			fld_len  = sizeof(short) ;
			break ;
		case	LONG : 
		case	DATE :
			fld_type = ORA_INT ;
			fld_len  = sizeof(long) ;
			break ;
		case	FLOAT : 
			fld_type = ORA_FLOAT ;
			fld_len  = sizeof(float) ;
			break ;
		case	DOUBLE : 
			fld_type = ORA_FLOAT ;
			fld_len  = sizeof(double) ;
			break ;
		case	CHAR : 
			/** fld_type = ORA_STR ; ***/
			/*** fld_len  = keysarray[ofset+1] ; **/
			/*
			* Length in keysarray in generally excluding null, so
			* that storage can be one less in ISAM. Here it should
			* be including null. So rather than depending on
			* keysarray length take the length from .def file.
			*/
			k = keysarray[ofset+2] ;	/* Position/Offset */
			for ( i = 0 ; i < hdr->no_fields ; i++)
				if ( (fld+i)->offset == k) break ;
			if(i == hdr->no_fields)		/* Shouldn't happen */
				fld_len  = keysarray[ofset+1] ;
			else {
				fld_len = (fld+i)->len ;
				keysarray[ofset+1] = fld_len ;
					/* to use in other functions,
					   eg: CheckForNull() */
			}

			if(fld_len == 1)
				fld_type = ORA_CHAR ;
			else
				fld_type = ORA_STR ;
			break ;
		default:
			strcpy(c_mesg,"DBH ERROR..  Key Parts binding ERROR");
			log_err(file_no, NULL, "bind_where", c_mesg+13 );
			dbexit( INVKEYTYPE, ERROR )
		}

		fld_offset = sql_rec + keysarray[ofset+2] ;

		code = obndrn(  ptr->cursor , j+1, fld_offset ,
		    fld_len, fld_type, -1, (char *)0, -1,-1 ) ;

		if ( code )
			return( errrpt( file_no, ptr, BINDWHERR, c_mesg ) );
	}
	return (0) ;
}	/* bind_where() */

/*
*	For CHAR type keys parts, having '\0' in first character is NULL value
*	for ORACLE. In where clause COLUMNx = NULL or > NULL or < NULL are
*	always false. Only IS NULL or IS NOT NULL are valid syntax and
*	semantics. Here WHERE clause uses comparison operators. This is invalid
*	w.r.t '\0' in first character of CHAR type parts. So, always insert CHAR*	type key parts with atleast one blank when it is '\0'. Similarly WHERE
*	cluase host language variables should contain blank when it is '\0'.
* 
*	This function puts blank in first character when it is '\0'.
*/

static
CheckForNull (file_no, key_no)
int	file_no ;
int	key_no ;
{
	int	ofset, parts, i, j ;

	/* SEQ type files "where clause" is always on SEQREC_NO, which
	   is numeric. */
	if ( id_array[file_no].id_f_type == SEQ ) return(0) ;

	ofset = id_array[file_no].keys_offset ;

	for ( i = 0 ; i < key_no ; i++ )
		ofset += keysarray[ofset] * 4 + 1 ;

	parts = keysarray[ofset] ;
	ofset++ ;

	for ( j = 0 ; j < parts ; j++, ofset += 4 )
		if( keysarray[ofset] == CHAR &&
				*(sql_rec + keysarray[ofset+2]) == '\0') {
			if(keysarray[ofset+1] > 1)	/* ORA_STR type */
				strcpy(sql_rec+keysarray[ofset+2], " ") ; /* SBO: This is a dangerous string copy.	*/
			else
				*(sql_rec + keysarray[ofset+2]) = ' ' ;
			
		}

	return (0) ;
}	/* CheckForNull() */

/*-------------------------------------------------------------------------*/
/*
*	User Calls
*/


/*
*	get_isrec()
*
*	Function to get a record for a given key, from an ISAM file.
*/

int
get_isrec ( record, file_no, key_no, mode, c_mesg)
char	*record ;	/* rec to be gotten in this area */
int	file_no ;	/* DBH file no .. */
int	key_no ;	/* main key = 0 , alt_key >= 1 or rec_no */
int	mode ;		/* mode for locking the record if required */
char	*c_mesg;	/* return message in case of error */
{
	int		code ;
	Tbl_struct	*handle ;

#ifdef SECURITY
	if( SecurityStatus != 0 ){
		/* Security Check */
		code = CheckAccess( file_no, BROWSE, c_mesg );
		if( code<0 )
			return( code );
	}
#endif
	if ( ( handle = get_cursor ( file_no, SELECT, key_no, mode, RANDOM,
				c_mesg ) ) == NULL) return(ERROR) ;

	scpy( sql_rec, record, id_array[file_no].reclen ) ;

	CheckForNull(file_no, key_no) ;	/* Replace '\0' with ' ' */

	if ( (code = oexec ( handle->cursor )) ) {
		if(code == ORLOCKERR) {
			crt_msg( file_no, key_no, LOCKED, c_mesg, record );
			dbexit( NODBERROR, LOCKED )
		}
		return ( errrpt( file_no, handle , ORAERROR, c_mesg ) );
	}

	if ( ofetch ( handle->cursor ) ) {
		crt_msg( file_no, key_no, UNDEF, c_mesg, record );
		dbexit( NODBERROR, UNDEF )
	}

	/*
	* As we need only one record in RANDOM mode , 
	* Signal end of fetch to release Unwanted resources 
	*/

	ocan ( handle->cursor ) ;

	scpy( record, sql_rec, id_array[file_no].reclen ) ;

	seq_over(file_no) ;	/* id_array[file_no].id_start = RANDOM ; */
	return( NOERROR ) ;
}


/*
*	get_next()
*
*	Routine to get next/previous record from TABLE w.r.t to current record.
*	For ORACLE DBH new direction (EQUAL) is introduced, to get the records
*	which are matching to first n-1 key parts and starting from nth key part
*	value. 
*
*	CAUTION : issue start_file before get_next() sequence .. Avoid 
*		flg_reset() call as it ignores all cursors ..  
*/

int
get_next( record,  file_no, key_no, direction, mode, c_mesg)
char	*record ;
int	file_no ;
int	key_no ;
int	direction ;		/* FORWARD, BACKWARD and EQUAL ONLY .. */
int	mode ;
char	*c_mesg ;
{
	int	code ;
	Tbl_struct	*handle ;

#ifdef SECURITY
	if( SecurityStatus != 0 ){
		/* Security Check */
		code = CheckAccess( file_no, BROWSE, c_mesg );
		if( code<0 )
			return( code );
	}
#endif
	if ( id_array[file_no].id_start != direction ){

	    /* Cancel all the cursors if reading in a different direction */
	    if ( id_array[file_no].id_start != RANDOM ) seq_over(file_no);

	    if( start_file(record, file_no, key_no, direction, mode, c_mesg) ==
				ERROR) return(ERROR) ;
	}

	if ( ( handle = get_cursor ( file_no, SELECT , key_no, mode, direction,
			c_mesg)) == NULL) return(ERROR) ;

	code = ofetch ( handle->cursor ) ;

	if ( code == OREFL ) {
		seq_over(file_no) ;  /* id_array[file_no].id_start = RANDOM ; */
		crt_msg( file_no, key_no, EFL, c_mesg, record );
		dbexit( NODBERROR, EFL )
	}
	else if(code)
		return( errrpt( file_no, handle, ORAERROR, c_mesg ) );

	scpy( record, sql_rec, id_array[file_no].reclen ) ;

	return(NOERROR) ;
}

int
start_file(start_rec, file_no, key_no, direction, mode, c_mesg)
int	file_no, 
	key_no, 
	mode ,
	direction ;
char	*start_rec,	
	*c_mesg  ;
{
	Tbl_struct	*handle ;

	if ( ( handle = get_cursor ( file_no, SELECT , key_no, mode, direction,
	    c_mesg)) == NULL) return(ERROR) ;

	scpy( sql_rec, start_rec , id_array[file_no].reclen ) ;

	CheckForNull(file_no, key_no) ;	/* Replace '\0' with ' ' */

	if ( oexec ( handle->cursor ) )
		return ( errrpt( file_no, handle, ORAERROR, c_mesg ) );

	id_array[file_no].id_start = direction ;
	return ( NOERROR ) ;
}


/*
*	flg_start()
*
*	Routine to test sequentialness of an DBH file. Returns current setting
*	of flag, i.e, -1 (in RAND), 0(in DYN forward), 1(in DYN backward) and
*	new direction 2 (EQUAL)
*/

int
flg_start ( file_no )
int	file_no ;
{
	if ( dbh_init == 0 ) return(-1) ;
	return( id_array[file_no].id_start );
}

/*
*	flg_reset() : Sets the id_start flag to RANDOM so that next get_next()
*		call will be forced to issue start_file ..  
*	Caution :  Supported for sake of compatibility with ISAM DBH .. Advice
*		 start_file instead of this ..
*/

int
flg_reset( file_no )
int	file_no ;
{
	if ( dbh_init == 0 ) return(0) ;
	seq_over(file_no) ;	/* To be safe */
	return(0) ;
}

/*
*  Seq_over : Terminates all sequential reads on the file ..Pl. note that
*  ALL cursors which are in Seq. mode for the file are Cancelled ..  
*/

int
seq_over ( file_no )
int	file_no ;
{
	Tbl_struct	*ptr ;

	if ( dbh_init == 0 ) return ( 0 ) ;

	for (ptr =  tbl_lst[file_no] ; ptr != NULL  ; ptr = ptr->nxt_lst){
		if ( ptr->dirn != RANDOM )
			ocan (ptr->cursor); /* Signal the end of the fetch */
	}

	id_array[file_no].id_start = RANDOM ;

	return ( 0 ) ;
}

/*
*	put_isrec()
*
*	Function to Insert/Update/Delete A record of a table  
*/

int
put_isrec ( record, mode, file_no, c_mesg)
char	*record ;	/* rec to be affected to the file */
int	file_no ;	/* DBH file no .. */
int	mode ;		/* mode for locking the record if required */
char	*c_mesg;	/* return message in case of error */
{
	int		code, op_code, i ;
	Tbl_struct	*handle ;

#ifdef SECURITY
	if( SecurityStatus != 0 ){
		/* Security Check */
		code = CheckAccess( file_no, mode, c_mesg );
		if( code<0 )
			return( code );
	}
#endif

	switch ( mode ) {
	case	ADD : 
		op_code = INSERT ;
		break ;
	case	UPDATE : 
		op_code = UPDATE_TBL ;
		break ;
	case	P_DEL : 
		op_code = DELETE ;
		break ;
	default	:
		form_key(record, file_no, 0, c_mesg) ;
		log_err(file_no, "InvMode", "put_isrec", c_mesg) ;
		crt_msg( file_no, 0, ERROR, c_mesg, record );
		return(ERROR) ;
	}

	if ( ( handle = get_cursor ( file_no, op_code , 0, 0, RANDOM,
			c_mesg) ) == NULL) return(ERROR) ;

	scpy( sql_rec, record, id_array[file_no].reclen ) ;

	/* Replace all null key parts with blank in all keys */
	for(i = 0 ; i < id_array[file_no].tot_keys ; i++)
		CheckForNull(file_no, i) ;	/* Replace '\0' with ' ' */

	code = oexec ( handle->cursor ) ;

	if ( mode == ADD && code == ORDUPERR  ){
		form_key(record, file_no, 0, c_mesg) ;
		log_err(file_no, "DUPE", "put_isrec", c_mesg) ;
		crt_msg( file_no, 0, DUPE, c_mesg, record );
		return( DUPE ) ;
	}

	if ( code )
		return ( errrpt( file_no, handle, ORAERROR, c_mesg ) );

	seq_over(file_no) ;	/* id_array[file_no].id_start = RANDOM ; */

	return ( NOERROR ) ;
}


/*
*	get_rec()
*
*	Routine to get a record for a Record#, from a SEQ file.
*
*	Function: Selects the where SEQREC_NO == rec_no into the 'record'.
*/

int
get_rec ( record, file_no, rec_no, mode, c_mesg)
char	*record ;	/* rec to be gotten in this area */
int	file_no ;	/* DBH file no .. */
int	rec_no ;	/* Record# */
int	mode ;		/* mode for locking the record if required */
char	*c_mesg;	/* return message in case of error */
{
	int	code ;

	if(rec_no <= 0) return(UNDEF) ;

	Seqrec_no = rec_no ;		/* Initialize where clause variable */

	code = get_isrec( record, file_no, 0, mode, c_mesg) ;
	if(code < 0) return(code) ;

	return((int)Seqrec_no) ;
}

/*
*	get_seqrec()
*
*	Routine to get a next/previous record from a SEQ file.
*
*/

int
get_seqrec ( record, file_no, rec_no, direction, mode, c_mesg)
char	*record ;	/* rec to be gotten in this area */
int	file_no ;	/* DBH file no .. */
int	rec_no ;	/* Record# */
int	direction ;	/* 0 - Forward, 1 - Backward */
int	mode ;		/* mode for locking the record if required */
char	*c_mesg;	/* return message in case of error */
{
	int	code ;

	Seqrec_no = rec_no ;		/* Initialize where clause variable */

	code = get_next( record, file_no, 0, direction, mode, c_mesg) ;
	if(code < 0) return(code) ;

	return((int)Seqrec_no) ;
}

/*
*	put_rec()
*
*	routine to write the record on to SEQ file.
*/

int
put_rec ( record, mode, file_no, rec_no, c_mesg)
char	*record ;
int	mode ;		/* ADD or UPDATE.. */
int	file_no ;	/* index in to id array .. */
int	rec_no ;
char	*c_mesg ;
{
	int		code, op_code ;
	Tbl_struct	*handle ;

#ifdef SECURITY
	if( SecurityStatus != 0 ){
		/* Security Check */
		code = CheckAccess( file_no, mode, c_mesg );
		if( code<0 )
			return( code );
	}
#endif
	switch ( mode ) {
	case	ADD :
		op_code = INSERT ;
		break ;
	case	UPDATE :
		op_code = UPDATE_TBL ;
		break ;
	default	:
		form_key(record, file_no, rec_no, c_mesg) ;
		log_err(file_no, "InvMode", "put_rec", c_mesg) ;
		crt_msg( file_no, 0, ERROR, c_mesg, record );
		return(ERROR) ;
	}

	if(mode == ADD) {
		/* First get the cursor which has the select statement to get
		   maximum recno+1 */

		if ( (handle = get_cursor (file_no, MAX_RECNO, 0, mode,
			RANDOM, c_mesg)) == NULL) return(ERROR) ;

		/* execute it and get the next record number */

		if( oexec (handle->cursor) || ofetch(handle->cursor) )
			return ( errrpt(file_no, handle, ORAERROR, c_mesg) ) ;

		Seqrec_no++ ;
	}
	else
		Seqrec_no = rec_no ;

	scpy( sql_rec, record, id_array[file_no].reclen ) ;

	if ( (handle = get_cursor (file_no, op_code, 0, mode, RANDOM,
			c_mesg)) == NULL) return(ERROR) ;

	if( mode == ADD ) {
		while( (code = oexec (handle->cursor)) == ORDUPERR)
			Seqrec_no++ ;		/* Try to add as next rec */
	}
	else
		code = oexec (handle->cursor) ;

	if( code )
		return ( errrpt(file_no, handle, ORAERROR, c_mesg) ) ;

	seq_over(file_no) ;	/* id_array[file_no].id_start = RANDOM ; */

	return((int)Seqrec_no);
}

/*
* lock_file : locks the entire file .. 
*/

int
lock_file ( file_no, c_mesg )
int	file_no ;
char	*c_mesg ;
{
	/* Create a cursor with No 'Where' clause but only Update clause .. */

	Tbl_struct	*handle ;

	if ( ( handle = get_cursor ( file_no, LOCK_TABLE , 0, UPDATE , RANDOM,
		c_mesg) ) == NULL) return(ERROR) ;

	if ( oexec ( handle->cursor ) )
		return ( errrpt( file_no, handle, ORAERROR, c_mesg ) );

	id_array[file_no].id_lock[0]  = '1' ;
	return(0) ;
}


/*
*	Unlock_file : Unlocks the entire file .. 
*/

int
unlock_file ( file_no, c_mesg )
int	file_no ;
char	*c_mesg ;
{
	Tbl_struct	*handle ;

	/* Get the cursor which is used for locking file in lock_file() and
	   cancel the same */

	if ( ( handle = get_cursor ( file_no, LOCK_TABLE, 0, UPDATE , RANDOM,
	    c_mesg) ) == NULL) return(ERROR) ;

	if( id_array[file_no].id_lock[0] == '0') return(0) ;

	if ( ocan ( handle->cursor ) )
		return ( errrpt( file_no, handle, ORAERROR, c_mesg ) );

	id_array[file_no].id_lock[0] = '0' ;

	return(0) ;
}

int
creat_tbl(file_no, c_mesg)
int	file_no ;
char	*c_mesg ;
{
	Tbl_struct	*handle ;

	if ( ( handle = get_cursor( file_no, CREAT_TBL, 0, 0, RANDOM,
						c_mesg) ) == NULL) {
		if(iserror == ORTBLEXISTS) return(NOERROR) ;
		return(ERROR) ;
	}

	if ( oexec ( handle->cursor ) )
		return ( errrpt( file_no, handle, ORAERROR, c_mesg ) );

	return ( NOERROR ) ;
}

int
creat_indx(file_no, key_no, c_mesg)
int	file_no,
	key_no ;		/* If key_no == 0 , UNIQUE Index is created */
char	*c_mesg ;
{
	Tbl_struct	*handle ;

	if ( ( handle = get_cursor ( file_no, CREAT_INDX, key_no, 0, RANDOM,
						c_mesg) ) == NULL) {
		if(iserror == ORINDXEXISTS) return(NOERROR) ;
		return(ERROR) ;
	}

	if ( oexec ( handle->cursor ) )
		return ( errrpt( file_no, handle, ORAERROR, c_mesg ) );

	return ( NOERROR ) ;
}

int
drop_tbl(file_no, c_mesg)
int	file_no ;
char	*c_mesg ;
{
	Tbl_struct	*handle ;

	close_file(file_no) ;	/* Close all cursors before droping it */

	if ( ( handle = get_cursor( file_no, DROP_TBL, 0, 0, RANDOM,
						c_mesg) ) == NULL) {
		if(iserror == ORTBLNOTEXISTS) return(NOERROR) ;
		return(ERROR) ;
	}

	if ( oexec ( handle->cursor ) )
		return ( errrpt( file_no, handle, ORAERROR, c_mesg ) );

	return ( NOERROR ) ;
}

int
roll_back(c_mesg)
char	*c_mesg ;
{
	char	o_mesg[133] ;

	if(dbh_init == 0) return( NOERROR ) ;

	if ( orol(LDA)) {
#ifdef	DEBUG
		fprintf(sqlfp, "Rollbk Error: code: %d op: %d\n", lda.csrrc,
			lda.csrfc);
#endif
		oermsg( lda.csrrc, o_mesg) ;

		/* Truncate to max. 80 characters for display through profom
		   screen */
		strncpy(c_mesg, o_mesg, 79) ;
		c_mesg[79] = '\0' ;
		log_err(-1, "orol", "roll_back", c_mesg);
		dbexit(ORAERROR, ERROR)
	}
	return( NOERROR ) ;
}

/*
*	Commits all cursors opened thus far ..  
*/

int
commit(c_mesg)
char	*c_mesg ;
{
	char	o_mesg[133] ;

	if(dbh_init == 0) return( NOERROR ) ;

	if ( ocom(LDA)) {
#ifdef	DEBUG
		fprintf(sqlfp, "Commit Error: code: %d op: %d\n", lda.csrrc,
			lda.csrfc);
#endif
		oermsg( lda.csrrc, o_mesg) ;

		/* Truncate to max. 80 characters for display through profom
		   screen */
		strncpy(c_mesg, o_mesg, 79) ;
		c_mesg[79] = '\0' ;
		log_err(-1, "ocom", "roll_back", c_mesg);
		roll_back(o_mesg) ;
		dbexit(ORAERROR, ERROR)
	}
	return( NOERROR ) ;
}


/*
* 	Closes all cursor and frees the allocation for the cursor and 
*	the Tbl_lst data structure ..
* 	
*	Should be sparingly used unlike ISAM based DBH as it releases all
*	cursor related data structure , SQL statement and its binding etc ..
*/

int
close_dbh()
{
	int	i ;

	/** Safety measure: return if dbh is already closed **/

	if( dbh_init == 0 ) return(0);

	for ( i = 0 ; i < TOTAL_FILES ; i++)
		close_file ( i ) ;

	if( sql_rec != NULL )
		free ( sql_rec ) ;
	if( keysarray != NULL )
		free ( (char *)keysarray ) ;

#ifdef	SECURITY
	SecurityStatus = -1 ;
#endif

	orol(LDA) ;	/* to be safe, roll back before logging off */

	/* 
	* We have freed the cursors already.. This could have been dangerous as 
	* ologog call might refer cursors for pending trnasaction to commit it..
	* But since we have not reused the memory yet, the log_off call should
	* be able to refer it ..
	*/

	ologof ( LDA ) ;
	dbh_init = 0 	;

#ifdef	DEBUG
	if(sqlfp != NULL && sqlfp != stderr)
		fclose(sqlfp) ;
#endif

	return(0) ;
}

int
close_file(file_no)
int	file_no ;
{
	Tbl_struct	*ptr, *temp ;

	for ( ptr = tbl_lst[file_no]; ptr != NULL ; ) {
		oclose ( ptr->cursor ) ;
		if(ptr->cursor != NULL)
			free ( (char *)ptr->cursor ) ;
		if(ptr->stmnt_ptr != NULL)
			free ( ptr->stmnt_ptr ) ;
		temp = ptr->nxt_lst ;
		free ( (char *)ptr ) ;
		ptr = temp ;
	}
	tbl_lst[file_no] = NULL ;

	id_array[file_no].id_start = RANDOM ;
	id_array[file_no].id_lock[0] = '0' ;

	return(0) ;
}


/*
*	getfileno()
*
*	routine to get the file_no for a given file name
*/

int
getfileno(name)
char	*name ;
{
	int	i ;

	if ( dbh_init == 0 )
		if ( init_dbh() < 0 ) return(ERROR) ;

	for( i = 0 ; i < TOTAL_FILES ; i++)
		if(strcmp(id_array[i].id_f_name, name) == 0) return(i) ;

	dbexit(FLNOTEXST,ERROR)
}

/*
*	getflnm()
*
*	routine to get the file name of a DBH file
*/

int
getflnm(file_no, name)
int	file_no ;
char	*name ;
{
	if ( dbh_init == 0 )
		if ( init_dbh() < 0 ) return(ERROR) ;

	if ( file_no < 0 || file_no >= TOTAL_FILES )
		dbexit(INVFILENO,ERROR)

	strcpy( name, id_array[file_no].id_f_name );

	return( 0 ) ;
}

/*
*	getuserflnm()
*
*	routine to get the User file name of a DBH file
*/

int
getuserflnm(file_no, name)
int	file_no ;
char	*name ;
{
	if ( dbh_init == 0 )
		if ( init_dbh() < 0 ) return(ERROR) ;

	if ( file_no < 0 || file_no >= TOTAL_FILES )
		dbexit(INVFILENO,ERROR)

	strcpy( name, id_array[file_no].fl_user_nm );

	return( 0 ) ;
}

/*
*	getfiletype()
*
*	routine to get the file type of a DBH file
*/

int
getfiletype(file_no)
int	file_no ;
{
	if ( dbh_init == 0 )
		if ( init_dbh() < 0 ) return(ERROR) ;

	if ( file_no < 0 || file_no >= TOTAL_FILES )
		dbexit(INVFILENO,ERROR)

	return( id_array[file_no].id_f_type );
}

/*
*	getreclen()
*
*	routine to get the Record Length of a DBH file
*/

int
getreclen(file_no)
int	file_no ;
{
	if ( dbh_init == 0 )
		if ( init_dbh() < 0 ) return(ERROR) ;

	if ( file_no < 0 || file_no >= TOTAL_FILES )
		dbexit(INVFILENO,ERROR)

	return(id_array[file_no].reclen) ;
}


/*
*	karraysize()
*
*	routine to get the size of Keysarray(no of ints) allocated for a
*	DBH file
*/

int
karraysize(file_no)
int	file_no ;
{
	if ( dbh_init == 0 )
		if ( init_dbh() < 0 ) return(ERROR) ;

	if ( file_no < 0 || file_no >= TOTAL_FILES )
		dbexit(INVFILENO,ERROR)

	if( file_no < TOTAL_FILES - 1) {	/* Not a last file */
		/* Return the difference between next file and this file
		   offsets */
		return(id_array[file_no+1].keys_offset -
			id_array[file_no].keys_offset);
	}
	else {	/* Last File */
		/* Return the difference between Total size and this file
		   offset */
		return((keysarray[0] - id_array[file_no].keys_offset)+1);
	}
}


/**
**
**	Copying source file's ( f_id and keysarray area )
**		to dest file's( f_id and keysarray area )
**		except the keys offset
**
**/

int
set_id( to_file_no, from_file_no, e_mesg) 
int	to_file_no ,
	from_file_no ;
char	*e_mesg;
{
	int	off_from_file, off_to_file;

	if ( dbh_init == 0 )
		if(init_dbh() < 0) {
			strcpy(e_mesg, "ERROR in Initializing DBH");
			return(ERROR) ;
		}

	if ( from_file_no < 0 || from_file_no >= TOTAL_FILES ){
		sprintf(e_mesg, "ERROR: Invalid File Number %d", from_file_no);
		return(ERROR) ;
	}

	if ( to_file_no < 0 || to_file_no >= TOTAL_FILES ){
		sprintf(e_mesg, "ERROR: Invalid File Number %d", to_file_no);
		return(ERROR) ;
	}

	if( karraysize(to_file_no) < karraysize(from_file_no) ) {
		strcpy(e_mesg,
		"Temporary Keysarray size is exceeding the Allocated size") ;
		return(ERROR) ;
	}

	close_file(to_file_no) ;	/* Safe if it is open */

	off_to_file   = id_array[to_file_no].keys_offset;
	off_from_file = id_array[from_file_no].keys_offset;

	scpy( (char*)&id_array[to_file_no], (char *)&id_array[from_file_no],
				sizeof(f_id) );

	/* Copy the keysarray of From_file to To_file */
	id_array[to_file_no].keys_offset = off_to_file; 
	scpy((char*)&keysarray[off_to_file], (char*)&keysarray[off_from_file],
		(int)(karraysize(from_file_no) * sizeof(int)) );

	return(0) ;
}

/*
*	unlink_file()
*
*	Function to Delete all rows form a geven table
*/

int
unlink_file(file_no)
int	file_no ;
{
	Tbl_struct	*handle ;
	char		c_mesg[133] ;

	close_file(file_no) ;	/* If it is open */

	if ( ( handle = get_cursor ( file_no, DELETE_ALL , 0, 0, RANDOM,
		c_mesg)) == NULL) return(ERROR) ;

	if ( oexec ( handle->cursor ) )
		return ( errrpt( file_no, handle, ORAERROR, c_mesg ) );

	commit(c_mesg) ;

	return(0) ;
}

/*
*	get_maxsno()
*
*	This is new call introduced in ORACLE DBH. This returns MAX value of
*	given key part (0 to n-1 or -1 for nth part), where the column valuess
*	are matching to first n-1 parts values for a given key_no.
*/

long
get_maxsno(file_no, record, key_no, part_no, c_mesg)
int	file_no ;
char	*record ;
int	key_no,
	part_no;	/* -1 - on last part, 0 to n-1 part */
char	*c_mesg;
{
	Tbl_struct	*handle;

	/* NOTE: Here part_no is passed as a mode */

	if((handle = get_cursor(file_no, MAX_RECNO, key_no, part_no, RANDOM,
		c_mesg)) == NULL) return(ERROR);

	scpy( sql_rec, record, id_array[file_no].reclen ) ;

	CheckForNull(file_no, key_no) ;	/* Replace '\0' with ' ' */

	if( oexec (handle->cursor) || ofetch(handle->cursor) )
		return ( errrpt( file_no, handle, ORAERROR, c_mesg ) );

	ocan(handle->cursor);

	return(Seqrec_no);
}


/*
*	log_err()
*
*	Routine to append i-o errors to ERRLOG file, for the files opened
*	under DBH.
*
*	User : mainly DBH routines ..
*
*	Working: Position at the end of ERRLOG File and append the error
*		 after forming appropriate packet.
*/

static	int
log_err ( file_no, in_request, in_routine, rec_key )
int	file_no ;
char	*in_request ;
char	*in_routine ;
char	*rec_key ;
{
	int	i;
	err_log	err_rec ;
	char	*e_ptr ;
	char	err_file[PATH_LEN] ;
	int	log_fd ;

	/* start of code.. */
	for(i = 0, e_ptr = (char*)&err_rec ; i < sizeof(err_log) ; i++)
		*(e_ptr+i) = ' ';

	sprintf(err_rec.err_is, "%-5d", iserror) ;
	sprintf(err_rec.err_db, "%-5d", dberror);
	sprintf(err_rec.err_no, "%-5d", errno) ;

	strncpy ( err_rec.user , User_Id,10);
	strncpy ( err_rec.prog_nm , PROG_NAME,10);
	if(dbh_init && file_no != -1)
		strncpy(err_rec.on_file , id_array[file_no].id_f_name ,9) ;
	else if(file_no != -1)
		sprintf(err_rec.on_file, "File: %d", file_no) ;

	if(in_routine != NULL)
		strncpy(err_rec.routine , in_routine ,9) ;
	if(in_request != NULL)
		strncpy(err_rec.request , in_request,7) ;
	if(rec_key != NULL)
		strncpy(err_rec.key, rec_key, 40);

	for(i = 0, e_ptr = (char*)&err_rec ; i < sizeof(err_log) - 1 ; i++)
		if(*(e_ptr+i) == '\0') *(e_ptr+i) = ' ';
	*(e_ptr+i) = '\n' ;	/* Last Char of Record */

	/****
	strcpy(err_file, DATA_PATH) ;
	strcat(err_file, ERR_LOG);
	****/
	form_f_name(ERR_LOG, err_file);

	if ( (log_fd=open (err_file, TXT_RWMODE)) < 0 )
		if( (log_fd = creat(err_file,TXT_CRMODE)) < 0)  {
			printf("Open Error On %s... errno: %d ... Contact Systems Manager\n",
				err_file, errno);
			return(-1) ;
		}
	
	lseek(log_fd, (long)0, 2 );
	
	write(log_fd, (char *)&err_rec, sizeof(err_rec) ) ;

	close(log_fd);
	return (0);
}

#ifdef	DEBUG
/*
*	prints sql string in 80 column mode ..
*/
prnt_sql( str )
char	*str ;
{
	fprintf( sqlfp, "%s\n\n", str) ;
}
#endif
/*-----------------------------END OF FILE----------------------*/

