
/* cfomfrm3.c : CPROFOM form processor - back end routines */

/***********
	Modification History :   
		30_oct : Removed comments around free stmts after fomcs()
		as switching screens in the same program was causing excessive
		memory buildup and was leading to funny behavour.

		9-Nov : 1. Removed alloc stmts from loadform. alloc routine
		rewritten to allocate 20k adhoc. Payment was bombing out at 
		Malloc .
*************/

#include <stdio.h>
#include "cfomfrm.h"		/* form file header */
#include "cfomstrc.h"		/* status record header */
#include "cfomfts.h"		/* feature test switches */
#include "cfomdef.h"		/* some general definitions */
#include "fcntl.h"

#define FMXTUC 	".NFM"	/* upper case extension for form file name */
#define FMXTLC	".nfm"	/* lower case extension for form file name */
#define VD_SZ	(fhp->vdsize)	 /* size of var data blk to be allocd */

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
		*userdr;	/* converted user data record */
extern	int	ffld,		/* index to first field in form */
		ffldno,		/* field no of first field */
		lfld,		/* index to last field in form */
		lfldno,		/* field no of last field */
		firstfld,	/* first field of multi-field range */
		lastfld;	/* last field of the range */
extern	int	out_scrn_image,	/* output screen image with ctrl-p? */
 	updt_dup_buf;		/* update dup buffers on fomwr/fomwf? */
extern	char	cfrmnam[SR_SCRNML];	/* if yes, here is its name */
extern	char *fomrdinfo;	/* for use by fomrd() */

struct	storform {
struct	frmhdr	*sffhp;
struct	frmfld	*sffap;
struct	fldinfo *sffiap;
int	sfffld,sflfld,sfffldno,sflfldno,*sfrdar;
char	*sffvp,*sfourdr,*sfuserdr,sffmnm[SR_SCRNML];
struct	storform *nextsf;
	};

#define STFMSZ	(sizeof(struct storform))

struct	storform  *sfhdp = NULL;

static	int	fd;		/* for reading form file */

char *malloc();

#define	BIG_SIZE	21000 
static int 	BUFF_ptr  = 0 ;

/*** Rewriiten On 9-Nov ...kvs *****/

char *alloc(sz)	/* allocate sz bytes of memory */
int sz;
{
static 	union abd{
	char		BIG_BUFF[BIG_SIZE] ;
	long		BIG_align ;
	}
	big_un ;

	char		*temp ;

	if ( (BUFF_ptr + sz + sizeof(long) ) >= BIG_SIZE )
	 	{printf("Alloc Error \n\n");   return (NULL) ;}

	temp = & (big_un.BIG_BUFF[BUFF_ptr]) ;	
	BUFF_ptr += sz ;		
	BUFF_ptr += BUFF_ptr % sizeof(long) ;	/* align again */ 

	return(temp);
	}

readstring(s,maxl)	/* read string from stdin */
char	*s;
int	maxl;{

	scanf("%s",s);
	while(*s){
		if (*s >= 'a' && *s <= 'z')
			*s = *s - 'a' + 'A';
		s++;
		}
	return(0);
	}

loadform()	/* load the form file required */
{
	char	t[SR_SCRNML+5];
	 int	i,j,offset;
	char 	*temp_char ;

	strcpy(t,sp->scrnam);
	strcat(t,FMXTUC);	/* add upper case extension */
	if ((fd = open(t,(O_RDONLY))) == -1){	/* file not found */
		strcpy(t,sp->scrnam);
		strcat(t,FMXTLC);	/* try lower case extn */
		if ((fd = open(t,(O_RDONLY))) == -1) /* STILL NOT FOUND */
			reterr(4)	/* file not found */
		}
	if ( (temp_char = alloc(FMH_SZ)) == NULL)
		reterr(6)	/* no space */
	fhp = (struct frmhdr *)temp_char ;
	if (read(fd,(char *)fhp,FMH_SZ) != FMH_SZ)	/* read header */
			reterr(5)	/* read error */
	if ((temp_char = alloc(fhp->noflds * FMF_SZ)) == NULL)					/* alloc for fields */
		reterr(6)	/* no space */
	fap = (struct frmfld *)temp_char ;
	if ((temp_char = alloc(fhp->noflds * FI_SZ) ) == NULL)
				/* alloc for internal data */
		reterr(6)	/* no space */
	fiap = (struct fldinfo *)temp_char ;
	for (i = 0; i < fhp->noflds; i++)	/* read fields */
		if (read(fd,(char *)(fap+i),FMF_SZ) != FMF_SZ)
			reterr(5)
	if ((fvp = alloc(VD_SZ)) == NULL)	/* alloc for variable data */
		reterr(6)
	if (read(fd,(char *)(fvp),fhp->vdsize) != fhp->vdsize) /* read var data */
		reterr(5)
	close(fd);
	strcpy(cfrmnam,sp->scrnam);	/* copy scrn name to curr scrn name */
	if ((ourdr = alloc(fhp->drsize)) == NULL)	/* alloc for data rec*/
		reterr(6)
	if ((userdr = alloc(fhp->drsize)) == NULL) /* converted userdr */
		reterr(6)
	if ((fomrdinfo = (char *)alloc(fhp->noflds)) == NULL)
		reterr(6)
	ffld = -1;
	for (i=0; i<fhp->noflds; i++)	/* find first field in form */
		if ((fap+i)->fldclas != CL_PROM) {
			ffld = i;
			ffldno = (fap+i)->fldno;
			break;
			}
	if (ffld == -1)
		reterr(23)
	for (i=fhp->noflds - 1; i >= 0; i--)
		if ((fap+i)->fldclas != CL_PROM) {
			lfld = i;
			lfldno = (fap+i)->fldno;
			break;
			}
	offset = 0;
	for (i=ffld; i<=lfld; i++){	/* set up internal data items */
		if (elm(i,fldclas) == CL_PROM)
			continue;
		cf = fap+i;
		cfi = fiap+i;
		if ((j = onscrnsz()) <= 0)
			fomintlerr(5)	/* on-screen-size = 0 */
		cfi->oscrnsz = j;
		fillconv();
		if (cfi->type == FITYP_NON){	/* non-numeric field */
				offset = adjoff(offset,ALGN_CHAR);
				cfi->aoff = offset;
				cfi->stortype = STOR_CHAR;
				offset += cf->dfsize+1;
				}
		else if (cfi->decimals == 0)	/* number without decimals */
			/*****
			Changed by amar on 21-jun-89. Now profom structures
			uses either short or long...
			if (cf->dfsize <= IMAXDIGITS) {	* regular int *
				offset = adjoff(offset,ALGN_INT);
				cfi->aoff = offset;
				cfi->stortype = STOR_INT;
				offset += sizeof(int);
				}
			****/
			/*****/
			if (cf->dfsize <= SMAXDIGITS) {	/* regular short */
			    if(cf->fldtyp == TYP_YN) {
				offset = adjoff(offset,ALGN_INT);
				cfi->aoff = offset;
				cfi->stortype = STOR_INT;
				offset += sizeof(int);
				}
			    else {
				offset = adjoff(offset,ALGN_SHORT);
				cfi->aoff = offset;
				cfi->stortype = STOR_SHORT;
				offset += sizeof(short);
				}
				}
			/****/
			else {	/* long int required */
				offset = adjoff(offset,ALGN_LONG);
				cfi->aoff = offset;
				cfi->stortype = STOR_LONG;
				offset += sizeof(long);
				}
		else if (cf->dfsize <= FMAXDIGITS){	/* regular float */
				offset = adjoff(offset,ALGN_FLOAT);
				cfi->aoff = offset;
				cfi->stortype = STOR_FLOAT;
				offset += sizeof(float);
				}
		else {	/* double required */
				offset = adjoff(offset,ALGN_DOUBLE);
				cfi->aoff = offset;
				cfi->stortype = STOR_DOUBLE;
				offset += sizeof(double);
				}
		convau();	/* convert value for current field */
		}
	if (showform())	/* put up initial screen */
		return(1);
	if (FT_CLEAR)
		flush();
	formup=1;
	return(0);
	}

oldform(){	/* check if the form is available and load it if yes */
	register struct storform *sfp;
	register int i;

/* removed code - klv -8th oct,85
	sfp = sfhdp;
	while (sfp != NULL)
		if (!strcmp(sfp->sffmnm, sp->scrnam)){
			fhp = sfp->sffhp;
			fap = sfp->sffap;
			fiap = sfp->sffiap;
			fvp = sfp->sffvp;
			strcpy(cfrmnam,sfp->sffmnm);
			ourdr = sfp->sfourdr;
			userdr = sfp->sfuserdr;
			ffld = sfp->sfffld;
			lfld = sfp->sflfld;
			ffldno = sfp->sfffldno;
			lfldno = sfp->sflfldno;
			fomrdinfo = sfp->sfrdar;
			for (i=ffld; i<=lfld; i++)
				if (elm(i,fldclas) != CL_PROM){
					cf = fap+i;
					cfi = fiap+i;
					convau();
					}
			if (showform())
				return(1);
			formup = 1;
			return(TRUE);
			}
		else
			sfp = sfp->nextsf;
end of removed code */

	return(FALSE);
	}

chkform()	/* check if a new form is required; if so, get it */
{
	if (!formup)		/* no form is currently up */
		if (oldform())
			return(0);
		else if (loadform())
			return(1);
		else
			return(0);
	if (!strcmp(sp->scrnam,cfrmnam)) /* compare names */
		return(-1);	/* same form */
	if (clrall())		/* clear current screen */
		return(1);
	if (oldform())
		return(0);
	if (loadform())	/* and load new one */
		return(1);
	else
		return(0);
	}
formreqd()		/* returns true if a new form is required */
{
	if (!formup)
		return(TRUE);
	if (!strcmp(sp->scrnam,cfrmnam))
		return(FALSE);
	return(TRUE);
	}

showform()	/* put up initial screen */
{
	register int i;
	
	for (i=0; i<fhp->drsize; i++)	/* copy user data record */
		*(ourdr+i) = *(userdr+i);
	if (clearscreen())
		return(1);
	for (i=0; i < fhp->noflds; i++){	/* loop to put up individual fields */
		cf = fap+i;		/* move field to cf */
		cfi = fiap+i;
		if (cf->fldclas == CL_PROM)	/* prompt only field */
			{ if (putprom()) return(1); }	/* so, put it up */
		else switch (chkdv()) { /* check for data record value */
			case -1 :	/* LOW_VALUES */
				{ if (putmask()) return(1); }
					/* so, show the mask */
				if (cf->fldclas == CL_PRMFLD)
					{ if (putprom()) return(1); }
						/* show prompt */
				break;
			case 0 :	/* data value */
				{ if (putdata()) return(1); }
					/* so, show data */
				if (cf->fldclas == CL_PRMFLD) 
					{ if (putprom()) return(1); }
						/* show prompt */
				break;
			case 1 :	/* HIGH_VALUES */
				break;	/* so, do nothing */
			default : 	/* profom internal error */
				fomintlerr(1)
				break;
			}
		}
	return(0);
	}

findfld()	/* find next field and put its address in cf else ret -1 */
{	register int i,nf;

	if (sp->nextfld != 0) {
				/* search by field num */
		nf = sp->nextfld;
		for (i=0; i<fhp->noflds; i++)
			if (elm(i,fldno) == nf){	/* found */
				cf = fap + i;
				cfi = fiap+i;
				return(i);
				}
		return(-1);	/* not found */
		}
	else {		/* search by field name */
		for (i=0; i<fhp->noflds; i++)
			if (!strcmp(sp->nextnam,velm(i,fldnam))){ /* found */
				cf = fap + i;
				cfi = fiap+i;
				return(i);
				}
		return(-1);
		}
	}
getfld(fld)	/* locate field by number and return its index else -1*/
register int	fld;
{
	register int i;
	
	for (i=0; i<fhp->noflds; i++)
		if (elm(i,fldno) == fld){	/* found it */
			cf = fap+i;
			cfi = fiap+i;
			return(i);
			}
	return(-1);
	}
clrall()	/* clear the screen and release all areas */
{
	register struct storform *sfp;

	if (clrscrn())
		return(1);

/* removed temporarily - klv - 8th oct 85
	sfp = sfhdp;
	while (sfp != NULL)
		if (!strcmp(cfrmnam,sfp->sffmnm)){
			formup = 0;
			return(0);
			}
		else
			sfp = sfp->nextsf;
	if ((sfp = (struct storform *) alloc(STFMSZ)) == NULL)
		reterr(6)
	sfp->sffhp = fhp;
	sfp->sffap = fap;
	sfp->sffiap = fiap;
	sfp->sffvp = fvp;
	sfp->sfffld = ffld;
	sfp->sflfld = lfld;
	sfp->sfffldno = ffldno;
	sfp->sflfldno = lfldno;
	sfp->sfrdar = fomrdinfo;
	sfp->sfourdr = ourdr;
	sfp->sfuserdr = userdr;
	strcpy(sfp->sffmnm,cfrmnam);
	sfp->nextsf = sfhdp;
	sfhdp = sfp;
end of removed portion */

/* new code */
/***** IT was commeted out .. I removed comments to see if Malloc memory
	faults can be handled by freeing ... kvs 30-Oct-86 ***/

/***** Well.. I commented it again as I allocated all memory in static 
    area as W50 was giving memory fault in paymnt screen while CM was
    Ok. Very mysterious as W50's total malloc is less then CM's malloc.
    As a last resort I had to do this ...kvs 9-Nov-86 ..
	free(fomrdinfo);
	free(userdr);
	free(ourdr);
	free(fvp);
	free(fiap);
	free(fap);
	free(fhp);
*******/

/** Instead **/  
	BUFF_ptr = 0 ;	

/**** Old end of commented code .. kvs  30-Oct-86 ****/

	cfrmnam[0] = 0;
/* end of new code */
	formup=0;
	return(0);
	}

fldrange()	/* find range of fields from info in status record and return
			0 if ok else return 1;
			if ok put range in firstfld & lastfld */
{	register int i,found,nf;

	found=FALSE;
	if (sp->nextfld == 0){
			/* search by field name */
		for (i=0; i<fhp->noflds; i++)	/* loop thru fields */
			if (!strcmp(sp->nextnam,velm(i,fldnam))){ /* found */
				firstfld=i;
				found=TRUE;
				break;
				}
		if (!found)
			reterr(7)
		/* search for end-field */
		found=FALSE;
		for (i=fhp->noflds-1; i>=0; i--)
			if (!strcmp(sp->endnam,velm(i,fldnam))){ /* found */
				lastfld=i;
				found=TRUE;
				break;
				}
		if (!found)
			lastfld=lfld;
		return(0);
		}
	else{		/* search by field number */
		if (sp->nextfld == 1)
			firstfld=ffld;
				/* assume start at first field in form */
		else{	/* search for the first field of range */
			nf=sp->nextfld;
			for (i=0; i<fhp->noflds; i++)
				if ((fap+i)->fldno == nf){
					found=TRUE;
					firstfld=i;
					break;
					}
			if (!found)
				reterr(7)	/* field not found */
			}
		if (sp->endfld == 0)
			lastfld=lfld;	/* assume end at last field in form */
		else{	/* search for last field  of range */
			nf=sp->endfld;
			found=FALSE;
			for (i=fhp->noflds-1; i>=0; i--)
				if ((fap+i)->fldno == nf){
					found=TRUE;
					lastfld=i;
					break;
					}
			if (!found)
				lastfld=lfld;
			}
		return(0);
		}	
}


