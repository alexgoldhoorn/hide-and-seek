#!/bin/sh
x=1
while [ $x -le 4 ]; do
	echo "====run $x===="
	tail /mnt/ramdisk/run$x/log.tmp
	x=$(( $x + 1 ))
done



