/*-----------------------------------------------------------------------------
PROGRAM: chqcheck.c
AUTHOR: Louis Robichaud
DESCRIPTION: This program will send all the process on the system to a
	file (psfile.tmp). Then a grep is done on that file to check for
	certain programs are running in the APS system. The grep that 
	checks the file does a line count of the processes and that number
	is stored in another file (psnbr.tmp). The file (psnbr.tmp) is then
	read and if it is 0 then no processes are running.
	This program looks to see if cheque processing is going on.

This program currently searches for the following processes:

	cheque.c	|	chqcancl.c
	pselect.c	|	deselect.c

This Program is called by:

	apinvc.c

Modifications:
  Programmer	Description
  ----------	-----------
------------------------------------------------------------------------------*/
#include <stdio.h>
#include <bfs_defs.h>

static char	e_mesg[50];

/* To check for other process, call checkprocess and pass the name of the 
 * process that is being looked for. Then check for an error or if any
 * processes were found. If process were found remove 2 temp files and pass
 * back number of processes */
chqcheck()
{
	int	processes, err;

	/* Put all processes into a file */
	strcpy(e_mesg, "ps -ef > psfile.tmp");
	err = system(e_mesg);
	if(err)
		return(ERROR);

	processes = checkprocess("cheque.out");
	if(processes == ERROR)
		return(ERROR);
	if(processes > 0){	/* Process running cheque.out */
		/* Remove temporary files */
		strcpy(e_mesg, "rm psfile.tmp psnbr.tmp");
		system(e_mesg);
		return(processes);
	}

	processes = checkprocess("chqcancl.out");
	if(processes == ERROR)
		return(ERROR);
	if(processes > 0){	/* Process running cheque cancelation */
		/* Remove temporary files */
		strcpy(e_mesg, "rm psfile.tmp psnbr.tmp");
		system(e_mesg);
		return(processes);
	}
	processes = checkprocess("pselect.out");
	if(processes == ERROR)
		return(ERROR);
	if(processes > 0){	/* Process running cheque cancelation */
		/* Remove temporary files */
		strcpy(e_mesg, "rm psfile.tmp psnbr.tmp");
		system(e_mesg);
		return(processes);
	}
	processes = checkprocess("deselect.out");
	if(processes == ERROR)
		return(ERROR);
	if(processes > 0){	/* Process running cheque cancelation */
		/* Remove temporary files */
		strcpy(e_mesg, "rm psfile.tmp psnbr.tmp");
		system(e_mesg);
		return(processes);
	}
	/* Remove temporary files */
	strcpy(e_mesg, "rm psfile.tmp psnbr.tmp");
	system(e_mesg);
	return(processes); /* processes will be 0 */
}

/* check psfile.tmp file for the program name passed from the calling 
 * function. Pass back the number of processes found.	*/
checkprocess(program)
	char	*program;
{
	int err, fd, processes ;
	char  ans[30];

	/* Search for cheque process, and put the number to file */
	sprintf(e_mesg, "grep %s psfile.tmp | wc -l > psnbr.tmp",program);
	err = system(e_mesg);
	if(err)
		return(ERROR);

	/* read file with number in it (psnbr.tmp) */
	fd=open("psnbr.tmp",RDMODE);
	read(fd,ans,80);

	/* convert number stored as char to int. */
	processes=atoi(ans);

	return(processes);
}
