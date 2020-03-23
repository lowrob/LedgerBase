/*---------------------------------------------------------------------
	Source Name: misc.c
	Created By : T Amarendra
	Created On : 3rd May 1989.

	Common functions used by Budgetary Financial System.
-----------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <bfs_defs.h>
#include <bfs_recs.h>
#include <isnames.h>
#include <filein.h>

#define	MAX_CHAR	0177
/*-------------------------------------------------------*/ 

safecopy(dest_str,source_str,str_size)
char *dest_str;
char *source_str;
int  str_size;
{
	strncpy(dest_str,source_str,str_size);
	dest_str[str_size-1] = '\0';
	return(0);
}
/*-------------------------------------------------------*/ 
scpy ( ptr1, ptr2, len) 
char    *ptr1, *ptr2 ; 
int    len ; 
{ 
        int  i ; 
 
        for ( i = 0 ; i < len ; i++ )  
                *(ptr1+i) = *(ptr2+i) ; 
        return(0) ; 
} 

/*-------------------------------------------------------*/ 
scmp ( ptr1, ptr2, len) 
char    *ptr1, *ptr2 ; 
int    len ; 
{ 
        int  i ; 
 
        for ( i = 0 ; i < len ; i++ )  
		if( *(ptr1+i) != *(ptr2+i))  
			return(1);
        return(0) ; 
} 

/*---------------------------------------------------------*/
/*   Returns system date in YYYYMMDD format */

long
get_date()
{
	Pa_rec 		param_rec;
	char 		e_mesg[80];
	int		retval;
	int		status, size, wait ;
	long		pos ;

#ifndef	MS_DOS
	sync() ;
#endif

	status = GetSecurityStatus();

	SetSecurityStatus( 0 );
	retval = get_param( &param_rec, BROWSE, 1, e_mesg );
	SetSecurityStatus( status );

	if( retval < 0 ) return(0);

/*******
	wait = 1 ;    * Wait if End of Day/Month End etc is running *
	size = 0 ;
	pos  = 0L ;
	e_lock(getdatafd(PARAM),RDLOCK,pos, wait, size ) ;
********/

	return( param_rec.pa_date );

/*********	
	struct	tm	*newtime;
	long	ltime ;
	long	run_date ;

	time (&ltime) ;
	newtime = localtime (&ltime) ;
	run_date = (long)newtime->tm_year * 10000 +
			(long)(++(newtime->tm_mon)) * 100 +
			(long)newtime->tm_mday ;
	if(newtime->tm_year < 70)
		run_date = 20000000 + run_date ;
	else
		run_date = 19000000 + run_date ;

	return(run_date);
*********/	
}
/*---------------------------------------------------------*/
/*   Returns system time in HHMM format */
get_time()
{
	struct	tm	*newtime;
	long	ltime ;
	int	run_time ;

	time (&ltime) ;
	newtime = localtime (&ltime) ;

	run_time = newtime->tm_hour * 100 + newtime->tm_min ;

	return(run_time);
}

 /*------- routine to convert date types ---------*/ 
 
long
conv_date ( date, from, to ) 
long	date ; 
int	from, to ;		/** 1 = mmddyy , 
				    2 = ddmmyy , 
				    3 = yymmdd , 
				    4 = mmddyyyy , 
				    5 = ddmmyyyy , 
				    6 = yyyymmdd   **/ 
{ 
	long	mm,dd,yy ; 
	long	out_date ; 
 
	if(from == to) return(date) ;

 	switch (from)  { 
        case 1 :	/* mmddyy */
		yy = date % 100 ;
		date = date / 100 ;
		dd = date % 100 ;
		mm = date / 100 ;
		break ; 
        case 2 :	/* ddmmyy */
		yy = date % 100 ;
		date = date / 100 ;
		mm = date % 100 ;
		dd = date / 100 ;
		break ; 
        case 4 :	/* mmddyyyy */
		yy = date % 10000 ;
		date = date / 10000 ;
		dd = date % 100 ;
		mm = date / 100 ;
		break ; 
        case 5 :	/* ddmmyyyy */
		yy = date % 10000 ;
		date = date / 10000 ;
		mm = date % 100 ;
		dd = date / 100 ;
		break ; 
        case 3 :	/* yymmdd */
        case 6 :	/* yyyymmdd */
		dd = date % 100 ;
		date = date / 100 ;
		mm = date % 100 ;
		yy = date / 100 ;
		break ; 
        } 
 
	switch (to) { 
	case 1 :
		out_date = mm * 10000L + dd * 100 + yy ;
		break ; 
	case 2 :
		out_date = dd * 10000L + mm * 100 + yy ;
		break ; 
        case 4 :
		out_date = mm * 1000000L + dd * 10000L + yy ;
		break ; 
        case 5 :
		out_date = dd * 1000000L + mm * 10000L + yy ;
		break ; 
        case 3 :
        case 6 :
		out_date = yy * 10000L + mm * 100 + dd ;
		break ; 
        } 

	return(out_date) ; 
} 
 
#ifdef	XXXX
/*-------------------------------------------------------*/ 
/**-- Routine to pad blanks and convert Lowwer to Upper case in a string -*/

/**
	Converts Lower to Upper case till a Null is hit. Thereonwards
   all characters will be replaced by blank .
**/
 
int 
pad_blanks (key, len)  
 
char    *key ; 
int     len  ; 
 
{ 
 
        int     flag ,i; 
 
        flag = 0 ; 
 
        for ( i = 0 ; i < len ; i++ )  
        { 
                if ( *(key+i) < 32 || flag == 1 ) 
                { 
                        *(key+i) = ' ' ; 
                        flag     = 1 ; 
                } 
                else if (flag == 0 ) 
                        if ( *(key+i) >= 'a' && *(key+i) <= 'z' ) 
                                *(key+i) = *(key+i) + ('A'-'a'); 
 
        } 
        return(0) ; 
 
}
#endif

int  mon_tbl[] = {
		0,	/* Dummy */
		31, 0,	31, 30, 31, 30, 31, 31, 30, 31, 30, 31
		};
/*----------------------------------------------------------------*/
long		/* For a given YYYYMMDD date returns date after given days */
date_plus(today,days)
long	today;	/* Date in YYYYMMDD */
int	days;	/* No of days */
{
	int dd,mm,yyyy;


	yyyy = (int)(today / 10000);
	mm = (int)((today / 100) % 100);
	dd = (int)(today % 100);

	if( (yyyy%400==0) || ( (yyyy%4==0)&&(yyyy%100!=0) ) )
		mon_tbl[2] = 29;	/* Leap Year */
	else
		mon_tbl[2] = 28;

	if(days >= 0){		/* Forward */
		/* add no of days to the current day. If no of days in the
		   current month execeeds then increment month. */
		dd += days ;
		for( ; dd > mon_tbl[mm] ; ) {
			dd -= mon_tbl[mm] ;   /* Find out days in next month */
			mm++;
			if(mm > 12){
				mm = 1;
				yyyy++;
				if( (yyyy%400==0) ||
						( (yyyy%4==0)&&(yyyy%100!=0) ) )
					mon_tbl[2] = 29;	/* Leap Year */
				else
					mon_tbl[2] = 28;
			}
		}
	}
	else {
		/* subtract no of days to the current day. If no of days in the
		   current month less than 1 decrement month. */
		dd += days ;
		for( ; dd < 1 ; ) {
			mm--;
			if(mm < 1){
				mm = 12;
				yyyy--;
				if( (yyyy%400==0) ||
						( (yyyy%4==0)&&(yyyy%100!=0) ) )
					mon_tbl[2] = 29;	/* Leap Year */
				else
					mon_tbl[2] = 28;
			}
			if(dd == 0) {	/* End of the previous month */
				dd = mon_tbl[mm] ;
				break ;
			}
			dd += mon_tbl[mm] ;   /* Find out days in next month */
		}
	}
	return((long)((long)yyyy * 10000 + (long)mm * 100 + (long)dd));
}
/*---------------------------------------------------------------*/

#ifdef MS_DOS
char *crypt(s1,s2)
char	*s1, *s2 ;
{
return(s1) ;
}
#endif
/*-----------------------------------------------------------------*/
/* find out no of days upto the given date in that century */

long
days(dt)
long	dt;	/* YYYYMMDD format */
{
	long	j;
	long	yyyy ;
	short	mm ;
	short	day ;

	/* Date is YYYYMMDD */
	yyyy = dt / 10000 ;
	mm = (dt / 100 % 100) ;
	day = (dt % 100) ;
	/* No of days upto the previous year */
	j = (yyyy - 1) * 365 + (yyyy - 1) / 4 - (yyyy - 1) / 100 +
			(yyyy - 1) / 400 ;

	/* No of days upto the previous month */
	switch(mm){	/* Month */
		case  2:j += 31;
			break;
		case  3:j += 59 ;
			break;
		case  4:j += 90 ;
			break;
		case  5:j += 120 ;
			break;
		case  6:j += 151 ;
			break;
		case  7:j += 181 ;
			break;
		case  8:j += 212 ;
			break;
		case  9:j += 243 ;
			break;
		case 10:j += 273 ;
			break;
		case 11:j += 304 ;
			break;
		case 12:j += 334 ;
	}
	/* No of days upto the Day */
	j += day ;

	/* If the current year is leap year and month > 2 add 1 more day */
	if( ((yyyy % 4 == 0 && yyyy % 100 != 0) || (yyyy % 400 == 0) ) &&
		mm > 2)j++;
	return(j);
}

/*-----------------------------------------------------------------*/
/* 
*	 Function to return the terminal number in a charcter string ..  
*/

int 
get_tnum(c_num)
char	*c_num ;		/* Minimum 4 Character string */
{
	char	*ttyname() ;
	char	*t_name ;
	char	*pointer;
	long	i;

#ifndef MS_DOS

	t_name = ttyname(fileno(stdin)) ;	/* fileno(stdin) == 0 */

/* SBO:	910313
	The following code will extract the last four characters of the basename
	portion of the ttyname.						*/

	
	pointer = strchr(t_name,'\0');
	for (i=0; i<5 && *pointer != '/'; i++)
		pointer--;
	pointer++;
	strcpy(c_num , pointer) ;

#else 
	strcpy(c_num, "00" );
#endif
	return(0);
}

int
month_of( format, dt )
int format;
long dt;
{
	switch( format ){
		case MMDDYY:
			return( (int)(dt/10000) );
		case DDMMYY:
		case YYMMDD:
			return( (int)( (dt/100)%100 ) );
		case MMDDYYYY:
			return( (int)(dt/1000000) );
		case DDMMYYYY:
			return( (int)( (dt/10000)%100 ) );
		case YYYYMMDD:
			return( (int)( (dt/100)%100 ) );
	}
}

int
date_of( format, dt )
int format;
long dt;
{
	switch( format ){
		case MMDDYY:
			return( (int)( (dt/100)%100 ) );
		case DDMMYY:
			return( (int)(dt/10000) );
		case YYMMDD:
			return( (int)(dt%100) );
		case MMDDYYYY:
			return( (int)( (dt/10000)%100 ) );
		case DDMMYYYY:
			return( (int)(dt/1000000) );
		case YYYYMMDD:
			return( (int)(dt%100) );
	}
}

int
year_of( format, dt )
int format;
long dt;
{
	switch( format ){
		case MMDDYY:
		case DDMMYY:
			return( (int)(dt%100) );
		case YYMMDD:
			return( (int)(dt/10000) );
		case MMDDYYYY:
		case DDMMYYYY:
			return( (int)(dt%10000) );
		case YYYYMMDD:
			return( (int)(dt/10000) );
	}
}

/*-----------------------------------------------------------------*/
/*
*	routine to get the next higher value of string given its length.
*	This function is used to postion the ISAM file to next key, when
*	the least part of the key is CHAR type
*/

inc_str( str, len, direction )
char	*str;		/* String to be incremented or decremented */
int	len;		/* Part length in key */
int	direction;	/* Reading Direction, Forward or Previous */
{
	int	i;

	i = strlen(str) ;
	if(i < len) {
		if(direction == FORWARD) {
			/* Increment the null and put the null in next char */
			str[i]++ ;
			str[i+1] = '\0' ;
		}
		else if(i) {
			/* decrement the before null char and set remaining
			   chars to max chars */
			str[i-1]-- ;
			for( ; i < len ; i++)
				str[i] = MAX_CHAR ;
		}
	}
	else {
		/* Increment/decrement the last char */
		if(direction == FORWARD)
			str[len - 1]++ ;
		else
			str[len - 1]-- ;
	}

	return(NOERROR) ;
}
/*-----------------------------------------------------------------*/
/** 	formats the date string as per "____/__/__" format 	***/

mkdate(date, dt_str)
long	date ;		/* Should be YYYYMMDD format  */
char	*dt_str ;	/* At least 12 character long */
{
	long	yr, mnth ;

	yr   = date / 10000  ;
	date = date - yr * 10000 ;
	mnth = date / 100 ;
	date = date - mnth * 100 ;

	sprintf(dt_str, "%4d/%02d/%02d", yr, mnth, date) ;
}
/*----------------------------------------------------------------*/
/*
*	acnt_chk()	- Validates and right justifies the given acnt#.
*
*  Check whether given account# is having any non numeric characters.
*  Account# is always right justified with leading blanks. After checking
*  for non numeric characters right justify it and copy it to source fld.
*  Return ERROR for invalid accont# else return NOERROR.
*/

acnt_chk(acnt_no)
char	*acnt_no ;
{
	int	i ;
	char	t_acnt[19] ;

	/* Skip initial blanks */
	for(i = 0 ; i < 18 ; i++) {
		if( acnt_no[i] == '\0' ) return(ERROR) ; /* No valid Digit */
		if( isspace(acnt_no[i]) ) continue ;
		break ;
	}

	/* Suppress Leading Zeroes */
	for( ; i < 18 ; i++) {
		if( acnt_no[i] == '\0' ) return(ERROR) ; /* No valid Digit */
		if( !isdigit(acnt_no[i]) ) return(ERROR) ;
		if( acnt_no[i] == '0' ) {
			acnt_no[i] = ' ' ; /* Replace Leading 0's with blanks */
			continue ;
		}
		break ;
	}
	if(i == 18) return(ERROR) ;

	/* Check whether there are any non numeric characters */
	for(; acnt_no[i] != '\0' && i < 18 ; i++)
		if( !isdigit(acnt_no[i]) ) return(ERROR) ;

	/* Right Justify the account number */
	sprintf(t_acnt,"%18.18s",acnt_no);
	strcpy(acnt_no,t_acnt);
	return(NOERROR) ;
}
/* str	 	- char pointer to unjustified str */
/* str_len   	- length of str 	     */
/*             Note: Embeded blanks terminate str at that point. */
/*                   Remainder of str is ignored                 */
Right_Justify_Numeric(str,str_len)
char *str;
int str_len;
{
char *temp;
int i;
char mask[10];

	if(str[0]=='\0')  return(0);
	/* skip leading blanks */
	for(i=0;i<str_len && str[i] == ' ';i++);
	/* Ignore trailing blanks */
	for(;i<str_len && str[i] != ' ' && str[i] != 0;i++)
		/* if not digit leave code as is */
		if(isdigit(str[i])==0){
		  	return(0);	
		}
	/* right justify str to tempingation string */
	if((temp = (char *)malloc(str_len+1)) == 0) return(0);
	sprintf(mask,"%%%d.%ds",str_len,i);
	strcpy(temp,str);
	sprintf(str,mask,temp);
	free(temp);
  	return(1);	
}
/************************************************************************/
/*	Routine:	D_Roundoff					*/
/*	Written by:	Steven Osborne					*/
/*	Date Written:	901217						*/
/*	Last Modified:							*/
/*	Description:							*/
/*		The following routine is used to round double precision	*/
/*	floating point values (representing dollar and cent amounts).	*/
/*	The roundoff factor adds five to the digit after the cents	*/
/*	before truncating.						*/
/************************************************************************/

double D_Roundoff(dvalue)
	double dvalue;
{
	long	cents;		/* The number of cents in the number.	*/
	long	dollars;	/* The number of dollars in the number.	*/
	double	fraction;	/* The fractional part of the number.	*/
	double	result;		/* The number after the rounding.	*/
	double	roundoff=0.005001;	/* The rounding factor.	*/

	/* Isolate whole dollars from fractional part */
	dollars = (long)dvalue;
	fraction = dvalue - (double)dollars;

	/* Calculate the number of cents.	*/
	if(dvalue < 0) 
		cents = 100.0 * (fraction - roundoff);
	else
		cents = 100.0 * (fraction + roundoff);
	
	/* Recombine the dollars and cents into a double precision.	*/
	result = (double)dollars + (double)cents / 100.0;

	return result;
}


Tax_Calculation(gst_tax,pst_tax,net_amt,tax_ret)
short gst_tax,pst_tax;
double net_amt;
Tax_cal *tax_ret;
{
	if(gst_tax == 0)
		tax_ret->gst_amt = 0.0;
	else{
		tax_ret->gst_amt = net_amt * ((double)gst_tax/100.0);
		/*D_Roundup(&tax_ret->gst_amt);*/
		tax_ret->gst_amt = D_Roundoff(tax_ret->gst_amt);
	}

	if(pst_tax == 0)
		tax_ret->pst_amt = 0.0;
	else{
		tax_ret->pst_amt = (net_amt + tax_ret->gst_amt)
						 * ((double)pst_tax/100.0);
		/*D_Roundup(&tax_ret->pst_amt);*/
		tax_ret->pst_amt = D_Roundoff(tax_ret->pst_amt);
	}
	tax_ret->gros_amt = net_amt + tax_ret->gst_amt + tax_ret->pst_amt;
}

#define	NOERROR	0
#define	BADVALUE	-1
#define	TRUE	1
#define	FALSE	0

/************************************************************************/
/*	Routine:	D_FixRound					*/
/*	Written by:	Steven Osborne					*/
/*	Date Written:	901218						*/
/*	Last Modified:							*/
/*	Description:							*/
/*		The following routine is used to round imprecise	*/
/*	floating point values (representing dollar and cent amounts)	*/
/*	when the precision is unknown.  The decimal place after the	*/
/*	cents (the third place) should be a zero or a nine if it is a	*/
/*	precise digit.  If it is a zero then the cent value is left	*/
/*	unchanged.  If it is a nine then the cent value is incremented	*/
/*	by one.  If the digit is neither, then the value is not rounded	*/
/*	and is returned as the rounded result, and a return code of	*/
/*	BADVALUE is set.						*/
/************************************************************************/

double D_FixRound(dvalue,rcode)
	double dvalue;
	long *rcode;
{
	double	atof();
	char	*decimal;	/* Pointer to the decimal point in string. */
	long	nextcent=FALSE;	/* Flag to increment the cent value.	*/
	double	result;		/* Result of rounding.	*/
	char	string[256];	/* String to hold the number.	*/

	/* Place the number into a string.	*/
	sprintf(string,"%30.16lf",dvalue);

	/* Find the decimal point.	*/
	decimal = strchr(string,'.');

	/* Move to the third place after the decimal point.	*/
	decimal += 3;

	/* If this is not a zero or a nine then we can't round.	*/
	if (*decimal != '0' && *decimal != '9')
	{
		*rcode = BADVALUE;
		return dvalue;
	}

	/* If this is a nine then note that the previous value of	*/
	/* cents should be incremented.					*/
	if (*decimal == '9')
		nextcent = TRUE;

	/* End the string after the cents value.	*/
	*decimal = '\0';

	/* Convert string back to a double precision number.	*/
	result = atof(string);

	/* Increment the cent value if the flag indicates it.	*/
	if (nextcent)
		if(dvalue > 0)
			result += 0.01;
		else
			result -= 0.01;
	
	/* Set the return code and return.	*/
	*rcode = NOERROR;
	return result;
}

double Commit_Calculation(gst_tax,pst_tax,rebate,net_amt,tax_ret)
short gst_tax,pst_tax,rebate;
double net_amt;
Tax_cal *tax_ret;
{
double commit_amt;

	Tax_Calculation(gst_tax,pst_tax,net_amt,tax_ret);
	commit_amt = net_amt + tax_ret->pst_amt + 
			(tax_ret->gst_amt * ((100.0 - ((double)rebate))/100));
	/*D_Roundup(&commit_amt);*/
	commit_amt = D_Roundoff(commit_amt);
	return(commit_amt);	
}
/*------------------------E N D   O F   F I L E------------------------*/
