#!/bin/sh
#cd build/
#cd $HOME/iri-lab/labrobotica/algorithms/hide-and-seek/trunk/build
echo "Make all HS programs"
if [ -z "$HSPATH" ]
then
cd ..
cd trunk
cd build
else
cd $HSPATH/trunk/build
fi
if [ $# -eq 0 ]
then
qmake ../XMLConfig_lib.pro && make && qmake ../hsserver.pro && make && qmake ../hsmomdp.pro && make && qmake ../hsmomdp_debug.pro && make && qmake ../hsclient.pro && make && qmake ../hsautohider.pro && make
else
if [ '$1'=='-n' ]
then
echo "No Debug!"
qmake ../XMLConfig_lib.pro && make && qmake ../hsserver.nodebug.pro && make clean && make && qmake ../hsmomdp.nodebug.pro && make clean && make && qmake ../hsmomdp_debug.pro && make && qmake ../hsclient.pro && make && qmake ../hsautohider.nodebug.pro && make clean && make
else
echo "Unknown parameter: '$1', use no parameters to compile all normally, and '-n' without debug"
fi
fi
