#include <stdio.h>
#include "fomlink.h"
#include <fcntl.h>
#include "cfomstrc.h"

#include "cfomfrm.h"
extern int errnumb;
extern struct frmhdr hdrrc;
extern struct frmfld field;
extern struct stat_rec statrec;
extern char *string, str[];
struct linkstr *first, *curp;
struct linkstr *mklinks(screnme) 
char *screnme;		  /*  make a linked structure from frmfile	*/
			  /*   return a pointer to the first entry	*/
{
char *strsave(), screen[16];
int i, j;			
int ip;
struct linkstr *putlnk();
errnumb = 0;
first = NULL;
string = str;
if(strcmp(hdrrc.language, "COBOL") == 0)	{
	strcpy(screen, screnme);
	strcat(screen, ".NFM");	}
else	{
	strcpy(screen, screnme);
	strcat(screen, ".nfm");	}
if ((ip = open(screen,(O_RDONLY))) == -1)
	{ errnumb = 3; return(NULL); }
if ((read(ip,(char *)&hdrrc, FMH_SZ)) < FMH_SZ)
	{ errnumb = 6; return(NULL); }
if (lseek(ip, (long)(hdrrc.noflds * FMF_SZ), 1) == -1)
	{ errnumb = 5; return(NULL);  }
if ((read(ip, string, hdrrc.vdsize)) < hdrrc.vdsize)
	{ errnumb = 6; return(NULL); }
if (lseek(ip, (long)FMH_SZ, 0) == -1)
	{ errnumb = 5; return(NULL); }
for (j=1; j<=hdrrc.noflds; j++)
 
{	i = read (ip, (char *)&field, FMF_SZ);
	if (i == -1)
		{ errnumb = 6; return(NULL); }
	else if (i == 0)
		{ errnumb = 7; return(NULL); }
	
	if ((curp = putlnk (field.fldno)) == NULL)
		{ errnumb = 8; return(NULL); }
	if(putinfo()== -1) return(NULL);
	
}
return(first);

/* putlnk () : this routine calls fech () to find out		*/	
/* whether the field is already present; if so fech ()	*/
/* returns a pointer to it; else NULL:				*/
/* if NULL putlnk () allocates area for the new field, 		*/	
/* creates links and returns a pointer to the field		*/	
/* putinfo () : this routine puts info into the structure */
/* just allocated using malloc(); */ 
}


putinfo()		/* put info into the current node of the linked */
			/* structure and return */
{
if (field.fldnam)
	curp->fldna = strsave(string+field.fldnam);
else
	curp->fldna = NULL;
if (field.prompt)
	curp->promp = strsave(string+field.prompt);
else
	curp->promp = NULL;
if (field.imask)
	curp->imas = strsave(string+field.imask);
else
	curp->imas = NULL;
if (field.dmask)
	curp->dmas = strsave(string+field.dmask);
else
	curp->dmas = NULL;
if (field.picstrng)
	curp->picst = strsave(string+field.picstrng);
if (field.helpmes)
	curp->helpm = strsave(string+field.helpmes);
else
	curp->helpm = NULL;
if (field.lbound)
	curp->lboun = strsave(string+field.lbound);
else
	curp->lboun = NULL;
if (field.ubound)
	curp->uboun = strsave(string+field.ubound);
else
	curp->uboun = NULL;
if (field.dupval)
	curp->dupva = strsave(string+field.dupval);
else
	curp->dupva = NULL;
curp->fldn =field.fldno;

curp->fldty = field.fldtyp;
curp->fldcla = field.fldclas;
curp->prox = field.promx ;
curp->proy = field.promy;
curp->flx =  field.fldx;
curp->fly = field.fldy;
curp->drlo = field.drloc;
curp->promc = field.promclr;
curp->fldc = field.fldclr;
curp->dupct = field.dupctrl;
curp->latt1 = field.lattr1;
curp->latt2 = field.lattr2;
curp->promv = field.promva;
curp->fldv = field.fldva;
curp->machar = field.maskchar;
curp->dfsiz = field.dfsize;
if (errnumb==1)
	return(-1);
else    return(0);
	
}
 

struct linkstr *fech(fldnum)  /* access field fldnum from the linked  */
				/* structure and return a pointer to it */
				/* if found. else return NULL.		*/
int fldnum;
{
if (first == NULL)
	return (NULL);
curp = first;
do	{
	if (fldnum == curp->fldn)
		return (curp);
	else
		curp = curp->nexp;
	} while (curp != first);
return (NULL);
}


struct linkstr *putlnk(fldnum)	/* allocate area to the new field	*/
				/* create links to the area and		*/
				/* the rest of the structure		*/
				/* return a pointer to the area		*/
int   	fldnum;			
{
struct linkstr *fech();
char 	*malloc();
static 	struct linkstr *prlink;
/* case of creation of a fresh linked structure */
/* case of adding a new link to the structure   */

if (first == NULL)
	prlink = NULL;		/* prlink=pointer to the previous field */
if ((curp = fech (fldnum)) == NULL)
			/* examine whether the field is already */
			/* available in the linked structure	*/   
  {			/* if not,				*/
	if ((curp=(struct linkstr *)malloc(sizeof(struct linkstr)))==NULL)
		return (NULL);
	else
	if (prlink != NULL)	
	{
		curp->prep = prlink;
		if (prlink->nexp == first)
			curp->nexp = first;
		else
			curp->nexp = prlink->nexp;
		prlink->nexp = curp;
		prlink = curp;
		first->prep = curp;
		return(curp);
	}
	else
	{
		curp->prep = curp->nexp = first = prlink = curp;
		return(curp);
	}					
}
else
{		/* there is already a field with this no. */
		errnumb = 9;
		return (NULL);
	}

}


char *strsave(s)	/* save string s somewhere */
char *s;
{
char *p, *malloc();
if ((p = malloc(strlen(s)+1)) == NULL)
	{ errnumb = 1; return(NULL);  }
strcpy(p, s);
return(p);
}
