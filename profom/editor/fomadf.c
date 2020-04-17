/* add a field to the linked structure */

#include <stdio.h>
#include "fomlink.h"
#include "cfomstrc.h"
#include "cfomfrm.h"
#include "PROFOMF.sth"
int temint;
extern struct fe_struct dr;
extern int errnumb;
extern struct stat_rec statrec;
extern struct linkstr *curp;
struct linkstr *addfld()	/* add a field & return a pointer to it */
{
struct linkstr *adlnk(), *fech(),*tmp;
char *strsave();
int fld, succ;
errnumb = 0;
statrec.nextfld = 150;
statrec.endfld = 0;
filldr(0);
succ = 0;
while (!succ)	{
statrec.nextfld = 150;
dr.fe150[0]=LV_CHAR;
fomrf((char *) &dr);
errext(&statrec);
if(dr.fe150[0]=='P')	{	/* prompt only */
	temint = CL_PROM;
	filldr(2);
	dr.fe200 = dr.fe500 = LV_LONG;
	dr.fe600[0] = LV_CHAR;	}
else
if(dr.fe150[0]=='F')	{
	temint=CL_FLD;
	filldr(0);
	dr.fe600[0] = HV_CHAR;
	dr.fe1400[0] = dr.fe1600[0] = dr.fe1700[0] = dr.fe2100[0] = HV_CHAR;
	dr.fe500 =  HV_LONG;
    dr.fe2000 = HV_SHORT;
	dr.fe1900[0]='N';	}
else
if(dr.fe150[0]=='B')	{
	temint=CL_PRMFLD;
	filldr(0);
	dr.fe1400[0] = dr.fe1600[0] = dr.fe1700[0] = dr.fe2100[0] = HV_CHAR;
	dr.fe2000 = HV_SHORT;
	dr.fe1900[0]='N';	}
else	{
strcpy(dr.fe4000, "WRONG OPTION! TRY AGAIN");
statrec.nextfld = 4000;
fomwf((char *) &dr);
errext(&statrec);
succ=0;
continue;	}
succ=1;
dr.fe4000[0]=HV_CHAR;	}
statrec.nextfld=200;
statrec.endfld=2300;
fomrd((char *) &dr);
errext(&statrec);

	
if((tmp=adlnk(dr.fe200)) ==NULL)
	{ strcpy(dr.fe4000, "CAN'T ADD FIELD");
	  statrec.nextfld=4000;
	  fomwf((char *) &dr);
	  errext(&statrec);
	  return(NULL);  }
curp=tmp;
if(mkstruct()==-1)	errnumb=1;
filldr(1);
statrec.nextfld = 1;
statrec.endfld = 0;
fomwr((char *) &dr);
errext(&statrec);
return(curp);
}


/* make structrure from data record */

mkstruct()		/* make structrure from data record */
{
char s[100];
int val, succ,xcor,ycor;
char  tmp[20];
curp->fldcla=temint;
curp->fldn = dr.fe200;

/* FIELD NAME */
if(curp->fldcla != CL_PROM)
	curp->fldna = strsave(dr.fe300);
	
/* PROMPT */
if(curp->fldcla != CL_FLD)
	curp->promp = strsave(dr.fe600);
	
/* MASK CHAR */
if(curp->fldcla != CL_PROM)	{
	curp->machar = dr.fe750[0];
	          
/* INPUT MASK */
	expand(dr.fe900, s);
	curp->imas = strsave(s);
	          
/* DISPLAY MASK */
	expand(dr.fe1200, s);
	curp->dmas = strsave(s);
	          

/* HELP MESSAGE */
		curp->helpm = NULL;
	          
/* LOWER BOUNDS */
		curp->lboun = NULL;
	          

/* UPPER BOUNDS */
		curp->uboun = NULL;

/* DUP VALUE */
		curp->dupva = NULL;
		curp->dupct = 0;
      }

/* PROMPT POSITION */
if(curp->fldcla != CL_FLD)	{
	succ=0;
	while(!succ)	{
		xcor=dr.fe500/1000;
		ycor=dr.fe500 - (dr.fe500/1000) * 1000;
		if(xcor==0 || ycor==0)   {
		   strcpy(dr.fe4000, "PROMPT COORDINATES CAN'T BE ZERO");
		   statrec.nextfld = 4000;
		   fomwf((char *) &dr);
		   errext(&statrec);
		   dr.fe500 = LV_LONG;
		   statrec.nextfld = 500;
		   fomrf((char *) &dr);
		   errext(&statrec);	}
	   	else  { 
	   	   dr.fe4000[0] = HV_CHAR;
	   	   succ=1;  }   }
	curp->prox=xcor;
	curp->proy=ycor;	}
	          

/* DATA TYPE */
if(curp->fldcla != CL_PROM)	{
	succ=0;
	while(!succ)	{
		succ=1;
		if(dr.fe800[0]=='D') curp->fldty=TYP_DATE; else
		if(dr.fe800[0]=='S') curp->fldty=TYP_STRING; else
		if(dr.fe800[0]=='N') curp->fldty=TYP_NUM; else
		if(dr.fe800[0]=='B') curp->fldty=TYP_YN; else
		{  strcpy(dr.fe4000, "DATA TYPE ERROR");
		   statrec.nextfld=4000;
		   fomwf((char *) &dr);
		   errext(&statrec);
		   succ=0;
		   dr.fe800[0]=LV_CHAR;
		   statrec.nextfld=800;
		   fomrf((char *) &dr);
		   errext(&statrec);	}
		dr.fe4000[0] = HV_CHAR;   }
	if((curp->dfsiz=valmask(curp->fldty, curp->machar, curp->imas, curp->dmas)) == -2)
		{ strcpy(dr.fe4000, "INPUT/DISPLAY/TYPE MISMATCH");
		  errnumb=14;
		  statrec.nextfld=4000;
		  fomwf((char *) &dr);
		  errext(&statrec);  }
	else {    dr.fe4000[0] = HV_CHAR;
		  errnumb=0;	}
		  
	if(curp->dupva != NULL)
		if(strlen(curp->dupva) != curp->dfsiz)
			fomer("DUP VALUE SIZE ERROR! CORRECT IT!");
		else    ;
	if(curp->lboun != NULL)
		if(strlen(curp->lboun) != curp->dfsiz)
			fomer("LOWER BOUNDS SIZE ERROR! CORRECT IT!");
		else    ;
	if(curp->uboun != NULL)
		if(strlen(curp->uboun) != curp->dfsiz)
			fomer("UPPER BOUNDS SIZE ERROR! CORRECT IT!");
		   
	    
/* MASK POSITION */
	succ=0;
	while(!succ)	{
		xcor=dr.fe1100/1000;
		ycor=dr.fe1100 - (dr.fe1100/1000) * 1000;
		if(xcor==0 || ycor==0)    {
		   strcpy(dr.fe4000, "MASK COORDINATES CAN'T BE ZERO");
		   statrec.nextfld = 4000;
		   fomwf((char *) &dr);
		   errext(&statrec);
		   dr.fe1100 = LV_LONG;
		   statrec.nextfld = 1100;
		   fomrf((char *) &dr);
		   errext(&statrec);	}
	   	else  { 
	   	   dr.fe4000[0] = HV_CHAR;
	   	   succ=1;  }   }
	
	
	curp->flx=xcor;
	curp->fly=ycor;
	          

}

}


struct linkstr *adlnk(fld)	/* add a field */
int fld;
{
extern struct linkstr *first;
struct linkstr *temp;
int succ, pos;
char *calloc();
curp=first;
succ=0;
do
{	if(fld < curp->fldn)
	{	succ=1;  break;	}
	else
	if(fld==curp->fldn)
	{	errnumb=12;
		return(NULL);	}
	else
	{	succ=0;  curp=curp->nexp;	}
}
while(curp!=first);

if(curp==first)
	if(succ==1)	pos=1;
	else		pos=0;
else pos=9;
curp=curp->prep;
if ((temp=(struct linkstr *)calloc(1,sizeof(struct linkstr)))==NULL)
	return(NULL);
temp->prep = curp;
temp->nexp = curp->nexp;
curp->nexp->prep = temp;
curp->nexp = temp;
if(pos==1) first = temp;
return(temp);
}
