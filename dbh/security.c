/******************************************************************************
		Sourcename    : security.c
		System        : Budgetary Financial System.
		Created on    : 90-01-24
		Created  By   : K HARISH.
******************************************************************************

About the file:
	This file contains functions used for security management.
	The following functions may be called by recio.c and switch.c.

NOTE:
	Some of these functions call some recio.c functions, which in turn
	call these functions again. As such, a co-routine results, i.e
	A() calling B()... B() calling A(). So one has to understand the
	implemantation very carefully to make any changes, in order to avoid
	infinite loop.

	Example:
		CheckAccess() calls get_isrec().
		get_isrec() calls CheckAccess().

HISTORY:
 Date           Programmer     Description of modification
____/__/__      __________     ___________________________

******************************************************************************/
#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#ifdef	SECURITY
int	SecurityStatus = -1;	/* Initialized when DBH is initialized */
#else
int	SecurityStatus = 0 ;
#endif

/*
*	CheckAccess()
* 	Check if the user has access right for the given file and operation
*	Return		0		if access allowed
*	Return		-1		if access disallowed
*	Return		DBH_ERR		if file read error or invalid file#
*
*	CheckClass()
*	Check if the current user is Administrator or Ordinary User
*	Return		DBH_ERR		if file read error  
*	Return		1		if User is Administrator
*	Return		0		if User is Ordinary one
*/

CheckAccess( filenumber, operation, e_mesg )
int	filenumber;	/* dbh file number */
int	operation;	/* BROWSE / ADD / UPDATE / P_DEL */
char	*e_mesg;	/* Error message filled in this area */
{
	UP_rec	up_rec;
	int	retval;
	char	buf[80];

	if(SecurityStatus == -1) {
		if(init_dbh() < 0){
#ifdef ENGLISH
		    strcpy(e_mesg, "SECURITY ERROR: Initializing DBH" );
#else
		    strcpy(e_mesg, "ERREUR DE SECURITE: Initialise DBH" );
#endif
		    return( DBH_ERR );
		}
	}

	if( SecurityStatus == 0 ) return( 0 );

	/* When the file# not within range assume it is -1 and return 0 */
	if( filenumber < 0 || filenumber >= TOTAL_FILES ) return(0) ;

	/* If a mainfile is specified, allow access on other files */
	if( mainfileno != -1 && mainfileno != filenumber ) return(0);

	SetSecurityStatus(0);

	/* User_Id set to login name by proc_switch */
	strcpy( up_rec.u_id, User_Id );	
	retval = get_isrec( &up_rec, USERPROF, 0, BROWSE, e_mesg );

	SetSecurityStatus(1);

	if(retval == UNDEF) ChangeMesg(e_mesg) ;
	if( retval < 0 ) return(DBH_ERR);

	if( !((int)up_rec.u_access[filenumber] & operation) ){
		getuserflnm( filenumber, buf );
		sprintf(e_mesg, "%-20.20s ",buf);

		switch( operation ){
		case ADD:
#ifdef ENGLISH
		    strcat(e_mesg, "Addition of records not allowed");
#else
		    strcat(e_mesg, "Ajouts des fiches pas permis");
#endif
		    break;
		case UPDATE:
#ifdef ENGLISH
		    strcat(e_mesg, "Update of records not allowed");
#else
		    strcat(e_mesg, "Mise a jour des fiches pas permise");
#endif
		    break;
		case P_DEL:
#ifdef ENGLISH
		    strcat(e_mesg, "Deletion of records not allowed");
#else
		    strcat(e_mesg, "Elimination de fiches pas permise");
#endif
		    break;
		case BROWSE:
		default:
#ifdef ENGLISH
		    strcat(e_mesg, "Browsing of records not allowed");
#else
		    strcat(e_mesg, "Interrogation des fiches pas permise");
#endif
		    break;
		}
		return(NOACCESS);
	}
	return(0);
}

GetUserClass(e_mesg)
char	*e_mesg;
{
	UP_rec	up_rec;
	int	retval;
	
	if(SecurityStatus == -1) {
		if(init_dbh() < 0){
#ifdef ENGLISH
		    strcpy(e_mesg, "SECURITY ERROR: Initializing DBH" );
#else
		    strcpy(e_mesg, "ERREUR DE SECURITE: Initialise DBH" );
#endif
		    return( DBH_ERR );
		}
	}

	if(SecurityStatus == 0) return(SUPERUSER) ;

	SetSecurityStatus(0) ;	/* Security OFF */

	/* User_Id set to login name by proc_switch */
	strcpy( up_rec.u_id, User_Id );
	retval = get_isrec( &up_rec, USERPROF, 0, BROWSE, e_mesg );

	SetSecurityStatus(1) ;

	if(retval == UNDEF) ChangeMesg(e_mesg) ;
	if( retval < 0 ) return(DBH_ERR);

	return( up_rec.u_class[0] );
}

GetSecurityStatus()
{
	if(SecurityStatus == -1 && init_dbh() < 0)
		SecurityStatus = 0 ;

	return( SecurityStatus );
}

SetSecurityStatus( value )
int	value;
{
	/* If the DBH is not initialized, first initialize it. Otherwise
	   next DBH call, overwrites this flag with the security status
	   in KEY_DESC file. That could be reverse to what is being set now */
	if(SecurityStatus == -1 && init_dbh() < 0)
		SecurityStatus = 0 ;

	SecurityStatus = value;
}

/* 	Indicate whether the user is allowed access through current terminal */
CheckTrmlAccess(e_mesg)
char	*e_mesg;
{
	UP_rec	up_rec;
	int	retval;
	char	tnum[5];
	
	if(SecurityStatus == -1) {
		if(init_dbh() < 0){
#ifdef ENGLISH
		    strcpy(e_mesg, "SECURITY ERROR: Initializing DBH" );
#else
		    strcpy(e_mesg, "ERREUR DE SECURITE: Initialise DBH");
#endif
		    return( DBH_ERR );
		}
	}

	if( SecurityStatus == 0 ) return( 0 );

	/* Get the user profile record to get his legal terminal */

	SetSecurityStatus(0);

	strcpy( up_rec.u_id, User_Id );
	retval = get_isrec( &up_rec, USERPROF, 0, BROWSE, e_mesg );

	SetSecurityStatus(1);

	if(retval == UNDEF) ChangeMesg(e_mesg) ;
	if( retval < 0 ) return(DBH_ERR);

	/* Compare and decide if access can be allowed */
	if( up_rec.u_trml[0]=='\0' )	/* Access is allowed on all trml */
		return(0);

	/* Get the user's current terminal */
	get_tnum(tnum);

	if( strcmp(up_rec.u_trml,tnum) ){
#ifdef ENGLISH
		sprintf(e_mesg,"Access denied through this terminal");
#else
		sprintf(e_mesg,"Acces nie a travers ce terminal");
#endif
		return( NOACCESS );
	}

	return( 0 );
}

static	int
ChangeMesg(e_mesg)
char	*e_mesg ;
{
#ifdef ENGLISH
	sprintf(e_mesg,"User ID: %s ..... Not an authorized USER", User_Id) ;
#else
	sprintf(e_mesg,"Identite de l'usager: %s ..... Pas un usager autorise", User_Id) ;
#endif
}
