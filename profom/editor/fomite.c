#define LAT1(A) curp->latt1=((curp->latt1&A)?1:0)?curp->latt1&~A:curp->latt1|A
#define LAT2(A) curp->latt2=((curp->latt2&A)?1:0)?curp->latt2&~A:curp->latt2|A
#define PATT(A) curp->promv=curp->promv|A
#define FATT(A) curp->fldv=curp->fldv|A
#define RAT1(A) curp->promv=curp->promv&~A
#define RAT2(A) curp->fldv=curp->fldv&~A
#include "cfomstrc.h"
#include "fomlink.h"
#include "PROFOMF.sth"
#include "cfomfrm.h"
#include <stdio.h>
static char temchar;
extern int errnumb;
itmedit(option)		/* edit item */

int option;
{
char  s[100], *chdmsk(), *tempar, *strsave(), *sarr, *suplzer(), *suptbl();		
     	
int j, val,success,done,jj;
extern struct stat_rec statrec;
extern struct linkstr *curp;
extern struct fe_struct dr;

if(curp->fldcla == CL_PROM)	{
	switch(option)	{
	case 4:
	case 3:
	case 18:
	case 22:
	case 21:
	case 17:
	case 25:
	case 31:
		break;
	default:
		strcpy(dr.fe4000, "ILLEGAL OPTION: PROMPT ONLY FIELD");
		statrec.nextfld = 4000;
		fomwf((char *) &dr);
		errext(&statrec);
		return;	}	}

else
if(curp->fldcla == CL_FLD)	{
	switch(option)	{
	case 4:
	case 3:
		strcpy(dr.fe4000,"ILLEGAL OPTION: MASK ONLY FIELD");
		statrec.nextfld = 4000;
		fomwf((char *) &dr);
		errext(&statrec);
		return;
	default:
		break;	}	}
dr.fe4000[0] = HV_CHAR;
statrec.nextfld = 4000;
fomwf((char *) &dr);
errext(&statrec);

success=0;	
switch (option) {	/* convert item (two digits) into integer */
case 31:
	while(!success)       {
	statrec.nextfld = 150;
	dr.fe150[0] = LV_CHAR;
	fomrf((char *) &dr);
	errext(&statrec);
	if(dr.fe150[0] == 'P')	curp->fldcla = CL_PROM; else
	if(dr.fe150[0] == 'F')	curp->fldcla = CL_FLD;  else
	if(dr.fe150[0] == 'B')	curp->fldcla = CL_PRMFLD; else
	{	strcpy(dr.fe4000,"P(PROMPT) F(MASK) OR B(BOTH) ONLY!RETRY!");
		statrec.nextfld = 4000;
		fomwf((char *) &dr);
		errext(&statrec);
		continue;   }
	success=1;   }
	switch(curp->fldcla)	{
	case CL_PROM:
		curp->fldna = curp->imas = curp->dmas = curp->helpm = NULL;
		curp->lboun = curp->uboun = curp->dupva = NULL;
		curp->fldty = curp->flx = curp->fly = curp->dupct = 0;
		curp->latt1 = curp->latt2 = curp->dfsiz = 0;
		break;
	case CL_FLD:
		curp->promp = NULL;
		curp->prox = curp->proy = 0;
		break;
	default:
		break;	}
		
	break;
case 1:
{	fomer ("FIELD NUMBER CHANGE NOT ALLOWED");
	break;	}
	
case 2:			/* read field name from screen */
	statrec.nextfld = 300;
	dr.fe300[0] = LV_CHAR;
	fomrf((char *) &dr);	/* read field name from screen */
	errext(&statrec);
	curp->fldna = strsave(dr.fe300); 
				     
	break;		/* strsave copies item into a place */
			/* returns a pointer to it */
			
case 4:			/* read prompt from screen */ 
	statrec.nextfld = 600;
	dr.fe600[0] = LV_CHAR;
	fomrf((char *) &dr);
	errext(&statrec);
	curp->promp = strsave(dr.fe600);
	break;
case 3:			/* prompt coordinates */
	while(!success)    {
	statrec.nextfld = 500;
	dr.fe500 = LV_LONG;
	fomrf((char *) &dr);
	errext(&statrec);
	if(dr.fe500 == 0)
	{	strcpy(dr.fe4000, "PROMPT COORDINATES CAN'T BE ZERO");
		statrec.nextfld = 4000;
		fomwf((char *) &dr);
		errext(&statrec);
		continue;   }
	success=1;    }
	curp->prox = dr.fe500/1000; 
	curp->proy = dr.fe500 - (dr.fe500/1000) * 1000;
	break;
case 5:
	while(!success)    {
	statrec.nextfld = 800;
	dr.fe800[0] = LV_CHAR;
	fomrf((char *) &dr);
	errext(&statrec);
	switch(dr.fe800[0])  {	/* type of feild */
	
	case 'S':
		curp->fldty = TYP_STRING;
		break;
	case 'D':
		curp->fldty = TYP_DATE;
		break;
	case 'B':
		curp->fldty = TYP_YN;
		break;
	case 'N':
		curp->fldty = TYP_NUM;
		break;
	default:
		curp->fldty = TYP_NONE;
		break;	}
	curp->dupva = NULL;
	curp->lboun = NULL;
	curp->uboun = NULL;
	curp->dupct = DUP_NONE;
	curp->latt2 = curp->latt2 & ~LA_SHODUP;
	curp->latt2 = curp->latt2 & ~LA_BOUNDS;
	
case 6:			/* followed by input mask */
	done=0;
	while(!done)     {
	statrec.nextfld = 900;
	dr.fe900[0] = LV_CHAR;
	fomrf((char *) &dr);
	errext(&statrec);
	if(expand(dr.fe900, s)==-2)
	{	fomer("ERROR IN INPUT MASK");
		errnumb=12;
		continue;   }
	errnumb=0;
	done=1;   }	/* while(!done) */
	curp->imas = strsave(s);
	
case 8:			/* followed by display mask */
	done=0;
	while(!done)    { 
	statrec.nextfld = 1200;
	dr.fe1200[0] = LV_CHAR;
	fomrf((char *) &dr);
	errext(&statrec);
	if(expand(dr.fe1200, s)==-2)
	{	fomer("ERROR IN DISPLAY MASK");
		errnumb=13;
		continue;   }
	errnumb=0;
	done=1;    }	/* while(!done) */
	curp->dmas = strsave(s);
	
	if((curp->dfsiz = valmask(curp->fldty, curp->machar, curp->imas, curp->dmas)) == -2)
	{	strcpy(dr.fe4000, "INPUT/DISPLAY MASK ERROR");
		errnumb=14;
		statrec.nextfld = 4000;
		fomwf((char *) &dr);
		errext(&statrec);
		continue;	}
	success=1;     }	/* while(!success) */
	errnumb=0;
	break;
case 30:
	while(!success)    {
	statrec.nextfld = 750;
	dr.fe750[0] = LV_CHAR;
	fomrf((char *) &dr);
	errext(&statrec);
	temchar = curp->machar;
	curp->machar = dr.fe750[0];
	tempar = chdmsk(curp->machar, curp->dmas);
	curp->dmas = strsave(tempar);
	if((curp->dfsiz = valmask(curp->fldty, curp->machar, curp->imas, curp->dmas)) == -2)
	{	strcpy(dr.fe4000, "MASK CHARACTER,DISPLAY MASK MISMATCH");
		errnumb=13;
		statrec.nextfld = 4000;
		fomwf((char *) &dr);
		errext(&statrec);
		continue;	}
	success=1;    }
	errnumb=0;
	dr.fe4000[0]=HV_CHAR;
	comp(s, curp->dmas);
	strcpy(dr.fe1200, s);
	statrec.nextfld=1200;
	fomwf((char *) &dr);
	errext(&statrec);
	break;
	
case 7:
	while(!success)    {
	statrec.nextfld = 1100;
	dr.fe1100 = LV_LONG;
	fomrf((char *) &dr);
	errext(&statrec);
	if(dr.fe1100 == 0)	
	{	strcpy(dr.fe4000, "MASK COORDINATES CAN'T BE ZERO");
		statrec.nextfld = 4000;
		fomwf((char *) &dr);
		errext(&statrec);
		continue;	}
	success=1;  }
	curp->flx = dr.fe1100 / 1000;
	curp->fly = dr.fe1100 - (dr.fe1100/1000) * 1000;
	break;
case 9:
	statrec.nextfld = 1400;
	dr.fe1400[0] = LV_CHAR;
	fomrf((char *) &dr);
	errext(&statrec);
	if(statrec.fillcode==FIL_OMITTED)
		curp->helpm = NULL;
	else
	if(dr.fe1400[0] == '\0')
		fill(dr.fe1400,curp->dfsiz);
	curp->helpm = strsave(dr.fe1400);
	break;
case 10:
	while(!success)    {
	statrec.nextfld = 1600;
	dr.fe1600[0] = LV_CHAR;
	fomrf((char *) &dr);
	errext(&statrec);
	if(strlen(dr.fe1600) > curp->dfsiz)
	{	fomer("FIELD SIZE TOO LARGE");
		continue;      }
	if(statrec.fillcode == FIL_OMITTED)
		curp->lboun = NULL;
	else   {
	if(dr.fe1600[0] != '\0')
	{   	sarr = strsave(dr.fe1600);
		if(curp->fldty == TYP_NUM)
			curp->lboun = suplzer(sarr,curp->dfsiz);
		else
		if(curp->fldty == TYP_STRING)
			curp->lboun = suptbl(sarr,curp->dfsiz);  }
	else
	{	fill(dr.fe1600,curp->dfsiz);
		curp->lboun = strsave(dr.fe1600);     }   }  /* else */
	if(curp->lboun != NULL)
		if(strlen(curp->lboun) != curp->dfsiz)
		{	curp->lboun=NULL;
			strcpy(dr.fe4000, "LOWER BOUNDS SIZE ERROR");
			statrec.nextfld = 4000;
			fomwf((char *) &dr);
			errext(&statrec);
			continue;	}
	success=1;    }
	if(curp->lboun == NULL)
		strcpy(dr.fe1600,"- None -");
	else
		strcpy(dr.fe1600,curp->lboun);
	statrec.nextfld=1600;
	fomwf((char *) &dr);
	errext(&statrec);
	
	success=0;
	while(!success)   {
	statrec.nextfld = 1700;
	dr.fe1700[0] = LV_CHAR;
	fomrf((char *) &dr);
	errext(&statrec);
	if(strlen(dr.fe1700) > curp->dfsiz)
	{	fomer("FIELD SIZE TOO LARGE");
		continue;      }
	if(statrec.fillcode == FIL_OMITTED)
		curp->uboun = NULL;
	else   {
	if(dr.fe1700[0] != '\0')
	{   	sarr = strsave(dr.fe1700);
		if(curp->fldty == TYP_NUM)
			curp->uboun = suplzer(sarr,curp->dfsiz);
		else
		if(curp->fldty == TYP_STRING)
			curp->uboun = suptbl(sarr,curp->dfsiz);  }
	else
	{	fill(dr.fe1700,curp->dfsiz);
		curp->uboun = strsave(dr.fe1700);     }   }  /* else */
	if(curp->uboun != NULL)
		if(strlen(curp->uboun) != curp->dfsiz)
		{	curp->uboun=NULL;
			strcpy(dr.fe4000, "UPPER BOUNDS SIZE ERROR");
			statrec.nextfld = 4000;
			fomwf((char *) &dr);
			errext(&statrec);
			continue;	}
	success=1;     }
	
	if(curp->uboun == NULL)
		strcpy(dr.fe1700, "- None -");
	else
		strcpy(dr.fe1700,curp->uboun);
	statrec.nextfld=1700;
	fomwf((char *) &dr);
	errext(&statrec);
	break;
case 13:
	while(!success)    {
	statrec.nextfld = 2100;
	dr.fe2100[0] = LV_CHAR;
	fomrf((char *) &dr);
	errext(&statrec);
	if(strlen(dr.fe2100) > curp->dfsiz)
	{	fomer("FIELD SIZE TOO LARGE");
		continue;      }
	if(statrec.fillcode == FIL_OMITTED)
		curp->dupva = NULL;
	else   {
	if(dr.fe2100[0] != '\0')
	{   	sarr = strsave(dr.fe2100);
		if(curp->fldty == TYP_DATE)   {
			jj=0;
			for( ;*(curp->imas) != '\0';)   {
				if(*(curp->imas) =='M' || *(curp->imas) == 'm')
					jj++;
				curp->imas++;
			}
			if(jj<=2)
				curp->dupva=suplzer(sarr,curp->dfsiz);
			else
				curp->dupva=suptbl(sarr,curp->dfsiz);
		}    else
		if(curp->fldty == TYP_NUM)
			curp->dupva = suplzer(sarr,curp->dfsiz);
		else
		if(curp->fldty == TYP_STRING)
			curp->dupva = suptbl(sarr,curp->dfsiz);  }
	else
	{	fill(dr.fe2100,curp->dfsiz);
		curp->dupva = strsave(dr.fe2100);     }   }  /* else */
	if(curp->dupva != NULL)
		if(strlen(curp->dupva) != curp->dfsiz)
		{	curp->dupva=NULL;
			strcpy(dr.fe4000, "DUP VALUE SIZE ERROR");
			statrec.nextfld = 4000;
			fomwf((char *) &dr);
			errext(&statrec);
			continue;	}
	success=1;    }
	
	if(curp->dupva == NULL)
		strcpy(dr.fe2100, "- None -");
	else
		strcpy(dr.fe2100,curp->dupva);
	statrec.nextfld=2100;
	fomwf((char *) &dr);
	errext(&statrec);
	break;
case 14:
	statrec.nextfld = 2000;
	dr.fe2000 = LV_SHORT;
	fomrf((char *) &dr);
	errext(&statrec);
	curp->dupct = dr.fe2000;
	break;
case 12:
	LAT2(LA_SHODUP);
	dr.fe1900[0] = lat(2, LA_SHODUP);
	statrec.nextfld = 1900;
	fomwf((char *) &dr);
	errext(&statrec);
	break;
case 15:
	LAT1(LA_REQ);
	dr.fe2400[0] = lat(1, LA_REQ);
	statrec.nextfld = 2400;
	fomwf((char *) &dr);
	errext(&statrec);
	break;
case 19:
	LAT1(LA_VALID);
	dr.fe2800[0] = lat(1, LA_VALID);
	statrec.nextfld = 2800;
	fomwf((char *) &dr);
	errext(&statrec);
	break;
case 23:
	LAT1(LA_UESC);
	dr.fe3200[0] = lat(1, LA_UESC);
	statrec.nextfld = 3200;
	fomwf((char *) &dr);
	errext(&statrec);
	break;
case 26:
	LAT1(LA_NOECHO);
	dr.fe3500[0] = lat(1, LA_NOECHO);
	statrec.nextfld = 3500;
	fomwf((char *) &dr);
	errext(&statrec);
	break;
case 28:
	LAT1(LA_HRET);
	dr.fe3700[0] = lat(1, LA_HRET);
	statrec.nextfld = 3700;
	fomwf((char *) &dr);
	errext(&statrec);
	break;
case 16:
	LAT1(LA_SUP);
	dr.fe2500[0] = lat(1, LA_SUP);
	statrec.nextfld = 2500;
	fomwf((char *) &dr);
	errext(&statrec);
	break;
case 20:
	LAT2(LA_BOUNDS);
	dr.fe2900[0] = lat(2, LA_BOUNDS);
	statrec.nextfld = 2900;
	fomwf((char *) &dr);
	errext(&statrec);
	break;
case 24:
	LAT2(LA_FH);
	dr.fe3300[0] = lat(2, LA_FH);
	statrec.nextfld = 3300;
	fomwf((char *) &dr);
	errext(&statrec);
	break;
case 27:
	LAT2(LA_UCASE);
	dr.fe3600[0] = lat(2, LA_UCASE);
	statrec.nextfld = 3600;
	fomwf((char *) &dr);
	errext(&statrec);
	break;
case 29:
	LAT2(LA_LCASE);
	dr.fe3800[0] = lat(2, LA_LCASE);
	statrec.nextfld = 3800;
	fomwf((char *) &dr);
	errext(&statrec);
	break;
case 18:
	success=0;
	while(!success)   {
	statrec.nextfld = 2700;
	dr.fe2700[0] = LV_CHAR;
	fomrf((char *) &dr);
	errext(&statrec);
	if(curp->fldcla == CL_PROM)
		if(dr.fe2700[0] != 'P'  && dr.fe2700[0] != 'N')
		{	strcpy(dr.fe4000, "ONLY P OR N ALLOWED");
			statrec.nextfld = 4000;
			fomwf((char *) &dr);
			errext(&statrec);
			continue;	}
		else 	success=1;	else
	if(curp->fldcla == CL_FLD)
		if(dr.fe2700[0] != 'F' && dr.fe2700[0] != 'N')
		{	strcpy(dr.fe4000, "ONLY F OR N ALLOWED");
			statrec.nextfld = 4000;
			fomwf((char *) &dr);
			errext(&statrec);
			continue;	}
		else    success=1;	else
	if(dr.fe2700[0] != 'P' && dr.fe2700[0] != 'B' && dr.fe2700[0] != 'F' && dr.fe2700[0] != 'N')
		{	strcpy(dr.fe4000, "ONLY P F B OR N ALLOWED");
			statrec.nextfld = 4000;
			fomwf((char *) &dr);
			errext(&statrec);
			continue;    }
		else  success=1;	}
	if(dr.fe2700[0]=='P') { PATT(VA_DIM); RAT2(VA_DIM);  }
	if(dr.fe2700[0]=='F') { FATT(VA_DIM); RAT1(VA_DIM);  }
	if(dr.fe2700[0]=='B') { PATT(VA_DIM); FATT(VA_DIM);  }
	if(dr.fe2700[0]=='N') { RAT1(VA_DIM); RAT2(VA_DIM);  }
	break;
case 22:
	success=0;
	while(!success)   {
	statrec.nextfld = 3100;
	dr.fe3100[0] = LV_CHAR;
	fomrf((char *) &dr);
	errext(&statrec);
	if(curp->fldcla == CL_PROM)
		if(dr.fe3100[0] != 'P' && dr.fe3100[0] != 'N')
		{	strcpy(dr.fe4000, "ONLY P OR N ALLOWED");
			statrec.nextfld = 4000;
			fomwf((char *) &dr);
			errext(&statrec);
			continue;	}
		else 	success=1;	else
	if(curp->fldcla == CL_FLD)
		if(dr.fe3100[0] != 'F' && dr.fe3100[0] != 'N')
		{	strcpy(dr.fe4000, "ONLY F OR N ALLOWED");
			statrec.nextfld = 4000;
			fomwf((char *) &dr);
			errext(&statrec);
			continue;	}
		else 	success=1;	else
	if(dr.fe3100[0] != 'P' && dr.fe3100[0] != 'B' && dr.fe3100[0] != 'F' && dr.fe3100[0] != 'N')
		{	strcpy(dr.fe4000, "ONLY P F B OR N ALLOWED");
			statrec.nextfld = 4000;
			fomwf((char *) &dr);
			errext(&statrec);
			continue;	}
		else    success=1;    }
	if(dr.fe3100[0]=='P') { PATT(VA_BLINK); RAT2(VA_BLINK);  }
	if(dr.fe3100[0]=='F') { FATT(VA_BLINK); RAT1(VA_BLINK);  }
	if(dr.fe3100[0]=='B') { FATT(VA_BLINK); PATT(VA_BLINK);  }
	if(dr.fe3100[0]=='N') { RAT2(VA_BLINK); RAT1(VA_BLINK);  }
	break;	
case 21:
	success = 0;
	while(!success)    {
	statrec.nextfld = 3000;
	dr.fe3000[0] = LV_CHAR;
	fomrf((char *) &dr);
	errext(&statrec);
	if(curp->fldcla == CL_PROM)
		if(dr.fe3000[0] != 'P' && dr.fe3000[0] != 'N')
		{	strcpy(dr.fe4000, "ONLY P OR N ALLOWED");
			statrec.nextfld = 4000;
			fomwf((char *) &dr);
			errext(&statrec);
			continue;	}
		else 	success=1;	else
	if(curp->fldcla == CL_FLD)
		if(dr.fe3000[0] != 'F' && dr.fe3000[0] != 'N')
		{	strcpy(dr.fe4000, "ONLY F OR N ALLOWED");
			statrec.nextfld = 4000;
			fomwf((char *) &dr);
			errext(&statrec);
			continue;	}
		else 	success=1;	else
	if(dr.fe3000[0] != 'P' && dr.fe3000[0] != 'B' && dr.fe3000[0] != 'F' && dr.fe3000[0] != 'N')
		{	strcpy(dr.fe4000, "ONLY P F B OR N ALLOWED");
			statrec.nextfld = 4000;
			fomwf((char *) &dr);
			errext(&statrec);
			continue;	}
		else success=1;   }
	if(dr.fe3000[0]=='P') {  PATT(VA_REVERSE); RAT2(VA_REVERSE);  }
	if(dr.fe3000[0]=='F') {  FATT(VA_REVERSE); RAT1(VA_REVERSE);  }
	if(dr.fe3000[0]=='B') {  PATT(VA_REVERSE); FATT(VA_REVERSE);  }
	if(dr.fe3000[0]=='N') {  RAT1(VA_REVERSE); RAT2(VA_REVERSE);  }
	break;
case 17:
	success = 0;
	while(!success)    {
	statrec.nextfld = 2600;
	dr.fe2600[0] = LV_CHAR;
	fomrf((char *) &dr);
	errext(&statrec);
	if(curp->fldcla == CL_PROM)
		if(dr.fe2600[0] != 'P'  && dr.fe2600[0] != 'N')
		{	strcpy(dr.fe4000, "ONLY P OR N ALLOWED");
			statrec.nextfld = 4000;
			fomwf((char *) &dr);
			errext(&statrec);
			continue;	}
		else 	success=1;	else
	if(curp->fldcla == CL_FLD)
		if(dr.fe2600[0] != 'F'  && dr.fe2600[0] != 'N')
		{	strcpy(dr.fe4000, "ONLY F OR N ALLOWED");
			statrec.nextfld = 4000;
			fomwf((char *) &dr);
			errext(&statrec);
			continue;	}
	else 	success=1;	else
	if(dr.fe2600[0] != 'P' && dr.fe2600[0] != 'B' && dr.fe2600[0] != 'F'  && dr.fe2600[0] != 'N')
		{	strcpy(dr.fe4000, "ONLY P F B OR N ALLOWED");
			statrec.nextfld = 4000;
			fomwf((char *) &dr);
			errext(&statrec);
			continue;	}
		else success=1;   }
	if(dr.fe2600[0]=='P') {  PATT(VA_ULINE);  RAT2(VA_ULINE);  }
	if(dr.fe2600[0]=='F') {  FATT(VA_ULINE);  RAT1(VA_ULINE);  }
	if(dr.fe2600[0]=='B') {  PATT(VA_ULINE);  FATT(VA_ULINE);  }
	if(dr.fe2600[0]=='N') {  RAT1(VA_ULINE);  RAT2(VA_ULINE);  }
	break;
case 25:
	success = 0;
	while(!success)     {
	statrec.nextfld = 3400;
	dr.fe3400[0] = LV_CHAR;
	fomrf((char *) &dr);
	errext(&statrec);
	if(curp->fldcla == CL_PROM)
		if(dr.fe3400[0] != 'P' && dr.fe3400[0] != 'N')
		{	strcpy(dr.fe4000, "ONLY P OR N ALLOWED");
			statrec.nextfld = 4000;
			fomwf((char *) &dr);
			errext(&statrec);
			continue;	}
		else 	success=1;	else
	if(curp->fldcla == CL_FLD)
		if(dr.fe3400[0] != 'F' && dr.fe3400[0] != 'N')
		{	strcpy(dr.fe4000, "ONLY F OR N ALLOWED");
			statrec.nextfld = 4000;
			fomwf((char *) &dr);
			errext(&statrec);
			continue;	}
	else    success=1;      else
	if(dr.fe3400[0] != 'P' && dr.fe3400[0] != 'B' && dr.fe3400[0] != 'F' && dr.fe3400[0] != 'N')
		{	strcpy(dr.fe4000, "ONLY P F B OR N ALLOWED");
			statrec.nextfld = 4000;
			fomwf((char *) &dr);
			errext(&statrec);
			continue;	}
		else    success=1;    }
	if(dr.fe3400[0]=='P')   {  PATT(VA_BOLD); RAT2(VA_BOLD);  }
	if(dr.fe3400[0]=='F')   {  FATT(VA_BOLD); RAT1(VA_BOLD);  }
	if(dr.fe3400[0]=='B')   {  PATT(VA_BOLD); FATT(VA_BOLD);  }
	if(dr.fe3400[0]=='N')   {  RAT1(VA_BOLD); RAT2(VA_BOLD);  }
	break;
}	/* end of item no. change switch */


return;
}

char *chdmsk(nch, s1)
char nch, *s1;
{
char *s2, *s3, *malloc();
if((s2=malloc(strlen(s1)+1)) == NULL)
{	errnumb = 1; 
	return(NULL);	}
s3 = s2;
for( ; *s1!='\0'; )	{
	if(*s1==temchar)
		*s3 = nch;
	else
		*s3 = *s1;
	s1++;
	s3++;	}
*s3 = '\0';
errnumb = 0;
return(s2);
}


fill(strng,sze)
char strng[];
int sze;
{
int i;

for(i=0; i<sze; i++)
	strng[i]=' ';
strng[i] = '\0';
return;
}


char *suptbl(s,sz)
char s[];
int sz;
{
int i,k,l;
char *d,*tmp,*malloc();
d = malloc((sz+1));
if(d == NULL)  return(NULL);
strcpy(d, s);
tmp = d;
i=strlen(s);

if(i==sz)
    return(tmp);

if(i<sz)	{
	for(k=0; *(d+k) != '\0'; k++)
		;
	for(l=k;l<sz;l++)
		*(d+l)=' ';
	*(d+l)='\0';
	return(tmp);	}
return(NULL);
	
}



char *suplzer(s,sz)
char s[];
int sz;
{
int i, j, k;
char *d,*tmp,*malloc();
d = malloc((sz+1));
if( d == NULL)  return(NULL);
strcpy(d,s);
i=strlen(d);
tmp = d;
if(i<sz)	{
	for(k=0; k<sz-i;k++)
		*(d+k) = '0';
	d = d+k;
	for(j=0; s[j]!='\0';j++)
		*(d++) = s[j];
	*d = '\0';
	return(tmp);	}
if(i==sz) return(tmp);
return(NULL);
}
