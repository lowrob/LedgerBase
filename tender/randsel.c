#define	MAIN

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <bfs_defs.h>
#include <bfs_recs.h>
#include <linklist.h>

/* 0 - primary list, 1 - secondary list, 2 - Selected List */
#define PRIMARY		0
#define SECONDARY	1
#define SELECTED	2
#define MAX_LIST	3

static List list_hdr[MAX_LIST]; 	
static long hdr_info[MAX_LIST];

static	PotBidder	pb_rec;
static	PotBidder	*pb_ptr, *sel_ptr;

static long	seed;
static char	e_mesg[80];
main(argc,argv)
int	argc;
char	*argv[];
{
	int retval;
	int first, list, i;
	long	prev_cnt;

	proc_switch(argc,argv) ;

	seed = get_fulltime();

	/* Initialize Link Lists */
	for(i=0;i<MAX_LIST;i++) {
		list_hdr[i] = list_make(sizeof(PotBidder));
		if(list_hdr[i]==NOLIST) {
			printf("Error Creating Link List");
			exit(0);
		}
		hdr_info[i] = 0;
	}

	/* Fill Linked List */
	pb_rec.pb_categ_num = atoi(argv[1]);	
	pb_rec.pb_select = 0;
	pb_rec.pb_supp[0] = '\0';
	flg_reset(POTBIDDER);

	list = 0;
	first = 0;
	for( ; ; ) {
		retval = get_n_potbidder(&pb_rec,BROWSE,1,FORWARD,e_mesg);
		if( retval < 0) {
			if(retval == EFL) break;
			printf("%s\n",e_mesg);
			exit(0);
		}

		if(pb_rec.pb_categ_num != atoi(argv[1])) {
			break;
		}
		if(first == 0) {
			prev_cnt = pb_rec.pb_select;
			first = 1;
		}
		if(prev_cnt != pb_rec.pb_select) {
			list++;
			prev_cnt = pb_rec.pb_select;
		}	
		list_set(list_hdr[list],LAST_NODE);
		pb_ptr = list_add(list_hdr[list],&pb_rec,AFTER);
		hdr_info[list]++;
	}

	/* Select Suppliers and put int Selected List */
	select_supplier(atoi(argv[2]));

	/* Show Suppliers to me */
	sel_ptr = list_get(list_hdr[SELECTED],FIRST_NODE);
	if(sel_ptr == NOLIST) return(ERROR);
	for(;sel_ptr!=NOOBJ;sel_ptr=list_get(list_hdr[SELECTED],NEXT_NODE)) {
		printf("Supplier: %s  Selected: %ld\n",
			sel_ptr->pb_supp,sel_ptr->pb_select);

		pb_rec.pb_categ_num = sel_ptr->pb_categ_num ;
		strcpy(pb_rec.pb_supp,sel_ptr->pb_supp);
		retval = get_potbidder(&pb_rec,UPDATE,0,e_mesg);
		if(retval < 0) {
			printf("%s\n",e_mesg);
			roll_back(e_mesg);
			exit(0);
		}

		pb_rec.pb_select++;
		retval = put_potbidder(&pb_rec,UPDATE,e_mesg);
		if(retval < 0) {
			printf("%s\n",e_mesg);
			roll_back(e_mesg);
			exit(0);
		}
		retval = commit(e_mesg);
		if(retval < 0) {
			printf("%s\n",e_mesg);
			roll_back(e_mesg);
			exit(0);
		}
	}
	for(i=0;i<MAX_LIST;i++) {
		list_kill(list_hdr[i]);
	}
	exit(0);
}
select_supplier(cnt)
int	cnt;
{
	int	i,j;
	int	list;
	long	rand;

	list = 0;
	for(i=0;i<cnt;i++) {
		if(hdr_info[list] == 0) list++;
		if(list == SELECTED) break;
		rand = random(hdr_info[list]);

		pb_ptr = list_get(list_hdr[list],FIRST_NODE);
		if(pb_ptr == NOLIST) return(ERROR);
		for(j=1;j<rand;j++) {
			pb_ptr = list_get(list_hdr[list],NEXT_NODE);
			if(pb_ptr == NOLIST) return(ERROR);
		}

		list_set(list_hdr[SELECTED],LAST_NODE);
		sel_ptr = list_add(list_hdr[SELECTED],pb_ptr,AFTER);
		list_remove(list_hdr[list]);
		hdr_info[list]--;
	}
	return(0);
}
random(limit) 
long	limit;
{
	long	result;
	seed = seed * (limit + 12345); 
	result = abs(((seed/(limit+2)) % (limit+1)));	
	if(result == 0) result++;
	return(result);	
}
/*---------------------------------------------------------*/
/*   Returns system time in HHMMSS format */
get_fulltime()
{
	struct	tm	*newtime;
	long	ltime ;
	int	run_time ;

	time (&ltime) ;
	newtime = localtime (&ltime) ;

	run_time = (newtime->tm_hour * 10000) + (newtime->tm_min * 100)
			+ (newtime->tm_sec);

	return(run_time);
}
