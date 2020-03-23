/*-----------------------------------------------------------------------
Source Name: getflds.c
System     : Budgetary Financial system.
Created  On: 22 June 89.
Created  By: T AMARENDRA.

Function to read FLDEF file and get fields info for a given file.

Format:
	int	GetFields(file_nm, hdr_ptr, fields, e_mesg)
	char	file_nm ;	* File name *
	Fld_hdr	*hdr_ptr ;	* ptr to Field header *
	Field	**fields ;	* ptr to fields ptr *
	char	*e_mesg ;	* Error message will be returned in this *

Description:
	For a given file first reads the FLDDEF file and reads the infor
	into ptrs.

	Error message will be returned in e_mesg, if there is any error in
	FLDDEF file. e_mesg to be allocated in calling program to atleast
	80 char length.

	
M O D I F I C A T I O N S :        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

------------------------------------------------------------------------*/


#include <stdio.h>
#include <bfs_defs.h>
#include <fld_defs.h>


GetFields(file_nm, hdr_ptr, fields, e_mesg)
char	*file_nm ;
Fld_hdr	*hdr_ptr ;
Field	**fields ;
char	*e_mesg ;
{
	char	defs_file[50] ;
	int	fd ;

	/***
	strcpy(defs_file,FMT_PATH) ;
	****/
	strcpy(defs_file, DATA_PATH) ;
	strcat(defs_file, FLDDEF_FILE);

	/***
	form_f_name(FLDDEF_FILE, defs_file) ;
	***/

	if((fd=open(defs_file,RDMODE)) < 0){
#ifdef ENGLISH
		sprintf(e_mesg,"%s File Open Error...",defs_file);
#else
		sprintf(e_mesg,"Erreur ouverte au dossier %s...",defs_file);
#endif
		return(ERROR) ;
	}

	for( ; ; ) {
		if(read(fd,(char*)hdr_ptr,sizeof(Fld_hdr)) < sizeof(Fld_hdr)) {
#ifdef ENGLISH
			sprintf(e_mesg,"%s.def File not included in %s file",
				file_nm, defs_file);
#else
			sprintf(e_mesg,"Dossier %s.def pas inclus dans le dossier %s",
				file_nm, defs_file);
#endif
			close(fd);
			return(ERROR) ;
		}
		if(strcmp(hdr_ptr->filenm,file_nm) == 0) break ;

		/* Skip Fields information */
		if(lseek(fd,(long)(hdr_ptr->no_fields * sizeof(Field)),1) < 0){
#ifdef ENGLISH
			sprintf(e_mesg,"Lseek() ERROR on %s File",defs_file);
#else
			sprintf(e_mesg,"ERREUR Lseek() sur le dossier %s",defs_file);
#endif
			close(fd);
			return(ERROR);
		}
	}

	(*fields) = (Field *)malloc(hdr_ptr->no_fields * sizeof(Field)) ;
	if((*fields) == NULL) {
#ifdef ENGLISH
		sprintf(e_mesg,"Reading %s File... Memory Allocation ERROR",
			defs_file);
#else
		sprintf(e_mesg,"Lecture du dossier %s... ERREUR d'allocation a la memoire",
			defs_file);
#endif
		close(fd);
		return(ERROR) ;
	}
	/* Read Fields */
	if(read(fd,(char*)(*fields),(hdr_ptr->no_fields * sizeof(Field))) <
			(hdr_ptr->no_fields * sizeof(Field))) {
#ifdef ENGLISH
		sprintf(e_mesg,"%s.def File.. Fields Information Read ERROR..",
				file_nm );
#else
		sprintf(e_mesg,"Dossier %s.def.. Erreur de lecture de l'information des champ..",
				file_nm );
#endif
		close(fd);
		return(ERROR) ;
	}
	close(fd);

	return(NOERROR) ;
}
/*--------------------E-N-D---O-F---F-I-L-E----------------------------*/

