/* fomuscr.c : update screen */

#include <fcntl.h>

#include "cfomfrm.h"
#include "fomlink.h"
#include <stdio.h>
#include "cfomstrc.h"
#include "PROFOMU.sth"
static struct linkstr *first;
int errnumb, modified;
extern char *string, str[];
extern struct stat_rec statrec;
extern struct frmhdr hdrrc;
extern struct frmfld field;
extern int smod;
updscrn()
{
struct us_struct ur;
struct linkstr *mklinks();
char buf[15], c;

string = str;
strcpy(statrec.scrnam, "/usr/bin/PROFOMU"); /* PROFOMU screen */
strcpy(ur.us200, hdrrc.scrnnam);
ur.us1100[0] = LV_CHAR;
ur.us1200[0] = HV_CHAR;
if((first=mklinks(ur.us200))==NULL)
	return(errnumb);
modified=0;

for ( ; ur.us1100[0] != '$';)
{       strcpy(statrec.scrnam, "/usr/bin/PROFOMU");
	strcpy(ur.us200,hdrrc.scrnnam);
	statrec.nextfld=200;
	fomwf((char *) &ur);
	errext(&statrec);
	statrec.nextfld = 1100;	 /* 1100 for editor options */
	ur.us1100[0] = LV_CHAR;
	fomrf((char *)&ur);
	errext(&statrec);
	switch (ur.us1100[0]) 	{
	case '0':
		if(modified)
			{ strcpy(ur.us1200, "SCREEN MODIFIED,NOT WRITTEN");
			  break;  }
			 
		ur.us1100[0] = '$';
		continue;
	case '1':			 /* screen edit option */
		scredit(1);
		strcpy(statrec.scrnam, "/usr/bin/PROFOMU");
		if (errnumb==0)
			strcpy(ur.us1200, "SCREEN LEVEL EDIT DONE");
		else    strcpy(ur.us1200, "ERROR IN SCREEN LEVEL EDIT");
		break;
	case '2':			 /* field edit option */
		fldedit();
		strcpy(statrec.scrnam, "/usr/bin/PROFOMU");
		if (errnumb==0)
			strcpy(ur.us1200, "FIELD EDIT DONE");
		else    strcpy(ur.us1200, "ERROR IN FIELD EDIT");
		break;
	case '3':
		if((errnumb = dispform()) != 0)  errnumb = 1;
		if (errnumb==0)
			strcpy(ur.us1200, "SHOW SCREEN DONE");
		else    strcpy(ur.us1200, "SHOW SCREEN NOT DONE");
		break;
	case '4':			 /* Quit -  no save */
		if(modified)  {
		    strcpy(ur.us1200,"QUIT (NO SAVE)? IF SO REPEAT OPTION");
		    modified=0;
		    smod=0;
		    break;   }		
		if (!modified)  
			{ errnumb=0;
			  ur.us1100[0]='$';  }
		
		break;
	case '5':			 /* create frm file and leave edit */
		strcpy(buf,hdrrc.scrnnam);
		if(strcmp(hdrrc.language, "COBOL")==0)
			strcat(buf, ".NFM");
		else
			strcat(buf, ".nfm");
		errnumb=mkfrm(buf);
		if(errnumb==0)
			strcpy(ur.us1200, "NFM FILE UPDATED");
		else
		if(errnumb==2)	
			if(strcmp(hdrrc.language, "COBOL")==0)
				strcpy(ur.us1200,".NFM FILE OPEN ERROR");
			else	strcpy(ur.us1200,".nfm FILE OPEN ERROR");
		else    
		strcpy(ur.us1200, "ERROR IN NFM FILE UPDATE");
		modified=0;
		smod=1;		 
		ur.us1100[0] = '$';
		break;
	case '6':		   /* create frm file and exit to monitor */
		strcpy(buf,hdrrc.scrnnam);
		if(strcmp(hdrrc.language, "COBOL") == 0)
			strcat(buf, ".NFM");
		else
			strcat(buf, ".nfm");
		errnumb=mkfrm(buf);
		if(errnumb==0)
			strcpy(ur.us1200, "NFM FILE UPDATED");
		else
		if(errnumb==2)	
			if(strcmp(hdrrc.language, "COBOL")==0)
				strcpy(ur.us1200,".NFM FILE OPEN ERROR");
			else	strcpy(ur.us1200,".nfm FILE OPEN ERROR");
		else
		strcpy(ur.us1200, "ERROR IN NFM FILE UPDATE");
		fomrt();		 
		fomcs();
		exit(0);
	case '7':
		reslog();
		if(errnumb==0)
			strcpy(ur.us1200, "FIELDS RESEQUENCED");
		else    strcpy(ur.us1200, "ERROR IN RESEQUENCING");
		modified=1;
		break;
	default:
		strcpy(ur.us1200, "WRONG OPTION! TRY AGAIN");
		break;
	}
	statrec.nextfld=1200;
	fomwf((char *) &ur);
	errext(&statrec);
}
return(errnumb);
}


reslog()	/* resequence field no. */
{
extern struct linkstr *first;
FILE *fd;
int incr;
struct linkstr *ptr;
char resfl[16];

incr = 0;
ptr = first;
strcpy(resfl, hdrrc.scrnnam);
strcat(resfl, ".res");
fd = fopen(resfl, "w");

do
{
incr += 100;
fprintf(fd, "Old field number = %d ", ptr->fldn);
ptr->fldn = incr;
fprintf(fd, "New field number = %d ", ptr->fldn);
fprintf(fd, "\n");
ptr = ptr->nexp;
}
while (ptr != first);
fclose(fd);
return;
}
