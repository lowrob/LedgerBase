/****
*	GST report by supplier
*	Source 	: suppgst.c 
*	Modifications : 
*	MODIFIED: FEB 21, 1991
*	DESC	: 
*
* 	MODIFICATIONS:
*
*	Programmer	YY/MM/DD	Description
*	~~~~~~~~~~	~~~~~~~~	~~~~~~~~~~~
****/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	EXIT	12

#ifdef ENGLISH
#define PRINTER 'P'
#define DISPLAY 'D'
#define FILE_IO	'F'
#else
#define PRINTER 'I'
#define DISPLAY 'A'
#define FILE_IO	'D'
#endif

gstbysupp() 
{
static 	char	chardate[11] ;

Tr_hdr		tr_hdr ; /**** 	Declarations for DBH record reading  ****/
Tr_item		tr_item ;
Supplier	supp_rec ;
Pa_rec		pa_rec ;
Ctl_rec		ctl_rec ;

int	code ;
char	e_mesg[80] ;
char 	*arayptr[5] ; 	/**** 	Declarations for Report writer usage ****/
char 	projname[50] ;
int 	logrec ;
int 	formno ;
int 	outcntl ;
short	copies ;
char 	discfile[20] ;
char	program[11];
int	retval;
extern	int	rperror ;

/****	
*	Accept Project code, Logical record number in the 
*	Project and format number for the logical record ..
*	Accept the output media code . Open the report ..
****/
#ifdef ENGLISH
	STRCPY(program,"SUPP_GST");
#else
	STRCPY(program,"SUPP_GST");
#endif

	STRCPY(projname,FMT_PATH) ;
	strcat(projname, "supp_gst");
	logrec = 1;
	formno = 1;

	code = get_param( &pa_rec, BROWSE, 1, e_mesg);
	if (code < 1) {
		printf("%s\n", e_mesg);
		close_dbh();
		return(-1);
	}

#ifdef ENGLISH
	STRCPY(e_mesg, "P");
#else
	STRCPY(e_mesg, "I");
#endif
	retval = GetOutputon(e_mesg);
	if (retval < 0) return(-1);
	else if (retval == EXIT) return(0);

	switch (*e_mesg) {
		case DISPLAY :	/*  Display on the terminal */
			outcntl = 0;
			break;
		case FILE_IO :	/*  Print to a file */
			outcntl = 1;
			break;
		case PRINTER :	/*  Print to the printer */
			outcntl = 2;
			break;
		default:
			outcntl = 2;
			break;
	}

	if(outcntl == 1)
		{
		STRCPY(e_mesg, "supp_gst.dat");
		if(GetFilename( e_mesg ) < 0) return(-1);
		STRCPY(discfile, e_mesg);
	}
	else
		discfile[0] = '\0' ;
	
	copies = 1;
	if(outcntl == 2) {
		if((retval == GetNbrCopies( &copies )) < 0)
			return(retval);
	}

	if ((retval = Confirm()) < 0) return(-1);
	else if (!retval) return(0);

	mkdate(get_date(), chardate) ;

	code = rpopen(projname,logrec,formno,outcntl,discfile, 
		program,chardate);
							  	
	if ( code < 0 ){
		sprintf(e_mesg, "Rpopen code :%d\n", code ) ;
		fomen(e_mesg);
		get();
		return(-1);
	}

	if(outcntl == 2)
		rpSetCopies( (int)copies );	/* number of copies to print */

 	rpChangetitle(1, pa_rec.pa_co_name); 

	/* For Terminals set pagesize to 23 lines */
	if(outcntl == 0)
		rpPagesize(23);

/***	Prepare to read transaction sequentialy ***/

	tr_hdr.th_supp_cd[0] = '\0';
	tr_hdr.th_create[0] = '\0';
	tr_hdr.th_seq_no = 0;
	flg_reset(GLTRHDR);

/***	Initialise the pointer array's first element to the record ***/

	arayptr[0] = (char *)&tr_hdr ; 
	arayptr[1] = (char *)&tr_item ; 
	arayptr[2] = (char *)&supp_rec ; 
	arayptr[3] = NULL ;

/***	Read record and call report writer to output it ***/

	for(;;) {
		code = get_n_trhdr(&tr_hdr, BROWSE, 2, 0, e_mesg) ;
		if(code < 0) break ;

		/* if no supplier code skip record */
		if (tr_hdr.th_supp_cd[0] == '\0')
			continue;

		/* Only accept tr_hdr.th_create equal to G */
		if (tr_hdr.th_create[0] != 'G')
			continue;

		if(strcmp(tr_hdr.th_supp_cd,supp_rec.s_supp_cd) !=0) {
			STRCPY(supp_rec.s_supp_cd, tr_hdr.th_supp_cd);
			code = get_supplier(&supp_rec, BROWSE, 0 , e_mesg);
			if(code < 0) 
				STRCPY(supp_rec.s_name,"????????????????????");
		}

		if(tr_hdr.th_fund != ctl_rec.fund) {
			ctl_rec.fund = tr_hdr.th_fund;
			code = get_ctl(&ctl_rec, BROWSE, 0 , e_mesg);
			if(code < 0)  {
				fomer(e_mesg); get();
				break;
			}
		}

		tr_item.ti_fund = tr_hdr.th_fund;
		tr_item.ti_reccod = tr_hdr.th_reccod;
		tr_item.ti_create[0] = tr_hdr.th_create[0];
		tr_item.ti_seq_no = tr_hdr.th_seq_no ;
		tr_item.ti_item_no = 0 ;
		flg_reset(GLTRAN);

 		for (;;) {
#ifndef	ORACLE
			code = get_n_tritem(&tr_item,BROWSE,0,FORWARD,e_mesg) ;
#else
			code = get_n_tritem(&tr_item,BROWSE,0,EQUAL,e_mesg) ;
#endif
			if(code < 0) {
				if (code == EFL) break;
				fomer(e_mesg); get(); 
				break ;
			}

#ifndef	ORACLE
			if (tr_item.ti_fund != tr_hdr.th_fund ||
			   tr_item.ti_reccod != tr_hdr.th_reccod ||
			   tr_item.ti_create[0] != tr_hdr.th_create[0] ||
			   tr_item.ti_seq_no != tr_hdr.th_seq_no)
				break ;
#endif
			if(strcmp(tr_item.ti_accno,ctl_rec.gst_tax_acnt)!=0)
				continue;

			if(rpline(arayptr) < 0)  {
				if(rperror < 0)  {
#ifdef ENGLISH
				sprintf(e_mesg,"Internal report error");
#else
				sprintf(e_mesg,"Erreur au rapport interne");
#endif
					code = REPORT_ERR;
				}
				else 
					code = EXIT;
				break ;
			}
		}
		if((code < NOERROR || code == EXIT) && code != EFL) break;
		seq_over(GLTRAN) ;
	}

	close_dbh();
	rpclose();
	return(0);	
}
