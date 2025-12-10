#!/bin/sh


x=1
while [ $x -le 50 ]
do
  if [ ! -f "EXP.RUN" ];
  then
	echo "STOP (file 'EXP.RUN' is not present)"
	break
  fi


  echo "Run $x"

./expbatch.py pomdpx/fme_place pomdpx/fme_place maps "9x12:1,2" "i$x" "12 45 0 0" "-T 300 -Tr -Tf -w 1" "fme9x12_Trf_w1" l

./expbatch.py pomdpx/fme_place pomdpx/fme_place maps "9x12:1,2" "i$x" "12 45 0 0" "-T 10 -Tr -Tf -um -w 1" "fme9x12_Trf_w1_10s" v

./expbatch.py pomdpx/fme_place pomdpx/fme_place maps "9x12:1,2" "i$x" "6 45 0 0" "-T 300 -Tr -Tf -w 1" "fme9x12_Trf_w1_2c" v

./expbatch.py pomdpx/fme_place pomdpx/fme_place maps "9x12:1,2" "i$x" "6 45 0 0" "-T 300 -Tr -Tf -w 1" "fme9x12_Trf_w1_2c" l



  x=$(( $x + 1 ))

rm *policy
rm debug*.txt


sleep 30

done
