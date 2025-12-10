#!/bin/sh
#assuming ramdisk mounted
mkdir /mnt/ramdisk/dmat/
echo "Generating distance maps"
echo "bcn_lab/fme2014_map4.txt"
~/iri-lab/labrobotica/algorithms/hide-and-seek/trunk/scripts/gendistmat.sh ~/iri-lab/labrobotica/algorithms/hide-and-seek/data/maps/bcn_lab/fme2014_map4.txt /mnt/ramdisk/dmat/fme2014_map4.dmat.txt -C
echo "brl/brl29a.txt"
~/iri-lab/labrobotica/algorithms/hide-and-seek/trunk/scripts/gendistmat.sh ~/iri-lab/labrobotica/algorithms/hide-and-seek/data/maps/brl/brl29a.txt /mnt/ramdisk/dmat/brl29a.dmat.txt -C
echo "brl/master29e.txt"
~/iri-lab/labrobotica/algorithms/hide-and-seek/trunk/scripts/gendistmat.sh ~/iri-lab/labrobotica/algorithms/hide-and-seek/data/maps/brl/master29e.txt /mnt/ramdisk/dmat/master29e.dmat.txt -C
echo "brl/master29f.txt"
~/iri-lab/labrobotica/algorithms/hide-and-seek/trunk/scripts/gendistmat.sh ~/iri-lab/labrobotica/algorithms/hide-and-seek/data/maps/brl/master29f.txt /mnt/ramdisk/dmat/master29f.dmat.txt -C
