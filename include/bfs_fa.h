/*
*    Source Name : bfs_fa.h
*    System      : Budgetary Financial System.
*    Sub system	 : Fixed Assets
*
*    Created On  : 18th Oct 1989.
*
*    Contains Definitions used in this system.
*/

#ifdef ENGLISH
/* Condition codes for the fixed asset stock items */
#define	CD_EXCELLENT	'E'
#define	CD_GOOD		'G'
#define	CD_FAIR		'F'
#define	CD_POOR		'P'
#define	CD_OBSOLETE     'O'

/* Conditions for the fixed asset stock items */
#define	EXCELLENT	"EXCELLENT"
#define	GOOD		"GOOD"
#define	FAIR		"FAIR"
#define	POOR		"POOR"
#define	OBSOLETE        "OBSOLETE"

#else	/* FRENCH */

/* Condition codes for the fixed asset stock items */
#define	CD_EXCELLENT	'E'
#define	CD_GOOD		'B'
#define	CD_FAIR		'P'
#define	CD_POOR		'M'
#define	CD_OBSOLETE     'O'

/* Conditions for the fixed asset stock items */
#define	EXCELLENT	"EXCELLENT"
#define	GOOD		"BON"
#define	FAIR		"PASSABLE"
#define	POOR		"MAUVAIS"
#define	OBSOLETE        "OBSOLETE"
#endif
