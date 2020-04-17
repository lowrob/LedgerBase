/* profom editor: rec file creation: fomcrec.c */

#include <stdio.h>
#include "cfomfrm.h"
#include <fcntl.h>
recfrm(ext)			/* rec file from frm file */
char *ext;
{
int flg;
extern struct frmhdr hdrrc;
extern struct frmfld field;
extern int errnumb;
extern char scrn[];
int ip;
FILE *op;
char temp[15], picture[100], fldname[30], s[100];
extern char *string, str[];
int i;
errnumb = 0;
string = str;
strcpy(temp, hdrrc.scrnnam);
if(strcmp(hdrrc.language, "COBOL") == 0)	{
	flg=0;
	strcat(temp,".NFM");	}
else
{	strcat(temp, ".nfm");
	flg=1;	}
if ((ip=open(temp,(O_RDONLY))) == -1)
	return(3);
if(read(ip,(char *) &hdrrc, FMH_SZ)<FMH_SZ)
	return(6);
if(flg==1)
       return(gencdr(ext));  
else   
strcpy(temp, hdrrc.scrnnam);
strcat(temp,".REC");
if ((op = fopen(temp,"w")) == NULL) /* open rec file */
	return(2);
if (lseek(ip, (long) (hdrrc.noflds*FMF_SZ), 1) == -1)
	return(5);
if (read(ip, string, hdrrc.vdsize) < hdrrc.vdsize)
	return(6);
if (lseek(ip, (long) FMH_SZ, 0) == -1)
	return(5);
for (i=1; i<=hdrrc.noflds; i++)	{
if (read(ip, (char *) &field, FMF_SZ) < FMF_SZ)
	return(6);
if (field.fldclas == CL_PROM) continue;
if (field.fldnam != 0)	strcpy(fldname, string+field.fldnam);
else strcpy(fldname, "FILLER");
if (field.picstrng != 0)  strcpy(picture, string+field.picstrng);
comp(s, picture);
fprintf(op, "           05   %s     PIC   %s.\n", fldname, s);
}
close(ip);
fclose(op);
return(0);
}
