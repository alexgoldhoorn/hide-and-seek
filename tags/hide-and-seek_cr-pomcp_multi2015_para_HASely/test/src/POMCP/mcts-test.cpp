#include "test/include/POMCP/mcts-test.h"
#include <QTest>
#include <iostream>
#include <cassert>

#include "POMCP/hssimulatorcont.h"
#include "POMCP/mcts.h"
#include "POMCP/belief.h"

#include "Base/hsglobaldata.h"

#include "Utils/timer.h"
#include "Utils/generic.h"

using namespace std;
using namespace pomcp;

int MCTSTest::MAX_STEP_DIST_TEST = 2;
int MCTSTest::MAX_STEP_DIST_TEST_WNOISE = 5;
int MCTSTest::OBS_NUM_OTHER_SEEKERS = 2;
int MCTSTest::NUM_TEST_MCTS_IT = 1000;
QString MCTSTest::MAP_OTHER_NAME = "map3_20x20.txt";


void MCTSTest::initTestCase() {
    _params = new SeekerHSParams();

    _params->rewardType = SeekerHSParams::REWARD_FIND_REV_DIST;
    _params->numBeliefStates = 20;
    _params->numSim = 100;
    _params->maxTreeDepth = 20;
    _params->expandCount = 2;
    _params->explorationConst = 10*10; //_map->rowCount()*_map->colCount();

    QString hspath = QProcessEnvironment::systemEnvironment().value("HSPATH",".");
    if (hspath.isEmpty())
        hspath = "/home/agoldhoorn/iri-lab/labrobotica/algorithms/hide-and-seek";
    _mapPath = hspath + "/data/maps/";
}

GMap* MCTSTest::generateGMap() {
    GMap* map = new GMap(4,4,_params);
    //gen map
    map->addObstacle(1,1);
    map->addObstacle(1,2);
    map->setMapFixed();
    return map;
}

GMap* MCTSTest::loadMap(QString name) {
    QString mapName = _mapPath+name;
    GMap* gmap = new GMap(mapName.toStdString(), _params);
    return gmap;
}


void MCTSTest::cleanupTestCase() {
    delete _params;
}


void MCTSTest::testMCTSDiscSimpleMapNormalMove() {
    cout << "--- TEST - MCTSTest::testMCTSDiscSimpleMapNormalMove --- "<<endl;
    _params->useContinuousPos = false;
    _params->setInitNodeValue = false;
    GMap* map = generateGMap();
    map->printMap();
    HSSimulator sim(map,_params);
    MCTS mcts(_params, &sim,NULL);

    testMCTS(&mcts, map, &sim, false, "testMCTSDiscSimpleMapNormalMove");

    delete map;
}

void MCTSTest::testMCTSDiscSimpleMapRandMove() {
    cout << "--- TEST - MCTSTest::testMCTSDiscSimpleMapRandMove --- "<<endl;
    _params->useContinuousPos = false;
    _params->setInitNodeValue = true;
    GMap* map = generateGMap();
    map->printMap();
    HSSimulator sim(map,_params);
    MCTS mcts(_params, &sim,NULL);

    testMCTS(&mcts, map, &sim, true, "testMCTSDiscSimpleMapRandMove");

    delete map;
}

void MCTSTest::testMCTSContLoadedMapNormalMove() {
    cout << "--- TEST - MCTSTest::testMCTSContLoadedMapNormalMove --- "<<endl;
    _params->useContinuousPos = true;
    _params->setInitNodeValue = false;
    double nextSeekerStateStdDev = _params->contNextSeekerStateStdDev ;

    GMap* map = loadMap(MAP_OTHER_NAME);
    map->printMap();
    HSSimulatorCont sim(map,_params);
    MCTS mcts(_params, &sim,NULL);

    testMCTS(&mcts, map, &sim, false, "testMCTSContLoadedMapNormalMove");

    delete map;
    _params->contNextSeekerStateStdDev = nextSeekerStateStdDev;
}

void MCTSTest::testMCTSContLoadedMapRandMove() {
    cout << "--- TEST - MCTSTest::testMCTSContLoadedMapRandMove --- "<<endl;
    _params->useContinuousPos = true;
    _params->setInitNodeValue = true;
    GMap* map = loadMap(MAP_OTHER_NAME);
    map->printMap();
    HSSimulatorCont sim(map,_params);
    MCTS mcts(_params, &sim,NULL);

    testMCTS(&mcts, map, &sim, true, "testMCTSContLoadedMapRandMove");

    delete map;
}

void MCTSTest::testMCTSFuncsDiscSimpleMap() {
    cout << "--- TEST - MCTSTest::testMCTSFuncsDiscSimpleMap --- "<<endl;
    _params->useContinuousPos = false;
    GMap* map = generateGMap();
    map->printMap();
    HSSimulator sim(map,_params);
    MCTS mcts(_params, &sim,NULL);

    itemTest(&mcts, map, &sim);

    delete map;
}

void MCTSTest::testMCTSFuncsContLoadedMap() {return;
    cout << "--- TEST - MCTSTest::testMCTSFuncsContLoadedMap --- "<<endl;
    _params->useContinuousPos = true;
    GMap* map = loadMap(MAP_OTHER_NAME);
    map->printMap();
    HSSimulatorCont sim(map,_params);
    MCTS mcts(_params, &sim,NULL);

    itemTest(&mcts, map, &sim);

    delete map;
}



void MCTSTest::prevTestNodes() {
    cout << "--- TEST - MCTSTest::prevTestNodes --- "<<endl;
    _params->useContinuousPos = true;
    GMap* map = generateGMap();
    map->printMap();
    HSSimulator sim(map,_params);
    MCTS mcts(_params, &sim,NULL);
    testNodes(&mcts, map, &sim);
    delete map;
}

void MCTSTest::itemTest(MCTS *mcts, GMap* map, Simulator* sim) {
    cout << "--- expandNode ---"<<endl;
    Node* node = new Node(sim, NULL);
    HSState* randState = genRandState(map);

    Node* nodeOut = mcts->expandNode(node, randState);
    QVERIFY(node == nodeOut);
    //check result
    for(uint a=0; a<sim->getNumActions(); a++) {
        bool isPossible = map->tryMove(a,randState->seekerPos).isSet();
        QVERIFY( (isPossible && node->getChild(a)!=NULL) || //should have the child
                (!isPossible && node->getChild(a)==NULL));  //should have the child
    }

    /*cout << "--- simulateNode ---"<<endl;
    //mcts->_treeDepth = 0;
    double v = mcts->simulateNode(node, randState);
    //todo:other sim + rollout + ..
    //+ loop to try more
    QVERIFY(mcts->_treeDepth>0 && mcts->_treeDepth<_params->maxTreeDepth);
    int treeDepth = calcTreeDepth(node);
    QCOMPARE((int)mcts->_treeDepth, treeDepth);

    node->deleteChildrenExceptFor(NULL);*/
    delete node;
    delete randState;

    //TODO: more specific tests for simulation, rollout, etc
}

int MCTSTest::calcTreeDepth(Node *node) {
    int maxD = 0;
    //search max depth
    for(size_t i=0; i<node->childCount(); i++) {
        NodeA* child = node->getChild(i);
        if (child!=NULL) {
            int d = calcTreeDepth(child);
            if (d>maxD)
                maxD = d;
        }
    }
    return maxD+1;
}
int MCTSTest::calcTreeDepth(NodeA *nodeA) {
    int maxD = 0;
    //search max depth
    for(size_t i=0; i<nodeA->childCount(); i++) {
        int d = calcTreeDepth(nodeA->getChildItem(i)->node);
        if (d>maxD)
            maxD = d;
    }
    return maxD+1;
}

void MCTSTest::testMCTS(MCTS *mcts, GMap* map, Simulator* sim, bool randMovement, string metaDescr) {
    HSObservation* obs = genObs(map, map->genRandomPos(), map->genRandomPos(), OBS_NUM_OTHER_SEEKERS);
    metaDescr = "["+metaDescr+"]";
    cout <<metaDescr<< "Obs: "<<obs->toString()<<endl;

    cout << "--- init ---"<<endl;
    mcts->init(obs);
    if (randMovement) {
        delete obs;
        obs  = NULL;
    }
    QVERIFY(mcts->getRoot()!=NULL);
    checkBelief(map, mcts->getRoot()->getBelief());
    QCOMPARE((uint)mcts->getRoot()->getBelief()->size(), _params->numBeliefStates);

    int lastA = -1;

    cout <<metaDescr<< "--- Loop "<<NUM_TEST_MCTS_IT<<" iterations of MCTS (update+learn+getAction)..."<<endl;
    for(int i = 0; i<NUM_TEST_MCTS_IT; i++) {
        cout <<metaDescr<< "**It "<<i<<"**"<<endl;
        if (lastA!=-1) {
            QVERIFY(mcts->getRoot()!=NULL);
            ulong lastRootID = mcts->getRoot()->getNodeID();

            //generate obs
            if (randMovement) {
                obs = genObs(map, map->genRandomPos(), map->genRandomPos(), OBS_NUM_OTHER_SEEKERS);
            } else {
                //move according to lastA
                obs->ownSeekerObs.seekerPos = map->tryMove(lastA, obs->ownSeekerObs.seekerPos);
                QVERIFY(lastA>=0 && lastA<HSGlobalData::NUM_ACTIONS);
                QVERIFY(obs->ownSeekerObs.seekerPos.isSet());
                //move hider
                obs->ownSeekerObs.hiderPos = randMove(map, obs->ownSeekerObs.hiderPos);
                //move others
                for(HSState& s : obs->otherSeekersObsVec) {
                    s.seekerPos = randMove(map, s.seekerPos);
                    if (s.hiderPos.isSet()) {
                        s.hiderPos = randMove(map, s.hiderPos);
                    } else {
                        if (randomDouble()<0.1) {
                            //crate random loc
                            s.hiderPos = map->genRandomPos();
                        }
                    }
                }
            }
            cout <<metaDescr<<"**"<<i<<") UPDATE** with obs="<<obs->toString()<<endl;

            //update
            mcts->update(lastA,obs);

            if (randMovement) delete obs;

            //get new root
            Node* newRoot = mcts->getRoot();

            //check new root
            QVERIFY(lastRootID=newRoot->getNodeID());
        }

        cout <<metaDescr<<"**"<<i<<") SELECT ACTION**"<<endl;
        lastA = mcts->selectAction();

        QVERIFY(lastA>=0 && lastA<HSGlobalData::NUM_ACTIONS);

        Node* root = mcts->getRoot();
        QVERIFY(root!=NULL);
        int treeDepth = calcTreeDepth(root);

        cout<<metaDescr<<"**"<<i<<" ACTION: "<<ACTION_COUT(lastA)<<", tree depth: "<<treeDepth<<endl;

        //check node
        checkTree(root);
        //check belief
        checkBelief(map, root->getBelief());

        QVERIFY(lastA>=0 && lastA<HSGlobalData::NUM_ACTIONS);
        QVERIFY(treeDepth>0 && treeDepth<_params->maxTreeDepth);
    }

    if (obs!=NULL && !randMovement) delete obs;
}

void MCTSTest::testNodes(MCTS *mcts, GMap* map, Simulator* sim) {
    //TODO: put QVERIFY

    cout<<"Generate root:"<<flush;
    Node* root = new Node(sim, NULL);
    cout <<"ok"<<endl<<"gen children:"<<flush;
    //State* state = _simulator->genInitState();
    HSState state;
    state.seekerPos = map->genRandomPos();
    state.hiderPos = map->genRandomPos();
    cout <<" state:"<<state.toString()<<".."<<flush;

    mcts->expandNode(root,&state);
    cout<<"ok"<<endl;

    for(int q=0; q<10; q++) {
        for(int a=0;a<(int)root->childCount();a++) { cout << "a="<<a<<flush;
            NodeA* nodeA = root->getChild(a);
            if (nodeA!=NULL) {
                for(int i=0;i<10;i++) {
                    HSState obs;
                    obs.seekerPos = map->genRandomPos();
                    obs.hiderPos = map->genRandomPos();
                    cout << " o="<<obs.toString()<<flush;
                    Node* node = nodeA->getChild(&obs);

                    if (node==NULL){
                        cout <<"#"<<flush;
                        node = nodeA->createChild(&obs);
                        cout <<"."<<flush;
                        mcts->expandNode(node,&obs);
                        cout <<"x"<<flush;
                    } else cout << "!"<<flush;
                }cout <<endl;
            }
        }

        cout <<"update root:"<<flush;
        NodeA* nodeAR = NULL;
        while (nodeAR==NULL) nodeAR = root->getChild( random(root->childCount()-1) );
        Node* nodeR = nodeAR->getChildItem( random(nodeAR->childCount()-1))->node;
        cout << nodeR->toString()<<endl;
        cout << "delete old:"<<flush;
        root->deleteChildrenExceptFor(nodeR);
        delete root;
        root = nodeR;
        cout << "ok"<<endl;
    }

    cout<<"TREE:"<<endl<<root->toString(true)<<endl;

    cout<<"delete all: "<<flush;
    //delete state;
    root->deleteChildrenExceptFor(NULL);
    cout<<" children .."<<flush;
    delete root;
    cout<<"root, all ok"<<endl;
}


void MCTSTest::testNodeCopy() {
    cout << "--- TEST - MCTSTest::testNodeCopy --- "<<endl;
    _params->useContinuousPos = false;
    GMap* map = generateGMap();
    map->printMap();
    HSSimulator sim(map,_params);
    MCTS mcts(_params, &sim,NULL);

    //test node
    Node* testNode = new Node(&sim,NULL);
    HSState* randState = genRandState(map);
    qDebug() << QString::fromStdString(randState->toString());
    mcts.expandNode(testNode,randState);
    QVERIFY(testNode->childCount()>0);
    //now copy node
    Node* copyNode = new Node(*testNode);
    //check equals
    ulong testNodeID = testNode->getNodeID();
    QCOMPARE(testNode->getNodeID(), copyNode->getNodeID());
    QCOMPARE(testNode->childCount(), copyNode->childCount());
    //test children
    for(uint a=0; a<testNode->childCount(); a++) {
        bool canMove = map->tryMove(a, randState->seekerPos).isSet();
        NodeA* nat = testNode->getChild(a);
        NodeA* nac = copyNode->getChild(a);
        if (canMove) {
            QVERIFY(nat!=NULL);
            QVERIFY(nac!=NULL);
            QCOMPARE(nat->getNodeID(), nac->getNodeID());
        } else {
            QVERIFY(nat==NULL);
            QVERIFY(nac==NULL);
        }
    }

    qDebug() << "Deleting TestNode children...";
    //now delete testNode and children
    testNode->deleteChildrenExceptFor(NULL);
    qDebug() << "Deleting TestNode...";
    delete testNode;
    //check if copyNode is still ok and it's children
    QCOMPARE(copyNode->getNodeID(), testNodeID);
    QVERIFY(copyNode->childCount()>0);
    //test children
    /*for(uint a=0; a<copyNode->childCount(); a++) {
        bool canMove = map->tryMove(a, randState->seekerPos).isSet();
        NodeA* nac = copyNode->getChild(a);
        if (canMove) {
            QVERIFY(nac!=NULL);
            QVERIFY(nac->getNodeID()>0);
        } else {
            QVERIFY(nac==NULL);
        }
    }
    qDebug() << "Deleting CopyNode children...";
    copyNode->deleteChildrenExceptFor(NULL);*/
    //TODO: or create copy constructor, or leave it.
    //now it doesn't work because the copying with default constructor copies child vector which
    //is list of pointers to nodes, so if we delete the original children, the children of copied node
    //will also have been deleted.
    qDebug() << "Deleting CopyNode...";
    delete copyNode;
    delete randState;
    delete map;
}

HSObservation* MCTSTest::genObs(GMap *map, const Pos& seekerPos, const Pos& hiderObsPos, int nOther) {
    double p = 1.0/(nOther+1);
    PlayerInfo thisPlayer;
    thisPlayer.currentPos = seekerPos;;
    thisPlayer.hiderObsPosWNoise = hiderObsPos;
    thisPlayer.useObsProb /*hiderObsTrustProb */= p;
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
        pi->useObsProb /*hiderObsTrustProb*/ = p;
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
}

void MCTSTest::checkBelief(GMap *map, Belief *belief) {
    if (_params->useContinuousPos)
        QCOMPARE((uint)belief->size(), _params->numBeliefStates);
    else
        QVERIFY(belief->size()>0);

    //check all states
    for(size_t i=0; i<belief->size(); i++) {
        HSState* s = HSState::castFromState((*belief)[i]);
        QVERIFY(s->seekerPos.isSet());
        QVERIFY(s->hiderPos.isSet());
        QVERIFY(map->isPosInMap(s->seekerPos));
        QVERIFY(map->isPosInMap(s->hiderPos));
    }
    //belief->get RandomSample(); ..
}

void MCTSTest::checkTree(Node *node, int level) {
    if (_params->setInitNodeValue) QVERIFY(node->getCount()>0);
    if (level==1) {
        if (node->getBelief()->size()==0) {
            qDebug()<<"empty belief, but expected contents: "<<QString::fromStdString(node->toString(true));
        }
        QVERIFY(node->getBelief()->size()>0);    
    }
    for(size_t i=0; i<node->childCount(); i++) {
        NodeA* child = node->getChild(i);
        if (child!=NULL) {
            checkTree(child,level);
        }
    }
}

void MCTSTest::checkTree(NodeA *node, int level) {
    if (_params->setInitNodeValue) QVERIFY(node->getCount()>0);
    for(size_t i=0; i<node->childCount(); i++) {
        Node* child = node->getChildItem(i)->node;
        QVERIFY(child!=NULL);
        checkTree(child,level+1);
    }
}


Pos MCTSTest::randMove(GMap *map, const Pos &pos) {
    Pos nextPos;

    do {
        int a = random(HSGlobalData::NUM_ACTIONS-1);
        nextPos = map->tryMove(a,pos);
    } while (!nextPos.isSet());

    return nextPos;
}

