#include "test/include/POMCP/simulator-test.h"
#include <QTest>
#include <iostream>

#include "POMCP/hssimulatorcont.h"

#include "Base/hsglobaldata.h"

#include "Utils/timer.h"
#include "Utils/generic.h"

using namespace std;
using namespace pomcp;

int SimulatorTest::MAX_STEP_DIST_TEST = 2;
int SimulatorTest::MAX_STEP_DIST_TEST_WNOISE = 5;
int SimulatorTest::OBS_NUM_OTHER_SEEKERS = 2;
QString SimulatorTest::MAP_OTHER_NAME = "map3_20x20.txt";
QList<QString> SimulatorTest::SKIP_TEST_FUNCS_LIST = {"hsSimulatorTestDiscSimple", "hsSimulatorTestContSimple",
                                                      "hsSimulatorTestDiscOther" ,"hsSimulatorTestContOther"
                                                     };


void SimulatorTest::initTestCase() {
    _params = new SeekerHSParams();

    _params->rewardType = SeekerHSParams::REWARD_FIND_REV_DIST;
    _params->numBeliefStates = 10;

    _params->contConsistCheckSeekerDist = 1.1;
    _params->contConsistCheckHiderDist  = 1.1;
    _params->contNextSeekerStateStdDev = 0;
    _params->contNextHiderStateStdDev = 0;
    _params->contSeekerObsStdDev = 0;
    _params->contHiderObsStdDev = 0;

    _params->contIncorPosProb = 0;
    _params->contFalseNegProb = 0;
    _params->contFalsePosProb = 0;

    _params->contUpdInconsistAcceptProb = 0;

    //_params->contNextHiderHaltProb = 0;
    //_params->contUseHiderPredStepProb = 0;

    _mapPath = QProcessEnvironment::systemEnvironment().value("HSPATH",".") + "/data/maps/";
}

GMap* SimulatorTest::generateGMap() {
    GMap* map = new GMap(4,4,_params);
    //gen map
    map->addObstacle(1,1);
    map->addObstacle(1,2);
    map->setMapFixed();
    return map;
}

GMap* SimulatorTest::loadMap(QString name) {
    QString mapName = _mapPath+name;
    GMap* gmap = new GMap(mapName.toStdString(), _params);
    return gmap;
}


void SimulatorTest::cleanupTestCase() {
    delete _params;
}

void SimulatorTest::testSkipFunc() {
    //test skip func
    for(const QString& skipFName : SKIP_TEST_FUNCS_LIST) {
        //check skip
        QString fname = "input Class::"+skipFName+"(params)";
        QVERIFY(skipTestFunc(fname.toStdString().c_str()));
        //check other
        fname = "input Class::"+skipFName+"X(params)";
        QVERIFY(!skipTestFunc(fname.toStdString().c_str()));
        fname = "input Class::X"+skipFName+"(params)";
        QVERIFY(!skipTestFunc(fname.toStdString().c_str()));
    }

    QVERIFY(!skipTestFunc(Q_FUNC_INFO));
    QVERIFY(skipTestFunc(" void SimulatorTest::hsSimulatorTestDiscSimple() "));
}

void SimulatorTest::hsSimulatorTestConsistencyCheck() {
    cout << "- SimulatorTest::hsSimulatorTestConsistencyCheck() -"<<endl;

    //test
    _map = generateGMap();
    _map->printMap(false,0,0,0,2);

    //consistency check obs..
    HSObservation* obs = genObs(_map, 0, 0, 0, 2, 0); //OBS_NUM_OTHER_SEEKERS);
    QVERIFY(obs->ownSeekerObs.seekerPos.equalsInt(0,0));
    QVERIFY(!obs->ownSeekerObs.hiderPos.isSet() || obs->ownSeekerObs.hiderPos.equalsInt(0,2));

    //simulator
    _params->useContinuousPos = true;
    HSSimulatorCont sim(_map,_params);
    HSState state(0,0,0,2);
    QVERIFY(state.seekerPos.equalsInt(0,0));
    QVERIFY(state.hiderPos.equalsInt(0,2));
    //AG160329: since the following functions are probability based we assume that at least
    // in 70% of the time the answer is as expected

    /*int nOk = 0;
    for(int i=0; i<REP_PROB_FUNCS; ++i) {
        //state should be visible, and thus the same as the obs
        if (sim.isStateConsistentWithObs(&state, obs)==true)
            ++nOk;
    }*/
    //QCOMPARE(sim.isStateConsistentWithObs(&state, obs), true);
    QVERIFY(checkStateConsistentWithObs(sim, state, obs)>=CONSIST_CHECK_PROB); //nOk>=0.7*REP_PROB_FUNCS);

    //make invisible obs, but same state
    obs->ownSeekerObs.hiderPos.clear();
    /*nOk = 0;
    for(int i=0; i<REP_PROB_FUNCS; ++i) {
        if (sim.isStateConsistentWithObs(&state, obs)==false)
            ++nOk;

    }*/
    //QCOMPARE(sim.isStateConsistentWithObs(&state, obs), false);
    QVERIFY(checkStateConsistentWithObs(sim, state, obs)<=(1.0-CONSIST_CHECK_PROB));//nOk>=0.7*REP_PROB_FUNCS);


    delete obs;
    delete _map;
}

double SimulatorTest::checkStateConsistentWithObs(pomcp::Simulator& sim, const State &state, const Observation* obs) {
    int nOk = 0;
    for(int i=0; i<REP_PROB_FUNCS; ++i) {
        //state should be visible, and thus the same as the obs
        if (sim.isStateConsistentWithObs(&state, obs)==true)
            ++nOk;
    }
    //QCOMPARE(sim.isStateConsistentWithObs(&state, obs), true);
    double p = 1.0*nOk/REP_PROB_FUNCS;
    qDebug() << "SimulatorTest::checkStateConsistentWithObs: Check consist. "<<state.toString().c_str()<<" with obs "
             << obs->toString().c_str() << " returned true: " << p;
    return p;
}

double SimulatorTest::checkStateConsistentWithObsProb(Simulator &sim, const HSState *hsState, const HSObservation *hsObs) {
    //AG160330: this function is copied from HSSimulatorCont::isStateConsistentWithObs

    //prob. of being consistent
    double p=0;

    assert(hsState!=nullptr);
    assert(hsObs!=nullptr);
    assert(hsState->seekerPos.isSet());
    assert(hsState->hiderPos.isSet());

    if (hsState->seekerPos.distanceEuc(hsObs->ownSeekerObs.seekerPos) > _params->contConsistCheckSeekerDist) {

        cout<<"SimulatorTest::checkStateConsistentWithObsProb: ERROR-S the seeker position (" << hsState->toString()
                            <<") is not the same of the state and obs.ownSeekerPos ("<<(hsObs->ownSeekerObs.toString())<<")"
                            <<" distance is "<<hsState->seekerPos.distanceEuc(hsObs->ownSeekerObs.seekerPos)
                            <<" while max is " <<_params->contConsistCheckSeekerDist
                           <<endl;
        p = 0;
    } else {
        //now check the hider pose

        //AG160213: use probability of gmap, taking into account distance to see
        double pVisib = _map->getVisibilityProb(hsObs->getSeekerPoses(), hsState->hiderPos,
                                                _params->takeDynObstOcclusionIntoAccountWhenLearning);

            bool isVisib = false;

            //now check the other observations
            for(size_t i=0; i<hsObs->size(); i++) {
                const HSState* hsOthObsState = hsObs->getObservationState(i);
                //hider pos should be same if there is an observation
                if (hsOthObsState->hiderVisible()) {
                    if (hsState->hiderPos.distanceEuc(hsOthObsState->hiderPos) > _params->contConsistCheckHiderDist) {
                        //AG150915: was contConsistCheckSeekerDist
                        cout <<"SimulatorTest::checkStateConsistentWithObsProb: ERROR-OBS-H  hider not equal to other observed state: obs="
                                <<hsOthObsState->hiderPos.toString()<<", state="<<hsState->toString()<<" dist larger than "<<_params->contConsistCheckHiderDist <<endl;
                        break; //AG160113: found failure, exit
                    } else {
                        isVisib = true;
                    }
                }
            }

            if (isVisib) {
                if (pVisib==0) {
                    //sure that it should be not visible
                    //ok = false;
                    p = 0;
                    cout <<"SimulatorTest::checkStateConsistentWithObsProb: ERROR-H-V hider should not be visible but is visible - "
                        <<"obs="<<hsObs->toString()<<", state="<<hsState->toString()
                        <<endl;

                } else {
                    //the probability of being consistent is given by p
                    ///double p = _uniformProbDistr(_randomGenerator);
                    //visible, so return ok depending on prob. of not visib
                    ///ok = (p<=pVisib);
                    p = pVisib;
                }
            } else {
                //not visible
                if (pVisib==1.0) {
                    //sure that it should be visible, BUT in practice this won't happen (p max by default is .85)
                    ///ok = false;
                    cout <<"SimulatorTest::checkStateConsistentWithObsProb: ERROR-H-NV hider should be visible but is not visible"<<endl;
                    p = 0;
                } else {
                    //the probability of being consistent is given by p
                    ///double p = _uniformProbDistr(_randomGenerator);
                    //not visible, so return ok depending on prob. of not visib
                    ///ok = (p>pVisib);
                    p = 1.0-pVisib;
                }
            }
    }

    return p;
}

void SimulatorTest::hsSimulatorTestDiscSimple() {
    if (skipTestFunc(Q_FUNC_INFO)) return;
    cout << "--- TEST - SimulatorTest::hsSimulatorTestDiscSimple --- "<<endl;
    _params->useContinuousPos = false;
    _map = generateGMap();
    _map->printMap();
    HSSimulator sim(_map,_params);
    testSimulator(&sim, _map,MAX_STEP_DIST_TEST);
    delete _map;
}

void SimulatorTest::hsSimulatorTestContSimple() {
    if (skipTestFunc(Q_FUNC_INFO)) return;
    cout << "--- TEST - SimulatorTest::hsSimulatorTestContSimple --- "<<endl;
    _params->useContinuousPos = true;
    _map = generateGMap();
    _map->printMap();
    HSSimulator sim(_map,_params);
    testSimulator(&sim, _map,MAX_STEP_DIST_TEST);
    delete _map;
}

void SimulatorTest::hsSimulatorTestDiscOther() {
    if (skipTestFunc(Q_FUNC_INFO)) return;
    cout << "--- TEST - SimulatorTest::hsSimulatorTestDiscOther --- "<<endl;
    _params->useContinuousPos = false;
    _map = loadMap(MAP_OTHER_NAME);
    _map->printMap();
    HSSimulator sim(_map,_params);
    testSimulator(&sim, _map,MAX_STEP_DIST_TEST);
    delete _map;
}

void SimulatorTest::hsSimulatorTestContOther() {
    if (skipTestFunc(Q_FUNC_INFO)) return;
    cout << "--- TEST - SimulatorTest::hsSimulatorTestContOther --- "<<endl;
    _params->useContinuousPos = true;
    _map = loadMap(MAP_OTHER_NAME);
    _map->printMap();
    HSSimulator sim(_map,_params);
    testSimulator(&sim, _map,MAX_STEP_DIST_TEST);
    delete _map;
}

void SimulatorTest::hsSimulatorContTestContSimple() {
    if (skipTestFunc(Q_FUNC_INFO)) return;
    cout << "--- TEST - SimulatorTest::hsSimulatorContTestContSimple --- "<<endl;
    _params->useContinuousPos = true;
    _map = generateGMap();
    _map->printMap();
    HSSimulatorCont sim(_map,_params);
    testSimulator(&sim, _map,MAX_STEP_DIST_TEST_WNOISE);
    delete _map;
}

void SimulatorTest::hsSimulatorContTestContOther() {
    if (skipTestFunc(Q_FUNC_INFO)) return;
    cout << "--- TEST - SimulatorTest::hsSimulatorContTestContOther --- "<<endl;
    _params->useContinuousPos = true;
    _map = loadMap(MAP_OTHER_NAME);
    _map->printMap();
    HSSimulatorCont sim(_map,_params);
    testSimulator(&sim, _map,MAX_STEP_DIST_TEST_WNOISE);
    delete _map;
}


HSObservation* SimulatorTest::genObs(GMap *map, int xRow, int xCol, int yRow, int yCol, int nOther) {
    double useObsProb = 1.0/nOther;
    PlayerInfo *thisPlayer = new PlayerInfo();
    thisPlayer->currentPos.set(xRow,xCol);
    thisPlayer->hiderObsPosWNoise.set(yRow,yCol);

    //AG160331: check if visible
    double pVisib = _map->getVisibilityProb(thisPlayer->currentPos, thisPlayer->hiderObsPosWNoise, true);
    if (pVisib==0 || hsutils::randomDouble()>pVisib)
        thisPlayer->hiderObsPosWNoise.clear(); // simulate not visible

    thisPlayer->useObsProb /*hiderObsTrustProb*/ = useObsProb;
    thisPlayer->posRead = true;
    int id = 0;
    thisPlayer->id = id++;

    PlayerInfo* hiderPlayer = new PlayerInfo();
    hiderPlayer->currentPos = map->genRandomPos();
    hiderPlayer->id = id++;
    //hiderPlayer.posRead = true;
    hiderPlayer->playerType = HSGlobalData::P_Hider;


    //other players
    vector<PlayerInfo*> playerInfoVec(nOther);
    for(PlayerInfo*& pi: playerInfoVec) {
        pi = new PlayerInfo();
        pi->posRead = true;
        pi->playerType = HSGlobalData::P_Seeker;
        pi->id = id++;
        pi->currentPos = map->genRandomPos();
        if (hsutils::randomDouble()<.7) {
            //generate only a random pos in some cases
            //AG160331: use location already passed
            pi->hiderObsPosWNoise.set(yRow,yCol); // =  map->genRandomPos();
            //AG160331: now check if should be visible
            pVisib = _map->getVisibilityProb(pi->currentPos, pi->hiderObsPosWNoise, true);
            if (pVisib==0 || hsutils::randomDouble()>pVisib)
                pi->hiderObsPosWNoise.clear(); // simulate not visible
        }
        //else: no obs -> 30% chance
        pi->useObsProb /*hiderObsTrustProb*/ = useObsProb;
    }

    playerInfoVec.push_back(thisPlayer);
    playerInfoVec.push_back(hiderPlayer);

    //generate obs
    HSObservation* obs = new HSObservation(thisPlayer, playerInfoVec);

    //delete PlayerInfo
    for(PlayerInfo* pi: playerInfoVec) {
        delete pi;
    }

    return obs;
}

void SimulatorTest::testSimulator(Simulator *sim, GMap* map, int maxStepDistTest, int numOtherSeekers) {
    Timer t;
    int sTime = t.startTimer();
    long c=0;
    //Pos x, y;
    int xRow,xCol,yRow,yCol; // ,x2Row,x2Col;

    //try cast to HSSimulator for specific functions
    HSSimulator* hsSim = dynamic_cast<HSSimulator*>(sim);

    QVERIFY(sim->getDiscount()>=0 && sim->getDiscount()<=1);


    for(xRow=0; xRow<map->rowCount(); xRow++) {
        for(xCol=0; xCol<map->colCount(); xCol++) {
            if (!map->isObstacle(xRow,xCol)) {

                QVERIFY(map->isPosInMap(xRow,xCol));

                //calc time
                int tid=t.startTimer();
                for(yRow=0; yRow<map->rowCount(); yRow++) {
                    for(yCol=0; yCol<map->colCount(); yCol++) {

                        QVERIFY(map->isPosInMap(yRow,yCol));

                        if (!map->isObstacle(yRow,yCol)) {
                            cout << "***** x/seeker: r"<<xRow<<"c"<<xCol<<" - y/hider: r"<<yRow<<"c"<<yCol<<" *****"<<endl;
                            cout << " distance: " << map->distance(xRow,xCol,yRow,yCol)<<endl;

                            pomcp::HSState hsstate(xRow,xCol,yRow,yCol);

                            //_map->printMap(xRow,xCol);

                            //cout << "Old isVisible: "<< _map->isVisible(y.x,y.y)<<endl;
                            cout << "New isVisible: "<< map->isVisible(xRow,xCol,yRow,yCol,_params->takeDynObstOcclusionIntoAccountWhenLearning,
                                                                       _params->simNotVisibDist)<<endl;

                            cout << "Invisible from pos 1:"<<endl;
                            vector<Pos> invisPosVec = map->getInvisiblePoints(xRow,xCol,_params->takeDynObstOcclusionIntoAccountWhenLearning,_params->simNotVisibDist);
                            FOR(i,invisPosVec.size()) {
                                cout << " "<<i<<"] " << invisPosVec[i].toString() <<endl;
                            }
                            cout <<endl;

                            cout << " -- genInitState --"<<endl;
                            bool v = map->isVisible(xRow,xCol,yRow,yCol,_params->takeDynObstOcclusionIntoAccountWhenLearning,_params->simNotVisibDist);
                            cout << "visiblity: (s->h): "<<v<<endl;
                            cout<<"10 init states: ";

                            //AG131104: use obs
                            //setSeekerHiderPos(x,y,v);
                            //pomcp::HSState obs(xRow,xCol,yRow,yCol);
                            HSObservation* obs = genObs(map, xRow,xCol,yRow,yCol,numOtherSeekers); //todo: test with different

                            QCOMPARE((int)obs->otherSeekersObsVec.size(), numOtherSeekers);
                            QVERIFY(obs->ownSeekerObs.seekerPos.equalsInt(xRow,xCol));
                            QVERIFY(!obs->ownSeekerObs.hiderPos.isSet() || obs->ownSeekerObs.hiderPos.equalsInt(yRow,yCol));

                            //if (!v) obs.hiderPos.clear();

                            FOR(i,10) {
                                pomcp::State* state = sim->genInitState(obs);

                                QVERIFY(state!=nullptr);
                                HSState* hsState = HSState::castFromState(state);
                                QVERIFY(hsState!=nullptr);
                                QVERIFY(hsstate.seekerPos.isSet());
                                QVERIFY(hsstate.hiderPos.isSet());
                                QVERIFY(map->isPosInMap(hsstate.hiderPos));
                                QVERIFY(map->isPosInMap(hsstate.seekerPos));

                                cout << state->toString()<<" ";
                                delete state;
                            }
                            cout << endl;

                            cout << " -- genAllInitStates --"<<_params->numBeliefStates<<endl;
                            vector<State*> allInitStates = sim->genAllInitStates(obs, _params->numBeliefStates);
                            cout << "#"<<allInitStates.size()<<": "<<flush;
                            QCOMPARE((uint)allInitStates.size(), _params->numBeliefStates);
                            for (const State* state : allInitStates) {
                                QVERIFY(state!=nullptr);

                                cout << state->toString()<<" ";

                                const HSState* hsState = HSState::castFromState(state);
                                QVERIFY(hsState!=nullptr);
                                QVERIFY(hsstate.seekerPos.isSet());
                                QVERIFY(hsstate.hiderPos.isSet());
                                QVERIFY(map->isPosInMap(hsstate.hiderPos));
                                QVERIFY(map->isPosInMap(hsstate.seekerPos));
                                //delete *it;
                            }
                            cout << endl;

                            cout << " -- getActions --"<<endl;
                            cout<<"possible actions from "<< hsstate.toString() <<": ";
                            vector<int> actVec;
                            sim->getActions(&hsstate,nullptr,actVec);
                            for(int& a : actVec) {
                                cout << ACTION_COUT(a)<<" ";
                                QVERIFY(a>=0 && a<HSGlobalData::NUM_ACTIONS);
                                Pos newPos=map->tryMove(a,hsstate.seekerPos);
                                QVERIFY(newPos.isSet());
                            }
                            cout << endl;

                            if (hsSim!=nullptr) {
                                cout << " -- getPossibleNextHiderPos --"<<endl;
                                cout<<"possible pos from r"<< xRow<<"c"<<xCol <<": ";
                                vector<Pos> posVec;
                                Pos x(xRow,xCol);
                                hsSim->getPossibleNextHiderPos(x,posVec);
                                for(const Pos& pos : posVec) {
                                    QVERIFY(pos.isSet());
                                    QVERIFY(map->isPosInMap(pos));

                                    cout << pos.toString()<<" ";

                                    //max step distance check
                                    if (_params->useContinuousPos)
                                        QVERIFY(pos.distanceEuc(x)<=M_SQRT2);
                                    else
                                        QVERIFY(pos.distanceEuc(x)==1.0);
                                }
                                cout << endl;

                                cout << " -- getPossibleNextHiderPos (with obs) --"<<endl;
                                cout<<"possible pos from r"<< xRow<<"c"<<xCol <<": ";
                                posVec.clear();
                                hsSim->getPossibleNextHiderPos(x,posVec,obs);
                                //AG160318: should not be empty, because the observation contains location
                                //QVERIFY(!posVec.empty());
                                for(const Pos& pos : posVec) {
                                    QVERIFY(pos.isSet());
                                    QVERIFY(map->isPosInMap(pos));

                                    cout << pos.toString()<<" ";

                                    //max step distance check
                                    if (_params->useContinuousPos)
                                        QVERIFY(pos.distanceEuc(x)<=M_SQRT2);
                                    else
                                        QVERIFY(pos.distanceEuc(x)==1.0);
                                }
                                cout << endl;
                            }


                            cout << " -- setInitialNodeValue --"<<endl;
                            cout<<"set init node value from "<< hsstate.toString() <<": "<<flush;
                            pomcp::Node node(sim,nullptr);
                            //set for each action
                            for(int a=-1; a<HSGlobalData::NUM_ACTIONS; a++) {
                                cout << " a="<<flush;
                                if (a<0)
                                    cout <<"none"<<flush;
                                else
                                    cout<<ACTION_COUT(a)<<flush;
                                sim->setInitialNodeValue(&hsstate,nullptr,&node,a);   //(x,posVec);

                                cout <<" count="<<node.getCount()<<" v="<<node.getValue()<<"; "<<flush;
                            }
                            cout << endl;

                            cout << " -- step --"<<endl;
                            cout<<"step from "<< hsstate.toString() <<": "<<flush;
                            for(int a=0; a<HSGlobalData::NUM_ACTIONS; a++) {

                                stepCheck(sim, hsstate, a);

                                /*
                                double r=0;
                                pomcp::State* obsS = nullptr;
                                pomcp::State* nextS = sim->step(&hsstate,a,obsS,r);

                                QVERIFY(nextS!=nullptr);
                                QVERIFY(obsS!=nullptr);

                                cout<<nextS->toString()<<",o="<<(obsS==nullptr?"NULL":obsS->toString())<<",r="<<r<<"; "<<flush;

                                HSState* hsNextS = HSState::castFromState(nextS);
                                HSState* hsObs = HSState::castFromState(obsS);

                                QVERIFY(hsNextS->seekerPos.isSet());
                                QVERIFY(hsNextS->hiderPos.isSet());
                                QVERIFY(map->isPosInMap(hsNextS->hiderPos));
                                QVERIFY(map->isPosInMap(hsNextS->seekerPos));

                                QVERIFY(hsObs->seekerPos.isSet());
                                QVERIFY(map->isPosInMap(hsObs->seekerPos));
                                QVERIFY(!hsObs->hiderVisible() || map->isPosInMap(hsObs->hiderPos));

                                QVERIFY(hsstate.seekerPos.distanceEuc(hsNextS->seekerPos) < maxStepDistTest);
                                QVERIFY(hsstate.hiderPos.distanceEuc(hsNextS->hiderPos) < maxStepDistTest);

                                delete nextS;
                                delete obsS ;*/
                            }
                            cout << endl;

                            cout << " -- step (with obs) --"<<endl;
                            cout<<"step from "<< hsstate.toString() <<" and obs " <<obs->toString()<< ": "<<flush;
                            for(int a=0; a<HSGlobalData::NUM_ACTIONS; a++) {
                                stepCheck(sim, hsstate, a, true, true);

                                /*
                                double r=0;
                                cout << " a="<<ACTION_COUT(a)<<"->next="<<flush;

                                pomcp::State* obsS = nullptr;
                                pomcp::State* nextS = sim->step(&hsstate,a,obsS,r,true,obs);

                                if (nextS==nullptr) {
                                    //AG160329: can be null if no next state that is consistent with obs.
                                    //check if all actions possible move the hider in an inconsistent state
                                    HSState state(hsstate.seekerPos, hsstate.hiderPos);
                                    for(int a=0; a<HSGlobalData::NUM_ACTIONS; ++a) {
                                        Pos newPos = map->tryMove(a, hsstate.hiderPos);
                                        if (newPos.isSet()) {
                                            //pomcpchecknewbelief..
                                            //the state should be inconsistent with the observation
                                            state.hiderPos = newPos;
                                            //QCOMPARE(sim->isStateConsistentWithObs(state, obs), false);
                                            //it should be inconsistent since nextS is null
                                            QVERIFY(checkStateConsistentWithObs(*sim, state,obs)<=(1.0-CONSIST_CHECK_PROB));
                                        }
                                    }
                                } else {
                                    QVERIFY(nextS!=nullptr);
                                    QVERIFY(obsS!=nullptr);

                                    cout<<nextS->toString()<<",o="<<(obsS==nullptr?"NULL":obsS->toString())<<",r="<<r<<"; "<<flush;

                                    HSState* hsNextS = HSState::castFromState(nextS);
                                    HSState* hsObs = HSState::castFromState(obsS);

                                    QVERIFY(hsNextS->seekerPos.isSet());
                                    QVERIFY(hsNextS->hiderPos.isSet());
                                    QVERIFY(map->isPosInMap(hsNextS->hiderPos));
                                    QVERIFY(map->isPosInMap(hsNextS->seekerPos));

                                    QVERIFY(hsObs->seekerPos.isSet());
                                    QVERIFY(map->isPosInMap(hsObs->seekerPos));
                                    QVERIFY(!hsObs->hiderVisible() || map->isPosInMap(hsObs->hiderPos));

                                    delete nextS;
                                    delete obsS;
                                }*/
                            }
                            cout << endl;

                            cout << " -- step from init states --"<<endl;
                            for (const State* state : allInitStates) {
                                cout<<"step from "<< state->toString() <<": "<<flush;

                                const HSState* stateHS = HSState::castFromState(state) ;

                                int a = hsutils::random(8);

                                stepCheck(sim, *stateHS, a);

                                /*;
                                double r=0;
                                cout << " a="<<ACTION_COUT(a)<<"->next="<<flush;
                                pomcp::State* obsS = nullptr;
                                pomcp::State* nextS = sim->step(stateHS,a,obsS,r);
                                cout<<nextS->toString()<<",o="<<(obsS==nullptr?"NULL":obsS->toString())<<",r="<<r<<"; "<<flush;

                                QVERIFY(nextS!=nullptr);
                                QVERIFY(obsS!=nullptr);

                                HSState* hsNextS = HSState::castFromState(nextS);
                                HSState* hsObs = HSState::castFromState(obsS);

                                QVERIFY(hsNextS->seekerPos.isSet());
                                QVERIFY(hsNextS->hiderPos.isSet());
                                QVERIFY(map->isPosInMap(hsNextS->hiderPos));
                                QVERIFY(map->isPosInMap(hsNextS->seekerPos));

                                QVERIFY(hsObs->seekerPos.isSet());
                                QVERIFY(map->isPosInMap(hsObs->seekerPos));
                                QVERIFY(!hsObs->hiderVisible() || map->isPosInMap(hsObs->hiderPos));

                                QVERIFY(stateHS->seekerPos.distanceEuc(hsNextS->seekerPos) < maxStepDistTest);
                                QVERIFY(stateHS->hiderPos.distanceEuc(hsNextS->hiderPos) < maxStepDistTest);

                                delete nextS;
                                delete obsS;*/
                            }
                            cout << endl;

                            cout << " -- step from init states (with obs) --"<<endl;
                            for (State* state : allInitStates) {
                                cout<<"step from "<< state->toString() <<" and obs " <<obs->toString()<<": "<<flush;

                                HSState* stateHS = HSState::castFromState(state) ;
                                int a = hsutils::random(8);
                                stepCheck(sim, *stateHS, a, true, true);

                                /*double r=0;
                                //int o=-1;
                                //HSState obs;
                                cout << " a="<<ACTION_COUT(a)<<"->next="<<flush;
                                pomcp::State* obsS = nullptr;
                                pomcp::State* nextS = sim->step(stateHS,a,obsS,r,true,obs);

                                if (nextS==nullptr)
                                cout<<nextS->toString()<<",o="<<(obsS==NULL?"NULL":obsS->toString())<<",r="<<r<<"; "<<flush;

                                QVERIFY(nextS!=nullptr);
                                QVERIFY(obsS!=nullptr);
                                cout<<nextS->toString()<<",o="<<(obsS==nullptr?"NULL":obsS->toString())<<",r="<<r<<"; "<<flush;

                                HSState* hsNextS = HSState::castFromState(nextS);
                                HSState* hsObs = HSState::castFromState(obsS);

                                QVERIFY(hsNextS->seekerPos.isSet());
                                QVERIFY(hsNextS->hiderPos.isSet());
                                QVERIFY(map->isPosInMap(hsNextS->hiderPos));
                                QVERIFY(map->isPosInMap(hsNextS->seekerPos));

                                QVERIFY(hsObs->seekerPos.isSet());
                                QVERIFY(map->isPosInMap(hsObs->seekerPos));
                                QVERIFY(!hsObs->hiderVisible() || map->isPosInMap(hsObs->hiderPos));

                                delete nextS;
                                delete obsS;
                                delete state; //since last test*/
                            }
                            cout << endl;

                            cout << " -- isFinal --"<<endl;
                            cout << hsstate.toString()<<": "<< sim->isFinal(&hsstate)<<endl;

                            cout << "-- immediateReward --"<<endl;
                            cout << hsstate.toString()<<": "<< sim->getImmediateReward(&hsstate)<<endl;


                            delete obs;

                        } // if y!=obs
                    } // for yCol

                } //for yRow

                c++;
                long runT = t.stopTimer(tid);
                cout << "time field: "<<runT<<endl;
                long iLeft = map->numFreeCells()-c;
                cout << "Itts left: "<<iLeft<<"; time: "<<(iLeft*runT)<<" s = "<<(iLeft*runT/3600.0)<<" h"<<endl;
            } //if x!=obst
        } //for xCol

    } //for xRow

    long totTime = t.getTime(sTime);
    cout << "TOTAL TIME: "<<totTime<<" s = "<<(totTime/3600.0)<<" h"<<endl<<endl;
}

void SimulatorTest::stepCheck(pomcp::Simulator *sim, const pomcp::HSState& hsstate, int a, bool genOutObs, bool useInObs, int numOtherSeekers) {
    double r=0;
    cout << " a="<<ACTION_COUT(a)<<"->next="<<flush;
    pomcp::State* obsS = nullptr;
    pomcp::HSObservation* obs = nullptr;
    if (useInObs) {
        obs = genObs(_map, hsstate.seekerPos.row(), hsstate.seekerPos.col(), hsstate.hiderPos.row(), hsstate.hiderPos.col(), numOtherSeekers);
    }
    pomcp::State* nextS = sim->step(&hsstate,a,obsS,r,genOutObs,obs);

    if (obs!=nullptr && nextS==nullptr) {
            //this only happens when HSSimulator::getPossibleNextHiderPos returns an empty list of possible next states
            //this means that all states were inconsistent
            //so: pConsist < 1.0, and for all in our case the chosen p>pConsist
            QVERIFY(obsS==nullptr);
            QVERIFY(_params->pomcpCheckNewBeliefGen); //used by HSSimulator::getPossibleNextHiderPos

            //AG160329: can be null if no next state that is consistent with obs.
            //check if all actions possible move the hider in an inconsistent state
            HSState state(hsstate.seekerPos, hsstate.hiderPos);
            for(int a=0; a<HSGlobalData::NUM_ACTIONS; ++a) {
                    Pos newPos = _map->tryMove(a, hsstate.hiderPos);
                    if (newPos.isSet()) {
                            //pomcpchecknewbelief..
                            //the state should be inconsistent with the observation
                            state.hiderPos = newPos;
                            //QCOMPARE(sim->isStateConsistentWithObs(state, obs), false);
                            //it should be inconsistent since nextS is null
                            ///double p = checkStateConsistentWithObs(*sim, state,obs);
                            /*if (p>(1.0-CONSIST_CHECK_PROB)) {
                                qDebug()<<"expected lower p, instead of: "<<p;
                            }
                            QVERIFY(p<=(1.0-CONSIST_CHECK_PROB));*/
                            //for it to accept as inconsistent it should have failed at least once
                            ///QVERIFY(p<1.0);

                            double p = checkStateConsistentWithObsProb(*sim, &state, obs);
                            QVERIFY(p<1.0);
                    }
            }
    } else {
            QVERIFY(nextS!=nullptr);
            QVERIFY(obsS!=nullptr);

            cout<<nextS->toString()<<",o="<<(obsS==nullptr?"NULL":obsS->toString())<<",r="<<r<<"; "<<flush;

            HSState* hsNextS = HSState::castFromState(nextS);
            HSState* hsObs = HSState::castFromState(obsS);

            QVERIFY(hsNextS->seekerPos.isSet());
            QVERIFY(hsNextS->hiderPos.isSet());
            QVERIFY(_map->isPosInMap(hsNextS->hiderPos));
            QVERIFY(_map->isPosInMap(hsNextS->seekerPos));

            QVERIFY(hsObs->seekerPos.isSet());
            QVERIFY(_map->isPosInMap(hsObs->seekerPos));
            QVERIFY(!hsObs->hiderVisible() || _map->isPosInMap(hsObs->hiderPos));

            delete nextS;
            delete obsS;
    }

    if (useInObs)
        delete obs;
}

bool SimulatorTest::skipTestFunc(const char *funcName) {
    QString funcNameStr = QString::fromLatin1(funcName);
    //qDebug() <<"TEST "<<funcNameStr;
    for(const QString& skipFuncName : SKIP_TEST_FUNCS_LIST) {
        QString skipFuncName2 = "::"+skipFuncName+"(";
        if (funcNameStr.contains(skipFuncName2)) {
            qDebug() << "Skipping: "<<funcName;
            return true;
        }
    }
    return false;
}

#ifdef OLD_CODE
void HSStateTest::createTest() {
    HSState s1(1,2,3,4);
    HSState s2(Pos(3,4),Pos(5,6));
    Pos empty;
    HSState s3(empty,empty);

    QVERIFY(s1.hiderVisible());
    QVERIFY(s2.hiderVisible());
    QVERIFY(!s3.hiderVisible());
}

void HSStateTest::compareTest() {
    HSState s1(1,2,3,4);
    HSState s2(Pos(3,4),Pos(5,6));
    Pos empty;
    HSState s3(Pos(6,4),empty);
    HSState s4(Pos(1,2),Pos(3,4));

    QCOMPARE(s1,s4);
    QVERIFY(s1==s4);
    QVERIFY(s1!=s2);
    QVERIFY(s1<s2);
    QVERIFY(s2>s1);
    QVERIFY(s1.getHash()==s4.getHash());
    QVERIFY(s1.getHash()!=s2.getHash());

    QVERIFY(s2.hiderVisible());
    QVERIFY(!s3.hiderVisible());

    HSState s5(1.1,2.5,3.6,4.7);
    s5.convPosToInt();
    QVERIFY(s5==s1);

}


void HSSimulator::testAllFunctions() {
    cout << " HSSimulator::testAllFunctions()"<<endl;

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
                            cout<<"10 next random states from "<< hsstate.toString() <<": ";
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
#endif



