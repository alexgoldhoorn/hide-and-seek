#!/bin/sh
cd ~
mkdir -p hs/$1
cd hs/$1
pwd
rosbag record --all -x "(.*)local_th(.*)||(.*)compressed(.*)||(.*)theora(.*)||(.*)image_color(.*)||(.*)image_mono(.*)||(.*)image_raw(.*)||(.*)image_rect_color(.*)||(.*)front_right(.*)||(.*)front_left(.*)||(.*)rear_left(.*)||(.*)rear_right(.*)"

