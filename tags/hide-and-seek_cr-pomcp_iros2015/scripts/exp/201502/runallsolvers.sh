#!/bin/sh
# params:
# 1. run number
# 2. map ID
# 3. map full path
# 4. dist matrix file
# 5. num steps
# 6. num auto walkers
# 7. noise std
# 8. num. sim.
# 9. num. belief points
# 10. port
# 11. zoom
# 12. exp-name (user name)

# for hider
#./hsautohider -create 10 $3 run$1_walker.txt 1 $5 C
# auto walker
if [ $6 -gt 0 ];
then
#./hsautohider -create 10 $3 run$1_autowalkers.txt $6 $5 C

# follower
#./hsmomdp -Gf -bi "" -Rfd -ud -s localhost ${10} -u ${12} -kf run$1_autowalkers.txt -of run$1_walker.txt -sfe -Ns $7 -Sn $5 -C -Sp start$1.txt -A $2 -dl $4


# POMCP
#./hsmomdp -Gf -bi "" -Rfd -ud -s localhost ${10} -u ${12} -kf run$1_autowalkers.txt -of run$1_walker.txt -scc -Ns $7 -Sn $5 -C -Sp start$1.txt -A $2 -dl $4 -ns $8 -ni $9
# POMCP highest belief
#./hsmomdp -Gf -bi "" -Rfd -ud -s localhost ${10} -u ${12} -kf run$1_autowalkers.txt -of run$1_walker.txt -shc -Ns $7 -Sn $5 -C -Sp start$1.txt -A $2 -dl $4 -ns $8 -ni $9 -bz ${11} -us 3
# Combi: POMCP + Follower
#./hsmomdp -Gf -bi "" -Rfd -ud -s localhost ${10} -u ${12} -kf run$1_autowalkers.txt -of run$1_walker.txt -sCc -Ns $7 -Sn $5 -C -Sp start$1.txt -A $2 -dl $4 -ns $8 -ni $9
# Combi: POMCP High. Belief + Follower
./hsmomdp -Gf -bi "" -Rfd -ud -s localhost ${10} -u ${12} -kf run$1_autowalkers.txt -of run$1_walker.txt -sCh -Ns $7 -Sn $5 -C -Sp start$1.txt -A $2 -dl $4 -ns $8 -ni $9 -bz ${11} -us 3

sleep 1

#multi with comm
#s1
./hsmomdp -Gf2 -bi "" -Rff -ud -s localhost ${10} -u ${12}_s1 -kf run$1_autowalkers.txt -of run$1_walker.txt -sM -Ns $7 -Sn $5 -C -Sp start$1.txt -A $2 -dl $4 -ns $8 -ni $9 -bz ${11} -us 3 -d 1 -bf -em 25 -mo 0.6 -en 10 -md 4 -fd 0 -mr 0 &
sleep 1
#s2
./hsmomdp -Gf2 -bi "" -Rff -ud -s localhost ${10} -u ${12}_s2 -kf run$1_autowalkers.txt -of run$1_walker.txt -sM -Ns $7 -Sn $5 -C -Sp start$1.txt -A $2 -dl $4 -ns $8 -ni $9 -bz ${11} -us 3 -d 1 -bf -em 25 -mo 0.6 -en 10 -md 4 -fd 0 -mr 0
sleep 1

#multi without comm
#s1
./hsmomdp -Gf2 -bi "" -Rff -ud -s localhost ${10} -u ${12}_nc_s1 -kf run$1_autowalkers.txt -of run$1_walker.txt -sM -Ns $7 -Sn $5 -C -Sp start$1.txt -A $2 -dl $4 -ns $8 -ni $9 -bz ${11} -us 3 -d 1 -bf -em 25 -mo 0.6 -en 10 -md 4 -fd 0 -mr 0 -nc &
sleep 1
#s2
./hsmomdp -Gf2 -bi "" -Rff -ud -s localhost ${10} -u ${12}_nc_s2 -kf run$1_autowalkers.txt -of run$1_walker.txt -sM -Ns $7 -Sn $5 -C -Sp start$1.txt -A $2 -dl $4 -ns $8 -ni $9 -bz ${11} -us 3 -d 1 -bf -em 25 -mo 0.6 -en 10 -md 4 -fd 0 -mr 0 -nc
sleep 1


# same multi - starting in different poses

#multi with comm
#s1
./hsmomdp -Gf2 -bi "" -Rff -ud -s localhost ${10} -u ${12}_op_s1 -kf run$1_autowalkers.txt -of run$1_walker.txt -sM -Ns $7 -Sn $5 -C -Sp start$1.txt -A $2 -dl $4 -ns $8 -ni $9 -bz ${11} -us 3 -d 1 -bf -em 25 -mo 0.6 -en 10 -md 4 -fd 0 -mr 0 &
sleep 1
#s2
./hsmomdp -Gf2 -bi "" -Rff -ud -s localhost ${10} -u ${12}_op_s2 -kf run$1_autowalkers.txt -of run$1_walker.txt -sM -Ns $7 -Sn $5 -C -Sp start2_$1.txt -A $2 -dl $4 -ns $8 -ni $9 -bz ${11} -us 3 -d 1 -bf -em 25 -mo 0.6 -en 10 -md 4 -fd 0 -mr 0
sleep 1

#multi without comm
#s1
./hsmomdp -Gf2 -bi "" -Rff -ud -s localhost ${10} -u ${12}_nc_op_s1 -kf run$1_autowalkers.txt -of run$1_walker.txt -sM -Ns $7 -Sn $5 -C -Sp start$1.txt -A $2 -dl $4 -ns $8 -ni $9 -bz ${11} -us 3 -d 1 -bf -em 25 -mo 0.6 -en 10 -md 4 -fd 0 -mr 0 -nc &
sleep 1
#s2
./hsmomdp -Gf2 -bi "" -Rff -ud -s localhost ${10} -u ${12}_nc_op_s2 -kf run$1_autowalkers.txt -of run$1_walker.txt -sM -Ns $7 -Sn $5 -C -Sp start2_$1.txt -A $2 -dl $4 -ns $8 -ni $9 -bz ${11} -us 3 -d 1 -bf -em 25 -mo 0.6 -en 10 -md 4 -fd 0 -mr 0 -nc
sleep 1


else

# follower
#./hsmomdp -Gf -bi "" -Rfd -ud -s localhost ${10} -u ${12} -of run$1_walker.txt -sfe -Ns $7 -Sn $5 -C -Sp start$1.txt -A $2 -dl $4

# POMCP
#./hsmomdp -Gf -bi "" -Rfd -ud -s localhost ${10} -u ${12} -of run$1_walker.txt -scc -Ns $7 -Sn $5 -C -Sp start$1.txt -A $2 -dl $4 -ns $8 -ni $9
# POMCP highest belief
#./hsmomdp -Gf -bi "" -Rfd -ud -s localhost ${10} -u ${12} -of run$1_walker.txt -shc -Ns $7 -Sn $5 -C -Sp start$1.txt -A $2 -dl $4 -ns $8 -ni $9 -bz ${11} -us 3
# Combi: POMCP + Follower
#./hsmomdp -Gf -bi "" -Rfd -ud -s localhost ${10} -u ${12} -of run$1_walker.txt -sCc -Ns $7 -Sn $5 -C -Sp start$1.txt -A $2 -dl $4 -ns $8 -ni $9
# Combi: POMCP High. Belief + Followe
./hsmomdp -Gf -bi "" -Rfd -ud -s localhost ${10} -u ${12} -of run$1_walker.txt -sCh -Ns $7 -Sn $5 -C -Sp start$1.txt -A $2 -dl $4 -ns $8 -ni $9 -bz ${11} -us 3

sleep 1


#multi with comm
#s1
./hsmomdp -Gf2 -bi "" -Rff -ud -s localhost ${10} -u ${12}_s1 -of run$1_walker.txt -sM -Ns $7 -Sn $5 -C -Sp start$1.txt -A $2 -dl $4 -ns $8 -ni $9 -bz ${11} -us 3 -d 1 -bf -em 25 -mo 0.6 -en 10 -md 4 -fd 0 -mr 0 &
sleep 1
#s2
./hsmomdp -Gf2 -bi "" -Rff -ud -s localhost ${10} -u ${12}_s2 -of run$1_walker.txt -sM -Ns $7 -Sn $5 -C -Sp start$1.txt -A $2 -dl $4 -ns $8 -ni $9 -bz ${11} -us 3 -d 1 -bf -em 25 -mo 0.6 -en 10 -md 4 -fd 0 -mr 0
sleep 1

#multi without comm
#s1
./hsmomdp -Gf2 -bi "" -Rff -ud -s localhost ${10} -u ${12}_nc_s1 -of run$1_walker.txt -sM -Ns $7 -Sn $5 -C -Sp start$1.txt -A $2 -dl $4 -ns $8 -ni $9 -bz ${11} -us 3 -d 1 -bf -em 25 -mo 0.6 -en 10 -md 4 -fd 0 -mr 0 &
sleep 1
#s2
./hsmomdp -Gf2 -bi "" -Rff -ud -s localhost ${10} -u ${12}_nc_s2 -of run$1_walker.txt -sM -Ns $7 -Sn $5 -C -Sp start$1.txt -A $2 -dl $4 -ns $8 -ni $9 -bz ${11} -us 3 -d 1 -bf -em 25 -mo 0.6 -en 10 -md 4 -fd 0 -mr 0
sleep 1


# same multi - starting in different poses

#multi with comm
#s1
./hsmomdp -Gf2 -bi "" -Rff -ud -s localhost ${10} -u ${12}_op_s1 -of run$1_walker.txt -sM -Ns $7 -Sn $5 -C -Sp start$1.txt -A $2 -dl $4 -ns $8 -ni $9 -bz ${11} -us 3 -d 1 -bf -em 25 -mo 0.6 -en 10 -md 4 -fd 0 -mr 0 &
sleep 1
#s2
./hsmomdp -Gf2 -bi "" -Rff -ud -s localhost ${10} -u ${12}_op_s2 -of run$1_walker.txt -sM -Ns $7 -Sn $5 -C -Sp start2_$1.txt -A $2 -dl $4 -ns $8 -ni $9 -bz ${11} -us 3 -d 1 -bf -em 25 -mo 0.6 -en 10 -md 4 -fd 0 -mr 0
sleep 1

#multi without comm
#s1
./hsmomdp -Gf2 -bi "" -Rff -ud -s localhost ${10} -u ${12}_nc_op_s1 -of run$1_walker.txt -sM -Ns $7 -Sn $5 -C -Sp start$1.txt -A $2 -dl $4 -ns $8 -ni $9 -bz ${11} -us 3 -d 1 -bf -em 25 -mo 0.6 -en 10 -md 4 -fd 0 -mr 0 -nc &
sleep 1
#s2
./hsmomdp -Gf2 -bi "" -Rff -ud -s localhost ${10} -u ${12}_nc_op_s2 -of run$1_walker.txt -sM -Ns $7 -Sn $5 -C -Sp start2_$1.txt -A $2 -dl $4 -ns $8 -ni $9 -bz ${11} -us 3 -d 1 -bf -em 25 -mo 0.6 -en 10 -md 4 -fd 0 -mr 0 -nc
sleep 1



fi


