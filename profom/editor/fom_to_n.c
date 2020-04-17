
/*
*	Converts given [XXXXX].fom to NFM
*/

/*
*	Compilation:	cc fom_to_n.c fomtfr.c fommsc.c -o fom_to_n
*/

#include <stdio.h>
#include "cfomfrm.h"
#include "cfomstrc.h"

#define STSZ 8192

struct frmhdr hdrrc;
struct frmfld field;

char *string, str[STSZ], temp[20];

struct stat_rec statrec;

main(argc,argv)
int	argc;
char	*argv[];
{
/**
	char	scrname[20] ;
**/
	int	ret ;


/**
	printf(" Screen Name :");
	scanf("%s", scrname) ;

	ret = frmfom(scrname) ;
**/
	ret = frmfom(argv[1]) ;
	printf("ret:%d\n", ret);

	exit(0);
}

