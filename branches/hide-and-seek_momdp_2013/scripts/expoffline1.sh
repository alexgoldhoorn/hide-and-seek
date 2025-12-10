#!/bin/sh

# offline

echo "OFFLINE"

echo "6x5"
echo "simple rew fs"

appl sol_nola ../pomdpx/simplerew_fs/map3_6x5_2o.pomdpx -o map3_6x5_srw_fs.pol > map3_6x5_srw_fs.log
mv SARSOP_time_log.txt SARSOP_time_log_map3_6x5_srw_fs.txt

appl sol_nola ../pomdpx/simplerew_fs/map4_6x5_2o.pomdpx -o map4_6x5_srw_fs.pol > map4_6x5_srw_fs.log
mv SARSOP_time_log.txt SARSOP_time_log_map4_6x5_srw_fs.txt

echo "simple rew fsc"

appl sol_nola ../pomdpx/simplerew_fsc/map3_6x5_2o.pomdpx -o map3_6x5_srw_fsc.pol > map3_6x5_srw_fsc.log
mv SARSOP_time_log.txt SARSOP_time_log_map3_6x5_srw_fsc.txt

appl sol_nola ../pomdpx/simplerew_fsc/map4_6x5_2o.pomdpx -o map4_6x5_srw_fsc.pol > map4_6x5_srw_fsc.log
mv SARSOP_time_log.txt SARSOP_time_log_map4_6x5_srw_fsc.txt

echo "newrew_fs_r3t1"

appl sol_nola ../pomdpx/newrew_fs_r3t1/map3_6x5_2o.pomdpx -o map3_6x5_newrew.pol > map3_6x5_newrew.log
mv SARSOP_time_log.txt SARSOP_time_log_map3_6x5_newrew.txt

appl sol_nola ../pomdpx/newrew_fs_r3t1/map4_6x5_2o.pomdpx -o map4_6x5_newrew.pol > map4_6x5_newrew.log
mv SARSOP_time_log.txt SARSOP_time_log_map4_6x5_newrew.txt



echo "10x10"

echo "simple rew fs"

appl sol_nola ../pomdpx/simplerew_fs/map1_10x10_1o.pomdpx -o map1_10x10_srw_fs.pol --timeout 21600 --memory 4000  > map1_10x10_srw_fs.log
mv SARSOP_time_log.txt SARSOP_time_log_map1_10x10_srw_fs.txt

appl sol_nola ../pomdpx/simplerew_fs/map3_10x10_3o.pomdpx -o map3_10x10_srw_fs.pol --timeout 21600 --memory 4000 > map3_10x10_srw_fs.log
mv SARSOP_time_log.txt SARSOP_time_log_map3_10x10_srw_fs.txt

echo "simple rew fsc"

appl sol_nola ../pomdpx/simplerew_fsc/map1_10x10_1o.pomdpx -o map1_10x10_srw_fsc.pol --timeout 21600 --memory 4000 > map1_10x10_srw_fsc.log
mv SARSOP_time_log.txt SARSOP_time_log_map1_10x10_srw_fsc.txt

appl sol_nola ../pomdpx/simplerew_fsc/map3_10x10_3o.pomdpx -o map3_10x10_srw_fsc.pol --timeout 21600 --memory 4000 > map3_10x10_srw_fsc.log
mv SARSOP_time_log.txt SARSOP_time_log_map3_10x10_srw_fsc.txt

echo "newrew_fs_r3t1"

appl sol_nola ../pomdpx/newrew_fs_r3t1/map1_10x10_1o.pomdpx -o map1_10x10_newrew.pol --timeout 21600 --memory 4000 > map1_10x10_newrew.log
mv SARSOP_time_log.txt SARSOP_time_log_map1_10x10_newrew.txt

appl sol_nola ../pomdpx/newrew_fs_r3t1/map3_10x10_3o.pomdpx -o map3_10x10_newrew.pol --timeout 21600 --memory 4000 > map3_10x10_newrew.log
mv SARSOP_time_log.txt SARSOP_time_log_map3_10x10_newrew.txt



rar a -m5 ~/Dropbox/Private/HideSeek/offline_logs_121122.rar *.txt *.log


