#include "test/include/POMCP/node-test.h"
#include <QTest>
#include <iostream>

#include "POMCP/node.h"
#include "POMCP/hssimulator.h"

#include "Base/hsglobaldata.h"

using namespace std;
using namespace pomcp;

void PomcpNodeTest::initTestCase() {
    _params = new SeekerHSParams();
    _params->useContinuousPos = true;
    _params->rewardType = SeekerHSParams::REWARD_FIND_REV_DIST;
    _map = new GMap(4,4,_params);
    //gen map
    _map->addObstacle(1,1);
    _map->addObstacle(1,2);
    _map->setMapFixed();
    //sim
    _sim = new HSSimulator(_map,_params);
}

void PomcpNodeTest::cleanupTestCase() {
    delete _sim;
    delete _map;
    delete _params;
}

void PomcpNodeTest::createAndCompareTest() {
    Node root(_sim, NULL);
    setBelief(root.getBelief(),5);

    //add some nodes
    QCOMPARE((uint)root.childCount(), _sim->getNumActions());
    QVERIFY(!root.isChildSet(0));
    NodeA* nodea0 = root.setChild(0, 2, 1.1);
    QCOMPARE((uint)root.childCount(), _sim->getNumActions());
    QCOMPARE((int)nodea0->childCount(), 0);
    NodeA* nodea3 = root.setChild(3, 32, -4.1);
    QCOMPARE((uint)root.childCount(), _sim->getNumActions());
    QCOMPARE((int)nodea3->childCount(), 0);
    //add child of child
    HSState obs1(3,2,-1,-1);
    Node* nodea3_n = nodea3->createChild(&obs1,false);
    setBelief(nodea3_n->getBelief(),5);
    //set this one's child
    NodeA* nodea3_n_a4 = nodea3_n->createChild(4);

    qDebug() << QString::fromStdString(root.toString(true));

    //now get check
    QVERIFY(root.getChild(0)==nodea0);
    QVERIFY(root.getChild(3)==nodea3);
    QVERIFY(root.getChild(5)==NULL);
    Node* child1 = nodea3->getChild(&obs1);
    QCOMPARE(child1->childCount(),nodea3_n->childCount());
    QCOMPARE(child1->toString(true),nodea3_n->toString(true));
    HSState obs2(4,4,5,5);
    QVERIFY(nodea3->getChild(&obs2)==NULL);

    //now clean test
    size_t rootChildCount = root.childCount();
    root.deleteChildrenExceptFor(&root);
    QCOMPARE(root.childCount(), rootChildCount);

    //now set new root
    Pos empty;
    HSState obs1_2(Pos(3,2),empty);
    Node* newRoot = root.getChild(3)->getChild(&obs1_2);
    QVERIFY(newRoot!=NULL);
    //delete other nodes
    root.deleteChildrenExceptFor(newRoot);
    //delete root;
    //show new tree

    qDebug() << QString::fromStdString(newRoot->toString(true));    
}


void PomcpNodeTest::setBelief(Belief *belief, int n) {
    for(int i=0;i<n;i++) {
        HSState* bpos = new HSState(_map->genRandomPos(),_map->genRandomPos());
        belief->add(bpos, true);
    }
}


