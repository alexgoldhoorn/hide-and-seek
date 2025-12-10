#include "PathPlan/pathplanner.h"


PathPlanner::PathPlanner() {
    _useContSpace = false;
}
PathPlanner::~PathPlanner(){
}


//unsigned int astar::Node::NUM_NODES = 0;
using namespace std;

bool PathPlanner::useContinuousSpace() {
    return _useContSpace;
}
void PathPlanner::setUseContinuousSpace(bool b) {
    _useContSpace = b;
}

double PathPlanner::distance(const Pos &p1, const Pos &p2) {
    return distance(p1.col(), p1.row(), p2.col(), p2.row());
}

Pos PathPlanner::nextStep(const Pos &p1, const Pos &p2) {
    Pos rPos;
    int nextR = -1, nextC = -1;

    nextStep(p1.col(), p1.row(), p2.col(), p2.row(), &nextC, &nextR);

    rPos.set(nextR,nextC);
    return rPos;
}


