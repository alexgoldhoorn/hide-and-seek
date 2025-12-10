#ifndef MCTSTEST_H
#define MCTSTEST_H

#include <QtTest/QtTest>

class SeekerHSParams;
class GMap;
class Pos;
namespace pomcp {
    class Simulator;
    class HSState;
    class Belief;
    class HSObservation;
    class MCTS;
    class Node;
    class NodeA;
}

/*!
 * \brief The MCTSTest class Unit test for MCTS.
 */
class MCTSTest: public QObject
{
    Q_OBJECT
private slots:

    void initTestCase();

    void testNodeCopy();

    void testMCTSFuncsDiscSimpleMap();

    void testMCTSFuncsContLoadedMap();

    void testMCTSDiscSimpleMapNormalMove();

    void testMCTSDiscSimpleMapRandMove();

    void testMCTSContLoadedMapNormalMove();

    void testMCTSContLoadedMapRandMove();

    void prevTestNodes();

    void cleanupTestCase();


private:
    //void testSimulator(pomcp::Simulator* sim, GMap* map, int maxStepDistTest=MAX_STEP_DIST_TEST, int numOtherSeekers=OBS_NUM_OTHER_SEEKERS);

    pomcp::HSObservation* genObs(GMap* map, const Pos& seekerPos, const Pos& hiderObsPos, int nOther);

    pomcp::HSState* genRandState(GMap* map);

    GMap* generateGMap();

    GMap* loadMap(QString name);

    void testMCTS(pomcp::MCTS* mcts, GMap* map, pomcp::Simulator* sim, bool randMovement, std::string metaDescr);

    //AG150701: from MCTS::testNodes(GMap* map)
    //was made to detect memory leak
    void testNodes(pomcp::MCTS* mcts, GMap* map, pomcp::Simulator* sim);

    void itemTest(pomcp::MCTS* mcts, GMap* map, pomcp::Simulator* sim);

    //check the belief
    void checkBelief(GMap* map, pomcp::Belief* belief);

    //check tree
    void checkTree(pomcp::Node *node, int level=0);
    void checkTree(pomcp::NodeA *nodeA, int level);

    Pos randMove(GMap* map, const Pos& pos);

    //calculate tree depth
    int calcTreeDepth(pomcp::Node* node);
    int calcTreeDepth(pomcp::NodeA* nodeA);

    SeekerHSParams* _params;

    QString _mapPath;

    static int MAX_STEP_DIST_TEST;
    static int MAX_STEP_DIST_TEST_WNOISE;
    static int OBS_NUM_OTHER_SEEKERS;
    static int NUM_TEST_MCTS_IT;
    static QString MAP_OTHER_NAME;

};



#endif
