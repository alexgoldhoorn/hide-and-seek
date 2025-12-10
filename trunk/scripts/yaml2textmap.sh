#!/bin/sh
echo "Converter of YAML file to text map"
echo
if [ $# -lt 5 ]; 
then
echo "Expecting the following parameters:"
echo
echo "	YAML-file cell-size row-distance col-distance out-txt-file"
echo
echo "sizes and distances are in meters"
else
#    -ay file cell-sz  load map PGM file, passing cell size (m) and map size of rowD x colD # -ay file cell-sz  load map PGM file, passing cell size (m) and map size of rowD x colD m
$HOME/iri-lab/labrobotica/algorithms/hide-and-seek/trunk/build/build/release/momdp/hsmomdp -ay $1 $2 $3 $4 -W $5
fi
echo
