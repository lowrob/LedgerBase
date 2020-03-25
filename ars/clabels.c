/* --------------------------------------------------------------------------
	SOURCE NAME:  CLABELS.C
	SYSTEM     :  BUGETARY FINANCIAL SYSTEM 
	MODULE     :  ACCOUNTS RECEIVABLE MODULE
	CREATED ON :  22 NOV. 1989
	CREATED BY :  Jonathan Prescott

DESCRIPTION:
	This program prints a range of specified customer 
        mailing labels, sorted on customer number or name.
	It allows for a maximum of 3 labels across.

MODIFICATIONS:

PROGRAMMER           YY/MM/DD        DESCRIPTION OF MODIFICATIONS
^^^^^^^^^^           ^^^^^^^^        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
C.Leadbeater	     90/12/07	     Change so that blank customer address
				     lines are not printed.

L.Robichaud	1993/10/13	Pass new parameter to close_rep function for 
				a banner to print with 3000 family.
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <repdef.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define  EXIT  12
#define  BYNO   0
#define  BYNAME 1
Pa_rec   pa_rec;             /*  Declarations for DBH record reading */
Cu_rec 	 cu_rec;

int 	 retval;
short    cnt;
short    nbr_labels, i;
int	 total_labels;
short	 printer;
char	 s_no1[11], s_no2[11];
char	 s_name1[31], s_name2[31];
char     program[11];
int	 keyno;
int	 code;

struct labels {
	char sname[31];
	char sadd1[31];
	char sadd3[31];
	char spc[8];
}labels[5];

static	short	copies;
extern char  e_mesg[80];

CLabels(mode)
int mode;
{
	LNSZ = 132;
	PGSIZE = 60;
	total_labels = 0;
	cnt = 0;

	STRCPY(program,PROG_NAME);

	copies = 1;
	printer = 1;
	if((retval = GetPrinter( &printer ))<0)
		return(retval);
	if((retval = GetNbrCopies( &copies ))<0)
		return(retval);
	nbr_labels = 0;
	for( ; nbr_labels > 4 || nbr_labels == 0; ) {
		nbr_labels = 2;
		if((retval = GetNbrup( &nbr_labels))<0)
			return(retval);
		if(nbr_labels > 3) {
#ifdef ENGLISH
			fomer("No more than 3 Labels across can be printed");
#else
			fomer("Pas plus de 3 etiquettes en largeur peuvent etre imprimees");
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
		STRCPY(s_no1,"         1");
		STRCPY(s_no2,"ZZZZZZZZZZ");
		retval = GetCNbrRange(s_no1,s_no2);
		if(retval < 0) return(retval);
		keyno = 0;
	}
	else {
		STRCPY(s_name1,"");
		STRCPY(s_name2,"zzzzzzzzzzzzzzzzzzzz");
		retval = GetCNameRange(s_name1,s_name2);
		if(retval < 0) return(retval);
		keyno = 1;
	}
	if(( retval = Confirm()) < 0) return(retval);
	else	if(!retval) return(0);

	retval = get_param(&pa_rec,BROWSE,1,e_mesg);
        if(retval < 1) {
		return(DBH_ERR);
	}

	/* must be "P" to open printer */
	retval = opn_prnt("P",'\0',printer,e_mesg,0);  
	if(retval < 0) {
		return(REPORT_ERR);
	}
	SetCopies( (int)copies );
	if(keyno == 0 ) {
		STRCPY(cu_rec.cu_code,s_no1);
	}
	else {
		STRCPY(cu_rec.cu_name,s_name1);
	}
	flg_reset( CUSTOMER );
        for( ; ; ) {
		code = get_n_cust(&cu_rec,BROWSE,keyno,FORWARD,e_mesg);
		if(code < 0) 
			break;

		if(mode == BYNO) {
			if(strcmp(cu_rec.cu_code,s_no2) > 0) break;
		}	
		else {
			if(strcmp(cu_rec.cu_name,s_name2) > 0) break;
		}

	/***  setup individual labels to print, make sure that no
	      blank address lines are printed by checking for NULL. (CL) ***/

		STRCPY(labels[cnt].sname,cu_rec.cu_name);

			/* label address line 1 */ 

		if (strcmp(cu_rec.cu_adr1,"\0")){
			STRCPY(labels[cnt].sadd1,cu_rec.cu_adr1);
			cu_rec.cu_adr1[0]='\0';
		}
		else
		if (strcmp(cu_rec.cu_adr3,"\0")){
			STRCPY(labels[cnt].sadd1,cu_rec.cu_adr3);
			cu_rec.cu_adr3[0]='\0';
		}
		else
		if (strcmp(cu_rec.cu_pc,"\0")){
			STRCPY(labels[cnt].sadd1,cu_rec.cu_pc);
			cu_rec.cu_pc[0]='\0';
		}
		else
			STRCPY(labels[cnt].sadd1,"     ");
	

			/* label address line 2 */ 

		if (strcmp(cu_rec.cu_adr1,"\0")){
			STRCPY(labels[cnt].sadd3,cu_rec.cu_adr1);
			cu_rec.cu_adr1[0]='\0';
		}
		else
		if (strcmp(cu_rec.cu_adr3,"\0")){
			STRCPY(labels[cnt].sadd3,cu_rec.cu_adr3);
			cu_rec.cu_adr3[0]='\0';
		}
		else
		if (strcmp(cu_rec.cu_pc,"\0")){
			STRCPY(labels[cnt].sadd3,cu_rec.cu_pc);
			cu_rec.cu_pc[0]='\0';
		}
		else
			STRCPY(labels[cnt].sadd3,"     ");
	

			/* label address line 3 */ 

		if (strcmp(cu_rec.cu_adr1,"\0")){
			STRCPY(labels[cnt].spc,cu_rec.cu_adr1);
			cu_rec.cu_adr1[0]='\0';
		}
		else
		if (strcmp(cu_rec.cu_adr3,"\0")){
			STRCPY(labels[cnt].spc,cu_rec.cu_adr3);
			cu_rec.cu_adr3[0]='\0';
		}
		else
		if (strcmp(cu_rec.cu_pc,"\0")){
			STRCPY(labels[cnt].spc,cu_rec.cu_pc);
			cu_rec.cu_pc[0]='\0';
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
	if(code < 0 && code != EFL) return(DBH_ERR);
	return(0);
}
/*---------------------------------------------------------------------------
	Description:  this function sets up the number of labels to print
		      across on a page and then prints our the labels.
---------------------------------------------------------------------------*/
print_labels()
{
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

