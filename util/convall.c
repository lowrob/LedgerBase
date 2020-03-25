/*
*       convfile.c
*
*       Program to convert field contents of given files using flddefs file.
*               to build a new index from different systems.
*/

#define MAIN
#define MAINFL		-1

#define SYSTEM		"UTILITIES"
#define MOD_DATE	"6-FEB-90"

#include <stdio.h>
#include <bfs_defs.h>
#include <isnames.h>
#include <fld_defs.h>
#include <filein.h>

#define OFIELD  (o_big_buff+o_cur_fld->offset)
#define IFIELD	(i_big_buff+i_cur_fld->offset)

static  char    *i_big_buff = NULL ;
static	char	*o_big_buff = NULL;

static  char    c_mesg[100];

static  int     file_no ;
static	int	conv_type;

static  Fld_hdr hdr ;
static  Field   *i_field = NULL ;
static	Field	*o_field = NULL ;

char	*mktemp() ;

static	int ALIGN_char;
static	int ALIGN_short;
static	int ALIGN_int;
static	int ALIGN_long;
static	int ALIGN_double;
static	int ALIGN_struct;

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
                printf("DBH Initialization ERROR.. DBERROR: %d\n",dberror);
		close_dbh() ;
                exit(-1) ;
        }

	i = GetUserClass(c_mesg);
	if(i < 0 || (i != SUPERUSER && i != ADMINISTRATOR)) {
		if(i == DBH_ERR)
			printf("%s\n",c_mesg);
		else
			printf("ACCESS DENIED");
		close_dbh() ;
		exit(-1);
	}

        /* Find out Max Reclen */
/*
        j = 0 ;
        for(i = 0 ; i < TOTAL_FILES ; i++)
                if(j < getreclen(i)) j = getreclen(i) ;
*/
	/* set Max Reclen to 1K */
	j = 2048;

        i_big_buff = malloc((unsigned) j) ;
        if(i_big_buff == NULL) {
                printf("Memory Allocation Error... size: %d\n", j);
		close_dbh() ;
                exit(-1);
        }

        o_big_buff = malloc((unsigned) j) ;
        if(o_big_buff == NULL) {
                printf("Memory Allocation Error... size: %d\n", j);
		close_dbh() ;
                exit(-1);
        }

        cur_file = -1 ;

	if(SW1) {
		do {
			printf("\n\t0. 650 to 3000\n");
			printf("\n\t1. 3000 to 650\n\n");
			printf("Enter Option: ");
			scanf("%d",&conv_type);
		} while(conv_type < 0 || conv_type > 1);

		if(conv_type==0) {
			ALIGN_char   = 1;
			ALIGN_short  = 2;
			ALIGN_int    = 2;
			ALIGN_long   = 2;
			ALIGN_double = 2;
			ALIGN_struct = 2;
		}
		else {
			ALIGN_char   = 1;
			ALIGN_short  = 2;
			ALIGN_int    = 4;
			ALIGN_long   = 4;
			ALIGN_double = 4;
			ALIGN_struct = 4;
		}
	}

        for(i=0;i<TOTAL_FILES;i++) {
                    getuserflnm(i,filenm);
                    printf("\t%2d. %-20.20s\n",i,filenm) ;

		i = CheckAccess(i,ADD,c_mesg);
		if(i < 0) {
			printf("%s\nPress RETURN to continue..",c_mesg);
			read(0,c_mesg,80) ;
			continue ;
		}

                getflnm(i, filenm) ;

                if(cur_file != i) {
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

                        if(GetFields(filenm, &hdr, &o_field, c_mesg) == ERROR) {
                                printf("%s\n",c_mesg) ;
                                break ;
                        }
			if(SW1) {
				/* To reserve memory and copy new offsets to */
                        	if(GetFields(filenm,&hdr,&i_field,c_mesg)==ERROR){
	                                printf("%s\n",c_mesg) ;
					break ;
				}
			}
                        cur_file = i ;
                }

                ConvertFileToDBH(filenm);
        }

        if ( i_field != NULL ) 
                free((char*)i_field) ;
	if ( o_field != NULL) 
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
        int     fd;
        int     code;
        char    outfile[50];
	char	tempfile[50];
        int     reclen ;
	long	i;

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
                printf("error opening file  errno: %d", errno);
                return(-1);
        }
	/* Get OffSets  & Record Length */
	if(SW1) {
		reclen = GetDataOffSets() ;
	}	
	else {
	        reclen = getreclen(file_no) ;
	}
        i = 0;  /* initialize i to zero */
        for( ; ; ){
#ifdef  STATUS_LEN
                if(read(fd,i_big_buff,STATUS_LEN) < STATUS_LEN) break;
                if(i_big_buff[0] == SET_DEL) {
                        if(read(fd,i_big_buff,reclen) < reclen) break;
                        continue ;
                }
#endif
                if(read(fd,i_big_buff,reclen) < reclen) break;
                if(SW1) {
			copy_fields();
                        convert_fields();
			if(getfiletype(file_no) == SEQ)
				code = put_rec(o_big_buff,ADD,file_no,0,c_mesg);
			else
				code = put_isrec(o_big_buff,ADD,file_no,c_mesg);
		}
		else {
			if(getfiletype(file_no) == SEQ)
				code = put_rec(i_big_buff,ADD,file_no,0,c_mesg);
			else
				code = put_isrec(i_big_buff,ADD,file_no,c_mesg);
		}
                if(code < 0){
                        printf("\n** ERROR **  Code: %d Dberror: %d Iserror: %d Errno: %d\n\t%s\n",
                                        code,dberror,iserror,errno,c_mesg);
			getchar();
			if(code == DUPE) {
				continue;
			}
                        return(-1);
                }

                i++;
                if(i % 10 == 0)
                        if((code = commit(c_mesg))<0) {
                                printf(c_mesg);
                                break;
                        }
		printf("Records Converted: %d\n",i);
        }
        if(i % 10 != 0)
                if((code = commit(c_mesg))<0) {
                        printf(c_mesg);
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
GetDataOffSets()
{
        int     j ;
	int	i, f_cnt;

	int	i_reclen;

        Field   *i_cur_fld ;
	Field	*o_cur_fld ;

        i_cur_fld = i_field ;
	o_cur_fld = o_field;

	i_reclen = 0;

        for(j = 0 ; j < hdr.no_fields ; j++ , i_cur_fld++ , o_cur_fld++ ) {
                switch ( i_cur_fld->type ) {
                case T_CHAR : 
			SET_OFFSET(i_reclen,ALIGN_char); 
			i_cur_fld->offset = i_reclen;
			i_reclen += i_cur_fld->len;
                        break;
                case T_FLOAT :
			SET_OFFSET(i_reclen,ALIGN_double); 
			i_cur_fld->offset = i_reclen;
			i_reclen += sizeof(float);
                        break ;
                case T_DOUBLE :
			SET_OFFSET(i_reclen,ALIGN_double); 
			i_cur_fld->offset = i_reclen;
			i_reclen += sizeof(double);
                        break ;
                case T_SHORT: 
			SET_OFFSET(i_reclen,ALIGN_short); 
			i_cur_fld->offset = i_reclen;
			i_reclen += sizeof(short);
                        break ;
                case T_INT: 
			SET_OFFSET(i_reclen,ALIGN_int); 
			i_cur_fld->offset = i_reclen;
			i_reclen += sizeof(int);
                        break ;
                case T_LONG :
			SET_OFFSET(i_reclen,ALIGN_long); 
			i_cur_fld->offset = i_reclen;
			i_reclen += sizeof(long);
                        break ;
                } /** switch ***/
/***
printf("Input  Name: %s  Offset: %d\n",i_cur_fld->name,i_cur_fld->offset);
printf("Output Name: %s  Offset: %d\n",o_cur_fld->name,o_cur_fld->offset);
getchar();
***/
        } /* for {} */

	/* Check Offset Of Structure */
	SET_OFFSET(i_reclen,ALIGN_struct);
/***
printf("Input reclen: %d\n",i_reclen);
getchar();
***/
        return(i_reclen);
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

        for(j = 0 ; j < hdr.no_fields ; j++ ) {
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


        for(j = 0 ; j < hdr.no_fields ; j++ , o_cur_fld++ ) {

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
convert(number,size)
char    *number;
int     size;
{
        int i, j;
        char dummy;
        for(i=0,j=size-1;i<size && j>i;i++,j--) {
                number[i]^=number[j];
                number[j]^=number[i];
                number[i]^=number[j];
        /****
                dummy=number[i];
                number[i]=number[j];
                number[j]=dummy;
        ****/
        }
}       
/*--------------------E-N-D---O-F---F-I-L-E----------------------------*/

