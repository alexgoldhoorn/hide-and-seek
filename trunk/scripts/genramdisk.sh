#!/bin/sh
cd $HSPATH/trunk #~/iri-lab/labrobotica/algorithms/hide-and-seek/trunk/

size=1024M
if [ $# -gt 0 ]; then
	size=$1
	echo "size:$s"
fi

if [ ! -d /mnt/ramdisk ]; then
	echo "Creating Ramdisk"
	sudo mkdir /mnt/ramdisk
	sudo chmod 777 /mnt/ramdisk/
fi
if [ ! -e /mnt/ramdisk/hsserver ]; then
	echo "Mounting Ramdisk of $size"
	sudo mount -t tmpfs -o size=$size tmpfs /mnt/ramdisk/
fi

cp build/build/release/momdp/hsmomdp /mnt/ramdisk/
#cp build/build/release/momdp_debug/hsmomdp_debug /mnt/ramdisk/
#cp build/build/release/mcvi/hsmcvi /mnt/ramdisk/
#cp build/build/release/actiongen/actiongen /mnt/ramdisk/
cp build/build/release/server/hsserver /mnt/ramdisk/
cp build/build/release/autohider/hsautohider /mnt/ramdisk/
#cp scripts/expbatch*py /mnt/ramdisk
#cp scripts/applsol_nolh.sh /mnt/ramdisk
cp cfg/EXP.RUN /mnt/ramdisk
cp scripts/exp/updramdisk.sh /mnt/ramdisk
cp scripts/gendistmat.sh /mnt/ramdisk
cp scripts/exp/gendmatfiles.sh /mnt/ramdisk
cp scripts/exp/explog.sh /mnt/ramdisk
cp scripts/exp/expstop.sh /mnt/ramdisk

x=1
while [ $x -le 4 ]; do
	mkdir /mnt/ramdisk/server$x
	cp cfg/serverconf_ramdisk$x.xml /mnt/ramdisk/server$x
	cp cfg/serverconf_ramdisk_rem$x.xml /mnt/ramdisk/server$x
	x=$(( $x + 1 ))
done

x=1
while [ $x -le 4 ]; do
	cd /mnt/ramdisk/server$x
	ln -s ../hsserver

	mkdir /mnt/ramdisk/run$x
	cd /mnt/ramdisk/run$x
	ln -s ../hsmomdp
#	ln -s ../hsmomdp_debug
#	ln -s ../actiongen
	ln -s ../hsautohider
	cp ../EXP.RUN .
        ln -s ../expstop.sh

#	ln -s /home/agoldhoorn/MyProjects/Experiments/pomdpx/
#	ln -s /home/agoldhoorn/MyProjects/Experiments/maps/

	x=$(( $x + 1 ))
done


cd /mnt/ramdisk
#ln -s /home/agoldhoorn/MyProjects/HideSeekGame/hscomplete3/
ln -s $HSPATH #~/iri-lab/labrobotica/algorithms/hide-and-seek/
ln -s ~/agoldhoorn/experiments/sim-scripts/


