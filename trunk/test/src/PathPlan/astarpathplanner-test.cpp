#include "test/include/PathPlan/astarpathplanner-test.h"

/*
AStarPathPlannerTest::AStarPathPlannerTest() {

}*/
void AStarPathPlannerTest::test() {
    astertest();
}

GMap* AStarPathPlannerTest::createMap() {
    _params.pathPlannerType = SeekerHSParams::PATHPLANNER_ASTAR;
    GMap* m = new GMap(3,3,&_params);
    m->addObstacle(1,1);
    m->setMapFixed();
    m->createPathPlanner(8);
    m->printMap();
    return m;
}


void AStarPathPlannerTest::astertest() {
    GMap* m = createMap();
    QVERIFY(m!=nullptr);

}

