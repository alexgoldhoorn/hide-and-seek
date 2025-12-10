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


./expbatch2.py pomdpx pol maps "6x5:1,2,3,4;10x10:1,2,3,4;12x12:1,2,3,4" "i$x" "12 45 0 0" "-T 300" "sim2" v

./expbatch2.py pomdpx pol maps "6x5:1,2,3,4;10x10:1,2,3,4;12x12:1,2,3,4" "i$x" "12 45 0 0" "-T 10" "sim2_10s" v

./expbatch2.py pomdpx pol maps "6x5:1,2,3,4;10x10:1,2,3,4;12x12:1,2,3,4" "i$x" "12 45 0 0" "-T 2 -um" "sim2_2s" v

./expbatch2.py pomdpx pol maps "6x5:1,2,3,4" "i$x" "3 45 0 0" "-T 300" "sim2_2c" v
./expbatch2.py pomdpx pol maps "10x10:1,2,3,4" "i$x" "5 45 0 0" "-T 300" "sim2_2c" v
./expbatch2.py pomdpx pol maps "12x12:1,2,3,4" "i$x" "6 45 0 0" "-T 300" "sim2_2c" v


  x=$(( $x + 1 ))

rm *policy
rm debug*.txt

rar m -m5 ~/Temp/logs_130508_sim2.rar *log

sleep 10

done
