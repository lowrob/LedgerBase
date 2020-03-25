/*-----------------------------------------------------------------------
Source Name: merge.c
System     : Merge of previous year to current year
Module     : General Ledger system.
Created  On: 09 August 89.
Created  By: Cathy Burns.


DESCRIPTION:
	Program to process the Merge of previous year to current year
after yearend has already been completed.

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
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
C.Leadbeater   90/12/19       Added D_Roundoff() to round calculated double
			      values before writing to file.

------------------------------------------------------------------------*/
#define  MAIN
#define  MAINFL		GLMAST		/* main file used */

#include <stdio.h>
#include <cfomstrc.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define	SYSTEM		"CLOSING PROCESSES"	/* Sub System Name */
#define	MOD_DATE	"19-DEC-90"		/* Program Last Modified */

#ifdef ENGLISH
#define U_YES	'Y'
#define L_YES	'y'
#else
#define U_YES	'O'
#define L_YES	'o'
#endif

static	char 	e_mesg[80];  		/* dbh will return err msg in this */
static  Gl_rec	gl_rec ;
static  Gl_rec	gl_old ;
static	Ctl_rec	ctl_rec ;
static  Pa_rec	pa_rec ;
static 	Tr_item	tr_item ;
static	double	difference;
static	double	actual_chg[NO_PERIODS];
static	int 	prev_fund;
double	D_Roundoff();

main(argc,argv) 
int argc;
char *argv[];
{
	int	err;

	strncpy(SYS_NAME,SYSTEM,50);	/* Sub system name */
	strncpy(CHNG_DATE,MOD_DATE,10);	/* Modification Date */

	proc_switch(argc, argv, MAINFL) ; 	/* Process Switches */
	/* Merge is from prev_yr to current data base */
	if(SW8 || SW9) {
#ifdef ENGLISH
		printf("ERROR: Wrong Switches...\n");
#else
		printf("ERREUR: mauvais indicateurs...\n");
#endif
		exit(-1) ;
	}
	
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


#ifdef ENGLISH
	printf("\n\n\n\n\n\tMERGE PROCEDURES\n\n\n");
	printf("\t\tMerge previous year adjustments to current year\n\n");
	printf("\t\tProceed (Y/N)? ");
#else
	printf("\n\n\n\n\n\tFUSION DES PROCEDURES\n\n\n");
	printf("\t\tFusionner les ajustements de l'annee precedente avec l'annee courante\n\n");
	printf("\t\tProceder (O/N)? ");
#endif


	scanf("%s",e_mesg);
	if(e_mesg[0] != L_YES && e_mesg[0] != U_YES) {
		close_dbh();
		exit(-1);
	}

	SW9 = 0;

	err = get_param(&pa_rec,UPDATE,1,e_mesg);
	if(err == ERROR)
		DispError(e_mesg);
	else
	if (err == UNDEF)
#ifdef ENGLISH
		DispError("Parameters are not Setup ...");
#else
		DispError("Parametres ne sont pas etablis... ");
#endif
	else
		err = initial() ;  	/*  initialization */

	close_dbh();			/* Close files */

	if(err < 0) {
#ifdef ENGLISH
	 	DispError("ERROR: Merge Unsuccessful. Press RETURN ") ;
#else
	 	DispError("ERREUR: Fusion n'est pas reussie. Appuyer sur RETURN");
#endif
 		exit(-1);
	}
#ifdef ENGLISH
	DispError("Merge successfully completed. Press RETURN ") ;
#else
	DispError("Fusion reussie. Appuyer sur RETURN");
#endif
	exit(0);
} /* END OF MAIN */
initial()
{
	int 	i,	j,	err;
	char	file1[50], file2[50];

#ifdef ENGLISH
	printf("\nSTEP 1:  PROCESSING PREVIOUS YEAR GLMAST\n");
#else
	printf("\nETAPE 1: TRAITEMENT DU G/L MAITRE DE L'ANNEE PRECEDENTE\n");
#endif
	prev_fund = 0 ;

	for (i=0;i < NO_PERIODS;i++) {
		actual_chg[i] = 0;
	}

	SW9 = 1;			/* Switch to set up previous glmast */
	close_dbh() ;
	if (  ProcessLastYear() < 0 ) return( ERROR );	/* Initiate Process */

	close_dbh() ;
	
#ifdef ENGLISH
	printf("\nSTEP 2:  PROCESSING CURRENT YEAR GLMAST\n");
#else
	printf("\nETAPE 2: TRAITEMENT DU G/L MAITRE DE L'ANNEE COURANTE\n");
#endif
	prev_fund = 0 ;

	/* Set the TMPINDX_1 as a GLMAST file. Open Previous Year GLMAST as
	   TMPINDX_1 file */
	if( set_id(TMPINDX_1, GLMAST, e_mesg) < 0) {
		printf("%s\n", e_mesg) ;
		close_dbh() ;
		exit(-1) ;
	}

#ifndef	ORACLE
	SW9 = 1 ;		/* Switch to set path to previous year data */
	if(init_file(TMPINDX_1) < 0) {
#ifdef ENGLISH
		printf("Error In Accessing Previous Year Data\n");
		printf("Iserror: %d    Dberror: %d    Errno: %d\n", iserror,
			dberror, errno) ;
#else
		printf("Erreur en accedant aux donnees de l'annee precedente\n");
		printf("Iserror: %d    Dberror: %d    Errno: %d\n", iserror,
			dberror, errno) ;
#endif
		close_dbh() ;
		exit(-1);
	}
#endif
	SW9 = 0;

	if (  LockSurplusDeficit() < 0 ) return( ERROR );
	if (  ProcessCurrentYear() < 0 ) return( ERROR );

	/****
#ifdef ENGLISH
	printf("\n\n  FINISHED PROCESSING MERGE \n\n");
#else
	printf("\n\n  TERMINE LE PROCESSUS DE FUSION \n\n");
#endif
	fflush( stdin );
	read(0, e_mesg, 80) ;
	****/

	return(NOERROR);
	
}
/* ------------------------------------------------------------------------- **
**	Making sure the Surplus/Deficit Exists before doing merge.           **
**	                                                                     **
******************************************************************************/
LockSurplusDeficit()
{
	int err ;

	ctl_rec.fund = 0 ;
	flg_reset(CONTROL) ;

	for( ; ; ) {
		err = get_n_ctl(&ctl_rec, BROWSE, 0, FORWARD, e_mesg);
		if (err < 0) {
			if (err == EFL) 
				break ;
			else {
				DispError(e_mesg);
		    		return(err);
			}
		}

		gl_rec.funds = ctl_rec.fund ;
		STRCPY( gl_rec.accno, ctl_rec.s_d_accm_acnt);
		gl_rec.reccod = 99 ;
		err = get_gl(&gl_rec, UPDATE, 0, e_mesg) ;
		if (err == ERROR) {
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
	} /* END OF FOR */
	seq_over(CONTROL);

	return(NOERROR) ;
}


/* ------------------------------------------------------------------------- **
**	Calculating Surplus / Deficit for previous year after audit          **
**	adjustments are entered.                                             **
******************************************************************************/
ProcessLastYear() 
{
	int 	err, i  ;
	int	tempfund;
	
	gl_rec.funds= 1 ;
	gl_rec.sect = 3 ;
	gl_rec.reccod = 99 ;
	gl_rec.accno[0] = '\0' ;
	flg_reset(GLMAST) ;

	for(;;) {
		err = get_n_gl(&gl_rec, BROWSE, 3, FORWARD, e_mesg) ;
		 if (err < 0 ) {
			if (err == EFL) 
				break;
			else {
#ifdef ENGLISH
				printf("PREV GLMAST - %s\n",e_mesg);
#else
				printf("G/L MAITRE PRECEDENT - %s\n",e_mesg);
#endif
				return(ERROR) ;
	  		}
	  	}

		if (gl_rec.reccod != 99) continue ;
		if (gl_rec.sect != 3 && gl_rec.sect != 4) continue ;

	  	if (gl_rec.funds != prev_fund)  {
		   if(	err = CheckFund(gl_rec.funds,BROWSE) < 0) return(ERROR);
			prev_fund = gl_rec.funds ;
			gl_rec.sect = 2 ;
			gl_rec.reccod = 99 ;
			STRCPY(gl_rec.accno, ctl_rec.s_d_accm_acnt);
			gl_rec.reccod = 99 ;
			err = get_gl(&gl_rec, BROWSE, 0, e_mesg) ;
			actual_chg[prev_fund - 1] += gl_rec.ytd ;
			gl_rec.sect = 3 ;
			gl_rec.reccod = 99 ;
			gl_rec.accno[0] = '\0' ;
			flg_reset(GLMAST);
			continue;
	  	}

		actual_chg[prev_fund - 1] += gl_rec.ytd ;

	}  /*  end of for */
	seq_over(GLMAST) ;

	return(NOERROR); 
}

/* ------------------------------------------------------------------------- **
**	Move effects of previous years adjustments to current year files.    **
**	                                                                     **
******************************************************************************/
ProcessCurrentYear()
{
	int 	err, i  ;
	int	tempfund;

	gl_old.funds = 0;
	gl_old.accno[0] = '\0' ;
	gl_old.reccod = 0 ;

#ifndef	ORACLE
	flg_reset(TMPINDX_1) ;
#else
	close_file(TMPINDX_1) ;	/* To be Safe */
	SW9 = 1;		/* Only next get_next opens the pr. year file */
#endif

	err = SurplusDeficit();

	for(;;) {
		err = get_next((char*)&gl_old, TMPINDX_1, 0, 0, BROWSE, e_mesg);
#ifdef	ORACLE
		if(SW9) SW9 = 0 ;	/* TMPINDX_1 is active on pr year file*/
#endif
	  	if (err < 0 ) {
			if (err == ERROR) {
#ifdef ENGLISH
				printf("PREV GLMAST - %s\n",e_mesg);
#else
				printf("G/L MAITRE PRECEDENT - %s\n",e_mesg);
#endif
				return(ERROR) ;
	  		}
			else if (err == EFL) break;
	  	}
	  	if (gl_old.reccod < 53) continue;/* title accts not processed */

	 	 gl_rec.funds = gl_old.funds;
		 STRCPY( gl_rec.accno, gl_old.accno);
		 gl_rec.reccod = gl_old.reccod;
		 err = get_gl(&gl_rec, UPDATE, 0, e_mesg) ;
	
		 if (err < 0 ) {
			if (err == UNDEF) {
#ifdef ENGLISH
				sprintf(e_mesg, "G/L Account no longer exists %s .", gl_old.accno) ;
#else
				sprintf(e_mesg, "Compte G/L n'existe plus %s .", gl_old.accno) ;
#endif
				DispError(e_mesg) ;
				continue ;
			}
		  	DispError(e_mesg);
			return(ERROR) ;
		}

		if( gl_rec.funds != ctl_rec.fund )  
		    if(	err = CheckFund(gl_rec.funds,BROWSE) < 0) return(ERROR);

		if( strcmp(gl_rec.accno, ctl_rec.s_d_accm_acnt) == 0 ) {
			roll_back(e_mesg) ;
			continue ;
		}
		if(strcmp(gl_rec.accno, ctl_rec.p_and_l_acnt) == 0) {
			roll_back(e_mesg) ;
			continue ;
		}
	
	  	difference = 0;
		
 	 	for (i=0;i < NO_PERIODS;i++) {
			gl_rec.prerel[i] = gl_old.currel[i] ;
			gl_rec.prebud[i] = gl_old.curbud[i] ; 
	  	}

	  	gl_rec.budpre = gl_old.budcur ;
	
	  	if( gl_rec.sect < 3 ) {
			difference = gl_old.ytd - gl_rec.opbal ;
			gl_rec.opbal += difference ;
			gl_rec.ytd += difference ;
	  	}

		if (  RiteCurrentRecord(UPDATE) < 0 ) return(ERROR);

	  	gl_old.reccod++ ;
	
	}  /*  end of for */
	seq_over(TMPINDX_1) ;

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

		diff = actual_chg[ctl_rec.fund -1] - gl_rec.opbal ;
		gl_rec.opbal += diff ;
		gl_rec.ytd += diff ;

		actual_chg[ctl_rec.fund - 1] = 0;
		diff = 0;

		err = RiteCurrentRecord(UPDATE) ;
		if (err < 0) return(err) ;	

  		if(commit(e_mesg) < 0)  {
#ifdef ENGLISH
			DispError("ERROR in saving records");
#else
			DispError("ERREUR en conservant les fiches");
#endif
			DispError(e_mesg) ;  
			return(DBH_ERR);
		}


	} /* END OF FOR */
	seq_over(CONTROL);


	return(NOERROR);
}

/*****************************************************************************/
/*   Updating current year record.                                          */
RiteCurrentRecord(md)
int	md ;

{
	int err ;

	gl_rec.opbal = D_Roundoff(gl_rec.opbal); 
	gl_rec.ytd = D_Roundoff(gl_rec.ytd); 

	err = put_gl(&gl_rec, md, e_mesg) ;
	if(err != NOERROR) {
#ifdef ENGLISH
		DispError("ERROR in saving records");
#else
		DispError("ERREUR en conservant les fiches");
#endif
		DispError(e_mesg) ;  
		roll_back(e_mesg) ;
		return(DBH_ERR) ;
	}

  	if(commit(e_mesg) < 0)  {
#ifdef ENGLISH
		DispError("ERROR in saving records");
#else
		DispError("ERREUR en conservant les fiches");
#endif
		DispError(e_mesg) ;  
		return(DBH_ERR);
	}

	return(NOERROR) ;
}

DispError(s)
char *s;
{
	printf("\n%s\n", s) ;
	read(0, e_mesg, 80) ;
}


/*-------------------- E n d   O f   P r o g r a m ---------------------*/

