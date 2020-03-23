/* RECV-PC.C from RASMUSSEN SOFTWARE INC.
   Version 1.0  11-21-89 
   Version 1.1  07-23-90 : We weren't always getting the 'end' indicator.
*/
 
#include        <stdio.h>
#include        <errno.h>
#include        <fcntl.h>
#include        <termio.h>

#define         DC1     0X11
#define         DC2     0X12
#define         DC3     0X13
#define         DC4     0X14
#define         BEL     0X07
#define         MAX     8000
#define         DONE    0X1F
#define         BADWRITE "ERROR ON WRITE IN UNIX"
#define         TRUE    1

char            ch;
char            bufr[MAX];
int             i;
int             result;
struct termio   tt_save;
struct termio   tt_work;
long            bytecount;
long            linecount;

Recv_PC(unixfile,pcfile,e_mesg)
   char	*unixfile;
   char *pcfile;
   char *e_mesg;
{
   int outfile;
 
   outfile = open(unixfile,O_WRONLY | O_CREAT | O_TRUNC, 0666);
   if (outfile == -1)
      {
      sprintf(e_mesg,"Can not open %s\n",unixfile);
      return(-1);
      }
   if (unixfile[0] != NULL && pcfile[0] != NULL)
      {
       putchar(DC1);
       printf("CLOSEI");
       putchar(DC3);
       putchar('\n');
       putchar(DC1);
       printf("OPENI/S %s",pcfile);
       putchar(DC3);
       putchar('\n');
       ch = getchar(); /*leading zero*/
       ch = getchar();
       if (ch != '0')
          {
          sprintf(e_mesg,"Can't open PC file named %s\n", pcfile);
          return(-1);
          }
       ch = getchar();
       putchar(DC1);
       printf("TRANSMIT TRAILER ");
       putchar(DONE);
       putchar(DC3);
       putchar('\n');
      }
   
   ioctl(0, TCGETA, &tt_save);
   tt_work = tt_save;
   tt_work.c_lflag &= ~ICANON; /* turn off icanon */
   tt_work.c_iflag |= IXOFF;
   tt_work.c_cc[4] = 4;        /* 1.01 */
   tt_work.c_cc[5] = 2;        /* 1.01 */
   ioctl(0, TCSETA, &tt_work);
 
   i = 0;
   linecount = 0;
   bytecount = 0;
   while (TRUE)
      {
      ch = getchar();
      if (ch == DONE)
         {
         putchar(BEL);
         if (i != (result = write(outfile, bufr, i)))
            {
            if (result == -1)
               {
               if (errno == EBADF)
                  printf("EBADF");
               else if (errno == ENOSPC)
                  printf("ENOSPC");
               }
            else
               printf(BADWRITE);
            ioctl(0, TCSETA, &tt_save);
            exit(1);
            }
         close(outfile);
         ioctl(0, TCSETA, &tt_save);
         printf("Lines: %d  Bytes: %d\n", linecount, bytecount);
         exit(0);
         }
      if (ch == '\n')
         linecount++;
      bytecount++;
      bufr[i++] = ch;
      if (i >= MAX)
         {
         if (MAX != write(outfile, bufr, MAX))
            {
            printf(BADWRITE);
            ioctl(0, TCSETA, &tt_save);
            exit(1);
            }
         i = 0;
         }
      }
}
 
 
