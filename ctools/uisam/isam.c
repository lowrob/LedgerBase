/******************************************************************************


	INDEXED SEQUENTIAL FILE IMPLEMENTATION USING B_TREE DATASTRUCTURES. IT
	ALLOWS TWO MODES OF ACCESS - SEQUENTIAL AND RANDOM .

	THE FOLLOWING COMMANDS ARE ALLOWED.
	------------------------------
	1)  iscreat	To create an ISAM file .. 
	2)  isopen	To open an existing ISAM file .
	3)  isclose	To close an open file.
	4)  isreadr	To perform random read on open file.
	5)  isreads	To perform sequential read on open file after isstart.
	6)  iswrite	To write another record to open file.
	7)  isstart	To position file pointer at a particular record for dynamic mode
	8)  isreadp	To read previous record after isstart is done .
	9)  isdelete	To delete Record and its associated keys .
	10) isdone	To unlock index nodes after sequential processing .
	11) isrelaese	To realease all locks on INDEX and DATA file


Modification History :

	Multiuser mode , B+ Tree .. Jaydeep .

	This has a dBase like record deletion and recall fecility
	That is isdelete no longer deletes the physical record and
	keys from index but marks the first byte of the record
	as deleted . It has its drawbacks and advantages .Simpl-
	icity of implementation and recovery possibiliy is the
	reason to go for it .

	 : Dt: 15-Jul-88	.. kvs

29-sep-89    Amar
	In variable record length version stored record length is inclusive
	of status byte. So serachval.slength contains (reclen+STATUS_LEN).
	For fixed length version whereever currslot.reclength is assigned
	to searchval.slength, STATUS_LEN is added. And some places
	searchval.slength is returned to user as a record length. This is
	changed to return (searchval.slength - STATUS_LEN).

01-NOV-89    Amar
	In iscreat() file closing and opening is moved to before writeheader(),
	to take care of write problem on created files in MS-DOS MS 5.0 version.

17-NOV-89    Amar (As per Shankar's Advice)
	"isrecrt.out" utility, which compresses the ISAM file by skipping
	all deleted records, reads the file in Seq order of main key and writes
	to new file. Because of this hierarchially arranged keys insertion,
	index file size will increase. To take care of this problem split_leaf()
	& split_node() are modified to spilt in specified ratio instead of fifty
	fifty.
20-NOV-89    Amar
	In "iscreat()", "pnumkeys" of each INDEX Key is initialized to zero.
	Earlier it is containing junk value, because it is not initialized.
26-MAR-90	AMAR
	Change:
		One line inserted in the begining of isstart() call to set
		"just_started" flag to OFF.

	BUG:
		"just_started" flag is set to ON in isstart() when key is
		matched, and set to OFF in readprv(). This flag should be set
		to OFF when the mode of start is either ISFIRST or ISLAST or
		(ISEQUAL and key not matched). Some times if isstart() set
		this flag to ON and next call is not isreadp() this flag remains		ON. This is causing problem when you call isstart() with ISLAST
		and now call isreadp().

05-Apr2020 Louis R.
	New compiler, need to define functions and prototype.

*****************************************************************************/

#include	"isnames.h"
#include	"isdef.h"
#include	"isflsys.h"
#include	<stdio.h>
#include	<string.h>
#include	"iserror.h"

#define LAST_ERCODE	123 	/* dt 8-Jan-86 ..kvs */

#define CR_READ		1
#define CR_WRITE	2
#define CR_UPDATE	3
#define CR_SREAD	4

#define currslot	slots[slotnum]
#define currindex	(*(currslot.ktable+currkeyno))

#define perror		prtf("\nError %d \n\n",iserror)

//char	*malloc();

/**************************************************************************
 last position in data file where record was written and the file fd .....
 Required by DBH .
***************************************************************************/

long	last_posn ;
int	data_fd ;

struct	searchret	searchval ;

/* search results are passed between procedures using this structure. */
/* iserror is a global variable in which error codes are returned. */

int	iserror;


/************************************************************************/
/*					*/
/*	All statics are declared here			*/
/*					*/
/************************************************************************/

static	union	buf_dummy {
	char	buff1[INXHSZ*3];
	int	dumm1;
	} dum_un1;

static	char	*ptr1 = dum_un1.buff1 ;
static	char	*ptr2 = dum_un1.buff1+INXHSZ ;
static	char	*ptr3 = dum_un1.buff1+2*INXHSZ ;	/* buf1+2048 */

static	int	currkeyno; /* The current key(one among the alternates )being used */

static	union	{
	char	kbuff1[MAX_KEY_LEN];
	int	dumm1;
	} key_dum2 ;

static	char	*keybuffer = key_dum2.kbuff1 ;

static	struct	indxinfo	slots[MAXFILES]; /* The file table */

static	int	slotnum;	/* Current slotnum */

/***** slotnum is a global variable which points to the file table entry */
/***** for the currently referenced file . */

#define	STACKSZ		20
#define	offset(i)	(FIXEDPART + (i-1)*currindex.pkeylength )

static	long	cur_addr ;	/* current address */

static	union	key_dummy {
	char	char_buff [MAX_KEY_LEN];
	int	dum_int;
	} dum_un3;	/* For integer aligned key */

static	char	*cur_key = dum_un3.char_buff ;

static	int	reclen ;

static	int	stacktop[MAXFILES];

static	int	posinnode[MAXFILES][STACKSZ];

static	int	stack[MAXFILES][STACKSZ];

static	int	status[MAXFILES] ;	/* assumed initialised to 0 at start */

static	int	just_started ;		/* if last successful call was isstart*/

/* Next line is inserted on 17-NOV-89 by Amar */
float	spl_ratio = 0.5 ;		/* Split ratio for node & leaf */

int	DT_TYPE = DDMMYY ;

/**** The functions delares start here - Louis R ***/
static int buildtables(int);
static int writeheader(void);
static int alloctable(void);
static int getslot(void);
static int occupyslot(int);
static int filopen(void);
static int readrand(char *, int);
static int getdata(char *, int);
static int isstatus(void);
static int datseek(long, int, int);
#ifdef	FIXED_RECLEN
static int writeout(char *, int *);
#else
static int writeout(char *, int,  int *);
#endif
static int stackunlock(void);
static int stack_empty(void);
static int assemblekey(char *, char *);
static int rewdat(char*, int);
static long seek(int);
static int insert(int, long);
static int getnode(void);
static int split_leaf(void);
static int split_node(void);
static int putin(int, struct header *);
static int newroot();
static int initialise();
static int readnode(int, char *);
static int output(int, char *);
static int push(int, int);
static int parent(int *);
static int putdata(char *, int);
int isreads(int, char *, int);
static int readseq(char *, int);
static int copykeys(char *, char *, int);
static int compkeys(char *, char *);
static int copyfields(struct field *, struct field *, int);
static int msearch(char *);
static int readprv(char *, int);
static int chkroot(int);
static int locknode(int, int);
static int writesearch(char *);
static int unlocknode(int);
static long conv_date(long, int);


/**** The functions start here	***/


/************************************************************************/
/*									*/
/* ISCREAT() :								*/
/*									*/
/*	This routine craetes INDEXED files for the file name		*/
/*	The file should not already exist				*/
/*									*/
/************************************************************************/

#ifdef	FIXED_RECLEN
int	iscreat(flnam,iomode,key_count,keysarray, reclen)
int	reclen ;	/* record length */
#else 
int	iscreat(flnam,iomode,key_count,keysarray )
#endif

char	*flnam;
int	iomode,		/* R,W, RWR */
	key_count,	/* Main key + alt keys */
	keysarray[];	/* Key descriptor array */
{
char	tfil[FLNM_LEN+4];
int	*pptr, i ;

	if(access(flnam,0) >= 0) 
		reterr(FLEXSTERR)
	if (strlen(flnam) >= FLNM_LEN )
		reterr(FLNAMEERR)

	if(iomode != RWR &&iomode != W && iomode != R)
		reterr(INVMODERR)

#ifdef	FIXED_RECLEN
	if(reclen>MAX_REC_LEN) reterr(INVRLNERR); /*RECORD LENGTH > MAXLENGTH*/
#endif

/******** Validate the keys data ***********/

	if((slotnum = getslot()) == ERROR)
		reterr(FILCNTERR); 
	currslot.numrecords = key_count;
	currslot.iomode = iomode;
	currslot.freedat = currslot.freenodes = currslot.nextnode = 0;
	currslot.altkeys = key_count;
	currslot.activekey = currslot.nxtposn = 0;
#ifdef	FIXED_RECLEN
	currslot.reclength = reclen ;
#endif

/**  initialise the key information in the key table of the curr slot  */

	if(alloctable() == ERROR)
		reterr(MEMORYERR); 
	for(currkeyno = 0;currkeyno<key_count;currkeyno++) {
		currindex.pkeyparts = *keysarray++ ;
		currindex.pkeylength = 0 ;
		pptr = keysarray ;

/**  the following loop computes the length of each key--the sum of parts */

		for(i = 0 ; i<currindex.pkeyparts ; i++, pptr += 4) {
			switch (*pptr) {

			case LONG :	currindex.pkeylength  += *(pptr+1)* sizeof(long) ;
					break ;

			case SHORT :	currindex.pkeylength  += *(pptr+1)* sizeof(short);
					break ;

			case DATE :	currindex.pkeylength  += *(pptr+1)* sizeof(long) ;
					break ;

			case FLOAT :	currindex.pkeylength  += *(pptr+1)* sizeof(float) ; 
					break ;

			case DOUBLE :	currindex.pkeylength  += *(pptr+1)* sizeof(double) ; 
					break ;

			case CHAR :	currindex.pkeylength  += *(pptr+1) ;
					break ;

			default :	reterr(INVKTYERR) ;
					
			}
			if (*(pptr+1) <0) reterr(INVKLNERR)	/* keypart length */
			if (*(pptr+2) <0) reterr(INVKPSERR)	/* keypart position*/
			if (*(pptr+3)  != ASCND && *(pptr+3)  != DESCND) reterr(INVORDERR) 
		}

		if ( currindex.pkeylength > MAX_KEY_LEN ) reterr(KEYSIZERR) 

		pptr = (int *) malloc((unsigned)((currindex.pkeyparts*4)*sizeof(int))) ;
		if ((currindex.partsarray = pptr) == NULL) reterr(MEMORYERR) ;
		for (i = 0 ; i<(currindex.pkeyparts*4) ; i++)
			*pptr++ = *keysarray++ ;
		currindex.proot     = currkeyno+1;
		currindex.pnodesize = MAXSZ;
		currindex.pnumkeys  = 0;  /* Inserted by AMAR on 20-NOV-89 */
		currindex.pmaxkeys  = ((MAXSZ-HEADSZ) /
				 	(2*(FIELDSZ+currindex.pkeylength )))*2 ;
	}
	strcpy(currslot.flnam,flnam);
	if((currslot.fd1 = creat(flnam , PMODE))<0)
		reterr(DFLCRTERR); 

/* CREATE ERROR FOR DATA FILE */

	strcpy(tfil,flnam);
	strcat(tfil,".IX");
	if((currslot.fd2 = creat(tfil,PMODE))<0)
		reterr(IFLCRTERR); 

/*
* 
* write static	header info.. at create time every thing 
* from curr slot has to be written.
* 
*/

	/*
	*  Next 3 lines are inserted before writeheader(), to take care of
	*  write problem on created files in MS-DOS MS 5.0 version.
	*  AMAR on 01-NOV-89.
	*/

	if(close(currslot.fd1)<0)  reterr(DFLCLSERR); 
	if(close(currslot.fd2)<0)  reterr(IFLCLSERR); 
	if(filopen() == ERROR) 	   return (ERROR) ;

	if(writeheader() == ERROR) return (ERROR) ;
	if(initialise() == ERROR)  reterr(INITERR); 
	/****
	if(close(currslot.fd1)<0)  reterr(DFLCLSERR); 
	if(close(currslot.fd2)<0)  reterr(IFLCLSERR); 
	if(filopen() == ERROR) 	   return (ERROR) ;
	****/

	occupyslot(slotnum);
	return (slotnum);
}


/************************************************************************/
/*									*/
/* ISOPEN :								*/
/*									*/
/************************************************************************/

int	isopen(flnam,iomode)
char	*flnam;
int	iomode;

{

	if(access(flnam,0) <0) 
		reterr(FLACCSERR) 
	if (strlen(flnam) >= FLNM_LEN )
		reterr(FLNAMEERR)

	if(iomode != RWR &&iomode != R && iomode != W)
		reterr(INVMODERR); /* CHECK IOMODE FOR R/W/RWR */
	if((slotnum = getslot()) == ERROR)
		reterr (FILCNTERR) ;

	currslot.iomode = iomode;
	currslot.activekey = currkeyno = 0;

	strcpy(currslot.flnam,flnam);
	if(filopen() == ERROR) return (ERROR);

	if(buildtables(1) == ERROR) return (ERROR);
	occupyslot(slotnum) ;
	currkeyno = 0 ;
	return (slotnum);
}

/****************************************************************************/

static	int	buildtables(alloc_flg) 
int	alloc_flg ;		/* if occupy slot etc are to be called */
{
struct	indxheader	*header ;
struct	keydat		*altkptr;

int	savekey, i ;
int	*sptr, *dptr ;

	savekey = currkeyno ;

	if(seek(0) == ERROR) 
		reterr(NODSKERR);
	if(read(currslot.fd2,ptr2,INXHSZ)<INXHSZ)
		reterr(IFLRDERR); /* SIZE OF HEADER BLOCK IN FILE < SIZE DECLARED IN ISNAMES.H */

	header = (struct indxheader *)ptr2 ;
	currslot.numrecords = header->pnumrecs ; /*added .. if i have luck*/
#ifdef	FIXED_RECLEN
	currslot.reclength =  header->preclength ;
#endif

	currslot.nxtposn = header->pnxtposn;
	currslot.nextnode = header->pnextnode;
	currslot.freedat = header->pfreedat;
	currslot.freenodes = header->pfreenodes;
	currslot.altkeys = header->paltkeys;

/** If occupyslot to be called then check **/

	if ( alloc_flg )
		if (alloctable() == ERROR) return (ERROR) ;
	altkptr = (struct keydat *)(++header); /* deserves attention..*/

	for(currkeyno = 0;currkeyno<currslot.altkeys;currkeyno++) {
		currindex.proot = altkptr->proot;
		if (currindex.proot == -1) reterr (CRPIXFLERR) ;
		currindex.pmaxkeys = altkptr->pmaxkeys;
		currindex.pnodesize = altkptr->pnodesize;
		currindex.pnumkeys = altkptr->pnumkeys;
		currindex.pkeyparts = altkptr->pkeyparts ;
		currindex.pkeylength = altkptr->pkeylength ;
		altkptr++ ;
		sptr = (int *)altkptr ;
		if (alloc_flg) {
			if ((currindex.partsarray = dptr = (int *)malloc((unsigned)(currindex.pkeyparts*4)*sizeof(int))) == NULL) reterr(MEMORYERR) ;
			for (i = 0 ; i<(currindex.pkeyparts*4) ; i++)
				*dptr++ = *sptr++ ;
		}
		else sptr += (currindex.pkeyparts*4) ;
		altkptr = (struct keydat *)sptr ;
	}
	currkeyno = savekey ; 	/*restore currkeyno to its initial value*/
	return (NOERROR);
}


/****************************************************************************/

static	int	writeheader()
{
struct	indxheader	*header;
struct	keydat		*altkptr;

int	i, hdr_sz, savekey ;
int	*sptr, *dptr ;

	savekey = currkeyno ;
	header = (struct indxheader *) ptr2;

	header->pnumrecs = currslot.numrecords ; /* added .. if lucky */
#ifdef	FIXED_RECLEN
	header->preclength  = currslot.reclength ;
#endif
	header->pnxtposn = currslot.nxtposn;
	header->pnextnode = currslot.nextnode;
	header->pfreedat = currslot.freedat;
	header->pfreenodes = currslot.freenodes;
	header->paltkeys = currslot.altkeys;

	altkptr = (struct keydat *) (++header); /*** deserves attention ****/

	hdr_sz = sizeof (struct indxheader) +
		 (sizeof (struct keydat) - sizeof (int *)) * currslot.altkeys ;
	if (hdr_sz > INXHSZ) reterr (KEYOVFERR) ;

	for(currkeyno = 0;currkeyno<currslot.altkeys;currkeyno++) {
		altkptr->proot = currindex.proot;
		altkptr->pmaxkeys = currindex.pmaxkeys;
		altkptr->pnodesize = currindex.pnodesize;
		altkptr->pnumkeys = currindex.pnumkeys;
		altkptr->pkeyparts = currindex.pkeyparts ;
		altkptr->pkeylength = currindex.pkeylength ;
		dptr = (int *)(altkptr+1) ; sptr = currindex.partsarray ;

		hdr_sz += currindex.pkeyparts * 4 * sizeof (int) ;
		if (hdr_sz > INXHSZ) reterr (KEYOVFERR) ;

		for(i = 0 ; i<(currindex.pkeyparts*4) ; i++)
			*dptr++ = *sptr++ ;
		altkptr = (struct keydat *) dptr ;
	}

	if ( seek(0) == ERROR ) 
		reterr(NODSKERR); 

	if(write(currslot.fd2,ptr2,INXHSZ) < INXHSZ) 
		reterr(IFLWRERR);

	currkeyno = savekey ;
	return (NOERROR);

}


/*****************************************************************************/

static	int	alloctable()
{

	if((currslot.ktable = (struct keydat *) malloc((unsigned)(currslot.altkeys*sizeof(struct keydat)))) == NULL) 
		reterr(MEMORYERR); /* KEY TABLE ALLOCATION ERROR */
	return (NOERROR);

}


/*
* slot maintenance routines 
*/
 
static	int	getslot ()	/* return the next available slot number */
{
int	slot;

	for(slot = 0;slot<MAXFILES;slot++)
		if(status[slot] == 0) return (slot);
	return (ERROR) ;
}


/****************************************************************************/

static	int	occupyslot(slot) 
int	slot ; /* Mark this slot as not avalilable */
{

	status[slot] = 1;
}


/****************************************************************************/


static	int	filopen()
{
char	indxflnam[FLNM_LEN+4];
int	mode ;

	strcpy(indxflnam,currslot.flnam);
	strcat(indxflnam,".IX");

	if (currslot.iomode == W) mode = WRONLY ;
	else
		if (currslot.iomode == R) mode = RDONLY ;
		else mode = READWR ;

	if ( (currslot.fd1 = open (currslot.flnam, mode))<0)
		reterr(DFLOPNERR); 	/* ERROR IN OPENING DATA FILE */

	if ( (currslot.fd2 = open (indxflnam, READWR))<0)
		reterr(IFLOPNERR); 	/* ERROR IN OPENING INDEX FILE */

	data_fd = currslot.fd1 ;
	return (NOERROR) ;

}


/****************************************************************************/


static	int	isfree(slot)		
int	slot ; /* Is this slot free ? */
{

	if(status[slot] == 0) return (TRUE);
	return (FALSE);
}


/****************************************************************************/


static	int	freeslot(slot)
int	slot ; /* Mark this as available */
{

	status[slot] = 0;
}


/************************************************************************/
/*									*/
/* ISCLOSE() :								*/
/*									*/
/************************************************************************/

int	isclose(fp)
int	fp; /* close slot fp */
{

	if(fp<0 ||fp >= MAXFILES)
		reterr(INVFDERR); /* INVALID fILE POintER */
	slotnum = fp;
	if(isfree(slotnum) == TRUE) 
		reterr(FLNOPNERR); /* FILE NOT OPENED */

/**************************************************************************
 
	 commented out for Multiuser ... 
 write the header if close called.. write the header with modified info 
	 if( writeheader() == ERROR) return (ERROR);
		
**************************************************************************/

	close(currslot.fd1);
	close(currslot.fd2);

/*************************************************************************
	 following three lines to be commented out if 
		
		 if(currslot.iomode == W ||currslot.iomode == RWR)
		 if(exchfiles() == ERROR)
		 return (ERROR);
*************************************************************************/

/** added by shankar 29/3/89 **/

	for(currkeyno = 0 ; currkeyno < currslot.altkeys ; currkeyno++) {
		free ((char *) currindex.partsarray) ;
	}

/**  end addition **/

	free((char *)currslot.ktable);
	freeslot(slotnum);
	return (NOERROR);

}


/************************************************************************/
/*									*/
/* 	ISREADR() :							*/
/*									*/
/************************************************************************/

int	isreadr(fp,bufkey,thekey, lock)
int	fp;			/* slotnum of file */
int	thekey ;		/* Key number of index */
int	lock ;			/* 0 - No data rec lock */
char	*bufkey ;		/* The key of the needed record in record */
{

	if(fp<0 ||fp >= MAXFILES) 
		reterr(INVFDERR); /* INVALID FILE POintER */
	slotnum = fp;
	if(currslot.altkeys <= thekey)
		reterr(INVFDERR); /* KEY NUMBER EXCEEDS KEYS DEFINED AT CREATE TIME */
	currslot.activekey = currkeyno = thekey;
	if(isfree(slotnum) == TRUE) 
		reterr(FLNOPNERR); /* FILE NOT OPENED */
	if (currslot.iomode == W) reterr (IMPMODERR) ;
	return (readrand(bufkey, lock));
}


/****************************************************************************/

static	int	readrand(bufkey, lock)
int	lock ;
char	*bufkey;
{
int	result ,i,p ;
int	o_p, o_i ;		/* for new delete logic .. kavi */
char	n_keybuf[MAX_KEY_LEN] ;

	assemblekey(bufkey,keybuffer) ;
	result = msearch(keybuffer) ;

/*
*	Save last Node on stack as it is going to be unstacked ..
*/
	o_p = parent(&o_i) ;
	push(o_p, o_i) ;

	/*** Unlock all Nodes ***/

	for(;;) {
		if ( (p = parent(&i)) == 0 ) break ;
		unlocknode(p) ;
	}

	if ( result == ERROR ) return (ERROR); 
	if(searchval.j == 0) return (UNDEF) ; 
	if ( isstatus() == SET_DEL ) {
		if (currkeyno == 0 ) return (UNDEF) ;	/*Only one active key */

/*
* 		Get next undeleted record whose key matches with that
*	of required key .. (New delete logic)
*/ 

		push(o_p, o_i) ;	/* for readseq */
		result = readseq(bufkey, lock) ;
		if(result == EFL ) result = UNDEF ;
		if ( result < 0 ) return (result) ;
		o_p = parent(&o_i) ;	/* Pop off */

		assemblekey(bufkey, n_keybuf) ;		/* form its key */
		if ( compkeys(keybuffer, n_keybuf)  != 0 ) return (UNDEF);
	}
	else
		if ((result = getdata(bufkey, lock)) == ERROR ||
			result == LOCKED ) return (result) ;

	return (searchval.slength - STATUS_LEN);
} /* readrand */


/****************************************************************************/


static	int	getdata(buffer, lock) /* details vaailable in searchval */
char	*buffer;
int	lock ;
{
int	length, result ;
long	position;

#ifdef	FIXED_RECLEN
	length = currslot.reclength ;
#else
	length = searchval.slength-STATUS_LEN;	/* Take care of status byte */
#endif
	position = searchval.sposition+STATUS_LEN;
	last_posn = position ;
	if ( (result = datseek(position, lock, length)) < 0 ) return (result);
	if(read(currslot.fd1,buffer,length)<length) 
		reterr(DFLRDERR);
	return (length);
}


static	int	isstatus() /* returns status byte of the record */
{
char	st_byte[2] ;

	if(lseek(currslot.fd1,searchval.sposition,0)<0) reterr(DFLSKERR)
	if(read(currslot.fd1,st_byte,1) < 1 ) reterr(DFLRDERR)
	return ((int)(st_byte[0]));
}


/****************************************************************************/


static	int	datseek(position, lock, size)
long	position;
int	lock , size;
{
int	wait, code;

	if(lseek(currslot.fd1,(long)position,0)<0)
		reterr(DFLSKERR)

	wait = 0 ;	/* No wait on record lock .. return with error..and
			you are going for only Read lock */
	if(lock){

/*
* 	 write lock to ensure no other locks are held.
* 	 Then convert to read lock 
*/

		code = e_lock( currslot.fd1, WRLOCK, wait, position, size) ;	
		if(code < 0)return (code);
		return ( e_lock( currslot.fd1, RDLOCK, wait, position, size) ) ;	
	}

/* Added on 18-jul-88 */
	else 
		return ( e_lock( currslot.fd1,UNLOCK, wait, position, size) ) ;	
}


#ifdef	FIXED_RECLEN
int	iswrite(fp, buffer, alt_write)
#else
int	iswrite(fp,buffer,length,alt_write)
int	length ;
#endif

int	fp;
char	*buffer; /* record te written */
int	*alt_write ;/* To write a key or Not */
{
	
	if(fp<0 ||fp>MAXFILES) 
		reterr(INVFDERR); /* INVALID FILE POIER */
#ifndef	FIXED_RECLEN
	if(length>MAX_REC_LEN) reterr(INVRLNERR);/* RECORD LENGTH > MAXLENGTH */
#endif
	slotnum = fp;
	if(isfree(slotnum) == TRUE) reterr(FLNOPNERR); /* FILE NOT OPENED */
	if (currslot.iomode == R) reterr (IMPMODERR) ;
#ifdef	FIXED_RECLEN
	return (writeout(buffer, alt_write));
#else
	return (writeout(buffer,length, alt_write));
#endif
}


/****************************************************************************/


#ifdef	FIXED_RECLEN
static	int	writeout(buffer, alt_write)
#else
static	int	writeout(buffer, length, alt_write)
int	length ;
#endif
char	*buffer;
int	*alt_write ;
{
int	retcode ;
long	saveposn ;
char	st_buff[2] ;

	if (locknode(0,WRLOCK) == ERROR) return (ERROR) ;
	if (buildtables(0) == ERROR) return (ERROR) ;
	
	st_buff[0] = SET_ACTIVE ;
	if(putdata(st_buff, STATUS_LEN) == ERROR) return (ERROR) ;

	saveposn = currslot.nxtposn ;
	currslot.nxtposn += STATUS_LEN;
#ifdef	FIXED_RECLEN
	if(putdata(buffer,currslot.reclength) == ERROR) return (ERROR) ;
#else
	if(putdata(buffer,length) == ERROR) return (ERROR) ;
#endif
	last_posn = currslot.nxtposn ;
	data_fd = currslot.fd1 ;
#ifdef	FIXED_RECLEN
	currslot.nxtposn += currslot.reclength ;
#else
	currslot.nxtposn += length ;
#endif

	for(currkeyno = 1; currkeyno<currslot.altkeys ; currkeyno++) 
		if ( *(alt_write + currkeyno) == 1 ) currindex.pnumkeys++;
	currkeyno = 0 ;
	currindex.pnumkeys++ ;
	if ((retcode = writeheader()) == ERROR) return (retcode) ; 
	if ((retcode = unlocknode(0)) == ERROR) return (retcode) ;
	currkeyno = 0 ;
	assemblekey(buffer,keybuffer) ;
	if((retcode = writesearch((keybuffer))) == ERROR) {
		stackunlock() ;
		return (retcode) ;	
	}

/* 	Checking searchval.j == 1 is No good .
* 	as writesearch may return UNDEF for main key even for this condition..
*/

	if ( retcode == DUPE ) {	
		stackunlock() ; 
/*
*	Set the status byte to indicate deleted record ..
*/

		searchval.sposition = saveposn ;
		st_buff[0] = SET_DEL ; 
		rewdat(st_buff, 1) ;	
		return (retcode) ; 
	}
#ifdef	FIXED_RECLEN
	if ((retcode = insert(currslot.reclength+STATUS_LEN,saveposn))==ERROR){
#else
	if ((retcode = insert(length+STATUS_LEN,saveposn)) == ERROR) {
#endif
		stackunlock() ;
		return (retcode) ;
	}
	for(currkeyno = 1; currkeyno<currslot.altkeys ; currkeyno++) {
		if ( *(alt_write + currkeyno ) == 0 ) continue ;
		assemblekey(buffer,keybuffer) ;
		if((retcode = writesearch((keybuffer))) == ERROR) {
			stackunlock();
			return (retcode) ;
		}
#ifdef	FIXED_RECLEN
		if((retcode = insert(currslot.reclength+STATUS_LEN,saveposn)) 
								== ERROR) {
#else
		if((retcode = insert(length+STATUS_LEN,saveposn)) == ERROR) {
#endif
			stackunlock();
			return (retcode) ;
		}
	}
#ifdef	FIXED_RECLEN
	return (currslot.reclength);
#else
	return(length) ;
#endif 

} /** Iswrite ***/

static	int	stackunlock()
{
int	p, i ;

	if (stack_empty ()) return ;

	while ((p = parent(&i)) != 0) unlocknode(p) ;
}


static	int	stack_empty ()
{
int	sp ;

	sp = stacktop[slotnum]-1;
	if(sp<0 ||sp >STACKSZ) return (1);
	return (0) ;
}


/************************************************************************/
/*									*/
/* ISREWRITE() :							*/
/*									*/
/************************************************************************/

#define ret_free(N)	{ free(datrecord) ; return (N) ; }


#ifdef	FIXED_RECLEN
int	isrewrite(fp,buffer, alt_write)
#else
int	isrewrite(fp,buffer,length,alt_write)
int	length; 
#endif
int	fp ;
char	*buffer;
int	*alt_write ;
{
long	saveposn ;
int	result ;
char	n_keybuff[MAX_KEY_LEN], st_byte[2] ;
char	*datrecord ;
	

	if(fp<0 ||fp>MAXFILES) reterr(INVFDERR); /* INVALID FILE POIER */
#ifndef	FIXED_RECLEN
	if(length>MAX_REC_LEN) reterr(INVRLNERR); /* RECORD LENGTH > MAXLENGTH*/
#endif
	slotnum = fp;
	if(isfree(slotnum) == TRUE) reterr(FLNOPNERR); /* FILE NOT OPENED */
	if (currslot.iomode != RWR) reterr (IMPMODERR) ;

	if (locknode(0,RDLOCK) == ERROR) return (ERROR) ;
	if (buildtables(0) == ERROR) return (ERROR) ;
	unlocknode(0) ;

	currkeyno = 0 ;
	assemblekey(buffer, keybuffer) ;
	result = writesearch(keybuffer) ;
/*
* Since we are not going to call "insert()" which would have unlocked nodes
* locked by writesearch, we will have to unlock here ..
*/
	stackunlock() ;	

	if ((result == ERROR) || (result == UNDEF)) return (result) ; 
	saveposn = searchval.sposition ; 


/*
* If old record is of same length as new record and all keys match then use
* the same area ..
*/
#ifndef	FIXED_RECLEN
	if ( searchval.slength - STATUS_LEN == length ) {
#endif
		if ( (datrecord = malloc((unsigned)searchval.slength)) == NULL ) reterr(MEMORYERR)
		if ( (result = getdata(datrecord, 0)) < 0 ) ret_free(result) 	
		for( currkeyno = 1 ; currkeyno < currslot.altkeys ; currkeyno++ ) {

/* Compare all keys .. Ignore alt_write here */

			assemblekey(buffer, n_keybuff) ;
			assemblekey(datrecord, keybuffer) ;
			if ( compkeys(keybuffer, n_keybuff)  != 0 ) break ;
		}
		free(datrecord) ;
 		if ( currkeyno == currslot.altkeys ) {
			searchval.sposition = saveposn ;
			searchval.sposition++ ;		/* Skip status byte */
#ifdef	FIXED_RECLEN
			return ( rewdat(buffer, currslot.reclength)) ;
#else
			return ( rewdat(buffer, length)) ; /* rewrite */
		}
#endif
	}

/* 	ELse ..
*	Delete the original record and insert new record .. This will give a
*	good logging mechanism also , as old records can be retrived and 
*	checked for what changes were made on them .
*/
	
/* if ( (result = isdelete(fp, buffer)) < 0 ) return (result) ; */

	searchval.sposition = saveposn ;
	st_byte[0] = SET_DEL ;
	rewdat(st_byte, STATUS_LEN) ; 
#ifdef	FIXED_RECLEN
	return (writeout(buffer, alt_write )) ;
#else
	return (writeout(buffer, length, alt_write )) ;
#endif

/*
* 	May put close() and reopen of Data file in rewrite Case if record
*	Appends !!!.
*/

}

/****************************************************************************/

static	int	assemblekey(datrecord,ptr)
char	*datrecord;
char	*ptr ; /* the key should be filled in this buffer**/
{
int	i, j, plen ;
int	*parray ;
char	*temp ;

	for (i = 0, parray = currindex.partsarray ; i<currindex.pkeyparts ; i++,parray += 4) {
		switch (*parray) {
		case LONG	:
		case DATE :	plen = *(parray+1)*sizeof(long) ;
				break ;
		case SHORT :	plen = *(parray+1)*sizeof(short) ;
				break ;
		case FLOAT :	plen = *(parray+1)*sizeof(float) ;
				break ;
		case DOUBLE :	plen = *(parray+1)*sizeof(double) ;
				break ;
		case CHAR :	plen = *(parray+1) ;
				break ;
		}
		temp = datrecord+*(parray+2) ;
		for(j = 0 ; j<plen ; j++)
			*ptr++ = *(temp+j) ;
	}
	return (NOERROR);
}


/****************************************************************************/

static	int	rewdat(buffer,length) /* rewrite data record on original record */
int	length;		/* length of new record */
char	*buffer;	/* data record */
{
int	result ;

/**	The record will be locked by rewrite.. Locking should		**/
/**	be done by isreadr(s) before updation attempt			**/
/**	If we have F_TEST call to test if the data record is locked	**/
/**	or not then that could be useful here 				**/

	if((result = datseek(searchval.sposition, 1,length))<0) return (result);
	last_posn = searchval.sposition ;
	if(write(currslot.fd1,buffer,length) < length) reterr(DFLWRERR);
	if((result = datseek(searchval.sposition, 0,length))<0) return (result);
	return (NOERROR);
}



/****************************************************************************/


/** internal seek routine **/

static	long	seek(p)		/* seek for the p'th record */
int	p; 	/* seek to the p'th record */
{
long	ret_pos ;

	if ((ret_pos = p) != 0)
		ret_pos = (long)INXHSZ + (long)currindex.pnodesize * (long)(p-1) ;
	if (lseek (currslot.fd2, ret_pos, 0) != ret_pos) return (ERROR) ;
	return (ret_pos) ;
}


/****************************************************************************/
 
static	int	insert(length,posn) /* Insert point located earlier writesearch */
int	length;
long	posn ;
{
struct	header	*head1, *head2 , *head3 ;
int	entry, addr1, addr2, addr3 ;
int	leaf, result ;

	cur_addr = posn ;
	reclen = length;
	head1 = (struct header *)ptr1 ;
	head2 = (struct header *)ptr2 ;
	head3 = (struct header *)ptr3 ;
	addr1 = parent(&entry) ;	/* pooff the unnecesary stack entry */
	leaf = 1 ;

/* it is necesary only for dyanamic mode */
/* sequential access 			 */

	do {

		if(readnode(addr1, ptr1) == ERROR) return (ERROR);
		
		putin(entry, head1) ;
		(head1->n)++ ;

		if(head1->n < currindex.pmaxkeys) {
			result = output(addr1 , ptr1) ;
			if (unlocknode(addr1) == ERROR) return (ERROR) ;
			return (result);
			}

		if (locknode(0,WRLOCK) == ERROR) return (ERROR) ;
		buildtables(0) ;
		if (leaf) {
			split_leaf () ;
			leaf = 0 ;
		}
		else split_node () ;

		if((addr2 = getnode()) <= 0) reterr (CRPIXFLERR) ;	/* new record is at end of file */

/**									**/
/**	changes made by shankar to account for absence of right link	**/
/**	in the node being split						**/
/**	29/3/89								**/
/**									**/

		if (addr3 = head1->ritlink)
			if (readnode(addr3,ptr3) == ERROR) return (ERROR) ;
		head2->ritlink = addr3 ;
		head1->ritlink = addr2 ;
		head2->lftlink = addr1 ;
		if (addr3) head3->lftlink = addr2 ;
		cur_addr = addr2;
		if(output(addr2,ptr2) == ERROR)
			return (ERROR) ; /* other hAlf of the split */
		if (addr3)
			if (output(addr3,ptr3) == ERROR) return (ERROR) ;
		result = output(addr1,ptr1) ;
		if (unlocknode(addr1) == ERROR) return (ERROR) ;
		if (result == ERROR) return (ERROR) ;

		currslot.numrecords++ ;

		if (writeheader() == ERROR) return (ERROR) ;
		if (unlocknode(0) == ERROR) return (ERROR) ;
		addr1 = parent(&entry) ;

	} while (addr1 != 0) ;

	if(newroot() == ERROR) return (ERROR); /* create a new root*/
	return (NOERROR);

} /* insert() */



/****************************************************************************/


static	int	getnode()
{

	return (currslot.numrecords+1);
}


static	int	split_leaf ()
{
struct	header	*head1, *head2 ;
struct	field	*ptrs, *ptrd ;
int	splitpt, nkeys ;

	head1 = (struct header *)ptr1 ;
	head2 = (struct header *)ptr2 ;
	ptrs = (struct field *)(head1+1) ;
	ptrd = (struct field *)(head2+1) ;

	/*** These 4 lines are changed to next 3 lines by Amar on 17-NOV-89....
	nkeys = head1->n ;
	splitpt = nkeys/2 ;

	head1->n = (nkeys - splitpt) ;
	
	head2->n = splitpt ;
	***/
	nkeys    = head1->n ;
	head2->n = nkeys * spl_ratio ;
	splitpt  = head1->n = nkeys - head2->n ;

/**  copy second half of keys to second node **/

	copykeys(ptr2+offset(1), ptr1+offset(splitpt+1), head2->n) ;
	copyfields(ptrd, ptrs+splitpt, head2->n) ;

	head2 -> a0 = 0 ;
	/*** Commented out by Amar on 17-NOV-89, in part of above change...
	if ( nkeys != splitpt * 2 ) splitpt++ ;
	*****/
 	copykeys (cur_key, ptr1+offset(splitpt), 1) ;
	return ;
}


static	int	split_node () 

{
struct	header	*head1, *head2 ;
struct	field	*ptrs, *ptrd ;
int	splitpt, nkeys, temp_len ;

	head1 = (struct header *)ptr1 ;
	head2 = (struct header *)ptr2 ;
	ptrs = (struct field *)(head1+1) ;
	ptrd = (struct field *)(head2+1) ;

	/*** These 5 lines are changed to next 4 lines by Amar on 17-NOV-89....
	nkeys = head1->n ;
	splitpt = nkeys/2 ;
	head1->n =  (nkeys - splitpt) -1  ;

	if ( nkeys != splitpt * 2 ) splitpt++ ;

	head2->n =  splitpt   ;
	****/
	nkeys    = head1->n ;
	head2->n = nkeys * spl_ratio ;
	head1->n = nkeys - head2->n - 1 ;
	splitpt  = head1->n + 1 ;

/**  copy second half set of keys into second node **/

	copykeys(ptr2+offset(1), ptr1+offset(splitpt+1), head2->n) ;
	copyfields(ptrd, ptrs+splitpt, head2->n) ;

	copykeys (cur_key, ptr1+offset(splitpt), 1) ;

	ptrs += splitpt -1 ;

	head2 -> a0 = ptrs -> a ;

#ifndef	FIXED_RECLEN
	temp_len = ptrs -> datlength ;
	ptrs -> datlength = reclen ;
	reclen = temp_len ;
#endif
	return ;
}


/****************************************************************************/

static	int	putin(ent_no, head)
int	ent_no ; 
struct	header	*head ;
{
struct	field	*ptrs ;
int	j, nkeys ;
char	*temp ;

	ptrs = (struct field *) (head+1) ;
	temp = (char *) head ;
	nkeys = head->n ;

	for(j = nkeys ; j>ent_no ; j--) {
		copykeys (temp+offset(j+1), temp+offset(j), 1) ;
		copyfields (ptrs+j, ptrs+(j-1), 1) ;
	}

	copykeys (temp+offset(ent_no+1), cur_key, 1) ;
	ptrs += ent_no ;
#ifndef	FIXED_RECLEN
	ptrs->datlength = reclen ; 
#endif
	ptrs->a = cur_addr ;
	return ;

} /* putin */


/****************************************************************************/

static	int	newroot() /* create and output a new root node */
{
struct	header	*head;
struct	field	*fldptr;

	head = (struct header *) ptr1;
	head->n = 1;
	head->a0 = currindex.proot;
	head->ritlink = head->lftlink = 0 ;

	fldptr = (struct field *) (head+1) ;

	copykeys((ptr1+offset(1)),cur_key,1);

	fldptr->a = cur_addr ;
#ifndef	FIXED_RECLEN
	fldptr->datlength = reclen;
#endif

	currindex.proot = -1 ;
	if (writeheader() == ERROR) return (ERROR) ;

	currindex.proot = ++currslot.numrecords ;

	if (output(currindex.proot,ptr1) == ERROR) return (ERROR) ;
	if (writeheader() == ERROR) return (ERROR) ;
	return (NOERROR) ;
}



/****************************************************************************/


static	int	initialise () /* Initia lise a new file pointed at by fd */
{
struct	header	*head;

	head = (struct header *)ptr1;
	head->n = head->a0 = 0;
	head->ritlink = head->lftlink = 0 ;

	for(currkeyno = 0;currkeyno<currslot.altkeys;currkeyno++)
		if ( output((currkeyno+1),ptr1) ) return (ERROR) ;

	return (NOERROR) ; 

}/* initialise () */



/****************************************************************************/

static	int	readnode(p,ptr1)
int	p;
char	*ptr1;
{

	if(p < 0) reterr(INVNODNO);

	if(seek(p) == ERROR)
		reterr(NODSKERR);

	if(read(currslot.fd2,ptr1,currindex.pnodesize)<currindex.pnodesize) 
		reterr(IFLRDERR);

	return (NOERROR);
}


/****************************************************************************/

static	int	output(p,ptr)
int	p; /* record number*/
char	*ptr; /* pointer to the record */
{

	if(p < 0) reterr(INVNODNO);

	if(seek(p) == ERROR)
		reterr(NODSKERR);

	if(write(currslot.fd2,ptr,currindex.pnodesize)<currindex.pnodesize )
		reterr(IFLWRERR);

	return (NOERROR);
}


/****************************************************************************/


static	int	stackinit() /* initialise the stack */
{

	stacktop[slotnum] = 0;
}


/****************************************************************************/

static	int	push(q,i) /* stack the q */
int	i;
int	q; 
{
int	sp;

	sp = stacktop[slotnum];

	if(sp<0 ||sp >= STACKSZ) reterr(STKFLERR);

	posinnode[slotnum][sp] = i;
	stack[slotnum][sp] = q;
	stacktop[slotnum]++;
	return (NOERROR);
}


/****************************************************************************/

static	int	parent(x)
int	*x; 
{ 

/* Find parent of current node ..simply unstack */

int	sp;

	sp = stacktop[slotnum]-1;

	if(sp<0 ||sp >STACKSZ) reterr(STKFLERR);

	stacktop[slotnum]--;
	*x = posinnode[slotnum][sp];
	return (stack[slotnum][sp]);
}


/****************************************************************************/

static	int	putdata(buffer,length)
char	*buffer;
int	length;
{
int	result ;

	if ((result = datseek(currslot.nxtposn, 0,0))<0) return (result) ;

	last_posn = currslot.nxtposn ;

	if(write(currslot.fd1,buffer,length)<length)
		reterr(DFLWRERR);

	return (length);
}


/************************************************************************/
/*									*/
/* ISREADS() :								*/
/*									*/
/************************************************************************/

int	isreads(fp,buffer, lock)
int	fp ;
int	lock ;
char	*buffer;
{

	if(fp<0 ||fp >= MAXFILES) 
	reterr(INVFDERR); /* INVALID FILE POintER */

	slotnum = fp;

	if(isfree(slotnum) == TRUE) 
		reterr(FLNOPNERR); /* FILE NOT OPENED */

	if (currslot.iomode == W) reterr (IMPMODERR) ;
	return (readseq(buffer, lock));
}


/****************************************************************************/

static	int	readseq(buffer, lock)
char	*buffer ;
int	lock ;
{
struct	header	*head ;
struct	field	*fldptr ;
int	p, i, rit , result;

	head = (struct header *)ptr1 ;
	fldptr = (struct field *)(head+1) ;
	currkeyno = currslot.activekey ;
	p = parent(&i) ;
	
	if (readnode(p,ptr1) == ERROR) return (ERROR) ;


	do {
	 	i++ ;
 		if (i > head->n)
 		do {
 		 if (head->ritlink == 0) {
 		 	push(p,--i) ;
 			if (unlocknode(p) == ERROR) return (ERROR) ;
 			return (EFL) ;
 			}
 		 rit = head->ritlink ;
 		 if (locknode(head->ritlink,RDLOCK) == ERROR) return (ERROR) ;
 		 if (readnode(head->ritlink,ptr1) == ERROR) return (ERROR) ;
 		 if (unlocknode(p) == ERROR) return (ERROR) ;
 		 p = rit ;
 		 i = 1 ;
 		} while (head->n == 0) ;

 		searchval.sposition = (fldptr+(i-1))->a ;
#ifdef	FIXED_RECLEN
		/* +1 is added by amar on 29-sep-89 */
 		searchval.slength = currslot.reclength + 1;
#else
 		searchval.slength = (fldptr+(i-1))->datlength ;
#endif
	} while (isstatus() == SET_DEL ) ;

	if((result = getdata(buffer, lock)) < 0 ) return (result) ;
		push(p,i) ; 
	return (searchval.slength - STATUS_LEN) ;
}

/************************************************************************/
/*									*/
/* ISDONE() :								*/
/*									*/
/************************************************************************/
/* 	Since sequential reads always result in some nodes left locked ..
	isdone indicates termination of sequential reads so that the locked
	nodes could be unlocked .. */

int	isdone (fp)
int	fp ;
{

int	p, i ;

	if ( fp<0 || fp>MAXFILES) 
	reterr(INVFDERR); /* INVALID FILE POintER */

	slotnum = fp ;

	if (isfree(slotnum) == TRUE ) 
		reterr(FLNOPNERR); /* FILE NOT OPENED */

	p = parent(&i) ;
	if (unlocknode(p) == ERROR) return (ERROR) ;
	/** push it back for later locking by isreads **/
	push(p, i) ;
	return (NOERROR);

}


/****************************************************************************/


static	int	findleast(a)
int	a;
{
struct	header	*head;
int	child ;

	head = (struct header*)ptr1;

	do {
		push(a,0);
		if (head->a0 != 0) {
			locknode(head->a0,RDLOCK) ;
			child = head->a0 ;
			if (readnode(head->a0,ptr1) == ERROR) return (ERROR) ;
			if (unlocknode(a) == ERROR) return (ERROR) ;
			a = child ;
		}
		else a = 0 ;

	} while(a != 0);

	return (NOERROR);
}

int	findlast (a)
int	a ;
{
struct	header	*head ;
struct	field	*fldptr ;
int	p, child ;

	head = (struct header*) ptr1 ;

	do {
		child = head -> n ;
		push (a, child) ;
		if (head -> a0) {
			if (child == 0) p = head -> a0 ;
			else {
				fldptr = (struct field *) (head + 1) + (child - 1) ;
				p = fldptr -> a ;
			}
		} else {
			p = 0 ;
		}

		if (p) {
			locknode(p, RDLOCK) ;
			if (readnode(p, ptr1) == ERROR) return (ERROR) ;
		}
		if (unlocknode(a) == ERROR) return (ERROR) ;
		a = p ;
	} while (a != 0) ;

	return (NOERROR) ;
}

/************************************************************************/
/*									*/
/* ISSTART() :								*/
/*									*/
/************************************************************************/

int	isstart(fp, bufkey, thekey, mode) /* start for dyanamic access */
int	fp; /* filenumber */
int	thekey;
int	mode ;
char	*bufkey;
{
int	result ;
char	*buffer , n_keybuff[MAX_KEY_LEN] ;
int	p ;

	just_started = 0 ;	/* Inserted by AMAR on 26-MAR-90 */

	if(fp<0 || fp >= MAXFILES) reterr(INVFDERR); /* INVALID FILE POintER */
	if(isfree(fp) == TRUE) reterr(FLNOPNERR); 	/* FILE NOT OPENED 	 */
	slotnum = fp;
	if(currslot.altkeys <= thekey) reterr(NOKEYERR); 
	currslot.activekey = currkeyno = thekey;

	if ((mode == ISFIRST) || (mode == ISLAST)) {

		for(;;) {
			p = currindex.proot;
			if (locknode(p,RDLOCK) == ERROR) return (ERROR) ;
			if (readnode(p,ptr1) == ERROR) return (ERROR) ;
			if (chkroot(p)) break ;
			else { 
				if (unlocknode(p) == ERROR) return (ERROR) ;
				if (buildtables(0) == ERROR) return (ERROR) ;
			}
		}
		stackinit() ;
		if (mode == ISFIRST) {
			if (findleast(currindex.proot) == ERROR) return (ERROR) ;
		} else {
			if (findlast (currindex.proot) == ERROR) return (ERROR) ;
		}
		return (NOERROR) ;

	}

	if (bufkey == NULL) reterr (NULRECPTR) 

	assemblekey(bufkey,keybuffer) ;
	if(msearch(keybuffer) == ERROR) return (ERROR);

	if(searchval.j == 0) return (UNDEF);

#ifdef	POS_POS		/* PMS users use it for checking if key exists .. so
			 advance position to next undeleted record .. */

	if ( isstatus() == SET_DEL ) {
		if ( currkeyno == 0 ) return (UNDEF) ;
#ifdef	FIXED_RECLEN
		if ((buffer = malloc((unsigned)currslot.reclength)) 
					== NULL)reterr(MEMORYERR)
#else
		if ((buffer = malloc((unsigned)2048)) == NULL)reterr(MEMORYERR)
#endif
		result = readseq(buffer, 0) ;
		if(result == EFL ) result = UNDEF ;
		if ( result < 0 ){
			free(buffer) ;
			return (result) ;
			}
		assemblekey(buffer, n_keybuff) ;
		free(buffer) ;
		if ( compkeys(n_keybuff, keybuffer)  != 0 ) return (UNDEF) ;
		/* assemblekey(buffer) ; .. Why ? */
		}
/***** Up To Here ****/

#endif
	just_started = 1 ;

	return (NOERROR);
}


/****************************************************************************/

static	int	copykeys(ptr1,ptr2,n)
char	*ptr1, *ptr2;
int	n;
{
/***the entire key is treated as a character array --- jdp ****/
int	i ;

	for ( i = 0 ; i < ( currindex.pkeylength )* n ; i++ ) 
		*ptr1++ = *ptr2++;
}


static	int	compkeys(ptr1,ptr2) /* compare keys pointed to by ptr1 & ptr2 */
char	*ptr1, *ptr2;
{
int	i, j, k ;
short	spart1, spart2 ;
long	lpart1, lpart2 ;
float	fpart1, fpart2 ;
double	dpart1, dpart2 ;
char	*lptr1, *lptr2, *fptr1, *fptr2 , *dptr1, *dptr2, *sptr1, *sptr2;
int	*ptrray ;
int	low, high ;

	ptrray = currindex.partsarray ;
	
	for(i = 0 ; i<currindex.pkeyparts ; i++, ptrray += 4) {
		if ( *(ptrray+3) == ASCND ) low = -1 ; else low = 1 ;
		high = low * -1 ;
		if (*ptrray == LONG	|| *ptrray == DATE) {
			lptr1 = (char *)&lpart1 ; lptr2 = (char *)&lpart2 ; 
			for(j = 0 ; j<*(ptrray+1) ; j++) {
				for(k = 0 ; k<sizeof(long) ; k++) {
					*(lptr1+k) = *ptr1++ ;
					*(lptr2+k) = *ptr2++ ;
				}
				if (*ptrray == DATE) {
					lpart1 = conv_date(lpart1, DT_TYPE);
					lpart2 = conv_date(lpart2, DT_TYPE);
				}
				if (lpart1<lpart2) return (low) ;
				else if (lpart1>lpart2) return (high) ;
			}
		}
		else
		if (*ptrray == SHORT) {
			sptr1 = (char *)&spart1 ; sptr2 = (char *)&spart2 ;
			for(j = 0 ; j<*(ptrray+1) ; j++) {
				for(k = 0 ; k<sizeof(short); k++) {
					*(sptr1 +k) = *ptr1++ ;
					*(sptr2 +k) = *ptr2++ ;
				}
				if (spart1<spart2) return (low) ;
				else if (spart1>spart2) return (high) ;
			}
		}
		else 
		if(*ptrray == FLOAT) {
			fptr1 = (char *)&fpart1 ; fptr2 = (char *)&fpart2 ;
			for(j = 0 ; j<*(ptrray+1) ; j++) {
				for(k = 0 ; k<sizeof(float) ; k++) {
					*(fptr1+k) = *ptr1++ ;
					*(fptr2+k) = *ptr2++ ;
				}
				if (fpart1<fpart2) return (low) ;
				else if (fpart1>fpart2) return (high) ;
			}
		}
		else 	
		if(*ptrray == DOUBLE) {
			dptr1 = (char *)&dpart1 ; dptr2 = (char *)&dpart2 ;
			for(j = 0 ; j<*(ptrray+1) ; j++) {
				for(k = 0 ; k<sizeof(double) ; k++) {
					*(dptr1+k) = *ptr1++ ;
					*(dptr2+k) = *ptr2++ ;
				}
				if (dpart1<dpart2) return (low) ;
				else if (dpart1>dpart2) return (high) ;
			}
		}
		else 	
/*
*	For character type comparison only upto NULL added on 1-1-89.. kavi
*/
		if ( *ptrray == CHAR)
			for(j = 0 ; j<*(ptrray+1) ; j++) {
				if ( *ptr1 == '\0' && *ptr2 == '\0'){
					ptr1 += *(ptrray+1) - j ;
					ptr2 += *(ptrray+1) - j ;
					 break ;
					}
				if (*ptr1<*ptr2) return (low) ;
				else if (*ptr1>*ptr2) return (high) ;
				ptr1++ ; ptr2++ ;
			}
		else
			printf(" Illegal Type .. SEVERE ERROR..\n");
		}
	return (0) ;
}


static	int	copyfields(ptrd,ptrs,n)
struct	field *ptrd,*ptrs;
int	n;
{
int	i;

	for(i = 1 ; i <= n ; i++, ptrd++, ptrs++) {
#ifndef	FIXED_RECLEN
		ptrd->datlength = ptrs->datlength;
#endif
		ptrd->a = ptrs->a;
	}
}


/****************************************************************************/

static	int	msearch(x) /* search for the node with key = x */
char	*x ;
{
struct	header	*head;
struct	field	*fldptr, *ptr;
int	p, q ; 		/* current record */
int	a0, numkeys ; 	/* number of keys in current node */
short	low, high, mid ;

	stackinit();
	for(;;) {
		p = currindex.proot;
		if (locknode(p,RDLOCK) == ERROR) return (ERROR) ;
		if (readnode(p,ptr1) == ERROR) return (ERROR) ;
		if (chkroot(p)) break ;
		else {
		 if (buildtables(0) == ERROR) return (ERROR) ;
			if (unlocknode(p) == ERROR) return (ERROR) ;
		}
	}
	push(0,0) ;
	copykeys(cur_key,x,1);
	head = (struct header *) ptr1 ;
	do {
		fldptr = (struct field *)(head+1) ;
		a0 = head->a0 ;
/**** Old style linear search ..
		for(i = 1;i <= numkeys;i++,fldptr++) {
			if(compkeys((ptr1+offset(i)),k) >= 0) break;
		}
*****/

/* New binary search .. */
		low = 1 ; high = head->n ;
		while(low <= high) {
			mid = (low+high)/2 ;
			if (compkeys((ptr1+offset(mid)),cur_key) >= 0)
				high = mid-1 ;
			else low = mid+1 ;
		}
		push(p,high);/* Later we may need parent of current record..on insert */
		q = p ;
		if (a0 != 0) {
			if(high == 0) p = a0 ;
			else {
				fldptr += high-1 ;
				p = fldptr->a ;
			}
			if (locknode(p,RDLOCK) == ERROR) return (ERROR) ;
			if (readnode(p,ptr1) == ERROR) return (ERROR) ;
			if (unlocknode(q) == ERROR) return (ERROR) ;
		}
	} 
	while(a0 != 0) ;
	if (low>head->n) {
		searchval.j = 0 ; 
		return (UNDEF) ; 
	}
	else if (compkeys((ptr1+offset(low)),cur_key) == 0) {
		fldptr += high ;
		searchval.j = 1 ;
		searchval.sposition = (fldptr)->a ;
#ifdef	FIXED_RECLEN
		/* +1 is added by amar on 29-sep-89 */
		searchval.slength = currslot.reclength + 1 ;
#else
		searchval.slength = (fldptr)->datlength ;
#endif
		searchval.p = q ;
		searchval.i = low ;
		return (DUPE) ;		/* Let caller decide if it is deleted */
	}
	else {
		searchval.j = 0 ;
		return (UNDEF) ;
	}
} /****msearch()****/



/************************************************************************/
/*									*/
/* ISDELETE() :								*/
/*									*/
/* Set the First Byte of Data record to '0'.				*/
/* In iswrite it was set to '1'						*/
/*									*/
/************************************************************************/

int	isdelete(fp, buffer) /*delete on main key only-----jdp*/
int	fp;
char	*buffer ;
{
int	p, i, result ;
int	retcode, addr ;
char	st_byte[2] ;
 
	if(fp<0 || fp >= MAXFILES) 
		reterr(INVFDERR); 				/* INVAL FILE POintER */
	if(isfree(fp) == TRUE) reterr(FLNOPNERR); 	/* FILE NOT NED */
	if (currslot.iomode == R) reterr (IMPMODERR) ;
	slotnum = fp;

	currslot.activekey = currkeyno = 0 ;
	if(currslot.iomode == R) reterr(IMPMODERR); 	/* NOT OPENED ON PROPER MODE*/
	if (locknode(0,WRLOCK) == ERROR) return (ERROR) ;
	if (buildtables(0) == ERROR) return (ERROR) ;
	unlocknode(0) ;

	 assemblekey(buffer,keybuffer);
	 result = writesearch(keybuffer) ;	/* So as to get ACTIVE Record */
	 stackunlock() ;			/* has to be done explicitely */
	 if ( result == ERROR ) return (result) ;
	 else
		 if(searchval.j == 0) return (UNDEF) ; 

	 st_byte[0] = SET_DEL ;
	 return ( rewdat(st_byte, STATUS_LEN) ) ;
}

/****************************************************************************/


int	isreadp(fp,buffer, lock)
int	fp ;
int	lock ;
char	*buffer;
{

	if(fp<0 ||fp >= MAXFILES) 
	reterr(INVFDERR); /* INVALID FILE POintER */
	slotnum = fp;
	if(isfree(slotnum) == TRUE) 
		reterr(FLNOPNERR); /* FILE NOT OPENED */
	if (currslot.iomode == W) reterr (IMPMODERR) ;
	return (readprv(buffer, lock));
}


/****************************************************************************/

static	int	readprv(buffer, lock)
char	*buffer ;
int	lock ;
{
struct	header	*head ;
struct	field	*fldptr ;
int	p, i ,result;
int	left, leftok ;

	head = (struct header *)ptr1 ;
	fldptr = (struct field *)(head+1) ;
	p = parent(&i) ;
	if (readnode(p,ptr1) == ERROR) return (ERROR) ;


	if (just_started) {
		 just_started = 0 ;
		 if (searchval.j == 1 ) i++ ;
		}

	do {
	 if (i == 0) {
		if (head->lftlink == 0) { 
			push(p,i) ; 
 			if (unlocknode(p) == ERROR) return (ERROR) ;
			return (EFL) ; 
		}
		do {
			left = head->lftlink ;
			if (locknode(head->lftlink,RDLOCK) == ERROR) return (ERROR) ;
			if (readnode(head->lftlink,ptr1) == ERROR) return (ERROR) ;
			if (head->ritlink != p) {
				leftok = 0 ;
				if (unlocknode(left) == ERROR) return (ERROR) ;
				if (readnode(p,ptr1) == ERROR) return (ERROR) ;
			}
			else {
				if (unlocknode(p) == ERROR) return (ERROR) ;
				leftok = 1 ;
				p = left ;
				i = head->n  ;
			}
		} 
		while(!leftok) ;
	 } /* if i == 0 */
	searchval.sposition = (fldptr+(i-1))->a ;
#ifdef	FIXED_RECLEN
	/* +1 is added by amar on 29-sep-89 */
	searchval.slength = currslot.reclength + 1 ; 
#else
	searchval.slength = (fldptr+(i-1))->datlength ;
#endif
	i-- ;
	} while ( isstatus() == SET_DEL ) ;

	if((result = getdata(buffer, lock))<0) return (result);
		push(p,i) ; 
	return (searchval.slength - STATUS_LEN) ;
}


/****************************************************************************/

static	int	chkroot(p)
int	p ; 
{
struct	header	*head ;

	head = (struct header *)ptr1 ;
	if (head->ritlink == 0 && head->lftlink == 0) return (1) ;
	else return (0) ;
}


static	int	locknode(p,mode)
int	p , mode; 
{
long	ret_pos ;
int	wait ;

	ret_pos = seek(p) ;
	wait = 1 ;		/* Wait for other process to release lock */
	if ( e_lock( currslot.fd2, mode ,wait,ret_pos ,MAXSZ) < 0)reterr(FLOCKERR)	
	return (NOERROR) ;
	
}

/****								**/
/*	If key is found and its delete flag is set return as	**/
/*	record NOT found					**/
/****								**/

static	int	writesearch(x) /* search for the node with key = x */
char	*x ;
{
struct	header	*head;
struct	field	*fldptr, *ptr;
int	p, q ; /* current record */
int	a0, numkeys ; 	/* number of keys in current node */
int	prev ;		/* previously locked node*/
short	high,low,mid ;

	stackinit();
	for(;;) {
		p = currindex.proot;
		if (locknode(p,WRLOCK) == ERROR) return (ERROR) ;
		if (readnode(p,ptr1) == ERROR) return (ERROR) ;
		if (chkroot(p)) break ;
		else { 
			if (unlocknode(p) == ERROR) return (ERROR) ;
			if (buildtables(0) == ERROR) return (ERROR) ;
		}
	}

	prev = p ;
	push(0,0) ;
	copykeys(cur_key,x,1);
	head = (struct header *) ptr1 ;
	do {
		fldptr = (struct field *)(head+1) ;
		a0 = head->a0;
/****
		for(i = 1;i <= numkeys;i++,fldptr++) {
			if(compkeys((ptr1+offset(i)),k) >= 0) break;
		}
****/
		low = 1 ; high = head->n ;
		while(low <= high) {
			mid = (low+high)/2 ;
			if(compkeys((ptr1+offset(mid)),cur_key) >= 0)
				high = mid-1 ;
			else low = mid+1 ;
		}
		push(p,high);/* Later we may need parent of current record..on insert */
		q = p ;
		if (a0 != 0) {
			if(high == 0) p = a0;
			else {
				fldptr += high-1 ;
				p = fldptr->a ;
			}
			if (readnode(p,ptr1) == ERROR) return (ERROR) ;
			if (head->n < currindex.pmaxkeys) {
				if (locknode(p,WRLOCK) == ERROR) return (ERROR) ;
				if (unlocknode(prev) == ERROR) return (ERROR) ;
				prev = p ;
			}
		}
	} 
	while(a0 != 0) ;
	if (locknode(q,WRLOCK) == ERROR) return (ERROR) ;
	if (low>head->n) {
		searchval.j = 0 ; 
		return (UNDEF) ; 
	}
	else if (compkeys((ptr1+offset(low)),cur_key) == 0) {
		fldptr += high ;
		searchval.j = 1 ;
		searchval.sposition = (fldptr)->a ;
#ifdef	FIXED_RECLEN
		/* +1 is added by amar on 29-sep-89 */
		searchval.slength = currslot.reclength + 1 ;
#else
		searchval.slength = (fldptr)->datlength ;
#endif
		searchval.p = q ;
		searchval.i = low ;
		if ( currkeyno  != 0 ) return (DUPE) ;
/*
*	Based on the fact that only one active key may exist in the Main
*	Index, and that key got to be at the top , we just check for status 
*	of the first record .. kavi.
*/
		if ( isstatus() == SET_DEL ) {
			searchval.j = 0 ;
			return (UNDEF) ;
			}
		else	return (DUPE) ;	/* While Modifying isnew.c ..10-Aug-88*/
	}
	else {
		searchval.j = 0 ;
		return (UNDEF) ;
	}
} /****writesearch()****/



/****************************************************************************/

static	int	unlocknode(p)
int	p ; 

{
long	ret_pos ;	
int	wait ;

	if ( (ret_pos = seek(p)) == ERROR ) return (ERROR) ;
	wait = 0 ;	/* No wait required for Unlock */
	if (e_lock( currslot.fd2,UNLOCK,wait, ret_pos, MAXSZ) < 0 ) reterr(FLOCKERR)	

	return (NOERROR) ;

}

/*
* Release all lockes from the files (INdex and Data )
*/

int	isrelease(fp) 
int	fp ;
{
int	wait ;

	if(fp<0 ||fp >= MAXFILES) 
	reterr(INVFDERR); /* INVALID FILE POintER */
	slotnum = fp;
	if(isfree(slotnum) == TRUE) 
		reterr(FLNOPNERR); /* FILE NOT OPENED */
	if ( lseek(currslot.fd1, 0L, 0) < 0 ) return (ERROR);
	wait = 0 ;
	if (e_lock( currslot.fd1, UNLOCK, wait, 0L ,0) < 0 ) reterr(FLOCKERR)
	return (NOERROR) ;
 }

 /*------- routine to convert date types ---------*/ 
 
static	long	conv_date ( date, type ) 
long	date ; 
int	type ;	/* date type */
{ 
long	mm,dd,yy ; 
 
	switch (type)  { 
        case YYMMDD :
        case YYYYMMDD :
		return(date) ;
	case MMDDYY :
		yy = date % 100 ;
		date = date / 100 ;
		dd = date % 100 ;
		mm = date / 100 ;
		break ;
	case DDMMYY :
		yy = date % 100 ;
		date = date / 100 ;
		mm = date % 100 ;
		dd = date / 100 ;
		break ;
        case MMDDYYYY :
		yy = date % 10000 ;
		date = date / 10000 ;
		dd = date % 100 ;
		mm = date / 100 ;
		break ; 
        case DDMMYYYY :
		yy = date % 10000 ;
		date = date / 10000 ;
		mm = date % 100 ;
		dd = date / 100 ;
		break ; 
	default :
		return(date) ;
        } 

        return((yy * 10000L + mm * 100 + dd)) ;
} 
 


#include "cr_indx.c"

