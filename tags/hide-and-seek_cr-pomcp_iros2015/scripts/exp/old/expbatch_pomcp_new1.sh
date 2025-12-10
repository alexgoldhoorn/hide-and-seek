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


./hsmomdp -o2 -sc -u Pomcp2_Ib -Rt -Ib 0 -m map3_10x10 -ns 100000  -s localhost 1120
./hsmomdp -o2 -sc -u Pomcp2_Ib -Rt -Ib 0 -m map4_20x20 -ns 100000  -s localhost 1120
./hsmomdp -o2 -sc -u Pomcp2_Ib -Rt -Ib 0 -m map4_40x40 -ns 100000  -s localhost 1120

sleep 1
# equal dist
./hsmomdp -o2 -sc -u Pomcp2_IdEq -Rt -Id 4 4 -m map3_10x10 -ns 100000  -s localhost 1120
./hsmomdp -o2 -sc -u Pomcp2_IdEq -Rt -Id 9 9 -m map4_20x20 -ns 100000  -s localhost 1120
./hsmomdp -o2 -sc -u Pomcp2_IdEq -Rt -Id 19 19 -m map4_40x40 -ns 100000  -s localhost 1120

sleep 1
# seeker closer
./hsmomdp -o2 -sc -u Pomcp2_IdSC -Rt -Id 2 4  -m map3_10x10 -ns 100000  -s localhost 1120
./hsmomdp -o2 -sc -u Pomcp2_IdSC -Rt -Id 4 9 -m map4_20x20 -ns 100000  -s localhost 1120
./hsmomdp -o2 -sc -u Pomcp2_IdSC -Rt -Id 8 19 -m map4_40x40 -ns 100000  -s localhost 1120


sleep 1
# hider closer
./hsmomdp -o2 -sc -u Pomcp2_IdHC -Rt -Id 4 2 -m map3_10x10 -ns 100000  -s localhost 1120
./hsmomdp -o2 -sc -u Pomcp2_IdHC -Rt -Id 9 4 -m map4_20x20 -ns 100000  -s localhost 1120
./hsmomdp -o2 -sc -u Pomcp2_IdHC -Rt -Id 19 8 -m map4_40x40 -ns 100000  -s localhost 1120









  x=$(( $x + 1 ))




sleep 2

done
