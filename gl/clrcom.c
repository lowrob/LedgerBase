
/*-----------------------------------------------------------------------
Source Name: clrcom.c
System     : Year End Processing
Module     : General Ledger system.
Created  On: 19 July 94.
Created  By: Louis Robichaud.


DESCRIPTION:
	This program was created to clear the commitments in the previous
	year. It was created using code copied from the yearend.c program.
	The reason for this is because the copying of the data to the previous
	year was not successful. The data was manualy copied to the previous
	year directory


------------------------------------------------------------------------*/
#define  MAIN
#define  MAINFL		-1	/* no main file used */

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define NO_PERIODS	13
#define	SYSTEM		"CLOSING PROCESSES"	/* Sub System Name */
#define	MOD_DATE	"19-JUL-94"		/* Program Last Modified */


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
/****************************************************************************/
/*  Remove remaining commitments in previous year general ledger            */
ProcessLastYear()	/* Initiate Process */
{
	int 	err ;

	gl_rec.funds = 0;
	gl_rec.accno[0] = '\0' ;
	gl_rec.reccod = 0 ;
	flg_reset( GLMAST );
	
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
