/* cfomfld1.c - CPROFOM fields processor module - part 1 */

#include <stdio.h>
#include "cfomdef.h"
#include "cfomfrm.h"
#include "cfomstrc.h"

extern	struct	frmfld	*cf;	/* current field of the form */
extern	struct	fldinfo	*cfi;	/* internal data items of *cf */
extern	char	*fvp;		/* variable data area */
extern	char	*ourdr,		/* PROFOM's copy of user data record */
		*userdr;	/* user data record */
extern struct stat_rec	*sp;	/* staus record */

chkdv()		/* check the value in internal data record for field in cf
			and return 1 if HIGH-VALUES, -1 if 
			LOW-VALUES and 0 otherwise */
{
	if (cmpcbc(ourdr+cf->drloc, cf->dfsize, LOW_VAL))
		return(-1);	/* contains LOW_VALUES */
	else if (cmpcbc(ourdr+cf->drloc, cf->dfsize, HIGH_VAL))
		return(1);	/* contains HIGH_VALUES */
	else 
		return(0);	/* neither */
	}

cmpcbx(s,t,l)	/* compare two COBOL X fields and return true if equal */
register char	*s,*t;
register int	l;
{
	register int i;
	
	for (i=0; i<l; i++)
		if (s[i] != t[i])
			return(FALSE);
	return(TRUE);
	}

cmpcbc(s,l,c)		/* if all characters of s equal c return TRUE */
register char	*s,c;
register int	l;
{	register int i;
	
	for (i=0; i<l; i++)
		if (*(s+i) != c)
			return(FALSE);
	return(TRUE);
	}
clrdupbuf()	/* clear dup buf of *cf */
{
	register char *i,c;
	
	c=((cfi->type == FITYP_NUM) ? COB_ZEROE : COB_SPACE);
	for (i= fvp + cf->dupval; i< fvp + cf->dupval + cf->dfsize; i++)
		*i = c;
	return(0);
	}
cmpstv(s,c)	/* return true if all characters of s are c */
register char	*s,c;
{
	while (*s)
		if (*s++ != c)
			return(FALSE);
	return(TRUE);
	}

cla1(t,mask)	/* change la corr to mask in lattr1 based on t */
int t,mask;
{
	if (t)
		cf->lattr1 |= mask;	/* turn on */
	else
		cf->lattr1 &= ~mask;	/* turn off */
	return(0);
	}

cla2(t,mask)	/* change la corr to mask in lattr2 based on t */
int t,mask;
{	if (t)
		cf->lattr2 |= mask;	/* turn on */
	else
		cf->lattr2 &= ~mask;	/* turn off */
	return(0);
	}

chva(t,mask)	/* change va corr to mask based on t */
int t,mask;
{
	switch (t){
	case 0:			/* off for both prompt and mask */
		cf->promva &= ~mask;
	case 5:			/* off for field only */
		cf->fldva &= ~mask;
		break;
	case 1:			/* on for both prompt and mask */
		cf->promva |= mask;
	case 3:			/* on for field only */
		cf->fldva |= mask;
		break;
	case 2:			/* on for prompt only */
		cf->promva |= mask;
		break;
	case 4:			/* off for prompt only */
		cf->promva &= ~mask;
		break;
	}
	return(0);
	}
cpscb(s,d,l)	/* copy string s to COBOL X field d of length l */
register char *s,*d;
register int  l;
{	register int i;

	for (i=0; i<l; i++)
/*		(unsigned)*/ (*d++) = (*s ? *s++ : COB_SPACE);
	return(0);
	}

cpcbs(x,l,s)	/* copy COBOL field x of length l to string s */
char	*x,*s;
int	l;
{
	register char *i,*j;

	for (i=x+l-1; i >= x; i--)	/* eliminate trailing spaces */
		if (*i != COB_SPACE)
			break;
	for (j=x; j <= i; j++)		/* copy what is left */
		*s++ = *j;
	*s = EOS_CHAR;
	return(0);
	}

cpdupval()	/* copy the value from user data record to dup buffer for cf */
{
	register int i;

	for (i=0; i < cf->dfsize; i++)
		*(fvp+cf->dupval+i) = *(userdr+cf->drloc+i);
	return(0);
	}

chkudv()	/* check the value in user data record for field in cf
			and return 1 if HIGH-VALUES, -1 if 
			LOW-VALUES and 0 otherwise */
{
	if (cmpcbc(userdr+cf->drloc, cf->dfsize, LOW_VAL))
		return(-1);	/* contains LOW_VALUES */
	else if (cmpcbc(userdr+cf->drloc, cf->dfsize, HIGH_VAL))
		return(1);	/* contains HIGH_VALUES */
	else 
		return(0);	/* neither */
	}

putmask(){	/* display the mask for current field */
	if (cf->fldclas == CL_PROM)
		fomintlerr(6)	/* prompt only field */
	if (display(cf->fldx,cf->fldy,cf->fldva,cvelm(dmask),MASK))
		return(1);
	return(0);
	}
putdata(){	/* display data for current field */
	if (cf->fldclas == CL_PROM)
		fomintlerr(7)	/* prompt only field */
	if (edit())
		return(1);
	if (display(cf->fldx,cf->fldy,cf->fldva,cvelm(eddata),DATA))
		return(1);
	return(0);
	}

putprom(){	/* display the prompt for current field */
	if (cf->fldclas == CL_FLD)
		fomintlerr(8)	/* mask only field */
	if (display(cf->promx,cf->promy,cf->promva,cvelm(prompt),PROMPT))
		return(1);
	return(0);
	}
clrprom(){	/* clear prompt for current field */
	if (cf->fldclas == CL_FLD)
		fomintlerr(9)	/* mask only field */
	if (clear(cf->promx,cf->promy,strlen(cvelm(prompt)),cf->promva))
		return(1);
	return(0);
	}

clrfld(){	/* clear mask part for current field */
	if (cf->fldclas == CL_PROM)
		fomintlerr(10)	/* prompt only field */
	if (clear(cf->fldx,cf->fldy,cfi->oscrnsz,cf->fldva))
		return(1);
	return(0);
	}

cpdata(){	/* copy data from userdr to ourdr for cf */
	register int k;
	register char *i,*j;

	i = userdr+cf->drloc;
	j = ourdr+cf->drloc;
	for (k=0; k<cf->dfsize; k++)
		*j++ = *i++;
	return(0);
	}

cpdvtou(){	/* copy dup value to user data record for cf */
	register int i;

	for (i=0; i<cf->dfsize; i++)
		*(userdr+cf->drloc+i) = *(fvp+cf->dupval+i);
	return(0);
	}

posinfld(n)	/* pos cursor to nth char in mask for cf */
int n;{
	int nl,nc;

	if (nextpos(cf->fldx,cf->fldy,n-1,&nl,&nc) == -1)
		reterr(37)
	if (poscur(nl,nc))
		return(1);
	return(0);
	}

inbounds(){	/* returns TRUE if value in userdr for cf is in bounds
			else returns FALSE */
	char s[82];
	int inb,uds,bs,savechar;

	if (cf->fldtyp != TYP_NUM && cf->fldtyp != TYP_STRING)
		return(TRUE);
	if (!(cf->lattr2 & LA_BOUNDS))
		return(TRUE);
	if (cf->lbound == 0 && cf->ubound == 0)
		return(TRUE);
	inb = TRUE;
	if (cf->fldtyp == TYP_STRING){
		cpcbs(userdr+cf->drloc,cf->dfsize,s);
		if (cf->lbound && (strcmp(s,cvelm(lbound)) < 0))
			inb = FALSE;
		if (cf->ubound && (strcmp(s,cvelm(ubound)) > 0))
			inb = FALSE;
		return(inb);
		}
	else{	/* type should be TYP_NUM */
		cpcbs(userdr+cf->drloc,cf->dfsize,s);
		if ((uds = signconv(s+cf->dfsize-1)) == 0)
			fomintlerr(20)
		if (cf->lbound){
			savechar = *(fvp+cf->lbound+cf->dfsize-1);
			if ((bs = signconv(fvp+cf->lbound+cf->dfsize-1)) == 0)
				fomintlerr(20)
			if (uds == -1)	/* udv -ve */
				if (bs == -1)	/* lb -ve */
					if (strcmp(s,cvelm(lbound)) > 0)
						inb = FALSE;
					else
						;
				else	/* lb +ve */
					inb = FALSE;
			else	/* udv +ve */
				if (bs == -1)	/* lb -ve */
					;
				else	/* lb +ve */
					if (strcmp(s,cvelm(lbound)) < 0)
						inb = FALSE;
					else
						;
			*(fvp+cf->lbound+cf->dfsize-1) = savechar;
			if (!inb)
			 	return(FALSE);
			}
		if (cf->ubound){
			savechar = *(fvp+cf->ubound+cf->dfsize-1);
			if ((bs = signconv(fvp+cf->ubound+cf->dfsize-1)) == 0)
				fomintlerr(20)
			if (uds == -1)	/* udv -ve */
				if (bs == -1)	/* ub -ve */
					if (strcmp(s,cvelm(ubound)) < 0)
						inb = FALSE;
					else
						;
				else	/* ub +ve */
					;
			else	/* udv +ve */
				if (bs == -1)	/* ub -ve */
					inb = FALSE;
				else	/* ub +ve */
					if (strcmp(s,cvelm(ubound)) > 0)
						inb = FALSE;
					else
						;
			*(fvp+cf->ubound+cf->dfsize-1) = savechar;
			}
		return(inb);
		}
	}

signconv(cp)	/* convert signed digit to plain char and return sign */
char *cp;{
	int c;

	c = *cp;
	if (c >= '0' && c <= '9')
		return(1);
	if (c == '{'){
		*cp = '0';
		return(1);
		}
	if (c == '}'){
		*cp = '0';
		return(-1);
		}
	if (c >= 'A' && c <= 'I'){
		*cp = (c - 'A' + '0' + 1);
		return(1);
		}
	if (c >= 'J' && c <= 'R'){
		*cp = (c - 'J' + '0' + 1);
		return(-1);
		}
	return(0);
	}

cmpxx(cp1,cp2,l)
char  *cp1,*cp2;
int l;
{
	while(l--)
	{
		if(*cp1 < *cp2) return(-1);
		else
			if(*cp1 > *cp2) return(1);
			else
			{
				cp1++; cp2++;
			}
	}
	return(0);
}

