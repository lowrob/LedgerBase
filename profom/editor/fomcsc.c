/* profom editor: scr file creation from frm file : fomcscr.c */
#include <stdio.h>
#include "cfomfrm.h"
#include <fcntl.h>
char screen[24][81];
int ip, op;
extern struct frmfld field;
extern struct frmhdr hdrrc;
extern char *string, str[];
extern int errnumb;
scrfrm()			/* scr file from frm file */
{
int i, k;
char temp[15];
errnumb = 0;
string = str;
strcpy (temp, hdrrc.scrnnam);
if(strcmp(hdrrc.language, "COBOL") == 0)
	strcat(temp, ".NFM");
else
	strcat(temp, ".nfm");
if ((ip = open (temp, (O_RDONLY))) == -1)	/* temp is forms file */
	return(3);
if ((read(ip,(char *)&hdrrc, FMH_SZ)) < FMH_SZ)
	return(6);
strcpy (temp, hdrrc.scrnnam);
if(strcmp(hdrrc.language, "COBOL") == 0)
	strcat(temp, ".SCR");
else
	strcat(temp, ".scr");
if ((op = creat (temp, 0755)) == -1)	/* temp is screen file */
	return(2);

if (lseek(ip,(long)(hdrrc.noflds * FMF_SZ), 1) == -1)
	return(5);
if ((read(ip, string, hdrrc.vdsize)) == -1)
	return(6);
if (lseek(ip, (long)FMH_SZ,0) == -1)
	return(5);
initscreen();		/* this routine initialises the screen image */
for (i=1; i<=hdrrc.noflds; i++)
{	if ((read(ip, (char *)&field, FMF_SZ)) < FMF_SZ)
		return(6);
	putscreen();
}
if (write(op, screen, 24 * 81) < 24 * 81)
	return(4);

close(ip);
close(op);
return(0);
}


initscreen()

{
int i, j;
for (i=0; i<=23; i++)
{	for (j=0; j<80; j++)
		screen[i][j] = ' ' ;
		screen[i][80] = '\n';   }
		
}		
		
		
putscreen()
{
if (field.fldclas == CL_PROM)
	arrcpy(&screen[field.promx-1][field.promy-1], string+field.prompt);
else
if (field.fldclas == CL_PRMFLD)
{	arrcpy(&screen[field.promx-1][field.promy-1], string+field.prompt);
	arrcpy(&screen[field.fldx-1][field.fldy-1], string+field.dmask);   }
else
if (field.fldclas == CL_FLD)
	arrcpy(&screen[field.fldx-1][field.fldy-1], string+field.dmask);

return;
}


arrcpy(s1, s2)
char s1[], s2[];
{
register int i;
for(i=0; s2[i] != '\0'; i++)
	s1[i] = s2[i];
s1[i] = ' ';
}
