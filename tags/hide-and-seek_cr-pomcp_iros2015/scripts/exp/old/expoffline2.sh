#!/bin/sh

# offline

echo "OFFLINE"

echo "6x5"
echo "simple rew fs"

appl sol_nola ../pomdpx/simplerew_fs/map2_6x5_1o.pomdpx -o map2_6x5_srw_fs.pol > map2_6x5_srw_fs.log
mv SARSOP_time_log.txt SARSOP_time_log_map2_6x5_srw_fs.txt

appl sol_nola ../pomdpx/simplerew_fs/map5_6x5_4o.pomdpx -o map5_6x5_srw_fs.pol > map5_6x5_srw_fs.log
mv SARSOP_time_log.txt SARSOP_time_log_map5_6x5_srw_fs.txt

echo "simple rew fsc"

appl sol_nola ../pomdpx/simplerew_fsc/map2_6x5_1o.pomdpx -o map2_6x5_srw_fsc.pol > map2_6x5_srw_fsc.log
mv SARSOP_time_log.txt SARSOP_time_log_map2_6x5_srw_fsc.txt

appl sol_nola ../pomdpx/simplerew_fsc/map5_6x5_4o.pomdpx -o map5_6x5_srw_fsc.pol > map5_6x5_srw_fsc.log
mv SARSOP_time_log.txt SARSOP_time_log_map5_6x5_srw_fsc.txt

echo "newrew_fs_r3t1"

appl sol_nola ../pomdpx/newrew_fs_r3t1/map2_6x5_1o.pomdpx -o map2_6x5_newrew.pol > map2_6x5_newrew.log
mv SARSOP_time_log.txt SARSOP_time_log_map2_6x5_newrew.txt

appl sol_nola ../pomdpx/newrew_fs_r3t1/map5_6x5_4o.pomdpx -o map5_6x5_newrew.pol > map5_6x5_newrew.log
mv SARSOP_time_log.txt SARSOP_time_log_map5_6x5_newrew.txt



echo "10x10"

echo "simple rew fs"

appl sol_nola ../pomdpx/simplerew_fs/map4_10x10_4o.pomdpx -o map4_10x10_srw_fs.pol --timeout 21600 --memory 4000  > map4_10x10_srw_fs.log
mv SARSOP_time_log.txt SARSOP_time_log_map4_10x10_srw_fs.txt


echo "simple rew fsc"

appl sol_nola ../pomdpx/simplerew_fsc/map4_10x10_4o.pomdpx -o map4_10x10_srw_fsc.pol --timeout 21600 --memory 4000 > map4_10x10_srw_fsc.log
mv SARSOP_time_log.txt SARSOP_time_log_map4_10x10_srw_fsc.txt

echo "newrew_fs_r3t1"

appl sol_nola ../pomdpx/newrew_fs_r3t1/map4_10x10_4o.pomdpx -o map4_10x10_newrew.pol --timeout 21600 --memory 4000 > map4_10x10_newrew.log
mv SARSOP_time_log.txt SARSOP_time_log_map4_10x10_newrew.txt




rar a -m5 ~/Dropbox/Private/HideSeek/offline_logs_121213.rar *.txt *.log


