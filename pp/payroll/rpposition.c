/* ---------------------------------------------------------------------------
*	Source 	: rpposition.c 
*
*	Program to Print Employee Pay Period Benefit using REPORT GENERATOR.
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

#define	PROJNAME	"rpposition"
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

static	double	total_income;
extern	int	rperror	;
extern char	e_mesg[80] ;

rpposition()
{
Pay_earn	pay_earn;
Emp		emp;
Position	position;
Pa_rec		pa_rec;


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
			STRCPY(e_mesg,"position.dat");
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

	strcpy(barg1,"     0");
	strcpy(barg2,"ZZZZZZ");
	retval = GetBargRange( barg1, barg2);
	if(retval < 0) return(retval);

	strcpy(posi1,"     0");
	strcpy(posi2,"ZZZZZZ");
	retval = GetPosRange( posi1, posi2);
	if(retval < 0) return(retval);

	strcpy(empl1,"           1");
	strcpy(empl2,"999999999999");
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
	
	/* For Terminals set pagesize to 23 lines */ 
	if(outcntl == 0)
		rpPagesize(23);

        arayptr[0] = (char *)&position ;
	arayptr[1] = (char *)&pay_earn ;
        arayptr[2] = NULL ;

	/***	Read record and call report writer to output it ***/

	/* Initialize Recs. to start printing benefit report on benefit no. */
	position.p_code[0] = '\0';

	flg_reset(POSITION);

	for( ; ; ) {
		retval = get_n_position(&position,BROWSE,0,FORWARD,e_mesg);

		if( retval < 0) {
			if(retval == EFL)  break ;
			retval = DBH_ERR;
			break ;
		}

		total_income = 0;

		strcpy(emp.em_pos, position.p_code);
		flg_reset(EMPLOYEE);

		for(;;) {
			code = get_n_employee(&emp,BROWSE,1,FORWARD,e_mesg);
			if( code < 0) {
				if(code == EFL) break;
				code = DBH_ERR;
				break;
			}
			if(strcmp(emp.em_pos, position.p_code) != 0)	break;

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

			strcpy(pay_earn.pe_numb,emp.em_numb);
			pay_earn.pe_pp = 0;
			pay_earn.pe_date = 0;

			flg_reset(PP_EARN);

			for(;;)  {
				code = get_n_pp_earn(&pay_earn,BROWSE,0,FORWARD
					,e_mesg);

				if(code < 0) {
					if(code == EFL) break;
					code = DBH_ERR;
					break;
				}
				if(strcmp(emp.em_numb,pay_earn.pe_numb) != 0)
					break;

				total_income += (pay_earn.pe_reg_inc1 +
						 pay_earn.pe_reg_inc2 +
						 pay_earn.pe_high_inc +
						 pay_earn.pe_ben +
						 pay_earn.pe_vac);
	
			}
		}
		if(retval == EFL )
			break;

		pay_earn.pe_vac = total_income;

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
	close_file(PP_EARN) ;
	close_file(EMPLOYEE) ;
	close_file(POSITION) ;

	if(code == EFL) return(0);
	return(code);

}
