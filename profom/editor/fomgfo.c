/* profom editor: fom file creation from frm file: genfom.c */

#include <stdio.h>
#include <fcntl.h>

#define subst if(!first) fprintf(fp,",");else first=0;
#include "cfomfrm.h"
fomfrm()			/* generate fom file from frm file */
{
	char temp[80];
	extern struct frmhdr hdrrc;
	extern struct frmfld field;
	
	FILE *fp,*fopen();
	char filnam[16];
	char *calloc(); 
	extern char *string, str[];
	int first, success,dotposn;
	int fd,i,n,val,j;
	strcpy(filnam,hdrrc.scrnnam);
	if(strcmp(hdrrc.language, "COBOL") == 0)
		strcat(filnam, ".NFM");
	else
		strcat(filnam,".nfm");
	success=0;
	string = str;
	while(!success) {
		for(dotposn=0;filnam[dotposn]!='.' && filnam[dotposn]!='\0';dotposn++);
		if(filnam[dotposn]=='.' &&dotposn>0) 
			if(strcmp((filnam+dotposn),".nfm")==0 || strcmp((filnam+dotposn), ".NFM")==0) success=1;
		
	}  /* while */



	if((fd=open(filnam,(O_RDONLY)))<=0) {
		
		return(3);
	}
	if(strcmp(hdrrc.language, "COBOL") == 0)
	{	filnam[dotposn+1]='F';
		filnam[dotposn+2]='O';
		filnam[dotposn+3]='M';	}
	else	{
	filnam[dotposn+1]='f';
	filnam[dotposn+2]='o';
	filnam[dotposn+3]='m';	}
	
	fp=fopen(filnam,"w");
	if(fp<=0) {
		return(2);
	}
	n=read(fd,(char *)&hdrrc,FMH_SZ); /* Read the header structure */
	fprintf(fp,"HEADER\n");
	fprintf(fp,"SCREEN  %s\n",hdrrc.scrnnam);
	fprintf(fp,"VERSION %s\n",hdrrc.version);
	fprintf(fp,"LANGUAGE %s\n",hdrrc.language);
	fprintf(fp,"END\n");
	lseek(fd,(long)(FMH_SZ+FMF_SZ*(hdrrc.noflds)),0);
	n=read(fd,string,hdrrc.vdsize);

	lseek(fd,(long)FMH_SZ,0);

	for (i=1;(i<=(hdrrc.noflds));i++) {
		n=read(fd,(char *) &field,FMF_SZ);
		fprintf(fp,"\n FIELD    %d \n",(field.fldno));
		if((field.fldnam)!=0)
			fprintf(fp,"\n FIELDNAME     %s \n",(field.fldnam)+string);
		if((field.fldclas) != CL_PROM)
		fprintf(fp,"\n MASK_CHAR    %c \n", (field.maskchar));
		if((field.fldclas)!=CL_FLD)
			fprintf(fp,"\n PROMPT [%d,%d]  \"%s\" \n",(field.promx),(field.promy),
			((field.prompt)+string));

		if ((field.fldclas)!=CL_PROM) {
			comp(temp, ((field.dmask)+string));
			fprintf(fp,"\n DISPLAY_MASK  [%d,%d] %s \n",(field.fldx),(field.fldy),
			temp);
			
			if ((field.imask)!=0) {
			
			
				switch(field.fldtyp) {
				case TYP_YN: 
					fprintf(fp,"BOOLEAN ");
					break;

				case TYP_DATE:
					fprintf(fp,"DATE  ");
					break;

				case TYP_NUM: 
					fprintf(fp,"NUMERIC ");
					break;

				case TYP_STRING: 
					fprintf(fp,"STRING");
				default:
					break;
				}
			comp(temp, ((field.imask)+string));
			fprintf(fp,"  %s\n",temp);

				
			}

		} /* if field !=    */
		if(field.lattr1>0 ||field.lattr2>0) {
			first=1;
			val=field.lattr1;
			fprintf(fp,"LOG_ATTR (");

			if((val)&LA_REQ)   {

				subst;
				fprintf(fp,"REQUIRED");
			}

			if((val)&LA_VALID)  {
				subst;

				fprintf(fp,"VALIDITY");
			}


			if((val)&LA_UESC)  {
				subst;

				fprintf(fp,"UESCAPE");
			}


			if((val)&LA_NOECHO)  {
				subst;

				fprintf(fp,"NO_ECHO");
			}



			if ((val)&LA_HRET )  {
				subst;

				fprintf(fp,"HELP_RET");
			}


			if ((val)&LA_SUP)  {
				subst;

				fprintf(fp,"SUP_ZEROES");
			}


			val=field.lattr2;
			if((val)&LA_BOUNDS)  {
				subst;

				fprintf(fp,"BOUNDS");
			}


			if((val)&LA_FH)  {
				subst;

				fprintf(fp,"FHOLD");
			}


			if((val)&LA_UCASE)  {
				subst;

				fprintf(fp,"UCASE");
			}

			if((val)&LA_LCASE)  {
				subst;

				fprintf(fp,"LCASE");
			}

			if((val)&LA_SHODUP)   {
				subst;

				fprintf(fp,"SHODUP");
			}






			fprintf(fp,")\n");


			if(val&LA_BOUNDS) {
				if ((field.lbound)!=0)
					fprintf(fp,"LOWER_BOUND  \"%s\"\n",(string+(field.lbound)));

				if((field.ubound)!=0)
					fprintf(fp,"UPPER_BOUND  \"%s\"\n",(string+(field.ubound)));
			}

		} /* if (lattr1!=0 ||lattr2!=0) */
		if(field.dupctrl!=DUP_NONE) {
			if(field.dupctrl ==DUP_MASTER)
				fprintf(fp,"\n DUP_CTRL  M \n");
			else         fprintf(fp,"\n DUP_CTRL  C    \n");

			if(field.dupval!=0)
				fprintf(fp,"\n DUP_VALUE \"%s\" \n",(string+field.dupval));
		}



		if((field.helpmes)!=0)
			fprintf(fp,"HELP_MESSAGE  \"%s\"\n",(string+(field.helpmes)));

		for(j=1;j<=2;j++) {
			first=0;
			if(j==1 && field.promva !=0) {
				val=field.promva;
				fprintf(fp,"PROMPT_VIDEO (");
				first=1;
			}

			if(j==2 && field.fldva!=0)  {
				val=field.fldva;
				fprintf(fp,"MASK_VIDEO (");
				first=1;
			}
			if(first)  {
				if((val)&VA_ULINE) {
					subst;
					fprintf(fp,"ULINE");
				}

				if((val)&VA_REVERSE) {
					subst;
					fprintf(fp,"REVERSE");
				}
				if((val)&VA_BOLD) {
					subst;
				fprintf(fp, "BOLD");
				}
				if((val)&VA_DIM)  {
					subst;
					fprintf(fp,"DIM");
				}

				if((val)&VA_BLINK)  {
					subst;
					fprintf(fp,"BLINK");
				}
				fprintf(fp,")\n");
			} /* if(first) */
                    } /* for j= ... */
                 fprintf(fp,"\nEND\n");
		} /*for i=1                               */
fprintf(fp,"\nENDFILE\n");
close(fd);
fclose(fp);	
return(0);
}
