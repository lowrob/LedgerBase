/* --------------------------------------------------------------------------
	SOURCE NAME:  slabels.c
	SYSTEM     :  BUGETARY FINANCIAL SYSTEM 
	MODULE     :  TENDERING SYSTEM 
	CREATED ON :  21 April 1992
	CREATED BY :  J. Prescott 

DESCRIPTION:
	This program prints Supplier Labels. 

MODIFICATIONS:

PROGRAMMER           YY/MM/DD        DESCRIPTION OF MODIFICATIONS
^^^^^^^^^^           ^^^^^^^^        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

L.Robichaud	1993/10/13	Pass new parameter to close_rep function for 
				a banner to print with 3000 family.
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <repdef.h>
#include <bfs_defs.h>
#include <bfs_recs.h>
#include <linklist.h>

#define  EXIT  12

#ifdef ENGLISH
#define PRINTER 'P'
#define DISPLAY 'D'
#define FILE_IO	'F'
#define SPOOL	'S'	/* added for requisitions */
#define YES	'Y'
#define NO	'N'

#define LABELS	'L'
#else
#define PRINTER 'I'
#define DISPLAY 'A'
#define FILE_IO	'D'
#define SPOOL	'B'	/* added for requisitions */
#define YES	'O'
#define NO	'N'

#define LABELS	'E'
#endif

static Pa_rec   pa_rec;             /*  Declarations for DBH record reading */
static PotBidder pb_rec;
static PotBidder *pb_ptr, *sel_ptr;
static Supplier supp_rec;

extern	char 	e_mesg[80];
static	int 	retval;
static 	long	longdate ;
static 	int	pgcnt;
static	short	copies;
static	short	printer;
static  char	discfile[15] ;
static 	int	outcntl ;
static	char    program[11];
static	char	resp[2] ;
static	int	mode;

/* Ranges */
static	short	tender1, tender2;
static	char	prntlabels[2];
static	short	select;
static	short	prev_tender;

struct labels {
	char sname[31];
	char sadd1[31];
	char sadd3[31];
	char spc[11];
}labels[5];

/* Defined for rotational Selection */
/* 0 - primary list, 1 - secondary list, 2 - Selected List */
#define PRIMARY		0
#define SECONDARY	1
#define SELECTED	2
#define MAX_LIST	3

static List list_hdr[MAX_LIST]; 	
static long hdr_info[MAX_LIST];

static long	seed;

static	int	cnt;
static	int	nbr_labels;

SupplierLabels()
{
	retval = get_param(&pa_rec,BROWSE,1,e_mesg);
        if(retval < 1) {
		fomer(e_mesg);
		close_dbh();
		return(-1);
	}

	STRCPY(program,"TENDREP");

#ifdef ENGLISH
	STRCPY(resp,"P");
#else
	STRCPY(resp,"I");
#endif
	retval = GetOutputon(resp);
	if(retval < 0) return(retval);

	switch( *resp){
		case DISPLAY:	/* display on terminal */
			resp[0] = 'D';
			outcntl = 0;
			break;
		case FILE_IO:	/* print to a file */
			resp[0] = 'F';
			outcntl = 1;
			break;
		case SPOOL:	/* spool report */
			outcntl = 1;
			break;
		case PRINTER:	/* print on printer */
			resp[0] = 'P';
			outcntl = 2;
			break;
		default:
			outcntl = 2;
			break;
	}
	if(outcntl == 1) {
		if(resp[0] = 'F') {
			STRCPY(e_mesg,"slabels.dat");
			if((retval = GetFilename(e_mesg)) < 0)
				return(retval);
			STRCPY(discfile,e_mesg);
		}
		else {
			sprintf(discfile,"spool%d",CC_no);
			resp[0] = 'F';
		}
	}
	else {	if(outcntl == 0 ) 
			STRCPY(discfile, terminal) ;
		else discfile[0] = '\0';
	}

	copies = 1;
	if(outcntl == 2) {
		if((retval = GetNbrCopies( &copies )) < 0)
			return(retval);
	}

	prntlabels[0] = ' ';
	retval = GetPrntLabel( prntlabels );
	if(retval < 0) return(retval);

	retval = GetTenderRange( &tender1, &tender2 );
	if(retval < 0) return(retval);
	

	if(prntlabels[0] == LABELS) {
		nbr_labels = 2;
		retval = GetNbrup( &nbr_labels );
		if(retval < 0) return(retval);

	}

	if(pa_rec.pa_rotational[0] == YES && prntlabels[0] == LABELS) {
		select = 5;
		retval = GetSelect( &select );
		if(retval < 0) return(retval);
	}

	if((retval = Confirm()) <= 0)
		return(retval);

	longdate = get_date() ;

	retval = opn_prnt(resp,discfile,1,e_mesg,0);  
	if(retval < 0) {
		fomer( e_mesg) ;
		close_dbh() ;
		return(-1);
	}
	SetCopies( (int)copies );

	LNSZ = 132;

	if(outcntl == 0) {
		PGSIZE = 22;
	}
	else {
		PGSIZE = 60;
	}

	pgcnt = 0 ;
	prev_tender = 0;
	cnt=0;
	
	if(prntlabels[0] == LABELS) {
		linecnt = 0;
		if(pa_rec.pa_rotational[0] == YES) 
			retval = PrintRotational();
		else 
			retval = PrintLabels();
		if(retval < 0) return(retval);
	}
	else {
		linecnt = PGSIZE;
		retval = PrintReport();
		if(retval < 0) return(retval);
	}


	if(pgcnt) {
		if(term < 99) 
			last_page() ;
#ifndef SPOOLER
		else
			rite_top() ;
#endif
	}
	close_rep(NOBANNER);
	close_dbh();
	if(retval == EFL) retval = NOERROR;
	return(retval);
}
PrintRotational()
{
	int	retval;
	int	i;

	retval = BuildLabelLists();
	if(retval < 0) return(retval);

	/* Show Suppliers to me */
	sel_ptr = list_get(list_hdr[SELECTED],FIRST_NODE);
	if(sel_ptr == NOLIST) return(NOERROR);
	for(;sel_ptr!=NOOBJ;sel_ptr=list_get(list_hdr[SELECTED],NEXT_NODE)) {
		strcpy(supp_rec.s_supp_cd,sel_ptr->pb_supp);
		retval = get_supplier(&supp_rec,BROWSE,0,e_mesg);
		if(retval < 0) {
			break;
		}
		retval = SetupLabels();
		if(retval < 0) return(retval);

		pb_rec.pb_categ_num = sel_ptr->pb_categ_num ;
		strcpy(pb_rec.pb_supp,sel_ptr->pb_supp);
		retval = get_potbidder(&pb_rec,UPDATE,0,e_mesg);
		if(retval < 0) {
			roll_back(e_mesg);
			return(retval);
		}

		pb_rec.pb_select++;
		retval = put_potbidder(&pb_rec,UPDATE,e_mesg);
		if(retval < 0) {
			roll_back(e_mesg);
			return(retval);
		}
		retval = commit(e_mesg);
		if(retval < 0) {
			roll_back(e_mesg);
			return(retval);
		}
	}

	/* used if more than 1 label up and not a full line */
	if(cnt != 0){
		print_labels();
	}

	list_kill(list_hdr[SELECTED]);

	return(NOERROR);
}
BuildLabelLists()
{
	int retval;
	int first, list, i;
	long	prev_cnt;
	short	curr_tender;

	list_hdr[SELECTED] = list_make(sizeof(PotBidder));
	if(list_hdr[SELECTED]==NOLIST) {
		sprintf(e_mesg,"Error Creating Link List");
		return(ERROR);
	}
	hdr_info[SELECTED] = 0;

	curr_tender = tender1;
	for( ; ; ) {
		seed = get_fulltime();

		/* Initialize Link Lists */
		for(i=0;i<SELECTED;i++) {
			list_hdr[i] = list_make(sizeof(PotBidder));
			if(list_hdr[i]==NOLIST) {
				sprintf(e_mesg,"Error Creating Link List");
				return(ERROR);
			}
			hdr_info[i] = 0;
		}

		/* Fill Linked List */
		pb_rec.pb_categ_num = curr_tender;	
		pb_rec.pb_select = 0;
		pb_rec.pb_supp[0] = '\0';
		flg_reset(POTBIDDER);

		list = 0;
		first = 0;
		for( ; ; ) {
			retval=get_n_potbidder(&pb_rec,BROWSE,1,FORWARD,e_mesg);
			if( retval < 0) {
				if(retval == EFL) break;
				return(retval);
			}

			if(first == 0) {
				curr_tender = pb_rec.pb_categ_num;
			}
			if(pb_rec.pb_categ_num > tender2) {
				retval = EFL;
				break;
			} 
			if(pb_rec.pb_categ_num != curr_tender) {
				curr_tender++;
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
		seq_over(POTBIDDER);

		/* Select Suppliers and put int Selected List */
		select_supplier(select);

		for(i=0;i<SELECTED;i++) {
			list_kill(list_hdr[i]);
		}

		if(retval == EFL)  break;
	}
	return(NOERROR);
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
PrintLabels()
{
	int	retval;

	pb_rec.pb_categ_num = tender1;
	pb_rec.pb_supp[0] = '\0';

	flg_reset( POTBIDDER );

        for( ; ; ) {
		retval = get_n_potbidder(&pb_rec,BROWSE,0,FORWARD,e_mesg);
		if(retval < 0) {
			if(retval == EFL) break;
			break;
		}

		if(pb_rec.pb_categ_num > tender2) {
			break;
		}

		strcpy(supp_rec.s_supp_cd,pb_rec.pb_supp);
		retval = get_supplier(&supp_rec,BROWSE,0,e_mesg);
		if(retval < 0) {
			break;
		}

		retval = SetupLabels();
		if(retval < 0) break;
	}
	/* used if more than 1 label up and not a full line */
	if(cnt != 0){
		print_labels();
	}

	return(NOERROR);
}
SetupLabels()
{
	/***  setup individual labels to print, check each supplier 
		address line for NULL so blank lines do not appear on
        	label. (CL) 				 ***/

	STRCPY(labels[cnt].sname,supp_rec.s_name);

	/* address label line 1 */ 

	if (strcmp(supp_rec.s_add1,"\0")){
		STRCPY(labels[cnt].sadd1,supp_rec.s_add1);
		supp_rec.s_add1[0]='\0';
	}
	else
	if (strcmp(supp_rec.s_add3,"\0")){
		STRCPY(labels[cnt].sadd1,supp_rec.s_add3);
		supp_rec.s_add3[0]='\0';
	}
	else
	if (strcmp(supp_rec.s_pc,"\0")){
		STRCPY(labels[cnt].sadd1,supp_rec.s_pc);
		supp_rec.s_pc[0]='\0';
	}
	else
		STRCPY(labels[cnt].sadd1,"     ");
	

	/* address label line 2 */ 

	if (strcmp(supp_rec.s_add1,"\0")){
		STRCPY(labels[cnt].sadd3,supp_rec.s_add1);
		supp_rec.s_add1[0]='\0';
	}
	else
	if (strcmp(supp_rec.s_add3,"\0")){
		STRCPY(labels[cnt].sadd3,supp_rec.s_add3);
		supp_rec.s_add3[0]='\0';
	}
	else
	if (strcmp(supp_rec.s_pc,"\0")){
		STRCPY(labels[cnt].sadd3,supp_rec.s_pc);
		supp_rec.s_pc[0]='\0';
	}
	else
		STRCPY(labels[cnt].sadd3,"     ");
	

	/* address label line 3 */ 

	if (strcmp(supp_rec.s_add1,"\0")){
		STRCPY(labels[cnt].spc,supp_rec.s_add1);
		supp_rec.s_add1[0]='\0';
	}
	else
	if (strcmp(supp_rec.s_add3,"\0")){
		STRCPY(labels[cnt].spc,supp_rec.s_add3);
		supp_rec.s_add3[0]='\0';
	}
	else
	if (strcmp(supp_rec.s_pc,"\0")){
		STRCPY(labels[cnt].spc,supp_rec.s_pc);
		supp_rec.s_pc[0]='\0';
	}
	else
		STRCPY(labels[cnt].spc,"     ");

	cnt++;
	if(cnt == nbr_labels) {
		print_labels();
/**
		if(linecnt >= PGSIZE) rite_top();
**/
	}
	return(NOERROR);
}
/*---------------------------------------------------------------------------
	Description:  this function sets up the number of labels to print
		      across on a page and then prints our the labels.
---------------------------------------------------------------------------*/
print_labels()
{
	int	i;

	prnt_line();
	for(i = 0; i < cnt;i++) {
		mkln(1 +(41 * i),labels[i].sname,40);
	}
	prnt_line();
	 for(i = 0; i < cnt;i++) {
		mkln(1 +(41 * i),labels[i].sadd1,30);
	}
	prnt_line();
	for(i = 0; i < cnt;i++) {
		mkln(1 +(41 * i),labels[i].sadd3,30);
	}
	prnt_line();
	for(i = 0; i < cnt;i++) {
		mkln(1 +(41 * i),labels[i].spc,7);
	}
	prnt_line();
	prnt_line();
	prnt_line();
	prnt_line();
	prnt_line();
	cnt = 0;
	return(NOERROR);
} 
PrintReport()
{
	int	retval; 

	pb_rec.pb_categ_num = tender1;
	pb_rec.pb_supp[0] = '\0';

	flg_reset( POTBIDDER );

        for( ; ; ) {
		retval = get_n_potbidder(&pb_rec,BROWSE,0,FORWARD,e_mesg);
		if(retval < 0) {
			if(retval == EFL) break;
			break;
		}

		if(pb_rec.pb_categ_num > tender2) {
			break;
		}

		strcpy(supp_rec.s_supp_cd,pb_rec.pb_supp);
		retval = get_supplier(&supp_rec,BROWSE,0,e_mesg);
		if(retval < 0) {
			break;
		}

		if(linecnt >= PGSIZE) {
			retval = PrintHeading();
			if(retval == EXIT) return(EXIT);
			if(retval < 0) {
				return(retval);
			}
		}

		retval = print_item();
		if(retval < 0) return(retval);

		prev_tender = pb_rec.pb_categ_num;

	}
	return(NOERROR);
}
static
PrintHeading()
{
	char	txt_line[80];
	long	longdate ;
	int	i ;
	int	year;

	if ( term < 99 && pgcnt )
		if (next_page() < 0) return(EXIT) ;
	
	if(pgcnt || term < 99) rite_top();   /* if not first page advance */

	linecnt = 0;

	mkln(1,PROG_NAME,10);
	longdate = get_date();
#ifdef ENGLISH
        mkln(51,"DATE:",5);
#else
        mkln(51,"DATE:",5);
#endif
	tedit((char *)&longdate,"____/__/__",txt_line,R_LONG);
	mkln(57,txt_line,10);
	pgcnt++;
#ifdef ENGLISH
	mkln(71,"PAGE:",5);
#else
	mkln(71,"PAGE:",5);
#endif
	tedit((char *)&pgcnt,"_0_",txt_line,R_INT);
	mkln(77,txt_line,3);
	if(prnt_line() < 0) return(ERROR);

	i = strlen(pa_rec.pa_co_name);
	mkln(((LNSZ-1-i)/2)+1,pa_rec.pa_co_name,sizeof(pa_rec.pa_co_name)); 
	if(prnt_line() < 0) return(ERROR);
	
#ifdef ENGLISH
	mkln(29,"POTENTIAL SUPPLIER LIST",23); 
#else
	mkln(29,"LISTE DES SOUMISSIONNAIRES POTENTIELS",37);
#endif
	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);

#ifdef ENGLISH
	mkln(1,"TENDER #",9);
	mkln(12,"SUPP CODE",9);
	mkln(30,"SUPPLIER NAME",13);
#else
	mkln(1,"#SOUMIS",7);
	mkln(12,"CODE FOURN",10);
	mkln(30,"NOM FOURN",9);
#endif
	if(prnt_line() < 0) return(ERROR);
	if(prnt_line() < 0) return(ERROR);

	return(NOERROR);
}
/****************************************************************************
****************************************************************************/
static
print_item()
{
	char	txt_line[80];

	if(prev_tender != pb_rec.pb_categ_num) {
		tedit((char *)&pb_rec.pb_categ_num,"0_",txt_line,R_SHORT);
		mkln(4,txt_line,2);
	}
	mkln(12,pb_rec.pb_supp,11);
	mkln(25,supp_rec.s_name,40);
	if(prnt_line() < 0) return(ERROR);

	return(NOERROR);
} 
/*-------------------------- END OF FILE ---------------------------------- */

