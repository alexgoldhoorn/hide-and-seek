#!/bin/sh


#./runAllOfflinePolicy3.py pomdpx/finstate_r3t1/ pomdpx/finstate_r3t1/ finstate_r3t1_3/ finstate_r3t3 100 -or

#./runAllOfflinePolicy4.py pomdpx/newrew_fs_r3t1/ pomdpx/newrew_fs_r3t1/ newrew_fs_r3t1/ newrew_fs_r3t1 100 -or

./runAllOfflinePolicy4.py pomdpx/simplerew_fs/ pomdpx/simplerew_fs/ simplerew/ simplerew1 100 -or

./runAllOfflinePolicy4.py pomdpx/simplerew_fs/ pomdpx/simplerew_fs/ simplerew/ simplerew1 100 -os

./runAllOfflinePolicy4.py pomdpx/newrew_fs_r3t1/ pomdpx/newrew_fs_r3t1/ newrew_fs_r3t1/ newrew_fs_r3t1 100 -os

#echo "Reward 3, type 1"
#./runAllOfflinePolicy3.py pomdpx/rew3maxtype1/ policy/rew3maxtype1/ rew3maxtype1_3/ nfs_r3t3 100 -or

#./runAllOfflinePolicy4.py pomdpx/rew3maxtype1/ policy/rew3maxtype1/ rew3maxtype1_3/ nfs_r3t3 10 -or  






