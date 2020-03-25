/*
*	superusr.c
*
*	Programme to create super user for given district.
*/

#define	MAIN

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define SYSTEM		"UTILITIES"
#define MOD_DATE	"01-APR-90"

main(argc,argv)
int	argc ;
char	*argv[] ;
{
	UP_rec	up_rec;
	int	i, status ;
	char	e_mesg[80];

	/* Set The environment */
	strncpy(SYS_NAME, SYSTEM, 50);
	strncpy(CHNG_DATE, MOD_DATE, 10);
	proc_switch(argc, argv, -1) ;

	if(dist_no[0] == '\0')
		set_dist() ;

	/* Fill up a Super user Profile */
	strcpy( up_rec.u_id, "nfld" );
	strcpy( up_rec.u_name, "SUPER USER" );
	strcpy( up_rec.u_passwd, "\0" );
	strcpy( up_rec.u_trml, "\0" );
	strcpy( up_rec.u_class, "S" );
	
	for( i=0; i<TOTAL_FILES; i++ )
		up_rec.u_access[i] =
			(char)( DFLT_CHAR | P_DEL | UPDATE | ADD | BROWSE );

	up_rec.u_access[TOTAL_FILES] = '\0' ;

	status = GetSecurityStatus();

	/* Write the record */
	SetSecurityStatus(0);
	i = put_userprof( &up_rec,ADD,e_mesg );
	if( i < 0 )
		printf("\nError in adding SuperUser Profile\n%s\n",e_mesg);
	else if( commit(e_mesg) < 0)
		printf("\nError in adding SuperUser Profile\n%s\n",e_mesg);

	SetSecurityStatus(status);

	close_dbh() ;
	exit(1) ;
}
/*-------------------------END OF FILE------------------------------*/

