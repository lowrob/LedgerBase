/* crterfl.c PROFOM error message file creation program */

#include <stdlib.h>
#include "stdio.h"
#include "cfomerr.h"
#include "fcntl.h"

#define MAXLL 256
static struct erflhdr eh;
static char mess[MAXERRORS*MAXEML+1];
static char line[MAXLL+1];

static char cem[MAXLL+1];
static int ofd,msoff,syntaxerr,eno,sl,sr;

char *fgets();
/** 3-JULY-90 added by J.Prescott           **/
/**  to replace "fomerr.src" & "fomerr.bin" **/
/**  in opens and creates 		    **/
#ifdef ENGLISH
#define FOMERRSRC	"fomerr.src"
#define FOMERRBIN	"fomerr.bin"
#else	/* French */
#define FOMERRSRC	"ffomerr.src"
#define FOMERRBIN	"ffomerr.bin"
#endif

/*------------------------------------------*/
main()
{
char	*cp1,*cp2;

	if(freopen(FOMERRSRC, "r",stdin) == NULL)	{
		fprintf(stderr,"unable to open %s\n",FOMERRSRC);
		exit(0);	}
	msoff = 0;
	mess[msoff++] = '\0';
	syntaxerr = 0;
while(fgets(line,MAXLL,stdin) != NULL)  {
	printf(line);
	if((sr = sscanf(line,"%d%s",&eno,cem)) == 0)
		continue;
	else
		if(sr != 2)  {
			printf(" : syntax error\n");
			syntaxerr = 1;
			continue;
			}
	cp1=line;
	while (*cp1 == '\t' || *cp1 == ' ')
		cp1++;
	while (*cp1 >= '0' && *cp1 <= '9')
		cp1++;
	while (*cp1 == '\t' || *cp1 == ' ')
		cp1++;
	cp2=cem;
	while (*cp1 != '\n' && *cp1 != '\0')
		*cp2++ = *cp1++;
	*cp2 = '\0';
	if(eno < 1 || eno > MAXERRORS)   {
		printf(" : error out of range - ignored\n");
		continue;
		}

	if(eh.eroff[eno - 1])  {
		printf(" : duplicate ignored\n");
		continue;	}

	if((sl = strlen(cem)) > MAXEML)  {
		printf(" : message will be truncated\n");
		sl = MAXEML;
		cem[MAXEML] = '\0';
		}
	strcpy(mess+msoff,cem);
	eh.eroff[eno - 1] = msoff;
	msoff += sl + 1;
	}
if(syntaxerr)   {
	fprintf(stderr,"output not generated due to syntax errors\n");
	exit(0);}
if((ofd = creat(FOMERRBIN,0667)) == -1)  {
	fprintf(stderr,"unable to open %s\n",FOMERRBIN);
	exit(0);	}
eh.messlen = msoff;
if(write(ofd,(char *) &eh, EHSZ) != EHSZ)  {
	fprintf(stderr, "write error on %s\n",FOMERRBIN);
	unlink(FOMERRBIN);
	exit(0);	}
if(write(ofd,mess,msoff) != msoff)  {
	fprintf(stderr,"write error on %s\n",FOMERRBIN);
	unlink(FOMERRBIN);
	exit(0);	}
close(ofd);
exit(0);
}

