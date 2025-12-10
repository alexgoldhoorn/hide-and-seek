#include "seekerhs.h"

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
#include "Smart/multiseekerhbexplorer.h"

#include "AutoHider/smarthider.h"

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

#include "Utils/hslog.h"
#include "Utils/generic.h"

//iriutils
#include "exceptions.h"

using namespace std;


SeekerHS::SeekerHS(string mapFile, string expName, int solverType, string pomdpFile, string policyFile,
                   string logFile, string timeLogFile, string gamelogFile) {
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

SeekerHS::SeekerHS(SeekerHSParams *params, bool genMap) {
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
    /*DEBUG_DELETE(cout<<"delete seeker player: "<<flush;);
    delete _seekerPlayer;
    DEBUG_DELETE(cout<<"ok"<<endl;);*/
    DEBUG_DELETE(cout<<"close game log: "<<flush;);
    _gamelog->close();
    DEBUG_DELETE(cout<<"ok"<<endl;);
    DEBUG_DELETE(cout<<"delete game log: "<<flush;);
    delete _gamelog;
    DEBUG_DELETE(cout<<"ok"<<endl;);
    DEBUG_DELETE(cout<<"delete next seeker pos: "<<flush;);
    delete _nextSeekerPos;
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

    //next pos
    _nextSeekerPos = new Pos();

    //max num actions are set in init
    /*_maxNumActions = */
    _numActions = 0;

    //_seekerPlayer = new Player();

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

    //open game log file
    openGameLogFile(params->gameLogFile);

    //name exp
    _expName = params->expName;

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
        case SeekerHSParams::SOLVER_MULTI_HB_EXPL: {
            pomcp::HSPOMCP* hspomcp = new pomcp::HSPOMCP(_params);
            MultiSeekerHBExplorer* multiSeekerHBExplorer = new MultiSeekerHBExplorer(_params, hspomcp);
            _autoPlayer = multiSeekerHBExplorer;
            break;
        }
        default: //also SOLVER_NOT_SET
            throw CException(_HERE_,"Unknown solver type");
            break;
    }

    if(genMap)
        setMap(_map);

    //AG150127: if there are two seekers the solver should support that
    if (_params->gameHas2Seekers() && !_autoPlayer->handles2Obs()) {
        throw CException(_HERE_,"Two seekers are participating, but this is not handled by the solver!");
    }

    assert(_autoPlayer!=NULL);
    cout << "AutoPlayer: "<<_autoPlayer->getName()<<endl;
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

int SeekerHS::getWinDistToHider() {
    return _params->winDist;
}

double SeekerHS::getCellSizeM() {
    return _params->cellSizeM;
}

unsigned int SeekerHS::getActionsDone() {
    return _numActions;
}


void SeekerHS::init(vector<double> seekerPosVector, vector<double> hiderPosVector) {
    throw CException(_HERE_,"SeekerHS::init: use initMultipleObs!");
}

void SeekerHS::filterMultipleObs(const std::vector<double> &seekerPosVectorIn, const std::vector<std::vector<double> > &hiderPosVectorsIn,
                                 std::vector<double> &seekerPosVectorOut, std::vector<double> &hiderPosVectorOut) {

    Pos seekerPosIn, seekerPosOut;
    IDPos hiderPosOut;
    vector<IDPos> hiderIDPosVector;
    getPosFromVectors(seekerPosVectorIn, hiderPosVectorsIn, seekerPosIn, hiderIDPosVector, _params->simulateNotVisible);

    bool dontExec = false;
    //check and filter the poses
    _autoPlayer->checkAndFilterPoses(seekerPosIn, hiderIDPosVector, seekerPosOut, hiderPosOut, dontExec);

    if (dontExec)
        DEBUG_SHS(cout<<"SeekerHS.getNextMultiplePoses: WARNING gives \"don't execute\" message'";);

    //return seeker pos without orientation
    setVectorFromPos(seekerPosVectorOut, seekerPosOut, 0,0,false);
    setVectorFromPos(hiderPosVectorOut, hiderPosOut, 0,0,false);

    _chosenHiderPosVec = hiderPosVectorOut;
}

void SeekerHS::initMultiSeekerObs(const std::vector<double> &seekerPosVectorMine, const std::vector<double> &hiderPosVectorMine,
                                  const std::vector<double> &seekerPosVectorOther, const std::vector<double> &hiderPosVectorOther) {

    DEBUG_SHS(cout << "SeekerHS.initMultiSeekerObs ..."<<endl;);

    assert(_autoPlayer->handles2Obs());
    assert(seekerPosVectorMine.size()>=2);


    //convert to pos
    Pos seekerPosMineIn, seekerPosOtherIn, hiderPosOtherIn;
    IDPos hiderPosMineIn;

    //set poses
    setPosFromVector(seekerPosMineIn, seekerPosVectorMine);    
    setPosFromVector(hiderPosMineIn, hiderPosVectorMine);
    setPosFromVector(seekerPosOtherIn, seekerPosVectorOther);
    setPosFromVector(hiderPosOtherIn, hiderPosVectorOther);

    _numActions = 0;

    DEBUG_SHS(cout << "Maximum number of actions: "<<_params->maxNumActions <<endl;);
    DEBUG_SHS(cout << "Maximum number of time: "<<_params->maxGameTime <<" s"<<endl;);

    //start timer
    _timerID = _timer.startTimer();

    //init player
    bool oppVisib = hiderPosMineIn.isSet(); //(could contian an exist

    if (seekerPosVectorOther.size()>0) {
        _autoPlayer->initBelief2(_map, seekerPosMineIn, hiderPosMineIn, oppVisib, seekerPosOtherIn, hiderPosOtherIn, _params->multiSeekerOwnObsChooseProb);
    } else {
        //single init
        _autoPlayer->initBelief(_map, seekerPosMineIn, hiderPosMineIn, oppVisib);
    }

    //init seeker and hider to follow steps
    /*_seekerPlayer->setCurPos(seekerPosMineIn);
    if (hiderPosMineIn.isSet()) {
        _seekerPlayer->setPlayer2Pos(chosenHiderPos);
    } else {
        _seekerPlayer->setPlayer2Pos(-1,-1);
    }*/

    //next seeker pos is the same pos
    *_nextSeekerPos = seekerPosMineIn;

    //log
    logLineInit2(seekerPosVectorMine, hiderPosVectorMine, seekerPosVectorOther, hiderPosVectorOther, seekerPosMineIn, hiderPosMineIn, oppVisib,
                 seekerPosOtherIn, hiderPosOtherIn);

    DEBUG_SHS(cout << "SeekerHS.initMultiSeekerObs: done"<<endl;);
}

void SeekerHS::initMultipleObs(vector<double> seekerPosVector, vector<vector<double>> hiderPosVector) {
    DEBUG_SHS(cout << "SeekerHS.initMultipleObs ..."<<endl;);

    //convert to pos
    Pos seekerPos;
    vector<IDPos> hiderIDPosVector;
    getPosFromVectors(seekerPosVector, hiderPosVector, seekerPos, hiderIDPosVector, _params->simulateNotVisible);

    _numActions = 0;

    DEBUG_SHS(cout << "Maximum number of actions: "<<_params->maxNumActions <<endl;);
    DEBUG_SHS(cout << "Maximum number of time: "<<_params->maxGameTime <<" s"<<endl;);

    //start timer
    _timerID = _timer.startTimer();

    //init player
    _autoPlayer->initBeliefWithFilter(_map, seekerPos, hiderIDPosVector);

    //get chosen hider pos
    IDPos chosenHiderPos(_autoPlayer->getLastHiderPos());

    //set chosen hider vector (just to have it in vector format and send to ROS)
    if (!chosenHiderPos.isSet()) {
        _chosenHiderPosVec.clear();
    } else {
        //_chosenHiderPosVec = hiderPosVector[chosenHiderPosIndex];
        _chosenHiderPosVec.resize(2);
        setVectorFromPos(_chosenHiderPosVec, chosenHiderPos, 0, 0, false);
    }

    //DEBUG_SHS(cout<<"Init belief done"<<endl;);

    //init seeker and hider to follow steps
    /*_seekerPlayer->setCurPos(seekerPos);
    if (chosenHiderPos.isSet()) {
        _seekerPlayer->setPlayer2Pos(chosenHiderPos);
    } else {
        _seekerPlayer->setPlayer2Pos(-1,-1);
    }*/

    //next seeker pos is the same pos
    *_nextSeekerPos = seekerPos;

    //log
    logLineInit(seekerPosVector, _chosenHiderPosVec, seekerPos, chosenHiderPos, chosenHiderPos.isSet());

    DEBUG_SHS(cout << "SeekerHS.initMultipleObs: done"<<endl;);
}

bool SeekerHS::hasTimePassedForNextIteration() {    
    if (_algoTimerID<0) {
        return true;
    } else {
        return _timer.getTime_ms(_algoTimerID) >= (long)_params->minTimeBetweenIterations_ms;
    }
}

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
            _chosenHiderPosVec.clear();
        } else {
            _chosenHiderPosVec.resize(2);
            setVectorFromPos(_chosenHiderPosVec, hiderPosMineIn, 0, 0, false);
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


    //cout <<"GETORIENTATION - ";

    if (_winState==HSGlobalData::GAME_STATE_RUNNING) {
        //AG140403: set pos
        //_lastPos = seekerPos;
        //cout <<"game-state-running - ";

        //if not possible, check how to fix
        _autoPlayer->setMinDistanceToObstacle(*_nextSeekerPos);

        //double orient = 0;        
        if (lastHiderPos.isSet()) { //TODO: CHECK !!!            
            //face to hider
            orient1st = getOrientation(*_nextSeekerPos, lastHiderPos);
            //DEBUG_SHS(cout<<"Orientation - "<<(180*orient/M_PI)<<"º "<<_nextSeekerPos->toString()<<" to " <<" hpos="<<hiderPos.toString(););
            //cout << "next seeker:"<<_nextSeekerPos->toString()<<" to last hider:"<<lastHiderPos.toString()<<" -> "<<orient1st*180/M_PI<<"º"<<endl;
        } else {
            //face in direction of going
            orient1st = getOrientation(lastSeekerPos, *_nextSeekerPos);
            //cout << "next seeker:"<<_nextSeekerPos->toString()<<" FROM last seeker:"<<lastSeekerPos.toString()<<" -> "<<orient1st*180/M_PI<<"º"<<endl;
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
                //cout <<"WARNING - NOT turned"<<endl;
                orient1st = _lastOrient; //don't turn
            }
        } else {
            //face in direction of going
            //if (orientDiff)
            orient1st = getOrientation(lastSeekerPos, *_nextSeekerPos); //TODO: should be same orientatin, no change.
        }

        //cout <<"game NOT running -> "<<orient1st*180/M_PI<<"º"<<endl;
    }


    //set output vector
    newSeekerPosVector.resize(3);
    setVectorFromPos(newSeekerPosVector, *_nextSeekerPos, orient1st); //AG140328: changed *_nextSeekerPos -> seekerPos

    _lastOrient = orient1st;


    //cout<<"ORIENTING: "<<orient1st*180/M_PI<<"º, new seeker pos vector: ["<<newSeekerPosVector[0]<<","<<newSeekerPosVector[1]<<","<<newSeekerPosVector[2]<<"]"<<endl;


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
        _chosenHiderPosVec.clear();
    } else {
        _chosenHiderPosVec.resize(2);
        setVectorFromPos(_chosenHiderPosVec, hiderPos, 0, 0, false);
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

    logLine(seekerPosVector,_chosenHiderPosVec,seekerPos,hiderPos,hiderVisib,newSeekerPosVector,*_nextSeekerPos,winState);

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

int SeekerHS::getNextPoseForMultipleObs(vector<double> seekerPosVector, vector< vector<double> > hiderPosVector,
                                        vector<double>& newSeekerPosVector, int *winState, bool resetItStartTime) {

    throw CException(_HERE_,"SeekerHS::getNextMultiplePoses: use getNextMultiplePosesForMultipleObs");
    /*if (resetItStartTime) {
        gettimeofday(&_tvLastItStartTime, NULL);
    }

    checkAndFixSeekerPosVector(seekerPosVector);

    vector<double> chosenHiderPosVec = chooseHiderPos(seekerPosVector, hiderPosVector, !_params->allowInconsistObs);

    return getNextPose(seekerPosVector, chosenHiderPosVec, newSeekerPosVector, winState, false);*/
}



/*void SeekerHS::setMinDistanceToObstacle(Pos &pos) {
    if (_params->useContinuousPos && _params->minDistToObstacle>0) {
        //now check if obstacle within min dist
        //NOTE: we assume the minDistToObstacle <=1

        bool obstLeft = isThereAnObstacleAt(pos,0,-_params->minDistToObstacle);
        bool obstRight = isThereAnObstacleAt(pos,0,_params->minDistToObstacle);
        bool obstUp = isThereAnObstacleAt(pos,-_params->minDistToObstacle,0);
        bool obstDown = isThereAnObstacleAt(pos,_params->minDistToObstacle,0);

        double newCol, newRow;

        //obst. at left/right
        if (obstLeft && obstRight) {
            //put in middle
            newCol = pos.col() + 0.5;
            DEBUG_SHS(cout<<"setMinDistanceToObstacle: obst left & right");
        } else if (obstLeft) {
            newCol = pos.col() + _params->minDistToObstacle;
            DEBUG_SHS(cout<<"setMinDistanceToObstacle: obst left");
        } else if (obstRight) {
            newCol = pos.col() + 1 - _params->minDistToObstacle;
            DEBUG_SHS(cout<<"setMinDistanceToObstacle: obst right");
        } else {
            newCol = pos.colDouble();
        }

        //obst up/down
        if (obstUp && obstDown) {
            //put in middle
            newRow = pos.row() + 0.5;
            DEBUG_SHS(cout<<"setMinDistanceToObstacle: obst up & down");
        } else if (obstUp) {
            newRow = pos.row() + _params->minDistToObstacle;
            DEBUG_SHS(cout<<"setMinDistanceToObstacle: obst up");
        } else if (obstDown) {
            newRow = pos.row() + 1 - _params->minDistToObstacle;
            DEBUG_SHS(cout<<"setMinDistanceToObstacle: obst down");
        } else {
            newRow = pos.rowDouble();
        }

        if (!obstLeft && !obstRight && !obstUp && !obstDown) {
            //handle diagonal obstaclse
            //recalculate distance horizontally and vertically if diagonal distance is minDistToObst
            double minObstDistForDiag = _params->minDistToObstacle / M_SQRT2; // GMap::SQRT_2;

            if (isThereAnObstacleAt(pos,-minObstDistForDiag,-minObstDistForDiag)) {
                //obstacle at left top
                newRow = pos.row() + minObstDistForDiag;
                newCol = pos.col() + minObstDistForDiag;
                DEBUG_SHS(cout<<"setMinDistanceToObstacle: obst left-up");
            } else if (isThereAnObstacleAt(pos,-minObstDistForDiag,minObstDistForDiag)) {
                //obstacle at right top
                newRow = pos.row() + minObstDistForDiag;
                newCol = pos.col() + 1 - minObstDistForDiag;
                DEBUG_SHS(cout<<"setMinDistanceToObstacle: obst right-up");
            } else if (isThereAnObstacleAt(pos,minObstDistForDiag,-minObstDistForDiag)) {
                //obstacle at left bottom
                newRow = pos.row() + 1 - minObstDistForDiag;
                newCol = pos.col() + minObstDistForDiag;
                DEBUG_SHS(cout<<"setMinDistanceToObstacle: obst left-down");
            } else if (isThereAnObstacleAt(pos,minObstDistForDiag,minObstDistForDiag)) {
                //obstacle at right bottom
                newRow = pos.row() + 1 - minObstDistForDiag;
                newCol = pos.col() + 1 - minObstDistForDiag;
                DEBUG_SHS(cout<<"setMinDistanceToObstacle: obst right-down");
            }
        }

        pos.set(newRow,newCol);
    }
}

bool SeekerHS::isThereAnObstacleAt(Pos p, double dr, double dc) {
    p.add(dr,dc);
    if (_map->isPosInMap(p)) {
        return _map->isObstacle(p);
    } else {
        return false;
    }
}
*/


/*void SeekerHS::checkAndFixSeekerPosVector(vector<double> &seekerPosVector) {
    if (!checkValidNextSeekerPos(seekerPosVector, !_params->allowInconsistObs)) {
        //throw CException(_HERE_, "the seeker position is not valid");
        cout << "WARNING: not valid seeker position: "<<seekerPosVector[0]<<","<<seekerPosVector[1]<<", finding closest"<<endl;

        //convert to pos
        Pos seekerPos;
        setPosFromVector(seekerPos,seekerPosVector);
	//AG140423: if continuous space and the cell is int, set it to center of cell
	if (_params->useContinuousPos && !seekerPos.hasDouble()) 
		seekerPos.add(0.5,0.5); 
        cout << "FIX: closest seeker pos from: "<<seekerPos.toString();
        //now get closest
        seekerPos = _autoPlayer->getClosestSeekerObs(seekerPos);
        cout<<" is: "<<seekerPos.toString()<<endl;
        //convert to vector
        setVectorFromPos(seekerPosVector, seekerPos, 0);
    }
}*/


/*
vector<double> SeekerHS::chooseHiderPos(vector<double> seekerPosVector, vector< vector<double> > hiderPosVector, bool checkPrev) {
    //There are several variants of scoring to choose the hider
    switch(_params->filterScoreType) {
        case HSGlobalData::FILTER_SCORE_OLD: {

            DEBUG_SHS(cout << "SeekerHS::chooseHiderPos v1: checking "<<hiderPosVector.size()<< " hider poses (seeker = ("
                      <<seekerPosVector[0]<<","<<seekerPosVector[1]<<")):"<<endl;);

            //vector<double> chosenHiderPosVec;

            double maxS = 0;
            vector<size_t> maxScoreObsVec;

            //TODO should also score hidden if there are several (or 1) observations given

            //AG140310: always score the hidden observation
            //note2: always added as last
            vector<double> hidden;
            hiderPosVector.push_back(hidden);
            //AG140310: indicates whether observation of a visible point has been found
            bool visibObsFound = false;

            //validate the hider pos and choose 'best' option
            for(size_t i = 0; i < hiderPosVector.size(); i++) {
                //get score
                double score = 0;

                if (hiderPosVector[i].size()>0) {
                    if (hiderPosVector[i][2]==0) {
                        //AG140411: an id of 0 means that it comes form the tag detector, and should be much more robust
                        score = _params->filterTagBaseScore;
                    } else {
                        score = 1.0; //ag140122: hider detected, but we don't care for the score of the measurement itself   hiderPosVector[i][2];
                    }
                } else {
                    score = _params->filterHiddenBaseScore; // hidden position, if it is added artificially
                }

                DEBUG_SHS(
                    cout << " * hider = (";
                    if (hiderPosVector[i].size()==0) {
                        cout<<"hidden";
                    } else {
                        if (hiderPosVector[i][2]==0) {
                            cout <<"tag; ";
                        } else {
                            cout <<"laser; ";
                        }
                        cout<<hiderPosVector[i][0]<<","<<hiderPosVector[i][1];
                    }
                    cout <<"), base score="<<score<<" ["<<flush;
                );

                //check
                if (!checkValidNextHiderPos(hiderPosVector[i],seekerPosVector, checkPrev)) {
                    DEBUG_SHS(cout<<"]: not valid!"<<endl;);
                    continue; //skip since not valid
                }

                DEBUG_SHS(cout<<"]";);

                Pos seekerPos, hiderPos;
                bool visib;
                getPosFromVectors(seekerPosVector, hiderPosVector[i], seekerPos, hiderPos, visib);

                if (_params->simulateNotVisible) {
                    if (visib && !_map->isVisible(seekerPos,hiderPos)) {
                        DEBUG_SHS(cout<<" simulating NOT visible, ignoring"<<endl;);
                        continue;
                    }
                }

                //ag140424: always store obs score
                double obsScore = 0;
                if (_autoPlayer->canScoreObservations()) {
                    //AG140117: check if we can score observations
                    obsScore = _autoPlayer->scoreObservation(seekerPos, hiderPos);

                    DEBUG_SHS(cout << ", obs score="<<obsScore<<flush;);

                    if (obsScore>0) {
                        score *= obsScore;
                        if (visib) visibObsFound = true;
                    }
                } else {
                    DEBUG_SHS(cout<<", (no obs score)"<<flush;);
                }
                //AG140122: use distance to prev as score
                //(AG140411: added check of hidden previous pos)
                if (visib && _numActions>0 && _seekerPlayer->getOppPos().isSet()) {
                    //the closer, the higher a score, BUT we need to prevent the score from
                    //growing exponantially, therefore round them (now 0.5)
                    //AG TODO: maybe an exponential function instead of continuous
                    double d = _map->distanceEuc(_seekerPlayer->getOppPos(),hiderPos);
                    assert(d>=0);

                    double distScore = 0;

                    //if (d>=0) {
                        d = round(d*DIST_SCORE_MULT);
                        if (d==0) {
                            distScore = MAX_DIST_SCORE;
                        } else {
                            distScore = MAX_DIST_SCORE/d;
                        }
                    //}
                    score += distScore;
                    DEBUG_SHS(cout<<", dist score="<<distScore<<flush;);
                    //cout<<"between:"<<_seekerPlayer->getOppPos().toString()<<"-"<<hiderPos.toString()<<" d="<<d<<endl;
                }
                DEBUG_SHS(cout<<", [";);

                if (checkPrev || checkValidNextHiderPos(hiderPosVector[i],seekerPosVector, true)) {
                    score *= 2; //increase score because it is consistent with prev score
                    DEBUG_SHS(cout<<"] valid score (x2)"<<flush;);
                } else
                    DEBUG_SHS(cout<<"]";);

                DEBUG_SHS(cout << ": total score="<<score;);

                if (score >= maxS) {
                    if (visib || !visib && !visibObsFound) {
                        //AG140310: max score AND either visible OR NOT visible AND no other visible observations found
                        if (score > maxS) {
                            maxScoreObsVec.clear();
                            maxS = score;
                        }

                        maxScoreObsVec.push_back(i);
                    } else {
                        DEBUG_SHS(cout << " (not used since other visible obs found)";);
                    }
                }
                DEBUG_SHS(cout << endl;);
            }
            if (maxScoreObsVec.size()==1) {
                _chosenHiderPosVec = hiderPosVector[maxScoreObsVec[0]];
            } else if (maxScoreObsVec.size()>1) {
                //randomly choose a best one
                int i = random(maxScoreObsVec.size()-1) ;
                _chosenHiderPosVec = hiderPosVector[maxScoreObsVec[i]];
            } // else: empty


            break;
        }

        case HSGlobalData::FILTER_SCORE_NEW1:
        case HSGlobalData::FILTER_SCORE_NEW2_WEIGHTED:
        case HSGlobalData::FILTER_SCORE_NEW2_PREF_TAG:
        {
            DEBUG_SHS(cout << "SeekerHS::chooseHiderPos(v2): checking "<<hiderPosVector.size()<< " hider poses (seeker = ("
                      <<seekerPosVector[0]<<","<<seekerPosVector[1]<<")):"<<endl;);

            //max score
            double maxS = 0;
            //list of observations with max score
            vector<size_t> maxScoreObsVec;

            //AG140310: always score the hidden observation
            //note2: always added as last
            vector<double> hidden;
            hiderPosVector.push_back(hidden);
            //AG140310: indicates whether observation of a visible point has been found
            bool visibObsFound = false;
            //last action
            int lastA = -2;

            //AG140508: update score
            if (_params->hiderStepDistance > _params->filterDistScoreMaxDist)
                cout << "WARNING: hiderStepDistance ("<<_params->hiderStepDistance<<") > distScoreMaxDist ("<<_params->filterDistScoreMaxDist<<")"<<endl;

            //validate the hider pos and choose 'best' option
            for(size_t i = 0; i < hiderPosVector.size(); i++) {
                //get score
                double score = 0;
                double sbase = 0;

                if (hiderPosVector[i].size()>0) {
                    if (hiderPosVector[i][2]==0) {
                        //AG140411: an id of 0 means that it comes form the tag detector, and should be much more robust
                        sbase = _params->filterTagBaseScore;
                    } else {
                        sbase = _params->filterLaserBaseScore; //ag140122: hider detected, but we don't care for the score of the measurement itself   hiderPosVector[i][2];
                    }
                } else {
                    sbase = _params->filterHiddenBaseScore; // hidden position, if it is added artificially
                }

                DEBUG_SHS(
                    cout << " * hider = (";
                    if (hiderPosVector[i].size()==0) {
                        cout<<"hidden";
                    } else {
                        if (hiderPosVector[i][2]==0) {
                            cout <<"tag; ";
                        } else {
                            cout <<"laser; ";
                        }
                        cout<<hiderPosVector[i][0]<<","<<hiderPosVector[i][1];
                    }
                    if (hiderPosVector[i].size()>2)
                        cout << "; id="<<hiderPosVector[i][2];
                    cout <<"), base score="<<sbase<<" ["<<flush;
                );

                //check
                if (!checkValidNextHiderPos(hiderPosVector[i],seekerPosVector, checkPrev)) {
                    DEBUG_SHS(cout<<"]: not valid!"<<endl;);
                    continue; //skip since not valid
                }

                DEBUG_SHS(cout<<"]";);

                if (_params->filterScoreType==HSGlobalData::FILTER_SCORE_NEW2_PREF_TAG && hiderPosVector[i].size()>2 && hiderPosVector[i][2]==0) {
                    //AG140507: if we prefer tag, just stop and choose this
                    DEBUG_SHS(cout<<"found a tag!"<<endl;);
                    maxScoreObsVec.clear();
                    maxScoreObsVec.push_back(i);;
                    break; //stop since we found the tage
                }

                Pos seekerPos, hiderPos;
                bool visib;
                getPosFromVectors(seekerPosVector, hiderPosVector[i], seekerPos, hiderPos, visib);

                if (_params->simulateNotVisible) {
                    if (visib && !_map->isVisible(seekerPos,hiderPos)) {
                        DEBUG_SHS(cout<<"] simulating NOT visible, ignoring"<<endl;);
                        continue;
                    }
                }

                //AG140505: added deduce action
                if (lastA==-2) {
                    if (_params->useDeducedAction) {
                        lastA = _autoPlayer->deduceAction(_lastPos,seekerPos);
                    } else {
                        lastA = -1;
                    }
                    cout <<", ded.act=";
                    if (lastA<0) cout <<"none"; else cout << ACTION_COUT(lastA);
                }

                double obsScore = 0;
                if (_autoPlayer->canScoreObservations()) {
                    //AG140117: check if we can score observations
                    obsScore = _autoPlayer->scoreObservation(seekerPos, hiderPos, lastA);

                    DEBUG_SHS(cout << ", obs score="<<obsScore<<flush;);

                    if (obsScore>0) {
                        score += obsScore;
                        if (visib) visibObsFound = true;
                    }
                } else {
                    DEBUG_SHS(cout<<", (no obs score)"<<flush;);
                }
                //AG140122: use distance to prev as score
                //(AG140411: added check of hidden previous pos)
                if (visib && _numActions>0 && _seekerPlayer->getOppPos().isSet()) {
                    //the closer, the higher a score, BUT we need to prevent the score from
                    //growing exponantially, therefore round them (now 0.5)
                    //AG TODO: maybe an exponential function instead of continuous
                    double d = _map->distanceEuc(_seekerPlayer->getOppPos(),hiderPos);
                    assert(d>=0);                    

                    DEBUG_SHS(cout<<", dist score="<<flush;);

                    if (d < _params->filterDistScoreMaxDist) {
                        double distScore = 0;

                        //AG140508: update score
                        if (d <= _params->hiderStepDistance) {//+_params->contNextHiderStateStdDev
                            distScore = 1;
                        } else {
                            distScore = 1 - (d - _params->hiderStepDistance) / (_params->filterDistScoreMaxDist - _params->hiderStepDistance);
                            if (distScore<0) cout <<"distscore="<<distScore<<";d="<<d<<";hiderstepd="<<_params->hiderStepDistance<<";filterdistscmaxd="<<_params->filterDistScoreMaxDist<<endl;
                            assert(distScore>=0);
                        }

                        //AG140425: weight based on num sim, to give more priority
                        if (_params->filterScoreType == HSGlobalData::FILTER_SCORE_NEW1) {
                            score += distScore;
                        } else {
                            score += distScore / _params->numSim;
                        }

                        DEBUG_SHS(cout<<distScore<<flush;);
                    } else {
                        DEBUG_SHS(cout<<", 0 (too far)"<<flush;);
                    }


                }  else if (obsScore==0) { //_numActions==0 || !_autoPlayer->canScoreObservations()
                    score = (_numActions==0 ? 1 : _params->filterMinScore);

                    //AG140416: first action, put a score in order to be able to compare with base score (otherwise all 0)
                    DEBUG_SHS(cout<<(_numActions==0?", 1st action score=":", cannot score obs->score=")<<score<<flush;);
                }

                DEBUG_SHS(cout<<", [";);

                if (checkPrev || checkValidNextHiderPos(hiderPosVector[i],seekerPosVector, true)) {
                    score *= 2; //increase score because it is consistent with prev score
                    DEBUG_SHS(cout<<"] valid score (x2)"<<flush;);
                } else
                    DEBUG_SHS(cout<<"]";);

                score *= sbase;

                DEBUG_SHS(cout << ": total score="<<score;);

                if (score >= maxS) {
                    //if (visib || !visib && !visibObsFound) {
                        //AG140310: max score AND either visible OR NOT visible AND no other visible observations found
                        if (score > maxS) {
                            maxScoreObsVec.clear();
                            maxS = score;
                        }

                        maxScoreObsVec.push_back(i);
                }
                DEBUG_SHS(cout << endl;);
            } // for all poses

            if (maxScoreObsVec.size()==1) {
                _chosenHiderPosVec = hiderPosVector[maxScoreObsVec[0]];
            } else if (maxScoreObsVec.size()>1) {
                //randomly choose a best one
                int i = random(maxScoreObsVec.size()-1) ;
                _chosenHiderPosVec = hiderPosVector[maxScoreObsVec[i]];
            } // else: empty


            break;
        }

        case HSGlobalData::FILTER_SCORE_USE_TAG_ONLY:
        case HSGlobalData::FILTER_SCORE_USE_ID: {
            DEBUG_SHS(
                cout << "SeekerHS::chooseHiderPos(use track id): checking "<<hiderPosVector.size()<< " hider poses (seeker = ("
                  <<seekerPosVector[0]<<","<<seekerPosVector[1]<<")), ";
                if (_params->filterScoreType==HSGlobalData::FILTER_SCORE_USE_TAG_ONLY)
                    cout << "searching tag:"<<flush;
                else
                    cout << "searching ID "<<_params->filterFollowID<<": "<<flush;
            );

            //follower ID
            int followerID = 0;
            if (_params->filterScoreType==HSGlobalData::FILTER_SCORE_USE_ID) {
                followerID = _params->filterFollowID;
            } //else followerID = 0 -> tag ID


            //first set to not visible
            _chosenHiderPosVec.clear();

            bool found = false;

            //search for the id, which should be on the 3rd position of the vector
            //NOTE: it assumes that the ID is unique
            for(size_t i = 0; i < hiderPosVector.size(); i++) {
                if (hiderPosVector[i].size()>2 && hiderPosVector[i][2]==followerID) {
                    //found the ID
                    found = true;

                    if (_params->simulateNotVisible) {
                        //we need to sim not visib, so check if it should be visible
                        Pos seekerPos, hiderPos;
                        bool visib;
                        getPosFromVectors(seekerPosVector, hiderPosVector[i], seekerPos, hiderPos, visib);

                        if (visib && _map->isPosInMap(hiderPos) && !_map->isObstacle(hiderPos) && _map->isVisible(seekerPos,hiderPos)) {
                            //DEBUG_SHS(cout<<"] simulating NOT visible, ignoring"<<endl;);
                            _chosenHiderPosVec = hiderPosVector[i];
                        } //else: simulate not visible
                    } else {
                        _chosenHiderPosVec = hiderPosVector[i];
                    }

                    break;
                }
            }

            //check
            if (!checkValidNextHiderPos(_chosenHiderPosVec,seekerPosVector, checkPrev)) {
                DEBUG_SHS(cout<<"found, but not valid"<<endl;);
                _chosenHiderPosVec.clear();
            } else {
                DEBUG_SHS(
                    if (found) {
                        cout << "FOUND"<<flush;
                        if (_chosenHiderPosVec.size()==0) {
                            cout << ", but simulating not visible";
                        }
                    } else {
                        cout << "NOT FOUND";
                    }
                    cout << endl;
                );
            }
            break;
        }

        default: {
            throw CException(_HERE_,"SeekerHS::chooseHiderPos: unknown filter score type");
            break;
        }

    }//switch score type

    DEBUG_SHS(
       cout << " -> CHOSEN HIDER POS.: ("<<flush;
       if (_chosenHiderPosVec.size()>0)
           cout <<_chosenHiderPosVec[0]<<","<<_chosenHiderPosVec[1]<<")"<<endl;
       else
           cout << "hidden)"<<endl;
    );

    return _chosenHiderPosVec;
}
*/



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

void SeekerHS::setExtOriginCoords(double x, double y, double rot) {
    _xExtOrigin = x;
    _yExtOrigin = y;
    _rotateExtOrigin = rot;

    //from external to my/map coordinaates
    _rotateMyOrigin = -rot;
    _xMyOrigin = x*cos(_rotateMyOrigin) - y*sin(_rotateMyOrigin);
    _yMyOrigin = x*sin(_rotateMyOrigin) + y*cos(_rotateMyOrigin);
}

void SeekerHS::setMap(GMap* map) {
    _map = map;

    //set use of continuous
    _map->setUsingContActions(_params->useContinuousPos);

    //set on player
    /*if (_seekerPlayer != NULL) {
        _seekerPlayer->setMap(map);
    }*/
    //calc max num actions
    if (_params->maxGameTime==-1) {
        _params->maxGameTime = /*_maxNumActions =*/ _map->colCount()*_map->rowCount(); //2*(_map->colCount()+_map->rowCount());
    }
    _autoPlayer->setMap(map);
}


void SeekerHS::openGameLogFile(string gamelogFile) {
    //check if log file already exists
    ifstream logFile;
    logFile.open(gamelogFile.c_str(), ifstream::in);
    bool exists = logFile.good();

    if (exists) {
        logFile.close();
    }

    //open log
    _gamelog = new HSLog(gamelogFile.c_str(),true);

    if (!exists) {
        //add header to log
        stringstream header;

        if (_params->gameHas2Seekers()) { //AG150209
            header << "exp_name,seeker_x,seeker_y,seeker_orient,seeker_row,seeker_col,hider_visible,hider_x,hider_y,"
                               << "hider_row,hider_col,hider_id,seeker2_x,seeker2_y,seeker2_row,seeker2_col,hidero2_x,hidero2_y,"
                               << "hidero2_row,hidero2_col,distance(s-h),distance(s-h2),distance(s2-h),distance(s2-h2),distance(s-s2),"
                               << "win_state,new_seeker1_froms1_x,new_seeker1_froms1_y,new_seeker1_froms1_row,new_seeker1_froms1_col,"
                               << "new_seeker2_froms1_x,new_seeker2_froms1_y,new_seeker2_froms1_row,new_seeker2_froms1_col,"
                               << "new_seeker1_froms1_belief,new_seeker2_froms1_belief,"
                               << "new_seeker1_froms2_x,new_seeker1_froms2_y,new_seeker1_froms2_row,new_seeker1_froms2_col,"
                               << "new_seeker2_froms2_x,new_seeker2_froms2_y,new_seeker2_froms2_row,new_seeker2_froms2_col,"
                               << "new_seeker1_froms2_belief,new_seeker2_froms2_belief,"
                               << "new_seeker1_goal_x,new_seeker1_goal_y,new_seeker1_goal_orient,new_seeker1_goal_row,new_seeker1_goal_col";
        } else {
            header << "exp_name,seeker_x,seeker_y,seeker_orient,seeker_row,seeker_col,hider_visible,hider_x,hider_y,"
                               << "hider_row,hider_col,hider_id,distance,new_seeker_x,new_seeker_y,new_seeker_orient,new_seeker_row,new_seeker_col,win_state";
        }

        _gamelog->printLine(header.str());
    }
}


void SeekerHS::stopGame(int winState) {
    //print end of log
    _gamelog->print(_expName);

    if (_params->gameHas2Seekers()) {
        for(int i=0;i<24;i++)
            _gamelog->print("_");
    } else {
        _gamelog->print("_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_");
    }
    _gamelog->printLine(winState);

    if (_params->gameHas2Seekers()) {
        for(int i=0;i<24;i++)
            _gamelog->print("_");
        _gamelog->printLine("_");
    }

    //do something more??
}

void SeekerHS::logLineInit2(const std::vector<double> &seekerPosVectorMine, const std::vector<double> &hiderPosVectorMine,
                            const std::vector<double> &seekerPosVectorOther, const std::vector<double> &hiderPosVectorOther,
                            const Pos &seekerPosMine, const IDPos &hiderPosMine, bool hiderVisible, const Pos &seekerPosOther,
                            const Pos &hiderPosOther)
{

    assert(_params->gameHas2Seekers()); //AG150209

    _gamelog->print(_expName);
    //my seeker pos x,y
    _gamelog->print(seekerPosVectorMine[0]);
    _gamelog->print(seekerPosVectorMine[1]);
    if (seekerPosVectorMine.size()>=3) {
        _gamelog->print(seekerPosVectorMine[2]);
    } else {
        _gamelog->print("_"); // no info
    }
    //my seeker pos row,col
    _gamelog->print(seekerPosMine);
    //hider visib
    _gamelog->print(hiderVisible);
    if (hiderVisible) {
        //hider x,y (mine)
        if (hiderPosVectorMine.size()>0) {
            _gamelog->print(hiderPosVectorMine[0]);
            _gamelog->print(hiderPosVectorMine[1]);
        } else {
            _gamelog->print("-1,-1");
        }
        //hider r,c (mine)
        _gamelog->print(hiderPosMine);
        //hider id
        _gamelog->print(hiderPosMine.id());
    } else {
        _gamelog->print("-1,-1,-1,-1,-1");
    }
    //other seeker pos x,y
    if (seekerPosVectorOther.empty()) {
        _gamelog->print("-1,-1");
    } else {
        _gamelog->print(seekerPosVectorOther[0]);
        _gamelog->print(seekerPosVectorOther[1]);
    }
    //other seeker pos r,c
    _gamelog->print(seekerPosOther);
    //other hider pos x,y
    if (hiderPosVectorOther.empty()) {
        _gamelog->print("-1,-1");
    } else {
        _gamelog->print(hiderPosVectorOther[0]);
        _gamelog->print(hiderPosVectorOther[1]);
    }
    //other hider pos r,c
    _gamelog->print(hiderPosOther);
    
    //distance(s-h)
    if (hiderPosMine.isSet()) {
        _gamelog->print(_map->distance(seekerPosMine,hiderPosMine)); 
    } else {
        _gamelog->print("NULL");
    }
    //distance(s-h2)
    if (hiderPosOther.isSet()) {
        _gamelog->print(_map->distance(seekerPosMine,hiderPosOther)); 
    } else {
        _gamelog->print("NULL");
    }
    //distance(s2-h)
    if (seekerPosOther.isSet() &&  hiderPosMine.isSet()) {
        _gamelog->print(_map->distance(seekerPosOther,hiderPosMine)); 
    } else {
        _gamelog->print("NULL");
    }
    //distance(s2-h2)
    if (seekerPosOther.isSet() && hiderPosOther.isSet()) {
        _gamelog->print(_map->distance(seekerPosOther,hiderPosOther));
    } else {
        _gamelog->print("NULL");
    }
    //distance(s-s2)
    if (seekerPosOther.isSet()) {
        _gamelog->print(_map->distance(seekerPosMine,seekerPosOther));
    } else {
        _gamelog->print("NULL");
    }
    //game state -1 is init
    _gamelog->print(-1);
    //the last 25 columns are new/suggested seeker poses
    for(int i=0;i<24;i++)
        _gamelog->print("_");
    _gamelog->printLine("_");
}


void SeekerHS::logLine2(const std::vector<double>& seekerPosVectorMine,  const std::vector<double>& hiderPosVectorMine,
                        const std::vector<double> seekerPosVectorOther, const std::vector<double> hiderPosVectorOther,
                        const Pos& seekerPosMine,  const IDPos &hiderPosMine, bool hiderVisible,
                        const Pos* seekerPosOther, const Pos* hiderPosOther,
                        const std::vector<double>& newSeekerPosVectorMine, const std::vector<double>& newSeekerPosVectorOther,
                        const Pos& newSeekerPosMine, const Pos& newSeekerPosOther,
                        double newSeekerPosMineBelief, double newSeekerPosOtherBelief, int winState)
{

    assert(_params->gameHas2Seekers()); //AG150209

    _gamelog->print(_expName);
    //my seeker pos x,y
    _gamelog->print(seekerPosVectorMine[0]);
    _gamelog->print(seekerPosVectorMine[1]);
    if (seekerPosVectorMine.size()>=3) {
        _gamelog->print(seekerPosVectorMine[2]);
    } else {
        _gamelog->print("_"); // no info
    }
    //my seeker pos row,col
    _gamelog->print(seekerPosMine);
    //hider visib
    _gamelog->print(hiderVisible);
    if (hiderVisible) {
        //hider x,y (mine)
        if (hiderPosVectorMine.size()>0) {
            _gamelog->print(hiderPosVectorMine[0]);
            _gamelog->print(hiderPosVectorMine[1]);
        } else {
            _gamelog->print("-1,-1");
        }
        //hider r,c (mine)
        _gamelog->print(hiderPosMine);
        //hider id
        _gamelog->print(hiderPosMine.id());
    } else {
        _gamelog->print("-1,-1,-1,-1,-1");
    }
    //other seeker pos x,y
    if (seekerPosVectorOther.empty()) {
        _gamelog->print("-1,-1");
    } else {
        _gamelog->print(seekerPosVectorOther[0]);
        _gamelog->print(seekerPosVectorOther[1]);
    }
    //other seeker pos r,c
    if (seekerPosOther==NULL) {
        _gamelog->print("_,_");
    } else {
        _gamelog->print(*seekerPosOther);
    }
    //other hider pos x,y
    if (hiderPosVectorOther.empty()) {
        _gamelog->print("-1,-1");
    } else {
        _gamelog->print(hiderPosVectorOther[0]);
        _gamelog->print(hiderPosVectorOther[1]);
    }
    //other hider pos r,c
    if (hiderPosOther==NULL) {
        _gamelog->print("_,_");
    } else {
        _gamelog->print(*hiderPosOther);
    }

    //distance(s-h)
    if (hiderPosMine.isSet()) {
        _gamelog->print(_map->distance(seekerPosMine,hiderPosMine));
    } else {
        _gamelog->print("NULL");
    }
    //distance(s-h2)
    if (hiderPosOther!=NULL && hiderPosOther->isSet()) {
        _gamelog->print(_map->distance(seekerPosMine,*hiderPosOther));
    } else {
        _gamelog->print("NULL");
    }
    //distance(s2-h)
    if (seekerPosOther!=NULL && seekerPosOther->isSet() &&  hiderPosMine.isSet()) {
        _gamelog->print(_map->distance(*seekerPosOther,hiderPosMine));
    } else {
        _gamelog->print("NULL");
    }
    //distance(s2-h2)
    if (seekerPosOther!=NULL && hiderPosOther!=NULL && seekerPosOther->isSet() && hiderPosOther->isSet()) {
        _gamelog->print(_map->distance(*seekerPosOther,*hiderPosOther));
    } else {
        _gamelog->print("NULL");
    }
    //distance(s-s2)
    if (seekerPosOther!=NULL && seekerPosOther->isSet()) {
        _gamelog->print(_map->distance(seekerPosMine,*seekerPosOther));
    } else {
        _gamelog->print("NULL");
    }
    //game state
    _gamelog->print(winState);
    //new seeker 1 from s1
    if (newSeekerPosVectorMine.empty()) {
        _gamelog->print("-1,-1");
    } else {
        _gamelog->print(newSeekerPosVectorMine[0]);
        _gamelog->print(newSeekerPosVectorMine[1]);
    }
    _gamelog->print(newSeekerPosMine);
    //new seeker 2 from s1
    if (newSeekerPosVectorOther.empty()) {
        _gamelog->print("-1,-1");
    } else {
        _gamelog->print(newSeekerPosVectorOther[0]);
        _gamelog->print(newSeekerPosVectorOther[1]);
    }
    _gamelog->print(newSeekerPosOther);
    //belief s1 from s1
    _gamelog->print(newSeekerPosMineBelief);
    //belief s2 from s1
    _gamelog->print(newSeekerPosOtherBelief);

    //rest is set through the logLine2Select
}


void SeekerHS::logLine2Select(const std::vector<double> &newSeekerPosVectorMineFromOther, const std::vector<double> &newSeekerPosVectorOtherFromOther,
                              const Pos &newSeekerPosMineFromOther, const Pos &newSeekerPosOtherFromOther, double newSeekerPosMineBeliefFromOther,
                              double newSeekerPosOtherBeliefFromOther,const std::vector<double>& selectedGoalPosVectorMine, const Pos& selectedGoalPosMine)
{

    //new seeker 1 from s2
    if (newSeekerPosVectorMineFromOther.empty()) {
        _gamelog->print("-1,-1");
    } else {
        _gamelog->print(newSeekerPosVectorMineFromOther[0]);
        _gamelog->print(newSeekerPosVectorMineFromOther[1]);
    }
    _gamelog->print(newSeekerPosMineFromOther);
    //new seeker 2 from s2
    if (newSeekerPosVectorOtherFromOther.empty()) {
        _gamelog->print("-1,-1");
    } else {
        _gamelog->print(newSeekerPosVectorOtherFromOther[0]);
        _gamelog->print(newSeekerPosVectorOtherFromOther[1]);
    }
    _gamelog->print(newSeekerPosOtherFromOther);
    //belief s1 from s1
    _gamelog->print(newSeekerPosMineBeliefFromOther);
    //belief s2 from s1
    _gamelog->print(newSeekerPosOtherBeliefFromOther);
    //selected goal by s1
    _gamelog->print(selectedGoalPosVectorMine[0]);
    _gamelog->print(selectedGoalPosVectorMine[1]);
    //orientation
    if (selectedGoalPosVectorMine.size()>=2) {
        _gamelog->print(selectedGoalPosVectorMine[2]);
    } else {
        _gamelog->print("_");
    }
    _gamelog->printLine(selectedGoalPosMine);
}

void SeekerHS::logLineInit(const vector<double> &seekerPosVector, const vector<double> &hiderPosVector,
                           const Pos &seekerPos, const IDPos &hiderPos, bool hiderVisible) {

    assert(!_params->gameHas2Seekers()); //AG150209

    _gamelog->print(_expName);
    _gamelog->print(seekerPosVector[0]);
    _gamelog->print(seekerPosVector[1]);
    if (seekerPosVector.size()>=3) {
        _gamelog->print(seekerPosVector[2]);
    } else {
        _gamelog->print("_"); // no info
    }
    _gamelog->print(seekerPos);
    _gamelog->print(hiderVisible);
    if (hiderVisible) {
        if (hiderPosVector.size()>0) {
            _gamelog->print(hiderPosVector[0]);
            _gamelog->print(hiderPosVector[1]);
        } else {
            _gamelog->print("-1,-1");
        }
        _gamelog->print(hiderPos);
        _gamelog->print(hiderPos.id());
        _gamelog->print(_map->distance(hiderPos,seekerPos)); //AG140409: distance seeker-hider
    } else {
        _gamelog->print("-1,-1,-1,-1,-1,NULL");
    }
    _gamelog->printLine("_,_,_,_,_,-1"); //play state: -1 = init
}

void SeekerHS::logLine(vector<double> &seekerPosVector, vector<double> &hiderPosVector,
                       Pos &seekerPos, IDPos &hiderPos, bool &hiderVisible, vector<double> &newSeekerPosVector, Pos &newSeekerPos, int winState) {

    assert(!_params->gameHas2Seekers()); //AG150209

    _gamelog->print(_expName);
    _gamelog->print(seekerPosVector[0]);
    _gamelog->print(seekerPosVector[1]);
    if (seekerPosVector.size()>=3) {
        _gamelog->print(seekerPosVector[2]);
    } else {
        _gamelog->print("_"); // no info
    }
    _gamelog->print(seekerPos);
    _gamelog->print(hiderVisible);
    if (hiderVisible) {
        if (hiderPosVector.size()>0) {
            _gamelog->print(hiderPosVector[0]);
            _gamelog->print(hiderPosVector[1]);
        } else {
            _gamelog->print("-1,-1");
        }
        _gamelog->print(hiderPos);
        _gamelog->print(hiderPos.id());
        _gamelog->print(_map->distance(hiderPos,seekerPos)); //AG140409: distance seeker-hider
    } else {
        _gamelog->print("-1,-1,-1,-1,-1,NULL");
    }
    _gamelog->print(newSeekerPosVector[0]);
    _gamelog->print(newSeekerPosVector[1]);
    if (newSeekerPosVector.size()>=3) {
        _gamelog->print(newSeekerPosVector[2]);
    } else {
        _gamelog->print("_"); // no info
    }
    _gamelog->print(newSeekerPos);
    _gamelog->printLine(winState);
}



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

/*
bool SeekerHS::checkValidNextSeekerPos(vector<double> seekerNextPosVector, bool checkPrev) {
    //assert(_map!=NULL);
    //assert(_map->width()>0);

    Pos seekerPos = _seekerPlayer->getCurPos();
    Pos seekerNextPos;
    setPosFromVector(seekerNextPos,seekerNextPosVector);
    bool valid = true;

    if (!_map->isPosInMap(seekerNextPos)) {
        //pos out of map
        DEBUG_SHS(cout << "SeekerHS.checkValidNextSeekerPos: ERROR position is out of map"<<endl;);
        valid = false;
    } else if (_map->isObstacle(seekerNextPos)) {
        //check if obstacle
        DEBUG_SHS(cout << "SeekerHS.checkValidNextSeekerPos: ERROR position is an obstacle"<<endl;);
        valid = false;
    } else if (checkPrev && _map->distance(seekerPos,seekerNextPos)>1) {
        //distance is higher than 1
        DEBUG_SHS(cout << "SeekerHS.checkValidNextSeekerPos: ERROR distance to to previous position higher than 1"<<endl;);
        valid = false;
    }

#ifdef DEBUG_SHS_ON
    if (!valid) {
        cout << "  NOT VALID Next seeker pos "<<seekerNextPos.toString();
        if (checkPrev)
            cout << " (current pos:"<<seekerPos.toString()<<")";
        cout<<endl;
    }
#endif


    //ok
    return valid;
}

bool SeekerHS::checkValidNextHiderPos(vector<double> hiderNextPosVector, vector<double> seekerNextPosVector, bool checkPrev) {
    Pos hiderPos = _seekerPlayer->getOppPos();
    Pos seekerPos = _seekerPlayer->getCurPos();
    bool visib = hiderPos.isSet();
    Pos hiderNextPos, seekerNextPos;
    bool visibNext;
    bool valid = true;

    getPosFromVectors(seekerNextPosVector, hiderNextPosVector, seekerNextPos, hiderNextPos, visibNext);


    if (visibNext && !_map->isPosInMap(hiderNextPos)) {
        //pos out of map
        DEBUG_SHS(cout << "SeekerHS.checkValidNextHiderPos: ERROR position is out of map"<<endl;);
        valid = false;
    } else if (visibNext && _map->isObstacle(hiderNextPos)) {
        //check if obstacle
        DEBUG_SHS(cout << "SeekerHS.checkValidNextHiderPos: ERROR position is an obstacle"<<endl;);
        valid = false;
    } else if (_map->numObstacles()==0 && !visibNext) {
        DEBUG_SHS(cout << "SeekerHS.checkValidNextHiderPos: ERROR position is hidden but there are no obstacles"<<endl;);
        valid = false;
    } else if (checkPrev) {
        //ag130417: imply my raytracing algo!  TODO: improve
        //
        //DEBUG_SHS(cout<<"visib="<<visib<<",visibNext="<<visibNext<<endl;);
        if (visib) {
            visib = _map->isVisible(seekerPos,hiderPos);
            DEBUG_SHS(if (!visib) cout << " (current visible but according to raytrace not) "<<flush;);
        }
        if (visibNext) {
            visibNext = _map->isVisible(seekerNextPos,hiderNextPos);
            DEBUG_SHS(if (!visibNext) cout << " (next visible but according to raytrace not) "<<flush);
        }

        if (visib && visibNext) {

        } else if (!visib && !visibNext) {
            //DEBUG_SHS(cout<<" - stays hidden"<<endl;);
            //AG TODO: this could als be a incorrect
        } else {

            if (visib) { // && !visibNext
                valid = false;
                //check if any of next pos is invisible seen from robot
                for(int r=hiderPos.row()-1; !valid && r<=hiderPos.row()+1; r++) {
                    for(int c=hiderPos.col()-1; !valid && c<=hiderPos.col()+1; c++) {
                        if (_map->isPosInMap(r,c) && !_map->isObstacle(r,c)) {
                            if (!_map->isVisible(seekerNextPos.row(), seekerNextPos.col(), r, c)) {
                                //DEBUG_SHS(cout<<"at least one next hidden - ok"<<endl;);
                                valid = true;
                            }
                        }
                    }
                }

                DEBUG_SHS(if (!valid) cout<<"SeekerHS.checkValidNextHiderPos: no hidden cells founds next to previous"<<endl;);

            } else if (visibNext) { // && !visib

                if (_autoPlayer->tracksBelief()) {
                    valid = false;
                    //check if previous was invisible, then there should be a belief >0 for any neigbhouring cell (or itself)
                    for(int r=hiderNextPos.row()-1; !valid && r<=hiderNextPos.row()+1; r++) {
                        for(int c=hiderNextPos.col()-1; !valid && c<=hiderNextPos.col()+1; c++) {
                            if (_map->isPosInMap(r,c) && !_map->isObstacle(r,c)) {
                                if (_autoPlayer->getBelief(r,c)>0) {//
                                    //if (!_map->isVisible(r,c,seekerNextPos.row,seekerNextPos.col)) {
                                    //DEBUG_SHS(cout<<"at least one belief>0 close - ok"<<endl;);
                                    valid = true;
                                }
                            }
                        }
                    }

                    DEBUG_SHS(if (!valid) cout<<"SeekerHS.checkValidNextHiderPos: no cell with belief>0 next to new location"<<endl;);
                }

            }
        }
    }

#ifdef DEBUG_SHS_ON
    if (!valid) {
        cout << "  NOT VALID next hider pos "<<(visibNext?hiderNextPos.toString():"hidden");
        if (checkPrev)
            cout << " (current pos:"<<(visib?hiderPos.toString():"hidden")<<")";
        cout <<"; seeker next pos: "<<seekerNextPos.toString();// <<endl;
    }
#endif

    return valid;
}*/


bool SeekerHS::isObstacle(const vector<double>& posVector) {
    Pos pos;
    setPosFromVector(pos,posVector);
    return _map->isObstacle(pos);
}

SeekerHSParams* SeekerHS::getParams() {
    return _params;
}

vector<double> SeekerHS::getChosenHiderPos() {
    return _chosenHiderPosVec;
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
