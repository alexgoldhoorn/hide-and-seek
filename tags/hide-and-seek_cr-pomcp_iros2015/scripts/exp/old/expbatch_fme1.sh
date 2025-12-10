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

./expbatch.py pomdpx/fme_place pomdpx/fme_place maps "9x10:1,2,3" "i$x" "10 45 0 0" "-T 300" "fme" l

./expbatch.py pomdpx/fme_place pomdpx/fme_place maps "9x10:1,2,3" "i$x" "10 45 0 0" "-T 300" "fme" s

./expbatch.py pomdpx/fme_place pomdpx/fme_place maps "9x10:1,2,3" "i$x" "5 45 0 0" "-T 300" "fme_2c" s



  x=$(( $x + 1 ))

rm *policy
rm debug*.txt


sleep 30

done
