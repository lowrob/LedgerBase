/*-------------------------------------------------------------------------*
VERSION 2.0 2-7-89     RASMUSSEN SOFTWARE INC.
      Made following changes:
         1. Removed delays, which had been required by DMV-COMM.
         2. Give banner screen
VERSION 2.1 10-16-90:
         1. Declare "ch" as (signed) integer, to work on IBM machines
            and others that assume type "char" is unsigned
 *-------------------------------------------------------------------------*/
#include <stdio.h>

/* Needed for Anzio Spooler */
#define         DC1     0X11
#define         DC2     0X12
#define         DC3     0X13
#define         DC4     0X14
#define         BEL     0X07
#define         MAX     2000
int             ch;
int             ctr;
long            ctdn;

Send_PC(filename,device,c_mesg)
char *filename;
char *device;		/* LST: - printer  or drive:filename for disk */
char *c_mesg;
{
	FILE *in;

	in = fopen(filename,"r");
	if (in == NULL) {   
		sprintf(c_mesg,"Can not open %s\n",filename);
		return(-1);
	}
	ctr = 0;
	if (filename[0] != NULL && device[0] != NULL) {   
		putchar(DC1);
		printf("CLOSEO");
		putchar(DC3);
		putchar('\n');
		putchar(DC1);
		printf("OPENO/N %s",device);
		putchar(DC3);
		putchar('\n');
	}
	putchar(DC1);
	printf("LOCK ON");
	putchar(DC3);
	putchar('\n');
	putchar(DC2);
	while ((ch = getc(in)) != EOF) {
		putchar(ch);
		ctr++;
		if (ctr > MAX) {
			handshake();
			ctr = 0;
		}
	}
	if(strcmp(device,"LST:") == 0) 
		putchar('\f');	/* form feed printer */

	if (ctr != 0) {
		handshake();
		ctr=0;
	}
	putchar(DC4);
	fclose(in);
	putchar(DC1);
	printf("CLOSEO");
  	putchar(DC3);
	putchar('\n');
	putchar(DC1);
	printf("LOCK OFF");
	putchar(DC3);
	putchar('\n');

	return(0);
}

handshake()
{
	putchar(DC4);
	putchar(BEL);
	ch = getchar();
	putchar(DC2);

	return(0);
}
