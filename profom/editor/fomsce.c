/*fomsced.c:  program to edit screen level attributes */

#include <stdio.h>
#include "fomlink.h"
#include "cfomfrm.h"
#include "cfomstrc.h"
#include "PROFOMS.sth"
scredit(flag)
int flag;			/* 0 for first time creation */
{
extern int errnumb, modified,smod;
extern struct stat_rec statrec; 
extern struct frmhdr hdrrc;
struct sc_struct dr;
register int fir1, fir2, fir3;
strcpy(dr.sc600, "ENTER SCREEN DETAILS");
if(flag!=0)   {                    
	strcpy(dr.sc200, hdrrc.scrnnam);
	strcpy(dr.sc300, hdrrc.version);
	strcpy(dr.sc400, hdrrc.language);	}
else
	dr.sc200[0]=dr.sc300[0]=dr.sc400[0]=LV_CHAR; 
	
	fir1=fir2=fir3=0;
for (; dr.sc500[0] != '0';)
{	
	strcpy(statrec.scrnam, "/usr/bin/PROFOMS");
	statrec.nextfld = 500;	/* 500 editor options */
	
	dr.sc500[0]=LV_CHAR;
	fomrf((char *) &dr);
	errext(&statrec);
	dr.sc600[0] = HV_CHAR;
	switch(dr.sc500[0])	{
	case 'F':		/* field edit option */
		if (flag == 0) 
			{ strcpy(dr.sc600, "FRM FILE NOT YET CREATED"); 
				break;    }
		else
		fldedit();
		if(errnumb==0)
			strcpy(dr.sc600, "FIELD EDIT DONE");
		else    strcpy(dr.sc600, "FIELD EDIT ERROR");
		break;
	case '1':		/* edit screen name */
		fir1 = 1;
		statrec.nextfld = 200;
		dr.sc200[0]=LV_CHAR;
		fomrf((char *) &dr);
		errext(&statrec);
		strcpy(hdrrc.scrnnam, dr.sc200);
		modified=1;
		break;
	case '2':		/* edit version no */
		fir2 = 1;
		statrec.nextfld = 300;
		dr.sc300[0]=LV_CHAR;
		fomrf((char *) &dr);
		errext(&statrec);
		strcpy(hdrrc.version, dr.sc300);
		modified=1;
		break;
	case '3':		/* edit language */
		fir3 = 1;
		statrec.nextfld = 400;
		dr.sc400[0]=LV_CHAR; 
		fomrf((char *) &dr);
		errext(&statrec);
		strcpy(hdrrc.language, dr.sc400);
		break;
	case 'E':
		if(flag==0) 
			if(!fir1||!fir2||!fir3)  {
			    strcpy(dr.sc600, "SCREEN DETAILS NOT COMPLETE");
			    break;     }
		if(hdrrc.scrnnam[0]=='\0') {
			strcpy(dr.sc600, "SCREEN NAME NOT FILLED UP");
			break;    }
			
		dr.sc500[0] = '0';
		errnumb=0;
		return;
	default:
		strcpy(dr.sc600, "SORRY!TRY AGAIN");
		break;
}	statrec.nextfld=600;
	fomwf((char *) &dr);
	errext(&statrec);

}

}

