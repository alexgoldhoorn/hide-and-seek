#!/bin/sh


# $1: map dir
# $2: map name
# $3: post_name
# $4: output_dir
# $5: count

x=1
while [ $x -le $5 ]
do

	echo "Run $2_actions_$3_i$x.txt"

	./actiongen $1/$2.txt $4/$2_actions_$3_i$x.txt $2_actions_$3_i$x

	x=$(( $x + 1 ))
sleep 1 
done
