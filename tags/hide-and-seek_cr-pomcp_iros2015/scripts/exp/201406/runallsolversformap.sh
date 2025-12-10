#!/bin/sh

# params:
# 1. run number
# 2. map ID
# 3. map full path
# 4. dist matrix file
# 5. num steps
# 6. num. sim.
# 7. num. belief points
# 8. port
# 9. belief zoom
# 10. exp name/user name

#generate step files
sforw=$(($5+2))
./hsautohider -create 10 $3 run$1_walker.txt 1 $sforw C
#first step for hider
./hsautohider -create 12 $3 start$1.txt 1 1 C

#now noise
./runallsolvers.sh $1 $2 $3 $4 $5 0 0.1 $6 $7 $8 $9 ${10}
./runallsolvers.sh $1 $2 $3 $4 $5 0 1 $6 $7 $8 $9 ${10}
./runallsolvers.sh $1 $2 $3 $4 $5 0 10 $6 $7 $8 $9 ${10}

#starting 0 auto walkers, 0 noise
./runallsolvers.sh $1 $2 $3 $4 $5 0 0 $6 $7 $8 $9 ${10}

#increase auto walkers
#./hsautohider -create 10 $3 run$1_autowalkers.txt 1 $sforw C
#./runallsolvers.sh $1 $2 $3 $4 $5 1 0 $6 $7 $8 $9 ${10}
./hsautohider -create 10 $3 run$1_autowalkers.txt 10 $sforw C
./runallsolvers.sh $1 $2 $3 $4 $5 10 0 $6 $7 $8 $9 ${10}
./hsautohider -create 10 $3 run$1_autowalkers.txt 50 $sforw C
./runallsolvers.sh $1 $2 $3 $4 $5 50 0 $6 $7 $8 $9 ${10}

#auto walkers + noise
#./hsautohider -create 10 $3 run$1_autowalkers.txt 10 $sforw C
#./runallsolvers.sh $1 $2 $3 $4 $5 10 1 $6 $7 $8 $9 ${10}
#./hsautohider -create 10 $3 run$1_autowalkers.txt 50 $sforw C
#./runallsolvers.sh $1 $2 $3 $4 $5 50 10 $6 $7 $8 $9 ${10}
