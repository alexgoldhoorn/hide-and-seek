#!/bin/sh


x=1
while [ $x -le 10 ]
do
  echo "Run $x"

./expbatch.py pomdpx off2 maps "10x10:1,2,3,4" "i$x" "10 45 0 0" "-T 300 -d 3 " "maxd3"

./expbatch.py pomdpx off2 maps "10x10:1,2,3,4" "i$x" "10 45 0 0" "-T 300 -d 10 " "maxd10"



#./expbatch.py pomdpx off2 maps "10x10:1,2;20x20,1,2,3" "i$x" "10 45 0 0" "-T 300" "expC2"

#./expbatch.py pomdpx off2 maps "12x12:1,2,3,4" "i$x" "6 45 0" "-T 300"


  x=$(( $x + 1 ))

rm *policy
rm debug*.txt

sleep 100

done
