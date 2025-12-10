#!/bin/sh

echo "Size: $1"

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
map="map${m}_${1}x${1}.pomdpx"

echo "Map: $map"

# random
./hsmomdp -ss -m $map -u SmartSeeker -s 192.168.100.220 1123 -oh > smartseeker_i$x_map$m_randh.log &
sleep 10
./hsautohider 192.168.100.220 1123 1  > hider_i$x_map$m_randh.log 

sleep 10

# very smart
./hsmomdp -ss -m $map -u SmartSeeker -s 192.168.100.220 1123 -oh > smartseeker_i$x_map$m_verysmarth.log &
sleep 10
./hsautohider 192.168.100.220 1123 5  > hider_i$x_map$m_verysmarth.log 

sleep 10

# all know. VERY smart
./hsmomdp -ss -m $map -u SmartSeeker -s 192.168.100.220 1123 -oh > smartseeker_i$x_map$m_allksmarth.log &
sleep 10
./hsautohider 192.168.100.220 1123 7  > hider_i$x_map$m_allksmarth.log 

sleep 10


  m=$(( $m + 1 ))

done




  x=$(( $x + 1 ))

tar czf logs_130510_smsk.tgz *log

sleep 10

done
