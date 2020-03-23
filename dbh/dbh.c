/*------------------------------------------------------------- */
/*	Source Name: dbh.c	(link DBH)			*/
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
#include <bfs_recs.h>

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
get_gstacct( rec, mode, key_no, c_mesg )
Gl_rec	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,GSTDIST,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_gstacct( rec, mode, key_no, direction, c_mesg )
Gl_rec	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,GSTDIST,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_gstacct( rec, mode, c_mesg )
Gl_rec	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,GSTDIST,c_mesg));
}
/*-----------------------------------------------------------*/
get_rehdr( rec, mode, key_no, c_mesg )
Re_hdr	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,RECHDR,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_rehdr( rec, mode, key_no, direction, c_mesg )
Re_hdr	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,RECHDR,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_rehdr( rec, mode, c_mesg )
Re_hdr	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,RECHDR,c_mesg));
}
/*-----------------------------------------------------------*/
get_reitem( rec, mode, key_no, c_mesg )
Re_item	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,RECTRAN,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_reitem( rec, mode, key_no, direction, c_mesg )
Re_item	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,RECTRAN,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_reitem( rec, mode, c_mesg )
Re_item	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,RECTRAN,c_mesg));
}
/*-----------------------------------------------------------*/
get_trhdrny( rec, mode, key_no, c_mesg )
Tr_hdr	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,GLTRHDRNY,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_trhdrny( rec, mode, key_no, direction, c_mesg )
Tr_hdr	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,GLTRHDRNY,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_trhdrny( rec, mode, c_mesg )
Tr_hdr	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,GLTRHDRNY,c_mesg));
}
/*-----------------------------------------------------------*/
get_tritemny( rec, mode, key_no, c_mesg )
Tr_item	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,GLTRANNY,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_tritemny( rec, mode, key_no, direction, c_mesg )
Tr_item	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,GLTRANNY,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_tritemny( rec, mode, c_mesg )
Tr_item	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,GLTRANNY,c_mesg));
}
/*-----------------------------------------------------------*/
get_bdhdr( rec, mode, key_no, c_mesg )
Bd_hdr	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,GLBDHDR,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_bdhdr( rec, mode, key_no, direction, c_mesg )
Bd_hdr	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,GLBDHDR,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_bdhdr( rec, mode, c_mesg )
Bd_hdr	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,GLBDHDR,c_mesg));
}
/*-----------------------------------------------------------*/
get_bditem( rec, mode, key_no, c_mesg )
Bd_item	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,GLBDITEM,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_bditem( rec, mode, key_no, direction, c_mesg )
Bd_item	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,GLBDITEM,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_bditem( rec, mode, c_mesg )
Bd_item	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,GLBDITEM,c_mesg));
}
/*-----------------------------------------------------------*/
get_stmast( rec, mode, key_no, c_mesg )
St_mast	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,STMAST,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_stmast( rec, mode, key_no, direction, c_mesg )
St_mast	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,STMAST,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_stmast( rec, mode, c_mesg )
St_mast	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,STMAST,c_mesg));
}
/*-----------------------------------------------------------*/
get_sttran( rec, mode, key_no, c_mesg )
St_tran	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,STTRAN,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_sttran( rec, mode, key_no, direction, c_mesg )
St_tran	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,STTRAN,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_sttran( rec, mode, c_mesg )
St_tran	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,STTRAN,c_mesg));
}
/*-----------------------------------------------------------*/
get_alloc( rec, mode, key_no, c_mesg )
Alloc_rec	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,ALLOCATION,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_alloc( rec, mode, key_no, direction, c_mesg )
Alloc_rec	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,ALLOCATION,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_alloc( rec, mode, c_mesg )
Alloc_rec	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,ALLOCATION,c_mesg));
}
/*-----------------------------------------------------------*/
get_section( rec, mode, rec_no, c_mesg )
St_sect	*rec ;
int 	mode ;
int	rec_no ;
char	*c_mesg ;
{
	return(get_rec((char*)rec,SECTION,rec_no,mode,c_mesg));
}

/*
*	NOTE:	get_n_section() is not provided, because there is
*		only one Section record in the system.
*/

/*-----------------------------------------------------------*/
put_section( rec, mode, rec_no, c_mesg )
St_sect	*rec ;
int 	mode ;
int	rec_no ;
char	*c_mesg ;
{
	if(rec_no != 1) {
#ifdef ENGLISH
		strcpy(c_mesg,
			"ERROR: Invalid Write Operation on SECTION File..");
#else
		strcpy(c_mesg,
			"ERREUR: Operation d'inscription invalide sur le dossier de SECTION..");
#endif
		return(ERROR) ;
	}
	return(put_rec((char*)rec,mode,SECTION,rec_no,c_mesg));
}
/*-----------------------------------------------------------*/
get_supplier( rec, mode, key_no, c_mesg )
Supplier	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,SUPPLIER,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_supplier( rec, mode, key_no, direction, c_mesg )
Supplier	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,SUPPLIER,key_no,direction,mode,c_mesg)); 
}
/*-----------------------------------------------------------*/
put_supplier( rec, mode, c_mesg )
Supplier	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,SUPPLIER,c_mesg));
}
/*-----------------------------------------------------------*/
get_pohdr( rec, mode, key_no, c_mesg )
Po_hdr	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,POHDR,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_pohdr( rec, mode, key_no, direction, c_mesg )
Po_hdr	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,POHDR,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_pohdr( rec, mode, c_mesg )
Po_hdr	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,POHDR,c_mesg));
}
/*-----------------------------------------------------------*/
get_poitem( rec, mode, key_no, c_mesg )
Po_item	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,POITEM,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_poitem( rec, mode, key_no, direction, c_mesg )
Po_item	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,POITEM,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_poitem( rec, mode, c_mesg )
Po_item	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,POITEM,c_mesg));
}
/*-----------------------------------------------------------*/
get_reqhdr( rec, mode, key_no, c_mesg )
Req_hdr	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,REQHDR,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_reqhdr( rec, mode, key_no, direction, c_mesg )
Req_hdr	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,REQHDR,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_reqhdr( rec, mode, c_mesg )
Req_hdr	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,REQHDR,c_mesg));
}
/*-----------------------------------------------------------*/
get_reqitem( rec, mode, key_no, c_mesg )
Req_item *rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,REQITEM,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_reqitem( rec, mode, key_no, direction, c_mesg )
Req_item *rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,REQITEM,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_reqitem( rec, mode, c_mesg )
Req_item *rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,REQITEM,c_mesg));
}
/*-----------------------------------------------------------*/
/*-----------------------------------------------------------*/
get_reqreason( rec, mode, key_no, c_mesg )
Req_reason *rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,REQREASON,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_reqreason( rec, mode, key_no, direction, c_mesg )
Req_reason *rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,REQREASON,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_reqreason( rec, mode, c_mesg )
Req_reason *rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,REQREASON,c_mesg));
}
/*-----------------------------------------------------------*/
get_famast( rec, mode, key_no, c_mesg )
Fa_rec	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,FAMAST,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_famast( rec, mode, key_no, direction, c_mesg )
Fa_rec	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,FAMAST,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_famast( rec, mode, c_mesg )
Fa_rec	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,FAMAST,c_mesg));
}
/*-----------------------------------------------------------*/
get_fatype( rec, mode, key_no, c_mesg )
Fa_type	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,FATYPE,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_fatype( rec, mode, key_no, direction, c_mesg )
Fa_type	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,FATYPE,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_fatype( rec, mode, c_mesg )
Fa_type	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,FATYPE,c_mesg));
}
/*-----------------------------------------------------------*/
get_fadept( rec, mode, key_no, c_mesg )
Fa_dept	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,FADEPT,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_fadept( rec, mode, key_no, direction, c_mesg )
Fa_dept	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,FADEPT,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_fadept( rec, mode, c_mesg )
Fa_dept	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,FADEPT,c_mesg));
}
/*-----------------------------------------------------------*/
get_fatran( rec, mode, key_no, c_mesg )
Fa_transfer	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,FATRAN,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_fatran( rec, mode, key_no, direction, c_mesg )
Fa_transfer	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,FATRAN,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_fatran( rec, mode, c_mesg )
Fa_transfer	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,FATRAN,c_mesg));
}
/*-----------------------------------------------------------*/
get_cust( rec, mode, key_no, c_mesg )
Cu_rec	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,CUSTOMER,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_cust( rec, mode, key_no, direction, c_mesg )
Cu_rec	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,CUSTOMER,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_cust( rec, mode, c_mesg )
Cu_rec	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,CUSTOMER,c_mesg));
}
/*-----------------------------------------------------------*/
get_arhdr( rec, mode, key_no, c_mesg )
Ar_hdr	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,ARSHDR,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_arhdr( rec, mode, key_no, direction, c_mesg )
Ar_hdr	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,ARSHDR,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_arhdr( rec, mode, c_mesg )
Ar_hdr	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,ARSHDR,c_mesg));
}
/*-----------------------------------------------------------*/
get_aritem( rec, mode, key_no, c_mesg )
Ar_item	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,ARSITEM,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_aritem( rec, mode, key_no, direction, c_mesg )
Ar_item	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,ARSITEM,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_aritem( rec, mode, c_mesg )
Ar_item	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,ARSITEM,c_mesg));
}
/*-----------------------------------------------------------*/
get_rcpthdr( rec, mode, key_no, c_mesg )
Rcpt_hdr	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,RCPTHDR,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_rcpthdr( rec, mode, key_no, direction, c_mesg )
Rcpt_hdr	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,RCPTHDR,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_rcpthdr( rec, mode, c_mesg )
Rcpt_hdr	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,RCPTHDR,c_mesg));
}
/*-----------------------------------------------------------*/
get_rcptitem( rec, mode, key_no, c_mesg )
Rcpt_item	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,RCPTITEM,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_rcptitem( rec, mode, key_no, direction, c_mesg )
Rcpt_item	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,RCPTITEM,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_rcptitem( rec, mode, c_mesg )
Rcpt_item	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,RCPTITEM,c_mesg));
}
/*-----------------------------------------------------------*/
get_invc( rec, mode, key_no, c_mesg )
In_hdr	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,APINVOICE,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_invc( rec, mode, key_no, direction, c_mesg )
In_hdr	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,APINVOICE,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_invc( rec, mode, c_mesg )
In_hdr	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,APINVOICE,c_mesg));
}
/*-----------------------------------------------------------*/
get_inhdr( rec, mode, key_no, c_mesg )
In_hdr	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,APINHDR,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_inhdr( rec, mode, key_no, direction, c_mesg )
In_hdr	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,APINHDR,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_inhdr( rec, mode, c_mesg )
In_hdr	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,APINHDR,c_mesg));
}
/*-----------------------------------------------------------*/
get_initem( rec, mode, key_no, c_mesg )
In_item	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,APINITEM,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_initem( rec, mode, key_no, direction, c_mesg )
In_item	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,APINITEM,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_initem( rec, mode, c_mesg )
In_item	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,APINITEM,c_mesg));
}
/*-----------------------------------------------------------*/
get_chq( rec, mode, key_no, c_mesg )
Chq_rec	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,CHEQUE,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_chq( rec, mode, key_no, direction, c_mesg )
Chq_rec	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,CHEQUE,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_chq( rec, mode, c_mesg )
Chq_rec	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,CHEQUE,c_mesg));
}
/*-----------------------------------------------------------*/
get_reg( rec, mode, key_no, c_mesg )
Reg_rec	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,CHQREG,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_reg( rec, mode, key_no, direction, c_mesg )
Reg_rec	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,CHQREG,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_reg( rec, mode, c_mesg )
Reg_rec	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,CHQREG,c_mesg));
}
/*-----------------------------------------------------------*/
get_aphist( rec, mode, key_no, c_mesg )
Ap_hist	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,APHIST,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_aphist( rec, mode, key_no, direction, c_mesg )
Ap_hist	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,APHIST,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_aphist( rec, mode, c_mesg )
Ap_hist	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,APHIST,c_mesg));
}
/*-----------------------------------------------------------*/
get_lastpo( rec, mode, rec_no, c_mesg )
Last_po	*rec ;
int 	mode ;
int	rec_no ;
char	*c_mesg ;
{
	return(get_rec((char*)rec,LAST_PO,rec_no,mode,c_mesg));
}

/*
*	NOTE:	get_n_lastpo() is not provided, because there is
*		only one Last Po record in the system.
*/

/*-----------------------------------------------------------*/
put_lastpo( rec, mode, rec_no, c_mesg )
Last_po	*rec ;
int 	mode ;
int	rec_no ;
char	*c_mesg ;
{
	if(rec_no != 1) {
#ifdef ENGLISH
		strcpy(c_mesg,
			"ERROR: Invalid Write Operation on LAST_PO File..");
#else
		strcpy(c_mesg,
			"ERREUR: Operation d'inscription invalide sur le dossier PARAMETRE..");
#endif
		return(ERROR) ;
	}
	return(put_rec((char*)rec,mode,LAST_PO,rec_no,c_mesg));
}
/*-----------------------------------------------------------*/
get_lastreq( rec, mode, rec_no, c_mesg )
Last_req	*rec ;
int 	mode ;
int	rec_no ;
char	*c_mesg ;
{
	return(get_rec((char*)rec,LAST_REQ,rec_no,mode,c_mesg));
}

/*
*	NOTE:	get_n_lastreq() is not provided, because there is
*		only one Last Requistion record in the system.
*/

/*-----------------------------------------------------------*/
put_lastreq( rec, mode, rec_no, c_mesg )
Last_req	*rec ;
int 	mode ;
int	rec_no ;
char	*c_mesg ;
{
	if(rec_no != 1) {
#ifdef ENGLISH
		strcpy(c_mesg,
			"ERROR: Invalid Write Operation on LAST_REQ File..");
#else
		strcpy(c_mesg,
			"ERREUR: Operation d'inscription invalide sur le dossier PARAMETRE..");
#endif
		return(ERROR) ;
	}
	return(put_rec((char*)rec,mode,LAST_REQ,rec_no,c_mesg));
}
/*-----------------------------END OF FILE-----------------------------*/
