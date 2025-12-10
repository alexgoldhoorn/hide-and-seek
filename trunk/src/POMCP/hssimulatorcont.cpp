#include "POMCP/hssimulatorcont.h"

#include <iostream>
#include <cassert>

#include "POMCP/hsstate.h"
#include "POMCP/hsobservation.h"

#include "Base/hsglobaldata.h"
#include "Base/seekerhs.h"

#include "Utils/generic.h"
#include "Utils/timer.h"
#include "Base/hsconfig.h"
#include "Smart/smartseeker.h"

//iriutils
#include "exceptions.h"

#include <cassert>
#include <vector>
#include <random>


using namespace pomcp;
using namespace std;

HSSimulatorCont::HSSimulatorCont(GMap *map, SeekerHSParams* params) : HSSimulator(map,params),
    _gausDistSeek(0, _params->contNextSeekerStateStdDev), _gausDistHider(0, _params->contNextHiderStateStdDev),
    _gausDistObsSeek(0, _params->contSeekerObsStdDev), _gausDistObsHider(0, _params->contHiderObsStdDev),
    _uniformDistrRows(0,_map->rowCount()-1), _uniformDistrCols(0,_map->colCount()-1)
{
    if (_params->contFalsePosProb<0 || _params->contFalsePosProb>1) {
        throw CException(_HERE_,"The false positive probability should be between 0 and 1");
    }
    if (_params->contFalseNegProb<0 || _params->contFalseNegProb>1) {
        throw CException(_HERE_,"The false negative probability should be between 0 and 1");
    }
    if (_params->contIncorPosProb<0 || _params->contIncorPosProb>1) {
        throw CException(_HERE_,"The incorrect positive probability should be between 0 and 1");
    }

    _addNoise = /*_params->solverType==SeekerHSParams::SOLVER_POMCP_CONT*//* _params->pomcpSimType==SeekerHSParams::POMCP_SIM_CONT &&*/
            (_params->contNextHiderStateStdDev>0 || _params->contSeekerObsStdDev>0 || _params->contNextSeekerStateStdDev>0 || _params->contHiderObsStdDev>0);

    if (_addNoise) {
        cout << "Noise will be added"<<endl;
    } else {
        cout << "Noise will NOT be added"<<endl;
    }

    //set continuous
    assert(params==map->getParams());
    map->setUsingContActions(true);

    //AG140909: init the vars
    //resetParams();
}

HSSimulatorCont::~HSSimulatorCont() {
}

State* HSSimulatorCont::genInitState(const Observation *obs) {
    //AG131104: the seeker pos should be passed, and hider pos if visible
    State* initState = HSSimulator::genInitState(obs);
    HSState* initHSState = HSState::castFromState(initState);

    if (_addNoise) {
        //not sure to add noise or not, but it makes sense to add noise to the positions
        addNoiseToPos(initHSState, _gausDistSeek, _gausDistHider, false);
    }

    return initHSState;
}

void HSSimulatorCont::addNoiseToPos(HSState *state, std::normal_distribution<double> &gausDistSeek,
                                    std::normal_distribution<double> &gausDistHider, bool useInt) {

    double seekerRow = 0, hiderRow = 0, seekerCol = 0, hiderCol = 0;

    bool ok = false;

    //decide real position
    if (useInt) { //when int, its in the center of the grid
        seekerRow = state->seekerPos.row() + 0.5;
        seekerCol = state->seekerPos.col() + 0.5;
    } else { //otherwise the real double values
        seekerRow = state->seekerPos.rowDouble();
        seekerCol = state->seekerPos.colDouble();
    }

    //same for hider pos
    if (state->hiderPos.isSet()) {
        if (useInt) {
            hiderRow = state->hiderPos.row() + 0.5;
            hiderCol = state->hiderPos.col() + 0.5;
        } else {
            hiderRow = state->hiderPos.rowDouble();
            hiderCol = state->hiderPos.colDouble();
        }
    }

    Pos pos;

    //get seeker noise position that is a legal position and not an obstacle
    do {
        pos.set(seekerRow + gausDistSeek(_randomGenerator),
                seekerCol + gausDistSeek(_randomGenerator) );

        ok = _map->isPosInMap(pos);

        if (ok) {
            ok = !_map->isObstacle(pos);
        }
    } while (!ok);

    //set noisy seeker pos
    state->seekerPos = pos;

    //get hider noise position that is a legal position and not an obstacle
    if (state->hiderPos.isSet()) {
        //AG160205: set the heading
        if (_params->useHeading)
            pos.setHeading(state->hiderPos.heading());

        //add noise to pos
        do {
            pos.set(hiderRow + gausDistHider(_randomGenerator),
                    hiderCol + gausDistHider(_randomGenerator) );

            ok = _map->isPosInMap(pos);

            if (ok) {
                ok = !_map->isObstacle(pos);
            }
        } while (!ok);

        //set noisy hider pos
        state->hiderPos = pos;
    }
}

std::vector<State*> HSSimulatorCont::genAllInitStates(const Observation *obs, int n) {
    //get all discrete states (ie all not visible or 1 if visible)
    vector<State*> allStatesVecDisc = HSSimulator::genAllInitStates(obs,n);

    //assert we get n states
    assert(allStatesVecDisc.size()==(size_t)n);

    //TODO: do more in future? checks?..

    if (_addNoise) {
        //loop all states and add noise
        for(State* state : allStatesVecDisc) {
            HSState* hsState = HSState::castFromState(state);
            addNoiseToPos(hsState, _gausDistSeek, _gausDistHider, false);

/*            //AG TMP
            if (!isStateConsistentWithObs(state, obs)) {
                cout << "HSSimulatorCont::genAllInitStates: inconsistent state "<<state->toString()<<" for obs "<<obs->toString()<<endl;
            }*/
        }
    }
    //else  cout << "HSSimulatorCont::genAllInitStates: NO NOISE added"<<endl;

    return allStatesVecDisc;
}

State* HSSimulatorCont::step(const State *state, int action, State *&obsOut, double &reward, bool genOutObs, const Observation *obs) {

    //HSSimulator creates enxt step
    State* nextState = HSSimulator::step(state,action,obsOut,reward,genOutObs,obs);

    if (nextState == NULL) return NULL; //i.e. failed

    HSState* nexthsState = HSState::castFromState(nextState);

    HSState* hsObsOut = NULL;
    if (genOutObs) {
        bool setNewRandomHiderPos = false;

        //generate an observation output
        hsObsOut = HSState::castFromState(obsOut);
        if (hsObsOut->hiderVisible()) { //visible
            if (_params->contFalseNegProb>0) {
                //check whether to add a false negative
                double p = _uniformProbDistr(_randomGenerator);
                if (p<=_params->contFalseNegProb) {
                    //set false negative
                    hsObsOut->hiderPos.clear();
                }
            }
            if (_params->contIncorPosProb>0 && hsObsOut->hiderPos.isSet()) {
                //AG140514: check to add incorrect reading
                //NOTE: now we give preference to false negative ..
                double p = _uniformProbDistr(_randomGenerator);
                if (p<=_params->contIncorPosProb) {
                    setNewRandomHiderPos = true;
                }
            }
        } else { //not visible
            if (_params->contFalsePosProb>0) {
                //check whether to add a false positive
                double p = _uniformProbDistr(_randomGenerator);
                if (p<=_params->contFalsePosProb) {
                    setNewRandomHiderPos = true;
                }
            }
        }

        if (setNewRandomHiderPos) {
            //set incorrect positive, first find a visible cell randomly
            Pos randPos;
            bool visib = false;
            //counter to have a maximum search time
            unsigned int counter = 0;
            do {
                if (counter>_params->numBeliefStates) {
                    DEBUG_WARN(cout << "WARNING: generating false positive, after "<<counter<<" times none visible found"<<endl;);
                    randPos.clear();
                    break;
                }

                randPos = _map->genRandomPos();

                //AG160504: add dyn obst. vec
                vector<IDPos> dynObstVec;
                if (obs!=nullptr && _params->useDynObstForVisibCheck)
                    dynObstVec = obs->dynObstVec;

                visib = _map->isVisible(hsObsOut->seekerPos,randPos,_params->takeDynObstOcclusionIntoAccountWhenLearning,_params->simNotVisibDist,&dynObstVec);
                                                //AG160216: TODO check if we should simnotvisibdist here!!
                counter++;
            } while (!visib);

            //set new pos
            hsObsOut->hiderPos = randPos;
        }
    }

    if (_addNoise) {
        //add noise to next state
        addNoiseToPos(nexthsState, _gausDistSeek, _gausDistHider, false);

        //add noise to obs
        if (genOutObs) {
            //TODO: certain chance of false positive/negative (?)
            addNoiseToPos(hsObsOut, _gausDistObsSeek, _gausDistObsHider, false);
        }
    }

    //AG160125: add noise to noise
    if (_params->useHeading &&_params->contHiderHeadStdDev>0) {
        double heading = nexthsState->hiderPos.heading();
        assert(heading>=0 && heading <= 2.0*M_PI);
        double newHead = hsutils::dnnmod( hsutils::randomGauss(heading, _params->contHiderHeadStdDev), 2.0*M_PI);
        assert(newHead >=0 && newHead <= 2.0*M_PI);
        nexthsState->hiderPos.setHeading(newHead);
    }

    return nexthsState;
}

//TODO/NOTE: in the future we might have states that are not consistent with observation due to uncertainty
bool HSSimulatorCont::checkNextStateObs(const State *state, const State *nextState, const Observation *obs) const {
    //(const State *state, const State *nextState, const State* obs) {

    //TODO: check more intelligtenly? now every legal position is allowed since in the real world observations can be 'not consistent'

    assert(nextState!=NULL);

    const HSState* hsNextState = HSState::castFromState(nextState);

    assert(hsNextState->seekerPos.isSet());
    assert(hsNextState->hiderPos.isSet());
    assert(_map->isPosInMap(hsNextState->seekerPos));
    assert(_map->isPosInMap(hsNextState->hiderPos));

    return true;
}

bool HSSimulatorCont::checkNextStateObs(const State *state, const State *nextState, const State* obs) {
    if (nextState == NULL) {
        cout << "HSSimulatorCont::checkNextStateObs: expected a next state!"<<endl;
        return false;
    }

    const HSState* hsNextState = HSState::castFromState(nextState);

    bool ok = true;

    //check if poses are ok
    if (!_map->isPosInMap(hsNextState->seekerPos)) {
        cout << "HSSimulatorCont::checkNextStateObs: ERROR next seeker pos is not in map of state: " << hsNextState->seekerPos.toString()<<endl;
        ok = false;
    }
    //check if poses are ok
    if (!_map->isPosInMap(hsNextState->hiderPos)) {
        cout << "HSSimulatorCont::checkNextStateObs: ERROR next hider pos is not in map of state: " << hsNextState->hiderPos.toString()<<endl;
        ok = false;
    }

    return ok;
}

bool HSSimulatorCont::isStateConsistentWithObs(const State *state, const Observation *obs)  {
        //(const State *state, const State *obs, const State* obs2) {

    //AG150616: the same as HSSimulator::isStateConsistentWithObs, but using distanceEuc<_params->contConsistCheckSeekerDist instead of
    //          equalsInt()

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
    if (hsState->seekerPos.distanceEuc(hsObs->ownSeekerObs.seekerPos) > _params->contConsistCheckSeekerDist) {

        DEBUG_POMCP_SIM(cout<<"HSSimulatorCont::isStateConsistentWithObs: the seeker position (" << hsState->toString()
                            <<") is not the same of the state and obs.ownSeekerPos ("<<(hsObs->ownSeekerObs.toString())<<")"<<endl;);
        ok = false;
    } else {
        //now check the hider pose

        //AG160213: use probability of gmap, taking into account distance to see
        double pVisib = _map->getVisibilityProb(hsObs->getSeekerPoses(), hsState->hiderPos,
                                                _params->takeDynObstOcclusionIntoAccountWhenLearning,
                                                      _params->useDynObstForVisibCheck?&hsObs->dynObstVec:nullptr);

        /*if (pVisib == 0) {
            //hider should not be visible from any location (also taking into account distance)
            ok = !(hsState->hiderVisible());
            DEBUG_POMCP_SIM(cout <<"HSSimulatorCont::isStateConsistentWithObs: hider should be not visible but is visible";<<endl);
        } else {*/
            //hider can be visible
            //is visible
            bool isVisib = false;

//            TODO: use previous consistency check OR assume visiblity if pVis>0.6.. OR return true if isVisbi return true with probab pVisbi, else return false with 1-pvisbi??

            //now check the other observations
            for(size_t i=0; i<hsObs->size() && ok; i++) {
                const HSState* hsOthObsState = hsObs->getObservationState(i);
                //hider pos should be same if there is an observation
                if (hsOthObsState->hiderVisible()) {
                    if (hsState->hiderPos.distanceEuc(hsOthObsState->hiderPos) > _params->contConsistCheckHiderDist) {
                        //AG150915: was contConsistCheckSeekerDist
                        DEBUG_POMCP_SIM(cout <<"HSSimulatorCont::isStateConsistentWithObs: hider not equal to other observed state:"
                                <<hsOthObsState->hiderPos.toString()<<", "<<hsState->toString() <<endl;);
                        ok = false;
                        break; //AG160113: found failure, exit
                    } else {
                        isVisib = true;
                    }
                }
            }

            if (isVisib) {
                if (pVisib==0) {
                    //sure that it should be not visible
                    ok = false;
                    DEBUG_POMCP_SIM(cout <<"HSSimulatorCont::isStateConsistentWithObs: hider should not be visible but is visible"<<endl);
                } else {
                    //the probability of being consistent is given by p
                    double p = _uniformProbDistr(_randomGenerator);
                    //visible, so return ok depending on prob. of not visib
                    ok = (p<=pVisib);
                }
            } else {
                //not visible
                if (pVisib==1.0) {
                    //sure that it should be visible, BUT in practice this won't happen (p max by default is .85)
                    ok = false;
                    DEBUG_POMCP_SIM(cout <<"HSSimulatorCont::isStateConsistentWithObs: hider should be visible but is not visible"<<endl);
                } else {
                    //the probability of being consistent is given by p
                    double p = _uniformProbDistr(_randomGenerator);
                    //not visible, so return ok depending on prob. of not visib
                    ok = (p>pVisib);
                }
            }



#ifdef OLD_CODE
        //check if hider should be visible
        bool visib = _map->isVisible(hsState->seekerPos,hsState->hiderPos,_params->takeDynObstOcclusionIntoAccountWhenLearning);

        if (hsObs->ownSeekerObs.hiderVisible()!=visib) {
            //hider pos visibility (of belief) should be consistent with (own) observation: visible or not
            ok = false;
            DEBUG_POMCP_SIM(cout <<"HSSimulatorCont::isStateConsistentWithObs: hider pos should not be visible in obs, state:"
                            <<hsState->toString() <<endl;);
        } else if (visib && hsState->hiderPos.distanceEuc(hsObs->ownSeekerObs.hiderPos) > _params->contConsistCheckHiderDist) { //AG150915: was contConsistCheckSeekerDist
            //if hider is visible, the pos should be the same
            ok = false;
            DEBUG_POMCP_SIM(cout <<"HSSimulatorCont::isStateConsistentWithObs: hider pos should be visible in obs, but they are not equal to 2nd seeker, state:"
                            <<hsState->toString()<< ", obs: "<<hsObs->ownSeekerObs.hiderPos.toString() <<endl;);
        } else {
            //now check the other observations
            for(size_t i=1; i<hsObs->size() && ok; i++) {
                const HSState* hsOthObsState = hsObs->getObservationState(i);

                //hider pos should be same if there is an observation
                if (hsOthObsState->hiderVisible() && hsState->hiderPos.distanceEuc(hsOthObsState->hiderPos) > _params->contConsistCheckHiderDist) { //AG150915: was contConsistCheckSeekerDist
                    DEBUG_POMCP_SIM(cout <<"HSSimulatorCont::isStateConsistentWithObs: hider not equal to other observed state:"
                                    <<hsOthObsState->hiderPos.toString()<<", "<<hsState->toString() <<endl;);
                    ok = false;
                }
            }
        }
#endif
    }

    return ok;
}

bool HSSimulatorCont::hiderWin(const HSState* hsstate) {
    return (_map->distanceEuc(hsstate->hiderPos,_map->getBase()) <= _params->winDist);
}

void HSSimulatorCont::resetParams() {
    //cout<<"HSSimulatorCont::resetParams()"<<endl;
    decltype(_gausDistSeek.param()) new_range_gausDistSeek (0, _params->contNextSeekerStateStdDev);
    _gausDistSeek.param(new_range_gausDistSeek);

    decltype(_gausDistHider.param()) new_range_gausDistHider (0, _params->contNextHiderStateStdDev);
    _gausDistHider.param(new_range_gausDistHider);

    decltype(_gausDistObsSeek.param()) new_range_gausDistObsSeek (0, _params->contSeekerObsStdDev);
    _gausDistObsSeek.param(new_range_gausDistObsSeek);

    decltype(_gausDistObsHider.param()) new_range_gausDistObsHider (0, _params->contHiderObsStdDev);
    _gausDistObsHider.param(new_range_gausDistObsHider);
}


