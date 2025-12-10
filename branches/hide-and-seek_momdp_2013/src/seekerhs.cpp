#include "seekerhs.h"

#include <iostream>
#include <fstream>
#include <assert.h>

//APPL
#include "solverUtils.h"

//#include "mcvi/hideseekDiscrete/hideseekmcvi.h"

//H&S classes
#include "Solver/hsmomdp_runpolicy.h"
#include "Solver/hsmomdp_layered.h"
#include "Smart/smartseeker.h"

#include "autoplayer.h"
#include "AutoHider/smarthider.h"

#include "HSGame/gmap.h"
#include "HSGame/gplayer.h"

#include "Segment/segment.h"
#include "Segment/basesegmenter.h"
#include "Segment/kmeanssegmenter.h"
#include "Segment/robotcenteredsegmentation.h"
#include "Segment/combinecenteredsegmentation.h"
#include "Segment/testsegmenter.h"

#include "POMCP/hspomcp.h"

#include "Utils/hslog.h"
#include "Utils/generic.h"



//iriutils
#include "exceptions.h"

using namespace std;


/*int main(int argc, char *argv[])
{
    //cout << "["<<currentTimeStamp()<<"]"<<endl;

    string s=argv[1];

    cout << "Param: '"<<s<<"'"<<endl;
}
*/


SeekerHS::SeekerHS(string mapFile, string expName, int solverType, string pomdpFile, string policyFile,
                   string logFile, string timeLogFile, string gamelogFile)
{
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
    DEBUG_DELETE(cout<<"delete auto player: "<<flush;);
    delete _autoPlayer;
    DEBUG_DELETE(cout<<"ok"<<endl;);
    DEBUG_DELETE(cout<<"delete map: "<<flush;);
    delete _map;
    DEBUG_DELETE(cout<<"ok"<<endl;);
    DEBUG_DELETE(cout<<"delete seeker player: "<<flush;);
    delete _seekerPlayer;
    DEBUG_DELETE(cout<<"ok"<<endl;);
    DEBUG_DELETE(cout<<"close game log: "<<flush;);
    _gamelog->close();
    DEBUG_DELETE(cout<<"ok"<<endl;);
    DEBUG_DELETE(cout<<"delete segmenter: "<<flush;);
    delete _gamelog;
    DEBUG_DELETE(cout<<"ok"<<endl;);
    DEBUG_DELETE(cout<<"delete next seeker pos: "<<flush;);
    delete _nextSeekerPos;
    DEBUG_DELETE(cout<<"ok"<<endl;);
}

void SeekerHS::initClass(SeekerHSParams *params, bool genMap) {
    //init randomizer
    initRandomizer();

    _params = params;

    if (genMap && params->mapFile.length()==0)
        throw CException(_HERE_,"a map file is required");
    if (params->pomdpFile.length()==0 && params->solverType!=SeekerHSParams::SOLVER_POMCP && params->solverType!=SeekerHSParams::SOLVER_SMART_SEEKER)
        throw CException(_HERE_,"a POMDP file is required");
    if (params->solverType == SeekerHSParams::SOLVER_NOT_SET)
        throw CException(_HERE_,"solver type has to be set");

    if (params->logFile.length()==0)
        params->logFile = "seekerhs_log.txt";
    if (params->timeLogFile.length()==0)
        params->timeLogFile = "seekerhs_time_log.txt";
    if (params->gameLogFile.length()==0)
        params->gameLogFile = "seekerhs_game_log.txt";
        //throw CException(_HERE_,"a game log file is required");

    //set solver params
    params->setSolverParams();

    //next pos
    _nextSeekerPos = new Pos();

    //max num actions are set in init
    _maxNumActions = _numActions = 0;

    if (genMap) {
        //load gmap
        _map = new GMap(params->mapFile.c_str());

        //player
        _seekerPlayer = new Player(_map);
    } else {
        _map = NULL;
        _seekerPlayer = new Player();
    }

    //ag130416: set pos of hider/seeker to -1,-1, i.e. not initialized
    _seekerPlayer->setcurpos(-1,-1);
    _seekerPlayer->setoppos(-1,-1);

    //open game log file
    openGameLogFile(params->gameLogFile);

    //name exp
    _expName = params->expName;

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


    switch (params->solverType) {
    case SeekerHSParams::SOLVER_OFFLINE: {
        if (params->policyFile.length()==0) throw CException(_HERE_,"a policy file is required");

        //generate the solver
        RunPolicyHSMOMDP* runPolHSMOMDP = new RunPolicyHSMOMDP();
        _autoPlayer = runPolHSMOMDP;
        //cout << "initializing ..." <<endl;
        bool ok = runPolHSMOMDP->init(params->pomdpFile.c_str(), params->logFile.c_str(), params->policyFile.c_str(), params->timeLogFile.c_str());

        if (!ok) {
            throw CException(_HERE_,"Failed while initializing the offline H&S solver.");
        }
        break;
    }
    case SeekerHSParams::SOLVER_LAYERED_COMPARE:
        if (params->policyFile.length()==0) throw CException(_HERE_,"a policy file is required");
        //no break, since same als solver_layered, but with (offline) policy file
    case SeekerHSParams::SOLVER_LAYERED: {
        //generate the solver
        LayeredHSMOMDP* layeredHSMOMDP = new LayeredHSMOMDP();
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
    case SeekerHSParams::SOLVER_POMCP: {
        pomcp::HSPOMCP* hspomcp = new pomcp::HSPOMCP(_params);
        _autoPlayer = hspomcp;
        break;
    }
    default: //also SOLVER_NOT_SET        
        throw CException(_HERE_,"Unknown solver type");
        break;
    }

    assert(_autoPlayer!=NULL);
}



void SeekerHS::init(vector<double> seekerPosVector, vector<double> hiderPosVector) {
    DEBUG_SHS(cout << "SeekerHS.init: Init seeker..."<<endl;);

    //assume same axis-frame
    Pos seekerPos, hiderPos;
    bool visib;

    //calc max num actions
    _maxNumActions = 2*(_map->colCount()+_map->rowCount());
    _numActions = 0;

    DEBUG_SHS(cout << "Maximum number of actions: "<<_maxNumActions<<endl;);

    getPosFromVectors(seekerPosVector, hiderPosVector, seekerPos, hiderPos, visib);

    DEBUG_SHS(cout << "Init seeker pos: "<<seekerPos.toString()<<"; hider pos: "<< (visib ? hiderPos.toString() : "hidden")<<endl;);

    if (!checkValidNextSeekerPos(seekerPosVector,false)) {
        throw CException(_HERE_,"seeker pos is not valid");
    }
    if (!checkValidNextHiderPos(hiderPosVector,seekerPosVector,false)) {
        throw CException(_HERE_,"hider pos is not valid");
    }


    DEBUG_SHS(cout<<"checks ok"<<endl<<"Init belief.."<<endl;);

    //init player
    _autoPlayer->initBelief(_map,seekerPos,hiderPos,visib);

    DEBUG_SHS(cout<<"Init belief done"<<endl;);

    //init seeker and hider to follow steps
    _seekerPlayer->setCurPos(seekerPos);
    if (visib) {
        _seekerPlayer->setOpPos(hiderPos);
    } else {
        _seekerPlayer->setoppos(-1,-1);
    }

    //next seeker pos is the same pos
    _nextSeekerPos->set(seekerPos);

    //log
    logLineInit(seekerPosVector,hiderPosVector,seekerPos,hiderPos,visib);

    DEBUG_SHS(cout << "SeekerHS.init: done"<<endl;);
}

void SeekerHS::initMultipleObs(vector<double> seekerPosVector, vector< vector<double> > hiderPosVector) {
    DEBUG_SHS(cout << "SeekerHS.initMultipleObs: Init seeker..."<<endl;);
    vector<double> chosenHiderPosVec = chooseHiderPos(seekerPosVector, hiderPosVector, !_params->allowInconsistObs);

    init(seekerPosVector, chosenHiderPosVec);

    DEBUG_SHS(cout << "SeekerHS.initMultipleObs: done"<<endl;);
}


int SeekerHS::getNextPose(vector<double> seekerPosVector, vector<double> hiderPosVector, vector<double>& newSeekerPosVector, int* winState) {
    //get action
    Pos seekerPos, hiderPos;
    bool visib;

    DEBUG_SHS(cout << "SeekerHS.getNextPose: Doing next action #"<<_numActions<<"/"<<_maxNumActions<<"..."<<endl;);

    getPosFromVectors(seekerPosVector, hiderPosVector, seekerPos, hiderPos, visib);

    if (visib) {
        visib = _map->isVisible(seekerPos,hiderPos);
        DEBUG_SHS(if (!visib) cout << " (current visible but according to raytrace not) "<<endl;);
    }


    _map->printMap();


    DEBUG_SHS(cout << "Seeker pos: "<<seekerPos.toString()<<"; hider pos: "<< (visib ? hiderPos.toString() : "hidden")<<endl;);

    if (!checkValidNextSeekerPos(seekerPosVector, !_params->allowInconsistObs)) {
        throw CException(_HERE_,"next seeker pos is not valid");
    }
    if (!checkValidNextHiderPos(hiderPosVector,seekerPosVector, !_params->allowInconsistObs)) {
        throw CException(_HERE_,"next hider pos is not valid");
    }

    //check if new seeker pos complies with expected
    if (!_params->allowInconsistObs && !seekerPos.equals(*_nextSeekerPos)) {
        cout << "WARNING! expected seeker's position: "<<_nextSeekerPos->toString()<<", but got: "<<seekerPos.toString()<<endl;
                //TODO ERROR??
        throw CException(_HERE_,"robot is not on expected location");
    }

    //get next action based on hider/seeker pos
    int action = _autoPlayer->getNextAction(seekerPos,hiderPos,visib);

    //current pos before action:
    _seekerPlayer->setCurPos(seekerPos);
    //apply action
    _seekerPlayer->move(action);
    //new pos
    Pos p = _seekerPlayer->Getcurpos();
    _nextSeekerPos->set(p);

    //ag130411: undo movement, since we should get the new position in the next iteration
    _seekerPlayer->setCurPos(seekerPos);
    //set hider pos
    if (visib) {
        _seekerPlayer->setOpPos(hiderPos);
    } else {
        _seekerPlayer->setoppos(-1,-1);
    }

    //win state
    *winState = getWinState(seekerPos,hiderPos,visib);


    double orient = 0;
    if (visib) { //TODO: CHECK !!!
        //face to hider
        orient = getOrientation(*_nextSeekerPos, hiderPos);
    } else {
        //face in direction of going
        orient = getOrientation(seekerPos, *_nextSeekerPos);
    }

    //set output vector
    newSeekerPosVector.resize(3);
    /*newSeekerPosVector[0] = newSeekerPos.col;
    newSeekerPosVector[1] = newSeekerPos.row;
    newSeekerPosVector[2] = orient;*/
    setVectorFromPos(newSeekerPosVector, *_nextSeekerPos, orient);

#ifdef DEBUG_SHS_ON
    cout << "Action: "<<action<<", new seeker pos: "<<_nextSeekerPos->toString()<<", orientation: "<<orient<<endl
        << "Win state: ";
    switch(*winState)  {
    case STATE_PLAYING:
        cout<<"playing";
        break;
    case STATE_TIE:
        cout << "TIE";
        break;
    case STATE_WIN_HIDER:
        cout << "HIDER WINS";
        break;
    case STATE_WIN_SEEKER:
        cout << "SEEKER WINS";
        break;
    }
    cout <<endl;
#endif

    //log
    logLine(seekerPosVector,hiderPosVector,seekerPos,hiderPos,visib,newSeekerPosVector,*_nextSeekerPos,*winState);

    //one action 'done'
    _numActions++;

    DEBUG_SHS(cout << "SeekerHS.getNextPose: done"<<endl;);

    return action;
}


vector<int> SeekerHS::getNextMultiplePoses(vector<double> seekerPosVector, vector<double> hiderPosVector, int n, vector<double>& newSeekerPosVector, int* winState) {
    assert(n>0);

    //get action
    Pos seekerPos, hiderPos;
    bool visib;

    DEBUG_SHS(cout << "SeekerHS.getNextPose: Doing next action #"<<_numActions<<"/"<<_maxNumActions<<"..."<<endl;);

    getPosFromVectors(seekerPosVector, hiderPosVector, seekerPos, hiderPos, visib);

    _map->printMap(seekerPos,hiderPos);

    if (visib) {
        visib = _map->isVisible(seekerPos,hiderPos);
        DEBUG_SHS(if (!visib) cout << " (current visible but according to raytrace not) "<<endl;);
    }

    DEBUG_SHS(cout << "Seeker pos: "<<seekerPos.toString()<<"; hider pos: "<< (visib ? hiderPos.toString() : "hidden")<<endl;);

    if (!checkValidNextSeekerPos(seekerPosVector, !_params->allowInconsistObs)) {
        throw CException(_HERE_,"next seeker pos is not valid");
    }
    if (!checkValidNextHiderPos(hiderPosVector,seekerPosVector, !_params->allowInconsistObs)) {
        throw CException(_HERE_,"next hider pos is not valid");
    }

    //check if new seeker pos complies with expected
    if (!_params->allowInconsistObs && !seekerPos.equals(*_nextSeekerPos)) {
        cout << "WARNING! expected seeker's position: "<<_nextSeekerPos->toString()<<", but got: "<<seekerPos.toString()<<endl;
                //TODO ERROR??
        throw CException(_HERE_,"robot is not on expected location");
    }

    vector<int> actions = _autoPlayer->getNextMultipleActions(seekerPos, hiderPos, visib, n);

    //get next action based on hider/seeker pos
    ///int action = _autoPlayer->getNextAction(seekerPos,hiderPos,visib);

    //current pos before action:
    _seekerPlayer->setCurPos(seekerPos);
    //apply action
    _seekerPlayer->move(actions[0]);
    //new pos
    Pos p = _seekerPlayer->Getcurpos();
    _nextSeekerPos->set(p);

    //ag130411: undo movement, since we should get the new position in the next iteration
    _seekerPlayer->setCurPos(seekerPos);
    //set hider pos
    if (visib) {
        _seekerPlayer->setOpPos(hiderPos);
    } else {
        _seekerPlayer->setoppos(-1,-1);
    }

    //win state
    *winState = getWinState(seekerPos,hiderPos,visib);


    Pos curPos;
    curPos.set(seekerPos);
    //set output vector
    newSeekerPosVector.resize(3*n);
    //1st orientation
    double orient1st = -1;

    //create pose list
    for(int i=0; i<n; i++) {
        //move
        Pos newPos = _seekerPlayer->tryMove(actions[i],curPos);
        if (!newPos.isSet()) {
            newPos.set(curPos);
        }
        double orient = 0;
        if (visib) { //TODO: CHECK !!!
            //face to hider
            orient = getOrientation(newPos, hiderPos);
        } else {
            //face in direction of going
            orient = getOrientation(curPos, newPos);
        }
        if (i==0) orient1st = orient;

        setVectorFromPos(newSeekerPosVector, newPos, orient, i);
        //set cur pos
        curPos.set(newPos);
    }

#ifdef DEBUG_SHS_ON
    cout << "Action: "<<actions[0]<<", new seeker pos: "<<_nextSeekerPos->toString()<<", orientation: "<<orient1st<<endl
        << "Win state: ";
    switch(*winState)  {
    case STATE_PLAYING:
        cout<<"playing";
        break;
    case STATE_TIE:
        cout << "TIE";
        break;
    case STATE_WIN_HIDER:
        cout << "HIDER WINS";
        break;
    case STATE_WIN_SEEKER:
        cout << "SEEKER WINS";
        break;
    }
    cout <<endl;
#endif

    //log
    logLine(seekerPosVector,hiderPosVector,seekerPos,hiderPos,visib,newSeekerPosVector,*_nextSeekerPos,*winState);

    //one action 'done'
    _numActions++;

    DEBUG_SHS(cout << "SeekerHS.getNextPose: done"<<endl;);

    return actions;
}


vector<int> SeekerHS::getNextMultiplePosesForMultipleObs(vector<double> seekerPosVector, vector< vector<double> > hiderPosVector, int n,
                                                         vector<double>& newSeekerPosVector, int* winState) {

    if (!checkValidNextSeekerPos(seekerPosVector, !_params->allowInconsistObs)) {
        throw CException(_HERE_, "the seeker position is not valid");
    }

    vector<double> chosenHiderPosVec = chooseHiderPos(seekerPosVector, hiderPosVector, !_params->allowInconsistObs);

    /*double maxS = 0;
    vector<int> maxScoreObsVec;

    //validate the hider pos and choose 'best' option
    for(size_t i = 0; i < hiderPosVector.size(); i++) {
        //check
        if (!checkValidNextHiderPos(hiderPosVector[i],seekerPosVector, !_params->allowInconsistObs)) {
            continue; //skip since not valid
        }

        //get score
        double score = hiderPosVector[i][2];

        if (!_params->allowInconsistObs || checkValidNextHiderPos(hiderPosVector[i],seekerPosVector, true)) {
            score *= 2; //increase score because it is consistent with prev score
        }


        if (score >= maxS) {
            if (score > maxS) {
                maxScoreObsVec.clear();
            }

            maxScoreObsVec.push_back(i);
        }
    }

    if (maxScoreObsVec.size()==1) {
        chosenHiderPosVec = hiderPosVector[0];
    } else {
        //randomly choose a best one
        int i = random(chosenHiderPosVec.size()) ;
        chosenHiderPosVec = hiderPosVector[i];
    }*/




    return getNextMultiplePoses(seekerPosVector, chosenHiderPosVec, n, newSeekerPosVector, winState);
}


vector<double> SeekerHS::chooseHiderPos(vector<double> seekerPosVector, vector< vector<double> > hiderPosVector, bool checkPrev) {
    vector<double> chosenHiderPosVec;

    double maxS = 0;
    vector<int> maxScoreObsVec;

    //validate the hider pos and choose 'best' option
    for(size_t i = 0; i < hiderPosVector.size(); i++) {
        //check
        if (!checkValidNextHiderPos(hiderPosVector[i],seekerPosVector, checkPrev)) {
            continue; //skip since not valid
        }

        //get score
        double score = hiderPosVector[i][2];

        if (checkPrev || checkValidNextHiderPos(hiderPosVector[i],seekerPosVector, true)) {
            score *= 2; //increase score because it is consistent with prev score
        }


        if (score >= maxS) {
            if (score > maxS) {
                maxScoreObsVec.clear();
            }

            maxScoreObsVec.push_back(i);
        }
    }

    if (maxScoreObsVec.size()==1) {
        chosenHiderPosVec = hiderPosVector[maxScoreObsVec[0]];
    } else if (maxScoreObsVec.size()>1) {
        //randomly choose a best one
        int i = random(chosenHiderPosVec.size()) ;
        chosenHiderPosVec = hiderPosVector[maxScoreObsVec[i]];
    } 

    //assume we found a hider pos
    assert(chosenHiderPosVec.size()==0 /*&& hiderPosVector.size()==0*/ || chosenHiderPosVec.size()==3);

    return chosenHiderPosVec;
}

void SeekerHS::getPosFromVectors(vector<double> &seekerPosVector, vector<double> &hiderPosVector, Pos &seekerPos, Pos &hiderPos, bool &hiderVisible) {
    hiderVisible = (hiderPosVector.size()>0);

    //cout <<"hvis:"<<hiderVisible<<endl;

    /*seekerPos.row = seekerPosVector[1]; ///TODO: CHECK USAGE OF x/row etc..
    seekerPos.col = seekerPosVector[0];*/
    setPosFromVector(seekerPos,seekerPosVector);

    //cout <<"seekerpos:"<<seekerPos.toString()<<endl;

    if (hiderVisible) {
        /*hiderPos.row = hiderPosVector[1]; ///TODO: CHECK USAGE OF x/row etc..
        hiderPos.col = hiderPosVector[0];*/
        setPosFromVector(hiderPos,hiderPosVector);

        //cout<<"hiderpos:"<<hiderPos.toString()<<endl;
    } else {
        hiderPos.row = hiderPos.col = -1;
    }
}


//calculate rotation as from y axis
double SeekerHS::ang(double y, double x) {
    /*double ang = atan2(y,x);
    //- M_PI_2;
    ang =  (ang + M_PI);
    if (ang>2*M_PI) ang -= 2*M_PI;

    ang = ang - M_PI - M_PI_2;*/


    double ang = atan2(-y,x);

    ang -= M_PI_2;

    if (ang<=-M_PI)
        ang += 2*M_PI;


    return ang;
}


double SeekerHS::getOrientation(Pos pos1, Pos pos2) {
    double x,y;
    x = pos1.col - pos2.col;
    y = pos1.row - pos2.row;

    return ang(y,x);
}


int SeekerHS::getWinState(Pos seekerPos, Pos hiderPos, bool hiderVisible) {
    if (!hiderVisible || !hiderPos.isSet()) {
        //TODO: if not visible what to to do to check if hider wins ..
        return STATE_PLAYING;
    } else if (_map->distance(seekerPos,hiderPos)<=_params->winDist) { //(seekerPos.equals(hiderPos)) {
        return STATE_WIN_SEEKER;
    } else if (_map->isBase(hiderPos.row,hiderPos.col)) {
        return STATE_WIN_HIDER;
    } else if (_numActions > _maxNumActions) {
        return STATE_TIE;
    } else {
        return STATE_PLAYING;
    }
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
        //add first line
        _gamelog->printLine("exp_name,seeker_x,seeker_y,seeker_orient,seeker_row,seeker_col,hider_visible,hider_x,hider_y,hider_row,hider_col,new_seeker_x,new_seeker_y,new_seeker_orient,new_seeker_row,new_seeker_col,win_state");
    }

}

void SeekerHS::stopGame(int winState) {
    //print end of log
    _gamelog->print(_expName);
    _gamelog->print("_,_,_,_,_,_,_,_,_,_,_,_,_,_,_");
    _gamelog->printLine(winState);

    //do something more??
}

void SeekerHS::setPosFromVector(Pos &pos, vector<double> &posVector) {
    //ag130416: use floor instead of round
    pos.row = (int)floor(posVector[0]/_params->cellSizeM);
    pos.col = (int)floor(posVector[1]/_params->cellSizeM);
    /*assert(posVector.size()>=2);

    double x = posVector[0];
    double y = posVector[1];

    double xn = x*cos(_rotateExtOrigin) - y * sin(_rotateExtOrigin) + _xExtOrigin;
    double yn = x*sin(_rotateExtOrigin) + y * cos(_rotateExtOrigin) + _yExtOrigin;*/
}

void SeekerHS::setVectorFromPos(vector<double> &posVector, Pos &pos, double orientation, int index) {
    /*newSeekerPosVector[0] = newSeekerPos.col;
    newSeekerPosVector[1] = newSeekerPos.row;
    newSeekerPosVector[2] = orient;
    */

    //ag130416: add half cell to go to center of the cell
    int offset = index*3;

    posVector[offset + 0] = pos.row * _params->cellSizeM + _params->cellSizeM/2.0;
    posVector[offset + 1] = pos.col * _params->cellSizeM + _params->cellSizeM/2.0;
    posVector[offset + 2] = orientation;
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
    if (_seekerPlayer != NULL) {
        _seekerPlayer->setMap(map);
    }
}

void SeekerHS::logLine(vector<double> &seekerPosVector, vector<double> &hiderPosVector, Pos &seekerPos, Pos &hiderPos,
                       bool &hiderVisible, vector<double> &newSeekerPosVector, Pos &newSeekerPos, int winState) {

    _gamelog->print(_expName);
    _gamelog->print(seekerPosVector[0]);
    _gamelog->print(seekerPosVector[1]);
    _gamelog->print(seekerPosVector[2]);
    _gamelog->print(seekerPos);
    _gamelog->print(hiderVisible);
    if (hiderVisible) {
        _gamelog->print(hiderPosVector[0]);
        _gamelog->print(hiderPosVector[1]);
        _gamelog->print(hiderPos);
    } else {
        _gamelog->print("-1,-1,-1,-1");
    }
    _gamelog->print(newSeekerPosVector[0]);
    _gamelog->print(newSeekerPosVector[1]);
    _gamelog->print(newSeekerPosVector[2]);
    _gamelog->print(newSeekerPos);
    _gamelog->printLine(winState);
}


void SeekerHS::logLineInit(vector<double> &seekerPosVector, vector<double> &hiderPosVector, Pos &seekerPos, Pos &hiderPos, bool &hiderVisible) {
    _gamelog->print(_expName);
    _gamelog->print(seekerPosVector[0]);
    _gamelog->print(seekerPosVector[1]);
    _gamelog->print(seekerPosVector[2]);
    _gamelog->print(seekerPos);
    _gamelog->print(hiderVisible);
    if (hiderVisible) {
        _gamelog->print(hiderPosVector[0]);
        _gamelog->print(hiderPosVector[1]);
        _gamelog->print(hiderPos);
    } else {
        _gamelog->print("-1,-1,-1,-1");
    }
    _gamelog->printLine("_,_,_,_,_,-1"); //play state: -1 = init
}


void SeekerHS::setWinDistToHider(int winDist) {
    assert(winDist>0);
    if (_map!=NULL && (_map->width()<=winDist || _map->height()<=winDist)) {
        throw CException(_HERE_,"the distance has to be smaller than the map size");
    }
    _params->winDist = winDist;
}

void SeekerHS::setCellSizeM(double cellSizeM) {
    assert(cellSizeM>0);
    _params->cellSizeM = cellSizeM;
}


bool SeekerHS::checkValidNextSeekerPos(vector<double> seekerNextPosVector, bool checkPrev) {
    Pos seekerPos = _seekerPlayer->Getcurpos();
    Pos seekerNextPos;
    setPosFromVector(seekerNextPos,seekerNextPosVector);
    bool valid = true;

#ifdef DEBUG_SHS_ON
    cout << "SeekerHS.checkValidNextSeekerPos: Check next seeker pos "<<seekerNextPos.toString();
    if (checkPrev)
        cout << " (current pos:"<<seekerPos.toString()<<")";
    cout <<": ";
#endif

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

    DEBUG_SHS(cout << "SeekerHS.checkValidNextSeekerPos: valid="<<valid<<endl;);

    //ok
    return valid;
}

bool SeekerHS::checkValidNextHiderPos(vector<double> hiderNextPosVector, vector<double> seekerNextPosVector, bool checkPrev) {
    Pos hiderPos = _seekerPlayer->Getoppos();
    Pos seekerPos = _seekerPlayer->Getcurpos();
    bool visib = (hiderPos.row!=-1);
    Pos hiderNextPos, seekerNextPos;
    bool visibNext;
    bool valid = true;

    getPosFromVectors(seekerNextPosVector, hiderNextPosVector, seekerNextPos, hiderNextPos, visibNext);

#ifdef DEBUG_SHS_ON
    cout << "SeekerHS.checkValidNextHiderPos: Check next hider pos "<<(visibNext?hiderNextPos.toString():"hidden");
    if (checkPrev)
        cout << " (current pos:"<<(visib?hiderPos.toString():"hidden")<<")";
    cout <<"; seeker next pos: "<<seekerNextPos.toString() << ": ";
#endif

    if (visibNext && !_map->isPosInMap(hiderNextPos)) {
        //pos out of map
        DEBUG_SHS(cout << "ERROR position is out of map"<<endl;);
        valid = false;
    } else if (visibNext && _map->isObstacle(hiderNextPos)) {
        //check if obstacle
        DEBUG_SHS(cout << "ERROR position is an obstacle"<<endl;);
        valid = false;
    } else if (_map->numObstacles()==0 && !visibNext) {
        DEBUG_SHS(cout << "ERROR position is hidden but there are no obstacles"<<endl;);
        valid = false;
    } else if (checkPrev) {
        //ag130417: imply my raytracing algo!  TODO: improve
        //

        if (visib) {
            visib = _map->isVisible(seekerPos,hiderPos);
            DEBUG_SHS(if (!visib) cout << " (current visible but according to raytrace not) ";);
        }
        if (visibNext) {
            visibNext = _map->isVisible(seekerNextPos,hiderNextPos);
            DEBUG_SHS(if (!visibNext) cout << " (next visible but according to raytrace not) ";);
        }


        if (visib && visibNext) {
            if (_map->distance(hiderPos,hiderNextPos)>1) {
                //distance is higher than 1
                DEBUG_SHS(cout << "ERROR distance to previous position higher than 1"<<endl;);
                valid = false;
            }
        } else if (!visib && !visibNext) {
            DEBUG_SHS(cout<<" - stays hidden"<<endl;);
            //AG TODO: this could als be a incorrect
        } else {

            if (visib) { // && !visibNext
                valid = false;
                //check if any of next pos is invisible seen from robot
                for(int r=hiderPos.row-1;!valid && r<=hiderPos.row+1;r++) {
                    for(int c=hiderPos.col-1;!valid && c<=hiderPos.col+1;c++) {
                        if (_map->isPosInMap(r,c) && !_map->isObstacle(r,c)) {
                            if (!_map->isVisible(seekerNextPos.row, seekerNextPos.col, r, c)) {
                                DEBUG_SHS(cout<<"at least one next hidden - ok"<<endl;);
                                valid = true;
                            }
                        }
                    }
                }

                DEBUG_SHS(if (!valid) cout<<"no hidden cells founds next to previous"<<endl;);

            } else if (visibNext) { // && !visib

                if (_autoPlayer->tracksBelief()) {
                    valid = false;
                    //check if previous was invisible, then there should be a belief >0 for any neigbhouring cell (or itself)
                    for(int r=hiderNextPos.row-1;!valid && r<=hiderNextPos.row+1;r++) {
                        for(int c=hiderNextPos.col-1;!valid && c<=hiderNextPos.col+1;c++) {
                            if (_map->isPosInMap(r,c) && !_map->isObstacle(r,c)) {
                                if (_autoPlayer->getBelief(r,c)>0) {//
                                //if (!_map->isVisible(r,c,seekerNextPos.row,seekerNextPos.col)) {
                                    DEBUG_SHS(cout<<"at least one belief>0 close - ok"<<endl;);
                                    valid = true;
                                }
                            }
                        }
                    }

                    DEBUG_SHS(if (!valid) cout<<"no cell with belief>0 next to new location"<<endl;);
                } else {
                    DEBUG_SHS(cout<<"can't check belief with current auto player"<<endl;);
                }

           }
        }
    }

    DEBUG_SHS(cout << "SeekerHS.checkValidNextHiderPos: valid="<<valid<<endl;);

    //ok

    return valid;
}


bool SeekerHS::isObstacle(vector<double> posVector) {
    Pos pos;
    setPosFromVector(pos,posVector);
    return _map->isObstacle(pos);
}


SeekerHSParams* SeekerHS::getParams() {
    return _params;
}


SeekerHSParams::SeekerHSParams() {
    //init all params
    //all files strings already empty
    solverType = SOLVER_NOT_SET;

    ubInitType = UPPERBOUND_INIT_FIB;

    winDist = 0;
    cellSizeM = 1;

    targetPrecision = SolverParams::DEFAULT_TARGET_PRECISION;
    targetInitPrecFact = SolverParams::CB_INITIALIZATION_PRECISION_FACTOR;

    setFinalStateOnTop = false;
    setFinalTopRewards = false;

    segmenterType = SEGMENTER_NOT_SET;
    segmenterTypeX = SEGMENTER_NOT_SET;
    segmentValueType = LayeredHSMOMDP::SEGMENT_VALUE_BELIEF_ONLY;
    topRewAggr = LayeredHSMOMDP::TOP_REWARD_SUM;
    k = -1;

    rewardType = REWARD_NOT_SET;
    maxGameTime = 0;

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
    mapID = -1;
    serverPort = DEFAULT_SERVER_PORT;
    serverIP = "localhost"; //DEFAULT_SERVER_IP;

    numInitBeliefStates = 0;
    numSim = 0;
    explorationConst = 0;
    expandCount = 0;

    allowInconsistObs = false;
    ownPosObs = false;

    smartSeekerScoreType = SmartSeeker::SCORE_AVG;
    smartSeekerMaxCalcsWhenHidden = 0;
}


void SeekerHSParams::setSolverParams() {
    SolverParams* solverParams = &GlobalResource::getInstance()->solverParams;
    solverParams->useLookahead = useLookahead;
    solverParams->timeoutSeconds = timeoutSeconds;
    solverParams->maxTreeDepth = maxTreeDepth;
    solverParams->memoryLimit = memoryLimit;
    solverParams->showParams = showParams;
}


void SeekerHSParams::printVariables(bool showExtraParams) {
    //now show params
    cout    << "Experiment name:        " << expName <<endl
            << "Solver Type:            ";

    switch(solverType) {
    case SOLVER_NOT_SET:
        cout << "[not set]"<<endl;
        break;
    case SOLVER_OFFLINE:
        cout << "Offline"<<endl;
        break;
    case SOLVER_LAYERED:
        cout <<"Layered"<<endl;
        break;
    case SOLVER_LAYERED_COMPARE:
        cout <<"Layered (compare to offline)"<<endl;
        break;
    case SOLVER_MCVI_OFFLINE:
        cout <<"MCVI (offline)"<<endl;
        break;
    case SOLVER_POMCP:
        cout <<"POMCP (online)"<<endl;
        break;
    case SOLVER_SMART_SEEKER:
        cout << "SmartSeeker (online)" << endl;
        break;
    default:
        cout <<"Unknown"<<endl;
    }

    cout    << "Reward type:            ";
    switch(rewardType) {
    case REWARD_FINAL:
        cout <<"Final (1 if win [same state], -1 if loose)"<<endl;
        break;
    case REWARD_FINAL_CROSS:
        cout <<"Final cross (1 if win [same state + cross], -1 if loose)"<<endl;
        break;
    case REWARD_TRIANGLE:
        cout <<"Triangle"<<endl;
        break;
    default:
        cout << "Unknown"<<endl;
        break;
    }

    cout    << "Max game time (->tie):  " << maxGameTime<<endl
            << "Win distance:           " << winDist << endl;

    if (solverType==SOLVER_LAYERED || solverType==SOLVER_LAYERED_COMPARE) {
        cout    << "Set top final states:   " << setFinalStateOnTop <<endl
                << "Set top rewards:        " << setFinalTopRewards <<endl                
                << "Best action lookahead:  " << (useLookahead?"true":"false") << endl
                << "Segmenter Type:         ";

        switch(segmenterType) {
        case SEGMENTER_NOT_SET:
            cout <<"[not set]";
            break;
        case SEGMENTER_BASIC:
            cout <<"Basic";
            break;
        case SEGMENTER_KMEANS:
            cout <<"K-means";
            break;
        case SEGMENTER_TEST:
            cout << "Test";
            break;
        case SEGMENTER_ROBOT_CENTERED:
            cout << "Robot Centred";
            break;
        case SEGMENTER_CENTERED_COMBINED:
            cout << "Combined Centred";
            break;
        default:
            cout <<"Unknown";
        }

        cout << " (k="<<k<<";sdist="<<rcSegmDist<<",angle="<<rcAngDist<<",base_rad="<<rcBaseRad <<",high res rad="<<rcHighResR<<")" << endl
                << "Segmenter Type X:       ";

        switch(segmenterTypeX) {
        case SEGMENTER_NOT_SET:
            cout <<"[not set]";
            break;
        case SEGMENTER_ROBOT_CENTERED:
            cout << "Robot Centred";
            break;
        case SEGMENTER_CENTERED_COMBINED:
            cout << "Combined Centred";
            break;
        default:
            cout <<"Unknown / not supported";
        }

        cout << " (k="<<k<<";sdist="<<rcXSegmDist<<",angle="<<rcXAngDist<<",base_rad="<<rcXBaseRad <<",high res rad="<<rcXHighResR<<")" << endl
                << "Segment value type:     ";

        switch(segmentValueType) {
        case LayeredHSMOMDP::SEGMENT_VALUE_BELIEF_ONLY:
            cout << "Belief only"<<endl;
            break;
        case LayeredHSMOMDP::SEGMENT_VALUE_REWARD_ONLY:
            cout << "Reward only"<<endl;
            break;
        case LayeredHSMOMDP::SEGMENT_VALUE_BELIEF_REWARD:
            cout << "Belief x reward"<<endl;
            break;
        default:
            cout << "Unknown"<<endl;
        }

        cout    << "Upper bound init:       ";

        switch(ubInitType) {
        case UPPERBOUND_INIT_FIB:
            cout << "FIB (Fast Informed Bound)"<<endl;
            break;
        case UPPERBOUND_INIT_MDP:
            cout << "(Q)MDP"<<endl;
            break;
        default:
            cout << "Unknown"<<endl;
        }

        cout    << "Target precision:       " << targetPrecision << endl
                << "Init targt precis fact: " << targetInitPrecFact << endl
                << "Top reward aggreg.:     ";

        switch(topRewAggr) {
        case LayeredHSMOMDP::TOP_REWARD_SUM:
            cout <<"Sum"<<endl;
            break;
        case LayeredHSMOMDP::TOP_REWARD_AVG:
            cout << "Average" <<endl;
            break;
        case LayeredHSMOMDP::TOP_REWARD_MIN:
            cout << "Minimum" <<endl;
            break;
        case LayeredHSMOMDP::TOP_REWARD_MAX:
            cout << "Maximum" <<endl;
            break;
        default:
            cout << "Unknown"<<endl;
            break;
        }
    }
    if (solverType==SOLVER_SMART_SEEKER) {
        cout    << "Smart S. score type:    ";
        switch(smartSeekerScoreType) {
        case SmartSeeker::SCORE_AVG:
            cout <<"Average"<<endl;
            break;
        case SmartSeeker::SCORE_MAX:
            cout <<"Max"<<endl;
            break;
        case SmartSeeker::SCORE_MIN:
            cout <<"Min"<<endl;
            break;
        default:
            cout << "Unknown"<<endl;
            break;
        }

        cout    << "Smart S. max calc cells:" << (smartSeekerMaxCalcsWhenHidden==0?"[no max] ":"") << smartSeekerMaxCalcsWhenHidden <<endl;
    }

    cout    << "MOMDP file:             " << (pomdpFile.length()==0?"[not set]":pomdpFile) <<endl
            << "Policy file:            " << (policyFile.length()==0?"[not set]":policyFile) <<endl
            << "Map file:               " << (mapFile.length()==0?"[not set]":mapFile)<<endl;

    if (showExtraParams) {
        cout    << "Map ID:                 " << (mapID==-1 ? "[not set] ": "")<<mapID<<endl
                << "Server IP:              " << (serverIP.length()==0?"[not set]":serverIP)<<endl
                << "       port:            " << (serverPort==-1?"[not set] ":"")<<serverPort<<endl
                << "User name:              " << userName <<endl
                << "Opponent type:          ";

        switch(opponentType) {
        case HSGlobalData::OPPONENT_TYPE_HIDER_RANDOM:
            cout << "Random Hider"<<endl;
            break;
        case HSGlobalData::OPPONENT_TYPE_HIDER_SMART:
            cout << "Smart Hider (noise std.="<<oppHiderNoiseStd<<")" <<endl;
            break;
        case HSGlobalData::OPPONENT_TYPE_HIDER_ACTION_LIST:
            cout << "Action List Hider (" << oppActionFile << ")"<< endl;
            break;
        case HSGlobalData::OPPONENT_TYPE_HIDER_ALLKNOWING:
            cout << "All Knowing Smart Hider (noise std.="<<oppHiderNoiseStd<<")" <<endl;
            break;
        case HSGlobalData::OPPONENT_TYPE_HIDER_VERYSMART:
            cout << "Very Smart Hider (noise std.="<<oppHiderNoiseStd<<")" <<endl;
            break;
        case HSGlobalData::OPPONENT_TYPE_HIDER_VALLKNOWING:
            cout << "All Knowing Very Smart Hider (noise std.="<<oppHiderNoiseStd<<")" <<endl;
            break;
        case HSGlobalData::OPPONENT_TYPE_HUMAN:
            cout << "Human"<<endl;
            break;
        default:
            cout << "Unknown"<<endl;
        }
    }

    cout    << "solver time out(s):     " << (timeoutSeconds<=0?"no time-out ":"")<<timeoutSeconds <<endl
            << "Max belief tree depth:  " << (maxTreeDepth<=0?"[not set] ":"")<<maxTreeDepth <<endl
            << "Num. initial belief st.:" << (numInitBeliefStates<=0?"[not set] ":"")<<numInitBeliefStates<<endl
            << "Number of simulations:  " << (numSim<=0?"[not set] ":"")<<numSim<<endl
            << "Exploration constant:   " << explorationConst <<endl
            << "Expand count:           " << expandCount<<endl
            //<< "Log prefix:           " << (logFilePrefix==NULL?"[not set]":logFilePrefix)<<endl
            << "Allow inconsistencies:  " << (allowInconsistObs?"yes":"no") <<endl
            << "Log:                    " << logFile<<endl
            << "Time log:               " << timeLogFile<<endl
            << "Game log:               " << gameLogFile<<endl
            << "Do pruning:             " << (doPruning?"yes":"no") << endl
            << "Memory limit:           " << (memoryLimit/(1024*1024)) << " " << (memoryLimit==0?"none":"MB") << endl
            << "Time:                   " << currentTimeStamp() << endl
            << endl<<endl;
}

