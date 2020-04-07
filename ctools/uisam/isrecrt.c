/*
*	isrecrt.c
*
*	Program to recreat U_ISAM file. This program compresses the given
*	U_ISAM file by removing the deleted records.
*
*/

#include	<stdio.h>
#include	"isdef.h"
#include	"isflsys.h"
#include	"isnames.h"

#define	DEBUG

/*
#define	PERQ
#define	FIXED_RECLEN
#undef	DEBUG
*/

extern	int	iserror, errno ;

/*
char	*mktemp () ;
*/
char	*tmpnam() ;

static	int	fd1,fd2,keycount,keysarray[1000];
static	char	buffer[MAX_REC_LEN];
static	char	infile[FLNM_LEN] ;
static	char	outfile[FLNM_LEN] ;

#ifdef	FIXED_RECLEN

static	int	rec_len ;

#endif

extern	float	spl_ratio ;

static	int	datepart ;

main(argc, argv)
int	argc ;
char	*argv[] ;
{
	int	i ;

#ifdef	FIXED_RECLEN
	printf("\n\tFixed Record Length Version\n\n");
#else
	printf("\n\tVariable Record Length Version\n\n");
#endif
	datepart = -1 ;
	infile[0] = '\0' ;

	if(argc >= 2) {
		i = 999;
		if (strncmp(argv[1],"-d",2) == 0) {
			i = argv[1][2] - '0'  ;
			DT_TYPE = i ;
			datepart = i ;
			if(argc > 2)
				strcpy(infile, argv[2]);
		}
		else
			strcpy(infile, argv[1]);

		/* If more than 3 args or (3 args && 2nd arg is not date type)
		   or (invalid date type) */
		if((argc > 3 || argc == 3 || i != 999) &&
				i != DDMMYY && i != MMDDYY &&
				i != YYMMDD && i != DDMMYYYY &&
				i != MMDDYYYY && i != YYYYMMDD) {
		    printf("Usage: %s [-d{%d%d%d%d%d%d}] {File Name}\n",
			argv[0], MMDDYY, DDMMYY, YYMMDD, MMDDYYYY,
			DDMMYYYY, YYYYMMDD) ;
		    exit(-1);
		}
	}

	if(infile[0] == '\0') {
		printf ( "File Name: ");
		scanf ("%s", infile);
	}

	/* tampering with split */
	spl_ratio = 0.3 ;	/* Split ratio 70 30 instead of 50 50 */

	if  (openfile () < 0) {
		exit (1) ;
	}
	if (copy_valid_records() < 0) {	  /* copy  the  undeleted  records  */
		printf ("\n") ;
		close_files () ;
		del_files (outfile) ;
		exit (1) ;
	}

	close_files () ;
	mv_tmpfile();

	exit(1) ;
}


int openfile()
{
	char	ifnm[FLNM_LEN] ;
	int	i,j,k,parts = 0,l = 0;
	char	ptr[INXHSZ];
	int	*intptr;
	struct	keydat		*altky;
	struct	indxheader	*hdr;

	strcpy (ifnm, infile);
	strcat(ifnm,".IX");

	/*  read  header  of  index   file  */

	if ((fd1 = open (ifnm, RDONLY)) == 0)
	{
		printf("Error in opening ISAM Index file\n");
		return(-1);
	}

	if (read (fd1,ptr,INXHSZ) < INXHSZ )
	{
		printf("Error in reading ISAM Index file\n");
		return(-1);
	}

	hdr = (struct indxheader *)ptr;

	/*
	*  we  want  to  create  an  identically  structured  index  file  
	*  to  which  only  the   undeleted  records  in  the  original  file
	*  will  be  copied . Hence ,  we  get  the  header  information  from
	*  the  original  file  header
	*/

#ifdef	FIXED_RECLEN

	/* Get the record length from header + 1 */
	rec_len = hdr->preclength + STATUS_LEN ;

#endif

	keycount  =  hdr->paltkeys;		/* no  of  keys  */
	altky =  (struct keydat *)++hdr;
	for  (i = 0; i < keycount;i++)	/* key part information of each key */
	{
		parts = altky->pkeyparts;
		keysarray[l++] =  parts;
		intptr = (int *)++altky;
		for  (j = 0 ; j <parts ; j++) {
			/* If date is not set thru command line arguments */
			if(*intptr == DATE && datepart == -1) {
			    printf("DATE is Part of Index Key... Valid Types are..\n");
			    printf("\tMMDDYY   %d\n", MMDDYY);
			    printf("\tDDMMYY   %d\n", DDMMYY);
			    printf("\tYYDDMM   %d\n", YYMMDD);
			    printf("\tMMDDYYYY %d\n", MMDDYYYY);
			    printf("\tDDMMYYYY %d\n", DDMMYYYY);
			    printf("\tYYYYDDMM %d\n\n", YYYYMMDD) ;
			    do {
			    	printf("Enter DATE Type: ");
			    	scanf("%d",&datepart);
			    } while (datepart != DDMMYY && datepart != MMDDYY &&
			    	datepart != YYMMDD && datepart != DDMMYYYY &&
			    	datepart != MMDDYYYY && datepart != YYYYMMDD) ;

			    DT_TYPE = datepart ;
			}

			for (k = 0 ; k <= 3;k++)
				keysarray[l++] = *intptr++;
			altky  =  (struct keydat *)intptr;
		}
	}

	close(fd1)  ;

#ifdef	FIXED_RECLEN

	if ((fd1 = open (infile, RDONLY)) < 0)
	{
		printf("Error in opening ISAM Data file\n");
		return(-1);
	}

#else

	if ((fd1 = isopen(infile,RWR)) < 0)	/* open as ISAM file */
	{
		printf("Error in opening ISAM Index file\n");
#ifdef	DEBUG
		printf ("Isam error code - %d\n", iserror) ;
#endif
		return (-1) ;
	}

#endif

	/****
	strcpy (outfile, "IS") ;
	strcat (outfile, "XXXXXX");
	mktemp (outfile) ;
	****/
	tmpnam (outfile) ;

	/*  create  new  index  file  */

#ifdef	FIXED_RECLEN
	if ((fd2 = iscreat(outfile,RWR,keycount,keysarray,
				(rec_len - STATUS_LEN)))== ERROR)
#else
	if ((fd2 = iscreat(outfile,RWR,keycount,keysarray))== ERROR)
#endif
	{
		printf("Error in creating ISAM file\n");
#ifdef	DEBUG
		printf ("Isam error code - %d\n", iserror) ;
#endif
		return (-1) ;
	}
	printf ("File Opened .....\n") ;
	return(0);
}


copy_valid_records()
{
	int	thekey = 0,code, length,alt_array[100];
	char	*buff ;
	int	k_cnt = 0 ;

	printf ("Recreating ISAM file.") ;

	for (thekey = 0;thekey < keycount;thekey++)
		alt_array[thekey] = 1; 	/* write all alternate keys  */

#ifndef	FIXED_RECLEN

	buff = buffer ;
	/*  position   at  beginning  of   the  given  index  file */
	thekey = 0;
	if  ((code = isstart(fd1,buffer,thekey,ISFIRST)) == ERROR)
	{
		printf("Error in positioning in ISAM file\n");
#ifdef	DEBUG
		printf ("Isam error code - %d\n", iserror) ;
#endif
		return (-1) ;
	}
#else
	buff = buffer + 1 ;
#endif

	/* read all undeleted records in the file and write to temporary file */

	do {
#ifdef	FIXED_RECLEN
		length = read (fd1, buffer, rec_len) ;
		if (length != rec_len) break ;
		if(buffer[0] == SET_DEL) continue ;	/* Skip Deleted Recs */
		length-- ;
#else
		length = isreads(fd1, buffer, 0) ;
		if ((length == EFL) || (length == ERROR)) break ;
#endif

#ifdef	FIXED_RECLEN
		code = iswrite(fd2, buff, alt_array) ;
#else
		code = iswrite(fd2, buff, length, alt_array) ;
#endif
		if (code == ERROR)
		{
			printf("Error in Recreation\n");
#ifdef	DEBUG
			printf ("Isam error code - %d\n", iserror) ;
#endif
			return (-1) ;
		}
		if (((k_cnt++) % 10) == 0) {
			printf (".") ;
			fflush(stdout) ;
		}
	} while (1) ;

	printf ("\n") ;
	return(0) ;
}

close_files ()
{

#ifdef	FIXED_RECLEN
	close (fd1) ;
#else
	isclose (fd1) ;
#endif
	isclose (fd2) ;

}

del_files (file_name)
char	*file_name ;
{
	char	tmp_file [FLNM_LEN+3] ;

	unlink (file_name) ;
	strcpy (tmp_file, file_name) ;
	strcat (tmp_file, ".IX") ;
	unlink (tmp_file) ;
}

mv_tmpfile()
{
	char	buff[2*FLNM_LEN+10] ;

#ifdef	MS_DOS

	del_files (infile) ;
	/*
	*  "ren" is replaced by "copy", because "ren" doesn't work across
	*  directories
	*/
	sprintf (buff, "copy %s %s", outfile, infile) ;
#else
	sprintf (buff, "mv %s %s", outfile, infile) ;
#endif

	system (buff) ;

#ifdef	MS_DOS

	/*
	*  "ren" is replaced by "copy", because "ren" doesn't work across
	*  directories
	*/
	sprintf (buff, "copy %s.IX %s.IX", outfile, infile) ;
#else
	sprintf (buff, "mv %s.IX %s.IX", outfile, infile) ;
#endif

	system (buff) ;

#ifdef	MS_DOS
	del_files (outfile) ;
#endif
}
