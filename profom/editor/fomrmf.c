#include "fomlink.h"
#include "cfomstrc.h"
#include <stdio.h>
#include "PROFOMF.sth"
struct linkstr *remfld ()	/* remove field from the linked structure */
{				/* return a pointer to the next */

extern struct linkstr *curp, *first;
extern struct stat_rec statrec;
extern struct fe_struct dr;
if(curp == first)  first = curp->nexp;				
curp->nexp->prep = curp->prep;
curp->prep->nexp = curp->nexp;
				/* show next field */
curp = curp->nexp;
filldr(1);
statrec.nextfld = 1;
statrec.endfld  = 0;

fomwr((char *) &dr);		/* Show next field on the screen */
if(statrec.retcode == RET_ERROR) fomer("error in screen write");
return (curp);

}
