/* cfomerr.h - CPROFOM error mesage file header */

#define MAXERRORS	50	/* max no of errors curently */
#define MAXEML		77	/* max length of an error message */
#define EHSZ		(sizeof(struct erflhdr))

struct erflhdr {
int	messlen,	/* size of the error message text string area */
	eroff[MAXERRORS];	/* offsets to individual message strings */
	};
