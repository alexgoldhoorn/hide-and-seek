#!/bin/sh
#1. port
#2. exp name / port

if [ $# -lt 2 ]
then
echo "Required parameters: port exp-name"
else

x=1
while [ $x -le 100 ]
do
./runallsolversformap.sh $x 120 $HSPATH/data/maps/brl/brl29a.txt ../dmat/brl29a.dmat.txt 200 2500 1000 $1 2 $2
 sleep 3
./runallsolversformap.sh $x 119 $HSPATH/data/maps/bcn_lab/fme2014_map4.txt ../dmat/fme2014_map4.dmat.txt 100 1000 500 $1 1 $2
sleep 3
./runallsolversformap.sh $x 122 $HSPATH/data/maps/brl/master29f.txt ../dmat/master29f.dmat.txt 200 2500 1000 $1 5 $2
sleep 3
./runallsolversformap.sh $x 121 $HSPATH/data/maps/brl/master29e.txt ../dmat/master29e.dmat.txt 200 2500 1000 $1 5 $2
sleep 3

  x=$(( $x + 1 ))

done
fi
