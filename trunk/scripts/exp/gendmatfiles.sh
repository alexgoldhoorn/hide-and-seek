#!/bin/sh

cd /mnt/ramdisk
mkdir dmat
./hsmomdp -a $HSPATH/data/maps/bcn_lab/fme2014_map4.txt -C -dw dmat/fme2014_map4.dmat.txt
./hsmomdp -a $HSPATH/data/maps/brl/brl29a.txt -C -dw dmat/brl29a.dmat.txt
./hsmomdp -a $HSPATH/data/maps/brl/master29e.txt -C -dw dmat/master29e.dmat.txt
./hsmomdp -a $HSPATH/data/maps/brl/master2015s2.txt -C -dw dmat/master2015s2.dmat.txt
./hsmomdp -a $HSPATH/data/maps/brl/master2016med.txt -C -dw dmat/master2016med.dmat.txt

# ./hsmomdp -a $HSPATH/data/maps/bcn_lab/upc_campus_1m.txt -C -dw dmat/upc_campus_1m.dmat.txt
# ./hsmomdp -a $HSPATH/data/maps/ext/nsh.txt -C -dw dmat/nsh.dmat.txt
#./hsmomdp -a $HSPATH/data/maps/ext/museum.txt -C -dw dmat/museum.dmat.txt


