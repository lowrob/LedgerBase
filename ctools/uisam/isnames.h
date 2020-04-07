/*
*	isnames.h
*/

/*** Key Types ***/

#define  CHAR           12
#define  SHORT          13
#define  FLOAT          14
#define  LONG           15
#define  DATE		16	/* should be declared long Format DDMMYY */
#define  DOUBLE		17

#define  MMDDYY		1
#define  DDMMYY		2
#define  YYMMDD		3
#define  MMDDYYYY	4
#define  DDMMYYYY	5
#define  YYYYMMDD	6

/*** Key Order ***/

#define  ASCND		0 	/* Ascending Order */
#define  DESCND		1	/* descending Order */

/***** Error Codes returned ****/

#define ERROR		-100
#define LOCKED		-50 
#define NOERROR		0
#define UNDEF		-3
#define DUPE		-5
#define EFL		-10

/**** File Open Modes ***/

#define RWR	3
#define R	4
#define	W	5

/****	Current key setting defines   ****/

#define	ISFIRST	1
#define	ISLAST	2
#define	ISEQUAL	3

int	DT_TYPE ;		/* Set to DDMMYY by default in isam.c */

int	iserror; /* Error code is returned in this global variable */

