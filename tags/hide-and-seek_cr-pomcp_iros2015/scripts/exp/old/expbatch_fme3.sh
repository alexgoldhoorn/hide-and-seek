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

# ./expbatch.py pomdpx/fme_place pomdpx/fme_place maps "7x9:1,2" "i$x" "10 45 0 0" "-T 10 -um -w 1" "fme7x9_w1_10s" l

./expbatch.py pomdpx/fme_place pomdpx/fme_place maps "7x9:1,2" "i$x" "10 45 0 0" "-T 300 -w 1" "fme7x9_w1_fs2" s

./expbatch.py pomdpx/fme_place pomdpx/fme_place maps "7x9:1,2" "i$x" "10 45 0 0" "-T 300 -w 1" "fme7x9_w1_fs2" l


#/expbatch.py pomdpx/fme_place pomdpx/fme_place maps "7x9:1,2" "i$x" "4 45 0 0" "-T 300" "fme7x9_w1_2c_fs" s



  x=$(( $x + 1 ))

rm *policy
rm debug*.txt


sleep 10

done
