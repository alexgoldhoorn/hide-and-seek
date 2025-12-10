#!/bin/sh

x=1
while [ $x -le 100 ]
do
  echo "Run $x - map3"

./actiongen maps/map3_10x10_3o.txt map3_10x10_actions_i$x.txt map3_10x10_actions_i$x

./hsmomdp -s localhost 1120 -so -m pomdpx/simplerew_fsc/map3_10x10_3o.pomdpx -p pomdpx/simplerew_fsc/map3_10x10_3o.policy -ol map3_10x10_actions_i$x.txt -L offline_map3_10x10_src_um_i$x -u offline_map3_src_10x10_um -A 14 -um > offline_map3_src_10x10_um_i$x.log 

./hsmomdp -s localhost 1120 -sl -T 30 -gb -m pomdpx/simplerew_fsc/map3_10x10_3o.pomdpx -ol map3_10x10_actions_i$x.txt -L layerBase_map3_10x10_src_um_i$x -u layerBase_map3_src_10x10_um -A 14 -um > layerBase_map3_src_10x10_um_i$x.log 

./hsmomdp -s localhost 1120 -sl -T 30 -gr -m pomdpx/simplerew_fsc/map3_10x10_3o.pomdpx -ol map3_10x10_actions_i$x.txt -L layerRC_map3_10x10_src_um_i$x -u layerRC_map3_src_10x10_um -A 14 -um > layerRC_map3_src_10x10_um_i$x.log 

./hsmomdp -s localhost 1120 -sl -T 30 -gr 5 45 -m pomdpx/simplerew_fsc/map3_10x10_3o.pomdpx -ol map3_10x10_actions_i$x.txt -L layerRCd5a45_map3_10x10_src_um_i$x -u layerRCd5a45_map3_src_10x10_um -A 14  -um > layerRCd5a45_map3_src_10x10_um_i$x.log 

echo "map 6"

./actiongen  maps/map6_10x10.txt map6_10x10_actions_i$x.txt map6_10x10_actions_i$x

./hsmomdp -s localhost 1120 -so -m pomdpx/simplerew_fsc/map6_10x10.pomdpx -p pomdpx/simplerew_fsc/map6_10x10.policy -ol map6_10x10_actions_i$x.txt -L offline_map6_10x10_src_um_i$x -u offline_map6_src_10x10_um -A 17 -um > offline_map6_src_10x10_um_i$x.log 

./hsmomdp -s localhost 1120 -sl -T 30 -gb -m pomdpx/simplerew_fsc/map6_10x10.pomdpx -ol map6_10x10_actions_i$x.txt -L layerBase_map6_10x10_src_um_i$x -u layerBase_map6_src_10x10_um -A 17 -um > layerBase_map6_src_10x10_um_i$x.log 

./hsmomdp -s localhost 1120 -sl -T 30 -gr -m pomdpx/simplerew_fsc/map6_10x10.pomdpx -ol map6_10x10_actions_i$x.txt -L layerRC_map6_10x10_src_um_i$x -u layerRC_map6_src_10x10_um -A 17 -um > layerRC_map6_src_10x10_um_i$x.log 


./hsmomdp -s localhost 1120 -sl -T 30 -gr 5 45 -m pomdpx/simplerew_fsc/map6_10x10.pomdpx -ol map6_10x10_actions_i$x.txt -L layerRCd5a45_map6_10x10_src_um_i$x -u layerRCd5a45_map6_src_10x10_um -A 17  -um > layerRCd5a45_map6_src_10x10_um_i$x.log 


echo "map 7"

./actiongen  maps/map7_10x10.txt map7_10x10_actions_i$x.txt map7_10x10_actions_i$x

./hsmomdp -s localhost 1120 -so -m pomdpx/simplerew_fsc/map7_10x10.pomdpx -p pomdpx/simplerew_fsc/map7_10x10.policy -ol map7_10x10_actions_i$x.txt -L offline_map7_10x10_src_um_i$x -u offline_map7_src_10x10_um -A 18 -um > offline_map7_src_10x10_um_i$x.log 

./hsmomdp -s localhost 1120 -sl -T 30 -gb -m pomdpx/simplerew_fsc/map7_10x10.pomdpx -ol map7_10x10_actions_i$x.txt -L layerBase_map7_10x10_src_um_i$x -u layerBase_map7_src_10x10_um -A 18 -um > layerBase_map7_src_10x10_um_i$x.log 

./hsmomdp -s localhost 1120 -sl -T 30 -gr -m pomdpx/simplerew_fsc/map7_10x10.pomdpx -ol map7_10x10_actions_i$x.txt -L layerRC_map7_10x10_src_um_i$x -u layerRC_map7_src_10x10_um -A 18 -um > layerRC_map7_src_10x10_um_i$x.log 


./hsmomdp -s localhost 1120 -sl -T 30 -gr 5 45 -m pomdpx/simplerew_fsc/map7_10x10.pomdpx -ol map7_10x10_actions_i$x.txt -L layerRCd5a45_map7_10x10_src_um_i$x -u layerRCd5a45_map7_src_10x10_um -A 18  -um > layerRCd5a45_map7_src_10x10_um_i$x.log 


  x=$(( $x + 1 ))

rm *policy
rm debug*.txt

done


