/*									*/
/*   schldrvr.h								*/
/*									*/

#define JUMP		12345	/** To Switch across the screens 1,2,3,4 & 5 **/
#define EXIT		12
#define	ESC_F		1

/**** To Switch across the school maintenance screens *****/
#define SCR_1		1     /* Screen -1 'DEMOGRAPHIC DATA' */
#define SCR_2		2     /* Screen -2 'EMPLOYMENT DATA' */
#define SCR_3		3     /* Screen -3 'RESPONSABILITY DATA' */
#define SCR_4		4     /* Screen -4 'EARNINGS DATA' */
#define SCR_5		5     /* Screen -5 'MISCELLANEOUS DATA ' */
#define SCR_6		6     /* Screen -6 'BENEFIT DATA ' */
#define SCR_7		7     /* Screen -7 'DEDUCTION DATA ' */
#define SCR_8		8     /* Screen -8 'CSB/LOAN DATA ' */
#define SCR_9		9     /* Screen -9 'GARNISHMENT DATA ' */
#define SCR_10		10     /* Screen -10 'CHEQUE LOCATION DATA ' */
#define SCR_11		11     /* Screen -11 'ATTENDANCE ' */
#define SCR_12		12     /* Screen -12 'SENIORITY ' */
#define SCR_13		13     /* Screen -13 'TEACHER QUALIFICATIONS */
#define SCR_14		14     /* Screen -14 'TEACHER ASSIGNMENT */
#define SCR_15		15     /* Screen -15 'STATUS INQUIRY */
#define SCR_16		16     /* Screen -16 'COMPETITION */

/* Maximum Number of Screens			*/
#define MAX_SCREEN	16    

#ifdef ENGLISH

/*** Various Functions performed on the School Record ***/
#define ADDREC		'A'
#define CHANGE		'C'
#define INQUIRE		'I'
#define NEXT_RECORD	'F'
#define PREV_RECORD	'B'
#define NEXT_SCR	'N'
#define PREV_SCR	'P'
#define SCREEN		'S'
#define EXITOPT		'E'

/*** Various SubOptions ***/
#define	SCR_EDIT	'S'
#define	HDR_EDIT	'H'
#define	ITM_EDIT	'I'

#define	LINE_EDIT	'L'
#define	ADD_ITEM	'A'
#define	DEL_ITEM	'D'
#define	REACTIVATE	'R'
#define	END_EDIT	'E'

#define	NEXT_TYPE	'N'
#define	PREV_TYPE	'P'

#define	YES		'Y'
#define	CANCEL		'C'

/*** Status Values to show in Multi Row screen ***/
#define	ADDED		"ADD"
#define	MODIFIED	"CHG"
#define	REMOVED		"DEL"

#else	/* FRENCH Version */

#endif	/**** #ifdef ENGLISH  *****/


#ifdef	MAIN

struct		stat_rec sr;		/* PROFOM status rec */
Emp		emp_rec,		/* School Master Record */
		pre_emp ;		/* To use rite_audit() */
short		Cur_Option ;	/* Active Screen# */
char		e_mesg[180];  		/* dbh will return err msg in this */

#else

extern	struct		stat_rec sr;	/* PROFOM status rec */
extern	Emp		emp_rec,	/* School Master Record */
			pre_emp ;	/* To use rite_audit() */
extern	char		e_mesg[];  	/* dbh will return err msg in this */
extern	short		Cur_Option ;	/* Active Screen# */

#endif	/** #ifdef MAIN **/

/*--------------------------END OF FILE------------------------------------*/

