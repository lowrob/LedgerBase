/*
*    Source Name : sb_msgs.h
*    System      : Student Base System.
*
*    Created On  : Feb. 21st, 1991
*
*    Contains Defined Messages used by system.
*/

/* Trying to add duplicate key */
#define DUPREC "Given Key Already Exists - Please Re-Enter"

#define NOHELP "No Help Window for this Field"                          

/* Entered an option on the bottom line that isn't there */
#define INVOPT "Invalid Option"                 

/* Entered a key thats not on file while in Change, Inquire or Delete mode */
#define NOKEY "Given Key Does Not Exist - Please Re-Enter"                 

/* If entering a field that cannot contain any spaces/blanks */
#define NOSPACES "Field Cannot Include Spaces - Please Re-Enter"

/* Entered a number at the Field prompt that is not on the screen */
#define BADFIELD "No Such Field Number"                            

/* Entered a number at the Field prompt user not allowed to access */
#define CANTFIELD "Can't Changed Specified Field"                    

/* Using Next pg option and on last page of items */
#define LASTPG "Last Page Displayed"                             

/* Using Previos pg option and on first page of items */
#define FIRSTPG "First Page Displayed"                             

/* Using Next or Previous record functions at at the last or first record */
#define NOMORE "No More Records"                               

/* Added header by no detail or deleted all detail while in change mode */
#define NOITEMS "No Items to Save - Cancel to Quit"             

/* Try to change a line item that's been deleted during current session */
#define NOTACT "Record not Active - Reactivate First"          

#define BADSAVE "Error Saving Record"                           

#define SAVEHDR  "Error Saving Header"                           

#define SAVEITM  "Error Saving Items"                           

#define SAVEAUD  "Error Saving Audit Records"                           

/* If school code passed to the program not in school file */
#define NOSCHOOL "School Code Does Not Exist"

/* Entering a value in a non-key field that must exist in a certain file but 
   does not */
#define NOFIELD "Value Entered Not On File"

/* Entered same detail line key twice */
#define DUPITEM "Item Already Exists - Please Enter a Unique Value"

/* Problem creating pages of memory */
#define MEMERR "Internal Memory Allocation Error - Press Any Key to Continue"

/* Trying to get detail for an existing header and none exists */
#define INVITEM "No Items Found for Key Entered - Press Any Key to Continue"

/* Sucessfully deleted a record */
#define DELETED "Record Deleted - Press Any Key to Continue"

/* If Delete canceled */
#define DELCAN "Record Not Deleted - Press Any Key to Continue" 

/* Validation problem for Y or N */
#define YORN "Must be Y(es) or N(o)"

/* For other validation problems give the message in the format:
   Must be X(xxx), Y(yyyy) or Z(zzzz) i.e. list answers */

/* Message displayed when prompting at Field: */
#define FLDPROMPT "Press RETURN to Conclude Edit                     "

/* Message displayed when prompting at key field (maintenance program) */
#define KEYPROMPT "Press ESC-F to Return to Fn:                     "

/* Message displayed when prompting at key field (maintenance program) */
#define KEYPROMPT2 "Press ESC-F to Return to Menu                     "

/* Message displayed when working with detail (line items) */
#define ITMMSG "Press ESC-F to Display Option Line"

/* Message displayed when programs are successfully finished */ 
#define SUCCESS_MSG "PROCESS COMPLETED SUCCESSFULLY... PRESS ANY KEY TO RETURN TO MENU"

/* Message displayed when starting code greater than ending code */
#define NOTGRTR "Starting Code Cannot Be Greater Than Ending Code"

/* Message Displayed when file locked */
#define BUSY "File Busy - Please Try Again Later"

/* Message displayed when line item matches one already added */
#define BEENENT "This Code Has Already Been Enterred - Please Re-enter"

/* Message to tell user to hit any key */
#define ANYKEY "Press Any Key"

/* Message to tell the user that the status is DELETED, so can't edit */
#define STATDEL "Item deleted - Can't edit"

/* Message to tell the user that the status is already DELETED */
#define ALDEL "Item already deleted"

/* Message to tell the user that the status is already ACTIVATED */
#define ALACT "Item already active"

/* Message to display while files being built etc. */
#define WAIT_MSG "Processing - Please Wait                                                      "
