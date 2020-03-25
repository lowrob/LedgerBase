/* --------------------------------------------------------------------------
	SOURCE NAME:  SLABELS.C
	SYSTEM     :  BUGETARY FINANCIAL SYSTEM 
	MODULE     :  PURCHASE ORDER MODULE
	CREATED ON :  11 OCT. 1989
	CREATED BY :  Jonathan Prescott

DESCRIPTION:
	This program prints a range of specified supplier 
        mailing labels, sorted on supplier number or abbrev. name.
	It allows for a maximum of 4 labels across.

MODIFICATIONS:

PROGRAMMER           YY/MM/DD        DESCRIPTION OF MODIFICATIONS
^^^^^^^^^^           ^^^^^^^^        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
C.Leadbeater	     90/12/07	     Changed so blank supplier address lines
				     are not printed.

L.Robichaud	1993/10/13	Pass new parameter to close_rep function for 
				a banner to print with 3000 family.
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <repdef.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#ifdef ENGLISH
#define YES	'Y'
#else
#define YES	'O'
#endif

#define  EXIT  12
#define  BYNO   0
#define  BYNAME 1

Pa_rec   pa_rec;             /*  Declarations for DBH record reading */
Supplier supp_rec;

char 	 e_mesg[80];
int 	 retval;
short    cnt;
short    nbr_labels;
short	 copies;
int	 total_labels;
short	 printer;
char	 s_no1[11], s_no2[11];
char	 s_name1[25], s_name2[25];
char     program[11];
char	 answer[2];
int	 keyno;
int	 code;

struct labels {
	char sname[31];
	char sadd1[31];
	char sadd3[31];
	char spc[11];
}labels[5];

SLabels(mode)
int mode;
{
	int field;

	LNSZ = 132;
	PGSIZE = 60;
	total_labels = 0;
	cnt = 0;

	STRCPY(program,"SLABELS");
	printer = 1;
	/*copies = 1;*/
	GetPrinter( &printer );
	if((retval = GetNbrCopies( &copies )) < 0)
		return(retval);
	nbr_labels = 0;
	for( ; nbr_labels > 3 || nbr_labels == 0; ) {
		nbr_labels = 2;
		GetNbrup( &nbr_labels);
		if(nbr_labels > 3) {
#ifdef ENGLISH
			fomer("No more than 3 Labels across can be printed");
#else
			fomer("Ne peut pas imprimer plus de 4 etiquettes de large");
#endif
		}
		if(nbr_labels == 0) {
#ifdef ENGLISH
			fomer("Number of labels across can't be 0");
#else
			fomer("Nombre d'etiquettes en largeur ne peut pas etre 0");
#endif
		}
	}
	if(mode == BYNO) {
		STRCPY(s_no1,"         0");
		STRCPY(s_no2,"ZZZZZZZZZZ");
		retval = GetSuppRange(s_no1,s_no2);
		if(retval < 0) return(-1);
		else	if(retval == EXIT) return(0);
		keyno = 0;
	}
	else {
		STRCPY(s_name1,"                        ");
		STRCPY(s_name2,"zzzzzzzzzzzzzzzzzzzzzzzz");
		retval = GetSNameRange(s_name1,s_name2);
		if(retval < 0) return(-1);
		else	if(retval == EXIT) return(0);
		keyno = 2;
	}
	if(( retval = Confirm()) <= 0) return(retval);

	retval = get_param(&pa_rec,BROWSE,1,e_mesg);
        if(retval < 1) {
		fomer(e_mesg);
		close_dbh();
		return(-1);
	}

	/* always to printer */
	retval = opn_prnt("P",'\0',printer,e_mesg,0);  
	if(retval < 0) {
		fomer( e_mesg) ;
		close_dbh() ;
		return(-1);
	}
	SetCopies( (int)copies );
	if(keyno == 0 ) {
		STRCPY(supp_rec.s_supp_cd,s_no1);
	}
	else {
		STRCPY(supp_rec.s_abb,s_name1);
	}
	flg_reset( SUPPLIER );
        for( ; ; ) {
		code = get_n_supplier(&supp_rec,BROWSE,keyno,FORWARD,e_mesg);
		if(code < 0) {
			if(code == EFL) break;
			fomer(e_mesg);
			break;
		}
		if(mode == BYNO) {
			if(strcmp(supp_rec.s_supp_cd,s_no2) > 0) break;
		}	
		else {
			if(strcmp(supp_rec.s_abb,s_name2) > 0) break;
		}

	/***  setup individual labels to print, check each supplier 
	      address line for NULL so blank lines do not appear on
              label. (CL) 						 ***/

		STRCPY(labels[cnt].sname,supp_rec.s_name);

			/* address label line 1 */ 

		if (strcmp(supp_rec.s_add1,"\0")){
			STRCPY(labels[cnt].sadd1,supp_rec.s_add1);
			supp_rec.s_add1[0]='\0';
		}
		else
		if (strcmp(supp_rec.s_add3,"\0")){
			STRCPY(labels[cnt].sadd1,supp_rec.s_add3);
			supp_rec.s_add3[0]='\0';
		}
		else
		if (strcmp(supp_rec.s_pc,"\0")){
			STRCPY(labels[cnt].sadd1,supp_rec.s_pc);
			supp_rec.s_pc[0]='\0';
		}
		else
			STRCPY(labels[cnt].sadd1,"     ");
	

			/* address label line 2 */ 

		if (strcmp(supp_rec.s_add1,"\0")){
			STRCPY(labels[cnt].sadd3,supp_rec.s_add1);
			supp_rec.s_add1[0]='\0';
		}
		else
		if (strcmp(supp_rec.s_add3,"\0")){
			STRCPY(labels[cnt].sadd3,supp_rec.s_add3);
			supp_rec.s_add3[0]='\0';
		}
		else
		if (strcmp(supp_rec.s_pc,"\0")){
			STRCPY(labels[cnt].sadd3,supp_rec.s_pc);
			supp_rec.s_pc[0]='\0';
		}
		else
			STRCPY(labels[cnt].sadd3,"     ");
	

			/* address label line 3 */ 

		if (strcmp(supp_rec.s_add1,"\0")){
			STRCPY(labels[cnt].spc,supp_rec.s_add1);
			supp_rec.s_add1[0]='\0';
		}
		else
		if (strcmp(supp_rec.s_add3,"\0")){
			STRCPY(labels[cnt].spc,supp_rec.s_add3);
			supp_rec.s_add3[0]='\0';
		}
		else
		if (strcmp(supp_rec.s_pc,"\0")){
			STRCPY(labels[cnt].spc,supp_rec.s_pc);
			supp_rec.s_pc[0]='\0';
		}
		else
			STRCPY(labels[cnt].spc,"     ");
	
		cnt++;
		if(cnt == nbr_labels) {
			print_labels();
/**
			if(linecnt >= PGSIZE) rite_top();
**/
		}
	}
	if(cnt != 0){
		print_labels();
	}
	prnt_line();
	tedit((char *)&total_labels,"__,_0_",e_mesg,R_INT);
#ifdef ENGLISH
	mkln(1,"Total Labels Printed: ",22);
	mkln(23,e_mesg,6);
#else
	mkln(1,"Total d'etiquettes imprimees: ",30);
	mkln(31,e_mesg,6);
#endif
	prnt_line();
	rite_top();
	total_labels = 0;
	cnt = 0;

	close_rep(NOBANNER);
	close_dbh();
	return(0);
}
/*---------------------------------------------------------------------------
	Description:  this function sets up the number of labels to print
		      across on a page and then prints our the labels.
---------------------------------------------------------------------------*/
print_labels()
{
	int	i;

	prnt_line();
	for(i = 0; i < cnt;i++) {
		mkln(1 +(41 * i),labels[i].sname,40);
	}
	prnt_line();
	 for(i = 0; i < cnt;i++) {
		mkln(1 +(41 * i),labels[i].sadd1,30);
	}
	prnt_line();
	for(i = 0; i < cnt;i++) {
		mkln(1 +(41 * i),labels[i].sadd3,30);
	}
	prnt_line();
	for(i = 0; i < cnt;i++) {
		mkln(1 +(41 * i),labels[i].spc,7);
	}
	prnt_line();
	prnt_line();
	prnt_line();
	prnt_line();
	prnt_line();
	total_labels += cnt;
	cnt = 0;
} 

/*-------------------------- END OF FILE ---------------------------------- */

