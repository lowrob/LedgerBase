/*-----------------------------------------------------------------------
Source Name: bdgt_tit.c
System     : Budgetary Financial system.
Module     : General Ledger system.
Created  On: 19th Jul 89.
Created  By: T AMARENDRA.

COBOL Sources(s): gl117 & Sort scripts from gl145 , tri-1, tri-2, tri-3,
		  tri-4, tri-5 & tri-6 Shells.

DESCRIPTION:
	Function to Creat Title & Data temporay Index files for the
	Budget Reports Program.

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

------------------------------------------------------------------------*/

#include <stdio.h>
#include <bdgt_rep.h>
#include <filein.h>
#include <isnames.h>
#ifdef	ORACLE
#include <fld_defs.h>
#include <incl_ora.h>
#endif

/*	Temporary File Names */

#define	GLTITLES	"ttitl"
#define	GLBUDGET	"tbdgt"
#define	GLSORT		"tsort"

static	short	BdgtSort = 0 ;			/* Yes if BdgtSort is DONE */
static	short	OthSorts[SUM_SORTS] =
				{0,0,0,0,0,0} ;	/* Yes whenever Sort is Done */

short	TitleSort = 0 ;				/* Yes if Title sort is Done */

short	SortKeys[SUM_SORTS][3] = {		/* Sorts are done on these user
						   keys */

		{ 1, 4, 0 } ,		/* Sort 1 */
		{ 4, 6, 0 } ,		/* Sort 2 */
		{ 2, 4, 7 } ,		/* Sort 3 */
		/**	2, 7 & 0 changed to 2, 4 & 7
		{ 2, 7, 0 } ,		Sort 3
		**/
		{ 4, 5, 0 } ,		/* Sort 4 */
		/** 6, 0, & 0 changed to 6, 7 & 0 (To get the details instead of
		   totals
		{ 6, 0, 0 } ,		Sort 5
		***/
		{ 6, 7, 0 } ,		/* Sort 5 */
		{ 3, 4, 0 }		/* Sort 6 */
} ;

/* Keys array for ISAM Index. Size is (No of Parts fld + (No of Parts * 4)).
   4 is for (Type,Length,Pos,order) of each key */

static	int	Titlearay[1+(USER_KEYS*4)] ;	/* Title Sort Keysarray. User
						   given 7 keys */
static	int	Bdgtaray[1+(8*4)] ;	/* Budget Rep sort Keysarray. Fund +
					   Sect + Admis + User K4 + User K5 +
					   User K6 + RecCod + Acnt# */
static	int	Otharay[SUM_SORTS][1+(8*4)] ;
					/* 6 sorts Keysarray. Fund + Sect +
					   Admis + 3 User keys(MAX) + RecCod +
					   Acnt# */

extern	int	data_fd ;
extern	f_id	id_array[] ;
extern	int	*keysarray ;
#ifdef	ORACLE
extern	Tbl_struct	*tbl_lst[] ;
#ifdef	DEBUG
extern	FILE	*sqlfp ;
#endif
#endif

/*--------------------------------------------------------------*/
/* Creat Title & temporay index (for budget report / sorts) files */
CreatTitles()
{
	int     option;
	char	tnum[5];
	char	ix_file[25] ;

	STRCPY(ix_file,GLTITLES) ;
	get_tnum(tnum) ;
	strcat(ix_file,tnum);    /* Append Terminal# to make file unique */

	if(TitleSort == 0) {	/* Title file not created */

		RemoveFiles() ;		/* First Time Remove existing Files */

		ret_cd = TitleFile(ix_file) ;
		if(ret_cd != NOERROR) return(ret_cd) ;

		if(ChkKarraysize(TMPINDX_1, Titlearay) < 0) return(ERROR) ;

		TitleSort = 1 ;
	}

	/* Link the Titles File to DBH Temporary File 1 */

	STRCPY( id_array[TMPINDX_1].id_f_name, ix_file) ;

	scpy((char*)&keysarray[id_array[TMPINDX_1].keys_offset], 
		(char*)Titlearay, (*(Titlearay) * 4 + 1) * sizeof(int) );

#ifdef	ORACLE
	id_array[TMPINDX_1].id_f_type = ISAM ;
	id_array[TMPINDX_1].id_io_mode = RWR ;
	id_array[TMPINDX_1].reclen = id_array[GLMAST].reclen ;
	id_array[TMPINDX_1].tot_keys = 1 ;
#endif
	close_file(TMPINDX_2) ;	/* Before Moving new file, make sure file is
				   closed */
	option = atoi(rp_sth.s_option);
	if(option <= 5) {		/* Budget Reports */
		STRCPY(ix_file,GLBUDGET) ;
		get_tnum(tnum) ;
		strcat(ix_file,tnum);		/* Append Terminal# to make file
						   unique */

		if(BdgtSort == 0) {	/* Budget file not created */
			ret_cd = CreatIxFile(ix_file, Bdgtaray) ;
			if(ret_cd != NOERROR) return(ret_cd) ;

			if(ChkKarraysize(TMPINDX_2, Bdgtaray) < 0)
				return(ERROR) ;

			BdgtSort = 1 ;
		}

#ifndef	ORACLE
		/* Link the Budget Reports SEQDATA File to DBH Temporary
		   File 2 */
		STRCPY( id_array[TMPINDX_2].id_f_name, ix_file) ;
#endif

		scpy((char*)&keysarray[id_array[TMPINDX_2].keys_offset], 
			(char*)Bdgtaray, (*(Bdgtaray) * 4 + 1) * sizeof(int) );
	}
	else {		/* Sort 1 to 6 required reports */
		sprintf(ix_file,"%s%d",GLSORT,rp_sth.s_sort_no) ;
		get_tnum(tnum) ;
		strcat(ix_file,tnum);		/* Append Terminal# to make file
						   unique */

		if(OthSorts[rp_sth.s_sort_no - 1] == 0) {	/* Sort file
								not created */
			ret_cd = CreatIxFile(ix_file,
				Otharay[rp_sth.s_sort_no - 1]) ;
			if(ret_cd != NOERROR) return(ret_cd) ;

			if(ChkKarraysize(TMPINDX_2,
			    Otharay[rp_sth.s_sort_no - 1]) < 0) return(ERROR) ;

			OthSorts[rp_sth.s_sort_no - 1] = 1; 
		}

#ifndef	ORACLE
		/* Link the Sort Reports SEQDATA File to DBH Temporary
		   File 2 */
		STRCPY( id_array[TMPINDX_2].id_f_name, ix_file) ;
#endif

		scpy((char*)&keysarray[id_array[TMPINDX_2].keys_offset], 
		    (char*)Otharay[rp_sth.s_sort_no - 1],
		    (*(Otharay[rp_sth.s_sort_no - 1]) * 4 + 1) * sizeof(int) );
	}
#ifdef	ORACLE
	STRCPY( id_array[TMPINDX_2].id_f_name, id_array[GLMAST].id_f_name) ;
	id_array[TMPINDX_2].id_f_type = ISAM ;
	id_array[TMPINDX_2].id_io_mode = R ;
	id_array[TMPINDX_2].reclen = id_array[GLMAST].reclen ;
	id_array[TMPINDX_2].tot_keys = 1 ;
#endif
	return(0) ;
}
/*--------------------------------------------------------------*/
RemoveFiles()	/* Remove temporary sort files */
{
	int	i ;
	char	ix_file[25] ;
	char	tnum[5];

#ifndef	ORACLE
	UnlinkFile(GLTITLES) ;
#else
	STRCPY(ix_file,GLTITLES) ;
	get_tnum(tnum) ;
	strcat(ix_file,tnum) ;
	STRCPY( id_array[TMPINDX_1].id_f_name, ix_file) ;
	drop_tbl(TMPINDX_1, e_mesg) ;
#endif
	TitleSort = 0 ;

#ifndef	ORACLE
	UnlinkFile(GLBUDGET) ;
#endif
	BdgtSort = 0 ;

	for( i = 1 ; i <= SUM_SORTS ; i++) {
#ifndef	ORACLE
		sprintf(ix_file,"%s%d",GLSORT,i) ;

		UnlinkFile(ix_file) ;
#endif
		OthSorts[i - 1] = 0; 
	}

	return(0) ;
}
#ifndef	ORACLE
/*--------------------------------------------------------------*/
static	int	/* Unlink the Given file */
UnlinkFile(ix_file)
char	*ix_file ;
{
	char	indx_flnm[50] ;
	char	tnum[5];

	form_f_name(ix_file, indx_flnm);
	get_tnum(tnum) ;
	strcat(indx_flnm,tnum);

	if ( access(indx_flnm,0) >= 0 ) 
		unlink(indx_flnm) ;		/* Unlink Data File */

	strcat(indx_flnm, ".IX");
	if ( access(indx_flnm,0) >= 0 ) 
		unlink(indx_flnm) ;		/* Unlink Index File */

	return(0);
}	/* UnlinkFile() */
#endif
/*--------------------------------------------------------------*/
static	int	/* Creat Titles File */
TitleFile(ix_file)
char	*ix_file ;	/* Index file name to be created */
{
	int	fd ;
	char	indx_flnm[50] ;
	int	i, j ;
#ifndef	ORACLE
	int	alt_array[1] ;
#endif

	Titlearay[0] = TitlesKarray(rp_sth.s_sort_keys, Titlearay) ;

#ifndef	ORACLE
	/* Form the data file name */
	form_f_name(ix_file, indx_flnm);

	fd = iscreat(indx_flnm, RWR, 1, Titlearay, sizeof(Gl_rec)) ;
	if(fd == ERROR) {
#ifdef ENGLISH
	    sprintf(e_mesg,"ERROR: %s Creation Error.. Iserror: %d errno: %d",
#else
	    sprintf(e_mesg,"ERREUR: %s Erreur de creation.. Iserror: %d errno: %d",
#endif
			indx_flnm, iserror, errno );
	    return(ERROR) ;
	}
#else
	STRCPY( id_array[TMPINDX_1].id_f_name, ix_file) ;

	scpy((char*)&keysarray[id_array[TMPINDX_1].keys_offset], 
		(char*)Titlearay, (*(Titlearay) * 4 + 1) * sizeof(int) );

	id_array[TMPINDX_1].id_f_type = ISAM ;
	id_array[TMPINDX_1].id_io_mode = RWR ;
	id_array[TMPINDX_1].reclen = id_array[GLMAST].reclen ;
	id_array[TMPINDX_1].tot_keys = 1 ;

	/* Create the creat_tbl cursor for title file using GLMAST .def file */

	if(CreateCursor(TMPINDX_1, CREAT_TBL, 0) < 0) return(ERROR) ;
	if(CreateCursor(TMPINDX_1, CREAT_INDX, 0) < 0) return(ERROR) ;

	init_file(TMPINDX_1, e_mesg) ;

	/* Create the insert cursor for title file using GLMAST .def file */
	if(CreateCursor(TMPINDX_1, INSERT, 0) < 0) return(ERROR) ;
#endif

	/* Read the file on Reccod sequence index key(2nd alt) */
	/* Initialize GL Master 2nd alt key */
	gl_rec.reccod = 41 ;		/* First Title Record Code */
	gl_rec.funds = 1 ;		/* First Possible fund in File */
#if	EXPENDITURE < INCOME
	gl_rec.sect = EXPENDITURE ;
#else
	gl_rec.sect = INCOME ;
#endif
/***
	gl_rec.sect = (EXPENDITURE < INCOME) ? EXPENDITURE : INCOME ;
***/
					/* First possible section code */
	gl_rec.accno[0] = '\0' ;
	flg_reset(GLMAST) ;

#ifndef	ORACLE
	alt_array[0] = 1;
#endif
	for( ; ; ) {
		ret_cd = get_n_gl(&gl_rec, BROWSE, 2, FORWARD, e_mesg) ;
		if(EFL == ret_cd) break ;

		if(ret_cd < 0) {
#ifndef	ORACLE
			isclose(fd) ;
#endif
			return(DBH_ERR) ;
		}

		if(gl_rec.reccod > 52) break ;	/* No More Title Records */

		/* Skip Assets & Liabilities records */
		if(gl_rec.sect != EXPENDITURE && gl_rec.sect != INCOME)
			continue ;

		/* Skip Record, whose record code is not in user keys */
		for(i = 0 ; i < USER_KEYS && rp_sth.s_sort_keys[i] ; i++)
			if((gl_rec.reccod-40) == rp_sth.s_sort_keys[i]) break ;

		if(i == USER_KEYS || rp_sth.s_sort_keys[i] == 0) continue ;

		/* if ALL_KEYS is OFF or matched Key# is < 8 move 0's the keys
		   upto matched POS */
		if( !ALL_KEYS || rp_sth.s_sort_keys[i] < 8)
			for(j = 0 ; j < (i - 1) ; j++)
				gl_rec.keys[rp_sth.s_sort_keys[j] - 1] = 0;

		/* Move 0's all the keys after Matched key pos */
		for(j = (i+1) ; j < USER_KEYS ; j++)
			gl_rec.keys[rp_sth.s_sort_keys[j] - 1] = 0;

		/* Skip record, if key value is zero in all the user keys */
		for(j = 0 ; j < USER_KEYS && rp_sth.s_sort_keys[j] ; j++)
			if(gl_rec.keys[rp_sth.s_sort_keys[j] - 1]) break ;

		if(j == USER_KEYS || rp_sth.s_sort_keys[j] == 0) continue ;

#ifndef	ORACLE
#ifdef	FIXED_RECLEN
		ret_cd = iswrite(fd, (char*)&gl_rec, alt_array) ;
#else
		ret_cd = iswrite(fd, (char*)&gl_rec, sizeof(Gl_rec),alt_array) ;
#endif
		if(ret_cd == ERROR) {
			sprintf(e_mesg,
#ifdef ENGLISH
			    "ERROR: in Title Writing.. Iserror: %d errno: %d",
#else
			    "ERREUR: Dans l'ecriture du titre.. Iserror: %d errno: %d",
#endif
			    iserror, errno );
			isclose(fd) ;
			return(DBH_ERR) ;
		}
#else
		ret_cd = put_isrec((char*)&gl_rec, ADD, TMPINDX_1, e_mesg) ;
		if(ret_cd == ERROR) return(DBH_ERR) ;
#endif
		if(ret_cd == DUPE)
#ifdef ENGLISH
			fomer("Duplicate Title") ;
#else
			fomer("Titre en double") ;
#endif
	}
	seq_over(GLMAST) ;
#ifndef	ORACLE
	isclose(fd) ;
#else
	if(commit(e_mesg) < 0) return(DBH_ERR) ;

	/* Create the get_isrec cursor for title file using GLMAST .def file */

	if(CreateCursor(TMPINDX_1, SELECT, BROWSE) < 0) return(ERROR) ;
#endif
	return(NOERROR) ;
}
/*-----------------------------------------------------------------*/
/* Build Keys array for titles sort & Return no of parts */

static	int
TitlesKarray(keynos,k_array)
short	*keynos ;
int	*k_array ;
{
	int	i ;

	k_array++ ;	/* Skip the No of parts fld */

	for( i = 0 ; i < USER_KEYS && *(keynos) ; i++, keynos++ ){
		*(k_array++) = LONG ;
		*(k_array++) = 1 ;
		*(k_array++) = (char*)&gl_rec.keys[*keynos - 1] -
				(char*)&gl_rec ;
		*(k_array++) = ASCND ;
	}

	return(i) ;	/* Return No Parts */

}	/* TitlesKarray() */
/*------------------------------------------------------------------------*/
static	int
CreatIxFile(ix_file, karray)
char	*ix_file ;
int	*karray ;
{
#ifndef	ORACLE
	int	fd, retcode ;
	char	source_flnm[50] ;
	char	indx_flnm[50] ;

	/* Form the data file names */
	form_f_name(GLMAST_FILE, source_flnm);
	form_f_name(ix_file, indx_flnm);
#endif

	karray[0] = BldKarray(karray) ;

#ifndef	ORACLE
	fd = isbuild(source_flnm, RWR, karray, indx_flnm, sizeof(Gl_rec)) ;
	if(fd == ERROR) {
#ifdef ENGLISH
	    sprintf(e_mesg,"ERROR: %s Creation Error.. Iserror: %d errno: %d",
#else
	    sprintf(e_mesg,"ERREUR: %s Erreur de creation.. Iserror: %d errno: %d",
#endif
			indx_flnm, iserror, errno );
	    return(DBH_ERR) ;
	}

	retcode = WriteIndex(fd,data_fd) ;

	isclose(fd) ;

	if(retcode != 0) return(DBH_ERR) ;
#endif
	return(NOERROR) ;
}
/*-----------------------------------------------------------------*/
/* Build Keys array for Budget & Sort Required file and Return no of parts */

static	int
BldKarray(k_array)
int	*k_array ;
{
	int	keys, j,option ;

	k_array++ ;	/* Skip the No of parts fld */

	keys = 0 ;

	/* Fund */
	*(k_array++) = SHORT ;
	*(k_array++) = 1 ;
	*(k_array++) = (char*)&gl_rec.funds - (char*)&gl_rec ;
	*(k_array++) = ASCND ;
	keys++ ;

	/* Scetion */
	*(k_array++) = SHORT ;
	*(k_array++) = 1 ;
	*(k_array++) = (char*)&gl_rec.sect - (char*)&gl_rec ;
	*(k_array++) = ASCND ;
	keys++ ;

	/* Admissibility */
	*(k_array++) = SHORT ;
	*(k_array++) = 1 ;
	*(k_array++) = (char*)&gl_rec.admis - (char*)&gl_rec ;
	*(k_array++) = ASCND ;
	keys++ ;

	option = atoi(rp_sth.s_option);
	if(option <= 5) {
	    /* User K4 to User k6 */
	    for( j = 4 ; j <= 6 && KEY_EXISTS(j) ; j++ ){
		*(k_array++) = LONG ;
		*(k_array++) = 1 ;
		*(k_array++) =
			(char*)&gl_rec.keys[KEY(j)] - (char*)&gl_rec ;
		*(k_array++) = ASCND ;
		keys++ ;
	    }
	}
	else {

		if(SK1 && KEY_EXISTS( SK1 )) {
			*(k_array++) = LONG ;
			*(k_array++) = 1 ;
			*(k_array++) = (char*)&gl_rec.keys[KEY( SK1 )] -
					(char*)&gl_rec ;
			*(k_array++) = ASCND ;

			keys++ ;
		}

		if(SK2 && KEY_EXISTS( SK2 )) {
			*(k_array++) = LONG ;
			*(k_array++) = 1 ;
			*(k_array++) = (char*)&gl_rec.keys[KEY( SK2 )] -
					(char*)&gl_rec ;
			*(k_array++) = ASCND ;

			keys++ ;
		}

		if(SK3 && KEY_EXISTS( SK3 )) {
			*(k_array++) = LONG ;
			*(k_array++) = 1 ;
			*(k_array++) = (char*)&gl_rec.keys[KEY( SK3 )] -
					(char*)&gl_rec ;
			*(k_array++) = ASCND ;

			keys++ ;
		}

	}	/* Else */

	/* Reccod */
	*(k_array++) = SHORT ;
	*(k_array++) = 1 ;
	*(k_array++) = (char*)&gl_rec.reccod - (char*)&gl_rec ;
	*(k_array++) = ASCND ;
	keys++ ;

	/* Account#. Added to get records in account order */
	*(k_array++) = CHAR ;
	*(k_array++) = 18 ;
	*(k_array++) = (char*)gl_rec.accno - (char*)&gl_rec ;
	*(k_array++) = ASCND ;
	keys++ ;

	return(keys) ;	/* Return No Parts */

}	/* BldKarray() */
#ifndef	ORACLE
/*---------------------------------------------------------------------*/
static	int
WriteIndex(fd, source_fd)
int	fd ;		/* ISAM file fd */
int	source_fd ;	/* Data File fd */
{
	char	status[1] ;
	int	retcode ;
	long	readpos ;

	readpos = 0L ;

	while(1) {

		/* Lseek to last position again. ixbuild() changes the
		  file position */
 		readpos = lseek(source_fd, readpos, 0) ;

#if	STATUS_LEN == 1
		if(read(source_fd, status, 1) < 1) break ;
		if(status[0] == SET_DEL) {
			readpos += sizeof(Gl_rec) + STATUS_LEN ;
			continue ;
		}
#endif
		if(read(source_fd, (char*)&gl_rec, sizeof(Gl_rec)) <= 0) break ;

		if(SeqConstrnt(&gl_rec) < 0) {
			readpos += sizeof(Gl_rec) + STATUS_LEN;
			continue ;
		}

#ifdef	FIXED_RECLEN
		retcode = ixbuild(fd, (char*)&gl_rec, readpos) ;
#else
		retcode = ixbuild(fd, (char*)&gl_rec, readpos, sizeof(Gl_rec)) ;
#endif
		if(retcode == ERROR) {
#ifdef ENGLISH
			sprintf(e_mesg,
				"INDEX WRITING ERROR... Iserror: %d Errno: %d",
				iserror, errno);
#else
			sprintf(e_mesg,
				"ERREUR D'INSCRIPTION A L'INDEX... Iserror: %d Errno: %d",
				iserror, errno);
#endif
			return (retcode) ;
		}

		readpos += sizeof(Gl_rec) + STATUS_LEN ;

	} /* While() */

	return(0) ;
}
#endif
/*----------------------------------------------------------------*/
#ifndef	ORACLE
static	int
#endif
SeqConstrnt(g_rec)
Gl_rec	*g_rec ;
{
	/* Skip Title records */
	if(g_rec->reccod < 97) return(ERROR) ;

	/* Skip Assets & Liabilities records */
	if(g_rec->sect != EXPENDITURE && g_rec->sect != INCOME)
			return(ERROR) ;

	/* Skip record, if key value of 1st user key is zero */
	if(KEY_EXISTS(1) && g_rec->keys[KEY(1)] == 0) return(ERROR) ;

	return(NOERROR) ;
}	/* SeqConstrnt() */
/*----------------------------------------------------------------*/
/* Check whether the keysarray size is exceeding the allocated size in DBH */
static	int
ChkKarraysize(file_no, karray)
int	file_no ;
int	*karray ;
{
	int	size ;

	size = karraysize(file_no) ;

	if( size < (*(karray) * 4 + 1) ) {
		sprintf(e_mesg,
#ifdef ENGLISH
		"Temporary Keysarray size is exceeding the Allocated size: %d",
#else
		"Grandeur du KEYSARRAY temporaire excede la grandeur allouee: %d",
#endif
		size);
		return(ERROR) ;
	}
	return(0);
}	/* ChkKarraysize() */
/*----------------------------------------------------------------*/
#ifdef	ORACLE
static	int
CreateCursor(file_no, op_code, mode)
int	file_no ;
int	op_code ;
int	mode ;
{
	Tbl_struct	*ptr, *AllocateCursor() ;
	Fld_hdr		hdr ;
	Field		*fld = NULL ;
	char		o_mesg[133] ;

	if ( tbl_lst[file_no] == NULL ) {
		if((tbl_lst[file_no] = AllocateCursor()) == NULL) goto last ;
		ptr = tbl_lst[file_no] ;
	}
	else {
		for (ptr =  tbl_lst[file_no] ; ; ptr = ptr->nxt_lst) {
			if ( ptr->key_no == 0 && ptr->mode == 0 
			    &&  ptr->op_code == op_code && ptr->dirn == RANDOM )
				return(NOERROR);

			if ( ptr->nxt_lst == NULL ) break ;
		}
		if ((ptr->nxt_lst = AllocateCursor()) == NULL) goto last;
		ptr = ptr->nxt_lst ;
	}

	ptr->key_no  = 0 ;
	ptr->dirn    = RANDOM ;
	ptr->mode    = mode ;
	ptr->op_code = op_code ;
	ptr->nxt_lst = NULL ;
	ptr->stmnt_ptr = NULL ;

#ifdef	DEBUG
	fprintf(sqlfp,"%s\n", __FILE__);
	fprintf(sqlfp,
	 "New Cursor..  File_no: %d  Op_code: %d Key_no: %d Mode: %d Dir: %d\n",
	     file_no, ptr->op_code, ptr->key_no, ptr->mode, ptr->dirn);
#endif

	if (oopen(ptr->cursor, LDA, (char*)0, -1, -1, (char*)0, -1)) {
		oermsg( ptr->cursor->csrrc, o_mesg );
		strncpy(e_mesg, o_mesg, 79) ;
		return (ERROR) ;
	}

	if ( GetFields (id_array[GLMAST].id_f_name, &hdr, &fld, e_mesg)
					!= NOERROR) return(ERROR) ;

	if ( form_sql (file_no, ptr ,  &hdr, fld ) != NOERROR ) {
		if(fld != NULL) free((char*)fld) ;
#ifdef ENGLISH
		STRCPY(e_mesg,"ERROR in SQL statement construction for TITLE file");
#else
		STRCPY(e_mesg,"ERREUR dans la construction du releve SQL pour le dossier TITRE");
#endif
		return(ERROR);
	}

#ifdef	DEBUG
	prnt_sql( ptr->stmnt_ptr ) ;
#endif

	if ( osql3( ptr->cursor, ptr->stmnt_ptr, -1 ) ) {
		oermsg( ptr->cursor->csrrc, o_mesg );
		strncpy(e_mesg, o_mesg, 79) ;
		if(fld != NULL) free((char*)fld) ;
		return (ERROR) ;
	}

	if ( bind_vars ( file_no, ptr , &hdr, fld, e_mesg ) == ERROR  || 
		bind_where ( file_no, ptr, &hdr, fld, e_mesg ) == ERROR ) {
		if(fld != NULL) free((char*)fld) ;
		return (ERROR) ;
	}

	if( fld != NULL ) free( (char*)fld ) ;

	return(NOERROR) ;

last:
	STRCPY(e_mesg, "Memory Allocation Error");
	return(ERROR) ;
}	/* CreateCursor() */
#endif
/*-----------------------------END OF FILE---------------------------*/

