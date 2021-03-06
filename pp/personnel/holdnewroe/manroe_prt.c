/*------------------------------------------------------------------------
Source Name: manroe_prt.c
System     : Personnel/Payroll.
Created  On: May 27, 1997.
Created  By: Louis Robichaud.

DESCRIPTION:
	Program to print a name and address on Records of Employments. The 
values would have to be writen in.  This program was writen to speed up the 
process of making ROE's for people not on the payroll system.

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
------------------------------------------------------------------------*/
#define 	MAIN 

#include <stdio.h>
#include <cfomstrc.h>
#include <repdef.h>
#include <bfs_defs.h>
#include <bfs_com.h>
#include <bfs_pp.h>

#define	SYSTEM		"PERSONNEL/PAYROLL"	/* Sub System Name */
#define	MOD_DATE	"27-MAY-97"		/* Program Last Modified */
#define	SCR_NAME	"manroe_prt"		/* First screen */

#define	ST_FLD		400 
#define PRINTER_FLD	400
#define	END_FLD		700 
#define MESG_FLD	800

#define EXIT	12

#ifdef ENGLISH

#define PRINTROE	'P'
#define EXITOPT	'E'

#define	YES	'Y'
#define NO	'N'
#define EDIT	'E'
#define CANCEL	'C'

#else

#define PROCCHQ	'P'
#define EXITOPT	'F'

#define	YES	'O'
#define NO	'N'
#define EDIT	'M'
#define CANCEL	'A'

#endif

/* manroe_prt.sth - header for C structure generated by PROFOM EDITOR */

typedef struct	{

	char	s_pgm[11];	/* Program Name		Field 100 */
	long	s_rundate;	/* Run Date		Field 300 */
	short	s_printer;	/* Printer number	Field 400 */
	long	s_no_print;	/* Number of ROE's 	Field 500 */
	char	s_name[41];	/* Issuer Name		Field 600 */
	char	s_phone[11];	/* Phone Number		Filed 700 */
	char	s_mesg[78];	/* STRING X(78) 	Field 800 */
	char	s_opt[2];	/* STRING X 		Field 900 */
	} S_STRUCT;

static	S_STRUCT	s_sth ;

/* PROFOM Related variables */

struct  	stat_rec  sr;	/* PROFOM status rec */

static	Pa_rec	param;
static	Sch_rec	scho;

char 	e_mesg[100];	/* dbh will return err msg in this */
static	char	temp_buf[80] ;	/* work variable for tedit */

int	Validation();
int	Window();

double 	D_Roundoff();

static	char	emp_name[48];
static	short	tot_num_ins_wk;
static	double	total;

main(argc,argv)
int argc;
char *argv[];
{

	strncpy(SYS_NAME,SYSTEM,50);	/* Sub system name */
	strncpy(CHNG_DATE,MOD_DATE,10);	/* Modification Date */
	proc_switch(argc, argv, SCHOOL) ; 	/* Process Switches */

	STRCPY(sr.termnm,terminal);	/* Copy Terminal Name */

	if (Initialize()<0)
		exit(-1);		/* Initialization routine */

	if ( Process() < 0) { 		/* Initiate Process */
		Close();
		exit(-1);
	}

	Close();			/* return to menu */
	exit(NOERROR);

} /* END OF MAIN */

/*-------------------------------------------------------------------*/
/* Reset information */
Close()
{
	/* Set terminal back to normal mode from PROFOM */

	fomcs();
	fomrt();
	close_dbh();			/* Close files */
	close_rep();
}
/*-------------------------------------------------------------------*/
/* Initialize PROFOM  and Screens*/
Initialize()
{
	if(InitProfom()<0) { 			/* Initialize PROFOM */
		fomcs();
		fomrt();
		return(-1);
	}	
	if(InitScreens()<0) { 
		fomcs();
		fomrt();
		return(-1);
	}	
	return(NOERROR);
}

/*-------------------------------------------------------------------*/
/* Initialize PROFOM */

InitProfom()
{
	fomin(&sr);
	/* Check for Error */
	ret( err_chk(&sr) );

	fomcf(1,1);			/* Enable Print screen option */
	return(NOERROR);
}	/* InitProfom() */
/*----------------------------------------------------------------*/
/* Initialize screens before going to process options */
InitScreens()
{

	STRCPY(sr.scrnam,NFM_PATH);
	strcat(sr.scrnam,SCR_NAME) ;

	STRCPY(s_sth.s_pgm,PROG_NAME);
	s_sth.s_rundate = get_date();	/* get Today's Date in YYMMDD format */
	s_sth.s_mesg[0] = HV_CHAR ;
	s_sth.s_opt[0] = HV_CHAR ;

	return(NOERROR);

}	/* InitScreens() */
/*-----------------------------------------------------------------*/
InitPrinter()
{
	char	resp[2] ;
	char	discfile[15] ;

	/* Always to Printer */
	STRCPY( resp, "P" );
	discfile[0] = '\0';

	if( opn_prnt( resp, discfile, s_sth.s_printer, e_mesg, 0 )<0 ){
		fomen(e_mesg); get();
		return(-1);
	}
	return(NOERROR) ;
}
/*-----------------------------------------------------------------*/
InitPrinter1()
{
	char	resp[2] ;
	char	discfile[15] ;

	/* Always to Printer */
	STRCPY( resp, "F" );
	strcpy(discfile,"manroe.dat");

	if( opn_prnt( resp, discfile, s_sth.s_printer, e_mesg, 0 )<0 ){
		fomen(e_mesg); get();
		return(-1);
	}
	return(NOERROR) ;
}
/*-----------------------------------------------------------------*/
Process()
{
	int 	retval ;
	int	prntno;
	char	devname[30];

	retval = ReadInfo() ;
	if( retval < 0 ) return(retval) ;
	if( retval == EXIT ) return(NOERROR) ;

	retval = ConfirmScreen() ;
	if( retval != YES ) 
		return(NOERROR) ;
	

	LNSZ = 132;		/* line size in no. of chars */
	if(InitPrinter()<0) {
		return(-1);
	}	

	retval = DoTestPrint() ;

	close_rep();
	if(InitPrinter1()<0) {
		return(-1);
	}	

	retval = PrntRec();
	if( retval < 0 ) 
		return(retval) ;

	close_rep();

	/* Printer option */
	prntno = s_sth.s_printer;
	retval = get_prn_fd(prntno,devname);
	if(retval < 0){

#ifdef ENGLISH
		strcpy(s_sth.s_mesg,"Given Printer# NOT Found in Terminal/Printer ");
		strcat(s_sth.s_mesg,"Maintenance File");
#else
		strcpy(s_sth.s_mesg,"#d'imprimante donne pas retrouve dans le Dossier d'entretien");
		strcat(s_sth.s_mesg," du term/imprimante");
#endif
		return(-1);
	}

	strcpy(e_mesg,"cat manroe.dat > ");
	strcat(e_mesg,devname);
	system(e_mesg);
	strcpy(e_mesg,"echo  > ");
	strcat(e_mesg,devname);
	system(e_mesg);


	/* Remove the manroe.dat file */
	strcpy(e_mesg,"rm manroe.dat");
	system(e_mesg);

	DispMesgFld((char *)&s_sth);
	fflush(stdout) ;
	retval = WriteFields((char *)&s_sth,ST_FLD,END_FLD) ;

	return(NOERROR) ;
 
}
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Reads Cheque processing information.                                  */
ReadInfo()
{
	int 	i,retval ;

	/* In Add mode turn off dup control for key fields.
	   Other modes reverse it */
	fomca1(750,19,2) ;	/* The printer duplication and ESC F */
	fomca1(750,10,1) ;

	sr.endfld = PRINTER_FLD;
	fomud((char*)&s_sth);	/* Update Dup Buffers */

#ifdef ENGLISH
	STRCPY(s_sth.s_mesg,"Press ESC-F to Go Back to Option:");
#else
	STRCPY(s_sth.s_mesg,"Appuyer sur ESC-F pour retourner a Option:");
#endif
	DispMesgFld((char *)&s_sth);

	s_sth.s_printer = 1 ;

	retval = WriteFields((char *)&s_sth,PRINTER_FLD,PRINTER_FLD) ;
	if (retval < 0) return(retval) ; 
	s_sth.s_printer = LV_SHORT ;
	s_sth.s_no_print = LV_LONG;
	s_sth.s_name[0] = LV_CHAR;
	s_sth.s_phone[0] = LV_CHAR;

	i = ReadFields((char *)&s_sth,ST_FLD, END_FLD,Validation,Window,1); 
	if(PROFOM_ERR == i || DBH_ERR == i) return(i) ;
	if(EXIT == i){
		s_sth.s_mesg[0] = HV_CHAR;
		DispMesgFld((char *)&s_sth);
		return(i) ;
	}
	return(NOERROR) ;
}

/*-----------------------------------------------------------------------*/
/*  Printing the Test as many times needed for cheques to line up        */
DoTestPrint()
{
	int 	i,	retval ;

	STRCPY(temp_buf, "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX") ;

	for( ; ; ) {
#ifdef ENGLISH
		retval = GetOption((char *)&s_sth,"Do you want a Test Print (Y/N)?","YN") ;
#else
		retval = GetOption((char *)&s_sth,"Desirez-vous un test d'impression (O/N)?","ON") ;
#endif

		if( retval == NO ) 
			break ;

		/* Print on one form */
		if(DoPrint()) return(ERROR);

#ifdef ENGLISH
		fomen("The Test Print Is Printing. Press any key to continue.");
#else
		fomen("La copie d'essaie imprime. Appuyer une touche pour continuer.");
#endif
		get();
	}
	return(NOERROR) ;
}
/*----------------------------------------------------------------*/
/* Validation function() for Key and Header fields when PROFOM returns
  RET_VAL_CHK */

Validation()
{
	return(NOERROR) ;
}	/* Validation() */
/*----------------------------------------------------------------*/
Window()
{
	return(NOERROR);
}
/*-----------------------------------------------------------------------*/
/*                                                                       */
ConfirmScreen() 
{
	int	err ;

	for( ; ; ) {
#ifdef ENGLISH
		err = GetOption((char *)&s_sth,"Y(es), E(dit), C(ancel)", "YEC");
#else
		err = GetOption((char *)&s_sth,"O(ui), M(odifier), A(nnuler)",
									 "OMA");
#endif
		if(err == PROFOM_ERR) return(err) ;

		switch(err) {
		case  YES :
			return(YES) ;
		case  EDIT  :
			err = ReadInfo();
			break ;
		case  CANCEL :
#ifdef ENGLISH
			err = GetOption((char *)&s_sth,"Confirm the Cancel (Y/N)?", "YN") ;
#else
			err = GetOption((char *)&s_sth,"Confirmer l'annulation (O/N)?", "ON") ;
#endif
			if(err == YES) { 
				roll_back(e_mesg) ;	/* Unlock  Records */
				return(CANCEL) ;
			}
			break ;
		}	/* switch err */

		if(err == PROFOM_ERR) return(err) ;
		if(err == DBH_ERR) return(err) ;
	}	/* for(; ; ) */
}	/* ConfirmScreen() */

/*----------------------------------------------------------------------------*/
/*  Print Roe information				      */

PrntRec()
{
	int	retval, i;

#ifdef ENGLISH
	STRCPY(s_sth.s_mesg, "ROE's being Printed, PLEASE WAIT");
#else
	STRCPY(s_sth.s_mesg, "Les RELEVE D'EMPLOI sont en train d'etre imprimes, ATTENDEZ S.V.P.");
#endif
	DispMesgFld((char *)&s_sth);
	fflush(stdout) ;

	retval = get_param(&param,BROWSE,1,e_mesg);
	if(retval < 0){
		DispError((char *)&s_sth,e_mesg) ;
		roll_back(e_mesg);
		return(-1);
	}

	scho.sc_numb = param.pa_distccno; 

	retval = get_sch(&scho,BROWSE,0,e_mesg);
	if(retval < 0){
		DispError((char *)&s_sth,e_mesg) ;
		return(-1);
	}

	/* Print the number of copies requested by user */
	for(i=0;i<s_sth.s_no_print;i++){
		if(DoPrint()) return(retval);
	}

}

DoPrint()
{

	int	i, pos;

	for(i=0;i<4;i++)
		if( prnt_line() < 0) return(-1) ;

	mkln(8,scho.sc_name,28) ;
	mkln(55,"886954288RP0001",15);
	if( prnt_line() < 0) return(-1) ;
	mkln(8,scho.sc_add1,30) ;
	if( prnt_line() < 0) return(-1) ;

	mkln(8,scho.sc_add2,30) ;
	mkln(55,"BI/WEEKLY",9);
	if( prnt_line() < 0) return(-1) ;

	mkln(8,scho.sc_add3,30) ;
	if( prnt_line() < 0) return(-1) ;
	if( prnt_line() < 0) return(-1) ;

	mkln(44,scho.sc_pc,8) ;

	for(i=0;i<16;i++)
		if( prnt_line() < 0) return(-1) ;

	/* Contact in box 16 as of 97 */
	mkln(55,s_sth.s_name,40);

	if( prnt_line() < 0) return(-1) ;
	if( prnt_line() < 0) return(-1) ;

	mkln(58,s_sth.s_phone,3);
	mkln(61," ",1);
	mkln(62,s_sth.s_phone+3,3);
	mkln(65,"-",1);
	mkln(66,s_sth.s_phone+6,4);

	for(i=0;i<27;i++)
		if( prnt_line() < 0) return(-1) ;

	mkln(4,"X",1);
	mkln(26,s_sth.s_phone,3);
	mkln(29," ",1);
	mkln(30,s_sth.s_phone+3,3);
	mkln(33,"-",1);
	mkln(34,s_sth.s_phone+6,4);
	if( prnt_line() < 0) return(-1) ;
	if( prnt_line() < 0) return(-1) ;
	if( prnt_line() < 0) return(-1) ;
	mkln(40,s_sth.s_name,40);
	if( prnt_line() < 0) return(-1) ;

	rite_top();
	
	return(NOERROR) ;
}
/*-------------------- E n d   O f   P r o g r a m ---------------------*/
