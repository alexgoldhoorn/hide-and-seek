#!/bin/grid_sh


x=1
while [ $x -le 50 ]
do
  echo "Run $x"

./expbatch.py pomdpx off2 maps "6x5:3,4;10x10:1,2,3,4;12x12:1,2,3,4" "i$x" "10 45 0 0" "-T 300 -s 192.168.100.220 1130 -d 3 " "grid_sh_maxd3" s

./expbatch.py pomdpx off2 maps "6x5:3,4;10x10:1,2,3,4;12x12:1,2,3,4" "i$x" "10 45 0 0" "-T 300 -s 192.168.100.220 1130 -d 10 " "grid_sh_maxd10" s

./expbatch.py pomdpx off2 maps "6x5:3,4;10x10:1,2,3,4;12x12:1,2,3,4" "i$x" "10 45 0 0" "-T 300 -s 192.168.100.220 1130 -d 20 " "grid_sh_maxd20" s

./expbatch.py pomdpx off2 maps "6x5:3,4;10x10:1,2,3,4;12x12:1,2,3,4" "i$x" "10 45 0 0" "-T 300 -s 192.168.100.220 1130 -d 100 " "grid_sh_maxd100" s


./expbatch.py pomdpx off2 maps "6x5:3,4;10x10:1,2,3,4;12x12:1,2,3,4" "i$x" "10 45 0 0" "-T 300 -s 192.168.100.220 1130 " "grid_sh" s

./expbatch.py pomdpx off2 maps "6x5:3,4;10x10:1,2,3,4;12x12:1,2,3,4" "i$x" "10 45 0 0" "-T 300 -s 192.168.100.220 1130 -grx 10 45 0 1 " "grid_sh_grx_1c" s 


./expbatch.py pomdpx off2 maps "10x10:1,2,3,4" "i$x" "5 45 0 0" "-T 300 -s 192.168.100.220 1130 " "grid_sh_2c" s 
./expbatch.py pomdpx off2 maps "12x12:1,2,3,4" "i$x" "6 45 0 0" "-T 300 -s 192.168.100.220 1130 " "grid_sh_2c" s



#./expbatch.py pomdpx off2 maps "10x10:1,2;20x20,1,2,3" "i$x" "10 45 0 0" "-T 300 -s 192.168.100.220 1130" "expC2"

#./expbatch.py pomdpx off2 maps "12x12:1,2,3,4" "i$x" "6 45 0" "-T 300 -s 192.168.100.220 1130"


  x=$(( $x + 1 ))

rm *policy
rm debug*.txt

sleep 100

done
