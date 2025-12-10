#include "Base/seekerhsparams.h"

#ifndef DO_NOT_USE_MOMDP
#include "Solver/hsmomdp_layered.h"
#endif

#include "Smart/smartseeker.h"

#include "Base/autoplayer.h"
#include "AutoHider/smarthider.h"

//#include "POMCP/hspomcp.h"
//#include "POMCP/hssimulatorcont.h"
//#include "POMCP/hssimulatorpred.h"

#include "Utils/generic.h"

#include "Base/hsglobaldata.h"

using namespace  std;

SeekerHSParams::SeekerHSParams() {
    //init all params
    //all files strings already empty
    solverType = SOLVER_NOT_SET;

    gameType = HSGlobalData::GAME_HIDE_AND_SEEK;
    stopAtWin = true;
    stopAfterNumSteps = 0;
    isSeeker = true;

    ubInitType = UPPERBOUND_INIT_FIB;

    winDist = 0;
    cellSizeM = 1;

#ifndef DO_NOT_USE_MOMDP
    targetPrecision = SolverParams::DEFAULT_TARGET_PRECISION;
    targetInitPrecFact = SolverParams::CB_INITIALIZATION_PRECISION_FACTOR;
#endif

    setFinalStateOnTop = false;
    setFinalTopRewards = false;

    segmenterType = SEGMENTER_NOT_SET;
    segmenterTypeX = SEGMENTER_NOT_SET;
#ifndef DO_NOT_USE_MOMDP
    segmentValueType = LayeredHSMOMDP::SEGMENT_VALUE_BELIEF_ONLY;
    topRewAggr = LayeredHSMOMDP::TOP_REWARD_SUM;
#endif
    k = -1;

    rewardType = REWARD_NOT_SET;
    maxGameTime = -1;
    maxNumActions = -1;

    numPlayersInClient = 1;
    numPlayersReq = 2; //AG150528: default 2 is the max (hide&seek)

    rcSegmDist = rcAngDist = -1;
    rcBaseRad = rcHighResR = 0;
    rcXSegmDist = rcXAngDist = -1;
    rcXBaseRad = rcXHighResR = 0;

    useLookahead = false;
    timeoutSeconds = -1;
    maxTreeDepth = -1;
    memoryLimit = 0;
    showParams = false;

    opponentType = -1;
    oppHiderNoiseStd = SmartHider::SCORE_DEFAULT_RAND_STD;
    autoWalkerType = HSGlobalData::AUTOWALKER_NONE;
    autoWalkerN = 0;
    autoWalkerPosFile = "";
    mapID = -1;
    serverPort = DEFAULT_SERVER_PORT;
    serverIP = "localhost"; //DEFAULT_SERVER_IP;
    pathPlannerType = PATHPLANNER_PROPDIST;

    numBeliefStates = 0;
    numSim = 0;
    explorationConst = -1;
    expandCount = -1;
    simHiderType = SIM_HIDER_TYPE_RANDOM;
    simHiderRandomActProb = 0;
    setInitNodeValue = false;
    expectedRewardCalc = EXPECTED_REWARD_CALC_SUM;
    useWeightedBeliefs = true;
    rolloutPolicyType = ROLL_OUT_POLICY_TYPE_RANDOM;
    pomcpFilterBeliefAtUpdate = false;
    pomcpCheckNewBeliefGen = true;
    pomcpSimMaxNumTriesHiderPos = 100;

    allowInconsistObs = false;
    ownPosObs = false;

    smartSeekerScoreHiddenType = SmartSeeker::SCORE_AVG;
    smartSeekerScoreActionType = SmartSeeker::SCORE_AVG;
    smartSeekerMaxCalcsWhenHidden = 0;

    discount = 0.95;

    initPosType = HSGlobalData::INIT_POS_TYPE_BASE;
    randomPosDistToBase = 0;
    randomPosDistToBaseHider = 0;
    initPosFixed = NULL;

    //std
    contNextSeekerStateStdDev = /*pomcp::HSSimulatorCont::*/CONT_NEXT_SEEKER_STATE_STDDEV;
    contNextHiderStateStdDev = /*pomcp::HSSimulatorCont::*/CONT_NEXT_HIDER_STATE_STDDEV;
    contSeekerObsStdDev = /*pomcp::HSSimulatorCont::*/CONT_SEEKER_OBS_STDDEV;
    contHiderObsStdDev = /*pomcp::HSSimulatorCont::*/CONT_HIDER_OBS_STDDEV;
    contFalsePosProb = /*pomcp::HSSimulatorCont::*/CONT_FALSE_POS_PROB;
    contFalseNegProb = /*pomcp::HSSimulatorCont::*/CONT_FALSE_NEG_PROB;
    contObserveIfNotVisibProb = /*pomcp::HSSimulatorCont::*/CONT_OBS_IF_NOT_VISIB_PROB;
    contIncorPosProb = /*pomcp::HSSimulatorCont::*/CONT_INCOR_POS_PROB;
    contUpdInconsistAcceptProb = CONT_UPD_INCONSIST_ACCEPT_PROB;
    contConsistCheckSeekerDist = 0;
    contConsistCheckHiderDist = 0;
    //AG140908
    contNextHiderHaltProb = /*pomcp::HSSimulatorPred::*/CONT_NEXT_HIDER_HALT_PROB;
    contUseHiderPredStepProb = /*pomcp::HSSimulatorPred::*/CONT_USE_HIDER_PRED_STEP_PROB;

    //AG140915
    ppDestinationFile = "";
    ppHorizonSteps = 10;

    useContinuousPos = false;
    minDistToObstacle = 0.8;
    minDistToDynObstacle = 0;
    doVisibCheckBeforeMove = true;
    checkDynamicObstacleCollisions = false;

    //propFirstStepLength = 0;

    beliefImageFile = "out.png";

    filterHiddenBaseScore = 0.1;
    filterTagBaseScore = 1; //1000;
    filterLaserBaseScore = 0.5;
    filterMinScore = 1e-5;
    filterScoreType = HSGlobalData::FILTER_SCORE_USE_TAG_ONLY;
    filterFollowID = -1;
    filterDistScoreMaxDist = 2;
    filterCanStopNumberOfIterations = 0;//0;

    useNextNPos = 1;
    beliefImageCellWidth = 20;

    simulateNotVisible = true; //false;
    simObsNoiseStd = 0;
    simObsFalseNegProb = 0;
    simObsFalsePosProb = 0;

    minTimeBetweenIterations_ms = 500;

    seekerStepDistance = 1;
    hiderStepDistance = 1;

    hiderNotMovingDistance = 0.5*hiderStepDistance;

    seekerStepDistancePartForHaltActionDeduction = 0.25;

    useDeducedAction = true; //false;

    pomcpSimType = POMCP_SIM_DISCR;

    simulateReceivingAllTracksAsPersons = true;

    highBeliefFollowerUpdateGoalTime_ms = 0;
    highBeliefFollowerUpdateGoalNumSteps = 0;
    highBeliefFollowerHighestDist = 0;
    searchGoalByShortestPathDist = true;

    beliefMapZoomFactor = 1;

    onlySendStepGoals = true;

    maxDiffOrientation_rad = 45*M_PI/180.0; //45º

    takeDynObstOcclusionIntoAccountWhenLearning = false;

    obsEqualsThreshDist = MSHB_OBS_EQUALS_THRESH_DIST;
    multiSeekerOwnObsChooseProb = MSHB_MULTI_SEEKER_OWN_OBS_CHOOSE_PROB;
    multiSeekerVisObsChooseProbIfMaybeDynObst = MSHB_MULTI_SEEKER_VIS_OBS_CHOOSE_PROB_IF_MAYBE_DYN_OBST;
    multiSeekerExplorerUtilWeight = MSHB_MULTI_SEEKER_EXPLORER_UTIL_WEIGHT;
    multiSeekerExplorerDistWeight = MSHB_MULTI_SEEKER_EXPLORER_DIST_WEIGHT;
    multiSeekerExplorerBeliefWeight = MSHB_MULTI_SEEKER_EXPLORER_BELIEF_WEIGHT;
    multiSeekerExplorerCheckNPoints = MSHB_MULTI_SEEKER_EXPLORER_CHECK_N_POINTS;
    multiSeekerExplorerMaxRange = MSHB_MULTI_SEEKER_EXPLORER_MAX_RANGE;
    multiSeekerSelectPosSmallDistBetwGoals = MSHB_MULTI_SEEKER_SELECT_POS_SMALL_DIST_BETW_GOALS;
    multiSeekerSelectPosBeliefDiffBetwGoals = MSHB_MULTI_SEEKER_SELECT_POS_BELIEF_DIFF_BETW_GOALS;
    multiSeekerSelectPosUseMaxBelief = true;

    multiSeekerProcessOrder = MSHB_PROC_ORDER_BELIEF;

    followPersonDist = FOLLOW_PERSON_DIST;
    minDistBetweenRobots = MIN_DIST_BETW_ROBOTS;
    //multiSeekerFollowAngle_rad = MULTI_SEEKER_FOLLOW_ANGLE_RAD;

    multiSeekerNoCommunication = false;

    pomcpDoNotLearn = false;

    //debugDontSendSecondSeeker = false;
}

#ifndef DO_NOT_USE_MOMDP
void SeekerHSParams::setSolverParams() {
    SolverParams* solverParams = &GlobalResource::getInstance()->solverParams;
    solverParams->useLookahead = useLookahead;
    solverParams->timeoutSeconds = timeoutSeconds;
    solverParams->maxTreeDepth = maxTreeDepth;
    solverParams->memoryLimit = memoryLimit;
    solverParams->showParams = showParams;
}
#endif

void SeekerHSParams::printVariables(ostream& stream, bool showExtraParams) {
    //now show params
    stream    << "Experiment name:        " << expName <<endl
              << "Game type:              ";
    switch (gameType) {
        case HSGlobalData::GAME_HIDE_AND_SEEK:
            stream << "hide-and-seek"<<endl;
            break;
        case HSGlobalData::GAME_FIND_AND_FOLLOW:
            stream << "find-and-follow"<<endl;
            break;
        case HSGlobalData::GAME_FIND_AND_FOLLOW_2ROB:
            stream << "find-and-follow (2 robots)"<<endl;
            break;
        case HSGlobalData::GAME_FIND_AND_FOLLOW_MULTI_ROB:
            stream << "find-and-follow (multi robots)"<<endl;
            break;
        default:
            stream << "unknown"<<endl;
            break;
    }

    stream    << "Stop when player wins:  " << (stopAtWin?"yes":"no, continue")<<endl
              << "Stop after # iterations:" << (stopAfterNumSteps==0?"not":"")<<stopAfterNumSteps<<endl
              << "Player type:            " << (isSeeker?"Seeker":"Hider")<<endl
              << "Solver Type:            ";

    switch(solverType) {
        case SOLVER_NOT_SET:
            stream << "[not set]"<<endl;
            break;
        case SOLVER_OFFLINE:
            stream << "Offline"<<endl;
            break;
        case SOLVER_LAYERED:
            stream <<"Layered"<<endl;
            break;
        case SOLVER_LAYERED_COMPARE:
            stream <<"Layered (compare to offline)"<<endl;
            break;
        case SOLVER_MCVI_OFFLINE:
            stream <<"MCVI (offline)"<<endl;
            break;
        case SOLVER_POMCP:
            stream <<"POMCP (online)"<<endl;
            break;
            /*case SOLVER_POMCP_CONT:
                stream <<"POMCP Continuous states (online)"<<endl;
                break;*/
        case SOLVER_SMART_SEEKER:
            stream << "SmartSeeker (online)" << endl;
            break;
        case SOLVER_FOLLOWER:
            stream << "Follower" << endl;
            break;
        case SOLVER_FOLLOWER_LAST_POS:
            stream << "Follower (to last pos)" << endl;
            break;
        case SOLVER_FOLLOWER_SEES_ALL:
            stream << "Follower (sees all)" << endl;
            break;
        case SOLVER_COMBI_POMCP_FOLLOWER:
            stream << "Combined: POMCP & Follower" << endl;
            break;
        case SOLVER_FOLLOWER_LAST_POS_EXACT:
            stream << "Follower (to last pos, exact)"<<endl;
            break;
        case SOLVER_FOLLOWER_HIGHEST_BELIEF:
            stream << "Highest Belief Follower (POMCP)"<<endl;
            break;
        case SOLVER_COMBI_FOLLOWER_HIGHEST_BELIEF:
            stream << "Combined: Highest Bel. Foll. (POMCP) & Follower"<<endl;
            break;
        case SOLVER_TWO_HB_EXPL:
            stream << "Two Seekers Highest Belief Explorer"<<endl;
            break;
        case SOLVER_MULTI_HB_EXPL:
            stream << "Multi Seeker Highest Belief Explorer"<<endl;
            break;
        case SOLVER_FILTER_KALMAN:
            stream << "Kalman Filter"<<endl;
            break;
        case SOLVER_FILTER_KALMAN_REQ_VISIB:
            stream << "Kalman Filter (req. visibility)"<<endl;
            break;
        default:
            stream <<"Unknown"<<endl;
    }
    if (solverType==SOLVER_POMCP || solverType==SOLVER_FOLLOWER_HIGHEST_BELIEF || solverType==SOLVER_COMBI_FOLLOWER_HIGHEST_BELIEF) {
        stream    << "   POMCP simulator:     ";
        switch(pomcpSimType) {
           case POMCP_SIM_CONT:
               stream<<"continuous with noise"<<endl;
               break;
           case POMCP_SIM_DISCR:
               stream<<"discrete"<<endl;
               break;
            case POMCP_SIM_PRED:
                stream<<"using person path prediction"<<endl;
                break;
           default:
               stream<<"unknown"<<endl;
               break;
        }
    }


    stream    << "Num. players this cl.:  "<<numPlayersInClient<<endl
              << "Number of req. players: "<<numPlayersReq<<endl
              << "Continuous/discrete:    "<<(useContinuousPos?"Continuous":"Discrete")<<endl
              << "Path planner:           ";
    switch(pathPlannerType) {
        case PATHPLANNER_PROPDIST:
            stream << "propogation"<<endl;
            break;
        case PATHPLANNER_ASTAR:
            stream << "A*"<<endl;
            break;
        default:
            stream<<"unknown"<<endl;
            break;
    }

    stream    << "Discount:               "<<discount<<endl
              << "Reward type:            ";
    switch(rewardType) {
        case REWARD_FINAL:
            stream <<"Final (1 if win [same state], -1 if lose)"<<endl;
            break;
        case REWARD_FINAL_CROSS:
            stream <<"Final cross (1 if win [same state + cross], -1 if lose)"<<endl;
            break;
        case REWARD_TRIANGLE:
            stream <<"Triangle"<<endl;
            break;
        case REWARD_FIND_SIMPLE:
            stream << "Find&Follow Simple"<<endl;
            break;
        case REWARD_FIND_REV_DIST:
            stream << "Find&Follow Rev. Dist."<<endl;
            break;
        default:
            stream << "Unknown"<<endl;
            break;
    }

    stream    << "Max game time (->tie):  " << maxGameTime << (maxGameTime<=0?" (inifinte)":"") <<endl
              << "Max num actions (->tie):" << maxNumActions << (maxNumActions<=0?" (inifinte)":"") <<endl
              << "Win distance:           " << winDist << endl
              << "Seeker step dist(cells):" << seekerStepDistance <<endl
              << "Hider step dist.(cells):" << hiderStepDistance <<endl
              << "Max number of players:  " << maxNumberOfPlayers()<<endl
              << "Follow distance (cells):" << followPersonDist<<endl;
    if (seekerSendsMultiplePoses()) {
        stream<< "Min dist betw. robots:  " << minDistBetweenRobots<<endl
              << "Use communication:      " << (multiSeekerNoCommunication?"NO":"yes")<<endl;
    }

#ifndef DO_NOT_USE_MOMDP
    if (solverType==SOLVER_LAYERED || solverType==SOLVER_LAYERED_COMPARE || solverType==SOLVER_OFFLINE) {
        stream << "Do pruning:             " << (doPruning?"yes":"no") << endl;
    }
    if (solverType==SOLVER_LAYERED || solverType==SOLVER_LAYERED_COMPARE) {
        stream    << "Set top final states:   " << setFinalStateOnTop <<endl
                  << "Set top rewards:        " << setFinalTopRewards <<endl
                  << "Best action lookahead:  " << (useLookahead?"true":"false") << endl
                  << "Segmenter Type:         ";

        switch(segmenterType) {
            case SEGMENTER_NOT_SET:
                stream <<"[not set]";
                break;
            case SEGMENTER_BASIC:
                stream <<"Basic";
                break;
            case SEGMENTER_KMEANS:
                stream <<"K-means";
                break;
            case SEGMENTER_TEST:
                stream << "Test";
                break;
            case SEGMENTER_ROBOT_CENTERED:
                stream << "Robot Centred";
                break;
            case SEGMENTER_CENTERED_COMBINED:
                stream << "Combined Centred";
                break;
            default:
                stream <<"Unknown";
        }

        stream << " (k="<<k<<";sdist="<<rcSegmDist<<",angle="<<rcAngDist<<",base_rad="<<rcBaseRad <<",high res rad="<<rcHighResR<<")" << endl
               << "Segmenter Type X:       ";

        switch(segmenterTypeX) {
            case SEGMENTER_NOT_SET:
                stream <<"[not set]";
                break;
            case SEGMENTER_ROBOT_CENTERED:
                stream << "Robot Centred";
                break;
            case SEGMENTER_CENTERED_COMBINED:
                stream << "Combined Centred";
                break;
            default:
                stream <<"Unknown / not supported";
        }

        stream << " (k="<<k<<";sdist="<<rcXSegmDist<<",angle="<<rcXAngDist<<",base_rad="<<rcXBaseRad <<",high res rad="<<rcXHighResR<<")" << endl
               << "Segment value type:     ";

        switch(segmentValueType) {
            case LayeredHSMOMDP::SEGMENT_VALUE_BELIEF_ONLY:
                stream << "Belief only"<<endl;
                break;
            case LayeredHSMOMDP::SEGMENT_VALUE_REWARD_ONLY:
                stream << "Reward only"<<endl;
                break;
            case LayeredHSMOMDP::SEGMENT_VALUE_BELIEF_REWARD:
                stream << "Belief x reward"<<endl;
                break;
            default:
                stream << "Unknown"<<endl;
        }

        stream    << "Upper bound init:       ";

        switch(ubInitType) {
            case UPPERBOUND_INIT_FIB:
                stream << "FIB (Fast Informed Bound)"<<endl;
                break;
            case UPPERBOUND_INIT_MDP:
                stream << "(Q)MDP"<<endl;
                break;
            default:
                stream << "Unknown"<<endl;
        }

        stream    << "Target precision:       " << targetPrecision << endl
                  << "Init targt precis fact: " << targetInitPrecFact << endl
                  << "Top reward aggreg.:     ";

        switch(topRewAggr) {
            case LayeredHSMOMDP::TOP_REWARD_SUM:
                stream <<"Sum"<<endl;
                break;
            case LayeredHSMOMDP::TOP_REWARD_AVG:
                stream << "Average" <<endl;
                break;
            case LayeredHSMOMDP::TOP_REWARD_MIN:
                stream << "Minimum" <<endl;
                break;
            case LayeredHSMOMDP::TOP_REWARD_MAX:
                stream << "Maximum" <<endl;
                break;
            default:
                stream << "Unknown"<<endl;
                break;
        }
    }
#endif

    if (solverType==SOLVER_SMART_SEEKER) {
        stream    << "Smart S. hidden sc. typ:";
        switch(smartSeekerScoreHiddenType) {
            case SmartSeeker::SCORE_AVG:
                stream <<"Average"<<endl;
                break;
            case SmartSeeker::SCORE_MAX:
                stream <<"Max"<<endl;
                break;
            case SmartSeeker::SCORE_MIN:
                stream <<"Min"<<endl;
                break;
            default:
                stream << "Unknown"<<endl;
                break;
        }
        stream    << "Smart S. act. sc. type: ";
        switch(smartSeekerScoreActionType) {
            case SmartSeeker::SCORE_AVG:
                stream <<"Average"<<endl;
                break;
            case SmartSeeker::SCORE_MAX:
                stream <<"Max"<<endl;
                break;
            case SmartSeeker::SCORE_MIN:
                stream <<"Min"<<endl;
                break;
            default:
                stream << "Unknown"<<endl;
                break;
        }

        stream    << "Smart S. max calc cells:" << (smartSeekerMaxCalcsWhenHidden==0?"[no max] ":"") << smartSeekerMaxCalcsWhenHidden <<endl;
    }

    stream  << "MOMDP file:             " << (pomdpFile.length()==0?"[not set]":pomdpFile) <<endl
            << "Policy file:            " << (policyFile.length()==0?"[not set]":policyFile) <<endl
            << "Map file:               " << (mapFile.length()==0?"[not set]":mapFile)<<endl
            << "Map dist. matrix file:  " << (mapDistanceMatrixFile.length()==0?"[not set]":mapDistanceMatrixFile)<<endl;

    if (showExtraParams) {
        stream  << "Map ID:                 " << (mapID==-1 ? "[not set] ": "")<<mapID<<endl
                << "Server IP:              " << (serverIP.length()==0?"[not set]":serverIP)<<endl
                << "       port:            " << (serverPort==-1?"[not set] ":"")<<serverPort<<endl
                << "User name:              " << userName <<endl
                << "Opponent type:          ";

        switch(opponentType) {
            case HSGlobalData::OPPONENT_TYPE_HIDER_RANDOM:
                stream << "Random Hider"<<endl;
                break;
            case HSGlobalData::OPPONENT_TYPE_HIDER_RANDOM_WALKER:
                stream << "Random Walker"<<endl;
                break;
            case HSGlobalData::OPPONENT_TYPE_HIDER_SMART:
                stream << "Smart Hider (noise std.="<<oppHiderNoiseStd<<")" <<endl;
                break;
            case HSGlobalData::OPPONENT_TYPE_HIDER_ACTION_LIST:
                stream << "Action List Hider (" << oppActionFile << ")"<< endl;
                break;
            case HSGlobalData::OPPONENT_TYPE_HIDER_ALLKNOWING:
                stream << "All Knowing Smart Hider (noise std.="<<oppHiderNoiseStd<<")" <<endl;
                break;
            case HSGlobalData::OPPONENT_TYPE_HIDER_VERYSMART:
                stream << "Very Smart Hider (noise std.="<<oppHiderNoiseStd<<")" <<endl;
                break;
            case HSGlobalData::OPPONENT_TYPE_HIDER_VALLKNOWING:
                stream << "All Knowing Very Smart Hider (noise std.="<<oppHiderNoiseStd<<")" <<endl;
                break;
            case HSGlobalData::OPPONENT_TYPE_HIDER_VERYSMART2:
                stream << "Very Smart Hider 2 (noise std.="<<oppHiderNoiseStd<<")" <<endl;
                break;
            case HSGlobalData::OPPONENT_TYPE_HIDER_V2ALLKNOWING:
                stream << "All Knowing Very Smart Hider 2 (noise std.="<<oppHiderNoiseStd<<")" <<endl;
                break;
            case HSGlobalData::OPPONENT_TYPE_HIDER_FILE:
                stream << "Hider File ("<<oppActionFile<<")"<<endl;
                break;
            case HSGlobalData::OPPONENT_TYPE_HUMAN:
                stream << "Human"<<endl;
                break;
            case HSGlobalData::OPPONENT_TYPE_HIDER_FIXED_RAND_POS:
                stream << "Fixed Random Pos"<<endl;
                break;
            default:
                stream << "Unknown"<<endl;
        }

        stream << "Auto walker type:       ";

        switch(autoWalkerType) {
            case HSGlobalData::AUTOWALKER_RANDOM:
                stream << "Random";
                break;
            case HSGlobalData::AUTOWALKER_NONE:
                stream << "None";
                break;
            case HSGlobalData::AUTOWALKER_SFM:
                stream << "Social Force Model";
                break;
            case HSGlobalData::AUTOWALKER_RANDOM_GOAL:
                stream << "Random goal";
                break;
            case HSGlobalData::AUTOWALKER_FIXED_POS:
                stream << "Fixed pos";
                break;
            case HSGlobalData::AUTOWALKER_FILE:
                stream << "File: "<<autoWalkerPosFile;
                break;
            default:
                stream << "Unknown";
        }
        stream << " (#" << autoWalkerN<<")"<<endl;

    } //showExtraParams

    if (solverType==SOLVER_POMCP || solverType==SOLVER_COMBI_POMCP_FOLLOWER || solverType==SOLVER_COMBI_FOLLOWER_HIGHEST_BELIEF
            || solverType==SOLVER_FOLLOWER_HIGHEST_BELIEF || solverType==SOLVER_TWO_HB_EXPL || solverType==SOLVER_MULTI_HB_EXPL) {

        stream << "POMCP learning:         " << (pomcpDoNotLearn?"DISABLED - only belief update":"enabled")<<endl
               << "Numb. belief states:    " << (numBeliefStates<=0?"[not set] ":"")<<numBeliefStates<<endl
               << "Number of simulations:  " << (numSim<=0?"[not set] ":"")<<numSim<<endl
               << "Exploration constant:   " << explorationConst <<endl
               << "Expand count:           " << expandCount<<endl
               << "Simulated hider type:   ";

        switch (simHiderType) {
            case SIM_HIDER_TYPE_RANDOM:
                stream << "Random"<<endl;
                break;
            case SIM_HIDER_TYPE_TOBASE:
                stream << "Directly to Base"<<endl;
                break;
            case SIM_HIDER_TYPE_SMART:
                stream << "Smart"<<endl;
                break;
            /*case SIM_HIDER_TYPE_PREDICT:
                stream << "Uses predicted person's path"<<endl;
                break;*/
            default:
                stream << "Unknown"<<endl;
                break;
        }

        stream << "Sim. hider random prob.:" << simHiderRandomActProb<<endl
               << "Set init node value:    " << (setInitNodeValue?"yes":"no")<<endl
               << "Expected reward calc.:  ";

        switch(expectedRewardCalc) {
            case EXPECTED_REWARD_CALC_SUM:
                stream << "Sum"<<endl;
                break;
            case EXPECTED_REWARD_CALC_NORM:
                stream << "Normalized by (1+discount)"<<endl;
                break;
            default:
                stream << "Unknown"<<endl;
                break;
        }
        stream << "Use weighted beliefs:   " << (useWeightedBeliefs?"yes":"no")<<endl;
        stream << "Rollout policy type:    ";
        switch (rolloutPolicyType) {
            case ROLL_OUT_POLICY_TYPE_RANDOM:
                stream << "Random"<<endl;
                break;
            case ROLL_OUT_POLICY_TYPE_SMART:
                stream << "SmartSeeker"<<endl;
                break;
            default:
                stream << "Unknown"<<endl;
                break;
        }

        if (pomcpSimType==POMCP_SIM_CONT || pomcpSimType==POMCP_SIM_PRED /*solverType==SOLVER_POMCP_CONT*/) {
            stream    << "Next seeker state stdev:" << contNextSeekerStateStdDev <<endl
                      << "Next hider state stddev:" << contNextHiderStateStdDev <<endl
                      << "Obs seeker std.dev.:    " << contSeekerObsStdDev << endl
                      << "Obs hider std.dev.:     " << contHiderObsStdDev << endl
                      << "False positive prob.:   " << contFalsePosProb << endl
                      << "False negative prob.:   " << contFalseNegProb << endl
                      << "Cont incor. pos. prob.: " << contIncorPosProb << endl
                      << "Cont Upd incons. acc. p:" << contUpdInconsistAcceptProb <<endl
                      << "Consist. check hider d: " <<contConsistCheckSeekerDist<<endl
                      << "Consist. check seeker d:" <<contConsistCheckHiderDist<<endl;
        }

        stream << "POMCP filter belief upd:" << (pomcpFilterBeliefAtUpdate?"yes":"no") <<endl;

        if (pomcpSimType==POMCP_SIM_PRED) {
            stream    << "Next hider halt prob.:  " << contNextHiderHaltProb <<endl
                      << "Use hider pred. prob.:  " << contUseHiderPredStepProb <<endl
                      << "People pred. dest. file:" << ppDestinationFile<<endl
                      << "People pred. fut. horiz:" << ppHorizonSteps<<endl;
        }

        stream        << "Obs. equals thresh dist:" << obsEqualsThreshDist << endl;
        if (seekerSendsMultiplePoses()) {
            stream        << "Multi seeker own obs pr:" <<multiSeekerOwnObsChooseProb << endl
                          << "Multi s. vis. obs. pr.: " <<multiSeekerVisObsChooseProbIfMaybeDynObst << endl
                          << "Multi s. expl. util. w.:" <<multiSeekerExplorerUtilWeight<<endl
                          << "Multi s. expl. dist. w.:" << multiSeekerExplorerDistWeight << endl
                          << "Multi s. expl. bel. w.: " << multiSeekerExplorerBeliefWeight << endl
                          << "Multi s. expl. n bel. p:" << multiSeekerExplorerCheckNPoints<<endl
                          << "Multi s. expl. max rang:" << multiSeekerExplorerMaxRange<<endl
                          << "Multi s. select pos dst:" << multiSeekerSelectPosSmallDistBetwGoals<<endl
                          << "Multi s. sel pos bel.df:" << multiSeekerSelectPosBeliefDiffBetwGoals<<endl
                          << "Multi s. sel pos w.bel: " << (multiSeekerSelectPosUseMaxBelief?"max":"sum")<<endl;
            stream        << "Multi s. process order: ";
            switch (multiSeekerProcessOrder) {
                case MSHB_PROC_ORDER_ID:
                    stream << "player ID"<<endl;
                    break;
                case MSHB_PROC_ORDER_BELIEF:
                    stream << "sum of belief"<<endl;
                    break;
                default:
                    stream << "Unknown"<<endl;
                    break;
            }
        }

    }

    if (solverType==SOLVER_COMBI_FOLLOWER_HIGHEST_BELIEF || solverType==SOLVER_FOLLOWER_HIGHEST_BELIEF || solverType==SOLVER_TWO_HB_EXPL) {
        stream    << "High.bel.fol.upd.time:  " << highBeliefFollowerUpdateGoalTime_ms <<" ms"<<endl
                  << "High.bel.fol.upd.steps: " <<highBeliefFollowerUpdateGoalNumSteps<<endl
                  << "High. b.f. highest dist:" << highBeliefFollowerHighestDist <<endl;
    }
    stream    << "Next goal dist. calc:   " << (searchGoalByShortestPathDist?"shortest path":"eucledian")<<endl;


    stream  << "Init position type:     ";
    switch (initPosType) {
        case HSGlobalData::INIT_POS_TYPE_RANDOM:
            stream << "Random"<<endl;
            break;
        case HSGlobalData::INIT_POS_TYPE_BASE:
            stream << "Base (seeker, hider random)"<<endl;
            break;
        case HSGlobalData::INIT_POS_TYPE_FIXED:
            stream << "Fixed: ";
            if (initPosFixed==NULL)
                stream << "not set"<<endl;
            else
                stream << initPosFixed->toString()<<endl;
            break;
        case HSGlobalData::INIT_POS_TYPE_DIST:
            stream << "Distance from base ("<<randomPosDistToBase<<")"<<endl;
            break;
    }
    stream << "Init pos type hider:    ";
    if (randomPosDistToBaseHider<=0) {
        stream << "Random (hidden from base)"<<endl;
    } else {
        stream << "Distance from base ("<<randomPosDistToBaseHider<<")"<<endl;
    }

    //SeekerHS
    stream   <<"When using SeekerHS:"<<endl
             << "  Filter hid. base sc.: " << filterHiddenBaseScore<<endl
             << "  Use next n positions: " << useNextNPos<<endl
             << "  Simulate not visible: " << (simulateNotVisible?"yes":"no")<<endl
             << "  Vis. check bef. move: " << (doVisibCheckBeforeMove?"yes":"no")<<endl
             << "  Min.time betw.it.(ms):" << minTimeBetweenIterations_ms <<endl
             << "Belief zoom factor:     " << beliefMapZoomFactor<<endl
             << "Sim. receive all tracks:";

    if (simulateReceivingAllTracksAsPersons) {
        stream << "yes"<<endl
               << "Filter score type:      ";
        switch(filterScoreType) {
            case HSGlobalData::FILTER_SCORE_NEW2_WEIGHTED:
                stream<<"weighted"<<endl;
                break;
            case HSGlobalData::FILTER_SCORE_USE_TAG_ONLY:
                stream<<"tag only"<<endl;
                break;
            default:
                stream<<"unknown/not implemented"<<endl;
                break;
        }
        stream << "Filter can stop #iterat:"<<filterCanStopNumberOfIterations<<endl;
    } else {
        stream << "no"<<endl;
    }

    stream  << "Only send sim. steps:   " << (onlySendStepGoals?"yes":"no")<<endl
            << "Sim. obs. noise std.dev:" << (simObsNoiseStd)<<endl
            << "Sim.obs. false neg.prob:" << simObsFalseNegProb<<endl
            << "Sim.obs. false pos.prob:" << simObsFalsePosProb<<endl
            << "Solver time out(s):     " << (timeoutSeconds<=0?"no time-out ":"")<<timeoutSeconds <<endl
            << "Dyn.obst.used for learn:" <<(takeDynObstOcclusionIntoAccountWhenLearning?"yes":"no")<<endl
            << "Max belief tree depth:  " << (maxTreeDepth<=0?"[not set] ":"")<<maxTreeDepth <<endl
            //<< "Log prefix:           " << (logFilePrefix==NULL?"[not set]":logFilePrefix)<<endl
            << "Allow inconsistencies:  " << (allowInconsistObs?"yes":"no") <<endl
            << "Use deduced action:     " << (useDeducedAction?"yes":"no")<<endl
            << "Min cell dist. to obst: " << minDistToObstacle<<endl
            << "Min c.dist. to dyn.obst:" << minDistToDynObstacle<<endl
#ifdef DYN_OBST_COLL
            << "Check dyn.obst. collis.:" << (checkDynamicObstacleCollisions?"yes":"no")<<endl
#else
            << "Check dyn.obst. collis.:NOT implemented/not activated"<<endl
#endif
            << "Log:                    " << logFile<<endl
            << "Time log:               " << timeLogFile<<endl
            << "Game log:               " << gameLogFile<<endl
            << "Belief image:           " << beliefImageFile <<endl
            << "Memory limit:           " << (memoryLimit/(1024*1024)) << " " << (memoryLimit==0?"none":"MB") << endl
            << "Time:                   " << currentTimeStamp() << endl
            << endl<<endl;
}


/*bool SeekerHSParams::gameHas2Seekers() {
    return gameType==HSGlobalData::GAME_FIND_AND_FOLLOW_2ROB; //TODO: could be more types
}*/

bool SeekerHSParams::seekerSendsMultiplePoses() {
    return gameType==HSGlobalData::GAME_FIND_AND_FOLLOW_2ROB || gameType==HSGlobalData::GAME_FIND_AND_FOLLOW_MULTI_ROB; //TODO: could be more types
}



unsigned int SeekerHSParams::maxNumberOfPlayers() {
    unsigned int maxp = 0;
    switch (gameType) {
        case HSGlobalData::GAME_FIND_AND_FOLLOW_2ROB:
            maxp = 3;
            break;
        case HSGlobalData::GAME_FIND_AND_FOLLOW_MULTI_ROB:
            maxp = HSGlobalData::MAX_NUM_PLAYERS;
            break;
        default:
            maxp = 2;
            break;
    }
    return maxp;
}



#ifdef USE_QT
/*
void SeekerHSParams::readFromStream(QDataStream& in) {

}

void SeekerHSParams::writeToStream(QDataStream& out) {

}    */
#endif

