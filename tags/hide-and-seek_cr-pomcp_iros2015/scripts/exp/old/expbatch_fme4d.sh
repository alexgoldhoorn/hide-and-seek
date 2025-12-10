#!/bin/sh


x=1
while [ $x -le 100 ]
do
  if [ ! -f "EXP.RUN" ];
  then
	echo "STOP (file 'EXP.RUN' is not present)"
	break
  fi


  echo "Run $x"

./hsmomdp -ov -sl -u fme9x12_Trf_w1 -m pomdpx/fme_place/simplerew_fs/map1_9x12.pomdpx -gr 12 45 0 0 -T 300 -Tr -Tf -w 1 > fme9x12_Trf_w1_map1_$x.log

./hsmomdp -ov -sl -u fme7x9_Trf_w1 -m pomdpx/fme_place/simplerew_fs/map1_7x9.pomdpx -gr 12 45 0 0 -T 300 -Tr -Tf -w 1 > fme7x9_Trf_w1_map1_$x.log

./hsmomdp -ov -sl -u fme9x12_Trf_w1 -m pomdpx/fme_place/simplerew_fs/map2_9x12.pomdpx -gr 12 45 0 0 -T 300 -Tr -Tf -w 1 > fme9x12_Trf_w1_map2_$x.log

./hsmomdp -ov -sl -u fme7x9_Trf_w1 -m pomdpx/fme_place/simplerew_fs/map2_7x9.pomdpx -gr 12 45 0 0 -T 300 -Tr -Tf -w 1 > fme7x9_Trf_w1_map2_$x.log




  x=$(( $x + 1 ))

rm *policy
rm debug*.txt


sleep 10

done
