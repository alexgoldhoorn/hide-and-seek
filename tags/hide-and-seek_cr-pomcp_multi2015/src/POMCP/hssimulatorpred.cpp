
#include "POMCP/hssimulatorpred.h"

#include <iostream>
#include <fstream>
#include <cassert>

#include "Base/hsglobaldata.h"
#include "Base/seekerhs.h"
#include "Base/hsconfig.h"

#include "Utils/generic.h"
#include "Utils/generic.h"

#include "exceptions.h"

using namespace pomcp;
using namespace std;

HSSimulatorPred::HSSimulatorPred(GMap *map, SeekerHSParams* params) : HSSimulatorCont(map,params), PersonPathPredConsumer()
    //_gaus(0, 1), _covarStep(2,2), /*_covarPredStep(2,2),*/ _mean(2)
{
    throw CException(_HERE_,"this class has not been completely implemented");
    //set continuous
    assert(params==map->getParams());
    map->setUsingContActions(true);

    //AG140909: init the vars
    resetParams();
}

HSSimulatorPred::~HSSimulatorPred() {
    //HSSimulator::~HSSimulator(); //done automatically!
}


State* HSSimulatorPred::step(const State *state, int action, State *&obsOut, double &reward, bool genOutObs,
                             const Observation *obs) {

    throw CException(_HERE_,"Implementation to be converted for multi agents"); //look at HSSimulator::step
    return NULL;
#ifdef OLD_CODE
    /*(const State *state, int action, State*& obsOut, double &reward, bool genOutObs, const State* obsIn,
                             const State *obs2In, double obs1p) {*/
    assert(state!=NULL);
    assert(action>=0 && action<HSGlobalData::NUM_ACTIONS);

    const HSState* hsstate = HSState::castFromState(state);

    Pos newHiderPos;
    Pos newSeekerPos;

    //check if to use the observation
    /*const HSState* hsObsIn = NULL;
    if (obsIn!=NULL) {
        hsObsIn = static_cast<const HSState*>(obsIn);
    }*/
    const HSObservation* hsObsIn = HSObservation::castFromObservation(obs);

    //seeker pos
    //if obs, get that value
    if (hsObsIn!=NULL) {
        newSeekerPos = hsObsIn->ownSeekerObs.seekerPos;
    } else {
        //do move of seeker
        newSeekerPos = _map->tryMove(action, hsstate->seekerPos);
        if (!newSeekerPos.isSet()) {
            //action not possible, so same position
            newSeekerPos = hsstate->seekerPos;
        }

        //cout << "seeker move "<<action<<" to "<<newSeekerPos.toString()<<endl;
    }

    //AG140909: add gaussian noise to seeker pos
    /*double rnoise = 0, cnoise = 0;
    _nextSeekerStepDistr.setMean(newSeekerPos);
    DEBUG_CHECK_INF_LOOP(uint loopCnt=0;);
    do {
        _nextSeekerStepDistr.getRandPoint(cnoise,rnoise);
        DEBUG_CHECK_INF_LOOP(
            if (loopCnt>99 && loopCnt % 100 == 0)
                cout<<"WARNING HSSimulatorPred::step repeated _nextSeekerStepDistr.getRandPoint "<<loopCnt<<" times! From "<<
                    newSeekerPos.toString()<<" last loc: r"<<rnoise<<"c"<<cnoise<<endl;
            loopCnt++;
        );
    } while (!_map->isPosInMap(rnoise,cnoise) || _map->isObstacle(rnoise,cnoise));
    newSeekerPos.set(rnoise, cnoise);*/
    addBivarNormalNoise(_nextSeekerStepDistr, newSeekerPos, newSeekerPos, "HSSimulatorPred::step(next seeker step)");

    //hider pos
    if (hsObsIn != NULL && hsObsIn->hiderVisible()) { //>=0 && obsIn != _hiddenObs) {
        //AG130827: not hidden, so we know the new pos of hider        
        /*_nextHiderStepDistr.setMean(hsObsIn->hiderPos);
        do {
            _nextHiderStepDistr.getRandPoint(cnoise,rnoise);
            DEBUG_CHECK_INF_LOOP(
                if (loopCnt>99 && loopCnt % 100 == 0)
                    cout<<"WARNING HSSimulatorPred::step repeated _nextHiderStepDistr.getRandPoint "<<loopCnt<<" times! From "<<
                        newSeekerPos.toString()<<" last loc: r"<<rnoise<<"c"<<cnoise<<endl;
                loopCnt++;
            );
        } while (!_map->isPosInMap(rnoise,cnoise) || _map->isObstacle(rnoise,cnoise));
        newHiderPos.set(rnoise, cnoise);*/
        addBivarNormalNoise(_nextHiderStepDistr, hsObsIn->hiderPos, newHiderPos, "HSSimulatorPred::step(next hider step - from obs)");

    } else {
        //no in Obs, or it is hidden
        //get random next pos
        double rnoise = 0, cnoise = 0;
        DEBUG_CHECK_INF_LOOP(uint loopCnt=0;);
        do {
            getRandomPersonPoint(hsstate->hiderPos.rowDouble(), hsstate->hiderPos.colDouble(), rnoise, cnoise);
            DEBUG_CHECK_INF_LOOP(
                if (loopCnt>99 && loopCnt % 100 == 0)
                    cout<<"WARNING HSSimulatorPred::step repeated getRandomPersonPoint "<<loopCnt<<" times! From "<<
                        newSeekerPos.toString()<<" last loc: r"<<rnoise<<"c"<<cnoise<<endl;
                loopCnt++;
            );
        } while (!_map->isPosInMap(rnoise,cnoise) || _map->isObstacle(rnoise,cnoise) );
        newHiderPos.set(rnoise,cnoise);

    } //no hsObsIn set

    if (genOutObs) {
        //generate observations
        HSState* hsObsOut = new HSState();
        obsOut = hsObsOut;
        //set obs seeker
        hsObsOut->seekerPos = newSeekerPos;

        bool setNewRandomHiderPos = false;

        //is visible
        //cout<< "New seekerpos="<<newSeekerPos.toString()<<", new hiderpos="<<newHiderPos.toString()<<endl;
        bool isVisib = _map->isVisible(newHiderPos,newSeekerPos,_params->takeDynObstOcclusionIntoAccountWhenLearning);

        //now apply probable events: false neg/pos/incorrect, ..
        if (isVisib) {
            if (_params->contFalseNegProb>0) {
                //check whether to add a false negative
                double p = _uniformProbDistr(_randomGenerator);
                if (p <= _params->contFalseNegProb) {
                    //set false negative
                    hsObsOut->hiderPos.clear();
                }
            }
            if (_params->contIncorPosProb>0 && hsObsOut->hiderPos.isSet()) {
                //AG140514: check to add incorrect reading
                //NOTE: now we give preference to false negative ..
                double p = _uniformProbDistr(_randomGenerator);
                if (p <= _params->contIncorPosProb) {
                    setNewRandomHiderPos = true;
                }
            }
        } else {
            //not visible
            if (_params->contObserveIfNotVisibProb>0) {
                double p = _uniformProbDistr(_randomGenerator);
                if (p <= _params->contObserveIfNotVisibProb) {
                    //make it visible even though raytrace says it isn't
                    isVisib = true;
                }
            }
            if (_params->contFalsePosProb>0) {
                //check whether to add a false positive
                double p = _uniformProbDistr(_randomGenerator);
                if (p <= _params->contFalsePosProb) {
                    setNewRandomHiderPos = true;
                }
            }
        }

        if (setNewRandomHiderPos) {
            //set incorrect positive, first find a visible cell randomly
            Pos randPos;
            bool visib = false;
            uint loopCnt = 0;
            do {
                if (loopCnt>_params->numBeliefStates) {
                    DEBUG_SHS(cout << "WARNING: generating false positive, after "<<loopCnt<<" times none visible found"<<endl;);
                    randPos.clear();
                    break;
                }

                randPos = _map->genRandomPos();
                visib = _map->isVisible(hsObsOut->seekerPos,randPos,_params->takeDynObstOcclusionIntoAccountWhenLearning);
                loopCnt++;
            } while (!visib);
            //set new pos
            hsObsOut->hiderPos = randPos;
        }

        //now decide observation
        if (isVisib) {
            //obsOut = _map->getIndexFromCoord(newHiderPos);
            hsObsOut->hiderPos = newHiderPos;
        } else {
            //obsOut = _hiddenObs;
            hsObsOut->hiderPos.clear();
        }

        //add noise to observation
        //AG140909: add gaussian noise to seeker pos obs
        /*rnoise = cnoise = 0;
        _seekerObsDistr.setMean(hsObsOut->seekerPos);
        DEBUG_CHECK_INF_LOOP(uint loopCnt=0;);
        do {
            _seekerObsDistr.getRandPoint(cnoise,rnoise);
            DEBUG_CHECK_INF_LOOP(
                if (loopCnt>99 && loopCnt % 100 == 0)
                    cout<<"WARNING HSSimulatorPred::step repeated _seekerObsDistr.getRandPoint "<<loopCnt<<" times! From "<<
                        newSeekerPos.toString()<<" last loc: r"<<rnoise<<"c"<<cnoise<<endl;
                loopCnt++;
            );
        } while (!_map->isPosInMap(rnoise,cnoise) || _map->isObstacle(rnoise,cnoise));
        hsObsOut->seekerPos.set(rnoise, cnoise);*/
        addBivarNormalNoise(_seekerObsDistr, hsObsOut->seekerPos, hsObsOut->seekerPos, "HSSimulatorPred::step(seeker obs)");

        //AG140909: add gaussian noise to hider pos obs
        if (hsObsOut->hiderPos.isSet()) {
            /*rnoise = cnoise = 0;
            _hiderObsDistr.setMean(hsObsOut->hiderPos);
            DEBUG_CHECK_INF_LOOP(loopCnt=0;);
            do {
                _hiderObsDistr.getRandPoint(cnoise,rnoise);
                DEBUG_CHECK_INF_LOOP(
                    if (loopCnt>99 && loopCnt % 100 == 0)
                        cout<<"WARNING HSSimulatorPred::step repeated _hiderObsDistr.getRandPoint "<<loopCnt<<" times! From "<<
                            newSeekerPos.toString()<<" last loc: r"<<rnoise<<"c"<<cnoise<<endl;
                    loopCnt++;
                );
            } while (!_map->isPosInMap(rnoise,cnoise) || _map->isObstacle(rnoise,cnoise));
            hsObsOut->hiderPos.set(rnoise, cnoise);*/
            addBivarNormalNoise(_hiderObsDistr, hsObsOut->hiderPos, hsObsOut->hiderPos, "HSSimulatorPred::step(hider obs)");
        }
    }

    //reward
    HSState* nexthsState = new HSState(newSeekerPos,newHiderPos);
    reward = getImmediateReward(state, nexthsState);

    return nexthsState;
#endif //OLD_CODE
}

void HSSimulatorPred::addBivarNormalNoise(NormalBivariateDist &dist, const Pos &meanPos, Pos &outPos, string whereDebug) {
    double rnoise = 0, cnoise = 0;
    dist.setMean(meanPos);
    DEBUG_CHECK_INF_LOOP(uint loopCnt=0;);
    do {
        dist.getRandPoint(cnoise,rnoise);
        DEBUG_CHECK_INF_LOOP(
            if (loopCnt>99 && loopCnt % 100 == 0)
                cout<<"WARNING "<<whereDebug<<" repeated distr.getRandPoint "<<loopCnt<<" times! From "<<
                    meanPos.toString()<<" last loc: r"<<rnoise<<"c"<<cnoise<<endl;
            loopCnt++;
        );
    } while (!_map->isPosInMap(rnoise,cnoise) || _map->isObstacle(rnoise,cnoise));
    outPos.set(rnoise, cnoise);
}

void HSSimulatorPred::resetParams() {
    DEBUG_CLIENT(cout<<"HSSimulatorPred::resetParams()"<<endl;);
    HSSimulatorCont::resetParams();

    //set covar next hider step
    vector<double> covarVec(4);
    covarVec[0] = covarVec[3] = sqrd( _params->contNextHiderStateStdDev );
    covarVec[1] = covarVec[2] = 0;
    _nextHiderStepDistr.setCovar(covarVec);

    //set covar next seeker step
    //vector<double> covarVec(4);
    covarVec[0] = covarVec[3] = sqrd( _params->contNextSeekerStateStdDev );
    //covarVec[1] = covarVec[2] = 0;
    _nextSeekerStepDistr.setCovar(covarVec);

    //set covar hider obs
    covarVec[0] = covarVec[3] = sqrd( _params->contHiderObsStdDev );
    _hiderObsDistr.setCovar(covarVec);

    //set covar seeker obs
    covarVec[0] = covarVec[3] = sqrd( _params->contSeekerObsStdDev );
    _hiderObsDistr.setCovar(covarVec);

    _ngetRandP = _ngetRandPPred = 0;
}


void HSSimulatorPred::getRandomPersonPoint(const double &r_in, const double &c_in, double &r_out, double &c_out) {
    //check if prediction available
    bool predAvail = predictionAvailable(_time+1, r_in, c_in);

    //if available, then based on probability choose predicted point
    if (predAvail && _uniformProbDistr(_randomGenerator) <= _params->contUseHiderPredStepProb) {
        //get point from prediction

        DEBUG_CHECK_INF_LOOP(uint loopCnt=0;);
        do {            
            getRandPredPoint(_time+1, r_out, c_out);
            DEBUG_CHECK_INF_LOOP(
                if (loopCnt>100)
                    cout<<"WARNING HSSimulatorPred::getRandomPersonPoint repeated getRandPredPoint more than 100 times!"<<endl;
                loopCnt++;
            );

        } while (!_map->isPosInMap(r_out,c_out) || _map->isObstacle(r_out,c_out));

        _ngetRandPPred++;

    } else {
        DEBUG_CHECK_INF_LOOP(uint loopCnt=0;);
        do {
            DEBUG_CHECK_INF_LOOP(
                if (loopCnt>100)
                    cout<<"WARNING HSSimulatorPred::getRandomPersonPoint repeated getting random pos more than 100 times!"<<endl;
                loopCnt++;
            );

            r_out = r_in;
            c_out = c_in;

            //use not the predicted point, check if should use next
            if (_uniformProbDistr(_randomGenerator) > _params->contNextHiderHaltProb) {
                //not a halt action

                //first random direction
                double ranDir = _uniformDirDistr(_randomGenerator);

                //next random pos direction
                r_out += _params->hiderStepDistance * sin(ranDir);
                c_out += _params->hiderStepDistance * cos(ranDir);
            }

            //now add the noise
            _nextHiderStepDistr.setMean(c_out, r_out);
            _nextHiderStepDistr.getRandPoint(c_out,r_out);

            _lastStepUsedPred = false;

        } while (!_map->isPosInMap(r_out,c_out) || !_map->isVisible(r_in, c_in, r_out, c_out, false)); // _map->isObstacle(r_out,c_out));
    }

    _ngetRandP++;
}

void HSSimulatorPred::resetTimer(unsigned int t) {
    HSSimulatorCont::resetTimer(t);
    _lastStepUsedPred = false;
}

#ifdef OLD_CODE
void HSSimulatorPred::testAllFunctions() {
    cout << " HSSimulatorPred::testAllFunctions()"<<endl;

    double r_in,c_in,r_out,c_out;

    vector<double> covarVec(4);
    covarVec[0] = covarVec[3] = sqrd( _params->contNextSeekerStateStdDev );
    covarVec[1] = covarVec[2] = 0;
    _nextSeekerStepDistr.setCovar(covarVec);



    cout<<"Testing HSSimulatorPred::getRandomPersonPoint. Give center location (of person): "<<endl<<"row: ";
    cin >> r_in;
    cout <<"col: ";
    cin >> c_in;
    uint n = 0;
    cout <<"number of samples to generate: ";
    cin >> n;

    char str[100];
    cout << "file: ";
    cin >> str;

    //>TEST4
    cout <<"Add predicted path, type -1 to stop adding."<<endl;
    double rp,cp;
    covarVec[0] = covarVec[3] = 1;
    vector<vector<double>> meanVecs,covarVecs;
    do {
        cout<<"row: ";
        cin >> rp;
        if (rp>=0) {
            cout <<"col: ";
            cin >> cp;
            if (cp>=0) {
                vector<double> meanV(2);
                meanV[0] = cp;
                meanV[1] = rp;
                meanVecs.push_back(meanV);
                covarVecs.push_back(covarVec);
            }
        }
    } while (rp>=0 && cp>=0);
    //set prediction path
    setPredPath(meanVecs,covarVecs);
    //<TEST4


    Pos newSeekerPos(r_in,c_in);

    //double rnoise = 0, cnoise = 0;
    _nextSeekerStepDistr.setMean(newSeekerPos);
    /*DEBUG_CHECK_INF_LOOP(uint loopCnt=0;);
    do {
        _nextSeekerStepDistr.getRandPoint(cnoise,rnoise);
        DEBUG_CHECK_INF_LOOP(
            if (loopCnt>100)
                cout<<"WARNING HSSimulatorPred::step repeated _nextSeekerStepDistr.getRandPoint more than 100 times!"<<endl;
            loopCnt++;
        );
    } while (!_map->isPosInMap(rnoise,cnoise) || _map->isObstacle(rnoise,cnoise));
    newSeekerPos.set(rnoise, cnoise);*/

    //TEST1: "_nextSeekerStepDistr.getRandPoint" - OK! (distribution, mean, std.dev.)
    //TEST2: TEST1 + consist. map pos check - OK
    //TEST3: use getRandomPersonPoint - OK
    //TEST4: TEST3 + pred. path - OK
    //TEST5: TEST4 + time increase - OK



    ofstream fout(str);
    double r=r_in,c=c_in;

    for(uint i=0; i<n; i++) {
        //_nextSeekerStepDistr.getRandPoint(r_out,c_out); //TEST1 (OK)

        /*//>TEST2 (OK)
        uint loopCnt=0;
        do {
            _nextSeekerStepDistr.getRandPoint(c_out,r_out);
            loopCnt++;
            );
        } while (!_map->isPosInMap(r_out,c_out) || _map->isObstacle(r_out,c_out));
        *///<TEST2

        int ngetRandPPred=_ngetRandPPred;

        getRandomPersonPoint(r,c,r_out,c_out);

        fout << r_out<<","<<c_out<<","<<_time<<","<<(ngetRandPPred!=(int)_ngetRandPPred) <<endl;//","<<loopCnt<<endl;

        //TEST5
        if (_time>=9) {
            resetTimer();
            r=r_in;c=c_in;
        } else {
            increaseTimer();
            r=r_out;c=c_out;
        }
    }
    fout.close();


    //TODO: check other poses

    //TODO: with check
    //then the functions used: getRandomPersonPoint, step
    //then with predictor


    cout<< "Number of predictions used: "<<_ngetRandPPred<<" / "<<_ngetRandP<<" ("<< (100.0*_ngetRandPPred/_ngetRandP) <<"%)"<<endl;

    return;


    HSSimulatorCont::testAllFunctions();

    return;

    //HSSimulatorCont::testAllFunctions();

    //choose rand pos
    /*Pos p;
    do  {
        p = _map->genRandomPos();
    } while ();*/

    //double r_in,c_in,r_out,c_out;
    /*cout<<"Testing HSSimulatorPred::getRandomPersonPoint. Give center location (of person): "<<endl<<"row: ";
    cin >> r_in;
    cout <<"col: ";
    cin >> c_in;

    uint n = 0;
    cout <<"number of samples to generate: ";
    cin >> n;

    char str[100];
    cout << "file: ";
    cin >> str;

    ofstream fout(str);
    for(uint i=0; i<n; i++) {
        getRandomPersonPoint(r_in,c_in,r_out,c_out);
        fout << r_out<<","<<c_out<<endl;
    }
    fout.close();*/
    //todo: add prediction..

    //TODO: first normalrel... in one file
    //THEN the funtion getrandampoint..
    //step..
       //..

}
#endif
