#define	MAIN

#define SYSTEM		"UTILITIES"
#define MOD_DATE	"23-JAN-90"

#include <stdio.h>
#include <bfs_defs.h>
#include <fld_defs.h>

Fld_hdr	hdr ;
Field	*fields = (Field *)NULL ;
char	e_mesg[80] ;

main(argc, argv)
int	argc ;
char	*argv[] ;
{
	char	name[40] ;

	strncpy(SYS_NAME, SYSTEM, 50);
	strncpy(CHNG_DATE, MOD_DATE, 10);
	proc_switch(argc, argv, -1) ;

	for( ; ; ) {
		printf("DEF File Name(*-Exit): ");
		scanf("%s",name);

		if(name[0] == '*') break ;

		if(GetFields(name, &hdr, &fields, e_mesg) == ERROR) {
			printf("%s\n",e_mesg);
			continue ;
		}
		print_defs() ;
		if(fields != NULL)
			free((char*)fields) ;
	}
	exit(0);
}

print_defs()
{
	int	i ;
	Field	*t_ptr ;

	t_ptr = fields ;

	fprintf(stderr, "File Name: %s   No Of Fields: %d   Reclen: %d\n\n",
		hdr.filenm, hdr.no_fields, hdr.reclen );
	for( i = 0 ; i < hdr.no_fields ; t_ptr++, i++ )
		fprintf(stderr, "\tFld Name: %-15.15s Typ: %c Offset: %-3d Len: %-3d Format: %s\n",
			t_ptr->name, t_ptr->type, t_ptr->offset, t_ptr->len,
			t_ptr->format );
}

