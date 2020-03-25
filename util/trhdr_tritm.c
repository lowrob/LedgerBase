/*
*	trhdr_tritm.c
*	MODIFIED: JUNE 20, 1990
*	DESC	: compare the transaction items to the header
*/

#define MAIN
#define MAINFL	-1

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	PROJNAM	"journal"
#define	LOGREC	1 
#define	FORMNO 	2
#define	OUTPUT	1			/* Printer */
#define	FILENAME	"hdr_itm.dat"	/* Filename that is created */

#define SYSTEM		"GENERAL LEDGER"
#define MOD_DATE	"23-JAN-90"
#define	EXIT		12	/* also defined in reputils.c */

main(argc,argv)
int	argc;
char	*argv[];
{
	long	date  ;
	Tr_hdr	tr_hdr ;
	Tr_item	trans ;
	int	code ;
	char 	*arayptr[5] ; 	/* Declarations for Report writer usage */
	char 	projname[50] ;
	char	dt_str[20] ;	
	char	c_mesg[80] ;	
	int	reccod ;
	Pa_rec	pa_rec ;

	/****	
	*	Accept Project code, Logical record number in the 
	*	Project and format number for the logical record ..
	*	Accept the output media code . Open the report ..
	****/

	reccod = 99;	/* always 99 accounts */ 

	strncpy( SYS_NAME, SYSTEM, 50 ); 	/* Module name */
	strncpy( CHNG_DATE, MOD_DATE, 10 );	/* Last date of change */
	
	proc_switch(argc,argv,MAINFL);
	if(argc < 3)
		exit(0);

	code = get_param(&pa_rec, BROWSE, 1, c_mesg);
	if(code < 0) return(code) ;

	date = get_date() ;

	mkdate(date, dt_str) ;

	strcpy(projname,FMT_PATH) ;
	strcat(projname,PROJNAM);

	code = rpopen(projname,LOGREC,FORMNO,OUTPUT,FILENAME, PROG_NAME,
								dt_str);
	if ( code < 0 ){
		sprintf(c_mesg,  "Rpopen code :%d\n", code ) ;
		return(ERROR) ;
	}

	/* Change first title line to Company/School district name */
	rpChangetitle(1, pa_rec.pa_co_name);

#ifdef ENGLISH
	sprintf(c_mesg,"Period: %d", pa_rec.pa_cur_period) ;
#else
	sprintf(c_mesg,"Periode: %d",pa_rec.pa_cur_period) ;
#endif
	rpChangetitle(3, c_mesg);

	arayptr[0] = (char *)&tr_hdr ;
	arayptr[1] = (char *)&trans ;
	arayptr[2] = NULL ;
	
	tr_hdr.th_sys_dt = 0;
	tr_hdr.th_fund = 0;
	tr_hdr.th_reccod = 0;
	tr_hdr.th_create[0] = '\0';
	tr_hdr.th_seq_no = 0;
	tr_hdr.th_userid[0] = '\0';
	flg_reset(GLTRHDR);

	trans.ti_fund = 0;
	trans.ti_reccod = 0;
	trans.ti_create[0] = '\0';
	trans.ti_seq_no = 0 ;
	trans.ti_item_no = 0;

	for(;;) {
		code = get_n_trhdr(&tr_hdr, BROWSE, 1, FORWARD, c_mesg) ;
		if(code == EFL) break;
		if(code == ERROR) return( DBH_ERR );
		if(code < 0) return(code) ;

		if(tr_hdr.th_reccod != reccod)
			continue;

		trans.ti_fund = tr_hdr.th_fund;
		trans.ti_reccod = tr_hdr.th_reccod;	
		trans.ti_create[0] = tr_hdr.th_create[0];
		trans.ti_seq_no = tr_hdr.th_seq_no;
		trans.ti_item_no = 0;
		flg_reset(GLTRAN);

		for(;;) {
#ifndef ORACLE
			code = get_n_tritem(&trans,BROWSE,0,FORWARD,c_mesg);
#else
			code = get_n_tritem(&trans,BROWSE,0,EQUAL,c_mesg);
#endif
			if(code < 0) break;

#ifndef ORACLE
			if(tr_hdr.th_fund != trans.ti_fund ||
			   tr_hdr.th_reccod != trans.ti_reccod ||
			   tr_hdr.th_create[0] != trans.ti_create[0] ||
			   tr_hdr.th_seq_no != trans.ti_seq_no) 
				break;
#endif
			rpline(arayptr) ;
			printf("jr# %8ld\tType: %s\tCreate: %s\tItem: %3ld\tAmount: %30.20lf\n",trans.ti_seq_no,tr_hdr.th_type,trans.ti_create,trans.ti_item_no,trans.ti_amount);
		}
		seq_over(GLTRAN);
	}
	
	rpclose() ;
	close_file(GLTRAN) ;
	close_file(GLTRHDR) ;
	return(NOERROR) ;
}

