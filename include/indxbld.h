/*-----------------------------------------------------------------------
Source Name: indxbld.h
System     : General Utility.
Created  On: 5th July 89.

DESCRIPTION:
	Contains temporay index building related definitions.

MODIFICATIONS:        

Programmer     YY/MM/DD       Description of modification
~~~~~~~~~~     ~~~~~~~~       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

------------------------------------------------------------------------*/

#define	INDX_SUFFIX	".KEY"		/* Appened to the given Index file,
					   to creat Information file */

#define	IXSIZE		10		/* Maiximum 10 parts & 10 constraints */

/*
*	Temporay Index file Information, such as source file name, key fields,
*	date of creation and constraint fields information.
*/

typedef	union	{

	short	c_short ;		/* Short Value */
	long	c_long	;		/* Long Value */
	float	c_float ;		/* Float Value */
	double	c_double ;		/* Double Value */
	char	c_char[21] ;		/* Character Value */

} C_val ;				/* Constraint Value */


typedef	struct	{

	short	c_fld_no ;		/* Constraint Field# */
	C_val	c_minimum ;		/* Contraint Minimum */
	C_val	c_maximum ;		/* Contraint Maximum */

} C_fld ;				/* Constraints */


typedef	struct	{

	int	t_source ;		/* Source File DBH Number */
	long	t_date ;		/* Date Of Last index Creation */
	short	t_keys[IXSIZE] ;	/* Index keys parts field numbers */
	char	t_order[IXSIZE] ;	/* Order of the part. A - ASCND,
					   D - DESCND */
	C_fld	t_cons[IXSIZE] ;	/* Constraints */

} Ix_info ;				/* Index Information */


/*--------------------E-N-D---O-F---F-I-L-E----------------------------*/

