# Setup varibles and folders for compile and use.

echo ""
echo "Reseting HOME variable to be $PWD, used for compiling and running code"
export HOME=$PWD

echo ""
echo "Need to have a bin and lib folder for files created at compile."
if [ ! -d $HOME/lib ] 
then
	echo " ** Creating lib folder"
	mkdir $HOME/lib
fi

if [ ! -d $HOME/bin ] 
then
	echo " ** Creating bin folder"
	mkdir $HOME/bin
fi
