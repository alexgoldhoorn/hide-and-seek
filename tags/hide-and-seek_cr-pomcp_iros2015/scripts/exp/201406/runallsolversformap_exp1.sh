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

#starting 0 auto walkers, 0 noise
./runallsolvers.sh $1 $2 $3 $4 $5 0 0 $6 $7 $8 $9

#increase auto walkers
./runallsolvers.sh $1 $2 $3 $4 $5 1 0 $6 $7 $8 $9
./runallsolvers.sh $1 $2 $3 $4 $5 5 0 $6 $7 $8 $9
./runallsolvers.sh $1 $2 $3 $4 $5 10 0 $6 $7 $8 $9
./runallsolvers.sh $1 $2 $3 $4 $5 50 0 $6 $7 $8 $9

#now noise
./runallsolvers.sh $1 $2 $3 $4 $5 0 0.1 $6 $7 $8 $9
./runallsolvers.sh $1 $2 $3 $4 $5 0 0.5 $6 $7 $8 $9
./runallsolvers.sh $1 $2 $3 $4 $5 0 1 $6 $7 $8 $9
./runallsolvers.sh $1 $2 $3 $4 $5 0 10 $6 $7 $8 $9

#auto walkers + noise
./runallsolvers.sh $1 $2 $3 $4 $5 10 1 $6 $7 $8 $9
./runallsolvers.sh $1 $2 $3 $4 $5 50 10 $6 $7 $8 $9
