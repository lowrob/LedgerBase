/****
*	Trial Balance Report ..
*	Source 	: trial.c 
*	Compile	: cc -I$I trial.c
*	Linking	: cc trial.o rp.o /usr/bfs/dbh/libbfs.a -o trial.out
*	Modifications : rpopen has more paramaters for new rp library
****/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>
#include <section.h>

#define	EXIT	12

#ifdef ENGLISH
#define PRINTER 'P'
#define DISPLAY 'D'
#define FILE_IO	'F'
#define YES	'Y'
#else
#define PRINTER 'I'
#define DISPLAY 'A'
#define FILE_IO	'D'
#define YES	'O'
#endif

typedef struct
{
	char glacctno[19]; 	/* gl account number */
} Acct_rec;

trial() 
{
static	char	chardate[12] ;

Gl_rec	gl_rec ; 	/**** 	Declarations for DBH record reading  ****/
Ctl_rec	ctl_rec ; 	/**** 	Declarations for DBH record reading  ****/
Pa_rec	pa_rec ;	/****   Declarations for DBH record reading  ****/
Acct_rec acct_rec ;

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
short 	period, fund1, fund2;
int	err, i ;
char	sub_title[132];
char	flag1[2], flag2[2];
	
/****	
*	Accept Project code, Logical record number in the 
*	Project and format number for the logical record ..
*	Accept the output media code . Open the report ..
****/
 	err = get_param(&pa_rec,BROWSE,1,e_mesg);
	if (err == ERROR) {
		fomen(e_mesg);
		get();
	}
	else
	if (err == UNDEF) {
#ifdef ENGLISH
		fomen("Parameters are not Setup ...");
#else
		fomen("Parametres ne sont pas etablis... ");
#endif
		get();
	}
	       
	period = pa_rec.pa_cur_period ;
	STRCPY(program,"TRIAL");
	STRCPY(projname,FMT_PATH) ;
	strcat(projname,"trial") ;
	logrec = 1 ;
	formno = 1;

 	code = get_param(&pa_rec,BROWSE,1,e_mesg);
	if (code < 1) {
		printf("%s\n", e_mesg);
		close_dbh();
		return(-1);
	}

#ifdef ENGLISH
	STRCPY( e_mesg, "P");
#else
	STRCPY( e_mesg, "I");
#endif
	retval = GetOutputon( e_mesg);
	if (retval < 0 ) return(-1);
	else if (retval == EXIT) return(0);
        switch( *e_mesg ) {
		case DISPLAY :	/*  Display on the terminal */
			outcntl = 0 ;
			break;
		case FILE_IO :	/*  Print to a file */
			outcntl = 1 ;
			break;
		case PRINTER :	/*  Print to a Printer */
			outcntl = 2 ;
			break;
		default:
			outcntl = 2 ;
			break;
	}
	if(outcntl == 1 ) {
		STRCPY( e_mesg, "trial.dat");
		if(GetFilename( e_mesg ) < 0) return(-1);
 		STRCPY( discfile, e_mesg) ;	
	}
	else
		discfile[0] = '\0' ;

	copies = 1;
	if(outcntl == 2) {
		if((retval == GetNbrCopies( &copies )) < 0)
			return(retval);
	}

	fund1 = 1;
	fund2 = 999;
	retval = GetFundRange( &fund1, &fund2);
	if (retval < 0) return(-1);
	else if (retval == EXIT) return(0);

	retval = GetPeriod(&period);
	if (retval < 0) return(-1);
	else if (retval == EXIT) return(0);

#ifdef ENGLISH
	DisplayMessage("Do you want to report on commitments (Y/N)?");
#else
	DisplayMessage("Desirez-vous faire rapport sur les engagements (O/N)?");
#endif
	GetResponse( flag1 );
	if (flag1[0] == YES) 
		formno = 2;

#ifdef ENGLISH
	DisplayMessage("Summary (Y/N)? ");
#else
	DisplayMessage("Resume (O/N)? ");
#endif
	GetResponse( flag2 );

	if (( retval = Confirm()) < 0) return(-1);
	else if (!retval) return(0);

	mkdate(get_date(),chardate);
							  	
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

#ifdef ENGLISH
	sprintf(e_mesg,"For the End of Period %d",period);
#else
	sprintf(e_mesg,"Pour la fin de periode %d",period);
#endif
	code = rpChangetitle(3,e_mesg);
	i_space(sub_title);
	if (flag1[0] == YES) 
#ifdef ENGLISH
		sprintf(&sub_title[59],"- - - - - - -  PERIOD  - - - - - - -       CUMULATIVE");
 	else	
		sprintf(&sub_title[70],"- - - - - - - -  PERIOD  - - - - - - - -      CUMULATIVE");
#else
		sprintf(&sub_title[59],"- - - - - - -  PERIODE - - - - - - -       CUMULATIF");
 	else	
		sprintf(&sub_title[70],"- - - - - - - -  PERIODE - - - - - - - -      CUMULATIF");
#endif
	rpAddSubtitle(sub_title);

/***	Prepare to read gl_mast sequentialy ***/

	gl_rec.funds = 0;
	gl_rec.reccod = 0;
	gl_rec.sect = 0 ;
	gl_rec.accno[0] = '\0';

	ctl_rec.fund = 0;

/***	Initialise the pointer array's first element to the record ***/

	arayptr[0] = (char *)&gl_rec ; 
	arayptr[1] = (char *)&ctl_rec ; 
	arayptr[2] = NULL ;
	arayptr[3] = (char *)&acct_rec;
	arayptr[4] = NULL ;

/***	Read record and call report writer to output it ***/

	if (flag2[0] == YES) 
		rpSummaryOn() ;

	for(;;) {
		code = get_n_gl(&gl_rec, BROWSE, 3, 0, e_mesg) ;
		if(code < 0) break ;
		
		if(gl_rec.reccod != 99) continue ;
		if(gl_rec.funds < fund1 || fund2 < gl_rec.funds) continue;
		if(gl_rec.funds != ctl_rec.fund) {
			ctl_rec.fund = gl_rec.funds ;
			code = get_ctl(&ctl_rec, BROWSE, 0, e_mesg) ;
			if(code < 0) break ;
		}
		/* Skip Surplus&Deficit account for trial balance report */
		if(strcmp(gl_rec.accno, ctl_rec.p_and_l_acnt) == 0) continue ;

		gl_rec.ytd = 0 ;
		for (i=0;i < period;i++)
			gl_rec.ytd += gl_rec.currel[i] ;

		gl_rec.currel[0] = gl_rec.currel[period - 1] ;
		if (period != pa_rec.pa_cur_period) {
			gl_rec.curcr = 0;
			gl_rec.curdb = 0;
		}

		arayptr[2] = (char *)&sect_rec[gl_rec.sect] ;
		STRCPY(acct_rec.glacctno,gl_rec.accno);
		acct_rec.glacctno[15] = '0';
		acct_rec.glacctno[16] = '0';
		acct_rec.glacctno[17] = '0';
		
		if(rpline(arayptr) < 0) break ;
	}


/****	Windup .. ****/

	close_dbh() ;
	rpclose() ;
	return(0);
   
}

