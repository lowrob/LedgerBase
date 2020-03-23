/*-----------------------------------------------------------------------
Source Name: audit.c
System     : Budgetary Financial system.
Module     : General Ledger system.
Created  On: 8th May 89.
Created  By: T AMARENDRA.

Function to write audit records for added, changed & deleted
Master File Records. This function works only for DBH files.

Format:
	int	rite_audit(screen, file_no, type, new_rec, old_rec, e_mesg )
	char	*screen		* Screen address for messages
	int	file_no ;	* Files DBH Number *
	int	type ;		* Update Type. ADD, UPDATE or P_DEL *
	char	new_rec ;	* Master Record After Changes *
	char	old_rec ;	* Master Record Before Changes *
	char	*e_mesg ;	* Error message will be returned in this *

Description:
	For a given file first gets field details.

	For DELETED recods moves the key to audit record , sets audit mode
	to P_DEL and writes one record.

	For changed record compares each field in new & old records, if it
	is changed writes audit record for each changed field with its number.
	Audit mode for change of fields is UPDATE.

	For added record if the field is having non zero or non blank value
	then writes one audit record for each field with its number.
	Audit mode for change of fields is ADD.

	Error message will be returned in e_mesg, if there is any error in
	building linked list. e_mesg to be allocated in calling program to
	atleast 80 char length.

	
M O D I F I C A T I O N S :        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
L.Robichaud    93/03/05		When two programs try to write to the audit 
				file at the same time, one will not be 
				successful. A question was added to allow 
				the retrying of the write to the audit
				file until "N" is answewred or the lock is 
				released. The screen is now also passed to
				the write audit function to allow the display
				of messages.
------------------------------------------------------------------------*/


#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>
#include <fld_defs.h>

#define	OLD_FLD	(old_rec+cur_fld->offset)
#define	NEW_FLD	(new_rec+cur_fld->offset)

#ifdef ENGLISH
#define YES 'Y'
#else
#define	YES 'O'
#endif

static	Aud_rec	au_rec ;		/* Audit Record */

static	Fld_hdr	hdr ;
static	Field	*field = NULL ;

/*-------------------------------------------------------------*/
rite_audit(screen, file_no, type, new_rec, old_rec, e_mesg )
char	*screen ; /* The screen address (s_sth) for mesages */
int	file_no ;
int	type ;
char	*new_rec ,
	*old_rec ;
char	*e_mesg ;
{
	int	i, code, err;
	char	file_nm[25] ;
	static	int	cur_file = -1 ;		/*  Current File */

	/*
	* Call getflnm() of DBH to get the file name
	*/

	if(getflnm(file_no,file_nm) < 0) {
#ifdef ENGLISH
		sprintf(e_mesg,
			"** ERROR In Initializing DBH ** Dberror: %d Errno: %d",
			dberror,errno);
#else
		sprintf(e_mesg,
	"** ERREUR en initializant DBH ** Dberror: %d Errno: %d",
			dberror,errno);
#endif
		return(ERROR) ;
	}
	/* If Field Offsets are not set then first get details */
	if(file_no != cur_file) {
		if(cur_file >= 0)	/* Free the previous allocation */
			free_audit() ;
		if(GetFields(file_nm,&hdr,&field,e_mesg) < 0)
			return(ERROR) ;
		cur_file = file_no ;
	}

	/* Copy the  basic fields of audit record */
	get_tnum(au_rec.terminal) ;	/* Copy the terminal name */
	strcpy(au_rec.user_id, User_Id) ;
	au_rec.file_no = file_no ;
	strcpy(au_rec.program_id, PROG_NAME);

	i = form_key(new_rec, file_no, 0, e_mesg);	/* Get the Record Key */
	if(i == ERROR) return(ERROR) ;

	strncpy(au_rec.rec_key, e_mesg, (sizeof(au_rec.rec_key) - 1));
				/* Copy the record key */
	au_rec.mode = type ;

	/* Write Changed fields */
	for(;;){
		code = rite_flds(type, new_rec, old_rec, e_mesg) ;
/*louis*/	if(code == LOCKED){
#ifdef ENGLISH
				strcpy(e_mesg, "AUDIT File locked, Try again (Y/N)\0");
				err = GetOption(screen,e_mesg,"YN");
#else
				strcpy(e_mesg, "Fichier Verification ferme, reessayer (O/N)\0");
				err = GetOption(screen,e_mesg,"ON");
#endif
			if(err == YES) continue;
		}
		break;
	}
	return(code);
}
/*-----------------------------------------------------------*/
/* Free the field, which is allocated in GetFields() */
free_audit()
{
	if(field != NULL )
		free((char*)field) ;

	field = NULL ;
	return(NOERROR) ;
}
/*-------------------------------------------------------------*/
/* Write Audit Records for changed fields */
static int
rite_flds(type, new_rec, old_rec, e_mesg)
int	type ;
char	*new_rec ;
char	*old_rec ;
char	*e_mesg ;
{
	Field	*cur_fld ;
	int	j,k,code;
	short	changed ;
	double	temp ;

#ifndef	ORACLE
	code = lock_file(AUDIT);
	if(code < 0) {
		if(code==LOCKED) return(LOCKED);
#ifdef ENGLISH
		strcpy(e_mesg,"ERROR in Writing the Audit File");
#else
		strcpy(e_mesg,"ERREUR en ecrire le dossier de verification");
#endif
		return(ERROR) ;
	}
#endif

	au_rec.run_date = get_date() ;	/* Get Run Date */
	au_rec.run_time = (short)get_time() ;	/* Get Current time */

	/* If record is Deleted write audit record with key and return */
	if(type == P_DEL) {		/* Deleted */
		au_rec.field_no = 0 ;
		au_rec.old_value[0] = '\0' ;
		au_rec.new_value[0] = '\0' ;

		code = put_audit(&au_rec, ADD, 0, e_mesg) ;
		if(code < 0){
			return(code) ;
		}

#ifndef	ORACLE
		unlock_file(AUDIT) ;
#endif
		return(NOERROR) ;
	}

	if(type == ADD)		/* ADD mode */
		au_rec.old_value[0] = '\0' ;

	/* Add or Change */

	/* Compare old & new fields, if they are not same write a audit rec. */
	/* For Add if field is not blank or zero write a audit record */

	cur_fld = field ;
	for( j = 0 ; j < hdr.no_fields ; j++, cur_fld++ ) {

		changed = 0 ;	/* Set to not changed */

		/* Check whether fld is changed. if yes set changed to YES */
		switch ( cur_fld->type ) {
		case T_CHAR : 
			/* in add mode if new_rec.field is not blank or
			   in change mode old & new value are not same */
			if( (type == ADD && *(NEW_FLD)) ||
				(type == UPDATE && strncmp(OLD_FLD,
					NEW_FLD, cur_fld->len )) )
				changed = 1;
			break ;
		case T_SHORT: 
			/* in add mode if new_rec.field is not 0 or
			   in change mode old & new value are not same */
			if( (type == ADD && *(short *)NEW_FLD) ||
			    (type == UPDATE && *(short *)OLD_FLD !=
					*(short *)NEW_FLD ) )
				changed = 1;
			break ;
		case T_INT: 
			/* in add mode if new_rec.field is not 0 or
			   in change mode old & new value are not same */
			if( (type == ADD && *(int *)NEW_FLD) ||
			    (type == UPDATE && *(int *)OLD_FLD !=
					*(int *)NEW_FLD ) )
				changed = 1;
			break ;
		case T_LONG :
			/* in add mode if new_rec.field is not 0 or
			   in change mode old & new value are not same */
			if( (type == ADD && *(long *)NEW_FLD) ||
			    (type == UPDATE && *(long *)OLD_FLD !=
					*(long *)NEW_FLD ) )
				changed = 1;
			break ;
		case T_FLOAT :
			/* in add mode if new_rec.field is not 0 or
			   in change mode old & new value are not same */
			if(type == ADD)
				temp = *(double *)NEW_FLD ;
			else
				temp = *(double *)OLD_FLD -
					*(double *)NEW_FLD ;

			if(temp < -0.005 || temp > 0.005)
				changed = 1;
			break ;
		case T_DOUBLE :
			/* in add mode if new_rec.field is not 0 or
			   in change mode old & new value are not same */
			if(type == ADD)
				temp = *(double *)NEW_FLD ;
			else
				temp = *(double *)OLD_FLD -
					*(double *)NEW_FLD ;

			if(temp < -0.005 || temp > 0.005)
				changed = 1;
			break ;
		} /** switch ***/

		/* If not changed need not write audit record */
		if(changed == 0) continue ;

		au_rec.field_no = j + 1 ;

		switch ( cur_fld->type ) {
		case T_CHAR : 
			/* sprintfs are changed to strncpy */
			if( cur_fld->len > sizeof(au_rec.old_value)-1 )
				k = sizeof(au_rec.old_value)-1 ;
			else
				k = cur_fld->len; 
			if(type == UPDATE){	/* Change mode */
				strncpy(au_rec.old_value, OLD_FLD,k);
				au_rec.old_value[k] = '\0';
			}
			strncpy(au_rec.new_value, NEW_FLD,k);
			au_rec.new_value[k] = '\0';
			break ;
		case T_SHORT: 
			if(type == UPDATE)	/* Change mode */
				sprintf(au_rec.old_value, cur_fld->format,
					*(short *)OLD_FLD );
			sprintf(au_rec.new_value, cur_fld->format,
				*(short *)NEW_FLD);
			break ;
		case T_INT: 
			if(type == UPDATE)	/* Change mode */
				sprintf(au_rec.old_value, cur_fld->format,
					*(int *)OLD_FLD );
			sprintf(au_rec.new_value, cur_fld->format,
				*(int *)NEW_FLD);
			break ;
		case T_LONG :
			if(type == UPDATE)	/* Change mode */
				sprintf(au_rec.old_value, cur_fld->format,
					*(long *)OLD_FLD );
			sprintf(au_rec.new_value, cur_fld->format,
				*(long *)NEW_FLD);
			break ;
		case T_FLOAT :
			if(type == UPDATE)	/* Change mode */
				sprintf(au_rec.old_value, cur_fld->format,
					*(double *)OLD_FLD );
			sprintf(au_rec.new_value, cur_fld->format,
				*(double *)NEW_FLD);
			break ;
		case T_DOUBLE :
			if(type == UPDATE)	/* Change mode */
				sprintf(au_rec.old_value, cur_fld->format,
					*(double *)OLD_FLD );
			sprintf(au_rec.new_value, cur_fld->format,
				*(double *)NEW_FLD);
			break ;
		} /** switch ***/

		code = put_audit(&au_rec, ADD, 0, e_mesg) ;
		if(code < 0) return(code) ;

	} /* for {} */

#ifndef	ORACLE
	unlock_file(AUDIT) ;
#endif

	return(NOERROR) ;
}
/*--------------------E-N-D---O-F---F-I-L-E----------------------------*/

