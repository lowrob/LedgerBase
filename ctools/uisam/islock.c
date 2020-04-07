/***
	New locking routine Using fcntl call ......
	Either Read_permit Or Read_Write_inhibit lock for The node as well
	as for a Record is set Here. Dt..19-Dec-86 ...Kvs.
***/
#include <stdio.h> 
#include "isdef.h"

#ifndef	MS_DOS	/* if not MS_DOS */

#include <fcntl.h>
#include <errno.h>

 int
f_lock(fd, set_val,  lock_type, pos, size)
int	fd, 
	set_val, 		/* Wait till lock succeeds Or Return Err(0/1) */
	lock_type; 		/* Read_permit Or No-Permit lock */
long	pos ;
int	size ;			/* Rec size to be locked */
{
int code ;
struct 	flock lk_struct ;

	lk_struct.l_whence = (short)0 ;
	lk_struct.l_start = pos ;
	lk_struct.l_len = (long)size ;
	lk_struct.l_type = (short)lock_type ;
	code =   fcntl( fd, set_val, &lk_struct ) ;
	return(code) ;
}

#endif		/* If not MS_DOS */

 int 
e_lock( fd1, lock,wait   , position, size)
int 	fd1, lock ,wait   ;
long	position ;
int	 size ; 
{
#ifndef	MS_DOS		/* If not MS_DOS */
int 	set_val ;
if ( wait == 0 ) set_val = F_SETLK ;	/* Return LOCKED if unsuccessful */
else
	set_val = F_SETLKW ;		/* Wait for successful lock */

	switch(lock) {
	case UNLOCK : 	
    		if ( f_lock(fd1,set_val, F_UNLCK, position,size) < 0 ) 
			return(LOCKED);
		break ;
	case 1 :
	case 3 :
	case RDLOCK :
	case 5 :
    	 	if ( f_lock(fd1,set_val, F_RDLCK, position,size) < 0 )  
			 return(LOCKED) ;
		break;
	case WRLOCK :
    		if ( f_lock(fd1,set_val, F_WRLCK, position,size) < 0 ) 
			return(LOCKED);
	}
#endif
	return(0);
}


