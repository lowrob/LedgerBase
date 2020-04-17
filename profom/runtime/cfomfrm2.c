/* cfomfrm2.c : CPROFOM form processor - front end routines - part 2 */

#include <stdio.h>
#include "cfomfrm.h"		/* form file header */
#include "cfomstrc.h"		/* status record header */
#include "cfomfts.h"		/* feature test switches */
#include "cfomdef.h"		/* some general definitions */

extern	struct 	stat_rec	*sp;
extern	struct  frmhdr	*fhp;	/* form file header area */
extern	struct	frmfld	*fap;	/* ptr to array of fields */
extern	struct	frmfld	*cf;	/* current field being accessed */
extern	struct	fldinfo	*fiap;	/* ptr to array of internal items */
extern	struct	fldinfo	*cfi;	/* current field's internal items */
extern	char	*fvp;		/* ptr to variable data area */
extern	int	formup;	/* is a form currently up? */
extern 	int 	tcrin;		/* defined in fomtcr.c : a tcr up? */
extern	char	*ourdr,		/* profom internal data record */
		*actudr,	/* actual user data record */
		*userdr;	/* user passed data record */
extern	int	ffld,		/* index to first field in form */
		ffldno,		/* field no of first field */
		lfld,		/* index to last field in form */
		lfldno,		/* field no of last field */
		firstfld,	/* first field of multi-field range */
		lastfld;	/* last field of the range */
extern	int	out_scrn_image,	/* output screen image with ctrl-p? */
 	updt_dup_buf;		/* update dup buffers on fomwr/fomwf? */

fomcf(func,status)	/* change function status */
int	func,status;
{
	chkstat
	switch (func) {
	case 1 :		/* change output-screen-image function */
		if (status == 0)
			out_scrn_image = FALSE;
		else if (status == 1)
			out_scrn_image = TRUE;
		else
			reterr(11)
		break;
	case 2 :
		if (status == 0)
			updt_dup_buf = FALSE;
		else if (status == 1)
			updt_dup_buf = TRUE;
		else
			reterr(11)
		break;
	default :
		reterr(11)
	}
	retnoerr
	} 


fomfp(fld,fldpos,fldlen) /* return pos of fld in data rec and length */
int	fld;
int	*fldpos,*fldlen;
{
	chkstat
	if (formreqd())
		reterr(10)	/* non-data rec call asking for new screen */
	if (getfld(fld) == -1)
		reterr(7)	/* field not found */
	if (cf->fldclas == CL_PROM){
		*fldpos = 0;
		*fldlen = 0;
		reterr(8)	/* prompt only field */
		}
	*fldpos = cfi->aoff;	/* C offset starts from 0 */
	switch (cfi->stortype){
	case STOR_CHAR	:	*fldlen = cf->dfsize+1;
				break;
	case STOR_SHORT :	*fldlen = sizeof(short);
				break;
	case STOR_INT	:	*fldlen = sizeof(int);
				break;
	case STOR_LONG	:	*fldlen = sizeof(long);
				break;
	case STOR_FLOAT	:	*fldlen = sizeof(float);
				break;
	case STOR_DOUBLE:	*fldlen = sizeof(double);
				break;
	}
	retnoerr
	}

fomxy(x,y)	/* positon cursor at (x,y) on screen */
int	x,y;
{
	if (poscur(x,y))
		return(1);
	else 
		retnoerr
	}

fommc(x,y,s)	/* pos curs at (x,y) and display s from there */
char	*s;
int	x,y;
{
	chkstat
	if (poscur(x,y))
		return(1);
	else {
		if (puts(s))
			return(1);
		retnoerr
		}
	}

fommf(mf)	/* return no of last field on screen */
int *mf;
{
	register int i;

	chkstat
	if (formreqd())
		reterr(10)
	for (i=lfld; i>=ffld; i--)	/* search down */
		if ((fap+i)->fldclas != CL_PROM) {
			cf = fap+i;
			if (chkdv() != 1) {
				*mf = cf->fldno;
				retnoerr
				}
			}
	*mf=0;
	reterr(13)
	}

fomca2(fld,func,arg1,arg2)	/* fomca call with 2 arguements */
int fld,func,arg1,arg2;
{
	int offset,show,t;

	chkstat
	if (formreqd())
		reterr(10)
	if (getfld(fld) == -1)
		reterr(7)	/* field not found */
	switch (func){
	case 1:			/* change prompt location */
		if (cf->fldclas == CL_FLD)
			reterr(14)	/* no prompt part */
		if (cf->fldclas == CL_PROM || chkdv() != 1){
			if (clrprom())	/* if up, remove the prompt */
				return(1);
			show = TRUE;
			}
		else
			show = FALSE;
		cf->promx = arg1;
		cf->promy = arg2;
		if (show)		/* show it again if necessary */
			if (putprom())
				return(1);
		break;
	case 2:			/* change field location */
		if (cf->fldclas == CL_PROM)
			reterr(8)	/* no field part */
		if ((t=chkdv()) != 1){	/* if up, remove the field */ 
			if (clrfld())
				return(1);
			show = TRUE;
			}
		else
			show = FALSE;
		cf->fldx = arg1;
		cf->fldy = arg2;
		if (show)		/* show it again if necessary */
			if (t==0)
				{ if (putdata()) return(1); }
			else
				{ if (putmask()) return(1); }
		break;
	case 3:			/* change whole field location */
		if (cf->fldclas != CL_PRMFLD)
			reterr(15)	/* not a prompt + field */
		if ((t = chkdv()) != 1){	/* if up, remove */
			if (clrfld())
				return(1);
			if (clrprom())
				return(1);
			show = TRUE;
			}
		else
			show = FALSE;
			/* calculate distance between prompt and field */
		offset = ((cf->fldy - 1) * DEF_WIDTH + cf->fldx) -
			 ((cf->promy - 1) * DEF_WIDTH + cf->promx);
		cf->promx = arg1;
		cf->promy = arg2;
			/* calculate new position of field */
		cf->fldy=((cf->promy-1)*DEF_WIDTH+cf->promx+offset)/DEF_WIDTH;
		cf->fldx=((cf->promy-1)*DEF_WIDTH+cf->promx+offset)%DEF_WIDTH;
		if (show){	/* show it again if necessary */
			if (putprom())
				return(1);
			if (t == 0)
				{ if (putdata()) return(1); }
			else
				{ if (putmask()) return(1); }
			}
		break;
	default :
		reterr(11)	/* unknown arg to fomca2  */
	}
	retnoerr
	}

fomca1(fld,func,arg)	/* fomca call with 1 arguement */
int fld,func,arg;
{
	int check,show,t;

	chkstat
	if (formreqd())
		reterr(10)
	if (getfld(fld) == -1)
		reterr(7)	/* field not found */
	check=FALSE;
	switch (func){	/* validate & note if the field to be redisplayed */
	case 4 :		/* change data type */
		reterr(16)	/* undesirable - hence unsupported */
	case 5 :		/* logical attribute change */
	case 6 :
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		if (cf->fldclas == CL_PROM)
			reterr(8)	/* prompt only field */
		break;
	case 7 :		/* change video attribute */
	case 8 :
	case 9 :
	case 15:
	case 16:
		switch (arg){
		case 0:
		case 1:
			if (cf->fldclas != CL_PRMFLD)
				reterr(15)	/* not prompt+field */
			break;
		case 3:
		case 5:
			if (cf->fldclas == CL_PROM)
				reterr(8)	/* prompt only */
			break;
		case 2:
		case 4:
			if (cf->fldclas == CL_FLD)
				reterr(14)	/* no prompt part */
			break;
		default:
			reterr(2)	/* invalid arguement */
		}
		check = TRUE;
		break;
	case 17:
		if (cf->fldclas == CL_FLD)
			reterr(14)	/* no prompt part */
		check = TRUE;
		break;
	case 18:
		if (cf->fldclas == CL_PROM)
			reterr(14)	/* prompt only */
		check = TRUE;
		break;
	case 19:	/* change dup control */
		if (cf->fldclas == CL_PROM)
			reterr(14)
		if (arg < 0 || arg > 2)
			reterr(2)
		break;
	default:
		reterr(11)	/* invalid arguement */
	}
	if (check)
		if (cf->fldclas == CL_PROM){
			if (clrprom())
				return(1);
			show = TRUE;
			}
		else {
			if ((t = chkdv()) != 1){
				if (clrfld())
					return(1);
				if (cf->fldclas == CL_PRMFLD) {
					if (clrprom())
						return(1);
				}
				show = TRUE;
			}
			else
				show = FALSE;
		}
	else
		show = FALSE;
	switch (func){	/* process */
	case 5:			/* change required flag */
		cla1(arg,LA_REQ);
		break;
	case 6:			/* change validity check flag */
		cla1(arg,LA_VALID);
		break;
	case 7:			/* change bold flag */
		chva(arg,VA_BOLD);
		break;
	case 8:			/* change blinking flag */
		chva(arg,VA_BLINK);
		break;
	case 9:			/* change reverse video flag */
		chva(arg,VA_REVERSE);
		break;
	case 10:		/* change user escape flag */
		cla1(arg,LA_UESC);
		break;
	case 11:		/* change help return flag */
		cla1(arg,LA_HRET);
		break;
	case 12:		/* change show dup data flag */
		cla2(arg,LA_SHODUP);
		break;
	case 13:		/* change no echo flag */
		cla1(arg,LA_NOECHO);
		break;
	case 14:		/* change supress zeros flag */
		cla1(arg,LA_SUP);
		break;
	case 15:		/* change dim flag */
		chva(arg,VA_DIM);
		break;
	case 16:		/* change underline flag */
		chva(arg,VA_ULINE);
		break;
	case 17:		/* change prompt colour */
		cf->promclr = arg;
		break;
	case 18:		/* change field colour */
		cf->fldclr = arg;
		break;
	case 19:		/* change dup control */
		switch (arg){
		case 0:
			cf->dupctrl = DUP_NONE;
			break;
		case 1:
			cf->dupctrl = DUP_MASTER;
			break;
		case 2:
			cf->dupctrl = DUP_COPY;
			break;
		}
		break;
	}
	if (show)	/* redisplay the field if necessary */
		if (cf->fldclas == CL_PROM)
			{ if (putprom()) return(1); }
		else {
			if (t == 0)
				{ if (putdata()) return(1); }
			else
				{ if (putmask()) return(1); }
			if (cf->fldclas == CL_PRMFLD)
				{ if (putprom()) return(1); }
			}
	retnoerr
	}
fomqy(func,fld,s)	/* return attribute of fld in s */
char *s;
int func,fld;
{
	chkstat
	if (formreqd())
		reterr(10)
	if (getfld(fld) == -1)
		reterr(7)
	/* in following switch all cvelm macros are inserted by amar on
	   23 DEC 86 */
	switch (func) {
	case 1:			/* help message */
		if (cf->fldclas == CL_PROM)
			reterr(8)
		if (!cf->helpmes)
			reterr(17)	/* no help message */
		strcpy(s,cvelm(helpmes)); /* changed cf->X to cvelm(X) */
		break;
	case 2:			/* display mask string */
		if (cf->fldclas == CL_PROM)
			reterr(8)
		strcpy(s,cvelm(dmask)); /* changed cf->X to cvelm(X) */
		break;
	case 3:			/* prompt string */
		if (cf->fldclas == CL_FLD)
			reterr(14)
		strcpy(s,cvelm(prompt)); /* changed cf->X to cvelm(X) */
		break;
	case 4:			/* lower bound */
		if (cf->fldclas == CL_PROM)
			reterr(8)
		if (!cf->lbound)
			reterr(18)	/* no lower bound */
		convtoa(cvelm(lbound),s); /* changed cf->X to cvelm(X) */
		break;
	case 5:			/* upper bound */
		if (cf->fldclas == CL_PROM)
			reterr(8)
		if (!cf->ubound)
			reterr(19)	/* no upper bound */
		convtoa(cvelm(ubound),s); /* changed cf->X to cvelm(X) */
		break;
	case 6:			/* duplication string */
		if (cf->fldclas == CL_PROM)
			reterr(8)
		if (!cf->dupval)
			reterr(9)	/* no dup string */
		convtoa(cvelm(dupval),s); /* changed cf->X to cvelm(X) */
		break;
	case 7:			/* field name */
		if (cf->fldclas == CL_PROM)
			reterr(8)
		if (!cf->fldnam)
			reterr(20)	/* no field name */
		strcpy(s,cvelm(fldnam)); /* changed cf->X to cvelm(X) */
		break;
	case 8:			/* field type */
		if (cf->fldclas == CL_PROM)
			reterr(8)
		switch (cf->fldtyp){
		case TYP_YN :
			strcpy(s,"BOOLEAN");
			break;
		case TYP_DATE :
			strcpy(s,"DATE");
			break;
		case TYP_NUM :
			strcpy(s,"NUMBER");
			break;
		case TYP_STRING :
			strcpy(s,"STRING");
			break;
		default :
			fomintlerr(3)	/* invalid field type ! */
		}
		break;
	default :
		reterr(2)	/* invalid arguement */
	}
	retnoerr
	}

fomss(s,lines,columns)	/* return screen image in s of size lines*columns */
char *s;
int lines,columns;
{
	chkstat
	if (scrimg(s,lines,columns))
		return(1);
retnoerr }
