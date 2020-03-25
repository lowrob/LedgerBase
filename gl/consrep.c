/******************************************************************************
*	Energy Consumption Report ..
*	Created by : K. Harish
*	Source 	: consrep.c 
*	Compile	: cc -I$I cons_rep.c
*	Linking	: cc cons_rep.o rp.o /usr/bfs/dbh/libbfs.a -o 
*******************************************************************************
About the file:
	It prints the energy consumption for a given period. The routine
	consrep() is called by the file glrep2.c . The report prints out
	the total cost, total consumption, the cost and consumption per 
	unit area, for the previous and current year.

Modified:
F.Tao 		1990/12/17 	Change to print the total for last type;
				Fix number of copies on the printer.

A.Cormier       1992/10/27      Change to print cost center up to 4 digits

L.Robichaud	1993/10/13	Pass new parameter to close_rep function for 
				a banner to print with 3000 family.
******************************************************************************/

#include <stdio.h>
#include <reports.h>

#define	EXIT	12

#define EXPENDITURE	3
#define STANDFUND	1
#define STARTREC	40

#ifdef ENGLISH
#define PRINTER 'P'
#define DISPLAY 'D'
#define FILE_IO	'F'
#else
#define PRINTER 'I'
#define DISPLAY 'A'
#define FILE_IO	'D'
#endif

static Gl_rec 	gl_rec,gl1_rec, gl2_rec, *itemptr[3];
static Sch_rec	sch_rec;
static Pa_rec	pa_rec;

static short	unittype[3], reccodtype[2];
static char	j_mesg[152] ,e_mesg[80];

static char	chardate[11];
static long	longdate;
static short	pageno;
static short	period;

static long	total_sqft;
static double	total_Pcons, total_Pcost;
static double	total_Ccons, total_Ccost;

static short 	Consump_Key;
static char	Consump_Desc[49];
static char 	Cons_Accno[19];
static char	program[11];
static char	unit[5];

consrep() 
{
/**** 	Declarations for DBH record reading  ****/
int	retval , i, j, err ;
char 	device[2] ;
short	copies ;
char 	discfile[20] ;
	
/****	
*	Accept Project code, Logical record number in the 
*	Project and format number for the logical record ..
*	Accept the output media code . Open the report ..
****/
	STRCPY(program,PROG_NAME);

	reccodtype[0] = 97;	/* for unit value of consumption */
	reccodtype[1] = 99;	/* for $ value of consumption */


	itemptr[0] = &gl_rec;
	itemptr[1] = &gl1_rec;
	itemptr[2] = &gl2_rec;

#ifdef ENGLISH
	STRCPY( device, "P" );
#else
	STRCPY( device, "I" );
#endif
	if( GetOutputon( device )<0 )
		return(-1);
	switch( *device ){
		case	FILE_IO:	/* Diskfile */
			device[0] = 'F';
			STRCPY( e_mesg, "consrep.dat" );
			if( GetFilename( e_mesg )<0 )
				return(-1);
			STRCPY( discfile, e_mesg );
			break;
		case	DISPLAY:	/* Display */
			device[0] = 'D';
			STRCPY( discfile, terminal) ;
			break;
		case	PRINTER:	/* Printer */
		default:
			device[0] = 'P';
			discfile[0] = '\0' ;
			break;
	}


	copies = 1;
	if(device[0] == 'P') {
		if((retval == GetNbrCopies( &copies )) < 0)
			return(retval);
	}

	longdate = get_date();
	mkdate(longdate, chardate) ;
	
	if(device[0] == 'D') 
		STRCPY( discfile, terminal ) ;

	if( opn_prnt( device, discfile, 1, e_mesg, 1 /* spool */)<0 ){
		fomen( e_mesg );
		get();
		return(REPORT_ERR);
	}
							  	
	if(device[0] == 'P')
		SetCopies( (int)copies );	/* number of copies to print */

	pageno = 0;
	LNSZ = 133;	 /* 132 column report */
	linecnt = PGSIZE;


/***	Prepare to read transaction sequentialy ***/
	if( get_param( &pa_rec, BROWSE, 1, e_mesg )<1 ){
		printf(e_mesg);
		return(-1);
	}
	/*  Get the period # from the user */
	for( ; ; ){
		period = pa_rec.pa_cur_period;
		if( GetPeriod( &period )<0 )
			return(-1);
		if( period<1 || period>pa_rec.pa_no_periods )
			continue;
		else 
			break;
	}

	if( (retval=Confirm())<0 )
		return(-1);
	if( !retval )	/* not confirmed */
		return(0);

/***	Read record and call report writer to output it ***/
	/* for the three types of energy, do */

	/* Initialize Totals for each unit to Zero */
	total_Pcons = total_Pcost = 0.0;
	total_Ccons = total_Ccost = 0.0;
	total_sqft = 0;


	for( ; ; ) {	/* fourty is add in order to create the correct 
			   record code to read the consumption DESC */
		gl_rec.reccod = (pa_rec.pa_cons_key + STARTREC);
		gl_rec.funds = STANDFUND;
		gl_rec.sect = EXPENDITURE;
		flg_reset(GLMAST);
		inc_str(gl_rec.accno,sizeof(gl_rec.accno)-1,FORWARD);  
		retval = get_n_gl(&gl_rec,BROWSE,2,FORWARD,e_mesg );
		if(gl_rec.reccod != (pa_rec.pa_cons_key + STARTREC)) break;
		if(retval==EFL ) break;
		if(retval != NOERROR ){
			printf("\n%s",e_mesg);
		  	return(0);
		}
		strcpy(Consump_Desc, gl_rec.desc);
		Consump_Key = gl_rec.keys[pa_rec.pa_cons_key-1];

		/* Set Totals for each unit to Zero */
		total_Pcons = total_Pcost = 0.0;
		total_Ccons = total_Ccost = 0.0;
		total_sqft = 0;
		linecnt = PGSIZE;	/* so new unit will print heading */
					/* if there are items to print    */
		itemptr[1]->funds = 1;
		sch_rec.sc_numb = 0;
		flg_reset( SCHOOL );

		for( ; ; ){
			
		   	retval = get_n_sch(&sch_rec,BROWSE,0,FORWARD,e_mesg );
		   	if( retval==EFL ) break;
		   	if( retval != NOERROR ){
				printf("\n%s",e_mesg);
		  		return(0);
		   		}

		   	scpy((char*)itemptr[2],(char*)itemptr[1],sizeof(Gl_rec));
			gl1_rec.funds = STANDFUND;
			gl1_rec.sect = EXPENDITURE;
			itemptr[1]->reccod = reccodtype[0];
			/* strcpy(gl1_rec.accno , 0); */
			flg_reset(GLMAST);
		   	for( ; ; ){	/* for unit and $ type accts */
				inc_str(gl1_rec.accno,sizeof(gl1_rec.accno)-1,
						FORWARD); 
				retval = get_n_gl(itemptr[1],BROWSE,2,FORWARD,
					e_mesg );
				if(itemptr[1]->reccod != reccodtype[0] ) break;
				if(retval==EFL ) break;
				if(retval==ERROR ){
					printf("\n%s",e_mesg);
		  			return(0);
				}
			
				if(sch_rec.sc_numb != 
				gl1_rec.keys[pa_rec.pa_cc_key -1] || 
				gl1_rec.keys[pa_rec.pa_cons_key - 1] != 
				Consump_Key){
					continue;
				}

				itemptr[2]->reccod = reccodtype[1]; 
				strcpy(gl2_rec.accno, gl1_rec.accno);
				retval = get_gl(itemptr[2],BROWSE,0,e_mesg);
				if( retval==ERROR ){
					printf("\n%s",e_mesg);
		  			return(0);
				}
				break;
			}


	/* budpre and budcur fields are being used to accumulate the actual */
	/* month to date expenditure. Just for convenience in report writing */
			itemptr[1]->budpre = 0.0;
			itemptr[1]->budcur = 0.0;
			itemptr[2]->budpre = 0.0;
		   	itemptr[2]->budcur = 0.0;
			if(sch_rec.sc_numb == gl1_rec.keys[pa_rec.pa_cc_key -1]
			&& gl1_rec.keys[pa_rec.pa_cons_key -1] == Consump_Key){
		   		for( j=0; j<period; j++ ){
					itemptr[1]->budpre += 
							itemptr[1]->prerel[j];
					itemptr[1]->budcur += 
							itemptr[1]->currel[j];
					itemptr[2]->budpre += 
							itemptr[2]->prerel[j];
					itemptr[2]->budcur += 
							itemptr[2]->currel[j];
				}
		   	}
		   	if( linecnt >= PGSIZE) 
				if((err = Print_Headings(i)) == EXIT) 
					 break ;
		   	if( (retval=Print_Detail()) < 0) break ;
		}
		Print_Total();
		if( err == EXIT) break;
	}
	if(err != EXIT) {
		if ( pageno) {
			if ( term < 99 )
				last_page() ;
#ifndef SPOOLER
			else
	 			rite_top() ;
#endif
		}
	}

/****	Windup .. ****/
	close_dbh() ;
	close_rep(BANNER) ;
	return(0);
}
static
Print_Headings(type)
int	type;
{
	char 	txt_line[80];
	int	offset;

	if(term < 99 && pageno) 
		if( next_page() < 0) return(EXIT) ;
	
	if ( pageno || term < 99) {
		if ( rite_top() < 0 ) return(REPORT_ERR);
	}
	else
	 	linecnt = 0;

	pageno++;

	mkln(1,program,10);
	offset = ( LNSZ-strlen(pa_rec.pa_co_name) )/2;
	mkln( offset, pa_rec.pa_co_name, strlen(pa_rec.pa_co_name) );
#ifdef ENGLISH
        mkln(115,"DATE:",5);
#else
        mkln(115,"DATE:",5);
#endif
	tedit((char *)&longdate,"____/__/__",txt_line,R_LONG);
	mkln(121,txt_line,10);
	if(prnt_line() < 0) return(REPORT_ERR);

#ifdef ENGLISH
	mkln(53,"ENERGY CONSUMPTION REPORT",25);
	mkln(115,"PAGE:",5);
#else
	mkln(49,"RAPPORT DE CONSOMMATION D'ENERGIE",33);
	mkln(115,"PAGE:",5);
#endif
	tedit((char *)&pageno,"__0_",txt_line,R_SHORT);
	mkln(121,txt_line,4);
	if(prnt_line() < 0) return(REPORT_ERR);

#ifdef ENGLISH
	sprintf(txt_line, "For the Period: %d", period);
#else
	sprintf(txt_line, "Pour la periode: %d", period);
#endif
	mkln((LNSZ-strlen(txt_line))/2,txt_line,strlen(txt_line));
	if(prnt_line() < 0) return(REPORT_ERR);
	if(prnt_line() < 0) return(REPORT_ERR);

/* SHERRY need the description for the first type of consumption */
	mkln(1,"TOTAL ",6);
	mkln(7,Consump_Desc,49);

	if(prnt_line() < 0) return(REPORT_ERR);
	if(prnt_line() < 0) return(REPORT_ERR);

#ifdef ENGLISH
	mkln(36,"<----------- PREVIOUS YEAR TO DATE ----------->",47);
	mkln(86,"<----------- CURRENT YEAR  TO DATE ----------->",47);
#else
	mkln(36,"<-------------- CUMUL PRECEDENT -------------->",47);
	mkln(86,"<-------------- CUMUL COURANT ---------------->",47);
#endif
	if(prnt_line() < 0) return(REPORT_ERR);
	
#ifdef ENGLISH
	mkln(1,"SC",2);
	mkln(10,"SCHOOL NAME",11);
	mkln(28,"SQ.FT",5);
	mkln(36,"CONSUMPTN",9);
	mkln(52,"COST",4);
	mkln(62,"CONS/SQFT",9);
	mkln(74,"COST/SQFT",9);
	mkln(86,"CONSUMPTN",9);
	mkln(102,"COST",4);
	mkln(112,"CONS/SQFT",9);
	mkln(124,"COST/SQFT",9);
#else
	mkln(1,"SECTION",7);
	mkln(10,"NOM DE L'ECOLE",14);
	mkln(28,"Pi Car",6);
	mkln(36,"CONSOMMATION",12);
	mkln(52,"COUT",4);
	mkln(62,"CONS/PiCar",10);
	mkln(74,"COUT/PiCar",10);
	mkln(86,"CONSOMMATION",12);
	mkln(102,"COUT",4);
	mkln(112,"CONS/PiCar",10);
	mkln(124,"COUT/PiCar",10);
#endif
	if(prnt_line() < 0) return(REPORT_ERR);
	if(prnt_line() < 0) return(REPORT_ERR);

	return(0);
}
static
Print_Detail()
{
	char	txt_line[80];
	double	Pconsft, Pcostft;
	double	Cconsft, Ccostft;

	tedit((char *)&sch_rec.sc_numb,"__0_",txt_line,R_SHORT);
	mkln(1,txt_line,4);
	mkln(6,sch_rec.sc_name,22);
	tedit((char *)&sch_rec.sc_size,"___,_0_",txt_line,R_LONG);
	mkln(29,txt_line,7);

	/**** Previous Year ****/
	tedit((char *)&itemptr[1]->budpre,"__,___,_0_",txt_line,R_DOUBLE);
	mkln(37,txt_line,10);			/** Consumption **/
	tedit((char *)&itemptr[2]->budpre,"__,___,_$_.__",txt_line,R_DOUBLE);
	mkln(48,txt_line,13);			/** Cost **/
	Pconsft = itemptr[1]->budpre / sch_rec.sc_size;	
	tedit((char *)&Pconsft,"__,__0.__",txt_line,R_DOUBLE);
	mkln(62,txt_line,9);			/** Cons per square foot **/
	Pcostft = itemptr[2]->budpre / sch_rec.sc_size;	
	tedit((char *)&Pcostft,"___,__$.__",txt_line,R_DOUBLE);
	mkln(73,txt_line,10);			/** Cost per square foot **/

	/**** Current Year ****/
	tedit((char *)&itemptr[1]->budcur,"__,___,_0_",txt_line,R_DOUBLE);
	mkln(85,txt_line,10);			/** Consumption **/
	tedit((char *)&itemptr[2]->budcur,"__,___,_$_.__",txt_line,R_DOUBLE);
	mkln(97,txt_line,13);			/** Cost **/
	Cconsft = itemptr[1]->budcur / sch_rec.sc_size;	
	tedit((char *)&Cconsft,"__,__0.__",txt_line,R_DOUBLE);
	mkln(112,txt_line,9);			/** Cons per square foot **/
	Ccostft = itemptr[2]->budcur / sch_rec.sc_size;	
	tedit((char *)&Ccostft,"___,__$.__",txt_line,R_DOUBLE);
	mkln(123,txt_line,10);			/** Cost per square foot **/
	if(prnt_line() < 0) return(REPORT_ERR);
	
	/** Calculate Totals **/
	total_sqft += sch_rec.sc_size;		/* total Sq.ft */

	total_Pcons += itemptr[1]->budpre;	/* total prev consumption */
	total_Pcost += itemptr[2]->budpre;	/* total prev cost */

	total_Ccons += itemptr[1]->budcur;	/* total curr consumption */
	total_Ccost += itemptr[2]->budcur;	/* total curr cost */

	return(0);
}
static
Print_Total()
{
	char	txt_line[80];
	double	average;
	
	if(prnt_line() < 0) return(REPORT_ERR);

#ifdef ENGLISH
	mkln(3,"TOTAL UNITS",11);
#else
	mkln(3,"TOTAL UNITS",11);
#endif
	if(prnt_line() < 0) return(REPORT_ERR);

	tedit((char *)&total_sqft,"_,___,_0_",txt_line,R_LONG);
	mkln(25,txt_line,9);

	/**** Previous Year ****/
	tedit((char *)&total_Pcons,"__,___,_0_",txt_line,R_DOUBLE);
	mkln(35,txt_line,10);			/** total Consumption **/
	tedit((char *)&total_Pcost,"__,___,_$_.__",txt_line,R_DOUBLE);
	mkln(47,txt_line,13);			/** total Cost **/
	average = (total_Pcons / total_sqft);
	tedit((char *)&average,"__,__0.__",txt_line,R_DOUBLE);
	mkln(62,txt_line,9);			/** Avg Cons per square foot **/
	average = (total_Pcost / total_sqft);
	tedit((char *)&average,"___,__$.__",txt_line,R_DOUBLE);
	mkln(73,txt_line,10);			/** Avg Cost per square foot **/

	/**** Current Year ****/
	tedit((char *)&total_Ccons,"__,___,_0_",txt_line,R_DOUBLE);
	mkln(85,txt_line,10);			/** total Consumption **/
	tedit((char *)&total_Ccost,"__,___,_$_.__",txt_line,R_DOUBLE);
	mkln(97,txt_line,13);			/** total Cost **/
	average = (total_Ccons / total_sqft);
	tedit((char *)&average,"__,__0.__",txt_line,R_DOUBLE);
	mkln(112,txt_line,9);			/** Avg Cons per square foot **/
	average = (total_Ccost / total_sqft);
	tedit((char *)&average,"___,__$.__",txt_line,R_DOUBLE);
	mkln(123,txt_line,10);			/** Avg Cost per square foot **/
	if(prnt_line() < 0) return(REPORT_ERR);

	return(0);
}
