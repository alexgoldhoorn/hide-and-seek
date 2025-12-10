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



./expbatch_pomcp4.py "6x5:1,2,3;10x10:1,2,3,4;20x20:1,2,3,4;40x40:1,2" i$x p8 r "-s localhost 1120"




  x=$(( $x + 1 ))




sleep 10

done
