#include "POMCP/hssimulator.h"
#include "POMCP/hsstate.h"
#include "POMCP/hsobservation.h"

#include "Base/hsglobaldata.h"
#include "Base/seekerhs.h"
#include "Base/hsconfig.h"

#include "Utils/generic.h"
//#include "Utils/timer.h"

#include "Smart/smartseeker.h"

//iriutils
#include "exceptions.h"

#include <cassert>
#include <cmath>
#include <vector>
#include <random>
#include <iostream>


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

HSSimulator::HSSimulator(GMap *map, SeekerHSParams* params) : _randomGenerator(_randomDevice()),
        _uniformProbDistr(0,1), _uniformDirDistr(0,2*M_PI) {

    assert(map!=NULL);
    _map = map;

    //max value for triangle reward
    _maxRewValue = map->colCount() * map->rowCount();
    //hidden obs (since observations are id-ed by state, the one after last is 'unknown' obs - consistent with POMDP impl)
    //_hiddenObs = map->numFreeCells();
    _basePos = map->getBase();
    _params = params;

    _player.setMap(map);

    //number of free cells + 1 (unknown)
    //ag130831: observations are ALL possible combis of position + not visible
    //      in practice a lot won't be used since certain seeker-hider positions
    //      because the hider won't be visible from the seeker's pos
    _numObs = (map->numFreeCells() + 1) * map->numFreeCells();

    //AG140108: only when really used
    _smartSeeker = NULL;
    _smartSeekerHiderPlayer = NULL;

    if (_params->rewardType==SeekerHSParams::REWARD_NOT_SET) {
        throw CException(_HERE_,"reward type has not been set");
    }
    if (_params->hiderStepDistance<=0) {
        throw CException(_HERE_, "the hider step distance should be more than 0");
    }

    //AG140909: init the vars
    //resetParams();
}

HSSimulator::~HSSimulator() {
    if (_smartSeeker!=NULL) {
        delete _smartSeeker;
        delete _smartSeekerHiderPlayer;
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

HSState* HSSimulator::genRandomHSState(const HSObservation* hsObs, const std::vector<Pos>& invisPosVec,
                                const std::vector<Pos>& seekerPosVec) {
    //get random state
    HSState* state = hsObs->getRandomState(_uniformProbDistr(_randomGenerator));

    //AG150703: set the seeker pos in the state (otherwise could be other seeker pos)
    state->seekerPos = hsObs->ownSeekerObs.seekerPos;

    //int src=0;

    if (!state->hiderVisible()) {
        //find a state that is consistent with the seeker poses
        //src = 1;

#ifdef OLD_CODE
        if (invisPosVec.empty()) {
            //generate random pos
            state->hiderPos = _map->genRandomPos();
        } else {
            //set a hidden hider pos
            state->hiderPos  = invisPosVec[hsutils::random(invisPosVec.size()-1)];
        }
#endif
        unsigned int c = 0;
        do {
            //get random pos
            state->hiderPos = _map->genRandomPos();
            //probability that one of the seekers sees the hider
            double pVisib = _map->getVisibilityProb(seekerPosVec, state->hiderPos, _params->takeDynObstOcclusionIntoAccountWhenLearning,
                                                      _params->useDynObstForVisibCheck?&hsObs->dynObstVec:nullptr);
            //now check if it is visible
            if (pVisib==0 || _uniformProbDistr(_randomGenerator)>pVisib) {
                //found
                //src=2;
                break;
            }
            //AG160226 TODO: could add pos anyway if random>pVisib

            //increment counter
            ++c;
        } while(c<_params->MAX_NUM_TRIES_AFTER_FAILURE);

        //if still not set, use another
        if (!state->hiderPos.isSet()) {
            if (invisPosVec.empty()) {
                //generate random pos
                state->hiderPos = _map->genRandomPos();
                //src=3;
            } else {
                //set a hidden hider pos
                state->hiderPos  = invisPosVec[hsutils::random(invisPosVec.size()-1)];
                //src=4;
            }
        }
    }

    //AG150125: set a heading
    if (_params->useHeading) {
        //set random heading
        state->hiderPos.setHeading(_uniformDirDistr(_randomGenerator));
        assert(state->hiderPos.hasHeading());
    }

    // AG: TMP
    /*if (!isStateConsistentWithObs(state, hsObs)) {
        cout << "HSSimulator::genRandomHSState: INCONSISTENT state "<<state->toString()<<" with obs="<<hsObs->toString()<<" set in src="<<src<<endl;
    }
    else {
        cout << "HSSimulator::genRandomHSState: consistent state "<<state->toString()<<" with obs="<<hsObs->toString()<<" set in src="<<src<<endl;
    }*/

    return state;
}

State* HSSimulator::genInitState(const Observation *obs) {
    DEBUG_POMCP(cout<<"HSSimulator::genInitState: ";);
    assert(obs!=NULL);

    //AG131104: the seeker pos should be passed, and hider pos if visible
    //generate obs
    const HSObservation* hsObs = HSObservation::castFromObservation(obs);

    //check invisible poses (only in case it doesn't work)
    vector<Pos> invisPosVec = _map->getInvisiblePoints(hsObs->getSeekerPoses(),_params->takeDynObstOcclusionIntoAccountWhenLearning,_params->simNotVisibDist,
                                                      _params->useDynObstForVisibCheck?&obs->dynObstVec:nullptr);

    //the positions of the seeker
    vector<Pos> seekerPoses = hsObs->getSeekerPoses();

    //AG160213 NOTE: passing these vectors is a bit of a overhead, but only done once,
    //	and method is much more efficient for genAllInitStates

    //get a random state of the observation
    return genRandomHSState(hsObs, invisPosVec, seekerPoses);
}

std::vector<State*> HSSimulator::genAllInitStates(const Observation *obs, int n) {

    DEBUG_POMCP(cout<<"HSSimulator::genAllInitStates ("<<n <<"): "<<flush;);
    assert(obs!=NULL);
    assert(n>0);

    //return vector
    vector<State*> allStatesVec;

    //AG131104: the seeker pos should be passed, and hider pos if visible
    const HSObservation* hsObs = HSObservation::castFromObservation(obs);

    //AG160211: use GMap::getVisibilityProb instead of getInvisiblePoints, since it will take into account max visib distance
    //check invisible poses (only in case it doesnt work
    vector<Pos> invisPosVec = _map->getInvisiblePoints(hsObs->getSeekerPoses(),_params->takeDynObstOcclusionIntoAccountWhenLearning,_params->simNotVisibDist,
                                                      _params->useDynObstForVisibCheck?&obs->dynObstVec:nullptr);

    //the positions of the seeker
    vector<Pos> seekerPoses = hsObs->getSeekerPoses();

    //generate list
    for(int i=0; i<n; ++i) {
        //generate random state
        HSState* state = genRandomHSState(hsObs, invisPosVec, seekerPoses);

        //add to list
        allStatesVec.push_back(state);
    }

    assert((int)allStatesVec.size()==n);

    return allStatesVec;
}

void HSSimulator::getActions(const State* state, History* history, std::vector<int>& actions, bool smart) {
    assert(state!=NULL);
    const HSState* hsstate = HSState::castFromState(state);

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

void HSSimulator::getPossibleNextHiderPos(const Pos &prevPos, std::vector<Pos> &posVec, const HSObservation *obs) {

    //should only be here if hiderpos is not set or no observation passed
    //AG150803: not valid, it can occur that a not seen obs is chosen (of another seeker)
    //while p_fp==0 && obs!=NULL
    //assert(_params->contFalsePosProb>0 || obs==NULL || !obs->ownSeekerObs.hiderPos.isSet()); //AG150126: added obs2Prob>0

    //AG150126 NOTE: for 2 seekers this should work, since only 1 observation of hider can be passed (should be consistent, if not choice
    //               should be made before.

    //for consistency check
    HSState* state = NULL;
    if (obs!=NULL) {
        //AG150122: added cast
        state = HSState::castFromState(obs->ownSeekerObs.copy());
        assert(state!=NULL);
        assert(state->seekerPos.isSet());
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
                if (_params->pomcpCheckNewBeliefGen && !isStateConsistentWithObs(state, obs)) {
                    //state is inconsistent
                    //AG150319: check if allow inconsistent states
                    if ( !(_params->pomcpSimType==SeekerHSParams::POMCP_SIM_CONT && _params->contUpdInconsistAcceptProb>0 &&
                                                _uniformProbDistr(_randomGenerator)<_params->contUpdInconsistAcceptProb) ) {
                        //do not accept state
                        continue;
                    } // else: accept (incorrect positive) even though not consistent
                }
            }

            posVec.push_back(newPos);
        }
    }

    if (state!=NULL) delete state;

    //if no observation check is done, then at least one new position must be received (from action halt)
    assert(obs!=NULL || !posVec.empty());
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

State* HSSimulator::step(const State *state, int action, State *&obsOut, double &reward, bool genOutObs, const Observation *obs) {
 //(const State *state, int action, State*& obsOut, double &reward, bool genOutObs, const State* obsIn,const State *obs2In, double obs1p) {

    //AG150615:   Slightly changed, now if obs is set then choose a random observation, if it has a visible hider pos, then
    //          use this as hider pos, otherwise choose a random step.
    //            Realistic because: choosing obs depends on the probability of the thrustworthyness (stored in HSState.prob)
    //          thus more probable (i.e. trustworthy) observations are chosen with higher probability. Although the observation might
    //          be 'hider not visible', these are then following the previous state, but the 'visible' states are then chosen on that
    //          place giving it a higher probability in the belief state.

    assert(state!=NULL);
    assert(action>=0 && action<HSGlobalData::NUM_ACTIONS);

    const HSState* hsstate = HSState::castFromState(state);
    //state has to have valid positions, because we simulate a real state (not an observation)
    assert(hsstate->seekerPos.isSet());
    assert(hsstate->hiderPos.isSet());

    //output state
    HSState* nextState = new HSState();

    //try to use observation
    const HSObservation* hsObsIn = HSObservation::castFromObservation(obs);

    //store previous heading
    double prevHeading = hsstate->hiderPos.heading();
    assert(!_params->useHeading || (prevHeading>=0 && prevHeading <= 2.0*M_PI));

    //AG160317: first check seeker
    if (obs!=nullptr) {
        //set seeker state
        nextState->seekerPos = hsObsIn->ownSeekerObs.seekerPos;
    } else {
        nextState->seekerPos = _map->tryMove(action, hsstate->seekerPos);
        if (!nextState->seekerPos.isSet()) {
            //action not possible, so same position
            nextState->seekerPos = hsstate->seekerPos;
        }
   }

    //check if to use the observation OR don't use the obs with the probability p_fp  (TODO: should this be another, false negative???)
    if (obs!=NULL && (_params->contFalsePosProb==0 || _uniformProbDistr(_randomGenerator) > _params->contFalsePosProb)) {
        //use the observation
        //choose a random state of the observations
        HSState* randState = hsObsIn->getRandomState(_uniformProbDistr(_randomGenerator));
        assert(randState!=NULL);

        if (randState->hiderVisible()) {
            //the chosen hider obs is visible, so use it
            nextState->hiderPos = randState->hiderPos;
            //TODO: could deduce orientation
        }

        //AG150826: delete it
        delete randState;
    } // else {   // no obs, or false positive    }

    //at least the seekerpos should be set
    assert(nextState->seekerPos.isSet());

    //if hiderPos not (yet) set
    if (!nextState->hiderPos.isSet()) {
        //move hider
        vector<Pos> posVec;
        //get next hider positions possible, taking into account the observations (if given)
        //ERROR!! AG160322: possible next states not found (IF OBS is set)
        getPossibleNextHiderPos(hsstate->hiderPos, posVec, hsObsIn);


        if (obs!=NULL && posVec.empty()) {
            //DEBUG_POMCP/*_DETAIL*/(cout << "HSSimulator::step: no consistent obs="<< hsObsIn->toString() <<",state="<<hsstate->toString()<<" "<<endl;);
            //getPossibleNextPos(hsstate->hiderPos, posVec);
            //NO consistent state found with the observation, this happens when the observation is 'not visible'
            //and of the chosen state all next hider positions are visible. Thus when it took a state that was not visible in the
            //previous state, but is now, it might be that all its neighbors also are visible, therefore no consistent new state
            //can be found.

            //delete result state
            delete nextState;

            return NULL;
        }
        assert(!posVec.empty());

        //choose next hider pos fromt the possible next hider position
        if (!_params->useContinuousPos && posVec.size()==1) {
            nextState->hiderPos = posVec[0];

        } else {
            //AG131022: add a check for use of random action
            //use the action strategy (if prob=0 -> always use strategy, otherwise chance of using only random)
            bool randMove = (_params->simHiderType==SeekerHSParams::SIM_HIDER_TYPE_RANDOM ||
                             _uniformProbDistr(_randomGenerator) < _params->simHiderRandomActProb );

            if (randMove) {
                if (_params->useContinuousPos) {
                    //generate a random pos at the hiderpos distance, and check if it is inside
                    //a legal next pos
                    bool ok = false;
                    unsigned int tries = 0;

                    //set
                    double ranDir = -1.0;
                    //AG150125: if a hider has heading, use that one, otherwise random
                    if (_params->useHeading && prevHeading>=0)
                        ranDir = prevHeading;

                    do {
                        //move in random dir
                        if (ranDir<0)
                            ranDir = _uniformDirDistr(_randomGenerator);

                        //assert(ranDir>=0 && ranDir<2*M_PI);
                        nextState->hiderPos = _map->tryMoveDir(ranDir, hsstate->hiderPos, _params->hiderStepDistance, true);
                        if (nextState->hiderPos.isSet()) {
                            //now check if it is a consistent next state, comparing them to the possible next states
                            for(const Pos& pos : posVec) {

                                if (pos.equalsInt(nextState->hiderPos)) {
                                    ok = true;

                                    if (_params->useHeading) //AG160125: set previous hider heading, since later on we add noise
                                        prevHeading = ranDir;

                                    break;
                                }
                            }
                        }
                        //increase tries
                        tries++;
                        //reset ranDir
                        ranDir = -1.0;
                    } while (tries<_params->pomcpSimMaxNumTriesHiderPos && !ok );

                    if (!ok) {
                        DEBUG_CLIENT(cout<<"WARNING: failed to simulate correct new hiderpos for "<<hsstate->hiderPos.toString()<<" after "<<tries<<" tries."<<endl;);
                        //use a discrete state
                        nextState->hiderPos = posVec[hsutils::random(posVec.size()-1)];
                        if (!nextState->hiderPos.hasDouble())
                            nextState->hiderPos.add(0.5,0.5);
                    } /*else {//with noise the state can be inconsistent
                        assert(nextState->hiderPos.isSet());
                        assert(obs==NULL || isStateConsistentWithObs(nextState,obs));
                    }*/

                } else {
                    //discrete random move
                    nextState->hiderPos = posVec[hsutils::random(posVec.size()-1)];
                }
            } else {
                //ag131017: check if using smart or random movement
                switch (_params->simHiderType) {
                /*case SeekerHSParams::SIM_HIDER_TYPE_RANDOM:
                    nextState->hiderPos = posVec[random(posVec.size()-1)];
                    break;*/
                case SeekerHSParams::SIM_HIDER_TYPE_SMART:
                    cout << "SIM_HIDER_TYPE_SMART not implemented"<<endl;
                    //AG150616: could make simulated movement more realistic:
                    //  going to random goals (like walker) or have higher prob. going same dir.

                case SeekerHSParams::SIM_HIDER_TYPE_TOBASE: {
                    //find shortest distance pos
                    nextState->hiderPos = findClosestPos(posVec, _basePos) ;
                    break;
                }
                default:
                    throw CException(_HERE_,"unknown simulation hider type");
                }
            }
        }

    } //else // if !newHiderPos.isSet

    assert(nextState->hiderPos.isSet());

    //AG160125: set the heading (with noise)
    if (_params->useHeading) {
        assert(prevHeading>=0 && prevHeading <= 2.0*M_PI);
        nextState->hiderPos.setHeading(prevHeading);
    }

    if (genOutObs) {
        //generate out observation
        HSState* hsObsOut = HSState::castFromState(nextState->copy());
        obsOut = hsObsOut;

        //AG160504: add dyn obst. vec
        vector<IDPos> dynObstVec;
        if (obs!=nullptr && _params->useDynObstForVisibCheck)
            dynObstVec = obs->dynObstVec;

        //check if the hider should be visible
        bool isVisib = _map->isVisible(nextState->seekerPos, nextState->hiderPos, _params->takeDynObstOcclusionIntoAccountWhenLearning, true, &dynObstVec);
        if (!isVisib && _params->contObserveIfNotVisibProb > 0) {
            if (_uniformProbDistr(_randomGenerator) <= _params->contObserveIfNotVisibProb) {
                //make it visible even though raytrace says it isn't
                isVisib = true;
            }
        } else if (isVisib && _params->simNotVisibDist) {
            //AG160127: if is visible, might be invisible due to distance
            /*double d = nextState->seekerPos.distanceEuc(nextState->hiderPos);
            double pVisib = _params->simNotVisibDist_y0;
            if (d > _params->simNotVisibDist_x0)
                pVisib += _params->simNotVisibDist_k0*(d-_params->simNotVisibDist_x0);*/
            //AG160210: now in gmap
            double pVisib = _map->getVisibilityProb(nextState->seekerPos, nextState->hiderPos, true);

            if (pVisib>0) {
                //probabililty of being seen depends on distance
                double p = _uniformProbDistr(_randomGenerator);
                if (p>pVisib) {
                    //DEBUG_CLIENT(cout<<"not visible due to distance prob. (p="<<pVisib<<")";);
                    //set false negative
                    isVisib = false;
                }
            } else {
                // too far, so not visible
                isVisib = false;
            }
        }

        //now decide observation
        if (!isVisib) {
            hsObsOut->hiderPos.clear();
        }
    }

    //calculate the reward
    reward = getImmediateReward(state, nextState);

    return nextState;
}

Pos HSSimulator::findClosestPos(const vector<Pos>& posVec, const Pos& toPos) {
    assert(!posVec.empty());

    double minDist = HSGlobalData::INFTY_POS_FLT;
    vector<Pos> minPosvec;

    //check minimum distance
    for(const Pos& pos : posVec) {
        double dist = _map->distance(pos,toPos);

        if (dist<=minDist) {
            if (dist<minDist) { //we found the smallest, so clear the vector
                minPosvec.clear();
                minDist = dist;
            }
            minPosvec.push_back(pos);
        }
    }

    assert(!minPosvec.empty());

    if (minPosvec.size()==1) {
        return minPosvec[0];
    } else {
        return minPosvec[hsutils::random(minPosvec.size()-1)];
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
    const HSState* hsstate = HSState::castFromState(state);
    const HSState* hsnextstate = HSState::castFromState(nextState);

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
            break;
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


bool HSSimulator::checkNextStateObs(const State *state, const State *nextState, const State* obs) {
    /*if (nextState == NULL) {
        //cout << "HSSimulator::checkNextStateObs: expected a next state!"<<endl;
        throw CException(_HERE_,"a next state has to be passed"); //AG150731: next state has to be passed
        return false;
    }*/
    assert(nextState!=NULL);//AG150731: next state has to be passed

    const HSState* hsState = HSState::castFromState(state);
    const HSState* hsNextState = HSState::castFromState(nextState);

    //AG150731: states have to have both a seeker and hider pos
    assert(hsNextState!=NULL);
    assert(hsNextState->seekerPos.isSet());
    assert(hsNextState->hiderPos.isSet());

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
    const HSState* hsObs = HSState::castFromState(obs);

    //check if seeker pos consist with obs

    if (!hsObs->hiderVisible()) { //obs == _hiddenObs) {
         //NOTE: we actually expect the next state of the hider to be set!!! Only the observation could be 'not set'
            // remember that we are using Monte Carlo in which locations are 'randomly' chosen in the case of not knowing
    } else {
        //check if obs pos is same as next pos of hider
        //Pos hiderObsPos = _map->getCoordFromIndex(obs);
        //if (!hsNextState->hiderPos.equals(hiderObsPos)) {
        if ( (!_params->useContinuousPos && !hsNextState->hiderPos.equals(hsObs->hiderPos)) ||
             (_params->useContinuousPos && hsNextState->hiderPos.distanceEuc(hsObs->hiderPos)>1.0 )) {

            cout << "HSSimulator::checkNextStateObs: ERROR: hider pos was " <<  (hsState!=NULL?hsState->hiderPos.toString():"<not set>")
                 << ", now is " << hsNextState->hiderPos.toString() << " but observation is "<<hsObs->hiderPos.toString()<<endl;
            ok = false;
        }
    }

    return ok;
}


//TODO/NOTE: in the future we might have states that are not consistent with observation due to uncertainty
bool HSSimulator::checkNextStateObs(const State *state, const State *nextState, const Observation *obs)  {
    //(const State *state, const State *nextState, const State* obs) {

    return isStateConsistentWithObs(nextState, obs);
}


bool HSSimulator::isStateConsistentWithObs(const State *state, const Observation *obs)  {
    //AG160213: use HSSimulatorCont, this function has to be improved taking into account the
    // max visib. distance
    /*throw CException(_HERE_, "TODO: this function has to be updated to take into account max. visib. distance, use HSSimulatorCont");
    return false;
#ifdef TO_FIX*/

    //(const State *state, const State *obs, const State* obs2) {
    //note here int is used since the states are assumed to be discrete

    //states should be passed
    assert(state!=NULL);
    assert(obs!=NULL);
    bool ok = true;

    //cast to hs state
    const HSState* hsState = HSState::castFromState(state);
    const HSObservation* hsObs = HSObservation::castFromObservation(obs);

    //states should contain seeker+hider pos
    assert(hsState->seekerPos.isSet());
    assert(hsState->hiderPos.isSet());
    assert(_map->isPosInMap(hsState->seekerPos));
    assert(_map->isPosInMap(hsState->hiderPos));

    //AG150615: check for multi robot
    //first check if seeker Pos the same
    if (!hsObs->ownSeekerObs.seekerPos.equalsInt(hsState->seekerPos)) {
        DEBUG_POMCP_SIM(cout<<"HSSimulator::isStateConsistentWithObs: the seeker position (" << hsState->toString()
                            <<") is not the same of the state and obs.ownSeekerPos ("<<(hsObs->ownSeekerObs.toString())<<")"<<endl;);
        ok = false;
    } else {
        //now check the hider pose

        //check if hider should be visible
        bool visib = _map->isVisible(hsState->seekerPos, hsState->hiderPos, _params->takeDynObstOcclusionIntoAccountWhenLearning, false);

        if (hsObs->ownSeekerObs.hiderVisible()!=visib) {
            //hider pos visibility (of belief) should be consistent with observation: visible or not
            ok = false;
            DEBUG_POMCP_SIM(cout <<"HSSimulator::isStateConsistentWithObs: hider pos should not be visible in obs, state:"
                            <<hsState->toString() <<endl;);
        } else if (visib && !hsObs->ownSeekerObs.hiderPos.equalsInt(hsState->hiderPos)) {
            //if hider is visible, the pos should be the same
            ok = false;
            DEBUG_POMCP_SIM(cout <<"HSSimulator::isStateConsistentWithObs: hider pos should be visible in obs, but they are not equal to 2nd seeker, state:"
                            <<hsState->toString()<< ", obs: "<<hsObs->ownSeekerObs.hiderPos.toString() <<endl;);
        } else {
            //now check the other observations
            for(size_t i=1; i<hsObs->size() && ok; i++) {
                const HSState* hsOthObsState = hsObs->getObservationState(i);

                //hider pos should be same if there is an observation
                if (hsOthObsState->hiderVisible() && !hsOthObsState->hiderPos.equalsInt(hsState->hiderPos)) {
                    DEBUG_POMCP_SIM(cout <<"HSSimulator::isStateConsistentWithObs: hider not equal to other observed state:"
                                    <<hsOthObsState->hiderPos.toString()<<", "<<hsState->toString() <<endl;);
                    ok = false;
                }
            }
        }
    }
    return ok;
//#endif
}


int HSSimulator::getActionForRollout(const State *state, History *history) {
    int a = -1;

    switch(_params->rolloutPolicyType) {
        case SeekerHSParams::ROLL_OUT_POLICY_TYPE_RANDOM: {
            //random action
            vector<int> actions;
            getActions(state, history, actions);

            assert(!actions.empty());

            //choose random action
            a = actions[hsutils::random(actions.size()-1)];

            break;
        }
        case SeekerHSParams::ROLL_OUT_POLICY_TYPE_SMART: {
            const HSState* hsState = HSState::castFromState(state);

            //since this is for the roll-out (and simulation), we can always say that the opponent is visible (?)
            //(or does this give an error?)
            //bool visib = true;

            //smart action
            if (_smartSeeker==NULL) {
                DEBUG_CLIENT(cout<<"Using SmartSeeker for roll-out policy"<<endl;);
                _smartSeeker = new SmartSeeker(_params);
                _smartSeekerHiderPlayer = new PlayerInfo();

                _smartSeeker->initBelief(_map, _smartSeekerHiderPlayer);  //(_map, hsState->seekerPos, hsState->hiderPos, visib);
            }


            //AG150616: set hider and seeker pos
            _smartSeekerHiderPlayer->currentPos = hsState->hiderPos;
            _smartSeeker->playerInfo.currentPos = hsState->seekerPos;

            //get actions from smart seeker
            //a = _smartSeeker->getNextAction(hsState->seekerPos, hsState->hiderPos, visib);
            Pos nextPos = _smartSeeker->getNextPos(-1, &a);

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
