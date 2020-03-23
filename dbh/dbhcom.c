
/*------------------------------------------------------------- */
/*	Source Name: dbhcom.c	(link DBH)			*/
/*	Created On : 4th May 1989.				*/
/*	System     : Budgetary Financial System			*/
/*								*/
/*	This file defines interface between user programs and	*/
/*	DBH. Here for each data file seperate function is pro-	*/
/*	vided for user convenience. And also good to have this	*/
/*	level, in future if any special attention to be given	*/
/*	to any file, then this is useful.			*/
/*--------------------------------------------------------------*/

#include <bfs_defs.h>
#include <bfs_com.h>

extern	int	*alt_array ;	/* Controls the alternate keys writing
				   of ISAM files */

/*--------------------------------------------------------------------*/
/*
*  alt_array[] is the array of integers to be passed to ISAM to control
*  alternate keys writing. By default all the keys are set to ON in DBH.
*  Unwanted keys DBH user can turn off thru this function. However main
*  key (alt_array[0]) is always written irrespective of the flag. This
*  function will be called by DBH before writing record to ISAM file.
*/

set_alt(rec,file_no,c_mesg)
char	*rec ;
int	file_no ;
char	*c_mesg ;
{
	if(file_no < 0 || file_no > TOTAL_FILES) {
#ifdef ENGLISH
		sprintf(c_mesg,"ERROR in Set_alt().... Invalid File... %d",
			file_no);
#else
		sprintf(c_mesg,"ERREUR dans Set_alt()....Dossier invalide... %d",
			file_no);
#endif
		return(ERROR);
	}

	return(NOERROR) ;
}
/*-----------------------------------------------------------*/
get_param( rec, mode, rec_no, c_mesg )
Pa_rec	*rec ;
int 	mode ;
int	rec_no ;
char	*c_mesg ;
{
	return(get_rec((char*)rec,PARAM,rec_no,mode,c_mesg));
}

/*
*	NOTE:	get_n_param() is not provided, because there is
*		only one Parameter record in the system.
*/

/*-----------------------------------------------------------*/
put_param( rec, mode, rec_no, c_mesg )
Pa_rec	*rec ;
int 	mode ;
int	rec_no ;
char	*c_mesg ;
{
	if(rec_no != 1) {
#ifdef ENGLISH
		strcpy(c_mesg,
			"ERROR: Invalid Write Operation on PARAMETER File..");
#else
		strcpy(c_mesg,
			"ERREUR: Operation d'inscription invalide sur le dossier PARAMETRE..");
#endif
		return(ERROR) ;
	}
	return(put_rec((char*)rec,mode,PARAM,rec_no,c_mesg));
}
/*-----------------------------------------------------------*/
get_ctl( rec, mode, key_no, c_mesg )
Ctl_rec	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,CONTROL,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_ctl( rec, mode, key_no, direction, c_mesg )
Ctl_rec	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,CONTROL,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_ctl( rec, mode, c_mesg )
Ctl_rec	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,CONTROL,c_mesg));
}
/*-----------------------------------------------------------*/
get_sch( rec, mode, key_no, c_mesg )
Sch_rec	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,SCHOOL,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_sch( rec, mode, key_no, direction, c_mesg )
Sch_rec	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,SCHOOL,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_sch( rec, mode, c_mesg )
Sch_rec	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,SCHOOL,c_mesg));
}
/*-----------------------------------------------------------*/
get_audit( rec, mode, rec_no, c_mesg )
Aud_rec	*rec ;
int 	mode ;
int	rec_no ;
char	*c_mesg ;
{
	return(get_rec((char*)rec,AUDIT,rec_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_audit( rec, mode, rec_no, direction, c_mesg )
Aud_rec	*rec ;
int 	mode ;
int	rec_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_seqrec((char*)rec,AUDIT,rec_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_audit( rec, mode, rec_no, c_mesg )
Aud_rec	*rec ;
int 	mode ;
int	rec_no ;
char	*c_mesg ;
{
	return(put_rec((char*)rec,mode,AUDIT,rec_no,c_mesg));
}
/*-----------------------------------------------------------*/
get_gl( rec, mode, key_no, c_mesg )
Gl_rec	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,GLMAST,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_gl( rec, mode, key_no, direction, c_mesg )
Gl_rec	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,GLMAST,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_gl( rec, mode, c_mesg )
Gl_rec	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,GLMAST,c_mesg));
}
/*-----------------------------------------------------------*/
get_trhdr( rec, mode, key_no, c_mesg )
Tr_hdr	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,GLTRHDR,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_trhdr( rec, mode, key_no, direction, c_mesg )
Tr_hdr	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,GLTRHDR,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_trhdr( rec, mode, c_mesg )
Tr_hdr	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,GLTRHDR,c_mesg));
}
/*-----------------------------------------------------------*/
get_tritem( rec, mode, key_no, c_mesg )
Tr_item	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,GLTRAN,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_tritem( rec, mode, key_no, direction, c_mesg )
Tr_item	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,GLTRAN,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_tritem( rec, mode, c_mesg )
Tr_item	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,GLTRAN,c_mesg));
}
/*-----------------------------------------------------------*/
get_chqhist( rec, mode, key_no, c_mesg )
Chq_hist	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,CHQHIST,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_chqhist( rec, mode, key_no, direction, c_mesg )
Chq_hist	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,CHQHIST,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_chqhist( rec, mode, c_mesg )
Chq_hist	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,CHQHIST,c_mesg));
}
/*-----------------------------------------------------------*/
get_userprof( rec, mode, key_no, c_mesg )
UP_rec	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,USERPROF,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_userprof( rec, mode, key_no, direction, c_mesg )
UP_rec	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,USERPROF,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_userprof( rec, mode, c_mesg )
UP_rec	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,USERPROF,c_mesg));
}
/*-----------------------------END OF FILE-----------------------------*/
