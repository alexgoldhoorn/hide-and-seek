#include "POMCP/hssimulator.h"
#include "POMCP/hsstate.h"

#include <iostream>

#include "hsglobaldata.h"
#include "seekerhs.h"

#include "Utils/generic.h"
#include "Utils/timer.h"
#include "hsconfig.h"
#include "Smart/smartseeker.h"

//iriutils
#include "exceptions.h"

#include <cassert>
#include <vector>
#include <random>


using namespace pomcp;
using namespace std;

//TODO CHECK why can't use inline function with inheritance
 bool HSSimulator::hiderWin(const HSState* hsstate) {
     /*if (_params->winDist==0) {
        return _basePos.equalsInt(hsstate->hiderPos); // && !hsstate->seekerPos.equals(hsstate->hiderPos);
     } else {
         return (_map->distanceEuc(hsstate->hiderPos,_map->getBase()) <= _params->winDist);
     }*/

     return _basePos.equalsInt(hsstate->hiderPos); // || _params->winDist>0 && _map->distanceEuc(hsstate->hiderPos,_map->getBase()) <= _params->winDist;
}

 bool HSSimulator::seekerWin(const HSState* hsstate) {
    if (_params->winDist==0) {
        //ag131205: added for efficiency
        return hsstate->seekerPos.equalsInt(hsstate->hiderPos);
    } else {
        return (_map->distanceEuc(hsstate->hiderPos,hsstate->seekerPos) <= _params->winDist);
    }
}

 //AG140305: disabled because it was used, but the _timer was niether initialized nor updated
 //          for simulations MCTS already takes the time into account!! Maybe later we want to have the reward in function of time...
 /*
 bool HSSimulator::tie(HSState* hsstate) {
    return (_params->maxNumActions>0 && _timer>=_params->maxNumActions);
            //(_params->maxGameTime>0 && _timer>=_params->maxGameTime); //TODO: use internal time estimating time each step takes
}*/


HSSimulator::HSSimulator(GMap *map, SeekerHSParams* params) : _randomGenerator(_randomDevice()),
        _uniformProbDistr(0,1), _uniformDirDistr(0,2*M_PI) {

    assert(map!=NULL);
    _map = map;

    if (params->rewardType==SeekerHSParams::REWARD_NOT_SET) {
        throw CException(_HERE_,"reward type has not been set");
    }

    //max value for triangle reward
    _maxRewValue = map->colCount() * map->rowCount();
    //hidden obs (since observations are id-ed by state, the one after last is 'unknown' obs - consistent with POMDP impl)
    //_hiddenObs = map->numFreeCells();
    _basePos = map->getBase();
    _params = params;

    /*_hiderPlayer.setMap(map);
    _seekerPlayer.setMap(map);*/
    _player.setMap(map);

    //number of free cells + 1 (unknown)
    //ag130831: observations are ALL possible combis of position + not visible
    //      in practice a lot won't be used since certain seeker-hider positions
    //      because the hider won't be visible from the seeker's pos
    _numObs = (map->numFreeCells() + 1) * map->numFreeCells();

    //AG140108: only when really used
    _smartSeeker = NULL;

    //AG140909: init the vars
    //resetParams();
}

HSSimulator::~HSSimulator() {
    if (_smartSeeker!=NULL) {
        delete _smartSeeker;
    }
}

unsigned int HSSimulator::getNumActions() {
    return HSGlobalData::NUM_ACTIONS;
}

unsigned int HSSimulator::getNumObservations() {
    return _numObs;
}

double HSSimulator::getDiscount() {
    return _params->discount;
}

State* HSSimulator::genInitState(const State* obs, const State* obs2, double obs1p) {
    DEBUG_POMCP(cout<<"HSSimulator::genInitState: ";);

    //AG131104: the seeker pos should be passed, and hider pos if visible
    const HSState* hsObs = NULL;
    if (obs!=NULL) {
        hsObs = static_cast<const HSState*>(obs);

        if (!hsObs->seekerPos.isSet()) {
            throw CException(_HERE_,"the seeker position should be known when given a random init state");
        }
    } else {
        throw CException(_HERE_,"the seeker position should be known when given a random init state, now NULL obs");
    }

    //AG150123: the observation of the seeker2
    const HSState* hsObs2 = NULL;
    if (obs2!=NULL) {
        hsObs2 = static_cast<const HSState*>(obs2);
    }

    //return state
    HSState* state = new HSState();

    //seeker pos
    assert(hsObs->seekerPos.isSet());
    state->seekerPos = hsObs->seekerPos;

    //hider pos
    if (hsObs->hiderVisible()) {

        if (hsObs2!=NULL && hsObs2->hiderVisible()) {
            DEBUG_POMCP(cout<<"obs1 or 2 (p1="<<obs1p<<") - ";);
            //both observations are available, now do a random choice between them with the given probabilities
            assert(obs1p>=0 && obs1p<=1);

            if (_uniformProbDistr(_randomGenerator) <= obs1p) {
                DEBUG_POMCP(cout<<"obs1";);
                state->hiderPos = hsObs->hiderPos;
            } else {
                DEBUG_POMCP(cout<<"obs2";);
                state->hiderPos = hsObs2->hiderPos;
            }

        } else {
            DEBUG_POMCP(cout<<"obs1";);
            state->hiderPos = hsObs->hiderPos;
        }

    } else if (hsObs2!=NULL && hsObs2->hiderVisible()) {
        DEBUG_POMCP(cout<<"obs2";);
        state->hiderPos = hsObs2->hiderPos;
    } else {
        DEBUG_POMCP(cout<<"hidden";);

        //get random non-visible pos
        vector<Pos> invisPosVec;
        if (hsObs2==NULL) {
            DEBUG_POMCP(cout<<" (from o1)";);
            invisPosVec = _map->getInvisiblePoints(hsObs->seekerPos, _params->takeDynObstOcclusionIntoAccountWhenLearning);
        } else {
            DEBUG_POMCP(cout<<" (from o1 & o2)";);
            invisPosVec = _map->getInvisiblePoints(hsObs->seekerPos, hsObs2->seekerPos ,_params->takeDynObstOcclusionIntoAccountWhenLearning);
        }
        //get random hiderpos
        state->hiderPos  = invisPosVec[random(invisPosVec.size()-1)];
    }

    //This is required because observations and a chosen invisible point are always discrete, therefore if
    //continuous states are used, the discrete coordinates are in the left bottom corner of a grid cell instead of the center.
    if (_params->useContinuousPos) state->convPosToCont();

    DEBUG_POMCP(cout<<" -> "<<state->toString()<<endl;);

    return state;
}


std::vector<State*> HSSimulator::genAllInitStates(const State* obs, int n, const State* obs2, double obs1p) {
    DEBUG_POMCP(cout<<"HSSimulator::genAllInitStates ("<<n <<"): "<<flush;);
    assert(n>0);

    //AG131104: the seeker pos should be passed, and hider pos if visible
    const HSState* hsObs = NULL;
    if (obs!=NULL) {
        hsObs = static_cast<const HSState*>(obs);

        if (!hsObs->seekerPos.isSet()) {
            throw CException(_HERE_,"the seeker position should be known when given a random init state");
        }
    } else {
        throw CException(_HERE_,"the seeker position should be known when given a random init state, now NULL obs");
    }

    //AG150123: the observation of the seeker2
    const HSState* hsObs2 = NULL;
    if (obs2!=NULL) {
        hsObs2 = static_cast<const HSState*>(obs2);
    }

    //return vector
    vector<State*> allStatesVec;

    if (hsObs->hiderVisible()) {

        if (hsObs2!=NULL && hsObs2->hiderVisible()) {
            DEBUG_POMCP(cout<<"obs1 or 2 (p1="<<obs1p<<")"<<endl;);
            //both observations are available, now do a random choice between them with the given probabilities
            assert(obs1p>=0 && obs1p<=1);

            //now fill the vector with n randomly chosen poses
            for(int i=0; i<n; i++) {
                HSState* state;
                if (_uniformProbDistr(_randomGenerator) <= obs1p) {
                    state = new HSState(hsObs->seekerPos, hsObs->hiderPos);
                } else {
                    state = new HSState(hsObs->seekerPos, hsObs2->hiderPos);
                }

                if (_params->useContinuousPos) state->convPosToCont();
                allStatesVec.push_back(state);
            }

        } else {
            //hider is visible on 1 place, so only 1 belief state (i.e. 100% belief)
            DEBUG_POMCP(cout<<"obs1"<<endl;);
            HSState* state = new HSState(hsObs->seekerPos, hsObs->hiderPos);
            if (_params->useContinuousPos) state->convPosToCont();
            allStatesVec.push_back(state);
        }

    } else if (hsObs2!=NULL && hsObs2->hiderVisible()) {
        DEBUG_POMCP(cout<<"obs2"<<endl;);
        //hider is visible on 2, so only 1 belief state (i.e. 100% belief)
        HSState* state = new HSState(hsObs->seekerPos, hsObs2->hiderPos);
        if (_params->useContinuousPos) state->convPosToCont();
        allStatesVec.push_back(state);

    } else {        
        DEBUG_POMCP(cout << "hidden"<<endl;);

        //get random non-visible pos
        vector<Pos> invisPosVec;
        if (hsObs2==NULL) {
            DEBUG_POMCP(cout<<" (from o1)"<<endl;);
            invisPosVec = _map->getInvisiblePoints(hsObs->seekerPos, _params->takeDynObstOcclusionIntoAccountWhenLearning);
        } else {
            DEBUG_POMCP(cout<<" (from o1 & o2)"<<endl;);
            invisPosVec = _map->getInvisiblePoints(hsObs->seekerPos, hsObs2->seekerPos, _params->takeDynObstOcclusionIntoAccountWhenLearning);
        }

        //AG150216: vector could be empty and cause out of bounds error on selecting the pos
        if (invisPosVec.empty()) {
            //if no obstacles or visibility from different locations, then everything could be visible
            //if (_map->numObstacles()==0 || hsObs2!=NULL) { //AG150224: in theory having #obs>0 doesn't ensure that there are places to hide
                cout << "HSSimulator::genAllInitStates - WARNING: no not visible cells found, therefore random visible points are selected"<<endl;
                for(int i=0; i<n; i++) {
                    HSState* state = new HSState(hsObs->seekerPos, _map->genRandomPos());
                    allStatesVec.push_back(state);
                }
            /*} else {
                throw CException(_HERE_, "There should be at least 1 location not visible");
            }*/
        } else {
            //AG150126: init n belief points instead of all (otherwise not sure what to do in the case of having both obs1 and obs2)
            /*if (n==0) {
                //now generate the initial states
                for (vector<Pos>::iterator it = invisPosVec.begin(); it!=invisPosVec.end(); it++) {
                    HSState* state = new HSState(hsObs->seekerPos,*it);
                    if (_params->useContinuousPos) state->convPosToCont();
                    allStatesVec.push_back(state);
                }
            } else {*/
                //generate n init states
                for(int i=0; i<n; i++) {
                    HSState* state = new HSState(hsObs->seekerPos, invisPosVec[random(invisPosVec.size()-1)]);
                    if (_params->useContinuousPos) state->convPosToCont();
                    allStatesVec.push_back(state);
                }
            //}
        }
    }

    return allStatesVec;
}

void HSSimulator::getActions(const State* state, History* history, std::vector<int>& actions, bool smart) {
    assert(state!=NULL);
    const HSState* hsstate = static_cast<const HSState*>(state);

    //clear actions (?)
    //actions.clear();
    //NOTE: smart not used as var yet, maybe in future option of smart or not

    //try actions
    FORs(a,HSGlobalData::NUM_ACTIONS) {
        //try action
        Pos newPos = /*_player.*/ _map->tryMove(a, hsstate->seekerPos);//AG150126: added obs2//AG150126: added obs2
        //if possible add to list
        if (newPos.isSet()) {
            actions.push_back(a);
        }
    }
}

void HSSimulator::getPossibleNextHiderPos(const Pos &prevPos, vector<Pos>& posVec, const HSState* obs, const HSState* obs2) {
    //should only be here if hiderpos is not set or no observation passed
    assert(_params->contFalsePosProb>0 || obs==NULL || !obs->hiderPos.isSet()); //AG150126: added obs2Prob>0

    //AG150126 NOTE: for 2 seekers this should work, since only 1 observation of hider can be passed (should be consistent, if not choice
    //               should be made before.

    //for consistency check
    HSState* state = NULL;
    if (obs!=NULL) {
        //AG150122: added cast
        state = dynamic_cast<HSState*>(obs->copy());
        assert(state!=NULL);
    }

    FORs(a,HSGlobalData::NUM_ACTIONS) {
        //try action
        Pos newPos = _map->tryMove(a, prevPos);

        //if possible add to list
        //and if we have an obs, check if it is consistent
        if (newPos.isSet()) {

            //check if obs passed, and then if consistent with observation
            if (obs != NULL) {
                //assume hider's pos given, check if observ is consistent
                state->hiderPos = newPos;
                //AG140324: us consistency check
                if (_params->pomcpCheckNewBeliefGen && !isStateConsistentWithObs(state, obs, obs2)) {
                    //AG150319: allow inconsistent states
                    if (!(_params->pomcpSimType==SeekerHSParams::POMCP_SIM_CONT && _params->contUpdInconsistAcceptProb>0 &&
                                                _uniformProbDistr(_randomGenerator)<_params->contUpdInconsistAcceptProb)) {
                        continue;
                    }
                }

            }

            posVec.push_back(newPos);
        }
    }

    if (obs!=NULL) delete state;
}

void HSSimulator::setInitialNodeValue(const State* state, History* history, BaseNode* node, int action) {
    double r = 0;
    if (action==-1) {
        r = getImmediateReward(state);
    } else {
        State* obsOut;
        State* nextState = step(state, action, obsOut, r);
        r = getImmediateReward(nextState);
        delete nextState;
        delete obsOut;
    }

    node->setCountAndValue(1,r); //?? IS THIS OK??? -> not done iby expandNode of silver!!!
}



State* HSSimulator::step(const State *state, int action, State*& obsOut, double &reward, bool genOutObs, const State* obsIn,
                         const State *obs2In, double obs1p) {
    assert(state!=NULL);
    assert(action>=0 && action<HSGlobalData::NUM_ACTIONS);    

    const HSState* hsstate = static_cast<const HSState*>(state);
    assert(hsstate->seekerPos.isSet());
    assert(hsstate->hiderPos.isSet());

    Pos newHiderPos;
    Pos newSeekerPos;

    //TODO: later on we should handle uncertainty of observation ....

    //check if to use the observation
    const HSState* hsObsIn = NULL;
    //AG150126: bools to prevent rechecking NULL and visibility
    bool hider1Visib = false;
    if (obsIn!=NULL) {
        hsObsIn = static_cast<const HSState*>(obsIn);
        hider1Visib = hsObsIn->hiderVisible();
    }
    //AG150126: added obs of seeker 2
    const HSState* hsObs2In = NULL;
    bool hider2Visib = false;
    if (obs2In!=NULL) {
        assert(hsObsIn!=NULL);//should also be set
        hsObs2In = static_cast<const HSState*>(obs2In);
        hider2Visib = hsObs2In->hiderVisible();
    }

    //seeker pos
    //if obs, get that value
    if (hsObsIn!=NULL) {
        newSeekerPos = hsObsIn->seekerPos;
    } else {
        //do move of seeker
        newSeekerPos = _map->tryMove(action, hsstate->seekerPos);
        if (!newSeekerPos.isSet()) {
            //action not possible, so same position
            newSeekerPos = hsstate->seekerPos;
        }
    }

    if ((hider1Visib || hider2Visib) && //AG150126: added obs2
            (_params->contFalsePosProb==0 || _uniformProbDistr(_randomGenerator) > _params->contFalsePosProb)) { //AG150325: added false pos prob
        //AG130827: not hidden, so we know the new pos of hider

        if (hider1Visib && hider2Visib) {
            assert(obs1p>=0 && obs1p<=1);
            //now use probability (if one had to be chosen it had to be decided by the caller)
            if (_uniformProbDistr(_randomGenerator) <= obs1p) {
                newHiderPos = hsObsIn->hiderPos;
            } else {
                newHiderPos = hsObs2In->hiderPos;
            }
        } else if (hider1Visib) {
            //obs 1
            newHiderPos = hsObsIn->hiderPos;
        } else {
            //obs 2
            newHiderPos = hsObs2In->hiderPos;
        }
        //hsObsOut->hiderPos.set(newHiderPos);
    } else {
        //no in Obs, or it is hidden

        //move hider
        vector<Pos> posVec;
        //get next hider positions possible
        getPossibleNextHiderPos(hsstate->hiderPos, posVec, hsObsIn, hsObs2In);

        if (hsObsIn!=NULL && posVec.size()==0) {
            DEBUG_POMCP_DETAIL(cout << "HSSimulator::step: no consistent obs="<< hsObsIn->toString() <<",state="<<hsstate->toString()<<" ";);
            //getPossibleNextPos(hsstate->hiderPos, posVec);
            //NO consistent state found with the observation, this happens when the observation is 'not visible'
            //and of the chosen state all next hider positions are visible
            return NULL;
        }
        assert(posVec.size()>0);

        //AG140326: we always do the previuos check to be sure that we can go a certain direction
        if (_params->useContinuousPos && _params->simHiderType==SeekerHSParams::SIM_HIDER_TYPE_RANDOM) {
            unsigned int tries = 0;
            do {
                //move in random dir
                double ranDir = _uniformDirDistr(_randomGenerator);
                newHiderPos = _map->tryMoveDir(ranDir, hsstate->hiderPos, _params->hiderStepDistance, true);
                tries++;
            } while (tries<_params->pomcpSimMaxNumTriesHiderPos && !newHiderPos.isSet());

            /*DEBUG_POMCP(*/
            if (!newHiderPos.isSet())
                cout<<"WARNING: failed to simulate correct new hiderpos for "<<hsstate->hiderPos.toString()<<" after "<<tries<<" tries."<<endl;//);
            /*else
                cout << "sim: "<<hsstate->hiderPos.toString() <<" to "<<newHiderPos.toString() << (newHiderPos.isSet()?"":"<NOT POSSIBLE>") << endl;*/
        }


        if (!newHiderPos.isSet()) { //if not yet a hiderpos has been set by continuous random pos generator
            //choose next hider pos
            if (posVec.size()==1) {
                newHiderPos = posVec[0];

            } else if (_uniformProbDistr(_randomGenerator) > _params->simHiderRandomActProb)  {
                //AG131022: add a check for use of random action
                //use the action strategy (if prob=0 -> always use strategy, otherwise chance of using only random)

                //ag131017: check if using smart or random movement
                switch (_params->simHiderType) {
                case SeekerHSParams::SIM_HIDER_TYPE_RANDOM:
                    newHiderPos = posVec[random(posVec.size()-1)];
                    break;
                case SeekerHSParams::SIM_HIDER_TYPE_SMART:
                    cout << "SIM_HIDER_TYPE_SMART not implemented"<<endl;

                case SeekerHSParams::SIM_HIDER_TYPE_TOBASE: {
                    //find shortest distance pos
                    newHiderPos = findClosestPos(posVec, _basePos) ;
                    break;
                }
                default:
                    throw CException(_HERE_,"unknown simulation hider type");
                }
            } else {
                //random
                newHiderPos = posVec[random(posVec.size()-1)];
            }
        } // if !newHiderPos.isSet
    } //no hsObsIn set

    if (genOutObs) {
        //out obs
        HSState* hsObsOut = new HSState();
        obsOut = hsObsOut;
        //set obs
        hsObsOut->seekerPos = newSeekerPos;

        bool isVisib = _map->isVisible(newHiderPos, newSeekerPos, _params->takeDynObstOcclusionIntoAccountWhenLearning);
        if (!isVisib && _params->contObserveIfNotVisibProb > 0) {
            if (_uniformProbDistr(_randomGenerator) <= _params->contObserveIfNotVisibProb) {
                //make it visible even though raytrace says it isn't
                isVisib = true;
            }
        }

        //now decide observation
        if (isVisib) {
            //obsOut = _map->getIndexFromCoord(newHiderPos);
            hsObsOut->hiderPos = newHiderPos;
        } else {
            //obsOut = _hiddenObs;
            hsObsOut->hiderPos.clear();
        }

    }

    //reward
    HSState* nexthsState = new HSState(newSeekerPos,newHiderPos);
    reward = getImmediateReward(state, nexthsState);

    return nexthsState;
}

Pos HSSimulator::findClosestPos(const vector<Pos>& posVec, const Pos& toPos) {
    double minDist = HSGlobalData::INFTY_POS_FLT;
    vector<Pos> minPosvec;

    //check minimum distance
    for(const Pos& pos : posVec/*vector<Pos>::iterator it = posVec.begin(); it != posVec.end(); it++*/) {
        double dist = _map->distance(pos,toPos);

        if (dist<=minDist) {
            if (dist<minDist) { //we found the smallest, so clear the vector
                minPosvec.clear();
                minDist = dist;
            }
            minPosvec.push_back(pos);
        }
    }

    if (minPosvec.size()==1) {
        return minPosvec[0];
    } else {
        return minPosvec[random(minPosvec.size()-1)];
    }
}


bool HSSimulator::isFinal(const State *state) {
    //check win state       //TODO: could be cached
    const HSState* hsstate = static_cast<const HSState*>(state);

    return  seekerWin(hsstate) || /*tie(hsstate) ||*/ (_params->gameType==HSGlobalData::GAME_HIDE_AND_SEEK && hiderWin(hsstate));

}


double HSSimulator::getImmediateReward(const State *state, const State* nextState) {
    assert(state!=NULL);

    //get states
    const HSState* hsstate = static_cast<const HSState*>(state);
    const HSState* hsnextstate = NULL;
    if (nextState!=NULL) {
        hsnextstate = static_cast<const HSState*>(nextState);
    }

    double r = 0;

    //calculate the reward
    switch(_params->rewardType) {
        case SeekerHSParams::REWARD_FIND_SIMPLE:
        case SeekerHSParams::REWARD_FINAL_CROSS:
            // if next pos seeker =
            if (hsnextstate!=NULL &&
                    hsstate->seekerPos.equals(hsnextstate->hiderPos) &&
                    hsstate->hiderPos.equals(hsnextstate->seekerPos)) {
                r = 1;
                break;
            } //else try if we are in a winning state:
        case SeekerHSParams::REWARD_FINAL: {     //TODO: check. should we use nextState (?)
                if (hsnextstate!=NULL) hsstate = hsnextstate;
                if (seekerWin(hsstate)) {
                    r = 1;
                } else if (_params->rewardType!=SeekerHSParams::REWARD_FIND_SIMPLE &&  hiderWin(hsstate)) {
                    r = -1;
                }
                break;
            }
        case SeekerHSParams::REWARD_TRIANGLE: {
            if (hsnextstate!=NULL) hsstate = hsnextstate;
            //check iflost
            if (seekerWin(hsstate)) {
                r = _maxRewValue;
            } else if (hiderWin(hsstate)) {
                r = -1.0 * _maxRewValue;
            }   else {
                //distances
                double dsh = _map->distance(hsstate->hiderPos, hsstate->seekerPos);
                double dhb = _map->distance(hsstate->hiderPos, _basePos);
                double dsb = _map->distance(hsstate->seekerPos, _basePos);

                if (dhb>=dsb) {
                    r = _maxRewValue - dsh;
                } else {
                    r = -dsh;
                }
                /*cout << "base:"<<_basePos.toString()<<",seeker:"<<hsstate->seekerPos.toString()<<",hider:"<<hsstate->hiderPos.toString()<<
                        ",dsh="<<dsh<<",dhb="<<dhb<<",dsb="<<dsb<<"->r="<<r<<endl;*/
            }
            break;
        }
        case SeekerHSParams::REWARD_FIND_REV_DIST: {
            if (hsnextstate!=NULL) hsstate = hsnextstate;
            double dsh = _map->distance(hsstate->hiderPos, hsstate->seekerPos);
            r = -dsh;
            break;
        }
        default:
            throw CException(_HERE_,"unkown reward type");
    }

    return r;
}

void HSSimulator::setMaxRewardValue(unsigned int maxRew) {
    _maxRewValue = maxRew;
}

char HSSimulator::getGameStatus(const HSState *state) {
    if (seekerWin(state)) {
        return  HSGlobalData::GAME_STATE_SEEKER_WON;
    } else if (hiderWin(state)) {
        return HSGlobalData::GAME_STATE_HIDER_WON;
    } /*else if (tie(state)) {
        return HSGlobalData::GAME_STATE_TIE;
    }*/ else {
        return HSGlobalData::GAME_STATE_RUNNING;
    }
}


//TODO/NOTE: in the future we might have states that are not consistent with observation due to uncertainty
bool HSSimulator::checkNextStateObs(const State *state, const State *nextState, const State* obs) {
    if (nextState == NULL) {
        cout << "HSSimulator::checkNextStateObs: expected a next state!"<<endl;
        return false;
    }

    const HSState* hsState = NULL;
    if (state != NULL)
        hsState = static_cast<const HSState*>(state);
    const HSState* hsNextState = static_cast<const HSState*>(nextState);

    bool ok = true;

    //check if poses are ok
    if (!_map->isPosInMap(hsNextState->seekerPos)) {
        cout << "HSSimulator::checkNextStateObs: ERROR next seeker pos is not in map of state: " << hsNextState->seekerPos.toString()<<endl;
        ok = false;
    }
    //check if poses are ok
    if (!_map->isPosInMap(hsNextState->hiderPos)) {
        cout << "HSSimulator::checkNextStateObs: ERROR next hider pos is not in map of state: " << hsNextState->hiderPos.toString()<<endl;
        ok = false;
    }

    //observation
    const HSState* hsObs = static_cast<const HSState*>(obs);

    //check if seeker pos consist with obs

    if (!hsObs->hiderVisible()) { //obs == _hiddenObs) {
         //NOTE: we actually expect the next state of the hider to be set!!! Only the observation could be 'not set'
            // remember that we are using Monte Carlo in which locations are 'randomly' chosen in the case of not knowing
    } else {
        //check if obs pos is same as next pos of hider
        //Pos hiderObsPos = _map->getCoordFromIndex(obs);
        //if (!hsNextState->hiderPos.equals(hiderObsPos)) {
        if (!hsNextState->hiderPos.equals(hsObs->hiderPos)) {
            cout << "HSSimulator::checkNextStateObs: ERROR: hider pos was " <<  (hsState!=NULL?hsState->hiderPos.toString():"<not set>")
                 << ", now is " << hsNextState->hiderPos.toString() << " but observation is "<<hsObs->hiderPos.toString()<<endl;
            ok = false;
        }
    }

    return ok;
}


bool HSSimulator::isStateConsistentWithObs(const State *state, const State *obs, const State* obs2) {
    //states should be passed
    assert(state!=NULL);
    assert(obs!=NULL);
    bool ok = true;

    //cast to hs state
    const HSState* hsState = static_cast<const HSState*>(state);
    const HSState* hsObs = static_cast<const HSState*>(obs);


    //states should contain seeker+hider pos
    assert(hsState->seekerPos.isSet());
    assert(hsState->hiderPos.isSet());
    assert(hsObs->seekerPos.isSet());


    if (!hsState->seekerPos.equalsInt(hsObs->seekerPos)) {
        //seeker pos not equal to obs
        DEBUG_POMCP_SIM(cout <<"HSSimulator::isStateConsistentWithObs: inconsistent seeker pos, state:"
                        <<hsState->seekerPos.toString()<<",obs: "<< hsObs->seekerPos.toString() <<endl;);
        ok = false;

    } else {
        //check obs visib
        bool visib = _map->isVisible(hsState->seekerPos,hsState->hiderPos,_params->takeDynObstOcclusionIntoAccountWhenLearning);

        if (hsObs->hiderVisible() != visib) {
            //hider pos visibility (of belief) should be consistent with observation: visible or not
            ok = false;
            DEBUG_POMCP_SIM(cout <<"HSSimulator::isStateConsistentWithObs: hider pos should not be visible in obs, state:"
                            <<hsState->toString() <<endl;);

        } else if ( hsObs->hiderVisible() && !hsObs->hiderPos.equalsInt(hsState->hiderPos) ) {
            //if hider is visible, the pos should be the same
            //ag131021: found bug! hsObs->hiderPos.isSet() was missing!!
            ok = false;
            DEBUG_POMCP_SIM(cout <<"HSSimulator::isStateConsistentWithObs: hider pos should be visible in obs, but they are not equal, state:"
                            <<hsState->hiderPos.toString() << ", obs: "<<hsObs->hiderPos.toString() <<endl;);
        }

        //AG150211: now check second observation
        if (ok && obs2!=NULL) {
            const HSState* hsObs2 = static_cast<const HSState*>(obs2);
            //check visib for seeker 2
            visib = _map->isVisible(hsObs2->seekerPos,hsState->hiderPos,_params->takeDynObstOcclusionIntoAccountWhenLearning);

            if (hsObs2->hiderVisible() != visib) {
                //hider pos visibility (of belief) should be consistent with observation of 2nd seeker: visible or not
                ok = false;
                DEBUG_POMCP_SIM(cout <<"HSSimulator::isStateConsistentWithObs: hider pos should not be visible in obs, state:"
                                <<hsState->toString()<<", as seen from obs2: "<<hsObs2->toString() <<endl;);

            } else if ( hsObs2->hiderVisible() && !hsObs2->hiderPos.equalsInt(hsState->hiderPos) ) {
                //if hider is visible, the pos should be the same
                ok = false;
                DEBUG_POMCP_SIM(cout <<"HSSimulator::isStateConsistentWithObs: hider pos should be visible in obs, but they are not equal to 2nd seeker, state:"
                                <<hsObs2->hiderPos.toString() << ", obs: "<<hsObs->hiderPos.toString() <<endl;);
            }
        }
    }

    return ok;
}


int HSSimulator::getActionForRollout(const State *state, History *history) {
    int a = -1;

    switch(_params->rolloutPolicyType) {
        case SeekerHSParams::ROLL_OUT_POLICY_TYPE_RANDOM: {
            //random action
            vector<int> actions;
            getActions(state, history, actions);

            //choose random action
            a = actions[random(actions.size()-1)];

            break;
        }
        case SeekerHSParams::ROLL_OUT_POLICY_TYPE_SMART: {
            const HSState* hsState = static_cast<const HSState*>(state);

            //since this is for the roll-out (and simulation), we can always say that the opponent is visible (?)
            //(or does this give an error?)
            bool visib = true;

            //smart action
            if (_smartSeeker==NULL) {
                DEBUG_CLIENT(cout<<"Using SmartSeeker for roll-out policy"<<endl;);
                _smartSeeker = new SmartSeeker(_params);
                _smartSeeker->initBelief(_map, hsState->seekerPos, hsState->hiderPos, visib);
            }
            //get actions from smart seeker
            a = _smartSeeker->getNextAction(hsState->seekerPos, hsState->hiderPos, visib);

            break;
        }
        default:
            cout << "ERROR: unknown roll-out policy!" << endl;
            break;
    }


    return a;
}


void HSSimulator::resetParams() {
    cout<<"HSSimulator::resetParams()"<<endl;
}

void HSSimulator::testAllFunctions() {
    cout << " HSSimulator::testAllFunctions()"<<endl;


/*    cout <<"Test of HSSimulatorCont"<<endl<<endl;
    cout << "Map size: "<<_map->rowCount()<<"x"<<_map->colCount()<<endl;
    _map->printMap();

    State* obs1;
    double r1;
    pomcp::HSState hsstate1(0,0,0,0);
    cout << " in: "<<hsstate1.toString()<<endl;

    pomcp::State* nextS1 = step(&hsstate1,HSGlobalData::ACT_SE,obs1,r1);

    cout <<" out after SE action: "<<nextS1->toString()<< ", obs="<<obs1->toString()<<", r="<<r1<<endl;


    //_map->setUsingContActions(false);

    cout << " in: "<<hsstate1.toString()<<endl;
    nextS1 = step(&hsstate1,HSGlobalData::ACT_S,obs1,r1);

        cout <<" out after S action: "<<nextS1->toString()<< ", obs="<<obs1->toString()<<", r="<<r1<<endl;
    return ;*/

    //Player player(&gmap);

    Timer t;
    int sTime = t.startTimer();
    long c=0;
    //Pos x, y;
    int xRow,xCol,yRow,yCol;/*,x2Row,x2Col;


    for(xRow=0; xRow<_map->rowCount(); xRow++) {
        for(xCol=0; xCol<_map->colCount(); xCol++) {
            if (!_map->isObstacle(xRow,xCol)) {

                //calc time
                int tid=t.startTimer();
                for(yRow=0; yRow<_map->rowCount(); yRow++) {
                    for(yCol=0; yCol<_map->colCount(); yCol++) {


                        for(x2Row=0; x2Row<_map->rowCount(); x2Row++) {
                            for(x2Col=0; x2Col<_map->colCount(); x2Col++) {
                                if (!_map->isObstacle(x2Row,x2Col)) {
                                    cout << "***** x/seeker: r"<<xRow<<"c"<<xCol<<" - x2/seeker2: r"<<x2Row<<"c"<<x2Col <<" - y/hider: r"<<yRow<<"c"<<yCol<<" *****"<<endl;
                                    cout << " distance: " << _map->distance(xRow,xCol,yRow,yCol)<<endl;


                                    cout << " -- genInitState --"<<endl;
                                    bool v = _map->isVisible(xRow,xCol,yRow,yCol,_params->takeDynObstOcclusionIntoAccountWhenLearning);
                                    cout << "visiblity: (s->h): "<<v<<endl;
                                    v = _map->isVisible(x2Row,x2Col,yRow,yCol,_params->takeDynObstOcclusionIntoAccountWhenLearning);
                                   cout << "visiblity: (s2->h): "<<v<<endl;
                                    cout<<"10 init states: ";

                                    //AG131104: use obs
                                    //setSeekerHiderPos(x,y,v);
                                    pomcp::HSState obs(xRow,xCol,yRow,yCol);

                                    if (!v) obs.hiderPos.clear();

                                    FOR(i,10) {
                                        pomcp::State* s = genInitState(&obs);
                                        cout << s->toString()<<" ";
                                        delete s;
                                    }
                                    cout << endl;

                                    cout << " -- genAllInitStates --"<<_params->numBeliefStates<<endl;
                                    vector<State*> allInitStates = genAllInitStates(&obs, _params->numBeliefStates);
                                    cout << "#"<<allInitStates.size()<<": ";
                                    for (vector<State*>::iterator it = allInitStates.begin(); it != allInitStates.end(); it++) {
                                        cout << (*it)->toString()<<" ";
                                        //delete *it;
                                    }
                                    cout << endl;


                                }
                            }
                        }



                    }
                }
            }
        }
    }

    return;*/



    for(xRow=0; xRow<_map->rowCount(); xRow++) {
        for(xCol=0; xCol<_map->colCount(); xCol++) {
            if (!_map->isObstacle(xRow,xCol)) {

                //calc time
                int tid=t.startTimer();
                for(yRow=0; yRow<_map->rowCount(); yRow++) {
                    for(yCol=0; yCol<_map->colCount(); yCol++) {

                        if (!_map->isObstacle(yRow,yCol)) {
                            cout << "***** x/seeker: r"<<xRow<<"c"<<xCol<<" - y/hider: r"<<yRow<<"c"<<yCol<<" *****"<<endl;
                            cout << " distance: " << _map->distance(xRow,xCol,yRow,yCol)<<endl;

                            //_map->printMap(xRow,xCol);

                            //cout << "Old isVisible: "<< _map->isVisible(y.x,y.y)<<endl;
                            cout << "New isVisible: "<< _map->isVisible(xRow,xCol,yRow,yCol,_params->takeDynObstOcclusionIntoAccountWhenLearning)<<endl;

                            cout << "Invisible from pos 1:"<<endl;
                            vector<Pos> invisPosVec = _map->getInvisiblePoints(xRow,xCol,_params->takeDynObstOcclusionIntoAccountWhenLearning);
                            FOR(i,invisPosVec.size()) {
                                cout << " "<<i<<"] " << invisPosVec[i].toString() <<endl;
                            }
                            cout <<endl;

                            cout << " -- genInitState --"<<endl;
                            bool v = _map->isVisible(xRow,xCol,yRow,yCol,_params->takeDynObstOcclusionIntoAccountWhenLearning);
                            cout << "visiblity: (s->h): "<<v<<endl;
                            cout<<"10 init states: ";

                            //AG131104: use obs
                            //setSeekerHiderPos(x,y,v);
                            pomcp::HSState obs(xRow,xCol,yRow,yCol);
                            if (!v) obs.hiderPos.clear();

                            FOR(i,10) {
                                pomcp::State* s = genInitState(&obs);
                                cout << s->toString()<<" ";
                                delete s;
                            }
                            cout << endl;

                            cout << " -- genAllInitStates --"<<_params->numBeliefStates<<endl;
                            vector<State*> allInitStates = genAllInitStates(&obs, _params->numBeliefStates);
                            cout << "#"<<allInitStates.size()<<": ";
                            for (vector<State*>::iterator it = allInitStates.begin(); it != allInitStates.end(); it++) {
                                cout << (*it)->toString()<<" ";
                                //delete *it;
                            }
                            cout << endl;

                            //cout << " -- genRandomState --"<<endl;
                            pomcp::HSState hsstate(xRow,xCol,yRow,yCol);
                            /*cout<<"10 next random states from "<< hsstate.toString() <<": ";
                            //setSeekerHiderPos(x,y,v);
                            FOR(i,10) {
                                pomcp::State* s = genRandomState(&hsstate,NULL);
                                cout << s->toString()<<" ";
                                delete s;
                            }
                            cout << endl;*/


                            cout << " -- getActions --"<<endl;
                            cout<<"possible actions from "<< hsstate.toString() <<": ";
                            vector<int> actVec;
                            getActions(&hsstate,NULL,actVec);
                            FOREACH(int,a,actVec) {
                                cout << ACTION_COUT(*a)<<" ";
                            }
                            cout << endl;


                            cout << " -- getPossibleNextPos --"<<endl;
                            cout<<"possible pos from r"<< xRow<<"c"<<xCol <<": ";
                            vector<Pos> posVec;
                            Pos x(xRow,xCol);
                            getPossibleNextHiderPos(x,posVec);
                            //FOREACHnc(Pos,p,posVec) {
                            for(vector<Pos>::iterator it = posVec.begin(); it != posVec.end(); it++) {
                                cout << it->toString()<<" ";
                            }
                            cout << endl;


                            cout << " -- setInitialNodeValue --"<<endl;
                            cout<<"set init node value from "<< hsstate.toString() <<": "<<flush;
                            pomcp::Node node(this,NULL);
                            for(int a=-1; a<HSGlobalData::NUM_ACTIONS; a++) {
                                cout << " a="<<flush;
                                if (a<0)
                                    cout <<"none"<<flush;
                                else
                                    cout<<ACTION_COUT(a)<<flush;
                                setInitialNodeValue(&hsstate,NULL,&node,a);   //(x,posVec);

                                cout <<" count="<<node.getCount()<<" v="<<node.getValue()<<"; "<<flush;
                            }
                            cout << endl;


                            cout << " -- step --"<<endl;
                            cout<<"step from "<< hsstate.toString() <<": "<<flush;
                            for(int a=0; a<HSGlobalData::NUM_ACTIONS; a++) {
                                double r=0;
                                //int o=-1;
                                //HSState obs;
                                cout << " a="<<ACTION_COUT(a)<<"->next="<<flush;
                                pomcp::State* obs = NULL;
                                pomcp::State* nextS = step(&hsstate,a,obs,r);
                                cout<<nextS->toString()<<",o="<<(obs==NULL?"NULL":obs->toString())<<",r="<<r<<"; ";
                                delete nextS;
                                delete obs;
                            }
                            cout << endl;


                            cout << " -- step from init states --"<<endl;
                            for (vector<State*>::iterator it = allInitStates.begin(); it != allInitStates.end(); it++) {
                                cout<<"step from "<< (*it)->toString() <<": "<<flush;

                                double r=0;
                                int a = random(8);
                                //int o=-1;
                                //HSState obs;
                                cout << " a="<<ACTION_COUT(a)<<"->next="<<flush;
                                pomcp::State* obs = NULL;
                                pomcp::State* nextS = step(*it,a,obs,r);
                                cout<<nextS->toString()<<",o="<<(obs==NULL?"NULL":obs->toString())<<",r="<<r<<"; ";
                                delete nextS;
                                delete obs;
                                delete *it;
                            }
                            cout << endl;


                            cout << " -- isFinal --"<<endl;
                            cout << hsstate.toString()<<": "<< isFinal(&hsstate)<<endl;

                            cout << "-- immediateReward --"<<endl;
                            cout << hsstate.toString()<<": "<< getImmediateReward(&hsstate)<<endl;



                        } // if y!=obs
                    } // for yCol

                } //for yRow

                c++;
                long runT = t.stopTimer(tid);
                cout << "time field: "<<runT<<endl;
                long iLeft = _map->numFreeCells()-c;
                cout << "Itts left: "<<iLeft<<"; time: "<<(iLeft*runT)<<" s = "<<(iLeft*runT/3600.0)<<" h"<<endl;
            } //if x!=obst
        } //for xCol

    } //for xRow

    long totTime = t.getTime(sTime);
    cout << "TOTAL TIME: "<<totTime<<" s = "<<(totTime/3600.0)<<" h"<<endl<<endl;
}




