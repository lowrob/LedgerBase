/*-----------------------------------------------------------------------
	Usage :   category_hlp   (categ_num, row, col )
		  categ_num	; selected category # will be moved to this.
		  row, col 	; self explanatory .
------------------------------------------------------------------------*/

category_hlp(categ_num, row, col) 
short	*categ_num ;
int	row,
	col;  	/* window parameters */
{
 	int 		i,k ; 
	Category	category ;
	short		t_categ_num[MAX_LINES];
	int		on_name = 0; /* If Yes help on description	*/

	TOT_STRNG = 51 ;

	/* Make The Starting key */

	/* accept the starting key */
#ifdef ENGLISH
	get_name(row, col, 10, "Starting Category Number: ", kee_name, 0);
#else
	get_name(row, col, 10, "Translate: ",kee_name, 0); 
#endif
	category.categ_num = atoi(kee_name);
	/* Start the file */
	flg_reset(CATEGORY);

#ifdef ENGLISH
	strcpy(line,"No  Category    Description                 ");
#else
	strcpy(line,"No  Code Dept   Desccription                ");
#endif
	expnd_line(line) ;
	height = 0;
	put_blanks = 1 ;

	code = get_n_category(&category, BROWSE, 0, FORWARD, err_msg) ;

	for(;;){
	  i = 1;
	  max_val = 0;
	  for( ; ;code = get_n_category(&category,BROWSE,0,FORWARD,err_msg)){
		if(ERROR == code ) return(DBH_ERR);
		if(code == EFL) break;
#ifdef JRNL
		SettleLock( (char *)&category, CATEGORY, err_msg );
#endif
		if(i == (23 - row - 5) || i == MAX_LINES)break;

		t_categ_num[max_val] = category.categ_num;

		text = line+i*(TOT_STRNG+1);

		sprintf(text, "%1d.   %2d      %-35.35s",
				(max_val+1), category.categ_num,
				category.categ_desc);
		expnd_line(text) ;
		i++;
		max_val++;
	  } 
#ifndef	ORACLE
	  seq_over(CATEGORY);
#endif
	  if(put_blanks == 1 )
		if(i == 1){
#ifdef ENGLISH
			fomen("No Category's to be Displayed ");
#else
			fomen("Pas de department a afficher ");
#endif
			/***
			return(-1);
			***/
			/* Returning 0 to redraw() the screen to wipe
			   out "Starting Number:" message */
			return(0);
		}
  		line[i*(TOT_STRNG+1)] = '\0';
		expnd_line(line+i*(TOT_STRNG+1));
		i++;
		line[i*(TOT_STRNG+1)] = '\0';

		if(i > height)height = i;

		text = line;
		if(window(row, col,WIDTH, (height+4), text, put_blanks ) < 0)
			return(0); 

		/* Get the user Response and Validate against
		   displayed Item Numbers ..**/

		if( code != EFL){
#ifdef ENGLISH
			k = acpt_resp("RETURN for more, #(Item), Q(uit):",
							row+i+2, col+4,1) ;
#else
			k = acpt_resp("RETURN pour plus, #(Article), R(etourner):",
							row+i+2, col+4,1) ;
#endif
		}
		else
		{
#ifdef ENGLISH
			k = acpt_resp("Select #(Item) Or Q(uit)  :",
							row+i+2, col+4,0) ;
#else
			k = acpt_resp("Choisir #(Article) ou R(etourner)  :",
							row+i+2, col+4,0) ;
#endif

		}
		if(k >= 1 && k < i-1){
			*categ_num = t_categ_num[k-1];
			return(1);
		}
		if(k < 0) return(0);
		put_blanks = 2 ;
	}
}  /* category_hlp ends */ 
/*-----------------------------------------------------------------------
	Usage :   catalogue_hlp   (cat_num, row, col )
		  cat_num	; selected catalogue # will be moved to this.
		  row, col 	; self explanatory .
------------------------------------------------------------------------*/

catalogue_hlp(cat_num, row, col) 
long	*cat_num ;
int	row,
	col;  	/* window parameters */
{
 	int 		i,k ; 
	Catalogue	catalogue ;
	long		t_cat_num[MAX_LINES];
	int		on_name = 0; /* If Yes help on description	*/

	TOT_STRNG = 51 ;

	/* Make The Starting key */

	/* accept the starting key */
#ifdef ENGLISH
	get_name(row, col, 10, "Starting Catalogue Number: ", kee_name, 0);
#else
	get_name(row, col, 10, "Translate: ",kee_name, 0); 
#endif
	catalogue.cat_num = atoi(kee_name);
	/* Start the file */
	flg_reset(CATALOGUE);

#ifdef ENGLISH
	strcpy(line,"No  Catalogue    Description                 ");
#else
	strcpy(line,"No  Code Dept   Desccription                ");
#endif
	expnd_line(line) ;
	height = 0;
	put_blanks = 1 ;

	code = get_n_catalogue(&catalogue, BROWSE, 0, FORWARD, err_msg) ;

	for(;;){
	  i = 1;
	  max_val = 0;
	  for( ; ;code = get_n_catalogue(&catalogue,BROWSE,0,FORWARD,err_msg)){
		if(ERROR == code ) return(DBH_ERR);
		if(code == EFL) break;
#ifdef JRNL
		SettleLock( (char *)&catalogue, CATALOGUE, err_msg );
#endif
		if(i == (23 - row - 5) || i == MAX_LINES)break;

		t_cat_num[max_val] = catalogue.cat_num;

		text = line+i*(TOT_STRNG+1);

		sprintf(text, "%1d.  %7ld    %-35.35s",
				(max_val+1), catalogue.cat_num,
				catalogue.cat_desc);
		expnd_line(text) ;
		i++;
		max_val++;
	  } 
#ifndef	ORACLE
	  seq_over(CATALOGUE);
#endif
	  if(put_blanks == 1 )
		if(i == 1){
#ifdef ENGLISH
			fomen("No Catalogue #'s to be Displayed ");
#else
			fomen("Pas de department a afficher ");
#endif
			/***
			return(-1);
			***/
			/* Returning 0 to redraw() the screen to wipe
			   out "Starting Number:" message */
			return(0);
		}
  		line[i*(TOT_STRNG+1)] = '\0';
		expnd_line(line+i*(TOT_STRNG+1));
		i++;
		line[i*(TOT_STRNG+1)] = '\0';

		if(i > height)height = i;

		text = line;
		if(window(row, col,WIDTH, (height+4), text, put_blanks ) < 0)
			return(0); 

		/* Get the user Response and Validate against
		   displayed Item Numbers ..**/

		if( code != EFL){
#ifdef ENGLISH
			k = acpt_resp("RETURN for more, #(Item), Q(uit):",
							row+i+2, col+4,1) ;
#else
			k = acpt_resp("RETURN pour plus, #(Article), R(etourner):",
							row+i+2, col+4,1) ;
#endif
		}
		else
		{
#ifdef ENGLISH
			k = acpt_resp("Select #(Item) Or Q(uit)  :",
							row+i+2, col+4,0) ;
#else
			k = acpt_resp("Choisir #(Article) ou R(etourner)  :",
							row+i+2, col+4,0) ;
#endif

		}
		if(k >= 1 && k < i-1){
			*cat_num = t_cat_num[k-1];
			return(1);
		}
		if(k < 0) return(0);
		put_blanks = 2 ;
	}
}  /* catalogue_hlp ends */ 
