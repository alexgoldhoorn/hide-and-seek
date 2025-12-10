#!/bin/sh
#1. port
x=1
while [ $x -le 100 ]
do
./runallsolversformap.sh $x 119 $HSPATH/data/maps/bcn_lab/fme2014_map4.txt ../dmat/fme2014_map4.dmat.txt 100 1000 500 $1 1
./runallsolversformap.sh $x 120 $HSPATH/data/maps/brl/brl29a.txt ../dmat/brl29a.dmat.txt 1000 3000 1000 $1 2
./runallsolversformap.sh $x 121 $HSPATH/data/maps/brl/master29e.txt ../dmat/master29e.dmat.txt 1000 3000 1000 $1 5
./runallsolversformap.sh $x 122 $HSPATH/data/maps/brl/master29f.txt ../dmat/master29f.dmat.txt 1000 3000 1000 $1 5

   sleep 10

  x=$(( $x + 1 ))

done

