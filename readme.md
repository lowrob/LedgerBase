Old code I have from a financial system that was writen in the late 80's, early 90's.  Need to recover more of the files but this is the start of what I have found so far.  

This code was ran on a server and vt100 was used to access the system.  It had a isam DB.  Will fill in more info if I can reocover the software from old tapes and drives.

Compile order:
profom;
	runtime/makecmnd
	editor/makecmnd
	cd editor
		fom_to_n PROFOMC
		fom_to_n PROFOMU
		fom_to_n PROFOMS
		fom_to_n PROFOMF
	linkfiles
	
dbh = database code and utilities.

nfm = menu system that was used on terminal - tnames limited access

util = has menuscrn that is used to call the menu items .nfm

pp = personnel / payroll

include = include files for all the code

formats = structures (don't remember how to use this yet - these are the source that builds data files of type .RPF, .RPN and .RPS)

