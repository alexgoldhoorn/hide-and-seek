#include "POMCP/hspomcp.h"

#include <iostream>
#include <cassert>

#include "Base/seekerhs.h"
#include "POMCP/hssimulator.h"
#include "POMCP/hssimulatorcont.h"
#include "POMCP/hssimulatorpred.h"
#include "POMCP/hsobservation.h"
#include "Utils/generic.h"
#include "Base/hsconfig.h"
#include "Base/autoplayer.h"
#include "Base/hsglobaldata.h"

//iriutils
#include "exceptions.h"


//TODO: put default values
// e: 2/10
// x: 2*rows*cols [triangle]; 2 [simple]
// d: maxSteps (rows+cols)*2
// ..

using namespace std;
using namespace pomcp;

HSPOMCP::HSPOMCP(SeekerHSParams* params, GMap* map) : AutoPlayer(params, map) {
    assert(params!=NULL);

    if (params->rewardType==SeekerHSParams::REWARD_NOT_SET) {
        throw CException(_HERE_, "HSPOMCP requires reward type");
    }
    if (_params->hiderStepDistance!=1 && (!_params->useContinuousPos ||
                                          _params->simHiderType!=SeekerHSParams::SIM_HIDER_TYPE_RANDOM)) {
        throw CException(_HERE_, "The hiderStepDistance!=1, but this is only supported by the random hider simulator type and with continuous movement.");
    }

    initRandomizer();

    _simulator = NULL;
    _mcts = NULL;
    _personPathPredConsumer = NULL;
    _ppWrapper = NULL;
    //_action = -1;
}

HSPOMCP::~HSPOMCP() {
    delete _mcts;
    delete _simulator;
    if (_personPathPredConsumer!=NULL) delete _personPathPredConsumer;
    if (_ppWrapper!=NULL) delete _ppWrapper;
}

/*vector<HSState*> HSPOMCP::generateObservations(HSState &ownObs) {
    //there should be at least 2 players
    assert(_playerInfoVec.size()>=2);
    //assert there are #players-2 other seekers
    //return in the vector observation of other seekers,
    //which are #players - 1 hider - this seeker
    vector<HSState*> obsVector; //(_playerInfoVec.size()-2);

    //now loop all players to get their obs
    for(PlayerInfo* pInfo : _playerInfoVec) {
        if (pInfo->isSeeker() && *pInfo!=this->playerInfo) {

            HSState* obs = new HSState();
            bool readOk = obs->readFromPlayer(*pInfo);

            if (readOk) {
                //another seeker, so stet a the player's obs
                obsVector.push_back(obs);
            }
        }
    }

    //now return own obs
    bool readOk = ownObs.readFromPlayer( playerInfo);

    assert(obsVector.size()<=_playerInfoVec.size()-2);
    assert(readOk);


    return obsVector;
}*/

//bool HSPOMCP::initBelief2(GMap *map, HSState *seeker1InitObs, HSState *seeker2InitObs, double obs1p) {
bool HSPOMCP::initBeliefRun() {
    DEBUG_POMCP(cout << "HSPOMCP::initBeliefRun"<<endl;);

    /*HSState initObs(seekerInitPos, hiderInitPos);
    if (!opponentVisible) initObs.hiderPos.clear();

    HSState initObs2(seeker2InitPos, hiderObs2InitPos);

    assert(seeker1InitObs!=NULL);
    assert(seeker1InitObs->seekerPos.isSet());*/
    //_map = map;
    //----

    //default values        //TODO: should be calculated base on a run of the algorithm with c=0 (see Silver 2010)
    if (_params->explorationConst < 0) {
        switch (_params->rewardType) {
            case SeekerHSParams::REWARD_FINAL:
            case SeekerHSParams::REWARD_FINAL_CROSS:
                _params->explorationConst = 2;
                break;
            case SeekerHSParams::REWARD_TRIANGLE:
                _params->explorationConst = _map->rowCount()*_map->colCount() + max(_map->rowCount(),_map->colCount());
                break;
                //AG140212: new rewards
            case SeekerHSParams::REWARD_FIND_SIMPLE:
                _params->explorationConst = 1;
                break;
            case SeekerHSParams::REWARD_FIND_REV_DIST:
                _params->explorationConst = _map->rowCount()*_map->colCount();
                break;
            default:
                throw CException(_HERE_, "HSPOMCP requires reward type");
                break;
        }
        cout << "Exploration constant:   " << _params->explorationConst <<endl;
    }

    if (_params->expandCount < 0) {
        _params->expandCount = 2;
        cout << "Expand count:           " << _params->expandCount <<endl;
    }

    if (_params->maxTreeDepth<0) {
        _params->maxTreeDepth = 2*(_map->rowCount()+_map->colCount());
        cout<< "Max belief tree depth:  " << (_params->maxTreeDepth<=0?"[not set] ":"")<<_params->maxTreeDepth <<endl;
    }

    if (_params->pomcpSimType==SeekerHSParams::POMCP_SIM_CONT ||
            _params->pomcpSimType==SeekerHSParams::POMCP_SIM_PRED) { //  _params->solverType==SeekerHSParams::SOLVER_POMCP_CONT) {
        //AG140317: for observation consistency check, if =0 -> 3*std.dev. (~99.6%)
        //AG140324: if cont..StdDev==0 was switched, now corrected
        if (_params->contConsistCheckSeekerDist==0) {
            if (_params->contSeekerObsStdDev==0) {
                _params->contConsistCheckSeekerDist = 1;
            } else {
                _params->contConsistCheckSeekerDist = _params->contSeekerObsStdDev * 3;
            }
        }
        if (_params->contConsistCheckHiderDist==0) {
            if (_params->contHiderObsStdDev==0) {
                _params->contConsistCheckHiderDist = 1;
            } else {
                _params->contConsistCheckHiderDist = _params->contHiderObsStdDev * 3;
            }
        }
        cout << "Consist. check seeker d:" << _params->contConsistCheckSeekerDist << endl
             << "Consist. check hider d: " << _params->contConsistCheckHiderDist << endl;
    }

    try {
        //DEBUG_POMCP(cout<<"Init simulator: "<<flush;);
        switch(_params->pomcpSimType) { //_params->solverType) {
            case SeekerHSParams::POMCP_SIM_DISCR: //SOLVER_POMCP:
                _simulator = new HSSimulator(_map, _params);
                break;
            case SeekerHSParams::POMCP_SIM_CONT: // SOLVER_POMCP_CONT:
                _simulator = new HSSimulatorCont(_map, _params);
                break;
            case SeekerHSParams::POMCP_SIM_PRED: { // SOLVER_POMCP_CONT:
                HSSimulatorPred* simPred = new HSSimulatorPred(_map, _params);
                _simulator = simPred;
                _personPathPredConsumer = simPred;
                //AG140918: check to generate the people predictor wrapper
                if (!_params->ppDestinationFile.empty()) {
                    _ppWrapper = new PeoplePredictionWrapper(_params);
                }
                break;
            }
            default:
                cout << "ERROR: unknown simulator type"<<endl;
                throw CException(_HERE_,"Unknown POMCP sim type to generate simulator for POMCP");
                break;
        }

        //DEBUG_POMCP(cout<<"ok"<<endl<<"Init MCTS... "<<flush;);
        _mcts = new MCTS(_params, _simulator, _personPathPredConsumer);
        //init action as to -1, 'none done yet'
        //_action = -1;
        //DEBUG_POMCP(cout<<"ok"<<endl<<"Init simulator + MCTS: "<<flush;);

        //cout <<"Creating obs of: "<<playerInfo.toString()<<" and # others: "<<_playerInfoVec.size()<<endl;
        //AG150612: set the observation which reads the current positions of the playerInfo
        HSObservation obs(&playerInfo, _playerInfoVec);

#ifdef DEBUG_ASSUMEPERFECTCONNECTION
        assert(_params->multiSeekerNoCommunication || obs.otherSeekersObsVec.size()==_playerInfoVec.size()-2);
#else
        assert(obs.otherSeekersObsVec.size()<=_playerInfoVec.size()-2);
#endif

        assert(obs.ownSeekerObs.seekerPos.isSet());

        //set init
        /*HSState initObs(seekerInitPos, hiderInitPos);
        if (!opponentVisible) initObs.hiderPos.clear();*/
        //init mcts
        //_mcts->init(seeker1InitObs, seeker2InitObs, obs1p);
        _mcts->init(&obs);

        //AG131031: set use weighted beliefs
        //Belief::USE_WEIGHTED_BELIEFS = _params->useWeightedBeliefs;

        //DEBUG_POMCP(cout << "Initial BELIEF: " << _mcts->getRoot()->getBelief()->toString(DEBUG_NUM_BELIEF_TOSTRING) << endl;);

        DEBUG_POMCP(cout << "HSPOMCP.initBelief done"<<endl;);

    } catch(CException &ce) {
        cout << "Exception at HSPOMCP: " << ce.what()<<endl;
        throw CException(_HERE_,ce.what());
    } catch(exception &e) {
        cout << "Exception at HSPOMCP: " << e.what()<<endl;
        //throw e;
        throw CException(_HERE_,e.what());
    }

    return true;
}

bool HSPOMCP::update(int actionDone) {
    //should be set both, this is assumed to be the function for 2 observations
    /*assert(seekerPos.isSet());
    assert(seeker2Pos.isSet());

    if (_action!=-1) {*/

    assert(playerInfo.lastAction!=-1);

    //it is not the first action

    /*assert(!opponentVisible || hiderPos.isSet()==opponentVisible); //if opponent visible it should be set, otherwise not
    if (!opponentVisible) hiderPos.clear();

    //create observations
    HSState seeker1Obs(seekerPos, hiderPos);
    HSState seeker2Obs(seeker2Pos, hiderObs2Pos);*/

    //AG140403: use action done
    if (actionDone==-1) {
        //action done is last action
        actionDone = playerInfo.lastAction;
    } else {
        DEBUG_POMCP(cout << "HSPOMCP::update: given action " << ACTION_COUT(playerInfo.lastAction) <<" different than done action "
                         <<ACTION_COUT(actionDone)<<endl;);
    }

    //AG140918: first update people predictor (if used)
    updatePeoplePrediction(playerInfo.hiderObsPosWNoise); //_hiderPlayer->currentPos); //AG150617: not yet implemented

    //AG150617: create the observation
    HSObservation obs(&playerInfo, _playerInfoVec);

    DEBUG_POMCP(cout << "HSPOMCP::update: action done="<<ACTION_COUT(actionDone)<<", obs="<<obs.toString()<<endl;);

    //do update of belief
    bool ok = _mcts->update(actionDone, &obs); //(actionDone, &seeker1Obs, 0, &seeker2Obs, obs1p /*,this*/); //NOTE: reward not used

    DEBUG_POMCP(cout << "HSPOMCP::update: Updated BELIEF: " << _mcts->getRoot()->getBelief()->toString(DEBUG_NUM_BELIEF_TOSTRING)<< endl;);
    /*}

    //AG150318: not learning, only belief update
    if (_params->pomcpDoNotLearn) {
        _action = 0;
        DEBUG_POMCP(cout << "HSPOMCP.getNextAction2: (NOT_LEARNING) "<<ACTION_COUT(_action)<< " from: s"<<seekerPos.toString()<<",s2"
                    <<seeker2Pos.toString()<<",h"<<hiderPos.toString()<<" (is also obs))"<<endl;);
    } else {
        _action = _mcts->selectAction();

        DEBUG_POMCP(cout << "HSPOMCP.getNextAction2: "<<ACTION_COUT(_action)<< " from: s"<<seekerPos.toString()<<",s2"
                    <<seeker2Pos.toString()<<",h"<<hiderPos.toString()<<" (is also obs))"<<endl;);
    }

    return _action;*/

    return ok;
}


Pos HSPOMCP::getNextPosRun(int actionDone, int *newAction) {
    //getNextPosRun2(Pos seekerPos, IDPos hiderPos, bool opponentVisible, Pos *seeker2Pos, Pos *hiderObs2Pos, double obs1p, std::vector<int> &actions, int actionDone, int n) {

    //assert already initialized
    assert(playerInfo.posRead);

    if (_numIterations>0) { //AG150710: changed to numIterations, since lastAction not always used; playerInfo.lastAction!=-1) {
        //do the update if no init has been done yet
        update(actionDone);
    }

    //AG150318: not learning, only belief update
    if (_params->pomcpDoNotLearn) {
        playerInfo.lastAction = HSGlobalData::ACT_H;
        DEBUG_POMCP(cout << "HSPOMCP::getNextPosRun: (NOT_LEARNING) "<<ACTION_COUT(playerInfo.lastAction)<< " from: s"
                         << playerInfo.currentPos.toString()<<", h:"<<_hiderPlayer->currentPos.toString()<<endl);
                    //",s2"<<seeker2Pos.toString()<<",h"<<hiderPos.toString()<<" (is also obs))"<<endl;);
    } else {
        //learn, and select action given the already passed observation
        double value = 0;
        playerInfo.lastAction = _mcts->selectAction(&value);
        playerInfo.seekerReward = value;

        DEBUG_POMCP(cout << "HSPOMCP::getNextPosRun: "<<ACTION_COUT(playerInfo.lastAction)<< " from: s"
                    << playerInfo.currentPos.toString()<<", h:"<<_hiderPlayer->currentPos.toString()<<endl);
               //",s2"<<seeker2Pos.toString()<<",h"<<hiderPos.toString()<<" (is also obs))"<<endl;);
    }

    //set return action
    if (newAction!=NULL)
        *newAction = playerInfo.lastAction;

    //now set the next position based on the action
    //set action for return
    if (playerInfo.lastAction == HSGlobalData::ACT_H) {
        //halt
        playerInfo.nextPos = playerInfo.currentPos;

    } else {
        //get direction
        double dir = HSGlobalData::getActionDirection(playerInfo.lastAction);

        //distance
        double dist = /*n* */_params->seekerStepDistance;

        if (playerInfo.hiderObsPosWNoise.isSet()) {    //_hiderPlayer->currentPos.isSet()) {
            //if the hider is visible, be sure to be a minimal distance from it
            double dsh = _map->distanceEuc(playerInfo.currentPos, playerInfo.hiderObsPosWNoise); //_hiderPlayer->currentPos);

            if (dist>dsh-_params->winDist) {
                //if the goal is too close, put it from a certain distance from the hider
                dist = dsh-_params->winDist;
            }
        }

        //move
        playerInfo.nextPos = _map->tryMoveDirStep(dir, playerInfo.currentPos, dist, _params->seekerStepDistance, _params->doVisibCheckBeforeMove);
    }

    return playerInfo.nextPos;
}

char HSPOMCP::getGameState(Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible) {
    assert(seekerInitPos.isSet());

    if (_simulator==NULL) {
        return 255; //no sim, not started yet
    } else {
        HSState state(seekerInitPos, hiderInitPos);
        return _simulator->getGameStatus(&state);
    }
}

double HSPOMCP::scoreObservation(Pos seekerPos, Pos hiderPos, int actionDone) {
    double score = 0;

    if (actionDone==-1) actionDone = playerInfo.lastAction; //_action;

    // get subtree from last action, then get observation with theses poses, and if exists return the value/num visits
    if (actionDone==-1) {
        //score = 0;
    } else {
        //generate obs state
        HSState obs(seekerPos,hiderPos);

        if (_params->useContinuousPos) {
            //convert to int
            obs.convPosToInt();
        }

        // get root
        Node* root = _mcts->getRoot();
        //get child action and observation
        NodeA* child = root->getChild(actionDone);
        if (child==NULL) {
            DEBUG_POMCP(cout << "HSPOMCP::scoreObservation: action child not found"<<endl;);
            //return 0; // no value
        } else {
            //obs
            Node* obsChild = child->getChild(&obs);
            if (obsChild==NULL) {
                DEBUG_POMCP(cout<<"HSPOMCP::scoreObservation: observation child not found"<<endl;);
                //return 0;
            } else {
                //number of times passed, not sure if we should take this value or the
                double obsCnt = obsChild->getCount();

                if (_params->filterScoreType==HSGlobalData::FILTER_SCORE_OLD) {
                    //return count as score
                    score = obsCnt;
                } else {
                    //return normalized score
                    score = obsCnt / _params->numSim;
                }
            }
        }
    }

    return score;
}


double HSPOMCP::getBelief(int r, int c) {
    assert(_mcts!=NULL);

    double b =  _mcts->getRoot()->getBelief()->getBeliefAvgAtPos(r,c,_map->rowCount(),_map->colCount());
    /*if (b==0) {
        bool hasObs = false;
        int startR = (int)(r*_params->beliefMapZoomFactor);
        int startC = (int)(c*_params->beliefMapZoomFactor);
        for(int r1=startR; !hasObs && r1<startR+_params->beliefMapZoomFactor && r1<_map->rowCount(); r1++) {
            for(int c1=startC; !hasObs && c1<startC+_params->beliefMapZoomFactor && c1<_map->colCount(); c1++) {
                if (_map->isObstacle(r1,c1))
                    hasObs = true;
            }
        }
        if (hasObs)
            b = -1;
    }*/

    return b;
}

bool HSPOMCP::tracksBelief() const {
    return _mcts!=NULL;
}

bool HSPOMCP::isSeeker() const {
    return true;
}

std::string HSPOMCP::getName() const {
    return "HSPOMCP";
}

bool HSPOMCP::canScoreObservations() {
    return playerInfo.lastAction>=0; //i.e. only when we have a legal action
}

Pos HSPOMCP::getClosestSeekerObs(Pos seekerPos) {
    Pos returnPos;

    if (playerInfo.lastAction!=-1) {
        // get root
        Node* root = _mcts->getRoot();
        //get child action and observation
        NodeA* child = root->getChild(playerInfo.lastAction);
        if (child==NULL) {
            DEBUG_POMCP(cout << "HSPOMCP::getClosestSeekerObs: action child not found"<<endl;);
            //return Pos(); // no value
        } else {

            //loop all observations and choos min distance
            double minDist = INFINITY;

            //for(int i=0; i<child->childCount(); i++) {
            for(std::map<std::string,ObsNodePair>::iterator it = child->itBeginChild(); it!=child->itEndChild(); it++) {
                //get observation
                HSState* hsObs = static_cast<HSState*>(it->second.observation);
                double d = _map->distanceEuc(seekerPos, hsObs->seekerPos);

                //check distance
                if (d<minDist) {
                    minDist = d;
                    returnPos = hsObs->seekerPos;
                }
            }

        }
    }

    if (returnPos.isSet()) {
        return returnPos;
    } else {
        return AutoPlayer::getClosestSeekerObs(seekerPos);
    }

}


double HSPOMCP::getReward() {
    double score = 0;
    if (playerInfo.lastAction==-1) {
        score = 0;
    } else {        
        // get root
        Node* root = _mcts->getRoot();
        if (root!=NULL)
            score = root->getValue();
    }

    return score;
}


void HSPOMCP::updatePeoplePrediction(const Pos &hiderPos) {
    //throw CException(_HERE_,"not yet implemented");

    if (_ppWrapper!=NULL) {
        if (hiderPos.isSet()) {
            //update from h iderpos
            //_ppWrapper->update(hiderPos.col(), hiderPos.row()); can be without velocity -> 0,0
        } else {

        }
    }
    //TODO!
}

bool HSPOMCP::handles2Obs() const {
    return true;
}
