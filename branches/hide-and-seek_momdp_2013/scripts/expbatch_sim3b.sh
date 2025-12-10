#!/bin/sh
#mkdir pol/
#hscomplete2/learnOffline.py pomdpx pol "6x5:1,2,3,4;10x10:1,2,3,4" "--memory 6000 --timeout 3600"
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

# ./expbatch.py pomdpx/fme_place pomdpx/fme_place maps "7x9:1,2" "i$x" "10 45 0 0" "-T 10 -um -w 1" "fme7x9_w1_10s" l

./expbatch2.py pomdpx pol maps "6x5:1,2,3,4;10x10:1,2,3,4;12x12:1,2,3,4" "i$x" "12 45 0 0" "-T 300 -s localhost 1122 -Tr -Tf" "sim3" v
./expbatch2.py pomdpx pol maps "6x5:1,2,3,4;10x10:1,2,3,4;12x12:1,2,3,4" "i$x" "12 45 0 0" "-T 300 -s localhost 1122 -Tr -Tf -grx 12 45 0 0" "sim3_grx" v

./expbatch2.py pomdpx pol maps "6x5:1,2,3,4;10x10:1,2,3,4;12x12:1,2,3,4" "i$x" "12 45 0 0" "-T 10 -s localhost 1122 -Tr -Tf" "sim3_10s" v
./expbatch2.py pomdpx pol maps "6x5:1,2,3,4;10x10:1,2,3,4;12x12:1,2,3,4" "i$x" "12 45 0 0" "-T 10 -s localhost 1122 -Tr -Tf -grx 12 45 0 0" "sim3_10s_grx" v

./expbatch2.py pomdpx pol maps "6x5:1,2,3,4;10x10:1,2,3,4;12x12:1,2,3,4" "i$x" "12 45 0 0" "-T 2 -s localhost 1122 -um -Tr -Tf" "sim3_2s" v
./expbatch2.py pomdpx pol maps "6x5:1,2,3,4;10x10:1,2,3,4;12x12:1,2,3,4" "i$x" "12 45 0 0" "-T 2 -s localhost 1122 -um -Tr -Tf -grx 12 45 0 0" "sim3_2s_grx" v

./expbatch2.py pomdpx pol maps "6x5:1,2,3,4" "i$x" "3 45 0 0" "-T 300 -s localhost 1122 -Tr -Tf" "sim3_2c" v
./expbatch2.py pomdpx pol maps "10x10:1,2,3,4" "i$x" "5 45 0 0" "-T 300 -s localhost 1122 -Tr -Tf" "sim3_2c" v
./expbatch2.py pomdpx pol maps "12x12:1,2,3,4" "i$x" "6 45 0 0" "-T 300 -s localhost 1122 -Tr -Tf" "sim3_2c" v
./expbatch2.py pomdpx pol maps "6x5:1,2,3,4" "i$x" "3 45 0 0" "-T 300 -s localhost 1122 -Tr -Tf -grx 12 45 0 0" "sim3_2c_grx" v
./expbatch2.py pomdpx pol maps "10x10:1,2,3,4" "i$x" "5 45 0 0" "-T 300 -s localhost 1122 -Tr -Tf -grx 12 45 0 0" "sim3_2c_grx" v
./expbatch2.py pomdpx pol maps "12x12:1,2,3,4" "i$x" "6 45 0 0" "-T 300 -s localhost 1122 -Tr -Tf -grx 12 45 0 0" "sim3_2c_grx" v


./expbatch3.py pomdpx pol maps "6x5:1,2,3,4;10x10:1,2,3,4" "i$x" "10 45 0 0" "-T 300 -s localhost 1122" "sim4" v




  x=$(( $x + 1 ))

rm *policy
rm debug*.txt

rar m -m5 ~/Temp/logs_130508_sim3_trtf.rar *log

sleep 10

done
