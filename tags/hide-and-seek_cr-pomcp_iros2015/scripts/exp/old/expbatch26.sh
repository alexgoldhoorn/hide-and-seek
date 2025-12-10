#!/bin/sh


x=1
while [ $x -le 50 ]
do
  echo "Run $x"

./expbatch.py pomdpx off2 maps "6x5:3,4;10x10:1,2,3,4;12x12:1,2,3,4" "i$x" "10 45 0 0" "-T 300 -d 3 " "sh_maxd3" s

./expbatch.py pomdpx off2 maps "6x5:3,4;10x10:1,2,3,4;12x12:1,2,3,4" "i$x" "10 45 0 0" "-T 300 -d 10 " "sh_maxd10" s

./expbatch.py pomdpx off2 maps "6x5:3,4;10x10:1,2,3,4;12x12:1,2,3,4" "i$x" "10 45 0 0" "-T 300 -d 20 " "sh_maxd20" s

./expbatch.py pomdpx off2 maps "6x5:3,4;10x10:1,2,3,4;12x12:1,2,3,4" "i$x" "10 45 0 0" "-T 300 -d 100 " "sh_maxd100" s


./expbatch.py pomdpx off2 maps "6x5:3,4;10x10:1,2,3,4;12x12:1,2,3,4" "i$x" "10 45 0 0" "-T 300 " "sh" s

./expbatch.py pomdpx off2 maps "6x5:3,4;10x10:1,2,3,4;12x12:1,2,3,4" "i$x" "10 45 0 0" "-T 300 -grx 10 45 0 1 " "sh_grx_1c" s 


./expbatch.py pomdpx off2 maps "10x10:1,2,3,4" "i$x" "5 45 0 0" "-T 300 " "sh_2c" s 
./expbatch.py pomdpx off2 maps "12x12:1,2,3,4" "i$x" "6 45 0 0" "-T 300 " "sh_2c" s



#./expbatch.py pomdpx off2 maps "10x10:1,2;20x20,1,2,3" "i$x" "10 45 0 0" "-T 300" "expC2"

#./expbatch.py pomdpx off2 maps "12x12:1,2,3,4" "i$x" "6 45 0" "-T 300"


  x=$(( $x + 1 ))

rm *policy
rm debug*.txt

  if [ ! -f "EXP.RUN" ];
  then
	echo "STOP"
	break
  fi


sleep 60

done
