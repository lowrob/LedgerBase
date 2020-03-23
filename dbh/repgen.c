/*----------------------------------------------------------------
	Source Name : repgen.c
	Author      : T Amarendra
	Created Date: 4th May 1989.

	Functions for report printing modules.

	SPOOLER: Report will be spooled, when spool option selected
		 for Output on Printer option, otherwise directly
		 printed on printer.
----------------------------------------------------------------*/

#include <stdio.h>
#include <bfs_defs.h>

#define	REP

#include <repdef.h>

/* Spooler Implementation */

#ifdef	SPOOLER
#define	RPTEMP	"RGXXXXXX"
char	*mktemp() ;
static	char	tempname[15] ;
static	char	*printer ;
#endif
static	int	numberofcopies;

/*---------------------------------------------------------*/
opn_prnt(disp,termnm,prntno,e_msg,spool) /* get printer device and open */
char	*disp;   /* "D" - display, "P" - print and "F" - Disk file */
char	*termnm;
int	prntno;
char	*e_msg;
int	spool ;	/* 1 - Spool the printer output */
{
	char	devname[30];

#ifdef	SPOOLER
	printer	= NULL ;	/* Wiil be used in close_rep() */
#endif
	linecnt = 0;
	cur_pos = 0;
	numberofcopies = 1;

	if(disp[0] == 'D'){
		if(get_term(termnm) < 0){
#ifdef ENGLISH
			strcpy(e_msg,"ERROR - This Terminal Not Found ");
			strcat(e_msg,"in REP_GEN Terminal Database... ");
#else
			strcpy(e_msg,"ERREUR - Ce terminal pas retrouve");
			strcat(e_msg,"dans la base de donnees du terminal REP_GEN... ");
#endif
			return(-1);
		}
		prfd = fileno(stdout);
		PGSIZE = 22;
		return(0);
	}

	term = 99 ;

	if(disp[0] == 'F') {
/*****
		strcpy(devname,WORK_DIR);
#ifdef	MS_DOS
		strcat(devname,"\\");
#else
		strcat(devname,"/");
#endif
		strcat(devname,termnm);
*****/
		strcpy(devname, termnm) ;
		if((prfd = creat(devname,TXT_CRMODE)) == -1){
#ifdef ENGLISH
			sprintf(e_msg,"Unable To Creat '%s'Disk File",devname);
#else
			sprintf(e_msg,"Incapable de cree le dossier '%s' du disque",devname);
#endif
			return(-1);
		}

		PGSIZE = 66;
		return(0);
	}

	/* Printer option */
	prfd = get_prn_fd(prntno,devname);
	if(prfd < 0){
#ifdef ENGLISH
		strcpy(e_msg,"Given Printer# NOT Found in Terminal/Printer ");
		strcat(e_msg,"Maintenance File");
#else
		strcpy(e_msg,"#d'imprimante donne pas retrouve dans le Dossier d'entretien");
		strcat(e_msg," du term/imprimante");
#endif
		return(-1);
	}

#ifdef	SPOOLER
	if(spool) {
		strcpy(tempname, RPTEMP) ;
		printer = mktemp(tempname) ;

		if(printer != NULL && (prfd = creat(printer,TXT_CRMODE)) == -1){
#ifdef ENGLISH
			strcpy(e_msg,"Unable To Open Printer");
#else
			strcpy(e_msg,"Incapable d'ouvrir l'imprimante");
#endif
			return(-1);
		}
	}
	else
#endif
	if((prfd = open(devname,TXT_WRMODE)) == -1){
#ifdef ENGLISH
		strcpy(e_msg,"Unable To Open Printer");
#else
		strcpy(e_msg,"Incapable d'ouvrir l'imprimante");
#endif
		return(-1);
	}

	return(0);
}
/*-------------------------------------------------------*/
mkln(offset,s,n)
int offset,n;	/* Offset is 1 to n */
char *s;
{
	int i;

	/* offset < cur_pos Ignore call */
	if( offset <= cur_pos )
		return(-1) ;
	for( ; cur_pos < offset - 1 ; cur_pos++)
		line[cur_pos] = ' ';
	for(i = 0 ; i < n && cur_pos < LNSZ - 1 && s[i] != '\0' ; i++,cur_pos++)
		line[cur_pos] = s[i];


	return(i) ;	/* No of characters Copied */
}
/*--------------------------------------------------------*/
prnt_line()
{
	if(cur_pos > (LNSZ - 1))cur_pos = (LNSZ - 1);
	line[cur_pos++] = '\n';
	if(write(prfd,line,cur_pos) != cur_pos) {
#ifdef ENGLISH
	    prnt_mesg("Error In Writing Output");
#else
	    prnt_mesg("Erreur en inscrivant la sortie des donnees");
#endif
	    return(-1);
	}
	cur_pos = 0;
	linecnt++;
	return(0);
}
/*---------------------------------------------------------------------------*/
next_page()
{
	char c[2];

	fflush(stdin);
	for( ; linecnt < PGSIZE ; linecnt++)
		write(prfd,"\n",1);
#ifdef ENGLISH
	write(prfd,"Press Q<RETURN> to Quit ELSE 'RETURN' to Continue  ",50); 
#else
	write(prfd,"Appuyer sur Q<RETURN> pour retourner sinon 'RETURN' pour continuer",66);
#endif
	read(fileno(stdin),c,2);
	if(c[0] == 'Q' || c[0] == 'q')
	        return(-1);
	return(0);
}
/*-------------------------------------------------------*/
last_page()
{
#ifdef ENGLISH
	prnt_mesg("Press RETURN to Continue  ");
#else
	prnt_mesg("Appuyer sur RETURN pour continuer  ");
#endif

	return(0) ;
}
/*-------------------------------------------------------------*/
close_rep()
{
#ifdef	SPOOLER
	int	yes ;
#endif

	if(prfd > 0 && term == 99) {	/* not terminal */
#ifdef	SPOOLER
		if(printer != NULL) {	/* Spool Option Selected */
			/* Check whether any report Generated */
			if(lseek(prfd, 0L, 2) > 0)
				yes = 1 ;
			else
				yes = 0 ;
		}
#endif
		close(prfd);
	}

#ifdef	SPOOLER
	if(printer != NULL) {	/* Printer Option Selected */
		if(yes) 
			SpoolReport( printer, 1, numberofcopies );
		unlink(printer);
		printer = NULL ;
	}
#endif

	prfd = -1;

	return(0) ;
}
/*-------------------------------------------------------------*/
static	int
prnt_mesg(s) /* display given message at the end of screen */
char *s;
{
	char c[2];

	fflush(stdin);
	for( ; linecnt < PGSIZE - 1; linecnt++)
	    write(prfd,"\n",1);
	write(prfd,s,strlen(s));
	read(fileno(stdin),c,2);
	fflush(stdin);
	if(term == 99)close(prfd);

	return(0) ;
}
/*-----------------------------------------------------------*/
get_term(s)
char *s;
{
    register int i;
    static char *tarr[] =
        {
            "oen",
            "tvi",
            "W50",
            "W85",
            "CM",
            "MM",
            "perq",
	    "N7901"
        };

    for(i = 0; i < 8 ; i++)
        if(strcmp(s,tarr[i])==0){
	    term = i + 1;
            return(0);
	}
    term = -1;
    return(-1);
}

/*------------------------------------------------------------------*/
rite_top()
{
    linecnt = 0;
    switch(term)
    {
        case 1:            /* oen */
	    write(prfd,"\033T",2);
            return(0);
        case 2:            /* tvi */
        case 3:           /* W50 */
	    write(prfd,"\033*",2);
            return(0);
        case 4:            /* W85 */
            write(prfd,"\033[2J\033[1;1H",10);
            return(0);
        case 5:            /* CM */
        case 6:            /* MM */
            write(prfd,"\033[2J\033[H",7);
            return(0);
        case 7:            /* perq */
	    write(prfd, "\033K",2);
            return(0);
	case 8:
	    write(prfd, "\014",1);
            return(0);
        default:
            if(write(prfd,"\014\0",1) != 1)
                return(-1);
            return(0);    
    } 
}
/*--------------------------------------------------------------------*/
/****
 Reads "tnames" file in NFM_PATH directory and gets the
 prinets device name & PGSIZE for a given printer number.

 This file will be different for different installations.
 It contains the information of terminals and printers for
 this installation. Format is

 Terminals:
        {Device Name} {Terminal PROFOM Name} {"0"}

 Printers:
        {Device Name} {"PRN"} {No of Print lines}


 Example:
    For a Installation containg 2 terminals & 2 printers
    file looks like this

        /dev/console MM 0
        /dev/tty1 MM 0
        /dev/tty2 PRN 60
        /dev/lp PRN  60

*****/

get_prn_fd(prntno,devname)	/* This routine will find the PRN entry
				   in "tnames" file & copies device name */
int prntno;
char *devname;
{
    register FILE *fpt;
    register int i,j=0;
    char name[30];
    char filenam[30];

    strcpy(filenam,NFM_PATH);
    strcat(filenam,"tnames");
    fpt=fopen(filenam,"r");
    if(fpt == NULL){
#ifdef	MS_DOS
		/* if tnames file not available then take default PRINTER
		   name as "PRN" */
		strcpy(devname,"PRN");
		PGSIZE = 66;
		return(0);
#else
#ifdef ENGLISH
        printf("\n\nTerminal names file  %s not found:<RETURN>",filenam);
#else
        printf("\n\nDossier des noms de terminaux %s pas retrouve:<RETURN>",filenam);
#endif
        read(fileno(stdin),name,1);
        exit(-1);
#endif
    }
    for(i=0; ;i++){
        if(fscanf(fpt,"%s %s %hd",devname,name,&PGSIZE) <= 0){
            i = -1;
            break;
        }
        if(strcmp(name,"PRN") == 0){
            j++;
            if(prntno == j)break;
        }
    }
    fclose(fpt);
    if(i < 0){
#ifdef	MS_DOS
		/* if tnames file not available then take default PRINTER
		   name as "PRN" */
		strcpy(devname,"PRN");
		PGSIZE = 66;
		return(0);
#else
    	return(-1);
#endif
	}
    return(0);
}

#ifdef SPOOLER
SpoolReport( filename, printer_no, no_of_copies )
char	*filename;
int	printer_no, no_of_copies;
{
	char	temp[100];

	if( access( filename, 0 ) < 0 )
		return(0);
	if( printer_no < 1 )
		printer_no = 1 ;
	if( no_of_copies < 1 )
		no_of_copies = 1 ;

#ifdef	XENIX
	sprintf(temp,"lp -s -c -n%d %s", no_of_copies, filename);
#else
#ifdef	i386
	sprintf(temp,"lp -s -o nobanner -c -n%d %s", no_of_copies,filename);
#else
	sprintf(temp,"lpr -cp %d %s", no_of_copies, filename);
#endif
#endif

	system( temp );

	return(0) ;
}
#endif

SetCopies( no_of_copies )
int	no_of_copies;
{
	numberofcopies = no_of_copies;
	return(0);
}
/*-------------------- E n d   O f   P r o g r a m ---------------------*/
