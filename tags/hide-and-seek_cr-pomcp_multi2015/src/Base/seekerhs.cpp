#include "Base/seekerhs.h"

#include <iostream>
#include <fstream>
#include <assert.h>
#include <string>
#include <sstream>
#include <cmath>
#include <sys/time.h>
#include <time.h>

//APPL
#ifndef DO_NOT_USE_MOMDP
#include "solverUtils.h"

//H&S classes
#include "Solver/hsmomdp_runpolicy.h"
#include "Solver/hsmomdp_layered.h"
#endif

//#include "mcvi/hideseekDiscrete/hideseekmcvi.h"

#include "Smart/smartseeker.h"
#include "Smart/follower.h"
#include "Smart/combinedseeker.h"
#include "Smart/highestbelieffollower.h"
#include "Smart/twoseekerhbexplorer.h"
#include "Smart/multiseekerhbexplorer.h"

#include "AutoHider/smarthider.h"
#include "Filter/kalmanfilterplayer.h"

#ifndef DO_NOT_USE_MOMDP
#include "Segment/segment.h"
#include "Segment/basesegmenter.h"
#include "Segment/kmeanssegmenter.h"
#include "Segment/robotcenteredsegmentation.h"
#include "Segment/combinecenteredsegmentation.h"
#include "Segment/testsegmenter.h"
#endif

#include "POMCP/hspomcp.h"
#include "POMCP/hssimulatorcont.h"

//#include "Utils/hslog.h"
#include "Utils/generic.h"

#include "Base/hsconfig.h"

//iriutils
#include "exceptions.h"

using namespace std;


SeekerHS::SeekerHS(string mapFile, string expName, int solverType, string pomdpFile, string policyFile,
                   string logFile, string timeLogFile, string gamelogFile) : _gameLog(this) {
    SeekerHSParams* params = new SeekerHSParams();

    params->mapFile = mapFile;
    params->expName = expName;
    params->solverType = solverType;
    params->pomdpFile = pomdpFile;
    params->policyFile = policyFile;
    params->logFile = logFile;
    params->timeLogFile = timeLogFile;
    params->gameLogFile = gamelogFile;

    initClass(params,true);
}

SeekerHS::SeekerHS(SeekerHSParams *params, bool genMap) : _gameLog(this) {
    initClass(params, genMap);
}


SeekerHS::~SeekerHS() {
    #ifndef DO_NOT_USE_MOMDP
    DEBUG_DELETE(cout<<"SeekerHS destructor:"<<endl;);
    if (_segmenter!=NULL) {
        DEBUG_DELETE(cout<<"delete segmenter: "<<flush;);
        delete _segmenter;
        DEBUG_DELETE(cout<<"ok"<<endl;);
    }
    if (_segmenterX!=NULL) {
        DEBUG_DELETE(cout<<"delete segmenterX: "<<flush;);
        delete _segmenterX;
        DEBUG_DELETE(cout<<"ok"<<endl;);
    }
    #endif
    DEBUG_DELETE(cout<<"delete auto player: "<<flush;);
    delete _autoPlayer;
    DEBUG_DELETE(cout<<"ok"<<endl;);
    DEBUG_DELETE(cout<<"delete map: "<<flush;);
    delete _map;
    DEBUG_DELETE(cout<<"ok"<<endl;);
    DEBUG_DELETE(cout<<"close game log: "<<flush;);
    _gameLog.close();
    DEBUG_DELETE(cout<<"ok"<<endl;);

#ifndef DO_NOT_WRITE_BELIEF_IMG
    DEBUG_DELETE(cout<<"delete next seeker pos: "<<flush;);
    if (_beliefImg!=NULL) delete _beliefImg;
    DEBUG_DELETE(cout<<"ok"<<endl;);
#endif
}

void SeekerHS::initClass(SeekerHSParams *params, bool genMap) {
    //init randomizer
    initRandomizer();

#ifndef DO_NOT_WRITE_BELIEF_IMG
    _beliefImg = NULL;
#endif

    _params = params;
    _autoPlayer = NULL;
    _algoTimerID = -1;

    if (genMap && params->mapFile.length()==0)
        throw CException(_HERE_,"a map file is required");
    if (params->solverType == SeekerHSParams::SOLVER_NOT_SET)
        throw CException(_HERE_,"solver type has to be set");

    if (params->logFile.length()==0)
        params->logFile = "seekerhs_log.txt";
    if (params->timeLogFile.length()==0)
        params->timeLogFile = "seekerhs_time_log.txt";
    if (params->gameLogFile.length()==0)
        params->gameLogFile = "seekerhs_game_log.txt";

    #ifndef DO_NOT_USE_MOMDP
    //set solver params
    params->setSolverParams();
    #endif

    //AG150618: depricated by PlayerInfo
    //next pos
    //_nextSeekerPos = new Pos();

    //max num actions are set in init
    //_numActions = 0; //AG150623: now stored in _thisPlayerInfo.numberActions

    if (genMap) {
        //load gmap
        DEBUG_SHS(cout<<"Loading map:"<<flush;);
        _map = new GMap(params->mapFile.c_str(), params);
        DEBUG_SHS(cout<<"ok"<<endl;);
        if (params->mapDistanceMatrixFile.length()>0) {
            DEBUG_SHS(cout<<"Loading distance matrix: "<<flush;);
            _map->readDistanceMatrixToFile(params->mapDistanceMatrixFile.c_str());
            DEBUG_SHS(cout<<"ok"<<endl;);
        }
        //setMap(_map);
    } else {
        _map = NULL;
    }

    //name exp
    //_expName = params->expName;

    #ifndef DO_NOT_USE_MOMDP
    _segmenter = _segmenterX = NULL;

    if (params->solverType==SeekerHSParams::SOLVER_LAYERED_COMPARE || params->solverType==SeekerHSParams::SOLVER_LAYERED) {
        //set the Y-space segmenter
        switch (params->segmenterType) {
            case SeekerHSParams::SEGMENTER_BASIC: {
                _segmenter = new BaseSegmenter(8);
                break;
            }
            case SeekerHSParams::SEGMENTER_KMEANS: {
                KMeansSegmenter* kmeansSegmenter = new KMeansSegmenter(8);
                if (params->k>0) kmeansSegmenter->setK(params->k);
                _segmenter = kmeansSegmenter;
                break;
            }
            case SeekerHSParams::SEGMENTER_ROBOT_CENTERED: {
                RobotCenteredSegmentation* rcSegmenter = new RobotCenteredSegmentation(8,params->rcSegmDist,params->rcAngDist,
                        RobotCenteredSegmentation::DIST_CHESS,params->rcBaseRad,params->rcHighResR);
                _segmenter = rcSegmenter;
                break;
            }
            case SeekerHSParams::SEGMENTER_CENTERED_COMBINED: {
                CombineCenteredSegmentation* ccSegmenter = new CombineCenteredSegmentation(8,params->rcSegmDist,params->rcAngDist,
                        RobotCenteredSegmentation::DIST_CHESS,params->rcBaseRad,params->rcHighResR);
                _segmenter = ccSegmenter;
                break;
            }
            case SeekerHSParams::SEGMENTER_TEST: {
                TestSegmenter* tSegmenter = new TestSegmenter(8);
                _segmenter = tSegmenter;
                break;
            }
            case SeekerHSParams::SEGMENTER_NOT_SET: {
                throw CException(_HERE_, "The layered solver requires a segmenter");
                break;
            }
            default:
                throw CException(_HERE_, "Unknown segmenter type");
        }

        //set X segmenter
        switch (params->segmenterTypeX) {
            case SeekerHSParams::SEGMENTER_NOT_SET:
                _segmenterX  = NULL;
                break; //i.e. X states not segmented
            case SeekerHSParams::SEGMENTER_ROBOT_CENTERED: {
                RobotCenteredSegmentation* rcxSegmenter = new RobotCenteredSegmentation(8,params->rcXSegmDist,params->rcXAngDist,
                        RobotCenteredSegmentation::DIST_CHESS,params->rcXBaseRad, params->rcXHighResR);
                _segmenterX = rcxSegmenter;
                break;
            }
            case SeekerHSParams::SEGMENTER_CENTERED_COMBINED: {
                CombineCenteredSegmentation* ccxSegmenter = new CombineCenteredSegmentation(8,params->rcXSegmDist,params->rcXAngDist,
                        RobotCenteredSegmentation::DIST_CHESS,params->rcXBaseRad, params->rcXHighResR);
                _segmenterX = ccxSegmenter;
                break;
            }
            default:
                throw CException(_HERE_, "Unknown segmenter type for X, or not supported");
        }
    }
    #endif

    // generate solver / auto player type
    _autoPlayer = NULL;

    //AG130506: check files
    if (params->solverType==SeekerHSParams::SOLVER_OFFLINE ||
            params->solverType==SeekerHSParams::SOLVER_LAYERED_COMPARE ||
            params->solverType==SeekerHSParams::SOLVER_LAYERED ||
            params->solverType==SeekerHSParams::SOLVER_MCVI_OFFLINE) {

        //check pomdp file
        if (params->pomdpFile.empty()) {
            throw CException(_HERE_,"A POMDP file is required");
        } else if (!fileExists(params->pomdpFile.c_str())) {
            throw CException(_HERE_,"could not find the POMDP file");
        }
    }
    if (params->solverType==SeekerHSParams::SOLVER_OFFLINE ||
            params->solverType==SeekerHSParams::SOLVER_LAYERED_COMPARE) {

        //check policy file
        if (params->policyFile.empty()) {
            throw CException(_HERE_,"A policy file is required");
        } else if (!fileExists(params->policyFile.c_str())) {
            throw CException(_HERE_,"could not find the policy file");
        }
    }

    //create the instance of the solver, depending of the selected type
    switch (params->solverType) {
        case SeekerHSParams::SOLVER_OFFLINE: {
            #ifdef DO_NOT_USE_MOMDP
            throw CException(_HERE_,"The solver has been compiled without MOMDP (DO_NOT_USE_MOMDP).");
            #else
            if (params->policyFile.length()==0) throw CException(_HERE_,"a policy file is required");

            //generate the solver
            RunPolicyHSMOMDP* runPolHSMOMDP = new RunPolicyHSMOMDP(params);
            _autoPlayer = runPolHSMOMDP;
            //cout << "initializing ..." <<endl;
            bool ok = runPolHSMOMDP->init(params->pomdpFile.c_str(), params->logFile.c_str(), params->policyFile.c_str(), params->timeLogFile.c_str());

            if (!ok) {
                throw CException(_HERE_,"Failed while initializing the offline H&S solver.");
            }
            #endif
            break;
        }
        case SeekerHSParams::SOLVER_LAYERED_COMPARE:
            if (params->policyFile.length()==0) throw CException(_HERE_,"a policy file is required");
            //no break, since same als solver_layered, but with (offline) policy file
        case SeekerHSParams::SOLVER_LAYERED: {
            #ifdef DO_NOT_USE_MOMDP
            throw CException(_HERE_,"The solver has been compiled without MOMDP (DO_NOT_USE_MOMDP).");
            #else
            //generate the solver
            LayeredHSMOMDP* layeredHSMOMDP = new LayeredHSMOMDP(params);
            _autoPlayer = layeredHSMOMDP;
            //cout << "initializing ..." <<endl;

            bool ok = false;
            bool useFIBInit = (params->ubInitType==SeekerHSParams::UPPERBOUND_INIT_FIB);

            //AG130101: set x segmenter
            bool usSegmentX = (_segmenterX!=NULL);

            if (params->solverType==SeekerHSParams::SOLVER_LAYERED_COMPARE) {
                ok = layeredHSMOMDP->init(params->pomdpFile.c_str(), params->logFile.c_str(), params->timeLogFile.c_str(), useFIBInit,
                                          params->segmentValueType, params->policyFile.c_str(), params->topRewAggr, usSegmentX, usSegmentX,
                                          params->setFinalStateOnTop,params->setFinalTopRewards);
            } else {
                ok = layeredHSMOMDP->init(params->pomdpFile.c_str(), params->logFile.c_str(), params->timeLogFile.c_str(), useFIBInit,
                                          params->segmentValueType, NULL, params->topRewAggr, usSegmentX, usSegmentX,
                                          params->setFinalStateOnTop,params->setFinalTopRewards);
            }

            if (!ok) {
                throw CException(_HERE_,"Failed while initializing the online layered H&S solver.");
            }

            //cout << "Segmenter pos: "<<_segmenter->getPosition().toString()<<endl;
            layeredHSMOMDP->setSegmenter(_segmenter);

            //AG130101: add x segmenter
            if (usSegmentX) {
                layeredHSMOMDP->setSegmenterX(_segmenterX);
            }

            //set UB initializer
            //layeredHSMOMDP->setUseFIBUBInit(ubInitType==UPPERBOUND_INIT_FIB);
            layeredHSMOMDP->setTargetPrecision(params->targetPrecision, params->targetInitPrecFact);
            #endif
            break;
        }
        case SeekerHSParams::SOLVER_MCVI_OFFLINE:
            //HideSeekMCVI* mcvi = new HideSeekMCVI(_params->policyFile);
            //_autoPlayer = mcvi;
            throw CException(_HERE_,"MCVI has not been implemented (yet)");
            break;
        case SeekerHSParams::SOLVER_SMART_SEEKER: {
            SmartSeeker* smartSeeker = new SmartSeeker(_params);
            _autoPlayer = smartSeeker;
            break;
        }
        /*case SeekerHSParams::SOLVER_POMCP_CONT: //same solver, different simulator
            if (!_params->useContinuousPos)
                throw CException(_HERE_,"continuous pos required for continuous solver");
            //else, fallthrough*/
        case SeekerHSParams::SOLVER_POMCP: {
            pomcp::HSPOMCP* hspomcp = new pomcp::HSPOMCP(_params);
            _autoPlayer = hspomcp;
            break;
        }
        case SeekerHSParams::SOLVER_FOLLOWER_LAST_POS: //the check is done in the Follower
        case SeekerHSParams::SOLVER_FOLLOWER_LAST_POS_EXACT: //idem
        case SeekerHSParams::SOLVER_FOLLOWER_SEES_ALL: //AG151114: sees all
        case SeekerHSParams::SOLVER_FOLLOWER: {
            Follower* follower = new Follower(_params);
            _autoPlayer = follower;
            break;
        }
        case SeekerHSParams::SOLVER_COMBI_POMCP_FOLLOWER: {
            Follower* follower = new Follower(_params);
            pomcp::HSPOMCP* hspomcp = new pomcp::HSPOMCP(_params);
            CombinedSeeker* combine = new CombinedSeeker(_params,follower,hspomcp);
            _autoPlayer = combine;
            break;
        }
        case SeekerHSParams::SOLVER_FOLLOWER_HIGHEST_BELIEF: {
            pomcp::HSPOMCP* hspomcp = new pomcp::HSPOMCP(_params);
            HighestBeliefFollower* hbFollower = new HighestBeliefFollower(_params, hspomcp);
            _autoPlayer = hbFollower;
            break;
        }
        case SeekerHSParams::SOLVER_COMBI_FOLLOWER_HIGHEST_BELIEF: {
            Follower* follower = new Follower(_params);
            pomcp::HSPOMCP* hspomcp = new pomcp::HSPOMCP(_params);
            HighestBeliefFollower* hbFollower = new HighestBeliefFollower(_params, hspomcp);
            CombinedSeeker* combine = new CombinedSeeker(_params,follower,hbFollower);
            _autoPlayer = combine;
            break;
        }
        case SeekerHSParams::SOLVER_TWO_HB_EXPL: {
            pomcp::HSPOMCP* hspomcp = new pomcp::HSPOMCP(_params);
            TwoSeekerHBExplorer* twoSeekerHBExplorer = new TwoSeekerHBExplorer(_params, hspomcp);
            _autoPlayer = twoSeekerHBExplorer;
            break;
        }
        case SeekerHSParams::SOLVER_MULTI_HB_EXPL: {
            pomcp::HSPOMCP* hspomcp = new pomcp::HSPOMCP(_params);
            MultiSeekerHBExplorer* multiSeekerHBExplorer = new MultiSeekerHBExplorer(_params, hspomcp);
            _autoPlayer = multiSeekerHBExplorer;
            break;
        }
        case SeekerHSParams::SOLVER_FILTER_KALMAN_REQ_VISIB:
        case SeekerHSParams::SOLVER_FILTER_KALMAN: {
            KalmanFilterPlayer* kfPlayer = new KalmanFilterPlayer(_params);
            _autoPlayer = kfPlayer;
            break;
        }
        default: //also SOLVER_NOT_SET
            throw CException(_HERE_,"Unknown solver type");
            break;
    }

    assert(_autoPlayer!=NULL);

    if(genMap)
        setMap(_map);

    //AG150622: init player info
    initPlayerInfo();

    cout << "AutoPlayer: "<<_autoPlayer->getName()<<endl;
}


void SeekerHS::initPlayerInfo() {
    //AG150618: set the playerInfo vars to keep track of the positions and their observations
    _thisPlayerInfo = &_autoPlayer->playerInfo;
    //now prepare the player info structs for the other seekers
    assert(_params->numPlayersReq>1); //should be at least 2 (seeker+hider)
    //set the vector size to be
    _otherPlayerInfoVec.resize(_params->numPlayersReq-2);

    //set hider type
    _hiderPlayerInfo.playerType = HSGlobalData::P_Hider;
    //AG150706: set if is seeker
    _params->isSeeker = _autoPlayer->isSeeker();
    _thisPlayerInfo->playerType = (_params->isSeeker ? HSGlobalData::P_Seeker :HSGlobalData::P_Hider);

    //own type should be seeker
    assert(_thisPlayerInfo->isSeeker());
    //set seeker type of other
    for(PlayerInfo& playerInfo : _otherPlayerInfoVec) {
        playerInfo.playerType = HSGlobalData::P_Seeker;
    }
}


AutoPlayer* SeekerHS::getAutoPlayer() {
    return _autoPlayer;
}

#ifndef DO_NOT_USE_MOMDP
Segmenter* SeekerHS::getSegmenter() {
    return _segmenter;
}

Segmenter* SeekerHS::getSegmenterX() {
    return _segmenterX;
}
#endif

GMap* SeekerHS::getMap() {
    return _map;
}

#ifdef OLD_CODE
int SeekerHS::getWinDistToHider() {
    return _params->winDist;
}

double SeekerHS::getCellSizeM() {
    return _params->cellSizeM;
}
#endif

unsigned int SeekerHS::getActionsDone() {
    if (_thisPlayerInfo==NULL)
        return 0;
    else
        return _thisPlayerInfo->numberActions;
}

void SeekerHS::setPlayerInfoFromRobotPoses(PlayerInfo *playerInfo, const PosXY &seekerPosXY, const PosXY *hiderPosXY) {
    //set id
    playerInfo->id = seekerPosXY.id;
    //assert(seekerPosXY.id>=0);
    //set the player info current pos to seeker pos
    playerInfo->currentPos = seekerPosXY.toPos(_params);
    //if available set hider pos
    if (hiderPosXY != NULL && hiderPosXY->isSet()) {
        playerInfo->hiderObsPosWNoise = hiderPosXY->toPos(_params);
    } else {
        playerInfo->hiderObsPosWNoise.clear();
    }
    if (hiderPosXY!=NULL) {
        playerInfo->useObsProb = hiderPosXY->b;
        assert(hiderPosXY->b>=0 && hiderPosXY->b<=1.0);
    }
    //flag that we have read the new pos
    playerInfo->posRead = seekerPosXY.isSet(); // true; //AG150728: only set when the currentpos was set
    if (!playerInfo->initPosSet) playerInfo->initPosSet = playerInfo->posRead;
}

void SeekerHS::init(const PosXY &seekerPosXY, const PosXY &hiderObsPosXY) {
    DEBUG_SHS(cout << "SeekerHS::init..."<<endl;);
    assert(seekerPosXY.id>=0);
    assert(seekerPosXY.isSet());
    //set this player's data
    setPlayerInfoFromRobotPoses(_thisPlayerInfo, seekerPosXY, &hiderObsPosXY);
    setPlayerInfoFromRobotPoses(&_hiderPlayerInfo, hiderObsPosXY, NULL);

    //set hider type
    /*_hiderPlayerInfo.playerType = HSGlobalData::P_Hider;
    _hiderPlayerInfo.currentPos = _thisPlayerInfo->hiderObsPosWNoise;
    _hiderPlayerInfo.id = hiderObsPosXY.id;*/

    //now add players to map
    addPlayerInfoToHashMap(_thisPlayerInfo);
    addPlayerInfoToHashMap(&_hiderPlayerInfo);

    //start timer
    _timerID = _timer.startTimer();

    //init belief
    _autoPlayer->initBelief(_map, &_hiderPlayerInfo);

    //log
    _gameLog.open(_params->gameLogFile);
    _gameLog.init(_map, _params, _thisPlayerInfo, NULL, seekerPosXY, hiderObsPosXY, NULL, NULL);

    DEBUG_SHS(cout << "SeekerHS::init: done"<<endl;);
}

void SeekerHS::initMultiSeeker(const PosXY &seekerPosXY, const PosXY &hiderObsPosXY, const std::vector<PosXY> &seekerOtherPosXYVec,
                               const std::vector<PosXY> &hiderObsOtherPosXYVec) {
    DEBUG_SHS(cout << "SeekerHS::initMultiSeekers ..."<<endl;);
cout<<"seeker:"<<seekerPosXY.toString()<<", hiderObs:"<<hiderObsPosXY.toString()
    <<", seekerOthPosVec.size="<<seekerOtherPosXYVec.size()<<", hidervec.sz="<<hiderObsOtherPosXYVec.size()
    <<endl;
for(size_t i=0;i<seekerOtherPosXYVec.size();i++)
    cout<<" "<<i<<") "<<seekerOtherPosXYVec[i].toString()<<", h:"<<hiderObsOtherPosXYVec[i].toString()<<endl;

    assert(_autoPlayer->handles2Obs());
    assert(seekerPosXY.id>=0);

    //at init each player's pos should be set
    if (seekerOtherPosXYVec.size()!=_otherPlayerInfoVec.size()) {
        throw CException(_HERE_, "at initialization all the seekers need to have sent their position (to assign ID)");
    }
    if (seekerOtherPosXYVec.size()!=hiderObsOtherPosXYVec.size()) {
        throw CException(_HERE_, "the other seeker's observations of the hider should be all sent");
    }

    //AG150723: assume we have 2 seekers
    if (seekerOtherPosXYVec.size()>1) {
        throw CException(_HERE_, "for the real experiments there can only be 2 robots, for more some code changes are required");
    }

    //set this player's data
    setPlayerInfoFromRobotPoses(_thisPlayerInfo, seekerPosXY, &hiderObsPosXY);
cout<<"thisPI:"<<_thisPlayerInfo->toString()<<endl;
    setPlayerInfoFromRobotPoses(&_hiderPlayerInfo, hiderObsPosXY, NULL);
cout<<"hiderpi:"<<_hiderPlayerInfo.toString()<<endl;

    //now add players to map
    addPlayerInfoToHashMap(_thisPlayerInfo);
    addPlayerInfoToHashMap(&_hiderPlayerInfo);

    //set other players
    for(size_t i=0; i<seekerOtherPosXYVec.size(); i++) {
        assert(seekerOtherPosXYVec[i].id>=0);
        setPlayerInfoFromRobotPoses(&_otherPlayerInfoVec[i], seekerOtherPosXYVec[i], &hiderObsOtherPosXYVec[i]);
        addPlayerInfoToHashMap(&_otherPlayerInfoVec[i]);
cout<<" other "<<i<<": "<<_otherPlayerInfoVec[i].toString()<<endl;
    }

    /*_numActions = 0;

    DEBUG_SHS(cout << "Maximum number of actions: "<<_params->maxNumActions <<endl;);
    DEBUG_SHS(cout << "Maximum number of time: "<<_params->maxGameTime <<" s"<<endl;);*/

    //AG150622: generate vector with PlayerInfo
    vector<PlayerInfo*> playerVec(_otherPlayerInfoVec.size()+2,NULL);
    //now fill the list
    /*playerVec[0] = _thisPlayerInfo;
    playerVec[1] = &_hiderPlayerInfo;*/
    assert(_thisPlayerInfo->id>=0 && _thisPlayerInfo->id<(int)playerVec.size());
    playerVec[_thisPlayerInfo->id] = _thisPlayerInfo;
    assert(_hiderPlayerInfo.id>=0 && _hiderPlayerInfo.id<(int)playerVec.size());
    playerVec[_hiderPlayerInfo.id] = &_hiderPlayerInfo;
    //add rest
    for(size_t i = 0; i<_otherPlayerInfoVec.size(); i++) {
        //playerVec[i+2] = &_otherPlayerInfoVec[i];
    assert(_otherPlayerInfoVec[i].id>=0 && _otherPlayerInfoVec[i].id<(int)playerVec.size());
    assert(playerVec[_otherPlayerInfoVec[i].id]==NULL);
    playerVec[_otherPlayerInfoVec[i].id] = &_otherPlayerInfoVec[i];
    }

    //start timer
    _timerID = _timer.startTimer();

    //init belief
    _autoPlayer->initBeliefMulti(_map, playerVec, 0, 1);

    //log
    _gameLog.open(_params->gameLogFile);
    _gameLog.init(_map, _params, _thisPlayerInfo, &_otherPlayerInfoVec[0], seekerPosXY, hiderObsPosXY, &seekerOtherPosXYVec[0], &hiderObsOtherPosXYVec[0]);

    DEBUG_SHS(cout << "SeekerHS::initMultiSeeker: done"<<endl;);
}

void SeekerHS::filterMultipleObs(const PosXY &seekerPosXYIn, const std::vector<PosXY>& hiderPosXYVecIn, PosXY &seekerPosXYOut,
                                 PosXY &hiderPosXYOut, bool& dontExec) {
    //convert poses
    Pos seekerPosIn = seekerPosXYIn.toPos(_params);
    vector<IDPos> hiderPosVecIn(hiderPosXYVecIn.size());
    //convert hider poses
    for(size_t i = 0; i<hiderPosXYVecIn.size(); i++) {
        hiderPosVecIn[i] = hiderPosXYVecIn[i].toIDPos(_params);
    }

    //output of filter
    Pos seekerPosOut;
    IDPos hiderPosOut;

    //now apply filter
    _autoPlayer->checkAndFilterPoses(seekerPosIn, hiderPosVecIn, seekerPosOut, hiderPosOut, dontExec);

    DEBUG_SHS(
        if (dontExec)
            cout<<"SeekerHS::filterMultipleObs: WARNING gives \"don't execute\" message'"<<endl;
    );

    //now convert again
    seekerPosXYOut.fromPos(seekerPosOut, _params);
    seekerPosXYOut.id = seekerPosXYIn.id; //AG150728: restore id
    hiderPosXYOut.fromIDPos(hiderPosOut, _params);

    _chosenHiderPosXY = hiderPosXYOut;
}

bool SeekerHS::hasTimePassedForNextIteration() {
    if (_algoTimerID<0) {
        return true;
    } else {
        return _timer.getTime_ms(_algoTimerID) >= (long)_params->minTimeBetweenIterations_ms;
    }
}

void SeekerHS::prepareNextStep() {
    _thisPlayerInfo->prepareNextStep();
    for(PlayerInfo& playerInfo : _otherPlayerInfoVec) {
        playerInfo.prepareNextStep();
    }
}

bool SeekerHS::calcNextMultiRobotPoses2(const PosXY& seekerPosXY, const PosXY& hiderObsPosXY, const PosXY* seekerOtherPosXY,
                                        const PosXY* hiderObsOtherPosXY, PosXY &newSeekerPosMine, PosXY &newSeekerPosOther,
                                        int *winState, bool *dontExecuteIteration, bool resetItStartTime)
{
    DEBUG_SHS(cout << "SeekerHS::calcNextMultiRobotPoses2 ..."<<endl;);

    assert(_autoPlayer->handles2Obs());
    //now made for this solver only
    assert(_params->solverType==SeekerHSParams::SOLVER_TWO_HB_EXPL);
    assert(_otherPlayerInfoVec.size()==1);

    if (resetItStartTime) {
        //gettimeofday(&_tvLastItStartTime, NULL);
        startAlgoTimer();
    }

    //get action done
    int actionDone = _autoPlayer->deduceAction();

    //first prepare next step for all players
    prepareNextStep();

    //update player poses
    setPlayerInfoFromRobotPoses(_thisPlayerInfo, seekerPosXY, &hiderObsPosXY);
    if (seekerOtherPosXY!=NULL && seekerOtherPosXY->isSet()) {
        assert(hiderObsOtherPosXY!=NULL);
        setPlayerInfoFromRobotPoses(&_otherPlayerInfoVec[0], *seekerOtherPosXY, hiderObsOtherPosXY);
    }

    //select locations
    _autoPlayer->calcNextRobotPoses2(actionDone);

    //assume to have calculated the goals
    assert(_thisPlayerInfo->multiHasGoalPoses);
    assert(_thisPlayerInfo->multiGoalBPosesVec.size()>=1);
    //assert(_thisPlayerInfo->multiGoalBeliefVec.size()==2);

    //now set output
    newSeekerPosMine.fromBPos(_thisPlayerInfo->multiGoalBPosesVec[0], _params); // 0, _thisPlayerInfo->multiGoalBeliefVec[0]);
    if (_thisPlayerInfo->multiGoalBPosesVec.size()>1) {
        newSeekerPosOther.fromBPos(_thisPlayerInfo->multiGoalBPosesVec[1], _params); //0, _thisPlayerInfo->multiGoalBeliefVec[1]);
    } else {
        newSeekerPosOther.clear();
    }

    //set win state (although now doesn't change, maybe in future)
    _winState = HSGlobalData::GAME_STATE_RUNNING;
    if (winState==NULL)
        *winState = _winState;

    //log
    _gameLog.logLine(seekerPosXY, hiderObsPosXY, seekerOtherPosXY, hiderObsOtherPosXY, _winState);

    updateBeliefImg();

    DEBUG_SHS(cout << "SeekerHS::calcNextMultiRobotPoses2: done"<<endl
                   << "------------------------------------<"<<endl;);

    return true;
}

bool SeekerHS::calcMultiHBs(const PosXY &seekerPosXY, const PosXY &hiderObsPosXY, const PosXY *seekerOtherPosXY,
                            const PosXY *hiderObsOtherPosXY, std::vector<PosXY> &hbVec, int *winState,
                            bool *dontExecuteIteration, bool resetItStartTime)
{
    DEBUG_SHS(cout << "SeekerHS::calcMultiHBs ..."<<endl;);

    assert(_autoPlayer->handles2Obs());
    //now made for this solver only
    assert(_params->solverType==SeekerHSParams::SOLVER_MULTI_HB_EXPL);
    //assert(_otherPlayerInfoVec.size()==1);

    if (resetItStartTime) {
        //gettimeofday(&_tvLastItStartTime, NULL);
        startAlgoTimer();
    }

    //first prepare next step for all players
    prepareNextStep();

    //update player poses
    setPlayerInfoFromRobotPoses(_thisPlayerInfo, seekerPosXY, &hiderObsPosXY);
    if (seekerOtherPosXY!=NULL && seekerOtherPosXY->isSet()) {
        assert(hiderObsOtherPosXY!=NULL);
        setPlayerInfoFromRobotPoses(&_otherPlayerInfoVec[0], *seekerOtherPosXY, hiderObsOtherPosXY);
    }

    //calc HB list
    _autoPlayer->calcNextHBList();

    //assume to have calculated the goals
    assert(_thisPlayerInfo->multiHasHBPoses);
    assert(_thisPlayerInfo->multiHBPosVec.size()>=1);
    //assert(_thisPlayerInfo->multiGoalBeliefVec.size()==2);

    //now set output
    hbVec.resize(_thisPlayerInfo->multiHBPosVec.size());
    for(size_t i=0; i<_thisPlayerInfo->multiHBPosVec.size(); i++) {
        hbVec[i].fromBPos(_thisPlayerInfo->multiHBPosVec[i],_params);
    }

    //set win state (although now doesn't change, maybe in future)
    _winState = HSGlobalData::GAME_STATE_RUNNING;
    if (winState==NULL)
        *winState = _winState;

    //log
    _gameLog.logLine(seekerPosXY, hiderObsPosXY, seekerOtherPosXY, hiderObsOtherPosXY, _winState);

    updateBeliefImg();

    DEBUG_SHS(cout << "SeekerHS::calcMultiHBs: done"<<endl
                   << "------------------------------------<"<<endl;);

    return true;
}


double SeekerHS::calcNextOrientation() {
    assert(_thisPlayerInfo->currentPos.isSet());
    assert(_thisPlayerInfo->nextPos.isSet());

    //get chosen hider pos, clear if not consistent
    Pos hiderPos;
    if (!_autoPlayer->getChosenHiderPos(hiderPos)) {
        hiderPos.clear();
    }

    double orient1st = 0;
    if (_winState==HSGlobalData::GAME_STATE_RUNNING) {
        //if not possible, check how to fix
        _autoPlayer->setMinDistanceToObstacle(_thisPlayerInfo->nextPos);

        if (hiderPos.isSet()) { //lastHiderPos.isSet()) { //TODO: CHECK !!!
            //face to hider
            orient1st = getOrientation(_thisPlayerInfo->nextPos, hiderPos);
            //DEBUG_SHS(cout<<"Orientation - "<<(180*orient/M_PI)<<"º "<<_nextSeekerPos->toString()<<" to " <<" hpos="<<hiderPos.toString(););
        } else {
            //face in direction of going
            orient1st = getOrientation(_thisPlayerInfo->currentPos, _thisPlayerInfo->nextPos); //lastSeekerPos, *_nextSeekerPos);
        }

    } else { //not running OR next to hider (AG140422)
        if (hiderPos.isSet()) {
            //face to hider
            orient1st = getOrientation(_thisPlayerInfo->currentPos, hiderPos);
            //AG140522: check if different
            if (orientDiff(_lastOrient,orient1st)<_params->maxDiffOrientation_rad) {
                orient1st = _lastOrient; //don't turn
            }
        } else {
            //face in direction of going
            orient1st = getOrientation(_thisPlayerInfo->currentPos, _thisPlayerInfo->nextPos); //TODO: should be same orientatin, no change.
        }
    }

    //_lastOrient = orient1st;
    return orient1st;
}

bool SeekerHS::select2SeekerPose(const PosXY* newSeekerPosXYMineFromOther, const PosXY* newSeekerPosXYOtherFromOther,
                                     PosXY& newSeekerPosXYMine)
{
                /*(const std::vector<double> &newSeekerPosVectorMineFromOther, const std::vector<double> &newSeekerPosVectorOtherFromOther,
                                     double newSeekerPosMineBeliefFromOther, double newSeekerPosOtherBeliefFromOther, int n,
                                     std::vector<double>& newSeekerPosVector)
{*/

    DEBUG_SHS(cout << "SeekerHS::select2SeekerPose ..."<<endl;);
    assert(_params->solverType==SeekerHSParams::SOLVER_TWO_HB_EXPL);
    //assume to have calculated the goals
    assert(_thisPlayerInfo->multiHasGoalPoses);
    assert(_thisPlayerInfo->multiGoalBPosesVec.size()>=1);
    //assert(_thisPlayerInfo->multiGoalBeliefVec.size()==2);
    assert(_otherPlayerInfoVec.size()==1);

    //set robot goals in other player
    PlayerInfo* otherSeekerPlayer = &_otherPlayerInfoVec[0];
    otherSeekerPlayer->multiGoalBPosesVec.clear(); //resize(2);
    //otherSeekerPlayer->multiGoalBeliefVec.resize(2);
    otherSeekerPlayer->multiGoalIDVec.clear(); //resize(2);

    //set the goal vector of the other player (if received)
    if (newSeekerPosXYOtherFromOther!=NULL || !newSeekerPosXYOtherFromOther->isSet()) {
        otherSeekerPlayer->multiGoalIDVec.push_back(otherSeekerPlayer->id);
        otherSeekerPlayer->multiGoalBPosesVec.push_back(newSeekerPosXYOtherFromOther->toBPos(_params));

        if (newSeekerPosXYMineFromOther!=NULL || !newSeekerPosXYMineFromOther->isSet()) {
            otherSeekerPlayer->multiGoalIDVec.push_back(_thisPlayerInfo->id);
            otherSeekerPlayer->multiGoalBPosesVec.push_back(newSeekerPosXYMineFromOther->toBPos(_params));
        }

        otherSeekerPlayer->multiHasGoalPoses = true;
    } else {
        assert(newSeekerPosXYMineFromOther==NULL || !newSeekerPosXYMineFromOther->isSet());
    }

    //calculate next pos
    _thisPlayerInfo->nextPos = _autoPlayer->selectRobotPosMulti();

    assert(_thisPlayerInfo->nextPos.isSet());

    _lastOrient = calcNextOrientation();

#ifdef DEBUG_SHS_ON
    cout <<"SeekerHS.selectMultiSeekerPose: new seeker pos: "<<_thisPlayerInfo->nextPos.toString()<<", orientation: "<<(180*_lastOrient/M_PI)<<"º, "
         << "Win state: ";
    switch(_winState)  {
        case HSGlobalData::GAME_STATE_RUNNING:
            cout<<"playing";
            break;
        case HSGlobalData::GAME_STATE_TIE:
            cout << "TIE";
            break;
        case HSGlobalData::GAME_STATE_HIDER_WON:
            cout << "HIDER WINS";
            break;
        case HSGlobalData::GAME_STATE_SEEKER_WON:
            cout << "SEEKER WINS";
            break;
    }
    cout <<endl;
#endif

    //now convert
    newSeekerPosXYMine.fromPos(_thisPlayerInfo->nextPos, _params, _lastOrient);

    //log
    _gameLog.logLine2(&newSeekerPosXYMine);

    _thisPlayerInfo->numberActions++;

    //AG150729: update belief to show chosenGoalPos
    if (_thisPlayerInfo->chosenGoalPos.isSet()) {
    updateBeliefImg();
    }

    DEBUG_SHS(cout << "SeekerHS::select2SeekerPose: done"<<endl
                   << "------------------------------------<"<<endl;);

    return true;
}


bool SeekerHS::selectMultiSeekerPoseFromHB(const std::vector<PosXY> &otherHBVec, PosXY &newSeekerPosXYMine)
{
    DEBUG_SHS(cout << "SeekerHS::selectMultiSeekerPoseFromHB ..."<<endl;);
    assert(_params->solverType==SeekerHSParams::SOLVER_MULTI_HB_EXPL);

    //assume to have calculated the goals
    assert(_thisPlayerInfo->multiHasHBPoses);
    assert(_thisPlayerInfo->multiHBPosVec.size()>=1);
    assert(_otherPlayerInfoVec.size()==1);

    //set robot goals in other player
    PlayerInfo* otherSeekerPlayer = &_otherPlayerInfoVec[0];

    if (!otherHBVec.empty()) {
        otherSeekerPlayer->multiHBPosVec.resize(otherHBVec.size());
        for(size_t i=0; i<otherHBVec.size(); i++) {
            otherSeekerPlayer->multiHBPosVec[i] = otherHBVec[i].toBPos(_params);
        }
        otherSeekerPlayer->multiHasHBPoses = true;
    } else {
        otherSeekerPlayer->multiHasHBPoses = false;
        otherSeekerPlayer->multiHBPosVec.clear();
    }

    //calculate next pos
    _thisPlayerInfo->nextPos = _autoPlayer->selectRobotPosMulti();

    assert(_thisPlayerInfo->nextPos.isSet());

    _lastOrient = calcNextOrientation();

#ifdef DEBUG_SHS_ON
    cout <<"SeekerHS.selectMultiSeekerPose: new seeker pos: "<<_thisPlayerInfo->nextPos.toString()<<", orientation: "<<(180*_lastOrient/M_PI)<<"º, "
         << "Win state: ";
    switch(_winState)  {
        case HSGlobalData::GAME_STATE_RUNNING:
            cout<<"playing";
            break;
        case HSGlobalData::GAME_STATE_TIE:
            cout << "TIE";
            break;
        case HSGlobalData::GAME_STATE_HIDER_WON:
            cout << "HIDER WINS";
            break;
        case HSGlobalData::GAME_STATE_SEEKER_WON:
            cout << "SEEKER WINS";
            break;
    }
    cout <<endl;
#endif

    //now convert
    newSeekerPosXYMine.fromPos(_thisPlayerInfo->nextPos, _params, _lastOrient);

    //log
    _gameLog.logLine2(&newSeekerPosXYMine);

    _thisPlayerInfo->numberActions++;

    //AG150729: update belief to show chosenGoalPos
    if (_thisPlayerInfo->chosenGoalPos.isSet()) {
    updateBeliefImg();
    }

    DEBUG_SHS(cout << "SeekerHS::selectMultiSeekerPoseFromHB: done"<<endl
                   << "------------------------------------<"<<endl;);

    return true;
}

//TODO get next pos multi input
int SeekerHS::getNextPos(const PosXY &seekerPosXY, const PosXY &hiderObsPosXY, PosXY &newSeekerPosXY, int *winState,
                          bool *dontExecuteIteration, bool resetItStartTime) {

/*(vector<double> seekerPosVector, vector<double> hiderPosVector, vector<double>& newSeekerPosVector,
                          int* winStateRet, bool resetItStartTime) {*/

    DEBUG_SHS(cout  << ">--------------------------------------------"<<endl
                    << "SeekerHS::getNextPose"<<endl;);

    //this should handle solvers for only 1 seeker, and 1 hider
    assert(!_autoPlayer->handles2Obs());
    assert(_otherPlayerInfoVec.empty());

    if (resetItStartTime) {
        //gettimeofday(&_tvLastItStartTime, NULL);
        startAlgoTimer();
    }

    bool ok = true;

    //get action done
    int actionDone = _autoPlayer->deduceAction();

    //first prepare next step for all players
    prepareNextStep();

    //update player poses
    setPlayerInfoFromRobotPoses(_thisPlayerInfo, seekerPosXY, &hiderObsPosXY);

    //new action
    int newAction = -1;

    if (dontExecuteIteration!=NULL && *dontExecuteIteration==true) {
        //don't execute, keep at same place
        DEBUG_SHS(cout<<"SeekerHS::getNextPos: NOT EXECUTING ITERATION";);
        _thisPlayerInfo->nextPos = _thisPlayerInfo->currentPos;
        ok = false;
    } else {
        //select locations
        _thisPlayerInfo->nextPos = _autoPlayer->getNextPos(actionDone, &newAction);
    }

    //assume to have calculated the goals
    assert(_thisPlayerInfo->nextPos.isSet());

    //set orientation
    _lastOrient = calcNextOrientation();

    //set win state (although now doesn't change, maybe in future)
    _winState = HSGlobalData::GAME_STATE_RUNNING;
    if (winState==NULL)
        *winState = _winState;

    newSeekerPosXY.fromPos(_thisPlayerInfo->nextPos, _params, _lastOrient);

    //TODO LOG
    //logLine2(seekerPosVectorMine, hiderPosVectorMine, seekerPosVectorOther, hiderPosVectorOther, seekerPosMineIn, hiderPosMineIn, hiderVisib,
    //         seekerPosOtherIn, hiderPosOtherIn, newSeekerPosVectorMine, newSeekerPosVectorOther, newSeekerPosMineOut, newSeekerPosOtherOut,
    //         newSeekerPosMineBelief, newSeekerPosOtherBelief, _winState);

    updateBeliefImg();

    //log
    _gameLog.logLine(seekerPosXY, hiderObsPosXY, NULL, NULL, _winState);
    _gameLog.logLine2(&newSeekerPosXY);

    DEBUG_SHS(cout << "SeekerHS.getNextMultiSeekerPoses: done (now waiting goals of other)"<<endl;);

    return newAction;
}


void SeekerHS::updateBeliefImg() {
#ifndef DO_NOT_WRITE_BELIEF_IMG
    //AG140502: added mutex and deleted old belief img
    _beliefImgMutex.enter();
    if (_beliefImg!=NULL) delete _beliefImg;
    _beliefImg = _autoPlayer->getMapBeliefAsImage(/*seekerPosMineIn, seekerPosOtherIn, hiderPosMineIn, hiderPosOtherIn,*/ _params->beliefImageCellWidth);

    //update belief img
    if (!_params->beliefImageFile.empty()) {
        _autoPlayer->storeImage(_beliefImg, _params->beliefImageFile);
    }
    _beliefImgMutex.exit();
#endif
}

#ifdef OLD_CODE
int SeekerHS::getNextPose(vector<double> seekerPosVector, vector<double> hiderPosVector, vector<double>& newSeekerPosVector,
                          int* winStateRet, bool resetItStartTime) {

    if (resetItStartTime) {
        //gettimeofday(&_tvLastItStartTime, NULL);
        startAlgoTimer();
    }

    vector<int> nextPoses = getNextMultiplePoses(seekerPosVector, hiderPosVector, 1, newSeekerPosVector, winStateRet, false);
    assert(nextPoses.size()==1);
    return nextPoses[0];
}

vector<int> SeekerHS::getNextMultiplePoses(vector<double> seekerPosVector, vector<double> hiderPosVector, int n,
        vector<double>& newSeekerPosVector, int* winStateRet, bool resetItStartTime) {
    throw CException(_HERE_,"SeekerHS::getNextMultiplePoses: use getNextMultiplePosesForMultipleObs");
}

bool SeekerHS::getNextMultiSeekerPoses(const std::vector<double> &seekerPosVectorMine, const std::vector<double> &hiderPosVectorMine,
                                       const std::vector<double> &seekerPosVectorOther, const std::vector<double> &hiderPosVectorOther,
                                       int n, std::vector<double> &newSeekerPosVectorMine, std::vector<double> &newSeekerPosVectorOther,
                                       double &newSeekerPosMineBelief, double &newSeekerPosOtherBelief, int *winState,
                                       bool *dontExecuteIteration, bool resetItStartTime) {

    DEBUG_SHS(cout  << ">--------------------------------------------"<<endl
                    << "SeekerHS.getNextMultiSeekerPoses - "<<flush;);

    bool ok = true;

    /*cout << "  * seeker (mine): ["<<seekerPosVectorMine.size()<<flush <<"] "<<seekerPosVectorMine[0]<<","<<seekerPosVectorMine[1]<<endl
    << "  * hider (mine):  ";
    if (hiderPosVectorMine.size()==0)
        cout <<"-";
    else
        cout << hiderPosVectorMine[0]<<","<<hiderPosVectorMine[1];
    cout << endl<<"  * seeker (other):  ";
    if (seekerPosVectorOther.size()==0)
        cout <<"-";
    else
        cout << seekerPosVectorOther[0]<<","<<seekerPosVectorOther[1];
    cout << endl << "  * hider (other):  ";
    if (hiderPosVectorOther.size()==0)
        cout <<"-";
    else
        cout << hiderPosVectorOther[0]<<","<<hiderPosVectorOther[1];
    cout << endl; // << " autopl.handles2obs="<<_autoPlayer->handles2Obs()<<",n=" << n <<endl;*/

    assert(n>0);
    assert(_autoPlayer->handles2Obs());
    assert(seekerPosVectorMine.size()>=2);

    if (resetItStartTime) {
        //gettimeofday(&_tvLastItStartTime, NULL);
        startAlgoTimer();
    }

    //convert to pos
    Pos seekerPosMineIn;
    IDPos hiderPosMineIn;
    Pos *seekerPosOtherIn=NULL;
    Pos *hiderPosOtherIn=NULL;

    //set poses
    //cout<<"set poses: s1="<<flush;
    setPosFromVector(seekerPosMineIn, seekerPosVectorMine);
    //cout <<seekerPosMineIn.toString()<<", h1="<<flush;
    setPosFromVector(hiderPosMineIn, hiderPosVectorMine);
    //cout <<hiderPosMineIn.toString()<<flush;
    if (seekerPosVectorOther.size()>0) {
        //cout<<", s2="<<flush;
        seekerPosOtherIn = new Pos();
        setPosFromVector(*seekerPosOtherIn, seekerPosVectorOther);
        //cout<<seekerPosOtherIn->toString()<<flush;
    }
    if (hiderPosVectorOther.size()>0) {
        //cout<<", h2="<<flush;
        hiderPosOtherIn = new Pos();
        setPosFromVector(*hiderPosOtherIn, hiderPosVectorOther);
        //cout<<hiderPosOtherIn->toString()<<flush;
    }
    //cout<<endl;

    //actions to be returned
    vector<int> actions;

    DEBUG_SHS(
        cout << "INPUT: "<<endl
        << "  * seeker (mine): "<<seekerPosVectorMine[0]<<","<<seekerPosVectorMine[1]<< " ("<<seekerPosMineIn.toString()<<")"<<endl
        << "  * hider (mine):  ";
        if (hiderPosVectorMine.size()==0)
            cout <<"-";
        else
            cout << hiderPosVectorMine[0]<<","<<hiderPosVectorMine[1];
        cout <<" ("<<hiderPosMineIn.toString()<<")"<<endl
             << "  * seeker (other):  ";
        if (seekerPosVectorOther.size()==0)
            cout <<"-";
        else
            cout << seekerPosVectorOther[0]<<","<<seekerPosVectorOther[1];
        if (seekerPosOtherIn!=NULL) cout <<" ("<<seekerPosOtherIn->toString()<<")";
        cout << endl << "  * hider (other):  ";
        if (hiderPosVectorOther.size()==0)
            cout <<"-";
        else
            cout << hiderPosVectorOther[0]<<","<<hiderPosVectorOther[1];
        if (hiderPosOtherIn!=NULL) cout <<" ("<<hiderPosOtherIn->toString()<<")";

        cout<<endl<<endl    << "  Doing next action #"<<_numActions;
        if (_params->maxNumActions>0)
            cout<<"/"<<_params->maxNumActions;
        cout << ", max time="<< _params->maxGameTime <<" s ..."<<endl;
    );

    if (dontExecuteIteration!=NULL && *dontExecuteIteration==true) {
        DEBUG_SHS(cout<<"SeekerHS.getNextMultiSeekerPoses: NOT EXECUTING ITERATION";);
        //actions.push_back(HSGlobalData::ACT_H);
        newSeekerPosVectorMine = seekerPosVectorMine;

        _winState = HSGlobalData::GAME_STATE_RUNNING;
        if (winState!=NULL) {
            *winState = _winState;
        }
        ok = false;
    }

    if (ok) {
        //hider is visible
        bool hiderVisib = hiderPosMineIn.isSet();

        //set chosen hider vector (just to have it in vector format and send to ROS)
        if (!hiderVisib) {
            _chosenHiderPosXY.clear();
        } else {
            _chosenHiderPosXY.resize(2);
            setVectorFromPos(_chosenHiderPosXY, hiderPosMineIn, 0, 0, false);
        }

        cout <<"Map of input:"<<endl;
        _map->printMap(seekerPosMineIn,hiderPosMineIn);


        /*
        //win state
        int winStateGame = getWinState(seekerPos, hiderPos, hiderVisib);

        //AG140310: allow to continue even though a 'final state' has been reached
        int winState = winStateGame;
        if (!_params->stopAtWin && winState > HSGlobalData::GAME_STATE_RUNNING) {
            //reset to running state, since we want to continue
            winState = HSGlobalData::GAME_STATE_RUNNING;
        }

        if (winStateRet!=NULL) {
            //returning win state
            *winStateRet = winState; //Game;
        }*/
        //AG150209: disabled win state, since this is find&follow
        _winState = HSGlobalData::GAME_STATE_RUNNING;
        if (winState!=NULL) {
            *winState = _winState;
        }

        //ag240422: no move if close
        bool nextToHider = false;
        if (hiderVisib && _map->distanceEuc(seekerPosMineIn,hiderPosMineIn)<=_params->winDist) {
            nextToHider = true;
        }

        //AG150209: always run, even if close, filter in 'selectpos..'
        /*double orient1st = -1;
        if (winState==HSGlobalData::GAME_STATE_RUNNING && !nextToHider) {*/
            //AG140310: for find-and-follow reduce steps in future if close to hider
        if (_params->gameType==HSGlobalData::GAME_FIND_AND_FOLLOW || _params->gameType==HSGlobalData::GAME_FIND_AND_FOLLOW_2ROB) {
            if (hiderVisib) {
                //lower n if the hider is closer
                double d = _map->distance(seekerPosMineIn,hiderPosMineIn);
                if (d<n) {
                    if (d<=1) {
                        n = 1;
                    } else {
                        n = (int)d;
                    }
                }
            }
        }


        //DEBUG_SHS(cout<<"Sending "<<n<<" pos: ";);


    #ifdef USE_ACTIONS //AG140328: now we are actually using getNextPos, so we don't have to 'execute' the position here
        throw CException(_HERE_,"USE_ACTIONS not implemented by this function");
    #endif //else: using new pos

        //AG140403
        int lastA = -1;
        if (_params->useDeducedAction) {
            lastA = _autoPlayer->deduceAction(seekerPosMineIn);
        }

        //AG150209: now get the next poses
        vector<Pos> goalPosesVec;
        vector<double> goalPosesBeliefVec;
        Pos newSeekerPosMineOut, newSeekerPosOtherOut;
        bool ok = _autoPlayer->getNextRobotPoses2(seekerPosMineIn, hiderPosMineIn, hiderVisib, seekerPosOtherIn, hiderPosOtherIn, actions,
                                        goalPosesVec, lastA, n, &goalPosesBeliefVec);

        if (!ok) {
            DEBUG_SHS(cout << "SeekerHS.getNextMultiSeekerPoses: getNextRobotPoses2 did not end successfully.");
        } else {
            //at least the own seeker location
            assert(goalPosesVec.size()>0);

            //convert poses to vector
            newSeekerPosMineOut = goalPosesVec[0];
            assert(newSeekerPosMineOut.isSet());
            setVectorFromPos(newSeekerPosVectorMine, newSeekerPosMineOut, 0, 0, false);
            if (goalPosesVec.size()>1) {
                newSeekerPosOtherOut = goalPosesVec[1];
                assert(newSeekerPosOtherOut.isSet());
                setVectorFromPos(newSeekerPosVectorOther, newSeekerPosOtherOut, 0, 0, false);
            } else {
                newSeekerPosVectorOther.clear();
            }

            //now belief
            newSeekerPosMineBelief = newSeekerPosOtherBelief = -1;
            if (goalPosesBeliefVec.size()>0) {
                newSeekerPosMineBelief = goalPosesBeliefVec[0];
                if (goalPosesBeliefVec.size()>1)
                    newSeekerPosOtherBelief = goalPosesBeliefVec[1];
            }

            DEBUG_SHS(cout << "SeekerHS.getNextMultiSeekerPoses: created planned goals");
        }

        logLine2(seekerPosVectorMine, hiderPosVectorMine, seekerPosVectorOther, hiderPosVectorOther, seekerPosMineIn, hiderPosMineIn, hiderVisib,
                 seekerPosOtherIn, hiderPosOtherIn, newSeekerPosVectorMine, newSeekerPosVectorOther, newSeekerPosMineOut, newSeekerPosOtherOut,
                 newSeekerPosMineBelief, newSeekerPosOtherBelief, _winState);

    #ifndef DO_NOT_WRITE_BELIEF_IMG
        //AG140502: added mutex and deleted old belief img
        _beliefImgMutex.enter();
        if (_beliefImg!=NULL) delete _beliefImg;
        _beliefImg = _autoPlayer->getMapBeliefAsImage(seekerPosMineIn, seekerPosOtherIn, hiderPosMineIn, hiderPosOtherIn, _params->beliefImageCellWidth);

        //update belief img
        if (!_params->beliefImageFile.empty()) {
            _autoPlayer->storeImage(_beliefImg, _params->beliefImageFile);
        }
        _beliefImgMutex.exit();
    #endif

        DEBUG_SHS(cout << "SeekerHS.getNextMultiSeekerPoses: done (now waiting goals of other)"<<endl;);

        ok = true;
    }

    if (seekerPosOtherIn!=NULL)
        delete seekerPosOtherIn;
    if (hiderPosOtherIn!=NULL)
        delete hiderPosOtherIn;

    return ok;
}

bool SeekerHS::selectMultiSeekerPose(const std::vector<double> &newSeekerPosVectorMineFromOther, const std::vector<double> &newSeekerPosVectorOtherFromOther,
                                     double newSeekerPosMineBeliefFromOther, double newSeekerPosOtherBeliefFromOther, int n,
                                     std::vector<double>& newSeekerPosVector)
{

    DEBUG_SHS(cout << "SeekerHS.selectMultiSeekerPose ..."<<endl;);

    assert(_autoPlayer->handles2Obs());

    //convert to pos
    Pos seekerPosMineFromOtherIn, seekerPosOtherFromOtherIn;

    //set poses
    setPosFromVector(seekerPosMineFromOtherIn, newSeekerPosVectorMineFromOther);
    setPosFromVector(seekerPosOtherFromOtherIn, newSeekerPosVectorOtherFromOther);

    //now select new pos
    Pos nextSeekerPos = _autoPlayer->selectRobotPos2(&seekerPosMineFromOtherIn, &seekerPosOtherFromOtherIn, newSeekerPosMineBeliefFromOther,
                                                  newSeekerPosOtherBeliefFromOther, n);


    if (!nextSeekerPos.isSet()) {
        DEBUG_SHS(
            /*cout<<"WARNING  Action(s) ";
            if (actions.size()>0) cout<<" #"<<actions.size()<<ACTION_COUT(actions[0]);
            cout <<" (from "<<seekerPos.toString()<<" to "<<_nextSeekerPos->toString()<<") not possible setting current pos as goal"<<endl;*/
            cout << "SeekerHS.selectMultiSeekerPose - WARNING: new seeker pos could not be obtained, using previous!"<<endl;
        );
        nextSeekerPos = *_nextSeekerPos;
    }
    *_nextSeekerPos = nextSeekerPos;

    double orient1st = 0;
    Pos lastHiderPos = _autoPlayer->getLastHiderPos();
    Pos lastSeekerPos = _autoPlayer->getLastSeekerPos();

    if (_winState==HSGlobalData::GAME_STATE_RUNNING) {
        //AG140403: set pos
        //_lastPos = seekerPos;

        //if not possible, check how to fix
        _autoPlayer->setMinDistanceToObstacle(*_nextSeekerPos);

        //double orient = 0;
        if (lastHiderPos.isSet()) { //TODO: CHECK !!!
            //face to hider
            orient1st = getOrientation(*_nextSeekerPos, lastHiderPos);
            //DEBUG_SHS(cout<<"Orientation - "<<(180*orient/M_PI)<<"º "<<_nextSeekerPos->toString()<<" to " <<" hpos="<<hiderPos.toString(););
        } else {
            //face in direction of going
            orient1st = getOrientation(lastSeekerPos, *_nextSeekerPos);
        }

        //set pos in return vector
        setVectorFromPos(newSeekerPosVector, *_nextSeekerPos, orient1st);


    } else { //not running OR next to hider (AG140422)
        //_seekerPlayer->setCurPos(seekerPos);
        *_nextSeekerPos = _autoPlayer->getLastSeekerPos();

        if (lastHiderPos.isSet()) {
            //face to hider
            orient1st = getOrientation(lastSeekerPos, lastHiderPos);
            //AG140522: check if different
            if (orientDiff(_lastOrient,orient1st)<_params->maxDiffOrientation_rad) {
                orient1st = _lastOrient; //don't turn
            }
        } else {
            //face in direction of going
            //if (orientDiff)
            orient1st = getOrientation(lastSeekerPos, *_nextSeekerPos); //TODO: should be same orientatin, no change.
        }
    }


    //set output vector
    newSeekerPosVector.resize(3);
    setVectorFromPos(newSeekerPosVector, *_nextSeekerPos, orient1st); //AG140328: changed *_nextSeekerPos -> seekerPos

    _lastOrient = orient1st;


    //new pos
    /*Pos p = _seekerPlayer->getCurPos();
    _nextSeekerPos->set(p);*/

    //ag130411: undo movement, since we should get the new position in the next iteration
    /*_seekerPlayer->setCurPos(seekerPos);
    //set hider pos
    if (hiderVisib) {
        _seekerPlayer->setPlayer2Pos(hiderPos);
    } else {
        _seekerPlayer->setPlayer2Pos(-1,-1);
    }*/

#ifdef DEBUG_SHS_ON
    /*cout << "Action: ";
    if (actions.size()>0)
        cout << ACTION_COUT(actions[0]);
    else
        cout <<"dir";*/
    cout <<"SeekerHS.selectMultiSeekerPose: new seeker pos: "<<_nextSeekerPos->toString()<<", orientation: "<<(180*orient1st/M_PI)<<"º, "
         << "Win state: ";
    switch(_winState)  {
        case HSGlobalData::GAME_STATE_RUNNING:
            cout<<"playing";
            break;
        case HSGlobalData::GAME_STATE_TIE:
            cout << "TIE";
            break;
        case HSGlobalData::GAME_STATE_HIDER_WON:
            cout << "HIDER WINS";
            break;
        case HSGlobalData::GAME_STATE_SEEKER_WON:
            cout << "SEEKER WINS";
            break;
    }
    /*if (winState!=winStateGame) {
        cout << " (but game continues)";
    }*/
    cout <<endl;
#endif

    //assert(seekerPosVector.size()>=2);
    assert(newSeekerPosVector.size()>=2);

    //assert(hiderPosVector.size()>=2 && hiderVisib || !hiderVisib && (hiderPosVector.size()==0 || _params->simulateNotVisible));

    /*cout <<"hidervisib="<<hiderVisib<<"; chosenhiderposidx="<<chosenHiderPosIndex<<"; hiderpsovec.sz="<<hiderPosVector.size()<<";chosenhiderpos:"<<flush;
    if (chosenHiderPosIndex<0) {
        cout<<"none";
    } else {
        if (hiderPosVector[chosenHiderPosIndex].empty()) {
            cout <<"empty";
        } else {
            cout <<hiderPosVector[chosenHiderPosIndex][0]<<","<<hiderPosVector[chosenHiderPosIndex][1];
        }
    } cout <<endl;

    assert( (!hiderVisib && (chosenHiderPosIndex<0 || hiderPosVector[chosenHiderPosIndex].empty())) ||
           (hiderVisib && chosenHiderPosIndex>=0 && hiderPosVector[chosenHiderPosIndex].size()>=2) );*/
    /*assert(newSeekerPosVector);*/


    logLine2Select(newSeekerPosVectorMineFromOther, newSeekerPosVectorOtherFromOther, seekerPosMineFromOtherIn,
                   seekerPosOtherFromOtherIn, newSeekerPosMineBeliefFromOther,
                   newSeekerPosOtherBeliefFromOther, newSeekerPosVector, *_nextSeekerPos);

    //one action 'done'
    _numActions++;

    DEBUG_SHS(cout << "SeekerHS.selectMultiSeekerPose: done"<<endl
                   << "------------------------------------<"<<endl;);


    return true;
}


vector<int> SeekerHS::getNextMultiplePosesForMultipleObs(vector<double> seekerPosVector, vector< vector<double> > hiderPosVector, int n,
        vector<double>& newSeekerPosVector, int* winStateRet, bool* dontExecuteIteration, bool resetItStartTime) {

    assert(n>0);

    if (resetItStartTime) {
        //gettimeofday(&_tvLastItStartTime, NULL);
        startAlgoTimer();
    }

    //convert to pos
    Pos seekerPosIn;
    vector<IDPos> hiderIDPosVector;
    getPosFromVectors(seekerPosVector, hiderPosVector, seekerPosIn, hiderIDPosVector, _params->simulateNotVisible);

    //actions to be returned
    vector<int> actions;

    DEBUG_SHS(
        cout << "SeekerHS.getNextMultiplePoses INPUT: "<<endl
        << "  * seeker: "<<seekerPosVector[0]<<","<<seekerPosVector[1]
        << " ("<<seekerPosIn.toString()<<")"<<endl
        << "  * hider(#"<<hiderPosVector.size()<<") ";
        /*if (hiderPosVector.size()>0) cout <<hiderPosVector[0]<<","<<hiderPosVector[1];
        cout << " ("<<hiderPos.toString()<<") hider visib="<<visib<<endl*/
        cout<<endl    << "  Doing next action #"<<_numActions;
        if (_params->maxNumActions>0)
            cout<<"/"<<_params->maxNumActions;
        cout << ", max time="<< _params->maxGameTime <<" s ..."<<endl;
    );

    Pos seekerPos;
    IDPos hiderPos;
    bool dontExec = false;
    //check and filter the poses
    _autoPlayer->checkAndFilterPoses(seekerPosIn, hiderIDPosVector, seekerPos, hiderPos, dontExec);

    if (dontExec) {
        DEBUG_SHS(cout<<"SeekerHS.getNextMultiplePoses: NOT EXECUTING ITERATION";);
        actions.push_back(HSGlobalData::ACT_H);
        newSeekerPosVector = seekerPosVector;
        if (dontExecuteIteration!=NULL) {
            *dontExecuteIteration = dontExec;
        }
        if (winStateRet!=NULL) {
            *winStateRet = HSGlobalData::GAME_STATE_RUNNING;
        }
        return actions;
    }

    //hider is visible
    bool hiderVisib = hiderPos.isSet();

    //set chosen hider vector (just to have it in vector format and send to ROS)
    if (!hiderVisib) {
        _chosenHiderPosXY.clear();
    } else {
        _chosenHiderPosXY.resize(2);
        setVectorFromPos(_chosenHiderPosXY, hiderPos, 0, 0, false);
    }


    /*if (visib) {
        bool visibSim = _map->isVisible(seekerPos,hiderPos);
        if (!visibSim) {
            DEBUG_SHS(cout << " (current visible but according to raytrace not - "<<endl;);
            if (_params->allowInconsistObs) {
                DEBUG_SHS(cout << "but allowing inconsistencies) "<<endl;);
            } else {
                visib = visibSim;
                DEBUG_SHS(cout << "reset visib to not visible) "<<endl;);
            }
        }
    }*/

    //_map->printMap();

    //DEBUG_SHS(cout << "Seeker pos: "<<seekerPos.toString()<<"; hider pos: "<< (visib ? hiderPos.toString() : "hidden")<<endl;);

/*#ifdef DEBUG_EXTRA_CHECKS
    if (!checkValidNextSeekerPos(seekerPosVector, !_params->allowInconsistObs)) {
        throw CException(_HERE_,"next seeker pos is not valid, even though it should have been fixed");
    }
#endif
    if (!checkValidNextHiderPos(hiderPosVector,seekerPosVector, !_params->allowInconsistObs)) {
        throw CException(_HERE_,"next hider pos is not valid");
    }

    //check if new seeker pos complies with expected
    if (!_params->allowInconsistObs && !seekerPos.equals(*_nextSeekerPos)) {
        cout << "WARNING! expected seeker's position: "<<_nextSeekerPos->toString()<<", but got: "<<seekerPos.toString()<<endl;
        //TODO ERROR??
        throw CException(_HERE_,"robot is not on expected location");
    }
*/


    cout <<"Map of input:"<<endl;
    _map->printMap(seekerPos,hiderPos);

    //win state
    int winStateGame = getWinState(seekerPos, hiderPos, hiderVisib);

    //AG140310: allow to continue even though a 'final state' has been reached
    int winState = winStateGame;
    if (!_params->stopAtWin && winState > HSGlobalData::GAME_STATE_RUNNING) {
        //reset to running state, since we want to continue
        winState = HSGlobalData::GAME_STATE_RUNNING;
    }

    if (winStateRet!=NULL) {
        //returning win state
        *winStateRet = winState; //Game;
    }

    //ag240422: no move if close
    bool nextToHider = false;
    if (hiderVisib && _map->distanceEuc(seekerPos,hiderPos)<=_params->winDist) {
        nextToHider = true;
    }

    double orient1st = -1;
    if (winState==HSGlobalData::GAME_STATE_RUNNING && !nextToHider) {
        //AG140310: for find-and-follow reduce steps in future if close to hider
        if (_params->gameType==HSGlobalData::GAME_FIND_AND_FOLLOW) {
            if (hiderVisib) {
                //lower n if the hider is closer
                double d = _map->distance(seekerPos,hiderPos);
                if (d<n) {
                    if (d<=1) {
                        n = 1;
                    } else {
                        n = (int)d;
                    }
                }
            }
        }


        DEBUG_SHS(cout<<"Sending "<<n<<" pos: ";);


#ifdef USE_ACTIONS //AG140328: now we are actually using getNextPos, so we don't have to 'execute' the position here

        //get next action based on hider/seeker pos
        actions = _autoPlayer->getNextMultipleActions(seekerPos, hiderPos, visib, n);
        //action = _autoPlayer->getNextAction(seekerPos,hiderPos,visib);

        if (actions.size()!=n) {
            n = actions.size();
            DEBUG_SHS(cout << "(n changed to "<<n<<") ";);
        }

        //list for multiple poses
        Pos curPos;
        curPos.set(seekerPos);
        //set output vector
        newSeekerPosVector.resize(3*n);
        //1st orientation

        DEBUG_SHS(cout<<"SeekerHS::getNextMultiplePoses: sending following poses: "<<endl;);
        //create pose list
        //TODO: REPLACE THIS BY GETNEXTPOS!!!
        for(int i=0; i<n; i++) {
            DEBUG_SHS(cout <<"  * "<<i<<":[a="<<ACTION_COUT(actions[i])<<"] "<<flush;);
            //move
            Pos newPos = _map->tryMove(actions[i],curPos);

            if (!newPos.isSet()) {
                //if not possible, check how to fix
                DEBUG_SHS(cout<<"WARNING  Action "<<ACTION_COUT(actions[i])<<" not possible, trying from center"<<endl;);
                if (_params->useContinuousPos) {
                    curPos.convertValuesToInt();
                    curPos.add(0.5,0.5);
                    newPos = _map->tryMove(actions[i],curPos);

                    if (!newPos.isSet()) {
                        newPos.set(curPos);
                        DEBUG_SHS(cout<<"ERROR Action i"<<ACTION_COUT(actions[i])<<" not possible"<<endl;);
                    }
                }
                DEBUG_SHS(cout<<"    ";);
            }

            setMinDistanceToObstacle(newPos);

            double orient = 0;
            if (visib) { //TODO: CHECK !!!
                //face to hider
                orient = getOrientation(newPos, hiderPos);
            } else {
                //face in direction of going
                orient = getOrientation(curPos, newPos);
            }
            if (i==0) orient1st = orient;

            //set pos in return vector
            setVectorFromPos(newSeekerPosVector, newPos, orient, i);
            //set cur pos
            curPos.set(newPos);

            if (i==0) {
                //store first step in player (can be used in global check of seekerpos)
                //_seekerPlayer->setCurPos(newPos);
                _nextSeekerPos->set(newPos);
            }

            DEBUG_SHS(cout <<" ("<<newSeekerPosVector[i*3]<<","<<newSeekerPosVector[i*3+1]<<") or="
                      << newSeekerPosVector[i*3+2]<<" ("<< newPos.toString()<<")"<<endl;);
        }

        DEBUG_SHS(cout<<endl;);

#else //using new pos

        //AG140403
        int lastA = -1;
        if (_params->useDeducedAction) {
            lastA = _autoPlayer->deduceAction(seekerPos);
        }

        //AG150126
        if (_autoPlayer->handles2Obs()) {
            //TODO!!!
            //*_nextSeekerPos = _autoPlayer->getNextPos2(seekerPos, hiderPos, hiderVisib, actions, lastA, n);
        } else {
            *_nextSeekerPos = _autoPlayer->getNextPos(seekerPos, hiderPos, hiderVisib, actions, lastA, n);
        }
        if (!_nextSeekerPos->isSet()) {
            DEBUG_SHS(
                cout<<"WARNING  Action(s) ";
                if (actions.size()>0) cout<<" #"<<actions.size()<<ACTION_COUT(actions[0]);
                cout <<" (from "<<seekerPos.toString()<<" to "<<_nextSeekerPos->toString()<<") not possible setting current pos as goal"<<endl;
            );
            *_nextSeekerPos = seekerPos;
        }

        //AG140403: set pos
        //_lastPos = seekerPos;

        newSeekerPosVector.resize(3);

        //if not possible, check how to fix
        _autoPlayer->setMinDistanceToObstacle(*_nextSeekerPos);

        //double orient = 0;
        if (hiderVisib) { //TODO: CHECK !!!
            //face to hider
            orient1st = getOrientation(*_nextSeekerPos, hiderPos);
            //DEBUG_SHS(cout<<"Orientation - "<<(180*orient/M_PI)<<"º "<<_nextSeekerPos->toString()<<" to " <<" hpos="<<hiderPos.toString(););
        } else {
            //face in direction of going
            orient1st = getOrientation(seekerPos, *_nextSeekerPos);
        }

        //set pos in return vector
        setVectorFromPos(newSeekerPosVector, *_nextSeekerPos, orient1st);
#endif

    } else { //not running OR next to hider (AG140422)
        actions.push_back(0);
        //_seekerPlayer->setCurPos(seekerPos);
        *_nextSeekerPos = seekerPos;

        if (hiderVisib) {
            //face to hider
            orient1st = getOrientation(seekerPos, hiderPos);
            //AG140522: check if different
            if (orientDiff(_lastOrient,orient1st)<_params->maxDiffOrientation_rad) {
                orient1st = _lastOrient; //don't turn
            }
        } else {
            //face in direction of going
            //if (orientDiff)
            orient1st = getOrientation(seekerPos, seekerPos); //TODO: should be same orientatin, no change.
        }

        //set output vector
        newSeekerPosVector.resize(3);
        setVectorFromPos(newSeekerPosVector, *_nextSeekerPos, orient1st); //AG140328: changed *_nextSeekerPos -> seekerPos

        _lastOrient = orient1st;

        DEBUG_SHS(cout<<"Sending same pos:"<<seekerPos.toString()<<", because "<<(nextToHider?"close to hider":"in final state")<<endl;);
    }



    //new pos
    /*Pos p = _seekerPlayer->getCurPos();
    _nextSeekerPos->set(p);*/

    //ag130411: undo movement, since we should get the new position in the next iteration
    /*_seekerPlayer->setCurPos(seekerPos);
    //set hider pos
    if (hiderVisib) {
        _seekerPlayer->setPlayer2Pos(hiderPos);
    } else {
        _seekerPlayer->setPlayer2Pos(-1,-1);
    }*/

#ifdef DEBUG_SHS_ON
    cout << "Action: ";
    if (actions.size()>0)
        cout << ACTION_COUT(actions[0]);
    else
        cout <<"dir";
    cout <<", new seeker pos: "<<_nextSeekerPos->toString()<<", orientation: "<<(180*orient1st/M_PI)<<"º"<<endl
         << "Win state: ";
    switch(winStateGame)  {
        case HSGlobalData::GAME_STATE_RUNNING:
            cout<<"playing";
            break;
        case HSGlobalData::GAME_STATE_TIE:
            cout << "TIE";
            break;
        case HSGlobalData::GAME_STATE_HIDER_WON:
            cout << "HIDER WINS";
            break;
        case HSGlobalData::GAME_STATE_SEEKER_WON:
            cout << "SEEKER WINS";
            break;
    }
    if (winState!=winStateGame) {
        cout << " (but game continues)";
    }
    cout <<endl;
#endif

    assert(seekerPosVector.size()>=2);
    assert(newSeekerPosVector.size()>=2);
    //assert(hiderPosVector.size()>=2 && hiderVisib || !hiderVisib && (hiderPosVector.size()==0 || _params->simulateNotVisible));

    /*cout <<"hidervisib="<<hiderVisib<<"; chosenhiderposidx="<<chosenHiderPosIndex<<"; hiderpsovec.sz="<<hiderPosVector.size()<<";chosenhiderpos:"<<flush;
    if (chosenHiderPosIndex<0) {
        cout<<"none";
    } else {
        if (hiderPosVector[chosenHiderPosIndex].empty()) {
            cout <<"empty";
        } else {
            cout <<hiderPosVector[chosenHiderPosIndex][0]<<","<<hiderPosVector[chosenHiderPosIndex][1];
        }
    } cout <<endl;

    assert( (!hiderVisib && (chosenHiderPosIndex<0 || hiderPosVector[chosenHiderPosIndex].empty())) ||
           (hiderVisib && chosenHiderPosIndex>=0 && hiderPosVector[chosenHiderPosIndex].size()>=2) );*/
    /*assert(newSeekerPosVector);*/

    logLine(seekerPosVector,_chosenHiderPosXY,seekerPos,hiderPos,hiderVisib,newSeekerPosVector,*_nextSeekerPos,winState);

    //one action 'done'
    _numActions++;

#ifndef DO_NOT_WRITE_BELIEF_IMG

    //AG140502: added mutex and deleted old belief img
    _beliefImgMutex.enter();
    if (_beliefImg!=NULL) delete _beliefImg;
    _beliefImg = NULL; //TODO!!!!!!  _autoPlayer->getMapBeliefAsImage(seekerPos, _params->beliefImageCellWidth);

    //update belief img
    if (!_params->beliefImageFile.empty()) {
        _autoPlayer->storeImage(_beliefImg, _params->beliefImageFile);
                //MapBeliefAsImage(_params->beliefImageFile,seekerPos, _params->beliefImageCellWidth);
    }
    _beliefImgMutex.exit();
#endif

    DEBUG_SHS(cout << "SeekerHS.getNextMultiplPoses: done"<<endl;);

    return actions;
}

/*int SeekerHS::getNextPoseForMultipleObs(vector<double> seekerPosVector, vector< vector<double> > hiderPosVector,
                                        vector<double>& newSeekerPosVector, int *winState, bool resetItStartTime) {

    throw CException(_HERE_,"SeekerHS::getNextMultiplePoses: use getNextMultiplePosesForMultipleObs");

}*/
#endif //--OLD_CODE


//calculate rotation as from y axis
double SeekerHS::ang(double y, double x) {
    /*double ang = atan2(y,x);
    //- M_PI_2;
    ang =  (ang + M_PI);
    if (ang>2*M_PI) ang -= 2*M_PI;

    ang = ang - M_PI - M_PI_2;*/


    double ang = 0;

    if (CHANGE_ORIENTATION) {
        ang = atan2(-x,-y); //-y,x);
    } else {
        ang = atan2(-y,x);
    }

    ang -= M_PI_2;

    if (ang<=-M_PI)
        ang += 2*M_PI;


    return ang;
}

//static
double SeekerHS::orientDiff(double or1, double or2) {
    double d = abs(or1-or2);
    double d2 = abs(2*M_PI-d);
    return (d<d2?d:d2);
}

double SeekerHS::getOrientation(const Pos& pos1, const Pos& pos2) {
    double x,y;
    //AG150513: bug fix changed from int to double!!
    x = pos1.colDouble() - pos2.colDouble();
    y = pos1.rowDouble() - pos2.rowDouble();

    return ang(y,x);
}

inline bool SeekerHS::seekerWins(const Pos& seekerPos, const Pos& hiderPos) {
    if (_params->useContinuousPos) {
        return _map->distanceEuc(seekerPos,hiderPos)<=_params->winDist;
    } else if (_params->winDist==0) {
        return seekerPos.equals(hiderPos);
    } else {
        return _map->distance(seekerPos,hiderPos)<=_params->winDist;
    }
}

int SeekerHS::getWinState(const Pos& seekerPos, const Pos& hiderPos, bool hiderVisible) {
    if (_params->maxGameTime>0 && _params->maxGameTime<_timer.getTime(_timerID)) { //_numActions > _params->maxGameTime /*_maxNumActions*/) {
        return HSGlobalData::GAME_STATE_TIE;
    } else if (!hiderVisible || !hiderPos.isSet()) {
        //TODO: if not visible what to to do to check if hider wins ..
        return HSGlobalData::GAME_STATE_RUNNING;
    } else if ( seekerWins(seekerPos, hiderPos) ) {
        return HSGlobalData::GAME_STATE_SEEKER_WON;
    } else if ( _params->gameType==HSGlobalData::GAME_HIDE_AND_SEEK && _map->isBase(hiderPos) ) {
        return HSGlobalData::GAME_STATE_HIDER_WON;
    } else {
        return HSGlobalData::GAME_STATE_RUNNING;
    }
}




/*void SeekerHS::getPosFromVectors(vector<double> &seekerPosVector, vector<double> &hiderPosVector, Pos &seekerPos, Pos &hiderPos, bool &hiderVisible) {
    hiderVisible = (hiderPosVector.size()>0);

    //cout <<"hvis:"<<hiderVisible<<endl;


    setPosFromVector(seekerPos,seekerPosVector);

    //cout <<"seekerpos:"<<seekerPos.toString()<<endl;

    if (hiderVisible) {
        setPosFromVector(hiderPos,hiderPosVector);

        //cout<<"hiderpos:"<<hiderPos.toString()<<endl;
    } else {
        //hiderPos.row = hiderPos.col = -1;
        hiderPos.clear();
    }
}*/
#ifdef OLD_CODE
void SeekerHS::getPosFromVectors(const std::vector<double> &seekerPosVectorIn, const std::vector<std::vector<double> > &hiderPosVectorIn,
                                 Pos &seekerPosOut, std::vector<IDPos> &hiderPosVectorOut, bool filterHidden) {

    //set seeker pos
    setPosFromVector(seekerPosOut, seekerPosVectorIn);

    //hider positions
    for(size_t i=0; i<hiderPosVectorIn.size(); i++) {
        //set pos from vector
        IDPos hiderPos;
        setPosFromVector(hiderPos, hiderPosVectorIn[i]);

        if (_map->isPosInMap(hiderPos) && !_map->isObstacle(hiderPos) && (!filterHidden || _map->isVisible(seekerPosOut,hiderPos,false)) ) {
            //TODO
            hiderPosVectorOut.push_back(hiderPos);
        }
    }
}

void SeekerHS::setPosFromVector(Pos &pos, const vector<double> &posVector) {
    if (posVector.empty()) {
        //AG150210: empty pos
        pos.clear();
    } else if (_params->useContinuousPos) {
        pos.set( /*_map->rowCount() -*/ posVector[ROW_INDEX]/_params->cellSizeM,
                                        /*_map->colCount() -*/ posVector[COL_INDEX]/_params->cellSizeM );
    } else {
        //ag130416: use floor instead of round
        pos.set( /*_map->rowCount() -*/ (int)floor(posVector[ROW_INDEX]/_params->cellSizeM),
                                        /*_map->colCount() -*/ (int)floor(posVector[COL_INDEX]/_params->cellSizeM) );
    }

    /*assert(posVector.size()>=2);

    double x = posVector[0];
    double y = posVector[1];

    double xn = x*cos(_rotateExtOrigin) - y * sin(_rotateExtOrigin) + _xExtOrigin;
    double yn = x*sin(_rotateExtOrigin) + y * cos(_rotateExtOrigin) + _yExtOrigin;*/
}

void SeekerHS::setPosFromVector(IDPos &pos, const vector<double> &posVector) {
    int id = -1;
    if (posVector.empty()) {
        //AG150210: empty pos
        pos.clear();
    } else {
        if (posVector.size()>2)
            id = (int)posVector[2];
        if (_params->useContinuousPos) {
            pos.set( /*_map->rowCount() -*/ posVector[ROW_INDEX]/_params->cellSizeM,
                                            /*_map->colCount() -*/ posVector[COL_INDEX]/_params->cellSizeM, id );
        } else {
            //ag130416: use floor instead of round
            pos.set( /*_map->rowCount() -*/ (int)floor(posVector[ROW_INDEX]/_params->cellSizeM),
                                            /*_map->colCount() -*/ (int)floor(posVector[COL_INDEX]/_params->cellSizeM), id );
        }
    }

    /*assert(posVector.size()>=2);

    double x = posVector[0];
    double y = posVector[1];

    double xn = x*cos(_rotateExtOrigin) - y * sin(_rotateExtOrigin) + _xExtOrigin;
    double yn = x*sin(_rotateExtOrigin) + y * cos(_rotateExtOrigin) + _yExtOrigin;*/
}

void SeekerHS::setVectorFromPos(vector<double> &posVector, const Pos &pos, double orientation, int index, bool setOrientation) {
    /*newSeekerPosVector[0] = newSeekerPos.col;
    newSeekerPosVector[1] = newSeekerPos.row;
    newSeekerPosVector[2] = orient;
    */

    //ag130416: add half cell to go to center of the cell
    int offset = index*(setOrientation ? 3 : 2);

    //AG150210: resize automatically if used to convert 1 pos to 1 vector
    if (index==0) {
        if (setOrientation && posVector.size()<3) {
            posVector.resize(3);
        } else if(!setOrientation && posVector.size()<2) {
            posVector.resize(2);
        }
    } else {
        //ag140423: check that the vector is big enough
        assert(offset + (setOrientation ? 2 : 1) < (int)posVector.size());
    }

    //ag140116: handle real positions
    if (_params->useContinuousPos) {
        //for continuous get the real value of the pos directly
        posVector[offset + ROW_INDEX] = (/*_map->rowCount() -*/ pos.rowDouble()) * _params->cellSizeM; // + _params->cellSizeM/2.0;
        posVector[offset + COL_INDEX] = (/*_map->colCount() -*/ pos.colDouble()) * _params->cellSizeM; // + _params->cellSizeM/2.0;
        if (setOrientation) posVector[offset + 2] = orientation;
    } else {
        //for int pos, get the
        posVector[offset + ROW_INDEX] = (/*_map->rowCount() -*/ pos.row()) * _params->cellSizeM + _params->cellSizeM/2.0;
        posVector[offset + COL_INDEX] = (/*_map->colCount()-*/ pos.col()) * _params->cellSizeM + _params->cellSizeM/2.0;
        if (setOrientation) posVector[offset + 2] = orientation;
    }
}
#endif


//AG150623: disabled since not used
/*void SeekerHS::setExtOriginCoords(double x, double y, double rot) {
    _xExtOrigin = x;
    _yExtOrigin = y;
    _rotateExtOrigin = rot;

    //from external to my/map coordinaates
    _rotateMyOrigin = -rot;
    _xMyOrigin = x*cos(_rotateMyOrigin) - y*sin(_rotateMyOrigin);
    _yMyOrigin = x*sin(_rotateMyOrigin) + y*cos(_rotateMyOrigin);
}*/

void SeekerHS::setMap(GMap* map) {
    _map = map;

    //set use of continuous
    _map->setUsingContActions(_params->useContinuousPos);

    //calc max num actions
    if (_params->maxGameTime==-1) {
        _params->maxGameTime = /*_maxNumActions =*/ _map->colCount()*_map->rowCount(); //2*(_map->colCount()+_map->rowCount());
    }
    _autoPlayer->setMap(map);
}



#ifdef OLD_CODE
void SeekerHS::setWinDistToHider(double winDist) {
    if (winDist<=0) {
        throw CException(_HERE_,"the win distance has to be greater than 0");
    }
    if (_map!=NULL && (_map->width()<=winDist || _map->height()<=winDist)) {
        throw CException(_HERE_,"the distance has to be smaller than the map size");
    }
    _params->winDist = winDist;
}

void SeekerHS::setCellSizeM(double cellSizeM) {
    //assert(cellSizeM>0);
    if (cellSizeM<=0) {
        throw CException(_HERE_,"the cell size has to be more than 0 m");
    }
    _params->cellSizeM = cellSizeM;
}

bool SeekerHS::isObstacle(const vector<double>& posVector) {
    Pos pos;
    setPosFromVector(pos,posVector);
    return _map->isObstacle(pos);
}
#endif

bool SeekerHS::isObstacle(const PosXY &posXY) {
    return _map->isObstacle(posXY.toPos(_params));
}

SeekerHSParams* SeekerHS::getParams() {
    return _params;
}

#ifdef OLD_CODE
vector<double> SeekerHS::getChosenHiderPos() {
    return _chosenHiderPosVec;
}
#endif
PosXY SeekerHS::getChosenHiderPos() {
    return _chosenHiderPosXY;
}

#ifndef DO_NOT_WRITE_BELIEF_IMG
cv::Mat* SeekerHS::getMapBeliefAsImage() {
    //http://wiki.ros.org/cv_bridge/Tutorials/UsingCvBridgeToConvertBetweenROSImagesAndOpenCVImages

    //AG140502: lock for mutex, copy the image
    cv::Mat* beliefImg = NULL;
    _beliefImgMutex.enter();
    if (_beliefImg!=NULL) {
        beliefImg = new cv::Mat(*_beliefImg);
    }
    _beliefImgMutex.exit();

    return beliefImg;
}
#endif

void SeekerHS::startAlgoTimer() {
    if (_algoTimerID<0) {
        _algoTimerID = _timer.startTimer();
    } else {
        _timer.restartTimer(_algoTimerID);
    }
}

void SeekerHS::stopGame(int winState) {
    _gameLog.stopGame(winState);
}


void SeekerHS::addPlayerInfoToHashMap(PlayerInfo *playerInfo) {
    if (playerInfo->id<0)
        throw CException(_HERE_,"the player info ID has to be set in order to add it to the map");
    if (getPlayerInfo(playerInfo->id)!=NULL)
        throw CException(_HERE_,"the player info cannot be added twice to the map");
    //add to map
    _playerInfoMap[playerInfo->id] = playerInfo;
}

const PlayerInfo* SeekerHS::getPlayerInfo(int id) {
    if (id<0 || _playerInfoMap.empty()) {
        return NULL;
    } else {
        map<int,PlayerInfo*>::iterator it = _playerInfoMap.find(id);
        if (it==_playerInfoMap.end()) {
            return NULL;
        } else {
            return it->second;
        }
    }
}
