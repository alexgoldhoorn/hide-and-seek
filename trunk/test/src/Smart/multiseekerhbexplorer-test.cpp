#include "test/include/Smart/multiseekerhbexplorer-test.h"
#include <QTest>
#include <iostream>

#include "Base/hsglobaldata.h"
#include "Smart/multiseekerhbexplorer.h"
#include "Base/abstractautoplayer.h"
#include "Base/posxy.h"

#include "Utils/generic.h"

using namespace std;


class AbstractAutoPlayerTester : public AbstractAutoPlayer {
public:
    AbstractAutoPlayerTester(SeekerHSParams* params, GMap* map) : AbstractAutoPlayer(params, map) {
    }
    virtual ~AbstractAutoPlayerTester() {}

    virtual bool tracksBelief() const {
        return true;
    }

    virtual double getBelief(int r, int c) {
        return 0.1;
    }

    virtual bool handles2Obs() const {
        return true;
    }

protected:
    virtual bool initBeliefRun() {
        return true;
    }
    virtual Pos getNextPosRun(int actionDone=-1, int* newAction=NULL) {
        playerInfo.nextPos = _map->genRandomPos();
        return playerInfo.nextPos;
    }
};

//!
class MultiSeekerHBExplorerTester : public MultiSeekerHBExplorer
{
public:

    MultiSeekerHBExplorerTester(SeekerHSParams* params, AutoPlayer* autoPlayerWBelief) : MultiSeekerHBExplorer(params,autoPlayerWBelief) {
    }

    virtual ~MultiSeekerHBExplorerTester() {}

    virtual void combineHBPoses(std::vector<BPos>& hbPosesVec) {
        MultiSeekerHBExplorer::combineHBPoses(hbPosesVec);
    }
    virtual bool chooseHiderObsFromMulti(Pos& chosenHider) {
        return MultiSeekerHBExplorer::chooseHiderObsFromMulti(chosenHider);
    }
    virtual int calcExplorationEvaluation(const Pos& seekerPos, const std::vector<BPos>& highestBeliefPosVec, std::vector<double>& uVec,
                                 bool updateU = true) {
        return MultiSeekerHBExplorer::calcExplorationEvaluation(seekerPos, highestBeliefPosVec, uVec, updateU);
    }
    virtual void useExplorer() {
        MultiSeekerHBExplorer::useExplorer();
    }

};

void MultiSeekerHBExplorerTest::initTestCase() {
    _params = new SeekerHSParams();
    _params->useContinuousPos = true;
    _params->rewardType = SeekerHSParams::REWARD_FIND_REV_DIST;
    _params->numBeliefStates = 10;
    _params->multiSeekerExplorerCheckNPoints = 10;
    _params->solverType = SeekerHSParams::SOLVER_MULTI_HB_EXPL;
    _params->gameType = HSGlobalData::GAME_FIND_AND_FOLLOW_MULTI_ROB;
    _params->numPlayersReq = OTHER_SEEKERS_N+2;
    _params->minDistBetweenRobots = 0;

    _map = new GMap(4,4,_params);
    //gen map
    _map->addObstacle(1,1);
    _map->addObstacle(1,2);
    _map->setMapFixed();
    //sim
//    _sim = new HSSimulator(_map,_params);
}

void MultiSeekerHBExplorerTest::cleanupTestCase() {
//    delete _sim;
    delete _map;
    delete _params;
}

void MultiSeekerHBExplorerTest::createAndCompareTest() {
    AbstractAutoPlayerTester aplayer(_params,_map);
    QVERIFY(aplayer.handles2Obs());
    AutoPlayer* autoPlayer = &aplayer;
    QVERIFY(autoPlayer->handles2Obs());

    MultiSeekerHBExplorerTester msTester(_params,&aplayer);
    vector<PlayerInfo*> playerVec(OTHER_SEEKERS_N+2);
    double p = 1.0/(OTHER_SEEKERS_N+1);

    msTester.playerInfo.posRead = msTester.playerInfo.initPosSet = true;
    msTester.playerInfo.currentPos = _map->genRandomPos();
    msTester.playerInfo.id = 0;
    msTester.playerInfo.playerType = HSGlobalData::P_Seeker;
    msTester.playerInfo.useObsProb = p;
    playerVec[0] = &msTester.playerInfo;

    PlayerInfo hiderPI;
    hiderPI.posRead=hiderPI.initPosSet=true;
    hiderPI.currentPos = _map->genRandomPos();
    hiderPI.playerType = HSGlobalData::P_Hider;
    hiderPI.id = 1;
    playerVec[1] = &hiderPI;

    vector<PlayerInfo> otherSeekerPlayerVec(OTHER_SEEKERS_N);
    for(size_t i=0; i<OTHER_SEEKERS_N; i++) {
        otherSeekerPlayerVec[i].posRead = otherSeekerPlayerVec[i].initPosSet = true;
        otherSeekerPlayerVec[i].currentPos = _map->genRandomPos();
        otherSeekerPlayerVec[i].playerType = HSGlobalData::P_Seeker;
        otherSeekerPlayerVec[i].id = i+2;
        otherSeekerPlayerVec[i].useObsProb = p;
        playerVec[i+2] = &otherSeekerPlayerVec[i];
    }

    //init
    msTester.initBeliefMulti(_map, playerVec, 0, 1);

    //calc hb list
    msTester.calcNextHBList();

    QVERIFY(msTester.playerInfo.multiHasHBPoses);
    QVERIFY(!msTester.playerInfo.multiHBPosVec.empty());
    QVERIFY((int)msTester.playerInfo.multiHBPosVec.size()<=_params->multiSeekerExplorerCheckNPoints);

    cout << "HB points of this:"<<endl;
    for(BPos bpos : msTester.playerInfo.multiHBPosVec) {
        cout << " - " <<bpos.toString()<<endl;//" ";
    }cout <<endl;

    QVERIFY(msTester.playerInfo.multiHBPosVec[0].b>=0);

    //generate other highest belief
    cout<<"other hb points:"<<endl;
    for(PlayerInfo& pInfo : otherSeekerPlayerVec) {
        cout<<" from "<<pInfo.toString()<<endl;
        pInfo.multiHasHBPoses = true;
        pInfo.multiHBPosVec.clear();
        int n = hsutils::random(1,_params->multiSeekerExplorerCheckNPoints);
        for(int i=0;i<n;i++) {
            BPos bpos(_map->genRandomPos().convertValuesToInt(),0.1);
            pInfo.multiHBPosVec.push_back(bpos);
            cout <<" - "<<bpos.toString()<<endl;
        }
    }

    //try to add again a hb pos of playerinfo
    BPos repPos = msTester.playerInfo.multiHBPosVec[0];
    otherSeekerPlayerVec[0].multiHBPosVec.push_back(repPos);

    //combine
    vector<BPos> hbPosVec;
    msTester.combineHBPoses(hbPosVec);

    QVERIFY(!hbPosVec.empty());
    QVERIFY(hbPosVec.size()>=msTester.playerInfo.multiHBPosVec.size());

    cout << "Combined points:"<<endl;
    cout << "HB points of this:"<<endl;
    for(BPos bpos : hbPosVec) {
        cout << " - " << bpos.toString()<<endl;
    }cout <<endl;

    //choose hider pos
    //now all hider obs not set, so should be ok
    Pos chosenHiderP;
    bool chosenHiderPConsist = msTester.chooseHiderObsFromMulti(chosenHiderP);
    QVERIFY(chosenHiderPConsist);
    QVERIFY(!chosenHiderP.isSet());

    //set player's observation
    msTester.playerInfo.hiderObsPosWNoise = _map->genRandomPos();
    chosenHiderPConsist = msTester.chooseHiderObsFromMulti(chosenHiderP);
    QVERIFY(chosenHiderPConsist);
    QVERIFY(chosenHiderP.isSet());
    QCOMPARE(chosenHiderP, msTester.playerInfo.hiderObsPosWNoise);

    //set other player's obs also, and add a bit of noise
    otherSeekerPlayerVec[0].hiderObsPosWNoise = chosenHiderP;
    otherSeekerPlayerVec[0].hiderObsPosWNoise.add(0.5,-.3);
    chosenHiderPConsist = msTester.chooseHiderObsFromMulti(chosenHiderP);
    QVERIFY(chosenHiderPConsist);
    QVERIFY(chosenHiderP.isSet());
    QCOMPARE(chosenHiderP, msTester.playerInfo.hiderObsPosWNoise); //TODO: change if we really take a (weighted) average over visible poses

    //now add more noise to the other pos, this should make it inconsistent
    otherSeekerPlayerVec[0].hiderObsPosWNoise.add(0.5,-.3);
    chosenHiderPConsist = msTester.chooseHiderObsFromMulti(chosenHiderP);
    QVERIFY(!chosenHiderPConsist);
    QVERIFY(!chosenHiderP.isSet());

    //now test calcExplorationEvaluation
    //generate hb point list
    hbPosVec.resize(2);
    hbPosVec[0] = _map->genRandomPos();
    hbPosVec[1] = _map->genRandomPos();
    vector<double> uVec(hbPosVec.size(),1.0);
    int seekerI1 = msTester.calcExplorationEvaluation(hbPosVec[0], hbPosVec, uVec, true);
    QVERIFY(seekerI1>=0 && seekerI1<(int)hbPosVec.size());
    QVERIFY(uVec[0]<1.0);
    QCOMPARE(seekerI1, 0);
    int seekerI2 = msTester.calcExplorationEvaluation(hbPosVec[1], hbPosVec, uVec, true);
    QVERIFY(seekerI2>=0 && seekerI2<(int)hbPosVec.size());
    QVERIFY(uVec[1]<1.0);
    QCOMPARE(seekerI2,1);

    //MultiSeekerHBExplorer::useExplorer
    msTester.useExplorer();
    //now of all players chosen goal pos should be set
    for(PlayerInfo* pInfo : playerVec) {
        QVERIFY(!pInfo->isSeeker() || pInfo->chosenGoalPos.isSet());
    }

    //AG150801: test adding the same Hb poses twice
    hbPosVec.clear();
    msTester.playerInfo.multiHasHBPoses = true;
    msTester.playerInfo.multiHBPosVec.resize(1);
    msTester.playerInfo.multiHBPosVec[0] = BPos(2.5,3.5,0.3);
    //set same multihbpos for 1 other seeker
    otherSeekerPlayerVec[0].multiHasHBPoses = true;
    otherSeekerPlayerVec[0].multiHBPosVec.resize(1);
    otherSeekerPlayerVec[0].multiHBPosVec[0] = BPos(2.5,3.5,0.3);
    //others not read
    for(size_t i=1;i<otherSeekerPlayerVec.size(); i++) {
        otherSeekerPlayerVec[i].multiHasHBPoses = false;
        otherSeekerPlayerVec[i].multiHBPosVec.clear();
    }
    //now combine
    msTester.combineHBPoses(hbPosVec);
    cout<<"New hbpos after combining:"<<endl;
    for(BPos& p : hbPosVec) cout <<" - "<<p.toString()<<endl;
    QCOMPARE((int)hbPosVec.size(), 1);
    QVERIFY(hbPosVec[0]==BPos(2.5,3.5,0.1));

    //AG150803: test adding random Hb poses twice
    hbPosVec.clear();
    msTester.playerInfo.multiHasHBPoses = true;
    otherSeekerPlayerVec[0].multiHasHBPoses = true;
    msTester.playerInfo.multiHBPosVec.resize(10);
    otherSeekerPlayerVec[0].multiHBPosVec.resize(10);
    //set same multihbpos for 1 other seeker
    for(size_t i=0;i<10; i++) {
        msTester.playerInfo.multiHBPosVec[i] = _map->genRandomPos();
        msTester.playerInfo.multiHBPosVec[i].b = 0.1;

        //convert to PosXY
        PosXY posXY;
        posXY.fromBPos(msTester.playerInfo.multiHBPosVec[i], _params);
        otherSeekerPlayerVec[0].multiHBPosVec[i] = posXY.toBPos(_params);

        otherSeekerPlayerVec[0].multiHBPosVec[i].b = 0.2;
    }

    //now combine
    msTester.combineHBPoses(hbPosVec);
    cout<<"New hbpos after combining:"<<endl;
    for(BPos& p : hbPosVec) cout <<" - "<<p.toString()<<endl;
    QCOMPARE((int)hbPosVec.size(), 10);
    //QVERIFY(hbPosVec[0]==BPos(2.5,3.5,0.1));
}




