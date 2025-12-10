#include "test/include/POMCP/hsobservation-test.h"
#include <QTest>
#include <QDebug>

#include "POMCP/hsobservation.h"

#include "Base/hsglobaldata.h"

using namespace std;
using namespace pomcp;

void HSObservationTest::createCompareTest() {
    HSObservation hsObs;

    PlayerInfo p1;
    PlayerInfo p2;
    PlayerInfo ph;//hider
    vector<PlayerInfo*> piVec;
    piVec.push_back(&p1);
    piVec.push_back(&p2);
    piVec.push_back(&ph);
    p1.playerType = HSGlobalData::P_Seeker;
    p2.playerType = HSGlobalData::P_Seeker;
    ph.playerType = HSGlobalData::P_Hider;
    p1.id = 0;
    p2.id = 1;
    ph.id = 2;
    p1.posRead = p2.posRead = true;

    p1.currentPos.set(5,6);
    p2.currentPos.set(1,2);
    p1.useObsProb /*hiderObsTrustProb*/ = 0.7;
    p1.hiderObsPosWNoise.set(7,8);
    p2.hiderObsPosWNoise.set(7.5,8.5);
    p2.useObsProb /*hiderObsTrustProb*/ = 0.3;

    //AG160602:add dyn obst
    p1.dynObsVisibleVec.push_back(IDPos(1,2,1));
    p1.dynObsVisibleVec.push_back(IDPos(3,2,2));
    p2.dynObsVisibleVec.push_back(IDPos(1,2,1));
    p2.dynObsVisibleVec.push_back(IDPos(4,4,2));

    //size should be 1 because always the 'own' seeker's observation is available (although could be not set)
    QCOMPARE((int)hsObs.size(),1);
    hsObs.readFromPlayerInfo(&p1,piVec);
    QCOMPARE((int)hsObs.size(),2);

    QVERIFY(*hsObs.getUpdateObservationState()==HSState(p1.currentPos,p1.hiderObsPosWNoise));
    QVERIFY(*hsObs.getObservationState(0)==HSState(p1.currentPos,p1.hiderObsPosWNoise));
    QVERIFY(*hsObs.getObservationState(1)==HSState(p2.currentPos,p2.hiderObsPosWNoise));
    QVERIFY(*hsObs.getRandomState(0.6)==HSState(p1.currentPos,p1.hiderObsPosWNoise));
    QVERIFY(*hsObs.getRandomState(1)==HSState(p2.currentPos,p2.hiderObsPosWNoise));

    QCOMPARE((int)hsObs.dynObstVec.size(), 3);
}
