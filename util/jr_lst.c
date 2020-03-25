/*
*	jr_lst.c
*
*	Program to print journal file
*/

#define	MAIN 

#define SYSTEM		"UTILITIES"
#define MOD_DATE	"23-JAN-90"

#include <stdio.h>
#include <bfs_defs.h>
#include <journal.h>

static	Area_hdr	commarea_hdr ;
static	int	jrnl_fd ;		/* File fd for journalling */
static	int	item ;
static	char	*big_buff = NULL ;

main(argc, argv)
int	argc ;
char	*argv[];
{
	char	jr_name[50] ;
	long	date, get_date() ;
	char	ans[12] ;
	int	i, j ;

	/* Set The environment */
	strncpy(SYS_NAME, SYSTEM, 50);
	strncpy(CHNG_DATE, MOD_DATE, 10);
	proc_switch(argc, argv, -1) ;

	if (dist_no[0] == '\0' ) 
		set_dist() ;

	/* Find out Max Reclen */
	j = 0 ;
	for(i = 0 ; i < TOTAL_FILES ; i++)
		if(j < getreclen(i)) j = getreclen(i) ;

	big_buff = malloc((unsigned) j) ;
	if(big_buff == NULL) {
		printf("Memory Allocation Error... size: %d\n", j);
		exit(-1);
	}

	form_f_name(JOURNAL,jr_name) ;

	mkdate( (date=get_date()) , ans );
	printf("Get Journal For Date: %s (Y/N)? ", ans) ;
	scanf("%s", ans) ;

	if ( ans[0] != 'y' && ans[0] != 'Y' )  {
		printf("Type Date in YYYYMMDD format: ") ;
		scanf("%ld", &date) ;
	}
	
	sprintf(&jr_name[strlen(jr_name)], "%ld", date) ;
	if ((jrnl_fd=open(jr_name, RDMODE)) < 0 ) {
		printf("Journal File \"%s\" Open ERROR...   errno: %d\n",
			jr_name, errno);
		exit(-1);
	}

	ReadJrnl() ;

	close(jrnl_fd);
	if(big_buff != NULL)
		free(big_buff) ;
	exit(1);
}


 int
ReadJrnl()
{
	int	i ;
	int	file_no ;

	for( ; ; ) {
	    if(ReadArea() < 0) return(0) ;

	    for(i = 0, item = 0  ; i < commarea_hdr.rec_count; i++) {

		if((file_no=ReadHdrAndRec()) < 0) return(0) ;

		/****
		lseek(jrnl_fd, (long)getreclen(file_no), 1) ;
		*****/
	    }
	}
}
		

/*
*	The file layout for commit area is.__
* 		<-Area_hdr-> { <-Rec_hdr + Record + Hole-> }	
*/		

 int
ReadArea()
{
	if (read(jrnl_fd, (char *)&commarea_hdr, sizeof(Area_hdr) )
				< sizeof(Area_hdr) )
		return(ERROR);

	fprintf(stderr,"\nUser: %-10.10s  Program: %-10.10s Terminal: %-3.3s  ",
		commarea_hdr.usrnm, commarea_hdr.program, commarea_hdr.termnm) ;
	fprintf(stderr,"Time: %d  Size: %-5d\n", commarea_hdr.c_time,
		commarea_hdr.size);
	if(commarea_hdr.rec_count >= 0)
		fprintf(stderr,"\tCOMMITED Records: %hd\n\n",
			commarea_hdr.rec_count) ;
	else {
		commarea_hdr.rec_count *= -1 ;
        	fprintf(stderr,"\tABORTED Records: %hd\n\n",
			commarea_hdr.rec_count) ;
	}
	
	return(0);
}


 int 
ReadHdrAndRec()
{
	char	f_name[50], key[50] ;
	char	op_str[10] ;
	int	file_no , op_code ;
	char	hdr[4] ;
	unsigned short	rec_no ;

	if (read(jrnl_fd, hdr, 4) < 4 ) {
		printf("Error in Reading Record Header\n");
		return(ERROR);
	}
	
	file_no = hdr[0] ;
	op_code = hdr[1] ;
	rec_no  = (hdr[2] << 8) | hdr[3] ;

	if ( getflnm( file_no, f_name) < 0 ) {
		printf("\tInvalid File no: %d encountered\n", file_no);
		return(-1) ;
	}

	if ( read(jrnl_fd, big_buff, getreclen(file_no)) < 
				getreclen(file_no)) {
		printf("Premature Termination of Journal file\n");
		return(ERROR) ;
	}
 
	form_key(big_buff,file_no,rec_no,key);

	switch(op_code) {
		case ADD	:
			strcpy(op_str, "ADD");
			break ;
		case UPDATE	:
			strcpy(op_str, "UPDATE");
			break ;
		case P_DEL	:
			strcpy(op_str, "DEL");
			break ;
		case NOOP 	:
			strcpy(op_str, "NOOP");
			break ;
		default 	:
			printf("Invalid Mode: %d\n", op_code) ;
			return(-1) ;
		}
		
	if(item == 0)
	    fprintf(stderr,"  Item#\tFile Name \tOperation\tRecord Key               \tRecLen\n\n");

	fprintf(stderr,"   %-4d\t%-10.10s\t%-9.9s\t%-25.25s\t%hd\n", ++item,
		f_name, op_str, key, getreclen(file_no)) ;

	return(file_no) ;
}

