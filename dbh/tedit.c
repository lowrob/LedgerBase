/*--------------------------------------------------------------*/
/*                     tedit.c                                  */
/*--------------------------------------------------------------*/

	
#include <stdio.h>
#include <repdef.h>
#include <gen_routine.h>

#define NEG -1
#define ZERO -2
#define SUP 1
#define FLT 2
#define STR 3
#define SUPPRESS { k=j; flag=SUP;   }
#define STAR     { k=j; flag=STR;   }
#define I_FLOAT    { k=j; flag=FLT;   }
#define edited   { strcpy(dest,ma); return(0);  }
#define mchar    ma[j]=='_'||ma[j]=='*'||ma[j]=='0'||ma[j]=='$'
#define punct    ma[j]==','||ma[j]=='.'||ma[j]==' '||ma[j]=='-'||ma[j]=='/'
static  short i,j,k,flag,nflag,nerr;
static	char ma[25],no[25];

/******
tedit(src,mask,dest)
long src;
char mask[], dest[];
{
****/

/** New edit **/

int
tedit(in_val,mask,dest,typ)
char *in_val ;
char *mask, *dest ;
int  typ ;
{
	union NUMRET {
		short	vshort ;
		int 	vint ;
		long	vlong ;
		float 	vfloat ;
		double 	vdouble ;
	      }
		val ;
	char	format[10];

	i=j=k=flag=nflag=nerr=0;

	switch (typ) {
		case R_SHORT :
			val.vshort =  (short)*((short *)(in_val)) ; 
			if(val.vshort== 0) nflag= ZERO ;
			if(val.vshort<0) { 
				nflag= NEG ; val.vshort *= -1 ; }
			sprintf(no,"%d",val.vshort) ;
			break ;
		case R_INT :
			val.vint = (int)*((int *)(in_val)) ; 
			if(val.vint== 0) nflag= ZERO ;
			if(val.vint<0){ nflag= NEG ; val.vint *= -1 ; }
			sprintf(no,"%d",val.vint) ;
			break ;
		case R_LONG :
			val.vlong = (long)*((long *)(in_val)) ; 
			if(val.vlong== 0) nflag= ZERO ;
			if(val.vlong<0){nflag= NEG ; val.vlong  *= -1 ; }
			sprintf(no,"%ld",val.vlong) ;
			break ;
/*
		case R_FLOAT :
			val.vfloat = (float)*((float *)(in_val)) ; 
			if(val.vfloat== 0) nflag= ZERO ;
			if(val.vfloat<0){nflag= NEG ; val.vfloat *= -1 ; }
			form_format(mask,format, typ);
			sprintf(no,format,val.vfloat) ;
			rm_dot( no ) ;
			break ;
*/
		case R_DOUBLE :
			val.vdouble = (double)*((double *)(in_val)) ; 
			if(val.vdouble== 0) nflag= ZERO ;
			if(val.vdouble<0){nflag= NEG ; val.vdouble *= -1 ; }
			form_format(mask,format, typ);
			sprintf(no,format,val.vdouble) ;
			rm_dot( no ) ;
			break ;
	}
	i= strlen(no) ;         /* i=length of digits string */
	strcpy(ma,mask);

/**/
    


/****    
    i=itoa(src,no);
    strcpy(ma,mask);
*****/

    for(j=0;ma[j]!='\0';j++){
        if(j!=0 && ma[j]=='$')  I_FLOAT
        if(ma[j]=='0') SUPPRESS
        if(ma[j]=='*') STAR
    }       /* sets the appropriate flag */

    if(ma[j-1]=='R'){        /* CR Case */  
        j--;
        if(nflag!=NEG)
            ma[j]=ma[j-1]=' ';
        j--;
    }
    else
    if(ma[j-1]=='-'){        /* Case of -Sign */
        j--;
        if(nflag!=NEG)
            ma[j]=' ';
    }

    if(nflag!=ZERO){
        nerr=0;
        for(j--;j>=0;j--){
            if (punct) ; 
            else
                if (mchar) {
                    ma[j]=no[--i];
                    if(i<=0) goto final;
                    else ;
                }
                
        }                       /* for(j--;j>=0;j--) */
    }       /* if(nflag!=ZERO) */

    nerr=0;
    for(j--;j>=0; j--){
        if(ma[j]=='_') { ma[j]='0';  continue;  }
        if(ma[j]=='0')  {  
            ma[j]=' '; 
            suppress(k); 
            edited
        }
        if(ma[j]=='*')  {  
            asterisk(k); 
            edited
        }
        if(ma[j]=='$')  {  
            last(); 
            edited
        }
    }
final:
    if(flag==SUP)  { 
        suppress(k); 
        edited
    }
    if(flag==STR)  { 
        asterisk(k); 
        edited
    }
    if(flag==FLT)  {
        flot(k);
        edited
    }
    nerr=0;
    for(j--;j>=0;j--){
        if(ma[j]=='$')  edited
        if(punct) continue;
        if(ma[j]=='_')  { 
            ma[j]='0'; 
            continue;  
        }
        else nerr=1;
    }
    edited
}

static int
suppress(t)
int t;
{
    for(j--;j>=0;j--){
        if(j<0) return;
        if(ma[j]=='$') return;
        if(ma[j]=='.' || ma[j]==',')
            if(j>t) continue;
            else
                if(j<t) { 
                    ma[j]=' ';  
                    continue; 
                }
        if(ma[j]=='_' && j>t)  { 
            ma[j]='0'; 
            continue;  
        }
        ma[j]=' ';
    }
}

static int
asterisk(t)
int t;
{
    for(j--;j>=0;j--){
        if(j==0)   {   ma[j] = '*';  return;  }
        if(ma[j]=='$') return;
        if(ma[j]=='.'||ma[j]==',')
            if(j>t) continue;
            else
                if(j<t) {  
                    ma[j]='*'; 
                    continue; 
                }
                else
                    if(ma[j]=='_' && j>t)  {  
                        ma[j]='0';  
                        continue;  
                    }
        ma[j]='*';
    }
}

static int
flot(t)
int t;
{
    for(j--;j>=0;j--){
        if(j==0)  {    ma[j] = '$'; return;   }
        if((ma[j]=='.'||ma[j]==',') && j>t) continue;
        if(ma[j]=='_' && j>t)  { 
            ma[j]='0'; 
            continue; 
        }
        ma[j] = '$';
        last();
    }
}

static int
last()
{
    for(j--;j>=0;j--)
        ma[j]=' ';
}

/******
itoa(n,s)
long n;
char s[];
{
        int i;
        long sign;
        if((sign=n) < 0)
            n = -n;
        i=0;
        do
            {
            s[i++]=n % 10 + '0';
        }
        while((n /= 10) > 0);
            if(sign<0)
                s[i++] = '-';
        s[i]='\0';
        reverse(s);
        return(i-1);
}

reverse(s)
char s[];
{
        int c,i,j;
        for(i=0,j=strlen(s) - 1; i<j; i++,j--)
        {
            c=s[i];
            s[i]=s[j];
            s[j]=c;
        }
}

******/

static	int
form_format( msk,form, type ) 
char	*msk,*form;
int 	type ;
{
	int 	i, j,k ;

	/* Default */
	strcpy(form , "%.2");

	if ( type == R_DOUBLE )
		strcat(form, "lf") ;
	else
		strcat(form, "f") ;

	for(i=0, j = 0, k = 0; i < strlen(msk) ; i++ ) {
		if ( *(msk+i) == '.' ){
			k = 1;
			continue ;
		}
		if(k) j++ ;
	}

	if ( j < 0 || j > 9 ) return(0) ;

	/* if last char is sign */
	if(msk[i-1] == '-' && j > 0) j--;
	form[2] = j + '0' ;
	return(0);
}

static	int
rm_dot( no ) 
char	*no ;
{
	int 	i, j ;

	for(i=0, j = 0; i < strlen(no) ; i++ ) {
		if ( *(no+i) == '.' ) continue ;
		*(no+j) = *(no+i) ;
		j++ ;
	}
	*(no+j) = '\0' ;
}

