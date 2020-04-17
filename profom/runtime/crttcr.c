/* Create TCR.BIN file IBM-PC */
#include <stdio.h>
#include "cfomtcr.h"
#include "cfomdef.h"
#include <fcntl.h> 

#define ESCAPE 033
#define TCRBIN "fomtcr.bin"
#define te(X)  tb.fix.X
#define PMODE 0644

struct tcrdir td;
struct tcrblk tb;
int fd;
int off;

main()	{
	register int i;
	off = 0;
	strcpy(td.trmnm[0],"MM");
	td.trmblk[0]=1;
	strcpy(td.trmnm[1],"CM");
	td.trmblk[1]=2;
	strcpy(td.trmnm[2],"W85");
	td.trmblk[2]=3;
	strcpy(td.trmnm[3],"W50");
	td.trmblk[3]=4;
	strcpy(td.trmnm[4],"N7901");
	td.trmblk[4]=5;
for(i=5;i<DIR_ENTS;i++)
{
	td.trmnm[i][0]='\0';
	td.trmblk[i]=0;
}

if((fd=creat(TCRBIN,0755)) == -1)
{
	printf("Unable To Create %s ",TCRBIN);
	exit(1);	}

if(write(fd,(char *) &td,TCD_SZ) != TCD_SZ)	
{
	printf("Write Error\n");
	exit(2);	}

/* MM (Monochrome Monitor) Entry */

off=0;
tb.ctrlcd[off++]='\0';
te(ttyp)=1;
te(lines)=25;
te(cols)=80;
te(lock)=0;
te(unlock)=0;
te(wrap)=1;
te(ceol)=sce("[K");
te(clin)=0;
te(cscrn)=sce("[2J");
te(left)=sce("[1D");
te(right)=sce("[1C");
te(up)=sce("[1A");
te(down)=sce("[1B");
te(home)=sce("[H");
te(sfa)=0;
te(efa)=sce("[0m");
te(cva)=0;
te(move)=sce("[");
te(movesep)=stc(";");
te(xy)=0;
te(constant)=0;
te(vabase)=0;
te(bell)=0;
te(rv)=sce("[7m");
te(blk)=sce("[5m");
te(hiten)=sce("[1m");
te(loten)=0;
te(uline)=sce("[4m");
te(mcode)=0;
te(colour)=0;
te(cscrndelay)=0;
te(clindelay)=0;
te(ceoldelay)=0;
if(write(fd,(char *) &tb, TCB_SZ) != TCB_SZ)
{	printf("write error\n");
	exit(3);	}

/* CM (Colour Monitor) Entry */

off=0;
tb.ctrlcd[off++]='\0';
te(ttyp)=1;
te(lines)=25;
te(cols)=80;
te(lock)=0;
te(unlock)=0;
te(wrap)=1;
te(ceol)=sce("[K");
te(clin)=0;
te(cscrn)=sce("[2J");
te(left)=sce("[1D");
te(right)=sce("[1C");
te(up)=sce("[1A");
te(down)=sce("[1B");
te(home)=sce("[H");
te(sfa)=0;
te(efa)=sce("[0m");
te(cva)=0;
te(move)=sce("[");
te(movesep)=stc(";");
te(xy)=0;
te(constant)=0;
te(vabase)=0;
te(bell)=0;
te(rv)=sce("[7m");
te(blk)=sce("[5m");
te(hiten)=sce("[1m");
te(loten)=0;
te(uline)=sce("[4m");
te(mcode)=0;
te(colour)=1;
te(cscrndelay)=0;
te(clindelay)=0;
te(ceoldelay)=0;
if(write(fd,(char *) &tb, TCB_SZ) != TCB_SZ)
{	printf("write error\n");
	exit(3);	}
/* Wyse 85 Terminal Characteristics */
off=0;
tb.ctrlcd[off++]='\0';
te(ttyp)=1;
te(lines)=24;
te(cols)=80;
te(lock)=0;
te(unlock)=0;
te(wrap)=1;
te(ceol)=sce("[0K");
te(clin)=sce("[2K");
te(cscrn)=sce("[2J");
te(left)=sce("[1D");
te(right)=sce("[1C");
te(up)=sce("[1A");
te(down)=sce("[1B");
te(home)=sce("[1;1H");
te(sfa)= 0 ;   /* sce("["); */
te(efa)=sce("[0m");
te(cva)=0;
te(move)=sce("[");
te(movesep)=stc(";");
te(xy)=0;
te(constant)=0;
te(vabase)=0;
te(bell)=0;
te(rv)=sce("[7m");
te(blk)=sce("[5m");
te(hiten)=sce("[1m");
te(loten)=sce("[2m");
te(uline)=sce("[4m");
te(mcode)=0;
te(colour)=0;
te(cscrndelay)=50;
te(clindelay)=0;
te(ceoldelay)=0;

if(write(fd, (char *) &tb, TCB_SZ) != TCB_SZ)
{printf("write error\n");
exit(0);
}

/* Wyse 50 Terminal characteristics */

off=0;
tb.ctrlcd[off++] = '\0';
te(ttyp)=0;
te(lines)=24;
te(cols)=80;
te(lock)=0;
te(unlock)=0;
te(wrap)=1;
te(ceol)=sce("t");
te(clin)=0;
te(cscrn)=sce("*");
te(left)=sc('H');
te(right)=sc('L');
te(up)=sc('K');
te(down)=sc('V');
te(home)=sc('^');
te(sfa)=sce("G");
te(efa)=sce("G0");
te(cva)=0;
te(move)=sce("=");
te(movesep)=0;
te(xy)=0;
te(constant)=32;
te(vabase)=00;
te(rv)='4';
te(blk)='2';
te(hiten)=0;
te(loten)='p';
te(uline)='8';
te(mcode)=0;
te(colour)=0;
te(cscrndelay)=0;
te(clindelay)=0;
te(ceoldelay)=0;
if(write(fd,(char *) &tb,TCB_SZ) != TCB_SZ)  {
	printf("write error\n");
	exit(4);                             }

/* NCR 7901 Terminal characteristics */

off=0;
tb.ctrlcd[off++] = '\0';
te(ttyp)=1;
te(lines)=24;
te(cols)=80;
te(lock)=0;
te(unlock)=0;
te(wrap)=1;
te(ceol)=sce("K");
te(clin)=sce("K");
te(cscrn)=sc('L');
te(left)=sc('U');
te(right)=sc('F');
te(up)=sc('Z');
te(down)=sc('J');
te(home)=0;
te(sfa)=sc('N');
te(efa)=sc('O');
te(cva)=0;
te(move)=sce("Y");
te(movesep)=0;
te(xy)=0;
te(constant)=32;
te(vabase)=00;
te(rv)=sce("0P");
te(blk)=sce("0B");
te(hiten)=0;
te(loten)=sce("0A");
te(uline)=sce("0\\");
te(mcode)=0;
te(colour)=0;
te(cscrndelay)=0;
te(clindelay)=0;
te(ceoldelay)=0;
if(write(fd,(char *) &tb,TCB_SZ) != TCB_SZ)  {
	printf("write error\n");
	exit(4);                             }

close(fd);
printf("Done Crttcr!!\n");
exit(0);
}




sce(s)
char *s;
{
int i;

i=off;
tb.ctrlcd[off++]=ESCAPE;
while(tb.ctrlcd[off++] = *(s++))
	;
return(i);
}
stc(s)
char *s;{
int i;

i=off;
while (tb.ctrlcd[off++] = *(s++))
	;
return(i);
}


sc(c)
char c;  {
int i;
i=off;
tb.ctrlcd[off++] = (c - '@');
tb.ctrlcd[off++] = '\0';
return(i);
}
