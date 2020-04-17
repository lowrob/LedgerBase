/* profom editor: main program : fomeditor.c */
#include <stdio.h>
#include "cfomfrm.h"
#include "fomlink.h"
#include "cfomstrc.h"
#include <ctype.h>
#include <fcntl.h>
#include "PROFOMC.sth"
#define STSZ 8192
struct frmhdr hdrrc;
struct frmfld field;
char *string, str[STSZ], temp[20];

struct stat_rec statrec;
char scrn[17]; 
int errind,smod;
FILE  *ttp;
main(argc,argv)
int	argc;
char	*argv[];
{
int tp, flag, cflag, begn;
struct cr_struct cs;
char *starcpy();
char *extn,extar[4];
	if (argc == 2){		/* file Name is given for FOM to NFM */
		errind=frmfom(argv[1]);
		if(errind==3) strcpy(cs.cr1200, ".fom/.FOM FILE NOT FOUND");
		else
		if(errind==2)
			if(strcmp(hdrrc.language,"COBOL")==0)
				strcpy(cs.cr1200,".NFM FILE OPEN ERROR");
			else	strcpy(cs.cr1200,".nfm FILE OPEN ERROR");
		else
		if(errind!=0)strcpy(cs.cr1200,"ERROR IN NFM FILE CREATION");
		else     
		{   	flag = 1;
			strcpy(cs.cr1200, "NFM FILE CREATED");    }    
		exit(0);
	}

errind = 0;
begn=1;
strcpy(statrec.scrnam, "/usr/bin/PROFOMC");
statrec.termnm[0] = LV_CHAR;

fomin(&statrec);			/* initialise profom runtime */
errext(&statrec);
BACK:
cs.cr1000[0] = cs.cr1100[0] = cs.cr1200[0] = HV_CHAR;
statrec.nextfld=200;
cs.cr200[0] = LV_CHAR;
fomrf((char *) &cs);
errext(&statrec);
strcpy(temp, cs.cr200);
strcat(temp, ".nfm");
if((tp=open(temp, (O_RDONLY))) == -1)  {
	strcpy(temp, cs.cr200);
	strcat(temp, ".NFM");
	if((tp=open(temp, (O_RDONLY))) == -1)
		flag = 0; else flag = 1;  }
else flag = 2;
if(flag!=0)	{
	read(tp, (char *) &hdrrc, FMH_SZ); 
	close(tp);	}

for ( ; cs.cr1000[0] != '0';)
{	
	strcpy(statrec.scrnam, "/usr/bin/PROFOMC");
	cs.cr1100[0] = HV_CHAR;
	if(begn) 
		begn=0;
	else
	if(!smod) ;
	else	 strcpy(cs.cr200,hdrrc.scrnnam);
	statrec.nextfld=200;
	fomwf((char *) &cs);
	errext(&statrec);
	statrec.nextfld = 1000;	
	cs.cr1000[0] = LV_CHAR;
	fomrf((char *) &cs);
	errext(&statrec);
	switch(cs.cr1000[0])	{
	case '1':		/* create frm file from scr file */
		if(flag)	{
		strcpy(cs.cr1200,"nfm/NFM EXISTS!");
		break;	}
		else   {
		errind=frmscr();
		strcpy(statrec.scrnam, "/usr/bin/PROFOMC");
		if(errind==3)
			if(strcmp(hdrrc.language,"COBOL")==0)
				strcpy(cs.cr1200,".SCR FILE NOT FOUND");
			else	strcpy(cs.cr1200,".scr FILE NOT FOUND");
		else
		if(errind==2)
			if(strcmp(hdrrc.language,"COBOL")==0)
				strcpy(cs.cr1200,".NFM FILE OPEN ERROR");
			else	strcpy(cs.cr1200,".nfm FILE OPEN ERROR");
		else
		if(errind!=0) strcpy(cs.cr1200,"ERROR IN NFM FILE CREATION");
		else    {   flag=1;
		strcpy(cs.cr1200, "NFM FILE CREATED");    }    }
		break;
	case '2':		/* create frm file from fom file */
		if(flag)	{
		strcpy(cs.cr1200,".nfm/.NFM FILE EXISTS !");
		break;    }
		else   {
		errind=frmfom(cs.cr200);
		if(errind==3) strcpy(cs.cr1200, ".fom/.FOM FILE NOT FOUND");
		else
		if(errind==2)
			if(strcmp(hdrrc.language,"COBOL")==0)
				strcpy(cs.cr1200,".NFM FILE OPEN ERROR");
			else	strcpy(cs.cr1200,".nfm FILE OPEN ERROR");
		else
		if(errind!=0)strcpy(cs.cr1200,"ERROR IN NFM FILE CREATION");
		else     
		{   	flag = 1;
			strcpy(cs.cr1200, "NFM FILE CREATED");    }    }
		break;
	case '3':		/* create rec file from frm file */
		if(flag==0) 
			{ strcpy(cs.cr1200, "SORRY! NO NFM FILE");
			  break;  }
		if(strcmp(hdrrc.language, "COBOL") != 0)  {
			cflag = 1;
			statrec.nextfld=1100;
			cs.cr1100[0] = LV_CHAR;
			fomrf((char *) &cs);
			errext(&statrec);
			printf("extn %s",cs.cr1100);
			extn=extar;
			starcpy(extn,cs.cr1100);  }
		else  {    strcpy(extn, "");
			   cflag=0;	}
		extn = extar;
		errind=recfrm(extn);
		if(errind==0) 
			if(cflag) 
				strcpy(cs.cr1200, "sth FILE CREATED");
			else 	strcpy(cs.cr1200, "REC FILE CREATED");
		else
		if(errind==2)
			if(cflag)
				strcpy(cs.cr1200,".sth FILE OPEN ERROR");
			else	strcpy(cs.cr1200,".REC FILE OPEN ERROR");
		else
		if(errind==3)
			if(cflag)
				strcpy(cs.cr1200,".nfm FILE OPEN ERROR");
			else	strcpy(cs.cr1200,".NFM FILE OPEN ERROR");
		else 	strcpy(cs.cr1200,"ERROR IN REC/sth FILE CREATION");
		break;
	case '4':		/* create fom file from frm file */
		if(flag==0) 
		{	strcpy(cs.cr1200, "SORRY NO NFM FILE");
			break;	}
		else
		errind=fomfrm();
		if(errind==0)  
		{	strcpy(cs.cr1200, "FOM FILE CREATED");
			break;	}
		else
		if(errind==2)
			if(strcmp(hdrrc.language, "COBOL")==0)
				strcpy(cs.cr1200,".FOM FILE OPEN ERROR");
			else	strcpy(cs.cr1200,".fom FILE OPEN ERROR");
		else
		if(errind==3)	
			if(strcmp(hdrrc.language, "COBOL")==0)
				strcpy(cs.cr1200,".NFM FILE OPEN ERROR");
			else	strcpy(cs.cr1200,".nfm FILE OPEN ERROR");
		else
			strcpy(cs.cr1200,"ERROR IN FOM FILE CREATION");
		break;
	case '5':		/* create scr file from frm file */
		if(flag==0) 
		{	strcpy(cs.cr1200, "SORRY NO NFM FILE");
			break;	}
		else
		errind=scrfrm();
		if(errind==0)
			  strcpy(cs.cr1200, "SCR FILE CREATED");
		else
		if(errind==2)
			if(strcmp(hdrrc.language,"COBOL")==0)
				strcpy(cs.cr1200,".SCR FILE OPEN ERROR");
			else	strcpy(cs.cr1200,".scr FILE OPEN ERROR");
		else
		if(errind==3)	
			if(strcmp(hdrrc.language, "COBOL")==0)
				strcpy(cs.cr1200,".NFM FILE OPEN ERROR");
			else	strcpy(cs.cr1200,".nfm FILE OPEN ERROR");
		else
			strcpy(cs.cr1200,"ERROR IN SCR FILE CREATION");
		break;
	case '6':		/* update screen (frm file) */
		if(flag==0) strcpy(cs.cr1200, "SORRY NO NFM FILE");
		else   {
		errind=updscrn();
		strcpy(statrec.scrnam, "/usr/bin/PROFOMC");
		if(errind==0)  strcpy(cs.cr1200, "SCREEN UPDATE DONE");
		else
		if(errind==3)	
			if(strcmp(hdrrc.language, "COBOL")==0)
				strcpy(cs.cr1200,".NFM FILE OPEN ERROR");
			else	strcpy(cs.cr1200,".nfm FILE OPEN ERROR");
		else
		strcpy(cs.cr1200, "ERROR IN SCREEN UPDATE"); }
		break;
	case '7':
		goto BACK;
	case '0':
		break;
	default:		/* unknown option */
		strcpy(cs.cr1200, "UNKNOWN OPTION! TRY AGAIN");
		break;
}       statrec.nextfld=1200;
	fomwf((char *) &cs);
	errext(&statrec);
}
fomcs();
if(errind != 0)
	printf("Profom Editor Error No: %d", errind);
fomrt();
exit(0);
}


errext(st)
struct stat_rec *st;
{
register int j;
if(st->retcode==RET_ERROR)	{
	j=st->errno;
	fomcs();
	fomrt();
	printf("Profom Error No = %d\n", j);
	exit(1);	}
else
	return;
}


