/******************************************************************************
		Sourcename    : convutil.c
		Created on    : 90-02-12
		Created  By   : K HARISH.
*******************************************************************************

About:	This file contains utilities for conversion of COBOL data files to 
	C data files.

*******************************************************************************/

#include <stdio.h>
#include <isnames.h>
#include <convtype.h>

long	conv_date();

xform( fromaddr, toaddr, fromtype, totype, befpoint, aftpoint, tolen )
char	*fromaddr, 	/* source address */
	*toaddr;	/* destination address */
int	fromtype,	/* data type of the input COBOL field */ 
	totype,		/* data type of the output C field */ 
	befpoint, 	/* digits before point in COMP field, length if CHAR */
	aftpoint,	/* digits after point in COMP field */ 
	tolen;		/* Length if destination is CHAR field */
{
	char	buf1[40];	/* to store intermediate data */
	char	buf2[40];	/* to store intermediate data */
	int	len ;		/* no. of bytes occupied by current field */
	short	i, j, addl;	/* counters and padding flag */
	unsigned char	store;	

	switch( fromtype ){
		case COMP3:	/* signed comp-3 */
			addl = 0;
			len = befpoint + aftpoint + 1;	/* 1 for Sign */
			if( len%2 == 1 ){
				addl = 1;
				len++;
			}
			i = 1;
			j = 0;
			for( j=0; j<len/2-1; j++ ){
				store = *(fromaddr+j);
				buf1[i++]=( (store>>4)+'0' );
				buf1[i++]=( (store & 0x0f)+'0' );
			}
			store = *(fromaddr+j);
			buf1[i++]=( (store>>4)+'0' );
			buf1[i]=( (store & 0x0f) );

			if( buf1[i]==0x0d )
				buf1[0] = '-';
			else
				buf1[0] = '0';
			buf1[i] = '\0';
			
			i=0; j=0;
			while( i<=befpoint+addl )
				buf2[j++] = buf1[i++];
			if( aftpoint>0 )
				buf2[j++] = '.';
			while( i<=len )
				buf2[j++] = buf1[i++];
			break;
		case CHAR:	/* Ordinary PIC clause */
			for( i=0; i<befpoint; i++ )
				buf2[i] = fromaddr[i];
			while( buf2[--i]==' ' )
				;
			buf2[++i] = '\0';
			break;
		case COMP6:
			addl = 0;
			len = befpoint + aftpoint ;
			if( len%2 == 1 ){
				addl = 1;
				len++;
			}
			i = 0;
			j = 0;
			for( j=0; j<len/2; j++ ){
				store = *(fromaddr+j);
				buf1[i++]=( (store>>4)+'0' );
				buf1[i++]=( (store & 0x0f)+'0' );
			}
			buf1[i] = '\0';
			
			i=0; j=0;
			while( i<befpoint+addl )
				buf2[j++] = buf1[i++];
			if( aftpoint>0 )
				buf2[j++] = '.';
			while( i<=len )
				buf2[j++] = buf1[i++];
			break;
		case COMP1:	/* Binary 2's complement, 2 bytes length */
				/* destination is always signed short */
			sprintf( buf2, "%d", *(short*)fromaddr );
			break;
		default:
			printf( "\n\tInvalid Input type %d\n",fromtype );
			return( -1 );
	}
	switch( totype ){
		case DOUBLE:
			sscanf( buf2, "%lf", (double *)toaddr);
			break;
		case FLOAT:
			sscanf( buf2, "%f", (float *)toaddr);
			break;
		case LONG:
			sscanf( buf2, "%ld", (long *)toaddr);
			break;
		case SHORT:
			sscanf( buf2, "%hd", (short *)toaddr);
			break;
		case CHAR:
			strncpy( toaddr, buf2, tolen );
			if(tolen > 1)
				toaddr[tolen-1] = '\0';
			break;
		case DATE:	/* source format YYMMDD */
			sscanf( buf2, "%ld", (long *)toaddr);
	
/*	test xxxxx
			*(long *)toaddr 
				= conv_date( *(long *)toaddr,2,3);
			if( *(long *)toaddr )
				*(long *)toaddr += 19000000;
*/

			break;
		case ACCT:
			strcpy( toaddr, buf2 );
			acnt_chk( toaddr );
			break;
		default:
			printf( "\n\tInvalid Output type %d\n",totype );
			return(-1);
	}
	return(0);
}

