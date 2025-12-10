#ifndef POMCPNODETEST_H
#define POMCPNODETEST_H

#include <QtTest/QtTest>

class SeekerHSParams;
class GMap;
namespace pomcp {
class Simulator;
class Belief;
}

/*!
 * \brief The PomcpNodeTest class Unit test for POMCP Nodes
 */
class PomcpNodeTest: public QObject
{
    Q_OBJECT
private slots:

    void initTestCase();

    void createAndCompareTest();

    void cleanupTestCase();

private:
    void setBelief(pomcp::Belief* belief, int n);

    GMap* _map;
    SeekerHSParams* _params;
    pomcp::Simulator* _sim;
};



#endif
