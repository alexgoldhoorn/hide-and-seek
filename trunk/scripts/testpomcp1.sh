#!/bin/sh

x=1
while [ $x -le 100 ]
do
  echo "Run $x "

./actiongen ../map1_30x30.txt map1_30x30_actions_i$x.txt map1_30x30_actions_i$x

./hsmomdp -s localhost 1120 -sc -as ../map1_30x30.txt -ol map1_30x30_actions_i$x.txt -u pomcp_map1_Rt -ns 10000 -Rt -En 
./hsmomdp -s localhost 1120 -sc -as ../map1_30x30.txt -ol map1_30x30_actions_i$x.txt -u pomcp_map1_Rf -ns 10000 -Rf -Es 
./hsmomdp -s localhost 1120 -ss -as ../map1_30x30.txt -ol map1_30x30_actions_i$x.txt -u smartseeker_map1 -ssan -sshn


./hsmomdp -s localhost 1120 -sc -as ../map1_30x30.txt -o2 -u pomcp_map1_Rt -ns 10000 -Rt -En 
./hsmomdp -s localhost 1120 -sc -as ../map1_30x30.txt -o2 u pomcp_map1_Rf -ns 10000 -Rf -Es 
./hsmomdp -s localhost 1120 -ss -as ../map1_30x30.txt -o2 -u smartseeker_map1 -ssan -sshn

sleep 1

./actiongen ../map2_30x30.txt map2_30x30_actions_i$x.txt map2_30x30_actions_i$x

./hsmomdp -s localhost 1120 -sc -as ../map2_30x30.txt -ol map2_30x30_actions_i$x.txt -u pomcp_map2_Rt -ns 10000 -Rt -En 
./hsmomdp -s localhost 1120 -sc -as ../map2_30x30.txt -ol map2_30x30_actions_i$x.txt -u pomcp_map2_Rf -ns 10000 -Rf -Es 
./hsmomdp -s localhost 1120 -ss -as ../map2_30x30.txt -ol map2_30x30_actions_i$x.txt -u smartseeker_map2 -ssan -sshn


./hsmomdp -s localhost 1120 -sc -as ../map2_30x30.txt -o2 -u pomcp_map2_Rt -ns 10000 -Rt -En 
./hsmomdp -s localhost 1120 -sc -as ../map2_30x30.txt -o2 u pomcp_map2_Rf -ns 10000 -Rf -Es 
./hsmomdp -s localhost 1120 -ss -as ../map2_30x30.txt -o2 -u smartseeker_map2 -ssan -sshn

sleep 1


  x=$(( $x + 1 ))

rm *policy
rm debug*.txt

done


