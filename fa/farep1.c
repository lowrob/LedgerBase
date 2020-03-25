/******************************************************************************
		Sourcename   : farep1.c
		System       : Budgetary Financial system.
		Module       : Fixed Assets System : Reports
		Created on   : 89-10-17
		Created  By  : K HARISH.
		Cobol Source : 

******************************************************************************
About the file:	
	This file has routines to print Fixed Asset Report. 
	It is called by the file farep.c .

History:
Programmer      Last change on    Details

K.HARISH__      1989/10/17
J. Cormier      1990/11/19        added OBSOLETE condition
J. Cormier      1990/11/20        changed wording "Rate" to "cost/item"
J. Cormier      1990/11/21-22     added an entire new option; to print
				  FA reports by current cost centers.
L.Robichaud	1993/10/13	Pass new parameter to close_rep function for 
				a banner to print with 3000 family.
******************************************************************************/

#include <stdio.h>
#include <reports.h>
#include <bfs_fa.h>

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

static Pa_rec	param_rec;
static Fa_rec	fa_rec;
static Fa_type	fa_type;
static Fa_dept	fa_dept;
static Sch_rec	school;

/*  Data items for storing the key range end values */
static short	cc1, cc2; 		/* for cost centres range */
static long	itemid1, itemid2;	/* for item ids range */
static char	dept1[5], dept2[5], type1[5], type2[5];
					/* for dept and type ranges */

static int	retval;
static char 	discfile[15];	/* for storing outputfile name */
static short	pgcnt; 		/* for page count */
static char	resp[2];	/* for storing response */

/* Data items for recording changes in key field values */
static short 	deptflag, typeflag, ccflag;

/* Data items for storing key field values */
static short	bufcc, curcc_or_cc;
static char	bufdept[5], buftype[5];
static	short	copies;

extern char 	e_mesg[80];	/* for storing error messages */

farep1(key)
int	key;	
/* 
	key == 0: report on  original cost-centres
	key == 1: report on  FA types
	key == 2: report on  departments
	key == 3: report on  current cost centres
*/
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
			break;
		case FILE_IO:
			resp[0]='F';
			STRCPY( discfile, "farep.dat" );
			if((retval = GetFilename(discfile))<0 )
				return(retval);
			break;
		case PRINTER:
		default:
			resp[0]='P';
			discfile[0]= '\0';
			break;
	}

	copies = 1;
	if( *resp=='P' ){
		if((retval = GetNbrCopies(&copies))<0 )
			return(retval);
	}

	bufcc = 0;
	curcc_or_cc = 0;
	bufdept[0] = '\0';
	buftype[0] = '\0';
	deptflag = typeflag = ccflag = 0;

	itemid1 = 1;	itemid2 = 999999;
	cc1 = 1;	cc2 = 9999;
	STRCPY( type1, "");	STRCPY( type2, "ZZZZ" );
	STRCPY( dept1, "");	STRCPY( dept2, "ZZZZ" );


	switch ( key ){
		case 0:	/* main key, report on original cost centres */
			if((retval = GetCostcenRange(&cc1,&cc2))<0 )
				return(retval);
			if((retval = GetItemidRange(&itemid1,&itemid2))<0 )
				return(retval);
			break;
		case 1:	/* alt key1, report on FA types */
			if((retval = GetFatypeRange(type1,type2))<0 )
				return(retval);
			if((retval = GetCostcenRange(&cc1,&cc2))<0 )
				return(retval);
			if((retval = GetItemidRange(&itemid1,&itemid2))<0 )
				return(retval);
			break;
		case 2:	/* alt key2, report on Departments */
			if((retval = GetDeptnoRange(dept1,dept2))<0 )
				return(retval);
			if((retval = GetFatypeRange(type1,type2))<0 )
				return(retval);
			if((retval = GetCostcenRange(&cc1,&cc2))<0 )
				return(retval);
			if((retval = GetItemidRange(&itemid1,&itemid2))<0 )
				return(retval);
			break;
		case 3:	/* alt key3, report on current cost centres */
			if((retval = GetCostcenRange(&cc1,&cc2))<0 )
				return(retval);
			break;
		default:
			break;
	}
	if( (retval=Confirm())<=0 )
		return(retval);

	retval = get_param( &param_rec, BROWSE, 1, e_mesg );
	if( retval<1 ){
		return(DBH_ERR);
	}

	if( opn_prnt( resp, discfile, 1, e_mesg, 1 /* spool */)<0 ){
		return(REPORT_ERR);
	}
	if( *resp=='P' )
		SetCopies( (int)copies );
	pgcnt = 0;		/* Page count is zero */
	LNSZ = 131;		/* line size in no. of chars */
	linecnt = PGSIZE;	/* Page size in no. of lines */


	retval = PrintRep(key);
	close_rep(BANNER);	/* close output file */
	close_dbh();
	return(retval);
}

static
PrintRep(key)
int	key;
{
	/* Initialize the Fa master key as required */
	switch( key ){
		case 0:
			fa_rec.fa_costcen = cc1;
			fa_rec.fa_itemid = itemid1;
			break;
		case 1:
			STRCPY( fa_rec.fa_type, type1 );
			fa_rec.fa_costcen = cc1;
			fa_rec.fa_itemid = itemid1;
			break;
		case 2:
			STRCPY( fa_rec.fa_dept, dept1 );
			STRCPY( fa_rec.fa_type, type1 );
			fa_rec.fa_costcen = cc1;
			fa_rec.fa_itemid = itemid1;
			break;
		case 3:
			fa_rec.fa_curcostcen = cc1;
			fa_rec.fa_costcen = 0;
			fa_rec.fa_itemid = 0;
			break;
		default:
			break;
	}
	flg_reset( FAMAST );
	for( ; ; ){
		retval = get_n_famast( &fa_rec,BROWSE,key,FORWARD,e_mesg );
		if( retval==EFL )
			break;
		else if( retval<0 ){
			return(DBH_ERR);
		}
		retval = CheckKey(key);
		switch( retval ){
			case -1:
				return(-1);
			case CONTINUE:
				continue;
			case EFL:
			case NOERROR:
				break;
		}
		if( retval==EFL )
			break;
		/* if linecount has equalled or exceeded page size */
		if( (linecnt >= PGSIZE) || 
	       	((key==0) && (fa_rec.fa_costcen != curcc_or_cc)) ||  
	       	((key==3) && (fa_rec.fa_curcostcen != curcc_or_cc)) ) {
			retval = PrntHdg(key);/* Print heading stuff */
			if(retval < 0)return(retval);
			if(retval == EXIT) return(0);
		}

		/* Now print the record values */
		if((retval = PrntRec( key ))<0 )
			return(retval);
		
		if((retval = PrntRec1()) <0) return(retval);
	       	if(key==0) 
			curcc_or_cc = fa_rec.fa_costcen;
	       	if(key==3) 
			curcc_or_cc = fa_rec.fa_curcostcen;
	}
	if(pgcnt) {
		if(term < 99) 
			last_page();	
#ifndef	SPOOLER
		else
			rite_top();
#endif
	}
	return(0);
}

static
PrntHdg(key)	/* Print heading  */
int	key;
{
	short	offset;
	long	sysdt ;

	if( pgcnt && term < 99)   /* new page and display */
		if(next_page()<0) return(EXIT);	
		
	if( pgcnt || term < 99) { /* if not the first page or display */
		if( rite_top()<0 ) return( -1 );	/* form_feed */
		ccflag=deptflag=typeflag=1;
		bufcc = 0L;
		bufdept[0] = '\0';
		buftype[0] = '\0';
	}
	else
		linecnt = 0;
	pgcnt++; 			/* increment page no */

	mkln( 1, PROG_NAME, 10 );
#ifdef ENGLISH
	mkln( 103, "Date: ", 6 );
#else
	mkln( 103, "Date: ", 6 );
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
	if( prnt_line()<0 )	return(REPORT_ERR);

	offset = ( LNSZ-strlen(param_rec.pa_co_name) )/2;
	mkln( offset, param_rec.pa_co_name, strlen(param_rec.pa_co_name) );
	if( prnt_line()<0 )	return(REPORT_ERR);
	switch( key ){
#ifdef ENGLISH
		case 0:
			mkln((LNSZ-39)/2,"FA ITEM LISTING BY ORIGINAL COST CENTER", 39 );
			break;
		case 1:
			mkln((LNSZ-31)/2,"FA ITEM LISTING BY FA ITEM TYPE",31);
			break;
		case 2:
			mkln((LNSZ-27)/2, "FA ITEM LISTING BY DEPT. NO", 27 );
			break;
		case 3:
			mkln((LNSZ-38)/2,"FA ITEM LISTING BY CURRENT COST CENTER", 38 );
			break;
#else
		case 0:
			mkln((LNSZ-51)/2,"LISTE DES ARTICLES AI PAR CENTRE ORIGINAL DE COUTS", 51 );
			break;
		case 1:
			mkln((LNSZ-44)/2,"LISTE DES ARTICLES AI PAR GENRE D'ARTICLE AI",44);
			break;
		case 2:
			mkln((LNSZ-37)/2, "LISTE DES ARTICLES AI PAR NO DE DEPT.", 37 );
			break;
		case 3:
			mkln((LNSZ-51)/2,"LISTE DES ARTICLES AI PAR CENTRE COURRANT DE COUTS", 51 );
			break;
#endif
	}
	if( prnt_line()<0 )	return(REPORT_ERR);
	if( prnt_line()<0 )	return(REPORT_ERR);

	switch( key ) {
		case 0:
#ifdef ENGLISH
			mkln(1,"CC#",3);
			mkln(5,"    CC-NAME    ",15);
			mkln(22,"ITEM#",5);
			mkln(29,"  DESCRIPTION  ",15);
			mkln(46,"TYPE",4);
			mkln(51,"   TYPE-DESC   ",15);
			mkln(68," MODEL/S.NO ",12);
			mkln(82,"DEPT",4);
			mkln(87,"   DEPT-DESC   ",15);
			mkln(104,"CURRENT-CC#",11);
			mkln(116,"    CC-NAME    ",15);
#else
			mkln(1,"#CC",3);
			mkln(5,"   NOM DU CC   ",15);
			mkln(22,"#ARTICLE",8);
			mkln(29,"  DESCRIPTION  ",15);
			mkln(46,"GENRE",5);
			mkln(51,"  GENRE-DESC   ",15);
			mkln(68," MODEL/No S ",12);
			mkln(82,"DEPT",4);
			mkln(87,"   DESC-DEPT   ",15);
			mkln(104,"#CC-COURRANT",12);
			mkln(117,"   NOM DU CC  ",14);
#endif
			break;
		case 1:
#ifdef ENGLISH
			mkln(1,"TYPE",4);
			mkln(6,"   TYPE-DESC   ",15);
			mkln(23,"CC#",3);
			mkln(27,"    CC-NAME    ",15);
			mkln(44,"ITEM#",5);
			mkln(51,"  DESCRIPTION  ",15);
			mkln(68," MODEL/S.NO ",12);
			mkln(82,"DEPT",4);
			mkln(87,"   DEPT-DESC   ",15);
			mkln(104,"CURRENT-CC#",11);
			mkln(116,"    CC-NAME    ",15);
#else
			mkln(1,"GENRE",5);
			mkln(6,"  GENRE-DESC   ",15);
			mkln(23,"#CC",3);
			mkln(27,"   NOM DU CC   ",15);
			mkln(44,"#ARTICLE",8);
			mkln(51,"  DESCRIPTION  ",15);
			mkln(68," MODEL/No S ",12);
			mkln(82,"DEPT",4);
			mkln(87,"   DEPT-DESC   ",15);
			mkln(104,"#CC COURANT",11);
			mkln(116,"   NOM DU CC   ",15);
#endif
			break;
		 case 2:
#ifdef ENGLISH
			mkln(1,"DEPT",4);
			mkln(6,"   DEPT-DESC   ",15);
			mkln(23,"TYPE",4);
			mkln(27,"    TYPE-DESC   ",15);
			mkln(44,"CC#",3);
			mkln(48,"    CC-NAME    ",15);
			mkln(65,"ITEM#",5);
			mkln(72,"  DESCRIPTION  ",15);
			mkln(89," MODEL/S.NO ",15);
			mkln(103," CURRENT-CC# ",13);
			mkln(116,"    CC-NAME    ",15);
#else
			mkln(1,"DEPT",4);
			mkln(6,"   DEPT-DESC   ",15);
			mkln(23,"GENRE",5);
			mkln(27,"   GENRE-DESC   ",15);
			mkln(44,"#CC",3);
			mkln(48,"   NOM DU CC   ",15);
			mkln(65,"#ARTICLE",8);
			mkln(72,"  DESCRIPTION  ",15);
			mkln(89," MODEL/No S ",15);
			mkln(103," #CC COURANT ",13);
			mkln(116,"    NOM DU CC  ",15);
#endif
			break;
		case 3:
#ifdef ENGLISH
			mkln(1,"CUR-CC#",7);
			mkln(8,"  CC-NAME    ",15);
			mkln(24,"ITEM#",5);
			mkln(29,"  DESCRIPTION  ",15);
			mkln(46,"TYPE",4);
			mkln(51,"   TYPE-DESC   ",15);
			mkln(68," MODEL/S.NO ",12);
			mkln(82,"DEPT",4);
			mkln(87,"   DEPT-DESC   ",15);
			mkln(104,"     CC#   ",11);
			mkln(116,"    CC-NAME    ",15);
#else
			mkln(1,"#CC-COUR",8);
			mkln(11,"NOM DU CC  ",15);
			mkln(22,"#ARTICLE",8);
			mkln(29,"  DESCRIPTION  ",15);
			mkln(46,"GENRE",5);
			mkln(51,"  GENRE-DESC   ",15);
			mkln(68," MODEL/No S ",12);
			mkln(82,"DEPT",4);
			mkln(87,"   DESC-DEPT   ",15);
			mkln(104,"     #CC  ",11);
			mkln(116,"    NOM DU CC  ",15);
#endif
			break;
	}	

	if( prnt_line()<0 )	return(REPORT_ERR);
	if( PrntHdg1()<0 )	return(REPORT_ERR);
	return(0);
}
static
PrntHdg1()
{
#ifdef ENGLISH
	mkln(25,"ROOM",4);
	mkln(31,"CONDITION",9);
	mkln(41,"   SUPPLIER-NAME    ",20);
	mkln(64,"INVOICE#",8);
	mkln(77,"DATE",4);
	mkln(86,"QUANTITY",8);
	mkln(98,"COST/ITEM",9);
	mkln(112,"VALUE",5);
#else
	mkln(25,"SALLE",5);
	mkln(31,"CONDITION",9);
	mkln(41,"  NOM FOURNISSEUR   ",20);
	mkln(64,"#FACTURE",8);
	mkln(77,"DATE",4);
	mkln(86,"QUANTITE",8);
	mkln(98,"COUT/ARTICLE",12);
	mkln(112,"VALEUR",6);
#endif

	if( prnt_line()<0 )	return(REPORT_ERR);
	if( prnt_line()<0 )	return(REPORT_ERR);
	if( prnt_line()<0 )	return(REPORT_ERR);
	return(0);
}
static
PrntRec(key)
{
	char textstr[80];

	/***  get the type desc and the dept desc for each line ***/

	STRCPY(fa_type.code,fa_rec.fa_type);
	retval = get_fatype(&fa_type,BROWSE,0,e_mesg);
	if(retval != NOERROR){
		STRCPY(fa_type.desc,"???????????????");
	}
	STRCPY(fa_dept.code,fa_rec.fa_dept);
	retval = get_fadept(&fa_dept,BROWSE,0,e_mesg);
	if(retval != NOERROR){
		STRCPY(fa_dept.desc,"???????????????");
	}

	switch( key ) {
		case 0:
			if(fa_rec.fa_costcen != bufcc) {
			   tedit((char *)&fa_rec.fa_costcen,"__0_",textstr,R_SHORT);
			   mkln(1,textstr,4);
			school.sc_numb = fa_rec.fa_costcen;
			retval = get_sch(&school,BROWSE,0,e_mesg);
			/*if(retval != NOERROR){*/
			if(retval == ERROR){
			STRCPY(school.sc_name,"????????????");
			}
			/*if(fa_rec.fa_costcen != bufcc) {
			   tedit((char *)&fa_rec.fa_costcen,"__0_",textstr,R_SHORT);
			   mkln(1,textstr,4);*/
			   mkln(5,school.sc_name,15);
			   bufcc = fa_rec.fa_costcen;
			}
			tedit((char *)&fa_rec.fa_itemid,"____0_",textstr,R_LONG);
			mkln(22,textstr,6);
			mkln(29,fa_rec.fa_desc,15);
			mkln(46,fa_rec.fa_type,4);
			mkln(51,fa_type.desc,15);
			mkln(68,fa_rec.fa_sno,12);
			mkln(82,fa_rec.fa_dept,4);
			mkln(87,fa_dept.desc,15);
			tedit((char *)&fa_rec.fa_curcostcen,"__0_",textstr,R_SHORT);
		 	mkln(109,textstr,4);
			school.sc_numb = fa_rec.fa_curcostcen;
			retval = get_sch(&school,BROWSE,0,e_mesg);
			if(retval == ERROR){
				STRCPY(school.sc_name,"???????????");
			}
			mkln(116,school.sc_name,15);
			break;
		case 1:
			if(strcmp(fa_rec.fa_type,buftype) != 0) {
			   mkln(1,fa_rec.fa_type,4);
			   mkln(6,fa_type.desc,15);
			   STRCPY(buftype,fa_rec.fa_type);
			}
			if(fa_rec.fa_costcen != bufcc) {
			   tedit((char *)&fa_rec.fa_costcen,"__0_",textstr,R_SHORT);
			   mkln(23,textstr,4);
			   school.sc_numb = fa_rec.fa_costcen;
			   retval = get_sch(&school,BROWSE,0,e_mesg);
			   if(retval == ERROR){
			   STRCPY(school.sc_name,"????????????");
			   }
			   mkln(27,school.sc_name,15);
			   bufcc = fa_rec.fa_costcen;
			}
			tedit((char *)&fa_rec.fa_itemid,"____0_",textstr,R_LONG);
			mkln(44,textstr,6);
			mkln(51,fa_rec.fa_desc,15);
			mkln(68,fa_rec.fa_sno,12);
			mkln(82,fa_rec.fa_dept,4);
			mkln(87,fa_dept.desc,15);
			tedit((char *)&fa_rec.fa_curcostcen,"__0_",textstr,R_SHORT);
			mkln(109,textstr,4);
			school.sc_numb = fa_rec.fa_curcostcen;
			retval = get_sch(&school,BROWSE,0,e_mesg);
			if(retval == ERROR){
				STRCPY(school.sc_name,"???????????");
			}
			mkln(116,school.sc_name,15);
			break;
		case 2:
			if(strcmp(fa_rec.fa_dept,bufdept) != 0) {
			   mkln(1,fa_rec.fa_dept,4);
			   mkln(6,fa_dept.desc,15);
			   STRCPY(bufdept,fa_rec.fa_dept);
			}
			if(strcmp(fa_rec.fa_type,buftype) != 0) {
			   mkln(23,fa_rec.fa_type,4);
			   mkln(27,fa_type.desc,15);
			   STRCPY(buftype,fa_rec.fa_type);
			}
			if(fa_rec.fa_costcen != bufcc) {
			   tedit((char *)&fa_rec.fa_costcen,"__0_",textstr,R_SHORT);
			   mkln(44,textstr,4);
			   mkln(48,school.sc_name,15);
			   bufcc = fa_rec.fa_costcen;
			}
			tedit((char *)&fa_rec.fa_itemid,"____0_",textstr,R_LONG);
			mkln(65,textstr,6);
			mkln(72,fa_rec.fa_desc,20);
			mkln(89,fa_rec.fa_sno,12);
			tedit((char *)&fa_rec.fa_curcostcen,"__0_",textstr,R_SHORT);
			mkln(108,textstr,4);
			school.sc_numb = fa_rec.fa_curcostcen;
			retval = get_sch(&school,BROWSE,0,e_mesg);
			if(retval == ERROR){
				STRCPY(school.sc_name,"???????????");
			}
			mkln(116,school.sc_name,15);
			break;
		case 3:
			school.sc_numb = fa_rec.fa_curcostcen;
			retval = get_sch(&school,BROWSE,0,e_mesg);
			if(retval != NOERROR){
			STRCPY(school.sc_name,"???????????????");
			}
			if(fa_rec.fa_curcostcen != bufcc) {
			   tedit((char *)&fa_rec.fa_curcostcen,"__0_",textstr,R_SHORT);
			   mkln(1,textstr,4);
			   mkln(5,school.sc_name,15);
			   bufcc = fa_rec.fa_curcostcen;
			}
			tedit((char *)&fa_rec.fa_itemid,"____0_",textstr,R_LONG);
			mkln(22,textstr,6);
			mkln(29,fa_rec.fa_desc,15);
			mkln(46,fa_rec.fa_type,4);
			mkln(51,fa_type.desc,15);
			mkln(68,fa_rec.fa_sno,12);
			mkln(82,fa_rec.fa_dept,4);
			mkln(87,fa_dept.desc,15);
			tedit((char *)&fa_rec.fa_costcen,"__0_",textstr,R_SHORT);
		 	mkln(109,textstr,4);
			school.sc_numb = fa_rec.fa_costcen;
			retval = get_sch(&school,BROWSE,0,e_mesg);
			if(retval == ERROR){
				STRCPY(school.sc_name,"???????????");
			}
			mkln(116,school.sc_name,15);
			break;
	}	

	if( prnt_line()<0 )	return(REPORT_ERR);
	
	return(0);
}

static
PrntRec1()
{
	char textstr[80];

	mkln(25,fa_rec.fa_roomno,5);
	switch( fa_rec.fa_cond[0] ){
		case CD_EXCELLENT:
				mkln( 31,EXCELLENT,sizeof(EXCELLENT) );
				break;
		case CD_GOOD:
				mkln( 31,GOOD,sizeof(GOOD) );
				break;
		case CD_FAIR:
				mkln( 31,FAIR,sizeof(FAIR) );
				break;
		case CD_POOR:
				mkln( 31,POOR,sizeof(POOR) );
				break;
		case CD_OBSOLETE:
				mkln( 31,OBSOLETE,sizeof(OBSOLETE) );
				break;
	}
	mkln(41,fa_rec.fa_suppname,20);
	mkln(63,fa_rec.fa_invc,10);
	tedit((char *)&fa_rec.fa_rectdate,"____/__/__",textstr,R_LONG);
	mkln(74,textstr,10);
	tedit((char *)&fa_rec.fa_qty,"_____0_",textstr,R_LONG);
	mkln(86,textstr,7);
	tedit((char *)&fa_rec.fa_rate,"______0_.__",textstr,R_DOUBLE);
	mkln(95,textstr,11);
	tedit((char *)&fa_rec.fa_value,"______0_.__",textstr,R_DOUBLE);
	mkln(109,textstr,11);

	if( prnt_line()<0 )	return(REPORT_ERR);
	if( prnt_line()<0 )	return(REPORT_ERR);
	
	return(0);
}

CheckKey(key)
int	key;	/* key #: 0, 1 or 2: determines report format also */
{
	switch( key ){
	    case 0:
		if( fa_rec.fa_costcen<=cc2 ){
			if(fa_rec.fa_itemid < itemid1) {
				fa_rec.fa_itemid = itemid1;
				flg_reset(FAMAST);
				return(CONTINUE);
			}
			else
				if( fa_rec.fa_itemid <= itemid2 )
					return(NOERROR);
				else  {
					fa_rec.fa_costcen++;
					fa_rec.fa_itemid = itemid1;
					flg_reset( FAMAST );
					return( CONTINUE );
				}
			}
			else
				return( EFL );
	    case 1:
		if( strcmp( fa_rec.fa_type, type2 ) <= 0 ){
			if(fa_rec.fa_costcen < cc1 ||
			   fa_rec.fa_itemid < itemid1) {
				if(fa_rec.fa_costcen < cc1) {
					fa_rec.fa_costcen = cc1;
					fa_rec.fa_itemid = itemid1;
				}
				if(fa_rec.fa_itemid < itemid1)
					fa_rec.fa_itemid = itemid1;
				flg_reset(FAMAST);
				return(CONTINUE);
			}
			else
				if( fa_rec.fa_costcen<=cc2 ){
					if( fa_rec.fa_itemid <= itemid2 )
						return(NOERROR);
					else {
						fa_rec.fa_costcen++;
						fa_rec.fa_itemid = itemid1;
						flg_reset( FAMAST );
						return( CONTINUE );
					}
				}
				else {
					inc_str( fa_rec.fa_type, 
						 sizeof(fa_rec.fa_type)-1,
					 	 FORWARD );
					fa_rec.fa_costcen = cc1;
					fa_rec.fa_itemid = itemid1;
					flg_reset( FAMAST );
					return( CONTINUE );
				}
		}
		else
			return( EFL );
	    case 2:
		if( strcmp( fa_rec.fa_dept, dept2 )<=0 ){
		    if(strcmp(fa_rec.fa_type, type1) < 0 ||
		       fa_rec.fa_costcen < cc1		 ||
		       fa_rec.fa_itemid  < itemid1 )  {
			if(strcmp(fa_rec.fa_type, type1) < 0) {
				STRCPY(fa_rec.fa_type, type1);
				fa_rec.fa_costcen = cc1;
				fa_rec.fa_itemid = itemid1;
			}
			else
			    if(fa_rec.fa_costcen < cc1) {
				fa_rec.fa_costcen = cc1;
				fa_rec.fa_itemid = itemid1;
			    }
			    else
				if(fa_rec.fa_itemid < itemid1)
					fa_rec.fa_itemid = itemid1;
			flg_reset(FAMAST);
			return(CONTINUE);
		    }	
		    else
		    if( strcmp( fa_rec.fa_type, type2 )<=0 ){
		    	if( fa_rec.fa_costcen<=cc2 ){
			    if( fa_rec.fa_itemid <= itemid2 )
				return( NOERROR );
			    else{
				fa_rec.fa_costcen++;
				fa_rec.fa_itemid = itemid1;
				flg_reset( FAMAST );
				return( CONTINUE );
			    }
		   	}
	   		else{
			    inc_str( fa_rec.fa_type, 
				 sizeof(fa_rec.fa_type)-1,
				 FORWARD );
			    fa_rec.fa_costcen = cc1;
			    fa_rec.fa_itemid = itemid1;
			    flg_reset( FAMAST );
			    return( CONTINUE );
			}
		    }
		    else{
			    inc_str( fa_rec.fa_dept, 
				 sizeof(fa_rec.fa_dept)-1,
				 FORWARD );
			    STRCPY( fa_rec.fa_type, type1 );
			    fa_rec.fa_costcen = cc1;
			    fa_rec.fa_itemid = itemid1;
			    flg_reset( FAMAST );
			    return( CONTINUE );
		    }
		}
		else
			return( EFL );
	    case 3:
		if( fa_rec.fa_curcostcen<=cc2 )
			return(NOERROR);
		else
			return( EFL );
	}
	return(NOERROR);
}
