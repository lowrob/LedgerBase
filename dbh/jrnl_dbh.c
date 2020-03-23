/*
*	jrnl_dbh.c
*
*	commit and roll_back calls are defined in this file..
*/


#include <stdio.h>
#include <bfs_defs.h>

#ifdef	JRNL

#include <isnames.h>
#include <filein.h>
#include <journal.h>
#include <dberror.h>


#define	WAIT		1	/* wait till lock succeeds */
#define	FROM		0L	/* lock from file start onwards */
#define	SIZE		0 	/* for complete file lock */

#define	BLK_SZ	512

char	*commit_area = NULL ;	/* start of commit/roll areas*/
char	*rollbk_area = NULL ;

static	unsigned int	commit_size = BLK_SZ ;	/* commit area size */
static	unsigned int	rollbk_size = BLK_SZ ;	/* rollback area size */

static	Area_hdr	*rollarea_hdr ,		/* header for entire area */
			*commarea_hdr ;

static 	char 		*rec_ptr ;

int	journaling = 0 ;
int	wr_jrnl ;			/* Write Commit area to Journal */

extern	f_id	id_array[TOTAL_FILES] ;		/* Key definitions for files */
extern	int	*keysarray ;			/* keys array for all files */
extern	long	last_posn;

/*
*	Sets the area size to user specified size .. Not a must as default
*	size if picked up from include file .
*/


int
set_jrnl_sz(n) 
int	n ;
{
	commit_size = (unsigned)n ;
	rollbk_size = (unsigned)n ;
}

/*
*	Open the Journal file .. The date rollover should have created it
*	(as well as init.out). It should contain the first block as defined
*	in 'journal.h' file . We should be able to switch to a server mechanism
*	by starting a journalling deamon. Then it would be good to create fixed
*	length journal files for a day.
*/

static	int
open_journal(rundate,e_mesg)
long	rundate ;
char	*e_mesg ;
{
	char	jr_name[50] ;
	int	jrnl_fd ;
	long	get_date() ;

	/** Append Date to jounal file name and open/rceate it **/

	form_f_name(JOURNAL,jr_name) ; 
	
	sprintf(&jr_name[strlen(jr_name)],"%ld", rundate );
	
	if(access(jr_name, 0) < 0) {
		if((jrnl_fd = creat(jr_name, CRMODE)) < 0) {
#ifdef ENGLISH
			strcpy(e_mesg, "Journal File Creation Error");
#else
			strcpy(e_mesg, "Erreur de creation du dossier journal");
#endif
			dbexit(JROPENERR, ERROR) ;
		}
		close(jrnl_fd);
	}
	if ( (jrnl_fd=open(jr_name, RWMODE)) < 0 ) dbexit(JROPENERR, ERROR) 
	return(jrnl_fd) ;
}

/*
*	Allocates areas for commit and rollback record images ..
*	Initialises the area headers and first record header .
*	Assumes commit and rollback areas are to be equal .
*/

int
init_journal() 
{

	if ( commit_size < BLK_SZ ) commit_size = BLK_SZ ; 
	if ( rollbk_size < BLK_SZ ) rollbk_size = BLK_SZ ; 

	commit_area = malloc( (unsigned)(commit_size) ) ;
	rollbk_area = malloc( (unsigned)(rollbk_size) ) ;
	
	if ( rollbk_area == NULL || commit_area == NULL ) 
		dbexit(JRALLOCER, ERROR) 

	rollarea_hdr = (Area_hdr *)rollbk_area ;	/* Allocate areas */
	commarea_hdr = (Area_hdr *)commit_area ;

	get_tnum( commarea_hdr->termnm ) ;	/* Initialise Terminal# */
	get_tnum( rollarea_hdr->termnm ) ;

	strcpy( commarea_hdr->usrnm, User_Id); /* Initialise user name */
	strcpy( rollarea_hdr->usrnm, User_Id);
	strcpy( commarea_hdr->program, PROG_NAME); /* Initialise Program name */
	strcpy( rollarea_hdr->program, PROG_NAME);
	
	init_flags() ;		/* Initialize Commit & Rollback Flags */

	wr_jrnl = 1 ;		/* Enable Writing to Journal */

	return(0) ;
}

/*
*	Initialize Commit & Roll areas.
*/

static	int
init_flags()
{
	/* Initialize Commit & Rollback Areas */
	commarea_hdr->rec_count = 0 ;
	rollarea_hdr->rec_count = 0 ;

	commarea_hdr->size = 0 ;
	rollarea_hdr->size = 0 ;

	return(0) ;
}

/*
*	Free Commit & Rollback area
*/

int
free_journal()
{
	if(commit_area != NULL)
		free(commit_area) ;
	if(rollbk_area != NULL)
		free(rollbk_area) ;

	commit_area = NULL ;
	rollbk_area = NULL ;
	return(0);
}

/*
*	Write the record along with its header in either commit area or
*	Rollback area..
*	The file layout for commit area is.__
* 		<-Area_hdr-> { <-Rec_hdr + Record-> }	
*	for rollback area it is
*		<-Area_hdr-> { <-Rec_hdr + Record-> }
*/		

int
writelog(record, file_no, op_code, rec_no, destination)
char	*record ;
int	file_no ,		/* DBH assigned file no */
   	op_code ,		/* Add, Change, Delete etc */
	rec_no ,		/* Record# for SEQ Files */
   	destination ;		/* Commit or Rollback ? */
{
	unsigned char	h_byte, l_byte ;
	unsigned short	slen ;
	int	 	prev_op ;	/* previous op_code on same record */
	int		code, new_size ;

	if ( destination == COMMIT_AREA ) {
		if(op_code != ADD 
		   && SrchArea(record,file_no,rec_no,0,ROLLBK_AREA,0) != 0 ){
			dbexit(JRNOTINRL, ERROR)
		}

		if ( op_code == ADD && id_array[file_no].id_f_type == SEQ )  
			code = 0 ;
		else if((code = SrchJournal(record,file_no,rec_no,1)) == 0 ) {

			/* rec_ptr is global variable and set to cur. rec in
			   above called function */
			prev_op = *(rec_ptr-3) ;
	
			if ( op_code == ADD ) {
				if (prev_op == ADD || prev_op == UPDATE) 
					return(DUPE) ;
				if (prev_op == P_DEL) op_code = UPDATE ;
				/* else prev_op == NOOP, so retain ADD */
			}
			else if ( op_code == UPDATE) {
				if (prev_op == P_DEL || prev_op == NOOP)
					return(UNDEF) ;
				if (prev_op == ADD)
					op_code = ADD ;
				/* else prev_op == UPDATE so retain */
			}
			else { 		/* op_code got to be P_DEL */
			  	if (prev_op == P_DEL || prev_op == NOOP) 
					return(UNDEF) ;
			   	if (prev_op == ADD ) 
					op_code = NOOP ;
				/* else retain P_DEL as prev_op is UPDATE */
			}
	
			/*-- Replace the old op_code and record --*/
	
			*(rec_ptr-3) = op_code ; 
			scpy( rec_ptr, record, id_array[file_no].reclen) ;
	
			return(0) ;
		}
		if(code == ERROR) return(ERROR) ;
	}

	/*
	*   NOTE: For SEQ files, when op_code == ADD correct rec_no is not
	*	  known now. commit() will copy the Correct# to commit area.
	*/
	slen = rec_no ;
	h_byte = slen >> (sizeof(short) - 1) * 8 ;
/* Cathy Burns Feb 5/91 to make ansi c compatible */
	l_byte = (slen % 256) ;

/*	l_byte = (slen << (sizeof(short)-1)*8) >> (sizeof(short)-1)*8 ;  */
		
	if ( destination == COMMIT_AREA ) {
		new_size = sizeof(Area_hdr) + commarea_hdr->size +
				id_array[file_no].reclen + 4 ;
		if (re_alloc(COMMIT_AREA, new_size) < 0) return(ERROR) ;

		rec_ptr  = commit_area + sizeof(Area_hdr) + commarea_hdr->size ;
		*rec_ptr++  = file_no ;
		*rec_ptr++  = op_code ; 
		*rec_ptr++  = h_byte ;
		*rec_ptr++  = l_byte ;
		scpy( rec_ptr, record, id_array[file_no].reclen) ;

		commarea_hdr->rec_count++ ;
		commarea_hdr->size += id_array[file_no].reclen + 4 ;
	}
	else {
		new_size = sizeof(Area_hdr) + rollarea_hdr->size +
				id_array[file_no].reclen + 4 ;
		if (re_alloc(ROLLBK_AREA, new_size) < 0) return(ERROR) ;

		rec_ptr  = rollbk_area + sizeof(Area_hdr) + rollarea_hdr->size ;
		*rec_ptr++  = file_no ;
		*rec_ptr++  = op_code ;
		*rec_ptr++  = h_byte  ;
		*rec_ptr++  = l_byte  ;
		scpy(rec_ptr, record, id_array[file_no].reclen) ;

		rollarea_hdr->rec_count++ ;
		rollarea_hdr->size += id_array[file_no].reclen + 4 ;
	}
	return(0) ;
}

/*
*	Commit to DBH files .. Read commit area records and write to 
*	DBH file. If any record returns error then undo all updates 
*	unsing rollback area and delete all records added till that
*	point.
*/

int
commit(e_mesg)
char	*e_mesg ;
{
	char 	*tmp ;
	int	code ;
	char	*record ;
	int	i, file_no, op_code , rec_no ;
	unsigned char	h_byte, l_byte ;
	unsigned short	slen ;
	long	rundate ;

	if(journaling == 0) {
		release_dbh() ;
		return(0) ;
	}

	/*
	*	Disable writing to commit/rollback areas.. write will be done to
	*	Data files now ..
	*/

	journaling = 0 ;	/* Disable journaling */
	
	/* get_date() is being called before commit(), to store the current
	   date, to pass it later to writejournal(). Otherwise End-of-day
	   process's, rolling date commit, will write journal to next date */

	if(wr_jrnl)
		rundate = get_date() ;

	tmp = commit_area + sizeof(Area_hdr) ;
	code = 0 ;
	for(i = 0 ; i < commarea_hdr->rec_count ; i++) {
		file_no = *tmp++ ;
		op_code = *tmp++ ;
		h_byte =  *tmp++ ;
		l_byte =  *tmp++ ;
		record  = tmp ;

		tmp += id_array[file_no].reclen ;

		if ( op_code == NOOP ) continue ;

		slen = (h_byte << ((sizeof(short) -1) * 8)) | l_byte ;

		rec_no = slen   ;	/* Will be replaced by actual NO */

		if(id_array[file_no].id_f_type == SEQ) {
			code = put_rec(record, op_code, file_no, rec_no,e_mesg);

			/* Programs don't know the number of the record, when
			   they ADD a record to SEQ files. Correct number will
			   be known only after put_rec() returns the number */
			if(code >= 0 && op_code == ADD) {
				/* put correct record number in commit area */
				slen = code ;
				*(record-2) = slen >> (sizeof(short) - 1) * 8 ;
			/* Cathy Burns Feb 5/91 to make ansi c compatible */
				*(record-1) = (slen % 256) ;

/*				*(record-1) = (slen << (sizeof(short)-1)*8) >>
							(sizeof(short)-1)*8 ; 
*/
			}
		}
		else	
			code = put_isrec( record, op_code, file_no, e_mesg) ;
		
		if ( code < 0 ) {
			 if ( i > 0 ) {
			 	if(UndoAdds(i, e_mesg) < 0)
#ifdef ENGLISH
				    strcpy(e_mesg,"ERROR: Undo Adds Failed");
#else
				    strcpy(e_mesg,"ERREUR: Defaire les ajouts manques");
#endif
			 	if(UndoUpdates(e_mesg) < 0)
#ifdef ENGLISH
				    strcpy(e_mesg,"ERROR: Undo Updates Failed");
#else
				    strcpy(e_mesg,"ERREUR: Defaire les mises a jour manquees");
#endif
			}
			/* Copy as -ve rec_count for aborted commit */
			commarea_hdr->rec_count = -commarea_hdr->rec_count ; 
			break ;
		} 
	}

	release_dbh() ;

	/*
	*  Next set of put_isrec calls will again use the commit/rollback areas.
	*/

	journaling = 1 ;	/* Enable journalling */

	if (wr_jrnl ) 
		writejournl(rundate, e_mesg) ;

	init_flags() ;
	
	if(code < 0) return(code) ;
	return(0) ;
}

static	int				
writejournl(rundate,e_mesg)
long	rundate ;
char	*e_mesg ;
{
	int	jrnl_fd ;
	int	size ;

	if ((jrnl_fd = open_journal(rundate,e_mesg)) < 0 ) return(ERROR) ;

	/* Lock the entire file */

	if(e_lock(jrnl_fd, WRLOCK, WAIT, FROM, SIZE) < 0)
		dbexit(JRLOCKERR, ERROR)

	commarea_hdr->c_time = get_time() ;

	size = sizeof(Area_hdr) + commarea_hdr->size ;
	lseek(jrnl_fd, 0L, 2) ;
	if (write(jrnl_fd, commit_area, size) < size)
		dbexit(JRWRITERR, ERROR) ; 
	/* 
	*  Close journal.. Will remove open and close if journal server exists .
	*/
	close(jrnl_fd) ;
	return(0) ;
}


/*
*	Undo all updates using old images from rollback area ..
*/

static	int
UndoUpdates(e_mesg)
char	*e_mesg ;
{
	char	*tmp ;
	int	code ;
	char	*record ;
	int	i ;
	int	file_no, 
		op_code ,
		rec_no ;
	unsigned char	h_byte, l_byte ;
	unsigned short	slen ;

	tmp = rollbk_area + sizeof(Area_hdr) ;

	for(i=0; i < rollarea_hdr->rec_count ; i++) {
		
		file_no = *tmp++ ;
		op_code = *tmp++ ;
		h_byte =  *tmp++ ;
		l_byte =  *tmp++ ;
		record  = tmp ;
		slen = (h_byte << ((sizeof(short) -1) * 8)) | l_byte ;

		rec_no = slen   ;	/* Will be replaced by actual NO */

		if(id_array[file_no].id_f_type == SEQ)
			code = put_rec(record, op_code, file_no, rec_no,e_mesg);
		else	
			code = put_isrec(record, op_code, file_no, e_mesg) ;
		
		if ( code < 0 ) return(code) ;

		tmp += id_array[file_no].reclen ;
	}

	return(0) ;
}
				
/*
*	If commit fails then Undoadds is called to delete/add the records
*	which might have been added/deleted by commit . This is done as rollback
*	area does not contain the records to be added..
*/

static	int
UndoAdds(count, e_mesg)
int	count ;
char	*e_mesg ;
{
	char	*tmp ;
	int	code ;
	char	*record ;
	int	i ;
	long	size, lseek() ;
	int	file_no, 
		op_code ,
		rec_no ;
	unsigned char	h_byte, l_byte ;
	unsigned short	slen ;

	tmp      = commit_area + sizeof(Area_hdr) ;

	for(i=0; i < count ; i++) {
		file_no = *tmp++ ;
		op_code = *tmp++ ;
		h_byte =  *tmp++ ;
		l_byte =  *tmp++ ;
		record  = tmp ;
		slen = (h_byte << ((sizeof(short) -1) * 8)) | l_byte ;

		rec_no = slen   ;	/* Will be replaced by actual NO */

		if ( op_code == ADD || op_code == P_DEL) {

		    if ( op_code == ADD) 
			op_code = P_DEL ;
		    else
			op_code = ADD ;

		    if(id_array[file_no].id_f_type == SEQ) {
			if ( op_code == ADD) 	/*Reversed in above statement*/
				code = put_rec(record, op_code, file_no, rec_no,e_mesg);
			else{ 		/* Change the file size */
				size = (rec_no - 1) * RECLEN(file_no) ;
				/* If size is already changed ignore */
#ifndef	UNIX		/* Not a 68000 Family Unix */
				if(size < lseek(id_array[file_no].id_fd, 0L,2))
					chsize(id_array[file_no].id_fd,size) ;
#endif
			    }
		    }
		    else {	/* ISAM File */
			code = put_isrec( record, op_code, file_no, e_mesg) ;
			if ( code < 0 ) return(code) ;
		    }
		}

		tmp += id_array[file_no].reclen  ;
	}

	return(0) ;
}

int
roll_back(e_mesg) 
char	*e_mesg;
{
	if(journaling == 0) {
		release_dbh() ;		/* Unlock Locked Records */
		return(0) ;
	}

	init_flags() ;		/* Initialize Commit & Roll flags & area */

	release_dbh() ;		/* Unlock Locked Records */

	return(0);
}

/*****
**	In case Journal Overflows, Re-allocate COMMIT/ROLL-BACK Sizes
*****/

int
re_alloc(area, new_size)
int	area ;			/* Commit area or Rollback area */
int	new_size ;
{
	int	old_size ;
	char	*temp_area ;

	if(area == COMMIT_AREA) {		/* Commit Area */
		if(new_size <= commit_size) return(0) ;

		/* Allocate New size in multiple of BLK_SZ blocks with last
		   block empty or partially filled */
		commit_size = new_size + (BLK_SZ - (new_size % BLK_SZ)) ;

		temp_area   = malloc( (unsigned)(commit_size) ) ;
		if ( temp_area == NULL ) dbexit(JRALLOCER, ERROR) 

		old_size = sizeof(Area_hdr) + commarea_hdr->size ;
		scpy(temp_area, commit_area, old_size) ;
		free( commit_area ) ;
		commit_area = temp_area ;
		commarea_hdr = (Area_hdr *)commit_area ;
	}
	else {
		if(new_size <= rollbk_size) return(0) ;

		/* Allocate New size in multiple of BLK_SZ blocks with last
		   block empty or partially filled */
		rollbk_size = new_size + (BLK_SZ - (new_size % BLK_SZ)) ;

		temp_area   = malloc( (unsigned)(rollbk_size) ) ;
		if ( temp_area == NULL ) dbexit(JRALLOCER, ERROR) 

		old_size = sizeof(Area_hdr) + rollarea_hdr->size ;
		scpy(temp_area, rollbk_area, old_size) ;
		free( rollbk_area ) ;
		rollbk_area = temp_area ;
		rollarea_hdr = (Area_hdr *)rollbk_area ;
	}

	return(0) ;
}

/*--------------------------------------------------------------------*/

/*
*	Search Commit Area ( rollback area also if Get type) for the record ..!
*/

int
SrchJournal(in_rec, in_flno, in_keyno, srch_type )
char	*in_rec ;
int	in_flno ;
int	in_keyno  ;	/* in_recno in case of SEQ files */ 
int	srch_type ;	/* 0 - Get Type. Search in both Commit and Roll areas.
			       If record found copy to in_rec.Skip NOOP records.
			       (Called form DBH get_rec() & get_isrec()
			   1 - Put Type. Search in only Commit area. If record
			       found return. Called from writelog() */
{
	int	code ;
	int	i, file_no, op_code , rec_no ;
	unsigned char	h_byte, l_byte ;
	unsigned short	slen ;
	Area_hdr	*curr_hdr ;
	char 		*curr_ptr ;	/* Current Area in Srch */

	if(journaling == 0  ) return(UNDEF) ;

	curr_ptr = commit_area ;	/* Either Srch type, 1st serach in
					   commit area */

Big_loop :	/**- Check Commit area first then Check rollback area --**/

	curr_hdr = (Area_hdr *)curr_ptr ;
	rec_ptr  = curr_ptr  + sizeof(Area_hdr) ;

	for(i = 0 ; i < curr_hdr->rec_count ;
			i++, rec_ptr += id_array[file_no].reclen) {
		file_no = *rec_ptr++ ;
		op_code = *rec_ptr++ ;
		h_byte  = *rec_ptr++ ;
		l_byte  = *rec_ptr++ ;

		/* For Get Srch Skip the NOOP records and continue search */
		if(srch_type == 0 && op_code == NOOP) continue ;

		/*** Changed by amar on 25-oct-89 as per kavi's suggestion
		if ( file_no != in_flno || rec_no != in_keyno ) continue ;
		***/
		if ( file_no != in_flno ) continue ;

		if(id_array[file_no].id_f_type == SEQ) {
			slen = (h_byte << ((sizeof(short) -1) * 8)) | l_byte ;
			rec_no  = slen   ;

			if( rec_no != in_keyno ) continue ;
			break ;
		}

		/* For ISAM files compare Keys */
		code = compkeys(in_rec, rec_ptr, file_no, in_keyno) ;
		if ( code == 0 ) break ;
		if ( code == ERROR ) return(code) ;	/* Abnormal */
	}

	if (curr_hdr->rec_count <= 0 || i == curr_hdr->rec_count ) {
		if ( srch_type == 0 ) {
			if ( curr_ptr == rollbk_area )  return(UNDEF) ;
			curr_ptr = rollbk_area ; 
			goto	Big_loop ;
		}
		return(UNDEF) ;
	}
	
	/* If the srch is Get Type, for non deleted records copy the record
	   to in_rec and return. For Deleted records return same */
	if ( srch_type == 0 ) {
		if(op_code == P_DEL) return(op_code) ;
		scpy( in_rec, rec_ptr, id_array[file_no].reclen) ;
	}

	return(0) ;
}

/*
*
*	Compares keys from two records . Returns 0 if matched ..
*
*/

int
compkeys(record, in_record, file_no, key_no) 
char	*record ,
	*in_record ;
int	file_no ,
	key_no ;
{
	int	i, j, type, len, pos, parts, order ;
	char	*ptr1, *ptr2 ; 
	long	l_ptr, l1_ptr ;
	float	f_ptr , f1_ptr;
	short	s_ptr, s1_ptr ;
	double	d_ptr , d1_ptr;
	int	*partsaray ;

	partsaray  = &keysarray[ id_array[file_no].keys_offset ] ;

	/**-- 	Position Partsarray to Appropriate Key_no --**/

	for(i=0; i<key_no; i++) 
		partsaray += ( *(partsaray) * 4 + 1 ) ;

	parts = *(partsaray++) ;

	for ( j = 0 ; j < parts ; j++) {
		type = *(partsaray++) ;
		len = *(partsaray++) ;
		pos = *(partsaray++) ;
		order = *(partsaray++);		/* correct increment */

		ptr1 = in_record + pos ;
		ptr2 = record + pos ;

		if ( order == ASCND)
			order = 1 ;
		else if ( order == DESCND)
			order = -1 ;

		switch ( type ) {
		case  DATE  :
		case  LONG  :
			for ( i = 0 ; i < len ; i++){
				scpy( ( char *)&l_ptr ,ptr1 , sizeof(long) );
				scpy( ( char *)&l1_ptr ,ptr2 , sizeof(long) );
				if ( l_ptr < l1_ptr ) return(1*order) ;
				if ( l_ptr > l1_ptr ) return(-1*order) ;
				ptr1 += sizeof(long);
				ptr2 += sizeof(long);
			}
			continue ;
		case  FLOAT :       
			for ( i = 0 ; i < len; i++){
				scpy( (char *)&f_ptr ,ptr1, sizeof(float) ) ;
				scpy( (char *)&f1_ptr ,ptr2, sizeof(float) ) ;
				if ( f_ptr < f1_ptr ) return(1*order) ;
				if ( f_ptr > f1_ptr ) return(-1*order) ;
				ptr1 += sizeof(float);
				ptr2 += sizeof(float);
			}
			continue ;
		case  DOUBLE :       
			for ( i = 0 ; i < len; i++){
				scpy( (char *)&d_ptr ,ptr1, sizeof(double) ) ;
				scpy( (char *)&d1_ptr ,ptr2, sizeof(double) ) ;
				if ( d_ptr < d1_ptr ) return(1*order) ;
				if ( d_ptr > d1_ptr ) return(-1*order) ;
				ptr1 += sizeof(double);
				ptr2 += sizeof(double);
			}
			continue ;
		case  SHORT :       
			for ( i = 0 ; i < len; i++){
				scpy( (char *)&s_ptr ,ptr1, sizeof(short) ) ;
				scpy( (char *)&s1_ptr ,ptr2, sizeof(short) ) ;
				if ( s_ptr < s1_ptr ) return(1*order) ;
				if ( s_ptr > s1_ptr ) return(-1*order) ;
				ptr1 += sizeof(short);
				ptr2 += sizeof(short);
			}
			continue ;
		case CHAR :           
			i = strncmp(ptr1, ptr2, len) ;
			if ( i < 0 ) return(1*order) ;
			if ( i > 0 ) return(-1*order) ;
			continue ;
		default :
			dbexit(JRKTYPERR, ERROR) 
		}
	}
	return(0) ;
}
/*--------------------------------------------------------------------*/

/*
*	Search Commit/Rollback Area for the record ..!
*/
static	int
SrchArea(in_rec, in_flno, in_keyno, srch_type, area, copyrecord )
char	*in_rec ;
int	in_flno ;
int	in_keyno  ;	/* in_recno in case of SEQ files */ 
int	srch_type ;	/* 0 - Get Type.
			   1 - Put Type. Search in specified area.
			       Skip NOOP records.
			       (Called form DBH put_rec()
int	area;		/* ROLLBK_AREA or COMMIT_AREA */
int	copyrecord;	/* copy or not the record from area, if found */
{
	int	code ;
	int	i, file_no, op_code , rec_no ;
	unsigned char	h_byte, l_byte ;
	unsigned short	slen ;
	Area_hdr	*curr_hdr ;
	char 		*curr_ptr ;	/* Current Area in Srch */

	if(journaling == 0  ) return(UNDEF) ;

	if( area!=ROLLBK_AREA  &&  area!=COMMIT_AREA )
		return(UNDEF);

	if( area == ROLLBK_AREA )
		curr_ptr = rollbk_area ;	/* pointer to rollback area */
	else
		curr_ptr = commit_area ;	/* pointer to commit area */

	curr_hdr = (Area_hdr *)curr_ptr ;
	rec_ptr  = curr_ptr  + sizeof(Area_hdr) ;

	for(i = 0 ; i < curr_hdr->rec_count ;
			i++, rec_ptr += id_array[file_no].reclen) {
		file_no = *rec_ptr++ ;
		op_code = *rec_ptr++ ;
		h_byte  = *rec_ptr++ ;
		l_byte  = *rec_ptr++ ;

		/* For Get Srch Skip the NOOP records and continue search */
		if(srch_type == 0 && op_code == NOOP) continue ;

		if ( file_no != in_flno ) continue ;

		if(id_array[file_no].id_f_type == SEQ) {
			slen = (h_byte << ((sizeof(short) -1) * 8)) | l_byte ;
			rec_no  = slen   ;

			if( rec_no != in_keyno ) continue ;
			break ;
		}

		/* For ISAM files compare Keys */
		code = compkeys(in_rec, rec_ptr, file_no, in_keyno) ;
		if ( code == 0 ) break ;
		if ( code == ERROR ) return(code) ;	/* Abnormal */
	}

	if (curr_hdr->rec_count <= 0 || i == curr_hdr->rec_count )
		return(UNDEF) ;
	if( copyrecord && srch_type==0 ){	
		if(op_code==P_DEL)	return(op_code);
		scpy( in_rec, rec_ptr, id_array[file_no].reclen) ;
	}

	return(0) ;
}
/*
*
*	This function searches the rollback area for a given dbh file record.
*	This is to detect if it is previously locked.  If so, the record is
* 	locked again. 
*	This is required to solve the help windows' problem of unlocking.
*	Calling file: hlp_win.c
*
*/
SettleLock( recptr, file_no, e_mesg )
char	*recptr;	/* pointer to record area */
int	file_no;	/* dbh file number */
char	*e_mesg;
{
	int	code = 0 ;
	long	posn;

	/* For Random read calls locked record is fetched from journal area,
	   which doesn't affect the lock. In such case just return */
	if(flg_start(file_no) < 0) return(NOERROR) ;	/* In Random mode */

	/* BROWSE doesn't affect locks on SEQ type files */
	if(getfiletype( file_no ) == SEQ) return(NOERROR) ;

	posn = last_posn;	/* ISAM sets the last_posn */

	code = SrchArea( recptr, file_no, 0, 0, ROLLBK_AREA, 0 );
	if( code==0 ){	/* found in rollback area */
		code = e_lock( id_array[file_no].id_data, RDLOCK,
				0, posn, id_array[file_no].reclen );
		if( code<0 ){
			sprintf(e_mesg,
#ifdef ENGLISH
				"Error in locking file %s record",
				id_array[file_no].fl_user_nm);
#else
				"Erreur en verrouillant la fiche %s du dossie",
				id_array[file_no].fl_user_nm);
#endif
			return(ERROR);
		}
	}
	return(code);
}
#else
/*-------------------------------------------------------------*/

commit(c_mesg)
char	*c_mesg;
{
	/* In multi user environment this function writes all
	   recs to the file & unlocks the recs. Now this is a
	   fake call */

	release_dbh() ;
	return(0) ;
}
/*------------------------------------------------------------*/
roll_back(c_mesg)
char	*c_mesg;
{
	/* In multi user environment this function abort all 
	   writes & unlocks the recs. Now this is a fake call */

	release_dbh() ;
	return(0) ;
}

#endif

/*-------------------------------End Of File-----------------------------*/
