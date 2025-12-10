#ifndef MULTISEEKERHBEXPLORERTEST_H
#define MULTISEEKERHBEXPLORERTEST_H

#include <QtTest/QtTest>

class SeekerHSParams;
class GMap;
namespace pomcp {
class Simulator;
class Belief;
}

/*!
 * \brief The MultiSeekerHBExplorerTest class Unit test for MultiSeekerHBExplorer
 */
class MultiSeekerHBExplorerTest: public QObject
{
    Q_OBJECT
private slots:

    void initTestCase();

    void createAndCompareTest();

    void cleanupTestCase();

private:
    static const std::size_t OTHER_SEEKERS_N = 2;
    //void setBelief(pomcp::Belief* belief, int n);

    GMap* _map;
    SeekerHSParams* _params;
    //pomcp::Simulator* _sim;
};



#endif
