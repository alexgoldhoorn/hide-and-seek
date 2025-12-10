#!/bin/sh

x=1
while [ $x -le 100 ]
do
  echo "Run $x "

./hsmomdp -so -m pomdpx/simplerew_fs/map3_10x10_3o_d06.pomdpx -p pomdpx/simplerew_fs/map3_10x10_3o_d06.policy -or -L map3_sr_d06_i$x -u map3_sr_d06 -A 14 > map3_sr_d06_i$x.log

./hsmomdp -so -m pomdpx/simplerew_fs/map3_10x10_3o.pomdpx -p pomdpx/simplerew_fs/map3_10x10_3o.policy -or -L map3_sr_d095_i$x -u map3_sr_d095 -A 14 > map3_sr_d095_i$x.log

  x=$(( $x + 1 ))

rm *policy
rm debug*.txt

done


