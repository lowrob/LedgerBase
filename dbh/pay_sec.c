/******************************************************************************
		Sourcename    : pay_sec.c
		System        : Payroll System.
		Created on    : June 9, 1992
		Created  By   : Eugene Roy
******************************************************************************
/*  			PAYROLL SECURITY				   */
/*									   */
/*   The payroll security verifies if the user can access the information  */
/*  for a bargaining unit.						   */
/***************************************************************************/

#include <ctype.h>
#include <cfomstrc.h>
#include <stdio.h>
#include <bfs_defs.h>
#include <bfs_com.h>
#include <bfs_pp.h>

/****************************************************************************/
/*									    */
UsrBargVal(mode, emp_numb, barg, type, e_mesg)
int	mode;
char	*emp_numb;
char	*barg;
int	type;
char	*e_mesg ;
{
	int	retval;
	UP_rec	userprof;
	Userbarg	userbarg;

	if(mode != ADD) {

		strcpy(userprof.u_id, User_Id);

		retval = get_userprof(&userprof,BROWSE,0,e_mesg) ;
		if(retval < 0) {
			return(retval);
		}
		if(strcmp(userprof.u_class, "A") == 0 || 
		   strcmp(userprof.u_class, "S") == 0)
			return(NOERROR);

		strcpy(userbarg.ub_id, User_Id);
		strcpy(userbarg.ub_barg, barg);

		retval = get_userbarg(&userbarg,BROWSE,0,e_mesg) ;

		if(retval == UNDEF)
			return(NOERROR);

		if(retval < 0) {
			return(retval);
		}
		if(mode == UPDATE)
		  if(strcmp(userbarg.ub_update, "Y") == 0)
			return(NOERROR);
		
		if(mode == P_DEL)
		  if(strcmp(userbarg.ub_delete, "Y") == 0)
			return(NOERROR);

		if(mode == BROWSE)
		  if(strcmp(userbarg.ub_browse,"Y") == 0)
			return(NOERROR);

		if(type == 1){
#ifdef ENGLISH
			sprintf(e_mesg,
			"User : %s ..... Does Not Have Authorization", User_Id);
#else
			sprintf(e_mesg,
			"Usager: %s ..... Pas autorise", User_Id) ; 
#endif
		}
		return(ERROR);
	}

	return(NOERROR);
}
