#!/bin/sh

# continuation of #16, using same action files

x=1
while [ $x -le 1000 ]
do
  echo "Run $x - map3_6x5"
# ./actiongen maps/map3_6x5_2o.txt map3_6x5_actions_i$x.txt map3_6x5_actions_i$x

./hsmomdp -ol map3_6x5_actions_i$x.txt -so -u spd_map3_6x5_sr_fsc_off -m pomdpx/simplerew_fsc/map3_6x5_2o.pomdpx -p off/map3_6x5_srw_fsc.pol > spd_map3_6x5_sr_fsc_off_i$x.log

mv SARSOP_time_log_spd_map3_6x5_sr_fsc_on_gr.txt SARSOP_time_log.txt
./hsmomdp -ol map3_6x5_actions_i$x.txt -slc -u spd_map3_6x5_sr_fsc_on_gr -T 300 -gr 10 45 0 -m pomdpx/simplerew_fsc/map3_6x5_2o.pomdpx -p off/map3_6x5_srw_fsc.pol > spd_map3_6x5_sr_fsc_on_gr_i$x.log
mv SARSOP_time_log.txt SARSOP_time_log_spd_map3_6x5_sr_fsc_on_gr.txt

mv SARSOP_time_log_spd_map3_6x5_sr_fsc_on_gc.txt SARSOP_time_log.txt
./hsmomdp -ol map3_6x5_actions_i$x.txt -slc -u spd_map3_6x5_sr_fsc_on_gc -T 300 -gc 10 45 0 -m pomdpx/simplerew_fsc/map3_6x5_2o.pomdpx -p off/map3_6x5_srw_fsc.pol > spd_map3_6x5_sr_fsc_on_gc_i$x.log
mv SARSOP_time_log.txt SARSOP_time_log_spd_map3_6x5_sr_fsc_on_gc.txt


./hsmomdp -ol map3_6x5_actions_i$x.txt -so -u spd_map3_6x5_newrew_off -m pomdpx/newrew_fs_r3t1/map3_6x5_2o.pomdpx -p off/map3_6x5_newrew.pol > spd_map3_6x5_newrew_off_i$x.log

mv SARSOP_time_log_spd_map3_6x5_newrew_on_gr.txt SARSOP_time_log.txt
./hsmomdp -ol map3_6x5_actions_i$x.txt -slc -u spd_map3_6x5_newrew_on_gr -T 300 -gr 10 45 0 -m pomdpx/newrew_fs_r3t1/map3_6x5_2o.pomdpx -p off/map3_6x5_newrew.pol > spd_map3_6x5_newrew_on_gr_i$x.log
mv SARSOP_time_log.txt SARSOP_time_log_spd_map3_6x5_newrew_on_gr.txt

mv SARSOP_time_log_spd_map3_6x5_newrew_on_gc.txt SARSOP_time_log.txt
./hsmomdp -ol map3_6x5_actions_i$x.txt -slc -u spd_map3_6x5_newrew_on_gc -T 300 -gc 10 45 0 -m pomdpx/newrew_fs_r3t1/map3_6x5_2o.pomdpx -p off/map3_6x5_newrew.pol > spd_map3_6x5_newrew_on_gc_i$x.log
mv SARSOP_time_log.txt SARSOP_time_log_spd_map3_6x5_newrew_on_gc.txt






  echo "Run $x - map4_6x5"
#./actiongen maps/map4_6x5_2o.txt map4_6x5_actions_i$x.txt map4_6x5_actions_i$x

./hsmomdp -ol map4_6x5_actions_i$x.txt -so -u spd_map4_6x5_sr_fsc_off -m pomdpx/simplerew_fsc/map4_6x5_2o.pomdpx -p off/map4_6x5_srw_fsc.pol > spd_map4_6x5_sr_fsc_off_i$x.log

mv SARSOP_time_log_spd_map4_6x5_sr_fsc_on_gr.txt SARSOP_time_log.txt
./hsmomdp -ol map4_6x5_actions_i$x.txt -slc -u spd_map4_6x5_sr_fsc_on_gr -T 300 -gr 10 45 0 -m pomdpx/simplerew_fsc/map4_6x5_2o.pomdpx -p off/map4_6x5_srw_fsc.pol > spd_map4_6x5_sr_fsc_on_gr_i$x.log
mv SARSOP_time_log.txt SARSOP_time_log_spd_map4_6x5_sr_fsc_on_gr.txt

mv SARSOP_time_log_spd_map4_6x5_sr_fsc_on_gc.txt SARSOP_time_log.txt
./hsmomdp -ol map4_6x5_actions_i$x.txt -slc -u spd_map4_6x5_sr_fsc_on_gc -T 300 -gc 10 45 0 -m pomdpx/simplerew_fsc/map4_6x5_2o.pomdpx -p off/map4_6x5_srw_fsc.pol > spd_map4_6x5_sr_fsc_on_gc_i$x.log
mv SARSOP_time_log.txt SARSOP_time_log_spd_map4_6x5_sr_fsc_on_gc.txt


./hsmomdp -ol map4_6x5_actions_i$x.txt -so -u spd_map4_6x5_newrew_off -m pomdpx/newrew_fs_r3t1/map4_6x5_2o.pomdpx -p off/map4_6x5_newrew.pol > spd_map4_6x5_newrew_off_i$x.log

mv SARSOP_time_log_spd_map4_6x5_newrew_on_gr.txt SARSOP_time_log.txt
./hsmomdp -ol map4_6x5_actions_i$x.txt -slc -u spd_map4_6x5_newrew_on_gr -T 300 -gr 10 45 0 -m pomdpx/newrew_fs_r3t1/map4_6x5_2o.pomdpx -p off/map4_6x5_newrew.pol > spd_map4_6x5_newrew_on_gr_i$x.log
mv SARSOP_time_log.txt SARSOP_time_log_spd_map4_6x5_newrew_on_gr.txt

mv SARSOP_time_log_spd_map4_6x5_newrew_on_gc.txt SARSOP_time_log.txt
./hsmomdp -ol map4_6x5_actions_i$x.txt -slc -u spd_map4_6x5_newrew_on_gc -T 300 -gc 10 45 0 -m pomdpx/newrew_fs_r3t1/map4_6x5_2o.pomdpx -p off/map4_6x5_newrew.pol > spd_map4_6x5_newrew_on_gc_i$x.log
mv SARSOP_time_log.txt SARSOP_time_log_spd_map4_6x5_newrew_on_gc.txt




  echo "Run $x - map1_10x10"
#./actiongen maps/map1_10x10_1o.txt map1_10x10_actions_i$x.txt map1_10x10_actions_i$x

./hsmomdp -ol map1_10x10_actions_i$x.txt -so -u spd_map1_10x10_sr_fsc_off -m pomdpx/simplerew_fsc/map1_10x10_1o.pomdpx -p off/map1_10x10_srw_fsc.pol > spd_map1_10x10_sr_fsc_off_i$x.log

mv SARSOP_time_log_spd_map1_10x10_sr_fsc_on_gr.txt SARSOP_time_log.txt
./hsmomdp -ol map1_10x10_actions_i$x.txt -slc -u spd_map1_10x10_sr_fsc_on_gr -T 300 -gr 10 45 0 -m pomdpx/simplerew_fsc/map1_10x10_1o.pomdpx -p off/map1_10x10_srw_fsc.pol > spd_map1_10x10_sr_fsc_on_gr_i$x.log
mv SARSOP_time_log.txt SARSOP_time_log_spd_map1_10x10_sr_fsc_on_gr.txt

mv SARSOP_time_log_spd_map1_10x10_sr_fsc_on_gc.txt SARSOP_time_log.txt
./hsmomdp -ol map1_10x10_actions_i$x.txt -slc -u spd_map1_10x10_sr_fsc_on_gc -T 300 -gc 10 45 0 -m pomdpx/simplerew_fsc/map1_10x10_1o.pomdpx -p off/map1_10x10_srw_fsc.pol > spd_map1_10x10_sr_fsc_on_gc_i$x.log
mv SARSOP_time_log.txt SARSOP_time_log_spd_map1_10x10_sr_fsc_on_gc.txt



./hsmomdp -ol map1_10x10_actions_i$x.txt -so -u spd_map1_10x10_newrew_off -m pomdpx/newrew_fs_r3t1/map1_10x10_1o.pomdpx -p off/map1_10x10_newrew.pol > spd_map1_10x10_newrew_off_i$x.log

mv SARSOP_time_log_spd_map1_10x10_newrew_on_gr.txt SARSOP_time_log.txt
./hsmomdp -ol map1_10x10_actions_i$x.txt -slc -u spd_map1_10x10_newrew_on_gr -T 300 -gr 10 45 0 -m pomdpx/newrew_fs_r3t1/map1_10x10_1o.pomdpx -p off/map1_10x10_newrew.pol > spd_map1_10x10_newrew_on_gr_i$x.log
mv SARSOP_time_log.txt SARSOP_time_log_spd_map1_10x10_newrew_on_gr.txt

mv SARSOP_time_log_spd_map1_10x10_newrew_on_gc.txt SARSOP_time_log.txt
./hsmomdp -ol map1_10x10_actions_i$x.txt -slc -u spd_map1_10x10_newrew_on_gc -T 300 -gc 10 45 0 -m pomdpx/newrew_fs_r3t1/map1_10x10_1o.pomdpx -p off/map1_10x10_newrew.pol > spd_map1_10x10_newrew_on_gc_i$x.log
mv SARSOP_time_log.txt SARSOP_time_log_spd_map1_10x10_newrew_on_gc.txt



  echo "Run $x - map3_10x10"
#./actiongen maps/map3_10x10_3o.txt map3_10x10_actions_i$x.txt map3_10x10_actions_i$x

./hsmomdp -ol map3_10x10_actions_i$x.txt -so -u spd_map3_10x10_sr_fsc_off -m pomdpx/simplerew_fsc/map3_10x10_3o.pomdpx -p off/map3_10x10_srw_fsc.pol > spd_map3_10x10_sr_fsc_off_i$x.log

mv SARSOP_time_log_spd_map3_10x10_sr_fsc_on_gr.txt SARSOP_time_log.txt
./hsmomdp -ol map3_10x10_actions_i$x.txt -slc -u spd_map3_10x10_sr_fsc_on_gr -T 300 -gr 10 45 0 -m pomdpx/simplerew_fsc/map3_10x10_1o.pomdpx -p off/map3_10x10_srw_fsc.pol > spd_map3_10x10_sr_fsc_on_gr_i$x.log
mv SARSOP_time_log.txt SARSOP_time_log_spd_map3_10x10_sr_fsc_on_gr.txt

mv SARSOP_time_log_spd_map3_10x10_sr_fsc_on_gc.txt SARSOP_time_log.txt
./hsmomdp -ol map3_10x10_actions_i$x.txt -slc -u spd_map3_10x10_sr_fsc_on_gc -T 300 -gc 10 45 0 -m pomdpx/simplerew_fsc/map3_10x10_3o.pomdpx -p off/map3_10x10_srw_fsc.pol > spd_map3_10x10_sr_fsc_on_gc_i$x.log
mv SARSOP_time_log.txt SARSOP_time_log_spd_map3_10x10_sr_fsc_on_gc.txt


./hsmomdp -ol map3_10x10_actions_i$x.txt -so -u spd_map3_10x10_newrew_off -m pomdpx/newrew_fs_r3t1/map3_10x10_3o.pomdpx -p off/map3_10x10_newrew.pol > spd_map3_10x10_newrew_off_i$x.log

mv SARSOP_time_log_spd_map3_10x10_newrew_on_gr.txt SARSOP_time_log.txt
./hsmomdp -ol map3_10x10_actions_i$x.txt -slc -u spd_map3_10x10_newrew_on_gr -T 300 -gr 10 45 0 -m pomdpx/newrew_fs_r3t1/map3_10x10_1o.pomdpx -p off/map3_10x10_newrew.pol > spd_map3_10x10_newrew_on_gr_i$x.log
mv SARSOP_time_log.txt SARSOP_time_log_spd_map3_10x10_newrew_on_gr.txt

mv SARSOP_time_log_spd_map3_10x10_newrew_on_gc.txt SARSOP_time_log.txt
./hsmomdp -ol map3_10x10_actions_i$x.txt -slc -u spd_map3_10x10_newrew_on_gc -T 300 -gc 10 45 0 -m pomdpx/newrew_fs_r3t1/map3_10x10_3o.pomdpx -p off/map3_10x10_newrew.pol > spd_map3_10x10_newrew_on_gc_i$x.log
mv SARSOP_time_log.txt SARSOP_time_log_spd_map3_10x10_newrew_on_gc.txt




# CORRECTED (map3_10x10_1 -> _3)
mv SARSOP_time_log_spd_map3_10x10_sr_fs_on_gr.txt SARSOP_time_log.txt
./hsmomdp -ol map3_10x10_actions_i$x.txt -slc -u spd_map3_10x10_sr_fs_on_gr -T 300 -gr 10 45 0 -m pomdpx/simplerew_fs/map3_10x10_1o.pomdpx -p off/map3_10x10_srw_fs.pol > spd_map3_10x10_sr_fs_on_gr_i$x.log
mv SARSOP_time_log.txt SARSOP_time_log_spd_map3_10x10_sr_fs_on_gr.txt


  x=$(( $x + 1 ))

rm *policy
rm debug*.txt

sleep 100

done


