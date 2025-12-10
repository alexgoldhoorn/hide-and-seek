#!/bin/sh

x=1
while [ $x -le 100 ]
do
  echo "Run $x "

./hsmomdp -so -m pomdpx/simplerew_fs/map3_10x10_3o_d06.pomdpx -p pomdpx/simplerew_fs/map3_10x10_3o_d06.policy -or -L offline_map3_sr_d06_i$x -u offline_map3_sr_d06 -A 14 > offline_map3_sr_d06_i$x.log 

./hsmomdp -so -m pomdpx/simplerew_fs/map3_10x10_3o.pomdpx -p pomdpx/simplerew_fs/map3_10x10_3o.policy -or -L offline_map3_sr_d095_i$x -u offline_map3_sr_d095 -A 14 > offline_map3_sr_d095_i$x.log


./hsmomdp -sl -gb -m pomdpx/simplerew_fs/map3_10x10_3o_d06.pomdpx -p pomdpx/simplerew_fs/map3_10x10_3o_d06.policy -or -L layerBase_map3_sr_d06_i$x -u layerBase_map3_sr_d06 -A 14 -T 300 > layerBase_map3_sr_d06_i$x.log

./hsmomdp -sl -gb -m pomdpx/simplerew_fs/map3_10x10_3o.pomdpx -p pomdpx/simplerew_fs/map3_10x10_3o.policy -or -L layerBase_map3_sr_d095_i$x -u layerBase_map3_sr_d095 -A 14 -T 300 > layerBase_map3_sr_d095_i$x.log

./hsmomdp -sl -gk 10 -m pomdpx/simplerew_fs/map3_10x10_3o_d06.pomdpx -p pomdpx/simplerew_fs/map3_10x10_3o_d06.policy -or -L layerK10_map3_sr_d06_i$x -u layerK10_map3_sr_d06 -A 14 -T 300 > layerK10_map3_sr_d06_i$x.log

./hsmomdp -sl -gk 10 -m pomdpx/simplerew_fs/map3_10x10_3o.pomdpx -p pomdpx/simplerew_fs/map3_10x10_3o.policy -or -L layerK10_map3_sr_d095_i$x -u layerK10_map3_sr_d095 -A 14 -T 300 > layerK10_map3_sr_d095_i$x.log

./hsmomdp -sl -gr -m pomdpx/simplerew_fs/map3_10x10_3o_d06.pomdpx -p pomdpx/simplerew_fs/map3_10x10_3o_d06.policy -or -L layerRC_map3_sr_d06_i$x -u layerRC_map3_sr_d06 -A 14 -T 300 > layerRC_map3_sr_d06_i$x.log

./hsmomdp -sl -gr -m pomdpx/simplerew_fs/map3_10x10_3o.pomdpx -p pomdpx/simplerew_fs/map3_10x10_3o.policy -or -L layerRC_map3_sr_d095_i$x -u layerRC_map3_sr_d095 -A 14 -T 300 > layerRC_map3_sr_d095_i$x.log

./hsmomdp -sl -gr 5 45 -m pomdpx/simplerew_fs/map3_10x10_3o_d06.pomdpx -p pomdpx/simplerew_fs/map3_10x10_3o_d06.policy -or -L layerRC_d5_a45_map3_sr_d06_i$x -u layerRC_d5_a45_map3_sr_d06 -A 14 -T 300 > layerRC_d5_a45_map3_sr_d06_i$x.log

./hsmomdp -sl -gr 5 45 -m pomdpx/simplerew_fs/map3_10x10_3o.pomdpx -p pomdpx/simplerew_fs/map3_10x10_3o.policy -or -L layerRC_d5_a45_map3_sr_d095_i$x -u layerRC_d5_a45_map3_sr_d095 -A 14 -T 300 > layerRC_d5_a45_map3_sr_d095_i$x.log


  x=$(( $x + 1 ))

rm *policy
rm debug*.txt

done


