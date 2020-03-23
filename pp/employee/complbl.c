/******************************************************************************
		Sourcename   : complbl.c
		System       : 
		Module       :
		Created on   : 93-JUNE-10 
******************************************************************************
About the file:	
	This program will print a list of employee mailing address labels
	by competition code.

History:
Programmer      Last change on    Details

******************************************************************************/

#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_pp.h>
#include <bfs_com.h>
#include <repdef.h>
#include <cfomstrc.h>
#include <filein.h>
#include <isnames.h>

#define CONTINUE	10
#define EXIT		12

#ifdef ENGLISH
#define PRINTER 	'P'
#define DISPLAY		'D'
#define FILE_IO		'F'
#else
#define PRINTER		'I'

#define DISPLAY		'A'
#define FILE_IO		'D'
#endif

static Comp		comp;
static Emp_comp		emp_comp;
static Emp		emp_rec;
static Pa_rec		pa_rec;

/*  Data items for storing the key range end values */
static char	comp1[8];	
static char	comp2[8];	

static int	flag;		/* flag to print the employee or not */
static int	PG_SIZE;
static int	retval;
static char 	discfile[15];	/* for storing outputfile name */
static short	pgcnt; 		/* for page count */
static char	resp[2];	/* for storing response */
static short	copies;

extern char 	e_mesg[200];	/* for storing error messages */

static	int	label_nbr;

	char	last_name_1[26]; 
	char	last_name_2 [26]; 
	char	last_name_3[26];
	char	first_name_1[16]; 
	char	first_name_2[16]; 
	char	first_name_3[16];
	char	mid_name_1[16]; 
	char	mid_name_2[16]; 
	char	mid_name_3[16];
	char	add1_1[31]; 
	char	add1_2[31]; 
	char	add1_3[31]; 
	char	add2_1[31];
	char	add2_2[31]; 
	char	add2_3[31]; 
	char	add3_1[31]; 
	char	add3_2[31];
	char	add3_3[31]; 
	char	pc_1[11]; 
	char	pc_2[11]; 
	char	pc_3[11]; 

complbl()
{
	char	firstresp[2];

	/* Get details for output medium */
#ifdef ENGLISH
	STRCPY(resp,"P");
#else
	STRCPY(resp,"I");
#endif
	if((retval =  GetOutputon(resp))<0 )
		return(retval);

	switch(*resp) {
		case DISPLAY:
			resp[0]='D';
			strcpy( discfile, terminal );
			PG_SIZE = 23;
			break;
		case FILE_IO:
			resp[0]='F';
			strcpy( discfile, "complbl.dat" );
			if((retval = GetFilename(discfile))<0 )
				return(retval);
			PG_SIZE = 55;
			break;
		case PRINTER:
		default:
			resp[0]='P';
			discfile[0]= '\0';
			PG_SIZE = 55;
			break;
	}

	copies = 1;
	if( *resp=='P' ){
		if((retval = GetNbrCopies(&copies))<0 )
			return(retval);
	}

	strcpy( comp1, "      0");
	strcpy( comp2, "ZZZZZZZ" );
	retval = GetCompRange( comp1,comp2 );
	if(retval < 0) return(retval);

	pgcnt = 0;		/* Page count is zero */
	LNSZ = 132;		/* line size in no. of chars */
	linecnt = PG_SIZE;	/* Page size in no. of lines */

	if( *resp=='P'){
		if((retval = GetResponse(firstresp))<0) {
			return(retval);
		}

		if( firstresp[0] == 'N' ) 
			return(0) ;

		if( opn_prnt( resp, discfile, 1, e_mesg, 0 /*do not spool*/)<0 )
			return(REPORT_ERR);
		retval = DoTestPrint();
		if(retval < 0)	return(retval);
	}

	if( (retval=Confirm())<=0 )
		return(retval);

	if( *resp=='P' )
		SetCopies( (int)copies );


	retval = PrintLbl();
	close_rep();	/* close output file */
	close_dbh();
	return(retval);
}

/*-----------------------------------------------------------------------*/
/*  Printing the Test as many times needed for labels to line up        */
static
DoTestPrint()
{
	int 	i,	retval ;
	char	temp_buf[132];
	char	testresp[2];

	STRCPY(temp_buf, 
	  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX") ;

	for( ; ; ) {

		if((retval=DisplayMessage("Do You Want a Test Print(Y/N)?"))<0){
			return(retval);
		}

		if((retval = GetResponse(testresp))<0) {
			return(retval);
		}

		if( testresp[0] == 'N' ) 
			break ;

		mkln(1,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXX",24);
		mkln(28,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXX",24);
		mkln(55,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXX",24);
		if(prnt_line() < 0 )	return(REPORT_ERR);

		mkln(1,"XXXXXXXXXXXXXXXXXXXXXXXX",24);
		mkln(28,"XXXXXXXXXXXXXXXXXXXXXXXX",24);
		mkln(55,"XXXXXXXXXXXXXXXXXXXXXXXX",24);
		if(prnt_line() < 0 )	return(REPORT_ERR);

		mkln(1,"XXXXXXXXXXXXXXXXXXXXXXXX",24);
		mkln(28,"XXXXXXXXXXXXXXXXXXXXXXXX",24);
		mkln(55,"XXXXXXXXXXXXXXXXXXXXXXXX",24);
		if(prnt_line() < 0 )	return(REPORT_ERR);

		mkln(1,"XXXXXXXXXX",10);
		mkln(28,"XXXXXXXXXX",10);
		mkln(55,"XXXXXXXXXX",10);
		if(prnt_line() < 0 )	return(REPORT_ERR);
		if(prnt_line() < 0 )	return(REPORT_ERR);
		if(prnt_line() < 0 )	return(REPORT_ERR);
		if(prnt_line() < 0 )	return(REPORT_ERR);
		if(prnt_line() < 0 )	return(REPORT_ERR);
		if(prnt_line() < 0 )	return(REPORT_ERR);

		mkln(1,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXX",24);
		mkln(28,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXX",24);
		mkln(55,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXX",24);
		if(prnt_line() < 0 )	return(REPORT_ERR);

		mkln(1,"XXXXXXXXXXXXXXXXXXXXXXXX",24);
		mkln(28,"XXXXXXXXXXXXXXXXXXXXXXXX",24);
		mkln(55,"XXXXXXXXXXXXXXXXXXXXXXXX",24);
		if(prnt_line() < 0 )	return(REPORT_ERR);

		mkln(1,"XXXXXXXXXXXXXXXXXXXXXXXX",24);
		mkln(28,"XXXXXXXXXXXXXXXXXXXXXXXX",24);
		mkln(55,"XXXXXXXXXXXXXXXXXXXXXXXX",24);
		if(prnt_line() < 0 )	return(REPORT_ERR);

		mkln(1,"XXXXXXXXXX",10);
		mkln(28,"XXXXXXXXXX",10);
		mkln(55,"XXXXXXXXXX",10);
		if(prnt_line() < 0 )	return(REPORT_ERR);
		if(prnt_line() < 0 )	return(REPORT_ERR);
		if(prnt_line() < 0 )	return(REPORT_ERR);
		if(prnt_line() < 0 )	return(REPORT_ERR);
		if(prnt_line() < 0 )	return(REPORT_ERR);
		if(prnt_line() < 0 )	return(REPORT_ERR);

#ifdef ENGLISH
		fomen("The Test Print Is Printing. Press any key to continue.");
#else
		fomen("La copie d'essaie imprime. Appuyer une touche pour continuer.");
#endif
		get();
	}
	return(NOERROR) ;
}
/******************************************************************************
Main logic of the program
******************************************************************************/
static
PrintLbl()
{
	label_nbr = 0;

	/* get the cost center name */
	retval = get_param(&pa_rec,BROWSE,1,e_mesg);
	if (retval < 0){
		fomer(e_mesg);
		get ();
	}
	strcpy(comp.cm_code,comp1);
	flg_reset(COMP);

	for(;;) {
		retval=get_n_comp(&comp,BROWSE,0,FORWARD,e_mesg);
		if(retval < 0) {
			if(retval == EFL)	break;
			fomer(e_mesg);
			get ();
			return(ERROR);
		}
			
		if(strcmp(comp.cm_code,comp1) < 0 ||
	   	   strcmp(comp.cm_code,comp2) > 0 ) 
			break;

		emp_rec.em_last_name[0] = '\0';
		emp_rec.em_first_name[0] = '\0';
		flg_reset(EMPLOYEE);

		for(;;) {
			retval=get_n_employee(&emp_rec,BROWSE,6,FORWARD,e_mesg);
			if(retval < 0) {
				if(retval == EFL)	break;
				fomer(e_mesg);
				get ();
				return(ERROR);
			}

			retval = UsrBargVal(BROWSE,emp_rec.em_numb,
				emp_rec.em_barg,0,e_mesg);
			if(retval < 0)	continue;

			strcpy(emp_comp.ec_numb,emp_rec.em_numb);
			strcpy(emp_comp.ec_code,comp.cm_code);

			retval=get_emp_comp(&emp_comp,BROWSE,0,e_mesg);
			if(retval < 0) {
				if(retval == UNDEF)	continue;
				fomer(e_mesg);
				get ();
				return(ERROR);
			}
	
			label_nbr++;

			retval = Load();
			if(retval < 0) 	return(retval);

	
			if(label_nbr == 3) {
				if ((retval = PrntAddress())<0)
					return(retval);
				if(retval == EXIT)	break;
			}
	
		} /*end of endless for loop*/
		seq_over(EMPLOYEE);	

		retval = LoadRest();
		if(retval < 0)	return(ERROR);
	}

	if(pgcnt){
		if(term<99)
			last_page();
	}
	return(NOERROR);
}
/*************************************************************************
Load variables for there label accross to be ready to be printed.
**************************************************************************/
static
Load()
{
	switch	(label_nbr) {
	  case 1:	
		strcpy(first_name_1, emp_rec.em_first_name);
		strcpy(mid_name_1, emp_rec.em_mid_name);
		strcpy(last_name_1, emp_rec.em_last_name);
		if(emp_rec.em_add1[0] == '\0')
			strcpy(add1_1,emp_rec.em_add2);
		else
			strcpy(add1_1,emp_rec.em_add1);
		strcpy(add2_1,emp_rec.em_add3);
		strcpy(add3_1,emp_rec.em_add4);
		strcpy(pc_1,emp_rec.em_pc);
		break;
	  case 2:	
		strcpy(first_name_2, emp_rec.em_first_name);
		strcpy(mid_name_2, emp_rec.em_mid_name);
		strcpy(last_name_2, emp_rec.em_last_name);
		if(emp_rec.em_add1[0] == '\0')
			strcpy(add1_2,emp_rec.em_add2);
		else
			strcpy(add1_2,emp_rec.em_add1);
		strcpy(add2_2,emp_rec.em_add3);
		strcpy(add3_2,emp_rec.em_add4);
		strcpy(pc_2,emp_rec.em_pc);
		break;
	  case 3:	
		strcpy(first_name_3, emp_rec.em_first_name);
		strcpy(mid_name_3, emp_rec.em_mid_name);
		strcpy(last_name_3, emp_rec.em_last_name);
		if(emp_rec.em_add1[0] == '\0')
			strcpy(add1_3,emp_rec.em_add2);
		else
			strcpy(add1_3,emp_rec.em_add1);
		strcpy(add2_3,emp_rec.em_add3);
		strcpy(add3_3,emp_rec.em_add4);
		strcpy(pc_3,emp_rec.em_pc);
		break;
	}

	return(NOERROR);
}
/*************************************************************************
Load variables for the rest of the labels if there is not three to print 
accross for each different competition code to be ready to be printed.
**************************************************************************/
static
LoadRest()
{
	if(label_nbr != 0){
		if(label_nbr == 3){
			retval = PrntAddress();
			if(retval < 0) 	return(retval);
		}
		else {
			strcpy(last_name_3, "               ");
			strcpy(add1_3, "                              ");
			strcpy(add2_3, "                              ");
			strcpy(add3_3, "                              ");
			strcpy(pc_3,"          ");
			strcpy(first_name_3, "                         ");
			strcpy(mid_name_3, "               ");
			if(label_nbr == 2) {
				retval = PrntAddress();
				if(retval < 0) 	return(retval);
			}
			else {
			  strcpy(first_name_2, "                         ");
			  strcpy(mid_name_2, "               ");
			  strcpy(last_name_2, "               ");
			  strcpy(add1_2, "                              ");
			  strcpy(add2_2, "                              ");
			  strcpy(add3_2, "                              ");
			  strcpy(pc_2,"          ");
			  retval = PrntAddress();
			  if(retval < 0) 	return(retval);
			}
		}
	}
	if( rite_top()<0 ) return( ERROR );	/* form_feed */

	return(NOERROR);
}
/*----------------------------------------------------------------------*/
static
PrntAddress()
{
	char	txt_line[132];

	strcpy(txt_line,first_name_1);
	strcat(txt_line," ");
	strcat(txt_line,mid_name_1);
	strcat(txt_line," ");
	strcat(txt_line,last_name_1);
	mkln(1,txt_line,24);

	strcpy(txt_line,first_name_2);
	strcat(txt_line," ");
	strcat(txt_line,mid_name_2);
	strcat(txt_line," ");
	strcat(txt_line,last_name_2);
	mkln(28,txt_line,24);

	strcpy(txt_line,first_name_3);
	strcat(txt_line," ");
	strcat(txt_line,mid_name_3);
	strcat(txt_line," ");
	strcat(txt_line,last_name_3);
	mkln(55,txt_line,24);

	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(1,add1_1,24);
	mkln(28,add1_2,24);
	mkln(55,add1_3,24);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	strcpy(txt_line,add2_1);
	strcat(txt_line,", ");
	strcat(txt_line,add3_1);
	mkln(1,txt_line,24);

	strcpy(txt_line,add2_2);
	strcat(txt_line,", ");
	strcat(txt_line,add3_2);
	mkln(28,txt_line,24);

	strcpy(txt_line,add2_3);
	strcat(txt_line,", ");
	strcat(txt_line,add3_3);
	mkln(55,txt_line,24);

	if(prnt_line() < 0 )	return(REPORT_ERR);

	mkln(1,pc_1,10);
	mkln(28,pc_2,10);
	mkln(55,pc_3,10);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);
	if(prnt_line() < 0 )	return(REPORT_ERR);

	label_nbr = 0;

	if (linecnt > PG_SIZE) {
		if(resp[0] == 'D') {
			if(next_page()<0) return(EXIT);	
		}
		else {
			if( rite_top()<0 ) return( -1 );	/* form_feed */
		}
	}

	return(NOERROR);
}
/******************   END OF PROGRAM *******************************/
