/* cfomtm.h - macro defs for tcr module */

#define TCRBIN	"/usr/bin/fomtcr.bin"	/* PROFOM tcr binary file name */
#define KILL_CHAR	01	/* CTRL/A */
#define REDRAW_CHAR	022	/* CTRL/R for redrawing the screen */

#define telm(X)	tb.fix.X	/* member X of tcr record */
#define cas(X)	(&tb.ctrlcd[tb.fix.X])	/* ptr to ctrl string corr to X */
#define SCRN_SZ	(telm(lines) * telm(cols))	/* screen size */
#define cvabyte	(*(vap+clin*telm(cols)+ccol))	/* current va byte */
#define csibyte	(*(sip+clin*telm(cols)+ccol))	/* current screen image byte */
#define cclrbyte (*(clrs+clin*telm(cols)+ccol))	/* current colour byte */
#define vab(X,Y) (*(vap+(X)*telm(cols)+Y))	/* (*vap)[X][Y] */
#define sib(X,Y) (*(sip+(X)*telm(cols)+Y))	/* (*sip)[X][Y] */
#define clrbyte(X,Y) (*(clrs+(X)*telm(cols)+Y))	/* (*clrs)[X][Y] */
