/*---------------------------------------------------------------------
	Source Name: profomfn.c
	Created By : T Amarendra
	Created On : 18-JUL-90.

	Common Functions for PROFOM using programs.

	NOTE:

	These functions are written using some of the PROFOM internal
	calls directly. And all these functions returns PROFOM_ERR when
	PROFOM calls returns error.

	GetOption(), DispError() and DispMesgFld() functions are written
	based on the following assumptions.

	Current Screen is designed as

		1. Last but one field is "Message" field of 77 characters

		2. Last field is "Response" field of 1 character.

		3. These two fields are numbered in step 100.

-----------------------------------------------------------------------*/

#include <stdio.h>
#include <bfs_defs.h>
#include <cfomstrc.h>

/* Following extern variables are declared in PROFOM runtime code */

#define	END_FLD	lfldno

extern	struct	stat_rec	*sp ;
extern	int	lfldno ;		/* Last Fld# which is not PROMPT */
extern	char	*actudr ;		/* Actual user data record */

/*------------------------------------------------------------*/
/*
*  Read the PROFOM Screen for a given Range of fields. This function uses
*  fomrf() for single field range and fomrd() for multiple fields range.
*  fomrf() or fomrd() are called in a loop, so that RET_VAL_CHK and
*  RET_USER_ESC kind of PROFOM interrupts can be processed and continue
*  redaing again. Processing of PROFOM interrupts is as follows.
*
*	1. When fomrd() or fomrf() are terminated with RET_VAL_CHK
*	   then this function calls user given Validate() function.
*
*	   For invalid input, validate function should move LOW values
*	   to that field and return ERROR and for valid input it should
*	   return NOERROR. Returning NOERROR for valid input is very
*	   important when the reading range is only one field, so that
*	   reading can be terminated.
*
*	   If validate function returns RET_USER_ESC, then this function
*	   terminates reading and returns the same to calling program.
*
*	2. RET_USER_ESC is processed based on the following assumptions:
*
*		a) ESC-F means terminate the reading and return. However
*		   this is valid only when esc_f flag is ON.
*
*		b) ESC-H means invoke supplied HelpWindows() function to
*		   display help window related to sp->curfld.
*
*		   This function also should return NOERROR for valid
*		   selection and ERROR for invalid selection as explained
*		   above (refer 1).
*
*	Following are return codes
*
*		NOERROR		- Successful reading
*		RET_USER_ESC	- User pressed ESC-F or validate function
*				  return RET_USER_ESC for termination.
*		DBH_ERR		- Validate or HelpWindows() returned this
*				  error.
*		PROFOM_ERR	- For any PROFOM error.
*/

ReadFields(screen, st_fld, end_fld, Validate, HelpWindows, esc_f)
char	*screen ;
int	st_fld ;
int	end_fld ;
int	(*Validate)() ;
int	(*HelpWindows)() ;
int	esc_f ;		/* Is ESC_F active? */
{
	int	err ;
	int	(*readfn)(), fomrf(), fomrd() ;

	if(st_fld == end_fld)
		readfn = fomrf ;
	else
		readfn = fomrd ;

	sp->nextfld = st_fld ;

	for( ; ;){
		sp->endfld  = end_fld ;

		(*readfn)( screen );
		ret(err_chk(sp));
		if(sp->retcode == RET_VAL_CHK){
		    if(Validate != NULL) {
			err = (*Validate)() ;
			if(RET_USER_ESC == err || DBH_ERR == err ||
				PROFOM_ERR == err) return(err);
			if(readfn == fomrf && err == NOERROR) break ;
		    }
		    sp->nextfld = sp->curfld ;
		    continue;
		}
		if(sp->retcode == RET_USER_ESC){
			if(esc_f &&
			    (sp->escchar[0] == 'f' || sp->escchar[0] == 'F') )
				return(RET_USER_ESC) ;
			if(sp->escchar[0] == 'h' || sp->escchar[0] == 'H'){
			    if(HelpWindows != NULL) {
				err = (*HelpWindows)() ;
				if(DBH_ERR == err || PROFOM_ERR == err)
					return(err) ;
				if(readfn == fomrf && err == NOERROR) {
					/* Display the selected value */
					fomwf(screen) ;
					ret(err_chk(sp)) ;
					break ;
				}
			    }
			}
			sp->nextfld = sp->curfld ;
			continue;
		}
		/* else RET_NO_ERROR */
		break;
	}

	return(NOERROR) ;
}	/* ReadFields() */
/*------------------------------------------------------------------------*/
/* Write fields on Screen for a given Range */

WriteFields(screen, st_fld, end_fld)
char	*screen ;
int	st_fld ;
int	end_fld ;
{
	sp->nextfld = st_fld ;
	sp->endfld  = end_fld ;

	fomwr( screen ) ;
	ret(err_chk(sp));

	return(NOERROR) ;
}	/* WriteFields() */
/*-----------------------------------------------------------------------*/
/* Display given message thru last but one field and get the option by
   reaing last field */

GetOption(screen, msg, options)
char	*screen ;
char	*msg ;
char	*options ;
{
	int	i, j ;
	int	k, l ;

	/* Call PROFOM internal call to make sure current screen is up.
	   So that END_FLD (lfldno) will be set */
	if(sp == NULL) return(PROFOM_ERR) ;
	actudr = screen ;
	if( chkform() == 1 )	return(PROFOM_ERR) ;

	fomfp(END_FLD-100, &k, &i) ;
	ret(err_chk(sp)) ;

	fomfp(END_FLD, &l, &j) ;
	ret(err_chk(sp)) ;

	strncpy((screen+k),msg,i);
	DispMesgFld(screen) ;

	sp->nextfld = END_FLD ;
	for( ; ; ) {
		fomrf( screen ) ;
		ret(err_chk(sp)) ;

		j = strlen(options) ;
		for( i = 0 ; i < j ; i++)
			if((screen+l)[0] == options[i]) break ;
		if(i != j) break ;	/* Valid Option Selected */
		fomer("Invalid Option..");
	}
	(screen+k)[0] = HV_CHAR;
	(screen+l)[0] = HV_CHAR;

	ret( WriteFields(screen, (END_FLD - 100), END_FLD) );

	return((int)(options[i])) ;
}	/* GetOption() */
/*-------------------------------------------------------------------------*/
/* Copy the given message to SCR message line, display it and seek
   user response */

DispError(screen, s)    /* show ERROR and wait */
char	*screen ;
char	*s;
{
	int	i, j ;

	/* Call PROFOM internal call to make sure current screen is up.
	   So that END_FLD (lfldno) will be set */
	if(sp == NULL) return(PROFOM_ERR) ;
	actudr = screen ;
	if( chkform() == 1 )	return(PROFOM_ERR) ;

	fomfp(END_FLD-100, &i, &j) ;
	ret(err_chk(sp)) ;

	strncpy((screen+i), s, j);
	DispMesgFld(screen);
	fomen("Type any key to continue");
	get();
	(screen+i)[0] = HV_CHAR;
	DispMesgFld(screen);

	return(NOERROR);
}	/* DispError() */
/*-------------------------------------------------------------------------*/
/* Write the Message Field */

DispMesgFld(screen)
char	*screen ;
{
	/* Call PROFOM internal call to make sure current screen is up.
	   So that END_FLD (lfldno) will be set */
	if(sp == NULL) return(PROFOM_ERR) ;
	actudr = screen ;
	if( chkform() == 1 )	return(PROFOM_ERR) ;

	sp->nextfld = END_FLD - 100;
	fomwf( screen ) ;
	ret(err_chk(sp)) ;

	return(NOERROR) ;
}	/* DispMesgFld() */

/*
* This function copies st_fld to end_fld values from 'image' to 'screen'
* and displays on the screen.
*
* Usage of this function is as follows:
*	Generally when user enters ESC-F while reading screen in Change mode,
*	i.e. with show dup value is ON, assumption is he completed his changes,
*	and remaining fields are same as old. But at this point STH will be
*	having low values in the remaining fields. So move the old values from
*	the saved image.
*/

CopyBack(screen, image, st_fld, end_fld)
char	*screen ;
char	*image ;
int	st_fld ;
int	end_fld ;
{
	int	i, j, k, len ;

	/* Get the offset from begining of STH of st_fld in 'k' */
	fomfp(st_fld,&k,&j) ;
	ret(err_chk(sp)) ;

	/* Get the offset from begining of STH of end_fld in 'j' and its
	   length in 'len' */
	fomfp(end_fld,&j,&len);
	ret(err_chk(sp)) ;

	i =  j - k + len ;	/* Length to be copied */

	scpy(screen+k, image+k, i) ;	/* Copy from image */

	/* Re display that area */
	return (WriteFields(screen, st_fld, end_fld)) ;
}	/* CopyBack() */

/*--------------------------END OF FILE----------------------------------*/
