/* cfomfrm1.c : CPROFOM form processor - front end routines - part 1 */

#include <stdio.h>
#include "cfomfrm.h"		/* form file header */
#include "cfomstrc.h"		/* status record header */
#include "cfomfts.h"		/* feature test switches */
#include "cfomdef.h"		/* some general definitions */

/* Commented out by Amar on 17-APR-87 to give real screen version
#define PFM_VERSN	"1A(032)-1"	 PROFOM version string 
*/

/* Added 3-july-90 by J.Prescott */
#ifdef ENGLISH
#define PFMERFL		"/usr/bin/fomerr.bin" /* English default error 
						 message file */
#else
#define PFMERFL		"/usr/bin/ffomerr.bin" /* French default error 
						  message file */
#endif
/*-------------------------------*/

struct 	stat_rec	*sp=NULL;	/* user status record ptr */
struct  frmhdr		*fhp;		/* form file header area */
struct	frmfld		*fap = NULL;	/* ptr to array of fields */
struct	frmfld		*cf = NULL;	/* current field being accessed */
struct	fldinfo		*fiap = NULL;	/* ptr to array of internal items */
struct	fldinfo		*cfi = NULL;	/* current field's internal items */
char	*fvp=NULL;			/* ptr to variable data area */

static	int	fd;		/* for reading form file */
	int	formup=0;	/* is a form currently up? */
char	cfrmnam[SR_SCRNML];	/* if yes, here is its name */
extern 	int 	tcrin,		/* defined in fomtcr.c : a tcr up? */
				colourpc;	/* defined in fomtcr.c */
char	*ourdr,		/* profom internal data record */
	*actudr,	/* actual user record */
	*userdr;	/* converted user data record */
char *fomrdinfo;	/* for use by fomrd() */
int	out_scrn_image = FALSE,	/* output screen image with ctrl-p? */
 	updt_dup_buf   = TRUE;	/* update dup buffers on fomwr/fomwf? */
int		ffld,		/* index to first field in form */
		ffldno,		/* field no of first field */
		lfld,		/* index to last field in form */
		lfldno,		/* field no of last field */
		firstfld,	/* first field of multi-field range */
		lastfld;	/* last field of the range */

char *malloc();

char *alloc();

fomin(srp)		/* init profom runtime */
struct stat_rec *srp;
{	

	FILE *prf;
	sp=srp;		/* note user status record address */
	if (!tcrin){	/* tcr initialisation not done */
		if(sp->termnm[0] == LV_CHAR){
			printf("enter terminal name\n");
			readstring(sp->termnm,SR_TNML);
			}
		fflush(stdin);
		if (tcrinit(sp->termnm))
			return(1);
		}
	if (sp->errset[0] == LV_CHAR)	/* error file name */
		strcpy(sp->errset,PFMERFL);
	if (errinit(sp->errset))
		return(1);
	retnoerr
	}

fomcs()		/* PROFOM clear screen call */
{
	chkstat
	if (!formup){
		if (clrscrn())
			return(1);
		retnoerr
		}
	if (clrall())		/* clear current screen */
		return(1);
	retnoerr
	}

fomce()		/* clear error line */
{
	chkstat
	if (clreln())
		return(1);
	retnoerr
	}

fomen(s)	/* display message on last line & keep cursor there */
char *s;
{
	chkstat
	if (ncmerr(s))
		return(1);
	retnoerr
	}

fomer(s)	/* display error message */
char *s;
{
	chkstat
	if (pmerln(s))
		return(1);
	retnoerr
	}

fomrt()		/* restore terminal characteristics to what they were before
			PROFOM changed them */
{
	chkstat
	if (rstty())
		return(1);
	retnoerr
	}

fomst()		/* set terminal characteristics to those needed by PROFOM */
{
	chkstat
	if (settty())
		return(1);
	retnoerr
	}

fomvn(s)	/* retutn PROFOM version string in s */
char *s;
{
	chkstat
	/* Next 2 lines inserted by Amar on 17-APR-87 to give real screen Version */
	if(formreqd())
		reterr(10)
	strcpy(s,fhp->version);  /* PFM_VERSN changed to fhp->version by Amar*/
	retnoerr
	}

fomud(audrp)		/* update dup buffers call */
char *audrp;{
	register int i,j,k;

	chkstat
	actudr = audrp;
	if ((k = chkform()) == 1)	/* see if new form required */
		return(1);
	if (fldrange())	/* find range of fields */
		return(1);
	for (i = firstfld; i <= lastfld; i++) {	/* loop thru all flds */
		cf = fap+i;
		if (cf->fldclas == CL_PROM)	/* prompt only field */
			continue;
		cfi = fiap+i;
		if (k)
			convau();	/* convert current field */
		switch (chkudv()){	/* check user data record */
		case 1 :	/* HIGH_VALUES */
			break;
		case 0 :	/* data value */
			if (cf->dupval == 0)	/* no dup buffer currently */
				fomintlerr(99)
			cpdupval();	/* copy value into buff */
			break;
		case -1 :	/* LOW_VALUES */
			if (cf->dupval == 0)	/* no dup buffer currently */
				fomintlerr(99)
			clrdupbuf();	/* clear the dup buffer */
			break;
		default : fomintlerr(1)
				break;
		} /* switch */
		} /* for */
		sp->endfld = 0;	/* clear end-field of status record */
		retnoerr
		}

fomwf(audrp)	/* display field with data */
char *audrp;{
	int pvc,newc,k;

	chkstat
	actudr = audrp;
	if ((k = chkform()) == 1)
		return(1);
	if (findfld() == -1)
		reterr(7)
	if (cf->fldclas == CL_PROM)
		reterr(8)
	if (k)
		convau();
	pvc=chkdv();
	newc=chkudv();
	switch (newc){
	case -1:	/* display with mask */
		switch (pvc){
		case 1:		/* not currently being displayed */
			if (cf->fldclas != CL_FLD)
				if (putprom())
					return(1);
		case 0:		/* being displayed with data */
			if (putmask())
				return(1);
			cpdata();
		case -1:	/* being displayed with mask */
			if (cf->dupval && cf->dupctrl && updt_dup_buf)
				clrdupbuf();
			break;
		default : fomintlerr(1)
		}
		break;
	case 0:		/* display with data */
		switch (pvc){
		case 1:
			if (cf->fldclas != CL_FLD)
				if (putprom())
					return(1);
		case 0:
			if(!cmpxx(ourdr+cf->drloc,userdr+cf->drloc,cf->dfsize))
				break;
		case -1:
			if (putdata())
				return(1);
			cpdata();
			if (cf->dupval && cf->dupctrl && updt_dup_buf)
				cpdupval();
			break;
		default : fomintlerr(1)
		}
		break;
	case 1:		/* field to be removed */
		switch (pvc){
		case -1:
		case 0:
			if (cf->fldclas != CL_FLD)
				if (clrprom())
					return(1);
			if (clrfld())
				return(1);
			cpdata();
		case 1:
			break;
		default : fomintlerr(1)
		}
		break;
	default : fomintlerr(11)
	}
	retnoerr
	}

fomrf(audrp)	/* read a single field from the screen */
char *audrp;{
	register int pvc,rv,k;

	chkstat
	actudr = audrp;
	if ((k = chkform()) == 1)
		return(1);
	if (sp->nextfld == -1)
		retnoerr
	if (findfld() == -1)
		reterr(7)
	if (cf->fldclas == CL_PROM)
		reterr(8)
	if (k)
		convau();
	if ((pvc = chkdv()) == 1 && cf->fldclas != CL_FLD)
		if (putprom())
			return(1);
	if ((cf->lattr2 & LA_SHODUP) && (cf->dupctrl != DUP_NONE) &&
		(cf->dupval != 0)){
		cpdvtou();
		if (putdata())
			return(1);
		}
	else if (pvc != -1) {
			/* Next line is added to take care of the BUG,
			   when ESC-char is entered writing the fld with
			   old value is not displaying it */
			fillcb(userdr+cf->drloc, cf->dfsize, LOW_VAL) ;
			if (putmask())
				return(1);
	}
	rv = fomaccept(FALSE,FALSE);
	cpdata();	/* copy data to ourdr */
	switch (rv){
	case 0:		/* normal return */
		convutoa();
		if (cf->lattr1 & LA_VALID){
			sp->retcode = RET_VAL_CHK;
			sp->curfld = cf->fldno;
			return(0);
		}
		retnoerr
	case 1:		/* error return */
		sp->curfld = cf->fldno;
		reterr(38)
	case -1:	/* field backspace */
		fomintlerr(11)
	case -2:	/* immediate return; same field sholud be read */
		sp->curfld = cf->fldno;;
		return(0);
	case -3:	/* field forward */
		fomintlerr(13)
	case -4:	/* immediate return; next call should return */
		sp->curfld = cf->fldno;
		sp->nextfld = -1;
		convutoa();
		return(0);
	default:fomintlerr(12)
	}
	}

fomwr(audrp)	/* multi-field write */
char *audrp;{

	int pvc,newc,k;
	register int i;

	chkstat
	actudr = audrp;
	if ((k = chkform()) == 1)
		return(1);
	if (fldrange())
		return(1);
	for (i=firstfld; i<=lastfld; i++){	/* loop thru fields */
		cf=fap+i;
		if (cf->fldclas == CL_PROM)
			continue;
		cfi=fiap+i;
		if (k)
			convau();
		pvc=chkdv();
		newc=chkudv();
		switch (newc){
		case -1:	/* display with mask */
			if (k) switch (pvc){
			case 1:		/* not currently being displayed */
				if (cf->fldclas != CL_FLD)
					if (putprom())
						return(1);
			case 0:		/* being displayed with data */
				if (putmask())
					return(1);
				cpdata();
			case -1:	/* being displayed with mask */
				break;
			default : fomintlerr(1)
			}
			if (cf->dupval && cf->dupctrl && updt_dup_buf)
				clrdupbuf();
			break;
		case 0:		/* display with data */
			if (k) switch (pvc){
			case 1:
				if (cf->fldclas != CL_FLD)
					if (putprom())
						return(1);
			case 0:
				if(!cmpxx(ourdr+cf->drloc,userdr+cf->drloc,cf->dfsize))
					break;
			case -1:
				if (putdata())
					return(1);
				cpdata();
				break;
			default : fomintlerr(1)
			}
			if (cf->dupval && cf->dupctrl && updt_dup_buf)
				cpdupval();
			break;
		case 1:		/* field to be removed */
			if (k) switch (pvc){
			case -1:
			case 0:
				if (cf->fldclas != CL_FLD)
					if (clrprom())
						return(1);
				if (clrfld())
					return(1);
				cpdata();
			case 1:
				break;
			default : fomintlerr(1)
			}
			break;
		default : fomintlerr(11)
		}
		}
	sp->endfld=0;
	retnoerr
	}

fomrd(audrp)	/* multi-field read */
char *audrp;{
	int pvc,newc,rv,ck;
	register int i,j,k,prevfld;

	chkstat
	actudr=audrp;
	if ((ck = chkform()) == 1)
		return(1);
	if (sp->nextfld == -1)
		retnoerr
	if (fldrange())
		return(1);
	/* first loop thru the fields once to process writes and erases */
	for (i=firstfld; i<=lastfld; i++){
		cf=fap+i;
		if (cf->fldclas == CL_PROM){
			*(fomrdinfo+i) = 0;	/* mark as prompt only */
			continue;
			}
		cfi=fiap+i;
		if (ck)
			convau();
		pvc=chkdv();
		newc=chkudv();
		switch (newc){
		case -1:	/* display with mask */
			*(fomrdinfo+i) = 1;	/* mark as to be read */
			switch (pvc){
			case 1:		/* not currently being displayed */
				if (cf->fldclas != CL_FLD)
					if (putprom())
						return(1);
			case 0:		/* being displayed with data */
				if ((cf->lattr2 & LA_SHODUP) &&
					(cf->dupctrl != DUP_NONE) &&
					(cf->dupval != 0)){
						cpdvtou();
					/* vasu 16 oct 86 */
						if(!cmpxx(ourdr+cf->drloc,userdr+cf->drloc,cf->dfsize))
						{	cpdata();
							break;
						}
					/* end of code */
						if (putdata())
							return(1);
					}
				else
					if (putmask())
						return(1);
				cpdata();
				break;
			case -1:	/* being displayed with mask */
				if ((cf->lattr2 & LA_SHODUP) && 
					(cf->dupctrl != DUP_NONE) &&
					(cf->dupval != 0)){
						cpdvtou();
						if (putdata())
							return(1);
						cpdata();
						}
				break;
			default : fomintlerr(1)
			}
			break;
		case 0:		/* display with data */
			*(fomrdinfo+i) = 2;
			if (ck) switch (pvc){
			case 1:
				if (cf->fldclas != CL_FLD)
					if (putprom())
						return(1);
			case 0:
				if(!cmpxx(ourdr+cf->drloc,userdr+cf->drloc,cf->dfsize))
					break;
			case -1:
				if (putdata())
					return(1);
				cpdata();
				break;
			default : fomintlerr(1)
			}
			break;
		case 1:		/* field to be removed */
			*(fomrdinfo+i) = 3;	/* mark as to be removed */
			if (ck) switch (pvc){
			case -1:
			case 0:
				if (cf->fldclas != CL_FLD)
					if (clrprom())
						return(1);
				if (clrfld())
					return(1);
				cpdata();
			case 1:
				break;
			default : fomintlerr(1)
			}
			break;
		default : fomintlerr(11)
		}
		}
	/* now loop through the fields again to read */
	for (i=firstfld; i<=lastfld; i++){
		if (*(fomrdinfo+i) != 1)	/* this field not to be read */
			continue;
		cf=fap+i;
		cfi=fiap+i;
		prevfld = -1;
		for (j=i-1; j>=firstfld; j--)	/* find previous field */
			if (*(fomrdinfo+j) == 1){
				prevfld = j;
				break;
				}
		newc=chkudv();
		rv = fomaccept((prevfld == -1 ? FALSE : TRUE),
				(newc == 0 ? TRUE : FALSE));
		if (rv == 0 && cf->lattr1 & LA_VALID){
			rv = -4;
			sp->retcode = RET_VAL_CHK;
			}
		switch (rv){
		case 0:		/* normal return */
			convutoa();
			cpdata();
			break;
		case 1:		/* error return */
			sp->curfld = cf->fldno;
			cpdata();
			reterr(38)
		case -1:	/* field back space */
			if (prevfld == -1)
				fomintlerr(11)
			i = prevfld - 1;
			cpdata();
			break;
		case -2:	/* immediate return, same field to be read */
			sp->nextfld = cf->fldno;
			sp->curfld = cf->fldno;
			/* clear masks of all fields to be read on return */
			for (j=i; j<=lastfld; j++)
			  if (*(fomrdinfo+j) == 1)
			  switch (ielm(j,stortype)){
			  case STOR_CHAR :
			  	*(actudr+ielm(j,aoff)) = LV_CHAR;
			  	break;
			  case STOR_SHORT :
			  	*(short *)(actudr+ielm(j,aoff)) = LV_SHORT;
			  	break;
			  case STOR_INT :
			  	*(int *)(actudr+ielm(j,aoff)) = LV_INT;
			  	break;
			  case STOR_LONG :
			  	*(long *)(actudr+ielm(j,aoff)) = (long) LV_LONG;
			  	break;
			  case STOR_FLOAT :
			  	*(float *)(actudr+ielm(j,aoff)) = LV_FLOAT;
			  	break;
			  case STOR_DOUBLE :
			  	*(double *)(actudr+ielm(j,aoff)) = LV_DOUBLE;
			  	break;
			  }
			return(0);
		case -3:	/* field forward */
			if (newc != 0)
				fomintlerr(13)
			break;
		case -4:	/* immediate return, next to be read later */
			sp->curfld = cf->fldno;
			sp->nextfld = -1;
			convutoa();
			cpdata();/** Inserted by AMR on 23-DEC-86 **/
			/* clear masks of all fields to be read on return */
			for (j=i+1; j<=lastfld; j++)
			  if (*(fomrdinfo+j) == 1){
			  if (sp->nextfld == -1)
				sp->nextfld = elm(j,fldno);
			  switch (ielm(j,stortype)){
			  case STOR_CHAR :
			  	*(actudr+ielm(j,aoff)) = LOW_VAL;
			  	break;
			  case STOR_SHORT :
			  	*(short *)(actudr+ielm(j,aoff)) = LV_SHORT;
			  	break;
			  case STOR_INT :
			  	*(int *)(actudr+ielm(j,aoff)) = LV_INT;
			  	break;
			  case STOR_LONG :
			  	*(long *)(actudr+ielm(j,aoff)) = (long) LV_LONG;
			  	break;
			  case STOR_FLOAT :
			  	*(float *)(actudr+ielm(j,aoff)) = LV_FLOAT;
			  	break;
			  case STOR_DOUBLE :
			  	*(double *)(actudr+ielm(j,aoff)) = LV_DOUBLE;
			  	break;
			  }
			  }
			return(0);
		default:fomintlerr(12)
		}
		}
	sp->endfld=0;
	retnoerr
	}

fomsc(bgclr,pfgclr,mfgclr,errclr)	/* set colours */
int bgclr,pfgclr,mfgclr,errclr;{

	chkstat
	if (setcolours(bgclr,pfgclr,mfgclr,errclr))
		return(1);
	retnoerr
	}

fomclroff(){	/* set colour pc to off */

	if (sp)
		return(1);
	colourpc = FALSE;
	return(0);
}
