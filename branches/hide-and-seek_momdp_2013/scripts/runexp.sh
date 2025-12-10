#!/bin/sh
# params:
# 1. pomdp
# 2. map id (for server, 0=map1,..)
# 3. solve timeout
# 4. opponent type: (0=human,1=random hider,2=smart hider)
# 5. log id


#[ 0]  "map1_6x5_0o.txt"<<"map2_6x5_1o.txt"<<"map3_6x5_2o.txt"
#[ 3]<<"map4_6x5_2o.txt"<<"map5_6x5_4o.txt"<< "map1_40x40_1o.txt" 
#[ 6]<< "map2_40x40_2o.txt"<< "map3_40x40_3o.txt"<< "map4_40x40_4o.txt"
#[ 9]<< "map5_40x40_5o.txt"<< "testmapbig1.txt" << "testmapbig2.txt"
#[12]<< "map1_10x10_1o.txt" << "map2_10x10_2o.txt" << "map3_10x10_3o.txt"
#[15]<< "map4_10x10_4o.txt" << "map5_10x10.txt" << "map6_10x10.txt"  
#[18] << "map7_10x10.txt"  << "mapbcn1.txt" << "mapbcn2.txt"
#[21] << "mapbcn3.txt" << "mapbcn1a.txt" << "mapbcn1b.txt"
#[24] << "mapbcn1c.txt";



if [ $# -lt 4 ]; then

echo "expected parameters: "
echo " 1. pomdp"
echo " 2. map id (for server, 0=map1,..)"
echo " 3. solve timeout"
echo " 4. opponent type: (0=human,1=random hider,2=smart hider)"
echo " 5. log id"

else

echo "MOMDP 	: $1"
echo "MapID 	: $2"
echo "Timeout 	: $3"
echo "OppType 	: $4"
echo "log 	: $5"
echo "username 	: online_user_$5"

rm momdp*.policy

./hideseek $1 localhost 1120 $2 $4 online_user_$5 log_time_online_real_$5.txt $3 log_online_real_$5.txt > log_stream_online_real_$5.txt

fi
