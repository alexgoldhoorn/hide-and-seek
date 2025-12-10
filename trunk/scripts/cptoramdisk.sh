#!/bin/sh
cd $HSPATH/trunk

cp build/build/release/momdp/hsmomdp /mnt/ramdisk/
#cp build/build/release/momdp_debug/hsmomdp_debug /mnt/ramdisk/
#cp build/build/release/mcvi/hsmcvi /mnt/ramdisk/
#cp build/build/release/actiongen/actiongen /mnt/ramdisk/
cp build/build/release/server/hsserver /mnt/ramdisk/
cp build/build/release/autohider/hsautohider /mnt/ramdisk/
#cp scripts/expbatch*py /mnt/ramdisk
#cp scripts/applsol_nolh.sh /mnt/ramdisk
cp cfg/EXP.RUN /mnt/ramdisk

