#!/bin/sh


x=1
while [ $x -le 50 ]
do
  if [ ! -f "EXP.RUN" ];
  then
	echo "STOP (file 'EXP.RUN' is not present)"
	break
  fi


  echo "Run $x"


./expbatch_pomcp4.py "40x40:1,2,3,4" i$x p8 v "-s localhost 1120"


  x=$(( $x + 1 ))




sleep 10

done
