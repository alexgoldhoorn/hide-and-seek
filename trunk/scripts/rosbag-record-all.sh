#!/bin/sh
if [ $# -lt 1 ]; then
echo "Required parameter: experiment-name"
exit
fi
cd ~
mkdir -p hs/$1
cd hs/$1
echo "Output dir: `pwd`"
rosbag record --all -x "(.*)local_th(.*)||(.*)compressed(.*)||(.*)theora(.*)||(.*)image_color(.*)||(.*)image_mono(.*)||(.*)image_raw(.*)||(.*)image_rect_color(.*)||(.*)front_right(.*)||(.*)front_left(.*)||(.*)rear_left(.*)||(.*)rear_right(.*)||(.*)ladybug_front(.*)||(.*)ladybug_top(.*)||(.*)bumblebee_right(.*)||(.*)bumblebee_left(.*)||(.*)imu(.*)||(.*)joy(.*)||(.*)wii(.*)||(.*)imu(.*)||(.*)people(.*)"

