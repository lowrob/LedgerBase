/******************************************************************************
		Sourcename   : eod.c
		System       : Budgetary Financial system.
		Module       : GL.
		Created on   : 89-07-28
		Created  By  : K HARISH.
		Cobol Source : 
******************************************************************************
About the program:

	This program does end of day processes which include:

	1.Posting Daily recurring entries ( Weekly, Bi-weekly too if reqd )
	2.Listing out all the transactions of the day on the printer
	3.Printing the audit report for the day, and later deleting auditfile
	4.Backing up the journal
	5.Rolling over the BFS date to the next day.

History:
Programmer      Last change on    Details
__________      ____/__/__       __________________________________

******************************************************************************/

#define MAIN
#define MAINFL	-1

#include <stdio.h>
#include <reports.h>

#define CLOSEDBHEXIT(X)	{ close_dbh(); exit(X); }
#define SYSTEM		"CLOSING PROCESSES"
#define MOD_DATE	"23-JAN-90"

#ifdef ENGLISH
#define U_YES	'Y'
#define	L_YES	'y'
#else
#define U_YES	'O'
#define	L_YES	'o'
#endif

static Pa_rec	param_rec;

char e_mesg[80];	/* for storing error messages */
static	char file1[40], file2[40], buffer[10];
long	get_date(),date_plus() ;
static long sysdt;	/* system date */
static int	err,retval;

extern	int	errno;

main( argc, argv)
int argc;
char *argv[];
{
	long	cur_date;
	
	strncpy( SYS_NAME, SYSTEM, 50 );	/* Module name */
	strncpy( CHNG_DATE, MOD_DATE, 10 );	/* Last date of change */
	proc_switch( argc,argv,MAINFL );	/* process the switches */

	retval = GetUserClass(e_mesg) ;
	if(retval < 0 || (retval != ADMINISTRATOR && retval != SUPERUSER)) {
		if(retval == DBH_ERR) 
			printf("\n%s\n",e_mesg);
		else
#ifdef ENGLISH
			printf("\n\n\tACCESS DENIED\n");
#else
			printf("\n\n\tACCESS NIE\n");
#endif
		exit(-1);
	}
	retval = get_param( &param_rec, BROWSE, 1, e_mesg );
	if( retval!=1 ){
		printf("%s\n", e_mesg) ;
		exit(-1);
	}
	if( param_rec.pa_cur_period==0 ){
#ifdef ENGLISH
		printf("\n\tEnd of Day not allowed in period 0");
#else
		printf("\n\tFin de journee pas permise dans la periode 0");
#endif
		exit(1);
	}

	if(lock_file(PARAM) < 0) {
#ifdef ENGLISH
		printf("System is up on other terminals... Can't Proceed\n");
#else
		printf("Systeme est sur d'autres terminaux... Ne peut pas proceder\n");
#endif
		close_dbh() ;
		exit(-1);
	}

	sysdt = get_date();
	mkdate(sysdt, e_mesg);

#ifdef ENGLISH
	printf("\n\n\n\n\n\tEND OF THE DAY for the Date: %s\n\n\n", e_mesg);
	printf("\t\t1. Posts Recurring Entries\n");
	printf("\t\t2. Prints Transactions of the Day\n");
	printf("\t\t3. Prints Audit Report for the Day\n");
	printf("\t\t4. Rolls over the Date to next Day\n");
#else
	printf("\n\n\n\n\n\tFIN DE JOURNEE pour la date: %s\n\n\n", e_mesg);
	printf("\t1. Affiche les entrees repetitives quotidiennes, hebdo. & bihebdo.\n");
	printf("\t   Affiche entrees repetitives mens./trim./semi-annuelles/annuelles\n");
	printf("\t2. Imprime les transactions de la journee\n");
	printf("\t3. Imprime le rapport de verification pour la journee\n");
	printf("\t4. Avance la date a la prochaine journee\n");
#endif

#ifdef	JRNL
#ifdef ENGLISH
	printf("\t\t5. Backs up the Journal\n");
#else
	printf("\t5. Sauvegarde le journal\n");
#endif
#endif

#ifdef ENGLISH
	printf("\n\t\tProceed (Y/N)? ");
#else
	printf("\n\t\tProceder (O/N)? ");
#endif
	scanf("%s",e_mesg);
	if(e_mesg[0] != L_YES && e_mesg[0] != U_YES) {
		close_dbh();
		exit(-1);
	}

#ifdef ENGLISH
	printf("\n\nGL SUBSYSTEM\n");
	printf("\tPosting Recurring Entries\n");
#else
	printf("\n\nSOUS-SYSTEME G/L\n");
	printf("\tAffiche les entrees repetitives\n");
#endif
	if( PostRecurringEntries()<0 )
		CLOSEDBHEXIT(-1);

#ifdef ENGLISH
	printf("\tPrinting GL Transaction Listing's\n");
#else
	printf("\tImprime les listes de transactions G/L\n");
#endif
	if( (retval = eod_tr_list(99,e_mesg))<0 ) { /* EOD trans listing */
		if ( retval == UNDEF ) 
#ifdef ENGLISH
			printf("\t\tNo 99 GL Transactions To Print\n");
		else
			printf("\t\tError In GL Transaction Listing: %s\n",
					e_mesg);
#else
			printf("\t\tPas de transactions G/L 99 a imprimer\n");
		else
			printf("\t\tErreur dans les listes de transactions G/L: %s\n",
					e_mesg);
#endif
	}
	if(retval == NOERROR) 
#ifdef	i386
		system("lp -c -o nobanner eodtrlst99.dat");
#else
		system("lpr -b eodtrlst99.dat");
#endif

	if( (retval = eod_tr_list(97,e_mesg))<0 ) { /* EOD trans listing */
		if ( retval == UNDEF ) 
#ifdef ENGLISH
			printf("\t\tNo 97 GL Transactions To Print\n");
		else
			printf("\t\tError In GL Transaction Listing: %s\n",
					e_mesg);
#else
			printf("\t\tPas de transactions G/L 97 a imprimer\n");
		else
			printf("\t\tErreur dans les listes de transactions G/L: %s\n",
					e_mesg);
#endif
	}
	if(retval == NOERROR) 
#ifdef	i386
		system("lp -c -o nobanner eodtrlst97.dat");
#else
		system("lpr -b eodtrlst97.dat");
#endif

	if( (retval = eod_tr_list(98,e_mesg))<0 ) { /* EOD trans listing */
		if ( retval == UNDEF ) 
#ifdef ENGLISH
			printf("\t\tNo 98 GL Transactions To Print\n");
		else
			printf("\t\tError In GL Transaction Listing: %s\n",
					e_mesg);
#else
			printf("\t\tPas de transactions G/L 98 a imprimer\n");
		else
			printf("\t\tErreur dans les listes de transactions G/L: %s\n",
					e_mesg);
#endif
	}
	if(retval == NOERROR) 
#ifdef	i386
		system("lp -c -o nobanner eodtrlst98.dat");
#else
		system("lpr -b eodtrlst98.dat");
#endif


#ifndef	ORACLE
	if( param_rec.pa_aps[0]==U_YES ){
#ifdef ENGLISH
		printf("\n\nAP SUBSYSTEM\n");
#else
		printf("\n\nSOUS-SYSTEME C/P\n");
#endif
		if( ixrecreat( APINHDR )<0 )
#ifdef ENGLISH
		    printf("\t\tError In Recreating AP Invoice Header File\n");
#else
		    printf("\t\tErreur en recreant le dossier en-tete des factures C/P\n");
#endif
		if( ixrecreat( APINITEM )<0 )
#ifdef ENGLISH
		    printf("\t\tError In Recreating AP Invoice Item File\n");
#else
		    printf("\t\tErreur en recreant le dossier d'articles de facture C/P\n");
#endif
	}
#endif

	if( param_rec.pa_stores[0]==U_YES ){
#ifdef ENGLISH
		printf("\n\nINVENTORY SUBSYSTEM\n");
		printf("\tPrinting Stock Transaction Listing\n");
#else
		printf("\n\nSOUS-SYSTEME INVENTAIRE\n");
		printf("\tImprime la liste des transactions de stock\n");
#endif
		if( tranlist( 0 )<0 )	/* 0: non interactive */
#ifdef ENGLISH
			printf("\t\tError In Printing Stock Transactions\n");
#else
			printf("\t\tErreur dans l'impression des transactions de stock\n");
#endif
	}

	if( param_rec.pa_ars[0]==U_YES ){
#ifdef ENGLISH
		printf("\n\nAR SUBSYSTEM\n");
		printf("\tPrinting Sales Transaction Listing\n");
#else
		printf("\n\nSOUS-SYSTEME C/R\n");
		printf("\tImprime la liste des transactions de ventes\n");
#endif
		if( Trans_list( 1 )<0 )	/* 1: non interactive */
#ifdef ENGLISH
			printf("\t\tError In Printing Sales Transactions\n");
#else
			printf("\t\tErreur dans l'impression des transactions de ventes\n");
#endif
#ifdef ENGLISH
		printf("\tPrinting Sales Receipts Listing\n");
#else
		printf("\tImprime la liste des recus de vente\n");
#endif
		if( Rcpt_list( 1 )<0 )	/* 1: non interactive */
#ifdef ENGLISH
			printf("\t\tError In Printing Sales Receipts\n");
#else
			printf("\t\tErreur en imprimant les recus de vente\n");
#endif
	}

	if( param_rec.pa_fa[0]==U_YES ){
#ifdef ENGLISH
		printf("\n\nFA SUBSYSTEM\n");
		printf("\tPrinting FA Transfers Listing\n");
#else
		printf("\n\nSOUS-SYSTEME A.I.\n");
		printf("\tImprime la liste des transferts d'A.I.\n");
#endif
		if( farep2( 4 )<0 )	/* 4: non interactive */
#ifdef ENGLISH
			printf("\t\tError In Printing FA Transfers\n");
#else
			printf("\t\tErreur en imprimant les transferts d'A.I.\n");
#endif
	}

#ifdef ENGLISH
	printf("\n\nGENERAL HOUSEKEEPING\n");
	printf("\tPrinting Audit Report\n");
#else
	printf("\n\nNON-CATEGORIELS\n");
	printf("\tImprime le rapport de verification\n");
#endif

#ifndef	ORACLE
	if( lock_file( AUDIT )==ERROR ){
#ifdef ENGLISH
		printf("\t\tError in Locking Audit File. Not Printing List.\n");
#else
		printf("\t\tErreur en verrouillant le dossier de verification. N'imprime pas liste\n");
#endif
		CLOSEDBHEXIT(-1);
	}
#endif
	if( rep_aud()<0 )	/* print audit report and delete audit file */
		CLOSEDBHEXIT(-1);

	cur_date = param_rec.pa_date;

#ifdef ENGLISH
	printf("\n\nDATE ROLLOVER\n");
#else
	printf("\n\nAvancement de la date\n");
#endif
	if( DateRollOver()<0 )
		CLOSEDBHEXIT(-1);

#ifdef	JRNL
#ifdef ENGLISH
	printf("\n\nBACKING UP TODAY's JOURNAL\n");
#else
	printf("\n\nSAUVEGARDE LE JOURNAL D'AUJOURD'HUI\n");
#endif
	if( BackupJournal(cur_date)<0 )	/* Backup journal file "jrnl.date" */
		CLOSEDBHEXIT(-1);
#endif

#ifdef ENGLISH
	printf("\n\nEnd of Day Successful. Press RETURN ");
#else
	printf("\n\nFin de journee reussie. Appuyer sur RETURN ");
#endif
	fflush( stdin );
	getchar();

	CLOSEDBHEXIT(0);
}
static
DateRollOver()
{
#ifdef ENGLISH
	printf("\tRolling over current year's date\n");
#else
	printf("\tAvancement de la date de l'annee courante\n");
#endif
	if( GetParamUpdt()<0 )	/* Get the parameter file for date rollover */
		CLOSEDBHEXIT(-1);

	/* put next day's date in the current year */
	/* parameter file & write the record 	   */
	param_rec.pa_date = date_plus( param_rec.pa_date, 1 );
	
	retval = put_param( &param_rec, UPDATE, 1, e_mesg );
	if( retval==ERROR ){
		printf("%s\n", e_mesg) ;
		return(-1);
	}
	if( commit(e_mesg)<0 ){
		printf("%s\n", e_mesg) ;
		roll_back(e_mesg);
		return(-1);
	}

	/* put next day's date in the previous year */
	/* parameter file & write the record 	   */

#ifdef ENGLISH
	printf("\tRolling over previous year's date\n\n");
#else
	printf("\tAvancement de la date de l'annee precedente\n");
#endif
	SW9 = 1;
	close_dbh();

	/* get twice to activate prev_year */
	if( GetParamUpdt()<0 )	/* Get the parameter file for date rollover */
		CLOSEDBHEXIT(-1);

	param_rec.pa_date = date_plus( param_rec.pa_date, 1 );

	retval = put_param( &param_rec, UPDATE, 1, e_mesg );
	if( retval==ERROR ){
		printf("%s\n", e_mesg) ;
		return(-1);
	}
	if( commit(e_mesg)<0 ){
		printf("%s\n", e_mesg) ;
		roll_back(e_mesg);
		return(-1);
	}
	close_dbh();

	/* switch back to current year */
	SW9 = 0;

	return(0);
}

#ifdef	JRNL
static
BackupJournal(dt)
long	dt;
{
	form_f_name( JOURNAL, file1 );	
	sprintf( buffer,"%ld",dt );
	strcat( file1, buffer );

	form_f_name( BACK_UP, file2 );	/* directory */

#ifdef	MS_DOS
	sprintf(e_mesg, "copy %s %s", file1, file2 );
	system( e_mesg );
	unlink(file1) ;
#else
	sprintf(e_mesg, "mv %s %s", file1, file2 );
	system( e_mesg );
#endif
#ifdef ENGLISH
	printf("\tJournal file shifted to backup area\n");
#else
	printf("\tDossier du journal deplace a la memoire auxiliaire\n");
#endif

	/* Clear next day's JOURNAL file. Generally it won't be
	   there. It is safe to remove, because DBH always appends
	   commit() images to this file */
	form_f_name( JOURNAL, file1 );	
	sprintf( buffer,"%ld",date_plus(dt, 1));
	strcat( file1, buffer );
	creat(file1, CRMODE) ;

	return(0);
}
#endif

static
rep_aud()	/* Print audit report from audit file & delete the file */
{
#ifdef ENGLISH
	/* Print audit report ordered on time */
	if( AudRep('T', "F", "eodaudrep.dat")<0 )
		return(ERROR) ;
#else
	/* Print audit report ordered on time */
	if( AudRep('H', "F", "eodaudrep.dat")<0 )
		return(ERROR) ;
#endif

	if( close_file( AUDIT )<0 ){
#ifdef ENGLISH
		printf("Error in Closing Audit File\n");
#else
		printf("Erreur en fermant le dossier de verification\n");
#endif
		return(-1);
	}
	unlink_file(AUDIT) ;

#ifdef	i386
	system("lp -c -o nobanner eodaudrep.dat");
#else
	system("lpr -b eodaudrep.dat");
#endif
	return(0);
}

static
GetParamUpdt()	/* Get the parameter file and lock it for updating later */
{
	retval = get_param( &param_rec, UPDATE, 1, e_mesg );
	if( retval!=1 ){
		printf("%s\n", e_mesg) ;
		return(-1);
	}

	return(0) ;
}

/*********** 
	The following dummy routines are provided keeping in view that this 
	file is linked with other report producing files in other modules. 
	These function calls are made by routines in other modules.
***********/

GetCostcenRange(par1,par2)
short	*par1, *par2;
{
	return(0);
}
GetCodeRange(par1,par2)
char	*par1, *par2;
{
	return(0);
}
GetCNbrRange(par1,par2)
char	*par1, *par2;
{
	return(0);
}
GetResponse(par1)
char	*par1;
{
	return(0);
}
DisplayMessage(par1)
char	*par1;
{
	return(0);
}
GetItemidRange(par1,par2)
{
	return(0);
}
GetDateRange(par1,par2)
long	*par1, *par2;
{
	return(0);
}
GetFilename(par1)
char	*par1;
{
	return(0);
}
GetOutputon(par1)
char	*par1;
{
	return(0);
}
GetFundRange(par1,par2)
short	*par1, *par2;
{
	return(0);
}
GetTransDateRange(par1,par2)
long	*par1, *par2;
{
	return(0);
}
Confirm()
{
	return(0);
}
GetInvcRange(par1,par2)
long	*par1, *par2;
{
	return(0);
}
GetRcptRange(par1,par2)
long	*par1, *par2;
{
	return(0);
}
GetTypeRange(par1,par2)
char	*par1, *par2;
{
	return(0);
}
GetNbrCopies( par1 )
short	*par1;
{
	return(0);
}

GetPeriodRange( par1, par2)
short	*par1, *par2;
{
	return(0);
}
