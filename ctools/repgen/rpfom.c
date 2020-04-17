/*
*	rpfom.c
*
*	Program to create .RFM (ASCII File) file for a given REP-GEN project
*/

#include <stdio.h>
#include <string.h>
#include "rep.h"
#include "repname.h"
#include "struct.h"
#include <errno.h>

#define	NO_FMT	10		/* end of all formats in a logical record */
#define	NO_LOG	20 		/* end of logical records */

#define fldhd	f_attri
#define fhd	f_global
#define fld_hd	fldattri
#define f_hd	fldglob

FILE	*fopen(), *fom_fd ;
long	lseek() ;

static	char	Stru_buf[MAXSTRUCT][STRBUFSIZE] ;
			 /* addresses array for structure defns  */
static	struct	rp_name repnamerec ;
static	char	*comp() ;
static	int	maxstruct ;

//extern	int	errno ;

/*** Define function and prototypes ***/
static int Rd_formrec(char*, int, int);
static int Rd_strrecs(char *);


main(argc, argv)
int	argc;
char	*argv[];
{
	char	proj[30] ;
	int	log_recno, form_no ;
	int	code, i ;
	char	fom_file[FILE_NAME_LEN];
	INHDR	*hdr ;

	if( argc>1 )
		strncpy( proj, argv[1], 29 );
	else{
		printf("Project Name: ");
		scanf("%s", proj) ;
	}

	strcpy(fom_file, proj) ;
	strcat(fom_file, FOMFILE) ;

	if ( (fom_fd = fopen(fom_file, "w")) == NULL ) {
		printf("Project File %s open error\n");
		exit(-1) ;
	}

	if(Rd_strrecs(proj) == ERROR) {
		fclose(fom_fd) ;
		unlink(fom_file) ;
		printf("Failed to create .RFM file\n");
		exit(-1);
	}

	fprintf(fom_fd,"PROJECT: %s\n", proj);
	fprintf(fom_fd,"\tRECORDS:");
	for(i = 0 ; i < maxstruct ; i++) {
		hdr = (INHDR *)Stru_buf[i] ;
		fprintf(fom_fd," %d %s", i+1, hdr->stname );
	}
	fprintf(fom_fd,"\n\n");

	for(log_recno = 1; ; log_recno++) {
 		code = prnt_logrec(proj, log_recno) ;
		if ( code  != 0 ) {
			if ( code == NO_FMT) continue ;
			else break ;
		}
	
		for(form_no=1; form_no <= repnamerec.formrecs; form_no++) {
			code = prnt_frmat(proj, log_recno, form_no) ;
			if ( code < 0 ) break ;
		}
	}

	fprintf(fom_fd, "\nENDFILE\n") ;	/* Termination pattern */
	fclose(fom_fd) ;

	printf("%d Logical records Converted\n", log_recno-1) ;

	exit(0) ;
}

/*---------------------------------------------------------------------*/
prnt_logrec(name, logrec) 
char	*name ;
int	logrec ;
{
	int	code, i ;

	code = Rd_formrec(name,logrec,1) ; 	/* AT least one format */
	if(code < 0 || code == NO_LOG)return(code) ;

	fprintf(fom_fd, "LOGICAL_RECORD: %d\n", logrec) ;
	fprintf(fom_fd, "\tNAME:          %s\n", comp(repnamerec.rep_name) );
	fprintf(fom_fd, "\tTOTAL_RECORDS: %d\n", repnamerec.numstruct );
	fprintf(fom_fd, "\tRECORD_NAMES: ");
	for(i = 0 ; i < repnamerec.numstruct ; i++)
		fprintf(fom_fd, " %d %s",
			repnamerec.defstruct[i].strnum+1,
			repnamerec.defstruct[i].strname) ;
	fprintf(fom_fd, "\n");
	fprintf(fom_fd, "\tTOTAL_FORMATS: %d\n", repnamerec.formrecs) ;

	return(code) ;		/* Could be NO_FMT from Rd_formrec() */
}
/*---------------------------------------------------------------------*/
prnt_frmat(name, logrec, formrec) 
char	*name ;
int	logrec, formrec ;
{
	int	recno , fldno, i ;
	INHDR	*hd_ptr ;
	FLDINFO	*ptr ;

	f_hd = (struct fhd *)(frmbuf) ;
	fld_hd = (struct fldhd *)(f_hd + 1) ; 

	i = Rd_formrec(name,logrec,formrec) ; 
	if(i != NOERROR) return(i) ;

	fprintf(fom_fd,"\n\n");	
	fprintf(fom_fd,"FORMAT_NAME: %s\n", comp(f_hd->formname));
	fprintf(fom_fd, "\tLINES:        %d\n", f_hd->page_lines );
	fprintf(fom_fd, "\tCOLUMNS:      %d\n", f_hd->page_columns) ;
	fprintf(fom_fd, "\tLINESPACE:    %d\n", f_hd->linespace) ;
	fprintf(fom_fd, "\tTOTAL_FIELDS: %d\n", f_hd->tot_flds);
	fprintf(fom_fd, "\tSUBTOTALS:    %d\n", f_hd->tot_keys);
	fprintf(fom_fd, "\tPAGE_BREAKS:  %d\n", f_hd->tot_pghdr );
	
	fprintf(fom_fd,"\n");	
	fprintf(fom_fd, "HEADINGS: %d\n", f_hd->mhdnos);
	for( i = 0 ; i < f_hd->mhdnos ; i++ )
		fprintf(fom_fd,"\t%s\n", comp(VARADDR+ f_hd->mhdoff[i]) ) ;

	fprintf(fom_fd,"\n");	

	for( i = 0 ; i < f_hd->tot_flds ; i++) {
		fprintf(fom_fd,"\n");
		fprintf(fom_fd, "FIELD_CLASS: %s\n",
			((fld_hd+i)->fld_class == 'I') ? "INPUT" : "COMPUTE") ;

		recno = (fld_hd+i)->in_takeno.fieldref.inrecno;
		fldno = (fld_hd+i)->in_takeno.fieldref.infieldno;
		if ( (fld_hd+i)->fld_class == INP_FLD ) {
			recno = repnamerec.defstruct[recno].strnum ;
				/* Printing real recno in .RFM File */
		      	hd_ptr=(INHDR *) (Stru_buf[recno]) ;
		      	ptr=(FLDINFO *) (hd_ptr + 1);
			ptr += fldno ;
			
			fprintf(fom_fd,"\tFIELD_DETAILS:   %d %d %s\n",
					recno+1, fldno+1, ptr->fldname);
		} 
		else { 
			fprintf(fom_fd,"\tFORMULA:         %s\n",
					comp(VARADDR+recno) );
			fprintf(fom_fd,"\tREVERSE_POLISH:  %s\n",
					comp(VARADDR+fldno));
		}
		fprintf(fom_fd, "\tFIELD_TYPE:      %s\n",
			((fld_hd+i)->fld_type == 'N') ? "NUMERIC" : "CHAR") ;

		if((fld_hd+i)->fld_type == 'N')
			fprintf(fom_fd,"\tEDIT_MASK:       %s\n",
				comp(VARADDR+(fld_hd+i)->edit_off) ) ;
		else
			fprintf(fom_fd,"\tSIZE:            %d\n",
				(fld_hd+i)->fld_size);

		fprintf(fom_fd, "\tCOLUMN_HDG:      %s\n",
			comp(VARADDR+(fld_hd+i)->title_off));

		fprintf(fom_fd, "\tREPEAT_SUPPRESS: %s\n",
			((fld_hd+i)->repeatsuppress == 1) ? "YES" : "NO") ;
		fprintf(fom_fd, "\tJUSTIFICATION:   %c\n",
			(fld_hd+i)->fld_justify) ;
		fprintf(fom_fd, "\tTOTALING:        %s\n",
			((fld_hd+i)->fld_totflag == 1) ? "YES" : "NO") ;

		fprintf(fom_fd, "\tCONSTRNT_MIN:    %s\n",
				comp(VARADDR+(fld_hd+i)->minoff));
		fprintf(fom_fd, "\tCONSTRNT_MAX:    %s\n",
				comp(VARADDR+(fld_hd+i)->maxoff));
	} /* for */

	for( i = 0 ; i < f_hd->tot_pghdr + f_hd->tot_keys ; i++) {
		recno = f_hd->keyno[i].inrecno ;
		fldno = f_hd->keyno[i].infieldno ;
		recno = repnamerec.defstruct[recno].strnum ;
			/* Printing real recno in .RFM File */
		hd_ptr=(INHDR *) (Stru_buf[recno]) ;
		ptr=(FLDINFO *) (hd_ptr + 1);
		ptr += fldno ;
		
		fprintf(fom_fd,"\n");
		if(i >= f_hd->tot_pghdr)
			fprintf(fom_fd,"SUBTOTAL_FLD:" );
		else
			fprintf(fom_fd,"PAGE_BREAK_FLD:") ;
		fprintf(fom_fd," %d %d %s\n", recno+1, fldno+1, ptr->fldname);
		fprintf(fom_fd,"\tMESSAGE:   %s\n",
			comp(VARADDR+f_hd->msg_textoff[i]));
		fprintf(fom_fd,"\tEDIT_MASK: %s\n",
			comp(VARADDR+f_hd->keyeditoff[i]));
	}

	return(0) ;
}

/*---------------------------------------------------------------------*/
static  char *comp(str)
char	*str ;
{
	int	i ;

	for(i = 0 ; i < strlen(str) ; i++) 
		if(str[i] == ' ') str[i] = '~' ;    /* tilde serves as blank */

	if(i == 0) return("~");			/* NULL string */

	return(str) ;
}
/*---------------------------------------------------------------------*/
/** Open structure and format definition file and read the field defs */

static	int
Rd_strrecs(proj)
char	*proj ;	/* project code	*/
{
	int	fds;			/* Structure def. file fd	*/
	int	size ;
	char	filename[FILE_NAME_LEN] ;
	
	strcpy(filename,proj) ;
	strcat(filename,STRFILE) ;	/* structure def. file	*/
	if((fds = open(filename,RDMODE)) == -1){
		fprintf(stderr,"'%s' file Open Error\n",filename) ;
		return(ERROR) ;
	}

	for(maxstruct = 0 ; maxstruct < MAXSTRUCT ; maxstruct++) {
		size = read(fds,Stru_buf[maxstruct],STRBUFSIZE) ;
		if(size == 0) break ;
		if(size < STRBUFSIZE) {
			fprintf(stderr,
			"Stucture record read error.. errno: %d  size: %d\n",
			errno, size) ;
			close(fds) ;
			return(ERROR) ;
		}
	}

	close(fds) ;

	return(NOERROR) ;
}
/*---------------------------------------------------------------------*/
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
		fprintf(stderr,"'%s' file Open Error\n",filename) ;
		return(ERROR) ;
	}

	if(logrec < 1 ) {
		fprintf(stderr,"Wrong logical record number %d\n", logrec);
		close(fdfr) ;
		return(ERROR) ;
	}

	if(lseek(fdfr,(long)((logrec - 1) * sizeof(struct rp_name)),0) < 0) {
		fprintf(stderr,"Lseek() error..  errno: %d\n",errno);
		close(fdfr) ;
		return(ERROR) ;
	}

	if(read(fdfr,(char *)&repnamerec ,sizeof(struct rp_name)) !=
						sizeof(struct rp_name )) {
		close(fdfr) ;
		return(NO_LOG) ;
	}

	/* validate the given format number */

	if(formrec > repnamerec.formrecs || formrec < 1){
		close(fdfr) ;
		return(NO_FMT) ;
	}
 
	close(fdfr) ;

	strcpy(filename + strlen(proj),FRMFILE) ;
	if((fdfr = open(filename,RDMODE)) == -1){
		fprintf(stderr,"'%s' file Open Error\n",filename) ;
		return(ERROR) ;
	}
	if (lseek(fdfr,
	   (long)(((long)(repnamerec.formoff[formrec - 1]))*FRMBUFSIZE),0) < 0){
		fprintf(stderr,"Format file seek error\n") ;
		close(fdfr) ;
		return(ERROR) ;
	}
	if (read(fdfr,frmbuf,FRMBUFSIZE) < FRMBUFSIZE){
		fprintf(stderr,"Format file read error\n") ;
		close(fdfr) ;
		return(ERROR) ;
	}

	close(fdfr) ;

	if(f_hd->Numrecs > repnamerec.numstruct) {
		fprintf(stderr,
			".RPN (logical record) file is having less no of structures\n") ;
		return(ERROR) ;
	}
	return(NOERROR) ;
}

