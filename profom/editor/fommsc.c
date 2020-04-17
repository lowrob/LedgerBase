#define NO_ERROR 0
#define ERROR -2
#include <stdio.h>
#include <ctype.h>
#include "cfomfrm.h"
char *imaskptr,*dmaskptr;
char expdmsk[100],expimsk[100];
char maskchar;
int ichar,dchar;
int datsz; /* size of data record */

valmask(type, masch, imask, dmask) 
int type;
char masch, *imask, *dmask;
{
maskchar = masch;
 datsz=0;  /* dfsize */
if((   expndmsk(imask,expimsk))==ERROR) return(ERROR);
if((   expndmsk(dmask,expdmsk))==ERROR)return(ERROR);
   dmaskptr=expdmsk;
   imaskptr=expimsk;
  if(type==TYP_NUM) return(prcnum());
  if(type==TYP_STRING) return(prcstrng());
  if(type==TYP_DATE)return(prcdt());
 if(type==TYP_YN)return(prcbool());
  return(ERROR);
 }






prcstrng() {
  int prevchar,maskfound;
  prevchar=0;
  maskfound=0;
  
  while((ichar= *imaskptr)!='\0') {
     if((dchar= *dmaskptr)=='\0') return(ERROR);

     switch(ichar) {
        case 'F'  :
                   if(dchar==maskchar ||dchar=='('||dchar==')') return(ERROR);
   
                   break;

        case 'X' :if(dchar!=maskchar) return(ERROR);
                  maskfound=1;
                  datsz++;
                  break;

	default:return(ERROR);
       } /*switch */

       prevchar= *imaskptr;
       imaskptr++;
       dmaskptr++;
   } /* while */

  if(*dmaskptr!='\0')return(ERROR);
  if(maskfound==0) return(ERROR);
  return(datsz);


} /* prcstrng() */

prcnum() {
 int zerosup,dotfound,maskfound,prevchar;
 int signfound;
 dotfound=0;
 zerosup=signfound=maskfound=prevchar=0;
 while((ichar= *imaskptr)!='\0') {
   if((dchar= *dmaskptr)=='\0') return(ERROR);

   switch(ichar) {
       case 'F' :
                if(dchar==maskchar ||dchar=='(' ||dchar==')' || dchar=='.' || dchar=='s' ||dchar=='S') return(ERROR);
                break;
       case '.':
       case 'v':
       case 'V': if(dchar!='.') return(ERROR);
                 if(dotfound==1) return(ERROR);
                 dotfound=1;
                 break;

       case 's':
       case 'S':
                if(dchar!=maskchar) return(ERROR);
                if(maskfound==1 || dotfound==1)return(ERROR);
                if(signfound==1) return(ERROR);
                signfound=1;
                break;


        case '9':if(dchar!=maskchar) return(ERROR);
                 if(maskfound==1 && zerosup==0) return(ERROR);
                 maskfound=1;
                 datsz++;
                 zerosup=1;
                 break;

        case '0': if(dchar!=maskchar) return(ERROR);
                  if(maskfound==1 && zerosup==1) return(ERROR);
                   zerosup=0;
                  maskfound=1;
                  datsz++;
                  break;

        default: return(ERROR);
      } /* switch  */

     prevchar= *imaskptr;
     dmaskptr++;
     imaskptr++;
  } /* while */

  if(*dmaskptr!='\0') return(ERROR);
  if(maskfound==0 || prevchar=='V') return(ERROR);
  return(datsz);
 } /* prcnum */


#define ERROR -2
expndmsk(maskptr,dstptr)
 char *maskptr,*dstptr;
{
 char *ptr;
 int masklength;
 int i,val;
 char *ptr1,*ptr2;
 char temp;
 
 ptr1=maskptr;
 ptr2=dstptr;
 masklength=0;
 while(*ptr1)
   if(*ptr1=='(') {
         for(ptr=ptr1+1;*ptr!=')' && *ptr!='\0';ptr++)
                        if(!isdigit(*ptr))return(ERROR);
          if(*ptr!=')') return(ERROR);
         if(ptr1==maskptr) return(ERROR);
        temp=(*(ptr1-1));
       ptr1++;
         sscanf(ptr1,"%d",&val);
      ptr1++;
      ptr1++;
      if(val>9) ptr1++;
      masklength+=(val-1);
      if(masklength>80)return(ERROR);
      for(i=1;i<val;i++)
                 *ptr2++=temp;
    }

   else { *(ptr2++)=(*(ptr1));masklength++;if(masklength>80 || *ptr1>126 ||*ptr1<32 ) return(ERROR); ptr1++ ;}
  *ptr2= '\0';
  return(0);
}






prcdt() {

 int mfound,yrfound,datefound;
  int length;
 mfound=yrfound=datefound=0;

 while((ichar= *imaskptr)!='\0') {
       if((dchar= *dmaskptr)=='\0') return(ERROR);
        switch(ichar) {
             case 'm':
              case 'M':
                   if(mfound==1)return(ERROR);
                   length=getln();
                  if(length!=2 && length!=3) return(ERROR);
                    datsz+=length;
                  mfound=1;
                  break;


           case 'y':
           case 'Y':
                if(yrfound==1)return(ERROR);
      length=getln();
                     if(length!=4 && length!=2) return(ERROR);
                     yrfound=1;
                       datsz+=length;
                     break;

           case 'd':
           case 'D': 
                     if(datefound==1)return(ERROR);
                     length=getln();
                     if(length!=2) return(ERROR);
                     datefound=1;
                     datsz+=length;

                     break;
         case 'F' :
                  if(dchar==maskchar) return(ERROR);
                  imaskptr++;
                  dmaskptr++;
		  break;
	default:
		return(ERROR);
     }/* switch */
  } /* while */

 if(yrfound==0 ||mfound==0 || datefound==0) return(ERROR);
 if(*dmaskptr!='\0') return(ERROR);
  return(datsz);
 
} /* prcdt() */



 getln(){
             int length;
                    length=0;
                while(*imaskptr==ichar){

                         if(*dmaskptr!=maskchar)return(ERROR);
                         length++;
                         dmaskptr++;
                          imaskptr++;
                     }

        return(length);
 }



prcbool() {
int maskfound;
maskfound=0;

while((ichar= *imaskptr)!='\0') {
    if((dchar= *dmaskptr)=='\0') return(ERROR);
   switch(ichar) {
      case 'F' : if(dchar==maskchar ) return(ERROR);
                 break;

     case  'b':
     case  'B':
                if(maskfound==1) return(ERROR);
                if(dchar!=maskchar) return(ERROR);
                maskfound=1;
                break;

     default: return(ERROR);
   }

   imaskptr++;
   dmaskptr++;
 }

if(maskfound==0) return(ERROR);
if(*dmaskptr!='\0') return(ERROR);
 return(1);
}



comp(s2, s1)
char s2[], s1[];
{
register char *s4, pchar;
register int i,k,j;
char s[100];
if(s1 == NULL)       {
	s2[0] = '\0';
	return;       }
s4=s2;
i=j=0;
for(; ;)
{	pchar=s1[i];	/* previous character is initialized */
	for(;s1[i]==pchar;i++)	/* count no. of repetitions */
		;
	j=i-j;
	if(j<5)		/* if < 5 don't pack them */
		for(k=1;k<=j;k++)
			*s4++ = pchar;
	else
	{	itoa(j,s);
		*s4++=pchar;
		*s4++='(';
		for(k=0;s[k]!='\0';k++)
			*s4++=s[k];
		*s4++=')';	}
	j=i;
	if(s1[i]=='\0')
	{       *s4 = '\0';
		return;   }
	pchar=s1[i];
}
}

itoa(n,s)
char s[];
int n;
{
register int i,sign;
if((sign=n)<0)
	n = -n;
i=0;
do
{
s[i++] = n % 10 + '0';
}	while ((n /= 10) > 0);
if(sign<0)
	s[i++] = '-';
s[i] = '\0';
reverse(s);
}


reverse(s)
char s[];
{
register int c,i,j;
for(i=0,j=strlen(s)-1; i<j; i++,j--)  {
	c=s[i];
	s[i]=s[j];
	s[j]=c;
	}
}
expand(maskptr,dstptr)
 char *maskptr,*dstptr;
{
 char *ptr;
 int masklength;
 int i,val;
 char *ptr1,*ptr2;
 char temp;
 
 ptr1=maskptr;
 ptr2=dstptr;
 masklength=0;
 while(*ptr1)
   if(*ptr1=='(') {
         for(ptr=ptr1+1;*ptr!=')' && *ptr!='\0';ptr++)
                        if(!isdigit(*ptr))return(ERROR);
          if(*ptr!=')') return(ERROR);
         if(ptr1==maskptr) return(ERROR);
        temp=(*(ptr1-1));
       ptr1++;
         sscanf(ptr1,"%d",&val);
      ptr1++;
      ptr1++;
      if(val>9) ptr1++;
      masklength+=(val-1);
      if(masklength>80)return(ERROR);
      for(i=1;i<val;i++)
                 *ptr2++=temp;
    }

   else { *(ptr2++)=(*(ptr1));masklength++;if(masklength>80 || *ptr1>126 ||*ptr1<32 ) return(ERROR); ptr1++ ;}
  *ptr2= '\0';
  return(0);
}

static int flag=0;
static char carct;

vgetc(fptr)
FILE *fptr;
{
if(flag==0)
	 return(carct=getc(fptr));
else
{
	flag=0;
	return(carct);
}

}



vungetc(chr,fptr)
char chr;
FILE *fptr;
{
carct = chr;
flag=1;
if(carct==EOF) return(EOF);
	return(0);
}
#include "stdio.h"
char *starcpy(t,s)
char *s, *t;
{
char *tmp;
tmp=t;
while (*tmp++ = *s++)
	;
return(t);
}

char *starcat(t,s)
char *t, *s;
{
char *tmp;
tmp=t;
while(*tmp)
	tmp++;
while(*tmp++ = *s++)
	;
return(t);
}
