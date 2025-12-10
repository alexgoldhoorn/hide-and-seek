#!/bin/sh
cd ~/iri-lab/labrobotica/algorithms/hide-and-seek/trunk/
if [ ! -d /mnt/ramdisk ]; then
	echo "Creating Ramdisk"
	sudo mkdir /mnt/ramdisk
	sudo chmod 777 /mnt/ramdisk/
fi
if [ ! -e /mnt/ramdisk/hsserver ]; then
	echo "Mounting Ramdisk"
	sudo mount -t tmpfs -o size=1024M tmpfs /mnt/ramdisk/
fi

cp build/build/release/momdp/hsmomdp /mnt/ramdisk/
cp build/build/release/momdp_debug/hsmomdp_debug /mnt/ramdisk/
#cp build/build/release/mcvi/hsmcvi /mnt/ramdisk/
#cp build/build/release/actiongen/actiongen /mnt/ramdisk/
cp build/build/release/server/hsserver /mnt/ramdisk/
cp build/build/release/autohider/hsautohider /mnt/ramdisk/
#cp scripts/expbatch*py /mnt/ramdisk
#cp scripts/applsol_nolh.sh /mnt/ramdisk
cp cfg/EXP.RUN /mnt/ramdisk

x=1
while [ $x -le 4 ]; do
	mkdir /mnt/ramdisk/server$x
	cp cfg/serverconf_ramdisk$x.xml /mnt/ramdisk/server$x
	x=$(( $x + 1 ))
done

x=1
while [ $x -le 4 ]; do
	cd /mnt/ramdisk/server$x
	ln -s ../hsserver

	mkdir /mnt/ramdisk/run$x
	cd /mnt/ramdisk/run$x
	ln -s ../hsmomdp
	ln -s ../hsmomdp_debug
#	ln -s ../actiongen
	ln -s ../hsautohider
	cp ../EXP.RUN .

#	ln -s /home/agoldhoorn/MyProjects/Experiments/pomdpx/
#	ln -s /home/agoldhoorn/MyProjects/Experiments/maps/

	x=$(( $x + 1 ))
done


cd /mnt/ramdisk
#ln -s /home/agoldhoorn/MyProjects/HideSeekGame/hscomplete3/
ln -s ~/iri-lab/labrobotica/algorithms/hide-and-seek/



