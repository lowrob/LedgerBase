/* cfomtcr2.c - CPROFOM terminal handler module - part 2 */

#include <stdio.h>
#include "cfomtcr.h"	/* tcr header file */
#include "cfomdef.h"	/* gen defs */
#include "cfomstrc.h"	/* for PROFOM status record structure */
#include "cfomfrm.h"	/* for definitions of VA masks */
#include "cfomfts.h"	/* PROFOM feature test switches */
#include "cfomtm.h"	/* macro defs for tcr module */
#define  HIBIT 0100

extern	int	tcrin,	/* tcr init done ? */
			ugchar;	/* ungot character */

extern	int	tmode,	/* terminal mode : 0,2=>unix 1=>PROFOM */
		clin,	/* cursor line position */
		ccol,	/* cursor column position */
		dva,	/* current vas in effect for dynamic terminal */
		errorup,	/* error messg being displayed? */
		errorva;	/* vas for error line */

extern	struct tcrdir *tdp;	/* ptr to directory block */
extern	struct tcrblk tb;	/* tcr record of current terminal */
extern	char	*vap,		/* ptr to va array */
		*sip,		/* ptr to screen image array */
		*clrs,		/* ptr to colour array */
		termnal[TERM_NM_L+1];	/* name of current terminal */

extern	int	colourpc,
			bgclr,pfgclr,mfgclr,efgclr,
			cfgclr,cbgclr;


extern struct stat_rec *sp;	/* defined in fomfrm.c */
extern int out_scrn_image;	/* defined in fomfrm.c */
extern char cfrmnam[SR_SCRNML];	/* defined in fomfrm.c */

char *malloc();
char *err();

extern int tfd;
redraw(){	/* redraw the screen from image */
	register int i,j,prva,c,pcb,pc,f,cb,cv;
	int svline,svcol,lcols,off,svfgc,svbgc;

	svline = clin;	/* save current cursor position */
	svcol = ccol;
	if (colourpc){
		svfgc = cfgclr;
		svbgc = cbgclr;
		}
	clearscreen();	/* clear physical screen */
	home();
	if (telm(ttyp) == 2)	/* no va capability */
		if (telm(wrap))	/* terminal wraps around */
			for (i=0; i<SCRN_SZ - 1; i++)
				putchar(*(sip+i));
					/* so send all of it in one shot */
		else		/* no wrap around */
			for (i=0; i<telm(lines); i++){
					/* so, send line by line */
				poscur(i+1,1);
				for (j=0; j<telm(cols); j++)
					putchar(*(sip+i*telm(cols)+j));
				}
	else if (telm(ttyp) == 1){	/* dynamic terminal */
		prva=0; /* Changed from NULL to 0 for SCO */
		if (colourpc)
			pcb=0; /* Changed from NULL to 0 for SCO */
		off=FALSE;
		j=SCRN_SZ - 1;
		for (i=0; i<j; i++){
			f=FALSE;
			if ((cv = *(vap+i)) != prva){
				poscur(i/telm(cols)+1,i%telm(cols)+1);
				genva(cv);
				prva=cv;
				f=TRUE;
				}
			if (colourpc && (cb = *(clrs+i)) != pcb){
				setfgclr(cb%10);
				setbgclr(cb/10);
				pcb=cb;
				f=TRUE;
				}
			if ((c = *(sip+i)) != ' ' || prva)
				f=TRUE;
			if (f){
				if (off){
					poscur(i/telm(cols)+1,i%telm(cols)+1);
					off=FALSE;
					}
				putchar(pc=c);
			/*      putch(pc=c);  */
				}
			else{
				if (off)
					;
				else if (pc==' ' && (!colourpc || bgclr==cb/10))
					off=TRUE;
				else
				putchar(pc=c);
			/*      putch(pc=c);    */
				}
			}
		}
	else		/* static terminal */
		if (telm(wrap)){	/* terminal wraps around */
			off = FALSE;
			prva = ' ';
			j = SCRN_SZ - 1;
			for (i=0; i<j; i++)	/* redraw text */
				if ((c = *(sip+i)) == ' ')
					if (off)
						;
					else if (prva == ' ')
						off = TRUE;
					else
						putchar(prva = ' ');
				else if (off){
					poscur(i/telm(cols)+1, i%telm(cols)+1);
					off = FALSE;
					putchar(prva = c);
					}
				else
					putchar(prva = c);
			for (i=j-1; i>=0; i--)	/* redraw video attrs */
				if ((c = *(vap+i))){
					poscur(i/telm(cols)+1,i%telm(cols)+1);
					genva(c);
					}
			}
		else		/* no wrap around */
			for (i=0; i<telm(lines); i++){
				if (!telm(wrap))
					poscur(i+1,1);		
				lcols = (i<telm(lines) - 1 || !telm(wrap) ?
					telm(cols) : telm(cols) - 1);
				for (j=0; j<lcols; j++)
					if ((c = vab(i,j)) != 0)/* changed from NULL to 0 for SCO */
						genva(c);
					else
						putchar(sib(i,j));
				}
	poscur(svline+1,svcol+1);	/* reposition cursor where it was */
	return(0);
	}

scrimg(s,lns,cls)	/* return screen image in s of size lns*cls */
char	*s;
int	lns,cls;{
	register int i,j;

	for (i=0; i<lns; i++)
		if (i >= telm(lines))
			*(s+i*cls) = EOS_CHAR;
		else
			cpcbs(sip+i*telm(cols),telm(cols),s+i*cls);
	return(0);
	}

snapscreen(){	/* append screen image to the screen-image-file */
	FILE *flp;
	char s[SR_SCRNML+5];
	register int i,j,k;

	if (!out_scrn_image){	/* switch is off, so no work */
		showerror(35);
		return(0);
		}
	cpcbs(cfrmnam,SR_SCRNML,s);	/* first part of file name */
	strcat(s,".IMG");	/* last part of file name */
	if ((flp = fopen(s,"a")) == NULL)
		reterr(32)	/* unabl to open screen-image-file */
	fputc(FORM_FEED,flp);
	for (i=0; i<telm(lines)*telm(cols); i += telm(cols)){
			/* loop thru lines, writing each */
		for (j=i+telm(cols)-1; j >= i; j--) /* skip trailing spaces */
			if (*(sip+j) != ' ')
				break;
		if (i <= j)	/* some stuff on the line tobe written */
			if (fwrite(sip+i,1,(k = j-i+1),flp) != k)
				reterr(33)	/* screen-img-file write err */
		fputc(LINE_FEED,flp);	/* end the line with ... */
		}
	fclose(flp);
	return(0);
	}


display(line,column,va,s,typ)	/* display string s with va starting
					at (line,column) */
int line,column,va,typ;
char	*s;{
	int x,y;

	if (telm(ttyp) == 2){	/* terminal has no va capability */
		if (poscur(line,column))
			return(1);
		if (putstring(s))
			return(1);
		return(0);
		}
	else if (telm(ttyp) == 1){	/* dynamic terminal */
		if (poscur(line,column))
			return(1);
		if (colourpc)
			if (typ == PROMPT)
				setfgclr(pfgclr);
			else
				setfgclr(mfgclr);
		if (sendva(va))
			return(1);
		if (putstring(s))
			return(1);
		if (resetva(va))
			return(1);
		return(0);
		}
	else{	/* static terminal */
		if (nextpos(line,column,strlen(s),&x,&y) == -1)
			reterr(34)	/* the next character pos will be
						outside the usable area */
		if (!vab(x,y)){
			if (poscur(x,y))
				return(1);
			if (resetva(va))
				return(1);
			}
		if (prevpos(line,column,&x,&y) == -1)
			reterr(34)	/* the previous character position
					will be outside usable area */
		if (poscur(x,y))
			return(1);
		if (sendva(va))
			return(1);
		if (putstring(s))
			return(1);
		if(resetva(va))
			return(1);
		/*************************/
		return(0);
		}
	}

nextpos(line,column,length,nlp,ncp)	/* calculate coordinates of char pos
		after (line,column)+length */
int line,column,length,*nlp,*ncp;{
	int i,j,k;

	i = (line - 1) * telm(cols) + column + length - 1;
	j = i/telm(cols);
	if (j >= telm(lines) - 1)
		return(-1);
	k = i % telm(cols);
	*nlp = j+1;
	*ncp = k+1;
	return(0);
	}

prevpos(line,column,nlp,ncp)	/* calculate coordinates of char pos
					(line,column)-1 */
int line,column,*nlp,*ncp;{
	int i;
	
	i=(line-1)*telm(cols)+column-2;
	if (i<0)
		return(-1);
	*nlp=(i/telm(cols))+1;
	*ncp=(i%telm(cols))+1;
	return(0);
	}

clear(x,y,l,va)	/* clear l char pos starting from (x,y) */
int x,y,l,va;{
	int px,py;

	if (telm(ttyp) == 2){	/* no va capability */
		if (poscur(x,y))
			return(1);
		if (putblanks(l))
			return(1);
		return(0);
		}
	else if (telm(ttyp) == 1){	/* dynamic terminal */
		if (poscur(x,y))
			return(1);
		if (sendva(NULL))
			return(1);
		if (putblanks(l))
			return(1);
		return(0);
		}
	/* static terminal */
	if (prevpos(x,y,&px,&py) == -1)
		reterr(35)	/* unable to clear region on screen */
	if (poscur(px,py))
		return(1);
	if (resetva(va))
		return(1);
	if (putblanks(l))
		return(1);
	return(0);
	}

putblanks(n)	/* write n blanks to screen */
int n;{
	while (n-- > 0)
		if (put(' '))
			return(1);
	return(0);
	}

backspace(){	/* back space non-destructively */
	int nl,nc;
	
	if (prevpos(clin+1,ccol+1,&nl,&nc) == -1)
		return(0);
	if (telm(left)){
		puts(cas(left));
               clin=nl-1;
               ccol=nc-1;
		}
	else
		if (poscur(nl,nc))
			return(1);
	return(0);
	}


/* flush removed from here */
genva(v)	/* generate va sequences */
char v;	{
	register int c;

	if (v == '\0' || v == HIBIT){ /* Changed from NULL to '\0' for SCO */
		puts(cas(efa));
		if (colourpc){
			setbgclr(cbgclr);
			setfgclr(cfgclr);
			}
		return(0);
		}
	if (telm(ttyp) == 0){	/* static terminal */
		c = telm(vabase);
		if (v & VA_REVERSE)
			c |= telm(rv);
		if (v & VA_BLINK)
			c |= telm(blk);
		if (v & VA_BOLD)
			c |= telm(hiten);
		if (v & VA_DIM)
			c |= telm(loten);
		if (v & VA_ULINE)
			c |= telm(uline);
		puts(cas(sfa));	/* send prefix sequence */
		putchar(c);	/* send va char */
		if(strcmp(sp->termnm,"W50"))
		putchar('m');
/* putch('m');  */
		return(0);
		}
	else{	/* dynamic terminal */
		puts(cas(sfa));	/* send prefix sequence */
			/* Added by Amar & Mohan on 29-mar-90 */
		if (v & VA_REVERSE)
			if (colourpc)
				;
			else
				puts(cas(rv));
		if (v & VA_BLINK)
			puts(cas(blk));
		if (v & VA_BOLD)
			puts(cas(hiten));
		if (v & VA_DIM)
			puts(cas(loten));
		if (v & VA_ULINE)
			puts(cas(uline));
		return(0);
		}
	}

sendva(v)	/* send sequences for all vas encoded in v */
char v;	{
	register int c,cv;
	int temp;

	if (telm(ttyp) == 2)	/* terminal incapable of vas */
		return(0);
	if (telm(ttyp) == 0){	/* static terminal */
		if (clin==telm(lines)-1 && ccol==telm(cols)-1 && telm(wrap))
			return(0);	/* terminal will scroll */
		c = telm(vabase);
		cv = 0; /* Changed from NULL to 0 for SCO */
		if (v & VA_REVERSE && telm(rv)){
			c |= telm(rv);
			cv |= VA_REVERSE;
			}
		if (v & VA_BLINK && telm(blk)){
			c |= telm(blk);
			cv |= VA_BLINK;
			}
		if (v & VA_BOLD && telm(hiten)){
			c |= telm(hiten);
			cv |= VA_BOLD;
			}
		if (v & VA_DIM && telm(loten)){
			c |= telm(loten);
			cv |= VA_DIM;
			}
		if (v & VA_ULINE && telm(uline)){
			c |= telm(uline);
			cv |= VA_ULINE;
			}
		if (!cv){	/* terminal is incapable of any of the vas */
			puts(cas(efa));	/* send reset sequence */
			cvabyte = HIBIT;
			csibyte = ' ';
			advcur();
			return(0);
			}
		puts(cas(sfa));	/* send prefix sequence */
		putchar(c);	/* send va char */
		cvabyte = cv;	/* store in va array */
		csibyte = ' ';	/* store a blank in screen image */
		advcur();
		return(0);
		}
	else{	/* dynamic terminal */
/* Changed from NULL to 0 for SCO in next line */
		cv = 0;
		puts(cas(sfa));	/* send prefix sequence */
			/* Added by Amar & Mohan on 29-mar-90 */
		if (v & VA_REVERSE && telm(rv))
			if (colourpc){
				temp=cbgclr;
				setbgclr(cfgclr);
				setfgclr(temp);
				cv |= VA_REVERSE;
				}
			else{
				cv |= VA_REVERSE;
				puts(cas(rv));
				}
		if (v & VA_BLINK && telm(blk)){
			cv |= VA_BLINK;
			puts(cas(blk));
			}
		if (v & VA_BOLD && telm(hiten)){
			cv |= VA_BOLD;
			puts(cas(hiten));
			}
		if (v & VA_DIM && telm(loten)){
			cv |= VA_DIM;
			puts(cas(loten));
			}
		if (v & VA_ULINE && telm(uline)){
			cv |= VA_ULINE;
			puts(cas(uline));
			}
		if (!cv){	/* terminal incapable of any of the vas */
			puts(cas(efa));
			cvabyte = HIBIT;
			if (colourpc){
				setbgclr(cbgclr);
				setfgclr(cfgclr);
				}
/* Changed from NULL to 0 for SCO in next line */
			dva=0;
			return(0);
			}
		dva |= cv;	/* add these to attrs already in effect */
		return(0);
		}
		}

resetva(va){	/* send reset va sequence to terminal */

	int temp;

	if (telm(ttyp) == 2)	/* no va capability */
		return(0);
	if (telm(ttyp) == 0){	/* static terminal */
		if (clin==telm(lines)-1 && ccol==telm(cols)-1 && telm(wrap))
			return(0);	/* terminal will scroll */
		puts(cas(efa));
		cvabyte = HIBIT;
		csibyte = ' ';	/* store blank in screen image */
		advcur();
}
	else{	/* dynamic terminal */
		puts(cas(efa));
		dva = HIBIT;	/* clear vas in effect to none */
		if (colourpc)
			if (va & VA_REVERSE){
				temp=cbgclr;
				setbgclr(cfgclr);
				setfgclr(temp);
				}
			else{
				setfgclr(cfgclr);
				setbgclr(cbgclr);
				}
		}
	return(0);
	}
