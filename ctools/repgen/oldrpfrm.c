/*
*	rpfrom.c
*
*	Program to create REP-GEN .RPF and .RPN files from .RFM file
*/

#include <stdio.h>
#include "repname.h"
#include "struct.h"
#include "rep.h"


#define CHKERROR(X)	if(X == ERROR) return(ERROR)
int	rperror ;

#define	reterr(X)	{ rperror = X ;\
			printf("Internal data error occurred.. Rperror: %hd\n",\
			rperror) ;\
			return(ERROR) ; }


#define fldhd	f_attri
#define fhd	f_global
#define fld_hd	fldattri
#define f_hd	fldglob


static  int	fdf ;
static int	fdfr ;
static	struct	rp_name repnamerec ;
FILE	*fopen(), *fom_fd ;

 int
creat_logrec() 
{
int	i ;
short	formats ;
int	logrec ;

	fscanf(fom_fd, "%*s%d%*s%s", 
					&logrec, repnamerec.rep_name) ;
	if ( logrec == 0 ) return(ERROR) ;

	fscanf(fom_fd, "%*s%hd", &repnamerec.numstruct );
	fscanf(fom_fd, "%*s");
	for(i=0; i<repnamerec.numstruct ;i++)
		fscanf(fom_fd, "%hd%s", &repnamerec.defstruct[i].strnum,
					repnamerec.defstruct[i].strname) ;
	fscanf(fom_fd, "%*s%hd", &formats) ;
	
	repnamerec.formrecs = 0 ;

	return(formats) ;

}
		
Add_logrec(proj, logrec) 
char	*proj ;
int	logrec ;
{
	char	filename[FILE_NAME_LEN] ;

	strcpy(filename,proj) ;
	strcat(filename,REPNAME) ;	/* structure def. file	*/

	if ( logrec == 1 || access(filename, RWMODE) < 0 ) {
		if ((fdf=creat(filename, CRMODE)) == -1 ) {
			fprintf(stderr,"%s file creatread error\n",filename) ;
			fprintf(stderr,"Module : rp.c. Func : Rd_formrec\n") ;
			return(OPENERR) ;
		}
		close(fdf) ;

		strcpy(filename,proj) ;
		strcat(filename,FRMFILE) ;	/* Formats file	*/
		unlink(filename) ;
	}

	strcpy(filename,proj) ;
	strcat(filename,REPNAME) ;	/* structure def. file	*/

	if((fdf = open(filename,RWMODE)) == -1){
		fprintf(stderr,"%s file open-read error\n",filename) ;
		fprintf(stderr,"Module : rp.c. Func : Rd_formrec\n") ;
		return(OPENERR) ;
	}

	if(logrec < 1 ) {
		fprintf(stderr,"Wrong logical record number\n");
			fprintf(stderr,"Module : rp.c . Func : Rp_formrec\n");
			return(INTERR) ;
	}
	if(lseek(fdf,(long)((logrec - 1) * sizeof(struct rp_name)), 0) < 0){
		fprintf(stderr,"Lseek error in repname file\n");
		fprintf(stderr,"Module : rp.c . Func : Rd_formrec\n");
		return(SEEKERR) ;
	}
	if(write(fdf,(char *)&repnamerec ,sizeof(struct rp_name)) !=
						sizeof(struct rp_name )) {
		fprintf(stderr,"%s file writeerror\n",filename ) ;
		fprintf(stderr,"Module : rp.c . Func :Rd_formrec\n") ;
		return(READERR) ;
	}
	return(0) ;
}


main(argc, argv)
int	argc ;
char	*argv[] ;
{
char	proj[30] ;
int	log_recno, form_no ;
char	fom_file[FILE_NAME_LEN];
int	formats ;

printf(" Project Name :");
scanf("%s", proj) ;

strcpy(fom_file, proj) ;
strcat(fom_file, FOMFILE) ;

if ( (fom_fd = fopen(fom_file, "r")) == NULL ) {
	printf("File %s open error \n");
	exit(-1) ;
}

strcpy(fom_file, proj) ;
strcat(fom_file, FRMFILE) ;
if(access(fom_file,0) >= 0) {
	printf("%s File Exists.. Can't Create\n", fom_file);
	fclose(fom_fd) ;
	exit(-1);
}

for(log_recno = 1;;){
 	formats = creat_logrec() ;
	if ( formats < 0 ) break ;
	if ( formats == 0 ) continue ; 
	Add_logrec(proj, log_recno) ;

	for(form_no=1; form_no <= formats ; form_no++) 
		creat_frmat(proj, log_recno);
	log_recno++ ;

	}

	fclose(fom_fd) ;

	printf(" %d Logical records Converted \n", log_recno-1) ;

}

 int
creat_frmat(name, logrec)
char	*name ;
int	logrec ;
{
short	recno , fldno, i, j ;
char	*comp() ;	/* compressing blanks */
int	form_no ;
char	dummy[80] ;
int	offset ;
char	s[10][10] ;

	f_hd = (struct fhd *)(frmbuf) ;
	fld_hd = (struct fldhd *)(f_hd + 1) ; 

	CHKERROR((form_no=Rd_formrec(name,logrec))) ; 

	fscanf(fom_fd,"%*s%s%*s%hd", f_hd->formname, &f_hd->Numrecs);

	for(j=0;j<strlen(f_hd->formname); j++) 
			if ( f_hd->formname[j] == '~' ) f_hd->formname[j] = ' ';

	fscanf(fom_fd,"%*s", dummy);

	for(i=0; i <f_hd->Numrecs; i++ ) { 
		fscanf(fom_fd,"%hd",&f_hd->Inrecno[i] ) ; 
		}

	fscanf(fom_fd,"%*s", dummy);

	for(i=0; i <f_hd->Numrecs; i++ ) { 
		fscanf(fom_fd,"%*s", dummy) ;
		}

	
	fscanf(fom_fd,"%*s%hd%*s%hd%*s%1s%*s%hd", &f_hd->page_lines, 
			&f_hd->page_columns, s[0], &f_hd->mhdnos);
	f_hd->linespace = s[0][0] ;
	printf("%hd  %hd  %c   %hd", f_hd->page_lines, 
			f_hd->page_columns, f_hd->linespace, f_hd->mhdnos);
	f_hd->linespace -= '0' ;
	
	fscanf(fom_fd,"%*s%hd%*s%hd%*s%hd",
		&f_hd->tot_flds, &f_hd->tot_keys, &f_hd->tot_pghdr );
	printf("Total fields : %hd   %hd   %hd\n",
		f_hd->tot_flds, f_hd->tot_keys, f_hd->tot_pghdr );
	 
	offset = 0 ;
	for( i = 0 ; i < f_hd->mhdnos ; i++) {
		f_hd->mhdoff[i] = offset ;
		fscanf(fom_fd,"%s", VARADDR+ f_hd->mhdoff[i] ) ;
		for(j=0; j<strlen(VARADDR+f_hd->mhdoff[i]) ;j++)
			if ( *(VARADDR+f_hd->mhdoff[i]+j) == '~' )  
				*(VARADDR+f_hd->mhdoff[i]+j) =  ' ' ; 
		offset += strlen(VARADDR + f_hd->mhdoff[i]) + 1 ;
		}

	for( i = 0 ; i < f_hd->tot_flds ; i++) {
		fscanf(fom_fd, "%1s%1s%hd%1s%1s%1s%hd",
						s[0],
						s[1],
						&(fld_hd+i)->fld_size, 
						s[4],
						s[2],
						s[3], 
				 		&(fld_hd+i)->fld_serno) ;

		(fld_hd+i)->fld_class = s[0][0] ;
		(fld_hd+i)->fld_type = s[1][0] ;
		(fld_hd+i)->fld_justify = s[2][0] ;
		(fld_hd+i)->fld_totflag = s[3][0]  - '0';
		(fld_hd+i)->repeatsuppress = s[4][0] - '0' ; 

		printf(  " Field Info :%c  %c   %hd   %c    %c %c  %hd\n",
						(fld_hd+i)->fld_class ,
						(fld_hd+i)->fld_type ,
						(fld_hd+i)->fld_size, 
						(fld_hd+i)->repeatsuppress, 
						(fld_hd+i)->fld_justify,
						(fld_hd+i)->fld_totflag ,
				 		(fld_hd+i)->fld_serno) ;




		if ( (fld_hd+i)->fld_class == INP_FLD ) {
			fscanf(fom_fd,"%hd%hd", &recno, &fldno) ;
			printf("Recno : %hd fieldno: %hd", recno, fldno) ;
			(fld_hd+i)->in_takeno.fieldref.inrecno = recno ;
			(fld_hd+i)->in_takeno.fieldref.infieldno = fldno ;
			fscanf(fom_fd,"%*s", dummy);
		} 
		else { 
			(fld_hd+i)->in_takeno.comp_off[0] = offset ;
			fscanf(fom_fd,"%s", dummy ) ;
			for(j=0;j<strlen(dummy); j++) 
				if ( dummy[j] == '~' ) dummy[j] = ' ' ;
			printf(" COmpute formula :%s", dummy) ;
			strcpy(VARADDR+offset , dummy+strlen("Formula:")) ;
			offset += strlen(VARADDR+offset)  + 1;
			(fld_hd+i)->in_takeno.comp_off[1] = offset ;
			fscanf(fom_fd,"%s", dummy ) ;
			for(j=0;j<strlen(dummy); j++) 
				if ( dummy[j] == '~' ) dummy[j] = ' ' ;
			strcpy(VARADDR+offset ,dummy+strlen("Rever_Polish:"));
			offset += strlen(VARADDR+offset)  + 1;
		}

	/****	Get Edit Mask *****/

		fscanf(fom_fd,"%s", dummy ) ;
		printf(" Edit Mask ?:%s\n", dummy) ;
		for(j=0;j<strlen(dummy); j++) 
				if ( dummy[j] == '~' ) dummy[j] = ' ' ;
		strcpy(VARADDR+offset ,dummy+strlen("Edit_Mask:") );
		(fld_hd+i)->edit_off = offset ;
		offset += strlen(VARADDR+offset) + 1 ;

	/****	Get COlumn Name ****/

		fscanf(fom_fd,"%s", dummy ) ;
		for(j=0;j<strlen(dummy); j++) 
				if ( dummy[j] == '~' ) dummy[j] = ' ' ;
		strcpy(VARADDR+offset ,dummy+strlen("Column_Name:") );
		(fld_hd+i)->title_off = offset ;
		offset += strlen(VARADDR+offset) + 1 ;

	/****	Get Field Name ****/

		fscanf(fom_fd,"%s", dummy ) ;
		for(j=0;j<strlen(dummy); j++) 
				if ( dummy[j] == '~' ) dummy[j] = ' ' ;
		strcpy(VARADDR+offset ,dummy+strlen("Field_Name:") );
		(fld_hd+i)->fname_off = offset ;
		offset += strlen(VARADDR+offset) + 1 ;

	/****	Initialise constraints to NULL ..(May do later) ****/

		(fld_hd+i)->minoff = offset ;
		*(VARADDR+offset) = '\0' ;
		offset += 1 ;
		(fld_hd+i)->maxoff = offset ;
		*(VARADDR+offset) = '\0' ;
		offset += 1 ;
		printf(" Offset Value :%d tot :%d\n", offset, f_hd->tot_flds ) ;

	} /* for */


	for( i = 0 ; i < f_hd->tot_keys + f_hd->tot_pghdr ; i++) {
		fscanf(fom_fd,"%hd%hd%hd", &recno,&fldno, &f_hd->subserno[i]) ;
		printf("Subtotal : %hd  %hd  %hd", recno,fldno, f_hd->subserno[i]) ;
		f_hd->keyno[i].inrecno  = recno ;
		f_hd->keyno[i].infieldno = fldno  ;
		/*** 4-oct-89 by amar
		fscanf(fom_fd,"%*s", dummy) ;
		****/

		/*** 4-oct-89 by amar **/
		fscanf(fom_fd,"%s", dummy) ;
		f_hd->keyoff[i] = offset ;
		strcpy(VARADDR+offset ,dummy);
		offset += strlen(VARADDR+offset) + 1 ;
		/****/
		
		fscanf(fom_fd,"%s", dummy ) ;
		printf(" Message?: %s\n", dummy) ;

		for(j=0;j<strlen(dummy); j++) 
				if ( dummy[j] == '~' ) dummy[j] = ' ' ;
		strcpy(VARADDR+offset ,dummy+strlen("Message:") );
		f_hd->msg_textoff[i] = offset ;
		offset += strlen(VARADDR+offset) + 1 ;
		fscanf(fom_fd,"%s", dummy ) ;
		printf(" Edit Mask? %s\n", dummy) ;
		for(j=0;j<strlen(dummy); j++) 
				if ( dummy[j] == '~' ) dummy[j] = ' ' ;
		strcpy(VARADDR+offset ,dummy+strlen("Edit_Mask:"));
		f_hd->keyeditoff[i] = offset ;
		offset += strlen(VARADDR+offset) + 1 ;
		/*** 4-oct-89 by amar
		f_hd->keyoff[i] = offset ;
		*(VARADDR+offset) = '\0' ;
		offset += 1 ;	
		****/
	}
	Wr_frmrec(name, logrec) ;	
 }

 char *
comp(str)
char	*str ;
{
int	i ;

for(i = 0 ; i < strlen(str) ; i++) 
	if ( str[i] == ' ') str[i] = '~' ;	/* tilde serves as blank */
return(str) ;
}

/** Open structure and format definition file	*/



/* read the specified format record 	*/
static int 
Rd_formrec(proj,logrec) 
char *proj ;
int   logrec ;
{
	char	filename[FILE_NAME_LEN] ;

	strcpy(filename,proj) ;
	strcat(filename,REPNAME) ;	/* structure def. file	*/
	if((fdf = open(filename,RWMODE)) == -1){
		fprintf(stderr,"%s file open-read error\n",filename) ;
		fprintf(stderr,"Module : rp.c. Func : Rd_formrec\n") ;
		return(OPENERR) ;
	}

	if(logrec < 1 ) {
		fprintf(stderr,"Wrong logical record number\n");
			fprintf(stderr,"Module : rp.c . Func : Rp_formrec\n");
			return(INTERR) ;
	}
	if(lseek(fdf,(long)((logrec - 1) * sizeof(struct rp_name)), 0) < 0){
		fprintf(stderr,"Lseek error in repname file\n");
		fprintf(stderr,"Module : rp.c . Func : Rd_formrec\n");
		return(SEEKERR) ;
	}
	if(read(fdf,(char *)&repnamerec ,sizeof(struct rp_name)) !=
						sizeof(struct rp_name )) {
		fprintf(stderr,"%s file read error\n",filename ) ;
		fprintf(stderr,"Module : rp.c . Func :Rd_formrec\n") ;
		return(READERR) ;
	}
	printf(" repname :%s numstr :%hd formrecs :%d\n", repnamerec.rep_name,
		repnamerec.numstruct, repnamerec.formrecs );

return(repnamerec.formrecs+1) ;	/* return next format number */

}



 Wr_frmrec(proj, logrec) 
char	*proj ; 
int	logrec ;
	{
	char	filename[FILE_NAME_LEN] ;
	int i, cur_formno ;
	long writeaddress ;

	strcpy(filename,proj) ;
	strcat(filename, FRMFILE) ;

Open_fl :
	if ( access(filename, RWMODE) == 0 ) {
		if((fdfr = open(filename,RWMODE)) == -1){
			fprintf(stderr,"%s file open-write error\n",filename) ;
			fprintf(stderr,"Module : rp.c. Func : Wr_formrec\n") ;
			return(OPENERR) ;
		}
	}
	else {
		if((fdfr = creat(filename,CRMODE)) == -1){
			fprintf(stderr,"%s file Cret-write error\n",filename) ;
			fprintf(stderr,"Module : rp.c. Func : Wr_formrec\n") ;
			return(OPENERR) ;
			}
		close(fdfr) ;
		goto Open_fl ;
	}

	/** copy structure numbers */
	
	/** check the bound */
	if(f_hd->Numrecs < 1 || f_hd->Numrecs > MAXSTRUCT) {
		fprintf(stderr,"Consistency error : Numbrecs: %hd\n",
			fldglob->Numrecs) ;
		fprintf(stderr,"Module:repform; Func: Write_curform\n");
		return(INTERR) ;
	}


#ifdef DEBUG
		fprintf(stderr,"New format to be added\n") ;
#endif
	if((writeaddress = lseek(fdfr,0L,2)) < 0) {
		fprintf(stderr,"Seek error :\n") ;
		fprintf(stderr,"Module: repform. Func:Write_curform\n");
		return(SEEKERR) ;
		}


	/* write two format blocks */

	if(write(fdfr,frmbuf,FRMBUFSIZE) < FRMBUFSIZE) {
		fprintf(stderr,"format block write error\n");
		fprintf(stderr,"Module : form. Func : Write_curform\n");
		return(WRITEERR) ;
		}

	printf(" repname :%s numstr :%hd formrecs :%d\n", repnamerec.rep_name,
		repnamerec.numstruct, repnamerec.formrecs );
	repnamerec.formrecs += 1 ;
	repnamerec.formoff[repnamerec.formrecs - 1] =
					writeaddress / FRMBUFSIZE ;
	printf("repstruct recs :%d offset :%d\n", repnamerec.formrecs ,
			repnamerec.formoff[repnamerec.formrecs-1] );

	cur_formno = repnamerec.formrecs ;

	if(lseek(fdf,(long)((logrec-1) * sizeof(struct rp_name)),0) < 0) {
		fprintf(stderr,"%hd file seek error\n",(logrec-1)) ;
		fprintf(stderr,"Module : form. Func : Write_curform\n");
		return(SEEKERR) ;
		}

	if(write(fdf,(char *)&repnamerec,sizeof(struct rp_name)) !=
						sizeof(struct rp_name)){
		fprintf(stderr," file write error\n") ;
		fprintf(stderr,"Module : form. Func : Write_curform\n");
		return(WRITEERR) ;
		}

		
	return(NOERROR) ;
}


