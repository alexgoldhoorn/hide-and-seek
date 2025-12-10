#!/bin/sh
SRC=/home/agoldhoorn/MyProjects/HideSeekGame/hscomplete2
DST=/home/agoldhoorn/iri-lab/labrobotica/algorithms/hide-and-seek/trunk

cp -r $SRC/src/HSGame/* $DST/src/HSGame/
cp -r $SRC/src/PathPlan/* $DST/src/PathPlan/
cp -r $SRC/src/Segment/* $DST/src/Segment/
cp -r $SRC/src/Utils/* $DST/src/Utils/
cp -r $SRC/src/Solver/* $DST/src/Solver/
cp -r $SRC/src/PathPlan/* $DST/src/PathPlan/
cp $SRC/src/hsconfig.h $DST/src
cp $SRC/src/hsglobaldata.h $DST/src
cp $SRC/src/autoplayer.h $DST/src
cp $SRC/src/seekerhs.h $DST/src
cp $SRC/src/hsglobaldata.cpp $DST/src
cp $SRC/src/seekerhs.cpp $DST/src
cp $SRC/src/Utils/hslog.* $DST/include/appl-0.95/Algorithms/SARSOP/


#cat test.txt | while read line; do echo "$line DST";done > test2.txt 
# 

