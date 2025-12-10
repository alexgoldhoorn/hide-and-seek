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


./hsmomdp -s localhost 1120 -sc -u u_pomcp_Rt -m map3_6x5 -Rt -d 40 -ns 10000 -ni 4 -x 2 -e 60 -or > out1.txt

./hsmomdp -s localhost 1120 -sc -u u_pomcp_Rt -m map3_10x10 -Rt -d 40 -ns 10000 -ni 10 -x 2 -e 200 -or > out2.txt

./hsmomdp -s localhost 1120 -sc -u u_pomcp_Rt -m map3_20x20 -Rt -d 40 -ns 10000 -ni 16 -x 2 -e 800 -or > out3.txt

./hsmomdp -s localhost 1120 -sc -u u_pomcp_Rt -m map3_6x5 -Rt -d 40 -ns 10000 -ni 4 -x 2 -e 60 -ov > out4.txt

./hsmomdp -s localhost 1120 -sc -u u_pomcp_Rt -m map3_10x10 -Rt -d 40 -ns 10000 -ni 10 -x 2 -e 200 -ov > out5.txt

./hsmomdp -s localhost 1120 -sc -u u_pomcp_Rt -m map3_20x20 -Rt -d 40 -ns 10000 -ni 16 -x 2 -e 800 -ov > out6.txt


  x=$(( $x + 1 ))




sleep 30

done
