/*------------------------------------------------------------------------
	Source Name: hlp_pp.c
	Created By : T Amarendra.
	Created On : 17-SEP-88.

	Displays help windows with relevent info against item number 1 to 9.
	User can select either an item number indicating the choise or
	give return to get the next set of values in the window. 'Q' will exit
	from window without any selection.

	RETURN VALUES:
	--------------
	Return value 0,1,-1 are to be checked by the user for appropirate
	action.

	Returns :  1 - For successful display of window and selection.
		   0 - Window was displayed but No selection made by user.
		   -1 -No window was displayed so no quesion of selection.

	For DBH or ISAM errors returns DBH_ERR.

	NOTE:
	----- 
		The routines for displaying the help windows read the respective
	dbh file in BROWSE mode. This involves a risk of unlocking any records
	previously locked by the same process. 
		A solution to this is implemented by locking the previously 
	locked record ( if locked ) again.  Function SettleLock() detects the
	lock and relocks the record.
		The detection of lock is possible only if Journalling is ON.
		If journalling is OFF, there is no way to know if a record is
	previously locked, and so the risk mentioned above remains.

-----------------------------------------------------------------------------*/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_pp.h>
#include <bfs_com.h>
#include <fld_defs.h>
#include <cfomstrc.h>

#ifndef	MS_DOS		/* If Not Ms_dos */
#define	O_PROFOM
#endif

#ifdef ENGLISH
#define U_YES		'Y'
#define L_YES		'y'
#define U_QUIT		'Q'
#define L_QUIT		'q'
#else
#define U_YES		'O'
#define L_YES		'o'
#define U_QUIT		'R'
#define L_QUIT		'r'
#endif

/******************* 	Screen Constants & Variables	*****************/

#define MAX_LINES 	10		/* maximun lines in window */
#define WIDTH	(TOT_STRNG+6)	/* Width of a window */

static	int	TOT_STRNG;

static	int	max_val;		/* No of rows in current window */
static	int	height,
		put_blanks;

static	char	*text ; 
static	char	line[600] ;	/* max chars in widow WIDTH*MAX_LINES*/
static	char	err_msg[100];
static	int	code;
static	char	kee_name[50];

/********************   Terminal Constants 	****************/

#define T_DEL	8				/* Backspace*/
#define T_TAB	9				/* Tab */
#define T_NL	10 				/* New line */
#define T_CRF   13 				/* Return   */

/*-----------------------------------------------------------------------
	Usage :   emp_hlp   (emp_code, row, col )
		  emp_code	; selected school# will be moved to this.
		  row, col 	; self explanatory .
------------------------------------------------------------------------*/

emp_hlp(emp_code, row, col) 
short	*emp_code ;
int	row,
	col;  	/* window parameters */
{
 	int 		i,k ; 
	Emp		emp ;
	char		t_emp[MAX_LINES][13] ;
	int		on_name = 0; /* If Yes help on description	*/
	int		key_no;
	char		name[60];
	char		e_mesg[100];

	TOT_STRNG = 51 ;

	/* Make The Starting key */

	if( !on_name ) {	/* If Not on Description */
#ifdef ENGLISH
		get_name(row, col, 1, "Help on Name(Y/N)?      ", kee_name, 0);
#else
		get_name(row, col, 1, "Assistance sur nom (O/N)?         ", kee_name, 0);
#endif
		if(kee_name[0] == L_YES || kee_name[0] == U_YES)
			on_name = 1;
                if (kee_name[0] == NULL) on_name = 1;
	}
	else {
#ifdef ENGLISH
		get_name(row, col, 1, "Help on Employee#(Y/N)? ", kee_name, 0);
#else
		get_name(row, col, 1, "Assistance sur #employee(O/N)? ", kee_name, 0);
#endif
		if(kee_name[0] == L_YES || kee_name[0] == U_YES)
			on_name = 0;
	}

	/* accept the starting key */
	if( on_name) {
#ifdef ENGLISH
		get_name(row, col, 30, "Starting Name:      ", kee_name, 0);
#else
		get_name(row, col, 30, "Nom debutant:       ", kee_name, 0);

#endif
		for(i=0;i<25;i++){
		  if(kee_name[i] == ',')
			break;
		}
		strncpy(emp.em_last_name,kee_name,i);
		i += 2;
		strcpy(emp.em_first_name,kee_name +i);
		key_no = 6;
	}
	else {
#ifdef ENGLISH
		get_name(row,col,12,"Starting on Employee Number: ",kee_name,0);
#else
		get_name(row,col,12,"Numero d'employee debutant: ",kee_name,0); 
#endif
		strcpy(emp.em_numb,kee_name,12);
		Right_Justify_Numeric(emp.em_numb,
					sizeof(emp.em_numb)-1);
		key_no = 0;
	}

	/* Start the file */
	flg_reset(EMPLOYEE);

#ifdef ENGLISH
	strcpy(line,"No  Employee No.   Employee Name               ");
#else
	strcpy(line,"No  Employee No.   Employee Name               ");
#endif
	expnd_line(line) ;
	height = 0;
	put_blanks = 1 ;

	for( ; ; ){
	  code = get_n_employee(&emp, BROWSE, key_no, FORWARD, err_msg);
	  if(code == EFL) break;

	  code = UsrBargVal(BROWSE,emp.em_numb,emp.em_barg,0,e_mesg);
	  if(code < 0){
		if(code == UNDEF){
			fomen(e_mesg);
			get();
			return(ERROR);
		}
	  }
	  else{
		break;
	  }
	}

	for(;;){
	  i = 1;
	  max_val = 0;
	  for( ; ; ){
		if(ERROR == code ) return(DBH_ERR);
		if(code == EFL) break;
#ifdef JRNL
		SettleLock( (char *)&emp, EMPLOYEE, err_msg );
#endif
		if(i == (23 - row - 5) || i == MAX_LINES)break;

		strcpy(t_emp[max_val],emp.em_numb);

		text = line+i*(TOT_STRNG+1);

		strcpy(name, emp.em_last_name);
		strcat(name, ", ");
		strcat(name, emp.em_first_name);
		strcat(name, " ");
		strcat(name, emp.em_mid_name);

		sprintf(text, "%1d.  %-12.12s  %-35.35s",
				(max_val+1), emp.em_numb,
				name);
		expnd_line(text) ;
		i++;
		max_val++;
		for( ; ; ){
		  code = get_n_employee(&emp, BROWSE, key_no, FORWARD, err_msg);
	  	  if(code == EFL) break;

		  code = UsrBargVal(BROWSE,emp.em_numb,emp.em_barg,0,e_mesg);
		  if(code < 0){
			if(code == UNDEF){
				fomen(e_mesg);
				get();
				return(ERROR);
			}
		  }
		  else{
			break;
		  }
		}
	  } 
#ifndef	ORACLE
	  seq_over(EMPLOYEE);
#endif
	  if(put_blanks == 1 )
		if(i == 1){
#ifdef ENGLISH
			fomen("No Employees to be Displayed ");
#else
			fomen("Pas de Employees a afficher ");
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
			strcpy(emp_code,t_emp[k-1]);
			return(1);
		}
		if(k < 0) return(0);
		put_blanks = 2 ;
	}
}  /* emp_hlp ends */ 


/*-----------------------------------------------------------------------
	Usage :   att_disp_hlp   (dept_cd, row, col )
		  att_disp_cd	; Fixed asset dept code
		  row, col 	; self explanatory .
------------------------------------------------------------------------*/

att_disp_hlp(att_disp_cd,row,col) 
char	*att_disp_cd ;
int	row,
	col;  	/* window parameters */
{
 	int 		i,k ; 
	char		t_code[MAX_LINES][5] ;
	Att		att;

	TOT_STRNG = 35 ;

	/* accept the starting key */
#ifdef ENGLISH
	get_name(row, col, 2, "Starting attend code:   ", kee_name, 0);
#else
	get_name(row, col, 2, "Starting attend code:   ", kee_name, 0);
#endif

	/* Make The Starting key */
	strncpy(att.at_disp_code,kee_name,2) ;

	/* Start the file */
	flg_reset(ATT);

#ifdef ENGLISH
	strcpy(line,"No Code     Description") ;
#else
	strcpy(line,"No Code     Description") ;
#endif
	expnd_line(line) ;
	height = 0;
	put_blanks = 1 ;

	code = get_n_att(&att, BROWSE, 0, FORWARD, err_msg) ;

	for(;;){
		i = 1;
		max_val = 0;
		for( ; ;code = get_n_att(&att,BROWSE,0,FORWARD,err_msg)){
			if(ERROR == code )return(DBH_ERR);

			if(code == EFL)break;

#ifdef JRNL
			SettleLock( (char *)&att, ATT, err_msg );
#endif
			if(i == (23 - row - 5) || i == MAX_LINES)break;

			strcpy(t_code[max_val],att.at_disp_code );

			text = line+i*(TOT_STRNG+1);

			sprintf(text, "%1d. %-1.1s %-30.30s",
				(max_val+1), att.at_disp_code, att.at_desc);
			expnd_line(text) ;
			i++;
			max_val++;
		} 
#ifndef	ORACLE
		seq_over(ATT);
#endif
		if(put_blanks == 1 )
			if(i == 1){
#ifdef ENGLISH
				fomen("No Codes to be Displayed ");
#else
				fomen("No Codes to be Displayed ");
#endif
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

		if( code != EFL)
#ifdef ENGLISH
			k = acpt_resp("RETURN for more, #(Item), Q(uit):",
							row+i+2, col+4,1) ;
#else
			k = acpt_resp("RETURN pour plus, #(Article), R(etourner):",
							row+i+2, col+4,1) ;
#endif
		else
#ifdef ENGLISH
			k = acpt_resp("Select #(Item) Or Q(uit)  :",
							row+i+2, col+4,0) ;
#else
			k = acpt_resp("Choisir #(Article) ou R(etourner)  :",
							row+i+2, col+4,0) ;
#endif

		if ( k >= 1 && k < i-1 ){
			strcpy(att_disp_cd, t_code[k - 1]) ;
			return(1) ;
		}
		if(k < 0)return(0);
		put_blanks = 2 ;
	}
} 
/*-----------------------------------------------------------------------
	Usage :   att_hlp   (att_cd, row, col )
		  att_disp_cd	; Fixed asset dept code
		  row, col 	; self explanatory .
------------------------------------------------------------------------*/

att_hlp(att_cd,row,col) 
char	*att_cd ;
int	row,
	col;  	/* window parameters */
{
 	int 		i,k ; 
	char		t_code[MAX_LINES][5] ;
	Att		att;

	TOT_STRNG = 51 ;

	/* accept the starting key */
#ifdef ENGLISH
	get_name(row, col, 2, "Starting attendance code:", kee_name, 0);
#else
	get_name(row, col, 2, "Starting attendance code:", kee_name, 0);
#endif

	/* Make The Starting key */
	strncpy(att.at_code,kee_name,2) ;

	/* Start the file */
	flg_reset(ATT);

#ifdef ENGLISH
	strcpy(line,"No Disp Code   Code     Description         Units") ;
#else
	strcpy(line,"No Code     Description") ;
#endif
	expnd_line(line) ;
	height = 0;
	put_blanks = 1 ;

	code = get_n_att(&att, BROWSE, 1, FORWARD, err_msg) ;

	for(;;){
		i = 1;
		max_val = 0;
		for( ; ;code = get_n_att(&att,BROWSE,1,FORWARD,err_msg)){
			if(ERROR == code )return(DBH_ERR);

			if(code == EFL)break;

#ifdef JRNL
			SettleLock( (char *)&att, ATT, err_msg );
#endif
			if(i == (23 - row - 5) || i == MAX_LINES)break;

			strcpy(t_code[max_val],att.at_code );

			text = line+i*(TOT_STRNG+1);

			sprintf(text, "%1d.     %-1.1s       %-3.3s    %-22.22s",
				(max_val+1), att.at_disp_code, att.at_code,
				att.at_desc);
			expnd_line(text) ;
			i++;
			max_val++;
		} 
#ifndef	ORACLE
		seq_over(ATT);
#endif
		if(put_blanks == 1 )
			if(i == 1){
#ifdef ENGLISH
				fomen("No Codes to be Displayed ");
#else
				fomen("No Codes to be Displayed ");
#endif
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

		if( code != EFL)
#ifdef ENGLISH
			k = acpt_resp("RETURN for more, #(Item), Q(uit):",
							row+i+2, col+4,1) ;
#else
			k = acpt_resp("RETURN pour plus, #(Article), R(etourner):",
							row+i+2, col+4,1) ;
#endif
		else
#ifdef ENGLISH
			k = acpt_resp("Select #(Item) Or Q(uit)  :",
							row+i+2, col+4,0) ;
#else
			k = acpt_resp("Choisir #(Article) ou R(etourner)  :",
							row+i+2, col+4,0) ;
#endif

		if ( k >= 1 && k < i-1 ){
			strcpy(att_cd, t_code[k - 1]) ;
			return(1) ;
		}
		if(k < 0)return(0);
		put_blanks = 2 ;
	}
} 


/*-----------------------------------------------------------------------
	Usage :   dept_hlp   (d_code, row, col )
		  d_code	; selected school# will be moved to this.
		  row, col 	; self explanatory .
------------------------------------------------------------------------*/

dept_hlp(d_code, row, col) 
short	*d_code ;
int	row,
	col;  	/* window parameters */
{
 	int 		i,k ; 
	Dept		department ;
	char		t_dept_code[MAX_LINES][7] ;
	int		on_name = 0; /* If Yes help on description	*/

	TOT_STRNG = 51 ;

	/* Make The Starting key */

	/* accept the starting key */
#ifdef ENGLISH
	get_name(row, col, 10, "Starting on Department Code: ", kee_name, 0);
#else
	get_name(row, col, 10, "Code de departement debutant: ",kee_name, 0); 
#endif
	strcpy(department.d_code,kee_name,6);
	Right_Justify_Numeric(department.d_code,
					sizeof(department.d_code)-1);
	/* Start the file */
	flg_reset(DEPARTMENT);

#ifdef ENGLISH
	strcpy(line,"No  Dept Code   Description                 ");
#else
	strcpy(line,"No  Code Dept   Desccription                ");
#endif
	expnd_line(line) ;
	height = 0;
	put_blanks = 1 ;

	code = get_n_dept(&department, BROWSE, 0, FORWARD, err_msg) ;

	for(;;){
	  i = 1;
	  max_val = 0;
	  for( ; ;code = get_n_dept(&department,BROWSE,0,FORWARD,err_msg)){
		if(ERROR == code ) return(DBH_ERR);
		if(code == EFL) break;
#ifdef JRNL
		SettleLock( (char *)&department, DEPARTMENT, err_msg );
#endif
		if(i == (23 - row - 5) || i == MAX_LINES)break;

		strcpy(t_dept_code[max_val],department.d_code);

		text = line+i*(TOT_STRNG+1);

		sprintf(text, "%1d.  %-10.10s  %-35.35s",
				(max_val+1), department.d_code,
				department.d_desc);
		expnd_line(text) ;
		i++;
		max_val++;
	  } 
#ifndef	ORACLE
	  seq_over(DEPARTMENT);
#endif
	  if(put_blanks == 1 )
		if(i == 1){
#ifdef ENGLISH
			fomen("No Departments to be Displayed ");
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
			strcpy(d_code,t_dept_code[k-1]);
			return(1);
		}
		if(k < 0) return(0);
		put_blanks = 2 ;
	}
}  /* dept_hlp ends */ 
/*-----------------------------------------------------------------------
	Usage :   area_spec_hlp   (as_code, row, col )
		  as_code	; selected school# will be moved to this.
		  row, col 	; self explanatory .
------------------------------------------------------------------------*/

area_spec_hlp(as_code, row, col) 
short	*as_code ;
int	row,
	col;  	/* window parameters */
{
 	int 		i,k ; 
	Area_spec	area_spec ;
	char		t_as_code[MAX_LINES][7] ;
	int		on_name = 0; /* If Yes help on description	*/

	TOT_STRNG = 51 ;

	/* Make The Starting key */

	/* accept the starting key */
#ifdef ENGLISH
	get_name(row, col, 10, "Starting on Area of Specialization Code: ", kee_name, 0);
#else
	get_name(row, col, 10, "Code de area of specialization debutant: ",kee_name, 0); 
#endif
	strcpy(area_spec.ar_code,kee_name,6);
	Right_Justify_Numeric(area_spec.ar_code,
					sizeof(area_spec.ar_code)-1);
	/* Start the file */
	flg_reset(AREA_SPEC);

#ifdef ENGLISH
	strcpy(line,"No  Area Spec. Code   Description                 ");
#else
	strcpy(line,"No  Code Area Spec.   Desccription                ");
#endif
	expnd_line(line) ;
	height = 0;
	put_blanks = 1 ;

	code = get_n_area_spec(&area_spec, BROWSE, 0, FORWARD, err_msg) ;

	for(;;){
	  i = 1;
	  max_val = 0;
	  for( ; ;code = get_n_area_spec(&area_spec,BROWSE,0,FORWARD,err_msg)){
		if(ERROR == code ) return(DBH_ERR);
		if(code == EFL) break;
#ifdef JRNL
		SettleLock( (char *)&area_spec, AREA_SPEC, err_msg );
#endif
		if(i == (23 - row - 5) || i == MAX_LINES)break;

		strcpy(t_as_code[max_val],area_spec.ar_code);

		text = line+i*(TOT_STRNG+1);

		sprintf(text, "%1d.  %-10.10s  %-35.35s",
				(max_val+1), area_spec.ar_code,
				area_spec.ar_desc);
		expnd_line(text) ;
		i++;
		max_val++;
	  } 
#ifndef	ORACLE
	  seq_over(AREA_SPEC);
#endif
	  if(put_blanks == 1 )
		if(i == 1){
#ifdef ENGLISH
			fomen("No Area of Speicialization to be Displayed ");
#else
			fomen("Pas de Area of Speicializationa afficher ");
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
			strcpy(as_code,t_as_code[k-1]);
			return(1);
		}
		if(k < 0) return(0);
		put_blanks = 2 ;
	}
}  /* area_spec_hlp ends */ 
/*-----------------------------------------------------------------------
	Usage :   term_hlp   (t_code, row, col )
		  t_code	; selected school# will be moved to this.
		  row, col 	; self explanatory .
------------------------------------------------------------------------*/

term_hlp(t_code, row, col) 
char	*t_code ;
int	row,
	col;  	/* window parameters */
{
 	int 		i,k ; 
	Term		termination ;
	char		t_term_code[MAX_LINES][7] ;
	int		on_name = 0; /* If Yes help on description	*/

	TOT_STRNG = 51 ;

	/* Make The Starting key */

	/* accept the starting key */
#ifdef ENGLISH
	get_name(row, col, 10, "Starting on Termination Code: ", kee_name, 0);
#else
	get_name(row, col, 10, "Code de termination debutant: ",kee_name, 0); 
#endif
	strcpy(termination.t_code,kee_name,6);
	Right_Justify_Numeric(termination.t_code,
					sizeof(termination.t_code)-1);
	/* Start the file */
	flg_reset(TERM);

#ifdef ENGLISH
	strcpy(line,"No  Term Code   Description                 ");
#else
	strcpy(line,"No  Code Term   Desccription                ");
#endif
	expnd_line(line) ;
	height = 0;
	put_blanks = 1 ;

	code = get_n_pterm(&termination, BROWSE, 0, FORWARD, err_msg) ;

	for(;;){
	  i = 1;
	  max_val = 0;
	  for( ; ;code = get_n_pterm(&termination,BROWSE,0,FORWARD,err_msg)){
		if(ERROR == code ) return(DBH_ERR);
		if(code == EFL) break;
#ifdef JRNL
		SettleLock( (char *)&termination, TERM, err_msg );
#endif
		if(i == (23 - row - 5) || i == MAX_LINES)break;

		strcpy(t_term_code[max_val],termination.t_code);

		text = line+i*(TOT_STRNG+1);

		sprintf(text, "%1d.  %-10.10s  %-35.35s",
				(max_val+1), termination.t_code,
				termination.t_desc);
		expnd_line(text) ;
		i++;
		max_val++;
	  } 
#ifndef	ORACLE
	  seq_over(termination);
#endif
	  if(put_blanks == 1 )
		if(i == 1){
#ifdef ENGLISH
			fomen("No terminations to be Displayed ");
#else
			fomen("Pas de termination a afficher ");
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
			strcpy(t_code,t_term_code[k-1]);
			return(1);
		}
		if(k < 0) return(0);
		put_blanks = 2 ;
	}
}  /* term_hlp ends */ 
/*-----------------------------------------------------------------------
	Usage :   loan_hlp  	 (cs_code, row, col )
		  cs_code	; selected code will be moved to this.
		  row, col 	; self explanatory .
------------------------------------------------------------------------*/

loan_hlp(cs_code, row, col) 
char	*cs_code ;
int	row,
	col;  	/* window parameters */
{
 	int 		i,k ; 
	Csb_loan	csb_loan ;
	char		t_csb_code[MAX_LINES][7] ;
	int		on_name = 0; /* If Yes help on description	*/

	TOT_STRNG = 51 ;

	/* Make The Starting key */

	/* accept the starting key */
#ifdef ENGLISH
	get_name(row, col, 10, "Starting on CSB/Loan Code: ", kee_name, 0);
#else
	get_name(row, col, 10, "Code de departement debutant: ",kee_name, 0); 
#endif
	strcpy(csb_loan.cs_code,kee_name,6);
	Right_Justify_Numeric(csb_loan.cs_code,
					sizeof(csb_loan.cs_code)-1);
	/* Start the file */
	flg_reset(LOAN);

#ifdef ENGLISH
	strcpy(line,"No  CSB/Loan Code   Description                 ");
#else
	strcpy(line,"No  Code CSB/Loan   Desccription                ");
#endif
	expnd_line(line) ;
	height = 0;
	put_blanks = 1 ;

	code = get_n_loan(&csb_loan, BROWSE, 0, FORWARD, err_msg) ;

	for(;;){
	  i = 1;
	  max_val = 0;
	  for( ; ;code = get_n_loan(&csb_loan,BROWSE,0,FORWARD,err_msg)){
		if(ERROR == code ) return(DBH_ERR);
		if(code == EFL) break;
#ifdef JRNL
		SettleLock( (char *)&csb_loan, LOAN, err_msg );
#endif
		if(i == (23 - row - 5) || i == MAX_LINES)break;

		strcpy(t_csb_code[max_val],csb_loan.cs_code);

		text = line+i*(TOT_STRNG+1);

		sprintf(text, "%1d.  %-10.10s  %-35.35s",
				(max_val+1), csb_loan.cs_code,
				csb_loan.cs_desc);
		expnd_line(text) ;
		i++;
		max_val++;
	  } 
#ifndef	ORACLE
	  seq_over(LOAN);
#endif
	  if(put_blanks == 1 )
		if(i == 1){
#ifdef ENGLISH
			fomen("No CSB/Loan to be Displayed ");
#else
			fomen("Pas de CSB/Loan a afficher ");
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
			strcpy(cs_code,t_csb_code[k-1]);
			return(1);
		}
		if(k < 0) return(0);
		put_blanks = 2 ;
	}
}  /* loan_hlp ends */ 
/*-----------------------------------------------------------------------
	Usage :   emp_loan_hlp 	 (el_numb, el_code, el_seq, row, col )
		  row, col 	; self explanatory .
------------------------------------------------------------------------*/

emp_loan_hlp(el_numb, el_code, el_seq, row, col) 
char	*el_numb ;
char	*el_code ;
short	*el_seq ;
int	row,
	col;  	/* window parameters */
{
 	int 		i,k ; 
	Emp_loan	emp_loan ;
	char		t_emp_numb[MAX_LINES][13] ;
	char		t_emp_code[MAX_LINES][7] ;
	short		t_emp_seq[MAX_LINES] ;
	int		on_name = 0; /* If Yes help on description	*/

	TOT_STRNG = 51 ;

	/* Make The Starting key */

	/* accept the starting key */
#ifdef ENGLISH
	get_name(row, col, 10, "Starting on Employee Number: ", kee_name, 0);
#else
	get_name(row, col, 10, "Numero d'Employee debutant: ",kee_name, 0); 
#endif
	strcpy(emp_loan.el_numb,kee_name,12);
	emp_loan.el_code[0] = '\0';
	emp_loan.el_seq = 0;
	Right_Justify_Numeric(emp_loan.el_numb,
					sizeof(emp_loan.el_numb)-1);
	/* Start the file */
	flg_reset(EMP_LOAN);

#ifdef ENGLISH
	strcpy(line,"No  Employee Number   CSB/Loan Code  Sequence #     ");
#else
	strcpy(line,"No  Employee Number   CSB/Loan Code  Sequence #     ");
#endif
	expnd_line(line) ;
	height = 0;
	put_blanks = 1 ;

	code = get_n_emp_loan(&emp_loan, BROWSE, 0, FORWARD, err_msg) ;

	for(;;){
	  i = 1;
	  max_val = 0;
	  for( ; ;code = get_n_emp_loan(&emp_loan,BROWSE,0,FORWARD,err_msg)){
		if(ERROR == code ) return(DBH_ERR);
		if(code == EFL) break;
#ifdef JRNL
		SettleLock( (char *)&emp_loan, EMP_LOAN, err_msg );
#endif
		if(i == (23 - row - 5) || i == MAX_LINES)break;

		strcpy(t_emp_numb[max_val],emp_loan.el_numb);
		strcpy(t_emp_code[max_val],emp_loan.el_code);
		t_emp_seq[max_val] = emp_loan.el_seq;

		text = line+i*(TOT_STRNG+1);

		sprintf(text, "%1d.  %-12.12s          %-6.6s       %-2.2d",
				(max_val+1), emp_loan.el_numb,
				emp_loan.el_code,emp_loan.el_seq);
		expnd_line(text) ;
		i++;
		max_val++;
	  } 
#ifndef	ORACLE
	  seq_over(EMP_LOAN);
#endif
	  if(put_blanks == 1 )
		if(i == 1){
#ifdef ENGLISH
			fomen("No Employee Number to be Displayed ");
#else
			fomen("Pas de Numero d'Employees a afficher ");
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
			strcpy(el_numb,t_emp_numb[k-1]);
			strcpy(el_code,t_emp_code[k-1]);
			*el_seq = t_emp_seq[k-1];
			return(1);
		}
		if(k < 0) return(0);
		put_blanks = 2 ;
	}
}  /* emp_loan_hlp ends */ 
/*-----------------------------------------------------------------------
	Usage :   emp_garn_hlp 	 (eg_numb, eg_pr_cd, eg_seq, row, col )
		  row, col 	; self explanatory .
------------------------------------------------------------------------*/

emp_garn_hlp(eg_numb, eg_pr_cd, eg_seq, row, col) 
char	*eg_numb ;
short	*eg_pr_cd ;
short	*eg_seq ;
int	row,
	col;  	/* window parameters */
{
 	int 		i,k ; 
	Emp_garn	emp_garn ;
	char		t_emp_numb[MAX_LINES][13] ;
	short		t_emp_pr_cd[MAX_LINES];
	short		t_emp_seq[MAX_LINES];
	int		on_name = 0; /* If Yes help on description	*/

	TOT_STRNG = 51 ;

	/* Make The Starting key */

	/* accept the starting key */
#ifdef ENGLISH
	get_name(row, col, 10, "Starting on Employee Number: ", kee_name, 0);
#else
	get_name(row, col, 10, "Numero d'Employee debutant: ",kee_name, 0); 
#endif
	strcpy(emp_garn.eg_numb,kee_name,12);
	emp_garn.eg_pr_cd = 0;
	emp_garn.eg_seq = 0;
	Right_Justify_Numeric(emp_garn.eg_numb,
					sizeof(emp_garn.eg_numb)-1);
	/* Start the file */
	flg_reset(EMP_GARN);

#ifdef ENGLISH
	strcpy(line,"No  Employee Number   Priority Code  Sequence #     ");
#else
	strcpy(line,"No  Employee Number   Priority Code  Sequence #     ");
#endif
	expnd_line(line) ;
	height = 0;
	put_blanks = 1 ;

	code = get_n_emp_garn(&emp_garn, BROWSE, 0, FORWARD, err_msg) ;

	for(;;){
	  i = 1;
	  max_val = 0;
	  for( ; ;code = get_n_emp_garn(&emp_garn,BROWSE,0,FORWARD,err_msg)){
		if(ERROR == code ) return(DBH_ERR);
		if(code == EFL) break;
#ifdef JRNL
		SettleLock( (char *)&emp_garn, EMP_GARN, err_msg );
#endif
		if(i == (23 - row - 5) || i == MAX_LINES)break;

		strcpy(t_emp_numb[max_val],emp_garn.eg_numb);
		t_emp_pr_cd[max_val] = emp_garn.eg_pr_cd;
		t_emp_seq[max_val] = emp_garn.eg_seq;

		text = line+i*(TOT_STRNG+1);

		sprintf(text, "%1d.  %-12.12s          %-2.2d       %-2.2d",
				(max_val+1), emp_garn.eg_numb,
				emp_garn.eg_pr_cd,emp_garn.eg_seq);
		expnd_line(text) ;
		i++;
		max_val++;
	  } 
#ifndef	ORACLE
	  seq_over(EMP_GARN);
#endif
	  if(put_blanks == 1 )
		if(i == 1){
#ifdef ENGLISH
			fomen("No Employee Number to be Displayed ");
#else
			fomen("Pas de Numero d'Employees a afficher ");
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
			strcpy(eg_numb,t_emp_numb[k-1]);
			*eg_pr_cd = t_emp_pr_cd[k-1];
			*eg_seq = t_emp_seq[k-1];
			return(1);
		}
		if(k < 0) return(0);
		put_blanks = 2 ;
	}
}  /* emp_garn_hlp ends */ 
/*-----------------------------------------------------------------------
	Usage :   area_hlp   (d_code,a_code, row, col )
		  d_code	; Department Code.
		  a_code	; Area Code.
		  row, col 	; self explanatory .
------------------------------------------------------------------------*/

area_hlp(d_code,a_code, row, col) 
char	*d_code ;
char	*a_code ;
int	row,
	col;  	/* window parameters */
{
 	int 		i,k ; 
	Area		area ;
	char		t_code[MAX_LINES][7] ;
	int		on_name = 0; /* If Yes help on description	*/

	TOT_STRNG = 51 ;

	/* Make The Starting key */

	/* accept the starting key */
#ifdef ENGLISH
	get_name(row, col, 10, "Starting on Area Code: ", kee_name, 0);
#else
	get_name(row, col, 10, "Code de departement debutant: ", kee_name, 0);
#endif
	strcpy(area.a_deptcode,d_code);
	strncpy(area.a_code,kee_name,6);
	/* Start the file */
	flg_reset(AREA);

#ifdef ENGLISH
	strcpy(line,"No  Dept Code  Area Code  Description           ");
#else
	strcpy(line,"No  Code Dept   Desccription                ");
#endif
	expnd_line(line) ;
	height = 0;
	put_blanks = 1 ;

	code = get_n_area(&area, BROWSE, 0, FORWARD, err_msg) ;

	for(;;){
		i = 1;
		max_val = 0;
		for( ; ;code = get_n_area(&area,BROWSE,0,FORWARD,err_msg)){
			if(ERROR == code )return(DBH_ERR);

			if(code == EFL)break;

			/*if(strcmp(area.a_code,d_code) != 0) break;*/

#ifdef JRNL
			SettleLock( (char *)&area, AREA, err_msg );
#endif
			if(i == (23 - row - 5) || i == MAX_LINES)break;

			strcpy(t_code[max_val],area.a_code);

			text = line+i*(TOT_STRNG+1);

			sprintf(text, "%1d.    %-6.6s     %-6.6s    %-30.30s",
				(max_val+1), area.a_deptcode,
				area.a_code,area.a_desc);
			expnd_line(text) ;
			i++;
			max_val++;
		} 
#ifndef	ORACLE
		seq_over(AREA);
#endif
		if(put_blanks == 1 )
			if(i == 1){
#ifdef ENGLISH
				fomen("No Areas to be Displayed ");
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

		if(k >= 1 && k < i-1 ) {
			strcpy(a_code,t_code[k-1]) ;
			return(1);
		}
		if(k < 0) return(0);
		put_blanks = 2 ;
	}
}  /* area_hlp ends */
/*-----------------------------------------------------------------------
	Usage :   exp_hlp   (ex_code, row, col )
		  ex_code	; expense Code.
		  row, col 	; self explanatory .
------------------------------------------------------------------------*/

exp_hlp(ex_code, row, col) 
char	*ex_code ;
int	row,
	col;  	/* window parameters */
{
 	int 		i,k ; 
	Exp		exp;
	char		e_code[MAX_LINES][7] ;
	int		on_name = 0; /* If Yes help on description	*/

	TOT_STRNG = 51 ;

	/* Make The Starting key */

	/* accept the starting key */
#ifdef ENGLISH
	get_name(row, col, 10, "Starting on Expense Code: ", kee_name, 0);
#else
	get_name(row, col, 10, "Starting on Expense Code: ", kee_name, 0);
#endif
	strncpy(exp.ex_code,kee_name,6);
	strcat(exp.ex_code,"\0");
	/* Start the file */
	flg_reset(EXPENSE);

#ifdef ENGLISH
	strcpy(line,"No  Exp Code  Description           ");
#else
	strcpy(line,"No  Exp Code  Description           ");
#endif
	expnd_line(line) ;
	height = 0;
	put_blanks = 1 ;

	code = get_n_exp(&exp, BROWSE, 0, FORWARD, err_msg) ;

	for(;;){
		i = 1;
		max_val = 0;
		for( ; ;code = get_n_exp(&exp,BROWSE,0,FORWARD,err_msg)){
			if(ERROR == code )return(DBH_ERR);
			if(code == EFL)break;

#ifdef JRNL
			SettleLock( (char *)&exp, EXPENSE, err_msg );
#endif
			if(i == (23 - row - 5) || i == MAX_LINES)break;

			strcpy(e_code[max_val],exp.ex_code);

			text = line+i*(TOT_STRNG+1);

			sprintf(text, "%1d.    %-6.6s     %-30.30s",
				(max_val+1), exp.ex_code,exp.ex_desc);
			expnd_line(text) ;
			i++;
			max_val++;
		} 
#ifndef	ORACLE
		seq_over(EXPENSE);
#endif
		if(put_blanks == 1 )
			if(i == 1){
#ifdef ENGLISH
				fomen("No Expense codes to be Displayed ");
#else
				fomen("No Expense codes to be Displayed ");
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

		if(k >= 1 && k < i-1 ) {
			strcpy(ex_code,e_code[k-1]) ;
			return(1);
		}
		if(k < 0) return(0);
		put_blanks = 2 ;
	}
}  /* expense help ends */
/*-----------------------------------------------------------------------
	Usage :   trans_hlp   (tr_code, row, col )
		  tr_code	; transaction Code.
		  row, col 	; self explanatory .
------------------------------------------------------------------------*/

trans_hlp(tr_code, row, col) 
char	*tr_code ;
int	row,
	col;  	/* window parameters */
{
 	int 		i,k ; 
	Trans		trans;
	char		t_code[MAX_LINES][7] ;
	int		on_name = 0; /* If Yes help on description	*/

	TOT_STRNG = 51 ;

	/* Make The Starting key */

	/* accept the starting key */
#ifdef ENGLISH
	get_name(row, col, 10, "Starting on Trans Code: ", kee_name, 0);
#else
	get_name(row, col, 10, "Starting on Trans Code: ", kee_name, 0);
#endif
	strncpy(trans.tr_code,kee_name,6);
	strcat(trans.tr_code,"\0");
	/* Start the file */
	flg_reset(TRANS);

#ifdef ENGLISH
	strcpy(line,"No  Trans Code  Description           ");
#else
	strcpy(line,"No  Trans Code  Description           ");
#endif
	expnd_line(line) ;
	height = 0;
	put_blanks = 1 ;

	code = get_n_trans(&trans, BROWSE, 0, FORWARD, err_msg) ;

	for(;;){
		i = 1;
		max_val = 0;
		for( ; ;code = get_n_trans(&trans,BROWSE,0,FORWARD,err_msg)){
			if(ERROR == code )return(DBH_ERR);
			if(code == EFL)break;
#ifdef JRNL
			SettleLock( (char *)&trans, TRANS, err_msg );
#endif
			if(i == (23 - row - 5) || i == MAX_LINES)break;

			strcpy(t_code[max_val],trans.tr_code);

			text = line+i*(TOT_STRNG+1);

			sprintf(text, "%1d.    %-6.6s     %-30.30s",
				(max_val+1), trans.tr_code,trans.tr_desc);
			expnd_line(text) ;
			i++;
			max_val++;
		} 
#ifndef	ORACLE
		seq_over(TRANS);
#endif
		if(put_blanks == 1 )
			if(i == 1){
#ifdef ENGLISH
				fomen("No Trans codes to be Displayed ");
#else
				fomen("No Trans codes to be Displayed ");
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

		if(k >= 1 && k < i-1 ) {
			strcpy(tr_code,t_code[k-1]) ;
			return(1);
		}
		if(k < 0) return(0);
		put_blanks = 2 ;
	}
}  /* transaction help ends */
/*-----------------------------------------------------------------------
	Usage :   position_hlp   (p_code, row, col )
		  p_code	; selected school# will be moved to this.
		  row, col 	; self explanatory .
------------------------------------------------------------------------*/

position_hlp(p_code, row, col) 
char	*p_code ;
int	row,
	col;  	/* window parameters */
{
 	int 		i,k ; 
	Position	position ;
	char		t_code[MAX_LINES][7] ;
	int		on_name = 0; /* If Yes help on description	*/

	TOT_STRNG = 51 ;

	/* Make The Starting key */

	/* accept the starting key */
#ifdef ENGLISH
	get_name(row, col, 10, "Starting on Position Code: ", kee_name, 0);
#else
	get_name(row, col, 10, "Code de positiont debutant: ",kee_name, 0); 
#endif
	strncpy(position.p_code,kee_name,6);
	Right_Justify_Numeric(position.p_code,
					sizeof(position.p_code)-1);
	/* Start the file */
	flg_reset(POSITION);

#ifdef ENGLISH
	strcpy(line,"No  Position Code   Description             ");
#else
	strcpy(line,"No  Code Position   Desccription            ");
#endif
	expnd_line(line) ;
	height = 0;
	put_blanks = 1 ;

	code = get_n_position(&position, BROWSE, 0, FORWARD, err_msg) ;

	for(;;){
	  i = 1;
	  max_val = 0;
	  for( ; ;code = get_n_position(&position,BROWSE,0,FORWARD,err_msg)){
		if(ERROR == code ) return(DBH_ERR);
		if(code == EFL) break;
#ifdef JRNL
		SettleLock( (char *)&position, POSITION, err_msg );
#endif
		if(i == (23 - row - 5) || i == MAX_LINES)break;

		strcpy(t_code[max_val],position.p_code);

		text = line+i*(TOT_STRNG+1);

		sprintf(text, "%1d.  %-6.6s  %-30.30s",
				(max_val+1), position.p_code,
				position.p_desc);
		expnd_line(text) ;
		i++;
		max_val++;
	  } 
#ifndef	ORACLE
	  seq_over(POSITION);
#endif
	  if(put_blanks == 1 )
		if(i == 1){
#ifdef ENGLISH
			fomen("No Positions to be Displayed ");
#else
			fomen("Pas de position a afficher ");
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
			strcpy(p_code,t_code[k-1]);
			return(1);
		}
		if(k < 0) return(0);
		put_blanks = 2 ;
	}
}  /* position_hlp ends */ 

/*-----------------------------------------------------------------------
	Usage :   class_hlp   (cl_code,cl_date, row, col )
		  cl_code	; Classification Code.
		  cl_date	; Effective Date.
		  row, col 	; self explanatory .
------------------------------------------------------------------------*/

class_hlp(cl_code,cl_date, row, col) 
char	*cl_code ;
long	*cl_date ;
int	row,
	col;  	/* window parameters */
{
 	int 		i,k ; 
	Class		class ;
	char		t_code[MAX_LINES][7] ;
	long		t_date[MAX_LINES] ;
	int		on_name = 0; /* If Yes help on description	*/
	char		year[5];
	char		month[3];
	char		day[3];

	TOT_STRNG = 51 ;

	/* Make The Starting key */

	if(cl_code[0] == '\0') {
		/* accept the starting key */
#ifdef ENGLISH
		get_name(row, col, 10, "Starting on Code: ", kee_name, 0);
#else
		get_name(row, col, 10, "Code de departement debutant: ", kee_name, 0);
#endif
		strncpy(class.c_code,kee_name,6);
	}
	else {
		strcpy(class.c_code,cl_code);
	}

	class.c_date = *cl_date;

	/* Start the file */
	flg_reset(CLASSIFICATION);

#ifdef ENGLISH
	strcpy(line,"No  Code    Date    Description           ");
#else
	strcpy(line,"No  Code    Date    Description           ");
#endif
	expnd_line(line) ;
	height = 0;
	put_blanks = 1 ;

	code = get_n_class(&class, BROWSE, 0, FORWARD, err_msg) ;

	for(;;){
		i = 1;
		max_val = 0;
		for( ; ;code = get_n_class(&class,BROWSE,0,FORWARD,err_msg)){
			if(ERROR == code )return(DBH_ERR);

			if(code == EFL)break;

			if(cl_code[0] != '\0') {
				if(strcmp(class.c_code,cl_code) != 0) break;
			}
#ifdef JRNL
			SettleLock( (char *)&class, CLASSIFICATION, err_msg );
#endif
			if(i == (23 - row - 5) || i == MAX_LINES)break;

			strcpy(t_code[max_val],class.c_code);
			t_date[max_val] = class.c_date;

			text = line+i*(TOT_STRNG+1);

			sprintf(year,"%ld",class.c_date / 10000);
			sprintf(month,"%02ld",(class.c_date / 100) % 100);
			sprintf(day,"%02ld",class.c_date % 100);

			sprintf(text, "%1d. %-6.6s %s/%s/%s %-30.30s",
				(max_val+1), class.c_code,year,month,day,
				class.c_desc);
			expnd_line(text) ;
			i++;
			max_val++;
		} 
#ifndef	ORACLE
		seq_over(CLASSIFICATION);
#endif
		if(put_blanks == 1 )
			if(i == 1){
#ifdef ENGLISH
				fomen("No Classification Codes to be Displayed ");
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

		if(k >= 1 && k < i-1 ) {
			strcpy(cl_code,t_code[k-1]) ;
			*cl_date = t_date[k-1]; 
			return(1);
		}
		if(k < 0) return(0);
		put_blanks = 2 ;
	}
}  /* class_hlp ends */

/*-----------------------------------------------------------------------
	Usage :   payper_hlp   (pp_code, row, col )
		  pp_code	; Pay Period Code.
		  row, col 	; self explanatory .
------------------------------------------------------------------------*/

payper_hlp(pp_code, row, col) 
char	*pp_code ;
int	row,
	col;  	/* window parameters */
{
 	int 		i,k ; 
	Pay_per 	pp_rec ;
	char		t_code[MAX_LINES][7] ;
	int		on_name = 0; /* If Yes help on description	*/

	TOT_STRNG = 51 ;

	/* Make The Starting key */

	/* accept the starting key */
#ifdef ENGLISH
	get_name(row, col, 10, "Starting on Code: ", kee_name, 0);
#else
	get_name(row, col, 10, "Code de departement debutant: ", kee_name, 0);
#endif

	strncpy(pp_rec.pp_code,kee_name,6);

	/* Start the file */
	flg_reset(PAY_PERIOD);

#ifdef ENGLISH
	strcpy(line,"No  Code Number Periods Description           ");
#else
	strcpy(line,"No  Code Number Periods Description           ");
#endif
	expnd_line(line) ;
	height = 0;
	put_blanks = 1 ;

	code = get_n_pay_per(&pp_rec, BROWSE, 0, FORWARD, err_msg) ;

	for(;;){
		i = 1;
		max_val = 0;
		for( ; ;code = get_n_pay_per(&pp_rec,BROWSE,0,FORWARD,err_msg)){
			if(ERROR == code )return(DBH_ERR);

			if(code == EFL)break;

#ifdef JRNL
			SettleLock( (char *)&pp_rec, PAY_PERIOD, err_msg );
#endif
			if(i == (23 - row - 5) || i == MAX_LINES)break;

			strcpy(t_code[max_val],pp_rec.pp_code);

			text = line+i*(TOT_STRNG+1);

			sprintf(text, "%1d. %-6.6s     %2d     %-30.30s",
				(max_val+1), pp_rec.pp_code,pp_rec.pp_numb,
				pp_rec.pp_desc);
			expnd_line(text) ;
			i++;
			max_val++;
		} 
#ifndef	ORACLE
		seq_over(PAY_PERIOD);
#endif
		if(put_blanks == 1 )
			if(i == 1){
#ifdef ENGLISH
				fomen("No Pay Period Codes to be Displayed ");
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

		if(k >= 1 && k < i-1 ) {
			strcpy(pp_code,t_code[k-1]) ;
			return(1);
		}
		if(k < 0) return(0);
		put_blanks = 2 ;
	}
}  /* payper_hlp ends */

/*-----------------------------------------------------------------------
	Usage :   cert_hlp   (cert_cd,cert_dt, row, col )
		  cert_cd	; Certification Code.
		  cert_dt	; Date.
		  row, col 	; self explanatory .
------------------------------------------------------------------------*/

cert_hlp(cert_cd,cert_dt, row, col) 
char	*cert_cd ;
long	*cert_dt ;
int	row,
	col;  	/* window parameters */
{
 	int 		i,k ; 
	Cert		cert ;
	char		t_code[MAX_LINES][7] ;
	long		t_date[MAX_LINES] ;
	int		on_name = 0; /* If Yes help on description	*/
	char		year[5];
	char		month[3];
	char		day[3];

	TOT_STRNG = 51 ;

	/* Make The Starting key */

	if(cert_cd[0] == '\0') {
		/* accept the starting key */
#ifdef ENGLISH
		get_name(row, col, 10, "Starting on Code: ", kee_name, 0);
#else
		get_name(row, col, 10, "Code de departement debutant: ", kee_name, 0);
#endif
		strncpy(cert.cr_code,kee_name,6);
	}
	else {
		strcpy(cert.cr_code,cert_cd);
	}

	cert.cr_date = *cert_dt;
        cert.cr_level = 0;

	/* Start the file */
	flg_reset(CERT);

#ifdef ENGLISH
	strcpy(line,"No  Code    Date ");
#else
	strcpy(line,"No  Code    Date ");
#endif
	expnd_line(line) ;
	height = 0;
	put_blanks = 1 ;

	code = get_n_cert(&cert, BROWSE, 0, FORWARD, err_msg) ;

	for(;;){
		i = 1;
		max_val = 0;
		for( ; ;code = get_n_cert(&cert,BROWSE,0,FORWARD,err_msg)){
			if(ERROR == code )return(DBH_ERR);

			if(code == EFL)break;

			if(cert_cd[0] != '\0') {
				if(strcmp(cert.cr_code,cert_cd) != 0) break;
			}
			if(max_val != 0) {
				if(strcmp(cert.cr_code,t_code[max_val-1])==0 &&
				   cert.cr_date == t_date[max_val-1]) continue;
			}
#ifdef JRNL
			SettleLock( (char *)&cert, CERT, err_msg );
#endif
			if(i == (23 - row - 5) || i == MAX_LINES)break;

			strcpy(t_code[max_val],cert.cr_code);
			t_date[max_val] = cert.cr_date;

			text = line+i*(TOT_STRNG+1);

			sprintf(year,"%ld",cert.cr_date / 10000);
			sprintf(month,"%02ld",(cert.cr_date / 100) % 100);
			sprintf(day,"%02ld",cert.cr_date % 100);

			sprintf(text, "%1d. %-6.6s %s/%s/%s",
				(max_val+1), cert.cr_code,year,month,day);
			expnd_line(text) ;
			i++;
			max_val++;
		} 
#ifndef	ORACLE
		seq_over(CERT);
#endif
		if(put_blanks == 1 )
			if(i == 1){
#ifdef ENGLISH
				fomen("No Certification Codes to be Displayed ");
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

		if(k >= 1 && k < i-1 ) {
			strcpy(cert_cd,t_code[k-1]) ;
			*cert_dt = t_date[k-1]; 
			return(1);
		}
		if(k < 0) return(0);
		put_blanks = 2 ;
	}
}  /* cert_hlp ends */
/*-----------------------------------------------------------------------
	Usage :   earn_hlp   (ea_date,ea_code, row, col )
		  ea_date	; Effective Date.
		  ea_code	; Classification Code.
		  row, col 	; self explanatory .
------------------------------------------------------------------------*/

earn_hlp(ea_date,ea_code, row, col) 
long	*ea_date ;
char	*ea_code ;
int	row,
	col;  	/* window parameters */
{
 	int 		i,k ; 
	Earn		earn ;
	long		t_date[MAX_LINES] ;
	char		t_code[MAX_LINES][7] ;
	int		on_name = 0; /* If Yes help on description	*/
	char		year[5];
	char		month[3];
	char		day[3];

	TOT_STRNG = 51 ;

	/* Make The Starting key */
	earn.ea_date = *ea_date;
	strcpy(earn.ea_code,ea_code);

	/* Start the file */
	flg_reset(EARN);

#ifdef ENGLISH
	strcpy(line,"No  Code    Date    Description           ");
#else
	strcpy(line,"No  Code    Date    Description           ");
#endif
	expnd_line(line) ;
	height = 0;
	put_blanks = 1 ;

	code = get_n_earn(&earn, BROWSE, 0, FORWARD, err_msg) ;

	for(;;){
		i = 1;
		max_val = 0;
		for( ; ;code = get_n_earn(&earn,BROWSE,0,FORWARD,err_msg)){
			if(ERROR == code )return(DBH_ERR);

			if(code == EFL)break;

#ifdef JRNL
			SettleLock( (char *)&earn, CLASSIFICATION, err_msg );
#endif
			if(i == (23 - row - 5) || i == MAX_LINES)break;

			t_date[max_val] = earn.ea_date;
			strcpy(t_code[max_val],earn.ea_code);

			text = line+i*(TOT_STRNG+1);

			sprintf(year,"%ld",earn.ea_date / 10000);
			sprintf(month,"%02ld",(earn.ea_date / 100) % 100);
			sprintf(day,"%02ld",earn.ea_date % 100);

			sprintf(text, "%1d. %-6.6s %s/%s/%s %-30.30s",
				(max_val+1), earn.ea_code,year,month,day,
				earn.ea_desc);
			expnd_line(text) ;
			i++;
			max_val++;
		} 
#ifndef	ORACLE
		seq_over(EARN);
#endif
		if(put_blanks == 1 )
			if(i == 1){
#ifdef ENGLISH
				fomen("No Earngins Dates to be Displayed ");
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

		if(k >= 1 && k < i-1 ) {
			*ea_date = t_date[k-1]; 
			strcpy(ea_code,t_code[k-1]) ;
			return(1);
		}
		if(k < 0) return(0);
		put_blanks = 2 ;
	}
}  /* earn_hlp ends */

/*-----------------------------------------------------------------------
	Usage :   barg_hlp   (barg_unit,barg_dt, row, col )
		  barg_unit	; Bargaining Unit.
		  barg_dt	; Date.
		  row, col 	; self explanatory .
------------------------------------------------------------------------*/

barg_hlp(barg_unit,barg_dt, row, col) 
char	*barg_unit ;
long	*barg_dt ;
int	row,
	col;  	/* window parameters */
{
 	int 		i,k ; 
	Barg_unit	barg ;
	char		t_code[MAX_LINES][7] ;
	long		t_date[MAX_LINES] ;
	int		on_name = 0; /* If Yes help on description	*/
	char		year[5];
	char		month[3];
	char		day[3];

	TOT_STRNG = 51 ;

	/* Make The Starting key */

	/* accept the starting key */
#ifdef ENGLISH
	get_name(row, col, 10, "Starting on Code: ", kee_name, 0);
#else
	get_name(row, col, 10, "Code de departement debutant: ", kee_name, 0);
#endif
	strncpy(barg.b_code,kee_name,6);

	barg.b_date = *barg_dt;

	/* Start the file */
	flg_reset(BARG);

#ifdef ENGLISH
	strcpy(line,"No  Code    Date      Description");
#else
	strcpy(line,"No  Code    Date      Description");
#endif
	expnd_line(line) ;
	height = 0;
	put_blanks = 1 ;

	code = get_n_barg(&barg, BROWSE, 0, FORWARD, err_msg) ;

	for(;;){
		i = 1;
		max_val = 0;
		for( ; ;code = get_n_barg(&barg,BROWSE,0,FORWARD,err_msg)){
			if(ERROR == code )return(DBH_ERR);

			if(code == EFL)break;

#ifdef JRNL
			SettleLock( (char *)&barg, BARG, err_msg );
#endif
			if(i == (23 - row - 5) || i == MAX_LINES)break;

			strcpy(t_code[max_val],barg.b_code);
			t_date[max_val] = barg.b_date;

			text = line+i*(TOT_STRNG+1);

			sprintf(year,"%ld",barg.b_date / 10000);
			sprintf(month,"%02ld",(barg.b_date / 100) % 100);
			sprintf(day,"%02ld",barg.b_date % 100);

			sprintf(text, "%1d. %-6.6s %s/%s/%s  %-30.30s",
				(max_val+1), barg.b_code,year,month,day,
				barg.b_name);
			expnd_line(text) ;
			i++;
			max_val++;
		} 
#ifndef	ORACLE
		seq_over(BARG);
#endif
		if(put_blanks == 1 )
			if(i == 1){
#ifdef ENGLISH
				fomen("No Bargaining Units to be Displayed ");
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

		if(k >= 1 && k < i-1 ) {
			strcpy(barg_unit,t_code[k-1]) ;
			*barg_dt = t_date[k-1]; 
			return(1);
		}
		if(k < 0) return(0);
		put_blanks = 2 ;
	}
}  /* barg_hlp ends */

/*-----------------------------------------------------------------------
	Usage :   benefit_hlp   (ben_code,pp_code, row, col )
		  ben_code	; Benefit Code.
		  pp_code	; Pay Period Code.
		  row, col 	; self explanatory .
------------------------------------------------------------------------*/

benefit_hlp(ben_code,pp_code, row, col) 
char	*ben_code ;
char	*pp_code ;
int	row,
	col;  	/* window parameters */
{
 	int 		i,k ; 
	Benefit		benefit ;
	char		t_code[MAX_LINES][7] ;
	char		t_pp_code[MAX_LINES][7] ;
	int		on_name = 0; /* If Yes help on description	*/

	TOT_STRNG = 51 ;

	/* Make The Starting key */

	/* accept the starting key */
#ifdef ENGLISH
	get_name(row, col, 10, "Starting on Code: ", kee_name, 0);
#else
	get_name(row, col, 10, "Code de departement debutant: ", kee_name, 0);
#endif
	strncpy(benefit.bn_code,kee_name,6);
	benefit.bn_pp_code[0] = '\0';

	/* Start the file */
	flg_reset(BENEFIT);

#ifdef ENGLISH
	strcpy(line,"No  Code    Pay Period ");
#else
	strcpy(line,"No  Code    Pay Period ");
#endif
	expnd_line(line) ;
	height = 0;
	put_blanks = 1 ;

	code = get_n_benefit(&benefit, BROWSE, 0, FORWARD, err_msg) ;

	for(;;){
		i = 1;
		max_val = 0;
		for( ; ;code = get_n_benefit(&benefit,BROWSE,0,FORWARD,err_msg)){
			if(ERROR == code )return(DBH_ERR);

			if(code == EFL)break;

#ifdef JRNL
			SettleLock( (char *)&benefit, BENEFIT, err_msg );
#endif
			if(pp_code[0] != '\0'){
				if(strcmp(pp_code,benefit.bn_pp_code) != 0)
					continue;
			}

			if(i == (23 - row - 5) || i == MAX_LINES)break;

			strcpy(t_code[max_val],benefit.bn_code);
			strcpy(t_pp_code[max_val],benefit.bn_pp_code);

			text = line+i*(TOT_STRNG+1);

			sprintf(text, "%1d. %-6.6s %-6.6s",
				(max_val+1), benefit.bn_code,benefit.bn_pp_code);
			expnd_line(text) ;
			i++;
			max_val++;
		} 
#ifndef	ORACLE
		seq_over(BENEFIT);
#endif
		if(put_blanks == 1 )
			if(i == 1){
#ifdef ENGLISH
				fomen("No Benefits Codes to be Displayed ");
#else
				fomen("TRANSLATION                  ");
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

		if(k >= 1 && k < i-1 ) {
			strcpy(ben_code,t_code[k-1]) ;
			strcpy(pp_code,t_pp_code[k-1]) ;
			return(1);
		}
		if(k < 0) return(0);
		put_blanks = 2 ;
	}
} /* Benefit Help Ends */
/*-----------------------------------------------------------------------
	Usage :   reg_pen_hlp   (reg_pen_code,pp_code, row, col )
		  ben_code	; Registration Pension Code.
		  pp_code	; Pay Period Code.
		  row, col 	; self explanatory .
------------------------------------------------------------------------*/

reg_pen_hlp(reg_pen_code,pp_code, row, col) 
char	*reg_pen_code ;
char	*pp_code ;
int	row,
	col;  	/* window parameters */
{
 	int 		i,k ; 
	Reg_pen		reg_pen ;
	char		t_code[MAX_LINES][7] ;
	char		t_pp_code[MAX_LINES][7] ;
	int		on_name = 0; /* If Yes help on description	*/

	TOT_STRNG = 51 ;

	/* Make The Starting key */

	/* accept the starting key */
#ifdef ENGLISH
	get_name(row, col, 10, "Starting on Code: ", kee_name, 0);
#else
	get_name(row, col, 10, "Code de departement debutant: ", kee_name, 0);
#endif
	strncpy(reg_pen.rg_code,kee_name,6);
	strncpy(reg_pen.rg_pp_code,pp_code,6);

	/* Start the file */
	flg_reset(REG_PEN);

#ifdef ENGLISH
	strcpy(line,"No  Code    Pay Period ");
#else
	strcpy(line,"No  Code    Pay Period ");
#endif
	expnd_line(line) ;
	height = 0;
	put_blanks = 1 ;

	code = get_n_reg_pen(&reg_pen, BROWSE, 0, FORWARD, err_msg) ;

	for(;;){
		i = 1;
		max_val = 0;
		for( ; ;code = get_n_reg_pen(&reg_pen,BROWSE,0,FORWARD,err_msg)){
			if(ERROR == code )return(DBH_ERR);

			if(code == EFL)break;

#ifdef JRNL
			SettleLock( (char *)&reg_pen, REG_PEN, err_msg );
#endif
			if(i == (23 - row - 5) || i == MAX_LINES)break;

			strcpy(t_code[max_val],reg_pen.rg_code);
			strcpy(t_pp_code[max_val],reg_pen.rg_pp_code);

			text = line+i*(TOT_STRNG+1);

			sprintf(text, "%1d. %-6.6s %-6.6s",
				(max_val+1), reg_pen.rg_code,reg_pen.rg_pp_code);
			expnd_line(text) ;
			i++;
			max_val++;
		} 
#ifndef	ORACLE
		seq_over(REG_PEN);
#endif
		if(put_blanks == 1 )
			if(i == 1){
#ifdef ENGLISH
				fomen("No Registration Pension Codes to be Displayed ");
#else
				fomen("TRANSLATION                  ");
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

		if(k >= 1 && k < i-1 ) {
			strcpy(reg_pen_code,t_code[k-1]) ;
			strcpy(pp_code,t_pp_code[k-1]) ;
			return(1);
		}
		if(k < 0) return(0);
		put_blanks = 2 ;
	}
} /* Registration Pension Help Ends */
/*-----------------------------------------------------------------------
	Usage :   deduction_hlp   (ded_code, row, col )
		  ded_code	; Deduction Code.
		  ded_desc	; Deduction Description.
		  row, col 	; self explanatory .
------------------------------------------------------------------------*/

deduction_hlp(ded_code, pp_code, row, col) 
char	*ded_code ;
char	*pp_code ;
int	row,
	col;  	/* window parameters */
{
 	int 		i,k ; 
	Deduction	deduction ;
	char		t_code[MAX_LINES][7] ;
	char		t_pp_code[MAX_LINES][7] ;
	int		on_name = 0; /* If Yes help on description	*/

	TOT_STRNG = 51 ;

	/* Make The Starting key */

	/* accept the starting key */
#ifdef ENGLISH
	get_name(row, col, 10, "Starting on Code: ", kee_name, 0);
#else
	get_name(row, col, 10, "Code de departement debutant: ", kee_name, 0);
#endif
	strncpy(deduction.dd_code,kee_name,6);
	deduction.dd_pp_code[0] = '\0';

	/* Start the file */
	flg_reset(DEDUCTION);

#ifdef ENGLISH
	strcpy(line,"No  Code     Pay Period");
#else
	strcpy(line,"No  Code     Pay Period");
#endif
	expnd_line(line) ;
	height = 0;
	put_blanks = 1 ;

	code = get_n_deduction(&deduction, BROWSE, 0, FORWARD, err_msg) ;

	for(;;){
		i = 1;
		max_val = 0;
		for( ; ;code = get_n_deduction(&deduction,BROWSE,0,FORWARD,err_msg)){
			if(ERROR == code )return(DBH_ERR);

			if(code == EFL)break;

#ifdef JRNL
			SettleLock( (char *)&deduction, DEDUCTION, err_msg );
#endif
			if(pp_code[0] != '\0') {
				if(strcmp(pp_code,deduction.dd_pp_code) != 0)
					continue;
			}
			if(i == (23 - row - 5) || i == MAX_LINES)break;

			strcpy(t_code[max_val],deduction.dd_code);
			strcpy(t_pp_code[max_val],deduction.dd_pp_code);

			text = line+i*(TOT_STRNG+1);

			sprintf(text, "%1d. %-6.6s      %-6.6s",
				(max_val+1), deduction.dd_code,deduction.dd_pp_code);
			expnd_line(text) ;
			i++;
			max_val++;
		} 
#ifndef	ORACLE
		seq_over(DEDUCTION);
#endif
		if(put_blanks == 1 )
			if(i == 1){
#ifdef ENGLISH
				fomen("No Deduction Codes to be Displayed ");
#else
				fomen("TRANSLATION                  ");
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

		if(k >= 1 && k < i-1 ) {
			strcpy(ded_code,t_code[k-1]) ;
			strcpy(pp_code,t_pp_code[k-1]) ;
			return(1);
		}
		if(k < 0) return(0);
		put_blanks = 2 ;
	}
} /* Deduction Help Ends */

/*-----------------------------------------------------------------------
	Usage :   ded_grp_hlp   (ded_code, row, col )
		  ded_code	; Deduction Code.
		  ded_group	; Deduction Code.
		  ded_desc	; Deduction Description.
		  row, col 	; self explanatory .
------------------------------------------------------------------------*/

ded_grp_hlp(ded_code, ded_group, row, col) 
char	*ded_code ;
char	*ded_group ;
int	row,
	col;  	/* window parameters */
{
 	int 		i,k ; 
	Ded_group	ded_grp ;
	char		t_code[MAX_LINES][7] ;
	char		t_group[MAX_LINES][7] ;
	int		on_name = 0; /* If Yes help on description	*/

	TOT_STRNG = 51 ;

	/* Make The Starting key */

	/* accept the starting key */
#ifdef ENGLISH
	get_name(row, col, 10, "Starting on Code: ", kee_name, 0);
#else
	get_name(row, col, 10, "Code de departement debutant: ", kee_name, 0);
#endif
	strncpy(ded_grp.dg_code,ded_code,6);
	strncpy(ded_grp.dg_group,kee_name,6);
	ded_grp.dg_pp_code[0] = '\0';

	/* Start the file */
	flg_reset(DED_GRP);

#ifdef ENGLISH
	strcpy(line,"No  Code     Group");
#else
	strcpy(line,"No  Code     Groupe");
#endif
	expnd_line(line) ;
	height = 0;
	put_blanks = 1 ;

	code = get_n_ded_grp(&ded_grp, BROWSE, 0, FORWARD, err_msg) ;

	for(;;){
		i = 1;
		max_val = 0;
		for( ; ;code = get_n_ded_grp(&ded_grp,BROWSE,0,FORWARD,err_msg)){
			if(ERROR == code )return(DBH_ERR);

			if(code == EFL)break;

#ifdef JRNL
			SettleLock( (char *)&ded_grp, DED_GRP, err_msg );
#endif
			if(ded_group[0] != '\0') {
				if(strcmp(ded_group,ded_grp.dg_group) != 0)
					continue;
			}
			if(i == (23 - row - 5) || i == MAX_LINES)break;

			strcpy(t_code[max_val],ded_grp.dg_code);
			strcpy(t_group[max_val],ded_grp.dg_group);

			text = line+i*(TOT_STRNG+1);

			sprintf(text, "%1d. %-6.6s      %-6.6s",
				(max_val+1), ded_grp.dg_code,ded_grp.dg_group);
			expnd_line(text) ;
			i++;
			max_val++;
		} 
#ifndef	ORACLE
		seq_over(DED_GRP);
#endif
		if(put_blanks == 1 )
			if(i == 1){
#ifdef ENGLISH
				fomen("No Group Deduction Codes to be Displayed ");
#else
				fomen("TRANSLATION                  ");
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

		if(k >= 1 && k < i-1 ) {
			strcpy(ded_code,t_code[k-1]) ;
			strcpy(ded_group,t_group[k-1]) ;
			return(1);
		}
		if(k < 0) return(0);
		put_blanks = 2 ;
	}
} /* Deduction Group Help Ends */
/*-----------------------------------------------------------------------
	Usage :   inact_hlp   (inact_cd, row, col )
		  inact_cd	; Fixed asset inactivation code
		  row, col 	; self explanatory .
------------------------------------------------------------------------*/

inact_hlp(inact_cd,row,col) 
char	*inact_cd ;
int	row,
	col;  	/* window parameters */
{
 	int 		i,k ; 
	char		t_code[MAX_LINES][4] ;
	Inact		inact;

	TOT_STRNG = 51 ;

	/* accept the starting key */
#ifdef ENGLISH
	get_name(row, col, 2, "Starting inactivation code:", kee_name, 0);
#else
	get_name(row, col, 2, "Starting inactivation code:", kee_name, 0);
#endif

	/* Make The Starting key */
	strncpy(inact.i_code,kee_name,2) ;

	/* Start the file */
	flg_reset(INACT_CODE);

#ifdef ENGLISH
	strcpy(line,"No  Inact Code  Description           ");
#else
	strcpy(line,"No  Inact Code  Description           ");
#endif
	expnd_line(line) ;
	height = 0;
	put_blanks = 1 ;

	code = get_n_inact(&inact, BROWSE, 0, FORWARD, err_msg) ;

	for(;;){
		i = 1;
		max_val = 0;
		for( ; ;code = get_n_inact(&inact,BROWSE,0,FORWARD,err_msg)){
			if(ERROR == code )return(DBH_ERR);

			if(code == EFL)break;

#ifdef JRNL
			SettleLock( (char *)&inact, INACT_CODE, err_msg );
#endif
			if(i == (23 - row - 5) || i == MAX_LINES)break;

			strcpy(t_code[max_val],inact.i_code );

			text = line+i*(TOT_STRNG+1);

			sprintf(text, "%1d.    %-3.3s     %-30.30s",
				(max_val+1), inact.i_code,inact.i_desc);
			expnd_line(text) ;
			i++;
			max_val++;
		} 
#ifndef	ORACLE
		seq_over(INACT_CODE);
#endif
		if(put_blanks == 1 )
			if(i == 1){
#ifdef ENGLISH
				fomen("No Codes to be Displayed ");
#else
				fomen("No Codes to be Displayed ");
#endif
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

		if( code != EFL)
#ifdef ENGLISH
			k = acpt_resp("RETURN for more, #(Item), Q(uit):",
							row+i+2, col+4,1) ;
#else
			k = acpt_resp("RETURN pour plus, #(Article), R(etourner):",
							row+i+2, col+4,1) ;
#endif
		else
#ifdef ENGLISH
			k = acpt_resp("Select #(Item) Or Q(uit)  :",
							row+i+2, col+4,0) ;
#else
			k = acpt_resp("Choisir #(Article) ou R(etourner)  :",
							row+i+2, col+4,0) ;
#endif

		if ( k >= 1 && k < i-1 ){
			strcpy(inact_cd, t_code[k - 1]) ;
			return(1) ;
		}
		if(k < 0)return(0);
		put_blanks = 2 ;
	}
} 

/*-----------------------------------------------------------------------
	Usage :   ben_cat_hlp   (ben_cat,ben_pp_code, row, col )
		  ben_cat	; Benefit Category Code.
		  row, col 	; self explanatory .
------------------------------------------------------------------------*/

ben_cat_hlp(ben_cat_code,pp_code, row, col) 
char	*ben_cat_code ;
char	*pp_code ;
int	row,
	col;  	/* window parameters */
{
 	int 		i,k ; 
	Ben_cat		ben_cat ;
	char		t_ben_cat[MAX_LINES][7] ;
	char		t_pp_code[MAX_LINES][7] ;
	int		on_name = 0; /* If Yes help on description	*/

	TOT_STRNG = 51 ;

	/* Make The Starting key */

	/* accept the starting key */
#ifdef ENGLISH
	get_name(row, col, 10, "Starting on Code: ", kee_name, 0);
#else
	get_name(row, col, 10, "Code de departement debutant: ", kee_name, 0);
#endif
	strncpy(ben_cat.bc_cat_code,kee_name,6);

	/* Start the file */
	flg_reset(BEN_CAT);

#ifdef ENGLISH
	strcpy(line,"No  Benefit Code   Pay Period ");
#else
	strcpy(line,"No     Code        Pay Period ");
#endif
	expnd_line(line) ;
	height = 0;
	put_blanks = 1 ;

	code = get_n_ben_cat(&ben_cat, BROWSE, 0, FORWARD, err_msg) ;

	for(;;){
		i = 1;
		max_val = 0;
		for( ; ;code = get_n_ben_cat(&ben_cat,BROWSE,0,FORWARD,err_msg)){
			if(ERROR == code )return(DBH_ERR);

			if(code == EFL)break;
#ifdef JRNL
			SettleLock( (char *)&ben_cat, BEN_CAT, err_msg );
#endif
			if(i == (23 - row - 5) || i == MAX_LINES)break;

			strcpy(t_ben_cat[max_val],ben_cat.bc_cat_code);
			strcpy(t_pp_code[max_val],ben_cat.bc_pp_code);

			text = line+i*(TOT_STRNG+1);

			sprintf(text, "%1d.    %-6.6s         %-6.6s",
				(max_val+1), ben_cat.bc_cat_code,ben_cat.bc_pp_code);
			expnd_line(text) ;
			i++;
			max_val++;
		} 
#ifndef	ORACLE
		seq_over(BEN_CAT);
#endif
		if(put_blanks == 1 )
			if(i == 1){
#ifdef ENGLISH
				fomen("No Benefit Category Codes to be Displayed ");
#else
				fomen("TRANSLATION                  ");
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

		if(k >= 1 && k < i-1 ) {
			strcpy(ben_cat_code,t_ben_cat[k-1]) ;
			strcpy(pp_code,t_pp_code[k-1]) ;
			return(1);
		}
		if(k < 0) return(0);
		put_blanks = 2 ;
	}
}	/* Benefit Category Help Ends */

/*-----------------------------------------------------------------------
	Usage :   ded_cat_hlp   (ded_cat, ded_pp_code, row, col )
		  ded_cat	; Deduction Category Code.
		  ded_pp_code	; Pay Period Code.
		  row, col 	; self explanatory .
------------------------------------------------------------------------*/

ded_cat_hlp(ded_cat_code, pp_code, row, col) 
char	*ded_cat_code ;
char	*pp_code ;
int	row,
	col;  	/* window parameters */
{
 	int 		i,k ; 
	Ded_cat		ded_cat ;
	char		t_ded_cat[MAX_LINES][7] ;
	char		t_pp_code[MAX_LINES][7] ;
	int		on_name = 0; /* If Yes help on description	*/

	TOT_STRNG = 51 ;

	/* Make The Starting key */

	/* accept the starting key */
#ifdef ENGLISH
	get_name(row, col, 10, "Starting on Code: ", kee_name, 0);
#else
	get_name(row, col, 10, "Code de departement debutant: ", kee_name, 0);
#endif
	strncpy(ded_cat.dc_cat_code,kee_name,6);

	/* Start the file */
	flg_reset(DED_CAT);

#ifdef ENGLISH
	strcpy(line,"No  Deduction Code  Pay Period ");
#else
	strcpy(line,"No       Code       Pay Period ");
#endif
	expnd_line(line) ;
	height = 0;
	put_blanks = 1 ;

	code = get_n_ded_cat(&ded_cat, BROWSE, 0, FORWARD, err_msg) ;

	for(;;){
		i = 1;
		max_val = 0;
		for( ; ;code = get_n_ded_cat(&ded_cat,BROWSE,0,FORWARD,err_msg)){
			if(ERROR == code )return(DBH_ERR);

			if(code == EFL)break;

#ifdef JRNL
			SettleLock( (char *)&ded_cat, DED_CAT, err_msg );
#endif
			if(i == (23 - row - 5) || i == MAX_LINES)break;

			strcpy(t_ded_cat[max_val],ded_cat.dc_cat_code);
			strcpy(t_pp_code[max_val],ded_cat.dc_pp_code);

			text = line+i*(TOT_STRNG+1);

			sprintf(text, "%1d.        %-6.6s      %-6.6s",
				(max_val+1), ded_cat.dc_cat_code,ded_cat.dc_pp_code);
			expnd_line(text) ;
			i++;
			max_val++;
		} 
#ifndef	ORACLE
		seq_over(DED_CAT);
#endif
		if(put_blanks == 1 )
			if(i == 1){
#ifdef ENGLISH
				fomen("No Deduction Category Codes to be Displayed ");
#else
				fomen("TRANSLATION                  ");
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

		if(k >= 1 && k < i-1 ) {
			strcpy(ded_cat_code,t_ded_cat[k-1]) ;
			strcpy(pp_code,t_pp_code[k-1]) ;
			return(1);
		}
		if(k < 0) return(0);
		put_blanks = 2 ;
	}
}	/* Deduction Category Help Ends */

/*-----------------------------------------------------------------------
	Usage :   fld_hlp   (fld_hdr, fld_info, fld_no, row, col )
		  fld_hdr	; Header information of Fields
		  fld_info	; Fields information 
		  fld_no	; Selected field# wiil be moved to this
		  row, col 	; self explanatory .
------------------------------------------------------------------------*/

fld_hlp(fld_hdr, fld_info, fld_no, row, col) 
Fld_hdr	fld_hdr ;
Field	*fld_info ;
int	*fld_no ;
int	row,
	col;  	/* window parameters */
{
 	int 	i, j, k ; 
	short	t_numb[MAX_LINES] ;

	TOT_STRNG = 34 ;

#ifdef ENGLISH
	sprintf(line,"File: %s",fld_hdr.filenm) ;
#else
	sprintf(line,"Dossier: %s",fld_hdr.filenm) ;
#endif
	expnd_line(line) ;
	text = line+TOT_STRNG+1 ;
#ifdef ENGLISH
	strcpy(text,"     Fld#    Name");
#else
	strcpy(text,"    #Champ    Nom");
#endif
	expnd_line(text) ;
	height = 0;
	put_blanks = 1 ;

	for(j = 0 ; j < fld_hdr.no_fields ; ){
		i = 2;
		max_val = 0;
		for( ; j < fld_hdr.no_fields ; j++ ){
			if(i == (23 - row - 5) || i == MAX_LINES)break;

			t_numb [max_val] = (j+1) ;

			text = line+i*(TOT_STRNG+1);

			sprintf(text, "%1d.    %3d    %-15.15s",
				(max_val+1), (j+1), (fld_info+j)->name) ;
			expnd_line(text) ;
			i++;
			max_val++;
		} 
		if(put_blanks == 1 )
			if(i == 2){
#ifdef ENGLISH
				fomen("No Fields to be Displayed ");
#else
				fomen("Pas de champs a afficher ");
#endif
				return(-1);
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

		if( j < fld_hdr.no_fields)
#ifdef ENGLISH
			k = acpt_resp("RETURN for more, #(Item), Q(uit):",
#else
			k = acpt_resp("RETURN pour plus, #(Article), R(etourner):",
#endif
							row+i+2, col+4,1) ;
		else
#ifdef ENGLISH
			k = acpt_resp("Select #(Item) Or Q(uit)  :",
							row+i+2, col+4,0) ;
#else
			k = acpt_resp("Choisir #(Article) ou R(etourner)  :",
							row+i+2, col+4,0) ;
#endif

		if ( k >= 1 && k < i-1 ){
			*fld_no = t_numb[k - 1] ;
			return(1) ;
		}
		if(k < 0)return(0);
		put_blanks = 2 ;
	}
}	/* fld_hlp ends */
/*---------------------------------------------------------------*/
/* pads line with blanks to expnd it to required characters. */

static
expnd_line(line) 
char	*line ;
{
	int 	j ;

	j=strlen(line);
	if(j > TOT_STRNG)j = TOT_STRNG;
	for ( ; j < TOT_STRNG; j++)
		*(line + j) = ' ' ;
	*(line+j) = '\n';
}

static
acpt_resp(message, row, col,more)
char 	*message ;
int 	row,col ;
int	more;
{
	int 	prompt_len ;
	char	d1 ;

	prompt_len = strlen(message) ;

	poscur( row, col) ;
	puts( message ) ;

	/** To Clear-off the junk which is appearing for some vague reason ***/
	puts(" ") ;

	while(1)
	{
		poscur( row, col+prompt_len) ;
#ifdef MS_DOS
		d1 = getch() ;
#else
		d1 = getchar() ;
#endif
		fomce();	/* Clears error message */
		if ( !valid_char(d1) ) continue ;
		if ( d1 == T_DEL ) continue ;
		putchar(d1) ;
		if ( more && (d1 == T_NL || d1 == T_CRF) ) return(0) ;
		if ( d1 == U_QUIT || d1 == L_QUIT ) return(-1) ;
		if (  d1 < '1' ||  d1 > (char)('0'+max_val) ) {
#ifdef ENGLISH
			fomen("Invalid Item Number .. Enter Again");
#else
			fomen("Numero d'article invalide.. Ressayer");
#endif
			continue ;
		}
		return(d1-'0') ;
	}
}
/*--------------------------------------------------------------------*/
static
valid_char(c) 
char	c ;
{
	if((c >= 'A' && c <= 'Z' ) || (c >= 'a' && c <= 'z') ||
	    (c >= '0' && c <= '9' ) || (c >= T_DEL && c <= T_CRF) || 
	    (c==' '||c==','||c==39||c=='!'||c=='#'||c=='*'||c=='%'||c=='$'||
	        c=='+'||
		c=='-'||c=='_'))
		return(1);
	else
		return(0) ;
}

#include <ctype.h>

#ifdef	O_PROFOM

#include <cfomtm.h>
#include <cfomtcr.h>

extern	struct tcrblk tb;

#else

extern	char	cviatb;

#endif
/*----------------------------------------------------------*/
static
get_name(row, col,len, prompt, ptr,type)
int 	row, col ;
int	len ;
char	*prompt ;
char	*ptr ;
int	type;	/* 1 - Numeric, 0 - String */
{
	int 	i ;
	char	c ;

#ifdef	O_PROFOM
	if(telm(ttyp) == 0){
        	c= telm(vabase) ; c |= telm(hiten) ; 
       		poscur(row,col+strlen(prompt)); puts(cas(efa)) ; 
      		poscur(row,col); 
       		puts(cas(sfa)) ; putchar(c) ;
		puts( prompt) ;
	} else {
        	poscur(row,col); 
        	puts(cas(hiten)) ; 
		puts( prompt) ;
       		poscur(row,col+strlen(prompt)); puts(cas(efa)) ; 
		putchar(' ');
	}
#else
   	poscur(row,col); 
	cviatb |= 0x8;
	puts(prompt);
#endif

	poscur(row, col+1+strlen(prompt) );
	puts("        ") ;   
	poscur(row, col+1+strlen(prompt) );

	for ( i = 0 ; i < len;) {
#ifdef MS_DOS
		c = getch();
#else
		c = getchar();
#endif
		if ( !valid_char(c) ) continue ;
		if ( c == T_NL  || c == T_CRF) 	/* Newline or Return */
			break ;
		if( c != T_DEL ) {		/* If not Backspace */
			if(type == 1 && !isdigit(c))continue;	/* numeric */
			/**
			*(ptr+i) = toupper(c) ;
			***/
			*(ptr+i) = c ;
			i++ ;
		} else
		    if ( i > 0 ) i-- ;
		/**
		putchar(toupper(c)) ;
		**/
		putchar(c) ;
	}
	if(type == 1 && i == 0){    /* if first char NULL sscanf won't work */
		*(ptr+i) = '0';
		i++;
	}
	*(ptr+i) = '\0' ;
}	
/*--------------------------------------------------------------*/
#ifdef	O_PROFOM
static
window(row,col,nch,nlines,text1, put_blanks ) 
int	row,
	col,
	nch,
	nlines ; 
int 	put_blanks ;		/* If The screen to be blanked before */
char	*text1 ; 
	        /*Displays window margin in reverse video */ 
{
	int n, m ;  
	char	*text;

	text = text1;
        if (row<1 || row>telm(lines) || col<1 || col>telm(cols)) {
#ifdef ENGLISH
		fomer("Window Row/Col Coordinates not Proper... ");
#else
		fomer("Coordonnees Rangee/Colonne ne sont pas convenables... ");
#endif
                return(-1);        /* row/col out of range */ 
	}
        if ((row+nlines)<1 || (row+nlines)>telm(lines)){
#ifdef ENGLISH
		fomer("Too Large a Window");
#else
		fomer("Fenetre trop grande");
#endif
		return(-1) ; 
	}
        if ((col+nch)<1 || (col+nch)>telm(cols)){
#ifdef ENGLISH
		fomer("Too Wide a Window");
#else
		fomer("Fenetre trop large");
#endif
		return(-1) ; 
	}
	if (put_blanks == 1) {
        	if (telm(ttyp)== 0) { 
			draw_0_ttyp(row,col,nch);
			draw_0_ttyp((row+1),col,2);
			draw_0_ttyp((row+1),(col+nch-2),2);
		} else {
			draw_1_ttyp(row,col,nch);
			draw_1_ttyp((row+1),col,2);
			draw_1_ttyp((row+1),(col+nch-2),2);
		}
        	poscur(row+1,col+3) ; 	/* Clearing First Line */
		for(n=0 ; n < nch-5 ; n++)putchar(' ');
	}
        for(n=2 ; n<nlines ; n++) { 
		if(put_blanks == 1){
			if(telm(ttyp) == 0){
				draw_0_ttyp(row+n,col,2);
				poscur(row+n,col+3);
				putchar(' ') ; 
        		} else
				draw_1_ttyp(row+n,col,2);
		}
                poscur(row+n,col+3);
		for(m = 0 ;*text && m < nch-6 ; m++, text++ ){
                	if(*text == '\n')break;
			putchar(*text);
                }
		for( ; *text ; text++)
			if(*text == '\n'){
				text++;
				break;
			}
	        for (; m < nch-6 ; m++)putchar(' ');

		if(put_blanks == 1){
        		if(telm(ttyp) == 0)poscur(row+n,col+nch-3) ; 
			putchar(' ') ; 
			if(telm(ttyp) == 0)
				draw_0_ttyp(row+n,(col+nch-2),2);
        		else
				draw_1_ttyp(row+n,(col+nch-2),2);
		}
	}
	if (put_blanks == 1)
        	if (telm(ttyp)== 0)
			draw_0_ttyp(row+nlines,col,nch);
		else
			draw_1_ttyp(row+nlines,col,nch);

	return(0);
} 

static
draw_0_ttyp(row,col,nch)
int	row,col,nch;
{
	int	n;
	char	c;

        c= telm(vabase) ; c |= telm(rv) ; 
       	poscur(row,col+nch); puts(cas(efa)) ; 
      	poscur(row,col); 
       	puts(cas(sfa)) ; putchar(c) ;
	for(n = 2; n <= nch ; n++)
		putchar(' ') ; 
}
static
draw_1_ttyp(row,col,nch)
int	row,col,nch;
{
	int n;

        poscur(row,col); 
        puts(cas(rv)) ; 
	for(n = 0 ; n < nch ; n++)
		putchar(' ');
       	poscur(row,col+nch); puts(cas(efa)) ; 
	putchar(' ');
}
#endif

/*-----------------------------------------------------------------------
	Usage :   rel_hlp   (rel_code, row, col )
		  rel_code	; selected school# will be moved to this.
		  row, col 	; self explanatory .
------------------------------------------------------------------------*/

rel_hlp(rel_code, row, col) 
char	*rel_code ;
int	row,
	col;  	/* window parameters */
{
 	int 		i,k ; 
	Religion		religion ;
	char		t_religion_code[MAX_LINES][7] ;
	int		on_name = 0; /* If Yes help on description	*/

	TOT_STRNG = 51 ;

	/* Make The Starting key */

	/* accept the starting key */
#ifdef ENGLISH
	get_name(row, col, 10, "Starting on Religion Code: ", kee_name, 0);
#else
	get_name(row, col, 10, "Code de religion debutant: ",kee_name, 0); 
#endif
	strcpy(religion.rel_code,kee_name,2);
	Right_Justify_Numeric(religion.rel_code,
					sizeof(religion.rel_code)-1);
	/* Start the file */
	flg_reset(RELIGION);

#ifdef ENGLISH
	strcpy(line,"No  Religion Code   Description                 ");
#else
	strcpy(line,"No  Code Religion  Desccription                ");
#endif
	expnd_line(line) ;
	height = 0;
	put_blanks = 1 ;

	code = get_n_rel(&religion, BROWSE, 0, FORWARD, err_msg) ;

	for(;;){
	  i = 1;
	  max_val = 0;
	  for( ; ;code = get_n_rel(&religion,BROWSE,0,FORWARD,err_msg)){
		if(ERROR == code ) return(DBH_ERR);
		if(code == EFL) break;
	  
#ifdef JRNL
		SettleLock( (char *)&religion, RELIGION, err_msg );
#endif
		if(i == (23 - row - 5) || i == MAX_LINES)break;

		strcpy(t_religion_code[max_val],religion.rel_code);

		text = line+i*(TOT_STRNG+1);

		sprintf(text, "%1d.  %-10.10s  %-35.35s",
				(max_val+1), religion.rel_code,
				religion.rel_desc);
		expnd_line(text) ;
		i++;
		max_val++;
	  } 
#ifndef	ORACLE
	  seq_over(RELIGION);
#endif
	  if(put_blanks == 1 )
		if(i == 1){
#ifdef ENGLISH
			fomen("No Religion Codes to be Displayed ");
#else
			fomen("Pas de religion a afficher ");
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
			strcpy(rel_code,t_religion_code[k-1]);
			return(1);
		}
		if(k < 0) return(0);
		put_blanks = 2 ;
	}
}  /* religion_hlp ends */ 
/*-----------------------------------------------------------------------
	Usage :   comp_hlp   (comp_code, row, col )
		  comp_code	; competition code will be moved to this.
		  row, col 	; self explanatory .
------------------------------------------------------------------------*/

comp_hlp(comp_code, row, col) 
char	*comp_code ;
int	row,
	col;  	/* window parameters */
{
 	int 		i,k ; 
	Comp		comp ;
	char		t_comp_code[MAX_LINES][8] ;
	int		on_name = 0; /* If Yes help on description	*/

	TOT_STRNG = 51 ;

	/* Make The Starting key */

	/* accept the starting key */
#ifdef ENGLISH
	get_name(row, col, 10, "Starting on Competition Code: ", kee_name, 0);
#else
	get_name(row, col, 10, "Code de competition debutant: ",kee_name, 0); 
#endif
	strcpy(comp.cm_code,kee_name,7);
	Right_Justify_Numeric(comp.cm_code,
					sizeof(comp.cm_code)-1);
	/* Start the file */
	flg_reset(COMP);

#ifdef ENGLISH
	strcpy(line,"No  Code   Description                 ");
#else
	strcpy(line,"No  Code   Desccription                ");
#endif
	expnd_line(line) ;
	height = 0;
	put_blanks = 1 ;

	code = get_n_comp(&comp, BROWSE, 0, FORWARD, err_msg) ;

	for(;;){
	  i = 1;
	  max_val = 0;
	  for( ; ;code = get_n_comp(&comp,BROWSE,0,FORWARD,err_msg)){
		if(ERROR == code ) return(DBH_ERR);
		if(code == EFL) break;
	  
#ifdef JRNL
		SettleLock( (char *)&comp, COMP, err_msg );
#endif
		if(i == (23 - row - 5) || i == MAX_LINES)break;

		strcpy(t_comp_code[max_val],comp.cm_code);

		text = line+i*(TOT_STRNG+1);

		sprintf(text, "%1d.  %-7.7s  %-40.40s",
				(max_val+1), comp.cm_code,
				comp.cm_desc);
		expnd_line(text) ;
		i++;
		max_val++;
	  } 
#ifndef	ORACLE
	  seq_over(COMP);
#endif
	  if(put_blanks == 1 )
		if(i == 1){
#ifdef ENGLISH
			fomen("No Competition Codes to be Displayed ");
#else
			fomen("Pas de code de competition a afficher ");
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
			strcpy(comp_code,t_comp_code[k-1]);
			return(1);
		}
		if(k < 0) return(0);
		put_blanks = 2 ;
	}
}  /* comp_hlp ends */ 
/*----------------E N D   O F   F I L E---------------------*/
