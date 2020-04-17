/* cfomfld2.c - CPROFOM fields processor module - part 2 */

#include <stdio.h>
#include "cfomdef.h"
#include "cfomfrm.h"
#include "cfomstrc.h"

extern	struct	frmfld	*cf;	/* current field of the form */
extern	struct	fldinfo	*cfi;	/* internal data items of *cf */
extern	char	*fvp;		/* variable data area */
extern	char	*ourdr,		/* PROFOM's copy of user data record */
		*actudr,	/* actual user record */
		*userdr;	/* converted user data record */
extern struct stat_rec	*sp;	/* staus record */

static char ednum[80];

long atol();
double atof();

onscrnsz(){
 int count,val;
 char *ptr;
 count=0;
 ptr=fvp+cf->dmask;
 while(*ptr) {
    if((*ptr)=='(') {
         ptr++;
         sscanf(ptr,"%d",&val);
         ptr+=2;
         if(val>9)ptr++;
         count=count+val-1;
     }
    else 
         count++;
    ptr++;
  }
  return(count);
}

fillconv()  /* Fill up some details about current field */
  {

   char *ptr;
  int mlength;
  cfi->Signed=0;
  cfi->decimals=0;
  cfi->type=FITYP_NUM;
  if(cf->fldtyp==TYP_DATE) {
     mlength=0;
    for(ptr=fvp+cf->imask;*ptr!='\0';ptr++)
                if(*ptr=='M' || *ptr=='m')mlength++;
                if(mlength==3) cfi->type=FITYP_NON;
  }

  if(cf->fldtyp==TYP_NUM) {
       for(ptr=fvp+cf->imask;*ptr!='\0' && *ptr!='.' && *ptr!='V' && *ptr!='v';ptr++)
                  if(*ptr=='s' || *ptr=='S') cfi->Signed=1;
      if(*ptr=='.' || *ptr=='V' || *ptr=='v') {
         for(ptr++;*ptr!='\0';ptr++)
                 if(*ptr != 'F') (cfi->decimals)++;
    }

}

  if(cf->fldtyp==TYP_STRING) cfi->type=FITYP_NON;
  return(0);

 } /* fillconv  */

edit()
 {
  char *ptr1b,*ptr2b,*ptr3b,*ptr1e,*ptr2e,*ptr3e;
  char mchar;
  char *ptr;
  int value,sign;
  ptr1b=userdr+cf->drloc;
  ptr1e=userdr+cf->drloc+cf->dfsize-1;/* Points to end of data area */
  ptr2b=fvp+cf->dmask;

  for(ptr2e=ptr2b;*ptr2e!='\0';ptr2e++);
  ptr2e--; /* Points to end of display mask */
  ptr3b=fvp+(cf->eddata);
  ptr3e=fvp+cf->eddata+cfi->oscrnsz-1;
  mchar=cf->maskchar;

  sign=0; /* Unsigned */
 value= *ptr1e; /* Save this value */
  if((cf->fldtyp==TYP_NUM &&cfi->Signed==1))
          if(*ptr1e=='{'){
                sign='+';
                *ptr1e='0';
           }
         else if(*ptr1e=='}') {
                  sign='-';
                  *ptr1e='0';
            }
         else if(*ptr1e>='A' && *ptr1e<='I') {
                  sign='+';
                  *ptr1e= *ptr1e-'A'+1+'0';
               }
       else if(*ptr1e>='J' && *ptr1e<='R') {
                 sign='-';
                 *ptr1e= *ptr1e-'J'+1+'0';
           }


      else if (*ptr1e>='0' && *ptr1e<='9') 
              sign='+';
 if(sign!=0) { /* It is a signed field */
   while(*ptr2b!=mchar) 
          *ptr3b++ = *ptr2b++ ; /* Copy the filler characters */
          *ptr3b++=sign == '-' ? '-' : ' ';
    ptr2b++;
 }
       while(ptr1b<=ptr1e) {
           /* Find a posn for the chara and insert */
        while(*ptr2b!=mchar)
           *(ptr3b++) = *(ptr2b++);
	  if(ptr2b <= ptr2e)  ;
		else   return(2);
          *(ptr3b++) = *(ptr1b++);
      ptr2b++;
         }
  if(sign!=0) *ptr1e=value;
  if(cf->fldtyp==TYP_NUM && ((cf->lattr1)&LA_SUP)){
   /* suppress leading zeroes in edited field */
 	if (cfi->decimals)
 		while(*ptr3e-- != '.')
 			;
 	while (*ptr3e < '0' || *ptr3e > '9')
 		ptr3e--;
    for(ptr1b=fvp+cf->eddata;ptr1b<ptr3e;ptr1b++) {
		if (*ptr1b >'0' && *ptr1b <= '9')
			break;
		else
			*ptr1b = COB_SPACE;
        }
    if (sign == '-')
    	*--ptr1b = '-';
   }
  return(0);
  } /* edit() */

/* new routines added for C interface - klv Oct 17 */

adjoff(offset,align)	/* adjust offset to ensure alignment */
int offset,align;{
	int rem;

	return((rem = offset % align) == 0 ? offset : offset+align-rem);
	}

convau(){	/* convert data from actual user data record for current fld */
	char s[128];
	int i;
	long l;
	float f;
	double d;

	switch (cfi->stortype){
	case STOR_CHAR :
		if (*(actudr+cfi->aoff) == LV_CHAR)
			fillcb(userdr+cf->drloc,cf->dfsize,LOW_VAL);
		else if (*(actudr+cfi->aoff) == HV_CHAR)
			fillcb(userdr+cf->drloc,cf->dfsize,HIGH_VAL);
		else
			cpscb(actudr+cfi->aoff,userdr+cf->drloc,cf->dfsize);
		return(0);
	case STOR_SHORT :
		i = *(short *)(actudr+cfi->aoff);
		if (i == LV_SHORT)
			fillcb(userdr+cf->drloc,cf->dfsize,LOW_VAL);
		else if (i == HV_SHORT)
			fillcb(userdr+cf->drloc,cf->dfsize,HIGH_VAL);
		else {
			sprintf(s," %d",i);
			copyint(s);
			}
		return(0);
	case STOR_INT :
		i = *(int *)(actudr+cfi->aoff);
		if (i == LV_INT)
			fillcb(userdr+cf->drloc,cf->dfsize,LOW_VAL);
		else if (i == HV_INT)
			fillcb(userdr+cf->drloc,cf->dfsize,HIGH_VAL);
		else if (cf->fldtyp == TYP_YN)
#ifdef ENGLISH
			if (i == BOOL_NO)
				*(userdr+cf->drloc) = 'N';
			else
				*(userdr+cf->drloc) = 'Y';
#else
			if (i == BOOL_NO)
				*(userdr+cf->drloc) = 'N';
			else
				*(userdr+cf->drloc) = 'O';
#endif
		else {
			sprintf(s," %d",i);
			copyint(s);
			}
		return(0);
	case STOR_LONG :
		l = *(long *)(actudr+cfi->aoff);
		if (l == LV_LONG)
			fillcb(userdr+cf->drloc,cf->dfsize,LOW_VAL);
		else if (l == HV_LONG)
			fillcb(userdr+cf->drloc,cf->dfsize,HIGH_VAL);
		else {
			sprintf(s," %ld",l);
			copyint(s);
			}
		return(0);
	case STOR_FLOAT :
		f = *(float *)(actudr+cfi->aoff);
		if (f == LV_FLOAT)
			fillcb(userdr+cf->drloc,cf->dfsize,LOW_VAL);
		else if (f == HV_FLOAT)
			fillcb(userdr+cf->drloc,cf->dfsize,HIGH_VAL);
		else {
			sprintf(s,"  %f",f);
			copydec(s);
			}
		return(0);
	case STOR_DOUBLE :
		d = *(double *)(actudr+cfi->aoff);
		if (d == LV_DOUBLE)
			fillcb(userdr+cf->drloc,cf->dfsize,LOW_VAL);
		else if (d == HV_DOUBLE)
			fillcb(userdr+cf->drloc,cf->dfsize,HIGH_VAL);
		else {
			sprintf(s,"  %lf",d);
			copydec(s);
			}
		return(0);
	default :
		fomintlerr(14)
	}
	}

copyint(s)	/* copy the intger in s to userdr for cf */
char *s;{
	register char *cp1,*cp2, *cp3;
	int sign;

	sign = -1;
	cp1=s+strlen(s)-1;
	for (cp2=userdr+cf->drloc+cf->dfsize-1,
		cp3=userdr+cf->drloc; cp2>=cp3; cp2--)
		if (*cp1 == ' ')
			*cp2='0';
		else if (*cp1 == '+'){
			*cp2='0';
			sign=1;
			cp1--;
			}
		else if (*cp1 == '-'){
			*cp2='0';
			sign=0;
			cp1--;
			}
		else {
			*cp2 = *cp1;
			cp1--;
			}
	if (cfi->Signed) {
		if (sign != -1)
			;
		else if (*s == '-')
			sign = 0;
		else
			sign = 1;
		gensign(userdr+cf->drloc+cf->dfsize-1,sign);
		}
	return(0);
	}

copydec(s)	/* copy the decimal no in s to userdr for cf */
char *s;{
	register char *cp1,*cp2,*cp3,*cp4;
	register char *tmp,*tmpd,*tmpb,*t;
	register char *olval;
	register int i,flag,j;
	int sign;

	flag = 0;
	cp1=s;
	/****
	printf("input string %s",s);
	*****/
	sign = -1;
	while (*cp1 && *cp1 != '.')
		cp1++;
	cp2=cp1-1;
	if (*cp1 == '.')
		cp1++;

	/** introduced by vasu for rounding 10 sep 86 **/


	tmpd=cp1+cfi->decimals;
	tmpb=s;
	if(*tmpd > '4')
	{
		for(t=tmpd-1,j=0; *t != ' '; t--)
		{
			if(*t == '9')
				j++;
		}
		if(j==cf->dfsize)
			goto NOROUND;
		for(tmp = --tmpd; *tmp == '9' || *tmp == '.' ; tmp--)
		{
			if(*tmp == '.')
			{
				flag=1;
				tmp--;
				break;
			}
			*tmp = '0';
		}
		if(flag==1)
		{
			for( ; *tmp == '9' && tmp > s; tmp--)
			{
				*tmp = '0';
			}

			if(tmp == s)
				*tmp = '1';
			else
			{
			if(tmp > s)
				for(t=tmp; t>s; t--)
					if(*t == ' ')
						*t = '0';
			}
		}
		if(tmp > s)
			(*tmp)++;
	}

	NOROUND:
	/****
	printf("s size j %s %d %d",s,cf->dfsize,j);
	*****/
	cp1=s;
	while (*cp1 && *cp1 != '.')
		cp1++;
	cp2=cp1-1;
	if (*cp1 == '.')
		cp1++;

	/**   end of code by vasu 10 sept 86   **/


	for(cp3=userdr+cf->drloc+cf->dfsize-cfi->decimals,
		cp4=userdr+cf->drloc+cf->dfsize-1; cp3<=cp4; cp3++)
		if (*cp1){
			*cp3 = *cp1;
			cp1++;
			}
		else
			*cp3 = '0';
	for(cp3=userdr+cf->drloc+cf->dfsize-cfi->decimals-1,
		cp4=userdr+cf->drloc; cp3>=cp4; cp3--)
		if (*cp2 == ' ')
			*cp3 = '0';
		else if (*cp2 == '+'){
			sign=1;
			*cp3='0';
			cp2--;
			}
		else if (*cp2 == '-'){
			sign=0;
			*cp3='0';
			cp2--;
			}
		else{
			*cp3 = *cp2;
			cp2--;
			}
	while (*cp2 != ' '){
		if (*cp2 == '-')
			sign = 0;
		cp2--;
		}
	if (cfi->Signed && sign != -1)
		gensign(userdr+cf->drloc+cf->dfsize-1,sign);
	return(0);
	}

fillcb(x,l,c)	/* fill cobol X field of length l with c */
register char *x;
register int l,c;{
	register char *cp;

	cp=x+l;
	for (; x<cp; x++)
		/*unsigned)*/( *x) = c ;
	return(0);
	}

gensign(cp,sign)	/* convert *cp to cobol signed character */
register char *cp;
register sign;{

	if (sign == 1)	/* positive */
		return(0);
	if (*cp == '0')
		*cp = '}';
	else
		*cp = *cp - '1' + 'J';
	return(0);
	}

edtnum(){	/* edit the value in userdr for cf for input to atoi() etc */
	register char *ivc,*evc;
	register int i,j;
	int sign,savechar;

	ivc = userdr+cf->drloc;
/**
printf ("ivc val - %ld\n", (long) ivc) ;
**/
	savechar = *(ivc+cf->dfsize-1);
	if ((sign = signconv(ivc+cf->dfsize-1)) == 0)
		fomintlerr(20)
	evc = ednum;
	if (sign == -1)	/* -ve number */
		*(evc++) = '-';
	if (cfi->decimals){
		for (i=0, j=cf->dfsize - cfi->decimals; i<j; i++)
			*(evc++) = *(ivc++);
		*(evc++) = '.';
		for (i=0; i<cfi->decimals; i++)
			*(evc++) = *(ivc++);
		}
	else
		for (i=0; i<cf->dfsize; i++)
			*(evc++) = *(ivc++);
	*evc = EOS_CHAR;
	*(userdr+cf->drloc+cf->dfsize-1) = savechar;
	return(0);
	}

convutoa(){	/* convert value in userdr for cf to actuser format */

	switch (cfi->stortype){
	case STOR_CHAR :
		cpcbs(userdr+cf->drloc,cf->dfsize,actudr+cfi->aoff);
		return(0);
	case STOR_SHORT :
		edtnum();
		*(short *)(actudr+cfi->aoff) = atoi(ednum);
		return(0);
	case STOR_INT :
		if (cf->fldtyp == TYP_YN)
#ifdef ENGLISH
		  *(int *)(actudr+cfi->aoff) =
			(*(userdr+cf->drloc) == 'Y' ||
			 *(userdr+cf->drloc) == 'y' ? BOOL_YES : BOOL_NO);
#else
		  *(int *)(actudr+cfi->aoff) =
			(*(userdr+cf->drloc) == 'O' ||
			 *(userdr+cf->drloc) == 'o' ? BOOL_YES : BOOL_NO);
#endif
		else if (cf->fldtyp == TYP_DATE){
			edtnum();
			*(int *)(actudr+cfi->aoff) = atoi(ednum);
			}
		else {
			edtnum();
			*(int *)(actudr+cfi->aoff) = atoi(ednum);
			}
		return(0);
	case STOR_LONG :
		edtnum();
/**
	printf ("edited value - %s\n", ednum) ;
**/
		*(long *)(actudr+cfi->aoff) = atol(ednum);
/**
	printf ("Long value - %ld\n", *(long *) (actudr+cfi->aoff)) ;
**/
		return(0);
	case STOR_FLOAT :
		edtnum();
		*(float *)(actudr+cfi->aoff) = atof(ednum);
		return(0);
	case STOR_DOUBLE :
		edtnum();
		*(double *)(actudr+cfi->aoff) = atof(ednum);
		return(0);
	default :
		fomintlerr(15)
	}
	}

convtoa(vp,cp)	/* convert unpacked value *vp to reqd type for cf and
			store it in recast *cp */
char *vp,*cp;{

	switch (cfi->stortype){
	case STOR_CHAR :
		cpcbs(vp,cf->dfsize,cp);
		return(0);
	case STOR_SHORT :
		editvar(vp);
		*(short *)cp = atoi(cvelm(eddata));
		return(0);
	case STOR_INT :
		if (cf->fldtyp == TYP_YN)
			*(int *)cp =
				(*vp == 'Y' || *vp == 'y' ? TRUE : FALSE);
		else {
			editvar(vp);
			*(int *)cp = atoi(cvelm(eddata));
			}
		return(0);
	case STOR_LONG :
		editvar(vp);
		*(long *)cp = atol(cvelm(eddata));
		return(0);
	case STOR_FLOAT :
		editvar(vp);
		*(float *)cp = atof(cvelm(eddata));
		return(0);
	case STOR_DOUBLE :
		editvar(vp);
		*(double *)cp = atof(cvelm(eddata));
		return(0);
	default :
		fomintlerr(15)
	}
	}

editvar(vp)	/* version of edit that takes the value to be edited as arg */
char *vp; {
  char *ptr1b,*ptr2b,*ptr3b,*ptr1e,*ptr2e,*ptr3e;
  char mchar;
  char *ptr;
  int value,sign;
  ptr1b=vp;
  ptr1e=vp+cf->dfsize-1;/* Points to end of data area */
  ptr2b=fvp+cf->dmask;

  for(ptr2e=ptr2b;*ptr2e!='\0';ptr2e++);
  ptr2e--; /* Points to end of display mask */
  ptr3b=fvp+(cf->eddata);
  ptr3e=fvp+cf->eddata+cfi->oscrnsz-1;
  mchar=cf->maskchar;

  sign=0; /* Unsigned */
 value= *ptr1e; /* Save this value */
  if((cf->fldtyp==TYP_NUM &&cfi->Signed==1))
          if(*ptr1e=='{'){
                sign='+';
                *ptr1e='0';
           }
         else if(*ptr1e=='}') {
                  sign='-';
                  *ptr1e='0';
            }
         else if(*ptr1e>='A' && *ptr1e<='I') {
                  sign='+';
                  *ptr1e= *ptr1e-'A'+1+'0';
               }
       else if(*ptr1e>='J' && *ptr1e<='R') {
                 sign='-';
                 *ptr1e= *ptr1e-'J'+1+'0';
           }


      else if (*ptr1e>='0' && *ptr1e<='9') 
              sign='+';
 if(sign!=0) { /* It is a signed field */
   while(*ptr2b!=mchar) 
          *ptr3b++ = *ptr2b++ ; /* Copy the filler characters */
          *ptr3b++=sign;
    ptr2b++;
 }
       while(ptr1b<=ptr1e) {
           /* Find a posn for the chara and insert */
        while(*ptr2b!=mchar)
           *(ptr3b++) = *(ptr2b++);
	  if(ptr2b <= ptr2e)   ;
		else  return(2);
          *(ptr3b++) = *(ptr1b++);
      ptr2b++;
         }
  if(sign!=0) *ptr1e=value;
  if(cf->fldtyp==TYP_NUM && ((cf->lattr1)&LA_SUP))
   /* suppress leading zeroes in edited field */
                 for(ptr1b=fvp+cf->eddata;ptr1b<=ptr3e;ptr1b++) {
                           if(*ptr1b==COB_ZEROE) *ptr1b=COB_SPACE;
                           else if((*ptr1b<='9' && *ptr1b>'0') ||*ptr1b=='.') break;
                    }
  return(0);
}

