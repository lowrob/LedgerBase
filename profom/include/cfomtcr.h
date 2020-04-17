/* cfomtcr.h : header for fomtcr.bin file */

#define stdinhnd 0
#define ioctl    0x44
#define clearbuf 0xC
#define getdevs  0
#define setdevs  1
#define RAW      0x20
#define bdummy   6
#define BLK_SZ		512		/* ideal block size for us */
#define TERM_NM_L	8		/* length of terminal name in file */
#define DIR_ENTS	BLK_SZ/(TERM_NM_L + sizeof(short))
					/* no of entries in directory block */
#define TCD_SZ		sizeof(struct tcrdir)	/* directory block size */
#define TCB_SZ		sizeof(struct tcrblk) 	/* data block size */

struct 	tcrdir	{	/* directory block format */

char	trmnm [DIR_ENTS] [TERM_NM_L];	/* terminal name array */
short	trmblk [DIR_ENTS];		/* block number */

	};

struct 	tcrfix	{	/* fixed part of data block */

short	ttyp,		/* terminal type : 0=static 1=dynamic 2=nova */
	lines,		/* length of the screen */
	cols,		/* width of the screen */
	bell,		/* ring bell : offset into ctrl code array */
	lock,		/* lock keyboard : offset into ctrl code array */
	unlock,		/* unlock keyboard : offset into ctrl code array */
	wrap,		/* terminals wraps around? : 0=no 1=yes */
	ceol,		/* clear to eol : offset into ctrl code array */
	clin,		/* clear line : offset into ctrl code array */
	cscrn,		/* clear screen : offset into ctrl code array */
	left,		/* move cursor left : offset into ctrl code array */
	right,		/* move cursor right : offset into ctrl code array */
	up,		/* move cursor up : offset into ctrl code array */
	down,		/* move cursor down : offset into ctrl code array */
	home,		/* move cursor home : offset into ctrl code array */
	sfa,		/* va prefix sequence for static terminals only */
	efa,		/* offset to va reset sequence */
	cva,		/* offset to clear va character sequence */
	vabase,		/* base char for vas - static terminal only */
	move,		/* offset to move cursor prefix sequence */
	movesep,	/* offset to X Y seperator sequence */
	xy,		/* row before column? : 0=no 1=yes */
	constant,	/* constant to be added to X and Y */
	rv,		/* reverse video : mask for static/offset for dynamic*/
	blk,		/* blink : mask for static/offset for dynamic*/
	hiten,		/* high intensity: mask for static/offset for dynamic*/
	loten,		/* low intensity : mask for static/offset for dynamic*/
	uline,		/* under line : mask for static/offset for dynamic*/
	mcode,		/* name of routine for cursor move - not used */
	colour,		/* is the terminal a colour monitor?  */
	cscrndelay,	/* delay for clear screen */
	clindelay,	/* delay for clear line command */
	ceoldelay;	/* delay for clear to end of line command */
	};

struct 	tcrblk	{	/* format of tcr data block */

	struct 	tcrfix 	fix;	/* fixed part of it */
	char	ctrlcd[BLK_SZ - sizeof(struct tcrfix)];
			/* control code array */
	};

/* the following are control command keys the operator can input to PROFOM */

#define OC_REFRESH	022	/* CTRL/R : refresh terminal screen */
#define OC_SAVE		020	/* CTRL/P : save screen image in file */
#define OC_FLDBKSP	007	/* CTRL/G : back space to previous field */
#define OC_BKSP		025	/* CTRL/U : back space to beginning of current
						field */
#define OC_DEL		010	/* CTRL/H : delete previous character */
#define OC_ABORT	003	/* CTRL/C : abort the job */

