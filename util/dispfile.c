/*
*	dispfile.c
*
*	Program to display field contents of given files using .DEF file.
*/

#define	MAIN
#define SYSTEM		"UTILITIES"
#define MOD_DATE	"23-JAN-90"

#include <stdio.h>
#include <bfs_defs.h>
#include <isnames.h>
#include <fld_defs.h>
#include <filein.h>

#define	FIELD	(big_buff+cur_fld->offset)

static	char	*big_buff = NULL , *old_buff;

static	char	c_mesg[100];

static	int	file_no, rec_or_key_no ;

static	int	code;

static	Fld_hdr	hdr ;
static	Field	*field  = NULL ;

#ifndef	ORACLE
extern	long	last_posn;
#endif
extern	f_id	id_array[] ;
extern	int	*keysarray ;

static	int	pending_trans = 0 ;

/*** Define functions and prototypes needed for new compilter ***/
static int ask_key(char *, int, int, char *);
static int SeqIsam(void);
static int RandIsam(void);
static int DispIsam(void);
static int AddRecord(void);
static int DispSeq(void);
static int rand_read(void);
static int seq_read(void);
static int disp_rec(void);
static int read_flds(int);

main(argc, argv)
int	argc ;
char	*argv[] ;
{
	int	i, j, cur_file ;
	char	filenm[50] ;

	/*
	*	Initialize DBH
	*/

	strncpy(SYS_NAME, SYSTEM, 50);
	strncpy(CHNG_DATE, MOD_DATE, 10);
	proc_switch(argc, argv, -1) ;

	if ( dist_no[0] == '\0' )
		set_dist() ;/* Ask district# to position in data directory */

#ifdef	SECURITY
	i = GetUserClass(c_mesg);
	if(i < 0 || (i != SUPERUSER && i != ADMINISTRATOR)) {
		if(i == DBH_ERR) 
			printf("\n%s\n",c_mesg);
		else
			printf("\n\n\n\tACCESS DENIED\n");
		close_dbh() ;
		exit(-1);
	}
#endif

	if(init_dbh() < 0) {
		printf("DBH Initialization ERROR.. DBERROR: %d\n",dberror);
		close_dbh() ;
		exit(-1) ;
	}

	/* Find out Max Reclen */
	j = 0 ;
	for(i = 0 ; i < TOTAL_FILES ; i++)
		if(j < getreclen(i)) j = getreclen(i) ;

	big_buff = malloc((unsigned) j) ;
	old_buff = malloc((unsigned) j) ;
	if(big_buff == NULL || old_buff==NULL) {
		printf("Memory Allocation Error... size: %d\n", j);
		close_dbh() ;
		exit(-1);
	}

	cur_file = -1 ;

	for( ; ; ) {
		printf("\n\n\t\t\t    DISPLAY FILE CONTENTS\n\n");

		/*
		*	Display File Names
		*/

		if(TOTAL_FILES > 18)
			j = TOTAL_FILES / 2 + TOTAL_FILES % 2 ;
		else
			j = TOTAL_FILES ;

		printf("\t 0. QUIT\n");

		for(i = 0 ; i < j ; i++) {
		    printf("\t%2d. %-20.20s",(i+1),id_array[i].fl_user_nm) ;
		    if(j != TOTAL_FILES && (j+i) < TOTAL_FILES)
			printf("\t%2d. %s\n",(j+i+1),id_array[j+i].fl_user_nm);
		    else
			printf("\n");
		}

		do {
			printf("\n\t\tSelect File#: ");
			scanf("%d",&file_no) ;
		} while (file_no < 0 || file_no > TOTAL_FILES) ;

		if(file_no == 0) break ;	/* QUIT */

		file_no-- ;

		printf("\n\t\t***  %s  ***  Data File Selected\n\n",
			id_array[file_no].id_f_name) ;

		if(cur_file != file_no) {
			if(cur_file >= 0) {
				if ( field != NULL ) {
					free((char*)field) ;
					field = NULL ;
				}
				close_file(cur_file) ;
			}

			getflnm(file_no, filenm) ;
			if(GetFields(filenm, &hdr, &field, c_mesg) == ERROR) {
				printf("%s\n",c_mesg) ;
				break ;
			}
			cur_file = file_no ;
		}

		do {
			printf("\n\t\t\t0. QUIT\n");
			printf("\t\t\t1. ADD Mode\n");
			printf("\t\t\t2. VIEW/UPDATE Mode\n");

			printf("\n\t\t\tSelect Mode: ");
			scanf("%d",&i);
		} while (i < 0 || i > 2) ;

		switch(i) {
		case	1 :
			AddRecord() ;
			break ;
		case	2 :
			if(id_array[file_no].id_f_type == SEQ)
				DispSeq() ;
			else
				DispIsam();
		}
	}

	if ( field != NULL )
		free((char*)field) ;
	if ( big_buff != NULL )
		free(big_buff);
	if ( old_buff != NULL )
		free(old_buff);

	if ( pending_trans   ) {
		printf("Uncommitted Transactions.. COmmit/Rollback ..?(C/R): ");
		scanf("%s", c_mesg) ;
		if ( c_mesg[0] == 'C' || c_mesg[0] == 'c' )  
			code = commit( c_mesg ) ;
		else 
			code = roll_back( c_mesg ) ;

		if ( code == ERROR ) printf( "%s\n", c_mesg ) ;
	}

	close_dbh() ;
	exit(1);
}
/*--------------------------------------------------------------------*/
static	int
AddRecord()
{
	scpy( old_buff, big_buff, id_array[file_no].reclen );

	read_flds(ADD);

	if(id_array[file_no].id_f_type == SEQ)
		code = put_rec(big_buff,ADD,file_no,rec_or_key_no,c_mesg) ;
	else
		code = put_isrec(big_buff, ADD, file_no, c_mesg);

/*	if( code>=0 )
		code=rite_audit( file_no, ADD, big_buff, old_buff, c_mesg );*/
/* audit function updated no screen structure to pass to it */

	if(code < 0){
		printf("\n** ERROR **  Code: %d Dberror: %d Iserror: %d Errno: %d\n\t%s\n",
				code,dberror,iserror,errno,c_mesg);
		return(-1);
	}

	printf("\nCommit Now(Y/N)? ");
	read(0,c_mesg,10);
	c_mesg[0] = toupper(c_mesg[0]);
	if(c_mesg[0] == 'Y') {
		if(commit(c_mesg) < 0)
			printf("%s\n",c_mesg) ;
		pending_trans = 0 ;
	}
	else
		pending_trans++ ;

	return(0);
}
/*--------------------------------------------------------------------*/
static	int
DispIsam()
{
	int	seq_rand ;

	for ( ; ; ){
		printf("\n\t\t\tSelect Key#(0-%d): ",
			id_array[file_no].tot_keys - 1);
		scanf("%d",&rec_or_key_no);

		if(rec_or_key_no < 0 || rec_or_key_no >= id_array[file_no].tot_keys)
			continue;

		printf("\n\t\t\t0. QUIT\n");
		printf("\t\t\t1. Random Mode\n");
		printf("\t\t\t2. Sequential Mode\n");

		printf("\n\t\t\tSelect Mode: ");
		scanf("%d",&seq_rand);

		if(seq_rand == 0)break;

		switch(seq_rand){
		case 1:
			RandIsam();
			break;
		case 2:
			SeqIsam();
			break;
		default :
			printf("\t\t\t*** Invalid Mode ***\n");
			continue;
		}
	}

	return(0);
}
/*-------------------------------------------------------------*/
static int
RandIsam()
{
	for( ; ; ){
		printf("\n\n\tGive Key Parts:\n\n");
		if(ask_key(big_buff,file_no,rec_or_key_no,c_mesg) < 0){
			printf("%s\n",c_mesg);
			return(ERROR);
		}
		code = get_isrec(big_buff,file_no,rec_or_key_no,BROWSE,c_mesg);
		if(code < 0)
			printf("\n** ERROR **  Code: %d Dberror: %d Iserror: %d Errno: %d\n\t%s\n",
				code,dberror,iserror,errno,c_mesg);
		else {
			form_key(big_buff,file_no,rec_or_key_no,c_mesg);
			fprintf(stderr,"\nKEY: %s",c_mesg);

			disp_rec();
		}

		printf("\nContinue(Y/N)? ");
	    	read(0,c_mesg,10);
		c_mesg[0] = toupper(c_mesg[0]);
		if(c_mesg[0] != 'N')continue;
		break;
	}
	return(NOERROR);
}
/*-------------------------------------------------------------*/
static int
SeqIsam()
{
	int	direction;

	do {
		printf("\n\t\tGive Direction:\n\n");
		printf("\t\t\t0. Forward\n");
		printf("\t\t\t1. Backward\n");

		printf("\n\t\t\tDirection: ");
		scanf("%d",&direction);
	}while (direction < 0 || direction > 1);

	printf("\n\n\tGive Start Key Parts:\n\n");
	if(ask_key(big_buff,file_no,rec_or_key_no,c_mesg) < 0){
		printf("%s\n",c_mesg);
		return(ERROR);
	}
	flg_reset(file_no);
	for( ; ; ){
	    code = get_next(big_buff,file_no,rec_or_key_no,direction,BROWSE,c_mesg);
	    if(code < 0)
		printf("\n** ERROR **  Code: %d Dberror: %d Iserror: %d Errno: %d\n\t%s\n",
			code,dberror,iserror,errno,c_mesg);
	    else {
		form_key(big_buff,file_no,rec_or_key_no,c_mesg);
		fprintf(stderr,"\nKEY: %s",c_mesg);

		disp_rec();
	    }

	    printf("\nContinue(Y/N)? ");
	    read(0,c_mesg,10);
	    c_mesg[0] = toupper(c_mesg[0]);
	    if(c_mesg[0] != 'N')continue;
#ifndef	ORACLE
	    seq_over(file_no) ;
#endif
	    break;
	}
	return(NOERROR);
}
/*-------------------------------------------------------------*/
/* for a given file no & key no ask each key part form user */

static int
ask_key ( rec, file_no, key_no, key_str)
char	*rec;		/* data record */
int	file_no;	/* File no */
int	key_no;		/* Key No. */
char	*key_str ;	/* Key will be returned in this */
{
	int    i, j, k, parts , *k_array ;
	char	t_buff[128];
	Field	*cur_fld ;

	/* validate file_no & key_no */
	if(file_no < 0 || file_no >= TOTAL_FILES){
		sprintf(key_str,"Unknown File: %d",file_no);
		return(-1);
	}
	if(key_no < 0 || key_no >= id_array[file_no].tot_keys){
		sprintf(key_str,"Unknown Key:%d",key_no);
		return(-1);
	}
	/* Calculate 'i' to position the keysarray for a given key_no */
	/* For main key (key_no = 0) i will be 0 */

	for(j = 0, i = 0; j < key_no ; j++){
		/* No of parts of previous key * 4 + 1 */
		i += (keysarray[id_array[file_no].keys_offset + i] * 4) + 1;
	}
	parts = keysarray[id_array[file_no].keys_offset+i]; /* NO of parts in the key */

	/* Take each part of the key and ask */

	i++;	/* Position at ist part */
	for ( j = 0 ; j < parts ; j++, i += 4) {

		k_array = &(keysarray[id_array[file_no].keys_offset + i]);

		/* Find out the Fld name using offset */
		cur_fld = field ;
		for(k = 0 ; k < hdr.no_fields ; k++ , cur_fld++ )
			if(cur_fld->offset == k_array[2]) break ;

		/***
		printf("\tPart %d (Size %2d) ",j,k_array[1]);
		**/
		printf("\tPart-%d: %s[%d]\t", j, cur_fld->name, k_array[1]);

		switch( k_array[0] ) {	/* TYpe of part */

		case DATE :
			switch(DATE_TYPE){
			case DDMMYY:
				printf("(DDMMYY)  :");
				break;
			case MMDDYY:
				printf("(MMDDYY)  :");
				break;
			case YYMMDD:
				printf("(YYMMDD)  :");
				break;
			case DDMMYYYY:
				printf("(DDMMYYYY):");
				break;
			case MMDDYYYY:
				printf("(MMDDYYYY):");
				break;
			case YYYYMMDD:
				printf("(YYYYMMDD):");
				break;
			}
			for ( k = 0 ; k < k_array[1] ; k++)
			    scanf("%ld",((long*)(rec+k_array[2]+k*sizeof(long))));
			continue;
		case LONG :
			printf("(LONG)    : ");
			for ( k = 0 ; k < k_array[1] ; k++)
			    scanf("%ld",((long*)(rec+k_array[2]+k*sizeof(long))));
			continue ;
		 case FLOAT :
			printf("(FLOAT)   : ");
			for ( k = 0 ; k < k_array[1] ; k++)
			    scanf("%f",((float*)(rec+k_array[2]+k*sizeof(float))));
			continue ;
		 case DOUBLE :
			printf("(DOUBLE)  : ");
			for ( k = 0 ; k < k_array[1] ; k++)
			    scanf("%lf",((double*)(rec+k_array[2]+k*sizeof(double))));
			continue ;
		 case SHORT :
			printf("(SHORT)   : ");
			for ( k = 0 ; k < k_array[1] ; k++)
			    scanf("%hd",((short*)(rec+k_array[2]+k*sizeof(short))));
			continue ;
		 case CHAR :
			printf("(CHAR)    : ");
			fflush(stdout);
			k = read(0,t_buff,100);
			t_buff[k-1] = '\0' ;	/* Replace \n */
			strncpy((rec+k_array[2]),t_buff,k_array[1]);
			continue ;
	  	 default :
			strcpy(key_str,"Illegal type...");
			return(-1) ;
		}
	}
	return(0);
}
/*-----------------------------------------------------------------*/
static	int
DispSeq()
{
	int	seq_rand;	/* 1 - Random Mode , 2 - Seq mode */

	for ( ; ; ){
		printf("\n\t\t\t0. QUIT\n");
		printf("\t\t\t1. Random Mode\n");
		printf("\t\t\t2. Sequential Mode\n");

		printf("\n\t\t\tSelect Mode: ");
		scanf("%d",&seq_rand);

		if(seq_rand == 0)break;

		switch(seq_rand){
		case 1:
			rand_read();
			break;
		case 2:
			seq_read();
			break;
		default :
			printf("\t\t\t*** Invalid Mode ***\n");
			continue;
		}
	}

	return(0);
}
/*-------------------------------------------------------------*/
static int
rand_read()
{
	for( ; ; ){
		printf("\n\n\tGive Rec#: ");
		scanf("%d",&rec_or_key_no) ;

		code = get_rec(big_buff,file_no,rec_or_key_no,BROWSE,c_mesg);
		if(code < 0)
			printf("\n** ERROR **  Code: %d Dberror: %d Iserror: %d Errno: %d\n\t%s\n",
				code,dberror,iserror,errno,c_mesg);
		else {
#ifndef	ORACLE
			last_posn = ((code-1) * id_array[file_no].reclen) ;
#endif
			fprintf(stderr,"\nRecord#: %d",rec_or_key_no);
			disp_rec();
		}

		printf("\nContinue(Y/N)? ");
	    	read(0,c_mesg,10);
		c_mesg[0] = toupper(c_mesg[0]);
		if(c_mesg[0] != 'N')continue;
		break;
	}
	return(NOERROR);
}
/*-------------------------------------------------------------*/
static int
seq_read()
{
	int	direction ;

	do {
		printf("\n\t\tGive Direction:\n\n");
		printf("\t\t\t0. Forward\n");
		printf("\t\t\t1. Backward\n");

		printf("\n\t\t\tDirection: ");
		scanf("%d",&direction);
	}while (direction < 0 || direction > 1);

	printf("\n\tGive Start Rec#: ");
	scanf("%d",&rec_or_key_no) ;

	flg_reset(file_no);
	for( ; ; ){
		code = get_seqrec(big_buff, file_no, rec_or_key_no, direction, BROWSE, c_mesg) ;
		if(code < 0)
			printf("\n** ERROR **  Code: %d Dberror: %d Iserror: %d Errno: %d\n\t%s\n",
			code,dberror,iserror,errno,c_mesg);
		else {
			rec_or_key_no = code ;
#ifndef	ORACLE
			last_posn = ((rec_or_key_no-1) * id_array[file_no].reclen) ;
#endif
			fprintf(stderr,"\nRecord#: %d",rec_or_key_no);
			disp_rec();
		}

		printf("\nContinue(Y/N)? ");
		read(0,c_mesg,10);
		c_mesg[0] = toupper(c_mesg[0]);
		if(c_mesg[0] == 'N')break ;
	}
	return(NOERROR);
}
/*-------------------------------------------------------------*/
static int
disp_rec()
{
	int	j, mode ;
	Field	*cur_fld ;

	fprintf(stderr,"\n");

	cur_fld = field ;

	for(j = 0 ; j < hdr.no_fields ; j++ , cur_fld++ ) {

		fprintf(stderr,"    :%s: ",cur_fld->name);
		switch ( cur_fld->type ) {
		case T_CHAR :
			fprintf(stderr,cur_fld->format,FIELD);
			break ;
		case T_SHORT:
			fprintf(stderr,cur_fld->format,*(short *)(FIELD));
			break ;
		case T_INT:
			fprintf(stderr,cur_fld->format,*(int *)(FIELD));
			break ;
		case T_LONG :
			fprintf(stderr,cur_fld->format,*(long *)(FIELD));
			break ;
		case T_FLOAT :
			fprintf(stderr,cur_fld->format,*(float *)(FIELD));
			break ;
		case T_DOUBLE :
			fprintf(stderr,cur_fld->format,*(double *)(FIELD));
			break ;
		default :
			fprintf(stderr,"\nDisp_rec(): Invalid TYpe: %c\n",cur_fld->type);
		} /** switch ***/
		
		if(j % 2 == 1) fprintf(stderr,"\n");
	} /* for {} */

	if(id_array[file_no].id_f_type == SEQ)
		printf("\nDo You Want To Update This Record (Y/N)? ");
	else
		printf("\nDo You Want To CHANGE/DELETE This Record (C/D/N)? ");
	fflush(stdout);
	read(0,c_mesg,10);
	c_mesg[0] = toupper(c_mesg[0]);

	if(id_array[file_no].id_f_type == SEQ) {
		if(c_mesg[0] != 'Y') return(0);
	}
	else {
		if(c_mesg[0] == 'C')
			mode = UPDATE ;
		else if(c_mesg[0] == 'D')
			mode = P_DEL ;
		else
			return(0);
	}

	/* Lock the Record in multi user. This is to test the multi user
	   environment */
	if(id_array[file_no].id_f_type == SEQ)
		code = get_rec(big_buff,file_no,rec_or_key_no,UPDATE,c_mesg) ;
	else
		code = get_isrec(big_buff,file_no,rec_or_key_no,UPDATE,c_mesg) ;

	if(code <  0){
		printf("\n** ERROR **  Code: %d Dberror: %d Iserror: %d Errno: %d\n\t%s\n",
				code,dberror,iserror,errno,c_mesg);
		return(-1);
	}

	scpy( old_buff, big_buff, id_array[file_no].reclen );

	if(id_array[file_no].id_f_type == SEQ || mode == UPDATE)
		read_flds(UPDATE);

	if(id_array[file_no].id_f_type == SEQ)
		code = put_rec(big_buff,UPDATE,file_no,rec_or_key_no,c_mesg) ;
	else
		code = put_isrec(big_buff, mode, file_no, c_mesg);

/*	if( code>=0 )
		code=rite_audit( file_no, ADD, big_buff, old_buff, c_mesg );*/
/* audit function updated no screen structure to pass to it */

	if(code < 0){
		printf("\n** ERROR **  Code: %d Dberror: %d Iserror: %d Errno: %d\n\t%s\n",
				code,dberror,iserror,errno,c_mesg);
		return(-1);
	}

	printf("\nCommit Now(Y/N)? ");
	read(0,c_mesg,10);
	c_mesg[0] = toupper(c_mesg[0]);
	if(c_mesg[0] == 'Y') {
		if(commit(c_mesg) < 0)
			printf("%s\n",c_mesg) ;
		pending_trans = 0 ;
	}
	else
		pending_trans++ ;

#ifdef	XXXX
	if (lseek(getdatafd(file_no), last_posn, 0) < 0){
		printf("Seek Err\n");
		return(ERROR);
	}
	if(write(getdatafd(file_no), big_buff, id_array[file_no].reclen) <
				id_array[file_no].reclen){
		printf("Write Err\n");
		return(ERROR);
	}

#endif
	return(0);
}
/*-------------------------------------------------------------*/
static int
read_flds(mode)
int	mode ;
{
	int	i, j ;
	char	buf[100];
	Field	*cur_fld ;

	cur_fld = field ;

	if(mode == UPDATE)
		printf("\n\tENTER '.' to retain the PREVIOUS Value\n");

	for(j = 0 ; j < hdr.no_fields ; j++ , cur_fld++ ) {

		fprintf(stdout,"%s: ",cur_fld->name);
		fflush(stdout) ;
		/*
		scanf("%s",buf);
		*/
		for( ; ; )
			if((i = read(0, buf, 50 ))) break ;
		buf[i-1] = '\0' ;	/* Replace new line char */

		if (mode == UPDATE && buf[0] == '.') continue;
		switch ( cur_fld->type ) {
		case T_CHAR :
			strncpy(FIELD, buf, cur_fld->len);
			break ;
		case T_SHORT:
			sscanf(buf,"%hd",(short *)(FIELD));
			break ;
		case T_INT:
			sscanf(buf,"%d",(int *)(FIELD));
			break ;
		case T_LONG :
			sscanf(buf,"%ld",(long *)(FIELD));
			break ;
		case T_FLOAT :
			sscanf(buf,"%f",(float *)(FIELD));
			break ;
		case T_DOUBLE :
			sscanf(buf,"%lf",(double *)(FIELD));
			break ;
		} /** switch ***/
	} /* for {} */

	return(0);
}
/*--------------------E-N-D---O-F---F-I-L-E----------------------------*/

