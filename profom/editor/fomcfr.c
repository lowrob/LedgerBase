/* profom editor: frm file creation from scr file: fomcfrm.c */

#include "cfomstrc.h"
#include  <stdio.h>
#include <fcntl.h>
#include  "cfomfrm.h"
#define PMODE 0664
char line[83];
static int len, offset;
int row, clm;
int recsz;			/*       data record size            */
static struct frmfld *field;
static int   ip,                       /*       input  file descriptor      */
      op;		        /*       output file descriptor      */
extern int errnumb;
extern struct frmhdr hdrrc;
extern struct stat_rec statrec;
int doff,eds;
extern char *string, str[];
frmscr()			/* create frm file from scr file */
{
char *calloc();
int  fldno;		/*       field sequence no.          */
int z,ij;
int   pos,
      nread;
char temp[80],tcrf;
static char suffix[] = ".nfm";
pos = 0;
if(errnumb==1)  return(-1);
errnumb=0;
				/* allocate area to all items */
if ((field = (struct frmfld *) calloc(1, FMF_SZ)) == NULL)
	return(1);
	
string = str;
scredit(0);
hdrrc.noflds = 0;
hdrrc.vdsize = 0;
hdrrc.drsize = 0;              /*        header filled up           */
strcpy (temp, hdrrc.scrnnam);
if(strcmp(hdrrc.language, "COBOL") == 0)
	strcat(temp, ".SCR");
else
	strcat (temp, ".scr");
if ((ip = open  (temp, O_RDONLY)) == -1)
	return(3);
strcpy (temp, hdrrc.scrnnam);
if(strcmp(hdrrc.language, "COBOL") == 0)
	strcat(temp, ".NFM");
else
	strcat (temp, ".nfm");
if ((op = creat(temp, 0755))==-1)
	return(2);
if (write(op, (char *)&hdrrc, FMH_SZ) < FMH_SZ)
	return(4);
doff  = 0;
fldno   = 0;
offset  = 1;
row     = 0;
*string++ = '\0'; 	
len = 1;
					/* getline() gets a line from input */
while ((z=getline()) >= 0)
{	pos=0;
	row++;
	while ((clm = skpblnk(pos)) >= 0)
	{	switch (clas())
		{
		case 1:
			field->fldclas = CL_PROM;
			break;
		case 2:
			field->fldclas = CL_PRMFLD;
			break;
		case 3:
			field->fldclas = CL_FLD;
		}
		pos = clm;
		
		if(field->fldclas == CL_PROM)
			field->fldtyp = TYP_NONE;
		field->fldnam = 0;	/* corresponding to FILLER   */
	
		field->fldno = ++fldno * 100;
					/* default nos 100,200,....  */
		if(field->fldclas != CL_PROM)
		{
			field->dupval=offset;
			if(field->fldtyp==TYP_NUM)
				tcrf='0';
			else
				tcrf=' ';
			for(ij=0;ij<field->dfsize;ij++)
			{	*string++ = tcrf;
				len++;
			}
			*string++ = '\0';
			len++;
			offset += ij+1;
		

			field->eddata=offset;
			for(ij=0;ij<eds;ij++)
			{	*string++ = ' ';
				len++;
			}
			*string++ = '\0';
			len++;
			offset += ij+1;

		}
		else
		{
			field->eddata = 0;
			field->dupval = 0;
		}
		field->picstrng = 0;
		if (write (op, (char *)field, FMF_SZ) < FMF_SZ)
			return(4);
		initfild();
		if (pos == 0)
			break;
	}
}					/* write the string at end */
if(z==-3) return(13);
if(z==-2) return(6);
string = str;
if (write (op, string, len) < len)
	return(4);
if (lseek (op, 0L, 0) == -1)		/* get to the beginning of the file */
	return(5);

hdrrc.noflds = fldno;
hdrrc.vdsize = len;
hdrrc.drsize = recsz;
if (write (op, (char *)&hdrrc, FMH_SZ) < FMH_SZ)
	return(5);

close(op);
close(ip);
return(0);
}       			/* end of program      */


getline()			/* read input;expand tabs;return a line */
{

int i;				/* byte count on read			*/
int k;
int l;
char c;
int col = 0;			/* column no of array line		*/
line[col++] = ' ';
for (l = 1; (i = read (ip, &c, 1)); l++)
	switch(c)
      {
       	case '\n':
       		line[col++] = c;
       		line[col]   = '\0';
       		return(col);
       		
       	case '\t':
       		exptabs(&col);
       		break;
       	default:
		if(col>80) return(-3);
       		line[col++]=c;	/* any others including blanks		*/
      }
if (i == -1)
	return(-2);
else
if (i == 0)
	return(-1);
}
 

#define TAB_LEN 8
#define SPACE ' '
exptabs(col)
int *col;
{
 	int i;
	if (*col % TAB_LEN)
		for (i = *col + TAB_LEN - *col % TAB_LEN;
			*col <= i; (*col)++) line[*col] = SPACE;
	else {
		line[*col] = SPACE;
		(*col)++;
	} 
		
}


clas()
/*  this routine takes the starting non blank position of   */
/*  a line and classifies the nature of field: it returns   */
/*  1 for prompt only field 2 for prompt and mask field     */
/*  3 for mask only field and modifies the current column   */
/*  to enable scanning of the next field in the line if any */
/*  if not, it returns 0 to show end of line                */
 
{
if (line[clm] != '_' && line[clm] != '#')
/* prompt routine should return 1 if a new line has come at end */
/* indicating a prompt only field; if it is terminated by a pair */
/* of blanks and followed by a mask it should return 2 else if   */
/* it is followed by another prompt it should return 3           */

	if (prom() == 1)
		{
		clm = 0;
		return(1);
		}
	else
		return(maask() ? 2 : 1 );
else

return(maask() ? 3 : 1);
}



prom()			/* process a prompt field */

{
int col;
register int size = 1;
for(col=clm;!(line[col+1]==' ' && line[col]==' ' || line[col]=='\n');col++)
{	*string++ = line[col];
	len++;
	size++;
}
*string++ = '\0';

len++;

field->prompt = offset;	/*       starting of this field	    */
offset += size;		/*       offset for next entry      */

field->promx = row;		
field->promy = clm;	/*  prompt started at this point    */
if (line[col] == '\n')
	return(1);
if ((clm = skpblnk(col)) < 0)
	return(1);
return(2);

}


char *dma;
maask()			/* process a mask */
{
char arr[81];
int col; 
register int size = 1;
register int dsize = 0;
dma = arr;
if (line[clm] != '_' && line[clm] != '#')
	return(0);
field->maskchar=line[clm];
if (line[clm] == '_')   field->fldtyp = TYP_STRING; else
			field->fldtyp = TYP_NUM;        
for (col=clm; line[col] != ' ' && line[col] != '\n'; col++)
{	*string++ = line[col];
	*dma++ = line[col];
	len++;
	size++;
	if (line[col] == '_' || line[col] == '#') ++dsize;
	if (line[col] == '#') field->fldtyp = TYP_NUM;
}
*string++ = '\0';
*dma++ = '\0';
field->dfsize = dsize;
field->drloc = doff;
doff  =  doff + dsize;
recsz = recsz + dsize;
len++;

field->dmask = offset; 
offset += size;
field->fldx = row;
field->fldy = clm;
clm = (line[col] == '\n') ? 0 : col;
dma = arr;
eds = strlen(dma);
crtima(field->fldtyp);
return(1);
}

char *ima, car[81];
crtima(flag)	/* create input mask */
int flag;
{
int i, j;
j = 1;
ima = car;
field->imask = offset;
switch (flag)	{
case TYP_YN:
	*string++ = 'B';
	*ima++ = 'X';
	len++;
	*string++ = '\0';
	*ima = '\0';
	len++;
	offset += 2;
	break;	
case TYP_STRING:
	for (i=1; *dma != '\0'; i++)
	{	if (*dma++ != '_')  *string++ = 'F';
		else    { *string++ = 'X'; *ima++ = 'X' ;  }
		len++;	}
	*ima = '\0';
	*string++ = '\0';
	len++;
	offset += i;
	break;
case TYP_NUM:
	for (i=1; *dma != '\0'; i++)
	{	if (*dma == '.') { *string++ = '.'; *ima++ = 'V';  }
		else 
		if (*dma != '#')
                        {  *ima = 'F';  *string++ = *ima++;  }
		else 	{  *string++ = '9';  *ima++ = '9'; }
		dma++;
		len++;	}
	*string++ =  '\0';   *ima++ = '\0';
	len++;
	offset += i;
	break;
}/******  deleted for nfm version dt 02-nov-85 
crtpic(field->fldtyp); ******/
}

crtpic(typ)
int typ;
{
int i, m;
field->picstrng = offset;
ima = car;
switch(typ)     {
case TYP_STRING:
	offset += cat ('X',field->dfsize);
	break;
case TYP_NUM:
	for (i=1; *ima != '\0'; i++)
	{	if(*ima=='.') *ima = 'V';
		*string++ = *ima++;
		len++;	}
	offset += i;
	break;
case TYP_YN:
	*string++ = 'X';
	offset += 2;
	break;
case TYP_DATE:
	for (m=0; *ima != '\0';)
		if (*ima++ == 'M') m++;
	if (m == 3) offset += cat ('X', field->dfsize); else 
		    offset += cat ('9', field->dfsize);
	}
*string++ = '\0';
len++;

}

cat(c, ln)
char c;
int ln;
{
int i;
for (i=1; i<=ln; i++)  {
	*string++ = c;
	len++;         }
return(i);
}




skpblnk(pos)				/* skip blanks */
int pos;
{
int i;
for (i=pos;line[i] == ' ';i++);
if (line[i] == '\n')
	return(-1);
return(i);
}

initfild()
{
/* initialize the fields of the structure & return */
field->fldnam = 0;
field->prompt = 0;
field->promx = 0;
field->promy = 0;
field->fldx = 0;
field->fldy = 0;
field->imask = 0;
field->dmask = 0;
field->drloc = 0;
field->picstrng = 0;
field->helpmes = 0;
field->lbound = 0;
field->ubound = 0;
field->promclr = 0;
field->fldclr = 0;
field->dupctrl = 0;
field->dupval = 0;
field->lattr1 = 0;
field->lattr2 = 0;
field->promva = 0;
field->fldva = 0;
field->maskchar = 0;
field->dfsize = 0;
}
