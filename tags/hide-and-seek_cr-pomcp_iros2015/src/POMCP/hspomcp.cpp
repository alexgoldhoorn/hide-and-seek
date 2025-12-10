#include "POMCP/hspomcp.h"

#include <iostream>
#include <cassert>

#include "seekerhs.h"
#include "POMCP/hssimulator.h"
#include "POMCP/hssimulatorcont.h"
#include "POMCP/hssimulatorpred.h"
#include "Utils/generic.h"
#include "hsconfig.h"
#include "autoplayer.h"
#include "hsglobaldata.h"

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
    _action = -1;
}

HSPOMCP::~HSPOMCP() {
    delete _mcts;
    delete _simulator;
    if (_personPathPredConsumer!=NULL) delete _personPathPredConsumer;
    if (_ppWrapper!=NULL) delete _ppWrapper;
}

bool HSPOMCP::initBelief2(GMap *map, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible, Pos seeker2InitPos, Pos hiderObs2InitPos, double obs1p) {
    //both poses should be set
    assert(seekerInitPos.isSet());
    assert(seeker2InitPos.isSet());

    //create observations
    HSState initObs(seekerInitPos, hiderInitPos);
    if (!opponentVisible) initObs.hiderPos.clear();

    HSState initObs2(seeker2InitPos, hiderObs2InitPos);

    bool ok = initBelief2(map, &initObs, &initObs2, obs1p);

    return ok;
}
bool HSPOMCP::initBelief(GMap* map, Pos seekerInitPos, Pos hiderInitPos, bool opponentVisible) {
    DEBUG_POMCP(cout << "HSPOMCP.initBelief"<<endl;);

    //seeker 1 should be set
    assert(seekerInitPos.isSet());

    //create observations
    HSState initObs(seekerInitPos, hiderInitPos);
    if (!opponentVisible) initObs.hiderPos.clear();

    bool ok = initBelief2(map, &initObs, NULL, -1);

    return ok;
}

bool HSPOMCP::initBelief2(GMap *map, HSState *seeker1InitObs, HSState *seeker2InitObs, double obs1p) {
    DEBUG_POMCP(cout << "HSPOMCP.initBelief2"<<endl;);
    assert(map!=NULL);
    assert(seeker1InitObs!=NULL);
    assert(seeker1InitObs->seekerPos.isSet());
    _map = map;

    //default values        //TODO: should be calculated base on a run of the algorithm with c=0 (see Silver 2010)
    if (_params->explorationConst < 0) {
        switch (_params->rewardType) {
            case SeekerHSParams::REWARD_FINAL:
            case SeekerHSParams::REWARD_FINAL_CROSS:
                _params->explorationConst = 2;
                break;
            case SeekerHSParams::REWARD_TRIANGLE:
                _params->explorationConst = map->rowCount()*map->colCount() + max(map->rowCount(),map->colCount());
                break;
                //AG140212: new rewards
            case SeekerHSParams::REWARD_FIND_SIMPLE:
                _params->explorationConst = 1;
                break;
            case SeekerHSParams::REWARD_FIND_REV_DIST:
                _params->explorationConst = map->rowCount()*map->colCount();
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
        _params->maxTreeDepth = 2*(map->rowCount()+map->colCount());
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
                _simulator = new HSSimulator(map, _params);
                break;
            case SeekerHSParams::POMCP_SIM_CONT: // SOLVER_POMCP_CONT:
                _simulator = new HSSimulatorCont(map, _params);
                break;
            case SeekerHSParams::POMCP_SIM_PRED: { // SOLVER_POMCP_CONT:
                HSSimulatorPred* simPred = new HSSimulatorPred(map, _params);
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
        _action = -1;
        //DEBUG_POMCP(cout<<"ok"<<endl<<"Init simulator + MCTS: "<<flush;);

        //set init        
        /*HSState initObs(seekerInitPos, hiderInitPos);
        if (!opponentVisible) initObs.hiderPos.clear();*/
        //init mcts
        _mcts->init(seeker1InitObs, seeker2InitObs, obs1p);

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


int HSPOMCP::getNextAction(Pos seekerPos, Pos hiderPos, bool opponentVisible, int actionDone) {
    assert(seekerPos.isSet());

    if (_action!=-1) {
        //it is not the first action        

        assert(!opponentVisible || hiderPos.isSet()==opponentVisible); //if opponent visible it should be set, otherwise not
        if (!opponentVisible) hiderPos.clear();
        HSState seeker1Obs(seekerPos,hiderPos);

        //AG140403: use action done
        if (actionDone==-1) {
            //action done is last action
            actionDone = _action;
        } else {
            DEBUG_POMCP(cout << "HSPOMCP.getNextAction: update, given action " << ACTION_COUT(_action) <<" different than done action "<<ACTION_COUT(actionDone)<<endl;);
        }

        //AG140918: first update people predictor (if used)
        updatePeoplePrediction(hiderPos);

        _mcts->update(actionDone, &seeker1Obs, 0); //NOTE: reward not used

        DEBUG_POMCP(cout << "Updated BELIEF: " << _mcts->getRoot()->getBelief()->toString(DEBUG_NUM_BELIEF_TOSTRING)<< endl;);

    } //else: just initialized (with b0)

    //AG150318: not learning, only belief update
    if (_params->pomcpDoNotLearn) {
        _action = 0;
        DEBUG_POMCP(cout << "HSPOMCP.getNextAction: (NOT LEARNING) "<<_action << " from: s"<<seekerPos.toString()<<",h"<<hiderPos.toString()<<" (is also obs))"<<endl;);
    } else {
        _action = _mcts->selectAction();
        DEBUG_POMCP(cout << "HSPOMCP.getNextAction: "<<ACTION_COUT(_action) << " from: s"<<seekerPos.toString()<<",h"<<hiderPos.toString()<<" (is also obs))"<<endl;);
    }

    return _action;
}

int HSPOMCP::getNextAction2(Pos seekerPos, Pos hiderPos, bool opponentVisible, Pos seeker2Pos, Pos hiderObs2Pos, double obs1p, int actionDone) {
    //should be set both, this is assumed to be the function for 2 observations
    assert(seekerPos.isSet());
    assert(seeker2Pos.isSet());

    if (_action!=-1) {
        //it is not the first action

        assert(!opponentVisible || hiderPos.isSet()==opponentVisible); //if opponent visible it should be set, otherwise not
        if (!opponentVisible) hiderPos.clear();

        //create observations
        HSState seeker1Obs(seekerPos, hiderPos);
        HSState seeker2Obs(seeker2Pos, hiderObs2Pos);

        //AG140403: use action done
        if (actionDone==-1) {
            //action done is last action
            actionDone = _action;
        } else {
            DEBUG_POMCP(cout << "HSPOMCP.getNextAction2: update, given action " << ACTION_COUT(_action) <<" different than done action "<<ACTION_COUT(actionDone)<<endl;);
        }

        //AG140918: first update people predictor (if used)
        updatePeoplePrediction(hiderPos);

        DEBUG_POMCP(cout << "HSPOMCP.getNextAction2: call update with: action done="<<ACTION_COUT(_action)<<", seeker 1 obs="<<seeker1Obs.toString()
                    <<", seeker 2 obs="<<seeker2Obs.toString()<<", p_obs1="<<obs1p<<endl;);

        _mcts->update(actionDone, &seeker1Obs, 0, &seeker2Obs, obs1p /*,this*/); //NOTE: reward not used

        DEBUG_POMCP(cout << "Updated BELIEF: " << _mcts->getRoot()->getBelief()->toString(DEBUG_NUM_BELIEF_TOSTRING)<< endl;);
    }

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

    return _action;
}

vector<int> HSPOMCP::getNextMultipleActions(Pos seekerPos, Pos hiderPos, bool opponentVisible, int actionDone, int n) {
    //AG150127: needs to be updated or removed
    throw CException(_HERE_,"HSPOMCP::getNextMultipleActions: this function is NOT up to date, use getNextAction(2) or getNextRun(2)");

    if (_action!=-1) {
        //it is not the first action
        //get observation:
        //o = _simulator->getObservation(seekerPos,hiderPos,opponentVisible);

        assert(!opponentVisible || hiderPos.isSet()==opponentVisible); //if opponent visible it should be set, otherwise not
        if (!opponentVisible) hiderPos.clear();
        HSState obs(seekerPos,hiderPos);

        /*if (_params->useContinuousPos) {
            //convert to int
            obs.convPosToInt();
        }*/

        //AG140403: use action done
        if (actionDone==-1) {
            //action done is last action
            actionDone = _action;
        }

        //AG140918: first update people predictor (if used)
        updatePeoplePrediction(hiderPos);

        //update, and learn
        _mcts->update(/*_action*/ actionDone, &obs, 0); //NOTE: reward not used

        DEBUG_POMCP(cout << "Updated BELIEF: " << _mcts->getRoot()->getBelief()->toString(DEBUG_NUM_BELIEF_TOSTRING) << endl;);
    } //else: just initialized (with b0)


    vector<int> actions;
    _action = 0;

    //AG150318: not learning, only belief update
    if (_params->pomcpDoNotLearn) {
        actions.push_back(_action);
        DEBUG_POMCP(cout << "HSPOMCP.getNextMultipleActions: (NOT LEARNING) next="<<  ACTION_COUT(_action) << " from: s"<<seekerPos.toString()
                    <<",h"<<hiderPos.toString()<<" (is also obs)"<<", n="<<n<<",real n="<<actions.size()<<endl;);
    } else {
        actions = _mcts->getNextMultipleActions(n);
        assert(actions.size()>0);

        _action = actions[0];

        DEBUG_POMCP(cout << "HSPOMCP.getNextMultipleActions: next="<<  ACTION_COUT(_action) << " from: s"<<seekerPos.toString()
                    <<",h"<<hiderPos.toString()<<" (is also obs)"<<", n="<<n<<",real n="<<actions.size()<<endl;);
    }

    return actions;
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

    if (actionDone==-1) actionDone = _action;

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
    return _action>=0; //i.e. only when we have a legal action
}

Pos HSPOMCP::getClosestSeekerObs(Pos seekerPos) {
    Pos returnPos;

    if (_action!=-1) {
        // get root
        Node* root = _mcts->getRoot();
        //get child action and observation
        NodeA* child = root->getChild(_action);
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

Pos HSPOMCP::getNextPosRun(Pos seekerPos, IDPos hiderPos, bool opponentVisible, std::vector<int> &actions, int actionDone, int n) {
    return getNextPosRun2(seekerPos,hiderPos,opponentVisible,NULL,NULL,-1,actions,actionDone, n);
}

Pos HSPOMCP::getNextPosRun2(Pos seekerPos, IDPos hiderPos, bool opponentVisible, Pos seeker2Pos, Pos hiderObs2Pos, double obs1p, std::vector<int> &actions, int actionDone, int n) {
    return getNextPosRun2(seekerPos,hiderPos,opponentVisible,&seeker2Pos,&hiderObs2Pos,obs1p,actions,actionDone, n);
}

Pos HSPOMCP::getNextPosRun2(Pos seekerPos, IDPos hiderPos, bool opponentVisible, Pos *seeker2Pos, Pos *hiderObs2Pos, double obs1p, std::vector<int> &actions, int actionDone, int n) {
    //TODO: finish
    //only 1 action is taken, this is used to get the direction, and if n>1 the length of this action will be increased.

    //AG150127: choose actions for 2 seekers, or for 1
    //first get action
    if (seeker2Pos==NULL) {
        assert(hiderObs2Pos==NULL);
        _lastAction = getNextAction(seekerPos,hiderPos,opponentVisible,actionDone);
    } else {
        assert(hiderObs2Pos!=NULL);
        _lastAction = getNextAction2(seekerPos,hiderPos,opponentVisible,*seeker2Pos,*hiderObs2Pos,obs1p,actionDone);
    }

    //set action for return
    actions.push_back(_lastAction);

    if (_lastAction == HSGlobalData::ACT_H) {
        //halt
        return seekerPos;

    } else {
        //get direction
        double dir = HSGlobalData::getActionDirection(_lastAction);

        //distance
        double dist = n*_params->seekerStepDistance;

        if (hiderPos.isSet()) {
            //if the hider is visible, be sure to be a minimal distance from it
            double dsh = _map->distanceEuc(seekerPos,hiderPos);

            if (dist>dsh-_params->winDist) {
                //if the goal is too close, put it from a certain distance from the hider
                dist = dsh-_params->winDist;
            }
        }

        //move
        return (_map->tryMoveDirStep(dir, seekerPos, dist, _params->seekerStepDistance, _params->doVisibCheckBeforeMove));
    }
}


double HSPOMCP::getReward() {
    double score = 0;
    if (_action==-1) {
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
