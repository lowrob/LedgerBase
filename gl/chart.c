/****
*	Chart Of Account Report
*	Source 	: Chart.c 
*	Compile	: cc -I$I chart.c
*	Linking	: cc trial.o rp.o /usr/bfs/dbh/libbfs.a -o trial.out
*	Make	: make -f makechart
*	Modifications : rpopen has more paramaters for new rp library
*
* 1993/06/28	N.Mckee	Add summary option for the chart of accounts
****/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>
#include <section.h>


#define	PROG	"CHART"
#define	PROJECT	"trial"
#define	LOGREC	1 

#define	ALTKEY	3 	/* read on 3rd ALt key for section sequence */

#ifdef ENGLISH
#define PRINTER		'P'
#define FILE_IO		'F'
#define DISPLAY		'D'
#define	YES		'Y'
#else
#define PRINTER		'I'
#define FILE_IO		'D'
#define DISPLAY		'A'
#define	YES		'O'
#endif

ChartOfAcc() 
{
	Gl_rec	gl_rec ; 	
	Ctl_rec	ctl_rec	; 
	Pa_rec	pa_rec	;
	int	code	;
	char	e_mesg[135] ;
	char	*arayptr[5] ; 	
	char	projname[50] ;
	int	outcntl ;
	short	copies ;
	char	discfile[20] ;
	long	get_date() ;
	char	summary[2];
	int	formno;
	int 	retval;	

	STRCPY(projname,FMT_PATH) ;
	strcat(projname,PROJECT) ;
	
#ifdef ENGLISH
	STRCPY(discfile, "P");
#else
	STRCPY(discfile, "I");
#endif
	GetOutputon(discfile) ;

	if ( discfile[0] == DISPLAY )
		outcntl = 0 ;
	else if ( discfile[0] == PRINTER )
		outcntl = 2 ;
	else {
		outcntl = 1 ;
		STRCPY(discfile, "chart.dat");
		GetFilename( discfile ) ;
	}
	
	copies = 1;
	if(outcntl == 2) {
		if((code == GetNbrCopies( &copies )) < 0)
			return(code);
	}

#ifdef ENGLISH
	if((retval = DisplayMessage("Summary (Y/N)?")) < 0) return(retval);
	if((retval = GetResponse(summary, "YN")) < 0) return(retval);
#else
	if((retval = DisplayMessage("Resume (O/N)?")) < 0) return(retval);
	if((retval = GetResponse(summary, "ON")) < 0) return(retval);
#endif
	if(summary[0] == YES) {
		formno = 4;
	}
	else{
		formno = 3;
	}

	code  = Confirm() ;
	if ( !code ) return(0) ;
 
	mkdate( get_date(), e_mesg) ;

	code = rpopen(projname, LOGREC, formno, outcntl, discfile, PROG,e_mesg);
							  	
	if ( code < 0 ){
		fomen( "Rpopen code :%d\n", code ) ;
		get();
		return(0) ;
	}

	if(outcntl == 2)
		rpSetCopies( (int)copies );	/* number of copies to print */

	code	= get_param( &pa_rec, BROWSE, 1, e_mesg) ;

	rpChangetitle(1, pa_rec.pa_co_name) ;

	i_space(e_mesg) ;
#ifdef ENGLISH
	STRCPY(e_mesg+69, "------------------- KEY VALUES -------------------");
#else
	STRCPY(e_mesg+69, "---------------- VALEUR DES CLES -----------------");
#endif
	rpAddSubtitle(e_mesg) ;

	/* for terminals set page size to 23 */
	if(outcntl == 0)
		rpPagesize(23);

/***	Prepare to read gl_mast sequentialy ***/

	gl_rec.funds = 0;
	gl_rec.reccod = 0;
	gl_rec.sect = 0 ;
	gl_rec.accno[0] = '\0';

	ctl_rec.fund = 0;

/***	Initialise the pointer array's first element to the record ***/

	arayptr[0] = (char *)&gl_rec ; 
	arayptr[1] = (char *)&ctl_rec ; 
	arayptr[2] = (char *)&sect_rec[0] ;
	arayptr[3] = NULL ;

/***	Read record and call report writer to output it ***/

	for(;;) {

		code = get_n_gl(&gl_rec, BROWSE, ALTKEY, 0, e_mesg) ;

		if(code < 0) break ;
		
		if(gl_rec.funds != ctl_rec.fund) {
			ctl_rec.fund = gl_rec.funds ;
			code = get_ctl(&ctl_rec, BROWSE, 0, e_mesg) ;
			if(code < 0) break ;
		}

		arayptr[2] = (char *)&sect_rec[gl_rec.sect] ;

		if(rpline(arayptr) < 0) break ;
	}


/****	Windup .. ****/

	close_dbh() ;
	rpclose() ;
	return(0) ;
   
}

