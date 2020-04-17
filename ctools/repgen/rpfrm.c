/*
*	rpfrm.c
*
*	Program to create .RPN and .RPF files from .RFM (ASCII) file
*/

#include <stdio.h>
#include <ctype.h>
#include "rep.h"
#include "repname.h"
#include "struct.h"
#include <errno.h>


#define	EFL		-200
#define CHKERROR(X)	if(X == ERROR) return(ERROR)
#define	prnterr(S)	fprintf(err_fd,"ERROR: %s\n", S)

#define	ERRFILE	".err"

#define fldhd	f_attri
#define fhd	f_global
#define fld_hd	fldattri
#define f_hd	fldglob


static	char	Stru_buf[MAXSTRUCT][STRBUFSIZE] ;
static	struct	rp_name repnamerec ;

static	int	fdf = -1;
static	int	maxstruct ;

long	lseek() ;
FILE	*fopen(), *fom_fd, *err_fd ;

static	char	*comp() ;	/* compressing blanks */

//extern	int	errno ;

char	errmsg[200] ;

/*** Define function and prototype ***/
static int Rd_strrecs(char*);
static int CheckRecords(char*);
static int Rd_formrec(char*, int);

main(argc,argv)
int	argc;
char	*argv[];
{
	char	proj[30] ;
	int	log_recno, form_no ;
	char	fom_file[FILE_NAME_LEN];
	int	formats = 0 ;
	int	code = 0 ;

	if( argc>1 )
		strncpy( proj, argv[1], 29 );
	else{
		printf("Project Name: ");
		scanf("%s", proj) ;
	}

	strcpy(fom_file, proj) ;
	strcat(fom_file, FOMFILE) ;

	if ( (fom_fd = fopen(fom_file, "r")) == NULL ) {
		printf("File %s open error\n", fom_file);
		exit(-1) ;
	}

	strcpy(fom_file, proj) ;
	strcat(fom_file, FRMFILE) ;

	if ( access(fom_file, 0) >= 0 ) {
		printf("%s File exists.. Can't Create\n", fom_file );
		fclose(fom_fd) ;
		exit(-1) ;
	}

	/* Open Error file */
	strcpy(fom_file, proj) ;
	strcat(fom_file, ERRFILE) ;
	if ( (err_fd = fopen(fom_file, "w")) == NULL ) {
		printf("File %s open error\n", fom_file);
		fclose(fom_fd);
		exit(0) ;
	}
	if(Rd_strrecs(proj) < 0) {
		fclose(fom_fd);
		fclose(err_fd);
		exit(-1);
	}

	if(CheckRecords(proj) < 0) {
		fclose(fom_fd);
		fclose(err_fd);
		exit(-1);
	}

	for(log_recno = 1;;log_recno++){
 		formats = ReadLogRec() ;
		if ( formats < 0) break ;
		code = Add_logrec(proj, log_recno) ;
		if(code != NOERROR) break ;
		if ( formats == 0 ) continue ; 

		for(form_no=1; form_no <= formats ; form_no++)  {
			code = creat_frmat(proj, log_recno);
			if(code != NOERROR) break ;
		}
		if(code != NOERROR) break ;
		close(fdf) ;
	}

	fclose(fom_fd) ;
	fclose(err_fd);
	close(fdf) ;

	if(code == ERROR || formats == ERROR) {
		printf("\nUnsuccessful Creation of .RPF file\n");
		exit(-1);
	}
	strcpy(fom_file, proj) ;
	strcat(fom_file, ERRFILE) ;
	unlink(fom_file) ;
	printf("%d Logical records Converted\n", log_recno-1) ;
	exit(1);
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
		fprintf(err_fd,"'%s' file Open Error..  errno: %d\n",
			filename, errno) ;
		return(ERROR) ;
	}

	for(maxstruct = 0 ; maxstruct < MAXSTRUCT ; maxstruct++) {
		size = read(fds,Stru_buf[maxstruct],STRBUFSIZE) ;
		if(size == 0) break ;
		if(size < STRBUFSIZE) {
			fprintf(err_fd,
			"Stucture record read error.. errno: %d  size: %d\n",
			errno, size) ;
			close(fds) ;
			return(ERROR) ;
		}
	}

	close(fds) ;

	return(NOERROR) ;
}
/*-------------------------------------------------------------------*/
/* Check Given records are same as in Stru_buf[] */

static	int
CheckRecords(proj)
char	*proj ;
{
	char	str[50] ;
	int	i, j ;
	INHDR	*hdr ;

	/* SKip "PROJECT:" and get project name */
	fscanf(fom_fd,"%*s%s", str) ;
	if(strcmp(str, proj) != 0) {
		prnterr("Project name is not matching");
		return(ERROR) ;
	}

	/* SKip "RECORDS:" */
	fscanf(fom_fd,"%*s") ;

	/* Structure names */
	for(i = 0 ; i < maxstruct ; i++) {
		fscanf(fom_fd, "%d%s", &j, str) ;
		j--;	/* index# one less than record# in ascii file */
		hdr = (INHDR *)Stru_buf[i] ;
		if(i != j || strcmp(str,hdr->stname) != 0) {
			prnterr("Record Names are not Proper");
			return(ERROR) ;
		}
	}

	return(NOERROR) ;
}	/* CheckRecords() */
/*-------------------------------------------------------------------*/
/* Read Logical Record Infomation */

ReadLogRec() 
{
	int	i, j ;
	short	formats ;
	int	logrec = 0 ;
	INHDR	*hdr ;
	char	str[50] ;

	fscanf(fom_fd,"%s", str) ;
	if(strcmp(str,"ENDFILE") == 0) return(EFL) ;
	if(strcmp(str,"LOGICAL_RECORD:") != 0) {
		prnterr("Not a Logical Record.. ") ;
		return(ERROR) ;
	}
	/*  get logrec#, skip "NAME:" and get name */
	fscanf(fom_fd, "%d%*s%s", &logrec, repnamerec.rep_name) ;
	if ( logrec <= 0 ) {
		sprintf(errmsg, "Invalid Logical Record.. Log_Rec#: %d",
			logrec) ;
		prnterr(errmsg);
		return(ERROR) ;
	}

	comp(repnamerec.rep_name);

	/* Skip "TOTAL_RECORDS:" and get number */
	fscanf(fom_fd, "%*s%hd", &repnamerec.numstruct );

	/* Skip "RECORD_NAMES:" */
	fscanf(fom_fd, "%*s");
	/* get record# and name */
	for(i = 0 ; i < repnamerec.numstruct ; i++) {
		fscanf(fom_fd, "%d%s", &j, repnamerec.defstruct[i].strname) ;
		j--;
		if(j < 0 || j >= maxstruct) {
			sprintf(errmsg, "Invalid Structure#.. Log_Rec#: %d",
				logrec) ;
			prnterr(errmsg);
			return(ERROR) ;
		}
		repnamerec.defstruct[i].strnum = j ;
		hdr = (INHDR *)Stru_buf[j] ;
		if(strcmp(hdr->stname, repnamerec.defstruct[i].strname) != 0) {
			sprintf(errmsg, "Invalid Structure name.. Log_Rec#: %d",
				logrec) ;
			prnterr(errmsg);
			return(ERROR) ;
		}
	}
	/* Skip "TOTAL_FORMATS:" and get no of formats */
	fscanf(fom_fd, "%*s%hd", &formats) ;
	
	repnamerec.formrecs = 0 ;
	repnamerec.del_flag = NOTDELETED ;

	return(formats) ;
}
/*-----------------------------------------------------------------*/

Add_logrec(proj, logrec) 
char	*proj ;
int	logrec ;
{
	char	filename[FILE_NAME_LEN] ;

	if(logrec < 1 ) {
		fprintf(err_fd,"Wrong logical record number %d\n",logrec);
		fprintf(err_fd,"Add_logrec()..  Internal Error\n");
		return(ERROR) ;
	}

	strcpy(filename,proj) ;
	strcat(filename,REPNAME) ;	/* structure def. file	*/

	if ( logrec == 1 || access(filename, 0) < 0 ) {
		if ((fdf = creat(filename, CRMODE)) == -1 ) {
			fprintf(err_fd,"%s file Creation error.. errno: %d\n",
				filename, errno) ;
			return(ERROR) ;
		}
		close(fdf) ;
		strcpy(filename,proj) ;
		strcat(filename, FRMFILE) ;
		unlink(filename) ;
	}

	strcpy(filename,proj) ;
	strcat(filename,REPNAME) ;	/* structure def. file	*/

	if((fdf = open(filename,RWMODE)) == -1){
		fprintf(err_fd,"%s file Open error errno: %d\n",
			filename, errno) ;
		return(ERROR) ;
	}

	if(lseek(fdf,(long)((logrec - 1) * sizeof(struct rp_name)), 0) < 0){
		fprintf(err_fd,"Lseek error in repname file.. errno: %d\n",
			errno);
		close(fdf) ;
		return(ERROR) ;
	}
	if(write(fdf,(char *)&repnamerec ,sizeof(struct rp_name)) !=
						sizeof(struct rp_name )) {
		fprintf(err_fd,"%s file write error. errno: %d\n",
			filename, errno) ;
		close(fdf) ;
		return(ERROR) ;
	}
	close(fdf) ;
	return(0) ;
}

/*--------------------------------------------------------------------*/

creat_frmat(name, logrec)
char	*name ;
int	logrec ;
{
	short	recno , fldno, i,j ;
	int	form_no ;
	char	dummy[80] ;
	int	offset ;
	int	formrecno[MAXSTRUCT] ;
	INHDR	*hd_ptr ;
	FLDINFO	*ptr ;

	f_hd = (struct fhd *)(frmbuf) ;
	fld_hd = (struct fldhd *)(f_hd + 1) ; 

	/* Get Next Format# */
	CHKERROR((form_no=Rd_formrec(name,logrec))) ; 

	/* get "FORMAT_NAME:" */
	fscanf(fom_fd,"%s", dummy) ;
	if(strcmp(dummy,"ENDFILE") == 0) return(EFL) ;
	if(strcmp(dummy,"FORMAT_NAME:") != 0) {
		sprintf(errmsg,"Not a FORMAT_NAME.. Log_Rec#: %d Format#: %d",
			logrec, form_no) ;
		prnterr(errmsg);
		return(ERROR);
	}
	/* get format name */
	fscanf(fom_fd,"%s", f_hd->formname );
	comp(f_hd->formname) ;

	f_hd->Numrecs = repnamerec.numstruct ;

	for(i=0; i <f_hd->Numrecs; i++ ) { 
		f_hd->Inrecno[i] = repnamerec.defstruct[i].strnum ; 
		/* Record# in a logical record for a given structure# */
		formrecno[f_hd->Inrecno[i]] = i ;
	}

	/* Skip "LINES:" and read lines */
	fscanf(fom_fd,"%*s%hd", &f_hd->page_lines );
	/* Skip "COLUMNS:" and read coulumns */
	fscanf(fom_fd,"%*s%hd", &f_hd->page_columns );
	/* Skip "LINESPACE:" and read linespace */
	fscanf(fom_fd,"%*s%hd%", &i) ;
	f_hd->linespace = i ;
	/* Skip "TOTAL_FIELDS:" and get tot_flds */
	fscanf(fom_fd,"%*s%hd", &f_hd->tot_flds );
	/* Skip "SUBTOTLAS:" and get tot_keys */
	fscanf(fom_fd,"%*s%hd", &f_hd->tot_keys );
	/* Skip "PAGE_BREAKS:" and get tot_pghdr */
	fscanf(fom_fd,"%*s%hd", &f_hd->tot_pghdr );

	offset = 0 ;
	strcpy(VARADDR+offset, "\0");
	offset++ ;

	/* Skip "HEADINGS:" and read haedings */
	fscanf(fom_fd,"%*s%hd", &f_hd->mhdnos );
	
	for( i = 0 ; i < f_hd->mhdnos ; i++) {
		f_hd->mhdoff[i] = offset ;
		fscanf(fom_fd,"%s", VARADDR+ f_hd->mhdoff[i] ) ;
		comp(VARADDR + f_hd->mhdoff[i]) ;
		offset += strlen(VARADDR + f_hd->mhdoff[i]) + 1 ;
	}

	for( i = 0 ; i < f_hd->tot_flds ; i++) {
		fscanf(fom_fd,"%s", dummy) ;
		if(strcmp(dummy,"FIELD_CLASS:") != 0) {
		    sprintf(errmsg,"Not a Field.. Log_Rec#: %d Format#: %d Fld#: %d",
			logrec, form_no, (i+1)) ;
		    prnterr(errmsg);
		    return(ERROR);
		}

		/* Get Fld class */
		fscanf(fom_fd,"%s", dummy) ;
		if(dummy[0] != INP_FLD && dummy[0] != COMP_FLD) {
		    sprintf(errmsg,"Invalid Class.. Log_Rec#: %d Format#: %d Fld#: %d",
			logrec, form_no, (i+1)) ;
		    prnterr(errmsg);
		    return(ERROR) ;
		}
		(fld_hd+i)->fld_class = dummy[0] ;

		if((fld_hd+i)->fld_class == INP_FLD) {
			/* Skip "FIELD_DETAILS:" & get recno, fldno, and name */
			fscanf(fom_fd,"%*s%hd%hd%s", &recno, &fldno,
				VARADDR+offset) ;
			recno--;
			fldno--;
			for(j = 0 ; j < f_hd->Numrecs; j++)
				if(recno == f_hd->Inrecno[j]) break ;
			if(j == f_hd->Numrecs) {	/* Record not matched */
			    sprintf(errmsg, "Invalid Rec#: %d.. Log_Rec#: %d Format#: %d Fld#: %d",
			    recno, logrec, form_no, (i+1)) ;
			    return(ERROR) ;
			}
			hd_ptr = (INHDR *)(Stru_buf[formrecno[recno]]) ;
			if(fldno >= hd_ptr->stflds) {
			    sprintf(errmsg, "Invalid Fld#: %d.. Log_Rec#: %d Format#: %d Fld#: %d",
			    fldno, logrec, form_no, (i+1)) ;
			    return(ERROR) ;
			}
			(fld_hd+i)->in_takeno.fieldref.inrecno =
					formrecno[recno] ;
			(fld_hd+i)->in_takeno.fieldref.infieldno = fldno ;

			(fld_hd+i)->fname_off = offset ;
			offset += strlen(VARADDR+offset) + 1 ;

			(fld_hd+i)->fld_serno =
				FldSerno(formrecno[recno]) + fldno + 1 ;
		}
		else { 
			/* Skip "FORMULA:" and get formula */
			fscanf(fom_fd,"%*s%s", dummy ) ;
			comp(dummy) ;
			strcpy(VARADDR+offset , dummy) ;
			(fld_hd+i)->in_takeno.comp_off[0] = offset ;
			offset += strlen(VARADDR+offset)  + 1;

			/* Skip "REVERSE_POLISH:" and get formula */
			fscanf(fom_fd,"%*s%s", dummy ) ;
			comp(dummy) ;
			strcpy(VARADDR+offset ,dummy);
			(fld_hd+i)->in_takeno.comp_off[1] = offset ;
			offset += strlen(VARADDR+offset)  + 1;

			/* Copy Name */
			strcpy(VARADDR+offset, "COMPUTED") ;
			(fld_hd+i)->fname_off = offset ;
			offset += strlen(VARADDR+offset) + 1 ;

			(fld_hd+i)->fld_serno = 0 ;
		}

		/* Skip "FIELD_TYPE:" and get Fld type */
		fscanf(fom_fd,"%*s%s", dummy) ;
		if((fld_hd+i)->fld_class == COMP_FLD) {
			if(dummy[0] != NUME_FLD) {
		 	    sprintf(errmsg,"Non Numeric type.. Log_Rec#: %d Format#: %d Fld#: %d",
			logrec, form_no, (i+1)) ;
			    prnterr(errmsg);
			    return(ERROR) ;
			}
		}

		(fld_hd+i)->fld_type = dummy[0] ;

		if((fld_hd+i)->fld_type == NUME_FLD) {
			/* Skip "EDIT_MASK" and get mask */
			fscanf(fom_fd,"%*s%s", dummy ) ;
			strcpy(VARADDR+offset ,dummy );
			(fld_hd+i)->edit_off = offset ;
			offset += strlen(VARADDR+offset) + 1 ;

			(fld_hd+i)->fld_size = strlen(dummy) ;
		}
		else {
			/* Skip "SIZE" and get size */
			fscanf(fom_fd,"%*s%hd", &(fld_hd+i)->fld_size ) ;
			(fld_hd+i)->edit_off = 0 ;
		}

		/* Skip "COLUMN_HDG:" and get field title */
		fscanf(fom_fd,"%*s%s", dummy ) ;
		comp(dummy) ;
		strcpy(VARADDR+offset ,dummy );
		(fld_hd+i)->title_off = offset ;
		offset += strlen(VARADDR+offset) + 1 ;

		/* Skip "REPEAT_SUPPRESS:" and get flag */
		fscanf(fom_fd,"%*s%s", dummy ) ;
		if(dummy[0] == 'Y')
			(fld_hd+i)->repeatsuppress = 1 ;
		else
			(fld_hd+i)->repeatsuppress = 0 ;

		/* Skip "JUSTIFICATION:" and get flag */
		fscanf(fom_fd,"%*s%s", dummy ) ;
		(fld_hd+i)->fld_justify = dummy[0] ;

		/* Skip "TOTALLING:" and get flag */
		fscanf(fom_fd,"%*s%s", dummy ) ;
		if(dummy[0] == 'Y')
			(fld_hd+i)->fld_totflag = 1 ;
		else
			(fld_hd+i)->fld_totflag = 0 ;

		/* Skip "CONSTRNT_MIN:" and get field */
		fscanf(fom_fd,"%*s%s", dummy ) ;
		comp(dummy) ;
		strcpy(VARADDR+offset ,dummy );
		(fld_hd+i)->minoff = offset ;
		offset += strlen(VARADDR+offset) + 1 ;

		/* Skip "CONSTRNT_MAX:" and get field */
		fscanf(fom_fd,"%*s%s", dummy ) ;
		comp(dummy) ;
		strcpy(VARADDR+offset ,dummy );
		(fld_hd+i)->maxoff = offset ;
		offset += strlen(VARADDR+offset) + 1 ;
	} /* for */

	for( i = 0 ; i < f_hd->tot_keys + f_hd->tot_pghdr ; i++) {
		fscanf(fom_fd,"%s", dummy) ;
		if(i >= f_hd->tot_pghdr) {
			if(strcmp(dummy,"SUBTOTAL_FLD:") != 0) {
		 	    sprintf(errmsg,"Not a SUBTOTAL Fld.. Log_Rec#: %d Format#: %d Fld#: %d",
			logrec, form_no, (i+1)) ;
			    prnterr(errmsg);
			    return(ERROR);
			}
		}
		else {
			if(strcmp(dummy,"PAGE_BREAK_FLD:") != 0) {
		 	    sprintf(errmsg,"Not a Page Break Fld.. Log_Rec#: %d Format#: %d Fld#: %d",
			logrec, form_no, (i+1)) ;
			    prnterr(errmsg);
			    return(ERROR);
			}
		}

		/* Get recno, fldno and name */
		fscanf(fom_fd,"%hd%hd%s", &recno,&fldno, dummy) ;

		recno--;
		fldno--;

		f_hd->keyno[i].inrecno  = formrecno[recno] ;
		f_hd->keyno[i].infieldno = fldno  ;
		strcpy(VARADDR+offset ,dummy );
		f_hd->keyoff[i] = offset ;
		offset += strlen(VARADDR+offset) + 1 ;
		
		f_hd->subserno[i] = FldSerno(formrecno[recno]) + fldno + 1;

		/* Skip "MESSAGE:" and get the message */
		fscanf(fom_fd,"%*s%s", dummy ) ;
		comp(dummy) ;
		strcpy(VARADDR+offset ,dummy);
		f_hd->msg_textoff[i] = offset ;
		offset += strlen(VARADDR+offset) + 1 ;

		/* Skip "EDIT_MASK:" and get the mask */
		fscanf(fom_fd,"%*s%s", dummy ) ;
		comp(dummy) ;
		strcpy(VARADDR+offset ,dummy);
		f_hd->keyeditoff[i] = offset ;
		offset += strlen(VARADDR+offset) + 1 ;
	}

	return(Wr_frmrec(name, logrec)) ;	
}
/*-----------------------------------------------------------------*/
/* Caluculate total fields up to previous structure in logical record */
FldSerno(recno)
int	recno ;
{
	INHDR	*hdr ;
	int	i, flds ;

	flds = 0 ;
	for(i = 0 ; i < recno ; i++) {
		hdr = (INHDR *)(Stru_buf[repnamerec.defstruct[i].strnum]) ;
		flds += hdr->stflds ;
	}

	return(flds) ;
}
/*-----------------------------------------------------------------*/
static char	*
comp(str)
char	*str ;
{
	int	i ;

	for(i = 0 ; i < strlen(str) ; i++) 
		if(str[i] == '~') str[i] = ' ' ;    /* tilde serves as blank */

	if(strcmp(str, " ")  == 0) str[0] = '\0' ;	/* NUll STRING */

	return(str) ;
}
/*-----------------------------------------------------------------*/
/** Open structure and format definition file	*/
/* read the specified format record 	*/

static int 
Rd_formrec(proj,logrec) 
char *proj ;
int   logrec ;
{
	char	filename[FILE_NAME_LEN] ;

	if(logrec < 1 ) {
		fprintf(err_fd,"Wrong logical record number %d\n",logrec);
		fprintf(err_fd,"Rd_formrec()..  Internal Error\n");
		return(ERROR) ;
	}

	strcpy(filename,proj) ;
	strcat(filename,REPNAME) ;	/* structure def. file	*/
	if((fdf = open(filename,RWMODE)) == -1){
		fprintf(err_fd,"%s file open error. errno: %d\n",
			filename, errno) ;
		return(ERROR) ;
	}

	if(lseek(fdf,(long)((logrec - 1) * sizeof(struct rp_name)), 0) < 0){
		fprintf(err_fd,"Lseek error in repname file.. errno: %d\n",
			errno);
		close(fdf);
		return(ERROR) ;
	}

	if(read(fdf,(char *)&repnamerec ,sizeof(struct rp_name)) !=
						sizeof(struct rp_name )) {
		fprintf(err_fd,"%s file read error.  errno: %d\n",
			filename, errno) ;
		close(fdf);
		return(ERROR) ;
	}

	return(repnamerec.formrecs+1) ;	/* return next format number */
}
/*-------------------------------------------------------------------*/

Wr_frmrec(proj, logrec) 
char	*proj ; 
int	logrec ;
{
	char	filename[FILE_NAME_LEN] ;
	int	i, cur_formno ;
	long	writeaddress ;
	int	fdfr ;

	strcpy(filename,proj) ;
	strcat(filename, FRMFILE) ;

	if ( access(filename, 0) < 0 ) {
		if((fdfr = creat(filename,CRMODE)) == -1){
			fprintf(err_fd,"%s file Create error. errno: %d\n",
				filename, errno) ;
			return(ERROR) ;
		}
		close(fdfr) ;
	}

	if((fdfr = open(filename,RWMODE)) == -1){
		fprintf(err_fd,"%s file open error. errno: %d\n",
				filename, errno) ;
		return(ERROR) ;
	}

	/** copy structure numbers */
	
	/** check the bound */
	if(f_hd->Numrecs < 1 || f_hd->Numrecs > MAXSTRUCT) {
		fprintf(err_fd,"Consistency error : Numbrecs: %hd\n",
			fldglob->Numrecs) ;
		return(ERROR) ;
	}

	if((writeaddress = lseek(fdfr,0L,2)) < 0) {
		fprintf(err_fd,"Wr_frmrec() Seek error  errno: %d\n", errno) ;
		return(ERROR) ;
	}

	/* write two format blocks */

	if(write(fdfr,frmbuf,FRMBUFSIZE) < FRMBUFSIZE) {
		fprintf(err_fd,"Format block write error  errno: %d\n", errno);
		return(ERROR) ;
	}

	repnamerec.formrecs += 1 ;
	repnamerec.formoff[repnamerec.formrecs - 1] =
					writeaddress / FRMBUFSIZE ;
	cur_formno = repnamerec.formrecs ;

	if(lseek(fdf,(long)((logrec-1) * sizeof(struct rp_name)),0) < 0) {
		fprintf(err_fd,"%hd file seek error errno\n",(logrec-1),errno) ;
		return(ERROR) ;
	}

	if(write(fdf,(char *)&repnamerec,sizeof(struct rp_name)) !=
						sizeof(struct rp_name)){
		fprintf(err_fd,"file write error\n") ;
		return(ERROR) ;
	}

	close(fdfr) ;
	return(NOERROR) ;
}


