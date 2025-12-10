#!/bin/sh
# smart NO NOISE
if [ $# -le 1 ];
then
	echo "expected params: map port"
	exit
fi

echo "Size: $1"
echo "port: $2"


x=1
while [ $x -le 100 ]
do
  if [ ! -f "EXP.RUN" ];
  then
	echo "STOP (file 'EXP.RUN' is not present)"
	break
  fi


  echo "Run $x"

m=1
while [ $m -le 4 ]
do
map="map${m}_${1}.pomdpx"

echo "Map: $map"

sleep 3

# very smart
echo "smartseeker_i${x}_map${m}_verysmarth"
./hsmomdp -ss -m $map -u SmartSeeker -s 192.168.100.220 $2 -oh > smartseeker_i${x}_map${m}_verysmarth.log &
sleep 5
echo "hider_i${x}_map${m}_verysmarth"
./hsautohider 192.168.100.220 $2 5 0  > hider_i${x}_map${m}_verysmarth.log 
tail -n 2 smartseeker_i${x}_map${m}_verysmarth.log
echo "DONE"
sleep 3

# all know. VERY smart
echo "smartseeker_i${x}_map${m}_allksmarth"
./hsmomdp -ss -m $map -u SmartSeeker -s 192.168.100.220 $2 -oh > smartseeker_i${x}_map${m}_allksmarth.log &
sleep 5
echo "hider_i${x}_map${m}_allksmarth"
./hsautohider 192.168.100.220 $2 7 0 > hider_i${x}_map${m}_allksmarth.log 
tail -n 2 smartseeker_i${x}_map${m}_allksmarth.log
echo "DONE"
sleep 3


  m=$(( $m + 1 ))

done




  x=$(( $x + 1 ))

#tar czf logs_130510_smsk.tgz *log
rar m -m5 ~/Temp/smart_logs_${map}_130516.rar *log

sleep 10

done
