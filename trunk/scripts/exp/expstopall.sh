#!/bin/sh
for d in run?/ ; do 
    echo $d 
    cd $d
    pwd
    ./expstop.sh $1
    cd ..
done
