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


./expbatch3.py pomdpx pol maps "6x5:1,2,3,4;10x10:1,2,3,4" "i$x" "10 45 0 0" "-T 300 -Tr -Tf -grx 10 45 0 0" "sim4" s
./expbatch3.py pomdpx pol maps "6x5:1,2,3,4;10x10:1,2,3,4" "i$x" "10 45 0 0" "-T 300 -Tr -Tf -grx 10 45 0 0" "sim4" l


  x=$(( $x + 1 ))

rm *policy
rm debug*.txt


sleep 10

done
