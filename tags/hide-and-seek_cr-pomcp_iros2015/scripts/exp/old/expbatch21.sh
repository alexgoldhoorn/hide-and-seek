#!/bin/sh


x=1
while [ $x -le 100 ]
do
  echo "Run $x"

	# 1 circle
  hscomplete2/expbatch.py pomdpx off2 maps "12x12:1,2,3,4" "i$x" "-T 300"

	# segmentation: 2 circles
  hscomplete2/expbatch.py pomdpx off2 maps "10x10:1,2,3,4" "i$x" "5 45 0" "-T 300"

  hscomplete2/expbatch.py pomdpx off2 maps "12x12:1,2,3,4" "i$x" "6 45 0" "-T 300"

#  hscomplete2/expbatch.py pomdpx off2 maps "15x15:3,4" "i$x" "8 45 0" "-T 300"

  x=$(( $x + 1 ))

rm *policy
rm debug*.txt

sleep 100

done
