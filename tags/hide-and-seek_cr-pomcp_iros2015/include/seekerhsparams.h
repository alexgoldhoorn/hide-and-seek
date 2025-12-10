#ifndef SEEKERHSPARAMS_H
#define SEEKERHSPARAMS_H

#include <ostream>
#include <cmath>

#include "HSGame/pos.h"

#ifdef USE_QT
//#include <QDataStream>
#endif

/*!
 * \brief The SeekerHSParams struct Parameters for the file
 */
struct SeekerHSParams {
    //! init
    SeekerHSParams();
    //! print variables
    void printVariables(std::ostream& stream, bool showExtraParams=false);
#ifndef DO_NOT_USE_MOMDP
    //! set solver params for APPL
    void setSolverParams();
#endif

    //solver types
    static const char SOLVER_NOT_SET = 0;
    static const char SOLVER_OFFLINE = 1;
    static const char SOLVER_LAYERED = 2;
    static const char SOLVER_LAYERED_COMPARE = 3;
    static const char SOLVER_MCVI_OFFLINE = 4;
    static const char SOLVER_POMCP = 5;
    //static const char SOLVER_POMCP_CONT = 6;
    static const char SOLVER_COMBI_POMCP_FOLLOWER = 10;
    //static const char SOLVER_COMBI_POMCP_FOLLOWER_CONT = 11;    

    //NOTE: these are not solvers, only heuristics
    static const char SOLVER_FOLLOWER = 8;
    static const char SOLVER_FOLLOWER_LAST_POS = 7;
    static const char SOLVER_SMART_SEEKER = 9;

    static const char SOLVER_FOLLOWER_LAST_POS_EXACT = 12;
    static const char SOLVER_FOLLOWER_HIGHEST_BELIEF = 13;
    static const char SOLVER_COMBI_FOLLOWER_HIGHEST_BELIEF = 14;

    static const char SOLVER_MULTI_HB_EXPL = 15;


    //segmenter type
    static const char SEGMENTER_NOT_SET = 0;
    static const char SEGMENTER_BASIC = 1;
    static const char SEGMENTER_KMEANS = 2;
    static const char SEGMENTER_ROBOT_CENTERED = 3;
    static const char SEGMENTER_TEST = 4;
    static const char SEGMENTER_CENTERED_COMBINED = 5;

    //upper bound init
    static const char UPPERBOUND_INIT_FIB = 0;
    static const char UPPERBOUND_INIT_MDP = 1;

    //reward types
    static const char REWARD_NOT_SET = 0;
    static const char REWARD_TRIANGLE = 3;
    static const char REWARD_FINAL = 4; //only final state (1 or -1)
    static const char REWARD_FINAL_CROSS = 14; //idem and 1 if both players cross eachother
    //AG140212: rewards for find-and-follow
    static const char REWARD_FIND_SIMPLE = 21;
    static const char REWARD_FIND_REV_DIST = 22;

    /* reward types, as defined in CreateMOMDP?.m (note only few used here)
                   - 1=only final states WIN score = L
                                         LOSE score =  -2L
                   - 2=all states:       WIN score = 2L
                                         LOSE score = -3L
                                         other score = (based distances)
                    - 3=all states
                    - 4=only final states: WIN = 1 / LOSE = -1
                    - 5=only final states win only: WIN = 1
                    - 6=only final states: WIN = 1 / LOSE = -0.2
    */

    //default IP
    static const int DEFAULT_SERVER_PORT = 1120;
    //static char* DEFAULT_SERVER_IP = "localhost";

    //simulated hider type
    static const char SIM_HIDER_TYPE_RANDOM = 0;
    static const char SIM_HIDER_TYPE_TOBASE = 1;
    static const char SIM_HIDER_TYPE_SMART = 2;
    //static const char SIM_HIDER_TYPE_PREDICT = 3;

    //expected reward calc.
    static const char EXPECTED_REWARD_CALC_SUM = 0;
    static const char EXPECTED_REWARD_CALC_NORM = 1; //normalization

    //rollout policy type
    static const char ROLL_OUT_POLICY_TYPE_RANDOM = 0;
    static const char ROLL_OUT_POLICY_TYPE_SMART = 1;

    //pomcp sim type
    static const char POMCP_SIM_DISCR = 0;
    static const char POMCP_SIM_CONT = 1;
    static const char POMCP_SIM_PRED = 2;


    //HSSimulatorPred
    static constexpr double CONT_NEXT_HIDER_HALT_PROB      = 0.1;
    static constexpr double CONT_USE_HIDER_PRED_STEP_PROB  = 0.4;
    //HSSimulatorCont
    static constexpr double CONT_NEXT_SEEKER_STATE_STDDEV   = 0.2;  // 0.05;
    static constexpr double CONT_NEXT_HIDER_STATE_STDDEV    = 0.3;  //0.05;
    static constexpr double CONT_SEEKER_OBS_STDDEV          = 0.1;  //0.2;
    static constexpr double CONT_HIDER_OBS_STDDEV           = 0.1;  //0.2;
    static constexpr double CONT_FALSE_POS_PROB             = 0.001; //0 in real
    static constexpr double CONT_FALSE_NEG_PROB             = 0.3;   //0.001;
    static constexpr double CONT_INCOR_POS_PROB             = 0.001; // 0 in real
    static constexpr double CONT_OBS_IF_NOT_VISIB_PROB      = 0.01;  //same
    static constexpr double CONT_UPD_INCONSIST_ACCEPT_PROB  = 0.3;

    //MultiSeekerHBExplorer
    static constexpr double MSHB_OBS_EQUALS_THRESH_DIST     = 1.0; //grid cells
    static constexpr double MSHB_MULTI_SEEKER_OWN_OBS_CHOOSE_PROB = 0.6;
    static constexpr double MSHB_MULTI_SEEKER_VIS_OBS_CHOOSE_PROB_IF_MAYBE_DYN_OBST = 0.75;

    static constexpr double MSHB_MULTI_SEEKER_EXPLORER_UTIL_WEIGHT = 0.4; //alpha
    static constexpr double MSHB_MULTI_SEEKER_EXPLORER_DIST_WEIGHT = 0.4; //0.08;  //beta
                            //(10 token as max_dist, make it dependend on map size makes it really small for big maps)
    static constexpr double MSHB_MULTI_SEEKER_EXPLORER_BELIEF_WEIGHT = 0.2; //gamma

    static const int MSHB_MULTI_SEEKER_EXPLORER_CHECK_N_POINTS = 5;
    static constexpr double MSHB_MULTI_SEEKER_EXPLORER_MAX_RANGE = 25;
    static constexpr double MSHB_MULTI_SEEKER_SELECT_POS_SMALL_DIST_BETW_GOALS = 4;
    static constexpr double MSHB_MULTI_SEEKER_SELECT_POS_BELIEF_DIFF_BETW_GOALS = 0.4;    

    static constexpr double FOLLOW_PERSON_DIST = 1.0;
    static constexpr double MIN_DIST_BETW_ROBOTS = 1.0;
    //static constexpr double MULTI_SEEKER_FOLLOW_ANGLE_RAD = M_PI/2;

    //Path planner type
    static const char PATHPLANNER_PROPDIST = 0;
    static const char PATHPLANNER_ASTAR = 1;


    // -- files --

    //! map file name
    std::string mapFile;
    //! pomdp file name
    std::string pomdpFile;
    //! policy file name
    std::string policyFile;
    //! log file name
    std::string logFile;
    //! time log file name (csv)
    std::string timeLogFile;
    //! game log file
    std::string gameLogFile;
    //AG140310
    //! belief image output
    std::string beliefImageFile;
    //AG140915
    //! people prediction destination file
    std::string ppDestinationFile;

    //AG140613
    std::string seekerPosFile;

    //AG140521
    //! map distance matrix file name
    std::string mapDistanceMatrixFile;

    //! experiment name
    std::string expName;

    //! solver type
    char solverType;

    //! game type
    char gameType;

    //! stop when we have a winner
    bool stopAtWin;

    //AG150319
    //! path planner type
    char pathPlannerType;

    //AG140519
    //! stop after the number of steps, if 0, not used
    unsigned int stopAfterNumSteps;

    //AG130613: reward type (like Matlab createMOMDPfile5.m)
    //! reward type
    char rewardType;

    //! discount
    double discount;

    //AG130613: game time
    //! max game time, after which ends in a tie (0=endless)
    long maxGameTime;

    //! max num actions
    long maxNumActions;

    //! win distance (to hider, default 0 i.e. on same place)
    double winDist;

    //! grid cell width (m) in the real world
    double cellSizeM;

    //AG140403
    //! use deduced action, using previous and current Pos
    bool useDeducedAction;

    // -- online parameters --
    //! upper bound init type
    char ubInitType;
    //! target precision
    double targetPrecision;
    //! target init precision factor
    double targetInitPrecFact;

    //ag130503
    //! set final states in the top model
    bool setFinalStateOnTop;
    //! set simple reward on top
    bool setFinalTopRewards;


    // segmenter type
    //! segmenter type
    char segmenterType;
    //! segmenter X type
    char segmenterTypeX;
    //! segment value type;
    char segmentValueType; // = LayeredHSMOMDP::SEGMENT_VALUE_BELIEF_ONLY;
    //! top reward aggregation
    char topRewAggr; // = LayeredHSMOMDP::TOP_REWARD_SUM;
    //! k for k-nearest neighbour segmentation
    int k;

    //robot centered segmentation params
    double rcSegmDist;
    double rcAngDist;
    double rcBaseRad;
    double rcHighResR;
    //robot centered segmentation X params
    double rcXSegmDist;
    double rcXAngDist;
    double rcXBaseRad;
    double rcXHighResR;


    // APPL options
    //! use best action look ahead
    bool useLookahead;
    //! time out for learning policy
    double timeoutSeconds;
    //! maximum search depth in tree
    int maxTreeDepth;
    //! do pruning or not
    bool doPruning;
    //! memory limit (MB), 0 is no limit
    unsigned long memoryLimit;
    //! show params
    bool showParams;


    // parameters for simulation / connection
    //! server IP
    std::string serverIP;
    //! server port
    int serverPort;
    //! username for database
    std::string userName;
    //! map ID
    int mapID;

    // opponent type and params
    //! opponent type
    int opponentType;
    //! opponent action file
    std::string oppActionFile;
    //! opponent std of noise (used by smart)
    double oppHiderNoiseStd;

    //AG140426: auto walkers (/dynamic obstacles)
    //! auto walker type
    int autoWalkerType;
    //! number of auto walkers
    unsigned int autoWalkerN;
    //! check collision dyn obstacles/autowalker
    bool checkDynamicObstacleCollisions;
    //! auto walker file
    std::string autoWalkerPosFile;

    //POMCP params
    //! number of initial belief states
    unsigned int numBeliefStates;
    //! number of simulations
    unsigned int numSim;
    //! maximum search depth in tree
    //int maxTreeDepth;
    //! exploration constant
    double explorationConst;
    //! max expand count
    /*unsigned*/ int expandCount;
    //! in simulator hider type
    char simHiderType;
    //! probab. of using random action of hider
    double simHiderRandomActProb;
    //! set init node value
    bool setInitNodeValue;
    //! expected reward type: sum (default) or 'normalized'
    char expectedRewardCalc;
    //! use weighted belief points (i.e. each belief has a 'weigth' / 'counter')
    bool useWeightedBeliefs;
    //! rollout policy type (default: random)
    char rolloutPolicyType;
    //! POMCP sim type
    char pomcpSimType;
    //! check consistency for newly generated belief
    bool pomcpCheckNewBeliefGen;
    //parameters for continuous movements in simulator
    //! std. dev. next state seeker
    double contNextSeekerStateStdDev;
    //! std. dev. next state hider
    double contNextHiderStateStdDev;
    //! std. dev. next state hider if random movement
    //double contNextHiderStateRandStdDev;
    //! std. dev. observation seeker
    double contSeekerObsStdDev;
    //! std. dev. observation hider
    double contHiderObsStdDev;
    //! false positive of a person detection probability, ie person is seen in any place when it is invisible (in its state)
    double contFalsePosProb;
    //! false negative of a person detection probability, ie person is visible, but detection is invisble
    double contFalseNegProb;
    //AG140514
    //! incorrect detection probability, i.e. seeing a completely different detection
    double contIncorPosProb;
    //AG150319
    //! probability of accepting inconsistent states in belief update
    double contUpdInconsistAcceptProb;
    //AG140508
    //! probability observing hider pos even if not visible
    double contObserveIfNotVisibProb;
    //AG140908
    //! probability of doing the halt action, used for continuous person step prediction (HSSimulatorPred)
    double contNextHiderHaltProb;
    //! probability of using the predicted step, used for continuous person step prediction (HSSimulatorPred)
    double contUseHiderPredStepProb;

    //AG140317
    //! consistency check seeker pos: (in HSSimulatorCont) distance between state and obs, such that it is marked as
    //! consistent, default is 3 x contSeekerObstStdev
    double contConsistCheckSeekerDist;
    //! consistency check hider pos
    double contConsistCheckHiderDist;

    //AG130722
    //! allow inconsistent observations (i.e. observation which is position futher than 1 step from prev time)
    bool allowInconsistObs;
    //! also observe own position
    bool ownPosObs;

    //AG130724
    //! Smart seeker score type: avg/sum/max, when hidden
    char smartSeekerScoreHiddenType;
    //! Smart seeker score type: avg/sum/max, when action
    char smartSeekerScoreActionType;
    //! Smart seeker max number of positions for which the score has to be calculated when
    //! it the hider is hidden (0=all)
    unsigned int smartSeekerMaxCalcsWhenHidden;

    //AG131220
    //! init pos type
    char initPosType;
    //! init pos distance to base
    int randomPosDistToBase;
    //! auto hider random pos distance to base
    int randomPosDistToBaseHider;
    //! init pos fixed
    Pos* initPosFixed;

    //! use continuous positions, if not int (default)
    bool useContinuousPos;
    //! is seeker, otherwise hider
    bool isSeeker;
    //! min distance to obstacle
    double minDistToObstacle;
    //AG140509
    //! min distance to dynamic obstacle obstacle/person
    double minDistToDynObstacle;

    //AG140212
    //! propogate first goal of real robot to increase speed
    //double propFirstStepLength;

    // only in seekerHS
    //! score of hidden observation
    double filterHiddenBaseScore;

    //! base score of tag
    double filterTagBaseScore;

    //! base score of laser
    double filterLaserBaseScore;

    //! min filter score
    double filterMinScore;

    //! score type: 0 is old one, 1 is newer
    char filterScoreType;

    //! follow hider ID (ID of tracker)
    int filterFollowID;

    //! filter max distance for distance score
    double filterDistScoreMaxDist;

    //! use next n pos
    int useNextNPos;

    //! belief image cell width
    int beliefImageCellWidth;

    //! in seekerHS set hider's observation to not visible if raytracing gives a 'not visible'
    bool simulateNotVisible;

    //AG140531
    //! simulation observation noise std.dev.
    double simObsNoiseStd;
    //AG140605
    //! simulation observation false negative probab.
    double simObsFalseNegProb;
    //! simulation observation false positive probab.
    double simObsFalsePosProb;

    //AG140508
    //! do a visibility check
    bool doVisibCheckBeforeMove;

    //! filter auto belief at update
    bool pomcpFilterBeliefAtUpdate;
    //! check when generating new belief points at update
    bool pomcpCheckWhenGenBeliefAtUpdate;
    //! max number of tries in sim. to get a random hider position using direction, otherwise using actions
    unsigned int pomcpSimMaxNumTriesHiderPos;

    //! min time (ms) between two iterations
    unsigned long minTimeBetweenIterations_ms;


    //AG140326
    //! movement distance of seeker (default 1 grid cell; when continuous this is the distance in any direction)
    double seekerStepDistance;

    //! movement distance of hider (default 1 grid cell)
    double hiderStepDistance;

    //AG150219
    //! movement distance of hider, below which it is asumed to stand still
    double hiderNotMovingDistance;

    //! the part of the seekerStepDistance which is used to deduce whether an action was 'halt' or movement
    double seekerStepDistancePartForHaltActionDeduction;

    //AG140512
    //! simulate that all tracks (hider and all other persons/dyn. obst) are sent to hider, and filter them
    bool simulateReceivingAllTracksAsPersons;

    //! number of iterations the filter can stop (if 0, not used)
    unsigned int filterCanStopNumberOfIterations;

    //! time after which the goal is updated
    double highBeliefFollowerUpdateGoalTime_ms;
    //! number of steps after which the goal is updated (Only for simulation)
    unsigned int highBeliefFollowerUpdateGoalNumSteps;
    //! max distance from robot where highest belief is searched (grid cells)
    double highBeliefFollowerHighestDist;
    //! for simulations force that only steps are sent instead of far away goals
    bool onlySendStepGoals;
    //! method used to calculate distance for next goal (used by HighestBeliefFollower): shortest path, otherwise euclidian
    bool searchGoalByShortestPathDist;

    //! zoom factor belief map
    double beliefMapZoomFactor;

    //! max diff orientation (rad)
    double maxDiffOrientation_rad;

    //AG140526
    //! take the dyn. obst. occlusion into account when learning
    bool takeDynObstOcclusionIntoAccountWhenLearning;

    //AG140915
    //! people prediction, numb. future steps (horizon)
    unsigned int ppHorizonSteps;

    //AG150128
    //! the maximum distance between two observation (e.g. of hider positions) to be considered equal
    double obsEqualsThreshDist;
    //! probability of choosing own observation, when using two seekers, and they have inconsistent values (i.e. not equal)
    double multiSeekerOwnObsChooseProb;
    //! Probability of choosing own observation, when using two seekers, and they have inconsistent values (i.e. not equal)
    //! of which one is a hidden one, but should seen (by raytracing) see the person. Could be because of dynamic obstacles.
    double multiSeekerVisObsChooseProbIfMaybeDynObst;

    //AG150129
    //! multi seeker explorer utility weight
    double multiSeekerExplorerUtilWeight;
    //! multi seeker explorer's weight of distance to potential targets (highest belief) (beta)
    double multiSeekerExplorerDistWeight;    
    //! multi seeker explorer's weight of belief of highest beliefs (gamma)
    double multiSeekerExplorerBeliefWeight;
    //! multi seeker explorer: how many highest belief points are checked
    int multiSeekerExplorerCheckNPoints;
    //! the maximum range which one robot can 'see', used to select locations to explore
    double multiSeekerExplorerMaxRange;    

    //AG150130
    //! distance to follow the person
    double followPersonDist;
    //! angle (rad) between the lines created by seeker 1 - hider and seeker 2 - hider
    //double multiSeekerFollowAngle_rad; //AG150221: depricated to minDistBetweenRobots
    //! min dist between robots
    double minDistBetweenRobots;
    //! threshold for distance between goal poses calculated by the different seekers
    double multiSeekerSelectPosSmallDistBetwGoals;
    //! threshold for (large) belief difference between goals
    double multiSeekerSelectPosBeliefDiffBetwGoals;
    //! for comparing goals of both seekers, use the maximum, otherwise use the sum
    bool multiSeekerSelectPosUseMaxBelief;
    //AG150217: was debugDontSendSecondSeeker
    //! disable communication (i.e. don't user other's observations)
    bool multiSeekerNoCommunication;
    //AG150318
    //! when using POMCP, do not learn, only propogate the belief
    bool pomcpDoNotLearn;

    //AG150122
    //! check whether 2 seekers are used (uses gameType)
    bool gameHas2Seekers();

    //! number of players allowed/required for the game
    unsigned int numberOfPlayers();

#ifdef USE_QT
    //AG140506: functions to read/write params between server and client
    //NOTE: this is not sending ALL params yet, only the ones that are used by the server/client

    /*!
     * \brief readFromStream reads *some* params (ones that are required by server/client) from stream
     * \param in
     */
    //void readFromStream(QDataStream& in);

    /*!
     * \brief writeToStream writes *some* params (ones that are required by server/client) to stream
     * \param out
     */
    //void writeToStream(QDataStream& out);
#endif


};


#endif // SEEKERHSPARAMS_H
