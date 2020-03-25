/*
*       convdata.c
*
*       Program to convert field contents of given files from two byte word
*		aligned format to four byte word aligned format and vice versa.
*/

#define MAIN
#define MAINFL		-1

#define SYSTEM		"UTILITIES"
#define MOD_DATE	"8-FEB-91"

#include <stdio.h>
#include <bfs_defs.h>
#include <isnames.h>
#include <fld_defs.h>
#include <filein.h>

#define IFIELD   (i_big_buff+i_cur_fld->offset)
#define OFIELD   (o_big_buff+o_cur_fld->offset)

static  char    *i_big_buff = NULL ;
static  char    *o_big_buff = NULL ;

static  char    c_mesg[100];

static  int     file_no ;

static  Fld_hdr i_hdr ;
static  Fld_hdr o_hdr ;
static  Field   *i_field  = NULL ;
static  Field   *o_field  = NULL ;

char	save_path[50];

char	*mktemp() ;

main(argc, argv)
int     argc ;
char    *argv[] ;
{
        int     i, j, cur_file ;
        char    filenm[50] ;

        /*
        *       Initialize DBH
        */

	strncpy(SYS_NAME, SYSTEM, 50);
	strncpy(CHNG_DATE, MOD_DATE, 10);
        proc_switch(argc, argv, MAINFL) ;

        if ( dist_no[0] == '\0' )
                set_dist() ;/* Ask district# to position in data directory */

        if(init_dbh() < 0) {
                fprintf(stderr,"DBH Initialization ERROR.. DBERROR: %d\n",dberror);
		close_dbh() ;
                exit(-1) ;
        }

	i = GetUserClass(c_mesg);
	if(i < 0 || (i != SUPERUSER && i != ADMINISTRATOR)) {
		if(i == DBH_ERR)
			fprintf(stderr,"%s\n",c_mesg);
		else
			fprintf(stderr,"ACCESS DENIED");
		close_dbh() ;
		exit(-1);
	}

        /* Find out Max Reclen */
        j = 0 ;
        for(i = 0 ; i < TOTAL_FILES ; i++)
                if(j < getreclen(i)) j = getreclen(i) ;

	fprintf(stderr,"Debug\tMaximum output record length is: %d\n",j);

        o_big_buff = malloc((unsigned) j) ;
        if(o_big_buff == NULL) {
                fprintf(stderr,"Memory Allocation Error... size: %d\n", j);
		close_dbh() ;
                exit(-1);
        }

	/* Find out max reclen of old format of files.	*/
	/* First save the DATA_PATH variable	*/
	strcpy(save_path,DATA_PATH);

	/* Replace it with the FMT_PATH (this is where the old flddefs is) */
	strcpy(DATA_PATH,FMT_PATH);

	/* Find out the maximum record length	*/
	j = GetMaxRecLen(c_mesg);
	if(j < 0) {
		fprintf(stderr,"%s\n",c_mesg);
		close_dbh() ;
		exit(-1);
	}

	fprintf(stderr,"Debug\tMaximum input record length is: %d\n",j);

	/* Replace the original DATA_PATH	*/
	strcpy(DATA_PATH,save_path);

	/* Allocate the buffer	*/
        i_big_buff = malloc((unsigned) j) ;
        if(i_big_buff == NULL) {
                fprintf(stderr,"Memory Allocation Error... size: %d\n", j);
		close_dbh() ;
                exit(-1);
        }

        cur_file = -1 ;

        for( ; ; ) {
               	printf("\n\n\t\t\t    CONVERT FILE\n\n");

                /*
                *       Display File Names
                */

                if(TOTAL_FILES > 18)
                        j = TOTAL_FILES / 2 + TOTAL_FILES % 2 ;
                else
                        j = TOTAL_FILES ;

                printf("\t 0. QUIT\n");

                for(i = 0 ; i < j ; i++) {
                    getuserflnm(i,filenm);
                    printf("\t%2d. %-20.20s",(i+1),filenm) ;
                    if(j != TOTAL_FILES && (j+i) < TOTAL_FILES) {
                        getuserflnm(j+i,filenm);
                        printf("\t%2d. %s\n",(j+i+1),filenm);
                    }
                    else
                        printf("\n");
                }

                do {
                        printf("\n\t\tSelect File#: ");
                        scanf("%d",&file_no) ;
                } while (file_no < 0 || file_no > TOTAL_FILES) ;

                if(file_no == 0) break ;        /* QUIT */

                file_no-- ;

		i = CheckAccess(file_no,ADD,c_mesg);
		if(i < 0) {
			printf("%s\nPress RETURN to continue..",c_mesg);
			read(0,c_mesg,80) ;
			continue ;
		}

                getflnm(file_no, filenm) ;
                fprintf(stderr,"\n\t\t***  %s  ***  Data File Selected\n\n", filenm) ;


                if(cur_file != file_no) {
                        if(cur_file >= 0) {
                                if ( i_field != NULL ) {
                                        free((char*)i_field) ;
                                        i_field = NULL ;
                                }
                                if ( o_field != NULL ) {
                                        free((char*)o_field) ;
                                        o_field = NULL ;
                                }
                                close_file(cur_file) ;
                        }

			strcpy(save_path,DATA_PATH);
			strcpy(DATA_PATH,FMT_PATH);
                        if(GetFields(filenm, &i_hdr, &i_field, c_mesg) == ERROR) {
                                fprintf(stderr,"%s\n",c_mesg) ;
                                break ;
                        }
			strcpy(DATA_PATH,save_path);

                        if(GetFields(filenm, &o_hdr, &o_field, c_mesg) == ERROR) {
                                fprintf(stderr,"%s\n",c_mesg) ;
                                break ;
                        }
                        cur_file = file_no ;
                }

		fprintf(stderr,"Debug\tName of file: %s\n",i_hdr.filenm);
		fprintf(stderr,"Debug\tNumber of fields in input record: %d\n",i_hdr.no_fields);
		fprintf(stderr,"Debug\tLength of input record: %d\n",i_hdr.reclen);
		fprintf(stderr,"Debug\tNumber of fields in output record: %d\n",o_hdr.no_fields);
		fprintf(stderr,"Debug\tLength of output record: %d\n",o_hdr.reclen);
                ConvertFileToDBH(filenm);
        }

        if ( i_field != NULL ) 
                free((char*)i_field) ;
        if ( o_field != NULL ) 
                free((char*)o_field) ;
        if ( i_big_buff != NULL ) 
                free(i_big_buff);
        if ( o_big_buff != NULL ) 
                free(o_big_buff);

        close_dbh() ;
        exit(1);
}
/*--------------------------------------------------------------------*/
static  int
ConvertFileToDBH(filenm)
char    *filenm;
{
        int     fd, i, j;
        int     code;
        char    outfile[50];
	char	tempfile[50];
        int     reclen ;

        form_f_name(filenm,outfile);
	strcpy(tempfile,"CFXXXXXX");
	mktemp(tempfile);
#ifdef  MS_DOS
        rename(outfile,tempfile);
#else
        link(outfile,tempfile);
        unlink(outfile);        
#endif

        strcat(outfile,".IX");
	strcpy(c_mesg, tempfile) ;
        strcat(c_mesg,".IX");
#ifdef	MS_DOS
        rename(outfile,c_mesg);
#else
        link(outfile,c_mesg);
        unlink(outfile);
#endif

        fd = open(tempfile,0);
        if(fd <0) {
                fprintf(stderr,"error opening file >%s< errno: %d", tempfile, errno);
                return(-1);
        }
        i = 0;  /* initialize i to zero */
        reclen = i_hdr.reclen;
/*	fprintf(stderr,"Debug\tInput record length: %d\n",reclen);*/
        for( ; ; ){
#ifdef  STATUS_LEN
                if(read(fd,i_big_buff,STATUS_LEN) < STATUS_LEN) break;
                if(i_big_buff[0] == SET_DEL) {
                        if(read(fd,i_big_buff,reclen) < reclen) break;
                        continue ;
                }
#endif
                if(read(fd,i_big_buff,reclen) < reclen) break;
		
/*		fprintf(stderr,"Debug\tInput record:\n");
		for (j=0; j<i_hdr.reclen; j++)
			fprintf(stderr," %d",(int)i_big_buff[j]);
		fprintf(stderr,"\nDebug\tEnd of input record\n");
*/
		
		copy_fields();
		
/*		fprintf(stderr,"Debug\tOutput record:\n");
		for (j=0; j<o_hdr.reclen; j++)
			fprintf(stderr," %d",(int)o_big_buff[j]);
		fprintf(stderr,"\nDebug\tEnd of output record\n");
*/
		
                convert_fields();

/*		fprintf(stderr,"Debug\tWriting record: %d\n",i+1);*/
                if(getfiletype(file_no) == SEQ)
                        code = put_rec(o_big_buff,ADD,file_no,0,c_mesg) ;
                else
                        code = put_isrec(o_big_buff, ADD, file_no, c_mesg);

                if(code < 0){
                        fprintf(stderr,"\n** ERROR **  Code: %d Dberror: %d Iserror: %d Errno: %d\n\t%s\n",
                                        code,dberror,iserror,errno,c_mesg);
                        return(-1);
                }

                i++;
                if(i % 10 == 0)
                        if((code = commit(c_mesg))<0) {
                                fprintf(stderr,c_mesg);
                                break;
                        }
        }
        if(i % 10 != 0)
                if((code = commit(c_mesg))<0) {
                        fprintf(stderr,c_mesg);
                }

        close(fd) ;
	unlink(tempfile);
	strcpy(c_mesg, tempfile) ;
        strcat(c_mesg,".IX");
	unlink(c_mesg);
        return(0);
}
/*--------------------------------------------------------------------*/
static int
copy_fields()
{
        int     j ;
        Field   *i_cur_fld ;
        Field   *o_cur_fld ;

        i_cur_fld = i_field ;
        o_cur_fld = o_field ;

        for(j = 0 ; j < i_hdr.no_fields ; j++ ) {
                switch ( i_cur_fld->type ) {
                case T_CHAR : 
			copyit(IFIELD,OFIELD,i_cur_fld->len);
                        break;
                case T_FLOAT :
                        copyit(IFIELD,OFIELD,sizeof(float));
                        break ;
                case T_DOUBLE :
                        copyit(IFIELD,OFIELD,sizeof(double));
                        break ;
                case T_SHORT: 
                        copyit(IFIELD,OFIELD,sizeof(short));
                        break ;
                case T_INT: 
                        copyit(IFIELD,OFIELD,sizeof(int));
                        break ;
                case T_LONG :
                        copyit(IFIELD,OFIELD,sizeof(long));
                        break ;
                } /** switch ***/
/*		fprintf(stderr,"Field offsets --- Input: %d\tOutput: %d\n",i_cur_fld->offset,o_cur_fld->offset);*/
		i_cur_fld++;
		o_cur_fld++;
        } /* for {} */

        return(0);
}
/*--------------------------------------------------------------------*/
copyit(from,to,size)
char    *from;
char    *to;
int     size;
{
        int i;

        for (i=0; i<size ;i++)
		*to++ = *from++;
}       
/*--------------------------------------------------------------------*/
static int
convert_fields()
{
        int     j ;
        Field   *o_cur_fld ;

        o_cur_fld = o_field ;


        for(j = 0 ; j < o_hdr.no_fields ; j++ , o_cur_fld++ ) {

/*		fprintf(stderr,"\nDebug\t\tName of field: %s\n",o_cur_fld->name);
		fprintf(stderr,"Debug\t\tFormat of field: %s\n",o_cur_fld->format);
		fprintf(stderr,"Debug\t\tType of field: %c\n",o_cur_fld->type);
		fprintf(stderr,"Debug\t\tOffset of field: %d\n",o_cur_fld->offset);
		fprintf(stderr,"Debug\t\tLength of character field: %d\n",o_cur_fld->len);
*/
                switch ( o_cur_fld->type ) {
                case T_CHAR : 
                        break;
                case T_FLOAT :
                        convert(OFIELD,sizeof(float));
                        break ;
                case T_DOUBLE :
                        convert(OFIELD,sizeof(double));
                        break ;
                case T_SHORT: 
                        convert(OFIELD,sizeof(short));
                        break ;
                case T_INT: 
                        convert(OFIELD,sizeof(int));
                        break ;
                case T_LONG :
                        convert(OFIELD,sizeof(long));
                        break ;
                } /** switch ***/
        } /* for {} */

        return(0);
}
/*--------------------------------------------------------------------*/
convert(number,size)
char    *number;
int     size;
{
        int i, j;
        char dummy;

	char	*bytes;
/*	fprintf(stderr,"Debug\t\tByte order before:");
	for (i=0,bytes=number; i<size; i++,bytes++)
		fprintf(stderr,"\t%d",(int)*bytes);
	fprintf(stderr,"\n");
*/

        for(i=0,j=size-1;i<size && j>i;i++,j--) {
                number[i]^=number[j];
                number[j]^=number[i];
                number[i]^=number[j];
        }
/*	fprintf(stderr,"Debug\t\tByte order after:");
	for (i=0,bytes=number; i<size; i++,bytes++)
		fprintf(stderr,"\t%d",(int)*bytes);
	fprintf(stderr,"\n");
*/
}       
/*--------------------------------------------------------------------*/
/*
Function to read FLDEF file and get the maximum record length of all files.

Format:
	int	GetMaxRecLen(e_mesg)
	char	*e_mesg ;	* Error message will be returned in this *

Description:
	Reads the FLDDEF file and keeps track of the largest record length.

	Error message will be returned in e_mesg, if there is any error in
	FLDDEF file. e_mesg to be allocated in calling program to atleast
	80 char length.
*/

GetMaxRecLen(e_mesg)
char	*e_mesg ;
{
	Fld_hdr	header ;
	char	defs_file[50] ;
	int	fd ;
	int	maxrec=-1;

	strcpy(defs_file, DATA_PATH) ;
	strcat(defs_file, FLDDEF_FILE);

	if((fd=open(defs_file,RDMODE)) < 0){
#ifdef ENGLISH
		sprintf(e_mesg,"%s File Open Error...",defs_file);
#else
		sprintf(e_mesg,"Erreur ouverte au dossier %s...",defs_file);
#endif
		return(ERROR) ;
	}

	for( ; ; ) {
		if(read(fd,(char*)&header,sizeof(Fld_hdr)) < sizeof(Fld_hdr))
			break;
		if (header.reclen > maxrec)
			maxrec = header.reclen;

		/* Skip Fields information */
		if(lseek(fd,(long)(header.no_fields * sizeof(Field)),1) < 0){
#ifdef ENGLISH
			sprintf(e_mesg,"Lseek() ERROR on %s File",defs_file);
#else
			sprintf(e_mesg,"ERREUR Lseek() sur le dossier %s",defs_file);
#endif
			close(fd);
			return(ERROR);
		}
	}

	close(fd);

	return(maxrec) ;
}
/*--------------------E-N-D---O-F---F-I-L-E----------------------------*/
