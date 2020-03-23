/* ---------------------------------------------------------------------------
*	Source 	: cheqreg.c 
*
*	Program to Print the Cheque Register.
*

Modifications:

	YY/MM/DD	Programmer	Description
	~~~~~~~~	~~~~~~~~~~	~~~~~~~~~~~
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_pp.h>
#include <bfs_com.h>
#include <repdef.h>

#define	PROJNAME	"cheqreg"
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

#define INTERACTIVE	0
extern	int	rperror	;
extern char	e_mesg[80] ;

cheqreg(mode)
int	mode;
{
Chq_reg		chq_reg;
Emp		emp;
Pa_rec		pa_rec;
Barg_unit	barg_unit;

char 	*arayptr[3] ; 	/** Declarations for Report writer usage **/
int 	outcntl ;
char 	discfile[20] ;

char    chardate[11];
int	code;
char 	projname[50] ;
char 	program[11] ;

short	copies ;
int	retval;

short	err;

	code = get_param(&pa_rec, BROWSE, 1, e_mesg);
 	if (code < 1) {
		printf("%s\n", e_mesg);
		close_dbh();
		return(-1);
	}

#ifdef ENGLISH
	STRCPY(e_mesg, "F");
#else
	STRCPY(e_mesg, "F");
#endif
	if(mode == INTERACTIVE){
		retval = GetOutputon(e_mesg);
		if(retval < 0) return(retval);
	}

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
			STRCPY(e_mesg,"cheqreg.dat");
			if((retval = GetFilename(e_mesg)) < 0)
				return(retval);
			STRCPY(discfile, e_mesg);
		}
		else {
			STRCPY(discfile,"spool.dat");
		}
	}
	else{
			STRCPY(e_mesg,"cheqreg.dat");
			STRCPY(discfile, e_mesg);
	}

	copies = 1;
	if(outcntl == 2 && mode == INTERACTIVE) {
		if((retval = GetNbrCopies( &copies )) < 0)
			return(retval);
	}

	if(mode == INTERACTIVE){
		if((retval = Confirm()) <= 0) 
			return(retval);
	}

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

        arayptr[0] = (char *)&chq_reg ;
        arayptr[1] = (char *)&barg_unit ;
        arayptr[2] = NULL ;

	/***	Read record and call report writer to output it ***/

	barg_unit.b_code[0] = '\0';
	barg_unit.b_date = 0 ;
	flg_reset(BARG);

	retval = get_n_barg(&barg_unit,BROWSE,0,FORWARD,e_mesg);
	if(retval < 0) return(retval);

	/* Initialize Recs. to start printing benefit report on benefit no. */
	chq_reg.cr_numb = 0;
	chq_reg.cr_date = 0;

	flg_reset(CHQ_REG);

	for( ; ; ) {
		code = get_n_chq_reg(&chq_reg,BROWSE,0,FORWARD,e_mesg);

		if( code < 0) {
			if(code == EFL)  break ;
			code = DBH_ERR;
			return(code) ;
		}
		strcpy(emp.em_numb, chq_reg.cr_emp_numb);

		code = get_employee(&emp,BROWSE,0,e_mesg);
/*		if( code < 0) {
			if(code == UNDEF) break;
			code = DBH_ERR;
			return(code);
		} */

		strcpy(barg_unit.b_name,emp.em_first_name);
		strcat(barg_unit.b_name," ");
		strcat(barg_unit.b_name,emp.em_last_name);

		if(chq_reg.cr_status[0] == 'O') {
			strcpy(barg_unit.b_cpp_acct,"Outstanding");
		}
		else {
			if(chq_reg.cr_status[0] == 'X') {
				strcpy(barg_unit.b_cpp_acct,"Cancelled");
			}
			else {
				strcpy(barg_unit.b_cpp_acct,"Cashed");
			}
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
	close_file(CHQ_REG) ;

	if(code == EFL) return(0);
	return(0);

}
