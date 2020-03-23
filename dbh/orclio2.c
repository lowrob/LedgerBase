/*
*	orclio2.c
*/

#include <stdio.h>
#include <isnames.h>
#include <bfs_defs.h>
#include <fld_defs.h>
#include <filein.h>
#include <dberror.h>
#include <incl_ora.h>

#define	MAX_SQL_LEN		3072

extern	int	dbh_init   ;		/* Whether DBH is UP or not */
extern	int	*keysarray ;		/* Keys array read from keydesc.id */
extern	f_id	id_array[] ;		/* file Descriptor Tabele */
extern	char	owner_prefix[] ;	/* Owner of Tables */
extern	char	tablespace[] ;		/* Owner of Tables */
extern	long	Seqrec_no ;		/* Current Sequential Record number */

#ifdef	DEBUG
extern	FILE	*sqlfp ;
#endif

static	char	*s_ptr ;

/*
* Constructs SQL statement given its type (INSERT, DELETE, SELECT, UPDATE_TBL
* etc..) and table number. WHere clause is constructed based on key_no ..
*
*	Bugs : Does not check for Sql statement Overflow in stmnt buffer ...
*/


int
form_sql ( file_no, ptr , hdr, fld )
int		file_no ;
Tbl_struct	*ptr ;
Fld_hdr		*hdr ;
Field		*fld ;
{
	char	*malloc() , *stmnt, temp[100] ;

	if ( (stmnt = malloc(MAX_SQL_LEN)) == NULL ) dbexit(MALLOCERR, ERROR)

	s_ptr = stmnt ;

	dberror = 0 ;

	switch( ptr->op_code ) {

	case INSERT :
		AppendToStmnt("INSERT INTO ");
		AddFilename( file_no ) ;
		AppendToStmnt( " ( " );

		if( id_array[file_no].id_f_type == SEQ )
			cp_seqrecno (0) ;

		cp_flds ( hdr, fld, 0 ) ;
		AppendToStmnt(" ) VALUES ( ");

		if( id_array[file_no].id_f_type == SEQ )
			cp_seqrecno (1) ;

		cp_flds ( hdr, fld, 1 ) ;
		AppendToStmnt(" ) ");
		break ;
	case SELECT :
	case LOCK_TABLE : 	/* Select without Where and with update clause*/
		AppendToStmnt("SELECT ");
		if( id_array[file_no].id_f_type == SEQ )
			cp_seqrecno (0) ;
		cp_flds ( hdr, fld, 0 ) ;
		AppendToStmnt(" FROM ");
		AddFilename( file_no ) ;
		if(ptr->op_code == SELECT){
			AppendToStmnt(" WHERE ");
			if( id_array[file_no].id_f_type == SEQ ){
			    if( seq_where((int)ptr->dirn, 0) == ERROR) break;
			}
			else if( cp_where(file_no, hdr, fld, (int)ptr->key_no,
				(int)ptr->dirn, 0) == ERROR) break;
		}
		if( id_array[file_no].id_f_type == SEQ ) {
		    if( seq_order((int)ptr->dirn) == ERROR) break ;
		}
		else
		    if( cp_order(file_no, hdr, fld,
			(int)ptr->key_no, (int)ptr->dirn) == ERROR) break ;

		if(ptr->mode == UPDATE)
		    cp_updt( hdr, fld, (int)ptr->mode) ;
		break ;
	case UPDATE_TBL :
		AppendToStmnt("UPDATE ");
		AddFilename( file_no ) ;
		AppendToStmnt(" SET ");
		cp_flds (hdr, fld, 2) ;
		AppendToStmnt(" WHERE ");
		if( id_array[file_no].id_f_type == SEQ ){
			if( seq_where((int)ptr->dirn, 0) == ERROR) break;
		}
		else if ( cp_where ( file_no, hdr, fld,
			(int)ptr->key_no, (int)ptr->dirn, 0) == ERROR) break ;
		break ;
	case	DELETE :	/* Delete from table where .... */
	case	DELETE_ALL :	/* Delete from table (without where clause) */
		AppendToStmnt ("DELETE FROM ");
		AddFilename( file_no ) ;
		if(ptr->op_code == DELETE) {
		    AppendToStmnt( " WHERE ");
		    if( cp_where ( file_no, hdr, fld,
			(int)ptr->key_no, (int)ptr->dirn, 0) == ERROR) break ;
		}
		break ;
	case 	CREAT_TBL :
		AppendToStmnt ("CREATE TABLE ");
		AddFilename( file_no ) ;
		AppendToStmnt ( " ( " );

		if( id_array[file_no].id_f_type == SEQ )
			cp_seqrecno (3) ;

		cp_flds (hdr, fld, 3) ;
		AppendToStmnt (" )");
		if(tablespace[0] != '\0') {
			AppendToStmnt(" TABLESPACE ") ;
			AppendToStmnt(tablespace) ;
		}
		break ;
	case 	CREAT_INDX :
		if ( id_array[file_no].id_f_type == SEQ || ptr->key_no == 0 )
			AppendToStmnt ("CREATE UNIQUE INDEX ");
		else
			AppendToStmnt ("CREATE INDEX ");

		AddFilename(file_no) ;

		/* Create Unique Index name by appending key_no to the file.. */
		sprintf( temp, "%d", (int)ptr->key_no);
		AppendToStmnt (temp);
		AppendToStmnt (" ON ");
		AddFilename( file_no ) ;
		AppendToStmnt ( " ( " );
		if( id_array[file_no].id_f_type == SEQ ) {
		    if( seq_where ((int)ptr->dirn, 1) == ERROR) break ;
		}
		else if( cp_where ( file_no, hdr, fld,
			(int)ptr->key_no, (int)ptr->dirn, 1) == ERROR) break ;
		AppendToStmnt (" )");
		if(tablespace[0] != '\0') {
			AppendToStmnt(" TABLESPACE ") ;
			AppendToStmnt(tablespace) ;
		}
		break ;
	case 	DROP_TBL :
		AppendToStmnt ("DROP TABLE ");
		AddFilename( file_no ) ;
		break ;
	case	MAX_RECNO :
		AppendToStmnt("SELECT NVL(MAX(");
		if( id_array[file_no].id_f_type == SEQ )
			AppendToStmnt(SEQREC_FLDNM) ;
		else
			Cp_MaxSnoWhere(file_no, hdr, fld, ptr, 0);
		AppendToStmnt( "),0) FROM ") ;
		AddFilename( file_no ) ;
		if( id_array[file_no].id_f_type != SEQ )  /* Only for ISAM */
			Cp_MaxSnoWhere(file_no, hdr, fld, ptr, 1);
		break ;
	default :
		dberror = INVOPCODE ;
	}

	*s_ptr++ = '\0' ;	/* Last char */

	if(dberror == 0) {
		if ( (ptr->stmnt_ptr = malloc((unsigned)(s_ptr - stmnt)))
				== NULL ) dbexit(MALLOCERR,ERROR)
		strcpy( ptr->stmnt_ptr, stmnt) ;
	}
	free ( stmnt ) ;

	if ( dberror != 0 ) return(ERROR) ;  /* Statement formation Error */

	return(0) ;
}	/* form_sql */


static	int
cp_flds (hdr, fld, flag)
Fld_hdr	*hdr ;
Field 	*fld ;
int	flag ;		/* 0 - No Colon(:) to Precede,
			   1 - Precede each field with ":"
			   2 - Copy field with : after =
			   3 - COpy column type and length for creat tbl */
{
	int	i ;
	char	temp[20] ;

	for ( i = 0 ; ; i++ ) {
		if ( flag == 1 ) AppendToStmnt(":") ;
		AppendToStmnt((fld+i)->name) ;
		if ( flag == 2 ) {
			AppendToStmnt (" = :");
			AppendToStmnt ((fld+i)->name) ;
		}
		else if ( flag == 3 ) {

			switch ( (fld+i)->type ) {
			case T_CHAR	:
				sprintf(temp, " CHAR(%d)" , (fld+i)->len);
				AppendToStmnt (temp);
				break ;
			case T_SHORT	:
				AppendToStmnt (" NUMBER(5)");
				break ;
			case T_INT	:
				if(sizeof(int) == sizeof(short)){
					AppendToStmnt (" NUMBER(5)");
					break ;
				}
				/* else fall thru to T_LONG */
			case T_LONG	:
				AppendToStmnt (" NUMBER(10)");
				break ;
			case T_FLOAT	:
				AppendToStmnt (" NUMBER(6,2)");
				break ;
			case T_DOUBLE	:
				AppendToStmnt (" NUMBER(12,4)");
				break ;
			}
		}
		if ( i < hdr->no_fields - 1  )
			AppendToStmnt(", ");
		else break ;
	}
	return(0);
}




/*
*	Sample where clause in select statement for get_next in FORWARD
*	direction.
*
*	For Keys, which have more than one part:
*
*	select * from TABLE where
*	   KP1 > KV1 or
*	      (KP1 = KV1 and (KP2 > KV2 or
*	         (KP2 = KV2 and (KP3 > KV3 or
*	             ...........
*	                (KPN-2 = KVN-2 and (KPN-1 > KVN-1 or
*	                   (KPN-1 = KVN-1 and (KPN > KVN or KPN = KVN))))))))
*
*	For keys which have single Part:
*
*		select * from TABLE where KP1 > KV1 or KP1 = KV1
*
*	Here KP is Key part and KV is Key part value.
*/



static	int
cp_where (file_no, hdr, fld, key_no, dirn, flag)
int	file_no ;
Fld_hdr	*hdr ;
Field	*fld ;
int	key_no ;	/* 0 - Main , Others - Alt */
int	dirn ;		/* 2 : RANDOM MODE , 0 - FORWARD, 1 - BACKWARD */
int	flag ;		/* 0 - Copy Substitution Variables, 1- Copy only list */
{
	int	ofset, parts, i, j, fld_offset, out_braces ;
	char	temp[50] ;

	out_braces = 0 ;
	ofset = id_array[file_no].keys_offset ;

	for ( i = 0 ; i < key_no ; i++ )
		ofset += keysarray[ofset] * 4 + 1 ;

	parts = keysarray[ofset] ;
	ofset += 3 ;			/* Position at the offset in TLPO */

	for ( j = 0 ; j < parts ; j++, ofset += 4 ) {

	    fld_offset = keysarray[ofset] ;		/* Ofset of keypart*/

	    for ( i = 0 ; i < hdr->no_fields ; i++) {

		if ( (fld+i)->offset != fld_offset) continue ;

		AppendToStmnt( (fld+i)->name ) ;
		if ( flag == 0 ) {
		    AppendToStmnt(" ");
		    if(dirn == RANDOM) {
			sprintf(temp, "= :%d", j+1);
			AppendToStmnt (temp);
			if ( j < parts - 1 )
				AppendToStmnt (" AND ");
		    }
		    else if(dirn == EQUAL) {
			if ( j == parts - 1 ) {
				if(keysarray[ofset+1] == ASCND)
					AppendToStmnt(">");
				else
					AppendToStmnt("<");
			}
			sprintf(temp, "= :%d", j+1);
			AppendToStmnt (temp);
			if ( j < parts - 1 )
				AppendToStmnt (" AND ");
		    }
		    else {
			if ( dirn == FORWARD ) {
				if(keysarray[ofset+1] == ASCND)
					AppendToStmnt(">");
				else
					AppendToStmnt("<");
			}
			else if ( dirn == BACKWARD ) {
				if(keysarray[ofset+1] == ASCND)
					AppendToStmnt("<");
				else
					AppendToStmnt(">");
			}

			if ( j < parts - 1 ) {
				sprintf(temp, " :%d", j+1);
				AppendToStmnt (temp);
				AppendToStmnt(" OR ( ");
				out_braces++ ;
				sprintf(temp, "%s = :%d", (fld+i)->name, j+1);
				AppendToStmnt (temp);
				AppendToStmnt (" AND");
				AppendToStmnt (" (");
				out_braces++ ;
				AppendToStmnt(" ");
			}
			else {
				sprintf(temp, "= :%d", j+1);
				AppendToStmnt (temp);
				AppendToStmnt(" OR ");
				sprintf(temp, "%s = :%d", (fld+i)->name, j+1);
				AppendToStmnt (temp);
			}
		    }	/* if dirn == RANDOM else */
		}	/* if flag == 0 */
		else if ( flag == 1 )  {
			if ( keysarray[ofset+1] == ASCND )
				AppendToStmnt(" ASC");
			else
				AppendToStmnt(" DESC");

			if ( j < parts  -1 )
				AppendToStmnt (", ");
		}

		break ;
	    }	/* for(.. ; i < hdr->no_fileds ; ..  ) */

	    if ( i == hdr->no_fields ) {
#ifdef	DEBUG
		fprintf(sqlfp, "NO Match for ofset: %d, part: %d\n file: %s\n",
			fld_offset, j, id_array[file_no].id_f_name);
		fprintf(sqlfp, "Keysarray Elm: %d %d %d %d\n",
			keysarray[ofset-2], keysarray[ofset-1],
			keysarray[ofset], keysarray[ofset+1]);
		fprintf(sqlfp, "id.offset: %d Ofset: %d Key_size: %d\n",
			id_array[file_no].keys_offset ,ofset, keysarray[0]);
#endif
		dbexit(OFFSETERR, ERROR)
	    }
	}	/* for() */

	for( ; out_braces > 0 ; out_braces--)
		AppendToStmnt(" )");

	return(0) ;
}	/* cp_where() */



static	int
cp_order (file_no, hdr, fld, key_no, dirn)
int	file_no ;
Fld_hdr	*hdr ;
Field	*fld ;
int	key_no ;	/* 0 - Main , Others - Alt */
int	dirn  ;		/* 2 : RANDOM , 0 - FORWARD, 1- BACKWARD */
{
	int	ofset, parts, i, j, fld_offset ;

	if ( dirn == RANDOM ) return (0) ;

	AppendToStmnt(" ORDER BY ");

	ofset = id_array[file_no].keys_offset ;

	for ( i = 0 ; i < key_no ; i++ )
		ofset += keysarray[ofset] * 4 + 1 ;

	parts = keysarray[ofset] ;
	ofset += 3 ;			/* Position at the offset in TLPO */

	for ( j = 0 ; j < parts ; j++, ofset += 4 ) {

		fld_offset = keysarray[ofset] ;		/* Ofset of keypart*/

		for ( i = 0 ; i < hdr->no_fields ; i++) {

			if ( (fld+i)->offset != fld_offset) continue ;

			AppendToStmnt( (fld+i)->name ) ;

			if ( dirn == BACKWARD ) {
				if(keysarray[ofset+1] == ASCND)
					AppendToStmnt(" DESC");
				else
					AppendToStmnt(" ASC");
			}
			else if ( dirn == FORWARD ) {
				if(keysarray[ofset+1] == ASCND)
					AppendToStmnt(" ASC");
				else
					AppendToStmnt(" DESC");
			}

			if ( j < parts -1 ) AppendToStmnt (", ");

			break ;
		}

		if ( i == hdr->no_fields ) {
#ifdef	DEBUG
			fprintf(sqlfp,
				"NO Match for ofset: %d, part: %d\n file: %s\n",
				fld_offset, j, id_array[file_no].id_f_name);
#endif
			dbexit(OFFSETERR, ERROR)
		}
	}

	return (0) ;
}	/* cp_order() */


static	int
cp_updt  ( hdr, fld, mode )
Fld_hdr	*hdr ;
Field	*fld ;
int	mode ;
{
	int	i ;

	if ( mode != UPDATE ) return (0) ; 	/* NO Locking By Default */

	AppendToStmnt(" FOR UPDATE OF ");	/* ELse Lock all fields  */

	for ( i = 0 ; ; i++) {
		AppendToStmnt( (fld+i)->name ) ;
		if ( i < hdr->no_fields -1 )
			AppendToStmnt(", ");
		else break ;
	}
	AppendToStmnt(" NOWAIT") ;

	return (0) ;
}	/* cp_updt() */


static	int
cp_seqrecno (flag)
int	flag ;		/* 0 - No Colon(:) to Precede,
			   1 - Precede each field with ":"
			   2 - Copy field with : after =
			   3 - COpy column type and length for creat tbl */
{
	if ( flag == 1 ) AppendToStmnt(":") ;

	AppendToStmnt(SEQREC_FLDNM) ;

	if ( flag == 2 ) {
		AppendToStmnt (" = :");
		AppendToStmnt (SEQREC_FLDNM) ;
	}
	else if ( flag == 3 ) {
		AppendToStmnt (" NUMBER(5)");
	}
	AppendToStmnt(", ");

	return(0);
}

static	int
seq_where ( dirn, flag)
int	dirn ;		/* 2 : RANDOM MODE , 0 - FORWARD, 1 - BACKWARD */
int	flag ;		/* 0 - Copy Substitution Variables, 1- Copy only list */
{
	AppendToStmnt( SEQREC_FLDNM ) ;

	if ( flag == 0 ) {
		if ( dirn == FORWARD )
			AppendToStmnt(" >");
		else if ( dirn == BACKWARD )
			AppendToStmnt(" <");
		else
			AppendToStmnt(" ");

		AppendToStmnt("= :1 ") ;
	}
	else if ( flag == 1 )  {
		AppendToStmnt(" ASC");
	}

	return(0) ;

}	/* seq_where() */


static	int
seq_order (dirn)
int	dirn  ;		/* 2 : RANDOM , 0 - FORWARD, 1- BACKWARD */
{
	if ( dirn == RANDOM ) return (0) ;

	AppendToStmnt(" ORDER BY ");

	AppendToStmnt( SEQREC_FLDNM ) ;

	if( dirn == BACKWARD )
		AppendToStmnt(" DESC");
	else if( dirn == FORWARD )
		AppendToStmnt(" ASC");

	return (0) ;
}	/* seq_order() */

static	int
AddFilename( file_no )
int	file_no ;
{
	if( owner_prefix[0]!='\0' ){
		AppendToStmnt(owner_prefix) ;
		/* concatinate the database number to it */
		AppendToStmnt(dist_no) ;
		/* Accessing Previous Year data */
		if(SW9)
			AppendToStmnt(PREV_YEAR) ;
		AppendToStmnt(".") ;
	}
	AppendToStmnt(id_array[file_no].id_f_name);

	return(0) ;
}	/* AddFilename() */

static int
Cp_MaxSnoWhere(file_no, hdr, fld, ptr, flag)
int		file_no;
Fld_hdr		*hdr;
Field		*fld;
Tbl_struct	*ptr ;
int		flag;	/* 0 Copy the field name on which MAX will be applied */
			/* 1 Frame the WHERE clause */
{
  int	i, j, ofset, parts, fld_offset ;
  char	buffer[50] ;

	ofset = id_array[file_no].keys_offset ;
	for ( i = 0 ; i < ptr->key_no ; i++ )
		ofset += keysarray[ofset] * 4 + 1 ;

	if(ptr->mode >= 0)		/* Part number */
		parts = ptr->mode + 1 ;
	else
		parts =	 keysarray[ofset] ;

	ofset += 3 ;	/* Position at the offset in TLPO */
	if( flag == 0 ) 
	{	fld_offset = keysarray[ofset + (parts - 1) * 4];
		for (i = 0 ; i < hdr->no_fields ; i++)
			if ((fld+i)->offset == fld_offset)
				break;
		if(i == hdr->no_fields)
			dbexit(OFFSETERR, ERROR)
		AppendToStmnt( (fld+i)->name ) ;
	}
	else {
		if(parts > 1)
		{	AppendToStmnt(" WHERE ");
			for(i = 1; i < parts ; i++)
			{	fld_offset = keysarray[ofset];
				if(i > 1)
					AppendToStmnt(" AND ");
				for(j = 0; j < hdr->no_fields; j++)
					if((fld+j)->offset == fld_offset)
						break;
				if(j == hdr->no_fields)
					dbexit(OFFSETERR, ERROR)
				AppendToStmnt((fld+j)->name);
				sprintf(buffer," = :%d ", j+1);
				AppendToStmnt( buffer );
				ofset += 4;
			}
		}
	}
	return(0) ;
}

static	int
AppendToStmnt(str)
char	*str ;
{
	while (*str)		/* NULL shouldn't be copied */
		*s_ptr++ = *str++ ;

	return(0) ;
}	/* AppendToStmnt() */

/*--------------------------------------------------------------*/
/* ERROR Message forming routines */


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
			strcpy(key_str,"DBH Initialization ERROR") ;
			return(ERROR) ;
		}

	/* validate file_no & key_no */
	if(file_no < 0 || file_no >= TOTAL_FILES){
		sprintf(key_str,"Unknown File: %d",file_no);
		return(ERROR);
	}
	if(id_array[file_no].id_f_type == SEQ) {
		sprintf(key_str, "Record#: %ld",Seqrec_no) ;
		return(NOERROR);
	}
	if(key_no < 0 || key_no >= id_array[file_no].tot_keys){
		sprintf(key_str,"Unknown Key:%d",key_no);
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
			strcpy(key_str,"Illegal type...");
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

crt_msg(file_no, key_no, err_code, c_mesg, rec)
int	file_no ;	/* DBH File# */
int	key_no;		/* Rec# / Key_no */
int	err_code ;	/* error code (iserror or code)   */	
char	*c_mesg ;	/* message string allocated by caller */
char	*rec;		/* rec in question */
{
	char	*cp_msg();

	if(file_no < 0 || file_no >= TOTAL_FILES || dbh_init == 0)
		strcpy(c_mesg,"System Error: crt_msg()  Unknown File Name");
	else {
		sprintf(c_mesg,"%-20.20s %-25.25s",
			id_array[file_no].fl_user_nm, cp_msg(err_code));
		if(id_array[file_no].id_f_type == ISAM)
			strcat(c_mesg,"KEY: ") ;
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

	"Record Read/Written..",		/* 0 - NOERROR */
	"Record Not Available..",		/* 1 - UNDEF */
	"Record Already Exists..",		/* 2 - DUPE */
	"Record Locked..",			/* 3 - LOCKED */
	"Severe Read/Write Error..",		/* 4 - ERROR */
	"End of File Encountered..",		/* 5 - EFL */
	"Undefined Error Code.."		/* 6 - Unknown err code */
};

static
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

/*-----------------------------END OF FILE -----------------------------*/

