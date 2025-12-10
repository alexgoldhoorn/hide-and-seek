#include "test/include/Base/autoplayer-test.h"
#include <QTest>
#include <iostream>
#include <cassert>

#include "Base/hsglobaldata.h"

#include "Utils/timer.h"
#include "Utils/generic.h"

#include "Base/autoplayer.h"

using namespace std;


AutoPlayerTest::AutoPlayerTest(SeekerHSParams *params, GMap *map, AutoPlayer *autoPlayer) :
    _params(params), _map(map), _autoPlayer(autoPlayer) {
}

void AutoPlayerTest::initTestCase() {
    //set player ID, 0, should be 1st in list
    _autoPlayer->playerInfo.id = 0;
    //set player type
    //_autoPlayer->playerInfo.playerType = HSGlobalData::P_Seeker;
    //if not set, set number of players
    if (_params->numPlayersReq == 0)
        _params->numPlayersReq = OBS_NUM_OTHER_SEEKERS+1;
    //if not set, set max number of steps
    if (_params->stopAfterNumSteps == 0)
        _params->stopAfterNumSteps = NUM_TEST_STEPS;
}

void AutoPlayerTest::autoPlayerTest() {
    //test all functions based on current params
    if (_params->seekerSendsMultiplePoses()) {
        autoPlayerMultiTest();
    } else {
        autoPlayerSingleTest();
    }
}

void AutoPlayerTest::cleanupTestCase() {
}

void AutoPlayerTest::autoPlayerSingleTest() {
    cout << "--- TEST - AutoPlayerTest::autoPlayerSingleTest --- "<<endl;
    QVERIFY(_params->numPlayersReq==2);

    //set up players    
    vector<PlayerInfo*> playerInfoVec;
    playerInfoVec.push_back(&_autoPlayer->playerInfo);
    createPlayerInfoVec(playerInfoVec, 0, 1, 1);
    QVERIFY(_autoPlayer->playerInfo.isSeeker());
    QVERIFY(_autoPlayer->playerInfo.useObsProb==1);
    QCOMPARE((int)playerInfoVec.size(),2);
    QVERIFY(playerInfoVec[1]->playerType==HSGlobalData::P_Hider);

    cout << "*** INIT ***"<<endl;
    _autoPlayer->initBelief(_map, playerInfoVec[1]);

    //TODO QVERIFY

    int lastA = -1;


    for(uint i=1; i<=_params->stopAfterNumSteps; i++) {
        cout <<"***It. "<<(i)<<" / "<<_params->stopAfterNumSteps<<"***"<<endl;

        if (lastA!=-1) {
            cout << "***["<<i<<"] READ NEW ***"<<endl;
            updatePlayerInfoVec(playerInfoVec);
        }

        cout << "***["<<i<<"] NEXT POS ***"<<endl;
        Pos nextPos = _autoPlayer->getNextPos(-1, &lastA);

        QVERIFY(nextPos.isSet());
        QCOMPARE(_autoPlayer->playerInfo.nextPos,nextPos);
        QVERIFY(_autoPlayer->playerInfo.chosenGoalPos.isSet());

        //TODO: more checks

    }

    deletePlayerInfoVec(playerInfoVec);
}

void AutoPlayerTest::autoPlayerMultiTest() {
    cout << "--- TEST - AutoPlayerTest::autoPlayerMultiTest --- "<<endl;
    QVERIFY(_params->solverType==SeekerHSParams::SOLVER_TWO_HB_EXPL || _params->solverType==SeekerHSParams::SOLVER_MULTI_HB_EXPL);

    if (_params->solverType==SeekerHSParams::SOLVER_TWO_HB_EXPL)
        QVERIFY(_params->numPlayersReq==3);
    else
        QVERIFY(_params->numPlayersReq>1);

    //set up players
    double p = 1.0/(_params->numPlayersReq-1); //+1
    cout << "p="<<p<<endl;
    vector<PlayerInfo*> playerInfoVec;
    playerInfoVec.push_back(&_autoPlayer->playerInfo);
    int hiderI = createPlayerInfoVec(playerInfoVec, _params->numPlayersReq-2, 1, p);
    QVERIFY(hiderI>=0);

    QCOMPARE((uint)playerInfoVec.size(),_params->numPlayersReq);  //1 this seeker, 1 hider
    QVERIFY(playerInfoVec[1]->playerType==HSGlobalData::P_Hider);


    //small test of player info comparison
    PlayerInfo* pi2=&_autoPlayer->playerInfo;
    QVERIFY(*pi2==_autoPlayer->playerInfo);
    QVERIFY(!(*pi2!=_autoPlayer->playerInfo));
    bool found=false;
    for(PlayerInfo* pi : playerInfoVec) {
        if (*pi==_autoPlayer->playerInfo) {
            found=true; break;
        } else {
            assert(*pi!=_autoPlayer->playerInfo);
        }
    }


    cout << "*** INIT ***"<<endl;
    _autoPlayer->initBeliefMulti(_map, playerInfoVec, 0, hiderI);

    int lastA = -1;

    for(uint i=1; i<=_params->stopAfterNumSteps; i++) {
        cout <<"***It. "<<i<<" / "<<_params->stopAfterNumSteps<<"***"<<endl;

        if (i>1) { //lastA!=-1) {
            cout << "***["<<i<<"] READ NEW ***"<<endl;
            QVERIFY(_autoPlayer->playerInfo.currentPos.isSet());
            updatePlayerInfoVec(playerInfoVec);

            QVERIFY(_autoPlayer->playerInfo.currentPos.isSet());
            QVERIFY(_autoPlayer->playerInfo.previousPos.isSet());
            lastA = _autoPlayer->deduceAction();
            cout <<"Action from "<<_autoPlayer->playerInfo.previousPos.toString()<<" to "<<_autoPlayer->playerInfo.currentPos.toString()<<":"<<lastA<<flush;
            QVERIFY(lastA>=0 && lastA<HSGlobalData::NUM_ACTIONS);
            cout << " = "<<ACTION_COUT(lastA)<<endl;
        }

        switch(_params->solverType) {
        case SeekerHSParams::SOLVER_TWO_HB_EXPL: {
            cout << "***["<<i<<"] CALC NEXT ROBOT POSES 2 ***"<<endl;
            bool ok = _autoPlayer->calcNextRobotPoses2(lastA);

            QVERIFY(ok);
            //this player should have goal poses
            QVERIFY(_autoPlayer->playerInfo.multiHasGoalPoses);
            QCOMPARE((int)_autoPlayer->playerInfo.multiGoalIDVec.size(), 2);
            //QCOMPARE((int)_autoPlayer->playerInfo.multiGoalBeliefVec.size(), 2);
            QCOMPARE((int)_autoPlayer->playerInfo.multiGoalBPosesVec.size(), 2);
            //check contents
            QCOMPARE(_autoPlayer->playerInfo.multiGoalIDVec[0], _autoPlayer->playerInfo.id);
            QVERIFY(_autoPlayer->playerInfo.multiGoalBPosesVec[0].isSet());
            QVERIFY(_map->isPosInMap(_autoPlayer->playerInfo.multiGoalBPosesVec[0]));
            QVERIFY(_autoPlayer->playerInfo.multiGoalIDVec[1]!=_autoPlayer->playerInfo.id &&
                    _autoPlayer->playerInfo.multiGoalIDVec[1]!=playerInfoVec[hiderI]->id);

            QVERIFY(_autoPlayer->playerInfo.multiGoalBPosesVec[0].b==-1 ||
                    (_autoPlayer->playerInfo.multiGoalBPosesVec[0].b>=0 && _autoPlayer->playerInfo.multiGoalBPosesVec[0].b<=1));
            QVERIFY(_autoPlayer->playerInfo.multiGoalBPosesVec[1].b==-1 ||
                    (_autoPlayer->playerInfo.multiGoalBPosesVec[1].b>=0 && _autoPlayer->playerInfo.multiGoalBPosesVec[1].b<=1));

            //TODO: set multi goal poses of others
            setMultiGoals2PlayerInfoVec(playerInfoVec);

            break;
        }
        case SeekerHSParams::SOLVER_MULTI_HB_EXPL: {
            cout << "***["<<i<<"] CALC NEXT HB LIST ***"<<endl;
            bool ok = _autoPlayer->calcNextHBList(lastA);
            QVERIFY(ok);
            QVERIFY(_autoPlayer->playerInfo.multiHasHBPoses);

            QVERIFY(_autoPlayer->playerInfo.multiHBPosVec[0].b==-1 ||
                    (_autoPlayer->playerInfo.multiHBPosVec[0].b>=0 && _autoPlayer->playerInfo.multiHBPosVec[0].b<=1));
            QVERIFY(_autoPlayer->playerInfo.multiHBPosVec[1].b==-1 ||
                    (_autoPlayer->playerInfo.multiHBPosVec[1].b>=0 && _autoPlayer->playerInfo.multiHBPosVec[1].b<=1));

            //TODO

            break;
        }
        default:
            QFAIL("should be different solver type");
            break;
        }

        cout << "***["<<i<<"] NEXT POS: SELECT ROBOT POS MULTI ***"<<endl;
        Pos nextPos = _autoPlayer->selectRobotPosMulti();

        QVERIFY(nextPos.isSet());
        QCOMPARE(_autoPlayer->playerInfo.nextPos, nextPos);
        QVERIFY(_autoPlayer->playerInfo.chosenGoalPos.isSet());

        //TODO: more checks
    }

    deletePlayerInfoVec(playerInfoVec);
}


int AutoPlayerTest::createPlayerInfoVec(std::vector<PlayerInfo *> &playerInfoVec,
                                         int nSeekers, int nHiders, double prob) {
    assert(prob>0);
    assert(prob<=1);
    int hi = -1;

    //first set info of existing players (seeker + hider)
    for(PlayerInfo* playerI : playerInfoVec) {
        assert(playerI->id>=0);
        playerI->currentPos = _map->genRandomPos();
        if (playerI->isSeeker()) {
            /*playerI->hiderObsTrustProb =*/ playerI->useObsProb = prob;
            if (randomDouble()>OBS_HIDER_NOT_SEEN_P) // X% chance of setting pos
                playerI->hiderObsPosWNoise = _map->genRandomPos();
        }
        playerI->posRead = true;
        playerI->initPosSet = true;
    }

    //randomly creating player info out of map
    for(int i=0; i<nSeekers+nHiders; i++) {
        PlayerInfo* playerI = new PlayerInfo();
        playerI->id = playerInfoVec.size();
        playerI->posRead = true;
        playerI->initPosSet = true;
        playerI->currentPos = _map->genRandomPos();
        /*playerI->hiderObsTrustProb =*/ playerI->useObsProb = prob;
        if (randomDouble()>OBS_HIDER_NOT_SEEN_P) // X% chance of setting pos
            playerI->hiderObsPosWNoise = _map->genRandomPos();

        if (i<nHiders) {
            playerI->playerType = HSGlobalData::P_Hider;
            if (hi==-1) hi = playerInfoVec.size();
        } else {
            playerI->playerType = HSGlobalData::P_Seeker;
        }

        playerInfoVec.push_back(playerI);
    }

    return hi;
}

void AutoPlayerTest::deletePlayerInfoVec(std::vector<PlayerInfo*> playerInfoVec) {
    for(PlayerInfo*& playerI : playerInfoVec) {
        //check if playerI is not of autoPlayer
        if (_autoPlayer->playerInfo!=*playerI)
            delete playerI;
    }
}

void AutoPlayerTest::updatePlayerInfoVec(std::vector<PlayerInfo *> &playerInfoVec) {
    for(size_t i=1; i<playerInfoVec.size(); i++) {
        PlayerInfo* pi = playerInfoVec[i];

        assert(pi!=NULL);
        assert(*pi!=_autoPlayer->playerInfo);

        pi->prepareNextStep();

        //  have a porb of not setting other seeker
        if (!_params->seekerSendsMultiplePoses() || randomDouble()<MULTISEEK2_CON_P) {
            //QVERIFY(pi->previousPos.isSet());
            if (pi->previousPos.isSet())
                pi->currentPos = randMove(_map, pi->previousPos);
            else
                pi->currentPos = _map->genRandomPos();
            QVERIFY(pi->currentPos.isSet());
            if (pi->isSeeker() && randomDouble()>OBS_HIDER_NOT_SEEN_P) { // X% chance of setting pos
                pi->hiderObsPosWNoise = _map->genRandomPos();
                QVERIFY(pi->hiderObsPosWNoise .isSet());
            }

            pi->posRead = true;
        } else {
            pi->posRead = false;
        }
    }

    //now current seeker
    _autoPlayer->playerInfo.prepareNextStep();
    _autoPlayer->playerInfo.currentPos = _map->genRandomPos(); //NOTE: we could use the .nextStep
    _autoPlayer->playerInfo.posRead = true;
}

void AutoPlayerTest::setMultiGoals2PlayerInfoVec(std::vector<PlayerInfo *> &playerInfoVec) {
    PlayerInfo* otherPI = NULL;
    for(PlayerInfo* pi: playerInfoVec) {
        if (pi->isSeeker() && pi->id!=_autoPlayer->playerInfo.id) {
            otherPI = pi;
            break;
        }
    }
    QVERIFY(otherPI!=NULL);

    if (randomDouble()<=MULTISEEK2_CON_P) {
        otherPI->multiHasGoalPoses = true;
        //otherPI->multiGoalBeliefVec.resize(2);
        otherPI->multiGoalIDVec.resize(2);
        otherPI->multiGoalBPosesVec.resize(2);
        //now fill
        otherPI->multiGoalIDVec[0] = otherPI->id;
        otherPI->multiGoalIDVec[1] = _autoPlayer->playerInfo.id;

        otherPI->multiGoalBPosesVec[0] = _map->genRandomPos();
        otherPI->multiGoalBPosesVec[1] = _map->genRandomPos();

        if (randomDouble()<MULTISEEK2_GOAL_NOBEL_P) {
            otherPI->multiGoalBPosesVec[0].b=otherPI->multiGoalBPosesVec[1].b=-1;
        } else {
            otherPI->multiGoalBPosesVec[0].b = randomDouble();
            otherPI->multiGoalBPosesVec[1].b = randomDouble();
        }

    } //else: no connection -> no poses/belief

}


/*HSObservation* MCTSTest::genObs(GMap *map, const Pos& seekerPos, const Pos& hiderObsPos, int nOther) {
    double p = 1.0/(nOther+1);
    PlayerInfo thisPlayer;
    thisPlayer.currentPos = seekerPos;;
    thisPlayer.hiderObsPosWNoise = hiderObsPos;
    thisPlayer.hiderObsTrustProb = p;
    thisPlayer.posRead = true;
    int id = 0;
    thisPlayer.id = id++;

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
        if (randomDouble()<.7)
            pi->hiderObsPosWNoise = map->genRandomPos();
        //else: no obs -> 30% chance
        pi->hiderObsTrustProb = p;
    }

    playerInfoVec.push_back(hiderPlayer);

    //generate obs
    HSObservation* obs = new HSObservation(&thisPlayer, playerInfoVec);

    //delete PlayerInfo
    for(PlayerInfo* pi: playerInfoVec) {
        delete pi;
    }

    //verify output
    double sumP = obs->ownSeekerObs.prob;
    for(const HSState& s : obs->otherSeekersObsVec) {
        sumP += s.prob;
    }
    assert(abs(sumP-1.0)<0.0001); //TODO should be double epsilon

    return obs;
}

HSState* MCTSTest::genRandState(GMap *map) {
    HSState* state = new HSState();
    state->prob = 0.5;
    state->seekerPos = map->genRandomPos();
    state->hiderPos = map->genRandomPos();
    return state;
}*/

Pos AutoPlayerTest::randMove(GMap *map, const Pos &pos) {
    Pos nextPos;

    do {
        int a = random(HSGlobalData::NUM_ACTIONS-1);
        nextPos = map->tryMove(a,pos);
    } while (!nextPos.isSet());

    return nextPos;
}

