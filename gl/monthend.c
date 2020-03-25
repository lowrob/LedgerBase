/*----------------------------------------------------------------------
Source Name: monthend.c
System     : Month End Processing
Module     : General Ledger system.
Created  On: 27 July 89.
Created  By: Cathy Burns.


DESCRIPTION:
	Program to process the month end procedures.

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
J.Prescott     90/11/23       Added prompt to change paper for customer 
			      statements.
F.Tao	       90/12/07	      Added a FOR Loop for calling customer statement.
J.Prescott     90/12/18       Added D_Roundoff to fix precision problem 
L.Robichaud    93/12/01		The DispError function does not display
				anything because their is no screen in this
				program. I wrote a DispError function and do
				NOT use fomen or fomer in this program.
------------------------------------------------------------------------*/
#define  MAIN
#define  MAINFL		-1		/* no main file used */

#include <cfomstrc.h>
#include <repdef.h>
#include <stdio.h>
#include <bfs_pp.h>
#include <bfs_defs.h>
#include <bfs_recs.h>
#include <requ.h>		/* for tax calculations */

#ifdef	MS_DOS
#include <dos.h>
#else			/* UNIX or XENIX */
#include <sys/types.h>
#include <sys/dir.h>
#endif

#define	SYSTEM		"CLOSING PROCESSES"	/* Sub System Name */
#define	MOD_DATE	"23-NOV-90"		/* Program Last Modified */

#ifdef ENGLISH
#define U_NO 	'N'
#define L_NO  	'n'
#define U_YES	'Y'
#define L_YES	'y'
#define EXIT	12
#else
#define U_NO 	'N'
#define L_NO  	'n'
#define U_YES	'O'
#define L_YES	'o'
#define EXIT	12
#endif

char 	e_mesg[80];  		/* dbh will return err msg in this */
static  Jrh_ent	jrh_ent ;
static  Tr_hdr	tr_hdr ;
static  Gl_rec	gl_rec ;
static  Emp	emp_rec ;
static  Pay_param	pay_param ;
static  Pay_per	pay_period ;
static  Barg_unit	barg_unit ;
static  Pa_rec	pa_rec ;
static  Ctl_rec	ctl_rec ;
static  Tr_item	tr_item ;
static	Man_chq	man_chq;

static	double	grnd_total ;
static	double	grnd_total_dr ;
static	double	grnd_total_cr ;
static	double	periodytd[13];
static	int 	prev_fund;

/* number of days in months */

static	short d_month[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

static int	PG_SIZE;
static short	pgcnt; 		/* for page count */
double	D_Roundoff();
static	short	accrual_day;
static	short	item_no1;
static	short	item_no2;
static	short	number_of_days;
static	long	tmp_seq_no,end_date_week,end_date_bi;
static	char	accrual_flag[2];
static 	long	sysdate;

static	short	before_period,after_period;

main(argc,argv) 
int argc;
char *argv[];
{
	int	err, sno=0;
	int	fd ;
	char	buffer[20] ;

	if(argc < 2){
#ifdef  DEVELOP
		printf("MAIN ARGUMENTS ARE NOT PROPER\n");
		printf("Usage : %s {-tterminal name}\n", argv[0]);
#endif
		exit(1);
	}

	strncpy(SYS_NAME,SYSTEM,50);	/* Sub system name */
	strncpy(CHNG_DATE,MOD_DATE,10);	/* Modification Date */

	proc_switch(argc, argv, MAINFL) ; 	/* Process Switches */

	err = GetUserClass(e_mesg) ;
	if(err < 0 || (err != ADMINISTRATOR && err != SUPERUSER)) {
		if(err == DBH_ERR)
			printf("%s\n",e_mesg);
		else
#ifdef ENGLISH
			printf("\n\n\tACCESS DENIED\n");
#else
			printf("\n\n\tACCES NIE\n");
#endif
		close_dbh();
		exit(-1);
	}


	if(lock_file(PARAM) < 0) {
#ifdef ENGLISH
		printf("System is up on other Terminals... Can't Proceed\n");
#else
		printf("Systeme est sur d'autres terminaux... Ne peut pas proceder\n");
#endif
		close_dbh();
		exit(-1);
	}

	err = get_param(&pa_rec,UPDATE,1,e_mesg);
	if(err < 0) {
		if(err == ERROR)
			DispError(e_mesg);
		else if (err == UNDEF)
#ifdef ENGLISH
			DispError("Parameters are not Setup ...");
#else
			DispError("Parametres ne sont pas etablis... ");
#endif
		close_dbh() ;
		exit(-1);
	}

	if (pa_rec.pa_cur_period == 0 ) {
#ifdef ENGLISH
		DispError("ERROR : Cannot Process Period End. All the Periods are Closed");
#else
		DispError("ERREUR: Ne peut pas traiter la fin de periode. Toutes les periodes sont fermees");
#endif
		close_dbh() ;
		exit(-1) ;
	}

	err = settax() ;
	if(err < 0) {
#ifdef ENGLISH
		DispError("ERROR: Setting up tax percentages");
#else
		DispError("ERREUR: Etablissement du pourcentage de taxe");
#endif
		close_dbh();
		exit(-1);		
	}


#ifdef	JRNL
	/* Between Month end process and eod process, there should not be
	   any updates to DataBase */

	form_f_name( JOURNAL, e_mesg );	
	sprintf( buffer,"%ld",get_date());
	strcat( e_mesg, buffer );
	fd = open(e_mesg, RDMODE) ;
	if(fd >= 0) {
		if( lseek( fd, 0L, 2) > 0) {
#ifdef ENGLISH
			DispError("ERROR: This process must be done immediately after End of Day Process") ;
#else
			DispError("ERREUR: Ce processus doit etre fait immediatement apres le processus de fin de journee") ;
#endif

			close(fd) ;
			close_dbh() ;
			exit(-1) ;
		}
		close(fd) ;
	}
#endif

#ifdef ENGLISH
	printf("\n\n\n\n\n\tPERIOD END PROCEDURES  for the Period: %d\n\n\n",
		pa_rec.pa_cur_period);
#else
	printf("\n\n\n\n\n\tPROCEDURES DE FIN DE PERIODE pour la periode: %d\n\n\n",
		pa_rec.pa_cur_period);
#endif

#ifndef	ORACLE
#ifdef ENGLISH
	printf("\t%2d. Backs up the Data Base\n",++sno);
#else
	printf("\t%2d. Sauvegarde la base des donnees\n",++sno);
#endif
#endif
#ifdef ENGLISH
	printf("\t%2d. Purges the pay advances over 30 days old\n",++sno);
#else
	printf("\t%2d. Purges the pay advances over 30 days old\n",++sno);
#endif

	if (pa_rec.pa_cur_period != 13) 
#ifdef ENGLISH
	printf("\t%2d. Prints Transactions of the Day\n",++sno);
#else
	printf("\t%2d. Imprime les transactions de la journee\n",++sno);
#endif

	if( pa_rec.pa_requisition[0]==U_YES ) 
#ifdef ENGLISH
		printf("\t%2d. Purges Processed/Disapproved Requisitions\n",++sno);
#else
		printf("\t%2d. Efface les requisition traitees/non-approuvees\n",++sno);
#endif

	if( pa_rec.pa_aps[0]==U_YES ) 
#ifdef ENGLISH
		printf("\t%2d. Purges Completed Purchase Orders\n",++sno);
#else
		printf("\t%2d. Efface les bons de commande completes\n",++sno);
#endif

	if( pa_rec.pa_stores[0]==U_YES )
#ifdef ENGLISH
		printf("\t%2d. Closes Inventory Stock for the Period\n",++sno);
#else
		printf("\t%2d. Ferme l'inventaire pour la periode\n",++sno);
#endif

	if( pa_rec.pa_ars[0]==U_YES ){
#ifdef ENGLISH
		printf("\t%2d. Prints Customer Statement\n",++sno);
		printf("\t%2d. Closes Customer Accounts for the Period\n",++sno);
#else
		printf("\t%2d. Imprime le releve du client\n",++sno);
		printf("\t%2d. Ferme les comptes clients pour la periode\n",++sno);
#endif
	}
#ifdef ENGLISH
	printf("\t%2d. Calculates the Surplus & Deficit for the Period\n",++sno);
	printf("\t%2d. Closes G/L Accounts for the Period\n",++sno);
	printf("\t%2d. Rolls the Period to next Period\n",++sno);
#ifdef JOURNAL
	printf("\t%2d. Purge Backup Journal Files\n\n",++sno);
#else
	printf("\n");
#endif
	printf("\t\tProceed (Y/N)? ");
#else
	printf("\t%2d. Calcule le surplus & deficit pour la periode\n",++sno);
	printf("\t%2d. Ferme les comptes G/L pour la periode\n",++sno);
	printf("\t%2d. Avance la periode a la periode suivante\n",++sno);
#ifdef JOURNAL
	printf("\t%2d. Efface la copie de sauvegarde des dossiers de journal\n\n",++sno);
#else
	printf("\n");
#endif
	printf("\t\tProceder (O/N)? ");
#endif

	scanf("%s",e_mesg);
	if(e_mesg[0] != L_YES && e_mesg[0] != U_YES) {
		close_dbh();
		exit(-1);
	}

	for(;;){
		printf("\n\n\t\tCalculate Payroll Accruals for this Month (Y/N)? ");

		scanf("%s",accrual_flag);
		if(accrual_flag[0] == L_YES || accrual_flag[0] == U_YES ||
		    accrual_flag[0] == L_NO || accrual_flag[0] == U_NO) 
			break;
	}

#ifndef ORACLE
#ifdef ENGLISH
	printf("\nBACKING UP FILES\n");
#else
	printf("\nSAUVEGARDE LES DOSSIERS\n");
#endif
	Backup();
#endif

/* Louis R. */
	printf("\n\nPurging the pay advances");
	err = padvpurge();
	if(err < 0)
		printf("\n\nProblem occured purging pay advances. Monthend will continue");

#ifdef ENGLISH
	printf("\n\nGL SUBSYSTEM");
	printf("\n\tPrinting transaction listing's\n");
#else
	printf("\n\nSOUS-SYSTEME G/L");
	printf("\n\tImprime la liste des transactions\n");
#endif

	if( (err = eod_tr_list(99,e_mesg))<0 ) { /* EOD trans listing */
		if ( err == UNDEF ) 
#ifdef ENGLISH
			printf("\n\tNo 99 Transactions To Be Printed\n\n");
		else
			printf("\n\tError In Transaction Listing: %s\n",e_mesg);
#else
			printf("\n\tPas de transactions 99 a imprimer\n");
		else
			printf("\n\tErreur dans la liste des transactions: %s\n",e_mesg);
#endif
	}
#ifdef	i386
	system("lp -o nobanner eodtrlst99.dat");
#else
	system("lpr -b eodtrlst99.dat");
#endif
	if( (err = eod_tr_list(97,e_mesg))<0 ) { /* EOD trans listing */
		if ( err == UNDEF ) 
#ifdef ENGLISH
			printf("\n\tNo 97 Transactions To Be Printed\n\n");
		else
			printf("\n\tError in Transaction Listing: %s\n",e_mesg);
#else
			printf("\n\tPas de transactions 97 a imprimer\n");
		else
			printf("\n\tErreur dans la liste des transactions: %s\n",e_mesg);
#endif
	}
#ifdef	i386
	system("lp -o nobanner eodtrlst97.dat");
#else
	system("lpr -b eodtrlst97.dat");
#endif
	if( (err = eod_tr_list(98,e_mesg))<0 ) { /* EOD trans listing */
		if ( err == UNDEF ) 
#ifdef ENGLISH
			printf("\n\tNo 98 Transactions To Be Printed\n\n");
		else
			printf("\n\tError In Transaction Listing: %s\n",e_mesg);
#else
			printf("\n\tPas de transactions 98 a imprimer\n\n");
		else
			printf("\n\tErreur dans la liste des transactions: %s\n",e_mesg);
#endif
	}
#ifdef	i386
	system("lp -o nobanner eodtrlst98.dat");
#else
	system("lpr -b eodtrlst98.dat");
#endif



	prev_fund = 0 ;
	err = initial() ;
	if(err < 0)
#ifdef ENGLISH
		printf("\n\nERROR: Period End is Not Done Successfully.");
	else
		printf("\n\nPeriod End Successful. Press RETURN ");
#else
		printf("\n\nERREUR: Fin de periode n'est pas reussie.");
	else
		printf("\n\nFin de periode reussi. Appuyer sur RETURN ");
#endif

	fflush( stdin );
	read(0, e_mesg, 50) ;

	close_dbh();			/* Close files */

	if(err != NOERROR)exit(-1);
	exit(0);
} /* END OF MAIN */

initial()
{
	int	i, err;
	
	prev_fund = 0 ;

	for (i=0;i < 13;i++)          	/* initializing ytd temp variables */  
		periodytd[i] = 0;

/***************************************************
The accruals is commented out because the month of Nov ended on a tuesday
which caused a problem with calculating accruals when 0 is in pay param
weekly field. The read of the Pay Period File gave an error.
Louis R.
**********************************************************/

	if(accrual_flag[0] == L_YES || accrual_flag[0] == U_YES){ 
		err = Accruals();
		if( err!=0){
#ifdef ENGLISH
			DispError("Problem with calculating payroll accruals");
#else
			DispError("Probleme en calculant les ");
#endif
			return(ERROR);
		}
	}

	if( pa_rec.pa_requisition[0]==U_YES ) {
#ifdef ENGLISH
		printf("\n\nREQUISITION SUBSYSTEM");
		printf("\n\tPurging Processed/Disapproved Requisitions\n");
#else
		printf("\n\nSOUS-SYSTEME DES BC");
		printf("\n\tEfface les requisitions traitees/non-approuvees\n");
#endif
		err = ReqPurge() ;	/* Removing completed PO */
		if( err!=0){
#ifdef ENGLISH
			DispError("Problem with purging Processed/Disapproved Requisitions");
#else
			DispError("Probleme en effacant les req traitees/non-approuvees");
#endif
			return(ERROR);
		}
	}

	if( pa_rec.pa_aps[0]==U_YES ) {
#ifdef ENGLISH
		printf("\n\nPO SUBSYSTEM");
		printf("\n\tPurging completed POs\n");
#else
		printf("\n\nSOUS-SYSTEME DES BC");
		printf("\n\tEfface les B.C. completes\n");
#endif
		err = PoPurge() ;	/* Removing completed PO */
		if( err!=0){
#ifdef ENGLISH
			DispError("Problem with purging completed PO");
#else
			DispError("Probleme en effacant le B.C. complete");
#endif
			return(ERROR);
		}
	}

	if( pa_rec.pa_stores[0]== U_YES ){
#ifdef ENGLISH
		printf("\n\nINVENTORY SUBSYSTEM");
		printf("\n\tClosing Inventory Stock\n");
#else
		printf("\n\nSOUS-SYSTEME INVENTAIRE");
		printf("\n\tFermeture de l'inventaire\n");
#endif
		err = StockClose( MONTH ); /* stock closing for the month */
		if( err!=0){
#ifdef ENGLISH
			DispError("Problem with Stock closing");  
#else
			DispError("Probleme avec la fermeture des stocks");
#endif
			return(ERROR);
		}
	}

	if( pa_rec.pa_ars[0]==U_YES ){
#ifdef ENGLISH
		printf("\n\nAR SUBSYSTEM");
		printf("\n\tPrinting Customer statement");
		printf("\n\n\tChange Paper for Customer Statements");
		printf("\n\tPress RETURN to continue");
#else
		printf("\n\nSOUS-SYSTEME DES C/R");
		printf("\n\tImprime le releve du client");
		printf("\n\n\tChanger de papier pour releve du client");
		printf("\n\tAppuyer sur RETURN pour continuer");
#endif

		fflush( stdin );
		read(0, e_mesg, 50) ;

		for (; ;){
#ifdef	ENGLISH
			printf("\n\n\t Is Customer Statement Form Aligned (Y/N)?");
#else
			printf("\n\n\t Est-ce que le formulaire de releve du client est aligne (O/N)?");
#endif
			scanf("%s",e_mesg);
			if(e_mesg[0] == L_YES || e_mesg[0] == U_YES) break;
			if(e_mesg[0] == L_NO  || e_mesg[0] == U_NO){ 
				err = custstat(2);
				if( err!=0){
#ifdef ENGLISH
					DispError("Problem with Customer Statement");  
#else
					DispError("Probleme avec le releve du client");
#endif	
					return(ERROR);
				}
			} 
		}	
		err = custstat( 1 );	/* non interactive statement */
		if( err!=0){
#ifdef ENGLISH
			DispError("Problem with Customer Statement");  
#else
			DispError("Probleme avec le releve du client");
#endif
			return(ERROR);
		}
#ifdef ENGLISH
		printf("\n\tChange Back to Blank Paper");
		printf("\n\tPress RETURN to Continue");
		fflush( stdin );
		read(0, e_mesg, 50) ;
		printf("\n\tClosing Customer Accounts\n");
#else
		printf("\n\tRechanger au papier blanc");
		printf("\n\tAppuyer sur RETURN pour continuer");
		fflush( stdin );
		read(0, e_mesg, 50) ;
		printf("\n\tFermeture des comptes clients\n");
#endif
		err = CustEnd( MONTH );	/* Month end for Customer */
		if( err!=0){
#ifdef ENGLISH
			DispError("Problem with Closing Customer Accounts");  
#else
			DispError("Probleme avec la fermeture des comptes clients");
#endif
			return(ERROR);
		}
	}

#ifdef ENGLISH
	printf("\n\nG/L SUBSYSTEM");
	printf("\n\tCalculating Surplus & Deficit");
	printf("\n\tClosing G/L Accounts\n");
#else
	printf("\n\nSOUS-SYSTEME G/L");
	printf("\n\tCalcule le surplus & deficit");
	printf("\n\tFermeture des comptes G/L\n");
#endif
	gl_rec.funds = 0;
	gl_rec.accno[0] = '\0' ;
	gl_rec.reccod = 0 ;
	err = Process(); 		/* Initiate Process */
	if( err!=NOERROR ){
#ifdef ENGLISH
		DispError("Problem with Period Closing");  
#else
		DispError("Probleme avec la fermeture de periode");
#endif
		return(ERROR);
	}

#ifdef ENGLISH
	printf("\n\nPERIOD ROLLOVER\n\n");
#else
	printf("\n\nAVANCEMENT DE LA PERIODE\n\n");
#endif
	err = get_param(&pa_rec,UPDATE,1,e_mesg);
	if(err < 0) {
		if(err == ERROR)
			DispError(e_mesg);
		else if (err == UNDEF)
#ifdef ENGLISH
			DispError("Parameters are not Setup ...");
#else
			DispError("Parametres ne sont pas etablis... ");
#endif
		close_dbh() ;
		exit(-1);
	}
	/*  Setting up for the next period to be processed */
	if (pa_rec.pa_cur_period == pa_rec.pa_no_periods)
		pa_rec.pa_cur_period = 0 ;
	else
		pa_rec.pa_cur_period += 1 ;

	err = put_param(&pa_rec,UPDATE,1,e_mesg);
	if(err < 0) {
#ifdef ENGLISH
		DispError("ERROR in saving records");
#else
		DispError("ERREUR en conservant les fiches");
#endif
		DispError(e_mesg) ;	
		roll_back(e_mesg);
	}
	else if ((err = commit(e_mesg)) < 0) {
#ifdef ENGLISH
		DispError("ERROR in saving records");
#else
		DispError("ERREUR en conservant les fiches");
#endif
		DispError(e_mesg) ;	
	}

#ifdef ENGLISH
	printf("\n\nPURGING BACKUP JOURNAL FILES\n\n");
#else
	printf("\n\nEFFACE LA COPIE DE SAUVEGARDE DES DOSSIERS DE JOURNAL\n\n");
#endif

	if(PurgeJournals()<0) {
#ifdef ENGLISH
		DispError("ERROR purging Backup Journal Files");
#else
		DispError("ERREUR en effacant la copie de sauvegarde des dossiers de journal");
#endif
	}

	return(err);
}

#ifndef	ORACLE
Backup()
{
	char	file1[50], file2[50] ;
	char	in_str[80] ;

	unlock_file(PARAM) ;	/* else cp command will hang being child */

	form_f_name("", file1);
	form_f_name(BACK_UP, file2); 
#ifdef	MS_DOS
	sprintf(in_str,"copy %s*.* %s", file1, file2);
#else
	sprintf(in_str,"cp %s* %s", file1, file2);
#endif
#ifdef ENGLISH
	printf( "Command: %s\n", in_str) ;
#else
	printf( "Commande: %s\n", in_str) ;
#endif
	system(in_str);

	if(lock_file(PARAM) < 0) {
#ifdef ENGLISH
		printf("System is up on other Terminals... Can't Proceed\n");
#else
		printf("Systeme est sur d'autres terminaux... Ne peut pas proceder\n");
#endif
		close_dbh();
		exit(-1);
	}
}
#endif

Process()
{
	int 	i, err ;
	int	tempfund;

	for(;;) {
	  err = get_n_gl(&gl_rec, UPDATE, 0, 0, e_mesg) ;
	  if (err == ERROR) {
	  	DispError(e_mesg);
		return(ERROR) ;
	  }

	  if (err == EFL) 
		break;
	
	  if (err < 0) {
		DispError(e_mesg); 
		return(ERROR);
	  }

	  if (gl_rec.reccod < 97) {
#ifndef	ORACLE
		roll_back(e_mesg) ;
#endif
		continue ;
	  }

	  if (gl_rec.funds != prev_fund)  {
		tempfund = gl_rec.funds;
		if (prev_fund != 0)  {
			err = FundBreak();
			if (err != NOERROR) { 
#ifdef ENGLISH
				DispError("Problem with Fund Break");
#else
				DispError("Probleme avec le sectionnement du fond");
#endif
				return(err);
			}
			prev_fund = tempfund ;
			gl_rec.funds = tempfund ;
			gl_rec.accno[0] = '\0' ; 	/* set up next fund */
			gl_rec.reccod = 0 ;
			flg_reset(GLMAST);
			continue;
		}
		else 	prev_fund = tempfund ;
	  }

		
	  if (gl_rec.sect > 2) 
		if (gl_rec.reccod == 99)      
			for(i=0;i < pa_rec.pa_no_periods;i++)
				periodytd[i] += gl_rec.currel[i] ; 

/*  The accrual entries that were posted must be recorded in the next period
*/
	  gl_rec.curdb = gl_rec.curcr = 0 ;
	  if (gl_rec.nextdb != 0 || gl_rec.nextcr != 0)
		  if (pa_rec.pa_cur_period != pa_rec.pa_no_periods &&
		   	pa_rec.pa_cur_period != 12)  {
			gl_rec.curdb = gl_rec.nextdb ;
			gl_rec.curcr = gl_rec.nextcr ;
			gl_rec.currel[pa_rec.pa_cur_period] = 
				gl_rec.nextdb + gl_rec.nextcr ;
			gl_rec.currel[pa_rec.pa_cur_period] = 
				D_Roundoff(gl_rec.currel[pa_rec.pa_cur_period]);

		        gl_rec.ytd = gl_rec.ytd + gl_rec.nextcr + gl_rec.nextdb;
		       	gl_rec.ytd=D_Roundoff(gl_rec.ytd);

			gl_rec.nextdb = gl_rec.nextcr = 0;
		  }


	  err = RiteRecord(UPDATE) ;	/* Writing record */
	  if (err == DBH_ERR) return(DBH_ERR) ;
	  gl_rec.reccod++ ;		/* Put re-positions the record */
	
	}  /* end of for */
	err = FundBreak();
	if (err != NOERROR) 
		return(ERROR);
	else 	return(NOERROR); 
}

/****  Each fund must be processed separately *****************************/
FundBreak() 
{
	int 	err;

	ctl_rec.fund = prev_fund ;
	err = get_ctl(&ctl_rec, BROWSE, 0, e_mesg);
	if (err != NOERROR) {
		DispError(e_mesg);
	    	return(err);
	}
	GetTrans(); 

	gl_rec.funds = prev_fund ;
	STRCPY( gl_rec.accno, ctl_rec.p_and_l_acnt);
	gl_rec.reccod = 99 ;
	err = get_gl(&gl_rec, UPDATE, 0, e_mesg) ;
	if (err == ERROR) {
		DispError(e_mesg);
		return(ERROR);
	}
	if(err == UNDEF) {
#ifdef ENGLISH
		DispError("Surplus/Deficit account not in file");
#else
		DispError("Compte surplus/deficit pas dans le dossier");
#endif
		return(ERROR);
	}
	if(err == LOCKED) {
		DispError(e_mesg);
		return(ERROR);
	}
	return(SurplusDeficit());
}

/*	Reading all transactions against the suplus/deficit account for each 
	account.
******************************************************************************/
GetTrans()
{
	int	code;

	tr_item.ti_fund = prev_fund ;
	tr_item.ti_reccod = 99 ;
	STRCPY( tr_item.ti_accno, ctl_rec.p_and_l_acnt );
	tr_item.ti_period = 0;
	tr_item.ti_seq_no = 0;
	tr_item.ti_item_no = 0;
	flg_reset(GLTRAN);
	for(;;) {
#ifndef	ORACLE
		code = get_n_tritem(&tr_item, BROWSE, 1, FORWARD, e_mesg) ;
#else
		code = get_n_tritem(&tr_item, BROWSE, 1, EQUAL, e_mesg) ;
#endif
		if (code < 0) break;
#ifndef	ORACLE
		if (tr_item.ti_fund != prev_fund ||
		    tr_item.ti_reccod != 99 ||
		    strcmp( ctl_rec.p_and_l_acnt, tr_item.ti_accno) != 0) 
			break;
#endif

		periodytd[tr_item.ti_period - 1] += tr_item.ti_amount ;
	}  /* end of for */
	seq_over(GLTRAN);
}

/***  The surplus/deficit is calculated for each period and accumulated for 
	a year to date value.  The values are written to the surplus 
	deficit account and reset for the next fund.
****************************************************************************/
SurplusDeficit()
{
	int 	i, err;
	double	ytd = 0;

	for(i=0;i < pa_rec.pa_no_periods;i++)  {
		gl_rec.currel[i] = periodytd[i] ;
		gl_rec.currel[i] = D_Roundoff(gl_rec.currel[i]) ;
		ytd += periodytd[i] ;
	}	
	gl_rec.ytd = ytd + gl_rec.opbal ;

	gl_rec.ytd = D_Roundoff(gl_rec.ytd) ;

	for(i=0;i < 13;i++)
		periodytd[i] = 0;
	ytd = 0;

	return(RiteRecord(UPDATE));
}

RiteRecord(md)
{
	int err ;

	err = put_gl(&gl_rec, md, e_mesg) ;
	if(err != NOERROR) {
#ifdef ENGLISH
		DispError("ERROR in saving records");
#else
		DispError("ERREUR en conservant les fiches");
#endif
		DispError(e_mesg);  
		roll_back(e_mesg) ;
		return(DBH_ERR) ;
	}

  	if(commit(e_mesg) < 0) {
#ifdef ENGLISH
		DispError("ERROR in saving records");
#else
		DispError("ERREUR en conservant les fiches");
#endif
		DispError(e_mesg);  
		return(DBH_ERR) ;
	}

	return(NOERROR) ;
}

/*--------------------------------------------------------------*/
/* 
*	Purge Journals
*
*	Get one by one Journal File Names from backup directory.
*	Purge only journals which are of earlier dates to Disk
*	Backed up DB's date.
*/

static	int
PurgeJournals()
{
	long	t_date ;
	int	i, fd ;
	char	t_str[50] ;
	char 	c_mesg[80];
#ifdef	MS_DOS
	struct	find_t	next_file ;
#else			/* UNIX or XENIX */
	struct	direct	dir_entry ;		/* Directory entry structure */
#endif

#ifdef	MS_DOS
	/* Make the search file string */
	SW8 = 1 ;			/* Change path to backup path */
	form_f_name("", t_str) ;	/* Data Path for 'backup' dir */
	strcat(t_str, JOURNAL);
	strcat(t_str, "*");
	SW8 = 0 ;
#else		/* UNIX or XENIX */
	/* Open directory file to get all the jrnl files */
	SW8 = 0 ;				/* Set path to current dir */
	form_f_name(BACK_UP, t_str);	/* Directory name of 'backup' */
	if ( (fd = open(t_str, RDMODE)) < 0 ) {
#ifdef	ENGLISH
		printf("ERROR in opening %s directory...  errno: %d\n",t_str,errno);
		printf("No Journal Files Could Be Found..\n");
#else
		printf("ERREUR en ouvrant le repertoire %s ...  errno: %d\n",t_str,errno);
		printf("Aucun dossier de journal a ete trouve..\n");
#endif
		return(ERROR) ;
	}
#endif

#ifdef ENGLISH
	printf("\tPurging Journal Files\n\n");
#else
	printf("\tEfface les dossiers de journal\n\n");
#endif

	SW8 = 1 ;	/* to Prefix the path with 'backup' to journal files */
	for( i = 0 ; ; i++ ) {
		/* get the next journal file */
#ifdef	MS_DOS
		if( i == 0)
		    retval = _dos_findfirst(t_str, _A_NORMAL, &next_file) ;
		else
		    retval = _dos_findnext(&next_file) ;
		if(retval != 0) break ;
		STRCPY(t_str, next_file.name) ;
#else
		if ( read(fd, (char*)&dir_entry, sizeof(struct direct)) <
						sizeof(struct direct) ) break ;
		if(dir_entry.d_ino == 0) continue ;
		if(strncmp(dir_entry.d_name,JOURNAL,strlen(JOURNAL))) continue ;
		STRCPY(t_str, dir_entry.d_name) ;
#endif
		form_f_name(t_str, c_mesg) ;

		unlink(c_mesg) ;		/* As explained below */
	}
	SW8 = 0 ;

#ifndef	MS_DOS		/* UNIX or XENIX */
	close(fd) ;
#endif
	return(NOERROR) ;
}	/* PurgeJournals() */

/*************
	A few reports related to different subsystems have been called by this
	file.  
	These dummy routines are defined to avoid link errors when linked with
	files from other modules for integrating the end of month process(es).
*************/

GetFilename()
{
	return(0);
}
GetOutputon()
{
	return(0);
}
GetCNbrRange()
{
	return(0);
}
GetDateRange()
{
	return(0);
}
GetPoRange()
{
	return(0);
}
DisplayMessage()
{
	return(0);
}
GetResponse()
{
	return(0);
}
GetNbrCopies()
{
	return(0);
}
Confirm()
{
	return(0);
}
HideMessage()
{
	return(0);
}

GetPrinter()
{
	return(0);
}
GetFundRange()
{
	return(0);
}
GetReqRange()
{
	return(0);
}
GetCCRange()
{
	return(0);
}
GetReqStatRange()
{
	return(0);
}
GetItmStatRange()
{
	return(0);
}

/*------------------------------------------------------------*/
/* This routine is necessary to produce an estimated journal entry
   in the G/L for payroll accrual. It calculates how many days 
   left in this month within the next pay week.
/*------------------------------------------------------------*/
static
Accruals()
{
	int	retval;

 	retval = get_pay_param(&pay_param,UPDATE,1,e_mesg);
	if(retval < 0) {
  	    DispError(e_mesg);
	    return(retval);
	}

	if(pay_param.pr_week_date == 0 && pay_param.pr_bi_date == 0)
		return(NOERROR);

	end_date_week = pay_param.pr_week_date;
	end_date_bi = pay_param.pr_bi_date;

#ifdef ENGLISH
	printf("\n\nPrinting Accrued Payroll Listing by Employee");
#else
	printf("\n\nPrinting Accrued Payroll Listing by Employee");
#endif
	retval = PrntRep();
	if(retval < 0)	return(retval);

#ifdef ENGLISH
	printf("\n\nUpdating G/L and Printing Accrued Payroll Journal Summary");
#else
	printf("\n\nUpdating G/L and Printing Accrued Payroll Journal Summary");
#endif

	retval = UpdtGl();
	if(retval < 0)	return(retval);

	pay_param.pr_week_date = 0;
	pay_param.pr_bi_date = 0;

	retval = put_pay_param(&pay_param,UPDATE,1,e_mesg);
	if(retval < 0) {
		DispError(e_mesg);
		roll_back(e_mesg);
		return(retval);
	}

	retval = commit(e_mesg) ;
	if(retval < 0) {
#ifdef ENGLISH
		DispError("ERROR in Saving Records"); 
#else
		DispError("ERREUR en conservant les fiches");
#endif
		DispError(e_mesg);
		roll_back(e_mesg);
		return(retval);
	}
	return(NOERROR);
}	/* Accrual() */

/*------------------------------------------------------------*/
static
CalcAccWeek()
{
	int	i,month,day;
	long	julian,remain;

	pay_param.pr_week_date = end_date_week;

	day = pay_param.pr_week_date%100;
	month = pay_param.pr_week_date/100%100;

	/* Check for leap year	*/
	if(month == 02 && (pay_param.pr_week_date/10000) % 4 == 0){
		d_month[month-1]++;		
	}

	accrual_day = 0;
	
	for(i=day+1;i<=d_month[month-1];i++) {
		pay_param.pr_week_date++;
		julian = days(pay_param.pr_week_date);
		remain = julian % 7;
		if(remain == 6 || remain == 0)	continue;

		accrual_day++;
	}

	return(NOERROR);
}	/* CalcAcc() */
/*------------------------------------------------------------*/
static
CalcAccBi()
{
	int	i,month,day;
	long	julian,remain;

	pay_param.pr_bi_date = end_date_bi;

	day = pay_param.pr_bi_date%100;
	month = pay_param.pr_bi_date/100%100;

	/* Check for leap year	*/
	if(month == 02 && (pay_param.pr_bi_date/10000) % 4 == 0){
		d_month[month-1]++;		
	}

	accrual_day = 0;
	
	for(i=day+1;i<=d_month[month-1];i++) {
		pay_param.pr_bi_date++;
		julian = days(pay_param.pr_bi_date);
		remain = julian % 7;
		if(remain == 6 || remain == 0)	continue;

		accrual_day++;
	}

	return(NOERROR);
}	/* CalcAcc() */
/*-----------------------------------------------------------------------*/ 
static
UpdtGl()
{
	int retval, err;
	char	prev_acct[19], prev_acct2[19];
	double	tot_amt, tot_db;
	short	prev_fund1, prev_fund2;

	grnd_total_dr = 0;
	grnd_total_cr = 0;

	if(InitPrinter()<0) {
		DispError("ERROR -(2) Initializeing the printer");
		return(-1);
	}	
#ifdef ENGLISH
	STRCPY(e_mesg, "Updating GL Master File, PLEASE WAIT");
#else
	STRCPY(e_mesg, "Le GL est en train de faire les mises a jour, ATTENDEZ S.V.P.");
#endif
	DispMesgFld(e_mesg);
	fflush(stdout) ;

	prev_acct[0] = '\0';
	prev_fund1 = 0;

	tot_db = 0;
	item_no1 = 1;
	item_no2 = 1;

	if((retval=PrntHdg(1)) == EXIT)	
		return(retval);

	before_period = pa_rec.pa_cur_period;

	if (before_period == pa_rec.pa_no_periods)
		after_period = 0 ;
	else
		after_period = before_period + 1 ;

	if((retval = WriteTrHdr(pay_param.pr_fund))< 0 ) return(retval);

	tmp_seq_no = tr_hdr.th_seq_no;

	for( ; ; ){
		tot_amt = 0;

		jrh_ent.jrh_fund = prev_fund1;
		strcpy(jrh_ent.jrh_acct,prev_acct);
		flg_reset(JRH_ENT);

		for( ; ; ) {
			err = get_n_jrh_ent(&jrh_ent,BROWSE,2,FORWARD,e_mesg) ;
			if(err == EFL) break;
			if( err < 0) {
				DispError(e_mesg) ;
				return(err) ;
			}

			if(jrh_ent.jrh_type[0] != 'E')
				continue;

			err = GetPPNumb();
			if(err < 0)	return(err);

			if(number_of_days == 5 && end_date_week == 0) {
				continue;
			}

			if(number_of_days == 5 && 
			   jrh_ent.jrh_date != end_date_week)
				continue;

			if(number_of_days == 10 && 
			   jrh_ent.jrh_date != end_date_bi)
				continue;

			if(prev_acct[0] == '\0'){
				strcpy(prev_acct, jrh_ent.jrh_acct);
				prev_fund1 = jrh_ent.jrh_fund;
			}

			if(prev_fund1 != jrh_ent.jrh_fund) {
				break;
			}

			if(strcmp(jrh_ent.jrh_acct,prev_acct) != 0) {
				break;
			}

			tot_amt += D_Roundoff(jrh_ent.jrh_amount * accrual_day /
				number_of_days);

		}
		tot_db += tot_amt;

		strcpy(prev_acct2, jrh_ent.jrh_acct);
		prev_fund2 = jrh_ent.jrh_fund;
		if((retval = WriteGlmast(prev_fund1,
			prev_acct,tot_amt,before_period))< 0 ) return(retval);

		if((retval = WriteTrItems1(prev_fund1,
				prev_acct,tot_amt))< 0 ) return(retval);

		tot_amt = tot_amt * -1;

		if((retval = WriteGlmast(prev_fund1,
			prev_acct,tot_amt,after_period))< 0 ) return(retval);

		if((retval = WriteTrItems2(prev_fund1,
				prev_acct,tot_amt))< 0 ) return(retval);

		tot_amt = tot_amt * -1;

		retval = PrntRec2(prev_acct,tot_amt,0.00);
		if(retval < 0)	return(retval);

		item_no1++;
		item_no2++;

		if(err == EFL)  break;

		strcpy(prev_acct, prev_acct2);
		prev_fund1 = prev_fund2;
	}
	seq_over(JRH_ENT);

	tot_db = tot_db * -1;

	if((retval = WriteGlmast(prev_fund1,
		"          21201099",tot_db,before_period))< 0 ) return(retval);

	if((retval = WriteTrItems1(1,
		"          21201099",tot_db))< 0 ) return(retval);

	tot_db = tot_db * -1;

	if((retval = WriteGlmast(prev_fund1,
		"          21201099",tot_db,after_period))< 0 ) return(retval);

	if((retval = WriteTrItems2(1,
		"          21201099",tot_db))< 0 ) return(retval);

	retval = PrntRec2("          21201099",0.00,tot_db);
	if(retval < 0)	return(retval);

	err = PrntTot2();
	if(err < 0)	return(err);


	tr_hdr.th_fund = pay_param.pr_fund;
	tr_hdr.th_reccod = 99;
	tr_hdr.th_create[0] = 'G';
	tr_hdr.th_seq_no = tmp_seq_no;

	retval = get_trhdr( &tr_hdr, UPDATE, 0, e_mesg );
	if( retval==ERROR ){
		DispError(e_mesg);
		return(-1);
	}

	tr_hdr.th_debits = tr_hdr.th_credits = tot_db;

	/*  Roundoff double items that have calculated values  */
	tr_hdr.th_debits 	= D_Roundoff(tr_hdr.th_debits);
	tr_hdr.th_credits 	= D_Roundoff(tr_hdr.th_credits);

	retval = put_trhdr( &tr_hdr, UPDATE, e_mesg );
	if(retval<0) {
		DispError(e_mesg);
		roll_back(e_mesg);
		return(retval);
	}
	if( commit(e_mesg)<0 ){
		DispError(e_mesg);
		return(-1);
	}

	tr_hdr.th_fund = pay_param.pr_fund;
	tr_hdr.th_reccod = 99;
	tr_hdr.th_create[0] = 'G';
	tr_hdr.th_seq_no = tmp_seq_no-1;

	retval = get_trhdr( &tr_hdr, UPDATE, 0, e_mesg );
	if( retval==ERROR ){
		DispError(e_mesg);
		return(-1);
	}

	tr_hdr.th_debits = tr_hdr.th_credits = tot_db;

	/*  Roundoff double items that have calculated values  */
	tr_hdr.th_debits 	= D_Roundoff(tr_hdr.th_debits);
	tr_hdr.th_credits 	= D_Roundoff(tr_hdr.th_credits);

	retval = put_trhdr( &tr_hdr, UPDATE, e_mesg );
	if(retval<0) {
		DispError(e_mesg);
		roll_back(e_mesg);
		return(retval);
	}
	if( commit(e_mesg)<0 ){
		DispError(e_mesg);
		return(-1);
	}

	close_rep();

	return(NOERROR);
}
/*-----------------------------------------------------------------------*/ 
static
WriteTrHdr(fund)
short	fund;
{
	int	retval;

#ifdef ORACLE
	long	sno, get_maxsno();
#endif
	tr_hdr.th_fund = fund;
	tr_hdr.th_reccod = 99;
	tr_hdr.th_create[0] = 'G';

#ifndef ORACLE
	tr_hdr.th_seq_no = HV_SHORT;
	retval = get_n_trhdr( &tr_hdr, BROWSE, 0, BACKWARD, e_mesg );
	seq_over( GLTRHDR );
	if( retval==ERROR ){
		DispError(e_mesg);
		roll_back(e_mesg);
		return(-1);
	}
	if( retval==EFL || 
	    tr_hdr.th_fund != fund ||	
	    tr_hdr.th_reccod != 99 || tr_hdr.th_create[0] != 'G' ){
		tr_hdr.th_fund = fund;
		tr_hdr.th_reccod = 99;
		tr_hdr.th_create[0] = 'G';
		tr_hdr.th_seq_no = 1;
	}
	else
		tr_hdr.th_seq_no++;
#else
	sno = get_maxsno(GLTRHDR,(char *)&tr_hdr,0,-1,e_mesg);
	if(sno < 0) {
		DispError(e_mesg);
		roll_back(e_mesg);
		return(-1);
	}
	tr_hdr.th_seq_no = sno + 1;
#endif

	strcpy( tr_hdr.th_userid, User_Id );
	tr_hdr.th_sys_dt = get_date() ;
	tr_hdr.th_period = before_period;
	tr_hdr.th_date = get_date();
	tr_hdr.th_debits = tr_hdr.th_credits = 0.00;

	/*  Roundoff double items that have calculated values  */
	tr_hdr.th_debits 	= D_Roundoff(tr_hdr.th_debits);
	tr_hdr.th_credits 	= D_Roundoff(tr_hdr.th_credits);

	strcpy(tr_hdr.th_descr, "PAYROLL ACCRUALS");
	tr_hdr.th_supp_cd[0] = '\0';
	tr_hdr.th_type[0] = 'P';
	tr_hdr.th_print[0] = 'N';
	strcpy(tr_hdr.th_reference, "PAYROLL");

	retval = put_trhdr( &tr_hdr, ADD, e_mesg );
	if(retval<0) {
		DispError(e_mesg);
		roll_back(e_mesg);
		return(retval);
	}

	tr_hdr.th_seq_no++;
	tr_hdr.th_period = after_period;

	retval = put_trhdr( &tr_hdr, ADD, e_mesg );
	if(retval<0) {
		DispError(e_mesg);
		roll_back(e_mesg);
		return(retval);
	}

	if( commit(e_mesg)<0 ){
		DispError(e_mesg);
		return(-1);
	}
	return(0);
}
/*-----------------------------------------------------------------------*/ 
static
WriteTrItems1(fund, acct, amount)
short	fund;
char	*acct;
double	amount;
{

	tr_item.ti_fund = fund;
	tr_item.ti_reccod = 99;
	tr_item.ti_create[0] = 'G';
	tr_item.ti_seq_no = tr_hdr.th_seq_no-1;
	tr_item.ti_item_no = item_no1;
	tr_item.ti_sys_dt = tr_hdr.th_sys_dt;
	tr_item.ti_period = before_period;
	strcpy(tr_item.ti_accno,acct);
	tr_item.ti_section = gl_rec.sect;
	tr_item.ti_amount = amount;
	tr_item.ti_status = 0;

	/* Roundoff ti_amount which is double	*/
	tr_item.ti_amount = D_Roundoff(tr_item.ti_amount);

	if( put_tritem(&tr_item, ADD, e_mesg)<0 ){
		DispError(e_mesg);
		roll_back(e_mesg);
		return(-1);
	}
	if( commit(e_mesg)<0 ){
		DispError(e_mesg);
		return(-1);
	}

	return(NOERROR);
}
/*-----------------------------------------------------------------------*/ 
static
WriteTrItems2(fund, acct, amount)
short	fund;
char	*acct;
double	amount;
{

	tr_item.ti_fund = fund;
	tr_item.ti_reccod = 99;
	tr_item.ti_create[0] = 'G';
	tr_item.ti_seq_no = tr_hdr.th_seq_no;
	tr_item.ti_item_no = item_no2;
	tr_item.ti_sys_dt = tr_hdr.th_sys_dt;
	tr_item.ti_period = after_period;
	strcpy(tr_item.ti_accno,acct);
	tr_item.ti_section = gl_rec.sect;
	tr_item.ti_amount = amount;
	tr_item.ti_status = 0;

	/* Roundoff ti_amount which is double	*/
	tr_item.ti_amount = D_Roundoff(tr_item.ti_amount);

	if( put_tritem(&tr_item, ADD, e_mesg)<0 ){
		DispError(e_mesg);
		roll_back(e_mesg);
		return(-1);
	}
	if( commit(e_mesg)<0 ){
		DispError(e_mesg);
		return(-1);
	}

	return(NOERROR);
}
/*-----------------------------------------------------------------------*/ 
static
WriteGlmast(fund, acct, amount,period)
short	fund;
char	*acct;
double	amount;
short	period;
{
	int	retval;

	gl_rec.funds = fund;
	strcpy(gl_rec.accno, acct);
	gl_rec.reccod = 99;
	retval = get_gl( &gl_rec, UPDATE, 0, e_mesg );
	if( retval!=NOERROR ){
		DispError(e_mesg);
		roll_back(e_mesg);
		return(retval);
	}
	gl_rec.currel[period-1] += amount; 
	gl_rec.ytd += amount;
	if(amount > 0.00)
		gl_rec.curdb += amount;
	else
		gl_rec.curcr += amount;

	gl_rec.currel[period-1] =
			 D_Roundoff(gl_rec.currel[period-1]);
	gl_rec.ytd = D_Roundoff(gl_rec.ytd);
	gl_rec.curdb = D_Roundoff(gl_rec.curdb);
	gl_rec.curcr = D_Roundoff(gl_rec.curcr);

	retval = put_gl( &gl_rec, UPDATE, e_mesg );
	if( retval!=NOERROR ){
		DispError(e_mesg);
		roll_back(e_mesg);
		return(retval);
	}
	if( commit(e_mesg)<0 ){
		DispError(e_mesg);
		return(-1);
	}

	return(NOERROR);

}
/*-----------------------------------------------------------------*/
static
PrntRep()
{
	int	retval,err,first_time;
	char	prev_emp[13];
	char	prev_acct[19];

	grnd_total = 0;

	if(InitPrinter()<0) {
		printf("\nERROR - Initializeing the printer");
		return(-1);
	}	

	first_time = 0;

	prev_emp[0] = '\0';

	jrh_ent.jrh_emp_numb[0] = '\0';
	jrh_ent.jrh_date = 0;
	jrh_ent.jrh_fund = 0;
	jrh_ent.jrh_acct[0] = '\0';
	flg_reset(JRH_ENT);

	for( ; ; ) {
		retval = get_n_jrh_ent(&jrh_ent,BROWSE,1,FORWARD,e_mesg) ;
		if(retval == EFL) break;
		if( retval < 0) {
			DispError(e_mesg);
			return(retval) ;
		}
		if(strcmp(prev_emp,jrh_ent.jrh_emp_numb) != 0){
			strcpy(prev_emp,jrh_ent.jrh_emp_numb);

			err = GetPPNumb();
			if(err < 0){
				return(err);
			}
		}

		if(number_of_days == 5 && end_date_week == 0) {
			inc_str(jrh_ent.jrh_emp_numb,
				sizeof(jrh_ent.jrh_emp_numb)-1,FORWARD);
			jrh_ent.jrh_date = 0;
			jrh_ent.jrh_fund = 0;
			jrh_ent.jrh_acct[0] = '\0';

			flg_reset(JRH_ENT);

			continue;
		}

		if(jrh_ent.jrh_type[0] != 'E')
			continue;

		if(number_of_days == 5 && 
		   jrh_ent.jrh_date != end_date_week)
			continue;

		if(number_of_days == 10 && 
		   jrh_ent.jrh_date != end_date_bi)
			continue;

		if(first_time == 0) {
			if((retval=PrntHdg(0)) == EXIT)	
				return(retval);
			first_time = 1;
		}

		err = PrntRec1();
		if(err < 0){
			return(err);
		}
	}

	err = PrntTot1();
	if(err < 0){
		return(err);
	}

	close_rep();
	return(NOERROR);

}
/*-----------------------------------------------------------------*/
static
GetPPNumb()
{
	int	err;
	strcpy(emp_rec.em_numb, jrh_ent.jrh_emp_numb);

	err = get_employee(&emp_rec,BROWSE,0,e_mesg);
	if(err != NOERROR){
  		DispError(e_mesg) ;
 		return(ERROR);
  	}

	strcpy(barg_unit.b_code,emp_rec.em_barg);
	barg_unit.b_date = get_date();
	flg_reset(BARG);

	err = get_n_barg(&barg_unit,BROWSE,0,BACKWARD,e_mesg);
	if(err == EFL ||
	     strcmp(barg_unit.b_code, emp_rec.em_barg) != 0){
  		DispError(e_mesg);
		return(ERROR);
	}
	if(err < 0){
  		DispError(e_mesg);
  		return(ERROR);
	}

	strcpy(pay_period.pp_code,barg_unit.b_pp_code);
	pay_period.pp_year = end_date_bi/10000;

	err = get_pay_per(&pay_period,BROWSE,0,e_mesg);
	if(err < 0){ 
  		DispError(e_mesg);
  		return(ERROR);
	}
	if(pay_period.pp_numb == 52) {
		number_of_days = 5;
		err = CalcAccWeek();
		if(err < 0)	return(err);
	}
	else {
		number_of_days = 10;
		err = CalcAccBi();
		if(err < 0)	return(err);
	}

	return(NOERROR);
}
/*-----------------------------------------------------------------*/
static
InitPrinter()
{
	char	resp[2] ;
	char	discfile[15] ;

	/* Always to Printer */
	STRCPY(resp,"F");
	discfile[0]= '\0';
	PG_SIZE = 60;

	if( opn_prnt( resp, "lou.dat", 1, e_mesg, 1 /* spool */)<0 ){
		return(REPORT_ERR);
	}
	pgcnt = 0;		/* Page count is zero */
	LNSZ = 132;		/* line size in no. of chars */
	linecnt = PG_SIZE;	/* Page size in no. of lines */

	return(NOERROR) ;
}
/******************************************************************************
Prints the headings
******************************************************************************/
static
PrntHdg(report)	/* Print heading  */
short	report;
{
	long	sysdt ;
	int	offset;

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
	if(prnt_line() < 0 )	return(REPORT_ERR);
 
	offset = ( LNSZ-strlen(pa_rec.pa_co_name) )/2;
	mkln( offset, pa_rec.pa_co_name, strlen(pa_rec.pa_co_name) );
	if( prnt_line()<0 )	return(REPORT_ERR);

	if(report == 0) {
#ifdef ENGLISH
		mkln((LNSZ-24)/2,"ACCRUED PAYROLL LISTING", 24 );
#else
		mkln((LNSZ-24)/2,"TRANSLATE        ", 24 );
#endif
	}
	else {
#ifdef ENGLISH
		mkln((LNSZ-31)/2,"ACCRUED PAYROLL JOURNAL SUMMARY", 31 );
#else
		mkln((LNSZ-31)/2,"TRANSLATE        ", 24 );
#endif
	}
	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	if(report == 0) {
		mkln(3,"EMPLOYEE",8);
		mkln(14,"EMPLOYEE",8);
		mkln(48,"G/L",3);
		mkln(66,"NUMBER OF",9);
		mkln(79,"ACCRUED",7);
	}
	else {
		mkln(3,"G/L ACCOUNT",11);
		mkln(22,"G/L ACCOUNT",11);
		mkln(74,"DEBIT",5);
		mkln(92,"CREDIT",6);
	}	

	if(prnt_line() < 0 )	return(REPORT_ERR);

	if(report == 0) {
		mkln(4,"NUMBER",6);
		mkln(18,"NAME",4);
		mkln(46,"ACCOUNT",7);
		mkln(64,"DAYS ACCRUED",12);
		mkln(82,"PAYROLL",7);
	}
	else {
		mkln(5,"NUMBER",6);
		mkln(24,"NAME",4);
	}

	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	return(NOERROR);
}
/****************************************************************************/
static
PrntRec1()
{
	char	txt_line[132];
	int	retval;
	double	total;

	mkln(1,emp_rec.em_numb,12);
	sprintf(txt_line,"%s, %s",
		emp_rec.em_last_name,
		emp_rec.em_first_name);
	mkln(15,txt_line,22);
	mkln(38,jrh_ent.jrh_acct,18);

	tedit((char*)&accrual_day,"0_",txt_line,R_SHORT);
	mkln(68,txt_line,13);

	total = D_Roundoff(jrh_ent.jrh_amount * accrual_day / number_of_days);

	tedit((char*)&total,"___,_0_.__-",txt_line,R_DOUBLE);
	mkln(80,txt_line,11);

	if(prnt_line() < 0 )	return(REPORT_ERR);
			
	if(linecnt > PG_SIZE) {
		if((retval=PrntHdg(0)) == EXIT)	
			return(retval);
	}

	grnd_total += total;

	return(NOERROR);
}
/****************************************************************************/
static
PrntRec2(acct,debit,credit)
char	*acct;
double	debit;
double	credit;
{
	char	txt_line[132];
	int	retval;
	double	total;

	mkln(1,acct,18);
	mkln(22,gl_rec.desc,48);

	tedit((char*)&debit,"___,_0_.__-",txt_line,R_DOUBLE);
	mkln(74,txt_line,11);

	tedit((char*)&credit,"___,_0_.__-",txt_line,R_DOUBLE);
	mkln(92,txt_line,11);

	if(prnt_line() < 0 )	return(REPORT_ERR);
			
	if(linecnt > PG_SIZE) {
		if((retval=PrntHdg(1)) == EXIT)	
			return(retval);
	}

	grnd_total_dr += debit;
	grnd_total_cr += credit;

	return(NOERROR);
}
/*--------------------------------------------------------------------*/
static
PrntTot1()
{
	char	txt_line[132];

	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(1,"TOTAL",5); 

	tedit((char*)&grnd_total,"___,_0_.__",txt_line,R_DOUBLE);
	mkln(80,txt_line,10);

	if(prnt_line() < 0 )	return(REPORT_ERR);

	return(NOERROR);
}
/*--------------------------------------------------------------------*/
static
PrntTot2()
{
	char	txt_line[132];

	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(1,"TOTAL",5); 

	tedit((char*)&grnd_total_dr,"___,_0_.__",txt_line,R_DOUBLE);
	mkln(74,txt_line,10);

	tedit((char*)&grnd_total_cr,"___,_0_.__",txt_line,R_DOUBLE);
	mkln(92,txt_line,10);

	if(prnt_line() < 0 )	return(REPORT_ERR);

	return(NOERROR);
}

/******************************************************************************
		Created on    : April 95
		Created  By   : Louis Robichaud
*******************************************************************************
About this modual:
	This routine is called from the monthend program and its function is
to purge any payadvances that have been processed over 30 days old.
It will read the entire man_cheque file and check two dates to see if they
are both over 30 days old.

HISTORY:
 Date           Programmer     Description of modification
____/__/__      __________     ___________________________
*******************************************************************************/

/*-----------------------------------------------------------------------*/
padvpurge()
{
	int	retval;
	long 	expire_date;

	/* Inialize values for report */
	InitPrinter();

	sysdate = get_date();

	/* Get the date for 31 days old */
	expire_date = date_plus(sysdate, -31);

	man_chq.mc_emp_numb[0] = '\0';
	man_chq.mc_date = 0;
	man_chq.mc_chq_numb = 0;

	flg_reset(MAN_CHQ);

	for(;;){
		/* increment lowest part of key */
		man_chq.mc_chq_numb ++;

		retval = get_n_man_chq(&man_chq,BROWSE,0,FORWARD,e_mesg);
		if(retval == EFL) break;

		if(retval < 0) {
			DispError("Error on man cheque file");
			DispError(e_mesg);
			return(retval);
		}

		retval = CheckDates(expire_date);
		if(retval<0) return(retval);

		seq_over(MAN_CHQ);
	}

	close_rep();
	return(NOERROR);

}
CheckDates(expire_date)
	long expire_date;
{
	int retval ;

	/* If the two dates don't exceed the expire date then return */
	if(man_chq.mc_date > expire_date || man_chq.mc_chq_numb > expire_date)
		return(NOERROR);

	strcpy(emp_rec.em_numb, man_chq.mc_emp_numb);

	retval = get_employee(&emp_rec,BROWSE,0,e_mesg);
	/* employee must exist for each advance. */
	if(retval < 0){
		printf("\n\ncould not match employee to advance %s", 
			emp_rec.em_numb);
		DispError(e_mesg);
		return(retval);
	}

	/* Print the entry to a report and Delete the advance. */

	retval = PrintAdv();
	if(retval < 0) return(retval);

	retval = DelAdv();
	if(retval < 0) return(retval);

	close_file(EMPLOYEE);
	return(NOERROR);
}
/***********************************************************
Print the header at the top of a page
************************************************************/
PrintHeader()
{
	char	txt_buff[132];
	short 	offset;

	if( pgcnt ) { /* if not the first page */
		if( rite_top()<0 ) return( -1 );	/* form_feed */
	}

	linecnt = 0;

	mkln( 1, PROG_NAME, 10); 

	offset = ( LNSZ-strlen(pa_rec.pa_co_name) )/2;
	mkln( offset, pa_rec.pa_co_name, strlen(pa_rec.pa_co_name) );

	mkln( 103, "DATE: ", 6 );
	tedit( (char *)&sysdate,"____/__/__",  line+cur_pos,R_LONG ); 
	cur_pos += 10;

	pgcnt++;
	mkln(122, "Page:", 5);
	tedit( (char *)&pgcnt,"__0_",  txt_buff, R_SHORT ); 
	mkln(127,txt_buff, 10);

	if( prnt_line()<0 )	return(-1);

	mkln((LNSZ-25)/2,"PURGED PAY ADVANCE REPORT",25);
	if( prnt_line()<0 )	return(-1);
	if( prnt_line()<0 )	return(-1);

	mkln(1,"EMPLOYEE",8);
	mkln(20,"EMPLOYEE NAME",13);
	mkln(58,"ISSUE",5);
	mkln(70,"ADJUSTMENT",10);
	mkln(82,"ADVANCE",7);
	mkln(94,"DEDUCTION", 9);
	if( prnt_line()<0 )	return(-1);
	mkln(2,"NUMBER",6);
	mkln(58,"DATE",4);
	mkln(73,"DATE",4);
	mkln(82,"AMOUNT",6);
	mkln(96,"CODE",4);
	if( prnt_line()<0 )	return(-1);
	if( prnt_line()<0 )	return(-1);
	if( prnt_line()<0 )	return(-1);

	return(NOERROR);

}

/***
Procedure that prints the advance that is about to be deleted. This acts as
an audit report.
***/
PrintAdv()
{
	char	txt_buff[132];


	/* Check for page break */
	if(linecnt >= PG_SIZE)
		PrintHeader();

	/* Employee number */
	mkln(1,man_chq.mc_emp_numb,12);

	/* Employee name */
/********
	sprintf(txt_buff,"%s, %s",emp_rec.em_last_name,
				emp_rec.em_first_name);
	mkln(14,txt_buff,strlen(txt_buff));
*******/

	/* Issue date */
	tedit( (char *)&man_chq.mc_date,"____/__/__",  txt_buff, R_LONG ); 
	mkln(58,txt_buff, 10);

	/* Adjustment date */
	tedit( (char *)&man_chq.mc_chq_numb,"____/__/__",  txt_buff, R_LONG ); 
	mkln(70,txt_buff,10);

	/* Amount */
	tedit((char *)&man_chq.mc_amount,"__,_0_.__-",txt_buff,R_DOUBLE);
	mkln(82,txt_buff,10);
	
	/* Deduction Code */
	mkln(94,man_chq.mc_ded_code,6);

	if( prnt_line()<0 )	return(-1);

	return(NOERROR);
}

/****
Procedure to delete advances from the man_chq file.
****/
DelAdv()
{
	int	retval;

	retval = get_man_chq(&man_chq,UPDATE,0,e_mesg);
	if(retval != NOERROR) {
		DispError(e_mesg);
		return(retval);
	}

	retval = put_man_chq(&man_chq,P_DEL,e_mesg);
	if(retval != NOERROR) {
		DispError(e_mesg);
		return(retval);
	}

	retval = commit(e_mesg);
	if(retval != NOERROR) {
		DispError(e_mesg);
		return(retval);
	}
	return(NOERROR);
}
/*******************************************************
 This function is used internaly to display errors in processing. There is 
no screen for this program so printf must be used to display messages.
*********************************************************/
static
DispError(msg)
char *msg;
{
	printf("\n\n%s",msg);
	printf("\nPress - RETURN to continue");
	getchar();
}
/*-------------------- E n d   O f   P r o g r a m ---------------------*/
