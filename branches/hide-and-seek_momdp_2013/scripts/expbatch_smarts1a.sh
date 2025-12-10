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

m=1
while [ $m -le 4 ]
do
map="map${m}_40x40.pomdpx"

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




m=1
while [ $m -le 3 ]
do
map="mapbcn${m}.pomdpx"

# random
./hsmomdp -ss -m $map -u SmartSeeker -s 192.168.100.220 1123 -oh > smartseeker_i$x_mapbcn$m_randh.log &
sleep 10
./hsautohider 192.168.100.220 1123 1  > hider_i$x_mapbcn$m_randh.log 

sleep 10


# very smart
./hsmomdp -ss -m $map -u SmartSeeker -s 192.168.100.220 1123 -oh > smartseeker_i$x_mapbcn$m_verysmarth.log &
sleep 10
./hsautohider 192.168.100.220 1123 5  > hider_i$x_mapbcn$m_verysmarth.log 

sleep 10


# all know. VERY smart 2
./hsmomdp -ss -m $map -u SmartSeeker -s 192.168.100.220 1123 -oh > smartseeker_i$xa_mapbcn$m_allksmarth.log &
sleep 10
./hsautohider 192.168.100.220 1123 7  > hider_i$xa_mapbcn$m_allksmarth.log 


  m=$(( $m + 1 ))

done



map="mapbcn_full.pomdpx"

# random
./hsmomdp -ss -m $map -u SmartSeeker -s 192.168.100.220 1123 -oh > smartseeker_i$x_mapbcnfull_randh.log &
sleep 10
./hsautohider 192.168.100.220 1123 1  > hider_i$x_mapbcnfull_randh.log 

sleep 10


# very smart
./hsmomdp -ss -m $map -u SmartSeeker -s 192.168.100.220 1123 -oh > smartseeker_i$x_mapbcnfull_verysmarth.log &
sleep 10
./hsautohider 192.168.100.220 1123 5  > hider_i$x_mapbcnfull_verysmarth.log 

sleep 10


# all know. VERY smart 2
./hsmomdp -ss -m $map -u SmartSeeker -s 192.168.100.220 1123 -oh > smartseeker_i$xa_mapbcnfull_allksmarth.log &
sleep 10
./hsautohider 192.168.100.220 1123 7  > hider_i$xa_mapbcnfull_allksmarth.log 




  x=$(( $x + 1 ))

tar czf logs_130509_smsk.tgz *log

sleep 10

done
