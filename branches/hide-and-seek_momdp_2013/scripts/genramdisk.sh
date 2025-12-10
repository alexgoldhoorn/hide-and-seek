#!/bin/sh
cd ~/MyProjects/HideSeekGame/hscomplete2
if [ ! -d /mnt/ramdisk ]; then
	sudo mkdir /mnt/ramdisk
	sudo chmod 777 /mnt/ramdisk/
fi
sudo mount -t tmpfs -o size=1024M tmpfs /mnt/ramdisk/

cp build/release/momdp/hsmomdp /mnt/ramdisk/
cp build/release/mcvi/hsmcvi /mnt/ramdisk/
cp build/release/algen/actiongen /mnt/ramdisk/
cp build/release/server/hsserver /mnt/ramdisk/
cp build/release/autohider/hsautohider /mnt/ramdisk/
cp scripts/expbatch*py /mnt/ramdisk
cp scripts/applsol_nolh.sh /mnt/ramdisk
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
	ln -s ../hsmcvi
	ln -s ../actiongen
	ln -s ../hsautohider
	ln -s ../expbatch.py
	ln -s ../expbatch3.py
	ln -s ../expbatch2.py
	cp ../EXP.RUN .

	ln -s /home/agoldhoorn/MyProjects/Experiments/pomdpx/
	ln -s /home/agoldhoorn/MyProjects/Experiments/maps/

	x=$(( $x + 1 ))
done


cd /mnt/ramdisk
ln -s /home/agoldhoorn/MyProjects/HideSeekGame/hscomplete2/



