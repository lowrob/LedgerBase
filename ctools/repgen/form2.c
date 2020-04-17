
/*** Functon declare and prototype ***/
static int sgetop(char *, short, char *);
static int symbol_test(char *);

/* Read field attributes for the reported fields	*/

change_fld()
{
	int i ;

	if(Sl_fldlst[Cur_dispfld - 1].Sl_class == INP_FLD) {
		sr.nextfld = COMP_EQA ;
		screen.Comp_eqa[0] = HV_CHAR ;
		fomwf((char *)&screen) ;
		chkerror() ;
	}

	if(Sl_fldlst[Cur_dispfld - 1].Sl_class == COMP_FLD) 
	  for(; ;) {
		sr.nextfld = COMP_EQA ;
		fomrf((char *)&screen) ;
		retcode = chkerror() ;
		if(sr.retcode == RET_USER_ESC)
		switch(retcode) {

		case EXIT : 
			fomen("Exit pressed ! Update is not done") ;
			return(EXIT) ;

		case HELP :
			fomen("Type the formula. Input field can be used by i[fld#.Op : +,-,*,/") ;		
			continue ;

		default : continue ;
		}
		if(strlen(screen.Comp_eqa) == 0) {
			fomen("Type Equation,Esc-E(xit),H(elp),D(elete),P(rev)") ;
			continue ;
		}

		if(alter_rp(screen.Comp_eqa,
				Sl_fldlst[Cur_dispfld-1].Sl_polcomp) == ERROR) 
			continue ;
#ifdef DEBUG
		fprintf(stderr,"After Read: Screen: %s, Sl: %s\n",
			screen.Comp_eqa,Sl_fldlst[Cur_dispfld - 1].Sl_polcomp) ;
#endif
		break ;
	}


	for(; ;) {
		sr.nextfld = REPEATSUPPRESS ;
		fomen("If same value is repeated in the output, show blanks ?Y/N") ;
	
		fomrf((char *)&screen) ;
		retcode = chkerror() ;
		if(sr.retcode == RET_USER_ESC)
		switch(retcode) {

		case EXIT : 
			fomen("Exit pressed ! Update is not done") ;
			return(EXIT) ;

		case HELP :
			fomen("If the same value is repeated in the output, show blanks?Y/N") ;		
			continue ;

		default : continue ;
		}
		if(screen.Repeatsuppress == BOOL_NO)
			fomen("Repeated items to be shown") ;
		break ;
	}

	for(; ;) {
		sr.nextfld = CUR_TITLE ;
		
		fomrf((char *)&screen) ;
		retcode = chkerror() ;
		if(sr.retcode == RET_USER_ESC)
		switch(retcode) {

		case EXIT : 
			fomen("Exit pressed ! Update is not done") ;
			return(EXIT) ;

		case HELP :
			fomen("Type Title ,Esc-E(xit),H(elp),D(elete),P(rev)") ;		
			continue ;

		case DELETE :
			screen.Cur_title[0] = LV_CHAR ;
			fomwr((char *)&screen) ;
			chkerror() ;
			continue ;

		default : continue ;

		}
		if(strlen(screen.Cur_title) == 0) 
			fomen("No title is given..") ;
		break ;
	}


	/* Ask width only for charcter fields. For numeric fields compute the
	   width after taking the edit mask */
	if(Sl_fldlst[Cur_dispfld - 1].Sl_type == CHAR_FLD)
	for(; ;) {
		sr.nextfld = CUR_WIDTH ;
		
		fomrf((char *)&screen) ;
		retcode = chkerror() ;
		if(sr.retcode == RET_USER_ESC)
		switch(retcode) {

		case EXIT : 
			fomen("Exit pressed ! Update is not done") ;
			return(EXIT) ;
		case HELP :
			fomen("Type the WIDTH of the field") ;		
			continue ;
		case DELETE :
			screen.Cur_width = LV_SHORT ;
			fomwr((char *)&screen) ;
			chkerror() ;
			continue ;
		default :
			continue ;
		}
		if(screen.Cur_width == 0)  {
			fomen("No width is given..") ;
			continue ;
		}
		break ;
	}


	if(Sl_fldlst[Cur_dispfld - 1].Sl_type != CHAR_FLD)
	    for(; ;) {
		sr.nextfld = CUR_TOTFLAG ;
		
		fomrf((char *)&screen) ;
		retcode = chkerror() ;
		if(sr.retcode == RET_USER_ESC)
		switch(retcode) {

		case EXIT : 
			fomen("Exit pressed ! Update is not done") ;
			return(EXIT) ;

		case HELP :
			fomen("Field value to be summed-up for subtotal display?") ;		
			continue ;

		default : continue ;
		}
		if(screen.Cur_totflag == BOOL_NO) 
			fomen("Not to be totalled..") ;
		break ;
	    }


	if(Sl_fldlst[Cur_dispfld - 1].Sl_type != CHAR_FLD)
		strcpy(screen.Justify,"R");
	else for(; ;) {
		sr.nextfld = JUSTIFY ;
		fomrf((char *)&screen) ;
		retcode = chkerror() ;
		if(sr.retcode == RET_USER_ESC)
		switch(retcode) {

		case EXIT : 
			fomen("Exit pressed ! Update is not done") ;
			return(EXIT) ;

		case HELP :
			fomen("Type just. code : R(ight)/L(eft)/C(entre).Esc-E(xit),H(elp),D(elete),P(rev)") ;		
			continue ;

		default: continue ;
		}
		if(strlen(screen.Justify) == 0) {
			fomen("Type just. code : R(ight)/L(eft)/C(entre).Esc-E(xit),H(elp),D(elete),P(rev)") ;		
			continue ;
		}
	
		break ;
	}


	if(Sl_fldlst[Cur_dispfld - 1].Sl_type == CHAR_FLD) {
		screen.Edit_mask[0] = '\0' ;
		sr.nextfld = EDIT_MASK ;
		fomwf((char*)&screen) ;
	}
	else for(; ;) {
		sr.nextfld = EDIT_MASK ;
		fomrf((char *)&screen) ;
		retcode = chkerror() ;
		if(sr.retcode == RET_USER_ESC)
		switch(retcode) {

		case EXIT : 
			fomen("Exit pressed ! Update is not done") ;
			return(EXIT) ;

		case HELP :
			fomen("Type Edit mask ,Esc-E(xit),H(elp),D(elete),P(rev)") ;		
			continue ;

		case DELETE :
			screen.Edit_mask[0] = LV_CHAR ;
			fomwr((char *)&screen) ;
			chkerror() ;
			continue ;

		default : continue ;
		}
		if((i = strlen(screen.Edit_mask)) == 0) {
			fomen("No Edit mask is given..") ;
			continue ;
		}
		screen.Cur_width = i ;
		sr.nextfld = CUR_WIDTH ;
		fomwf((char*)&screen) ;
		break ;
	}

	fomen("Type bounds for the valid output record ") ;
	for(i = 0; i < 2 ; i++) {
		sr.nextfld = MINBOUND + i * 100 ;
		fomrf((char *)&screen) ;
		retcode = chkerror() ;
		if(sr.retcode == RET_USER_ESC)
		switch(retcode) {
		case EXIT :
			fomen("Exit Pressed ! Update is not done ...") ;
			return(EXIT) ;
			
		case HELP :
			fomen("Type bounds for the valid output record") ;
			i-- ; continue ;

		case PREV : 
			if(sr.nextfld > MINBOUND) {
				i-- ;
				continue ;
			}
			fomen("Field Backword not allowed ") ;
			i-- ; continue  ;

		default :
			i-- ; continue  ;

		}
	
		if(sr.fillcode == FIL_DUP)
			continue ;
		i-- ;
	}

	return(NOERROR) ;
}

			
/* Select the field to be displayed depending upon the option	*/
			
Select_fld(option)
char option ;
{
	switch(option) {

	case NEXT_FLD : 
		if(Cur_dispfld == norepflds) {
			fomen("End of the field list.. No forward allowed") ;
			break ;
		}
		
		fomca1(REP_START + (Cur_dispfld - 1) * 100,9,5) ;
		fomca1(REP_START + (Cur_dispfld) * 100,9,3) ;
		Cur_dispfld += 1 ;
		Disp_fld() ;	/* read and display this field */
		break ;

	case PREV_FLD :
		if(Cur_dispfld == 1) {
			fomen("End of the field list.. No backward allowed") ;
			break ;
		}

		Cur_dispfld -= 1 ;
		fomca1(REP_START + (Cur_dispfld) * 100,9,5) ;
		fomca1(REP_START + (Cur_dispfld - 1) * 100,9,3) ;
		Disp_fld() ;	/* read and display this field */
		break ;

	default :
		fprintf(stderr,"Wrong parameter passed : Select_fld \n") ;
	}

	return(NOERROR) ;
}



/* Copy the pointed entry from Sl_fldlst[] and display */

Disp_fld()
{
	if(Cur_dispfld < 1 || Cur_dispfld > MAX_REPFLD)
		fprintf(stderr,"Cur_dispfld out of bound : %d \n",Cur_dispfld) ,
		fprintf(stderr,"Module : repform. Func : Disp_fld \n") ,
		abexit(INTERR) ;

	strcpy(screen.Cur_field,Sl_fldlst[Cur_dispfld - 1].Sl_fldname) ;
	if(Sl_fldlst[Cur_dispfld - 1].Sl_type == CHAR_FLD)
		strcpy(screen.Cur_type,"CHAR") ;
	else
		strcpy(screen.Cur_type,"NUME") ;

	if(Sl_fldlst[Cur_dispfld - 1].Sl_class == INP_FLD)
			strcpy(screen.Cur_class,"INPUT") ;
	else
			strcpy(screen.Cur_class,"COMP") ;

	strcpy(screen.Cur_title,Sl_fldlst[Cur_dispfld - 1].Sl_title) ;
	if(screen.Cur_class[0] == INP_FLD) 
		screen.Comp_eqa[0] = HV_CHAR ;
	else {
		strcpy(screen.Comp_eqa,Sl_fldlst[Cur_dispfld - 1].Sl_compeqa) ;
#ifdef DEBUG
		fprintf(stderr,"Disp_fld screen : Screen : %s, Sl :%s\n",
			screen.Comp_eqa,Sl_fldlst[Cur_dispfld - 1].Sl_compeqa) ;
#endif
	}

	screen.Cur_totflag = Sl_fldlst[Cur_dispfld - 1].Sl_totflag ;
	strcpy(screen.Edit_mask,Sl_fldlst[Cur_dispfld - 1].Sl_edit_mask) ;
	screen.Justify[0] = Sl_fldlst[Cur_dispfld - 1].Sl_justify ;
	screen.Cur_width = Sl_fldlst[Cur_dispfld - 1].Sl_size ;
	screen.Repeatsuppress = Sl_fldlst[Cur_dispfld - 1].Sl_repeatsuppress ;
	strcpy(screen.Minbound,Sl_fldlst[Cur_dispfld -1].Sl_minbound) ;
	strcpy(screen.Maxbound,Sl_fldlst[Cur_dispfld - 1].Sl_maxbound) ;
#ifdef DEBUG
	fprintf(stderr,"Disp_fld: edit: %s,just: %d, minbound:%s fldname: %s\n",
			Sl_fldlst[Cur_dispfld - 1].Sl_edit_mask,
			Sl_fldlst[Cur_dispfld - 1].Sl_justify,
			Sl_fldlst[Cur_dispfld - 1].Sl_minbound,
			Sl_fldlst[Cur_dispfld - 1].Sl_fldname) ;
#endif

	sr.nextfld = ATTR_START ;
	sr.endfld = ATTR_END ;
	fomwr((char *)&screen) ;	/* Write & Update Dup Buffers */
	chkerror() ;
	
	return(NOERROR) ;
}

/** Just now user has entered field attributes on the screen , */
/* let us save the data in Sl_fldlst[] buffer	*/

Up_slfld()
{
	if(Cur_dispfld < 1 || Cur_dispfld > MAX_REPFLD)
		fprintf(stderr,"Cur_dispfld out of bound : %d \n",Cur_dispfld) ,
		fprintf(stderr,"Module :repform. Func : Up_slfld \n") ,
		abexit(INTERR) ;

	strcpy(Sl_fldlst[Cur_dispfld - 1].Sl_fldname, screen.Cur_field) ;
	Sl_fldlst[Cur_dispfld - 1].Sl_type = screen.Cur_type[0] ;
	Sl_fldlst[Cur_dispfld - 1].Sl_class = screen.Cur_class[0] ;
	strcpy(Sl_fldlst[Cur_dispfld - 1].Sl_title,screen.Cur_title) ;
	Sl_fldlst[Cur_dispfld - 1].Sl_totflag = screen.Cur_totflag ;
	Sl_fldlst[Cur_dispfld - 1].Sl_size = screen.Cur_width ;
	Sl_fldlst[Cur_dispfld - 1].Sl_repeatsuppress = screen.Repeatsuppress ;
	strcpy(Sl_fldlst[Cur_dispfld - 1].Sl_minbound,screen.Minbound) ;
	strcpy(Sl_fldlst[Cur_dispfld - 1].Sl_maxbound,screen.Maxbound) ;
	
	
	if(screen.Cur_class[0] == INP_FLD) 
		Sl_fldlst[Cur_dispfld - 1].Sl_compeqa[0] = '\0';
	else{
		strcpy(Sl_fldlst[Cur_dispfld - 1].Sl_compeqa, screen.Comp_eqa);
#ifdef DEBUG
		fprintf(stderr,"After Update Sl : Screen : %s, Sl :%s\n",
			screen.Comp_eqa,Sl_fldlst[Cur_dispfld - 1].Sl_compeqa) ;
#endif
	}	
	strcpy(Sl_fldlst[Cur_dispfld - 1].Sl_edit_mask,screen.Edit_mask) ;
	Sl_fldlst[Cur_dispfld - 1].Sl_justify = screen.Justify[0] ;

	return(NOERROR) ;
}

/** Read Page Break & total fields numbers and fill-up the corresponding field
    names */

Rd_pagetotflds()
{
	int	i ;
	struct fld_list *fldlptr, *Get_fldelement();

	sr.nextfld = PB_SUPFLD ;
	screen.Pb_supfld[0] = ' ' ;
	fomwf((char *)&screen) ;
	chkerror() ;

	for(i = 0 ; i < 2 ; i++) { 
		sr.fillcode = FIL_OMITTED ;
		sr.nextfld = PBNO_START + i * 100 ;
		fomrf((char *)&screen) ;
		retcode = chkerror() ;

		if(sr.retcode == RET_USER_ESC)
		switch(retcode) {
		case EXIT :
			fomen("Exit pressed! Update is not done") ;
			return(EXIT) ;

		case HELP :

			/* User will select appropriate field number */
			/* Validation has to be done on the field number */
			/* Display the field names */

			if(Show_fldnames(INT_FLD) == NOTSELECTED) {
				i-- ;
				continue ;
			}
			screen.Pb_no[i] = ICfield.INTfieldno ;
			
			fomwf((char *)&screen) ;
			chkerror() ;
			break ;

		case PREV :
			if(sr.nextfld == PBNO_START) {
				fomer("Field Backword not allowed ") ;
				i-- ;
				continue ;
			}
			i -= 2 ;
			continue ;

		case DELETE :
			if(screen.Pb_no[i] == 0 || screen.Pb_no[i] == LV_SHORT){
				fomen("Nothing exists to delete ") ;
				i-- ;
				continue ;
			}

			if(i == 0) {
				screen.Pb_no[0] = screen.Pb_no[1] ;
				strcpy(screen.Pb_totflds[0],
						screen.Pb_totflds[1]) ;
				strcpy(screen.Pb_text[0],
						screen.Pb_text[1]) ;
				strcpy(screen.Pbedit_mask[0],
						screen.Pbedit_mask[1]) ;

				pbtotflds[0].infieldno =
						pbtotflds[1].infieldno;
				pbtotflds[0].inrecno =
						pbtotflds[1].inrecno;

			}

			screen.Pb_no[1] = 0 ;
			screen.Pb_totflds[1][0] = LV_CHAR ;
			screen.Pb_text[1][0] = LV_CHAR ;
			screen.Pbedit_mask[1][0] = LV_CHAR ;

			pbtotflds[1].infieldno = 0 ;
			pbtotflds[1].inrecno = 0 ;

			sr.endfld = PBMASK_END ;
			fomwr((char *)&screen) ;
			chkerror() ;
			i-- ;
			continue ;

		default :
			i-- ;
			continue ;
		}

		if(screen.Pb_no[i] == 0) break ;	/* No field defined */

		if(sr.retcode == RET_NO_ERROR && sr.fillcode == FIL_DUP) {
						/* already in the list */
			/* now read the mesg. text  */

			fomen("Type the Message Text, printed along with the key field") ;
			sr.nextfld = PBMSG_START + i * 100 ;
			fomrf((char *)&screen) ;
			chkerror() ;
	
			fomen("Type the Edit mask for the key value display") ;
			sr.nextfld = PBMASK_START + i * 100 ;
			fomrf((char *)&screen) ;
			chkerror() ;

			continue ;
		}

		if((fldlptr = Get_fldelement(screen.Pb_no[i])) == NULL) {
			fomen("Field Number not selected properly ") ;
			i-- ;
			continue ;
		}
		else {	/* display the field name and update the global entr*/
				/* fldattri will have this field info.	*/
			
			strcpy(screen.Pb_totflds[i],fldlptr->Fldname) ;
			sr.nextfld = PBTO_START + i * 100 ;
			fomwf((char *)&screen) ;
			chkerror() ;

			if(fldlptr->Fld_typ != CHAR)
				Dflt_mask(screen.Pbedit_mask[i],
					fldlptr->Fld_typ) ;
			else
				screen.Pbedit_mask[i][0] = '\0' ;

			sr.nextfld = PBMASK_START + i * 100 ;
			fomwf((char *)&screen) ;
			chkerror() ;

			pbtotflds[i].infieldno = fldlptr->Fldno ;
			pbtotflds[i].inrecno = fldlptr->Recno ;
		}
		i-- ;
	}

	for( ; i < 2 ; i++) {
		screen.Pb_no[i] = LV_SHORT ;
		screen.Pb_totflds[i][0] = LV_CHAR ;
		screen.Pb_text[i][0] = LV_CHAR ;
		screen.Pbedit_mask[i][0] = LV_CHAR ;

		pbtotflds[i].infieldno = 0 ;
		pbtotflds[i].inrecno = 0 ;	
	}
	sr.nextfld = PBNO_START ;
	sr.endfld = PBMASK_END ;
	fomwr((char *)&screen) ;
	chkerror() ;
	return(NOERROR) ;
}

/** Read Subtotal fields numbers and fill-up the corresponding field names */

Rd_subtotflds()
{
	int	i, j  ;
	struct fld_list *fldlptr, *Get_fldelement();

	sr.nextfld = SUB_SUPFLD ;
	screen.Sub_supfld[0] = ' ' ;
	fomwf((char *)&screen) ;
	chkerror() ;

	for(i = 0 ; i < 3 ; i++) { 
		sr.fillcode = FIL_OMITTED ;
		sr.nextfld = SBNO_START + i * 100 ;
		fomrf((char *)&screen) ;
		retcode = chkerror() ;
		if(sr.retcode == RET_USER_ESC)
		switch(retcode) {
		case EXIT :
			fomen("Exit pressed! Update is not done") ;
			return(EXIT) ;

		case HELP :

			/* User will select appropriate field number */
			/* Validation has to be done on the field number */
			/* Display the field names */

			if(Show_fldnames(INT_FLD) == NOTSELECTED) {
				i-- ;
				continue ;
			}
			screen.Sub_no[i] = ICfield.INTfieldno ;
			
			fomwf((char *)&screen) ;
			chkerror() ;
			break ;

		case PREV :
			if(sr.nextfld == SBNO_START) {
				fomer("Field Backword not allowed") ;
				i-- ;
				continue ;
			}
			i -=2 ;
			continue ;

		case DELETE :
			if(screen.Sub_no[i] == 0 ||
					screen.Sub_no[i] == LV_SHORT) {
				fomen("Nothing exists to delete ") ;
				i-- ;
				continue ;
			}

			for(j = i; j < 2 ; j++) {
				screen.Sub_no[j] = screen.Sub_no[j + 1] ;
				strcpy(screen.Sub_totflds[j],
						screen.Sub_totflds[j + 1]) ;
				strcpy(screen.Msg_text[j],
						screen.Msg_text[j + 1]) ;
				strcpy(screen.Keyedit_mask[j],
						screen.Keyedit_mask[j + 1]) ;

				subtotflds[j].infieldno =
						subtotflds[j + 1].infieldno;
				subtotflds[j].inrecno =
						subtotflds[j + 1].inrecno;
			}

			screen.Sub_no[2] = 0 ;
			screen.Sub_totflds[2][0] = LV_CHAR ;
			screen.Msg_text[2][0] = LV_CHAR ;
			screen.Keyedit_mask[2][0] = LV_CHAR ;

			subtotflds[2].infieldno = 0 ;
			subtotflds[2].inrecno = 0 ;

			sr.endfld = KEYMASK_END ;
			fomwr((char *)&screen) ;
			chkerror() ;
			i-- ;
			continue ;

		default :
			i-- ;
			continue ;
		}

		if(screen.Sub_no[i] == 0) break ;	/* No field defined */

		if(sr.retcode == RET_NO_ERROR && sr.fillcode == FIL_DUP) {
						/* already in the list */
			/* now read the mesg. text  */

			fomen("Type the Message Text, printed along with the key field") ;
			sr.nextfld = MSG_START + i * 100 ;
			fomrf((char *)&screen) ;
			chkerror() ;
	
			fomen("Type the Edit mask for the key value display") ;
			sr.nextfld = KEYMASK_START + i * 100 ;
			fomrf((char *)&screen) ;
			chkerror() ;
	
			continue ;
		}

		if((fldlptr = Get_fldelement(screen.Sub_no[i])) == NULL) {
			fomen("Field Number not selected properly ") ;
			i-- ;
			continue ;
		}
		else {	/* display the field name and update the global entr*/
				/* fldattri will have this field info.	*/
			
			strcpy(screen.Sub_totflds[i],fldlptr->Fldname) ;
			sr.nextfld = SBTO_START + i * 100 ;
			fomwf((char *)&screen) ;
			chkerror() ;

			if(fldlptr->Fld_typ != CHAR)
			    Dflt_mask(screen.Keyedit_mask[i],fldlptr->Fld_typ) ;
			else
			    screen.Keyedit_mask[i][0] = '\0' ;

			sr.nextfld = KEYMASK_START + i * 100 ;
			fomwf((char *)&screen) ;
			chkerror() ;

			subtotflds[i].infieldno = fldlptr->Fldno ;
			subtotflds[i].inrecno = fldlptr->Recno ;
		}
		i-- ;
	}

	for(j = i; j < 3 ; j++) {
		screen.Sub_no[j] = LV_SHORT ;
		screen.Sub_totflds[j][0] = LV_CHAR ;
		screen.Msg_text[j][0] = LV_CHAR ;
		screen.Keyedit_mask[j][0] = LV_CHAR ;

		subtotflds[j].infieldno = 0 ;
		subtotflds[j].inrecno = 0 ;	
	}
	sr.nextfld = SBNO_START ;
	sr.endfld = KEYMASK_END ;
	fomwr((char *)&screen) ;
	chkerror() ;
	return(NOERROR) ;
}

/** Given serial# find and return the address of the element in the list */

struct fld_list *
Get_fldelement(fldnum)
int fldnum ;
{
	struct fld_list *curptr = Fld_lsptr ;

	if(fldnum == 0) return(NULL) ;

	while(--fldnum > 0 && curptr != NULL)
		curptr = curptr->nxtfld ;
	return(curptr) ;
}



/** Fill with null character on to the given string	*/

Putnull(str,n)
char *str;
int n ;
{
	while(n-- > 0)
		*(str++) = '\0' ;
	return(NOERROR) ;
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


/** Confirm before exiting from the sreen	*/

Exit_prompt()
{
	if(Get_prompt("Want to continue ? Y(es)/N(o)")  == EXIT)
		return(EXIT) ;
	else
		return(CONTINUE) ;
}





/** Display the prompt line and get response code YES/NO from the user */

Get_prompt(pmtstr)
char *pmtstr ;
{
	int retvalue ;

	strcpy(screen.Prompt,pmtstr) ;
	screen.Pmt_yes = LV_INT ;
	sr.nextfld = PROMPT ;
	sr.endfld = PMT_YES ;
	fomrd((char *)&screen) ;
	chkerror() ;

	retvalue = (screen.Pmt_yes == BOOL_YES) ? CONTINUE : EXIT ;
	
	screen.Prompt[0] = HV_CHAR ; /* suppress the prompt display	*/
	screen.Pmt_yes = HV_INT ;
	sr.nextfld = PROMPT ;
	sr.endfld = PMT_YES ;
	fomwr((char *)&screen) ;
	chkerror() ;

	return(retvalue) ;
}




/** routine to conver integer number to character string	*/
static int
itoa(num,buf)
int num ;
char *buf ;
{
	sprintf(buf,"%d\0",num) ;
	return(NOERROR) ;
}

	

/** If any error occurred in the previous profom call return error	*/
/*  If any escape character is pressed ,check if HELP/DELETE 		*/
/* or EXIT character is pressed then return value			*/

chkerror()
{
	if(sr.retcode == RET_ERROR) {
		fomxy(24,1);
		fprintf(stderr,"PROFOM ERROR Occurred: %d Curfld: %d",
			sr.errno, sr.curfld) ;
		get() ;
		abexit(sr.errno) ;
	}

	if(sr.retcode == RET_USER_ESC) 
		switch(sr.escchar[0]) {

		case 'P' :
		case 'p' :
			return(PREV) ;

		case 'H' :
		case 'h' :
			return(HELP) ;

		case 'D' :
		case 'd' :
			return(DELETE) ;

		case 'E' :
		case 'e' :
			return(EXIT) ;
		
		default : fomer("Not supported by the system ") ;
			return(NOTSUPP) ;
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


/* Copy n chars from buff2 to buff1	*/
scpy(buff1,buff2,n)
char	*buff1,*buff2 ;
int	n ;
{
	while(--n >= 0) *(buff1 + n) = *(buff2 + n) ;
	
	return(NOERROR) ;
}


/****************************************************************************/
/*	This routine,based on an algorithm by Trembley & sorenson,does the
	exhaustive function oif converting a given infix coomputable
	equation into reverse_polist(suffix) form (to facilitate computing
	at a later stage ) -- the operators include * + - / % ! ( )         */
/****************************************************************************/



/*			PROJ-CONV-REV-POL
			-----------------

 			-M.Ravi kumar   */


/* scal for the user-given string & spol for the
	string-to-be-converted-&-passed-to-.hdrbin */

static short scp;


#define MAXCOL		80
#define MAXOP		50 
#define MAXLINES	50
#define OVER		2             

#define AOPERATOR	0		/* + - */
#define BOPERATOR	1		/* * / % */
#define COPERATOR	2		/*  !  */
#define OPERAND		3		/* infld/numl */
#define OPPAR		4		/*  ( */
#define CLPAR		5		/*  )  */

static char lineptr[MAXLINES][MAXOP];
static int precedence_table[6][3]= {
	{ 1, 2,-1},			/*  +, -  	*/
	{ 3, 4,-1},			/*  *, /, %	*/
	{ 5, 6,-1},			/*   ! 	*/
	{ 7, 8, 1},			/*  operand */
	{ 9, 0,-9},			/*  (		*/ /* -9 for NA */
	{ 0,-9,-9}			/*  )		*/
};



static int
squeeze(s,c)					/*  delete all c from s  */
char *s ;
int c;
{
	short i,j ;
	for(i=j=0 ; s[i] != '\0' ; i++)
		if(s[i] != c)
			s[j++] = s[i];
	s[j] = '\0';
}

static int
alter_rp(scal,spol)
char *scal, *spol ;
{
	short	nlines=0;
	short	rank ;
	short	end_index = 0;
	char	s[MAXOP],t[MAXOP] ;

#ifdef DEBUG
	fprintf(stderr,"alter_rp : Formula : %s\n",scal) ;
#endif

	scp= 0;
	squeeze(scal,' ');
	squeeze(scal,'\t');

	rank=0;
	spol[0]= '\0' ;
	while((end_index = sgetop(s,MAXOP,scal)) == NOERROR) {
		if (symbol_test(s)== CLPAR) {
			while(nlines &&
				symbol_test(lineptr[--nlines]) != OPPAR) {

				strcpy(t,lineptr[nlines]);
				strcat(spol,t); strcat(spol," ") ;
				rank+=precedence_table[symbol_test(t)][2];
				if(rank < 1) {
#ifdef DEBUG
					fprintf(stderr,
					"invalid expression  - rank < 1 %s\n",
					scal);
#endif
					return(ERROR) ;
				}
			}
		}
		else {	
			while(nlines &&
			   precedence_table[symbol_test(s)][0] <=
			   precedence_table[symbol_test(lineptr[nlines-1])][1]){
				strcpy(t,lineptr[--nlines]);
				strcat(spol,t);
				strcat(spol," ") ;
#ifdef DEBUG
				printf("loop1 %s %d\n",t,symbol_test(t)) ;
#endif
				rank += precedence_table[symbol_test(t)][2];
				if(rank < 1) {
#ifdef DEBUG
					fprintf(stderr,
					"invalid expression  - rank < 1 %s\n",
					scal);
#endif
					return(ERROR) ;
				}
			}
			strcpy(lineptr[nlines++],s);
		}
	}
	if(end_index == ERROR)
	{
#ifdef DEBUG
		fprintf(stderr,"error in sgetop\n");
#endif
		return(ERROR) ;
	}

	while(nlines) {
		strcat(spol,lineptr[--nlines]) ; strcat(spol," ") ;
		rank+=precedence_table[symbol_test(lineptr[nlines])][2];
	}
	if (rank!=1)
	{
#ifdef DEBUG
		fprintf(stderr,"invalid expression  %s %d\n",scal, rank);
#endif
		return(ERROR) ;
	}
	return(NOERROR) ;
}


/****
		This routine is to return operator or operand(long or i[..])      
*******/


static int
sgetop(s,lim,scal)
char *scal ;
char *s ;
short lim;
{
	short i= 0 ;
	char c,
		 fldbuf[80], *ptr ;
	struct fld_list *tptr,*Get_fldelement() ;
	ptr = fldbuf ;

	if((s[i++]= c= scal[scp++]) == '\0') /* the string is over */
		return(OVER);
	if(c == 'i') { /**** input field ***/
		c=scal[scp++] ;		/*  '['  */
		if(c== '[')
			s[i++]= c ;
		else {
#ifdef DEBUG
			fprintf(stderr,"error in input field format\n");
#endif
			return(ERROR) ;
		}
		for(;(c=scal[scp++]) >= '0' && c <= '9' && c != ']';)
				*ptr++ = c;
		if(c == ']') {
			*ptr = '\0' ;
			if((tptr = Get_fldelement(atoi(fldbuf))) == NULL) {
				sprintf(prom_buf,"Wrong field number in formula : %s",fldbuf) ;
				fomer(prom_buf) ;
				return(ERROR) ;
			}
			if(tptr->Fld_typ == CHAR) {
				sprintf(prom_buf,"Char field : %s in the formula ",fldbuf) ;
				fomer(prom_buf) ;
				return(ERROR) ;
			}
			sprintf(fldbuf,"%d %d\0",tptr->Recno,tptr->Fldno) ;
#ifdef DEBUG
			fprintf(stderr,"FLDNUF : %s \n",fldbuf) ;
#endif
			for(ptr = fldbuf ; *ptr != '\0'; ptr++)
					s[i++] = *ptr ;
			s[i++] = ']';
			s[i] = '\0' ;
#ifdef DEBUG
			fprintf(stderr,"polish form : %s \n",s) ;
#endif
		}			
		else {
#ifdef DEBUG
			fprintf(stderr,"illegal interspersing of ops\n");
#endif
			return(ERROR) ;
		}
		return(NOERROR) ;
	}

	if(c=='+' || c=='-' || c=='/' || c== '*'){
		s[i] = '\0' ;		/* operator	*/
		return(NOERROR) ;
	}

	if(c== '(' || c== ')') {		/* parenthesis */
		s[i] = '\0' ;
		return(NOERROR) ;
	}

	if(c >= '0' && c <= '9') {		/* numeral	*/
		for(;(c=scal[scp++]) >= '0' && c <= '9'; i++)
			s[i] = c ; 
		s[i] = '\0' ;
		scp-- ;
		return(NOERROR) ;
	}
	s[i]='\0';
	sprintf(prom_buf,"Wrong character: '%c' in formula ",c) ;
	fomer(prom_buf) ;
	return(ERROR) ;
}


static int
symbol_test(s)
char *s ;
{
	char c;
	if(((c=s[0]) == 'i') || ((c >= '0') && (c <= '9')))
		return(OPERAND);
	if((c == '+') || (c == '-'))
		return(AOPERATOR);
	if((c == '*') || (c == '/') || (c == '%'))
		return(BOPERATOR);
	switch(c) {
	case '!':
		return(COPERATOR);
	case '(':
		return(OPPAR);
	case ')':
		return(CLPAR);
	default:
		fprintf(stderr,"unknown operator %c\n",c);
		return(ERROR) ;
	}
}


		

