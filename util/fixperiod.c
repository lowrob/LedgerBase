/****
*       Source  : fixperiod.c 
DESC:
	This program is to be used once to reset the values in the GL,
sttran, transheader, and the transitem files. A problem was found that
transactions were being recorded with the period that was found in the 
requisition header. This posted things to periods 9,10,11, and 12 of this
year. This program reads the gltrans file sequentialy by date, starting at
1993/07/20 to put entries in the proper period and correct the current
balance in the glmast.
*/


#define MAIN
#define MAINFL	GLTRAN

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_recs.h>

#define SYSTEM          "UTILITIES"       /*Sub System Name */
#define MOD_DATE        "12-SEPT-93"    /* Program Last Modified */
#define PROC_PERIOD     12
#define PER1START       19930720
#define PER2START       19930807
#define PER3START       19930904
#define FILE_NAME	"perfix9.dat"

static  St_tran  st_tran;   
static  Tr_hdr  tr_hdr ;
static  Tr_item tr_item ;
static  Gl_rec  gl_rec ;

static  char e_mesg[50];
static  short correctperiod;


main(argc,argv)
int argc;
char *argv[];
{
	int err;
	
	strncpy(SYS_NAME,SYSTEM,50);    /* Sub system name */
	strncpy(CHNG_DATE,MOD_DATE,10); /* Modification Date */
	proc_switch(argc, argv, MAINFL) ;       /* Process Switches */

	err = process();
	if(err < 0){
		printf("\n\t Problem occured %d !!!!!!!!!!",err);
		getchar();
	}
	close_dbh();
	exit(0);
}

process()
{

	int     retval;

	tr_hdr.th_fund = 1;
	tr_hdr.th_reccod = 99;
	tr_hdr.th_create[0] = 'G';
	tr_hdr.th_seq_no = 0;   
	flg_reset(GLTRHDR);

	/* Read trans file until EOF */
	for(;;){
		retval = get_n_trhdr(&tr_hdr, BROWSE, 0, FORWARD, e_mesg) ;
		if(retval == EFL) break;
		if(retval < 0) {
			printf("\n\t%s",e_mesg);
			printf("\n\tTRANS HEADER: %ld",tr_hdr.th_seq_no);
			roll_back(e_mesg);
			return(retval);
		}
		/* See if period is wrong */
		if(tr_hdr.th_period == PROC_PERIOD){
			retval = get_trhdr(&tr_hdr, UPDATE, 0,e_mesg);
			if(retval < 0) {
				printf("\n\tRead in update on trhdr %s",e_mesg);
				roll_back(e_mesg);
				return(retval);
			}
			retval = fixperiod();
			if(retval < 0)
				return(retval);
			commit();
		}
	}
	return(NOERROR);
}

/* Problem was found, make adjustments */
fixperiod()
{
	
	int retval;

	printf("\nChangeing Gltransaction: %d, %d, %s, %ld",tr_hdr.th_fund,
		tr_hdr.th_reccod, tr_hdr.th_create, tr_hdr.th_seq_no);
	fprintf(FILE_NAME,
	    "\nChangeing Gltransaction: %d, %d, %s, %ld",tr_hdr.th_fund,
		tr_hdr.th_reccod, tr_hdr.th_create, tr_hdr.th_seq_no);

	/* Determine correct period */
	if(tr_hdr.th_date <= PER1START){
		printf("\nDate is lower PER1START, period is diff %d %d %s %ld",
			tr_hdr.th_fund,tr_hdr.th_reccod,tr_hdr.th_create,
			tr_hdr.th_seq_no);
		getchar();
		return(NOERROR);
	}
	else if(tr_hdr.th_date >= PER1START && tr_hdr.th_date < PER2START)
		correctperiod = 1;
	else if(tr_hdr.th_date >= PER2START && tr_hdr.th_date < PER3START)
		correctperiod = 2;
	else if(tr_hdr.th_date >= PER3START)
		correctperiod = 3;
	printf("\nCorrecting period %d to %d",tr_hdr.th_period, correctperiod);

	/* set up trans item key 0 */
	tr_item.ti_fund = tr_hdr.th_fund;
	tr_item.ti_reccod = tr_hdr.th_reccod;
	strcpy(tr_item.ti_create, tr_hdr.th_create);
	tr_item.ti_seq_no = tr_hdr.th_seq_no;
	tr_item.ti_item_no = 0;
	flg_reset(GLTRAN);

	for(;;tr_item.ti_item_no ++){
		
		/* Read next trans item that belongs to header */
		retval = get_n_tritem(&tr_item,UPDATE,0,FORWARD,e_mesg);
		if(retval == EFL) break;
		if(retval < 0){
			printf("\nDBH ERROR in tritem occured:%d :%s",retval, e_mesg);
			getchar();
			roll_back(e_mesg);
			return(retval);
		}
		/* Check item belongs to header */
		if(tr_item.ti_seq_no != tr_hdr.th_seq_no)
			break;

		gl_rec.funds = tr_item.ti_fund;
		strcpy(gl_rec.accno, tr_item.ti_accno);
		gl_rec.reccod = tr_item.ti_reccod;

		/* Get GL master file for future changes */
		retval = get_gl(&gl_rec,UPDATE,0,e_mesg);
		if(retval == UNDEF) {
			printf("\n\nGL record not found: %d-%s-%d\n",
				gl_rec.funds,gl_rec.accno,gl_rec.reccod);
			getchar();
			roll_back(e_mesg);
			return(retval);
		}
		if(retval < 0){
			printf("\n\t%s",e_mesg);
			roll_back(e_mesg);
			return(retval);
		}

		/* Set Stock trans file for read */
		st_tran.st_date = tr_hdr.th_date;
		strcpy(st_tran.st_type, "IS");
		st_tran.st_seq_no = 0;
		flg_reset(STTRAN);

		/* Get Stock trans file to make correction to period */
		for(;;){
			retval=get_n_sttran(&st_tran,BROWSE,0,FORWARD,e_mesg);
			if(retval < 0) {
				printf("DBH ERROR: %s",e_mesg);
				getchar();
				roll_back(e_mesg);
				return(retval);
			}
			/* see if sttran file is same as item file */
			if(st_tran.st_date != tr_hdr.th_date){
				printf("\nMatching STTRAN not found for Trans Item %d, %d,%s, %ld",
					tr_item.ti_fund, tr_item.ti_reccod, 
					tr_item.ti_create, tr_item.ti_seq_no);
				getchar();
				roll_back(e_mesg);
				return(NOERROR);
			}
			
			/* check all the rest of the fields */
			if(abs(st_tran.st_amount)==abs(tr_item.ti_amount) &&
			    (strcmp(st_tran.st_db_acc, tr_item.ti_accno)==0||
			    strcmp(st_tran.st_cr_acc, tr_item.ti_accno)==0)){
				/* Have all the correct files now make change and write */
				retval = makechange();
				if(retval < 0)
					return(retval);
				break;
			}
		}
	} /* End of for loop */

}

/* Make changes to the files and write. */
makechange()
{
	int     retval;

	/*set the stock trans period correct if it has not already been done*/
	if(st_tran.st_period != correctperiod){
		retval = get_sttran(&st_tran,UPDATE,0,e_mesg);
		if(retval < 0){
			printf("\n\nError on read STTRAN: %s",e_mesg);
			roll_back(e_mesg);
			getchar();
			return(retval);
		}        
		st_tran.st_period = correctperiod;
		retval = put_sttran(&st_tran, UPDATE, e_mesg);
		if(retval < 0){
			printf("\n\nWrite Error to STTRAN: %s",e_mesg);
			roll_back(e_mesg);
			getchar();
			return(retval);
		}        
	}
	/* Set transaction header period correct if it has not yet been done */
	if(tr_hdr.th_period != correctperiod){
		tr_hdr.th_period = correctperiod;
		retval = put_trhdr(&tr_hdr, UPDATE, e_mesg);
		if(retval < 0){
			printf("\n\nWrite Error to TRHDR: %s",e_mesg);
			roll_back(e_mesg);
			getchar();
			return(retval);
		}
	}
	/* Change the current amounts for periods in GL */
	/* Decrease wrong period total, increase correct period */

	fprintf(FILE_NAME,
	    "\nChangeing amount of %lf from period %d to %d for acct: %s",
	    tr_item.ti_amount,tr_item.ti_period,correctperiod,gl_rec.accno);
	printf("\nChangeing amount of %lf from period %d to %d for acct: %s",
	    tr_item.ti_amount,tr_item.ti_period,correctperiod,gl_rec.accno);

	gl_rec.currel[tr_item.ti_period-1] -= tr_item.ti_amount;
	gl_rec.currel[correctperiod-1] += tr_item.ti_amount;

	retval = put_gl(&gl_rec, UPDATE, e_mesg);
	if(retval < 0){
		printf("\n\nWrite Error to GLMAST: %s",e_mesg);
		roll_back(e_mesg);
		getchar();
		return(retval);
	}

	/* Set transaction item period correctly */
	tr_item.ti_period = correctperiod;
	retval = put_tritem(&tr_item, UPDATE, e_mesg);
	if(retval < 0){
		printf("\n\nWrite Error to TRITEM: %s",e_mesg);
		roll_back(e_mesg);
		getchar();
		return(retval);
	}
	return(NOERROR);
}	
