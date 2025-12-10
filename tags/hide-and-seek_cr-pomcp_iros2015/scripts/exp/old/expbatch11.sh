#!/bin/sh

x=1
while [ $x -le 100 ]
do
  echo "Run $x "


./hsmomdp -s localhost 1121 -so -m pomdpx/simplerew_fs/map3_10x10_3o.pomdpx -p pomdpx/simplerew_fs/map3_10x10_3o.policy -or -L offline_map3_10x10_sr_i$x -u offline_map3_sr_10x10 -A 14 > offline_map3_sr_10x10_i$x.log 

./hsmomdp -s localhost 1121 -so -m pomdpx/newrew_fs_r3t1/map3_10x10_3o.pomdpx -p pomdpx/newrew_fs_r3t1/map3_10x10_3o.policy -or -L offline_map3_10x10_nr_i$x -u offline_map3_nr_10x10 -A 14 > offline_map3_nr_10x10_i$x.log 

./hsmomdp -s localhost 1121 -sl -T 30 -gb -m pomdpx/simplerew_fs/map3_10x10_3o.pomdpx -or -L layerBase_map3_10x10_sr_i$x -u layerBase_map3_sr_10x10 -A 14 > layerBase_map3_sr_10x10_i$x.log 

./hsmomdp -s localhost 1121 -sl -T 30 -gb -m pomdpx/newrew_fs_r3t1/map3_10x10_3o.pomdpx -or -L layerBase_map3_10x10_nr_i$x -u layerBase_map3_nr_10x10 -A 14 > layerBase_map3_nr_10x10_i$x.log 

./hsmomdp -s localhost 1121 -sl -T 30 -gr -m pomdpx/simplerew_fs/map3_10x10_3o.pomdpx -or -L layerRC_map3_10x10_sr_i$x -u layerRC_map3_sr_10x10 -A 14 > layerRC_map3_sr_10x10_i$x.log 

./hsmomdp -s localhost 1121 -sl -T 30 -gr -m pomdpx/newrew_fs_r3t1/map3_10x10_3o.pomdpx -or -L layerRC_map3_10x10_nr_i$x -u layerRC_map3_nr_10x10 -A 14 > layerRC_map3_nr_10x10_i$x.log 

./hsmomdp -s localhost 1121 -sl -T 30 -gr 5 45 -m pomdpx/simplerew_fs/map3_10x10_3o.pomdpx -or -L layerRCd5a45_map3_10x10_sr_i$x -u layerRCd5a45_map3_sr_10x10 -A 14 > layerRCd5a45_map3_sr_10x10_i$x.log 

./hsmomdp -s localhost 1121 -sl -T 30 -gr 5 45 -m pomdpx/newrew_fs_r3t1/map3_10x10_3o.pomdpx -or -L layerRCd5a45_map3_10x10_nr_i$x -u layerRCd5a45_map3_nr_10x10 -A 14 > layerRCd5a45_map3_nr_10x10_i$x.log 



  x=$(( $x + 1 ))

rm *policy
rm debug*.txt

done


