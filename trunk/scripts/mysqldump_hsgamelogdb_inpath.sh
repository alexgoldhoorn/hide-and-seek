#!/bin/sh
savepath=.
if [ $# -gt 0 ];
then
savepath=$1
fi
today=`date +%y%m%d_%H%M`
$HSPATH/trunk/scripts/mysqldump_hsgamelogdb.sh $savepath/hsgamelogdb_$today.sql ! 
