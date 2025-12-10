#!/bin/sh

cd /mnt/ramdisk
ln -s /home/agoldhoorn/MyProjects/Experiments/pomdpx/
ln -s /home/agoldhoorn/MyProjects/Experiments/maps/
ln -s /home/agoldhoorn/MyProjects/Experiments/policy/APPL/ policy

mkdir rew3maxtype1/
mkdir rew3maxtype2/

echo "Reward 3, type 1"
./runAllOfflinePolicy.py pomdpx/rew3maxtype1/ policy/rew3maxtype1/ rew3maxtype1/ 100 -or 
echo "Reward 3, type 2"
./runAllOfflinePolicy.py pomdpx/rew3maxtype2/ policy/rew3maxtype2/ rew3maxtype2/ 100 -or

#TODO: user prefix -> to distinguis r

# add:
#	user prefix
# or in db/code: experiment-id, seeker params/.. 
