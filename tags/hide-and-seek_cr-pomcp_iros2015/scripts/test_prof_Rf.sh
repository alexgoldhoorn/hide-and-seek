#!/bin/sh



./hsmomdp -or -m map4_20x20 -sc -u pomcp_prof_opt_20_Rf -s localhost 1120  -hsr -Es -ns 10000 -Rf
gprof ./hsmomdp3 > prof_opt2_Rf_map4_20x20.txt

#./hsmomdp3 -or -as ../mapx_100x100.txt -sc -u pomcp_prof_opt2_100_Rf -s localhost 1122  -hsr -En -ns 10000 -Rf
#gprof ./hsmomdp3 > prof_opt2_Rf_mapx_100x100.txt

./hsmomdp3 -or -as maps/mapx_50x50.txt -sc -u pomcp_prof_opt_50_Rf -s localhost 1120  -hsr -Es -ns 10000 -Rf
gprof ./hsmomdp > prof_opt2_Rf_mapx_50x50.txt
