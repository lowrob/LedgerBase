

/************************************************/
/*  This is a major routine which takes care of */
/*   printing of   the line				*/
/***********************************************/

static	short	repflag ;

rpline(record)
char	**record ;	/* Array of pointers to the input records	*/
{
 	short	n ;
  	short	key_no ;
  	char	strg[MXFLDSZ] ;
  	short	noff;

	userec = record ;

	if(BoundsExists)
		for(n = 0; n<f_hd->tot_flds ; n++)
			if(check_bound(n) == INVALID_REC)
				return(NOERROR) ;


	if (pageno == 0) { 	/* Report start */

		/* Next 2 functions calling sequence is interchanged on
		   28-SEP-89 by AMAR */
		initiate_keys(0) ;
		print_header() ; 

		/* First time copy values to prev_values. It may be used
		   in repeat suppression check */
		CHKERROR(store_val()) ;

		repflag = 1 ;	/* Force Printing of Repeat supress fields */
	}
	else if((key_no = check_keys()) != NOCHANGE) {

		if(subtotals(key_no) < 0) return(-1) ;
		underline(2);

		/* Inserted on 28-sep-89 by amar */
		initiate_keys(key_no) ;	/* Initialize Cur Key */

		if (key_no < f_hd->tot_pghdr){	/* key_no betwn 0 &1 */
			if(print_header() < 0) return(-1) ;
		}
		repflag = 1 ;	/* Force Printing of Repeat supress fields */
	}

	noff = -colspace ;	/* Initialize noff to -column spaces to strart
				   first al 1st column */

	if(linecount >= maxlines) {
		if(print_header() < 0) return(-1) ;
		repflag = 1 ;	/* Force Printing of Repeat supress fields */
	}

	for(n = 0 ; n < f_hd->tot_flds ; n++) {
		CHKERROR(field(n,strg)) ;

		if ( summary ) continue ;

#ifdef DEBUG
		dump_val(n) ;
#endif
		if(((fld_hd+n)->repeatsuppress)) {
			if(check_repeat(n) != VALUE_MATCH) {
				CHKERROR(get_fieldval(prev_val+n,n)) ;	
				repflag = 1 ;
			}
			if(repflag == 0) {
				noff += size[n] + colspace ;
				continue ;
			}
		}
		noff = cp(cline,(noff+colspace),strg);
		strcpy(strg,"");
	}

	repflag = 0 ;

	if ( summary ) return(NOERROR) ;

	cline[maxcols-1] ='\n';
	putline(cline);
	i_space(cline);
	put_linespace() ;
	return(NOERROR) ;
}

