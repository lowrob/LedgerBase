#include <stdio.h>
#include <ctype.h>
#include "fomlink.h"
#include "cfomstrc.h"
#include "cfomfrm.h"
#include "PROFOMF.sth"
struct fe_struct dr;
extern struct linkstr *curp, *first;
extern int modified, errnumb;
extern struct frmhdr hdrrc;
extern struct stat_rec statrec;
fldedit()
{
struct linkstr *temp;
int flag, item, found;
curp=first;
flag=1;  
found = 0;
strcpy(statrec.scrnam, "/usr/bin/PROFOMF");
statrec.nextfld = 100;
statrec.endfld=3900;
filldr(1);
fomwr((char *) &dr);
errext(&statrec);
for( ; flag; )	{
	filldr(1);
	statrec.nextfld=3900;
	fomrf((char *) &dr);
	errext(&statrec);
	if(item=isitem(dr.fe3900))   { itmedit(item);  modified = 1;  }
	else	
	switch(item=isopt(dr.fe3900))	{
	case 1:			/* ADD AT CURRENT FIELD  */
	     if(addfld() == 0)  /* Changed From NULL To 0 For SCO */
	     	if(errnumb == 11)	{
	     	fomer("DUPLICATE FIELD CAN'T INSERT");
	     	break;	}
	     	else return;
	     modified = 1;
	     break;
	case 2:			/* REMOVE CURRENT FIELD */
	     remfld();
	     modified = 1;
	     break;
	case 3:			/* EDIT SCREEN LEVEL ATTRIBUTES */
	     scredit(1);
	     strcpy(statrec.scrnam, "/usr/bin/PROFOMF");
	     break;
	case 4:			/* DISPLAY NEXT FIELD  */
	     curp=curp->nexp;
	     statrec.nextfld=1;
	     statrec.endfld=0;
	     filldr(1);
	     statrec.nextfld=1;
	     statrec.endfld=0;
	     fomwr((char *) &dr);
	     errext(&statrec);
	     break;
	case 5:			/* DISPLAY PREVIOUS FIELD */
	     curp=curp->prep;
	     statrec.nextfld=1;
	     statrec.endfld=0;
	     filldr(1);
	     fomwr((char *) &dr);
	     errext(&statrec);
	     break;
	case 6:			/* DISPLAY A FIELD */
	     statrec.nextfld = 200;
	     dr.fe200 = LV_LONG;
	     fomrf((char *) &dr);
	     errext(&statrec);	
	     temp=curp;
	     do   {
	     	  if(curp->fldn==dr.fe200) 
	     	  { 	found=1; 
	     	  	break;  }
	     	  curp=curp->nexp;
	     	  }  while (curp != temp);
	     if(!found) 
	     {  fomer("FIELD NOT FOUND"); 
	     	break;  }
	     found = 0;
	     statrec.nextfld=1;
	     statrec.endfld=0;
	     filldr(1);
	     fomwr((char *) &dr);
	     errext(&statrec);
	     break;
	case 7:			/* END */
	     flag=0;
	     break;
	case 8:			/* DISPLAY SCREEN */
	     if((errnumb=dispform()) != 0)
	     	errnumb=1;
	     if(errnumb==0)
	     	strcpy(dr.fe4000, "SHOW SCREEN DONE");
	     else
	     	strcpy(dr.fe4000, "SHOW SCREEN NOT DONE");
	     statrec.nextfld = 4000;
	     fomwf((char *) &dr);
	     errext(&statrec);
	     break;
	}		/* end of switch */
    }		/* end of for */
}	/* end  */




isopt(option)
char option[];
{
int i;
static char *optcode[] = {
	"DU",
	"AD",
	"RE",
	"SC",
	"NE",
	"PR",
	"FI",
	"EN",
	"DI"		  };
if(option[0] == '\0')
	return(4);
for(i=0;i<9;i++)
	if(strcmp(option, optcode[i]) == 0) return(i);
if (i>=9) return(0);
}


isitem(option)
char option[];
{
if(isdigit(option[0]))
	return(atoi(option));
else 	return(0);
}

filldr(ind)	
int ind;
{
int IVAL, CVAL;
long ILONG;
extern struct fe_struct dr;
char s[100];
dr.fe4000[0] = HV_CHAR;
strcpy(dr.fe100, hdrrc.scrnnam);
if (ind == 0) { IVAL = LV_SHORT;
		ILONG = LV_LONG;
	        CVAL = LV_CHAR;   }
else
if (ind == 2) { IVAL = HV_SHORT;
		ILONG = HV_LONG;
		CVAL = HV_CHAR;   }

if (ind != 1)  {	/* move low or high values */
dr.fe200 = dr.fe500 = dr.fe1100 = ILONG;
dr.fe2000 = IVAL;
dr.fe150[0] = CVAL;
dr.fe300[0] = dr.fe600[0] = dr.fe750[0] = dr.fe800[0] = dr.fe900[0] = dr.fe1200[0] = dr.fe1400[0] = CVAL;

dr.fe1600[0] = dr.fe1700[0] = CVAL;
dr.fe2100[0]= dr.fe2400[0] = dr.fe2500[0] = dr.fe2600[0] = dr.fe2700[0] = dr.fe2800[0] = dr.fe2900[0] = CVAL;
dr.fe3000[0]=dr.fe3100[0]=dr.fe3200[0]=dr.fe3300[0]=dr.fe3400[0]=CVAL;
dr.fe3500[0]=dr.fe3600[0]=dr.fe3700[0]=dr.fe3800[0]=dr.fe3900[0]=CVAL;
return;      }

else	     {
clst(curp->fldna, dr.fe300);
clst(curp->promp, dr.fe600);
comp(s, curp->imas);
clst(s, dr.fe900);
comp(s,curp->dmas);
clst(s, dr.fe1200);
clst(curp->helpm, dr.fe1400);
clst(curp->lboun, dr.fe1600);
clst(curp->uboun, dr.fe1700);
clst(curp->dupva, dr.fe2100);

dr.fe1900[0] = lat(2, LA_SHODUP);
dr.fe2400[0] = lat(1, LA_REQ);
dr.fe2500[0] = lat(1, LA_SUP);
dr.fe2600[0] = vat(VA_ULINE);
dr.fe2700[0] = vat(VA_DIM);
dr.fe2800[0] = lat(1, LA_VALID);
dr.fe2900[0] = lat(2, LA_BOUNDS);
dr.fe3000[0] = vat(VA_REVERSE);
dr.fe3100[0] = vat(VA_BLINK);
dr.fe3200[0] = lat(1, LA_UESC);
dr.fe3300[0] = lat(2, LA_FH);
dr.fe3400[0] = vat(VA_BOLD);
dr.fe3500[0] = lat(1, LA_NOECHO);
dr.fe3600[0] = lat(2, LA_UCASE);
dr.fe3700[0] = lat(1, LA_HRET);
dr.fe3800[0] = lat(2, LA_LCASE);


dr.fe200 = curp->fldn;
dr.fe500 = curp->prox * 1000 + curp->proy;
dr.fe1100 = curp->flx * 1000 + curp->fly;

if(curp->fldcla == CL_PROM)
	strcpy(dr.fe150, "P");
else
if(curp->fldcla == CL_FLD)
	strcpy(dr.fe150, "F");
else
strcpy(dr.fe150, "B");
dr.fe750[0] = curp->machar;
dr.fe750[1] = '\0';

if (curp->fldty == TYP_STRING) strcpy(dr.fe800, "STRING");
else  if (curp->fldty == TYP_NUM) strcpy(dr.fe800, "NUMERIC");
else  if (curp->fldty == TYP_YN) strcpy(dr.fe800, "BOOLEAN");
else  if (curp->fldty == TYP_DATE) strcpy(dr.fe800, "DATE");
else  strcpy(dr.fe800, "-None-");
dr.fe2000 = curp->dupct;
dr.fe3900[0] = LV_CHAR;
return;     }
}

clst(s1, s2)	/* copy linked structure fields into dr */
char *s1, *s2;
{
if(s1 != NULL)
	strcpy(s2, s1);
else strcpy(s2, "--None--");
return;
}

lat(flag, mask)

int flag, mask;
{
int *val;
if (flag == 1) 
	val = &curp->latt1;
else    val = &curp->latt2;

if (*val & mask) return('Y');
else return('N');
}


vat(mask)
int mask;
{
int pfl, mfl;
if (curp->promv & mask) 
	pfl = 1; else
	pfl = 0;
if (curp->fldv & mask)
	mfl = 1; else
	mfl = 0;
if (pfl && mfl) return('B'); else 
if (pfl)	return('P'); else
if (mfl)	return('F'); else
		return('N');
	
}
