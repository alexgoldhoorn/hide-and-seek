#ifndef PLAYERINFOWRAPPERTEST_H
#define PLAYERINFOWRAPPERTEST_H

#include <QtTest/QtTest>
#include <vector>

#include "Base/seekerhsparams.h"
#include "Base/playerinfowrapper.h"

/*!
 * \brief The PlayerInfoWrapperTest class Unit test for (ordening with) PlayerInfoWrapper.
 */
class PlayerInfoWrapperTest: public QObject
{        
    Q_OBJECT

public:
    PlayerInfoWrapperTest();

private slots:

    void initTestCase();

    void beliefSumTest();

    void compTest();

    void sortIDTest();

    void sortBeliefTest();

    void cleanupTestCase();


private:
    //static const int NUM_PLAYERS = 3;

    void printPlayers(QString msg);

    SeekerHSParams* _params;

    std::vector<PlayerInfoWrapper> _piWrapperVec;
    std::vector<PlayerInfo> _piVec;
};



#endif
