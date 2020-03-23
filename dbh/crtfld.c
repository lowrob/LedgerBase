/*-----------------------------------------------------------------------
Source Name: crtfld.c
System     : General Utility.
Created  On: 8th May 89.
Created  By: T AMARENDRA.

Program to create a Field Definitions file for a given file names.

	
M O D I F I C A T I O N S :        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

------------------------------------------------------------------------*/

#define	MAIN

#define SYSTEM		"DBH INITIALIZATION"
#define MOD_DATE	"23-JAN-90"

#include <stdio.h>
#include <bfs_defs.h>
#include <fld_defs.h>
#include <filein.h>

extern	f_id	id_array[] ;

static	int	fd ;	/* DEF file decriptor */

main(argc, argv)
int	argc ;
char	*argv[] ;
{
	char	file_name[50];
	int	i ;

	/* Set the environment */
	strncpy(SYS_NAME, SYSTEM, 50);
	strncpy(CHNG_DATE, MOD_DATE, 10);
	proc_switch(argc, argv, -1) ;

	/******
	if(dist_no[0] == '\0')
		set_dist();
	*****/

	/* Intialize DBH */
	if(init_dbh() < 0) {
		printf("DBH Initialization ERROR.. DBERROR: %d\n",dberror);
		exit(-1) ;
	}

	/*** If file exists then delete and recreate ***/

	/***
	strcpy (file_name, FMT_PATH);
	***/
	strcpy (file_name, DATA_PATH);
	strcat (file_name, FLDDEF_FILE);

	/*** 
	form_f_name(FLDDEF_FILE, file_name) ;
	***/
	/***** If file exists then delete and recreate  ******/

	if ( access(file_name,0) >= 0 )
		unlink(file_name);

	/*** If file exists then delete and recreate  ***/

	if ( (fd=creat(file_name, CRMODE)) < 0 ) {
		printf("Create error In Id initialisation File:%s\n",file_name);
		exit(-1) ;
	}

	for(i = 0 ; i < TOTAL_FILES ; i++)
		CrtRecord(id_array[i].id_f_name);

	close(fd) ;
	close_dbh();
	exit(0);
}

/*-------------------------------------------------------------*/
/*
*  read given file DEF file and find out offsets of fields
*  and write to FLDDEF file.
*/

CrtRecord(file_nm)
char	*file_nm ;
{
	short	level ;
	char	f_type[10], f_name[25], f_line[81] ;
	int	dimen[MAX_DIM] ;
	int	i, f_cnt ;
	Fld_hdr	hdr ;
	Field	field ;
	long	h_addr ;
	FILE	*fp = NULL ;

	sprintf(f_line,"%s%s.def",FMT_PATH, file_nm);

	if((fp=fopen(f_line,"r")) == NULL){
		printf("%s.def\tFile Open Error\n",file_nm);
		return(ERROR) ;
	}

	/*
	*	Read Input file of format ._
	*
	*  	< field_type >	< field_name >	< Comments >
	*
	*/

	strcpy(hdr.filenm,file_nm) ;
	hdr.no_fields = 0 ;
	hdr.reclen = 0;

	if((h_addr = lseek(fd,0L,2)) < 0) {
		printf("Lseek Error\n");
		fclose(fp) ;
		return(ERROR) ;
	}

	if(write(fd,(char*)&hdr,sizeof(Fld_hdr)) < sizeof(Fld_hdr)) {
		printf("Header write error\n");
		fclose(fp);
		return(ERROR) ;
	}

	for( ; ; ) {

	    if ( fgets(f_line, 80, fp) == NULL ) break  ;
	    if ( strlen(f_line) < 5 ) continue ;	
	    sscanf(f_line, "%s%s", f_type, f_name ) ;

	    if(strcmp(f_type, "char") && strcmp(f_type, "int") &&
			strcmp(f_type, "short") && strcmp(f_type, "long") &&
			strcmp(f_type, "float") && strcmp(f_type, "double") ) {

		    printf("%s.def file not Proper.. Fld Type: %s\n",
			file_nm, f_type);
		    fclose(fp) ;
		    return(ERROR);
	    }

	    level = GetLevel(f_name, dimen) ;
	    if(strlen(f_name) >= sizeof(field.name))
		f_name[sizeof(field.name) - 1] = '\0' ;

	    field.len = 0 ;

	    switch (f_type[0]) {
	    case T_CHAR :
		for(i = 0, f_cnt = 1 ; i < level - 1 ; i++)
			f_cnt *= dimen[i] ;
		for(i = 0 ; i < f_cnt ; i++) {
			SET_OFFSET(hdr.reclen, ALGN_CHAR) ;
			field.offset = hdr.reclen ;
			field.type = T_CHAR ;
			field.len = dimen[level - 1] ;
			hdr.reclen += dimen[level - 1] ; 
			strcpy(field.name, f_name) ;
			if(f_cnt > 1)
				sprintf(field.name+strlen(f_name),
					"%d",(i+1)) ;
			if(field.len > 1)
				sprintf(field.format,"%%-%d.%ds",
					field.len-1, field.len-1);
			else
				strcpy(field.format, "%-1.1s");

			/* Write Field to file */
			if(write(fd,(char*)&field, sizeof(Field)) <
							sizeof(Field)) {
				printf("Field Write Error\n");
				fclose(fp);
				return(ERROR) ;
			}
			hdr.no_fields++;
		}
		break ;
	    case T_SHORT : 
		for(i = 0, f_cnt = 1 ; i < level ; i++)
			f_cnt *= dimen[i] ;
		for(i = 0 ; i < f_cnt ; i++) {
			SET_OFFSET(hdr.reclen, ALGN_SHORT) ;
			field.offset = hdr.reclen ;
			field.type = T_SHORT ;
			hdr.reclen  += sizeof(short) ;
			strcpy(field.name, f_name) ;
			if(f_cnt > 1)
				sprintf(field.name+strlen(f_name),
					"%d",(i+1)) ;
			strcpy(field.format,"%d");

			/* Write Field to file */
			if(write(fd,(char*)&field, sizeof(Field)) <
							sizeof(Field)) {
				printf("Field Write Error\n");
				fclose(fp);
				return(ERROR) ;
			}
			hdr.no_fields++;
		}
		break ;
	    case T_INT :
		for(i = 0, f_cnt = 1 ; i < level ; i++)
			f_cnt *= dimen[i] ;
		for(i = 0 ; i < f_cnt ; i++) {
			SET_OFFSET(hdr.reclen, ALGN_INT) ;
			field.offset = hdr.reclen ;
			field.type = T_INT ;
			hdr.reclen  += sizeof(int) ;
			strcpy(field.name, f_name) ;
			if(f_cnt > 1)
				sprintf(field.name+strlen(f_name),
					"%d",(i+1)) ;
			strcpy(field.format,"%d");

			/* Write Field to file */
			if(write(fd,(char*)&field, sizeof(Field)) <
							sizeof(Field)) {
				printf("Field Write Error\n");
				fclose(fp);
				return(ERROR) ;
			}
			hdr.no_fields++;
		}
		break ;
	    case T_LONG :
		for(i = 0, f_cnt = 1 ; i < level ; i++)
			f_cnt *= dimen[i] ;
		for(i = 0 ; i < f_cnt ; i++) {
			SET_OFFSET(hdr.reclen, ALGN_LONG) ;
			field.offset = hdr.reclen ;
			field.type = T_LONG  ;
			hdr.reclen  += sizeof(long) ;
			strcpy(field.name, f_name) ;
			if(f_cnt > 1)
				sprintf(field.name+strlen(f_name),
					"%d",(i+1)) ;
			strcpy(field.format,"%ld");

			/* Write Field to file */
			if(write(fd,(char*)&field, sizeof(Field)) <
							sizeof(Field)) {
				printf("Field Write Error\n");
				fclose(fp);
				return(ERROR) ;
			}
			hdr.no_fields++;
		}
		break ;
	    case T_FLOAT :
		for(i = 0, f_cnt = 1 ; i < level ; i++)
			f_cnt *= dimen[i] ;
		for(i = 0 ; i < f_cnt ; i++) {
			SET_OFFSET(hdr.reclen, ALGN_FLOAT) ;
			field.offset = hdr.reclen ;
			field.type = T_FLOAT ;
			hdr.reclen  += sizeof(float) ;
			strcpy(field.name, f_name) ;
			if(f_cnt > 1)
				sprintf(field.name+strlen(f_name),
					"%d",(i+1)) ;
			strcpy(field.format,"%.4f");

			/* Write Field to file */
			if(write(fd,(char*)&field, sizeof(Field)) <
							sizeof(Field)) {
				printf("Field Write Error\n");
				fclose(fp);
				return(ERROR) ;
			}
			hdr.no_fields++;
		}
		break ;
	    case T_DOUBLE :
		for(i = 0, f_cnt = 1 ; i < level ; i++)
			f_cnt *= dimen[i] ;
		for(i = 0 ; i < f_cnt ; i++) {
			SET_OFFSET(hdr.reclen, ALGN_DOUBLE) ;
			field.offset = hdr.reclen ;
			field.type = T_DOUBLE ;
			hdr.reclen  += sizeof(double) ;
			strcpy(field.name, f_name) ;
			if(f_cnt > 1)
				sprintf(field.name+strlen(f_name),
					"%d",(i+1)) ;
			strcpy(field.format,"%.4f");

			/* Write Field to file */
			if(write(fd,(char*)&field, sizeof(Field)) <
							sizeof(Field)) {
				printf("Field Write Error\n");
				fclose(fp);
				return(ERROR) ;
			}
			hdr.no_fields++;
		}
		break ;
	    } /* switch */
	} /* for */

	fclose(fp);

	SET_OFFSET(hdr.reclen, ALGN_STRUCT) ; /* Align the structure itself */

	/* Write again header with no of fields and reclen info */

	if(lseek(fd,h_addr,0) < 0) {
		printf("Lseek Error\n");
		return(ERROR) ;
	}

	if(write(fd,(char*)&hdr,sizeof(Fld_hdr)) < sizeof(Fld_hdr)) {
		printf("Header write error\n");
		return(ERROR) ;
	}
	printf("FILE: %s\tRecLen: %d\tFields: %d\n",hdr.filenm,
		hdr.reclen, hdr.no_fields);

	return(NOERROR);
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
/*--------------------E-N-D---O-F---F-I-L-E----------------------------*/

