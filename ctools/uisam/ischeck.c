/*--------------------------------------------------------------------------+ 
 
   ISAM Check convertion program.     	(Source File :ischeck.c )

	Usage : ischeck.out 
			  
  	Compilation : CC ischeck.c isam.o -o ischeck.out 

	Modification History :
		Created : Dt. 1-AUG-89 .	Author :Kvs.
 
+---------------------------------------------------------------------------*/

#include <stdio.h>
#include "isnames.h"
#include "isdef.h"
#include "isflsys.h" 

#define MAX_INDX     25
#define KEY 0 
#define REC 1
#define   offset(I,J)  (HEADSZ+FIELDSZ*maxk[J] + len[J]*I)  

static  int fd, fd2 ;
static  int len[MAX_INDX], maxk[MAX_INDX] ;
static	int partsarray[1024], *aray ;
static	int *paray[MAX_INDX] ;
static  int blen ;

extern	int errno  ;

long    lseek() ;
char	*malloc() ;

static struct
{
	char 	buff[MAXSZ] ;
	long 	dumm ;
}	node ;

/***
static	char	*rec, *old_rec ;
***/
static	char	rec[MAX_REC_LEN], old_rec[MAX_REC_LEN] ;
static  char    filename[FLNM_LEN];

main(argc, argv)
int  argc ;
char *argv[] ;
{
	struct  indxheader    *header ;
	struct  keydat        *altkptr ;
	int                   count , i, j ,altnos ,reset_flag ;
	char                  *ptr1 ;
	int                   *prts, parts;
	char                  datfile[FLNM_LEN];
	unsigned              the_root[MAX_INDX] ;
	int		      code, first_time, temp1 ;

#ifdef	FIXED_RECLEN
	printf("\n\tFixed Record Length Version\n\n");
#else
	printf("\n\tVariable Record Length Version\n\n");
#endif

	if(argc < 2) {
		printf ( "File Name: ");
		scanf ("%s", datfile);
	}
	else
		strcpy(datfile, argv[1]) ;

	strcpy (filename, datfile);
	strcat (filename, ".IX");
	if ( (fd=open(filename, RDONLY)) < 0 )
	{
		printf ("**** file %s could not be opened ...\n", filename) ;
		return(0) ;
	}
	if ( read(fd, node.buff,1024) < 1024)
	{
		printf ("**** error in 0th rec read file %s ****\n",filename);
		return(0);
	}
	ptr1 = &node.buff[0] ;
	header = (struct indxheader *) ptr1 ;

	printf (" total index nodes: %d\n", header->pnumrecs) ;
#ifdef	FIXED_RECLEN
	printf (" Record Length    : %d\n", (blen=header->preclength));
#endif

	printf (" next data position: %ld\n", header->pnxtposn );
	altnos=header->paltkeys ;
	printf (" Number Of Keys            : %d\n", altnos);

	altkptr = (struct keydat *)(++header) ;
	aray = partsarray ;

	for ( i = 0 ; i < altnos ; i++)
	{

		printf(
	"key length: %d Parts: %d root node: %d tot keys: %d Maxkes:%d\n", 
		    (len[i]=altkptr->pkeylength), (parts=altkptr->pkeyparts), 
		    (the_root[i]=altkptr->proot),(altkptr->pnumkeys), 
		    (maxk[i] = altkptr->pmaxkeys ));
		
		altkptr++;
		prts = (int *)(altkptr) ;
		paray[i] = aray ;
		*(aray++) = parts ;

		for ( j = 0; j < parts ; j++) {
			printf(
			"  Key part: %d  Order: %d  Pos: %d  Length: %d Type: %d\n",
			    j,
			    (*(aray++) = *(prts++)),
			    (*(aray++) = *(prts++)),
			    (*(aray++) = *(prts++)),
			    (*(aray++) = *(prts++)));
		}
		altkptr = (struct keydat *)prts ;
	}

	close(fd) ;

	/*--- read all index nodes ...----*/

	do {
		printf ( "Check On which key(0-%d)?",altnos-1);
		scanf ("%d", &reset_flag) ;
	} while ( reset_flag >= altnos) ;
	
	fd  = isopen(datfile, R) ;
	fd2 = isopen(datfile, R) ;

	if ( fd < 0 || fd2 < 0 ) {
		printf(" isopen error :%d\n", iserror) ;
		exit(0) ;
		}

	code = isstart(fd, (char *)NULL, reset_flag, ISFIRST) ;

	if ( code == ERROR ) printf("isstart error :%d\n", iserror) , exit(0);

	/*****
	rec	= malloc( (unsigned) blen) ;
	old_rec	= malloc( (unsigned) blen) ;
	
	if ( rec == NULL || old_rec == NULL ) {
		printf(" Allocation Error \n");
		exit(0) ;
	}
	******/

	for(count=0, first_time = 1;;count++) {
		code = isreads(fd, rec, 0) ;
		if ( code == EFL ) break ;
		if ( code < 0 ) {
			printf(" Isreads Error :%d\n", iserror) ;
			break ;
		}
		/** Compare this and previous key **/

		if ( !first_time)  {
			if (comp_key(old_rec, rec,paray[reset_flag])
						< 0 ) {
				printf(" Non Sequential Order Found \n");
			    	diaplay_key( old_rec, paray[reset_flag], REC );
			    	printf("Type Integer to Continue: ");
			    	scanf("%d", &temp1) ;
				}
		}
		if ( first_time ) first_time = 0 ;
		scpy(old_rec, rec, code) ;

		code = isreadr(fd2, rec, reset_flag, 0) ;
		if ( code < 0 ) {
			printf(" Key Absent :");
			diaplay_key(rec, paray[reset_flag], REC) ;
			}
		}

	printf(" %d Records Verified \n\n", count) ;

	exit(1) ;
} /* end of main */

int
scpy(ptr1, ptr2, len)
char	*ptr1, *ptr2;
int 	len ;
{
	int i;
	for(i = 0 ; i < len ; i++)
		*(ptr1+i) = *(ptr2+i) ;
}
/*--------------------------------------------------------*/

int
comp_key ( key, rec, partsaray)
char           *key, *rec ;
int            *partsaray  ;
{
	int	i, j, type, len, pos, parts, order ;
	char	c, c1, *o_key, *k_key;
	long	l_ptr, l1_ptr ;
	float	f_ptr , f1_ptr;
	short	s_ptr, s1_ptr ;
	double	d_ptr , d1_ptr;

	parts = *(partsaray++) ;

	for ( j = 0 ; j < parts ; j++) {
		type = *(partsaray++) ;
		len = *(partsaray++) ;
		pos = *(partsaray++) ;
		order = *(partsaray++);		/* correct increment */
		o_key = rec + pos ;
		k_key = key + pos ;

		if ( order == ASCND)
			order = 1 ;
		else if ( order == DESCND)
			order = -1 ;

		switch ( type ) {

		case  DATE  :
		case  LONG  :

			for ( i = 0 ; i < len ; i++){
				scpy( ( char *)&l_ptr ,k_key , sizeof(long) );
				scpy( ( char *)&l1_ptr ,o_key , sizeof(long) );
				if ( l_ptr < l1_ptr ) return(1*order) ;
				if ( l_ptr > l1_ptr ) return(-1*order) ;
				k_key += sizeof(long);
				o_key += sizeof(long);
			}
			continue ;
	
		case  FLOAT :       
			for ( i = 0 ; i < len; i++){
				scpy( (char *)&f_ptr ,k_key, sizeof(float) ) ;
				scpy( (char *)&f1_ptr ,o_key, sizeof(float) ) ;
				if ( f_ptr < f1_ptr ) return(1*order) ;
				if ( f_ptr > f1_ptr ) return(-1*order) ;
				k_key += sizeof(float);
				o_key += sizeof(float);
			}
			continue ;

		case  DOUBLE :       
			for ( i = 0 ; i < len; i++){
				scpy( (char *)&d_ptr ,k_key, sizeof(double) ) ;
				scpy( (char *)&d1_ptr ,o_key, sizeof(double) ) ;
				if ( d_ptr < d1_ptr ) return(1*order) ;
				if ( d_ptr > d1_ptr ) return(-1*order) ;
				k_key += sizeof(double);
				o_key += sizeof(double);
			}
			continue ;

		case  SHORT :       
			for ( i = 0 ; i < len; i++){
				scpy( (char *)&s_ptr ,k_key, sizeof(short) ) ;
				scpy( (char *)&s1_ptr ,o_key, sizeof(short) ) ;
				if ( s_ptr < s1_ptr ) return(1*order) ;
				if ( s_ptr > s1_ptr ) return(-1*order) ;
				k_key += sizeof(short);
				o_key += sizeof(short);
			}
			continue ;

		case CHAR :           
#ifdef	O_ISAM
			for ( i=0; i < len; i++ ) {
				c = *(k_key++) ;
				c1 = *(o_key++) ;
				if ( c < c1 ) return(1*order) ;
				if ( c > c1 ) return(-1*order) ;
			}
#else
			/*
			*	For character type comparison only upto NULL
			*	added on 2-OCT-89.. amar
			*/
			for ( i=0; i < len; i++ ) {
				c = *(k_key++) ;
				c1 = *(o_key++) ;
				if ( c == '\0' && c1 == '\0'){
					k_key += len - i - 1 ;
					o_key += len - i - 1 ;
					break ;
				}
				if ( c < c1 ) return(1*order) ;
				if ( c > c1 ) return(-1*order) ;
			}
#endif
			continue ;
		default :
			printf(" Illegal Type ...");
			return(1*order) ;
		}
	}

	return(0) ;
}

int
diaplay_key ( key, partsaray, r_type)
char           *key ;
int            *partsaray , r_type ;
{
	int	i, j, type, len, pos, parts ;
	char	c, *o_key, *k_key;
	long	l_ptr ;
	float	f_ptr ;
	short	s_ptr ;
	double	d_ptr ;

	parts = *(partsaray++) ;
	k_key = o_key = key ;

	for ( j = 0 ; j < parts ; j++) {
		type = *(partsaray++) ;
		len = *(partsaray++) ;
		pos = *(partsaray++) ;
		partsaray++ ;			/* for correct display */

		if ( r_type == 1 ) k_key = o_key + pos ;
		printf(" Part-%d:", j) ;

		switch( type ) {

		   case DATE :
		   case LONG :
			for ( i = 0 ; i < len ; i++ ){
				scpy( ( char *)&l_ptr ,k_key , sizeof(long) );
				printf(" %ld",  l_ptr) ;
				k_key += sizeof(long) ;
			}
			continue ;

		   case FLOAT :        
			for ( i = 0 ; i < len; i++){
				scpy( (char *)&f_ptr ,k_key, sizeof(float) ) ;
				printf(" %f", f_ptr) ;
				k_key += sizeof(float);
			}
			continue ;

		   case DOUBLE :        
			for ( i = 0 ; i < len; i++){
				scpy( (char *)&d_ptr ,k_key, sizeof(double) ) ;
				printf(" %lf", d_ptr) ;
				k_key += sizeof(double);
			}
			continue ;

		   case SHORT :        
			for ( i = 0 ; i < len; i++){
				scpy( (char *)&s_ptr ,k_key, sizeof(short) ) ;
				printf(" %d", s_ptr) ;
				k_key += sizeof(short);
			}
			continue ;

		   case CHAR :           
#ifdef	O_ISAM
			for ( i=0; i < len; i++ ) {
				c = *(k_key) ;
				printf( "%c", c) ;
				k_key++ ;
			}
#else
			printf(" ");
			for ( i=0 ; i < len && *k_key ; i++ ) {
				c = *(k_key) ;
				printf( "%c", c) ;
				k_key++ ;
			}
			k_key += len - i ;
#endif
			continue ;
	  	     default :
			printf("illegal type ...");
			return(1) ;
		}
	}
	printf("\n");

	return(0) ;
}

