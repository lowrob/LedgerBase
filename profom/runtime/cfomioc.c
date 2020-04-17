/********************

	Modification History : 

	Dt 27-Oct-86 :  for sco xenix ... kavi .
			ioctl calls are changed . Earlier struct sgttyb was 
	used which gave funny results in sco( It should have given in IBM
	XENIX too  because sgttyb is defined only for perq..? ). 
		Now termio struct is used and flags are set in 
	accordance to sco xenix manual. Seems OK so far.

*********************/


/***  #include <sgtty.h>  		Supressed .. Dt 25-Oct-86 ******/
#include <termio.h>
#include <stdio.h>

/*** struct sgttyb  fomtty, unixtty; ***/
struct termio fomtty, unixtty ;

extern int ugchar,tmode,tfd;

settty(){	/* set terminal to PROFOM mode */
	if (tmode == 1)
		return(0);	/* already in PROFOM mode */
	if (tmode == 2){	/* in UNIX mode */
		/*** ioctl(tfd,TIOCSETP,&fomtty); ***/
		ioctl(tfd, TCSETA, &fomtty) ; /* NEW */
		tmode = 1;
		return(0);
		}
	/* must be tmode = 0 : first call to settty */
		/* 2nd ioctl call */

	/**** Old calls ... See new calls after commented code ..
	ioctl(tfd,TIOCGETP,&unixtty);
	ioctl(tfd,TIOCGETP,&fomtty);
	***/

	ioctl(tfd,TCGETA,&unixtty);
	ioctl(tfd,TCGETA,&fomtty);

	/*** Old calls .... 
	fomtty.sg_flags |= (RAW | ALLDELAY);
	fomtty.sg_flags &= ~(ECHO | CRMOD | XTABS | LCASE | CBREAK);
	ioctl(tfd,TIOCSETP,&fomtty);
	****/

	fomtty.c_iflag  |= ICRNL ;		/* CR to Nl on input */
	fomtty.c_iflag  &= ~(IUCLC) ;		/* No Mapping of Upper to
						Lower case on input */
	fomtty.c_oflag  |= OCRNL ; 		/* CR to Nl on output */
	fomtty.c_oflag  &= ~(TAB3) ;		/* No tab expansion on
						 output */

	/*** No canonical procewsing  , No echo , No signal processing ,
	etc .... Equivilant to RAW mode of perq ****/

	fomtty.c_lflag  &= ~(ICANON | ECHO  | XCASE | ISIG | ECHOE | ECHOK |
				ECHONL );

	/**
		Set VMIN and VTIME characters for c_cc array of termio 
	struct .
	***/

	fomtty.c_cc[VMIN] = 1 ; /* Min one character input */
	fomtty.c_cc[VTIME] = 1 ; /* Time delay 1/10 sec before sending char */

	ioctl(tfd,TCSETA,&fomtty);
	tmode = 1;
	return(0);
	}

rstty(){	/* reset terminal mode to UNIX */
	if (tmode == 2)
		return(0);	/* already in UNIX mode */
	if (tmode == 1){		/* in PROFOM mode */
		/****
		ioctl(tfd,TIOCSETP,&unixtty);
		****/
		ioctl(tfd, TCSETA, &unixtty) ; /* NEW */
		tmode = 2;
		return(0);
		}
	}

flush(){	/* flush type-ahead buffers */

int flg;
 
flg = 0;
	lockkb();
	ugchar = -1;
	/******
	ioctl(tfd,TIOCFLUSH,flg);
	*****/
	ioctl(tfd, TCFLSH, flg) ; /* NEW */
	unlockkb();
	return(0);
	}


