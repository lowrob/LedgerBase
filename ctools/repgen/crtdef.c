
/****
#define DEBUG
***/

#include <stdio.h>
#include <string.h>
#include "rep.h"
#include "struct.h"
#include "repname.h"
#include <errno.h>

//extern	int	errno ;

/*
*	Maximum Input fields (MAX_FIELDS) allowed are 127 at the current 
*	value of STRBUFSIZE which is 2048.
*/

#define	MAX_FIELDS	(STRBUFSIZE-sizeof(INHDR))/sizeof(FLDINFO) 
#define	MAX_DIM		5

static 	FLDINFO	*elm ;
static 	INHDR 	*hdr ;
static 	char 	blk[STRBUFSIZE] ;
static	char	c_name[MAX_FIELDS][12];	/* Field names */
static	char	c_type[MAX_FIELDS] ;	/* Type of Field */
static	short 	c_pos[MAX_FIELDS]  ;	/* Offset of Field in Record */
static	short	c_len[MAX_FIELDS] ;	/* Field length for CHAR fields */

FILE	*fp = NULL,
	*fopen() ;
static	int	t_flds = 0 ;	/* Total Fields */


FILE *fopen() ;		/*  *fpin input file format */
static int fdout ;	/* ouput file descriptor */

static	int	array  ;

/*** Define functions and prototypes ***/
static int initialize(char *);
static int openfiles(char *);

main(argc,argv)
int argc ;
char **argv ;
{
	short fldcount ;
	char	proj_name[15] ;
	int	file_no ;
	int	rec_len ;
	char	ans[5] ;

	if(argc < 2) {
		fprintf(stderr,"Usage : %s def_file(s)\n", argv[0]);
		exit(-1) ;
	}
 
	printf(" Project Name :") ;
	scanf("%s", proj_name) ;

	printf(" Supress Arrays ?(y/n) :");
	scanf("%s", ans) ;

	if ( ans[0] == 'n' || ans[0] == 'N' ) 
		array = 1 ;
	else 
		array = 0 ;

	fdout = - 1;

	for( file_no = 1 ; file_no < argc ; file_no++) {

		rec_len = initialize(argv[file_no]) ;
		if(rec_len < 0) continue ;
		printf("Record: %s  Flds: %d  Length: %d\n", argv[file_no],
				t_flds, rec_len);

		if(fdout == -1)
			openfiles(proj_name) ;	/* Supressed fpin open */

		hdr= (INHDR *)(blk) ;
		elm= (FLDINFO *)(blk+sizeof(INHDR)) ;

		for(fldcount=0; fldcount < t_flds; fldcount++,elm++) {
			strcpy(elm->fldname, c_name[fldcount]) ;
			elm->typf= c_type[fldcount] ; 
			elm->posf= c_pos[fldcount] ;
			elm->lenf= c_len[fldcount] ; 
			}
		blkwrite(t_flds,rec_len,argv[file_no]) ;
		}
	close(fdout) ;
	exit(0) ;
}


blkwrite(nflds, siz, name)
short	siz,
	nflds ;
char	*name ;
{
	hdr->stsiz= siz ;
	hdr->stflds= nflds ;
	strcpy(hdr->stname,name) ;
	if (write(fdout,blk,STRBUFSIZE)<STRBUFSIZE){
		fprintf(stderr,"Write error occurred \n") ;
		fprintf(stderr,"Module : crtstr.c Func : blkwrite \n");
		exit(WRITEERR) ;
	}	
}


static int 
openfiles(projcode)
char *projcode ;
{
	char filename[FILE_NAME_LEN] ;

	strcpy(filename, projcode) ;
	strcat(filename, STRFILE) ;	/* structure definition file ALL */

	if((fdout = creat(filename,CRMODE)) == -1) {
		fprintf(stderr,"%s file creat error \n",filename) ;
		fprintf(stderr,"Module : crtstr.c Func : openfile \n");
		exit(CREATERR) ;
		}
		
	close(fdout) ;

	if((fdout = open(filename,RWMODE)) == -1) {
		fprintf(stderr,"%s file open-write error \n",filename) ;
		fprintf(stderr,"Module : rp.c. Func : inoutopen \n") ;
		exit(OPENERR) ;
		}

	return(NOERROR) ;
}


/*-----------------------------------------------------------------------*/

/* read *.def file and find out offsets of fields */

static int
initialize(fl_name)
char	*fl_name;		/* .def file */
{
	int	off_set ;
	short	len, i;
	char	f_type[10] ;
	char	f_name[20] ;
	int	bound, line_no ;
	char	path_name[30], line[80], *fgets() ;
	int	dimen[MAX_DIM] ;
	int	level ;

	sprintf(path_name,"%s.def",  fl_name) ;	

	if((fp=fopen(path_name,"r")) == NULL){
		printf("File Open Error...file:%s errno:%d\n", fl_name, errno);
		return(OPENERR) ;
	}

/*
*	Read Input file of format ._
*
*  	< field_type >	< field_name >	< Comments >
*
*	Process each field . If array is to be considered as one field then
*	only first element of the array ( 0th) is considered .
*/

	for( t_flds=0, line_no = 0 , off_set = 0 ; ; line_no++){
		if ( t_flds > MAX_FIELDS ) {
		    printf("Too Many Flds In :%s.. Allowed only %ld",
							fl_name,MAX_FIELDS);
		    return(OPENERR);
		}

		if ( fgets(line, 80, fp) == NULL ) break  ;
		if ( strlen(line) < 5 ) continue ;	
		sscanf(line, "%s%s", f_type, f_name ) ;

		if ( strcmp(f_type, "char") != 0  &&
			strcmp(f_type, "int") != 0  &&
			strcmp(f_type, "short") != 0  &&
			strcmp(f_type, "long") != 0  &&
			strcmp(f_type, "float") != 0  &&
			strcmp(f_type, "double") != 0 ) {
			printf(
			"Line :%3d: typ:%s nam:%s   *** IGNORED ***\n",
					line_no, f_type, f_name);
			continue ;
		}

		level = GetLevel(f_name, dimen) ;
		if(f_type[0] == 'c') {
			for(i = 0, len = 1 ; i < level - 1 ; i++)
				len *= dimen[i] ;
		}
		else {
			for(i = 0, len = 1 ; i < level ; i++)
				len *= dimen[i] ;
		}

		switch (f_type[0]) {
		case  'c' : 
			if ( array ) 
			   for(bound=0; bound<len;bound++){
				SET_OFFSET(off_set,ALGN_CHAR) ;
				c_pos[t_flds+bound] = off_set ;
				c_type[t_flds+bound] = CHAR ;
				c_len[t_flds+bound] = dimen[level - 1] ;
				off_set += dimen[level - 1] ; 
				}
			else  {
				SET_OFFSET(off_set,ALGN_CHAR) ;
				c_pos[t_flds] = off_set ;
				c_type[t_flds] = CHAR ;
				c_len[t_flds] = dimen[level - 1] ;
				off_set += len * dimen[level - 1] ; 
				len = 1  ; /* for character fields */
				}
			make_name(f_name, len, t_flds) ;
			break ;
		case  's' : 
			if ( array ) 
			   for(bound=0; bound<len;bound++){
				SET_OFFSET(off_set, ALGN_SHORT);
				c_pos[t_flds+bound] = off_set ;
				c_type[t_flds+bound] = SHORT ;
				c_len[t_flds+bound] = 1 ;
				off_set  += sizeof(short) ;
				}
			else {
				SET_OFFSET(off_set, ALGN_SHORT);
				c_pos[t_flds] = off_set ;
				c_type[t_flds] = SHORT ;
				c_len[t_flds] = 1 ;
				off_set  += sizeof(short)*len ;
				len  = 1 ;
			    }
			make_name(f_name, len, t_flds) ;
			break ;
		case  'i' :
			if ( array )
			    for(bound=0; bound<len;bound++){
				SET_OFFSET(off_set, ALGN_INT) ; 
				c_pos[t_flds+bound] = off_set ;
				c_type[t_flds+bound] = INT ;
				c_len[t_flds+bound] = 1 ;
				off_set  += sizeof(int)  ;
				}
			else {
				SET_OFFSET(off_set, ALGN_INT) ; 
				c_pos[t_flds] = off_set ;
				c_type[t_flds] = INT ;
				c_len[t_flds] = 1 ;
				off_set  += sizeof(int) * len   ;
				len = 1 ;
			     	}
			make_name(f_name, len, t_flds) ;
			break ;
		case  'l' :   
			if ( array )
			    for(bound=0; bound<len;bound++){
				SET_OFFSET(off_set, ALGN_LONG) ; 
				c_pos[t_flds+bound] = off_set ;
				c_type[t_flds+bound] = LONG  ;
				c_len[t_flds+bound] = 1 ;
				off_set  += sizeof(long) ;
				}
			else {
				SET_OFFSET(off_set, ALGN_LONG) ; 
				c_pos[t_flds] = off_set ;
				c_type[t_flds] = LONG  ;
				c_len[t_flds] = 1 ;
				off_set  += sizeof(long)  * len ;
				len = 1 ;
				}
			make_name(f_name, len, t_flds) ;
			break ;
		case  'f' :
			if ( array )
			   for(bound=0;bound<len;bound++){
				SET_OFFSET(off_set, ALGN_FLOAT) ;
				c_pos[t_flds+bound] = off_set ;
				c_type[t_flds+bound] = FLOAT ;
				c_len[t_flds+bound] = 1 ;
				off_set  += sizeof(float) ;
				}
			else {
				SET_OFFSET(off_set, ALGN_FLOAT) ;
				c_pos[t_flds] = off_set ;
				c_type[t_flds] = FLOAT ;
				c_len[t_flds] = 1 ;
				off_set  += sizeof(float) * len ;
				len = 1 ;
				}
			make_name(f_name, len, t_flds) ;
			break ;
		case  'd' :
			if ( array )
			   for(bound=0;bound<len;bound++){
				SET_OFFSET(off_set, ALGN_DOUBLE);
				c_pos[t_flds+bound] = off_set ;
				c_type[t_flds+bound] = DOUBLE ;
				c_len[t_flds+bound] = 1 ;
				off_set  += sizeof(double) ;
				}
			else {
				SET_OFFSET(off_set, ALGN_DOUBLE);
				c_pos[t_flds] = off_set ;
				c_type[t_flds] = DOUBLE ;
				c_len[t_flds] = 1 ;
				off_set  += sizeof(double)  * len ;
				len = 1 ;
				}
			make_name(f_name, len, t_flds) ;
			break ;
		default :
			printf(" Assuming comment and continuing \n"); 
			continue ;
		} /* switch */

		t_flds += len ;
	} /* for */

	SET_OFFSET(off_set, ALGN_STRUCT) ; 

	fclose(fp) ;
	return(off_set);	/* Retuen Record Length */
}

/*----------------------------------------------------------------------*/
/*	Returns the array bound value and returs the name without them. */

GetLevel(name, dimen) 
char	*name ;
int	*dimen ;
{
	int	array_found ;
	int	level ;

	level = 0;
	array_found  = 0 ;

	for ( ; *name && *name != ';' ; name++ ) {
		if ( !array_found ) {
			if ( *name == '[' ) {
				*name = '\0' ;		/* make name */
				array_found = 1 ;
				*dimen = 0 ;
			}
		}
		else  {
			if ( *name == ']' ) {
				level++ ;
				if(level == MAX_DIM) break ;
				dimen++;
				array_found  = 0 ;
			}
			else
				*dimen = (*dimen) * 10 + (*name - '0') ;
		}
	}
	if(*name == ';') *name = '\0' ;

	if(level == 0) {
		*dimen = 1 ;
		return(1) ;
	}
	
	return(level) ;	/* Level of dimension */
}

/*---------------------------------------*/
void
make_name(fld_name, len, fld_no)
char	*fld_name ;
int	len , fld_no ;
{
	int 	i ;
	char	name[15] ;

	if(len == 1) {
		strncpy(c_name[fld_no] , fld_name, L_NAME-1) ;
		c_name[fld_no][L_NAME-1] = '\0' ;
		return ;
	}

	for ( i = 0 ; i < len ; i++) {
		strcpy(name, fld_name) ;
		sprintf(name+strlen(name), "%d", (i+1)) ;
		strncpy(c_name[fld_no+i] , name, L_NAME-1) ;
		c_name[fld_no][L_NAME-1] = '\0' ;
	}
	return ;
}
	
/*--------------------E-N-D---O-F---F-I-L-E----------------------------*/

	
