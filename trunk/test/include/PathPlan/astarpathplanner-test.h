#ifndef ASTARPATHPLANNERTEST_H
#define ASTARPATHPLANNERTEST_H

#include <QtTest/QtTest>
#include "HSGame/gmap.h"

class AStarPathPlannerTest: public QObject
{
    Q_OBJECT
public:

 //   AStarPathPlannerTest();
    void test();

private:
    GMap* createMap();
private slots:

    void astertest();


private:
    SeekerHSParams _params;
};



#endif
