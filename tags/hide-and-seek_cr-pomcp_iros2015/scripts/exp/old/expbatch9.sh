#!/bin/sh

x=1
while [ $x -le 100 ]
do
  echo "Run $x "

./hsmomdp -so -m pomdpx/simplerew_fs/map3_6x5_2o.pomdpx -p pomdpx/simplerew_fs/map3_6x5_2o.policy -or -L offline_map3_6x5_sr_i$x -u offline_map3_sr_6x5 -A 2 > offline_map3_sr_6x5_i$x.log 

./hsmomdp -so -m pomdpx/newrew_fs_r3t1/map3_6x5_2o.pomdpx -p pomdpx/newrew_fs_r3t1/map3_6x5_2o.policy -or -L offline_map3_6x5_nr_i$x -u offline_map3_nr_6x5 -A 2 > offline_map3_nr_6x5_i$x.log 

./hsmomdp -sl -T 30 -gb -m pomdpx/simplerew_fs/map3_6x5_2o.pomdpx -or -L layerBase_map3_6x5_sr_i$x -u layerBase_map3_sr_6x5 -A 2 > layerBase_map3_sr_6x5_i$x.log 

./hsmomdp -sl -T 30 -gb -m pomdpx/newrew_fs_r3t1/map3_6x5_2o.pomdpx -or -L layerBase_map3_6x5_nr_i$x -u layerBase_map3_nr_6x5 -A 2 > layerBase_map3_nr_6x5_i$x.log 

./hsmomdp -sk -T 30 -gk 6 -m pomdpx/simplerew_fs/map3_6x5_2o.pomdpx -or -L layerK6_map3_6x5_sr_i$x -u layerK6_map3_sr_6x5 -A 2 > layerK6_map3_sr_6x5_i$x.log 

./hsmomdp -sk -T 30 -gk 6 -m pomdpx/newrew_fs_r3t1/map3_6x5_2o.pomdpx -or -L layerK6_map3_6x5_nr_i$x -u layerK6_map3_nr_6x5 -A 2 > layerK6_map3_nr_6x5_i$x.log 


./hsmomdp -sl -T 30 -gr -m pomdpx/simplerew_fs/map3_6x5_2o.pomdpx -or -L layerRC_map3_6x5_sr_i$x -u layerRC_map3_sr_6x5 -A 2 > layerRC_map3_sr_6x5_i$x.log 

./hsmomdp -sl -T 30 -gr -m pomdpx/newrew_fs_r3t1/map3_6x5_2o.pomdpx -or -L layerRC_map3_6x5_nr_i$x -u layerRC_map3_nr_6x5 -A 2 > layerRC_map3_nr_6x5_i$x.log 

./hsmomdp -sl -T 30 -gr 10 45 -m pomdpx/simplerew_fs/map3_6x5_2o.pomdpx -or -L layerRCd10a45_map3_6x5_sr_i$x -u layerRCd10a45_map3_sr_6x5 -A 2 > layerRCd10a45_map3_sr_6x5_i$x.log 

./hsmomdp -sl -T 30 -gr 10 45 -m pomdpx/newrew_fs_r3t1/map3_6x5_2o.pomdpx -or -L layerRCd10a45_map3_6x5_nr_i$x -u layerRCd10a45_map3_nr_6x5 -A 2 > layerRCd10a45_map3_nr_6x5_i$x.log 



  x=$(( $x + 1 ))

rm *policy
rm debug*.txt

done


