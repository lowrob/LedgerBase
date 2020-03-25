/*
*	recovery.c
*
*	Recreate the Database from AIJ Files ..
*
*/

#define	MAIN 

#define SYSTEM		"UTILITIES"
#define MOD_DATE	"23-JAN-90"

#include <stdio.h>
#include <bfs_defs.h>
#include <filein.h>
#include <journal.h>
#include <dberror.h>
#include <isnames.h>
#ifdef	MS_DOS
#include <dos.h>
#else			/* UNIX or XENIX */
#include <sys/types.h>
#include <sys/dir.h>
#endif

typedef	struct	{

	long	JrnlDate ;		/* Date in (YY)YYMMDD format */
	char	FileName[50] ;		/* Jrnl File Name */

}	JrnlFiles ;

static	JrnlFiles	**JrnlList = NULL ;
static	int		NoOfFiles = 0 ;

static	int	option ;
static	long	strt_date, end_date, cur_date ;

extern 	char	*commit_area ;		/* start of commit area */
extern	int	wr_jrnl ;		/* Journal file write enable flag */

char	c_mesg[80] ;

long	date_plus() ;
char	*malloc(), *realloc() ;
void	qsort(), free() ;

main(argc, argv)
int	argc ;
char	*argv[] ;
{
	int	i, retval ;

	strncpy(SYS_NAME, SYSTEM, 50);
	strncpy(CHNG_DATE, MOD_DATE, 10);

	proc_switch(argc, argv, -1) ;
	if(SW8 || SW9) {
		printf("ERROR: Wrong SWITCHES...\n") ;
		exit(-1) ;
	}

	if(dist_no[0] == '\0')
		set_dist() ;

	retval = GetUserClass(c_mesg);
	if(retval < 0 || (retval != ADMINISTRATOR && retval != SUPERUSER)) {
		if(retval == DBH_ERR) 
			printf("\n%s\n",c_mesg);
		else
			printf("\n\n\n\tACCESS DENIED\n");
		close_dbh() ;
		exit(-1);
	}

	cur_date = get_date() ;

	printf("\n\n\n\tDATABASE RECOVERY UTILITY.....District: %s\n\n\n",
		dist_no) ;
	printf("\t\t0. QUIT\n\n");
	printf("\t\t1. Recreate using Current Data Files as initial files\n\n");
	printf("\t\t2. Recreate using 'backup' Data Files as initial files\n\n");
	printf("\t\t3. Purge Journal files\n\n");
	do {
		printf("\t\tSelect: ");
		scanf("%d", &option) ;
	} while ( option < 0 || option > 3) ;

	if(option == 0) {
		close_dbh() ;
		exit(-1);
	}

	if(init_dbh() < 0) {		/* read keydesc.id to refer files */
		printf("DBH Initialization ERROR.. DBERROR: %d\n",dberror);
		close_dbh() ;
		exit(-1) ;
	}

	/* Set the start JRNL Date to initial DB date. */
	if(option == 2) {	/* Get the date from 'backup' parm record */
		close_file(PARAM) ;
		SW8 = 1 ;
	}
	strt_date = conv_date(get_date(), DATE_TYPE,6) ;
	if(option == 2) {
		close_file(PARAM) ;
		SW8 = 0 ;
	}

	if(option == 1 || option == 2)
		retval = RecoveryOptions() ;
	else {
		printf("\n\n\tPurges Journals Before %ld\n\n", strt_date) ;
		retval = NOERROR ;	/* For Next statement */
	}

	if(retval == NOERROR) {
		printf("\n\tContinue (Y/N)? ");
		scanf("%s", c_mesg) ;
	}

	if ( retval == ERROR || (c_mesg[0] != 'y' && c_mesg[0] != 'Y' )) {
		close_dbh() ;
		exit(-1) ;
	}

	if(option == 1 || option == 2) {
		SetSecurityStatus(0) ;	/* Set Security OFF */

		retval = GetJrnlFiles() ;
		if(retval == NOERROR) {

			SetBaseDataBase() ;

			retval = RecreateDataBase() ;
		}
	}
	else
		retval = PurgeJournals() ;

	for(i = 0 ; i < NoOfFiles ; i++)
		if(JrnlList[i] != NULL) free((char*)JrnlList[i]) ;
	if(JrnlList != NULL)
		free((char*)JrnlList) ;

	close_dbh() ;

	if(retval == NOERROR) {
	    if(option == 3)
		printf("\n\n\n**** The Journal Files are Purged ***\n\n");
	    else {
		printf("\n\n\n**** The Data Files are recreated ***\n\n");
		printf("*** Please Check Data & continue the operations ***\n");
	    }
	    exit(1) ;
	}
	else {
		if(option == 3)
			printf("ERROR: Unsuccessful Purge\n");
		else
			printf("ERROR: Unsuccessful RECOVERY\n");
		exit(-1) ;
	}
}

/*---------------------------------------------------------------------*/
/* DB Recovery Options */

static	int
RecoveryOptions()
{
	char	t_str[50] ;

	if ( commit_area == NULL ) {
		printf("ERROR: Recovery is not Possible.. Journalling is DISABLED\n");
		return(ERROR) ;
	}

	printf("\n\n");
	for( ; ; ) {
		printf("Applies Journals from Date        : %ld\n",
			conv_date(strt_date, 6, DATE_TYPE)) ;
		printf("Apply Journals upto Date");
		switch(DATE_TYPE){
		case DDMMYY:
			printf("(DDMMYY)  : ");
			break;
		case MMDDYY:
			printf("(MMDDYY)  : ");
			break;
		case YYMMDD:
			printf("(YYMMDD)  : ");
			break;
		case DDMMYYYY:
			printf("(DDMMYYYY): ");
			break;
		case MMDDYYYY:
			printf("(MMDDYYYY): ");
			break;
		case YYYYMMDD:
			printf("(YYYYMMDD): ");
			break;
		}
		scanf("%ld", &end_date ) ;
		end_date = conv_date(end_date, DATE_TYPE, 6) ;
		if(end_date < strt_date) continue ;

		if(option == 2 && cur_date > 0 &&
				end_date > conv_date(cur_date, DATE_TYPE,6)) {
			printf("\nEnd Date can't be after %ld\n", cur_date);
			continue ;
		}

		/* If the journal exists either in current or backup directory,
		   then end_date is valid date */
		sprintf(c_mesg, "%s%ld", JOURNAL, end_date) ;
		form_f_name(c_mesg, t_str) ;
		if(access(t_str, 0) < 0) {
			/* Check in backup directory */
			SW8 = 1 ;
			sprintf(c_mesg, "%s%ld", JOURNAL, end_date) ;
			form_f_name(c_mesg, t_str) ;
			SW8 = 0 ;
			if(access(t_str, 0) < 0) {
				printf("Invalid End date\n");
				continue ;
			}
		}
		break ;
	}

	if(end_date < conv_date(cur_date, DATE_TYPE, 6)) {
		sprintf(c_mesg, "%s%ld", JOURNAL, cur_date) ;
		form_f_name(c_mesg, t_str) ;
		if(access(t_str, 0) >= 0) {
			printf("\n\tWARNING: %s Journal will be DELETED\n\n",
				t_str);
		}
	}
	if(option == 2) {
		printf("\nThis Option..\n");
		printf("\t1. Removes Current Data Files\n");
		printf("\t2. Recreates Database from 'backup' data files\n");
	}

	return(NOERROR) ;
}	/* RecoveryOptions() */
/*----------------------------------------------------------*/
/*
*	When option is 1 Journal files will be applied on existing DataBase.
*	Otherwise DataBase wil be copied from 'backup' directory, and on
*	that journal files will be applied.
*/
static	int
SetBaseDataBase()
{
	int	i ;
	char	data_path[50] ;
	char	backup_path[50] ;
	char	t_str[50] ;

	if(option == 1) return(NOERROR) ;

	/*
	*  Copy the Files form backup directory to current directory
	*  one by one
	*/

	printf("\n\n");
	printf("Copying files from 'backup' directory as Initial DATABASE");
	printf("\n\n");

	SW8 = 1 ;			/* Change path to backup path */
	form_f_name("", backup_path) ;	/* Data Path for 'backup' dir */

	SW8 = 0 ;			/* Set path to current dir */
	form_f_name("", data_path);	/* Data Path to Current Dir */

	for(i = 0 ; i < TOTAL_FILES ; i++) {
		unlink_file(i) ;	/* Unlink Existing file */
		getflnm(i, t_str) ;
#ifdef	MS_DOS
		sprintf(c_mesg,"copy %s%s*.* %s", backup_path,
			t_str, data_path);
#else
		sprintf(c_mesg,"cp %s%s* %s", backup_path,
			t_str, data_path);
#endif
		printf( "\tCommand: %s\n",  c_mesg) ;
		system(c_mesg);
	}

	return(NOERROR) ;
}	/* SetBaseDataBase() */
/*----------------------------------------------------------*/
/* 
*	Get one by one Journal File Names from backup directory.
*	When there is a initial DB(i.e. get_date() returns non 0)s
*	pickup only journals which are of later dates to current
*	DB's date. Otherwise pickup all the journal files(First
*	journal file will be having 0 date).
*/
static	int
GetJrnlFiles()
{
	long	t_date ;
	int	i, fd ;
	char	t_str[50] ;
#ifdef	MS_DOS
	struct	find_t	next_file ;
#else			/* UNIX or XENIX */
	struct	direct	dir_entry ;		/* Directory entry structure */
#endif
	int	CompDates() ;

#ifdef	MS_DOS
	/* Make the serach file string */
	SW8 = 1 ;			/* Change path to backup path */
	form_f_name("", t_str) ;	/* Data Path for 'backup' dir */
	strcat(t_str, JOURNAL);
	strcat(t_str, "*");
	SW8 = 0 ;
#else		/* UNIX or XENIX */
	/* Open directory file to get all the jrnl files */
	SW8 = 0 ;				/* Set path to current dir */
	form_f_name(BACK_UP, t_str);	/* Directory name of 'backup' */
	if ( (fd = open(t_str, RDMODE)) < 0 ) {
		printf("ERROR in opening %s directory...  errno: %d\n",errno);
		printf("No Journal Files Could Be Found..\n");
		return(ERROR) ;
	}
#endif

	JrnlList = NULL ;
	NoOfFiles = 0 ;

	SW8 = 1 ;	/* to Prefix the path with 'backup' to journal files */
	for( i = 0 ; ; i++ ) {
		/* get the next journal file */
#ifdef	MS_DOS
		if( i == 0)
		    retval = _dos_findfirst(t_str, _A_NORMAL, &next_file) ;
		else
		    retval = _dos_findnext(&next_file) ;
		if(retval != 0) break ;
		strcpy(t_str, next_file.name) ;
#else
		if ( read(fd, (char*)&dir_entry, sizeof(struct direct)) <
						sizeof(struct direct) ) break ;
		if(dir_entry.d_ino == 0) continue ;
		if(strncmp(dir_entry.d_name,JOURNAL,strlen(JOURNAL))) continue ;
		strcpy(t_str, dir_entry.d_name) ;
#endif
		/* Get the date of the file */
		sscanf(t_str+strlen(JOURNAL), "%ld", &t_date) ;
		t_date = conv_date(t_date,DATE_TYPE,6) ;

		if(strt_date > 0)
			if(t_date < strt_date) continue ;
		if(t_date > end_date) continue ;

		if(AllocateMemory(NoOfFiles) < 0) return(ERROR) ;

		JrnlList[NoOfFiles]->JrnlDate = t_date ;
		form_f_name(t_str, JrnlList[NoOfFiles]->FileName);
		NoOfFiles++ ;
	}
	SW8 = 0 ;
	/* If the start date is 0 copy the start journal i.e 0 date's */
	if(strt_date == 0) {
		sprintf(c_mesg,"%s%ld", JOURNAL, strt_date) ;
		form_f_name(c_mesg, t_str);
		if(access(t_str, 0) >= 0) {	/* If File Exits */
			if(AllocateMemory(NoOfFiles) < 0) return(ERROR) ;
			JrnlList[NoOfFiles]->JrnlDate = 0 ;
			strcpy(JrnlList[NoOfFiles]->FileName, t_str);
			NoOfFiles++ ;
		}
	}

	/*
	*  Copy the latest journal form current directory
	*/

	/*
	*  cur_date and start_date will be same in 2 situations:
	*	- Option 1, i.e data restored from tape
	*	- Option 2, Recovery is being done on the same day of
	*	  month end process
	*
	*  In the first situation, cur_date journal shouldn't be copied,
	*  complete journal of that day will be available in 'backup'
	*  directory, and that is already copied to list.
	*/

	sprintf(c_mesg,"%s%ld", JOURNAL, conv_date(end_date,6,DATE_TYPE)) ;
	form_f_name(c_mesg, t_str);
	t_date = conv_date(cur_date, DATE_TYPE, 6) ;
	if((option == 1 && t_date > strt_date) ||
				(option == 2 && end_date == t_date)) {
	    if(access(t_str, 0) >= 0) {	/* If File Exits */
		if(option == 2 && strt_date == t_date) {
			printf("Apply current dates (%ld) journal (Y/N)? ",
					cur_date );
			scanf("%s", c_mesg) ;
		}
		else
			c_mesg[0] = 'y' ;

		if(c_mesg[0] == 'y' || c_mesg[0] == 'Y') {
			if(AllocateMemory(NoOfFiles) < 0) return(ERROR) ;
			JrnlList[NoOfFiles]->JrnlDate = t_date ;
			strcpy(JrnlList[NoOfFiles]->FileName, t_str);
			NoOfFiles++ ;
		}
		else
			unlink(t_str) ;		/* As explained below */
	    }
	}
	else if( option == 2) {
		/* Remove the journal from current data directory. Otherwise
		   DBH always tries to append commit_area for the existing
		   journal file of that date. Now the recovery is being done
		   upto the earlier dates, rather than upto current date. */

		unlink(t_str) ;
	}

	if(NoOfFiles)
		qsort((char*)JrnlList,NoOfFiles,sizeof(JrnlFiles*),CompDates) ;

#ifndef	MS_DOS		/* UNIX or XENIX */
	close(fd) ;
#endif
	return(NOERROR) ;
}	/* GetJrnlFiles() */
/*----------------------------------------------------------*/
static	int
AllocateMemory(file_no)
int	file_no ;
{
	if(JrnlList == NULL)
		JrnlList = (JrnlFiles **)malloc(sizeof(JrnlFiles *)) ;
	else
		JrnlList = (JrnlFiles **)realloc((char*)JrnlList,
			(unsigned)(file_no+1) * sizeof(JrnlFiles *)) ;

	if(JrnlList == NULL) {
		printf("MEMORY Allocation ERROR\n");
		return(ERROR) ;
	}

	JrnlList[file_no] = (JrnlFiles *)malloc(sizeof(JrnlFiles)) ;

	if(JrnlList[file_no] == NULL) {
		printf("MEMORY Allocation ERROR\n");
		return(ERROR) ;
	}

	return(0) ;
}	/* AllocateMemory() */
/*----------------------------------------------------------*/
/* Comparison routine for qsort() */
static	int
CompDates(jl1, jl2)
JrnlFiles	**jl1, **jl2 ;
{
	return( (*jl1)->JrnlDate - (*jl2)->JrnlDate ) ;
}	/* CompDates() */
/*----------------------------------------------------------*/
static	int
RecreateDataBase()
{
	int	i ;

	printf("\n\n");
	wr_jrnl = 0 ;		/* Disable writing to journal file .. */
	for(i = 0 ; i < NoOfFiles ; i++)
		if(ApplyJournal(JrnlList[i]->FileName, (i+1)) < 0)return(ERROR);

	return(NOERROR) ;
}	/* RecreateDataBase() */
/*----------------------------------------------------------*/
static	int
ApplyJournal(jrnlfile, file_no)
char	*jrnlfile ;
int	file_no ;
{
	int		jrnl_fd, t_fd ;
	Area_hdr	*temp ;
	int		code, count ;
	long		newsize, oldsize ;
	char		t_str[50] ;

	printf("Applying Journal File: %s\n", jrnlfile ) ;
	printf("Type RETURN to continue ");
	fflush(stdout) ;
	read(0, c_mesg, 50) ;

	if ( (jrnl_fd = open(jrnlfile, RDMODE)) < 0 ) {
		printf("Journal File: %s Open Error\n", jrnlfile);
		printf("Recreation of Data Base in not Done\n");
		return(ERROR);
	}

	/*
	*  For option 1, assumption is files are restored from tape, all 
	*  journals after that backup should be applied. This tape might have
	*  been backed up at any time in the day. So restoring this tape,
	*  restores that day's journal file at that point in the current
	*  directory. But the complete journal file of that day, is copied to
	*  'backup' directory, by that day's end-of-day process. When that
	*  day's journal file is being applied, apply only commits after the
	*  backup. This is done by reading commits after the size of restored
	*  journal file only.
	*/

	if(strt_date > 0 && file_no == 1 && option == 1) {
		sprintf(c_mesg, "%s%ld", JOURNAL, strt_date) ;
		form_f_name(c_mesg, t_str) ;
		t_fd = open(t_str, RDMODE) ;
		if(t_fd >= 0) {
			lseek(jrnl_fd, lseek(t_fd, 0L, 2), 0) ;
			close(t_fd) ;
		}
	}

	temp = (Area_hdr *)commit_area ;

	for(;;) {
		if ( read(jrnl_fd, (char *)temp, sizeof(Area_hdr)) <
					sizeof(Area_hdr) )  break ;

		/* temporarily initialize size to zero. If the commit area has
		   to be reallocated in re_alloc() of DBH, after reallocation
		   it copies records from old area to new area, which we have
		   not read yet */

		newsize = sizeof(Area_hdr) + temp->size ;
		oldsize = temp->size ;
		temp->size = 0 ;
		if(re_alloc(COMMIT_AREA, newsize) < 0) {
			printf("ERROR in Memory allocation... Dberror: %d\n",
				dberror) ;
			close(jrnl_fd) ;
			return(ERROR) ;
		}
		temp = (Area_hdr *)commit_area ;
		temp->size = oldsize ;

		if (read(jrnl_fd, commit_area+sizeof(Area_hdr),
				(unsigned)temp->size) < (unsigned)temp->size) {
			printf("Incomplete Last Journal Area..\n");
			close(jrnl_fd) ;
			return(ERROR) ;
		}
		/* Commit() initializes commit_hdr. Before committing, store
		  'rec_count' in temporary variable */
		if ( (count = temp->rec_count) > 0 ) {
			code = commit(c_mesg) ;
			if ( code < 0 ) {
				printf("Commit Error: %s\n", c_mesg) ;
				close(jrnl_fd) ;
				return(ERROR);
			}
			printf("\tRecords Committed: %d\n", count);
		}
	}

	close(jrnl_fd) ;
	return(NOERROR);
}
/*--------------------------------------------------------------*/
/* 
*	Purge Journals
*
*	Get one by one Journal File Names from backup directory.
*	Purge only journals which are of earlier dates to Disk
*	Backed up DB's date.
*/

static	int
PurgeJournals()
{
	long	t_date ;
	int	i, fd ;
	char	t_str[50] ;
#ifdef	MS_DOS
	struct	find_t	next_file ;
#else			/* UNIX or XENIX */
	struct	direct	dir_entry ;		/* Directory entry structure */
#endif

#ifdef	MS_DOS
	/* Make the serach file string */
	SW8 = 1 ;			/* Change path to backup path */
	form_f_name("", t_str) ;	/* Data Path for 'backup' dir */
	strcat(t_str, JOURNAL);
	strcat(t_str, "*");
	SW8 = 0 ;
#else		/* UNIX or XENIX */
	/* Open directory file to get all the jrnl files */
	SW8 = 0 ;				/* Set path to current dir */
	form_f_name(BACK_UP, t_str);	/* Directory name of 'backup' */
	if ( (fd = open(t_str, RDMODE)) < 0 ) {
		printf("ERROR in opening %s directory...  errno: %d\n",errno);
		printf("No Journal Files Could Be Found..\n");
		return(ERROR) ;
	}
#endif

	printf("Purging Journal Files\n\n");

	SW8 = 1 ;	/* to Prefix the path with 'backup' to journal files */
	for( i = 0 ; ; i++ ) {
		/* get the next journal file */
#ifdef	MS_DOS
		if( i == 0)
		    retval = _dos_findfirst(t_str, _A_NORMAL, &next_file) ;
		else
		    retval = _dos_findnext(&next_file) ;
		if(retval != 0) break ;
		strcpy(t_str, next_file.name) ;
#else
		if ( read(fd, (char*)&dir_entry, sizeof(struct direct)) <
						sizeof(struct direct) ) break ;
		if(dir_entry.d_ino == 0) continue ;
		if(strncmp(dir_entry.d_name,JOURNAL,strlen(JOURNAL))) continue ;
		strcpy(t_str, dir_entry.d_name) ;
#endif
		/* Get the date of the file */
		sscanf(t_str+strlen(JOURNAL), "%ld", &t_date) ;
		t_date = conv_date(t_date,DATE_TYPE,6) ;

		/* skip the files which are later than backup data date */
		if(t_date >= strt_date) continue ;

		form_f_name(t_str, c_mesg) ;

		unlink(c_mesg) ;		/* As explained below */
	}
	SW8 = 0 ;

#ifndef	MS_DOS		/* UNIX or XENIX */
	close(fd) ;
#endif
	return(NOERROR) ;
}	/* PurgeJournals() */
/*--------------------------END OF FILE------------------------------*/
