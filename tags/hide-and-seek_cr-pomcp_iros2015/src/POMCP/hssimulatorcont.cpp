#include "POMCP/hssimulatorcont.h"

#include <iostream>
#include <cassert>

#include "POMCP/hsstate.h"

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


State* HSSimulatorCont::genInitState(const State* obs, const State* obs2, double obs1p) {
    //AG131104: the seeker pos should be passed, and hider pos if visible
    State* initState = HSSimulator::genInitState(obs,obs2,obs1p);
    HSState* initHSState = static_cast<HSState*>(initState);

    if (_addNoise) {
        //not sure to add noise or not, but it makes sense to add noise to the positions
        addNoiseToPos(initHSState, _gausDistSeek, _gausDistHider, false /*true*/);
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


std::vector<State*> HSSimulatorCont::genAllInitStates(const State* obs, int n, const State* obs2, double obs1p) {
    assert(n>0);

    //get all discrete states (ie all not visible or 1 if visible)
    vector<State*> allStatesVecDisc = HSSimulator::genAllInitStates(obs,n, obs2, obs1p);
    //AG150126: TODO CHECK - not sure if it is better to directly copy this vector and just add random noise,
    //          since it already will have a length of n (unless not visible)

    if (_addNoise) {
        vector<State*> allStatesWithNoiseVec;
        //random value for index
        std::uniform_int_distribution<int> indexDistr(0,allStatesVecDisc.size()-1);

        //get n random samples and add noise
        for(int i=0; i<n; i++) {
            HSState *state = static_cast<HSState*>( allStatesVecDisc[indexDistr(_randomGenerator)]->copy() );
            addNoiseToPos(state, _gausDistSeek, _gausDistHider, false /*true*/);
            allStatesWithNoiseVec.push_back(state);
        }

        //delete discrete states
        for(vector<State*>::iterator it = allStatesVecDisc.begin(); it!=allStatesVecDisc.end(); it++) {            
            delete *it;
        }

        return allStatesWithNoiseVec;

    } else {
        const HSState* hsobs = static_cast<const HSState*>(obs);
        if (!hsobs->seekerPos.hasDouble() && !hsobs->hiderPos.hasDouble()) {
               //AG140710: only add 0.5 if it is not continuous
            //pass all states and put them in centre of cell
            for(vector<State*>::iterator it = allStatesVecDisc.begin(); it!=allStatesVecDisc.end(); it++) {
                HSState *state = static_cast<HSState*>(*it);
                state->seekerPos.add(0.5,0.5);
                state->hiderPos.add(0.5,0.5);
            }
        }

        return allStatesVecDisc;
    }
}


State* HSSimulatorCont::step(const State *state, int action, State*& obsOut, double &reward, bool genOutObs, const State* obsIn,
                             const State *obs2In, double obs1p) {
    State* nextState = HSSimulator::step(state,action,obsOut,reward,genOutObs,obsIn,obs2In,obs1p);

    if (nextState == NULL) return NULL;

    HSState* nexthsState = static_cast<HSState*>(nextState);

    HSState* hsObsOut = NULL;
    if (genOutObs) {
        bool setNewRandomHiderPos = false;

        //generate an observation output
        hsObsOut = static_cast<HSState*>(obsOut);
        if (hsObsOut->hiderPos.isSet()) { //visible
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
            unsigned int c = 0;
            do {
                if (c>_params->numBeliefStates) {
                    DEBUG_SHS(cout << "WARNING: generating false positive, after "<<c<<" times none visible found"<<endl;);
                    randPos.clear();
                    break;
                }

                randPos = _map->genRandomPos();
                visib = _map->isVisible(hsObsOut->seekerPos,randPos,_params->takeDynObstOcclusionIntoAccountWhenLearning);
                c++;
            } while (!visib);

            //set new pos
            hsObsOut->hiderPos = randPos;
        }

    }

    if (_addNoise) {
        //add noise starting from the previous location
        addNoiseToPos(nexthsState, _gausDistSeek, _gausDistHider, false);

        //add noise to obs
        if (genOutObs) {
            //TODO: certain chance of false positive/negative (?)
            addNoiseToPos(hsObsOut, _gausDistObsSeek, _gausDistObsHider, false);
        }
    }

    return nexthsState;
}

//AG150212: not used, disabled
/*bool HSSimulatorCont::checkNextStateAction(State *state, State *nextState, int action) {
    if (nextState == NULL) {
        cout << "HSSimulatorCont::checkNextStateAction: expected a next state!"<<endl;
        return false;
    }

    HSState* hsNextState = static_cast<HSState*>(nextState);

    bool ok = true;

    //check if poses are ok
    if (!_map->isPosInMap(hsNextState->seekerPos)) {
        cout << "HSSimulatorCont::checkNextStateAction: ERROR next seeker pos is not in map of state: " << hsNextState->seekerPos.toString()<<endl;
        ok = false;
    }
    //check if poses are ok
    if (!_map->isPosInMap(hsNextState->hiderPos)) {
        cout << "HSSimulatorCont::checkNextStateAction: ERROR next hider pos is not in map of state: " << hsNextState->hiderPos.toString()<<endl;
        ok = false;
    }
    //return ok;
    //AG140113: we accept all
    //if (!ok) cout << "HSSimulatorCont::checkNextStateAction: unexpected next state, but accepting because continuous HSSimulator"<<endl;
    //TODO: smarter processing
    return ok;
}*/


//TODO/NOTE: in the future we might have states that are not consistent with observation due to uncertainty
bool HSSimulatorCont::checkNextStateObs(const State *state, const State *nextState, const State* obs) {
    if (nextState == NULL) {
        cout << "HSSimulatorCont::checkNextStateObs: expected a next state!"<<endl;
        return false;
    }

    const HSState* hsNextState = static_cast<const HSState*>(nextState);

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


bool HSSimulatorCont::isStateConsistentWithObs(const State *state, const State *obs, const State* obs2) {
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


    if (_map->distanceEuc(hsState->seekerPos, hsObs->seekerPos) > _params->contConsistCheckSeekerDist) {
        //seeker pos not equal to obs
        DEBUG_POMCP_SIM(cout <<"HSSimulatorCont::isStateConsistentWithObs: inconsistent seeker pos, state:"
                        <<hsState->seekerPos.toString()<<",obs: "<< hsObs->seekerPos.toString() <<endl;);
        ok = false;

    } else {
        //check obs visib
        bool visib = _map->isVisible(hsState->seekerPos,hsState->hiderPos,_params->takeDynObstOcclusionIntoAccountWhenLearning);

        if (hsObs->hiderPos.isSet() != visib) {
            //hider pos visibility (of belief) should be consistent with observation: visible or not
            ok = false;
            DEBUG_POMCP_SIM(cout <<"HSSimulatorCont::isStateConsistentWithObs: hider pos should not be visible in obs, state:"
                            <<hsState->toString() <<endl;);

        } else if ( hsObs->hiderPos.isSet() && _map->distanceEuc(hsState->hiderPos, hsObs->hiderPos) > _params->contConsistCheckHiderDist) {
            //if hider is visible, the pos should be the same
            //ag131021: found bug! hsObs->hiderPos.isSet() was missing!!
            ok = false;
            DEBUG_POMCP_SIM(cout <<"HSSimulatorCont::isStateConsistentWithObs: hider pos should be visible in obs, but they are not equal, state:"
                            <<hsState->hiderPos.toString() << ", obs: "<<hsObs->hiderPos.toString() <<endl;);
        }        

        //AG150212: now check second observation
        if (ok && obs2!=NULL) {
            const HSState* hsObs2 = static_cast<const HSState*>(obs2);
            //check visib for seeker 2
            visib = _map->isVisible(hsObs2->seekerPos,hsState->hiderPos,_params->takeDynObstOcclusionIntoAccountWhenLearning);

            if (hsObs2->hiderVisible() != visib) {
                //hider pos visibility (of belief) should be consistent with observation of 2nd seeker: visible or not
                ok = false;
                DEBUG_POMCP_SIM(cout <<"HSSimulator::isStateConsistentWithObs: hider pos should not be visible in obs, state:"
                                <<hsState->toString()<<", as seen from obs2: "<<hsObs2->toString() <<endl;);

            } else if ( hsObs2->hiderVisible() && _map->distanceEuc(hsState->hiderPos, hsObs2->hiderPos) > _params->contConsistCheckHiderDist) {
                //if hider is visible, the pos should be the same
                ok = false;
                DEBUG_POMCP_SIM(cout <<"HSSimulator::isStateConsistentWithObs: hider pos should be visible in obs, but they are not equal to 2nd seeker, state:"
                                <<hsObs2->hiderPos.toString() << ", obs: "<<hsObs->hiderPos.toString() <<endl;);
            }
        }

        //AG150212: add a probability of allowing false positives/negatives
        if (!ok) {
            if (hsState->hiderVisible() && _params->contFalsePosProb>0) {
                if (_uniformProbDistr(_randomGenerator) < _params->contFalsePosProb) {
                    ok = true;
                }
            } else {
                if (_uniformProbDistr(_randomGenerator) < _params->contFalseNegProb) {
                    ok = true;
                }
            }

        }
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




/*void HSSimulatorCont::testAllFunctions() {
    cout << " HSSimulatorCont::testAllFunctions()"<<endl;

    return;
}*/
