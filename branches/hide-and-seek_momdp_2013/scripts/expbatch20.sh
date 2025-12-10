#!/bin/sh

x=1
while [ $x -le 100 ]
do
  echo "Run $x"

  hscomplete2/expbatch.py pomdpx off2 maps "12x12:1,2,3,4;15x15:3,4" "i$x" "-T 300"

  x=$(( $x + 1 ))

rm *policy
rm debug*.txt

sleep 100

done


