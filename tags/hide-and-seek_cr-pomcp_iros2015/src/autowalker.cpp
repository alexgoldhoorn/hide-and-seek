#include "autowalker.h"

#include <cassert>
#include <iostream>

using namespace std;

AutoWalker::AutoWalker(size_t n) : _autoWalkerVec(n) {
}

AutoWalker::~AutoWalker() {
}


bool AutoWalker::checkMovement(size_t persI, Pos& newPos, Pos& seekerPos, Pos& hiderPos) {
    assert(newPos.isSet());

#ifdef DYN_OBST_COLL
    //max dist to persons
    double minDist = getParams()->minDistToObstacle;

    bool moveOk = true;

    //check if too close to hider/seeker
    if (seekerPos.isSet() && newPos.distanceEuc(seekerPos)<minDist || hiderPos.isSet() && newPos.distanceEuc(hiderPos)<minDist) {
        DEBUG_AUTOHIDER(cout << "AutoWalker::checkMovement: filtered person "<<persI<<" to "<<newPos.toString()<<" due to seeker/hider pos"<<endl;);
        moveOk = false;
    } else {
        //check if too close to other walkers
        for(auto i=0; i<_autoWalkerVec.size(); i++) {
            if (i==persI) continue; //and ignore, since this is the prev pos of the walker we are checking

            if (newPos.distanceEuc(_autoWalkerVec[i])<minDist) {
                DEBUG_AUTOHIDER(cout << "AutoWalker::checkMovement: filtered person "<<persI<<" to "<<newPos.toString()<<" due to person "
                                <<i<<" pos: "<<_autoWalkerVec[i].toString()<<endl;);
                moveOk = false;
                break;
            }
        }
    }

    return moveOk;
#else
    return true;
#endif
}

