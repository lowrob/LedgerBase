/******************************************************************************
		Sourcename   : audrep.c
		System       : Budgetary Financial system.
		Module       : GL.
		Created on   : 89-05-11
		Created  By  : K HARISH.
		Cobol Source : gl020f--00

******************************************************************************
About the file:	
	This file has utilities to print audit report. It is called by two
	programs, eod and utlrep 
	The routines in this file that are used by the outside files are

	AudRep( char key );	prints the audit report sorted on given key
	InitDisplay( char *terminal );
	InitPrinter();

	File eod.c, calls AudRep() for printing audit report as an end of day
	process. File glrep2.c  calls auditrep(), which interacts with the
	user for sort key selection ( File, User or Time ), and inturn,
	calls AudRep().

History:
Programmer      Last change on    Details

K.HARISH__      1989/07/28      Removed main() & made it obj module
				It is called by two processes.
				1. end_day.c	End of day process.
				2. glrep2.c	Audit report on different keys

L.Robichaud	1993/10/13	Pass new parameter to close_rep function for 
				a banner to print with 3000 family.
******************************************************************************/

#include <stdio.h>
#include <reports.h>
#include <fld_defs.h>
#include <isnames.h>

#define SYSTEM		"AUDIT TRAIL"
#define MOD_DATE	"28-JUL-89"
#define DELETED		1
#define CHANGED		2
#define CREATED		3
#define CANCELED	4
#define YES		5

#define PATH_FILE_SIZE	50
#ifdef ENGLISH
#define AUD_LIST_HDG	"AUDIT REPORT" 
#define BYTIME		'T'
#define BYUSER		'U'
#define BYFILE		'F'

#define DISPLAY		'D'
#define PRINTER		'P'
#define FILE_IO		'F'
#else
#define AUD_LIST_HDG	"RAPPORT DE VERIFICATION"
#define BYTIME		'H'
#define BYUSER		'U'
#define BYFILE		'D'

#define DISPLAY		'A'
#define PRINTER		'I'
#define FILE_IO		'D'
#endif

static Pa_rec	param_rec;
static Aud_rec aud_rec, old_rec;	/* audit record and old buffer */
static char e_mesg[80];	/* for storing error messages */
Fld_hdr	fld_hdr[TOTAL_FILES];
Field	*fields[TOTAL_FILES];

long	get_date();
static int	retval;
static short	pgcnt; 	/* for page count */
static long sysdt;	/* system date */


/* key: Sort on F(ile)/U(ser)/T(ime) */
AudRep( key, outputon, filename )
char key;	/* Default: sort on time T(ime) */
char *outputon, *filename;
{
	int	i;
	char	tmpindxfile[20] ;
	char	tnum[5];

	old_rec.file_no = -1;

	if( outputon[0]==DISPLAY )
		STRCPY( filename, terminal );	/* terminal global variable */

	switch(*outputon) {
		case DISPLAY:
			outputon[0]='D';
			break;
		case FILE_IO:
			outputon[0]='F';
			break;
		case PRINTER:
		default:
			outputon[0]='P';
			break;
	}	

	if( InitOutput(outputon, filename)<0 )
		return(-1);

	LNSZ = 132;
	pgcnt = 0;		/* Page count is zero */
	linecnt = PGSIZE;	/* Page size in no. of lines */

	retval = get_param( &param_rec, BROWSE, 1, e_mesg );

	if( retval<0 ){
		printf(e_mesg);
		return(-1);
	}
	if( retval!=1 ){
#ifdef ENGLISH
		printf("Error in getting parameter file record");
#else
		printf("Erreur dans l'obtention du dossier parametres");
#endif
		return(-1);
	} 

	sysdt = get_date() ;

	/* used in getting field names from the field# & dbh file# */
	for( i=0; i<TOTAL_FILES; i++ )
		fld_hdr[i].filenm[0] = '\0';

	/* A temporary index on audit file is to be created if type is not 'T'*/
	if( key!=BYTIME ) {		/* Sort on File or User */
		STRCPY(tmpindxfile, "auditemp");
		get_tnum(tnum);
		strcat(tmpindxfile,tnum);
		if( CreateIndex( key, tmpindxfile )<0 )
			return(-1);
	}

	/* Now that the index & data files are ready, read and print records */
	if( PrintAud( key )<0 )
		return(-1);

	/* If temporary index was created earlier, delete it now */
	if( key!=BYTIME ){
		close_file(TMPINDX_1);
#ifndef	ORACLE
		if( DeleteIndex(tmpindxfile)<0 )
			return(-1);
#endif
	}
	else
		close_file(AUDIT) ;

	for( i=0; i<TOTAL_FILES; i++ )	/* free memory allocated for fields */
		if( fields[i] ){
			free( (char *)fields[i] );
			fields[i] = (Field *)NULL;
		}

	if( close_rep(BANNER)<0 )	/* close output file */
		return(-1);

	return(0);
}

InitOutput(outputon, filename)	/* Initialize the printer */
char	*outputon, *filename;
{
	/* always print to  printer# 1 if printer */
	if( opn_prnt( outputon, filename, 1, e_mesg, 1 )<0 ){
		printf(e_mesg);
		return(-1);
	} 
	return(0);
}

static
PrintAud( key )	/* Fetch record and print in a loop */
char key;
{
	int retval;
	char	oldfilenm[21],curfilenm[21];	/* file names */
	char	olduser[11],		/* user name */
		*ptr;

	for( retval=0,ptr=(char *)&aud_rec; 
	     retval<sizeof(Aud_rec)	  ; 
	     retval++ )
		*ptr++ = '\0';
	*oldfilenm = *curfilenm = *olduser = '\0';
	for( ; ; ){ 
		/* Get the next audit record */ 

		if( key!=BYTIME ) /* Read from temp. index file */
			retval = get_next((char *)&aud_rec,TMPINDX_1,0,
						FORWARD,BROWSE,e_mesg);

		else		/* read from regular audit index file */
			retval = get_n_audit( &aud_rec,BROWSE,0,FORWARD,e_mesg);

		if( retval==EFL )	/* No more records */
			break;		/* So quit */
		else if( retval<0 ){
			printf(e_mesg);
			return(retval);
		}
		/* Record was accessed well, so print the record */
		/* clear the duplication audit record if a new write occurs */
		if( strcmp(old_rec.terminal,aud_rec.terminal) ||
		    strcmp(old_rec.user_id,aud_rec.user_id) ||
		    old_rec.file_no!=aud_rec.file_no ||
		    old_rec.mode!=aud_rec.mode ||
		    old_rec.run_time!=aud_rec.run_time ||
		    strcmp(old_rec.rec_key,aud_rec.rec_key) ){
			if( prnt_line()<0 )
				return(REPORT_ERR);
			for( retval=0,ptr=(char *)&old_rec; 
			     retval<sizeof(Aud_rec)	  ; 
			     retval++
			)
			*ptr++ = '\0';
			old_rec.file_no = -1;
		}

		/* if sort on file and filename changed, start new page */
		getuserflnm( (int)aud_rec.file_no, curfilenm );
		if( key==BYFILE && strcmp( curfilenm, oldfilenm ) ){
			linecnt = PGSIZE;
			STRCPY( oldfilenm, curfilenm );
		}

		/* if sort on user_id and it has changed, start new page */
		if( key==BYUSER && strcmp( aud_rec.user_id,olduser ) ){
			linecnt = PGSIZE;
			STRCPY( olduser, aud_rec.user_id );
		}

		/* if linecount has equalled or exceeded page size */
		if( linecnt+3 >= PGSIZE ){		/* If it has */
			if((retval=PrntHdg( key ))<0)	/* Print hdg stuff */
				return(-1);
			if(retval == CANCELED) return(0) ;

			for( retval=0,ptr=(char *)&old_rec; 
			     retval<sizeof(Aud_rec)	  ; 
			     retval++	)
				*ptr++ = '\0';
			old_rec.file_no = -1;	
		}

		/* Now print the record values */
		if( PrntRec( key )<0 )
			return(-1);
	}
	if(pgcnt) {
		if(term == 99) {	/* Not Display */
#ifndef	SPOOLER
			if( rite_top()<0 )
				return(-1);
#endif
		}
		else
			last_page() ;
	}
	/** close_dbh(); ***/

	return(0);
}

static
PrntHdg( key )	/* Print heading with sort detail ( on which field ) */
char key;
{
	short offset;

	/* For display pause the report */
	if(term < 99){	/* Output On terminal */
		if(pgcnt){
			if(next_page() < 0)return(CANCELED);
		}
	}
	/* Skip first time page skipping for not display outputs */
	if(pgcnt || term < 99 ){
		if(rite_top() < 0)return(-1);
	}

	pgcnt++; 			/* increment page no */
	linecnt = 0;

	offset = ( LNSZ-strlen(param_rec.pa_co_name) )/2;
	mkln( offset, param_rec.pa_co_name, strlen(param_rec.pa_co_name) );
	if( prnt_line()<0 )	return(-1);
	
	mkln( 1, PROG_NAME, 10 );
#ifdef ENGLISH
	mkln( 55, "A U D I T   R E P O R T", 23 );
	mkln( 113, "PAGE: ", 6 );
#else
	mkln( 55, "R A P P O R T  D E  V E R I F I C A T I O N", 46 );
	mkln( 113, "PAGE: ", 6 );
#endif

	tedit( (char *)&pgcnt,"__0_",  line+cur_pos, R_SHORT ); 
	cur_pos += 4;
	if( prnt_line()<0 )	return(-1);

#ifdef ENGLISH
	mkln( 59, "SORTED ON: ", 11 );
#else
	mkln( 59, "TRIE SUR: ", 10 );
#endif
	switch( key ){
		case BYTIME:	/* chronological order */
#ifdef ENGLISH
			mkln( 65, "TIME", 4 );
#else
			mkln( 65, "HEURE", 5 );
#endif
			break;
		case BYFILE:	/* dbh file # order */
#ifdef ENGLISH
			mkln( 65, "FILE", 4 );
#else
			mkln( 65, "DOSSIER", 7 );
#endif
			break;
		case BYUSER:	/* order on user_id */
#ifdef ENGLISH
			mkln( 65, "USER", 4 );
#else
			mkln( 65, "USAGER", 6 );
#endif
			break;
		default:
			break;
	}
	mkln( 113, "DATE: ", 6 );
	tedit( (char *)&sysdt,"____/__/__",  line+cur_pos,R_LONG ); 
	cur_pos += 10;
	if( prnt_line()<0 )	return(-1);
	if( prnt_line()<0 )	return(-1);

	switch( key ){
		case BYTIME:	/* chronological order */
#ifdef ENGLISH
			mkln( 3, "TIME", 4 );
			mkln( 9, "TERML", 5 );
			mkln( 15, "<--USER-->", 10 );
			mkln( 27, "<--DATE-->", 10 );
			mkln( 40, "<-------FILE------->", 20 );
#else
			mkln( 3, "HEURE", 5 );
			mkln( 9, "TERML", 5 );
			mkln( 15, "<-USAGER->", 10 );
			mkln( 27, "<--DATE-->", 10 );
			mkln( 40, "<------DOSSIER----->", 20 );
#endif
			break;
		case BYFILE:	/* sorted on file# */
#ifdef ENGLISH
			mkln( 2, "F#", 2 );
			mkln( 7, "<-------FILE------->", 20 );
			mkln( 29, "TIME", 4 );
			mkln( 34, "TERML", 5 );
			mkln( 40, "<--USER-->", 10 );
			mkln( 51, "<--DATE-->", 10 );
#else
			mkln( 2, "#C", 2 );
			mkln( 7, "<------DOSSIER----->", 20 );
			mkln( 29, "HEURE", 5 );
			mkln( 34, "TERML", 5 );
			mkln( 40, "<-USAGER->", 10 );
			mkln( 51, "<--DATE-->", 10 );
#endif
			/**
#ifdef ENGLISH
			mkln( 2, "<-------FILE------->", 20 );
			mkln( 25, "TIME", 4 );
			mkln( 31, "TERML", 5 );
			mkln( 38, "<--USER-->", 10 );
			mkln( 50, "<--DATE-->", 10 );
#else
			mkln( 2, "<------DOSSIER----->", 20 );
			mkln( 25, "HEURE", 5 );
			mkln( 31, "TERML", 5 );
			mkln( 38, "<-USAGER->", 10 );
			mkln( 50, "<--DATE-->", 10 );
#endif
			**/
			break;
		case BYUSER:	/* sorted on User-id# */
#ifdef ENGLISH
			mkln( 2, "<--USER-->", 10 );
			mkln( 14, "TERML", 5 );
			mkln( 22, "TIME", 4 );
			mkln( 28, "<-------FILE------->", 20 );
			mkln( 50, "<--DATE-->", 10 );
#else
			mkln( 2, "<-USAGER->", 10 );
			mkln( 14, "TERML", 5 );
			mkln( 22, "HEURE", 5 );
			mkln( 28, "<------DOSSIER----->", 20 );
			mkln( 50, "<--DATE-->", 10 );
#endif
			break;
	}
#ifdef ENGLISH
	mkln( 62, "PROGRAM-ID", 10);
	mkln( 75, "<-RECORD MAIN KEY VALUE->", 25);
	mkln( 104, "OPERATION", 9 );
#else
	mkln( 62, "ID DU PROGR.", 12);
	mkln( 75, "<-VALEUR CLE PRINC DE FICHE->", 29);
	mkln( 104, "  OPERATION", 11 );
#endif
	if( prnt_line()<0 )	return(-1);

#ifdef ENGLISH
	mkln( 12, "<--FIELD-->", 11 );
	mkln( 29, "<--------------V A L U E    A F T E R------------>", 50 );
	mkln( 81, "<--------------V A L U E   B E F O R E----------->", 50 );
#else
	mkln( 12, "<--CHAMP-->", 11 );
	mkln( 29, "<--------------V A L E U R   A P R E S----------->", 50 );
	mkln( 81, "<--------------V A L E U R   A V A N T----------->", 50 );
#endif
	if( prnt_line()<0 )	return(-1);
	if( prnt_line()<0 )	return(-1);
	return(0);
}
static
PrntRec( key )	/* Print the audit record details in two lines */
char key;
{
	char	filename[21];

	/* Print first line depending on key */
	if( (retval=Pr_Keyline(key))<0 )
		return(retval);

	if( aud_rec.mode==P_DEL )	/* return if DELETED mode */
		return(0);

	/* 	print the following in the second line 

		name of the field which was changed
		old value of the field unless the record was added
		new value of the field unless the record was deleted
	*/

	/*
		Get the filename from the file number and field name from
		the field number 
	*/
	getflnm( (int)aud_rec.file_no, filename );
	if( fld_hdr[aud_rec.file_no].filenm[0]=='\0' )
		if( GetFields( filename, &fld_hdr[aud_rec.file_no], 
				&fields[aud_rec.file_no], e_mesg )<0 ){
			printf(e_mesg);
			return(-1);
		}
	mkln( 12, (fields[aud_rec.file_no] + aud_rec.field_no-1)->name, 15 );

	mkln( 29, aud_rec.new_value, 50 );
	if( aud_rec.mode!=ADD )
		mkln( 81, aud_rec.old_value, 50 );
	if( prnt_line()<0 )	return(-1);

	return(0);
}

Pr_Keyline( key )
char	key;
{
	/* 	Print the following in the first line

		terminal from where the change was made
		id of the user who changed the record
		date of change, 
		time of change,
		name of file to which the record belongs
		id of the program through which change was made
		main key value of the record which was changed
		change made i.e, whether a record was 
			added  or  changed  or deleted
		the order depending on key
	*/
    switch( key ){
	case BYTIME:	/* Report on chronological order */
		return( T_repline() );
	case BYFILE:	/* Report on file# order */
		return( F_repline() );
	case BYUSER:	/* Report ordered on user id */
		return( U_repline() );
	default:
		return(0);
    }
}
T_repline()	/* Report on chronological order */
{
	char filename[21];
	short	firstlinewritten = 0;

	if( old_rec.run_time!=aud_rec.run_time){	
		mkln( 1, " ", 1 );
		tedit( (char *)&aud_rec.run_time, "__:__", 
					line+cur_pos, R_SHORT );
		old_rec.run_time = aud_rec.run_time;
		cur_pos += 5;
		firstlinewritten = 1;
	}
	if( strcmp(old_rec.terminal,aud_rec.terminal) ){	
		mkln( 10, aud_rec.terminal, 4 );
		STRCPY( old_rec.terminal,aud_rec.terminal );
		firstlinewritten = 1;
	}
	if( strcmp(old_rec.user_id,aud_rec.user_id) ){	
		mkln( 15, aud_rec.user_id, 10 );
		STRCPY( old_rec.user_id,aud_rec.user_id );
		firstlinewritten = 1;
	}
	if( old_rec.run_date!=aud_rec.run_date){	
		mkln( 26, " ", 1 );
		tedit( (char *)&aud_rec.run_date, "____/__/__", 
					line+cur_pos, R_LONG );
		old_rec.run_date = aud_rec.run_date;
		cur_pos += 10;
		firstlinewritten = 1;
	}
	if( old_rec.file_no!=aud_rec.file_no){	
		getuserflnm( (int)aud_rec.file_no, filename );
		old_rec.file_no = aud_rec.file_no;
		mkln( 40, filename, 20 );
		firstlinewritten = 1;
	}
	if( strcmp(old_rec.program_id,aud_rec.program_id) ){	
		mkln( 62, aud_rec.program_id, 10 );
		STRCPY( old_rec.program_id,aud_rec.program_id );
		firstlinewritten = 1;
	}
	if( strcmp(old_rec.rec_key,aud_rec.rec_key) ){	
		mkln( 75, aud_rec.rec_key, 25 );
		STRCPY( old_rec.rec_key,aud_rec.rec_key );
		firstlinewritten = 1;
	}
	if( old_rec.mode!=aud_rec.mode){	
		switch( aud_rec.mode ){
			case ADD:
#ifdef ENGLISH
				mkln( 104, "ADDITION", 8 );
#else
				mkln( 104, "AJOUT", 5 );
#endif
				break;
			case UPDATE:
#ifdef ENGLISH
				mkln( 105, "CHANGE", 6 );
#else
				mkln( 105, "CHANGEMENT", 10 );
#endif
				break;
			case P_DEL:
#ifdef ENGLISH
				mkln( 104, "DELETION", 8 );
#else
				mkln( 104, "ELIMINATION", 11 );
#endif
				break;
			default:
				break;
		}
		old_rec.mode = aud_rec.mode;
		firstlinewritten = 1;
	}
	if( firstlinewritten  )
		return( prnt_line() );
	else
		return(0);
}
F_repline()	/* Report on file# order */
{
	char filename[21];
	short	firstlinewritten = 0;
	short	file_no;

	if( old_rec.file_no!=aud_rec.file_no){	
		getuserflnm( (int)aud_rec.file_no, filename );
		old_rec.file_no = aud_rec.file_no;
		file_no = aud_rec.file_no + 1;
		mkln( 1, " ", 1 );
		tedit((char *)&file_no,"0_",line+cur_pos,R_SHORT) ;
		cur_pos += 2;
		mkln( 7, filename, 20 );
		firstlinewritten = 1;
	}
	if( old_rec.run_time!=aud_rec.run_time){	
		mkln( 27, " ", 1 );
		tedit( (char *)&aud_rec.run_time, "__:__", 
					line+cur_pos, R_SHORT );
		old_rec.run_time = aud_rec.run_time;
		cur_pos += 5;
		firstlinewritten = 1;
	}
	if( strcmp(old_rec.terminal,aud_rec.terminal) ){	
		mkln( 36, aud_rec.terminal, 3 );
		STRCPY( old_rec.terminal,aud_rec.terminal );
		firstlinewritten = 1;
	}
	if( strcmp(old_rec.user_id,aud_rec.user_id) ){	
		mkln( 40, aud_rec.user_id, 10 );
		STRCPY( old_rec.user_id,aud_rec.user_id );
		firstlinewritten = 1;
	}
	if( old_rec.run_date!=aud_rec.run_date){	
		mkln( 50, " ", 1 );
		tedit( (char *)&aud_rec.run_date, "____/__/__", 
					line+cur_pos, R_LONG );
		old_rec.run_date = aud_rec.run_date;
		cur_pos += 10;
		firstlinewritten = 1;
	}
	if( strcmp(old_rec.program_id,aud_rec.program_id) ){	
		mkln( 62, aud_rec.program_id, 10 );
		STRCPY( old_rec.program_id,aud_rec.program_id );
		firstlinewritten = 1;
	}
	if( strcmp(old_rec.rec_key,aud_rec.rec_key) ){	
		mkln( 75, aud_rec.rec_key, 25 );
		STRCPY( old_rec.rec_key,aud_rec.rec_key );
		firstlinewritten = 1;
	}
	if( old_rec.mode!=aud_rec.mode){	
		switch( aud_rec.mode ){
			case ADD:
#ifdef ENGLISH
				mkln( 104, "ADDITION", 8 );
#else
				mkln( 104, "AJOUT", 5 ); 
#endif
				break;
			case UPDATE:
#ifdef ENGLISH
				mkln( 105, "CHANGE", 6 );
#else
				mkln( 105, "CHANGEMENT", 10 );
#endif
				break;
			case P_DEL:
#ifdef ENGLISH
				mkln( 104, "DELETION", 8 );
#else
				mkln( 104, "ELIMINATION", 11 );
#endif
				break;
			default:
				break;
		}
		old_rec.mode = aud_rec.mode;
		firstlinewritten = 1;
	}
	if( firstlinewritten  )
		return( prnt_line() );
	else
		return(0);
}
U_repline()	/* Report ordered on user id */
{
	char filename[21];
	short	firstlinewritten = 0;

	if( strcmp(old_rec.user_id,aud_rec.user_id) ){	
		mkln( 2, aud_rec.user_id, 10 );
		STRCPY( old_rec.user_id,aud_rec.user_id );
		firstlinewritten = 1;
	}
	if( strcmp(old_rec.terminal,aud_rec.terminal) ){	
		mkln( 14, aud_rec.terminal, 3 );
		STRCPY( old_rec.terminal,aud_rec.terminal );
		firstlinewritten = 1;
	}
	if( old_rec.run_time!=aud_rec.run_time){	
		mkln( 20, " ", 1 );
		tedit( (char *)&aud_rec.run_time, "__:__", 
					line+cur_pos, R_SHORT );
		old_rec.run_time = aud_rec.run_time;
		cur_pos += 5;
		firstlinewritten = 1;
	}
	if( old_rec.file_no!=aud_rec.file_no){	
		getuserflnm( (int)aud_rec.file_no, filename );
		old_rec.file_no = aud_rec.file_no;
		mkln( 28, filename, 20 );
		firstlinewritten = 1;
	}
	if( old_rec.run_date!=aud_rec.run_date){	
		mkln( 49, " ", 1 );
		tedit( (char *)&aud_rec.run_date, "____/__/__", 
					line+cur_pos, R_LONG );
		old_rec.run_date = aud_rec.run_date;
		cur_pos += 10;
		firstlinewritten = 1;
	}
	if( strcmp(old_rec.program_id,aud_rec.program_id) ){	
		mkln( 62, aud_rec.program_id, 10 );
		STRCPY( old_rec.program_id,aud_rec.program_id );
		firstlinewritten = 1;
	}
	if( strcmp(old_rec.rec_key,aud_rec.rec_key) ){	
		mkln( 75, aud_rec.rec_key, 25 );
		STRCPY( old_rec.rec_key,aud_rec.rec_key );
		firstlinewritten = 1;
	}
	if( old_rec.mode!=aud_rec.mode){	
		switch( aud_rec.mode ){
			case ADD:
#ifdef ENGLISH
				mkln( 104, "ADDITION", 8 );
#else
				mkln( 104, "AJOUT", 5 );
#endif
				break;
			case UPDATE:
#ifdef ENGLISH
				mkln( 105, "CHANGE", 6 );
#else
				mkln( 105, "CHANGEMENT", 10 );
#endif
				break;
			case P_DEL:
#ifdef ENGLISH
				mkln( 104, "DELETION", 8 );
#else
				mkln( 104, "ELIMINATION", 11 );
#endif
				break;
			default:
				break;
		}
		old_rec.mode = aud_rec.mode;
		firstlinewritten = 1;
	}
	if( firstlinewritten  )
		return( prnt_line() );
	else
		return(0);
}
static
CreateIndex( key,tempindxnm )	/* Create temporary index & data file */
char key;
char *tempindxnm;	/* temporary index file name */
{
	int	keysarray[30];
	int i = 0;

	
	switch (key){
		case BYUSER:	/* user id */
			keysarray[i++] = 6;	/* no of parts in key */
						/* part 1 */
			keysarray[i++] = CHAR;
			keysarray[i++] = 10;
			keysarray[i++] = (char *)aud_rec.user_id
				      -(char *)&aud_rec;
			keysarray[i++] = ASCND;
						/* part 2 */
			keysarray[i++] = SHORT;
			keysarray[i++] = 1;
			keysarray[i++] = (char *)&aud_rec.run_time
				      -(char *)&aud_rec;
			keysarray[i++] = ASCND;
						/* part 3 */
			keysarray[i++] = SHORT;
			keysarray[i++] = 1;
			keysarray[i++] = (char *)&aud_rec.file_no
				      -(char *)&aud_rec;
			keysarray[i++] = ASCND;
						/* part 4 */
			keysarray[i++] = SHORT;
			keysarray[i++] = 1;
			keysarray[i++] = (char *)&aud_rec.mode
				      -(char *)&aud_rec;
			keysarray[i++] = ASCND;
						/* part 5 */
			keysarray[i++] = CHAR;
			keysarray[i++] = 25;
			keysarray[i++] = (char *)aud_rec.rec_key
				      -(char *)&aud_rec;
			keysarray[i++] = ASCND;
						/* part 6 */
			keysarray[i++] = SHORT;
			keysarray[i++] = 1;
			keysarray[i++] = (char *)&aud_rec.field_no
				      -(char *)&aud_rec;
			keysarray[i++] = ASCND;
			break;

		case BYFILE:	/* file # */

			keysarray[i++] = 5;	/* no of parts in key */
						/* part 1 */
			keysarray[i++] = SHORT;
			keysarray[i++] = 1;
			keysarray[i++] = (char *)&aud_rec.file_no
				      -(char *)&aud_rec;
			keysarray[i++] = ASCND;
						/* part 2 */
			keysarray[i++] = SHORT;
			keysarray[i++] = 1;
			keysarray[i++] = (char *)&aud_rec.run_time
				      -(char *)&aud_rec;
			keysarray[i++] = ASCND;
						/* part 3 */
			keysarray[i++] = SHORT;
			keysarray[i++] = 1;
			keysarray[i++] = (char *)&aud_rec.mode
				      -(char *)&aud_rec;
			keysarray[i++] = ASCND;
						/* part 4 */
			keysarray[i++] = CHAR;
			keysarray[i++] = 25;
			keysarray[i++] = (char *)aud_rec.rec_key
				      -(char *)&aud_rec;
			keysarray[i++] = ASCND;
						/* part 5 */
			keysarray[i++] = SHORT;
			keysarray[i++] = 1;
			keysarray[i++] = (char *)&aud_rec.field_no
				      -(char *)&aud_rec;
			keysarray[i++] = ASCND;
			break;
		default:
			break;
	}

	/* Get a handle on the data file through new index */
	/* NULL is dummy ptr to a record validating fn() that returns integer */
	i = CrtTmpIndx( AUDIT, TMPINDX_1, keysarray, tempindxnm,
						( int (*)() )NULL, e_mesg );
	if( i<0 ){
		printf( e_mesg );
		return(-1);
	}
	return(0);
}

#ifndef	ORACLE
/* Delete the temporary index and data file created */
static
DeleteIndex( tempindxnm )
char *tempindxnm;
{
	char	path_n_file[50];

	/* form the logical filename including the path */
	form_f_name( tempindxnm,path_n_file );

	/* Check for its existence and delete it */
	if( access(path_n_file,RDMODE)==0 )
		unlink( path_n_file );

	/* Check for the existence of the index file and delete it */
	strcat (path_n_file, ".IX");
	if( access(path_n_file,RDMODE)==0 )
		unlink( path_n_file );
	return(0);
}
#endif

