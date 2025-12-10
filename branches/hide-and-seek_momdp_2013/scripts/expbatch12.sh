#!/bin/sh

x=1
while [ $x -le 100 ]
do
  echo "Run $x "

./actiongen map3_10x10_3o.txt map3_10x10_actions_i$x.txt map3_10x10_actions_i$x

./hsmomdp -s localhost 1120 -so -m pomdpx/simplerew_fs/map3_10x10_3o.pomdpx -p pomdpx/simplerew_fs/map3_10x10_3o.policy -ol map3_10x10_actions_i$x.txt -L offline_map3_10x10_sr_um_i$x -u offline_map3_sr_10x10_um -A 14 -um > offline_map3_sr_10x10_um_i$x.log 

./hsmomdp -s localhost 1120 -so -m pomdpx/newrew_fs_r3t1/map3_10x10_3o.pomdpx -p pomdpx/newrew_fs_r3t1/map3_10x10_3o.policy -ol map3_10x10_actions_i$x.txt -L offline_map3_10x10_nr_um_i$x -u offline_map3_nr_10x10_um -A 14 -um > offline_map3_nr_10x10_um_i$x.log 

./hsmomdp -s localhost 1120 -sl -T 30 -gb -m pomdpx/simplerew_fs/map3_10x10_3o.pomdpx -ol map3_10x10_actions_i$x.txt -L layerBase_map3_10x10_sr_um_i$x -u layerBase_map3_sr_10x10_um -A 14 -um > layerBase_map3_sr_10x10_um_i$x.log 

./hsmomdp -s localhost 1120 -sl -T 30 -gb -m pomdpx/newrew_fs_r3t1/map3_10x10_3o.pomdpx -ol map3_10x10_actions_i$x.txt -L layerBase_map3_10x10_nr_um_i$x -u layerBase_map3_nr_10x10_um -A 14 -um > layerBase_map3_nr_10x10_um_i$x.log 

./hsmomdp -s localhost 1120 -sl -T 30 -gr -m pomdpx/simplerew_fs/map3_10x10_3o.pomdpx -ol map3_10x10_actions_i$x.txt -L layerRC_map3_10x10_sr_um_i$x -u layerRC_map3_sr_10x10_um -A 14 -um > layerRC_map3_sr_10x10_um_i$x.log 

./hsmomdp -s localhost 1120 -sl -T 30 -gr -m pomdpx/newrew_fs_r3t1/map3_10x10_3o.pomdpx -ol map3_10x10_actions_i$x.txt -L layerRC_map3_10x10_nr_um_i$x -u layerRC_map3_nr_10x10_um -A 14 -um > layerRC_map3_nr_10x10_um_i$x.log 

./hsmomdp -s localhost 1120 -sl -T 30 -gr 5 45 -m pomdpx/simplerew_fs/map3_10x10_3o.pomdpx -ol map3_10x10_actions_i$x.txt -L layerRCd5a45_map3_10x10_sr_um_i$x -u layerRCd5a45_map3_sr_10x10_um -A 14  -um > layerRCd5a45_map3_sr_10x10_um_i$x.log 

./hsmomdp -s localhost 1120 -sl -T 30 -gr 5 45 -m pomdpx/newrew_fs_r3t1/map3_10x10_3o.pomdpx -ol map3_10x10_actions_i$x.txt -L layerRCd5a45_map3_10x10_um_nr_i$x -u layerRCd5a45_map3_nr_10x10_um -A 14 -um > layerRCd5a45_map3_nr_10x10_um_i$x.log 



  x=$(( $x + 1 ))

rm *policy
rm debug*.txt

done


