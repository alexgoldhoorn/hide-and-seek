#!/bin/sh

x=1
while [ $x -le 100 ]
do
  echo "Run $x "

./hideseek -sl -gk -m pomdpx/testmapbig2.pomdpx -A 11 -u bigmap2_R$x -T 600 -or -L testbigmap2_R$x -um -pi 0.1 > bigmap2_stream_R$x.txt

./hideseek -sl -gk -m pomdpx/testmapbig2.pomdpx -A 11 -u bigmap2_S$x -T 600 -os -L testbigmap2_S$x -um -pi 0.1 > bigmap2_stream_S$x.txt

  x=$(( $x + 1 ))

rm *policy
rm debug*.txt

done


