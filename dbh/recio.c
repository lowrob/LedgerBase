/************************************************************

	file ._ recio.c
	
	Function - To call isam routines after performing necessary file opening
		closing etc .

	User Calls ._
		get_isrec() 	** Gets the user record from a ISAM file **
		put_isrec()	** Puts a record in to a ISAM file **
		get_next()	** Gets the next record from a ISAM file **
		get_rec() 	** Gets the user record from a SEQ file **
		put_rec()	** Puts a record in to a SEQ file **
		get_seqrec()	** Gets the next record from a SEQ file **
		close_file()	** CLoses a DBH file ..called by close_dbh **
		close_dbh()	** Closes all files opened under DBH **
		relse_file()	** Releases all lockes from a DBH file **
		release_dbh()	** Calls release_file for all DBH files **
		flg_start()	** Tests sequentialness of an file **
		flg_reset()	** Forces next get_next call to position the
				   file on a given record **
		seqover()	** Releases node locks. To be called after end
				   of Sequential read on ISAM file **
		getfileno()	** Get file no of a DBH file given its name **
		getflnm()	** Get file name of a DBH file given its id **
		getuserflnm()	** Get user file name of a DBH file given its
				   id **
		getdatafd()	** Get data file fd of a DBH file given its id
				   and it is open **
		getfiletype()	** Return file type(i.e ISAM or SEQ) of a DBH
				   file given its id **
		getreclen()	** Get Rec Length of a DBH file given its id **
		form_key()	** Forms key string for a given file# & key# **
		lock_file()	** Locks(Read Lock) complete file for a given
				   file# data file **
		unlock_file()	** Releases all locks on data file for a given
				   file# **
		unlink_file()	** Unlinks the files of given file# **
		karraysize()	** routine to get the size of Keysarray
				   (no of ints) allocated for a	DBH file **
		set_id()	** copies f_id & keysarray area **
		
	Internal Calls ._

		init_file()	** Sees to it that a file has been opened **
		chmode()	** Called to call init_file if file is closed **
		init_dbh()	** Reads file "keydesc.id" to load file & DBH
				   parameters **
		log_err()	** Log the error in read/write **
		seek_file()	** Positions SEQ file at given record# **
		readdata()	** Reads data form SEQ file. Locks if
				   necessary **
		start_file()	** Positions a file on a Given record **	
		crt_msg()	** Creates appropriate error message for given
				   file#, key# & error code, to pass to User **
		cp_msg()	** Returns appropriate string for a given error
				   code **

	Author ._ K. Sharma .

Modification History ._ ????

	Journaling & SEQ files logic Added . Dt 24-JUN-89. Kvs .

14-May-90:- Amarendra T			Change to File closing Policy:
	When the no-of-files opened is exceeding the OPEN_LIMIT, earlier all
	the files which are in RANDOM access mode are being closed. This is
	causing a problem when there are locks on such file. The new policy is
	close the file only when the mode is RANDOM access and no locks existing
	on that. To check for existence of locks id_lock[] flag is being used.
	Whenever get call is issued with UPDATE mode this flag is set to '2' for
	that file. Later relse_file() will set this flag to '0', when commit()
	or roll_back() is called.

*****************************************************************/

				
#define  FILEST 1	/* to supress some definitions which are common in
			   isnames.h and USERdef.h */

#include <stdio.h>
#include <isnames.h>
#include <bfs_defs.h>
#include <filein.h>
#include <dberror.h>
#ifdef	JRNL
#include <journal.h>
#endif

#define	ID_SIZE		(sizeof(f_id)*TOTAL_FILES)

#ifndef	MS_DOS
#define	OPEN_LIMIT	40	/* Assuming UNIX versions are SYSTEM-V or
				   above and ISAM can open 20 INDEX files  */
#else
#define	OPEN_LIMIT	14
#endif

#define	PATH_LEN	60

static 	int	open_count = 0;

static 	int	close_all = 1 ;		/* if complete DBH to be closed */

static	int	request ;		/* Create file for write request only*/

static	int	dbh_init = 0 ;		/* To read in id_array from descriptor
					   file */
int	*alt_array = NULL ;		/* ISAM keys write status array. Alloca-
					   ted to MAX keys in init_dbh() */
int	*keysarray = NULL ;		/* Keys array read from keydesc.id */

long	lseek();

/************* Function & protocol definitions ************/

static long seek_file(int, int) ;
static int readdata(int, char *, long, int );
static int crt_msg(int, int, int, char *, char *);


#ifdef JRNL
extern	int	journaling ;		/* Enable/Disable journaling */
#endif

#ifdef SECURITY
extern	int	SecurityStatus ;	/* Security ON/OFF */
#endif

/*-----------------------------------------------------------------*/

/*
*	init_file()
*
*	Function to initialise (creat/open) file.
*
*	User : mainly DBH routines ..
*
*	Working :if the file's FD is nonnegative then the file is considered
*		initialised, else an open/create is attempted on it and the FD
*		is initialiased in fd_array ..
*
*/

int
init_file(file_no)
int	file_no ;
{
	char	path_name[PATH_LEN] ;
	char	path_ind[PATH_LEN] ;
	int	fd ;

	if ( file_no >= TOTAL_FILES || file_no < 0 ) dbexit(INVFILENO, ERROR)

	/*-- See if id_array is initialised.. If not then read in file --*/

	if ( dbh_init == 0 )
		if ( init_dbh() < 0 ) return(ERROR) ;

	/**--- See if Too many files are Open Under DBH --***/

	if((id_array[file_no].id_f_type == SEQ && open_count >= OPEN_LIMIT) ||
	  (id_array[file_no].id_f_type == ISAM && open_count >= OPEN_LIMIT-1)) {
		/***
#ifdef ENGLISH
		printf ( "DBH being closed.. too many files..\n");
#else
		printf ( "DBH ferme...trop de dossiers..\n");
#endif
		***/
		close_all = 0 ;
		close_dbh();
		close_all = 1 ;
	}

	if ( id_array[file_no].id_fd >= 0 )
		return (id_array[file_no].id_fd) ;

	form_f_name(id_array[file_no].id_f_name, path_name);

	if(id_array[file_no].id_f_type == ISAM) {
		strcpy ( path_ind, path_name) ;
		strcat ( path_ind, ".IX" ) ;
	}

	if ( access(path_name, 0) &&
		(id_array[file_no].id_f_type == SEQ || access(path_ind, 0)) ){

		/*-- error on read request and the file is missing -- */

		if ( request == BROWSE ) dbexit(FLNOTEXST, ERROR)

		/* create file afresh .. */

		if(id_array[file_no].id_f_type == SEQ) {
		    if((fd = creat(path_name, CRMODE)) < 0)
				dbexit(SEQCRTERR, ERROR)

		    close(fd) ;

		    if((fd = open(path_name,id_array[file_no].id_io_mode)) < 0)
			dbexit(SEQOPNERR, ERROR)
		}
		else {	/* ISAM FILE */
			if((fd=iscreat(path_name,
				id_array[file_no].id_io_mode,
				id_array[file_no].tot_keys,
				&keysarray[id_array[file_no].keys_offset]
#ifdef	FIXED_RECLEN
				,id_array[file_no].reclen
#endif
			)) == ERROR)
				dbexit(ISMCRTERR, ERROR)
		}
	}
	else {
		if(id_array[file_no].id_f_type == SEQ) {
		    if((fd = open(path_name, id_array[file_no].id_io_mode)) < 0)
			dbexit(SEQOPNERR, ERROR)
		}
		else {	/* ISAM File */
			/* see if one of the files is missing ..*/

			if ( access(path_name, 0) || access( path_ind, 0 ) )
				dbexit(FLMISSING, ERROR)

			/* open files and return ffs.. */

			if( (fd = isopen (path_name,
				id_array[file_no].id_io_mode))== ERROR)
				dbexit(ISMOPNERR, ERROR)
		}
	}

	id_array[file_no].id_fd = fd ;

	if(id_array[file_no].id_f_type == SEQ) {
		id_array[file_no].id_data = fd ;	/* No seperate data_fd
							   for SEQ files */
		open_count++;
	}
	else {
		id_array[file_no].id_data = data_fd ;
		open_count += 2 ;
	}

	return ( fd ) ;
}

/*
*	init_dbh()
*
*	Read the key descriptor in id_array
*/

int
init_dbh()
{
	char	key_file[PATH_LEN] ;
	int	security, i, no_keys, fd ;
#ifdef	JRNL
	int	jrnl_sz ;
#endif

	if ( dbh_init == 1 ) return(0) ;

	DT_TYPE = DATE_TYPE ;	/* ISAM Date type. Defined in XXXX_def.h */

	strcpy(key_file, DATA_PATH) ;
	strcat(key_file, KEY_DESC );
	/***
	form_f_name(KEY_DESC, key_file);
	****/

	if ( (fd=open(key_file, RDMODE)) < 0 )
		dbexit(KEYOPNERR, ERROR)

#ifdef	JRNL
	/* Read Journalling On/OFF flag */
	if(read(fd, (char*)&journaling, sizeof(int)) < sizeof(int)) {
		close(fd);
		dbexit(KEYSIZERR,ERROR)
	}
	/* Read Journalling area */
	if(read(fd, (char*)&jrnl_sz, sizeof(int)) < sizeof(int)) {
		close(fd) ;
		dbexit(KEYSIZERR,ERROR)
	}
#endif
#ifdef	SECURITY
	/* Read Security On/OFF flag */
	if(read(fd, (char*)&security, sizeof(int)) < sizeof(int)) {
		close(fd);
		dbexit(KEYSIZERR,ERROR)
	}
#endif
	/* Read keys information */
	if ( read(fd, (char *)id_array, ID_SIZE) < ID_SIZE ) {
		close(fd) ;
		dbexit(KEYSIZERR,ERROR)
	}

	if ( read(fd, (char *)&no_keys , sizeof(int)) < sizeof(int)) {
		close(fd) ;
		dbexit(KEYSIZERR,ERROR)
	}

	/* Allocate the size for no of parts & 1 more to store size in the
	   beginging of array */
	if ( (keysarray = (int *)malloc( (no_keys+1) * sizeof(int) )) == NULL ) 
		dbexit(MALLOCERR,ERROR)

	keysarray[0] = no_keys ;

	if ( read(fd, (char *)(keysarray+1) , sizeof(int) * no_keys) 
					< sizeof(int) * no_keys){
		close(fd) ;
		dbexit(KEYSIZERR,ERROR)
	}
	close(fd) ;
     
#ifdef	JRNL
	/*
	*	Allocate 'Cursor' for commit and rollback areas ..
	*/

	if(journaling) {
		set_jrnl_sz(jrnl_sz) ;	/* Set Jrnl size to user given size */
		if(init_journal() < 0)
			return(ERROR) ;	
	}
#endif

	/* Find out Maximum ISAM keys and allocate alt_array */
	no_keys = 0;
	for(i = 0 ; i < TOTAL_FILES ; i++)
		if(id_array[i].id_f_type == ISAM &&
				id_array[i].tot_keys > no_keys)
			no_keys = id_array[i].tot_keys ;

	if(no_keys > 0) {
		alt_array = (int *)malloc(no_keys * sizeof(int));
		if(alt_array == NULL) dbexit(MALLOCERR,ERROR)
	}
	
	dbh_init = 1 ;

	/* security should be turned on/off after dbh_init is set to 1.
	   Otherwise it might go to recursion */
#ifdef	SECURITY
	if(security == 1)
		SecurityStatus = 1 ;	/* Turn ON */
	else
		SecurityStatus = 0 ;	/* Turn OFF */
#endif

	return(0) ;
}

/*
*	chmode()
*
*	Change file access mode.
*/

static	int
chmode ( file_no, new_mode)
int	file_no,
	new_mode ;
{
	int	fd ;

	errno = iserror = 0 ;	

	if ( dbh_init == 0 )
		if ( init_dbh() < 0 ) return(ERROR) ;

	if ( file_no < 0 || file_no >= TOTAL_FILES )
		 dbexit(INVFILENO, ERROR)

	/* If the file is ISAM and trying to do SEQ file operations
	   return ERROR */
	if(id_array[file_no].id_f_type == ISAM &&
			(new_mode != IRAND && new_mode != IDYN) )
		dbexit(NONISMMOD, ERROR)

	/* If the file is SEQ and trying to do ISAM file operations
	   return ERROR */
	if(id_array[file_no].id_f_type == SEQ &&
			(new_mode != SRAND && new_mode != SDYN) )
		dbexit(NONSEQMOD, ERROR)

	if ( (fd = id_array[file_no].id_fd) < 0  )
		if( (fd = init_file(file_no)) < 0 ) return(ERROR) ;

	if ( new_mode == IDYN || new_mode == SDYN )
		id_array[file_no].id_start = fd ;	/* This will be set to
					   reading direction in calling fn() */
	else
		id_array[file_no].id_start = -1 ;

	if(id_array[file_no].id_f_type == ISAM)
		isdone(fd) ;		/* Unlock node if  locked */

	return ( fd ) ;
}


/*
*	close_file()
*
*	Function to close a file opened under DBH.
*
*	User: Mainly DBH
*
*	Working: If the fd of the file is negative then it is considered
*		closed, else close/isclose is attempted on it .
*/


int
close_file ( file_no )
int	file_no ;
{
	int	fd ;

	if ( dbh_init == 0 ) return(0) ;
	if ( file_no  >= TOTAL_FILES || file_no < 0 )dbexit(INVFILENO, ERROR)
	if ( (fd = id_array[file_no].id_fd) < 0 ) return(0) ;

	if(id_array[file_no].id_f_type == SEQ) {
		close(fd);
		open_count-- ;
	}
	else {	/* ISAM files */
		if ( isclose (fd) == ERROR ) dbexit(ISMCLSERR, ERROR)

		/*-- reduce number of open files by 2 for ISAM ---*/

		open_count -= 2;
	}

	id_array[file_no].id_fd = -1 ;	/* signal close */

	/*-- reset start flag for next start --*/

	id_array[file_no].id_start = -1 ;

	/*--- reset the lock flag ---*/

	id_array[file_no].id_lock[0] = '0' ;

	return(0) ;
}

/*
*	close_dbh()
*
*	Function to close all files open under DBH.
*/

int
close_dbh()
{
	int	i;

	if ( dbh_init == 0 ) return(0);

	for ( i = 0 ; i < TOTAL_FILES ; i++ ) {
	    if(id_array[i].id_fd >= 0 && (close_all ||
		(id_array[i].id_lock[0] == '0' && id_array[i].id_start < 0 ))){
		/*****
		(id_array[i].id_f_type == ISAM && id_array[i].id_start < 0 ))){
		****/

		    if(close_file(i) == ERROR)
#ifdef ENGLISH
			printf("ERROR Iserror: %d Dberror: %d in Closing file: %s\n",
					iserror,dberror,id_array[i].id_f_name);
#else
			printf("ERREUR Iserror: %d Dberror: %d en fermant le dossier: %s\n",
					iserror,dberror,id_array[i].id_f_name);
#endif

	    }
	}

	if(close_all && open_count > 0)
#ifdef ENGLISH
		printf("All files could not be closed\n");
#else
		printf("Tous les dossiers ne pouvaient pas etre fermes\n");
#endif

	if(close_all) {	/* Close complete DBH */
#ifdef	JRNL
		free_journal() ;	/* Free Journal area */
#endif
		if(keysarray != NULL)
			free((char*)keysarray) ;
		keysarray = NULL ;
		if(alt_array != NULL)
			free((char*)alt_array) ;
		alt_array = NULL ;

		dbh_init = 0 ;		/* Initialize files again */
	}

	return(0) ;
}
/*
*	lock_file()
*
*	Function to lock a complete file .. No writes/updates by other users .
*
*/

int
lock_file(file_no)
int	file_no ;
{
	int	wait, size ;
	long	pos ;
	int	code ;

	request = UPDATE ;	/* To creat file if it doesn't exist */

	if ( init_file(file_no) == ERROR ) return(ERROR) ;

	/***    lock the data file ..  with write_lock ***/

	wait 	= 0 ;		/* No Wait for successful lock  */
	size	= 0 ;		/* lock the entire file 	*/
	pos 	= 0L ;		/* from the start 		*/

	code = e_lock(id_array[file_no].id_data, WRLOCK, wait, pos, size) ;

	if ( code == 0 ) {
		if ( (code=e_lock(id_array[file_no].id_data, RDLOCK, wait, pos,
						 size)) < 0 ) return(code) ;
		}
	else 
		if ( code < 0 ) return(code) ;

	id_array[file_no].id_lock[0] = '1' ;

	return(0) ;
}

/*
*	unlock_file()
*
*	Function to Unlock a file ..
*
*/

int
unlock_file(file_no)
int	file_no ;
{
	int	wait, size ;
	long	pos ;

	if ( dbh_init == 0 ) return(0) ;

	if ( file_no >= TOTAL_FILES || file_no < 0 )dbexit(INVFILENO, ERROR)

	if ( id_array[file_no].id_fd < 0 ) return(0) ;

	if(id_array[file_no].id_lock[0] == '0') return(0) ;

	/***    Unlock the data file, which is locked using lock_file(), and
		reset the lock flag .. ***/

	wait 	= 0 ;	
	size	= 0 ;		/* Unlock the entire file 	*/
	pos 	= 0L ;		/* from the start 		*/

	if ( e_lock(id_array[file_no].id_data, UNLOCK, wait, pos, size) < 0 )
						dbexit(LOCKRELER, ERROR) 
	id_array[file_no].id_lock[0] = '0' ;

	return(0) ;
}

/*
*	relse_file()
*
*	Function to release all locks from a DBH file.
*/

int
relse_file(file_no)
int	file_no ;
{
	int	fd ;

	if ( dbh_init == 0 ) return(0) ;

	if ( file_no >= TOTAL_FILES || file_no < 0 )dbexit(INVFILENO, ERROR)

	if ( (fd = id_array[file_no].id_fd) < 0 ) return(0) ;

	/***	DO not unlock the file if lock flag is set 	***/

	if ( id_array[file_no].id_lock[0] == '1' ) return(0) ;

	if(id_array[file_no].id_f_type == SEQ) {
		if(e_lock(fd, UNLOCK, 0, 0L, 0) < 0) dbexit(LOCKRELER, ERROR) ;
	}
	else {	/* ISAM File */
		if ( isrelease(fd) < 0 ) dbexit(LOCKRELER, ERROR)
	}

	id_array[file_no].id_lock[0] = '0' ;

	return(0) ;
}

/*
*	release_dbh()
*
*	Routine to release all locks from all files opened under DBH
*/

int
release_dbh()
{
	int	i;

	if ( dbh_init == 0 ) return(0);

	for ( i = 0 ; i < TOTAL_FILES ; i++ ) {
	    if (id_array[i].id_fd >= 0) {
		if ( relse_file ( i ) == ERROR )
		    printf(
#ifdef ENGLISH
			"ERROR in releasing Locks %s Dberror %d Iserror %d\n",
			id_array[i].id_f_name,dberror,iserror);
#else
			"ERREUR en relachant les verrous %s Dberror %d Iserror %d\n",
			id_array[i].id_f_name,dberror,iserror);
#endif
		}
	}
	return(0) ;
}

/*
*	flg_start()
*
*	Routine to test sequentialness of an isam file. Returns current setting
*	of flag, i.e, -1 (in RAND), 0(in DYN forward) and 1(in DYN backward).
*/

int
flg_start ( file_no )
int	file_no ;
{
	if ( dbh_init == 0 ) return(-1) ;
	return( id_array[file_no].id_start );
}

/*
*	flg_reset()
*
*	Routine to reset id_start to -1. This will force next get_next()
*	to give start_file.
*/

int
flg_reset(file_no)
int	file_no;
{
	id_array[file_no].id_start = -1;
	return(0);
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
log_err ( in_file, record, key_no, in_request, in_routine )
int	in_file ;
int	key_no ;
char	*record ;
char	*in_request ;
char	*in_routine ;
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
	if(dbh_init)
		strncpy(err_rec.on_file , id_array[in_file].id_f_name ,9) ;
	else
		err_rec.on_file[0] = '\0' ;
	strncpy(err_rec.routine , in_routine ,9) ;
	strncpy(err_rec.request , in_request,7) ;
	/* err_rec.key is allocated for 40 characters. If the key is bigger
	   than 40 form_key will goof */
	form_key(record, in_file, key_no, err_rec.key);

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
#ifdef ENGLISH
			printf("Open Error On %s... errno: %d ... Contact Systems Manager\n",
				err_file, errno);
#else
			printf("Erreur ouverte sur %s... errno: %d ...Contactez groupe support du logiciel\n",
				err_file, errno);
#endif
			return(-1) ;
		}
	
	lseek(log_fd, (long)0, 2 );
	
	write(log_fd, (char *)&err_rec, sizeof(err_rec) ) ;

	close(log_fd);
	return (0);
}

/*
*	get_rec()
*
*	Routine to get a record for a Record#, form a SEQ file.
*
*	Function: Lseeks to the given record and reads record into
*		the 'record'.
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
	long	position ;

#ifdef	SECURITY
	if(SecurityStatus != 0) {
		/* Security Check */
		code = CheckAccess( file_no, BROWSE, c_mesg );
		if( code < 0 ) return(code);
	}
#endif

	/*-- set request to read and try to access file using chmode --*/

	request = BROWSE ;

	if ( chmode (file_no, SRAND) == ERROR ) {
		if(dberror == FLNOTEXST) {
			crt_msg(file_no,rec_no,UNDEF,c_mesg,record);
			return(UNDEF);
		}
		goto last ;
	}

#ifdef JRNL /*-- See if the record is in commit or rollback area --*/

	code = SrchJournal(record, file_no, rec_no, 0) ; 
	if ( code < 0 && code != UNDEF ) goto last ;
	if(code == P_DEL) {
		crt_msg(file_no,rec_no,UNDEF,c_mesg,record) ;
		dbexit(NODBERROR,UNDEF) ;
	}
	if ( code == 0 ) return( rec_no ) ;
#endif
	if(rec_no > 0) {
		position = seek_file(file_no, rec_no) ;

		if(position == ERROR) goto last ;

		if(position == EFL)
			code = UNDEF ;
		else {
			code = readdata(file_no, record, position,
						(mode == BROWSE) ? 0 : 1 ) ;
			if(code == 0) code = UNDEF ;
		}
	}
	else
		code = UNDEF ;

	if(code == LOCKED || code == UNDEF) {
		crt_msg(file_no, rec_no, code, c_mesg, record) ;
		dbexit(NODBERROR, code);
	}

	if(code < id_array[file_no].reclen) {
		dberror = SEQREADER ;
		goto last ;
	}

#ifdef JRNL
	/* Write to rollback area .. */
	if ( mode != BROWSE && journaling) {
		code = writelog(record, file_no, mode, rec_no, ROLLBK_AREA);
		if(code < 0) goto last ;
	}
#endif
	if( mode != BROWSE && id_array[file_no].id_lock[0] == '0' )
		id_array[file_no].id_lock[0] = '2' ;	/* Rec level lock */

	return(rec_no) ;

last :
	log_err ( file_no, record, rec_no, "READ", "get_rec" ) ;
	crt_msg(file_no,rec_no,ERROR,c_mesg,record);
	return(ERROR) ;
}

/*
*	put_rec()
*
*	routine to write the record on to SEQ file.
*/

int
put_rec ( p_rec, mode, file_no, rec_no, c_mesg)
char	*p_rec ;
int	mode ;		/* ADD or UPDATE.. */
int	file_no ;	/* index in to id array .. */
int	rec_no ;
char	*c_mesg ;
{
	int	code, fd, wait ;
	char	modestr[8] ;
	long	position ;

	/* ------- start of code ----------*/

#ifdef	SECURITY
	if(SecurityStatus != 0) {
		/* Security Check */
		code = CheckAccess( file_no, mode, c_mesg );
		if( code < 0 ) return(code);
	}
#endif

	/*-- set request to write and try to access file in correct mode
	   using chmode ---*/

	request = ADD ;
	if ( (fd = chmode (file_no, SRAND) ) == ERROR ) {
		code = ERROR;
		strcpy(modestr,"CHMODE");
		goto err_last ;
	}

#ifdef JRNL
	/* Write to commit area ..*/
	if ( journaling ) {
		code = writelog(p_rec, file_no, mode, rec_no, COMMIT_AREA);
		if(code < 0){
			strcpy(modestr,"WRITLOG");
			goto err_last ;
		}
		return(0);
	}
#endif

	if ( mode == ADD ) {
		position = lseek(fd,0L,2) ;

		rec_no = (position / RECLEN(file_no)) + 1 ;	/* New Record */

		/*
		* 	 write lock to ensure no other locks are held.
		* 	 Then convert to read lock
		*/

		wait = 1 ;	/* Wait to grab the file */
		code = e_lock(fd,WRLOCK,wait,position,RECLEN(file_no));
		
		if(code == 0)	/* Convert to Read lock */
			code =e_lock(fd,RDLOCK,0,position,RECLEN(file_no));

		if(code < 0) {
			dberror = ADDLOCKER ;
			strcpy (modestr, "ADD");
			goto err_last ;
		}
	}
	else if(mode == UPDATE) {
		if(rec_no < 1 || (position = seek_file(file_no, rec_no)) < 0) {
			strcpy(modestr, "UPDATE");
			goto err_last ;
		}
	}
	else {
		strcpy(modestr,"UNKNOWN");
		goto err_last ;
	}

#if	STATUS_LEN == 1
	code = write(fd, "0", 1) ;
#endif 
	code = write(fd, p_rec, id_array[file_no].reclen);

	if ( code < id_array[file_no].reclen) {
		if ( mode == ADD ) {
			strcpy (modestr, "ADD");
			goto err_last ;
		}
		else {
			strcpy(modestr, "UPDATE");
			goto err_last ;
		}
	}

	return(rec_no);

/* log the error .. */

err_last :
	log_err ( file_no, p_rec, rec_no, modestr, "put_rec" ) ;
	crt_msg(file_no,rec_no,ERROR,c_mesg,p_rec);
	return(ERROR) ;
}

/*
*	seek_file()
*
*	Issue lseek() on a SEQ file to seek to the given Record#
*/

static	long
seek_file(file_no, rec_no )
int	file_no ;
int	rec_no ;
{
	long	position ;

	if(rec_no < 1)
		position = 0;
	else
		position = (long)((rec_no - 1) * RECLEN(file_no)) ;

	if(position < lseek(id_array[file_no].id_fd, 0L, 2)) {
		if(lseek(id_array[file_no].id_fd, position, 0L) < 0)
			dbexit(LSEEKERR, ERROR)
	}
	else
		return(EFL) ;

	return(position) ;
}

/*
*	readdata()
*
*/

static	int
readdata(file_no, record, position, lock)
int	file_no ;
char	*record ;
long	position ;
int	lock ;
{
	int	code;
	char	status[2] ;

	code = 0 ;

	if(lock){

		/*
		* 	 write lock to ensure no other locks are held.
		* 	 Then convert to read lock
		*/

		code = e_lock(id_array[file_no].id_fd,WRLOCK,0,position,
							RECLEN(file_no)) ;

		if(code == 0)	/* Convert to Read lock */
		   code = e_lock(id_array[file_no].id_fd,RDLOCK,0,position,
							RECLEN(file_no));
	}
	/*****
	else if(id_array[file_no].id_lock[0] != '1')	* File not locked *
		code = e_lock(id_array[file_no].id_fd,UNLOCK,0,position,
							RECLEN(file_no)) ;
					* Releses if exists *
	******/

	if(code == LOCKED) return(LOCKED) ;

#if	STATUS_LEN == 1
	code = read( id_array[file_no].id_fd, status, 1) ;
#endif
	code = read(id_array[file_no].id_fd, record, id_array[file_no].reclen) ;

	return(code) ;
}

/*
*	get_seqrec()
*
*	Routine to get a next/previous record form a SEQ file.
*
*	Function: Lseeks to the given record and reads record into
*		the 'record'.
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
	long	position ;

#ifdef	SECURITY
	if(SecurityStatus != 0) {
		/* Security Check */
		code = CheckAccess( file_no, BROWSE, c_mesg );
		if( code < 0 ) return(code);
	}
#endif

	/*-- set request to read and try to access file using chmode --*/

	request = BROWSE ;

	if(dbh_init == 0 || id_array[file_no].id_start != direction) {
		if ( chmode (file_no, SDYN) == ERROR ) {
			if(dberror == FLNOTEXST) {
				crt_msg(file_no,0,EFL,c_mesg,record);
				return(EFL);
			}
			goto last ;
		}
		id_array[file_no].id_start = direction ;
		if(direction == BACKWARD && rec_no < 1) {	/* Previous */
			crt_msg(file_no,0,EFL,c_mesg,record);
			return(EFL);
		}

		if((position = seek_file(file_no, rec_no)) == ERROR)
			goto last ;

		if(position == EFL) {
		    position = lseek(id_array[file_no].id_fd, 0L, 2);
						/* Get EOF Position */
		    if(direction == BACKWARD) {	/* Previous */
			/* Current rec_no */
			rec_no = (position / RECLEN(file_no)) ;

			if((position = seek_file(file_no, rec_no)) < 0)
				goto last ;
		    }
		}
	}
	else {
		position = lseek(id_array[file_no].id_fd, 0L, 1);
					/* Get The Current Position */
		if(direction == BACKWARD) {	/* Previous */
			/* Previous rec_no */
			rec_no = (position / RECLEN(file_no)) ;

			if(rec_no == 1) {
				crt_msg(file_no,0,EFL,c_mesg,record);
				return(EFL);
			}
			if((position = seek_file(file_no, (rec_no - 1))) < 0)
				goto last ;
		}
	}

	rec_no = (position / RECLEN(file_no)) + 1 ;	/* Cur Rec# */

	code = readdata(file_no, record, position, (mode == BROWSE) ? 0 : 1 ) ;
	if(code == 0) code = EFL ;

	if(code == LOCKED || code == EFL) {
		crt_msg(file_no, rec_no, code, c_mesg, record) ;
		dbexit(NODBERROR, code);
	}

	if(code < id_array[file_no].reclen) {
		dberror = SEQREADER ;
		goto last ;
	}

#ifdef JRNL
	/* Write to rollback area .. */
	if ( mode != BROWSE && journaling) {
		code = writelog(record, file_no, mode, rec_no, ROLLBK_AREA);
		if(code < 0) goto last ;
	}
#endif
	if( mode != BROWSE && id_array[file_no].id_lock[0] == '0' )
		id_array[file_no].id_lock[0] = '2' ;	/* Rec level lock */

	return(rec_no) ;

last :
	log_err ( file_no, record, rec_no, "READS", "get_seqrec" ) ;
	crt_msg(file_no,rec_no,ERROR,c_mesg,record);
	return(ERROR) ;
}

/*
*	get_isrec()
*
*	Function to get a record for a given key, form an ISAM file.
*/

int
get_isrec ( record, file_no, key_no, mode, c_mesg)
char	*record ;	/* rec to be gotten in this area */
int	file_no ;	/* DBH file no .. */
int	key_no ;	/* main key = 0 , alt_key >= 1    */
int	mode ;		/* mode for locking the record if required */
char	*c_mesg;	/* return message in case of error */
{
	int	code, fd;

#ifdef	SECURITY
	if(SecurityStatus != 0) {
		/* Security Check */
		code = CheckAccess( file_no, BROWSE, c_mesg );
		if( code < 0 ) return(code);
	}
#endif
	
	/*-- set request to read and try to access file using chmode --*/

	request = BROWSE ;

	if ( (fd = chmode (file_no, IRAND) ) == ERROR ) {
		if(dberror == FLNOTEXST) {
			crt_msg(file_no,key_no,UNDEF,c_mesg,record);
			return(UNDEF);
		}
		code = ERROR;
		goto last ;
	}

#ifdef JRNL  /*-- See if the record is in commit or rollback area --*/

	code = SrchJournal(record, file_no, key_no, 0) ;
	if ( code < 0  && code != UNDEF ) goto last ;
	if(code == P_DEL) {
		crt_msg(file_no,key_no,UNDEF,c_mesg,record) ;
		dbexit(NODBERROR,UNDEF) ;
	}
	if ( code == 0 ) return(code) ;
#endif
	code = isreadr ( fd, record, key_no, (mode == BROWSE) ? 0 : 1);

	if ( code == ERROR ) goto last ;

	if ( code == LOCKED || code == UNDEF ) {
		crt_msg(file_no,key_no,code,c_mesg,record);
		dbexit(NODBERROR, code)
	}

#ifdef JRNL
	/* Write to rollback area .. */
	if ( mode != BROWSE && journaling) {
		code = writelog(record, file_no, mode, key_no, ROLLBK_AREA);
		if(code < 0) goto last ;
	}
#endif
	if( mode != BROWSE && id_array[file_no].id_lock[0] == '0' )
		id_array[file_no].id_lock[0] = '2' ;	/* Rec level lock */

	return(0) ;

last :
	log_err ( file_no, record, key_no, "READ", "get_isrec" ) ;
	crt_msg(file_no,key_no,code,c_mesg,record);
	return(code) ;
}

/*
*	put_isrec()
*
*	routine to write the record on to ISAM file.
*/

int
put_isrec ( p_rec, mode, file_no, c_mesg)
char	*p_rec ;
int	mode ;		/* ADD or UPDATE.. */
int	file_no ;	/* index in to id array .. */
char	*c_mesg ;
{
	int	code, i, fd ;
	char	modestr[8] ;

	/* ------- start of code ----------*/

#ifdef	SECURITY
	if(SecurityStatus != 0) {
		/* Security Check */
		code = CheckAccess( file_no, mode, c_mesg );
		if( code < 0 ) return(code);
	}
#endif

	/*-- set request to write and try to access file in correct mode
	   using chmode ---*/

	request = ADD ;
	if ( (fd = chmode (file_no, IRAND) ) == ERROR ) {
		code = ERROR;
		strcpy(modestr,"CHMODE");
		goto err_last ;
	}

#ifdef JRNL
	/* Write to commit area ..*/
	if ( journaling ) {
		code = writelog(p_rec, file_no, mode, 0, COMMIT_AREA);
		if(code < 0){
			strcpy(modestr,"WRITLOG");
			goto err_last ;
		}
		/* Added by J.Prescott April 5/91. because of problem found */
		/* when adding and updating the same record in the same */
		/* session. */
		if(mode == ADD) {
			code = writelog(p_rec, file_no, mode, 0, ROLLBK_AREA);
			if(code < 0){
				strcpy(modestr,"WRITLOG");
				goto err_last ;
			}
		}
		return(0);
	}
#endif
	/*
	*	Set alt_array[] array
	*/

	/* By default set all the keys to be written */
	for( i = 0 ; i < id_array[file_no].tot_keys ; i++)
		*(alt_array+i) = 1;

	/* Call user function. He might set some keys Off */
	if(set_alt(p_rec, file_no, c_mesg) < 0) dbexit(INVFILENO,ERROR)

	if ( mode == ADD ) {
#ifdef	FIXED_RECLEN
		code = iswrite ( fd, p_rec, alt_array);
#else
		code = iswrite(fd, p_rec, id_array[file_no].reclen, alt_array);
#endif
		if ( code == DUPE || code == ERROR ) {
			if ( code == DUPE ) strcpy(modestr, "DUP");
			else
				strcpy (modestr, "ADD");
			goto err_last ;
		}
	} else if ( mode == P_DEL ) {
		code = isdelete(fd, p_rec) ;	/* No alt_write Here */
		if ( code == ERROR || code == UNDEF ) {
			strcpy ( modestr, "PDEL") ;
			goto err_last ;
		}
	} else if ( mode == UPDATE ) {
#ifdef	FIXED_RECLEN
		code = isrewrite( fd, p_rec, alt_array );
#else
		code = isrewrite(fd,p_rec,id_array[file_no].reclen,alt_array);
#endif

		if(code == UNDEF || code == ERROR){
			strcpy(modestr, "UPDATE");
			goto err_last ;
		}

	} else {
		strcpy(modestr,"UNKNOWN");
		code = ERROR;
		goto err_last ;
	}

	return(0);

/* log the error .. */

err_last :
	log_err ( file_no, p_rec, 0, modestr, "put_isrec" ) ;
	crt_msg(file_no,0,code,c_mesg,p_rec);
	return(code) ;
}

/*
*	start_file()
*
*	Issue isstart() on an ISAM file with the given key.
*/

static	int
start_file ( start_rec, file_no, key_no, direction)
char	*start_rec ;
int	file_no ;
int	key_no ;
int	direction ;
{
	int	code, fd ;

	request = BROWSE ;

	errno = iserror = 0 ;	

	if(dbh_init == 1 && id_array[file_no].id_start == direction &&
				id_array[file_no].id_curkey == key_no)
		return(id_array[file_no].id_fd) ;

	if ( (fd = chmode (file_no, IDYN) ) == ERROR ) return(ERROR);
/**	if ( isdone(fd) < 0) return(ERROR); ** I Donno why ?? **/

	code = isstart ( fd, start_rec, key_no, ISEQUAL) ;

	if ( code == ERROR )  dbexit(NODBERROR,ERROR)   /* no error if UNDEF */
	id_array[file_no].id_curkey = key_no ;
	id_array[file_no].id_start = direction ;
	return(fd) ;
}


/*
*	seq_over()
*
*	routine to signal end of sequential read on ISAM file.
*/

int
seq_over(file_no)
int	file_no ;
{
	int	fd ;

	if ( file_no >= TOTAL_FILES || file_no < 0 || dbh_init == 0)
			return(ERROR);

	if ( (fd=id_array[file_no].id_fd) < 0 ) return(0) ;

	if(id_array[file_no].id_f_type == SEQ) return(0);

	return( isdone(fd) ) ;
}

/*
*	get_next()
*
*	Routine to get next/previous record from ISAM file.
*/

int
get_next( start_rec,  file_no, key_no, direction, mode, c_mesg)
char	*start_rec ;
int	file_no ;
int	key_no ;
int	direction ;		/* 0 = forward 1 = backward */
int	mode ;
char	*c_mesg ;
{
	int	code, fd ;

#ifdef	SECURITY
	if(SecurityStatus != 0) {
		/* Security Check */
		code = CheckAccess( file_no, BROWSE, c_mesg );
		if( code < 0 ) return(code);
	}
#endif

	/*-- request is read and start the file with the given key --*/

	request = BROWSE ;
	if((fd = start_file(start_rec, file_no, key_no, direction)) == ERROR) {
		if(dberror == FLNOTEXST) {
			crt_msg(file_no,key_no,EFL,c_mesg,start_rec);
			return(EFL);
		}
		code = ERROR;
		goto err_last;
	}

	if ( direction == FORWARD )
		code = isreads( fd , start_rec, (mode == BROWSE) ? 0 : 1) ;
	else
		code = isreadp(fd, start_rec, (mode == BROWSE) ? 0 : 1) ;

	if ( code == ERROR ) goto err_last ;
	if ( code == LOCKED || code == EFL ) {
		crt_msg(file_no,key_no,code,c_mesg,start_rec);
		dbexit(NODBERROR, code)
	}

#ifdef JRNL
	/* Write to rollback area */
	if ( mode != BROWSE && journaling) {
		code = writelog( start_rec, file_no, mode, key_no, ROLLBK_AREA);
		if(code < 0) goto err_last ;
	}
#endif
	if( mode != BROWSE && id_array[file_no].id_lock[0] == '0' )
		id_array[file_no].id_lock[0] = '2' ;	/* Rec level lock */

	return(0);

err_last :
	log_err ( file_no, start_rec, key_no, "READS", "get_next" ) ;
	crt_msg(file_no,key_no,ERROR,c_mesg,start_rec);
	return(code) ;
}

/*
*	getdatafd()
*
*	routine to return data file's fd of an Open SEQ/ISAM file.
*/

int
getdatafd(file_no)
int	file_no ;
{
	if ( file_no < 0 || file_no >= TOTAL_FILES || dbh_init == 0)
			dbexit(INVFILENO,ERROR)
	if ( id_array[file_no].id_fd < 0 )
		return(ERROR) ;

	return(id_array[file_no].id_data) ;
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

/*
*	form_key()
*
*	Forms the key string for a given ISAM file & key no.
*/

int
form_key ( rec, file_no, key_no, key_str)
char	*rec;		/* data record */
int	file_no;	/* File no */
int	key_no;		/* Key No. */
char	*key_str ;	/* Key will be returned in this */
{
	int	i, j, k, parts, *k_array ;
	short	s_value ;
	long	l_value ;
	float	f_value ;
	double	d_value ;

	if ( dbh_init == 0 )
		if ( init_dbh() < 0 ) {
#ifdef ENGLISH
			strcpy(key_str,"DBH Initialization ERROR") ;
#else
			strcpy(key_str,"ERREUR d'initialisation DBH") ;
#endif
			return(ERROR) ;
		}

	/* validate file_no & key_no */
	if(file_no < 0 || file_no >= TOTAL_FILES){
#ifdef ENGLISH
		sprintf(key_str,"Unknown File: %d",file_no);
#else
		sprintf(key_str,"Dossier inconnu: %d",file_no);
#endif
		return(ERROR);
	}
	if(id_array[file_no].id_f_type == SEQ) {
#ifdef ENGLISH
		sprintf(key_str, "Record#: %d",key_no) ;
#else
		sprintf(key_str, "#Fiche %d",key_no) ;
#endif
		return(NOERROR);
	}
	if(key_no < 0 || key_no >= id_array[file_no].tot_keys){
#ifdef ENGLISH
		sprintf(key_str,"Unknown Key:%d",key_no);
#else
		sprintf(key_str,"Cle inconnue:%d",key_no);
#endif
		return(ERROR);
	}
	/* Calculate 'i' to position the keysarray for a given key_no */
	/* For main key (key_no = 0) i will be 0 */

	for(j = 0, i = 0 ; j < key_no ; j++){
		/* No of parts of previous key * 4 + 1 */
		/* i += (id_array[file_no].keysarray[i] * 4) + 1; */
		i += (keysarray[id_array[file_no].keys_offset + i] * 4) + 1;
	}
	/***
	parts = id_array[file_no].keysarray[i];  * NO of parts in the key *
	***/
	parts = keysarray[id_array[file_no].keys_offset + i];

	/* Take each part of the key and form string */

	key_str[0] = '\0';

	i++;	/* Position at ist part */
	for ( j = 0 ; j < parts ; j++, i += 4) {
		/***
		k_array = &(id_array[file_no].keysarray[i]);
		***/
		k_array = &(keysarray[id_array[file_no].keys_offset+i]); 
		if(j)strcat(key_str,"-");

		switch( k_array[0] ) {	/* TYpe of part */

		case DATE :
		case LONG :
			for ( k = 0 ; k < k_array[1] ; k++ ){
			    if(k)strcat(key_str,"-");
			    scpy((char*)&l_value,
				(rec+k_array[2]+k*sizeof(long)),sizeof(long));
			    sprintf((key_str+strlen(key_str)),"%ld",l_value);
			}
			continue ;
		 case FLOAT :
			for ( k = 0 ; k < k_array[1] ; k++ ){
			    if(k)strcat(key_str,"-");
			    scpy((char*)&f_value,
				(rec+k_array[2]+k*sizeof(float)),sizeof(float));
			    sprintf((key_str+strlen(key_str)),"%f",f_value);
			}
			continue ;
		 case DOUBLE :
			for ( k = 0 ; k < k_array[1] ; k++ ){
			    if(k)strcat(key_str,"-");
			    scpy((char*)&d_value,
			      (rec+k_array[2]+k*sizeof(double)),sizeof(double));
			    sprintf((key_str+strlen(key_str)),"%lf",d_value);
			}
			continue ;
		 case SHORT :
			for ( k = 0 ; k < k_array[1] ; k++ ){
			    if(k)strcat(key_str,"-");
			    scpy((char*)&s_value,
			        (rec+k_array[2]+k*sizeof(short)),sizeof(short));
			    sprintf((key_str+strlen(key_str)),"%d",s_value);
			}
			continue ;
		 case CHAR :
		 	/* skip copying trailing blank characters. If all
			   blanks copy atleast one char */
		 	for( k = k_array[1] ; k > 1 ; k--)
		 		if((rec+k_array[2])[k - 1] != ' ')break;
			strncat(key_str,(rec+k_array[2]),k);
			continue ;
	  	 default :
#ifdef ENGLISH
			strcpy(key_str,"Illegal type...");
#else
			strcpy(key_str,"Genre illegal...");
#endif
			return(ERROR) ;
		}
	}
	return(0);
}

/*
*	crt_msg()
*
*	Routine to create an appropriate error message for the
*	given file number and err_code reurtned by DBH/ISAM..
*
*	Called By : Mostly to be called after get request . Can be called
*		After put request But since the record is written in log
*		file first and if error occurs then rollback is attempted
*		it is tough to Record correct error.
*/

static	int
crt_msg(file_no, key_no, err_code, c_mesg, rec)
int	file_no ;	/* DBH File# */
int	key_no;		/* Rec# / Key_no */
int	err_code ;	/* error code (iserror or code)   */	
char	*c_mesg ;	/* message string allocated by caller */
char	*rec;		/* rec in question */
{
	char	*cp_msg();

	if(file_no < 0 || file_no >= TOTAL_FILES || dbh_init == 0)
#ifdef ENGLISH
		strcpy(c_mesg,"System Error: crt_msg()  Unknown File Name");
#else
		strcpy(c_mesg,"Erreur du Systeme: crt_msg()  Nom de dossier inconnu");
#endif
	else {
		sprintf(c_mesg,"%-20.20s %-25.25s",
			id_array[file_no].fl_user_nm, cp_msg(err_code));
		if(id_array[file_no].id_f_type == ISAM)
#ifdef ENGLISH
			strcat(c_mesg,"KEY: ") ;
#else
			strcat(c_mesg,"CLE: ") ;
#endif
		/* Copying key to c_mesg from 52nd char. If the c_mesg is
		   not big enough to fit the key, form_key will goof */
		form_key(rec,file_no,key_no,c_mesg+strlen(c_mesg));
	}
	return(0);
}

/*
*	cp_msg()
*
*	Routine to create an apprepoiate error message for the
*	given Error code.
*/

/* This Message should not exceed 25 chars */
static	char	*msg[] = {

#ifdef ENGLISH
	"Record Read/Written..",		/* 0 - NOERROR */
	"Record Not Available..",		/* 1 - UNDEF */
	"Record Already Exists..",		/* 2 - DUPE */
	"Record Locked..",			/* 3 - LOCKED */
	"Severe Read/Write Error..",		/* 4 - ERROR */
	"End of File Encountered..",		/* 5 - EFL */
	"Undefined Error Code.."		/* 6 - Unknown err code */
#else
	"Fiche lue/ecrite..",    	    	/* 0 - NOERROR */
	"Fiche pas disponible..",		/* 1 - UNDEF */
	"Fiche existe deja..",	                /* 2 - DUPE */
	"Fiche verrouillee..",                  /* 3 - LOCKED */
	"Erreur de Lecture/ecriture grave..",   /* 4 - ERROR */
	"Fin du dossier rencontree..",          /* 5 - EFL */
	"Code d'erreur indefini.."		/* 6 - Unknown err code */
#endif
};

char	* cp_msg(err_code)
int	err_code ;			/* error code (iserror or code)   */	
{
	switch (err_code) {
	case NOERROR :
		return(msg[0]);
	case UNDEF :
		return(msg[1]);
	case DUPE :
		return(msg[2]);
	case LOCKED :
		return(msg[3]);
	case ERROR :
		return(msg[4]);
	case EFL :
		return(msg[5]);
	default :
		return(msg[6]);
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
#ifdef ENGLISH
			strcpy(e_mesg, "ERROR in Initializing DBH");
#else
			strcpy(e_mesg, "ERREUR en initialisant DBH");
#endif
			return(ERROR) ;
		}

	if ( from_file_no < 0 || from_file_no >= TOTAL_FILES ){
#ifdef ENGLISH
		sprintf(e_mesg, "ERROR: Invalid File Number %d", from_file_no);
#else
		sprintf(e_mesg, "ERREUR: Numero de dossier %d invalide", from_file_no);
#endif
		return(ERROR) ;
	}

	if ( to_file_no < 0 || to_file_no >= TOTAL_FILES ){
#ifdef ENGLISH
		sprintf(e_mesg, "ERROR: Invalid File Number %d", to_file_no);
#else
		sprintf(e_mesg, "ERREUR: Numero de dossier %d invalide", to_file_no);
#endif
		return(ERROR) ;
	}

	if( karraysize(to_file_no) < karraysize(from_file_no) ) {
		strcpy(e_mesg,
#ifdef ENGLISH
		"Temporary Keysarray size is exceeding the Allocated size") ;
#else
		"Grandeur du KEYSARRAY temporaire excede la grandeur allouee") ;
#endif
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
		karraysize(from_file_no) * sizeof(int) );

	/* Set the to_file file open modes to close status, previous scpy()
	   might have copied from_file's status */
	id_array[to_file_no].id_fd       = -1 ;	/* signal close */
	id_array[to_file_no].id_start    = -1 ;
	id_array[to_file_no].id_lock[0]  = '0' ;

	return(0) ;
}

/*
*	unlink_file()
*
*	Function to Unlink files for a given DBH file.
*/

int
unlink_file(file_no)
int	file_no ;
{
	char	file_nm[PATH_LEN] ;

	if ( dbh_init == 0 )
		if ( init_dbh() < 0 ) return(ERROR) ;

	if ( file_no < 0 || file_no >= TOTAL_FILES )
		dbexit(INVFILENO,ERROR)

	close_file(file_no) ;	/* If it is open */

	form_f_name(id_array[file_no].id_f_name, file_nm);
	unlink(file_nm) ;

	if(id_array[file_no].id_f_type == ISAM) {
		strcat ( file_nm, ".IX" ) ;
		unlink(file_nm) ;
	}

	/* create the file back again with 0 length */
	request = UPDATE ;
	init_file( file_no );

	return(0) ;
}

/*--------------------E-N-D---O-F---F-I-L-E----------------------------*/

