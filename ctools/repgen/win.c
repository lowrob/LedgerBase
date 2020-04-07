#include <stdio.h> 
#include <cfomtm.h>
#include <cfomtcr.h> 
extern struct tcrblk tb ; 
 
 
window(row,col,nch,nlines,text1, put_blanks ) 
int row,col,nch,nlines ; 
int 	put_blanks ;		/* If The screen to be blanked before */
char *text1 ; 
	        /*Displays window margin in reverse video */ 
{
	int n, m ;  
	char	*text;

	text = text1;
        if (row<1 || row>telm(lines) || col<1 || col>telm(cols)) {
		fomer("Window Row/Col Coordinates not Proper... ");
                return(-1);        /* row/col out of range */ 
	}
        if ((row+nlines)<1 || (row+nlines)>telm(lines)){
		fomer("Too Large a Window");
		return(-1) ; 
	}
        if ((col+nch)<1 || (col+nch)>telm(cols)){
		fomer("Too Wide a Window");
		return(-1) ; 
	}
	if (put_blanks == 1) {
        	if (telm(ttyp)== 0) { 
			draw_0_ttyp(row,col,nch);
			draw_0_ttyp((row+1),col,2);
			draw_0_ttyp((row+1),(col+nch-2),2);
		} else {
			draw_1_ttyp(row,col,nch);
			draw_1_ttyp((row+1),col,2);
			draw_1_ttyp((row+1),(col+nch-2),2);
		}
        	poscur(row+1,col+3) ; 	/* Clearing First Line */
		for(n=0 ; n < nch-5 ; n++)putchar(' ');
	}
        for(n=2 ; n<nlines ; n++) { 
		if(put_blanks == 1){
			if(telm(ttyp) == 0){
				draw_0_ttyp(row+n,col,2);
				poscur(row+n,col+3);
				putchar(' ') ; 
        		} else
				draw_1_ttyp(row+n,col,2);
		}
                poscur(row+n,col+3);
		for(m = 0 ;*text && m < nch-6 ; m++, text++ ){
                	if(*text == '\n')break;
			putchar(*text);
                }
		for( ; *text ; text++)
			if(*text == '\n'){
				text++;
				break;
			}
	        for (; m < nch-6 ; m++)putchar(' ');

		if(put_blanks == 1){
        		if(telm(ttyp) == 0)poscur(row+n,col+nch-3) ; 
			putchar(' ') ; 
			if(telm(ttyp) == 0)
				draw_0_ttyp(row+n,(col+nch-2),2);
        		else
				draw_1_ttyp(row+n,(col+nch-2),2);
		}
	}
	if (put_blanks == 1)
        	if (telm(ttyp)== 0)
			draw_0_ttyp(row+nlines,col,nch);
		else
			draw_1_ttyp(row+nlines,col,nch);

	return(0);
} 

draw_0_ttyp(row,col,nch)
int	row,col,nch;
{
	int	n;
	char	c;

        c= telm(vabase) ; c |= telm(rv) ; 
       	poscur(row,col+nch); puts(cas(efa)) ; 
      	poscur(row,col); 
       	puts(cas(sfa)) ; putchar(c) ;
	for(n = 2; n <= nch ; n++)
		putchar(' ') ; 
}

draw_1_ttyp(row,col,nch)
int	row,col,nch;
{
	int n;

        poscur(row,col); 
        puts(cas(rv)) ; 
	for(n = 0 ; n < nch ; n++)
		putchar(' ');
       	poscur(row,col+nch); puts(cas(efa)) ; 
	putchar(' ');
}

