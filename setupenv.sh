#!/bin/bash

# This script is needed to setup the environment for the make files to correctly compile and the
# applicaton to run.

# This has to run with the source command to take effect on the environment varible in the current shell
# source ./setupenv
echo "To affect the current shell this has to be run as follows \"source ./setupenv\""

# Setup varibles and folders for compile and use.

echo "Reseting HOME variable to be $PWD, used for compiling and running code"

export HOME=$PWD;
export LBASE=$PWD;

echo "Need to have a bin and lib folder for files created at compile.";
if [ ! -d $LBASE/lib ]
then
	echo " ** Creating lib folder"
	mkdir $HOME/lib
fi

if [ ! -d $LBASE/bin ]
then
	echo " ** Creating bin folder"
	mkdir $LBASE/bin
fi

echo "Compile order:"
ehco "profom compile, need to run linkfiles then"
echo "		for each fom file need to create an nfn file so run fom_to_n ####.fom"
echo "ctools"
echo "nfm - need to run crtnfm.SH and to do so need PATH to include profom/editor (where fom_to_n is)"
echo "dbh"
echo "Util - needed to build menu system"
