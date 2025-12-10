#!/bin/sh

x=1
while [ $x -le 100 ]
do
  echo "Run $x "


./hsmomdp -so -m pomdpx/simplerew_fs/map5_6x5_3o.pomdpx -p pomdpx/simplerew_fs/map5_6x5_3o.policy -or -L offline_map5_6x5_sr_i$x -u offline_map5_sr_6x5 -A 4 > offline_map5_sr_6x5_i$x.log 

./hsmomdp -so -m pomdpx/newrew_fs_r3t1/map5_6x5_3o.pomdpx -p pomdpx/newrew_fs_r3t1/map5_6x5_3o.policy -or -L offline_map5_6x5_nr_i$x -u offline_map5_nr_6x5 -A 4 > offline_map5_nr_6x5_i$x.log 

./hsmomdp -sl -T 30 -gb -m pomdpx/simplerew_fs/map5_6x5_3o.pomdpx -or -L layerBase_map5_6x5_sr_i$x -u layerBase_map5_sr_6x5 -A 4 > layerBase_map5_sr_6x5_i$x.log 

./hsmomdp -sl -T 30 -gb -m pomdpx/newrew_fs_r3t1/map5_6x5_3o.pomdpx -or -L layerBase_map5_6x5_nr_i$x -u layerBase_map5_nr_6x5 -A 4 > layerBase_map5_nr_6x5_i$x.log 

./hsmomdp -sl -T 30 -gk 6 -m pomdpx/simplerew_fs/map5_6x5_3o.pomdpx -or -L layerK6_map5_6x5_sr_i$x -u layerK6_map5_sr_6x5 -A 4 > layerK6_map5_sr_6x5_i$x.log 

./hsmomdp -sl -T 30 -gk 6 -m pomdpx/newrew_fs_r3t1/map5_6x5_3o.pomdpx -or -L layerK6_map5_6x5_nr_i$x -u layerK6_map5_nr_6x5 -A 4 > layerK6_map5_nr_6x5_i$x.log 


./hsmomdp -sl -T 30 -gr -m pomdpx/simplerew_fs/map5_6x5_3o.pomdpx -or -L layerRC_map5_6x5_sr_i$x -u layerRC_map5_sr_6x5 -A 4 > layerRC_map5_sr_6x5_i$x.log 

./hsmomdp -sl -T 30 -gr -m pomdpx/newrew_fs_r3t1/map5_6x5_3o.pomdpx -or -L layerRC_map5_6x5_nr_i$x -u layerRC_map5_nr_6x5 -A 4 > layerRC_map5_nr_6x5_i$x.log 

./hsmomdp -sl -T 30 -gr 10 45 -m pomdpx/simplerew_fs/map5_6x5_3o.pomdpx -or -L layerRCd10a45_map5_6x5_sr_i$x -u layerRCd10a45_map5_sr_6x5 -A 4 > layerRCd10a45_map5_sr_6x5_i$x.log 

./hsmomdp -sl -T 30 -gr 10 45 -m pomdpx/newrew_fs_r3t1/map5_6x5_3o.pomdpx -or -L layerRCd10a45_map5_6x5_nr_i$x -u layerRCd10a45_map5_nr_6x5 -A 4 > layerRCd10a45_map5_nr_6x5_i$x.log 

echo " - "


./hsmomdp -so -m pomdpx/simplerew_fs/map1_6x5_0o.pomdpx -p pomdpx/simplerew_fs/map1_6x5_0o.policy -or -L offline_map1_6x5_sr_i$x -u offline_map1_sr_6x5 -A 0 > offline_map1_sr_6x5_i$x.log 

./hsmomdp -so -m pomdpx/newrew_fs_r3t1/map1_6x5_0o.pomdpx -p pomdpx/newrew_fs_r3t1/map1_6x5_0o.policy -or -L offline_map1_6x5_nr_i$x -u offline_map1_nr_6x5 -A 0 > offline_map1_nr_6x5_i$x.log 

./hsmomdp -sl -T 30 -gb -m pomdpx/simplerew_fs/map1_6x5_0o.pomdpx -or -L layerBase_map1_6x5_sr_i$x -u layerBase_map1_sr_6x5 -A 0 > layerBase_map1_sr_6x5_i$x.log 

./hsmomdp -sl -T 30 -gb -m pomdpx/newrew_fs_r3t1/map1_6x5_0o.pomdpx -or -L layerBase_map1_6x5_nr_i$x -u layerBase_map1_nr_6x5 -A 0 > layerBase_map1_nr_6x5_i$x.log 

./hsmomdp -sl -T 30 -gk 6 -m pomdpx/simplerew_fs/map1_6x5_0o.pomdpx -or -L layerK6_map1_6x5_sr_i$x -u layerK6_map1_sr_6x5 -A 0 > layerK6_map1_sr_6x5_i$x.log 

./hsmomdp -sl -T 30 -gk 6 -m pomdpx/newrew_fs_r3t1/map1_6x5_0o.pomdpx -or -L layerK6_map1_6x5_nr_i$x -u layerK6_map1_nr_6x5 -A 0 > layerK6_map1_nr_6x5_i$x.log 


./hsmomdp -sl -T 30 -gr -m pomdpx/simplerew_fs/map1_6x5_0o.pomdpx -or -L layerRC_map1_6x5_sr_i$x -u layerRC_map1_sr_6x5 -A 0 > layerRC_map1_sr_6x5_i$x.log 

./hsmomdp -sl -T 30 -gr -m pomdpx/newrew_fs_r3t1/map1_6x5_0o.pomdpx -or -L layerRC_map1_6x5_nr_i$x -u layerRC_map1_nr_6x5 -A 0 > layerRC_map1_nr_6x5_i$x.log 

./hsmomdp -sl -T 30 -gr 10 45 -m pomdpx/simplerew_fs/map1_6x5_0o.pomdpx -or -L layerRCd10a45_map1_6x5_sr_i$x -u layerRCd10a45_map1_sr_6x5 -A 0 > layerRCd10a45_map1_sr_6x5_i$x.log 

./hsmomdp -sl -T 30 -gr 10 45 -m pomdpx/newrew_fs_r3t1/map1_6x5_0o.pomdpx -or -L layerRCd10a45_map1_6x5_nr_i$x -u layerRCd10a45_map1_nr_6x5 -A 0 > layerRCd10a45_map1_nr_6x5_i$x.log 


  x=$(( $x + 1 ))

rm *policy
rm debug*.txt

done


