#!/bin/sh
echo "MySQL Dump of hsgamelog database"
if [ $# -lt 1 ];
then
echo "Parameters:"
echo "	sql-file [rar]"
echo 
echo "If rar is set a rar file is generated, if it is '!' then the rar file will be: sql-file.rar"
else
echo "Generating SQL file: $1 ..."
mysqldump -R -u root -p hsgamelog > $1
echo " done"
ls -lh $1
echo
if [ $# -gt 1 ];
then
rarf=$2
if [ "$rarf"=="!" ]; 
then
rarf=$1.rar
fi
echo "Generating rar file: $rarf ..."
rar m -m5 $rarf $1
ls -lh $rarf
fi
fi
echo
