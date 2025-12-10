#!/bin/sh
mkdir pol/
hscomplete2/learnOffline.py pomdpx pol "6x5:1,2,3,4;10x10:1,2,3,4"
echo "DONE learning" > EXP.RUN

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

./expbatch2.py pomdpx pol maps "6x5:1,2,3,4;10x10:1,2,3,4" "i$x" "10 45 0 0" "-T 300" "sim2" s
./expbatch2.py pomdpx pol maps "6x5:1,2,3,4;10x10:1,2,3,4" "i$x" "10 45 0 0" "-T 300" "sim2" l

./expbatch2.py pomdpx pol maps "6x5:1,2,3,4;10x10:1,2,3,4" "i$x" "10 45 0 0" "-T 10" "sim2_10s" s
./expbatch2.py pomdpx pol maps "6x5:1,2,3,4;10x10:1,2,3,4" "i$x" "10 45 0 0" "-T 10" "sim2_10s" l

./expbatch2.py pomdpx pol maps "6x5:1,2,3,4;10x10:1,2,3,4" "i$x" "10 45 0 0" "-T 2 -um" "sim2_2s" s
./expbatch2.py pomdpx pol maps "6x5:1,2,3,4;10x10:1,2,3,4" "i$x" "10 45 0 0" "-T 2 -um" "sim2_2s" l

./expbatch2.py pomdpx pol maps "6x5:1,2,3,4" "i$x" "3 45 0 0" "-T 300" "sim2_2c" s
./expbatch2.py pomdpx pol maps "10x10:1,2,3,4" "i$x" "5 45 0 0" "-T 300" "sim2_2c" s
./expbatch2.py pomdpx pol maps "6x5:1,2,3,4" "i$x" "3 45 0 0" "-T 300" "sim2_2c" l
./expbatch2.py pomdpx pol maps "10x10:1,2,3,4" "i$x" "5 45 0 0" "-T 300" "sim2_2c" l


  x=$(( $x + 1 ))

rm *policy
rm debug*.txt


sleep 10

done
