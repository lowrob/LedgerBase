/*----------------------------------------------------------------
	File Name : isdef.h

	Include file for C Isam source.
-------------------------------------------------------------------*/

/* define MS_DOS, OMEGA, UNIX or XENIX for MS_DOS,OMEGA 58000, 68000 Family
   Unix and IBM-AT SCO-XENIX systems respectively */

#define	UNIX		/* 68000 Family Unix (NCR Tower) */

#ifdef  XXXX
#define	MS_DOS		/* MS_DOS version */
#define	OMEGA		/* OMEGA 58000 Version */
#define	XENIX		/* IBM AT SCO-XENIX */
#endif

/* #define	O_ISAM */

#ifdef	O_ISAM
#define	STATUS_LEN	0	/* Del status Byte */
#else
#define	STATUS_LEN	1	/* Del status Byte */
#define	SET_DEL		'1'	/* Record Deleted */
#define	SET_ACTIVE	'0'	/* Record is Active (Not deleted) */
#endif


/*
*	If defined Record length has to passed thru iscreat(), and that will be
*	stored in Index Haeder. If not record length has to passed thru
*	iswrite() & isrewrite() calls.
*/

/* #define	FIXED_RECLEN	* To compile with fixe record length option */

#define  MAXKEYS        10
#ifdef	MS_DOS
#define  MAXFILES       10
#else
#define  MAXFILES       20	/* on UNIX ver 5.0 or above compatibles */
#endif
#define  MAX_KEY_LEN  	128 
#define	 FLNM_LEN	50 	/* Path name length stored in index */
#define  MAXSZ          1024
#define  MAX_REC_LEN    10000
#define  ERROR          -100
#define  LOCKED         -50   	/* record locked */
#define  NOERROR        0
#define  UNDEF          -3
#define  DUPE           -5
#define  TRUE           0
#define  FALSE          -2 

#define  INXHSZ         1024

/* file opening modes */

#ifdef  MS_DOS 

#include <fcntl.h >
#include <sys/types.h>
#include <sys/stat.h>

#define	PMODE	(O_BINARY | S_IREAD | S_IWRITE) 

#define RDONLY	O_BINARY | O_RDONLY
#define	WRONLY	O_BINARY | O_WRONLY
#define READWR	O_BINARY | O_RDWR

#else

#define PMODE	0666

#define RDONLY	0
#define	WRONLY	1
#define READWR	2

#endif

#ifdef	UNOS

/* define constants for "lockf()"  */

#define	F_ULOCK		0	/* unlock a region */
#define	F_LOCK		1	/* lock a region and wait if reqd */
#define	F_TLOCK		2	/* lock a region, but no wait */
#define	F_TEST		3	/* test for existance of lock */

#define	F_SETLK		6
#define	F_SETLKW	7
#define	F_RDLCK		01
#define	F_WRLCK		02
#define	F_UNLCK		03

#endif

#define	UNLOCK		0	/* Unlocks a region 	*/
#define	RDLOCK		4	/* Puts the region under read-permit lock */
#define	WRLOCK		2 	/* No reads/writes permitted  with this lock */

