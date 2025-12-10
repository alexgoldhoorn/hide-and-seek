#!/bin/sh

cd /mnt/ramdisk
mkdir dmat
./hsmomdp -a $HSPATH/data/maps/bcn_lab/fme2014_map4.txt -C -dw dmat/fme2014_map4.dmat.txt
./hsmomdp -a $HSPATH/data/maps/brl/brl29a.txt -C -dw dmat/brl29a.dmat.txt
./hsmomdp -a $HSPATH/data/maps/brl/master29e.txt -C -dw dmat/master29e.dmat.txt
