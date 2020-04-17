#include <stdio.h>
#include <fcntl.h>
#include "cfomfrm.h"
#include "fomlink.h"
int len, offset;
int mchr;
extern struct frmfld  field;	/* field description record of frmfile */
extern struct frmhdr  hdrrc;	/* frmfile screen header record */
extern struct linkstr *first;
extern char *string, str[];
mkfrm(flnm)			/* make frmfile from linked structure */
char flnm[];		/* pointer to the first element of list */
{
int op, doff;
char temp[15];

char tmpcr[81],tar[81];
int iii;
int recsz;
struct linkstr *ptr;
int fldnum, size;
len = 0;  fldnum = 0; recsz = doff = 0;
string = str;
*string++ = '\0';
strcpy(temp, flnm);
if((op=creat(temp,0755)) == -1)
	return(2);
if(write (op, ( char*) &hdrrc, FMH_SZ)<FMH_SZ)
	return(4);
ptr = first;
/* convert linkstr into frm file format & write it */
do
{				
				/* mkstring places the contents of its */
				/* argument in the array string and    */
				/* returns the size of the argument    */
				/* (a char type). if argument points   */
				/* to NULL string it returns zero. */
	field.dfsize  = ptr->dfsiz;
	size = mkstring(ptr->fldna);
	field.fldnam = size ? (len+1) : 0;
	len += size;		

	
	size = mkstring(ptr->promp);
	field.prompt = size ? (len+1) : 0;
	len += size;

	
	size = mkstring(ptr->imas);
	field.imask= size ? (len+1) : 0;
	len += size;	
	
	
	size = mkstring(ptr->dmas);
	field.dmask= size ? (len+1) : 0;
	len += size;

	if (field.prompt && field.imask && field.dmask) 
		field.fldclas = CL_PRMFLD;   else
	if (field.prompt) 
		field.fldclas = CL_PROM;     else 
	field.fldclas = CL_FLD; 
	
	if(field.fldclas!=CL_PROM)
		field.fldtyp = ptr->fldty;

    else
    	field.fldtyp = TYP_NONE;
	
    if(field.fldclas != CL_PROM)
    {
    for(iii=0;iii<(strlen(ptr->dmas));iii++)
		tmpcr[iii]=' ';
	tmpcr[iii]='\0';

	size = mkstring(tmpcr);
	field.eddata=size ? (len+1) : 0;
	len += size;
     }
	else
	field.eddata = 0;


/******* *** deleted for nfm version ***	
	if(field.dmask && field.imask)	     {
		size = scrtpist(ptr->fldty, ptr->imas);
		field.picstrng =  len+1;      }
		len += size;
*******/
    	
    field.picstrng = 0;
    scrtpist(ptr->fldty,ptr->imas);  /**** this simply sets or resets mflg now ****/
	
	size = mkstring(ptr->helpm);
	field.helpmes= size ? (len+1) : 0;
	len += size;
	
	
	size = mkstring(ptr->lboun);
	field.lbound= size ? (len+1) : 0;
	len += size;
	
	
	size = mkstring(ptr->uboun);
	field.ubound= size ? (len+1) : 0;
	len += size;
	
	if(ptr->dupva!=NULL) {
		size = mkstring(ptr->dupva);
		field.dupval= size ? (len+1) : 0;
		len += size;	 }
	else
	    if(field.fldclas!=CL_PROM)	 {
		switch(field.fldtyp) {
		case TYP_NUM:
			flme(tar,'0',field.dfsize);
			break;
		case TYP_STRING:
			flme(tar,' ',field.dfsize);
			break;
		case TYP_DATE:
			if(mchr)
				flme(tar,' ',field.dfsize);
			else
				flme(tar,'0',field.dfsize);
			break;
		case TYP_YN:
			flme(tar,'0',field.dfsize);
			break;
		default:
			break;
		}
		size = mkstring(tar);
		field.dupval=size ? (len+1) : 0;
		len += size;

    }
    else field.dupval=0;
	
	/* move the rest of the fields as they are */
	
	field.drloc    = doff;
	doff += ptr->dfsiz;
	field.promx    = ptr->prox;
	field.promy    = ptr->proy;
	field.fldx     = ptr->flx;
	field.fldy     = ptr->fly;
	field.promclr  = ptr->promc;
	field.fldclr   = ptr->fldc;
	field.dupctrl  = ptr->dupct;
	field.lattr1   = ptr->latt1;
	field.lattr2   = ptr->latt2;
	field.promva   = ptr->promv;
	field.fldva    = ptr->fldv;
	field.maskchar = ptr->machar;
	field.fldno    = ptr->fldn;
	
	

	
	if (field.fldclas != 1)
		recsz = recsz + field.dfsize;
	fldnum++;
	/* recsz contains the data record size   */
	/* fldnum contains total no of fields */
	/* len contains string length at end */
	
	if (write (op, (char *)&field, FMF_SZ) < FMF_SZ)
		return(4);
ptr = ptr->nexp;
} while (ptr != first);
len++;
hdrrc.vdsize = len;
hdrrc.noflds = fldnum;
hdrrc.drsize = recsz;

string = str;
if (write (op, string, len) < len)
	return(4);
lseek(op, 0L, 0);
if (write (op, (char *)&hdrrc, FMH_SZ) < FMH_SZ)
	return(4);


ptr = first;
do
{
free((char *)ptr);
ptr = ptr->nexp;
}
while(ptr != first);



close(op);
return(0);
}

mkstring(chstr)		/* chstr is the set of characters to be */
			/* concatenated to the string */
char *chstr;
{

int size = 0;
char c;
if (chstr == NULL) return(0);
for ( ;*chstr != '\0'; size++)
	*string++ = *chstr++;
if (size) 
{	*string++ = '\0';
	return(size+1);	}
else 	return(0);
}



scrtpist(typ, imsptr) /****** now a routine setting or resetting mchr ******/
int typ;
char *imsptr;
{
int i, m, size;
char *ima;
ima = imsptr;
switch(typ)     {
case TYP_STRING:
	mchr=0;
	break;
case TYP_NUM:
	break;

case TYP_YN:
	break;

case TYP_DATE:
	for (m=0; *ima != '\0';)
		if (*ima++ == 'M') m++;
	if (m == 3){
		mchr=1;
 		}
    else {
    	 mchr=0;
		 }
	}
return(0);

}

scat(c, ln)
char c;
int ln;
{
int i;
for (i=1; i<=ln; i++)  
	*string++ = c;
return(i-1);
}

flme(ctr,ch,sz)
char ctr[],ch;
int sz;{
   	register int iit;
   	for(iit=0;iit<sz;iit++)
    	ctr[iit] = ch;
	ctr[iit] = '\0';
	}
