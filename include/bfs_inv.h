/******************************************************************************
		Sourcename    : bfs_inv.h
		System        : Budgetary Financial System.
		Subsystem     : Inventory : Header file for definitions
		Created on    : 89-09-21
		Created  By   : K HARISH.


HISTORY:
 Date           Programmer     Description of modification
____/__/__      __________     ___________________________

******************************************************************************/
#define	INV_PROJECT	"stockrep"

#define	LR_PCREP	1	/* logical record for physical count reports */
#define	LR_STATUS	1	/* logical record for stock status reports */
#define	LR_BELMIN	1	/* logical record for below minimum report */
#define	LR_TRBAL	1	/* logical record for trial balance report */
#define	LR_STMAST	1	/* logical record for stock master listing */
#define	LR_STTRAN	2	/* logical record for transactions listing */
#define	LR_ALLOC	3	/* logical record for allocations listing */

#define	FM_PCWS 	1	/* Report format for physical count WS */
#define	FM_PCREP	2	/* Report format for physical count REPORT */
#define	FM_MSTAT	3	/* Report format for monthend status report */
#define	FM_YSTAT	4	/* Report format for yearend status report */
#define	FM_BELMIN	5	/* Report format for below minimum report */
#define	FM_TRBAL	6	/* Report format for trial balance */
#define	FM_STMAST	7	/* Report format for stock master listing */
#define	FM_AL_LOC	1	/* Report format for alloc list on location */
#define	FM_AL_COD	2	/* Report format for alloc list on codes */
#define	FM_TR_DATE	1	/* Report format for trans list datewise */
#define	FM_TR_CODE	2	/* Report format for trans list codewise */

