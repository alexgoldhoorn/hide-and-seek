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


#./expbatch2.py pomdpx pol maps "12x12:1,2,3,4" "i$x" "12 45 0 0" "-T 300 -Tr -Tf -grx 12 45 0 0 -s localhost 1121" "sim3_grx" v

./expbatch2.py pomdpx pol maps "12x12:1,2,3,4" "i$x" "12 45 0 0" "-T 10 -Tr -Tf -grx 12 45 0 0 -s localhost 1121" "sim3_10s_grx" v
./expbatch2.py pomdpx pol maps "12x12:1,2,3,4" "i$x" "12 45 0 0" "-T 10 -Tr -Tf -grx 12 45 0 0 -s localhost 1121" "sim3_10s_grx" A

./expbatch2.py pomdpx pol maps "12x12:1,2,3,4" "i$x" "12 45 0 0" "-T 2 -um -Tr -Tf -grx 12 45 0 0 -s localhost 1121" "sim3_2s_grx" v
./expbatch2.py pomdpx pol maps "12x12:1,2,3,4" "i$x" "12 45 0 0" "-T 2 -um -Tr -Tf -grx 12 45 0 0 -s localhost 1121" "sim3_2s_grx" A

sleep 10

./expbatch2.py pomdpx pol maps "12x12:1,2,3,4" "i$x" "6 45 0 0" "-T 10 -Tr -Tf -grx 6 45 0 0 -s localhost 1121" "sim3_2c_10s_grx" v
./expbatch2.py pomdpx pol maps "12x12:1,2,3,4" "i$x" "6 45 0 0" "-T 10 -Tr -Tf -grx 6 45 0 0 -s localhost 1121" "sim3_2c_10s_grx" A

# smart seeker:
./expbatch3.py pomdpx pol maps "12x12:1,2,3,4" "i$x" "12 45 0 0" "-T 10 -s localhost 1121" "sim4" v
./expbatch3.py pomdpx pol maps "12x12:1,2,3,4" "i$x" "12 45 0 0" "-T 10 -s localhost 1121" "sim4" A



  x=$(( $x + 1 ))

rm *policy
rm debug*.txt

rar m -m5 ~/Temp/logs_130508_sim3c_grx.rar *log


sleep 20

done
