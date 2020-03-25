/*-----------------------------------------------------------------------
Source Name: yearend.c
System     : Year End Processing
Module     : General Ledger system.
Created  On: 19 July 89.
Created  By: Cathy Burns.


DESCRIPTION:
	Program to process the year end procedures.

Usage of SWITCHES when they are ON :
	SW1 :
	SW2 :
	SW3 :
	SW4 :
	SW5 :
	SW6 :
	SW7 :
	SW8 :
		Not Used.


MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
J.Prescott     90/11/23       Added ability to carry budget figures to new year
F.Tao 	       90/12/19	      Round up amounts before writing to file.

------------------------------------------------------------------------*/
#define  MAIN
#define  MAINFL		-1	/* no main file used */

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define NO_PERIODS	13
#define	SYSTEM		"CLOSING PROCESSES"	/* Sub System Name */
#define	MOD_DATE	"19-DEC-90"		/* Program Last Modified */

#ifdef	ORACLE
extern	char	owner_prefix[];
#endif

#ifdef	ENGLISH
#define U_YES	'Y'
#define L_YES	'y'
#define NO	'N'
#else
#define U_YES	'O'
#define L_YES	'o'
#define NO	'N'
#endif

char 	e_mesg[80];  		/* dbh will return err msg in this */
static  Gl_rec	gl_rec ;
static  Ctl_rec	ctl_rec ;
static  Pa_rec	pa_rec ;
static	double	actual_chg[5] ;
double  D_Roundoff();

main(argc,argv) 
int argc;
char *argv[];
{
	int	err, sno=0;
	char	filenm1[50];
	char	filenm2[50];

	strncpy(SYS_NAME,SYSTEM,50);	/* Sub system name */
	strncpy(CHNG_DATE,MOD_DATE,10);	/* Modification Date */
	proc_switch(argc, argv, MAINFL) ; 	/* Process Switches */

	err = GetUserClass(e_mesg) ;
	if(err < 0 || (err != ADMINISTRATOR && err != SUPERUSER)) {
		if(err == DBH_ERR)
			printf("\n%s\n",e_mesg);
		else
#ifdef ENGLISH
			printf("\n\n\tACCESS DENIED\n");
#else
			printf("\n\n\tACCES NIE\n");
#endif
		exit(-1);
	}


/*  Reading the Parameter file to make sure monthends have been done 
    for all other periods.
*******************************************************************************/

	err = get_param(&pa_rec,UPDATE,1,e_mesg);
	if(err < 0 ) {
		if(err == ERROR)
			DispError(e_mesg);
		else if (err == UNDEF)
#ifdef ENGLISH
			DispError("Parameters are not Setup ...");
#else
			DispError("Parametres ne sont pas Etablis... ");
#endif
		close_dbh() ;
		exit(-1);
	}

	if (pa_rec.pa_cur_period != 0) {
		roll_back(e_mesg) ;
#ifdef ENGLISH
		DispError("ERROR : Cannot Process Year End. All the Periods not Closed");
#else
		DispError("ERREUR: Ne peut pas traiter la fin d'annee. Toutes les periodes ne sont pas fermees.");
#endif
		close_dbh() ;
		exit(-1) ;
	}
#ifdef ENGLISH
	printf("\n\n\n\n\n\tYEAR END PROCEDURES  for the Period: %d\n\n\n",
		pa_rec.pa_cur_period);
	printf("\t\t%2d. Backs up the Data Base\n",++sno);
	printf("\t\t%2d. Closes G/L Accounts for this Year\n",++sno);
#else
	printf("\n\n\n\n\n\tPROCEDURES DE FIN D'ANNEE pour la periode: %d\n\n\n",
		pa_rec.pa_cur_period);
	printf("\t\t%2d. Sauvegarde la base de donnees\n",++sno);
	printf("\t\t%2d. Ferme les comptes G/L pour cette annee\n",++sno);
#endif

	if (pa_rec.pa_pos[0] == U_YES && pa_rec.pa_aps[0] == U_YES &&
					pa_rec.pa_stores[0] == NO) 
#ifdef ENGLISH
		printf("\t\t%2d. Closes Purchase Orders for this Year\n",++sno);
#else
		printf("\t\t%2d. Ferme les bons de commande pour cette annee\n",++sno);
#endif
	if (pa_rec.pa_aps[0] == U_YES) 
#ifdef ENGLISH
		printf("\t\t%2d. Does yearend closing for the Suppliers\n",++sno);
#else
		printf("\t\t%2d. Fait la fermeture d'annee pour les fournisseurs\n",++sno);
#endif
	if( pa_rec.pa_ars[0]==U_YES )
#ifdef ENGLISH
		printf("\t\t%2d. Does yearend closing for the Customers\n",++sno);
#else
		printf("\t\t%2d. Fait la fermeture de fin d'annee pour les clients\n",++sno);
#endif
	if( pa_rec.pa_stores[0]==U_YES )
#ifdef ENGLISH
		printf("\t\t%2d. Closes Inventory for this Year\n",++sno);
#else
		printf("\t\t%2d. Ferme l'inventaire pour cette annee\n",++sno);
#endif
#ifdef ENGLISH
	printf("\t\t%2d. Rolls the Period to First Period\n\n",++sno);
	printf("\t\tProceed(Y/N)? ");
#else
	printf("\t\t%2d. Avance la periode a la premiere periode\n\n",++sno);
	printf("\t\tProceder (O/N)? ");
#endif

	scanf("%s",e_mesg);
	if(e_mesg[0] != L_YES && e_mesg[0] != U_YES) {
		close_dbh();
		exit(-1);
	}

	err = initial() ;  	/*  initialization */

	if(err == NOERROR) {
#ifdef ENGLISH
		printf("\n\nYear End Successful.");
		unlink_file(GLTRHDR);
		unlink_file(GLTRAN);
		/* Copy Accrual journal entries to regular journal entries */
		form_f_name("",filenm1); 
		form_f_name("",filenm2); 
		strcat(filenm1,GLTRHDRNY_FILE);
		strcat(filenm2,GLTRHDR_FILE);
		sprintf(e_mesg,"mv %s %s",filenm1,filenm2);
		system(e_mesg);
		strcat(filenm1,".IX");
		strcat(filenm2,".IX");
		sprintf(e_mesg,"mv %s %s",filenm1,filenm2);
		system(e_mesg);

		form_f_name("",filenm1); 
		form_f_name("",filenm2); 
		strcat(filenm1,GLTRANNY_FILE);
		strcat(filenm2,GLTRAN_FILE);
		sprintf(e_mesg,"mv %s %s",filenm1,filenm2);
		system(e_mesg);
		strcat(filenm1,".IX");
		strcat(filenm2,".IX");
		sprintf(e_mesg,"mv %s %s",filenm1,filenm2);
		system(e_mesg);
	}
	else
		printf("\n\nERROR: Year End is Not Done Successfully.");
#else
		printf("\n\nFin d'annee reussi.");
		unlink_file(GLTRHDR);
		unlink_file(GLTRAN);
	}
	else
		printf("\n\nERREUR: Fin d'annee n'est pas reussie.");
#endif

#ifdef ENGLISH
	printf("Press RETURN<CR> ");
#else
	printf("Appuyer sur RETURN<CR> ");
#endif
	fflush( stdin );
	read(0, e_mesg, 80);

	close_dbh();			/* Close files */

	if(err != NOERROR) exit(-1);
	exit(0);
} /* END OF MAIN */

initial()
{
	int 	err;
	char	file1[50], file2[50];
	char	in_str[300];

#ifdef ENGLISH
	printf("\n\tBackup Year End Files\n");
#else
	printf("\n\tSauvegarde les dossiers de fin d'annee\n");
#endif

#ifndef	ORACLE
	form_f_name("", file1);
	form_f_name(PREV_YEAR, file2); 
#ifdef	MS_DOS
	sprintf(in_str,"copy %s*.* %s", file1, file2);
#else
	sprintf(in_str,"cp %s* %s", file1, file2);
#endif
#ifdef ENGLISH
	printf( "Command: %s\n",  in_str) ;
#else
	printf( "Commande: %s\n",  in_str) ;
#endif
	system(in_str);
#else

	/***********************************************************************
	EXPORT format(entire info in database w.r.t the cur. year owner):
		exp userid/passwd file=*.dmp buffer=10240 owner=ownerid grants=y
			indexes=y rows=y

	IMPORT format(entire data from the exported file to prev yr. owner):
		imp userid/passwd file=*.dmp buffer=10240 ignore=n grants=y
			indexes=y rows=y full=y fromuser=ownerid touser=ownerid
			tables=* commit=n

	NOTE:	1. '*' in 'file=*.dmp' keyword is the name corr. to the ownerid.
		2. 'table=*' indicates all tables.
		3. buffersize in 'buffer=' can be any suitable value.
		4. userid must have DBA previlege.
	**********************************************************************/

	/* Frame the EXPORT command with the filename formed from ownerid */

	sprintf(in_str, "exp %s/%s file=%s%s.dmp buffer=10240 owner=%s%s \
			grants=y indexes=y rows=y",
		User_Id, UserPasswd,
		owner_prefix, dist_no,
		owner_prefix, dist_no);

#ifdef ENGLISH
	printf( "Command: %s\n",  in_str) ;
#else
	printf( "Commande: %s\n",  in_str) ;
#endif
	system(in_str);

	/* First drop all the tables of previous year */
	SW9 = 1 ;
#ifdef ENGLISH
	printf("\nDropping Previous Year Tables\n");
#else
	printf("\nAbandonne les tables de l'annee precedente\n");
#endif
	for(err = 0 ; err < TOTAL_FILES ; err++) {
		getflnm(err, e_mesg) ;
#ifdef ENGLISH
		printf("\tDropping %s\n", e_mesg);
#else
		printf("\tAbandonne %s\n", e_mesg);
#endif
		if(drop_tbl(err, e_mesg) < 0) {
			DispError(e_mesg) ; 	
			return(-1) ;
		}
	}
	SW9 = 0 ;

	/* Frame the IMPORT command with the old userid & new ownerid */

	sprintf(in_str, "imp %s/%s file=%s%s.dmp buffer=10240 fromuser=%s%s \
	   touser=%s%s%s tables=* grants=y indexes=y rows=y ignore=n commit=n",
		User_Id, UserPasswd,
		owner_prefix, dist_no,
		owner_prefix, dist_no,
		owner_prefix, dist_no, PREV_YEAR);

#ifdef ENGLISH
	printf( "Command: %s\n",  in_str) ;
#else
	printf( "Commande: %s\n",  in_str) ;
#endif
	system(in_str);

	/* Unlink the temporary dump file */
	sprintf(in_str, "%s%s.dmp", owner_prefix, dist_no) ;
	unlink(in_str) ;
#endif

#ifdef ENGLISH
	printf("\nGENERAL LEDGER SYSTEM\n");
#else
	printf("\nSYSTEME GRAND LIVRE\n");
#endif
#ifdef ENGLISH
	printf("\n\tProcessing GLMAST For Year End\n");
#else
	printf("\n\tTraitement du G/L maitre pour la fin d'annee\n");
#endif
	gl_rec.funds = 0;
	gl_rec.accno[0] = '\0' ;
	gl_rec.reccod = 0 ;
	flg_reset( GLMAST );
	err = Process(); 		/* Initiate Process */

        if (err < 0) {
#ifdef ENGLISH
		DispError("Problem with Processing Year End");
#else
		DispError("Probleme en traitant la fin d'annee");
#endif
		return(-1);
	}

	SW9 = 1;			/* Switch to set up previous glmast */
	close_dbh() ;
	gl_rec.funds = 0;
	gl_rec.accno[0] = '\0' ;
	gl_rec.reccod = 0 ;
	flg_reset( GLMAST );
	err = ProcessLastYear();	/* Initiate Process */

	close_dbh() ;
	SW9 = 0;			/* Switch to set up previous glmast */
	
        if (err < 0) {
#ifdef ENGLISH
		DispError("Problem with Processing Year End");
#else
		DispError("Probleme en traitant la fin d'annee");
#endif
		return(-1);
	}

	if( pa_rec.pa_aps[0] == U_YES ) {
#ifdef ENGLISH
		printf("\nPURCHASE ORDER SYSTEM\n");
#else
		printf("\nSYSTEME BONS DE COMMANDE\n");
#endif
		if( pa_rec.pa_stores[0] == NO && pa_rec.pa_pos[0] == U_YES){
#ifdef ENGLISH
			printf("\n\tClosing Purchase Orders For Year End\n");
#else
			printf("\n\tFermeture des bons de commande pour la fin d'annee\n");
#endif
			if( PoYearEnd()<0 ){
#ifdef ENGLISH
				DispError("Problem with Year End Purchase Order closing");
#else
				DispError("Probleme avec la fermeture des bons de commande pour la fin d'annee");
#endif
				return(-1);
			}
		}
#ifdef ENGLISH
		printf("\n\tClosing Suppliers For Year End\n");
#else
		printf("\n\tFermeture des fournisseurs pour la fin d'annee\n");
#endif
		if( SupplierYearEnd()<0 ){
#ifdef ENGLISH
			DispError("Problem with Year End Supplier closing");
#else
			DispError("Probleme avec la fermeture des fournisseurs pour la fin d'annee");
#endif
			return(-1);
		}
	}

	if( pa_rec.pa_ars[0]==U_YES ){
#ifdef ENGLISH
		printf("\nACCOUNTS RECEIVABLE SYSTEM\n");
#else
		printf("\nSYSTEME COMPTES RECEVABLES\n");
#endif
#ifdef ENGLISH
		printf("\tCustomers' Year End\n");
#else
		printf("\tFin d'annee des clients\n");
#endif
		if( CustEnd( YEAR )<0 ){
#ifdef ENGLISH
			DispError("Problem with Customer Year End");
#else
			DispError("Probleme avec fin d'annee des clients");
#endif
			return(-1);
		}
	}

	if( pa_rec.pa_stores[0]==U_YES ){
#ifdef ENGLISH
		printf("\nINVENTORY SYSTEM\n");
#else
		printf("\nSYSTEME INVENTAIRE\n");
#endif
#ifdef ENGLISH
		printf("\tClosing Inventory For Year End\n");
#else
		printf("\tFermeture de l'inventaire pour la fin d'annee\n");
#endif
		if( StockClose( YEAR )<0 ){
#ifdef ENGLISH
			DispError("Problem with Year End Inventory closing");
#else
			DispError("Probleme avec la fermeture de l'inventaire pour la fin d'annee");
#endif
			return(-1);
		}
	}

#ifdef ENGLISH
	printf("\nYEAR END ROLLOVER\n");
#else
	printf("\nAVANCEMENT APRES LA FIN D'ANNEE\n");
#endif
	err = get_param(&pa_rec,UPDATE,1,e_mesg);	/* get again as you
						 have lost the commit buffer */
	pa_rec.pa_cur_period = 1 ;
	err = put_param(&pa_rec,UPDATE,1,e_mesg);
	if(err < 0 ) {
#ifdef ENGLISH
		DispError("ERROR in saving records");
#else
		DispError("ERREUR en conservant les fiches");
#endif
		DispError(e_mesg) ; 	
		roll_back(e_mesg);
		return(-1);
	}

	if (commit(e_mesg) < 0) {
#ifdef ENGLISH
		DispError("ERROR in saving records");
#else
		DispError("ERREUR en conservant les fiches");
#endif
		DispError(e_mesg) ;	
		return(ERROR);
	}

	return(NOERROR);
}

Process()
{
	int 	err ;
	char	file1[50];

	for(;;) {
	  gl_rec.reccod++ ;
	  err = get_n_gl(&gl_rec, UPDATE, 0, 0, e_mesg) ;

	  if (err < 0 ) {
		if ( err == ERROR ) {
	  		DispError(e_mesg);
			return(-1) ;
	  	}
		else if (err == EFL) break;
	  }

	  if (gl_rec.reccod < 97) {
#ifndef	ORACLE
		roll_back(e_mesg) ;
#endif
		continue ;
  	  }

	  if(gl_rec.funds != ctl_rec.fund) 
		if ( CheckFund(gl_rec.funds,BROWSE) < 0) return(ERROR);
			

	  MoveAmount();		/* move current figures to previous */
		
	  if (gl_rec.sect < 3) {
		AssLiab();	/* Assets & Liabilities routine */
	  	/* Reset the current surplus deficit Account 
			but do not Accumulate */
		if(strcmp(gl_rec.accno, ctl_rec.p_and_l_acnt) == 0) {
			gl_rec.opbal = gl_rec.ytd = 0.00 ;
			gl_rec.opbal = D_Roundoff(gl_rec.opbal);
			gl_rec.ytd = D_Roundoff(gl_rec.ytd);
		}
  	  }
	  else 
		ExpRev(); 	/* Expense & Revenues routine */

	  /*  Checking to see if any recurring entries were posted
	      if so set up the amount properly 				*/
	  if (gl_rec.nextdb != 0 || gl_rec.nextcr != 0) {
	 	gl_rec.curdb = gl_rec.nextdb ;
		gl_rec.curdb = D_Roundoff(gl_rec.curdb);
 	 	gl_rec.curcr = gl_rec.nextcr ;
		gl_rec.curcr = D_Roundoff(gl_rec.curcr);
		gl_rec.currel[0] = gl_rec.nextdb + gl_rec.nextcr ;
		gl_rec.currel[0] = D_Roundoff(gl_rec.currel[0]);
		gl_rec.ytd = gl_rec.ytd + gl_rec.nextdb + gl_rec.nextcr ;
		gl_rec.ytd = D_Roundoff(gl_rec.ytd);
		gl_rec.nextcr = gl_rec.nextdb = 0.00 ;
		gl_rec.nextcr = D_Roundoff(gl_rec.nextcr);
		gl_rec.nextdb = D_Roundoff(gl_rec.nextdb);
	  }

	  err = RiteRecord(UPDATE) ;	/* Writing record */
	  if ( err < 0 ) {
#ifdef ENGLISH
		DispError("Problem with UPDATE");
#else
		DispError("Probleme avec la MISE A JOUR");
#endif
		return(-1) ;
	  }
	
	}  /*  end of for */
	
	/* Remove Budget Transaction Files */
	if(pa_rec.pa_budget[0] != U_YES){
		form_f_name(BDHDR_FILE, file1);
		sprintf(e_mesg,"rm %s*",file1);
		system(e_mesg);
		form_f_name(BDITEM_FILE, file1);
		sprintf(e_mesg,"rm %s*",file1);
		system(e_mesg);
	}
	err = SurplusDeficit() ;
	return(NOERROR);
}

/* ------------------------------------------------------------------------- **
** 	The surplus/deficit is calculated for each period and accmulated for 
**	a year to date value.  The values are written to the surplus 
**	deficit account and reset for the next fund.
****************************************************************************/
SurplusDeficit()
{
	int 	i, err;
	double	diff = 0;
	
	ctl_rec.fund = 0 ;
	flg_reset(CONTROL);
	for( ;; ) {
		err = get_n_ctl(&ctl_rec, UPDATE, 0, FORWARD, e_mesg) ;
		if( err < 0) {
			if (err == EFL) 
				break;
			else {
				DispError(e_mesg);
				return(err);
			}
		}
		gl_rec.funds = ctl_rec.fund ;
		STRCPY( gl_rec.accno, ctl_rec. s_d_accm_acnt ) ;
		gl_rec.reccod = 99 ;
		err = get_gl(&gl_rec, UPDATE, 0, e_mesg) ;
		if(err == ERROR)  {
			DispError(e_mesg);
			return(ERROR);
		}
		if(err == UNDEF) {
			DispError(e_mesg);
#ifdef ENGLISH
			sprintf(e_mesg,"\n\tSurplus/Deficit: %s not in file",
				ctl_rec.s_d_accm_acnt);
#else
			sprintf(e_mesg,"\n\tSurplus/Deficit: %s pas dans le dossier",
				ctl_rec.s_d_accm_acnt);
#endif
			DispError(e_mesg);
			return(ERROR);
		}
		if(err == LOCKED) {
			DispError(e_mesg);
			return(ERROR);
	  	}

		diff = actual_chg[ctl_rec.fund -1] ;
		gl_rec.opbal += diff ;
		gl_rec.ytd += diff ;
		gl_rec.opbal = D_Roundoff(gl_rec.opbal);
		gl_rec.ytd   = D_Roundoff(gl_rec.ytd);
		
		actual_chg[ctl_rec.fund - 1] = 0;
		diff = 0;

		err = RiteRecord(UPDATE) ;
		if (err < 0) return(err) ;	


	} /* END OF FOR */
	seq_over(CONTROL);


	return(NOERROR);
}
/*----------------------------------------------------------------*/
/* Check the given funds availability in file */

CheckFund(fund,mode)
short	fund ;
int	mode ;
{
	int	err ;

	ctl_rec.fund = fund ;

	err = get_ctl(&ctl_rec, mode, 0, e_mesg) ;
	if(ERROR == err) return(DBH_ERR);
	if(err < 0) {
		DispError(e_mesg) ;
		return(ERROR) ;
	}
	return(NOERROR) ;
}	/* CheckFund() */
/*---------------------------------------------------------------*/
/* Lock Ordinary, Contract, Bank and discount records from the   */
/*  Set up new year by moving current figures to previous */
/**********************************************************/
MoveAmount()
{
	int i ;

	for (i=0;i < NO_PERIODS;i++) {
		gl_rec.prerel[i] = gl_rec.currel[i] ;
		gl_rec.prerel[i] = D_Roundoff(gl_rec.prerel[i]);
		gl_rec.currel[i] = 0.00 ;
		gl_rec.currel[i] = D_Roundoff(gl_rec.currel[i]);
		gl_rec.prebud[i] = gl_rec.curbud[i] ;
		gl_rec.prebud[i] = D_Roundoff(gl_rec.prebud[i] );
		if(pa_rec.pa_budget[0] != U_YES) {  /* carry budget figures */
			gl_rec.curbud[i] = 0.00;
			gl_rec.curbud[i] = D_Roundoff(gl_rec.curbud[i]);
		}
	}
	gl_rec.budpre = gl_rec.budcur ;
	gl_rec.budpre = D_Roundoff(gl_rec.budpre);

	if(pa_rec.pa_budget[0] != U_YES)  { /* carry budget figures */
		gl_rec.budcur = 0.00;
		gl_rec.budcur = D_Roundoff(gl_rec.budcur);
	}

	gl_rec.curdb = gl_rec.curcr = 0.00 ;
	gl_rec.curdb = D_Roundoff(gl_rec.curdb);
	gl_rec.curcr = D_Roundoff(gl_rec.curcr);
}

AssLiab()
{
	gl_rec.opbal = gl_rec.ytd ;
	gl_rec.opbal = D_Roundoff(gl_rec.opbal);
}

ExpRev()
{
	if (gl_rec.reccod == 99){            
		actual_chg[ctl_rec.fund -1] += gl_rec.ytd ;  
		actual_chg[ctl_rec.fund -1]  = 
			D_Roundoff(actual_chg[ctl_rec.fund -1]);
	}
	gl_rec.ytd = gl_rec.opbal = 0.00 ;
	gl_rec.ytd = D_Roundoff(gl_rec.ytd);
	gl_rec.opbal = D_Roundoff(gl_rec.opbal);
}
/****************************************************************************/
/*  Remove remaining commitments in previous year general ledger            */
ProcessLastYear()	/* Initiate Process */
{
	int 	err ;

	for(;;) {
	  gl_rec.reccod++ ;
	  err = get_n_gl(&gl_rec, UPDATE, 0, 0, e_mesg) ;

	  if (err < 0 ) {
		if ( err == ERROR ) {
	  		DispError(e_mesg);
			return(-1) ;
	  	}
		else if (err == EFL) break;
	  }

	  if (gl_rec.reccod < 97) {
#ifndef	ORACLE
		roll_back(e_mesg) ;
#endif
		continue ;
  	  }

	  if(gl_rec.funds != ctl_rec.fund) 
		if ( CheckFund(gl_rec.funds,BROWSE) < 0) return(ERROR);
			
	  gl_rec.comdat = 0.00 ;
	  gl_rec.comdat = D_Roundoff(gl_rec.comdat);

	  err = RiteRecord(UPDATE) ;	/* Writing record */
	  if ( err < 0 ) {
#ifdef ENGLISH
		DispError("Problem with UPDATE");
#else
		DispError("Probleme avec la MISE A JOUR");
#endif
		return(-1) ;
	  }
	
	}  /*  end of for */
	return(NOERROR);
}

RiteRecord(md)
{
	int err ;

	err = put_gl(&gl_rec, md, e_mesg) ;
	if(err < 0 ) {
#ifdef ENGLISH
		DispError("ERROR in saving records");
#else
		DispError("ERREUR en conservant les fiches");
#endif
		DispError(e_mesg) ;  
		roll_back(e_mesg) ;
		return(-1) ;
	}

  	if(commit(e_mesg) < 0)  {
#ifdef ENGLISH
		DispError("ERROR in saving records");
#else
		DispError("ERREUR en conservant les fiches");
#endif
		DispError(e_mesg) ;  
		return(-1);
	}

	return(NOERROR) ;
}


DispError(s)
char *s;
{
	printf("\n%s\n", s) ;
#ifdef ENGLISH
	printf("Press RETURN<CR>\n");
#else
	printf("Appuyer sur RETURN<CR>\n");
#endif
	fflush( stdin );
	getchar();
}

/*************
	A few reports related to different subsystems have been called by this
	file.  
	These dummy routines are defined to avoid link errors when linked with
	files from other modules for integrating the end of month process(es).
*************/

GetFilename(ptr)
char	*ptr;
{
	return(0);
}
GetOutputon(ptr)
char	*ptr;
{
	return(0);
}
GetDateRange(ptr)
char	*ptr;
{
	return(0);
}
GetPoRange(ptr)
char	*ptr;
{
	return(0);
}
DisplayMessage(ptr)
char	*ptr;
{
	return(0);
}
GetResponse(ptr)
char	*ptr;
{
	return(0);
}
Confirm()
{
	return(0);
}

GetNbrCopies(par)
short	par;
{
	return(0);
}

/*-------------------- E n d   O f   P r o g r a m ---------------------*/
