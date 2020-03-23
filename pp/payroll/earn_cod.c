/******************************************************************************
		Sourcename   : earn_cod.c
		System       : Budgetary Financial system.
		Module       : Fixed Assets System : Reports
		Created on   : 91-11-04
		Created  By  : Andre Cormier 
		Cobol Source : 

******************************************************************************
About the file:	
	This file has routines to print Fixed Asset Report. 
	It is called by the file pprep.c .

History:
Programmer      Last change on    Details

******************************************************************************/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_pp.h>
#include <bfs_com.h>
#include <repdef.h>

#define CONTINUE	10
#define EXIT		12

#ifdef ENGLISH
#define PRINTER 	'P'
#define DISPLAY		'D'
#define FILE_IO		'F'
#else
#define PRINTER		'I'

#define DISPLAY		'A'
#define FILE_IO		'D'
#endif

static Pa_rec		pa_rec;
static Emp		emp;
static Emp_sched1	emp_sched1;
static Class		class;
static Time		time;
static Pay_param	pay_param;
static Earn		earn;

/*  Data items for storing the key range end values */
static char	barg1[7];
static char	barg2[7];
static char	posi1[7];
static char	posi2[7];
static char	empl1[13];
static char	empl2[13];
	
static int	flag;		/* flag to print the employee or not */
static int	PG_SIZE;
static int	retval;
static char 	discfile[15];	/* for storing outputfile name */
static short	pgcnt; 		/* for page count */
static char	resp[2];	/* for storing response */
static short	copies;
static double	tot_week_units;	/* total for week units */
static double	tot_week_amt;	/* total for week amount */
static double	tot_per_units;	/* total for period units */
static double	tot_per_amt;	/* total for period amount */
static double	tot_emp_units;	/* total for all employees units */
static double	tot_emp_amt;	/* total for all employees amount */

extern char 	e_mesg[80];	/* for storing error messages */

earn_cod()
{

	/* Get details for output medium */
#ifdef ENGLISH
	STRCPY(resp,"P");
#else
	STRCPY(resp,"I");
#endif
	if((retval =  GetOutputon(resp))<0 )
		return(retval);

	switch(*resp) {
		case DISPLAY:
			resp[0]='D';
			STRCPY( discfile, terminal );
			PG_SIZE = 22;
			break;
		case FILE_IO:
			resp[0]='F';
			STRCPY( discfile, "earn_cod.dat" );
			if((retval = GetFilename(discfile))<0 )
				return(retval);
			PG_SIZE = 60;
			break;
		case PRINTER:
		default:
			resp[0]='P';
			discfile[0]= '\0';
			PG_SIZE = 60;
			break;
	}

	copies = 1;
	if( *resp=='P' ){
		if((retval = GetNbrCopies(&copies))<0 )
			return(retval);
	}

	strcpy( barg1, "     0");
	strcpy( barg2, "ZZZZZZ" );
	retval = GetBargRange( barg1,barg2 );
	if(retval < 0) return(retval);

	strcpy( posi1, "     0");
	strcpy( posi2, "ZZZZZZ" );
	retval = GetPosRange( posi1,posi2 );
	if(retval < 0) return(retval);

	strcpy( empl1, "           1");
	strcpy( empl2, "ZZZZZZZZZZZZ" );
	retval = GetEmpRange( empl1,empl2 );
	if(retval < 0) return(retval);

	if( (retval=Confirm())<=0 )
		return(retval);

	retval = get_param( &pa_rec, BROWSE, 1, e_mesg );
	if( retval<1 ){
		return(DBH_ERR);
	}

	if( opn_prnt( resp, discfile, 1, e_mesg, 1 /* spool */)<0 ){
		return(REPORT_ERR);
	}
	if( *resp=='P' )
		SetCopies( (int)copies );
	pgcnt = 0;		/* Page count is zero */
	LNSZ = 132;		/* line size in no. of chars */
	linecnt = PG_SIZE;	/* Page size in no. of lines */
	tot_week_units = 0;
	tot_week_amt = 0;

	retval = PrintFirst();
	if(retval == EXIT)	 return(retval);

	retval = PrintSecond();
	if(retval == EXIT)	 return(retval);

	close_rep();	/* close output file */
	close_dbh();
	return(retval);
}
/******************************************************************************
Main logic of the program
******************************************************************************/
static
PrintFirst()
{
	int	old_week;
	char	old_emp_numb[13];

	flag = 0;

	retval = get_pay_param( &pay_param, BROWSE, 1, e_mesg );
	if( retval<1 ){
		return(DBH_ERR);
	}

	strcpy(earn.ea_code,pay_param.pr_reg_earn);
	earn.ea_date = get_date();

	flg_reset(EARN);

	get_n_earn(&earn,BROWSE,1,BACKWARD,e_mesg);
	if(strcmp(earn.ea_code,pay_param.pr_reg_earn) != 0) {
		fomen("EARNINGS CODE DOES NOT EXIST");
		get();
		return(ERROR);
	}

	retval = PrntHdg();
	if(retval == EXIT)	return(retval);
	retval = PrntHdgDet();
	if(retval < 0)	return(retval);

	emp_sched1.es_numb[0] = '\0';
	emp_sched1.es_week = 0;
	emp_sched1.es_fund = 0;
	emp_sched1.es_class[0] = '\0';

	flg_reset(EMP_SCHED1);

	for(;;) {
		retval = get_n_emp_sched1(&emp_sched1,BROWSE,0,FORWARD,e_mesg);

		if(retval ==EFL) 
			break;

		strcpy(emp.em_numb,emp_sched1.es_numb);

		retval = get_employee(&emp,BROWSE,0,e_mesg);
		if(retval < 0) {
			fomen(e_mesg);
			get();
			return(ERROR);
		}

		if(strcmp(emp.em_barg,barg1) < 0 ||
		   strcmp(emp.em_barg,barg2) > 0) {
			continue;
		}

		if(strcmp(emp.em_pos,posi1) < 0 ||
		   strcmp(emp.em_pos,posi2) > 0) {
			continue;
		}

		if(strcmp(emp.em_numb,empl1) < 0 ||
		   strcmp(emp.em_numb,empl2) > 0) {
			continue;
		}
		break;
	}

	strcpy(time.tm_earn,pay_param.pr_reg_earn);
	time.tm_numb[0] = '\0';
	time.tm_week = 0;
	time.tm_date = 0;
	time.tm_no = 0;

	flg_reset(TIME);

	for(;;) {
		retval = get_n_ptime(&time,BROWSE,2,FORWARD,e_mesg);

		if(retval ==EFL) 
			break;

		strcpy(emp.em_numb,time.tm_numb);

		retval = get_employee(&emp,BROWSE,0,e_mesg);
		if(retval < 0) {
			fomen(e_mesg);
			get();
			return(ERROR);
		}

		if(strcmp(emp.em_barg,barg1) < 0 ||
		   strcmp(emp.em_barg,barg2) > 0) {
			continue;
		}

		if(strcmp(emp.em_pos,posi1) < 0 ||
		   strcmp(emp.em_pos,posi2) > 0) {
			continue;
		}

		if(strcmp(emp.em_numb,empl1) < 0 ||
		   strcmp(emp.em_numb,empl2) > 0) {
			continue;
		}
		break;
	}

	if(strcmp(emp_sched1.es_numb,time.tm_numb) <= 0) {
		old_week = emp_sched1.es_week;
		strcpy(old_emp_numb,emp_sched1.es_numb);
	}
	else {
		old_week = time.tm_week;
		strcpy(old_emp_numb,time.tm_numb);
	}

	for(;;) {
		if(old_week != emp_sched1.es_week &&
		   old_week != time.tm_week) {

			if((retval = PrntTotWeek()) < 0)
				return(retval);
			if(retval == EXIT)	return(retval);

			if(strcmp(emp_sched1.es_week,time.tm_week) <= 0) {
				old_week = emp_sched1.es_week;
			}
			else {
				old_week = time.tm_week;
			}
		}
		if(strcmp(time.tm_numb,"999999999999") == 0 && 
	   	   strcmp(emp_sched1.es_numb,"999999999999") == 0)
			break;

		if( emp_sched1.es_week <= time.tm_week &&
			strcmp(emp_sched1.es_numb,time.tm_numb) <= 0) {

			strcpy(class.c_code,emp_sched1.es_class);
			class.c_date = get_date();

			flg_reset(CLASSIFICATION);

			retval = get_n_class(&class,BROWSE,0,BACKWARD,e_mesg);

			if(strcmp(emp_sched1.es_class,class.c_code) != 0){
				fomen("CLASIFICATION CODE DOES NOT EXIST");
				get();
				return(ERROR);
			}

			strcpy(emp.em_numb,emp_sched1.es_numb);

			retval = get_employee(&emp,BROWSE,0,e_mesg);
			if(retval < 0) {
				fomen(e_mesg);
				get();
				return(ERROR);
			}

			if((retval = PrntEmpSched())<0 )
				return(retval);
			if(retval == EXIT)	return(retval);
			
			for(;;) {
				retval = get_n_emp_sched1(&emp_sched1,BROWSE,0,
					FORWARD,e_mesg);

				if(retval ==EFL) {
					emp_sched1.es_week = 9;
					strcpy(emp_sched1.es_numb,"999999999999");
					break;
				}

				strcpy(emp.em_numb,emp_sched1.es_numb);

				retval = get_employee(&emp,BROWSE,0,e_mesg);
				if(retval < 0) {
					fomen(e_mesg);
					get();
					return(ERROR);
				}

				if(strcmp(emp.em_barg,barg1) < 0 ||
				   strcmp(emp.em_barg,barg2) > 0) {
					continue;
				}

				if(strcmp(emp.em_pos,posi1) < 0 ||
				   strcmp(emp.em_pos,posi2) > 0) {
					continue;
				}

				if(strcmp(emp.em_numb,empl1) < 0 ||
				   strcmp(emp.em_numb,empl2) > 0) {
					continue;
				}
				break;
			}

			if(strcmp(emp_sched1.es_numb,old_emp_numb) != 0 &&
			   strcmp(time.tm_numb,old_emp_numb) != 0) {

				if((retval = PrntTotWeek()) < 0)
					return(retval);
				if(retval == EXIT)	return(retval);

				if((retval = PrntTotPer()) < 0)
					return(retval);
				if(retval == EXIT)	return(retval);

				old_week = emp_sched1.es_week;
				strcpy(old_emp_numb,emp_sched1.es_numb);	

				if(strcmp(old_emp_numb,time.tm_numb) > 0) {
					old_week = time.tm_week;
					strcpy(old_emp_numb,time.tm_numb);	
				}
			}


		}
		else {
			strcpy(class.c_code,time.tm_class);
			class.c_date = get_date();

			flg_reset(CLASSIFICATION);

			retval = get_n_class(&class,BROWSE,0,BACKWARD,e_mesg);

			if(strcmp(time.tm_class,class.c_code) != 0){
				fomen("CLASSIFICATION CODE DOES NOT EXIST");
				get();
				return(ERROR);
			}

			strcpy(emp.em_numb,time.tm_numb);

			retval = get_employee(&emp,BROWSE,0,e_mesg);
			if(retval < 0) {
				fomen(e_mesg);
				get();
				return(ERROR);
			}

			if(strcmp(time.tm_earn,pay_param.pr_reg_earn) == 0) {
				if((retval = PrntTime())<0 )
					return(retval);
				if(retval == EXIT)	return(retval);
			}		

			for(;;) {
				retval = get_n_ptime(&time,BROWSE,2,FORWARD,
					e_mesg);

				if(retval ==EFL)  {
					time.tm_week = 9;
					strcpy(time.tm_numb,"999999999999");
					break;
				}

				strcpy(emp.em_numb,time.tm_numb);

				retval = get_employee(&emp,BROWSE,0,e_mesg);
				if(retval < 0) {
					fomen(e_mesg);
					get();
					return(ERROR);
				}

				if(strcmp(emp.em_barg,barg1) < 0 ||
				   strcmp(emp.em_barg,barg2) > 0) {
					continue;
				}

				if(strcmp(emp.em_pos,posi1) < 0 ||
				   strcmp(emp.em_pos,posi2) > 0) {
					continue;
				}

				if(strcmp(emp.em_numb,empl1) < 0 ||
				   strcmp(emp.em_numb,empl2) > 0) {
					continue;
				}
				break;
			}

			if(strcmp(time.tm_earn,pay_param.pr_reg_earn) != 0) {
				time.tm_week = 9;
				strcpy(time.tm_numb,"999999999999");
			}

			if(strcmp(emp_sched1.es_numb,old_emp_numb) != 0 &&
			   strcmp(time.tm_numb,old_emp_numb) != 0) {

				if((retval = PrntTotWeek()) < 0)
					return(retval);
				if(retval == EXIT)	return(retval);

				if((retval = PrntTotPer()) < 0)
					return(retval);
				if(retval == EXIT)	return(retval);

				old_week = emp_sched1.es_week;
				strcpy(old_emp_numb,emp_sched1.es_numb);	

				if(strcmp(old_emp_numb,time.tm_numb) > 0) {
					old_week = time.tm_week;
					strcpy(old_emp_numb,time.tm_numb);	
				}
			}

			if(strcmp(time.tm_earn,pay_param.pr_reg_earn) != 0 ||
			   retval == EFL) {
				time.tm_week = 9;
				strcpy(time.tm_numb,"999999999999");
			}
		}

		if(strcmp(time.tm_numb,"999999999999") == 0 && 
		   strcmp(emp_sched1.es_numb,"999999999999") == 0)
			break;

	}	

	if(pgcnt) {
		if(term < 99) 
			last_page();	
#ifndef	SPOOLER
		else
			rite_top();
#endif
	}
	return(NOERROR);
}

/******************************************************************************
Main logic of the program
******************************************************************************/
static
PrintSecond()
{
	int	old_week;
	char	old_earn[7];
	char	old_emp_numb[13];

	flag = 0;

	time.tm_earn[0] = '\0';
	time.tm_numb[0] = '\0';
	time.tm_week = 0;
	time.tm_date = 0;
	time.tm_no = 0;

	flg_reset(TIME);

	for(;;) {
		retval = get_n_ptime(&time,BROWSE,2,FORWARD,e_mesg);

		if(retval ==EFL) 
			break;

		strcpy(emp.em_numb,time.tm_numb);

		retval = get_employee(&emp,BROWSE,0,e_mesg);
		if(retval < 0) {
			fomen(e_mesg);
			get();
			return(ERROR);
		}

		if(strcmp(emp.em_barg,barg1) < 0 ||
		   strcmp(emp.em_barg,barg2) > 0) {
			continue;
		}

		if(strcmp(emp.em_pos,posi1) < 0 ||
		   strcmp(emp.em_pos,posi2) > 0) {
			continue;
		}

		if(strcmp(emp.em_numb,empl1) < 0 ||
		   strcmp(emp.em_numb,empl2) > 0) {
			continue;
		}
		break;
	}

	strcpy(old_emp_numb,time.tm_numb);
	old_week = time.tm_week;

	strcpy(earn.ea_code,time.tm_earn);
	earn.ea_date = get_date();

	flg_reset(EARN);

	get_n_earn(&earn,BROWSE,1,BACKWARD,e_mesg);

	if(strcmp(earn.ea_code,time.tm_earn) != 0) {
		sprintf(e_mesg,"Earnings code not found %s",time.tm_earn);
		fomen(e_mesg);
		get();
		return(ERROR);
	}

	retval = PrntHdgDet();
	if(retval < 0) return(EXIT);

	strcpy(emp.em_numb,time.tm_numb);

	retval = get_employee(&emp,BROWSE,0,e_mesg);
	if(retval < 0) {
		fomen(e_mesg);
		get();
		return(ERROR);
	}

	for(;;) {
		if(strcmp(time.tm_earn,pay_param.pr_reg_earn) == 0) {
			for(;;) {
				retval = get_n_ptime(&time,BROWSE,2,FORWARD,e_mesg);

				if(retval ==EFL) {
					time.tm_week = 9;
					break;
				}

				strcpy(emp.em_numb,time.tm_numb);

				retval = get_employee(&emp,BROWSE,0,e_mesg);
				if(retval < 0) {
					fomen(e_mesg);
					get();
					return(ERROR);
				}

				if(strcmp(emp.em_barg,barg1) < 0 ||
			   	strcmp(emp.em_barg,barg2) > 0) {
					continue;
				}

				if(strcmp(emp.em_pos,posi1) < 0 ||
				   strcmp(emp.em_pos,posi2) > 0) {
					continue;
				}

				if(strcmp(emp.em_numb,empl1) < 0 ||
				   strcmp(emp.em_numb,empl2) > 0) {
					continue;
				}
				break;
			}

			if(time.tm_week == 9)
				break;

			continue;
		}
		else
			strcpy(old_earn,time.tm_earn);

		strcpy(class.c_code,time.tm_class);
		class.c_date = 0;

		retval = get_n_class(&class,BROWSE,0,FORWARD,e_mesg);

		strcpy(emp.em_numb,time.tm_numb);

		retval = get_employee(&emp,BROWSE,0,e_mesg);
		if(retval < 0) {
			fomen(e_mesg);
			get();
			return(ERROR);
		}

		if((retval = PrntTime())<0 )
			return(retval);
		if(retval == EXIT)	return(retval);
		
		for(;;) {
			retval = get_n_ptime(&time,BROWSE,2,FORWARD,e_mesg);

			if(retval ==EFL) {
				time.tm_week = 9;
				break;
			}

			strcpy(emp.em_numb,time.tm_numb);

			retval = get_employee(&emp,BROWSE,0,e_mesg);
			if(retval < 0) {
				fomen(e_mesg);
				get();
				return(ERROR);
			}

			if(strcmp(emp.em_barg,barg1) < 0 ||
			   strcmp(emp.em_barg,barg2) > 0) {
				continue;
			}

			if(strcmp(emp.em_pos,posi1) < 0 ||
			   strcmp(emp.em_pos,posi2) > 0) {
				continue;
			}

			if(strcmp(emp.em_numb,empl1) < 0 ||
			   strcmp(emp.em_numb,empl2) > 0) {
				continue;
			}
			break;
		}

		if(time.tm_week == 9) 
			break;

		if(old_week != time.tm_week || 
		   strcmp(old_emp_numb,time.tm_numb)!= 0 ||
		   strcmp(old_earn,time.tm_earn) != 0) {

			if((retval == PrntTotWeek()) < 0)
				return(retval);
			if(retval == EXIT)	return(retval);

			strcpy(old_emp_numb,time.tm_numb);
			old_week = time.tm_week;
		}

		if(strcmp(old_earn,time.tm_earn) != 0) {

			if((retval = PrntTotPer())<0 )
				return(retval);
			if(retval == EXIT)	return(retval);

			strcpy(earn.ea_code,time.tm_earn);
			earn.ea_date = get_date();

			flg_reset(EARN);

			get_n_earn(&earn,BROWSE,1,BACKWARD,e_mesg);

			if(strcmp(earn.ea_code,time.tm_earn) != 0) {
				sprintf(e_mesg,"Earnings code not found %s"
					,time.tm_earn);
				fomen(e_mesg);
				get();
				return(ERROR);
			}

			for(;;) {
				if(strcmp(time.tm_earn,pay_param.pr_reg_earn)
				==0){
					for(;;) {
						retval=get_n_ptime(&time,BROWSE,
							2,FORWARD,e_mesg);

						if(retval ==EFL) {
							time.tm_week = 9;
							break;
						}

						strcpy(emp.em_numb,time.tm_numb);

						retval=get_employee(&emp,BROWSE,
							0,e_mesg);
						if(retval < 0) {
							fomen(e_mesg);
							get();
							return(ERROR);
						}

						if(strcmp(emp.em_barg,barg1)<0||
						   strcmp(emp.em_barg,barg2)>0){
							continue;
						}

						if(strcmp(emp.em_pos,posi1)<0||
						   strcmp(emp.em_pos,posi2)>0){
							continue;
						}

						if(strcmp(emp.em_numb,empl1)<0||
						   strcmp(emp.em_numb,empl2)>0){
							continue;
						}
						break;
					}

					if(time.tm_week == 9)
						break;

					continue;
				}
				else
					break;
				}
				strcpy(earn.ea_code,time.tm_earn);
				earn.ea_date = get_date();

				flg_reset(EARN);

				get_n_earn(&earn,BROWSE,1,BACKWARD,e_mesg);

				if(strcmp(earn.ea_code,time.tm_earn) != 0) {
					sprintf(e_mesg,"Earnings code not found %s",time.tm_earn);
					fomen(e_mesg);
					get();
					return(ERROR);
				}
				retval = PrntHdgDet();
				if(retval < 0) return(EXIT);

		}
		if(retval == EFL)
			break;

	}	

	if((retval = PrntTotWeek())<0 )
		return(retval);
	if(retval == EXIT)	return(retval);

	if((retval = PrntTotPer())<0 )
		return(retval);
	if(retval == EXIT)	return(retval);

	if((retval = PrntTotEmp())<0 )
		return(retval);
	if(retval == EXIT)	return(retval);

	if(pgcnt) {
		if(term < 99) 
			last_page();	
#ifndef	SPOOLER
		else
			rite_top();
#endif
	}
	return(NOERROR);
}

/******************************************************************************
Prints the headings of the report GROSS EARNINGS BY EARNINGS CODE 
******************************************************************************/
static
PrntHdg()	/* Print heading  */
{
	short	offset;
	long	sysdt ;

	if( pgcnt && term < 99)   /* new page and display */
		if(next_page()<0) return(EXIT);	
		
	if( pgcnt || term < 99) { /* if not the first page or display */
		if( rite_top()<0 ) return( -1 );	/* form_feed */
	}
	else
		linecnt = 0;
	pgcnt++; 			/* increment page no */

	mkln( 1, PROG_NAME, 10 );
#ifdef ENGLISH
	mkln( 103, "Date:", 5 );
#else
	mkln( 103, "Date:", 5 );
#endif
	sysdt = get_date() ;
	tedit( (char *)&sysdt,"____/__/__",line+cur_pos, R_LONG ); 
	cur_pos += 10;

#ifdef ENGLISH
	mkln( 122, "PAGE:", 5 );
#else
	mkln( 122, "PAGE:", 5 );
#endif
	tedit( (char *)&pgcnt,"__0_",  line+cur_pos, R_SHORT ); 
	cur_pos += 4;
	if( PrintLine()<0 )	return(REPORT_ERR);

	offset = ( LNSZ-strlen(pa_rec.pa_co_name) )/2;
	mkln( offset, pa_rec.pa_co_name, strlen(pa_rec.pa_co_name) );
	if( PrintLine()<0 )	return(REPORT_ERR);

#ifdef ENGLISH
	mkln((LNSZ-31)/2,"GROSS EARNINGS BY EARNINGS CODE", 31 );
#else
	mkln((LNSZ-25)/2,"GROSS EARNINGS FOR PERIOD", 25 );
#endif
	if( PrintLine()<0 )	return(REPORT_ERR);
	if( PrintLine()<0 )	return(REPORT_ERR);

	return(NOERROR);
}

/******************************************************************************
Prints the headings of the report GROSS EARNINGS BY EARNINGS CODE
******************************************************************************/
static
PrntHdgDet()	/* Print heading  */
{
	mkln(1,"EARNINGS CODE:",14);
	mkln(16,earn.ea_code,6);
	mkln(23,"DESCRIPTION:",12);
	mkln(46,earn.ea_desc,12);

	if( PrintLine()<0 )	return(REPORT_ERR);
	if( PrintLine()<0 )	return(REPORT_ERR);

	mkln(1,"EMPLOYEE",8);
	mkln(23,"EMPLOYEE NAME",13);
	mkln(57,"WEEK",4);
	mkln(62,"CLASSIFICATION",14);
	mkln(77,"EARNINGS",8);
	mkln(86,"CC #",4);
	mkln(95,"TOTAL",5);
	mkln(110,"RATE",4);
	mkln(122,"AMOUNT",6);
	
	if( PrintLine()<0 )	return(REPORT_ERR);

	mkln(2,"NUMBER",6);
	mkln(67,"CODE",4);
	mkln(79,"CODE",4);
	mkln(95,"UNITS",5);

	if( PrintLine()<0 )	return(REPORT_ERR);
	if( PrintLine()<0 )	return(REPORT_ERR);

	return(NOERROR);
}
/******************************************************************************
Prints the detail line for employee schedule file
******************************************************************************/
static
PrntEmpSched()
{
	char	txt_buff[132];
	double	tot_units;
	int	i;
	double	rate;

	tot_units = 0;

	for(i = 0;i < 7;i++)
		tot_units = tot_units + emp_sched1.es_units[i];

	rate = (class.c_yrly_inc / class.c_units);

	tot_week_units = tot_week_units + tot_units;
	tot_week_amt = tot_week_amt + emp_sched1.es_amount;

	if(flag == 0) {
		mkln(1,emp_sched1.es_numb,12);
		mkln(14,emp.em_first_name,15);
		mkln(30,emp.em_last_name,25);
		tedit((char *)&emp_sched1.es_week,"_",txt_buff,R_SHORT);
		mkln(58,txt_buff,1);
	}
	mkln(66,emp_sched1.es_class,6);
	mkln(78,pay_param.pr_reg_earn,6);
	tedit((char *)&emp_sched1.es_cost,"__",txt_buff,R_SHORT);
	mkln(87,txt_buff,2);
	tedit((char *)&tot_units,"_,___,_0_.__-",txt_buff,R_DOUBLE);
	mkln(91,txt_buff,13);
	tedit((char *)&rate,"_,___,_0_.__-",txt_buff,R_DOUBLE);
	mkln(105,txt_buff,13);
	tedit((char *)&emp_sched1.es_amount,"_,___,_0_.__-",txt_buff,R_DOUBLE);
	mkln(119,txt_buff,13);

	if( PrintLine()<0 )	return(REPORT_ERR);
	if(retval == EXIT)	return(retval);

	flag = 1;

	return(NOERROR);
}


/******************************************************************************
Prints the detail line for time file
******************************************************************************/
static
PrntTime()
{
	char	txt_buff[132];
	double	tot_units;
	int	i;
	double	rate;

	tot_units = 0;

	for(i = 0;i < 7;i++)
		tot_units = tot_units + time.tm_units[i];

	rate = (class.c_yrly_inc / class.c_units);

	tot_week_units = tot_week_units + tot_units;
	tot_week_amt = tot_week_amt + time.tm_tot_amt;

	if(flag == 0) {
		mkln(1,time.tm_numb,12);
		mkln(14,emp.em_first_name,15);
		mkln(30,emp.em_last_name,25);
		tedit((char *)&time.tm_week,"_",txt_buff,R_SHORT);
		mkln(58,txt_buff,1);
	}
	mkln(66,time.tm_class,6);
	mkln(78,time.tm_earn,6);
	tedit((char *)&time.tm_cost,"__",txt_buff,R_SHORT);
	mkln(87,txt_buff,2);
	tedit((char *)&tot_units,"_,___,_0_.__-",txt_buff,R_DOUBLE);
	mkln(91,txt_buff,13);
	tedit((char *)&rate,"_,___,_0_.__-",txt_buff,R_DOUBLE);
	mkln(105,txt_buff,13);
	tedit((char *)&time.tm_tot_amt,"_,___,_0_.__-",txt_buff,R_DOUBLE);
	mkln(119,txt_buff,13);

	if( PrintLine()<0 )	return(REPORT_ERR);
	if(retval == EXIT)	return(retval);

	flag = 1;

	return(NOERROR);
}

/******************************************************************************
Prints the total line for every week 
******************************************************************************/
static
PrntTotWeek()
{
	char	txt_buff[132];

	tot_per_units = tot_per_units + tot_week_units;
	tot_per_amt = tot_per_amt + tot_week_amt;

	if( PrintLine()<0 )	return(REPORT_ERR);
	if(retval == EXIT)	return(retval);

	mkln(1,"TOTAL FOR WEEK:",15);
	tedit((char *)&tot_week_units,"_,___,_0_.__-",txt_buff,R_DOUBLE);
	mkln(91,txt_buff,13);
	tedit((char *)&tot_week_amt,"_,___,_0_.__-",txt_buff,R_DOUBLE);
	mkln(119,txt_buff,13);

	if( PrintLine()<0 )	return(REPORT_ERR);
	if(retval == EXIT)	return(retval);
	if( PrintLine()<0 )	return(REPORT_ERR);
	if(retval == EXIT)	return(retval);

	tot_week_units = 0;
	tot_week_amt = 0;

	flag = 0;

	return(NOERROR);
}

/******************************************************************************
Prints the total line for each employees 
******************************************************************************/
static
PrntTotPer()
{
	char	txt_buff[132];

	tot_emp_units = tot_emp_units + tot_per_units;
	tot_emp_amt = tot_emp_amt + tot_per_amt;

	mkln(1,"TOTAL FOR PAY PERIOD:",21);
	tedit((char *)&tot_per_units,"_,___,_0_.__-",txt_buff,R_DOUBLE);
	mkln(91,txt_buff,13);
	tedit((char *)&tot_per_amt,"_,___,_0_.__-",txt_buff,R_DOUBLE);
	mkln(119,txt_buff,13);

	if( PrintLine()<0 )	return(REPORT_ERR);
	if(retval == EXIT)	return(retval);
	if( PrintLine()<0 )	return(REPORT_ERR);
	if(retval == EXIT)	return(retval);

	tot_per_units = 0;
	tot_per_amt = 0;

	return(NOERROR);
}

/******************************************************************************
Prints the total line for all employees 
******************************************************************************/
static
PrntTotEmp()
{
	char	txt_buff[132];

	mkln(1,"TOTAL FOR ALL EMPLOYEES:",24);
	tedit((char *)&tot_emp_units,"_,___,_0_.__-",txt_buff,R_DOUBLE);
	mkln(91,txt_buff,13);
	tedit((char *)&tot_emp_amt,"_,___,_0_.__-",txt_buff,R_DOUBLE);
	mkln(119,txt_buff,13);

	if( PrintLine()<0 )	return(REPORT_ERR);
	if(retval == EXIT)	return(retval);
	if( PrintLine()<0 )	return(REPORT_ERR);
	if(retval == EXIT)	return(retval);

	tot_emp_units = 0;
	tot_emp_amt = 0;

	return(NOERROR);
}



/******************************************************************************
Function that prints every line of the report 
******************************************************************************/
static
PrintLine()
{
	if(prnt_line() < 0 )	return(REPORT_ERR);

	if(linecnt > PG_SIZE) {
		retval = PrntHdg();
		if(retval == EXIT)	return(retval);
		retval = PrntHdgDet();
		if(retval == EXIT)	return(retval);
	}

	return(NOERROR);
}

