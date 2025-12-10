#!/bin/bash 
if [ "$1" == "-h" ] || [ "$1" == "--help" ]
then
   echo "qmake_all [-d | -n]"
   echo 
   echo " -n : no debug"
   echo " -d : use default compiler (g++/gcc)"
   exit
fi


function changeToDefault {
#change to g++ instead of g++-5 and don't use OpenCV 3
perl -p -e 's/g\+\+-5/g\+\+/g;s/gcc-5/gcc/g;s/CONFIG\+\s*=\s*USE\_OPENCV3/CONFIG\+=USE\_OPENCV2/g' $1.pro > $1$default.pro
}

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

if [ "$1" == "-n" ] || [ "$2" == "-n" ]
then
    noDebug=1
else
    noDebug=0
fi
if [ "$1" == "-d" ] || [ "$2" == "-d" ]
then
   default=.def
   changeToDefault ../XMLConfig_lib
   changeToDefault ../hsserver
   changeToDefault ../hsmomdp
   changeToDefault ../hsclient
   changeToDefault ../hsautohider
else
   default=
fi


if [ $noDebug -eq 0 ]
then
qmake ../XMLConfig_lib$default.pro && make && qmake ../hsserver$default.pro && make && qmake ../hsmomdp$default.pro && make && qmake ../hsclient$default.pro && make && qmake ../hsautohider$default.pro && make
else
#if [ '$1'=='-n' ]
#then
echo "No Debug!"
qmake ../XMLConfig_lib.pro && make && qmake ../hsserver.nodebug.pro && make clean && make && qmake ../hsmomdp.nodebug.pro && make clean && make qmake ../hsclient.pro && make && qmake ../hsautohider.nodebug.pro && make clean && make
#else
#echo "Unknown parameter: '$1', use no parameters to compile all normally, and '-n' without debug"
#fi
fi
