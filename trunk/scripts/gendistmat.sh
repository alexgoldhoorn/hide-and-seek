#!/bin/sh
echo "Generate a distance matrix"
echo
if [ $# -lt 2 ]; 
then
echo "Expecting the following parameters:"
echo
echo "	map-file dist-matrix-file [-C]"
echo
echo "-C: continuous distances"
else
#-as outtest.txt -dw outtest.dmat.txt
#AG140606: added -C (continuous distance)
$HOME/iri-lab/labrobotica/algorithms/hide-and-seek/trunk/build/build/release/momdp/hsmomdp -as $1 -dw $2 $3
fi
echo


