/*----------------------------------------------------------------
	Source Name : brkrpt.c
	Author      : D Richardson
	Created     : 20th October 1992.

	This file contains the signal handler that is called when
	a SIGINT, interrupt process, signal is intercepted. It
	will remove all new and temporary files created by a 
	report generation.  	
----------------------------------------------------------------*/

#include <stdio.h>
#include <signal.h>
#include <string.h>

char bk_filename[40];
short bk_rpt_opn_flg;
extern int errno;

/*--------------------------------------------------------------------------
    Parameters: integer, which represents a signal that is defined in
		signal.h
    Returns:    Nothing.
    Calls:      sigsend - for systems with UNIX SystemV.4 or greater
		kill    - for systems with lesser release of UNIX
    Called By:  - in the DBH for 'mkln' reports, recio.c:
			get_isrec()
			get_next()
			get_rec()
			get_seqrec()
		- in the ReportWriter for rpopen reports, rp.c:
			rpline()
		- in send-pc for Anzio spooled reports, send-pc.c:
			Send_PC()
    Modifies:   kills the calling process and deletes any temporary
		report files and unlocks the keyboard for Anzio
    Description:
 
    This routine is called when the 'break key' is pressed during
    report generation routines.  It will kill the report, unlock the keyboard
    for Anzio, and remove and remove any files associated with this report,
    i.e. temporary print files or data files. It is called from these functions:
	In the DBH for a mkln report,
		get_isrec()
		get_next()
		get_rec()
		get_seqrec()
	In the reportwriter for a rpopen report,
		rpline()
	In Send_PC for Anzio spooled reports,
		Send_PC()
 -------------------------------------------------------------------------*/

void BreakReport(int sig)
{
	char command[80];
	
	strcpy(command,"rm ");
	strcat(command,bk_filename);
	strcat(command," > /dev/null");
	system(command);

/* 'sigsend' is only available in UNIX V.4r2 or greater */
/*
	if ( sigsend(P_PID,P_MYID,SIGKILL) == -1 )
*/

	if ( kill(getpid(),SIGKILL) == -1 )
		fprintf(stderr,"\nSignal processing error number %d\n",errno);	
}

/*-------------------- E n d   O f   P r o g r a m ---------------------*/
