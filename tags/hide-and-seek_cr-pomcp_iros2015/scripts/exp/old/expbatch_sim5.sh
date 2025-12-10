#!/bin/sh
#mkdir pol/
#hscomplete2/learnOffline.py pomdpx pol "12x12:1,2,3,4" "--memory 6000 --timeout 3600"
#echo "DONE learning" > EXP.RUN

x=1
while [ $x -le 100 ]
do
  if [ ! -f "EXP.RUN" ];
  then
	echo "STOP (file 'EXP.RUN' is not present)"
	break
  fi


  echo "Run $x"

# ./expbatch.py pomdpx/fme_place pomdpx/fme_place maps "7x9:1,2" "i$x" "12 45 0 0" "-T 10 -um -w 1" "fme7x9_w1_10s" l

./expbatch2.py pomdpx pol maps "12x12:1,2,3,4" "i$x" "12 45 0 0" "-T 300 -Tr -Tf -grx 12 45 0 0 -s localhost 1121" "sim3_grx" s
./expbatch2.py pomdpx pol maps "12x12:1,2,3,4" "i$x" "12 45 0 0" "-T 300 -Tr -Tf -grx 12 45 0 0 -s localhost 1121" "sim3_grx" l

./expbatch2.py pomdpx pol maps "12x12:1,2,3,4" "i$x" "12 45 0 0" "-T 10 -Tr -Tf -grx 12 45 0 0 -s localhost 1121" "sim3_10s_grx" s
./expbatch2.py pomdpx pol maps "12x12:1,2,3,4" "i$x" "12 45 0 0" "-T 10 -Tr -Tf -grx 12 45 0 0 -s localhost 1121" "sim3_10s_grx" l

./expbatch2.py pomdpx pol maps "12x12:1,2,3,4" "i$x" "12 45 0 0" "-T 2 -um -Tr -Tf -grx 12 45 0 0 -s localhost 1121" "sim3_2s_grx" s
./expbatch2.py pomdpx pol maps "12x12:1,2,3,4" "i$x" "12 45 0 0" "-T 2 -um -Tr -Tf -grx 12 45 0 0 -s localhost 1121" "sim3_2s_grx" l

sleep 10

./expbatch2.py pomdpx pol maps "12x12:1,2,3,4" "i$x" "6 45 0 0" "-T 300 -Tr -Tf -grx 6 45 0 0 -s localhost 1121" "sim3_2c_grx" s
./expbatch2.py pomdpx pol maps "12x12:1,2,3,4" "i$x" "6 45 0 0" "-T 300 -Tr -Tf -grx 6 45 0 0 -s localhost 1121" "sim3_2c_grx" l

./expbatch3.py pomdpx pol maps "12x12:1,2,3,4" "i$x" "12 45 0 0" "-T 300 -s localhost 1121" "sim4" s
./expbatch3.py pomdpx pol maps "12x12:1,2,3,4" "i$x" "12 45 0 0" "-T 300 -s localhost 1121" "sim4" l




  x=$(( $x + 1 ))

rm *policy
rm debug*.txt


sleep 20

done
