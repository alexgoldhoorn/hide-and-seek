#!/bin/sh

x=1
while [ $x -le 1000 ]
do
  echo "Run $x - map1"
./actiongen maps/map1_6x5_0o.txt map1_6x5_actions_i$x.txt map1_6x5_actions_i$x
	
./hsmomdp -s localhost 1120 -so -m pomdpx/simplerew_fsc/map1_6x5_0o.pomdpx -p pomdpx/simplerew_fsc/map1_6x5_0o.policy -ol map1_6x5_actions_i$x.txt -L offline_map1_6x5_src_um_i$x -u offline_map1_src_6x5_um -A 0 -um > offline_map1_src_6x5_um_i$x.log 

./hsmomdp -s localhost 1120 -sl -T 30 -gb -m pomdpx/simplerew_fsc/map1_6x5_0o.pomdpx -ol map1_6x5_actions_i$x.txt -L layerBase_map1_6x5_src_um_i$x -u layerBase_map1_src_6x5_um -A 0 -um > layerBase_map1_src_6x5_um_i$x.log 

./hsmomdp -s localhost 1120 -sl -T 30 -gr -m pomdpx/simplerew_fsc/map1_6x5_0o.pomdpx -ol map1_6x5_actions_i$x.txt -L layerRC_map1_6x5_src_um_i$x -u layerRC_map1_src_6x5_um -A 0 -um > layerRC_map1_src_6x5_um_i$x.log 

./hsmomdp -s localhost 1120 -sl -T 30 -gr 5 45 -m pomdpx/simplerew_fsc/map1_6x5_0o.pomdpx -ol map1_6x5_actions_i$x.txt -L layerRCd5a45_map1_6x5_src_um_i$x -u layerRCd5a45_map1_src_6x5_um -A 0  -um > layerRCd5a45_map1_src_6x5_um_i$x.log 

  ./hsmomdp -bl -s localhost 1120 -so -m pomdpx/simplerew_fsc/map1_6x5_0o.pomdpx -p pomdpx/simplerew_fsc/map1_6x5_0o.policy -ol map1_6x5_actions_i$x.txt -L offline_map1_6x5_src_um_abl_i$x -u offline_map1_src_6x5_um_abl -A 0 -um > offline_map1_src_6x5_um_abl_i$x.log 

./hsmomdp -bl -s localhost 1120 -sl -T 30 -gb -m pomdpx/simplerew_fsc/map1_6x5_0o.pomdpx -ol map1_6x5_actions_i$x.txt -L layerBase_map1_6x5_src_um_abl_i$x -u layerBase_map1_src_6x5_umv -A 0 -um > layerBase_map1_src_6x5_um_abl_i$x.log 

./hsmomdp -bl -s localhost 1120 -sl -T 30 -gr -m pomdpx/simplerew_fsc/map1_6x5_0o.pomdpx -ol map1_6x5_actions_i$x.txt -L layerRC_map1_6x5_src_um_abl_i$x -u layerRC_map1_src_6x5_um_abl -A 0 -um > layerRC_map1_src_6x5_um_abl_i$x.log 

./hsmomdp -bl -s localhost 1120 -sl -T 30 -gr 5 45 -m pomdpx/simplerew_fsc/map1_6x5_0o.pomdpx -ol map1_6x5_actions_i$x.txt -L layerRCd5a45_map1_6x5_src_um_abl_i$x -u layerRCd5a45_map1_src_6x5_um_abl -A 0  -um > layerRCd5a45_map1_src_6x5_um_abl_i$x.log 


  echo "map 2"
  ./actiongen maps/map2_6x5_1o.txt map2_6x5_actions_i$x.txt map2_6x5_actions_i$x

./hsmomdp -s localhost 1120 -so -m pomdpx/simplerew_fsc/map2_6x5_1o.pomdpx -p pomdpx/simplerew_fsc/map2_6x5_1o.policy -ol map2_6x5_actions_i$x.txt -L offline_map2_6x5_src_um_i$x -u offline_map2_src_6x5_um -A 1 -um > offline_map2_src_6x5_um_i$x.log 

./hsmomdp -s localhost 1120 -sl -T 30 -gb -m pomdpx/simplerew_fsc/map2_6x5_1o.pomdpx -ol map2_6x5_actions_i$x.txt -L layerBase_map2_6x5_src_um_i$x -u layerBase_map2_src_6x5_um -A 1 -um > layerBase_map2_src_6x5_um_i$x.log 

./hsmomdp -s localhost 1120 -sl -T 30 -gr -m pomdpx/simplerew_fsc/map2_6x5_1o.pomdpx -ol map2_6x5_actions_i$x.txt -L layerRC_map2_6x5_src_um_i$x -u layerRC_map2_src_6x5_um -A 1 -um > layerRC_map2_src_6x5_um_i$x.log 

./hsmomdp -s localhost 1120 -sl -T 30 -gr 5 45 -m pomdpx/simplerew_fsc/map2_6x5_1o.pomdpx -ol map2_6x5_actions_i$x.txt -L layerRCd5a45_map2_6x5_src_um_i$x -u layerRCd5a45_map2_src_6x5_um -A 1  -um > layerRCd5a45_map2_src_6x5_um_i$x.log 

  
./hsmomdp -bl -s localhost 1120 -so -m pomdpx/simplerew_fsc/map2_6x5_1o.pomdpx -p pomdpx/simplerew_fsc/map2_6x5_1o.policy -ol map2_6x5_actions_i$x.txt -L offline_map2_6x5_src_um_abl_i$x -u offline_map2_src_6x5_um_abl -A 1 -um > offline_map2_src_6x5_um_abl_i$x.log 

./hsmomdp -bl -s localhost 1120 -sl -T 30 -gb -m pomdpx/simplerew_fsc/map2_6x5_1o.pomdpx -ol map2_6x5_actions_i$x.txt -L layerBase_map2_6x5_src_um_abl_i$x -u layerBase_map2_src_6x5_um_abl -A 1 -um > layerBase_map2_src_6x5_um_abl_i$x.log 

./hsmomdp -bl -s localhost 1120 -sl -T 30 -gr -m pomdpx/simplerew_fsc/map2_6x5_1o.pomdpx -ol map2_6x5_actions_i$x.txt -L layerRC_map2_6x5_src_um_abl_i$x -u layerRC_map2_src_6x5_um_abl -A 1 -um > layerRC_map2_src_6x5_um_abl_i$x.log 

./hsmomdp -bl -s localhost 1120 -sl -T 30 -gr 5 45 -m pomdpx/simplerew_fsc/map2_6x5_1o.pomdpx -ol map2_6x5_actions_i$x.txt -L layerRCd5a45_map2_6x5_src_um_abl_i$x -u layerRCd5a45_map2_src_6x5_um_abl -A 1  -um > layerRCd5a45_map2_src_6x5_um_abl_i$x.log 



  echo "map 3"
  
./actiongen maps/map3_6x5_2o.txt map3_6x5_actions_i$x.txt map3_6x5_actions_i$x

./hsmomdp -s localhost 1120 -so -m pomdpx/simplerew_fsc/map3_6x5_2o.pomdpx -p pomdpx/simplerew_fsc/map3_6x5_2o.policy -ol map3_6x5_actions_i$x.txt -L offline_map3_6x5_src_um_i$x -u offline_map3_src_6x5_um -A 2 -um > offline_map3_src_6x5_um_i$x.log 

./hsmomdp -s localhost 1120 -sl -T 30 -gb -m pomdpx/simplerew_fsc/map3_6x5_2o.pomdpx -ol map3_6x5_actions_i$x.txt -L layerBase_map3_6x5_src_um_i$x -u layerBase_map3_src_6x5_um -A 2 -um > layerBase_map3_src_6x5_um_i$x.log 

./hsmomdp -s localhost 1120 -sl -T 30 -gr -m pomdpx/simplerew_fsc/map3_6x5_2o.pomdpx -ol map3_6x5_actions_i$x.txt -L layerRC_map3_6x5_src_um_i$x -u layerRC_map3_src_6x5_um -A 2 -um > layerRC_map3_src_6x5_um_i$x.log 

./hsmomdp -s localhost 1120 -sl -T 30 -gr 5 45 -m pomdpx/simplerew_fsc/map3_6x5_2o.pomdpx -ol map3_6x5_actions_i$x.txt -L layerRCd5a45_map3_6x5_src_um_i$x -u layerRCd5a45_map3_src_6x5_um -A 2  -um > layerRCd5a45_map3_src_6x5_um_i$x.log 

./hsmomdp -bl -s localhost 1120 -so -m pomdpx/simplerew_fsc/map3_6x5_2o.pomdpx -p pomdpx/simplerew_fsc/map3_6x5_2o.policy -ol map3_6x5_actions_i$x.txt -L offline_map3_6x5_src_um_abl_i$x -u offline_map3_src_6x5_um_abl -A 2 -um > offline_map3_src_6x5_um_abl_i$x.log 

./hsmomdp -bl -s localhost 1120 -sl -T 30 -gb -m pomdpx/simplerew_fsc/map3_6x5_2o.pomdpx -ol map3_6x5_actions_i$x.txt -L layerBase_map3_6x5_src_um_abl_i$x -u layerBase_map3_src_6x5_um_abl -A 2 -um > layerBase_map3_src_6x5_um_abl_i$x.log 

./hsmomdp -bl -s localhost 1120 -sl -T 30 -gr -m pomdpx/simplerew_fsc/map3_6x5_2o.pomdpx -ol map3_6x5_actions_i$x.txt -L layerRC_map3_6x5_src_um_abl_i$x -u layerRC_map3_src_6x5_um_abl -A 2 -um > layerRC_map3_src_6x5_um_abl_i$x.log 

./hsmomdp -bl -s localhost 1120 -sl -T 30 -gr 5 45 -m pomdpx/simplerew_fsc/map3_6x5_2o.pomdpx -ol map3_6x5_actions_i$x.txt -L layerRCd5a45_map3_6x5_src_um_abl_i$x -u layerRCd5a45_map3_src_6x5_um_abl -A 2  -um > layerRCd5a45_map3_src_6x5_um_abl_i$x.log 



echo "map 4"

./actiongen maps/map4_6x5_2o.txt map4_6x5_actions_i$x.txt map4_6x5_actions_i$x

./hsmomdp -s localhost 1120 -so -m pomdpx/simplerew_fsc/map4_6x5_2o.pomdpx -p pomdpx/simplerew_fsc/map4_6x5_2o.policy -ol map4_6x5_actions_i$x.txt -L offline_map4_6x5_src_um_i$x -u offline_map4_src_6x5_um -A 3 -um > offline_map4_src_6x5_um_i$x.log 

./hsmomdp -s localhost 1120 -sl -T 30 -gb -m pomdpx/simplerew_fsc/map4_6x5_2o.pomdpx -ol map4_6x5_actions_i$x.txt -L layerBase_map4_6x5_src_um_i$x -u layerBase_map4_src_6x5_um -A 3 -um > layerBase_map4_src_6x5_um_i$x.log 

./hsmomdp -s localhost 1120 -sl -T 30 -gr -m pomdpx/simplerew_fsc/map4_6x5_2o.pomdpx -ol map4_6x5_actions_i$x.txt -L layerRC_map4_6x5_src_um_i$x -u layerRC_map4_src_6x5_um -A 3 -um > layerRC_map4_src_6x5_um_i$x.log 

./hsmomdp -s localhost 1120 -sl -T 30 -gr 5 45 -m pomdpx/simplerew_fsc/map4_6x5_2o.pomdpx -ol map4_6x5_actions_i$x.txt -L layerRCd5a45_map4_6x5_src_um_i$x -u layerRCd5a45_map4_src_6x5_um -A 3  -um > layerRCd5a45_map4_src_6x5_um_i$x.log 


./hsmomdp -bl -s localhost 1120 -so -m pomdpx/simplerew_fsc/map4_6x5_2o.pomdpx -p pomdpx/simplerew_fsc/map4_6x5_2o.policy -ol map4_6x5_actions_i$x.txt -L offline_map4_6x5_src_um_abl_i$x -u offline_map4_src_6x5_um_abl -A 3 -um > offline_map4_src_6x5_um_abl_i$x.log 

./hsmomdp -bl -s localhost 1120 -sl -T 30 -gb -m pomdpx/simplerew_fsc/map4_6x5_2o.pomdpx -ol map4_6x5_actions_i$x.txt -L layerBase_map4_6x5_src_um_abl_i$x -u layerBase_map4_src_6x5_um_abl -A 3 -um > layerBase_map4_src_6x5_um_abl_i$x.log 

./hsmomdp -bl -s localhost 1120 -sl -T 30 -gr -m pomdpx/simplerew_fsc/map4_6x5_2o.pomdpx -ol map4_6x5_actions_i$x.txt -L layerRC_map4_6x5_src_um_abl_i$x -u layerRC_map4_src_6x5_um_abl -A 3 -um > layerRC_map4_src_6x5_um_abl_i$x.log 

./hsmomdp -bl -s localhost 1120 -sl -T 30 -gr 5 45 -m pomdpx/simplerew_fsc/map4_6x5_2o.pomdpx -ol map4_6x5_actions_i$x.txt -L layerRCd5a45_map4_6x5_src_um_abl_i$x -u layerRCd5a45_map4_src_6x5_um_abl -A 3  -um > layerRCd5a45_map4_src_6x5_um_abl_i$x.log 



echo "map 5"

./actiongen maps/map5_6x5_4o.txt map5_6x5_actions_i$x.txt map5_6x5_actions_i$x

./hsmomdp -s localhost 1120 -so -m pomdpx/simplerew_fsc/map5_6x5_4o.pomdpx -p pomdpx/simplerew_fsc/map5_6x5_4o.policy -ol map5_6x5_actions_i$x.txt -L offline_map5_6x5_src_um_i$x -u offline_map5_src_6x5_um -A 4 -um > offline_map5_src_6x5_um_i$x.log 

./hsmomdp -s localhost 1120 -sl -T 30 -gb -m pomdpx/simplerew_fsc/map5_6x5_4o.pomdpx -ol map5_6x5_actions_i$x.txt -L layerBase_map5_6x5_src_um_i$x -u layerBase_map5_src_6x5_um -A 4 -um > layerBase_map5_src_6x5_um_i$x.log 

./hsmomdp -s localhost 1120 -sl -T 30 -gr -m pomdpx/simplerew_fsc/map5_6x5_4o.pomdpx -ol map5_6x5_actions_i$x.txt -L layerRC_map5_6x5_src_um_i$x -u layerRC_map5_src_6x5_um -A 4 -um > layerRC_map5_src_6x5_um_i$x.log 

./hsmomdp -s localhost 1120 -sl -T 30 -gr 5 45 -m pomdpx/simplerew_fsc/map5_6x5_4o.pomdpx -ol map5_6x5_actions_i$x.txt -L layerRCd5a45_map5_6x5_src_um_i$x -u layerRCd5a45_map5_src_6x5_um -A 4  -um > layerRCd5a45_map5_src_6x5_um_i$x.log 


./hsmomdp -bl -s localhost 1120 -so -m pomdpx/simplerew_fsc/map5_6x5_4o.pomdpx -p pomdpx/simplerew_fsc/map5_6x5_4o.policy -ol map5_6x5_actions_i$x.txt -L offline_map5_6x5_src_um_abl_i$x -u offline_map5_src_6x5_um_abl -A 4 -um > offline_map5_src_6x5_um_abl_i$x.log 

./hsmomdp -bl -s localhost 1120 -sl -T 30 -gb -m pomdpx/simplerew_fsc/map5_6x5_4o.pomdpx -ol map5_6x5_actions_i$x.txt -L layerBase_map5_6x5_src_um_abl_i$x -u layerBase_map5_src_6x5_um_abl -A 4 -um > layerBase_map5_src_6x5_um_abl_i$x.log 

./hsmomdp -bl -s localhost 1120 -sl -T 30 -gr -m pomdpx/simplerew_fsc/map5_6x5_4o.pomdpx -ol map5_6x5_actions_i$x.txt -L layerRC_map5_6x5_src_um_abl_i$x -u layerRC_map5_src_6x5_um_abl -A 4 -um > layerRC_map5_src_6x5_um_abl_i$x.log 

./hsmomdp -bl -s localhost 1120 -sl -T 30 -gr 5 45 -m pomdpx/simplerew_fsc/map5_6x5_4o.pomdpx -ol map5_6x5_actions_i$x.txt -L layerRCd5a45_map5_6x5_src_um_abl_i$x -u layerRCd5a45_map5_src_6x5_um_abl -A 4  -um > layerRCd5a45_map5_src_6x5_um_abl_i$x.log 


echo "Run $x - map3"

./actiongen maps/map3_10x10_3o.txt map3_10x10_actions_i$x.txt map3_10x10_actions_i$x

./hsmomdp -s localhost 1120 -so -m pomdpx/simplerew_fsc/map3_10x10_3o.pomdpx -p pomdpx/simplerew_fsc/map3_10x10_3o.policy -ol map3_10x10_actions_i$x.txt -L offline_map3_10x10_src_um_i$x -u offline_map3_src_10x10_um -A 14 -um > offline_map3_src_10x10_um_i$x.log 

./hsmomdp -s localhost 1120 -sl -T 30 -gb -m pomdpx/simplerew_fsc/map3_10x10_3o.pomdpx -ol map3_10x10_actions_i$x.txt -L layerBase_map3_10x10_src_um_i$x -u layerBase_map3_src_10x10_um -A 14 -um > layerBase_map3_src_10x10_um_i$x.log 

./hsmomdp -s localhost 1120 -sl -T 30 -gr -m pomdpx/simplerew_fsc/map3_10x10_3o.pomdpx -ol map3_10x10_actions_i$x.txt -L layerRC_map3_10x10_src_um_i$x -u layerRC_map3_src_10x10_um -A 14 -um > layerRC_map3_src_10x10_um_i$x.log 

./hsmomdp -s localhost 1120 -sl -T 30 -gr 5 45 -m pomdpx/simplerew_fsc/map3_10x10_3o.pomdpx -ol map3_10x10_actions_i$x.txt -L layerRCd5a45_map3_10x10_src_um_i$x -u layerRCd5a45_map3_src_10x10_um -A 14  -um > layerRCd5a45_map3_src_10x10_um_i$x.log 

./hsmomdp -bl -s localhost 1120 -so -m pomdpx/simplerew_fsc/map3_10x10_3o.pomdpx -p pomdpx/simplerew_fsc/map3_10x10_3o.policy -ol map3_10x10_actions_i$x.txt -L offline_map3_10x10_src_um_abl_i$x -u offline_map3_src_10x10_um_abl -A 14 -um > offline_map3_src_10x10_um_abl_i$x.log 

./hsmomdp -bl -s localhost 1120 -sl -T 30 -gb -m pomdpx/simplerew_fsc/map3_10x10_3o.pomdpx -ol map3_10x10_actions_i$x.txt -L layerBase_map3_10x10_src_um_abl_i$x -u layerBase_map3_src_10x10_um_abl -A 14 -um > layerBase_map3_src_10x10_um_abl_i$x.log 

./hsmomdp -bl -s localhost 1120 -sl -T 30 -gr -m pomdpx/simplerew_fsc/map3_10x10_3o.pomdpx -ol map3_10x10_actions_i$x.txt -L layerRC_map3_10x10_src_um_abl_i$x -u layerRC_map3_src_10x10_um_abl -A 14 -um > layerRC_map3_src_10x10_um_abl_i$x.log 

./hsmomdp -bl -s localhost 1120 -sl -T 30 -gr 5 45 -m pomdpx/simplerew_fsc/map3_10x10_3o.pomdpx -ol map3_10x10_actions_i$x.txt -L layerRCd5a45_map3_10x10_src_um_abl_i$x -u layerRCd5a45_map3_src_10x10_um_abl -A 14  -um > layerRCd5a45_map3_src_10x10_um_abl_i$x.log 


echo "map 6"

./actiongen  maps/map6_10x10.txt map6_10x10_actions_i$x.txt map6_10x10_actions_i$x

./hsmomdp -s localhost 1120 -so -m pomdpx/simplerew_fsc/map6_10x10.pomdpx -p pomdpx/simplerew_fsc/map6_10x10.policy -ol map6_10x10_actions_i$x.txt -L offline_map6_10x10_src_um_i$x -u offline_map6_src_10x10_um -A 17 -um > offline_map6_src_10x10_um_i$x.log 

./hsmomdp -s localhost 1120 -sl -T 30 -gb -m pomdpx/simplerew_fsc/map6_10x10.pomdpx -ol map6_10x10_actions_i$x.txt -L layerBase_map6_10x10_src_um_i$x -u layerBase_map6_src_10x10_um -A 17 -um > layerBase_map6_src_10x10_um_i$x.log 

./hsmomdp -s localhost 1120 -sl -T 30 -gr -m pomdpx/simplerew_fsc/map6_10x10.pomdpx -ol map6_10x10_actions_i$x.txt -L layerRC_map6_10x10_src_um_i$x -u layerRC_map6_src_10x10_um -A 17 -um > layerRC_map6_src_10x10_um_i$x.log 


./hsmomdp -s localhost 1120 -sl -T 30 -gr 5 45 -m pomdpx/simplerew_fsc/map6_10x10.pomdpx -ol map6_10x10_actions_i$x.txt -L layerRCd5a45_map6_10x10_src_um_i$x -u layerRCd5a45_map6_src_10x10_um -A 17  -um > layerRCd5a45_map6_src_10x10_um_i$x.log 

./hsmomdp -bl -s localhost 1120 -so -m pomdpx/simplerew_fsc/map6_10x10.pomdpx -p pomdpx/simplerew_fsc/map6_10x10.policy -ol map6_10x10_actions_i$x.txt -L offline_map6_10x10_src_um_abl_i$x -u offline_map6_src_10x10_um_abl -A 17 -um > offline_map6_src_10x10_um_abl_i$x.log 

./hsmomdp -bl -s localhost 1120 -sl -T 30 -gb -m pomdpx/simplerew_fsc/map6_10x10.pomdpx -ol map6_10x10_actions_i$x.txt -L layerBase_map6_10x10_src_um_abl_i$x -u layerBase_map6_src_10x10_um_abl -A 17 -um > layerBase_map6_src_10x10_um_abl_i$x.log 

./hsmomdp -bl -s localhost 1120 -sl -T 30 -gr -m pomdpx/simplerew_fsc/map6_10x10.pomdpx -ol map6_10x10_actions_i$x.txt -L layerRC_map6_10x10_src_um_abl_i$x -u layerRC_map6_src_10x10_um_abl -A 17 -um > layerRC_map6_src_10x10_um_abl_i$x.log 


./hsmomdp -bl -s localhost 1120 -sl -T 30 -gr 5 45 -m pomdpx/simplerew_fsc/map6_10x10.pomdpx -ol map6_10x10_actions_i$x.txt -L layerRCd5a45_map6_10x10_src_um_abl_i$x -u layerRCd5a45_map6_src_10x10_um_abl -A 17  -um > layerRCd5a45_map6_src_10x10_um_abl_i$x.log 



echo "map 7"

./actiongen  maps/map7_10x10.txt map7_10x10_actions_i$x.txt map7_10x10_actions_i$x

./hsmomdp -s localhost 1120 -so -m pomdpx/simplerew_fsc/map7_10x10.pomdpx -p pomdpx/simplerew_fsc/map7_10x10.policy -ol map7_10x10_actions_i$x.txt -L offline_map7_10x10_src_um_i$x -u offline_map7_src_10x10_um -A 18 -um > offline_map7_src_10x10_um_i$x.log 

./hsmomdp -s localhost 1120 -sl -T 30 -gb -m pomdpx/simplerew_fsc/map7_10x10.pomdpx -ol map7_10x10_actions_i$x.txt -L layerBase_map7_10x10_src_um_i$x -u layerBase_map7_src_10x10_um -A 18 -um > layerBase_map7_src_10x10_um_i$x.log 

./hsmomdp -s localhost 1120 -sl -T 30 -gr -m pomdpx/simplerew_fsc/map7_10x10.pomdpx -ol map7_10x10_actions_i$x.txt -L layerRC_map7_10x10_src_um_i$x -u layerRC_map7_src_10x10_um -A 18 -um > layerRC_map7_src_10x10_um_i$x.log 


./hsmomdp -s localhost 1120 -sl -T 30 -gr 5 45 -m pomdpx/simplerew_fsc/map7_10x10.pomdpx -ol map7_10x10_actions_i$x.txt -L layerRCd5a45_map7_10x10_src_um_i$x -u layerRCd5a45_map7_src_10x10_um -A 18  -um > layerRCd5a45_map7_src_10x10_um_i$x.log 


./hsmomdp -bl -s localhost 1120 -so -m pomdpx/simplerew_fsc/map7_10x10.pomdpx -p pomdpx/simplerew_fsc/map7_10x10.policy -ol map7_10x10_actions_i$x.txt -L offline_map7_10x10_src_um_abl_i$x -u offline_map7_src_10x10_um_abl -A 18 -um > offline_map7_src_10x10_um_i$x.log 

./hsmomdp -bl -s localhost 1120 -sl -T 30 -gb -m pomdpx/simplerew_fsc/map7_10x10.pomdpx -ol map7_10x10_actions_i$x.txt -L layerBase_map7_10x10_src_um_abl_i$x -u layerBase_map7_src_10x10_um_abl -A 18 -um > layerBase_map7_src_10x10_um_abl_i$x.log 

./hsmomdp -bl -s localhost 1120 -sl -T 30 -gr -m pomdpx/simplerew_fsc/map7_10x10.pomdpx -ol map7_10x10_actions_i$x.txt -L layerRC_map7_10x10_src_um_abl_i$x -u layerRC_map7_src_10x10_um_abl -A 18 -um > layerRC_map7_src_10x10_um_abl_i$x.log 


./hsmomdp -bl -s localhost 1120 -sl -T 30 -gr 5 45 -m pomdpx/simplerew_fsc/map7_10x10.pomdpx -ol map7_10x10_actions_i$x.txt -L layerRCd5a45_map7_10x10_src_um_abl_i$x -u layerRCd5a45_map7_src_10x10_um_abl -A 18  -um > layerRCd5a45_map7_src_10x10_um_abl_i$x.log 


  x=$(( $x + 1 ))

rm *policy
rm debug*.txt

sleep 100

done


