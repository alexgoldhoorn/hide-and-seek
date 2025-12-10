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
#include "Filter/particlefilterplayer.h"

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

    initClass(params,true, 0);
}

SeekerHS::SeekerHS(SeekerHSParams *params, bool genMap, int seekerID) : _gameLog(this) {
    initClass(params, genMap, seekerID);
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

void SeekerHS::initClass(SeekerHSParams *params, bool genMap, int seekerID) {
    //init randomizer
    hsutils::initRandomizer();

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
        } else if (!hsutils::fileExists(params->pomdpFile.c_str())) {
            throw CException(_HERE_,"could not find the POMDP file");
        }
    }
    if (params->solverType==SeekerHSParams::SOLVER_OFFLINE ||
            params->solverType==SeekerHSParams::SOLVER_LAYERED_COMPARE) {

        //check policy file
        if (params->policyFile.empty()) {
            throw CException(_HERE_,"A policy file is required");
        } else if (!hsutils::fileExists(params->policyFile.c_str())) {
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
            CombinedSeeker* combine = new CombinedSeeker(_params,follower,hspomcp,true);
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
            CombinedSeeker* combine = new CombinedSeeker(_params,follower,hbFollower,true);
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
        case SeekerHSParams::SOLVER_FILTER_PARTICLE_REQ_VISIB:
        case SeekerHSParams::SOLVER_FILTER_PARTICLE_ALWAYS_UPDATES:
        case SeekerHSParams::SOLVER_FILTER_PARTICLE: {
            ParticleFilterPlayer* pfPlayer = new ParticleFilterPlayer(_params);
            _autoPlayer = pfPlayer;
            break;
        }
        case SeekerHSParams::SOLVER_FILTER_PARTICLE_ALWAYS_UPDATES_HB: {
            Follower* follower = new Follower(_params);
            ParticleFilterPlayer* pfPlayer = new ParticleFilterPlayer(_params);
            HighestBeliefFollower* hbFollower = new HighestBeliefFollower(_params, pfPlayer);
            //AG160406: add CombinedSeeker
            CombinedSeeker* combine = new CombinedSeeker(_params,follower,hbFollower,true);
            _autoPlayer = combine;
            break;
        }
        case SeekerHSParams::SOLVER_MULTI_FILTER_ALWAYS_UPDATES_HB_PARTICLE: {
            ParticleFilterPlayer* pfPlayer = new ParticleFilterPlayer(_params);
            MultiSeekerHBExplorer* multiSeekerPFHBExplorer = new MultiSeekerHBExplorer(_params, pfPlayer);
            _autoPlayer = multiSeekerPFHBExplorer;
            break;
        }
        default: //also SOLVER_NOT_SET
            throw CException(_HERE_,"Unknown solver type");
            break;
    }

    assert(_autoPlayer!=NULL);

    if(genMap)
        setMap(_map);

    //AG160614: set id player
    _autoPlayer->playerInfo.seekerID = seekerID;


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

void SeekerHS::setPlayerInfoFromRobotPoses(PlayerInfo *playerInfo, const PosXY &seekerPosXY, const PosXY *hiderPosXY, const std::vector<PosXY>* dynObstPosXYVec) {
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
    //AG160509: read dyn. obstacles
    playerInfo->dynObsVisibleVec.clear();
    if (dynObstPosXYVec!=nullptr) {
        //playerInfo->dynObsVisibleVec.resize(dynObstPosXYVec->size());
        // AG160517: we cannot use the same size, since some might be invalid
        for(size_t i = 0; i<dynObstPosXYVec->size(); i++) {
            Pos pos = (*dynObstPosXYVec)[i].toPos(_params);
            if (_map->isPosInMap(pos) && !_map->isObstacle(pos)) {
                IDPos idPos(pos,i);
                playerInfo->dynObsVisibleVec.push_back(idPos); //[i] = pos;
            } else {
                cout<<"SeekerHS::setPlayerInfoFromRobotPoses: dyn. obst. at "<<pos.toString()<<" is not in the map."<<endl;
            }
        }
    }
    //flag that we have read the new pos
    playerInfo->posRead = seekerPosXY.isSet(); // true; //AG150728: only set when the currentpos was set
    if (!playerInfo->initPosSet) playerInfo->initPosSet = playerInfo->posRead;
}

void SeekerHS::init(const PosXY &seekerPosXY, const PosXY &hiderObsPosXY, const std::vector<PosXY>& dynObstPosXYVec) {
    DEBUG_SHS(cout << "SeekerHS::init..."<<endl;);
    assert(seekerPosXY.id>=0);
    assert(seekerPosXY.isSet());
    //set this player's data
    setPlayerInfoFromRobotPoses(_thisPlayerInfo, seekerPosXY, &hiderObsPosXY, &dynObstPosXYVec);
    setPlayerInfoFromRobotPoses(&_hiderPlayerInfo, hiderObsPosXY, nullptr, nullptr);

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

// TODO: WHY DOES using 1 ROBOT DO pass the dyn. obst (TODO TEST) but a MULTI (init and MultiHBS) does not??

void SeekerHS::initMultiSeeker(const PosXY &seekerPosXY, const PosXY &hiderObsPosXY, const std::vector<PosXY>& dynObstPosXYVec,
                               const std::vector<PosXY> &seekerOtherPosXYVec, const std::vector<PosXY> &hiderObsOtherPosXYVec,
                               const std::vector<std::vector<PosXY>>& otherDynObstPosXYVecVec) {
    DEBUG_SHS(
        cout << "SeekerHS::initMultiSeekers ..."<<endl;
    );

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
    setPlayerInfoFromRobotPoses(_thisPlayerInfo, seekerPosXY, &hiderObsPosXY, &dynObstPosXYVec);
    setPlayerInfoFromRobotPoses(&_hiderPlayerInfo, hiderObsPosXY, nullptr, nullptr);

    //now add players to map
    addPlayerInfoToHashMap(_thisPlayerInfo);
    addPlayerInfoToHashMap(&_hiderPlayerInfo);

    //set other players
    for(size_t i=0; i<seekerOtherPosXYVec.size(); i++) {
        assert(seekerOtherPosXYVec[i].id>=0);
        setPlayerInfoFromRobotPoses(&_otherPlayerInfoVec[i], seekerOtherPosXYVec[i], &hiderObsOtherPosXYVec[i], &otherDynObstPosXYVecVec[i]);
        addPlayerInfoToHashMap(&_otherPlayerInfoVec[i]);
    }

    //AG150622: generate vector with PlayerInfo
    vector<PlayerInfo*> playerVec(_otherPlayerInfoVec.size()+2,NULL);
    //now fill the list

    cout<<"SeekerHS::initMultiSeeker(): thisPlayerID: "<<_thisPlayerInfo->id<<", hider id: "<<_hiderPlayerInfo.id<<"; player vec size: "<<playerVec.size()<<endl;

    assert(_thisPlayerInfo->id>=0 && _thisPlayerInfo->id<(int)playerVec.size());
    playerVec[_thisPlayerInfo->id] = _thisPlayerInfo;
    assert(_hiderPlayerInfo.id>=0 && _hiderPlayerInfo.id<(int)playerVec.size());
    playerVec[_hiderPlayerInfo.id] = &_hiderPlayerInfo;
    //add rest
    for(size_t i = 0; i<_otherPlayerInfoVec.size(); i++) {
        assert(_otherPlayerInfoVec[i].id>=0 && _otherPlayerInfoVec[i].id<(int)playerVec.size());
        assert(playerVec[_otherPlayerInfoVec[i].id]==NULL);
        playerVec[_otherPlayerInfoVec[i].id] = &_otherPlayerInfoVec[i];
    }

    //start timer
    _timerID = _timer.startTimer();

    //init belief
    //AG160316: use id's of stuct instead of 0,1
    _autoPlayer->initBeliefMulti(_map, playerVec, _thisPlayerInfo->id, _hiderPlayerInfo.id);

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

bool SeekerHS::calcNextMultiRobotPoses2(const PosXY& seekerPosXY, const PosXY& hiderObsPosXY, const std::vector<PosXY>& dynObstPosXYVec,
                                        const PosXY* seekerOtherPosXY, const PosXY* hiderObsOtherPosXY, const std::vector<PosXY>* otherDynObstPosXYVec,
                                        PosXY &newSeekerPosMine, PosXY &newSeekerPosOther, int *winState, bool *dontExecuteIteration, bool resetItStartTime)
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
    setPlayerInfoFromRobotPoses(_thisPlayerInfo, seekerPosXY, &hiderObsPosXY, &dynObstPosXYVec);
    if (seekerOtherPosXY!=NULL && seekerOtherPosXY->isSet()) {
        assert(hiderObsOtherPosXY!=NULL);
        setPlayerInfoFromRobotPoses(&_otherPlayerInfoVec[0], *seekerOtherPosXY, hiderObsOtherPosXY, otherDynObstPosXYVec);
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

bool SeekerHS::calcMultiHBs(const PosXY &seekerPosXY, const PosXY &hiderObsPosXY, const std::vector<PosXY>& dynObstPosXYVec,
                            const PosXY *seekerOtherPosXY, const PosXY *hiderObsOtherPosXY, const std::vector<PosXY>* otherDynObstPosXYVec,
                            std::vector<PosXY> &hbVec, int *winState, bool *dontExecuteIteration, bool resetItStartTime)
{
    DEBUG_SHS(cout << "SeekerHS::calcMultiHBs ..."<<endl;);

    assert(_autoPlayer->handles2Obs());
    //now made for this solver only
    assert(_params->solverType==SeekerHSParams::SOLVER_MULTI_HB_EXPL ||
           _params->solverType==SeekerHSParams::SOLVER_MULTI_FILTER_ALWAYS_UPDATES_HB_PARTICLE);
    //assert(_otherPlayerInfoVec.size()==1);

    if (resetItStartTime) {
        //gettimeofday(&_tvLastItStartTime, NULL);
        startAlgoTimer();
    }
    //first prepare next step for all players
    prepareNextStep();

    //update player poses
    setPlayerInfoFromRobotPoses(_thisPlayerInfo, seekerPosXY, &hiderObsPosXY, &dynObstPosXYVec);
    // AG160616: no assert since it might be a different number because of filtering out objects in the map
    //assert(_thisPlayerInfo->dynObsVisibleVec.size()==dynObstPosXYVec.size());

    if (seekerOtherPosXY!=NULL && seekerOtherPosXY->isSet()) {
        assert(hiderObsOtherPosXY!=NULL);
        setPlayerInfoFromRobotPoses(&_otherPlayerInfoVec[0], *seekerOtherPosXY, hiderObsOtherPosXY, otherDynObstPosXYVec);
        // AG160616: no assert since it might be a different number because of filtering out objects in the map
        // assert(_otherPlayerInfoVec[0].dynObsVisibleVec.size()==otherDynObstPosXYVec->size());
    } else if (_params->numPlayersReq>2){ //AG160316: check for number players required
        //AG160315: if not set
        _otherPlayerInfoVec[0].posRead = false;
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
    assert(_params->solverType==SeekerHSParams::SOLVER_MULTI_HB_EXPL ||
           _params->solverType==SeekerHSParams::SOLVER_MULTI_FILTER_ALWAYS_UPDATES_HB_PARTICLE);

    //assume to have calculated the goals
    assert(_thisPlayerInfo->multiHasHBPoses);
    assert(_thisPlayerInfo->multiHBPosVec.size()>=1);
    //assert(_otherPlayerInfoVec.size()==1); //AG160314: disabled constrained because we can use it as single, although a multi method

    //set robot goals in other player
    PlayerInfo* otherSeekerPlayer = &_otherPlayerInfoVec[0];

    if (!otherHBVec.empty()) {
        otherSeekerPlayer->multiHBPosVec.resize(otherHBVec.size());
        for(size_t i=0; i<otherHBVec.size(); i++) {
            otherSeekerPlayer->multiHBPosVec[i] = otherHBVec[i].toBPos(_params);
        }
        otherSeekerPlayer->multiHasHBPoses = true;
    } else if (_params->numPlayersReq>2) { //AG160314: required to have more than 2 seekers
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

    /// TODO: HERE AND IN GETNEXTPOS -> BE SURE TO FILTER FOR OBSTACLE TOO CLOSE!!!
    /// OTHERWISE CHANGE MAP

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
int SeekerHS::getNextPos(const PosXY& seekerPosXY, const PosXY& hiderObsPosXY, const std::vector<PosXY>& dynObstPosXYVec, PosXY& newSeekerPosXY,
                         int *winState, bool *dontExecuteIteration, bool resetItStartTime) {

    DEBUG_SHS(cout  << ">--------------------------------------------"<<endl
                    << "SeekerHS::getNextPose"<<endl;);

    //this should handle solvers for only 1 seeker, and 1 hider
    assert(!_autoPlayer->handles2Obs());
    assert(_otherPlayerInfoVec.empty());

    if (resetItStartTime) {
        //gettimeofday(&_tvLastItStartTime, NULL);
        startAlgoTimer();
    }

    //bool ok = true;

    //get action done
    int actionDone = _autoPlayer->deduceAction();

    //first prepare next step for all players
    prepareNextStep();

    //update player poses
    setPlayerInfoFromRobotPoses(_thisPlayerInfo, seekerPosXY, &hiderObsPosXY, &dynObstPosXYVec);

    //new action
    int newAction = -1;

    if (dontExecuteIteration!=NULL && *dontExecuteIteration==true) {
        //don't execute, keep at same place
        DEBUG_SHS(cout<<"SeekerHS::getNextPos: NOT EXECUTING ITERATION";);
        _thisPlayerInfo->nextPos = _thisPlayerInfo->currentPos;
        //ok = false;
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
    _beliefImg = _autoPlayer->getMapBeliefAsImage(_params->beliefImageCellWidth);

    //update belief img
    if (!_params->beliefImageFile.empty()) {
        string beliefImg = _params->beliefImageFile;
#ifdef USE_QT
        //AG170815: if '#' in name, create a file name based on number
        QString beliefImgQStr = QString::fromStdString(beliefImg);
        if (beliefImgQStr.contains("#")) {
            //replace by action number
            beliefImgQStr = beliefImgQStr.replace("#",QString::number(_thisPlayerInfo->numberActions));
            beliefImg = beliefImgQStr.toStdString();
        }
#endif
        _autoPlayer->storeImage(_beliefImg, beliefImg);
    }
    _beliefImgMutex.exit();
#endif
}

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
