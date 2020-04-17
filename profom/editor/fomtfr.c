/* profom editor: frm file creation from fom file: frmcrt.c */

#include <stdio.h>
#include "cfomfrm.h"
#include "fcntl.h"
#define SETLATT1(A) field.lattr1=field.lattr1|A
#define SETLATT2(A) field.lattr2=field.lattr2|A
#define SETPVATT(A) field.promva=field.promva|A
#define SETFVATT(A) field.fldva=field.fldva|A
extern struct frmhdr hdrrc;
extern struct frmfld field;
extern char *string, str[];
static int line, len, doff, dfs, offset, recsz;
static FILE *ip;		/* fom file */
static int op;			/* FRM file */
char  aword[90], *word,chrflg;
int errflag;
int nmp,iii;
FILE *ep;
frmfom(scrnme)		/* create FRM file from fom file  */
char *scrnme;
{
char  *wrd, machar,temp[100];
static int   verifier, flag, fldno;
int tint,excd;
int xnum, ynum, lflag, PROM, nochar, i, k, eflag;
char fomfile[16], frmfile[16], errfile[16];
char c;
int atfl,dupflg,mylen;
excd=0;
PROM=0;			/* Open files */
string = str;
word=aword;
dupflg=0;
strcpy(fomfile, scrnme);
strcat(fomfile, ".fom");

if ((ip = fopen(fomfile, "r")) ==NULL)		{
	strcpy(fomfile, scrnme);
	strcat(fomfile, ".FOM");
	if((ip=fopen(fomfile, "r")) ==NULL)
		return(3);	}

strcpy(errfile, scrnme);
strcat(errfile, ".err");
if ((ep = fopen(errfile, "w")) == NULL)	
	return(2);

len = doff = verifier = recsz = fldno = 0;
dfs = line = flag = 1;
*string++ = '\0';
len++;
offset = 1;
skpwhite();		/* skip white space characters */
if ((tint=getword())!= 1)	/* HEADER = 1 */
	filerr("HEADER expected %d", line);
for ( ;flag; )		{
	skpwhite();
	switch(tint=getword())	{
	
	case -1:	/* unsuccessful */
		filerr("not a reserved word %d", line);
		continue;
	case 0:		/* EOF = 0 */
		flag = 0;
		filerr("Premature EOF %d ", line);
		unlink(frmfile);
		exit(1);
	case 23:	/* case of END */
		flag = 0;
		strcpy(frmfile, scrnme);
		if(strcmp(hdrrc.language, "COBOL") == 0)
			strcat(frmfile, ".NFM");
		else	strcat(frmfile, ".nfm");
		if((op=creat(frmfile, 0755)) == -1)
			return(2);
		continue;
	case 2:		/* SCREEN = 2 */
		skpwhite();
		getname(hdrrc.scrnnam);
		++verifier;
		continue;
	case 3:		/* VERSION = 3 */
		skpwhite();
		getname(hdrrc.version);
		++verifier;
		continue;
	case 4:		/* LANGUAGE */
		skpwhite();
		getname(hdrrc.language);
		++verifier;
		continue;
	default:
		filerr("Illegal word in screen header %d", line);
		continue;
	}
	}
	eflag = 1;	/* process individual fields */
	if (verifier != 3)   {	/* 4 */
		verifier = 0;
	filerr ("HEADER record incomplete %d", line);	}
	if (write (op, (char *)&hdrrc, FMH_SZ) < FMH_SZ)
		return(4);
	for( ;eflag; )	{	/* 5 */
		if(excd==1) break;
		tint=skpwhite();
		if(tint == -1)  {
			eflag=0;
			continue;   }
		if ((i = getword()) == -1)
		{	fprintf(ep,"Not A RES.WORD %s\n",word);
			errflag=1;
			break;	}
	if(i==25) /* end of the hing*/
		goto wrt;
else if (i!=5)

			filerr("FIELD expected %d ", line);
		/* fscanf(ip, "%d", &field.fldno); */
		getno(&field.fldno);
		flag = 1;
		for( ;flag; )	{	/* 6 */
			if(excd==1)  break;
			if(skpwhite()==EOF)  { eflag =0; flag = 0; continue;   }
			switch(i = getword())	{	/* 7 */
			case 0:
				eflag = 0; flag = 0;
				continue;
			case 24:
				skpwhite();
				/* fscanf(ip, "%s", word); */
				gname(word);
				field.fldnam = offset;
				offset += mkstrng(word);
				continue;
			case 23:	/* END */
			    if(dupflg==0 && field.fldclas != CL_PROM)
			    {
			    	field.dupval = offset;
					offset += putinstr(field.fldtyp,field.dfsize);
				}
				if(field.fldclas != CL_PROM)
				{
					field.eddata = offset;
					offset += putinstr(TYP_STRING,mylen);
				}
				else
					field.eddata = 0;
				if (write(op,(char *)&field,FMF_SZ) < FMF_SZ)
					return(4);
				dupflg=0;
		
				initfld();
				fldno++;
				PROM=0;
				flag = 0;
				continue;
			case 6:		/* PROMPT */		
				skpwhite();
				field.prompt = offset;
				offset += getcoord(&xnum, &ynum);
				field.promx = xnum;
				field.promy = ynum;
				PROM = 1;	/* for classification */
				field.fldclas = CL_PROM;
				continue;
			case 7:		/* INPUT MASK */
			case 8:
			case 9:
			case 10:
				if(i == 7)field.fldtyp = TYP_DATE; 
				if(i == 8)field.fldtyp = TYP_STRING; 
				if(i == 9)field.fldtyp =  TYP_YN; 
				if(i==10)field.fldtyp=TYP_NUM;
				/* fscanf(ip, "%s", temp); */
				gname(temp);
		expand(temp, word);
		        /*for(nmp=0,iii=0;word[iii] != '\0';iii++)
		        	if(word[iii] == 'm' || word[iii] == 'M') nmp++;
		        if(nmp<3)  chrflg=0;
		        	else   chrflg=1;*/
				field.imask = offset;
				offset += mkstrng(&word[0]);
				if((field.dfsize=valmask(field.fldtyp, field.maskchar, (field.imask+str), (field.dmask+str))) == -2)
				filerr("input/display masks? %d", line);
				field.drloc = doff;
				doff += field.dfsize;
				recsz += field.dfsize;
				field.picstrng = 0;
				/****** deleted for nfm version
				field.picstrng = offset;******/
				if (PROM == 0) field.fldclas = CL_FLD;
				else {
				PROM = 0; field.fldclas = CL_PRMFLD; }
				/****** deleted for nfm version 
				offset += pcrtpist(field.fldtyp, &word[0]);******/
				continue;
			case 11:	/* DISPLAY MASK */
				skpwhite();
				field.dmask = offset;
				/* fscanf(ip, "[%d,%d]", &xnum, &ynum);*/
				coord(&xnum,&ynum);
				skpwhite();
				/* fscanf(ip, "%s", temp); */
				gname(temp);
		expand(temp, word);
				mylen=strlen(word);
				offset += mkstrng(&word[0]);
				field.fldx = xnum;
				field.fldy = ynum;
				continue;
			case 12:
				skpwhite();
				if ((c = vgetc(ip)) != '(')
				    filerr("syntax error %d ", line);
	    			    for( ;lflag = getattr('L', &atfl); )   {
					
					switch(lflag)  {	/* 9 */
					case 1:
					     SETLATT1(LA_REQ);
				             break;
			  		case 2:
			  		     SETLATT1(LA_VALID);
			  		     break;
			  	        case 3:
			  		     SETLATT1(LA_UESC);
			  		     break;
			  	        case 4:
			  		     SETLATT1(LA_NOECHO);
			  		     break;
			  	        case 5:
			  		     SETLATT1(LA_HRET);
			  		     break;
			  		case 6:
			  		     SETLATT1(LA_SUP);
			  		     break;
			  		case 7:
			  		     SETLATT2(LA_BOUNDS);
			  		     break;
			  		case 8:
			  		     SETLATT2(LA_FH);
			  		     break;
			  		case 9:
			  		     SETLATT2(LA_UCASE);
			  		     break;
			  		case 10:
			  		     SETLATT2(LA_LCASE);
			  		     break;
			  		case 11:
			  		     SETLATT2(LA_SHODUP);
			  		     break;
			  		case -1:
			  		     filerr("syntax error %d ",line);
			  		     lflag = 0;
			  		     break;
				}   if(!atfl)   {  atfl = 1; break;  }    }
				  continue;
			  case 13:
			  	  skpwhite();
			  	  field.helpmes = offset;
			  	  offset += getstring();
			  	  continue;
			  case 14:
			  	  skpwhite();
			  	  field.lbound = offset;
			  	  offset += getstring();
			  	  continue;
			  case 15:
			  	  skpwhite();
			  	  field.ubound = offset;
			  	  offset += getstring();
			  	  continue;
			  case 16:
			  	  skpwhite();
			  	/*   fscanf(ip, "%d", &field.promclr); */
			  	getno(&field.promclr);
			  	  continue;
			  case 17:
			  	  skpwhite();
			  	 /*  fscanf(ip, "%d", &field.fldclr); */
			  	getno(&field.fldclr);
			  	  continue;
			  case 18:
			  	  skpwhite();
			  	  c = vgetc(ip);
			  	  if (c == 'M') field.dupctrl = DUP_MASTER;
			  	  else
			  	  if (c == 'C') field.dupctrl = DUP_COPY;
			  	  else
			  	  if (c == 'N') field.dupctrl = 0;
			  	  else filerr("unknown option %d ", line);
			  	  continue;
			  case 19:
			      dupflg=1;
			  	  skpwhite();
			  	  field.dupval = offset;
			  	  offset += getstring();
				  continue;
			  case 20:
				  skpwhite();
				  if ((c=vgetc(ip))!= '(')
					filerr("syntax error in %d", line);
			  	  for( ;lflag=getattr('V', &atfl);)	{    
				     switch (lflag)   {
					case 1:
					     SETPVATT(VA_ULINE);
					     break;
					case 2:
					     SETPVATT(VA_REVERSE);
					     break;
					case 3:
					     SETPVATT(VA_BOLD);
					     break;
					case 4:
					     SETPVATT(VA_DIM);
					     break;
					case 5:
					     SETPVATT(VA_BLINK);
					     break;
					case -1:
					     filerr("error in line %d",line);
					     break;
				}    if(!atfl)  { atfl = 1; break;  }	}
				
				  continue;
			  case 21:
				  skpwhite();
				  if((c=vgetc(ip))!= '(')
					filerr("syntax error in %d", line);
				  for ( ;lflag=getattr('V', &atfl); )	{
				      switch(lflag)	{
					 case 1:
					      SETFVATT(VA_ULINE);
					      break;
					 case 2:
					      SETFVATT(VA_REVERSE);
					      break;
					 case 3:
					      SETFVATT(VA_BOLD);
					      break;
					 case 4:
				              SETFVATT(VA_DIM);
					      break;
					 case 5:
					      SETFVATT(VA_BLINK);
					      break;
					 case -1:
					      filerr("error in line %d",line);
					      break;
				}    if (!atfl)  {  atfl = 1;  break; }	}
				  continue;
			  case 25:
				eflag=0;
				flag=0;
			        excd = 1;
				goto wrt;
			  case 22:
				skpwhite();
				field.maskchar = vgetc(ip);
		}     }  }
wrt:
if (errflag==1)  {
	close(op);
	unlink(frmfile);
	fclose(ep);
	return(11);}
string = str;
if(write (op, string, len) < len)   
	return(4);
if (lseek (op,0L, 0) == -1)	
	return(5);
hdrrc.vdsize = len;
hdrrc.drsize = recsz;
hdrrc.noflds = fldno;
if (write(op, (char *)&hdrrc, FMH_SZ) < FMH_SZ)
	return(4);
fclose(ip);
close(op);
fclose(ep);
unlink(errfile);
return(0);
}


skpwhite()		/* skip white space */
{
char ch;
int efl;
for(; (ch=vgetc(ip))!=EOF&&(ch==' ' || ch== '\t' || ch== '\n'); )   
	if (ch== '\n') ++line;
if(ch == EOF)  return(-1);
efl=vungetc(ch, ip);
if(efl == EOF)  return(-1);
if(ch == EOF)	return(-1); else return(0);
  }


getword()		/* get reserved word */
{
char ch;
int i;
for (i=0;((ch=vgetc(ip)) != EOF) && ch != ' ' && ch != '\t' && ch != '\n';i++)
	word[i] = ch;
if(ch == '\n') ++line;
word[i] = '\0';
if(ch == EOF) return(0);
if((i=isresword(word)) == 0)	return(-1);
else return(i);
}


filerr(s1,d1)
char *s1;
int d1;
{
fprintf(ep, s1, d1);
fprintf(ep,"Word %s \n",word);
fprintf(ep, "\n");
errflag = 1;
return;
}


getname(name)
char name[];
{
int i;
char ch;
for(i=0; (ch = vgetc(ip)) != EOF && ch!=' '&&ch!='\t'&&ch!='\n'; i++)
	name[i] = ch;
if (ch == '\n') ++line;
if (ch == EOF) filerr("premature EOF on fomfile %d",line);
name[i] = '\0';
return;
}



getattr(a,flg)		/* get logical and video attributes */
char a;			/* if a = 'L' get logical and if 'V' get video */
int *flg;
{
char ch;
int i;
skpwhite();
for (i=0;(ch=vgetc(ip))!=EOF&&ch!=' '&&ch!='\t'&&ch!='\n'&&ch!=','&&ch!=')';i++)
	word[i] = ch;
if(ch == '\n') ++line;
word[i] = '\0';
if(ch==')')  *flg = 0; else *flg = 1;
if (a == 'L')	return(logattr());
else return(vidattr());

}


logattr()
{
char *wrd;
int i;
static char *lattr[] = {
	"dummy",
	"REQUIRED",
	"VALIDITY",
	"UESCAPE",
	"NO_ECHO",
	"HELP_RET",
	"SUP_ZEROES",
	"BOUNDS",
	"FHOLD",
	"UCASE",
	"LCASE",
	"SHODUP",
	"Illegal case"
};
wrd = &word[0];

for (i=1;i<12;i++)
	if(strcmp(wrd,lattr[i]) == 0) return(i);
if(i>=12) return(-1);
}


vidattr()

{
char *wrd;
int i;
static char *vattr[]	  = {
	"dummy",
	"ULINE",
	"REVERSE",
	"BOLD",
	"DIM",
	"BLINK",
	"Illegal case"
};
wrd = &word[0];
for(i=0;i<6;i++)
	if(strcmp(wrd,vattr[i]) == 0) return(i);
if(i>=6) return(-1);
}



getstring()
{
char ch;
int i;
if((ch=vgetc(ip)) != '"')     {
	filerr("quote expected %d ", line);
/*putc(ep, ch);*/    }
for(i=0;(ch=vgetc(ip)) != EOF && ch != '"';i++)
{	*string++ = ch; len++;
/*putc(ep, ch);*/    }
if(ch == EOF) filerr("premature EOF %d", line);
*string++ = '\0';  len++;
return(i+1);
}



getcoord(x,y)
int *x, *y;
{
int i, k;
char osb,csb,comma;
/* k = fscanf(ip, "[%d,%d]", x, y); */
 coord(x,y);
/* if (k == -1) */
	/* filerr("premature EOF %d", line); */
skpwhite();
i = getstring();
return(i);
}

mkstrng(chstr)		/* chstr is the set of characters to be */
			/* concatenated to the string */
char *chstr;
{

int size = 0;
char c;
if (chstr == NULL) return(0);
for ( ;*chstr != '\0'; size++)
{
	*string++ = *chstr++; len++; }
if (size) 
{	*string++ = '\0';  len++;
	return(size+1);	}
else 	return(0);
}



isresword(token)		/* returns token no if reserved word else 0 */
char token[];
{
int i;
static char *resword[] = {
		"Dummy",
		"HEADER",		/*   1  */
		"SCREEN",		/*   2  */
		"VERSION",		/*   3  */
		"LANGUAGE",		/*   4  */
		"FIELD",		/*   5  */
		"PROMPT",		/*   6  */
		"DATE",			/*   7 */
		"STRING",		/*   8 */
		"BOOLEAN",		/*   9  */
		"NUMERIC",		/*  10  */
		"DISPLAY_MASK",		/*  11  */
		"LOG_ATTR",		/*  12  */
		"HELP_MESSAGE",		/*  13  */
		"LOWER_BOUND",		/*  14  */
		"UPPER_BOUND",		/*  15  */
		"COLOR_PROMPT",		/*   16 */
		"COLOR_MASK",		/*  17  */
		"DUP_CTRL",		/*   18 */
		"DUP_VALUE",		/*   19 */
		"PROMPT_VIDEO",		/*  20  */
		"MASK_VIDEO",		/*   21 */
		"MASK_CHAR",		/*  22  */
		"END",			/*  23   */
		"FIELDNAME",		/*  24  */
		"ENDFILE",	/* 25  */
		"Illegal word"	};

for (i = 0; i<=25; i++)
	if (strcmp(token, resword[i]) != 0) continue;
	else break;
return ((i>25) ? 0 : i);

}

pcrtpist(typ, wrd)
int typ;
char *wrd;
{
int i, m;
char *ima;
int size;
ima = wrd;
switch(typ)     {
case TYP_STRING:
	size= pcat ('X',field.dfsize);
	break;
case TYP_NUM:
	for (i=1; *ima != '\0'; i++)
	{	*string++ = *ima++;
		len++;	}
	size= i;
	break;
case TYP_YN:
	*string++ = 'X';
	size= 2;
	break;
case TYP_DATE:
	for (m=0; *ima != '\0';)
		if (*ima++ == 'M') m++;
	if (m == 3){chrflg=1; size= pcat ('X', field.dfsize);}
 else {		chrflg=0;
		    size= pcat ('9', field.dfsize); }
	break;
	}
*string++ = '\0';
len++;
return(size);

}

pcat(c, ln)
char c;
int ln;
{
int i;
for (i=1; i<=ln; i++)  {
	*string++ = c;
	len++;         }
return(i);
}

initfld()
{
/* initialize the fields of the structure & return */
field.fldnam = 0;
field.prompt = 0;
field.promx = 0;
field.promy = 0;
field.fldx = 0;
field.fldy = 0;
field.imask = 0;
field.dmask = 0;
field.drloc = 0;
field.picstrng = 0;
field.helpmes = 0;
field.lbound = 0;
field.ubound = 0;
field.promclr = 0;
field.fldclr = 0;
field.dupctrl = 0;
field.dupval = 0;
field.lattr1 = 0;
field.lattr2 = 0;
field.promva = 0;
field.fldva = 0;
field.maskchar = 0;
field.dfsize = 0;
field.eddata=0;
field.fldtyp=TYP_NONE;
}

getno(num)
int *num;
{
char charr[6];
int jjj;
skpwhite();
gname(charr);
jjj=atoi(charr);
*num=jjj;
return(jjj);
}

gname(arry)
char arry[];
{
int noi;
char ch;
skpwhite();
for(noi=0;(ch=vgetc(ip)) != EOF && ch != ' ' && ch != '\t' && ch != '\n'; noi++)
	arry[noi] = ch;
arry[noi] = '\0';

if(ch == '\n') ++line;
return;
}

coord(xptr,yptr)
 int *xptr,*yptr;
 {

	char ch;
	skpwhite();
	if((ch=vgetc(ip))!='[')
			return(-1);
	*xptr=0;*yptr=0;

	while((ch=vgetc(ip))>='0' &&ch<='9')
		*xptr=(*xptr)*10+ch-'0';

	if(ch!=',')
		return(-1);

	while((ch=vgetc(ip))>='0' &&ch<='9')
		*yptr=(*yptr)*10+ch-'0';
	if(ch!=']')
		return(-1);
return(0);
  }  /*coord  */
 


putinstr(typstrn,size)
char typstrn;
int size;
{
int rin;
char tcrc;
switch(typstrn)
{
case TYP_STRING:
	tcrc=' ';
	break;

case TYP_NUM:
	tcrc='0';
	break;

case TYP_DATE:
	if(chrflg)  tcrc=' ';
	else		tcrc='0';
	break;
case TYP_YN:
	tcrc='0';
	break;


default:
	break;

}
for(rin=0;rin<size;rin++)
{	*string++ = tcrc;
	len++;              
}
*string++ = '\0';
len++;
return(rin+1);
}
