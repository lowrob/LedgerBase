/*
*
*	init_id.c
*
*	Synopsis:
*		This programs creates a file called KEY_DESC (see '.h' files
*		for real name), which contains file attributes for all DBH
*		files. For ISAM files, key positions are calculated from the
*		start of record. Key types and length are similarly gathered.
*		All this info is placed in a structure called "id_array".
*		One id_array record for each DBH file is written in to the
*		KEY_DESC file. DBH reads this file once in similar "id_array"
*		in "recio.c" and thus gets all file attributes required to
*		fullfil a I-O request on a DBH file .
*
*		Two tempoary files (TMPINDX_1, TMPINDX2) are added at the end
*		to KEY_DESC file. These are useful to create & open temporay
*		files.
*/

#define	MAIN

#define SYSTEM		"DBH INITIALIZATION"
#define MOD_DATE	"23-JAN-90"

#include <stdio.h>
#include <isnames.h>
#include <bfs_defs.h>
#include <bfs_recs.h>
#include <bfs_pp.h>

#include <filein.h>

#define	TOT_KEYS	10000  		/* keysarray size for all files' keys */

#ifdef	JRNL
extern	int	journaling ;
#endif

static	char	c_mesg[80] ;

extern	f_id	id_array[];

main(argc, argv)
int	argc ;
char	*argv[] ;
{
	int	k_fd,i ;		/* file fd for keys etc*/
	f_id 	file_desc ;		/* FIle descriptor     */
	char	file_name[50];
	int	*keysarray ;		/* keysarray (earlier in f_id) */
	int	offset = 0 ;
	char	*malloc() ;
	int	max_size ;		/* Max Keyarray size used by any file */
	int	code, j ;

	/* Set The Environmnet */
	strncpy(SYS_NAME, SYSTEM, 50);
	strncpy(CHNG_DATE, MOD_DATE, 10);
	proc_switch(argc, argv, -1) ;

	/****
	if ( dist_no[0] == '\0' )
		set_dist() ;		*****/   /* Set the district number */

	/*** If file exists then delete and recreate ***/

	strcpy (file_name, DATA_PATH);
	strcat (file_name, KEY_DESC);
	/**
	form_f_name(KEY_DESC, file_name);
	**/

	if ( access(file_name,0) >= 0 )
		unlink(file_name);

	if ( (k_fd=creat(file_name, CRMODE)) < 0 ) {
		printf("Create error In Id initialisation\n") ;
		exit(-1) ;
	}

	if ( (keysarray = (int *)malloc(TOT_KEYS * sizeof(int))) == NULL ) {
		printf(" Allocation error \n");
		exit(-1) ;
	}
	keysarray[0] = 0 ;		/* first int gives total size */
	offset++ ;
	max_size = 0 ;

#ifdef	JRNL
	/* Allocate BUFSIZ bytes for journal area non interactively */
/*******
	printf("Do You Want Journalling? (1-yes, 0-no): ");
	scanf("%d",&journaling);
	if(journaling != 1) journaling = 0 ;
*******/
	journaling = 1;
	if(write(k_fd,(char*)&journaling,sizeof(int)) < sizeof(int)) {
		printf("ERROR while Writing\n");
		close(k_fd);
		exit(-1);
	}
/*******
	if(journaling) {
		printf("Journal Area Size: ");
		scanf("%d",&journaling);
		if(journaling < 0) journaling = 0 ;
	}
*******/
	journaling = BUFSIZ;
	if(write(k_fd,(char*)&journaling,sizeof(int)) < sizeof(int)) {
		printf("ERROR while Writing\n");
		close(k_fd);
		exit(-1);
	}
#endif

#ifdef	SECURITY
/*******
	printf("Do You Want Security ON? (1-yes, 0-no): ");
	scanf("%d",&i);
	if(i != 1) i = 0 ;
*******/
	i = 1;
	if(write(k_fd,(char*)&i,sizeof(int)) < sizeof(int)) {
		printf("ERROR while Writing\n");
		close(k_fd);
		exit(-1);
	}
#endif

	file_desc.id_lock[0] 	= '0' ;
	file_desc.id_fd 	= -1 ;
	file_desc.id_start 	= -1 ;
	file_desc.id_data 	= 0 ;
	file_desc.id_curkey 	= 0 ;

	printf ("PARAM FILE...\t");

	strcpy(file_desc.id_f_name, PARAM_FILE );
	strcpy(file_desc.fl_user_nm,"Parameters");
	file_desc.id_f_type 	= SEQ ;
	file_desc.id_io_mode 	= RWMODE ;
	file_desc.reclen   =  sizeof(Pa_rec) ;

	file_desc.tot_keys = 0 ;
	file_desc.keys_offset = offset ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("CONTROL FILE...\n");

	strcpy(file_desc.id_f_name, CONTROL_FILE );
	strcpy(file_desc.fl_user_nm,"Control File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Ctl_rec) ;

	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Ctl_rec *)NULL)->fund -
				   (char *)((Ctl_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("\tSCHOOL FILE...\t");

	strcpy(file_desc.id_f_name, SCHOOL_FILE );
	strcpy(file_desc.fl_user_nm,"School File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Sch_rec) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Sch_rec *)NULL)->sc_numb -
				   (char *)((Sch_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("AUDIT FILE...\n");

	strcpy(file_desc.id_f_name, AUDIT_FILE );
	strcpy(file_desc.fl_user_nm,"Audit File");
	file_desc.id_f_type 	= SEQ ;
	file_desc.id_io_mode 	= RWMODE ;
	file_desc.reclen   =  sizeof(Aud_rec) ;

	file_desc.tot_keys = 0 ;
	file_desc.keys_offset = offset ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("GL_MASTER FILE...\t");

	strcpy(file_desc.id_f_name, GLMAST_FILE );
	strcpy(file_desc.fl_user_nm,"GL Master File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Gl_rec) ;
	file_desc.tot_keys = 4 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Gl_rec *)NULL)->funds -
				   (char *)((Gl_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] = 18 ;
	keysarray[offset++] = (char *)&((Gl_rec *)NULL)->accno[0] -
				   (char *)((Gl_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Gl_rec *)NULL)->reccod -
				   (char *)((Gl_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 1st alternate Key */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Gl_rec *)NULL)->funds -
				   (char *)((Gl_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] = DESC_KEY ;
	keysarray[offset++] = (char *)&((Gl_rec *)NULL)->desc[0] -
				   (char *)((Gl_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 2nd alternate Key */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Gl_rec *)NULL)->reccod -
				   (char *)((Gl_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Gl_rec *)NULL)->funds -
				   (char *)((Gl_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] = (char *)&((Gl_rec *)NULL)->sect -
				   (char *)((Gl_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] = 18 ;
	keysarray[offset++] = (char *)&((Gl_rec *)NULL)->accno[0] -
				   (char *)((Gl_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 3rd alternate Key */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Gl_rec *)NULL)->funds -
				   (char *)((Gl_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] = (char *)&((Gl_rec *)NULL)->sect -
				   (char *)((Gl_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Gl_rec *)NULL)->reccod -
				   (char *)((Gl_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] = 18 ;
	keysarray[offset++] = (char *)&((Gl_rec *)NULL)->accno[0] -
				   (char *)((Gl_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("GST DISTRIBUTION FILE...\t");

	strcpy(file_desc.id_f_name, GSTDIST_FILE );
	strcpy(file_desc.fl_user_nm,"GST Dist File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(GST_dist) ;
	file_desc.tot_keys = 2 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((GST_dist *)NULL)->fund -
				   (char *)((GST_dist *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] = 18 ;
	keysarray[offset++] = (char *)&((GST_dist *)NULL)->gst_accno[0] -
				   (char *)((GST_dist *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] = 18 ;
	keysarray[offset++] = (char *)&((GST_dist *)NULL)->accno[0] -
				   (char *)((GST_dist *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt-Key 1 */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((GST_dist *)NULL)->fund -
				   (char *)((GST_dist *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] = 18 ;
	keysarray[offset++] = (char *)&((GST_dist *)NULL)->accno[0] -
				   (char *)((GST_dist *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("GL_MASTER FILE...\t");
	printf ("RECURRING ENTRY HEADER FILE...\n");

	strcpy(file_desc.id_f_name, RECHDR_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Recurring Entry Hdr");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Re_hdr) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Re_hdr *)NULL)->rh_fund -
				   (char *)((Re_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Re_hdr *)NULL)->rh_sno -
				   (char *)((Re_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("\tRECURRING ENTRY ITEMS FILE...\t");

	strcpy(file_desc.id_f_name, RECTRAN_FILE );
	strcpy(file_desc.fl_user_nm,"Recurring Entry Item");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Re_item) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Re_item *)NULL)->ri_fund -
				   (char *)((Re_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Re_item *)NULL)->ri_sno -
				   (char *)((Re_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Re_item *)NULL)->ri_item_no -
				   (char *)((Re_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("G/L TR_HEADER FILE...\n");

	strcpy(file_desc.id_f_name, GLTRHDR_FILE );
	strcpy(file_desc.fl_user_nm,"Trans. Header File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Tr_hdr) ;
	file_desc.tot_keys = 3 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tr_hdr *)NULL)->th_fund -
				   (char *)((Tr_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tr_hdr *)NULL)->th_reccod -
				   (char *)((Tr_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tr_hdr *)NULL)->th_create[0] -
				   (char *)((Tr_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] = (char *)&((Tr_hdr *)NULL)->th_seq_no -
				   (char *)((Tr_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt Key-1 */

	keysarray[offset++] = 6 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = DATE ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] = (char *)&((Tr_hdr *)NULL)->th_sys_dt -
				   (char *)((Tr_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tr_hdr *)NULL)->th_fund -
				   (char *)((Tr_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tr_hdr *)NULL)->th_reccod -
				   (char *)((Tr_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tr_hdr *)NULL)->th_create[0] -
				   (char *)((Tr_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 5 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] = (char *)&((Tr_hdr *)NULL)->th_seq_no -
				   (char *)((Tr_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 6 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  10 ;
	keysarray[offset++] =  (char *)&((Tr_hdr *)NULL)->th_userid[0] -
				   (char *)((Tr_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt key-6 */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  10 ;
	keysarray[offset++] =  (char *)&((Tr_hdr *)NULL)->th_supp_cd[0] -
				   (char *)((Tr_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tr_hdr *)NULL)->th_create[0] -
				   (char *)((Tr_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] = (char *)&((Tr_hdr *)NULL)->th_seq_no -
				   (char *)((Tr_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("\tGL TR_ITEMS FILE...\t");

	strcpy(file_desc.id_f_name, GLTRAN_FILE );
	strcpy(file_desc.fl_user_nm,"GL Trans File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Tr_item);
	file_desc.tot_keys = 2 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 5 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tr_item *)NULL)->ti_fund -
				   (char *)((Tr_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tr_item *)NULL)->ti_reccod -
				   (char *)((Tr_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tr_item *)NULL)->ti_create[0] -
				   (char *)((Tr_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] = (char *)&((Tr_item *)NULL)->ti_seq_no -
				   (char *)((Tr_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 5 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tr_item *)NULL)->ti_item_no -
				   (char *)((Tr_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt Key-1 */

	keysarray[offset++] = 6 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tr_item *)NULL)->ti_fund -
				   (char *)((Tr_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tr_item *)NULL)->ti_reccod -
				   (char *)((Tr_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] = 18 ;
	keysarray[offset++] = (char *)&((Tr_item *)NULL)->ti_accno[0] -
				   (char *)((Tr_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tr_item *)NULL)->ti_period -
				   (char *)((Tr_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 5 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] = (char *)&((Tr_item *)NULL)->ti_seq_no -
				   (char *)((Tr_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 6 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tr_item *)NULL)->ti_item_no -
				   (char *)((Tr_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("G/L TR_HEADER FILE NEW YEAR...\n");

	strcpy(file_desc.id_f_name, GLTRHDRNY_FILE );
	strcpy(file_desc.fl_user_nm,"Trans. Header New Year File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Tr_hdr) ;
	file_desc.tot_keys = 3 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tr_hdr *)NULL)->th_fund -
				   (char *)((Tr_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tr_hdr *)NULL)->th_reccod -
				   (char *)((Tr_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tr_hdr *)NULL)->th_create[0] -
				   (char *)((Tr_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] = (char *)&((Tr_hdr *)NULL)->th_seq_no -
				   (char *)((Tr_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt Key-1 */

	keysarray[offset++] = 6 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = DATE ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] = (char *)&((Tr_hdr *)NULL)->th_sys_dt -
				   (char *)((Tr_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tr_hdr *)NULL)->th_fund -
				   (char *)((Tr_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tr_hdr *)NULL)->th_reccod -
				   (char *)((Tr_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tr_hdr *)NULL)->th_create[0] -
				   (char *)((Tr_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 5 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] = (char *)&((Tr_hdr *)NULL)->th_seq_no -
				   (char *)((Tr_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 6 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  10 ;
	keysarray[offset++] =  (char *)&((Tr_hdr *)NULL)->th_userid[0] -
				   (char *)((Tr_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt key-6 */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  10 ;
	keysarray[offset++] =  (char *)&((Tr_hdr *)NULL)->th_supp_cd[0] -
				   (char *)((Tr_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tr_hdr *)NULL)->th_create[0] -
				   (char *)((Tr_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] = (char *)&((Tr_hdr *)NULL)->th_seq_no -
				   (char *)((Tr_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("\tGL TR_ITEMS FILE NEW YEAR...\t");

	strcpy(file_desc.id_f_name, GLTRANNY_FILE );
	strcpy(file_desc.fl_user_nm,"GL Trans New Year File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Tr_item);
	file_desc.tot_keys = 2 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 5 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tr_item *)NULL)->ti_fund -
				   (char *)((Tr_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tr_item *)NULL)->ti_reccod -
				   (char *)((Tr_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tr_item *)NULL)->ti_create[0] -
				   (char *)((Tr_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] = (char *)&((Tr_item *)NULL)->ti_seq_no -
				   (char *)((Tr_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 5 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tr_item *)NULL)->ti_item_no -
				   (char *)((Tr_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt Key-1 */

	keysarray[offset++] = 6 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tr_item *)NULL)->ti_fund -
				   (char *)((Tr_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tr_item *)NULL)->ti_reccod -
				   (char *)((Tr_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] = 18 ;
	keysarray[offset++] = (char *)&((Tr_item *)NULL)->ti_accno[0] -
				   (char *)((Tr_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tr_item *)NULL)->ti_period -
				   (char *)((Tr_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 5 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] = (char *)&((Tr_item *)NULL)->ti_seq_no -
				   (char *)((Tr_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 6 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tr_item *)NULL)->ti_item_no -
				   (char *)((Tr_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("BUDGET HEADER FILE...\n");

	strcpy(file_desc.id_f_name, BDHDR_FILE );
	strcpy(file_desc.fl_user_nm,"Budget Header File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Bd_hdr) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  4 ;
	keysarray[offset++] =  (char *)&((Bd_hdr *)NULL)->tr_term[0] -
				   (char *)((Bd_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = DATE ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] = (char *)&((Bd_hdr *)NULL)->tr_sys_dt -
				   (char *)((Bd_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] = (char *)&((Bd_hdr *)NULL)->tr_seq_no -
				   (char *)((Bd_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("\tBUDGET ITEMS FILE...\n");

	strcpy(file_desc.id_f_name, BDITEM_FILE );
	strcpy(file_desc.fl_user_nm,"GL Budgetitem File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Bd_item);
	file_desc.tot_keys = 3 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  4 ;
	keysarray[offset++] =  (char *)&((Bd_item *)NULL)->tr_term[0] -
				   (char *)((Bd_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = DATE ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] = (char *)&((Bd_item *)NULL)->tr_sys_dt -
				   (char *)((Bd_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] = (char *)&((Bd_item *)NULL)->tr_seq_no -
				   (char *)((Bd_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Bd_item *)NULL)->tr_item_no -
				   (char *)((Bd_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt Key-1 */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Bd_item *)NULL)->tr_period -
				   (char *)((Bd_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Bd_item *)NULL)->tr_fund -
				   (char *)((Bd_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Bd_item *)NULL)->tr_reccod -
				   (char *)((Bd_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] = 18 ;
	keysarray[offset++] = (char *)&((Bd_item *)NULL)->tr_accno[0] -
				   (char *)((Bd_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt Key-2 */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Bd_item *)NULL)->tr_fund -
				   (char *)((Bd_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Bd_item *)NULL)->tr_reccod -
				   (char *)((Bd_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] = 18 ;
	keysarray[offset++] = (char *)&((Bd_item *)NULL)->tr_accno[0] -
				   (char *)((Bd_item *)NULL) ;
	keysarray[offset++] = ASCND ;
	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("STOCK MASTER FILE...\t");

	strcpy(file_desc.id_f_name, STMAST_FILE );
	strcpy(file_desc.fl_user_nm,"Stock Master File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(St_mast);
	file_desc.tot_keys = 3 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((St_mast *)NULL)->st_fund -
				   (char *)((St_mast *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] = 10 ;
	keysarray[offset++] = (char *)&((St_mast *)NULL)->st_code[0] -
				   (char *)((St_mast *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 1st alternate Key */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((St_mast *)NULL)->st_section -
				   (char *)((St_mast *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((St_mast *)NULL)->st_fund -
				   (char *)((St_mast *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] = 10 ;
	keysarray[offset++] = (char *)&((St_mast *)NULL)->st_code[0] -
				   (char *)((St_mast *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 2nd alternate Key */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((St_mast *)NULL)->st_fund -
				   (char *)((St_mast *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] = 30 ;
	keysarray[offset++] = (char *)&((St_mast *)NULL)->st_desc[0] -
				   (char *)((St_mast *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("STOCK TRANSACTION FILE...\n");

	strcpy(file_desc.id_f_name, STTRAN_FILE );
	strcpy(file_desc.fl_user_nm,"Stock Trans. File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(St_tran);
	file_desc.tot_keys = 3 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = LONG;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((St_tran *)NULL)->st_date -
				   (char *)((St_tran *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] = 2 ;
	keysarray[offset++] = (char *)&((St_tran *)NULL)->st_type[0] -
				   (char *)((St_tran *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] = (char *)&((St_tran *)NULL)->st_seq_no -
				   (char *)((St_tran *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* First alternate Key  */

	keysarray[offset++] = 3 ;	/* Parts */

	/****** part 1  removed because stock receipts and 
	 		returns have no fund
	keysarray[offset++] = SHORT ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] = (char *)&((St_tran *)NULL)->st_fund -
				   (char *)((St_tran *)NULL) ;
	keysarray[offset++] = ASCND ;
	********/
	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] = 10 ;
	keysarray[offset++] = (char *)&((St_tran *)NULL)->st_code[0] -
				   (char *)((St_tran *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = LONG;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((St_tran *)NULL)->st_date -
				   (char *)((St_tran *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] = 2 ;
	keysarray[offset++] = (char *)&((St_tran *)NULL)->st_type[0] -
				   (char *)((St_tran *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt-Key 2 */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = LONG;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((St_tran *)NULL)->st_po_no -
				   (char *)((St_tran *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] = 2 ;
	keysarray[offset++] = (char *)&((St_tran *)NULL)->st_type[0] -
				   (char *)((St_tran *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = LONG;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((St_tran *)NULL)->st_date -
				   (char *)((St_tran *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] = (char *)&((St_tran *)NULL)->st_seq_no -
				   (char *)((St_tran *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("\tALLOCATION FILE...\t");

	strcpy(file_desc.id_f_name, ALLOC_FILE );

	strcpy(file_desc.fl_user_nm,"Stock Allocation");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Alloc_rec);
	file_desc.tot_keys = 3 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] = (char *)&((Alloc_rec *)NULL)->st_fund -
				   (char *)((Alloc_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] = 10 ;
	keysarray[offset++] = (char *)&((Alloc_rec *)NULL)->st_code[0] -
				   (char *)((Alloc_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] = (char *)&((Alloc_rec *)NULL)->st_location -
				   (char *)((Alloc_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] = 18 ;
	keysarray[offset++] = (char *)&((Alloc_rec *)NULL)->st_expacc[0] -
				   (char *)((Alloc_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 1st alternate  Key */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] = (char *)&((Alloc_rec *)NULL)->st_fund -
				   (char *)((Alloc_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] = 10 ;
	keysarray[offset++] = (char *)&((Alloc_rec *)NULL)->st_code[0] -
				   (char *)((Alloc_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] = (char *)&((Alloc_rec *)NULL)->st_date -
				   (char *)((Alloc_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] = (char *)&((Alloc_rec *)NULL)->st_time -
				   (char *)((Alloc_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 2nd alternate  Key */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] = (char *)&((Alloc_rec *)NULL)->st_location -
				   (char *)((Alloc_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] = (char *)&((Alloc_rec *)NULL)->st_fund -
				   (char *)((Alloc_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] = 10 ;
	keysarray[offset++] = (char *)&((Alloc_rec *)NULL)->st_code[0] -
				   (char *)((Alloc_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("STOCK SECTION FILE...\n");

	strcpy(file_desc.id_f_name, SECTION_FILE );
	strcpy(file_desc.fl_user_nm,"Stock Section");
	file_desc.id_f_type 	= SEQ ;
	file_desc.id_io_mode 	= RWMODE ;
	file_desc.reclen   =  sizeof(St_sect) ;

	file_desc.tot_keys = 0 ;
	file_desc.keys_offset = offset ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("SUPPLIER FILE ...\t");

	strcpy(file_desc.id_f_name, SUPPLIER_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Supplier File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Supplier) ;
	file_desc.tot_keys = 3 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  10 ;
	keysarray[offset++] =  (char *)&((Supplier *)NULL)->s_supp_cd[0] -
				   (char *)((Supplier *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alternate key Number 1 */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  24 ;
	keysarray[offset++] =  (char *)&((Supplier *)NULL)->s_name[0] -
				   (char *)((Supplier *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alternate key Number 2 */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  24 ;
	keysarray[offset++] =  (char *)&((Supplier *)NULL)->s_abb[0] -
				   (char *)((Supplier *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("PURCHASE ORDER HEADER FILE ...\n");

	strcpy(file_desc.id_f_name, POHDR_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Purchase Order Hdr");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Po_hdr) ;
	file_desc.tot_keys = 2 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Po_hdr *)NULL)->ph_code -
				   (char *)((Po_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt Key 1 */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  10 ;
	keysarray[offset++] =  (char *)&((Po_hdr *)NULL)->ph_supp_cd[0] -
				   (char *)((Po_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Po_hdr *)NULL)->ph_code -
				   (char *)((Po_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("\tPURCHASE ORDER ITEMS FILE...\n");

	strcpy(file_desc.id_f_name, POITEM_FILE );
	strcpy(file_desc.fl_user_nm,"Purchase Order Item");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Po_item) ;
	file_desc.tot_keys = 3 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Po_item *)NULL)->pi_code -
				   (char *)((Po_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Po_item *)NULL)->pi_item_no -
				   (char *)((Po_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt Key 1 */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Po_item *)NULL)->pi_fund -
				   (char *)((Po_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  18 ;
	keysarray[offset++] =  (char *)&((Po_item *)NULL)->pi_acct[0] -
				   (char *)((Po_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Po_item *)NULL)->pi_code -
				   (char *)((Po_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Po_item *)NULL)->pi_item_no -
				   (char *)((Po_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt Key 2 */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  10 ;
	keysarray[offset++] =  (char *)&((Po_item *)NULL)->pi_st_code[0] -
				   (char *)((Po_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Po_item *)NULL)->pi_code -
				   (char *)((Po_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Po_item *)NULL)->pi_item_no -
				   (char *)((Po_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}
	printf ("REQUISITION HEADER FILE ...\n");

	strcpy(file_desc.id_f_name, REQHDR_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Requisition Hdr");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Req_hdr) ;
	file_desc.tot_keys = 5 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Req_hdr *)NULL)->code -
				   (char *)((Req_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt Key 1 */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Req_hdr *)NULL)->funds -
				   (char *)((Req_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Req_hdr *)NULL)->code -
				   (char *)((Req_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;
	
	/* Alt Key 2 */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  10 ;
	keysarray[offset++] =  (char *)&((Req_hdr *)NULL)->supp_cd[0] -
				   (char *)((Req_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Req_hdr *)NULL)->code -
				   (char *)((Req_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt Key 3 */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Req_hdr *)NULL)->funds -
				   (char *)((Req_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  10 ;
	keysarray[offset++] =  (char *)&((Req_hdr *)NULL)->supp_cd[0] -
				   (char *)((Req_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Req_hdr *)NULL)->code -
				   (char *)((Req_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt Key 4 */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Req_hdr *)NULL)->funds -
				   (char *)((Req_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Req_hdr *)NULL)->costcenter -
				   (char *)((Req_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Req_hdr *)NULL)->code -
				   (char *)((Req_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;
	
	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("\tREQUISITION ITEMS FILE...\n");

	strcpy(file_desc.id_f_name, REQITEM_FILE );
	strcpy(file_desc.fl_user_nm,"Requisition Item");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Req_item) ;
	file_desc.tot_keys = 4 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Req_item *)NULL)->code -
				   (char *)((Req_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Req_item *)NULL)->item_no -
				   (char *)((Req_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt Key 1 */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Req_item *)NULL)->fund -
				   (char *)((Req_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  18 ;
	keysarray[offset++] =  (char *)&((Req_item *)NULL)->acct[0] -
				   (char *)((Req_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Req_item *)NULL)->code -
				   (char *)((Req_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Req_item *)NULL)->item_no -
				   (char *)((Req_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt Key 2 */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Req_item *)NULL)->fund -
				   (char *)((Req_item *)NULL) ;
	keysarray[offset++] = ASCND ;
	
	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  11 ;
	keysarray[offset++] =  (char *)&((Req_item *)NULL)->st_code[0] -
				   (char *)((Req_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Req_item *)NULL)->code -
				   (char *)((Req_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Req_item *)NULL)->item_no -
				   (char *)((Req_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt Key 3 */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Req_item *)NULL)->code -
				   (char *)((Req_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  2 ;
	keysarray[offset++] =  (char *)&((Req_item *)NULL)->appstat[0] -
				   (char *)((Req_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  11 ;
	keysarray[offset++] =  (char *)&((Req_item *)NULL)->st_code[0] -
				   (char *)((Req_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Req_item *)NULL)->item_no -
				   (char *)((Req_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("\tREQUISITION REASON FILE...\n");

	strcpy(file_desc.id_f_name, REQREAS_FILE );
	strcpy(file_desc.fl_user_nm,"Requisition Reason");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Req_reason) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Req_reason *)NULL)->reqr_code -
				   (char *)((Req_reason *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Req_reason *)NULL)->reqr_item_no -
				   (char *)((Req_reason *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}







	printf ("\tCATEGORY FILE...\n");

	strcpy(file_desc.id_f_name, CATEGORY_FILE );
	strcpy(file_desc.fl_user_nm,"Category");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Category) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Category *)NULL)->categ_num -
				   (char *)((Category *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("\tITEM GROUP FILE...\n");

	strcpy(file_desc.id_f_name, ITEMGROUP_FILE );
	strcpy(file_desc.fl_user_nm,"Item Group");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Item_grp) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Item_grp *)NULL)->itmgrp_num -
				   (char *)((Item_grp *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("\tCATALOGUE FILE...\n");

	strcpy(file_desc.id_f_name, CATALOGUE_FILE );
	strcpy(file_desc.fl_user_nm,"Catalogue");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Catalogue) ;
	file_desc.tot_keys = 2 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Catalogue *)NULL)->cat_num -
				   (char *)((Catalogue *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt-Key 1*/

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  11 ;
	keysarray[offset++] =  (char *)&((Catalogue *)NULL)->cat_awd_supp[0] -
				   (char *)((Catalogue *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Catalogue *)NULL)->cat_num -
				   (char *)((Catalogue *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("\tPOTENTIAL BIDDER FILE...\n");

	strcpy(file_desc.id_f_name, POTBIDDER_FILE );
	strcpy(file_desc.fl_user_nm,"Potential Bidder");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(PotBidder) ;
	file_desc.tot_keys = 2 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((PotBidder *)NULL)->pb_categ_num -
				   (char *)((PotBidder *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  11 ;
	keysarray[offset++] =  (char *)&((PotBidder *)NULL)->pb_supp[0] -
				   (char *)((PotBidder *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt-key 1 */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((PotBidder *)NULL)->pb_categ_num -
				   (char *)((PotBidder *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((PotBidder *)NULL)->pb_select -
				   (char *)((PotBidder *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  11 ;
	keysarray[offset++] =  (char *)&((PotBidder *)NULL)->pb_supp[0] -
				   (char *)((PotBidder *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("\tBID FILE...\n");

	strcpy(file_desc.id_f_name, BID_FILE );
	strcpy(file_desc.fl_user_nm,"Bid File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Bid) ;
	file_desc.tot_keys = 5 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  11 ;
	keysarray[offset++] =  (char *)&((Bid *)NULL)->bid_supp_cd[0] -
				   (char *)((Bid *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Bid *)NULL)->bid_cat_num -
				   (char *)((Bid *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt Key-1 */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Bid *)NULL)->bid_tend_num -
				   (char *)((Bid *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  11 ;
	keysarray[offset++] =  (char *)&((Bid *)NULL)->bid_supp_cd[0] -
				   (char *)((Bid *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Bid *)NULL)->bid_cat_num -
				   (char *)((Bid *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt Key-2 */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Bid *)NULL)->bid_cat_num -
				   (char *)((Bid *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = DOUBLE ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Bid *)NULL)->bid_price -
				   (char *)((Bid *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  11 ;
	keysarray[offset++] =  (char *)&((Bid *)NULL)->bid_supp_cd[0] -
				   (char *)((Bid *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt Key-3 */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Bid *)NULL)->bid_cat_num -
				   (char *)((Bid *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  4 ;
	keysarray[offset++] =  (char *)&((Bid *)NULL)->bid_status[0] -
				   (char *)((Bid *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt Key-4 */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Bid *)NULL)->bid_cat_num -
				   (char *)((Bid *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  11 ;
	keysarray[offset++] =  (char *)&((Bid *)NULL)->bid_supp_cd[0] -
				   (char *)((Bid *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("\tTENDER HISTORY FILE...\n");

	strcpy(file_desc.id_f_name, TENDHIST_FILE );
	strcpy(file_desc.fl_user_nm,"Tender Hisory");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Tend_Hist) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tend_Hist *)NULL)->th_cat_num -
				   (char *)((Tend_Hist *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}










	printf ("FA MASTER FILE ...\t");

	strcpy(file_desc.id_f_name, FAMAST_FILE ) ;
	strcpy(file_desc.fl_user_nm,"FA Master File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Fa_rec) ;
	file_desc.tot_keys = 6 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Fa_rec *)NULL)->fa_costcen -
				   (char *)((Fa_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Fa_rec *)NULL)->fa_itemid -
				   (char *)((Fa_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alternate Key 1 */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  4 ;
	keysarray[offset++] =  (char *)((Fa_rec *)NULL)->fa_type -
				   (char *)((Fa_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Fa_rec *)NULL)->fa_costcen -
				   (char *)((Fa_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Fa_rec *)NULL)->fa_itemid -
				   (char *)((Fa_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alternate Key 2 */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  4 ;
	keysarray[offset++] =  (char *)((Fa_rec *)NULL)->fa_dept -
				   (char *)((Fa_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  4 ;
	keysarray[offset++] =  (char *)((Fa_rec *)NULL)->fa_type -
				   (char *)((Fa_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Fa_rec *)NULL)->fa_costcen -
				   (char *)((Fa_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Fa_rec *)NULL)->fa_itemid -
				   (char *)((Fa_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alternate Key 3 */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Fa_rec *)NULL)->fa_curcostcen -
				   (char *)((Fa_rec *)NULL);
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Fa_rec *)NULL)->fa_costcen -
				   (char *)((Fa_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Fa_rec *)NULL)->fa_itemid -
				   (char *)((Fa_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alternate Key 4 */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Fa_rec *)NULL)->fa_curcostcen -
				   (char *)((Fa_rec *)NULL);
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  5 ;
	keysarray[offset++] =  (char *)&((Fa_rec *)NULL)->fa_roomno[0] -
				   (char *)((Fa_rec *)NULL);
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Fa_rec *)NULL)->fa_itemid -
				   (char *)((Fa_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alternate Key 5 */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Fa_rec *)NULL)->fa_costcen -
				   (char *)((Fa_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  DESC_KEY ;
	keysarray[offset++] =  (char *)&((Fa_rec *)NULL)->fa_desc[0] -
				   (char *)((Fa_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("FA ITEM TYPE FILE...\n");

	strcpy(file_desc.id_f_name, FATYPE_FILE ) ;
	strcpy(file_desc.fl_user_nm,"FA Item Types File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Fa_type) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  4 ;
	keysarray[offset++] =  (char *)((Fa_type *)NULL)->code -
				   (char *)((Fa_type *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("\tFA DEPT CODE FILE...\t");

	strcpy(file_desc.id_f_name, FADEPT_FILE ) ;
	strcpy(file_desc.fl_user_nm,"FA Dept Code File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Fa_dept) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  4 ;
	keysarray[offset++] =  (char *)((Fa_dept *)NULL)->code -
				   (char *)((Fa_dept *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("FATRAN FILE...\n");

	strcpy(file_desc.id_f_name, FATRAN_FILE ) ;
	strcpy(file_desc.fl_user_nm,"FA Tranfers File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Fa_transfer) ;
	file_desc.tot_keys = 4;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] =  (char *)&((Fa_transfer *)NULL)->fatr_numb -
				   (char *)((Fa_transfer *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alternate key 1 */
	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] =  (char *)&((Fa_transfer *)NULL)->fatr_costcen -
				   (char *)((Fa_transfer *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] =  (char *)&((Fa_transfer *)NULL)->fatr_itemid -
				   (char *)((Fa_transfer *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] =  (char *)&((Fa_transfer *)NULL)->fatr_numb -
				   (char *)((Fa_transfer *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alternate key 2 */
	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] =  (char *)&((Fa_transfer *)NULL)->fatr_tocostcen -
				   (char *)((Fa_transfer *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] =  (char *)&((Fa_transfer *)NULL)->fatr_numb -
				   (char *)((Fa_transfer *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alternate key 3 */

	keysarray[offset++] = 3 ;	/* parts */

	/* part 1 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] = 1 ;
 	keysarray[offset++] =  (char *)&((Fa_transfer *)NULL)->fatr_date -
				   (char *)((Fa_transfer *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] =  (char *)&((Fa_transfer *)NULL)->fatr_costcen -
				   (char *)((Fa_transfer *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] =  (char *)&((Fa_transfer *)NULL)->fatr_numb -
				   (char *)((Fa_transfer *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("CUSTOMER FILE...\t");

	strcpy(file_desc.id_f_name, CUST_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Customer File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Cu_rec) ;
	file_desc.tot_keys = 3 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  10 ;
	keysarray[offset++] =  (char *)((Cu_rec *)NULL)->cu_code -
				   (char *)((Cu_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alternate key 1 */

	keysarray[offset++] = 1;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] = 31 ;
	keysarray[offset++] = (char *)((Cu_rec *)NULL)->cu_name -
				  (char *)((Cu_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alternate key 2 */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  31 ;
	keysarray[offset++] =  (char *)((Cu_rec *)NULL)->cu_abrev -
				   (char *)((Cu_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("SALES HEADER FILE...\n");

	strcpy(file_desc.id_f_name, ARSHDR_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Sales Header File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Ar_hdr) ;
	file_desc.tot_keys = 3 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Ar_hdr *)NULL)->ah_fund -
				   (char *)((Ar_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Ar_hdr *)NULL)->ah_inv_no -
				   (char *)((Ar_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Ar_hdr *)NULL)->ah_sno -
				   (char *)((Ar_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt Key 1 */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  10 ;
	keysarray[offset++] =  (char *)((Ar_hdr *)NULL)->ah_cu_code -
				   (char *)((Ar_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Ar_hdr *)NULL)->ah_fund -
				   (char *)((Ar_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Ar_hdr *)NULL)->ah_inv_no -
				   (char *)((Ar_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Ar_hdr *)NULL)->ah_sno -
				   (char *)((Ar_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt Key 2 */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Ar_hdr *)NULL)->ah_trandt -
				   (char *)((Ar_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Ar_hdr *)NULL)->ah_fund -
				   (char *)((Ar_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Ar_hdr *)NULL)->ah_inv_no -
				   (char *)((Ar_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Ar_hdr *)NULL)->ah_sno -
				   (char *)((Ar_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("\tSALES ITEMS FILE...\t");

	strcpy(file_desc.id_f_name, ARSITEM_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Sales Items File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Ar_item) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Ar_item *)NULL)->ai_fund -
				   (char *)((Ar_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Ar_item *)NULL)->ai_inv_no -
				   (char *)((Ar_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Ar_item *)NULL)->ai_hno -
				   (char *)((Ar_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Ar_item *)NULL)->ai_sno -
				   (char *)((Ar_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("RECEIPTS HEADER FILE...\n");

	strcpy(file_desc.id_f_name, RCPTHDR_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Receipts Header File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Rcpt_hdr) ;
	file_desc.tot_keys = 3 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Rcpt_hdr *)NULL)->rhdr_refno -
				   (char *)((Rcpt_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt Key 1 */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Rcpt_hdr *)NULL)->rhdr_rcptdate -
				   (char *)((Rcpt_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Rcpt_hdr *)NULL)->rhdr_fund -
				   (char *)((Rcpt_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Rcpt_hdr *)NULL)->rhdr_refno -
				   (char *)((Rcpt_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt Key 2 */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  10 ;
	keysarray[offset++] =  (char *)((Rcpt_hdr *)NULL)->rhdr_cust -
				   (char *)((Rcpt_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Rcpt_hdr *)NULL)->rhdr_fund -
				   (char *)((Rcpt_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Rcpt_hdr *)NULL)->rhdr_refno -
				   (char *)((Rcpt_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("RECEIPTS ITEM FILE...\n");

	strcpy(file_desc.id_f_name, RCPTITEM_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Receipts Item File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Rcpt_item) ;
	file_desc.tot_keys = 2 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Rcpt_item *)NULL)->ritm_refno -
				   (char *)((Rcpt_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Rcpt_item *)NULL)->ritm_seqno -
				   (char *)((Rcpt_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt Key-1 */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  10 ;
	keysarray[offset++] =  (char *)&((Rcpt_item *)NULL)->ritm_cust[0] -
				   (char *)((Rcpt_item *)NULL) ;
	keysarray[offset++] = ASCND ;
	
	/* part 2 */	
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Rcpt_item *)NULL)->ritm_invnumb -
				   (char *)((Rcpt_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Rcpt_item *)NULL)->ritm_refno -
				   (char *)((Rcpt_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Rcpt_item *)NULL)->ritm_seqno -
				   (char *)((Rcpt_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("PURCHASE INVOICES FILE ...\t");

	strcpy(file_desc.id_f_name, APINVOICE_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Purchase Invoices");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Invoice) ;
	file_desc.tot_keys = 3 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  10 ;
	keysarray[offset++] =  (char *)&((Invoice *)NULL)->in_supp_cd[0] -
				   (char *)((Invoice *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  15 ;
	keysarray[offset++] =  (char *)&((Invoice *)NULL)->in_invc_no[0] -
				   (char *)((Invoice *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  2 ;
	keysarray[offset++] =  (char *)&((Invoice *)NULL)->in_tr_type[0] -
				   (char *)((Invoice *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt Key1 */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Invoice *)NULL)->in_funds -
				   (char *)((Invoice *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  10 ;
	keysarray[offset++] =  (char *)&((Invoice *)NULL)->in_supp_cd[0] -
				   (char *)((Invoice *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  15 ;
	keysarray[offset++] =  (char *)&((Invoice *)NULL)->in_invc_no[0] -
				   (char *)((Invoice *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  2 ;
	keysarray[offset++] =  (char *)&((Invoice *)NULL)->in_tr_type[0] -
				   (char *)((Invoice *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt-Key 2 */

	keysarray[offset++] = 5 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Invoice *)NULL)->in_funds -
				   (char *)((Invoice *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  10 ;
	keysarray[offset++] =  (char *)&((Invoice *)NULL)->in_supp_cd[0] -
				   (char *)((Invoice *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Invoice *)NULL)->in_chq_no -
				   (char *)((Invoice *)NULL) ;
	keysarray[offset++] = ASCND ;


	/* part 4 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  15 ;
	keysarray[offset++] =  (char *)&((Invoice *)NULL)->in_invc_no[0] -
				   (char *)((Invoice *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 5 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  2 ;
	keysarray[offset++] =  (char *)&((Invoice *)NULL)->in_tr_type[0] -
				   (char *)((Invoice *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("PURCHASE INVOICE HEADER FILE ...\n");

	strcpy(file_desc.id_f_name, APINHDR_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Pur. Invoice Hdr");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(In_hdr) ;
	file_desc.tot_keys = 2 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  4 ;
	keysarray[offset++] =  (char *)&((In_hdr *)NULL)->h_term[0] -
				   (char *)((In_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((In_hdr *)NULL)->h_batch -
				   (char *)((In_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((In_hdr *)NULL)->h_sno -
				   (char *)((In_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt Key1 */

	keysarray[offset++] = 7 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  4 ;
	keysarray[offset++] =  (char *)&((In_hdr *)NULL)->h_term[0] -
				   (char *)((In_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((In_hdr *)NULL)->h_period -
				   (char *)((In_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((In_hdr *)NULL)->h_funds -
				   (char *)((In_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  10 ;
	keysarray[offset++] =  (char *)&((In_hdr *)NULL)->h_supp_cd[0] -
				   (char *)((In_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 5 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  15 ;
	keysarray[offset++] =  (char *)&((In_hdr *)NULL)->h_invc_no[0] -
				   (char *)((In_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 6 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  2 ;
	keysarray[offset++] =  (char *)&((In_hdr *)NULL)->h_tr_type[0] -
				   (char *)((In_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 7 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((In_hdr *)NULL)->h_sno -
				   (char *)((In_hdr *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("\tPURCHASE INVOICE ITEMS FILE...\t");

	strcpy(file_desc.id_f_name, APINITEM_FILE );
	strcpy(file_desc.fl_user_nm,"Pur. Invoice Item");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(In_item) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  10 ;
	keysarray[offset++] =  (char *)&((In_item *)NULL)->i_supp_cd[0] -
				   (char *)((In_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  15 ;
	keysarray[offset++] =  (char *)&((In_item *)NULL)->i_invc_no[0] -
				   (char *)((In_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  2 ;
	keysarray[offset++] =  (char *)&((In_item *)NULL)->i_tr_type[0] -
				   (char *)((In_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((In_item *)NULL)->i_item_no -
				   (char *)((In_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("CHEQUE FILE ...\n");

	strcpy(file_desc.id_f_name, CHEQUE_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Cheque File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Chq_rec) ;
	file_desc.tot_keys = 2 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 5 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  10 ;
	keysarray[offset++] =  (char *)&((Chq_rec *)NULL)->c_supp_cd[0] -
				   (char *)((Chq_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Chq_rec *)NULL)->c_funds -
				   (char *)((Chq_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Chq_rec *)NULL)->c_chq_no -
				   (char *)((Chq_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  15 ;
	keysarray[offset++] =  (char *)&((Chq_rec *)NULL)->c_invc_no[0] -
				   (char *)((Chq_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 5 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  2 ;
	keysarray[offset++] =  (char *)&((Chq_rec *)NULL)->c_tr_type[0] -
				   (char *)((Chq_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt Key 1 */

	keysarray[offset++] = 6 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Chq_rec *)NULL)->c_funds -
				   (char *)((Chq_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  18 ;
	keysarray[offset++] =  (char *)&((Chq_rec *)NULL)->c_accno[0] -
				   (char *)((Chq_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Chq_rec *)NULL)->c_chq_no -
				   (char *)((Chq_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  10 ;
	keysarray[offset++] =  (char *)&((Chq_rec *)NULL)->c_supp_cd[0] -
				   (char *)((Chq_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 5 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  15 ;
	keysarray[offset++] =  (char *)&((Chq_rec *)NULL)->c_invc_no[0] -
				   (char *)((Chq_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 6 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  2 ;
	keysarray[offset++] =  (char *)&((Chq_rec *)NULL)->c_tr_type[0] -
				   (char *)((Chq_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("\tCHEQUE HISTORY FILE ...\t");

	strcpy(file_desc.id_f_name, CHQHIST_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Cheque History File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Chq_hist) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Chq_hist *)NULL)->ch_funds -
				   (char *)((Chq_hist *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  18 ;
	keysarray[offset++] =  (char *)&((Chq_hist *)NULL)->ch_accno[0] -
				   (char *)((Chq_hist *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Chq_hist *)NULL)->ch_chq_no -
				   (char *)((Chq_hist *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("CHEQUE REGISTER FILE ...\n");

	strcpy(file_desc.id_f_name, CHQREG_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Cheque Register File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Reg_rec) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Reg_rec *)NULL)->cr_funds -
				   (char *)((Reg_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Reg_rec *)NULL)->cr_chq_no -
				   (char *)((Reg_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}


/* xxxx 		chgs.  here		 */

	printf ("\tAP HISTORY FILE ...\n");

	strcpy(file_desc.id_f_name, APHIST_FILE ) ;
	strcpy(file_desc.fl_user_nm,"AP History File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Ap_hist) ;
	file_desc.tot_keys = 4 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  10 ;
	keysarray[offset++] =  (char *)&((Ap_hist *)NULL)->a_supp_cd[0] -
				   (char *)((Ap_hist *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  15 ;
	keysarray[offset++] =  (char *)&((Ap_hist *)NULL)->a_invc_no[0] -
				   (char *)((Ap_hist *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  2 ;
	keysarray[offset++] =  (char *)&((Ap_hist *)NULL)->a_tr_type[0] -
				   (char *)((Ap_hist *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Ap_hist *)NULL)->a_sno -
				   (char *)((Ap_hist *)NULL) ;
	keysarray[offset++] = ASCND ;


	/* Alternate Key 1 */

	keysarray[offset++] = 5 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Ap_hist *)NULL)->a_period -
				   (char *)((Ap_hist *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  10 ;
	keysarray[offset++] =  (char *)&((Ap_hist *)NULL)->a_supp_cd[0] -
				   (char *)((Ap_hist *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  15 ;
	keysarray[offset++] =  (char *)&((Ap_hist *)NULL)->a_invc_no[0] -
				   (char *)((Ap_hist *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  2 ;
	keysarray[offset++] =  (char *)&((Ap_hist *)NULL)->a_tr_type[0] -
				   (char *)((Ap_hist *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 5 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Ap_hist *)NULL)->a_sno -
				   (char *)((Ap_hist *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alternate Key 2 */

	keysarray[offset++] = 5 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] = (char *)&((Ap_hist *)NULL)->a_tr_date -
				   (char *)((Ap_hist *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  10 ;
	keysarray[offset++] =  (char *)&((Ap_hist *)NULL)->a_supp_cd[0] -
				   (char *)((Ap_hist *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  15 ;
	keysarray[offset++] =  (char *)&((Ap_hist *)NULL)->a_invc_no[0] -
				   (char *)((Ap_hist *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  2 ;
	keysarray[offset++] =  (char *)&((Ap_hist *)NULL)->a_tr_type[0] -
				   (char *)((Ap_hist *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 5 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Ap_hist *)NULL)->a_sno -
				   (char *)((Ap_hist *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alternate key 4 */
	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  10 ;
	keysarray[offset++] =  (char *)&((Ap_hist *)NULL)->a_supp_cd[0] -
				   (char *)((Ap_hist *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] = (char *)&((Ap_hist *)NULL)->a_chq_no -
				   (char *)((Ap_hist *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Ap_hist *)NULL)->a_sno -
				   (char *)((Ap_hist *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}
	printf ("LASTPO FILE...\t");

	strcpy(file_desc.id_f_name, LASTPO_FILE );
	strcpy(file_desc.fl_user_nm,"Last Po");
	file_desc.id_f_type 	= SEQ ;
	file_desc.id_io_mode 	= RWMODE ;
	file_desc.reclen   =  sizeof(Last_po) ;

	file_desc.tot_keys = 0 ;
	file_desc.keys_offset = offset ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("LASTREQ FILE...\t");

	strcpy(file_desc.id_f_name, LASTREQ_FILE );
	strcpy(file_desc.fl_user_nm,"Last Requistion");
	file_desc.id_f_type 	= SEQ ;
	file_desc.id_io_mode 	= RWMODE ;
	file_desc.reclen   =  sizeof(Last_req) ;

	file_desc.tot_keys = 0 ;
	file_desc.keys_offset = offset ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

/*      *****    main and alternate keys for the personnel/payroll   ***** */

	printf ("DEPARTMENT FILE ...\n");

	strcpy(file_desc.id_f_name, DEPARTMENT_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Department File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Dept) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Dept *)NULL)->d_code[0] -
				   (char *)((Dept *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("AREA FILE ...\n");

	strcpy(file_desc.id_f_name, AREA_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Area File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Area) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Area *)NULL)->a_deptcode[0] -
				   (char *)((Area *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Area *)NULL)->a_code[0] -
				   (char *)((Area *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("POSITION FILE ...\n");

	strcpy(file_desc.id_f_name, POSITION_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Position File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Position) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Position *)NULL)->p_code[0] -
				   (char *)((Position *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("CLASSIFICATION FILE ...\n");

	strcpy(file_desc.id_f_name, CLASS_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Classification File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Class) ;
	file_desc.tot_keys = 3 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Class *)NULL)->c_code[0] -
				   (char *)((Class *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Class *)NULL)->c_date -
				   (char *)((Class *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt Key 1 */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Class *)NULL)->c_date -
				   (char *)((Class *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Class *)NULL)->c_pos[0] -
				   (char *)((Class *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Class *)NULL)->c_code[0] -
				   (char *)((Class *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt Key 3 */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Class *)NULL)->c_date -
				   (char *)((Class *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Class *)NULL)->c_code[0] -
				   (char *)((Class *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}


	printf ("CLASSIFICATION ITEM FILE ...\n");

	strcpy(file_desc.id_f_name, CLASS_ITEM_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Class Item File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Class_item) ;
	file_desc.tot_keys = 2 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Class_item *)NULL)->ci_code[0] -
				   (char *)((Class_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Class_item *)NULL)->ci_date -
				   (char *)((Class_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Class_item *)NULL)->ci_fund -
				   (char *)((Class_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 1st Alternate Key */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Class_item *)NULL)->ci_code[0] -
				   (char *)((Class_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Class_item *)NULL)->ci_fund -
				   (char *)((Class_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Class_item *)NULL)->ci_date -
				   (char *)((Class_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("BARGAINING UNIT FILE ...\n");

	strcpy(file_desc.id_f_name, BARG_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Bargaining Unit File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Barg_unit) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Barg_unit *)NULL)->b_code[0] -
				   (char *)((Barg_unit *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Barg_unit *)NULL)->b_date -
				   (char *)((Barg_unit *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}


	printf ("BANK FILE ...\n");

	strcpy(file_desc.id_f_name, BANK_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Bank File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Bank) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Bank *)NULL)->bk_numb[0] -
				   (char *)((Bank *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("PAY PERIOD FILE ...\n");

	strcpy(file_desc.id_f_name, PAY_PERIOD_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Pay Period File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Pay_per) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Pay_per *)NULL)->pp_code[0] -
				   (char *)((Pay_per *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Pay_per *)NULL)->pp_year -
				   (char *)((Pay_per *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("PAY PERIOD ITEM FILE ...\n");

	strcpy(file_desc.id_f_name, PAY_PER_IT_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Pay Period Item File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Pay_per_it) ;
	file_desc.tot_keys = 4 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Pay_per_it *)NULL)->ppi_code[0] -
				   (char *)((Pay_per_it *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Pay_per_it *)NULL)->ppi_year -
				   (char *)((Pay_per_it *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Pay_per_it *)NULL)->ppi_numb -
				   (char *)((Pay_per_it *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt Key 1*/

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Pay_per_it *)NULL)->ppi_code[0] -
				   (char *)((Pay_per_it *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Pay_per_it *)NULL)->ppi_st_date -
				   (char *)((Pay_per_it *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt Key 2*/

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] =  SHORT;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Pay_per_it *)NULL)->ppi_year -
				   (char *)((Pay_per_it *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Pay_per_it *)NULL)->ppi_st_date -
				   (char *)((Pay_per_it *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt Key 3 */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Pay_per_it *)NULL)->ppi_code[0] -
				   (char *)((Pay_per_it *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Pay_per_it *)NULL)->ppi_numb -
				   (char *)((Pay_per_it *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3  */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Pay_per_it *)NULL)->ppi_year -
				   (char *)((Pay_per_it *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("UIC TABLE FILE ...\n");

	strcpy(file_desc.id_f_name, UIC_FILE ) ;
	strcpy(file_desc.fl_user_nm,"UIC Table File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Uic) ;
	file_desc.tot_keys = 2 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Uic *)NULL)->ui_numb -
				   (char *)((Uic *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Uic *)NULL)->ui_date -
				   (char *)((Uic *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt Key 1 */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 2 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Uic *)NULL)->ui_date -
				   (char *)((Uic *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Uic *)NULL)->ui_numb -
				   (char *)((Uic *)NULL) ;
	keysarray[offset++] = ASCND ;


	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("TAX TABLE FILE ...\n");

	strcpy(file_desc.id_f_name, TAX_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Tax Table File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Tax) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tax *)NULL)->tx_date -
				   (char *)((Tax *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = DOUBLE ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tax *)NULL)->tx_low_amnt -
				   (char *)((Tax *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = DOUBLE ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tax *)NULL)->tx_high_amnt -
				   (char *)((Tax *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("CERTIFICATION TABLE FILE ...\n");

	strcpy(file_desc.id_f_name, CERT_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Certification File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Cert) ;
	file_desc.tot_keys = 2 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Cert *)NULL)->cr_code[0] -
				   (char *)((Cert *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Cert *)NULL)->cr_date -
				   (char *)((Cert *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Part 3 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Cert *)NULL)->cr_level -
				   (char *)((Cert *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt Key */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Cert *)NULL)->cr_date -
				   (char *)((Cert *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Cert *)NULL)->cr_code[0] -
				   (char *)((Cert *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Part 3 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Cert *)NULL)->cr_level -
				   (char *)((Cert *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("EARNINGS FILE ...\n");

	strcpy(file_desc.id_f_name, EARN_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Earnings File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Earn) ;
	file_desc.tot_keys = 2 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Earn *)NULL)->ea_date -
				   (char *)((Earn *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Earn *)NULL)->ea_code[0] -
				   (char *)((Earn *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 1st Alternate Key */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Earn *)NULL)->ea_code[0] -
				   (char *)((Earn *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Earn *)NULL)->ea_date -
				   (char *)((Earn *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}


	printf ("TRANSACTION CODE FILE ...\n");

	strcpy(file_desc.id_f_name, TRANS_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Transaction File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Trans) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Trans *)NULL)->tr_code[0] -
				   (char *)((Trans *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("TRANSACTION ITEM FILE ...\n");

	strcpy(file_desc.id_f_name, TRANS_ITEM_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Transaction Item File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Trans_item) ;
	file_desc.tot_keys = 2;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Trans_item *)NULL)->tri_code[0] -
				   (char *)((Trans_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Trans_item *)NULL)->tri_class[0] -
				   (char *)((Trans_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Trans_item *)NULL)->tri_earn[0] -
				   (char *)((Trans_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 1st Alternate Key */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Trans_item *)NULL)->tri_class[0] -
				   (char *)((Trans_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Trans_item *)NULL)->tri_earn[0] -
				   (char *)((Trans_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("EXPENSE FILE ...\n");

	strcpy(file_desc.id_f_name, EXPENSE_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Expense File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Exp) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Exp *)NULL)->ex_code[0] -
				   (char *)((Exp *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("EXPENSE ITEM FILE ...\n");

	strcpy(file_desc.id_f_name, EXP_ITEM_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Expense Item File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Exp_item) ;
	file_desc.tot_keys = 2 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Exp_item *)NULL)->exi_code[0] -
				   (char *)((Exp_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Exp_item *)NULL)->exi_class[0] -
				   (char *)((Exp_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Exp_item *)NULL)->exi_earn[0] -
				   (char *)((Exp_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 1st Alternate Key */
	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Exp_item *)NULL)->exi_class[0] -
				   (char *)((Exp_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Exp_item *)NULL)->exi_earn[0] -
				   (char *)((Exp_item *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("TERMINATION FILE ...\n");

	strcpy(file_desc.id_f_name, TERM_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Termination File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Term) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  2 ;
	keysarray[offset++] =  (char *)&((Term *)NULL)->t_code[0] -
				   (char *)((Term *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("STATUTORY HOLIDAY FILE ...\n");

	strcpy(file_desc.id_f_name, STAT_HOL_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Stat Holiday File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Stat) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Stat *)NULL)->s_code[0] -
				   (char *)((Stat *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Stat *)NULL)->s_date -
				   (char *)((Stat *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("ATTENDANCE FILE ...\n");

	strcpy(file_desc.id_f_name, ATT_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Attendance File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Att) ;
	file_desc.tot_keys = 2 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Att *)NULL)->at_disp_code[0] -
				   (char *)((Att *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  3 ;
	keysarray[offset++] =  (char *)&((Att *)NULL)->at_code[0] -
				   (char *)((Att *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 1st Alternate Key */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  3 ;
	keysarray[offset++] =  (char *)&((Att *)NULL)->at_code[0] -
				   (char *)((Att *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("INACTIVATION FILE ...\n");

	strcpy(file_desc.id_f_name, INACT_CODE_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Inactivation File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Inact) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  3 ;
	keysarray[offset++] =  (char *)&((Inact *)NULL)->i_code[0] -
				   (char *)((Inact *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("AREA OF SPECIALIZATION FILE ...\n");

	strcpy(file_desc.id_f_name, AREA_SPEC_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Area of Special File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Area_spec) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Area_spec *)NULL)->ar_code[0] -
				   (char *)((Area_spec *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("CSB/LOAN FILE ...\n");

	strcpy(file_desc.id_f_name, LOAN_FILE ) ;
	strcpy(file_desc.fl_user_nm,"CSB/Loan File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Csb_loan) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Csb_loan *)NULL)->cs_code[0] -
				   (char *)((Csb_loan *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("PERS/PAY PARAMETER FILE ...\n");

	strcpy(file_desc.id_f_name, PAY_PARAM_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Payrol Parameter File");
	file_desc.id_f_type 	= SEQ ;
	file_desc.id_io_mode 	= RWMODE ;
	file_desc.reclen   =  sizeof(Pay_param) ;

	file_desc.tot_keys = 0 ;
	file_desc.keys_offset = offset ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("SALARY G/L ACCOUNT FILE ...\n");

	strcpy(file_desc.id_f_name, SALARY_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Salary G/L Acct File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Salary) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Salary *)NULL)->sa_fund -
				   (char *)((Salary *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Salary *)NULL)->sa_class[0] -
				   (char *)((Salary *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Salary *)NULL)->sa_earn[0] -
				   (char *)((Salary *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("BENEFIT FILE ...\n");

	strcpy(file_desc.id_f_name, BENEFIT_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Benefit File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Benefit) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Benefit *)NULL)->bn_code[0] -
				   (char *)((Benefit *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Benefit *)NULL)->bn_pp_code[0] -
				   (char *)((Benefit *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}


	printf ("BENEFIT CATEGORY FILE ...\n");

	strcpy(file_desc.id_f_name, BEN_CAT_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Benefit Category File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Ben_cat) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Ben_cat *)NULL)->bc_cat_code[0] -
				   (char *)((Ben_cat *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Ben_cat *)NULL)->bc_code[0] -
				   (char *)((Ben_cat *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Ben_cat *)NULL)->bc_pp_code[0] -
				   (char *)((Ben_cat *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("DEDUCTION FILE ...\n");

	strcpy(file_desc.id_f_name, DEDUCTION_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Deduction File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Deduction) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Deduction *)NULL)->dd_code[0] -
				   (char *)((Deduction *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Deduction *)NULL)->dd_pp_code[0] -
				   (char *)((Deduction *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

 	printf ("DEDUCTION GROUP FILE ...\n");

	strcpy(file_desc.id_f_name, DED_GRP_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Deduction Group File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Ded_group) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Ded_group *)NULL)->dg_code[0] -
				   (char *)((Ded_group *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Ded_group *)NULL)->dg_pp_code[0] -
				   (char *)((Ded_group *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Ded_group *)NULL)->dg_group[0] -
				   (char *)((Ded_group *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("DEDUCTION CATEGORY FILE ...\n");

	strcpy(file_desc.id_f_name, DED_CAT_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Deduct Category File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Ded_cat) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Ded_cat *)NULL)->dc_cat_code[0] -
				   (char *)((Ded_cat *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Ded_cat *)NULL)->dc_code[0] -
				   (char *)((Ded_cat *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("REGISTERED PENSION PLAN FILE ...\n");

	strcpy(file_desc.id_f_name, REG_PEN_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Reg Pension Plan File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Reg_pen) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Reg_pen *)NULL)->rg_code[0] -
				   (char *)((Reg_pen *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Reg_pen *)NULL)->rg_pp_code[0] -
				   (char *)((Reg_pen *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("EMPLOYEE FILE ...\n");

	strcpy(file_desc.id_f_name, EMPLOYEE_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Employee File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Emp) ;
	file_desc.tot_keys = 7 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Emp *)NULL)->em_numb[0] -
				   (char *)((Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 1st Alternate Key */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Emp *)NULL)->em_ben_cat[0] -
				   (char *)((Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 2nd Alternate Key */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Emp *)NULL)->em_pos[0] -
				   (char *)((Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt Key 3 */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  9 ;
	keysarray[offset++] =  (char *)&((Emp *)NULL)->em_sin[0] -
				   (char *)((Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Emp *)NULL)->em_numb[0] -
				   (char *)((Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt Key 4 */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Emp *)NULL)->em_reg_pen[0] -
				   (char *)((Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Emp *)NULL)->em_numb[0] -
				   (char *)((Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 5nd Alternate Key */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Emp *)NULL)->em_barg[0] -
				   (char *)((Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 6th Alternate Key */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  25 ;
	keysarray[offset++] =  (char *)&((Emp *)NULL)->em_last_name[0] -
				   (char *)((Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  15 ;
	keysarray[offset++] =  (char *)&((Emp *)NULL)->em_first_name[0] -
				   (char *)((Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("EMPLOYEE EMPLOYMENT FILE ...\n");

	strcpy(file_desc.id_f_name, EMP_EMP_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Empl Employment File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Emp_emp) ;
	file_desc.tot_keys = 2 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Emp_emp *)NULL)->ep_numb[0] -
				   (char *)((Emp_emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt-Key-1 */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Emp_emp *)NULL)->ep_class[0] -
				   (char *)((Emp_emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Emp_emp *)NULL)->ep_numb[0] -
				   (char *)((Emp_emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("EMPLOYEE SCHEDULE1 FILE ...\n");

	strcpy(file_desc.id_f_name, EMP_SCHED1_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Empl Schedule1 File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Emp_sched1) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 5 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Emp_sched1 *)NULL)->es_numb[0] -
				   (char *)((Emp_sched1 *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Emp_sched1 *)NULL)->es_week -
				   (char *)((Emp_sched1 *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Emp_sched1 *)NULL)->es_fund -
				   (char *)((Emp_sched1 *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Emp_sched1 *)NULL)->es_class[0] -
				   (char *)((Emp_sched1 *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 5 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Emp_sched1 *)NULL)->es_cost -
				   (char *)((Emp_sched1 *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("EMPLOYEE SCHEDULE2 FILE ...\n");

	strcpy(file_desc.id_f_name, EMP_SCHED2_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Empl Schedule2 File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Emp_sched2) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Emp_sched2 *)NULL)->es2_numb[0] -
				   (char *)((Emp_sched2 *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Emp_sched2 *)NULL)->es2_week -
				   (char *)((Emp_sched2 *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("EMPLOYEE SCHEDULE EXTRA FILE ...\n");

	strcpy(file_desc.id_f_name, EMP_EXTRA_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Empl Sched Extra File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Emp_extra) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Emp_extra *)NULL)->ee_numb[0] -
				   (char *)((Emp_extra *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Emp_extra *)NULL)->ee_type[0] -
				   (char *)((Emp_extra *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Emp_extra *)NULL)->ee_class[0] -
				   (char *)((Emp_extra *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("EMPLOYEE EARNINGS HISTORY FILE ...\n");

	strcpy(file_desc.id_f_name, EMP_EARN_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Empl Earn Hist File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Emp_earn) ;
	file_desc.tot_keys = 2 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Emp_earn *)NULL)->en_numb[0] -
				   (char *)((Emp_earn *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Emp_earn *)NULL)->en_date -
				   (char *)((Emp_earn *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Emp_earn *)NULL)->en_pp -
				   (char *)((Emp_earn *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Emp_earn *)NULL)->en_week -
				   (char *)((Emp_earn *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 1st Alternate Key */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Emp_earn *)NULL)->en_numb[0] -
				   (char *)((Emp_earn *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Emp_earn *)NULL)->en_year -
				   (char *)((Emp_earn *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Emp_earn *)NULL)->en_pp -
				   (char *)((Emp_earn *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Emp_earn *)NULL)->en_week -
				   (char *)((Emp_earn *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("EMPLOYEE INSURABLE EARNINGS FILE ...\n");

	strcpy(file_desc.id_f_name, EMP_INS_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Empl Insur Earn File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Emp_ins) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Emp_ins *)NULL)->in_numb[0] -
				   (char *)((Emp_ins *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Emp_ins *)NULL)->in_pp -
				   (char *)((Emp_ins *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Emp_ins *)NULL)->in_date -
				   (char *)((Emp_ins *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("EMPLOYEE BENEFIT FILE ...\n");

	strcpy(file_desc.id_f_name, EMP_BEN_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Employee Benefit File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Emp_ben) ;
	file_desc.tot_keys = 2 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Emp_ben *)NULL)->eb_numb[0] -
				   (char *)((Emp_ben *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Emp_ben *)NULL)->eb_code[0] -
				   (char *)((Emp_ben *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Emp_ben *)NULL)->eb_pp_code[0] -
				   (char *)((Emp_ben *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 1st alternate Key */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Emp_ben *)NULL)->eb_code[0] -
				   (char *)((Emp_ben *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Emp_ben *)NULL)->eb_pp_code[0] -
				   (char *)((Emp_ben *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Emp_ben *)NULL)->eb_numb[0] -
				   (char *)((Emp_ben *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("EMPLOYEE BENEFIT HISTORY FILE ...\n");

	strcpy(file_desc.id_f_name, EMP_BN_HIS_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Emp Benefit Hist File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Emp_bh) ;
	file_desc.tot_keys = 2 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Emp_bh *)NULL)->ebh_numb[0] -
				   (char *)((Emp_bh *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Emp_bh *)NULL)->ebh_pp -
				   (char *)((Emp_bh *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Emp_bh *)NULL)->ebh_date -
				   (char *)((Emp_bh *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Emp_bh *)NULL)->ebh_code[0] -
				   (char *)((Emp_bh *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt Key */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Emp_bh *)NULL)->ebh_numb[0] -
				   (char *)((Emp_bh *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Emp_bh *)NULL)->ebh_code[0] -
				   (char *)((Emp_bh *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Emp_bh *)NULL)->ebh_date -
				   (char *)((Emp_bh *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("EMPLOYEE DEDUCTION FILE ...\n");

	strcpy(file_desc.id_f_name, EMP_DED_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Empl Deduction File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Emp_ded) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Emp_ded *)NULL)->ed_numb[0] -
				   (char *)((Emp_ded *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Emp_ded *)NULL)->ed_code[0] -
				   (char *)((Emp_ded *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Emp_ded *)NULL)->ed_group[0] -
				   (char *)((Emp_ded *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("EMPLOYEE DEDUCTION HISTORY FILE ...\n");

	strcpy(file_desc.id_f_name, EMP_DD_HIS_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Emp Deduct Hist File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Emp_dh) ;
	file_desc.tot_keys = 3 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 5 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Emp_dh *)NULL)->edh_numb[0] -
				   (char *)((Emp_dh *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] =  (char *)&((Emp_dh *)NULL)->edh_pp -
				   (char *)((Emp_dh *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Emp_dh *)NULL)->edh_date -
				   (char *)((Emp_dh *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Emp_dh *)NULL)->edh_code[0] -
				   (char *)((Emp_dh *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Emp_dh *)NULL)->edh_group[0] -
				   (char *)((Emp_dh *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt Key */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Emp_dh *)NULL)->edh_numb[0] -
				   (char *)((Emp_dh *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Emp_dh *)NULL)->edh_code[0] -
				   (char *)((Emp_dh *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Emp_dh *)NULL)->edh_group[0] -
				   (char *)((Emp_dh *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Emp_dh *)NULL)->edh_date -
				   (char *)((Emp_dh *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 2 Alt Key */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Emp_dh *)NULL)->edh_code[0] -
				   (char *)((Emp_dh *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Emp_dh *)NULL)->edh_group[0] -
				   (char *)((Emp_dh *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Emp_dh *)NULL)->edh_numb[0] -
				   (char *)((Emp_dh *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Emp_dh *)NULL)->edh_date -
				   (char *)((Emp_dh *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("EMPLOYEE CSB/LOAN FILE ...\n");

	strcpy(file_desc.id_f_name, EMP_LOAN_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Empl CSB/Loan File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Emp_loan) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Emp_loan *)NULL)->el_numb[0] -
				   (char *)((Emp_loan *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Emp_loan *)NULL)->el_code[0] -
				   (char *)((Emp_loan *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Emp_loan *)NULL)->el_seq -
				   (char *)((Emp_loan *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("EMPLOYEE CSB/LOAN HISTORY FILE ...\n");

	strcpy(file_desc.id_f_name, EMP_LN_HIS_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Emp CSB/Loan His File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Emp_ln_his) ;
	file_desc.tot_keys = 2 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 5 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Emp_ln_his *)NULL)->elh_numb[0] -
				   (char *)((Emp_ln_his *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Emp_ln_his *)NULL)->elh_pp -
				   (char *)((Emp_ln_his *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Emp_ln_his *)NULL)->elh_date -
				   (char *)((Emp_ln_his *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Emp_ln_his *)NULL)->elh_code[0] -
				   (char *)((Emp_ln_his *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 5 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Emp_ln_his *)NULL)->elh_seq -
				   (char *)((Emp_ln_his *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 1st Alt Key */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Emp_ln_his *)NULL)->elh_code[0] -
				   (char *)((Emp_ln_his *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Emp_ln_his *)NULL)->elh_numb[0] -
				   (char *)((Emp_ln_his *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Emp_ln_his *)NULL)->elh_date -
				   (char *)((Emp_ln_his *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("EMPLOYEE GARNISHMENT FILE ...\n");

	strcpy(file_desc.id_f_name, EMP_GARN_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Emp Garnishment File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Emp_garn) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Emp_garn *)NULL)->eg_numb[0] -
				   (char *)((Emp_garn *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Emp_garn *)NULL)->eg_pr_cd -
				   (char *)((Emp_garn *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] =  (char *)&((Emp_garn *)NULL)->eg_seq -
				   (char *)((Emp_garn *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("EMPLOYEE GARNISHMENT HISTORY FILE ...\n");

	strcpy(file_desc.id_f_name, EMP_GR_HIS_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Emp Garnish Hist File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Emp_gr_his) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 5 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Emp_gr_his *)NULL)->egh_numb[0] -
				   (char *)((Emp_gr_his *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] =  (char *)&((Emp_gr_his *)NULL)->egh_pp -
				   (char *)((Emp_gr_his *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Emp_gr_his *)NULL)->egh_date -
				   (char *)((Emp_gr_his *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Emp_gr_his *)NULL)->egh_pr_cd -
				   (char *)((Emp_gr_his *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 5 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Emp_gr_his *)NULL)->egh_seq -
				   (char *)((Emp_gr_his *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}


	printf ("TEACHER ASSIGNMENT FILE ...\n");

	strcpy(file_desc.id_f_name, TEACH_ASS_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Teach Assignment File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Teach_ass) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Teach_ass *)NULL)->tc_numb[0] -
				   (char *)((Teach_ass *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Teach_ass *)NULL)->tc_cost -
				   (char *)((Teach_ass *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Teach_ass *)NULL)->tc_ar_sp[0] -
				   (char *)((Teach_ass *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("TEACHER QUALIFICATIONS FILE ...\n");

	strcpy(file_desc.id_f_name, TEACH_QUAL_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Teach Qualify File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Teach_qual) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Teach_qual *)NULL)->tq_numb[0] -
				   (char *)((Teach_qual *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Teach_qual *)NULL)->tq_code[0] -
				   (char *)((Teach_qual *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("EMPLOYEE ATTENDANCE HISTORY FILE ...\n");

	strcpy(file_desc.id_f_name, EMP_ATT_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Emp Attend Hist File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Emp_at_his) ;
	file_desc.tot_keys = 3 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Emp_at_his *)NULL)->eah_numb[0] -
				   (char *)((Emp_at_his *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Emp_at_his *)NULL)->eah_date -
				   (char *)((Emp_at_his *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 1st Alternate Key */

	keysarray[offset++] = 2;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  3 ;
	keysarray[offset++] =  (char *)&((Emp_at_his *)NULL)->eah_code[0] -
				   (char *)((Emp_at_his *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Emp_at_his *)NULL)->eah_date -
				   (char *)((Emp_at_his *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 2nd Alternate Key */

	keysarray[offset++] = 3;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Emp_at_his *)NULL)->eah_numb[0] -
				   (char *)((Emp_at_his *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  3 ;
	keysarray[offset++] =  (char *)&((Emp_at_his *)NULL)->eah_code[0] -
				   (char *)((Emp_at_his *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Emp_at_his *)NULL)->eah_date -
				   (char *)((Emp_at_his *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("TIME FILE ...\n");

	strcpy(file_desc.id_f_name, TIME_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Time File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Time) ;
	file_desc.tot_keys = 5 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Time *)NULL)->tm_numb[0] -
				   (char *)((Time *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Time *)NULL)->tm_date -
				   (char *)((Time *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Time *)NULL)->tm_no -
				   (char *)((Time *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 1st Alternate Key */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Time *)NULL)->tm_numb[0] -
				   (char *)((Time *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Time *)NULL)->tm_week -
				   (char *)((Time *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Time *)NULL)->tm_date -
				   (char *)((Time *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Time *)NULL)->tm_no -
				   (char *)((Time *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 2nd Alternate Key */

	keysarray[offset++] = 5 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Time *)NULL)->tm_earn[0] -
				   (char *)((Time *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Time *)NULL)->tm_numb[0] -
				   (char *)((Time *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Time *)NULL)->tm_week -
				   (char *)((Time *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Time *)NULL)->tm_date -
				   (char *)((Time *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 5 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Time *)NULL)->tm_no -
				   (char *)((Time *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 3rd Alternate Key */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Time *)NULL)->tm_numb[0] -
				   (char *)((Time *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */ 
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Time *)NULL)->tm_year -
				   (char *)((Time *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Time *)NULL)->tm_pp -
				   (char *)((Time *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Time *)NULL)->tm_week -
				   (char *)((Time *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 4rd Alternate Key */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Time *)NULL)->tm_numb[0] -
				   (char *)((Time *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Time *)NULL)->tm_earn[0] -
				   (char *)((Time *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Time *)NULL)->tm_date -
				   (char *)((Time *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Time *)NULL)->tm_no -
				   (char *)((Time *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("TIME HISTORY FILE ...\n");

	strcpy(file_desc.id_f_name, TIME_HIS_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Time History File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Time_his) ;
	file_desc.tot_keys = 2 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Time_his *)NULL)->tmh_numb[0] -
				   (char *)((Time_his *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Time_his *)NULL)->tmh_date -
				   (char *)((Time_his *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Time_his *)NULL)->tmh_no -
				   (char *)((Time_his *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 1st Alternate Key */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Time_his *)NULL)->tmh_numb[0] -
				   (char *)((Time_his *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Time_his *)NULL)->tmh_year -
				   (char *)((Time_his *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Time_his *)NULL)->tmh_pp -
				   (char *)((Time_his *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Time_his *)NULL)->tmh_week -
				   (char *)((Time_his *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("PAY PERIOD EARNINGS FILE ...\n");

	strcpy(file_desc.id_f_name, PP_EARN_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Pay Period Earn File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Pay_earn) ;
	file_desc.tot_keys = 2 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Pay_earn *)NULL)->pe_numb[0] -
				   (char *)((Pay_earn *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] =  (char *)&((Pay_earn *)NULL)->pe_pp -
				   (char *)((Pay_earn *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Pay_earn *)NULL)->pe_date -
				   (char *)((Pay_earn *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt Key 1 */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] =  (char *)&((Pay_earn *)NULL)->pe_cc -
				   (char *)((Pay_earn *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Pay_earn *)NULL)->pe_numb[0] -
				   (char *)((Pay_earn *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] =  (char *)&((Pay_earn *)NULL)->pe_pp -
				   (char *)((Pay_earn *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Pay_earn *)NULL)->pe_date -
				   (char *)((Pay_earn *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("PAY PERIOD BENEFIT FILE ...\n");

	strcpy(file_desc.id_f_name, PP_BEN_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Pay Pd Benefit File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Pp_ben) ;
	file_desc.tot_keys = 2 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 6 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Pp_ben *)NULL)->pb_numb[0] -
				   (char *)((Pp_ben *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] =  (char *)&((Pp_ben *)NULL)->pb_pp -
				   (char *)((Pp_ben *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Pp_ben *)NULL)->pb_date -
				   (char *)((Pp_ben *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Pp_ben *)NULL)->pb_code[0] -
				   (char *)((Pp_ben *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 5 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Pp_ben *)NULL)->pb_fund -
				   (char *)((Pp_ben *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 6 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  18 ;
	keysarray[offset++] =  (char *)&((Pp_ben *)NULL)->pb_acct[0] -
				   (char *)((Pp_ben *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	/* 1st Alternate Code */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Pp_ben *)NULL)->pb_numb[0] -
				   (char *)((Pp_ben *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Pp_ben *)NULL)->pb_code[0] -
				   (char *)((Pp_ben *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] =  (char *)&((Pp_ben *)NULL)->pb_pp -
				   (char *)((Pp_ben *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  18 ;
	keysarray[offset++] =  (char *)&((Pp_ben *)NULL)->pb_acct[0] -
				   (char *)((Pp_ben *)NULL) ;
	keysarray[offset++] = ASCND ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("PAY PERIOD DEDUCTION FILE ...\n");

	strcpy(file_desc.id_f_name, PP_DED_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Pay Pd Deduction File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Pay_ded) ;
	file_desc.tot_keys = 2 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 7 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Pay_ded *)NULL)->pd_numb[0] -
				   (char *)((Pay_ded *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] =  (char *)&((Pay_ded *)NULL)->pd_pp -
				   (char *)((Pay_ded *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Pay_ded *)NULL)->pd_date -
				   (char *)((Pay_ded *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Pay_ded *)NULL)->pd_code[0] -
				   (char *)((Pay_ded *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 5 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Pay_ded *)NULL)->pd_group[0] -
				   (char *)((Pay_ded *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 6 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Pay_ded *)NULL)->pd_fund -
				   (char *)((Pay_ded *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 7 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  13 ;
	keysarray[offset++] =  (char *)&((Pay_ded *)NULL)->pd_acct[0] -
				   (char *)((Pay_ded *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 1st alternate Key */

	keysarray[offset++] = 6 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Pay_ded *)NULL)->pd_numb[0] -
				   (char *)((Pay_ded *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Pay_ded *)NULL)->pd_code[0] -
				   (char *)((Pay_ded *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Pay_ded *)NULL)->pd_group[0] -
				   (char *)((Pay_ded *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] =  (char *)&((Pay_ded *)NULL)->pd_pp -
				   (char *)((Pay_ded *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 5 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Pay_ded *)NULL)->pd_fund -
				   (char *)((Pay_ded *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 6 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  13 ;
	keysarray[offset++] =  (char *)&((Pay_ded *)NULL)->pd_acct[0] -
				   (char *)((Pay_ded *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("PAY PERIOD CSB/LOAN FILE ...\n");

	strcpy(file_desc.id_f_name, PP_LOAN_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Pay Pd CSB/Loan File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Pay_loan) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 7 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Pay_loan *)NULL)->pc_numb[0] -
				   (char *)((Pay_loan *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Pay_loan *)NULL)->pc_pp -
				   (char *)((Pay_loan *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Pay_loan *)NULL)->pc_date -
				   (char *)((Pay_loan *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Pay_loan *)NULL)->pc_code[0] -
				   (char *)((Pay_loan *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 5 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Pay_loan *)NULL)->pc_seq -
				   (char *)((Pay_loan *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 6 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Pay_loan *)NULL)->pc_fund -
				   (char *)((Pay_loan *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 7 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  13 ;
	keysarray[offset++] =  (char *)&((Pay_loan *)NULL)->pc_acct[0] -
				   (char *)((Pay_loan *)NULL) ;
	keysarray[offset++] = ASCND ;


	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("PAY PERIOD GARNISHMENT FILE ...\n");

	strcpy(file_desc.id_f_name, PP_GARN_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Pay Pd Garnish File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Pay_garn);
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 7 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Pay_garn *)NULL)->pg_numb[0] -
				   (char *)((Pay_garn *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] =  (char *)&((Pay_garn *)NULL)->pg_pp -
				   (char *)((Pay_garn *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Pay_garn *)NULL)->pg_date -
				   (char *)((Pay_garn *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Pay_garn *)NULL)->pg_pr_cd -
				   (char *)((Pay_garn *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 5 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] = 1 ;
	keysarray[offset++] =  (char *)&((Pay_garn *)NULL)->pg_seq -
				   (char *)((Pay_garn *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 6 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Pay_garn *)NULL)->pg_fund -
				   (char *)((Pay_garn *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 7 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  13 ;
	keysarray[offset++] =  (char *)&((Pay_garn *)NULL)->pg_acct[0] -
				   (char *)((Pay_garn *)NULL) ;
	keysarray[offset++] = ASCND ;


	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("CHEQUE MESSAGE FILE ...\n");

	strcpy(file_desc.id_f_name, CHQ_MESS_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Cheque Message File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Chq_mess) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Chq_mess *)NULL)->m_code[0] -
				   (char *)((Chq_mess *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("CHEQUE MESSAGE ASSIGNMENT FILE ...\n");

	strcpy(file_desc.id_f_name, CHQ_MS_ASS_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Chq Msg Assign File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Chq_mess_ass) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Chq_mess_ass *)NULL)->ma_numb[0] -
				   (char *)((Chq_mess_ass *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Chq_mess_ass *)NULL)->ma_code[0] -
				   (char *)((Chq_mess_ass *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}


	printf ("REGISTERED PENSION ADJUSTMENT FILE ...\n");

	strcpy(file_desc.id_f_name, RG_PEN_ADJ_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Reg Pension Adj File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Reg_pen_adj) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Reg_pen_adj *)NULL)->rpa_numb[0] -
				   (char *)((Reg_pen_adj *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("T4 FIELD CODE FILE ...\n");

	strcpy(file_desc.id_f_name, T4_REC_FILE ) ;
	strcpy(file_desc.fl_user_nm,"T4 Field Code File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(T4_rec) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((T4_rec *)NULL)->t4_code[0] -
				   (char *)((T4_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("G/L PAYROLL ACCOUNT FILE ...\n");

	strcpy(file_desc.id_f_name, GLACCT_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Payroll Account File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Gl_acct) ;
	file_desc.tot_keys = 4 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 5 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Gl_acct *)NULL)->gl_fund -
				   (char *)((Gl_acct *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Gl_acct *)NULL)->gl_cc -
				   (char *)((Gl_acct *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Gl_acct *)NULL)->gl_type[0] -
				   (char *)((Gl_acct *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Gl_acct *)NULL)->gl_earn[0] -
				   (char *)((Gl_acct *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 5 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Gl_acct *)NULL)->gl_class[0] -
				   (char *)((Gl_acct *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alternate Key 1 */

	keysarray[offset++] = 5 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Gl_acct *)NULL)->gl_fund -
				   (char *)((Gl_acct *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Gl_acct *)NULL)->gl_type[0] -
				   (char *)((Gl_acct *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Gl_acct *)NULL)->gl_class[0] -
				   (char *)((Gl_acct *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Gl_acct *)NULL)->gl_earn[0] -
				   (char *)((Gl_acct *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 5 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Gl_acct *)NULL)->gl_cc -
				   (char *)((Gl_acct *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 2nd Alternate Key */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Gl_acct *)NULL)->gl_fund -
				   (char *)((Gl_acct *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  18 ;
	keysarray[offset++] =  (char *)&((Gl_acct *)NULL)->gl_acct[0] -
				   (char *)((Gl_acct *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Gl_acct *)NULL)->gl_class[0] -
				   (char *)((Gl_acct *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alternate Key 3 */

	keysarray[offset++] = 5 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Gl_acct *)NULL)->gl_earn[0] -
				   (char *)((Gl_acct *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Gl_acct *)NULL)->gl_fund -
				   (char *)((Gl_acct *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Gl_acct *)NULL)->gl_type[0] -
				   (char *)((Gl_acct *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Gl_acct *)NULL)->gl_class[0] -
				   (char *)((Gl_acct *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 5 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Gl_acct *)NULL)->gl_cc -
				   (char *)((Gl_acct *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("JOURNAL ENTRY FILE ...\n");

	strcpy(file_desc.id_f_name, JR_ENT_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Journal File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Jr_ent) ;
	file_desc.tot_keys = 6 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Jr_ent *)NULL)->jr_fund -
				   (char *)((Jr_ent *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Jr_ent *)NULL)->jr_no -
				   (char *)((Jr_ent *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alternate Key 1 */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Jr_ent *)NULL)->jr_emp_numb[0] -
				   (char *)((Jr_ent *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Jr_ent *)NULL)->jr_code[0] -
				   (char *)((Jr_ent *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Jr_ent *)NULL)->jr_fund -
				   (char *)((Jr_ent *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Jr_ent *)NULL)->jr_no -
				   (char *)((Jr_ent *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alternate Key 2 */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Jr_ent *)NULL)->jr_emp_numb[0] -
				   (char *)((Jr_ent *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Jr_ent *)NULL)->jr_type[0] -
				   (char *)((Jr_ent *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Jr_ent *)NULL)->jr_fund -
				   (char *)((Jr_ent *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Jr_ent *)NULL)->jr_no -
				   (char *)((Jr_ent *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alternate Key 3 */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Jr_ent *)NULL)->jr_fund -
				   (char *)((Jr_ent *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  2 ;
	keysarray[offset++] =  (char *)&((Jr_ent *)NULL)->jr_type[0] -
				   (char *)((Jr_ent *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  18 ;
	keysarray[offset++] =  (char *)&((Jr_ent *)NULL)->jr_acct[0] -
				   (char *)((Jr_ent *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Jr_ent *)NULL)->jr_no -
				   (char *)((Jr_ent *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alternate Key 4 */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Jr_ent *)NULL)->jr_emp_numb[0] -
				   (char *)((Jr_ent *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Jr_ent *)NULL)->jr_fund -
				   (char *)((Jr_ent *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  18 ;
	keysarray[offset++] =  (char *)&((Jr_ent *)NULL)->jr_acct[0] -
				   (char *)((Jr_ent *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alternate Key 5 */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Jr_ent *)NULL)->jr_fund -
				   (char *)((Jr_ent *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  18 ;
	keysarray[offset++] =  (char *)&((Jr_ent *)NULL)->jr_acct[0] -
				   (char *)((Jr_ent *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("JOURNAL HISTORY FILE ...\n");

	strcpy(file_desc.id_f_name, JRH_ENT_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Journal Hist File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Jrh_ent) ;
	file_desc.tot_keys = 3 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Jrh_ent *)NULL)->jrh_fund -
				   (char *)((Jrh_ent *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Jrh_ent *)NULL)->jrh_no -
				   (char *)((Jrh_ent *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Jrh_ent *)NULL)->jrh_cheque -
				   (char *)((Jrh_ent *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alternate Key 1 */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Jrh_ent *)NULL)->jrh_emp_numb[0] -
				   (char *)((Jrh_ent *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Jrh_ent *)NULL)->jrh_date -
				   (char *)((Jrh_ent *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Jrh_ent *)NULL)->jrh_fund -
				   (char *)((Jrh_ent *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  18 ;
	keysarray[offset++] =  (char *)&((Jrh_ent *)NULL)->jrh_acct[0] -
				   (char *)((Jrh_ent *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alternate Key 2 */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Jrh_ent *)NULL)->jrh_fund -
				   (char *)((Jrh_ent *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  18 ;
	keysarray[offset++] =  (char *)&((Jrh_ent *)NULL)->jrh_acct[0] -
				   (char *)((Jrh_ent *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("PAYROLL AUDIT FILE ...\n");

	strcpy(file_desc.id_f_name, AUD_PAY_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Payroll Audit File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Aud_pay) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Aud_pay *)NULL)->aud_emp[0] -
				   (char *)((Aud_pay *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  20 ;
	keysarray[offset++] =  (char *)&((Aud_pay *)NULL)->aud_code[0] -
				   (char *)((Aud_pay *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}


	printf ("EC FILE ...\n");

	strcpy(file_desc.id_f_name, EC_REC_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Ec File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Ec_rec) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Ec_rec *)NULL)->ec_numb[0] -
				   (char *)((Ec_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("PAYROLL CHEQUE REG FILE ...\n ");

	strcpy(file_desc.id_f_name, CHQ_REG_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Cheque Register File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Chq_reg) ;
	file_desc.tot_keys = 2 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Chq_reg *)NULL)->cr_numb -
				   (char *)((Chq_reg *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Chq_reg *)NULL)->cr_date -
				   (char *)((Chq_reg *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt Key 1 */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Chq_reg *)NULL)->cr_emp_numb[0] -
				   (char *)((Chq_reg *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Chq_reg *)NULL)->cr_date -
				   (char *)((Chq_reg *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("MANUAL CHEQUE FILE ...\n ");

	strcpy(file_desc.id_f_name, MAN_CHQ_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Manual Cheque File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Man_chq) ;
	file_desc.tot_keys = 4 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Man_chq *)NULL)->mc_emp_numb[0] -
				   (char *)((Man_chq *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Man_chq *)NULL)->mc_date -
				   (char *)((Man_chq *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Man_chq *)NULL)->mc_chq_numb -
				   (char *)((Man_chq *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alternate Key 1 */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Man_chq *)NULL)->mc_date -
				   (char *)((Man_chq *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Man_chq *)NULL)->mc_emp_numb[0] -
				   (char *)((Man_chq *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alternate Key 2 */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Man_chq *)NULL)->mc_chq_numb -
				   (char *)((Man_chq *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alternate Key 3 */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Man_chq *)NULL)->mc_emp_numb[0] -
				   (char *)((Man_chq *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Man_chq *)NULL)->mc_ded_code[0] -
				   (char *)((Man_chq *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("ROE FILE ...\n");

	strcpy(file_desc.id_f_name, ROE_FILE ) ;
	strcpy(file_desc.fl_user_nm,"ROE File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Roe) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Roe *)NULL)->ro_emp_numb[0] -
				   (char *)((Roe *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("TEMP SCHEDULE1 FILE ...\n");

	strcpy(file_desc.id_f_name, TMP_SCHED1_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Tmp Schedule1 File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Tmp_sched1) ;
	file_desc.tot_keys = 2 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 7 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Tmp_sched1 *)NULL)->tm_numb[0] -
				   (char *)((Tmp_sched1 *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tmp_sched1 *)NULL)->tm_week -
				   (char *)((Tmp_sched1 *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tmp_sched1 *)NULL)->tm_fund -
				   (char *)((Tmp_sched1 *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Tmp_sched1 *)NULL)->tm_class[0] -
				   (char *)((Tmp_sched1 *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 5 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tmp_sched1 *)NULL)->tm_cost -
				   (char *)((Tmp_sched1 *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 6 */
	keysarray[offset++] = 	CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Tmp_sched1 *)NULL)->tm_dept[0] -
				   (char *)((Tmp_sched1 *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 7 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Tmp_sched1 *)NULL)->tm_area[0] -
				   (char *)((Tmp_sched1 *)NULL) ;
	keysarray[offset++] = ASCND ;

	/*Altername Key 1 */

	keysarray[offset++] = 3;		/*Parts*/

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  40 ;
	keysarray[offset++] =  (char *)&((Tmp_sched1 *)NULL)->tm_sortk_1[0] -
				   (char *)((Tmp_sched1 *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  40 ;
	keysarray[offset++] =  (char *)&((Tmp_sched1 *)NULL)->tm_sortk_2[0] -
				   (char *)((Tmp_sched1 *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  40 ;
	keysarray[offset++] =  (char *)&((Tmp_sched1 *)NULL)->tm_sortk_3[0] -
				   (char *)((Tmp_sched1 *)NULL) ;
	keysarray[offset++] = ASCND ;


	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("SEN PARAM FILE ...\n");

	strcpy(file_desc.id_f_name, SEN_PAR_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Sen Param File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Sen_par) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Sen_par *)NULL)->sn_position[0] -
				   (char *)((Sen_par *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Sen_par *)NULL)->sn_eff_date -
				   (char *)((Sen_par *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("EMP SENIORITY FILE ...\n");

	strcpy(file_desc.id_f_name, EMP_SEN_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Emp Sen File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Emp_sen) ;
	file_desc.tot_keys = 2 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Emp_sen *)NULL)->esn_numb[0] -
				   (char *)((Emp_sen *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Emp_sen *)NULL)->esn_month -
				   (char *)((Emp_sen *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  7 ;
	keysarray[offset++] =  (char *)&((Emp_sen *)NULL)->esn_pos[0] -
				   (char *)((Emp_sen *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  7 ;
	keysarray[offset++] =  (char *)&((Emp_sen *)NULL)->esn_class[0] -
				   (char *)((Emp_sen *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* Alt Key 1 */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Emp_sen *)NULL)->esn_month -
				   (char *)((Emp_sen *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Emp_sen *)NULL)->esn_numb[0] -
				   (char *)((Emp_sen *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  7 ;
	keysarray[offset++] =  (char *)&((Emp_sen *)NULL)->esn_pos[0] -
				   (char *)((Emp_sen *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  7 ;
	keysarray[offset++] =  (char *)&((Emp_sen *)NULL)->esn_class[0] -
				   (char *)((Emp_sen *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("TMP SENIORITY FILE ...\n");

	strcpy(file_desc.id_f_name, TMP_SEN_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Tmp Sen File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Tmp_sen) ;
	file_desc.tot_keys = 2 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Tmp_sen *)NULL)->tsn_numb[0] -
				   (char *)((Tmp_sen *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tmp_sen *)NULL)->tsn_month -
				   (char *)((Tmp_sen *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  7 ;
	keysarray[offset++] =  (char *)&((Tmp_sen *)NULL)->tsn_pos[0] -
				   (char *)((Tmp_sen *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  7 ;
	keysarray[offset++] =  (char *)&((Tmp_sen *)NULL)->tsn_class[0] -
				   (char *)((Tmp_sen *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* ist Alternate Key */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tmp_sen *)NULL)->tsn_month -
				   (char *)((Tmp_sen *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Tmp_sen *)NULL)->tsn_numb[0] -
				   (char *)((Tmp_sen *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  7 ;
	keysarray[offset++] =  (char *)&((Tmp_sen *)NULL)->tsn_pos[0] -
				   (char *)((Tmp_sen *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  7 ;
	keysarray[offset++] =  (char *)&((Tmp_sen *)NULL)->tsn_class[0] -
				   (char *)((Tmp_sen *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("GOVERNMENT PARAMETER FILE ...\n");

	strcpy(file_desc.id_f_name, GOV_PARAM_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Govern Param File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Gov_param) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = LONG ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Gov_param *)NULL)->gp_eff_date -
				   (char *)((Gov_param *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("T4 ADJUSTMENT FILE ...\n");

	strcpy(file_desc.id_f_name, T4_ADJ_FILE ) ;
	strcpy(file_desc.fl_user_nm,"T4 Adjustment File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(T4_adj) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((T4_adj *)NULL)->ta_numb[0] -
				   (char *)((T4_adj *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("RELIGION FILE ...\n");

	strcpy(file_desc.id_f_name, RELIGION_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Religion File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Religion) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Religion *)NULL)->rel_code[0] -
				   (char *)((Religion *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("PAYROLL SECURITY FILE ...\n");

	strcpy(file_desc.id_f_name, USERBARG_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Userbarg File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Userbarg) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  10 ;
	keysarray[offset++] =  (char *)&((Userbarg *)NULL)->ub_id[0] -
				   (char *)((Userbarg *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Userbarg *)NULL)->ub_barg[0] -
				   (char *)((Userbarg *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("TEMPORARY EMPLOYEE FILE ...\n");

	strcpy(file_desc.id_f_name, TMP_EMP_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Temp Employee File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Tmp_Emp) ;
	file_desc.tot_keys = 10 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Tmp_Emp *)NULL)->tem_numb[0] -
				   (char *)((Tmp_Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 1st Alternate Key */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Tmp_Emp *)NULL)->tem_barg[0] -
				   (char *)((Tmp_Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Tmp_Emp *)NULL)->tem_numb[0] -
				   (char *)((Tmp_Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 2st Alternate Key */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Tmp_Emp *)NULL)->tem_barg[0] -
				   (char *)((Tmp_Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  25 ;
	keysarray[offset++] =  (char *)&((Tmp_Emp *)NULL)->tem_last_name[0] -
				   (char *)((Tmp_Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  15 ;
	keysarray[offset++] =  (char *)&((Tmp_Emp *)NULL)->tem_first_name[0] -
				   (char *)((Tmp_Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  15 ;
	keysarray[offset++] =  (char *)&((Tmp_Emp *)NULL)->tem_mid_name[0] -
				   (char *)((Tmp_Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 3nd Alternate Key */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Tmp_Emp *)NULL)->tem_pos[0] -
				   (char *)((Tmp_Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Tmp_Emp *)NULL)->tem_numb[0] -
				   (char *)((Tmp_Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 4nd Alternate Key */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Tmp_Emp *)NULL)->tem_pos[0] -
				   (char *)((Tmp_Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  25 ;
	keysarray[offset++] =  (char *)&((Tmp_Emp *)NULL)->tem_last_name[0] -
				   (char *)((Tmp_Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  15 ;
	keysarray[offset++] =  (char *)&((Tmp_Emp *)NULL)->tem_first_name[0] -
				   (char *)((Tmp_Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  15 ;
	keysarray[offset++] =  (char *)&((Tmp_Emp *)NULL)->tem_mid_name[0] -
				   (char *)((Tmp_Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 5nd Alternate Key */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tmp_Emp *)NULL)->tem_cc -
				   (char *)((Tmp_Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Tmp_Emp *)NULL)->tem_numb[0] -
				   (char *)((Tmp_Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 6nd Alternate Key */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tmp_Emp *)NULL)->tem_cc -
				   (char *)((Tmp_Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  25 ;
	keysarray[offset++] =  (char *)&((Tmp_Emp *)NULL)->tem_last_name[0] -
				   (char *)((Tmp_Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  15 ;
	keysarray[offset++] =  (char *)&((Tmp_Emp *)NULL)->tem_first_name[0] -
				   (char *)((Tmp_Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  15 ;
	keysarray[offset++] =  (char *)&((Tmp_Emp *)NULL)->tem_mid_name[0] -
				   (char *)((Tmp_Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 7nd Alternate Key */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  25 ;
	keysarray[offset++] =  (char *)&((Tmp_Emp *)NULL)->tem_last_name[0] -
				   (char *)((Tmp_Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  15 ;
	keysarray[offset++] =  (char *)&((Tmp_Emp *)NULL)->tem_first_name[0] -
				   (char *)((Tmp_Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  15 ;
	keysarray[offset++] =  (char *)&((Tmp_Emp *)NULL)->tem_mid_name[0] -
				   (char *)((Tmp_Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 8th Alternate Key */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Tmp_Emp *)NULL)->tem_barg[0] -
				   (char *)((Tmp_Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tmp_Emp *)NULL)->tem_cc -
				   (char *)((Tmp_Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Tmp_Emp *)NULL)->tem_numb[0] -
				   (char *)((Tmp_Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 9th Alternate Key */

	keysarray[offset++] = 5 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Tmp_Emp *)NULL)->tem_barg[0] -
				   (char *)((Tmp_Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tmp_Emp *)NULL)->tem_cc -
				   (char *)((Tmp_Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  25 ;
	keysarray[offset++] =  (char *)&((Tmp_Emp *)NULL)->tem_last_name[0] -
				   (char *)((Tmp_Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  15 ;
	keysarray[offset++] =  (char *)&((Tmp_Emp *)NULL)->tem_first_name[0] -
				   (char *)((Tmp_Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 5 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  15 ;
	keysarray[offset++] =  (char *)&((Tmp_Emp *)NULL)->tem_mid_name[0] -
				   (char *)((Tmp_Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("TEMPORARY2 EMPLOYEE FILE ...\n");

	strcpy(file_desc.id_f_name, TMP2_EMP_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Temp2 Employee File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Tmp2_Emp) ;
	file_desc.tot_keys = 3 ;
	file_desc.keys_offset = offset ;

	/* Main Key */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Tmp2_Emp *)NULL)->tem2_cc -
				   (char *)((Tmp2_Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  25 ;
	keysarray[offset++] =  (char *)&((Tmp2_Emp *)NULL)->tem2_last_name[0] -
				   (char *)((Tmp2_Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  15 ;
	keysarray[offset++] =  (char *)&((Tmp2_Emp *)NULL)->tem2_first_name[0] -
				   (char *)((Tmp2_Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Tmp2_Emp *)NULL)->tem2_numb[0] -
				   (char *)((Tmp2_Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 1st Alternate Key */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  25 ;
	keysarray[offset++] =  (char *)&((Tmp2_Emp *)NULL)->tem2_last_name[0] -
				   (char *)((Tmp2_Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  15 ;
	keysarray[offset++] =  (char *)&((Tmp2_Emp *)NULL)->tem2_first_name[0] -
				   (char *)((Tmp2_Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 2nd Alternate Key */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Tmp2_Emp *)NULL)->tem2_numb[0] -
				   (char *)((Tmp2_Emp *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

/* *****                                                       ***** */
	printf ("TEMPORARY SENIORITY SORT FILE ...\n");

	strcpy(file_desc.id_f_name, TS_SEN_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Temp Sen Sort File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Ts_sen) ;
	file_desc.tot_keys = 4 ;
	file_desc.keys_offset = offset ;

	/* Main Key */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Ts_sen *)NULL)->ts_barg[0] -
				   (char *)((Ts_sen *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Ts_sen *)NULL)->ts_total_years -
				   (char *)((Ts_sen *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = DOUBLE ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Ts_sen *)NULL)->ts_total_days -
				   (char *)((Ts_sen *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Ts_sen *)NULL)->ts_emp_numb[0] -
				   (char *)((Ts_sen *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 1st Alternate Key */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Ts_sen *)NULL)->ts_position[0] -
				   (char *)((Ts_sen *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Ts_sen *)NULL)->ts_total_years -
				   (char *)((Ts_sen *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = DOUBLE ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Ts_sen *)NULL)->ts_total_days -
				   (char *)((Ts_sen *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Ts_sen *)NULL)->ts_emp_numb[0] -
				   (char *)((Ts_sen *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 2nd Alternate Key */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Ts_sen *)NULL)->ts_cc -
				   (char *)((Ts_sen *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Ts_sen *)NULL)->ts_total_years -
				   (char *)((Ts_sen *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = DOUBLE ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Ts_sen *)NULL)->ts_total_days -
				   (char *)((Ts_sen *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Ts_sen *)NULL)->ts_emp_numb[0] -
				   (char *)((Ts_sen *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* 3rd Alternate Key */

	keysarray[offset++] = 4 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Ts_sen *)NULL)->ts_class[0] -
				   (char *)((Ts_sen *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = SHORT ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Ts_sen *)NULL)->ts_total_years -
				   (char *)((Ts_sen *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = DOUBLE ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Ts_sen *)NULL)->ts_total_days -
				   (char *)((Ts_sen *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 4 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Ts_sen *)NULL)->ts_emp_numb[0] -
				   (char *)((Ts_sen *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

/* *****                                                       ***** */
	printf ("VACATION ACCRUAL FILE ...\n");

	strcpy(file_desc.id_f_name, VC_ACC_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Vac. Accrual File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Vc_acc) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* Main Key */

	keysarray[offset++] = 3 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  6 ;
	keysarray[offset++] =  (char *)&((Vc_acc *)NULL)->vc_barg[0] -
				   (char *)((Vc_acc *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = DOUBLE ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Vc_acc *)NULL)->vc_low_sen -
				   (char *)((Vc_acc *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 3 */
	keysarray[offset++] = DOUBLE ;
	keysarray[offset++] =  1 ;
	keysarray[offset++] =  (char *)&((Vc_acc *)NULL)->vc_high_sen -
				   (char *)((Vc_acc *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

/* *****                                                       ***** */
	printf ("EMPLOYEE COMPETITION FILE ...\n");

	strcpy(file_desc.id_f_name, EMP_COMP_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Employee Competition File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Emp_comp) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* Main Key */

	keysarray[offset++] = 2 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  12 ;
	keysarray[offset++] =  (char *)&((Emp_comp *)NULL)->ec_numb[0] -
				   (char *)((Emp_comp *)NULL) ;
	keysarray[offset++] = ASCND ;

	/* part 2 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  7 ;
	keysarray[offset++] =  (char *)&((Emp_comp *)NULL)->ec_code[0] -
				   (char *)((Emp_comp *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

/* *****                                                       ***** */
	printf ("COMPETITION FILE ...\n");

	strcpy(file_desc.id_f_name, COMP_FILE ) ;
	strcpy(file_desc.fl_user_nm,"Competition File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(Comp) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* Main Key */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  7 ;
	keysarray[offset++] =  (char *)&((Comp *)NULL)->cm_code[0] -
				   (char *)((Comp *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

/* *****                                                       ***** */

	printf ("USER PROFILE FILE ...\n");

	strcpy(file_desc.id_f_name, USERPROF_FILE ) ;
	strcpy(file_desc.fl_user_nm,"User Profile File");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= RWR ;
	file_desc.reclen   =  sizeof(UP_rec) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	/* main Key */

	keysarray[offset++] = 1 ;	/* Parts */

	/* part 1 */
	keysarray[offset++] = CHAR ;
	keysarray[offset++] =  10 ;
	keysarray[offset++] =  (char *)((UP_rec *)NULL)->u_id -
				   (char *)((UP_rec *)NULL) ;
	keysarray[offset++] = ASCND ;

	if((i = offset - file_desc.keys_offset) > max_size)
		max_size = i ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("TEMPIDX FILE...1\t");

	strcpy(file_desc.id_f_name, TMPIX_FILE_1) ;
	strcpy(file_desc.fl_user_nm,"Temp. Index File1");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= R ;	/* Read only for temporay files */
	file_desc.reclen   =  sizeof(Emp) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	if(TMPMAX > max_size) max_size = TMPMAX ;
		/* Allocate minimum 50 Integers for temp keysarray */

	offset += max_size  ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

	printf ("TEMPIDX FILE...2 \n");

	strcpy(file_desc.id_f_name, TMPIX_FILE_2) ;
	strcpy(file_desc.fl_user_nm,"Temp. Index File2");
	file_desc.id_f_type 	= ISAM ;
	file_desc.id_io_mode 	= R ;	/* Read only for temporay files */
	file_desc.reclen   =  sizeof(Emp) ;
	file_desc.tot_keys = 1 ;
	file_desc.keys_offset = offset ;

	offset += max_size ;

	if(write(k_fd, (char *)&file_desc, sizeof(f_id)) < sizeof(f_id)){
		printf("ERROR: %s : While writing\n", file_desc.fl_user_nm );
		close(k_fd);
		exit(-1);
	}

/***	Write keysarray at the end *****/

	if ( offset >= TOT_KEYS ) {
		printf(" Too many keys .. change TOT_KEYS in init_id.c \n");
		exit(-1) ;
	}

	keysarray[0] = offset -1 ;	/* set the size in first byte */
	printf("Offset: %d", offset ) ;
	printf("\tSize of keydesc Record: %d\n", sizeof(f_id));

	if(write(k_fd, (char *)keysarray, sizeof(int) * offset)
						< sizeof(int) * offset ){
		printf("ERROR: in writing keysarray \n");
		close(k_fd);
		exit(-1);
	}

	close(k_fd) ;

	printf("\nId_array Successfully Initialised\n");

	if(dist_no[0] != '\0') {
		/***	This file should be seperate for each data base
		strcpy(file_name,DATA_PATH);
		strcat(file_name,ERR_LOG);
		***/
		form_f_name(ERR_LOG, file_name);

		if ( (k_fd = creat(file_name, TXT_CRMODE)) < 0 )
			printf("Errlog could Not Be Created...\n");
		else
			printf("Error Log created\n");
		close(k_fd) ;

		/* Initialize files */

		if ( init_dbh()  < 0 ) {
			printf("DBH Initialization ERROR.. DBERROR: %d\n",
				dberror);
			exit(-1);
		}

		for(i = 0 ; i < TOTAL_FILES ; i++) {

#ifdef	ORACLE
			code= init_file(i, c_mesg) ;
			if(code == NOERROR)
			    for (j = 1 ; j < id_array[i].tot_keys ; j++)
				if((code = creat_indx(i, j, c_mesg)) < 0)break ;
#else
			code = init_file(i) ;
#endif
			if(code < 0) {
			    printf("File#: %d   Name: %s\n", i,
				id_array[i].id_f_name) ;
			    printf("\tERROR: Dberror: %d Iserror: %d Errno: %d\n",
				dberror,iserror,errno);
#ifdef	ORACLE
			    printf( "%s\n", c_mesg ) ;
#endif
			    printf("Hit RETURN to continue.....\n");
			    fflush(stdout) ;
			    read(0, c_mesg, 80) ;
			    continue ;
			}
	/* this was add to fix the too many files open error */
	/* added by J.Prescott Mar. 20/91 */
			close_file(i);
		}
		close_dbh();
	}
	exit(0) ;
}
/*---------------------E N D   O F   F I L E--------------------------*/

/*---------------------E N D   O F   F I L E--------------------------*/

