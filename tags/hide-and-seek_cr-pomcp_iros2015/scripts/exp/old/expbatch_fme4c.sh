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

./expbatch2.py pomdpx/fme_place pomdpx/fme_place maps "7x9:1,2" "i$x" "12 45 0 0" "-T 300 -s localhost 1121 -Tr -Tf -w 1" "fme7x9_Trf_w1" l

./expbatch2.py pomdpx/fme_place pomdpx/fme_place maps "7x9:1,2" "i$x" "12 45 0 0" "-T 10 -s localhost 1121 -Tr -Tf -um -w 1" "fme7x9_Trf_w1_10s" v

./expbatch2.py pomdpx/fme_place pomdpx/fme_place maps "7x9:1,2" "i$x" "6 45 0 0" "-T 300 -s localhost 1121 -Tr -Tf -w 1" "fme7x9_Trf_w1_2c" v

./expbatch2.py pomdpx/fme_place pomdpx/fme_place maps "7x9:1,2" "i$x" "6 45 0 0" "-T 300 -s localhost 1121 -Tr -Tf -w 1" "fme7x9_Trf_w1_2c" l



  x=$(( $x + 1 ))

rm *policy
rm debug*.txt


sleep 30

done
