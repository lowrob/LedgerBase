/*-----------------------------------------------------------------------
Source Name: Conversion       
System     : Budgetary Financial system.
Module     : General Ledger system.
Created  On: Nov 26, 1990.
Created  By: peter ralph.

DESCRIPTION:
	To right justify all numeric supplier codes and leave alphanumeric
	supplier codes as they are. Files which are to be converted are:
	
	File Name		Fields
	---------		------
	CUSTOMER_FILE		cu_code 
	ARSDR    		ah_cu_cd 

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

------------------------------------------------------------------------*/
/* --------------------------------------------------------------------------

MODIFICATIONS:

PROGRAMMER           YY/MM/DD        DESCRIPTION OF MODIFICATIONS
^^^^^^^^^^           ^^^^^^^^        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

----------------------------------------------------------------------------*/

#define MAIN
#define MAINFL		SUPPLIER

#include <stdio.h>
#include <ctype.h>
#include <repdef.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

static Cu_rec   cu,rj_cu;
static Ar_hdr  	ar,rj_ar;

static	char 	e_mesg[80];
static	int 	retval;
static	int	code;

main(argc,argv)
int	argc;
char	*argv[];
{

	proc_switch(argc,argv,MAINFL);
	printf("Right Justify Customer Code\n\n");

	if(rj_customer()!= NOERROR)
		printf("!!!!! CONVERSION OF CUSTOMER_FILE FAILED\n");
	else
		printf("Conversion of CUSTOMER_FILE completed\n");

	if(rj_invoice()!= NOERROR)
		printf("!!!!! CONVERSION OF APINVOICE_FILE FAILED\n");
	else
		printf("Conversion of APINVOICE_FILE completed\n");

	close_dbh();
	exit(0);
}


rj_customer()
{
	printf("\n\n----------FILE:CUSTOMER_FILE----------\n\n");
	cu.cu_code[0] = 0;
	flg_reset(CUSTOMER);

        for( ; ; ) {
		code = get_n_cust(&cu,UPDATE,0,FORWARD,e_mesg);
		if(code < 0) {
			if(code == EFL) break;
			printf("\n%s\n",e_mesg);
			roll_back(e_mesg);
			break;
		}
  		memcpy(&rj_cu,&cu,sizeof(Cu_rec));  
		if((Right_Justify_Numeric(rj_cu.cu_code,
						sizeof(cu.cu_code)-1))){

		   if((strcmp(rj_cu.cu_code,cu.cu_code) != 0)){
		           

			printf("Customer Code %s \n",rj_cu.cu_code);
             		code = put_cust(&cu,P_DEL,e_mesg);
			if(code < 0) {
				printf("\n%s\n",e_mesg);
				roll_back(e_mesg);
				break;
			} 
          		code = put_cust(&rj_cu,ADD,e_mesg);
			if(code < 0) {
				printf("\n%s\n",e_mesg);
				roll_back(e_mesg); 
				break;
			}
			commit(e_mesg);
		   } 
		}else roll_back(e_mesg);
		inc_str(cu.cu_code,sizeof(cu.cu_code)-1,FORWARD);
	}
	if(code == EFL) {
		printf("\nEnd-Of-File\n");
		return(NOERROR);
	}
	return(ERROR);
}

rj_invoice()
{

	printf("\n\n----------FILE:APINVOICE_FILE----------\n\n");
	
	flg_reset(ARSHDR);
	ar.ah_fund   = 0;
	ar.ah_inv_no = 0;
	ar.ah_sno    = 0;

        for( ; ; ) {
		code = get_n_arhdr(&ar,UPDATE,0,FORWARD,e_mesg);
		if(code < 0) {
			if(code == EFL) break;
			printf("\n%s\n",e_mesg);
			roll_back(e_mesg);
			break;
		}
  		memcpy(&rj_ar,&ar,sizeof(ARSHDR));  
		if(Right_Justify_Numeric(rj_ar.ah_cu_code,
					 sizeof(rj_ar.ah_cu_code)-1)){ 

		   if(strcmp(rj_ar.ah_cu_code,ar.ah_cu_code) !=0){
			printf("Supplier Code %s\n",ar.ah_cu_code);
             		code = put_arhdr(&ar,P_DEL,e_mesg);
			if(code < 0) {
				printf("\n%s\n",e_mesg);
				roll_back(e_mesg);
				break;
			} 
			commit(e_mesg);
          		code = put_arhdr(&rj_ar,ADD,e_mesg);
			if(code < 0) {
				printf("\n%s\n",e_mesg);
				roll_back(e_mesg); 
				break;
			}
			commit(e_mesg);
		   } 
		}else roll_back(e_mesg);
		inc_str(ar.ah_cu_code,sizeof(ar.ah_cu_code)-1,FORWARD);

  			  
	}
	if(code == EFL) {
		printf("\nEnd-Of-File\n");
		return(NOERROR);
	}
	return(ERROR);
}

