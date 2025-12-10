#!/bin/sh



./hsmomdp -or -m map4_20x20 -sc -u pomcp_prof_opt_20_Rt -s localhost 1121  -hsr -En -ns 10000 -Rt
gprof ./hsmomdp2 > prof_opt1_Rt_map4_20x20.txt

#./hsmomdp2 -or -as ../mapx_100x100.txt -sc -u testS -s localhost 1123  -hsr -En -ns 10000 -Rt
#gprof ./hsmomdp2 > prof_opt1_Rt_mapx_100x100.txt

./hsmomdp2 -or -as maps/mapx_50x50.txt -sc -u pomcp_prof_opt_20_Rt -s localhost 1121  -hsr -En -ns 10000 -Rt
gprof ./hsmomdp > prof_opt1_Rt_mapx_50x50.txt
