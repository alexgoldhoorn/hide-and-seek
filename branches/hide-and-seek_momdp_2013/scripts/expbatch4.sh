#!/bin/sh


#./runAllOfflinePolicy3.py pomdpx/finstate_r3t1/ pomdpx/finstate_r3t1/ finstate_r3t1_3/ finstate_r3t3 100 -or

./runAllOfflinePolicy3.py pomdpx/finstate_r3t1/ pomdpx/finstate_r3t1/ finstate_r3t1_3/ finstate_r3t3 10 -or

echo "Reward 3, type 1"
#./runAllOfflinePolicy3.py pomdpx/rew3maxtype1/ policy/rew3maxtype1/ rew3maxtype1_3/ nfs_r3t3 100 -or

./runAllOfflinePolicy3.py pomdpx/rew3maxtype1/ policy/rew3maxtype1/ rew3maxtype1_3/ nfs_r3t3 10 -or  






