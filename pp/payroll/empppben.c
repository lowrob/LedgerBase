/* ---------------------------------------------------------------------------
*	Source 	: empppben.c 
*
*	Program to Print Employee Pay Period Benefit using REPORT GENERATOR.
*

Modifications:

	YY/MM/DD	Programmer	Description
	~~~~~~~~	~~~~~~~~~~	~~~~~~~~~~~
	92-07-18	m. galvin	Added the some checks for end of file. 
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_pp.h>
#include <bfs_com.h>
#include <repdef.h>

#define	PROJNAME	"empppben"
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

empppben()
{
Pp_ben		pp_ben;
Emp		emp;
Benefit		benefit;
Pa_rec		pa_rec;
Barg_unit	barg_unit;


char    chardate[11];
int	code;
char 	*arayptr[4] ; 	/** Declarations for Report writer usage **/
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
			STRCPY(e_mesg,"empppben.dat");
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

	strcpy(barg1,"     1");
	strcpy(barg2,"ZZZZZZ");
	retval = GetBargRange( barg1, barg2);
	if(retval < 0) return(retval);

	strcpy(posi1,"     1");
	strcpy(posi2,"ZZZZZZ");
	retval = GetPosRange( posi1, posi2);
	if(retval < 0) return(retval);

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

	arayptr[0] = (char *)&pp_ben ;
        arayptr[1] = (char *)&emp ;
        arayptr[2] = (char *)&benefit ;
        arayptr[3] = NULL ;

	/***	Read record and call report writer to output it ***/

	/* Initialize Recs. to start printing benefit report on benefit no. */
	pp_ben.pb_numb[0] = '\0';
	pp_ben.pb_pp = 0;
	pp_ben.pb_date = 0;
	pp_ben.pb_code[0] = '\0';
	pp_ben.pb_fund = 0;
	pp_ben.pb_acct[0] = '\0';

	flg_reset(PP_BEN);

	for( ; ; ) {
		code = get_n_pp_ben(&pp_ben,BROWSE,0,FORWARD,e_mesg);

		if(code == EFL)		break;
		if( code < 0) {
			fomen(e_mesg);
			get();
			roll_back(e_mesg);
			return(-1);
		}

		strcpy(emp.em_numb, pp_ben.pb_numb);

		code = get_employee(&emp,BROWSE,0,e_mesg);
		if( code < 0) {
			fomen(e_mesg);	
			get();
			return(-1);
		}

		strcpy(barg_unit.b_code,emp.em_barg);
		barg_unit.b_date = get_date();
		flg_reset(BARG);

		retval = get_n_barg(&barg_unit,BROWSE,0,BACKWARD,e_mesg);
		if(retval == EFL ||
			strcmp(barg_unit.b_code, emp.em_barg) != 0){
			sprintf(e_mesg,"Bargaining Unit does no exist: %s",
				emp.em_barg);
			fomer(e_mesg);
			return(NOERROR);
		}
		if(retval < 0){
			fomer(e_mesg);
  			return(ERROR);
		}
		seq_over(BARG);

		strcpy(benefit.bn_code,pp_ben.pb_code);
		strcpy(benefit.bn_pp_code,barg_unit.b_pp_code);

		code = get_benefit(&benefit,BROWSE,0,e_mesg);

		if(code < 0) {
			fomen(e_mesg);	
			get();
			return(-1);
		}

		if(strcmp(emp.em_barg,barg1) < 0 || 
		   strcmp(emp.em_barg,barg2) > 0) { 
			roll_back(e_mesg);
			continue;
		}

		if(strcmp(emp.em_pos,posi1) < 0 || 
		   strcmp(emp.em_pos,posi2) > 0) { 
			roll_back(e_mesg);
			continue;
		}

		if(strcmp(emp.em_numb,empl1) < 0 || 
		   strcmp(emp.em_numb,empl2) > 0) { 
			roll_back(e_mesg);
			continue;
		}

		strcpy(barg_unit.b_name,emp.em_first_name);
		strcat(barg_unit.b_name," ");
		strcat(barg_unit.b_name,emp.em_last_name);

		code = rpline(arayptr);
		if(code<0) {
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
	close_file(PP_BEN) ;
	close_file(EMPLOYEE) ;
	close_file(BENEFIT) ; 
	close_file(BARG) ; 
	rpclose();

	if(code == EFL) return(0);
	return(code);

}
