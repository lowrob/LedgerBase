/*
*	convusrp.c
*
*	Programme to create super user for given district.
*/

#define	MAIN

#include <stdio.h>
#include <isnames.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define SYSTEM		"UTILITIES"
#define MOD_DATE	"01-APR-90"

typedef struct {
	char	u_id[11];	/* user's login name */
	char	u_name[31];	/* user's name for identification */
	char	u_passwd[15];	/* user's password */
	char	u_trml[4];	/* user's login teminal */
	char	u_class[2];	/* user's class: Administrator or User */
	char	u_access[46];	/* All Bit flags in one byte */
} Old_usr;

Old_usr old_usr;

static	char filenm[50];
static	char outfile[50];
static	char tempfile[50];

main(argc,argv)
int	argc ;
char	*argv[] ;
{
	UP_rec	up_rec;
	int	i, status ;
	char	e_mesg[80];
	int	usr_fd;
	int	iostat;

	/* Set The environment */
	strncpy(SYS_NAME, SYSTEM, 50);
	strncpy(CHNG_DATE, MOD_DATE, 10);
	proc_switch(argc, argv, -1) ;

	if(dist_no[0] == '\0')
		set_dist() ;

	status = GetSecurityStatus();
	/* Write the record */
	SetSecurityStatus(0);

	strcpy(filenm,"olduser");

        form_f_name(filenm,outfile);

	usr_fd = isopen(outfile,RWR);
	if(usr_fd < 0) {
	  printf("Error opening old userprof file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(usr_fd);
	  exit(-1);
	}

	iostat = isstart(usr_fd,(char *)&old_usr,0,ISFIRST);
	if(iostat < 0) {
	  printf("Error starting old userprof file. Iserror: %d\n",iserror);
	  close_dbh();
	  isclose(usr_fd);
	  exit(-1);
	}
	for( ; ; ) {
		iostat = isreads(usr_fd,(char *)&old_usr,0);
		if(iostat < 0) {
			if(iostat == EFL) break;
		  	printf("Error reading old userprof file. Iserror: %d\n"
					,iserror);
			break;
		}

		strcpy(up_rec.u_id,old_usr.u_id);
		strcpy(up_rec.u_name,old_usr.u_name);
		strcpy(up_rec.u_passwd,old_usr.u_passwd);
		strcpy(up_rec.u_trml,old_usr.u_trml);
		strcpy(up_rec.u_class,old_usr.u_class);
		for( i=0; i<43; i++ ) {
			up_rec.u_access[i] = old_usr.u_access[i];
		}
/**
			(char)( DFLT_CHAR | P_DEL | UPDATE | ADD | BROWSE );
**/
		for(i=43;i<TOTAL_FILES-1;i++) {
			if(up_rec.u_class[0] == ORD_USER) {
			  up_rec.u_access[i] = (char)( DFLT_CHAR );
			}
			else {
			  up_rec.u_access[i] =
			  (char)( DFLT_CHAR | P_DEL | UPDATE | ADD | BROWSE );
			}
		}
		up_rec.u_access[TOTAL_FILES-1] = old_usr.u_class[45];
		up_rec.u_access[TOTAL_FILES-2] = old_usr.u_class[44];
		up_rec.u_access[TOTAL_FILES-3] = old_usr.u_class[43];

		up_rec.u_access[TOTAL_FILES] = '\0' ;


		i = put_userprof( &up_rec,ADD,e_mesg );
		if( i < 0 )
			printf("\nError in adding SuperUser Profile\n%s\n",e_mesg);
		else if( commit(e_mesg) < 0)
			printf("\nError in adding SuperUser Profile\n%s\n",e_mesg);

	}
	SetSecurityStatus(status);

	close_dbh() ;
	exit(1) ;
}
/*-------------------------END OF FILE------------------------------*/

