#!/bin/sh
# params:
# 1. run number
# 2. map ID
# 3. map full path
# 4. dist matrix file
# 5. max num steps
# 6. num auto walkers
# 7. noise std
# 8. num. sim.
# 9. num. belief points
# 10. port
# 11. zoom
# 12. exp-name (user name)

# for hider
#./hsautohider -create 10 $3 run$1_walker.txt 1 $5 -wd 1 C
# auto walker
if [ $6 -gt 0 ];
then
#./hsautohider -create 10 $3 run$1_autowalkers.txt $6 $5 -wd 1 C

# follower
./hsmomdp -Gh -Rfd -ud -s localhost ${10} -u ${12} -kf run$1_autowalkers.txt -of run$1_walker.txt -sfe -Ns $7 -MS $5 -wd 1 -C -A $2 -dl $4 
# POMCP
./hsmomdp -Gh -Rfd -ud -s localhost ${10} -u ${12} -kf run$1_autowalkers.txt -of run$1_walker.txt -scc -Ns $7 -MS $5 -wd 1 -C -A $2 -dl $4 -ns $8 -ni $9
# POMCP highest belief
./hsmomdp -Gh -Rfd -ud -s localhost ${10} -u ${12} -kf run$1_autowalkers.txt -of run$1_walker.txt -shc -Ns $7 -MS $5 -wd 1 -C -A $2 -dl $4 -ns $8 -ni $9 -bz ${11} -us 3
# Combi: POMCP + Follower
./hsmomdp -Gh -Rfd -ud -s localhost ${10} -u ${12} -kf run$1_autowalkers.txt -of run$1_walker.txt -sCc -Ns $7 -MS $5 -wd 1 -C -A $2 -dl $4 -ns $8 -ni $9 
# Combi: POMCP High. Belief + Follower
./hsmomdp -Gh -Rfd -ud -s localhost ${10} -u ${12} -kf run$1_autowalkers.txt -of run$1_walker.txt -sCh -Ns $7 -MS $5 -wd 1 -C -A $2 -dl $4 -ns $8 -ni $9 -bz ${11} -us 3

else

# follower
./hsmomdp -Gh -Rfd -ud -s localhost ${10} -u ${12} -of run$1_walker.txt -sfe -Ns $7 -MS $5 -wd 1 -C -A $2 -dl $4 
# POMCP
./hsmomdp -Gh -Rfd -ud -s localhost ${10} -u ${12} -of run$1_walker.txt -scc -Ns $7 -MS $5 -wd 1 -C -A $2 -dl $4 -ns $8 -ni $9
# POMCP highest belief
./hsmomdp -Gh -Rfd -ud -s localhost ${10} -u ${12} -of run$1_walker.txt -shc -Ns $7 -MS $5 -wd 1 -C -A $2 -dl $4 -ns $8 -ni $9 -bz ${11} -us 3
# Combi: POMCP + Follower
./hsmomdp -Gh -Rfd -ud -s localhost ${10} -u ${12} -of run$1_walker.txt -sCc -Ns $7 -MS $5 -wd 1 -C -A $2 -dl $4 -ns $8 -ni $9 
# Combi: POMCP High. Belief + Followe
./hsmomdp -Gh -Rfd -ud -s localhost ${10} -u ${12} -of run$1_walker.txt -sCh -Ns $7 -MS $5 -wd 1 -C -A $2 -dl $4 -ns $8 -ni $9 -bz ${11} -us 3

fi


