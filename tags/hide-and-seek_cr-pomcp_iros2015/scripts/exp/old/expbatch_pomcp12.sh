#!/bin/sh


x=1
while [ $x -le 100 ]
do
  if [ ! -f "EXP.RUN" ];
  then
	echo "STOP (file 'EXP.RUN' is not present)"
	break
  fi


  echo "Run $x"



./expbatch_pomcp8.py "6x5:3,4;20x20:4,5;40x40:1,3,4" i$x pn2 2 "-s localhost 1122 "




  x=$(( $x + 1 ))




sleep 10

done
