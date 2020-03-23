

/******
	Section Definition record ..
	This record is used by report programmes to map the section code
	to section description ..

	See Also : section.def in formats. 
*******/

typedef	struct	{
	int	section ;
	char	name[16];
	}
	Sect_Rec ;

#ifdef ENGLISH
static	Sect_Rec	sect_rec[5] = {	{0, "INVALID"},
					{1, "ASSET"},
					{2, "LIABILITY"},
					{3, "EXPENSES"},
					{4, "INCOME"} } ; 
#else
static	Sect_Rec	sect_rec[5] = {	{0, "INVALIDE"},
					{1, "ACTIFS"},
					{2, "PASSIFS"},
					{3, "DEPENSES"},
					{4, "REVENUS"} } ; 
#endif
