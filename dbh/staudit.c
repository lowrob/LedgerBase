#include <bfs_defs.h>
#include <bfs_recs.h>

static char	err_file[60] ;
static int	log_fd ;

// Louis R. 2020/04/04
/*** New compiler need to define functions with prototypes ***/
static int open_log(void);

stock_audit(prog_name,old_rec,new_rec,flag)
char	*prog_name;
St_mast	*old_rec ;
St_mast	*new_rec ;
int	flag;
{
	int	i;
	char  	st_log[132];
	char  	st_log1[132];
	char  	st_log2[132];
	char	junk[132];

	for(i=0;i<132;i++) {
		st_log[i] = '\0';
		st_log1[i] = '\0';
		st_log2[i] = '\0';
	}


	sprintf(st_log,"%d-%s\n",old_rec->st_fund,old_rec->st_code);
	sprintf(st_log1,"old: %12.4lf  %12.4lf  %12.4lf  %12.2lf  %12.2lf\n",
		old_rec->st_on_hand,old_rec->st_on_order,old_rec->st_paidfor,
		old_rec->st_value,old_rec->st_rate);
	sprintf(st_log2,"new: %12.4lf  %12.4lf  %12.4lf  %12.2lf  %12.2lf\n",
		new_rec->st_on_hand,new_rec->st_on_order,new_rec->st_paidfor,
		new_rec->st_value,new_rec->st_rate);

/***
	strcpy(err_file, DATA_PATH) ;
	strcat(err_file, "stock_log");
***/
	
	if(open_log()<0) return(-1);

	if(flag==0) {
		sprintf(junk,"<============================= %11s =============================>\n",prog_name);
		write(log_fd, junk , strlen(junk));
	}
	write(log_fd, st_log, strlen(st_log) ) ;
	write(log_fd, st_log1, strlen(st_log1) ) ;
	write(log_fd, st_log2, strlen(st_log2) ) ;

	close(log_fd);
	return (0);
}
stock_commit()
{
	char	st_log[80];

	if(open_log()<0) return(-1);

	sprintf(st_log,"<=============================== Committed ===============================>\n");
	write(log_fd, st_log, strlen(st_log) ) ;

	close(log_fd);
	return (0);
}
static int
open_log() {
	form_f_name("stock_log",err_file);

	if ( (log_fd=open (err_file, TXT_RWMODE)) < 0 )
		if( (log_fd = creat(err_file,TXT_CRMODE)) < 0)  {
#ifdef ENGLISH
			printf("Open Error On %s... errno: %d ... Contact Systems Manager\n",
				err_file, errno);
#else
			printf("Erreur ouverte sur %s... errno: %d ...Contactez groupe support du logiciel\n",
				err_file, errno);
#endif
			return(-1) ;
		}
	
	lseek(log_fd, (long)0, 2 );
}
