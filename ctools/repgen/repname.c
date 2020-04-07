/**
File Name : repname.c

Written by : V.SUBRAMANI.

Usage : Used to define output records (in terms of primitive structures)
	These output records will be used to define formats, before generating
	the reports.

Date :  27 July 1987.

Compilation : cc repname.c -lfom win.o -o repname

Execution : repname projcode outputrecord#

Description :  Unlimitted number of logical (output) records can be defined for 		a project. To display already existing logical record, <Esc>H, 
		in which case output record names are displayed in a window.
		You can change already existing record, by selecting the 
		corresponding serial number. To introduce new output record,
		directly enter the record desciptions.

		Primitive structure names can also be displayed in a window, by
		typing <Esp>H and selection can be typing the serial number

Modification History :

4-Jan-90  Amar : Deletion of Logical Record is suppressed. This is not
		implemented completely.
*/


#include <stdio.h>
#include <cfomstrc.h>     /*  STATUS RECORD HEADER  */
#include "rep.h"
#include "struct.h"
#include "repname.h"
#include <cfomtm.h>
#include <cfomtcr.h>

#define REPSCREEN "report"
#define MAX_STR 5	/* Max. #of structures allowed per output */

#define YES 1
#define NO  0

/** Screen Escape return values	*/

#define	HELP	-10	/* to display the help window 	*/
#define DELETE	-20	/* Delete current field value	*/
#define PREV 	-30	/* go to previous field	*/
#define EXIT	-40	/* Go to continue field		*/

/* define the field names in the screen	*/
#define SCRN_START	100
#define REPT_NAME	300
#define STRU_START	500
#define STRU_END	900
#define STRN_START	1000
#define STRN_END	1400
#define CONT_FLAG	1500	
#define SCRN_END	1500


/* screen def. structure */

struct scr_struct {
	char rept_name[NAME_LEN] ;	/* report name	*/
	short stru_num[MAX_STR] ; 	/* user selected struct. numbers */
	char stru_name[MAX_STR][21] ; 	/* struc. name defined in header */
	int   cont_flag ; 		/* Boolean  */
} ;

static struct	stat_rec  sr ;   	/*  PROFOM status record */
static struct 	scr_struct screen ;	/* Screen Image buffer 	*/
static struct 	rp_name rpnambuf ;	/* report name buffer - definied in repname.h	*/


static short cur_rec;		/* Current report name rec. displayed	*/

char *malloc() ;		/* memory allocator */

/** file fds	*/

static int fdr;	/* rep. name file fd	*/
static int fds;	/* structure def. file fd	*/


/* structure name list	*/
static struct st_list {
	struct 	str_def	strc ;		/* defined in repname.h	*/
	struct st_list *next ;
	} ;
static 	struct st_list *strlist = NULL ; /* Structure name list address */
static short availstruct ;	/* available #of structures	*/

main(argc,argv)
int argc ;
char **argv;
{
	char *nfmdir;

	if(argc < 3) 
		printf("Usage : repname proj termnm\n"), exit(1) ;
	
	nfmdir = getenv("CTOOLS");/* Added to get the path L.R*/
	if(nfmdir == NULL){
#ifdef ENGLISH
		printf("\"CTOOLS\" environment variable is not exported\n");
#else
		printf("\"CTOOLS\"Variable de l'environnement n'est pas exportee\n");
#endif
		exit(-1) ;
	}
/* NFM_PATH has been commented out in repname.h to avoid hard coding of the path L.R
	strcpy(sr.scrnam,NFM_PATH) ; added two lines to replace */
	strcpy(sr.scrnam,nfmdir) ;
	strcat(sr.scrnam,"/") ;

	strcat(sr.scrnam,REPSCREEN) ;
	strcpy(sr.termnm,argv[2]) ;
	fomin(&sr) ;	
	chkerror() ;

	Set_dupbuf() ;
	
	Open_files(argv[1]) ;
	Read_strdef() ;	/** read structure def. and create the list	*/

	for(; ;) {
		Proc_screen() ;
		sr.nextfld = CONT_FLAG ;
		fomrf((char *)&screen) ;
		chkerror() ;
		if(screen.cont_flag == BOOL_NO)
			break ;
	}

	fomcs() ;
	fomrt() ;
	Pr_repnames() ; /* display all available report names */
	Close_files() ;
	exit(1) ;
}



/** Extract the string before the '.' and append with '.RPNM' to get the  */
/* report name file							  */


Open_files(infile)
char *infile ;
{
	char Rpname[FILE_NAME_LEN] ;
	char Stname[FILE_NAME_LEN] ;

	strcpy(Rpname,infile) ;
	strcat(Rpname,REPNAME) ;

	strcpy(Stname,infile) ;
	strcat(Stname,STRFILE) ;


	if(access(Rpname,RDMODE) < 0) 	/* Rpname file not available */
		if((fdr = creat(Rpname,CRMODE)) < 0) 
			fprintf(stderr,"Creation Error : %s file \n",Rpname),
			abexit(-50) ;
		else
			close(fdr) ;

	if((fdr = open(Rpname,RWMODE)) < 0) 
		fprintf(stderr,"Open error with appendmode : %s \n",Rpname) ,
		abexit(-30) ;

	/* open structure definition file	*/

	if((fds = open(Stname,RDMODE)) < 0)
		fprintf(stderr,"%s file read open error \n",Stname), 
		abexit(-25) ;
	return(NOERROR) ;
}




/** Close Rpname and structure def. files	*/

Close_files()
{
	close(fds) ;
	close(fdr) ;
	return(NOERROR) ;
}



/** From structure def. file , read all structure names and the numbers */

Read_strdef()		
{
	char 	strblk[STRBUFSIZE] ;	/* structure to read structure def. */
	struct st_list *curptr ;
	INHDR *strheader ;
	strheader = (INHDR *) (strblk) ;

	availstruct = 0 ;
	if(read(fds,strblk,STRBUFSIZE) < STRBUFSIZE)
		fprintf(stderr,"Structure def. file is empty \n"), abexit(-40) ;

	curptr = strlist = (struct st_list *) malloc(sizeof(struct st_list)) ;

	if(curptr == NULL)
		fprintf(stderr,"Memory allocation error \n") , abexit(-20) ;

	curptr->strc.strnum = availstruct++ ;
	strcpy(curptr->strc.strname,strheader->stname) ;
	curptr->next = NULL ;

	for(; ;){	/* read remaining recs. and create the list */
		if(read(fds,strblk,STRBUFSIZE) < STRBUFSIZE) 
			break ;
		/* fprintf(stderr,"strnum :%2.2d,stname :%s \n",availstruct,strheader->stname) ; **/

		curptr->next = (struct st_list *) malloc(sizeof(struct st_list)) ;
		if(curptr->next == NULL )
			fprintf(stderr,"Memory allocation error \n"), abexit(-20) ;
		curptr = curptr->next ;
		curptr->strc.strnum = availstruct++ ;
		strcpy(curptr->strc.strname,strheader->stname) ;
		curptr->next = NULL ;
	}

	return(NOERROR) ;
}

#define NEW_RECORD	'n'	/* New record to be added	*/
#define NEXT_PAGE	'p'	/* Display the next page	*/
#define NOT_SELECT	'n'	/* Skip the selection	*/
#define STR_SELECT	's'	/* str. record has been selected */

	

	/****************************************************************/
	/*								*/
	/* Process the screen and store the data - repname records	*/
	/* Three esc functions are provided in the mask	 fields		*/
	/* HELP - to display available structure names / report names	*/
	/*	  in a window						*/
	/* DELETE - to delete a structure name under a report name	*/
	/* EXIT - Abort the editing of the current report name record 	*/
	/* goto continue field, Current update not saved is done.	*/
	/*								*/
	/****************************************************************/



Proc_screen()
{
	int done = 0 ;	/* till done = 0 read this field	*/
	int numitems ;	/* # of structure names User has entered */
	int retcode ;	/* value returned by profom calls	*/
	int i,
	    j ;
	struct st_list *temptr ;

	char retchar ;

	init_screen() ;		/* put blank page	*/

	/* read and display repname	*/

	cur_rec = 0 ;	/* No repname record read	*/
	rpnambuf.numstruct = 0 ;
	
	for(;;) {
		sr.nextfld = REPT_NAME ;
		sr.endfld = REPT_NAME ;

		sr.fillcode = FIL_PARTLY ;
		fomrf((char *)&screen) ;
		retcode = chkerror() ;
		switch(retcode) {

		case HELP :
			ShowRepnames() ;
			
			
			/* User selected report name will be displayed in the current field */
		break ;

#ifdef	DELETE_LOGREC
	/* This is not implemented completely, temporarily suppressed by AMAR
	   on 4-JAN-90 */
		case DELETE :

			/* Check if some report name is displayed ;	*/
			/* Time being suppress this option,  currently while reading	*/
			/* rep. name records deleted records should be ignored - this is */
			/* not done.				*/

			fomen("Confirm : Type Y to Delete / N to ignore ") ;
			retchar = get() ;
			if(retchar == 'Y' || retchar == 'y') {
				DEL_reprec() ;
				break ;
			}

#endif
		case EXIT :
			return(NOERROR) ;
		}

		if(strlen(screen.rept_name) == 0) {
#ifdef	DELETE_LOGREC
			fomen("Type <CR>/<Esc> followed by H(elp)/D(elete)/E(xit)") ;
#else
			fomen("Type <CR>/<Esc> followed by H(elp)/E(xit)") ;
#endif
			continue ;
		}

		if(sr.fillcode == FIL_DUP)
			break ;
		sr.endfld = sr.nextfld = REPT_NAME ;
		fomwr((char *)&screen) ;
		chkerror() ;
		fomud((char *)&screen) ;
		chkerror() ;
		
	}

	/* now read the structure names	*/

	done = 0 ;
	for(i = 0, sr.nextfld = STRU_START; (sr.nextfld <= STRU_END) && (!done) ; i++,sr.nextfld += 100) {
		sr.fillcode = FIL_PARTLY ;
		fomrf((char *)&screen) ;
		retcode = chkerror() ;
		switch(retcode) {
		case HELP:
			/* User may want to rewrite the structure name with the one */
			/* he is going to select or Enter new structure name		*/

			if(Disp_strwindow(i) == NOT_SELECT){
				sr.nextfld -= 100; i-- ;
				continue ;
			}
			sr.nextfld -= 100; i-- ;
			/* read the same field once again	*/
		break ;

		case DELETE :

			/* check if any field name is displayed or not 	*/
			/* whenever rpnambuf does not contain image of the actual rec. */

			/* otherwise scroll the bottom	items to the top */
			if(screen.stru_num[i] == 0 || screen.stru_num[i] == LV_SHORT) 
				fomen("No data available ") ;
			else {
				for(j = i; (j < MAX_STR - 1) && screen.stru_num[j + 1] != HV_SHORT ; j++) {
					screen.stru_num[j] = screen.stru_num[j + 1] ;
					strcpy(screen.stru_name[j],screen.stru_name[j + 1]) ;
				}
				screen.stru_num[j] = LV_SHORT ;
				sr.endfld = STRN_END ; /* fill 0 */
				fomud((char *)&screen) ;
				chkerror() ;	
				screen.stru_num[j] = HV_SHORT ;
				screen.stru_name[j][0] = HV_CHAR ; 
				sr.endfld = STRN_END ;
				fomwr((char *)&screen) ;
				chkerror() ;
				fomud((char *)&screen) ;
				chkerror() ;
				
			}
			i--;
			sr.nextfld = STRU_START + i * 100 ;
						/* read the same field */
		break ;		

		case PREV :
			if(sr.nextfld == STRU_START) {
				fomen("Field backword not allowed") ;
				sr.nextfld -= 100 ; i-- ;
			}
			else {
				sr.nextfld -= 200 ; i -= 2 ;
			}
			continue ;


		case EXIT :
			return(NOERROR) ;

		default :
			if( screen.stru_num[i] == 0 || screen.stru_num[i] == LV_SHORT) {
				done = 1 ;
				for(j = i; j < MAX_STR ; j++) {
					screen.stru_num[j] = HV_SHORT;
					screen.stru_name[j][0] = HV_CHAR ;
				}
				sr.endfld = STRN_END ;
				fomwr((char *)&screen) ;
				chkerror() ;
				i-- ; /* In all the cases `i` will get incremented, and */
						/* it will tell #of str. names available */
			}
			if(sr.fillcode == FIL_DUP)
				continue ;

			/* user has typed a struc. #, verify and display */

			if(screen.stru_num[i] > availstruct) {
				fomer("Invalid Structure number! Try again") ;
				screen.stru_num[i] = 0 ;
				sr.endfld = sr.nextfld ;
				fomwf((char *)&screen) ;
				chkerror() ;
				fomud((char *)&screen) ;
				chkerror() ;
				sr.nextfld -= 100 ; i-- ;
				continue ;
			}

			/* check if it has already been entered	*/
			for(j = 0 ; j < i ; j++)
				if(screen.stru_num[j] == screen.stru_num[i])
					break ;

			if(j < i) {	/* str.# already exists	*/
				fomer("This structure has already been selected") ;
				sr.nextfld -= 100 ; i-- ;
				continue ;
			}

			/* display the structure name	*/
			for(j = 1,temptr = strlist ; (j < screen.stru_num[i]) && (temptr != NULL ); j++,temptr = temptr->next ) ;
			if(temptr == NULL) 
				fprintf(stderr,"Inconsistency error : stru_num : % \n",screen.stru_num[i]) ,
				fprintf(stderr,"Module : repname . Func : Proc_screen \n") ,
				abexit(INTERR) ;
				
			strcpy(screen.stru_name[i],temptr->strc.strname) ;
			sr.endfld = sr.nextfld ;
			fomud((char *)&screen) ;
			chkerror() ;
			sr.endfld = sr.nextfld = STRN_START + i * 100 ;
			fomwr((char *)&screen) ;
			chkerror() ;
			fomud((char *)&screen) ;
			chkerror() ;
			sr.nextfld = STRU_START + i * 100 ;
			/* read once again	*/
		
			sr.nextfld -= 100 ; i-- ;				
		break ;			
		}
	}

	/* now write the repname record after copying from the screen	*/
	if(i > 0)
		Wr_Reprec(i) ;
	else
		fomer("No structure names selected; Record not written") ;

	return(NOERROR) ;
}




/** initialisa the screen with low values	*/

init_screen() 
{
	int i ;

	screen.rept_name[0] = LV_CHAR ;
	screen.stru_num[0] = LV_SHORT;
	screen.stru_name[0][0] = HV_CHAR ;
	for(i = 1; i < MAX_STR ; i++) {
		screen.stru_num[i] = LV_SHORT;
		screen.stru_name[i][0] = HV_CHAR ;
	}
	screen.cont_flag = HV_INT ;

	sr.nextfld = SCRN_START ;
	sr.endfld = SCRN_END ;
	fomwr((char *)&screen) ;
	chkerror() ;
	fomud((char *)&screen) ;
	chkerror() ;
	for(i = 1 ; i < MAX_STR ; i++)
		screen.stru_num[i] = HV_SHORT;
	fomwr((char *)&screen) ;
	chkerror() ;
	fomud((char *)&screen) ;
	chkerror() ;
	return(NOERROR) ;
}


/* Report name window boundary	*/

#define ST_ROW	4
#define ST_COLUMN 42
#define ST_WIDTH 26
#define ST_LINES 13
#define ST_EFFLINES	(ST_LINES - 3)

static char STRname[ST_EFFLINES][NAME_LEN];
static short STRnum[ST_EFFLINES] ;

/* display the structure name window and once the selection is made */
/* update the screen with the value				    */

Disp_strwindow(scrpos)
int scrpos ;
{

	int i,done,
	    retchar,
	    chardisp ;	/* #of chars. displayed on the window	*/
	char *text ;

	struct st_list *strtemp ;
	if((text = malloc(ST_WIDTH * ST_LINES)) == NULL) 
		fprintf(stderr,"Memory allocation error \n"), abexit(-20) ;

	strtemp = strlist ;

	for(; ;) {

		/* display the header line	*/
		sprintf(text,"%-25.25s\n","Sr#  Structure Name") ;
		chardisp = 26 ;

		for(i = 1; (i <= ST_EFFLINES) &&  (strtemp != NULL) ;
			i++, strtemp = strtemp->next, chardisp += ST_WIDTH) {
			strcpy(STRname[i - 1],strtemp->strc.strname) ;
			STRnum[i - 1] = strtemp->strc.strnum + 1 ;
			/*
			fprintf(stderr,"strnum : %d, strname : %s \n",
				strtemp->strc.strnum,strtemp->strc.strname) ;
			**/

			sprintf(text + i * ST_WIDTH,"%2d. %-21.21s\n",
				i,strtemp->strc.strname) ;
		}
		if(i == 1) {
			fomen("No more structure names available ") ;
			return(NOERROR) ;
		}
		if(i >= ST_EFFLINES)
			sprintf(text + i * ST_WIDTH,"%-25.25s\n",
				"Type #/N(ext)/S(kip)") ;
		else
			sprintf(text + i * ST_WIDTH,"%-25.25s",
				"Type #/S(kip)") ;

		chardisp += 26 ;
		padnull(text,chardisp) ; /* remove inbetween nulls and put  */
					/* null at the end	*/

		window(ST_ROW,ST_COLUMN,ST_WIDTH + 5,ST_LINES + 6,text,1) ;
		done = 0 ;	/* to get the correct input	*/
		while(!done) {
			retchar = get() ;
			/** redraw() ;	...let user refresh the screen */

			switch(retchar) {
			case 'N' :
			case 'n' :
				done = 1 ;
				break ;	/* show the next page	*/

			case 's' :
			case 'S' :
				return(NOT_SELECT) ;

			default :
				if(retchar - '0' < 1 || retchar - '0' > i - 1) {
					fomer("Wrong selection ") ;
					break ;
				}

				/* save the strnum and copy strname on to buffer */
				/*  fprintf(stderr,"retchar : %d \n",retchar - '0' - 1) ;
				fprintf(stderr,"strname : %s \n",STRname[retchar - '0' - 1]) ;	*/

				strcpy(screen.stru_name[scrpos],STRname[retchar - '0' -1]) ;
				screen.stru_num[scrpos] = STRnum[retchar - '0' - 1]  ;
				rpnambuf.defstruct[scrpos].strnum = STRnum[retchar - '0' -1] - 1  ;
				/* display structure number on to the screen	*/
				sr.nextfld = sr.endfld = STRU_START + scrpos * 100 ;
				fomwr((char *)&screen) ;
				chkerror() ;
				fomud((char *)&screen) ;
				chkerror() ;

				/* display structure name on to the screen	*/
				sr.nextfld = sr.endfld = STRN_START + scrpos * 100 ;
				fomwr((char *)&screen) ;
				chkerror() ;
				fomud((char *)&screen) ;
				chkerror() ;
				sr.nextfld = STRU_START + scrpos * 100 ;

				return(NOERROR) ;
			}	/* while */
		}
	}

}
			



/* copy the repname and structure names from the screen buffer and write */
/* on the address pointed by `cur_rec`					 */

Wr_Reprec(numitems)
int numitems ;
{
	int i ;

/*	fprintf(stderr,"cur_rec : %d \n",cur_rec)  ; ****/

	strcpy(rpnambuf.rep_name,screen.rept_name) ;
	rpnambuf.numstruct = numitems ;

	for(i = 0; i < numitems ; i++ ) {
		strcpy(rpnambuf.defstruct[i].strname,screen.stru_name[i]) ;
		rpnambuf.defstruct[i].strnum = screen.stru_num[i] - 1;
	}


	/* `strname` value` has already been strored	*/
	
	/* now write the record	*/

	if(cur_rec == 0){	/* new record is to be added	*/
		rpnambuf.formrecs = 0 ;
		lseek(fdr,0L,2) ;	/* write at the end	*/
	}
	else	/* update the existing record	*/
		if(lseek(fdr,(long)((cur_rec - 1) * sizeof(struct rp_name)),0) < 0)
			fprintf(stderr,"Lseek error1 \n"),
			fprintf(stderr,"Rpname ,Wr_Reprec \n"), 
			exit(-5) ;

	if(write(fdr,(char *)&rpnambuf,sizeof(struct rp_name)) < sizeof(struct rp_name))
		fprintf(stderr,"Write error \n"),
		fprintf(stderr,"Rpname ,Wr_Reprec \n"), 
		exit(-30) ;

	return(NOERROR) ;
}


#ifdef	DELETE_LOGREC
/* Delete currently pointed report name record	*/

DEL_reprec()
{

	if(lseek(fdr,(long) ((cur_rec - 1) * sizeof(struct rp_name)),0) < 0)
		fprintf(stderr,"Lseek error1 \n"),
		fprintf(stderr,"Rpname ,Dele_reprec \n"), 
		exit(-5) ;

	if(read(fdr,(char *)&rpnambuf,sizeof(struct rp_name)) < sizeof(struct rp_name))
		fprintf(stderr,"Read error \n"),
		fprintf(stderr,"Rpname ,Dele_reprec \n"), 
		abexit(-10) ;

	rpnambuf.del_flag = DELETED ;
	
	if(lseek(fdr,(long)(-1 * sizeof(struct rp_name)),1) < 0)
		fprintf(stderr,"Lseek error2 \n"),
		fprintf(stderr,"Rpname ,Dele_reprec \n"), 
		exit(-5) ;

	if(write(fdr,(char *)&rpnambuf,sizeof(struct rp_name)) < sizeof(struct rp_name))
		fprintf(stderr,"Read error \n"),
		fprintf(stderr,"Rpname ,Dele_reprec \n"), 
		exit(-30) ;

	return(NOERROR) ;
}
#endif

/* Report name window boundary	*/

#define RP_ROW	4
#define RP_COLUMN 42
#define RP_WIDTH 30
#define RP_LINES 13
#define RP_EFFLINES	(RP_LINES - 3)


ShowRepnames()
{
	int pagenum = 0;
	int retval ;
	int i ,
	    chardisp;	/* #of characters displayed on the window	*/
	char *text;

	chardisp = 0 ;
	lseek(fdr,0L,0) ;
	if((text = malloc(RP_WIDTH * RP_LINES)) == NULL) 
		fprintf(stderr,"Memory allocation error \n"), abexit(-20) ;
	
	for(; ;) {
		retval = dispnext(text) ;
		if(retval == NEXT_PAGE) {
			pagenum++;
			continue ;
		}
		if(retval == NEW_RECORD)
			cur_rec = 0 ;
		else	{
			cur_rec = pagenum * RP_EFFLINES + retval ;
			disp_reprec() ;
		}
		return(NOERROR) ;
	}

}


dispnext(text)
char *text ;
{
	int i ,
	    chardisp;	/* #of characters displayed on the window	*/
	char retchar ;
	
	chardisp = 0 ;
	/* Include the title string	*/
	sprintf(text,"%-29.29s\n","Sr#   Report Name") ;
	chardisp += 30 ;

	for(i = 1; i <= RP_EFFLINES; i++,chardisp += RP_WIDTH) 
		if(read(fdr,(char *)&rpnambuf,sizeof(struct rp_name)) ==
							sizeof(struct rp_name))
			sprintf(text+RP_WIDTH * i,"%2d.%-26.26s\n",
				i,rpnambuf.rep_name) ;
		else {
			if(i == 1) {
				fomen("No report name is available ") ;
				return(NEW_RECORD) ;
			}
			break ;
		}
				
	sprintf(text + RP_WIDTH * i,"%-29.29s\n","Type #/ N(ew)/P(age next)") ;
	chardisp += 30 ;
	padnull(text,chardisp) ;	/* remove in between nulls and append */
					/* null at the end	*/

	
	window(RP_ROW,RP_COLUMN,RP_WIDTH + 5,RP_LINES + 6,text,1) ;
	retchar = get() ;
	redraw() ;

	switch(retchar) {
	case 'P' :
	case 'p' :
		return(NEXT_PAGE) ;

	case 'N' :
	case 'n' :
		return(NEW_RECORD) ;

	default :
		if(retchar - '0' < 1 || retchar - '0' > i) {
			fomen("Wrong selection ") ;
			return(NEW_RECORD) ;
		}
			
		return(retchar - '0') ;
	}
}	




/** Remove null if exists in first `n` characters and append null */

padnull(text,n) 
char *text ;
int	n ;
{

	*(text + n) = '\0' ;
	while(--n >= 0) {
		if(*(text + n) == '\0')
			*(text + n) = ' ' ;
	}
	return(NOERROR) ;
}




/** read the record addressed by cur_rec and display	*/

disp_reprec()
{
	int i ;
	
	/*  fprintf(stderr,"disp : cur_rec :%d \n",cur_rec) ; */
	if(lseek(fdr,(long)((cur_rec - 1) * sizeof(struct rp_name)),0) < 0)
		fprintf(stderr,"Lseek error1 \n"),
		fprintf(stderr,"Rpname ,disp_reprec \n"), 
		abexit(-5) ;

	if(read(fdr,(char *)&rpnambuf,sizeof(struct rp_name)) < sizeof(struct rp_name))
		fprintf(stderr,"Read error \n"),
		fprintf(stderr,"Rpname ,disp_reprec \n"), 
		abexit(-10) ;

	strcpy(screen.rept_name,rpnambuf.rep_name) ;
	for(i = 0; i < rpnambuf.numstruct ; i++) { 
		strcpy(screen.stru_name[i],rpnambuf.defstruct[i].strname) ;
		screen.stru_num[i] = rpnambuf.defstruct[i].strnum + 1 ;
	}

	for(; i < MAX_STR ; i++) {
		screen.stru_name[i][0] = HV_CHAR ;
		screen.stru_num[i] = HV_SHORT;
	}
	sr.nextfld = REPT_NAME ;
	sr.endfld = STRN_END ;
	fomwr((char *)&screen) ;
	chkerror() ;
	fomud((char *)&screen) ;
	chkerror() ;
}

/* Display all report names on to the screen	*/

Pr_repnames()
{
	int i = 1;
	lseek(fdr,0L,0) ;
	fprintf(stderr,"%-5.5s %-20.20s\n\n","REP#","REPORT NAME") ;
	
	for(;;) {
		if(read(fdr,(char *)&rpnambuf,sizeof(struct rp_name)) != sizeof(struct rp_name))
			break ;
		fprintf(stderr,"%5.5d %20.20s\n",i,rpnambuf.rep_name) ;
		i++ ;
	}
	fprintf(stderr,"\n\n%d Report Name(s) available \n",i) ;
}


	
/** If any error occurred in the previous profom call return error*/
/*  If any escape character is pressed ,check if HELP/DELETE/PREV*/
/* or EXIT character is pressed then return value	*/

chkerror()
{

	if(sr.retcode == RET_ERROR) {
		fprintf(stderr,"Error Occurred in profom call : %d \n",sr.errno) ,
		abexit(sr.errno) ;
	}

	if(sr.retcode == RET_USER_ESC) 
		switch(sr.escchar[0]) {

		case 'H' :
		case 'h' :
			return(HELP) ;

		case 'D' :
		case 'd' :
			return(DELETE) ;

		case 'E' :
		case 'e' :
			return(EXIT) ;

		case 'P' :
		case 'p' :
			return(PREV) ;
		
		default : fomer("Not supported by the system ") ;
		}

	return(NOERROR) ;
}



	
abexit(error)
int error ;
{
	
	fomrt() ;
	fprintf(stderr,"Error Occurred; Abormal exit : %d\n",error) ;
	exit(-1) ;
}



/* set dup. buffer to all the mask fields	*/
	
Set_dupbuf()
{
	int i ;


	for(i = STRU_START; i <= STRU_END; i+= 100)
		fomca1(i,19,2) ;
}

