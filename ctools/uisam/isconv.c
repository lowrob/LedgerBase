/*--------------------------------------------------------------------------+ 
 
	Source File: isconv.c

	Usage : isconv.out 
			  
  	Compilation : CC isconv.c -o isconv.out 

	Synopsis : Traverses thru the index nodes and displays the Index
		information of given ISAM file. Reads the header block of
		the supplied file and displays key parameters.

	Limitations:
		Max keys(Indices) in a file	= 25 .
		Max file names acceped		= 50 Chars.
	
	Modification History :
		Created : Dt. 7-Jan-87 .	Author :Kvs.
 
+---------------------------------------------------------------------------*/

#include <stdio.h>
#include <errno.h>
#include "isnames.h"
#include "isdef.h"
#include "isflsys.h" 

#define MAX_INDX     25
#define KEY 0 
#define REC 1
#define   offset(I,J)  (HEADSZ+FIELDSZ*maxk[J] + len[J]*I)  


static	union kb {
	long 	kumm ;
	char 	keybuff[MAX_REC_LEN] ;
}	key, old_key  ;

static struct bf {
	char 	buff[1028] ;
	long 	dumm ;
}	b ;

static  int fd, fd2 ;
static  int len[MAX_INDX], maxk[MAX_INDX] ;
static	int partsarray[1024], *aray ;
static  int level ;
static  int j, blen ;
static	int	first_time = 1; 

static  char    *ptr1;
static  int 	of_ptr1;
static  char    filename[FLNM_LEN];
static	int *paray[MAX_INDX] ;
static	int whichkey ;

extern	int	errno , iserror;
int 	DT_TYPE ;

long    lseek() ;


main(argc, argv)
int  argc ;
char *argv[] ;
{
	struct  indxheader    *header ;
	struct  keydat        *altkptr ;
	int                   i, j ,altnos ,reset_flag ;
	char                  *ptr1 ;
	int                   *prts, parts;
	char                  datfile[FLNM_LEN];
	unsigned              the_root[MAX_INDX] ;
	unsigned 	      start_node ;

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

	if ( (fd2=open (datfile, RDONLY)) < 0 )
	{
		printf ("**** file %s could not be opened ..\n", datfile) ;
		return(0) ;
	}
	strcpy (filename, datfile);
	strcat (filename, ".IX");
	if ( (fd=open(filename, RDONLY)) < 0 )
	{
		printf ("**** file %s could not be opened ...\n", filename) ;
		return(0) ;
	}
	if ( read(fd, b.buff,1024) < 1024)
	{
		printf ("**** error in 0th rec read file %s ****\n",filename);
		return(0);
	}
	ptr1 = &b.buff[0] ;
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

	/*--- read all index nodes ...----*/

	printf ( "traverse on which key(0-%d)?",altnos-1);
	scanf ("%d", &reset_flag) ;
	/****
	printf ( " Start Node Number :(0-n)?") ;
	scanf("%d", &start_node) ;
	*****/
	start_node = 0 ;

	whichkey = reset_flag ;
	if ( start_node == 0 ) 
		display_node( the_root[whichkey] ) ;
	else display_node( start_node ) ;

	printf ( "level: %d\n", level) ;
	close(fd) ;
	close(fd2) ;
	exit(1) ;
} /* end of main */


display_node(i)
unsigned int i ;
{
	char	status[4] ;
	int	temp = 1, temp1 ;
	unsigned        int brother,  root;
	int     numkeys ,ii ;
	struct  field        *fldptr ;
	struct  header       *head   ;
	int 	rec_count = 0, deleted = 0 ;

	/** -- findleast node --**/

	for(;;) {
		if ( rdnode(i) < 0 ) return(-1) ;
		level++ ;
		ptr1 = b.buff ;
		head = (struct header *)ptr1  ;
		numkeys = head->n;
		root    = head->a0 ;

		printf("numkeys in node %d are: %d with child node at: %d\n", 
		    i,numkeys, root);
		printf ("subindex nodes are ...\n");

		if ( root == 0 ) break ;
		i = root ;
	}

	/** at the leaf node.. so read all nodes and display info **/

	brother = i ;
	j = whichkey ;

	do 
	{
		ptr1 = b.buff ;
		head = (struct header *)ptr1 ;
		numkeys = head->n ;
		printf("Node: %d numks: %d right link: %d\n",
		    brother, numkeys,head->ritlink);
		brother = head->ritlink ;
		head++ ;
		fldptr = (struct field *)(head) ;

		for (ii = 0 ; ii < numkeys ; ii++,fldptr++, rec_count++) {
			if ( lseek(fd2, (long)(fldptr->a), 0) < 0 )
				printf ("seek error on data file: errno %d\n",
						errno);
			if ( temp == STATUS_LEN ) {
				read(fd2, status, STATUS_LEN) ;
				if ( status[0] != '0' ) {
					printf("* DELETED * "); 
					deleted++ ;
				}
			}
#ifndef	FIXED_RECLEN
			blen = fldptr->datlength - STATUS_LEN ;
			printf(" Addr :%ld len :%d ", fldptr->a, blen); 
#endif
			of_ptr1 = sizeof(struct header) + maxk[j] *
					sizeof(struct field)  + len[j] * ii ;
			if ( read(fd2, key.keybuff, blen) < blen)
				printf("key read error\n");

			diaplay_key( ptr1+of_ptr1, paray[j], KEY   );

			/** match the index and data key **/
			if(comp_key(ptr1+of_ptr1, key.keybuff, paray[j] ) != 0){
			    printf("key mismatch ..\n");
			    printf("Rec#: %d  Data addr: %ld  Data len: %d\n",
				    rec_count,fldptr->a, blen) ;
			    printf(" Offset: %d\n", of_ptr1) ;
			    diaplay_key( key.keybuff, paray[j], REC );

			    /* Wait for users response when key mismatch */

			    printf("Type Integer to Continue: ");
			    scanf("%d", &temp1) ;

			    continue ;
			}
			/** Compare this and previous key **/

			if ( !first_time)  {
			   if (comp_key(old_key.keybuff,key.keybuff,paray[j])
							< 0 ) {
				printf(" Non Sequential Order Found \n");
			    	diaplay_key( old_key.keybuff, paray[j], KEY );
			    	printf("Type Integer to Continue: ");
			    	scanf("%d", &temp1) ;
				}
			}
			if ( first_time ) first_time = 0 ;


			scpy(old_key.keybuff, ptr1+of_ptr1, len[j] ) ;

		} /** for **/

		if ( brother != 0 ) {
			printf(" Reading node: %d\n", brother) ;
			if ( rdnode(brother) < 0 ) return(-1) ;
		}
		else 
			break ;
	}
	while (1);

	printf("Total Records Read: %d", rec_count );
	if(STATUS_LEN)
		printf("\tDeleted: %d\n", deleted) ;
	else
		printf("\n");

	return(0) ;
}

 int             
rdnode(i)
unsigned i ;
{
int ret_len ;

	if (lseek ( fd, (long) ((long)1024*(i)), 0) < 0 )
	{
		printf("seek errn");
		return(-1);
	}
	if ( (ret_len=read(fd, b.buff,1024)) < 1024)
	{
		printf("error in reading node: %d errno %d ret len: %d ",
			i, errno, ret_len);
		return(-1);
	}
	return(0) ;
}

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
comp_key ( i_key, rec, partsaray)
char	*i_key, *rec ;
int	*partsaray  ;
{
	int	i, j, type, len, pos, parts, order ;
	char	c, c1, *o_key, *o_rec, *k_key;
	long	l_ptr, l1_ptr ;
	float	f_ptr , f1_ptr;
	short	s_ptr, s1_ptr ;
	double	d_ptr , d1_ptr;

	parts = *(partsaray++) ;
	o_rec = o_key = rec ;
	k_key = i_key ;

	for ( j = 0 ; j < parts ; j++) {
		type = *(partsaray++) ;
		len = *(partsaray++) ;
		pos = *(partsaray++) ;
		order = *(partsaray++);		/* correct increment */
		o_rec = o_key + pos ;

		if ( order == ASCND)
			order = 1 ;
		else if ( order == DESCND)
			order = -1 ;

		switch ( type ) {

		case  DATE  :
		case  LONG  :

			for ( i = 0 ; i < len ; i++){
				scpy( ( char *)&l_ptr ,k_key , sizeof(long) );
				scpy( ( char *)&l1_ptr ,o_rec , sizeof(long) );
				if ( l_ptr < l1_ptr ) return(1*order) ;
				if ( l_ptr > l1_ptr ) return(-1*order) ;
				k_key += sizeof(long);
				o_rec += sizeof(long);
			}
			continue ;
	
		case  FLOAT :       
			for ( i = 0 ; i < len; i++){
				scpy( (char *)&f_ptr ,k_key, sizeof(float) ) ;
				scpy( (char *)&f1_ptr ,o_rec, sizeof(float) ) ;
				if ( f_ptr < f1_ptr ) return(1*order) ;
				if ( f_ptr > f1_ptr ) return(-1*order) ;
				k_key += sizeof(float);
				o_rec += sizeof(float);
			}
			continue ;

		case  DOUBLE :       
			for ( i = 0 ; i < len; i++){
				scpy( (char *)&d_ptr ,k_key, sizeof(double) ) ;
				scpy( (char *)&d1_ptr ,o_rec, sizeof(double) ) ;
				if ( d_ptr < d1_ptr ) return(1*order) ;
				if ( d_ptr > d1_ptr ) return(-1*order) ;
				k_key += sizeof(double);
				o_rec += sizeof(double);
			}
			continue ;

		case  SHORT :       
			for ( i = 0 ; i < len; i++){
				scpy( (char *)&s_ptr ,k_key, sizeof(short) ) ;
				scpy( (char *)&s1_ptr ,o_rec, sizeof(short) ) ;
				if ( s_ptr < s1_ptr ) return(1*order) ;
				if ( s_ptr > s1_ptr ) return(-1*order) ;
				k_key += sizeof(short);
				o_rec += sizeof(short);
			}
			continue ;

		case CHAR :           
#ifdef	O_ISAM
			for ( i=0; i < len; i++ ) {
				c = *(k_key++) ;
				c1 = *(o_rec++) ;
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
				c1 = *(o_rec++) ;
				if ( c == '\0' && c1 == '\0'){
					k_key += len - i - 1 ;
					o_rec += len - i - 1 ;
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
diaplay_key ( i_key, partsaray, r_type)
char	*i_key ;
int	*partsaray , r_type ;
{
	int	i, j, type, len, pos, parts ;
	char	c, *o_key, *k_key;
	long	l_ptr ;
	float	f_ptr ;
	short	s_ptr ;
	double	d_ptr ;

	parts = *(partsaray++) ;
	k_key = o_key = i_key ;

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

