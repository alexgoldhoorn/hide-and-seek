README HIDE&SEEK
================

First compile APPL-0.95:
- go to include/appl-0.95
- do: make

LIBRARY VERION
--------------
To build the library:
cd build
cmake ..
make


Requires: g++ 4.8 or newer, iriutils, APPL 0.95 (in include), OpenCV 2.3 (in include)



CONSOLE SIMULATOR VERSION
-------------------------
To build the console command and server you need Qt 
(Ubuntu version or newest at: http://qt-project.org/downloads)
The Qt Creator Project files are:
- hsmomdp.pro: the console version to start the automated seeker
- hsserver.pro: the server for the hide&seek game
- actiongen.pro: random action generator (for automated random hider)i
In order to run the game in simulation (either against another player or an automated player) you need to install a MySQL server (http://www.mysql.com, or Ubuntu version; MySQL server 5.6 or newer (I used 5.6.14), note: by default Ubuntu (until 13.10) installs version 5.5 which does NOT support millisecond precision).
Then create the database with the SQL script sql/servergamelog.sql, use the command:
 mysql -u root -p < sql/servergamelog.sql)


REQUIREMENTS:
- g++ v4.8
- iriutils
- Qt 5 (not when building the library with cmake)
- MySQL 5.6 or newer
- Qt - MySQL Lib: libqt5-sql-mysql (use Synaptic Package Manager)
- OpenCV 2.3
- APPL 0.95 (in include/)
- Eigen v3





----- run with ROS:

1. roscore
2. rosrun rqt_gui rqt_gui (open dynamic reconfigure plugin)

export ROBOT=tibi; export OTHER_ROBOT=dabo; export ROS_MODE=sim

2. roslaunch tibi_dabo_faf tibi_dabo_faf.launch

3. roslaunch tibi_dabo_tag_people tibi_dabo_tag_people.launch

export ROBOT=dabo; export OTHER_ROBOT=tibi; export ROS_MODE=sim

4. roslaunch tibi_dabo_faf tibi_dabo_faf.launch
5. roslaunch tibi_dabo_tag_people tibi_dabo_tag_people.launch

dynamic_reconfigure:
faf: START, DEBUG OFF
tag_people: obs_mode MANUAL, give x,y values

pues en otra terminal
6:
export ROBOT=dabo; OTHER_ROBOT=tibi

y roslaunch tibi_dabo_faf tibi_dabo_faf.launch
y 7, lo mismo, export robot etc, y lo mismo que en 3







+-----------------------------------+
| Automatic Seeker Hide&Seek Client |
+-----------------------------------+

hsmomdp [-t | -ts | -tm | -tc | -tC | -tg | -tp | -h | -v] [ -Gh | -Gf[2|m] ] [-so | -sl | -slc | -sm | -ss | -s[2|M][c|d] | -sc[c|d|p] | -sf[e] | -sC[c|h|H] | -sh ] [-Rf | -Rc | -Rt | -Rff | -Rfd ] [-D discount] [-gt | -gb | -gk [k] | -g[r|c][x] [d a b h]] [-m momdp-file] [-p policy-file] [-a[s|p] map-file] [-A map-id] [-b row col] [-u username] [-s server port] [-PP | -PA] [-oh | -or | -ow | -os | -ov | -oa | -oA | -ox | -of [file]] -dc [-L log-prefix] [-um | -uf] [-pt target-prec] [-pi target-init-fac] [-pd] [-pp] [-rs|-ra|-rn|-rx] [-d max_depth] [-M max-mem] [-w win-dist] [-ni num_init_belief_points -ns num_sim x expl_const -e expand_cnt -d max_search_depth [-hsr -hsb -hss]] [-dc] [-sss[a|x|n]] [-ssm max_cells] [-I[r|b|d|f] [dist | r c]] [ -rp[r|s] ] [-MS max_steps | -MT max-time-sec] [-ud] [-k[r|s|x] [n] | -kf file] [-f[w|t]] [-S[n] [num-steps] [-d[w|l] file] [-us steps] [-c comments] [-Ns std-dev | -Nfn false-neg-prob | -Nfp false-pos-prob] [-Sp start-pos-file] [-ppf dest-file | -pph horiz-time] | -bi belief-img | -ew utilw distw bel_w | -em expl_max_r | -en expl_n_bel | -mo own_obs_p | -md pos-small-d] [-bf] | [-fd follow-dist] | [-nc] | [-ld] | -np num-players-req
   -t                test manual input of data
   -ts [belief-file] test segmentation, if belief-file not given than random number used
   -tm               test map, show the map
   -t[c|C]           test POMCP simulator / MCTS
   -tp               test people prediction
   -tg               show Tree GUI
   -t[S|D|N]         S/D: use SeekerHS (for testing); D: manually add tracks; N: don't send second seeker
   -h                this help
   -G[h|f[2|m]]      game of hide-and-seek or find-and-follow (-Gf2: for 2 seekers; -Gfm: for multiple)
   -P[P|A]           path planner: propagation (default) / A*
   -v                shows version and build
   -so               solver: uses offline learned policy
   -sl               solver: layered MOMDP
   -slc              solver: layered MOMDP, but compare with offline
   -sm               solver: MCVI (offline) [not implemented yet]
   -sc[d|c|p]        solver: POMCP (online) uses MCTS (discrete/continuous states/continuous with predictor)
   -ss               smart seeker
   -sf[e]            follower / exact: when not visible, goes to last point
   -sh[d|c|p]        solver: follower of highest belief of POMCP (discrete/continuous states/continous with prediction)
   -sC               solver: combi POMCP and follower
   -sCc              solver: combi POMCP and follower, with continuous states
   -sCh              solver: combi highest POMCP belief follower and follower (cont. states)
   -sCH              solver: combi highest POMCP belief follower and follower (cont. states and person prediction)
   -s2[c|d]          solver: muti (2) robot highest POMCP belief explorer (c/d: continuous/discrete)
   -sM[c|d]          solver: muti robot highest POMCP belief explorer (c/d: continuous/discrete)
   -W map_file       write map to map file
   -R[f|c|t]         Rewards: f=final, c=final and crossing, t=triangle
   -Rf[f|d]          Rewards Find&Follow: fs=simple, fd=rev. distance
   -D discount       discount factor (gamma)
   -bl               use best action look ahead
   -m momdp-file     MOMDP file or map name (without extension - should be on server!)
   -p policy-file    policy file
   -a[s] file        load map file, -as: send map to server
   -ap file zoom-f   load map PGM file, using zoom-factor
   -ay file cell-sz  load map PGM file, passing cell size (m) and map size of rowD x colD m
       rowD colD     
   -A [map-id | ?]   map ID (? shows list of maps and IDs)
   -b row col        base (row,col), overwrites base in file
   -u user           username of the player in the game
   -s server port    server and port (if 0 then no server is used; default: localhost:1120)
   -o[h|r|s|v|a|w]   opponent: Human/Random/Smart hider/Very Smart Hider/All knowing hider/Random walker (has goals)
     -o[A|2|T]       All know. Very Smart H./Very Smart H. v2/All know. Very Sm. H. v2
     -ol act-file    Action list with file name (no path)
     -of pos-file    Position file
     -ox             opponent pos random fixed
   -L log-file-pre   log file prefix

                     The folowing parameters are for the online solver only:
   -T timeout        solver time out in seconds (default: 0, not taken into account)
   -Tr               set top reward (if not then uses average of bottom)
   -Tf               set top final state (if not uses average of bottom)
   -gb | -gk [k] |   segmenter: basic (3 segments: <0,=0,>0) / k-means (default k based on map size and #obstacles) /
   -gr [d a b h]     robot centered with 'circles'/'squares' of a certain distance d and crossing with certian angle a,
                     center node has radius b (0=only center node), and h is de radius from which no segmentation is done
                     (i.e. high resolution cells are taken)
   -gc [d a b h]     combines robot centered and base centered, same params as -gr
   -grx [d a b h]    same as -gr, but for the X states (i.e. robot's position)
   -gvr|-gvb|-gvbr   segment value: reward only / belief only / belief*reward
   -gt               segmenter test (uses same 'segmentation' as map)
   -um | -uf         initializer upper bound (see APPL): (Q)MDP (-um) or FIB (-uf, default)
   -pd               pruning disabled, i.e. no pruning (for debug purpuse)
   -pp               print (APPL solver) parameters before solving
   -rs|-ra|-rn|-rx   top reward aggregation: sum / average / min / max
   -d max-depth      maximum depth to search belief tree
   -M max-memory-MB  maximum memory to be used by SARSOP (MB)
   -w win_dist       maximum distance to hider for seeker to win (default: 0)
   -ni numb_init_bel number of init beliefs, if 0 (default) all belief points of init will be used
   -ns numb_sims     number of simulations
   -x expl_constant  exploration constant
   -e expand_count   expand count (after so many counts node is expanded)
   -dc               disable consistency check / allow inconcistencies
   -ssa[a|x|n]       smart seeker score type for actions opponents: average/max/min
   -ssh[a|x|n]       smart seeker score type for hidden positions: average/max/min
   -ssm max_cells    smart seeker max. cells to calculate when hidden (random choice)
   -hs[r|b|s] [rp]   hider simulator: random/directly to base/smart, rp=prob. of using random action
   -sci              set init node value for POMCP (default false)
   -E[s|n]           expected reward count: sum (default) or normalized
   -I[r|b]           set initial position to be random for both, or random for the hider and seeker for the base
   -Id dist          set initial to be random at a given distance d from the base
   -If r c           set the initial position to be fixed: (r,c)
   -rp[r|s]          roll-out policy: random/smart(seeker)
   -csn[s|h]         std_dev continuous std. dev. for next step seeker / hider (default = 0.2 / 0.3)
   -cso[s|h]         std_dev continuous std. dev. for observation seeker / hide (default = 0.1 / 0.1)
   -cpf[p|n]         probability for false positive / negative of detecting a person
   -cpip             probability for incorrect positive of detecting a person (default = 0.001)
   -cppn             next hider halt probability (default = 0.1)
   -cppp             use prediction step for next pos. probability (default = 0.4)
   -cpiu             probability for belief update to accept inconsistent (with obs) steps (default = 0.3)
   -C                continuous space
   -t[S|D]           Use SeekerHS to test it, D: debug, enter tracks manually
   -MS max-steps     max. steps
   -MT max-time-sec  max. time in seconds
   -ud               use deduced action
   -k[r|s|g|x] [n]   automated walkers/dyn. obstacles: random / social force model / random goal / fixed pos; n walkers (default 1)
   -kf file          automated file reading data from file
   -f[w|t]           filter all possible hider poses: weighted score / tag only
   -fs num-it        filter input tracks at maximum num-it iterations (using observation score and distance)
   -S[n] num-steps   stop after num-steps (0 is default, not stopping); -Sn: do not stop after win
   -d[w|l] file      write/load distance file
   -bz zoom-fac      belief zoom factor
   -us update-steps  number of steps after which the highest belief is checked and goal is updated
   -c comments       comments, use quotes (") when writing spaces
   -Ns std-dev       noise std.dev. for osbervation of hider's pos. sent to seeker
   -Nfn false-neg-p. false negative prob. for obs. of hider's pos sent to seeker
   -Nfp false-pos-p. false positive prob. for obs. of hider's pos sent to seeker
   -Sp start-posfile start pos file (made with autohider file type 
   -ppf dest-file    destination file for people preditor
   -pph t-horizon    time-horizon for people predictor
   -bi belief-image  image file of belief (png, default: out.png)
   -ew uw dw bw      set weights for explorer formula: util, distance, belief weight
   -em expl-max-r    explorer max range
   -en expl-n-belp   explorer use n belief points
   -md pos-small-d   multi: small distance for selecting poses
   -mo own-obs-p     multi: own observation probability
   -bf               filter belief in update retrieving from root (important for multi)
   -fd follow-dist   follow distance (default: 1)
   -mr min-rob-dist  min. robot distance (default: 1)
   -nc               no communication, i.e. don't use other's observations
   -ld               disable learning for POMCP, only belief update
   -np num-pl-req    number of players required

