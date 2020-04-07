/*
*	bld_indx.c
*
*	Creates An index file over an existing file. The temporary files in
*	id_array are suppose to be destination files ..
*
*	NOTE:
*	In the ISAM DBH validation of records is done here. For ORACLE DBH
*	validation should be done at the time of reading.
*/

#include <stdio.h>
#include <bfs_defs.h>
#include <isnames.h>
#include <filein.h>
#include <dberror.h>

extern	f_id	id_array[] ;	/* DBH File IO table */
extern	int	*keysarray ;	/* keysarray allocated in init_dbh */
#ifndef	ORACLE
extern	int	data_fd ;
#endif

/*** Function and prototype declares ***/
static int 	WriteIndex(int, int, int, int (*val_func)(), char *);

int
CrtTmpIndx(source, dest, ixkeysarray, ix_file, val_func,  e_mesg) 
int 	source ;		/* Source File Number (DBH Number) */
int	dest ;			/* Destination file Number (TEMP1, TEMP2 etc) */
int	*ixkeysarray ;		/* keys array ( parts + {T,L,P,O}* ) */
char	*ix_file ;		/* Temporay Index File Name */
int	(*val_func)();		/* Ptr to user's fn() which validates a rec */
char	*e_mesg ;

{
#ifndef	ORACLE
	int	fd ;
	char	source_flnm[50], source_dbnm[50] ;
	char	indx_flnm[50] ;
	int	retcode, reclen, data_type, tot_size ;
#else
	int	retcode, tot_size ;
#endif

	/*
	*	Security Check
	*/
	retcode = CheckAccess(source, BROWSE, e_mesg) ;
	if(retcode < 0) return(retcode) ;

	/* Check whether Keysarray size is exceeding the allocated size in
	   DBH */
	if ( (tot_size = ( *(ixkeysarray) * 4 +1) * sizeof(int)) > 
			karraysize(dest) * sizeof(int) ) dbexit(TMPKEYERR,ERROR) 
	close_file(dest) ;	/* Close the file(if opened), before copying
				   new file */

#ifndef	ORACLE
	/*
	*	Delete the temporary index file, if exististing.
	*/

	form_f_name(ix_file, indx_flnm);
	if ( access(indx_flnm,0) >= 0 ) 
		unlink(indx_flnm) ;
	strcat(indx_flnm, ".IX") ;
	if ( access(indx_flnm,0) >= 0 ) 
		unlink(indx_flnm) ;

	/* Form the file names again without '.IX' */
	
	form_f_name(ix_file, indx_flnm);
	if  ( getflnm( source, source_flnm) < 0 ) return(retcode) ;
	form_f_name(source_flnm, source_dbnm) ;

	if ( access(source_dbnm,0) < 0 ) {
#ifdef ENGLISH
	    sprintf(e_mesg,"ERROR: %s File access Error.. errno: %d",
			source_dbnm, errno );
	    return(ERROR) ;
#else
	    sprintf(e_mesg,"ERREUR: %s Erreur d'acces au dossier.. errno: %d",
			source_dbnm, errno );
	    return(ERROR) ;
#endif
	}

	reclen =  id_array[source].reclen ;

	fd = isbuild(source_dbnm, RWR, ixkeysarray, indx_flnm, reclen) ;

	if(fd == ERROR) {
#ifdef ENGLISH
	    sprintf(e_mesg,"ERROR: %s Creation Error.. Iserror: %d errno: %d",
			indx_flnm, iserror, errno );
	    return(ERROR) ;
#else
	    sprintf(e_mesg,"ERREUR: %s Erreur de creation.. Iserror: %d errno: %d",
			indx_flnm, iserror, errno );
	    return(ERROR) ;
#endif
	}


	if ( (retcode = WriteIndex(fd, data_fd, reclen, val_func, e_mesg)) < 0 )
		return(retcode) ;

	isclose(fd) ;

	strcpy( id_array[dest].id_f_name, ix_file) ;
#else
	strcpy( id_array[dest].id_f_name, id_array[source].id_f_name) ;
#endif

	scpy((char *)&keysarray[id_array[dest].keys_offset],
					(char *)ixkeysarray, tot_size) ;	
	
	id_array[dest].id_f_type = ISAM ;
	id_array[dest].id_io_mode = R ;
	id_array[dest].reclen = id_array[source].reclen ;
	id_array[dest].tot_keys = 1 ;

	return(NOERROR) ;
}
#ifndef	ORACLE
/*---------------------------------------------------------------------*/

static	int
WriteIndex(fd, fd_data, reclen, val_func, e_mesg)
int	fd ;		/* ISAM file fd */
int	fd_data ;	/* Data file fd */
int	reclen ;	/* Record Length */
int	(*val_func)();	/* Ptr to user's function which validates a record */
char	*e_mesg ;
{
	char	source_flnm[50] ;
	char	*ix_buff ;
	int	pos, retsize,
		IX_BSIZE = 0,
		retcode ;
	long	readpos ;

	/*
	* read the data file in bolcks and write records without writing to 
	* data file ..
	*/

 	IX_BSIZE = (reclen + STATUS_LEN) * 50 ;

	ix_buff = (char *)malloc((unsigned)IX_BSIZE) ;
	if(ix_buff == NULL) {
#ifdef ENGLISH
		strcpy(e_mesg,"MEMORY ALLOCATION ERROR in WriteIndex()...");
		return(ERROR)  ;
#else
		strcpy(e_mesg,"ERREUR D'ALLOCATION A LA MEMOIRE dans le WriteIndex()...");
		return(ERROR)  ;
#endif
	}

	readpos = 0L ;

	while(1) {

		/* Lseek to last position again. ixbuild() changes the
		  file position */
 		readpos = lseek(fd_data, readpos, 0) ;

		if((retsize = read(fd_data, ix_buff, (unsigned)IX_BSIZE))
							<= 0) break ;

		if(retsize < (reclen + STATUS_LEN)) break ;

		for(pos = 0 ; pos < retsize ; pos += reclen ){

#if	STATUS_LEN == 1
			if(ix_buff[pos++] == SET_DEL) {
				readpos += reclen + STATUS_LEN ;
				continue ;
			}
#endif
			/* Call user's function to check if this rec is valid */
			if( val_func != NULL )
			    if( (*val_func)(ix_buff+pos) <0 ){
				readpos += reclen + STATUS_LEN;
				continue;
			    }

#ifdef	FIXED_RECLEN
			retcode = ixbuild(fd, (ix_buff+pos), readpos) ;
#else
			retcode = ixbuild(fd, (ix_buff+pos), readpos, reclen) ;
#endif
			if(retcode == ERROR) {
#ifdef ENGLISH
				sprintf(e_mesg,
				"INDEX WRITING ERROR... Iserror: %d Errno: %d",
				iserror, errno);
				return (retcode) ;
#else
				sprintf(e_mesg,
				"ERREUR D'INSCRIPTION A L'INDEX... Iserror: %d Errno: %d",
				iserror, errno);
				return (retcode) ;
#endif
			}

			readpos += reclen + STATUS_LEN ;

		} /* for() */

	} /* While() */

	free(ix_buff) ;
	return(0) ;
}
#endif
/*---------------------------------------------------------------------*/

