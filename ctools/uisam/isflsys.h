#define  reterr(x)      {iserror=x; return(ERROR);}  /* funny null stmt */
	
struct  indxheader {
	int     pnumrecs ;
	long    pnxtposn ;
	int     pnextnode ;
	long    pfreedat ;
	int     pfreenodes;
#ifdef	FIXED_RECLEN
	int	preclength;		/* Added 23-Jun-89 .. kavi */
#endif
	int paltkeys ;
};

struct  keydat {
	int     pkeylength;
	long	proot;
	int	pmaxkeys;
	long	pnodesize;
	int	pnumkeys;
	int	pkeyparts ;
	int	partsarray ;
};
/*** Trying something
struct  keydat {
	int     pkeylength,
		proot,
		pmaxkeys,
		pnodesize,
		pnumkeys,
		pkeyparts ,
		*partsarray ;
};
***/
struct  indxinfo {
	int     numrecords ;
	long    nxtposn ;
#ifdef	FIXED_RECLEN
	int	reclength ;		/* Added new */
#endif
	int     nextnode,
	        freenodes ;
	long    freedat ;
	int     iomode,
	        accessmode,
		altkeys,
		activekey,
		fd1,
		fd2;
	char    flnam[FLNM_LEN];              /* file name */
	struct  keydat  *ktable ;
};
 


struct  header {
        unsigned int n;
        unsigned int a0;
        unsigned int ritlink ;
	unsigned int lftlink ;
 	} ;



struct  field  {
#ifndef	FIXED_RECLEN
	int		datlength ;
#endif 
        long		 a ;
	 } ;



#define	HEADSZ		sizeof(struct header)
#define FIELDSZ        	sizeof(struct  field)
#define FIXEDPART  	(HEADSZ+currindex.pmaxkeys*FIELDSZ)
#define BUFFSZ         	(NODESIZE+FIELDSZ)



 struct  searchret {
                int     p;
                int     i;
		int	slength ;
                long    sposition ;
                int     j;
} ;


