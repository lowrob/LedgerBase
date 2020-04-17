/* cfomfld3.c - Cprofom fields processor module - part 3 */

/*
* BUG: In the middle field entry, if USER ESCAPE is used, PROFOM should show
*	old field value, i.e either dup value or mask, but it is not doing so.
*	Here note, this is happening only in the middle of the field, not at
*	the begining of the field.
*
* FIX: (17-JUL-91 AMARENDRA T)
*	In the end of fomaccept() function, else part if(sr.retcode !=
*	RET_USER_ESC) condition is changed to take care of this bug.
*/

#include <stdio.h>
#include "cfomdef.h"
#include "cfomfrm.h"
#include "cfomstrc.h"
#include <ctype.h>	/* for John's use */

extern	struct	frmfld	*cf;	/* current field of the form */
extern	struct	fldinfo	*cfi;	/* internal data items of *cf */
extern	char	*fvp;		/* variable data area */
extern	char	*ourdr,		/* profom's copy of user data record */
		*userdr;	/* user data record */
extern struct stat_rec	*sp;	/* staus record */

#define FLD_BACK 2
#define FLD_FORWARD 6
#define REGEN 18
#define SNAP  16
#define CURR_BACK 21
#define DELETE 8
#define RETURN 13
#define LF 10
#define TAB 9
#define ESCAPE 27
#define DOT '.'
#define HRET '?'
#define VALID 0
#define INVALID -1
#define NORM_RET 0
#define ERR_RET 1
#define FORWARDED -3
#define BACKED -1
#define RET_IMM -2
#define SPACE ' '

static int xposn,yposn;
static int dotfound,signfound;
static char *dmaskptr;
static char *imaskptr;
static int buffptr,echopt,signposn,dotposn,type,c,fldhold,required,uesc;
static int lcase,ucase;
static char buff[100];
static char buffer[100];
static int mflength,yrlength; /* used in processdate routine */
static int maskchar;  /* The mask character for this field */

firstmask() {
       posinfld(1);  /* First posn of mask */
       dmaskptr=fvp+cf->dmask;
       imaskptr=fvp+cf->imask;
       while(*(imaskptr)=='F')imaskptr++ ;
       while (*(dmaskptr)!=maskchar && *dmaskptr!='\0'){
       		put(*dmaskptr);
           	dmaskptr++;
    		}
 } /* firstmask */

fomaccept(b,f)

/* Routine to accept input streing form terminal  */
/* Input argument b indicates whether field backspacing */
/* is permitted for this field         */

    int b;  /* indicates whether field backspace is allowed */
    int f; /* indicates whether field forward is allowed */

{
  int i,paddval,success;
  char *ptrs,*ptrd;  /* Temporary */
  int firstchar; /* says whethe currentr the chara is the first chara entered */
  /* initialise option variables */
  type=cf->fldtyp;
  uesc=cf->lattr1 & LA_UESC;
  required=cf->lattr1 & LA_REQ;
  echopt=!(cf->lattr1 & LA_NOECHO);
  lcase=cf->lattr2&LA_LCASE;
  ucase=cf->lattr2&LA_UCASE;
  if(type==TYP_YN) required=1;
  fldhold=(cf->lattr2 &LA_FH);
 maskchar=cf->maskchar;
  firstchar=1; /* Next char read is the first one */
  
  /* find position of dot and sign in input string if field is numeric */
  if(type==TYP_NUM)
              fixup(&signposn,&dotposn);
 if(type==TYP_DATE) fixdate(); /* set mflength and yrlength */

  do {
     firstmask(); /* position cursor at the first mask position */
       c=get();
   if(lcase!=0 && isupper(c)) c=tolower(c);
   if(ucase!=0 && islower(c)) c=toupper(c);

       success= INVALID;
       switch(c)  {
          case ESCAPE :  /* user escape has been pressed */
                         if(( success=uefom())!=INVALID) return(RET_IMM);
                         else showerror(1);/*"User escape not allowed "*/
                         break;

          case FLD_BACK:  /* Field backspace */
                         if(b)  {
                             return(BACKED);
                           }
                          else showerror(2);/*"field backspace not allowed "*/
                          break;

          case FLD_FORWARD: if(firstchar==1 && f==1) return(FORWARDED);
                            else showerror(3);/*"Forward not allowed "*/
                            break;
           case  HRET :  /* Help return */
                        success=hretp();
                        if(success==VALID)return(RET_IMM );
                        else break;


           case TAB:
           case LF:
           case RETURN :if((cf->dupctrl != DUP_NONE) && (cf->dupval>0)) {
                                      /* if a duplicate value is available */
                            filldat(1);/* fill user data area from dup */
                            sp->retcode=RET_NO_ERROR;
                            sp->fillcode=FIL_DUP;
                            edit(); /* edit the duplicn data */
                             if(cf->lattr2&LA_SHODUP)
                            putdata(); /* Display edited data  */
                            return(NORM_RET);
                         }

                        else if(!required) { 
                                sp->fillcode=FIL_OMITTED;
                                 sp->retcode=RET_NO_ERROR;
                              if(cfi->type==FITYP_NUM) paddval=COB_ZEROE;
                              else paddval=COB_SPACE;
/**
	printf ("pointer in fomaccept - %ld\n", (long) (userdr + cf->drloc)) ;
**/
                                 for(ptrd=userdr+cf->drloc,i=cf->dfsize;i>0;*ptrd++=paddval,i--);
                                 /* fill usrdr with blanks or zeros */
                                 edit();
                                 putdata();
                                 return(NORM_RET);
                         }
                        else showerror(4);/*"It is a required field "*/
                        break;

            case REGEN: redraw();
                        break;

            case SNAP: snapscreen();
                       break;
            default :
                           putmask();
                           firstmask();/*position cursor at first mask */
                   if(type==TYP_NUM ) /* valid numeric ? */
                               success=procnumeric();
                      else if(type==TYP_STRING )
                               success=procalpha();
                       else if(type==TYP_DATE )
                                success=procdate();
    
                      else if(type==TYP_YN) 
                              success=procbool();
                       else showerror(5);/*"Illegal character "*/

                       break;

              }

  if(success==VALID) { 
         filldat(0);
         if(!inbounds()) {
                showerror(6);/*"Input out of bounds"*/
                 success = INVALID;
            }
    }
   if (success==INVALID) 
       if((cf->dupctrl!=DUP_NONE) &&(cf->dupval !=0) &&(cf->lattr2&LA_SHODUP)) {
              filldat(1);
              edit();
               putdata() ;/* display edited data */
          }
  
      else putmask();

    firstchar=0; /* To handle fld forward */
   } while(success== INVALID);

 if(sp->retcode!=RET_USER_ESC) {
   filldat(0) ;/* FIll data in user record */
   edit();
   if(echopt) putdata(); /* Display edited data*/
   /* if it is copy duplicn,fill up duplication area */
   if(cf->dupctrl==DUP_COPY && cf->dupval!=0) {
     filldat(0);
     ptrs=userdr+cf->drloc;
     ptrd=fvp+cf->dupval; /* destination */
     for(i=0;i<cf->dfsize;i++)
               *ptrd++ = *ptrs++;
   }
   return(NORM_RET);
 }
 else {
    /**** This if condition is added on 17-JUL-91 by Amarendra T ****/
    if((cf->dupctrl!=DUP_NONE) &&(cf->dupval !=0) &&(cf->lattr2&LA_SHODUP)) {
	filldat(1);
	edit();
	putdata() ;/* display edited data */
    }
    else
	putmask() ;
    /************/
    return(RET_IMM);
 }
}

uefom()   /* proc user escape request */
 {
   int c;
   if(!uesc) return(INVALID);
   else {
	/* showerror(7); input next char for uesc */
         c=get();
   if(lcase!=0 && isupper(c)) c=tolower(c);
   if(ucase!=0 && islower(c)) c=toupper(c);
         sp->retcode =RET_USER_ESC;
         sp->escchar[0]=c;
         return(VALID);
     }
 } /* end uefom()  */

hretp() /* proc a help return */

 {
  /* if hret is on fill code and return  */
  if(cf->lattr1&LA_HRET ) { /* help return is on for this field */
        sp->retcode=RET_HELP;
        return(VALID);
     }

  else if(cf->helpmes>0) {  /* help showerror is available */
              pmerln(fvp+cf->helpmes);
              return(INVALID);
         }


  else showerror(8);/*"Error .. no help messgaqe "*/
  return(INVALID);
 } /* end of  hretp(); */

 showchar(c) /* Display the character in c if it is printable character */
   int c;
  {
   if(c<=126 && c>=32 && echopt)  /* This is the range of printaqbles */
            if(echopt) put(c); else put(*dmaskptr);
    else put(*dmaskptr);
    return(0);
  }

valnum(c) /* checks if the chara in c is a valid nymeric */
 int c;
 {
   if(isdigit(c))
         if(type==TYP_NUM && dotposn!=0)
                        return(VALID);
         else return(INVALID); /* It is invalid numeric */
   else if(c=='.' && dotposn>=0)
                     return(INVALID);
 }

valalpha(c) /* checks if chara is valid alphanumeric */
          /* returns zero if yes.else returns -1    */

 int c;
 {
  if(c>=32 &&c<= 126)
        return(VALID);
  else return(INVALID);
 }

int digfound;
int curjump;

 char *tempptr; /* save dmaskptr value in case of a jump */
procnumeric()
{
 int 
    temp,
  success;

 if(signposn==0 && (c!='+' && c!='-'))
 	if (c >= '0' && c <= '9'){
 		fomunget(c);
 		c = '+';
 		}
 	else {
        showerror(9);/*"SIGn first"*/
        return(INVALID);
     	}
 dotfound=0;
 digfound=0;
 buffptr=0;
 curjump=0;
 signfound=0;
 

 do {
    success=0;
    if(*dmaskptr=='\0' && fldhold)  {
               if(c!=RETURN && c != LF && c!=DELETE)  {
                        showerror(10);/*" cannot enter buffer full  "*/
                        c=get();
   if(lcase!=0 && isupper(c)) c=tolower(c);
   if(ucase!=0 && islower(c)) c=toupper(c);
                        continue;
                }



       }
    if(isdigit(c)) 
     if(signposn==1 && signfound==1)  
                     showerror(11);/*"sign shld be last character"*/
     else  {
       digfound++;
       buff[buffptr++]=c;
       if(echopt) put(c); else put(*dmaskptr);
       success=TRUE;
     }

    else switch (c)  {
       

       case CURR_BACK: buffptr=0;
                       break;
        case REGEN: redraw();
                    break;
        case SNAP: snapscreen();
                  break;
        case RETURN:
        case TAB:
        case LF:                /* Inserted By T Hemadri To map CR To LF */
                  if(c==RETURN || c==LF)
                         sp->termcode=FT_CR;
                  else   sp->termcode=FT_TAB;
                  if(*dmaskptr!='\0')
                        sp->fillcode=FIL_PARTLY;
                  else  sp->fillcode=FIL_FULLY;
                  sp->retcode=RET_NO_ERROR;
                  return(VALID);
                 break;
        case ESCAPE : if(uefom()!=INVALID) return(RET_IMM);
        else
           showerror(12);/*"User escape not allowed for the field "*/
           break;


       case '.' : if (dotfound) {
                         showerror(13);/*"error..dec pt already found "*/
                         if(echopt) put(c); else put(*dmaskptr);
                         backspace();

                  }


                 else if (dotposn>=0) {

                         curjump=1;  /* making a jump to align with dec point */
                         dotfound=1;
                         curpos(&xposn,&yposn); /* save the current posnition */
                         tempptr=dmaskptr; /* Save in case you have to jump back  */

                         align();
                         buff[buffptr++]='.';
                         success=TRUE;
                     }

                 else {
                     showerror(14);/*"error .. no drcimal point in mask "*/
                     if(echopt) put(c); else put(*dmaskptr);
                     backspace();
                   }


                break;  /* case '.' */
  

     case '+' :

     case '-' :
 if(signfound==1)   {
   showerror(15);/*"sign has occured"*/
   break;
  }
 if(signposn==-1)  {/* no sign in input mask */
                    showerror(16);/*"Error ... no sign in mask "*/
                    if(echopt) put(c); else put(*dmaskptr);
                    backspace();
                   break;
                 }

               if(signposn==0)  { /* sign in first position */
                       if(buffptr==0)  {  /* Then accept */
                            buff[buffptr++]=c;
                            if(echopt) put(c); else put(*dmaskptr);
                            success=TRUE;
                            signfound=1;
                         }

                       else {
                            showerror(17);/*"Error.. sign first "*/
                            if(echopt) put(c); else put(*dmaskptr);
                            backspace();
                         }


          break;
              } /* if signposn==0 */
               if(signposn==1) { /* Sign in the last character position */
                     buff[buffptr++]=c;
                     put(c);
                     signfound=1;
                     success=TRUE;
                     break;
               }


     case DELETE : if (buffptr==0) showerror(18);/*"Error .. cannot delete "*/
                  else  { delete();
                   }
                 break;

     default: showerror(19);/*"Error .. illegal character"*/
              showchar(c);
              backspace();
              break;


  } /* switch */


 if (success==TRUE) {
         while(*(++dmaskptr)!=maskchar && *dmaskptr!='\0' && *dmaskptr!='.')put(*dmaskptr);
         if(*dmaskptr=='.')  { dotfound=1;
             buff[buffptr++]='.';
             put(DOT);
            while(*(++dmaskptr)!=maskchar && *dmaskptr!='\0' ) put(*dmaskptr);
           }
    }

 if(buffptr==0) return(INVALID) ; /* back to upper level module to  */
                            /* take care of special characters*/
 if(*dmaskptr || fldhold ) c=get();
   if(lcase!=0 && isupper(c)) c=tolower(c);
   if(ucase!=0 && islower(c)) c=toupper(c);
 } while(*dmaskptr  || fldhold );

 sp->retcode=RET_NO_ERROR;
 sp->fillcode=FIL_FULLY;
 sp->termcode=FT_FULL;
 return(VALID);
 
} /* End of proc numeric */

fixup(signptr,dotptr)
/* Routine to fin the position of sign and decimal point in the */
/* current display mask.                                        */
 int *signptr,*dotptr;
 {
  int i;
  char *ptr;
 *signptr= -1;
 *dotptr= -1;
 for(ptr=fvp+cf->imask,i=0;*ptr!='\0';ptr++,i++)
         if(*ptr=='S' || *ptr=='s') *signptr=i;
         else if(*ptr=='.') *dotptr=i;
if(*signptr>0) *signptr=1;
return(VALID);

}

delete()

 {

   buffptr--;  /* look at the previous character */
   c=buff[buffptr];
 if(isdigit(c)) 
        digfound--; /* One digit removed */
 else if(c=='.') 
        dotfound=0;
 else if(c=='+' || c=='-') signfound=0;
 backspace();
 while(*(--dmaskptr)!=maskchar && *dmaskptr!='\0' && *dmaskptr!='.') backspace();
 put(*dmaskptr);
 backspace();
 if(*dmaskptr=='.') { 
    if(curjump)  {
        poscur(xposn,yposn);
       dmaskptr=tempptr;
        curjump=0;
      }
    else  { backspace(); while(*(--dmaskptr)!=maskchar) backspace();
         put(*dmaskptr);
        backspace();
      digfound--;
       buffptr--;
      }
  }
}

int alfound; 
procalpha()
 {
   int success ; /* number of characters encountered */


   alfound=0;
   buffptr=0;
   do {
      success=FALSE;
     if(*dmaskptr=='\0' && fldhold)  {
             if(c!=RETURN && c != LF && c!=DELETE)  {
                            showerror(20);/*"cannot.. buff is full "*/
                            c=get();
   if(lcase!=0 && isupper(c)) c=tolower(c);
   if(ucase!=0 && islower(c)) c=toupper(c);
                            continue;
              }
    }


      if(valalpha(c)==VALID){
              alfound++;
              buff[buffptr++]=c;
              if(echopt) put(c); else put(*dmaskptr);
              success=1;
        }



     else if(c==ESCAPE)  {  /* User escape character  */
          if((success=uefom())!=INVALID) return(VALID); /* received valid user esc */

         else showerror(21);/*"Error ..Usr escape not on for this field "*/
         break;


    }
     else if(c==RETURN || c==LF || c==TAB) {
               if(c==RETURN || c==LF)
                       sp->termcode=FT_CR;
               else    sp->termcode=FT_TAB;
              

               if(*dmaskptr!='\0')
                       sp->fillcode=FIL_PARTLY;
               else 
                       sp->fillcode=FIL_FULLY;
               sp->retcode=RET_NO_ERROR;
                return(VALID);
       }
     else if(c==CURR_BACK) {
                      buffptr=0;
        }

     else if(c==REGEN) {
                redraw();
        }

     else if(c==SNAP) {
              snapscreen();
         }

     else if(c==DELETE)  
             if(buffptr==0)
                            showerror(22);/*"Error .. cannot delete "*/
           else {
		deletea();
            }

  else {
     showchar(c);
     backspace();
     showerror(23);/*" Illegal character"*/
   }



 if(success==TRUE) { /* update the cursor position */
      while(*(++dmaskptr)!=maskchar && *dmaskptr!='\0')  put(*dmaskptr);
   }


 if(buffptr==0) return(INVALID);
 if(*dmaskptr || fldhold ) c=get();
   if(lcase!=0 && isupper(c)) c=tolower(c);
   if(ucase!=0 && islower(c)) c=toupper(c);
 } while(*dmaskptr || fldhold);
/* fill return .....  */

 sp->retcode=RET_NO_ERROR;
 sp->fillcode=FIL_FULLY;
 sp->termcode=FT_FULL;
 return(VALID);

}

deletea()  /* delete for case =string */
{
 buffptr--;
 c=buff[buffptr];
 if(valalpha(c)) 
              alfound--;
 backspace();
 while(*(--dmaskptr)!=maskchar )
              backspace();
 put(*dmaskptr);
 backspace();
}

  int bufffull;
procdate()
{
 int success;
 buffptr=0;
 for(;*dmaskptr!=maskchar;dmaskptr++,imaskptr++);
 bufffull=FALSE;
 do {  /* do  till fldhold or end !enf of field */
      /* If success =1 at the end of the loop ,it means cursor posn and */
     /* dmask and imask positions have to be updated                    */
    success=FALSE;
    if(c==ESCAPE) 
             if(uefom()==VALID)
                      return(VALID);
             else showerror(24);/*" Uesc not on for this field "*/
   else if(c==RETURN || c==LF||c==TAB)
               if(!bufffull)
                       showerror(25);/*"buffer to be filled"*/
               else {
                      if(checkdate(c)==INVALID) return(INVALID);
                      sp->retcode=RET_NO_ERROR;
                      sp->fillcode=FIL_FULLY;
                      if(c==RETURN || c==LF)
                            sp->termcode=FT_CR;
                      else  sp->termcode=FT_TAB;
                      return(VALID);
                    }

  else if(c==CURR_BACK){
                    buffptr=0;
              }
 else if(c==REGEN)  {
           redraw();
           continue;
      }

else if(c==SNAP) {
    snapscreen();
    continue;
}

  else if(c==DELETE)
             if(buffptr==0)
                    showerror(26);/*" Cannot delete"*/
              else {
                     deleted();
                    }

 else if(bufffull)
               showerror(27);/*"cannot enter ..buffer full "*/
 else if(valchar(c)==VALID) {
                          buff[buffptr++]=c;
                          if(echopt) put(c); else put(*dmaskptr);
                          success=TRUE;
                    }

else  {
	/** showerror(28);  **/
          showchar(c);
          backspace();
     }

if(success==TRUE) { /* update cursor posn,dmaskptr,imaskptr  */
            while(*(++dmaskptr)!=maskchar && *dmaskptr!='\0') put(*dmaskptr);
            while(*(++imaskptr)=='F');
            if(*dmaskptr=='\0') bufffull=TRUE;
     }
if(buffptr==0) return(INVALID);
 if((!bufffull) || fldhold)c=get();
   if(lcase!=0 && isupper(c)) c=tolower(c);
   if(ucase!=0 && islower(c)) c=toupper(c);
} while(*dmaskptr!='\0' || fldhold);

if(checkdate()==INVALID) return(INVALID);
sp->retcode=RET_NO_ERROR;
sp->fillcode=FIL_FULLY;
sp->termcode=FT_FULL;
return(VALID);
}/* GETDATE() */

deleted()

{
 bufffull=FALSE;
 buffptr--;
 backspace();
 while(*(--dmaskptr)!=maskchar) {
            backspace();
            imaskptr--;
     }
 imaskptr--;

 put(*dmaskptr);
 backspace();
}


valchar(c)
 int c;
{
 if(*imaskptr=='M')
     if(mflength==3)
             if(isalpha(c))
                  return(VALID);
             else {
                  showerror(29);/*"alphabetical chara required "*/
                  return(INVALID);
              }

   

if(isdigit(c))
            return(VALID);
else {
      showerror(30);/*"Numeric expected "*/
      return(INVALID);
    }

} /* valchar() */

static int monthlength[12]= {31,28,31,30,31,30,31,31,30,31,30,31};
checkdate() /* check date field for corrctness */

{
 /* Find startr of next mask posiotion */
 char *imaskptr;
 char * buffposn;
 int year,date,monthnum;
 char monthname[4];
 buffposn=buff;

 imaskptr=cf->imask+fvp;
 while(*imaskptr=='F') imaskptr++;
 while(*imaskptr!='\0') {
    if(*imaskptr=='D') {
             sscanf(buffposn,"%2d",&date);
             buffposn+=2;
            imaskptr +=2;
       }

    else if(*imaskptr=='M') {
                if(mflength==2) 
                          sscanf(buffposn,"%2d",&monthnum);
                  

                else 
                         sscanf(buffposn,"%3s",monthname);
                         buffposn+=mflength;
                         imaskptr+=mflength;
                    }


   else { if(yrlength==4) 
                     sscanf(buffposn,"%4d",&year);
        else          sscanf(buffposn,"%2d",&year);
       buffposn+=yrlength;
      imaskptr+=yrlength;
      }
  while(*imaskptr=='F') imaskptr++;
 }

if(year == 0 && monthnum == 0 && date == 0) {
	return(VALID);
}

if(!(mflength==2))
            monthnum=convmonth(monthname);
if(monthnum<1 ||monthnum>12) {
              showerror(31);/*" Month out of range "*/
              return(INVALID);
          }

if(monthnum==2)
   if(leap(year)==VALID) monthlength[1]=29;
   else monthlength[1]=28;
  if(date<=monthlength[monthnum-1] &&date>0) return(VALID);
  else {
       showerror(32);/*"date out of bounds "*/
       return(INVALID);
    }

} /* datecheck() */


convmonth(name)  /* convert month name to mont number */
 char *name;
{

 static char* months[12]= {"jan","feb","mar","apr","may","jun","jul","aug",
                     "sep","oct","nov","dec"};
static char* months2[12]={"JAN","FEB","MAR","APR","MAY","JUN","JUL","AUG",
                     "SEP","OCT","NOV","DEC"};
 int i;

 for (i=0;i<=11;i++)
        if(strcmp(name,months[i])==0) return(i+1);
        else if(strcmp(name,months2[i])==0) return(i+1);
 return(-1);

}

/* align the decimal pointer with the one int the mask */

align()

{
  while(*(dmaskptr)!='.' && *dmaskptr!='\0'  ) { put(*dmaskptr);
        dmaskptr++;
    }
 put(*dmaskptr);
}



fixdate() /* fixup mflength and yrlength */
{
 char *ptr;
 mflength=0;
 yrlength=0;
 for((ptr=fvp+cf->imask); *ptr!='\0';ptr++)
       if(*ptr=='M')
               mflength++;
       else if (*ptr=='Y')
               yrlength++;
}

valdate(c) /* check if c is a valid character according to the mask */
int c;
{
 char *ptr;
 ptr=fvp+cf->imask;
 while(*(ptr)=='F')ptr++;
 if(mflength==3 && (*ptr)=='M')
           if(isalpha(c)) return(VALID);
            else return(INVALID);
 else if(isdigit(c)) return(VALID);
 else return(INVALID);
}

/* from=1 means  duplication */

leap(year)
  int year;
  {
   if(year % 4==0 && year % 100 !=0 || year % 400 ==0)
      return(VALID);
   else return(INVALID);
  }

/* from=1 means  duplication */

filldat(from)
 int from;
 {
  int size;
 int maskchar;
 int sign; /* The sign that was input */
 int signval; /* value to be anded with last digit..RM cobol  */
 char *buffer;
 char *ptr;
 int dotlocn; /* position of dot in mask ,counting only mask characters */
  char *ptrsb,*ptrse,*ptrdb,*ptrde,*ptrd,*ptrs,*temp;
 buff[buffptr]='\0';
  size=cf->dfsize;
  ptrdb=userdr+cf->drloc;
  ptrde=ptrdb+size-1;
  if(from==1)
     for(ptrs=fvp+cf->dupval,ptrd=ptrdb;size--!=0;*(ptrd++)= *(ptrs++));
 else if(type!=TYP_NUM) {
    /* scan from left and fillup   */
    for(ptrs=buff,ptrd=ptrdb;ptrs<(buff+buffptr);*(ptrd++)= *(ptrs++));
    /* fillup rsst of destination area with spaces        */
    for(;ptrd<=ptrde;*(ptrd++)=COB_SPACE);
 }

 else {
        buffer=buff;
        sign='+';
               if(signposn==0) {
                      sign=buff[0];
                      buffer++;
                   }

      if(dotposn>=0)  {  /* display mask contains a decimal point */
        /* find value of dotposition */

       dotlocn=0;
       maskchar=cf->maskchar;
       for(ptr=fvp+cf->dmask;*ptr!='.';ptr++)
               if(*ptr==maskchar)dotlocn++;
        if(signposn==0) dotlocn--; /* sign position */
        if(dotfound>0)  { /* user has input a decimal point */
           
             /* Fill up rhs of the decimal point */
             for(ptrs=buffer;*ptrs!='.';ptrs++);
             temp=ptrs; /* Save this value  */
             ptrs++;
             ptrd=userdr+cf->drloc+dotlocn;
             while(ptrs<(buff+buffptr))
                     *(ptrd++)= *(ptrs++);
             

            /* padd with zeroes  */
            while(ptrd<=ptrde) *(ptrd++)=COB_ZEROE;
           
            ptrd=userdr+cf->drloc+dotlocn-1;
           ptrs=temp-1;  /* to fill in the nondecimal part of number */
       }  /* if (dotfound)  */

     else {
            for(ptrd=userdr+cf->drloc+dotlocn;ptrd<=ptrde;*(ptrd++)=COB_ZEROE);
            ptrs=buff+buffptr-1;
            ptrd=userdr+cf->drloc+dotlocn-1;  /* To fill up non decimal part */
          }


  }
    else {  /* if(dotlocn>=0) */
        ptrd=ptrde;
        ptrs=buff+buffptr-1;
      }

    /* fillup whole number part of input  */
    
   while(ptrs>=buffer) *(ptrd--)= *(ptrs--);
   while(ptrd>=ptrdb) *(ptrd--)= COB_ZEROE;
  ptrd=userdr+cf->drloc+cf->dfsize-1;
   if(type==TYP_NUM &&signposn>=0 )
            if(sign=='+')
                 if(*(ptrd)=='0') *ptrd='{';
                  else *ptrd='A'+*ptrd-'0'-1;
            else if(*ptrd=='0') *ptrd='}';
                 else *ptrd='J'+*ptrd -'0'-1;
  }  /* else */
  } /* filldat()  */

procbool() {
#ifdef ENGLISH
 if(c!='y'&&c!='n'&&c!='Y'&&c!='N') {
#else
 if(c!='o'&&c!='n'&&c!='O'&&c!='N') {
#endif
      showerror(33);/*"Press Y   N "*/
      return(INVALID);
 }
 buff[0]=c;
 buffptr=1;
 if(fldhold)
   for(;;) {
     c=get();
   if(lcase!=0 && isupper(c)) c=tolower(c);
   if(ucase!=0 && islower(c)) c=toupper(c);
     if(c==DELETE || c==CURR_BACK) {
        buffptr=0;
        return(INVALID);
      }

     if(c==TAB ||c==RETURN || c==LF) {
       sp->retcode=RET_NO_ERROR;
       sp->fillcode=FIL_FULLY;
       if(c==RETURN || c==LF) sp->termcode=FT_CR;
        else sp->termcode=FT_TAB;
      return(VALID);
     }

    if(c==REGEN) 
       redraw();
    else if(c==SNAP)
      snapscreen();
   else showerror(34);/*"Field full ..Press CR or TAB "*/
 }

else {
    sp->retcode=RET_NO_ERROR;
    sp->fillcode=FIL_FULLY;
    sp->termcode=FT_FULL;
    return(VALID);
  }


} /* procbool  */
