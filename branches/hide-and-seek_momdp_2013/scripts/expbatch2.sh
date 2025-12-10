#!/bin/sh

cd /mnt/ramdisk
pwd
mkdir pomdp/
mkdir pomdp/rew3maxtype1/
mkdir pomdp/rew3maxtype2/
cd pomdp/rew3maxtype1/
cp ~/MyProjects/Experiments/pomdp/rew3maxtype1/map3_6x5_2o.*  .
cp ~/MyProjects/Experiments/pomdp/rew3maxtype1/map3_10x10_3o.* .
rar x ~/MyProjects/Experiments/pomdp/rew3maxtype1/map20x20.rar map5_20x20* 
cd ../rew3maxtype2
cp ~/MyProjects/Experiments/pomdp/rew3maxtype2/map3_6x5_2o.*  .
cp ~/MyProjects/Experiments/pomdp/rew3maxtype2/map3_10x10_3o.* .
rar x ~/MyProjects/Experiments/pomdp/rew3maxtype2/map20x20.rar map5_20x20* 

cd /mnt/ramdisk/

x=1
while [ $x -le 10 ]
do
  echo "Run $x "


name="tt_map3_6x5_segBase_im_or_r3mt1_pi01_i$x"
echo $name
mv SARSOP_time_log_$name.txt SARSOP_time_log.txt
./hideseek -sl -gb -m pomdp/rew3maxtype1/map3_6x5_2o.pomdpx -A 2 -u $name -T 600 -or -L $name -um -pi 0.1 > $name_stream.txt
mv SARSOP_time_log.txt SARSOP_time_log_$name.txt

name="tt_map3_6x5_segK3_im_or_r3mt1_pi01_i$x"
echo $name
mv SARSOP_time_log_$name.txt SARSOP_time_log.txt
./hideseek -sl -gk 3 -m pomdp/rew3maxtype1/map3_6x5_2o.pomdpx -A 2 -u $name -T 600 -or -L $name -um -pi 0.1 > $name_stream.txt
mv SARSOP_time_log.txt SARSOP_time_log_$name.txt

name="tt_map3_6x5_segRC_im_or_r3mt1_pi01_i$x"
echo $name
mv SARSOP_time_log_$name.txt SARSOP_time_log.txt
./hideseek -sl -gr -m pomdp/rew3maxtype1/map3_6x5_2o.pomdpx -A 2 -u $name -T 600 -or -L $name -um -pi 0.1 > $name_stream.txt
mv SARSOP_time_log.txt SARSOP_time_log_$name.txt



name="tt_map3_10x10_segBase_im_or_r3mt1_pi01_i$x"
echo $name
mv SARSOP_time_log_$name.txt SARSOP_time_log.txt
./hideseek -sl -gb -m pomdp/rew3maxtype1/map3_10x10_3o.pomdpx -A 14 -u $name -T 600 -or -L $name -um -pi 0.1 > $name_stream.txt
mv SARSOP_time_log.txt SARSOP_time_log_$name.txt

name="tt_map3_10x10_segK3_im_or_r3mt1_pi01_i$x"
echo $name
mv SARSOP_time_log_$name.txt SARSOP_time_log.txt
./hideseek -sl -gk 3 -m pomdp/rew3maxtype1/map3_10x10_3o.pomdpx -A 14 -u $name -T 600 -or -L $name -um -pi 0.1 > $name_stream.txt
mv SARSOP_time_log.txt SARSOP_time_log_$name.txt

name="tt_map3_10x10_segRC_im_or_r3mt1_pi01_i$x"
echo $name
mv SARSOP_time_log_$name.txt SARSOP_time_log.txt
./hideseek -sl -gr -m pomdp/rew3maxtype1/map3_10x10_3o.pomdpx -A 14 -u $name -T 600 -or -L $name -um -pi 0.1 > $name_stream.txt
mv SARSOP_time_log.txt SARSOP_time_log_$name.txt



name="tt_map5_20x20_segBase_im_or_r3mt1_pi01_i$x"
echo $name
mv SARSOP_time_log_$name.txt SARSOP_time_log.txt
./hideseek -sl -gb -m pomdp/rew3maxtype1/map5_20x20_4o.pomdpx -A 29 -u $name -T 600 -or -L $name -um -pi 0.1 > $name_stream.txt
mv SARSOP_time_log.txt SARSOP_time_log_$name.txt

name="tt_map5_20x20_segK3_im_or_r3mt1_pi01_i$x"
echo $name
mv SARSOP_time_log_$name.txt SARSOP_time_log.txt
./hideseek -sl -gk 3 -m pomdp/rew3maxtype1/map5_20x20_4o.pomdpx -A 29 -u $name -T 600 -or -L $name -um -pi 0.1 > $name_stream.txt
mv SARSOP_time_log.txt SARSOP_time_log_$name.txt

name="tt_map5_20x20_segRC_im_or_r3mt1_pi01_i$x"
echo $name
mv SARSOP_time_log_$name.txt SARSOP_time_log.txt
./hideseek -sl -gr -m pomdp/rew3maxtype1/map5_20x20_4o.pomdpx -A 29 -u $name -T 600 -or -L $name -um -pi 0.1 > $name_stream.txt
mv SARSOP_time_log.txt SARSOP_time_log_$name.txt






name="tt_map3_6x5_segBase_im_or_r3mt2_pi01_i$x"
echo $name
mv SARSOP_time_log_$name.txt SARSOP_time_log.txt
./hideseek -sl -gb -m pomdp/rew3maxtype2/map3_6x5_2o.pomdpx -A 2 -u $name -T 600 -or -L $name -um -pi 0.1 > $name_stream.txt
mv SARSOP_time_log.txt SARSOP_time_log_$name.txt

name="tt_map3_6x5_segK3_im_or_r3mt2_pi01_i$x"
echo $name
mv SARSOP_time_log_$name.txt SARSOP_time_log.txt
./hideseek -sl -gk 3 -m pomdp/rew3maxtype2/map3_6x5_2o.pomdpx -A 2 -u $name -T 600 -or -L $name -um -pi 0.1 > $name_stream.txt
mv SARSOP_time_log.txt SARSOP_time_log_$name.txt

name="tt_map3_6x5_segRC_im_or_r3mt2_pi01_i$x"
echo $name
mv SARSOP_time_log_$name.txt SARSOP_time_log.txt
./hideseek -sl -gr -m pomdp/rew3maxtype2/map3_6x5_2o.pomdpx -A 2 -u $name -T 600 -or -L $name -um -pi 0.1 > $name_stream.txt
mv SARSOP_time_log.txt SARSOP_time_log_$name.txt



name="tt_map3_10x10_segBase_im_or_r3mt2_pi01_i$x"
echo $name
mv SARSOP_time_log_$name.txt SARSOP_time_log.txt
./hideseek -sl -gb -m pomdp/rew3maxtype2/map3_10x10_3o.pomdpx -A 14 -u $name -T 600 -or -L $name -um -pi 0.1 > $name_stream.txt
mv SARSOP_time_log.txt SARSOP_time_log_$name.txt

name="tt_map3_10x10_segK3_im_or_r3mt2_pi01_i$x"
echo $name
mv SARSOP_time_log_$name.txt SARSOP_time_log.txt
./hideseek -sl -gk 3 -m pomdp/rew3maxtype2/map3_10x10_3o.pomdpx -A 14 -u $name -T 600 -or -L $name -um -pi 0.1 > $name_stream.txt
mv SARSOP_time_log.txt SARSOP_time_log_$name.txt

name="tt_map3_10x10_segRC_im_or_r3mt2_pi01_i$x"
echo $name
mv SARSOP_time_log_$name.txt SARSOP_time_log.txt
./hideseek -sl -gr -m pomdp/rew3maxtype2/map3_10x10_3o.pomdpx -A 14 -u $name -T 600 -or -L $name -um -pi 0.1 > $name_stream.txt
mv SARSOP_time_log.txt SARSOP_time_log_$name.txt



name="tt_map5_20x20_segBase_im_or_r3mt2_pi01_i$x"
echo $name
mv SARSOP_time_log_$name.txt SARSOP_time_log.txt
./hideseek -sl -gb -m pomdp/rew3maxtype2/map5_20x20_4o.pomdpx -A 29 -u $name -T 600 -or -L $name -um -pi 0.1 > $name_stream.txt
mv SARSOP_time_log.txt SARSOP_time_log_$name.txt

name="tt_map5_20x20_segK3_im_or_r3mt2_pi01_i$x"
echo $name
mv SARSOP_time_log_$name.txt SARSOP_time_log.txt
./hideseek -sl -gk 3 -m pomdp/rew3maxtype2/map5_20x20_4o.pomdpx -A 29 -u $name -T 600 -or -L $name -um -pi 0.1 > $name_stream.txt
mv SARSOP_time_log.txt SARSOP_time_log_$name.txt

name="tt_map5_20x20_segRC_im_or_r3mt2_pi01_i$x"
echo $name
mv SARSOP_time_log_$name.txt SARSOP_time_log.txt
./hideseek -sl -gr -m pomdp/rew3maxtype2/map5_20x20_4o.pomdpx -A 29 -u $name -T 600 -or -L $name -um -pi 0.1 > $name_stream.txt
mv SARSOP_time_log.txt SARSOP_time_log_$name.txt





#./hideseek -sl -gk -m pomdp/testmapbig2.pomdpx -A 11 -u bigmap2_S$x -T 600 -os -L testbigmap2_S$x -um -pi 0.1 > bigmap2_stream_S$x.txt

  x=$(( $x + 1 ))


mybak

rm *policy
rm debug*.txt

done


