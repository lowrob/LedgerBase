/*------------------------------------------------------------- */
/*	Source Name: dbhtend.c	(link DBH)			*/
/*	Created On : 7th Apr 1992.				*/
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

/*-----------------------------------------------------------*/ 

/* **********************  TENDERING  ********************** */ 

/*-----------------------------------------------------------*/
get_category( rec, mode, key_no, c_mesg )
Category	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,CATEGORY,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_category( rec, mode, key_no, direction, c_mesg )
Category	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,CATEGORY,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_category( rec, mode, c_mesg )
Category	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,CATEGORY,c_mesg));
}
/*-----------------------------------------------------------*/
get_itemgrp( rec, mode, key_no, c_mesg )
Item_grp	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,ITEM_GROUP,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_itemgrp( rec, mode, key_no, direction, c_mesg )
Item_grp	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,ITEM_GROUP,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_itemgrp( rec, mode, c_mesg )
Item_grp	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,ITEM_GROUP,c_mesg));
}
/*-----------------------------------------------------------*/
get_catalogue( rec, mode, key_no, c_mesg )
Catalogue	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,CATALOGUE,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_catalogue( rec, mode, key_no, direction, c_mesg )
Catalogue	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,CATALOGUE,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_catalogue( rec, mode, c_mesg )
Catalogue	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,CATALOGUE,c_mesg));
}
/*-----------------------------------------------------------*/
get_potbidder( rec, mode, key_no, c_mesg )
PotBidder	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,POTBIDDER,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_potbidder( rec, mode, key_no, direction, c_mesg )
PotBidder	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,POTBIDDER,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_potbidder( rec, mode, c_mesg )
PotBidder	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,POTBIDDER,c_mesg));
}
/*-----------------------------------------------------------*/
get_bid( rec, mode, key_no, c_mesg )
Bid	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,BID,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_bid( rec, mode, key_no, direction, c_mesg )
Bid	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,BID,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_bid( rec, mode, c_mesg )
Bid	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,BID,c_mesg));
}
/*-----------------------------------------------------------*/
get_tendhist( rec, mode, key_no, c_mesg )
Tend_Hist	*rec ;
int 	mode ;
int	key_no ;
char	*c_mesg ;
{
	return(get_isrec((char*)rec,TENDHIST,key_no,mode,c_mesg));
}
/*-----------------------------------------------------------*/
get_n_tendhist( rec, mode, key_no, direction, c_mesg )
Tend_Hist	*rec ;
int 	mode ;
int	key_no ;
int	direction ;
char	*c_mesg ;
{
	return(get_next((char*)rec,TENDHIST,key_no,direction,mode,c_mesg));
}
/*-----------------------------------------------------------*/
put_tendhist( rec, mode, c_mesg )
Tend_Hist	*rec ;
int 	mode ;
char	*c_mesg ;
{
	return(put_isrec((char*)rec,mode,TENDHIST,c_mesg));
}
