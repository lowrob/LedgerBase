/* ---------------------------------------------------------------------------
*	Source 	: adjust.c 
*
*	Program to Print Employee Pay Period Benefit using REPORT GENERATOR.
*

Modifications:

	YY/MM/DD	Programmer	Description
	~~~~~~~~	~~~~~~~~~~	~~~~~~~~~~~
	92-07-18	m. galvin	Added a check for earnings or deductions
					and calculated the differences for each
					and put the values into fields not
					used in order to display using the   
					report generator.    
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_pp.h>
#include <bfs_com.h>
#include <repdef.h>

#define	PROJNAME	"adjust"
#define	LOG_REC		1
#define	FORMNO		1
#define EXIT		12

#ifdef ENGLISH
#define PRINTER 'P'
#define DISPLAY 'D'
#define FILE_IO	'F'
#define SPOOL	'S'
#define YES	'Y'
#define NO	'N'
#else
#define PRINTER 'I'
#define DISPLAY 'A'
#define FILE_IO	'D'
#define SPOOL	'B'
#define YES	'O'
#define NO	'N'
#endif

extern	int	rperror	;
extern char	e_mesg[80] ;

adjust()
{
Aud_pay		aud_pay;
Pa_rec		pa_rec;
Emp		emp;
Barg_unit	barg_unit;

char    chardate[11];
int	code;
char 	*arayptr[3] ; 	/** Declarations for Report writer usage **/
char 	projname[50] ;
char 	program[11] ;
int 	outcntl ;
char 	discfile[20] ;
int	retval;
char    barg1[7];
char    barg2[7];
char    posi1[7];
char    posi2[7];
char    empl1[13];
char    empl2[13];

short	copies ;

short	err;

	code = get_param(&pa_rec, BROWSE, 1, e_mesg);
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
	if(retval < 0) return(retval);

	switch( *e_mesg){
		case DISPLAY:	/* display on Terminal */
			outcntl = 0;
			break;
		case FILE_IO:	/* print to a file */
		case SPOOL:	/* spool report */
			outcntl = 1;
			break;
		case PRINTER:
			outcntl = 2;
			break;
		default:
			outcntl = 2;
			break;
	}
	if(outcntl == 1) {
		if(e_mesg[0]==FILE_IO) {
			STRCPY(e_mesg,"adjust.dat");
			if((retval = GetFilename(e_mesg)) < 0)
				return(retval);
			STRCPY(discfile, e_mesg);
		}
		else {
			STRCPY(discfile,"spool.dat");
		}
	}
	else 	discfile[0] = '\0';

	copies = 1;
	if(outcntl == 2) {
		if((retval = GetNbrCopies( &copies )) < 0)
			return(retval);
	}

	strcpy(empl1,"           1");
	strcpy(empl2,"ZZZZZZZZZZZZ");
	retval = GetEmpRange( empl1, empl2);
	if(retval < 0) return(retval);

	if((retval = Confirm()) <= 0) 
		return(retval);

	mkdate(get_date(),chardate);

	STRCPY(projname,FMT_PATH) ;
	strcat(projname,PROJNAME) ;
	STRCPY(program, PROG_NAME );

	code = rpopen(projname,LOG_REC,FORMNO,outcntl,discfile,program,
			chardate);
							  	
	if ( code < 0 ){
		sprintf(e_mesg,"Rpopen code :%d\n", code ) ;
		close_dbh() ;
		return(REPORT_ERR);
	}

	if(outcntl == 2) 
		rpSetCopies( (int)copies );   /* number of copies to print */

	/* Change first title line to Company/School district name */
	if((retval = rpChangetitle(1, pa_rec.pa_co_name))<0) {
#ifdef ENGLISH
		sprintf(e_mesg,"Internal report error");
#else
		sprintf(e_mesg,"Erreur au rapport interne");
#endif
		rpclose();
		close_dbh();
		return(REPORT_ERR);
	}
	
	/* For Terminals set pagesize to 22 lines */ 
	if(outcntl == 0)
		rpPagesize(22);

        arayptr[0] = (char *)&aud_pay ;
	arayptr[1] = (char *)&barg_unit ;
        arayptr[2] = NULL ;

	/***	Read record and call report writer to output it ***/

	barg_unit.b_code[0] = '\0';
	barg_unit.b_date = 0 ;
	flg_reset(BARG);

	retval = get_n_barg(&barg_unit,BROWSE,0,FORWARD,e_mesg);
	if(retval < 0) return(retval);

	/* Initialize Recs. to start printing benefit report on benefit no. */
	aud_pay.aud_emp[0] = '\0';
	aud_pay.aud_code[0] = '\0';

	flg_reset(AUD_PAY);

	for( ; ; ) {
		code = get_n_aud_pay(&aud_pay,BROWSE,0,FORWARD,e_mesg);
		if(code == EFL) 	break;
		if( code < 0) {
			fomen(e_mesg);
			get();
			return(-1);
		}

		strcpy(emp.em_numb, aud_pay.aud_emp);

		code = get_employee(&emp,BROWSE,0,e_mesg);
		if( code < 0) {
			fomen(e_mesg);
			get();
			return(-1);
		}

		if(strcmp(emp.em_numb,empl1) < 0 || 
		   strcmp(emp.em_numb,empl2) > 0) { 
			roll_back(e_mesg);
			continue;
		}

		strcpy(barg_unit.b_name,emp.em_first_name);
		strcat(barg_unit.b_name," ");
		strcat(barg_unit.b_name,emp.em_last_name);

		if(strcmp(aud_pay.aud_flag,"E") == 0){
			barg_unit.b_stat_thrs = (aud_pay.aud_new_amnt - 
					   	 aud_pay.aud_old_amnt);
			barg_unit.b_stat_hpd = 0;
		}
		else{
			barg_unit.b_stat_thrs = 0;
			barg_unit.b_stat_hpd = (aud_pay.aud_old_amnt - 
					   	aud_pay.aud_new_amnt);
		}

		if((code = rpline(arayptr)) <0) {
			if(rperror < 0)  {
#ifdef ENGLISH
				sprintf(e_mesg,"Internal report error");
#else
				sprintf(e_mesg,"Erreur au rapport interne");
#endif
				code = REPORT_ERR;
			}
			else {
				code = EXIT ;
				break;
			}
		}
	}
	rpclose();
	close_file(AUD_PAY) ;
	close_file(EMPLOYEE) ;
	close_file(BARG);

	if(code == EFL) return(0);
	return(code);

}
