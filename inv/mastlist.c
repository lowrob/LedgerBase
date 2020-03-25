/******************************************************************************
		Sourcename    : mastlist.c
		System        : Budgetary Financial system.
		Module        : Inventory reports
		Created on    : 89-10-04
		Created  By   : Jonathan Prescott
		Cobol sources : 
*******************************************************************************
About this file:
	This routine generates one of the following :
		1.Stock master listing.
	Calling file:	stockrep.c
Modifications:

L.Robichaud	1993/10/13	Pass new parameter to close_rep function for 
				a banner to print with 3000 family.
******************************************************************************/
#define	EXIT	12		/* as defined in streputl.c */

#include <stdio.h>
#include <repdef.h>
#include <bfs_defs.h>
#include <bfs_recs.h>
#include <bfs_inv.h>

#ifdef ENGLISH
#define PRINTER		'P'
#define DISPLAY		'D'
#define FILE_IO		'F'
#else
#define PRINTER		'I'
#define DISPLAY		'A'
#define FILE_IO		'D'
#endif

static int 	pgcnt;
static long	longdate ;
static int	detailline;
static int	retval;
static short	fund1, fund2, tempfund;
static short	sect1, sect2, tempsect;
static char	stcode1[11], stcode2[11];
static char	tempsname[36];

static St_mast		stmast;
static St_sect		st_sect;
static Pa_rec		pa_rec;

static char 	discfile[20] ;
static char	program[11];
static char	outdev[2];	
static short	copies ;

extern char	e_mesg[80] ;

mastlist() 
{
	int	key_init;

	discfile[0] = '\0';

#ifdef ENGLISH
	STRCPY( outdev, "P" );
#else
	STRCPY( outdev, "I" );
#endif
	if((retval = GetOutputon( outdev ))<0)
		return( retval );

	switch (*outdev) {
		case DISPLAY :	/*  Display on Terminal */
			outdev[0] = 'D';
			STRCPY(discfile,terminal);
			break;
		case FILE_IO : 	/*  Print to a disk file */ 
			outdev[0] = 'F';
			STRCPY( e_mesg, "status.dat");
			if((retval = GetFilename(e_mesg))<0)
				return(retval);
			STRCPY (discfile, e_mesg) ;
			break;
		case PRINTER : 	/*  Print to a printer */ 
		default  :
			outdev[0] = 'P';
			discfile[0] = '\0';
			break;
	}
	
	copies = 1;
	if(*outdev == 'P') {
		if((retval = GetNbrCopies( &copies )) < 0)
			return(retval);
	}

	STRCPY(program, "STOCKREP") ;

	if(opn_prnt(outdev,discfile,1,e_mesg,1) < 0)
		return(REPORT_ERR) ;

	if(*outdev == 'P') 
		SetCopies( (int)copies );	/* number of copies to print */

	if(*outdev == 'P' || *outdev == 'F') 
		PGSIZE = 60;

	pgcnt = 0;
	LNSZ = 131;
	linecnt = PGSIZE;

	fund1 = 1;
	fund2 = 999;
	sect1 = 1;
	sect2 = 15;
	STRCPY(stcode1,"");
	STRCPY(stcode2,"ZZZZZZZZZZ");
#ifdef ENGLISH
	fomen("Enter the starting and ending SECTION numbers");
#else
	fomen("Entrer les numeros debutant et finnissant de la section");
#endif
	if((retval = GetLocRange(&sect1,&sect2))<0 )
		return(retval);
	if((retval = GetFundRange(&fund1,&fund2))<0 )
		return(retval);
	if((retval = GetCodeRange(stcode1,stcode2))<0 )
		return(retval);
	if ( (retval=Confirm())<= 0) 
		return(retval);

	if( get_param(&pa_rec,BROWSE,1,e_mesg)<1 )
		return(DBH_ERR);

	tempsect = 0;
	tempsname[0] = '\0';
	tempfund = 0;
	longdate = get_date() ;
	if( get_section(&st_sect,BROWSE,1,e_mesg)<1 )
		return(DBH_ERR);
/***	Prepare to read stmast sequentialy ***/

	stmast.st_section = sect1;
	stmast.st_fund = fund1;
	STRCPY( stmast.st_code, stcode1 );  
	flg_reset( STMAST );
	key_init = 0;	/* initialized to 0 after each flg_reset */

	for(;;) {
		if( key_init==1 ){
			flg_reset( STMAST );
			key_init=0;
		}
		retval = get_n_stmast(&stmast, BROWSE, 1, FORWARD, e_mesg) ;
		if(retval < 0) {
			if(retval == EFL) { 
				retval=0; 
			}
			else
				retval == DBH_ERR;
			break ;
		}

		if( stmast.st_section>sect2 )
			break;
		if( stmast.st_fund<fund1 || stmast.st_fund>fund2 ){
			if( stmast.st_fund>fund2 )
				stmast.st_section++;
			stmast.st_fund = fund1;
			STRCPY( stmast.st_code, stcode1 );
			key_init = 1;
			continue;
		}
		if( strcmp(stmast.st_code,stcode1)<0 ||
		    strcmp(stmast.st_code,stcode2)>0 ){
			if( strcmp(stmast.st_code,stcode2)>0 )
				stmast.st_fund++;
			STRCPY( stmast.st_code, stcode1 );
			key_init = 1;
			continue;
		}

		if(linecnt >= PGSIZE) {
			retval = print_heading();
			if(retval < 0) return(retval);
			else if(retval == EXIT) break; 
		}
		if((retval = print_detail_line()) < 0) 
			return(retval);
	}

	if( pgcnt ){
		if( term<99 )
			last_page();
#ifndef SPOOLER
		else
			rite_top();
#endif
	}

	if(retval==REPORT_ERR)
#ifdef ENGLISH
		sprintf(e_mesg,"Internal report writing error");
#else
		sprintf(e_mesg,"Erreur d'inscription au rapport interne");
#endif
	close_dbh() ;
	close_rep(BANNER) ;
	return(retval);
}

print_heading()
{
	char txt_line[80];
	int  offset;

	tempsect = 0;
	tempsname[0] = '\0';
	tempfund = 0;

	if(term < 99 && pgcnt)	/* max page size and display */
		if(next_page()<0) return(EXIT);

	if(pgcnt || term < 99) {
		if(rite_top() < 0) return(REPORT_ERR);
	}
	else linecnt = 0;
	pgcnt++;

	mkln(2,program,10);
#ifdef ENGLISH
        mkln(102,"Date:",5);
#else
        mkln(102,"Date:",5);
#endif
	tedit((char *)&longdate,"____/__/__",txt_line,R_LONG);
	mkln(108,txt_line,10);
#ifdef ENGLISH
	mkln(121,"Page:",5);
#else
	mkln(121,"Page:",5);
#endif
	tedit((char *)&pgcnt,"__0_",txt_line,R_INT);
	mkln(127,txt_line,4);
	if(prnt_line() < 0) return(REPORT_ERR);
	offset = ( LNSZ-strlen(pa_rec.pa_co_name) )/2;
	mkln( offset, pa_rec.pa_co_name, strlen(pa_rec.pa_co_name) );
	if(prnt_line() < 0) return(REPORT_ERR);

#ifdef ENGLISH
	mkln(55,"Stock Master Listing",20);
#else
	mkln(53,"Liste des stocks maitres",24);
#endif
	if(prnt_line() < 0) return(REPORT_ERR);
	if(prnt_line() < 0) return(REPORT_ERR);
#ifdef ENGLISH
	mkln(1,"Sect",4);
	mkln(11,"Section Name",12);
	mkln(28,"Fund",4);
	mkln(34,"Stock Code",10);
	mkln(50,"Description",11);
	mkln(67,"Units",5);
	mkln(77,"On Hand",7);
	mkln(91,"On Order",8);
	mkln(104,"Allocated",9);
#else
	mkln(1,"Sect",4);
	mkln(11,"Nom de section",14);
	mkln(28,"Fond",4);
	mkln(34,"Code de stock",13);
	mkln(50,"Description",11);
	mkln(67,"Unites",6);
	mkln(77,"En maison",9);
	mkln(91,"Sur commande",13);
	mkln(106,"Alloue",6);
#endif
#ifdef ENGLISH
	mkln(119,"Paid For",8);
#else
	mkln(119,"Paye",4);
#endif
	if(prnt_line() < 0) return(REPORT_ERR);
#ifdef ENGLISH
	mkln(31,"Min.Limit",9);
	mkln(45,"Max.Limit",9);
	mkln(62,"Account No.",11);
	mkln(77,"Last Received",13);
	mkln(92,"Average Cost",12);
	mkln(110,"Value",5);
	mkln(118,"Lead Days",9);
#else
	mkln(31,"Limite min.",11);
	mkln(45,"Limite max.",11);
	mkln(62,"# de compte",11);
	mkln(74,"Derniere reception",18);
	mkln(93,"Cout moyen",10);
	mkln(107,"Valeur",6);
	mkln(115,"Delai de reappr.",16);
#endif
	if(prnt_line() < 0) return(REPORT_ERR);
	if(prnt_line() < 0) return(REPORT_ERR);
	return(NOERROR);
}

print_detail_line()
{
 char txt_line[80];
	if(tempsect == stmast.st_section) {
		mkln(2,"  ",2);
	}
	else {
		tedit((char *)&stmast.st_section,"0_",txt_line,R_SHORT);
		mkln(2,txt_line,2);
		tempsect = stmast.st_section;
	}
	if(strcmp(tempsname,st_sect.name[stmast.st_section -1]) == 0) {
		mkln(7,"                         ",20);
	}
	else {
		mkln(7,st_sect.name[stmast.st_section - 1],20);
		STRCPY(tempsname,st_sect.name[stmast.st_section - 1]);
	}
	if(tempfund == stmast.st_fund) {
		mkln(28,"   ",3);
	}
	else {
		tedit((char *)&stmast.st_fund,"_0_",txt_line,R_SHORT);
		mkln(28,txt_line,3);
		tempfund = stmast.st_fund;
	}
	mkln(34,stmast.st_code,10);
	mkln(46,stmast.st_desc,20);
	mkln(67,stmast.st_unit,6);
	tedit((char *)&stmast.st_on_hand,"____0_.____-",txt_line,R_DOUBLE);
	mkln(75,txt_line,12);
	tedit((char *)&stmast.st_on_order,"____0_.____-",txt_line,R_DOUBLE);
	mkln(89,txt_line,12);
	tedit((char *)&stmast.st_alloc,"____0_.____-",txt_line,R_DOUBLE);
	mkln(103,txt_line,12);
	tedit((char *)&stmast.st_paidfor,"_____0_.____-",txt_line,R_DOUBLE);
	mkln(117,txt_line,12);
	if(prnt_line() < 0) return(REPORT_ERR);
	tedit((char *)&stmast.st_min,"____0_.____-",txt_line,R_DOUBLE);
	mkln(30,txt_line,12);
	tedit((char *)&stmast.st_max,"____0_.____-",txt_line,R_DOUBLE);
	mkln(44,txt_line,12);
	mkln(58,stmast.st_accno,18);
	tedit((char *)&stmast.st_lastdate,"____/__/__",txt_line,R_LONG);
	mkln(78,txt_line,10);
	tedit((char *)&stmast.st_rate,"____0_.____-",txt_line,R_DOUBLE);
	mkln(90,txt_line,10);
	tedit((char *)&stmast.st_value,"____0_.____-",txt_line,R_DOUBLE);
	mkln(105,txt_line,12);
	tedit((char *)&stmast.st_leaddays,"_0_",txt_line,R_SHORT);
	mkln(119,txt_line,3);
	if(prnt_line() < 0) return(REPORT_ERR);
	if(prnt_line() < 0) return(REPORT_ERR);
	return(NOERROR);
}

