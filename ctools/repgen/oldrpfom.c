
#include <stdio.h>
#include "repname.h"
#include "struct.h"
#include "rep.h"

#define	NO_FMT	-10		/* end of all formats in a logical record */
#define	NO_LOG	-20 		/* end of logical records */

#define CHKERROR(X)	if(X == ERROR) return(ERROR)
int	rperror ;

#define	reterr(X)	{ rperror = X ;\
			if ( rperror != NO_FMT && rperror != NO_LOG) \
			printf("Internal data error occurred.. Rperror: %d\n",\
			rperror) ;\
			return(ERROR) ; }


#define fldhd	f_attri
#define fhd	f_global
#define fld_hd	fldattri
#define f_hd	fldglob

FILE	*fopen(), *fom_fd ;

static	char	Stru_buf[MAXSTRUCT][STRBUFSIZE] ;
				 /* addresses array for structure defns  */
static	struct	rp_name repnamerec ;
static	char	*comp() ;

main(argc, argv)
int	argc ;
char	*argv[] ;
{
char	proj[30] ;
int	log_recno, form_no ;
int	code ;
char	fom_file[FILE_NAME_LEN];



printf(" Project Name :");
scanf("%s", proj) ;

strcpy(fom_file, proj) ;
strcat(fom_file, FOMFILE) ;

if ( (fom_fd = fopen(fom_file, "w")) == NULL ) {
	printf("File %s open error \n");
	exit(0) ;
	}

/*	printf(" Logical Record Number :");
*	scanf("%d", &log_recno ) ;
*	printf(" Format Number :");
*	scanf("%d", &form_no) ;
*/

for(log_recno = 1; ; log_recno++) {
 	code = prnt_logrec(proj, log_recno) ;
	if ( code  != 0 ) {
		if ( rperror == NO_FMT) continue ;
		else break ;
		}
	
	for(form_no=1; form_no <= repnamerec.formrecs; form_no++) {
		code = prnt_frmat(proj, log_recno, form_no) ;
		if ( code < 0 ) break ;
		}
	}

end:
	fprintf(fom_fd, "\n\n0 0 0 0\n") ;	/* Termination pattern */
	fclose(fom_fd) ;

	printf(" %d Logical records Converted \n", log_recno-1) ;

	exit(0) ;

}


 int
prnt_logrec(name, logrec) 
char	*name ;
int	logrec ;
{
int	i ;

	CHKERROR(Rd_formrec(name,logrec,1)) ; 	/* AT least one format */

	fprintf(fom_fd, "Logical_Rec#: %d	Name: %s\n", 
					logrec, comp(repnamerec.rep_name) );
	fprintf(fom_fd, "Total_Records: %d\n", repnamerec.numstruct );
	fprintf(fom_fd, "Records_Are: ");
	for(i=0; i<repnamerec.numstruct ;i++)
		fprintf(fom_fd, "%d %s ", repnamerec.defstruct[i].strnum ,
					 repnamerec.defstruct[i].strname) ;
	fprintf(fom_fd, "\n");
	fprintf(fom_fd, "Total_Formats: %d\n", repnamerec.formrecs   );
	
	fprintf(fom_fd, "\n\n");
	return(0) ;

}
		

 int
prnt_frmat(name, logrec, formrec) 
char	*name ;
int	logrec, formrec ;
{
int	recno , fldno, i ;
INHDR	*hd_ptr ;
FLDINFO	*ptr ;
char	*comp() ;	/* compressing blanks */

	f_hd = (struct fhd *)(frmbuf) ;
	fld_hd = (struct fldhd *)(f_hd + 1) ; 

	CHKERROR(Open_str(name)) ;
	CHKERROR(Rd_formrec(name,logrec,formrec)) ; 
	CHKERROR(Rd_strrecs()) ;
	fprintf(fom_fd,"Format_Name: %s 	Records: %d \n", 
				comp(f_hd->formname), f_hd->Numrecs);
	fprintf(fom_fd,"\n\n");
	fprintf(fom_fd,"Record_Numbers: ");
	for(i=0; i <f_hd->Numrecs; i++ ) { 
		fprintf(fom_fd,"%d ", f_hd->Inrecno[i] ) ;
	}

	fprintf(fom_fd,"Record_Names: ");
	for(i=0; i <f_hd->Numrecs; i++ ) { 
		/****
		recno= f_hd->Inrecno[i];
		hd_ptr = (INHDR *)(Stru_buf[recno]) ;
		***/
		hd_ptr = (INHDR *)(Stru_buf[i]) ;
		fprintf(fom_fd," %s ", hd_ptr->stname) ;
	}

	fprintf(fom_fd,"\n\n");
	
	fprintf(fom_fd, "Lines: %d Columns: %d Linespace: %d Headings: %d\n",
	   f_hd->page_lines, f_hd->page_columns, f_hd->linespace, f_hd->mhdnos);

	fprintf(fom_fd,"Total_fields: %d	Subtotals: %d	Page_Breaks: %d\n\n",
		f_hd->tot_flds, f_hd->tot_keys, f_hd->tot_pghdr );
	
	for( i = 0 ; i < f_hd->mhdnos ; i++ ) {
		fprintf(fom_fd,"\t%s\n", comp(VARADDR+ f_hd->mhdoff[i]) ) ;
		}
	fprintf(fom_fd,"\n");	
	for( i = 0 ; i < f_hd->tot_flds ; i++) {
		fprintf(fom_fd, "%c %c %d %d %c %d %d ",
						(fld_hd+i)->fld_class ,
						(fld_hd+i)->fld_type ,
						(fld_hd+i)->fld_size, 
						(fld_hd+i)->repeatsuppress, 
						(fld_hd+i)->fld_justify,
						(fld_hd+i)->fld_totflag ,
				 		(fld_hd+i)->fld_serno) ;

		recno = (fld_hd+i)->in_takeno.fieldref.inrecno;
		fldno = (fld_hd+i)->in_takeno.fieldref.infieldno;
		if ( (fld_hd+i)->fld_class == INP_FLD ) {
		      	hd_ptr=(INHDR *) (Stru_buf[recno]) ;
		      	ptr=(FLDINFO *) (hd_ptr + 1);
			ptr += fldno ;
			
			fprintf(fom_fd,"%d %d %s\n",recno, fldno, ptr->fldname);
		} 
		else { 
			fprintf(fom_fd,"\n\tFormula:%s \n\tRever_Polish:%s\n",
					VARADDR+recno, comp(VARADDR+fldno));
		}
		fprintf(fom_fd, 
		"\tEdit_Mask:%s\n\tColumn_Name:%s\n\tField_Name:%s\n",
					VARADDR+(fld_hd+i)->edit_off,
					comp(VARADDR+(fld_hd+i)->title_off), 
					comp(VARADDR+(fld_hd+i)->fname_off));

	} /* for */


	for( i = 0 ; i < f_hd->tot_keys + f_hd->tot_pghdr ; i++) {
		recno = f_hd->keyno[i].inrecno ;
		fldno = f_hd->keyno[i].infieldno ;
		hd_ptr=(INHDR *) (Stru_buf[recno]) ;
		ptr=(FLDINFO *) (hd_ptr + 1);
		ptr += fldno ;
		
		fprintf(fom_fd,"%d %d %d %s ", 
			recno,fldno, f_hd->subserno[i], ptr->fldname);	
		fprintf(fom_fd,"\n\tMessage:%s ",
					comp(VARADDR+f_hd->msg_textoff[i]));
		fprintf(fom_fd,"\n\tEdit_Mask:%s \n",VARADDR+f_hd->keyeditoff[i]);
	}
	
 }

static  char *
comp(str)
char	*str ;
{
int	i ;

for(i = 0 ; i < strlen(str) ; i++) 
	if ( str[i] == ' ') str[i] = '~' ;	/* tilde serves as blank */
return(str) ;
}

/** Open structure and format definition file	*/

static int fds;			/* Structure def. file fd	*/

static int
Open_str(proj)
char *proj ;	/* project code	*/
{
	char filename[FILE_NAME_LEN] ;
	
	strcpy(filename,proj) ;
	strcat(filename,STRFILE) ;	/* structure def. file	*/
	if((fds = open(filename,RDMODE)) == -1){
		fprintf(stderr,"%s file open-read error\n",filename) ;
		fprintf(stderr,"Module : rp.c. Func : inoutopen\n") ;
		reterr(OPENERR) ;
	}

	return(NOERROR) ;
}


/* read the specified format record 	*/
static int 
Rd_formrec(proj,logrec,formrec)
char	*proj ;
int	logrec ;
int	formrec ;
{
	char	filename[FILE_NAME_LEN] ;
	int	fdfr ;

	strcpy(filename,proj) ;
	strcat(filename,REPNAME) ;	/* structure def. file	*/
	if((fdfr = open(filename,RDMODE)) == -1){
		fprintf(stderr,"%s file open-read error\n",filename) ;
		fprintf(stderr,"Module : rp.c. Func : Rd_formrec\n") ;
		reterr(OPENERR) ;
	}

	if(logrec < 1 ) {
		fprintf(stderr,"Wrong logical record number\n");
			fprintf(stderr,"Module : rp.c . Func : Rp_formrec\n");
			reterr(INTERR) ;
	}
	if(lseek(fdfr,(long)((logrec - 1) * sizeof(struct rp_name)), 0) < 0){
/***
		fprintf(stderr,"Lseek error in repname file\n");
		fprintf(stderr,"Module : rp.c . Func : Rd_formrec\n");
***/
		reterr(NO_LOG) ;
	}
	if(read(fdfr,(char *)&repnamerec ,sizeof(struct rp_name)) !=
						sizeof(struct rp_name )) {
/***
		fprintf(stderr,"%s file read error\n",filename ) ;
		fprintf(stderr,"Module : rp.c . Func :Rd_formrec\n") ;
***/
		reterr(NO_LOG) ;
	}

	/* validate the given format number */

	if(formrec > repnamerec.formrecs || formrec < 1){
/***
		fprintf(stderr,
		"Format# out of bound ! format#: %d, max form: %d\n",
		formrec,repnamerec.formrecs) ;
		fprintf(stderr,"Repname : %s\n", repnamerec.rep_name) ;
		fprintf(stderr,"Module : rp.c . Func : Rd_formrecs()\n");
***/
		reterr(NO_FMT) ;
	}
 
	strcpy(filename + strlen(proj),FRMFILE) ;
	if((fdfr = open(filename,RDMODE)) == -1){
		fprintf(stderr,"%s file open-read error\n",filename) ;
		fprintf(stderr,"Module : rp.c. Func : Rd_formrec\n") ;
		reterr(OPENERR) ;
	}
	if (lseek(fdfr,
	   (long)(((long)(repnamerec.formoff[formrec - 1]))*FRMBUFSIZE),0) < 0){
		fprintf(stderr,"Format file seek error\n") ;
		fprintf(stderr,"Module : rp.c. Func : Rd_formrec\n") ;
		reterr(SEEKERR) ;
	}
	if (read(fdfr,frmbuf,FRMBUFSIZE) < FRMBUFSIZE){
		fprintf(stderr,"Format file read error\n") ;
		fprintf(stderr,"Module : rp.c. Func : Rd_formrec\n") ;
		reterr(READERR) ;
	}
	close(fdfr) ;

	return(NOERROR) ;
}


/** read all the structure records used for the current report	*/
static int 
Rd_strrecs()
{
	int i ;

	/* Max. 'MAXSTRUCT' structures per report allowed */

	if(f_hd->Numrecs < 1 || f_hd->Numrecs > MAXSTRUCT){
		fprintf(stderr,"Internal consistency error : Numstruct: %d\n",
			f_hd->Numrecs) ;
		fprintf(stderr,"File : rp.c Func : Rd_strrecs\n") ;
		reterr(INTERR) ;
	}

	for(i = 0 ; i < f_hd->Numrecs ; i++) {

		if(lseek(fds,(long)((long)(f_hd->Inrecno[i])) * STRBUFSIZE,0) <
									0) {
			fprintf(stderr,
			"Seek error on structure def. file : Inrecno: %d\n",
			f_hd->Inrecno[i]) ;
			fprintf(stderr,"File : rp.c. Func : Rd_strrec");
			reterr(SEEKERR) ;
		}

		if(read(fds,Stru_buf[i],STRBUFSIZE) < STRBUFSIZE) {
			fprintf(stderr,
			"Stucture record read error,Inrecno: %d\n",
			f_hd->Inrecno[i]) ;
			fprintf(stderr,"File rp.c. Func : Rd_strrec\n") ;
			reterr(READERR) ;
		}
	}

	close(fds) ;

	return(NOERROR) ;
}

