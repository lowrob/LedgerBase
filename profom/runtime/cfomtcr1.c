/* cfomtcr1.c - CPROFOM terminal handler module - part 1 */

#include <stdio.h>
#include "cfomtcr.h"	/* tcr header file */
#include "cfomdef.h"	/* gen defs */
#include "cfomstrc.h"	/* for PROFOM status record structure */
#include "cfomfrm.h"	/* for definitions of VA masks */
#include "cfomfts.h"	/* PROFOM feature test switches */
#include "cfomtm.h"	/* macro defs for tcr module */
#include <fcntl.h>


int	tcrin = 0,	/* tcr init done ? */
	ugchar = -1;	/* ungot character */

/* Introduced For Colour Support */

int	colourpc=TRUE;	/* Is The Terminal A IBM Colour Monitor */

int	bgclr = 0,	/* default background clour - cyan --old=6-- */
	pfgclr = 2,	/* default prompt foreground colour - red --old 1-- */
	mfgclr = 7,	/* default mask foreground clour - blue --old 4-- */
	efgclr = 6,	/* default error message colour - magenta --old 5-- */
	cbgclr = 0,	/* current background colour */
	cfgclr = 2;	/* current foreground colour */

char *clrs;	/* ptr to array for storing colours */

/* End Of Colour Additions */

static	int	fd;	/* for reading TCRBIN file */
	int	tmode,	/* terminal mode : 0,2=>unix 1=>PROFOM */
		clin,	/* cursor line position */
		ccol,	/* cursor column position */
		dva,	/* current vas in effect for dynamic terminal */
		errorup=FALSE,	/* error messg being displayed? */
		errorva=0;	/* vas for error line */

struct tcrdir *tdp;	/* ptr to directory block */
struct tcrblk tb;	/* tcr record of current terminal */
char	*vap,		/* ptr to va array */
	*sip,		/* ptr to screen image array */
	termnal[TERM_NM_L+1];	/* name of current terminal */

extern struct stat_rec *sp;	/* defined in fomfrm.c */
extern int out_scrn_image;	/* defined in fomfrm.c */
extern char cfrmnam[SR_SCRNML];	/* defined in fomfrm.c */

int tfd;
char *malloc();
char *err();

bell()	{	/* ring bell */
	puts(cas(bell));
	return(0);
	}

lockkb(){	/* lock key board */
	puts(cas(lock));
	return(0);
	}

unlockkb(){	/* unlock key board */
	puts(cas(unlock));
	return(0);
	}

puts(s)	/* write string s to terminal without updating screen image */
const char *s;{
	char c;

	while (c = *s++)
		putchar(c & CMASK);
	return(0);
	}

/* settty rstty removed from here */

poscur(x,y)	/* position cursor to line x col y */
int x,y;{
	char s[10],*cp;

	if (x<1 || x>telm(lines) || y<1 || y>telm(cols))
		reterr(27)	/* row/col out of range */
	puts(cas(move));
	/* Modified by Amar and Mohan to add Ncr 7901 terminal */
	/***
	if(strcmp(sp->termnm,"W50"))
	{
	*****/
	if(telm(ttyp) == 1 && strcmp(sp->termnm, "N7901")) {
		if(telm(xy))
			sprintf(s,"%d",y);
		else
			sprintf(s,"%d",x);
		cp=s; while (*cp == ' ') cp++;
		puts(cp);
		if(telm(movesep))
			puts(cas(movesep));
		if(telm(xy))
			sprintf(s,"%d",x);
		else
			sprintf(s,"%d",y);
		cp=s; while (*cp == ' ') cp++;
		puts(cp);
		putchar('H');
	}
	else {
		if(telm(xy))
			putchar(y - 1 + telm(constant));
		else
			putchar(x - 1 + telm(constant));
		if(telm(movesep))
			puts(cas(movesep));
		if(telm(xy))
			putchar(x - 1 + telm(constant));
		else
			putchar(y - 1 + telm(constant));
	}
	clin = x - 1;
	ccol = y - 1;
	return(0);
	}

clrscrn(){	/* clear terminal screen and screen image */
	register int i,j;

	clearscreen();		/* clear physical screen */
	if (telm(ttyp) != 2)		/* terminal has va capability */
		for (i=0; i<SCRN_SZ; i++)	/* clear va array */
			*(vap+i) = '\0';   /* Changed for SCO */
	for (i=0; i<SCRN_SZ; i++)	/* clear screen image */
		*(sip+i) = ' ';
	if (colourpc){
		j=cbgclr*10+cfgclr;
		for (i=0; i<SCRN_SZ; i++)
			*(clrs+i) = j;
		}
	home();	/* leave cursor at home position */
	errorup=FALSE;
	return(0);
	}

tcrinit(s)	/* initialise tcr module for terminal named in s */
char *s; {
	register int i,found,j;

	if (tcrin)
		reterr(28)	/* tcrinint already done */
	tfd=fileno(stdin);
	if ((fd = open(TCRBIN,(O_RDONLY))) == -1)
		reterr(29)	/* TCRBIN open error */
	if ((tdp = (struct tcrdir *) malloc((unsigned)TCD_SZ)) == NULL)
		reterr(6)	/* no space */
	if (read(fd,(char *)tdp,TCD_SZ) != TCD_SZ)
		reterr(30)	/* TCRBIN read error */
	found = FALSE;
	for (i=0; i<DIR_ENTS; i++)	/* search for terminal named in s */
		if (!strcmp(s,tdp->trmnm[i])) {
			found = TRUE;
			break;
			}
	if (!found)
		reterr(31)	/* terminal not found */
	strcpy(termnal,tdp->trmnm[i]);	/* copy terminal name */
	lseek(fd,(long)TCB_SZ*(tdp->trmblk[i] - 1),1);
	if (read(fd,(char *)&tb,TCB_SZ) != TCB_SZ)
		reterr(30)	/* TCRBIN read error */
//	if(free((char *) tdp) == 0)
//		reterr(88);
	if ((sip = malloc((unsigned)SCRN_SZ)) == NULL)
			/* allocate for screen image */
		reterr(6)
	if (telm(ttyp) != 2)	/* terminal has va capabilities */
		if ((vap = malloc((unsigned)SCRN_SZ)) == NULL)
				/* so alloc va array */
			reterr(6)
		else
			for (i=0; i<SCRN_SZ; i++) /* and init it */
				*(vap+i) = '\0'; /* Changed for SCO */
	for (i=0; i<SCRN_SZ; i++)	/* clear screen image area */
		*(sip+i) = ' ';
	colourpc=telm(colour);
	if (colourpc){
		if ((clrs=malloc((unsigned)SCRN_SZ)) == NULL)
			reterr(6)
		j=bgclr*10+cfgclr;
		for (i=0; i<SCRN_SZ; i++)
			*(clrs+i) = j;
		/* old is puts("\033[=3h"); */
		puts("\033[=");
		}
	clin=0;	/* assume cursor is at home */
	ccol=0;
	if (telm(ttyp) == 1)	/* dynamic terminal */
		dva = 0; /* Changed from NULL to 0  for SCO */	/* assume no vas active */
	close(fd);
	if (settty())
		return(1);
	tcrin=TRUE;
	return(0);
	}

home(){	/* move cursor home */
	if (telm(home))
		puts(cas(home));
	else
		poscur(1,1);
	clin = 0;
	ccol = 0;
	return(0);
	}

advcur(){	/* calculate new position of cursor */
	if (ccol != telm(cols) - 1)
		ccol++;
	else if (!telm(wrap) || clin == telm(lines) - 1)
		;		/* do nothing */
	else {
		clin++;
		ccol = 0;
		}
	return(0);
	}

put(c)		/* send c to screen and image also */
char c;{
	if (clin == telm(lines)-1 && ccol == telm(cols)-1 && telm(wrap))
		return(0);
			/* if you write this terminal will scroll, so dont */
	if (telm(ttyp) == 0 && telm(cva) && cvabyte){
		puts(cas(cva));
		if (backspace())
			return(1);
		}
	putchar(c);
	csibyte = c;		/* store it in screen image */
	if (colourpc)
		cclrbyte = cbgclr*10+cfgclr;
	if (telm(ttyp) == 1)	/* dynamic terminal */
		cvabyte = dva;	/* store current vas in va array */
	else if (telm(ttyp) == 0)
		cvabyte = '\0'; /* changed for SCO */
	/* the following code that calculates new cursor positon is simple
		minded and assumes that you are not writing characters
		like C/R, L/F and TAB to the screen */
	if (ccol != telm(cols) - 1)		/* not at end of line */
		ccol++;			/* so incr column */
	else if (!telm(wrap)){	/* terminal doesnt wrap */
		if (poscur(clin+2,1))
			return(1);
		}
	else {
		clin++;
		ccol = 0;
		}
	return(0);
	}

clearscreen(){	/* clear physical screen but not the image */
	register int i,j;
	
	if (telm(ttyp) == 0 && telm(cva)) /* static terminal needing cva */
		for (i=0; i<telm(lines); i++)
			for (j=0; j<telm(cols); j++)
				if (vab(i,j)){
					poscur(i+1,j+1);
					puts(cas(cva));
					}
	if (colourpc)
		setbgclr(bgclr);
	puts(cas(cscrn));	/* send clear screen sequence */
	delay(telm(cscrndelay));
	return(0);
	}

putstring(s)	/* send string s and update screen image */
char *s;{
	register int c;

	while (c = *s++)
		put(c);
	return(0);
	}

delay(u)	/* generate u units of delay for the terminal */
register int u;{

	for (; u>0; u--)
		putchar('\0');
	return(0);
	}

clreln(){	/* clear error line */
	register int i,svline,svcol,j;
	
	if (!errorup)	/* no error message is being currently displayed */
		return(0);
	svline=clin+1;
	svcol=ccol+1;
	poscur(telm(lines),1);	/* pos at error line */
	if (errorva)
		resetva(errorva);	/* reset vas for error line */
	if (colourpc)
		j=cbgclr*10+cfgclr;
	if (telm(clin)){	/* terminal has clear0line capability */
		puts(cas(clin));
		delay(telm(clindelay));
		for (i=0; i<telm(cols); i++)
			sib(clin,i)=' ';	/* clear screen image */
		if (colourpc)
			for (i=0; i<telm(cols); i++)
				clrbyte(clin,i) = j;
		}
	else if (telm(ceol)){	/* terminal has clear-to-eol capability */
		puts(cas(ceol));
		delay(telm(ceoldelay));
		for (i=ccol; i<telm(cols); i++)	/* clear image */
			sib(clin,i)=' ';
		if (colourpc)
			for (i=0; i<telm(cols); i++)
				clrbyte(clin,i) = j;
		}
	else{	/* terminal has no clear-to-eol capability */
		for (i=ccol; i<telm(cols); i++)	/* clear screen & image */
			put(' ');
		if (colourpc)
			for (i=0; i<telm(cols); i++)
				clrbyte(clin,i) = j;
		if (!telm(wrap))
			put(' ');
		}
	errorup=FALSE;
	if (poscur(svline,svcol))	/* restore cursor */
		return(1);
	return(0);
	}

curpos(xp,yp)	/* return current position of cursor */
int *xp,*yp;{
	*xp=clin+1;
	*yp=ccol+1;
	return(0);
	}

pmerln(s)	/* display message s on error line */
char *s;{
	int svline,svcol,svfgc;
	
	svline=clin+1;
	svcol=ccol+1;
	poscur(telm(lines),1);	/* pos on error line */
	if (colourpc){
		svfgc=cfgclr;
		setfgclr(efgclr);
		}
	if (errorva)
		sendva(errorva);
	putstring(s);
	if (errorva && ccol < telm(cols)-1)	/* reset vas if not at eol */
		resetva(errorva);
	errorup=TRUE;
	if (poscur(svline,svcol))	/* restore cursor */
		return(1);
	if (colourpc)
		setfgclr(svfgc);
	if (FT_BELL)
		bell();
	flush();
	return(0);
	}

showerror(eno)	/* display error corr to eno */
int eno;{
	if (pmerln(err(eno)))
		return(1);
	return(0);
	}

ncmerr(s)	/* put message on error line no cursor move */
char *s;{
	int svfgc;

	poscur(telm(lines),1);
	if (colourpc){
		svfgc=cfgclr;
		setfgclr(efgclr);
		}
	if (putstring(s))
		return(1);
	errorup=TRUE;
	if (colourpc)
		setfgclr(svfgc);
	return(0);
	}

get(){	/* get next input character */
	int c;

	if (ugchar != -1){
		c = ugchar;
		ugchar = -1;
		return(c);
		}
	if (tmode != 1)
		settty();
	while ((c=getchar()) == REDRAW_CHAR)
		redraw();
	if (c == EOF){
		printf("\nEOF on stdin!\n");
		rstty();
		exit(0);
		}
	else if (c == KILL_CHAR){
		rstty();
		exit(0);
		}
	if (errorup)
		clreln();
	return(c);
	}

setbgclr(bgc){	/* set background colour */
	char s[3];

	puts("\033["); sprintf(s,"%d",40+bgc);
	puts(s); putchar('m');
	cbgclr = bgc;
	return(0);
	}

setfgclr(fgc){	/* set foregroung colour */

	char s[3];

	puts("\033["); sprintf(s,"%d",30+fgc);
	puts(s); putchar('m');
	cfgclr = fgc;
	return(0);
	}

setcolours(bgc,pfgc,mfgc,efgc)	/* set colours */
int bgc,pfgc,mfgc,efgc;{

	if (!colourpc || bgc<0 || bgc>7 || pfgc<0 || pfgc>7 ||
		mfgc<0 || mfgc>7 || efgc<0 || efgc>7 ||
		bgc==pfgc || bgc==mfgc || bgc==efgc)
		reterr(50)
	bgclr=bgc; pfgclr=pfgc; mfgclr=mfgc; efgclr=efgc;
	return(0);
	}

fomunget(c)
int c;{

	ugchar=c;
	return(0);
	}
