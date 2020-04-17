struct	linkstr {		/* format of linked structure */

char	*fldna;			/* offset to field name if any else 0 */
int	fldn,			/* field number */
	fldty,			/* type of field */
	fldcla;			/* class of field */
char	*promp;			/* offset to prompt string if any else 0 */
int	prox,			/* line no of promt */
	proy,			/* column no of prompt */
	flx,			/* line no of field mask */
	fly;			/* column no of field mask */
char	*imas,			/* offset to input mask string */
	*dmas;			/* offset to display mask string */
int	drlo;			/* displacement within data record */
char 	*picst,			/* offset to COBOL picture string */
	*helpm,			/* offset to help message if any */
	*lboun,			/* offset to lower bound string if any */
	*uboun;			/* offset to upper bound string if any */
int	promc,			/* prompt colour */
	fldc,			/* field mask colour */
	dupct;			/* duplication control */
char  	*dupva;			/* offset to dup value if any */
int	latt1,			/* logical attribute set 1 */
	latt2,			/* logical attribute set 2 */
	promv,			/* prompt video attributes */
	fldv;			/* field mask video attributes */
char	machar;			/* mask character */
int	dfsiz;			/* size of the field in data record */
struct linkstr *prep;
struct linkstr *nexp;
};


