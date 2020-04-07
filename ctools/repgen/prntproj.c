/*
*	file:	prntproj.c
*	Print the contents of prooject.RPS file
*/

#include <stdio.h>
#include "rep.h"
#include "struct.h"
#include "repname.h"

static	char	buff[STRBUFSIZE] ;
static	int	sno ;

main()
{
	char	projname[40] ;
	int	fd, i ;

	for( ; ; ) {
		printf("Project Name(*-Exit): ");
		scanf("%s",projname);

		if(projname[0] == '*') break ;

		strcat(projname, STRFILE) ;

		fd = open(projname, RDMODE) ;
		if(fd < 0) {
			printf("%s Open Error\n", projname);
			continue ;
		}
		sno = 0 ;
		for( i = 0 ; ; i++ ) {
			if(read(fd, buff, STRBUFSIZE) < STRBUFSIZE) break ;
			print_defs(i) ;
		}
		close(fd) ;
	}
	exit(0);
}

print_defs(rec_no)
int	rec_no ;
{
	int	i ;
	INHDR	*hdr ;
	FLDINFO	*t_ptr ;

	t_ptr = (FLDINFO *) (buff + sizeof(INHDR)) ;
	hdr = (INHDR *)buff ;

	fprintf(stderr,
	    "\nRec#: %d\tRecord Name: %s\tNo Of Fields: %d\tReclen: %d\n\n",
	    rec_no+1, hdr->stname, hdr->stflds, hdr->stsiz );
	for( i = 0 ; i < hdr->stflds ; t_ptr++, i++ )
	    fprintf(stderr,
	      "%d\t%d\tFld Name: %-10.10s\tTyp: %c\tOffset: %d\tLen: %d\n",
	      ++sno, i+1, t_ptr->fldname, t_ptr->typf, t_ptr->posf, t_ptr->lenf) ;
}

